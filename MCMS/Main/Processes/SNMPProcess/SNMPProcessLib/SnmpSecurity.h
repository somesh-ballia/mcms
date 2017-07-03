#ifndef SNMPSECURITY_H_
#define SNMPSECURITY_H_

#include <vector>
#include <list>
#include "SNMPDefines.h"
#include "SnmpCommunityPermission.h"
#include "SerializeObject.h"

class CSnmpTrapCommunity;
class CSnmpV3SecurityParams;

class CSnmpSecurity : public CSerializeObject
{
CLASS_TYPE_1(CSnmpSecurity,CSerializeObject)
public:

    //Constructors
    
    CSnmpSecurity();                                                                            
    CSnmpSecurity(const CSnmpSecurity &other);
    CSnmpSecurity& operator = (const CSnmpSecurity& other);
    virtual ~CSnmpSecurity();

    void InitDefaults();

    virtual CSerializeObject* Clone() {return new CSnmpSecurity();}  
    virtual void   SerializeXml(CXMLDOMElement*& pParentNode) const;
    virtual void   SerializeXml(CXMLDOMElement*& pParentNode,bool isToEma) const;
    virtual int    DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);
    
    const char*  NameOf() const;                
    
    
    WORD   SendAuthenticationTraps() const;             //Get - Is a Trap sent on Autentcation Failure
    void   SetSendAuthenticationTraps(const WORD yesNo);//Set - Is a Trap sent on Autentcation Failure
    
    WORD   AcceptAllRequests() const;                   //Get - Should requests from all IPs be treated?
    void   SetAcceptAllRequests(const WORD yesNo);      //Set - Should requests from all IPs be treated?
    const CSnmpCommunityPermission&   GetCommunityPermission()const;
    eSnmpVersionTrap GetSnmpTrapVersion() const {return m_snmpTrapVersion;}
    void SetSnmpTrapVersion(eSnmpVersionTrap snmpTrapVersion) { m_snmpTrapVersion = snmpTrapVersion;}

    void CommunityCfg(ostream & str) const;
    void TrapCfg(ostream & str) const;
    const CSnmpV3SecurityParams& GetSnmpV3Param(void) const;

    virtual void UnSetIsFromEma() const;
    void CleanSecurityData() { m_community.CleanSecurityData(); } ;

    const vector<CSnmpTrapCommunity>& GetTrapDestinations() const { return m_trapDestinations; };

    void DeleteTraps(list<CSnmpTrapCommunity>& trapList);

private:

    void Init();


    eSnmpVersionTrap m_snmpTrapVersion;
    WORD  m_sendAuthenticationTraps; //Send trap on authentication error
    WORD  m_acceptAllRequests;       //Accept All request regardless of source
    
    vector<CSnmpTrapCommunity> m_trapDestinations;
    vector<DWORD> m_requestSources;
    CSnmpCommunityPermission m_community;
    mutable bool		m_isFromEma;

    friend class CSnmpData;

};

#endif /*SNMPSECURITY_H_*/
