// MplApiManager.cpp

#include "MplApiManager.h"

#include <stdlib.h>

#include "MplApiRxSocket.h"
#include "MplApiTxSocket.h"
#include "MplApiDispatcherTask.h"
#include "ListenSocketApi.h"
#include "ListenSocket.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsShelfMngr.h"
#include "Macros.h"
#include "MplMcmsStructs.h"
#include "IpService.h"
#include "TraceStream.h"
#include "FaultsDefines.h"
#include "SystemFunctions.h"
#include "ConfigManagerApi.h"
#include "MplMcmsProtocolTracer.h"
#include "StringsLen.h"
#include "MplApiDefines.h"
#include "SysConfig.h"
#include "MplApiProcess.h"
#include "TerminalCommand.h"
#include "DefinesGeneral.h"
#include "OpcodesMcmsInternal.h"


extern void MplApiMonitorEntryPoint(void* appParam);

std::vector< DWORD > IgnoredOpcodesList;

PBEGIN_MESSAGE_MAP(CMplApiManager)
    ONEVENT(MPLAPI_MSG               ,ANYCASE  , CMplApiManager::OnMplApiMsg )
    ONEVENT(OPEN_SOCKET_CONNECTION ,ANYCASE , CMplApiManager::OnMplApiOpenCardConnection)
    ONEVENT(CLOSE_SOCKET_CONNECTION  ,ANYCASE  , CMplApiManager::OnMplApiCloseCardConnection)
    ONEVENT(MPL_CTRL_IP_CONFIG_IND   ,ANYCASE  , CMplApiManager::OnCtrlIpConfigInd )
    ONEVENT(UPGRADE_STARTED_IND, ANYCASE, CMplApiManager::OnStartUpgradeInd )
    ONEVENT(SET_LOG_LEVEL_IND, ANYCASE, CMplApiManager::OnSetLogLevelInd)
    ONEVENT(NACK_LOG_LEVEL_OUT_OF_RANGE, ANYCASE, CMplApiManager::OnNackLogLevelOutOfRange)
    ONEVENT(MCMS_SYSTEM_CARDS_MODE_IND,  ANYCASE, CMplApiManager::OnSystemCardsModeInd)//Cards
PEND_MESSAGE_MAP(CMplApiManager,CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CMplApiManager)
// ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CMplApiManager::HandleOperLogin)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CMplApiManager)
	ONCOMMAND("conn",CMplApiManager::HandleTerminalConn,"displays [connection : board id] table")
    ONCOMMAND("lock_card",CMplApiManager::HandleTerminalLockCard,"simulate card got stuck")
    ONCOMMAND("set_signal",CMplApiManager::HandleTerminalSetSignal,"MPL-API set to use or unuse the signal of SetSelfKill")
	ONCOMMAND("opcode_on",CMplApiManager::HandleTerminalOpcodeOn,"MPL-API starts normal handling of the opcode")
	ONCOMMAND("opcode_off",CMplApiManager::HandleTerminalOpcodeOff,"MPL-API ignores sending the opcode")
	ONCOMMAND("disconn",CMplApiManager::HandleTerminalDisconn,"displays disconnection counters per card");
END_TERMINAL_COMMANDS

static CMplApiProcess *pMplApiProcess = NULL;

