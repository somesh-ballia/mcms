#include "Member.h"

#include "Error.h"
#include "XLang.h"

///////////////////////////////////////////////////////////////////////////
#include "Enum.h"
#include "Define.h"
#include "ClassMeta.h"

#include "Generator.h"

///////////////////////////////////////////////////////////////////////////
#include <ostream>
#include <sstream>

#include <algorithm>

///////////////////////////////////////////////////////////////////////////
Member::Member(Context& context)
	: IParserEntityInfo(context)
	, reserve_(0)
	, type_(mt_Undefined)
	, container_(ct_Single)
	, has_default_(false)
	, disp_(d_Element)
{}

///////////////////////////////////////////////////////////////////////////
bool Member::isPointerType() const
{ return !!(typeMeta(type_) & tmi_isPointer); }

///////////////////////////////////////////////////////////////////////////
bool Member::isSequenceType() const
{ return !!(typeMeta(type_) & tmi_isSequence); }

///////////////////////////////////////////////////////////////////////////
bool Member::isRecordType() const
{ return !!(typeMeta(type_) & tmi_isRecord); }

///////////////////////////////////////////////////////////////////////////
bool Member::isTypeRefRequired() const
{ return !!(typeMeta(type_) & tmi_owsTypeRef); }

///////////////////////////////////////////////////////////////////////////
bool Member::isCompoundType() const
{ return !!(typeMeta(type_) & tmi_isCompound); }

///////////////////////////////////////////////////////////////////////////
bool Member::isBuiltInType() const
{ return (container_ == ct_Single) && (typeMeta(type_) & tmi_isBuilt_in); }

///////////////////////////////////////////////////////////////////////////
bool Member::isNullableType() const
{ return !!(typeMeta(type_) & tmi_isNullable); }

///////////////////////////////////////////////////////////////////////////
bool Member::isSpecialType() const
{ return !!(typeMeta(type_) & tmi_isSpecial); }

