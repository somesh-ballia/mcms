#ifndef PARSER_H__
#define PARSER_H__

///////////////////////////////////////////////////////////////////////////
#include <string>
#include <map>

#include <fstream>

///////////////////////////////////////////////////////////////////////////
class SourceFile;

struct ParserDB;

///////////////////////////////////////////////////////////////////////////
typedef struct _xmlNode xmlNode;

///////////////////////////////////////////////////////////////////////////
class ErrorMessage;

///////////////////////////////////////////////////////////////////////////
class Context
{
public:

	Context(const ParserDB& pdb, const std::string& folder, const std::string& schema, const std::string& list_file_name);

	enum NotationEnum
	{ n_Hungarian = 0, n_Simple };

public:

	void onCreateSourceFile(const std::string& file_name);

	void onError()
	{ ++errors_; }

	size_t nextId()
	{ return ++id_; }

public:

	size_t errors() const
	{ return errors_; }

public:

	const ParserDB& db;
	
	const std::string& output_folder;
	const std::string list_name;

	std::string schema_name;
	std::string api_name;
	std::string api_ns;
	
	NotationEnum notation;

	typedef std::string Tag;
	typedef std::string TagContext;
	typedef std::string Name;
	typedef std::string NS;

	typedef std::map< Name, std::pair<Tag, NS> > XmlNamesMap; // name to tag mapping
	XmlNamesMap xmlNames;

private:

	std::ofstream list_file_;
	size_t        id_;

	size_t        errors_;
};

///////////////////////////////////////////////////////////////////////////
struct DumpContext
{
	DumpContext(SourceFile* h = NULL, SourceFile* s = NULL)
		: header_(h)
		, source_(s)
	{}

	SourceFile* const header_;
	SourceFile* const source_;
};

///////////////////////////////////////////////////////////////////////////
class IParserEntityInfo
{
public:

	virtual void initFromXml(const xmlNode* node) = 0;
	virtual void dump(DumpContext& dc) const = 0;

public:

	virtual const std::string& name() const
	{ return name_; }

	const xmlNode* node() const
	{ return node_; }

	bool operator ==(const IParserEntityInfo& other) const
	{ return name_ == other.name_; }

protected:

	IParserEntityInfo(Context& context)
		: ctx(context)
		, node_(NULL)
	{}

	virtual ~IParserEntityInfo()
	{}

protected:

	Context& ctx;

	const xmlNode* node_;

	std::string error_;

	std::string name_;
	std::string description_;
};

///////////////////////////////////////////////////////////////////////////
#endif // PARSER_H__
