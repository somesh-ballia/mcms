#ifndef MCCFCODEC_H__
#define MCCFCODEC_H__

#include <string>

/////////////////////////////////////////////////////////////////////////////
class ApiBaseObject;

/////////////////////////////////////////////////////////////////////////////
struct CMccfXmlCodec
{
	static void Encode(const ApiBaseObject& obj, std::string& out, bool pretty = true);
	static bool Decode(const char* str, size_t size, ApiBaseObject& obj);

	static const char* mimeType;
	static const char* mimeSubtype;
};

/////////////////////////////////////////////////////////////////////////////
#endif // MCCFCODEC_H__
