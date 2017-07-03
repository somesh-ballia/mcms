//+========================================================================+
//                            PartyMonitor.CPP                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PartyMonitor.CPP                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: ANAT A                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |21/01/04     |                                                     |
//+========================================================================+

#include <ostream>
#include <istream>
#include "PartyMonitor.h"
#include "InitCommonStrings.h"
#include "ConfPartyProcess.h"
#include "StringsMaps.h"
#include "Capabilities.h"
#include "StatusesGeneral.h"

#include "PObject.h"
#include "Segment.h"
#include "ChannelParams.h"
#include "ObjString.h"
#include "Trace.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "CapInfo.h"
#include "IceCmInd.h"
#include "EnumsToStrings.h"

///////////////////////////////////////////////////////////////////////////////////////
BYTE IsContainAnnex(int eAnnex,DWORD annexes)
{
	BYTE bRes = FALSE;
	annexes_fd_set  annexesMask;

	annexesMask.fds_bits[0] = annexes;

	if (CAP_FD_ISSET(eAnnex, &(annexesMask)))
		bRes = TRUE;

	return bRes;
}

/*************************************************************************************/
/*							CPrtMontrBaseParams										 */
/*************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////
//Party monitoring base
CPrtMontrBaseParams::CPrtMontrBaseParams()
{
	m_mapProblem	= 0;
	m_bitRate		= 0xFFFFFFFF;
	m_protocol		= 0;
	m_channelIndex	= 0;
	m_channelType	= 0;
	// IpV6 - Monitoring
	memset(&m_partyAddr,0,sizeof(mcTransportAddress));
	memset(&m_mcuAddr,0,sizeof(mcTransportAddress));

	memset(&m_IcePartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_IceMcuAddr,0,sizeof(mcTransportAddress));

	m_IceConnectionType = kNone;
	m_connectionStatus = 0;
	m_IsIce = 0;

}

///////////////////////////////////////////////////////////////////////////////////////
CPrtMontrBaseParams::CPrtMontrBaseParams(DWORD channelType)
{
	m_mapProblem	= 0;
	m_bitRate		= 0xFFFFFFFF;
	m_protocol		= 0;
	m_channelIndex	= 0;
	m_channelType	= channelType;
	// IpV6 - Monitoring
	memset(&m_partyAddr,0,sizeof(mcTransportAddress));
	memset(&m_mcuAddr,0,sizeof(mcTransportAddress));

	memset(&m_IcePartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_IceMcuAddr,0,sizeof(mcTransportAddress));

	m_IceConnectionType = kNone;
	m_connectionStatus = 0;

	m_IsIce = 0;

}

///////////////////////////////////////////////////////////////////////////////////////
CPrtMontrBaseParams::CPrtMontrBaseParams(const CPrtMontrBaseParams &other):CPObject(other)
{
	m_mapProblem	= other.m_mapProblem;
	m_bitRate		= other.m_bitRate;
	m_protocol		= other.m_protocol;
	m_channelIndex	= other.m_channelIndex;
	m_channelType	= other.m_channelType;
	memset(&m_partyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_partyAddr),&(other.m_partyAddr), sizeof(mcTransportAddress));
	memset(&m_mcuAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_mcuAddr),&(other.m_mcuAddr), sizeof(mcTransportAddress));


	m_IsIce = other.m_IsIce;

	memset(&m_IcePartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_IcePartyAddr),&(other.m_IcePartyAddr), sizeof(mcTransportAddress));
	memset(&m_IceMcuAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_IceMcuAddr),&(other.m_IceMcuAddr), sizeof(mcTransportAddress));

	m_IceConnectionType = other.m_IceConnectionType;

//	m_partyAddr		= other.m_partyAddr;
//	m_mcuAddr		= other.m_mcuAddr;
	m_connectionStatus = other.m_connectionStatus;
//	m_partyPort		= other.m_partyPort;
//	m_mcuPort		= other.m_mcuPort;
}
///////////////////////////////////////////////////////////////////////////////////////
CPrtMontrBaseParams& CPrtMontrBaseParams::operator=(const CPrtMontrBaseParams& other)
{
	if(this != &other)
	{
		m_connectionStatus = other.m_connectionStatus;

		m_bitRate = other.m_bitRate;
		// IpV6 - Monitoring
		if (::isApiTaNull(const_cast<mcTransportAddress*>(&other.m_partyAddr)) == FALSE && ::isIpTaNonValid(const_cast<mcTransportAddress*>(&other.m_partyAddr)) == FALSE)
		{
			memset(&m_partyAddr,0,sizeof(mcTransportAddress));
			memcpy(&(m_partyAddr),&(other.m_partyAddr), sizeof(mcTransportAddress));

		}
		if (::isApiTaNull(const_cast<mcTransportAddress*>(&other.m_mcuAddr)) == FALSE && ::isIpTaNonValid(const_cast<mcTransportAddress*>(&other.m_mcuAddr)) == FALSE)
		{
			memset(&m_mcuAddr,0,sizeof(mcTransportAddress));
			memcpy(&(m_mcuAddr),&(other.m_mcuAddr), sizeof(mcTransportAddress));
		}

		m_mapProblem	= other.m_mapProblem;
		m_protocol		= other.m_protocol;
		m_channelIndex	= other.m_channelIndex;
		m_channelType	= other.m_channelType;


		m_IsIce = other.m_IsIce;

		// ICE
		if (::isApiTaNull(const_cast<mcTransportAddress*>(&other.m_IcePartyAddr)) == FALSE && ::isIpTaNonValid(const_cast<mcTransportAddress*>(&other.m_IcePartyAddr)) == FALSE)
		{
			memset(&m_IcePartyAddr,0,sizeof(mcTransportAddress));
			memcpy(&(m_IcePartyAddr),&(other.m_IcePartyAddr), sizeof(mcTransportAddress));
		}
		if (::isApiTaNull(const_cast<mcTransportAddress*>(&other.m_IceMcuAddr)) == FALSE && ::isIpTaNonValid(const_cast<mcTransportAddress*>(&other.m_IceMcuAddr)) == FALSE)
		{
			memset(&m_IceMcuAddr,0,sizeof(mcTransportAddress));
			memcpy(&(m_IceMcuAddr),&(other.m_IceMcuAddr), sizeof(mcTransportAddress));
		}

		m_IceConnectionType = other.m_IceConnectionType;

	}

	return *this;
}
///////////////////////////////////////////////////////////////////////////////////////
DWORD CPrtMontrBaseParams::operator==(const CPrtMontrBaseParams& other)
{
	if(this == &other)
		return TRUE;

	if( m_mapProblem == other.m_mapProblem	&&
	    m_bitRate	== other.m_bitRate		&&
		m_protocol	== other.m_protocol		&&
		m_channelIndex	== other.m_channelIndex &&
		m_channelType	== other.m_channelType	&&
		(::isIpAddressEqual(&m_partyAddr,	const_cast<mcTransportAddress*>(&other.m_partyAddr)) == TRUE)	&&
		(::isIpAddressEqual(&m_mcuAddr,	const_cast<mcTransportAddress*>(&other.m_mcuAddr)) == TRUE)		&&
		m_connectionStatus == other.m_connectionStatus &&
		m_partyAddr.port	== other.m_partyAddr.port	&&
		m_mcuAddr.port	== other.m_mcuAddr.port &&
		m_IsIce == other.m_IsIce &&
		(::isIpAddressEqual(&m_IcePartyAddr,	const_cast<mcTransportAddress*>(&other.m_IcePartyAddr)) == TRUE)	&&
		(::isIpAddressEqual(&m_IceMcuAddr,	const_cast<mcTransportAddress*>(&other.m_IceMcuAddr)) == TRUE)	&&
		m_IcePartyAddr.port	== other.m_IcePartyAddr.port	&&
		m_IceMcuAddr.port	== other.m_IceMcuAddr.port &&
		m_IceConnectionType == other.m_IceConnectionType)

		return TRUE;
	else
		return FALSE;

}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CPrtMontrBaseParams::operator!=(const CPrtMontrBaseParams& other)
{
	return (!(*this == other));
}

///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrBaseParams::CopyClass(CPrtMontrBaseParams &other)
{
	*this = other;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CPrtMontrBaseParams::IsEqual(CPrtMontrBaseParams &other)
{
	return (*this == other);
}

///////////////////////////////////////////////////////////////////////////////////////
CPrtMontrBaseParams *CPrtMontrBaseParams::AllocNewClass(EIpChannelType channelType)
{
	CPrtMontrBaseParams  *pNewClass = NULL;

	switch(channelType)
	{
	case H225:
	case H245:
		pNewClass = new CPrtMontrBaseParams(channelType);
		break;
	case AUDIO_IN:
		pNewClass = new CAdvanceAudioIn;
		break;
	case AUDIO_OUT:
		pNewClass = new CAdvanceAudioOut;
		break;
	case VIDEO_IN:
	case VIDEO_CONT_IN:
		pNewClass = new CAdvanceVideoIn(channelType);
		break;
	case VIDEO_OUT:
	case VIDEO_CONT_OUT:
		pNewClass = new CAdvanceVideoOut(channelType);
		break;
	case FECC_IN:
		pNewClass = new CAdvanceFeccIn;
		break;
	case FECC_OUT:
		pNewClass = new CAdvanceFeccOut;
		break;
	case AUDIO_CONT_IN:
	case AUDIO_CONT_OUT:
		break;
	case BFCP_IN:
	case BFCP_OUT:
	case BFCP:
	case BFCP_UDP:
		pNewClass = new CPrtMontrBaseParams(channelType);
		break;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
	}

	return pNewClass;
}

//////////////////////////////////////////////////////////////////////////////////////
CPrtMontrBaseParams *CPrtMontrBaseParams::AllocNewClass(const CPrtMontrBaseParams &other)
{
	CPrtMontrBaseParams  *pNewClass = NULL;
	EIpChannelType		channelType = (EIpChannelType)other.GetChannelType();

	switch(channelType)
	{
	case H225:
	case H245:
		pNewClass = new CPrtMontrBaseParams(other);
		break;
	case AUDIO_IN:
		pNewClass = new CAdvanceAudioIn((const CAdvanceAudioIn&)other);
		break;
	case AUDIO_OUT:
		pNewClass = new CAdvanceAudioOut((const CAdvanceAudioOut&)other);
		break;
	case VIDEO_IN:
	case VIDEO_CONT_IN:
		pNewClass = new CAdvanceVideoIn((const CAdvanceVideoIn&)other);
		break;
	case VIDEO_OUT:
	case VIDEO_CONT_OUT:
		pNewClass = new CAdvanceVideoOut((const CAdvanceVideoOut&)other);
		break;
	case FECC_IN:
		pNewClass = new CAdvanceFeccIn((const CAdvanceFeccIn&)other);
		break;
	case FECC_OUT:
		pNewClass = new CAdvanceFeccOut((const CAdvanceFeccOut&)other);
		break;
	case AUDIO_CONT_IN:
	case AUDIO_CONT_OUT:
		break;
	case BFCP_IN:
	case BFCP_OUT:
		pNewClass = new CPrtMontrBaseParams(other);
		break;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
	}

	return pNewClass;
}


///////////////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void  CPrtMontrBaseParams::Serialize(WORD format,CSegment  &seg) const
{

	//FPTRACE(eLevelInfoNormal,"CPrtMontrBaseParams::Serialize ");

	switch ( format )
	{

	case NATIVE     :
		{
			seg	<<  m_mapProblem;
			seg	<<  m_bitRate;
			seg	<<  m_protocol;
			seg	<<  m_channelIndex;
			seg	<<  m_channelType;
			seg <<  m_IsIce;
			seg <<  (DWORD)m_IceConnectionType;

			// IpV6 - Monitoring
			seg << (DWORD)m_partyAddr.ipVersion ;
			seg << (DWORD)m_partyAddr.port;
			seg << (DWORD)m_partyAddr.distribution;
			seg << (DWORD)m_partyAddr.transportType;
			if ((enIpVersion)m_partyAddr.ipVersion == eIpVersion4)
				seg << (DWORD)m_partyAddr.addr.v4.ip;
			else
			{
				seg << (DWORD)m_partyAddr.addr.v6.scopeId;
				char szIP[64];
				::ipToString(m_partyAddr, szIP,1); // With Brackets
				seg << szIP ;
			}
			seg << (DWORD)m_mcuAddr.ipVersion ;
			seg << (DWORD)m_mcuAddr.port;
			seg << (DWORD)m_mcuAddr.distribution;
			seg << (DWORD)m_mcuAddr.transportType;
			if ((enIpVersion)m_mcuAddr.ipVersion == eIpVersion4)
				seg << (DWORD)m_mcuAddr.addr.v4.ip;
			else
			{
				seg << (DWORD)m_mcuAddr.addr.v6.scopeId;
				char szIP1[64];
				::ipToString(m_mcuAddr, szIP1,1); // With Brackets
				seg << szIP1 ;
			}

			if(m_IsIce)
			{
				seg << (DWORD)m_IcePartyAddr.ipVersion ;
				seg << (DWORD)m_IcePartyAddr.port;
				seg << (DWORD)m_IcePartyAddr.distribution;
				seg << (DWORD)m_IcePartyAddr.transportType;
				if ((enIpVersion)m_IcePartyAddr.ipVersion == eIpVersion4)
					seg << (DWORD)m_IcePartyAddr.addr.v4.ip;
				else
				{
					seg << (DWORD)m_IcePartyAddr.addr.v6.scopeId;
					char szIP2[64];
					::ipToString(m_IcePartyAddr, szIP2,1); // With Brackets
					seg << szIP2 ;
				}

				seg << (DWORD)m_IceMcuAddr.ipVersion ;
				seg << (DWORD)m_IceMcuAddr.port;
				seg << (DWORD)m_IceMcuAddr.distribution;
				seg << (DWORD)m_IceMcuAddr.transportType;
				if ((enIpVersion)m_IceMcuAddr.ipVersion == eIpVersion4)
					seg << (DWORD)m_IceMcuAddr.addr.v4.ip;
				else
				{
					seg << (DWORD)m_IceMcuAddr.addr.v6.scopeId;
					char szIP3[64];
					::ipToString(m_IceMcuAddr, szIP3,1); // With Brackets
					seg << szIP3 ;
				}

			}

//			seg	<<  m_partyAddr;
//			seg	<<  m_mcuAddr;
			seg	<<  m_connectionStatus;
//			seg	<<  m_partyPort;
//			seg	<<  m_mcuPort;

			break;
		}
	default :
		break;
    }
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CPrtMontrBaseParams::DeSerialize(WORD format,CSegment &seg)
{
	//FPTRACE(eLevelInfoNormal,"CPrtMontrBaseParams::DeSerialize");

	switch ( format )
	{

	case NATIVE     :
		{
			seg >> m_mapProblem;
			seg >> m_bitRate;
			seg >> m_protocol;
			seg >> m_channelIndex;
			seg >> m_channelType;
			seg >> m_IsIce;

			DWORD tmp = 0;
			seg >> tmp;
			m_IceConnectionType = (EIceConnectionType)tmp;

			// IpV6
			seg >> m_partyAddr.ipVersion;
			seg >> m_partyAddr.port;
			seg >> m_partyAddr.distribution;
			seg >> m_partyAddr.transportType;
			if ((enIpVersion)m_partyAddr.ipVersion == eIpVersion4)
				seg >> m_partyAddr.addr.v4.ip;
			else
			{
				seg >> m_partyAddr.addr.v6.scopeId ;
				char szIP[64];
				memset(szIP,'\0',64);
				seg >> szIP;
				::stringToIp(&m_partyAddr, szIP); // With Brackets
			}


			// IpV6
			seg >> m_mcuAddr.ipVersion;
			seg >> m_mcuAddr.port;
			seg >> m_mcuAddr.distribution;
			seg >> m_mcuAddr.transportType;
			if ((enIpVersion)m_mcuAddr.ipVersion == eIpVersion4)
				seg >> m_mcuAddr.addr.v4.ip;
			else
			{
				seg >> m_mcuAddr.addr.v6.scopeId ;
				char szIP1[64];
				memset(szIP1,'\0',64);
				seg >> szIP1;
				::stringToIp(&m_mcuAddr, szIP1); // With Brackets
			}


			if(m_IsIce)
			{
				seg >> m_IcePartyAddr.ipVersion;
				seg >> m_IcePartyAddr.port;
				seg >> m_IcePartyAddr.distribution;
				seg >> m_IcePartyAddr.transportType;
				if ((enIpVersion)m_IcePartyAddr.ipVersion == eIpVersion4)
					seg >> m_IcePartyAddr.addr.v4.ip;
				else
				{
					seg >> m_IcePartyAddr.addr.v6.scopeId ;
					char szIP2[64];
					memset(szIP2,'\0',64);
					seg >> szIP2;
					::stringToIp(&m_IcePartyAddr, szIP2); // With Brackets
				}

				seg >> m_IceMcuAddr.ipVersion;
				seg >> m_IceMcuAddr.port;
				seg >> m_IceMcuAddr.distribution;
				seg >> m_IceMcuAddr.transportType;
				if ((enIpVersion)m_IceMcuAddr.ipVersion == eIpVersion4)
					seg >> m_IceMcuAddr.addr.v4.ip;
				else
				{
					seg >> m_IcePartyAddr.addr.v6.scopeId ;
					char szIP3[64];
					memset(szIP3,'\0',64);
					seg >> szIP3;
					::stringToIp(&m_IcePartyAddr, szIP3); // With Brackets
				}


			}

//			seg >> m_partyAddr;
//			seg	>> m_mcuAddr;
			seg >> m_connectionStatus;
//			seg >> m_partyPort;
//			seg >> m_mcuPort;


			break;
		}
	default : {
		break;
			  }
    }
}

///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrBaseParams::Dump1()
{
	CLargeString strBase1;
	char szPartyIP[64];
	char szMcuIP[64];
	char IcePartyIP[64];
	char IceMcuIP[64];

	memset(szPartyIP, '\0', 64);
	memset(szMcuIP, '\0', 64);
	memset(IcePartyIP, '\0', 64);
	memset(IceMcuIP, '\0', 64);

	::ipToString(m_partyAddr, szPartyIP, 1); // With Brackets
	::ipToString(m_mcuAddr, szMcuIP, 1); // With Brackets

	if (m_IsIce)
		::ipToString(m_IcePartyAddr, IcePartyIP, 1); // With Brackets
		::ipToString(m_IceMcuAddr, IceMcuIP, 1); // With Brackets

	strBase1 << " : \nm_mapProblem = " << m_mapProblem << "\nm_bitRate = " << m_bitRate
	<< "\nm_protocol = " << m_protocol << "\nm_channelIndex = " << m_channelIndex << "\nm_channelType = " << m_channelType
	<< "\nm_partyAddr = " << szPartyIP << "\npartyPort = " << m_partyAddr.port << "\nm_mcuAddr = "  << szMcuIP
	<< "\nm_mcuPort = " << m_mcuAddr.port << "\nm_connectionStatus = " << m_connectionStatus
	<< "\nm_IsICE = " << m_IsIce;

	if (m_IsIce)
	{
		strBase1 << "\nm_IcePartyAdd = " << IcePartyIP <<"\npartyPort = " << m_IcePartyAddr.port
		<<	"\nm_IceMcuAdd = " << IceMcuIP <<  "\nm_IceMcuPort = " << m_IceMcuAddr.port
		<< "\nm_IceConnectionType = "<<  m_IceConnectionType;
	}

	FPTRACE2(eLevelInfoNormal,"CPrtMontrBaseParams::Dump ", strBase1.GetString());
}

///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrBaseParams::SerializeXml(CXMLDOMElement* pFatherNode)
{


	CXMLDOMElement* pPrtMontrBaseParams = pFatherNode->AddChildNode("BASIC_PARAM");

	CXMLDOMElement* pChildNode = pPrtMontrBaseParams->AddChildNode("MAP_PROBLEM");
	if (m_mapProblem&BitRateProblem)
		pChildNode->AddChildNode("PROBLEM",BitRateProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&FractionLossProblem)
		pChildNode->AddChildNode("PROBLEM",FractionLossProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&JitterProblem)
		pChildNode->AddChildNode("PROBLEM",JitterProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&LatencyProblem)
		pChildNode->AddChildNode("PROBLEM",LatencyProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&ProtocolProblem)
		pChildNode->AddChildNode("PROBLEM",ProtocolProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&ResolutionProblem)
		pChildNode->AddChildNode("PROBLEM",ResolutionProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&FrameRateProblem)
		pChildNode->AddChildNode("PROBLEM",FrameRateProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&AnnexesProblem)
		pChildNode->AddChildNode("PROBLEM",AnnexesProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&FramePerPacketProblem)
		pChildNode->AddChildNode("PROBLEM",FramePerPacketProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&ActualLossAccProblem)
		pChildNode->AddChildNode("PROBLEM",ActualLossAccProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&ActualLossInterProblem)
		pChildNode->AddChildNode("PROBLEM",ActualLossInterProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&OutOfOrderAccProblem)
		pChildNode->AddChildNode("PROBLEM",OutOfOrderAccProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&OutOfOrderInterProblem)
		pChildNode->AddChildNode("PROBLEM",OutOfOrderInterProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&FragmentedAccProblem)
		pChildNode->AddChildNode("PROBLEM",FragmentedAccProblem,MAP_PROBLEM_ENUM);
	if (m_mapProblem&FragmentedInterProblem)
		pChildNode->AddChildNode("PROBLEM",FragmentedInterProblem,MAP_PROBLEM_ENUM);
	pPrtMontrBaseParams->AddChildNode("BIT_RATE",m_bitRate);
	pPrtMontrBaseParams->AddChildNode("PROTOCOL",m_protocol, CAP_CODE_ENUM);

	pPrtMontrBaseParams->AddChildNode("CHANNEL_INDEX",m_channelIndex);
	pPrtMontrBaseParams->AddChildNode("CHANNEL_TYPE",m_channelType, IP_CHANNEL_TYPE_ENUM);
	// IpV6 - Monitoring
	if ((enIpVersion)m_partyAddr.ipVersion == eIpVersion4)
		pPrtMontrBaseParams->AddIPChildNode("PARTY_ADDRESS", m_partyAddr);


	// IpV6 - Monitoring
	if ((enIpVersion)m_mcuAddr.ipVersion == eIpVersion4)
		pPrtMontrBaseParams->AddIPChildNode("MCU_ADDRESS",m_mcuAddr);

	pPrtMontrBaseParams->AddChildNode("PARTY_PORT",m_partyAddr.port);
	pPrtMontrBaseParams->AddChildNode("MCU_PORT",m_mcuAddr.port);

	pPrtMontrBaseParams->AddChildNode("PARTY_TRANSPORT_TYPE", m_partyAddr.transportType, TRANSPORT_TYPE_ENUM);
	pPrtMontrBaseParams->AddChildNode("MCU_TRANSPORT_TYPE", m_mcuAddr.transportType, TRANSPORT_TYPE_ENUM);

	pPrtMontrBaseParams->AddChildNode("CONNECTION_STATUS",m_connectionStatus);

	//Ice
		if ((enIpVersion)m_IcePartyAddr.ipVersion == eIpVersion4)
		{
			pPrtMontrBaseParams->AddIPChildNode("ICE_PARTY_ADDRESS", m_IcePartyAddr);

		}

		if ((enIpVersion)m_IceMcuAddr.ipVersion == eIpVersion4)
		{
			pPrtMontrBaseParams->AddIPChildNode("ICE_MCU_ADDRESS",m_IceMcuAddr);
		}

		pPrtMontrBaseParams->AddChildNode("ICE_PARTY_PORT",m_IcePartyAddr.port);
		pPrtMontrBaseParams->AddChildNode("ICE_MCU_PORT",m_IceMcuAddr.port);

		pPrtMontrBaseParams->AddChildNode("ICE_PARTY_TRANSPORT_TYPE", m_IcePartyAddr.transportType, TRANSPORT_TYPE_ENUM);
		pPrtMontrBaseParams->AddChildNode("ICE_MCU_TRANSPORT_TYPE", m_IceMcuAddr.transportType, TRANSPORT_TYPE_ENUM);

		pPrtMontrBaseParams->AddChildNode("ICE_CONNECTION_TYPE",m_IceConnectionType,ICE_CONNECTION_TYPE_ENUM);


		if ((enIpVersion)m_partyAddr.ipVersion == eIpVersion6)
			pPrtMontrBaseParams->AddIPChildNode("PARTY_IPV6_ADDRESS",m_partyAddr,1);
		if ((enIpVersion)m_mcuAddr.ipVersion == eIpVersion6)
			pPrtMontrBaseParams->AddIPChildNode("MCU_IPV6_ADDRESS",m_mcuAddr,1);

}

///////////////////////////////////////////////////////////////////////////////////////
int CPrtMontrBaseParams::DeSerializeXml(CXMLDOMElement *pFatherNode,char *pszError)
{

	int nStatus = STATUS_OK;
	CXMLDOMElement *pActionNode, *pChildNode, *pNode;

	GET_CHILD_NODE(pFatherNode,"BASIC_PARAM", pActionNode);

	if (pActionNode)
	{
		m_mapProblem = 0;
		GET_CHILD_NODE(pActionNode,"MAP_PROBLEM", pChildNode);

		if (pChildNode)
		{
			int num;
			GET_FIRST_CHILD_NODE(pChildNode,"PROBLEM", pNode);

			while (pNode)
			{
				GET_VALIDATE(pNode, &num, MAP_PROBLEM_ENUM);
				switch(num)
				{
				case BitRateProblem:
					m_mapProblem |= BitRateProblem;
					break;
				case FractionLossProblem:
					m_mapProblem |= FractionLossProblem;
					break;
				case JitterProblem:
					m_mapProblem |= JitterProblem;
					break;
				case LatencyProblem:
					m_mapProblem |= LatencyProblem;
					break;
				case ProtocolProblem:
					m_mapProblem |= ProtocolProblem;
					break;
				case ResolutionProblem:
					m_mapProblem |= ResolutionProblem;
					break;
				case FrameRateProblem:
					m_mapProblem |= FrameRateProblem;
					break;
				case AnnexesProblem:
					m_mapProblem |= AnnexesProblem;
					break;
				case FramePerPacketProblem:
					m_mapProblem |= FramePerPacketProblem;
					break;
				case ActualLossAccProblem:
					m_mapProblem |= ActualLossAccProblem;
					break;
				case ActualLossInterProblem:
					m_mapProblem |= ActualLossInterProblem;
					break;
				case OutOfOrderAccProblem:

					m_mapProblem |= OutOfOrderAccProblem;
					break;
				case OutOfOrderInterProblem:
					m_mapProblem |= OutOfOrderInterProblem;
					break;
				case FragmentedAccProblem:
					m_mapProblem |= FragmentedAccProblem;
					break;
				case FragmentedInterProblem:
					m_mapProblem |= FragmentedInterProblem;
					break;
				}

				GET_NEXT_CHILD_NODE(pChildNode, "PROBLEM", pNode);
			}
		}
		BYTE tmpTransportType = 0;

		GET_VALIDATE_CHILD(pActionNode,"BIT_RATE",&m_bitRate,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pActionNode,"PROTOCOL",&m_protocol,CAP_CODE_ENUM);
		GET_VALIDATE_CHILD(pActionNode,"CHANNEL_INDEX",&m_channelIndex,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pActionNode,"CHANNEL_TYPE",&m_channelType,IP_CHANNEL_TYPE_ENUM);
		GET_VALIDATE_CHILD(pActionNode,"PARTY_ADDRESS",&m_partyAddr,IP_ADDRESS);
		GET_VALIDATE_CHILD(pActionNode,"MCU_ADDRESS",&m_mcuAddr,IP_ADDRESS);
		GET_VALIDATE_CHILD(pActionNode,"PARTY_PORT",&m_partyAddr.port,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"MCU_PORT",&m_mcuAddr.port,_0_TO_WORD);

		tmpTransportType = 0;
		GET_VALIDATE_CHILD(pActionNode,"PARTY_TRANSPORT_TYPE",&tmpTransportType,TRANSPORT_TYPE_ENUM);
		m_partyAddr.transportType = (enTransportType)tmpTransportType;

		tmpTransportType = 0;
		GET_VALIDATE_CHILD(pActionNode,"MCU_TRANSPORT_TYPE",&tmpTransportType,TRANSPORT_TYPE_ENUM);
		m_mcuAddr.transportType = (enTransportType)tmpTransportType;

		GET_VALIDATE_CHILD(pActionNode,"CONNECTION_STATUS",&m_connectionStatus,_0_TO_DWORD);

		//Ice
		GET_VALIDATE_CHILD(pActionNode,"ICE_PARTY_ADDRESS",&m_IcePartyAddr,IP_ADDRESS);
		GET_VALIDATE_CHILD(pActionNode,"ICE_MCU_ADDRESS",&m_IceMcuAddr,IP_ADDRESS);
		GET_VALIDATE_CHILD(pActionNode,"ICE_PARTY_PORT",&m_IcePartyAddr.port,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"ICE_MCU_PORT",&m_IceMcuAddr.port,_0_TO_WORD);

		tmpTransportType = 0;
		GET_VALIDATE_CHILD(pActionNode,"ICE_PARTY_TRANSPORT_TYPE",&tmpTransportType, TRANSPORT_TYPE_ENUM);
		m_IcePartyAddr.transportType = (enTransportType)tmpTransportType;

		tmpTransportType = 0;
		GET_VALIDATE_CHILD(pActionNode,"ICE_MCU_TRANSPORT_TYPE",&tmpTransportType, TRANSPORT_TYPE_ENUM);
		m_IceMcuAddr.transportType = (enTransportType)tmpTransportType;


		DWORD tmp =(DWORD) m_IceConnectionType;
		GET_VALIDATE_CHILD(pActionNode,"ICE_CONNECTION_TYPE",&tmp,ICE_CONNECTION_TYPE_ENUM);

	    mcTransportAddress tempIpV6Addr;
	    memset(&tempIpV6Addr,0,sizeof(mcTransportAddress));
	    GET_VALIDATE_CHILD(pActionNode,"PARTY_IPV6_ADDRESS",&tempIpV6Addr,IP_ADDRESS);
	    if (tempIpV6Addr.ipVersion == (DWORD)eIpVersion6)
	    	memcpy(&m_partyAddr, &tempIpV6Addr, sizeof(mcTransportAddress));
	    memset(&tempIpV6Addr,0,sizeof(mcTransportAddress));
	    GET_VALIDATE_CHILD(pActionNode,"MCU_IPV6_ADDRESS",&tempIpV6Addr,IP_ADDRESS);
	    if (tempIpV6Addr.ipVersion == (DWORD)eIpVersion6)
	    	memcpy(&m_mcuAddr, &tempIpV6Addr, sizeof(mcTransportAddress));

	}

	return STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrBaseParams::CheckExceedingFieldsRules(DWORD rate,DWORD protocol,CMedString &strExcFieldsRules)
{
//#ifdef __HIGHC__
	// get system.cfg values
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "THRESHOLD_BITRATE";
	DWORD thresholdBitRate = 11000;//::GetpSystemCfg()->GetThresholdBitRate();
	pSysConfig->GetDWORDDataByKey(key, thresholdBitRate);
	DWORD currBitRate = m_bitRate / 1000;

	//We move to units between 1 to 10000 that 10000 is 100%
	if((thresholdBitRate != 10000) && (currBitRate * 10000) > (rate * thresholdBitRate)) //Need to check if the new rate is over the current rate by 110%
	{
		strExcFieldsRules <<" Exceed bit rate. Current: "<<currBitRate<<" ,expected: "<<rate;
		SetProblem(BitRateProblem);
	}

	if (m_protocol != protocol)
	{
		strExcFieldsRules
			<< "Current:" << CapEnumToString((CapEnum)m_protocol)
			<< ", Expected: " << CapEnumToString((CapEnum)protocol)
			<< " - Different protocol";

		if (m_protocol < eUnknownAlgorithemCapCode && protocol < eUnknownAlgorithemCapCode)
		{
			SetProblem(ProtocolProblem);
		}
		else
		{
			SetProblem(ProtocolProblem);
			DBGPASSERT(m_protocol);
			DBGPASSERT(protocol);
		}
	}
//#endif
}

///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrBaseParams::SetPartyAddr(mcTransportAddress* partyAddr)
{
	memset(&m_partyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_partyAddr),partyAddr, sizeof(mcTransportAddress));
}

///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrBaseParams::SetMcuAddr(mcTransportAddress* mcuAddr)
{
	memset(&m_mcuAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_mcuAddr),mcuAddr, sizeof(mcTransportAddress));
}
///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrBaseParams::SetIcePartyAddr(mcTransportAddress* IcePartyAddr)
{
	memset(&m_IcePartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_IcePartyAddr),IcePartyAddr, sizeof(mcTransportAddress));
}

///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrBaseParams::SetIceMcuAddr(mcTransportAddress* IceMcuAddr)
{
	memset(&m_IceMcuAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_IceMcuAddr),IceMcuAddr, sizeof(mcTransportAddress));
}
///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrBaseParams::SetIceConnectionType(EIceConnectionType IceConnectionType)
{
	m_IceConnectionType = IceConnectionType;

}



/*************************************************************************************/
/*							CPrtMontrGlobalParams										 */
/*************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////
CPrtMontrGlobalParams::CPrtMontrGlobalParams()
{
	m_numOfPackets	= 0;
	m_latency		= 0;
	m_packetLoss	= 0;
	m_jitter		= 0;
	m_jitterPeak	= 0;
	m_franctioLoss	= 0;
	m_franctioLossPeak	= 0;
}

///////////////////////////////////////////////////////////////////////////////////////
CPrtMontrGlobalParams::CPrtMontrGlobalParams(const CPrtMontrGlobalParams &other):CPrtMontrBaseParams(other)
{
	m_numOfPackets	= other.m_numOfPackets;
	m_latency		= other.m_latency;
	m_packetLoss	= other.m_packetLoss;
	m_jitter		= other.m_jitter;
	m_jitterPeak	= other.m_jitterPeak;
	m_franctioLoss	= other.m_franctioLoss;
	m_franctioLossPeak	= other.m_franctioLossPeak;
}

///////////////////////////////////////////////////////////////////////////////////////
CPrtMontrGlobalParams& CPrtMontrGlobalParams::operator=(const CPrtMontrGlobalParams& other)
{
	if(this != &other)
	{
		m_numOfPackets	= other.m_numOfPackets;
		m_latency		= other.m_latency;
		m_packetLoss	= other.m_packetLoss;
		m_jitter		= other.m_jitter;
		m_jitterPeak	= other.m_jitterPeak;
		m_franctioLoss	= other.m_franctioLoss;
		m_franctioLossPeak	= other.m_franctioLossPeak;
		(CPrtMontrBaseParams &)*this = (const CPrtMontrBaseParams &)other;
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CPrtMontrGlobalParams::operator==(const CPrtMontrGlobalParams& other)
{
	if(this == &other)
		return TRUE;
	if((CPrtMontrBaseParams &)*this == (const CPrtMontrBaseParams &)other &&
		m_numOfPackets	== other.m_numOfPackets	&&
		m_latency		== other.m_latency		&&
		m_packetLoss	== other.m_packetLoss	&&
		m_jitter		== other.m_jitter		&&
		m_jitterPeak	== other.m_jitterPeak	&&
		m_franctioLoss	== other.m_franctioLoss	&&
		m_franctioLossPeak	== other.m_franctioLossPeak)
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CPrtMontrGlobalParams::operator!=(const CPrtMontrGlobalParams& other)
{
	return (!(*this == other));
}

///////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrGlobalParams::CopyClass(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CPrtMontrGlobalParams"))
		*this = (CPrtMontrGlobalParams &)other;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CPrtMontrGlobalParams::IsEqual(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CPrtMontrGlobalParams"))
		return (*this == (CPrtMontrGlobalParams &)other);
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void  CPrtMontrGlobalParams::Serialize(WORD format,CSegment  &seg) const
{
	switch ( format )
	{

	case NATIVE     :
		{
			CPrtMontrBaseParams::Serialize(format,seg);
			seg	<< m_numOfPackets;
			seg	<< m_latency;
			seg	<< m_packetLoss;
			seg	<< m_jitter;
			seg	<< m_jitterPeak;
			seg	<< m_franctioLoss;
			seg	<< m_franctioLossPeak;

			break;
		}
	default :
		break;
    }

}

/////////////////////////////////////////////////////////////////////////////	* /
void  CPrtMontrGlobalParams::DeSerialize(WORD format,CSegment &seg)
{
	switch ( format )
	{

	case NATIVE     :
		{
			CPrtMontrBaseParams::DeSerialize(format,seg);
			seg >> m_numOfPackets;
			seg >> m_latency;
			seg >> m_packetLoss;
			seg >> m_jitter;
			seg >> m_jitterPeak;
			seg >> m_franctioLoss;
			seg >> m_franctioLossPeak;

			break;
		}
	default : {
		break;
			  }
    }
}

/////////////////////////////////////////////////////////////////////////////	* /
void CPrtMontrGlobalParams::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pPrtMontrGlobalParams = pFatherNode->AddChildNode("GLOBAL_PARAM");

	CPrtMontrBaseParams::SerializeXml(pPrtMontrGlobalParams);
	pPrtMontrGlobalParams->AddChildNode("NUMBER_OF_PACKETS",m_numOfPackets);
	pPrtMontrGlobalParams->AddChildNode("LATENCY",m_latency);
	pPrtMontrGlobalParams->AddChildNode("PACKET_LOSS",m_packetLoss);
	pPrtMontrGlobalParams->AddChildNode("JITTER",m_jitter);
	pPrtMontrGlobalParams->AddChildNode("JITTER_PEAK",m_jitterPeak);
	pPrtMontrGlobalParams->AddChildNode("FRACTION_LOSS",m_franctioLoss);
	pPrtMontrGlobalParams->AddChildNode("FRACTION_LOSS_PEAK",m_franctioLossPeak);
}

/////////////////////////////////////////////////////////////////////////////	* /
int CPrtMontrGlobalParams::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pChildNode;

	GET_CHILD_NODE(pActionNode, "GLOBAL_PARAM", pChildNode);
	if (pChildNode)
	{
		nStatus = CPrtMontrBaseParams::DeSerializeXml(pChildNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;

		GET_VALIDATE_CHILD(pChildNode,"NUMBER_OF_PACKETS",&m_numOfPackets,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChildNode,"LATENCY",&m_latency,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChildNode,"PACKET_LOSS",&m_packetLoss,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChildNode,"JITTER",&m_jitter,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChildNode,"JITTER_PEAK",&m_jitterPeak,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChildNode,"FRACTION_LOSS",&m_franctioLoss,_0_TO_WORD);
		GET_VALIDATE_CHILD(pChildNode,"FRACTION_LOSS_PEAK",&m_franctioLossPeak,_0_TO_WORD);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CPrtMontrGlobalParams::SetGlobalParam(const CPrtMontrGlobalParams& globalParam)
{
	//we do not want the operator= that copy the Base too.
	m_numOfPackets	= globalParam.m_numOfPackets;
	m_latency		= globalParam.m_latency;
	m_packetLoss	= globalParam.m_packetLoss;
	m_jitter		= globalParam.m_jitter;
	m_jitterPeak	= globalParam.m_jitterPeak;
	m_franctioLoss	= globalParam.m_franctioLoss;
	m_franctioLossPeak	= globalParam.m_franctioLossPeak;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CPrtMontrGlobalParams::SetJitter(DWORD jitter)
{
	if((m_channelType == AUDIO_IN) || (m_channelType == AUDIO_OUT) || (m_channelType == FECC_OUT) || (m_channelType == FECC_IN))
		m_jitter = jitter / AudiokHz;	// FecckHz equal to AudiokHz
	else
		m_jitter = jitter / VideokHz;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CPrtMontrGlobalParams::SetJitterPeak(DWORD jitterPeak)
{
	if((m_channelType == AUDIO_IN) || (m_channelType == AUDIO_OUT) || (m_channelType == FECC_OUT) || (m_channelType == FECC_IN))
		m_jitterPeak = jitterPeak / AudiokHz;	// FecckHz equal to AudiokHz
	else
		m_jitterPeak = jitterPeak / VideokHz;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CPrtMontrGlobalParams::SetFranctionLoss(unsigned short franctioLost)
{
	m_franctioLoss = (franctioLost*10000) / FractionLossPercent;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CPrtMontrGlobalParams::SetFranctionLossPeak(unsigned short franctioLostPeak)
{
	m_franctioLossPeak = (franctioLostPeak*10000) / FractionLossPercent;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CPrtMontrGlobalParams::SetLatency(DWORD latency)
{
	// WORD  seconds	   = 0;
// 	DWORD milliseconds = 0;
// 	DWORD hexConvert = 65536;//2^16
// 	seconds = latency >> 16;
// 	milliseconds = (latency & LSBmask);
// 	milliseconds *= 1000;
// 	milliseconds /= hexConvert;
// 	milliseconds += seconds*1000;
// 	m_latency = milliseconds;
    m_latency = latency;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CPrtMontrGlobalParams::CheckExceedingFieldsRules(DWORD rate,DWORD protocol,CMedString &strExcFieldsRules)
{
//#ifdef __HIGHC__
	// get system.cfg values
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DWORD thresholdBitRate = 11000;//::GetpSystemCfg()->GetThresholdBitRate();
	DWORD thresholdLatency		= 300;//::GetpSystemCfg()->GetThresholdLatency();
	DWORD thresholdFractionLoss = 50;//::GetpSystemCfg()->GetThresholdFractionLoss();
	DWORD thresholdJitter		= 80;//::GetpSystemCfg()->GetThresholdJitter();

	std::string key = "THRESHOLD_BITRATE";
	pSysConfig->GetDWORDDataByKey(key, thresholdBitRate);
	key = "THRESHOLD_LATENCY";
	pSysConfig->GetDWORDDataByKey(key, thresholdLatency);
	key = "THRESHOLD_FRACTION_LOSS";
	pSysConfig->GetDWORDDataByKey(key, thresholdFractionLoss);
	key = "THRESHOLD_JITTER";
	pSysConfig->GetDWORDDataByKey(key, thresholdJitter);


	if(thresholdLatency && (m_latency  > thresholdLatency))
	{
		strExcFieldsRules<<" Exceed latency = "<< m_latency;
		strExcFieldsRules<<" threshold "<<thresholdLatency;
		SetProblem(LatencyProblem);
	}

	if(thresholdJitter && (m_jitter > thresholdJitter))
	{
		strExcFieldsRules<<" Exceed jitter "<<m_jitter;
		SetProblem(JitterProblem);
	}

	// the m_franctioLoss is now x (from the card) * 1000 / 256 need to be bigger than y (trashhold)
	// y is system.cfg param which is % * 1000
	if(thresholdFractionLoss && (m_franctioLoss >= thresholdFractionLoss))
	{
		strExcFieldsRules<<" Exceed franction loss "<<m_franctioLoss;
		SetProblem(FractionLossProblem);
	}

	CPrtMontrBaseParams::CheckExceedingFieldsRules(rate,protocol,strExcFieldsRules);
//#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
void CPrtMontrGlobalParams::Dump1()
{
	CLargeString strBase1;
	strBase1<< " : \nm_numOfPackets = " << m_numOfPackets << "\nm_latency = " << m_latency
	<< "\nm_packetLoss = " << m_packetLoss << "\nm_jitter = " << m_jitter << "\nm_jitterPeak = " << m_jitterPeak
	<< "\nm_franctioLoss = " << m_franctioLoss << "\nm_franctioLossPeak = " << m_franctioLossPeak;

	CPrtMontrBaseParams::Dump1();
	FPTRACE2(eLevelInfoNormal,"CPrtMontrGlobalParams::Dump ", strBase1.GetString());

}

/*************************************************************************************/
/*							CAdvanceChInfo										 */
/*************************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////
CAdvanceChInfo::CAdvanceChInfo()
{
	m_accumulate		= 0;
	m_accumulatePercent = 0xFFFFFFFF;
	m_intervalPercent	= 0xFFFFFFFF;
	m_interval			= 0xFFFF;
	m_intervalPeak		= 0;
}

////////////////////////////////////////////////////////////////////////////////////////
CAdvanceChInfo::CAdvanceChInfo(const CAdvanceChInfo &other):CPObject(other)
{
	m_accumulate		= other.m_accumulate;
	m_accumulatePercent = other.m_accumulatePercent;
	m_intervalPercent	= other.m_intervalPercent;
	m_interval			= other.m_interval;
	m_intervalPeak		= other.m_intervalPeak;
}

////////////////////////////////////////////////////////////////////////////////////////
CAdvanceChInfo& CAdvanceChInfo::operator=(const CAdvanceChInfo& other)
{
	if(this != &other)
	{
		m_accumulate		= other.m_accumulate;
		m_accumulatePercent = other.m_accumulatePercent;
		m_intervalPercent	= other.m_intervalPercent;
		m_interval			= other.m_interval;
		m_intervalPeak		= other.m_intervalPeak;
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////	* /
DWORD CAdvanceChInfo::operator==(const CAdvanceChInfo& other)
{
	if(this == &other)
		return TRUE;

	if(	m_accumulate		== other.m_accumulate	&&
		m_accumulatePercent == other.m_accumulatePercent &&
		m_intervalPercent	== other.m_intervalPercent	 &&
		m_interval			== other.m_interval		&&
		m_intervalPeak		== other.m_intervalPeak)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////	* /
DWORD CAdvanceChInfo::operator!=(const CAdvanceChInfo& other)
{
	return (!(*this == other));
}

/////////////////////////////////////////////////////////////////////////////	* /
//#ifdef __HIGHC__
void  CAdvanceChInfo::Serialize(WORD format,CSegment  &seg) const
{
	switch ( format )
	{

	case NATIVE     :
		{

			seg	<< m_accumulate;
			seg	<< m_accumulatePercent;
			seg	<< m_intervalPercent;
			seg	<< m_interval;
			seg	<< m_intervalPeak;

			break;
		}
	default :
		break;
    }
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CAdvanceChInfo::DeSerialize(WORD format,CSegment &seg)
{
	switch ( format )
	{

	case NATIVE     :
		{
			seg	>> m_accumulate;
			seg	>> m_accumulatePercent;
			seg	>> m_intervalPercent;
			seg	>> m_interval;
			seg	>> m_intervalPeak;

			break;
		}
	default : {
		break;
			  }
    }
}
//#endif

/////////////////////////////////////////////////////////////////////////////////////////
void CAdvanceChInfo::SerializeXml(CXMLDOMElement* pFatherNode)
{
	pFatherNode->AddChildNode("ACCUMULATE",m_accumulate);
	pFatherNode->AddChildNode("ACCUMULATE_PERCENT",m_accumulatePercent);
	pFatherNode->AddChildNode("INTERVAL_PERCENT",m_intervalPercent);
	pFatherNode->AddChildNode("INTERVAL_NUMBER",m_interval);
	pFatherNode->AddChildNode("INTERVAL_PEAK",m_intervalPeak);

}

/////////////////////////////////////////////////////////////////////////////	* /
int CAdvanceChInfo::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"ACCUMULATE",&m_accumulate,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"ACCUMULATE_PERCENT",&m_accumulatePercent,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"INTERVAL_PERCENT",&m_intervalPercent,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"INTERVAL_NUMBER",&m_interval,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"INTERVAL_PEAK",&m_intervalPeak,_0_TO_WORD);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CAdvanceChInfo::Dump1()
{
	CLargeString strBase1;
	strBase1<< " : \nm_accumulate = " << m_accumulate << "\nm_accumulatePercent = " << m_accumulatePercent
	<< "\nm_intervalPercent = " << m_intervalPercent << "\nm_interval = " << m_interval << "\nm_intervalPeak = " << m_intervalPeak;

	FPTRACE2(eLevelInfoNormal,"CAdvanceChInfo::Dump ", strBase1.GetString());


}
/*************************************************************************************/
/*							CRtpInfo												 */
/*************************************************************************************/

