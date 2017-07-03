//+========================================================================+
//                            CallAndChannelInfo.h	                       |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       CallAndChannelInfo.h	                                       |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: Guy D,													   |
// Date : 16/6/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        
/*

This header file contains information that is proprietary so called MCMS processes withing CARMEL product.
In this document we set the header files convertions of variables type definition.

*/

#ifndef __CONFANDCHANNELINFO_H__
#define __CONFANDCHANNELINFO_H__

#include "PObject.h"
#include "Capabilities.h"
#include "IpRtpFeccRoleToken.h"
#include "IpCsContentRoleToken.h"
#include "ChannelParams.h"
#include "H323CsInd.h"
#include "CapInfo.h"
#include "ConfPartyDefines.h"
#include "CapClass.h"

class CChannel;
//For the streamOn - since the streamOn send directly to the timer we need to ive him this value.
#define		InvalidChannelEntry		0xFFFFFFFF 

// The channel status
typedef enum {
	kDisconnectedState,
	kFirstConnectingState,
	kConnectingState,
	kBeforeResponseReq, // relevant only for incoming channel
	kBeforeConnectedInd,
	kLastConnectingState,
	kConnectedState,
	kFirstDisconnectingState,
	kDisconnectingState,
	kWaitToSendChannelDrop,
	kCheckSendCallDrop,  // After channel will be closed, check sending call_drop.
	kNoNeedToDisconnect, //In case we send open_udp and after that the call has start to disconnected, we will reject the channel and send close_udp.
		//When we will get the close_udp_ack, we need to do nothing regarding the cs, because we have rejected the channel towards cs.
		//Therefore, we won't get start_close_channel from cs on this channel.
		//Need to notice that in usual reject channel we remove the channel from the array. But in this case we don't do it,
		//because the cm has to answer on close_udp_req. Therfore, we assign a specific state for this channel. 
	kLastDisconnectingState
} ECsChannelState; 

typedef enum {
	kNotSendOpenYet,
	kSendOpen,
	kRecieveOpenAck,
	kNeedsToBeClosed,
	kSendClose,
	kRecieveCloseAck,
} ECmUdpChannelState; 

typedef enum {
	kRtpPortNotSendOpenYet,
	kRtpPortOpenSent,
	kRtpPortUpdateSent,
	kRtpPortReceivedOpenAck,
	kRtpPortReceivedUpdateAck
} EArtRtpPortChannelState;

typedef enum {
	eNoLprAckReceived,
	ePeopleAckReceived,
	eContentAckReceived,
	eBothLprAckReceived
} ELprRtpAckStatus;

class CCall: public CPObject
{
	CLASS_TYPE_1(CCall, CPObject)
public:
	
    // Constructors 
	CCall();
	~CCall();
	
	// Operations
	void SetCallIndex(DWORD callIndex);
	DWORD GetCallIndex ();
	virtual const char* NameOf() const {return "CCall";}
	void SetCallStatus(int status);
	int GetCallStatus();

	void SetConnectionId(WORD conId);
	WORD GetConnectionId();

	void IncreaseChannelsCounter();
	void DecreaseChannelsCounter();
	DWORD GetChannelsCounter();
	void  SetChannelsCounter(DWORD counter);

	CChannel*	GetSpecificChannel(DWORD index, bool aIsExternal = true) const;
	int	        SetCurrentChannel(APIU32 csIndex, APIU32 mcmsIndex, CChannel **pChannel,APIS8 *pIndex) ;																			
	CChannel*	FindChannelInList(DWORD sameSessionCahhnelIndex) const;
	CChannel*	FindChannelInList(cmCapDataType eType, BYTE bIsTransmiting, ERoleLabel eRole = kRolePeople, bool aIsExternal = true) const;
	DWORD       GetChannelIndexInList(CChannel *pChannel, bool aIsExternal) const;
	DWORD       GetChannelIndexInList(bool aIsExternal, cmCapDataType eType, BYTE bIsTransmiting, ERoleLabel eRole = kRolePeople) const;
	int			UpdateMcChannelParams1(BOOL bIsTransmit, CChannel *&pMcChannel, CCapSetInfo capInfo,ERoleLabel eRole,
								DWORD rate, mcIndIncomingChannel *pinChnlInd = NULL, EenMediaType encType = kUnKnownMediaType, APIS32 status = 0);
	void 		RemoveChannel(CChannel *pChannel);
	void		RemoveChannel(DWORD arrayIndex);	

