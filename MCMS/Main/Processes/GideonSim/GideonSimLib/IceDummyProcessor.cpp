// Author: Victor
#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#include <assert.h>
#include <stdarg.h>

#include "MplMcmsStructs.h"
#include "MplMcmsProtocol.h"
//#include "Opcodes.h"
//#include "Opcodes.h"
#include "DataTypes.h"
//#include "CommonDefs.h"
#include "IpChannelParams.h"
#include "IceDummyProcessor.h"
//#include "IceDefs.h"
#include "auto_array.h"
#include "IceCmReq.h"
#include "IceCmInd.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "Segment.h"
#include "IceLogAdapter.h"

#define itoa(value, string) sprintf(string, "%d", value)


static const char *initICE_SDP="v=0\r\n\
o=Eyeball 1928321677 1452934389 IN IP4 172.22.192.49\r\n\
s=Eyeball AnyFirewall Engine Dec 29 2009\r\n\
c=IN IP4 172.22.192.49\r\n\
t=0 0\r\n\
a=ice-pwd:6JoJtyNaaj6oARyC6JoJtyNa\r\n\
a=ice-ufrag:j6oA\r\n";

static const char *CandidateFormat="m=%s %d RTP/AVP 0\r\n\
a=candidate:2 1 UDP 2130706431 172.22.192.49 %d typ host\r\n\
a=candidate:1 1 UDP 16777215 172.26.129.227 %d typ relay raddr 172.26.129.220 rport %d\r\n\
a=rtcp:%d\r\n\
a=candidate:2 2 UDP 2130706430 172.22.192.49 %d typ host\r\n\
a=candidate:1 2 UDP 16777214 172.26.129.227 %d typ relay raddr 172.26.129.220 rport %d\r\n";

static const char *ChosenCandidateFormat="m=%s %d RTP/AVP 0\r\n\
a=candidate:2 1 UDP 2130706431 172.22.192.49 %d typ host\r\n\
a=rtcp:%d\r\n\
a=candidate:2 2 UDP 2130706430 172.22.192.49 %d typ host\r\n\
a=remote-candidates:1 %s %d 2 %s %d\r\n";

static const char *parseCandidateFormat="%d %d %s %d %s %d typ %s";

#if 1

static int IceGetGenericAtt(char *pSdp, char *attr, char *pCallAttr)
{
                char *pStartStr = NULL;
                char format[256] = "";
                sprintf(format, "\r\n%s", attr);

                pStartStr = strstr(pSdp, format);
                if (pStartStr) {
                                sprintf(format, "\r\n%s %%s", attr);
                                if (sscanf(pStartStr, format, pCallAttr))
                                                return strlen(pCallAttr);
                }

                return 0;
}

static int IceGetConnIp(char *pSdp)
{
                char ip[256] = "";

                if (IceGetGenericAtt(pSdp, "c=IN IP4", ip)) {
                                //sipSimConfigParamSt.localRtpAddress.addr.v4.ip = htonl(inet_addr(ip));
                                return strlen(ip);
                }
                return 0;
}



