
#include <stdlib.h>
#include "RtmIsdnPhoneNumberRange.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"



CRtmIsdnPhoneNumberRange::CRtmIsdnPhoneNumberRange()
{
	//defaults
	m_firstPhoneNumber[0] = '\0';
	m_lastPhoneNumber[0]  = '\0';
	m_category            = 0;
	m_firstPortId         = 0;
	m_dialInGroupId       = 0;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnPhoneNumberRange::~CRtmIsdnPhoneNumberRange()
{
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnPhoneNumberRange::Serialize(WORD format, COstrStream &m_ostr)
{
	// assuming format = OPERATOR_MCMS
	m_ostr << m_firstPhoneNumber   << "\n"; 
	m_ostr << m_lastPhoneNumber   << "\n"; 
	m_ostr << m_category   << "\n"; 
	m_ostr << (WORD)m_firstPortId   << "\n"; 
	m_ostr << m_dialInGroupId   << "\n"; 
} 

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnPhoneNumberRange::DeSerialize(WORD format, CIstrStream &m_istr)
{
	// assuming format = OPERATOR_MCMS
	//m_istr.ignore(1);
	m_istr.getline(m_firstPhoneNumber,ISDN_PHONE_NUMBER_DIGITS_LEN+1,'\n');
	m_istr.getline(m_lastPhoneNumber,ISDN_PHONE_NUMBER_DIGITS_LEN+1,'\n');
	m_istr >> m_category;
	WORD tmp;
	m_istr >> tmp;
	m_firstPortId = (BYTE)tmp;
	m_istr >> m_dialInGroupId;
} 
/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnPhoneNumberRange::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pIsdnPhoneNumNode = pFatherNode->AddChildNode("NET_PHONE");
	pIsdnPhoneNumNode->AddChildNode("FIRST_PHONE_NUMBER", m_firstPhoneNumber);
	pIsdnPhoneNumNode->AddChildNode("LAST_PHONE_NUMBER", m_lastPhoneNumber);
	pIsdnPhoneNumNode->AddChildNode("CATEGORY", m_category);
	pIsdnPhoneNumNode->AddChildNode("FIRST_PORT_ID", m_firstPortId);
	pIsdnPhoneNumNode->AddChildNode("DIAL_IN_GROUP_ID", m_dialInGroupId);
}
/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnPhoneNumberRange::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"FIRST_PHONE_NUMBER",m_firstPhoneNumber, PHONE_NUMBER_DIGITS_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"LAST_PHONE_NUMBER",m_lastPhoneNumber, PHONE_NUMBER_DIGITS_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"CATEGORY",&m_category, _0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"FIRST_PORT_ID",&m_firstPortId, _0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"DIAL_IN_GROUP_ID",&m_dialInGroupId, _0_TO_WORD);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
WORD  CRtmIsdnPhoneNumberRange::GetDialInGroupId () const                 
{
    return m_dialInGroupId;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnPhoneNumberRange::SetDialInGroupId(const WORD  dialInGroupId)                 
{
	m_dialInGroupId=dialInGroupId;
}


/////////////////////////////////////////////////////////////////////////////
BYTE  CRtmIsdnPhoneNumberRange::GetFirstPortId() const                 
{
    return m_firstPortId;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnPhoneNumberRange::SetFirstPortId(const BYTE  portId)                 
{
	m_firstPortId=portId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CRtmIsdnPhoneNumberRange::GetCategory() const                 
{
    return m_category;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnPhoneNumberRange::SetCategory(const DWORD  category)                 
{
	m_category=category;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CRtmIsdnPhoneNumberRange::GetFirstPhoneNumber () const                 
{
	return m_firstPhoneNumber;
}


/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnPhoneNumberRange::SetFirstPhoneNumber(const char* phoneNumber)                 
{
	strncpy(m_firstPhoneNumber, phoneNumber, ISDN_PHONE_NUMBER_DIGITS_LEN - 1);

	// Cheaper to simply assign the null than to check: int len = strlen(phoneNumber);if (ISDN_PHONE_NUMBER_DIGITS_LEN <= len)
	m_firstPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN-1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
const char*  CRtmIsdnPhoneNumberRange::GetLastPhoneNumber () const                 
{
	return m_lastPhoneNumber;
}


/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnPhoneNumberRange::SetLastPhoneNumber(const char* phoneNumber)                 
{
	strncpy(m_lastPhoneNumber, phoneNumber, ISDN_PHONE_NUMBER_DIGITS_LEN - 1);

	// Cheaper to simply assign the null than to check: int len = strlen(phoneNumber);	if (ISDN_PHONE_NUMBER_DIGITS_LEN <= len)
	m_lastPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN-1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnPhoneNumberRange::IsMaxNumbersInRangeExceeded() const
{
	bool isExceeded = false;

	// ===== 1. get num of digits in MAX_NUMBERS_IN_PHONE_RANGE
	char bufMax[ISDN_PHONE_NUMBER_DIGITS_LEN];
	int maxNumLen = sprintf(bufMax, "%d", MAX_NUMBERS_IN_PHONE_RANGE);

	// ===== 2. get num of digits of the phone numbers in current range
    string phoneNumStr = m_firstPhoneNumber;
	int phoneNumLen = phoneNumStr.length();
	
	// ===== 3. find the 1st digit in which lastPhoneNum is greater than firstPhoneNum
	int i=0;
	for (i=0; i<phoneNumLen; i++)
	{
		if ( m_lastPhoneNumber[i] > m_firstPhoneNumber[i] )
			break;						// now 'i' is the idx of that digit
	}
	int digitPlace = phoneNumLen - i;	// the place (from the right) of that digit
	

	
	if ( digitPlace > maxNumLen )		// it's obviously exceeds
	{
		isExceeded = true;
	}
	else
	{
		DWORD dFirstNum = atoi(m_firstPhoneNumber + i), // get the substring starting from that digit
		      dLastNum  = atoi(m_lastPhoneNumber + i);
		DWORD totalNumbers = dLastNum - dFirstNum;		// get the number of phoneNumbers in the range
		
		if (MAX_NUMBERS_IN_PHONE_RANGE <= totalNumbers)
			isExceeded = true;
	}

	return isExceeded;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CRtmIsdnPhoneNumberRange::NameOf() const                
{
	return "CRtmIsdnPhoneNumberRange";
}                                                                      

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnPhoneNumberRange::SetParams(RTM_ISDN_PHONE_RANGE_S &phoneRangeStruct)
{
    SetDialInGroupId(phoneRangeStruct.dialInGroupId);    
    SetFirstPhoneNumber( (char*)(phoneRangeStruct.firstPhoneNumber) );
    SetLastPhoneNumber( (char*)(phoneRangeStruct.lastPhoneNumber) );
    SetCategory(phoneRangeStruct.category);
    SetFirstPortId(phoneRangeStruct.firstPortId);
}
