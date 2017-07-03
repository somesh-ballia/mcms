// LdapModuleCfg.cpp: implementation of the CLdapModuleCfg class.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "Segment.h"
#include "LdapModuleCfg.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "OpcodesMcmsInternal.h"
#include "ManagerApi.h"
#include "TraceStream.h"

#include <algorithm>

const char* CLdapModuleCfg::ACTIVE_DIRECTORY_CONFIG_FILE_NAME = "Cfg/ActiveDirectoryConfiguration.xml";
#define BoolToStream(stream,bVal) if(bVal) {stream << "yes";}else {stream << "no";}
using namespace std;

extern char* LdapDirTypeToString(int ldapDirType);
extern char* LdapDirPortToString(int ldapDirPort);
extern char* LdapAuthenticationTypeToString(int ldapAuthenticationType);




////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////////////////////////////////////
CLdapModuleCfg::CLdapModuleCfg()
{
	m_bEnableDirServices = FALSE;
	m_dirType = eMsActiveDirectory;
	m_dirNameAddress = "";
	m_dirNameAddressForConnection = "";
	m_dirPort = e389;	//default suits for non-jitc mode
	m_baseDn = "";
	m_authenticationType = eNTLM;
	m_ldapAdministratorRole = "";
	m_ldapAdministratorReadOnlyRole = "";	
	m_ldapAuditorRole = "";
	m_ldapChairpersonRole = "";
	m_ldapOperatorRole = "";
	m_checkAvailabilityBindTimeout = 5;

	 m_isRequestPeerCertificate =0;
	 m_isOCSPEnabled =0;
	 m_ocspGlobalResponderURI ="";
	 m_isAlwaysUseGlobalOCSPResponder=0;
	 m_isUseResponderOcspURI =0;
	 m_isIncompleteRevocation =0;
	 m_isSkipValidateOcspCert =0;
	 m_revocationMethodType =0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CLdapModuleCfg& CLdapModuleCfg::operator = (const CLdapModuleCfg &other)
{
	m_bEnableDirServices = other.m_bEnableDirServices;
	m_dirType = other.m_dirType;
	m_dirNameAddress = other.m_dirNameAddress;
	m_dirNameAddressForConnection = other.m_dirNameAddressForConnection;
	m_dirPort = other.m_dirPort;
	m_baseDn = other.m_baseDn;
	m_authenticationType = other.m_authenticationType;
	m_ldapAdministratorRole = other.m_ldapAdministratorRole;
	m_ldapAdministratorReadOnlyRole = other.m_ldapAdministratorReadOnlyRole;	
	m_ldapAuditorRole = other.m_ldapAuditorRole;
	m_ldapChairpersonRole = other.m_ldapChairpersonRole;
	m_ldapOperatorRole = other.m_ldapOperatorRole;
	m_checkAvailabilityBindTimeout = other.m_checkAvailabilityBindTimeout;

	 m_isRequestPeerCertificate =other.m_isRequestPeerCertificate;;
	 m_isOCSPEnabled =other.m_isOCSPEnabled;
	 m_ocspGlobalResponderURI =other.m_ocspGlobalResponderURI;
	 m_isAlwaysUseGlobalOCSPResponder=other.m_isAlwaysUseGlobalOCSPResponder;
	 m_isUseResponderOcspURI =other.m_isUseResponderOcspURI;
	 m_isIncompleteRevocation =other.m_isIncompleteRevocation;
	 m_isSkipValidateOcspCert =other.m_isSkipValidateOcspCert;
	 m_revocationMethodType =other.m_revocationMethodType;

	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL operator == (const CLdapModuleCfg& left, const CLdapModuleCfg& right)
{
	return(left.m_bEnableDirServices == right.m_bEnableDirServices
		&& left.m_dirType == right.m_dirType
		&& left.m_dirNameAddress == right.m_dirNameAddress
		&& left.m_dirNameAddressForConnection == right.m_dirNameAddressForConnection
		&& left.m_dirPort == right.m_dirPort
		&& left.m_baseDn == right.m_baseDn
		&& left.m_authenticationType == right.m_authenticationType
		&& left.m_ldapAdministratorRole == right.m_ldapAdministratorRole
		&& left.m_ldapAdministratorReadOnlyRole == right.m_ldapAdministratorReadOnlyRole		
		&& left.m_ldapAuditorRole == right.m_ldapAuditorRole
		&& left.m_ldapChairpersonRole == right.m_ldapChairpersonRole
		&& left.m_ldapOperatorRole == right.m_ldapOperatorRole
		&& left.m_checkAvailabilityBindTimeout == right.m_checkAvailabilityBindTimeout
		);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CLdapModuleCfg::SetParams(const CLdapModuleCfg &other)
{
	*this = other;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CLdapModuleCfg::~CLdapModuleCfg()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLdapModuleCfg::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pMainNode;

	if(!pFatherNode)
		return;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("ACTIVE_DIRECTORY_CONFIG_PARAMS");
		pMainNode = pFatherNode;
	}
	else
	{
		pMainNode = pFatherNode->AddChildNode("ACTIVE_DIRECTORY_CONFIG_PARAMS");
	}
	pMainNode->AddChildNode("ENABLE_DIRECTORY_SERVICES",m_bEnableDirServices,_BOOL);
	pMainNode->AddChildNode("DIRECTORY_TYPE",m_dirType, LDAP_DIR_TYPE_ENUM);
	pMainNode->AddChildNode("DIRECTORY_NAME_ADDRESS",m_dirNameAddress);
	pMainNode->AddChildNode("DIRECTORY_PORT",m_dirPort, LDAP_DIR_PORT_ENUM);
	pMainNode->AddChildNode("BASE_DN",m_baseDn);
	pMainNode->AddChildNode("AUTHENTICATION_TYPE",m_authenticationType, LDAP_AUTHENTICATION_TYPE_ENUM);
	pMainNode->AddChildNode("LDAP_ADMINISTRATOR_ROLE",m_ldapAdministratorRole);
	pMainNode->AddChildNode("LDAP_ADMINISTRATOR_READONLY_ROLE",m_ldapAdministratorReadOnlyRole);	
	pMainNode->AddChildNode("LDAP_AUDITOR_ROLE",m_ldapAuditorRole);
	pMainNode->AddChildNode("LDAP_CHAIRPERSON_ROLE",m_ldapChairpersonRole);
	pMainNode->AddChildNode("LDAP_OPERATOR_ROLE",m_ldapOperatorRole);
	pMainNode->AddChildNode("CHECK_AVAILABILITY_BIND_TIMEOUT",m_checkAvailabilityBindTimeout, _0_TO_20_DECIMAL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int CLdapModuleCfg::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pActiveDirCfgNode;
	char* ParentNodeName;
//	BOOL bReadTime=TRUE;

	pActionNode->get_nodeName(&ParentNodeName);
	if(!strcmp(ParentNodeName, "ACTIVE_DIRECTORY_CONFIG_PARAMS"))
	{
		pActiveDirCfgNode = pActionNode;
// 		bReadTime = FALSE;//no need to read time not relevant when loading from file
	}
	else
		GET_MANDATORY_CHILD_NODE(pActionNode, "ACTIVE_DIRECTORY_CONFIG_PARAMS", pActiveDirCfgNode);

	if ( pActiveDirCfgNode )
	{
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"ENABLE_DIRECTORY_SERVICES",&m_bEnableDirServices,_BOOL);
		WORD tmp;
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"DIRECTORY_TYPE",&tmp,LDAP_DIR_TYPE_ENUM);
		m_dirType = (eLdapDirType)tmp;
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"DIRECTORY_NAME_ADDRESS",m_dirNameAddress,_0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"DIRECTORY_PORT",&tmp, LDAP_DIR_PORT_ENUM);
		m_dirPort = (eLdapDirPort)tmp;
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"BASE_DN",m_baseDn,_0_TO_255_DECIMAL);
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"AUTHENTICATION_TYPE",&tmp, LDAP_AUTHENTICATION_TYPE_ENUM);
		m_authenticationType = (eLdapAuthenticationType)tmp;
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"LDAP_ADMINISTRATOR_ROLE",m_ldapAdministratorRole, _0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"LDAP_ADMINISTRATOR_READONLY_ROLE",m_ldapAdministratorReadOnlyRole, _0_TO_H243_NAME_LENGTH);		
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"LDAP_AUDITOR_ROLE",m_ldapAuditorRole, _0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"LDAP_CHAIRPERSON_ROLE",m_ldapChairpersonRole, _0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pActiveDirCfgNode,"LDAP_OPERATOR_ROLE",m_ldapOperatorRole, _0_TO_H243_NAME_LENGTH);

		m_ldapAdministratorRole = inplace_trim(m_ldapAdministratorRole);
		m_ldapAdministratorReadOnlyRole = inplace_trim(m_ldapAdministratorReadOnlyRole);
		m_ldapAuditorRole			   = inplace_trim(m_ldapAuditorRole);
		m_ldapChairpersonRole		   = inplace_trim(m_ldapChairpersonRole);
		m_ldapOperatorRole			   = inplace_trim(m_ldapOperatorRole);
