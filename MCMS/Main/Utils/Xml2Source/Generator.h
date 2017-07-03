#ifndef GENERATOR_H__
#define GENERATOR_H__

///////////////////////////////////////////////////////////////////////////
#include "Common.h"

#include "Parser.h" // Context

#include <map>

///////////////////////////////////////////////////////////////////////////
typedef std::map<std::string, IParserEntityInfo*> EntityDB;

///////////////////////////////////////////////////////////////////////////
struct ParserDB
{
	EntityDB defines;
	EntityDB enums;
	EntityDB classes;
	EntityDB namespaces;

	const EntityDB* get(eMemberType type) const;
};

///////////////////////////////////////////////////////////////////////////
bool ProcessSchema(Context& ctx);
void GenerateSources(Context& ctx, const ParserDB& db);

///////////////////////////////////////////////////////////////////////////
void DumpNamespaces(DumpContext& dc, const EntityDB& namespaces, const std::string& name);

///////////////////////////////////////////////////////////////////////////
#endif // GENERATOR_H__
