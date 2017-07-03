#include "DtmfAlgDB.h"
#include "OsFileIF.h"
#include "MediaMngrCfg.h"
#include "IpCommonUtilTrace.h"

//////////////////////////////////////////
//CDtmfAlgDB
//////////////////////////////////////////

CDtmfAlgDB::CDtmfAlgDB() : CPObject()
{
	
}

CDtmfAlgDB::~CDtmfAlgDB()
{
	TRACEINTO << "CDtmfAlgDB::~CDtmfAlgDB";
	
	for (int i = 0; i < E_ALG_DTMF_COUNT; ++i)
	{
		POBJDELETE(m_dtmfAlgArray[i]);
	}
}

void CDtmfAlgDB::Init()
{
	for (int i = 0; i < E_ALG_DTMF_COUNT; ++i)
	{
		m_dtmfAlgArray[i] = new CDtmfAlgSet();
		m_dtmfAlgArray[i]->Init(i);
	}
	
	TRACEINTO << "CDtmfAlgDB::Init Finished";
}


CDtmfElement* CDtmfAlgDB::GetDtmfElement(CapEnum audioAlg, EAudioTone audioTone)
{
	int algDtmf = GetAlgDtmf(audioAlg);
	if (algDtmf == STATUS_ERROR)
	{	
		TRACEINTO << "MM ERROR CDtmfAlgDB::GetDtmfElement algDtmf is STATUS_ERROR - Audio Alg was not found.";
		return NULL;
	}
	
	CDtmfAlgSet* algSet = m_dtmfAlgArray[algDtmf];
	if (algSet == NULL)
	{
		TRACEINTO << "MM ERROR CDtmfAlgDB::GetDtmfElement algSet== NULL";
		return NULL;
	}
	
	int dtmfIndex = GetAudioTone(audioTone);
	if (dtmfIndex == STATUS_ERROR)
	{
		TRACEINTO << "MM ERROR CDtmfAlgDB::GetDtmfElement dtmfIndex is STATUS_ERROR - audio tone is not supported. ";
		return NULL;
	}
	
	return algSet->GetDtmfElement(dtmfIndex);
}


int CDtmfAlgDB::GetAlgDtmf(CapEnum audioAlg)
{
	TRACEINTO << "CDtmfAlgDB::GetAlgDtmf audioAlg = <" << CapEnumToString(audioAlg) << ">";
	
	switch (audioAlg)
	{
		//G711
		case eG711Alaw64kCapCode:
		case eG711Ulaw64kCapCode:		
		{
			return E_G711_64K_DTMF_ALG;
		}
		
		//G722
		case eG722_48kCapCode:
			return E_G722_48K_DTMF_ALG;
		case eG722_56kCapCode:
			return E_G722_56K_DTMF_ALG;
		case eG722_64kCapCode:
			return E_G722_64K_DTMF_ALG;

		
		//G729
		case eG729CapCode:
		case eG729AnnexACapCode:
		case eG729wAnnexBCapCode:
		case eG729AnnexAwAnnexBCapCode:
		{	
			return E_G729_8K_DTMF_ALG;
		}
			
		//G722.1
		case eG7221_24kCapCode:
			return E_G722_1_24K_DTMF_ALG;
		case eG7221_32kCapCode:
			return E_G722_1_32K_DTMF_ALG;
			
			
		//Siren14
		case eSiren14_24kCapCode:
			return E_SIREN_14_24K_DTMF_ALG;
		case eSiren14_32kCapCode:
			return E_SIREN_14_32K_DTMF_ALG;
		case eSiren14_48kCapCode:
			return E_SIREN_14_48K_DTMF_ALG;
		//Siren14 Stereo
		case eSiren14Stereo_48kCapCode:
			return E_SIREN_14_STEREO_48K_DTMF_ALG;
		case eSiren14Stereo_56kCapCode:
			return E_SIREN_14_STEREO_56K_DTMF_ALG;
		case eSiren14Stereo_64kCapCode:
			return E_SIREN_14_STEREO_64K_DTMF_ALG;
		case eSiren14Stereo_96kCapCode:
			return E_SIREN_14_STEREO_96K_DTMF_ALG;
		
		//G722.1.C
		case eG7221C_24kCapCode:
			return E_G722_1_C_24K_DTMF_ALG;
		case eG7221C_32kCapCode:
			return E_G722_1_C_32K_DTMF_ALG;
		case eG7221C_48kCapCode:
			return E_G722_1_C_48K_DTMF_ALG;
		
	
		/*//G723.1
		case eG7231CapCode:
		case eG7231AnnexCapCode:
		{							
			return E_G723_1_DTMF_ALG;
		}*/
	
		//G719
		case eG719_32kCapCode:
			return E_G719_32K_DTMF_ALG;
		case eG719_48kCapCode:
			return E_G719_48K_DTMF_ALG;
		case eG719_64kCapCode:
			return E_G719_64K_DTMF_ALG;

		//G719 Stereo
		case eG719Stereo_64kCapCode:
			return E_G719_STEREO_64K_DTMF_ALG;
		case eG719Stereo_96kCapCode:
			return E_G719_STEREO_96K_DTMF_ALG;
		case eG719Stereo_128kCapCode:
			return E_G719_STEREO_128K_DTMF_ALG;

		//Siren22
		case eSiren22_32kCapCode:
			return E_SIREN_22_32K_DTMF_ALG;
		case eSiren22_48kCapCode:
			return E_SIREN_22_48K_DTMF_ALG;
		case eSiren22_64kCapCode:
			return E_SIREN_22_64K_DTMF_ALG;

		//Siren22 Stereo
		case eSiren22Stereo_64kCapCode:
			return E_SIREN_22_STEREO_64K_DTMF_ALG;
		case eSiren22Stereo_96kCapCode:
			return E_SIREN_22_STEREO_96K_DTMF_ALG;
		case eSiren22Stereo_128kCapCode:
			return E_SIREN_22_STEREO_128K_DTMF_ALG;
		// SirenLPR
		case eSirenLPR_32kCapCode:
			return E_SIREN_LPR_32K_DTMF_ALG;
		case eSirenLPR_48kCapCode:
			return E_SIREN_LPR_48K_DTMF_ALG;

		case eSirenLPR_64kCapCode:
			return E_SIREN_LPR_64K_DTMF_ALG;
		case eSirenLPRStereo_64kCapCode:
			return E_SIREN_LPR_STEREO_64K_DTMF_ALG;
		case eSirenLPRStereo_96kCapCode:
			return E_SIREN_LPR_STEREO_96K_DTMF_ALG;
		case eSirenLPRStereo_128kCapCode:
			return E_SIREN_LPR_STEREO_128K_DTMF_ALG;

		default:
			return STATUS_ERROR;
	}

}


