#ifndef SNMPCOMMUNITYPERMISSION_H_
#define SNMPCOMMUNITYPERMISSION_H_

#include "SerializeObject.h"
#include "psosxml.h"
#include "SnmpV3SecurityParams.h"

class CSnmpCommunityPermission : public CSerializeObject
{
CLASS_TYPE_1(CSnmpCommunityPermission,CSerializeObject)
    public:
    //Constructors
    CSnmpCommunityPermission(); 
    CSnmpCommunityPermission(const string& communityName, WORD permission);
    CSnmpCommunityPermission(const CSnmpCommunityPermission& other);
    virtual ~CSnmpCommunityPermission();
    void InitDefaults();
    virtual CSerializeObject* Clone(void);

    //Operators
    CSnmpCommunityPermission& operator=(const CSnmpCommunityPermission& other);
    
    //Operations
    const char* NameOf() const;
    
    WORD  GetPermission() const;
    const string& GetCommunityName(void) const;
    const CSnmpV3SecurityParams& GetSnmpV3Param(void) const;
    
    virtual void SerializeXml(CXMLDOMElement*& pParentNode) const;
    virtual void SerializeXml(CXMLDOMElement*& pParentNode,bool isToEma) const;
    
    virtual int DeSerializeXml(CXMLDOMElement* pCommunityPermissionNode,
                               char *pszError,
                               const char* action);

    
    void CleanSecurityData() { m_snmpV3Params.CleanData(); } ;


private:

    void Init();

    string m_communityName; //Community Name string
    DWORD  m_permission;    //NO PERMISSION, READ ONLY,READ WRITE,WRITE CREATE
    CSnmpV3SecurityParams m_snmpV3Params;  // user credentials

    friend class CSnmpData;

};

#endif /*SNMPCOMMUNITYPERMISSION_H_*/
