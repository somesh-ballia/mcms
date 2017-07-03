// DemoManager.cpp

#include "McmsNetworkManager.h"

#include "Trace.h"
#include "TraceStream.h"
//#include "FipsMode.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "McmsDaemonApi.h"
#include "NetCommonDefines.h"
#include "FaultsDefines.h"


using namespace McmsNetworkPackage;

//#define MCMS_NETWORK_TEST_TIMER 		 	1001 //for develop and testing
#define MCMSNETWORK_IPV6_AUTO_CONFIG_TIMER  1002 // for waiting for ipv6 autoconfig
#define MCMSNETWORK_SELF_KILL_TIMER  		1003 // for self kill once mcmnetwork is finish and sent active alarms if any to mcu manager it job is done.
#define MCMS_NETWORK_TEST_INTERVAL       	5 * SECOND  // wait for 5 seconds
#define MCMS_NETWORK_IPV6_AUTO_INTERVAL     2 * SECOND
#define MCMSNETWORK_SELF_KILL_INTERVAL		60 *SECOND
PBEGIN_MESSAGE_MAP(CMcmsNetworkManager)
  ONEVENT(XML_REQUEST, IDLE, CMcmsNetworkManager::HandlePostRequest)
  //ONEVENT(MCMS_NETWORK_TEST_TIMER, ANYCASE, CMcmsNetworkManager::onTimerTest)
  ONEVENT(MCMSNETWORK_IPV6_AUTO_CONFIG_TIMER, ANYCASE, CMcmsNetworkManager::OnTimerIpv6AutoConfig)
  ONEVENT(MCMSNETWORK_SELF_KILL_TIMER, ANYCASE, CMcmsNetworkManager::OnTimerSelfKill)
  ONEVENT(MCUMNGR_VM_IP_READY_TIMER,	ANYCASE,  CMcmsNetworkManager::OnTimerSoftVMWaitingForIp )
PEND_MESSAGE_MAP(CMcmsNetworkManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CMcmsNetworkManager)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CMcmsNetworkManager)
END_TERMINAL_COMMANDS



extern void McmsNetworkMonitorEntryPoint(void* appParam);

void McmsNetworkManagerEntryPoint(void* appParam)
{
  CMcmsNetworkManager* mngr = new CMcmsNetworkManager;
  mngr->Create(*(CSegment*)appParam);
}

TaskEntryPoint CMcmsNetworkManager::GetMonitorEntryPoint()
{
  return McmsNetworkMonitorEntryPoint;
}

CMcmsNetworkManager::CMcmsNetworkManager()
{
	m_pProcess = (CMcmsNetworkProcess*)CMcmsNetworkProcess::GetProcess();
	m_McmsDaemonInd = FALSE;
	m_pMngnt_net	= NULL;
	m_pSgnlMd_net = NULL;
	m_wIpv6AutoConfigTimeOut = 40  * SECOND ;
	m_IpV6AutoConfigAccumulatedTime =0;
	m_eLastMngmtStatus = STATUS_OK;
	m_eLastSgnlMdStatus = STATUS_OK;
	m_wAutoNetworkSpend = 0;
	InitReadSysFlags();
}

CMcmsNetworkManager::~CMcmsNetworkManager()
{}

void CMcmsNetworkManager::InitReadSysFlags()
{
	CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (pSysConfig)
	{
		pSysConfig->GetDWORDDataByKey(CFG_IPV6_AUTO_CONFIG_TIMEOUT,m_wIpv6AutoConfigTimeOut);
		m_wIpv6AutoConfigTimeOut = m_wIpv6AutoConfigTimeOut * SECOND;
	}
}

void CMcmsNetworkManager::SendFinishMsgToMcmsDaemon()
{
	STATUS status = STATUS_OK;
	TRACESTR(eLevelInfoNormal) << "Create Network Configuration file and Send Finish Message to McmsDaemon  ";
	status = CNetworkFactory::GetInstance().WriteNetworkConfigurationXML();
	if(STATUS_OK != status)
		 TRACESTR(eLevelWarn) <<  "Warning Writing Network Configuration status : " << m_pProcess->GetStatusAsString(status).c_str();
	m_McmsDaemonInd = TRUE;
	CMcmsDaemonApi api;
	api.SendOpcodeMsg(MCMSNETWORK_TO_MCMSDAEMON_FINISH);
}

