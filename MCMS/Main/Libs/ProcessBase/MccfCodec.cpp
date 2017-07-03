#include "MccfCodec.h"

#include "ApiBaseObject.h"

#include <sstream>


///////////////////////////////////////////////////////////////////////////
const char* CMccfXmlCodec::mimeType = "application";
const char* CMccfXmlCodec::mimeSubtype = "xml";

///////////////////////////////////////////////////////////////////////////
void CMccfXmlCodec::Encode(const ApiBaseObject& obj, std::string& out, bool pretty/* = true*/)
{
	std::ostringstream os;
	os << obj;
	out = os.str();
}

///////////////////////////////////////////////////////////////////////////
bool CMccfXmlCodec::Decode(const char* s, size_t size, ApiBaseObject& obj)
{
	return obj.ReadFromXmlStream(s, size);
}

///////////////////////////////////////////////////////////////////////////
