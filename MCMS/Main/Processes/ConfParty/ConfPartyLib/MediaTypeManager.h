#ifndef MEDIATYPEMANAGER_H__
#define MEDIATYPEMANAGER_H__

/////////////////////////////////////////////////////////////////////////////
#include "Singleton.h"

#include "Tokenizer.h"
#include "MediaTypes.h"

#include <map>

/////////////////////////////////////////////////////////////////////////////
class CMediaTypeManager : public SingletonHolder<CMediaTypeManager>
{
	friend class SingletonHolder<CMediaTypeManager>; // provide access to non-public constructor

public:

	static MediaFileTypeEnum DeriveMediaType(const std::string& mediaType, const std::string& fileName);

public:

	MediaFileTypeEnum LookupByMediaType(const std::string& mediaType) const;
	MediaFileTypeEnum LookupByFileExtension(const std::string& fileName) const;

private:

	CMediaTypeManager();

private:

	typedef std::map<CLexeme, MediaFileTypeEnum> MediaTypesMap;
	MediaTypesMap map_;
};

/////////////////////////////////////////////////////////////////////////////
#endif // MEDIATYPEMANAGER_H__
