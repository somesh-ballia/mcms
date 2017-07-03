//+========================================================================+
//                        BridgeVideoOutCopEncoder.h                            |
//					      Copyright 2005 Polycom                           |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// FILE:       BridgeVideoOutCopEncoder.h                                   	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Keren                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2009  | Description                                    |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#ifndef _BRIDGE_VIDEO_IN_COP_DECODER
#define _BRIDGE_VIDEO_IN_COP_DECODER

#include "BridgePartyVideoIn.h"
//#include "VisualEffectsParams.h"
//#include "VideoBridgePartyCntl.h"
//#include "VideoDefines.h"
//#include "VideoLayout.h"
//#include "VideoApiDefinitions.h"
//#include "TextOnScreenMngr.h"
//#include "BridgePartyVideoParams.h"
//
//

class CBridgePartyVideoIn;
class CBridgePartyCntl;
class CRsrcParams;
class CBridgePartyVideoInParams;
class CTaskApi;
class CConf;


class CBridgeVideoInCopDecoder : public CBridgePartyVideoIn
{
CLASS_TYPE_1(CBridgeVideoInCopDecoder,CBridgePartyVideoIn)
public:

	// States definition
	enum STATE{SETUP = (IDLE+1), CONNECTED, DISCONNECTING, CLOSING, CONNECTING, OPENED};


	//constructors
	CBridgeVideoInCopDecoder ();
	virtual ~CBridgeVideoInCopDecoder();

	virtual const char*  NameOf() const{ return "CBridgeVideoInCopDecoder";}
	virtual void Create(const CBridgePartyCntl*	pBridgePartyCntl, const CRsrcParams* pRsrcParams,
								const CBridgePartyMediaParams * pBridgePartyVideoOutParams);
	WORD GetCopDecoderIndex() const{return m_copDecoderIndex;}
	DWORD GetCopDecoderArtId()const{return m_copArtPartyId;}
/*
 *
 * virtual void	Connect()	;
	virtual void	DisConnect();
	virtual const char*  NameOf() const{ return "CBridgePartyVideoIn";}
	virtual void	Create (const CBridgePartyCntl*	pBridgePartyCntl,
							const CRsrcParams* pRsrcParams,
							const CBridgePartyMediaParams* pBridgePartyMediaParams);
	virtual void   CreateForMove(const CBridgePartyCntl* pBridgePartyCntl,
								 const CRsrcParams* pRsrcParams,
								 const CBridgePartyMediaParams * pBridgePartyVideoInParams);
	virtual const CImage* GetPartyImage(void)const{ return m_pImage;}

	virtual void UpdateSelfMute(RequestPriority who, EOnOff eOnOff);
	void UpdateNewConfParamsForOpenedPortAfterMove(DWORD confRsrcId, const CBridgePartyMediaParams * pBridgePartyVideoInParams);

	virtual BOOL	IsConnected() { return ( (m_state == CONNECTED) ? TRUE : FALSE ); }
	virtual BOOL	IsConnecting() {return ((m_state == SETUP) ? TRUE : FALSE ); }
	virtual BOOL	IsDisConnected() { return ( (m_state == IDLE) ? TRUE : FALSE ); }
	*/
	virtual BOOL	IsClosed() { return ( (m_state == IDLE) ? TRUE : FALSE ); }

	/*

	virtual void    UpdateVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void	DumpAllInfoOnConnectionState(CMedString* pMedString,bool isShortPrint=false);

	virtual void    UpdateVideoClarity(WORD isVideoClarityEnabled);

	BYTE	IsPartyImageSyncLoss(){return m_pImage->isSyncLost(); }
	void	SetSiteName(const char* visualName);
	void 	ReSync();
    void    UpdatePartyInParams(CUpdatePartyVideoInitParams* pUpdatePartyVideoInitParams);
    BYTE 	IsAckOnOpenPort(){return m_isConnected;} //for debug trace only

    //void   CopyParamsForMove(const CVideoBridgePartyCntl* pOldPartyCntl);
 *
 */
	virtual void Close();
	void SendKillPort();
	BYTE IsDetectedVideoMatchCurrentResRatio();

protected:
/*
	virtual void  SendOpenDecoder();*/
		virtual void  SendConnectToRtp();
		virtual void  SendDisconnectFromRtp();

