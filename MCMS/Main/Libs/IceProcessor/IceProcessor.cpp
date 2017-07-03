//Author: Victor
#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#include <string>
#include "IceProcessor.h"
#include "IceCmInd.h"
#include "MplMcmsProtocol.h"
#include "StatusesGeneral.h"
#include "IpChannelParams.h"
#include "OpcodesMcmsCardMngrICE.h"

bool IceProcessor::HandleIceMplMsg(CMplMcmsProtocol &mplMsg)
{
	OPCODE opCode = mplMsg.getOpcode();
	int sessID=0;

	switch(opCode) {
		case ICE_INIT_REQ:
			return Init(mplMsg);
			break;
		case ICE_MAKE_OFFER_REQ:
			return MakeOffer(mplMsg, sessID);
			break;
		case ICE_PROCESS_ANSWER_REQ:
			return ProcessAnswer(mplMsg);
			break;
		case ICE_MAKE_ANSWER_REQ:
			return MakeAnswer(mplMsg, sessID);
			break;
		case ICE_CLOSE_SESSION_REQ:
			return CloseSession(mplMsg);
			break;
		case ICE_MODIFY_SESSION_ANSWER_REQ:
			return MakeAnswerNoCreate(mplMsg);
			break;
		case ICE_MODIFY_SESSION_OFFER_REQ:
			return MakeOfferNoCreate(mplMsg);
			break;
	}
	return false;
}

void IceProcessor::SendForMplApi(CMplMcmsProtocol & rMplProtocol) const
{
	m_MplMsgSender.SendForMplApi(rMplProtocol);
};

void IceProcessor::SendCloseSessionInd(CMplMcmsProtocol & mplMsg) const
{
	ICE_CLOSE_SESSION_REQ_S *pIceCloseSessionReq = (ICE_CLOSE_SESSION_REQ_S *) mplMsg.GetData();

	ICE_CLOSE_SESSION_IND_S closeInd;
	memset(&closeInd,0,sizeof(closeInd));

	closeInd.status=STATUS_OK;//always send STATUS_OK back to avoid MCMS can not release
	closeInd.ice_session_index=pIceCloseSessionReq->ice_session_index;

	mplMsg.AddCommonHeader(ICE_CLOSE_SESSION_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	mplMsg.AddData(sizeof(closeInd), (char*)&closeInd);

	SendForMplApi(mplMsg);

	return;
}

auto_array<char> IceProcessor::BuildIceSdpIndByReq(const ICE_GENERAL_REQ_S &ICEreq,
												    const std::string &localSdp, int &size)
{
	ICE_GENERAL_IND_S ICEind;
	memset(&ICEind,0,sizeof(ICEind));

	ICEind.status=localSdp.size()>0 ? STATUS_OK : STATUS_FAIL;
	ICEind.ice_session_index=ICEreq.ice_session_index;

	const std::string &SDP_string=localSdp;
	for(int i=0;i<NumOfMediaTypes;++i) {
		if(ICEreq.candidate_list[i].channelType==kIpAudioChnlType) {
			ICEind.ice_channels_id.ice_audio_rtp_id=1;
			ICEind.ice_channels_id.ice_audio_rtcp_id=2;

			//ICEind.ice_local_ports.audioRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.audioRtcpPort=ICEreq.candidate_list[i].rtcp_port;
		}
		else if(ICEreq.candidate_list[i].channelType==kIpVideoChnlType) {
			ICEind.ice_channels_id.ice_video_rtp_id=3;
			ICEind.ice_channels_id.ice_video_rtcp_id=4;

			//ICEind.ice_local_ports.videoRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.videoRtcpPort=ICEreq.candidate_list[i].rtcp_port;

		}
		else if(ICEreq.candidate_list[i].channelType==kIpFeccChnlType) {
			ICEind.ice_channels_id.ice_data_rtp_id=5;
			ICEind.ice_channels_id.ice_data_rtcp_id=6;

			//ICEind.ice_local_ports.dataRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.dataRtcpPort=ICEreq.candidate_list[i].rtcp_port;

		}
		else if(ICEreq.candidate_list[i].channelType==kIpContentChnlType) {
			ICEind.ice_channels_id.ice_content_rtp_id=7;
			ICEind.ice_channels_id.ice_content_rtcp_id=8;

			//ICEind.ice_local_ports.contentRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.contentRtcpPort=ICEreq.candidate_list[i].rtcp_port;

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

#endif	//__DISABLE_ICE__
