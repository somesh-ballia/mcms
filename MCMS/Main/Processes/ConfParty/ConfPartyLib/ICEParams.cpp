/*
 * ICEParams.cpp
 *
 *  Created on: Feb 2, 2010
 *      Author: inga
 */
#include "ICEParams.h"
#include "WrappersCommon.h"

#include "SystemFunctions.h"



CIceParams::CIceParams()
{
	m_ice_session_id = 0;

	m_ice_audio_rtp_id = 0;
	m_ice_audio_rtcp_id = 0;

	m_ice_video_rtp_id = 0;
	m_ice_video_rtcp_id = 0;

	m_ice_data_rtp_id = 0;
	m_ice_data_rtcp_id = 0;

	m_ice_content_rtp_id = 0;
	m_ice_content_rtcp_id = 0;

	m_audioRtpPort=0;
	m_audioRtcpPort=0;

	m_videoRtpPort=0;
	m_videoRtcpPort=0;

	m_dataRtpPort=0;
	m_dataRtcpPort=0;

	m_contentRtpPort=0;
	m_contentRtcpPort=0;

	memset (&m_GeneralIp, 0, sizeof(mcTransportAddress));
    memset (&m_AudioIp, 0, sizeof(mcTransportAddress));
    memset (&m_VideoIp, 0, sizeof(mcTransportAddress));
    memset (&m_DataIp, 0, sizeof(mcTransportAddress));
    memset (&m_ContentIp, 0, sizeof(mcTransportAddress));

    m_audioSubType = eMediaLineSubTypeRtpAvp;
	m_videoSubType = eMediaLineSubTypeRtpAvp;
	m_dataSubType = eMediaLineSubTypeRtpAvp;
}
/////////////////////////////////////////////////////////////////
CIceParams::~CIceParams()
{

}

