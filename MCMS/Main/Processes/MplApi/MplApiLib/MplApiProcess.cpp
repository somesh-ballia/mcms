// MplApiProcess.cpp: implementation of the CMplApiProcess class.
//
//////////////////////////////////////////////////////////////////////

#include <iomanip>

#include "MplApiProcess.h"
#include "TraceStream.h"
#include "MplApiStatuses.h"
#include "TraceStream.h"
#include "MplMcmsProtocolTracer.h"
#include "ObjString.h"
#include "MplApiDefines.h"
#include "MplApiOpcodes.h"



extern void MplApiManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CMplApiProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CMplApiProcess::GetManagerEntryPoint()
{
	return MplApiManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMplApiProcess::CMplApiProcess()
{
	m_pSharedMemoryMap = NULL;
	m_isUpgradeStarted              = NO;
// 1080_60
	m_systemCardsBasedMode 			= eSystemCardsMode_breeze;
	m_masterSlaveReqHandler 		= new CMplApiMasterSlaveReqAsyncHandler;

	m_layoutSharedMemoryMap 		= NULL;
}

//////////////////////////////////////////////////////////////////////
int CMplApiProcess::SetUp()
{
	CProcessBase::SetUp();
	
	m_pSharedMemoryMap = new CSharedMemMap (CONN_TO_CARD_TABLE_NAME, 
		               						1,
		               						CONN_TO_CARD_TABLE_SIZE);
	InitializeLayoutSharedMemory();
	InitializeIndicationIconSharedMemory();

    return 0;
}

//////////////////////////////////////////////////////////////////////
int CMplApiProcess::TearDown()
{
	PDELETE(m_pSharedMemoryMap);
	FreeLayoutSharedMemory();
	FreeIndicationIconSharedMemory();
	CProcessBase::TearDown();
    return 0;
}

//////////////////////////////////////////////////////////////////////
CMplApiProcess::~CMplApiProcess()
{
// 1080_60
	POBJDELETE(m_masterSlaveReqHandler);
}

//////////////////////////////////////////////////////////////////////
void CMplApiProcess::AddExtraStatusesStrings()
{
	AddStatusString(STATUS_PROBLEMS_IN_SHARED_MEMORY, "Shared memory error");
}

//////////////////////////////////////////////////////////////////////
CSharedMemMap* CMplApiProcess::GetSharedMemoryMap()
{
	return m_pSharedMemoryMap;
}

//////////////////////////////////////////////////////////////////////
void CMplApiProcess::SetUpgradeStarted()
{
	m_isUpgradeStarted = YES;
}

void CMplApiProcess::OnSpecialCommanderFailure(CMplMcmsProtocol &mplPrtcl, OPCODE opcodeBefore, STATUS status)
{
/*	
	CMplMcmsProtocolTracer tracer(mplPrtcl);
	tracer.TraceMplMcmsProtocol("Bad Package, it's dropped");
*/
	const string &strOpcodeBefore 	= GetOpcodeAsString(opcodeBefore);
	const string &strOpcodeAfter 	= GetOpcodeAsString(mplPrtcl.getCommonHeaderOpcode());
	const string &strStatus			= GetStatusAsString(status);
	
	CLargeString buff;
	buff << "Special command handler FAILED. message canceled; " << '\n'
		 << "Opcode Before : " << strOpcodeBefore.c_str() 		 << '\n'
		 << "Opcode After : "  << strOpcodeAfter.c_str() 		 << '\n'
		 << "Status : "		   << strStatus.c_str()				 << '\n';
//	PASSERTMSG(1, buff.GetString());
//      sagi - this assert was disabled until BRIDGE-1242 is fixed
}

//////////////////////////////////////////////////////////////////////
void CMplApiProcess::CloseConnection(const WORD conId)
{
	TRACEINTO << "\nCMplApiProcess::CloseConnection Close connection in : conn id = " << conId;
	if (m_isUpgradeStarted == YES)
		m_CardOrCsConnIdTable.CloseConnectionNoFault(conId);
	else
		CApiProcess::CloseConnection(conId);


}
//////////////////////////////////////////////////////////////////////
void CMplApiProcess::GetDisconnectCounters(std::ostream& answer)
{

	/*prints the array - 5 is the maximum cards + switch*/
	for(int id = 0 ; id < MAX_NUM_OF_BOARDS ; id++)
	{
		answer<< "card #"<< id <<" resets " << m_CardOrCsConnIdTable.m_boardIdDisconnectionsCounter[id]<< endl;
	}


}
//////////////////////////////////////////////////////////////////////
std::string CMplApiProcess::GetIPAddressByBoardId(DWORD board_id)
{
	std::string ip = "";
	switch(board_id)
	{
		case(1):
		{
			ip = MEDIA_CARD_1_IP_ADDRESS;
			break;
		}
		case(2):
		{
			ip = MEDIA_CARD_2_IP_ADDRESS;
			break;
		}
		case(3):
		{
			ip = MEDIA_CARD_3_IP_ADDRESS;
			break;
		}
		case(4):
		{
			ip = MEDIA_CARD_4_IP_ADDRESS;
			break;
		}
		case(5):  // this is used only for Terminal command no matter the productType
		{
			ip = SWITCH_IP_ADDRESS;
			break;
		}
	}
	return ip;
}

BOOL CMplApiProcess::IsStartupFinished() const
{
    eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
    if( eMcuState_Invalid == systemState || eMcuState_Startup == systemState )
        return FALSE;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1080_60
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMplApiProcess::SetSystemCardsBasedMode(const eSystemCardsMode systemCardsBasedMode)
{
	if (m_systemCardsBasedMode != systemCardsBasedMode)
	{
		m_systemCardsBasedMode = systemCardsBasedMode;
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
eSystemCardsMode CMplApiProcess::GetSystemCardsBasedMode()
{
	return m_systemCardsBasedMode;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMplApiMasterSlaveReqAsyncHandler* CMplApiProcess::getMasterSlaveReqHandler()
{
	return m_masterSlaveReqHandler;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Change Layout Improvement - Layout Shared Memory (CL-SM)
//////////////////////////////////////////////////////////////////////
void CMplApiProcess::InitializeLayoutSharedMemory()
{
	m_layoutSharedMemoryMap = new CLayoutSharedMemoryMap();
	TRACESTR (eLevelInfoNormal) <<"Create Layout Shared Memory";
}

//////////////////////////////////////////////////////////////////////
CLayoutSharedMemoryMap* CMplApiProcess::GetLayoutSharedMemory()
{
	PASSERTMSG(!m_layoutSharedMemoryMap, "CConfPartyProcess::GetLayoutSharedMemory is NULL");

	return m_layoutSharedMemoryMap;
}

//////////////////////////////////////////////////////////////////////
void CMplApiProcess::FreeLayoutSharedMemory()
{
	POBJDELETE(m_layoutSharedMemoryMap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Indication Icon change Improvement - Indication Icon Shared Memory (CL-SM)
//////////////////////////////////////////////////////////////////////
void CMplApiProcess::InitializeIndicationIconSharedMemory()
{
	m_indicationIconSharedMemoryMap = new CIndicationIconSharedMemoryMap();
	TRACESTR (eLevelInfoNormal) <<"Create Indication Icon Shared Memory";
}

//////////////////////////////////////////////////////////////////////
CIndicationIconSharedMemoryMap* CMplApiProcess::GetIndicationIconSharedMemory()
{
	PASSERTMSG(!m_indicationIconSharedMemoryMap, "CConfPartyProcess::GetIndicationIconSharedMemory is NULL");

	return m_indicationIconSharedMemoryMap;
}

//////////////////////////////////////////////////////////////////////
void CMplApiProcess::FreeIndicationIconSharedMemory()
{
	POBJDELETE(m_indicationIconSharedMemoryMap);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
