// IpChannelDetails.cpp

#include <ostream>
#include <istream>

#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "IpChannelDetails.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"

CIpChannelDetails::CIpChannelDetails()
{
	m_channelType = H225;
	m_connectionStatus = NO;
	m_actualRate = 0;
	// IpV6 - Monitoring
	memset(&m_partyAddrPort,0,sizeof(mcTransportAddress));
	memset(&m_mcuAddrPort,0,sizeof(mcTransportAddress))	;
	m_packetsCounterIn = 0;
	m_packetsCounterUse = 0;
	m_frameRate = 0;
	m_videoResolution = -1;

	m_IsIce = 0;

	memset(&m_IcePartyAddrPort,0,sizeof(mcTransportAddress));
	memset(&m_IceMcuAddrPort,0,sizeof(mcTransportAddress))	;
	m_IceConnectionType = kNone;
}

CIpChannelDetails::CIpChannelDetails(const CIpChannelDetails &other)
:CPObject(other)
{
	*this = other;
}

CIpChannelDetails& CIpChannelDetails::operator = (const CIpChannelDetails& other)
{
	m_channelType = other.m_channelType;
	m_connectionStatus = other.m_connectionStatus;
	m_actualRate = other.m_actualRate;
	memset(&m_partyAddrPort,0,sizeof(mcTransportAddress));
	memcpy(&(m_partyAddrPort),&(other.m_partyAddrPort), sizeof(mcTransportAddress));	
	memset(&m_mcuAddrPort,0,sizeof(mcTransportAddress));
	memcpy(&(m_mcuAddrPort),&(other.m_mcuAddrPort), sizeof(mcTransportAddress));	
	m_packetsCounterIn = other.m_packetsCounterIn;
	m_packetsCounterUse = other.m_packetsCounterUse;
	m_frameRate = other.m_frameRate;
	m_videoResolution = other.m_videoResolution;

	m_IsIce = other.m_IsIce;


	if(&(other.m_IcePartyAddrPort) )
	{
		memset(&m_IcePartyAddrPort,0,sizeof(mcTransportAddress));
		memcpy(&(m_IcePartyAddrPort),&(other.m_IcePartyAddrPort), sizeof(mcTransportAddress));
	}
	if(&(other.m_IceMcuAddrPort))
	{
		memset(&m_IceMcuAddrPort,0,sizeof(mcTransportAddress));
		memcpy(&(m_IceMcuAddrPort),&(other.m_IceMcuAddrPort), sizeof(mcTransportAddress));
	}

	m_IceConnectionType = other.m_IceConnectionType;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CIpChannelDetails::~CIpChannelDetails()
{
}

/////////////////////////////////////////////////////////////////////////////
void  CIpChannelDetails::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  m_ostr << m_channelType   << "\n";
  m_ostr << m_connectionStatus   << "\n";
  m_ostr << m_IsIce << "\n";
  m_ostr <<	m_IceConnectionType << "\n";

  if (apiNum>=51 || format != OPERATOR_MCMS)
	  m_ostr << m_actualRate   << "\n";
  else
  {
	  if (m_actualRate == 0xFFFFFFFF)
		  m_ostr << (WORD)0xFFFF   << "\n";
	  else
		  if (m_actualRate > 0xFFFF)
			   m_ostr << (WORD) 0xFFFE  << "\n";
		  else
			   m_ostr << (WORD) m_actualRate  << "\n";
  }

  // IpV6
  m_ostr << (DWORD)m_partyAddrPort.ipVersion << "\n";
  m_ostr << (DWORD)m_partyAddrPort.port << "\n";
  m_ostr << (DWORD)m_partyAddrPort.distribution << "\n";
  m_ostr << (DWORD)m_partyAddrPort.transportType << "\n";
  if ((enIpVersion)m_partyAddrPort.ipVersion == eIpVersion4)
	  m_ostr << (DWORD)m_partyAddrPort.addr.v4.ip << "\n";
  else
  {
	  m_ostr << (DWORD)m_partyAddrPort.addr.v6.scopeId << "\n";
	  char szIP[64];
	  ::ipToString(m_partyAddrPort, szIP,1); // With Brackets
	  m_ostr << szIP << "\n";
  }
  m_ostr << (DWORD)m_mcuAddrPort.ipVersion << "\n";
  m_ostr << (DWORD)m_mcuAddrPort.port << "\n";
  m_ostr << (DWORD)m_mcuAddrPort.distribution << "\n";
  m_ostr << (DWORD)m_mcuAddrPort.transportType << "\n";
  if ((enIpVersion)m_mcuAddrPort.ipVersion == eIpVersion4)
	  m_ostr << (DWORD)m_mcuAddrPort.addr.v4.ip << "\n";
  else
  {
	  m_ostr << (DWORD)m_mcuAddrPort.addr.v6.scopeId << "\n";
	  char szIP1[64];
	  ::ipToString(m_mcuAddrPort, szIP1,1); // With Brackets
	  m_ostr << szIP1 << "\n";
  }  

   m_ostr << m_packetsCounterIn   << "\n";
   m_ostr << m_packetsCounterUse   << "\n";

  if (apiNum>=50 || format != OPERATOR_MCMS)
	  m_ostr << m_frameRate   << "\n";

    if (apiNum>=532 || format != OPERATOR_MCMS)
	  m_ostr << (DWORD)m_videoResolution   << "\n";



  m_ostr << (DWORD)m_IcePartyAddrPort.ipVersion << "\n";
  m_ostr << (DWORD)m_IcePartyAddrPort.port << "\n";
  m_ostr << (DWORD)m_IcePartyAddrPort.distribution << "\n";
  m_ostr << (DWORD)m_IcePartyAddrPort.transportType << "\n";
  if ((enIpVersion)m_IcePartyAddrPort.ipVersion == eIpVersion4)
	  m_ostr << (DWORD)m_IcePartyAddrPort.addr.v4.ip << "\n";
  else
  {
	  m_ostr << (DWORD)m_IcePartyAddrPort.addr.v6.scopeId << "\n";
   	  char szIP2[64];
   	  ::ipToString(m_IcePartyAddrPort, szIP2,1); // With Brackets
   	  m_ostr << szIP2 << "\n";
  }

  m_ostr << (DWORD)m_IceMcuAddrPort.ipVersion << "\n";
  m_ostr << (DWORD)m_IceMcuAddrPort.port << "\n";
  m_ostr << (DWORD)m_IceMcuAddrPort.distribution << "\n";
  m_ostr << (DWORD)m_IceMcuAddrPort.transportType << "\n";
  if ((enIpVersion)m_IceMcuAddrPort.ipVersion == eIpVersion4)
	  m_ostr << (DWORD)m_IceMcuAddrPort.addr.v4.ip << "\n";
  else
  {
  	 m_ostr << (DWORD)m_IceMcuAddrPort.addr.v6.scopeId << "\n";
   	 char szIP3[64];
   	 ::ipToString(m_IceMcuAddrPort, szIP3,1); // With Brackets
   	 m_ostr << szIP3 << "\n";
  }

}

