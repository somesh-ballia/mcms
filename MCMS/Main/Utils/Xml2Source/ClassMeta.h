#ifndef CLASS_META_H__
#define CLASS_META_H__

///////////////////////////////////////////////////////////////////////////
#include "Generator.h"

#include "SourceFile.h"

#include "Member.h"

///////////////////////////////////////////////////////////////////////////
#include "Singleton.h"

///////////////////////////////////////////////////////////////////////////
#include <list>
#include <set>

#include <queue>

///////////////////////////////////////////////////////////////////////////
class ClassMeta : public IParserEntityInfo
{
public:

	static void GenerateMemberFunction(SourceFile& os, const char* functionName, const char* object2, const char* param, const Member& it);

public:

	ClassMeta(Context& context)
		: IParserEntityInfo(context)
		, id_(context.nextId())
		, has_built_in_types_(false)
		, has_non_optional_types_(false)
		, has_defaults_(false)
		, has_reserves_(false)
		, is_root_(false)
	{}

	typedef std::list<Member*> MembersList;

	const MembersList& members() const
	{ return members_; }

	const EntityDB& namespaces() const
	{ return namespaces_; }

public:

	virtual void initFromXml(const xmlNode* node);

	virtual void dump(DumpContext& /*dc*/) const
	{ dump(); }

protected:

	virtual std::string GetParentClass() const;

protected:

	bool hasAttributes() const;
	bool hasElements() const;

	const Member* findNs(const std::string& prefix, const xmlNode* node) const;

private:

	void dump_Header_Includes(SourceFile& os) const;
	void dump_header(SourceFile& os) const;

	void dump_Source_Includes(SourceFile& os) const;
	void dump_source(SourceFile& os) const;

	void dump_Source_Constructor(SourceFile& os) const;
	void dump_Source_IsTheSame(SourceFile& os) const;

	void dump_Source_CurrentBinarySize(SourceFile& os) const;

	void dump_Source_WriteToSegment(SourceFile& os) const;
	void dump_Source_ReadFromSegment(SourceFile& os) const;

	void dump_Source_WriteToStream(SourceFile& os) const;

	void dump_Source_ReadFromXml(SourceFile& os) const;

	void dump_Source_IsAssigned(SourceFile& os) const;

	void dump() const;

protected:

	std::string tag_;
	std::string ns_;

	std::string base_object_;

	EntityDB namespaces_;
	MembersList members_;

	size_t id_;

	bool has_built_in_types_;
	bool has_non_optional_types_;
	bool has_defaults_;
	bool has_reserves_;
	bool is_root_;              // is root class in the hierarchy
};

///////////////////////////////////////////////////////////////////////////
#endif // CLASS_META_H__
