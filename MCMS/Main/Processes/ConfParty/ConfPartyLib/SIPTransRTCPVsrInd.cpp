//+========================================================================+
//               SIPTransRTCPVsrInd.cpp    				       	           |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransRTCPVsrInd.cpp                          	           |
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
#include "SIPTransRTCPVsrInd.h"
#include "CapClass.h"
#include "SIPControl.h"
#include "SIPParty.h"
#include "CommConf.h"
#include "CommConfDB.h"
#include "SipScm.h"
#include "MsSvcMode.h"
#include "MsVsrMsg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSIPTransRTCPVsrInd)

ONEVENT(SIP_TRANS_VSR_MSG_IND,	IDLE,									CSIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd)

ONEVENT(SIP_CONF_BRIDGES_UPDATED_RES,		sTRANS_RECVIDEOPREFEUPDATEBRIDGE,		CSIPTransRTCPVsrInd::OnConfBridgesUpdatedUpdateBridges)

ONEVENT(SIP_PARTY_CHANS_UPDATED,		sTRANS_RECREINVITEUPDATECHANN,				CSIPTransRTCPVsrInd::OnPartyChannelsUpdated)

// timeout
ONEVENT(UPDATEBRIDGESTOUT,					sTRANS_RECVIDEOPREFEUPDATEBRIDGE, 		CSIPTransRTCPVsrInd::OnUpdateBridgesTout)

PEND_MESSAGE_MAP(CSIPTransRTCPVsrInd, CSipTransaction);

///////////////////////////////////////////////////////////
CSIPTransRTCPVsrInd::CSIPTransRTCPVsrInd(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = TRUE;
	m_bIsReInvite = FALSE;
	m_isKbitOn = FALSE;
	m_isNeedToUpdateArtRegardingFEC = FALSE;
	//VALIDATEMESSAGEMAP
}

///////////////////////////////////////////////////////
CSIPTransRTCPVsrInd::~CSIPTransRTCPVsrInd()
{
}

