//+========================================================================+
//               SIPTransRTCPVideoUpdateInd.cpp    				       	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransRTCPVideoUpdateInd.cpp                          	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


//#include "SIPTransInviteNoSdpInd.h"
#include "Segment.h"
#include "StateMachine.h"
#include "Macros.h"
#include "SIPTransaction.h"
#include "ConfPartyOpcodes.h"
#include "SIPTransRTCPVideoUpdateInd.h"
#include "CapClass.h"
#include "SIPControl.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransRTCPVideoUpdateInd)

ONEVENT(SIP_TRANS_UPDATE_VIDEO_PREFERENCE,	IDLE,									CSipTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges)
ONEVENT(SIP_CONF_BRIDGES_UPDATED_RES,		sTRANS_RECVIDEOPREFEUPDATEBRIDGE,		CSipTransRTCPVideoUpdateInd::OnConfBridgesUpdatedUpdateBridges)

// timeout
ONEVENT(UPDATEBRIDGESTOUT,					sTRANS_RECVIDEOPREFEUPDATEBRIDGE, 		CSipTransRTCPVideoUpdateInd::OnUpdateBridgesTout)



PEND_MESSAGE_MAP(CSipTransRTCPVideoUpdateInd, CSipTransaction);

///////////////////////////////////////////////////////////
CSipTransRTCPVideoUpdateInd::CSipTransRTCPVideoUpdateInd(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = TRUE;
	m_bIsReInvite = FALSE;
	//VALIDATEMESSAGEMAP
}

///////////////////////////////////////////////////////
CSipTransRTCPVideoUpdateInd::~CSipTransRTCPVideoUpdateInd()
{
}
///////////////////////////////////////////////////////
//	Default FR MOC 2010
// QCIF,CIF: 15 fps
// VGA, HD: 30 fps

void CSipTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"SIPTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges: Name ", m_pPartyConfName);

	DWORD Width;
	DWORD Height;
	DWORD BitRate;
	DWORD FrameRate;

	DWORD thisFs  = 0;
	DWORD thisMbps  = 0;

	*pParam >> Width >> Height >> FrameRate >> BitRate; 

	CSipCaps*	pRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
	const CSipCaps* pLocalCaps = m_pSipCntl->GetLocalCaps();

	DWORD details	= 0;
	int  arrIndex	= 0;
	DWORD ValuesToCompare = kCapCode|kFormat;

	CComModeH323* pScmWithNewRes =  new CComModeH323;
	*pScmWithNewRes = *m_pTargetMode;
	pScmWithNewRes->SetRtvScm(Width,Height,FrameRate,cmCapTransmit,BitRate);
	const CBaseCap* pModeNewRes = pScmWithNewRes->GetMediaAsCapClass(cmCapVideo,cmCapTransmit);
	BYTE IsSupportNewRes = pRemoteCaps->IsContainingCapSet(cmCapReceive, *pModeNewRes, ValuesToCompare, &details, &arrIndex);
	BYTE isLocalSupportNewRes = pLocalCaps->IsContainingCapSet(cmCapReceive, *pModeNewRes, ValuesToCompare, &details, &arrIndex);

	if (!IsSupportNewRes ||  !isLocalSupportNewRes)
	{
		PTRACE(eLevelInfoNormal,"SIPTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges HD preference is not supported in Remote or Local caps -> trying to switch to 640/480");

		if (Width == 1280 && Height == 720)
		{
			Width = 640;
			Height = 480;
			BitRate = 0;

			CComModeH323* pScmWithNewRes =  new CComModeH323;
			*pScmWithNewRes = *m_pTargetMode;
			pScmWithNewRes->SetRtvScm(Width,Height,FrameRate,cmCapTransmit,BitRate);
			const CBaseCap* pModeNewRes = pScmWithNewRes->GetMediaAsCapClass(cmCapVideo,cmCapTransmit);
			IsSupportNewRes = pRemoteCaps->IsContainingCapSet(cmCapReceive, *pModeNewRes, ValuesToCompare, &details, &arrIndex);
			isLocalSupportNewRes = pLocalCaps->IsContainingCapSet(cmCapReceive, *pModeNewRes, ValuesToCompare, &details, &arrIndex);
		}
	}

	//Need to find in the remote caps the relevant FR for the requested resolution
	if(IsSupportNewRes && isLocalSupportNewRes)
	{
		DWORD RemoteCapsFrameRate=0;

		thisFs = (Width * Height)/256;
		pRemoteCaps->GetRtvCapFRAccordingToFS(RemoteCapsFrameRate,thisFs);

		if(RemoteCapsFrameRate)
		{
			thisMbps =  thisFs * RemoteCapsFrameRate;
			FrameRate = thisMbps/thisFs;

			PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges- thisMbps: ",thisMbps);
			PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges- FrameRate: ",FrameRate);

		}
		else
		{
			PTRACE(eLevelInfoNormal,"SIPTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges Not found FR");
			EndTransaction(STATUS_OK);
			return;
		}

	}
	else
	{
		PTRACE(eLevelInfoNormal,"SIPTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges new Res is not supported in Remote caps or our local caps ");
		EndTransaction(STATUS_OK);
		DBGPASSERT_AND_RETURN(1);
	}


	if(thisMbps && thisFs)
	{
		eVideoPartyType videoPartyType = eVideo_party_type_dummy;
		videoPartyType = ::GetCPH264ResourceVideoPartyType((DWORD)thisFs, (DWORD)thisMbps);


		CSmallString msg;
		msg << " fs: " << thisFs << " mbps: " << thisMbps << " videoPartyType: " << eVideoPartyTypeNames[videoPartyType]
		                                                 << " Width: " <<Width <<" Height: " <<Height;
		PTRACE2(eLevelInfoNormal,"SIPTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges: ",msg.GetString());

		//change only the transmit mode
		m_pTargetMode->SetRtvScm(Width,Height,FrameRate,cmCapTransmit,BitRate);
		m_pCurrentMode->SetRtvScm(Width,Height,FrameRate,cmCapTransmit,BitRate);

		COstrStream msg1;
		m_pTargetMode->Dump(msg1);
		PTRACE2(eLevelInfoNormal,"SIPTransRTCPVideoUpdateInd::OnPartyReceivedRTCPVideoChanges, m_pTargetMode : ", msg1.str().c_str());

		m_state = sTRANS_RECVIDEOPREFEUPDATEBRIDGE;
		StartTimer(UPDATEBRIDGESTOUT, BRIDGES_TIME * SECOND);

		SendVideoChangeResolutionToParty(Width,Height,FrameRate,BitRate);

	}

}
//////////////////////////////////////////////////////
void CSipTransRTCPVideoUpdateInd::OnConfBridgesUpdatedUpdateBridges(CSegment* pParam)
{
	if (IsValidTimer(UPDATEBRIDGESTOUT))
	{
		DeleteTimer(UPDATEBRIDGESTOUT);
		PTRACE(eLevelInfoNormal,"CSipTransRTCPVideoUpdateInd::OnConfBridgesUpdatedUpdateBridges: DeleteTimer(UPDATEBRIDGESTOUT) ");
	}

	//SIP re-invite to check if the change mode succeeded

	PTRACE2(eLevelInfoNormal,"CSipTransRTCPVideoUpdateInd::OnConfBridgesUpdatedUpdateBridges, Name ",m_pPartyConfName);

	EndTransaction(STATUS_OK);

}
/////////////////////////////////////////////////////////////////////
void CSipTransRTCPVideoUpdateInd::OnUpdateBridgesTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransRTCPVideoUpdateInd::OnUpdateBridgesTout");
	WORD callIndex = m_pSipCntl->GetCallIndex();
	DBGPASSERT(callIndex);
	EndTransaction(STATUS_OK);
}
