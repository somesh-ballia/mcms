#include "SnmpV3SecurityParams.h"

#include "psosxml.h"
#include "StatusesGeneral.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "EncodeHelper.h"

static const char* AuthProtocolStr(eSnmpAuthProtocol ap)
{
	switch(ap)
	{
	case eSapNone:
		return "none";

	case eSapMD5:
		return "MD5";

	case eSapSHA:
		return "SHA";

	default:
	    FPASSERTSTREAM_AND_RETURN_VALUE(1,
           "Unknown SNMPv3 Authentication Protocol: " << ap,
           "unknown");
	}

	return "none";
}

static const char* PrivProtocolStr(eSnmpPrivProtocol pp)
{
	switch(pp)
	{
	case eSppNone:
		return "none";

	case eSppDES:
		return "DES";
	case eSppAES:
		return "AES";

	default:
	    FPASSERTSTREAM_AND_RETURN_VALUE(1,
           "Unknown SNMPv3 Private Protocol: " << pp,
           "unknown");
	}

	return "none";
}

static const char* SecurityLevelStr(eSnmpSecurityLevel pp)
{
	switch(pp)
	{
	case eSslNone:
		return "none";
	case eSslNoAuth:
		return "noauth";
	case eSslAuth:
		return "auth";
	case eSslPriv:
		return "priv";

	default:
	    FPASSERTSTREAM_AND_RETURN_VALUE(1,
           "Unknown SNMPv3 Security Level: " << pp,
           "unknown");
	}

	return "none";
}

static const char* SecurityLevelStr1(eSnmpSecurityLevel pp)
{
	switch(pp)
	{
	case eSslNone:
		return "none";
	case eSslNoAuth:
		return "noAuthNoPriv";
	case eSslAuth:
		return "authNoPriv";
	case eSslPriv:
		return "authPriv";

	default:
	    FPASSERTSTREAM_AND_RETURN_VALUE(1,
           "Unknown SNMPv3 Security Level: " << pp,
           "unknown");
	}

	return "none";
}

CSnmpV3SecurityParams::CSnmpV3SecurityParams(void) :
	m_userName(""),
	m_engineID(""),
	m_secLevel(eSslPriv),

	m_authProtocol(eSapSHA),
	m_authPassword(""),
	m_authPassword_enc(""),
	m_privProtocol(eSppAES),
	m_privPassword(""),
	m_privPassword_enc(""),
	m_isFromEma(true)

{ }

CSnmpV3SecurityParams::CSnmpV3SecurityParams(const CSnmpV3SecurityParams& rhs) :
	CSerializeObject(rhs),
	m_userName(rhs.m_userName),
	m_engineID(rhs.m_engineID),
	m_secLevel(rhs.m_secLevel),
	m_authProtocol(rhs.m_authProtocol),
	m_authPassword(rhs.m_authPassword),
	m_authPassword_enc(rhs.m_authPassword_enc),
	m_privProtocol(rhs.m_privProtocol),
	m_privPassword(rhs.m_privPassword),
	m_privPassword_enc(rhs.m_privPassword_enc),
	m_isFromEma(rhs.m_isFromEma)
{
}

void CSnmpV3SecurityParams::IniDefaults()
{
	m_userName = "";
	m_engineID = "";
	m_secLevel = eSslPriv;
	m_authProtocol = eSapSHA;
	m_authPassword = "";
	m_authPassword_enc = "";
	m_privProtocol = eSppAES;
	m_privPassword = "";
	m_privPassword_enc = "";
}
CSnmpV3SecurityParams& CSnmpV3SecurityParams::operator=(const CSnmpV3SecurityParams& rhs)
{
	CSerializeObject::operator=(rhs);

	m_userName = rhs.m_userName;
	m_engineID = rhs.m_engineID;
	m_secLevel     = rhs.m_secLevel;
	m_authProtocol = rhs.m_authProtocol;
	m_authPassword = rhs.m_authPassword;
	m_authPassword_enc = rhs.m_authPassword_enc;
	m_privProtocol = rhs.m_privProtocol;
	m_privPassword = rhs.m_privPassword;
	m_privPassword_enc = rhs.m_privPassword_enc;
	m_isFromEma = rhs.m_isFromEma;

	return *this;
}

