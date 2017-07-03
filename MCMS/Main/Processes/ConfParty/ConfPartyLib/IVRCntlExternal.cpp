// IVRCntlExternal.cpp
// Created on: Oct 31, 2012 for AT&T external IVR driven by DMA
// Author: yoella

///////////////////////////////////////////////////////////////////////////
#include "IVRCntlExternal.h"

#include "IvrPackageStatusCodes.h"
#include "MccfIvrPackageResponse.h"

#include "MscIvr.h"
#include "DialogStart.h"

#include "MediaTypeManager.h"
#include "MccfHelper.h"

#include "FilesCache.h"

////////////////////////////////////////////////////////////////////////////
#include "IVRDtmfColl.h"
#include "ConfAppMngr.h"
#include "IVRSlidesList.h"

///////////////////////////////////////////////////////////////////////////
#include "SysConfigKeys.h"

///////////////////////////////////////////////////////////////////////////
#define MESSAGE_PLAY_END_TIMER ((WORD)200)
#define DIALOG_TERM_TIMER ((WORD)201)

///////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CIvrCntlExternal)
	// Application Server's driven external IVR
	ONEVENT(EXTERNAL_IVR_PALY_MUSIC,     ANYCASE, CIvrCntlExternal::OnPlayMusic)
	ONEVENT(EXTERNAL_IVR_SHOW_SLIDE,     ANYCASE, CIvrCntlExternal::OnShowSlide)
	ONEVENT(EXTERNAL_IVR_COLLECT_DIGITS, ANYCASE, CIvrCntlExternal::OnCollectDigits)
	ONEVENT(IVR_END_OF_FEATURE,          ANYCASE, CIvrCntlExternal::OnEndFeature)
	ONEVENT(EXTERNAL_IVR_DIALOG_START,   ANYCASE, CIvrCntlExternal::OnDialogStart)

	ONEVENT(DIALOG_TERM_TIMER,           ANYCASE, CIvrCntlExternal::OnDialogTermTimer) // Dtmf on connect party
	ONEVENT(IP_DTMF_INPUT_IND,           ANYCASE, CIvrCntlExternal::HandlePartyDtmfForExternalIvr)
	ONEVENT(CAM_TO_IVR_PARTY_MEDIA_CONNECTED,  ANYCASE, CIvrCntlExternal::HandleMediaConnectionForExternalIvr)
	ONEVENT(CAM_TO_IVR_PARTY_MEDIA_DISCONNECTED,  ANYCASE, CIvrCntlExternal::HandleMediaDisconnectionForExternalIvr)

	ONEVENT(EXTERNAL_IVR_DIALOG_TIMEOUT_MASTER_TO_SLAVE,      ANYCASE, CIvrCntlExternal::OnDialogTermExternal)
	ONEVENT(EXTERNAL_IVR_DTMF_BARGE_IN,                       ANYCASE, CIvrCntlExternal::OnMediaBargedInByDTMF)
	ONEVENT(EXTERNAL_IVR_DTMF_BARGE_IN_MASTER_TO_SLAVE,       ANYCASE, CIvrCntlExternal::OnMediaBargeInExternal)
	ONEVENT(EXTERNAL_IVR_PLAY_MEDIA_COMPLETE_SLAVE_TO_MASTER, ANYCASE, CIvrCntlExternal::OnMediaCompletedExternal)
PEND_MESSAGE_MAP(CIvrCntlExternal, CIvrCntl);

////////////////////////////////////////////////////////////////////////////
//                        CIvrCntlExternal
////////////////////////////////////////////////////////////////////////////
CIvrCntlExternal::CIvrCntlExternal(CTaskApp* pOwnerTask, const DWORD dwMonitorConfId):CIvrCntl(pOwnerTask, dwMonitorConfId)
{
	TRACEINTO;
	m_DialogContextByID.clear();
	m_isBargeInAllowed = FALSE;
	m_isCollectDialog = FALSE;
	m_isAudioOnly = FALSE;
	m_isPartyAudioConnected = FALSE;
	m_isPartyVideoConnected = FALSE;

	m_dialogStartTime = 0;
	taskCounter = 0;
    m_sDialogBargedInByTIPMaster.clear();
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrCntlExternal::~CIvrCntlExternal()
{
}

//--------------------------------------------------------------------------
void CIvrCntlExternal::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	switch (opCode)
	{
		case AUD_DTMF_IND_VAL:
			// go to DTMF Collector StateMachine
			TRACEINTO << "Received dtmf indication, passing to dtmf collector";
			if (m_pDtmfCollector)
			{
				PASSERTMSG(m_pDtmfCollector->GetState() == NOTACTIVE, "dtmf collector still not in active state!!");
				m_pDtmfCollector->HandleEvent(pMsg, msgLen, opCode);
			}
			else
				PASSERTMSG(TRUE, "no dtmf collector for AUD_DTMF_IND_VAL");
			break;

		case FIRST_DTMF_WAS_DIALED:
			// go to Sub IVR Feature StateMachine
			TRACEINTO << "Received \"first dtmf\" indication, passing to IVR feature";
			if (m_pIvrSubGenSM)
				m_pIvrSubGenSM->HandleEvent(pMsg, msgLen, opCode);
			else
			{
				if(IsValidPObjectPtr(m_pParty) && m_pParty->GetIsTipCall() && (m_pParty->GetTipPartyType() == eTipMasterCenter))
				{
					TRACEINTO << " TIP master EP received DTMF bargein while not in active IVR subfeature (m_pIvrSubGenSM is NULL).";
					// TODO Guy- currently handling only one dialog. replace with implementation of concurrect dialogs.
					TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.begin();
					PASSERTMSG_AND_RETURN(dit == m_DialogContextByID.end(), "dialog not found for DTMF barge-in event");
					CDialogContext* dialog_context = dit->second;
					if (dialog_context->GetIsBargeIn())
					{
						CSegment seg;
						seg << dit->first;
						OnMediaBargedInByDTMF(&seg);
					}
				}
				PTRACE(eLevelError, "no feature for FIRST_DTMF_WAS_DIALED");
			}
			break;

		case DTMF_STRING_IDENT:
			// go to Sub IVR Feature StateMachine
			TRACEINTO << "Received dtmf string indication, passing to dtmf collector";
			if (m_pIvrSubGenSM)
				m_pIvrSubGenSM->HandleEvent(pMsg, msgLen, opCode);
			else
				PTRACE(eLevelError, "no feature for DTMF_STRING_IDENT");
			break;

		case IVR_SUB_FEATURE_MESSG:
			if (m_pIvrSubGenSM)
			{
				DWORD msgLen = 0;
				DWORD opCode = 0;
				*pMsg >> opCode;
				//m_pIvrSubGenSM->HandleEvent(pMsg, msgLen, opCode);
			}
			break;

		case DTMF_IVR_CLEAR_BUFFER:
			if (m_pDtmfCollector)
				m_pDtmfCollector->ResetDtmfBuffer();

			break;

		case IVR_END_OF_FEATURE:
			OnEndFeature(pMsg);
			break;

		default:
			DispatchEvent(opCode, pMsg);
			break;
	}
}


void CIvrCntlExternal::OnEndFeature(CSegment* pParam)
{
	// Delete current Feature
	POBJDELETE(m_pIvrSubGenSM);

	// sets end status
	WORD status;
	*pParam >> status;
	TRACEINTO << " feature ended with status " << status;
}


void CIvrCntlExternal::OnMediaCompletedExternal(CSegment* pParam)
{
	TRACEINTO;
	if(!m_pParty->GetIsTipCall() && (m_pParty->GetTipPartyType() != eTipMasterCenter))//BRIDGE-3510
	{
		PASSERTMSG_AND_RETURN(TRUE, "CIvrCntlExternal::OnMediaCompletedExternal received in TIP slave party");
	}
	// if there is an active feature (slide), end it to prepare for next feature
	if (IsValidPObjectPtr(m_pIvrSubGenSM))
	{
		CSegment seg;
		seg << STATUS_OK;
		OnEndFeature(&seg);
	}
	std::string dialogId, status;
	*pParam >> dialogId >> status;
	OnMediaCompleted(dialogId, status);
}


