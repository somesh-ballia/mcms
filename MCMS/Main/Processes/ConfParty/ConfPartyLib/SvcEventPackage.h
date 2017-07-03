/*
 * SvcEventPackage.h
 *
 *  Created on: Aug 29, 2012
 *      Author: bguelfand
 */

#ifndef SVCEVENTPACKAGE_H_
#define SVCEVENTPACKAGE_H_


//===== Include Files =====
#include <vector>
#include <map>
#include <queue>
#include "ObjString.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SIPMsftCX.h"
#include "MplMcmsProtocol.h"
#include "ConfApi.h"
#include "ConfPartyManagerLocalApi.h"
#include "Conf.h"
#include "TaskApp.h"
#include "Conf.h"
#include "ObjString.h"
#include "SIPMsftCX.h"
#include "Media.h"

#include "SipEventPackageCommon.h"

#define MEDIA_STREAMS	4


// ************************************************************************************
//
//	CSvcBasicConfPackageType
//
// ************************************************************************************


class CSvcBasicConfPackageType : public CPObject
{
	CLASS_TYPE_1(CSvcBasicConfPackageType, CPObject )

// public functions
public:

	virtual const char* NameOf() const { return "CSvcBasicConfPackageType";}
	CSvcBasicConfPackageType();
	CSvcBasicConfPackageType(char* pName);
	virtual ~CSvcBasicConfPackageType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode) = 0;

	const char*		GetElementName();

protected:
	std::string GetTagWithNsPrefix(const char* pszTag);
	std::string GetTagWithNsPrefix(const char* pszNamespace, const char* pszTag);

	std::string	m_sNameSpacePrefix;
	char	m_elementName[H243_NAME_LEN];
};


// ************************************************************************************
//
//	CSvcConfPackageType
//
// ************************************************************************************


class CSvcConfPackageType : public CSvcBasicConfPackageType
{
	CLASS_TYPE_1(CSvcConfPackageType, CSvcBasicConfPackageType )
// public functions
public:
	CSvcConfPackageType();
	CSvcConfPackageType(char* pName);
	virtual const char* NameOf() const { return "CSvcConfPackageType";}
	virtual ~CSvcConfPackageType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode);
	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull) = 0;
	virtual void	SerializeState(CXMLDOMElement* pFatherNode, BYTE bFull);

	eTypeState GetState();
	const char* GetStateByString();

	bool	IsNew() { return m_bNew; }
	void	SetOld() { m_bNew = false; }

protected:

	eTypeState	m_state;
	bool		m_bNew;

private:
	CSvcConfPackageType(const CSvcConfPackageType& other);
};

class CSvcUserType;
class CSvcConfInfoType;

typedef std::map<long, CSvcUserType*> SvcUsersMap;

#define NUM_OF_ACTIONS  4
#define MAX_NOTIFY_CONTENT_LEN		14336

#define EMPTY_LIST		-1

class CSvcEventPackageManager;

// ************************************************************************************
//
//	CSvcSubscriber
//
// ************************************************************************************
class CSvcSubscriber : public CStateMachine
{
	CLASS_TYPE_1(CSvcSubscriber, CStateMachine )
	// public functions
public:
	CSvcSubscriber();
	CSvcSubscriber(CSvcEventPackageManager* pCSvcEventPackageManager, DWORD dwPartyRsrcID, DWORD dwPartyMonitorID, WORD boardId,
	               const char* pFrom, const char* pFromTag, const char* pTo, const char* pToTag, const char* pContact,     const char* pCallId,
	               mcXmlTransportAddress transportAddress, WORD expires, WORD srcUnitId, DWORD callIndex,
	               eRoleType role = eParticipant, DWORD cs_Id = 1);
	virtual ~CSvcSubscriber();
	virtual const char* NameOf() const {return "CSvcSubscriber"; }
	virtual void        HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	friend WORD operator ==(const CSvcSubscriber& first, const CSvcSubscriber& second);