void CMcmsNetworkManager::OnManagmentStatusConfig(STATUS status)
{
		TRACEINTO << "\n OnManagmentStatusConfig  IsMngmntInterfaceReady- status: " << m_pProcess->GetStatusAsString(status).c_str();
		 switch(status)
		 {
				 case MANGMENT_STATUS_INTERFACE_READY:
					 status = m_pMngnt_net->ConfigOtherNetComponents();
					 //configure signal/media network
					 m_eLastSgnlMdStatus = ConfigureSignalMediaNetwork();
					 OnSignalMediaStatusConfig(m_eLastSgnlMdStatus);

					 break;
				 case MANGMENT_STATUS_INTERFACE_WAITING_FOR_IPV6:
					 StartTimer(MCMSNETWORK_IPV6_AUTO_CONFIG_TIMER, MCMS_NETWORK_IPV6_AUTO_INTERVAL);
					 TRACEINTO << "\nCMcmsNetworkManager::OnManagmentStatusConfig - currently failed to retrieve auto ipV6; re attempting...";
					 break;
				 case MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV6:
				 case MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV4:
					 //raise active alarm
					 if(MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV6 == status)
					 {
						 m_pMngnt_net->SetNetworkMngmtConfigStatus(eNetConfigurationFailureActionChangeMngntIp4Type);
						 status = MANGMENT_STATUS_IPTYPE_CHANGE_FROM_BOTH_TO_IPV4;
					 }
					 AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,AA_NO_MANAGEMENT_IP_INTERFACE,
					 						                      MAJOR_ERROR_LEVEL,m_pProcess->GetStatusAsString(status).c_str(),
					 						                      true,true);
					 TRACESTR(eLevelWarn) <<  "Failed to config interface, status : " << m_pProcess->GetStatusAsString(status).c_str();
					 status = m_pMngnt_net->ConfigOtherNetComponents();
					 SendFinishMsgToMcmsDaemon();
					 StartTimer(MCMSNETWORK_SELF_KILL_TIMER, MCMSNETWORK_SELF_KILL_INTERVAL);
				 	break;
				 case MANGMENT_STATUS_INTERFACE_ERROR_CONFILCT_IPV4:
				 case MANGMENT_STATUS_INTERFACE_ERROR_CONFILCT_IPV6:
					 TRACESTR(eLevelWarn) <<  "Conflicats on Ipv4 or ipv6 configuration status : " << m_pProcess->GetStatusAsString(status).c_str();
					 //raise active alarm ?
					 status = m_pMngnt_net->ConfigOtherNetComponents();
					 SendFinishMsgToMcmsDaemon();
					 StartTimer(MCMSNETWORK_SELF_KILL_TIMER, MCMSNETWORK_SELF_KILL_INTERVAL);
					 break;
				case  MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV4:
				case  MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV6:
					 StartTimer(MCUMNGR_VM_IP_READY_TIMER, MCMS_NETWORK_IPV6_AUTO_INTERVAL);
					 TRACEINTO << "\nCMcmsNetworkManager::OnManagmentStatusConfig - currently failed to retrieve auto ipV4/6 for vm ; re attempting...";
					 break;
				case PLATFORM_STATUS_UNKNOWN_PRODUCT_TYPE:
					TRACEINTO << "\nCMcmsNetworkManager::OnManagmentStatusConfig - unknown product type";
					SendFinishMsgToMcmsDaemon();
					StartTimer(MCMSNETWORK_SELF_KILL_TIMER, MCMSNETWORK_SELF_KILL_INTERVAL);
					break;
				default:
					OnRecoverStatus(status);
					break;
		}


}

void CMcmsNetworkManager::OnSignalMediaStatusConfig(STATUS status)
{
	TRACEINTO << "\n OnSignalMediaStatusConfig - status: " << m_pProcess->GetStatusAsString(status).c_str();
	switch(status)
	{
	case STATUS_OK:
		SendFinishMsgToMcmsDaemon();
		StartTimer(MCMSNETWORK_SELF_KILL_TIMER, MCMSNETWORK_SELF_KILL_INTERVAL);
		break;
	case SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV4:
	case SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV6:
		// start timer to wait for IPv6 auto configuration
		StartTimer(MCUMNGR_VM_IP_READY_TIMER, MCMS_NETWORK_IPV6_AUTO_INTERVAL);
		TRACEINTO << "CMcmsNetworkManager::OnSignalMediaStatusConfig - currently failed to retrieve auto ipv4/6 for vm; re attempting...";
		break;
	default:
		OnRecoverStatus(status);
		break;
	}
}


