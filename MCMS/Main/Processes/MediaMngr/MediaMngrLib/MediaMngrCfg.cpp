// MediaMngrCfg.cpp: configuration of system.
//
//////////////////////////////////////////////////////////////////////

#include "MediaMngrCfg.h"
#include "MediaMngr.h"
#include "StatusesGeneral.h"

#include "psosxml.h"
#include "XmlDefines.h"
#include "XmlApi.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "TraceStream.h"



/////////////////////////////////////////////////////////////////////////////
CMediaMngrCfg::CMediaMngrCfg()
{
}

/////////////////////////////////////////////////////////////////////////////

CMediaMngrCfg::~CMediaMngrCfg()
{
	::SetMediaMngrCfg(NULL);
	
	POBJDELETE(m_audioLibrary);
	POBJDELETE(m_videoLibrary);
	POBJDELETE(m_contentLibrary);
	
	for( WORD i=0; i<m_wMaxNicSlots; i++ )
		POBJDELETE(m_ppNicArr[i]);
	PDELETEA(m_ppNicArr);
}


/////////////////////////////////////////////////////////////////////////////

CMediaMngrCfg::CMediaMngrCfg(const CMediaMngrCfg& rOther)
	: CSerializeObject(rOther)
{
	*this = rOther;
}

/////////////////////////////////////////////////////////////////////////////

CMediaMngrCfg& CMediaMngrCfg::operator =(const CMediaMngrCfg& reference)
{
	if(this == &reference)
		return *this;
	
	//clean old media library
	POBJDELETE(m_audioLibrary);
	POBJDELETE(m_videoLibrary);
	POBJDELETE(m_contentLibrary);
	
	
	//  clean old nics 
	for( WORD i=0; i<m_wMaxNicSlots; i++ )
		POBJDELETE(m_ppNicArr[i]);
	
	PDELETEA(m_ppNicArr);
	
	m_sPrimaryAudioFileReadPath = reference.m_sPrimaryAudioFileReadPath;
	m_sPrimaryVideoFileReadPath = reference.m_sPrimaryVideoFileReadPath;
	m_sPrimaryContentFileReadPath = reference.m_sPrimaryContentFileReadPath;
	m_sPrimaryDtmfFileReadPath = reference.m_sPrimaryDtmfFileReadPath;
	
	m_sInstallationAudioFileReadPath = reference.m_sInstallationAudioFileReadPath;
	m_sInstallationVideoFileReadPath = reference.m_sInstallationVideoFileReadPath;
	m_sInstallationContentFileReadPath = reference.m_sInstallationContentFileReadPath;
	m_sInstallationDtmfFileReadPath = reference.m_sInstallationDtmfFileReadPath;
	
	m_sMediaFileWritePath = reference.m_sMediaFileWritePath;
	
	m_nMaxNumberSpeaker      = reference.m_nMaxNumberSpeaker;
	m_nSwitchAudioInterval   = reference.m_nSwitchAudioInterval;
	m_nSwitchAudio           = reference.m_nSwitchAudio;
	
	m_intraDelayTimeResponse = reference.m_intraDelayTimeResponse;
	
	
		
	m_audioLibrary = reference.m_audioLibrary->CreateCopy();
	m_videoLibrary = reference.m_videoLibrary->CreateCopy();
	m_contentLibrary = reference.m_contentLibrary->CreateCopy();
		
	
	m_wMaxNicSlots = reference.m_wMaxNicSlots;

	m_ppNicArr = new CNicCfg*[m_wMaxNicSlots];
	for( WORD k=0; k<m_wMaxNicSlots; k++ )
	{
		if( reference.m_ppNicArr[k] )
			m_ppNicArr[k] = reference.m_ppNicArr[k]->CreateCopy();
	}

	return *this;
}


/////////////////////////////////////////////////////////////////////////////

