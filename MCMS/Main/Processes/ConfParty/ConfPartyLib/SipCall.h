//+========================================================================+
//                            SipCall.h                                    |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SipCall.h                                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#ifndef _SIP_CALL
#define _SIP_CALL

#include "CapClass.h"
#include "IpQosDefinitions.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "ConfPartyApiDefines.h"


typedef enum
{
	kUnknown = -1,
	kDisconnected = 0,
	kConnected,
	kDisconnecting,
	kConnecting,
	kUpdating, //Updated is like connected
} EConnectionState;

typedef enum
{
	kUnknownUpdateType = -1,
	kIpAddress = 1,
	kChannelParams,
	kBoth,
	kChangePayload = 8,
	kChangeLpr = 16,
	kChangeSdes = 32,
	kChangeDtls = 64,
	kChangeMSSsrc = 128

} EUpdateType;

typedef enum
{
	kDiffPayload_NotNeeded,
	kDiffPayload_NeedToSendUpdate,
	kDiffPayload_UpdateSent
} EDiffPayloadState;

typedef enum
{
	eNoPendTrans = 0,
	etransReinvite ,
	etransBye
} EPendingTransType;


class CSipChannel;


class CSipCall: public CPObject
{
CLASS_TYPE_1(CSipCall,CPObject)
public:
	CSipCall();
	virtual ~CSipCall();
	virtual const char* NameOf() const { return "CSipCall";}
	WORD				GetConId() const {return m_mcmsConId;}
	DWORD				GetCardIndex() const {return m_cardIndex;}
	CSipHeaderList*		GetCallLegHeaders() const {return m_pCallLegHeaders;}
	CSipHeaderList*		GetCdrPrivateHeaders() const {return m_pCdrPrivateHeaders;}
	const CSipHeader*	GetCdrSpecificHeader(int strLen,const char* strHeaderName);
	int					GetCallLegHeadersLen() const;
	sipMessageHeaders*	GetRmtHeaders() {return m_pRmtHeaders;}
	int					GetCdrPrivateHeadersLen() const;
	EConnectionState	GetConnectionState() const {return m_eConnectionState;}
	BYTE				IsCallInitiator() const {return m_bIsCallInitiator? YES: NO;}
	BYTE				IsCloseInitiator() const {return m_bIsCloseInitiator? YES: NO;}
	BYTE				IsViolentClose() const {return m_bIsViolentClose? YES: NO;}
	BYTE				IsReInviteInitiator() const	{return m_bIsReInviteInitiator? YES: NO;}
	enSipCodes			GetRejectReason() const {return m_eRejectReason;}
	enSipWarningCodes	GetWarning() const {return m_eWarning;}
	const char*			GetWarningString() const {return m_warningStr;}
	ECancelType			GetCancelType() const {return m_eCancelType;}
	const char*			GetForwardAddr() const {return m_forwardAddrStr;}
	int					GetNumOfChannels() const {return m_numOfChannels;}
	int					GetNumOfMediaChannels(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	CSipChannel*		GetChannel(bool aIsExternal, cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople, DWORD aRtpConnectionId = INVALID,bool addChn=false) const;
	CSipChannel*		GetChannel(DWORD mcmsChannelIndex,DWORD cardChannelIndex) const;
	CSipChannel*		GetChannel(int arrIndex, bool aIsExternal) const;
	CSipChannel*		GetChannel(EIpChannelType eChanType) const;
    bool IsInternalChannel(CSipChannel* pChannel);
	EConnectionState	GetChannelConnectionState(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	DWORD				GetCallRate(cmCapDirection eDirection) const;
	DWORD               GetAudioRate(cmCapDirection eDirection) const;
	DWORD               GetVideoCallRate(cmCapDirection eDirection) const;
	int					CopyChannelInfoStructsToBuffer(BYTE* pStruct,int bufSize,int* pNumOfChannels,
														 int arrSize,EIpChannelType chanArr[],
														 BYTE bSetSpecificParams=YES,EConnectionState eState=kUnknown) const;
	int 				CopyCapsToCapStruct(sipMediaLinesEntrySt* pCapabilities,int structSize,const CSipCaps* pRemoteCaps,const CSipCaps* pLastRemoteCaps, cmCapDirection eDirection, BYTE bTakeSdesFromTx, BYTE m_bIsReInviteTransaction, BOOL bAddSdesIfAvailable = TRUE, EConnectionState eState=kUnknown, APIU16 m_plcmRequireMask = 0 , const CSipCaps* pChosenLocalCap = NULL) const; //_dtls_
	int					CalcCapBuffersSize(cmCapDirection eDirection,BYTE bCountMutedChannels,EConnectionState eState=kUnknown) const;

	int					CopyCapBuffersToBuffer(BYTE* buffer,int bufSize,int* pNumOfCaps,const CSipCaps* pRemoteCaps,const CSipCaps* pLastRemoteCaps,cmCapDirection eDirection, BYTE bTakeSdesFromTx, BYTE m_bIsReInviteTransaction, BOOL bAddSdesIfAvailable = TRUE, EConnectionState eState=kUnknown, APIU16 m_plcmRequireMask = 0 , const CSipCaps* pChosenLocalCap = NULL) const; //_dtls_

	mcTransportAddress	GetChannelIpAddress(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	DWORD				GetChannelRate(cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	void 				SetChannelRate(DWORD newRate, cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole = kRolePeople);
//	BYTE				AreAllChannelsStreamOn(BYTE bCheckMutedChannels) const;
//	BYTE				AreMediaChannelStreamOn(cmCapDataType eMediaType,cmCapDirection eDirection,BYTE bCheckMutedChannels, ERoleLabel eRole) const;
	BYTE				AreAllOpenChannelsConnectionState(EConnectionState eState,BYTE bIsMrcCall,BYTE confType) const;
	BYTE				AreAllChannelsSameDirection(int arrSize, EIpChannelType chanArr[], cmCapDirection eDirection) const;
	BYTE				IsAtLeastOneChannelConnectionState(EConnectionState eState) const;
	BYTE				IsMedia(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	BYTE				IsChannelMuted(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	void				SetConId(WORD mcmsConId) {m_mcmsConId = mcmsConId;}
	void                SetCardIndex(DWORD cardCallIndex) {m_cardIndex = cardCallIndex;}
	void				SetCallLegAndCdrHeaders(const sipMessageHeaders& headers);
	void 				SetRemoteHeaders(const sipSdpAndHeadersSt* pSdpAndHeaders);
	void                SetConnectionState(EConnectionState eNewState) {m_eConnectionState = eNewState;}
	void				SetCallInitiator(BYTE bInitiator)  {m_bIsCallInitiator = bInitiator? YES:NO;}
	void				SetCloseInitiator(BYTE bInitiator) {m_bIsCloseInitiator = bInitiator? YES:NO;}
	void				SetViolentClose(BYTE bYes) {m_bIsViolentClose = bYes? YES:NO;}
	void				SetReInviteInitiator(BYTE bInitiator) {m_bIsReInviteInitiator = bInitiator?YES:NO;}
	void				SetRejectReason(enSipCodes eReason);
	void				SetWarning(enSipWarningCodes eWarning) {m_eWarning = eWarning;}
	void				SetWarningString(const char* warningStr);
	void				SetCancelType(ECancelType eCancelType) {m_eCancelType = eCancelType;}
	void				SetForwardAddr(const char* forwardAddrStr);
	void				SetNumOfChannels(const int num) {m_numOfChannels=num;}
	int					SetChannelsConnectingState(EConnectionState eNewState,int arrSize,EIpChannelType chanArr[], EConnectionState eState = kUnknown);
	CSipChannel*        InsertNewChannel(cmCapDataType eMediaType,cmCapDirection eDirection,ERoleLabel eRole,CapEnum eAlgorithm,CSdesCap *pSdesCap, CDtlsCap *pDtlsCap , const BYTE data[],int length, const std::list <StreamDesc>* pStreams = NULL); //_dtls_
	void				RemoveChannel(CSipChannel* pChannel);
	void				RemoveChannel(DWORD mcmsChannelIndex,DWORD cardChannelIndex);
//	void				RemoveChannelInAState(EConnectionState eState);
	void				SetChannels(CSipComMode* pComMode,cmCapDirection eDirection);
	int					SetUpdatingChannels(CSipComMode* pComMode,int arrSize,EIpChannelType chanArr[], EUpdateType eUpdate);
	void				SetMediaPayloadType(cmCapDirection eDirToSet,  const CSipCaps* pLocalCaps, int arrSize, EIpChannelType chanArr[], const CSipCaps* pRemoteCaps = NULL);
	void                SetMediaPayloadTypeAccordingToNewPayload(cmCapDirection eDirToSet,EIpChannelType chanArr[],int arrSize,WORD payload);
	void				SetMediaRtpAddress(const sipSdpAndHeadersSt& sdpAndHeaders,cmCapDirection eDirection, BYTE confIsEncrypt);
	void				SetMediaRtpAddress(const mcTransportAddress& ipAddress, cmCapDataType eType, cmCapDirection eDirection, ERoleLabel eRole = kRolePeople);
	void 				SetMediaRtcpPort(const sipSdpAndHeadersSt& sdpAndHeaders, cmCapDirection eDirection, BYTE confIsEncrypt);
	void 				SetMediaRtcpPort(const DWORD rtcpPort, cmCapDataType eType, cmCapDirection eDirection, ERoleLabel eRole);
	void				SetMediaTos(int value,int rtcpValue,cmCapDataType eType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople);
//	void				CloseAllChannels();

	void				MuteChannels(BYTE bIsMute,int arrSize,EIpChannelType chanArr[]);
	void				SetCallIndex(DWORD callIndex);
	DWORD				GetCallIndex ();
	// Media IP mismatch
	mcTransportAddress* GetDnsProxyIpAddress(WORD place);
	void 				SetDnsProxyIpAddressArray(ipAddressStruct* dnsProxyAddrArr);
	void 				UpdateSdesChannels(CSipCaps* localCaps, BYTE bTakeSdesFromTx);
	void 				SetIceData(CSipCaps* LocalCaps);
	void 				ClearIceData();

	void                SetLprCapStruct(lprCapCallStruct* lprStruct, BYTE direction);
	lprCapCallStruct*   GetLprCapStruct(BYTE direction);

	// LPR
	lprCapCallStruct	m_lprCapStruct[NUM_OF_LPR_CAPS];			// m_lprCapStruct[0] - Local | m_lprCapStruct[1] - Remote

	// TIP
	void 				SetIsTipCall(BYTE bTipCall);
	BYTE 				GetIsTipCall() const;

    void  SetConfMediaType(eConfMediaType aConfMediaType) ;//{m_confMediaType = aConfMediaType;}
    eConfMediaType GetConfMediaType() const {return m_confMediaType;}

	int	FindNextAvailableIndexInChannelArrEx() const;
	void SetChannelsEx(CSipComMode* pComMode, cmCapDirection eDirection,BYTE bIsMrcCall);
	CSipChannel* InsertNewChannelEx(cmCapDataType eMediaType,cmCapDirection eDirection,ERoleLabel eRole,CapEnum eAlgorithm,CSdesCap *pSdesCap, const BYTE data[],int length, const std::list <StreamDesc>& rStreams, APIU32 aSsrcId = INVALID);
	int					GetNumOfChannelsEx() const {return m_numOfChannelsEx;}
	void SetMediaPayloadTypeEx(cmCapDirection eDirToSet, const CSipCaps* pLocalCaps, int arrSize, EIpChannelType chanArr[], const CSipCaps* pRemoteCaps=NULL);
	CSipChannel* AddChannelInternal(CSipComMode* pComMode, cmCapDataType aMediaType, cmCapDirection aDirection, CapEnum aCapCode, APIU32 aSsrcId = INVALID);
	// DTLS
	BYTE AreAllDtlsChannelsConnectionState(EConnectionState eState) const;
	BYTE AreAllDtlsChannelsNotInConnectionState(EConnectionState eState) const;
	BYTE AreAllDtlsChannelsConnected() const;
	BYTE IsOnlyOneDtlsChannelDisconnectionState(EConnectionState eState) const;
	DWORD  GetMediaDetectionTimer() const {return m_stMediaDetectInfo.detectTimeLen;}
	void  SetMediaDetectionTimer(DWORD timeLen) {m_stMediaDetectInfo.detectTimeLen = timeLen;}
	int	GetMediaDetectionHasVideo()const {return m_stMediaDetectInfo.hasVideo;}
	void  SetMediaDetectioHasVideo(int iHasVideo = TRUE) {m_stMediaDetectInfo.hasVideo = iHasVideo;}
	int	GetMediaDetectionIsSvcOpened()const {return m_stMediaDetectInfo.isSVCOpened;}
	void  SetMediaDetectioIsSvcOpened(int isSVC = TRUE) {m_stMediaDetectInfo.isSVCOpened= isSVC;}
	BYTE	HandleMediaDetectionInd(kChanneltype  ChannelType=kEmptyChnlType, BYTE  isRTP=0);

	// Exposing member, as cell removal functionality has been moved into it
	//========================================================================
	stMediaDetectionInfo& MediaDetectionInfo() {return m_stMediaDetectInfo;}

protected:
	int					FindNextAvailableIndexInChannelArr() const;
	void				FindAndSetSameSassionChannel(CSipChannel* pChannel);

	WORD				m_mcmsConId;
	DWORD				m_cardIndex;
	sipMessageHeaders*	m_pRmtHeaders;
	CSipHeaderList*		m_pCallLegHeaders;
	CSipHeaderList*		m_pCdrPrivateHeaders;
	EConnectionState	m_eConnectionState;
	BYTE				m_bIsCallInitiator;
	BYTE				m_bIsCloseInitiator;
	BYTE				m_bIsViolentClose;
	BYTE				m_bIsReInviteInitiator;
	enSipCodes			m_eRejectReason;
	enSipWarningCodes	m_eWarning;
	char*               m_warningStr;
	ECancelType			m_eCancelType;
	char*   			m_forwardAddrStr;
	int					m_numOfChannels;
	CSipChannel*		m_channelsPtrArr[MAX_SIP_CHANNELS];
	CSipChannel*		m_channelsPtrArrEx[MAX_INTERNAL_CHANNELS]; /* for  mixed mode    */
	int					m_numOfChannelsEx;
	DWORD				m_callIndex;								// Identify with the PartyManager
	// Media IP mismatch
	mcTransportAddress  m_DnsProxyIpAddressArray[TOTAL_NUM_OF_IP_ADDRESSES];
	mcXmlTransportAddress m_dummyMediaIp;
	BYTE				m_bIsBfcpSupported;

	// TIP
	BYTE				m_bIsTipCall;
    eConfMediaType      m_confMediaType;

	//Sip disconnect detection
	stMediaDetectionInfo	m_stMediaDetectInfo;
};


class CSipChannel: public CPObject
{
CLASS_TYPE_1(CSipChannel,CPObject)
public:
	CSipChannel();
	virtual ~CSipChannel();
	friend class CSipCall;
	virtual const char* NameOf() const { return "CSipChannel";}
	CSipCall*			GetCallPtr() const {return m_pCall;}
	int					GetArrIndexInCall() const {return m_arrIndexInCall;}
	DWORD				GetMcmsIndex() const {return m_mcmsIndex;}
	DWORD				GetCardIndex() const {return m_cardIndex;}
	DWORD				GetCardEntry() const {return m_cardEntry;}
    DWORD               GetRtpConnectionId() const {return m_rtpConnectionId;}
	cmCapDataType		GetMediaType() const {return m_eMediaType;}
	cmCapDirection		GetDirection() const {return m_eDirection;}
	ERoleLabel			GetRoleLabel() const {return m_eRole;}
	EConnectionState	GetConnectionState() const {return m_eConnectionState;}
	EConnectionState	GetRtpConnectionState() const {return m_eRtpConnectionState;}
	EConnectionState	GetCmConnectionState() const {return m_eCmConnectionState;}
	EConnectionState	GetDtlsConnectionState() const {return m_eDtlsConnectionState;}
//	EConnectionState	GetStreamState() const {return m_eStreamState;}
	BYTE				IsMuted() const {return m_bIsMuted;}
	BYTE				IsDirection(cmCapDirection eDirection)  const {return(m_eDirection == eDirection);}
	BOOL				IsMediaChannel() const{return (m_eMediaType != cmCapBfcp);}
	CapEnum				GetAlgorithm() const {return m_eAlgorithm;}
	mcTransportAddress	GetAddress() const {return m_address;}
	DWORD				GetCurrentRate() const {return m_currentRate;}
	int					GetTos(int index) const {return m_tosValue[index];}
	BYTE*				GetData() const {return m_pData;}
	int					GetDataLength() const {return m_dataLength;}
	CBaseCap*			GetDataAsCapClass() const; //alloc memory
	CSipChannel*		GetSameSessionChannel() const {return m_pSameSessionChannel;}
	BYTE				IsChannelType(EIpChannelType eChanType) const;
	void				SetCallPtr(CSipCall* pCall) {m_pCall = pCall;}
	void				SetArrIndexInCall(int i) {m_arrIndexInCall = i;}
	void				SetMcmsIndex(DWORD index) {m_mcmsIndex = index;}
	void				SetCardIndex(DWORD index) {m_cardIndex = index;}
	void				SetCardEntry(DWORD entry) {m_cardEntry = entry;}
    void                SetRtpConnectionId(DWORD connectionId) {m_rtpConnectionId = connectionId;}
	void				SetMediaType(cmCapDataType eMediaType) {m_eMediaType = eMediaType;}
	void				SetDirection(cmCapDirection eDirection) {m_eDirection = eDirection;}
	void				SetRoleLabel(ERoleLabel eRole) {m_eRole = eRole;}
	void				SetConnectionState(EConnectionState eNewState);// {m_eConnectionState = eNewState;}
	void				SetRtpConnectionState(EConnectionState eNewState) {m_eRtpConnectionState = eNewState;}
	void				SetCmConnectionState(EConnectionState eNewState);// {m_eCmConnectionState = eNewState;}
	void 				SetDtlsConnectionState(EConnectionState eNewState) {m_eDtlsConnectionState = eNewState;}
//	void				SetStreamState(EConnectionState eNewState) {m_eStreamState = eNewState;}
	void				Mute(BYTE bOnOff) {m_bIsMuted = (bOnOff? YES: NO);}
	void				SetAlgorithm(CapEnum eAlgorithm) {m_eAlgorithm = eAlgorithm;}
	void				SetAddress(const mcTransportAddress& ipAddress) { memcpy(&m_address,&ipAddress,sizeof(mcTransportAddress));}
	void				SetCurrentRate(DWORD currentRate) {m_currentRate = currentRate;}
	void				SetTos(int index, int value) {m_tosValue[index] = value;}
	void				SetData(const BYTE data[], int length);
	void				SetSameSessionChannel(CSipChannel* pSameSessionChannel);
	void				SetChannelQualities(CapEnum eAlgorithm,const BYTE data[],int length);

	DWORD 				GetPayloadType() const {return (DWORD) m_payloadType;}
	void 				SetPayloadType(DWORD payloadType); //{m_payloadType = payloadType;}
	BYTE				IsChannelDynamicPayloadType();
	virtual void  		SetDtmfPayloadType(APIU8 payload) {}
	virtual APIU8 		GetDtmfPayloadType() const {return _UnKnown;}
	void                SetSeqNumRtp(DWORD seqNum){m_SeqNumRtp = seqNum;};
	DWORD               GetSeqNumRtp(){return m_SeqNumRtp;};
	void                SetSeqNumCm(DWORD seqNum){m_SeqNumCm = seqNum;};
	DWORD               GetSeqNumCm(){return m_SeqNumCm;};
    void                SetSeqNumMrmp(DWORD seqNum) {m_SeqNumMrmp = seqNum;};
    DWORD               GetSeqNumMrmp() {return m_SeqNumMrmp;};
	void                SetRtcpPort(APIU32 rtcpPort){m_rtcpPort = rtcpPort;};
	APIU32              GetRtcpPort(){return m_rtcpPort;};
	void                SetRtcpRmtPort(APIU32 rtcpPort){m_rtcpRmtPort = rtcpPort;};
	APIU32              GetRtcpRmtPort(){return m_rtcpRmtPort;};
	void 		    SetRmtAddress(mcTransportAddress pRmtTrAddr){ memcpy(&m_RmtAddress,&pRmtTrAddr,sizeof(mcTransportAddress));};
	mcTransportAddress* GetRmtAddress() { return &m_RmtAddress;};
	EDiffPayloadState   GetDiffPayloadState(){return m_eDiffPayloadState;};
	void                SetDiffPayloadState(EDiffPayloadState eDiffPayloadState){m_eDiffPayloadState = eDiffPayloadState;}
	BYTE                GetIsSupportLpr(){return m_isSupportLpr;};
	void                SetIsSupportLpr(BYTE bSupportLpr){m_isSupportLpr = bSupportLpr;};
	void 				SetChannelSdes(CSdesCap *pSdesCap);
	CSdesCap*			GetChannelSdes();
	APIU32				GetSdesCapLen();
	BYTE				IsChannelSdesEnabled();
	void 				RemoveChannelSdes();

	//LYNC2013_FEC_RED:
	void                SetFecPayloadType(APIU32 payloadTypeFec){m_payloadTypeFec = payloadTypeFec;};
	void                SetRedPayloadType(APIU32 payloadTypeRed){m_payloadTypeRed = payloadTypeRed;};
	APIU32              GetFecPayloadType() {return m_payloadTypeFec;}
	APIU32              GetRedPayloadType() {return m_payloadTypeRed;}

	void				SetIceData(CSipCaps* Caps);
	WORD 				GetNumOfIceCaps();
	capBuffer*          GetIceCapBuffer(WORD index);
	void				CleanIceCaps();
    void                Dump(char * header);
    void                ShortDump(char *header);

    // TIP
    DWORD 				GetTipBitRate() const;
    void 				SetTipBitRate(DWORD newRate);
	void				SetChannelHandle(APIU32      channelHandle) {m_channelHandle = channelHandle;}
	APIU32				GetChannelHandle() {return  m_channelHandle;}

	const std::list <StreamDesc>& GetStreams() const {return m_streams;}
    void                SetStreamsList(const std::list <StreamDesc>&  rStreams, APIU32 aSsrcId = INVALID);

	// DTLS
	void 				SetChannelDtls(CDtlsCap *pDtlsCap);
	APIU32 				GetDtlsCapLen();
	CDtlsCap* 			GetChannelDtls();
	BYTE 				IsChannelDtlsEnabled();
	void 				RemoveChannelDtls();


protected:
	CSipCall*			m_pCall;				// call associated with this channel
	int					m_arrIndexInCall;
	DWORD				m_mcmsIndex;
	DWORD				m_cardIndex;
	DWORD				m_cardEntry;
	cmCapDataType		m_eMediaType;
	cmCapDirection		m_eDirection;
	ERoleLabel			m_eRole;
	EConnectionState	m_eConnectionState;
	EConnectionState	m_eRtpConnectionState;
	EConnectionState	m_eCmConnectionState;
	EConnectionState	m_eDtlsConnectionState;
//	EConnectionState	m_eStreamState;
	BYTE				m_bIsMuted;
	CapEnum				m_eAlgorithm;
	mcTransportAddress  m_address;
	APIU32				m_rtcpPort;
	APIU32				m_rtcpRmtPort;
	DWORD				m_currentRate;		// video by 100 bits per sec, audio by K bits per sec.
	int					m_tosValue[NumberOfTosValues];
	BYTE*				m_pData;
	int					m_dataLength;
	APIU8				m_payloadType;	// number above 90 or NA if algorithm is not dynamic
	CSipChannel*		m_pSameSessionChannel;	// channel with the same type on opposite direction
	DWORD               m_SeqNumRtp;
	DWORD               m_SeqNumCm;
    DWORD               m_SeqNumMrmp;
	mcTransportAddress			m_RmtAddress;
	EDiffPayloadState	m_eDiffPayloadState;
	CSdesCap*			m_pSdesData;
	CDtlsCap*			m_pDtlsData;
	WORD 				m_numOfIceCaps;
	capBuffer*  		m_IceCapList[MAX_MEDIA_CAPSETS];
	BYTE                m_isSupportLpr;

	//LYNC2013_FEC_RED:
	APIU32              m_payloadTypeFec;
	APIU32              m_payloadTypeRed;

	DWORD				m_tipBitRate;
	APIS32              m_channelHandle;  /* channel handle for soft mcu  */
	DWORD               m_rtpConnectionId;
	std::list <StreamDesc>  m_streams;
	void DumpStreamsList(std::ostream& msg);
};



class CSipAudioChannel: public CSipChannel
{
CLASS_TYPE_1(CSipAudioChannel,CSipChannel)
public:
	CSipAudioChannel();
	virtual ~CSipAudioChannel(){}
	friend class CSipCall;
	virtual const char* NameOf() const { return "CSipAudioChannel";}

	virtual void  SetDtmfPayloadType(APIU8 payload) {m_dtmfPayloadType = payload;}
	virtual APIU8 GetDtmfPayloadType() const {return m_dtmfPayloadType;}

protected:

	APIU8 m_dtmfPayloadType;	// (dynamic) number above 90 or NA if algorithm is not dynamic
};




class CSipChanDif: public CPObject
{
CLASS_TYPE_1(CSipChanDif,CPObject)
public:
	CSipChanDif();
	virtual ~CSipChanDif(){}
	virtual const char* NameOf() const { return "CSipChanDif";}

	virtual void Serialize(WORD format,CSegment& seg) const;
	virtual void DeSerialize(WORD format,CSegment& seg);
	virtual void DumpToString(CObjString& string) const;

	EIpChannelType	GetChannelType()	const;
	WORD			GetNumOfChanges()	const;
	BYTE			IsChange(BYTE (CSipChanDif::*isFunction) (void) const) const;
	BYTE			IsAddChannel()		const;
	BYTE			IsRemoveChannel()   const;
	BYTE			IsChangeIp()		const;
	BYTE			IsChangePort()		const;
	BYTE			IsChangeRtcpPort()	const;
	BYTE			IsMute()			const;
	BYTE			IsUnMute()			const;
	BYTE			IsChangeAlg()		const;
	BYTE			IsChangePayload()	const;
	BYTE			IsChangeAudFpp()	const;
	BYTE            IsChangeLpr()       const ;
	BYTE			IsChangeMSSsrc()	const;
	BYTE			IsChangeSdes()		const;
	BYTE			IsChangeQoS()		const;
	BYTE			IsChangeBfcpTransportProtocol()		const;
//	BYTE			IsChangeDtls()		const;

	// add here more functions

	void			SetDetails(WORD details);
	void			SetChannelType(EIpChannelType t);
	void			SetAddChannel(BYTE yesno);
	void			SetRemoveChannel(BYTE yesno);
	void			SetChangeIp(BYTE yesno);
	void			SetChangePort(BYTE yesno);
	void 			SetChangeRtcpPort(BYTE yesno);
	void			SetMute(BYTE yesno);
	void			SetUnMute(BYTE yesno);
	void			SetChangeAlg(BYTE yesno);
	void			SetChangePayload(BYTE yesno);
	void			SetChangeAudFpp(BYTE yesno);
	void            SetChangeLpr(BYTE yesno);
	void			SetChangeMSSsrc(BYTE yesno);
	// add here more functions
	void			SetChangeSdes(BYTE yesno);
	void			SetChangeQoS(BYTE yesno);
	void			SetChangeBfcpTransportProtocol(BYTE yesno);
//	void 			SetChangeDtls(BYTE yesno);


protected:

	EIpChannelType	m_eType;
	WORD			m_nNumOfChanges;
	BYTE			m_bAdd;					//YES/NO - nonzero port
	BYTE			m_bRemove;				//YES/NO - zero port
	BYTE			m_bChangeIp;			//YES/NO
	BYTE			m_bChangePort;			//YES/NO
	BYTE			m_bChangeRtcpPort;		//YES/NO
	BYTE			m_bMute;				//YES/NO
	BYTE			m_bUnMute;				//YES/NO
	BYTE			m_bChangeAlg;			//YES/NO
	BYTE			m_bChangePayload;		//YES/NO - for dynamic payload

	// for audio channel only
	BYTE			m_bChangeAudFpp;		//YES/NO

	// for video channel only
	BYTE			m_bChangeVidRate;		//YES/NO
	BYTE			m_bChangeVidResolution;	//YES/NO
	BYTE			m_bChangeVidFps;		//YES/NO
	BYTE			m_bAddVidAnnex;			//YES/NO
	BYTE			m_bRemoveVidAnnex;		//YES/NO
	BYTE            m_bChangeLpr;           //YES/NO
	BYTE            m_bChangeMSSsrc;        //YES/NO
	// for SDES
	BYTE			m_bChangeSdes;			//YES/NO
//	BYTE			m_bChangeDtls;

	//for BFCP
	BYTE			m_bChangeBfcpTransportProtocol; //YES/NO

	// For guess what?
	BYTE			m_bChangeQoS;			//YES/NO
};


class CSipChanDifArr: public CPObject
{
	CLASS_TYPE_1(CSipChanDifArr,CPObject)
public:
	CSipChanDifArr();
	virtual ~CSipChanDifArr(){}
	virtual const char* NameOf() const { return "CSipChanDifArr";}

	virtual void Serialize(WORD format,CSegment& seg) const;
	virtual void DeSerialize(WORD format,CSegment& seg);
	virtual void DumpToString(CObjString& string) const;
	CSipChanDif* operator[] (int i);
	CSipChanDif* GetChanDif(EIpChannelType type);
	CSipChanDif* GetChanDif(cmCapDataType eMediaType,cmCapDirection eDirection,ERoleLabel eRole = kRolePeople);

	BYTE IsChange(BYTE (CSipChanDif::*isFunction) (void) const,int arrSize,EIpChannelType chanArr[]);
	int  SetArrOfChanges(BYTE (CSipChanDif::*isFunction) (void) const,int& arrSize,EIpChannelType chanArr[]);

protected:
	CSipChanDif m_arr[MAX_SIP_CHANNELS];
};


#define MASK_FOR_VSW_AVC_PIPEID 0x000FFFFF

const char* ConnectionStateToString(EConnectionState connectionState);

#endif //_SIP_CALL

