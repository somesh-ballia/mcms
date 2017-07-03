// McuCmdProcess.cpp

#include "McuCmdProcess.h"

#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include "OsQueue.h"
#include "TaskApi.h"
#include "OpcodesMcmsCommon.h"
#include "TraceClass.h"
#include "StatusesGeneral.h"
#include "MplMcmsProtocol.h"
#include "IpCsOpcodes.h"
#include "ManagerApi.h"
#include "HostCommonDefinitions.h"
#include "OpcodesMcmsInternal.h"
#include "ObjString.h"

using namespace std;

static const char* STR_LIVE_PROCESS = "live_process";
static const char* STR_TRACE_LEVEL  = "trace_level";

static const BYTE DefaultUnitId  = 0;
static const BYTE DefaultBoardId = 0xFF;
static const BYTE DefaultSubBoardId = 1;
static const BYTE ALL_POSSIBLE = 0xFF;
static const char ALL_POSSIBLE_CHAR = '#';
static const char RANGE_CHAR = '-';
static const char SYNC_MSG_CHAR = '@';
static const DWORD MAX_UNIT_NUMBER = 40;
static DWORD const TERMINAL_COMMAND_RSP_TIMEOUT = 100;
static const bool IsPrintReport = false;

CProcessBase* CreateNewProcess()
{
  return new CMcuCmdProcess;
}

TaskEntryPoint CMcuCmdProcess::GetManagerEntryPoint()
{
  return NULL;
}

CMcuCmdProcess::CMcuCmdProcess()
{
  m_SendMethodsMap[GetDestinationName(eDestAll)] = &CMcuCmdProcess::SendToAll;
  m_SendMethodsMap[GetDestinationName(eDestMCMS)] = &CMcuCmdProcess::SendToMCMS;
  m_SendMethodsMap[GetDestinationName(eDestCS)] = &CMcuCmdProcess::SendToCS;
}

CMcuCmdProcess::~CMcuCmdProcess()
{}

int CMcuCmdProcess::Run()
{
  int argc = GetArgc();
  if (argc < 3)
  {
    cout << "Error. " << endl;
    cout << "usage: Bin/McuCmd [command name] [destination] {param}" << endl <<
    endl;
    return 1;
  }

  const char* terminal_file_name = ttyname(0);
  if (NULL == terminal_file_name)
    terminal_file_name = "/dev/null";

  char** argv = (char**) GetArgv();


  CTerminalCommand command(terminal_file_name, argv, argc);
  DWORD            numOfParams = command.GetNumOfParams();

  const string& commandName = command.GetCommandName();
  if (commandName == STR_LIVE_PROCESS)
    CProcessBase::FindLiveProcesses(std::cout);
  else if (commandName == STR_TRACE_LEVEL)
    CTrace::FillTraceLevelNames(std::cout);
  else
  {
    STATUS status = STATUS_FAIL;
    bool   sync = IsCommandSync(command);
    if (sync)
    {
      const string& oldCommandName = command.GetCommandName();
      string        newCommandName = oldCommandName.substr(1, oldCommandName.length() - 1);
      command.SetCommandName(newCommandName);
      status = SendSyncTerminalCommand(command);
    }
    else
    {
      status = SendTerminalCommand(command);
      SystemSleep(30, FALSE);
    }

    if (IsPrintReport)
      cout << "Status     : "<< status << endl << endl;
  }

  return 0;
}

bool CMcuCmdProcess::IsCommandSync(CTerminalCommand& command)
{
  const string& commandName = command.GetCommandName();
  bool          result = (commandName.c_str()[0] == SYNC_MSG_CHAR);
  return result;
}

STATUS CMcuCmdProcess::SendSyncTerminalCommand(CTerminalCommand& command)
{
  STATUS status = STATUS_FAIL;

  const string destinationName = command.GetDestinationName();
  CObjString::ToUpper((char*)(destinationName.c_str()));

  const char* strMcms = GetDestinationName(eDestMCMS);
  if (0 == strncmp(destinationName.c_str(), strMcms, strlen(strMcms)))
  {
    status = SendSyncTerminalCommandToMcms(command);
    return status;
  }

  eProcessType processType = CProcessBase::GetProcessValueByString(destinationName.c_str());
  if (eProcessTypeInvalid != processType)
  {
    status = SendSyncToMCMSProcess(processType, command);
    return status;
  }

  cout << "Illegal destination during sync command: " << destinationName <<
  endl << endl;
  return status;
}

