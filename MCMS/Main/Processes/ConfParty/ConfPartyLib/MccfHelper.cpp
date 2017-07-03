#include "MccfHelper.h"

#include "OsQueue.h"

#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"

#include "MediaTypeManager.h"
#include "FilesCache.h"

#include "DialogElementType.h"

#include "Trace.h"
#include "TraceStream.h"

#include "ConfPartyDefines.h"

#include "OsFileIF.h"


#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////
std::map<std::string, std::string> g_mapMediaUrlAndAppServerIp;

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, MediaFileTypeEnum v)
{
	static const char* mediaTypes[mft_LAST_] =
	{
		"*unknown*",
		"audio",
		"video",
		"image"
	};

	if (size_t(v) < sizeof(mediaTypes)/sizeof(mediaTypes[0]))
		ostr << mediaTypes[static_cast<size_t>(v)];
	else
		ostr << "unexpected media type " << size_t(v);

	return ostr;
}

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, DialogState::ActionEnum v)
{
	return ostr << (v == DialogState::dsa_stop ? "stop" : "start");
}

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const DialogState& obj)
{
	return ostr
		<< "{action:" << obj.action
		<< ", id:" << obj.appServerID
		<< ", hMccf:" << obj.hMccfMsg
		<< ", base:" << obj.baseObject
		<< ", dlg:" << obj.dialogID
		<< ", RspMbx:" << &obj.clientRspMbx
		<< ", seqNum:" << obj.seqNum
		<< ", appServerIp" << obj.appServerIp
		<< '}';
}

/////////////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const DialogState& obj)
{
	return seg << obj.action << obj.appServerID << obj.hMccfMsg << (void*)obj.baseObject << obj.dialogID << obj.clientRspMbx << obj.seqNum << obj.dialogProceedTime;
}

CSegment& operator >>(CSegment& seg, DialogState& obj)
{
	return seg >> obj.action >> obj.appServerID >> obj.hMccfMsg >> (void*&)obj.baseObject >> obj.dialogID >> obj.clientRspMbx >> obj.seqNum >> obj.dialogProceedTime;
}

/////////////////////////////////////////////////////////////////////////////
Duration decodeTimeDesignation(const std::string& time)
{
	if (time.empty())
		return static_cast<Duration>(-1);

	char* next = NULL;
	double d = strtod(time.c_str(), &next);
	FPASSERT_AND_RETURN_VALUE(!next, 0);

	if (0 == strcmp(next, "s"))
		d *= 1000; // in ms
	else if (0 != strcmp(next, "ms"))
		FPASSERT_AND_RETURN_VALUE(*next, 0);

	return static_cast<Duration>(d);
}

/////////////////////////////////////////////////////////////////////////////
void GetBaseFolderByMediaType(MediaFileTypeEnum type, const std::string & mediaUrl, std::string& baseFolder)
{
    std::string promptSetName;
    GetMediaPromptSetNameByUrl(mediaUrl, promptSetName);

    std::string appServerIp;
    GetAppServerIpByMediaUrl(mediaUrl, appServerIp);

    baseFolder = IVR_EXTERNAL_FOLDER_MAIN;
    baseFolder += "/" + appServerIp + "/" + promptSetName;
    
    switch (type)
	{
	case mft_Audio:
		baseFolder += IVR_FOLDER_MUSIC;
		break;

	case mft_Video:
	case mft_Image:
		baseFolder += IVR_FOLDER_SLIDES;
		break;

	default:
		FPASSERT(type);
	}
}

