//+========================================================================+
//                AvcToSvcTranslator.H                                     +
//+========================================================================+

#ifndef _AVCTOSVCTRANSLATOR_H_
#define _AVCTOSVCTRANSLATOR_H_

#include "StateMachine.h"
#include "ConfPartyDefines.h"
#include "ConfPartyOpcodes.h"
#include "VideoDefines.h"
#include "MplMcmsStructs.h"
#include "VideoApiDefinitions.h"
#include "Layout.h"
#include "VideoStructs.h"

typedef struct {
	DWORD speakerPlaceInLayout;
	eVideoResolution videoEncoderResolution;
	DWORD decoderDetectedModeWidth;
	DWORD decoderDetectedModeHeight;
	DWORD decoderDetectedSampleAspectRatioWidth;
	DWORD decoderDetectedSampleAspectRatioHeight;
	DWORD videoAlg;
	DWORD fs;
	DWORD mbps;
	eVideoConfType videoConfType;
	BYTE isSiteNamesEnabled;
	BYTE bUseSharedMemForChangeLayoutReq;
	BYTE isVSW;
} AVC_TO_SVC_CHANGE_LAYOUT_ST;


typedef struct {
	eVideoConfType	videoConfType;
	APIS32		nVideoEncoderType;
	APIS32		videoBitRate;
	APIS32		videoAlg;
	APIS32 		sampleAspectRatio;
	APIS32		videoResolutionTableType;
	APIU32		parsingMode;
	APIU32		nFpsMode;
	CROPPING_PARAM_S	tCroppingParams;
    APIUBOOL 	bEnableMbRefresh;
 	APIUBOOL	bIsTipMode;
	APIUBOOL	bIsLinkEncoder;
    APIUBOOL	bUseIntermediateSDResolution;
    APIUBOOL    bIsCallGenerator;
    H264_SVC_VIDEO_PARAM_S	tH264SvcVideoParams;
} OPEN_TRANSLATOR_ENCODER;

class CVideoHardwareInterface;
class CConfApi;
class CRsrcParams;
class CHardwareInterface;
class CBridgePartyVideoIn;
class CAvcToSvcParams;
class CBridgePartyCntl;


class CAvcToSvcTranslator : public CStateMachine {
CLASS_TYPE_1(CAvcToSvcTranslator,CStateMachine)

public:
	CAvcToSvcTranslator ();
	CAvcToSvcTranslator (const CAvcToSvcTranslator& rOtherAvcToSvcTranslator);
	virtual ~CAvcToSvcTranslator ();
	virtual const char* NameOf() const { return "CAvcToSvcTranslator";}
	CAvcToSvcTranslator&   operator=(const CAvcToSvcTranslator& rOtherAvcToSvcTranslator);

	enum STATE {SETUP = (IDLE+1), CHANGE_LAYOUT, CONNECTED, DISCONNECTING};

