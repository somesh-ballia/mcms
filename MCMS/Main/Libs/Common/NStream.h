#ifndef N_STREAM_H__
#define N_STREAM_H__

///////////////////////////////////////////////////////////////////////////
#include "PObject.h"
#include "Segment.h"

///////////////////////////////////////////////////////////////////////////
#include <sstream>
#include <string>

///////////////////////////////////////////////////////////////////////////
using namespace std; // TODO: eliminate this line later!!!

///////////////////////////////////////////////////////////////////////////
class COstrStream : public CPObject, public std::ostringstream
{
	CLASS_TYPE_1(COstrStream, CPObject)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	virtual ~COstrStream();

	COstrStream();
	COstrStream(const char* buffer);

	void Serialize(CSegment& seg) const
	{ seg << this->str(); }

	void Deserialize(CSegment& seg);
};

///////////////////////////////////////////////////////////////////////////
class CIstrStream : public CPObject, public std::istringstream
{
	CLASS_TYPE_1(CIstrStream, CPObject)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	virtual ~CIstrStream();

	CIstrStream();
	CIstrStream(const char* buffer);
	CIstrStream(const std::string& s);
	CIstrStream(CSegment& seg);

	void Serialize(CSegment& seg) const
	{ seg << str(); }
};

///////////////////////////////////////////////////////////////////////////
#endif // N_STREAM_H__