STATUS CMcuCmdProcess::SendSyncTerminalCommandToMcms(CTerminalCommand& command) const
{
  STATUS       status = STATUS_FAIL;
  eProcessType destProcessType = eProcessTypeInvalid;
  for (int i = 0; i < NUM_OF_PROCESS_TYPES; i++)
  {
    destProcessType = (eProcessType)i;
    if (eProcessDemo == destProcessType ||
        eProcessTestClient == destProcessType ||
        eProcessMcuCmd == destProcessType ||
        eProcessClientLogger == destProcessType ||
        eProcessCsModule == destProcessType)
      continue;

    status = SendSyncToMCMSProcess(destProcessType, command);
  }

  return status;
}

STATUS CMcuCmdProcess::SendTerminalCommand(CTerminalCommand& command)
{
  STATUS status = STATUS_FAIL;

  // try to send to   "all",
  // "mcms",
  // "cs",
  const string destinationName = command.GetDestinationName();
  CObjString::ToUpper((char*)(destinationName.c_str()));

  SendMethod method = m_SendMethodsMap[destinationName];
  if (NULL != method)
  {
    status = (this->*method)(command);
    return status;
  }

  // try to send to a single MFA
  const char* strBoard = GetDestinationName(eDestMfaBoard);
  // if process name starts will b/B
  bool process =
    (strlen(destinationName.c_str()) > 1 && !isdigit(destinationName[1]));

  if (!process &&
      (0 == strncmp(destinationName.c_str(), strBoard, strlen(strBoard))))
  {
    status = SendToSomeMfa(command);
    return status;
  }

  // try to send to a single mcms process
  eProcessType processType = CProcessBase::GetProcessValueByString(destinationName.c_str());
  if (eProcessTypeInvalid != processType)
  {
    CSegment* pSeg = new CSegment;
    command.Serialize(*pSeg);
    status = SendToMCMSProcess(processType, pSeg);
    return status;
  }

  cout << "Illegal destination: " << destinationName << endl << endl;
  return status;
}

STATUS CMcuCmdProcess::SendToAll(CTerminalCommand& command) const
{
  STATUS status = SendToMCMS(command);
  status = SendToAllMfa(command);
  status = SendToCS(command);

  return status;
}

STATUS CMcuCmdProcess::SendToMCMS(CTerminalCommand& command) const
{
  STATUS       status = STATUS_FAIL;
  eProcessType destProcessType = eProcessTypeInvalid;
  for (int i = 0; i < NUM_OF_PROCESS_TYPES; i++)
  {
    destProcessType = (eProcessType)i;

    CSegment* pSeg = new CSegment;
    command.Serialize(*pSeg);
    status = SendToMCMSProcess(destProcessType, pSeg);
  }

  return status;
}

STATUS CMcuCmdProcess::SendToMCMSProcess(eProcessType destProcessType,
                                         CSegment* seg) const
{
  CManagerApi api(destProcessType);
  STATUS      status =  api.SendMsg(seg, TERMINAL_COMMAND);
  api.DestroyOnlyApi();

  return status;
}

STATUS CMcuCmdProcess::SendSyncToMCMSProcess(eProcessType destProcessType,
                                             CTerminalCommand& command) const
{
  OPCODE   rspOpcode;
  CSegment rspSeg;

  char uniqueName[MAX_QUEUE_NAME_LEN];
  COsQueue::CreateUniqueQueueName(uniqueName);
  COsQueue rspMbxRead;
  rspMbxRead.CreateRead(eProcessMcuCmd, uniqueName);

  CSegment* pSeg = new CSegment;
  *pSeg << uniqueName;
  command.Serialize(*pSeg);

  CManagerApi api(destProcessType);
  STATUS      status =  api.SendMsg(pSeg, SYNC_TERMINAL_COMMAND);
  api.DestroyOnlyApi();

  CMessageHeader rspHeader;
  status = rspMbxRead.Receive(rspHeader, TERMINAL_COMMAND_RSP_TIMEOUT);

  if (STATUS_OK != status)
  {
    cout << endl << "Failed to send sync message to "
         << CProcessBase::GetProcessName((eProcessType)destProcessType)
         << endl;
    cout.flush();
  }
  else
  {
    char* answer = rspHeader.m_segment->GetString();
    cout << endl << answer << endl;
    cout.flush();
    delete [] answer;
  }

  return status;
}

