// LoggerConsoleCommand.h: interface for the CTerminalCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TerminalCommand_H__)
#define _TerminalCommand_H__


#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include "PObject.h"
#include "DataTypes.h"

class CSegment;


typedef vector<string> CCmdTokenVector;



enum eCommandParamsIndex
{
	eCmdTerminalName = 0,
	eCmdCommandName,
	eCmdDestinationName,
	eCmdParam1,
	eCmdParam2,
	eCmdParam3,
	eCmdParam4,
	eCmdParam5,
	eCmdParam6,
	eCmdParam7,
	eCmdParam8,
	eCmdParam9,
	eCmdParam10
};




class CTerminalCommand : public CPObject  
{
CLASS_TYPE_1(CTerminalCommand, CPObject)	
public:
	CTerminalCommand();
	CTerminalCommand(const char * terminal_file_name, char **argv, int argc);
	CTerminalCommand(const CTerminalCommand&);
	virtual const char* NameOf() const { return "CTerminalCommand";}
	virtual ~CTerminalCommand();
	
	bool operator==(const CTerminalCommand&);
	bool operator!=(const CTerminalCommand &rHnd);
	
	void Clean();
	void SetCommandLine(const char * terminal_file_name, char **argv, int argc);
	const string& GetTerminalName()const;
	const string& GetCommandName()const;
	const string& GetDestinationName()const;
	const string& GetToken(eCommandParamsIndex index)const;

    void SetCommandName(const string & name);

    void SetNumOfDestParams(DWORD numDestParams){m_NumDestParams = numDestParams;}
    
    
    
	void AddToken(const string &token);
	void AddToken(DWORD &token);
	void InsertToken(const string &token, eCommandParamsIndex index);
	DWORD GetNumOfParams()const {return m_CmdTokenVector.size() - eCmdParam1;}
	
	void SerializeMfa(ostream &ostr);
	void Serialize(ostream &ostr);
	void DeSerialize(istream &istr);
	
	void Serialize(CSegment &seg);
	void DeSerialize(CSegment &seg);
	
private:
		// disabled
	CTerminalCommand& operator=(const CTerminalCommand&);

	CCmdTokenVector m_CmdTokenVector;
	string 			m_ErrorToken;
    DWORD           m_NumDestParams;
};

#endif // #if !defined(_CTerminalCommand_H__)
