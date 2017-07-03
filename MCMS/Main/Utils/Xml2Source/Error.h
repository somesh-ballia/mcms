#ifndef ERROR_H__
#define ERROR_H__

///////////////////////////////////////////////////////////////////////////
#include <string>

#include <libxml/tree.h>

///////////////////////////////////////////////////////////////////////////
enum SyntaxMessagesEnum
{
	// syntax errors
	sntx_FIRST_ = 1000,
	sntx_Unsupported_Declarator,
	sntx_Unsupported_Attribute,
	sntx_Bad_Attribute_Value,
	sntx_Unexpected_Nested_Declarator,
};

///////////////////////////////////////////////////////////////////////////
enum SemanticMessagesEnum
{
	// semantic errors
	sem_FIRST_ = 2000,
	sem_Refer_To_Definition,
	sem_Required_Attribute,
	sem_Unsupported_Base_Type,
	sem_Illegal_Indexed_Access,
	sem_Illegal_Space_Reservation,
	sem_Illegal_Attribute,
	sem_Unsupported_Feature,
	sem_Name_Already_Defined,
	sem_Tag_Ns_Already_Defined,
	sem_Undefined_Type_Reference,
	sem_Undefined_Name_Reference,
	sem_Choice_Already_Defined,
};

///////////////////////////////////////////////////////////////////////////
class Context;

///////////////////////////////////////////////////////////////////////////
class ErrorMessage
{
public:

	ErrorMessage(Context& context, std::string& output, bool indent = true)
		: ctx_(context)
		, error_(output)
		, indent_(indent)
	{}

	void add(SyntaxMessagesEnum err, const xmlNode* node, const xmlAttr* attr);
	void add(SemanticMessagesEnum err, const xmlNode* node, const char* v);

private:

	void forSource(std::ostream& os, size_t err, const xmlNode* node);
	void forTerminal(std::ostream& os, size_t err, const xmlNode* node);

private:

	Context& ctx_;

	std::string& error_;
	bool indent_;
};

///////////////////////////////////////////////////////////////////////////
#endif // ERROR_H__
