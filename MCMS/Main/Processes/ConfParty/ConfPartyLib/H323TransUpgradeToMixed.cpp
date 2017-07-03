/*
 * H323TransAvcUpgradeToMixed.cpp
 *
 *  Created on: Feb 18, 2013
 *      Author: asilver
 */



#include "Segment.h"
#include "StateMachine.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "Trace.h"
#include "Macros.h"
#include "NStream.h"
#include "NetSetup.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "Conf.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"
#include "IpCsOpcodes.h"
#include "TaskApi.h"
#include "Party.h"
#include "PartyApi.h"
//#include "SipUtils.h"
#include "IpNetSetup.h"
#include "CsInterface.h"
#include "ConfApi.h"
#include "IPParty.h"
#include "H323Control.h"
#include "Lobby.h"
#include "H323Party.h"
#include "ConfPartyGlobals.h"
#include "IpServiceListManager.h"
#include "IPParty.h"

#include "H323TransUpgradeToMixed.h"

////////////////////////////////////////////////////////////////////////////
const WORD   TRANS_WAIT_FOR_UPDATE_CHANN	= 142;
const WORD   TRANS_CHANNS_UPDATED_WAIT_FOR_BRIDGES_UPGRADE	= 143;

PBEGIN_MESSAGE_MAP(CH323TransUpgradeToMixed)

ONEVENT(PARTY_UPGRADE_TO_MIXED,				IDLE,							CH323TransUpgradeToMixed::OnPartyUpgradeToMixed)
ONEVENT(PARTY_TRANSLATOR_ARTS_CONNECTED,	PARTY_ALLOCATE_TRANSLATOR_ARTS,	CH323TransUpgradeToMixed::OnPartyTranslatorArtsConnected)
ONEVENT(H323_PARTY_CHANS_UPDATED,			TRANS_WAIT_FOR_UPDATE_CHANN, 		CH323TransUpgradeToMixed::OnPartyChannelsUpdated)
ONEVENT(END_VIDEO_UPGRADE_TO_MIX_AVC_SVC,	TRANS_CHANNS_UPDATED_WAIT_FOR_BRIDGES_UPGRADE, CH323TransUpgradeToMixed::OnEndVideoUpgradeToMix)
PEND_MESSAGE_MAP(CH323TransUpgradeToMixed, CStateMachine);

CH323TransUpgradeToMixed::CH323TransUpgradeToMixed(CTaskApp *pOwnerTask):CStateMachine(pOwnerTask)
{
	m_pH323Cntl 				= NULL;
	m_pPartyConfName 			= NULL;
	m_pCurrentMode 				= NULL;
	m_pTargetMode 				= NULL;
	m_pPartyApi					= new CPartyApi;
	m_pParty					= NULL;
}


CH323TransUpgradeToMixed::~CH323TransUpgradeToMixed()
{
	POBJDELETE(m_pPartyApi);
}


///////////////////////////////////////////////////////////////////////////////////////
void CH323TransUpgradeToMixed::InitTransaction(CH323Party* pParty, CH323Cntl* pPartyH323Cntl, CIpComMode* pPartyCurrentMode, CIpComMode* pPartyTargetMode, char* pPartyConfName/*, WORD voice, char* alternativeAddrStr,CIpComMode* pTargetModeMaxAllocation, BYTE bTransactionSetContentOn, BYTE isContentSuggested*/)
{
	TRACEINTO << "mix_mode: Name - " << pPartyConfName;

	m_pH323Cntl 			= pPartyH323Cntl;
	m_pCurrentMode 			= pPartyCurrentMode;
	m_pTargetMode 			= pPartyTargetMode;
//	m_pEDialState 			= pPartyEDialState;
	m_pPartyConfName 		= pPartyConfName;
//	m_voice					= voice;
//	m_pAlternativeAddrStr	= alternativeAddrStr;
	m_pPartyApi->CreateOnlyApi(pParty->GetRcvMbx(), this);
	m_pPartyApi->SetLocalMbx(pParty->GetLocalQueue());
//	m_pTargetModeMaxAllocation = pTargetModeMaxAllocation;
	m_pParty				= pParty;
//
//	m_bTransactionSetContentOn = bTransactionSetContentOn;
//
//	m_isContentSuggested 	= isContentSuggested;

//	PTRACE2INT(eLevelInfoNormal,"CSipTransaction::InitTransaction : TIP mode: ", m_pTargetMode->GetIsTipMode());
//	PTRACE2INT(eLevelInfoNormal,"CSipTransaction::InitTransaction : m_bTransactionSetContentOn: ", m_bTransactionSetContentOn);
}