bool CSnmpV3SecurityParams::operator==(const CSnmpV3SecurityParams& other)
{
	return (m_userName == other.m_userName &&
			m_engineID == other.m_engineID &&
			m_secLevel == other.m_secLevel &&
			m_authProtocol == other.m_authProtocol &&
			m_authPassword == other.m_authPassword &&
			m_authPassword_enc == other.m_authPassword_enc &&
			m_privProtocol == other.m_privProtocol &&
			m_privPassword == other.m_privPassword &&
			m_privPassword_enc == other.m_privPassword_enc);
}


// virtual
CSnmpV3SecurityParams::~CSnmpV3SecurityParams(void)
{ }

const char* CSnmpV3SecurityParams::NameOf(void) const
{
	return "CSnmpV3SecurityParams";
}

// virtual
CSerializeObject* CSnmpV3SecurityParams::Clone(void)
{
	return new CSnmpV3SecurityParams(*this);
}

// virtual
void CSnmpV3SecurityParams::SerializeXml(CXMLDOMElement*& root,bool isToEma) const
{
	root->AddChildNode("SECURITY_USER_NAME", m_userName);
	root->AddChildNode("ENGINE_ID", m_engineID);
	root->AddChildNode("SECURITY_LEVEL", m_secLevel,SNMP_SECURITY_LEVEL);
	root->AddChildNode("AUTHENTICATION_PROTOCOL", m_authProtocol, SNMP_AUTH_PROTOCOL);

	root->AddChildNode("PRIVACY_PROTOCOL", m_privProtocol, SNMP_PRIV_PROTOCOL);

	if (true == isToEma)
	{
		root->AddChildNode("PRIVACY_PASSWORD", m_privPassword);
		root->AddChildNode("AUTHENTICATION_PASSWORD", m_authPassword);

	}
	else // to file
	{

		root->AddChildNode("PRIVACY_PASSWORD", m_privPassword_enc);
		root->AddChildNode("AUTHENTICATION_PASSWORD",m_authPassword_enc);


	}

}

// virtual
void CSnmpV3SecurityParams::SerializeXml(CXMLDOMElement*& root) const
{
	root->AddChildNode("SECURITY_USER_NAME", m_userName);
	root->AddChildNode("ENGINE_ID", m_engineID);
	root->AddChildNode("SECURITY_LEVEL", m_secLevel,SNMP_SECURITY_LEVEL);
	root->AddChildNode("AUTHENTICATION_PROTOCOL", m_authProtocol, SNMP_AUTH_PROTOCOL);

	root->AddChildNode("PRIVACY_PROTOCOL", m_privProtocol, SNMP_PRIV_PROTOCOL);


	root->AddChildNode("PRIVACY_PASSWORD", m_privPassword);
	root->AddChildNode("AUTHENTICATION_PASSWORD", m_authPassword);

}



/*void CSnmpV3SecurityParams::EncryptPassword(std::string  srcPass , std::string& dstPass)
{
  std::string sEnc;
  std::string base64tempEnc;


  EncodeHelper eH;
  eH.EncodeAes(srcPass, sEnc);
  base64tempEnc = eH.base64_encode((unsigned char*)sEnc.c_str(), strlen(sEnc.c_str()));

  dstPass = base64tempEnc;
}

void CSnmpV3SecurityParams::DecryptPassword(std::string  srcPass , std::string& dstPass)
{
  TRACESTR(eLevelInfoNormal) << "\nCSnmpV3SecurityParams::DecryptH323Password";


  std::string base64tempEnc;
  std::string base64tempDec;

  EncodeHelper eH;


  if (srcPass.length() == 0)
    return;

  base64tempEnc = srcPass.c_str();
  base64tempDec = eH.base64_decode(base64tempEnc);
  eH.DecodeAes(base64tempDec, dstPass);


}*/