int CDtmfAlgDB::GetAudioTone(EAudioTone audioTone)
{
	TRACEINTO << "CDtmfAlgDB::GetAudioTone audioTone = <" << ::GetTone(audioTone) << ">";
	
	switch (audioTone)
	{
		case E_AUDIO_TONE_DTMF_0:		return E_DTMF_0;
		case E_AUDIO_TONE_DTMF_1:		return E_DTMF_1;
		case E_AUDIO_TONE_DTMF_2:		return E_DTMF_2;
		case E_AUDIO_TONE_DTMF_3:		return E_DTMF_3;
		case E_AUDIO_TONE_DTMF_4:		return E_DTMF_4;
		case E_AUDIO_TONE_DTMF_5:		return E_DTMF_5;
		case E_AUDIO_TONE_DTMF_6:		return E_DTMF_6;
		case E_AUDIO_TONE_DTMF_7:		return E_DTMF_7;
		case E_AUDIO_TONE_DTMF_8:		return E_DTMF_8;
		case E_AUDIO_TONE_DTMF_9:		return E_DTMF_9;
		case E_AUDIO_TONE_DTMF_STAR:	return E_DTMF_STAR;
		case E_AUDIO_TONE_DTMF_PAUND:	return E_DTMF_POUND;
	}
	
	return STATUS_ERROR;
}


//////////////////////////////////////////
//CDtmfAlgSet
//////////////////////////////////////////

CDtmfAlgSet::CDtmfAlgSet() : CPObject()
{
	
}

CDtmfAlgSet::~CDtmfAlgSet()
{
	TRACEINTO << "CDtmfAlgSet::~CDtmfAlgSet m_dtmfAlgType= " << m_dtmfAlgType;
	
	for (int i = 0; i < E_DTMF_COUNT; ++i)
	{
		POBJDELETE(m_dtmfArray[i]);
	}
}

void CDtmfAlgSet::Init(int dtmfAlg)
{
	m_dtmfAlgType = (EDtmfAlg)dtmfAlg;
	for (int i = 0; i < E_DTMF_COUNT; ++i)
	{
		m_dtmfArray[i] = new CDtmfElement();
		m_dtmfArray[i]->Init(i, m_dtmfAlgType);
	}
}


CDtmfElement* CDtmfAlgSet::GetDtmfElement(int dtmfIndex)
{
	return m_dtmfArray[dtmfIndex];
}


