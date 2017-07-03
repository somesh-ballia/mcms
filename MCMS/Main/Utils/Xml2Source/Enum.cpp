#include "Enum.h"

///////////////////////////////////////////////////////////////////////////
#include "Error.h"

#include "XLang.h"

///////////////////////////////////////////////////////////////////////////
#include <string.h>

///////////////////////////////////////////////////////////////////////////
Enum::Enum(Context& context)
	: IParserEntityInfo(context)
{}

///////////////////////////////////////////////////////////////////////////
void Enum::initFromXml(const xmlNode* node)
{
	node_ = node;

	ErrorMessage e(ctx, error_, false);

	const char* infix = "_";

	for (xmlAttr* attr = node->properties; attr; attr = attr->next)
	{
		if (attr->name == xlang_name)
		{
			SourceFile::tagToName((const char*)attr->children->content, name_, true);

			if (!SourceFile::isValidName(name_))
				e.add(sntx_Bad_Attribute_Value, node, attr);
		}

		else if (attr->name == xlang_base_value || attr->name == xlang_default)
			base_value_ = (const char*)attr->children->content;

		else if (attr->name == xlang_notation)
		{
			if (0 == strcmp((const char*)attr->children->content, "simple"))
				infix = "";
		}

		else if (attr->name == xlang_description)
			description_ = (const char*)attr->children->content;

		else
			e.add(sntx_Unsupported_Attribute, node, attr);
	}

	if (name_.empty())
		e.add(sem_Required_Attribute, node, "name");

	if (!description_.empty())
		SourceFile::unescape(description_);

	for (xmlNode* value = node->children; value; value = value->next)
	{
		if (value->type != XML_ELEMENT_NODE)
			continue;

		if (value->name == xlang_VALUE)
		{
			EnumValue v;

			ErrorMessage e(ctx, v.error_);

			for (xmlAttr* attr = value->properties; attr; attr = attr->next)
			{
				if (attr->name == xlang_name)
				{
					v.name_ = (const char*)attr->children->content;

					if (!SourceFile::isValidName(v.name_))
						e.add(sntx_Bad_Attribute_Value, value, attr);
				}

				else if (attr->name == xlang_tag)
				{
					v.t_assigned_ = true;
					v.tag_ = (const char*)attr->children->content;

					if (v.tag_.empty())
						e.add(sntx_Bad_Attribute_Value, value, attr);
				}

				else if (attr->name == xlang_description)
					v.description_ = (const char*)attr->children->content;

				else
					e.add(sntx_Unsupported_Attribute, value, attr);
			}

			if (!v.t_assigned_)
				e.add(sem_Required_Attribute, value, "tag");

			if (v.name_.empty())
				v.name_ = v.tag_;

			SourceFile::tagToName(v.name_, v.name_, true);
			v.name_ = "e" + name_ + infix + v.name_;
			values_.push_back(v);
		}

		else
			e.add(sntx_Unsupported_Declarator, value, NULL);
	}

	if (values_.empty())
		e.add(sem_Unsupported_Feature, node, "Empty enumerations are not valid");
}

///////////////////////////////////////////////////////////////////////////
const std::string Enum::enumValueName(const std::string& enum_type_name, const std::string& value_name)
{ return "e" + enum_type_name + "_" + SourceFile::toName(value_name, true); }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::enumDefaultValue(const std::string& enum_type_name)
{ return "e" + enum_type_name + "_DEFAULT"; }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::enumDefaultValue() const
{ return "e" + name_ + "_DEFAULT"; }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::enumLastValue() const
{ return "e" + name_ + "_LAST"; }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::stringsArrayName() const
{ return name_ + "_strings"; }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::methodName_toString() const
{ return "to_string"; }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::signature_toString() const
{ return "const char* " + methodName_toString() + "(" + name_ + " val)"; }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::methodName_toTag() const
{ return "to_tag"; }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::signature_toTag() const
{ return "const std::string " + methodName_toTag() + "(" + name_ + " val)"; }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::methodName_toEnum() const
{ return name_ + "_to_enum"; }

///////////////////////////////////////////////////////////////////////////
const std::string Enum::signature_toEnum() const
{ return name_ + " " + methodName_toEnum() + "(const char* val)"; }