void CMediaMngrCfg::Init()
{
	//init class members
	m_sPrimaryAudioFileReadPath = "";
	m_sPrimaryVideoFileReadPath = "";
	m_sPrimaryContentFileReadPath = "";
	m_sPrimaryDtmfFileReadPath = "";
	
	m_sInstallationAudioFileReadPath = "";
	m_sInstallationVideoFileReadPath = "";
	m_sInstallationContentFileReadPath = "";
	m_sInstallationDtmfFileReadPath = "";
	
	
	m_sMediaFileWritePath = "";

	m_nMaxNumberSpeaker = 2;
	m_nSwitchAudioInterval = 3000;	
	m_nSwitchAudio = 0;

	
	m_intraDelayTimeResponse = 150;
	
	m_audioLibrary = NULL;
	m_videoLibrary = NULL;
	m_contentLibrary = NULL;

	m_wMaxNicSlots = MAX_NUM_OF_NIC_SLOTS;
	m_ppNicArr = new CNicCfg* [m_wMaxNicSlots];
	for( WORD i=0; i<m_wMaxNicSlots; i++ )
		m_ppNicArr[i] = NULL;
		
	//read XML file
	int status = ReadXmlFile();
	
	TRACEINTO << " CMediaMngrCfg::CMediaMngrCfg() - ReadXmlFile status = " << status;
	
	if( status != STATUS_OK )
		WriteXmlFile();
	
	::SetMediaMngrCfg(this);
}

/////////////////////////////////////////////////////////////////////////////

void CMediaMngrCfg::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// create <MEDIA_DETAILS> section
	CXMLDOMElement* pMediaDetailsNode = pFatherNode->AddChildNode("MEDIA_DETAILS");

	// <MEDIA_DETAILS> fields
	pMediaDetailsNode->AddChildNode("PRIMARY_AUDIO_FILE_READ_PATH", m_sPrimaryAudioFileReadPath);
	pMediaDetailsNode->AddChildNode("PRIMARY_VIDEO_FILE_READ_PATH",m_sPrimaryVideoFileReadPath);
	pMediaDetailsNode->AddChildNode("PRIMARY_CONTENT_FILE_READ_PATH",m_sPrimaryContentFileReadPath);
	pMediaDetailsNode->AddChildNode("PRIMARY_DTMF_FILE_READ_PATH", m_sPrimaryDtmfFileReadPath);

	pMediaDetailsNode->AddChildNode("INSTALLATION_AUDIO_FILE_READ_PATH", m_sInstallationAudioFileReadPath);
	pMediaDetailsNode->AddChildNode("INSTALLATION_VIDEO_FILE_READ_PATH",m_sInstallationVideoFileReadPath);
	pMediaDetailsNode->AddChildNode("INSTALLATION_CONTENT_FILE_READ_PATH",m_sInstallationContentFileReadPath);
	pMediaDetailsNode->AddChildNode("INSTALLATION_DTMF_FILE_READ_PATH", m_sInstallationDtmfFileReadPath);
	
	pMediaDetailsNode->AddChildNode("MEDIA_FILE_WRITE_PATH", m_sMediaFileWritePath);

	pMediaDetailsNode->AddChildNode("MAX_NUMBER_SPEAKER", m_nMaxNumberSpeaker);
	pMediaDetailsNode->AddChildNode("SWITCH_AUDIO_INTERVAL", m_nSwitchAudioInterval);
	pMediaDetailsNode->AddChildNode("ENABLE_AUDIO_SWITCH", m_nSwitchAudio);
	
	// create <VIDEO_PARAM> section
	CXMLDOMElement* pVideoParamNode = pFatherNode->AddChildNode("VIDEO_PARAM");
	// <VIDEO_PARAM> fields
	pVideoParamNode->AddChildNode("INTRA_DELAY_TIME_RESPONSE_CENTISECOND", m_intraDelayTimeResponse);
	
	
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
	
	
	
	// create <NIC_LIST> section
	CXMLDOMElement* pNicListNode = pFatherNode->AddChildNode("NIC_LIST");
	for( WORD i=0; i<m_wMaxNicSlots; i++ )
	{
		// if NIC in slot 'i' exists - serialize it
		if( ( m_ppNicArr != NULL ) && CPObject::IsValidPObjectPtr(m_ppNicArr[i]) )
			m_ppNicArr[i]->SerializeXml(pNicListNode);
	}
}

