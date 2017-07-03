#ifndef MEDIACHANNEL_H_
#define MEDIACHANNEL_H_

#include "StateMachine.h"
#include "MplMcmsProtocol.h"
#include "DataTypes.h"
#include "IpChannelParams.h"
#include "AudRequestStructs.h"
#include "VideoStructs.h"
#include "MediaMngrCfg.h"

#include "MediaMngr.h"
#include "UdpSocket.h"

//#include "VideoChannel.h"
//#include "AudioChannel.h"


#include "IpEncryptionDefinitions.h"

extern "C"
{
	#include "AESLib.h"
}


using namespace std;

class CClientSocket;
//class CListenSocketApi;
class CMediaElement;


//////////////////////////////////////////
//CMediaChannel
//////////////////////////////////////////

class CMediaChannel : public CStateMachine
{
CLASS_TYPE_1(CMediaChannel, CStateMachine)
public:
	CMediaChannel();
	CMediaChannel(CTaskApp* pOwnerTask, INT32 channelDirection);
	virtual ~CMediaChannel() = 0;

	virtual void* GetMessageMap();
	virtual const char * NameOf() const;
	virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);

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

	void UpdateMediaChannelParams(CSegment* pParam);
	//encryption
	EenMediaType GetEncryptionType() const;
	string GetEncryptionTypeStr() const;
	APIU8* GetEncryptionSessionKey();
	string GetEncryptionSessionKeyStr() const;


	// Action functions
	void OnListEventMessageAll(CSegment* pParam);
	void OnActivateEventMessageAll(CSegment* pParam);
	void OnChannelInParamEventMessageAll(CSegment* pMsgIncomingParams);
	void OnTimerSendMedia();
	void OnTimerRecvMedia();

	//participant ticket
	void SetParticipantTicket(string participantTicket);
	string GetParticipantTicket();

protected:

	//Media Handling
	////////////
	virtual string ChannelData();
	virtual int SetupMediaFile() = 0;

	virtual int GetTimeToNextTransmission(DWORD timeStamp);
	virtual DWORD GetTimeToNextTransmissionMl(DWORD timeStamp);

	void PrepareMediaFrame();
	void SendMedia();

	//Channel methods
	////////////
	void SaveChannelParams(CMplMcmsProtocol* pMplProtocol);
	void CloseChannel();

	// channel parameters for Intra
	DWORD		m_boardId;
	DWORD		m_subBoardId;
	DWORD		m_unitId;
	DWORD		m_conferenceId;
	DWORD		m_partyId;
	DWORD		m_connectionId;

	//Addressing
	////////////
	void PrepareMediaOutAddressing();
	void PrepareMediaInAddressing();


	//Tx
	////////////////////////

	//Media Tx Start/Stop/Restart
	void StartMediaTx();
	virtual void RestartMediaTx() {};
	void StopMediaTx();

	//Sockets Tx
	void CreateTxSocket();
	//void InitTxSocket();
	void SendTxSocket();
	void SendTxSocket(const char* buffer);
	void SendTxSocket(const BYTE* buffer, DWORD size);
	void CloseTxSocket();


	//Rx
	////////////////////////

	//Media Rx Start/Stop/Restart
	void StartMediaRx();
	void StopMediaRx(); //??

	//Sockets Rx
	void CreateRxSocket();
	//void InitRxSocket();
	void ReceiveRxSocket();
	void CloseRxSocket();

	void StartWriteUdpSocket();
	int WriteBuffer();
	int FileOpen();
	int FileClose();
	int FileSize();
	int ChecksLegalPacketSeqNumber( UINT16 curPacketSeqNumber );
	BOOL CheckIfFullH264IntraWasDetected();
	BOOL CheckIfFullH263IntraWasDetected();
	BOOL CheckIfH263StartOfFrame();
	BOOL CheckIfH263Intra1stPacket();
	int	SaveMediaAndWrite( int sizeRecv, int lastPacketInFrame );
	BOOL CheckMarkerForEndOfFrame();


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
	EVideoProtocol m_eVideoProtocol;
	

	//address

	//CG
	mcTransportAddress m_taLocalAddress;
	struct sockaddr_in m_local_addr;

	//EP
	mcTransportAddress m_taRemoteAddress;
	struct sockaddr_in m_remote_addr;


	// udp sockets
	CSocket*  m_pSocketOut;
	CSocket*  m_pSocketIn;

	TEventMessage*	m_pEventMessageArr[MAX_EVENT_MESSAGE_STACK_SIZE];

	BOOL m_bOpenUdpPort;
	BOOL m_bRtpUpdatePort;
	BOOL m_bVideoOutParam;

	BOOL m_bIsChannelOpen;

	// media processing
	BYTE* m_mediaFileBuffer;
	int m_fileSize;
	int m_readInd;

	int m_lastReadInd;
	int m_saveLastReadInd;

	int m_loopCounter;
	int m_saveLoopCounter;



	WORD m_firstConnection;

	//media repository
	CMediaElement* m_mediaRepositoryElement;

	//RTP
	BYTE m_rtpPacketsArr[MAX_PACKETS_PER_FRAME][MAX_PACKET_SIZE_ENCRYPTION];
	DWORD m_rtpPacketsSize[MAX_PACKETS_PER_FRAME];

	//packet sequence number
	WORD m_packetSeqNumber;

	//frame time stamp
	DWORD m_currentFrameTS;
