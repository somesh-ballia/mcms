// rsrc.cpp : implementation of the CRsrc class
//
#include "NetHardwareInterface.h"
#include "Trace.h"
#include "Macros.h"
#include "MplMcmsProtocolTracer.h"
#include "Q931Structs.h"
#include "OpcodesRanges.h"  // for NET_ opcodes (until implemented by other sides)
#include "IsdnNetSetup.h"
#include "ConfPartyRoutingTable.h"
#include "TraceStream.h"
#include "AllocateStructs.h"
#include "IpCmReq.h"
#include "StatusesGeneral.h"

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );


/////////////////////////////////////////////////////////////////////////////
CNetHardwareInterface::CNetHardwareInterface()  	// constructor
  :m_netConnId(0),
   m_audioDecoderConnId(0),
   m_audioEncoderConnId(0)
{
  // m_firstPort = 0;
  // 	m_netConnectionId = 0;
  // 	m_nfasId = 0;
  // 	m_numPorts = 1;
}
/////////////////////////////////////////////////////////////////////////////
CNetHardwareInterface::~CNetHardwareInterface()     // destructor
{
}
/////////////////////////////////////////////////////////////////////////////
void  CNetHardwareInterface::Create(CRsrcParams &rRsrcParams)
{
  POBJDELETE(m_pRsrcParams);
  m_pRsrcParams = new CRsrcParams(rRsrcParams);
  eLogicalResourceTypes lrt = eLogical_net;

  CRsrcDesc* pNetRsrcDesc =  m_pRsrcParams->GetRsrcDesc();//::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(GetPartyRsrcId(),lrt);
  CRsrcDesc* pEncoderRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(GetPartyRsrcId(), eLogical_audio_encoder);
  CRsrcDesc* pDecoderRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(GetPartyRsrcId(), eLogical_audio_decoder);

  if(!pNetRsrcDesc || !pEncoderRsrcDesc || !pDecoderRsrcDesc)
    {
      TRACESTR (eLevelError) << "CNetChnlCntl::OnConnectHardwareIdle Could not get all Rsrc connections ID!";
      DBGPASSERT(101);
      return;
    }
  m_netConnId = pNetRsrcDesc->GetConnectionId();
  m_audioDecoderConnId=pDecoderRsrcDesc->GetConnectionId();
  m_audioEncoderConnId=pEncoderRsrcDesc->GetConnectionId();
}

/////////////////////////////////////////////////////////////////////////////
void  CNetHardwareInterface::Setup(CIsdnNetSetup& netSetup)
{
  NET_SETUP_REQ_S * pSetupStruct= new NET_SETUP_REQ_S;
  memset(pSetupStruct,0,sizeof(NET_SETUP_REQ_S));

  pSetupStruct->net_spfc=netSetup.m_netSpcf;
  pSetupStruct->call_type=netSetup.m_callType;

  //Getting the phone numbers
  WORD numDigits = 0;

  //Calling Party
  char callingNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
  netSetup.GetCallingNumber(&numDigits, callingNumber);
  pSetupStruct->calling_party.num_digits = numDigits;
  memcpy(&(pSetupStruct->calling_party.digits) , callingNumber,numDigits);
  pSetupStruct->calling_party.num_type=GetNumTypeAsApiVal(eDfltNumType(netSetup.m_calling.m_numType));
  pSetupStruct->calling_party.num_plan=GetNumPlanAsApiVal(eNumPlanType(netSetup.m_calling.m_numPlan));

  //Called Party
  char calledNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
  netSetup.GetCalledNumber(&numDigits, calledNumber);
  pSetupStruct->called_party.num_digits = numDigits;
  memcpy(&(pSetupStruct->called_party.digits) , calledNumber , numDigits);
  pSetupStruct->called_party.num_type=GetNumTypeAsApiVal(eDfltNumType(netSetup.m_called.m_numType));
  pSetupStruct->called_party.num_plan=GetNumPlanAsApiVal(eNumPlanType(netSetup.m_called.m_numPlan));

  SendMsgToMPL((BYTE*)(pSetupStruct),sizeof(NET_SETUP_REQ_S),NET_SETUP_REQ,netSetup);

  PDELETE(pSetupStruct);
}
/////////////////////////////////////////////////////////////////////////////
void  CNetHardwareInterface::Clear(CNetCause& cause,const CIsdnNetSetup & netSetup)
{
  // disconnect line
  NET_CLEAR_REQ_S clearReqStruct;
  memset(&clearReqStruct,0,sizeof(NET_CLEAR_REQ_S));

  //Set the Cause struct
  clearReqStruct.cause.cause_val=cause.m_causeVal;

  SendMsgToMPL((BYTE*)(&clearReqStruct),sizeof(NET_CLEAR_REQ_S),NET_CLEAR_REQ,netSetup);
}

