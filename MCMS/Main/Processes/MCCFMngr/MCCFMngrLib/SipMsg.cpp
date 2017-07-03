#include "MplMcmsProtocol.h"
#include "SystemFunctions.h"

#include "DataTypes.h"

#include "Trace.h"
#include "TraceStream.h"

#include "HostCommonDefinitions.h"
#include "IpCsOpcodes.h"
#include "SipCsInd.h"
#include "SipCsReq.h"
#include "ProcessBase.h"
#include "ConfigHelper.h"
#include "SysConfigKeys.h"

#include "SipMsg.h"

////////////////////////////////////////////////////////////////////////////
SipCallID::SipCallID(const CMplMcmsProtocol& protocol)
	: callID(protocol.getCentralSignalingHeaderCallIndex())
	, serviceID(protocol.getCentralSignalingHeaderCsId())
	, unitID(protocol.getCentralSignalingHeaderSrcUnitId())
{
}

////////////////////////////////////////////////////////////////////////////
SipCallID& SipCallID::operator =(const CMplMcmsProtocol& protocol)
{
	callID = protocol.getCentralSignalingHeaderCallIndex();
	serviceID = protocol.getCentralSignalingHeaderCsId();
	unitID = protocol.getCentralSignalingHeaderSrcUnitId();

	return *this;
}

////////////////////////////////////////////////////////////////////////////
bool operator <(const SipCallID& a, const SipCallID& b)
{
	return
		a.callID < b.callID ||
		(a.callID == b.callID && a.serviceID < b.serviceID) ||
		(a.callID == b.callID && a.serviceID == b.serviceID && a.unitID < b.unitID);
}

////////////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const SipCallID& obj)
{
	seg.Put(reinterpret_cast<const BYTE*>(&obj), sizeof(obj));
	return seg;
}

////////////////////////////////////////////////////////////////////////////
CSegment& operator >>(CSegment& seg, SipCallID& obj)
{
	seg.Get(reinterpret_cast<BYTE*>(&obj), sizeof(obj));
	return seg;
}

////////////////////////////////////////////////////////////////////////////
enSipCodes SetDialInSessionTimerHeaders(CSipHeaderList* headerList);

