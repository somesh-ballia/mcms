
// IVRCntlExternal.h
// Created on: Oct 31, 2012
// Author: yoella

//+========================================================================+
//                            IVRCntlExternal.h                            |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IVRCNTLExternal.h                                           |
// SUBSYSTEM:  MCMS                                                        |
//                                                                         |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 25.09.2012 |                                                      |
//+========================================================================+

#ifndef IVRCNTLEXTERNAL_H_
#define IVRCNTLEXTERNAL_H_

#include "IVRCntl.h"

#include "Event.h"

#include "MediaElementType.h"
#include "MediaTypes.h"

#include <list>

////////////////////////////////////////////////////////////////////////////
class CParty;
class CLocalFileDescriptor;

////////////////////////////////////////////////////////////////////////////
enum DialogStatusEnum
{
	eDialogStatusTerminated = 0,
	eDialogStatusCompleted,
	eDialogStatusDisconnected,
	eDialogStatusTimedOut,
	eDialogStatusExecutionError,
};

////////////////////////////////////////////////////////////////////////////
typedef std::list<MediaElementType*> DIALOG_MEDIA_LIST;

////////////////////////////////////////////////////////////////////////////
class CDialogContext : public CPObject
{
//	friend bool         operator <(const CDialogContext& rhs, const CDialogContext& lhs)

	CLASS_TYPE_1(CDialogContext, CPObject)
	virtual const char* NameOf() const                         { return "CDialogContext";}

public:

	CDialogContext();
	CDialogContext(DialogState& state);
	~CDialogContext();
	const DialogState&  getDialogStateForResponse() { return m_pResponseDialogState;}
	MscIvr*             getMscIvr() { return m_pMscIvr;}
	void                ReadMediaFromMscIvr(MscIvr *mscIvr);
	void                ReadCollectFromMscIvr(MscIvr *mscIvr);
	void                InitResponse(DialogState& state);
	BOOL                GetIsFirstMedia() { return m_pCurrentMediaIterator == NULL; }
	MediaElementType*   GetNextMediaElement();
	void                AdvanceToNextMediaElement();
	MediaElementType*   GetCurrentMediaElement();
	CollectElementType* GetCollectElement() { return m_pCollectElement;}
	MediaElementType*   GetMediaElement(MediaFileTypeEnum mediaFileType)	;
	void                InsertMediaElement(MediaFileTypeEnum mediaFileType, MediaElementType *mediaElement);
	Event*              GetRespnseEvent() { return (Event*)m_pResponseIvr->m_pResponseType; }
	WORD                GetMediaListSize();
	WORD                GetPlayedMediaCounter();
	BOOL                CheckIsDialogCompleted(BOOL forceCompleteAfterCollect);
	BOOL                CheckIsDialogCompletedAndRespond(BOOL forceCompleteAfterCollect = FALSE);
	BOOL                GetIsBargeIn() { return m_isBargeInAllowed; }
	BOOL                GetIsCollectRequested() { return m_isCollectRequested; }

	BOOL                GetIsShowSlideRequested() { return m_isSlideRequested; }
	BOOL                GetIsPlayMediaRequested() { return m_isPlayRequested; }

	void                OnDialogTermTimer(CSegment *pParam);
//	BOOL                AreAllTasksCompleted();

private:

	DialogState m_pResponseDialogState;
	MscIvr* m_pMscIvr;
	MscIvr* m_pResponseIvr;
	BOOL m_isCollectRequested;
	BOOL m_isSlideRequested;
	BOOL m_isPlayRequested;
	BOOL m_isBargeInAllowed;
	WORD m_playedMediaCounter;

	DIALOG_MEDIA_LIST		m_pMediaList;
	DIALOG_MEDIA_LIST::iterator *m_pCurrentMediaIterator;
	CollectElementType*		m_pCollectElement;
	//	BYTE m_nTasksInDialog;
};

////////////////////////////////////////////////////////////////////////////
typedef std::map<std::string, CDialogContext*> TASKS_BY_DIALOG;

