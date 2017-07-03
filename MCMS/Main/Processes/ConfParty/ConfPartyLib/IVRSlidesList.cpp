#include "IVRSlidesList.h"

#include "ConfPartyDefines.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ConfPartySharedDefines.h"

#include "H221.h"
#include "IvrApiStructures.h"
#include "OsFileIF.h"
#include "IvrStructs.h"

#include "Tokenizer.h"

#include "PrettyTable.h"
#include "TraceStream.h"

/////////////////////////////////////////////////////////////////////////////
CIVRSlidesList::CIVRSlidesList()
	: m_numberOfSlides(0)
{
	for (size_t i = 0; i < MAX_SLIDES_IN_LIST; ++i)
		m_pSlide[i] = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CIVRSlidesList::~CIVRSlidesList()
{
	for (size_t i = 0; i < MAX_SLIDES_IN_LIST; ++i)
		POBJDELETE(m_pSlide[i]);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIVRSlidesList::AddSlide(const char* baseName, bool isExternalIvr)
{
	if (!baseName)
	{
		TRACEINTO << "slide name is NULL";
		return STATUS_FAIL;
	}

	size_t len = strlen(baseName);
	if (!len || len >= MAX_SLIDE_NAME_LEN)
	{
		TRACEINTO << "illegal slide name length";
		return STATUS_FAIL;
	}

	if (FindSlide(baseName) >= 0)
	{
		TRACEINTO << "Slide already exists:" << baseName;
		return STATUS_OK;	// slide already exists (okay)
	}

	if (MAX_SLIDES_IN_LIST <= m_numberOfSlides)
	{
		TRACEINTO << "slides number overflow";
		return STATUS_FAIL;
	}

	m_pSlide[m_numberOfSlides] = new CIVRSlide;
	STATUS status = m_pSlide[m_numberOfSlides]->AddSlide(baseName, isExternalIvr);

	if (status != STATUS_OK)
	{
		POBJDELETE(m_pSlide[m_numberOfSlides]);
		return status;
	}

	++m_numberOfSlides;
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIVRSlidesList::DelSlide(const char* name)
{
	return STATUS_OK; // TODO: implement!!
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIVRSlidesList::GetSlideParams(
	const char* baseName,
	eVideoResolution resolution,
	WORD protocol,
	DWORD fs,
	DWORD mbps,
	char* realSlideName,
	WORD maxRealSlideNameLen,
	WORD& checksum,
	DWORD& mediaSize,
	BYTE isTipMode)
{
	// find the desired slide according to base filename
	SSlideParams* slide = NULL;

	int ind = FindSlide(baseName);

	if (ind >= 0)
	{
		slide = m_pSlide[ind]->GetSlide(protocol, resolution, fs, mbps ,isTipMode);

		if (!slide)
		{
			TRACESTRFUNC(eLevelError) << "slide format not found, name:" << baseName;
			return STATUS_FAIL;
		}
	}
	else
	{
		TRACESTRFUNC(eLevelError) << "slide not found, name:" << baseName;
		return STATUS_FAIL;
	}

	time_t fileMsgLastModified = 0;
	bool needParamsUpdate = true;

	if (slide->lastupdate)
	{
		fileMsgLastModified = GetLastModified(slide->fileName);

		if (fileMsgLastModified == slide->lastupdate)
			needParamsUpdate = false;
	}

	if (needParamsUpdate)
		m_pSlide[ind]->GetSlideParams(slide, fileMsgLastModified);

	checksum = slide->checksum;
	mediaSize = slide->dataSize;

	if (strlen(slide->fileName) < maxRealSlideNameLen)
	{
		const char* fileName = slide->fileName;

		if (0 == strncmp(slide->fileName, "Cfg/", 4))
			fileName += 4;

		strcpy(realSlideName, fileName);
	}
	else
	{
		TRACESTRFUNC(eLevelError) << "illegal name length:" << slide->fileName;
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CIVRSlidesList::FindSlide( const char* name )
{
	for (int i = 0; i < m_numberOfSlides; ++i)
		if (0 == strcmp(name, m_pSlide[i]->GetSlideBaseName()))
			return i;

	return -1;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIVRSlidesList::CheckSlideFilesExistance(const char* name)
{
	CIVRSlide slide;
	slide.AddSlide(name);
	return slide.CheckSlideFilesExistance();
}

/////////////////////////////////////////////////////////////////////////////
CIVRSlide::CIVRSlide()
{
	m_baseSlideName[0] = 0;

	for (int i = 0; i < MAX_SLIDE_PROTOCOL; i++)
		for (int j = 0; j < MAX_SLIDE_RES; j++)
			memset(&m_slideParams[i][j], 0, sizeof(SSlideParams));
}

/////////////////////////////////////////////////////////////////////////////
bool CIVRSlide::ValidateSlidename(const char* baseSlideName)
{
	if (!baseSlideName)
	{
		FTRACESTRFUNC(eLevelError) << "Failed, the slide name is NULL";
		return false;
	}

	size_t len = strlen(baseSlideName);
	if (len > MAX_SLIDE_NAME_LEN)
	{
		FTRACESTRFUNC(eLevelError) << "Failed, the slide name is too long";
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIVRSlide::AddSlide(const char* baseSlideName, bool isExternalIvr)
{
	if (!ValidateSlidename(baseSlideName))
		return STATUS_FAIL;

	strncpy(m_baseSlideName, baseSlideName, MAX_SLIDE_NAME_LEN - 1);
	m_baseSlideName[MAX_SLIDE_NAME_LEN - 1] = '\0';


	std::string slideBasePathname(isExternalIvr ? "" : IVR_FOLDER_MAIN);

    if (!isExternalIvr)
    {
        slideBasePathname += IVR_FOLDER_SLIDES;
        slideBasePathname += "/";
    }
    slideBasePathname += m_baseSlideName;

	if (!isExternalIvr)
	{
		std::string resolvedLinkPath;
		BOOL isSoftLink = ReadSymbolicLink(slideBasePathname, resolvedLinkPath, TRUE);
		size_t folder_main_pos = resolvedLinkPath.find(IVR_FOLDER_MAIN);
		if (folder_main_pos != std::string::npos)
		{
			slideBasePathname = resolvedLinkPath.substr(folder_main_pos);
			TRACEINTO << "resolving slide path symbolic link to \"" << slideBasePathname << "\"";
		}
	}

	CPrettyTable<size_t, size_t, std::string> t("Protocol", "Resolution", "File Name");
	t.SetCaption(slideBasePathname.c_str());

	slideBasePathname += "/";
	slideBasePathname += "S";

	for (size_t protocol = 0; protocol < MAX_SLIDE_PROTOCOL; ++protocol)
	{
		std::string slideName_protocol(slideBasePathname);

		switch (protocol)
		{
		case SLIDE_PROTOCOL_261: slideName_protocol += "_PTC261"; break;
		case SLIDE_PROTOCOL_263: slideName_protocol += "_PTC263"; break;
		case SLIDE_PROTOCOL_264: slideName_protocol += "_PTC264"; break;
		case SLIDE_PROTOCOL_RTV: slideName_protocol += "_PTCRTV"; break;
		case SLIDE_PROTOCOL_264_TIP: slideName_protocol += "_TIP264"; break;
		case SLIDE_PROTOCOL_MSSVC: slideName_protocol += "_MSSVC"; break;
		case SLIDE_PROTOCOL_VP8: slideName_protocol += "_PTCVP8"; break;

		default:
			PASSERTMSG_AND_RETURN_VALUE(1, "illegal protocol name", STATUS_FAIL);
		}

		for (size_t res = 0; res < MAX_SLIDE_RES; ++res)
		{
			std::string finalSlideName(slideName_protocol);

			switch (res)
			{
			case SLIDE_RESOLUTION_CIF   : finalSlideName += "_RESCIF";     break;
			case SLIDE_RESOLUTION_QCIF  : finalSlideName += "_RESQCIF";    break;
			case SLIDE_RESOLUTION_SD    : finalSlideName += "_RESSD";      break;
			case SLIDE_RESOLUTION_HD720 : finalSlideName += "_RESHD720";   break;
			case SLIDE_RESOLUTION_4CIF  : finalSlideName += "_RES4CIF";    break;
			case SLIDE_RESOLUTION_HD1080: finalSlideName += "_RESHD1080";  break;

			default:
				PASSERTMSG_AND_RETURN_VALUE(1, "illegal resolution", STATUS_FAIL);
			}

			finalSlideName += ".slide";
			// Check protocol and resolution fitness
			if ((SLIDE_PROTOCOL_MSSVC == protocol) &&
					(/*(SLIDE_RESOLUTION_CIF == res) ||*/(SLIDE_RESOLUTION_QCIF == res) ||(SLIDE_RESOLUTION_4CIF == res)) )
				continue;

			if ((SLIDE_PROTOCOL_263 == protocol || (SLIDE_PROTOCOL_261 == protocol)) && ((SLIDE_RESOLUTION_SD == res) || (SLIDE_RESOLUTION_HD720 == res) || (SLIDE_RESOLUTION_HD1080 == res)))
				continue;

			// Check protocol and resolution fitness
			if (((SLIDE_PROTOCOL_264 == protocol) || (SLIDE_PROTOCOL_RTV == protocol)) && (SLIDE_RESOLUTION_4CIF == res))
				continue;

			//Check protocol and resolution
			if (SLIDE_PROTOCOL_264_TIP == protocol && SLIDE_RESOLUTION_HD720 != res && SLIDE_RESOLUTION_HD1080 != res)
				continue;

			// Check protocol and resolution fitness
			if ((SLIDE_PROTOCOL_RTV == protocol) && (SLIDE_RESOLUTION_HD1080 == res))
				continue;

			// Check HD/SD/4CIF slides actual existence
			if ((SLIDE_RESOLUTION_SD == res) || (SLIDE_RESOLUTION_HD720 == res) || (SLIDE_RESOLUTION_4CIF == res) || (SLIDE_RESOLUTION_HD1080 == res))
				if (!IsFileExists(finalSlideName.c_str()))
				{
					if (SLIDE_PROTOCOL_MSSVC == protocol)
						TRACEINTO << "TEST - REMOVE - MSSVC slides not checked for existence";
					else
						continue;
				}

			// H261 slides are not mandatory
			if (SLIDE_PROTOCOL_261 == protocol)
				if (!IsFileExists(finalSlideName.c_str()))
					continue;

			// checking protocol and resolution validity
			if ((protocol >= MAX_SLIDE_PROTOCOL) || (res >= MAX_SLIDE_RES)) {
				PASSERTMSG_AND_RETURN_VALUE(1, "illegal slide index", STATUS_FAIL);
			}

			// checking name length
			size_t len = finalSlideName.length();
			if (len >= MAX_SLIDE_FULL_PATHNAME_LEN) {
				PASSERTMSG_AND_RETURN_VALUE(1, "illegal slide name length", STATUS_FAIL);
			}

			// set the specific name
			strncpy(m_slideParams[protocol][res].fileName, finalSlideName.c_str(), sizeof(m_slideParams[protocol][res].fileName)-1);
			m_slideParams[protocol][res].fileName[sizeof(m_slideParams[protocol][res].fileName)-1] = '\0';
			m_slideParams[protocol][res].checksum   = static_cast<DWORD>(-1);
			m_slideParams[protocol][res].dataSize   = 0;
			m_slideParams[protocol][res].lastupdate = 0;

			t.Add(protocol, res, m_slideParams[protocol][res].fileName);
		}
	}

	TRACEINTO << t.Get();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
SSlideParams* CIVRSlide::GetSlide(WORD protocol, WORD resolution, DWORD fs, DWORD mbps, BYTE isTipMode)
{
	int protocolIndex = 0;
	int resolutionIndex = 0;

	switch (protocol)
	{
	case H261:
		protocolIndex = SLIDE_PROTOCOL_261;
		resolutionIndex = GetH261SlideResolution(resolution);
		break;

	case H263:
		protocolIndex = SLIDE_PROTOCOL_263;
		resolutionIndex = GetH263SlideResolution(resolution);
		break;

	case MS_SVC:
		protocolIndex = SLIDE_PROTOCOL_MSSVC;
		resolutionIndex = GetMSSvcSlideResolution(fs, mbps);
		break;

	case H264:
	case SVC:
		protocolIndex = isTipMode ? SLIDE_PROTOCOL_264_TIP : SLIDE_PROTOCOL_264;
		resolutionIndex = GetH264SlideResolution(fs, mbps);
		break;

	case RTV:
		protocolIndex = SLIDE_PROTOCOL_RTV;
		resolutionIndex = GetH264SlideResolution(fs, mbps);
		break;

	case VP8:
		protocolIndex = SLIDE_PROTOCOL_VP8;
		resolutionIndex = GetH264SlideResolution(fs, mbps);	// Amir: need to implement for VP8
		break;


	default:
		TRACESTRFUNC(eLevelError) << "illegal PROTOCOL value:" << protocol;
		return NULL;
	}

	PASSERTSTREAM_AND_RETURN_VALUE((protocolIndex >= MAX_SLIDE_PROTOCOL || resolutionIndex >= MAX_SLIDE_RES), "illegal slide index:" << m_baseSlideName, NULL);

	if (0 == m_slideParams[protocolIndex][resolutionIndex].fileName[0])
	{
		if (protocolIndex == SLIDE_PROTOCOL_MSSVC)
		{
			if ((resolutionIndex != SLIDE_RESOLUTION_CIF) && (0 != m_slideParams[protocolIndex][SLIDE_RESOLUTION_CIF].fileName[0]))
			{
				TRACEINTO << "MS SVC Slide with resolution index (" << resolutionIndex << ") was not found, sending CIF instead. Slide name:" << m_baseSlideName;
				return &m_slideParams[protocolIndex][SLIDE_RESOLUTION_CIF];
			}
			else
				PASSERTSTREAM_AND_RETURN_VALUE(true, "ERROR: SD slide was not found:" << m_baseSlideName, NULL);
		}

		switch (resolutionIndex)
		{
		case SLIDE_RESOLUTION_SD:
		case SLIDE_RESOLUTION_HD720:
		case SLIDE_RESOLUTION_4CIF:
		case SLIDE_RESOLUTION_HD1080:
			// V2.0: HD & SD slides existence is not mandatory: in case they are not found - send a CIF slide instead
			// V3.0: 4CIF slide is not mandatory: in case it is not found - send a CIF slide instead
			// V4.0: HD1080 slide - as mentioned above
			if (0 != m_slideParams[protocolIndex][SLIDE_RESOLUTION_CIF].fileName[0])
			{
				TRACEINTO << "HD/SD/4CIF slide was not found, sending CIF instead. Slide name:" << m_baseSlideName;
				return &m_slideParams[protocolIndex][SLIDE_RESOLUTION_CIF];
			}
			else
				PASSERTSTREAM_AND_RETURN_VALUE(true, "ERROR: CIF slide was not found:" << m_baseSlideName, NULL);

			break;

		default:
			PASSERTSTREAM_AND_RETURN_VALUE(SLIDE_PROTOCOL_261 != protocolIndex, "ERROR: slide was not found:" << m_baseSlideName, NULL);

			TRACESTRFUNC(eLevelError) << "H261 supported only for factory slides. slide was not found:" << m_baseSlideName;
			return NULL;
		}
	}

	// return the specific file belonging to the base slide
	return &m_slideParams[protocolIndex][resolutionIndex];
}

int CIVRSlide::GetH261SlideResolution(WORD resolution)
{
	int resolutionIndex = SLIDE_RESOLUTION_CIF;

	switch (resolution)
	{
	case eVideoResolutionCIF:
		resolutionIndex = SLIDE_RESOLUTION_CIF;
		break;

	case eVideoResolutionQCIF:
		resolutionIndex = SLIDE_RESOLUTION_QCIF;
		break;

	default:
		PASSERTSTREAM(true, "Illegal resolution value " << resolution);
		break;
	}

	return resolutionIndex;
}

/////////////////////////////////////////////////////////////////////////////
int CIVRSlide::GetH263SlideResolution(WORD resolution)
{
	int internalIndexResolution = SLIDE_RESOLUTION_CIF;

	switch (resolution)
	{
	case eVideoResolutionCIF:
		internalIndexResolution = SLIDE_RESOLUTION_CIF;
		break;

	case eVideoResolutionQCIF:
		internalIndexResolution = SLIDE_RESOLUTION_QCIF;
		break;

	case eVideoResolution4CIF:
		internalIndexResolution = SLIDE_RESOLUTION_4CIF;
		break;

	default:
		PASSERTSTREAM(true, "Illegal resolution value " << resolution);
		break;
	}

	return internalIndexResolution;
}

/////////////////////////////////////////////////////////////////////////////
//In H264 Video Protocol we use the Frame Size and MBPS to determine the slide resolution
int CIVRSlide::GetH264SlideResolution(DWORD fs, DWORD mbps)
{
	int resolutionIndex = 0;

	enum MinFrameSizeEnum {
		mfsQCIF   = 176  * 144  / 256, // = 99
		mfsCIF    = 352  * 288  / 256, // = 396
		mfsSD     = 704  * 576  / 256, // = 1584, 4CIF resolution is in use for "SD resolution" slide
		mfsHD720  = 1280 * 720  / 256, // = 3600
		mfsHD1080 = 1920 * 1088 / 256, // = 8160
	};

	if (INVALID == fs)
	{
		PASSERT(fs);
		resolutionIndex = SLIDE_RESOLUTION_CIF;
	}
	else if (fs >= mfsHD1080)
		resolutionIndex = SLIDE_RESOLUTION_HD1080;
	else if (fs >= mfsHD720)
		resolutionIndex = SLIDE_RESOLUTION_HD720;
	else if (fs >= mfsSD)
		resolutionIndex = SLIDE_RESOLUTION_SD;
	else if (fs >= mfsCIF)
		resolutionIndex = SLIDE_RESOLUTION_CIF;
	else if (fs >= mfsQCIF)
		resolutionIndex = SLIDE_RESOLUTION_QCIF;
	else
	{
		PASSERT(fs);
		resolutionIndex = SLIDE_RESOLUTION_QCIF;
	}

	TRACEINTOFUNC << " FS=" << fs << ", MBPS=" << mbps << ", resolution=" << resolutionIndex;
	return resolutionIndex;
}
/////////////////////////////////////////////////////////////////////////////
//In H264 Video Protocol we use the Frame Size and MBPS to determine the slide resolution
int CIVRSlide::GetMSSvcSlideResolution(DWORD fs, DWORD mbps)
{
	int resolutionIndex = 0;

	enum MinFrameSizeEnum {
		mfsQCIF   = 176  * 144  / 256, // = 99
		mfsCIF    = 352  * 288  / 256, // = 396
		mfsSD     = 704  * 576  / 256, // = 1584, 4CIF resolution is in use for "SD resolution" slide
		mfsHD720  = 1280 * 720  / 256, // = 3600
		mfsHD1080 = 1920 * 1088 / 256, // = 8160
	};
	TRACEINTOFUNC << " supported MS SVC slide resolutions: CIF, SD, HD720, HD1080";
	if (INVALID == fs)
	{
		PASSERT(fs);
		resolutionIndex = SLIDE_RESOLUTION_CIF;
	}
	else if (fs >= mfsHD1080)
		resolutionIndex = SLIDE_RESOLUTION_HD1080;
	else if (fs >= mfsHD720)
		resolutionIndex = SLIDE_RESOLUTION_HD720;
	else if (fs >= mfsSD)
		resolutionIndex = SLIDE_RESOLUTION_SD;
	else if (fs >= mfsCIF)
		resolutionIndex = SLIDE_RESOLUTION_CIF;
	/*else if (fs >= mfsQCIF)
		resolutionIndex = SLIDE_RESOLUTION_QCIF;*/
	else
	{
		PASSERTSTREAM(true, "FS value " << fs << " is too small for MS_SVC slides. sending CIF slide");
		resolutionIndex = SLIDE_RESOLUTION_CIF;
	}

	TRACEINTOFUNC << " FS=" << fs << ", MBPS=" << mbps << ", resolution=" << resolutionIndex;
	return resolutionIndex;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CIVRSlide::GetSlideParams(SSlideParams* slide, time_t fileMsgLastModified)
{
	if (!fileMsgLastModified)
		fileMsgLastModified = GetLastModified(slide->fileName);

	slide->lastupdate = fileMsgLastModified;

	FILE* infile = fopen(slide->fileName, "rb");
	if (!infile)
	{
		TRACESTRFUNC(eLevelError) << "Cannot open file:" << slide->fileName;
		return STATUS_FAIL;
	}

	SSlideHeader header;
	const size_t size = sizeof(SSlideHeader);

	size_t count = fread(&header, 1, size, infile);
	fclose(infile);

	if (size != count)
	{
		TRACESTRFUNC(eLevelError) << "Read file error, file:" << slide->fileName;
		return STATUS_FAIL;
	}

	TRACEINTO
		<< "slideHeaderStruct {marker:'" << CLexeme(header.marker, sizeof(header.marker))
		<< "', fileHeaderSize:" << header.fileHeaderSize
		<< ", version:" << CLexeme(header.version, sizeof(header.version)) << '}';

	static const char MARKER[] = "POLYCOM ";
	static const char VERSION[] = "0001";

	if (header.fileHeaderSize != SLIDE_HEADER_SIZE           ||
		0 != strncmp(MARKER, header.marker, sizeof(MARKER) - 1)  ||
		0 != strncmp(VERSION, header.version, sizeof(VERSION) - 1))
	{
		TRACESTRFUNC(eLevelError) << "Illegal Slide Header, file:" << slide->fileName;
		// return STATUS_FAIL;
	}

	slide->checksum = header.checksum; // name+checksum defines the file
	slide->dataSize = header.fileSize - header.fileHeaderSize;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CIVRSlide::CheckSlideFilesExistance()
{
	for (int protocol = 0; protocol < MAX_SLIDE_PROTOCOL; ++protocol)
	{
		if ((SLIDE_PROTOCOL_MSSVC == protocol) || (SLIDE_PROTOCOL_261 == protocol) ||
		    (SLIDE_PROTOCOL_RTV == protocol) || (SLIDE_PROTOCOL_VP8 == protocol))
		{
			TRACEINTO << " protocol: " << (DWORD)protocol << " checking was skipped";
			continue;
		}

		for (int res = 0; res < MAX_SLIDE_RES; ++res)
		{
			if ((SLIDE_RESOLUTION_SD == res) || (SLIDE_RESOLUTION_HD720 == res)
				|| (SLIDE_RESOLUTION_4CIF == res) || (SLIDE_RESOLUTION_HD1080 == res))
				continue;	// Slides are not mandatory: 	from V2.0: HD/SD;  from V3.0: 4CIF;  from V4.0: HD1080

			//Check protocol and resolution
			if (SLIDE_PROTOCOL_264_TIP == protocol && SLIDE_RESOLUTION_HD720 != res && SLIDE_RESOLUTION_HD1080 != res)
				continue;

			const char* slideName = m_slideParams[protocol][res].fileName;

			if (!IsFileExists(slideName))
			{
				PTRACE2(eLevelError,"CIVRSlide::CheckSlideFilesExistance - slide file doesn't exist, file = ", slideName);
				return STATUS_IVR_VIDEO_FILE_DOES_NOT_EXIST;
			}
		}
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
