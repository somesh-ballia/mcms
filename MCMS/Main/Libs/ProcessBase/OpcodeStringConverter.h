// OpcodeStringConverter.h

#ifndef OPCODE_STRING_CONVERTER_H_
#define OPCODE_STRING_CONVERTER_H_

#include <string>
#include "ConvertorBase.h"

class COpcodeStringConverter : public CConvertorBase<OPCODE, std::string>
{
 public:
                      COpcodeStringConverter();

  virtual const char* NameOf() const { return "COpcodeStringConverter";}
  const std::string&  GetStringByOpcode(OPCODE opcode);
  void                AddOpcodeString(OPCODE opcode, const std::string& str);

 private:
  void                InitAllOpcodeString();
  void                InitOpcodeStringCommon();

  void                InitOpcodeStringProcessApacheModule();
  void                InitOpcodeStringProcessAuditor();
  void                InitOpcodeStringProcessAuthentication();
  void                InitOpcodeStringProcessBackupRestore();
  void                InitOpcodeStringProcessCards();
  void                InitOpcodeStringProcessCDR();
  void                InitOpcodeStringProcessCertMngr();
  void                InitOpcodeStringProcessClientLogger();
  void                InitOpcodeStringProcessCollector();
  void                InitOpcodeStringProcessConfigurator();
  void                InitOpcodeStringProcessConfParty();
  void                InitOpcodeStringProcessCS();
  void                InitOpcodeStringProcessCSApi();
  void                InitOpcodeStringProcessCSMngr();
  void                InitOpcodeStringProcessDemo();
  void                InitOpcodeStringProcessDiagnostics();
  void                InitOpcodeStringProcessDNSAgent();
  void                InitOpcodeStringProcessEncryptionKeyServer();
  void                InitOpcodeStringProcessEndpointsSim();
  void                InitOpcodeStringProcessExchangeModule();
  void                InitOpcodeStringProcessFailover();
  void                InitOpcodeStringProcessGideonSim();
  void                InitOpcodeStringProcessGatekeeper();
  void                InitOpcodeStringProcessInstaller();
  void                InitOpcodeStringProcessIPMCInterface();
  void                InitOpcodeStringProcessLdapModule();
  void                InitOpcodeStringProcessLogger();
  void                InitOpcodeStringProcessMcmsDaemon();
  void                InitOpcodeStringProcessMcmsFaults();
  void                InitOpcodeStringProcessMcuCmd();
  void                InitOpcodeStringProcessMcuMngr();
  void                InitOpcodeStringProcessMediaMngr();
  void                InitOpcodeStringProcessMplApi();
  void                InitOpcodeStringProcessQAAPI();
  void                InitOpcodeStringProcessResource();
  void                InitOpcodeStringProcessRtmIsdnMngr();
  void                InitOpcodeStringProcessSipProxy();
  void                InitOpcodeStringProcessSNMPProcess();
  void                InitOpcodeStringProcessSystemMonitoring();
  void                InitOpcodeStringProcessTestClient();
  void                InitOpcodeStringProcessTestServer();
  void                InitOpcodeStringProcessUtility();
  void                InitOpcodeStringProcessMCCFMngr();
  void                InitOpcodeStringProcessLicenseServer();
};

#endif
