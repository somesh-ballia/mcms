// McmsProcesses.h

#ifndef MCMS_PROCESSES_H_
#define MCMS_PROCESSES_H_

#include "Macros.h"

// Every change in this enum should be updated also in:
// 1. file Processes/McmsDaemon/McmsDaemonLib/McmsDaemonProcess.cpp ( function IsWatched(eProcessType process) )
// 2. file Processes/McmsDaemon/McmsDaemonLib/McmsDaemonProcess.cpp ( function IsLaunched(eProcessType process) )
// 3. Processes/McmsDaemon/McmsDaemonLib/ProcessPolicy.cpp ( function CProcessPolicy::GetFaultedPolicy )
// 4. The following string array (ProcessNames[]) respectively.
//
// It is recommended to add new processes before eProcessApacheModule.
enum eProcessType
{
	eProcessTypeInvalid = -1,
	eProcessMcmsDaemon, // Should be always first
	eProcessLogger,
	eProcessFaults,
	eProcessIPMCInterface,
	eProcessMcmsNetwork,
	eProcessConfigurator,
	eProcessAuditor,
	eProcessSNMPProcess,
	eProcessCSMngr,
	eProcessCertMngr,
	eProcessMcuMngr,
	eProcessLicenseServer,
	eProcessConfParty,
	eProcessCards,
	eProcessCDR,
	eProcessResource,
	eProcessSipProxy,	
	eProcessDNSAgent,
	eProcessGatekeeper,
	eProcessQAAPI,
	eProcessExchangeModule,
	eProcessEncryptionKeyServer,
	eProcessAuthentication,
	eProcessInstaller,
	eProcessRtmIsdnMngr,
	eProcessBackupRestore,
	eProcessMplApi,
	eProcessCSApi,
	eProcessCollector,
	eProcessSystemMonitoring,
	eProcessMediaMngr,
	eProcessFailover,
	eProcessLdapModule,
	eProcessUtility,
	eProcessMCCFMngr,
	eProcessNotificationMngr,
	eProcessIce,
	eProcessApacheModule,
	eProcessGideonSim,
	eProcessEndpointsSim,
	eProcessTestClient,
	eProcessMcuCmd,
	eProcessClientLogger,
	eProcessDemo,
	eProcessDiagnostics,
	eProcessCsModule,

	// Every change in this enum should be updated also in:
	// 1. file Processes/McmsDaemon/McmsDaemonLib/McmsDaemonProcess.cpp ( function IsWatched(eProcessType process) )
	// 2. file Processes/McmsDaemon/McmsDaemonLib/McmsDaemonProcess.cpp ( function IsLaunched(eProcessType process) )
	// 3. Processes/McmsDaemon/McmsDaemonLib/ProcessPolicy.cpp ( function CProcessPolicy::GetFaultedPolicy )
	// 4. The following string array (ProcessNames[]) respectively.
	//
	// It is recommended to add new processes before eProcessApacheModule.

	// Must be last
	NUM_OF_PROCESS_TYPES
};

// Allows iterations on the enum, as ++i
inline eProcessType& operator++(eProcessType& type)
{
	return enum_increment(type, eProcessTypeInvalid, NUM_OF_PROCESS_TYPES);
}

static const char* ProcessNames[] =
{
	"McmsDaemon",           // eProcessMcmsDaemon
	"Logger",               // eProcessLogger
	"Faults",               // eProcessFaults
	"IPMCInterface",        // eProcessIPMCInterface
	"McmsNetwork",			// eProcessMcmsNetwork
	"Configurator",         // eProcessConfigurator
	"Auditor",              // eProcessAuditor
	"SNMPProcess",          // eProcessSNMPProcess
	"CSMngr",               // eProcessCSMngr
	"CertMngr",             // eProcessCertMngr
	"McuMngr",              // eProcessMcuMngr
	"LicenseServer",        // eProcessLicenseServer
	"ConfParty",            // eProcessConfParty
	"Cards",                // eProcessCards
	"CDR",                  // eProcessCDR
	"Resource",             // eProcessResource
	"SipProxy",             // eProcessSipProxy	
	"DNSAgent",             // eProcessDNSAgent
	"Gatekeeper",           // eProcessGatekeeper
	"QAAPI",                // eProcessQAAPI
	"ExchangeModule",       // eProcessExchangeModule,
	"EncryptionKeyServer",  // eProcessEncryptionKeyServer
	"Authentication",       // eProcessAuthentication
	"Installer",            // eProcessInstaller
	"RtmIsdnMngr",          // eProcessRtmIsdnMngr
	"BackupRestore",        // eProcessBackupRestore
	"MplApi",               // eProcessMplApi
	"CSApi",                // eProcessCSApi
	"Collector",            // eProcessCollector
	"SystemMonitoring",     // eProcessSystemMonitoring
	"MediaMngr",            // eProcessMediaMngr
	"Failover",             // eProcessFailover
	"LdapModule",           // eProcessLdapModule
	"Utility" ,             // eProcessUtility
	"MCCFMngr",             // eProcessMCCFMngr
	"NotificationMngr",     // eProcessNotificationMngr
	"Ice",                  // eProcessIce
	"ApacheModule",         // eProcessApacheModule
	"GideonSim",            // eProcessGideonSim
	"EndpointsSim",         // eProcessEndpointsSim
	"TestClient",           // eProcessTestClient
	"McuCmd",               // eProcessMcuCmd
	"ClientLogger",         // eProcessClientLogger
	"Demo",                 // eProcessDemo
	"Diagnostics",          // eProcessDiagnostics
	"CS",                   // eProcessCsModule
};

inline const char* ProcessTypeToStr(eProcessType type)
{
	return (type >= 0 && static_cast<unsigned int>(type) < ARRAYSIZE(ProcessNames))
		? ProcessNames[type] : "InvalideProcess";
}

#endif  // MCMS_PROCESSES_H_
