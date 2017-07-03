#ifndef VIDEO_OUT_STREAMS_HANDLER_H_
#define VIDEO_OUT_STREAMS_HANDLER_H_


#include <map>
#include <utility>
#include "PObject.h"
#include "RelayIntraData.h"
#include "ScpNotificationWrapper.h"
#include "VideoRelayInOutMediaStream.h"
#include "VideoBridge.h"
#include "VideoRelayBridgePartyCntl.h"
#include "VideoRelaySourcesParams.h"
#include "BridgePartyVideoRelayMediaParams.h"
#include "ConfPartyGlobals.h"


typedef struct
{
	bool bIsNeedToSendUpdate;
	PartyRsrcID idOfPartyToUpdate;
	PartyRsrcID idOfSeenParty;
}UpdateOnSeenImageStruct;


class CVideoRelayOutParamsStore : public CPObject
{
	CLASS_TYPE_1(CVideoRelayOutParamsStore, CPObject)

public:
    CVideoRelayOutParamsStore();

    virtual ~CVideoRelayOutParamsStore();

    virtual const char* NameOf() const { return "CVideoRelayOutParamsStore";}
    void Init( const CBridgePartyVideoRelayMediaParams* bridgePartyVideoParams );
    bool GetIsStreamsPriorityAccordingToPriorityFlag()const {return m_isStreamsPriorityAccordingToPriorityFlag;}
    void InitAllIvrSlideParams();
    void InitUpdateOnSeenImageStruct();


    CBridgePartyVideoRelayMediaParams*        m_pCurrentMediaParam;   // last updated media params from video bridge

    std::multimap<int, CVideoRelayOutMediaStream>   m_videoStreamsSortedByResolutionOrPriority;
    std::multimap<int, CVideoRelayOutMediaStream>   m_forceSpecVideoStreamsByLayerId;

    CVideoRelaySourcesParams*                 m_pCurrentVideoSourcesRequest; // last VideoSourcesRequest sent to party
    CScpNotificationWrapper* 				  m_pCurrentVideoScpNotificationRequest;// last Video Scp notification sent to party

    bool                                      m_needToSendScpStreamsNotification;

    bool                                      m_isStreamsPriorityAccordingToPriorityFlag;//new value was added to SCP spec, according to it the streams priority is defined for backward compatibility we will support both

                                              //StartShowSlide
    bool                                      m_bIsShowSlideToHardwareSent;
    bool                                      m_bIsAckOnIvrScpShowSlideReceived;
   // DWORD                                     m_pipeIdForIvrSlide;

                                              //StopShowSlide
    bool                                      m_bIsAckOnIvrScpStopSlideReceived;
    bool                                      m_bIsSCPShowSlideReqCanBeSent;
    bool                                      m_bIsConnectDuringStopSlideState;
    DWORD                                     m_showSlideSCPSeqNumber;
    UpdateOnSeenImageStruct                   m_updateOnSeenImageStruct;

private:
    void UpdateIsPrioritySetAccordingToPriorityFlag();


    CVideoRelayOutParamsStore(const CVideoRelayOutParamsStore& rOther) : CPObject(rOther) { *this = rOther; }
    CVideoRelayOutParamsStore& operator= (const CVideoRelayOutParamsStore& rOther) { return *this; }

};



class CVideoRelayOutStreamsHandler : public CPObject
{
	CLASS_TYPE_1(CVideoRelayOutStreamsHandler, CPObject)

public:
	CVideoRelayOutStreamsHandler(const CVideoBridgeCP* pVideoBridge, const CVideoRelayBridgePartyCntl* pCVideoBridgePartyCntl);
	virtual ~CVideoRelayOutStreamsHandler();

	virtual const char* NameOf() const { return "CVideoRelayOutStreamsHandler";}

    bool BuildVideoSourcesRequest( CVideoRelayOutParamsStore& pVideoMediaParam, AskForRelayIntra& rEpIntraParams);

	bool FillVideoSource(CVideoRelayOutMediaStream& rCVideoRelayOutMediaStream, const CImage* pImage, CVideoRelaySourcesParams& rResultVideoSourcesRequest, bool cantHaveSameSource);

	bool IsSpeakerImage(const CImage* pImage);

//	bool GetBestSsrcAndLayerIdFromImage(int requestedLayerId,const CImage* pImage, unsigned int& resultSsrc,int &resultLayerId);
//	bool IsSameResolutionLayerId(int requestedLayerId,int streamLayerId);

	bool GetCsrsForPipeId(CVideoRelaySourcesParams videoSourcesRequest, DWORD pipeId, DWORD& rCsrcOfPipId);
	void AddIntraReq(RelayIntraParams& rEpIntraParams, PartyRsrcID partyRsrcId, DWORD csrc,bool isGdr);
	bool GetPartyIdFromSSRC(int ssrrc,PartyRsrcID& rPartyRsrcId);

