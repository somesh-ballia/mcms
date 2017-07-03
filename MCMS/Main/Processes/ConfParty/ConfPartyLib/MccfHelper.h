#ifndef MCCFHELPER_H__
#define MCCFHELPER_H__

/////////////////////////////////////////////////////////////////////////////
#include "DataTypes.h"
#include "MediaTypes.h"

#include "ApiBaseObject.h"
#include "OsQueue.h"

#include <string>
#include <sstream>
#include <map>

/////////////////////////////////////////////////////////////////////////////
typedef size_t Duration; // in milli-seconds

/////////////////////////////////////////////////////////////////////////////
typedef HANDLE AppServerID;

/////////////////////////////////////////////////////////////////////////////
class CSegment;

class DialogElementType;
class MediaElementType;

/////////////////////////////////////////////////////////////////////////////
struct DialogState
{
	enum ActionEnum { dsa_stop = 0, dsa_start };

	ActionEnum     action;
	HANDLE         hMccfMsg;
	std::string    dialogID;
	ApiBaseObject* baseObject;
	AppServerID    appServerID;
	DWORD          seqNum;
	time_t      dialogProceedTime;
	COsQueue       clientRspMbx;
	std::string    appServerIp;

	DialogState()
	: action(dsa_start)
	, hMccfMsg(0)
	, baseObject(NULL)
	, appServerID(0)
	, seqNum(1)
	,dialogProceedTime(time(NULL))
	{}
};

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, MediaFileTypeEnum v);
std::ostream& operator <<(std::ostream& ostr, DialogState::ActionEnum v);
std::ostream& operator <<(std::ostream& ostr, const DialogState& obj);

/////////////////////////////////////////////////////////////////////////////
inline CSegment& operator <<(CSegment& seg, DialogState::ActionEnum obj)
{ return seg << static_cast<byte>(obj); }

inline CSegment& operator >>(CSegment& seg, DialogState::ActionEnum& obj)
{ return seg >> reinterpret_cast<byte&>(obj); }

/////////////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const DialogState& obj);
CSegment& operator >>(CSegment& seg, DialogState& obj);

/////////////////////////////////////////////////////////////////////////////
Duration decodeTimeDesignation(const std::string& time);

/////////////////////////////////////////////////////////////////////////////
void GetBaseFolderByMediaType(MediaFileTypeEnum type, const std::string & mediaUrl, std::string& baseFolder);
bool PrepareExternalMediaFileIVR(const DialogElementType& dialog, const MediaElementType& media, AppServerID appServerID, const std::string & appServerIp);
bool ConvertExternalSlideIVR(const std::string & url, const std::string & baseFolder);
std::string GetImageNameWithoutExtension(const std::string file_name);
void GetMediaPromptSetNameByUrl(const std::string & mediaUrl, std::string & promptSetName);

void AddMediaUrlAndAppServerIp(const std::string & mediaUrl, const std::string & appServerIp);
bool GetAppServerIpByMediaUrl(const std::string & mediaUrl, std::string & appServerIp);
bool CreateFolderForMediaUrl(MediaFileTypeEnum type, const std::string & mediaUrl);


/////////////////////////////////////////////////////////////////////////////
#endif // MCCFHELPER_H__