	void UpdateCallSignalAddress(ipAddressIf* ipAddress, APIU32 port, mcTransportAddress* pCallSignalAddressToSet, BYTE isIpV6);

	void UpdateSourceCallSignalAddressPort(APIU32 port);

	
	mcTerminalParams GetSrcTerminalParams();
	void SetSrcTerminalParams(mcTerminalParams srcTerParams);
	void SetSrcTerminalCallSignalAddress(const mcTransportAddress& callSignalAddress);

	mcTerminalParams GetDestTerminalParams();
	void SetDestTerminalParams(mcTerminalParams destTerParams);
	void SetDestTerminalCallSignalAddress(const mcTransportAddress& callSignalAddress);

	char* GetSourceInfoAlias();
	void  SetSourceInfoAlias(const char* sourceInfo);

	DWORD* GetSrcInfoAliasType();
	void   SetSrcInfoAliasType(DWORD* srcInfoAliasesType);

	char* GetDestinationInfoAlias();
	void  SetDestinationInfoAlias(char* destinationInfo);

	DWORD* GetDestInfoAliasType();
	void   SetDestInfoAliasType(DWORD* destInfoAliasesType);

	BOOL  GetCanMapAlias();
	void  SetCanMapAlias(BOOL canMapAlias);

	char* GetCallId();
	void  SetCallId(const char* callId);

	char* GetConferenceId();
	void  SetConferenceId(const char* conferenceId);

	int   GetReferenceValueForEp();
	void  SetReferenceValueForEp(int RefValForEp);

	int   GetReferenceValueForGk();
	void  SetReferenceValueForGk(int RefValForGk);

	cmCallType GetCallType();
	void  SetCallType(cmCallType callType);

	cmCallModelType GetCallModelType();
	void  SetCallModelType(cmCallModelType callModelType);

	BOOL  GetIsActiveMc();
	void  SetIsActiveMc(BOOL isActiveMc);

	BOOL  GetIsOrigin();
	void  SetIsOrigin(BOOL isOrigin);
	
	BOOL  GetIsClosingProcess();
	void  SetIsClosingProcess(BOOL isClosingProcess);

	DWORD GetCallCloseInitiator();
	void  SetCallCloseInitiator(DWORD callCloseInitiator);

	mcTransportAddress* GetSetupH245Address();
	void SetSetupH245Address(mcTransportAddress setUpH245Addr);

	mcTransportAddress* GetAnswerH245Address();
	void SetAnswerH245Address(mcXmlTransportAddress answerH245Address);
	
	mcTransportAddress* GetControlH245Address();
	void SetControlH245Address(mcTransportAddress controlH245Address);

	BOOL  GetH245Establish();
	void  SetH245Establish(BOOL h245Establish);

	DWORD GetMaxRate();
	void  SetMaxRate( DWORD maxRate);

	DWORD GetMinRate();
	void  SetMinRate( DWORD minRate);
	
	DWORD GetRate();
	void  SetRate( DWORD rate);

	DWORD GetBandwidth();
	void  SetBandwidth( DWORD bandwidth);

	int   GetMasterSlaveStatus();
	void  SetMasterSlaveStatus(int masterSlaveStatus);

	mcCallTransient GetCallTransient();
	void  SetCallTransient(const mcCallTransient callTransient); 
	void SetCallTransientDisplay(const char* pDisplay);
	void SetCallTransientUserUser(const char* pUserUser);
	char* GetCallTransientUserUser();
	