	// state machine functions
	virtual void	HandleEvent( CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	virtual void*	GetMessageMap();
	virtual void 	Dump() const;

	// API functions (from CBridgePartyVideoRelayIn)
	virtual void  Create (const CBridgePartyCntl* pBridgePartyCntl, const CBridgePartyVideoIn* pBridgePartyVideoIn, const CRsrcParams* pRsrcParamsVideo, bool bConnectToSACEncoder );
	virtual void  Connect();
	virtual void  Disconnect();
	virtual void  UpdateImage();
	virtual void  RemoveConfParams();
	virtual void  UpdateNewConfParams ( const CBridgePartyVideoIn* pBridgePartyVideoIn, DWORD confRsrcId);
	virtual void  RequestChangeLayout();

	// others
//	DWORD GetMyId();
	virtual void  UpdateParams ( int layerId, DWORD resHight, DWORD resWidth, DWORD ssrc, DWORD channelHandle );
	virtual void  SendRelayIntraRequestToAvcToSvcTranslator();

	// others Get/Is functions
	bool  IsConnected();
	bool  IsActive();
	bool  IsReadyToBeKilled();
	DWORD GetEncoderConnectionId();
	DWORD GetLastReqId(){return m_lastReqId;}
	DWORD GetLastReq(){return m_lastReq;}
	bool  IsVswStream() const {return m_bIsVswStream;}
	bool  IsConnectToSACEncoder()const { return m_bConnectToSACEncoder; }
	DWORD GetSsrc();



public:
	// Action functions
	void  OnVideoBridgePartyConnectIDLE( CSegment* pParam );
	void  OnVideoBridgePartyConnectSETUP( CSegment* pParam );
	void  OnVideoBridgePartyConnectCHANGELAYOUT( CSegment* pParam );
	void  OnVideoBridgePartyConnectCONNECTED( CSegment* pParam );
	void  OnVideoBridgePartyConnectDISCONNECTING( CSegment* pParam );

	void  OnVideoBridgePartyDisConnectIDLE( CSegment* pParam );
	void  OnVideoBridgePartyDisConnectSETUP( CSegment* pParam );
	void  OnVideoBridgePartyDisConnectCHANGELAYOUT( CSegment* pParam );
	void  OnVideoBridgePartyDisConnectCONNECTED( CSegment* pParam );
	void  OnVideoBridgePartyDisConnectDISCONNECTING( CSegment* pParam );

	void  OnMplAckIDLE( CSegment* pParam );
	void  OnMplAckSETUP( CSegment* pParam );
	void  OnMplAckCHANGELAYOUT( CSegment* pParam );
	void  OnMplAckCONNECTED( CSegment* pParam );
	void  OnMplAckDISCONNECTING( CSegment* pParam );

	void  OnTimerOpenEncoderIDLE( CSegment* pParam );
	void  OnTimerOpenEncoderCHANGELAYOUT( CSegment* pParam );
	void  OnTimerOpenEncoderCONNECTED( CSegment* pParam );
	void  OnTimerOpenEncoderDISCONNECTING( CSegment* pParam );
	void  OnTimerOpenEncoderSETUP( CSegment* pParam );

	void  OnTimerChangeLayoutIDLE( CSegment* pParam );
	void  OnTimerChangeLayoutSETUP( CSegment* pParam );
	void  OnTimerChangeLayoutCONNECTED( CSegment* pParam );
	void  OnTimerChangeLayoutDISCONNECTING( CSegment* pParam );
	void  OnTimerChangeLayoutCHANGELAYOUT( CSegment* pParam );

	void  OnTimerCloseEncoderIDLE( CSegment* pParam );
	void  OnTimerCloseEncoderSETUP( CSegment* pParam );
	void  OnTimerCloseEncoderCONNECTED( CSegment* pParam );
	void  OnTimerCloseEncoderDISCONNECTING( CSegment* pParam );
	void  OnTimerCloseEncoderCHANGELAYOUT( CSegment* pParam );

	void  OnVideoBridgePartyFastUpdateCHANGELAYOUT( CSegment* pParam );
	void  OnVideoBridgePartyFastUpdateDISCONNECTING( CSegment* pParam );
	void  OnVideoBridgePartyFastUpdateCONNECTED( CSegment* pParam );

	void  OnEncoderResolutionChangedCHANGELAYOUT( CSegment* pParam );
	void  OnEncoderResolutionChangedCONNECTED( CSegment* pParam );
	void  OnTimerConnectEncoderSETUP( CSegment* pParam );

	void  NullActionFunction( CSegment* pParam );
	void  IgnoreFunction( CSegment* pParam );


protected:
	// set / get

	// other functions
	EStat PrepareOpenTranslatorEncoder( OPEN_TRANSLATOR_ENCODER *ote );
	void  OnMplOpenPortAck( STATUS status);
	void  OnMplConnectEncoderArtAck(STATUS status);
	EStat SendChangeLayout_1x1();
	void  OnMplChangeLayoutAck( STATUS status );
	void  OnMplCloseEncoderAck( STATUS status );
	void  CloseEncoderAvcToSvc();
	const char* GetMyFullName();
	const char* GetMyConfName();
	eVideoProfile  	GetProfile( WORD videoProfile );
	eVideoFrameRate GetFameRate( DWORD OperationPointFrameRate );
	void  UpdateImageUponAVCtoSVCReady();
	bool  PrepareLayoutParameters( CLayout* pLayout );
	void  PrepareChangeLayoutRequest( AVC_TO_SVC_CHANGE_LAYOUT_ST *cl, CVisualEffectsParams* sVisualEffects, CSiteNameInfo* sSiteNameInfo );
	void  NotifySvcEpsOnDisconnectAvcToSvcTranslator();
	void  UpdateImageUponAVCtoSVCClosing();
	DWORD GetDelayIntraTime();
	EStat ConnectEncoderToART();
	void  InformConnectEnded( EStat status );
	void  InformDisconnectEnded( EStat status );
	EStat SendChangeLayoutAgain();
	void  UpdateLastReqUponError();
	void  OnTimerSendRelayIntraRequestToAvcToSvcTranslator( CSegment* pParam );
	
protected:

	PDECLAR_MESSAGE_MAP;

	CBridgePartyCntl* 			m_pBridgePartyCntl;
	CBridgePartyVideoIn*		m_pBridgePartyVideoIn;
	CVideoHardwareInterface*	m_pHardwareInterfaceEncoder;
	DWORD 						m_lastReqId; // for mcu internal problem print in case request fails / timeout
	DWORD						m_lastReq;
	DWORD						m_resolution;
	int							m_layerId;
	DWORD						m_resHight;
	DWORD						m_resWidth;
	DWORD						m_ssrc;
	DWORD						m_channelHandle;
	DWORD						m_EncoderConnectionId;
	DWORD						m_tDelayIntraTime;
	bool						m_bConnectToSACEncoder;
	bool						m_bIsReadyToBeKilled;
	bool						m_bIsTranslatorError;
	int							m_iChangeLayoutCounter;
	DWORD						m_partyRsrcId;
	bool						m_bResendChangeLayout;
	bool						m_bSentOpenEncoder;
	bool						m_bIsVswStream;
};


class CAvcToSvcTranslatorVsw : public CAvcToSvcTranslator
{
CLASS_TYPE_1(CAvcToSvcTranslatorVsw,CAvcToSvcTranslator)

public:
	CAvcToSvcTranslatorVsw ();
	~CAvcToSvcTranslatorVsw ();
	//CAvcToSvcTranslatorVsw (const CAvcToSvcTranslatorVsw& rOtherAvcToSvcTranslatorVsw);
	virtual const char* NameOf() const { return "CAvcToSvcTranslatorVsw";}
//	CAvcToSvcTranslatorVsw&   operator=(const CAvcToSvcTranslatorVsw& rOtherAvcToSvcTranslatorVsw);

	// others
	virtual void  SendRelayIntraRequestToAvcToSvcTranslator();
	virtual void  RequestChangeLayout();
public:

	void  OnVideoBridgePartyConnectIDLE( CSegment* pParam );
	void  OnVideoBridgePartyConnectCONNECTED( CSegment* pParam );

	void  OnVideoBridgePartyDisConnectIDLE( CSegment* pParam );
	void  OnVideoBridgePartyDisConnectCONNECTED( CSegment* pParam );

protected:


protected:


	PDECLAR_MESSAGE_MAP

};


class CAvcToSvcOpenEncoder {
public:
	CAvcToSvcOpenEncoder ();
public:
	OPEN_TRANSLATOR_ENCODER oe;
};

#endif //_AVCTOSVCTRANSLATOR_H_



