// CSApiManager.cpp: implementation of the CCSApiManager class.
//
//////////////////////////////////////////////////////////////////////

#include "CSApiManager.h"
#include "CSApiRxSocket.h"
#include "CSApiTxSocket.h"
#include "CSApiDispatcherTask.h"
#include "ListenSocketApi.h"
#include "ListenSocket.h"
#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "Macros.h"
#include "MplMcmsStructs.h"
#include "XmlEngineApi.h"
#include "XmlFormat.h"
#include "XmlBuilder.h"
#include "SysConfig.h"
#include "CSApiProcess.h"
#include "HlogApi.h"
#include "TraceStream.h"
#include "FaultsDefines.h"

extern void CSApiMonitorEntryPoint(void* appParam);
extern "C" void CSApiDispatcherEntryPoint(void* appParam);

//const WORD  IDLE			= 0;        // default state
const WORD	CONNECTED					= 1;
const WORD	DISCONNECTED 				= 2;





////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CCSApiManager)
  ONEVENT(XML_REQUEST    ,IDLE    ,  CCSApiManager::HandlePostRequest )
  ONEVENT(CLOSE_SOCKET_CONNECTION ,ANYCASE , CCSApiManager::OnCSApiCloseCardConnection)
  ONEVENT(OPEN_SOCKET_CONNECTION ,ANYCASE , CCSApiManager::OnCSApiOpenCardConnection)
PEND_MESSAGE_MAP(CCSApiManager,CManagerTask);



////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CCSApiManager)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CCSApiManager::HandleOperLogin)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CCSApiManager)
	ONCOMMAND("conn",CCSApiManager::HandleTerminalConn,"displays [connection : board id] table")
END_TERMINAL_COMMANDS



////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void CSApiManagerEntryPoint(void* appParam)
{
	CCSApiManager * pCSApiManager = new CCSApiManager;
	pCSApiManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CCSApiManager::GetMonitorEntryPoint()
{
	return CSApiMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCSApiManager::CCSApiManager()
{
	m_pListenSocketApi = NULL;
	m_pCsApiProcess = (CCSApiProcess*)CCSApiProcess::GetProcess();
	m_state = IDLE;
}

//////////////////////////////////////////////////////////////////////
CCSApiManager::~CCSApiManager()
{
    POBJDELETE(m_pListenSocketApi);
}

//////////////////////////////////////////////////////////////////////
void CCSApiManager::ManagerPostInitActionsPoint()
{
	int status = InitXmlEngine();

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
    	if(eProductFamilySoftMcu ==CProcessBase::GetProcess()->GetProductFamily())
    		MplApiIp = "127.0.0.1";

    m_pListenSocketApi = new CListenSocketApi(CSApiSocketRxEntryPoint,
											  CSApiSocketTxEntryPoint,
											  CS_API_LISTEN_SOCKET_PORT_NUM,
											  MplApiIp);
    m_pListenSocketApi->SetMaxNumConnections(9);
	m_pListenSocketApi->Create(*m_pRcvMbx);
}

//////////////////////////////////////////////////////////////////////
void CCSApiManager::CreateDispatcher()
{
	m_pDispatcherApi = new CTaskApi;
	CreateTask(m_pDispatcherApi, CSApiDispatcherEntryPoint, m_pRcvMbx);
}

/////////////////////////////////////////////////////////////////////////////
void CCSApiManager::SelfKill()
{
	m_pListenSocketApi->Destroy();
	CManagerTask::SelfKill();
}

///////////////////////////////////////////////////////////////////////////////
void  CCSApiManager::OnCSApiCloseCardConnection(CSegment* pMsg)
{
	WORD conId=0xFF;
	*pMsg >> conId;

	// ===== 1. logger
	TRACEINTO << ">>>><<<<CCSApiManager::OnCSApiCloseCardConnection -\n"
	          << "CS Disconnected. connection Id: " << conId ;

	// ===== 3. update card2conn table, there is fault inside
	m_pCsApiProcess->CloseConnection(conId);

	m_state = DISCONNECTED;
}

///////////////////////////////////////////////////////////////////////////////
void  CCSApiManager::OnCSApiOpenCardConnection(CSegment* pMsg)
{
// 24.01.07: the fault is produced within <CCardConnIdTable::UpdateCard2ConnectionId>
//	if(DISCONNECTED == m_state)
//	{
//		CHlogApi::SocketReconnect();
//	}

	m_state = CONNECTED;
}


///////////////////////////////////////////////////////////////////////////////
void*  CCSApiManager::GetMessageMap()
{
  return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////
STATUS CCSApiManager::HandleTerminalConn(CTerminalCommand &command, std::ostream& answer)
{
	m_pCsApiProcess->DumpConnectionTable(answer);
	return STATUS_OK;
}