		/*virtual void  SendCloseDecoder();

		void UpdateDBLocalVideoSyncState(BYTE isSynced);
*/
		virtual void SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
/*
		virtual void MuteYourSelf(RequestPriority who);
		virtual void UnMuteYourSelf(RequestPriority who);

		//action functions
		virtual void  OnVideoBridgePartyConnectIDLE(CSegment* pParam);
		virtual void  OnVideoBridgePartyConnectSETUP(CSegment* pParam);
		virtual void  OnVideoBridgePartyConnectCONNECTED(CSegment* pParam);
		virtual void  OnVideoBridgePartyConnectDISCONNECTING(CSegment* pParam);
*/
		virtual void  OnVideoBridgePartyConnectOPENED(CSegment* pParam);
		virtual void  OnVideoBridgePartyConnectCLOSING(CSegment* pParam);
		virtual void  OnVideoBridgePartyConnectCONNECTING(CSegment* pParam);
 /*
		virtual void  OnVideoBridgePartyDisConnectIDLE(CSegment* pParam);
		virtual void  OnVideoBridgePartyDisConnectSETUP(CSegment* pParam);
		virtual void  OnVideoBridgePartyDisConnectCONNECTED(CSegment* pParam);
		*/
		virtual void  OnVideoBridgePartyDisConnectDISCONNECTING(CSegment* pParam);
		virtual void  OnVideoBridgePartyDisConnect(CSegment* pSeg);
		virtual void  OnVideoBridgePartyDisConnectOPENED(CSegment* pSeg);
		virtual void  OnVideoBridgePartyDisConnectCLOSING(CSegment* pSeg);
		virtual void  OnVideoBridgePartyDisConnectCONNECTING(CSegment* pSeg);
/*
		virtual void  OnMplAckSETUP(CSegment* pParam);*/
		virtual void  OnMplAckDISCONNECTING(CSegment* pParam);
	/*  virtual void  OnMplDecoderSyncSETUP(CSegment* pParam);
		virtual void  OnMplDecoderSyncCONNECTED(CSegment* pParam);*/
		virtual void  OnMplDecoderSyncOPENED(CSegment* pParam);
		virtual void  OnMplDecoderSyncCLOSING(CSegment* pParam);
		virtual void  OnMplDecoderSyncCONNECTING(CSegment* pParam);

		virtual void  OnMplAckCLOSING(CSegment* pParam);
		virtual void  OnMplAckCONNECTING(CSegment* pParam);
		/*

		virtual void  OnMplOpenPortAck(STATUS  status);
		virtual void  OnMplConnectAck(STATUS  status);
		*/
		virtual void  OnMplClosePortAck(STATUS  status);
		virtual void  OnMplDisconnectAck(STATUS  status);
/*
		virtual void  OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam);
		virtual void  OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam);
		virtual void  OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam);
		virtual void  OnVideoBridgePartyUpdateVideoParamsDISCONNECTING(CSegment* pParam);
*/
		virtual void  OnVideoBridgePartyUpdateVideoParamOPENED(CSegment* pParam);
		virtual void  OnVideoBridgePartyUpdateVideoParamCLOSING(CSegment* pParam);
		virtual void  OnVideoBridgePartyUpdateVideoParamCONNECTING(CSegment* pParam);
		/*

		virtual void  OnVideoBridgePartyUpdateVideoClarityIDLE(CSegment* pParam);
		virtual void  OnVideoBridgePartyUpdateVideoClaritySETUP(CSegment* pParam);
		virtual void  OnVideoBridgePartyUpdateVideoClarityCONNECTED(CSegment* pParam);
		virtual void  OnVideoBridgePartyUpdateVideoClarityDISCONNECTING(CSegment* pParam);

		virtual void  OnTimerDecoderSyncCONNECTED(CSegment* pParam);
		*/
		virtual void OnVideoBridgePartyCloseIDLE(CSegment* pParam);
		virtual void OnVideoBridgePartyCloseSETUP(CSegment* pParam);
		virtual void OnVideoBridgePartyCloseCONNECTED(CSegment* pParam);
		virtual void OnVideoBridgePartyCloseDISCONNECTING(CSegment* pParam);
		virtual void OnVideoBridgePartyCloseCLOSING(CSegment* pParam);
		virtual void OnVideoBridgePartyCloseCONNECTING(CSegment* pParam);
		virtual void OnVideoBridgePartyCloseOPENED(CSegment* pParam);
		virtual void OnVideoBridgePartyClose(CSegment* pParam);

		//members
		DWORD m_copArtConnectionId;
		DWORD m_copArtPartyId;
		WORD  m_copDecoderIndex;
		/*
		BYTE m_isConnected; //for debug trace only
		CImage* m_pImage;
		CSegment* m_pDecoderSyncSegm;
		eTelePresencePartyType m_eTelePresenceMode;  // PCI bug patch (to be removed in V3.x)
		DWORD	m_backgroundImageID; //The background id is needed for optimization - in order to fit to FPGA scaler output resolutions more accurately
*/
		PDECLAR_MESSAGE_MAP


};


#endif //_BRIDGE_VIDEO_IN_COP_DECODER
