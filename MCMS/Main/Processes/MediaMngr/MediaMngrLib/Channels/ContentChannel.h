#ifndef CONTENTCHANNEL_H_
#define CONTENTCHANNEL_H_

#include "StateMachine.h"
#include "MplMcmsProtocol.h"
#include "DataTypes.h"
#include "IpChannelParams.h"
#include "AudRequestStructs.h"
#include "VideoStructs.h"
#include "MediaMngrCfg.h"

#include "MediaMngr.h"
#include "UdpSocket.h"

#include "VideoChannel.h"
//#include "AudioChannel.h"


#include "IpEncryptionDefinitions.h"

//////////////////////////////////////////
//CContentChannel
//////////////////////////////////////////

class CContentChannel : public CVideoChannel
{
CLASS_TYPE_1(CContentChannel, CVideoChannel)
public:
	CContentChannel();
	CContentChannel(CTaskApp* pOwnerTask, INT32 channelDirection);
	virtual ~CContentChannel();

	virtual void* GetMessageMap();
	virtual const char * NameOf() const;
	virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);


protected:
	string ChannelData();
	int SetupMediaFile();


	void RestartMediaTx() {};

	string GetBitrateStr(DWORD bitrate);

	PDECLAR_MESSAGE_MAP
};
#endif /*CONTENTCHANNEL_H_*/
