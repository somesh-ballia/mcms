#ifndef SOURCE_FILE_H__
#define SOURCE_FILE_H__

///////////////////////////////////////////////////////////////////////////
#include "Parser.h"

#include <string>
#include <fstream>
#include <sstream>

///////////////////////////////////////////////////////////////////////////
class SourceFile : public std::ofstream
{
public:

	static void AddSeparator(std::ostream& os);

	static void unescape(std::string& s);
	static void splitCamelCase(std::string& s);

	static bool isValidName(const std::string& name);
	static bool isValidTag(const std::string& tag);

	static void tagToName(const std::string& tag, std::string& name, bool toClassName = false);
	static void nameToTag(const std::string& name, std::string& tag);
	static void nameToDefine(const std::string& name, std::string& define);

	static const std::string toName(const std::string& tag, bool toClassName = false);
	static const std::string toTag(const std::string& name);
	static const std::string toDefine(const std::string& name);

	static const char* version()
	{ return version_; }

public:

	SourceFile(Context& context, const std::string& file_name);
	~SourceFile();

public:

	void addSystemInclude(const std::string& include);
	void addUserInclude(const std::string& include);

	void addUsingNamespace();

	void addOpenNamespace();
	void addCloseNamespace();

	void addSeparator()
	{ SourceFile::AddSeparator(*this); }

private:

	void addFileHeader(const std::string& schema_name);

	void addHeaderGuardPrefix(const std::string& name);
	void addHeaderGuardPostfix();

private:

	std::string name_def_;

	Context&    context_;
	bool        hasNamespace_;

private:

	static const char* version_;
};

///////////////////////////////////////////////////////////////////////////
class FilePath
{
public:

	static void fix(std::string& path);
	static bool isRelative(const std::string& path);
	
	static const std::string pathOf(const std::string& file_name);
	static const std::string nameOf(const std::string& file_name);

private:

	FilePath();
};

///////////////////////////////////////////////////////////////////////////
#endif // SOURCE_FILE_H__
