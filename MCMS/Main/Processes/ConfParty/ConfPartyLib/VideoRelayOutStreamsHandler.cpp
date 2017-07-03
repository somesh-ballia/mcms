#include <vector>
#include "SipUtils.h"
#include "ConfPartyGlobals.h"
#include "VideoRelayOutStreamsHandler.h"


//#include "PartyVideoSourcesRequest.h"
//#include "LogHelper.h"
//#include "VideoOutParamsStore.h"



////////////////////////////////////////////////////////
CVideoRelayOutParamsStore::CVideoRelayOutParamsStore()
{
    m_pCurrentVideoSourcesRequest = new CVideoRelaySourcesParams();
    m_pCurrentVideoSourcesRequest->InitDefaults();

    m_pCurrentMediaParam = new CBridgePartyVideoRelayMediaParams();
//    m_pCurrentMediaParam->InitDefaults();

    m_pCurrentVideoScpNotificationRequest = new CScpNotificationWrapper;
    m_pCurrentVideoScpNotificationRequest->InitDefaults();

    m_needToSendScpStreamsNotification = false;
    m_isStreamsPriorityAccordingToPriorityFlag = false;
    //m_pipeIdForIvrSlide = 0;

    InitAllIvrSlideParams();
    InitUpdateOnSeenImageStruct();
}

////////////////////////////////////////////////////////
CVideoRelayOutParamsStore::~CVideoRelayOutParamsStore()
{
    POBJDELETE(m_pCurrentMediaParam);
    POBJDELETE(m_pCurrentVideoSourcesRequest);
    POBJDELETE(m_pCurrentVideoScpNotificationRequest);
}

