//+========================================================================+
//                     MSOrganizerMngr.cpp                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MSOrganizerMngr.cpp	                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | July-2013  |                                                      |
//+========================================================================+

#include "MSOrganizerMngr.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "SIPCommon.h"
#include "IpCsOpcodes.h"
#include "SipDefinitions.h"

//const DWORD TraceXMLBufferSize = 1024 * 1024;

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
extern void DumpXMLToStream(std::ostream &ostr, sipSdpAndHeadersSt* sdpAndHeaders);

////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CMSOrganizerMngr)

ONEVENT(SIP_CS_SIG_INVITE_ACK_IND 				,sMS_CONNECTING, 	 CMSOrganizerMngr::OnSipInviteAckIndMSConnecting)
ONEVENT(PARTYCONNECTTOUT						,sMS_CONNECTING, 	 CMSOrganizerMngr::OnTimerConnectOrganizer)

ONEVENT(SIP_CS_SIG_BYE_IND                      ,sMS_CONNECTED,		 CMSAvMCUMngr::OnSipByeInd)





PEND_MESSAGE_MAP(CMSOrganizerMngr,CMSAvMCUMngr);
////////////////////////////////////////////////////////////////////////////////////


CMSOrganizerMngr::CMSOrganizerMngr()
{
	m_LyncSdp 			 = NULL;
	m_MsConfInviteParser = NULL;
	m_IsCallThroughDma  = NO;

}
/////////////////////////////////////////////////////////////////////////////////
CMSOrganizerMngr::CMSOrganizerMngr(const CMSOrganizerMngr &other)
:CMSAvMCUMngr(other)
{
	m_LyncSdp 			 = NULL;
	m_MsConfInviteParser = NULL;
	m_IsCallThroughDma  = NO;
	m_isRejectForEscalation = FALSE;

}
/////////////////////////////////////////////////////////////////////////////////


CMSOrganizerMngr::~CMSOrganizerMngr()
{
	POBJDELETE(m_LyncSdp);
	POBJDELETE(m_MsConfInviteParser);
}

//////////////////////////////////////////////////////////////////////////////////

void CMSOrganizerMngr::Create(CRsrcParams* pRsrcParams/* ,CPartyApi* pPartyApi*/,CConf* pConf,sipSdpAndHeadersSt* MsConfReq,DWORD PartyId,CSipNetSetup* SipNetSetup,DWORD ServiceId,BYTE isRejectForEscalation)
{

	CMSAvMCUMngr::Create(pRsrcParams /*,pPartyApi*/,pConf, SipNetSetup, ServiceId,PartyId);

	PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::Create ");


	m_callIndex = SipNetSetup->GetCallIndex();


	int length = sizeof(sipSdpAndHeadersBaseSt) + MsConfReq->lenOfDynamicSection;
	m_LyncSdp = (sipSdpAndHeadersSt *)new BYTE[length];
	memset(m_LyncSdp,'\0', length);
	memcpy(m_LyncSdp, MsConfReq, length-1);


	const char*  PlcmCallId = GetPlcmCallId(MsConfReq);
	COstrStream ostr1;
	if(PlcmCallId && PlcmCallId[0])
	{
		TRACEINTO << "CMSOrganizerMngr::Create - pPlcmHeaderStr " << PlcmCallId;
		m_IsCallThroughDma = YES;
	}
	//PTRACE2(eLevelInfoNormal,"CMSOrganizerMngr::create - dump XML2",ostr1.str().c_str()); //debug
	m_isRejectForEscalation = isRejectForEscalation;
	if(STATUS_OK == ParseXML(PartyId))
	{
	//	SipRingingReq(MsConfReq,SipNetSetup,m_isRejectForEscalation);
		BuildResponse(MsConfReq,SipNetSetup,m_isRejectForEscalation);
	}

}
////////////////////////////////////////////////////////////////////////
const char* CMSOrganizerMngr::GetPlcmCallId(sipSdpAndHeadersSt* MsConfReq)
{
	const char* pPlcmHeaderStr = NULL;

	BYTE* pStart = (BYTE*) &MsConfReq->capsAndHeaders + MsConfReq->sipHeadersOffset;
	sipMessageHeaders* pHeaders = (sipMessageHeaders*) pStart;




	if(pHeaders)
	{
		CSipHeaderList headerList(*pHeaders);
		const CSipHeader* pPlcmHeader = headerList.GetNextPrivateOrProprietyHeader(kProprietyHeader, strlen(PLCM_CALL_ID_HEADER), PLCM_CALL_ID_HEADER);

		if (pPlcmHeader)
			pPlcmHeaderStr = pPlcmHeader->GetHeaderStr();
		if (pPlcmHeaderStr)
		{
			TRACEINTO << "header str - " << pPlcmHeaderStr;
			pPlcmHeaderStr += sizeof(PLCM_CALL_ID_HEADER); 	// (skipping header, semicolon & space)
			if(pPlcmHeaderStr && pPlcmHeaderStr[0])
				TRACEINTO << "Plcm call id: field value - " << pPlcmHeaderStr;
		}
	}
	return pPlcmHeaderStr;
}

