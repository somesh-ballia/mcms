// SNMPProcessManager.h

#ifndef SNMP_PROCESS_MANAGER_H_
#define SNMP_PROCESS_MANAGER_H_

#include <vector>

#include "StringsLen.h"
#include "SnmpData.h"
#include "ManagerTask.h"
#include "Macros.h"
#include "InitCommonStrings.h"
#include "ifTable.h"
#include <set>

void SNMPProcessManagerEntryPoint(void* appParam);

class CTelemetry;

class CSNMPProcessManager : public CManagerTask, CNonCopyable
{
  CLASS_TYPE_1(CSNMPProcessManager, CManagerTask)

 public:
                 CSNMPProcessManager();
  virtual       ~CSNMPProcessManager();
  virtual void   SelfKill();
  virtual void   ManagerStartupActionsPoint();
  virtual void   DeclareStartupConditions();
  STATUS         HandleUpdateSnmpData(CRequest* pGetRequest);
  STATUS         HandleSNMPD_WD(CSegment* pMsg);
  STATUS 		HandleFixSNMPD(CSegment* pMsg);

  STATUS         HandleMngmntIpConfigInd(CSegment* pMsg);
  STATUS         HandleCsIpInd(CSegment* pMsg);
  STATUS         HandleMfaIpInd(CSegment* pMsg);
  STATUS         HandleSnmpAgentReadyInd(CSegment* pMsg);
  STATUS         HandleSnmpUpdateTelemetryInd(CSegment* pMsg);
  STATUS         HandleSnmpUpdateMultipleTelemetryInd(CSegment* pMsg);
  void           HandleSnmpGetTelemetryReq(CSegment* seg);
  void	        TrigerSNMPConfigToOtherProcess(CSegment* seg);
  TaskEntryPoint GetMonitorEntryPoint();

 private:

  typedef enum {FIX_SNMPD_NONE, FIX_SNMPD_RECONFIGURE, FIX_SNMPD_RERUN} FixSNMPDEnum;

  bool CheckSnmpMFWTraps(CSnmpData& snmpData, std::string& errorMessage, bool deleteInvalid = false) const;

  bool CheckSNMPTraps(CSnmpData& snmpData, std::string& errorMessage, bool deleteInvalid = false) const;

  bool CheckDataForMfw(const CSnmpData& snmpData, std::string& errorMessage) const;

  bool CheckSNMPData(const CSnmpData& snmpData, std::string& errorMessage) const;

  bool CheckAuthAgentIsSHA(const CSnmpData& snmpData, std::string& errorMessage) const;

  bool CheckPrivAgentIsAES(const CSnmpData& snmpData, std::string& errorMessage) const;
  bool CheckAuthTrapIsSHA(const CSnmpTrapCommunity& trapCommunity, std::string& errorMessage) const;
  bool CheckPrivTrapIsAES(const CSnmpTrapCommunity& trapCommunity, std::string& errorMessage) const;
  bool CheckGeneralTrapData(const CSnmpTrapCommunity& trapCommunity, std::string& errorMessage) const;


  static std::vector<unsigned char> SNMPDateAndTime(const char* date);
  static bool                       IsProcessUp();

  STATUS         ConfigureSnmpDaemon(bool isNeedReset) const;
  void           UpdateManagerIp(CSegment* pSeg);
  void           CsUpdateSnmp(CSegment* pSeg);
  void 			 UpdateSnmp_MNGMNT(const string& mngmntAddress, const byte* ipv6Address, bool isMngmtIpv6Address);
  void           UpdateSnmp_CS(const string& csAddress);
  void           UpdateSnmp_GK(const string& gkAddress);
  void 			 UpdateSnmp_Switch(const string& shelfAddress, const byte* ipv6Address, bool isShelfIpv6Address);
  void           UpdateSnmp_Mfa1(const string& mfa1Address);
  void           UpdateSnmp_Mfa2(const string& mfa2Address);
  void           UpdateSnmp_Mfa3(const string& mfa3Address);
  void           UpdateSnmp_Mfa4(const string& mfa4Address);
  void           RestartSnmpd() const;
  void           HardRestartSnmpd(bool enable) const;
  bool           DoesNeedHardReset(const CSnmpData& snmpDataPrev, const CSnmpData& snmpDataNew);

  void           UpdateSysFlagBool(const char* flag, eTelemetryType type);
  void           UpdateSysFlagString(const char* flag, eTelemetryType type);
  void           ConvertSysFlagStringToIntAndUpdate(const char* flag, eTelemetryType type);

  STATUS         HandleTerminalMakeMibFile(CTerminalCommand& command,
                                           std::ostream& answer);
  STATUS         HandleTerminalUpdateGk(CTerminalCommand& command,
                                        std::ostream& answer);
  STATUS         HandleTerminalAddTrap(CTerminalCommand& command,
                                       std::ostream& answer);
  STATUS         HandleTerminalAddTelemetry(CTerminalCommand& command,
                                            std::ostream& answer);
  STATUS         HandleTerminalGetTelemetry(CTerminalCommand& command,
                                            std::ostream& answer);
  
  void           SendSNMPConfigToOtherProcess(bool enabled);
  void           SendSNMPConfigToOtherProcess(eProcessType ptype, bool enabled);
  void 			 ConfigSNMPState();

  inline bool IsSNMPEnabled() const;


  STATUS HandleMfaLinkStatusInd(CSegment* pMsg);
  STATUS HandleCsLinkStatusInd(CSegment* pMsg);

  //void AddInterfaceIndex(WORD 	boardId, WORD   portId, eInterfaceIndex interfaceIndex);
  //eInterfaceIndex GetInterfaceIndex(DWORD 	boardId, WORD   portId) const;
  void AddInterfaceIndex(WORD 	boardId, eInterfaceIndex interfaceIndex);
  eInterfaceIndex GetInterfaceIndex(DWORD 	boardId) const;

  
  void SendLinkUpDownTrap(BOOL isUp, unsigned int ifIndex) const;

  void 			NotifyAllReadyProcessesOnReady(bool enabled);
  OPCODE 	WriteSNMPFile(const std::string& fname, const std::string& content, mode_t mode) const;


  void FixSNMPIpInConfFIleIfNeed();

  bool ValidatePassword(const CSnmpV3SecurityParams& snmpV3SecurityParams, std::string& errorMessage) const;


  DWORD       	m_MngmntIp;
  bool 			m_configForIpv6;
  CTelemetry* 	m_telemetry;
  bool 			m_isFipsMode;
  bool			m_snmpReady;
  FixSNMPDEnum 		m_fixSNMPDState;
  int 			m_triesFix;

  std::set<unsigned int> m_readyProcesses;

  // check how to support with ports
  // std::map< std::pair<DWORD, WORD>, eInterfaceIndex > m_boarPortToInterfaceIndex;
  std::map< WORD, eInterfaceIndex > m_boardToInterfaceIndex;

  PDECLAR_MESSAGE_MAP;
  PDECLAR_TRANSACTION_FACTORY;
  PDECLAR_TERMINAL_COMMANDS;
};

#endif  // SNMP_PROCESS_MANAGER_H_
