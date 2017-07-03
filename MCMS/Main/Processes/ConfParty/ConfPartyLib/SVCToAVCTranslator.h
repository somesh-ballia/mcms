//+========================================================================+
//                SVCToAVCTranslator.H                                     +
//+========================================================================+

#ifndef _CSVCToAVCTranslator_H_
#define _CSVCToAVCTranslator_H_

#include "StateMachine.h"
#include "ConfPartyDefines.h"
#include "ConfPartyOpcodes.h"
#include "MrcStructs.h"
#include "VideoDefines.h"
#include "MplMcmsStructs.h"

class CVideoHardwareInterface;
struct VideoOperationPoint;

//#define SVC_TO_AVC_FAILURE_TOUT	111
#define MAX_NONAME_NAME 16

// OpenDecoder struct
typedef struct
{
	WORD 				videoAlg;
	DWORD 				videoBitRate;
	eVideoResolution 	videoResolution;
	eVideoFrameRate 	videoQCifFrameRate;
	eVideoFrameRate 	videoCifFrameRate;
	eVideoFrameRate 	video4CifFrameRate;
	DWORD 				mbps;
	DWORD 				fs;
	DWORD 				sampleAspectRatio;
	DWORD 				staticMB;
	BYTE 				isVSW;
	DWORD 				backgroundImageID;
	BYTE 				isVideoClarityEnabled;
	eVideoConfType 		videoConfType;
	DWORD 				dpb;
	DWORD 				parsingMode;
	eTelePresencePartyType eTelePresenceMode;
	BOOL 				isAutoBrightness;
	eVideoFrameRate 	videoVGAFrameRate;
	eVideoFrameRate 	videoSVGAFrameRate;
	eVideoFrameRate 	videoXGAFrameRate;
	EVideoDecoderType 	decoderType;
	eVideoFrameRate 	resolutionFrameRate;
	eVideoProfile 		profile;
	eVideoPacketPayloadFormat packetPayloadFormat;
	BYTE 				bIsTipMode;
	BOOL                bIsCallGenerator;

} stOpenDecoderParams;

class CConfApi;
class CRsrcParams;
class CHardwareInterface;
class CBridgePartyVideoRelayIn;
class CSvcToAvcParams;
class MrmpOpenChannelRequestMessage;
class CBridgePartyCntl;


class CSVCToAVCTranslator : public CStateMachine {
CLASS_TYPE_1(CSVCToAVCTranslator,CStateMachine)

public:
	CSVCToAVCTranslator ();
	virtual ~CSVCToAVCTranslator ();
	virtual const char* NameOf() const { return "CSVCToAVCTranslator";}

	enum STATE {SETUP = (IDLE+1), CHANGE_STREAMS, CONNECTED_WITHOUT_AVC, CONNECTED, DISCONNECTING};
	enum ELastVideoStreamReq {eEmptyStreams, eNotEmptyStreams};

