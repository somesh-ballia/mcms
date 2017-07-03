/*
 * SnmpV3SecurityParams.h
 */

#ifndef SNMPV3SECURITYPARAMS_H_
#define SNMPV3SECURITYPARAMS_H_

#include <string>
#include <iostream>
#include "SerializeObject.h"
#include "SNMPDefines.h"

class CXMLDOMElement;

class CSnmpV3SecurityParams : public CSerializeObject
{
CLASS_TYPE_1(CSnmpV3SecurityParams, CSerializeObject)
public:
	CSnmpV3SecurityParams(void);
    CSnmpV3SecurityParams(const CSnmpV3SecurityParams& rhs);
    CSnmpV3SecurityParams& operator=(const CSnmpV3SecurityParams& rhs);
    bool operator==(const CSnmpV3SecurityParams& other);

    ~CSnmpV3SecurityParams(void);

    void IniDefaults();

    const char* NameOf(void) const;
    CSerializeObject* Clone(void);

    void SerializeXml(CXMLDOMElement*& root) const;
    void SerializeXml(CXMLDOMElement*& root,bool isToEma) const;
    int DeSerializeXml(CXMLDOMElement* root,
                       char* pszError,
                       const char* action);
    void UserConfig(std::ostream& out) const;
    void TrapConfig(std::ostream& out, BOOL ignoreEngineID) const;

    void CleanData();

   const string& GetUserName() const { return m_userName; }
   
   const string& GetEngineID() const { return m_engineID; }
   
   

    void UnSetIsFromEma() const { m_isFromEma = false ; }

    //void EncryptPassword(std::string  srcPass , std::string& dstPass);
    //void DecryptPassword(std::string  srcPass , std::string& dstPass);


   eSnmpSecurityLevel GetSecLevel() const  { return m_secLevel; }

    eSnmpAuthProtocol GetAuthProtocol() const  { return m_authProtocol; }

    eSnmpPrivProtocol GetPrivProtocol() const  { return m_privProtocol; }

    void SetAuthPassword(const std::string & authPassword);
    void SetPrivPassword(const std::string & privPassword);

    std::string GetAuthPassword() const;
    std::string GetPrivPassword() const;

    void SetUserName(const std::string & userName);

private:
    std::string       m_userName;
    std::string       m_engineID;	
	eSnmpSecurityLevel m_secLevel;
    eSnmpAuthProtocol m_authProtocol;
    std::string   	  m_authPassword;
    std::string   	  m_authPassword_enc;
    eSnmpPrivProtocol m_privProtocol;
    std::string       m_privPassword;
    std::string       m_privPassword_enc;

    mutable bool		m_isFromEma;

};

#endif /* SNMPV3SECURITYPARAMS_H_*/
