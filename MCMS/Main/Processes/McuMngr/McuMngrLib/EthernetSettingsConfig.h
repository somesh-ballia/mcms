// EthernetSettingsConfig.h: interface for the CEthernetSettingsConfig class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ETHERNETSETTINGS_H_
#define ETHERNETSETTINGS_H_


#include "PObject.h"
#include "psosxml.h"
#include "CommonStructs.h"
#include "McuMngrDefines.h"
#include "SerializeObject.h"

#define ETHERNET_SETTINGS_MULT_SERV_PATH	((std::string)(MCU_MCMS_DIR+"/Cfg/EthernetSettingsMultipleServices.xml"))
#define ETHERNET_SETTINGS_PATH	((std::string)(MCU_MCMS_DIR+"/Cfg/EthernetSettings.xml"))


#define NAME_LEN_802_1x             256


//////////////////////////////
class CEthernetSettingsConfig : public CSerializeObject
{
CLASS_TYPE_1(CEthernetSettingsConfig, CSerializeObject)

public:
	CEthernetSettingsConfig ();
	CEthernetSettingsConfig (const CEthernetSettingsConfig& other);
	virtual ~CEthernetSettingsConfig ();
	const char* NameOf() const {return "CEthernetSettingsConfig";}
	CEthernetSettingsConfig& operator = (const CEthernetSettingsConfig &rOther);
	virtual CSerializeObject* Clone(){return new CEthernetSettingsConfig;}

	virtual void SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual void SerializeXmlForEma(CXMLDOMElement*& pParentNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	friend bool operator==(const CEthernetSettingsConfig& first, const CEthernetSettingsConfig& second);    
	friend bool operator==(const CEthernetSettingsConfig& theObject, const ETH_SETTINGS_PORT_DESC_S& theStruct);    
	
	void InitMembers();
	virtual void Dump(std::ostream& msg) const;

	DWORD				GetSlotId() const;
	void					SetSlotId(const DWORD slotId);

	DWORD				GetPortId() const;
	void					SetPortId(const DWORD portId);

	eEthPortType		GetPortType() const;
	void					SetPortType(const eEthPortType portType);
	
	void SetIsFromEma(const bool isFromEma);

	eEthMediaPortType		GetMediaPortType() const;
	void					SetMediaPortType(const eEthMediaPortType mediaPortType);


	eEth802_1xAuthenticationType	Get802_1xAuthenticationProtocol() const;
	void                            Set802_1xAuthenticationProtocol(const eEth802_1xAuthenticationType authenticationType);

	ePortSpeedType	GetPortSpeed() const;
	void					SetPortSpeed(const ePortSpeedType portSpeed);

	const char* Get802_1xUserName() const;
	const char* Get802_1xUserPassword() const;
	//const char* Get802_1xUserPassword_enc()  { return m_802_1xPassword_enc ; }

	void Set802_1xUserName( const char* user);
	void Set802_1xUserPassword( const char* password);

    
protected:
	DWORD					              m_slotId;
	DWORD					              m_portId;
	eEthPortType			              m_portType;
	eEthMediaPortType			          m_mediaPortType;
	ePortSpeedType			              m_portSpeed;

	eEth802_1xAuthenticationType          m_802_1xAuthenticationProtocol;
	char                                  m_802_1xUserName[NAME_LEN_802_1x];
	std::string                           m_802_1xPassword;
	std::string                           m_802_1xPassword_enc;
	mutable bool                          m_IsFromEma;


};



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//					CEthernetSettingsConfigList
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

class CEthernetSettingsConfigList : public CSerializeObject
{
CLASS_TYPE_1(CEthernetSettingsConfigList, CSerializeObject)

public:
	CEthernetSettingsConfigList ();
	CEthernetSettingsConfigList (const CEthernetSettingsConfigList& other);
	virtual ~CEthernetSettingsConfigList ();
	const char* NameOf() const {return "CEthernetSettingsConfigList";}
	CEthernetSettingsConfigList& operator = (const CEthernetSettingsConfigList &rOther);
	virtual CSerializeObject* Clone(){return new CEthernetSettingsConfigList;}

	virtual void SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	virtual void SerializeXmlForEma(CXMLDOMElement*& pParentNode) const;
	friend bool operator==(const CEthernetSettingsConfigList& first,const CEthernetSettingsConfigList& second);    
	
	void InitMembers();
	virtual void Dump(std::ostream& msg) const;
	void SetIsFromEma(bool isForEma);

	CEthernetSettingsConfig* GetSpecEthernetSettingsConfig(int idx) const;
	void SetSpecEthernetSettingsConfig(const CEthernetSettingsConfig &ethernetSettings);
	

	eEthPortType GetSpecPortSpeed(int idx);
	
protected:
	CEthernetSettingsConfig* m_pEthernetSettingsConfigList[MAX_NUM_OF_LAN_PORTS];
};




#endif /*ETHERNETSETTINGS_H_*/
