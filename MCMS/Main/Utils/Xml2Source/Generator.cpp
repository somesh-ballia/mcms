#include "Generator.h"

///////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "Error.h"

#include "XLang.h"

#include "SourceFile.h"

///////////////////////////////////////////////////////////////////////////
#include "Enum.h"
#include "Define.h"
#include "ClassMeta.h"

///////////////////////////////////////////////////////////////////////////
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <fstream>

#include <stdio.h> // remove / rename files

///////////////////////////////////////////////////////////////////////////
const EntityDB* ParserDB::get(eMemberType type) const
{
	switch (type)
	{
	case mt_Class:
		return &classes;

	case mt_Enum:
		return &enums;

	case mt_Typedef:
		return &defines;

	default:
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////
void RemovePreviousSources(const std::string& output_folder, const std::string& list_name)
{
	std::ifstream f(list_name.c_str());

	std::string file_name;

	while (!f.eof() && !f.fail())
	{
		f >> file_name;

		if (file_name.empty())
			continue;

		std::cout << "Removing " << file_name << "\n";

		std::string old_path(output_folder + file_name);
		remove(old_path.c_str());
	}
}

///////////////////////////////////////////////////////////////////////////
void DumpNamespaces(DumpContext& dc, const EntityDB& namespaces, const std::string& name)
{
	if (namespaces.empty())
		return;

	dc.header_->addSeparator();

	*dc.header_ <<
		"struct " << name << "_ns\n"
		"{\n";

	for (EntityDB::const_iterator it = namespaces.begin(); it != namespaces.end(); ++it)
		it->second->dump(dc);

	*dc.header_ << "};\n";

	dc.source_->addSeparator();
}

///////////////////////////////////////////////////////////////////////////
void GenerateSources(Context& ctx, const ParserDB& db)
{
	std::cout << "Generating sources for " << ctx.schema_name << "\n";

	const std::string& base_name = ctx.api_name;

	if (!db.defines.empty())
	{
		SourceFile header(ctx, base_name + "ApiDefines.h");
		DumpContext dc(&header);

		for (EntityDB::const_iterator it = db.defines.begin(); it != db.defines.end(); ++it)
			it->second->dump(dc);
	}

	if (!db.enums.empty())
	{
		SourceFile header(ctx, base_name + "ApiEnums.h");

		header << "#include \"ApiTypeHelper.h\"\n";
		SourceFile::AddSeparator(header);
		header << "class CSegment;\n";

		SourceFile source(ctx, base_name + "ApiEnums.cpp");
		source <<
			"#include \"" << base_name << "ApiEnums.h\"\n"
			"#include \"Segment.h\"\n"
			"\n"
			"#include <string>\n";

		source.addOpenNamespace();

		DumpContext dc(&header, &source);

		for (EntityDB::const_iterator it = db.enums.begin(); it != db.enums.end(); ++it)
			it->second->dump(dc);

		SourceFile::AddSeparator(source);
	}

	if (!ctx.db.classes.empty())
	{
		const std::string file_name(ctx.api_name + "_ns");

		SourceFile ns_source(ctx, file_name + ".cpp");
		SourceFile ns_header(ctx, file_name + ".h");

		DumpContext dc(&ns_header, &ns_source);

		ns_source.addUserInclude(file_name + ".h");
		ns_source.addSeparator();

		ns_header <<
			"typedef unsigned char utfChar;\n"
			"\n";

		DumpNamespaces(dc, ctx.db.namespaces, ctx.api_name);

		const std::string h_name(base_name + "ApiFactory.h");
		SourceFile header(ctx, h_name);
		header.addUserInclude("ApiFactory.h");
		header.addSeparator();
		header << "DECLARE_API_OBJECTS_FACTORY(" << base_name << "ApiFactory);\n";

		const std::string c_name(base_name + "ApiClasses.h");
		SourceFile classes(ctx, c_name);

		SourceFile factory(ctx, base_name + "ApiFactory.cpp");
		factory.addUserInclude(h_name);
		factory.addUserInclude(c_name);
		factory.addUsingNamespace();
		factory.addSeparator();
		factory <<
			"template <>\n"
			"ApiObjectsFactory<" << base_name << "ApiFactory_TAG>::ApiObjectsFactory()\n"
			"	: registrar_(ApiObjectsRegistrar::instance())\n"
			"	, id_(\"" << base_name << "\")\n"
			"{\n"
			"	index_.resize(" << ctx.nextId() << ");\n"
			"\n";

		for (EntityDB::const_iterator it = db.classes.begin(); it != db.classes.end(); ++it)
		{
			classes.addUserInclude(it->second->name() + ".h");

			factory << "	registerObject<" << it->second->name() << ">();\n";

			it->second->dump(dc);

			const ClassMeta& x = *reinterpret_cast<ClassMeta*>(it->second);
			DumpNamespaces(dc, x.namespaces(), x.name());
		}

		factory << "}\n";

		factory.addSeparator();
	}
}

///////////////////////////////////////////////////////////////////////////
bool ProcessSchema(Context& ctx, ParserDB& db)
{
	XmlParser p(ctx.schema_name);

	if (!p)
	{
		std::cerr << "Failed to parse schema '" << ctx.schema_name << "'\n";
		return false;
	}

	std::cout << "Parsing schema: '"<< ctx.schema_name << "'\n";

	std::string error;
	ErrorMessage e(ctx, error, false);

	const xmlNode* root_element = p;

	if (root_element->name != xlang_API)
	{
		e.add(sntx_Unsupported_Declarator, root_element, NULL);
		e.add(sem_Unsupported_Feature, root_element, "'API' declarator is expected");
		return false;
	}

	for (const xmlAttr* attr = root_element->properties; attr; attr = attr->next)
	{
		if (attr->name == xlang_name)
		{
			if (ctx.api_name.empty())
			{
				std::string tag((const char*)attr->children->content);
				SourceFile::tagToName(tag, ctx.api_name, true);

				if (!SourceFile::isValidName(ctx.api_name))
					e.add(sntx_Bad_Attribute_Value, root_element, attr);
			}
			else
				e.add(sem_Illegal_Attribute, root_element, (const char*)attr->name);
		}

		else if (attr->name == xlang_ns)
		{
			std::string ns((const char*)attr->children->content);
			SourceFile::tagToName(ns, ctx.api_ns, true);

			if (!SourceFile::isValidName(ctx.api_ns))
				e.add(sntx_Bad_Attribute_Value, root_element, attr);
		}

		else if (attr->name == xlang_notation)
		{
			if (strcmp((const char*)attr->children->content, "hungarian"))
				ctx.notation = Context::n_Simple;
		}

		else
			e.add(sntx_Unsupported_Attribute, root_element, attr);
	}

	if (ctx.api_name.empty())
		e.add(sem_Required_Attribute, root_element, (const char*)xlang_name.c_str());

	for (const xmlNode* node = root_element->children; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE)
			continue;

		if (node->name == xlang_INCLUDE)
		{
			std::string path;

			for (xmlAttr* attr = node->properties; attr; attr = attr->next)
			{
				if (attr->name == xlang_name)
				{
					path = (const char*)attr->children->content;

					if (FilePath::isRelative(path))
						path = FilePath::pathOf(ctx.schema_name) + path;

					std::string schema_name(ctx.schema_name);
					ctx.schema_name = path;

					if (!ProcessSchema(ctx, db))
						e.add(sntx_Bad_Attribute_Value, node, attr);

					ctx.schema_name = schema_name;
				}

				else
					e.add(sntx_Unsupported_Attribute, node, attr);
			}

			if (path.empty())
				e.add(sem_Required_Attribute, node, "name");

			continue;
		}

		IParserEntityInfo* item = NULL;
		EntityDB* edb = NULL;

		if (xlang_CLASS == node->name)
		{
			item = new ClassMeta(ctx);
			edb = &db.classes;
		}

		else if (node->name == xlang_ENUM)
		{
			item = new Enum(ctx);
			edb = &db.enums;
		}

		else if (xlang_NAMESPACE == node->name)
		{
			item = new Member(ctx);
			edb = &db.namespaces;
		}

		else if (node->name == xlang_DEFINE || node->name == xlang_TYPEDEF)
		{
			item = new Define(ctx);
			edb = &db.defines;
		}

		if (item)
		{
			item->initFromXml(node);

			std::pair<EntityDB::iterator, bool> status = edb->insert(std::make_pair(item->name(), item));

			if (!status.second)
			{
				e.add(sem_Name_Already_Defined, node, item->name().c_str());

				const IParserEntityInfo* i = status.first->second;
				e.add(sem_Refer_To_Definition, i->node(), i->name().c_str());
			}
		}
		else
			e.add(sntx_Unsupported_Declarator, node, NULL);
	}

	return error.empty();
}

///////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	const std::string schema_path(argv[1]);
	std::string output_folder(argv[2]);

	LIBXML_TEST_VERSION;

	if (output_folder.empty())
		output_folder = FilePath::pathOf(schema_path);
	else
		FilePath::fix(output_folder);

	std::cout
		<< FilePath::nameOf(argv[0]) << " version " << SourceFile::version() << "\n"
		"Schema: " << schema_path << "\n"
		"Target folder: " << output_folder << "\n";

	std::string list_name(output_folder + FilePath::nameOf(schema_path) + ".list");
	RemovePreviousSources(output_folder, list_name);

	ParserDB db;
	Context ctx(db, output_folder, schema_path, list_name);

	bool ok = ProcessSchema(ctx, db);

	if (ok)
		GenerateSources(ctx, db);

	ok &= !ctx.errors();

	std::cout << (ok ? "Done" : "Failed") << ": " << schema_path << "\n";

	if (ctx.errors())
		std::cout << ctx.errors() << " syntax error(s) encountered.\n";

	return ok ? 0 : -1;
}

///////////////////////////////////////////////////////////////////////////
