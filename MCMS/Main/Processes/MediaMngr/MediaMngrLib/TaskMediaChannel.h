#ifndef TASKMEDIACHANNEL_H_
#define TASKMEDIACHANNEL_H_

#include "DataTypes.h"
#include "MplMcmsProtocol.h"
#include "MediaMngr.h"
#include "IpChannelParams.h"
#include "TaskApp.h"
#include "TaskApi.h"
#include "AudRequestStructs.h"
#include "VideoStructs.h"

#include "UdpSocket.h"

using namespace std;

class CClientSocket;
class CListenSocketApi;
class CMediaElement;


//   EXTERNALS:
//   *** SOCKETS ***
// Tx Sockets entry points
/*extern "C" void VideoTxEntryPoint(void* appParam);
extern "C" void AudioTxEntryPoint(void* appParam);
extern "C" void ContentTxEntryPoint(void* appParam);
extern "C" void FeccTxEntryPoint(void* appParam);*/

// Rx Sockets entry points
extern "C" void VideoRxEntryPoint(void* appParam);
extern "C" void AudioRxEntryPoint(void* appParam);
extern "C" void ContentRxEntryPoint(void* appParam);
extern "C" void FeccRxEntryPoint(void* appParam);


//////////////////////////////////////////
//CMediaChannel
//////////////////////////////////////////

class CTaskMediaChannel : public CTaskApp
{
CLASS_TYPE_1(CTaskMediaChannel, CTaskApp)
public:
	CTaskMediaChannel() {};
	CTaskMediaChannel(INT32 channelDirection);
	virtual ~CTaskMediaChannel() = 0;
	
	virtual const char * NameOf() const {return "CTaskMediaChannel";}
	virtual const char* GetTaskName() const {return "TaskMediaChannel"; }
	
	BOOL  IsSingleton() const {return NO;}
	virtual void  InitTask();
	virtual void  SelfKill();
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();
	
	
	// class Methods
	/////////////////////////////////////////////
	INT32 GetChannelDirection() const;
	string GetChannelDirectionStr() const;
	CapEnum GetCapTypeCode() const;
	string GetCapTypeCodeStr() const;
	APIU32 GetBitRate() const;
	string GetBitRateStr() const;
	APIU32 GetPayloadType() const;
	string GetPayloadTypeStr() const;	
	APIU32 GetDtmfPayloadType() const;
	string GetDtmfPayloadTypeStr() const;	
	APIU32 GetIsH263Plus() const;
	string GetIsH263PlusStr() const;
	APIU32 GetAnnexesPlus() const;
	string GetAnnexesPlusStr() const;
	APIU32 GetCustomMaxMbpsValue() const;
	string GetCustomMaxMbpsValueStr() const;
	APIU32 GetMaxFramesPerPacket() const;
	string GetMaxFramesPerPacketStr() const;
	
	
	// Action functions
	void OnListEventMessageAll(CSegment* pParam);
	void OnActivateEventMessageAll(CSegment* pParam);
	void OnTimerSendMedia(CSegment* pParam);
	
	
protected:
	
	//Media Handling
	////////////
	virtual string ChannelData();
	virtual int SetupMediaFile() = 0;
	virtual TaskEntryPoint GetMediaRxTaskEntryPoint() = 0;
	
	virtual int GetTimeToNextTransmission(DWORD timeStamp);
	virtual void SendMedia() {};
	
	void StartMediaTx();
	void StartMediaRx();
	virtual void RestartMediaTx() {};
	
	void StopMediaTx();
	
	void PrepareMediaFrame();
	
	virtual string GetMediaPrefix() = 0;
	
	
	//Addressing
	////////////
	void PrepareMediaOutAddressing();
	void PrepareMediaInAddressing();
	
	
	//Channel methods
	////////////
	void SaveChannelParams(CMplMcmsProtocol* pMplProtocol);
	void CloseChannel();
	
	
	//Sockets Tx
	void CreateTxSocket();
	void InitTxSocket();
	void SendTxSocket();
	void SendTxSocket(const char* buffer);
	void SendTxSocket(const BYTE* buffer, DWORD size);
	void CloseTxSocket();
	