void MplApiManagerEntryPoint(void* appParam)
{
	CMplApiManager * pMplApiManager = new CMplApiManager;
	pMplApiManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CMplApiManager::GetMonitorEntryPoint()
{
	return MplApiMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
extern "C" void MplApiDispatcherEntryPoint(void* appParam);

CMplApiManager::CMplApiManager()
{
	pMplApiProcess 					= dynamic_cast<CMplApiProcess*>(CProcessBase::GetProcess());
	m_pListenSocketApi           	= NULL;
	m_pCntrlIpParams_asIpService 	= new CIPService;
	m_isSystemTarget				= YES;
}

/////////////////////////////////////////////////////////////////////////////
CMplApiManager::~CMplApiManager()
{
	POBJDELETE(m_pListenSocketApi);
	POBJDELETE(m_pCntrlIpParams_asIpService);
}

/////////////////////////////////////////////////////////////////////////////
void CMplApiManager::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(MPLAPI_MSG);
}

/////////////////////////////////////////////////////////////////////////////
void CMplApiManager::CreateDispatcher()
{
	m_pDispatcherApi = new CTaskApi();
	CreateTask(m_pDispatcherApi, MplApiDispatcherEntryPoint, m_pRcvMbx);
}

/////////////////////////////////////////////////////////////////////////////
void CMplApiManager::SelfKill()
{
	m_pListenSocketApi->Destroy();
	CManagerTask::SelfKill();
}

/////////////////////////////////////////////////////////////////////////////
void CMplApiManager::OnMplApiMsg(CSegment* pSeg)
{
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pSeg);
	CMplMcmsProtocolTracer(mplMcmsProtocol).TraceMplMcmsProtocol("MPLAPI_RECEIVED_FROM_MPL");

	OPCODE opcd = mplMcmsProtocol.getOpcode(); // extract the internal opcode...

	pSeg->ResetRead();
	DispatchEvent(opcd, pSeg);                //  ... and send it to the stateMachine
	PushMessageToQueue(opcd, pSeg->GetLen(), eProcessTypeInvalid);
}

///////////////////////////////////////////////////////////////////////////////
void  CMplApiManager::OnMplApiOpenCardConnection(CSegment* pMsg)
{
// 24.01.07: the fault is produced within <CCardConnIdTable::UpdateCard2ConnectionId>
//	if(m_DisconectReconectCnt > 0)
//	{
//		CHlogApi::SocketReconnect();
//		m_DisconectReconectCnt--;
//	}
}

///////////////////////////////////////////////////////////////////////////////
void  CMplApiManager::OnMplApiCloseCardConnection(CSegment* pMsg)
{
	WORD conId=0xFF;
	*pMsg >> conId;

	// ===== 1. logger
	TRACEINTO << ">>>><<<<CMplApiManager::OnMplApiCloseCardConnection -\n"
	          << "Card Disconnected. connection Id: " << conId ;

	// ===== 2. fault
// 24.01.07: the fault is produced within <CCardConnIdTable::CloseConnection>
//	CHlogApi::SocketDisconnect();
//  m_DisconectReconectCnt++;

	// ===== 3. update card2conn table
	pMplApiProcess->CloseConnection(conId);
}

/////////////////////////////////////////////////////////////////////////////
void CMplApiManager::OnCtrlIpConfigInd(CSegment* pSeg)
{
	// SAGI - in first version we will use hard coded address for the control network
//	PTRACE(eLevelInfoNormal, "CMplApiManager::OnCtrlIpConfigInd ");
//
//	// ===== 1. fill CMcuMngrManager's attribute with data from segment received
//	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
//	pMplMcmsProtocol->DeSerialize(*pSeg);
//	m_controlIpParams.SetData(pMplMcmsProtocol->GetData());
//	POBJDELETE(pMplMcmsProtocol);
//
//	// ===== 2. print the data to trace (using Dump function)
//    TRACEINTO << m_controlIpParams;
//
//	// ===== 3. convert to CIPService
//	m_controlIpParams.ConvertToIpService(*m_pCntrlIpParams_asIpService);
//	m_pCntrlIpParams_asIpService->SetName(CONTROL_NETWORK_NAME);
//
//	// ===== 4. update NetworkInterface (if needed)
//	//if ( ipAddress_received != ipAddress_in_CONTROL_NETWORK_CONFIG_PATH )
//		UpdateNetworkInterface();
}