	void UpdateNotificationsOnAllNonIVRRequestedStreamsCantBeProvided(CVideoRelayOutParamsStore& rVideoMediaParam,  DWORD ivrSsrc);

protected:
    bool FillVideoSourcesRequest( const CVideoRelayOutParamsStore& pVideoMediaParam,
                                  CVideoRelaySourcesParams& rResultVideoSourcesRequest,
    							  CScpNotificationWrapper& rResultVideoScpNotificationRequest,
    							  UpdateOnSeenImageStruct& resultUpdateOnImageStruct);
    bool FillVideoSourcesRequestForCascadeLink(const CVideoRelayOutParamsStore& pVideoMediaParam, CVideoRelaySourcesParams& rResultVideoSourcesRequest,CScpNotificationWrapper& rResultVideoScpNotificationRequest, UpdateOnSeenImageStruct& resultUpdateOnImageStruct, std::list<const CImage*> listImagesToSend, PartyRsrcID  partyRsrcId);
	void FillEpIntaParams(CVideoRelaySourcesParams& rPrevVideoSourcesRequest,CVideoRelaySourcesParams& rResultVideoSourcesRequest,AskForRelayIntra& rEpIntraParams);
	void CreateEmptyRequest(CVideoRelaySourcesParams& rResultVideoSourcesRequest, int channelHandle);
	void FillStreamsAccordingToResolutionOrPriority(WORD & num_of_video_sources_found, const CVideoRelayOutParamsStore& pVideoMediaParam, std::list<const CImage*> & listImagesToSend, std::map<DWORD,CVideoRelaySourceApi>& mapPreviousVideoSourcesSent, CVideoRelaySourcesParams& rResultVideoSourcesRequest, CScpNotificationWrapper& rResultVideoScpNotificationRequest);
	void FillStreamsAccordingToPriority(WORD & num_of_video_sources_found, const CVideoRelayOutParamsStore& pVideoMediaParam, std::list<const CImage*> & listImagesToSend, std::map<DWORD,CVideoRelaySourceApi>& mapPreviousVideoSourcesSent, CVideoRelaySourcesParams& rResultVideoSourcesRequest, CScpNotificationWrapper& rResultVideoScpNotificationRequest);
	void FillStreamsAccordingToResolution(WORD & num_of_video_sources_found, const CVideoRelayOutParamsStore& pVideoMediaParam, std::list<const CImage*> & listImagesToSend, std::map<DWORD,CVideoRelaySourceApi>& mapPreviousVideoSourcesSent, CVideoRelaySourcesParams& rResultVideoSourcesRequest, CScpNotificationWrapper& rResultVideoScpNotificationRequest);
	void FillStreamsForAllStreamsWithSameKey(WORD & numOfVideoSourcesFound, int key, WORD &countTheLayerKey,WORD &countTheLayerLeft,const CVideoRelayOutParamsStore& pVideoMediaParam,std::list<const CImage*> & listImagesToSend, std::map<DWORD,CVideoRelaySourceApi>& mapPreviousVideoSourcesSent, CVideoRelaySourcesParams& rResultVideoSourcesRequest,CScpNotificationWrapper& rResultVideoScpNotificationRequest );
	void UpdateNotificationsOnAllRequestedStreamsCantBeProvided(CVideoRelayOutParamsStore& rVideoMediaParam /*std::list<CRelayMediaStream*> requestedMediaStream*/ ,CScpNotificationWrapper& rResultVideoScpNotificationRequest);


    bool IsPartyVideoSourcesRequestsEqual(CVideoRelaySourcesParams& rFisrtPartyVideoSourcesRequest, CVideoRelaySourcesParams& rSecondPartyVideoSourcesRequest);
    bool CompareUpdateVideoScpNotificationRequest(CScpNotificationWrapper& rFisrt, CScpNotificationWrapper& rSecond);
	void FillScpPipe( const CImage* pCurrentImageToSend, CScpPipeWrapper& rScpPipe, bool isSliseState = false);
	bool CompareUpdateOnSeenImageStruct(UpdateOnSeenImageStruct first,UpdateOnSeenImageStruct second );
	void UpdateOnNoImageSeen(UpdateOnSeenImageStruct & rUpdateOnImageStruct);





	// attributes
	const CVideoBridgeCP* m_pVideoBridge;
	const CVideoBridgePartyCntl* m_pCVideoBridgePartyCntl;

};

#endif //* VIDEO_OUT_STREAMS_HANDLER_H_ */