	void                   Refresh(mcXmlTransportAddress transportAddress, const char* pCallId, WORD expires);
	void                   OnIndNotifyResp(DWORD status);
	WORD                   GetBoardId();
	char*                  GetFrom();
	char*                  GetFromTag();
	char*                  GetTo();
	char*                  GetToTag();
	char*                  GetContact();
	char*                  GetCallId();
	DWORD                  GetPartyRsrcID()                             { return m_dwPartyRsrcID; }
	DWORD                  GetPartyMonitorID()                          { return m_dwPartyMonitorID; }
	void                   SetPartyMonitorID(DWORD dwNewPartyMonitorID) { m_dwPartyMonitorID = dwNewPartyMonitorID; }
	mcXmlTransportAddress* GetTransportAddress()                        { return &m_transportAddress; }
	DWORD                  GetIp();
	DWORD                  GetCsId();
	WORD                   GetPort();
	WORD                   GetTransport();
	eRoleType              GetRole();
	WORD                   GetSrcUnitId();
	DWORD                  GetCallIndex();
	CStructTm              GetExpireTime();
	void                   SetExpireTime(CStructTm* newTime);
	int                    GetNotificationVersion();
	void                   SetNotificationVersion(int iVersion);
	DWORD                  GetCSeq()             { return m_CSeq; }
	void                   SetCSeq(DWORD dwCSeq) { m_CSeq = dwCSeq; }
	int                    GetAndIncrementNotifyVersionCounter();
	void                   StartFullNotification();
	bool                   AddUrgentNotification(long iUserID, std::string& sXml);
	bool                   AddTimerNotification(std::list<long> listUserID, std::string& sXml);
	bool                   AddTimerNotification(std::string& sXml);
	void                   Disconnect();
	void                   Connect();
	void                   Serialize(CSegment& seg);
	void                   DeSerialize(CSegment& seg);
	void                   SetEventPackageManager(CSvcEventPackageManager* pCSvcEventPackageManager) {m_pCSvcEventPackageManager = pCSvcEventPackageManager; }
	std::string            ToString();

protected:
	void                   OnNextNotification(CSegment* pParam);
	void                   OnLastNotificationSent(CSegment* pParam);
	void                   OnWaitAckFullNotification(CSegment* pParam);
	void                   OnWaitAckLastNotificationSent(CSegment* pParam);
	void                   OnAckLastNotificationSent(CSegment* pParam);
	void                   OnAckNotificationSent(CSegment* pParam);

protected:
	CSvcEventPackageManager* m_pCSvcEventPackageManager;
	char                   * m_pFromUri, * m_pFromTag, * m_pToUri, * m_pToTag, * m_pContact, * m_pCallId;
	DWORD                    m_dwPartyRsrcID;
	DWORD                    m_dwPartyMonitorID;
	mcXmlTransportAddress    m_transportAddress;
	int                      m_iXmlMaxUsers;
	WORD                     m_serviceID, m_srcUnitId;
	DWORD                    m_csId;
	DWORD                    m_callIndex;
	eRoleType                m_role;
	CStructTm                m_ExpiresTime;
	int                      m_notifyVersionCounter;
	bool                     m_bFullNotificationSent;
	int                      m_iNotificationVersion;
	DWORD                    m_CSeq;
	long                     m_iLastUser;
	std::list<std::string>   m_listUrgentNotificationsQueue;
	std::list<std::string>   m_listTimerNotificationsQueue;

	PDECLAR_MESSAGE_MAP

private:
	CSvcSubscriber& operator =(const CSvcSubscriber& other) {return *this; }
};


// ************************************************************************************
//
//	CSvcEventPackageManager
//
// ************************************************************************************
typedef struct NOTIFY_MSG_TO_CS_STRUCT_tag
{
	DWORD dwPartyRsrcID;
	DWORD dwCsID;
	CSegment* pSegment;
	NOTIFY_MSG_TO_CS_STRUCT_tag()
	{
		dwPartyRsrcID = 0;
		dwCsID = 0;
		pSegment = NULL;
	}
	NOTIFY_MSG_TO_CS_STRUCT_tag(DWORD dwPartyRsrcId, DWORD dwCsId, CSegment* pSeg)
	{
		dwPartyRsrcID = dwPartyRsrcId;
		dwCsID = dwCsId;
		pSegment = pSeg;
	}
}NOTIFY_MSG_TO_CS_STRUCT;

