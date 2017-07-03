#ifndef SCP_HANDLER_H_
#define SCP_HANDLER_H_


#include <arpa/inet.h>
#include <stdlib.h>
#include "Segment.h"
#include "StateMachine.h"
#include "PartyApi.h"

class CScpNotificationWrapper;
class CScpPipeWrapper;


typedef std::map< ESipMediaChannelType, std::list <CScpPipeWrapper> > SCP_NOTIFICATIONS_TO_PARTY_MAP;

typedef struct
{
	eIvrState	 m_ivrState;
	unsigned short m_localSeqNum;
} ScpIvrStateNotification;

class CMrmpStreamDescWrap : public CPObject {
CLASS_TYPE_1(CMrmpStreamDescWrap,CPObject)
public:
	CMrmpStreamDescWrap();
	CMrmpStreamDescWrap& operator= (const CMrmpStreamDescWrap &other);
    CMrmpStreamDescWrap& operator= (const StreamDesc &other);
    CMrmpStreamDescWrap& operator= (const MrmpStreamDesc &other);

	virtual void  Serialize(WORD format,CSegment& seg) const;
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual const char*         NameOf() const                                 { return "CMrmpStreamDescWrap"; }
	void  Print();
	APIU32 GetChannelType(){return m_ChannelType;}
	APIU32 GetPayloadType(){return m_PayloadType;}
	APIU32 GetSpecificSourceSsrc(){return m_SpecificSourceSsrc;}
	APIU32 GetBitRate(){return m_BitRate;}
	APIU32 GetFrameRate(){return m_FrameRate;}
	APIU32 GetFrameHeight(){return m_Height;}
	APIU32 GetFrameWidth(){return m_Width;}
	APIU32 GetFramePipeIdSsrc(){return m_PipeIdSsrc;}
	APIU32 GetFrameSourceIdSsrc(){return m_SourceIdSsrc;}
	APIU32 GetPriority(){return m_Priority;}
    APIU32 GetIsLegal(){return m_isLegal;}

	void SetBitRate(unsigned int bitRate){m_BitRate=bitRate;}
	void SetFrameRate(unsigned int frameRate){m_FrameRate=frameRate;}
	void SetFrameHeight(unsigned int height){m_Height=height;}
	void SetFrameWidth(unsigned int width){m_Width=width;}
    void SetIsLegal(APIU32 isLegal){m_isLegal = isLegal;}

private:
	APIU32 m_ChannelType;		// kChanneltype
	APIU32 m_PayloadType;
	APIU32 m_SpecificSourceSsrc;
	APIU32 m_BitRate;
	APIU32 m_FrameRate;
	APIU32 m_Height;
	APIU32 m_Width;
	APIU32 m_PipeIdSsrc;
	APIU32 m_SourceIdSsrc;
	APIU32 m_Priority;
    APIU32 m_isLegal;
};

/////////////////////////////////////////////////////////////////////////////
class CMrmpScpStreamsRequestStructWrap: public CPObject {
CLASS_TYPE_1(CMrmpScpStreamsRequestStructWrap,CPObject)
public:
	CMrmpScpStreamsRequestStructWrap();
	CMrmpScpStreamsRequestStructWrap& operator= (const CMrmpScpStreamsRequestStructWrap &other);
    CMrmpScpStreamsRequestStructWrap& operator= (const MrmpScpStreamsRequestStruct &other);
	~CMrmpScpStreamsRequestStructWrap();
	virtual void  Serialize(WORD format,CSegment& seg);
	virtual void  DeSerialize(WORD format,CSegment& seg);
	void Print();
	virtual const char*         NameOf() const                                 { return "CMrmpScpStreamsRequestStructWrap"; }
	APIU32 GetSequenceNumber(){return m_SequenceNumber;}
	APIU32 GetNumberOfMediaStreams(){return m_NumberOfMediaStream;}
	CMrmpStreamDescWrap* GetMediaStreams(){return m_mediaStreams;}

private:
	APIU32 		   m_ChannelHandle;
	APIU32 	   	   m_SequenceNumber;
	APIS32 	   	   m_NumberOfMediaStream;
	CMrmpStreamDescWrap* m_mediaStreams;
};

