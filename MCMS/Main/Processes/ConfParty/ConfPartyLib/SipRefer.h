
//=================================================================================================
//
//Copyright (C) 2005 POLYCOM
//This file contains confidential information proprietary to ACCORD Networks Ltd. The use or 
//disclosure of any information contained in this file without the written consent of an officer of
//ACCORD Networks Ltd. is expressly forbidden.
//
//=================================================================================================

//=================================================================================================
//
//Module Name:  SIPREFER.H
//
//General Description:  
//
//    1.	control SIP conference package
//
//Generated By: Ori P.                            Date: 9.03.05
//
//Revisions and Updates: 
//
//Date         Updated By         Description
//========   ==============   =====================================================================
// 
//=================================================================================================

#ifndef __SIPREFER_
#define __SIPREFER_

//===== Include Files =====
#include "SIPConfPack.h"

class CSIPREFERSubscriber : public CSIPSubscriber
{
CLASS_TYPE_1(CSIPREFERSubscriber, CSIPSubscriber )
	// public functions
public:
	CSIPREFERSubscriber();
	CSIPREFERSubscriber(WORD boardId, const char* pFrom, const char* pFromTag, const char* pTo, const char* pToTag,
						const char* pCallId, DWORD CSeq, const char* pReferTo, BYTE bReferWithBye,
						DWORD ip, WORD port, WORD transport, WORD expires, WORD srcUnitId, eRoleType role,
						DWORD callIndex, const char* pMSConversationIDStr=NULL,DWORD cs_Id=1);
	virtual ~CSIPREFERSubscriber();
	virtual const char* NameOf() const { return "CSIPREFERSubscriber";}

	
	char*	GetReferTo();
	char*   GetClickToConfID();
	BYTE	IsReferWithBye();
	DWORD	GetCSeq();
//	DWORD	GetCallIndex();
	BYTE	RetryConnect();

protected:
	
	BYTE	m_bReferWithBye;
	char	*m_pReferTo, *m_pClickToConfIDStr;//*m_pMSConversationIDStr;
	DWORD	m_CSeq/*, m_CallIndex*/;
	BYTE	m_ConnectRetry;
};


class CSIPReferEventPackageManager: public CSIPEventPackageManager
{
CLASS_TYPE_1(CSIPReferEventPackageManager, CSIPEventPackageManager )
	// public functions
public:
	CSIPReferEventPackageManager(CSIPEventPackageDispatcher* pSIPDispatcher=NULL, COsQueue* pRcvMbx=NULL, CMplMcmsProtocol* pMplMcmsProtocol=NULL,
							CConfPartyManagerLocalApi* pConfPartyManagerApi=NULL, CConfApi* pConfApi=NULL);
	~CSIPReferEventPackageManager();
	virtual const char* NameOf() const { return "CSIPReferEventPackageManager";}
	
	// action functions
	virtual void	OnSubscribeTout(CSegment* pParam);
	void	OnConnectReferredPartyTout(CSegment* pParam);
	virtual void	OnNotifyTimerTout(CSegment* pParam);

	virtual void	ObserverUpdate(CSIPSubscriber* pSubscriber, WORD event, DWORD val);
	void			Refer(mcIndRefer* pReferInd, DWORD callIndex, WORD srcUnitId, eRoleType role=eParticipant, DWORD serviceId=0);
	int				ParseAddress(const char* strAddress,/*out*/char* strParsedAddress,/*out*/BYTE& bIsTelephone) const;
	virtual BYTE	Notify(CSIPSubscriber* pSubscriber, char* content, char* state, BYTE isDistribute = false);
	BYTE			CheckRequestValidity(mcIndRefer* pReferInd, enSipCodes *SipStatus, CConfParty **ReqParty, CConfParty **pDestParty);
	BYTE			CheckIfReferWithBye(CSipHeaderList *pHeadersList, char* pReferToStr, char* pFullReferToStr = NULL);
	CSIPREFERSubscriber*	FindParty(const char* from, DWORD CSeq, char* callId);
	BYTE			FindParty(void* pSubscriber);
	virtual void	Dump(COstrStream& msg) const;

protected:
	
	void 	ParseReferHeaders(CSipHeaderList *pTemp, char **pFromStr, char **pFromTagStr, char **pToStr, char **pToTagStr,
								char **pCallIdStr, char **pRefferedByStr, char **pMSAssociatedStr, char **pMSConversationIDStr);
	CConfParty* 	FindReferedPartyInDB(char* referTo, CSIPREFERSubscriber* pSubscriber) const;
	void	SendSipReferResponse(mcIndRefer* pReferInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId, DWORD serviceid);

	WORD	m_countConnectRefer;
	CSIPEventPackageDispatcher* m_pSIPDispatcher;

	PDECLAR_MESSAGE_MAP
};


#endif