class CSvcEventPackageManager: public CStateMachine
{
CLASS_TYPE_1(CSvcEventPackageManager, CStateMachine )
	// public functions
public:
	CSvcEventPackageManager(CConf* pConf, COsQueue* pRcvMbx=NULL, CMplMcmsProtocol* pMplMcmsProtocol=NULL,
							CConfPartyManagerLocalApi* pMcuApi=NULL, CConfApi* pConfApi=NULL);
	virtual ~CSvcEventPackageManager();
	virtual const char* NameOf() const {return "CSvcEventPackageManager";}
	virtual void	HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	// action functions
	void		SetIsEntryQueue(bool bEQ);
	void		OnSubscribeTout(CSegment* pParam);
	void		OnNotifyTimerTout(CSegment* pParam);
	void		OnNotifyDelay(CSegment* pParam);

	virtual void	ObserverUpdate(void* pSubscriber, WORD event, DWORD val);
	virtual void	Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId);
	void			OnIndNotifyResp(mcIndNotifyResp* pIndNotifyResp);
//	virtual void	Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex);
	virtual BYTE	Notify(CSvcSubscriber* pSubscriber, char* content, char* state, BYTE isDistribute = false);
	void			NotifyFull(CSvcSubscriber* pSubscriber);
	virtual BYTE	CheckRequestValidity(mcIndSubscribe* pSubscribeInd, enSipCodes *SipStatus);
	virtual CObjString*  BuildNotifyContent(BYTE bFull, BYTE bIsNewSubscriber = FALSE);
	CObjString*  	BuildUrgentNotifyContent(CSvcUserType* pUserUrgentUpdated);
	void			SetNotificationVersion(std::string& sXml, int iVersion);
	virtual void	Dump(COstrStream& msg) const;
	virtual void	TerminateConf();
	virtual void	SetConfName(const char* pConfName);
	const char*		GetConfName();
	WORD			CalcTimer();
	CSvcSubscriber*		FindParty(const char* from);
	CSvcSubscriber*		FindSubscriberByRsrcID(DWORD dwPartyRsrcID);
	CSvcSubscriber*		FindSubscriberByMonitorID(DWORD dwPartyMonitorId);
	bool			DisconnectSubscriberByRsrcID(DWORD dwPartyRsrcID, CSegment& seg);
	bool			ConnectSubscriberByRsrcID(DWORD dwPartyRsrcID);
	bool			DeleteSubscriberByRsrcID(DWORD dwPartyRsrcID);
	bool			UnchainSubscriberByRsrcID(DWORD dwPartyRsrcID, CSegment& seg);
	bool			UnchainSubscriberByMonitorID(DWORD dwPartyMonitorId, CSegment& seg);
	bool			ChainSubscriber(CSegment& seg, DWORD dwNewPartyMonitorId);
	void			PartyLeaveIVR(DWORD dwPartyRsrcID);

	//Don't use, but for TDD
	void	ResetNotifyTimer();
	void	ExpireTime();

	int		NotifySubscriberFull(CSvcSubscriber* pSubscriber, long iLastUser, int iNotificationVersion, int iXmlMaxUsers);

protected:

	void	RemoveFromVector(CSvcSubscriber* pSubscriber);

	void	SendSipSubscribeResponse(mcIndSubscribe* pSubscribeInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId = eSipBalancer, DWORD serviceId=0);
	void	SendSipNotify(DWORD dwPartyRsrcID, mcReqNotify* pNotifyMsg, WORD srcUnitId = eSipBalancer, DWORD callIndex = 0, DWORD serviceId=0);
	void	SendMsgToCS(DWORD dwPartyRsrcID, DWORD cs_Id, OPCODE opcode, CSegment* pSeg);

	void	AttachEvents(CCommConf *pCommConf);
	void    DetachEvents(CCommConf *pCommConf);
	const char*	EventToString(const WORD event);
	void	FillSipNotifyStruct(CSipNotifyStruct* pSipNotifyStruct, CSvcSubscriber* pSubscriber);

	CConf* 				m_pConf;

	BYTE				m_runInTdd;
	CMplMcmsProtocol*	m_pMplMcmsProtocol;
	COsQueue*			m_pRcvMbx;
	CConfPartyManagerLocalApi*	m_pConfPartyManagerApi;
	CConfApi*			m_pConfApi;
	BYTE				m_SubTimer, m_connectTimer, m_NotifyTimer, m_LoadTimer;
	int					m_version;
	std::vector< CSvcSubscriber * >	m_EventSubscribersList;
	std::set<DWORD>		m_setAddedParticipantsRsrcIDs;
	std::set<DWORD>		m_setConnectedSuscribersRsrcIDs;
	CSvcConfInfoType* 	m_pEventConfInfo;
	CObjString*			m_pXMLStr;
	char*				m_pConfName;
	mcReqNotify*		m_pNotifyMsg;

	PDECLAR_MESSAGE_MAP