////////////////////////////////////////////////////////
void CVideoRelayOutParamsStore::Init (const CBridgePartyVideoRelayMediaParams* newBridgePartyVideoParams)
{
    if( !newBridgePartyVideoParams )
        return;

    *m_pCurrentMediaParam = *newBridgePartyVideoParams;


/*	CRelayMediaStream* pRelayMediaStream = m_pCurrentMediaParam->m_pStreamsList.front();
	CVideoRelayOutMediaStream* pIvrSlideVideoRelayOutMediaStream = (CVideoRelayOutMediaStream*)pRelayMediaStream;
	m_pCurrentMediaParam->SetIvrSlideStreamParams(pIvrSlideVideoRelayOutMediaStream);*/

    std::ostringstream ostr;
    ostr << "CVideoRelayOutParamsStore::Init :   channel = " << m_pCurrentMediaParam->GetChannelHandle();
    std::list<CRelayMediaStream *>::const_iterator itr_list = m_pCurrentMediaParam->m_pStreamsList.begin();
    for ( ; itr_list != m_pCurrentMediaParam->m_pStreamsList.end(); ++itr_list)
    {
        CVideoRelayOutMediaStream* pVideoOutMediaStream = (CVideoRelayOutMediaStream*)(*itr_list);
        ostr << "\n layer=" << pVideoOutMediaStream->GetLayerId() << ", pipe=" << pVideoOutMediaStream->GetSsrc()<< ", priority=" << pVideoOutMediaStream->GetPriority() << ", GetIsSpecificSourceCsrc:"<<((DWORD)pVideoOutMediaStream->GetIsSpecificSourceCsrc())<< " CSRC=" << pVideoOutMediaStream->GetCsrc();
    }
    PTRACE(eLevelInfoHigh,ostr.str().c_str());


    m_videoStreamsSortedByResolutionOrPriority.clear();
    m_forceSpecVideoStreamsByLayerId.clear();

    UpdateIsPrioritySetAccordingToPriorityFlag();


    DWORD prevRemoteSeq = m_pCurrentVideoScpNotificationRequest->m_remoteSequenceNumber;
    if( 0 == newBridgePartyVideoParams->m_scpRequestSequenceNumber || newBridgePartyVideoParams->m_scpRequestSequenceNumber != prevRemoteSeq )
    {
        //each ScpStreamsNotification message obsoletes all previous ScpStreamsNotification which were sent
    	m_pCurrentVideoScpNotificationRequest->InitDefaults();
        m_needToSendScpStreamsNotification = false;
    }
    std::list<CRelayMediaStream *>::const_iterator itr_streamsList = m_pCurrentMediaParam->m_pStreamsList.begin();
    for ( ; itr_streamsList != m_pCurrentMediaParam->m_pStreamsList.end(); ++itr_streamsList)
    {
        CVideoRelayOutMediaStream* pVideoOutMediaStream = (CVideoRelayOutMediaStream*)(*itr_streamsList);
        CVideoRelayOutMediaStream  aVideoOutMediaStream(*pVideoOutMediaStream);

        if( pVideoOutMediaStream->GetIsSpecificSourceCsrc() )
        {
        	TRACEINTO << " - GetIsSpecificSourceCsrc=true";
            m_forceSpecVideoStreamsByLayerId.insert( std::pair<int, CVideoRelayOutMediaStream>(pVideoOutMediaStream->GetLayerId(), aVideoOutMediaStream) );
        }
        else
        {
        	if(m_isStreamsPriorityAccordingToPriorityFlag)
        	{
        		//TRACEINTO << " according to priority = true";
           		m_videoStreamsSortedByResolutionOrPriority.insert( std::pair<int, CVideoRelayOutMediaStream>(pVideoOutMediaStream->GetPriority(), aVideoOutMediaStream) );

        	}
        	else
        	{
        		//TRACEINTO << " according to priority = false";
        		m_videoStreamsSortedByResolutionOrPriority.insert( std::pair<int, CVideoRelayOutMediaStream>(pVideoOutMediaStream->GetResolutionHeight(), aVideoOutMediaStream) );
        	}
        }
    }

//    static int countSCP = 0; //TEMP - to check SCP notify
//    if( countSCP > 1 )
//    {
//    	DWORD ssrc = 111111;
//        std::list<CRelayMediaStream *>::const_iterator itr_streamsList = m_pCurrentMediaParam->m_pStreamsList.begin();
//        CVideoRelayOutMediaStream* pVideoOutMediaStream = (CVideoRelayOutMediaStream*)(*itr_streamsList);
//        if( pVideoOutMediaStream )
//        {
//			CVideoRelayOutMediaStream aVideoOutMediaStream (*pVideoOutMediaStream);
//			aVideoOutMediaStream.SetSsrc(ssrc);
//			m_videoStreamsSortedByResolutionOrPriority.insert( std::pair<int, CVideoRelayOutMediaStream>(aVideoOutMediaStream.GetResolutionHeight(), aVideoOutMediaStream) );
//			ssrc += 100;
//        }
//    }
//    countSCP++;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayOutParamsStore::UpdateIsPrioritySetAccordingToPriorityFlag()
{
	bool areAllStreamsPrioritySetToZero = true;
	std::list<CRelayMediaStream *>::const_iterator itr_list = m_pCurrentMediaParam->m_pStreamsList.begin();
	for ( ; itr_list != m_pCurrentMediaParam->m_pStreamsList.end(); ++itr_list)
	{
	     CVideoRelayOutMediaStream* pVideoOutMediaStream = (CVideoRelayOutMediaStream*)(*itr_list);
	     if(pVideoOutMediaStream->GetPriority()!=0)
	     {
//	    	 TRACEINTO << "CVideoRelayOutParamsStore::UpdateIsPrioritySetAccordingToPriorityFlag : areAllStreamsPrioritySetToZero = false; ssrc: "<<  pVideoOutMediaStream->GetSsrc();
	    	 areAllStreamsPrioritySetToZero = false;
	    	 break;
	     }
	}
	if(areAllStreamsPrioritySetToZero)
	{
		m_isStreamsPriorityAccordingToPriorityFlag = false;
	}
	else
		m_isStreamsPriorityAccordingToPriorityFlag = true;

	TRACEINTO << "m_isStreamsPriorityAccordingToPriorityFlag = " << (WORD)m_isStreamsPriorityAccordingToPriorityFlag;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayOutParamsStore::InitAllIvrSlideParams()
{
	m_bIsAckOnIvrScpShowSlideReceived = false;
	m_bIsAckOnIvrScpStopSlideReceived = false;
	m_bIsConnectDuringStopSlideState = false;
	m_bIsShowSlideToHardwareSent = false;
	m_bIsSCPShowSlideReqCanBeSent = false;
	m_showSlideSCPSeqNumber = 0;

}
void CVideoRelayOutParamsStore::InitUpdateOnSeenImageStruct()
{
	m_updateOnSeenImageStruct.bIsNeedToSendUpdate = false;
	m_updateOnSeenImageStruct.idOfPartyToUpdate = INVALID;
	m_updateOnSeenImageStruct.idOfSeenParty = INVALID;

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CVideoRelayOutStreamsHandler::CVideoRelayOutStreamsHandler(const CVideoBridgeCP* pVideoBridge, const CVideoRelayBridgePartyCntl* pCVideoBridgePartyCntl)
{
    m_pVideoBridge = pVideoBridge;
    m_pCVideoBridgePartyCntl = pCVideoBridgePartyCntl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVideoRelayOutStreamsHandler::~CVideoRelayOutStreamsHandler()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVideoRelayOutStreamsHandler::FillVideoSource(CVideoRelayOutMediaStream& rVideoOutMediaStream,
                                              const CImage* pImage,
                                              CVideoRelaySourcesParams& rResultVideoSourcesRequest, bool cantHaveSameSource)
{
    bool succeeded = true;

    // ssrc and layer id - from first image, regarding operation points
    int requestedLayerId = rVideoOutMediaStream.GetLayerId();//m_layerId;
    unsigned int resultSsrc = 0;
    int resultLayerId = -1;

    const CVideoOperationPointsSet* pConfOperationPointsSet = m_pVideoBridge->GetConfVideoOperationPointsSet();

    succeeded = GetBestSsrcAndLayerIdFromImage(requestedLayerId, pImage, resultSsrc, resultLayerId, pConfOperationPointsSet);
    DWORD resultTid = GetTidFromLayerId(resultLayerId, pConfOperationPointsSet);

    if(resultSsrc != 0 && resultLayerId != -1)
    {
    	bool sourceAlreadyInRequest = false;
    	if(cantHaveSameSource)
    	{
    		sourceAlreadyInRequest = rResultVideoSourcesRequest.IsVideoRelaySourcesHasSource(resultSsrc);
    	}
    	if((! cantHaveSameSource) || (cantHaveSameSource && !sourceAlreadyInRequest))
        {
        CVideoRelaySourceApi rVideoSourceApi;
        rVideoSourceApi.SetChannelHandle( pImage->GetVideoRelayInChannelHandle() );;
        rVideoSourceApi.SetSyncSource( resultSsrc ); // from CImage (relevant VideoMediaStream m_ssrc)
        rVideoSourceApi.SetLayerId( resultLayerId ); // from CImage (relevant VideoMediaStream m_layerId)
        rVideoSourceApi.SetPipeId( rVideoOutMediaStream.GetSsrc() ); // pipe id - from video media streams request - from BridgePartyVideoOut -> BridgePartyVideoParams -> MediaStream m_ssrc
        rVideoSourceApi.SetIsSpeaker( IsSpeakerImage(pImage) );
        rVideoSourceApi.SetTid(resultTid);

        rResultVideoSourcesRequest.GetVideoSourcesList().push_back(rVideoSourceApi);
        }
    	else
    	{
    		succeeded = false;
    		if(cantHaveSameSource && sourceAlreadyInRequest)
    			TRACEINTO << "cantHaveSameSource and the source is already in  the request" << resultSsrc;
    	}
    }
    else
    {
        TRACEINTO << "failed to get ssrc and layer id from image, requestedLayerId:"<< requestedLayerId;
    }

    return succeeded;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVideoRelayOutStreamsHandler::IsSpeakerImage(const CImage* pImage)
{
	DWORD activeAudioSpeakerId = (m_pVideoBridge) ? m_pVideoBridge->GetLastActiveAudioSpeakerId() : INVALID;
    if(pImage && (INVALID != activeAudioSpeakerId) && (activeAudioSpeakerId == pImage->GetPartyRsrcId()))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVideoRelayOutStreamsHandler::BuildVideoSourcesRequest( CVideoRelayOutParamsStore& pVideoMediaParam, AskForRelayIntra& rEpIntraParams)
{
    bool succeeded = true;
    bool isNewRequestNeedToBeEmpty = false;

    //1. save the prev request & notification
    CVideoRelaySourcesParams prevVideoSourcesRequest ( *pVideoMediaParam.m_pCurrentVideoSourcesRequest );
    CScpNotificationWrapper prevVideoScpNotification = *pVideoMediaParam.m_pCurrentVideoScpNotificationRequest;
    UpdateOnSeenImageStruct  prevUpdateOnImageStruct = pVideoMediaParam.m_updateOnSeenImageStruct;

    //2. validity check
    int prevRequestNumOfStream = prevVideoSourcesRequest.GetVideoSourcesList().size();
    if(NULL == pVideoMediaParam.m_pCurrentMediaParam )
    {
        PASSERTMSG(1,"pMediaParam is NUll");
        isNewRequestNeedToBeEmpty = true;
        return false;
    }

    int requested_num_of_streams = pVideoMediaParam.m_pCurrentMediaParam->m_pStreamsList.size();
    if(requested_num_of_streams == 0)
    {
    	TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : requested_num_of_streams is 0";
        isNewRequestNeedToBeEmpty = true;
    }

    // check if all streams are illegal (layer_id==-1)
    // Streams can have layer_id=-1 if the EP sent illegal values or if there is not enough bit rate
    // to display all the streams.
    bool areAllStreamsIllegal = true;
    std::list<CRelayMediaStream *>::const_iterator itr_list = pVideoMediaParam.m_pCurrentMediaParam->m_pStreamsList.begin();
    for ( ; itr_list != pVideoMediaParam.m_pCurrentMediaParam->m_pStreamsList.end(); ++itr_list)
    {
        CVideoRelayOutMediaStream* pVideoOutMediaStream = (CVideoRelayOutMediaStream*)(*itr_list);
        if (pVideoOutMediaStream->GetLayerId() >= 0)
        {
            areAllStreamsIllegal = false;
            break;
        }
    }
    if (areAllStreamsIllegal)
    {
        TRACEINTO << "DEBUG_SCP : all streams are illegal.";
        isNewRequestNeedToBeEmpty = true;//@@@
    }

    //choose valid images according to speaker order
    std::list<const CImage*> listImagesToSend;
    bool isCascadeLink = pVideoMediaParam.m_pCurrentMediaParam->m_bIsCascadeLink;
    m_pVideoBridge->GetValidRelayImagesForParty( m_pCVideoBridgePartyCntl->GetPartyRsrcID(), listImagesToSend, isCascadeLink );
    DWORD num_of_images_in_video_bridge = listImagesToSend.size();

    if(num_of_images_in_video_bridge == 0)
    {
        TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : num_of_images_in_video_bridge is 0";
        isNewRequestNeedToBeEmpty = true;
    }

//    if(isSlideState)
//    {
//    	isNewRequestNeedToBeEmpty = true;
//    	TRACEINTOFUNC << " isSlideState = " << (WORD)isSlideState;
//    }
    if (isNewRequestNeedToBeEmpty)
    {
    	if (prevRequestNumOfStream>0 || isCascadeLink)
        {
        	TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : prev send request wasn't empty we will send an empty request to MRMP via Party";
            CreateEmptyRequest( *pVideoMediaParam.m_pCurrentVideoSourcesRequest, pVideoMediaParam.m_pCurrentMediaParam->m_channelHandle );

            if (((num_of_images_in_video_bridge==0 ) && (requested_num_of_streams!=0 || isCascadeLink)) || areAllStreamsIllegal)
            {

            	UpdateNotificationsOnAllRequestedStreamsCantBeProvided(pVideoMediaParam,*pVideoMediaParam.m_pCurrentVideoScpNotificationRequest);

            	// Compare between previous VideoScpNotificationRequest and the current
				bool isEqual = CompareUpdateVideoScpNotificationRequest( prevVideoScpNotification, *pVideoMediaParam.m_pCurrentVideoScpNotificationRequest );
				if( isEqual )
				{
					*pVideoMediaParam.m_pCurrentVideoScpNotificationRequest = prevVideoScpNotification;
					pVideoMediaParam.m_needToSendScpStreamsNotification = false;
				}
				else
				{
					PTRACE(eLevelInfoNormal, "CVideoRelayOutStreamsHandler::BuildVideoSourcesRequest need to send notification on all unavailable streams ");
					pVideoMediaParam.m_needToSendScpStreamsNotification = true;
				}

            }
            UpdateOnNoImageSeen(pVideoMediaParam.m_updateOnSeenImageStruct);

            return true;
        }
        else//no need to send any update to MRMP
        {
            return false;
        }
    }


    //3. Fill the VideoSourcesRequest
    succeeded = FillVideoSourcesRequest( pVideoMediaParam, *pVideoMediaParam.m_pCurrentVideoSourcesRequest, *pVideoMediaParam.m_pCurrentVideoScpNotificationRequest, pVideoMediaParam.m_updateOnSeenImageStruct );


    bool needToSendVideoSourcesRequest = succeeded;
    //4. Compare between prevVideoSourcesRequest and current rResultVideoSourcesRequest
        // if both prevVideoSourcesRequest and rResultVideoSourcesRequest are the same then no changes happened,
        //no need to send change streams request- > succeeded = false
    if( needToSendVideoSourcesRequest && IsPartyVideoSourcesRequestsEqual(prevVideoSourcesRequest, *pVideoMediaParam.m_pCurrentVideoSourcesRequest) )
    {
        needToSendVideoSourcesRequest = false;
    }
    if( !needToSendVideoSourcesRequest )
    {
        //If we wont send a request we need to save the prev. request
        *pVideoMediaParam.m_pCurrentVideoSourcesRequest = prevVideoSourcesRequest;
        TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : Didn't create new request save again the prev. sent request";
    }
    else
    {
        FillEpIntaParams(prevVideoSourcesRequest, *pVideoMediaParam.m_pCurrentVideoSourcesRequest, rEpIntraParams);
    }

    //5. Compare between previous VideoScpNotificationRequest and the current
    bool isEqual = CompareUpdateVideoScpNotificationRequest( prevVideoScpNotification, *pVideoMediaParam.m_pCurrentVideoScpNotificationRequest );
    if( isEqual )
    {
        *pVideoMediaParam.m_pCurrentVideoScpNotificationRequest = prevVideoScpNotification;
        pVideoMediaParam.m_needToSendScpStreamsNotification = false;
    }
    else
    {
        pVideoMediaParam.m_needToSendScpStreamsNotification = true;
    }
    isEqual = CompareUpdateOnSeenImageStruct(prevUpdateOnImageStruct,pVideoMediaParam.m_updateOnSeenImageStruct );
    if(isEqual)
    {
    	pVideoMediaParam.m_updateOnSeenImageStruct.bIsNeedToSendUpdate = false;
    	TRACEINTO<<" Update on Image struct didnt change no need to send";
    }

    return needToSendVideoSourcesRequest;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayOutStreamsHandler::CreateEmptyRequest(CVideoRelaySourcesParams& rResultVideoSourcesRequest, int channelHandle)
{
    // Clear the new request
    rResultVideoSourcesRequest.InitDefaults();


    //Fill request general params
    rResultVideoSourcesRequest.SetChannelHandle( channelHandle );

    // sequence number - party should fill this field
    rResultVideoSourcesRequest.SetSeqNum(-1);

    // operation point set id (conf id) - from video bridge operation points set
    int confId = -1;
    confId = m_pCVideoBridgePartyCntl->GetConfRsrcID();
    rResultVideoSourcesRequest.SetSourceOperationPointSetId(confId);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//void CVideoRelayOutStreamsHandler::UpdateNotificationsOnAllRequestedStreamsCantBeProvided(std::list<CRelayMediaStream*> requestedMediaStream,CScpNotificationWrapper& rResultVideoScpNotificationRequest, bool isSlideState)
//{
////	PTRACE(eLevelInfoNormal, "CVideoRelayOutStreamsHandler::UpdateNotificationsOnAllRequestedStreamsCantBeProvided ");
//	std::list<CRelayMediaStream *>::const_iterator itr_list = requestedMediaStream.begin();
//	for ( ; itr_list != requestedMediaStream.end(); ++itr_list)
//	{
//	        CVideoRelayOutMediaStream* pVideoOutMediaStream = (CVideoRelayOutMediaStream*)(*itr_list);
//	        CScpPipeWrapper scpPipe;
//            scpPipe.m_pipeId = pVideoOutMediaStream->GetSsrc();
//            FillScpPipe( NULL, scpPipe, isSlideState);
//            rResultVideoScpNotificationRequest.AddPipe(scpPipe);
//	}
//
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayOutStreamsHandler::UpdateNotificationsOnAllRequestedStreamsCantBeProvided(CVideoRelayOutParamsStore& rVideoMediaParam /*std::list<CRelayMediaStream*> requestedMediaStream*/ ,CScpNotificationWrapper& rResultVideoScpNotificationRequest)
{

	std::list<CRelayMediaStream*> requestedMediaStream = rVideoMediaParam.m_pCurrentMediaParam->GetRelayMediaStream();
	// clean the new scp notification request and fill its general params
	rResultVideoScpNotificationRequest.InitDefaults();
	rResultVideoScpNotificationRequest.m_channelHandle = rVideoMediaParam.m_pCurrentMediaParam->m_channelHandle;
	rResultVideoScpNotificationRequest.m_remoteSequenceNumber = rVideoMediaParam.m_pCurrentMediaParam->m_scpRequestSequenceNumber;

	//            PTRACE(eLevelInfoNormal, "CVideoRelayOutStreamsHandler::UpdateNotificationsOnAllRequestedStreamsCantBeProvided ");
	std::list<CRelayMediaStream *>::const_iterator itr_list = requestedMediaStream.begin();
	for ( ; itr_list != requestedMediaStream.end(); ++itr_list)
	{
		CVideoRelayOutMediaStream* pVideoOutMediaStream = (CVideoRelayOutMediaStream*)(*itr_list);
		CScpPipeWrapper scpPipe;
		scpPipe.m_pipeId = pVideoOutMediaStream->GetSsrc();
		FillScpPipe( NULL, scpPipe);
		rResultVideoScpNotificationRequest.AddPipe(scpPipe);
	}

}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CVideoRelayOutStreamsHandler::FillStreamsAccordingToResolutionOrPriority(WORD & numOfVideoSourcesFound, const CVideoRelayOutParamsStore& pVideoMediaParam, std::list<const CImage*> & listImagesToSend, std::map<DWORD, CVideoRelaySourceApi>& mapPreviousVideoSourcesSent, CVideoRelaySourcesParams& rResultVideoSourcesRequest, CScpNotificationWrapper& rResultVideoScpNotificationRequest)
{
	if(pVideoMediaParam.GetIsStreamsPriorityAccordingToPriorityFlag())
	{
		FillStreamsAccordingToPriority(numOfVideoSourcesFound,pVideoMediaParam,listImagesToSend, mapPreviousVideoSourcesSent, rResultVideoSourcesRequest, rResultVideoScpNotificationRequest);

	}
	else
	{
		FillStreamsAccordingToResolution(numOfVideoSourcesFound,pVideoMediaParam,listImagesToSend, mapPreviousVideoSourcesSent, rResultVideoSourcesRequest, rResultVideoScpNotificationRequest);

	}

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVideoRelayOutStreamsHandler::FillStreamsForAllStreamsWithSameKey(WORD & numOfVideoSourcesFound, int key, WORD &counterTheKey,WORD &countTheKeyLeft,const CVideoRelayOutParamsStore& pVideoMediaParam,std::list<const CImage*> & listImagesToSend, std::map<DWORD,CVideoRelaySourceApi>& mapPreviousVideoSourcesSent, CVideoRelaySourcesParams& rResultVideoSourcesRequest, CScpNotificationWrapper& rResultVideoScpNotificationRequest )
{
//	TRACEINTO << "Key = " << key << ", counterTheKey = " << counterTheKey;//TMP for debug
	std::pair <std::multimap<int, CVideoRelayOutMediaStream>::const_iterator,
			   std::multimap<int, CVideoRelayOutMediaStream>::const_iterator> keyRange = pVideoMediaParam.m_videoStreamsSortedByResolutionOrPriority.equal_range(key);

	std::multimap<int, CVideoRelayOutMediaStream>::const_iterator theLayerKeyIter;
    for (theLayerKeyIter = keyRange.first; theLayerKeyIter != keyRange.second;  ++theLayerKeyIter)//for all the CVideoRelayOutMediaStream with the same key priority(according to resolution or scp priority)
	{
    	CVideoRelayOutMediaStream aVideoOutMediaStream = (*theLayerKeyIter).second;

    	const CImage* pCurrentImageToSend = NULL, *pSkipImage = NULL;
        if( aVideoOutMediaStream.GetLayerId() >= 0 ) //If EP asks for not legal OP, then PartyControl will send layer=-1 => add to notification request
        {
        	int new_pipe_id = aVideoOutMediaStream.GetSsrc();
            if( aVideoOutMediaStream.GetCsrc() > 0 )
            	TRACEINTO << "WARNING: force csrc = " << aVideoOutMediaStream.GetCsrc();

            WORD image_index = 0;

            std::list<const CImage*>::iterator listImagesToSendIter = listImagesToSend.begin(), freeImageIter = listImagesToSend.end();

            //If number of the streams with the same priority = x try to find in the x (or less if number of images is less) if that image was already been assigned to that pipe in prev request.
            while( listImagesToSendIter != listImagesToSend.end() && (image_index<countTheKeyLeft) )
            {
                  const CImage* pCurrentImage = (*listImagesToSendIter);
                  DWORD channel = pCurrentImage->GetVideoRelayInChannelHandle();

                  std::map<DWORD, CVideoRelaySourceApi>::iterator previousVideoSourcesSentIter = mapPreviousVideoSourcesSent.find( channel );//try to find if image was in the prev video relay sources
		          if( previousVideoSourcesSentIter != mapPreviousVideoSourcesSent.end() )
		          {
		        	  int prev_pipe_id = previousVideoSourcesSentIter->second.GetPipeId();
		        	  TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : (1) found in previous video sources , key = " << key <<  ", prev_pipe_id = " << prev_pipe_id << ", new_pipe_id = " << new_pipe_id;//TMP for debug
		              if( new_pipe_id == prev_pipe_id )
		              {
		            	  pCurrentImageToSend = pCurrentImage;
		                  TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : (2) found in previous video sources , prev_pipe_id == new_pipe_id = " << prev_pipe_id << ", party_id = " << pCurrentImage->GetPartyRsrcId();//TMP for debug
		                  listImagesToSend.erase( listImagesToSendIter );
		                  break;
		               }
		               else
		               {

							std::multimap<int, CVideoRelayOutMediaStream>::const_iterator theLayerKeyIterNext = theLayerKeyIter;
							//maybe the pipeid of the stream we try to fill is in another stream with same priority, we will skip this image, in the next iteration of main loop we will handle it
							for( ++theLayerKeyIterNext; theLayerKeyIterNext != keyRange.second;  ++theLayerKeyIterNext )
							{
								CVideoRelayOutMediaStream aVideoOutMediaStreamNext = (*theLayerKeyIterNext).second;
								int new_pipe_id_next = aVideoOutMediaStreamNext.GetSsrc();
								if( new_pipe_id_next == prev_pipe_id )
								{
									TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : (3) skip this image because it found in previous sent request , prev_pipe_id == new_pipe_id_next = " << prev_pipe_id << ", party_id = " << pCurrentImage->GetPartyRsrcId();//TMP for debug
									pSkipImage = pCurrentImage;
									break;
								}
							}
						}
					}

		          if( freeImageIter == listImagesToSend.end() || (*freeImageIter) == pSkipImage )
						freeImageIter = listImagesToSendIter;

		          listImagesToSendIter++;
		          image_index++;
			}

			if( !pCurrentImageToSend && listImagesToSend.begin() != listImagesToSend.end() )
			{
				if( pSkipImage && freeImageIter != listImagesToSend.end() )
				{
					listImagesToSendIter = freeImageIter;
					pCurrentImageToSend = *freeImageIter;
				}
				else
				{
					listImagesToSendIter = listImagesToSend.begin();
					pCurrentImageToSend = *listImagesToSendIter;
				}
				TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : use the image, party_id = " << pCurrentImageToSend->GetPartyRsrcId()
						<< ", channelHandle = " << pCurrentImageToSend->GetVideoRelayInChannelHandle();   //TMP for debug

				listImagesToSend.erase( listImagesToSendIter );
			}
		}

		bool image_found = false;
		if( pCurrentImageToSend )
		{
			bool cantHaveSameSource = false;
			image_found = FillVideoSource( aVideoOutMediaStream, pCurrentImageToSend, rResultVideoSourcesRequest,cantHaveSameSource);
		}

		if( image_found )
			numOfVideoSourcesFound++;
		else
		{
			CScpPipeWrapper scpPipe;
			scpPipe.m_pipeId = aVideoOutMediaStream.GetSsrc();
			FillScpPipe( pCurrentImageToSend, scpPipe );

			rResultVideoScpNotificationRequest.AddPipe(scpPipe);
//			TRACEINTO << "DEBUG_SCP : add to CScpNotificationWrapper : pipeId = " << scpPipe.m_pipeId << ", key = " << key;//TMP for debug
		}

		countTheKeyLeft--;
	}

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CVideoRelayOutStreamsHandler::FillStreamsAccordingToPriority(WORD & numOfVideoSourcesFound, const CVideoRelayOutParamsStore& pVideoMediaParam, std::list<const CImage*> & listImagesToSend, std::map<DWORD,CVideoRelaySourceApi>& mapPreviousVideoSourcesSent, CVideoRelaySourcesParams& rResultVideoSourcesRequest, CScpNotificationWrapper& rResultVideoScpNotificationRequest)
{
	TRACESTRFUNC(eLevelDebug) << "CVideoRelayOutStreamsHandler::FillStreamsAccordingToPriority";

	 //look for streams to send: we need to begin from end of the map because it is sorted by priority, so the highest priority is at the end
	 std::multimap<int, CVideoRelayOutMediaStream>::const_reverse_iterator  itr_streamsMap = pVideoMediaParam.m_videoStreamsSortedByResolutionOrPriority.rbegin();
	 while ( itr_streamsMap != pVideoMediaParam.m_videoStreamsSortedByResolutionOrPriority.rend() )
	 {
		 int priorityKey = (*itr_streamsMap).first;
		 WORD countTheKey  = pVideoMediaParam.m_videoStreamsSortedByResolutionOrPriority.count(priorityKey);
		 WORD countTheKeyLeft = countTheKey;

		FillStreamsForAllStreamsWithSameKey(numOfVideoSourcesFound, priorityKey, countTheKey,countTheKeyLeft, pVideoMediaParam, listImagesToSend, mapPreviousVideoSourcesSent, rResultVideoSourcesRequest, rResultVideoScpNotificationRequest );
	    WORD i = 0;
	    while( i<countTheKey )
	    {
	           itr_streamsMap++;// move the iterator to the next layer
	           i++;
	    }
	 }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CVideoRelayOutStreamsHandler::FillStreamsAccordingToResolution(WORD & numOfVideoSourcesFound, const CVideoRelayOutParamsStore& pVideoMediaParam, std::list<const CImage*> & listImagesToSend, std::map<DWORD,CVideoRelaySourceApi>& mapPreviousVideoSourcesSent, CVideoRelaySourcesParams& rResultVideoSourcesRequest, CScpNotificationWrapper& rResultVideoScpNotificationRequest)
{
	PTRACE(eLevelInfoHigh, "CVideoRelayOutStreamsHandler::FillStreamsAccordingToResolution" );
	//look for streams to send: we need to begin from end of the map because it is sorted by resolution, so the highest resolution is at the end
	std::multimap<int, CVideoRelayOutMediaStream>::const_reverse_iterator  itr_streamsMap = pVideoMediaParam.m_videoStreamsSortedByResolutionOrPriority.rbegin();
	while ( itr_streamsMap != pVideoMediaParam.m_videoStreamsSortedByResolutionOrPriority.rend() )
	{
		int resolutionHeightKey = (*itr_streamsMap).first;
		WORD countTheKey  = pVideoMediaParam.m_videoStreamsSortedByResolutionOrPriority.count(resolutionHeightKey);
		WORD countTheKeyLeft = countTheKey;

		FillStreamsForAllStreamsWithSameKey(numOfVideoSourcesFound, resolutionHeightKey, countTheKey,countTheKeyLeft, pVideoMediaParam, listImagesToSend, mapPreviousVideoSourcesSent, rResultVideoSourcesRequest, rResultVideoScpNotificationRequest );
        WORD i = 0;
	    while( i<countTheKey )
	    {
	           itr_streamsMap++;// move the iterator to the next layer
	           i++;
	     }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVideoRelayOutStreamsHandler::FillVideoSourcesRequest( const CVideoRelayOutParamsStore& pVideoMediaParam,
                                                            CVideoRelaySourcesParams& rResultVideoSourcesRequest,
                                                            CScpNotificationWrapper& rResultVideoScpNotificationRequest, UpdateOnSeenImageStruct& resultUpdateOnImageStruct)
{
    bool succeeded = true;

    CVideoRelaySourcesParams prevVideoSourcesRequest(rResultVideoSourcesRequest);

    std::map<DWORD, CVideoRelaySourceApi> mapPreviousVideoSourcesSent;    // sort the previous video sources according to the channelHandle
    std::list<CVideoRelaySourceApi>::iterator itr_videoSources = prevVideoSourcesRequest.GetVideoSourcesList().begin();
    while( itr_videoSources != prevVideoSourcesRequest.GetVideoSourcesList().end() )
    {
        mapPreviousVideoSourcesSent[(*itr_videoSources).GetChannelHandle()] = (*itr_videoSources);
        itr_videoSources++;
    }

    // clean the new scp notification request and fill its general params
    rResultVideoScpNotificationRequest.InitDefaults();
    rResultVideoScpNotificationRequest.m_channelHandle = pVideoMediaParam.m_pCurrentMediaParam->m_channelHandle;
    rResultVideoScpNotificationRequest.m_remoteSequenceNumber = pVideoMediaParam.m_pCurrentMediaParam->m_scpRequestSequenceNumber;

    // clear the new party video request and fill general params
    rResultVideoSourcesRequest.InitDefaults();
    // channel handle - from BridgePartyVideoOut
    rResultVideoSourcesRequest.SetChannelHandle( pVideoMediaParam.m_pCurrentMediaParam->m_channelHandle );
    // sequence number - party should fill this field
    rResultVideoSourcesRequest.SetSeqNum(-1);
    // operation point set id (conf id) - from video bridge operation points set
    rResultVideoSourcesRequest.SetSourceOperationPointSetId( m_pCVideoBridgePartyCntl->GetConfRsrcID() );

    WORD num_of_video_sources_found = 0;
    const CImage* pCurrentImage = NULL;

    int num_of_images_in_video_bridge = m_pVideoBridge->GetPartyImageVectorSize();
//    const CImage* pSelfImage = m_pCVideoBridgePartyCntl->GetPartyImage();

    std::list<const CImage*> listImagesToSend;

    //choose valid images according to speaker order
    PartyRsrcID  partyRsrcId = m_pCVideoBridgePartyCntl->GetPartyRsrcID();
    bool isCascadeLink = pVideoMediaParam.m_pCurrentMediaParam->m_bIsCascadeLink;
    m_pVideoBridge->GetValidRelayImagesForParty( partyRsrcId, listImagesToSend, isCascadeLink );

    const CVideoOperationPointsSet* pConfOperationPointsSet = m_pVideoBridge->GetConfVideoOperationPointsSet();

    if(isCascadeLink)
    {

    	succeeded = FillVideoSourcesRequestForCascadeLink(pVideoMediaParam, rResultVideoSourcesRequest,rResultVideoScpNotificationRequest, resultUpdateOnImageStruct, listImagesToSend, partyRsrcId);
    	return succeeded;
    }
    CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();

    // fill video streams
    std::multimap<int, CVideoRelayOutMediaStream>::const_iterator  itr_forceStreamMap = pVideoMediaParam.m_forceSpecVideoStreamsByLayerId.begin();
    for ( ; itr_forceStreamMap != pVideoMediaParam.m_forceSpecVideoStreamsByLayerId.end(); ++itr_forceStreamMap )
    {
    	CVideoRelayOutMediaStream aVideoOutMediaStream = (*itr_forceStreamMap).second;

        const CImage* pCurrentImageToSend = NULL;

        int theLayerKey = (*itr_forceStreamMap).first;
        if( theLayerKey >= 0 ) //If EP asks for not legal OP, then PartyControl will send layer=-1 => add to notification request
        {

            unsigned int force_csrc  = aVideoOutMediaStream.GetCsrc();
            TRACEINTO << "DEBUG_SCP : EP asks to receive a stream from a specific source : layerId = " << theLayerKey << ", csrc = " << force_csrc;

            //go over all images because MRE might ask to see the same specific image in several streams
            for( WORD image_index = 0; image_index < num_of_images_in_video_bridge; image_index++ )
            {
                DWORD partyRscId = m_pVideoBridge->GetPartyImageIdByPosition(image_index);
                if (!partyRscId)
                    continue;

                pCurrentImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
                PASSERTSTREAM(!pCurrentImage, "CVideoRelayOutStreamsHandler::FillVideoSourcesRequest - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

                if( !pCurrentImage || !pCurrentImage->IsVideoRelayImage())
                    continue;

                std::list <CVideoRelayInMediaStream*> pVideoMediaStreamsList = pCurrentImage->GetVideoRelayMediaStreamsList();
                bool bIsImageHasSSRC = false;
                // look for same layer id
                for( std::list <CVideoRelayInMediaStream*>::iterator itVideoMediaStream = pVideoMediaStreamsList.begin();
                        itVideoMediaStream != pVideoMediaStreamsList.end(); ++itVideoMediaStream )
                {
                    CVideoRelayInMediaStream* pVideoInMediaStream = *itVideoMediaStream;

                    // if the stream has the requested layer and csrc - finished
                    bool bIsSameResolutionLayerId = IsSameResolutionLayerId(theLayerKey, pVideoInMediaStream->GetLayerId(), pConfOperationPointsSet);
                    bool bIsStreamHasSSRC = false;
                    if(pVideoInMediaStream->GetSsrc() == force_csrc)
                    {
                    	bIsStreamHasSSRC = true;
                    	bIsImageHasSSRC = true;

                    }

                    if( (pVideoInMediaStream->GetLayerId() == theLayerKey || bIsSameResolutionLayerId) && bIsStreamHasSSRC)
                    {
                        pCurrentImageToSend = pCurrentImage;
                        TRACESTRFUNC(eLevelDebug) << "found the specific source : layerId = " << theLayerKey
                                   << ", csrc = " << force_csrc << ", party_id = " << pCurrentImage->GetPartyRsrcId();
                        listImagesToSend.remove( pCurrentImage ); //no need to send the image twice
                        break;
                    }

                }
                if(!pCurrentImageToSend && bIsImageHasSSRC)
                {
                	 unsigned int resultSsrc = 0;
                	 int resultLayerId = -1;
                	// const CVideoOperationPointsSet* pConfOperationPointsSet = m_pVideoBridge->GetConfVideoOperationPointsSet();
               	     bool bCanUseImage = GetBestSsrcAndLayerIdFromImage(theLayerKey, pCurrentImage, resultSsrc, resultLayerId, pConfOperationPointsSet);
                	 if(bCanUseImage && resultSsrc != 0 && resultLayerId != -1)
                	 {
                		pCurrentImageToSend = pCurrentImage;
                		 TRACEINTO << " found the specific source with same image but different ssrc : requested layerId = " << theLayerKey
                		           << ", requested csrc = " << force_csrc << ", party_id = " << pCurrentImage->GetPartyRsrcId()
                		           << ", actual ssrc: "<< resultSsrc <<", actual layer Id:"<< resultLayerId ;
                		listImagesToSend.remove( pCurrentImage ); //no need to send the image twice
                	}
                }
                if( pCurrentImageToSend )
                    break;
            }
        }
        bool image_found = false;
        if( pCurrentImageToSend && /*!pCurrentImageToSend->IsImageWithProblem() &&*/ !pCurrentImageToSend->isMuted() )
        {
        	bool cantHaveSameSource = false;
            image_found = FillVideoSource( aVideoOutMediaStream, pCurrentImageToSend, rResultVideoSourcesRequest, cantHaveSameSource);
        }
        if( image_found )
            num_of_video_sources_found++;
        else
        {   //In case the stream is muted/problematic/there is no stream with that ID, add SCP notification
            CScpPipeWrapper scpPipe;
            scpPipe.m_pipeId = aVideoOutMediaStream.GetSsrc();
            FillScpPipe( pCurrentImageToSend, scpPipe );

            rResultVideoScpNotificationRequest.AddPipe(scpPipe);
//            TRACEINTO << "DEBUG_SCP : add to CScpNotificationWrapper : pipeId = " << scpPipe.m_pipeId << ", layer = " << theLayerKey;//TMP for debug
        }
    }

    FillStreamsAccordingToResolutionOrPriority(num_of_video_sources_found, pVideoMediaParam, listImagesToSend, mapPreviousVideoSourcesSent, rResultVideoSourcesRequest, rResultVideoScpNotificationRequest);
    if( num_of_video_sources_found > 0 )
    {
        succeeded = true;
    }
    else
    {
        succeeded = false;
    }
    return succeeded;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVideoRelayOutStreamsHandler::FillVideoSourcesRequestForCascadeLink(const CVideoRelayOutParamsStore& pVideoMediaParam, CVideoRelaySourcesParams& rResultVideoSourcesRequest,CScpNotificationWrapper& rResultVideoScpNotificationRequest, UpdateOnSeenImageStruct& resultUpdateOnImageStruct, std::list<const CImage*> listImagesToSend, PartyRsrcID  partyRsrcId)
{
	TRACEINTO<< "party id = " << partyRsrcId;
	bool succeeded = true;
	//We only need one image.
	if(listImagesToSend.size()==0)
	{
		//no images send an empty request,  we shouldn't have entered here in that case.
		TRACEINTO<< "Image list is empty partyRsrcId:" << partyRsrcId;
		succeeded = false;
	}
	if(succeeded)
	{
		std::list<const CImage*>::iterator listImagesToSendIter = listImagesToSend.begin();

		const CImage* pCurrentImageToSend = NULL;
		PartyRsrcID imagePartyRsrcID = INVALID;
		bool foundValidImage = false;
		for(;listImagesToSendIter!=listImagesToSend.end();++listImagesToSendIter)
		{
			pCurrentImageToSend = *listImagesToSendIter;
			if(pCurrentImageToSend!=NULL)
			{
				imagePartyRsrcID = pCurrentImageToSend->GetPartyRsrcId();
				int mediaStreamsListSize = pCurrentImageToSend->GetVideoRelayMediaStreamsList().size();
				bool isImageMuted = pCurrentImageToSend->isMuted();
				if(!isImageMuted && mediaStreamsListSize != 0 && imagePartyRsrcID != partyRsrcId)//for cascade link to avoid loop back we can't use same party image
				{
					//valid image found;
					foundValidImage = true;
					break;
				}
			}
		}
		if(!foundValidImage || (pCurrentImageToSend==NULL ))
		{
			//no valid images found
			TRACEINTO<< "No valid image to send partyRsrcId = " << partyRsrcId;
			succeeded = false;
		}
		if(succeeded)
		{
			//we need to first fill the stream with lowest resolution (layer id) and then not to choose same stream again
			 std::multimap<int, CVideoRelayOutMediaStream>::const_iterator  itr_streamsMap = pVideoMediaParam.m_videoStreamsSortedByResolutionOrPriority.begin();
			 for (; itr_streamsMap != pVideoMediaParam.m_videoStreamsSortedByResolutionOrPriority.end();  ++itr_streamsMap )
			 {
				bool image_found = false;
				CVideoRelayOutMediaStream aVideoOutMediaStream = (*itr_streamsMap).second;
				bool cantHaveSameSource = true;
				image_found = FillVideoSource( aVideoOutMediaStream, pCurrentImageToSend, rResultVideoSourcesRequest, cantHaveSameSource);
				if(! image_found )
				{
					CScpPipeWrapper scpPipe;
					scpPipe.m_pipeId = aVideoOutMediaStream.GetSsrc();
					FillScpPipe( pCurrentImageToSend, scpPipe );

					rResultVideoScpNotificationRequest.AddPipe(scpPipe);
					//			TRACEINTO << "DEBUG_SCP : add to CScpNotificationWrapper : pipeId = " << scpPipe.m_pipeId << ", key = " << key;//TMP for debug
				}
			 }
			 if(rResultVideoSourcesRequest.GetNumSources()>0)
			 {
				 resultUpdateOnImageStruct.bIsNeedToSendUpdate = true;
				 resultUpdateOnImageStruct.idOfPartyToUpdate = partyRsrcId;
				 resultUpdateOnImageStruct.idOfSeenParty = imagePartyRsrcID;

			 }

		}

	}

	if(!succeeded)
	{
		UpdateNotificationsOnAllRequestedStreamsCantBeProvided(((CVideoRelayOutParamsStore& )pVideoMediaParam),rResultVideoScpNotificationRequest);
		UpdateOnNoImageSeen(resultUpdateOnImageStruct);
		return succeeded;
	}

	return succeeded;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVideoRelayOutStreamsHandler::IsPartyVideoSourcesRequestsEqual(CVideoRelaySourcesParams& rFisrtPartyVideoSourcesRequest, CVideoRelaySourcesParams& rSecondPartyVideoSourcesRequest)
{
    bool isEqual = true;
    bool isContain = false;
    if(rFisrtPartyVideoSourcesRequest == rSecondPartyVideoSourcesRequest)
    {
//        TRACEINTO << "rFisrtPartyVideoSourcesRequest == rSecondPartyVideoSourcesRequest";
        return true;
    }
    if(rFisrtPartyVideoSourcesRequest.GetVideoSourcesList().size() != rSecondPartyVideoSourcesRequest.GetVideoSourcesList().size())
    {
//        TRACEINTO << "rFisrtPartyVideoSourcesRequest.m_videoSourcesParams.m_videoSources.size() != rSecondPartyVideoSourcesRequest.m_videoSourcesParams.m_videoSources.size()";
        return false;
    }
    else
    {
        std::list <CVideoRelaySourceApi>::iterator itFirtsVideoSource = rFisrtPartyVideoSourcesRequest.GetVideoSourcesList().begin();
        for(; itFirtsVideoSource!=rFisrtPartyVideoSourcesRequest.GetVideoSourcesList().end(); ++itFirtsVideoSource)
        {
            std::list <CVideoRelaySourceApi>::iterator itSecondVideoSource = rSecondPartyVideoSourcesRequest.GetVideoSourcesList().begin();
            for(; itSecondVideoSource!=rSecondPartyVideoSourcesRequest.GetVideoSourcesList().end(); ++itSecondVideoSource)
            {
                isContain = false;
                if(itFirtsVideoSource == itSecondVideoSource)
                {
//                    TRACEINTO << "itFirtsVideoSource == itSecondVideoSource";
                    isContain = true;
                    break;
                }
            }
            if( !isContain )
            {
//                TRACEINTO << "false == isContain";
                isEqual = false;
                break;
            }
        }
    }

    return isEqual;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayOutStreamsHandler::FillEpIntaParams(CVideoRelaySourcesParams& rPrevVideoSourcesRequest,
													CVideoRelaySourcesParams& rResultVideoSourcesRequest,
													AskForRelayIntra& rEpIntraParams)
{
    // Need to go over the new request.
    // In case there is a pipeId that wasn't in the prev. request
    // or it was in the prev. request but it was filled with a different source
    // we will add the source SSRC to the list

    std::list <CVideoRelaySourceApi>::iterator itr = rResultVideoSourcesRequest.GetVideoSourcesList().begin();
    while (itr != rResultVideoSourcesRequest.GetVideoSourcesList().end())
    {
        unsigned int pipeId = (*itr).GetPipeId();
        unsigned int  csrc = (*itr).GetSyncSource();
        unsigned int prevCsrcOfPipId = 0;
        bool wasPipeIdInPrevReq =  GetCsrsForPipeId(rPrevVideoSourcesRequest,pipeId,prevCsrcOfPipId );
        if( !wasPipeIdInPrevReq || (wasPipeIdInPrevReq && (prevCsrcOfPipId!= csrc)) )
        {
            const CImage* seenImage = m_pVideoBridge->GetImageBySsrc(csrc);
            if(seenImage)
            {
                DWORD partyId = seenImage->GetPartyRsrcId();
                bool isGDR= false;//in this version we initiate GDR only when EP/MRMP initiate
                TRACESTRFUNC(eLevelDebug) << "Add to intra request :  partyId=" << partyId << ", csrc="<< csrc << ", isGDR= "<< isGDR;
                AddIntraReq( rEpIntraParams.m_relayIntraParameters, partyId, csrc, isGDR );
            }
            else
                PASSERTMSG( csrc,"CVideoRelayOutStreamsHandler::FillEpIntaParams : Can't find image with this ssrc");
        }
        itr++;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVideoRelayOutStreamsHandler::GetCsrsForPipeId(CVideoRelaySourcesParams videoSourcesRequest,DWORD pipeId, DWORD& rCsrcOfPipId)
{
    bool isPipeIdInReq = false;
    std::list <CVideoRelaySourceApi>::iterator itr = videoSourcesRequest.GetVideoSourcesList().begin();
    while (isPipeIdInReq == false && itr != videoSourcesRequest.GetVideoSourcesList().end())
    {
        if ((*itr).GetPipeId() == pipeId)
        {
            isPipeIdInReq = true;
            rCsrcOfPipId = (*itr).GetSyncSource();

        }
        itr++;
    }
    return isPipeIdInReq;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayOutStreamsHandler::AddIntraReq(RelayIntraParams& rEpIntraParams, PartyRsrcID partyRsrcId, DWORD csrc, bool isGdr)
{
    std::list <RelayIntraParam>::iterator itr = rEpIntraParams.begin();
    bool foundPartyId = false;
    while (foundPartyId == false && itr != rEpIntraParams.end())
    {
        if ((*itr).m_partyRsrcId == partyRsrcId)
        {
            foundPartyId = true;
            std::list<unsigned int>::iterator ssrcItr = (*itr).m_listSsrc.begin();
            bool foundSsrcInList = false;
            while (foundSsrcInList == false && ssrcItr != (*itr).m_listSsrc.end())//not to add the same id twice
            {
                if((*ssrcItr)==csrc)
                {
                    foundSsrcInList = true;
                 }
                ssrcItr++;
            }
            if(foundSsrcInList)
            {
                TRACEINTO << "the CSRC is already in the intra list no need to add, partyRsrcId:"<< partyRsrcId << ", csrc:" << csrc;
            }
            else
            {
                //Add new SSRC to the list to the party with partyRsrcId
                (*itr).m_listSsrc.push_back(csrc);
            }
        }

        itr++;
    }
    if(!foundPartyId)
    {
        //Add new entry to the list
    	RelayIntraParam epintraParam;
    	epintraParam.m_partyRsrcId = partyRsrcId;
        epintraParam.m_listSsrc.push_back(csrc);
        epintraParam.m_bIsGdr = isGdr;
        rEpIntraParams.push_back(epintraParam);
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVideoRelayOutStreamsHandler::GetPartyIdFromSSRC(int ssrrc, PartyRsrcID& rPartyRsrcId)
{
    bool found = false;
    const CImage* seenImage = m_pVideoBridge->GetImageBySsrc(ssrrc);
    if(seenImage)
    {
        rPartyRsrcId = seenImage->GetPartyRsrcId();
        found = true;
    }
    return found;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVideoRelayOutStreamsHandler::FillScpPipe( const CImage* pCurrentImageToSend, CScpPipeWrapper& rScpPipe, bool isSlideState)
{
    // MRM cannot provide the stream(s), we indicate this by ScpStreamsNotification message
    rScpPipe.m_notificationType = eStreamCannotBeProvided;
    rScpPipe.m_reason     = eStreamNotAvailable;
    rScpPipe.m_bIsPermanent  = true;

    if( !pCurrentImageToSend )
        return;

//    if( pCurrentImageToSend->IsImageWithProblem() )
//    {
//        rScpPipe.m_reason = eStreamNotProvidedBecauseOfErrorResiliency;
//    }
    else if( pCurrentImageToSend->isMuted() )
    {
        rScpPipe.m_reason = eStreamIsMuted;
    }
    else if(isSlideState)
    {
    	rScpPipe.m_reason = eStreamNotProvidedBecauseOfIvrSlideState;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVideoRelayOutStreamsHandler::CompareUpdateVideoScpNotificationRequest( CScpNotificationWrapper& rPrev, CScpNotificationWrapper& rCurrent)
{
    if( rPrev == rCurrent )
    {
//    	TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : same VideoScpNotificationRequest - no need to send!!!  numOfPipes = " << rPrev.m_numOfPipes;
        return true;
    }
    std::list<CScpPipeWrapper> pipes_to_add;
    std::list<CScpPipeWrapper>::const_iterator itr_pipes_prev = rPrev.m_pipes.begin();
    for ( ; itr_pipes_prev != rPrev.m_pipes.end(); ++itr_pipes_prev )
    {
    	CScpPipeWrapper stream_not_provided_before = *itr_pipes_prev;
        bool stream_found_in_current_notify_req = false;
        std::list<CScpPipeWrapper>::const_iterator itr_pipes_curr = rCurrent.m_pipes.begin();
        for ( ;itr_pipes_curr != rCurrent.m_pipes.end(); ++itr_pipes_curr )
        {
        	CScpPipeWrapper stream_not_provided_current = *itr_pipes_curr;
            if( stream_not_provided_before.m_pipeId == stream_not_provided_current.m_pipeId )
            {
                stream_found_in_current_notify_req = true;
                break;
            }
        }
        if( !stream_found_in_current_notify_req && stream_not_provided_before.m_notificationType == eStreamCannotBeProvided ) //means that stream(s) previously indicated as not provided, are now provided.
        {
            pipes_to_add.push_back( stream_not_provided_before );
        }
    }

    //to add all streams previously indicated as not provided
    for ( std::list<CScpPipeWrapper>::const_iterator itr_pipes_add = pipes_to_add.begin(); itr_pipes_add != pipes_to_add.end(); ++itr_pipes_add )
    {
    	CScpPipeWrapper stream_not_provided_before = *itr_pipes_add;
        stream_not_provided_before.m_notificationType = eStreamCanNowBeProvided;
        stream_not_provided_before.m_reason = eStreamIsNowProvided;
        TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : stream_not_provided_before.pipeId = " << stream_not_provided_before.m_pipeId;
        rCurrent.m_pipes.push_back( stream_not_provided_before );
        rCurrent.m_numOfPipes++;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVideoRelayOutStreamsHandler::CompareUpdateOnSeenImageStruct(UpdateOnSeenImageStruct first,UpdateOnSeenImageStruct second )
{
	bool ans = false;
	if((first.idOfPartyToUpdate==second.idOfPartyToUpdate) && (first.idOfSeenParty == second.idOfSeenParty))
		ans = true;
	return ans;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayOutStreamsHandler::UpdateOnNoImageSeen(UpdateOnSeenImageStruct & rUpdateOnImageStruct)
{

	rUpdateOnImageStruct.bIsNeedToSendUpdate = true;
	PartyRsrcID  partyRsrcId = m_pCVideoBridgePartyCntl->GetPartyRsrcID();
	TRACEINTO << "partyRsrcId: " << partyRsrcId;

	rUpdateOnImageStruct.idOfPartyToUpdate=partyRsrcId;
	rUpdateOnImageStruct.idOfSeenParty = INVALID;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//void CVideoRelayOutStreamsHandler::FillVideoScpNotificationWrapper( CScpNotificationWrapper& rScpNotifyWrapper, std::list<CScpPipeWrapper>& rPipesList)
//{
//	DWORD numOfPipes = rPipesList.size(), i = 0;
//
//	rScpNotifyWrapper.m_numOfPipes = numOfPipes;
//	rScpNotifyWrapper.m_pPipes = new CScpPipeWrapper [numOfPipes];
//
//	for(std::list <CScpPipeWrapper>::iterator itr = rPipesList.begin(); itr!=rPipesList.end();itr++)
//	{
//		rScpNotifyWrapper.m_pPipes[i] = (*itr);
//		i++;
//	}
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayOutStreamsHandler::UpdateNotificationsOnAllNonIVRRequestedStreamsCantBeProvided(CVideoRelayOutParamsStore& rVideoMediaParam, DWORD ivrSsrc)
{
    //1. save the prev notification
    CScpNotificationWrapper prevVideoScpNotification = *rVideoMediaParam.m_pCurrentVideoScpNotificationRequest;

	std::list<CRelayMediaStream*> requestedMediaStream = rVideoMediaParam.m_pCurrentMediaParam->GetRelayMediaStream();
	rVideoMediaParam.m_needToSendScpStreamsNotification = true;
	// clean the new scp notification request and fill its general params
	rVideoMediaParam.m_pCurrentVideoScpNotificationRequest->InitDefaults();
	rVideoMediaParam.m_pCurrentVideoScpNotificationRequest->m_channelHandle = rVideoMediaParam.m_pCurrentMediaParam->m_channelHandle;
	rVideoMediaParam.m_pCurrentVideoScpNotificationRequest->m_remoteSequenceNumber = rVideoMediaParam.m_pCurrentMediaParam->m_scpRequestSequenceNumber;

	//2. Fill SCPPipe of non IVR streams
	std::list<CRelayMediaStream *>::const_iterator itr_list = requestedMediaStream.begin();
	for ( ; itr_list != requestedMediaStream.end(); ++itr_list)
	{
		CVideoRelayOutMediaStream* pVideoOutMediaStream = (CVideoRelayOutMediaStream*)(*itr_list);
		if(pVideoOutMediaStream->GetSsrc() != ivrSsrc)
		{
			CScpPipeWrapper scpPipe;
			bool isSlideState = true;
			scpPipe.m_pipeId = pVideoOutMediaStream->GetSsrc();
			FillScpPipe( NULL, scpPipe, isSlideState);
			rVideoMediaParam.m_pCurrentVideoScpNotificationRequest->AddPipe(scpPipe);
		}
	}

	//3. Check if Notification wasn's changed , we don't need to send it again.
    bool isEqual = CompareUpdateVideoScpNotificationRequest( prevVideoScpNotification, *rVideoMediaParam.m_pCurrentVideoScpNotificationRequest );
	if( isEqual )
	{
		*rVideoMediaParam.m_pCurrentVideoScpNotificationRequest = prevVideoScpNotification;
		rVideoMediaParam.m_needToSendScpStreamsNotification = false;
	}
	else
	{
		TRACEINTO << "We need to send notification on all non IVR unavailable streams";
		rVideoMediaParam.m_needToSendScpStreamsNotification = true;
	}
}