	DWORD GetRmtType();
	void  SetRmtType( DWORD rmtType);

	CChannel** GetChannelsArray();

	void SetUserUserNameAndSize(APIS8* userUser, APIS32 userUserSize);
	void SetDestinationEpType(cmRASEndpointType EpType);
	void SetSourceEpType(cmRASEndpointType EpType);
	void SetControlH245AddressPort(APIU16 controlPort);

	void SetSpecificChannel(APIS32 index,CChannel* pChannel);

	void SetSrcTerminalPartyAddr(const char* srcparty, APIU32 size);
	void SetDestTerminalPartyAddr(const char* destparty, APIU32 size);
	
	int	TestUpdateMcChannelParams(BOOL bIsTransmit, CChannel *&pMcChannel, CCapSetInfo capInfo,ERoleLabel eRole,
								DWORD rate, mcIndIncomingChannel *pinChnlInd = NULL, EenMediaType encType = kUnKnownMediaType, APIS32 status = 0);
	WORD GetNumOfSrcAliases () { return m_numOfSrcAliases;};
    WORD GetNumOfDestAliases () { return m_numOfDestAliases;};
    
    void SetLprCapStruct(lprCapCallStruct* lprStruct, BYTE direction);
    lprCapCallStruct* GetLprCapStruct(BYTE direction);

    CChannel* AddChannelInternal(CComModeH323* pComMode, cmCapDataType aMediaType, cmCapDirection aDirection, CapEnum aCapCode, DWORD rate, APIU32 aSsrcId = INVALID);
    int       GetNumOfInternalChannels() const {return m_numOfInternalChannels;}
    BYTE      AreAllInternalChannelsConnected();
    int FindNextAvailableIndexInInternalChannelArr() const;
    CChannel* GetInternalChannel(cmCapDataType eMediaType, BOOL aIsTransmitDirection, ERoleLabel eRole, DWORD aRtpConnectionId = INVALID) const;
    CChannel* GetInternalChannelBySsrc(cmCapDataType eMediaType, BOOL eDirection, ERoleLabel eRole, DWORD aSsrcId = INVALID) const;
    void RemoveChannelInternal(DWORD arrayIndex);
    void RemoveChannelInternal(CChannel *pChannel);

    //===========================================
    // Media detection logic, imported from SIP
    //===========================================
	DWORD	GetMediaDetectionTimer() const 					{return m_stMediaDetectInfo.detectTimeLen;}
	void	SetMediaDetectionTimer(DWORD timeLen)			{m_stMediaDetectInfo.detectTimeLen = timeLen;}
	int		GetMediaDetectionHasVideo()const				{return m_stMediaDetectInfo.hasVideo;}
	void	SetMediaDetectioHasVideo(int iHasVideo = TRUE)	{m_stMediaDetectInfo.hasVideo = iHasVideo;}

	BYTE	HandleMediaDetectionInd(kChanneltype  ChannelType=kEmptyChnlType, BYTE  isRTP=0);