private:
	CSvcEventPackageManager& operator=(const CSvcEventPackageManager &other){return *this;}

	std::queue<NOTIFY_MSG_TO_CS_STRUCT> m_queueCS;
};

// ************************************************************************************
//
//	CSvcEventPackageDispatcher
//
// ************************************************************************************
class CSvcEventPackageDispatcher: public CPObject
{
	CLASS_TYPE_1(CSvcEventPackageDispatcher, CPObject )
	// public functions
public:
	CSvcEventPackageDispatcher(CConf* pConf, COsQueue* pRcvMbx = NULL, CMplMcmsProtocol* pMplMcmsProtocol = NULL, CConfPartyManagerLocalApi* pConfPartyManagerApi = NULL);
	virtual ~CSvcEventPackageDispatcher();
	virtual const char* NameOf() const {return "CSvcEventPackageDispatcher";}

	void	SetIsEntryQueue(bool bEQ);
	void	Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId);
	void	OnIndNotifyResp(mcIndNotifyResp* pIndNotifyResp);
	CSvcSubscriber* 	FindParty(char* from, DWORD CSeq, char* callId);
	CSvcSubscriber*		FindPartyInSIPCX(const char* pFromStr);
	void	Dump();
	void	TerminateConf();
	void	HandleObserverUpdate(CSegment *pSeg, WORD type);
	void	LoadManagerAccept();

	//Don't use, but for TDD
	void	ResetNotifyTimer();

	bool	DisconnectSubscriberByRsrcID(DWORD dwPartyRsrcID, CSegment& seg);
	bool	ConnectSubscriberByRsrcID(DWORD dwPartyRsrcID);
	bool	DeleteSubscriberByRsrcID(DWORD dwPartyRsrcID);
	bool	UnchainSubscriberByRsrcID(DWORD dwPartyRsrcID, CSegment& seg);
	bool	UnchainSubscriberByMonitorID(DWORD dwPartyMonitorId, CSegment& seg);
	bool	ChainSubscriber(CSegment& seg, DWORD dwNewPartyMonitorId);
	void	PartyLeaveIVR(DWORD dwPartyRsrcID);

protected:

	void	SendSipSubscribeResponse(mcIndSubscribe* pSubscribeInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId=0, DWORD serviceId=0);

	BYTE	m_terminating;

	CSvcEventPackageManager* m_pSvcConfPackage;
	CMplMcmsProtocol*	m_pMplMcmsProtocol;

private:
	CSvcEventPackageDispatcher(const CSvcEventPackageDispatcher &other);
	CSvcEventPackageDispatcher& operator=(const CSvcEventPackageDispatcher &other){return *this;}
};


// ************************************************************************************
//
//	CSvcConfDescriptionType
//
// ************************************************************************************
class CSvcConfDescriptionType : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcConfDescriptionType, CSvcConfPackageType )
// public functions
public:
	CSvcConfDescriptionType();
	CSvcConfDescriptionType(const char* pszUri, CStructTm& stConfStartDateTime);
	virtual const char* NameOf() const { return "CSvcConfDescriptionType";}
	virtual ~CSvcConfDescriptionType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);

protected:
	std::string m_sConfStartDateTime;
	std::string m_sUri;

private:
	CSvcConfDescriptionType& operator=(const CSvcConfDescriptionType &other){return *this;}
};

