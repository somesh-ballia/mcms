#ifndef API_XML_PARSER_H__
#define API_XML_PARSER_H__

///////////////////////////////////////////////////////////////////////////
#include <libxml/parser.h>
#include <libxml/tree.h>

///////////////////////////////////////////////////////////////////////////
class XmlParser
{
public:

	virtual ~XmlParser()
	{ xmlFreeDoc(doc_); }

	// parse file
	XmlParser(const std::string& file_name)
		: doc_(xmlReadFile(file_name.c_str(), NULL, 0))
	{}

	// parse in-memory buffer
	XmlParser(const char* xmlBuf, size_t size)
		: doc_(xmlReadMemory(xmlBuf, size, NULL, NULL/*"UTF-8"*/, 0))
	{}

	bool operator !() const
	{ return !doc_; }

	operator const xmlNode*() const
	{ return xmlDocGetRootElement(doc_); }

private:

	xmlDocPtr doc_;
};

///////////////////////////////////////////////////////////////////////////
#endif // API_XML_PARSER_H__
