#ifndef MEMBER_H__
#define MEMBER_H__

///////////////////////////////////////////////////////////////////////////
#include "Parser.h"
#include "Common.h"

#include "ApiType.h"

///////////////////////////////////////////////////////////////////////////
#include <libxml/tree.h>

#include <string>
#include <set>

///////////////////////////////////////////////////////////////////////////
class ClassMeta;

///////////////////////////////////////////////////////////////////////////
class ChoiceMeta
{
public:

	ChoiceMeta(const xmlNode* node)
		: node_(node)
	{}

	const xmlNode* node() const
	{ return node_; }

	bool operator <(const ChoiceMeta& obj) const
	{ return type_ref_ < obj.type_ref_; }

public:

	std::string type_ref_;
	std::string tag_;
	std::string ns_;

private:

	const xmlNode* node_;
};

///////////////////////////////////////////////////////////////////////////
class Member : public IParserEntityInfo
{
	friend class ClassMeta;

public:

	enum DispositionEnum
	{ d_Namespace, d_Attribute, d_Element };

	Member(Context& context);

public:

	virtual void initFromXml(const xmlNode* node);

	virtual void dump(DumpContext& dc) const
	{
		if (isNamespace())
		{
			dump_header_ns(*dc.header_);
			dump_source_ns(*dc.source_);
		}
		else
			dump_definition(*dc.header_);
	}

	virtual const std::string& name() const
	{ return isNamespace() ? tag_ : name_; }

public:

	const std::string& memberName() const
	{ return member_name_; }

	const std::string& typeName() const
	{ return type_name_; }

	typedef std::set<ChoiceMeta> ChoicesList;

	const ChoicesList& choices() const
	{ return choice_type_refs_; }

	bool isProperty() const
	{ return d_Attribute == disp_; }

	bool isNamespace() const
	{ return d_Namespace == disp_; }

	bool isElement() const
	{ return d_Element == disp_; }

	eMemberType type() const
	{ return type_; }

	bool isChoiceType() const
	{ return mt_Choice == type_; }

	bool isPointerType() const;

	bool isSequenceType() const;

	bool isRecordType() const;

	bool isTypeRefRequired() const;

	bool isContainerType() const
	{ return container_ > ct_Single; }

	bool isCompoundType() const;

	// is the C++'s built-in type, like int, bool, etc. that is not being initialized by default
	bool isBuiltInType() const;

	// in addition to all the special types, also `enum` and `handle`
	bool isNullableType() const;

	// is a special type having support for NULL-like value (having a state of 'assigned')
	bool isSpecialType() const;

	bool hasDefaultValue() const
	{ return has_default_; }

	const std::string& defaultValue() const
	{ return default_; }

	const char* dataSource(bool write) const;

private:

	bool initChoicesFromXml(const xmlNode* node, ErrorMessage& e);

	void sync();

	bool requireNoChildNodes(const xmlNode* node, ErrorMessage& e) const;

	void dump_header_ns(SourceFile& os) const;
	void dump_source_ns(SourceFile& os) const;

	void dump_definition(SourceFile& os) const;

private:

	std::string tag_;
	std::string ns_;

	std::string member_name_;

	std::string key_;    // the key being used for indexing map or multimap
	size_t reserve_;     // amount of vector's element to reserve the space for

	eMemberType type_;

	ContainerTypeEnum container_;

	std::string type_ref_;
	std::string type_name_;

	std::string default_; // formatted default value (if any)

	ChoicesList choice_type_refs_;

	bool has_default_;

	DispositionEnum disp_;
};

///////////////////////////////////////////////////////////////////////////
struct MemberFindPredicate
{
	MemberFindPredicate(const std::string& name)
		: name_(name)
	{}

	bool operator ()(const Member* m) const
	{ return m->name() == name_; }

private:

	const std::string& name_;
};

///////////////////////////////////////////////////////////////////////////
#endif // MEMBER_H__
