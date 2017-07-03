#ifndef SNMPDATA_H_
#define SNMPDATA_H_

#include "SerializeObject.h"
#include "StringsLen.h"
#include "SnmpSecurity.h"
#include "SnmpTrapCommunity.h"
#include "SNMPDefines.h"

class CSnmpData : public CSerializeObject
{
CLASS_TYPE_1(CSnmpData,CSerializeObject)
public:
		const string & GetSystemName() const;
		void SetSystemName(const string & name);
		const string &  GetContactName() const;
		void SetContactName(const string &   name);
		const string &  GetLocation() const;
		void SetLocation(const string & location);

		const string &  GetEngineID() const;


		DWORD  GetMngmntIp() const;

		void  SetMngmntIp(DWORD mngmntIp);

	   //Constructors
	CSnmpData() ;                                                                            
	CSnmpData(const CSnmpData &other) ;
	CSnmpData& operator = (const CSnmpData& other);
	virtual ~CSnmpData();

	void InitDefaults();
		
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	virtual CSerializeObject* Clone() {return new CSnmpData();}

		
	const char*  NameOf() const;                
	
	 const CSnmpSecurity & GetSecurityInfo() const;            //Returns Security information 
	 int TestValidity() const;                    //Test validity of snmp info


	  void WriteSNMPXmlFile() const;
      WORD Is_enable_snmp()const;
      void Set_enable_snmp(const WORD yesNo);
      void SetVersion(eSnmpVersion ver);
      eSnmpVersion GetVersion(void) const;
      void SetIsForEMA(BYTE yesNo);
      BYTE GetIsForEMA();
      void UnSetIsFromEma() const;

      void DeleteTraps(list<CSnmpTrapCommunity>& trapList)
      {
    	  m_Security.DeleteTraps(trapList);
      }

private:

  	void Init();

    // Attributes
	CSnmpSecurity m_Security;                 //Snmp Security Information
	string m_location;
	string m_contactName;
	string m_systemName;
	WORD  m_enable_snmp;
	eSnmpVersion m_version;

	std::string       m_engineID;
	DWORD     m_mngmntIp;

	BYTE m_bIsForEMA;

	eProductType 	m_curProductType;

};

#endif /*SNMPDATA_H_*/