	// state machine functions
	virtual void	HandleEvent( CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	virtual void*	GetMessageMap();

	// API functions (from CBridgePartyVideoRelayIn)
	virtual void	Create(const CBridgePartyCntl* pBridgePartyCntl, const CBridgePartyVideoRelayIn* pBridgePartyVideoRelayIn, const CRsrcParams* pRsrcParamsDecoder);
	virtual void	Connect();
	virtual void	Disconnect();
	virtual void	UpdateAvcImageInConf();
	virtual void    UpdateVideoRelaySsrc();
	virtual void    UpdateArtOnTranslateVideoSSRCAck( DWORD status );
	virtual void    UpdateMrmpStreamIsMustAck( DWORD status );

//	virtual int 	GetTranslatorStatus();
	ConnectionID GetConnectionId();


public:
	// Action functions
	void  OnSVCToAVCTranslatorDisconnectIDLE( CSegment* pParam );
	void  OnSVCToAVCTranslatorDisconnectSETUP( CSegment* pParam );
	void  OnSVCToAVCTranslatorDisconnectCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnSVCToAVCTranslatorDisconnectCONNECTED( CSegment* pParam );
	void  OnSVCToAVCTranslatorDisconnectDISCONNECTING( CSegment* pParam );

	void  OnSVCToAVCTranslatorConnectIDLE( CSegment* pParam );
	void  OnSVCToAVCTranslatorConnectSETUP( CSegment* pParam );
	void  OnSVCToAVCTranslatorConnectCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnSVCToAVCTranslatorConnectCONNECTED( CSegment* pParam );
	void  OnSVCToAVCTranslatorConnectDISCONNECTING( CSegment* pParam );


	void  OnSVCToAVCTranslatorAckIDLE( CSegment* pParam );
	void  OnSVCToAVCTranslatorAckCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnSVCToAVCTranslatorAckCONNECTED( CSegment* pParam );
	void  OnSVCToAVCTranslatorAckDISCONNECTING( CSegment* pParam );
	void  OnSVCToAVCTranslatorAckSETUP( CSegment* pParam );

	void  OnTimerDecoderSyncCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnTimerDecoderSyncCONNECTED( CSegment* pParam );
	void  OnTimerDecoderSyncDISCONNECTING( CSegment* pParam );
	void  OnTimerDecoderSyncIDLE( CSegment* pParam );
	void  OnTimerDecoderSyncSETUP( CSegment* pParam );

	void  OnSVCToAVCTranslatorDecoderSyncIDLE( CSegment* pParam );
	void  OnSVCToAVCTranslatorDecoderSyncSETUP( CSegment* pParam );
	void  OnSVCToAVCTranslatorDecoderSyncDISCONNECTING( CSegment* pParam );
	void  OnSVCToAVCTranslatorDecoderSyncCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnSVCToAVCTranslatorDecoderSyncCONNECTED( CSegment* pParam );

	void  OnTimerFailureIDLE( CSegment* pParam );
	void  OnTimerFailureSETUP( CSegment* pParam );
	void  OnTimerFailureCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnTimerFailureCONNECTED( CSegment* pParam );
	void  OnTimerFailureDISCONNECTING( CSegment* pParam );

	void  OnTimerOpenConnectDecoderIDLE( CSegment* pParam );
	void  OnTimerOpenConnectDecoderCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnTimerOpenConnectDecoderCONNECTED( CSegment* pParam );
	void  OnTimerOpenConnectDecoderDISCONNECTING( CSegment* pParam );
	void  OnTimerOpenConnectDecoderSETUP( CSegment* pParam );

	void  OnUpdateAVCInConfIDLE( CSegment* pParam );
	void  OnUpdateAVCInConfSETUP( CSegment* pParam );
	void  OnUpdateAVCInConfCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnUpdateAVCInConfCONNECTED( CSegment* pParam );
	void  OnUpdateAVCInConfDISCONNECTING( CSegment* pParam );

	void  OnTimerDecoderSyncFailureWaitCONNECTED( CSegment* pParam );
	void  OnTimerDecoderSyncFailureWaitANYCASE( CSegment* pParam );

	void  OnUpdateArtSsrcAckSETUP( CSegment* pParam );
	void  OnUpdateArtSsrcAckCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnUpdateArtSsrcAckANYCASE( CSegment* pParam );

	void  OnUpdateMrmpStreamIsMustAckSETUP( CSegment* pParam );
	void  OnUpdateMrmpStreamIsMustAckCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnUpdateMrmpStreamIsMustAckCONNECTED( CSegment* pParam );
	void  OnUpdateMrmpStreamIsMustAckANYCASE( CSegment* pParam );

	void  OnUpdateSsrcSETUP( CSegment* pParam );
	void  OnUpdateSsrcCONNECTED( CSegment* pParam );
	void  OnUpdateSsrcANYCASE( CSegment* pParam );

	void  OnTimerUpdateSsrcCONNECTED_WITHOUT_AVC( CSegment* pParam );
	void  OnTimerUpdateSsrcANYCASE( CSegment* pParam );

	void  OnTimerConnectDecoderSETUP( CSegment* pParam );

	void  OnTimerDisconnectDISCONNECTING( CSegment* pParam );

	void  OnTimerStreamIsMustANYCASE( CSegment* pParam );

	void  NullActionFunction( CSegment* pParam );

protected:
	int   SendOpenDecoder();
	int   SendCloseDecoder();
	BOOL  IsAllOpenReqGotAck();
	BOOL  IsAllClosed();
	void  OnMplOpenPortAck(STATUS status);
	void  FillOpenDecoderParameters( stOpenDecoderParams *od);
	void  OnStartTranslatorCompleted();
	void  UpdateAllNonSVConSVCImage();
	void  RequestIntraAndStartTimer(const char* str);
	void  SendCloseAll();
	void  UpdateImageUponSVCtoAVCReady(DWORD decoderDetectedModeWidth,
															DWORD decoderDetectedModeHeight,
															DWORD decoderDetectedSampleAspectRatioWidth,
															DWORD decoderDetectedSampleAspectRatioHeight);
	void  UpdateImageUponSVCtoAVCNotReady();
	const char*  GetMyFullName();
	bool  IsAvcInConf();
	void  OnMRMPVideoStreamReqAck(STATUS status, ACK_IND_S* pAckIndStruct);
	void  FindSvcDecoderParams( DWORD &videoBitRate, DWORD &mbps, DWORD &fs );
//	VideoOperationPoint* GetCorrectLOperationPoint(VideoOperationPoint *sVideoOperationPoint, int numStreams);
	int   UpdateArtWIthSSRC( bool bStartIsMustTimer = true );
	void  OnMplConnectDecoderAck(STATUS  status);
	void  OnOpenAndUpdateAck();
	int   ConnectDecoderToArt();
	void  TranslatorEnded();
	void  InformConnectEnded( EStat status );
	void  InformDisconnectEnded( EStat status );
	void  UpdateLastReqUponError();

	// set / get
	void  SetChannelHandle(DWORD handle) {m_channelHandle = handle;}
	DWORD GetChannelHandle()const { return m_channelHandle; }
	void  SetAckOpenDecoder( BOOL YesNo );
	void  SetAckConnectDecoder( BOOL YesNo );
	void  SetAckUpdateArtSsrc( BOOL YesNo );
	void  SetAckUpdateMrmpStreamIsMust( BOOL YesNo );
	void  SetAckOpenOutChannel( BOOL YesNo );
	BOOL  GetAckOpenDecoder()const { return m_ackOpenDecoderRecieved; }
	BOOL  GetAckConnectDecoder()const { return m_ackConnectDecoderRecieved; }
	BOOL  GetAckOpenOutChannel() const { return m_ackUpdateArtSsrcRecieved; }
	void  SetBridgePartyVideoRelayIn( CBridgePartyVideoRelayIn*	pBPVideoRelayIn) { m_pBridgePartyVideoRelayIn = pBPVideoRelayIn; }
	CBridgePartyVideoRelayIn* GetBridgePartyVideoRelayIn() const { return m_pBridgePartyVideoRelayIn; }
	//void  GetTranslatorDebugMode();
	//bool  IsDebugMode( DWORD dubugNum );
	DWORD GetLastReqId(){return m_lastReqId;}
	DWORD GetLastReq(){return m_lastReq;}
	void  MyTest();

protected:

	PDECLAR_MESSAGE_MAP;

	CBridgePartyCntl* 			m_pBridgePartyCntl;
	CBridgePartyVideoRelayIn*	m_pBridgePartyVideoRelayIn;
	CVideoHardwareInterface*	m_pHardwareInterfaceDecoder;
	BYTE 						m_closePortAckStatus;// we need to save the status from the Ack to the close port request.
	BOOL						m_ackOpenDecoderRecieved;
	BOOL						m_ackConnectDecoderRecieved;
	BOOL						m_ackUpdateArtSsrcRecieved;
	BOOL						m_ackUpdateMrmpStreamIsMust;
	BOOL						m_ackCloseDecoderRecieved;
	BOOL						m_ackCloseOutChannelRecieved;
	DWORD 						m_channelHandle;	// Channel Handle for the TX (we open in CSVCToAVCTranslator)
	STATUS						m_closeStatus;
	BOOL						m_bDecoderSynced;
	ELastVideoStreamReq			m_lastVideoStreamsReq;
	DWORD						m_dDebugIntraCounter;
	DWORD						m_dDebugDecoderSyncFailureCounter;
	DWORD						m_dDebugDecoderSyncFailureCounterAcc;
	DWORD						m_lastReqSSRC;
	DWORD						m_lastIsMustSsrc;
	int							m_connectionStatus;
	WORD						m_fs;
	WORD						m_mbps;
	DWORD						m_partyRsrcId;
	DWORD 						m_lastReqId; // for mcu internal problem print in case request fails / timeout
	DWORD						m_lastReq;
};


//class CMrmpOpenChannelRequestContainer  {
//public:
//	CMrmpOpenChannelRequestContainer();
//
//public:
//	MrmpOpenChannelRequestStruct m_openChannel;
//};
#include "NStream.h"
#include "DataTypes.h"

class CIsLoggerToSend  : public COstrStream
{
public:

	CIsLoggerToSend();
	CIsLoggerToSend& operator<<(const char* str);

	virtual const char* NameOf() const { return "CIsLoggerToSend";}

	bool IsLoggerLevelNotToSend( int level);
	int m_level;
};




#endif //_CSVCToAVCTranslator_H_



