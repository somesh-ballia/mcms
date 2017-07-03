#ifndef DEFINE_H__
#define DEFINE_H__

#include "Common.h"

#include "SourceFile.h"

#include <string>

#include <libxml/tree.h>

///////////////////////////////////////////////////////////////////////////
class Define : public IParserEntityInfo
{
public:

	static std::string ConvertDefine(const std::string& name);

public:

	Define(Context& context);

public:

	virtual void initFromXml(const xmlNode* node);

	virtual void dump(DumpContext& dc) const
	{ dump_header(*dc.header_); }

private:

	void dump_header(SourceFile& os) const;

private:

	std::string value_;

	bool has_value_;
	bool has_name_;

	bool isTypeDef_;
};

///////////////////////////////////////////////////////////////////////////
#endif // DEFINE_H__
