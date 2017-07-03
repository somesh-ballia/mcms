// KeyCode.cpp

#include "KeyCode.h"

#include <stdio.h>
#include "c_keycode.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "McuMngrStructs.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "Versions.h"

// ------------------------------------------------------------
CKeyCode::CKeyCode (CMedString keyCodeStr)
{
	m_keyCode = keyCodeStr;
}

CKeyCode::CKeyCode (const char * str)
{
	CMedString keyCodeStr(str);
	m_keyCode = keyCodeStr;
}
// ------------------------------------------------------------
CKeyCode::CKeyCode(const CKeyCode &other):
CSerializeObject(other)
{
	m_keyCode = other.m_keyCode;
}


// ------------------------------------------------------------
CKeyCode::~CKeyCode ()
{
}


// ------------------------------------------------------------
void  CKeyCode::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n\n"
	    << "KeyCode::Dump\n"
		<< "-------------\n";

	msg << "KeyCode: " << m_keyCode
	    << " (type: "  << m_keyCode[0] << ")";
}


// ------------------------------------------------------------
void CKeyCode::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pKeycodeNode = pFatherNode->AddChildNode( "KEY_CODE", m_keyCode.GetString() );
}


// ------------------------------------------------------------
int	 CKeyCode::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
    STATUS nStatus = STATUS_OK;

    char keycodeStr[KEYCODE_LENGTH];
    memset(keycodeStr,0,KEYCODE_LENGTH);
    GET_VALIDATE_CHILD(pActionNode ,"KEY_CODE",keycodeStr,_0_TO_KEYCODE_LENGTH);
    m_keyCode = keycodeStr;

    return nStatus;
}

// ------------------------------------------------------------
int	 CKeyCode::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int res = DeSerializeXml(pActionNode, pszError);
    return res;
}


// ------------------------------------------------------------
CKeyCode& CKeyCode::operator = (const CKeyCode &rOther)
{
    m_keyCode = rOther.m_keyCode;
    return *this;
}


// ------------------------------------------------------------
CKeyCode& CKeyCode::operator = (const char* rOther)
{
    m_keyCode = rOther;
    return *this;
}


// ------------------------------------------------------------
void CKeyCode::SetKeyCode(CMedString  KeyCode)
{
    m_keyCode = KeyCode;
}


// ------------------------------------------------------------
CMedString  CKeyCode::GetKeyCode()
{
	return m_keyCode;
}

// ------------------------------------------------------------
char CKeyCode::GetKeyCodeType()
{
	return m_keyCode[0];
}


// ------------------------------------------------------------
int CKeyCode::GetKeyCodeLength()
{
	int keyCodeLength = strlen ( m_keyCode.GetString() );
	return keyCodeLength;
}

STATUS CKeyCode::Validate(eProductType productType, CSmallString mplSerialNumberStr,
                          int verMajor/*=-1*/,
                          int verMinor/*=-1*/,
                          VERSION_S* keyCodeVersion/*=NULL*/)
{
	STATUS retStatus = STATUS_OK;
	CLargeString resStr = "\nCKeyCode::Validate - ";

	if (EXACT_KEYCODE_LENGTH_IN_GL1 != m_keyCode.GetStringLength())
	{
		retStatus = STATUS_FAIL;
		resStr << " Illegal keyCode length!"
		       << "\nlength: " << m_keyCode.GetStringLength() << " bytes "
		       << " (while legal length is " << EXACT_KEYCODE_LENGTH_IN_GL1 << " bytes)";
	}
	else
	{
		// ===== 1. validation: compare original keycode to generated keycode
		CSmallString inputOptionBitmask;
		GenerateInputBitmask(productType, inputOptionBitmask, verMajor, verMinor, keyCodeVersion);

		char szCreatedKeyCode[KEYCODE_LENGTH] = {0};
		memset(szCreatedKeyCode, 0, KEYCODE_LENGTH);

		GenerateKeyCode(szCreatedKeyCode,
		                (char*)(mplSerialNumberStr.GetString()),
		                (char*)(inputOptionBitmask.GetString()),
		                GetKeyCodeType() );

		resStr << "\nOriginal KeyCode:  " << m_keyCode.GetString()
		       << " (serial number: "<< mplSerialNumberStr.GetString() << ")";

		if (m_keyCode != szCreatedKeyCode)
		{
			retStatus = STATUS_CFS_MSG_13;
			resStr << " does not match the expected keycode";
		}

		// ===== 2. print to log
		if (STATUS_OK == retStatus)
		{
			resStr << "\n ---- KeyCode Validation succeeded!";
		}
		else
		{
			resStr << "\n ---- Mismatch!! KeyCode Validation failed!";
		}

	} // end if (length == EXACT_KEYCODE_LENGTH_IN_GL1)

	TRACESTR(eLevelInfoNormal) << resStr.GetString();

	return retStatus;
}