////////////////////////////////////////////////////////////////////////////
bool ParseCsInviteIndEvent(const CMplMcmsProtocol& protocol, CFWID cfwID, mcTransportAddress& appServerAddress, SipCallID& callID)
{
	callID = protocol;

	mcIndInvite* pInviteMsg = (mcIndInvite*)(protocol.getpData());

	const sipSdpAndHeadersSt* apSdpAndHeaders = &pInviteMsg->sipSdpAndHeaders;
	int sipMediaLinesOffset = apSdpAndHeaders->sipMediaLinesOffset;
	sipMediaLinesEntrySt* pMediaEntry = (sipMediaLinesEntrySt*)(&apSdpAndHeaders->capsAndHeaders[sipMediaLinesOffset]);

	if (pMediaEntry->numberOfMediaLines > 0)
	{
		sipMediaLineSt* apMediaLine = (sipMediaLineSt *) &pMediaEntry->mediaLines[0];
		capBuffer* pCapBuffer = (capBuffer *) &apMediaLine->caps[0];
		mccfCapStruct* pMCCFCap = (mccfCapStruct*)pCapBuffer->dataCap;

		strncpy(cfwID, (const char*)&pMCCFCap->cfw_id, CFW_ID_LEN);
		appServerAddress = apMediaLine->mediaIp.transAddr;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Invite Response 200 OK
bool BuildSipInviteResponseReq(const CMplMcmsProtocol& protocol, enSipCodes inStatus, const mcTransportAddress& hostIpAddress, CSegment& segOut, APIU32& opcode)
{
	enSipCodes statusToSend = inStatus;
	opcode = SIP_CS_MCCF_SIG_INVITE_RESPONSE_REQ;

	mcIndInvite* pInviteMsg = (mcIndInvite*)(protocol.getpData());
	if (!pInviteMsg)
		return false;

	unsigned char aBuffer[5000];
	memset(&aBuffer,'\0', sizeof(aBuffer));
	mcReqInviteResponse* pInviteResponseMsg = (mcReqInviteResponse*)(aBuffer);

	const sipSdpAndHeadersSt* apSdpAndHeadersInv = &pInviteMsg->sipSdpAndHeaders;
	DWORD sdpAndHeadersLen = sizeof(sipSdpAndHeadersBaseSt) + apSdpAndHeadersInv->lenOfDynamicSection;
	DWORD lengthStructure = sizeof(mcReqInviteResponse) + sdpAndHeadersLen;
	memcpy((void*)(&pInviteResponseMsg->sipSdpAndHeaders),apSdpAndHeadersInv,sdpAndHeadersLen);

	sipSdpAndHeadersSt* apSdpAndHeaders = &pInviteResponseMsg->sipSdpAndHeaders;

	// we do not want to send "Bandwidth information"
	apSdpAndHeaders->callRate = 0;

	//Set setupMode in MCCFCaps to ePassive
	if (statusToSend == SipCodesOk && apSdpAndHeaders->sipMediaLinesLength > 0 && apSdpAndHeaders->sipHeadersOffset > 0) //at least one cap exist
	{
		int sipMediaLinesOffset = apSdpAndHeaders->sipMediaLinesOffset;
		sipMediaLinesEntrySt *pMediaEntry = NULL;
		sipMediaLineSt *apMediaLine = NULL;

		pMediaEntry = (sipMediaLinesEntrySt*)(&apSdpAndHeaders->capsAndHeaders[sipMediaLinesOffset]);
		if (pMediaEntry->bMainMediaIpPresent)
			pMediaEntry->mainMediaIp.transAddr = hostIpAddress;

		if (pMediaEntry->numberOfMediaLines > 0)
		{
			apMediaLine = (sipMediaLineSt *) &pMediaEntry->mediaLines[0];
			capBuffer* pCapBuffer = (capBuffer *) &apMediaLine->caps[0];
			mccfCapStruct* pMCCFCap = (mccfCapStruct*)pCapBuffer->dataCap;

			pMCCFCap->setupMode = ePassive;
			pMCCFCap->connectionType = eNewConn;

			apMediaLine->mediaIp.transAddr = hostIpAddress;
		}
	}

	//Remove Contact header
	sipMessageHeaders* pHeaders = (sipMessageHeaders*)((char*)apSdpAndHeaders->capsAndHeaders + apSdpAndHeaders->sipHeadersOffset);
	CSipHeaderList* pHeaderList = new CSipHeaderList(*pHeaders);
	pHeaderList->RemoveNextHeader(kContact);
	pHeaderList->RemoveNextHeader(kSupported);

	//Set Session timers
	if (statusToSend == SipCodesOk)
	{
		enSipCodes sessionTimerStatus = SetDialInSessionTimerHeaders(pHeaderList);
		statusToSend = (sessionTimerStatus == SipCodesSipUnknownStatus) ? statusToSend : sessionTimerStatus;
	}

	pInviteResponseMsg->status = (statusToSend == SipCodesOk)?STATUS_OK:statusToSend;

	//Build message
	pHeaderList->BuildMessage(pHeaders);
	POBJDELETE(pHeaderList);

	size_t size = sizeof(mcReqInviteResponse) + pInviteResponseMsg->sipSdpAndHeaders.lenOfDynamicSection;
	segOut.Put((BYTE*)pInviteResponseMsg, size);

	if (inStatus == SipCodesOk && statusToSend != SipCodesOk)
	{
		//Send Error back to CS
		SipCallID callID(protocol);
		SendSipMsgToCS(opcode, segOut, callID);
		return false;
	}

	return true;
}

void SendSipMsgToCS(OPCODE opcode, CSegment& seg, const SipCallID& callID)
{
	CMplMcmsProtocol protocol;

	protocol.AddCommonHeader(opcode);
	protocol.AddMessageDescriptionHeader();
	protocol.AddPortDescriptionHeader(-1, -1, MCCF_CONNECTION_ID);

	const WORD csId = callID.serviceID;
	const APIS32 status = 0;
	const DWORD channelIndex = 0;
	const DWORD mcChannelIndex = 0;

	protocol.AddCSHeader(csId, 0, callID.unitID, callID.callID, callID.serviceID, channelIndex, mcChannelIndex, status);

	seg.ResetRead();
	const DWORD nMsgLen = seg.GetWrtOffset() - seg.GetRdOffset();
	protocol.AddData(nMsgLen, (const char*)seg.GetPtr(true));
	protocol.AddPayload_len(CS_API_TYPE);

	STATUS stat = protocol.SendMsgToCSApiCommandDispatcher();
}

bool ParseCsByeIndEvent(const CMplMcmsProtocol& protocol, DWORD& outStatus, SipCallID& callID)
{
	callID = protocol;

	mcIndBye* pByeMsg = (mcIndBye*)(protocol.getpData());
	outStatus = pByeMsg->status;

	if (outStatus != STATUS_OK && (outStatus < LOW_REJECT_VAL || outStatus >= HIGH_REJECT_VAL)) // something is wrong
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BuildSipBye200OKReq(CSegment& segOut, APIU32& opcode)
{
	opcode = SIP_CS_MCCF_SIG_BYE_200_OK_REQ;

	mcReqBye200Ok oBye200OkMsg;
	size_t size = sizeof(mcReqBye200Ok);
	memset(&oBye200OkMsg, 0, size);
	segOut.Put((BYTE*)&oBye200OkMsg, size);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParseCsBye200OKIndEvent(const CMplMcmsProtocol& protocol, DWORD& outStatus, SipCallID& callID)
{
	callID = protocol;

	mcIndBye200Ok* pBye200OkMsg = (mcIndBye200Ok*)(protocol.getpData());
	outStatus = pBye200OkMsg->status;

	if (outStatus != STATUS_OK && (outStatus < LOW_REJECT_VAL || outStatus >= HIGH_REJECT_VAL)) // something is wrong
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BuildSipByeReq(CSegment& segOut, APIU32& opcode)
{
	opcode = SIP_CS_MCCF_SIG_BYE_REQ;

	mcReqBye oByeMsg;
	size_t size = sizeof(mcReqBye);
	memset(&oByeMsg, 0, size);

	segOut.Put((const BYTE*)&oByeMsg, size);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParseCsInviteAckIndEvent(const CMplMcmsProtocol& protocol, DWORD& outStatus, SipCallID& callID)
{
	callID = protocol;

	mcIndInviteAck* pInviteAckMsg = (mcIndInviteAck*) (protocol.getpData());
	outStatus = pInviteAckMsg->status;

	if (outStatus != STATUS_OK && (outStatus < LOW_REJECT_VAL || outStatus >= HIGH_REJECT_VAL)) // something is wrong
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
enSipCodes SetDialInSessionTimerHeaders(CSipHeaderList* headerList)
{
	char cHeaderValue[256] = {0};
	CSipHeader* pSessionExpireHeader = NULL;
	DWORD RmtSessionTimer = 0;

	if (!headerList)
		return SipCodesSipUnknownStatus;

	DWORD sessionTimer = GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_SESSION_EXPIRES);
	DWORD MinSeconds = GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_MIN_SEC);
	CSmallString strSessionExpire;
	CSmallString strMinSec;
	strSessionExpire << sessionTimer;
	strMinSec << MinSeconds;

	pSessionExpireHeader = (CSipHeader*) headerList->GetNextHeader(kSessionExpires);
	if (pSessionExpireHeader)
	{
		strncpy(cHeaderValue, pSessionExpireHeader->GetHeaderStr(), sizeof(cHeaderValue)-1);
		cHeaderValue[sizeof(cHeaderValue)-1] = '\0';
		RmtSessionTimer = atoi(cHeaderValue);
	}

	//Remove the session timer headers
	headerList->RemoveNextHeader(kSessionExpires);
	headerList->RemoveNextHeader(kMinSe);

	if (sessionTimer > 0 && RmtSessionTimer > 0 && MinSeconds > 0)
	{
		if (RmtSessionTimer >= MinSeconds)
		{
			//200ok and support for Session Timer
			CSmallString strRmtSessionExpire;;
			strRmtSessionExpire << RmtSessionTimer;
			headerList->AddHeader(kSessionExpires, strRmtSessionExpire.GetStringLength(), strRmtSessionExpire.GetString());
			headerList->AddHeader(kMinSe, strMinSec.GetStringLength(), strMinSec.GetString());
		}
		else //if(RmtSessionTimer < MinSeconds)
		{
			//422 remote has to change (make bigger) its session timer
			headerList->AddHeader(kMinSe, strMinSec.GetStringLength(), strMinSec.GetString());
			return SipCodesIntervalTooSmall;
		}
	}
	//else
	//{
		//200ok and no support for Session Timer
	//}

	return SipCodesOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParseCsTransportErrorIndEvent(const CMplMcmsProtocol& protocol, DWORD& outStatus, const char*& szErrorMessage, SipCallID& callID)
{
	callID = protocol;

	mcIndTransportError* pTransportErrorMsg = (mcIndTransportError*) (protocol.getpData());
	outStatus = pTransportErrorMsg->status;

	if (outStatus != STATUS_OK && (outStatus < LOW_REJECT_VAL || outStatus >= HIGH_REJECT_VAL)) // something is wrong
		return false;

	sipTransportErrorExpectedReq transportErrorType = (sipTransportErrorExpectedReq)pTransportErrorMsg->expectedReq;
	szErrorMessage = pTransportErrorMsg->sErrMsg;

	//if (transportErrorType == SipTransportErrorDelete)
	//{
		//		m_pCsInterface->SendMsgToCS(SIP_CS_SIG_DEL_NEW_CALL_REQ, pSegment, m_serviceId,
	//}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParseCsBadStatusIndEvent(const CMplMcmsProtocol& protocol, DWORD& outFailOpcode, const char*& szErrorMessage, SipCallID& callID)
{
	callID = protocol;

	mcIndBadStatus* pBadStatusMsg = (mcIndBadStatus*)(protocol.getpData());

	outFailOpcode = pBadStatusMsg->FailedrequestOpcode;
	szErrorMessage = pBadStatusMsg->sErrMsg;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
