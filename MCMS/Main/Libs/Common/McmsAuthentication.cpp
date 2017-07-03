
#include "McmsAuthentication.h"
#include "ProductType.h"
#include "SysConfigKeys.h"


#include "ConfigManagerApi.h"
#include "ApiStatuses.h"
#include "Trace.h"
#include "TraceStream.h"
#include "SysConfig.h"


std::ostream& operator<< (std::ostream& os, const McmsAuthenticationS& obj )
{
	eProductType theType = (eProductType)(obj.productType);

	os  << "\nProduct Type:   "		<< ProductTypeToString(theType)
		<< "\nSwitch boardId: "		<< (int)(obj.switchBoardId)
		<< "\nSwitch subBoardId: "	<< (int)(obj.switchSubBoardId)
		<< "\nSwitch isCtrlNewGeneration: "	<< (int)(obj.isCtrlNewGeneration)
		<< std::endl;

	return os;
}





/////////////////////////////////////////////////////////////////////////////
STATUS CMcmsAuthentication::IsLegalStrongPassword(const std::string login, const std::string password, CObjString & description)
{
	// Strong password definition :
	// ============================
	// 1) 8-20 characters long (15-20 for JITC)
	// 2) At least 1 characters alphabetic, upper case (2  for JITC)
	// 3) At least 1 characters alphabetic, lower case (2  for JITC)
	// 4) At least one numeric character (2  for JITC)
	// 5) At least one special character (2  for JITC)
	// 6) Passwords shall not contain the associated login (e.g. login: John, password: 298@john - not allowed, 298@joh - allowed)
	// 7) No more than 4 successive digits
	int status = STATUS_OK;



	if ( "" == password )
		status = STATUS_INVALID_STRONG_PASSWORD;



	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bJitcMode = FALSE;
	if (sysConfig)
   	     sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

	DWORD cgfPwdLength = 0;
	if (sysConfig)
		sysConfig->GetDWORDDataByKey(CFG_KEY_MIN_PASSWORD_LENGTH, cgfPwdLength);

	WORD passLength = password.length();

	FTRACEINTO <<  "\npassword length "<< passLength;

	// ==== condition 1: pwd length should be at least as long as specified in system.cfg
    // 0 == Off, don't check pwd length
    if (STATUS_OK == status)
    {
        if (0 != cgfPwdLength && passLength < cgfPwdLength)
        {
            status = STATUS_PASSWORD_LENGTH_IS_TOO_SHORT;
        }
    }

	// ==== conditions 2-4: characters limitations
	if (STATUS_OK == status)
	{
		BOOL	l_NumOfUpperAlphabeticExist	= 0,
				l_NumOfLowerAlphabeticExist	= 0,
				l_NumOfNumericExist			= 0,
				l_NumOfSpecialCharExist		= 0,
		        l_NumOfConsecutiveRepeatedExist = 0,
		        l_NumOfMaxConsecutiveRepeatedExist = 0;

		// loop over characters
		for ( WORD i = 0 ; i < passLength ; i++ )
		{
			const char curChar = password[i];

			if ( isalpha(curChar) )
			{
				// ----- conditions 2: Num of characters alphabetic, upper case
				if ( isupper(curChar) )
				{
					l_NumOfUpperAlphabeticExist++;
					//FTRACEINTO <<  "\n i = "<< i << "upper curChar " <<curChar;
				}
				// ----- conditions 3: Num of characters alphabetic, lower case
				else
				{
					l_NumOfLowerAlphabeticExist++;
					//FTRACEINTO <<  "\n i = "<< i << "lower curChar " <<curChar;
				}
			}

			// ----- conditions 4: Num of mumeric character
			else if ( isdigit(curChar) )
			{
				l_NumOfNumericExist++;
				//FTRACEINTO <<  "\n i = "<< i << "digit curChar " <<curChar;
			}

			// ----- conditions 5: Num of special character
			else
			{
				if ( (curChar == '!' ||
                                          curChar == '@' ||	//	should be considered if '@' is a legal password character
					  curChar == '#' ||	//	should be considered if '#' is a legal password character
					  curChar == '$' ||
					  curChar == '%' ||
					  curChar == '^' ||
					  curChar == '&' ||
					  curChar == '*' ||
					  curChar == '(' ||
					  curChar == ')' ||
					  curChar == '_' ||
					  curChar == '-' ||
					  curChar == '=' ||
					  curChar == '+' ||
					  curChar == '|' ||
					  curChar == '}' ||
					  curChar == '{' ||
					  curChar == ':' ||
					  curChar == '"' ||
					  curChar == '\\'||
					  curChar == ']' ||
					  curChar == '[' ||
					  curChar == '\''||
					  curChar == ';' ||
					  curChar == '/' ||
					  curChar == '?' ||
					  curChar == '>' ||
					  curChar == '<' ||
					  curChar == ',' ||
					  curChar == '.' ||
					  curChar == ' ' ||
					  curChar == '~' ) )
				{
					 l_NumOfSpecialCharExist++;
					 //FTRACEINTO <<  "\n i = "<< i << "special curChar " <<curChar;
				}
			}


			// ==== conditions 7: Max Num of consequtive chars
			if(1 <= i)
			{
        		const char prevChar = password[i-1];
        		//if prevChar == curChar
        		if(0 == curChar - prevChar)
        		{
        			//start off with l_NumOfConsecutiveRepeatedExist=2
        		   if (l_NumOfMaxConsecutiveRepeatedExist==0)
        			   l_NumOfConsecutiveRepeatedExist=2;
        		   //increase l_NumOfConsecutiveRepeatedExist by one
        		   else
        		   {
        			   l_NumOfConsecutiveRepeatedExist++;

        		   }
        		   //update l_NumOfMaxConsecutiveRepeatedExist with highest (there might be more than one chars which are repeated)
        		   if (l_NumOfConsecutiveRepeatedExist >l_NumOfMaxConsecutiveRepeatedExist)
        		       l_NumOfMaxConsecutiveRepeatedExist=l_NumOfConsecutiveRepeatedExist;
        		}
        		else
        			  l_NumOfConsecutiveRepeatedExist=0;


			}

		}



			DWORD cgfUpperAlphabetic = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_UPPER_CASE_ALPHABETIC, cgfUpperAlphabetic);

			DWORD cgfLowerAlphabetic = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_LOWER_CASE_ALPHABETIC, cgfLowerAlphabetic);

			DWORD cgfNumeric = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_NUMERIC, cgfNumeric);

			DWORD cgfSpecialChar = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_SPECIAL_CHAR, cgfSpecialChar);

			DWORD cgfNaxConsecChar = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_MAX_PASSWORD_REPEATED_CHAR, cgfNaxConsecChar);


			if( (((DWORD)l_NumOfUpperAlphabeticExist )< cgfUpperAlphabetic)	||
			    (((DWORD)l_NumOfLowerAlphabeticExist) < cgfLowerAlphabetic)	||
			    (((DWORD)l_NumOfNumericExist) < cgfNumeric)			||
			    (((DWORD)l_NumOfSpecialCharExist)< cgfSpecialChar) ||
			    (((DWORD)l_NumOfMaxConsecutiveRepeatedExist )> cgfNaxConsecChar))
			{
				status = STATUS_INVALID_STRONG_PASSWORD;
				FTRACEINTO <<  "STATUS_INVALID_STRONG_PASSWORD  "<<
				 "\nl_NumOfUpperAlphabeticExist "<< (int)l_NumOfUpperAlphabeticExist <<"cgfUpperAlphabetic " << cgfUpperAlphabetic <<
				 "\nl_NumOfLowerAlphabeticExist "<<(int)l_NumOfLowerAlphabeticExist << "cgfLowerAlphabetic " << cgfLowerAlphabetic <<
				 "\nl_NumOfNumericExist " <<(int)l_NumOfNumericExist << "cgfNumeric " << cgfNumeric <<
				 "\nl_NumOfSpecialCharExist " <<(int)l_NumOfSpecialCharExist << "cgfSpecialChar " << cgfSpecialChar <<
				 "\nl_NumOfMaxConsecutiveRepeatedExist " <<(int)l_NumOfMaxConsecutiveRepeatedExist << "cgfNaxConsecChar " << cgfNaxConsecChar ;
			}

	}

	//if login==NULL it means it is not a password from AuthenticationManager
		// it is a pass from confParty Process and we are not in Jitc than do nothing.
	if ( (STATUS_OK == status) && (login.c_str() != NULL) )
	{
		// ==== conditions 6: Passwords shall not contain the associated login
		CObjString tmpLogin(login.c_str());
		CObjString tmpPwd(password.c_str());
		tmpPwd.ToLower();
		tmpLogin.ToLower();
		if ( NULL != strstr( tmpPwd.GetString(), tmpLogin.GetString() ))
		{
			status = STATUS_INVALID_STRONG_PASSWORD;

		}
	}

	// ==== conditions 7: No 4 successive digits
	if ( STATUS_OK == status )
	{
		BOOL numOfSuccessiveDigits = 0,
			 max_successiveDigits = 0;
		// loop over characters
		for ( WORD i = 0 ; i < passLength ; i++ )
		{
			const char curChar = password[i];
			if ( isdigit(curChar) )
				numOfSuccessiveDigits++;
			else
			{
				if(numOfSuccessiveDigits && numOfSuccessiveDigits > max_successiveDigits)
					max_successiveDigits = numOfSuccessiveDigits;

				numOfSuccessiveDigits=0;
			}
		}
		if(numOfSuccessiveDigits && numOfSuccessiveDigits > max_successiveDigits)
			max_successiveDigits = numOfSuccessiveDigits;

		if(4 < max_successiveDigits)
			status = STATUS_INVALID_STRONG_PASSWORD;

	}

	if(status == STATUS_INVALID_STRONG_PASSWORD && bJitcMode)
		status = STATUS_INVALID_JITC_STRONG_PASSWORD;

	if(STATUS_OK != status)
	{
		DWORD cgfUpperAlphabetic = 0;
		sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_UPPER_CASE_ALPHABETIC, cgfUpperAlphabetic);

			DWORD cgfLowerAlphabetic = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_LOWER_CASE_ALPHABETIC, cgfLowerAlphabetic);

			DWORD cgfNumeric = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_NUMERIC, cgfNumeric);

			DWORD cgfSpecialChar = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_SPECIAL_CHAR, cgfSpecialChar);

			DWORD cgfNaxConsecChar = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_MAX_PASSWORD_REPEATED_CHAR, cgfNaxConsecChar);

		description = "The password should:\n";
		description << "1) Be "<< cgfPwdLength <<"-20 characters long.\n";
		description << "2) Contain at least "<< cgfUpperAlphabetic <<"characters alphabetic, upper case\n";
		description << "3) Contain at least "<< cgfLowerAlphabetic << " characters alphabetic, lower case\n";
		description << "4) At least " << cgfNumeric << " numeric characters\n";
		description << "5) At least " << cgfSpecialChar << " special characters\n";
		description << "6) Passwords shall not contain the associated login (e.g. login: John, password: 298@john - not allowed, 298@joh - allowed)\n";
		description << "7) Passwords shall not contain more than "<< cgfNaxConsecChar<<" successive digits.\n";
	}


	return status;
}