#define CA_COMMAND_SIMULATION "ps -ef | grep acloader | grep -v grep | cut -d'S' -f 2 | cut -d' ' -f 1 2>&1"
#define CA_COMMAND_TARGET     "ps | grep acloader | grep -v grep | cut -d'S' -f 2 | cut -d' ' -f 1 2>&1"

STATUS CMcuCmdProcess::SendToCS(CTerminalCommand& command) const
{
  COstrStream ostr;
  command.Serialize(ostr);

  // Gets numbers of running CSs
  std::string ans;
  char cmd[256] ="";
  if(FALSE == IsTarget())
  {
	  strncpy(cmd, CA_COMMAND_SIMULATION, sizeof(cmd));
  }
  else
  {
      strncpy(cmd, CA_COMMAND_TARGET    , sizeof(cmd));
  }

  STATUS status = SystemPipedCommand(cmd, ans);
  PASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != status,
      "SystemPipedCommand: " << cmd << ": " << ans,
      status);

  // Sends message to running CSs
  std::istringstream iss(ans);
  int sid=0;

  while (!(!(iss >> sid)))
  {
	  if (NULL != getenv ("CS_SIMULATION_TEST"))
	  {
		  //allways one service
		  sid=1;
	  }
    CMplMcmsProtocol mplProt;
    mplProt.AddCommonHeader(CS_TERMINAL_COMMAND_REQ);
    mplProt.AddMessageDescriptionHeader();
    mplProt.AddCSHeader(sid, 0, eConfigurator);

    // 4 bytes of full msg length | msg | \0
    std::string commandStr = ostr.str();
    DWORD       commandLen = commandStr.length() + 1;
    DWORD       fullLen    = sizeof(DWORD) + commandLen;

    char* buffer = new char[fullLen];
    *((DWORD*)buffer) = commandLen;
    memcpy(buffer + sizeof(DWORD), commandStr.c_str(), commandLen);

    mplProt.AddData(fullLen, buffer);
    status = mplProt.SendMsgToCSApiCommandDispatcher();
    PASSERTSTREAM(STATUS_OK != status, "Send failed to SID " << sid);

    PDELETEA(buffer);

    if (IsPrintReport)
      std::cout << "Sent Report" << std::endl
                << "-----------" << std::endl
                << "Data   : "   << commandStr.c_str() << std::endl;

  }

  return STATUS_OK;
}

STATUS CMcuCmdProcess::SendToAllMfa(CTerminalCommand& command) const
{
  DWORD  numOfDestParams = 0;
  STATUS status = SendToMfaUnit(INVALID_BOARD_ID,
                                DefaultSubBoardId,
                                DefaultUnitId,
                                numOfDestParams,
                                command);
  return status;
}

// ca b1 sb1 u1
STATUS CMcuCmdProcess::SendToSomeMfa(CTerminalCommand& command) const
{
  // VNGR-24448: indices wanted are 0 through the index denoted by MAX_..., including.
  // adding the '+ 1' to support such indexing
  bool boardId[MAX_NUM_OF_BOARDS + 1];
  bool subBoardId[MAX_NUM_OF_SUBBOARDS + 1];
  bool unitId[MAX_UNIT_NUMBER + 1];

  DWORD numberOfDestParams = 0;

  memset(boardId, false, sizeof(boardId));
  memset(subBoardId, false, sizeof(subBoardId));
  memset(unitId, false, sizeof(unitId));

  bool resExtract = ExtractMfaDestParams(command, boardId, subBoardId, unitId,
                                         numberOfDestParams);
  if (false == resExtract)
    return STATUS_FAIL;

  STATUS status = STATUS_FAIL;
  bool   successfulSent = false;
  long   timeToSleep = 50;
  for (WORD b = 0; b <= MAX_NUM_OF_BOARDS; ++b)
  {
    if (boardId[b])
      for (WORD sb = 0; sb <= MAX_NUM_OF_SUBBOARDS; ++sb)
      {
        if (subBoardId[sb])
          for (WORD u = 0; u <= MAX_UNIT_NUMBER; ++u)
          {
            if (unitId[u])
            {
              status = SendToMfaUnit(b, sb, u, numberOfDestParams, command);
              // too fast sending will fail
              usleep(timeToSleep);
              if (status != STATUS_OK)
                cout << "SendToMfaUnit failed for board: " << b
                     << "\tsubBoard: " << sb
                     << "\tunit: " << u
                     << endl;
              else
                successfulSent = true;
            }
          }
      }  // end loop subBoard

  }  // end loop board

  // if at least one sending was successful, return OK
  if (!successfulSent)
    return STATUS_FAIL;

  return STATUS_OK;
}

