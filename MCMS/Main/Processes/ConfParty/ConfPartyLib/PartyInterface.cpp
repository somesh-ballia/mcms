// rsrc.cpp : implementation of the CRsrc class
//
#include "PartyInterface.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "TBStructs.h"
#include "HostCommonDefinitions.h"
#include "ArtRequestStructs.h"
#include "ConfPartyGlobals.h"
#include "IpRtpReq.h"

/////////////////////////////////////////////////////////////////////////////
CPartyInterface::CPartyInterface()  	// constructor
{
}
/////////////////////////////////////////////////////////////////////////////
CPartyInterface::CPartyInterface(DWORD partyRsrcId, DWORD confRsrcId)
{
	m_pRsrcParams = new CRsrcParams(DUMMY_CONNECTION_ID, partyRsrcId, confRsrcId);
}

/////////////////////////////////////////////////////////////////////////////
CPartyInterface::~CPartyInterface()     // destructor
{
}
/////////////////////////////////////////////////////////////////////////////
CPartyInterface::CPartyInterface(const CPartyInterface& rhs)
:CHardwareInterface(rhs)
{
}
/////////////////////////////////////////////////////////////////////////////
DWORD  CPartyInterface::SendCreateParty(ENetworkType networkType, DWORD maxVideoTxBitsPer10ms, BYTE bIsMrcCall, eConfMediaType aMediaType, BOOL isEnableHighVideoResInAvcToSvcMixMode)
{
  TOpenArtReq* pOpenArtReq = new TOpenArtReq;
  memset(pOpenArtReq, 0, sizeof(TOpenArtReq));
  pOpenArtReq->enNetworkType = networkType;
  pOpenArtReq->unVideoTxMaxNumberOfBitsPer10ms = maxVideoTxBitsPer10ms;
  pOpenArtReq->ConnectContent = bIsMrcCall;

  pOpenArtReq->nMediaMode = eMediaModeTranscoding;
  if (bIsMrcCall)
  {
      pOpenArtReq->nMediaMode = eMediaModeRelayOnly;
      if (aMediaType == eMixAvcSvc)
          pOpenArtReq->nMediaMode = eMediaModeRelayAndTranscoding;
  }
  else
  {
      pOpenArtReq->nMediaMode = eMediaModeTranscoding;
      if (aMediaType == eMixAvcSvcVsw)
          pOpenArtReq->nMediaMode = eMediaModeRelayOnly;
      if (aMediaType == eMixAvcSvc && isEnableHighVideoResInAvcToSvcMixMode)
      {
   		  pOpenArtReq->nMediaMode = eMediaModeTranscodingAndVsw;
      }
  }

  CSegment* pSegParam  = new CSegment;
  pSegParam->Put((BYTE*)(pOpenArtReq), sizeof(TOpenArtReq));

  DWORD reqId = SendMsgToMPL(CONF_MPL_CREATE_PARTY_REQ, pSegParam);

  POBJDELETE(pSegParam);
  PDELETE(pOpenArtReq);

  return reqId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CPartyInterface::SendConnect(DWORD ConnectionID1, DWORD ConnectionID2,DWORD PartyID1, DWORD PartyID2)
{
	TB_MSG_CONNECT_S tConnecttStruct;
	memset(&tConnecttStruct,0,sizeof(TB_MSG_CONNECT_S));
	tConnecttStruct.physical_port1.connection_id = ConnectionID1;
	tConnecttStruct.physical_port1.party_id = PartyID1;
	tConnecttStruct.physical_port2.connection_id = ConnectionID2;
	tConnecttStruct.physical_port2.party_id =PartyID2;

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tConnecttStruct),sizeof(TB_MSG_CONNECT_S));

	DWORD reqId = SendMsgToMPL(TB_MSG_CONNECT_REQ, pMsg);

	POBJDELETE(pMsg);

	return reqId;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CPartyInterface::SendDeleteParty(DWORD isNeedToCollectInfoFromArt)
{
  TCloseArtReq* pCloseArtReq = new TCloseArtReq;
  memset(pCloseArtReq, 0, sizeof(TCloseArtReq));

  pCloseArtReq->bIsNeedToCollectInfoFromArt = isNeedToCollectInfoFromArt;

  CSegment* pSegParam  = new CSegment;
  pSegParam->Put((BYTE*)(pCloseArtReq),sizeof(TCloseArtReq));

  DWORD reqId = SendMsgToMPL(CONF_MPL_DELETE_PARTY_REQ, pSegParam);

  POBJDELETE(pSegParam);
  PDELETE(pCloseArtReq);

  return reqId;
}