void CIvrCntlExternal::OnMediaCompleted(std::string& dialogId, std::string& status)
{

	TRACEINTO << "dialog id: " << dialogId << ", status: " << status;

	CDialogContext *dialog_context = NULL;
	TASKS_BY_DIALOG::iterator it = m_DialogContextByID.find(dialogId);
	PASSERTMSG_AND_RETURN(it == m_DialogContextByID.end(), "CIvrCntlExternal::OnMediaCompleted- dialog id not found");

	dialog_context = it->second;
	// update the status of the "promptinfo" response section
	if ((status.length() > 0) &&
		(dialog_context->GetIsPlayMediaRequested() || dialog_context->GetIsShowSlideRequested()))
		OnPlayFileResult(mft_Audio, dialogId, status);
	BOOL is_main_EP = (m_pParty->GetTipPartyType() == eTipMasterCenter) || (m_pParty->GetTipPartyType() == eTipNone);
	// initiate the digit collection
	if (is_main_EP )
	{
		if (dialog_context->GetIsCollectRequested())
		{
			// if the media has stopped, stop the collection and send
			if (status.compare("stopped") == 0)
				OnCollectDigitsResult(dialogId, "", status);
			else
				OnCollectDtmfDigits(dialogId, dialog_context->GetCollectElement());
		}
		else
			CheckIsDialogCompletedAndRespond(dialogId, FALSE);
	}
	else
	{
		PASSERTSTREAM(m_pParty->GetTipPartyType() != eTipSlaveAux, "unexpected party TIP type " << (int)m_pParty->GetTipPartyType());
		TRACEINTO << "media completed for TIP slave. deleting context";
		m_DialogContextByID.erase(it);
	}
}
//--------------------------------------------------------------------------
void CIvrCntlExternal::SetFeaturesToDo(DWORD featureType)
{
  PASSERT_AND_RETURN(!m_pParty);

  std::ostringstream msg;
  msg << "CIvrCntlExternal::SetFeaturesToDo() - PartyName:" << m_pParty->GetName();

  switch(featureType)

  {
      case (PLAY_AUDIO_EXTERNAL):// conference password entry
      {
          m_featuresList[IVR_STAGE_AUDIO_EXTERNAL].bToDo = 1;
          break;
      }

      case (COLLECT_DIGITS_EXTERNAL):// Any digit entry
	  {
		 m_featuresList[IVR_STAGE_COLLECT_EXTERNAL].bToDo = 1;
		 break;
	  }

      default:
      {
    	 //TRACE...
         break;
      }

  }
}


//--------------------------------------------------------------------------
void CIvrCntlExternal::Start(CSegment* pParam)
{

	TRACEINTO ;

	m_state = ACTIVE;

    PASSERT_AND_RETURN(!m_pConfApi);

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	m_isAudioOnly =  pConfParty->GetVoice();
	TRACEINTO << "isAudioOnly: " << m_isAudioOnly;
  if (m_pDtmfCollector)
  {
     m_pDtmfCollector->ResetDtmfBuffer();
     m_pDtmfCollector->EndIvrPartSession();
  }

   // sign as IVR (and removes from 'operator help queue')
   OnStartIVR();

   // set feature params
   if (m_pIvrSubGenSM != NULL)
   StartNewFeature();
   return;
}

///////////////////////////////// Ext. IVR phase II - dialog handled by IVR ////////////////////
void CIvrCntlExternal::OnDialogStart(CSegment* pParam)
{
	DialogState state;
	DWORD previous_mcms_delay = 0;
	DWORD term_timeout_ms = 0;
	*pParam >> state >> previous_mcms_delay;

	TRACEINTO << "dialogID: " << state.dialogID;

	CSmallString ivr_delay_string;
	if (previous_mcms_delay > 0)
		ivr_delay_string << "dialog was delayed " << previous_mcms_delay << "msecs until IVR started.";
	PASSERTSTREAM_AND_RETURN(!state.baseObject, "CIvrCntlExternal::OnDialogStart MscIvr object is not valid");
	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	DialogStart* dialogStart = (DialogStart*)(mscIvr.m_pResponseType);
	PASSERTSTREAM_AND_RETURN(!dialogStart, "CIvrCntlExternal::OnDialogStart dialogStart object is not valid");
	DialogElementType& dialog = dialogStart->m_dialog;

	BOOL is_barge_in_by_previous_dtmf = FALSE;
	CDialogContext *dialog_context = NULL;

	// BRIDGE-8290 check if dtmf was already collected in a previous dialog - if it is, end treatment
	// (in a barge-in scenario, TIP master will not be expecting a 'media complete' message from slave_axu)
	if ((m_pParty->GetTipPartyType() == eTipSlaveAux) && (m_sDialogBargedInByTIPMaster.length()>0) && (m_sDialogBargedInByTIPMaster == state.dialogID))
	{
		TRACEINTO << "TIP slave received dialog that was already barged by dtmf (in TIP master) - complete without playing.";
		m_sDialogBargedInByTIPMaster.clear();
		return;
	}

	TASKS_BY_DIALOG::iterator it = m_DialogContextByID.find(state.dialogID);
	// insert dialog info
	if (it == m_DialogContextByID.end())
	{
		TRACEINTO << "received new dialog with id \"" << state.dialogID << "\". " << ivr_delay_string;
		dialog_context = new CDialogContext(state);
		it = m_DialogContextByID.insert(std::make_pair<std::string, CDialogContext*>(state.dialogID, dialog_context)).first;
	}
	else
	{
		TRACEINTO << "received EXISTING dialog with id \"" << state.dialogID << "\"";
		dialog_context = it->second;
	}
	if (!IsPartyMediaConnected())
	{
		TRACEINTO << " party media not connected!! returning \"stopped\" status to DMA.";
		std::string status = "stopped";
		OnMediaCompleted(state.dialogID, status);
	}
	if (dialog_context->GetIsBargeIn())
		m_isBargeInAllowed = TRUE;
	m_dialogStartTime = SystemGetTickCount();

	TRACEINTO << "is tip call: " << m_pParty->GetIsTipCall() << ", tip party type: " << m_pParty->GetTipPartyType();

	ETipPartyTypeAndPosition TIP_type = m_pParty->GetTipPartyType();
	if ((TIP_type == eTipMasterCenter || TIP_type == eTipNone))
	{
		if (dialog_context->GetIsCollectRequested())
		{
			m_isCollectDialog = TRUE;
			CollectElementType* collect_element = dialog_context->GetCollectElement();
			if (collect_element->m_termTimeOut.IsAssigned())
			{
				term_timeout_ms = decodeTimeDesignation(collect_element->m_termTimeOut);
			}
			// barge in setting
			if (m_pDtmfCollector && m_isBargeInAllowed)
				m_pDtmfCollector->InformAboutFirstNumbPressed();
			if (term_timeout_ms > 0 && previous_mcms_delay < term_timeout_ms)
			{
				TRACEINTO << " Starting term timeout of " << (term_timeout_ms - previous_mcms_delay)  << " ms";
				CSegment *pSeg = new CSegment();
				*pSeg << state.dialogID;
				StartTimer(DIALOG_TERM_TIMER, term_timeout_ms/10, pSeg);
			}
			else
				TRACEINTO << " no term timeout defined for the dialog";
			if (m_pDtmfCollector)
			{
				if (collect_element->m_clearDigitBuffer.IsAssigned() && collect_element->m_clearDigitBuffer == true)
				{
					TRACEINTO << "ClearDigitBuffer= TRUE. dtmf collection buffer cleared";
					m_pDtmfCollector->ResetDtmfBuffer();
					collect_element->m_clearDigitBuffer = false;
				}
				else if (m_isBargeInAllowed && (m_pDtmfCollector->IsDtmfInBuffer() > 0) && (m_pParty->GetTipPartyType() == eTipNone || (m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter)))
				{
					CSmallString msg = "previous dtmf buffer detected - continuing dialog without playing media";
					if (m_pParty->GetTipPartyType() == eTipMasterCenter)
						msg << ", slave_aux EP will not receive the dialogstart event";
					TRACEINTO << msg.GetString();
					is_barge_in_by_previous_dtmf = TRUE;
					m_pDtmfCollector->ResetInformAboutFirstNumbPressed();
					std::string status = "bargein";
					OnMediaCompleted(state.dialogID, status);
				}
			}
		}
		else
		{
			TRACEINTO << " No collect element found - starting DEFAULT term timeout of 15 seconds";
			CSegment *pSeg = new CSegment();
			*pSeg << state.dialogID;
			StartTimer(DIALOG_TERM_TIMER, 15*SECOND, pSeg);
		}
	}

	// check if necessary to pass the IVR dialog to TIP slave_aux for audio playback
	if (!is_barge_in_by_previous_dtmf && dialog_context->GetIsPlayMediaRequested() && m_pParty->GetIsTipCall() && (m_pParty->GetTipPartyType() == eTipMasterCenter))
	{
		TRACEINTO << " Forwarding IVR dialogstart to TIP slave.";
		DialogState new_state = state;
		new_state.baseObject = ((MscIvr*)state.baseObject)->NewCopy();
		CSegment pSeg;
		pSeg << new_state;
		m_pConfApi->PartytoPartyCntlMsgFromMasterToSlave(m_rsrcPartyId, (WORD)eTipSlaveAux, (WORD)EXTERNAL_IVR_DIALOG_START, &pSeg);
	}
	BOOL playing_media = PlayNextMedia(state.dialogID);
	// sanity timer check- have we delayed too much already?
	if ((term_timeout_ms > 0) && (previous_mcms_delay > term_timeout_ms))
	{
		PASSERTSTREAM(true, "dialog id \"" << state.dialogID << "\" has been delayed past it's term timeout!");
		std::string status = "stopped";
		if (m_isCollectDialog)
			OnCollectDigitsResult(state.dialogID, "", status);
		else
			OnMediaCompleted(state.dialogID, status);
	}
	if (!playing_media)
	{
		TRACEINTO << "PlayNextMedia returned FALSE- completing dialog";
		std::string status = "";
		OnMediaCompleted(state.dialogID, status);
	}

	// now we wait for message complete acks.
}