// ************************************************************************************
//
//	CSvcHostInfoType
//
// ************************************************************************************

class CSvcHostInfoType : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcHostInfoType, CSvcConfPackageType )
// public functions
public:
	CSvcHostInfoType();
	CSvcHostInfoType(const char* pszDisplayText, const char* pszWebPage);
	virtual const char* NameOf() const { return "CSvcHostInfoType";}
	virtual ~CSvcHostInfoType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);

protected:
	std::string m_sDisplayText;
	std::string m_sWebPage;

private:
	CSvcHostInfoType& operator=(const CSvcHostInfoType &other){return *this;}
};

// ************************************************************************************
//
//	CSvcConfStateType
//
// ************************************************************************************

class CSvcConfStateType : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcConfStateType, CSvcConfPackageType )
public:
	CSvcConfStateType();
	virtual const char* NameOf() const { return "CSvcConfStateType";}
	virtual ~CSvcConfStateType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);
	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, int nUsersCount, BYTE bFull);
	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, SvcUsersMap & mapUsers, BYTE bFull);

private:
	CSvcConfStateType& operator=(const CSvcConfStateType &other){return *this;}
};

// ************************************************************************************
//
//	CSvcFloorInfoType
//
// ************************************************************************************
class CSvcFloorInfoType : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcFloorInfoType, CSvcConfPackageType )
// public functions
public:
	CSvcFloorInfoType();
	CSvcFloorInfoType(DWORD dwConfID);
	virtual const char* NameOf() const { return "CSvcFloorInfoType";}
	virtual ~CSvcFloorInfoType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);

protected:
	DWORD				m_dwConfID;

private:
	CSvcFloorInfoType& operator=(const CSvcFloorInfoType &other){return *this;}
};

// ************************************************************************************
//
//	CSvcEndPointType
//
// ************************************************************************************
class CSvcEndPointType : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcEndPointType, CSvcConfPackageType )
// public functions
public:
	CSvcEndPointType();
	CSvcEndPointType(const char*  endpointName, DWORD dwPartyID /*,WORD isAudioOnly*/);
	CSvcEndPointType(const char*  endpointName, std::string sFrom, std::string sTo, const CConfParty* pConfParty /*,WORD isAudioOnly*/);
	virtual const char* NameOf() const { return "CSvcEndPointType";}
	virtual ~CSvcEndPointType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);
	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, bool bIsNewSubscriber, WORD bIsAudioOnly, bool bUrgent);
	virtual void	SerializeXmlFull(CXMLDOMElement* pFatherNode);
	eEndPointStatusType	GetEndPointStatus();
	char*			 	GetEndPointStatusByString() const;
	const char* 		GetEndpointUri();

	/*void*/BYTE 		SetEndPointStatus(DWORD status);
	void  		SetMediaStatus(eMediaStatusType mediaType);
	void		MuteViaFocus(BYTE mutedViaFocus, eMediaContentType mediaType);
	BYTE  		IsMutedViaFocus(eMediaContentType mediaType);
	//for media
	void	MuteMedia(eMediaContentType mediaType,BYTE onOff);
	BYTE	IsMuted(eMediaContentType mediaType);

	void	SetActiveSpeaker(bool bSpeaker);
	bool	IsActiveSpeaker() {return m_bActiveSpeaker;}
	bool	UpdateMedia(CConfParty* pConfParty, bool& bUrgent);

protected:
	CSmallString	m_endpointEntity;
	CMediaList	m_mediaList;
	eEndPointStatusType	m_EndPointStatus;
	BYTE m_audioMutedViaFocus;
	BYTE m_videoMutedViaFocus;

	bool			m_bActiveSpeaker;

	std::string		m_sFrom;
	std::string		m_sTo;
	std::string		m_sEntity;
	std::string		m_sTime;

	const CConfParty* m_pConfParty;

private:
	CSvcEndPointType& operator=(const CSvcEndPointType &other){return *this;}
};



