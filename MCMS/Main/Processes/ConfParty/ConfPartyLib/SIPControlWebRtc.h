//+========================================================================+
//                            SIPControlWebRtc.h                                 |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPControlWebRtc.h                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 16/04/14   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#ifndef __SIP_CONTROL_WEBRTC__
#define __SIP_CONTROL_WEBRTC__

typedef struct {
	char *pMline;
	char *pProto;
	char *pFmt;
	char *pFmtEnd;
	char *pMlineEnd;
} MlineInfo;

typedef struct {
	char *pAttr;
	char *pAttrEnd;
} AttrInfo;


class CSipWebRtcCntl : public CSipCntl
{
CLASS_TYPE_1(CSipWebRtcCntl, CSipCntl)

public:

	CSipWebRtcCntl(CTaskApp * pOwnerTask);
	virtual const char* NameOf() const { return "CSipWebRtcCntl";}
	virtual ~CSipWebRtcCntl();

	virtual CSipWebRtcCntl *GetWebRtcCntl() {return this;}
	virtual void DisconnectWebRtcCntl();
	virtual void AddInviteResponseReqHeader(CSipHeaderList& rHeaderList);
	virtual void OverwriteMonitoring(EIpChannelType channelType, mcTransportAddress &partyAddr, mcTransportAddress &mcuAddr, BYTE &IsIce, mcTransportAddress &partyIceAddr, mcTransportAddress &mcuIceAddr);

	void RestoreOrignAddr() {RestoreAddr(m_orignLocalUdpAddressesParams, m_orignRemoteAudioAddr, m_orignRemoteVideoAddr);}
	void RestoreNewAddr() {RestoreAddr(m_newLocalUdpAddressesParams, m_newRemoteAudioAddr, m_newRemoteVideoAddr);}
	BYTE SendWebRtcConnectReq(CSipComMode * pBestMode);
	BYTE SendWebRtcDisconnectReq();

protected:
	void OnCmWebRtcConnectAck(CSegment *pParam);
	void OnCmWebRtcConnectInd(CSegment *pParam);
	void OnCmWebRtcConnectReqTout(CSegment *pParam);
	void OnCmWebRtcConnectIndTout(CSegment *pParam);

protected:

	PDECLAR_MESSAGE_MAP

	void SetUdpInfo(mcReqCmOpenUdpPortOrUpdateUdpAddr &udpAddr, kChanneltype, cmCapDirection, int address, int cardPort, int adapterPort);
	void GetExternalAddr(UdpAddresses &udpAddresses, mcTransportAddress &addr);
	void OverwriteLocalAddress();
	void OverwriteRemoteAddress();
	void OverwriteRemoteAddressAudio(sipMediaLineSt *pMediaLine);
	void OverwriteRemoteAddressVideo(sipMediaLineSt *pMediaLine);
	void CleanSdp();
	void SetSdp(int sdpSize, char *sdp);
	void RestoreAddr(UdpAddresses &localUdpAddressesParams, mcXmlTransportAddress &remoteAudioAddr, mcXmlTransportAddress &remoteVideoAddr);

	BYTE UpdateSdp(const CSipComMode * pBestMode, mcReqCmWebRtcConnect *pStruct, int maxSdpSize);
	BYTE OverwriteSdpMediaPort0(const char *media, mcReqCmWebRtcConnect *pStruct, int maxSdpSize);
	BYTE ReplaceStrInSdp(mcReqCmWebRtcConnect *pStruct, int maxSdpSize, char *p1, char *p2, char *str, int len);
	BYTE ReplaceStrInMline(mcReqCmWebRtcConnect *pStruct, int maxSdpSize, MlineInfo &mlineInfo, char *p1, char *p2, char *str, int len);
	BYTE ReplaceAudioOpus(const CSipComMode * pBestMode, mcReqCmWebRtcConnect *pStruct, MlineInfo &mlineInfo, int payload, int maxSdpSize);
	BYTE FilterSdpAudio(const CSipComMode * pBestMode, mcReqCmWebRtcConnect *pStruct, int maxSdpSize);
	BYTE FilterSdpVideo(const CSipComMode * pBestMode, mcReqCmWebRtcConnect *pStruct, int maxSdpSize);
	BYTE GetSdpAudioPayload(const CSipComMode * pBestMode, APIU8 &payload, APIU8 &payloadDtmf);
	BYTE GetSdpVideoPayload(const CSipComMode * pBestMode, APIU8 &payload);
	BYTE GetSdpMlineInfo(const CSipComMode * pBestMode, const char *media, mcReqCmWebRtcConnect *pStruct, MlineInfo &mlineInfo);
	BYTE GetSdpAttrInfo(const mcReqCmWebRtcConnect *pStruct, MlineInfo &mlineInfo, const char *attr, AttrInfo &attrInfo);
	char *GetSdpStrNext(const char *start, const char *end);


	int m_sdpSize;
	char *m_sdp;
	BYTE m_bIsConnected;

	UdpAddresses m_orignLocalUdpAddressesParams;
	mcXmlTransportAddress m_orignRemoteAudioAddr;
	mcXmlTransportAddress m_orignRemoteVideoAddr;

	UdpAddresses m_newLocalUdpAddressesParams;
	mcXmlTransportAddress m_newRemoteAudioAddr;
	mcXmlTransportAddress m_newRemoteVideoAddr;
};


#endif



