#ifndef SNMPTRAPCOMMUNITY_H_
#define SNMPTRAPCOMMUNITY_H_

#include "SerializeObject.h"
#include "SnmpV3SecurityParams.h"

class COstrStream;
class CIstrStream;

class CSnmpTrapCommunity : public CSerializeObject
{
CLASS_TYPE_1(CSnmpTrapCommunity,CSerializeObject)
public:
    //Constructors
    CSnmpTrapCommunity(); 
    CSnmpTrapCommunity(const string generalIpAddr, const string & communityName, BOOL trapInformEnabled);
    CSnmpTrapCommunity(const CSnmpTrapCommunity &other);
    virtual ~CSnmpTrapCommunity();
    virtual CSerializeObject* Clone();

    //Operators
    CSnmpTrapCommunity& operator= (const CSnmpTrapCommunity& other);
    bool operator==(const CSnmpTrapCommunity& other);

    const char*  NameOf() const; 
  
    const  string & GetCommunityName() const;
    const CSnmpV3SecurityParams& GetSnmpV3Param(void) const;
    eSnmpVersionTrap GetSnmpTrapVersionForConfig() const {return m_snmpTrapVersionForConfig;}

    string GetGeneralTrapDestination() const;
    BOOL	GetTrapInformEnabled() const;


    virtual void   SerializeXml(CXMLDOMElement*& pParentNode)const;
    virtual void   SerializeXml(CXMLDOMElement*& pParentNode,bool isToEma)const;
    virtual int    DeSerializeXml(CXMLDOMElement* pTrapCommunityNode,char *pszError,const char* action);

    // Attributes
private:

    eSnmpVersionTrap m_snmpTrapVersionForConfig;
    string  m_generalTrapDestination; //IP address of manager

    string  m_communityName;   //Community Name string
    CSnmpV3SecurityParams m_snmpV3Params;  // user credentials
    BYTE	m_trapInformEnabled;

 };

#endif /*SNMPTRAPCOMMUNITY_H_*/