/////////////////////////////////////////////////////////////////////////////
bool PrepareExternalMediaFileIVR(const DialogElementType& dialog, const MediaElementType& media, AppServerID appServerID, const std::string & appServerIp)
{
	if (!media.m_loc.IsAssigned())
		return true;

	AddMediaUrlAndAppServerIp(media.m_loc, appServerIp);
    
	MediaFileTypeEnum type = CMediaTypeManager::DeriveMediaType(media.m_type, media.m_loc);

	CreateFolderForMediaUrl(type, media.m_loc);

	std::string baseFolder;
	GetBaseFolderByMediaType(type, media.m_loc, baseFolder);

	
	const CLocalFileDescriptor* file = CFilesCache::instance().AddFile(appServerID, media.m_loc, type);

	CSegment* pSeg = new CSegment;
	*pSeg << eProcessConfParty << MCCF_IVR_FILE_DOWNLOADED << media.m_loc->c_str() << baseFolder << file->path();

	FTRACEINTO
		<< "appServer:" << appServerID << ", type:" << type << ", available:" << file->available()
		<< "\nURL:" << media.m_loc;

	CManagerApi api(eProcessUtility);
	api.SendMsg(pSeg, UTILITY_DOWNLOAD_FILE);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
bool ConvertExternalSlideIVR(const string & url, const std::string & baseFolder)
{

	const CLocalFileDescriptor* file = CFilesCache::const_instance().fileDescriptor(url);
	FPASSERT_AND_RETURN_VALUE(file == NULL,false);

	std::string slideFolder = GetImageNameWithoutExtension(file->path());

	std::string outputPath = baseFolder + '/' + slideFolder;
	std::string inputFile = baseFolder +  '/' + file->path();



	FPASSERT_AND_RETURN_VALUE(CreateDirectory(outputPath.c_str(),0777) == FALSE,false);

	int slideConversionMethod = eIvrSlideLowHighRes;      //For DMA request, convert high and low resolution slide together.
	int imageType = eIvrSlideImageJpg; // For DMA request, the image is .Jpg
    
	CSegment* pSeg = new CSegment;
	*pSeg << eProcessConfParty << MCCF_IVR_SLIDE_CONVERTED << url<< outputPath << inputFile
			<< slideConversionMethod << imageType;

	FTRACEINTO
		<< "\n URL:" << url
		<< "\n Output path:" << outputPath
		<< "\n Input file:" << inputFile
		<< "\n slideConversionMethod:"<< slideConversionMethod;

	CManagerApi api(eProcessUtility);
	api.SendMsg(pSeg, UTILITY_CONVERT_SLIDE);

	return true;
}

std::string GetImageNameWithoutExtension(const std::string file_name)
{


  const char* noExtensionName = strrchr(file_name.c_str(),'.');
  if(noExtensionName == NULL)
  {
	  FTRACEINTO << "image file have no extersion name";   
	  return "";
  }
  if((strcmp(noExtensionName,".jpg")==0)
	  ||(strcmp(noExtensionName,".bmp")==0))
  {
	  noExtensionName = file_name.substr(0, file_name.length()-4).c_str();
  }
  else if((strcmp(noExtensionName,".jpeg")==0))
  {
	  noExtensionName = file_name.substr(0, file_name.length()-5).c_str();
  }
  else
  {
	  FTRACEINTO << "unsupported extension of image file" << noExtensionName;	  
	  return "";
  }   
  return noExtensionName;

}

/////////////////////////////////////////////////////////////////////////////
void GetMediaPromptSetNameByUrl(const std::string & mediaUrl, std::string & promptSetName)
{
    std::string::size_type rFirstSlashPos = mediaUrl.rfind('/');
    if (std::string::npos != rFirstSlashPos)
    {
        std::string::size_type rSecondSlashPos = mediaUrl.rfind('/', rFirstSlashPos-1);
        if (std::string::npos != rSecondSlashPos)
        {
            promptSetName = mediaUrl.substr(rSecondSlashPos+1, rFirstSlashPos-rSecondSlashPos-1);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void AddMediaUrlAndAppServerIp(const std::string & mediaUrl, const std::string & appServerIp)
{
    g_mapMediaUrlAndAppServerIp[mediaUrl] = appServerIp;
}

/////////////////////////////////////////////////////////////////////////////
bool GetAppServerIpByMediaUrl(const std::string & mediaUrl, std::string & appServerIp)
{
    if (g_mapMediaUrlAndAppServerIp.end() != g_mapMediaUrlAndAppServerIp.find(mediaUrl))
    {
        appServerIp = g_mapMediaUrlAndAppServerIp[mediaUrl];
        return true;
    }
    else
    {
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////
bool CreateFolderForMediaUrl(MediaFileTypeEnum type, const std::string & mediaUrl)
{
    std::string promptSetName;
    GetMediaPromptSetNameByUrl(mediaUrl, promptSetName);

    std::string appServerIp;
    GetAppServerIpByMediaUrl(mediaUrl, appServerIp);

    std::string baseFolder = IVR_EXTERNAL_FOLDER_MAIN;
    baseFolder += "/";
    baseFolder += appServerIp;
    DIR * dir = opendir(baseFolder.c_str());
    if (NULL == dir)
    {
        FPASSERT_AND_RETURN_VALUE(CreateDirectory(baseFolder.c_str(),0777) == FALSE, false);
    }
    else
    {
        closedir(dir);
    }

    baseFolder += "/" + promptSetName;
    dir = opendir(baseFolder.c_str());
    if (NULL == dir)
    {
        FPASSERT_AND_RETURN_VALUE(CreateDirectory(baseFolder.c_str(),0777) == FALSE, false);
    }
    else
    {
        closedir(dir);
    }

    switch (type)
	{
	case mft_Audio:
		baseFolder += IVR_FOLDER_MUSIC;
		break;

	case mft_Video:
	case mft_Image:
		baseFolder += IVR_FOLDER_SLIDES;
		break;

	default:
		FPASSERT(type);
	}

    dir = opendir(baseFolder.c_str());
    if (NULL == dir)
    {
        FPASSERT_AND_RETURN_VALUE(CreateDirectory(baseFolder.c_str(),0777) == FALSE, false);
    }
    else
    {
        closedir(dir);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
