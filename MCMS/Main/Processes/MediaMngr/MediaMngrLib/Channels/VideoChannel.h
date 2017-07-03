#ifndef VIDEOCHANNEL_H_
#define VIDEOCHANNEL_H_

#include "StateMachine.h"
#include "MplMcmsProtocol.h"
#include "DataTypes.h"
#include "IpChannelParams.h"
#include "AudRequestStructs.h"
#include "VideoStructs.h"
#include "MediaMngrCfg.h"

#include "MediaMngr.h"
#include "UdpSocket.h"

#include "IpEncryptionDefinitions.h"
#include "MediaChannel.h"

extern "C"
{
	#include "AESLib.h"
}


using namespace std;

class CClientSocket;
//class CListenSocketApi;
class CMediaElement;

//////////////////////////////////////////
//CVideoChannel
//////////////////////////////////////////

class CVideoChannel : public CMediaChannel
{
CLASS_TYPE_1(CVideoChannel, CMediaChannel)
public:
	CVideoChannel();
	CVideoChannel(CTaskApp* pOwnerTask, INT32 channelDirection);
	virtual ~CVideoChannel();

	virtual void* GetMessageMap();
	virtual const char * NameOf() const;
	virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);


	// Action functions
	void OnVideoOutParamMessageAll(CSegment* pParam);
	void OnVideoInParamMessageAll(CSegment* pParam);
	void OnTimerEnableIntra(CSegment* pParam);
	void OnVideoOutUpdateParamMessageAll(CSegment* pParam); //on ART_CONTENT_ON_REQ/ART_CONTENT_OFF_REQ
	void OnVideoOutResetMessageAll(CSegment* pParam);	//Reset Video Out
	void OnVideoInUpdatePicMessageAll(CSegment* pParam); // request intra from EP
	void OnVideoOutEncoderUpdateParamMessageAll(CSegment* pParam); //on VIDEO_ENCODER_UPDATE_PARAM_REQ


protected:
	string ChannelData();
	int SetupMediaFile();

	int GetTimeToNextTransmission(DWORD timeStamp);
	DWORD GetTimeToNextTransmissionMl(DWORD timeStamp);

	void SetFramePosition(DWORD timeStamp);

	void RestartMediaTx();

	string GetBitrateStr(DWORD bitrate);


	void UpdateVideoParam(DWORD contentBitrate, DWORD contentMode);
	void RequestFastUpdateFromEP();


protected:

	int m_framePosition;

	TVideoParam m_tCurrentVideoParam;
	TVideoParam m_tSaveContentVideoParam; //saved original data for content

	//Intra
	BOOL m_bAllowIntraNow;
	int m_intraDelayTimeResponse; // in centisecond

	//media item name is save (till TB_MSG_OPEN_PORT_REQ arrives)
	string m_videoItemName;



	PDECLAR_MESSAGE_MAP
};

#endif /*VIDEOCHANNEL_H_*/