	// protocol methods
	TEventMessage* GetEventMessage(OPCODE eventOpcode);
	BOOL ListEventMessage(const string opCodeStr, OPCODE opCode, CMplMcmsProtocol* pMplProtocol);
	void ActivateEventMessage(const string opCodeStr, OPCODE opCode, CMplMcmsProtocol* pMplProtocol);
	void DumpEventMessageList();


protected:
	
	//members
	INT32 m_channelDirection;
	CapEnum m_capTypeCode;
	APIU32 m_bitRate;
	APIU32 m_payloadType;
	APIU32 m_dtmfPayloadType;
	APIU32 m_isH263Plus;
	APIU32 m_annexesMask;
	APIU32 m_customMaxMbpsValue;
	APIU32 m_maxFramesPerPacket;
	
	
	//address
	
	//CG
	mcTransportAddress m_taLocalAddress;
	struct sockaddr_in m_local_addr;
	
	//EP
	mcTransportAddress m_taRemoteAddress;
	struct sockaddr_in m_remote_addr;
	
		
	// udp sockets
	CUdpSocket*  m_pUdpSocketOut;
	CTaskApi*	 m_pUdpListenSocketTaskApi;
	
	TEventMessage*	m_pEventMessageArr[MAX_EVENT_MESSAGE_STACK_SIZE];
	
	BOOL m_bOpenUdpPort;
	BOOL m_bRtpUpdatePort;
	BOOL m_bVideoOutParam;
	
	BOOL m_bIsChannelOpen;

	// media processing
	BYTE* m_mediaFileBuffer;
	int m_fileSize;
	int m_readInd;
	WORD m_firstConnection;	
	
	//media repository
	CMediaElement* m_mediaRepositoryElement;
	
	BYTE m_packetsArr[MAX_PACKETS_PER_FRAME][MAX_PACKET_SIZE];
	DWORD m_payloadSize[MAX_PACKETS_PER_FRAME];
	
	//packet sequence number
	WORD m_packetSeqNumber;
	
	//frame time stamp
	DWORD m_currentFrameTS;
	DWORD m_iterationDeltaTS;
	
	DWORD m_firstBufferTS;
	DWORD m_lastUsedTS;
	
	WORD m_eofFactorTS;
	
	//transmission data
	int m_timeToNextTransmission;
	int m_numOfPacketsForTrans;
	
	//file name
	string m_strFileName;
	
	//for less than 10 decimal^2 sec
	DWORD m_fraction;
	
	//DTMF flag
	BOOL m_bIsDtmfSession;
	
		
	PDECLAR_MESSAGE_MAP

};



//////////////////////////////////////////
//CTaskVideoChannel
//////////////////////////////////////////

//Video task create entry points
extern "C" void VideoOutChannelEntryPoint(void* appParam);
extern "C" void VideoInChannelEntryPoint(void* appParam);

class CTaskVideoChannel : public CTaskMediaChannel
{
public:
	CTaskVideoChannel() {};
	CTaskVideoChannel(INT32 channelDirection);
	virtual ~CTaskVideoChannel();
		
	virtual const char * NameOf() const {return "CTaskVideoChannel";}
	virtual const char* GetTaskName() const {return "TaskVideoChannel"; }
	
	void  SelfKill();
	
	// Action functions
	void OnVideoOutParamMessageAll(CSegment* pParam);
	void OnTimerEnableIntra(CSegment* pParam);
	void OnVideoOutUpdateParamMessageAll(CSegment* pParam);	
	
	/*
	//video rate change
	void UpdateVideoOldParam(ENCODER_PARAM_S* pEncoderParamsStruct);
	*/	
		
protected:
	string ChannelData();
	int SetupMediaFile();
	TaskEntryPoint GetMediaRxTaskEntryPoint() { return VideoRxEntryPoint;}
	
	int GetTimeToNextTransmission(DWORD timeStamp);
	void SendMedia();
	
	void SetFramePosition(DWORD timeStamp);
	
	void RestartMediaTx();
	
	string GetMediaPrefix();
	
