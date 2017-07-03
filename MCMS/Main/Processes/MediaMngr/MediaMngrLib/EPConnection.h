#ifndef ENDPOINTCONNECTION_H_
#define ENDPOINTCONNECTION_H_

#include "StateMachine.h"
#include "TaskMediaChannel.h"
#include "MediaChannel.h"
#include "VideoChannel.h"
#include "AudioChannel.h"
#include "ContentChannel.h"

#include "MediaMngr.h"
#include "MplMcmsProtocol.h"
#include "H221.h"
#include "H264.h"
#include "VideoApiDefinitionsStrings.h"
#include "VideoStructs.h"

#define NO_DW_VALUE			0xFFFFFFFF


using namespace std;

//   EXTERNALS:
// Media Channels Tasks entry points
extern "C" void VideoOutChannelEntryPoint(void* appParam);
extern "C" void VideoInChannelEntryPoint(void* appParam);

extern "C" void AudioOutChannelEntryPoint(void* appParam);
extern "C" void AudioInChannelEntryPoint(void* appParam);

extern "C" void ContentOutChannelEntryPoint(void* appParam);
extern "C" void ContentInChannelEntryPoint(void* appParam);

extern "C" void FeccOutChannelEntryPoint(void* appParam);
extern "C" void FeccInChannelEntryPoint(void* appParam);

TaskEntryPoint GetTaskMediaChannelEntryPoint(int channelIndex);



class CEPConnection : public CStateMachine
{
CLASS_TYPE_1(CEPConnection, CStateMachine)
public:
	CEPConnection(CTaskApp* pOwnerTask);
	virtual ~CEPConnection();

	virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode) {};
	virtual const char * NameOf() const {return "CEPConnection";}


	virtual void ExecuteEventMessage(CMplMcmsProtocol* pMplProtocol);

	//api transactions
	void ExecuteAPIMessage(CSegment* pSeg);

	TaskEntryPoint GetTaskEntryPoint(int channelIndex);


	//media param
	string GetVideoFileNameTx() { return m_sVideoFileNameTx;}
	string GetAudioFileNameTx() { return m_sAudioFileNameTx;}
	string GetContentFileNameTx() { return m_sContentFileNameTx;}

	string GetVideoFileNameRx() { return m_sVideoFileNameRx;}
	string GetAudioFileNameRx() { return m_sAudioFileNameRx;}
	string GetContentFileNameRx() { return m_sContentFileNameRx;}

	void SetSilentStream(BOOL bSilentStream) { m_bSendSilentStream = bSilentStream;}

protected:
	CTaskApi*    m_pMediaChannelApiArr[NUM_OF_MEDIA_CHANNELS];

	BOOL m_bSendSilentStream;

	//Video variables
	EVideoProtocol	m_eVideoProtocol;
	TVideoParam		m_tVideoParam;

	//Content
	APIU32 m_contentBitrate;
	
	//Incoming channel params
	TIncomingChannelParam	m_tIncomingChannelParam;
	
	PDECLAR_MESSAGE_MAP

//private:
protected:
	int GetMediaChannelIndex(kChanneltype channelType, cmCapDirection channelDirection);

	//H264
	void SetH264VideoParam(ENCODER_PARAM_S* pEncoderStruct);
	void SetH264Resolution(int frameSizeMacroBlock);
	void SetH264FrameRate(int numMacroBlockPerSec);

	//H263
	void SetH263VideoParam(ENCODER_PARAM_S* pEncoderStruct);
	void SetH263ResolutionFrameRate(H263_H261_VIDEO_PARAM_S H263Video);



	//end point media param
	///////////////////////////////////
	//file names to transmit
	string m_sVideoFileNameTx;
	string m_sAudioFileNameTx;
	string m_sContentFileNameTx;

	//file names to save - if string is empty do NOT save
	string m_sVideoFileNameRx;
	string m_sAudioFileNameRx;
	string m_sContentFileNameRx;

};