BOOL CIvrCntlExternal::PlayNextMedia(std::string& dialogId)
{
	BOOL has_next_media = FALSE;
	CDialogContext *dialog_context = NULL;
	MediaElementType* media_element = NULL;
	ETipPartyTypeAndPosition TIP_type = m_pParty->GetTipPartyType();

	TASKS_BY_DIALOG::iterator it = m_DialogContextByID.find(dialogId);
	PASSERTSTREAM_AND_RETURN_VALUE(it == m_DialogContextByID.end(), "CIvrCntlExternal::PlayNextMedia missing dialog context", FALSE);
	dialog_context = it->second;
	BOOL is_first_media = dialog_context->GetIsFirstMedia();
	BOOL is_audio_only = FALSE;

	media_element = dialog_context->GetNextMediaElement();
	has_next_media = media_element != NULL;
	BOOL already_playing_next = FALSE;
	BOOL playing_initiated = TRUE;
	if (has_next_media)
	{
		// sanity check for media playing iteration
		PASSERTSTREAM_AND_RETURN_VALUE(dialog_context->GetPlayedMediaCounter() >= dialog_context->GetMediaListSize(), "unexpected attempt to play media- only " << dialog_context->GetMediaListSize() << " media items were found int the dialog", FALSE);
		MediaFileTypeEnum ivrFileType = CMediaTypeManager::DeriveMediaType(media_element->m_type, media_element->m_loc);
		TRACEINTO << "party " << m_pParty->GetName() << " of TIP type " << (int)m_pParty->GetTipPartyType() << " to play next media \"" << media_element->m_loc << "\", file type " << ivrFileType;
		if (is_first_media &&
				((TIP_type == eTipMasterCenter) || (TIP_type == eTipNone)))
			CMccfIvrPackageResponse::ResponseReportMsg(dialog_context->getDialogStateForResponse(), mccf_ivr_OK);
		switch (ivrFileType)
		{
		case mft_Image:
		{
			switch (TIP_type)
			{
			case eTipMasterCenter:
			case eTipNone:
				is_audio_only = IsPartyAudioOnly();
				if (is_audio_only)		//BRIDGE-4374
				{
					// set dialog status to "stopped". if there are music files that play successfully, this status will be overwritten
					OnPlayFileResult(mft_Image, dialogId, "stopped");
					has_next_media = dialog_context->GetIsPlayMediaRequested();
				}
				else
					playing_initiated = OnPlayFile(ivrFileType, dialogId, media_element, dialog_context->GetIsBargeIn());
				// if play file failed, discontinue the dialog
				if (!playing_initiated)
					break;
				if (!dialog_context->GetIsPlayMediaRequested() && !is_audio_only)
				{
					// image and no music- we're finished here (no wait for ack)
					OnPlayFileResult(mft_Image, dialogId, "completed");
					has_next_media = FALSE;
				}
				if (has_next_media) 	// there are more files to play
				{
					// non-tip advances to next file (to play audio)
					if (TIP_type == eTipNone && is_first_media)
					{
						dialog_context->AdvanceToNextMediaElement();
						already_playing_next = TRUE;
						has_next_media = PlayNextMedia(dialogId);
					}
				}

				break;
			case eTipSlaveAux:
				TRACEINTO << " TIP slave party- skipping image file";
				//dialog_context->AdvanceToNextMediaElement();
				//has_next_media = PlayNextMedia(dialogId);
				break;
			default:
				PASSERTMSG_AND_RETURN_VALUE(TRUE, "unexpected TIP party type handling media", FALSE);
			}
			break;
		}
		case mft_Audio:
		{
			switch (TIP_type)
			{
			case eTipSlaveAux:
				// BRIDGE-8290 check for barge-in from TIP master
				if (m_sDialogBargedInByTIPMaster.length() > 0 && dialogId == m_sDialogBargedInByTIPMaster)
				{
					TRACEINTO << "TIP slave received dialog that was already barged by dtmf (in TIP master) - complete without playing.";
					// end playing (OnMediaCompleted will be called)
					has_next_media = FALSE;
					m_sDialogBargedInByTIPMaster.clear();
					break;
				}
			case eTipNone:
				playing_initiated = OnPlayFile(ivrFileType, dialogId, media_element, dialog_context->GetIsBargeIn());
				break;
				//			default:
				//				dialog_context->AdvanceToNextMediaElement();
			case eTipMasterCenter:
				TRACEINTO << " TIP master party skipping audio file";
				if (dialog_context->GetIsShowSlideRequested())
				{
					//dialog_context->AdvanceToNextMediaElement();
					if (is_first_media)
					{
						PASSERTMSG(is_first_media, "Dialog's media list doesn't start with image");
						dialog_context->AdvanceToNextMediaElement();
						already_playing_next = TRUE;
						has_next_media = PlayNextMedia(dialogId);
					}

				}
				break;
			default:
				PASSERTMSG_AND_RETURN_VALUE(TRUE, "unexpected TIP party type handling media", FALSE);
			}
			break;
		}
		case mft_Video:
		default:
			TRACEINTO << " stopping media-  unsupported file";
			// unsupported media types
			OnPlayFileResult(mft_Unknown, dialogId, "stopped");
			has_next_media = FALSE;
			PASSERTSTREAM(TRUE, "unexpected media type: " << (int)TIP_type);
		}
		// if we entered with a file to play next, we now:
		// * check that we were able to initiate the play
		// * advance the media file iterator. it will be checked when the play 'ack' is handled
		if (has_next_media)
		{
			if (!playing_initiated)
			{

				TRACEINTO << " unsuccessfull play file attempt. the dialog will be stopped.";
				OnPlayFileResult(mft_Unknown, dialogId, "stopped");
				has_next_media = FALSE;
			}
			else if (already_playing_next)
				TRACEINTO << " already initiated \"next media\"";
			else
			{
				TRACEINTO << " advancing reference to \"next media\"";
				dialog_context->AdvanceToNextMediaElement();
			}
		}
	}
	else
		TRACEINTO << "party " << m_pParty->GetName() << " of TIP type " << (int)m_pParty->GetTipPartyType() << ": no prompt media elements left to play";

	return has_next_media;
}
void CIvrCntlExternal::OnCollectDtmfDigits(std::string dialogId, CollectElementType* collect)
{
	bool isClearDigitBuffer = false;
	// clearing dtmf buffer is handled in the beginning of the dialg (in OnDialogStyart)
	WORD dtmf_max_digits = 0;
//	if (collect->m_maxDigits.IsAssigned())
//		dtmf_max_digits = collect->m_maxDigits.m_uint;
	size_t time_value_ms = 0;
	DWORD dtmf_interdigit_timeout = 30; // in seconds
	time_value_ms = 0;
	if (collect->m_interDigitTimeOut.IsAssigned())
	{
		time_value_ms = decodeTimeDesignation(collect->m_interDigitTimeOut);
		if (time_value_ms > 0)
			dtmf_interdigit_timeout = (DWORD)(time_value_ms/1000);
	}
	std::string dtmf_termchar = "#";
	if (collect->m_termChar.IsAssigned())
		dtmf_termchar = collect->m_termChar.value();
//
//	std::string dtmf_escapechar = "";
//	if (collect_node.m_escapeKey.IsAssigned())
//		dtmf_escapechar = collect_node.m_escapeKey.value();

	CSmallString msg;
	msg << "CIvrCntlExternal::OnCollectDigits - " <<
			"dtmf_termchar: " << dtmf_termchar << ", dtmf_interdigit_timeout: " << dtmf_interdigit_timeout;
	PTRACE(eLevelInfoNormal, msg.GetString());

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	if (!IsValidPObjectPtr(m_pDtmfCollector))
	{
		PASSERTSTREAM(TRUE, "CIvrCntlExternal::OnPlayFile missing m_pDtmfCollector for dialog");
		OnCollectDigitsResult(dialogId, "", "stopped");
		return;
	}
		m_pDtmfCollector->ResetInformAboutFirstNumbPressed();

	m_pIvrSubGenSM = (CIvrSubBaseSM*) new CIvrSubExternalInputCollect(dialogId, dtmf_termchar.c_str()[0], dtmf_interdigit_timeout, dtmf_max_digits, isClearDigitBuffer/*,dtmf_escapechar*/);
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogId);
	PASSERTSTREAM_AND_RETURN(dit == m_DialogContextByID.end(), "CIvrCntlExternal::OnPlayFile missing dialog context for dialog");
	CDialogContext* dialog_context = dit->second;
	m_pIvrSubGenSM->SetAllowDtmfBargeIn(dialog_context->GetIsBargeIn());

	m_state = ACTIVE;

	// set feature params
	StartNewFeature();
	OnStartIVR();
}

