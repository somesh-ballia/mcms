#include "BaseProcessor.h"

//compile comand: g++ -g cppTest.cpp -o gTest
//g++  cppTest.cpp -o cTest
BaseProcessor::BaseProcessor()
{
	//Do nothing
}

BaseProcessor::~BaseProcessor()
{
	//Do nothing
}

void BaseProcessor::AddCliEntry(const char * cli_name,
					   HANDLE_COMMAND func,
					   const char * cli_func_name,
					   const char * cli_help)
{
	CLiEntry temp;
	temp.cli_name = cli_name;
	temp.cli_function = func;
	temp.cli_function_name = "cli_function_name";
	temp.cli_help = cli_help;

	m_cli_table.push_back(temp);
}

void BaseProcessor::HandleTerminalCommand(const std::string &msg)
{
	std::string param = msg; 
	
	std::list<CLiEntry>::iterator itr;
	bool found = false;

	int cliInd = 0;
	for (itr = m_cli_table.begin(); itr != m_cli_table.end(); itr++ )
	{
		CLiEntry e = *itr;
		if (e.cli_name == param)
		{
			found = true;
			(this->*(e.cli_function))(msg);
		}
	}
	
	if (!found)
	{
		cout << "cannot find CLI:" << msg << std::endl;
	}

	return;
}


