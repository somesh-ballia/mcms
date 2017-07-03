#ifndef API_XML_ENCODER_H__
#define API_XML_ENCODER_H__

///////////////////////////////////////////////////////////////////////////
#include "ApiEncoder.h"

///////////////////////////////////////////////////////////////////////////
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

///////////////////////////////////////////////////////////////////////////
class ApiObjectXmlEncoder : public IApiObjectEncoder
{
public:

	ApiObjectXmlEncoder();

	virtual ~ApiObjectXmlEncoder();

	virtual bool addNamespace(const utfChar* href, const utfChar* prefix = NULL)
	{
		return 0 <=
			(prefix && *prefix ?
				xmlTextWriterWriteAttributeNS(writer_, (const xmlChar*)"xmlns", prefix, NULL, href) :
				xmlTextWriterWriteAttributeNS(writer_, NULL, (const xmlChar*)"xmlns", NULL, href)
			);
	}

	virtual bool startElement(const utfChar* tag, const utfChar* prefix = NULL)
	{ return xmlTextWriterStartElementNS(writer_, prefix, tag, NULL) >= 0; }

	virtual bool endElement()
	{ return xmlTextWriterEndElement(writer_) >= 0; }

	virtual bool addElement(const utfChar* tag, const utfChar* value, const utfChar* prefix = NULL)
	{
		bool ok = xmlTextWriterWriteElementNS(writer_, prefix, tag, NULL, value) >= 0;

		if (!value)
			xmlTextWriterEndElement(writer_);

		return ok;
	}

	virtual bool addAttribute(const utfChar* tag, const utfChar* value, const utfChar* prefix = NULL)
	{ return xmlTextWriterWriteAttributeNS(writer_, prefix, tag, NULL, value) >= 0; }

	virtual bool addComment(const utfChar* text)
	{ return xmlTextWriterWriteComment(writer_, text) >= 0; }

	virtual const utfChar* buffer() const
	{
		xmlTextWriterEndDocument(writer_);
		return buf_->content;
	}

private:

	xmlBufferPtr buf_;
	xmlTextWriterPtr writer_;
};

///////////////////////////////////////////////////////////////////////////
#endif // API_XML_ENCODER_H__
