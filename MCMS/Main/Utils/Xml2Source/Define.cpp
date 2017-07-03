#include "Define.h"

#include "Error.h"

#include "XLang.h"

///////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////
Define::Define(Context& context)
	: IParserEntityInfo(context)
	, has_value_()
	, has_name_()
	, isTypeDef_()
{}

///////////////////////////////////////////////////////////////////////////
void Define::initFromXml(const xmlNode* node)
{
	node_ = node;

	ErrorMessage e(ctx, error_, false);

	isTypeDef_ = (node->name == xlang_TYPEDEF);

	for (xmlAttr* attr = node->properties; attr; attr = attr->next)
	{
		if (attr->name == xlang_name)
		{
			has_name_ = true;
			name_ = (const char*)attr->children->content;

			if (SourceFile::isValidName(name_))
			{
				if (isTypeDef_)
					name_ = SourceFile::toName(name_, true);
				else
					name_ = SourceFile::toDefine(name_);
			}
			else
				e.add(sntx_Bad_Attribute_Value, node, attr);
		}

		else if (attr->name == xlang_value)
		{
			has_value_ = true;
			value_ = (const char*)attr->children->content;

			bool ok = isTypeDef_ ? SourceFile::isValidName(value_) : !value_.empty();

			if (!ok)
				e.add(sntx_Bad_Attribute_Value, node, attr);
		}

		else if (attr->name == xlang_description)
			description_ = (const char*)attr->children->content;

		else
			e.add(sntx_Unsupported_Attribute, node, attr);
	}

	if (!has_name_)
		e.add(sem_Required_Attribute, node, "name");

	if (!has_value_)
		e.add(sem_Required_Attribute, node, "value");
}

///////////////////////////////////////////////////////////////////////////
void Define::dump_header(SourceFile& os) const
{
	if (!error_.empty())
		os << error_;

	if (!description_.empty())
		os << "// " << description_ << "\n";

	if (isTypeDef_)
		os << "typedef " << value_ << " " << SourceFile::toName(name_) << ";\n";
	else
		os << "#define " << ConvertDefine(name_) << " " << value_ << "\n";
}

///////////////////////////////////////////////////////////////////////////
std::string Define::ConvertDefine(const std::string& name)
{ return SourceFile::toDefine(name); }

///////////////////////////////////////////////////////////////////////////