void CMcmsNetworkManager::OnTimerIpv6AutoConfig(CSegment* pMsg)
{

	m_IpV6AutoConfigAccumulatedTime += MCMS_NETWORK_IPV6_AUTO_INTERVAL;
	TRACEINTO << "\nCMcmsNetworkManager::OnTimerIpv6AutoConfig - accumulated time: "
				  << (m_IpV6AutoConfigAccumulatedTime/SECOND) << " seconds  "
				  << (m_wIpv6AutoConfigTimeOut/SECOND) << "max time till timeout in seconds";

		// ===== 1. timeout was not reached yet - keep on sampling...
		if(m_IpV6AutoConfigAccumulatedTime < m_wIpv6AutoConfigTimeOut)
		{
			m_eLastMngmtStatus =  m_pMngnt_net->IsMngmntInterfaceReady(eIpType_IpV6);
			TRACEINTO << "\n OnTimerIpv6AutoConfig  IsMngmntInterfaceReady ipv6 - status: " << m_pProcess->GetStatusAsString(m_eLastMngmtStatus).c_str();
			OnManagmentStatusConfig(m_eLastMngmtStatus);
		}
		// ===== 2. timeout is reached!
		else
		{
			TRACEINTO << "\nCMcmsNetworkManager::OnTimerIpv6AutoConfig - failed to retrieve auto ipV6; giving up";
			if (false == IsRmxSimulation())
			{
				PASSERTMSG(1, " Failed to configure auto ipV6");
			}
			OnManagmentStatusConfig(MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV6);
		}
}

void CMcmsNetworkManager::OnRecoverStatus(STATUS status)
{
	TRACEINTO << "\n CMcmsNetworkManager::OnRecoverStatus - status: " << m_pProcess->GetStatusAsString(status).c_str();
	switch(status)
	{
	case PLATFORM_STATUS_MNGMNT_IS_NULL:
	case PLATFORM_STATUS_SGNLMD_ERR_CONFIG:
	case STATUS_MUST_ASSIGN_INTERFACE_IN_MFW:
	case STATUS_IP_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_IPV6_SITE_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_IPV6_LINK_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_NONE_IPV4_IPV6_ADDRESS:
		TryToRecover();
		break;
	}

}

void CMcmsNetworkManager::ManagerPostInitActionsPoint()
{
	RunStartup();
}

STATUS CMcmsNetworkManager::ConfigureManagementNetwork()
{
	STATUS stat = STATUS_OK;
	STATUS readFileStatus  = STATUS_OK;
	CIPService* pMngmntService = CNetworkFactory::GetInstance().ReadMngmntService(readFileStatus);

	 m_pMngnt_net = CNetworkFactory::GetInstance().CreateMngmntNetwork(m_pProcess->GetProductType());
	 if(m_pMngnt_net)
	 {
		 TRACESTR(eLevelDebug) << "CManagmentNetwork was created for Product " <<  ProductTypeToString(m_pProcess->GetProductType());
		 stat = m_pMngnt_net->ConfigNetworkMngmnt(pMngmntService);
		 // send indication mcms daemon that we are done

		 if(STATUS_OK == stat)
		 {
			 //check if Management interfaces are ready
			 stat =  m_pMngnt_net->IsMngmntInterfaceReady();
			 //OnManagmentStatusConfig(statusMngmt);

		 }
		 //StartTimer(MCMSNETWORK_SELF_KILL_TIMER, MCMSNETWORK_SELF_KILL_INTERVAL);

	 }
	 else
	 {
		 TRACESTR(eLevelError) << "Failed to create  CManagmentNetwork object for product " << ProductTypeToString(m_pProcess->GetProductType());
		 //SendFinishMsgToMcmsDaemon();
		 return PLATFORM_STATUS_UNKNOWN_PRODUCT_TYPE;
	 }


	return stat;
}

STATUS CMcmsNetworkManager::ConfigureSignalMediaNetwork()
{
	STATUS stat = STATUS_OK;
	m_pSgnlMd_net = CNetworkFactory::GetInstance().CreateSignalMediaNetwork(m_pProcess->GetProductType());
	if(m_pSgnlMd_net)
	{
		stat = m_pSgnlMd_net->ConfigNetworkSgnlMd();
	}
	else
	{
		TRACESTR(eLevelError) << "Failed to create CSignalMediaNetwork object for product "<< ProductTypeToString(m_pProcess->GetProductType());
		return PLATFORM_STATUS_UNKNOWN_PRODUCT_TYPE;
	}

	return stat;
}


void CMcmsNetworkManager::RunStartup()
{

	// configure management network
	m_eLastMngmtStatus = ConfigureManagementNetwork();
	OnManagmentStatusConfig(m_eLastMngmtStatus);
	// due to BRIDGE-13995 call to the signalling configuration functions was moved to OnManagmentStatusConfig on MANGMENT_STATUS_INTERFACE_READY status

}