	// Exposing member, as cell removal functionality has been moved into it
	//========================================================================
	stMediaDetectionInfo& MediaDetectionInfo() {return m_stMediaDetectInfo;}

protected:
	void SetCallSignalAddress(const mcTransportAddress& callSignalAddress, mcTransportAddress* pCallSignalAddressToSet);

private:
	DWORD						m_callIndex;								// Identify with the PartyManager
	int							m_status;					
	WORD						m_mcmsConnId;								// Call ID as known to MCMS  // ????
	DWORD						m_channelsCounter;						// open channels number
	CChannel*					m_pChannelsArray[MaxChannelsPerCall];		// array of channels established for this call
    CChannel*                   m_pInternalChannelsArray[MAX_INTERNAL_CHANNELS];       // array of internal channels established for this call
    int                         m_numOfInternalChannels;
	mcTerminalParams			m_srcTerminal;    
	mcTerminalParams			m_destTerminal;
	char						m_sourceInfo[MaxAddressListSize];			// Aliases for source
	DWORD						m_srcInfoAliasesType[MaxNumberOfAliases];	// enum - cmAliasType
	char						m_destinationInfo[MaxAddressListSize];	// Aliases for destination
	DWORD						m_destInfoAliasesType[MaxNumberOfAliases];// enum - cmAliasType
//	DWORD						gatekeeperIndex;						// Index of gatekeeper in GkManager table	
	BOOL						m_bCanMapAlias;
	char						m_callId[Size16];
	char						m_conferenceId[MaxConferenceIdSize];		// Conference Id														
//	cmConferenceGoalType		conferenceGoal;							// Conference goal:Create,Join,Invite	
	int							m_referenceValueForEp;					// CRV for mcu and EP.
	int							m_referenceValueForGk;					// CRV for mcu and GK.
	cmCallType					m_callType;									// Call type: P2P, One2N, N2N, etc. 
 	cmCallModelType				m_model;									// Call model: Direct, Gatekeeper routed
	BOOL						m_bIsActiveMc;							// Is active MC in this call 
	BOOL						m_bIsOrigin;								// Is call originated by this endpoint
	BOOL						m_bIsClosingProcess;						// Call's channels in closing process.
	initiatorOfClose			m_callCloseInitiator;						// Mc or Pm or Gk
	mcTransportAddress			m_setupH245Address;						// H.245 address as selected by sender 
	mcTransportAddress			m_answerH245Address;						// H.245 address as selected by receiver 
	mcTransportAddress			m_controlH245Address;						// H.245 address as selected for control	
	BOOL						m_bH245Establish; 										   
	DWORD						m_maxRate;								// Maximal rate of call
	DWORD						m_minRate;								// Minimal rate of call
	DWORD						m_rate;									// Rate of the call in bit/sec
	DWORD						m_bandwidth;								// Amount of all video&audio channels rate
	int							m_masterSlaveStatus;						// master,slave or error
	mcCallTransient				m_callTransient;
	cmEndpointType				m_rmtType;
	APIU8						m_dynamicPayloadTypeIndex;

    WORD m_numOfSrcAliases;
    WORD m_numOfDestAliases;
    
    // LPR
    lprCapCallStruct				m_lprCapStruct[NUM_OF_LPR_CAPS];			// m_lprCapStruct[0] - Local | m_lprCapStruct[1] - Remote
    
    // Media disconnection detection, imported from SIP
	stMediaDetectionInfo	m_stMediaDetectInfo;

};


class CChannel: public CPObject
{
	CLASS_TYPE_1(CChannel, CPObject)
public:
    // Constructors 
	CChannel();
	~CChannel();

	// Operations
	virtual const char* NameOf() const {return "CChannel";}

	void UpdateChannelParams(CCall *pCall,int channelArrayIndex, BOOL bIsOutgoingChannel, CCapSetInfo capInfo,ERoleLabel eRole,
					   DWORD rate, mcIndIncomingChannel *pinChnlInd, EenMediaType encMediaType, APIU32 incomingChnlStatus);


	DWORD		GetIndex();
	void		SetIndex(DWORD index);

	DWORD		GetCsIndex();
	void		SetCsIndex(DWORD csIndex);

	int			GetStatus();
	void		SetStatus(int status);

	BOOL		IsOutgoingDirection();
	cmCapDirection GetChannelDirection();
	void		SetChannelDirection(BOOL channelDirection);
	
	BOOL		GetIsActive();
	void		SetIsActive(BOOL isActive);

	DWORD		GetChannelCloseInitiator();
	void		SetChannelCloseInitiator(DWORD channelCloseInitiator);

	DWORD		GetPayloadType();
	void		SetPayloadType(DWORD payloadType);

	CapEnum		GetCapNameEnum();
	void		SetCapNameEnum(CapEnum capNameEnum);

	cmCapDataType GetType();
	void		SetType(cmCapDataType dataType);