void CSnmpV3SecurityParams::SetAuthPassword(const std::string & authPassword)
{
	m_authPassword =  authPassword;
	m_authPassword_enc = "";
	EncodeHelper eH;
	eH.EncryptPassword(m_authPassword , m_authPassword_enc);
}

void CSnmpV3SecurityParams::SetPrivPassword(const std::string &privPassword)
{
	m_privPassword =  privPassword;
	m_privPassword_enc = "";
	EncodeHelper eH;
	eH.EncryptPassword(m_privPassword , m_privPassword_enc);
}

std::string CSnmpV3SecurityParams::GetAuthPassword() const
{
	return m_authPassword;
}
string CSnmpV3SecurityParams::GetPrivPassword() const
{
	return m_privPassword;
}

void CSnmpV3SecurityParams::SetUserName(const std::string & userName)
{
	m_userName = userName;
}


// virtual
int CSnmpV3SecurityParams::DeSerializeXml(CXMLDOMElement* root,
										  char* pszError,
										  const char* /* action */)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_ASCII_CHILD(root,
	                         "SECURITY_USER_NAME",
	                         m_userName,
	                         _0_TO_SNMP_STRING_LENGTH);

	GET_VALIDATE_ASCII_CHILD(root,
	                         "ENGINE_ID",
	                         m_engineID,
	                         _0_TO_SNMP_STRING_LENGTH);

	WORD val;
	val = (WORD) eSslPriv;

	GET_VALIDATE_CHILD(root,
                       "SECURITY_LEVEL",
                       &val,
                       SNMP_SECURITY_LEVEL);
	m_secLevel = (eSnmpSecurityLevel) val;

	val = (WORD) eSapSHA;

	GET_VALIDATE_CHILD(root,
                       "AUTHENTICATION_PROTOCOL",
                       &val,
                       SNMP_AUTH_PROTOCOL);

	m_authProtocol = (eSnmpAuthProtocol) val;

	val = (WORD) eSppAES;

	GET_VALIDATE_CHILD(root,
                       "PRIVACY_PROTOCOL",
                       &val,
                       SNMP_PRIV_PROTOCOL);
	m_privProtocol = (eSnmpPrivProtocol) val;

	EncodeHelper eH;

	if (m_isFromEma)
	{
		m_authPassword = "";
		m_authPassword_enc = "";
		GET_VALIDATE_ASCII_CHILD(root,
		                         "AUTHENTICATION_PASSWORD",
		                         m_authPassword,
		                         _0_TO_SNMP_STRING_LENGTH);

		eH.EncryptPassword(m_authPassword , m_authPassword_enc);

		m_privPassword = "";
		m_privPassword_enc = "";

		GET_VALIDATE_ASCII_CHILD(root,
								 "PRIVACY_PASSWORD",
								 m_privPassword,
								 _0_TO_SNMP_STRING_LENGTH);


		eH.EncryptPassword(m_privPassword , m_privPassword_enc);

	}
	else
	{

		m_authPassword = "";
		m_authPassword_enc = "";
		GET_VALIDATE_ASCII_CHILD(root,
		                         "AUTHENTICATION_PASSWORD",
		                         m_authPassword_enc,
		                         _0_TO_SNMP_STRING_LENGTH);

		if (m_authPassword_enc.length() > 0)
		{
			eH.DecryptPassword(m_authPassword_enc , m_authPassword);
		}

		m_privPassword = "";
		m_privPassword_enc = "";

		GET_VALIDATE_ASCII_CHILD(root,
								 "PRIVACY_PASSWORD",
								 m_privPassword_enc,
								 _0_TO_SNMP_STRING_LENGTH);

		if (m_privPassword_enc.length() > 0)
		{
			eH.DecryptPassword(m_privPassword_enc , m_privPassword);
		}

	}

    return nStatus;
}