void CH323TransUpgradeToMixed::OnPartyUpgradeToMixed(CSegment* pParam)
{
	TRACEINTO << "mix_mode: dynMixed arrived at transaction";
	CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];

	CRsrcParams* pMrmpRsrcParams = NULL;

	DeSerializeNonMandatoryRsrcParams(pParam, pMrmpRsrcParams, "mix_mode: MRMP");
	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		// what about allocation of mrmp???
		DeSerializeNonMandatoryRsrcParams(pParam, avcToSvcTranslatorRsrcParams[i], "mix_mode: translator");
	}

	m_pH323Cntl->SetInternalControllerResource(avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);

	m_pH323Cntl->AddToInternalRoutingTable();
	bool flag=false;


	if(m_pTargetMode->GetConfMediaType() == eMixAvcSvc)
	{
		m_state = PARTY_ALLOCATE_TRANSLATOR_ARTS;
		TRACEINTO << "mix_mode: mixed mode detected";
		if (m_pH323Cntl->OpenInternalArts(E_NETWORK_TYPE_IP) > 0)
		  flag = true;
	}
	else
	{
		TRACEINTO<<"mix_mode: dynmixedErr mixed mode wasn't detected";
	}

	if(flag==false)
	{
		TRACEINTO<<"mix_mode: there are no translators";
		OnPartyTranslatorArtsConnected();
	}


	POBJDELETE(pMrmpRsrcParams);
	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
	}
}

void CH323TransUpgradeToMixed::OnPartyTranslatorArtsConnected()
{
	TRACEINTO<<"mix_mode: arts connected";
	m_state=TRANS_WAIT_FOR_UPDATE_CHANN;
	m_pTargetMode->Dump("***mix_mode: CH323TransUpgradeToMixed::OnPartyTranslatorArtsConnected", eLevelInfoNormal);
    if (m_pH323Cntl->UpgradeAvcChannelReq((CComModeH323*)m_pTargetMode)<=0)
    {
   		TRACEINTO<<"mix_mode: dynmixedErr no requests to open internal channels"; // ey_20866
    }
}

void CH323TransUpgradeToMixed::OnPartyChannelsUpdated(CSegment* pParam)
{
	TRACEINTO<<"mix_mode: all channels have been  upgraded (arrived at transaction: after channels update)";
	CSegment* pSeg = new CSegment;
	m_state = TRANS_CHANNS_UPDATED_WAIT_FOR_BRIDGES_UPGRADE;
	SendMessageToParty(PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED,NULL/*pSeg*/);
	POBJDELETE(pSeg);

}
void CH323TransUpgradeToMixed::OnEndVideoUpgradeToMix(CSegment* pParam)
{
	EndTransaction();
	TRACEINTO<<"mix_mode: @#@ ";
}

/////////////////////////////////////////////////////////////////////////////////
// All messages from Sip-Transaction to Sip-Party must be sent by this function
void CH323TransUpgradeToMixed::SendMessageToParty(OPCODE event, CSegment *pSeg)
{
	m_pPartyApi->SendLocalMessage(pSeg, event);
}

///////////////////////////////////////////////////////////////////////////////////////
void CH323TransUpgradeToMixed::EndTransaction(DWORD retStatus)
{
	TRACEINTO<< "Name - " << m_pPartyConfName;
	CleanTransaction();
//	StartFECIfNeeded();
	SendEndTransactionToParty(retStatus);
	m_state = sTRANS_END_WAS_SENT;
}

void CH323TransUpgradeToMixed::CleanTransaction()
{
	if (IsValidTimer(OPENBRIDGESTOUT))
		DeleteTimer(OPENBRIDGESTOUT);
	if (IsValidTimer(UPDATEBRIDGESTOUT))
		DeleteTimer(UPDATEBRIDGESTOUT);
	if (IsValidTimer(ICEPORTSRETRYTOUT))
		DeleteTimer(ICEPORTSRETRYTOUT);
	if (IsValidTimer(UPGRADETOMIXTOUT))
		DeleteTimer(UPGRADETOMIXTOUT);
}


/////////////////////////////////////////////////////////////////////////////
void  CH323TransUpgradeToMixed::SendEndTransactionToParty(DWORD retStatus)
{
	CSegment * pSeg = new CSegment;
	*pSeg << (DWORD)retStatus;
	SendMessageToParty(H323_TRANS_END_TRANSACTION, pSeg);
}

BOOL CH323TransUpgradeToMixed::HandlePartyEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
	m_pPartyApi->SendSipTransMsg(opCode, pMsg);
	return TRUE;
}

BOOL CH323TransUpgradeToMixed::DispatchEvent(OPCODE event,CSegment* pParam)
{
    if (IsEventExist(event))
    {
        return CStateMachine::DispatchEvent(event,pParam);
    }
    return FALSE;
}