///	DWORD m_firstBufferTS;
	DWORD m_prevUsedTS;
	DWORD m_prevBaseTS;
	WORD m_eofTsFactor;

	//transmission data
	//int m_timeToNextTransmission;
	int m_numOfPacketsForTransmission;

	int m_timeToNextTransmission; //save time in MilliSec

	//file name
	string m_strFileName;

	//for less than 10 decimal^2 sec
	DWORD m_fraction;

	//DTMF flag
	BOOL m_bIsDtmfSession;

	//first loop on file flag
	//BOOL m_bIsFirstLoopOnFile;

	UINT16 m_lastPacketSeqNumber;
	UINT16 m_saveLastPacketSeqNumber;


	//Rx Buffer
	BYTE m_recvBuffer[MAX_NUM_OF_BYTES_IN_UDP_DATAGRAM + sizeof(TMediaHeaderIpNetwork)];
	struct sockaddr_in m_from_addr;

	//Recording file
	BYTE* m_mediaBuffer;
	int   m_mediaBufferIndex;
	int   m_frameBufferIndex;
//	BOOL  m_bWriteFlag;
	TIncomingChannelParam	m_tIncomingChannelParam;
	
	BOOL m_1stIntraPacketWasDetected;
	BOOL m_2ndIntraPacketWasDetected;
	BOOL m_3rdIntraPacketWasDetected;	
	int m_h263HeaderSize;							
	
	FILE* m_pFile;
	string m_sFullFileName;
	DWORD m_writtenFileSize;


	//participant ticket [arrayIndex, participantIndex]
	string m_participantTicket;

	//last Rx packet sequence number
	UINT16 m_lastRxPacketSeqNumber;

	//media library
	CMediaLibrary* m_mediaLibrary;


	//Encryption
	EenMediaType m_unEncryptionType;
	APIU8  m_aucSessionKey[sizeOf128Key];
	UINT32 m_aunExpandedRoundKey[NUM_WORDS_FOR_EXPANDED_KEY];

	int m_iceRtpChannelID;
	//int m_iceRtcpChannelID;//RTCP maybe not needed yet

	PDECLAR_MESSAGE_MAP
};










//////////////////////////////////////////
//CFeccChannel
//////////////////////////////////////////


class CFeccChannel : public CMediaChannel
{
CLASS_TYPE_1(CFeccChannel, CMediaChannel)
public:
	CFeccChannel();
	CFeccChannel(CTaskApp* pOwnerTask, INT32 channelDirection);
	virtual ~CFeccChannel();

	virtual void* GetMessageMap();
	virtual const char * NameOf() const;
	virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);

protected:
	string ChannelData();
	int SetupMediaFile() { return STATUS_OK; }

	void RestartMediaTx() {};

	PDECLAR_MESSAGE_MAP
};


#endif /*MEDIACHANNEL_H_*/