/////////////////////////////////////////////////////////////////////////////

int CMediaMngrCfg::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	//  clean old nics info
	WORD i=0;
	for( i=0; i<m_wMaxNicSlots; i++ )
		POBJDELETE(m_ppNicArr[i]);
	
	int nStatus = STATUS_OK;
	char* pszChildName = NULL;

	// get <MEDIA_DETAILS> section
	CXMLDOMElement* pMediaDetailsNode = NULL;
	GET_CHILD_NODE(pActionNode ,"MEDIA_DETAILS", pMediaDetailsNode);
	
	// get fields from <MEDIA_DETAILS> section
	if( pMediaDetailsNode )
	{
		GET_VALIDATE_CHILD(pMediaDetailsNode, "PRIMARY_AUDIO_FILE_READ_PATH", m_sPrimaryAudioFileReadPath, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pMediaDetailsNode, "PRIMARY_VIDEO_FILE_READ_PATH", m_sPrimaryVideoFileReadPath, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pMediaDetailsNode, "PRIMARY_CONTENT_FILE_READ_PATH", m_sPrimaryContentFileReadPath, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pMediaDetailsNode, "PRIMARY_DTMF_FILE_READ_PATH", m_sPrimaryDtmfFileReadPath , ONE_LINE_BUFFER_LENGTH);
		
		GET_VALIDATE_CHILD(pMediaDetailsNode, "INSTALLATION_AUDIO_FILE_READ_PATH", m_sInstallationAudioFileReadPath, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pMediaDetailsNode, "INSTALLATION_VIDEO_FILE_READ_PATH", m_sInstallationVideoFileReadPath, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pMediaDetailsNode, "INSTALLATION_CONTENT_FILE_READ_PATH", m_sInstallationContentFileReadPath, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pMediaDetailsNode, "INSTALLATION_DTMF_FILE_READ_PATH", m_sInstallationDtmfFileReadPath , ONE_LINE_BUFFER_LENGTH);
		
		GET_VALIDATE_CHILD(pMediaDetailsNode, "MEDIA_FILE_WRITE_PATH", m_sMediaFileWritePath, ONE_LINE_BUFFER_LENGTH);

		GET_VALIDATE_CHILD(pMediaDetailsNode, "MAX_NUMBER_SPEAKER", &m_nMaxNumberSpeaker, _0_TO_WORD);
		GET_VALIDATE_CHILD(pMediaDetailsNode, "SWITCH_AUDIO_INTERVAL", &m_nSwitchAudioInterval, _0_TO_WORD);
		GET_VALIDATE_CHILD(pMediaDetailsNode, "ENABLE_AUDIO_SWITCH", &m_nSwitchAudio, _0_TO_WORD);
	}
	
	
	// get <VIDEO_PARAM> section
	CXMLDOMElement* pVideoParamNode = NULL;
	GET_CHILD_NODE(pActionNode, "VIDEO_PARAM", pVideoParamNode);
	
	// get fields from <VIDEO_PARAM> section
	if( pVideoParamNode )
	{
		GET_VALIDATE_CHILD(pVideoParamNode, "INTRA_DELAY_TIME_RESPONSE_CENTISECOND", &m_intraDelayTimeResponse, _0_TO_WORD);
	}
	
	
	// get <MEDIA_LIBRARY> section
	CXMLDOMElement* pMediaLibraryListNode = NULL;
	GET_CHILD_NODE(pActionNode, "MEDIA_LIBRARY", pMediaLibraryListNode);
	
	m_audioLibrary = NULL;
	m_videoLibrary = NULL;
	m_contentLibrary = NULL;
	
	
	// get fields from <MEDIA_LIBRARY> section
	if( pMediaLibraryListNode )
	{
		CMediaLibrary* pMediaLibrary = NULL;
		CXMLDOMElement* pMediaLibraryNode = NULL;
		
		nStatus = pMediaLibraryListNode->firstChildNode(&pMediaLibraryNode, "MEDIA_AUDIO");
		if( pMediaLibraryNode && nStatus == STATUS_OK )
		{
			pMediaLibrary = new CMediaLibrary(audioChannelMediaType);

			// if type of media library is valid
			if( CPObject::IsValidPObjectPtr(pMediaLibrary) )
			{
				// deserialize details of media library
				nStatus = pMediaLibrary->DeSerializeXml(pMediaLibraryNode, pszError, action);
				if(nStatus == STATUS_OK)
				{
					m_audioLibrary = pMediaLibrary->CreateCopy();
					//m_audioLibrary->SetMediaType(audioChannelMediaType);
				}
			}
			POBJDELETE(pMediaLibrary);
		}
		
		
		nStatus = pMediaLibraryListNode->nextChildNode(&pMediaLibraryNode, "MEDIA_VIDEO");
		if( pMediaLibraryNode && nStatus == STATUS_OK )
		{
			pMediaLibrary = new CMediaLibrary(videoChannelMediaType);			

			// if type of media library is valid
			if( CPObject::IsValidPObjectPtr(pMediaLibrary) )
			{
				// deserialize details of media library
				nStatus = pMediaLibrary->DeSerializeXml(pMediaLibraryNode, pszError, action);
				if(nStatus == STATUS_OK)
				{
					m_videoLibrary = pMediaLibrary;
					//m_videoLibrary->SetMediaType(videoChannelMediaType); 
				}
			}
		}
		
		
		nStatus = pMediaLibraryListNode->nextChildNode(&pMediaLibraryNode, "MEDIA_CONTENT");
		if( pMediaLibraryNode && nStatus == STATUS_OK )
		{
			pMediaLibrary = new CMediaLibrary(t120ChannelMediaType);			

			// if type of media library is valid
			if( CPObject::IsValidPObjectPtr(pMediaLibrary) )
			{
				// deserialize details of media library
				nStatus = pMediaLibrary->DeSerializeXml(pMediaLibraryNode, pszError, action);
				if(nStatus == STATUS_OK)
				{
					m_contentLibrary = pMediaLibrary;
					//m_contentLibrary->SetMediaType(t120ChannelMediaType);
				}
			}
		}
	}
	
	
	
		
	if( !m_wMaxNicSlots )
		m_wMaxNicSlots = 1;
	
	PDELETEA(m_ppNicArr);
	m_ppNicArr = new CNicCfg*[m_wMaxNicSlots];
	
	for( i=0; i<m_wMaxNicSlots; i++ )
		m_ppNicArr[i] = NULL;
	
	// get <NIC_LIST> section
	CXMLDOMElement* pNicListNode = NULL;
	GET_CHILD_NODE(pActionNode, "NIC_LIST", pNicListNode);
	
	// get cards from <NIC_LIST> section
	if( pNicListNode )
	{
		WORD nicId = 0;
		CNicCfg* pNicCfg = NULL;
		CXMLDOMElement* pNicNode = NULL;
		
		nStatus = pNicListNode->firstChildNode(&pNicNode);

		while( pNicNode && nStatus == STATUS_OK )
		{
			pNicCfg = new CNicCfg;
			//pNicNode->get_nodeName(&pszChildName);

			// if type of card is valid
			if( CPObject::IsValidPObjectPtr(pNicCfg) )
			{
				// deserialize details of card
				nStatus = pNicCfg->DeSerializeXml(pNicNode, pszError, action);
				if( nStatus )
					break;

				nicId = pNicCfg->GetNicId();
				if( nicId < m_wMaxNicSlots )
					m_ppNicArr[nicId] = pNicCfg;
				else 
					POBJDELETE(pNicCfg);
			}
			
			pNicListNode->nextChildNode(&pNicNode);
		}
	}
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