/////////////////////////////////////////////////////////////////
void  CIceParams::SetSessionIndex(DWORD Index)
{
	m_ice_session_id = Index;
}
/////////////////////////////////////////////////////////////////
DWORD CIceParams::GetSessionIndex()
{
	return m_ice_session_id;
}
/////////////////////////////////////////////////////////////////
void  CIceParams::SetAudioRtpPort(WORD port)
{
	m_audioRtpPort = port;
}
/////////////////////////////////////////////////////////////////
WORD CIceParams::GetAudioRtpPort()
{
	return m_audioRtpPort;
}
/////////////////////////////////////////////////////////////////
void  CIceParams::SetAudioRtcpPort(WORD port)
{
	m_audioRtcpPort = port;
}
/////////////////////////////////////////////////////////////////
WORD  CIceParams::GetAudioRtcpPort()
{
	return m_audioRtcpPort;
}
/////////////////////////////////////////////////////////////////
void  CIceParams::SetVideoRtpPort(WORD port)
{
	m_videoRtpPort = port;
}
/////////////////////////////////////////////////////////////////
WORD CIceParams::GetVideoRtpPort()
{
	return m_videoRtpPort;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetVideoRtcpPort(WORD port)
{
	m_videoRtcpPort = port;
}
/////////////////////////////////////////////////////////////////
WORD CIceParams::GetVideoRtcpPort()
{
	return m_videoRtcpPort;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetDataRtpPort(WORD port)
{
	m_dataRtpPort = port;
}
/////////////////////////////////////////////////////////////////
WORD CIceParams::GetDataRtpPort()
{
	return m_dataRtpPort;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetDataRtcpPort(WORD port)
{
	m_dataRtcpPort = port;
}
/////////////////////////////////////////////////////////////////
WORD CIceParams::GetDataRtcpPort()
{
	return m_dataRtcpPort;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetContentRtpPort(WORD port)
{
	m_contentRtpPort = port;
}
/////////////////////////////////////////////////////////////////
WORD CIceParams::GetContentRtpPort()
{
	return m_contentRtpPort;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetContentRtcpPort(WORD port)
{
	m_contentRtcpPort = port;
}
/////////////////////////////////////////////////////////////////
WORD CIceParams::GetContentRtcpPort()
{
	return m_contentRtcpPort;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetAudioRtpId(DWORD id)
{
	m_ice_audio_rtp_id = id;
}
/////////////////////////////////////////////////////////////////
DWORD CIceParams::GetAudioRtpId()
{
	return m_ice_audio_rtp_id;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetAudioRtcpId(DWORD id)
{
	m_ice_audio_rtcp_id = id;
}
/////////////////////////////////////////////////////////////////
DWORD CIceParams::GetAudioRtcpId()
{
	return m_ice_audio_rtcp_id;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetVideoRtpId(DWORD id)
{
	m_ice_video_rtp_id = id;
}
/////////////////////////////////////////////////////////////////
DWORD CIceParams::GetVideoRtpId()
{
	return m_ice_video_rtp_id;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetVideoRtcpId(DWORD id)
{
	m_ice_video_rtcp_id = id;
}
/////////////////////////////////////////////////////////////////
DWORD CIceParams::GetVideoRtcpId()
{
	return m_ice_video_rtcp_id;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetDataRtpId(DWORD id)
{
	m_ice_data_rtp_id = id;
}
/////////////////////////////////////////////////////////////////
DWORD CIceParams::GetDataRtpId()
{
	return m_ice_data_rtp_id;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetDataRtcpId(DWORD id)
{
	m_ice_data_rtcp_id = id;
}
/////////////////////////////////////////////////////////////////
DWORD CIceParams::GetDataRtcpId()
{
	return m_ice_data_rtcp_id;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetContentRtpId(DWORD id)
{
	m_ice_content_rtp_id = id;
}
/////////////////////////////////////////////////////////////////
DWORD CIceParams::GetContentRtpId()
{
	return m_ice_content_rtp_id;
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetContentRtcpId(DWORD id)
{
	m_ice_content_rtcp_id = id;
}
/////////////////////////////////////////////////////////////////
DWORD CIceParams::GetContentRtcpId()
{
	return m_ice_content_rtcp_id;
}

/////////////////////////////////////////////////////////////////
void CIceParams::Dump (char * headerStr)
{
	char ipV6str[IPV6_ADDRESS_LEN];
    CMedString str;
    str << "CIceParams::Dump " << headerStr
        << "\n m_ice_session_id = " << m_ice_session_id
        << "\n===   IDs === " 
        << "\n m_ice_audio_rtp_id = " << m_ice_audio_rtp_id
        << "\n m_ice_audio_rtcp_id = " << m_ice_audio_rtcp_id
        << "\n m_ice_video_rtp_id = " << m_ice_video_rtp_id
        << "\n m_ice_video_rtcp_id = " << m_ice_video_rtcp_id
        << "\n m_ice_content_rtp_id = " << m_ice_content_rtp_id
        << "\n m_ice_content_rtcp_id = " << m_ice_content_rtcp_id
        << "\n m_ice_data_rtp_id = " << m_ice_data_rtp_id
        << "\n m_ice_data_rtcp_id = " << m_ice_data_rtcp_id
        << "\n===   Ports === ";
    	if(m_GeneralIp.ipVersion == eIpVersion6 )
    	{
    		memset(ipV6str, 0, sizeof(ipV6str));
    		str << "\n General Ip = " << ::ipV6ToString(m_GeneralIp.addr.v6.ip, ipV6str, FALSE);
    	}	else
    		str << "\n General Ip = "<< CIPV4Wrapper(m_GeneralIp.addr.v4);
    	if(m_AudioIp.ipVersion == eIpVersion6 )
		{
    		memset(ipV6str, 0, sizeof(ipV6str));
			str << "\n Audio Ip = " << ::ipV6ToString(m_AudioIp.addr.v6.ip, ipV6str, FALSE);
		} else
			str << "\n Audio Ip = " << CIPV4Wrapper(m_AudioIp.addr.v4);
        str << "\n m_audioRtpPort = " << m_audioRtpPort
        << "\n m_audioRtcpPort = " << m_audioRtcpPort;
        if(m_VideoIp.ipVersion == eIpVersion6 )
		{
			memset(ipV6str, 0, sizeof(ipV6str));
			str << "\n Video Ip = " << ::ipV6ToString(m_VideoIp.addr.v6.ip, ipV6str, FALSE);
		} else
			str << "\n Video Ip = " << CIPV4Wrapper(m_VideoIp.addr.v4);
		str << "\n m_videoRtpPort = " << m_videoRtpPort
        << "\n m_videoRtcpPort = " << m_videoRtcpPort;
        if(m_DataIp.ipVersion == eIpVersion6 )
		{
			memset(ipV6str, 0, sizeof(ipV6str));
			str << "\n Data Ip = " << ::ipV6ToString(m_DataIp.addr.v6.ip, ipV6str, FALSE);
		} else
			str << "\n Data Ip = " << CIPV4Wrapper(m_DataIp.addr.v4);
		str << "\n m_dataRtpPort = " << m_dataRtpPort
        << "\n m_dataRtcpPort = " << m_dataRtcpPort;
		if(m_ContentIp.ipVersion == eIpVersion6 )
		{
			memset(ipV6str, 0, sizeof(ipV6str));
			str << "\n Content Ip = " << ::ipV6ToString(m_ContentIp.addr.v6.ip, ipV6str, FALSE);
		} else
			str << "\n Content Ip = " << CIPV4Wrapper(m_ContentIp.addr.v4);
        str << "\n m_contentRtpPort = " << m_contentRtpPort
        << "\n m_contentRtcpPort = " << m_contentRtcpPort
		<< "\n===   SubTypes === "
		<< "\n Audio Sub Type = "<< (BYTE)m_audioSubType
		<< "\n Video Sub Type = "<< (BYTE)m_videoSubType
		<< "\n Data Sub Type = "<< (BYTE)m_dataSubType;
    PTRACE (eLevelInfoNormal, str.GetString());
}
/////////////////////////////////////////////////////////////////
mcTransportAddress * CIceParams::GetIceMediaIp (ICESessionsTypes mediaType)
{
    mcTransportAddress *pIp = NULL;
    switch (mediaType)
    {
        case eAudioSession:
            pIp = &m_AudioIp;
            break;
        case eVideoSession:
            pIp = &m_VideoIp;
            break;
        case eDataSession:
            pIp = &m_DataIp;
            break;
        case eGeneralSession:
        	pIp = &m_GeneralIp;
        	break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
    }
    return pIp;
    
}
/////////////////////////////////////////////////////////////////
void CIceParams::SetIceMediaIp (ICESessionsTypes mediaType, mcTransportAddress * pIp)
{
    if (NULL == pIp)
    {
        PTRACE2INT (eLevelInfoNormal, "No ip sent for media - ", mediaType);
        return;
    }
    
    switch (mediaType)
    {
        case eAudioSession:
        {
            	memset (&m_AudioIp, 0, sizeof(mcTransportAddress));
            	memcpy (&m_AudioIp, pIp, sizeof(mcTransportAddress));
            	break;
        }
            
        case eVideoSession:
        {
            	memset (&m_VideoIp, 0, sizeof(mcTransportAddress));
            	memcpy (&m_VideoIp, pIp, sizeof(mcTransportAddress));
            	break;
        }
        case eDataSession:
        {
            	memset (&m_DataIp, 0, sizeof(mcTransportAddress));
            	memcpy (&m_DataIp, pIp, sizeof(mcTransportAddress));
            	break;
        }
        case eGeneralSession:
        {
				memset (&m_GeneralIp, 0, sizeof(mcTransportAddress));
				memcpy (&m_GeneralIp, pIp, sizeof(mcTransportAddress));
				memset (&m_AudioIp, 0, sizeof(mcTransportAddress));
            	memcpy (&m_AudioIp, pIp, sizeof(mcTransportAddress));
            	memset (&m_VideoIp, 0, sizeof(mcTransportAddress));
            	memcpy (&m_VideoIp, pIp, sizeof(mcTransportAddress));
            	memset (&m_DataIp, 0, sizeof(mcTransportAddress));
            	memcpy (&m_DataIp, pIp, sizeof(mcTransportAddress));
            	break;
        }
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
    }
}

void  CIceParams::SetSubType(cmCapDataType dataType,  eMediaLineSubType subType)
{

	switch(dataType)
	{
		case cmCapAudio:
			m_audioSubType = subType;
			break;
		case cmCapVideo:
			m_videoSubType = subType;
			break;
		case cmCapData:
			m_dataSubType = subType;
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
}

eMediaLineSubType  CIceParams::GetSubType(cmCapDataType dataType, bool bSdes)
{
	return ConvertSdesSubType(GetSubType(dataType), bSdes);
}

eMediaLineSubType  CIceParams::GetSubType(cmCapDataType dataType)
{
	eMediaLineSubType retVal = eMediaLineSubTypeRtpAvp;

	switch(dataType)
	{
		case cmCapAudio:
			retVal =  m_audioSubType;
			break;
		case cmCapVideo:
			retVal = m_videoSubType;
			break;
		case cmCapData:
			retVal = m_dataSubType;
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	return retVal;
}

eMediaLineSubType CIceParams::ConvertSdesSubType(eMediaLineSubType subType, bool bSdes)
{
	switch(subType)
	{
		case eMediaLineSubTypeRtpAvp:
		case eMediaLineSubTypeRtpSavp:
		{
			return (bSdes)?(eMediaLineSubTypeRtpSavp):(eMediaLineSubTypeRtpAvp);
		}
		case eMediaLineSubTypeTcpRtpAvp:
		case eMediaLineSubTypeTcpRtpSavp:
		{
			return (bSdes)?(eMediaLineSubTypeTcpRtpSavp):(eMediaLineSubTypeTcpRtpAvp);
		}
		default:
			return (bSdes)?(eMediaLineSubTypeRtpSavp):(eMediaLineSubTypeRtpAvp);
	}
}

////////////////////////////////////////////////////////////////////////////////
bool CIceParams::IsSubTypeSdes(eMediaLineSubType subType)
{
	switch(subType)
	{
		case eMediaLineSubTypeRtpSavp:
		case eMediaLineSubTypeTcpRtpSavp:
			return true;
		default:
			return false;
	}
}

////////////////////////////////////////////////////////////////////////////////
void CIceParams::Serialize(WORD format,CSegment& seg)
{
	if (format == NATIVE)
	{
		seg << m_ice_session_id;
		seg << m_ice_audio_rtp_id;
		seg << m_ice_audio_rtcp_id;

		seg << m_ice_video_rtp_id;
		seg << m_ice_video_rtcp_id;

		seg << m_ice_data_rtp_id;
		seg << m_ice_data_rtcp_id;

		seg << m_ice_content_rtp_id;
		seg << m_ice_content_rtcp_id;

		seg << (DWORD)m_GeneralIp.ipVersion;
		seg << (DWORD)m_GeneralIp.port;
		seg << (DWORD)m_GeneralIp.distribution;
		seg << (DWORD)m_GeneralIp.transportType;
		if ((enIpVersion)m_GeneralIp.ipVersion == eIpVersion4)
			seg << (DWORD)m_GeneralIp.addr.v4.ip;
		else
		{
			seg << (DWORD)m_GeneralIp.addr.v6.scopeId;
			char szIP[64];
			::ipToString(m_GeneralIp, szIP,0); // With Brackets
			seg << szIP;
		}
			//Audio
		seg << m_audioRtpPort;
		seg << m_audioRtcpPort;

		seg << (DWORD)m_AudioIp.ipVersion;
		seg << (DWORD)m_AudioIp.port;
		seg << (DWORD)m_AudioIp.distribution;
		seg << (DWORD)m_AudioIp.transportType;
		if ((enIpVersion)m_AudioIp.ipVersion == eIpVersion4)
			seg << (DWORD)m_AudioIp.addr.v4.ip;
		else
		{
			seg << (DWORD)m_AudioIp.addr.v6.scopeId;
			char szIP[64];
			::ipToString(m_AudioIp, szIP,0); // With Brackets
			seg << szIP;
		}

		//Video
		seg << m_videoRtpPort;
		seg << m_videoRtcpPort;

		seg << (DWORD)m_VideoIp.ipVersion;
		seg << (DWORD)m_VideoIp.port;
		seg << (DWORD)m_VideoIp.distribution;
		seg << (DWORD)m_VideoIp.transportType;
		if ((enIpVersion)m_VideoIp.ipVersion == eIpVersion4)
			seg << (DWORD)m_VideoIp.addr.v4.ip;
		else
		{
			seg << (DWORD)m_VideoIp.addr.v6.scopeId;
			char szIP[64];
			::ipToString(m_VideoIp, szIP,0); // With Brackets
			seg << szIP;
		}

		//Data
		seg << 	m_dataRtpPort;
		seg << 	m_dataRtcpPort;

		seg << (DWORD)m_DataIp.ipVersion;
		seg << (DWORD)m_DataIp.port;
		seg << (DWORD)m_DataIp.distribution;
		seg << (DWORD)m_DataIp.transportType;
		if ((enIpVersion)m_DataIp.ipVersion == eIpVersion4)
			seg << (DWORD)m_DataIp.addr.v4.ip;
		else
		{
			seg << (DWORD)m_DataIp.addr.v6.scopeId;
			char szIP[64];
			::ipToString(m_DataIp, szIP,0); // With Brackets
			seg << szIP;
		}

		//Content
		seg << 	m_contentRtpPort;
		seg <<	m_contentRtcpPort;

		seg << (DWORD)m_DataIp.ipVersion;
		seg << (DWORD)m_DataIp.port;
		seg << (DWORD)m_DataIp.distribution;
		seg << (DWORD)m_DataIp.transportType;
		if ((enIpVersion)m_DataIp.ipVersion == eIpVersion4)
			seg << (DWORD)m_DataIp.addr.v4.ip;
		else
		{
			seg << (DWORD)m_ContentIp.addr.v6.scopeId;
			char szIP[64];
			::ipToString(m_ContentIp, szIP,0); // With Brackets
			seg << szIP;
		}

		seg << (BYTE)m_audioSubType;
		seg << (BYTE)m_videoSubType;
		seg << (BYTE)m_dataSubType;

	}
}

////////////////////////////////////////////////////////////////////////////////
void CIceParams::DeSerialize(WORD format,CSegment& seg)
{
	if (format == NATIVE)
	{
		seg >> m_ice_session_id;
		seg >> m_ice_audio_rtp_id;
		seg >> m_ice_audio_rtcp_id;

		seg >> m_ice_video_rtp_id;
		seg >> m_ice_video_rtcp_id;

		seg >> m_ice_data_rtp_id;
		seg >> m_ice_data_rtcp_id;

		seg >> m_ice_content_rtp_id;
		seg >> m_ice_content_rtcp_id;

		seg >> m_GeneralIp.ipVersion;
		seg >> m_GeneralIp.port;
		seg >> m_GeneralIp.distribution;
		seg >> m_GeneralIp.transportType;
		if ((enIpVersion)m_GeneralIp.ipVersion == eIpVersion4)
			seg >> m_GeneralIp.addr.v4.ip;
		else
		{
			seg >> m_GeneralIp.addr.v6.scopeId ;
			char szIP[64];
			memset(szIP,'\0',64);
			seg >> szIP;
			::stringToIp(&m_GeneralIp, szIP); // With Brackets
		}

		//Audio
		seg >> m_audioRtpPort;
		seg >> m_audioRtcpPort;

		seg >> m_AudioIp.ipVersion;
		seg >> m_AudioIp.port;
		seg >> m_AudioIp.distribution;
		seg >> m_AudioIp.transportType;
		if ((enIpVersion)m_AudioIp.ipVersion == eIpVersion4)
			seg >> m_AudioIp.addr.v4.ip;
		else
		{
			seg >> m_AudioIp.addr.v6.scopeId ;
			char szIP[64];
			memset(szIP,'\0',64);
			seg >> szIP;
			::stringToIp(&m_AudioIp, szIP); // With Brackets
		}

		//Video
		seg >> m_videoRtpPort;
		seg >> m_videoRtcpPort;

		seg >> m_VideoIp.ipVersion;
		seg >> m_VideoIp.port;
		seg >> m_VideoIp.distribution;
		seg >> m_VideoIp.transportType;
		if ((enIpVersion)m_VideoIp.ipVersion == eIpVersion4)
			seg >> m_VideoIp.addr.v4.ip;
		else
		{
			seg >> m_VideoIp.addr.v6.scopeId ;
			char szIP[64];
			memset(szIP,'\0',64);
			seg >> szIP;
			::stringToIp(&m_VideoIp, szIP); // With Brackets
		}

		//Data
		seg >> 	m_dataRtpPort;
		seg >> 	m_dataRtcpPort;

		seg >> m_DataIp.ipVersion;
		seg >> m_DataIp.port;
		seg >> m_DataIp.distribution;
		seg >> m_DataIp.transportType;
		if ((enIpVersion)m_DataIp.ipVersion == eIpVersion4)
			seg >> m_DataIp.addr.v4.ip;
		else
		{
			seg >> m_DataIp.addr.v6.scopeId ;
			char szIP[64];
			memset(szIP,'\0',64);
			seg >> szIP;
			::stringToIp(&m_DataIp, szIP); // With Brackets
		}

		//Content
		seg >> 	m_contentRtpPort;
		seg >>	m_contentRtcpPort;

		seg >> m_ContentIp.ipVersion;
		seg >> m_ContentIp.port;
		seg >> m_ContentIp.distribution;
		seg >> m_ContentIp.transportType;
		if ((enIpVersion)m_ContentIp.ipVersion == eIpVersion4)
			seg >> m_ContentIp.addr.v4.ip;
		else
		{
			seg >> m_ContentIp.addr.v6.scopeId ;
			char szIP[64];
			memset(szIP,'\0',64);
			seg >> szIP;
			::stringToIp(&m_ContentIp, szIP); // With Brackets
		}

		BYTE subType;
		subType = 0;
		seg >> subType;
		m_audioSubType = (eMediaLineSubType)subType;
		subType = 0;
		seg >> subType;
		m_videoSubType = (eMediaLineSubType)subType;
		subType = 0;
		seg >> subType;
		m_dataSubType = (eMediaLineSubType)subType;
	}
}

///////////////////////////////////////////////////////////////////////////////
CIceParams& CIceParams::operator =(const CIceParams& other)
{

		m_ice_session_id = other.m_ice_session_id;

		m_ice_audio_rtp_id = other.m_ice_audio_rtp_id;
		m_ice_audio_rtcp_id = other.m_ice_audio_rtcp_id;

		m_ice_video_rtp_id = other.m_ice_video_rtp_id;
		m_ice_video_rtcp_id = other.m_ice_video_rtcp_id;

		m_ice_data_rtp_id = other.m_ice_data_rtp_id;
		m_ice_data_rtcp_id = other.m_ice_data_rtcp_id;

		m_ice_content_rtp_id = other.m_ice_content_rtp_id;
		m_ice_content_rtcp_id = other.m_ice_content_rtcp_id;

		memset(&m_GeneralIp,0,sizeof(mcTransportAddress));
		memcpy(&m_GeneralIp, &other.m_GeneralIp, sizeof(mcTransportAddress));

		m_audioRtpPort = other.m_audioRtpPort;
		m_audioRtcpPort = other.m_audioRtcpPort;

		memset(&m_AudioIp,0,sizeof(mcTransportAddress));
		memcpy(&m_AudioIp, &other.m_AudioIp, sizeof(mcTransportAddress));

		m_videoRtpPort = other.m_videoRtpPort;
		m_videoRtcpPort = other.m_videoRtcpPort;

		memset(&m_VideoIp,0,sizeof(mcTransportAddress));
		memcpy(&m_VideoIp, &other.m_VideoIp, sizeof(mcTransportAddress));

		m_dataRtpPort = other.m_dataRtpPort;
		m_dataRtcpPort = other.m_dataRtcpPort;

		memset(&m_DataIp,0,sizeof(mcTransportAddress));
		memcpy(&m_DataIp, &other.m_DataIp, sizeof(mcTransportAddress));

		m_contentRtpPort = other.m_contentRtpPort;
		m_contentRtcpPort = other.m_contentRtcpPort;

		memset(&m_ContentIp,0,sizeof(mcTransportAddress));
		memcpy(&m_ContentIp, &other.m_ContentIp, sizeof(mcTransportAddress));

		m_audioSubType = other.m_audioSubType;
		m_videoSubType = other.m_videoSubType;
		m_dataSubType  = other.m_dataSubType;

	    return *this;
}