//////////////////////////////////////////
//CDtmfElement
//////////////////////////////////////////

CDtmfElement::CDtmfElement() : CPObject()
{
	m_dtmfBuffer = NULL;
	m_dtmfBufferSize = 0;
	m_strFullFileName = "";
}

CDtmfElement::~CDtmfElement()
{
	TRACEINTO << "CDtmfElement::~CDtmfElement m_dtmfBufferSize= " << m_dtmfBufferSize;
	
	PDELETEA(m_dtmfBuffer);
}

void CDtmfElement::Init(int dtmfTone, EDtmfAlg dtmfAlgType)
{
	string fileName = GetDtmfFileName(dtmfTone, dtmfAlgType);
	
	int status = ReadDtmfFile(fileName);
	if( STATUS_OK != status)
	{
		TRACEINTO << "MM ERROR CDtmfElement::Init - Error - Failed in ReadDtmfFile: " << fileName.c_str();
		return;
	}
	
	m_strFullFileName = fileName;
	
	TRACEINTO << "CDtmfElement::Init Finished - " << fileName;
}

string CDtmfElement::GetDtmfFileName(int dtmfTone, EDtmfAlg dtmfAlgType)
{
	string fileFullName = ::GetMediaMngrCfg()->GetInstallationDtmfFileReadPath();

	fileFullName += "/";
	
	fileFullName += EDtmfAlgStr[dtmfAlgType];
	
	fileFullName += "/";
	
	fileFullName += EDtmfIndexStr[dtmfTone];
	
	fileFullName += ".dtmf";
	
	TRACEINTO << "CDtmfElement::GetDtmfFileName - dtmf file name: " << fileFullName;
	
	return fileFullName;
}


int CDtmfElement::ReadDtmfFile(string fileName)
{
	// open file
	if (!IsFileExists( fileName.c_str() ))
	{	
		TRACEINTO << "MM ERROR CDtmfElement::ReadDtmfFile - Error - File NotExists: " << fileName.c_str();
		return STATUS_ERROR;
	}	
	
	FILE* file = fopen( fileName.c_str(), "rb" );
	if (NULL == file)
	{
		TRACEINTO << "MM ERROR CDtmfElement::ReadDtmfFile - Error - File Open Error: " <<  fileName.c_str();
		return STATUS_ERROR;
	}
		
	// read file
	int fileSize = GetFileSize(fileName);
	if (fileSize <= 0 || fileSize >= MAX_FILE_BUFFER_SIZE)
	{
		TRACEINTO << "MM ERROR CDtmfElement::ReadDtmfFile - Error - File is empty: " <<  fileName.c_str();
		fclose(file);
		return STATUS_ERROR;
	}
	
	m_dtmfBufferSize = fileSize;
	m_dtmfBuffer = new BYTE[m_dtmfBufferSize];
	
	int numRead = fread( m_dtmfBuffer, 1, fileSize, file);
	if (numRead != fileSize)
	{
		TRACEINTO << "MM ERROR CDtmfElement::ReadDtmfFile - Error - File Read Error: " << fileName.c_str();
		fclose(file);
		return STATUS_ERROR;
	}
	
	// close file & return	
	fclose(file);	
	return STATUS_OK;
}




/////////////////////////////////////////////////////////////////////////////
//    GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////


char GetTone(unsigned long toneId)
{
	switch (toneId)
	{
		case E_AUDIO_TONE_DTMF_0:	return '0';
		case E_AUDIO_TONE_DTMF_1:	return '1';
		case E_AUDIO_TONE_DTMF_2:	return '2';
		case E_AUDIO_TONE_DTMF_3:	return '3';
		case E_AUDIO_TONE_DTMF_4:	return '4';
		case E_AUDIO_TONE_DTMF_5:	return '5';
		case E_AUDIO_TONE_DTMF_6:	return '6';
		case E_AUDIO_TONE_DTMF_7:	return '7';
		case E_AUDIO_TONE_DTMF_8:	return '8';
		case E_AUDIO_TONE_DTMF_9:	return '9';
		case E_AUDIO_TONE_DTMF_STAR:	return '*';
		case E_AUDIO_TONE_DTMF_PAUND:	return '#';
		
		case E_AUDIO_TONE_DTMF_A:   return 'A';
		case E_AUDIO_TONE_DTMF_B:   return 'B';
		case E_AUDIO_TONE_DTMF_C:   return 'C';
		case E_AUDIO_TONE_DTMF_D:   return 'D';
	}

	return ' ';
}

