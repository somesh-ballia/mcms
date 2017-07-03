#include "ApiBaseObjectPtr.h"

#include "ApiXmlParser.h"
#include "ApiXmlEncoder.h"

#include "Segment.h"

#include "Trace.h"
#include "TraceStream.h"

///////////////////////////////////////////////////////////////////////////
#include <map>

#include <fstream>
#include <string.h>

///////////////////////////////////////////////////////////////////////////
ApiEncodersFactory::ApiEncodersFactory()
{
	registerEncoder<ApiObjectXmlEncoder>("xml");
	registerEncoder<ApiObjectXmlEncoder>("application/xml");
}

///////////////////////////////////////////////////////////////////////////
std::auto_ptr<IApiObjectEncoder> ApiEncodersFactory::encoder(const char* codec) const
{
	Map::const_iterator it = map_.find(CLexeme(codec));

	return std::auto_ptr<IApiObjectEncoder>(it != map_.end() ? (*it->second)() : NULL);
}

///////////////////////////////////////////////////////////////////////////
bool ApiBaseObject::ReadFromXmlStream(const char* xmlBuf, size_t size)
{
	XmlParser p(xmlBuf, size);
	FPASSERTMSG_AND_RETURN_VALUE(!p, "Failed to parse XML", false);

	ReadFromXml(p);
	return true;
}

///////////////////////////////////////////////////////////////////////////
bool ApiBaseObject::ReadFromXmlFile(const std::string& file_path)
{
	XmlParser p(file_path);
	FPASSERTSTREAM_AND_RETURN_VALUE(!p, "Failed to open and parse XML file:" << file_path, false);

	ReadFromXml(p);
	return true;
}

///////////////////////////////////////////////////////////////////////////
bool ApiBaseObject::WriteToXmlFile(const std::string& file_path, bool bOverride)
{
	std::ifstream ifs;
	ifs.open(file_path.c_str(), std::ios::in);

	if (ifs)
	{
		ifs.close();

		if (!bOverride)
			return false;
	}

	std::ofstream ofs;
	ofs.open(file_path.c_str(), std::ios::trunc);

	if (!ofs)
		return false;

	ofs << *this;
	return true;
}

///////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& os, const ApiBaseObject& obj)
{
	if (!obj.IsAssigned())
		return os;

	std::auto_ptr<IApiObjectEncoder> pEncoder(ApiEncodersFactory::const_instance().encoder("xml"));

	IApiObjectEncoder& encoder = *pEncoder.get();

	encoder.startElement(obj.objectTag(), obj.objectNsPrefix());
	obj.WriteTo(encoder);
	encoder.endElement();

	return os << reinterpret_cast<const char*>(encoder.buffer());
}

///////////////////////////////////////////////////////////////////////////
void ApiBaseObject::InvokeInitHelper(ReadersMeta::const_iterator it, const char* initializer, const xmlNode* node, ApiBaseObject* pThis, bool initDefaults)
{
	if (!initDefaults && !initializer && !node)
		return;

	const DecoderFuncPtr& funcPtr = it->second.funcPtr;

	MemberPtr memberPtr = it->second.var;
	void* varPtr = &(pThis->*memberPtr);

	(*funcPtr)(varPtr, initializer, node);
}

///////////////////////////////////////////////////////////////////////////
void ApiBaseObject::HandleReadXmlAttributes(const ReadersMeta& map, const xmlNode* node, ApiBaseObject* pThis, bool initDefaults/* = true*/)
{
	ReadersMeta::const_iterator end = map.end();

	for (xmlAttr* item = node->properties; item; item = item->next)
	{
		const char* ns = (item->ns && item->ns->href) ? reinterpret_cast<const char*>(item->ns->href) : NULL;
		ReadersMeta::const_iterator it = map.find(std::make_pair(reinterpret_cast<const char*>(item->name), ns));

		if (it != end)
		{
			const char* initializer = item->children ? reinterpret_cast<const char*>(item->children->content) : NULL;
			InvokeInitHelper(it, initializer, NULL, pThis, initDefaults);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void ApiBaseObject::HandleReadXmlElements(const ReadersMeta& map, const xmlNode* node, ApiBaseObject* pThis, bool initDefaults/* = true*/)
{
	ReadersMeta::const_iterator end = map.end();

	for (xmlNode* item = node->children; item; item = item->next)
	{
		if (XML_ELEMENT_NODE != item->type)
			continue;

		const char* ns = (item->ns && item->ns->href) ? reinterpret_cast<const char*>(item->ns->href) : NULL;
		ReadersMeta::const_iterator it = map.find(std::make_pair(item->name, ns));

		if (it == end)
			it = map.find(std::make_pair(CLexeme(), CLexeme()));

		if (it != end)
		{
			const char* initializer;

			switch (it->second.source)
			{
			case ds_member:
				initializer = NULL;
				break;

			case ds_choice:
				initializer = reinterpret_cast<const char*>(item->name);
				break;

			case ds_text:
				initializer = item->children ? reinterpret_cast<const char*>(item->children->content) : NULL; // might be NULL only when there's no text data
				break;

			default:
				FPASSERT(it->second.source);
				continue;
			}

			InvokeInitHelper(it, initializer, (ds_text != it->second.source ? item : NULL), pThis, initDefaults);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
CSegment& ApiBaseObjectPtr::WriteTo(CSegment& seg) const
{
	seg << ApiTypeBase(IsAssigned());

	if (IsAssigned())
	{
		seg << ptr_->objectFactory()->id() << ptr_->objectId();
		ptr_->WriteTo(seg);
	}

	return seg;
}

///////////////////////////////////////////////////////////////////////////
CSegment& ApiBaseObjectPtr::ReadFrom(CSegment& seg)
{
	destroy();

	ApiTypeBase base;
	seg >> base;

	if (base.IsAssigned())
	{
		std::string factoryId;
		size_t objectId = -1;
		seg >> factoryId >> objectId;

		create(factoryId, objectId);
		FPASSERTSTREAM_AND_RETURN_VALUE(!ptr_, "No object registered with ID '" << objectId << "' for factory '" << factoryId << "'", seg);

		if (ptr_)
			ptr_->ReadFrom(seg);
	}

	return seg;
}

///////////////////////////////////////////////////////////////////////////