void CIpChannelDetails::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  DWORD dwTmp = 0;
  m_istr >> tmp;
  m_channelType = (EIpChannelType)tmp;
  m_istr >> tmp;
  m_connectionStatus = (BYTE)tmp;

  m_istr >> tmp;
  m_IsIce = (BYTE)tmp;
  m_istr >> tmp;
  m_IceConnectionType = (EIceConnectionType)tmp;

  if (apiNum>=51 || format != OPERATOR_MCMS)
	  m_istr >> m_actualRate;
  else{
	  WORD wTmp;

	  m_istr >> wTmp;
	  if (wTmp==0xFFFF)
		  m_actualRate = 0xFFFFFFFF;
	  else
		  m_actualRate = (DWORD) wTmp;
  }

  // IpV6
  m_istr >> m_partyAddrPort.ipVersion;
  m_istr >> m_partyAddrPort.port;
  m_istr >> m_partyAddrPort.distribution;
  m_istr >> m_partyAddrPort.transportType;
  if ((enIpVersion)m_partyAddrPort.ipVersion == eIpVersion4)
	  m_istr >> m_partyAddrPort.addr.v4.ip;
  else
  {
	  m_istr >> m_partyAddrPort.addr.v6.scopeId ;
	  char szIP[64];
	  memset(szIP,'\0',64);
	  m_istr >> szIP;
	  ::stringToIp(&m_partyAddrPort, szIP); // With Brackets
  } 
  // IpV6
  m_istr >> m_mcuAddrPort.ipVersion;
  m_istr >> m_mcuAddrPort.port;
  m_istr >> m_mcuAddrPort.distribution;
  m_istr >> m_mcuAddrPort.transportType;
  if ((enIpVersion)m_mcuAddrPort.ipVersion == eIpVersion4)
	  m_istr >> m_mcuAddrPort.addr.v4.ip;
  else
  {
	  m_istr >> m_mcuAddrPort.addr.v6.scopeId ;
	  char szIP[64];
	  memset(szIP,'\0',64);
	  m_istr >> szIP;
	  ::stringToIp(&m_mcuAddrPort, szIP); // With Brackets
  }   

  m_istr >> m_packetsCounterIn;
  m_istr >> m_packetsCounterUse;

  if (apiNum>=50 || format != OPERATOR_MCMS)
	  m_istr >> m_frameRate;

    if (apiNum>=532 || format != OPERATOR_MCMS)
	{
	  m_istr >> dwTmp;
	  m_videoResolution = (int)dwTmp;
	}

    m_istr >> m_IcePartyAddrPort.ipVersion;
    m_istr >> m_IcePartyAddrPort.port;
      m_istr >> m_IcePartyAddrPort.distribution;
      m_istr >> m_IcePartyAddrPort.transportType;
      if ((enIpVersion)m_IcePartyAddrPort.ipVersion == eIpVersion4)
    	  m_istr >> m_IcePartyAddrPort.addr.v4.ip;
      else
      {
    	  m_istr >> m_IcePartyAddrPort.addr.v6.scopeId ;
    	  char szIP2[64];
    	  memset(szIP2,'\0',64);
    	  m_istr >> szIP2;
    	  ::stringToIp(&m_IcePartyAddrPort, szIP2); // With Brackets
      }
      // IpV6
      m_istr >> m_IceMcuAddrPort.ipVersion;
      m_istr >> m_IceMcuAddrPort.port;

      m_istr >> m_IceMcuAddrPort.distribution;
      m_istr >> m_IceMcuAddrPort.transportType;
      if ((enIpVersion)m_IceMcuAddrPort.ipVersion == eIpVersion4)
    	  m_istr >> m_IceMcuAddrPort.addr.v4.ip;
      else
      {
    	  m_istr >> m_IceMcuAddrPort.addr.v6.scopeId ;
    	  char szIP3[64];
    	  memset(szIP3,'\0',64);
    	  m_istr >> szIP3;
    	  ::stringToIp(&m_IceMcuAddrPort, szIP3); // With Brackets
      }



                                             
}

