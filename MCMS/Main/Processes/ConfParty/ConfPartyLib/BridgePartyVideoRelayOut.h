#ifndef BRIDGEPARTYVIDEORELAYOUT_H_
#define BRIDGEPARTYVIDEORELAYOUT_H_

#include "RelayIntraData.h"
#include "BridgePartyMediaUniDirection.h"
#include "VideoRelayBridgePartyCntl.h"
#include "VideoRelayOutStreamsHandler.h"
#include "UpdatePartyVideoRelayInitParams.h"
#include "RelayIntraData.h"
#include "IVRPlayMessage.h"

// Time-out values
#define VIDEO_OUT_CHANGE_STREAMS_TOUT  5*SECOND  //8 second timeout
#define STOP_SHOW_SLIDE_TIMER          5*SECOND

class CBridgePartyVideoRelayOut : public CBridgePartyMediaUniDirection
{
  CLASS_TYPE_1(CBridgePartyVideoRelayOut, CBridgePartyMediaUniDirection)

public:
  // States definition
  	enum STATE{SETUP = (IDLE+1), CONNECTED, CHANGE_STREAMS, SLIDE, STOP_SLIDE};//, DISCONNECTING};
  	virtual void*	GetMessageMap() {return (void*)m_msgEntries;}

  	CBridgePartyVideoRelayOut();
  	CBridgePartyVideoRelayOut(const CBridgePartyVideoRelayOut& rOtherBridgePartyVideRelayoIn);
  	virtual                       ~CBridgePartyVideoRelayOut ();
  	virtual const char*            NameOf() const   { return "CBridgePartyVideoRelayOut";}


  	virtual void	Create (const CBridgePartyCntl*	pBridgePartyCntl);
    virtual void    Create(const CBridgePartyCntl* pBridgePartyCntl,const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyMediaParams);


  	virtual void	Connect();
  	virtual void	DisConnect();


  	virtual BOOL	IsConnected();
  	virtual BOOL	IsConnecting() {return ((m_state == SETUP) ? TRUE : FALSE ); }
  	virtual BOOL	IsDisConnected() { return ( (m_state == IDLE) ? TRUE : FALSE ); }
  	virtual BOOL    IsDisconnecting() {return FALSE;}// ( (m_state == DISCONNECTING) ? TRUE : FALSE ); }
  	virtual bool    IsStateIsConnected();


  	void UpdatePartyOutParams(CUpdatePartyVideoRelayInitParams* updatePartyVideoRelayInitParams);//TODO WHEN CONNECT WHILE DISCONNECTING
  	void UpdateVideoParams(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams);//WHEN UPDATE VIDEO OUT

  	void AddImage();
  	void DelImage();
  	void MuteImage();
  	void UnMuteImage();
  	void ChangeSpeakers();
  	void ChangeAudioSpeaker();
  	void UpdateImage();

  	void EpAskForIntra(const RelayIntraParam& intraParam);
  	void ShowSlide(CSegment* pDataSeg);
  	void StopShowSlide(CSegment *pDataSeg);
  	void AckOnIvrScpShowSlide(BYTE bIsAck);
  	void AckOnIvrScpStopShowSlide(BYTE bIsAck);

	virtual void UpdateOnImageAvcToSvcTranslate();
	virtual void GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap);


protected:

  //	virtual void  SaveUpdatedVideoOutRelayParams( (const CBridgePartyVideoRelayMediaParams*)pBridgePartyMediaParams);
