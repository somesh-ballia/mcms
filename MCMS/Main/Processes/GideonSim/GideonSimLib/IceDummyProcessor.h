#ifndef _ICE_DUMMY_PROC_H_
#define _ICE_DUMMY_PROC_H_

#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#include <memory>
#include <map>
#include "auto_array.h"
#include "IceCmReq.h"
#include "IceCmInd.h"
#include "IceProcessor.h"

using std::string;

#define MaxCandidatesInMediaLine 10


class CMplMcmsProtocol;
class CSegment;

struct ICE_SessionInfo_S
{
	char icePwd[256];
	char iceUfrag[256];
	char iceOptions[256];

	int rtpAudioPort;
	int rtcpAudioPort;
	char rtpAudioCandidate[MaxCandidatesInMediaLine][256];
	char rtcpAudioCandidate[MaxCandidatesInMediaLine][256];
	char remoteAudioCandidate[256];

	char audioRtpChosenCandIP[128];
	char audioRtcpChosenCandIP[128];
	int audioRtpChosenCandPort;
	int audioRtcpChosenCandPort;

	int rtpVideoPort;
	int rtcpVideoPort;
	char rtpVideoCandidate[MaxCandidatesInMediaLine][256];
	char rtcpVideoCandidate[MaxCandidatesInMediaLine][256];
	char remoteVideoCandidate[256];

	char videoRtpChosenCandIP[128];
	char videoRtcpChosenCandIP[128];
	int videoRtpChosenCandPort;
	int videoRtcpChosenCandPort;

	int rtpFeccPort;
	int rtcpFeccPort;
	char rtpFeccCandidate[MaxCandidatesInMediaLine][256];
	char rtcpFeccCandidate[MaxCandidatesInMediaLine][256];
	char remoteFeccCandidate[256];

	char feccRtpChosenCandIP[128];
	char feccRtcpChosenCandIP[128];
	int feccRtpChosenCandPort;
	int feccRtcpChosenCandPort;
};



class IceDummyProcessor : public IceProcessor
{
public:
	bool Init(CMplMcmsProtocol &mplMsg);

	bool MakeOffer(CMplMcmsProtocol &mplMsg, int &iceSessionID);
	bool MakeAnswer(CMplMcmsProtocol &mplMsg, int &iceSessionID);
	bool MakeAnswerNoCreate(CMplMcmsProtocol &mplMsg);
	bool MakeOfferNoCreate(CMplMcmsProtocol &mplMsg);
	bool CloseSession(CMplMcmsProtocol &mplMsg);
	//bool ModifySession(void*, int opcode);
	bool ProcessAnswer(CMplMcmsProtocol &mplMsg);
	void OnTimerCheckComplete (CSegment * pParam);

	IceDummyProcessor(IceMplMsgSender &mplSender):IceProcessor(mplSender), m_lastIceSessionID(0) {};
	~IceDummyProcessor();
	
	static std::string BuildCandidateSDP(kChanneltype chanType, int rtp, int rtcp);
	static auto_array<char> BuildIceReInviteInd(const ICE_GENERAL_REQ_S &ICEreq, const struct ICE_SessionInfo_S &remoteIceArr, int &size);
	static std::string BuildChosenCandidateSDP(kChanneltype chanType, int rtp, int rtcp, const struct ICE_SessionInfo_S &remoteIceArr);
	
protected:
	bool BuildCheckCompleteInd (ICE_CHECK_COMPLETE_IND_S *CheckCompleteInd, int iceSessionIndex);

private:
	std::map<int, ICE_SessionInfo_S*> m_IcePartiesRemoteArr;
	int m_lastIceSessionID;
};



#endif	//__DISABLE_ICE__

#endif

