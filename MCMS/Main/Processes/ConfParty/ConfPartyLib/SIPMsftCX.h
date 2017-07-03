
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
//Module Name:  SIPMSPKG.h
//
//General Description:  
//
//    1.	SIP CX - Microsoft Conference package
//
//Generated By: Ori P.                            Date: 19.05.05
//
//Revisions and Updates: 
//
//Date         Updated By         Description
//========   ==============   =====================================================================
// 
//=================================================================================================

#ifndef __SIP_MS_CONFPACKAGE_H__
#define __SIP_MS_CONFPACKAGE_H__


#include "TaskApp.h"
#include "Conf.h"
#include "ObjString.h"
#include "SipEventPackageCommon.h"


#define MEDIA_STREAMS	4



// ************************************************************************************
//
//	CBasicConfPackageType
//
// ************************************************************************************


class CBasicConfPackageType : public CPObject
{
	CLASS_TYPE_1(CBasicConfPackageType, CPObject )	

// public functions
public: 
	
	virtual const char* NameOf() const { return "CBasicConfPackageType";}
	CBasicConfPackageType();
	CBasicConfPackageType(char* pName);
	virtual ~CBasicConfPackageType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode) = 0;

	const char*		GetElementName();

protected:

	char	m_elementName[H243_NAME_LEN];
};


// ************************************************************************************
//
//	CConfPackageType
//
// ************************************************************************************


class CConfPackageType : public CBasicConfPackageType
{
	CLASS_TYPE_1(CConfPackageType, CBasicConfPackageType )
// public functions
public: 
	CConfPackageType();
	CConfPackageType(char* pName);
	virtual const char* NameOf() const { return "CConfPackageType";}
	virtual ~CConfPackageType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode);
	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull) = 0;
	virtual void	SerializeState(CXMLDOMElement* pFatherNode, BYTE bFull);

	eTypeState GetState();
	const char* GetStateByString();
	
protected:

	eTypeState m_state;
};



// ************************************************************************************
//
//	CCXMediaStateType
//
// ************************************************************************************


class CCXMediaStateType : public CBasicConfPackageType
{
	CLASS_TYPE_1(CCXMediaStateType, CBasicConfPackageType )
// public functions
public: 
	CCXMediaStateType();
	CCXMediaStateType(BYTE onOff);
	virtual const char* NameOf() const { return "CCXMediaStateType";}
	virtual ~CCXMediaStateType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode);

	void	SetMute(BYTE onOff);
	BYTE	IsMuted();
	
protected:

	BYTE	m_bMuteAttribute;
};


// ************************************************************************************
//
//	CCXMediaType
//
// ************************************************************************************


class CCXMediaType : public CConfPackageType
{
	CLASS_TYPE_1(CCXMediaType, CConfPackageType )
// public functions
public: 
	CCXMediaType();
	CCXMediaType(eMediaContentType mediaType);
	virtual const char* NameOf() const { return "CCXMediaType";}
	virtual ~CCXMediaType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);

	eMediaContentType	GetContent();
	const char*		GetContentAsString();
	//for media
	void	MuteMedia(BYTE onOff);
	BYTE	IsMuted();
	
protected:

	CCXMediaStateType*	m_pReceiveState;
	eMediaContentType	m_MediaContent;
};

	
// ************************************************************************************
//
//	CCXInitialState
//
// ************************************************************************************


class CCXInitialState : public CBasicConfPackageType
{
	CLASS_TYPE_1(CCXInitialState, CBasicConfPackageType )
// public functions
public: 
	CCXInitialState();
	virtual ~CCXInitialState();
	virtual const char* NameOf() const { return "CCXInitialState";}

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode);

	void	SetMute(BYTE onOff);
	void	SetAdmin();
	void	SetJoinAsDialOut();
	void	SetRejoinAsDialOut();
	const char* GetRole();
	const char*	GetJoinType();
	const char*	GetRejoinType();
	
protected:

	eRoleType	m_role;
	eJoinModeType	m_joinType;
	eJoinModeType	m_rejoinType;
	BYTE	m_bMute;
};

// ************************************************************************************
//
//	CCXPropertiesType
//
// ************************************************************************************


class CCXPropertiesType : public CConfPackageType
{
	CLASS_TYPE_1(CCXPropertiesType, CConfPackageType )
// public functions
public: 
	CCXPropertiesType();
	CCXPropertiesType(CCommConf* pCommConf);
	virtual const char* NameOf() const { return "CCXPropertiesType";}
	virtual ~CCXPropertiesType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);

	void	SetActive(BYTE onOff);
	void	SetLocked(BYTE onOff);
	void	SetRollCall(BYTE onOff);
	void	SetAnnouncements(BYTE onOff);
	void	SetRecorded(BYTE onOff);

	BYTE	GetActive();
	BYTE	GetLocked();
	