////////////////////////////////////////////////////


class CEPAccess : public CStateMachine
{
CLASS_TYPE_1(CEPAccess,CStateMachine)
public:
	CEPAccess(CTaskApp* pOwnerTask);
	virtual ~CEPAccess();

	// overrides
	virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode) {};
	virtual const char * NameOf() const {return "CEPAccess";}

	DWORD GetEpRsrcId() const;
	void SetEpRsrcId(const WORD epRscId);

	int GetIndex() const;
	void SetIndex(const int index);

	CEPConnection* GetEPConnectionPtr();
	void SetEPConnectionPtr(CEPConnection* ptr);

	virtual void ExecuteEventMessage(CMplMcmsProtocol* pMplProtocol);


	// for Media Param transaction
	DWORD GetMonitorPartyId() const;
	void SetMonitorPartyId(const DWORD monitorPartyId);
	DWORD GetMonitorConfId() const;
	void SetMonitorConfId(const DWORD monitorConfId);

	void ExecuteAPIMessage(CSegment* pSeg);

	virtual void InvokeEPGlobalTxTimer() = 0;
	virtual void InvokeEPResetChannel(BYTE channelType) = 0;
	virtual void InvokeEPVideoUpdatePic() = 0;
	virtual void InvokeEPAudioUpdateChannel() = 0;

protected:
	DWORD	m_epRsrcId;  // party Id
	int		m_index;	 // party index at party array

	CEPConnection* m_pEPConnection;

	// for Media Param transaction
	DWORD m_monitorPartyId;
	DWORD m_monitorConfId;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Classes for Object implementation
////////////////////////////////////////////////////
////////////////////////////////////////////////////

class CEPConnectionObj : public CEPConnection
{
CLASS_TYPE_1(CEPConnectionObj, CEPConnection)
public:
	CEPConnectionObj(CTaskApp* pOwnerTask);
	virtual ~CEPConnectionObj();

	virtual const char * NameOf() const {return "CEPConnectionObj";}

	void ExecuteEventMessage(CMplMcmsProtocol* pMplProtocol);
	void ExecuteAPIMessage(CSegment* pSeg);
	void InvokeEPGlobalTxTimer();
	void InvokeEPResetChannel(BYTE channelType);
	void InvokeEPVideoUpdatePic();
	void InvokeEPAudioUpdateChannel();
	
	void SetParticipantTicket(string participantTicket);
	string GetParticipantTicket();

protected:
	void OnRtpUpdatePortOpenChannelReq( CMplMcmsProtocol* pMplProtocol );
	void UpdateMediaChannelParams( CMediaChannel* pMediaChannel );

protected:

	CMediaChannel*	m_pMediaChannelArr[NUM_OF_MEDIA_CHANNELS];

	//participant ticket [arrayIndex, participantIndex]
	string 	m_participantTicket;

	DWORD		m_boardId;
	DWORD		m_subBoardId;
	DWORD		m_unitId;
	DWORD		m_conferenceId;
	DWORD		m_partyId;
	DWORD		m_connectionId;
	
	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////



class CEPAccessObj : public CEPAccess
{
CLASS_TYPE_1(CEPAccessObj,CEPAccess)
public:
	CEPAccessObj(CTaskApp* pOwnerTask);
	virtual ~CEPAccessObj();

	virtual const char * NameOf() const {return "CEPAccessObj";}

	void ExecuteEventMessage(CMplMcmsProtocol* pMplProtocol);
	void InvokeEPGlobalTxTimer();
	void InvokeEPResetChannel(BYTE channelType);
	void InvokeEPVideoUpdatePic();
	void InvokeEPAudioUpdateChannel();

	void PrepareParticipantTicket();
	string GetParticipantTicket();

protected:

	//participant ticket [arrayIndex, participantIndex]
	string m_participantTicket;

	PDECLAR_MESSAGE_MAP
};



#endif /*ENDPOINTCONNECTION_H_*/