/////////////////////////////////////////////////////////////////////////////
void CIpChannelDetails::SerializeXml(CXMLDOMElement* pParentNode)
{
	
	PTRACE(eLevelInfoNormal,"CIpChannelDetails::SerializeXml");

	if(pParentNode==NULL)
		return;

	CXMLDOMElement* pChannelNode = pParentNode->AddChildNode("H323_CHANNEL");

	pChannelNode->AddChildNode("H323_CHANNEL_TYPE",m_channelType,IP_CHANNEL_TYPE_ENUM);
	pChannelNode->AddChildNode("CONNECTED",m_connectionStatus,_BOOL);
	pChannelNode->AddChildNode("ACTUAL_RATE",m_actualRate);
	// IpV6 - Monitoring
	if ((enIpVersion)m_partyAddrPort.ipVersion == eIpVersion4)
		pChannelNode->AddIPChildNode("PARTY_ADDRESS", m_partyAddrPort);
	pChannelNode->AddChildNode("PARTY_PORT",m_partyAddrPort.port);
	// IpV6 - Monitoring
	if ((enIpVersion)m_mcuAddrPort.ipVersion == eIpVersion4)
		pChannelNode->AddIPChildNode("MCU_ADDRESS",m_mcuAddrPort);
	pChannelNode->AddChildNode("MCU_PORT",m_mcuAddrPort.port);

	pChannelNode->AddChildNode("PACKETS_IN",m_packetsCounterIn);
	pChannelNode->AddChildNode("PACKETS_USE",m_packetsCounterUse);

	//if (m_channelType==VIDEO_IN || m_channelType==VIDEO_OUT)
		pChannelNode->AddChildNode("H323_FRAME_RATE",m_frameRate);

	pChannelNode->AddChildNode("VIDEO_RESOLUTION",m_videoResolution,VIDEO_RESOLUTION_ENUM);
	if ((enIpVersion)m_partyAddrPort.ipVersion == eIpVersion6)
		pChannelNode->AddIPChildNode("PARTY_IPV6_ADDRESS",m_partyAddrPort,1);
	if ((enIpVersion)m_mcuAddrPort.ipVersion == eIpVersion6)
		pChannelNode->AddIPChildNode("MCU_IPV6_ADDRESS",m_mcuAddrPort,1);

	///////
	if ((enIpVersion)m_IcePartyAddrPort.ipVersion == eIpVersion4)
			pChannelNode->AddIPChildNode("ICE_PARTY_ADDRESS", m_IcePartyAddrPort);
	pChannelNode->AddChildNode("ICE_PARTY_PORT",m_IcePartyAddrPort.port);
		// IpV6 - Monitoring
	if ((enIpVersion)m_IceMcuAddrPort.ipVersion == eIpVersion4)
		pChannelNode->AddIPChildNode("ICE_MCU_ADDRESS",m_IceMcuAddrPort);
	pChannelNode->AddChildNode("ICE_MCU_PORT",m_IceMcuAddrPort.port);

	pChannelNode->AddChildNode("ICE_CONNECTION_TYPE",m_IceConnectionType,ICE_CONNECTION_TYPE_ENUM);

}