static int IceGetMediaAttr(
                                char *pSdp,
                                char *mline,
                                int *pRtpPort,
                                int *pRtcpPort,
                                char (&rtpCandidate)[MaxCandidatesInMediaLine][256],
                                char (&rtcpCandidate)[MaxCandidatesInMediaLine][256],
                                char (&remoteCandidate)[256])
{
                char format[256] = "";
                char *pStartStr = NULL;
                char *pEndStr = NULL;
                int i;

                sprintf(format, "\r\n%s", mline);
                pStartStr = strstr(pSdp, format);

                if (pStartStr) {
                                pEndStr = strstr(pStartStr + 1, "\r\nm=");
                                if (pEndStr)
                                                pEndStr[2] = '\0';
                }
                else
                                return 0;

                //Get rtp port
		sprintf(format, "\r\n%s %%d", mline);
		sscanf(pStartStr, format, pRtpPort);

                //Get rtp candidate
                char *pCandStart=pStartStr;
		int rtpCandIndex=0, rtcpCandIndex=0;
                //pStartStr = strstr(pStartStr + 1, "\r\na=candidate:");
		do {
			pCandStart=strstr(pCandStart + 1, "\r\na=candidate:");
			if(!pCandStart)
				break;

			int foundatioin=0, componentID=0;
			sscanf(pCandStart, "\r\na=candidate:%d %d ", &foundatioin, &componentID);
			if(componentID==1) {//RTP candidate
				char *pEOL = strstr(pCandStart + 1, "\r\n");
				if (pEOL) {
					pEOL[0] = '\0';
					strcpy(rtpCandidate[rtpCandIndex++], &pCandStart[strlen("\r\na=candidate:")]);
					pEOL[0] = '\r';
				}

				pCandStart += strlen("\r\na=candidate:") + strlen(rtpCandidate[i]);

				if(rtpCandIndex>=MaxCandidatesInMediaLine)
					--rtpCandIndex;//overwrite the last candidate
			}
			else if(componentID==2) {
				char *pEOL = strstr(pCandStart + 1, "\r\n");
				if (pEOL) {
					pEOL[0] = '\0';
					strcpy(rtcpCandidate[rtcpCandIndex++], &pCandStart[strlen("\r\na=candidate:")]);
					pEOL[0] = '\r';
				}

				pCandStart += strlen("\r\na=candidate:") + strlen(rtpCandidate[i]);

				if(rtcpCandIndex>=MaxCandidatesInMediaLine)
					--rtcpCandIndex;//overwrite the last candidate
			}
		}
		while(pCandStart);

                //Get rtcp port
		char *pRtcpPortStr=strstr(pStartStr, "\r\na=rtcp:");
		if (pRtcpPortStr) {
			sscanf(pRtcpPortStr, "\r\na=rtcp:%d\r\n", pRtcpPort);
		}

		//There should be only one remote candidate
                //Get remote candidate
                char *remoteCandStr=strstr(pStartStr, "\r\na=remote-candidates:");
		if(remoteCandStr) {
			char *pEOL = strstr(remoteCandStr + 1, "\r\n");
			if (pEOL) {
				pEOL[0] = '\0';
				strcpy(remoteCandidate, &remoteCandStr[strlen("\r\na=remote-candidates:")]);
				pEOL[0] = '\r';
			}
		}

                if (pEndStr)
                                pEndStr[2] = 'm';
                return 0;
}

#endif

//bool m_bAFEConnectivityCheckOk;
static int IceParseSdp(
                struct ICE_SessionInfo_S             &iceSess,
                int                           sdpSize,
                char                       *pIceSdp)

{
                int           len;
                len = IceGetConnIp(pIceSdp);
                len = IceGetGenericAtt(pIceSdp, "a=ice-pwd:", iceSess.icePwd);
                len = IceGetGenericAtt(pIceSdp, "a=ice-ufrag:", iceSess.iceUfrag);
                len = IceGetGenericAtt(pIceSdp, "a=ice-options:", iceSess.iceOptions);
                len = IceGetMediaAttr(pIceSdp, "m=audio", &iceSess.rtpAudioPort, &iceSess.rtcpAudioPort, iceSess.rtpAudioCandidate, iceSess.rtcpAudioCandidate, iceSess.remoteAudioCandidate);
                len = IceGetMediaAttr(pIceSdp, "m=video", &iceSess.rtpVideoPort, &iceSess.rtcpVideoPort, iceSess.rtpVideoCandidate, iceSess.rtcpVideoCandidate, iceSess.remoteVideoCandidate);
                len = IceGetMediaAttr(pIceSdp, "m=application", &iceSess.rtpFeccPort, &iceSess.rtcpFeccPort, iceSess.rtpFeccCandidate, iceSess.rtcpFeccCandidate, iceSess.remoteFeccCandidate);

		int foundation=0;
		int componentId=0;
		char transport[32]={0};
		long priority=0;
		char type[32]={0};

		sscanf(iceSess.rtpAudioCandidate[0], parseCandidateFormat,
			&foundation, &componentId, transport, &priority,
			iceSess.audioRtpChosenCandIP, &iceSess.audioRtpChosenCandPort, type);
		sscanf(iceSess.rtcpAudioCandidate[0], parseCandidateFormat,
			&foundation, &componentId, transport, &priority,
			iceSess.audioRtcpChosenCandIP, &iceSess.audioRtcpChosenCandPort, type);

		sscanf(iceSess.rtpVideoCandidate[0], parseCandidateFormat,
			&foundation, &componentId, transport, &priority,
			iceSess.videoRtpChosenCandIP, &iceSess.videoRtpChosenCandPort, type);
		sscanf(iceSess.rtcpVideoCandidate[0], parseCandidateFormat,
			&foundation, &componentId, transport, &priority,
			iceSess.videoRtcpChosenCandIP, &iceSess.videoRtcpChosenCandPort, type);

  		sscanf(iceSess.rtpFeccCandidate[0], parseCandidateFormat,
			&foundation, &componentId, transport, &priority,
			iceSess.feccRtpChosenCandIP, &iceSess.feccRtpChosenCandPort, type);
		sscanf(iceSess.rtcpFeccCandidate[0], parseCandidateFormat,
			&foundation, &componentId, transport, &priority,
			iceSess.feccRtcpChosenCandIP, &iceSess.feccRtcpChosenCandPort, type);

		return 0;
}


