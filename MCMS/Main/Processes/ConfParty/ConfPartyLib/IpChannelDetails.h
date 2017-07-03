// IpChannelDetails.h

#if !defined(_IpChannelDetails_H__)
#define _IpChannelDetails_H__

#include "PObject.h"
#include "ConfPartyH323Defines.h"
#include "ConfPartyDefines.h"
#include "IceCmInd.h"
#include "SipConfPartyDefinitions.h"

class CXMLDOMElement;

class CIpChannelDetails : public CPObject
{
CLASS_TYPE_1(CIpChannelDetails, CPObject)
public:
  CIpChannelDetails();
  CIpChannelDetails(const CIpChannelDetails &other);
	virtual const char* NameOf() const { return "CIpChannelDetails";}
	CIpChannelDetails& operator = (const CIpChannelDetails& other);
  virtual ~CIpChannelDetails();
  void   Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);
  void   DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	void   SerializeXml(CXMLDOMElement* pParentNode);
	int	   DeSerializeXml(CXMLDOMElement* pChannelNode,char *pszError);
  void   SetChannelType(const EIpChannelType channelType);
  EIpChannelType   GetChannelType() const;
	void   SetConnectionStatus(const BYTE connectionStatus);                 
  WORD   IsConnectedStatus() const;
	void   SetActualRate(const DWORD actualRate);                 
  DWORD  GetActualRate() const;
	void   SetPartyAddrPort(mcTransportAddress* ip_address);                 
  const  mcTransportAddress*  GetPartyAddrPort() const;
	void   SetMcuAddrPort(mcTransportAddress* ip_address);                 
  const  mcTransportAddress*  GetMcuAddrPort() const;
	void   SetPacketsCounterIn(const DWORD packetsCounterIn);                 
  DWORD  GetPacketsCounterIn() const;
	void   SetPacketsCounterUse(const DWORD packetsCounterUse);                 
  DWORD  GetPacketsCounterUse() const;
	void   SetFrameRate(const WORD frameRate);                 
  WORD   GetFrameRate() const;
	void   SetVideoResolution(const int videoResolution);
	int    GetVideoResolution() const;

	void   SetIsIce(BYTE IsIce) {m_IsIce = IsIce;}
	void   SetIcePartyAddrPort(mcTransportAddress* ip_address);
	const  mcTransportAddress*  GetIcePartyAddrPort() const;
	void   SetIceMcuAddrPort(mcTransportAddress* ip_address);
	const  mcTransportAddress*  GetIceMcuAddrPort() const;
	void   SetIceConnectionType(EIceConnectionType ConnectionType);
	EIceConnectionType GetIceConnectionType() const;

	EIpChannelType m_channelType;
	BYTE		m_connectionStatus; //YES/NO

	//API - 51     :  WORD to DWORD for m_actualRate
	DWORD		m_actualRate; 

	mcTransportAddress  m_partyAddrPort; 
	mcTransportAddress  m_mcuAddrPort; 
	DWORD   m_packetsCounterIn;
	DWORD   m_packetsCounterUse;

	//API - 50
  WORD	m_frameRate;

	//API532
	int		m_videoResolution;

	mcTransportAddress  m_IcePartyAddrPort;
	mcTransportAddress  m_IceMcuAddrPort;

	EIceConnectionType m_IceConnectionType;
	BYTE m_IsIce;
};

#endif //_IpChannelDetails_H__