///////////////////////////////////////////////////////////////////////////////
void*  CMplApiManager::GetMessageMap()
{
  return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMplApiManager::InitNetworkInterface()
{
	STATUS retStatus = STATUS_OK;

	// ===== 1. read from file to CntrlIpService
	retStatus = m_pCntrlIpParams_asIpService->ReadXmlFile(CONTROL_NETWORK_CONFIG_PATH);

	// on firat phase, no file should be exist anyway
	if ( STATUS_OK != retStatus ) // no file exists (yet) - create a default, hard-coded CntrlNetworkInterface
	{
		m_pCntrlIpParams_asIpService->SetName(CONTROL_NETWORK_NAME);
		m_pCntrlIpParams_asIpService->SetIpServiceType(eIpServiceType_Control);
		m_pCntrlIpParams_asIpService->SetNetIPaddress(0x0a000001);
		m_pCntrlIpParams_asIpService->SetNetMask(0xffffff00);
		m_pCntrlIpParams_asIpService->SetDefaultGatewayIPv4(0x0a000000);
		m_pCntrlIpParams_asIpService->SetVlanId(2093);

		CIPSpan* cntrlSpan = new CIPSpan;
		cntrlSpan->SetIPv4Address(0x0a000001);
		m_pCntrlIpParams_asIpService->ReplaceSpan_NoCheck(0, *cntrlSpan); // (there's 1st span only)
		POBJDELETE(cntrlSpan);

		retStatus = STATUS_OK;
	}

	// ===== 2. accordingly, init controlIpParams structure
	IP_PARAMS_S *pIpParamsStruct = m_controlIpParams.GetIpParamsStruct();
	memset(pIpParamsStruct, 0, sizeof(IP_PARAMS_S));
	m_pCntrlIpParams_asIpService->ConvertToIpParamsStruct( *pIpParamsStruct );

	// ===== 3. add NetworkInterface
	//retStatus = ConfigNetworkInterfaceInOS();
    // sagi - now done using Start.sh script.

	return retStatus;
}

////////////////////////////////////////////////////////////////////////////
STATUS CMplApiManager::UpdateNetworkInterface()
{
	STATUS retStatus = STATUS_OK;

	// ===== 1. write to file
	m_pCntrlIpParams_asIpService->WriteXmlFile(CONTROL_NETWORK_CONFIG_PATH, "CntrlNetwork");

	// ===== 2. replace NetworkInterface
	retStatus = ConfigNetworkInterfaceInOS();

	return retStatus;
}
////////////////////////////////////////////////////////////////////////////
STATUS CMplApiManager::ConfigNetworkInterfaceInOS()
{
	STATUS retStatus = STATUS_OK;

	CIPSpan* pTmpSpan = m_pCntrlIpParams_asIpService->GetSpanByIdx(0); // Mngmnt params are stored in the 1st span (idx==0)
	
	if(pTmpSpan == NULL)
	{
		FTRACEINTO << " CMplApiManager::ConfigNetworkInterfaceInOS pTmpSpan is NULL!!";
		return STATUS_FAIL;
	}
	DWORD ipAddress   = pTmpSpan->GetIPv4Address();

	DWORD netMask   = m_pCntrlIpParams_asIpService->GetNetMask();
	DWORD defGW     = m_pCntrlIpParams_asIpService->GetDefaultGatewayIPv4();
	DWORD vLanId    = m_pCntrlIpParams_asIpService->GetVlanId();

	char ipStr[IP_ADDRESS_LEN],
		 netMaskStr[IP_ADDRESS_LEN],
		 broadcastStr[IP_ADDRESS_LEN];

	SystemDWORDToIpString(ipAddress,ipStr);
	SystemDWORDToIpString(netMask,netMaskStr);
    //The OS receives only Broadcast address.
    //The terminology of defGW in the context of Vlan configuration is dead wrong
	SystemDWORDToIpString(defGW,broadcastStr);

	// ===== 1. call IpInterfaceConfig method
	if (YES == m_isSystemTarget) // otherwise no configuration should be done
	{
		CConfigManagerApi api;
		//retStatus = api.AddIpInterface(eInternalNetwork,ipStr,netMaskStr,defGwStr,vLanId);
	    FTRACEINTO << " Sending the Vlan Configuration from MplApi";
	    retStatus = api.AddVlan(eInternalNetwork,ipStr,netMaskStr,broadcastStr,vLanId);
	}

	// ===== 2. print to log
    FTRACEINTO << "\nCMplApiManager::ConfigNetworkInterfaceInOS\n"
               << " Add Interface to the system (sent from McuMngr)"
               << ": ip address - "          << ipStr
               << ", subnet mask - "         << netMaskStr
               << ", broadcast - "           << broadcastStr
               << ", vlan id - "             << vLanId
               << "\nConfiguration status: " << retStatus;

    return retStatus;
}

////////////////////////////////////////////////////////////////////////////
void CMplApiManager::ManagerPostInitActionsPoint()
{
	// 1,2. READ CNTRL NETWORK CONFIG PATH
	// 		CONFIG IP IN LINUX

	m_isSystemTarget = (TRUE == IsTarget() ? YES : NO);
	STATUS retStatus = InitNetworkInterface();

    const char *MplApiIp =  "0.0.0.0";
    if (IsTarget())
    {
        switch (CProcessBase::GetProcess()->GetProductFamily())
        {
            case eProductFamilyRMX:
            case eProductFamilySoftMcu:
                MplApiIp = "169.254.128.10";
                break;
            case eProductFamilyCallGenerator:
                MplApiIp = "127.0.0.1";
                break;
            default:
                PASSERT(1);
        }
    }
    else
    	  if(eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
    		  MplApiIp = "127.0.0.1";

	m_pListenSocketApi = new CListenSocketApi(MplApiSocketRxEntryPoint,
                                                  MplApiSocketTxEntryPoint,
                                                  MPL_API_LISTEN_SOCKET_PORT_NUM,
                                              	  MplApiIp);
	m_pListenSocketApi->SetMaxNumConnections(8);
	m_pListenSocketApi->Create(*m_pRcvMbx);


/*
	if (STATUS_OK != retStatus)
	{
		AddActiveAlarm(FAULT_GENERAL_SUBJECT,
					   NO_CONTROL_IP_INTERFACE,
					   MAJOR_ERROR_LEVEL,
					   "No Control ip interface",
					   true,
					   true);
	}
*/
}

/////////////////////////////////////////////////////////////////////
void CMplApiManager::DeclareStartupConditions()
{
/*
 // the action moved to IdleActionsPoint

	CActiveAlarm aa(FAULT_GENERAL_SUBJECT,
					NO_CONTROL_IP_INTERFACE,
					MAJOR_ERROR_LEVEL,
					"No Control ip interface",
					true,
					true);
 	AddStartupCondition(aa);
*/
}

////////////////////////////////////////////////////////////////////////////
void CMplApiManager::ManagerStartupActionsPoint()
{
/*
 // the action moved to IdleActionsPoint

	STATUS retStatus = STATUS_OK;

	CSysConfig* pSysConfig = pMplApiProcess->GetSysConfig();
	std::string data = " ";
	std::string key  = "SYSTEM_TYPE";
	pSysConfig->GetDataByKey(key, data);

	if ( !strcmp( data.c_str(), "TARGET") )
	{
		retStatus = InitNetworkInterface();
	}
	else
	{
		m_isSystemTarget = NO;
	}

	if (STATUS_OK == retStatus)
	{
		RemoveActiveAlarmByErrorCode(NO_CONTROL_IP_INTERFACE);
	}
	else
	{
		UpdateStartupConditionByErrorCode(NO_CONTROL_IP_INTERFACE, eStartupConditionFail);
	}
*/
}

/////////////////////////////////////////////////////////////////////
STATUS CMplApiManager::HandleTerminalConn(CTerminalCommand &command, std::ostream& answer)
{
	pMplApiProcess->DumpConnectionTable(answer);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CMplApiManager::HandleTerminalLockCard(CTerminalCommand &command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "board id should be 1 to MFA1 2 to MFA2 ...5 to switch  \n";
		answer << "usage: Bin/McuCmd lock_card MplApi [YES/NO]\n";
		return STATUS_FAIL;
	}

	const string &strIsCardLock = command.GetToken(eCmdParam1);
	const string &board_id      = command.GetToken(eCmdParam2);
	if ( ("YES" != strIsCardLock) && ("NO" != strIsCardLock) )
	{
		answer << "error: Illegal first parameters should be YES / NO \n";
		answer << "board id should be 1 to MFA1 2 to MFA2 ...5 to switch  \n";
		answer << "usage: Bin/McuCmd lock_card MplApi [YES/NO] [board_id]\n";
		return STATUS_FAIL;
	}

    bool isNumeric = CObjString::IsNumeric(board_id.c_str());
    if (false == isNumeric)
    {
      answer << "error: Parameter must be numeric, not " << board_id.c_str()
             << '\n'
             << "usage: Bin/McuCmd lock_card MplApi [YES/NO] [board_id]\n";
      return STATUS_FAIL;
    }

    int boardNnumber = atoi(board_id.c_str());

	if ( (1 > boardNnumber) || (5 < boardNnumber) )
	{
		answer << "error: Illegal second parameters  \n";
		answer << "board id should be 1 to MFA1 2 to MFA2 ...5 to switch  \n";
		answer << "usage: Bin/McuCmd lock_card MplApi [board_id]\n";
		return STATUS_FAIL;
	}

	if ("NO" == strIsCardLock)
	{

		    pMplApiProcess->SetTestFlag(FALSE,boardNnumber);
	}
	else
	{

		    pMplApiProcess->SetTestFlag(TRUE,boardNnumber);
	}
	return STATUS_OK;
}



/////////////////////////////////////////////////////////////////////
STATUS CMplApiManager::HandleTerminalSetSignal(CTerminalCommand &command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd set_signal MplApi [YES/NO]\n";
		return STATUS_FAIL;
	}

	const string &strIsCardLock = command.GetToken(eCmdParam1);
	const string &board_id      = command.GetToken(eCmdParam2);
	if ( ("YES" != strIsCardLock) && ("NO" != strIsCardLock) )
	{
		answer << "error: Illegal first parameters should be YES / NO \n";
		answer << "usage: Bin/McuCmd set_signal MplApi [YES/NO]\n";
		return STATUS_FAIL;
	}



	if ("NO" == strIsCardLock)
	{

		    pMplApiProcess->SetTestSignalFlag(FALSE);
	}
	else
	{

		    pMplApiProcess->SetTestSignalFlag(TRUE);
	}
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////
STATUS CMplApiManager::HandleTerminalOpcodeOn(CTerminalCommand &command, std::ostream& answer)
{
	CSysConfig* pSysConfig = pMplApiProcess->GetSysConfig();
	BOOL bDebugMode = FALSE;
	pSysConfig->GetBOOLDataByKey("DEBUG_MODE", bDebugMode);

	if( bDebugMode ) {
		STATUS returnStatus = STATUS_OK;
		DWORD numOfParams = command.GetNumOfParams();
		if(numOfParams > 1)
		{
			answer << "error: too much params: HandleTerminalOpcodeOff\n";
			answer << "usage: Bin/McuCmd opcode_on [opcode value]\n";
			return STATUS_FAIL;
		}

		if ( numOfParams != 0 ) {
			const string &strOpcode = command.GetToken(eCmdParam1);

			if ( ! strcmp (strOpcode.c_str(), "ALL") ) {
				std::vector< DWORD >::iterator itr =  IgnoredOpcodesList.begin();
				while ( itr != IgnoredOpcodesList.end() ) {
						IgnoredOpcodesList.erase(itr);
						itr = IgnoredOpcodesList.begin();
				}
			}
			else {
				bool isNumeric = CObjString::IsNumeric(strOpcode.c_str());
				if (false == isNumeric)
				{
				      answer << "error: Parameter must be numeric, not " << strOpcode.c_str()
				             << '\n'
				             << "usage: Bin/McuCmd opcode_on [opcode value]\n";
				      return STATUS_FAIL;
				}

				DWORD dwOpcode = atoi(strOpcode.c_str());

				bool bIsOpcodeAlreadyOn = true;
				if (dwOpcode > 0)
				{
					for (std::vector< DWORD >::iterator itr =  IgnoredOpcodesList.begin(); itr != IgnoredOpcodesList.end(); itr++) {
						if ( dwOpcode == (*itr) ) {
							bIsOpcodeAlreadyOn = false;
							IgnoredOpcodesList.erase(itr);
							break;
						}
					}
				}

				if (true == bIsOpcodeAlreadyOn ) {
					answer <<  "No action was done. Opcode is already ON!!!\n";
					returnStatus = STATUS_FAIL;
				}
			}
		}

		PrintIgnoredOpcodesList(answer);

		return returnStatus;
	}
	else
		answer << "No action was done: DEBUG_MODE is OFF!!!\n";

    return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////
STATUS CMplApiManager::HandleTerminalOpcodeOff(CTerminalCommand &command, std::ostream& answer)
{
	CSysConfig* pSysConfig = pMplApiProcess->GetSysConfig();
	BOOL bDebugMode = FALSE;
	pSysConfig->GetBOOLDataByKey("DEBUG_MODE", bDebugMode);

	if( bDebugMode ) {

		STATUS returnStatus = STATUS_OK;
		DWORD numOfParams = command.GetNumOfParams();
		if(numOfParams > 1)
		{
			answer << "error: too much params: HandleTerminalOpcodeOff\n";
			answer << "usage: Bin/McuCmd opcode_off [opcode value]\n";
			return STATUS_FAIL;
		}

		if ( numOfParams != 0 ) {
			const string &strOpcode = command.GetToken(eCmdParam1);

			bool isNumeric = CObjString::IsNumeric(strOpcode.c_str());
			if (false == isNumeric)
			{
			     answer << "error: Parameter must be numeric, not " << strOpcode.c_str()
			            << '\n'
			            << "usage: Bin/McuCmd opcode_off [opcode value]\n";
			     return STATUS_FAIL;
			}

			DWORD dwOpcode = atoi(strOpcode.c_str());

			bool bIsOpcodeAlreadyOff = false;
			if (dwOpcode>0)
			{
				for (std::vector< DWORD >::iterator itr =  IgnoredOpcodesList.begin(); itr != IgnoredOpcodesList.end(); itr++) {
					if ( dwOpcode == (*itr) ) {
						bIsOpcodeAlreadyOff = true;
						answer <<  "No action was done. Opcode is already OFF!!!\n";
						break;
					}
				}
			}
			if (false == bIsOpcodeAlreadyOff) {
				IgnoredOpcodesList.push_back(dwOpcode);
			}
		}

		PrintIgnoredOpcodesList(answer);

		return returnStatus;
	}
	else
		answer << "No action was done: DEBUG_MODE is OFF!!!\n";

    return STATUS_OK;
}

void CMplApiManager::PrintIgnoredOpcodesList(std::ostream& outputStream)
{
	outputStream <<  "\nIgnored Opcodes:\n";
	char pBuffer [20];

	std::vector< DWORD >::iterator itr =  IgnoredOpcodesList.begin();

	while (itr != IgnoredOpcodesList.end())
	{
		sprintf(pBuffer, "%d", (*itr));
		outputStream <<  pBuffer << "\n";
		itr++;
	}
}

void CMplApiManager::OnStartUpgradeInd(CSegment* pSeg)
{
	BOOL upgradeStarted = NO;
	*pSeg >> upgradeStarted;

	TRACEINTO << "CMplApiManager::OnStartUpgradeInd -\n"
	          << "upgradeStarted : " << upgradeStarted ;
	pMplApiProcess->SetUpgradeStarted();
}

void CMplApiManager::OnSetLogLevelInd(CSegment*)
{
    FTRACEINTOFUNC << "Pong";
}

void CMplApiManager::OnNackLogLevelOutOfRange(CSegment*)
{
    PASSERT(true);
}
/////////////////////////////////////////////////////////////////////
STATUS CMplApiManager::HandleTerminalDisconn(CTerminalCommand &command, std::ostream& answer)

{
	pMplApiProcess->GetDisconnectCounters(answer);
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////
// 1080_60
void CMplApiManager::OnSystemCardsModeInd(CSegment* pMsg)
{
    DWORD tempMode = (DWORD) eSystemCardsMode_illegal;
    *pMsg >> tempMode;

    pMplApiProcess->SetSystemCardsBasedMode((eSystemCardsMode)tempMode );
    TRACESTR(eLevelInfoNormal) << "\nCMplApiManager::OnSystemCardsModeInd"
                           << "\nMode received from Cards process: " << (eSystemCardsMode)tempMode;
            
}
/////////////////////////////////////////////////////////////////////