//--------------------------------------------------------------------------
void CIvrCntlExternal::OnMediaBargedInByDTMF(CSegment *pSeg)
{
	TRACEINTO;
	std::string dialogIdStr, statusStr = "bargein";
	*pSeg >> dialogIdStr;
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogIdStr);
	PASSERTSTREAM_AND_RETURN(dit == m_DialogContextByID.end(), "CIvrCntlExternal::OnMediaBargedInByDTMF missing dialog context for dialog");
	CDialogContext* dialog_context = dit->second;
	if  (!dialog_context->GetIsPlayMediaRequested())
	{
		TRACEINTO << "dtmf barge-in reported but no audio was requested, party " << m_pParty->GetName();
		return;
	}

	if ((m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter) || (m_pParty->GetTipPartyType() == eTipNone))
	{
		TRACEINTO << "barge-in reported while playing media, party " << m_pParty->GetName();
		if (m_pParty->GetTipPartyType() == eTipMasterCenter)
		{
			TRACEINTO << "sending EXTERNAL_IVR_DTMF_BARGE_IN_MASTER_TO_SLAVE to slave EP";
			CSegment pSegment;
			pSegment << dialogIdStr;
			m_pConfApi->PartytoPartyCntlMsgFromMasterToSlave(m_rsrcPartyId, (WORD)eTipSlaveAux, (WORD)EXTERNAL_IVR_DTMF_BARGE_IN_MASTER_TO_SLAVE, &pSegment);
		}
		else if (m_pParty->GetTipPartyType() == eTipNone)
		{
			if (IsValidPObjectPtr(m_pIvrSubGenSM))
			{
				m_pIvrSubGenSM->StopPlayMessage();
				CSegment seg;
				seg << STATUS_OK;
				OnEndFeature(&seg);
				m_pConfApi->SendCAMRemoveFeatureFromList(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_REMOVE_FEATURE, 1);
			}
		}
	}
	else
	{
		// media is barged and collect is not relevant- delete dialog
		TRACEINTO << "barge-in reported in TIP party type:" << (int)m_pParty->GetTipPartyType() << ". stopping play";
		if (IsValidPObjectPtr(m_pIvrSubGenSM))
			m_pIvrSubGenSM->StopPlayMessage();
	}
	OnMediaCompleted(dialogIdStr, statusStr);


}

void CIvrCntlExternal::OnMediaBargeInExternal(CSegment *pSeg)
{
	std::string dialogIdStr;
	*pSeg >> dialogIdStr;
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogIdStr);
	PASSERTSTREAM_AND_RETURN(m_pParty->GetTipPartyType() != eTipSlaveAux, "received external bargein with unknown dialog id \"" << dialogIdStr << "\" in TIP party type " << (int)m_pParty->GetTipPartyType());
	m_sDialogBargedInByTIPMaster = dialogIdStr;

	if (dit == m_DialogContextByID.end())
	{
		TRACEINTO << "TIP slave EP received barge-in for unknown dialog id \"" << dialogIdStr << "\". saving for next dialog.";
		return;
	}
	//PASSERTSTREAM_AND_RETURN(dit == m_DialogContextByID.end(), "CIvrCntlExternal::OnMediaBargeInExternal missing dialog context for dialog");
	CDialogContext* dialog_context = dit->second;
	if ((m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipSlaveAux))
	{
		// media is barged and collect is not relevant- delete dialog
		TRACEINTO << "barge-in received from master in TIP party type:" << (int)m_pParty->GetTipPartyType() << ". ending dialog";
		if (IsValidPObjectPtr(m_pIvrSubGenSM))
		{
			m_pIvrSubGenSM->StopPlayMessage();
			CSegment seg;
			seg << STATUS_OK;
			OnEndFeature(&seg);
			m_pConfApi->SendCAMRemoveFeatureFromList(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_REMOVE_FEATURE, 1);
			std::string status = "";
			OnMediaCompleted(dialogIdStr, status);
		}
	}
}
//--------------------------------------------------------------------------
void CIvrCntlExternal::OnPlayFileResult(MediaFileTypeEnum type, std::string dialogIdStr, std::string statusStr)
{
	TRACEINTO;


	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogIdStr);

	PASSERTSTREAM_AND_RETURN(dit == m_DialogContextByID.end(), "CIvrCntlExternal::OnPlayFileResult missing dialog context for dialog");
	CDialogContext* dialog_context = dit->second;

	if ((m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter) || (m_pParty->GetTipPartyType() == eTipNone))
	{
		Event* responseEvent = dialog_context->GetRespnseEvent();
		// update response
		responseEvent->m_dialogExit.m_promptInfo.m_termMode = statusStr;
	}
	else
		TRACEINTO << "skipping update of the response status- not a TIP master or stand-alone EP";

}

//--------------------------------------------------------------------------
void CIvrCntlExternal::RecivedPlayMessageAckTimer(CSegment *pSeg)
{
	TRACEINTO;
	RecivedPlayMessageAck();
}

//--------------------------------------------------------------------------
BOOL CIvrCntlExternal::OnPlayFile(MediaFileTypeEnum ivrFileType, std::string& dialogId, MediaElementType *mediaElement, BOOL isBargeInAllowed)
{
	BOOL able_to_play = TRUE;
	PASSERTSTREAM_AND_RETURN_VALUE(mediaElement == NULL, " CIvrCntlExternal::OnPlayFile - invalid media element", FALSE);
	TRACEINTO << "dialogID:" << dialogId << std::hex << ", media:" << mediaElement << ", URL:" << mediaElement->m_loc;
#if 0 // TODO: Guy - fix the mechanism of fetching URL by Dialog ID and media type
	const CLocalFileDescriptor* file = GetRequestedExternalFile(dialogId, ivrFileType);
#else
	const CLocalFileDescriptor* file = CFilesCache::const_instance().fileDescriptor(mediaElement->m_loc);
#endif
	able_to_play = file != NULL;
	PASSERTSTREAM_AND_RETURN_VALUE(!able_to_play, " CIvrCntlExternal::OnPlayFile - file name " << mediaElement->m_loc->c_str() << " is not found", FALSE);

	std::string baseFolder;
	GetBaseFolderByMediaType(ivrFileType, mediaElement->m_loc, baseFolder);

	std::string path = baseFolder + '/';
	path += file->path();

	// update CAM (list) for new feature
	// TRACEINTO << " - SendCAMGeneralActionCommand - start feature";
	//pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE, 1);

//	if(m_pIvrSubGenSM != NULL)
//	{
//		PTRACE(eLevelError, "Failed, Can't start feature while other exists");
//		// TODO Guy - send error to mccf if concurrent ext. IVR are not supported.
//	}

#if 0
	DWORD duration = file->duration(); // currently, it defaults to static_cast<size_t>(-1), meaning *infinity*, it is unsupported by Embedded
#else
	DWORD duration = 5; // some hard-coded value
	TRACEINTO << "PATCH: using a hard-coded duration:" << duration;
#endif

	bool isAudioOnly = false; //BRIDGE-4374

	switch (ivrFileType)
	{
	case mft_Audio:
		m_pConfApi->SendCAMAddFeatureToList(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL, 1);
		m_pIvrSubGenSM = new CIvrSubAudioExternal(path.c_str(), duration, dialogId);
		if(m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter) // BRIDGE-3510
		{
			return FALSE; //don't send IVR because slave_aux will do this.
			//TODO: where to delete m_pIvrSubGenSM in this case??
		}
		break;

	case mft_Image:
	{
		CIVRSlidesList* pSlidesList = ::GetpSlidesList();

		if (!pSlidesList)
		{
			TRACEINTO << "create slide list";
			pSlidesList = new CIVRSlidesList;
			::SetpSlidesList(pSlidesList);
		}

        //remove the extersion name for the slide request
        std::string file_name = file->path();

		std::string slideName = GetImageNameWithoutExtension(file_name);
		TRACEINTO << "slideName is:"<< slideName;
		PASSERTSTREAM_AND_RETURN_VALUE("" == slideName, " CIvrCntlExternal::OnPlayFile - slide file extension is not supported " << mediaElement->m_loc->c_str(), FALSE);

		std::string slidePath = baseFolder + '/' +slideName;
		//const char* fileName = file->path().c_str();
		pSlidesList->AddSlide(slidePath.c_str(), true);


		m_pIvrSubGenSM = new CIvrSubVideoExternal(slidePath.c_str(), duration, dialogId);
		break;
	}

	case mft_Video:
		m_pIvrSubGenSM = new CIvrSubVideoExternal(path.c_str(), duration, dialogId);
		break;

	default:
		break;
	}
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogId);
	PASSERTSTREAM_AND_RETURN_VALUE(dit == m_DialogContextByID.end(), "CIvrCntlExternal::OnPlayFile missing dialog context for dialog",FALSE);
	CDialogContext* dialog_context = dit->second;
	m_pIvrSubGenSM->SetAllowDtmfBargeIn(dialog_context->GetIsBargeIn());
	m_state = ACTIVE;


	// set feature params
	StartNewFeature();
	OnStartIVR();

	return able_to_play;

}

