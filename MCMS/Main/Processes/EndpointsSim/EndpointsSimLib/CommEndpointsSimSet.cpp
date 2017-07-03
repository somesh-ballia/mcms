// CommEndpointsSimSet.cpp:
//
//
//Date         Updated By         Description
//
//21/11/05	     Vasily 		Used in XML transaction.
//========   ==============   =====================================================================

#include <stdlib.h>
#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "CommEndpointsSimSet.h"
#include "InitCommonStrings.h"
#include "EpSimCapSetsList.h"
#include "OperCfg.h"
#include "EpSimCapSetsList.h"
#include "EndpointsSim.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////
//		USE IN 'DEL H323 PARTY' TRANSACTION
//		USE IN 'CONNECT H323 PARTY' TRANSACTION
//		USE IN 'DISCONNECT H323 PARTY' TRANSACTION
//		USE IN 'DEL SIP PARTY' TRANSACTION
//		USE IN 'CONNECT SIP PARTY' TRANSACTION
//		USE IN 'DISCONNECT SIP PARTY' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetPartyCommon::CCommSetPartyCommon()
{
	memset(m_szPartyName,0,H243_NAME_LEN);
	strncpy(m_szPartyName,"JUNK_NAME",H243_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyCommon::CCommSetPartyCommon(const CCommSetPartyCommon& other) : CSerializeObject(other)
{
//	memset(m_szPartyName,0,H243_NAME_LEN);
//	strncpy(m_szPartyName,other.m_szPartyName,H243_NAME_LEN);
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyCommon::~CCommSetPartyCommon()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetPartyCommon& CCommSetPartyCommon::operator= (const CCommSetPartyCommon& other)
{
	if( this == &other )
		return *this;

	memset(m_szPartyName,0,H243_NAME_LEN);
	strncpy(m_szPartyName,other.m_szPartyName,H243_NAME_LEN);

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetPartyCommon::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetPartyCommon::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"PARTY_NAME",m_szPartyName,_1_TO_H243_NAME_LENGTH);

	return nStatus;
}


//////////////////////////////////////////////////////////////////////
//		USE IN 'SIP_MUTE' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetPartyMute::CCommSetPartyMute()
{
	m_wMuteAudioByPort = 0xc020;
	m_wMuteVideoByPort = 0xaac2;
	memset(m_szMuteAudioByDirection,0,H243_NAME_LEN);
	memset(m_szMuteVideoByDirection,0,H243_NAME_LEN);
	memset(m_szMuteAudioByInactive,0,H243_NAME_LEN);
	memset(m_szMuteVideoByInactive,0,H243_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyMute::CCommSetPartyMute(const CCommSetPartyMute& other) : CCommSetPartyCommon(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyMute::~CCommSetPartyMute()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetPartyMute& CCommSetPartyMute::operator= (const CCommSetPartyMute& other)
{
	if( this == &other )
		return *this;

	*((CCommSetPartyCommon*)this) = *((CCommSetPartyCommon*)&other);

	m_wMuteAudioByPort = other.m_wMuteAudioByPort;
	m_wMuteVideoByPort = other.m_wMuteVideoByPort;

	memset(m_szMuteAudioByDirection,0,H243_NAME_LEN);
	strncpy(m_szMuteAudioByDirection,other.m_szMuteAudioByDirection,H243_NAME_LEN);
	memset(m_szMuteVideoByDirection,0,H243_NAME_LEN);
	strncpy(m_szMuteVideoByDirection,other.m_szMuteVideoByDirection,H243_NAME_LEN);
	memset(m_szMuteAudioByInactive,0,H243_NAME_LEN);
	strncpy(m_szMuteAudioByInactive,other.m_szMuteAudioByInactive,H243_NAME_LEN);
	memset(m_szMuteVideoByInactive,0,H243_NAME_LEN);
	strncpy(m_szMuteVideoByInactive,other.m_szMuteVideoByInactive,H243_NAME_LEN);

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetPartyMute::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetPartyMute::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = CCommSetPartyCommon::DeSerializeXml(pActionNode,pszError,action);

	if( STATUS_OK == nStatus ) {
		GET_VALIDATE_CHILD(pActionNode,"MUTE_BY_PORT_AUDIO",&m_wMuteAudioByPort,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"MUTE_BY_PORT_VIDEO",&m_wMuteVideoByPort,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"MUTE_BY_DIRECTION_AUDIO",m_szMuteAudioByDirection,_1_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pActionNode,"MUTE_BY_DIRECTION_VIDEO",m_szMuteVideoByDirection,_1_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pActionNode,"MUTE_BY_INACTIVE_AUDIO",m_szMuteAudioByInactive,_1_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pActionNode,"MUTE_BY_INACTIVE_VIDEO",m_szMuteVideoByInactive,_1_TO_H243_NAME_LENGTH);
	}

	return nStatus;
}

//////////////////////////////////////////////////////////////////////
//		USE IN 'ADD H323 PARTY' TRANSACTION
//		USE IN 'ADD SIP PARTY' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetPartyAdd::CCommSetPartyAdd()
{
	memset(m_szConfName,0,H243_NAME_LEN);
	memset(m_szCapSetName,0,H243_NAME_LEN);
	strncpy(m_szCapSetName,"FULL CAPSET",H243_NAME_LEN);
	m_ipVersion = 0;
	m_csID = (DWORD)-1;
	memset(m_szManufacturer,0,H243_NAME_LEN);
	memset(m_sourcePartyAlias,0,H243_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyAdd::CCommSetPartyAdd(const CCommSetPartyAdd& other) : CCommSetPartyCommon(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyAdd::~CCommSetPartyAdd()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetPartyAdd& CCommSetPartyAdd::operator= (const CCommSetPartyAdd& other)
{
	if( this == &other )
		return *this;

	*((CCommSetPartyCommon*)this) = *((CCommSetPartyCommon*)&other);

	memset(m_szConfName,0,H243_NAME_LEN);
	strncpy(m_szConfName,other.m_szConfName,H243_NAME_LEN);
	memset(m_szCapSetName,0,H243_NAME_LEN);
	strncpy(m_szCapSetName,other.m_szCapSetName,H243_NAME_LEN);
	m_ipVersion = other.m_ipVersion;
	m_csID = other.m_csID;
	strcpy(m_szManufacturer,other.m_szManufacturer);
	strcpy(m_sourcePartyAlias,other.m_sourcePartyAlias);

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetPartyAdd::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetPartyAdd::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = CCommSetPartyCommon::DeSerializeXml(pActionNode,pszError,action);
	PTRACE(eLevelInfoNormal,"CCommSetPartyAdd::DeSerializeXml");
	if( STATUS_OK != nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"CONF_NAME",m_szConfName,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"CAPSET_NAME",m_szCapSetName,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"IP_VER",&m_ipVersion,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"CS_ID",&m_csID,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"MANUFUCTURER_NAME",m_szManufacturer,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"SOURCE_PARTY_ALIAS",m_sourcePartyAlias,_1_TO_H243_NAME_LENGTH);

	return STATUS_OK;
}

DWORD CCommSetPartyAdd::GetCSID(void) const
{
    return m_csID;
}


/////////////////////////////////////////////////////////////
//USE IN 'ADD SIP PARTY' TRANSACTION
//////////////////////////////////////////////////////////////////////

CCommSetPartyAddSipEP::CCommSetPartyAddSipEP()
{
	memset(m_szUserAgent,0,IP_STRING_LEN);
}
CCommSetPartyAddSipEP::~CCommSetPartyAddSipEP()
{
}
//////////////////////////////////////////////////////////////////////
CCommSetPartyAddSipEP::CCommSetPartyAddSipEP(const CCommSetPartyAddSipEP& other) : CCommSetPartyAdd(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CCommSetPartyAddSipEP& CCommSetPartyAddSipEP::operator= (const CCommSetPartyAddSipEP& other)
{
	if( this == &other )
		return *this;

	*((CCommSetPartyAdd*)this) = *((CCommSetPartyAdd*)&other);

	memset(m_szUserAgent,0,IP_STRING_LEN);
	strncpy(m_szUserAgent,other.m_szUserAgent,IP_STRING_LEN-1);


	return *this;
}
int CCommSetPartyAddSipEP::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = CCommSetPartyAdd::DeSerializeXml(pActionNode,pszError,action);

	if( STATUS_OK != nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"USER_AGENT",m_szUserAgent,IP_STRING_LENGTH);
	PTRACE2(eLevelInfoNormal,"CCommSetPartyAddSipEP::DeSerializeXml",m_szUserAgent);


	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//		USE IN 'ADD ISDN PARTY' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetAddIsdnEndpoint::CCommSetAddIsdnEndpoint()
{
	memset(m_szPhoneNum,0,H243_NAME_LEN);
	memset(m_szCapSetName,0,H243_NAME_LEN);
	strncpy(m_szCapSetName,"FULL CAPSET",H243_NAME_LEN);
	m_numberOfChannels = 6;
}

//////////////////////////////////////////////////////////////////////
CCommSetAddIsdnEndpoint::CCommSetAddIsdnEndpoint(const CCommSetAddIsdnEndpoint& other) : CCommSetPartyCommon(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetAddIsdnEndpoint::~CCommSetAddIsdnEndpoint()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetAddIsdnEndpoint& CCommSetAddIsdnEndpoint::operator= (const CCommSetAddIsdnEndpoint& other)
{
	if( this == &other )
		return *this;

	*((CCommSetPartyCommon*)this) = *((CCommSetPartyCommon*)&other);

	memset(m_szPhoneNum,0,H243_NAME_LEN);
	strncpy(m_szPhoneNum,other.m_szPhoneNum,H243_NAME_LEN);
	memset(m_szCapSetName,0,H243_NAME_LEN);
	strncpy(m_szCapSetName,other.m_szCapSetName,H243_NAME_LEN);
	m_numberOfChannels = other.m_numberOfChannels;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetAddIsdnEndpoint::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetAddIsdnEndpoint::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = CCommSetPartyCommon::DeSerializeXml(pActionNode,pszError,action);

	if( STATUS_OK != nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"CAPSET_NAME",m_szCapSetName,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PHONE_NUMBER",m_szPhoneNum,_1_TO_H243_NAME_LENGTH);
	//GET_VALIDATE_CHILD(pActionNode,"CHANNELS_DIAL_IN_NUMBER",&m_numberOfChannels,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"CHANNELS_DIAL_IN_NUMBER",&m_numberOfChannels,NET_CHANNEL_NUMBER_ENUM);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
//		USE IN 'H323 PARTY SENDS DTMF' TRANSACTION
//		USE IN 'SIP PARTY SENDS DTMF' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetPartyDtmf::CCommSetPartyDtmf()
{
	memset(m_szDtmfString,0,H243_NAME_LEN);
	m_wDtmfSource = (WORD)(-1);
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyDtmf::CCommSetPartyDtmf(const CCommSetPartyDtmf& other) : CCommSetPartyCommon(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyDtmf::~CCommSetPartyDtmf()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetPartyDtmf& CCommSetPartyDtmf::operator= (const CCommSetPartyDtmf& other)
{
	if( this == &other )
		return *this;

	*((CCommSetPartyCommon*)this) = *((CCommSetPartyCommon*)&other);

	m_wDtmfSource = other.m_wDtmfSource;

	memcpy(m_szDtmfString,other.m_szDtmfString,H243_NAME_LEN);

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetPartyDtmf::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetPartyDtmf::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = CCommSetPartyCommon::DeSerializeXml(pActionNode,pszError,action);

	if( STATUS_OK == nStatus ) {
		GET_VALIDATE_CHILD(pActionNode,"DTMF_STRING",m_szDtmfString,_1_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pActionNode,"DTMF_SOURCE",&m_wDtmfSource,DTMF_SOURCE_TYPE_ENUM);
		m_wDtmfSource = eDtmfSourceAUDIO;
	}

	return nStatus;
}


//////////////////////////////////////////////////////////////////////
//		USE IN 'H323_PARTY_BITRATE_ERROR' TRANSACTION
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
CCommSetBitRateError::CCommSetBitRateError()
{
	m_errorBitRate = 0;
}

//////////////////////////////////////////////////////////////////////
CCommSetBitRateError::CCommSetBitRateError(const CCommSetBitRateError& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetBitRateError::~CCommSetBitRateError()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetBitRateError& CCommSetBitRateError::operator= (const CCommSetBitRateError& other)
{
	if( this == &other )
		return *this;

//	*((CCommSetPartyCommon*)this) = *((CCommSetPartyCommon*)&other);

	m_errorBitRate = other.m_errorBitRate;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetBitRateError::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetBitRateError::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK; //CCommSetPartyCommon::DeSerializeXml(pActionNode,pszError,action);

//	if( STATUS_OK == nStatus ) {
		GET_VALIDATE_CHILD(pActionNode,"PARTY_H323_ERROR_BITRATE",&m_errorBitRate,_0_TO_DWORD);
//	}

	return nStatus;
}

//////////////////////////////////////////////////////////////////////
//		USE IN 'ENDPOINT_UPDATE_CHANNELS' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetPartyUpdateChannels::CCommSetPartyUpdateChannels()
{
	m_isAudioChannelOpen = TRUE;
	m_isVideoChannelOpen = TRUE;
	m_isFeccChannelOpen  = TRUE;
	m_isH239ChannelOpen  = TRUE;
	m_nRecapMode = 0;
	m_iCapId = 0;
	memset(m_szManufacturer,0,H243_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyUpdateChannels::CCommSetPartyUpdateChannels(const CCommSetPartyUpdateChannels& other) : CCommSetPartyCommon(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyUpdateChannels::~CCommSetPartyUpdateChannels()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetPartyUpdateChannels& CCommSetPartyUpdateChannels::operator= (const CCommSetPartyUpdateChannels& other)
{
	if( this == &other )
		return *this;

	*((CCommSetPartyCommon*)this) = *((CCommSetPartyCommon*)&other);

	m_isAudioChannelOpen = other.m_isAudioChannelOpen;
	m_isVideoChannelOpen = other.m_isVideoChannelOpen;
	m_isFeccChannelOpen  = other.m_isFeccChannelOpen;
	m_isH239ChannelOpen  = other.m_isH239ChannelOpen;
	m_nRecapMode = other.m_nRecapMode;
	m_iCapId = other.m_iCapId;
	strncpy(m_szManufacturer,other.m_szManufacturer,H243_NAME_LEN);

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetPartyUpdateChannels::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetPartyUpdateChannels::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = CCommSetPartyCommon::DeSerializeXml(pActionNode,pszError,action);

	if( STATUS_OK == nStatus ) {
		GET_VALIDATE_CHILD(pActionNode,"AUDIO_CHANNEL_OPEN",&m_isAudioChannelOpen,_BOOL);
		GET_VALIDATE_CHILD(pActionNode,"VIDEO_CHANNEL_OPEN",&m_isVideoChannelOpen,_BOOL);
		GET_VALIDATE_CHILD(pActionNode,"FECC_CHANNEL_OPEN",&m_isFeccChannelOpen,_BOOL);
		GET_VALIDATE_CHILD(pActionNode,"H239_CHANNEL_OPEN",&m_isH239ChannelOpen,_BOOL);
		GET_VALIDATE_CHILD(pActionNode,"RECAP_MODE",&m_nRecapMode,SIM_RECAP_MODE_ENUM);
		GET_VALIDATE_CHILD(pActionNode,"CAPSET_ID",&m_iCapId,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pActionNode,"MANUFACTURER",m_szManufacturer,_0_TO_H243_NAME_LENGTH);
	}

	return nStatus;
}

//////////////////////////////////////////////////////////////////////
//		USE IN 'ADD_CAP_SET' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetAddCapset::CCommSetAddCapset()
{
	memset(m_szCapsetName,0,H243_NAME_LEN);
	m_nCallRate = 384;
	m_fecc = TRUE;
	m_encrypted = FALSE;
	m_h239 = TRUE;
	m_h264 = TRUE;
	m_mode264 = (WORD)eVideoModeCif;
	m_aspectRatio = (DWORD)-1;
	m_staticMB = (DWORD)-1;
	m_h263 = TRUE;
	m_VP8 = TRUE; //N.A. DEBUG VP8
	m_g711 = TRUE;
	m_g722 = TRUE;
	m_g7221 = TRUE;
	m_g7221C = TRUE;
	m_g7231 = TRUE;
	m_g728 = TRUE;
	m_g729 = TRUE;
	m_siren14 = TRUE;
	m_siren7 = TRUE;
	m_opus = TRUE;

	//m_AAC_LD = TRUE;// TIP
}

//////////////////////////////////////////////////////////////////////
CCommSetAddCapset::CCommSetAddCapset(const CCommSetAddCapset& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetAddCapset::~CCommSetAddCapset()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetAddCapset& CCommSetAddCapset::operator= (const CCommSetAddCapset& other)
{
	if( this == &other )
		return *this;

	strncpy(m_szCapsetName,other.m_szCapsetName,H243_NAME_LEN);
	m_nCallRate = other.m_nCallRate;
	m_encrypted = other.m_encrypted;
	m_fecc  = other.m_fecc;
	m_h239  = other.m_h239;
	m_h264  = other.m_h264;
	m_mode264 = other.m_mode264;
	m_aspectRatio = other.m_aspectRatio;
	m_staticMB = other.m_staticMB;
	m_h263  = other.m_h263;
	m_VP8	= other.m_VP8; //N.A. DEBUG VP8
	m_g711  = other.m_g711;
	m_g722  = other.m_g722;
	m_g7221 = other.m_g7221;
	m_g7221C = other.m_g7221C;
	m_g7231 = other.m_g7231;
	m_g728  = other.m_g728;
	m_g729  = other.m_g729;
	m_siren7 = other.m_siren7;
	m_siren14 = other.m_siren14;
	m_opus = other.m_opus;
	//m_AAC_LD = other.m_AAC_LD;// TIP

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetAddCapset::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetAddCapset::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_szCapsetName,_1_TO_H243_NAME_LENGTH);
	if( STATUS_OK != nStatus )
		return nStatus; // must field

	WORD  wTmp = 384;
	GET_VALIDATE_CHILD(pActionNode,"CALL_RATE",&wTmp,_0_TO_WORD);
	if( STATUS_OK == nStatus )
	{
		if( wTmp % 64 == 0 )
			m_nCallRate = wTmp;
		else
			m_nCallRate = (wTmp / 64 + 1) * 64; // calibrate to 64k
	}

	GET_VALIDATE_CHILD(pActionNode,"FECC",&m_fecc,_BOOL);

	GET_VALIDATE_CHILD(pActionNode,"ENCRYPTED",&m_encrypted,_BOOL);

	GET_VALIDATE_CHILD(pActionNode,"H239",&m_h239,_BOOL);

		// audio
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_G711",&m_g711,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_G722",&m_g722,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_G7221",&m_g7221,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_G7221C",&m_g7221C,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_G7231",&m_g7231,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_G728",&m_g728,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_G729",&m_g729,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_SIREN7",&m_siren7,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_SIREN14",&m_siren14,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_OPUS",&m_opus,_BOOL);
	//GET_VALIDATE_CHILD(pActionNode,"AUDIO_AACLD",&m_AAC_LD,_BOOL);// TIP

		// video
			// H264
	GET_VALIDATE_CHILD(pActionNode,"VIDEO_H264",&m_h264,_BOOL);

	// get <VIDEO_H264_DETAILS> section
	CXMLDOMElement*		pChildNode = NULL;
	GET_CHILD_NODE(pActionNode,"VIDEO_H264_DETAILS",pChildNode);

	if( pChildNode )
	{
		GET_VALIDATE_CHILD(pChildNode,"VIDEO_MODE_H264",&m_mode264,VIDEO_MODE_H264_ENUM);
		int tmp = -1;

//		GET_VALIDATE_CHILD(pChildNode,"H264_ASPECT_RATIO",&tmp,SIM_VIDEO_H264_ASPECT_RATIO_LIMITS);
		CXMLDOMElement*		pRatioNode = NULL;
		GET_CHILD_NODE(pChildNode,"H264_ASPECT_RATIO",pRatioNode);
		if( NULL != pRatioNode )
		{
			char* pszNodeValue = NULL;
			pRatioNode->get_nodeValue(&pszNodeValue);
			if( NULL != pszNodeValue )
			{
				if( 0 == strcmp(pszNodeValue,"-1") )
					m_aspectRatio = (DWORD)-1;
				else
					m_aspectRatio = atoi(pszNodeValue);
			}
		}

		CXMLDOMElement*		pStaticMbMode = NULL;
		GET_CHILD_NODE(pChildNode,"H264_STATIC_MB",pStaticMbMode);
		if( NULL != pStaticMbMode )
		{
			char* pszNodeValue = NULL;
			pStaticMbMode->get_nodeValue(&pszNodeValue);
			if( NULL != pszNodeValue )
			{
				if( 0 == strcmp(pszNodeValue,"-1") )
					m_staticMB = (DWORD)-1;
				else
					m_staticMB = atoi(pszNodeValue);
			}
		}
	}

			// H263
	GET_VALIDATE_CHILD(pActionNode,"VIDEO_H263",&m_h263,_BOOL);

	//VP8 //N.A. DEBUG VP8
	GET_VALIDATE_CHILD(pActionNode,"VIDEO_VP8",&m_VP8,_BOOL);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
//		USE IN 'REQUEST_CONF_DETAILS' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetExternalDbCreate::CCommSetExternalDbCreate()
{
		// input parameters
	memset(m_in_szNumericId,0,H243_NAME_LEN);
	memset(m_in_szPhone1,0,H243_NAME_LEN);
	memset(m_in_szPhone2,0,H243_NAME_LEN);
		// output parameters
	memset(m_out_szName,0,H243_NAME_LEN);
	m_out_wMaxParties = 25;
	m_out_wMinParties = 0;
	memset(m_out_szPassword,0,H243_NAME_LEN);
	memset(m_out_szEntryPassword,0,H243_NAME_LEN);
	memset(m_out_szBillingData,0,H243_NAME_LEN);
	memset(m_out_szOwner,0,H243_NAME_LEN);
	for( int i=0; i<3; i++ )
		memset(m_out_aszContactInfo[i],0,H243_NAME_LEN);
	memset(m_out_szDisplayName,0,H243_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////
CCommSetExternalDbCreate::CCommSetExternalDbCreate(const CCommSetExternalDbCreate& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetExternalDbCreate::~CCommSetExternalDbCreate()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetExternalDbCreate& CCommSetExternalDbCreate::operator= (const CCommSetExternalDbCreate& other)
{
	if( this == &other )
		return *this;

		// input parameters
	strncpy(m_in_szNumericId,other.m_in_szNumericId,H243_NAME_LEN);
	strncpy(m_in_szPhone1,other.m_in_szPhone1,H243_NAME_LEN);
	strncpy(m_in_szPhone2,other.m_in_szPhone2,H243_NAME_LEN);

		// output parameters
	strncpy(m_out_szName,other.m_out_szName,H243_NAME_LEN);
	m_out_wMaxParties = other.m_out_wMaxParties;
	m_out_wMinParties = other.m_out_wMinParties;
	strncpy(m_out_szPassword,other.m_out_szPassword,H243_NAME_LEN);
	strncpy(m_out_szEntryPassword,other.m_out_szEntryPassword,H243_NAME_LEN);
	strncpy(m_out_szBillingData,other.m_out_szBillingData,H243_NAME_LEN);
	strncpy(m_out_szOwner,other.m_out_szOwner,H243_NAME_LEN);
	for( int i=0; i<3; i++ )
		strncpy(m_out_aszContactInfo[i],other.m_out_aszContactInfo[i],H243_NAME_LEN);
	strncpy(m_out_szDisplayName, other.m_out_szDisplayName, H243_NAME_LEN);

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SetName(const char* pszNewName)
{
	strncpy(m_out_szName,pszNewName,sizeof(m_out_szName) - 1);
	m_out_szName[sizeof(m_out_szName) - 1] = '\0';
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SetMaxParties(const WORD wMaxParties)
{
	m_out_wMaxParties = wMaxParties;
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SetMinParties(const WORD wMinParties)
{
	m_out_wMinParties = wMinParties;
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SetPassword(const char* pszNewPwd)
{
	strncpy(m_out_szPassword,pszNewPwd,sizeof(m_out_szPassword) - 1);
	m_out_szPassword[sizeof(m_out_szPassword) - 1] = '\0';
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SetEntryPassword(const char* pszNewPwd)
{
	strncpy(m_out_szEntryPassword,pszNewPwd,sizeof(m_out_szEntryPassword) - 1);
	m_out_szEntryPassword[sizeof(m_out_szEntryPassword) - 1] = '\0';
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SetBillingData(const char* pszNewBillData)
{
	strncpy(m_out_szBillingData,pszNewBillData,sizeof(m_out_szBillingData) - 1);
	m_out_szBillingData[sizeof(m_out_szBillingData) - 1] = '\0';
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SetOwner(const char* pszNewOwner)
{
	strncpy(m_out_szOwner,pszNewOwner,sizeof(m_out_szOwner) - 1);
	m_out_szOwner[sizeof(m_out_szOwner) - 1] = '\0';
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SetContactInfo(const int ind,const char* pszInfo)
{
	if( ind >= 0 && ind < 3 )
	{
		strncpy(m_out_aszContactInfo[ind],pszInfo,sizeof(m_out_aszContactInfo[ind]) - 1);
		m_out_aszContactInfo[ind][sizeof(m_out_aszContactInfo[ind]) - 1] = '\0';
	}
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SetDisplayName(const char* pszNewDisplayName)
{
	strncpy(m_out_szDisplayName,pszNewDisplayName,sizeof(m_out_szDisplayName) - 1);
	m_out_szDisplayName[sizeof(m_out_szDisplayName) - 1] = '\0';
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbCreate::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("NAME",m_out_szName);
	pFatherNode->AddChildNode("MAX_PARTIES",m_out_wMaxParties);
	pFatherNode->AddChildNode("MIN_NUM_OF_PARTIES",m_out_wMinParties);
	pFatherNode->AddChildNode("PASSWORD",m_out_szPassword);
	pFatherNode->AddChildNode("ENTRY_PASSWORD",m_out_szEntryPassword);
	pFatherNode->AddChildNode("BILLING_DATA",m_out_szBillingData);
	pFatherNode->AddChildNode("OWNER",m_out_szOwner);

	// create <CONTACT_INFO_LIST> section
	CXMLDOMElement* pContactListNode = pFatherNode->AddChildNode("CONTACT_INFO_LIST");
	if( NULL != pContactListNode )
	{
		for( int i=0; i<3; i++ )
			pContactListNode->AddChildNode("CONTACT_INFO",m_out_aszContactInfo[i]);
	}

	pFatherNode->AddChildNode("DISPLAY_NAME",m_out_szDisplayName);
}

//////////////////////////////////////////////////////////////////////
int CCommSetExternalDbCreate::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NUMERIC_ID",m_in_szNumericId,_0_TO_H243_NAME_LENGTH);
//	if( STATUS_OK != nStatus )
//		return nStatus; // must field

	// get <ACTUAL_PARTY_PHONES> section
	CXMLDOMElement*		pChildNode = NULL;
	GET_CHILD_NODE(pActionNode,"ACTUAL_PARTY_PHONES",pChildNode);

	if( pChildNode )
	{
		GET_VALIDATE_CHILD(pChildNode,"PHONE1",m_in_szPhone1,_0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pChildNode,"PHONE2",m_in_szPhone2,_0_TO_H243_NAME_LENGTH);
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
//		USE IN 'REQUEST_PARTY_DETAILS' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetExternalDbAdd::CCommSetExternalDbAdd()
{
		// input parameters
	memset(m_in_szNumericId,0,H243_NAME_LEN);
	memset(m_in_szPhone1,0,H243_NAME_LEN);
	memset(m_in_szPhone2,0,H243_NAME_LEN);
	memset(m_in_szPassword,0,H243_NAME_LEN);
	m_in_bIsGuest  = FALSE;
	m_in_bIsLeader = FALSE;
		// output parameters
	memset(m_out_szName,0,H243_NAME_LEN);
	m_out_bIsLeader = FALSE;
	m_out_bIsVip    = TRUE;
	for( int i=0; i<4; i++ )
		memset(m_out_aszUserInfo[i],0,H243_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////
CCommSetExternalDbAdd::CCommSetExternalDbAdd(const CCommSetExternalDbAdd& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetExternalDbAdd::~CCommSetExternalDbAdd()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetExternalDbAdd& CCommSetExternalDbAdd::operator= (const CCommSetExternalDbAdd& other)
{
	if( this == &other )
		return *this;

		// input parameters
	strncpy(m_in_szNumericId,other.m_in_szNumericId,sizeof(m_in_szNumericId) - 1);
	m_in_szNumericId[sizeof(m_in_szNumericId) - 1 ] = '\0';
	strncpy(m_in_szPhone1,other.m_in_szPhone1,H243_NAME_LEN);
	strncpy(m_in_szPhone2,other.m_in_szPhone2,H243_NAME_LEN);
	strncpy(m_in_szPassword,other.m_in_szPassword,H243_NAME_LEN);

	m_in_bIsGuest  = other.m_in_bIsGuest;
	m_in_bIsLeader = other.m_in_bIsLeader;

		// output parameters
	strncpy(m_out_szName,other.m_out_szName,H243_NAME_LEN);
	m_out_bIsLeader = other.m_out_bIsLeader;
	m_out_bIsVip    = other.m_out_bIsVip;
	for( int i=0; i<4; i++ )
		strncpy(m_out_aszUserInfo[i],other.m_out_aszUserInfo[i],H243_NAME_LEN);

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbAdd::SetName(const char* pszNewName)
{
	strncpy(m_out_szName,pszNewName,sizeof(m_out_szName) - 1);
	m_out_szName[sizeof(m_out_szName) - 1] = '\0';
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbAdd::SetLeader(const BOOL bIsLeader)
{
	m_out_bIsLeader = bIsLeader;
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbAdd::SetVip(const BOOL bIsVip)
{
	m_out_bIsVip = bIsVip;
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbAdd::SetUserInfo(const int ind,const char* pszInfo)
{
	if( ind >= 0 && ind < 4 )
		strncpy(m_out_aszUserInfo[ind],pszInfo,H243_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////
void CCommSetExternalDbAdd::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("LEADER",m_out_bIsLeader,_BOOL);
	pFatherNode->AddChildNode("NAME",m_out_szName);
	pFatherNode->AddChildNode("VIP",m_out_bIsVip,_BOOL);

	// create <CONTACT_INFO_LIST> section
	CXMLDOMElement* pContactListNode = pFatherNode->AddChildNode("CONTACT_INFO_LIST");
	if( NULL == pContactListNode )
		return;
	for( int i=0; i<4; i++ )
		pContactListNode->AddChildNode("CONTACT_INFO",m_out_aszUserInfo[i]);
}

//////////////////////////////////////////////////////////////////////
int CCommSetExternalDbAdd::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NUMERIC_ID",m_in_szNumericId,_0_TO_H243_NAME_LENGTH);
//	if( STATUS_OK != nStatus )
//		return nStatus; // must field
	GET_VALIDATE_CHILD(pActionNode,"PASSWORD",m_in_szPassword,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"GUEST",&m_in_bIsGuest,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"LEADER",&m_in_bIsLeader,_BOOL);

	// get <ACTUAL_PARTY_PHONES> section
	CXMLDOMElement*		pChildNode = NULL;
	GET_CHILD_NODE(pActionNode,"ACTUAL_PARTY_PHONES",pChildNode);

	if( pChildNode )
	{
		GET_VALIDATE_CHILD(pChildNode,"PHONE1",m_in_szPhone1,_0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pChildNode,"PHONE2",m_in_szPhone2,_0_TO_H243_NAME_LENGTH);
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
//		USE IN 'REQUEST_USER_DETAILS' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetExternalDbUser::CCommSetExternalDbUser()
{
		// input parameters
	memset(m_in_szUserName,0,H243_NAME_LEN);
	memset(m_in_szPassword,0,H243_NAME_LEN);
	memset(m_in_szStationName,0,MAX_AUDIT_WORKSTATION_NAME_LEN);
	memset(m_in_szStationIp,0,H243_NAME_LEN);
		// output parameters
	m_out_group = GUEST;
}

//////////////////////////////////////////////////////////////////////
CCommSetExternalDbUser::CCommSetExternalDbUser(const CCommSetExternalDbUser& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetExternalDbUser::~CCommSetExternalDbUser()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetExternalDbUser& CCommSetExternalDbUser::operator= (const CCommSetExternalDbUser& other)
{
	if( this == &other )
		return *this;

		// input parameters
	strncpy(m_in_szUserName,other.m_in_szUserName,H243_NAME_LEN);
	strncpy(m_in_szPassword,other.m_in_szPassword,H243_NAME_LEN);
	strncpy(m_in_szStationName,other.m_in_szStationName,MAX_AUDIT_WORKSTATION_NAME_LEN);
	strncpy(m_in_szStationIp,other.m_in_szStationIp,H243_NAME_LEN);

		// output parameters
	m_out_group  = other.m_out_group;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CCommSetExternalDbUser::SetGroup(const WORD group)
{
	m_out_group  = group;
}

/////////////////////////////////////////////////////////////////////////////
void CCommSetExternalDbUser::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("AUTHORIZATION_GROUP",m_out_group,AUTHORIZATION_GROUP_ENUM);
}

/////////////////////////////////////////////////////////////////////////////
int CCommSetExternalDbUser::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"USER_NAME",m_in_szUserName,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PASSWORD",m_in_szPassword,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"STATION_NAME",m_in_szStationName,_1_TO_STATION_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"STATION_IP",m_in_szStationIp,_0_TO_H243_NAME_LENGTH);
//	if( STATUS_OK != nStatus )
//		return nStatus; // must field

	return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//		USE IN 'SIP_ADD_SUBSCRIPTION' TRANSACTION
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
CCommSetAddSubscription::CCommSetAddSubscription()
{
		// input parameters
	memset(m_szConfName,0,H243_NAME_LEN);
	memset(m_szSubscriber,0,H243_NAME_LEN);
	memset(m_szEvent,0,H243_NAME_LEN);
	m_wExpires = 3600;
		// output parameters
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CCommSetAddSubscription::CCommSetAddSubscription(const CCommSetAddSubscription& other) : CSerializeObject(other)
{
	*this = other;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CCommSetAddSubscription::~CCommSetAddSubscription()
{
	memset(m_szConfName,0,H243_NAME_LEN);
	memset(m_szSubscriber,0,H243_NAME_LEN);
	memset(m_szEvent,0,H243_NAME_LEN);
	m_wExpires = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CCommSetAddSubscription& CCommSetAddSubscription::operator= (const CCommSetAddSubscription& other)
{
	if( this == &other )
		return *this;

		// input parameters
	strncpy(m_szConfName,other.m_szConfName,H243_NAME_LEN);
	strncpy(m_szSubscriber,other.m_szSubscriber,H243_NAME_LEN);
	strncpy(m_szEvent,other.m_szEvent,H243_NAME_LEN);
	m_wExpires = other.m_wExpires;

		// output parameters

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CCommSetAddSubscription::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int CCommSetAddSubscription::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"CONF_NAME",m_szConfName,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"SUBSCRIBER",m_szSubscriber,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"EVENT",m_szEvent,_0_TO_H243_NAME_LENGTH);

	GET_VALIDATE_CHILD(pActionNode,"EXPIRES",&m_wExpires,_0_TO_WORD);

	return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//		USE IN 'SIP_GET_NOTIFICATION' TRANSACTION
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
CCommSetGetNotification::CCommSetGetNotification()
{
		// input parameters
	memset(m_szConfName,0,H243_NAME_LEN);
	memset(m_szSubscriber,0,H243_NAME_LEN);
	memset(m_szEvent,0,H243_NAME_LEN);
		// output parameters
	m_pszSubsciptNotification = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CCommSetGetNotification::CCommSetGetNotification(const CCommSetGetNotification& other) : CSerializeObject(other)
{
	m_pszSubsciptNotification = NULL;
	*this = other;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CCommSetGetNotification::~CCommSetGetNotification()
{
	memset(m_szConfName,0,H243_NAME_LEN);
	memset(m_szSubscriber,0,H243_NAME_LEN);
	memset(m_szEvent,0,H243_NAME_LEN);
	PDELETEA(m_pszSubsciptNotification);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CCommSetGetNotification& CCommSetGetNotification::operator= (const CCommSetGetNotification& other)
{
	if( this == &other )
		return *this;

		// input parameters
	strncpy(m_szConfName,other.m_szConfName,H243_NAME_LEN);
	strncpy(m_szSubscriber,other.m_szSubscriber,H243_NAME_LEN);
	strncpy(m_szEvent,other.m_szEvent,H243_NAME_LEN);

		// output parameters
	SetSubscriptionNotification(other.m_pszSubsciptNotification);

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CCommSetGetNotification::SetSubscriptionNotification(const char* pszNotif)
{
	PDELETEA(m_pszSubsciptNotification);
	if( NULL != pszNotif )
	{
		DWORD len = strlen(pszNotif);
		m_pszSubsciptNotification = new char[len+1];
		strncpy(m_pszSubsciptNotification,pszNotif,len);
		m_pszSubsciptNotification[len] = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CCommSetGetNotification::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	if( NULL != m_pszSubsciptNotification )
		pFatherNode->AddChildNode("SUBSCR_NOTIFICATION",m_pszSubsciptNotification);
	else
		pFatherNode->AddChildNode("SUBSCR_NOTIFICATION","");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int CCommSetGetNotification::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"CONF_NAME",m_szConfName,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"SUBSCRIBER",m_szSubscriber,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"EVENT",m_szEvent,_0_TO_H243_NAME_LENGTH);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
//		USE IN 'LPR_MODE_CHANGE_REQUEST' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetLprModeChangeReq::CCommSetLprModeChangeReq()
{
	m_lossProtection = 0;
	m_mtbf = 0;
	m_congestionCeiling = 0;
	m_fill = 0;
	m_modeTimeout = 0;
}

//////////////////////////////////////////////////////////////////////
CCommSetLprModeChangeReq::CCommSetLprModeChangeReq(const CCommSetLprModeChangeReq& other) : CCommSetPartyCommon(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetLprModeChangeReq::~CCommSetLprModeChangeReq()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetLprModeChangeReq& CCommSetLprModeChangeReq::operator= (const CCommSetLprModeChangeReq& other)
{
	if( this == &other )
		return *this;

	*((CCommSetPartyCommon*)this) = *((CCommSetPartyCommon*)&other);

	m_lossProtection = other.m_lossProtection;
	m_mtbf = other.m_mtbf;
	m_congestionCeiling = other.m_congestionCeiling;
	m_fill = other.m_fill;
	m_modeTimeout = other.m_modeTimeout;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetLprModeChangeReq::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetLprModeChangeReq::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = CCommSetPartyCommon::DeSerializeXml(pActionNode,pszError,action);

	if( STATUS_OK != nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"LOSS_PROTECTION",&m_lossProtection,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"MTBF",&m_mtbf,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"CONGESTION_CEILING",&m_congestionCeiling,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"FILL",&m_fill,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"MODE_TIMEOUT",&m_modeTimeout,_0_TO_DWORD);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
//		USE IN 'H323 PARTY SENDS FECC' TRANSACTION
//		USE IN 'SIP PARTY SENDS FECC' TRANSACTION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCommSetPartyFecc::CCommSetPartyFecc()
{
	memset(m_szFeccString,0,H243_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyFecc::CCommSetPartyFecc(const CCommSetPartyFecc& other) : CCommSetPartyCommon(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetPartyFecc::~CCommSetPartyFecc()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetPartyFecc& CCommSetPartyFecc::operator= (const CCommSetPartyFecc& other)
{
	if( this == &other )
		return *this;

	*((CCommSetPartyCommon*)this) = *((CCommSetPartyCommon*)&other);

	memcpy(m_szFeccString,other.m_szFeccString,H243_NAME_LEN);

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetPartyFecc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetPartyFecc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = CCommSetPartyCommon::DeSerializeXml(pActionNode,pszError,action);

	if( STATUS_OK == nStatus ) {
		GET_VALIDATE_CHILD(pActionNode,"FECC_KEY",m_szFeccString,_1_TO_H243_NAME_LENGTH);
	}

	return nStatus;
}


////////////////////////////////////////////////
//		USE IN 'DEL CAP SET' TRANSACTION
////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
CCommSetDelCapset::CCommSetDelCapset()
{
	memset(m_szCapsetName,0,H243_NAME_LEN);
	strncpy(m_szCapsetName,"JUNK_NAME",H243_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////
CCommSetDelCapset::CCommSetDelCapset(const CCommSetDelCapset& other) : CSerializeObject(other)
{
//	memset(m_szPartyName,0,H243_NAME_LEN);
//	strncpy(m_szPartyName,other.m_szPartyName,H243_NAME_LEN);
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetDelCapset::~CCommSetDelCapset()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetDelCapset& CCommSetDelCapset::operator= (const CCommSetDelCapset& other)
{
	if( this == &other )
		return *this;

	memset(m_szCapsetName,0,H243_NAME_LEN);
	strncpy(m_szCapsetName,other.m_szCapsetName,H243_NAME_LEN);

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetDelCapset::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetDelCapset::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_szCapsetName,_1_TO_H243_NAME_LENGTH);

	return nStatus;
}

////////////////////////////////////////////////
//		USE IN 'SCP_STREAMS_REQUEST' TRANSACTION
////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
CCommSetScpStreamsRequest::CCommSetScpStreamsRequest()
{
	memset(m_szConfName,0,H243_NAME_LEN);
	memset(m_szPartyName, 0, H243_NAME_LEN);
	m_numberOfStreams = 0;
	memset(&m_streams, 0, sizeof(m_streams));
}

//////////////////////////////////////////////////////////////////////
CCommSetScpStreamsRequest::CCommSetScpStreamsRequest(const CCommSetScpStreamsRequest& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetScpStreamsRequest::~CCommSetScpStreamsRequest()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetScpStreamsRequest& CCommSetScpStreamsRequest::operator= (const CCommSetScpStreamsRequest& other)
{
	if( this == &other )
		return *this;

	memset(m_szConfName, 0, H243_NAME_LEN);
	strncpy(m_szConfName, other.m_szConfName, H243_NAME_LEN);
	memset(m_szPartyName, 0, H243_NAME_LEN);
	strncpy(m_szPartyName, other.m_szPartyName, H243_NAME_LEN);
	this->m_numberOfStreams = other.m_numberOfStreams;
	memcpy(&this->m_streams, &other.m_streams, sizeof(m_streams));

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetScpStreamsRequest::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetScpStreamsRequest::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"CONF_NAME",m_szConfName,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"PARTY_NAME",m_szPartyName,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"NUMBER_OF_STREAMS",&m_numberOfStreams,_0_TO_WORD);

	CXMLDOMElement *pStreamDescNode;
	GET_FIRST_CHILD_NODE(pActionNode, "STREAM_DESCRIPTION", pStreamDescNode);

	int i = 0;
	memset(&this->m_streams[0], 0, sizeof(m_streams));

	TRACEINTO << "CCommSetScpStreamsRequest::DeSerializeXml - conf name=" << m_szConfName
			<< "  party name=" << m_szPartyName
			<< "  request streams number = " <<  m_numberOfStreams;

	while (pStreamDescNode)  //no need to check ranges (done with AddCell)
	{
        DWORD tmp = 0;
		GET_VALIDATE_MANDATORY_CHILD(pStreamDescNode,"BIT_RATE",&tmp,_0_TO_DWORD);
		m_streams[i].unBitRate = tmp;
		m_streams[i].unChannelType = 2;
		GET_VALIDATE_MANDATORY_CHILD(pStreamDescNode,"FRAME_RATE",&tmp,_0_TO_DWORD);
		m_streams[i].unFrameRate = tmp;

		if(75 == tmp){
			m_streams[i].unFrameRate = 0;
 		}

		if(150 == tmp){
			m_streams[i].unFrameRate = 1;
 		}

		if(300 == tmp){
			m_streams[i].unFrameRate = 2;
 		}

		GET_VALIDATE_MANDATORY_CHILD(pStreamDescNode,"HEIGHT",&tmp,_0_TO_DWORD);
		m_streams[i].unHeight = tmp;
		m_streams[i].unPayloadType = CCapSet::GetPayloadType(eSvcCapCode);
		m_streams[i].unPipeIdSsrc = 210501 + i;
		m_streams[i].unPriority = 2;
		GET_VALIDATE_MANDATORY_CHILD(pStreamDescNode,"SRC_ID",&tmp,_0_TO_DWORD);
		m_streams[i].unSourceIdSsrc = tmp;
		if(m_streams[i].unSourceIdSsrc){
			m_streams[i].unSpecificSourceSsrc = 1;
		}
		GET_VALIDATE_MANDATORY_CHILD(pStreamDescNode,"WIDTH",&tmp,_0_TO_DWORD);
		m_streams[i].unWidth = tmp;

		// dump
		TRACEINTO << " bitrate = " << m_streams[i].unBitRate
				  << " framerate = " << m_streams[i].unFrameRate
				  << " height = " << m_streams[i].unHeight
				  << " ssrc = " << m_streams[i].unSourceIdSsrc
				  << " width = " << m_streams[i].unWidth;

		GET_NEXT_CHILD_NODE(pActionNode, "STREAM_DESCRIPTION", pStreamDescNode);
		++i;
	}

	return nStatus;
}
















