//+========================================================================+
//                            SIPControl.cpp                               |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPControl.cpp                                              |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:--															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+
#include <arpa/inet.h>
#include <stdlib.h>



#include "Segment.h"
#include "StateMachine.h"
#include "PartyApi.h"
#include "ProcessBase.h"
#include "ConfPartyProcess.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "IpCsOpcodes.h"
#include "Conf.h"
#include "IpAddressDefinitions.h"
#include "ConfPartyDefines.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyGlobals.h"
#include "StatusesGeneral.h"
#include "SIPCommon.h"
#include "SipScm.h"
#include "SipCaps.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "Party.h"
#include "IPParty.h"
#include "IpMfaOpcodes.h"
#include "CsInterface.h"
#include "SIPParty.h"
#include "SIPControl.h"
#include "IpWebRtcReq.h"
#include "IpWebRtcInd.h"
#include "SIPControlWebRtc.h"
#include "SipUtils.h"
#include "H264Util.h"
#include "IpRtpInd.h"
#include "IpCmInd.h"



#define WEBRTC_INTERNAL_ADDRESS 	0x7F000001
#define WEBRTC_STR_SIZE 			64
#define WEBRTC_FMTP_SIZE			1024


PBEGIN_MESSAGE_MAP(CSipWebRtcCntl)

// state machine for request
ONEVENT(ACK_IND, 							IP_CHANGEMODE,			CSipWebRtcCntl::OnCmWebRtcConnectAck)
ONEVENT(SIP_WEBRTC_PARTY_CONNECT_REQ_TIMER,	IP_CONNECTING, 			CSipWebRtcCntl::OnCmWebRtcConnectReqTout)
ONEVENT(SIP_WEBRTC_PARTY_CONNECT_IND_TIMER,	IP_CONNECTING, 			CSipWebRtcCntl::OnCmWebRtcConnectIndTout)
ONEVENT(IP_CM_WEBRTC_CONNECT_IND,			IP_CONNECTING, 			CSipWebRtcCntl::OnCmWebRtcConnectInd)


PEND_MESSAGE_MAP(CSipWebRtcCntl,CSipCntl);


//////////////////////////////////////////////////////////////////////
CSipWebRtcCntl::CSipWebRtcCntl(CTaskApp *pOwnerTask)
        :CSipCntl(pOwnerTask)
{
	m_sdpSize = 0;
	m_sdp = NULL;
	m_bIsConnected = FALSE;

	memset(&m_orignLocalUdpAddressesParams, 0, sizeof(m_orignLocalUdpAddressesParams));
	memset(&m_orignRemoteAudioAddr, 0, sizeof(m_orignRemoteAudioAddr));
	memset(&m_orignRemoteVideoAddr, 0, sizeof(m_orignRemoteVideoAddr));

	memset(&m_newLocalUdpAddressesParams, 0, sizeof(m_newLocalUdpAddressesParams));
	memset(&m_newRemoteAudioAddr, 0, sizeof(m_newRemoteAudioAddr));
	memset(&m_newRemoteVideoAddr, 0, sizeof(m_newRemoteVideoAddr));
}


////////////////////////////////////////////
CSipWebRtcCntl::~CSipWebRtcCntl()
{
	CleanSdp();
}

////////////////////////////////////////////
void CSipWebRtcCntl::DisconnectWebRtcCntl()
{
	SendWebRtcDisconnectReq();
}

////////////////////////////////////////////
void CSipWebRtcCntl::CleanSdp()
{
	m_sdpSize = 0;
	if (m_sdp) {
		delete [] m_sdp;
		m_sdp = NULL;
	}
}