///////////////////////////////////////////////////////////////////////////
bool Member::requireNoChildNodes(const xmlNode* node, ErrorMessage& e) const
{
	for (xmlNode* child = node->children; child; child = child->next)
	{
		if (child->type == XML_ELEMENT_NODE)
		{
			e.add(sntx_Unexpected_Nested_Declarator, child, NULL);
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
void Member::initFromXml(const xmlNode* value)
{
	node_ = value;
	
	ErrorMessage e(ctx, error_);

	if (value->name == xlang_NAMESPACE)
	{
		for (xmlAttr* attr = value->properties; attr; attr = attr->next)
		{
			if (attr->name == xlang_tag)
				tag_ = (const char*)attr->children->content;

			else if (attr->name == xlang_name)
				name_ = (const char*)attr->children->content;

			else if (attr->name == xlang_default || attr->name == xlang_value)
			{
				has_default_ = true;
				default_ = (const char*)attr->children->content;

				if (default_.empty())
					e.add(sntx_Bad_Attribute_Value, value, attr);
			}

			else if (attr->name == xlang_description)
				description_ = (const char*)attr->children->content;

			else
				e.add(sntx_Unsupported_Attribute, value, attr);
		}

		type_ = mt_String;
		disp_ = d_Namespace;

		if (name_.empty())
			name_ = tag_;

		if (name_.empty())
			name_ = "default";

		if (!tag_.empty() && !has_default_)
			e.add(sem_Required_Attribute, value, "value");

		if (default_.empty() && !has_default_)
			default_ = "0";

		ns_ = ctx.api_name + "_ns";

		requireNoChildNodes(value, e);
	}

	else if (value->name == xlang_MEMBER || value->name == xlang_PROPERTY)
	{
		disp_ = value->name == xlang_PROPERTY ? d_Attribute : d_Element;

		for (xmlAttr* attr = value->properties; attr; attr = attr->next)
		{
			if (attr->name == xlang_name)
			{
				name_ = (const char*)attr->children->content;

				if (!SourceFile::isValidName(name_))
					e.add(sntx_Bad_Attribute_Value, value, attr);
			}

			else if (attr->name == xlang_ns)
			{
				ns_ = (const char*)attr->children->content;

				if (ns_.empty())
					e.add(sntx_Bad_Attribute_Value, value, attr);
			}

			else if (attr->name == xlang_key)
			{
				if (d_Attribute == disp_)
					e.add(sem_Illegal_Attribute, value, (const char*)attr->name);
				
				else if (SourceFile::isValidName((const char*)attr->children->content))
					SourceFile::tagToName((const char*)attr->children->content, key_);

				else
					e.add(sntx_Bad_Attribute_Value, value, attr);
			}

			else if (attr->name == xlang_reserve)
			{
				if (d_Attribute == disp_)
					e.add(sem_Illegal_Attribute, value, (const char*)attr->name);

				else
				{
					reserve_ = atol((const char*)attr->children->content);

					if (!reserve_)
						e.add(sntx_Bad_Attribute_Value, value, attr);
				}
			}

			else if (attr->name == xlang_container)
			{
				if (d_Attribute == disp_)
					e.add(sem_Illegal_Attribute, value, (const char*)attr->name);

				container_ = ContainerFromName((const char*)attr->children->content);

				if (ct_Unknown == container_)
					e.add(sntx_Bad_Attribute_Value, value, attr);
			}

			else if (attr->name == xlang_type)
			{
				type_ = TypeFromName((const char*)attr->children->content);

				if (mt_Unknown == type_)
					e.add(sntx_Bad_Attribute_Value, value, attr);

				else if (d_Attribute == disp_ && (isCompoundType() || isContainerType()))
				{
					e.add(sem_Unsupported_Base_Type, value, (const char*)attr->children->content);
					type_ = mt_Unknown;
				}
			}

			else if (attr->name == xlang_description)
				description_ = (const char*)attr->children->content;

			else if (attr->name == xlang_type_ref)
			{
				if (!isTypeRefRequired())
					e.add(sem_Illegal_Attribute, value, (const char*)attr->name);

				else
				{
					SourceFile::tagToName((const char*)attr->children->content, type_ref_, true);

					if (!SourceFile::isValidName(type_ref_))
						e.add(sntx_Bad_Attribute_Value, value, attr);
				}
			}

			else if (attr->name == xlang_tag)
			{
				tag_ = (const char*)attr->children->content;

				if (!SourceFile::isValidTag(tag_))
					e.add(sntx_Bad_Attribute_Value, value, attr);
			}

			else if (attr->name == xlang_default)
			{
				has_default_ = true;
				default_ = (const char*)attr->children->content;
			}

			else
				e.add(sntx_Unsupported_Attribute, value, attr);
		}

		if (has_default_ && isCompoundType())
			e.add(sem_Unsupported_Feature, value, "Default value is allowed for simple types only");

		if (!key_.empty() && container_ < ct_Map)
			e.add(sem_Illegal_Indexed_Access, value, containerName(container_));

		if (container_ >= ct_Map)
		{
			if (!isCompoundType())
				e.add(sem_Unsupported_Feature, value, "Map containers are supported for compound types only, use 'set' or 'multiset' instead");

			else if (key_.empty() && !isPointerType())
				e.add(sem_Required_Attribute, value, "key");
		}
		else if (container_ >= ct_Set)
		{
			if (isCompoundType())
				e.add(sem_Unsupported_Feature, value, "Set containers are supported for simple types only, use 'map' or 'multimap' instead");
		}

		if (reserve_ && ct_Vector != container_ && mt_Handle != type_)
			e.add(sem_Illegal_Space_Reservation, value, containerName(container_));

		if (mt_Undefined == type_)
			e.add(sem_Required_Attribute, value, "type");

		if (isTypeRefRequired() && type_ref_.empty())
			e.add(sem_Required_Attribute, value, "type_ref");

		if (isSequenceType() && !isContainerType())
			e.add(sem_Required_Attribute, value, "container");

		if (isPointerType())
		{
			if (name_.empty())
				e.add(sem_Required_Attribute, value, "name");

			if (container_ > ct_Vector)
				e.add(sem_Unsupported_Feature, value, "Unsupported container type for choice-like types");

			if (!ns_.empty())
				e.add(sem_Illegal_Attribute, value, "ns");

			if (!tag_.empty())
				e.add(sem_Illegal_Attribute, value, "tag");
		}

		else if (tag_.empty() && name_.empty())
			e.add(sem_Unsupported_Feature, value, "At least one of 'tag' or 'name' is required");

		else if (name_.empty())
			name_ = tag_;

		else if (tag_.empty())
			SourceFile::nameToTag(name_, tag_);

		if (isChoiceType())
		{
			if (!initChoicesFromXml(value, e))
				e.add(sem_Unsupported_Feature, value, "Unspecified choice alternatives");
		}
		else
			requireNoChildNodes(value, e);
	}

	else
		e.add(sntx_Unsupported_Declarator, value, NULL);

	if (hasDefaultValue() || mt_Enum == type_)
	{
		switch (type())
		{
			case mt_String:
			case mt_String_opt:
				default_ = "\"" + default_ + "\"";
				break;

			case mt_Enum:
				default_ = default_.empty() ? Enum::enumDefaultValue(type_ref_) : Enum::enumValueName(type_ref_, default_);
				has_default_ = true;
				break;

			default:
				default_ = Define::ConvertDefine(default_);
				break;
		}
	}

	if (type_ > mt_Unknown)
		SourceFile::tagToName(name_, name_);

	sync();
}

///////////////////////////////////////////////////////////////////////////
bool Member::initChoicesFromXml(const xmlNode* node, ErrorMessage& e)
{
	bool ok = false;

	for (xmlNode* choice = node->children; choice; choice = choice->next)
	{
		if (choice->type != XML_ELEMENT_NODE)
			continue;

		if (choice->name == xlang_MEMBER || choice->name == xlang_CHOICE)
		{
			ok = true;
			bool assigned = false;

			ChoiceMeta c(choice);

			for (xmlAttr* attr = choice->properties; attr; attr = attr->next)
			{
				if (attr->name == xlang_type_ref)
				{
					assigned = true;
					c.type_ref_ = (const char*)attr->children->content;
				}
#if 0
				else if (attr->name == xlang_tag)
				{
					c.tag_ = (const char*)attr->children->content;

					if (!SourceFile::isValidTag(c.tag_))
						e.add(sntx_Bad_Attribute_Value, choice, attr);
				}

				else if (attr->name == xlang_ns)
				{
					c.ns_ = (const char*)attr->children->content;

					if (!SourceFile::isValidTag(c.ns_))
						e.add(sntx_Bad_Attribute_Value, choice, attr);
				}
#endif
				else
				{
					e.add(sntx_Unsupported_Attribute, choice, attr);
					continue;
				}

				if (!*attr->children->content)
					e.add(sntx_Bad_Attribute_Value, choice, attr);
			}

			if (assigned)
			{
				SourceFile::tagToName(c.type_ref_, c.type_ref_, true);

				std::pair<Member::ChoicesList::iterator, bool> x = choice_type_refs_.insert(c);

				if (!x.second)
				{
					e.add(sem_Choice_Already_Defined, choice, c.type_ref_.c_str());
					e.add(sem_Refer_To_Definition, x.first->node(), x.first->type_ref_.c_str());
				}
			}
			else
				e.add(sem_Required_Attribute, choice, "type_ref");

			requireNoChildNodes(choice, e);
		}

		else
			e.add(sntx_Unsupported_Declarator, choice, NULL);
	}

	return ok;
}

///////////////////////////////////////////////////////////////////////////
void Member::sync()
{
	const bool isHungarian = ctx.notation == Context::n_Hungarian;

	if (isHungarian)
		member_name_ = "m_";

	if (isPointerType() && container_ == ct_Single && isHungarian)
	{
		member_name_ += 'p';
		member_name_ += SourceFile::toName(name_, true);
	}
	else
		member_name_ += name_;

	if (!isHungarian)
		member_name_ += '_';

	///////////////////////////////////////////////////////////////////////////
	if (isTypeRefRequired())
		type_name_ = type_ref_;

	const char* suffix = ::typeName(type_);

	if (suffix)
		type_name_ += suffix;

	if (container_ > ct_Single)
	{
		type_ref_ = type_name_;
		type_name_ = SourceFile::toName(name_, true) + "Container";
	}
}

///////////////////////////////////////////////////////////////////////////
const char* Member::dataSource(bool write) const
{
	switch (disp_)
	{
	case d_Attribute:
		return "ds_attr";

	case d_Element:
		if (write)
			return (isContainerType() || isCompoundType()) ? "ds_member" : "ds_text";

		return isPointerType() ? "ds_choice" : (isCompoundType() ? "ds_member" : "ds_text");

	default:
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////
void Member::dump_header_ns(SourceFile& os) const
{
	if (!error_.empty())
		os << error_;

	if (!tag_.empty())
		os << "	static const utfChar* tag_" << name_ << ";\n";

	os << "	static const utfChar* urn_" << name_ << ';';

	if (!description_.empty())
		os << " // " << description_;

	os << "\n\n";
}

///////////////////////////////////////////////////////////////////////////
void Member::dump_source_ns(SourceFile& os) const
{
	if (!tag_.empty())
		os << "const utfChar* " << ns_ << "::tag_" << name_ << " = (const utfChar*)\"" << tag_ << "\";\n";

	os << "const utfChar* " << ns_ << "::urn_" << name_ << " = (const utfChar*)" << defaultValue() << ";\n\n";
}

///////////////////////////////////////////////////////////////////////////
void Member::dump_definition(SourceFile& os) const
{
	if (!error_.empty())
		os << '\n' << error_;

	if (name_.empty())
		return;

	std::string error;
	ErrorMessage e(ctx, error);

	const EntityDB* edb = ctx.db.get(type_);
	EntityDB::const_iterator it;

	if (edb)
	{
		const std::string& type_name(ct_Single == container_ ? type_name_ : type_ref_);

		it = edb->find(type_name);

		if (it == edb->end())
			e.add(sem_Undefined_Type_Reference, node_, type_name.c_str());
	}

	if (!error.empty())
	{
		os << '\n' << error;
		error.clear();
	}

	os << '	';

	if (container_ > ct_Single)
	{
		os << "typedef std::" << container_ << '<';

		if (!key_.empty())
		{
			if (&ctx.db.classes == edb && it != edb->end())
			{
				const ClassMeta::MembersList& members = ((const ClassMeta*)it->second)->members();

				MemberFindPredicate m(key_);
				ClassMeta::MembersList::const_iterator it_member = std::find_if(members.begin(), members.end(), m);

				if (it_member == members.end())
				{
					e.add(sem_Undefined_Name_Reference, node_, key_.c_str());
					e.add(sem_Refer_To_Definition, it->second->node(), type_name_.c_str());
					os << ::typeName(mt_Unknown);
				}
				else
				{
					const Member* m = *it_member;
					os << m->typeName();

					if (m->isCompoundType())
					{
						e.add(sem_Unsupported_Feature, node_, "Key must refer to a member of a simple type only");
						e.add(sem_Refer_To_Definition, m->node(), m->name().c_str());
					}
				}
			}
			else
				os << ::typeName(mt_String); // ???

			os << ", ";
		}

		os << type_ref_ << "> " << type_name_ << ";";

		if (!error.empty())
			os << '\n' << error;

		os << "\n	";
	}

	os << type_name_ << ' ' << member_name_ << ';';

	if (!description_.empty())
		os << " // " << description_;

	os << '\n';

	if (!error_.empty())
		os << '\n';
}

///////////////////////////////////////////////////////////////////////////