//////////////////////////////////////////////////////////////////////
BOOL CMcmsAuthentication::IsForceStrongPassword()
{
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bForceStrongPassword;
	sysConfig->GetBOOLDataByKey(CFG_KEY_FORCE_STRONG_PASSWORD_POLICY, bForceStrongPassword);

	return bForceStrongPassword;
}

STATUS CMcmsAuthentication::VerifyStrongPassword(char * password,CObjString & description)
{
	   STATUS status = STATUS_OK;


	   eStringValidityStatus isLegalAsciiPwd = CObjString::IsLegalAsciiString(password,
																				strlen(password)+1,
	                                                                             false,true);

	    if (eStringValid != isLegalAsciiPwd)
	    {
	        FTRACESTR(eLevelInfoNormal) << "CMcmsAuthentication::VerifyStrongPassword: Illegal ASCII characters:\n";

	        status=STATUS_ILLEGAL_CHARACTERS_IN_PASSWORD;
	    	return status;
	    }


	    BOOL bForcePwdPolicy = CMcmsAuthentication::IsForceStrongPassword();

	    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	    BOOL bJitcMode = FALSE;
	    sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);



	    if(bJitcMode || bForcePwdPolicy)
	    	status = CMcmsAuthentication::IsLegalStrongPassword("", password, description);

	    return status;

}





