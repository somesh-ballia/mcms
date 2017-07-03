// LdapModuleCfg.h: interface for the CLdapModuleCfg class.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __LdapModuleCfg_H__
#define __LdapModuleCfg_H__


#include "psosxml.h"
#include "Request.h"
#include "SerializeObject.h"
#include "DefinesGeneral.h"



class CLdapModuleCfg : public CSerializeObject
{
CLASS_TYPE_1(CLdapModuleCfg, CPObject)
public:

	//Constructors
	CLdapModuleCfg();
	CLdapModuleCfg(const CLdapModuleCfg &other);
	CLdapModuleCfg& operator = (const CLdapModuleCfg& other);
	virtual ~CLdapModuleCfg();

	friend BOOL operator == (const CLdapModuleCfg& left ,const CLdapModuleCfg& right);

	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	virtual CSerializeObject* Clone() {return new CLdapModuleCfg;}
/*	void Serialize(CSegment& rSegment) const;
	void DeSerialize(CSegment& seg);
*/
	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(ACTIVE_DIRECTORY_CONFIG_FILE_NAME); }
	void WriteXmlFile() { CSerializeObject::WriteXmlFile(ACTIVE_DIRECTORY_CONFIG_FILE_NAME,"ACTIVE_DIRECTORY_CONFIGURATION"); }

	void SetParams(const CLdapModuleCfg &other);

	BOOL GetEnableDirServices() const { return m_bEnableDirServices; }
	eLdapDirType GetDirType() const { return m_dirType; }
	const char* GetDirNameAddress() const { return m_dirNameAddress.c_str(); }
	void SetDirNameAddress(string dirNameAddress) { m_dirNameAddress = dirNameAddress; }
	const char* GetDirNameAddressForConnection() const { return m_dirNameAddressForConnection.c_str(); }
	void SetDirNameAddressForConnection(string dirNameAddressForConnection) { m_dirNameAddressForConnection = dirNameAddressForConnection; }
	eLdapDirPort GetDirPort() const { return m_dirPort; }
	const char* GetBaseDn() const { return m_baseDn.c_str(); }
	eLdapAuthenticationType GetAuthenticationType() const { return m_authenticationType; }
	const char* GetLdapAdministratorRole() const { return m_ldapAdministratorRole.c_str(); }
	const char* GetLdapAuditorRole() const { return m_ldapAuditorRole.c_str(); }
	const char* GetLdapAdministratorReadOnlyRole() const { return m_ldapAdministratorReadOnlyRole.c_str(); }	
	const char* GetLdapChairpersonRole() const { return m_ldapChairpersonRole.c_str(); }
	const char* GetLdapOperatorRole() const { return m_ldapOperatorRole.c_str(); }
	int GetCheckAvailabilityBindTimeout() const { return m_checkAvailabilityBindTimeout; }
	static STATUS ValidateBaseDn(string baseDn);
	const char * NameOf() const { return "CLdapModuleCfg"; }

	std::string Dump() const;
	STATUS ValidateConfiguration();
	static BOOL IsStringAlphaNumeric(char *str);
	BOOL IsJitcMode();
	void UpdtaeDirNameAddressForConnection();
	STATUS CheckOperatorList();

	static const char* ACTIVE_DIRECTORY_CONFIG_FILE_NAME;
	void DeSerializePki(CSegment* pMsg);
	void SerializePki(CSegment* pMsg);
	BOOL 	IsRequestPeerCertificate() {return m_isRequestPeerCertificate;};
	std::string	GetOCSPGlobalResponderURI(){return m_ocspGlobalResponderURI;};
	BOOL    IsUseResponderOcspUri(){return m_isUseResponderOcspURI;};
	BOOL    IsIncompleteRevocation(){return m_isIncompleteRevocation;};
	BOOL    IsSkipValidationOcspCert(){return m_isSkipValidateOcspCert;};
	eRevocationMethod getRevocationMethodType(){return (eRevocationMethod)m_revocationMethodType;};
	void DumpPKI(COstrStream& msg);
protected:
	BOOL m_bEnableDirServices;
	eLdapDirType m_dirType;
	std::string m_dirNameAddress;
	std::string m_dirNameAddressForConnection;
	eLdapDirPort m_dirPort;
    std::string m_baseDn;
    eLdapAuthenticationType m_authenticationType;
	std::string m_ldapAdministratorRole;
	std::string m_ldapAdministratorReadOnlyRole;	
	std::string m_ldapAuditorRole;
	std::string m_ldapChairpersonRole;
	std::string m_ldapOperatorRole;
	int m_checkAvailabilityBindTimeout;

	BYTE m_isRequestPeerCertificate;
	BYTE m_isOCSPEnabled;
	std::string m_ocspGlobalResponderURI;
	BYTE m_isAlwaysUseGlobalOCSPResponder;
	BYTE m_isUseResponderOcspURI;
	BYTE m_isIncompleteRevocation;
	BYTE m_isSkipValidateOcspCert;
	BYTE  m_revocationMethodType;
};


inline std::string & inplace_ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

inline std::string & inplace_rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

inline std::string & inplace_trim(std::string &s)
{
    return inplace_ltrim(inplace_rtrim(s));
}


#endif /*__LdapModuleCfg_H__*/


