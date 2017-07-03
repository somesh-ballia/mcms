//
//************************************************************************
//   (c) Polycom
//
//  Name:   McmsNetworkManager.h
//
//  Description:
/*
 *
 *
 *	 Flags:
 *
 *   Documentation List :
 *   HLD-
 *   	1. http://isrportal07/sites/rd/carmel/Restricted%20Documents/SW%20Department/Management/V100/ReWrite/Startup%20New%20design.pptx
 *   TestList
 *
 */
//
//  Revision History:
//
//  Date            Author           Functional/Interface Changes
//  -----------    -------------    ----------------------------------------
//  July 18, 2013    stanny			  created class
//************************************************************************

#ifndef MCMSNETWORK_MANAGER_H_
#define MCMSNETWORK_MANAGER_H_

#include "ManagerTask.h"
#include "Macros.h"
#include "McmsNetworkProcess.h"
#include "CNetworkFactory.h"

void McmsNetworkManagerEntryPoint(void* appParam);

class CMcmsNetworkManager : public CManagerTask, CNonCopyable
{
  CLASS_TYPE_1(CMcmsNetworkManager, CManagerTask)

 public:
  CMcmsNetworkManager();
  virtual       ~CMcmsNetworkManager();
  void           ManagerPostInitActionsPoint();
  TaskEntryPoint GetMonitorEntryPoint();

 private:
  CMcmsNetworkProcess*  m_pProcess;
  BOOL					m_McmsDaemonInd;
  McmsNetworkPackage::CManagmentNetwork*  	m_pMngnt_net;
  McmsNetworkPackage::CSignalMediaNetwork* m_pSgnlMd_net;
  DWORD					m_wIpv6AutoConfigTimeOut;
  DWORD 				m_IpV6AutoConfigAccumulatedTime;

  STATUS                m_eLastMngmtStatus;
  STATUS                m_eLastSgnlMdStatus;
  DWORD					m_wAutoNetworkSpend;


  void OnTimerSelfKill(CSegment* pMsg);
  void OnTimerIpv6AutoConfig(CSegment* pMsg);
  void OnTimerSoftVMWaitingForIp(CSegment* pMsg);
  void OnManagmentStatusConfig(STATUS status);
  void RunStartup();
  STATUS ConfigureManagementNetwork();
  STATUS ConfigureSignalMediaNetwork();
  void OnSignalMediaStatusConfig(STATUS status);
  void SendFinishMsgToMcmsDaemon();
  void InitReadSysFlags();
  void OnRecoverStatus(STATUS status);
  STATUS TryToRecover();



  PDECLAR_MESSAGE_MAP
  PDECLAR_TRANSACTION_FACTORY
  PDECLAR_TERMINAL_COMMANDS
};

#endif