// ************************************************************************************
//
//	CSvcActionType
//
// ************************************************************************************
class CSvcActionType : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcActionType, CSvcConfPackageType )
// public functions
public:
	CSvcActionType();
	CSvcActionType(eActionType actionType);
	virtual const char* NameOf() const { return "CSvcActionType";}
	virtual ~CSvcActionType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);

	eActionType GetActionType();
	const char* GetActionTypeAsString();
	void SetActionType(eActionType actionType);
	void SetActionTypeStatus(eActionType actionType);
protected:

	eActionType	m_ActionType;

private:
	CSvcActionType& operator=(const CSvcActionType &other){return *this;}
};

// ************************************************************************************
//
//	CSvcUserActions
//
// ************************************************************************************
class CSvcUserActions : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcUserActions, CSvcConfPackageType )
// public functions
public:
	CSvcUserActions();
	CSvcUserActions(const char*  userName);
	virtual const char* NameOf() const { return "CSvcUserActions";}
	virtual ~CSvcUserActions();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);
	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull,WORD bIsAudioOnly);

	eTypeState		GetUserActionTypeState(eActionType actionType);
	void			SetUserActionType(eActionType type);

protected:
	CSmallString	m_userEntity;
	CSvcActionType*	m_pActionStream[NUM_OF_ACTIONS];
//	eUserActionType	m_UserActionType;

private:
	CSvcUserActions& operator=(const CSvcUserActions &other){return *this;}
};

// ************************************************************************************
//
//	CSvcPartyExtension
//
// ************************************************************************************
class CSvcPartyExtension : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcPartyExtension, CSvcConfPackageType )
// public functions
public:
	CSvcPartyExtension();
	CSvcPartyExtension(const char* userEntity,const char* userContactInfo,DWORD partyId);
	virtual const char* NameOf() const { return "CSvcPartyExtension";}
	virtual ~CSvcPartyExtension();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);
	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull,WORD bIsAudioOnly);

	eTypeState GetUserActionTypeState(eActionType actionType);
	void 		SetUserActionType(eActionType type);
	const 	char*   GetUserContactInfo() const;
	void 	SetUserContactInfo(const char* userContactInfo);

	DWORD 		GetPartyId();
protected:
	DWORD 			m_partyId;
	CSmallString	m_userContactInfo;
	CSvcUserActions*   m_pUserActions;

private:
	CSvcPartyExtension& operator=(const CSvcPartyExtension &other){return *this;}
};

// ************************************************************************************
//
//	CSvcActiveSpeaker
//
// ************************************************************************************
class CSvcActiveSpeaker : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcActiveSpeaker, CSvcConfPackageType )

public:
	CSvcActiveSpeaker();
	virtual const char* NameOf() const { return "CSvcActiveSpeaker";}
	virtual ~CSvcActiveSpeaker();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);

	void	SetActiveSpeaker(const char* activeSpeaker, const char* speakerUri);

	const char* 	GetActiveSpeaker() const;
	const char* 	GetSpeakerUri() const;
	void 			SetSpeakerContactInfo(const char* userContactInfo);
	const char* 	GetSpeakerContactInfo() const;
	void 			SetSpeakerPartyId(DWORD partyId);
	DWORD 			GetSpeakerPartyId();
protected:

	CSmallString m_speakerUri;
	CSmallString m_activeSpeaker;
	CSmallString m_speakerContactInfo;
	DWORD   m_partyId;

private:
	CSvcActiveSpeaker& operator=(const CSvcActiveSpeaker &other){return *this;}
};


// ************************************************************************************
//
//	CSvcConfExtension
//
// ************************************************************************************
class CSvcConfExtension : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcConfExtension, CSvcConfPackageType )