//--------------------------------------------------------------------------
void CIvrCntlExternal::RecivedPlayMessageAckMultipleMedia()
{
	PASSERTMSG_AND_RETURN(m_pIvrSubGenSM == NULL, "Received play message ack with no IVR feature object. dialog id unknown.");
	std::string dialogId = m_pIvrSubGenSM->GetDialogID();
	BOOL playing_next = FALSE;
	BOOL is_EP_playing_media = (m_pParty->GetTipPartyType() == eTipSlaveAux) || (m_pParty->GetTipPartyType() == eTipNone);
	TRACEINTO << "is_EP_playing_media: " << (int)is_EP_playing_media;
	// stand alone or slave EPs - attempt to play next media
	if (!m_pParty->GetIsTipCall() || is_EP_playing_media) // tip slave- forward music complete
	{
		TRACEINTO << "Attempting to play next media";
		// try to play next media
		playing_next = PlayNextMedia(dialogId);
		if (!playing_next)	// end feature
		{
			TRACEINTO << "no new media found";
			//update CAM (list) - delete feature
			m_pConfApi->SendCAMRemoveFeatureFromList(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_REMOVE_FEATURE, 1);
			// if TIP slave- notify master
			if (m_pParty->GetIsTipCall() && (m_pParty->GetTipPartyType() == eTipSlaveAux)) // tip slave- forward music complete
			{	//forward complete ack to master
				CSegment pSeg;
				std::string status = "completed";
				pSeg << dialogId << status;
				m_pConfApi->PartyToPartyCntlMsgFromSlaveToMaster(m_rsrcPartyId, m_pParty->GetTipPartyType(), EXTERNAL_IVR_PLAY_MEDIA_COMPLETE_SLAVE_TO_MASTER, &pSeg);
			}
			CSegment seg;
			seg << STATUS_OK;
			OnEndFeature(&seg);
		}
	}
	if (!playing_next)
	{
		TRACEINTO << "completing media play";
		std::string status = "completed";
		OnMediaCompleted(dialogId, status);
	}
}

void CIvrCntlExternal::HandlePartyDtmfForExternalIvr(CSegment* pParam)
{
	DWORD dtmfOpcode;
	BYTE dtmf[MAX_DTMF_LEN_FROM_SRC];
	CSegment pParamDtmf;
	*pParam >> dtmfOpcode;
	TRACEINTO << "- Opcode:"  << dtmfOpcode ;
	switch (dtmfOpcode)
	{
	case AUD_DTMF_IND_VAL:
	{
		pParam->Get(dtmf, 4);                                                                         // the Audio DSP always send 4 bytes
		char tran[12] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#' };
		char charDTMF = (BYTE)-1;


		/* VNGR-23481: check if the DTMF code is invalid */
		if (dtmf[0] > 11)
		{
			TRACEINTO << "CFeatureObject::DtmfReceived - Failed, Invalid DTMF code";
			return;
		}
		else
			charDTMF =tran[dtmf[0]];

		pParamDtmf << (DWORD)1;                                                                        // Length - for now only one DTMF at a time from the Audio DSP
		pParamDtmf << (BYTE&)charDTMF;                                                                 // Fill DTMF params
		break;
	}

	case SIGNALLING_DTMF_INPUT_IND:                                                                   // from SIGNALLING (IP or SIP), can be number of DTMF
	case RTP_DTMF_INPUT_IND:                                                                          // from RTP (IP or SIP), can be number of DTMF
	{
		DWORD dtmfLength;
		*pParam >> dtmfLength;
		if ((dtmfLength == 0) || (dtmfLength > MAX_DTMF_LEN_FROM_SRC))
		{
			TRACEINTO << "CFeatureObject::DtmfReceived - Failed, H245 DTMF illegal length (0 or too long)";
			return;
		}

		pParam->Get((BYTE*)dtmf, dtmfLength);
		pParamDtmf << (DWORD)dtmfLength;                                                               // Length
		pParamDtmf.Put((BYTE*)dtmf, dtmfLength);
		break;
	}

	default:
		TRACEINTO << " Failed, Illegal opcode, Opcode:" << dtmfOpcode;
		return;
	}
	HandleEvent(&pParamDtmf, pParamDtmf.GetLen(), AUD_DTMF_IND_VAL);// handle
}

//--------------------------------------------------------------------------
void CIvrCntlExternal::HandleMediaConnectionForExternalIvr(CSegment* pParam)
{
	DWORD audioOrVideo;
	*pParam >> audioOrVideo;

	PASSERTSTREAM_AND_RETURN(!IsExternalIVR(), " CIvrCntlExternal::HandleMediaConnectionForExternalIvr called for internal IVR.")

	COstrStream msg;
	msg << "connected media is ";
	if (audioOrVideo == CAM_VIDEO)
	{
		msg << "VIDEO";
		m_isPartyVideoConnected = TRUE;
	}
	else if (audioOrVideo == CAM_AUDIO)
	{
		msg << "AUDIO";
		m_isPartyAudioConnected = TRUE;
	}
	else
		PASSERTSTREAM_AND_RETURN(true, "unexpected media type-" << audioOrVideo << " for party id " << m_rsrcPartyId);


	TRACEINTO << msg.str();
}
//--------------------------------------------------------------------------
void CIvrCntlExternal::HandleMediaDisconnectionForExternalIvr(CSegment* pParam)
{
	DWORD audioOrVideo;
	*pParam >> audioOrVideo;

	PASSERTSTREAM_AND_RETURN(!IsExternalIVR(), " CIvrCntlExternal::HandleMediaDisconnectionForExternalIvr called for internal IVR.")

	COstrStream msg;
	msg << "disconnected media is ";
	if (audioOrVideo == CAM_VIDEO)
	{
		msg << "VIDEO";
		m_isPartyVideoConnected = FALSE;
	}
	else if (audioOrVideo == CAM_AUDIO)
	{
		msg << "AUDIO";
		m_isPartyAudioConnected = FALSE;
	}
	else
		PASSERTSTREAM_AND_RETURN(true, "unexpected media type-" << audioOrVideo << " for party id " << m_rsrcPartyId);

	msg << ", IVRCntl state is " << m_state;
	if (m_state != ACTIVE)
	{
		msg << ", returning since IVR hasn't started yet.";
		return;
	}

	// locating current external IVR dialog
	TASKS_BY_DIALOG::iterator itr_end =  m_DialogContextByID.end();
	TASKS_BY_DIALOG::iterator dit = itr_end;
	if (m_pIvrSubGenSM != NULL)
	{
		msg << " current active external IVR feature has dialog id \"" << m_pIvrSubGenSM->GetDialogID() << "\". ";
		dit = m_DialogContextByID.find(m_pIvrSubGenSM->GetDialogID());
	}
	if (dit == itr_end)
	{
		msg << " no current active external IVR feature was found. trying with first mapped dialog id. ";
		dit = m_DialogContextByID.begin();
	}

	// found current IVR dialog- end it.
	if (dit != itr_end)
	{
		std::string dialogId = dit->first;
		msg << "ending dialog id \"" << dialogId << "\" with status \"stopped\"";
		TRACEINTO << msg.str();

		// "stopped" status forces completion of the 'dialogstart' ext. IVR message. the reponse returned to the DMA will also include the "stopped" status.
		std::string status = "stopped";
		OnMediaCompleted(dialogId, status);
	}
	// not handling external IVR dialog currently. return.
	else
	{
		msg << " no active IVR dialog was found.";
		TRACEINTO << msg.str();
	}



}

bool CIvrCntlExternal::IsPartyMediaConnected() const
{
	TRACEINTO << "audio connected: " << m_isPartyAudioConnected << ", video connected: " << m_isPartyVideoConnected;
	// for audio-only and for TIP slave_aux participants- only audio is connected.
	if ((m_isAudioOnly != FALSE) || (m_pParty != NULL && m_pParty->GetTipPartyType() == eTipSlaveAux))
		return m_isPartyAudioConnected;
	else
		return (m_isPartyAudioConnected && m_isPartyVideoConnected);
}

//////////////////////////////////////////////////////////////// end Ext. IVR phase II - dialog handled by IVR additions


//--------------------------------------------------------------------------
void CIvrCntlExternal::OnPlayMusic(CSegment* pParam)
{
	TRACEINTO;
	OnPlayFile(pParam, mft_Audio);
}

//--------------------------------------------------------------------------
void CIvrCntlExternal::OnShowSlide(CSegment* pParam)
{
	TRACEINTO;
	OnPlayFile(pParam, mft_Image);
}