std::string IceDummyProcessor::BuildCandidateSDP(kChanneltype chanType, int rtp, int rtcp)
{
	std::string media;
	switch(chanType)
	{
	case kIpAudioChnlType:
		media="audio";
		break;
	case kIpVideoChnlType:
		media="video";
		break;
	case kIpFeccChnlType:
		media="application";
		break;
	//not supported yet
	case kIpContentChnlType:
	default:
		return "";
	}

	int relayRtpPort=rtp+50000;
	int relayRtcpPort=rtcp+50000;
	int mappedRtpPort=rtp+49600;
	int mappedRtcpPort=rtcp+49600;
	auto_array<char> candidateSDP(new char[2*strlen(CandidateFormat)]);//I think it should be big enough
	sprintf(candidateSDP.c_array(), CandidateFormat
		, media.c_str(), rtp
		, rtp
		, relayRtpPort, mappedRtpPort
		, rtcp
		, rtcp
		, relayRtcpPort, mappedRtcpPort
		);

	return candidateSDP.c_array();
};

std::string IceDummyProcessor::BuildChosenCandidateSDP(kChanneltype chanType, int rtp, int rtcp, const struct ICE_SessionInfo_S &remoteIceArr)
{
	const char *chosenRemoteRtpIP, *chosenRemoteRtcpIP;
	int chosenRemoteRtpPort, chosenRemoteRtcpPort;

	std::string media;
	switch(chanType)
	{
	case kIpAudioChnlType:
		media="audio";
		chosenRemoteRtpIP=remoteIceArr.audioRtpChosenCandIP;
		chosenRemoteRtcpIP=remoteIceArr.audioRtcpChosenCandIP;
		chosenRemoteRtpPort=remoteIceArr.audioRtpChosenCandPort;
		chosenRemoteRtcpPort=remoteIceArr.audioRtcpChosenCandPort;
		break;
	case kIpVideoChnlType:
		media="video";
		chosenRemoteRtpIP=remoteIceArr.videoRtpChosenCandIP;
		chosenRemoteRtcpIP=remoteIceArr.videoRtcpChosenCandIP;
		chosenRemoteRtpPort=remoteIceArr.videoRtpChosenCandPort;
		chosenRemoteRtcpPort=remoteIceArr.videoRtcpChosenCandPort;
		break;
	case kIpFeccChnlType:
		media="application";
		chosenRemoteRtpIP=remoteIceArr.feccRtpChosenCandIP;
		chosenRemoteRtcpIP=remoteIceArr.feccRtcpChosenCandIP;
		chosenRemoteRtpPort=remoteIceArr.feccRtpChosenCandPort;
		chosenRemoteRtcpPort=remoteIceArr.feccRtcpChosenCandPort;
		break;
	//not supported yet
	case kIpContentChnlType:
	default:
		return "";
	}

	int relayRtpPort=rtp+50000;
	int relayRtcpPort=rtcp+50000;
	int mappedRtpPort=rtp+49600;
	int mappedRtcpPort=rtcp+49600;
	auto_array<char> candidateSDP(new char[3*strlen(ChosenCandidateFormat)]);//it should be big enough
	sprintf(candidateSDP.c_array(), ChosenCandidateFormat
		, media.c_str(), rtp
		, rtp
		, rtcp
		, rtcp
		, chosenRemoteRtpIP, chosenRemoteRtpPort
		, chosenRemoteRtcpIP, chosenRemoteRtcpPort);

	return candidateSDP.c_array();
};

