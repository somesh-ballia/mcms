/*
 * ICEParams.h
 *
 *  Created on: Feb 2, 2010
 *      Author: inga
 */

#ifndef ICEPARAMS_H_
#define ICEPARAMS_H_

#include "PObject.h"
#include "ObjString.h"
#include "Segment.h"
#include "SipUtils.h"

typedef enum
{
	eAudioSession,
	eVideoSession,
	eDataSession,
	eGeneralSession,
	eNotSupportedSession,

}ICESessionsTypes;

static ICESessionsTypes globalIceSessionTypes[] = {eAudioSession,eVideoSession,eDataSession};

class CIceParams : public CPObject
{
CLASS_TYPE_1(CIceParams, CPObject)
public:
	CIceParams ();
	~CIceParams ();

	CIceParams(const CIceParams&);

	virtual const char* NameOf() const { return "CIceParams";}

	virtual void Serialize(WORD format,CSegment& seg);
	virtual void DeSerialize(WORD format,CSegment& seg);

	CIceParams& operator =(const CIceParams& other);

	void  SetSessionIndex(DWORD id);
	DWORD GetSessionIndex();

	void  SetAudioRtpPort(WORD port);
	WORD GetAudioRtpPort();
	void  SetAudioRtcpPort(WORD port);
	WORD GetAudioRtcpPort();

	void  SetVideoRtpPort(WORD port);
	WORD GetVideoRtpPort();
	void  SetVideoRtcpPort(WORD port);
	WORD GetVideoRtcpPort();

	void  SetDataRtpPort(WORD port);
	WORD GetDataRtpPort();
	void  SetDataRtcpPort(WORD port);
	WORD GetDataRtcpPort();

	void  SetContentRtpPort(WORD port);
	WORD GetContentRtpPort();
	void  SetContentRtcpPort(WORD port);
	WORD GetContentRtcpPort();

	void  SetAudioRtpId(DWORD id);
	DWORD  GetAudioRtpId();
	void  SetAudioRtcpId(DWORD id);
	DWORD  GetAudioRtcpId();

	void  SetVideoRtpId(DWORD id);
	DWORD GetVideoRtpId();
	void  SetVideoRtcpId(DWORD id);
	DWORD GetVideoRtcpId();

	void  SetDataRtpId(DWORD id);
	DWORD GetDataRtpId();
	void  SetDataRtcpId(DWORD id);
	DWORD GetDataRtcpId();

	void  SetContentRtpId(DWORD id);
	DWORD GetContentRtpId();
	void  SetContentRtcpId(DWORD id);
	DWORD GetContentRtcpId();

    mcTransportAddress * GetIceMediaIp (ICESessionsTypes mediaType);
    void SetIceMediaIp (ICESessionsTypes mediaType, mcTransportAddress * pIp);
    void Dump (char * headerStr);
    

    void  SetSubType(cmCapDataType dataType,  eMediaLineSubType subType);
    eMediaLineSubType  GetSubType(cmCapDataType dataType);
    eMediaLineSubType  GetSubType(cmCapDataType dataType, bool bSdes);
    eMediaLineSubType ConvertSdesSubType(eMediaLineSubType pSubType, bool bSdes);
    bool IsSubTypeSdes(eMediaLineSubType subType);

protected:
	DWORD 	m_ice_session_id;

	DWORD 	m_ice_audio_rtp_id;
	DWORD	m_ice_audio_rtcp_id;

	DWORD 	m_ice_video_rtp_id;
	DWORD 	m_ice_video_rtcp_id;

	DWORD 	m_ice_data_rtp_id;
	DWORD 	m_ice_data_rtcp_id;

	DWORD 	m_ice_content_rtp_id;
	DWORD 	m_ice_content_rtcp_id;

	mcTransportAddress m_GeneralIp;

	WORD 	m_audioRtpPort;
	WORD 	m_audioRtcpPort;
    mcTransportAddress m_AudioIp;
    
	WORD 	m_videoRtpPort;
	WORD 	m_videoRtcpPort;
    mcTransportAddress m_VideoIp;
    
	WORD 	m_dataRtpPort;
	WORD 	m_dataRtcpPort;
    mcTransportAddress m_DataIp;
    
	WORD 	m_contentRtpPort;
	WORD 	m_contentRtcpPort;
    mcTransportAddress m_ContentIp;

	eMediaLineSubType m_audioSubType;
	eMediaLineSubType m_videoSubType;
	eMediaLineSubType m_dataSubType;
};



#endif /* ICEPARAMS_H_ */
