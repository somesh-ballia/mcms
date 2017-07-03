#ifndef AUDIOCHANNEL_H_
#define AUDIOCHANNEL_H_

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
//CAudioChannel
//////////////////////////////////////////


class CAudioChannel : public CMediaChannel
{
CLASS_TYPE_1(CAudioChannel, CMediaChannel)
public:
	CAudioChannel();
	CAudioChannel(CTaskApp* pOwnerTask, INT32 channelDirection);
	virtual ~CAudioChannel();

	virtual void* GetMessageMap();
	virtual const char * NameOf() const;
	virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);

	// Action functions
	void OnAudioOutResetMessageAll(CSegment* pParam);	//Reset Audio Out
	void OnAudioOutUpdateChannelMessageAll(CSegment* pParam);	//Read silent stream or normal stream



	//dtmf public methods called from MediaChannel code
	void SendDTMF(SPlayToneStruct* tPlayToneStruct);
	void PrepareNextDtmf();
	void SetDtmfSession(BOOL isDtmfSession);
	void PrintDtmfBuffer();

	void SetSilentStream(BOOL bSilentStream) { m_bSendSilentStream = bSilentStream;}
protected:
	string ChannelData();
	int SetupMediaFile();

	int GetTimeToNextTransmission(DWORD timeStamp);
	DWORD GetTimeToNextTransmissionMl(DWORD timeStamp);

	void RestartMediaTx() {};


private:
	EAudioTone m_dtmfBuffer[MAX_DTMF_LEN];
	int m_numOfDtmf;
	int m_currDtmfIndex;

	BYTE* m_saveMediaFileBuffer;
	int m_saveFileSize;
	int m_saveReadInd;

	BOOL m_bSendSilentStream;

	PDECLAR_MESSAGE_MAP
};


#endif /*VAUDIOCHANNEL_H_*/