//--------------------------------------------------------------------------
void CIvrCntlExternal::OnPlayFile(CSegment* pParam, MediaFileTypeEnum ivrFileType)
{
	DialogState state;
	MediaElementType* media;
	*pParam >> state >> (void*&)media;
	PASSERT_AND_RETURN(!media);

	TRACEINTO << "dialogID:" << state.dialogID << std::hex << ", media:" << media << ", URL:" << media->m_loc;

	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	DialogStart* dialogStart = (DialogStart*)(mscIvr.m_pResponseType);
	DialogElementType& dialog = dialogStart->m_dialog;
	//CollectElementType collect_node = dialogStart->m_dialog.m_collect;

	TRACEINTO << "is tip call: " << m_pParty->GetIsTipCall() << "tip party type: " << m_pParty->GetTipPartyType();
	if (!m_pParty->GetIsTipCall() ||
		(m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter)) //BRIDGE-3510
	{
		TASKS_BY_DIALOG::iterator it = m_DialogContextByID.find(state.dialogID);
		if (it == m_DialogContextByID.end())
		{
			CDialogContext *new_dialog = new CDialogContext(state);
			it = m_DialogContextByID.insert(std::make_pair<std::string, CDialogContext*>(state.dialogID, new_dialog)).first;
		}
	}

#if 0 // TODO: Guy - fix the mechanism of fetching URL by Dialog ID and media type
	const CLocalFileDescriptor* file = GetRequestedExternalFile(state.dialogID, ivrFileType);
#else
	const CLocalFileDescriptor* file = CFilesCache::const_instance().fileDescriptor(media->m_loc);
#endif

	PASSERT_AND_RETURN(!file);

	std::string path;
	GetBaseFolderByMediaType(ivrFileType, media->m_loc, path);
	path += '/';
	path += file->path();

	// update CAM (list) for new feature
	// TRACEINTO << " - SendCAMGeneralActionCommand - start feature";
	//pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE, 1);

//	if(m_pIvrSubGenSM != NULL)
//	{
//		PTRACE(eLevelError, "Failed, Can't start feature while other exists");
//		// TODO Guy - send error to mccf if concurrent ext. IVR are not supported.
//	}

#if 0
	DWORD duration = file->duration(); // currently, it defaults to static_cast<size_t>(-1), meaning *infinity*, it is unsupported by Embedded
#else
	DWORD duration = 5; // some hard-coded value
	TRACEINTO << "PATCH: using a hard-coded duration:" << duration;
#endif

	bool isAudioOnly = false; //BRIDGE-4374

	switch (ivrFileType)
	{
	case mft_Audio:
		m_pConfApi->SendCAMAddFeatureToList(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL, 1);
		m_pIvrSubGenSM = new CIvrSubAudioExternal(path.c_str(), duration, state.dialogID);

		if(m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter) // BRIDGE-3510
		{
			return; //don't send IVR because slave_aux will do this.
		}

		break;

	case mft_Image:
	case mft_Video:
	{
		isAudioOnly = IsPartyAudioOnly();
		TRACEINTO << "isAudioOnly: " << isAudioOnly;

		if(!isAudioOnly)
		{
			CIVRSlidesList* pSlidesList = ::GetpSlidesList();

			if (!pSlidesList)
			{
				TRACEINTO << "create slide list";
				pSlidesList = new CIVRSlidesList;
				::SetpSlidesList(pSlidesList);
			}

			#if 0 // TODO: use external IVR slide file, when will be supported
					const char* fileName = file->path().c_str();
					pSlidesList->AddSlide(fileName, true);
			#else
					std::string slides_env;
					CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey(CFG_KEY_SLIDES_ENV, slides_env);

					const bool isGeneric = slides_env == "GENERIC";
					const char* fileName = isGeneric ? "General_Polycom_Slide" : "ATT_Waiting_Room_Slide";
					TRACEINTO << "PATCH: using a hard-coded IVR slide '" << fileName << "' instead of '" << file->path() << "'";
					pSlidesList->AddSlide(fileName, !isGeneric);
			#endif

			m_pIvrSubGenSM = new CIvrSubVideoExternal(fileName, duration, state.dialogID);
		}
		break;
	}

	default:
		break;
	}

	CMccfIvrPackageResponse::ResponseReportMsg(state, mccf_ivr_OK);

	if(!isAudioOnly) //BRIDGE-4374
	{
		m_state = ACTIVE;

		// set feature params
		StartNewFeature();
		OnStartIVR();
	}

	else
	{
		OnShowSlideResult(state.dialogID, "stopped"); //BRIDGE-4374
	}

}

//--------------------------------------------------------------------------
void CIvrCntlExternal::OnCollectDigits(CSegment* pParam)
{
	TRACEINTO ;

	DialogState state;
	CollectElementType* collect; //TODO check which param we get here
	*pParam >> state >> (void*&)collect;
	PASSERT_AND_RETURN(!collect);

	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	DialogStart* dialog = (DialogStart*)(mscIvr.m_pResponseType);
	PASSERTMSG_AND_RETURN(!dialog, "CIvrCntlExternal::OnCollectDigits - got NULL dialog object");

	//const CollectElementType& collect_node = dialog->m_dialog.m_collect;
	PASSERTMSG_AND_RETURN(!collect->IsAssigned(), "CIvrCntlExternal::OnCollectDigits - 'collect' not initialized!");

	TASKS_BY_DIALOG::iterator it = m_DialogContextByID.find(state.dialogID);
	if (it == m_DialogContextByID.end())
	{
		TRACEINTO << " new dialog found for collect digits";
		CDialogContext *new_dialog = new CDialogContext(state);
		it = m_DialogContextByID.insert(std::make_pair<std::string, CDialogContext*>(state.dialogID, new_dialog)).first;
	}

	//	if(m_pIvrSubGenSM != NULL)
	//	{
	//		PTRACE(eLevelError, "Failed, Can't start feature while other exists");
	//		// TODO Guy - send error to mccf if concurrent ext. IVR are not supported.
	//	}

	bool isClearDigitBuffer = false;
	// buffer is cleared at the beginning of the dialog, before this.
//	if (collect->m_clearDigitBuffer.IsAssigned())
//		isClearDigitBuffer = collect->m_clearDigitBuffer;

	WORD dtmf_max_digits = 0;
//	if (collect->m_maxDigits.IsAssigned())
//		dtmf_max_digits = collect->m_maxDigits.m_uint;
	size_t time_value_ms = 0;
//	DWORD dtmf_timeout = 0;
//	if (collect_node.m_timeOut.IsAssigned())
//	{
//		time_value_ms = decodeTimeDesignation(collect_node.m_timeOut.IsAssigned);
//		if (time_value_ms > 0)
//			dtmf_timeout = (DWORD)(time_value_ms/1000);
//	}
		//	duration = 0;
	DWORD dtmf_interdigit_timeout = 30; // in seconds
	time_value_ms = 0;
	if (collect->m_interDigitTimeOut.IsAssigned())
	{
		time_value_ms = decodeTimeDesignation(collect->m_interDigitTimeOut);
		if (time_value_ms > 0)
			dtmf_interdigit_timeout = (DWORD)(time_value_ms/1000);
	}
	std::string dtmf_termchar = "#";
	if (collect->m_termChar.IsAssigned())
		dtmf_termchar = collect->m_termChar.value();
//
//	std::string dtmf_escapechar = "";
//	if (collect_node.m_escapeKey.IsAssigned())
//		dtmf_escapechar = collect_node.m_escapeKey.value();


	CSmallString msg;
	msg << "CIvrCntlExternal::OnCollectDigits - " <<
			"dtmf_termchar: " << dtmf_termchar << ", dtmf_interdigit_timeout: " << dtmf_interdigit_timeout;
	PTRACE(eLevelInfoNormal, msg.GetString());

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);


	MccfIvrErrorCodesEnum status = mccf_ivr_OK;


//	//return response message to AS (via MCCF)
//	CMccfIvrPackageResponse::ResponseReportMsg((MscIvr*)mscIvr->NewCopy(), status, pMccfMsg, dialog_id_str);
//
	TRACEINTO << " before new CIvrSubExternalInputCollect";
	m_pIvrSubGenSM = (CIvrSubBaseSM*) new CIvrSubExternalInputCollect(state.dialogID, dtmf_termchar.c_str()[0], dtmf_interdigit_timeout, dtmf_max_digits, isClearDigitBuffer/*,dtmf_escapechar*/);

	m_state = ACTIVE;

	TRACEINTO << " before StartNewFeature, dialog id:" << state.dialogID;
	// set feature params
	StartNewFeature();
	OnStartIVR();
}


void CIvrCntlExternal::OnDialogTermExternal(CSegment *pParam)
{
	std::string dialogIdStr;
	*pParam >> dialogIdStr;

	TRACEINTO << "party " << m_pParty->GetName() << " received termtimer TO for dialog \"" << dialogIdStr << "\"";
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogIdStr);

	PASSERTSTREAM_AND_RETURN(dit == m_DialogContextByID.end(), "CIvrCntlExternal::OnDialogTermExternal missing dialog context for dialog id \""	 << dialogIdStr << "\"" );
	PASSERTSTREAM_AND_RETURN(!m_pParty->GetIsTipCall() || (m_pParty->GetTipPartyType() != eTipSlaveAux), "party " << m_pParty->GetName() << " received external dialog termination but is not a TIP slave party");
	if (IsValidPObjectPtr(m_pIvrSubGenSM))
	{
		PASSERTSTREAM(dialogIdStr != m_pIvrSubGenSM->GetDialogID(), "party " << m_pParty->GetName() << " received dialog termination for unexpected dialog id \"" << dialogIdStr
																	<< "\". current feature's dialog id is \"" << m_pIvrSubGenSM->GetDialogID() << "\"" );
		TRACEINTO << "stopping message playing";
		m_pIvrSubGenSM->StopPlayMessage();
		m_pIvrSubGenSM->EndFeature(STATUS_OK);
	}
	else
	{
		TRACEINTO << "no feature object found";
	}
	m_DialogContextByID.erase(dit);

}