///////////////////////////////////////////////////////////////////////////
void Enum::dump_header(SourceFile& os) const
{
	os.addOpenNamespace();

	SourceFile::AddSeparator(os);

	if (!error_.empty())
		os << error_;

	if (!description_.empty())
		os << "/*\n" << description_ << "\n*/\n";

	os <<
		"enum " << name_ << "\n"
		"{\n"
		"	" << enumDefaultValue();

	if (!base_value_.empty())
		os << " = " << base_value_;

	os <<
		",\n"
		"\n";

	for (std::list<EnumValue>::const_iterator it = values_.begin(); it != values_.end(); ++it)
	{
		if (!it->error_.empty())
			os << it->error_;

		os << "	" << it->name_ << ',';

		if (!it->description_.empty())
			os << " // " << it->description_;

		os << '\n';
	}

	os <<
		"\n" << "	" << enumLastValue() << "\n"
		"};\n";

	SourceFile::AddSeparator(os);

	os <<
		signature_toString() << ";\n" <<
		signature_toTag() << ";\n" <<
		signature_toEnum() << ";\n";

	SourceFile::AddSeparator(os);

	os <<
		"CSegment& operator <<(CSegment& seg, " << name_ << " val);\n"
		"CSegment& operator >>(CSegment& seg, " << name_ << "& val);\n";

	SourceFile::AddSeparator(os);

	os <<
		"inline std::ostream& operator <<(std::ostream& os, " << name_ << " val)\n"
		"{ return os << " << methodName_toString() << "(val); }\n";

	os.addCloseNamespace();

	SourceFile::AddSeparator(os);

	const std::string prefix(ctx.api_ns.empty() ? "" : ctx.api_ns + "::");

	os <<
		"template <>\n"
		"inline void ValueFormatter<" << prefix << name_ << ">::get(ParamType obj, std::string& value)\n"
		"{ value = " << prefix << methodName_toTag() << "(obj); }\n";

	SourceFile::AddSeparator(os);

	os <<
		"template <>\n"
		"inline void ApiTypeHelper<" << prefix << name_ << ">::InitHelper(void* var, const char* initializer, const xmlNode* /*node*/)\n"
		"{\n"
		"	ValueType& obj = *reinterpret_cast<ValueType*>(var);\n"
		"	obj = initializer ? " << prefix << methodName_toEnum() << "(reinterpret_cast<const char*>(initializer)) : " << prefix << enumDefaultValue() << ";\n"
		"}\n";

	SourceFile::AddSeparator(os);

	os <<
		"template <>\n"
		"inline bool ApiTypeHelper<" << prefix << name_ << ">::IsAssigned(ParamType val)\n"
		"{ return " << prefix << enumDefaultValue() << " != val; }\n";

	SourceFile::AddSeparator(os);

	os <<
		"template <>\n"
		"inline size_t ApiTypeHelper<" << prefix << name_ << ">::CurrentBinarySize(ParamType val)\n"
		"{ return sizeof(size_t); }\n";
}

///////////////////////////////////////////////////////////////////////////
void Enum::dump_source(SourceFile& os) const
{
	dump_StrArray(os);
	dump_GetStrMethods(os);

	SourceFile::AddSeparator(os);

	os <<
		"CSegment& operator <<(CSegment& seg, " << name_ << " val)\n"
		"{ return seg << static_cast<size_t>(val); }\n";

	SourceFile::AddSeparator(os);

	os <<
		"CSegment& operator >>(CSegment& seg, " << name_ << "& val)\n"
		"{\n"
		"	size_t t = 0;\n"
		"	seg >> t;\n"
		"\n"
		"	val = static_cast<" << name_ << ">(t);\n"
		"	return seg;\n"
		"}\n";
}

///////////////////////////////////////////////////////////////////////////
void Enum::dump_StrArray(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os <<
		"const char* " << stringsArrayName() << "[] =\n"
		"{\n"
		"	\"uninitialized_" << name_ << "\",\n";

	for (std::list<EnumValue>::const_iterator it = values_.begin(); it != values_.end(); ++it)
		os << "	\"" << it->name_ << "\",\n";

	os << "};\n";

	SourceFile::AddSeparator(os);

	os <<
		"const char* " << stringsArrayName() << "_XML[] =\n"
		"{\n"
		"	\"uninitialized " << name_ << "\",\n";

	for (std::list<EnumValue>::const_iterator it = values_.begin(); it != values_.end(); ++it)
		os << "	\"" << (!it->t_assigned_ ? it->name_ : it->tag_) << "\",\n";

	os << "};\n";
}

///////////////////////////////////////////////////////////////////////////
void Enum::dump_GetStrMethods(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os <<
		signature_toString() << "\n"
		"{\n"
		"	const char* name =\n"
		"		" << enumDefaultValue() << " <= val && val < " << enumLastValue() << "\n"
		"		" << "? " << stringsArrayName() << "[val";

	if (!base_value_.empty())
		os << " - " << enumDefaultValue();

	os <<
		"]\n"
		"		: \"Invalid value\";\n"
		"\n"
		"	return name;\n"
		"}\n";

	SourceFile::AddSeparator(os);
	os <<
		signature_toTag() << "\n"
		"{\n"
		"	if (" << enumDefaultValue() << " <= val && val < " << enumLastValue() << ")\n"
		"		return " << stringsArrayName() << "_XML[val";

	if (!base_value_.empty())
		os << " - " << enumDefaultValue();

	os <<
		"];\n"
		"\n"
		"	std::ostringstream os;\n"
		"	os << \"Invalid value: \" << static_cast<long>(val);\n"
		"	return os.str();\n"
		"}\n";

	SourceFile::AddSeparator(os);

	os <<
		signature_toEnum() << "\n"
		"{\n"
		"	for (" << name_ << " i = " << enumDefaultValue() << "; i < " << enumLastValue() << "; i = static_cast<" << name_ << ">(i + 1))\n"
		"	{\n"
		"		if (" << methodName_toTag() << "(i) == val)\n"
		"			return i;\n"
		"	}\n"
		"\n"
		"	return " << enumDefaultValue() << ";\n"
		"}\n";
}

///////////////////////////////////////////////////////////////////////////
