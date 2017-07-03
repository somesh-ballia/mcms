#include "MediaTypeManager.h"

#include "TraceStream.h"

/////////////////////////////////////////////////////////////////////////////
CMediaTypeManager::CMediaTypeManager()
{
	// by Media Type (MIME)
	map_.insert(std::make_pair("audio/", mft_Audio));
	map_.insert(std::make_pair("video/", mft_Video));
	map_.insert(std::make_pair("image/", mft_Image));

	// by File Extension
	map_.insert(std::make_pair(".wav", mft_Audio));
	map_.insert(std::make_pair(".mp3", mft_Audio));

	map_.insert(std::make_pair(".3gp", mft_Video));

	map_.insert(std::make_pair(".jpg", mft_Image));
	map_.insert(std::make_pair(".png", mft_Image));
	map_.insert(std::make_pair(".gif", mft_Image));
	map_.insert(std::make_pair(".bmp", mft_Image));
}

/////////////////////////////////////////////////////////////////////////////
MediaFileTypeEnum CMediaTypeManager::LookupByMediaType(const std::string& mediaType) const
{
	std::string::size_type pos = mediaType.find_last_of('/');

	if (pos == std::string::npos)
	{
		FTRACESTRFUNC(eLevelWarn) << "Bad format for mediaType:" << mediaType;
		return mft_Unknown;
	}

	CLexeme type(mediaType.c_str(), pos + 1);
	FTRACEINTO << "media type:" << type;

	MediaTypesMap::const_iterator it = map_.find(type);

	if (it == map_.end())
	{
		FTRACESTRFUNC(eLevelError) << "Unregistered media type:" << type;
		return mft_Unknown;
	}

	return it->second;
}

/////////////////////////////////////////////////////////////////////////////
MediaFileTypeEnum CMediaTypeManager::LookupByFileExtension(const std::string& fileName) const
{
	std::string::size_type pos_ext = fileName.find_last_of('.');
	std::string::size_type pos_dir = fileName.find_last_of('/');

	if (pos_ext < pos_dir || pos_ext == std::string::npos)
	{
		FTRACESTRFUNC(eLevelWarn) << "No file extension found for file:" << fileName;
		return mft_Unknown;
	}

	CLexeme extension(fileName.c_str() + pos_ext, fileName.size() - pos_ext);
	FTRACEINTO << "extension:" << extension;

	MediaTypesMap::const_iterator it = map_.find(extension);

	if (it == map_.end())
	{
		FTRACESTRFUNC(eLevelError) << "Unregistered file extension:" << extension;
		return mft_Unknown;
	}

	return it->second;
}

/////////////////////////////////////////////////////////////////////////////
MediaFileTypeEnum CMediaTypeManager::DeriveMediaType(const std::string& mediaType, const std::string& fileName)
{
	const CMediaTypeManager& manager = CMediaTypeManager::instance();
	return mediaType.empty() ? manager.LookupByFileExtension(fileName) : manager.LookupByMediaType(mediaType);
}

/////////////////////////////////////////////////////////////////////////////