void CKeyCode::GenerateInputBitmask(eProductType productType, CSmallString& inBitmask,
                                    int verMajor /* =-1 */,
                                    int verMinor /* =-1 */,
                                    VERSION_S* keyCodeVersionFromSwitch /* =NULL */)
{
	// ===== 1. get version number
  VERSION_S mcuVer;
  bool is_downgrade = false;

  if (NULL != keyCodeVersionFromSwitch)
  {
    TRACEINTOFUNC << "keyCodeVersion->ver_major " << keyCodeVersionFromSwitch->ver_major
                  << ", keyCodeVersion->ver_minor " << keyCodeVersionFromSwitch->ver_minor;
  }

  if (-1 == verMajor)
  {
    // ----- from file
    CVersions version;
    if (TRUE==IsTarget() //For Simulation do not match the Keycode , use default KeyCode
	|| (eProductTypeGesher==productType || eProductTypeNinja==productType || eProductTypeEdgeAxis==productType))
    {
    	std::string versionFilePath = VERSIONS_FILE_PATH;
    	version.ReadXmlFile(versionFilePath.c_str());
    }
    mcuVer = version.GetMcuVersion();

    // If downgrade - use version of the key code (Licensing backward compatibility)
    // on startup on authenticationInd msg Switch sends the lastKeycode he has for backward compatibility
    // if the current mcuVersion is less that the keycode we have from switch .
    // we still use the Licensing backward compatibility which mean the keycode from switch keyCodeVersionFromSwitch.
    if (NULL != keyCodeVersionFromSwitch && (APIU32)-1 != keyCodeVersionFromSwitch->ver_major)
    {
      if (mcuVer.ver_major < keyCodeVersionFromSwitch->ver_major ||
          (mcuVer.ver_major == keyCodeVersionFromSwitch->ver_major &&
           mcuVer.ver_minor < keyCodeVersionFromSwitch->ver_minor))
      {
        is_downgrade = true;
      }
    }
  }
	else
	{
		// ----- from parameters
		mcuVer.ver_major = (APIU32)verMajor;
		mcuVer.ver_minor = (APIU32)verMinor;
		mcuVer.ver_release = 0;
		mcuVer.ver_internal	= 0;
	}

	if (is_downgrade)
	{
		inBitmask << "D"
		          << keyCodeVersionFromSwitch->ver_major
		          << "."
		          << keyCodeVersionFromSwitch->ver_minor;
	}
	else
	{
		inBitmask << "D"
		          << mcuVer.ver_major
		          << "."
		          << mcuVer.ver_minor;
	}

	// ===== 2. get bitmask from keycode
	char szOptions[CFS_OPTIONS_BITMASK_LENGTH] = {0};
	memset(szOptions, 0, CFS_OPTIONS_BITMASK_LENGTH);
	GetOptionsFromKeyCode(szOptions, (char*)(m_keyCode.GetString()));
	inBitmask << szOptions;

	TRACEINTOFUNC << "inBitmask " << inBitmask.GetString();
}

void CKeyCode::GenerateKeyCode(char keycode[], char serial[], char option[], char c_type)
{
	// call Polycom's CFS function
	generateKeyCode(keycode, serial, option, c_type);
}

// ------------------------------------------------------------
void CKeyCode::GetOptionsFromKeyCode(char option_mask[], char key_code[])
{
	// call Polycom's CFS function
	getOptionsFromKeyCode(option_mask, key_code);
}

// ------------------------------------------------------------
void CKeyCode::GetVersion(VERSION_S& ver)
{
	// in U_keycode, the option string contains versionNumber in hex format (e.g. "00000325" for version 8.5)

	// ===== 1. get options string
	char szOptions[CFS_OPTIONS_BITMASK_LENGTH];
	memset(szOptions, 0, CFS_OPTIONS_BITMASK_LENGTH);
	getOptionsFromKeyCode( szOptions, (char*)(m_keyCode.GetString()) );

	// ===== 2. convert to int
	int verNumber = 0;
	sscanf( szOptions, "%x", &verNumber);	// in the example: "00000325" -> 805

	// ===== 3. fill VERSION_S
	ver.ver_major    = verNumber / 100;		// in the example: 8
	ver.ver_minor    = verNumber % 100;		// in the example: 5
	ver.ver_release  = 0;					// default
	ver.ver_internal = 0;					// default

/*
	// in U_keycode, the option string contains versionNumber in hex format (e.g. "0000004b" for version 7.5)

	// ===== 1. get options string
	char szOptions[CFS_OPTIONS_BITMASK_LENGTH];
	memset(szOptions, 0, CFS_OPTIONS_BITMASK_LENGTH);
	getOptionsFromKeyCode( szOptions, (char*)(m_keyCode.GetString()) );

	// ===== 2. convert to int
	int verNumber = 0;
	sscanf( szOptions, "%x", &verNumber);	// in the example: "0000004b" -> 75

	// ===== 3. fill VERSION_S
	ver.ver_major    = verNumber / 10;		// in the example: 7
	ver.ver_minor    = verNumber % 10;		// in the example: 5
	ver.ver_release  = 0;					// default
	ver.ver_internal = 0;					// default
*/
}
