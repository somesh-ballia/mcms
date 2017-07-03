
#include "CommMediaMngrSet.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "DummyEntry.h"
#include "ApiStatuses.h"


//CCommSetEPMediaParam
////////////////////////////

CCommSetEPMediaParam::CCommSetEPMediaParam()
{
	m_monitorPartyId = 0;
	m_monitorConfId = 0;
	
	memset(m_szVideoFileName, 0, MEDIA_FILE_NAME_LEN);
	memset(m_szAudioFileName, 0, MEDIA_FILE_NAME_LEN);
	memset(m_szContentFileName, 0, MEDIA_FILE_NAME_LEN);
	
	memset(m_szSaveVideoFileName, 0, MEDIA_FILE_NAME_LEN);
	memset(m_szSaveAudioFileName, 0, MEDIA_FILE_NAME_LEN);
	memset(m_szSaveContentFileName, 0, MEDIA_FILE_NAME_LEN);
}

/////////////////////////////////////////////////////////////////////////////

CCommSetEPMediaParam::CCommSetEPMediaParam(const CCommSetEPMediaParam& other) : CSerializeObject(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////

CCommSetEPMediaParam::~CCommSetEPMediaParam()
{
}

/////////////////////////////////////////////////////////////////////////////

CCommSetEPMediaParam& CCommSetEPMediaParam::operator= (const CCommSetEPMediaParam& other)
{
	if( this == &other )
		return *this;
	
	m_monitorPartyId = other.m_monitorPartyId;
	m_monitorConfId = other.m_monitorConfId;
	
	memcpy(m_szVideoFileName, other.m_szVideoFileName, MEDIA_FILE_NAME_LEN);
	memcpy(m_szAudioFileName, other.m_szAudioFileName, MEDIA_FILE_NAME_LEN);
	memcpy(m_szContentFileName, other.m_szContentFileName, MEDIA_FILE_NAME_LEN);
	
	memcpy(m_szSaveVideoFileName, other.m_szSaveVideoFileName, MEDIA_FILE_NAME_LEN);
	memcpy(m_szSaveAudioFileName, other.m_szSaveAudioFileName, MEDIA_FILE_NAME_LEN);
	memcpy(m_szSaveContentFileName, other.m_szSaveContentFileName, MEDIA_FILE_NAME_LEN);
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////


void CCommSetEPMediaParam::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetEPMediaParam::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;
	
	CXMLDOMElement *pTraceNode;
	GET_MANDATORY_CHILD_NODE(pActionNode, "EP_MEDIA_PARAM", pTraceNode);
	
	GET_VALIDATE_CHILD(pTraceNode, "MONITOR_PARTY_ID", &m_monitorPartyId, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pTraceNode, "MONITOR_CONF_ID", &m_monitorConfId, _0_TO_DWORD);

	
	GET_VALIDATE_CHILD(pTraceNode,"VIDEO_FILE_NAME", m_szVideoFileName, _0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pTraceNode,"AUDIO_FILE_NAME", m_szAudioFileName, _0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pTraceNode,"CONTENT_FILE_NAME", m_szContentFileName, _0_TO_H243_NAME_LENGTH);
	
	GET_VALIDATE_CHILD(pTraceNode,"SAVE_VIDEO_FILE_NAME", m_szSaveVideoFileName, _0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pTraceNode,"SAVE_AUDIO_FILE_NAME", m_szSaveAudioFileName, _0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pTraceNode,"SAVE_CONTENT_FILE_NAME", m_szSaveContentFileName, _0_TO_H243_NAME_LENGTH);	
		
	return nStatus;
}





//CCommResetChannelOut
////////////////////////////

CCommResetChannelOut::CCommResetChannelOut()
{
	memset(m_szEndPointList, 0, ENDPOINT_LIST_LEN);
	memset(m_szChannelType, 0, CHANNEL_TYPE_LEN);
}

/////////////////////////////////////////////////////////////////////////////

CCommResetChannelOut::CCommResetChannelOut(const CCommResetChannelOut& other) : CSerializeObject(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////

CCommResetChannelOut::~CCommResetChannelOut()
{
}

/////////////////////////////////////////////////////////////////////////////

CCommResetChannelOut& CCommResetChannelOut::operator= (const CCommResetChannelOut& other)
{
	if( this == &other )
		return *this;
	
	memcpy(m_szEndPointList, other.m_szEndPointList, ENDPOINT_LIST_LEN);
	memcpy(m_szChannelType, other.m_szChannelType, CHANNEL_TYPE_LEN);
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////


void CCommResetChannelOut::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////

int CCommResetChannelOut::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode, "ENDPOINT_LIST", m_szEndPointList, _0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode, "CHANNEL_TYPE", m_szChannelType, _0_TO_H243_NAME_LENGTH);

	return STATUS_OK;
}




//CCommMediaLibrary
////////////////////////////

CCommMediaLibrary::CCommMediaLibrary()
{
	m_audioLibrary = NULL;
	m_videoLibrary = NULL;
	m_contentLibrary = NULL;
}

/////////////////////////////////////////////////////////////////////////////

CCommMediaLibrary::CCommMediaLibrary(const CCommMediaLibrary& other) : CSerializeObject(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////

CCommMediaLibrary::~CCommMediaLibrary()
{
	POBJDELETE(m_audioLibrary);
	POBJDELETE(m_videoLibrary);
	POBJDELETE(m_contentLibrary);
}

/////////////////////////////////////////////////////////////////////////////

CCommMediaLibrary& CCommMediaLibrary::operator= (const CCommMediaLibrary& other)
{
	if( this == &other )
		return *this;
	
	m_audioLibrary = other.m_audioLibrary;
	m_videoLibrary = other.m_videoLibrary;
	m_contentLibrary = other.m_contentLibrary;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////


void CCommMediaLibrary::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// create <MEDIA_LIBRARY> section
	CXMLDOMElement* pMediaLibraryNode = pFatherNode->AddChildNode("MEDIA_LIBRARY");
	if( CPObject::IsValidPObjectPtr(m_audioLibrary) )
	{
		m_audioLibrary->SerializeXml(pMediaLibraryNode);
	}
	if( CPObject::IsValidPObjectPtr(m_videoLibrary) )
	{
		m_videoLibrary->SerializeXml(pMediaLibraryNode);
	}	
	if( CPObject::IsValidPObjectPtr(m_contentLibrary) )
	{
		m_contentLibrary->SerializeXml(pMediaLibraryNode);
	}
}

//////////////////////////////////////////////////////////////////////

int CCommMediaLibrary::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;
	
	// get <MEDIA_LIBRARY> section
	CXMLDOMElement* pMediaLibraryListNode = NULL;
	GET_CHILD_NODE(pActionNode, "MEDIA_LIBRARY", pMediaLibraryListNode);

	// get fields from <MEDIA_LIBRARY> section
	if( pMediaLibraryListNode )
	{
		CMediaLibrary* pMediaLibrary = NULL;
		CXMLDOMElement* pMediaLibraryNode = NULL;
		
		nStatus = pMediaLibraryListNode->firstChildNode(&pMediaLibraryNode, "MEDIA_AUDIO");
		if( pMediaLibraryNode && nStatus == STATUS_OK )
		{
			pMediaLibrary = new CMediaLibrary();

			// if type of media library is valid
			if( CPObject::IsValidPObjectPtr(pMediaLibrary) )
			{
				// deserialize details of media library
				nStatus = pMediaLibrary->DeSerializeXml(pMediaLibraryNode, pszError, action);
				if(nStatus == STATUS_OK)
				{
					m_audioLibrary = pMediaLibrary;
					m_audioLibrary->SetMediaType(audioChannelMediaType); 
				}
			}
		}
		
		
		nStatus = pMediaLibraryListNode->nextChildNode(&pMediaLibraryNode, "MEDIA_VIDEO");
		if( pMediaLibraryNode && nStatus == STATUS_OK )
		{
			pMediaLibrary = new CMediaLibrary();			

			// if type of media library is valid
			if( CPObject::IsValidPObjectPtr(pMediaLibrary) )
			{
				// deserialize details of media library
				nStatus = pMediaLibrary->DeSerializeXml(pMediaLibraryNode, pszError, action);
				if(nStatus == STATUS_OK)
				{
					m_videoLibrary = pMediaLibrary;
					m_videoLibrary->SetMediaType(videoChannelMediaType); 
				}
			}
		}
		
		
		nStatus = pMediaLibraryListNode->nextChildNode(&pMediaLibraryNode, "MEDIA_CONTENT");
		if( pMediaLibraryNode && nStatus == STATUS_OK )
		{
			pMediaLibrary = new CMediaLibrary();			

			// if type of media library is valid
			if( CPObject::IsValidPObjectPtr(pMediaLibrary) )
			{
				// deserialize details of media library
				nStatus = pMediaLibrary->DeSerializeXml(pMediaLibraryNode, pszError, action);
				if(nStatus == STATUS_OK)
				{
					m_contentLibrary = pMediaLibrary;
					m_contentLibrary->SetMediaType(t120ChannelMediaType);
				}
			}
		}
	}
	


	return STATUS_OK;
}