////////////////////////////////////////////
void CSipWebRtcCntl::SetSdp(int sdpSize, char *sdp)
{
	CleanSdp();

	if (sdpSize) {
		m_sdpSize = sdpSize;
		m_sdp = new char [sdpSize];
		memcpy(m_sdp, sdp, sdpSize);
	}
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::ReplaceStrInSdp(mcReqCmWebRtcConnect *pStruct, int maxSdpSize, char *p1, char *p2, char *str, int len)
{
	if (!(p1 && p2 && p1 <= p2 && pStruct && pStruct->sdp && pStruct->sdpSize))
		return FALSE;

	char *src = p2;
	char *dst = p1 + len;
	int size = &pStruct->sdp[pStruct->sdpSize - 1] - p2 + 1;

	int newSdpSize = pStruct->sdpSize - (p2 - p1) + len;

	if (newSdpSize > maxSdpSize)
		return FALSE;

	if (dst != src)
		memmove(dst, src, size);

	if (str && len)
		memcpy(p1, str, len);

	pStruct->sdpSize = newSdpSize;

	return TRUE;
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::ReplaceStrInMline(mcReqCmWebRtcConnect *pStruct, int maxSdpSize, MlineInfo &mlineInfo, char *p1, char *p2, char *str, int len)
{
	if (ReplaceStrInSdp(pStruct, maxSdpSize, p1, p2, str, len)) {
		mlineInfo.pMlineEnd += len - (p2 - p1);
		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::OverwriteSdpMediaPort0(const char *media, mcReqCmWebRtcConnect *pStruct, int maxSdpSize)
{
	if (!(pStruct && pStruct->sdp && pStruct->sdpSize))
		return FALSE;

	char str[WEBRTC_STR_SIZE];
	char *p1 = NULL;
	char *p2 = NULL;
	int port = 0;

	snprintf(str, sizeof(str), "\r\nm=%s ", media);
	p1 = strstr(&pStruct->sdp[0], str);
	if (p1) {
		snprintf(str, sizeof(str), "\r\nm=%s %%d", media);
		sscanf(p1, str, &port);
		p2 = strstr(p1, "RTP/");
	}

	if (!(p1 && p2 && port))
		return FALSE;

	if (port == 0)
		return TRUE;

	snprintf(str, sizeof(str), "\r\nm=%s 0 ", media);
	int len = strlen(str);

	return ReplaceStrInSdp(pStruct, maxSdpSize, p1, p2, &str[0], len);
}

////////////////////////////////////////////
char *CSipWebRtcCntl::GetSdpStrNext(const char *start, const char *end)
{
	if (start[0] != ' ') {
		while(start < end && start[0] != ' ')
			start++;

	}

	while(start < end && start[0] == ' ')
		start++;

	if (start == end)
		return NULL;

	return (char *)start;
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::GetSdpAttrInfo(const mcReqCmWebRtcConnect *pStruct, MlineInfo &mlineInfo, const char *attr, AttrInfo &attrInfo)
{
	memset(&attrInfo, 0, sizeof(attrInfo));

	char c = mlineInfo.pMlineEnd[0];
	mlineInfo.pMlineEnd[0] = '\0';

	attrInfo.pAttr = strstr(mlineInfo.pMline, attr);
	if (attrInfo.pAttr) {
		attrInfo.pAttrEnd = strstr(attrInfo.pAttr + 1, "\r\n");
		if (!attrInfo.pAttrEnd)
			attrInfo.pAttrEnd = mlineInfo.pMlineEnd;
	}

	mlineInfo.pMlineEnd[0] = c;

	return (attrInfo.pAttrEnd != NULL);
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::GetSdpMlineInfo(const CSipComMode * pBestMode, const char *media, mcReqCmWebRtcConnect *pStruct, MlineInfo &mlineInfo)
{
	memset(&mlineInfo, 0, sizeof(mlineInfo));

	char c = pStruct->sdp[pStruct->sdpSize - 1];
	pStruct->sdp[pStruct->sdpSize - 1] = '\0';

	char mline[WEBRTC_STR_SIZE];
	snprintf(mline, sizeof(mline), "\r\nm=%s", media);

	mlineInfo.pMline = strstr(&pStruct->sdp[0], mline);
	if (mlineInfo.pMline) {
		mlineInfo.pProto = strstr(mlineInfo.pMline, "RTP/");
		if (mlineInfo.pProto) {
			mlineInfo.pFmt = strstr(mlineInfo.pProto, " ");
			if (mlineInfo.pFmt) {
				mlineInfo.pFmtEnd = strstr(mlineInfo.pProto , "\r\n");
				if (mlineInfo.pFmtEnd) {
					mlineInfo.pMlineEnd = strstr(mlineInfo.pFmtEnd, "\r\nm=");
					if (!mlineInfo.pMlineEnd)
						mlineInfo.pMlineEnd = &pStruct->sdp[pStruct->sdpSize - 2];
				}
			}
		}
	}

	pStruct->sdp[pStruct->sdpSize - 1] = c;

	return (mlineInfo.pMlineEnd != NULL);
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::GetSdpAudioPayload(const CSipComMode * pBestMode, APIU8 &payload, APIU8 &payloadDtmf)
{
	payload = _UnKnown;
	payloadDtmf = _UnKnown;

	CCapSetInfo capInfo = (CapEnum) pBestMode->GetMediaType(cmCapAudio);
	payload = m_pLastRemoteCaps->GetPayloadTypeByDynamicPreference(capInfo, H264_Profile_None, kRolePeople);

	CCapSetInfo dtmfInfo = eRfc2833DtmfCapCode;
	if (m_pChosenLocalCap->IsCapSet(dtmfInfo) && m_pLastRemoteCaps->IsCapSet(dtmfInfo))
		payloadDtmf = m_pLastRemoteCaps->GetPayloadTypeByDynamicPreference(dtmfInfo, H264_Profile_None, kRolePeople);

	return (payload != _UnKnown);
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::GetSdpVideoPayload(const CSipComMode * pBestMode, APIU8 &payload)
{
	CCapSetInfo capInfo = (CapEnum) pBestMode->GetMediaType(cmCapVideo);
	payload = m_pLastRemoteCaps->GetPayloadTypeByDynamicPreference(capInfo, H264_Profile_None, kRolePeople);

	return (payload != _UnKnown);
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::ReplaceAudioOpus(const CSipComMode * pBestMode, mcReqCmWebRtcConnect *pStruct, MlineInfo &mlineInfo, int payload, int maxSdpSize)
{
	CBaseAudioCap *pCap = (CBaseAudioCap *)pBestMode->GetMediaAsCapClass(cmCapAudio,cmCapReceive);
	if (!pCap)
		return TRUE;

	CapEnum capEnum = pCap->GetCapCode();

	if (capEnum == eOpus_CapCode || capEnum == eOpusStereo_CapCode) {

		char attr[WEBRTC_STR_SIZE];
		AttrInfo attrInfo;

		BYTE data[sizeof(opus_CapStruct)];
		opus_CapStruct *pOpus = (opus_CapStruct *) pCap->GetStruct();
		pBestMode->CopyMediaData(data, cmCapAudio, cmCapReceive);

		snprintf(attr, sizeof(attr), "\r\na=fmtp:%d", payload);

		if (GetSdpAttrInfo(pStruct, mlineInfo, attr, attrInfo)) {

			char fmtp[WEBRTC_FMTP_SIZE];
			char tmp[WEBRTC_FMTP_SIZE];
			int pos = 0;

			if (pos < WEBRTC_FMTP_SIZE) {
				sprintf(tmp, "\r\na=fmtp:%d sprop-maxplaybackrate=48000", payload);
				strncpy(&fmtp[pos], tmp, sizeof(fmtp) - pos);
				pos += strlen(tmp);
			}

			if (pos < WEBRTC_FMTP_SIZE && pOpus->minPtime) {
				sprintf(tmp, "; minptime=%d", pOpus->minPtime);
				strncpy(&fmtp[pos], tmp, sizeof(fmtp) - pos);
				pos += strlen(tmp);
			}

			if (pos < WEBRTC_FMTP_SIZE) {
				if (capEnum == eOpusStereo_CapCode)
					sprintf(tmp, "; maxaveragebitrate=%d; sprop-stereo=1; stereo=1", pOpus->maxAverageBitrate);
				else if (capEnum == eOpus_CapCode)
					sprintf(tmp, "; maxaveragebitrate=%d; sprop-stereo=0; stereo=0", pOpus->maxAverageBitrate);
				strncpy(&fmtp[pos], tmp, sizeof(fmtp) - pos);
				pos += strlen(tmp);
			}

			if (pos < WEBRTC_FMTP_SIZE && pOpus->cbr) {
				sprintf(tmp, "; cbr=%d", pOpus->cbr);
				strncpy(&fmtp[pos], tmp, sizeof(fmtp) - pos);
				pos += strlen(tmp);
			}

			if (pos < WEBRTC_FMTP_SIZE && pOpus->useInbandFec) {
				sprintf(tmp, "; useinbandfec=%d", pOpus->useInbandFec);
				strncpy(&fmtp[pos], tmp, sizeof(fmtp) - pos);
				pos += strlen(tmp);
			}

			if (pos < WEBRTC_FMTP_SIZE && pOpus->useDtx) {
				sprintf(tmp, "; usedtx=%d", pOpus->useDtx);
				strncpy(&fmtp[pos], tmp, sizeof(fmtp) - pos);
				pos += strlen(tmp);
			}

			if (pos < WEBRTC_FMTP_SIZE && pOpus->useDtx) {
				sprintf(tmp, "; usedtx=%d", pOpus->useDtx);
				strncpy(&fmtp[pos], tmp, sizeof(fmtp) - pos);
				pos += strlen(tmp);
			}

            if(pos > WEBRTC_FMTP_SIZE) {
                pos = WEBRTC_FMTP_SIZE;
                PASSERTMSG(1, "pos > WEBRTC_FMTP_SIZE");
            }
 
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, attrInfo.pAttr, attrInfo.pAttrEnd, fmtp, pos);
		}

		snprintf(attr, sizeof(attr), "\r\na=ptime:");
		if (GetSdpAttrInfo(pStruct, mlineInfo, attr, attrInfo))
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, attrInfo.pAttr, attrInfo.pAttrEnd, NULL, 0);

		snprintf(attr, sizeof(attr), "\r\na=maxptime:");
		if (GetSdpAttrInfo(pStruct, mlineInfo, attr, attrInfo))
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, attrInfo.pAttr, attrInfo.pAttrEnd, NULL, 0);

		if (pOpus->maxValue > 0) {
			char str[WEBRTC_STR_SIZE];
			snprintf(str, sizeof(str), "\r\na=ptime:%d", (int)pOpus->maxValue);
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, mlineInfo.pMlineEnd, mlineInfo.pMlineEnd, str, strlen(str));
		}

		if (pOpus->maxPtime > 0) {
			char str[WEBRTC_STR_SIZE];
			snprintf(str, sizeof(str), "\r\na=maxptime:%d", (int)pOpus->maxPtime);
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, mlineInfo.pMlineEnd, mlineInfo.pMlineEnd, str, strlen(str));
		}
	}

	POBJDELETE(pCap);

	return TRUE;
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::FilterSdpAudio(const CSipComMode * pBestMode, mcReqCmWebRtcConnect *pStruct, int maxSdpSize)
{
	APIU8 payload = _UnKnown;
	APIU8 payloadDtmf = _UnKnown;
	MlineInfo mlineInfo;

	if (!GetSdpAudioPayload(pBestMode, payload, payloadDtmf))
		return FALSE;

	if (!GetSdpMlineInfo(pBestMode, "audio", pStruct, mlineInfo))
		return FALSE;

	char *pFmt = GetSdpStrNext(mlineInfo.pFmt, mlineInfo.pFmtEnd);

	while(pFmt) {

		char attr[WEBRTC_STR_SIZE];
		AttrInfo attrInfo;
		int pt = _UnKnown;

		if (!sscanf(pFmt, "%d", &pt))
			return FALSE;

		pFmt = GetSdpStrNext(pFmt, mlineInfo.pFmtEnd);

		if (pt == payloadDtmf)
			continue;

		if (pt == payload) {
			//ReplaceAudioOpus(pBestMode, pStruct, mlineInfo, payload, maxSdpSize);
			continue;
		}

		snprintf(attr, sizeof(attr), "\r\na=rtpmap:%d", pt);
		if (GetSdpAttrInfo(pStruct, mlineInfo, attr, attrInfo))
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, attrInfo.pAttr, attrInfo.pAttrEnd, NULL, 0);

		snprintf(attr, sizeof(attr), "\r\na=fmtp:%d", pt);
		if (GetSdpAttrInfo(pStruct, mlineInfo, attr, attrInfo))
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, attrInfo.pAttr, attrInfo.pAttrEnd, NULL, 0);
	}

	char fmt[WEBRTC_STR_SIZE];
	memset(fmt, 0, WEBRTC_STR_SIZE);
	if (payload != _UnKnown && payloadDtmf != _UnKnown)
		snprintf(fmt, sizeof(fmt), " %d %d", payload, payloadDtmf);
	else if (payload != _UnKnown)
		snprintf(fmt, sizeof(fmt), " %d", payload);
	else if (payloadDtmf != _UnKnown)
		snprintf(fmt, sizeof(fmt), " %d", payloadDtmf);

	return ReplaceStrInSdp(pStruct, maxSdpSize, mlineInfo.pFmt, mlineInfo.pFmtEnd, fmt, strlen(fmt));
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::FilterSdpVideo(const CSipComMode * pBestMode, mcReqCmWebRtcConnect *pStruct, int maxSdpSize)
{
	APIU8 payload = _UnKnown;
	MlineInfo mlineInfo;

	if (!GetSdpVideoPayload(pBestMode, payload))
		return FALSE;

	if (!GetSdpMlineInfo(pBestMode, "video", pStruct, mlineInfo))
		return FALSE;

	char *pFmt = GetSdpStrNext(mlineInfo.pFmt, mlineInfo.pFmtEnd);

	while(pFmt) {

		int pt = _UnKnown;
		char attr[WEBRTC_STR_SIZE];
		AttrInfo attrInfo;

		if (!sscanf(pFmt, "%d", &pt))
			return FALSE;

		pFmt = GetSdpStrNext(pFmt, mlineInfo.pFmtEnd);

		if (pt == payload)
			continue;

		snprintf(attr, sizeof(attr), "\r\na=rtpmap:%d", pt);
		if (GetSdpAttrInfo(pStruct, mlineInfo, attr, attrInfo))
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, attrInfo.pAttr, attrInfo.pAttrEnd, NULL, 0);

		snprintf(attr, sizeof(attr), "\r\na=fmtp:%d", pt);
		if (GetSdpAttrInfo(pStruct, mlineInfo, attr, attrInfo))
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, attrInfo.pAttr, attrInfo.pAttrEnd, NULL, 0);

		snprintf(attr, sizeof(attr), "\r\na=rtcp-fb:%d", pt);
		while (GetSdpAttrInfo(pStruct, mlineInfo, attr, attrInfo))
			ReplaceStrInMline(pStruct, maxSdpSize, mlineInfo, attrInfo.pAttr, attrInfo.pAttrEnd, NULL, 0);

	}

	char fmt[WEBRTC_STR_SIZE];
	memset(fmt, 0, WEBRTC_STR_SIZE);
	if (payload != _UnKnown)
		snprintf(fmt, sizeof(fmt), " %d", payload);

	return ReplaceStrInSdp(pStruct, maxSdpSize, mlineInfo.pFmt, mlineInfo.pFmtEnd, fmt, strlen(fmt));
}

////////////////////////////////////////////
BYTE CSipWebRtcCntl::UpdateSdp(const CSipComMode * pBestMode, mcReqCmWebRtcConnect *pStruct, int maxSdpSize)
{
	BYTE status = TRUE;

	if (pBestMode->IsMediaOff(cmCapAudio,cmCapReceiveAndTransmit))
		status = OverwriteSdpMediaPort0("audio", pStruct, maxSdpSize);
	else
		status = FilterSdpAudio(pBestMode, pStruct, maxSdpSize);

	if (pBestMode->IsMediaOff(cmCapVideo,cmCapReceiveAndTransmit))
		status = OverwriteSdpMediaPort0("video", pStruct, maxSdpSize);
	else
		status = FilterSdpVideo(pBestMode, pStruct, maxSdpSize);

	return status;
}

////////////////////////////////////////////
void CSipWebRtcCntl::RestoreAddr(UdpAddresses &localUdpAddressesParams, mcXmlTransportAddress &remoteAudioAddr, mcXmlTransportAddress &remoteVideoAddr)
{
	m_UdpAddressesParams = localUdpAddressesParams;

	sipSdpAndHeaders *pSdpAndHeaders = (sipSdpAndHeaders *) m_pRemoteSdp;

	if (pSdpAndHeaders && pSdpAndHeaders->sipMediaLinesLength) {

		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipMediaLinesOffset];
		sipMediaLineSt *pMediaLine = NULL;
		unsigned int mediaLinePos = 0;

		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if (pMediaLine->internalType == kMediaLineInternalTypeAudio)
				pMediaLine->mediaIp = remoteAudioAddr;
			else if (pMediaLine->internalType == kMediaLineInternalTypeVideo)
				pMediaLine->mediaIp = remoteVideoAddr;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::AddInviteResponseReqHeader(CSipHeaderList& rHeaderList)
{
	if (m_sdpSize && m_sdp)
		rHeaderList.AddHeader(kWebRtcSdp, m_sdpSize, m_sdp);
}

////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::OverwriteMonitoring(EIpChannelType channelType, mcTransportAddress &partyAddr, mcTransportAddress &mcuAddr, BYTE &IsIce, mcTransportAddress &partyIceAddr, mcTransportAddress &mcuIceAddr)
{
	if (channelType == AUDIO_IN || channelType == AUDIO_OUT) {

		GetExternalAddr(m_newLocalUdpAddressesParams, partyAddr);
		GetExternalAddr(m_newLocalUdpAddressesParams, mcuAddr);
		partyAddr.transportType = eTransportTypeUdp;
		mcuAddr.transportType = eTransportTypeUdp;
		partyAddr.port = m_newLocalUdpAddressesParams.AudioChannelAdditionalPorts;
		mcuAddr.port = m_newLocalUdpAddressesParams.AudioChannelPort;

		IsIce = 1;
		partyIceAddr = m_orignRemoteAudioAddr.transAddr;
		GetExternalAddr(m_orignLocalUdpAddressesParams, mcuIceAddr);
	}
	else if (channelType == VIDEO_IN || channelType == VIDEO_OUT) {

		GetExternalAddr(m_newLocalUdpAddressesParams, partyAddr);
		GetExternalAddr(m_newLocalUdpAddressesParams, mcuAddr);
		partyAddr.transportType = eTransportTypeUdp;
		mcuAddr.transportType = eTransportTypeUdp;
		partyAddr.port = m_newLocalUdpAddressesParams.VideoChannelAdditionalPorts;
		mcuAddr.port = m_newLocalUdpAddressesParams.VideoChannelPort;

		IsIce = 1;
		partyIceAddr = m_orignRemoteVideoAddr.transAddr;
		GetExternalAddr(m_orignLocalUdpAddressesParams, mcuIceAddr);
	}
}

////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::OverwriteLocalAddress()
{
	m_orignLocalUdpAddressesParams = m_UdpAddressesParams;

	m_UdpAddressesParams.IpType = eIpType_IpV4;
	m_UdpAddressesParams.IpV4Addr.ip = WEBRTC_INTERNAL_ADDRESS;

	m_newLocalUdpAddressesParams = m_UdpAddressesParams;
}

////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::OverwriteRemoteAddressAudio(sipMediaLineSt *pMediaLine)
{
	m_orignRemoteAudioAddr = pMediaLine->mediaIp;

	pMediaLine->mediaIp.transAddr.ipVersion = eIpVersion4;
	pMediaLine->mediaIp.transAddr.transportType = eTransportTypeUdp;
	pMediaLine->mediaIp.transAddr.addr.v4.ip = WEBRTC_INTERNAL_ADDRESS;
	pMediaLine->mediaIp.transAddr.port = m_UdpAddressesParams.AudioChannelAdditionalPorts;
	pMediaLine->rtcpPort = m_UdpAddressesParams.AudioChannelAdditionalPorts + 1;

	m_newRemoteAudioAddr = pMediaLine->mediaIp;
}

////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::OverwriteRemoteAddressVideo(sipMediaLineSt *pMediaLine)
{
	m_orignRemoteVideoAddr = pMediaLine->mediaIp;

	pMediaLine->mediaIp.transAddr.ipVersion = eIpVersion4;
	pMediaLine->mediaIp.transAddr.transportType = eTransportTypeUdp;
	pMediaLine->mediaIp.transAddr.addr.v4.ip = WEBRTC_INTERNAL_ADDRESS;
	pMediaLine->mediaIp.transAddr.port = m_UdpAddressesParams.VideoChannelAdditionalPorts;
	pMediaLine->rtcpPort = m_UdpAddressesParams.VideoChannelAdditionalPorts + 1;

	m_newRemoteVideoAddr = pMediaLine->mediaIp;
}

////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::OverwriteRemoteAddress()
{
	sipSdpAndHeaders *pSdpAndHeaders = (sipSdpAndHeaders *) m_pRemoteSdp;

	if (pSdpAndHeaders && pSdpAndHeaders->sipMediaLinesLength) {

		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipMediaLinesOffset];
		sipMediaLineSt *pMediaLine = NULL;
		unsigned int mediaLinePos = 0;

		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if (pMediaLine->internalType == kMediaLineInternalTypeAudio)
				OverwriteRemoteAddressAudio(pMediaLine);
			else if (pMediaLine->internalType == kMediaLineInternalTypeVideo)
				OverwriteRemoteAddressVideo(pMediaLine);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::SetUdpInfo(mcReqCmOpenUdpPortOrUpdateUdpAddr &udpAddr, kChanneltype type, cmCapDirection direction, int address, int cardPort, int adapterPort)
{
	memset(&udpAddr, 0, sizeof(udpAddr));

	udpAddr.channelType = type;
	udpAddr.channelDirection = direction;

	udpAddr.CmLocalUdpAddressIp.ipVersion = eIpVersion4;
	udpAddr.CmLocalUdpAddressIp.transportType = eTransportTypeUdp;
	udpAddr.CmLocalUdpAddressIp.addr.v4.ip = address;
	udpAddr.CmLocalUdpAddressIp.port = adapterPort;
	udpAddr.LocalRtcpPort = adapterPort + 1;

	udpAddr.CmRemoteUdpAddressIp.ipVersion = eIpVersion4;
	udpAddr.CmRemoteUdpAddressIp.transportType = eTransportTypeUdp;
	udpAddr.CmRemoteUdpAddressIp.addr.v4.ip = address;
	udpAddr.CmRemoteUdpAddressIp.port = cardPort;
	udpAddr.RemoteRtcpPort = cardPort + 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::GetExternalAddr(UdpAddresses &udpAddresses, mcTransportAddress &addr)
{
	if (udpAddresses.IpType == eIpType_IpV6) {
		BYTE place = FindMatchingIpV6MediaAddressByScopeId();
		addr.ipVersion = eIpVersion6;
		addr.addr.v6 = udpAddresses.IpV6AddrArray[place];
		addr.addr.v6.scopeId = udpAddresses.IpV6AddrArray[place].scopeId;
	}
	else {
		addr.ipVersion = eIpVersion4;
		addr.addr.v4 = udpAddresses.IpV4Addr;
	}

	addr.port = udpAddresses.AudioChannelPort;
	addr.transportType = eTransportTypeUdp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipWebRtcCntl::SendWebRtcConnectReq(CSipComMode * pBestMode)
{
	PTRACE(eLevelInfoNormal,"CSipWebRtcCntl::SendWebRtcConnectReq");

	sipSdpAndHeaders* pSdpAndHeaders = GetRemoteSdp();

	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength) {

		char buf[sizeof(mcReqCmWebRtcConnect) + WEBRTC_SDP_MAX_SIZE];
		mcReqCmWebRtcConnect *pStruct = (mcReqCmWebRtcConnect *)buf;
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];

		if (SipGetHeaderValue(pHeaders, kWebRtcSdp, pStruct->sdp, WEBRTC_SDP_MAX_SIZE)) {

			//external address and multiplex port
			GetExternalAddr(m_UdpAddressesParams, pStruct->mcExternalUdpAddressIp);

			//internal addresses and ports
			memset(&pStruct->openUdpPorts, 0, sizeof(pStruct->openUdpPorts));
			SetUdpInfo(pStruct->openUdpPorts[0], kIpAudioChnlType, cmCapReceiveAndTransmit, WEBRTC_INTERNAL_ADDRESS, m_UdpAddressesParams.AudioChannelPort, m_UdpAddressesParams.AudioChannelAdditionalPorts);
			SetUdpInfo(pStruct->openUdpPorts[1], kIpVideoChnlType, cmCapReceiveAndTransmit, WEBRTC_INTERNAL_ADDRESS, m_UdpAddressesParams.VideoChannelPort, m_UdpAddressesParams.VideoChannelAdditionalPorts);

			//remote sdp
			pStruct->sdpSize = strlen(pStruct->sdp);

			if (UpdateSdp(pBestMode, pStruct, WEBRTC_SDP_MAX_SIZE)) {
				int size = sizeof(mcReqCmWebRtcConnect) + pStruct->sdpSize - 1;
				SendMsgToMpl((BYTE*)(pStruct), size, IP_CM_WEBRTC_CONNECT_REQ);
				StartTimer(SIP_WEBRTC_PARTY_CONNECT_REQ_TIMER, 10 * SECOND);
				m_state = IP_CHANGEMODE;
			}
			else {
				PTRACE(eLevelInfoNormal,"CSipWebRtcCntl::SendWebRtcConnectReq: UpdateSdp failed");
				m_pPartyApi->SendWebRtcConnectFailure();
			}
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipWebRtcCntl::SendWebRtcDisconnectReq()
{
	PTRACE(eLevelInfoNormal,"CSipWebRtcCntl::SendWebRtcDisconnectReq");

	if (m_bIsConnected) {
		mcReqCmWebRtcDisconnect msg;
		SendMsgToMpl((BYTE*)(&msg), sizeof(msg), IP_CM_WEBRTC_DISCONNECT_REQ);
		m_bIsConnected = FALSE;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::OnCmWebRtcConnectAck(CSegment *pParam)
{
	m_state = IP_CONNECTING;

	if (IsValidTimer(SIP_WEBRTC_PARTY_CONNECT_REQ_TIMER))
		DeleteTimer(SIP_WEBRTC_PARTY_CONNECT_REQ_TIMER);

	ACK_IND_S* pAckIndStruct = (ACK_IND_S*)pParam->GetPtr(1);

	if (pAckIndStruct->ack_base.status != STATUS_OK) {
		PTRACE2INT(eLevelInfoNormal,"CSipWebRtcCntl::OnCmWebRtcConnectAck: status ", pAckIndStruct->ack_base.status);
		m_pPartyApi->SendWebRtcConnectFailure();
		return;
	}

	m_bIsConnected = TRUE;
	StartTimer(SIP_WEBRTC_PARTY_CONNECT_IND_TIMER, 10 * SECOND);
}

////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::OnCmWebRtcConnectReqTout(CSegment *pParam)
{
	PTRACE(eLevelInfoNormal,"CSipWebRtcCntl::OnCmWebRtcConnectReqTout");
	m_pPartyApi->SendWebRtcConnectTout();
}

////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::OnCmWebRtcConnectIndTout(CSegment *pParam)
{
	PTRACE(eLevelInfoNormal,"CSipWebRtcCntl::OnCmWebRtcConnectIndTout");
	m_pPartyApi->SendWebRtcConnectTout();
}

////////////////////////////////////////////////////////////////////////////
void CSipWebRtcCntl::OnCmWebRtcConnectInd(CSegment *pParam)
{
	if (IsValidTimer(SIP_WEBRTC_PARTY_CONNECT_IND_TIMER))
		DeleteTimer(SIP_WEBRTC_PARTY_CONNECT_IND_TIMER);

	mcIndCmWebRtcConnectInd *pWebRtcConnectInd = (mcIndCmWebRtcConnectInd *)pParam->GetPtr(0);

	if (pWebRtcConnectInd && pWebRtcConnectInd->sdpSize) {
		SetSdp(pWebRtcConnectInd->sdpSize, pWebRtcConnectInd->sdp);
		OverwriteLocalAddress();
		OverwriteRemoteAddress();
	}

	m_pPartyApi->SendWebRtcPartyEstablishCallIdle();
}