///////////////////////////////////////////////////////
void CSIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd: Name ", m_pPartyConfName);

	//we need to update tx target mode according to current resource which is local caps.
	//we will update it to be equal to local caps and then call bestmode inorder to retrive best mode according to new vsr and current allocation
	const CSipCaps* pLocalCaps = m_pSipCntl->GetLocalCaps();
	// *pLocalCaps->Dump();
	DWORD vidBitRate = m_pTargetMode->GetVideoBitRate(cmCapReceive,kRolePeople);
	DWORD tempMaxRateForSvc = 0;//noa to remove
	DWORD tempMaxRateForRtv = 0;//noa to remove

	if(vidBitRate == 0)
	{
		vidBitRate = m_pTargetMode->GetVideoBitRate(cmCapTransmit,kRolePeople);
	}
	if(pLocalCaps->IsCapSet(eMsSvcCapCode))
	{
		MsSvcVideoModeDetails MsSvcDetails;
		if (!pLocalCaps->GetMsSvcVidMode(MsSvcDetails))
		{
			PTRACE(eLevelInfoNormal, "CSIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd can't find msSvc details params, vsr send fail!");
			DBGPASSERT(YES);
		}
		else
			m_pTargetMode->SetMsSvcScm(MsSvcDetails,cmCapTransmit,vidBitRate);
	}
	else if(pLocalCaps->IsCapSet(eRtvCapCode))
	{
		RTVVideoModeDetails rtvDetails;
		memset(&rtvDetails, 0, sizeof(rtvDetails));//KW 4997
		DWORD rtvBitRate = 0;
		pLocalCaps->GetRtvCap(rtvDetails,rtvBitRate);
		if(rtvBitRate == 0)
		{
			PTRACE(eLevelInfoNormal, "CSIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd error rtv video rate =0");
			DBGPASSERT(YES);
			m_pTargetMode->SetRtvVideoParams(rtvDetails,cmCapTransmit);
			m_pTargetMode->SetVideoBitRate(vidBitRate,cmCapTransmit);

		}
		else
		{
			m_pTargetMode->SetRtvVideoParams(rtvDetails,cmCapTransmit);
			m_pTargetMode->SetVideoBitRate(rtvBitRate,cmCapTransmit);
		}

	}
	else
	{
		PTRACE(eLevelInfoNormal, "CSIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd error no RTV and no MS SVC");
		DBGPASSERT(YES);
		return;
	}
	
	/////
	ST_VSR_SINGLE_STREAM vsr;
	pParam->Get(reinterpret_cast<BYTE*>(&vsr), sizeof(ST_VSR_SINGLE_STREAM));

	CMsVsrMsg vsrMsg(vsr);
	vsrMsg.Dump();

	CSipCaps*           pRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
    if(vsr.msi != VSR_SOURCE_NONE)
    {
		PTRACE(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd:-switch to unmute");
		m_pSipCntl->setMsftStreamOnState(TRUE);
    }
    else
    {
		m_pSipCntl->setMsftStreamOnState(FALSE);
    }
    PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd: vsr.num_vsrs_params ", vsr.num_vsrs_params);
    PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd: vsr.msi ", vsr.msi);
	/* MSSlave Flora Question: why mute media here ? you means ,if there is no vsrc params in the VSR Request, the receiver is not requesting a video source ? */
	/* just the same as setting the MSI: SOURCE_NONE ? */
	/* Question 2: i think for MSSlave Out, there should be only one VSR entry here, and vsr.num_vsrs_params should be 1. */
	if(vsr.num_vsrs_params == 0) // muted
    {
		if(m_pSipCntl->isMs2013Active() == eMsft2013AvMCU)
		{
			//send pacsi with mute: pr_id=DUMMY
			BYTE isMute = TRUE;
			TRACEINTO << " ConfName:" << m_pPartyConfName  << " - no vsr params changing mute AVMCU case";
			SendSingleUpdatePacsiInfoToParty(isMute);
			// ?
			SendMuteMediaToParty(cmCapTransmit);
			EndTransaction(STATUS_OK);
			// end ?
			return;
		}else // Lync client
		{
			TRACEINTO << " ConfName:" << m_pPartyConfName  << " - no vsr params changing mute if needed and returning Lync client case";
			SendMuteMediaToParty(cmCapTransmit);
			EndTransaction(STATUS_OK);
			return;
		}
    }
	PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd: vsr.key_frame ", vsr.key_frame);
	if(vsr.key_frame == 1)
	{
		m_isKbitOn  = TRUE;
	}

	RTVVideoModeDetails rtvVidModeDetails;
	memset(&rtvVidModeDetails, 0, sizeof(RTVVideoModeDetails));
	int whichi =0;


	for (DWORD i=0; i<vsr.num_vsrs_params; i++)
	{
		if (eMsSvcDynamicPayload == vsr.st_vsrs_params[i].payload_type)
		{
			TRACEINTO << "LYNC2013_FEC_RED:DEBUG Handling MSsvc VSR, i:" << i;

			ST_VSR_PARAMS* pStVsrParam = &(vsr.st_vsrs_params[i]);

			MsSvcVideoModeDetails MsSvcDetails;
			memset(&MsSvcDetails, 0, sizeof(MsSvcVideoModeDetails));
			MsSvcDetails.aspectRatio = pStVsrParam->aspect_ratio;
			PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd: MsSvcDetails.aspectRatio ", MsSvcDetails.aspectRatio);
            if(MsSvcDetails.aspectRatio == E_VIDEO_RES_ASPECT_RATIO_4_3 && pStVsrParam->max_width >= 1280  )
            {
				MsSvcDetails.aspectRatio = E_VIDEO_RES_ASPECT_RATIO_16_9;
				PTRACE(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd -as this is HD and aspect ratio is 4 on 3 we change it manually to be 16:9 ");
            }
            // bridge-8059
          
            {
            	DWORD MaxRateAccordingToMsDescion = 0;
            	DWORD maxRate = 0;
            	CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();
            	MsSvcVidMode->ReturnMaxRateForMsAccordingToResolution(MaxRateAccordingToMsDescion,pStVsrParam->max_width,pStVsrParam->max_height);
            	if (MaxRateAccordingToMsDescion != 0)
            	{
            		maxRate = min(pRemoteCaps->getMsftRxVideoBwLimitation()*100,MaxRateAccordingToMsDescion);
            	}
            	else
            		maxRate = pRemoteCaps->getMsftRxVideoBwLimitation()*100;

            	maxRate = min(maxRate,pStVsrParam->min_bitrate);
            	MsSvcDetails.maxBitRate = maxRate;
            	tempMaxRateForSvc = maxRate / 100;

            	POBJDELETE(MsSvcVidMode);

            	//////////////////////////////////////////////////////
            	//LYNC2013_FEC_RED: in case of packet loss we want to calc video bit rate:
            	UpdateVideoBitRateCausedByFecIfNeeded(eMsSvcCapCode,pStVsrParam->frame_rate,vidBitRate,tempMaxRateForSvc,vsr.st_vsrs_params[i].qualityReportHistogram);

            	/*
            	if ( m_pTargetMode->GetIsFec() || m_pSipCntl->GetIsRedOn() )  //if (m_pSipCntl->GetIsFecOn() || m_pSipCntl->GetIsRedOn())
            	{
            		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
            		eAvMcuLinkType avMcuLinkType = eAvMcuLinkNone;

            		if (pCommConf)
            		{
            			CConfParty* pConfParty 	= pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());

            			if (pConfParty)
            			{
            				avMcuLinkType = pConfParty->GetAvMcuLinkType();
            				if (avMcuLinkType == eAvMcuLinkSlaveOut)
            					vidBitRate = pLocalCaps->GetVideoRate();
            			}
            		}

            		DWORD maxRateDependsOnFecAndRed = min(tempMaxRateForSvc,vidBitRate);


            		//DWORD newFecPercent = 0;
            		//for (int j=1; j<VSR_NUM_OF_QUALITY_LEVELS; ++j)
            		//		if (vsr.st_vsrs_params[i].qualityReportHistogram[j] == 1)
            		//			newFecPercent = j;


            		UpdateFecParams(vsr.st_vsrs_params[0].qualityReportHistogram);
            		//UpdateFecParams(newFecPercent);

            		if (m_pSipCntl->GetIsFecOn())
            			maxRateDependsOnFecAndRed = m_pParty->CalcVideoRateForMSsvcFEC(maxRateDependsOnFecAndRed,m_pSipCntl->GetFecFractionLossInPercent());
            		if (m_pSipCntl->GetIsRedOn())
            			maxRateDependsOnFecAndRed = m_pParty->CalcVideoRateForRED(maxRateDependsOnFecAndRed);

            		TRACEINTO << "LYNC2013_FEC_RED: set new rate for targetModeTx according to VSR msg:" << maxRateDependsOnFecAndRed << ", avMcuLinkType:" << (DWORD)avMcuLinkType
            				  << ", vidBitRate:" << vidBitRate << ", tempMaxRateForSvc:" << tempMaxRateForSvc;

            		m_pTargetMode->SetVideoBitRate(maxRateDependsOnFecAndRed, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);
            	}
            	else
            		TRACEINTO << "LYNC2013_FEC_RED: MsSvcDetails.av mcu MaxRateAccordingToMsDescion:" << maxRate << ", vidBitRate:" << vidBitRate << ", tempMaxRateForSvc:" << tempMaxRateForSvc;
            	*/
            }


			MsSvcDetails.minBitRate = pStVsrParam->min_bitrate;
			if(m_pSipCntl->GetCacBwLimitation() != -1)
			{
				MsSvcDetails.minBitRate = min(MsSvcDetails.minBitRate,((unsigned int)( m_pSipCntl->GetCacBwLimitation()*100 ) ));
				PTRACE2INT(eLevelInfoNormal,"CSIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd CAC is on - final rate is ",MsSvcDetails.minBitRate);
			}

			MsSvcDetails.maxFrameRate = pStVsrParam->frame_rate;
			PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd: MsSvcDetails.maxFrameRate ", MsSvcDetails.maxFrameRate);
			MsSvcDetails.maxHeight = pStVsrParam->max_height;
			MsSvcDetails.maxWidth = pStVsrParam->max_width;
			MsSvcDetails.maxNumOfPixels = pStVsrParam->maxNumOfPixels;

			if (NO == pRemoteCaps->SetMsSvcVidMode(MsSvcDetails))
			{
				PTRACE(eLevelInfoNormal,"CSIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd Failed updating MsSvc VSR details");
			}
		}

		if (eRtvDynamicPayload == vsr.st_vsrs_params[i].payload_type)
		{

			TRACEINTO << "LYNC2013_FEC_RED:DEBUG Handling RTV VSR, i:" << i;
			whichi = i;

			// check if we received VSR with only RTV -> remove MSSVC cap if exists
			if (1 == vsr.num_vsrs_params)
			{
				CCapSetInfo capInfoMsSvc (eMsSvcCapCode);
				pRemoteCaps->RemoveCapSet(capInfoMsSvc);
			}

			ST_VSR_PARAMS* pStVsrParam = &(vsr.st_vsrs_params[i]);

			RTVVideoModeDetails rtvVidModeDetails;
			memset(&rtvVidModeDetails, 0, sizeof(RTVVideoModeDetails));

			//                            rtvVidModeDetails.aspectRatio = pStVsrParam->aspect_ratio; //Shmulik: TBD
			rtvVidModeDetails.FR    = pStVsrParam->frame_rate;
			rtvVidModeDetails.Height = pStVsrParam->max_height;
			rtvVidModeDetails.Width  = pStVsrParam->max_width;

			{
				tempMaxRateForRtv = pStVsrParam->min_bitrate /100;
				DWORD remoteBitRate = 0;
				if(pRemoteCaps->GetRtvCapBitRateAccordingToResolution(rtvVidModeDetails.Width, rtvVidModeDetails.Height, remoteBitRate))
					PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd: -RTV AV-MCU remoteBitRate  ",remoteBitRate);
				if(remoteBitRate !=0 )
					tempMaxRateForRtv = min(tempMaxRateForRtv,remoteBitRate);

				tempMaxRateForRtv = min(tempMaxRateForRtv,pRemoteCaps->getMsftRxVideoBwLimitation());

				if(m_pSipCntl->GetCacBwLimitation() != -1)
				{
					tempMaxRateForRtv = min(tempMaxRateForRtv,(unsigned int)m_pSipCntl->GetCacBwLimitation());
				}

				PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd: -RTV AV-MCU tempMaxRateForRtv  ",tempMaxRateForRtv);
			}

			//////////////////////////////////////////////////////
			//LYNC2013_FEC_RED: if FEC/RED is ON we want to calc again with the new rate..
			UpdateVideoBitRateCausedByFecIfNeeded(eRtvCapCode,rtvVidModeDetails.FR,vidBitRate,tempMaxRateForRtv,vsr.st_vsrs_params[i].qualityReportHistogram);

			/*
			if ( m_pTargetMode->GetIsFec() || m_pSipCntl->GetIsRedOn() )  //if (m_pSipCntl->GetIsFecOn() || m_pSipCntl->GetIsRedOn())
			{
				CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
				eAvMcuLinkType avMcuLinkType = eAvMcuLinkNone;

				if (pCommConf)
				{
					CConfParty* pConfParty 	= pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());

					if (pConfParty)
					{
						avMcuLinkType = pConfParty->GetAvMcuLinkType();
						if (avMcuLinkType == eAvMcuLinkSlaveOut)
							vidBitRate = pLocalCaps->GetVideoRate();
					}
				}

				DWORD maxRateDependsOnFecAndRed = min(tempMaxRateForRtv,vidBitRate);


				//DWORD newFecPercent = 0;
				//for (int j=1; j<VSR_NUM_OF_QUALITY_LEVELS; ++j)
				//	if (vsr.st_vsrs_params[i].qualityReportHistogram[j] == 1)
				//		newFecPercent = j;


				UpdateFecParams(vsr.st_vsrs_params[i].qualityReportHistogram);
				//UpdateFecParams(newFecPercent);

				if (m_pSipCntl->GetIsFecOn())
					maxRateDependsOnFecAndRed = m_pParty->CalcVideoRateForMSrtvFEC(maxRateDependsOnFecAndRed/10);
				if (m_pSipCntl->GetIsRedOn())
					maxRateDependsOnFecAndRed = m_pParty->CalcVideoRateForRED(maxRateDependsOnFecAndRed);

				TRACEINTO << "LYNC2013_FEC_RED: set new rate for targetModeTx according to VSR msg:" << maxRateDependsOnFecAndRed
						  << ", tempMaxRateForRtv:" << tempMaxRateForRtv << ", vidBitRate:" << vidBitRate << ", avMcuLinkType:" << (DWORD)avMcuLinkType;

				m_pTargetMode->SetVideoBitRate(maxRateDependsOnFecAndRed, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);
			}
			else
				TRACEINTO << "LYNC2013_FEC_RED: MsSvcDetails.av mcu MaxRateAccordingToMsDescion: there is no FEC/RED: tempMaxRateForRtv:" << tempMaxRateForRtv << ", vidBitRate:" << vidBitRate;
			*/

			rtvVidModeDetails.videoModeType = GetRtvVideoModeTypeByRes(pStVsrParam->max_height, pStVsrParam->max_width);
			BYTE isForceFps = TRUE;
			pRemoteCaps->SetRtvParams(rtvVidModeDetails, kRolePeople, 0, isForceFps);
		}
	}

	CSipComMode* pBestMode = NULL;

	pBestMode = m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, TRUE,FALSE/*intersect with max caps*/);  
	if(pBestMode)
	{
		//COstrStream msg1;
		//m_pTargetMode->Dump(msg1);

		//if(pBestMode->IsMediaOn(cmCapVideo,cmCapTransmit))
		m_pTargetMode->CopyMediaMode(*pBestMode, cmCapVideo, cmCapTransmit, kRolePeople);
		{
			PTRACE(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd: -noa dbg av-mcu update rate ");
			//m_pTargetMode->CopyMediaModeToOppositeDirection(*pBestMode, cmCapVideo, cmCapReceive, kRolePeople);
			//m_pTargetMode->SetMediaMode(pBestMode->GetMediaMode(cmCapVideo, cmCapReceive), cmCapVideo, cmCapTransmit);
			//CapEnum videoAlg = (CapEnum)(m_pTargetMode->GetMediaType(cmCapVideo, cmCapTransmit));

			CapEnum algorithm = (CapEnum)(m_pTargetMode->GetMediaType(cmCapVideo, cmCapTransmit));

			if ( (algorithm == eMsSvcCapCode) && (tempMaxRateForSvc != 0) )
			{
				DWORD oldTargetModeBitRate = m_pTargetMode->GetVideoBitRate(cmCapTransmit,kRolePeople);

				tempMaxRateForSvc = min(tempMaxRateForSvc, oldTargetModeBitRate);
				//tempMaxRateForSvc = 3500;//NOA TMP 240k

				TRACEINTO << "LYNC2013_FEC_RED: av-mcu UPDATE TARGETMODE bitRate(tempMaxRateForSvc):" << tempMaxRateForSvc
						  << ", old bitRate(TargetModeTxVideoRate):" << oldTargetModeBitRate;

				m_pTargetMode->SetVideoBitRate(tempMaxRateForSvc,cmCapTransmit,kRolePeople);
			}
			else if ( (algorithm == eRtvCapCode) && (tempMaxRateForRtv != 0) )
			{
				PTRACE2INT(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd:av-mcu tempMaxRateForRtv  ",tempMaxRateForRtv);
				DWORD rate = m_pTargetMode->GetVideoBitRate(cmCapTransmit,kRolePeople); // very bad patch for BRIDGE-14090. Code needs to be rewritten - need to update remote caps in the next function and not target mode !!!!!!!!!!!!!!!!!!!!!!!!!!
				UpdateVideoBitRateCausedByFecIfNeeded(eRtvCapCode,rtvVidModeDetails.FR,rate,rate,vsr.st_vsrs_params[whichi].qualityReportHistogram);
				tempMaxRateForRtv = min(tempMaxRateForRtv,m_pTargetMode->GetVideoBitRate(cmCapTransmit,kRolePeople) );
				//tempMaxRateForRtv = 3500;//NOA TMP 240k
				m_pTargetMode->SetVideoBitRate(tempMaxRateForRtv,cmCapTransmit,kRolePeople);

			}
		}

		COstrStream msg2;
		m_pTargetMode->Dump(msg2);
		PTRACE2(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd, m_pTargetMode after receive VSR:", msg2.str().c_str());


		if(m_pSipCntl->isMs2013Active() == eMsft2013AvMCU)
		{
			TRACEINTO << " ConfName:" << m_pPartyConfName  << " - avmcu 2013";
			CapEnum algorithm = (CapEnum)(m_pTargetMode->GetMediaType(cmCapVideo, cmCapTransmit));

			if(/*m_pTargetMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) && */(eMsSvcCapCode == algorithm || eRtvCapCode == algorithm) )
			{
				PTRACE(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd, media on");
				m_state = sTRANS_RECVIDEOPREFEUPDATEBRIDGE;
				StartTimer(UPDATEBRIDGESTOUT, BRIDGES_TIME * SECOND);
				SendSingleUpdatePacsiInfoToParty(FALSE); /*isMute = FALSE*/

			}
			else
			{
				PTRACE(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyReceivedRTCPVsrInd, video transmit is off");
			}
		}
		else  //lync 2013 client
		{
			TRACEINTO << " ConfName:" << m_pPartyConfName  <<  " - lync client, not avmcu";
			//update video bridge
			m_state = sTRANS_RECVIDEOPREFEUPDATEBRIDGE;
			StartTimer(UPDATEBRIDGESTOUT, BRIDGES_TIME * SECOND);
			SendVideoChangeAfterVsrMsgToParty();
		}
	}

	POBJDELETE(pBestMode);
}

//////////////////////////////////////////////////////////////
ERtvVideoModeType CSIPTransRTCPVsrInd::GetRtvVideoModeTypeByRes(DWORD height, DWORD width)
{
	DWORD res = height * width;
	if (res >= 1280*720)
	{
		return e_rtv_HD720Symmetric;
	}

	if (res >= 640*480)
	{
		return e_rtv_SD30;
	}

	if (res >= 352*288)
	{
		return e_rtv_CIF30;
	}
	
	return e_rtv_QCIF30;
	
}

//////////////////////////////////////////////////////
void CSIPTransRTCPVsrInd::OnConfBridgesUpdatedUpdateBridges(CSegment* pParam)
{
	if (IsValidTimer(UPDATEBRIDGESTOUT))
	{
		DeleteTimer(UPDATEBRIDGESTOUT);
		PTRACE(eLevelInfoNormal,"CSIPTransRTCPVsrInd::OnConfBridgesUpdatedUpdateBridges: DeleteTimer(UPDATEBRIDGESTOUT) ");
	}


	PTRACE2(eLevelInfoNormal,"CSIPTransRTCPVsrInd::OnConfBridgesUpdatedUpdateBridges, Name ",m_pPartyConfName);
	m_state = sTRANS_RECREINVITEUPDATECHANN;
	UpdateChannelsIfNeeded();
}
/////////////////////////////////////////////////////////////////////
void CSIPTransRTCPVsrInd::OnUpdateBridgesTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSIPTransRTCPVsrInd::OnUpdateBridgesTout");
	WORD callIndex = m_pSipCntl->GetCallIndex();
	DBGPASSERT(callIndex);
	EndTransaction(STATUS_OK);
}

////////////////////////////////////////////////////////////////////
void CSIPTransRTCPVsrInd::OnPartyChannelsUpdated(CSegment* pParam)
{
	m_pCurrentMode->CopyMediaMode(*m_pTargetMode, cmCapVideo, cmCapTransmit, kRolePeople);
	

	COstrStream msg2;
	m_pCurrentMode->Dump(msg2);
	PTRACE2(eLevelInfoNormal,"SIPTransRTCPVsrInd::OnPartyChannelsUpdated, m_pCurrentMode UPDATED:", msg2.str().c_str());

	SendMuteMediaToParty(cmCapTransmit);
	if(m_isKbitOn == TRUE)
	{
		m_isKbitOn = FALSE;
		TRACEINTO << " sending intra from vsr k bit";
		CSegment*  seg = new CSegment;
		ERoleLabel eRole = kRolePeople;
		BYTE bIsGradualIntra = FALSE;
		*seg << (WORD)Fast_Update << (WORD)eRole << (WORD)1 << bIsGradualIntra;
		SendMessageToParty(RMTH230, seg);
	}

	TRACEINTO << "LYNC2013_FEC_RED:DEBUG: PartyID:" << m_pParty->GetPartyId() << ", isNeedToUpdateArtRegardingFEC:" << (DWORD)m_isNeedToUpdateArtRegardingFEC;

	if(m_isNeedToUpdateArtRegardingFEC == TRUE)
	{
		m_isNeedToUpdateArtRegardingFEC = FALSE;
		m_pSipCntl->SendFecOrRedReqToART(cmCapVideo,statOK);
	}

	EndTransaction(STATUS_OK);
}
////////////////////////////////////////////////////////////////////
void CSIPTransRTCPVsrInd::SendSingleUpdatePacsiInfoToParty(BYTE isMute)
{
	CSegment * pSeg = new CSegment;

	m_pTargetMode->Serialize(NATIVE, *pSeg);
	*pSeg << (BYTE)isMute;

	SendMessageToParty(SIP_TRANS_SINGLE_UPDATE_PACSI_INFO_MSG, pSeg);
}

////////////////////////////////////////////////////////////////////
void CSIPTransRTCPVsrInd::UpdateVideoBitRateCausedByFecIfNeeded(CapEnum algorithm, DWORD frameRate, DWORD vidBitRate,
		                                                        DWORD tempMaxRateForSvcOrRtv,APIU16 (&qualityReportHistogram)[VSR_NUM_OF_QUALITY_LEVELS])
{
	if ( m_pTargetMode->GetIsFec() || m_pSipCntl->GetIsRedOn() )  //if (m_pSipCntl->GetIsFecOn() || m_pSipCntl->GetIsRedOn())
	{
		CCommConf*      pCommConf     = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
		eAvMcuLinkType  avMcuLinkType = eAvMcuLinkNone;
		const CSipCaps* pLocalCaps    = m_pSipCntl->GetLocalCaps();

		if (pCommConf)
		{
			CConfParty* pConfParty 	= pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());

			if (pConfParty)
			{
				avMcuLinkType = pConfParty->GetAvMcuLinkType();
				if (avMcuLinkType == eAvMcuLinkSlaveOut)
					vidBitRate = pLocalCaps->GetVideoRate();
			}
		}

		DWORD maxRateDependsOnFecAndRed = min(tempMaxRateForSvcOrRtv,vidBitRate);

		TRACEINTO << "LYNC2013_FEC_RED:DEBUG: algorithm (91=MsSVC,65=RTV):" << algorithm << ", maxRateDependsOnFecAndRed before:" << maxRateDependsOnFecAndRed;

		UpdateFecParams(qualityReportHistogram);

		if (m_pSipCntl->GetIsFecOn())
		{
			if (algorithm==eMsSvcCapCode)
				maxRateDependsOnFecAndRed = (m_pParty->CalcVideoRateForMSsvcFEC(maxRateDependsOnFecAndRed*100,m_pSipCntl->GetFecFractionLossInPercent(),frameRate))/1000;
			else if (algorithm==eRtvCapCode)
				maxRateDependsOnFecAndRed = m_pParty->CalcVideoRateForMSrtvFEC(maxRateDependsOnFecAndRed*100,frameRate);
		}
		if (m_pSipCntl->GetIsRedOn())
			maxRateDependsOnFecAndRed = m_pParty->CalcVideoRateForRED(maxRateDependsOnFecAndRed);

		TRACEINTO << "LYNC2013_FEC_RED:DEBUG: set new rate for targetModeTx according to VSR msg:" << maxRateDependsOnFecAndRed
				<< ", avMcuLinkType:" << (DWORD)avMcuLinkType
				<< ", vidBitRate:" << vidBitRate << ", tempMaxRateForSvcOrRtv:" << tempMaxRateForSvcOrRtv << ", frameRate:" << frameRate;

		TRACEINTO << "LYNC2013_FEC_RED:DEBUG: target algo:" << (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople);

		if (algorithm == (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople) )
		{
			m_pTargetMode->SetVideoBitRate(maxRateDependsOnFecAndRed, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);
			m_pTargetMode->Dump("CSIPTransRTCPVsrInd::UpdateVideoBitRateCausedByFecIfNeeded: LYNC2013_FEC_RED:DEBUG: m_pTargetMode:",eLevelInfoNormal);
		}
		//else
		//	TRACEINTO << "LYNC2013_FEC_RED:DEBUG: target algo:" << (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople);
	}
	else
		TRACEINTO << "LYNC2013_FEC_RED:DEBUG: MsSvcDetails.av mcu MaxRateAccordingToMsDescion: vidBitRate:" << vidBitRate << ", tempMaxRateForSvcOrRtv:" << tempMaxRateForSvcOrRtv;

}

////////////////////////////////////////////////////////////////////
//1. calc FEC percent (packet loss) according to QR.
//2. SetIsFecOn(TRUE/FALSE)
//3. set m_FecFractionLossInPercent = newFecPercent;
void CSIPTransRTCPVsrInd::UpdateFecParams(APIU16 (&qualityReportHistogram)[VSR_NUM_OF_QUALITY_LEVELS])
{

	DWORD newFecPercent = 0;
	DWORD fecFractionLossInPercent = m_pSipCntl->GetFecFractionLossInPercent();

	for (int j=1; j<VSR_NUM_OF_QUALITY_LEVELS; ++j)
		// bridge-15521 AV-MCU sends Quality histogram : 0 0 0 0 0 2 0 0, changed to >= 1
		if (qualityReportHistogram[j] >= 1)
			newFecPercent = j;

	if ( (newFecPercent>0 && fecFractionLossInPercent==0) ||
		 (newFecPercent>=7 && fecFractionLossInPercent<7) ||
		 (newFecPercent>=4 && newFecPercent<=6 && (fecFractionLossInPercent<4 || fecFractionLossInPercent>6)) ||
		 (newFecPercent>=1 && newFecPercent<=3 && fecFractionLossInPercent>3) )
	{
		TRACEINTO << "LYNC2013_FEC_RED:DEBUG: PartyID:" << m_pParty->GetPartyId() << " - FEC status = ON - newFecPercent:" << newFecPercent << ", fecFractionLossInPercent:" << fecFractionLossInPercent;

		m_pSipCntl->SetFecFractionLossInPercent(newFecPercent);
		m_pSipCntl->SetIsFecOn(TRUE);

		m_isNeedToUpdateArtRegardingFEC = TRUE;
	}
	else if(newFecPercent==0 && m_pSipCntl->GetIsFecOn() && m_pSipCntl->GetFecFractionLossInPercent()>0)
	{
		TRACEINTO << "LYNC2013_FEC_RED:DEBUG: PartyID:" << m_pParty->GetPartyId() << " - Set FEC OFF - newFecPercent:" << newFecPercent << ", fecFractionLossInPercent:" << fecFractionLossInPercent;
		m_pSipCntl->SetFecFractionLossInPercent(0);
		m_pSipCntl->SetIsFecOn(FALSE);

		m_isNeedToUpdateArtRegardingFEC = TRUE;
	}
	else
	{
		TRACEINTO << "LYNC2013_FEC_RED:DEBUG: PartyID:" << m_pParty->GetPartyId() << " - NO changes are needed - newFecPercent:" << newFecPercent << ", fecFractionLossInPercent:" << fecFractionLossInPercent;
	}

}
