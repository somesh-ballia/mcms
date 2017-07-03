// LoggerConsoleCommand.cpp: implementation of the CTerminalCommand class.
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include "TerminalCommand.h"
#include "SharedDefines.h"
#include "Segment.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////





/*---------------------------------------------------------------------------------------
  Implementation of Publics
---------------------------------------------------------------------------------------*/

CTerminalCommand::CTerminalCommand()
{
	m_ErrorToken = "Invalide Token";
    m_NumDestParams = 0;
}

CTerminalCommand::CTerminalCommand(const char * terminal_file_name, char **argv, int argc)
{
	m_ErrorToken = "Invalide Token";
    m_NumDestParams = 0;
	SetCommandLine(terminal_file_name, argv, argc);
}

CTerminalCommand::CTerminalCommand(const CTerminalCommand &rHnd)
:CPObject(rHnd), m_CmdTokenVector(rHnd.m_CmdTokenVector),m_ErrorToken(rHnd.m_ErrorToken)
{}

CTerminalCommand::~CTerminalCommand()
{
	
}

void CTerminalCommand::Clean()
{
	m_CmdTokenVector.clear();
    m_NumDestParams = 0;
}

void CTerminalCommand::SetCommandLine(const char * terminal_file_name, char **argv, int argc)
{
	m_CmdTokenVector.push_back(terminal_file_name);
	for(int i = 1 ; i < argc ; i++)
	{
		m_CmdTokenVector.push_back(argv[i]);
	}
}

const string& CTerminalCommand::GetTerminalName()const
{
	const string &strTmp = GetToken(eCmdTerminalName);
	return strTmp;
}

const string& CTerminalCommand::GetCommandName()const
{
	const string &strTmp = GetToken(eCmdCommandName);
	return strTmp;
}

const string& CTerminalCommand::GetDestinationName()const
{
	const string &strTmp = GetToken(eCmdDestinationName);
	return strTmp;
}

const string& CTerminalCommand::GetToken(eCommandParamsIndex index)const
{
	const string &strTmp = ((DWORD)index < m_CmdTokenVector.size()
                            ? 
                            m_CmdTokenVector[index] : m_ErrorToken);
	return strTmp;
}

void CTerminalCommand::SetCommandName(const string & name)
{
    if((DWORD)eCmdCommandName < m_CmdTokenVector.size())
    {
        m_CmdTokenVector[eCmdCommandName] = name;
    }
    else
    {
        PASSERTMSG(m_CmdTokenVector.size() + 100, "Bad Index(size + 100)");
    }
    
}

void CTerminalCommand::AddToken(const string &token)
{
	m_CmdTokenVector.push_back(token);
}
void CTerminalCommand::AddToken(DWORD &token)
{
	ostringstream ostr;
	ostr << token;
	string strToken = ostr.str();
	AddToken(strToken);;
}

void CTerminalCommand::InsertToken(const string &token, eCommandParamsIndex index)
{
	int cnt = 0;
	CCmdTokenVector::iterator iTer = m_CmdTokenVector.begin();
	CCmdTokenVector::iterator iEnd = m_CmdTokenVector.end();	
	while(iTer != iEnd && cnt < index)
	{
		iTer++;
		cnt++;
	}
	if(iTer == iEnd)
	{
		return;
	}
	m_CmdTokenVector.insert(iTer, token);
}
	
void CTerminalCommand::Serialize(CSegment &seg)
{
	DWORD size = m_CmdTokenVector.size();
	seg << size;
	
	CCmdTokenVector::iterator iTer = m_CmdTokenVector.begin();
	CCmdTokenVector::iterator iEnd = m_CmdTokenVector.end();	
	while(iTer != iEnd)
	{
		string &strTmp = *iTer;
		seg << strTmp;
		
		iTer++;
	}
}

void CTerminalCommand::DeSerialize(CSegment &seg)
{
	DWORD size = 0;
	seg >> size;
	m_CmdTokenVector.reserve(size);
	
	string strTmp;
	for(WORD i = 0 ; i < size ; i++)
	{
		seg >> strTmp;
		m_CmdTokenVector.push_back(strTmp);
	}
}

void CTerminalCommand::Serialize(ostream &ostr) 
{	
	DWORD size = m_CmdTokenVector.size();
	ostr << size;
	ostr << ' ';
	
	CCmdTokenVector::iterator iTer = m_CmdTokenVector.begin();
	CCmdTokenVector::iterator iEnd = m_CmdTokenVector.end();	
	while(iTer != iEnd)
	{
		string &strTmp = *iTer;
		ostr << strTmp;
		ostr << ' ';
		
		iTer++;
	}
}

void CTerminalCommand::DeSerialize(istream &istr)
{
	DWORD size = 0;
	istr >> size;
	// fix ' '
	m_CmdTokenVector.reserve(size);

	for(WORD i = 0 ; i < size ; i++)
	{
		string strTmp;
		istr >> strTmp;
		m_CmdTokenVector.push_back(strTmp);
	}
}

void CTerminalCommand::SerializeMfa(ostream &ostr)
{
	DWORD size = m_CmdTokenVector.size();
    DWORD numOfStringsToSend = (size - m_NumDestParams);
	ostr << numOfStringsToSend;
	ostr << ' ';
	
	const string &terminalName = GetTerminalName();
	ostr << terminalName << ' ';
	
	const string &commandName = GetCommandName();
	ostr << commandName << ' ';
    
    eCommandParamsIndex start = (eCommandParamsIndex)(eCmdDestinationName + m_NumDestParams);
	for(eCommandParamsIndex i = start ; (DWORD)i < size ; i = (eCommandParamsIndex)(i + 1))
	{
		const string &currentToken = GetToken(i);
		ostr << currentToken << ' ';
	}
}

bool CTerminalCommand::operator==(const CTerminalCommand &rHnd)
{
	bool result = (m_CmdTokenVector == rHnd.m_CmdTokenVector);
	return result;
}

bool CTerminalCommand::operator!=(const CTerminalCommand &rHnd)
{
	bool result = CTerminalCommand::operator==(rHnd);
	return !result;
}
