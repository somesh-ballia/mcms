#include "ClassMeta.h"

///////////////////////////////////////////////////////////////////////////
#include "Error.h"

#include "XLang.h"

///////////////////////////////////////////////////////////////////////////
#include "SourceFile.h"
#include "Generator.h"

///////////////////////////////////////////////////////////////////////////
#include "Enum.h"

///////////////////////////////////////////////////////////////////////////
#include <string>
#include <set>

///////////////////////////////////////////////////////////////////////////
std::string ClassMeta::GetParentClass() const
{ return base_object_.empty() ? "ApiBaseObject" : base_object_; }

///////////////////////////////////////////////////////////////////////////
const Member* ClassMeta::findNs(const std::string& prefix, const xmlNode* node) const
{
	const EntityDB* edbs[] = { &namespaces_, &ctx.db.namespaces };

	for (size_t i = 0; i < sizeof(edbs)/sizeof(edbs[0]); ++i)
	{
		const EntityDB* edb = edbs[i];
		EntityDB::const_iterator ns_it =  edb->find(prefix);

		if (ns_it != edb->end())
			return reinterpret_cast<Member*>(ns_it->second);
	}

	if (!prefix.empty())
	{
		std::string error;
		ErrorMessage e(ctx, error);

		e.add(sem_Undefined_Name_Reference, node, prefix.c_str());
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::initFromXml(const xmlNode* node)
{
	node_ = node;

	ErrorMessage e(ctx, error_, false);

	for (xmlAttr* attr = node->properties; attr; attr = attr->next)
	{
		if (attr->name == xlang_name)
		{
			SourceFile::tagToName((const char*)attr->children->content, name_, true);

			if (!SourceFile::isValidName(name_))
				e.add(sntx_Bad_Attribute_Value, node, attr);
		}

		else if (attr->name == xlang_tag)
		{
			tag_ = (const char*)(attr->children->content);

			if (!SourceFile::isValidTag(tag_))
				e.add(sntx_Bad_Attribute_Value, node, attr);
		}

		else if (attr->name == xlang_ns)
			ns_ = (const char*)(attr->children->content);

		else if (attr->name == xlang_extend)
		{
			SourceFile::tagToName((const char*)(attr->children->content), base_object_, "true");

			if (!SourceFile::isValidName(base_object_))
				e.add(sntx_Bad_Attribute_Value, node, attr);
		}

		else if (attr->name == xlang_root)
			is_root_ = (0 == strcmp((const char*)(attr->children->content), "true"));

		else if (attr->name == xlang_description)
			description_ = (const char*)(attr->children->content);

		else
			e.add(sntx_Unsupported_Attribute, node, attr);
	}

	if (!description_.empty())
		SourceFile::unescape(description_);

	if (is_root_ && tag_.empty())
		e.add(sem_Required_Attribute, node, "tag");

	if (name_.empty())
	{
		if (tag_.empty())
			e.add(sem_Unsupported_Feature, node, "At least one of 'name' or 'tag' must be specified");
		else
			SourceFile::tagToName(tag_, name_, true);
	}

	if (tag_.empty())
		SourceFile::nameToTag(name_, tag_);

	ctx.xmlNames.insert(std::make_pair(name_, std::make_pair(tag_, ns_))); // it is used only to support the [[ type="choice" ]]

	const std::string ns_class_name(name_ + "_ns");

	MembersList properties;

	for (xmlNode* value = node->children; value; value = value->next)
	{
		if (value->type != XML_ELEMENT_NODE)
			continue;

		Member* m = new Member(ctx);
		m->initFromXml(value);

		switch (m->disp_)
		{
		case Member::d_Attribute:
			properties.push_back(m);
			break;

		case Member::d_Element:
			members_.push_back(m);
			break;

		case Member::d_Namespace:
			if (is_root_)
			{
				m->ns_ = ns_class_name;
				std::pair<EntityDB::iterator, bool> status = namespaces_.insert(std::make_pair(m->tag_, m));

				if (!status.second)
				{
					e.add(sem_Name_Already_Defined, m->node(), m->tag_.c_str());

					const IParserEntityInfo* i = status.first->second;
					e.add(sem_Refer_To_Definition, i->node(), i->name().c_str());
				}
			}
			else
				e.add(sem_Unsupported_Feature, value, "Namespace declaration is allowed for 'root' classes only");

			break;
		}

		if (m->error_.empty() && m->disp_ != Member::d_Namespace)
		{
			has_built_in_types_ = has_built_in_types_ || m->isBuiltInType();

			has_non_optional_types_ = has_non_optional_types_ || (!m->isNullableType() && !m->isRecordType());

			has_defaults_ = has_defaults_ || m->hasDefaultValue();
			has_reserves_ = has_reserves_ || m->reserve_;
		}
	}

	// put first the non-members, then members (to the properties list; the members_ is empty)
	properties.splice(properties.end(), members_);
	// swap them, as we use members_ throughout the code
	std::swap(properties, members_);

	if (members_.empty())
		e.add(sem_Unsupported_Feature, node, "Empty classes are not allowed");
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Header_Includes(SourceFile& os) const
{
	std::set<std::string> user_includes;
	std::set<std::string> handle_typedefs;

	bool requiresApiTypeList = false;
	bool hasCompexTypes = false;

	bool requiresSet = false;
	bool requiresMap = false;
	bool requiresVector = false;
	bool requiresList = false;
	bool requiresPtr = false;

	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		const Member& m = **it;

		switch (m.type_)
		{
		case mt_Enum:
			user_includes.insert(ctx.api_name + "ApiEnums.h");
			break;

		case mt_Typedef:
			user_includes.insert(ctx.api_name + "ApiDefines.h");
			break;

		case mt_Handle:
			handle_typedefs.insert(m.type_ref_);
			// *** no break; here ***

		case mt_Class:
			switch (m.container_)
			{
			case ct_Set:
			case ct_Multiset:
				requiresSet = true;

			case ct_Map:
			case ct_Multimap:
				requiresMap = true;
				break;

			case ct_Vector:
				requiresVector = true;
				break;

			case ct_List:
				requiresList = true;
				break;

			default:
				break;
			}

			hasCompexTypes = true;
			user_includes.insert(m.type_ref_ + ".h");
			break;

		case mt_Choice:
		case mt_Any:
			requiresPtr = true;
			break;

		default:
			break;
		}

		if (m.container_ > ct_Single)
			requiresApiTypeList = true;
	}

	if (!ctx.db.namespaces.empty() || !namespaces_.empty())
		os.addUserInclude(ctx.api_name + "_ns.h");

	os.addUserInclude(ctx.api_name + "ApiFactory.h");

	if (base_object_.empty())
	{
		if (!hasCompexTypes && !requiresPtr)
			os.addUserInclude("ApiBaseObject.h");
	}
	else
		os.addUserInclude(base_object_ + ".h");

	if (requiresPtr)
		os.addUserInclude("ApiBaseObjectPtr.h");

	for (std::set<std::string>::iterator it = user_includes.begin(); it != user_includes.end(); ++it)
		os.addUserInclude(*it);

	if (requiresApiTypeList)
	{
		os << '\n';
		os.addUserInclude("ApiTypeList.h");
	}

	if (!handle_typedefs.empty())
	{
		os << '\n';
		os.addUserInclude("HandleManager.h");
	}

	SourceFile::AddSeparator(os);

	if (requiresList)
		os.addSystemInclude("list");

	if (requiresSet)
		os.addSystemInclude("set");

	if (requiresMap)
		os.addSystemInclude("map");

	if (requiresVector)
		os.addSystemInclude("vector");

	os.addSystemInclude("sstream");
	os.addSystemInclude("iostream");

	SourceFile::AddSeparator(os);

	for (std::set<std::string>::iterator it = handle_typedefs.begin(); it != handle_typedefs.end(); ++it)
		os <<
			"typedef HandleManager<" << *it << "*> " << *it << "Manager;\n"
			"typedef " << *it << "Manager::HANDLE " << *it << "Handle;\n\n";

	if (!handle_typedefs.empty())
		SourceFile::AddSeparator(os);

	if (!error_.empty())
	{
		os << error_;
		SourceFile::AddSeparator(os);
	}
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Source_Includes(SourceFile& os) const
{
	std::set<std::string> user_includes;

	bool requiresTokenizer = false;

	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		if ((*it)->isChoiceType())
		{
			requiresTokenizer = true;

			std::string error;
			ErrorMessage e(ctx, error, false);

			for (Member::ChoicesList::const_iterator it_choice = (*it)->choices().begin(); it_choice != (*it)->choices().end(); ++it_choice)
			{
				user_includes.insert(it_choice->type_ref_ + ".h");

				EntityDB::const_iterator it_type_ref = ctx.db.classes.find(it_choice->type_ref_);

				if (it_type_ref == ctx.db.classes.end())
				{
					e.add(sem_Undefined_Type_Reference, it_choice->node(), it_choice->type_ref_.c_str());
					continue;
				}
			}

			if (!error.empty())
				os << error << '\n';
		}
	}

	os.addUserInclude(name_ + ".h");
	os << "\n";

	if (user_includes.size())
	{
		for (std::set<std::string>::iterator it = user_includes.begin(); it != user_includes.end(); ++it)
			os.addUserInclude(*it);
	}

	if (requiresTokenizer)
	{
		os << '\n';
		os.addUserInclude("Tokenizer.h");
	}
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_header(SourceFile& os) const
{
	dump_Header_Includes(os);

	os.addOpenNamespace();

	if (!description_.empty())
	{
		os <<
			"/*\n"
			"	" << description_ << "\n"
			"*/\n";
	}

	os <<
		"class " << name_ << " : public " << GetParentClass() << "\n"
		"{\n"
		"	API_TYPE";

	const Member* ns_member = findNs(ns_, node());

	if (ns_member)
		os << "_NS";

	os << "(" << ctx.api_name << "ApiFactory, " << id_ << ", " << name_ << ", \"" << tag_ << '"';

	if (ns_member)
	{
		os << ", ";

		if (ns_.empty())
			os << "NULL";
		else
			os << ns_member->ns_ << "::tag_" << ns_member->name_;

		os << ", " << ns_member->ns_ << "::urn_" << ns_member->name_;
	}

	os <<
		")\n"
		"\n";

	if (has_built_in_types_ || has_defaults_ || has_reserves_)
		os <<
			'	' << name_ << "();\n"
			"\n";

	if (!has_non_optional_types_)
		os <<
			"	virtual bool IsAssigned() const;\n"
			"\n";

	if (is_root_)
		os <<
			"	virtual bool IsRootObject() const\n"
			"	{ return true; }\n"
			"\n";

	os <<
		"\n"
		"public:\n"
		"\n";

	DumpContext dc(&os);

	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
		(*it)->dump(dc);

	os <<
		"};\n"
		"\n";
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_source(SourceFile& os) const
{
	dump_Source_Includes(os);

	os.addUsingNamespace();

	if (has_built_in_types_ || has_defaults_ || has_reserves_)
		dump_Source_Constructor(os);

	dump_Source_IsTheSame(os);

	if (!has_non_optional_types_)
		dump_Source_IsAssigned(os);

	dump_Source_CurrentBinarySize(os);

	dump_Source_WriteToSegment(os);
	dump_Source_ReadFromSegment(os);

	dump_Source_WriteToStream(os);

	dump_Source_ReadFromXml(os);

	SourceFile::AddSeparator(os);
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Source_Constructor(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os << name_ << "::" << name_ << "()\n";

	bool first = true;

	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		const Member& m = **it;

		if (m.isBuiltInType() || m.hasDefaultValue())
		{
			os << '	' << (first ? ':' : ',');
			first = false;

			os << ' ' << m.memberName() << "(" << m.defaultValue() << ")\n";
		}
	}

	os << "{\n";
	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		const Member& m = **it;

		if (m.reserve_)
		{
			if (m.key_.empty())
				os << '	' << m.memberName() << ".reserve(" << m.reserve_ << ");\n";

			if (mt_Handle == m.type_)
				os << '	' << m.type_ref_ << "Manager::instance().reserve(" << m.reserve_ << ");\n";
		}
	}
	os << "}\n";
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::GenerateMemberFunction(SourceFile& os, const char* functionName, const char* object2, const char* param, const Member& it)
{
	bool do_method_call = it.isSpecialType() && ct_Single == it.container_;

	if (do_method_call)
	{
		os << it.memberName() << "." << functionName << '(';

		if (object2)
			os << object2 << it.memberName();
	}
	else
	{
		os << "ApiTypeHelper<" << it.typeName() << ">::" << functionName << '(' << it.memberName();

		if (object2)
			os << ", " << object2 << it.memberName();
	}

	if (param)
	{
		if (object2 || !do_method_call)
			os  << ", ";

		os << param;
	}

	os << ')';
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Source_IsTheSame(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os << "bool " << name_ << "::IsTheSame(const ApiBaseObject& base) const\n";

	os <<
		"{\n"
		"	if (0 != strcmp(objectCodeName(), base.objectCodeName()))\n"
		"		return false;\n"
		"\n";

	if (!base_object_.empty() || members_.size() > 0)
		os << "	const " << name_ << "& obj = (const " << name_ << "&)base;\n\n";

	os << "	return";

	bool empty = true;

	if (!base_object_.empty())
	{
		empty = false;
		os << "\n		" << GetParentClass() << "::IsTheSame(base)";
	}

	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		if (!empty)
			os << " &&";

		empty = false;
		os << "\n		";

		GenerateMemberFunction(os, "IsTheSame", "obj.", NULL, **it);
	}

	if (empty)
		os << " true";

	os <<
		";\n"
		"}\n";
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Source_CurrentBinarySize(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os <<
		"size_t " << name_ << "::CurrentBinarySize() const\n"
		"{\n"
		"	return";

	bool empty = true;

	if (!base_object_.empty())
	{
		os << "\n		" << GetParentClass() << "::CurrentBinarySize()";
		empty = false;
	}

	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		if (!empty)
			os << " +";

		empty = false;
		os <<
			"\n"
			"		";

		GenerateMemberFunction(os, "CurrentBinarySize", NULL, NULL, **it);
	}

	if (empty)
		os << " 0";

	os <<
		";\n"
		"}\n";
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Source_WriteToSegment(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os <<
		"CSegment& " << name_ << "::WriteTo(CSegment& seg) const\n"
		"{\n";

	if (is_root_)
		os << "	seg.EnsureWriteOf(CurrentBinarySize());\n\n";

	if (!base_object_.empty())
		os << "	" << GetParentClass() << "::WriteTo(seg);\n";

	os << "	return seg";

	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
		os << "\n		<< " << (*it)->memberName();

	os <<
		";\n"
		"}\n";
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Source_ReadFromSegment(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os <<
		"CSegment& " << name_ << "::ReadFrom(CSegment& seg)\n"
		"{\n";

	if (!base_object_.empty())
		os << "	" << GetParentClass() << "::ReadFrom(seg);\n";

	os << "	return seg";

	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
		os << "\n		>> " << (*it)->memberName();

	os <<
		";\n"
		"}\n";
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Source_IsAssigned(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os <<
		"bool " << name_ << "::IsAssigned() const\n"
		"{\n"
		"	return";

	bool empty = true;

	if (!base_object_.empty())
	{
		os << "\n" << "	" << "	" << GetParentClass()  << "::IsAssigned()";
		empty = false;
	}

	for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		if (!empty)
			os << " ||";

		empty = false;
		os << "\n		";
		GenerateMemberFunction(os, "IsAssigned", NULL, NULL, **it);
	}

	if (empty)
		os << " true";

	os <<
		";\n"
		"}\n";
}

///////////////////////////////////////////////////////////////////////////
bool ClassMeta::hasAttributes() const
{
	MembersList::const_iterator it = members_.begin();

	return it != members_.end() && (*it)->isProperty();
}

///////////////////////////////////////////////////////////////////////////
bool ClassMeta::hasElements() const
{
	MembersList::const_reverse_iterator it = members_.rbegin();

	return it != members_.rend() && (*it)->isElement();
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Source_WriteToStream(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os <<
		"void " << name_ << "::WriteTo(IApiObjectEncoder& encoder) const\n"
		"{\n";

	if (is_root_)
	{
		const EntityDB* edbs[] = { &namespaces_, &ctx.db.namespaces };

		for (size_t i = 0; i < sizeof(edbs)/sizeof(edbs[0]); ++i)
		{
			for (EntityDB::const_iterator it = edbs[i]->begin(); it != edbs[i]->end(); ++it)
			{
				if (i && edbs[0]->find(it->first) != edbs[0]->end())
					continue;

				const Member* m = reinterpret_cast<Member*>(it->second);

				os << "	encoder.addNamespace(" << m->ns_ << "::urn_" << m->name_;

				if (!m->tag_.empty())
					os << ", " << m->ns_ << "::tag_" << m->name_;

				os << ");\n";
			}
		}
	}

	if (!base_object_.empty())
		os << "	" << GetParentClass() << "::WriteTo(encoder);\n";

	if (!members_.empty())
	{
		for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
		{
			const Member& m = **it;
			os << "	WriteToHelper<" << m.typeName() << ", " << m.dataSource(true) << ">::WriteTo(" << m.memberName() << ", encoder";

			if (!m.tag_.empty())
				os << ", (const utfChar*)\"" << m.tag_ << '"';

			if (!m.ns_.empty())
			{
				const Member* ns = findNs(m.ns_, m.node());

				if (ns)
					os << ", " << ns->ns_ << "::tag_" << ns->name_;
			}

			os << ");\n";
		}
	}

	os << "}\n";
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump_Source_ReadFromXml(SourceFile& os) const
{
	SourceFile::AddSeparator(os);

	os <<
		"void " << name_ << "::ReadFromXml(const xmlNode* node, bool initDefaults/* = true*/)\n"
		"{\n"
		"	if (!node)\n"
		"		return;\n";

	if (!base_object_.empty())
		os <<
			"\n"
			"	" << GetParentClass() << "::ReadFromXml(node, initDefaults);\n";

	const bool bAttrs = hasAttributes();
	const bool bElems = hasElements();

	const size_t maps = static_cast<size_t>(bAttrs) + static_cast<size_t>(bElems);

	if (maps)
	{
		const char* maps_names[2] = { "map", "maps[2]" };
		const char* map_access[2] = { "maps[0]", "maps[1]" };

		size_t map_index = 0;

		const char* map_name_attr = maps > 1 ? map_access[map_index] : maps_names[0];
		if (bAttrs) ++map_index;

		const char* map_name_elem = maps > 1 ? map_access[map_index] : maps_names[0];
		if (bElems) ++map_index;

		const size_t readers = members_.size();

		os <<
			"\n"
			"	static ReadersMeta " << maps_names[maps-1] << ";\n"
			"\n"
			"	if (" << (maps > 1 ? map_access[0] : maps_names[0]) << ".empty())\n"
			"	{\n";

		for (MembersList::const_iterator it = members_.begin(); it != members_.end(); ++it)
		{
			const Member& m = **it;
			const char* map_name = m.isProperty() ? map_name_attr : map_name_elem;

			std::string header("		ApiBaseObject::AddReader<");
			header += m.typeName();
			header += ">(";
			header += map_name;
			header += ", &";
			header += name_;
			header += "::";

			switch (m.type_)
			{
			case mt_Choice:
				for (Member::ChoicesList::const_iterator it_choice = m.choices().begin(); it_choice != m.choices().end(); ++it_choice)
				{
					const std::string& tag(it_choice->tag_.empty() ? ctx.xmlNames[it_choice->type_ref_].first : it_choice->tag_);
					const Member& m = **it;

					os << header << m.memberName() << ", " << m.dataSource(false) << ", (const utfChar*)\"" << tag << '"';

					const std::string& ns_prefix(it_choice->ns_.empty() ? ctx.xmlNames[it_choice->type_ref_].second : it_choice->ns_);
					const Member* ns = findNs(ns_prefix, m.node());

					if (ns)
						os << ", " << ns->ns_ << "::urn_" << ns->name_;

					os << ");\n";
				}
				break;

			case mt_Any:
				os << header << m.memberName() << ", " << m.dataSource(false) << ", NULL);\n";
				break;

			default:
				os << header << m.memberName() << ", " << m.dataSource(false) << ", (const utfChar*)\"" << m.tag_ << '"';

				// attributes do not inherit default namespace
				if (!m.isProperty() || !m.ns_.empty())
				{
					const Member* ns = findNs(m.ns_, m.node());

					if (ns)
						os << ", " << ns->ns_ << "::urn_" << ns->name_;
				}

				os << ");\n";
				break;
			}
		}

		os <<
			"	}\n"
			"\n";

		if (bAttrs)
			os << "	ApiBaseObject::HandleReadXmlAttributes(" << map_name_attr << ", node, this, initDefaults);\n";

		if (bElems)
			os << "	ApiBaseObject::HandleReadXmlElements(" << map_name_elem << ", node, this, initDefaults);\n";
	}

	os << "}\n";
}

///////////////////////////////////////////////////////////////////////////
void ClassMeta::dump() const
{
	SourceFile header(ctx, name_ + ".h");
	SourceFile source(ctx, name_ + ".cpp");

	dump_header(header);
	dump_source(source);
}

///////////////////////////////////////////////////////////////////////////