	ERoleLabel	GetRoleLabel();
	void		SetRoleLabel(ERoleLabel roleLabel);

	DWORD		GetSizeOfChannelParams();
	void		SetSizeOfChannelParams(DWORD sizeOfChannelParams);

	char*		GetChannelParams();
	void		SetChannelParams(DWORD size,char *pChannelParams);

	DWORD		GetRate();
	void		SetRate(DWORD rate);

	DWORD		GetMaxSkew();
	void		SetMaxSkew(DWORD maxSkew);

	BYTE            IsChannelConnected();
	
	BYTE            IsChannelCsConnected();
	ECsChannelState GetCsChannelState();
	void		    SetCsChannelState(ECsChannelState eCsChannelState);
	BOOL			IsCsChannelStateConnecting();
	BOOL			IsCsChannelStateDisconnecting();

	EStreamState	GetStreamState();
	void			SetStreamState(EStreamState streamState);

	BOOL		GetIsRejectChannel();
	void		SetIsRejectChannel(BOOL isRejectChannel);

	BOOL		GetIsStreamOnSent();
	void		SetIsStreamOnSent(BOOL isStreamOnSent);

	BOOL		GetIsEncrypted();
	void		SetIsEncrypted(BOOL isEncrypted);

	EenMediaType GetEncryptionType();
	void		 SetEncryptionType(EenMediaType encType);

	APIS32		GetSessionId();
	void		SetSessionId(APIS32 sessionId);

	APIS8		GetIsDbc2();
	void		SetIsDbc2(APIS8 isDbc2);

	void		SetCallPointer(CCall *pCall);

	APIU8		GetDynamicPayloadType();
	void		SetDynamicPayloadType(APIU8 dynamicPayloadType);
	
	void		SetIsH263Plus(APIU8 bIsH263Plus);
	APIU8		IsH263Plus();
	
	BYTE            	IsChannelCmConnected();
	void				SetCmUdpChannelState(ECmUdpChannelState eCmState);
	ECmUdpChannelState  GetCmUdpChannelState();
	
	void 					SetRtpPortChannelState(EArtRtpPortChannelState eRtpPortState);
	EArtRtpPortChannelState GetRtpPortChannelState();
	
//	void        SetCsChannelDisconnectCase(ECsChannelDisconnectCase eCsChannelDisconnectCase);
//	ECsChannelDisconnectCase GetCsChannelDisconnectCase();
	
	void TestUpdateChannelParams(CCall *pCall,int channelArrayIndex, BOOL bIsOutgoingChannel, CCapSetInfo capInfo,ERoleLabel eRole,
					   DWORD rate, mcIndIncomingChannel *pinChnlInd, EenMediaType encMediaType, APIU32 incomingChnlStatus);
	
	void 		SetH235SessionKey(APIU8*	pSessionKey);
	APIU8*		GetH235SessionKey();
	
	void 		SetH235EncryptedSessionKey(APIU8*	pEncSessionKey);
	APIU8*		GetH235EncryptedSessionKey();		
	
	//IP ADDRESS
	void		SetRmtAddressVer(APIU32	ipVersion){ m_RmtAddress.ipVersion =  ipVersion;};	
	APIU32		GetRmtAddressVer(void){ return m_RmtAddress.ipVersion;};
	// IpV6
	void 		SetRmtAddress(mcTransportAddress pRmtTrAddr);
	mcTransportAddress* GetRmtAddress() { return &m_RmtAddress;};
//	void		SetRmtAddressPort(APIU32	port){ m_RmtAddress.port =  port;};	
	APIU32		GetRmtAddressPort(void){ return m_RmtAddress.port;};

	void                SetRtcpPort(APIU32 rtcpPort){m_rtcpPort = rtcpPort;};
	APIU32              GetRtcpPort(){return m_rtcpPort;};
	void                SetRtcpRmtPort(APIU32 rtcpPort){m_rtcpRmtPort = rtcpPort;};
	APIU32              GetRtcpRmtPort(){return m_rtcpRmtPort;};
	
	
	