/////////////////////////////////////////////////////////////////////////////
// In oreder to Fill the Header we wrapped SendMsgToMPL
/////////////////////////////////////////////////////////////////////////////
void  CNetHardwareInterface::SendMsgToMPL(BYTE* pStructure, int structureSize,
										  DWORD opcode, const CIsdnNetSetup& netSetup)
{
 	if(!m_pRsrcParams)
 	{
 		PASSERT(101);
 		return;
 	}
 	CRsrcDesc* pNetRsrcDesc = m_pRsrcParams->GetRsrcDesc();
 	if(!pNetRsrcDesc)
 	{
 		PASSERT(102);
 		return;
 	}
	if (NET_SETUP_REQ == opcode)
	  FillNetSetupHeader(pStructure,structureSize,netSetup);
	else
	  FillNetHeader(pStructure,structureSize,netSetup);

	CSegment* pSegParam  = new CSegment;
	pSegParam->Put(pStructure , structureSize);

	//Send to Hardwarte
	CHardwareInterface::SendMsgToMPL(opcode,pSegParam);

 	POBJDELETE(pSegParam);
}

// ///////////////////////////////////////////////////////////////////////////
// DWORD CNetHardwareInterface::GetNetConnectionId() const
// {
// 	return m_netConnectionId;
// }
// /////////////////////////////////////////////////////////////////////////////
// WORD CNetHardwareInterface::GetNfasId() const
// {
// 	return m_nfasId;
// }
// /////////////////////////////////////////////////////////////////////////////
// WORD CNetHardwareInterface::GetNumPorts() const
// {
// 	return m_numPorts;
// }
// /////////////////////////////////////////////////////////////////////////////
// WORD CNetHardwareInterface::GetFirstPort() const
// {
// 	return m_firstPort;
// }
// ///////////////////////////////////////////////////////////////////////////
// void CNetHardwareInterface::SetNetConnectionId(WORD netConnectionId)
// {
// 	m_netConnectionId = netConnectionId;
// }
// /////////////////////////////////////////////////////////////////////////////
// void CNetHardwareInterface::SetNfasId(WORD nfasId)
// {
// 	m_nfasId = nfasId;
// }
// /////////////////////////////////////////////////////////////////////////////
// void CNetHardwareInterface::SetNumPorts(WORD numOfPorts)
// {
// 	m_numPorts = numOfPorts;
// }
// /////////////////////////////////////////////////////////////////////////////
// void CNetHardwareInterface::SetFirstPort(WORD firstPort)
// {
// 	m_firstPort = firstPort;
// }

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::Alert(const CIsdnNetSetup & netSetup)
{
  NET_ALERT_REQ_S * pAlertStruct= new NET_ALERT_REQ_S;
  memset(pAlertStruct,0,sizeof(NET_ALERT_REQ_S));
  SendMsgToMPL((BYTE*)(pAlertStruct),sizeof(NET_ALERT_REQ_S),NET_ALERT_REQ,netSetup);
  PDELETE(pAlertStruct);
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::Connect(const CIsdnNetSetup & netSetup)
{
  NET_CONNECT_REQ_S * pConnectReq= new NET_CONNECT_REQ_S;
  memset(pConnectReq,0,sizeof(NET_CONNECT_REQ_S));
  SendMsgToMPL((BYTE*)(pConnectReq),sizeof(NET_CONNECT_REQ_S),NET_CONNECT_REQ,netSetup);
  PDELETE(pConnectReq);
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::DisconnectReq(CNetCause& cause,const CIsdnNetSetup & netSetup)
{
  NET_DISCONNECT_ACK_REQ_S disccAckReq;
  memset(&disccAckReq,0,sizeof(NET_DISCONNECT_ACK_REQ_S));
  disccAckReq.cause.cause_val=cause.m_causeVal;
  SendMsgToMPL((BYTE*)(&disccAckReq),sizeof(NET_DISCONNECT_ACK_REQ_S),NET_DISCONNECT_ACK_REQ,netSetup);
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::ConnectNetToAudioMux()
{
  TRACESTR (eLevelInfoNormal) << "CNetHardwareInterface::ConnectNetToAudio";
  SendOpenPort(kRtmChnlType,cmCapReceive,m_netConnId);
  SendOpenPort(kRtmChnlType,cmCapTransmit,m_netConnId);
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::ConnectAudioToNet()
{
  TRACESTR (eLevelInfoNormal) << "CNetHardwareInterface::ConnectAudioToNet";
  SendOpenPort(kPstnAudioChnlType,cmCapReceive,m_audioDecoderConnId);
  SendOpenPort(kPstnAudioChnlType,cmCapTransmit,m_audioEncoderConnId);
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::ConnectMuxToNet()
{
  TRACESTR (eLevelInfoNormal) << "CNetHardwareInterface::ConnectMuxToNet";
  SendOpenPort(kIsdnMuxChnlType,cmCapReceive,m_audioDecoderConnId);
  SendOpenPort(kIsdnMuxChnlType,cmCapTransmit,m_audioEncoderConnId);
}
/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::DisconnectNetFromAudio(cmCapDirection direction)
{
  SendClosePort(kRtmChnlType,direction,m_netConnId);
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::DisconnectAudioFromNet(cmCapDirection direction)
{
  if (cmCapTransmit == direction)
    SendClosePort(kPstnAudioChnlType,direction,m_audioEncoderConnId);
  else
    SendClosePort(kPstnAudioChnlType,direction,m_audioDecoderConnId);
}
/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::DisconnectMuxFromNet(cmCapDirection direction)
{
  TRACESTR (eLevelInfoNormal) << "CNetHardwareInterface::DisconnectMuxFromNet";
  if (cmCapTransmit == direction)
    SendClosePort(kIsdnMuxChnlType, direction, m_audioEncoderConnId);
  else
    SendClosePort(kIsdnMuxChnlType, direction, m_audioDecoderConnId);
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::SendClosePort(kChanneltype channelType,cmCapDirection direction,DWORD connectionId)
{
  TCloseUdpPortMessageStruct* pStruct = new TCloseUdpPortMessageStruct;
  memset(pStruct,0,sizeof(TCloseUdpPortMessageStruct));

  mcReqCmCloseUdpPort* pUdpSt = &pStruct->tCmCloseUdpPort;
  pUdpSt->channelType	        = channelType;
  pUdpSt->channelDirection    = direction;

  pStruct->physicalPort.connection_id = connectionId;
  pStruct->physicalPort.party_id = GetPartyRsrcId();



  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE *)(pStruct),sizeof(TCloseUdpPortMessageStruct));
  CHardwareInterface::SendMsgToMPL(CONFPARTY_CM_CLOSE_UDP_PORT_REQ,pMsg);
  POBJDELETE(pMsg);
  PDELETE(pStruct);
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::SendOpenPort(kChanneltype channelType,cmCapDirection direction,DWORD connectionId)
{
  TOpenUdpPortOrUpdateUdpAddrMessageStruct* pStruct = new TOpenUdpPortOrUpdateUdpAddrMessageStruct;
  memset(pStruct,0,sizeof(TOpenUdpPortOrUpdateUdpAddrMessageStruct));

  mcReqCmOpenUdpPortOrUpdateUdpAddr* pUdpSt  = &pStruct->tCmOpenUdpPortOrUpdateUdpAddr;

  pUdpSt->channelType = channelType;
  pUdpSt->channelDirection = direction;
  pStruct->physicalPort.connection_id = connectionId;
  pStruct->physicalPort.party_id = GetPartyRsrcId();


  //fill fields for both local and remote addresses
  pUdpSt->CmLocalUdpAddressIp.distribution  = eDistributionUnicast;
  pUdpSt->CmRemoteUdpAddressIp.distribution = eDistributionUnicast;

  pUdpSt->CmLocalUdpAddressIp.ipVersion=eIpVersion4;
  pUdpSt->CmRemoteUdpAddressIp.ipVersion =eIpVersion4;

  pUdpSt->CmLocalUdpAddressIp.transportType = eTransportTypeUdp;
  pUdpSt->CmRemoteUdpAddressIp.transportType = eTransportTypeUdp;

  pUdpSt->ice_channel_rtcp_id = 0;
  pUdpSt->ice_channel_rtp_id = 0;

  pUdpSt->capProtocolType = eUnknownAlgorithemCapCode; // Currently there is no use for this field in ISDN

  // TIP
  //pUdpSt->bIsTipEnable = FALSE;

  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE *)(pStruct),sizeof(TOpenUdpPortOrUpdateUdpAddrMessageStruct));
  CHardwareInterface::SendMsgToMPL(CONFPARTY_CM_OPEN_UDP_PORT_REQ,pMsg);
  POBJDELETE(pMsg);

  PDELETE(pStruct);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CNetHardwareInterface::GetAckIndParams(kChanneltype& channelType, cmCapDirection& direction, APIU32& ackReason, CSegment* pParam)
{
  ACK_IND_S ackIndStruct;

  *pParam >> ackIndStruct.ack_base.ack_opcode
          >> ackIndStruct.ack_base.ack_seq_num
          >> ackIndStruct.ack_base.status
          >> ackIndStruct.ack_base.reason
          >> ackIndStruct.media_type
          >> ackIndStruct.media_direction;

  channelType = (kChanneltype)ackIndStruct.media_type;
  direction   = (cmCapDirection)ackIndStruct.media_direction;
  ackReason   = ackIndStruct.ack_base.reason;

  return (ackIndStruct.ack_base.status);
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::ClosePort(kChanneltype channelType,cmCapDirection direction)
{
  if (kRtmChnlType == channelType) //close RTM ports
    DisconnectNetFromAudio(direction);
  else if(kPstnAudioChnlType == channelType)  //close ART ports
	DisconnectAudioFromNet(direction);
  else //close MUX ports
	DisconnectMuxFromNet(direction);
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CNetHardwareInterface::GetNumTypeAsApiVal(eDfltNumType numType)const
{
  APIU32 retVal = ACU_NB_TYPE_UNKNOWN;

  if ( eDfltNumTypeInternational == numType )
    retVal = ACU_NB_TYPE_INTERNATIONAL;
  else if (eDfltNumTypeNational== numType )
    retVal = ACU_NB_TYPE_NATIONAL;
  else if (eDfltNumTypeSubscriber== numType )
    retVal = ACU_NB_TYPE_SUBSCRIBER;
  else if (eDfltNumTypeAbbreviated == numType )
    retVal = ACU_NB_TYPE_ABBREVIATED;

  return retVal;
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CNetHardwareInterface::GetNumPlanAsApiVal(eNumPlanType numPlan)const
{
  APIU32 retVal =ACU_NB_PLAN_UNKNOWN;

  if ( eNumPlanTypeUnknown == numPlan)
    retVal =ACU_NB_PLAN_UNKNOWN;
  else if ( eNumPlanTypeIsdn == numPlan )
    retVal = ACU_NB_PLAN_ISDN;
  else if ( eNumPlanTypeNational == numPlan )
    retVal = ACU_NB_PLAN_TELEPHONE;
  else if ( eNumPlanTypePrivate == numPlan )
    retVal = ACU_NB_PLAN_PRIVATE;

  return retVal;
}

/////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::FillNetHeader(BYTE* pStructure, int structureSize,const CIsdnNetSetup& netSetup)
{
  //Get Net Common Header
  NET_COMMON_PARAM_S * pCommNetParams = (NET_COMMON_PARAM_S * )(pStructure);

  //Set Common Net Params
  pCommNetParams->span_id=netSetup.m_spanId[0];                         //Span ID
  pCommNetParams->net_connection_id=netSetup.m_net_connection_id;       //Set by card in dial in
  pCommNetParams->physical_port_number=netSetup.m_physical_port_number; //Set by card (Physical Port ID)
}

//////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::FillNetSetupHeader(BYTE* pStructure, int structureSize,const CIsdnNetSetup& netSetup)
{
  //Get Net Common Header
  NET_SETUP_REQ_HEADER_S* pCommNetParams = (NET_SETUP_REQ_HEADER_S*)(pStructure);

  //Set Common Net Params
  for (BYTE i=0; i<MAX_NUM_SPANS_ORDER; i++)
	  pCommNetParams->spans_order[i]=netSetup.m_spanId[i];              //Span ID
  pCommNetParams->net_connection_id=netSetup.m_net_connection_id;       //Set by card in dial in
  pCommNetParams->physical_port_number=netSetup.m_physical_port_number; //Set by card (Physical Port ID)
}
////////////////////////////////////////////////////////////////////////////
void CNetHardwareInterface::SendMsgWithPhysicalInfo(OPCODE opcode,CRsrcParams & rsrcParams,
						    const CIsdnNetSetup & netSetup, WORD discause)

{
  BYTE* pStructure=0;
  int structureSize=0;
  STATUS status = STATUS_OK;
  CAUSE_DATA_S* pCauseData=NULL;

  TRACESTR(eLevelInfoNormal) << "CNetHardwareInterface::SendMsgWithPhysicalInfo " ;

  CMplMcmsProtocol *pMplMcmsProtocol= new CMplMcmsProtocol(); AUTO_DELETE(pMplMcmsProtocol);

  pMplMcmsProtocol->AddCommonHeader(opcode);
  pMplMcmsProtocol->AddMessageDescriptionHeader();

  //Filling the physical header
  pMplMcmsProtocol->AddPhysicalHeader(netSetup.m_box_id,netSetup.m_boardId,netSetup.m_sub_board_id,
			      0, 0xFFFF, 0/*accelerator_id*/,ePhysical_rtm);

  pMplMcmsProtocol->AddPortDescriptionHeader(rsrcParams.GetPartyRsrcId(),
					     rsrcParams.GetConfRsrcId(),
					     rsrcParams.GetConnectionId(),
					     rsrcParams.GetLogicalRsrcType());

  if ( NET_CLEAR_REQ == opcode)
    {
      pStructure = (BYTE *) (new NET_CLEAR_REQ_S);
      structureSize = sizeof(NET_CLEAR_REQ_S);
    }
  else if (NET_DISCONNECT_ACK_REQ == opcode)
    {
      pStructure = (BYTE *) (new NET_DISCONNECT_ACK_REQ_S);
      structureSize = sizeof(NET_DISCONNECT_ACK_REQ_S);
    }
  else
    {
      TRACESTR (eLevelError) << "CNetHardwareInterface::SendMsgWithPhysicalInfo Unknowkn OPCODE!";
      PASSERT_AND_RETURN(1);
    }

  memset(pStructure ,0,structureSize);
  FillNetHeader(pStructure,structureSize,netSetup);
  pCauseData = (CAUSE_DATA_S*)(pStructure + sizeof(NET_COMMON_PARAM_S));
  pCauseData->cause_val = discause;

  CSegment* pSegParam  = new CSegment;
  pSegParam->Put(pStructure , structureSize);

  DWORD nMsgLen = pSegParam->GetWrtOffset() - pSegParam->GetRdOffset();
  BYTE* pMessage = new BYTE[nMsgLen];
  pSegParam->Get(pMessage,nMsgLen);
  pMplMcmsProtocol->AddData(nMsgLen,(const char*)pMessage);
  PDELETEA(pMessage);


  CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("***CNetHardwareInterface::SendMsgToMPL");
  status = pMplMcmsProtocol->SendMsgToMplApiCommandDispatcher();
  PASSERT(status);

  POBJDELETE(pMplMcmsProtocol);
  POBJDELETE(pSegParam);
  POBJDELETE(pStructure);
}