void CSnmpV3SecurityParams::UserConfig(std::ostream& out) const
{
    PASSERTMSG_AND_RETURN(m_userName.empty(), "SNMPv3 user name is invalid");

  
	out << "createUser "
		<< m_userName;

	string szPasswd;


	if(m_authProtocol != eSapNone)
	{
		if("" == m_authPassword || (m_secLevel != eSslAuth && m_secLevel != eSslPriv))
		{
			szPasswd = "11111111";
					
			
		}
		else
		{
			szPasswd = m_authPassword;	
					
		}

		out << " "
			<< AuthProtocolStr(m_authProtocol)
			<< " "
			<< szPasswd;
	}


	if(m_privProtocol != eSppNone)
	{
		string privPasswd;
		if("" == m_privPassword || (m_secLevel != eSslPriv))
		{

			privPasswd = "";
		}
		else
		{
			privPasswd = m_privPassword;
		}

		out << " "
			<< PrivProtocolStr(m_privProtocol)
			<< " "
			<< privPasswd;
	}

	out << std::endl
		<< "rouser "
		<< m_userName
		<< " "
		<< SecurityLevelStr(m_secLevel)
		<< std::endl;
}

void CSnmpV3SecurityParams::TrapConfig(std::ostream& out, BOOL ignoreEngineID) const
{
    PASSERTMSG_AND_RETURN(m_userName.empty(), "SNMPv3 user name is invalid");

  
	//str << "createUser traptest SHA mypassword  AES mypassword" <<endl;
	out << "createUser"
		<< " "
		<< m_userName;

	string szPasswd;

	if(m_authProtocol != eSapNone)
	{
		if("" == m_authPassword  || (m_secLevel != eSslAuth && m_secLevel != eSslPriv))
		{
			szPasswd = "11111111";

		}
		else
		{
			szPasswd = m_authPassword;			
		}
	
		out << " "
			<< AuthProtocolStr(m_authProtocol)
			<< " "
			<< szPasswd;
	}

	if(m_privProtocol != eSppNone)
	{
		string privPasswd;

		if("" == m_privPassword || (m_secLevel != eSslPriv))
		{

			privPasswd = "";
		}
		else
		{
			privPasswd = m_privPassword;
		}

		out << " "
			<< PrivProtocolStr(m_privProtocol)
			<< " "
			<< privPasswd;
	}
	out << std::endl;
	
	//str << "trapsess -v 3 -u traptest -a SHA -A mypassword -x AES -X mypassword -l authPriv -e 0x8000000001020304 172.21.116.108" <<endl;

	out << "trapsess"
		<< " -v 3"
		<< " -u "
		<< m_userName;

	if(m_authProtocol != eSapNone)
	{
		if("" == m_authPassword  || (m_secLevel != eSslAuth && m_secLevel != eSslPriv))
		{
			szPasswd = "11111111";
		}
		else
		{
			szPasswd = m_authPassword;			
		}
	
		out << " -a "
			<< AuthProtocolStr(m_authProtocol)
			<< " -A"
			<< szPasswd;
	}

	if(m_privProtocol != eSppNone)
	{
		if("" == m_privPassword || (m_secLevel != eSslPriv))
		{
			szPasswd = "11111111";
		}
		else
		{
			szPasswd = m_privPassword;			
		}
	
		out << " -x "
			<< PrivProtocolStr(m_privProtocol)
			<< " -X"
			<< szPasswd;
	}

	out << " -l "
		<< SecurityLevelStr1(m_secLevel);

	
	if (!ignoreEngineID && m_engineID.length() > 0)
	{
			out	<< " -e "
					<< "0x"
					<< m_engineID;
	}

	//out << std::endl;
}

void CSnmpV3SecurityParams::CleanData()
{
	m_userName = "";
	m_engineID = "";
	m_secLevel = eSslNone;
	m_authProtocol = eSapNone;
	m_privProtocol = eSppNone;
	m_authPassword = "";
	m_authPassword_enc = "";
	m_privPassword = "";
	m_privPassword_enc = "";

}