public:
	CSvcConfExtension();
	CSvcConfExtension(const char* contactInfo, DWORD confId);
	virtual const char* NameOf() const { return "CSvcConfExtension"; }
	virtual ~CSvcConfExtension();

	virtual void SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);

	void        SetActive(BYTE onOff)                                               { m_bActive = onOff; }
	BYTE        GetActive()                                                         { return m_bActive; }
	void        SetActiveSpeaker(const char* activeSpeaker, const char* speakerUri) { m_pActiveSpeaker->SetActiveSpeaker(activeSpeaker, speakerUri); }
	const char* GetActiveSpeaker() const                                            { return m_pActiveSpeaker->GetActiveSpeaker(); }
	const char* GetSpeakerUri() const                                               { return m_pActiveSpeaker->GetSpeakerUri(); }
	void        SetSpeakerContactInfo(const char* userContactInfo);
	const char* GetSpeakerContactInfo() const                                       { return m_pActiveSpeaker->GetSpeakerContactInfo(); }
	void        SetConfContactInfo(const char* contactInfo);
	const char* GetConfContactInfo() const                                          { return m_confContactInfo.GetString(); }
	DWORD       GetConfId()                                                         { return m_confId; }
	void        SetSpeakerPartyId(DWORD partyId)                                    { m_pActiveSpeaker->SetSpeakerPartyId(partyId); }
	DWORD       GetSpeakerPartyId()                                                 { return m_pActiveSpeaker->GetSpeakerPartyId(); }

protected:
	BYTE               m_bActive;
	CSvcActiveSpeaker* m_pActiveSpeaker;
	CSmallString       m_confContactInfo;
	DWORD              m_confId;

private:
	CSvcConfExtension& operator =(const CSvcConfExtension& other){
		return *this;
	}
};


// ************************************************************************************
//
//	CSvcUserType
//
// ************************************************************************************
class CSvcUserType : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcUserType, CSvcConfPackageType )
public:
	CSvcUserType();
	CSvcUserType(CConfParty* pConfParty, DWORD dwUserRsrcId, BOOL bIsWebDialInParty = FALSE);
	CSvcUserType(CConfParty* pConfParty, std::string sFrom, std::string sFromTag, std::string sTo, BOOL bIsWebDialInParty = FALSE);
	virtual const char* NameOf() const {
		return "CSvcUserType";
	}
	virtual ~CSvcUserType();

	virtual void   SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);
	virtual void   SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, BYTE bIsNewSubscriber, bool bUrgent);
	virtual void   SerializeXmlFull(CXMLDOMElement* pFatherNode);

	friend WORD    operator==(const CSvcUserType& first, const CSvcUserType& second);

	void           MarkForDelete();
	BYTE           IsMarkedForDelete();
	BYTE           CanDelParty();

	BYTE           SetEndPointStatus(DWORD status);
	void           MuteViaFocus(BYTE mutedViaFocus, eMediaContentType mediaType);
	BYTE           IsMutedViaFocus(eMediaContentType mediaType);
	void           SetActiveSpeaker(bool bSpeaker);
	void           SetUriAttribute(char* uri);

	char*          GetPartyStatus() const;
	PartyMonitorID GetUserMonitorId() { return m_dwUserMonitorId; }
	PartyRsrcID    GetUserRsrcId() { return m_dwUserRsrcId; }
	void           SetUserRsrcId(PartyRsrcID partyId) { m_dwUserRsrcId = partyId; }
	const char*    GetUriAttribute() const { return m_uriAttribute.GetString(); }
	const char*    GetUserContactInfo() const { return m_userContactInfo.GetString(); }
	void           SetUserContactInfo(const char* userContactInfo);
	BYTE           WasUpdated() { return (eNoChange != m_state); }

	//for media
	void           MuteMedia(eMediaContentType mediaType, BYTE onOff);
	void           SetMediaStatus(eMediaStatusType mediaType);
	BYTE           IsMediaMuted(eMediaContentType mediaType) { return m_pEndPoint->IsMuted(mediaType); }
	BOOL           GetIsWebDialInParty() { return m_bIsWebDialInParty; }
	bool           UpdateMedia(CConfParty* pConfParty, bool& bUrgent);
	void           LeaveIVR() { m_bInIVR = false; }
	bool           IsInIVR() { return m_bInIVR; }

	std::string  m_sFrom;

protected:
	bool                m_bInIVR;
	BOOL                m_bIsWebDialInParty;
	PartyMonitorID      m_dwUserMonitorId;
	PartyRsrcID         m_dwUserRsrcId;
	WORD                m_bIsAudioOnly;
	CSvcEndPointType*   m_pEndPoint;
	CSvcUserActions*    m_pUserActions;
	CSvcPartyExtension* m_pPartyExtension;
	CSmallString        m_uriAttribute;
	CSmallString        m_userContactInfo;
	BYTE                m_markedforDelete;
	std::string         m_sFromTag;
	std::string         m_sTo;
	CConfParty*         m_pConfParty;
	time_t              m_longTime;