void CIvrCntlExternal::OnDialogTermTimer(CSegment *pParam)
{
	std::string dialog_id = "";
	*pParam >> dialog_id;

	TRACEINTO << "received a term timeout for dialog \"" << dialog_id << "\". ending collection and sending response.";
	std::string current_dtmf_buffer = "";


	if (m_pParty->GetIsTipCall())
	{
		if (m_pParty->GetTipPartyType() == eTipMasterCenter)
		{
			TRACEINTO << "Sending EXTERNAL_IVR_DIALOG_TIMEOUT_MASTER_TO_SLAVE";
			CSegment pSeg;
			pSeg << dialog_id;
			m_pConfApi->PartytoPartyCntlMsgFromMasterToSlave(m_rsrcPartyId, (WORD)eTipSlaveAux, EXTERNAL_IVR_DIALOG_TIMEOUT_MASTER_TO_SLAVE, &pSeg);
		}
		else
			PASSERTSTREAM_AND_RETURN(TRUE, "TIP slave party \"" << m_pParty->GetName() << "\" ID: " << m_pParty->GetPartyRsrcID() << " TIP party type: " << (int)m_pParty->GetTipPartyType() << "received 'DIALOG_TERM_TIMER' - only TIP master or stand-alone EPs handle this timer!");
	}
	OnPlayFileResult(mft_Unknown, dialog_id, "stopped");
	if (m_isCollectDialog)
	{

		if (IsValidPObjectPtr(m_pDtmfCollector))
		{
			TRACEINTO << "retrieving current dtmf buffer";
			current_dtmf_buffer = m_pDtmfCollector->GetDtmfBuffer();
			if (IsValidPObjectPtr(m_pIvrSubGenSM))
				m_pIvrSubGenSM->EndFeature(DTMF_TIMEOUT_EXTERNAL_IVR_TIMER);
		}
		else
			TRACEINTO << "no dtmf collector found in IVR control!";
		OnCollectDigitsResult(dialog_id, current_dtmf_buffer, "stopped");
	}
	else
	{
		PASSERTSTREAM(TRUE, "party " << m_pParty->GetName() << " received termtimer TO without a collect elemt in the dialog ");
		CheckIsDialogCompletedAndRespond(dialog_id, FALSE);
	}
}

//--------------------------------------------------------------------------
void CIvrCntlExternal::OnCollectDigitsResult(std::string dialogIdStr, std::string receivedDtmfStr, std::string statusStr)
{
	TRACEINTO << "received DTMF for dialog " << dialogIdStr << " : \"" << receivedDtmfStr << "\", status " << "\"" << statusStr << "\"";
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogIdStr);

	PASSERTSTREAM_AND_RETURN(dit == m_DialogContextByID.end(), "CIvrCntlExternal::OnCollecDigitstResult missing dialog context for dialog id \""
																 << dialogIdStr << "\" to send collected DTMF entry: " << receivedDtmfStr);
	CDialogContext* dialog_context = dit->second;
	Event* responseEvent = dialog_context->GetRespnseEvent();
	responseEvent->m_dialogExit.m_collectInfo.m_dtmf = receivedDtmfStr;
	responseEvent->m_dialogExit.m_collectInfo.m_termMode = statusStr;
	if (statusStr == "stopped")
		responseEvent->m_dialogExit.m_status = eDialogStatusTimedOut;
	if (statusStr == "error")
	{
		statusStr = "stopped";
		responseEvent->m_dialogExit.m_status = eDialogStatusExecutionError;
	}
	CheckIsDialogCompletedAndRespond(dialogIdStr, TRUE);
}

//--------------------------------------------------------------------------
void CIvrCntlExternal::OnPlayMusicResult(std::string dialogIdStr, std::string statusStr)
{
	TRACEINTO;
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogIdStr);

	PASSERTSTREAM_AND_RETURN(dit == m_DialogContextByID.end(), "CIvrCntlExternal::OnPlayMusicResult missing dialog context for dialog");

	CDialogContext* dialog_context = dit->second;
	Event* responseEvent = dialog_context->GetRespnseEvent();
	responseEvent->m_dialogExit.m_promptInfo.m_termMode = statusStr;
	CheckIsDialogCompletedAndRespond(dialogIdStr);
}

//--------------------------------------------------------------------------
// deprecated.  in multiple media IVR mode we use "OnMediaCompleted"
void CIvrCntlExternal::OnShowSlideResult(std::string dialogIdStr, std::string statusStr)
{
	TRACEINTO;
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogIdStr);

	PASSERTSTREAM_AND_RETURN(dit == m_DialogContextByID.end(), "CIvrCntlExternal::OnShowSlideResult missing dialog context for dialog");

	CDialogContext* dialog_context = dit->second;
	Event* responseEvent = dialog_context->GetRespnseEvent();
	responseEvent->m_dialogExit.m_promptInfo.m_termMode = statusStr;
	//DialogStart* dialog_start = reinterpret_cast<DialogStart*>(dialog_context->getMscIvr()->m_pResponseType);
	CheckIsDialogCompletedAndRespond(dialogIdStr);
}

//--------------------------------------------------------------------------
void CIvrCntlExternal::CheckIsDialogCompletedAndRespond(std::string dialogId, BOOL forceCompleteAfterCollect)
{
	TRACEINTO;
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.find(dialogId); //TODO Guy - replace with: find(dialogId);

	PASSERTSTREAM_AND_RETURN(dit == m_DialogContextByID.end(), "CIvrCntlExternal::CheckIsDialogCompletedAndRespond missing dialog context");

	CDialogContext* dialog_context = dit->second;
	/***for testing without waiting for slide***/
	//Event* responseEvent = dialog_context->GetRespnseEvent();
	/****/
	TRACEINTO << "before CheckIsDialogCompletedAndRespond ";
	BOOL is_complete = dialog_context->CheckIsDialogCompletedAndRespond(forceCompleteAfterCollect);

	// TODO guy - check if delete needed or is carried out in mccf mngr
	if (is_complete)
	{
		TRACEINTO << " dialog complete, erasing context object and term timer.";
		DeleteTimer(DIALOG_TERM_TIMER);
		m_DialogContextByID.erase(dit);
	}
}

//--------------------------------------------------------------------------
BOOL CDialogContext::CheckIsDialogCompletedAndRespond(BOOL forceCompleteAfterCollect)
{
	TRACEINTO;

	Event* responseEvent = GetRespnseEvent();
	BOOL all_completed = TRUE;
	if (m_isCollectRequested)
	{
		TRACEINTO << " checking collect task status";
		all_completed &= (responseEvent->m_dialogExit.m_collectInfo.m_termMode.IsAssigned())? TRUE : FALSE;
	}
	if(!all_completed)
	{
		TRACEINTO << " collect task is not yet complete";
		return FALSE;
	}
	if ((forceCompleteAfterCollect != TRUE))
	{
		if (m_isPlayRequested || m_isSlideRequested)
		{
			WORD should_play = GetMediaListSize();
			WORD already_played = GetPlayedMediaCounter();
			TRACEINTO << " checking play slide/music task status - media files in dialog list: " << should_play << ", media files played: " << already_played;;
				all_completed &= (responseEvent->m_dialogExit.m_promptInfo.m_termMode.IsAssigned())? TRUE : FALSE;
		}
	}
	if (all_completed)
	{
		TRACEINTO << "before response send size: " << m_pResponseIvr->CurrentBinarySize();
		CMccfIvrPackageResponse::ResponseControlMsg(m_pResponseDialogState);
		TRACEINTO << "after response ";

	}
	else
		TRACEINTO << " tasks are not yet complete";
	return all_completed;
}

//--------------------------------------------------------------------------
const CLocalFileDescriptor* CIvrCntlExternal::GetRequestedExternalFile(const std::string& dialogId, MediaFileTypeEnum fileMediaType)
{
	TRACEINTO << "searching file for dialog " << dialogId << " of type " << fileMediaType;

	// TODO Guy- currently handling only one dialog. replace with implementation of concurrect dialogs.
	TASKS_BY_DIALOG::iterator dit = m_DialogContextByID.begin();
	PASSERTMSG_AND_RETURN_VALUE(dit == m_DialogContextByID.end(), "dialog not found", NULL);

	CDialogContext* dialog_context = dit->second;
	MediaElementType* media = dialog_context->GetMediaElement(fileMediaType);
	PASSERTSTREAM_AND_RETURN_VALUE(!media, "No media found for type " << fileMediaType, NULL);

	const CLocalFileDescriptor* file = CFilesCache::const_instance().fileDescriptor(media->m_loc);
	PASSERTMSG(!file, "file not found");

	return file;
}

//--------------------------------------------------------------------------
void CIvrCntlExternal::RecivedPlayMessageAck()
{
	TRACEINTO;
	RecivedPlayMessageAckMultipleMedia();

//	//update CAM (list) - delete feature
//	m_pConfApi->SendCAMRemoveFeatureFromList(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_REMOVE_FEATURE, 1);

//	if(m_pIvrSubGenSM != NULL)
//		OnPlayMusicResult(m_pIvrSubGenSM->GetDialogID(), "completed");

}

//--------------------------------------------------------------------------
void CIvrCntlExternal::RecivedShowSlideAck()
{
	TRACEINTO;

//	if(m_pIvrSubGenSM != NULL)
//		OnShowSlideResult(m_pIvrSubGenSM->GetDialogID(), "completed");
}