/////////////////////////////////////////////////////////////////////////////

class CScpHandler : public CPObject {
CLASS_TYPE_1(CScpHandler,CPObject)
public:
	 CScpHandler();
	 CScpHandler(const CScpHandler &other);
	 ~CScpHandler();
	 bool HandleScpRequest(CMrmpScpStreamsRequestStructWrap &aScpStreamReq, CIpComMode*  pTarget,CSipCaps* pSipLocalCaps);
	 virtual const char* NameOf() const { return "CScpHandler"; }
     void Init(CPartyApi* pPartyApi);

     void UpdateNotificationsMap(const CScpNotificationWrapper &notifyReq);
	 bool HandleScpNotificationInd(CScpNotificationWrapper &aScpNotifyInd, CIpComMode*  pTarget);
	 void SendAckForNotification(APIU32 sequenceNumber);

	 unsigned int GetLastScpRequestSeqNumber();
     unsigned int GetLastScpStreamsNotificationLocalSeqNumber();
     unsigned int GetLastScpIvrStateNotificationLocalSeqNumber();
     unsigned int GetLastScpPipesMappingNotificationLocalSeqNumber();
     void UpdateLastScpStreamsNotificationLocalSeqNumber();
     void UpdateLastScpPipesMappingNotificationLocalSeqNumber();

     const ScpIvrStateNotification& GetLastScpIvrStateNotification (){return m_lastScpIvrStateNotification;}
     void UpdateLastScpIvrStateNotification(eIvrState newState);

     unsigned int CalculateDesiredBitRate(CMrmpStreamDescWrap* apMediaStreams, APIS32 aNumberOfMediaStreams, CIpComMode*  apTargetMode,CSipCaps* apSipLocalCaps, BYTE aIsScpRequest=FALSE);

	 void UpdateScm( CMrmpStreamDescWrap* apMediaStreams,
			 	 	 APIS32 aNumberOfMediaStreams,
			 	 	 CIpComMode* pTargetMode,
			 	 	 cmCapDirection direction);

	 void UpdateScpNotificationInScm(CScpNotificationWrapper *pScpNotifyInd, CIpComMode* apTargetMode);

     CMrmpScpStreamsRequestStructWrap* m_pScpStreamNewReq;
     SCP_NOTIFICATIONS_TO_PARTY_MAP m_notificationsMap; //stores all streams (pipes) that notified to party sorted by media type

protected:
	 CMrmpScpStreamsRequestStructWrap	*m_pScpStreamCurrentReq;

	 void SendAckForReq();
	 unsigned int CalculateDesiredBitRateRec(APIS32 aNumberOfMediaStreams, VIDEO_OPERATION_POINT_SET_S* vopS,int* layerId,unsigned int maxVideoBitRate);
	 unsigned int GetRidOfStreams(CMrmpStreamDescWrap* apMediaStreams, APIS32 aNumberOfMediaStreams, unsigned int maxVideoBitRate);


	 //ScpStreamsNotificationParams m_lastStreamsNotificationSent;


private:
  	CPartyApi* m_pPartyApi;
  	unsigned int IncreaseScpNotificationLocalSeqNumber(); //increase global counter

  	unsigned short m_lastScpNotificationLocalSeqNum; // global counter for all notifications that sent from this party
  	unsigned short m_lastScpStreamsNotificationLocalSeqNum; // counter for streams notifications
	unsigned short m_lastScpPipesMappingNotificationLocalSeqNum;
	
	ScpIvrStateNotification m_lastScpIvrStateNotification;

};



#endif /* SCP_HANDLER_H_ */