/////////////////////////////////////////////////////////////////////////////	* /
CRtpInfo::CRtpInfo():m_rtpPacketLoss(),m_rtpOutOfOrder(),m_rtpFragmentPackets(),m_jitterBufferSize(),m_jitterLatePackets(),m_jitterOverflows(),m_jitterSamplePacketInterval()
{
}

/////////////////////////////////////////////////////////////////////////////	* /
CRtpInfo::CRtpInfo(const CRtpInfo &other):CPObject(other)
{
	m_rtpPacketLoss			= other.m_rtpPacketLoss;
	m_rtpOutOfOrder			= other.m_rtpOutOfOrder;
	m_rtpFragmentPackets	= other.m_rtpFragmentPackets;
	m_jitterBufferSize		= other.m_jitterBufferSize;
	m_jitterLatePackets		= other.m_jitterLatePackets;
	m_jitterOverflows		= other.m_jitterOverflows;
	m_jitterSamplePacketInterval	= other.m_jitterSamplePacketInterval;
}

/////////////////////////////////////////////////////////////////////////////	* /
CRtpInfo& CRtpInfo::operator=(const CRtpInfo& other)
{
	if(this != &other)
	{
		m_rtpPacketLoss			= other.m_rtpPacketLoss;
		m_rtpOutOfOrder			= other.m_rtpOutOfOrder;
		m_rtpFragmentPackets	= other.m_rtpFragmentPackets;
		m_jitterBufferSize		= other.m_jitterBufferSize;
		m_jitterLatePackets		= other.m_jitterLatePackets;
		m_jitterOverflows		= other.m_jitterOverflows;
		m_jitterSamplePacketInterval	= other.m_jitterSamplePacketInterval;
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////	* /
DWORD CRtpInfo::operator==(const CRtpInfo& other)
{
	if(this == &other)
		return TRUE;

	if(	m_rtpPacketLoss			== other.m_rtpPacketLoss	&&
		m_rtpOutOfOrder			== other.m_rtpOutOfOrder	&&
		m_rtpFragmentPackets	== other.m_rtpFragmentPackets &&
		m_jitterBufferSize		== other.m_jitterBufferSize	&&
		m_jitterLatePackets		== other.m_jitterLatePackets &&
		m_jitterOverflows		== other.m_jitterOverflows	&&
		m_jitterSamplePacketInterval	== other.m_jitterSamplePacketInterval)
		return TRUE;
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////	* /
DWORD CRtpInfo::operator!=(const CRtpInfo& other)
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////	* /
//#ifdef __HIGHC__
void  CRtpInfo::Serialize(WORD format,CSegment  &seg) const
{
	switch ( format )
	{

	case NATIVE     :
		{
			m_rtpPacketLoss.Serialize(format,seg);
			m_rtpOutOfOrder.Serialize(format,seg);
			m_rtpFragmentPackets.Serialize(format,seg);
			m_jitterBufferSize.Serialize(format,seg);
			m_jitterLatePackets.Serialize(format,seg);
			m_jitterOverflows.Serialize(format,seg);
			m_jitterSamplePacketInterval.Serialize(format,seg);

			break;
		}
	default :
		break;
    }
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CRtpInfo::DeSerialize(WORD format,CSegment &seg)
{
	switch ( format )
	{

	case NATIVE     :
		{
			m_rtpPacketLoss.DeSerialize(format,seg);
			m_rtpOutOfOrder.DeSerialize(format,seg);
			m_rtpFragmentPackets.DeSerialize(format,seg);
			m_jitterBufferSize.DeSerialize(format,seg);
			m_jitterLatePackets.DeSerialize(format,seg);
			m_jitterOverflows.DeSerialize(format,seg);
			m_jitterSamplePacketInterval.DeSerialize(format,seg);

			break;
		}
	default : {
		break;
			  }
    }
}
//#endif

/////////////////////////////////////////////////////////////////////////////	* /
void CRtpInfo::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pRtpInfo = pFatherNode->AddChildNode("RTP_INFO");

	CXMLDOMElement* pTempNode = pRtpInfo->AddChildNode("RTP_PACKET_LOSS");
	m_rtpPacketLoss.SerializeXml(pTempNode);
	pTempNode = pRtpInfo->AddChildNode("RTP_OUT_OF_ORDER");
	m_rtpOutOfOrder.SerializeXml(pTempNode);
	pTempNode = pRtpInfo->AddChildNode("RTP_FRAGMENT_PACKETS");
	m_rtpFragmentPackets.SerializeXml(pTempNode);
	pTempNode = pRtpInfo->AddChildNode("JITTER_BUFFER_SIZE");
	m_jitterBufferSize.SerializeXml(pTempNode);
	pTempNode = pRtpInfo->AddChildNode("JITTER_LATE_PACKETS");
	m_jitterLatePackets.SerializeXml(pTempNode);
	pTempNode = pRtpInfo->AddChildNode("JITTER_OVERFLOWS");
	m_jitterOverflows.SerializeXml(pTempNode);
	pTempNode = pRtpInfo->AddChildNode("JITTER_SAMPLE_PACKET_INTERVAL");
	m_jitterSamplePacketInterval.SerializeXml(pTempNode);
}

/////////////////////////////////////////////////////////////////////////////	* /
int CRtpInfo::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement* pChildNode;

	GET_CHILD_NODE(pActionNode, "RTP_PACKET_LOSS", pChildNode);
	if (pChildNode)
	{
		nStatus = m_rtpPacketLoss.DeSerializeXml(pChildNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "RTP_OUT_OF_ORDER", pChildNode);
	if (pChildNode)
	{
		nStatus = m_rtpOutOfOrder.DeSerializeXml(pChildNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "RTP_FRAGMENT_PACKETS", pChildNode);
	if (pChildNode)
	{
		nStatus = m_rtpFragmentPackets.DeSerializeXml(pChildNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "JITTER_BUFFER_SIZE", pChildNode);
	if (pChildNode)
	{
		nStatus = m_jitterBufferSize.DeSerializeXml(pChildNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "JITTER_LATE_PACKETS", pChildNode);
	if (pChildNode)
	{
		nStatus = m_jitterLatePackets.DeSerializeXml(pChildNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "JITTER_OVERFLOWS", pChildNode);
	if (pChildNode)
	{
		nStatus = m_jitterOverflows.DeSerializeXml(pChildNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "JITTER_SAMPLE_PACKET_INTERVAL", pChildNode);
	if (pChildNode)
	{
		nStatus = m_jitterSamplePacketInterval.DeSerializeXml(pChildNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CRtpInfo::SetRtpInfo(const CRtpInfo& rtpInfo)
{
	*this = rtpInfo;
}

/////////////////////////////////////////////////////////////////////////////
void CRtpInfo::DumpStr()
{
	PTRACE(eLevelInfoNormal,"CRtpInfo::Dump");
	PTRACE(eLevelInfoNormal,"\nm_rtpPacketLoss ");
	m_rtpPacketLoss.Dump1();
	PTRACE(eLevelInfoNormal,"\nm_rtpOutOfOrder ");
	m_rtpOutOfOrder.Dump1();
	PTRACE(eLevelInfoNormal,"\nm_rtpFragmentPackets ");
	m_rtpFragmentPackets.Dump1();
	PTRACE(eLevelInfoNormal,"\nm_jitterBufferSize ");
	m_jitterBufferSize.Dump1();
	PTRACE(eLevelInfoNormal,"\nm_jitterLatePackets ");
	m_jitterLatePackets.Dump1();
	PTRACE(eLevelInfoNormal,"\nm_jitterOverflows ");
	m_jitterOverflows.Dump1();
	PTRACE(eLevelInfoNormal,"\nm_jitterSamplePacketInterval ");
	m_jitterSamplePacketInterval.Dump1();
}

/////////////////////////////////////////////////////////////////////////////	* /
DWORD CRtpInfo::CheckExceedingFieldsRules(CMedString &strExcFieldsRules)
{
	DWORD mapProblem = 0;
//#ifdef __HIGHC__
	DWORD res = 0;
	WORD quotient;	//full
	WORD remainder;

	//PacketLoss
	// get system.cfg values
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "";

	DWORD thresholdNumber = 50;//::GetpSystemCfg()->GetThresholdAccPacketLoss();
	key = "THRESHOLD_ACCUMULATE_PACKET_LOSS";
	pSysConfig->GetDWORDDataByKey(key, thresholdNumber);
	res = m_rtpPacketLoss.GetAccumulatePercent();
	if(res == 0xFFFFFFFF)// this is the default value, its happened some times in the first monitoring indication. Should ignore this error
		res = 0;
	res = res /10;
	if(thresholdNumber && (res > thresholdNumber))
	{
		quotient	= res / 100;
		remainder	= res % 100;
		strExcFieldsRules<<" Exceed rtp packet loss accumulate "<<quotient<<"."<<remainder;
		mapProblem |= ActualLossAccProblem;
	}

	thresholdNumber = 50;//::GetpSystemCfg()->GetThresholdInterPacketLoss();
	key = "THRESHOLD_INTERVAL_PACKET_LOSS";
	pSysConfig->GetDWORDDataByKey(key, thresholdNumber);
	res = m_rtpPacketLoss.GetIntervalPercent();
	if(res == 0xFFFFFFFF)// this is the default value, its happened some times in the first monitoring indication. Should ignore this error
		res = 0;
	res = res /10;
	if(thresholdNumber && (res > thresholdNumber))
	{
		quotient	= res / 100;
		remainder	= res % 100;
		strExcFieldsRules<<" Exceed rtp packet loss interval "<<quotient<<"."<<remainder;
		mapProblem |= ActualLossInterProblem;
	}


	//OutOfOrder
	thresholdNumber = 50;//::GetpSystemCfg()->GetThresholdAccOutOfOrder();
	key = "THRESHOLD_ACCUMULATE_OUT_OF_ORDER";
	pSysConfig->GetDWORDDataByKey(key, thresholdNumber);
	res = m_rtpOutOfOrder.GetAccumulatePercent();
	if(res == 0xFFFFFFFF)// this is the default value, its happened some times in the first monitoring indication. Should ignore this error
		res = 0;
	res = res /10;
	if(thresholdNumber && (res > thresholdNumber))
	{
		quotient	= res / 100;
		remainder	= res % 100;
		strExcFieldsRules<<" Exceed out of order accumulate "<<quotient<<"."<<remainder;
		mapProblem |= OutOfOrderAccProblem;
	}

	thresholdNumber = 50;//::GetpSystemCfg()->GetThresholdIntervalOutOfOrder();
	key = "THRESHOLD_INTERVAL_OUT_OF_ORDER";
	pSysConfig->GetDWORDDataByKey(key, thresholdNumber);
	res = m_rtpOutOfOrder.GetIntervalPercent();
	if(res == 0xFFFFFFFF)// this is the default value, its happened some times in the first monitoring indication. Should ignore this error
		res = 0;
	res = res /10;
	if(thresholdNumber && (res > thresholdNumber))
	{
		quotient	= res / 100;
		remainder	= res % 100;
		strExcFieldsRules<<" Exceed out of order interval "<<quotient<<"."<<remainder;
		mapProblem |= OutOfOrderInterProblem;
	}

	//Fragmented
	thresholdNumber = 50;//::GetpSystemCfg()->GetThresholdAccFragmented();
	key = "THRESHOLD_ACCUMULATE_FRAGMENTED";
	pSysConfig->GetDWORDDataByKey(key, thresholdNumber);
	res = m_rtpFragmentPackets.GetAccumulatePercent();
	if(res == 0xFFFFFFFF)// this is the default value, its happened some times in the first monitoring indication. Should ignore this error
		res = 0;
	res = res /10;
	if(thresholdNumber && (res > thresholdNumber))
	{
		quotient	= res / 100;
		remainder	= res % 100;
		strExcFieldsRules<<" Exceed fragment packets accumulate "<<quotient<<"."<<remainder;
		mapProblem |= FragmentedAccProblem;
	}

	thresholdNumber = 50;//::GetpSystemCfg()->GetThresholdIntervalFragmented();
	key = "THRESHOLD_INTERVAL_FRAGMENTED";
	pSysConfig->GetDWORDDataByKey(key, thresholdNumber);
	res = m_rtpFragmentPackets.GetIntervalPercent();
	if(res == 0xFFFFFFFF)// this is the default value, its happened some times in the first monitoring indication. Should ignore this error
		res = 0;
	res = res /10;
	if(thresholdNumber && (res > thresholdNumber))
	{
		quotient	= res / 100;
		remainder	= res % 100;
		strExcFieldsRules<<" Exceed fragment packets interval "<<quotient<<"."<<remainder;
		mapProblem |= FragmentedInterProblem;
	}
//#endif
	return mapProblem;
}

/*************************************************************************************/
/*							CAdvanceAudio    										 */
/*************************************************************************************/

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceAudio::CAdvanceAudio(const CAdvanceAudio &other):CPrtMontrGlobalParams(other)
{
	m_framePerPacket = other.m_framePerPacket;
}

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceAudio& CAdvanceAudio::operator=(const CAdvanceAudio& other)
{
	if(this != &other)
	{
		m_framePerPacket = other.m_framePerPacket;
		(CPrtMontrGlobalParams &)*this = (CPrtMontrGlobalParams &)other;
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////	* /
DWORD CAdvanceAudio::operator==(const CAdvanceAudio& other)
{
	if(this == &other)
		return TRUE;

	if((CPrtMontrGlobalParams &)*this == (CPrtMontrGlobalParams &)other &&
	    m_framePerPacket == other.m_framePerPacket)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////	* /
DWORD CAdvanceAudio::operator!=(const CAdvanceAudio& other)
{
	return (!(*this == other));
}

///////////////////////////////////////////////////////////////////////////////////////
void CAdvanceAudio::CopyClass(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceAudio"))
	{
		*this = (CAdvanceAudio &)other;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CAdvanceAudio::IsEqual(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceAudio"))
		return (*this == (CAdvanceAudio &)other);
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////	* /
//#ifdef __HIGHC__
void  CAdvanceAudio::Serialize(WORD format,CSegment  &seg) const
{
	switch ( format )
	{

	case NATIVE     :
		{
			CPrtMontrGlobalParams::Serialize(format,seg);
			seg	<< m_framePerPacket;

			break;
		}
	default :
		break;
    }
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CAdvanceAudio::DeSerialize(WORD format,CSegment &seg)
{
	switch ( format )
	{

	case NATIVE     :
		{
			CPrtMontrGlobalParams::DeSerialize(format,seg);
			seg >> m_framePerPacket;

			break;
		}
	default : {
		break;
			  }
    }
}
//#endif

///////////////////////////////////////////////////////////////////	* /
void CAdvanceAudio::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pAdvanceAudio = pFatherNode->AddChildNode("ADVANCED_AUDIO_PARAM");

	CPrtMontrGlobalParams::SerializeXml(pAdvanceAudio);

	pAdvanceAudio->AddChildNode("FRAME_PER_PACKET",m_framePerPacket);
}

/////////////////////////////////////////////////////////////////////////////	* /
int CAdvanceAudio::DeSerializeXml(CXMLDOMElement *pFatherNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pActionNode;

	GET_CHILD_NODE(pFatherNode, "ADVANCED_AUDIO_PARAM", pActionNode);

	if (pActionNode)
	{
		nStatus = CPrtMontrGlobalParams::DeSerializeXml(pActionNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;

		GET_VALIDATE_CHILD(pActionNode,"FRAME_PER_PACKET",&m_framePerPacket,_0_TO_DWORD);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CAdvanceAudio::CheckExceedingFieldsRules(DWORD rate,DWORD protocol,
											  DWORD framePerPacket,CMedString &strExcFieldsRules)
{
	if(m_framePerPacket != framePerPacket)
	{
		strExcFieldsRules<<" Different frame per packet. Current: "<<m_framePerPacket;
		strExcFieldsRules<<" ,expected: "<<framePerPacket;
		SetProblem(FramePerPacketProblem);
	}

	CPrtMontrGlobalParams::CheckExceedingFieldsRules(rate,protocol,strExcFieldsRules);
}

/////////////////////////////////////////////////////////////////////////////
void CAdvanceAudio::Dump1()
{
	CLargeString strBase1;
	strBase1<< " : \nm_framePerPacket = " << m_framePerPacket;

	FPTRACE2(eLevelInfoNormal,"CAdvanceAudio::Dump ", strBase1.GetString());
	CPrtMontrGlobalParams::Dump1();


}


/*************************************************************************************/
/*							CAdvanceAudioOut    									 */
/*************************************************************************************/

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceAudioOut::CAdvanceAudioOut()
{
	m_channelType = AUDIO_OUT;
}
////////////////////////////////////////////////////
void CAdvanceAudioOut::CopyClass(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceAudioOut"))
	{
		*this = (CAdvanceAudioOut &)other;
	}
}


/////////////////////////////////////////////////////////////////////////////
void CAdvanceAudioOut::Dump1()
{
	PTRACE(eLevelInfoNormal,"CAdvanceAudioOut::Dump \n");
	CAdvanceAudio::Dump1();

}
/*************************************************************************************/
/*							CAdvanceAudioIn     									 */
/*************************************************************************************/


/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceAudioIn::CAdvanceAudioIn()
{
	m_channelType = AUDIO_IN;
}

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceAudioIn::CAdvanceAudioIn(const CAdvanceAudioIn &other):CAdvanceAudio(other),CRtpInfo(other)
{
}

/////////////////////////////////////////////////////////////////////////////
/*void CAdvanceAudioIn::Dump1()
{
	PTRACE(eLevelInfoNormal,"CAdvanceAudioIn::Dump \n");
	((CAdvanceAudio&)*this).Dump1();

}*/
/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceAudioIn& CAdvanceAudioIn::operator=(const CAdvanceAudioIn& other)
{
	if(this != &other)
	{
		(CAdvanceAudio &)*this = (CAdvanceAudio &)other;
		m_rtpPacketLoss			= other.m_rtpPacketLoss;
		m_rtpOutOfOrder			= other.m_rtpOutOfOrder;
		m_rtpFragmentPackets	= other.m_rtpFragmentPackets;
		m_jitterBufferSize		= other.m_jitterBufferSize;
		m_jitterLatePackets		= other.m_jitterLatePackets;
		m_jitterOverflows		= other.m_jitterOverflows;
		m_jitterSamplePacketInterval	= other.m_jitterSamplePacketInterval;
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CAdvanceAudioIn::operator==(const CAdvanceAudioIn& other)
{
	if(this == &other)
		return TRUE;

	if( (CAdvanceAudio &)*this == (CAdvanceAudio &)other &&
		m_rtpPacketLoss		== other.m_rtpPacketLoss	 &&
		m_rtpOutOfOrder		== other.m_rtpOutOfOrder	 &&
		m_rtpFragmentPackets	== other.m_rtpFragmentPackets	&&
		m_jitterBufferSize		== other.m_jitterBufferSize		&&
		m_jitterLatePackets		== other.m_jitterLatePackets	&&
		m_jitterOverflows		== other.m_jitterOverflows		&&
		m_jitterSamplePacketInterval	== other.m_jitterSamplePacketInterval)
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CAdvanceAudioIn::operator!=(const CAdvanceAudioIn& other)
{
	return (!(*this == other));
}

///////////////////////////////////////////////////////////////////////////////////////
void CAdvanceAudioIn::CopyClass(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceAudioIn"))
	{
		*this = (CAdvanceAudioIn &)other;
	}
}

/////////////////////////////////////////////////////////////////////////////	*
DWORD CAdvanceAudioIn::IsEqual(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceAudioIn"))
		return (*this == (CAdvanceAudioIn &)other);
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////	*
//#ifdef __HIGHC__
void  CAdvanceAudioIn::Serialize(WORD format,CSegment  &seg) const
{
	switch ( format )
	{

	case NATIVE     :
		{
			CAdvanceAudio::Serialize(format,seg);
			CRtpInfo::Serialize(format,seg);

			break;
		}
	default :
		break;
    }
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CAdvanceAudioIn::DeSerialize(WORD format,CSegment &seg)
{
	switch ( format )
	{

	case NATIVE     :
		{
			CAdvanceAudio::DeSerialize(format,seg);
			CRtpInfo::DeSerialize(format,seg);

			break;
		}
	default : {
		break;
			  }
    }
}
//#endif

/////////////////////////////////////////////////////////////////////////////	* /
void CAdvanceAudioIn::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pAdvanceAudioIn = pFatherNode->AddChildNode("ADVANCED_AUDIO_IN");

	CAdvanceAudio::SerializeXml(pAdvanceAudioIn);
	CRtpInfo::SerializeXml(pAdvanceAudioIn);
}

/////////////////////////////////////////////////////////////////////////////	* /
int CAdvanceAudioIn::DeSerializeXml(CXMLDOMElement *pFatherNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pChildNode, *pActionNode;

	GET_CHILD_NODE(pFatherNode, "ADVANCED_AUDIO_IN", pActionNode);

	if (pActionNode)
	{
		nStatus = CAdvanceAudio::DeSerializeXml(pActionNode,pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "RTP_INFO", pChildNode);
	if (pChildNode)
	{
		nStatus = CRtpInfo::DeSerializeXml(pChildNode,pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CAdvanceAudioIn::CheckExceedingFieldsRules(DWORD rate,DWORD protocol,
												long framePerPacket,CMedString &strExcFieldsRules)
{
	DWORD mapProblem = CRtpInfo::CheckExceedingFieldsRules(strExcFieldsRules);
	CAdvanceAudio::CheckExceedingFieldsRules(rate,protocol,framePerPacket,strExcFieldsRules);

	SetProblem(mapProblem);
}


/*************************************************************************************/
/*							CAdvanceVideo       									 */
/*************************************************************************************/


/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceVideo::CAdvanceVideo()
{
	m_annexes			 = 0;
	m_resolution		 = 0xFFFFFFFF;
	m_maxResolution		 = 0xFFFFFFFF;
	m_minResolution		 = 0xFFFFFFFF;
	m_frameRate			 = 0;
	m_maxFrameRate			 = 0;
	m_minFrameRate			 = 0;
	m_resolutionWidth    = 0;
	m_maxResolutionWidth    = 0;
	m_minResolutionWidth    = 0;
	m_resolutionHeight   = 0;
	m_maxResolutionHeight   = 0;
	m_minResolutionHeight   = 0;
}

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceVideo::CAdvanceVideo(const CAdvanceVideo &other):CPrtMontrGlobalParams(other)
{
	m_annexes			= other.m_annexes;
	m_resolution		= other.m_resolution;
	m_maxResolution		= other.m_maxResolution;
	m_minResolution		= other.m_minResolution;
	m_frameRate			= other.m_frameRate;
	m_maxFrameRate		= other.m_maxFrameRate;
	m_minFrameRate			= other.m_minFrameRate;
	m_resolutionWidth   = other.m_resolutionWidth;
	m_maxResolutionWidth   = other.m_maxResolutionWidth;
	m_minResolutionWidth   = other.m_minResolutionWidth;
	m_resolutionHeight  = other.m_resolutionHeight;
	m_maxResolutionHeight  = other.m_maxResolutionHeight;
	m_minResolutionHeight  = other.m_minResolutionHeight;
}

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceVideo& CAdvanceVideo::operator=(const CAdvanceVideo& other)
{
	if(this != &other)
	{
		m_annexes			= other.m_annexes;
		m_resolution		= other.m_resolution;
		m_maxResolution		= other.m_maxResolution;
		m_minResolution		= other.m_minResolution;
		m_frameRate			= other.m_frameRate;
		m_maxFrameRate		= other.m_maxFrameRate;
		m_minFrameRate			= other.m_minFrameRate;
		m_resolutionWidth   = other.m_resolutionWidth;
		m_maxResolutionWidth   = other.m_maxResolutionWidth;
		m_minResolutionWidth   = other.m_minResolutionWidth;
		m_resolutionHeight  = other.m_resolutionHeight;
		m_maxResolutionHeight  = other.m_maxResolutionHeight;
		m_minResolutionHeight  = other.m_minResolutionHeight;


		(CPrtMontrGlobalParams &)*this = (CPrtMontrGlobalParams &)other;
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////	* /
DWORD CAdvanceVideo::operator==(const CAdvanceVideo& other)
{
	if(this == &other)
		return TRUE;

	if(	(CPrtMontrGlobalParams &)*this == (CPrtMontrGlobalParams &)other &&
		m_annexes		== other.m_annexes		&&
		m_resolution	== other.m_resolution	&&
		m_maxResolution	== other.m_maxResolution	&&
		m_minResolution	== other.m_minResolution	&&
		m_frameRate		== other.m_frameRate &&
		m_maxFrameRate		== other.m_maxFrameRate &&
		m_minFrameRate		== other.m_minFrameRate &&
		m_resolutionWidth   == other.m_resolutionWidth &&
		m_maxResolutionWidth   == other.m_maxResolutionWidth &&
		m_minResolutionWidth   == other.m_minResolutionWidth &&
		m_maxResolutionHeight   == other.m_maxResolutionHeight &&
		m_minResolutionHeight   == other.m_minResolutionHeight &&
		m_resolutionHeight  == other.m_resolutionHeight)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////	* /
DWORD CAdvanceVideo::operator!=(const CAdvanceVideo& other)
{
	return (!(*this == other));
}

///////////////////////////////////////////////////////////////////////////////////////
void CAdvanceVideo::CopyClass(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceVideo"))
	{
		*this = (CAdvanceVideo &)other;
	}
}

/////////////////////////////////////////////////////////////////////////////	*
DWORD CAdvanceVideo::IsEqual(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceVideo"))
		return (*this == (CAdvanceVideo &)other);
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////	* /
//#ifdef __HIGHC__
void  CAdvanceVideo::Serialize(WORD format,CSegment  &seg) const
{
	switch ( format )
	{

	case NATIVE     :
		{
			CPrtMontrGlobalParams::Serialize(format,seg);
			seg << m_annexes;
			seg << (DWORD)m_resolution;
			seg << (DWORD)m_maxResolution;
			seg << (DWORD)m_minResolution;
			seg << m_frameRate;
			seg << m_maxFrameRate;
			seg << m_minFrameRate;
			seg << m_resolutionWidth;
			seg << m_maxResolutionWidth;
			seg << m_minResolutionWidth;
			seg << m_resolutionHeight;
			seg << m_maxResolutionHeight;
			seg << m_minResolutionHeight;

			break;
		}
	default :
		break;
    }
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CAdvanceVideo::DeSerialize(WORD format,CSegment &seg)
{
	switch ( format )
	{

	case NATIVE     :
		{
			CPrtMontrGlobalParams::DeSerialize(format,seg);
			DWORD	resolution;
			DWORD   maxResolution;
			DWORD   minResolution;
			seg >> m_annexes;
			seg >> resolution;
			seg >> maxResolution;
			seg >> minResolution;
			seg >> m_frameRate;
			seg >> m_maxFrameRate;
			seg >> m_minFrameRate;
			seg >> m_resolutionWidth;
			seg >> m_maxResolutionWidth;
			seg >> m_minResolutionWidth;
			seg >> m_resolutionHeight;
			seg >> m_maxResolutionHeight;
			seg >> m_minResolutionHeight;

			m_resolution = (int)resolution;
			m_maxResolution = (int)maxResolution;
			m_minResolution = (int)minResolution;
			break;
		}
	default : {
		break;
			  }
    }
}
//#endif

/////////////////////////////////////////////////////////////////////////////	* /
void CAdvanceVideo::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pAdvanceVideo = pFatherNode->AddChildNode("ADVANCED_VIDEO_PARAM");

	CPrtMontrGlobalParams::SerializeXml(pAdvanceVideo);

	CXMLDOMElement* pChildNode = pAdvanceVideo->AddChildNode("ANNEXES");
	for (int i=0; i<H263_Annexes_Number; i++)
	{
		if (IsAnnex(i))
			pChildNode->AddChildNode("ANNEX",i,ANNEXES_ENUM);
	}

	pAdvanceVideo->AddChildNode("RESOLUTION",m_resolution, VIDEO_RESOLUTION_ENUM);
	pAdvanceVideo->AddChildNode("VIDEO_FRAME_RATE",m_frameRate);
	pAdvanceVideo->AddChildNode("RESOLUTION_WIDTH",m_resolutionWidth);
	pAdvanceVideo->AddChildNode("RESOLUTION_HEIGHT",m_resolutionHeight);
}

/////////////////////////////////////////////////////////////////////////////	* /
BYTE CAdvanceVideo::IsAnnex(int annex) const
{
	BOOL bRes = FALSE;

	annexes_fd_set  annexesMask;
	annexesListEn eAnnex = (annexesListEn)annex;
	annexesMask.fds_bits[0] = m_annexes;

	BOOL bIsParameterOK = (eAnnex < H263_Annexes_Number);

	if (bIsParameterOK)
	{
		if (CAP_FD_ISSET(eAnnex, &(annexesMask)))
			bRes = TRUE;
	}

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////	* /
int CAdvanceVideo::DeSerializeXml(CXMLDOMElement *pFatherNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pActionNode, *pChildNode, *pAnnexNode;

	GET_CHILD_NODE(pFatherNode, "ADVANCED_VIDEO_PARAM", pActionNode);

	if (pActionNode)
	{
		nStatus = CPrtMontrGlobalParams::DeSerializeXml(pActionNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;

		GET_CHILD_NODE(pActionNode,"ANNEXES", pChildNode);
		if (pChildNode)
		{
			int num;
			GET_FIRST_CHILD_NODE(pChildNode, "ANNEX", pAnnexNode);

			annexes_fd_set tempMask;
			tempMask.fds_bits[0] = 0;
			while (pAnnexNode)
			{
				GET_VALIDATE(pAnnexNode, &num, ANNEXES_ENUM);
				//update the annexes
				AddAnnex(&tempMask,num);
				GET_NEXT_CHILD_NODE(pChildNode, "ANNEX", pAnnexNode);
			}
			m_annexes = tempMask.fds_bits[0];
		}
		GET_VALIDATE_CHILD(pActionNode,"RESOLUTION",&m_resolution,VIDEO_RESOLUTION_ENUM);
		GET_VALIDATE_CHILD(pActionNode,"VIDEO_FRAME_RATE",&m_frameRate,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"RESOLUTION_WIDTH",&m_resolutionWidth,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"RESOLUTION_HEIGHT",&m_resolutionHeight,_0_TO_WORD);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////	* /
void CAdvanceVideo::AddAnnex(annexes_fd_set *pTempMask,DWORD eAnnex) const
{
	BYTE bIsParameterOK = (eAnnex < H263_Annexes_Number);

	if (bIsParameterOK)
		CAP_FD_SET(eAnnex, pTempMask);
}

/////////////////////////////////////////////////////////////////////////////
void CAdvanceVideo::Dump1()
{
	CLargeString strBase1;
	strBase1<< " : \nm_annexes = " << m_annexes << "\nm_resolution = " << m_resolution
	<< "\nm_frameRate = " << m_frameRate << "\nm_resolutionWidth = " << m_resolutionWidth << "\nm_resolutionHeight = " << m_resolutionHeight;

	FPTRACE2(eLevelInfoNormal,"CAdvanceVideo::Dump ", strBase1.GetString());
	CPrtMontrGlobalParams::Dump1();
}

/////////////////////////////////////////////////////////////////////////////	* /
BYTE CAdvanceVideo::bIsResolutionOk(DWORD resolution)
{
/*m_resolution - is the resolution that we received from the card
  resolution - it the resolution from the channel.
  In case the m_resolution is bigger then the resolution we have in the channel it is a problem, we should check with the
  right order!
*/
	DWORD roundedUp_m_resolution = m_resolution;
	if(roundedUp_m_resolution == (DWORD)kUnknownFormat)
		roundedUp_m_resolution = CCapSetInfo::GetResolutionFormat(m_resolutionWidth, m_resolutionHeight);

	if ((roundedUp_m_resolution >=  kLastFormat) || (resolution >= kLastFormat ))
		return FALSE;

	BYTE bIsOk = TRUE;

	EOrderedFormat eReportedResolution = CCapSetInfo::ConvertFormatToOrderedFormat((EFormat)roundedUp_m_resolution);
	EOrderedFormat eChannelResolution  = CCapSetInfo::ConvertFormatToOrderedFormat((EFormat)resolution);

	if (roundedUp_m_resolution != (DWORD)kUnknownFormat && eReportedResolution > eChannelResolution)
		bIsOk = FALSE;

	return bIsOk;
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CAdvanceVideo::CheckExceedingFieldsRules(DWORD rate,DWORD protocol,DWORD annexes,
											   int resolution,unsigned short frameRate,CMedString &strExcFieldsRules)
{
//#ifdef __HIGHC__
	// get system.cfg values
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "THRESHOLD_FRAME_RATE";
	DWORD thresholdFrameRate = 12000;//::GetpSystemCfg()->GetThresholdFrameRate();
	pSysConfig->GetDWORDDataByKey(key, thresholdFrameRate);

	if((kUnknownFormat <= m_resolution && m_resolution < kLastFormat) &&
		(kUnknownFormat <= resolution && resolution < kLastFormat))
	{
		BYTE bIsOk = bIsResolutionOk(resolution);
		if(bIsOk == FALSE)
		{
			CCapSetInfo capInfo;
			strExcFieldsRules<<" Exceed resolution. Current: " << capInfo.GetFormatStr((EFormat)m_resolution);
			//::GetResolutionFormatName(m_resolution,strExcFieldsRules);
			strExcFieldsRules<<" is bigger than expected: " << capInfo.GetFormatStr((EFormat)resolution);
			//::GetResolutionFormatName(resolution,strExcFieldsRules);
			SetProblem(ResolutionProblem);
		}
	}
	else
	{
		strExcFieldsRules<<"The resolution exceed the number of known resolution.";
		strExcFieldsRules<<" m_resolution	= "<<m_resolution;
		strExcFieldsRules<<" resolution	= "<<resolution;
		SetProblem(ResolutionProblem);
		DBGPASSERT(m_resolution);
		DBGPASSERT(resolution);
	}


	if((thresholdFrameRate != 10000) &&
	   ((m_frameRate * (DWORD)10000) > (frameRate * thresholdFrameRate)))  //if the m_frameRate > 120%
	{
		strExcFieldsRules<<" Exceed frame rate. Current: "<< m_frameRate;
		strExcFieldsRules<<" ,expected: "<<frameRate;
		SetProblem(FrameRateProblem);
	}

	//For checking difference we need to check if the group of the union is remain the
	//current annexes
	BOOL bIsProblem = FALSE;
//	for(annexesListEn eAnnex = typeAnnexB; eAnnex < H263_Annexes_Number && !bIsProblem; eAnnex++)
//	{
//		if(IsContainAnnex((int)eAnnex, m_annexes) && (!IsContainAnnex((int)eAnnex, annexes)))
//		{
//			if(eAnnex != typeAnnexI)
//				bIsProblem = TRUE;
//		}
//	}
	DWORD eAnnex;
	for(eAnnex = typeAnnexB; eAnnex < H263_Annexes_Number && !bIsProblem; eAnnex++)
	{
		if(IsContainAnnex(eAnnex, m_annexes) && (!IsContainAnnex(eAnnex, annexes)))
		{
			if(eAnnex != typeAnnexI)
				bIsProblem = TRUE;
		}
	}

	if(bIsProblem)
	{
		strExcFieldsRules<<" Annexes problem. m_annexes = "<<m_annexes<<" annexes = "<<--annexes;
		strExcFieldsRules<<" Problematic annex is: "<<eAnnex;
		SetProblem(AnnexesProblem);
	}

	CPrtMontrGlobalParams::CheckExceedingFieldsRules(rate,protocol,strExcFieldsRules);
//#endif
}


/*************************************************************************************/
/*							CAdvanceVideoOut    									 */
/*************************************************************************************/

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceVideoOut::CAdvanceVideoOut(EIpChannelType channelType)
{
	m_channelType = channelType; //VIDEO_OUT or VIDEO_CONT_OUT
}
////////////////////////////////////////////////////
void CAdvanceVideoOut::CopyClass(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceVideoOut"))
	{
		*this = (CAdvanceVideoOut &)other;
	}
}


/////////////////////////////////////////////////////////////////////////////
void CAdvanceVideoOut::Dump1()
{

	PTRACE(eLevelInfoNormal,"CAdvanceVideoOut::Dump \n");
	CAdvanceVideo::Dump1();
}

/*************************************************************************************/
/*							CAdvanceVideoIn     									 */
/*************************************************************************************/

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceVideoIn::CAdvanceVideoIn(EIpChannelType channelType)
{
	m_channelType = channelType; //VIDEO_IN or VIDEO_CONT_IN
}

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceVideoIn::CAdvanceVideoIn(const CAdvanceVideoIn &other):CAdvanceVideo(other),CRtpInfo(other)
{
	m_errorResilience = other.m_errorResilience;
}

/////////////////////////////////////////////////////////////////////////////
/*void CAdvanceVideoIn::Dump12()
{
	PTRACE(eLevelInfoNormal,"CAdvanceVideoOut::Dump \n");
	m_errorResilience.Dump1();
}*/
/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceVideoIn& CAdvanceVideoIn::operator=(const CAdvanceVideoIn& other)
{
	if(this != &other)
	{
		m_errorResilience = other.m_errorResilience;

		(CAdvanceVideo &)*this = (CAdvanceVideo &)other;
		m_rtpPacketLoss			= other.m_rtpPacketLoss;
		m_rtpOutOfOrder			= other.m_rtpOutOfOrder;
		m_rtpFragmentPackets	= other.m_rtpFragmentPackets;
		m_jitterBufferSize		= other.m_jitterBufferSize;
		m_jitterLatePackets		= other.m_jitterLatePackets;
		m_jitterOverflows		= other.m_jitterOverflows;
		m_jitterSamplePacketInterval	= other.m_jitterSamplePacketInterval;
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CAdvanceVideoIn::operator==(const CAdvanceVideoIn& other)
{
	if(this == &other)
		return TRUE;

	if(	(CAdvanceVideo &)*this == (CAdvanceVideo &)other	&&
		m_errorResilience		== other.m_errorResilience	&&
		m_rtpPacketLoss			== other.m_rtpPacketLoss	&&
		m_rtpOutOfOrder			== other.m_rtpOutOfOrder	&&
		m_rtpFragmentPackets	== other.m_rtpFragmentPackets &&
		m_jitterBufferSize		== other.m_jitterBufferSize	&&
		m_jitterLatePackets		== other.m_jitterLatePackets &&
		m_jitterOverflows		== other.m_jitterOverflows	&&
		m_jitterSamplePacketInterval	== other.m_jitterSamplePacketInterval)
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CAdvanceVideoIn::operator!=(const CAdvanceVideoIn& other)
{
	return (!(*this == other));
}

///////////////////////////////////////////////////////////////////////////////////////
void CAdvanceVideoIn::CopyClass(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceVideoIn"))
	{
		*this = (CAdvanceVideoIn &)other;
	}
}

/////////////////////////////////////////////////////////////////////////////	*
DWORD CAdvanceVideoIn::IsEqual(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceVideoIn"))
		return (*this == (CAdvanceVideoIn &)other);
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////	* /
//#ifdef __HIGHC__
void  CAdvanceVideoIn::Serialize(WORD format,CSegment  &seg) const
{
	switch ( format )
	{

	case NATIVE     :
		{
			CAdvanceVideo::Serialize(format,seg);
			CRtpInfo::Serialize(format,seg);
			m_errorResilience.Serialize(format,seg);

			break;
		}
	default :
		break;
    }
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CAdvanceVideoIn::DeSerialize(WORD format,CSegment &seg)
{
	switch ( format )
	{

	case NATIVE     :
		{
			CAdvanceVideo::DeSerialize(format,seg);
			CRtpInfo::DeSerialize(format,seg);
			m_errorResilience.DeSerialize(format,seg);

			break;
		}
	default : {
		break;
			  }
    }
}
//#endif

/////////////////////////////////////////////////////////////////////////////	* /
void CAdvanceVideoIn::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pAdvanceVideoIn = pFatherNode->AddChildNode("ADVANCED_VIDEO_IN");
	CXMLDOMElement* pTempNode;

	CAdvanceVideo::SerializeXml(pAdvanceVideoIn);
	CRtpInfo::SerializeXml(pAdvanceVideoIn);
	pTempNode = pAdvanceVideoIn->AddChildNode("ERROR_RESILIENCE");
	m_errorResilience.SerializeXml(pTempNode);
}

/////////////////////////////////////////////////////////////////////////////	* /
int CAdvanceVideoIn::DeSerializeXml(CXMLDOMElement *pFatherNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pChildNode, *pActionNode;

	GET_CHILD_NODE(pFatherNode, "ADVANCED_VIDEO_IN", pActionNode);

	if (pActionNode)
	{
		nStatus = CAdvanceVideo::DeSerializeXml(pActionNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;

		GET_CHILD_NODE(pActionNode, "RTP_INFO", pChildNode);
		if (pChildNode)
		{
			nStatus = CRtpInfo::DeSerializeXml(pChildNode, pszError);
			if (nStatus!=STATUS_OK)
				return nStatus;
		}

		GET_CHILD_NODE(pActionNode, "ERROR_RESILIENCE", pChildNode);
		if (pChildNode)
		{
			nStatus = m_errorResilience.DeSerializeXml(pChildNode, pszError);
			if (nStatus!=STATUS_OK)
				return nStatus;
		}
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CAdvanceVideoIn::CheckExceedingFieldsRules(DWORD rate,DWORD protocol,DWORD annexes,
											DWORD resolution,DWORD frameRate,CMedString &strExcFieldsRules)
{
	DWORD mapProblem = CRtpInfo::CheckExceedingFieldsRules(strExcFieldsRules);
	CAdvanceVideo::CheckExceedingFieldsRules(rate,protocol,annexes,resolution,frameRate,strExcFieldsRules);
	SetProblem(mapProblem);
}


/*************************************************************************************/
/*							CAdvanceFeccOut     									 */
/*************************************************************************************/

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceFeccOut::CAdvanceFeccOut()
{
	m_channelType = FECC_OUT;
}
///////////////////////////////////
void CAdvanceFeccOut::CopyClass(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceFeccOut"))
	{
		*this = (CAdvanceFeccOut &)other;
	}
}
////////////////////////////////////////
void CAdvanceFeccOut::Dump1()
{
	PTRACE(eLevelInfoNormal,"CAdvanceFeccOut::Dump \n");
	CPrtMontrGlobalParams::Dump1();
}

/*************************************************************************************/
/*							CAdvanceFeccIn      									 */
/*************************************************************************************/

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceFeccIn::CAdvanceFeccIn()
{
	m_channelType = FECC_IN;
}

/////////////////////////////////////////////////////////////////////////////
void CAdvanceFeccIn::Dump1()
{
	PTRACE(eLevelInfoNormal,"CAdvanceFeccIn::Dump \n");
	PTRACE(eLevelInfoNormal,"m_rtpPacketLoss \n");
	m_rtpPacketLoss.Dump1();
	PTRACE(eLevelInfoNormal,"m_rtpOutOfOrder \n");
	m_rtpOutOfOrder.Dump1();
	PTRACE(eLevelInfoNormal,"m_rtpFragmentPackets \n");
	m_rtpFragmentPackets.Dump1();
	CPrtMontrGlobalParams::Dump1();
}
/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceFeccIn::CAdvanceFeccIn(const CAdvanceFeccIn &other):CPrtMontrGlobalParams(other)
{
	m_rtpPacketLoss = other.m_rtpPacketLoss;
	m_rtpOutOfOrder = other.m_rtpOutOfOrder;
	m_rtpFragmentPackets = other.m_rtpFragmentPackets;
}

/////////////////////////////////////////////////////////////////////////////	* /
CAdvanceFeccIn& CAdvanceFeccIn::operator=(const CAdvanceFeccIn& other)
{
	if(this != &other)
	{
		m_rtpPacketLoss			= other.m_rtpPacketLoss;
		m_rtpOutOfOrder			= other.m_rtpOutOfOrder;
		m_rtpFragmentPackets	= other.m_rtpFragmentPackets;

		(CPrtMontrGlobalParams &)*this = (CPrtMontrGlobalParams &)other;
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CAdvanceFeccIn::operator==(const CAdvanceFeccIn& other)
{
	if(this == &other)
		return TRUE;

	if(	(CPrtMontrGlobalParams &)*this == (CPrtMontrGlobalParams &)other &&
		m_rtpPacketLoss			== other.m_rtpPacketLoss	&&
		m_rtpOutOfOrder			== other.m_rtpOutOfOrder	&&
		m_rtpFragmentPackets	== other.m_rtpFragmentPackets)
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CAdvanceFeccIn::operator!=(const CAdvanceFeccIn& other)
{
	return (!(*this == other));
}

///////////////////////////////////////////////////////////////////////////////////////
void CAdvanceFeccIn::CopyClass(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceFeccIn"))
	{
		*this = (CAdvanceFeccIn &)other;
	}
}

/////////////////////////////////////////////////////////////////////////////	*
DWORD CAdvanceFeccIn::IsEqual(CPrtMontrBaseParams &other)
{
	if(!strcmp(other.NameOf(),"CAdvanceVideoIn"))
		return (*this == (CAdvanceFeccIn &)other);
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////	* /
//#ifdef __HIGHC__
void  CAdvanceFeccIn::Serialize(WORD format,CSegment  &seg) const
{
	switch ( format )
	{

	case NATIVE     :
		{
			CPrtMontrGlobalParams::Serialize(format,seg);
			m_rtpPacketLoss.Serialize(format,seg);
			m_rtpOutOfOrder.Serialize(format,seg);
			m_rtpFragmentPackets.Serialize(format,seg);

			break;
		}
	default :
		break;
    }
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CAdvanceFeccIn::DeSerialize(WORD format,CSegment &seg)
{
	switch ( format )
	{

	case NATIVE     :
		{
			CPrtMontrGlobalParams::DeSerialize(format,seg);
			m_rtpPacketLoss.DeSerialize(format,seg);
			m_rtpOutOfOrder.DeSerialize(format,seg);
			m_rtpFragmentPackets.DeSerialize(format,seg);

			break;
		}
	default : {
		break;
			  }
    }
}
//#endif

/////////////////////////////////////////////////////////////////////////////	* /
void CAdvanceFeccIn::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pAdvanceFeccIn = pFatherNode->AddChildNode("ADVANCED_FECC_IN");
	CXMLDOMElement* pTempNode;

	CPrtMontrGlobalParams::SerializeXml(pAdvanceFeccIn);
	pTempNode = pAdvanceFeccIn->AddChildNode("RTP_PACKET_LOSS");
	m_rtpPacketLoss.SerializeXml(pTempNode);
	pTempNode = pAdvanceFeccIn->AddChildNode("RTP_OUT_OF_ORDER");
	m_rtpOutOfOrder.SerializeXml(pTempNode);
	pTempNode = pAdvanceFeccIn->AddChildNode("RTP_FRAGMENT_PACKETS");
	m_rtpFragmentPackets.SerializeXml(pTempNode);
}

/////////////////////////////////////////////////////////////////////////////	* /
int CAdvanceFeccIn::DeSerializeXml(CXMLDOMElement *pFatherNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pChildNode, *pActionNode;

	GET_CHILD_NODE(pFatherNode, "ADVANCED_FECC_IN", pActionNode);

	if (pActionNode)
	{
		nStatus = CPrtMontrGlobalParams::DeSerializeXml(pActionNode, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;

		GET_CHILD_NODE(pActionNode, "RTP_PACKET_LOSS", pChildNode);
		if (pChildNode)
		{
			nStatus = m_rtpPacketLoss.DeSerializeXml(pChildNode, pszError);
			if (nStatus!=STATUS_OK)
				return nStatus;
		}

		GET_CHILD_NODE(pActionNode, "RTP_OUT_OF_ORDER", pChildNode);
		if (pChildNode)
		{
			nStatus = m_rtpOutOfOrder.DeSerializeXml(pChildNode, pszError);
			if (nStatus!=STATUS_OK)
				return nStatus;
		}

		GET_CHILD_NODE(pActionNode, "RTP_FRAGMENT_PACKETS", pChildNode);
		if (pChildNode)
		{
			nStatus = m_rtpFragmentPackets.DeSerializeXml(pChildNode, pszError);
			if (nStatus!=STATUS_OK)
				return nStatus;
		}
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////	* /