protected:

	BYTE	m_bActive, m_bLocked, m_bRollCall, m_bAnnouncement, m_bRecorded;
	CCXMediaType*	m_pMediaStream[MEDIA_STREAMS];
	CCXInitialState*	m_pInitialState;	
};

// ************************************************************************************
//
//	CCXUserType
//
// ************************************************************************************


class CCXUserType : public CConfPackageType
{
	CLASS_TYPE_1(CCXUserType, CConfPackageType )
// public functions
public: 
	CCXUserType();
	CCXUserType(CConfParty* pConfParty);
	virtual const char* NameOf() const { return "CCXUserType";}
	virtual ~CCXUserType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);
	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, BYTE bIsNewSubscriber);

	friend WORD operator==(const CCXUserType& first,const CCXUserType& second);

	void	MarkForDelete();
	BYTE	IsMarkedForDelete();
	BYTE	CanDelParty();

	void	SetDisplayName(char* name);
	void	SetDisplayPhoneNumber(char* num);
	void	SetSipUri(char* sipUri);
	void	SetAdministrator(BYTE onOff);
	void	SetPartyStatus(DWORD status, DWORD attendedStatus);
	void	SetDialOutParty();
	BYTE	SetDisconnectReason(DWORD reason);
	void	SetActiveSpeaker(BYTE onOff);
	void	SetUriAttribute(char* uri);

	const char*	GetDisplayName() const;
	char*	GetPartyRole() const;
	char*	GetPartyStatus() const;
	char*   GetPartyJoinMode() const;
	char*	GetPartyDisconnectReason() const;
	DWORD	GetUserId();
	BYTE	GetActiveSpeaker();

	//for media
	void	MuteMedia(eMediaContentType mediaType, BYTE onOff);
	BYTE	IsMediaMuted(eMediaContentType mediaType);

protected:


	DWORD	m_userId;
	CSmallString	m_displayName;
	CSmallString	m_displayPhoneNumber;
	BYTE	m_isSip;
	CSmallString	m_sipUri;
	eRoleType	m_role;
	eStatusType	m_status;
	eJoinModeType	m_joinMode;
	eDisconnectReasonType	m_disconnectReason;
	BYTE	m_activelyTalking;
	CCXMediaType*	m_pMediaStream[MEDIA_STREAMS];
	CSmallString	m_uriAttribute;
	eItemState	m_sendDataBeforeDeleted;
	BYTE	m_isCallBack, m_markedforDelete;
};


// ************************************************************************************
//
//	CCXConfInfoType
//
// ************************************************************************************


class CCXConfInfoType : public CConfPackageType
{
	CLASS_TYPE_1(CCXConfInfoType, CConfPackageType )
// public functions
public: 
	CCXConfInfoType();
	CCXConfInfoType(CCommConf* pCommConf, const char* confUri);
	virtual const char* NameOf() const { return "CCXConfInfoType";}
	virtual ~CCXConfInfoType();

	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull);
	virtual void	SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, BYTE bIsNewSubscriber);

	const char*	GetEntity();
	CCXUserType*	FindUser(DWORD partyId);

	BYTE	WasUpdated();

	//party functions
	void	AddParty(CConfParty* pConfParty);
	void	DelParty(DWORD partyId, int numSubscribers);
	void	DelDisconnectedParties();
	void	SetDisplayName(DWORD partyId, char* name);
	void	SetDisplayPhoneNumber(DWORD partyId, char* num);
	void	SetSipUri(DWORD partyId, char* sipUri);
	void	SetAdministrator(DWORD partyId, BYTE onOff);
	void	SetPartyStatus(DWORD partyId, DWORD status, DWORD attendedStatus, int len);
	void	SetDialOutParty(DWORD partyId);
	void	SetDisconnectReason(DWORD partyId, DWORD reason);
	void	SetActiveSpeaker(DWORD partyId, BYTE onOff);
	void	SetUriAttribute(DWORD partyId, char* uri);
	void	MutePartyMedia(DWORD partyId, eMediaContentType mediaType, BYTE onOff);
	
	//conf properties functions
	void	SetActive(BYTE onOff);
	void	SetLocked(BYTE onOff);
	void	SetRollCall(BYTE onOff);
	void	SetAnnouncements(BYTE onOff);
	void	SetRecorded(BYTE onOff);
	
protected:

	void	SetupUsersList(CCommConf* pCommConf);

	CCXPropertiesType*	m_pProperties;
	std::vector< CCXUserType * > 	m_UsersList;
	CSmallString	m_entityAtr;
};


#endif