STATUS CMcuCmdProcess::SendToMfaUnit(BYTE boardId,
                                     BYTE subBoardId,
                                     BYTE unitId,
                                     DWORD numDestParams,
                                     CTerminalCommand& command) const
{
  command.SetNumOfDestParams(numDestParams);
  COstrStream ostr;
  command.SerializeMfa(ostr);
  CMplMcmsProtocol mplProt;
  mplProt.AddCommonHeader(TERMINAL_COMMAND);
  mplProt.AddMessageDescriptionHeader();
  mplProt.AddPhysicalHeader(1, boardId, subBoardId, unitId);
  mplProt.AddPortDescriptionHeader(DUMMY_PARTY_ID,
                                   DUMMY_CONF_ID,
                                   DUMMY_CONNECTION_ID);

  string data = ostr.str();
  char   buffer[1024];
  strncpy(buffer, data.c_str(), sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  mplProt.AddData(1024, buffer);

  STATUS status = mplProt.SendMsgToMplApiCommandDispatcher();

  if (IsPrintReport)
    cout << "Sent Report" << endl
         << "-----------" << endl
         << "Board Id : "  << (DWORD)boardId     << endl
         << "SubBoard Id : "  << (DWORD)subBoardId  << endl
         << "Unit Id : "  << (DWORD)unitId      << endl
         << "Data : "  << data.c_str()       << endl;

  return status;
}

bool CMcuCmdProcess::ExtractMfaDestParams(const CTerminalCommand& command,
                                          bool* outBoardId,
                                          bool* outSubBoardId,
                                          bool* outUnitId,
                                          DWORD& numDestParams) const
{
  numDestParams = 0;
  const char* strBoardName = GetDestinationName(eDestMfaBoard);
  const DWORD boardNameLen = strlen(strBoardName);

  const char* strSubBoardName = GetDestinationName(eDestMfaSubBoard);
  const DWORD subBoardNameLen = strlen(strSubBoardName);

  const char* strUnitName = GetDestinationName(eDestMfaUnit);
  const DWORD unitNameLen = strlen(strUnitName);

  const int numOfParams = command.GetNumOfParams();
  const int numOfParamsOffset = numOfParams + eCmdParam1;
  const int limit = MIN_(numOfParamsOffset, eCmdParam3);

  // ca b1 sb1 u1
  // ca b1 u1 sb1
  // ca b1 ???
  for (eCommandParamsIndex i = eCmdDestinationName;
       i < limit; i = (eCommandParamsIndex)(i + 1))
  {
    string refCurrentParam = command.GetToken(i);
    string currentParam = refCurrentParam.c_str();
    CObjString::ToUpper((char*)(currentParam.c_str()));

    if (0 == strncmp(currentParam.c_str(), strBoardName, boardNameLen))
    {
      // ======= 1. determine board to send
      string allBoards(GetDestinationName(eDestMfaBoard));
      allBoards += ALL_POSSIBLE_CHAR;

      // check if it's #, which means broadcast - for all boards
      if (currentParam == allBoards)
        memset(outBoardId, true, MAX_NUM_OF_BOARDS + 1);
      else
      {
        bool result = ParseMfaCommand(currentParam,
                                      eDestMfaBoard,
                                      ALL_POSSIBLE_CHAR,
                                      outBoardId,
                                      MAX_NUM_OF_BOARDS + 1);
        if (!result)
        {
          cout << "FAILED to extract board id: " << currentParam << endl << endl;
          return false;
        }
      }

      numDestParams++;

      // ======= 2. determine subBoard + unit to send
      eCommandParamsIndex idx = (eCommandParamsIndex)(i+1);
      refCurrentParam = command.GetToken(idx);
      currentParam = refCurrentParam.c_str();
      CObjString::ToUpper((char*)(currentParam.c_str()));

      const char* unitPrefix = GetDestinationName(eDestMfaUnit);
      const DWORD unitPrefixLen = strlen(unitPrefix);

      const char* subBoardPrefix = GetDestinationName(eDestMfaSubBoard);
      const DWORD subBoardPrefixLen = strlen(subBoardPrefix);

      // if the next (2nd) param isn't subBoard
      if (0 != strncmp(currentParam.c_str(), subBoardPrefix, subBoardPrefixLen))
      {
        outSubBoardId[1] = true;

        // if the 2nd param isn't Unit, as well
        if (0 != strncmp(currentParam.c_str(), unitPrefix, unitPrefixLen))
          memset(outUnitId, true, MAX_UNIT_NUMBER + 1);
      }
      else     // 2nd param is subBoard
      {
        eCommandParamsIndex idx = (eCommandParamsIndex)(i+2);
        refCurrentParam = command.GetToken(idx);
        currentParam = refCurrentParam.c_str();
        CObjString::ToUpper((char*)(currentParam.c_str()));

        // if "unit" parameter doesn't exist
        if (0 != strncmp(currentParam.c_str(), unitPrefix, unitPrefixLen))
          memset(outUnitId, true, MAX_UNIT_NUMBER + 1);
      }
    }
    else if (0 == strncmp(currentParam.c_str(), strSubBoardName, subBoardNameLen))
    {
      bool result = ParseMfaCommand(currentParam,
                                    eDestMfaSubBoard,
                                    '\0',
                                    outSubBoardId,
                                    MAX_NUM_OF_SUBBOARDS + 1);
      if (!result)
      {
        cout << "FAILED to extract sub board id: " << currentParam << endl << endl;
        return false;
      }

      numDestParams++;
    }
    else if (0 == strncmp(currentParam.c_str(), strUnitName, unitNameLen))
    {
      bool result = ParseMfaCommand(currentParam,
                                    eDestMfaUnit,
                                    '\0',
                                    outUnitId,
                                    MAX_UNIT_NUMBER + 1);
      if (!result)
      {
        cout << "FAILED to extract unit id: " << currentParam << endl << endl;
        return false;
      }

      numDestParams++;
    }
  }

  return true;
}

// Parse numeric value or range value
bool CMcuCmdProcess::ParseToken(const char* token,
                                bool* numbers,
                                int length) const
{
  // check if current token is a range one
  const char* pRange = strchr(token, '-');
  if (pRange)
  {
    int  range[] = {-1, -1};
    bool isRange = GetRange(token, range, length);

    // if it's not a range, a false should be returned, since range char (-) was found.
    if (!isRange)
    {
      cout << "String is invalid, because of the substring: " << token << endl;
      return false;
    }
    else
      for (int i = range[0]; i <= range[1]; ++i)
      {
        numbers[i] = true;
      }

  }
  else // it should be a numeric one
  {
    bool isNum = CObjString::IsNumeric(token);
    int  num = atoi(token);
    if (!isNum || num < 0)
    {
      cout << "Invalid number: " << token << endl;
      return false;
    }

    if (num >= length)
    {
      cout << "Value: " << num << " ( > " << length << " ) is out of range.\n";
      return false;
    }

    numbers[num] = true;
  }

  return true;
}


// Valid complex is one of the following:
// Number: {n|n >= 0, n < "length"}
// Range: < <Number>-<Number> >
// Complex: Number+ / Range+ / {Number*,Range*}+ (at least one Range or Number)
bool CMcuCmdProcess::GetComplex(const char* str,
                                bool* numbers,
                                int length) const
{
  if (!str || !numbers || !strlen(str))
  {
    cout << "String isn't valid " << endl;
    return false;
  }

  // can get range of < <7digids>-<7digits> >, or 15digits number...
  char        token[15]={0};
  const char* pTmp = strchr(str, ',');

  bool isParseOK = true;
  if (!pTmp)
    // should be Number or Range
    isParseOK = ParseToken(str, numbers, length);
  else
  {
    const int maxStrSizeForKW = min(pTmp-str, (int)sizeof(token) - 1);
    strncpy(token, str, sizeof(token)-1);
    token[maxStrSizeForKW] = '\0';

    bool isTokenValid = ParseToken(token, numbers, length);
    if (!isTokenValid)
    {
      cout << "token: " << token << ", isn't valid" << endl;
      return false;
    }

    // Recursively, parse the right expression after the 1st comma of "str"
    isParseOK = GetComplex(pTmp+1, numbers, length);
  }

  return isParseOK;
}

// Range format is : <number>-<number>, e.g. : 2-5
bool CMcuCmdProcess::GetRange(const char* str, int* range, int maxRange) const
{
  if (!str || !range || !strlen(str) || (str[strlen(str) - 1] == '-'))
  {
    cout << "String isn't valid " << endl;
    return false;
  }

  // 10 - to save numbers < 2^32
  char s1[10], s2[10];

  const char* pTmp = strchr(str, '-');

  // check if "-" exists, which means it's range
  if (!pTmp)
  {
    cout << "No range char was found." << endl;
    return false;
  }

  strncpy(s1, str, sizeof(s1)-1);

  s1[pTmp - str] = '\0';

  strncpy(s2, pTmp + 1, sizeof(s2)-1);
  s2[sizeof(s2)-1] = '\0';

  range[0] = atoi(s1);
  range[1] = atoi(s2);

   // Validate the following:
   // -> s1, s2 are numbers
   // -> range are from smaller to bigger (e.i. i-(i+p), p>=0 )
   // -> both range numbers are in the allowed range
  bool isValid = CObjString::IsNumeric(s1) && CObjString::IsNumeric(s2) &&
                 range[0] <= range[1] && maxRange > range[1];

  return isValid;
}

bool CMcuCmdProcess::ParseMfaCommand(const string& strToParse,
                                     eDestinations dest,
                                     const char possibleNonNumeric,
                                     bool* result,
                                     UINT32 resultLength) const
{
  const char* strExpectedDest = GetDestinationName(dest);
  const DWORD expectedDestLen = strlen(strExpectedDest);
  if (strncmp(strExpectedDest, strToParse.c_str(), expectedDestLen))
  {
    cout << "Bad Destination. Expected : " << strExpectedDest << "; Got : " <<
    strToParse << endl;
    return false;
  }

  if (strToParse.length() <= expectedDestLen)
  {
    cout << "Bad Destination. Expected : " << strExpectedDest <<
    " + some number; Got : " << strToParse << endl;
    return false;
  }

  string strResult = strToParse.substr(expectedDestLen, strToParse.length());

  bool isValidExpression = GetComplex(strResult.c_str(), result, resultLength);

  if (!isValidExpression)
  {
    const char ch = ('\0' == possibleNonNumeric ? ' ' : possibleNonNumeric);
    cout << "Bad Destination, extension must be numeric(1,2,1-2) and up to " <<
    resultLength-1 << " ";

    if ('\0' != possibleNonNumeric)
      cout << "OR [" << ch << "].";

    cout << "not : " << strResult.c_str() << endl;
    return false;
  }

  return true;
}

void CMcuCmdProcess::PrintCharArray(const char* arg, int num) const
{
  for (int i = 0; i < num; i++)
  {
    cout << "arg[" << i << "] = " << arg[i] << endl;
  }
}

void CMcuCmdProcess::AddUnitIdToCommandToMfa(CTerminalCommand& command) const
{
  DWORD numOfParams = command.GetNumOfParams();
  if (0 == numOfParams)
  {
    command.AddToken("U0");
    return;
  }

  const string param1 = command.GetToken(eCmdParam1);
  CObjString::ToUpper((char*)(param1.c_str()));

  const char* strExpectedDest = GetDestinationName(eDestMfaUnit);
  const DWORD expectedDestLen = strlen(strExpectedDest);
  if (0 == strncmp(strExpectedDest, param1.c_str(), expectedDestLen))
    return;

  command.InsertToken("U0", eCmdParam1);
}