void CMcmsNetworkManager::OnTimerSelfKill(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "OnTimerSelfKill " ;
	BOOL sentInd = CProcessBase::GetProcess()->IsFaultsSentMcuMngr();
	if(sentInd && m_McmsDaemonInd )
	{
		TRACESTR(eLevelInfoNormal) << "call Teardown" ;
		 CProcessBase::GetProcess()->TearDown();
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "waiting for indications sentInd && m_McmsDaemonInd  to be true..." ;
		 StartTimer(MCMSNETWORK_SELF_KILL_TIMER, MCMSNETWORK_SELF_KILL_INTERVAL);
	}
}



void CMcmsNetworkManager::OnTimerSoftVMWaitingForIp(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "OnTimerSoftVMWaitingForIp  waited 2 seconds and try to configure again";
	m_wAutoNetworkSpend += MCMS_NETWORK_IPV6_AUTO_INTERVAL;
	if ( m_pMngnt_net->GetTimeoutofVM() <= m_wAutoNetworkSpend ) //time out and fall back
	{
		TRACEINTO << "CMcmsNetworkManager::OnTimerSoftVMWaitingForIp - Time out to retrieve System IP setting. Fall back to default mode";
		TryToRecover();
		return;
	}
	RunStartup();

}

STATUS CMcmsNetworkManager::TryToRecover()
{
	STATUS stat = STATUS_OK;

	if(!m_pMngnt_net)
		m_pMngnt_net = CNetworkFactory::GetInstance().CreateMngmntNetwork(m_pProcess->GetProductType());
    PASSERT_AND_RETURN_VALUE(NULL == m_pMngnt_net, STATUS_FAIL);
	if(!m_pSgnlMd_net)
		m_pSgnlMd_net = CNetworkFactory::GetInstance().CreateSignalMediaNetwork(m_pProcess->GetProductType());
    PASSERT_AND_RETURN_VALUE(NULL == m_pSgnlMd_net, STATUS_FAIL);

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (MANGMENT_STATUS_INTERFACE_READY != m_eLastMngmtStatus && curProductType != eProductTypeEdgeAxis)
	{
		std::string startup_alarm = m_pMngnt_net->GetAlarm();
		 TRACEINTO << "CMcmsNetworkManager::TryToRecover  management startup_alarm:" << startup_alarm;
		 if ("" != startup_alarm)
		 {
			 AddActiveAlarmSingleton(
				FAULT_GENERAL_SUBJECT,//FAULT_STARTUP_SUBJECT,
				AA_LAST_CONFIGURED_NETWORK_INTERFACE_IS_INVALID,
				STARTUP_ERROR_LEVEL,
				startup_alarm,
				true,
				true);
		 }

	}
	else if( STATUS_OK != m_eLastSgnlMdStatus && curProductType != eProductTypeEdgeAxis)
	{
		std::string startup_alarm = m_pSgnlMd_net->GetAlarm();
		 TRACEINTO << "CMcmsNetworkManager::TryToRecover signal/media startup_alarm:" << startup_alarm;
		 if ("" != startup_alarm)
		 {
			 AddActiveAlarmSingleton(
				FAULT_GENERAL_SUBJECT,//FAULT_STARTUP_SUBJECT,
				AA_LAST_CONFIGURED_NETWORK_INTERFACE_IS_INVALID,
				STARTUP_ERROR_LEVEL,
				startup_alarm,
				true,
				true);
		 }

	}
	else
	{
		TRACEINTO << "CMcmsNetworkManager::TryToRecover - last management status is "<< m_pProcess->GetStatusAsString(m_eLastMngmtStatus).c_str()
				<< " and signal/media status is "<< m_pProcess->GetStatusAsString(m_eLastSgnlMdStatus).c_str();
	}

	stat = m_pMngnt_net->TryToRecover(m_eLastMngmtStatus, m_eLastSgnlMdStatus, m_pSgnlMd_net);
	TRACEINTO << "CMcmsNetworkManager::TryToRecover - after recover management, status is "<< m_pProcess->GetStatusAsString(stat).c_str();
	stat = m_pSgnlMd_net->TryToRecover(m_eLastSgnlMdStatus, m_eLastMngmtStatus, m_pMngnt_net);
	TRACEINTO << "CMcmsNetworkManager::TryToRecover - after recover signal/media, status is "<< m_pProcess->GetStatusAsString(stat).c_str();

	SendFinishMsgToMcmsDaemon();
	StartTimer(MCMSNETWORK_SELF_KILL_TIMER, MCMSNETWORK_SELF_KILL_INTERVAL);
	return stat;
}