	string GetBitrateStr(DWORD bitrate);
	
	
	void UpdateVideoParam(DWORD contentBitrate, DWORD contentMode);
	
protected:
	
	int m_framePosition;
	
	BOOL m_bAllowIntraNow;
	
	EVideoProtocol	m_eVideoOutProtocol;
	
	TVideoParam m_tCurrentVideoParam;
	TVideoParam m_tSaveContentVideoParam; //saved original data for content
		
	PDECLAR_MESSAGE_MAP
};



//////////////////////////////////////////
//CTaskAudioChannel
//////////////////////////////////////////

//Audio task create entry points
extern "C" void AudioOutChannelEntryPoint(void* appParam);
extern "C" void AudioInChannelEntryPoint(void* appParam);

class CTaskAudioChannel : public CTaskMediaChannel
{
public:
	CTaskAudioChannel() {};
	CTaskAudioChannel(INT32 channelDirection);
	virtual ~CTaskAudioChannel();
		
	virtual const char * NameOf() const {return "CTaskAudioChannel";}
	virtual const char* GetTaskName() const {return "TaskAudioChannel"; }
	
	void  SelfKill();
	
	//dtmf public methods called from TaskMediaChannel code
	void SendDTMF(SPlayToneStruct* tPlayToneStruct);
	void PrepareNextDtmf();
	void SetDtmfSession(BOOL isDtmfSession);
	void PrintDtmfBuffer();
	
protected:
	string ChannelData();
	int SetupMediaFile();
	TaskEntryPoint GetMediaRxTaskEntryPoint() { return AudioRxEntryPoint;}
	
	int GetTimeToNextTransmission(DWORD timeStamp);
	void SendMedia();
	
	void RestartMediaTx() {};
	
	string GetMediaPrefix();
	
private:
	EAudioTone m_dtmfBuffer[MAX_DTMF_LEN];
	int m_numOfDtmf;
	int m_currDtmfIndex;
	
	BYTE* m_saveMediaFileBuffer;
	int m_saveFileSize;
	int m_saveReadInd;
	
	
	PDECLAR_MESSAGE_MAP
};



//////////////////////////////////////////
//CTaskContentChannel
//////////////////////////////////////////

//Content task create entry points
extern "C" void ContentOutChannelEntryPoint(void* appParam);
extern "C" void ContentInChannelEntryPoint(void* appParam);

class CTaskContentChannel : public CTaskVideoChannel
{
public:
	CTaskContentChannel() {};
	CTaskContentChannel(INT32 channelDirection);
	virtual ~CTaskContentChannel();
	
	virtual const char * NameOf() const {return "CTaskContentChannel";}
	virtual const char* GetTaskName() const {return "TaskContentChannel"; }
	
	void  SelfKill();
	
protected:
	string ChannelData();
	int SetupMediaFile();
	
	TaskEntryPoint GetMediaRxTaskEntryPoint() { return ContentRxEntryPoint;}
	
	void RestartMediaTx() {};
	
	void SendMedia();
	
	string GetMediaPrefix();
	
	PDECLAR_MESSAGE_MAP	
};



//////////////////////////////////////////
//CTaskFeccChannel
//////////////////////////////////////////

//Fecc task create entry points
extern "C" void FeccOutChannelEntryPoint(void* appParam);
extern "C" void FeccInChannelEntryPoint(void* appParam);

class CTaskFeccChannel : public CTaskMediaChannel
{
public:
	CTaskFeccChannel() {};
	CTaskFeccChannel(INT32 channelDirection);
	virtual ~CTaskFeccChannel();
	
	virtual const char * NameOf() const {return "CTaskFeccChannel";}
	virtual const char* GetTaskName() const {return "TaskFeccChannel"; }
	
	void  SelfKill();		
		
protected:
	string ChannelData();
	int SetupMediaFile() { return STATUS_OK; }
	TaskEntryPoint GetMediaRxTaskEntryPoint() { return FeccRxEntryPoint;}
	
	void RestartMediaTx() {};
	
	string GetMediaPrefix() { return "";}
	
	PDECLAR_MESSAGE_MAP	
};




#endif /*TASKMEDIACHANNEL_H_*/
