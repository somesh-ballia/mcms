// McuCmdProcess.h

#ifndef MCU_CMD_PROCESS_H_
#define MCU_CMD_PROCESS_H_

#include <map>
#include <string>

#include "NStream.h"
#include "ProcessBase.h"
#include "TerminalCommand.h"

class CSegment;
class CMcuCmdProcess;

typedef STATUS (CMcuCmdProcess::* SendMethod)(CTerminalCommand& command) const;
typedef std::map<const std::string, SendMethod> CSendMethodsMap;

enum eDestinations
{
  eDestAll,
  eDestMCMS,
  eDestCS,
  eDestMfaBoard,
  eDestMfaSubBoard,
  eDestMfaUnit,

  NumOfDestinations
};

static const char* GetDestinationName(eDestinations dest)
{
  static char* names[] =
  {
    "ALL",   // eDestAll
    "MCMS",  // eDestMCMS
    "CS",    // eDestCS
    "B",     // eDestMfaBoard
    "SB",    // eDestMfaSubBoard
    "U"      // eDestMfaUnit
  };

  return (dest >= eDestAll && (unsigned int) dest < ARRAYSIZE(names)
      ? names[dest] : "Invalid");
}

class CMcuCmdProcess : public CProcessBase
{
  friend class CTestMcuCmdProcess;
  CLASS_TYPE_1(CMcuCmdProcess, CProcessBase);
public:
  CMcuCmdProcess();
  virtual ~CMcuCmdProcess();
  virtual eProcessType   GetProcessType() {return eProcessMcuCmd;}
  virtual BOOL           UsingSockets()   {return NO;}
  virtual TaskEntryPoint GetManagerEntryPoint();
  virtual int            Run();

private:
  STATUS                 SendTerminalCommand(CTerminalCommand& command);
  STATUS                 SendSyncTerminalCommand(CTerminalCommand& command);
  STATUS                 SendSyncToMCMSProcess(eProcessType destProcessType,
                                               CTerminalCommand& command) const;
  STATUS                 SendSyncTerminalCommandToMcms(CTerminalCommand& command) const;
  bool                   IsCommandSync(CTerminalCommand& command);
  STATUS                 SendToAll(CTerminalCommand& command) const;
  STATUS                 SendToMCMS(CTerminalCommand& command) const;
  STATUS                 SendToCS(CTerminalCommand& command) const;
  STATUS                 SendToSomeMfa(CTerminalCommand& command) const;
  STATUS                 SendToAllMfa(CTerminalCommand& command) const;
  STATUS                 SendToMCMSProcess(eProcessType destProcessType, CSegment* seg) const;
  STATUS                 SendToMfaUnit(BYTE boardId, BYTE subBoardId,
                                       BYTE unitId, DWORD numDestParams,
                                       CTerminalCommand& command) const;
  bool                   ParseMfaCommand(const string& strToParse,
                                         eDestinations dest,
                                         const char possibleNonNumeric,
                                         bool* result,
                                         UINT32 resultLength) const;
  bool                   GetRange(const char* str, int* range, int maxRange) const;
  bool                   GetComplex(const char* str, bool* numbers, int length) const;
  bool                   ParseToken(const char* token, bool* numbers, int length) const;
  void                   AddUnitIdToCommandToMfa(CTerminalCommand& command) const;
  void                   PrintCharArray(const char* arg, int num) const;
  bool                   ExtractMfaDestParams(const CTerminalCommand& command,
                                              bool* outBoardId,
                                              bool* outSubBoardId,
                                              bool* outUnitId,
                                              DWORD& numDestParams) const;

  CSendMethodsMap m_SendMethodsMap;
};

#endif