////////////////////////////////////////////////////////////////////////////
class CIvrCntlExternal : public CIvrCntl
{
	CLASS_TYPE_1(CIvrCntlExternal, CIvrCntl)

public:

	CIvrCntlExternal(CTaskApp* pOwnerTask,const DWORD dwMonitorConfId);

	~CIvrCntlExternal();
	virtual const char* NameOf() const            { return "CIvrCntlExternal";}
	virtual void*       GetMessageMap()           { return (void*)m_msgEntries; }
	virtual void        HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	void                SetFeaturesToDo(DWORD featureType);
	virtual void        RecivedPlayMessageAck();
	virtual void        RecivedPlayMessageAckMultipleMedia();
	virtual void        RecivedPlayMessageAckTimer(CSegment *pSeg);
	virtual void        RecivedShowSlideAck();
	virtual BOOL        IsExternalIVR() { return TRUE; }
	virtual void        HandlePartyDtmfForExternalIvr(CSegment* pParam);
	virtual void        HandleMediaConnectionForExternalIvr(CSegment* pParam);
	virtual void        HandleMediaDisconnectionForExternalIvr(CSegment* pParam);
	bool				IsPartyMediaConnected() const;

	void                Start(CSegment* pParam = NULL);
	void                StartFeature(BYTE feature);

	//action functions
	virtual void          OnEndFeature(CSegment* pParam);
	virtual void          OnPlayMusic(CSegment* pParam);            //AT&T
	virtual void          OnPlayFile(CSegment* pParam, MediaFileTypeEnum ivrFileType);            //AT&T
	virtual void          OnDialogStart(CSegment* pParam);            //AT&T
	BOOL                  OnPlayFile(MediaFileTypeEnum ivrFileType, std::string& dialogId, MediaElementType *mediaElement, BOOL isBargeInAllowed = TRUE);            //AT&T
	void                  OnCollectDtmfDigits(std::string dialogId, CollectElementType* collect);        //AT&T

	void                  OnPlayFileResult(MediaFileTypeEnum type,std::string dialogIdStr, std::string statusStr);   //AT&T
	virtual void          OnMediaCompletedExternal(CSegment* pParam);
	void                  OnMediaCompleted(std::string& dialogId, std::string& status);
	void                  OnShowSlide(CSegment* pParam);            //AT&T
	void                  OnCollectDigits(CSegment* pParam);        //AT&T

	void                  OnCollectDigitsResult(std::string dialogIdStr, std::string receivedDtmfStr, std::string statusStr);   //AT&T
	void                  OnShowSlideResult(std::string dialogIdStr, std::string statusStr);
	void                  OnPlayMusicResult(std::string dialogIdStr, std::string statusStr);
	//void                  OnPromptResult(std::string dialogIdStr, std::string statusStr);
	void                  OnMediaBargedInByDTMF(CSegment *pSeg);
	void                  OnMediaBargeInExternal(CSegment* pParam);
	void                  OnDialogTermTimer(CSegment *pParam);
	void                  OnDialogTermExternal(CSegment *pParam);
private:
	BOOL                  PlayNextMedia(std::string& dialogId);
	BOOL                  IsPartyAudioOnly() { return m_isAudioOnly; }
	void                  CheckIsDialogCompletedAndRespond(std::string dialogId, BOOL forceCompleteAfterCollect = FALSE);

	const CLocalFileDescriptor* GetRequestedExternalFile(const std::string& dialogId, MediaFileTypeEnum fileMediaType);

private:

	TASKS_BY_DIALOG m_DialogContextByID;
	DWORD taskCounter;
	BOOL m_isBargeInAllowed;
	BOOL m_isCollectDialog;
	BOOL m_isAudioOnly;
	BOOL m_isPartyAudioConnected;
	BOOL m_isPartyVideoConnected;
	TICKS m_dialogStartTime;
	std::string m_sDialogBargedInByTIPMaster;

	PDECLAR_MESSAGE_MAP
};

#endif /* IVRCNTLEXTERNAL_H_ */