////--------------------------------------------------------------------------
//CIvrSubBaseSM* CIvrCntlExternal::StartNextFeature()
//{
//  while (TRUE)
//  {
//    if (m_stage >= NUM_OF_FEATURES)
//    {
//	   m_stage = 0;
//	   return NULL;
//	}
//
//	if (m_featuresList[m_stage].bToDo)
//	{
//	    TRACEINTO << IvrStageToString(m_stage) << "(" << m_stage << ")";
//	    PASSERT(555777);//to update IvrStageToString
//
//	    switch (m_stage)
//	    {
//	       case IVR_STAGE_AUDIO_EXTERNAL: // welcome
//	          return (CIvrSubBaseSM*) new CIvrSubAudioExternal();
//
////	       case IVR_STAGE_COLLECT_EXTERNAL:   // No video resources
////	          return (CIvrSubBaseSM*) new CIvrSubNoVideoResources(IVR_FEATURE_GENERAL);
//
//	       default: // error
//	          return NULL;
//	     } // switch
//	 }
//	 // step to next feature
//	 m_stage++;
//  }
//}


//--------------------------------------------------------------------------
CDialogContext::CDialogContext()
{
	m_pResponseDialogState.baseObject = NULL;
	m_pMscIvr = NULL;
	m_pCollectElement = NULL;
	m_pCurrentMediaIterator = NULL;
	m_playedMediaCounter = 0;
}

//--------------------------------------------------------------------------
CDialogContext::CDialogContext(DialogState& state)
{
	m_pResponseDialogState = state;
	m_isSlideRequested = FALSE;
	m_isPlayRequested = FALSE;
	m_isCollectRequested = FALSE;
	m_isBargeInAllowed = FALSE;
	m_pCurrentMediaIterator = NULL;
	m_pMscIvr = (MscIvr*)(state.baseObject->NewCopy());
	m_pCollectElement = NULL;
	m_playedMediaCounter = 0;
	ReadMediaFromMscIvr(m_pMscIvr);
	ReadCollectFromMscIvr(m_pMscIvr);
	InitResponse(m_pResponseDialogState);
}

//--------------------------------------------------------------------------
void CDialogContext::ReadMediaFromMscIvr(MscIvr *mscIvr)
{
	TRACEINTO;
	if (mscIvr == NULL)
	{
		TRACEINTO << " mscIvr is NULL. no media elements";
		return;
	}
	DialogStart* dialog = (DialogStart*)(mscIvr->m_pResponseType);
	if (dialog->m_dialog.m_prompt.IsAssigned())
	{
		if (dialog->m_dialog.m_prompt.m_bargein.IsAssigned())
			m_isBargeInAllowed = dialog->m_dialog.m_prompt.m_bargein;
		TRACEINTO << " Prompt element \"bargein\" : " << (int)m_isBargeInAllowed;
	std::list <MediaElementType>::iterator mit = dialog->m_dialog.m_prompt.m_media.begin();
	TRACEINTO << "reading " << dialog->m_dialog.m_prompt.m_media.size() << " media elements from message";
	MediaFileTypeEnum type = mft_Unknown;
	for (;mit != dialog->m_dialog.m_prompt.m_media.end(); mit++)
	{
		type = CMediaTypeManager::DeriveMediaType((*mit).m_type, (*mit).m_loc);
		switch (type)
		{
		case mft_Image:
			m_isSlideRequested = TRUE;
			break;
		case mft_Audio:
			m_isPlayRequested= TRUE;
			break;
		case mft_Video:
			m_isSlideRequested = TRUE;	// TODO Guy - support video
			break;
		default:
			PASSERTSTREAM(TRUE, "Found unrecognized media type: " << type);
			}
		InsertMediaElement(type, (MediaElementType*)(*mit).NewCopy());

	}
		TRACEINTO << "inserted a total of " << m_pMediaList.size() << " media items";
	if (dialog->m_dialog.m_collect.IsAssigned())
		m_isCollectRequested = TRUE;
	}
	else
	{
		m_pCurrentMediaIterator = NULL;
		TRACEINTO << " no prompt elements.";
	}
}

void CDialogContext::ReadCollectFromMscIvr(MscIvr *mscIvr)
{
	TRACEINTO;
	if (mscIvr == NULL)
	{
		TRACEINTO << " mscIvr is NULL.";
		return;
	}
	DialogStart* dialog = (DialogStart*)(mscIvr->m_pResponseType);
	if (dialog->m_dialog.m_collect.IsAssigned())
		m_pCollectElement = dialog->m_dialog.m_collect.NewCopy();
	else
		TRACEINTO << " no collect elements";
}
//--------------------------------------------------------------------------
void CDialogContext::InitResponse(DialogState& state)
{
	m_pResponseIvr = m_pMscIvr->NewCopy();
	m_pResponseDialogState.baseObject = m_pResponseIvr;
	CMccfIvrPackageResponse::BuildControlMsg(m_pResponseIvr, m_pResponseDialogState.dialogID, (DWORD)eDialogStatusCompleted);
	Event* responseEvent = (Event*)(m_pResponseIvr->m_pResponseType);
	responseEvent->Clear();
	responseEvent->m_dialogId = state.dialogID;
	responseEvent->m_dialogExit.m_status = (DWORD)eDialogStatusCompleted;
}

//--------------------------------------------------------------------------
CDialogContext::~CDialogContext()
{
	if (m_pMscIvr)
	{
		TRACEINTO << "deleting m_pMscIvr " << (void*)m_pMscIvr << ", m_pMscIvr->m_pResponseType " << (void*)m_pMscIvr->m_pResponseType;
		POBJDELETE(m_pMscIvr);
	}

	if (m_pResponseIvr)
	{
		TRACEINTO << "deleting m_pResponseIvr " << (void*)m_pResponseIvr << ", m_pResponseIvr->m_pResponseType " << (void*)m_pResponseIvr->m_pResponseType;
		POBJDELETE(m_pResponseIvr);
	}

	for (DIALOG_MEDIA_LIST::iterator mit = m_pMediaList.begin(); mit != m_pMediaList.end(); mit++)
		POBJDELETE(*mit);

	if (m_pCurrentMediaIterator)
		delete m_pCurrentMediaIterator;

	POBJDELETE(m_pCollectElement);
}

//--------------------------------------------------------------------------
WORD CDialogContext::GetMediaListSize()
{
	return (WORD)m_pMediaList.size();
}

//--------------------------------------------------------------------------
WORD CDialogContext::GetPlayedMediaCounter()
{
	return m_playedMediaCounter;
}
//--------------------------------------------------------------------------
MediaElementType *CDialogContext::GetMediaElement(MediaFileTypeEnum mediaFileType)
{
	MediaElementType *media_element = GetNextMediaElement();
	while (media_element != NULL)
	{
		if (mediaFileType != CMediaTypeManager::DeriveMediaType(media_element->m_type, media_element->m_loc))
		{
			AdvanceToNextMediaElement();
			media_element = GetNextMediaElement();
		}
		else
			break;
	}

	return media_element;

//	DIALOG_MEDIA_MAP::iterator mit = media_map.find(mediaFileType);
//	if (mit != media_map.end())
//	{
//		return mit->second;
//	}
//	else
//		return NULL;
}

MediaElementType* CDialogContext::GetNextMediaElement()
{
	MediaElementType *media_element = NULL;
	if (m_pCurrentMediaIterator == NULL)
	{
		if (m_pMediaList.size() == 0)
		{
			TRACEINTO << " media list is empty";
			//*m_pCurrentMediaIterator == m_pMediaList.end();
		}
		else
		{
			TRACEINTO << "first request. returning first media file out of " << m_pMediaList.size();
			media_element = *m_pMediaList.begin();
		}
	}
	else if (*m_pCurrentMediaIterator == m_pMediaList.end())
	{
		TRACEINTO << "reached the end of the dialog's media list.";
	}
	else
	{
		DIALOG_MEDIA_LIST::iterator tmp = *m_pCurrentMediaIterator;
		if (++tmp == m_pMediaList.end())
		{
			TRACEINTO << " current media element is the last.";
		}
		else
			media_element =  (MediaElementType*)(*tmp);
	}
	return media_element;
}

void CDialogContext::AdvanceToNextMediaElement()
{
	TRACEINTO;
	if (m_pCurrentMediaIterator != NULL)
	(*m_pCurrentMediaIterator)++;
	else
	{
		m_pCurrentMediaIterator = new DIALOG_MEDIA_LIST::iterator();
		*m_pCurrentMediaIterator = m_pMediaList.begin();
	}
	m_playedMediaCounter++;
}

MediaElementType* CDialogContext::GetCurrentMediaElement()
{
	if ((m_pMediaList.size() == 0) || (m_pCurrentMediaIterator == NULL))
		return NULL;
	else
		return (MediaElementType*)(**m_pCurrentMediaIterator);
}



//--------------------------------------------------------------------------
void CDialogContext::InsertMediaElement(MediaFileTypeEnum mediaFileType, MediaElementType *mediaElement)
{
	TRACEINTO <<  "inserting " << mediaElement->m_loc ;
	m_pMediaList.push_back(mediaElement);
}

//--------------------------------------------------------------------------