//state machines (the common functions with same names as bridgepartyvideoIn is in case we will add common base class)
  	virtual void  OnVideoBridgePartyConnect(CSegment* pParam);
	virtual void  OnVideoBridgePartyConnectIDLE(CSegment* pParam);
	virtual void  OnVideoBridgePartyConnectSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyConnectSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyConnectCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyConnectCHANGESTREAMS(CSegment* pParam);
	virtual void  OnVideoBridgePartyConnectSTOPSLIDE(CSegment* pParam);

	virtual void  OnVideoBridgePartyDisconnect(CSegment* pParam);
	virtual void  OnVideoBridgePartyDisconnectIDLE(CSegment* pParam);
	virtual void  OnVideoBridgePartyDisconnectSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyDisconnectSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyDisconnectCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyDisconnectCHANGESTREAMS(CSegment* pParam);

	virtual void  OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateVideoParamsSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateVideoParamsSTOPSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateVideoParamsCHANGESTREAMS(CSegment* pParam);

	virtual void OnMplAckIDLE(CSegment* pParam);
	virtual void OnMplAckSLIDE(CSegment* pParam);
	virtual void OnMplAckSETUP(CSegment* pParam);
	virtual void OnMplAckCONNECTED(CSegment* pParam);
	virtual void OnMplAckCHANGESTREAMS(CSegment* pParam);


	virtual void OnMplVideoSourcesAck(STATUS status);

	virtual void  OnVideoBridgePartyAddImageSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyAddImageSTOPSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyAddImageSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyAddImageCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyAddImageCHANGESTREAMS(CSegment* pParam);


	virtual void  OnVideoBridgePartyDelImageSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyDelImageSTOPSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyDelImageSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyDelImageCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyDelImageCHANGESTREAMS(CSegment* pParam);


	virtual void  OnVideoBridgePartyMuteImageSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyMuteImageSTOPSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyMuteImageSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyMuteImageCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyMuteImageCHANGESTREAMS(CSegment* pParam);


	virtual void  OnVideoBridgePartyUnMuteImageSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyUnMuteImageSTOPSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyUnMuteImageSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyUnMuteImageCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyUnMuteImageCHANGESTREAMS(CSegment* pParam);


	virtual void  OnVideoBridgePartyChangeSpeakersSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyChangeSpeakersSTOPSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyChangeSpeakersSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyChangeSpeakersCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyChangeSpeakersCHANGESTREAMS(CSegment* pParam);


	virtual void  OnVideoBridgePartyChangeAudioSpeakerSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyChangeAudioSpeakerSTOPSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyChangeAudioSpeakerSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyChangeAudioSpeakerCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyChangeAudioSpeakerCHANGESTREAMS(CSegment* pParam);


	virtual void  OnVideoBridgePartyUpdateImageSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateImageSTOPSLIDE(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateImageSETUP(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateImageCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyUpdateImageCHANGESTREAMS(CSegment* pParam);

	virtual void  OnEpAskForIntra(CSegment* pParam);
	virtual void  OnEpAskForIntraCONNECTED(CSegment* pParam);
	virtual void  OnVideoBridgePartyFastUpdateSLIDE(CSegment* pParam);
	virtual void  OnEpAskForIntraIDLE(CSegment* pParam);
	virtual void  OnEpAskForIntraSTOP_SLIDE(CSegment* pParam);
	virtual void  OnEpAskForIntraSETUP(CSegment* pParam);
	virtual void  OnEpAskForIntraCHANGE_STREAMS(CSegment* pParam);



	virtual void  OnVideoBridgePartyShowSlideIDLE(CSegment* pParam);
	virtual void  OnVideoBridgePartyShowSlideSLIDE(CSegment* pParam);

	virtual void  OnVideoBridgePartyStopShowSlideSLIDE(CSegment *pParam);
	virtual void  OnVideoBridgePartyStopShowSlideIDLE(CSegment *pParam);

	virtual void  OnPartyCntlAckOnIvrScpShowSlideSLIDE(CSegment* pParam);
	virtual void  OnPartyCntlAckOnIvrScpStopShowSlideSTOPSLIDE(CSegment* pParam);

	virtual void OnVideoBridgePartyUpdateOnImageAvcToSvcSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateOnImageAvcToSvcCONNECTED(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateOnImageAvcToSvcSETUP(CSegment* pParam);
	virtual void OnVideoBridgePartyUpdateOnImageAvcToSvcCHANGESTREAMS(CSegment* pParam);

	virtual void OnTimerStopShowSlideSTOPSLIDE(CSegment* pParam);
	virtual void OnVideoBridgePartyToutCHANGESTREAMS(CSegment* pParam);

	void SaveUpdatedVideoParams(const CBridgePartyVideoRelayMediaParams* pBridgePartyMediaParams);
    bool StartChangeStreams(bool bImmediately);
    bool CalculateChangeStreamsRequest(AskForRelayIntra& rEpIntraParams);
    bool SendVideoSourcesRequest();
    void SendShowSlide();
    void SendUpdateToAudioBridgeOnSeenImageIfNeeded();

private:
	bool CalcSlideParamsForHardware(int layerId, DWORD &fs, DWORD &mbps, DWORD &videoBitRate);
    void SendShowSlideToHardware();
    void SendUpdateSlideToHardware(int layerIdForIvrSlide);
    bool IsOneStreamWithIVRPipeIdInStraemsList();
    void SetIsShowSlideToHardwareSent(bool bYesNo);
    void SetIsAckOnIvrScpShowSlideReceived(bool bYesNo);
    bool CanSlideBeSent();
    void SetIsAckOnIvrScpStopShowSlideReceived(bool bYesNo);
    void SetIsConnectDuringStopSlideState(bool bYesNo);
    bool CanSlideStopStateBeFinished();
    int  GetLayerIdForUpdateIvrSlide();
    void SendScpIvrShowSlideReqToPartyCntl();
    void SendNotificationIfNeededInSlideStage();
    void StartChangeStreamsIsChangeStreamWhileWaitingForAck();


//members
protected:
   	BOOL m_isReady;

   	CVideoRelayOutStreamsHandler* m_pVideoOutStreamsHandler; // build the VideoSourcesRequest

   	bool m_isChangeStreamWhileWaitingForAck;

   	CVideoRelayOutParamsStore m_videoOutParamsStore;

   	CIVRPlayMessage*     m_pIVRPlayMessage;

	PDECLAR_MESSAGE_MAP


};



#endif /* BRIDGEPARTYVIDEORELAYOUT_H_ */