	// LPR
	void 		SetIsLprSupported(WORD isSupportLpr);
	WORD		GetIsLprSupported();
	
	//prinitg of seq num for debug purpose
	
	DWORD GetSeqNumRtp(){return m_seqNumRtp;};
	void SetSeqNumRtp(DWORD seqNum){ m_seqNumRtp=seqNum;};
	DWORD GetSeqNumCm(){return m_seqNumCm;};
    void SetSeqNumCm(DWORD seqNum){ m_seqNumCm=seqNum;};
	
    void                SetChannelHandle(APIU32      channelHandle) {m_channelHandle = channelHandle;}
    APIU32              GetChannelHandle() {return  m_channelHandle;}

    void                SetRtpConnectionId(DWORD connectionId) {m_rtpConnectionId = connectionId;}
    DWORD               GetRtpConnectionId() const {return m_rtpConnectionId;}
    const std::list <StreamDesc>& GetStreams() const {return m_streams;}
    void                SetStreamsList(const std::list <StreamDesc>&  rStreams, APIU32 aSsrcId = INVALID);
    void DumpStreamsList(std::ostream& msg);
    void InitChannelParams(CCall *apCall, BOOL bIsOutgoingChannel, CCapSetInfo capInfo,ERoleLabel eRole,
                       DWORD rate, DWORD localCapRate, mcIndIncomingChannel *pinChnlInd, APIS32 status, APIU32 CsChannelIndex, APIU16 isLpr, EenMediaType encAlg);
    void Dump(char * header);

private:

	CCall					   *m_pCall;										// Pointer to call channel belongs to
	DWORD						m_index;										// Channel index - as known to MCMS
	DWORD						m_csIndex;									// Channel index - as known to Party Manager (CS)
	int							m_status;										// Status of channel
	BOOL						m_directionInOut;									// Is this a transmitting channel
	BOOL						m_bIsActive;									// Is this an active channel // ???????
	initiatorOfClose			m_channelCloseInitiator;						// Mc or Pm (RtpStreamOff allready sent)
	payload_en					m_payloadType;								// g7231, h261, etc
	CapEnum                     m_nameEnum;									// Algorithm type
	cmCapDataType				m_dataType;										// Audio, Video, Data, Non-Standard
	ERoleLabel                  m_roleLabel;									// people, content
	DWORD						m_sizeOfChannelParams;
	char*						m_pChannelParams;
	DWORD						m_rate;										// Rate of channel
	DWORD						m_maxSkew;									// Max skew between these channels // Is still needed ?
	ECsChannelState				m_eCsChannelState;								// State of channel
	EStreamState		        m_eStreamState;								// State of stream
	BOOL						m_bIsRejectChannel;
	BOOL						m_bIsStreamOnSent;
	BOOL						m_bIsEncrypted;
	EenMediaType				m_encryType;
	APIS32						m_sessionId;
	APIS8						m_isDbc2;
	APIU8						m_dynamicPayloadType;
	APIU8						m_bIsH263Plus;

	ECmUdpChannelState			m_eCmUdpChannelState;
//	ECsChannelDisconnectCase	m_eCsChannelDisconnectCase;
	APIU8						m_sessionKey[sizeOf128Key];
	APIU8						m_encSessionKey[sizeOf128Key];
	mcTransportAddress			m_RmtAddress;
	APIU32				m_rtcpPort;
	APIU32				m_rtcpRmtPort;
	
	EArtRtpPortChannelState		m_eRtpPortChannelState;	
	
	WORD						m_isSupportLpr;
	DWORD                       m_seqNumRtp;
	DWORD                       m_seqNumCm;

    APIS32                      m_channelHandle;  /* channel handle for soft mcu  */
    DWORD                       m_rtpConnectionId; /* connection id for avc2svc translator arts */
    std::list <StreamDesc>  m_streams;

};



#endif