auto_array<char> IceDummyProcessor::BuildIceReInviteInd(const ICE_GENERAL_REQ_S &ICEreq, const struct ICE_SessionInfo_S &remoteIceArr, int &size)
{
	ICE_GENERAL_IND_S ICEind;
	memset(&ICEind,0,sizeof(ICEind));

	ICEind.status=STATUS_OK;
	ICEind.ice_session_index=ICEreq.ice_session_index;

	std::string SDP_string=initICE_SDP;
	for(int i=0;i<NumOfMediaTypes-1;++i) {
		if(ICEreq.candidate_list[i].channelType==kIpAudioChnlType) {
			ICEind.ice_channels_id.ice_audio_rtp_id=1;
			ICEind.ice_channels_id.ice_audio_rtcp_id=2;

			//ICEind.ice_local_ports.audioRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.audioRtcpPort=ICEreq.candidate_list[i].rtcp_port;

			SDP_string+=BuildChosenCandidateSDP(kIpAudioChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]
									,remoteIceArr);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpVideoChnlType) {
			ICEind.ice_channels_id.ice_video_rtp_id=3;
			ICEind.ice_channels_id.ice_video_rtcp_id=4;

			//ICEind.ice_local_ports.videoRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.videoRtcpPort=ICEreq.candidate_list[i].rtcp_port;

			SDP_string+=BuildChosenCandidateSDP(kIpVideoChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]
									,remoteIceArr);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpFeccChnlType) {
			ICEind.ice_channels_id.ice_data_rtp_id=5;
			ICEind.ice_channels_id.ice_data_rtcp_id=6;

			//ICEind.ice_local_ports.dataRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.dataRtcpPort=ICEreq.candidate_list[i].rtcp_port;

			SDP_string+=BuildChosenCandidateSDP(kIpFeccChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]
									,remoteIceArr);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpContentChnlType) {
			ICEind.ice_channels_id.ice_content_rtp_id=7;
			ICEind.ice_channels_id.ice_content_rtcp_id=8;

			//ICEind.ice_local_ports.contentRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.contentRtcpPort=ICEreq.candidate_list[i].rtcp_port;

			SDP_string+=BuildChosenCandidateSDP(kIpContentChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]
									,remoteIceArr);
		}
	}

	ICEind.sdp_size=SDP_string.size()+1;//include '\0' size

	size=sizeof(ICEind)+SDP_string.size();
	auto_array<char> msgbuf(new char[size]);
	memset(msgbuf.c_array(), 0, size);

	ICE_GENERAL_IND_S *pICEInd=(ICE_GENERAL_IND_S*)msgbuf.c_array();
	*pICEInd=ICEind;
	strcpy(pICEInd->sdp, SDP_string.c_str());

	return msgbuf;
};

IceDummyProcessor::~IceDummyProcessor()
{
	for(std::map<int, ICE_SessionInfo_S*>::iterator pos=m_IcePartiesRemoteArr.begin();
		pos!=m_IcePartiesRemoteArr.end(); ++pos) {
		delete pos->second;
	}
};

