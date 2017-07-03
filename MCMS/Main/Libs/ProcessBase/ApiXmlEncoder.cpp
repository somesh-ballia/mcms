#include "ApiXmlEncoder.h"

///////////////////////////////////////////////////////////////////////////
ApiObjectXmlEncoder::ApiObjectXmlEncoder()
	: buf_(xmlBufferCreate())
	, writer_(xmlNewTextWriterMemory(buf_, false))
{
	xmlTextWriterSetIndent(writer_, true);
	xmlTextWriterSetIndentString(writer_, (const xmlChar*)"  ");
	xmlTextWriterStartDocument(writer_, NULL, NULL, NULL);
}

///////////////////////////////////////////////////////////////////////////
ApiObjectXmlEncoder::~ApiObjectXmlEncoder()
{
	xmlFreeTextWriter(writer_);
	xmlBufferFree(buf_);
}

///////////////////////////////////////////////////////////////////////////