private:
	CSvcUserType& operator=(const CSvcUserType& other){
		return *this;
	}
};


// ************************************************************************************
//
//	CSvcConfInfoType
//
// ************************************************************************************
class CSvcConfInfoType : public CSvcConfPackageType
{
	CLASS_TYPE_1(CSvcConfInfoType, CSvcConfPackageType )

public:
	CSvcConfInfoType();
	CSvcConfInfoType(CCommConf* pCommConf, const char* confUri, const char* contactInfo);
	CSvcConfInfoType(CConf* pConf, const char* confUri, const char* contactInfo);
	virtual const char* NameOf() const
	{
		return "CSvcConfInfoType";
	}
	virtual ~CSvcConfInfoType();

	void           SerializeUrgentXml(CXMLDOMElement* pFatherNode, CSvcUserType* pUserUrgentUpdated);
	virtual void   SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);
	virtual void   SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, BYTE bIsNewSubscriber);
	virtual int    SerializeXmlFull(std::list<std::string>* pListXml, int nMaxUsersInXml);
	virtual long   SerializeXmlFull(std::string* psFullStr, long iLastUser, int iNotificationVersion, int iXmlMaxUsers);

	const char*    GetEntity() { return m_entityAtr.GetString(); }
	CSvcUserType*  FindUserByMonitorID(PartyMonitorID partyId);
	CSvcUserType*  FindUserByRsrcID(PartyRsrcID partyId);
	long           FindUserID(CSvcUserType* pUser);
	BYTE           WasUpdated() { return (eNoChange != m_state); }

	//party functions
	CSvcUserType*  AddParty(CConfParty* pConfParty, PartyRsrcID partyId, BOOL bIsWebDialInParty);
	void           AddParty(CConfParty* pConfParty, const std::string& sFrom, const std::string& sFromTag, const std::string& sTo, BOOL bIsWebDialInParty);
	void           DelParty(PartyMonitorID partyId, int numSubscribers);
	void           DelDisconnectedParties();
	void           DelDelletedParties();
	void           SetEndPointStatus(PartyMonitorID partyId, DWORD status);
	void           SetActiveSpeaker(PartyMonitorID partyId, const char* speaker);
	void           SetUserContactInfo(PartyMonitorID partyId, const char* userContactInfo);
	void           SetActive(DWORD onOff) { m_pConfExtension->SetActive(onOff); }
	void           SetUriAttribute(PartyMonitorID partyId, char* uri);
	void           MutePartyMedia(PartyMonitorID partyId, eMediaContentType mediaType, BYTE onOff);
	void           MuteViaFocus(PartyMonitorID partyId, eMediaContentType mediaType, BYTE mutedViaFocus);
	const char*    GetActiveSpeaker() const { return m_pConfExtension->GetActiveSpeaker(); }

	void           SetConfContactInfo(const char* contactInfo);
	const char*    GetConfContactInfo() const { return m_pConfExtension->GetConfContactInfo(); }

	bool           UpdateMedia(CConfParty* pConfParty, PartyRsrcID partyId, bool& bUrgent, CSvcUserType** ppUserUrgentUpdated);
	void           PartyLeaveIVR(PartyRsrcID partyId);

protected:

	void          SetupUsersList(CCommConf* pCommConf);

	CConf*                   m_pConf;
	CSvcConfDescriptionType* m_pConfDescription;
	CSvcHostInfoType*        m_pHostInfo;
	CSvcConfStateType*       m_pConfState;
	CSvcFloorInfoType*       m_pFloorInfo;
	CSvcConfExtension*       m_pConfExtension;
	SvcUsersMap              m_mapUsers;
	CSmallString             m_entityAtr;
	unsigned int             m_version;

private:
	long                     m_iUserCounter;

private:
	CSvcConfInfoType& operator=(const CSvcConfInfoType& other)
	{
		return *this;
	}
};


#endif /* SVCEVENTPACKAGE_H_ */
