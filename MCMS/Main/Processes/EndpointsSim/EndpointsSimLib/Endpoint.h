//+========================================================================+
//                       Endpoint.h								           |
//				Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       Endpoint.h												   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily													   |
//+========================================================================+

#ifndef __ENDPOINT_H__
#define __ENDPOINT_H__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
#include "StateMachine.h"
#include "BoardDetails.h"
#include "VendorInfo.h"
#include "IpCsContentRoleToken.h"
#include "EndpointsSim.h"

////////////////////////////////////////////////////////////////////////////
//  DECLARATIONS
//
class CCapSet;
class CCapSetsList;
class CH323Behavior;
class CH323BehaviorList;
class CTaskApi;
class CMplMcmsProtocol;


////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
//
#define MAX_EP_NAME		H243_NAME_LEN
#define MAX_CONF_NAME	H243_NAME_LEN
#define RAND_NUM_FOR_EP_SIM	   7   //for token requests from EpSim

const OPCODE ROLE_TOKEN_OWNER_TOUT	= 11001;
const DWORD  ROLE_TOKEN_OWNER_TIME	= 10;

enum enEndpointState {
	eEpStateIdle = 0,
	eEpStateConnecting,
	eEpStateConnected,
	eEpStateDisconnected,
	eEpStateDisconnecting,
	eEpStateUnknown // should be last
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CCommonComMode : public CPObject
{
	CLASS_TYPE_1(CCommonComMode,CPObject)

public:
	// constructors, distructors and operators
	CCommonComMode();
	virtual const char* NameOf() const { return "CCommonComMode";}
	void Create(const CCapSet& rCapSet);
	virtual ~CCommonComMode();


	DWORD	GetAudioMode() const { return m_audio; }
	BOOL	IsAudioToOpen() const;
	DWORD	GetVideoMode() const { return m_video; }
	DWORD	GetPresentVideoMode() const { return m_presentVideo; }
	BOOL	IsVideoToOpen() const;
	BOOL	IsFeccToOpen() const { return m_isFecc; }
	BOOL	IsEncryptedToOpen() const { return m_isEncrypted; }
	BOOL	IsH239ToOpen() const { return m_isH239; }

protected:
	DWORD	m_audio;
	DWORD	m_video;
	DWORD	m_presentVideo;
	BOOL	m_isFecc;
	BOOL	m_isEncrypted;
	BOOL	m_isH239;
	BOOL	m_isLPR;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CEndpoint : public CStateMachine
{
	CLASS_TYPE_1(CEndpoint,CStateMachine)

	
public:
	// constructors, distructors and operators
	virtual const char* NameOf() const { return "CEndpoint";}
	CEndpoint( CTaskApi *pCSApi, const CCapSet& rCap, const CH323Behavior& rBehav );
	CEndpoint(const CEndpoint& other);
	virtual ~CEndpoint();
	CEndpoint& operator =(const CEndpoint& other);


		// pure virtual functions
	virtual char* GetIpStr() const = 0;
	virtual void GetIpStr(char* str) const = 0;
	
	virtual void SetIp( const char* pszEpIp ) = 0;
	virtual DWORD GetIpDword() = 0;
	
	
	virtual void SetIpVersion( DWORD ipVer ) = 0;
	virtual DWORD GetIpVersion() const = 0;
	
	virtual void SetTransportAddress( mcTransportAddress* mcTA ) = 0;
	virtual mcTransportAddress* GetTransportAddress() = 0;
	
	virtual void OnScpStreamsRequest(CSegment* pParam);
	virtual void OnGuiConnectEp() = 0;
	virtual void OnGuiDisconnectEp() = 0;
	virtual void OnGuiUpdateEp(CSegment* pParam) = 0;
	virtual void OnGuiDeleteEp() = 0;
	virtual eEndpointsTypes GetEpType() const = 0;
	virtual void Serialize( CSegment& rSegment ) const = 0;
	virtual void SerializeDetails( CSegment& rSegment ) const = 0;
	virtual void DeSerialize(CSegment& rParam) = 0;
	virtual void HandleProtocolEvent( CMplMcmsProtocol* pMplProtocol ) = 0;
	virtual void HandleIsdnProtocolEvent( const DWORD opcode, BYTE* pData, CSegment* pNetSetupParams=NULL ) {}
	virtual void SendChangeMode() = 0;

	// GET / SET
	virtual void SetArrayIndex( const WORD ind );
	const char* GetName() const { return m_szEpName; }
	void	SetName( const char* pszEpName );
	DWORD	GetID() const { return m_nEpID; }
	void	SetID(const DWORD nId);
	BOOL	IsChanged() const { return m_isChanged; }
	void	ClearChanged() { m_isChanged = FALSE; }
	BOOL	IsMuted() const { return m_isMuted; }
	BOOL	IsReadyToDelete() const { return m_isReadyToDelete; }
	DWORD	GetDialDirection() const { return m_dialDirection; }
	void	SetDialDirection(const DWORD dir);
	void	SetState(const enEndpointState state);
	DWORD 	GetEpPartyID() const { return m_partyID; }
	DWORD 	GetEpConfID() const { return m_confID; }
	DWORD	GetEpConnectionID() const { return m_connectionID; }
	void	SetEpConfID( const DWORD confID );
	void	SetEpPartyID( const DWORD partyID );
	virtual void	SetEpConnectionID( const DWORD connID, const DWORD netConnectionId=(DWORD)(-1));
	const char* GetConfName() const { return m_szConfName; }
	void	SetConfName( const char* pszConfName );
	const	CBoardDetails* GetAudioBoard() const { return &m_rAudioBoard; }
	BOOL	SetAudioBoardDetails(const WORD boardId,const WORD subBoardId,const WORD unitId,const WORD portId)
					{ return m_rAudioBoard.SetBoardDetails(boardId,subBoardId,unitId,portId); }
	void	CleanAudioBoardDetails()
					{ return m_rAudioBoard.CleanDetails(); }
	virtual BOOL SetNetBoardDetails(const WORD boardId, const WORD subBoardId, const WORD spanId, const WORD portId, const DWORD netConnectionId=(DWORD)-1)
					{ PASSERT(1); return FALSE; }
	virtual DWORD GetCallIndex() const { return m_nCsCallIndex; } // get Cs CallIndex for IP or net_connection_id for ISDN
	virtual void SetCallIndex(const DWORD nCsCallIndex); // set Cs CallIndex for IP or net_connection_id for ISDN
	void	SetCsHandle(const WORD wCsHandle);
	void	SetCsSrcUnit(const WORD wCsSrcUnit);
	virtual BOOL IsFeccCall() const { return FALSE; }
	virtual BOOL IsH239Call() const { return FALSE; }
	virtual BOOL IsLprCall() const { return FALSE; }
	DWORD GetCSID(void) const;
	void  SetMRMInfo(const CBoardDetails &mrmBoard, const DWORD &connectionId, const DWORD &channelId);

		/// API calls
	void	SendDtmf(const char* pszDtmf, WORD wDtmfSource = eDtmfSourceAUDIO) const;
	void	ActiveSpeaker() const;
	void 	AudioSpeaker() const;
	void	Mute();
	void	Unmute();
	void	FeccTokenRequest();
	void	FeccTokenRelease();
	void	FeccKeyRequest(const char* pszDtmf);
	void	H239TokenRequest();
	void	H239TokenRelease();
	virtual void Mute(WORD wAudioMuteByPort, WORD wVideoMuteByPort,
						char *szAudioMuteByDirection, char *szVideoMuteByDirection,
						char *szAudioMuteByInactive, char *szVideoMuteByInactive){}
	virtual void UpdateChannels(const bool au=true,const bool vi=true,const bool fecc=true,const bool h239=true,
						const BYTE recapMode=0,const char* pszManufacturerName="No change", const CCapSet *pCapSet=0);
	virtual void OnGuiSetCapsForEp(CCapSet *pCap);
	void   LprModeChangeReq(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout);
	CVendorInfo* GetVendor(){return &m_rVendor;}
	void SetVendor(CVendorInfo* newVendor) {m_rVendor = *newVendor;}

protected:
		// pure virtual functions
	virtual void CleanAfterDisconnect() = 0;
	virtual BOOL IsConnectionCompleted() const = 0;
	virtual void Disconnect( CMplMcmsProtocol* pMplProtocol ) = 0;
	virtual void SendMuteIndication() const = 0;
	virtual void SendUnmuteIndication() const = 0;
	virtual void SendFeccTokenRequestIndication() const = 0;
	virtual void SendFeccTokenReleaseIndication() const = 0;
	virtual void SendFeccKeyRequestIndication(const char* pszDtmf) const = 0;
	virtual void SendH239TokenRequestIndication() = 0;
	virtual void SendH239TokenReleaseIndication() = 0;
	virtual void SendLprModeChangeRequestIndication(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout) = 0;

	BYTE	DtmfChar2Byte(const char& cDtmf) const;

//	void 	FillCsProtocol( CMplMcmsProtocol* pMplProt,
//				const DWORD opcode,const BYTE* pData,const DWORD nDataLen,
//				const DWORD channelIndexParam = 0xFFFFFFFF,const DWORD channelMcIndexParam = 0xFFFFFFFF ) const;

protected:
	char			m_szEpName[MAX_EP_NAME];
	DWORD			m_nEpID;
	WORD			m_arrayIndex;
	BOOL			m_isChanged;
	BOOL			m_isToBeDeleted;
	BOOL			m_isReadyToDelete;
	DWORD			m_dialDirection;
//	CTaskApi*		m_pCSApi;
	CCapSet*		m_pCap;
	CCapSet*		m_pConfCap;
	CCommonComMode	m_rComMode;
	CH323Behavior*	m_pBehavior;
	enEndpointState	m_enEpState; // don't set directly, use SetState()
	DWORD			m_confID;
	DWORD			m_partyID;
	DWORD			m_connectionID;
	DWORD			m_nCsCallIndex;
	WORD			m_wCsHandle;
	WORD			m_wCsSrcUnit;
	char			m_szConfName[MAX_CONF_NAME];
	BOOL			m_isMuted;
	
	CVendorInfo		m_rVendor;

	// audio board details for DTMF and audio messages
	CBoardDetails		m_rAudioBoard;

	DWORD	m_nMcuId;
	DWORD	m_nTerminalId;
	ERoleTokenOpcode  m_enRoleTokenLastCmd;
	
	// mrm info for scp messages
	CBoardDetails		m_rMrmBoard;
	DWORD m_nMrmConnectionId;
	DWORD m_nMrmChannelId;


	PDECLAR_MESSAGE_MAP
};


#endif // __ENDPOINT_H__ 






