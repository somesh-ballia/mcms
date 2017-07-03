#include "Parser.h"
#include "SourceFile.h"

///////////////////////////////////////////////////////////////////////////
Context::Context(const ParserDB& pdb, const std::string& folder, const std::string& schema, const std::string& list_file_name)
	: db(pdb)
	, output_folder(folder)
	, list_name(list_file_name)
	, schema_name(schema)
	, notation(n_Hungarian)
	, list_file_(list_name.c_str())
	, id_(static_cast<size_t>(-1))
	, errors_(0)
{}

///////////////////////////////////////////////////////////////////////////
void Context::onCreateSourceFile(const std::string& file_name)
{ list_file_ << file_name << '\n'; }

///////////////////////////////////////////////////////////////////////////