//		GET_(pActiveDirCfgNode,"CHECK_AVAILABILITY_BIND_TIMEOUT",&m_checkAvailabilityBindTimeout);
		pActiveDirCfgNode->getChildNodeDecValueByName("CHECK_AVAILABILITY_BIND_TIMEOUT",&m_checkAvailabilityBindTimeout);
	}

	return nStatus;
}
/*////////////////////////////////////////////////////////////////////////////////////////////////////
void CLdapModuleCfg::Serialize(CSegment& rSegment) const
{
	rSegment << m_bEnableDirServices
			 //<< m_dirType
			 << m_dirNameAddress
			 //<< m_dirPort
			 << m_baseDn
			 //<< m_authenticationType
			 << m_ldapAdministratorRole
			 << m_ldapAuditorRole
			 << m_ldapChairpersonRole
			 << m_ldapOperatorRole;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CLdapModuleCfg::DeSerialize(CSegment& rSegment)
{
	rSegment >> m_bEnableDirServices
			 //>> m_dirType
			 >> m_dirNameAddress
			 //>> m_dirPort
			 >> m_baseDn
			 //>> m_authenticationType
			 >> m_ldapAdministratorRole
			 >> m_ldapAuditorRole
			 >> m_ldapChairpersonRole
			 >> m_ldapOperatorRole;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
std::string CLdapModuleCfg::Dump() const
{
	
	char* dirType = LdapDirTypeToString(m_dirType);
	if(dirType == NULL)
		dirType = "NULL";
	
	char* dirPort = LdapDirPortToString(m_dirPort);
	if(dirPort == NULL)
		dirPort = "NULL";
	
	char* dirAuthentication = LdapAuthenticationTypeToString(m_authenticationType);
	if(dirAuthentication == NULL)
		dirAuthentication = "NULL";

	std::string str;
	str.clear();
	str.append("\nCLdapModuleCfg::Dump\n--------------------------------------------------");
	str.append("\n\tEnable Directory Services  = "); str.append((m_bEnableDirServices) ? "Yes" : "No");
	str.append("\n\tDirectory Type 			   = ");
	str += dirType;
	str.append("\n\tDirectory Name or Address  = " + m_dirNameAddress);
	str.append("\n\tDirectory Port             = ");
	str += dirPort;
	str.append("\n\tBase DN                    = " + m_baseDn);
	str.append("\n\tAuthentication Type        = ");
	str += dirAuthentication;
	str.append("\n\tAuthorization to Group Mapping:");
	str.append("\n\t\tAdministrator            = " + m_ldapAdministratorRole);
	str.append("\n\t\tAdministrator Read Only           = " + m_ldapAdministratorReadOnlyRole);	
	str.append("\n\t\tOperator                 = " + m_ldapOperatorRole);
	str.append("\n\t\tChairperson              = " + m_ldapChairpersonRole);
	str.append("\n\t\tAuditor                  = " + m_ldapAuditorRole);
	str.append("\n--------------------------------------------------\n");

	return str;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CLdapModuleCfg::ValidateConfiguration()
{
	if ( FALSE == GetEnableDirServices() )
		return STATUS_OK;

	BOOL bIsJitcMode = IsJitcMode();

	if ( GetDirType() != eMsActiveDirectory )
		return STATUS_DIRECTORY_TYPE_INCORRECT;

	if ( strlen(GetDirNameAddress()) < 1 )
		return STATUS_DIRECTORY_NAME_ADDRESS_EMPTY;

	if ( (GetDirPort() != e389 )&& (GetDirPort() != e636))
		return STATUS_DIRECTORY_PORT_INCORRECT;

	char* baseDn = (char*)(GetBaseDn());
	if (ValidateBaseDn(baseDn) == STATUS_BASE_DN_INCORRECT)
		return STATUS_BASE_DN_INCORRECT;

	if ((( GetAuthenticationType() != ePlain ) && ( GetAuthenticationType() != eNTLM ))
		|| (( bIsJitcMode ) && ( GetAuthenticationType() == ePlain )))
		return STATUS_AUTHENTICATION_TYPE_INCORRECT;

	if (( strlen(GetLdapAdministratorRole()) < 1 ) && ( strlen(GetLdapOperatorRole()) < 1 ))
	{
		if (( bIsJitcMode ) ||
			(( !bIsJitcMode ) && (( strlen(GetLdapChairpersonRole()) < 1 ) && ( strlen(GetLdapAuditorRole()) < 1 ) && ( strlen(GetLdapAdministratorReadOnlyRole()) < 1 ))))
			return STATUS_ALL_DIR_AUTHORIZATION_GROUPS_ARE_EMPTY;
	}

	// if seems as IPv6 address, add []
	UpdtaeDirNameAddressForConnection();

	return STATUS_OK;
}

STATUS CLdapModuleCfg::ValidateBaseDn(string sBaseDn)
{
	FPTRACE2(eLevelInfoNormal,"LdapModuleCfg::ValidateBaseDn - sBaseDn: " , sBaseDn.c_str());
	const char * baseDn = sBaseDn.c_str();
	if ( strlen(baseDn) < 1 )
			return STATUS_BASE_DN_EMPTY;
		if ( strlen(baseDn) >= 1 )
		{
			char tmpBaseDn[512];
		  strncpy(tmpBaseDn, baseDn, sizeof(tmpBaseDn)-1);
		  tmpBaseDn[sizeof(tmpBaseDn)-1] = '\0';
			// check the validity of the Base DN configured
			char* tmpStr = strtok(tmpBaseDn, "=");
			// check that tmpStr contains only alphanumeric characters
			if (!IsStringAlphaNumeric(tmpStr))
				return STATUS_BASE_DN_INCORRECT;
			int cntr = 1;
			while (tmpStr != NULL)
			{
				if (cntr % 2 == 1)
				{
					tmpStr = (strtok(NULL, ","));
					if (tmpStr == NULL)
						return STATUS_BASE_DN_INCORRECT;
					// check that tmpStr contains only alphanumeric characters
					if (!IsStringAlphaNumeric(tmpStr))
						return STATUS_BASE_DN_INCORRECT;
				}
				else
				{
					tmpStr = (strtok(NULL, "="));
					if (tmpStr != NULL)
					{
						// check that tmpStr contains only alphanumeric characters
						if (!IsStringAlphaNumeric(tmpStr))
							return STATUS_BASE_DN_INCORRECT;
					}
				}

				cntr++;
			}
		}
		return STATUS_OK;

}

//////////////////////////////////////////////////////////////////////
BOOL CLdapModuleCfg::IsStringAlphaNumeric(char *str)
{
	if (str == NULL)
		return FALSE;
	int stringlen = strlen(str);
	for (int i=0; i < stringlen; i++)
		if (((str[i] < 48) ||
			((str[i] > 57) && (str[i] < 65)) ||
			((str[i] > 90) && (str[i] < 97)) ||
			(str[i] > 122)) &&
			(str[i] != 9) && (str[i] != 32) && (str[i] != 38)&&	// space/tab/ & character			
			(str[i] != 45) && (str[i] != 46) && (str[i] != 95))		// dash/period/underscore
			return FALSE;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
BOOL CLdapModuleCfg::IsJitcMode()
{
	BYTE bJitcMode = FALSE;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

	return bJitcMode;
}


//////////////////////////////////////////////////////////////////////
void CLdapModuleCfg::UpdtaeDirNameAddressForConnection()
{
	m_dirNameAddressForConnection.clear();

	BOOL bIpv6Address = FALSE;
	BOOL bContainsSqrBrackets = FALSE;
	std::string dirNameAddress = GetDirNameAddress();

	size_t found;
	int cnt = 0;
	found = dirNameAddress.find(':');
	if (found != string::npos)
	{
		for (cnt = 1; cnt < 7; cnt++)
		{
			found = dirNameAddress.find(':', found+1);
			if (found == string::npos)
				break;
		}
	}
	if (7 == cnt)
		bIpv6Address = TRUE;

	if (('[' == dirNameAddress[0]) && ((']' == dirNameAddress[dirNameAddress.size()-1])))
		bContainsSqrBrackets = TRUE;

	// if this seems as an IPv6 address && if 1st and last characters are NOT []
	if ((bIpv6Address) && (!bContainsSqrBrackets))
	{
		// Add Square brackets
		m_dirNameAddressForConnection.append("[" + dirNameAddress + "]");
	}
	else
	{
		m_dirNameAddressForConnection = dirNameAddress;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CLdapModuleCfg::CheckOperatorList()
{
	CManagerApi authenticationMngrApi(eProcessAuthentication);
	CSegment rspMsg;
	OPCODE resOpcode = CHECK_USER_LIST_FOR_LDAP_IND;
	STATUS responseStatus  = authenticationMngrApi.SendMessageSync(NULL, CHECK_USER_LIST_FOR_LDAP_REQ, 2*SECOND, resOpcode, rspMsg);
	STATUS operatorListStatus = STATUS_FAIL;
	if (responseStatus == STATUS_OK)
		rspMsg >> operatorListStatus;

	return operatorListStatus;
}


/////////////////////////////////////////////////////////////////////////////
//    GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

static CLdapModuleCfg* g_pLdapModuleCfg = NULL;

CLdapModuleCfg* GetLdapModuleCfg()
{
	return g_pLdapModuleCfg;
}

/////////////////////////////////////////////////////////////////////////////

void SetLdapModuleCfg(CLdapModuleCfg* pCfg)
{
	g_pLdapModuleCfg = pCfg;

}
/////////////////////////////////////////////////////////////////////////////
void CLdapModuleCfg::DeSerializePki(CSegment* pMsg)
{
	*pMsg >> m_isRequestPeerCertificate;
	*pMsg >> m_isOCSPEnabled;
	*pMsg >> m_ocspGlobalResponderURI;
	*pMsg >> m_isAlwaysUseGlobalOCSPResponder;
	*pMsg >> m_isUseResponderOcspURI;
	*pMsg >> m_isIncompleteRevocation;
	*pMsg >> m_isSkipValidateOcspCert;
	*pMsg >>  m_revocationMethodType;
}

void CLdapModuleCfg::SerializePki(CSegment* pMsg)
{
	*pMsg << m_isRequestPeerCertificate;
	*pMsg << m_isOCSPEnabled;
	*pMsg << m_ocspGlobalResponderURI;
	*pMsg << m_isAlwaysUseGlobalOCSPResponder;
	*pMsg << m_isUseResponderOcspURI;
	*pMsg << m_isIncompleteRevocation;
	*pMsg << m_isSkipValidateOcspCert;
	*pMsg << m_revocationMethodType;
}

void CLdapModuleCfg::DumpPKI(COstrStream& msg)
{

	msg << "-----Ldap PKI Setting -------";
	msg << "\n m_isRequestPeerCertificate = ";BoolToStream(msg,m_isRequestPeerCertificate);
	msg << "\n m_ocspGlobalResponderURI = " << m_ocspGlobalResponderURI;
	msg << "\n m_isUseResponderOcspURI = ";BoolToStream(msg,m_isUseResponderOcspURI);
	msg << "\n m_isIncompleteRevocation = ";BoolToStream(msg,m_isIncompleteRevocation);
	msg << "\n m_isSkipValidateOcspCert = ";BoolToStream(msg,m_isSkipValidateOcspCert);
	msg << "\n m_revocationMethodType   = " ;
	switch(m_revocationMethodType)
	{
		case eNoneMethod:   msg << "None";
							break;
		case eCrl:           msg << "Crl";
							break;
		case eOcsp:			msg << "Ocsp";
							break;
	}
	msg << "\n -----END PKI Setting -------";
}