//////////////////////////////////////////////////////////////////////////////////
STATUS CMSOrganizerMngr::ParseXML(DWORD partyId)
{
	PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::ParseXML ");
	//Need to parse here the XML
	int size = m_LyncSdp->sipMediaLinesLength;
	char* DynamicSection = &m_LyncSdp->capsAndHeaders[m_LyncSdp->sipMediaLinesOffset];
	char *pXMLBuffer = new char[size+1]; AUTO_DELETE_ARRAY(pXMLBuffer);
	PASSERTSTREAM_AND_RETURN_VALUE(!pXMLBuffer,"PartyId:" << partyId,STATUS_INCONSISTENT_PARAMETERS);

	memset(pXMLBuffer,'\0', size);
	memcpy(pXMLBuffer,DynamicSection,size);
	pXMLBuffer[size] = '\0';

	PTRACE2(eLevelInfoNormal,"CMSOrganizerMngr::ParseXML - dump XML2",pXMLBuffer);



	//LyncConfInvite* tmpPars = m_MsConfInviteParser;

	m_MsConfInviteParser = new EventPackage::ConfInvite;

	PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::ParseXML debugg1");

	PASSERTSTREAM_AND_RETURN_VALUE(!m_MsConfInviteParser->ReadFromXmlStream(pXMLBuffer, size), "PartyId:" << partyId, STATUS_INCONSISTENT_PARAMETERS);

//	TRACEINTO << "m_MsConfInviteParser: " << (*m_MsConfInviteParser);

	//return STATUS_FAIL;


	PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::ParseXML debugg2");

	std::string &FocusUriTmp = m_MsConfInviteParser->m_focusUri;
	std::string &MsConversationId = m_MsConfInviteParser->m_conversationId;

	//<focus-uri>sip:boris@isrexchlab.local;gruu;opaque=app:conf:focus:id:MHHLP6C0</focus-uri>


	PTRACE2(eLevelInfoNormal,"CMSOrganizerMngr::ParseXML - FocusUriTmp",FocusUriTmp.c_str());
	PTRACE2(eLevelInfoNormal,"CMSOrganizerMngr::ParseXML - MsConversationId",MsConversationId.c_str());


	int len = strlen(FocusUriTmp.c_str());
		int MsconversationIDlen = strlen(MsConversationId.c_str());

		PTRACE2INT(eLevelInfoNormal,"CMSOrganizerMngr::ParseXML - len",len);
	PTRACE2INT(eLevelInfoNormal,"CMSOrganizerMngr::ParseXML - MsConversationId len",MsconversationIDlen);





	PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::ParseXML debugg3");

	m_FocusUri = new char[len+1];
	 memset(m_FocusUri, 0, len);
	strncpy(m_FocusUri,FocusUriTmp.c_str(),len);
	m_FocusUri[len] = '\0';

	PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::ParseXML debugg4");

	m_MsConversationId = new char[MsconversationIDlen+1];
	memset(m_MsConversationId, 0, MsconversationIDlen);
	strncpy(m_MsConversationId,MsConversationId.c_str(),MsconversationIDlen);
	m_MsConversationId[MsconversationIDlen] = '\0';


	PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::ParseXML debugg5");



	     return STATUS_OK;




}
/*
////////////////////////////////////////////////////////////////////////////////
BYTE CMSOrganizerMngr::SipRingingReq(sipSdpAndHeadersSt* MsConfReq,CSipNetSetup* SipNetSetup,BYTE IsReject)
{
	PTRACE(eLevelInfoNormal,"CMSOrganizerMngr::SipRingingReq");
	BYTE bMessageSent = NO;


	BYTE* pStart = (BYTE*) &MsConfReq->capsAndHeaders + MsConfReq->sipHeadersOffset;
			const sipMessageHeaders* ptmpHeaders = (sipMessageHeaders*) pStart;
			SetHeaders(*pHeaders);

			CSipHeaderList* temp = new CSipHeaderList(*ptmpHeaders);

			// call leg headers
			const CSipHeader* pToDisplay	= temp->GetNextHeader(kToDisplay);
			const CSipHeader* pTo			= temp->GetNextHeader(kTo);
			const CSipHeader* pFromDisplay	= temp->GetNextHeader(kFromDisplay);
			const CSipHeader* pFrom			= temp->GetNextHeader(kFrom);



		//	pReqHeaders = new CSipHeaderList(MIN_ALLOC_HEADERS,0);

			CSipHeaderList*		pReqHeaders = new CSipHeaderList(MIN_ALLOC_HEADERS,0);

			if (pToDisplay)
				pReqHeaders->AddHeader(*pToDisplay);
			if (pTo)
				pReqHeaders->AddHeader(*pTo);
			if (pFromDisplay)
				pReqHeaders->AddHeader(*pFromDisplay);
			if (pFrom)
				pReqHeaders->AddHeader(*pFrom);

			//Contact header
			char strContact[IP_STRING_LEN];
			const char* strLocalAddr		= SipNetSetup->GetLocalSipAddress();
			snprintf(strContact,IP_STRING_LEN,"%s",strLocalAddr);

			pReqHeaders->AddHeader(kContact,strlen(strContact),strContact);

	int headersSize = pReqHeaders->GetTotalLen() - sizeof(sipMessageHeadersBase);	// the header base is included inside the ringing base
	mcReqRinging* pRingingMsg = (mcReqRinging*)new BYTE[sizeof(mcReqRinging) + headersSize];
	sipMessageHeaders* pHeaders = &(pRingingMsg->sipHeaders);
	pReqHeaders->BuildMessage(pHeaders);
	size_t size = sizeof(mcReqRinging) + headersSize;
	SendSIPMsgToCS(SIP_CS_SIG_RINGING_REQ, pRingingMsg, size);

//	bMessageSent = YES;
	PDELETEA(pRingingMsg);
	return bMessageSent;
}
*/
//////////////////////////////////////////////////////////////////////////////////
void CMSOrganizerMngr::BuildResponse(sipSdpAndHeadersSt* MsConfReq,CSipNetSetup* SipNetSetup,BYTE IsReject)
{
	PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::BuildResponse");
	DBGPASSERT_AND_RETURN(MsConfReq == NULL);
	DBGPASSERT_AND_RETURN(SipNetSetup == NULL);
	if (MsConfReq->sipHeadersLength)
	{

		m_state = sMS_CONNECTING;

		TRACEINTO << "m_FocusUri :" << m_FocusUri;

		BYTE* pStart = (BYTE*) &MsConfReq->capsAndHeaders + MsConfReq->sipHeadersOffset;
		const sipMessageHeaders* ptmpHeaders = (sipMessageHeaders*) pStart;
		//SetHeaders(*pHeaders);

		if (ptmpHeaders)
		{
			CSipHeaderList temp(*ptmpHeaders);

			// call leg headers
			const CSipHeader* pToDisplay	= temp.GetNextHeader(kToDisplay);
			const CSipHeader* pTo			= temp.GetNextHeader(kTo);
			const CSipHeader* pFromDisplay	= temp.GetNextHeader(kFromDisplay);
			const CSipHeader* pFrom			= temp.GetNextHeader(kFrom);

			//	pReqHeaders = new CSipHeaderList(MIN_ALLOC_HEADERS,0);

			CSipHeaderList*	pReqHeaders = new CSipHeaderList(MIN_ALLOC_HEADERS,0); AUTO_DELETE(pReqHeaders);

			if (pToDisplay)
				pReqHeaders->AddHeader(*pToDisplay);
			if (pTo)
				pReqHeaders->AddHeader(*pTo);
			if (pFromDisplay)
				pReqHeaders->AddHeader(*pFromDisplay);
			if (pFrom)
				pReqHeaders->AddHeader(*pFrom);

			//Contact header
			char strContact[IP_STRING_LEN];
			const char* strLocalAddr		= SipNetSetup->GetLocalSipAddress();
			snprintf(strContact,IP_STRING_LEN,"%s",strLocalAddr);

			pReqHeaders->AddHeader(kContact,strlen(strContact),strContact);

			PTRACE2(eLevelInfoNormal, "CMSOrganizerMngr::BuildResponse - add on behalf: ", DMA_HEADER_BEHALF_MS_HEADER);
			//pReqHeaders->AddHeader(kProprietyHeader, strlen(DMA_HEADER_BEHALF_MS_HEADER), DMA_HEADER_BEHALF_MS_HEADER);
			//pReqHeaders->AddHeader(kSupported, strlen("ms-early-media"), "ms-early-media");
			//pReqHeaders->AddHeader(kSupported, strlen("replaces"), "replaces");
			//	pReqHeaders->AddHeader(kSupported, strlen("ms-safe-transfer"), "ms-safe-transfer");
			pReqHeaders->AddHeader(kSupported, strlen("ms-conf-invite"), "ms-conf-invite");
			//pReqHeaders->AddHeader(kSupported, strlen("histinfo"), "histinfo");
			//	pReqHeaders->AddHeader(kSupported, strlen("100rel"), "100rel");
			//	pReqHeaders->AddHeader(kSupported, strlen("ms-sender"), "ms-sender");
			//	pReqHeaders->AddHeader(kSupported, strlen("tdialog"), "tdialog");
			//session timer..  --> Is needed??
			DWORD sessionTimer	= GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_SESSION_EXPIRES);
			DWORD MinSeconds 	= GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_MIN_SEC);


			sessionTimer =1200;
			MinSeconds =1200;
			PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::BuildResponse - noa temp hardcoded timer 1200 if 90 as default call disconnect");
			CSmallString strSessionExpire;
			CSmallString strMinSec;
			strSessionExpire << sessionTimer;
			strMinSec << MinSeconds;

			PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::BuildResponse - Session Timer ");
			pReqHeaders->AddHeader(kSessionExpires,	strSessionExpire.GetStringLength(),	strSessionExpire.GetString());
			pReqHeaders->AddHeader(kMinSe,			strMinSec.GetStringLength(),			strMinSec.GetString());


			//	int msgLen = sizeof(mcReqInviteResponse) + capsAndHeadersSize + 10000; //Amihay: 10k extra to prevent overwrite memory
			//	DWORD sizeAdded = 0;

			mcReqInviteResponse* pInviteResponseMsg = NULL;
			BYTE* tmp = new BYTE[SIP_INIT_INVITE_RESPONSE_BUFFER_SIZE]; AUTO_DELETE_ARRAY(tmp);
			pInviteResponseMsg = (mcReqInviteResponse* )tmp;
			//pInviteResponseMsg = (mcReqInviteResponse* )new BYTE[msgLen];
			memset(pInviteResponseMsg, 0 , SIP_INIT_INVITE_RESPONSE_BUFFER_SIZE);

			// Generic fields
			if(IsReject)
				pInviteResponseMsg->status = SipCodesUnsuppMediaType;
			else
				pInviteResponseMsg->status = STATUS_OK;
			//		sizeAdded += sizeof(pInviteResponseMsg->status);
			pInviteResponseMsg->bIsFocus	= FALSE;
			// 		sizeAdded += sizeof(pInviteResponseMsg->bIsFocus);

			sipSdpAndHeadersSt* pSdpAndHeaders = &pInviteResponseMsg->sipSdpAndHeaders;

			//Build msg
			pSdpAndHeaders->lenOfDynamicSection = 0;
			pSdpAndHeaders->sipMediaLinesOffset = 0;
			pSdpAndHeaders->sipMediaLinesLength = 0;


			//	int capabilitiesSize    = 0;
			//	capabilitiesSize = sizeof(sipMediaLinesEntrySt);
			pSdpAndHeaders->sipHeadersOffset = pSdpAndHeaders->lenOfDynamicSection;
			pSdpAndHeaders->sipHeadersLength = pReqHeaders->GetTotalLen();

			sipMessageHeaders* pHeaders		 = (sipMessageHeaders*)((char*)pSdpAndHeaders->capsAndHeaders + pSdpAndHeaders->sipHeadersOffset);
			pSdpAndHeaders->lenOfDynamicSection += pReqHeaders->BuildMessage(pHeaders);

			//	sipSdpAndHeadersSt* pSdpAndHeaders  = &pInviteResponseMsg->sipSdpAndHeaders;
			//		pSdpAndHeaders->sipHeadersOffset = pSdpAndHeaders->lenOfDynamicSection;
			//	pSdpAndHeaders->sipHeadersLength = 	pReqHeaders.GetTotalLen();

			TRACEINTOFUNC   << "\n--- DBG caps and headers -----"
					<< "\nDynamic section len = " << pSdpAndHeaders->lenOfDynamicSection
					<< "\nMedaia lines offset = " << pSdpAndHeaders->sipMediaLinesOffset
					<< "\nMedia lines length = " << pSdpAndHeaders->sipMediaLinesLength
					<< "\nICE Generals offset = " << pSdpAndHeaders->sipIceOffset
					<< "\nICE Generals length = " << pSdpAndHeaders->sipIceLength
					<< "\nHeaders offset = " << pSdpAndHeaders->sipHeadersOffset
					<< "\nHeaders length = " << pSdpAndHeaders->sipHeadersLength;

			int size = sizeof(mcReqInviteResponse) + pInviteResponseMsg->sipSdpAndHeaders.lenOfDynamicSection;
			SendSIPMsgToCS(SIP_CS_SIG_INVITE_RESPONSE_REQ, pInviteResponseMsg, size);

			DWORD SIPMsgTout = GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_MSG_TIMEOUT);
			StartTimer(PARTYCONNECTTOUT, SIPMsgTout * SECOND);
		}


	}
	else // no headers
		DBGPASSERT(YES);



}
//////////////////////////////////////////////////////////////////////////////////
void CMSOrganizerMngr::OnSipInviteAckIndMSConnecting(CSegment* pParam)
{
	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIU16 srcUnitId = 0;
	CMedString str;

	*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;
	PTRACE(eLevelInfoNormal,"CMSOrganizerMngr::OnSipInviteAckIndMSConnecting");

	if (IsValidTimer(PARTYCONNECTTOUT))
		DeleteTimer(PARTYCONNECTTOUT);

	mcIndInviteAck* pInviteAckMsg = (mcIndInviteAck*) pParam->GetPtr(1);
	DWORD status = pInviteAckMsg->status;

	if (status == STATUS_OK)
	{
		PTRACE(eLevelInfoNormal,"CMSOrganizerMngr::OnSipInviteAckIndMSConnecting - status OK");
		m_state = sMS_CONNECTED;
	}
	else
	{
		PTRACE2INT (eLevelInfoNormal,"CMSOrganizerMngr::OnSipInviteAckIndMSConnecting - status: ", status );
		m_state = sMS_CONNECTED;
	}

	if(m_isRejectForEscalation)
	{
		PTRACE(eLevelInfoNormal,"CMSOrganizerMngr::OnSipInviteAckIndMSConnecting - reject for escalate");
		status = STATUS_FAIL;
	}

	m_pTaskApi->MSOrganizerEndConnection(m_PartyId,status,m_FocusUri,m_MsConversationId,m_IsCallThroughDma);




}
///////////////////////////////////////////////////////////////////////////
void CMSOrganizerMngr::OnTimerConnectOrganizer(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CMSOrganizerMngr::OnTimerConnectOrganizer");

	if (IsValidTimer(PARTYCONNECTTOUT))
		DeleteTimer(PARTYCONNECTTOUT);

//	m_pPartyApi->SipPartyConnectTout();
}
///////////////////////////////////////////////////////////////////////////
/*
void CMSOrganizerMngr::OnSipByeInd(CSegment* pParam)
{


	m_state = sMS_DISCONNECTING;
	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIU16 srcUnitId = 0;
	PTRACE(eLevelInfoNormal,"CMSOrganizerMngr::OnSipByeInd");

	TRACECOND_AND_RETURN(!pParam , "pParam is NULL");
	*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

	mcIndBye* pByeMsg = (mcIndBye *)pParam->GetPtr(1);
	DWORD status = pByeMsg->status;
	DBGPASSERT(status); // only if status != STATUS_OK

	if (IsValidTimer(PARTYCONNECTTOUT))
		DeleteTimer(PARTYCONNECTTOUT);


	//Send BYE ACK

	mcReqBye200Ok* pBye200OkMsg = new mcReqBye200Ok;
	size_t size = sizeof(mcReqBye200Ok);
	memset(pBye200OkMsg, 0, size);
	SendSIPMsgToCS(SIP_CS_SIG_BYE_200_OK_REQ, pBye200OkMsg, size);


	m_state = sMS_DISCONNECTED;

	RemoveFromRsrcTbl();

	PDELETE(pBye200OkMsg);
	PTRACE(eLevelInfoNormal,"CMSOrganizerMngr::OnSipByeInd");

	//No Need to updaet the DB on disconnect

}
*/
