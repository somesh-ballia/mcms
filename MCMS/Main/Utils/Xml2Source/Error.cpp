#include "Error.h"

#include "Parser.h"

///////////////////////////////////////////////////////////////////////////
#include <sstream>
#include <ostream>
#include <iostream>

///////////////////////////////////////////////////////////////////////////
void ErrorMessage::forSource(std::ostream& os, size_t err, const xmlNode* node)
{
	os <<
		(indent_ ? "	" : "") << "// " << ctx_.schema_name << "(" << node->line << "): XS" << static_cast<size_t>(err) << "\n" <<
		(indent_ ? "	" : "") << "#error ";
}

///////////////////////////////////////////////////////////////////////////
void ErrorMessage::forTerminal(std::ostream& os, size_t err, const xmlNode* node)
{
	os << ctx_.schema_name << "(" << node->line << "): XS" << static_cast<size_t>(err) << ": ";
}

///////////////////////////////////////////////////////////////////////////
void ErrorMessage::add(SyntaxMessagesEnum err, const xmlNode* node, const xmlAttr* attr)
{
	ctx_.onError();

	std::ostringstream fs;
	forSource(fs, err, node);

	std::ostringstream ts;
	forTerminal(ts, err, node);

	std::ostringstream os;

	switch (err)
	{
	case sntx_Unsupported_Declarator:
		os << "Unsupported declarator '" << node->name << "'";
		break;

	case sntx_Unsupported_Attribute:
		os << "Unsupported attribute '" << attr->name << "' for declarator '" << node->name << "'";
		break;

	case sntx_Bad_Attribute_Value:
		os << "Bad attribute value '" << attr->children->content << "' for attribute '" << attr->name << "' of declarator '" << node->name << "'";
		break;

	case sntx_Unexpected_Nested_Declarator:
		os << "Unexpected nested declarator '" << node->name << "'";
		break;

	default:
		os << "Please, report this issue to the developer: Unknown error for '" << node->name << "'";

		if (attr)
			os << ", attribute '" << attr->name << "'";
		break;
	}

	fs << '"' << os.str() << '"' << '\n';
	error_ += fs.str();

	std::cerr << ts.str() << os.str() << '\n';
}

///////////////////////////////////////////////////////////////////////////
void ErrorMessage::add(SemanticMessagesEnum err, const xmlNode* node, const char* v)
{
	if (err != sem_Refer_To_Definition)
		ctx_.onError();

	std::ostringstream fs;
	forSource(fs, err, node);

	std::ostringstream ts;
	forTerminal(ts, err, node);

	std::ostringstream os;

	switch (err)
	{
	case sem_Refer_To_Definition:
		os << "Please, refer to the definition of '" << v << "'";
		break;

	case sem_Unsupported_Base_Type:
		os << "Unsupported type '" << v << "'";
		break;

	case sem_Illegal_Indexed_Access:
		os << "By-key indexing is attempted for container '" << v << "'";
		break;

	case sem_Illegal_Space_Reservation:
		os << "Space reservation is attempted for container '" << v << "'";
		break;

	case sem_Illegal_Attribute:
		os << "The attribute '" << v << "' is illegal in this context";
		break;

	case sem_Undefined_Type_Reference:
		os << "An undefined type '" << v << "' is being referenced";
		break;

	case sem_Undefined_Name_Reference:
		os << "An undefined name '" << v << "' is being referenced";
		break;

	case sem_Unsupported_Feature:
		os << v;
		break;

	case sem_Required_Attribute:
		os << "'" << node->name << "' requires attribute '" << v << "' in this context";
		break;

	case sem_Name_Already_Defined:
		os << node->name << ": The name '" << v << "' is already defined";
		break;

	case sem_Tag_Ns_Already_Defined:
		os << node->name << ": The tag '" << v << "' together with the corresponding namespace is already defined";
		break;

	case sem_Choice_Already_Defined:
		os << node->name << ": The choice alternative '" << v << "' is already defined";
		break;

	default:
		os << "Please, report this issue to the developer: Unknown error for '" << node->name << "'";

		if (v && *v)
			os << ", data '" << v << "'";
		break;
	}

	fs << '"' << os.str() << '"' << '\n';
	error_ += fs.str();

	std::cerr << ts.str() << os.str() << '\n';
}

///////////////////////////////////////////////////////////////////////////