CNicCfg*  CMediaMngrCfg::GetNicCfg( const WORD index )
{
	if( index < m_wMaxNicSlots )
		return m_ppNicArr[index];
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CMediaMngrCfg::SetNicCfg( const WORD index, CNicCfg* nicCfg)
{
	if( index < m_wMaxNicSlots )
		m_ppNicArr[index] = nicCfg;
}

/////////////////////////////////////////////////////////////////////////////

void CMediaMngrCfg::ClearNicCfg( const WORD index )
{
	if( index < m_wMaxNicSlots )
		POBJDELETE(m_ppNicArr[index]);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


void CMediaMngrCfg::SetAudioLibrary(CMediaLibrary* audioLibrary)
{
	//audioLibrary->SetMediaType(audioChannelMediaType);
	m_audioLibrary = audioLibrary;
}

/////////////////////////////////////////////////////////////////////////////

void CMediaMngrCfg::SetVideoLibrary(CMediaLibrary* videoLibrary)
{
	//videoLibrary->SetMediaType(videoChannelMediaType);
	m_videoLibrary = videoLibrary;
}

/////////////////////////////////////////////////////////////////////////////

void CMediaMngrCfg::SetContentLibrary(CMediaLibrary* contentLibrary)
{
	//contentLibrary->SetMediaType(t120ChannelMediaType);
	m_contentLibrary = contentLibrary;
}

/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMediaLibrary
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CMediaLibrary::CMediaLibrary()
{
	m_eMediaType = unknownChannelMediaType;
	m_wFilesNum = 0;
	m_wCurrentFileIndex = 0;
	
	for(int i=0; i<MAX_MEDIA_FILES_IN_LIBRARY; i++ )
		m_strMediaList[i] = "";
}

/////////////////////////////////////////////////////////////////////////////
CMediaLibrary::CMediaLibrary(channelMediaType mediaType)
{
	CMediaLibrary();
	m_eMediaType = mediaType;
}

/////////////////////////////////////////////////////////////////////////////
CMediaLibrary::~CMediaLibrary()
{
}

/////////////////////////////////////////////////////////////////////////////
CMediaLibrary::CMediaLibrary(const CMediaLibrary& rOther)
 	: CSerializeObject(rOther)
{
	*this = rOther;
}

/////////////////////////////////////////////////////////////////////////////
CMediaLibrary& CMediaLibrary::operator= (const CMediaLibrary& other)
{
	if( this == &other )
		return *this;

	m_eMediaType = other.m_eMediaType;
	m_wFilesNum = other.m_wFilesNum;
	//m_wCurrentFileIndex = other.m_wCurrentFileIndex;
	m_wCurrentFileIndex = 0;
	
	for(int i=0; i<MAX_MEDIA_FILES_IN_LIBRARY; i++ )
		m_strMediaList[i] = other.m_strMediaList[i];

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CMediaLibrary::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pMediaTypeNode = NULL;
	if (m_eMediaType == audioChannelMediaType)
	{
		pMediaTypeNode = pFatherNode->AddChildNode("MEDIA_AUDIO");		
	}
	else if (m_eMediaType == videoChannelMediaType)
	{
		pMediaTypeNode = pFatherNode->AddChildNode("MEDIA_VIDEO");
	}
	else if (m_eMediaType == t120ChannelMediaType)
	{
		pMediaTypeNode = pFatherNode->AddChildNode("MEDIA_CONTENT");
	}
	

	// fields in media details
	pMediaTypeNode->AddChildNode("FILES_NUM", m_wFilesNum);
	
	char buffer[16];
	for(int i=0; i<MAX_MEDIA_FILES_IN_LIBRARY; i++ )
	{
		snprintf(buffer, sizeof(buffer), "FILE_%d", (i+1));
		pMediaTypeNode->AddChildNode(buffer, m_strMediaList[i]);
	}
	
}

/////////////////////////////////////////////////////////////////////////////
int	CMediaLibrary::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int	nStatus	= STATUS_OK;
	
	
	m_wCurrentFileIndex = 0;

	// get common card fields
	GET_VALIDATE_CHILD(pActionNode, "FILES_NUM", &m_wFilesNum, _0_TO_WORD);
	
	ALLOCBUFFER(buff, 256);
	int j;
	
	for(int i=0; i<MAX_MEDIA_FILES_IN_LIBRARY; i++ )
	{
		j = i + 1;
		sprintf(buff, "FILE_%d", j);
		GET_VALIDATE_CHILD(pActionNode, buff, m_strMediaList[i], ONE_LINE_BUFFER_LENGTH);
	}
	DEALLOCBUFFER(buff);
	
	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////

CMediaLibrary* CMediaLibrary::CreateCopy()
{
	CMediaLibrary* pMyCopy = new CMediaLibrary(*this);
	return (CMediaLibrary*)pMyCopy;
}

/////////////////////////////////////////////////////////////////////////////

string CMediaLibrary::GetNextMediaItemName()
{
	string mediaItemName = m_strMediaList[m_wCurrentFileIndex];
	m_wCurrentFileIndex++;
	
	//current index should remain cyclic
	if (m_wCurrentFileIndex == m_wFilesNum)
		m_wCurrentFileIndex = 0;
	
	return mediaItemName;
}

/////////////////////////////////////////////////////////////////////////////
/*
void CMediaLibrary::Dump()
{
	TRACEINTO << " CMediaLibrary::Dump()";
	
	
	TRACEINTO << " m_eMediaType = " << m_eMediaType;
	
	
	TRACEINTO << "m_wCurrentFileIndex=" << m_wCurrentFileIndex << " m_wFilesNum=" << m_wFilesNum;
	
	for (int i=0; i <MAX_MEDIA_FILES_IN_LIBRARY; i++)
	{
		TRACEINTO << " m_strMediaList[" << i << "]=" << m_strMediaList[i];
	}
}*/

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CNicCfg
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CNicCfg::CNicCfg()
{
	m_wNicId = 0;
	strncpy(m_szNicIp, "0.0.0.0", IP_ADDRESS_STR_LEN);
	m_bIsActive = false;
}

/////////////////////////////////////////////////////////////////////////////
CNicCfg::~CNicCfg()
{
}

/////////////////////////////////////////////////////////////////////////////
CNicCfg::CNicCfg(const CNicCfg& rOther)
 	: CSerializeObject(rOther)
{
	*this = rOther;
}

/////////////////////////////////////////////////////////////////////////////
CNicCfg& CNicCfg::operator= (const CNicCfg& other)
{
	if( this == &other )
		return *this;

	m_wNicId = other.m_wNicId;
	strncpy(m_szNicIp, other.m_szNicIp, IP_ADDRESS_STR_LEN );
	m_bIsActive = other.m_bIsActive;

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CNicCfg::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pNicNode = pFatherNode->AddChildNode("NIC_CFG");
	
	// fields in nic's details
	pNicNode->AddChildNode("NIC_ID", m_wNicId);
	pNicNode->AddChildNode("NIC_IP_ADDRESS", m_szNicIp);
	
	if (m_bIsActive)
		pNicNode->AddChildNode("IS_ACTIVE", "true");
	else
		pNicNode->AddChildNode("IS_ACTIVE", "false");
}

/////////////////////////////////////////////////////////////////////////////
int	CNicCfg::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int	nStatus	= STATUS_OK;

	// get common card fields
	GET_VALIDATE_CHILD(pActionNode, "NIC_ID", &m_wNicId, _0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode, "NIC_IP_ADDRESS", m_szNicIp, _1_TO_IP_ADDRESS_LENGTH);
	GET_VALIDATE_CHILD(pActionNode, "IS_ACTIVE", &m_bIsActive, _BOOL);

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
CNicCfg* CNicCfg::CreateCopy()
{
	CNicCfg* pMyCopy = new CNicCfg(*this);
	return (CNicCfg*)pMyCopy;
}



/////////////////////////////////////////////////////////////////////////////
//    GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CMediaMngrCfg* GetMediaMngrCfg()
{
	return g_pMediaMngrCfg;
}

/////////////////////////////////////////////////////////////////////////////

void SetMediaMngrCfg(CMediaMngrCfg* pCfg)
{
	g_pMediaMngrCfg = pCfg;
}