/////////////////////////////////////////////////////////////////////////////
int	CIpChannelDetails::DeSerializeXml(CXMLDOMElement* pChannelNode,char *pszError)
{

	PTRACE(eLevelInfoNormal,"CIpChannelDetails::DeSerializeXml");

	int nChanType = (int)IP_CHANNEL_TYPES_NUMBER;
	int nStatus=STATUS_OK;

	PASSERT(pChannelNode != NULL);

	GET_VALIDATE_CHILD(pChannelNode,"H323_CHANNEL_TYPE",&nChanType,IP_CHANNEL_TYPE_ENUM);

	if(nStatus != STATUS_OK)
		return nStatus;
	else
		m_channelType = (EIpChannelType)nChanType;

	GET_VALIDATE_CHILD(pChannelNode,"CONNECTED",&m_connectionStatus,_BOOL);
	
	if(nStatus != STATUS_OK)
		return nStatus;

	GET_VALIDATE_CHILD(pChannelNode,"ACTUAL_RATE",&m_actualRate,_BOOL);
	GET_VALIDATE_CHILD(pChannelNode,"PARTY_ADDRESS",&m_partyAddrPort,IP_ADDRESS);
	GET_VALIDATE_CHILD(pChannelNode,"PARTY_PORT",&(m_partyAddrPort.port),_0_TO_DWORD);
	GET_VALIDATE_CHILD(pChannelNode,"MCU_ADDRESS",&m_mcuAddrPort,IP_ADDRESS);
	GET_VALIDATE_CHILD(pChannelNode,"MCU_PORT",&(m_mcuAddrPort.port),_0_TO_DWORD);

	GET_VALIDATE_CHILD(pChannelNode,"PACKETS_IN",&m_packetsCounterIn,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pChannelNode,"PACKETS_USE",&m_packetsCounterUse,_0_TO_DWORD);

	//if (m_channelType==VIDEO_IN || m_channelType==VIDEO_OUT)
		GET_VALIDATE_CHILD(pChannelNode,"H323_FRAME_RATE",&m_frameRate,_0_TO_DWORD);

	GET_VALIDATE_CHILD(pChannelNode,"VIDEO_RESOLUTION",&m_videoResolution,VIDEO_RESOLUTION_ENUM);
	// IpV6 - Monitoring
    mcTransportAddress tempIpV6Addr;
    memset(&tempIpV6Addr,0,sizeof(mcTransportAddress));
    GET_VALIDATE_CHILD(pChannelNode,"PARTY_IPV6_ADDRESS",&tempIpV6Addr,IP_ADDRESS);
    if (tempIpV6Addr.ipVersion == (DWORD)eIpVersion6)
    	memcpy(&m_partyAddrPort, &tempIpV6Addr, sizeof(mcTransportAddress));
    memset(&tempIpV6Addr,0,sizeof(mcTransportAddress));
    GET_VALIDATE_CHILD(pChannelNode,"MCU_IPV6_ADDRESS",&tempIpV6Addr,IP_ADDRESS);
    if (tempIpV6Addr.ipVersion == (DWORD)eIpVersion6)
    	memcpy(&m_mcuAddrPort, &tempIpV6Addr, sizeof(mcTransportAddress));	


   	GET_VALIDATE_CHILD(pChannelNode,"ICE_PARTY_ADDRESS",&m_IcePartyAddrPort,IP_ADDRESS);
   	GET_VALIDATE_CHILD(pChannelNode,"ICE_PARTY_PORT",&(m_IcePartyAddrPort.port),_0_TO_DWORD);
   	GET_VALIDATE_CHILD(pChannelNode,"ICE_MCU_ADDRESS",&m_IceMcuAddrPort,IP_ADDRESS);
   	GET_VALIDATE_CHILD(pChannelNode,"ICE_MCU_PORT",&(m_IceMcuAddrPort.port),_0_TO_DWORD);

   	DWORD tmp =(DWORD) m_IceConnectionType;
   	GET_VALIDATE_CHILD(pChannelNode,"ICE_CONNECTION_TYPE",&tmp,ICE_CONNECTION_TYPE_ENUM);


	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
void   CIpChannelDetails::SetChannelType(const EIpChannelType channelType)
{
  m_channelType	= channelType;
}

/////////////////////////////////////////////////////////////////////////////
EIpChannelType   CIpChannelDetails::GetChannelType() const
{
  return m_channelType;
}

/////////////////////////////////////////////////////////////////////////////
void   CIpChannelDetails::SetConnectionStatus(const BYTE connectionStatus)
{
	m_connectionStatus	= connectionStatus;
}

/////////////////////////////////////////////////////////////////////////////
WORD   CIpChannelDetails::IsConnectedStatus() const
{
	if (m_connectionStatus)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void   CIpChannelDetails::SetActualRate(const DWORD actualRate)
{
	m_actualRate = actualRate;
}

/////////////////////////////////////////////////////////////////////////////
DWORD   CIpChannelDetails::GetActualRate() const
{
	return m_actualRate;
}

void CIpChannelDetails::SetPartyAddrPort(mcTransportAddress* ip_address)
{
	if (::isApiTaNull(ip_address) != TRUE && ::isIpTaNonValid(ip_address) != TRUE)
	  m_partyAddrPort = *ip_address;
}

void CIpChannelDetails::SetMcuAddrPort(mcTransportAddress* ip_address)
{
	if (::isApiTaNull(ip_address) != TRUE && ::isIpTaNonValid(ip_address) != TRUE)
	  m_mcuAddrPort = *ip_address;
}

const mcTransportAddress* CIpChannelDetails::GetMcuAddrPort() const
{
   return &m_mcuAddrPort;
}

const mcTransportAddress*  CIpChannelDetails::GetPartyAddrPort() const
{
   return &m_partyAddrPort;
}
/////////////////////////////////////////////////////////////////////////////
void   CIpChannelDetails::SetPacketsCounterIn(const DWORD packetsCounterIn)
{
	m_packetsCounterIn = packetsCounterIn;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CIpChannelDetails::GetPacketsCounterIn() const
{
	return m_packetsCounterIn;
}

/////////////////////////////////////////////////////////////////////////////
void   CIpChannelDetails::SetPacketsCounterUse(const DWORD packetsCounterUse)
{
	m_packetsCounterUse = packetsCounterUse;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CIpChannelDetails::GetPacketsCounterUse() const
{
	return m_packetsCounterUse;
}

/////////////////////////////////////////////////////////////////////////////
void   CIpChannelDetails::SetFrameRate(const WORD frameRate)
{
	m_frameRate = frameRate;
}

/////////////////////////////////////////////////////////////////////////////
WORD   CIpChannelDetails::GetFrameRate() const
{
	return m_frameRate;
}

/////////////////////////////////////////////////////////////////////////////
void   CIpChannelDetails::SetVideoResolution(const int videoResolution)
{
	m_videoResolution = videoResolution;
}
/////////////////////////////////////////////////////////////////////////////
int   CIpChannelDetails::GetVideoResolution() const
{
	return m_videoResolution;
}

void CIpChannelDetails::SetIcePartyAddrPort(mcTransportAddress* ip_address)
{
	if (ip_address && ::isApiTaNull(ip_address) != TRUE && ::isIpTaNonValid(ip_address) != TRUE)
	  m_IcePartyAddrPort = *ip_address;
}

const mcTransportAddress*  CIpChannelDetails::GetIcePartyAddrPort() const
{
	return &m_IcePartyAddrPort;
}

void CIpChannelDetails::SetIceMcuAddrPort(mcTransportAddress* ip_address)
{
	if (ip_address && ::isApiTaNull(ip_address) != TRUE && ::isIpTaNonValid(ip_address) != TRUE)
	  m_IceMcuAddrPort = *ip_address;
}

const mcTransportAddress* CIpChannelDetails::GetIceMcuAddrPort() const
{
	return &m_IceMcuAddrPort;
}

void CIpChannelDetails::SetIceConnectionType(EIceConnectionType ConnectionType)
{
	m_IceConnectionType = ConnectionType;
}
/////////////////////////////////////////////////////////////////////////////
EIceConnectionType  CIpChannelDetails::GetIceConnectionType() const
{
	return m_IceConnectionType;
}