bool IceDummyProcessor::Init(CMplMcmsProtocol &mplMsg)
{
	ICE_INIT_IND_S  iceInitIND;
	ICE_INIT_REQ_S *iceInitREQ=(ICE_INIT_REQ_S*)mplMsg.GetData();

	memset(&iceInitIND,0,sizeof(iceInitIND));
	iceInitIND.req_id=iceInitREQ->ice_servers.req_id;
	iceInitIND.status= STATUS_OK;
	iceInitIND.STUN_Pass_status=eIceServerUnavailble;
	iceInitIND.STUN_udp_status=eIceServerUnavailble;
	iceInitIND.STUN_tcp_status=eIceServerUnavailble;
	iceInitIND.Relay_udp_status=eIceInitOk;
	iceInitIND.Relay_tcp_status=eIceServerUnavailble;
	iceInitIND.fw_type=eFwTypeUdp;

	mplMsg.AddCommonHeader(ICE_INIT_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	mplMsg.AddData(sizeof(iceInitIND),(char*)(&iceInitIND));

	SendForMplApi(mplMsg);

	return true;
};

bool IceDummyProcessor::BuildCheckCompleteInd (ICE_CHECK_COMPLETE_IND_S *CheckCompleteInd, int iceSessionIndex)
{
	std::map<int, ICE_SessionInfo_S*>::iterator pos=m_IcePartiesRemoteArr.find(iceSessionIndex);
	if(pos==m_IcePartiesRemoteArr.end()
		|| !pos->second) {
		ICE_LOG_TRACE("BuildCheckCompleteInd failed! No matched ICE_SessionInfo_S is found by ice session id:%d", iceSessionIndex);
		return false;
	}

	ICE_SessionInfo_S &sessInfo=*pos->second;

	CheckCompleteInd->status = STATUS_OK;
	CheckCompleteInd->ice_session_index = iceSessionIndex;

	char connectType[8] = "local";

	//Audio
	CheckCompleteInd->chosen_candidates[0].local_candidate.mediaType = kIpAudioChnlType;
	strncpy (CheckCompleteInd->chosen_candidates[0].local_candidate.type, connectType, sizeof (connectType));
	strncpy (CheckCompleteInd->chosen_candidates[0].local_candidate.ip,
	         sessInfo.rtpAudioCandidate[0],CandidateIpLen);
	CheckCompleteInd->chosen_candidates[0].local_candidate.port = sessInfo.rtpAudioPort;

	CheckCompleteInd->chosen_candidates[0].remote_candidate.mediaType = kIpAudioChnlType;
	strncpy (CheckCompleteInd->chosen_candidates[0].remote_candidate.type, connectType, sizeof (connectType));
	strncpy (CheckCompleteInd->chosen_candidates[0].remote_candidate.ip,
	         sessInfo.remoteAudioCandidate, CandidateIpLen);
	CheckCompleteInd->chosen_candidates[0].remote_candidate.port = sessInfo.rtpAudioPort;

	//Video
	CheckCompleteInd->chosen_candidates[1].local_candidate.mediaType = kIpVideoChnlType;
	strncpy (CheckCompleteInd->chosen_candidates[1].local_candidate.type, connectType, sizeof (connectType));
	strncpy (CheckCompleteInd->chosen_candidates[1].local_candidate.ip,
	         sessInfo.rtpVideoCandidate[0],CandidateIpLen);
	CheckCompleteInd->chosen_candidates[1].local_candidate.port = sessInfo.rtpVideoPort;

	CheckCompleteInd->chosen_candidates[1].remote_candidate.mediaType = kIpVideoChnlType;
	strncpy (CheckCompleteInd->chosen_candidates[1].remote_candidate.type, connectType, sizeof (connectType));
	strncpy (CheckCompleteInd->chosen_candidates[1].remote_candidate.ip,
	         sessInfo.remoteVideoCandidate, CandidateIpLen);
	CheckCompleteInd->chosen_candidates[1].remote_candidate.port = sessInfo.rtpVideoPort;

	//FECC
	CheckCompleteInd->chosen_candidates[2].local_candidate.mediaType = kIpFeccChnlType;
	strncpy (CheckCompleteInd->chosen_candidates[2].local_candidate.type, connectType, sizeof (connectType));
	strncpy (CheckCompleteInd->chosen_candidates[2].local_candidate.ip,
	         sessInfo.rtpFeccCandidate[0],CandidateIpLen);
	CheckCompleteInd->chosen_candidates[2].local_candidate.port = sessInfo.rtpFeccPort;

	CheckCompleteInd->chosen_candidates[2].remote_candidate.mediaType = kIpFeccChnlType;
	strncpy (CheckCompleteInd->chosen_candidates[2].remote_candidate.type, connectType, sizeof (connectType));
	strncpy (CheckCompleteInd->chosen_candidates[2].remote_candidate.ip,
	         sessInfo.remoteFeccCandidate, CandidateIpLen);
	CheckCompleteInd->chosen_candidates[2].remote_candidate.port = sessInfo.rtpFeccPort;

	return true;
}

void IceDummyProcessor::OnTimerCheckComplete (CSegment * pParam)
{
	DWORD sessionId = 0;
	CMplMcmsProtocol rMplProt;
	*pParam >> sessionId;

	rMplProt.DeSerialize(*pParam);

	//Check complete indication
	ICE_CHECK_COMPLETE_IND_S CheckCompleteInd;
	memset (&CheckCompleteInd, 0, sizeof(CheckCompleteInd));
	BuildCheckCompleteInd (&CheckCompleteInd, sessionId);

	rMplProt.AddCommonHeader(ICE_CHECK_COMPLETE_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
	rMplProt.AddData(sizeof(CheckCompleteInd), (char*)&CheckCompleteInd);

	SendForMplApi(rMplProt);
}

#if 0
bool IceDummyProcessor::ModifySession(void *pMsg, int opcode)
{
	return true;
}
#endif

bool IceDummyProcessor::MakeOffer(CMplMcmsProtocol &mplMsg, int &iceSessionID)
{
	ICE_MAKE_OFFER_REQ_S &ICEreq=*((ICE_MAKE_OFFER_REQ_S *) mplMsg.GetData());

	std::string localSdp=initICE_SDP;
	for(int i=0;i<NumOfMediaTypes-1;++i) {
		if(ICEreq.candidate_list[i].channelType==kIpAudioChnlType) {
			localSdp+=BuildCandidateSDP(kIpAudioChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpVideoChnlType) {
			localSdp+=BuildCandidateSDP(kIpVideoChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpFeccChnlType) {
			localSdp+=BuildCandidateSDP(kIpFeccChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpContentChnlType) {
			localSdp+=BuildCandidateSDP(kIpContentChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
	}

	int size;
	ICEreq.ice_session_index=++m_lastIceSessionID;
	iceSessionID=m_lastIceSessionID;
	
	auto_array<char> msgbuf=BuildIceSdpIndByReq(ICEreq, localSdp, size);

	mplMsg.AddCommonHeader(ICE_MAKE_OFFER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	mplMsg.AddData(size,msgbuf.c_array());

	SendForMplApi(mplMsg);
	return true;
}

bool IceDummyProcessor::MakeAnswerNoCreate(CMplMcmsProtocol &mplMsg)
{
	ICE_GENERAL_REQ_S &ICEreq=*((ICE_GENERAL_REQ_S *) mplMsg.GetData());

	std::string localSdp=initICE_SDP;
	for(int i=0;i<NumOfMediaTypes-1;++i) {
		if(ICEreq.candidate_list[i].channelType==kIpAudioChnlType) {
			localSdp+=BuildCandidateSDP(kIpAudioChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpVideoChnlType) {
			localSdp+=BuildCandidateSDP(kIpVideoChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpFeccChnlType) {
			localSdp+=BuildCandidateSDP(kIpFeccChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpContentChnlType) {
			localSdp+=BuildCandidateSDP(kIpContentChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
	}

	int size;
	auto_array<char> msgbuf=BuildIceSdpIndByReq(ICEreq, localSdp, size);

	mplMsg.AddCommonHeader(ICE_MODIFY_SESSION_ANSWER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	mplMsg.AddData(size,msgbuf.c_array());

	SendForMplApi(mplMsg);
	return true;
}

bool IceDummyProcessor::MakeOfferNoCreate(CMplMcmsProtocol &mplMsg)
{
	ICE_GENERAL_REQ_S &ICEreq=*((ICE_GENERAL_REQ_S *) mplMsg.GetData());

	std::string localSdp=initICE_SDP;
	for(int i=0;i<NumOfMediaTypes-1;++i) {
		if(ICEreq.candidate_list[i].channelType==kIpAudioChnlType) {
			localSdp+=BuildCandidateSDP(kIpAudioChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpVideoChnlType) {
			localSdp+=BuildCandidateSDP(kIpVideoChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpFeccChnlType) {
			localSdp+=BuildCandidateSDP(kIpFeccChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpContentChnlType) {
			localSdp+=BuildCandidateSDP(kIpContentChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
	}

	int size;
	auto_array<char> msgbuf=BuildIceSdpIndByReq(ICEreq, localSdp, size);

	mplMsg.AddCommonHeader(ICE_MODIFY_SESSION_OFFER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	mplMsg.AddData(size,msgbuf.c_array());

	SendForMplApi(mplMsg);
	return true;
}

bool IceDummyProcessor::MakeAnswer(CMplMcmsProtocol &mplMsg, int &iceSessionID)
{
	ICE_MAKE_ANSWER_REQ_S &ICEreq=*((ICE_MAKE_ANSWER_REQ_S *) mplMsg.GetData());

	std::string localSdp=initICE_SDP;
	for(int i=0;i<NumOfMediaTypes-1;++i) {
		if(ICEreq.candidate_list[i].channelType==kIpAudioChnlType) {
			localSdp+=BuildCandidateSDP(kIpAudioChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpVideoChnlType) {
			localSdp+=BuildCandidateSDP(kIpVideoChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpFeccChnlType) {
			localSdp+=BuildCandidateSDP(kIpFeccChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
		else if(ICEreq.candidate_list[i].channelType==kIpContentChnlType) {
			localSdp+=BuildCandidateSDP(kIpContentChnlType
									,ICEreq.candidate_list[i].rtp_port[0]
									,ICEreq.candidate_list[i].rtcp_port[0]);
		}
	}

	int size;
	ICEreq.ice_session_index=++m_lastIceSessionID;
	iceSessionID=m_lastIceSessionID;

	auto_array<char> msgbuf=BuildIceSdpIndByReq(ICEreq, localSdp, size);

	mplMsg.AddCommonHeader(ICE_MAKE_ANSWER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	mplMsg.AddData(size,msgbuf.c_array());

	SendForMplApi(mplMsg);
	return true;
}

bool IceDummyProcessor::CloseSession(CMplMcmsProtocol &mplMsg)
{
	ICE_CLOSE_SESSION_REQ_S *pIceCloseSessionReq = (ICE_CLOSE_SESSION_REQ_S *) mplMsg.GetData();

	int	sessionId = pIceCloseSessionReq->ice_session_index;

	std::map<int, ICE_SessionInfo_S*>::iterator pos=m_IcePartiesRemoteArr.find(sessionId);
	if(pos==m_IcePartiesRemoteArr.end()) {
		SendCloseSessionInd(mplMsg);
		return true;
	}

	ICE_SessionInfo_S *sessInfo=pos->second;
	delete sessInfo;
	m_IcePartiesRemoteArr.erase(pos);

	SendCloseSessionInd(mplMsg);

	return true;
}

bool IceDummyProcessor::ProcessAnswer(CMplMcmsProtocol &mplMsg)
{
	ICE_GENERAL_REQ_S *processAnswerReq=(ICE_GENERAL_REQ_S*)mplMsg.GetData();

	{
		ICE_PROCESS_ANSWER_IND_S procAnsInd;
		memset(&procAnsInd, 0, sizeof(procAnsInd));

		procAnsInd.status=STATUS_OK;
		procAnsInd.ice_session_index=processAnswerReq->ice_session_index;

		CMplMcmsProtocol sendAnswerInd(mplMsg);
		sendAnswerInd.AddCommonHeader(ICE_PROCESS_ANSWER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
		sendAnswerInd.AddData(sizeof(procAnsInd), (char*)&procAnsInd);

		SendForMplApi(sendAnswerInd);
	}


	int sessId=processAnswerReq->ice_session_index;

	std::map<int, ICE_SessionInfo_S*>::iterator pos=m_IcePartiesRemoteArr.find(sessId);
	ICE_SessionInfo_S *sessInfo=0;
	if(pos==m_IcePartiesRemoteArr.end()) {
		sessInfo=new ICE_SessionInfo_S;
		m_IcePartiesRemoteArr[sessId]=sessInfo;
	}
	else {
		sessInfo=pos->second;
	}

	IceParseSdp(*sessInfo, processAnswerReq->sdp_size
				,processAnswerReq->sdp);

	{
		//Check complete indication
		ICE_CHECK_COMPLETE_IND_S CheckCompleteInd;
		memset (&CheckCompleteInd, 0, sizeof(CheckCompleteInd));
		BuildCheckCompleteInd (&CheckCompleteInd, processAnswerReq->ice_session_index);

		CMplMcmsProtocol checkCompleteIndMsg(mplMsg);
		checkCompleteIndMsg.AddCommonHeader(ICE_CHECK_COMPLETE_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
		checkCompleteIndMsg.AddData(sizeof(CheckCompleteInd), (char*)&CheckCompleteInd);
		SendForMplApi(checkCompleteIndMsg);

		//build and send   Re-INVITE  Indication
		int size;
		auto_array<char> msgbuf=BuildIceReInviteInd(*processAnswerReq, *sessInfo, size);

		CMplMcmsProtocol sendReinvInd(mplMsg);
		sendReinvInd.AddCommonHeader(ICE_REINVITE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

		sendReinvInd.AddData(size,msgbuf.c_array());

		SendForMplApi(sendReinvInd);
	}

	return true;
}


#endif	//__DISABLE_ICE__
