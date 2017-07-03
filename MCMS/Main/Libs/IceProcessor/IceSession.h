#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

// IceSession.h
// Ami Noy

#ifndef ICESESSION_H_
#define ICESESSION_H_

#include <memory>
#include "IceCmReq.h"
#include "auto_array.h"
#include "AnyFirewallEngine_dll.h"

using std::string;

class IceManager;
class IceSession;
class CMplMcmsProtocol;
class IceMplMsgSender;

class CMediaCallback : public IAfCallbackHandler 
{ 
	public:
	
	CAnyFirewallEngine 	*m_pAFEngine;
	
	CMediaCallback(CAnyFirewallEngine* pAFEngine) {m_pAFEngine = pAFEngine;}
	void HandleCallback(int, char*, int, const CAfStdString&, int, const void*); 	
};

class CSessionCallback : public IAfCallbackHandler 
{ 
	public:
	
	IceSession *m_pIceSession;
	
	CSessionCallback(IceSession *pIceSession){m_pIceSession = pIceSession;}
	void HandleCallback(int, char*, int, const CAfStdString&, int, const void*); 	
};

class IceSession
{
public:
	IceSession(CAnyFirewallEngine*,int, const IceMplMsgSender &);
	~IceSession();
	
	
	bool		Offer(CMplMcmsProtocol &mplMsg);
	bool		Close();
	bool		Answer(CMplMcmsProtocol &mplMsg);
	bool		MakeAnswerNoCreate(CMplMcmsProtocol &mplMsg);
	bool		MakeOfferNoCreate(CMplMcmsProtocol &mplMsg);
	bool		ProcessAnswer(char*);
	
	//void 	GetSdp(char*);	
	//int		GetSdpSize();
	
	const std::string& GetLocalSdp() const
	{
		return m_sMediaSdpInfo;
	};

	void 	SendReinvite();
	void		SetIceCheckCompleteInd(int event);

	void StoreLastMplMcmsMsg(const CMplMcmsProtocol &mplMsg);

	void SetAudioRtpCallBack(IAfCallbackHandler* pHandler);
	void SetVideoRtpCallBack(IAfCallbackHandler* pHandler);
	
protected:
	//static void *SessionRecvThread(void*);
	//void 	SessionRecvThreadImpl();
	
	auto_array<char> BuildIceSdpIndByReq(const ICE_GENERAL_REQ_S &ICEreq,
										const std::string &localSdp, int &size) const;
	
	void		AnswerNoCreate(const char *pRemoteSdp);
	//bool 	ModifySessionAnswerReq(mcIceChannelParams *pChannel, const char*);
	void		OfferNoCreate();
	//bool 	ModifySessionOfferReq(mcIceChannelParams *pChannel);
	bool ModifySession(mcIceChannelParams *pChannel);
	
	bool		Create(mcIceChannelParams*);
	void 	CloseChannels();
	void CheckChannelsExistence();

	void PrintLocalAddress();
	
//	pthread_t m_iSessionThreadID;
//	pthread_t m_iAudioThreadID;
//	pthread_t m_iVideoThreadID;
//	pthread_t m_iFeccThreadID;
//	pthread_t m_iContentThreadID;

private:
	//int		m_connId;
	CAnyFirewallEngine 	*m_pAFEngine;
	int		m_sessionId;
	int		m_sdpSize;
	
	int m_iAudioChannelRTP;		// The RTP Audio AFE channel
	int m_iAudioChannelRTCP;	// The RTCP Audio AFE channel
	int m_iVideoChannelRTP;		// The RTP Video AFE channel
	int m_iVideoChannelRTCP;	// The RTCP Video AFE channel
	int m_iFeccChannelRTP;		// The RTP Fecc AFE channel
	int m_iFeccChannelRTCP;		// The RTCP Fecc AFE channel
	int m_iContentChannelRTP;	// The RTP Content AFE channel
	int m_iContentChannelRTCP;	// The RTCP Content AFE channel
	
	int m_iTextChannel;		// The text messaging channel
	int m_iTimeChannel;		// The time channel (the time is sent periodically between peers)
	//int m_iControlChannel;	// The control loopback channel for AFE
	
	int m_iSession;         // The session
		
	std::string	m_sMediaSdpInfo;
	
	bool m_bReinviteSent;

	bool m_bCaller;

	// Indicates if user is in a call or not
	//	Call means that we can now send/recv data
	bool m_bInCall;
	
	CSessionCallback 	*m_SessionCallback;	
	IAfCallbackHandler	*m_pAudioRtpCallback;
	IAfCallbackHandler	*m_pAudioRtcpCallback;
	IAfCallbackHandler	*m_pVideoRtpCallback;
	IAfCallbackHandler	*m_pVideoRtcpCallback;

	std::auto_ptr<CMplMcmsProtocol>	m_lastMplMcmsMsg;
	const IceMplMsgSender &m_MplMsgSender;
};

#endif /*ICESESSION_H_*/

#endif	//__DISABLE_ICE__
