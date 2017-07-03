#include "MediaRepository.h"
#include "OsFileIF.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////class CMediaRepository
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CMediaRepository::CMediaRepository() : CPObject()
{
	m_pDtmfAlgDB = new CDtmfAlgDB;
	m_pVideoDB = new CMediaDB(E_DB_MEDIA_VIDEO);
	m_pContentDB = new CMediaDB(E_DB_MEDIA_CONTENT);
	m_pAudioDB = new CMediaDB(E_DB_MEDIA_AUDIO);
}

CMediaRepository::~CMediaRepository()
{
	::SetMediaRepository(NULL);
	
	
	TRACEINTO << "CMediaRepository::~CMediaRepository";
	
	
	POBJDELETE(m_pDtmfAlgDB);
	POBJDELETE(m_pVideoDB);
	POBJDELETE(m_pContentDB);
	POBJDELETE(m_pAudioDB);
}



void CMediaRepository::Init()
{
	//dtmf DB
	m_pDtmfAlgDB->Init();
	
	//video DB
	m_pVideoDB->Init();
	
	//content DB
	m_pContentDB->Init();
	
	//audio DB
	m_pAudioDB->Init();
	
	::SetMediaRepository(this);
	
	TRACEINTO << "CMediaRepository::Init Finished";
}



void CMediaRepository::PrintMediaRepository()
{
	
	
	
}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////class CMediaDB
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CMediaDB::CMediaDB(EDBMedia dbMediaType) : CPObject()
{
	m_cacheSize = 0;
	m_maxCacheFiles = 0;
	m_maxCacheFileSize = 0;
	m_maxCacheFilesSize = 0;
	m_eDBMediaType = dbMediaType;
}

CMediaDB::~CMediaDB()
{
	TRACEINTO << "CMediaDB::~CMediaDB [" << EDBMediaNames[m_eDBMediaType] << "] map size= " << m_mediaMap.size();
	
	PrintMediaDB();
	
	MediaMap::iterator it;
	for (it = m_mediaMap.begin() ; it != m_mediaMap.end(); it++)
	{
		TRACEINTO << "MediaMap[" << (*it).first << "] ==> " << (*it).second;
		POBJDELETE((*it).second);
		//m_mediaMap.erase(it);
	}

	m_mediaMap.clear();
	TRACEINTO << "CMediaDB::~CMediaDB [" << EDBMediaNames[m_eDBMediaType] << "] (after clear...) map size= " << m_mediaMap.size();
	
}

void CMediaDB::Init()
{
		
}


CMediaElement* CMediaDB::GetMediaElement(const string mediaName)
{
	MediaMap::iterator it;
	it = m_mediaMap.find(mediaName);
	
	if (it == m_mediaMap.end())
	{
		//not found
		TRACEINTO << "CMediaDB::GetMediaElement mediaName:" << mediaName << " not found in DB... loading from file...";
		CMediaElement* element = new CMediaElement();
		int status = element->LoadFile(mediaName);
		if (status != STATUS_OK)
		{
			TRACEINTO << "MM ERROR CMediaDB::GetMediaElement - Error in LoadFile: " << mediaName;
			return NULL;
		}
		
		//set element in map
		m_mediaMap[mediaName] = element;
		
		it = m_mediaMap.find(mediaName);
	}
	
	//inc ref counter
	((*it).second)->IncRefCounter();	
	
	return (*it).second;
}


void CMediaDB::AddMediaElement(CMediaElement* mediaElement)
{
	string elementKey = mediaElement->GetName();
	m_mediaMap.insert( pair<string,CMediaElement*>(elementKey, mediaElement));
}


void CMediaDB::PrintMediaDB()
{
	MediaMap::iterator it;
	
	TRACEINTO << "CMediaDB::PrintMediaDB [" << EDBMediaNames[m_eDBMediaType] << "]"; 
	
	// show content:
	for ( it = m_mediaMap.begin() ; it != m_mediaMap.end(); it++ )
	{
		TRACEINTO << "MediaMap[" << (*it).first << "] ==> " << (*it).second << " RefCount:" << ((*it).second)->GetRefCounter();
	}
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////class CMediaElement
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CMediaElement::CMediaElement() : CPObject()
{	
	m_fileName = "";
	m_fileSize = 0;
	m_refCounter = 0;
	m_lastUsed = 0;
	
	m_dataBuffer = NULL;
}

CMediaElement::~CMediaElement()
{
	TRACEINTO << "CMediaElement::~CMediaElement name: " << m_fileName;
	PDELETEA(m_dataBuffer);
}


string CMediaElement::GetName()
{
	return m_fileName;
}

DWORD CMediaElement::GetSize()
{
	return m_fileSize;
	
}

DWORD CMediaElement::GetRefCounter()
{
	return m_refCounter;
}

DWORD CMediaElement::GetLastUsed()
{
	return m_lastUsed;
}

BYTE* CMediaElement::GetDataBuffer()
{
	return m_dataBuffer;
}


int CMediaElement::LoadFile(string fileName)
{
	m_fileName = fileName;
	
	// open file
	if (!IsFileExists( m_fileName.c_str() ))
	{
		TRACEINTO << "MM ERROR CMediaElement::LoadFile - Error - File not exists: " << m_fileName;
		return STATUS_ERROR;
	}
	
	FILE* file = fopen( m_fileName.c_str(), "rb" );
	if (NULL == file)
	{
		TRACEINTO << "MM ERROR CMediaElement::LoadFile - Error - File open error:  " << m_fileName;
		return STATUS_ERROR;
	}
	
	// read file
	m_fileSize = GetFileSize(m_fileName);
	if ((0 >= m_fileSize) || (MAX_FILE_BUFFER_SIZE < m_fileSize))
	{
		TRACEINTO << "MM ERROR CMediaElement::LoadFile - Error - File name:  " << m_fileName << " File size: " << m_fileSize;
		fclose(file);
		return STATUS_ERROR;
	}
	
	m_dataBuffer = new BYTE[m_fileSize];
	
	DWORD numRead = fread( m_dataBuffer, 1, m_fileSize, file );
	if (numRead != m_fileSize)
	{
		TRACEINTO << "MM ERROR CMediaElement::LoadFile - Error - File read error:  " << m_fileName;
		PDELETEA(m_dataBuffer);
		fclose(file);
		return STATUS_ERROR;
	} 
	
	// close file & return	
	fclose(file);	
	return STATUS_OK;
}


void CMediaElement::IncRefCounter()
{
	m_refCounter++;
}


void CMediaElement::DecRefCounter()
{
	if (m_refCounter > 0)
		m_refCounter--;
}

/////////////////////////////////////////////////////////////////////////////
//    GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CMediaRepository* GetMediaRepository()
{
	return g_pMediaRepository;
}

/////////////////////////////////////////////////////////////////////////////

void SetMediaRepository(CMediaRepository* pMediaRepository)
{
	g_pMediaRepository = pMediaRepository;
}


