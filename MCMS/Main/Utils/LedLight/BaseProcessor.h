#ifndef BASEPROCESSOR_H_
#define BASEPROCESSOR_H_
#include "commonDate.h"

using namespace std;

class BaseProcessor;

typedef DWORD STATUS;
typedef STATUS (BaseProcessor::*HANDLE_COMMAND)(const std::string & parameters);
#define ADD_CLI(CLI,FUNC,HELP) AddCliEntry(CLI,(HANDLE_COMMAND)&FUNC,#FUNC,HELP)

struct CLiEntry
{
	const char * cli_name;
	HANDLE_COMMAND cli_function;
	const char * cli_function_name;
	const char * cli_help;
};

class BaseProcessor
{
public:
    BaseProcessor();
    virtual ~BaseProcessor();
    void AddCliEntry(const char * cli_name,
					   HANDLE_COMMAND func,
					   const char * cli_func_name,
					   const char * cli_help);
    void HandleTerminalCommand(const std::string &msg);
    virtual void InitCliTable() {}
private:
    std::list<CLiEntry> m_cli_table;

};

#endif


