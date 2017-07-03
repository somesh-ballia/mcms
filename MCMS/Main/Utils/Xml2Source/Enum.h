#ifndef ENUM_H__
#define ENUM_H__

#include "Common.h"

#include "SourceFile.h"

#include <string>
#include <list>
#include <ostream>

#include <libxml/tree.h>

///////////////////////////////////////////////////////////////////////////
struct EnumValue
{
	std::string error_;
	std::string description_;

	std::string name_;
	std::string tag_;

	bool t_assigned_;

	EnumValue()
		: t_assigned_(false)
	{}
};

///////////////////////////////////////////////////////////////////////////
class Enum : public IParserEntityInfo
{
public:

	static const std::string enumDefaultValue(const std::string& enum_type_name);
	static const std::string enumValueName(const std::string& enum_type_name, const std::string& value_name);

public:

	Enum(Context& context);

public:

	virtual void initFromXml(const xmlNode* node);

	virtual void dump(DumpContext& dc) const
	{
		dump_header(*dc.header_);
		dump_source(*dc.source_);
	}

private:

	const std::string enumDefaultValue() const;
	const std::string enumLastValue() const;

	const std::string stringsArrayName() const;

	const std::string methodName_toString() const;
	const std::string signature_toString() const;

	const std::string methodName_toTag() const;
	const std::string signature_toTag() const;

	const std::string methodName_toEnum() const;
	const std::string signature_toEnum() const;

private:

	void dump_header(SourceFile& os) const;
	void dump_source(SourceFile& os) const;

	void dump_StrArray(SourceFile& os) const;
	void dump_GetStrMethods(SourceFile& os) const;

private:

	std::string base_value_;
	std::list<EnumValue> values_;
};

///////////////////////////////////////////////////////////////////////////
#endif // ENUM_H__
