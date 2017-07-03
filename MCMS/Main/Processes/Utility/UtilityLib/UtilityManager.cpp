#include "UtilityManager.h"

#include "UtilityProcess.h"

#include "Segment.h"
#include "ManagerApi.h"

#include "ProcessBase.h"
#include "ManagerTask.h"

#include "ProcessSettings.h"

#include "StatusesGeneral.h"
#include "ApiStatuses.h"

#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"

#include "FaultsDefines.h"

#include "AllocateStructs.h"
#include "Request.h"
#include "DummyEntry.h"
#include "UtilityMngrInternalStruct.h"


#include "SystemFunctions.h"
#include "IfConfig.h"

#include "FileDownloaderTask.h"
#include "IFileDownloader.h"

#include "SysConfigKeys.h"
#include "SysConfig.h"

#include "Trace.h"
#include "TraceStream.h"

#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////
#define UTILITY_NO_TCP_DUMP_IND_TIMEOUT 20

////////////////////////////////////////////////////////////////////////////
#define ERROR_TEMPLATE_FAILED_CAPTURE "%s failed to capture the TCP traffic file."
#define ERROR_TEMPLATE_FAILED_CAPTURE_FILTER_REASON "%s failed to capture the TCP traffic file.\nPlease check the syntax of the filter."

////////////////////////////////////////////////////////////////////////////
extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);

////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CUtilityManager)
	ONEVENT(CS_TCPDUMP_IP_SERVICE_PARAM_IND,        ANYCASE, CUtilityManager::OnCSTCPDumpIPServiceParamInd)
	ONEVENT(CS_UTILITY_MEDIA_IP_PARAMS_IND,         ANYCASE, CUtilityManager::OnMediaIpParamsFromCsInd)

	ONEVENT(TCPDUMP_REQUEST_IP_SERVICE_TIMER,       ANYCASE, CUtilityManager::HandleRequestIPServiceTimer)

	ONEVENT(UTILITY_START_TCP_DUMP,                 ANYCASE, CUtilityManager::OnStartTcpDump)
	ONEVENT(UTILITY_STOP_TCP_DUMP,                  ANYCASE, CUtilityManager::OnStopTcpDump)

	ONEVENT(UTILITY_CLEAR_TCP_DUMP_STORAGE,         ANYCASE, CUtilityManager::OnClearTcpDumpStorage)

	ONEVENT(UTILITY_START_TCP_DUMP_TIMER,           ANYCASE, CUtilityManager::OnStartTcpDumpTimer)

	ONEVENT(UTILITY_NO_START_TCP_DUMP_IND_TIMER,    ANYCASE, CUtilityManager::HandleNoStartTcpDumpIndicationReceivedTimer)
	ONEVENT(UTILITY_NO_STOP_TCP_DUMP_IND_TIMER,     ANYCASE, CUtilityManager::HandleNoStopTcpDumpIndicationReceivedTimer)

	ONEVENT(CARDS_TO_UTILITY_START_TCP_DUMP_STATUS, ANYCASE, CUtilityManager::OnStartTcpDumpStatusInd)
	ONEVENT(CARDS_TO_UTILITY_STOP_TCP_DUMP_STATUS,  ANYCASE, CUtilityManager::OnStopTcpDumpStatusInd)

	ONEVENT(UTILITY_DOWNLOAD_FILE,                  ANYCASE, CUtilityManager::OnDownloadFile)
	
	ONEVENT(UTILITY_FORWARD_TRACE_TO_LOGGER,        ANYCASE, CUtilityManager::ForwardTraceToLogger)
	ONEVENT(MCMS_SYSTEM_CARDS_MODE_IND,			    ANYCASE, CUtilityManager::OnSystemCardsModeInd )
    ONEVENT(RTM_LANS_AND_ISDN_SLOT_IND,			    ANYCASE, CUtilityManager::OnRtmLanAndIsdnSlotInd )

	ONEVENT(UTILITY_CONVERT_SLIDE,                  ANYCASE, CUtilityManager::OnConvertSlide)
PEND_MESSAGE_MAP(CUtilityManager, CManagerTask);

////////////////////////////////////////////////////////////////////////////
BEGIN_SET_TRANSACTION_FACTORY(CUtilityManager)
	ON_TRANS("TRANS_TCP_DUMP", "START_TCP_DUMP", CTcpDumpEntityList, CUtilityManager::HandleStartTcpDump)
	ON_TRANS("TRANS_TCP_DUMP", "STOP_TCP_DUMP",  CDummyEntry , CUtilityManager::HandleStopTcpDump)
	ON_TRANS("TRANS_TCP_DUMP", "CLEAR_STORAGE",  CDummyEntry , CUtilityManager::HandleTcpDumpClearStorage)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CUtilityManager)
//  ONCOMMAND("ping", CUtilityManager::HandleTerminalPing, "test terminal commands")
END_TERMINAL_COMMANDS

////////////////////////////////////////////////////////////////////////////
void UtilityManagerEntryPoint(void* appParam)
{
	CUtilityManager* pUtilityManager = new CUtilityManager;
	pUtilityManager->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////
CUtilityManager::CUtilityManager()
{
	m_proc = static_cast<CUtilityProcess*>(CProcessBase::GetProcess());
	PASSERTMSG(!m_proc, "Unable to continue");

	// VNGR-21782
	m_tcpdumpCalls = 0;

	memset(m_mfaBoardCnfArray,0,MAX_NUM_OF_SLOTS*sizeof(SLOTS_CONFIGURATION_S));


}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::ManagerPostInitActionsPoint()
{
	// Request for IP Services data in couple of seconds. It gives time to
	// CSMngr to startup and initialize its data.
	StartTimer(TCPDUMP_REQUEST_IP_SERVICE_TIMER, 30 * SECOND);

	//BRIDGE-8045
	SendSystemBasedModeReqToCardMngr();

}

////////////////////////////////////////////////////////////////////////////////////
void CUtilityManager::SendSystemBasedModeReqToCardMngr()
{
	PTRACE(eLevelInfoNormal, "CUtilityManager::SendSystemBasedModeReqToCardMngr");
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCardsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);

	STATUS res = pCardsMbx->Send(pRetParam, UTILITY_SYSTEM_CARDS_MODE_REQ);

}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::HandleRequestIPServiceTimer(CSegment*)
{
	const eProcessType dest = eProcessCSMngr;
	const OPCODE opcode = CS_TCPDUMP_IP_SERVICE_PARAM_REQ;

	STATUS stat = CManagerApi(dest).SendOpcodeMsg(opcode);
	if (STATUS_OK == stat)
	{
		TRACEINTOFUNC << "Request " << m_proc->GetOpcodeAsString(opcode) << " (" << opcode << ") to " << ProcessNames[dest] << " was sent successfully";
	}
	else
	{
		PASSERTSTREAM(true, "Unable to send " << m_proc->GetOpcodeAsString(opcode) << " (" << opcode << ") to " << ProcessNames[dest] << ": " << m_proc->GetStatusAsString(stat));
	}
}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnCSTCPDumpIPServiceParamInd(CSegment* seg)
{

	PASSERT_AND_RETURN(!seg);

	BYTE* raw = seg->GetPtr();
	PASSERT_AND_RETURN(!raw);

	// Interprets received raw data as known structure
	const IP_SERVICE_TCPDUMP_S& prm = *reinterpret_cast<IP_SERVICE_TCPDUMP_S*>(raw);

	// Get symbol name of the Service Type
	const char* type;
	BYTE stat = CStringsMaps::GetDescription(IP_SERVICE_TYPE_ENUM, prm.type, &type);
	PASSERT_AND_RETURN(!stat);

	// Prints received data to log
	std::ostringstream buf;
	buf << prm.id << " "
		<< prm.name << " "
		<< type << " (" << prm.type << "):";

	for (unsigned int i = 0; i < ARRAYSIZE(prm.span_ips); i++)
	{
		if (prm.span_ips[i] == 0)
			continue;

		char ip_str[IP_ADDRESS_LEN];
		SystemDWORDToIpString(prm.span_ips[i], ip_str);

		buf << "\n" << i << " " << ip_str;
	}

	TRACEINTOFUNC << buf.str();

	CTcpDumpEntityList* list = m_proc->GetTcpDumpEntityList();
	PASSERT_AND_RETURN(!list);

	// Service ID is an index in the IP Repository, the value should match
	PASSERTSTREAM_AND_RETURN(prm.id >= ARRAYSIZE(list->m_pTcpDumpEntities), "Service ID " << prm.id << " >= " << ARRAYSIZE(list->m_pTcpDumpEntities));

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	CTcpDumpEntity* ent;
	size_t i; // span_ips's index

	switch (prm.type)
	{
		case eIpServiceType_Management:
			ent = list->m_pTcpDumpEntities[e_EntityType_Management];
			ent->SetEntityType(e_EntityType_Management);

			// Sets IP addresses of the Service
			for (i = 0; i < ARRAYSIZE(prm.span_ips); ++i)
			{
				if (prm.span_ips[i] == 0)
					continue;

				ent->AddIpEntity(prm.span_ips[i]);
			}
			// SoftMcu, mfw, axis, Add "lo"
			break;

		case eIpServiceType_Signaling:
            /*SoftMcu, mfw, axis, only one external interface, don't need this*/
            if (eProductTypeSoftMCU == curProductType || eProductTypeSoftMCUMfw == curProductType || eProductTypeEdgeAxis == curProductType)
            {
                TRACEINTOFUNC << "CUtilityManager::OnCSTCPDumpIPServiceParamInd: SoftMcu,MFW,Axis only have one interface.";
                break;
            }

			ent = list->m_pTcpDumpEntities[e_EntityType_Central_Signaling];
			ent->SetEntityType(e_EntityType_Central_Signaling);
			ent->AddIpEntity(prm.span_ips[0]); // that should be the cs ip

			//the next are the media
			int board_id;
			if (prm.ipType != eIpType_IpV6)
			{
				for (i = 0; i < ARRAYSIZE(prm.span_ips); ++i)
				{
					if (prm.span_ips[i] == 0)
						continue;

					char ip_str[IP_ADDRESS_LEN];
					SystemDWORDToIpString(prm.span_ips[i], ip_str);

					buf <<  "\n" << i << " " << ip_str;
				}
			}
			else
			{
				if (*prm.span_ipv6s)
					buf <<  "\n" << '1' << " " << prm.span_ipv6s;
			}

			if (prm.ipType != eIpType_IpV6)
			{
				// Sets IP addresses of the Service
				for (i = 0; i < ARRAYSIZE(prm.span_ips); ++i)
				{
					if (prm.span_ips[i] == 0)
						continue;

					//ent->AddIpEntity(prm.span_ips[i]);
					TRACEINTOFUNC << "CUtilityManager::OnCSTCPDumpIPServiceParamInd: Add Management IPV4 Entity: " << prm.span_ips[i];

				}
			}
			else if (prm.ipType == eIpType_IpV6)
			{
				ent->AddIpV6Entity((char*)prm.span_ipv6s);
				TRACEINTOFUNC << "CUtilityManager::OnCSTCPDumpIPServiceParamInd: Add Management IPV6 Entity: " << prm.span_ipv6s;
			}

			ent->SetIpType(prm.ipType);

			if ((prm.ipType == eIpType_IpV4) || (prm.ipType == eIpType_Both))
			{
				//ent->AddIpEntity(prm.span_ips[0]); // that should be the cs ip
				TRACEINTOFUNC << "CUtilityManager::OnCSTCPDumpIPServiceParamInd: Add Central_Signaling IPV4 Entity: " << prm.span_ips[0];
			}
			else if (prm.ipType == eIpType_IpV6)
			{
				ent->AddIpV6Entity((char*)prm.span_ipv6s);
				TRACEINTOFUNC << "CUtilityManager::OnCSTCPDumpIPServiceParamInd: Add Central_Signaling IPV6 Entity: " << prm.span_ipv6s;
			}
			else
			{
				PASSERTSTREAM_AND_RETURN(true, "Invalid IP type: " << prm.ipType);
				break;
			}

			// there is no need to capture media alone in Gesher/Ninja. (Captured signaling and media at the same interface)
			//ent->AddIpEntity(prm.span_ips[i]);

			if ((prm.ipType == eIpType_IpV4) || (prm.ipType == eIpType_Both))
				ent->AddIpEntity(prm.span_ips[i]); // TODO: check, what i's value to use?? ***

			else if (prm.ipType == eIpType_IpV6)
				ent->AddIpV6Entity((char*)prm.span_ipv6s);

			else
			{
				PASSERTSTREAM_AND_RETURN(true, "Invalid IP type: " << prm.ipType);
				break;
			}

			if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
			{
			// there is no need to capture media alone in Gesher/Ninja. (Captured signaling and media at the same interface)
#			if 0
				for (i = 1; i < 10; ++i)
				{
					board_id = 1; //(i=1 --> boardid 1) (i=2 --> boardid 1) (i=3 --> boardid 1)...
					ent = list->m_pTcpDumpEntities[board_id+1];
					ent->SetEntityType(e_EntityType_Media_Card);

					if (prm.span_ips[i] == 0)
						continue;

					ent->SetBoardId(board_id);
					ent->AddIpEntity(prm.span_ips[i]);
				}
#			endif
			}
			else
			{
				for (i = 1; i < 10/*ARRAYSIZE(prm.span_ips)*/; ++i)
				{
					board_id = i/2 +i%2; //(i=1,i=2 --> boardid 1) (i=3,i=4 --> boardid 2)...
					ent = list->m_pTcpDumpEntities[board_id+1];
					ent->SetEntityType(e_EntityType_Media_Card);

					if (prm.span_ips[i] == 0)
						continue;

					ent->SetBoardId(board_id);
					ent->AddIpEntity(prm.span_ips[i]);
				}
			}
			break;

		default:
			PASSERTSTREAM_AND_RETURN(true, "Invalid Service type " << prm.type);
	}

	// Sizes of the Repository and message structure should match
	PASSERTSTREAM_AND_RETURN(
		ARRAYSIZE(prm.span_ips) != ARRAYSIZE(ent->m_pIPList),
		"Snap IPs size " << ARRAYSIZE(prm.span_ips) << " != " << ARRAYSIZE(ent->m_pIPList));

	char ip_str[IP_ADDRESS_LEN];

	for (size_t i = 0; i < 10; ++i)
	{
		ent = list->m_pTcpDumpEntities[i];

		for (size_t j = 0; j < 30; ++j)
		{
			if (ent->m_pIPList[j]->GetIpAddress() == 0xFFFFFFFF)
				continue;

			SystemDWORDToIpString(ent->m_pIPList[j]->GetIpAddress(), ip_str);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
STATUS CUtilityManager::HandleStartTcpDump(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() != SUPER)
	{
		TRACEINTO << "No permission";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		pRequest->SetConfirmObject(new CDummyEntry);
		return STATUS_OK;
	}

	if (m_tcpdumpCalls)
	{
		TRACESTRFUNC(eLevelWarn) << "m_tcpdumpCalls:" << m_tcpdumpCalls;
		pRequest->SetStatus(STATUS_FAIL);
		pRequest->SetExDescription("Already ongoing or stop not finished.");
		pRequest->SetConfirmObject(new CDummyEntry);
		return STATUS_OK;
	}

	TRACEINTO;

	CTcpDumpEntityList* pTcpDumpStartList = (CTcpDumpEntityList*) pRequest->GetRequestObject();

	char ip_str[IP_ADDRESS_LEN];
	CTcpDumpEntity* ent;
	BOOL bFilterIllegal = FALSE;
	BOOL bFilterTokenLegal = TRUE;
	std::string errorDescription;
	std::string answer;
	std::string filter;
	std::string commandLine;
	size_t found;
	char tmp_num_str[32];

	switch (pTcpDumpStartList->GetMaxCaptureSize())
	{
		case e_MaxCaptureSize_0_5_gb:
			m_maxCaptureSize = 5;
			break;

		case e_MaxCaptureSize_1_gb:
			m_maxCaptureSize = 10;
			break;

		case e_MaxCaptureSize_1_5_gb:
			m_maxCaptureSize = 15;
			break;

		case e_MaxCaptureSize_2_5_gb:
			m_maxCaptureSize = 25;
			break;

		default:
		{
			TRACESTRFUNC(eLevelWarn) << "pTcpDumpStartList->GetMaxCaptureSize() " <<
				pTcpDumpStartList->GetMaxCaptureSize();

			pRequest->SetStatus(STATUS_FAIL);
			pRequest->SetExDescription("Unsupported max capture size.");
			pRequest->SetConfirmObject(new CDummyEntry);

			return STATUS_OK;
		}
	}

	m_totalTcpdumpUnits = 0;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	for (int i = 0; i < MAX_NUM_OF_ENTITIES; ++i)
	{
		ent = pTcpDumpStartList->m_pTcpDumpEntities[i];

		TRACEINTO << "\nENTITY i HandleStartTcpDump " << i << " m_entityType " << ent->GetEntityType();

		for (int j = 0; j < MAX_SPAN_NUMBER_IN_SERVICE; ++j)
		{
			if (ent->m_pIPList[j]->GetIpAddress() == 0xFFFFFFFF) continue;

			SystemDWORDToIpString(ent->m_pIPList[j]->GetIpAddress(), ip_str);
			TRACEINTO << "\n j " << j << " Ipaddress " << ip_str << " m_selected " << (DWORD)ent->m_pIPList[j]->GetIsSelected() ;

			if (!ent->m_pIPList[j]->GetIsSelected())
				continue;

			switch (ent->GetEntityType())
			{
				case e_EntityType_Management:
					m_totalTcpdumpUnits += 2;
					break;

				case e_EntityType_Central_Signaling:
					m_totalTcpdumpUnits += 2;
					break;

				case e_EntityType_Media_Card:
					m_totalTcpdumpUnits += 6;
					break;
			}
		}
	}


	*m_proc->GetTcpDumpStartList() = *((CTcpDumpEntityList*)(pRequest->GetRequestObject()));

	pRequest->SetStatus(STATUS_OK);
	pRequest->SetConfirmObject(new CDummyEntry);

	CManagerApi api(eProcessUtility);
	api.SendOpcodeMsg(UTILITY_START_TCP_DUMP);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnStartTcpDump(CSegment* seg)
{
	DWORD tcpdumpEmaCall = 0;

	TRACEINTO << "m_tcpdumpCalls:" << m_tcpdumpCalls;

	if (m_tcpdumpCalls)
		return; // already started

	STATUS test_filters = TestFilters();

	if (test_filters != STATUS_OK)
	{
		// after this timer expires - current tcp dum will be cleaned. timer is for EMA get get the status.
		TRACESTRFUNC(eLevelWarn) << "One of the filters is wrong - starting timer";
		StartTimer(UTILITY_NO_STOP_TCP_DUMP_IND_TIMER, 10*SECOND);
		m_tcpdumpCalls = 1; // avoid starting new tcp dump
		return;
	}

	TRACESTRFUNC(eLevelWarn) << "Test filters passed\n";

	ClearTcpDumpStorage();

	CStructTm startTime;
	SystemGetTime(startTime);
	char startTimeStr[32];

	// VNGR-21782: construct the start time str in format: yymmdd_hh:mm:ss
	startTimeStr[0] = ((startTime.m_year / 10) % 10) + '0';
	startTimeStr[1] = (startTime.m_year % 10) + '0';
	startTimeStr[2] = ((startTime.m_mon / 10) % 10) + '0';
	startTimeStr[3] = (startTime.m_mon % 10) + '0';
	startTimeStr[4] = ((startTime.m_day / 10) % 10) + '0';
	startTimeStr[5] = (startTime.m_day % 10) + '0';
	startTimeStr[6] = '_';
	startTimeStr[7] = ((startTime.m_hour / 10) % 10) + '0';
	startTimeStr[8] = (startTime.m_hour % 10) + '0';
	startTimeStr[9] = ':';
	startTimeStr[10] = ((startTime.m_min / 10) % 10) + '0';
	startTimeStr[11] = (startTime.m_min % 10) + '0';
	startTimeStr[12] = ':';
	startTimeStr[13] = ((startTime.m_sec / 10) % 10) + '0';
	startTimeStr[14] = (startTime.m_sec % 10) + '0';
	startTimeStr[15] = '\0';

	m_startTimeStr = startTimeStr;

	m_bShouldNoStartTimerStart = true;
	m_bNoStartTimerStarted = false;
	CTcpDumpEntityList*  pTcpDumpEntityList=m_proc->GetTcpDumpEntityList();
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	CTcpDumpEntityList*  pTcpDumpStartList= m_proc->GetTcpDumpStartList();
	CTcpDumpStatus * pCTcpDumpStatus = m_proc->GetTcpDumpStatus() ;
	for (int i = 0; i < MAX_NUM_OF_ENTITIES; i++)
	{
		CTcpDumpEntity *ent = pTcpDumpStartList->m_pTcpDumpEntities[i];

		for (int j = 0; j < MAX_SPAN_NUMBER_IN_SERVICE; j++)
		{
			if (ent->m_pIPList[j]->GetIpAddress() != 0xFFFFFFFF
				&& ent->m_pIPList[j]->GetIsSelected())
			{
				TRACEINTO << (int)ent->m_pIPList[j]->GetIsSelected()<< " " << (int)ent->GetEntityType();

				if ((ent->GetEntityType() == e_EntityType_Management) || (ent->GetEntityType() == e_EntityType_Central_Signaling))
				{
					char ipStr[IP_ADDRESS_LEN];

					SystemDWORDToIpString(ent->m_pIPList[j]->GetIpAddress(), ipStr);

					DWORD allocatedUnits = 2 * m_maxCaptureSize / m_totalTcpdumpUnits;

					if (allocatedUnits < 2)
						allocatedUnits = 2;

					STATUS stat = SendStartTcpDumpToConfigurator(ent, startTimeStr, ipStr, allocatedUnits);

					if (stat == STATUS_OK)
					{
						// VNGR-21782
						++m_tcpdumpCalls;
						TRACEINTO << "VNGR-21782: m_tcpdumpCalls:" << m_tcpdumpCalls << " status:" << stat;

						pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Running);
						pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Running);

						StartTcpDumpDurationTimer(pTcpDumpStartList->GetMaxCaptureDuration());

						if (m_proc->GetIsTcpDumpRunning() == false)
						{
							// VNGR-21815
							AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
								AA_UTILITY_NETWORK_TRAFFIC_CAPTURE_ON,
								MAJOR_ERROR_LEVEL,
								"Network traffic capture is on.",
								true,
								true);
							m_proc->SetIsTcpDumpRunning(true);

							CStructTm curTime ;

							SystemGetTime(curTime);
							m_proc->SetTimeElapsed(curTime);

							TRACESTR(eLevelInfoNormal) << "CUtilityManager::OnStartTcpDumpStatus sec = "
								<< curTime.m_sec<<" minutes " << curTime.m_min;
						}
					}
					else
					{
						pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Failed);
						pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Failed, ERROR_TEMPLATE_FAILED_CAPTURE, ent->GetTcpDumpEntityName().c_str());
					}

					pTcpDumpEntityList->SetMaxCaptureDuration(pTcpDumpStartList->GetMaxCaptureDuration());
					pTcpDumpEntityList->SetMaxCaptureSize(pTcpDumpStartList->GetMaxCaptureSize());
				}

				if (ent->GetEntityType() == e_EntityType_Media_Card)
				{
					std::string commandLine;
					BuildCommandLine(commandLine,ent->m_pIPList[j]->GetIpAddress(),ent, startTimeStr);
					ent->SetTcpDumpState(e_TcpDumpState_Idle);
					if (SendStartTcpDumpToCardsMngr(ent->GetBoardId(),commandLine.c_str()) == STATUS_OK)
					{
						++m_tcpdumpCalls;
						++tcpdumpEmaCall;
						TRACEINTO << "VNGR-21782: m_tcpdumpCalls " << m_tcpdumpCalls;
						pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Success);
						pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Success);
						ent->SetTcpDumpState(e_TcpDumpState_Success);
					}
					else
					{
						++m_tcpdumpCalls;
						TRACEINTO << "Failed SendStartTcpDumpToCardsMngr for board " << ent->GetBoardId();
						pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Failed);
						pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Failed, ERROR_TEMPLATE_FAILED_CAPTURE, ent->GetTcpDumpEntityName().c_str());
					}
				}
			}
		}
	}

	if (m_tcpdumpCalls > 0)
	{
		TRACEINTO << "After starting tcpdumps  m_tcpdumpCalls:" << m_tcpdumpCalls << " UTILITY_NO_START_TCP_DUMP_IND_TIMER should start " << (DWORD) m_bShouldNoStartTimerStart;
		if (m_bShouldNoStartTimerStart)
		{
			if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
			{
				TRACEINTO << "Product type:" << curProductType;

				if (tcpdumpEmaCall > 0)
				{
					StartTimer(UTILITY_NO_START_TCP_DUMP_IND_TIMER, UTILITY_NO_TCP_DUMP_IND_TIMEOUT*SECOND);
					m_bNoStartTimerStarted = true;
				}
			}
			else
			{
				StartTimer(UTILITY_NO_START_TCP_DUMP_IND_TIMER, UTILITY_NO_TCP_DUMP_IND_TIMEOUT*SECOND);
				m_bNoStartTimerStarted = true;
			}
		}
	}
	else
	{
		TRACEINTO << "After starting tcpdumps, m_tcpdumpCalls is 0";
		m_bShouldNoStartTimerStart = false;
	}
}

STATUS CUtilityManager::TestFilters()
{
	BOOL bFilterIllegal = FALSE;
	BOOL bFilterTokenLegal = TRUE;
	std::string commandLine;
	std::string answer;
	std::string filter;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	CTcpDumpEntityList*  pTcpDumpStartList= m_proc->GetTcpDumpStartList();
	CTcpDumpEntity* ent;
	size_t found;

	CTcpDumpStatus * pCTcpDumpStatus = m_proc->GetTcpDumpStatus() ;

	for (int i = 0; i < MAX_NUM_OF_ENTITIES ; i++)
	{
		ent = pTcpDumpStartList->m_pTcpDumpEntities[i];

		for (int j = 0; j < MAX_SPAN_NUMBER_IN_SERVICE; j++)
		{
			if (ent->m_pIPList[j]->GetIpAddress() != 0xFFFFFFFF &&
				ent->m_pIPList[j]->GetIsSelected())
			{
				filter = ent->GetFilter();
				if (filter != "")
				{
					found = filter.find("'");
					TRACEINTO << "\nENTITY i j " << i << " " <<  j  << " filter is "<< filter;

					if (found != string::npos)
					{
						TRACESTRFUNC(eLevelWarn) << "Filter string shouldn't include token '. " << filter;

						bFilterTokenLegal = FALSE;
					}

					if (bFilterTokenLegal)
					{
						found = filter.find("\"");

						if (found != string::npos)
						{
							TRACESTRFUNC(eLevelWarn) << "Filter string shouldn't include token \"" << filter;

							bFilterTokenLegal = FALSE;

						}
					}

					if (bFilterTokenLegal)
					{
						if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
						{
							commandLine = "sudo /usr/sbin/tcpdump -c 1 -r "+MCU_MCMS_DIR+"/Scripts/VNGR_20148_Sample_Tcpdump_Output.sh '" + filter + "' 2>&1";
						}
						else
						{
							commandLine = "tcpdump -c 1 -r "+MCU_MCMS_DIR+"/Scripts/VNGR_20148_Sample_Tcpdump_Output.sh '" + filter + "' 2>&1";
						}

						SystemPipedCommand(commandLine.c_str(), answer);

						TRACESTRFUNC(eLevelInfoNormal) << "answer: \n" << answer;

						found = answer.find("syntax error");
						if(found == string::npos) found = answer.find("tcpdump:");   //fix BRIDGE-2224
					}

					if (!bFilterTokenLegal || found != string::npos)
					{
						if (!bFilterIllegal)
						{
							bFilterIllegal = TRUE;

							//errorDescription = "Filter syntax or token error, the following errors detected while \n"
							//	"starting the capture.\n";
						}

						TRACESTRFUNC(eLevelWarn) << "Filter token  is not legal bFilterIllegal " << (int)bFilterIllegal << "entity "
							<< ent->GetTcpDumpEntityName() << " type " << (int)ent->GetEntityType();

						pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Failed, ERROR_TEMPLATE_FAILED_CAPTURE_FILTER_REASON, ent->GetTcpDumpEntityName().c_str());
						return STATUS_FAIL;
					}
				}
			}
		}
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CUtilityManager::StartTcpDumpDurationTimer(eMaxCaptureDurationType maxCaptureDutration)
{
	//start duration timer
	TRACEINTO << "m_tcpdumpCalls:" << m_tcpdumpCalls ;

	if (m_bNoStartTimerStarted)
	{
		DeleteTimer(UTILITY_NO_START_TCP_DUMP_IND_TIMER);
		m_bNoStartTimerStarted = false;
	}

	m_bShouldNoStartTimerStart = false;

	switch (maxCaptureDutration)
	{
		case e_MaxCaptureDuration_15_sec:

			StartTimer(UTILITY_START_TCP_DUMP_TIMER, 15*SECOND);
			break;
		case e_MaxCaptureDuration_30_sec:
			StartTimer(UTILITY_START_TCP_DUMP_TIMER, 30*SECOND);
			break;

		case e_MaxCaptureDuration_1_min:
			StartTimer(UTILITY_START_TCP_DUMP_TIMER, 60*SECOND);
			break;

		case e_MaxCaptureDuration_2_min:
			StartTimer(UTILITY_START_TCP_DUMP_TIMER, 120*SECOND);
			break;

		case e_MaxCaptureDuration_3_min:
			StartTimer(UTILITY_START_TCP_DUMP_TIMER, 180*SECOND);
			break;

		case e_MaxCaptureDuration_4_min:
			StartTimer(UTILITY_START_TCP_DUMP_TIMER, 240*SECOND);
			break;

		case e_MaxCaptureDuration_5_min:
			StartTimer(UTILITY_START_TCP_DUMP_TIMER,300*SECOND);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CUtilityManager::BuildCommandLine(std::string& commandLine, DWORD ipaddress, CTcpDumpEntity* ent, const char* startTimeStr)
{
	char ip_str[IP_ADDRESS_LEN];

	// tcp dump host ip_str + filter;
	SystemDWORDToIpString(ipaddress, ip_str);
	std::string ip_str_temp(ip_str);

	char cardNumber[8]={0,'_'};
	snprintf(cardNumber, sizeof(cardNumber), "%02d", ent->GetBoardId());

	string eth= ConvertIpHostToInterface(ent->GetBoardId(), ipaddress);

	DWORD allocatedUnits = 6 * m_maxCaptureSize / m_totalTcpdumpUnits;

	if (allocatedUnits < 2)
		allocatedUnits = 2;

	char allocatedUnitsStr[32];

	sprintf(allocatedUnitsStr, "%d", allocatedUnits);

	const eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
	if (eProductFamilySoftMcu == curProductFamily /* all SoftMcu products */)
	{
		commandLine = "sudo /usr/sbin/tcpdump -s 0 -i " + eth + " " + ent->GetFilter() +" -v -w "+MCU_OUTPUT_DIR+"/tcp_dump/emb/"
			+ startTimeStr + "_mc_" + cardNumber + "_" + ip_str + ".cap." + " -Z root -C 100 -W "
			+ allocatedUnitsStr;
	}
	else
	{
		commandLine = "tcpdump -s 0 -i " + eth + " " + ent->GetFilter() +" -v -w /mnt/mfa_cm_fs/tcpdump/"
			+ startTimeStr + "_mc_" + cardNumber + "_" + ip_str + ".cap." + " -C 100 -W "
			+ allocatedUnitsStr;
	}

	TRACEINTOFUNC << commandLine;
}

/////////////////////////////////////////////////////////////////////////////
string CUtilityManager::ConvertIpHostToInterface(APIU32 boardId, DWORD ipaddress)
{
	eSystemCardsMode   systemCardsBasedMode = m_proc->GetSystemCardsMode();

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bLAN_REDUNDANCY = YES;
	BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_LAN_REDUNDANCY, bLAN_REDUNDANCY);
	BOOL isV35JitcSupport = NO;
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);
	BOOL isMultipleServicesSupport = NO;
	sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServicesSupport);
	BOOL bRTM_LAN = NO;
        
        if (systemCardsBasedMode == eSystemCardsMode_mpmrx) bRTM_LAN=YES;
        else
	   res = sysConfig->GetBOOLDataByKey(CFG_KEY_RMX2000_RTM_LAN, bRTM_LAN);



	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	string eth;

	//it is the internal network interface.
	string def = "eth0";

	if (systemCardsBasedMode == eSystemCardsMode_mpmrx)
		def = "eth4";

	APIU32   pqId = 0xFFFFFFFF;
	//find the pqId according to the board id
	for (int i = 0; i < MAX_NUM_OF_INTERFACES; ++i)
	{

		/*TRACESTR(eLevelInfoNormal) << "board_id= "<<m_interfacesList[i].boardId <<"pqId= "<< m_interfacesList[i].pqId;*/
		if(m_interfacesList[i].boardId == boardId && m_interfacesList[i].iPv4Address == ipaddress)
		{
			pqId = m_interfacesList[i].pqId;

			// VNGR-25369: Lan redundancy is not supported in IPv6
			if (m_interfacesList[i].ipType == eIpType_IpV6 ||
				m_interfacesList[i].ipType == eIpType_Both)
			{
				bLAN_REDUNDANCY = NO;
			}

			break;
		}

	}
	TRACESTR(eLevelInfoNormal) << "pid= "<< m_interfacesList[0].pqId<< "eth= "<<eth;
	/*product type*/
	if(curProductType == eProductTypeRMX1500 || curProductType == eProductTypeRMX4000)
	{
		if(bLAN_REDUNDANCY == YES && isMultipleServicesSupport == NO && isV35JitcSupport == NO)
		{
			return "bond0";
		}



		if(isMultipleServicesSupport == YES || isV35JitcSupport == YES)
		{
			if (pqId == 1)
			{
				if (systemCardsBasedMode == eSystemCardsMode_mpmrx && (m_mfaBoardCnfArray[boardId].rtmCompType == eShmComp_RtmLan))
					eth = "eth0";
				else
					eth = "eth2";
			}
			else if (pqId == 2)
			{
				if (systemCardsBasedMode == eSystemCardsMode_mpmrx && (m_mfaBoardCnfArray[boardId].rtmCompType == eShmComp_RtmLan))
					eth = "eth1";
				else
					eth = "eth3";
			}
			else
			{
				eth = def;
			}
		}

		else
		{

			if ((systemCardsBasedMode == eSystemCardsMode_mpmrx) && (m_mfaBoardCnfArray[boardId].rtmCompType == eShmComp_RtmLan))
				eth = "eth0";
			else
				eth = "eth2";
		}


	}
	/*curProductType == eProductTypeRMX2000*/
	else if(curProductType == eProductTypeRMX2000 || eProductTypeSoftMCU == curProductType ||
			eProductTypeSoftMCUMfw == curProductType || eProductTypeEdgeAxis == curProductType ||
			eProductTypeCallGeneratorSoftMCU == curProductType)
	{
		if(bRTM_LAN == YES)
		{
			if(bLAN_REDUNDANCY == YES && isMultipleServicesSupport == NO && isV35JitcSupport == NO)
			{
				return "bond0";
			}


			if(isMultipleServicesSupport == YES || isV35JitcSupport == YES)
			{
				if (pqId == 1)
				{
					if (systemCardsBasedMode == eSystemCardsMode_mpmrx && (m_mfaBoardCnfArray[boardId].rtmCompType == eShmComp_RtmLan))

						eth = "eth0";
					else
						eth = "eth2";
				}
				else if (pqId == 2)
				{
					if (systemCardsBasedMode == eSystemCardsMode_mpmrx && (m_mfaBoardCnfArray[boardId].rtmCompType == eShmComp_RtmLan))
						eth = "eth1";
					else
						eth = "eth3";
				}
				else
				{
					eth = def;
				}
			}
			else
			{
				if ((systemCardsBasedMode == eSystemCardsMode_mpmrx) && (m_mfaBoardCnfArray[boardId].rtmCompType == eShmComp_RtmLan))
					eth = "eth0";
				else
					eth = "eth2";
			}

		}
		else
		{
			eth = def;
		}
	}
	else if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
	{
		char ip_str[IP_ADDRESS_LEN];
		SystemDWORDToIpString(ipaddress, ip_str);
		GetNICFromIpAddress(ip_str,  eth);
		TRACESTR(eLevelInfoNormal) << "CUtilityManager::ConvertIpHostToInterface GetNICFromIpAddress IpAddress: " << ip_str <<" NIC: " << eth;
	}

	return eth;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CUtilityManager::SendStartTcpDumpToCardsMngr(DWORD boardId, const char* param)
{
	// ===== 1. prepare data to send
	UTILITY_START_TCP_DUMP_S TcpDumpParams;
	memset(&TcpDumpParams, 0, sizeof(TcpDumpParams));
	memcpy(&TcpDumpParams.Params.sTcpDumpString, param, TCP_DUMP_PARAM_LEN);
	TcpDumpParams.Params.sTcpDumpString[TCP_DUMP_PARAM_LEN - 1] = '\0';

	TcpDumpParams.boardId = boardId;


	TRACEINTOFUNC
		<< "boardId:" << boardId
		<<"\nParams:" << param
		<<"\nTcpDumpParams.Params.sTcpDumpString:" << TcpDumpParams.Params.sTcpDumpString;

	CSegment* pRetParam = new CSegment;
	pRetParam->Put((BYTE*)&TcpDumpParams, sizeof(TcpDumpParams));

	const COsQueue* pCardsMngrMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);
	STATUS res = pCardsMngrMbx->Send(pRetParam, UTILITY_TO_CARDS_START_TCP_DUMP_REQ);

	return res;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CUtilityManager::SendStopTcpDumpToCardsMngr(DWORD boardId)
{
	UTILITY_START_TCP_DUMP_S TcpDumpParams;
	memset(&TcpDumpParams, 0, sizeof(TcpDumpParams));

	TcpDumpParams.boardId = boardId;
	TRACEINTOFUNC << "boardId:" << boardId;

	CSegment* pRetParam = new CSegment;
	pRetParam->Put((BYTE*)&TcpDumpParams, sizeof(TcpDumpParams));

	const COsQueue* pCardsMngrMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);
	STATUS res = pCardsMngrMbx->Send(pRetParam, UTILITY_TO_CARDS_STOP_TCP_DUMP_REQ);

	return res;
}

////////////////////////////////////////////////////////////////////////////
STATUS CUtilityManager::HandleStopTcpDump(CRequest *pRequest)
{
	if (pRequest->GetAuthorization() != SUPER)
	{
		TRACEINTO << "No permission";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		pRequest->SetConfirmObject(new CDummyEntry);
		return STATUS_OK;
	}

	TRACEINTO;

	// ===== reply to EMA
	pRequest->SetStatus(STATUS_OK);
	pRequest->SetConfirmObject(new CDummyEntry);

	CManagerApi api(eProcessUtility);
	api.SendOpcodeMsg(UTILITY_STOP_TCP_DUMP);

	TRACEINTO << "END";
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CUtilityManager::HandleTcpDumpClearStorage(CRequest *pRequest)
{
	if (pRequest->GetAuthorization() != SUPER)
	{
		TRACEINTO << "No permission";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		pRequest->SetConfirmObject(new CDummyEntry);
		return STATUS_OK;
	}

	TRACEINTO;

	pRequest->SetStatus(STATUS_OK);
	pRequest->SetConfirmObject(new CDummyEntry);

	CManagerApi api(eProcessUtility);
	api.SendOpcodeMsg(UTILITY_CLEAR_TCP_DUMP_STORAGE);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
// VNGR-26526. Stop tcp dump when no start indication revieced.
void CUtilityManager::HandleNoStartTcpDumpIndicationReceivedTimer(CSegment*)
{
	TRACEINTO;
	// we didnt receive start tcp dump indication. Can happen when sending tcp dump to missing card
	MoveToRunningState();
}

////////////////////////////////////////////////////////////////////////////
// VNGR-26526. Stop tcp dump when no stop indication revieced.
void CUtilityManager::HandleNoStopTcpDumpIndicationReceivedTimer(CSegment*)
{
	TRACEINTO;
	// we didnt reveive stop tcp dump indication. (usually will not happen)
	MoveToRunningState();
}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnStopTcpDump(CSegment*)
{
	StopTcpDump(e_TcpDumpState_Idle);
}

////////////////////////////////////////////////////////////////////////////
STATUS CUtilityManager::StopTcpDump(eTcpDumpState tcpDumpState)
{
	TRACEINTO << "tcpDumpState:" << (DWORD)tcpDumpState;

	CTcpDumpEntityList* pTcpDumpEntityList = m_proc->GetTcpDumpEntityList();
	CTcpDumpEntityList* pTcpDumpStartList = m_proc->GetTcpDumpStartList();
	CTcpDumpStatus* pCTcpDumpStatus = m_proc->GetTcpDumpStatus() ;

	pCTcpDumpStatus->SetDescription("");

	for (size_t i = 0; i < MAX_NUM_OF_ENTITIES; ++i)
	{
		CTcpDumpEntity* ent = pTcpDumpStartList->m_pTcpDumpEntities[i];

		for (size_t j = 0; j < MAX_SPAN_NUMBER_IN_SERVICE; ++j)
		{
			if (ent->m_pIPList[j]->GetIpAddress() != 0xFFFFFFFF)
			{
				TRACEINTO << ent->m_pIPList[j]->GetIsSelected() << " " << ent->GetEntityType();

				if ((ent->GetEntityType() == e_EntityType_Management) || (ent->GetEntityType() == e_EntityType_Central_Signaling))
				{
					STATUS stat = SendStopTcpDumpToConfigurator();

					if (m_tcpdumpCalls > 0)
						--m_tcpdumpCalls;

					TRACEINTO << "OnStopTcpDump SendStopTcpDumpToConfigurator m_tcpdumpCalls:" << m_tcpdumpCalls << ", status:" << stat;

					pTcpDumpEntityList->SetTcpDumpState(tcpDumpState);
					pCTcpDumpStatus->SetTcpDumpState(tcpDumpState);

					DeleteTimer(UTILITY_START_TCP_DUMP_TIMER);
				}

				if (ent->GetEntityType() == e_EntityType_Media_Card)
				{

					if (ent->GetTcpDumpState() == e_TcpDumpState_Success )
					{
					SendStopTcpDumpToCardsMngr(ent->GetBoardId()) ;
					    ent->SetTcpDumpState(e_TcpDumpState_Idle);
					}
					TRACEINTO << "VNGR-21782: e_EntityType_Media_Card m_tcpdumpCalls " << m_tcpdumpCalls ;
					pTcpDumpEntityList->SetTcpDumpState(tcpDumpState);
					pCTcpDumpStatus->SetTcpDumpState(tcpDumpState);
				}
			}
		}
	}


	// VNGR-26528. In case we came from HandleNoTcpDumpIndicationReceivedTimer (tcpDumpState: e_TcpDumpState_Failed) - dont hope for stop indication from other entities (embedded).
	if (m_tcpdumpCalls == 0 || tcpDumpState == e_TcpDumpState_Failed)
	{
		TRACEINTO << "Clear curernt tcp dump";

		RemoveActiveAlarmByErrorCode(AA_UTILITY_NETWORK_TRAFFIC_CAPTURE_ON);

		pCTcpDumpStatus->ClearMembers();
		pTcpDumpStartList->ClearMembers();
		m_tcpdumpCalls = 0;
		m_proc->SetIsTcpDumpRunning(false);
	}
	else
	{
		TRACEINTO << "Start timer UTILITY_NO_STOP_TCP_DUMP_IND_TIMER m_tcpdumpCalls:" << m_tcpdumpCalls;
		StartTimer(UTILITY_NO_STOP_TCP_DUMP_IND_TIMER, 15*SECOND);
	}

#if 0 // According to Rachel, rename is unneccessary
	// VNGR-21782: trigger rename if all stopped
	if (m_tcpdumpCalls == 0)
	{
		SendRenameTcpDumpOutputToConfigurator(m_startTimeStr);
	}
#endif

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CUtilityManager::MoveToRunningState()
{
	TRACEINTO;

	DeleteTimer(UTILITY_NO_STOP_TCP_DUMP_IND_TIMER);
	RemoveActiveAlarmByErrorCode(AA_UTILITY_NETWORK_TRAFFIC_CAPTURE_ON);

	m_proc->GetTcpDumpStatus()->ClearMembers();
	m_proc->GetTcpDumpStartList()->ClearMembers();
	m_tcpdumpCalls = 0;
	m_proc->SetIsTcpDumpRunning(false);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CUtilityManager::ClearTcpDumpStorage()
{
	CConfigManagerApi configuratorApi;
	return configuratorApi.ClearTcpDumpStorage();
}

/////////////////////////////////////////////////////////////////////////////
STATUS CUtilityManager::SendStartTcpDumpToConfigurator(CTcpDumpEntity* ent, char* startTimeStr, char* ipStr, DWORD allocatedUnits)
{
	CConfigManagerApi configuratorApi;
	std::string result;

	CTcpDumpEntityList* list = m_proc->GetTcpDumpEntityList();
	PASSERT_AND_RETURN_VALUE(!list, STATUS_FAIL);

	STATUS status = configuratorApi.StartTcpDump(
		(ent->GetEntityType()),
		ent->GetFilter(),
		startTimeStr,
		ipStr,
		allocatedUnits,
		list->m_pTcpDumpEntities[e_EntityType_Management]->GetIpType(),
		result);

	TRACEINTO << "filterStr:" << ent->GetFilter() << ", result:" << result << ", status:" << status;

	status = result.empty() ? STATUS_FAIL : STATUS_OK;
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CUtilityManager::SendStopTcpDumpToConfigurator()
{
	TRACESTR(eLevelInfoNormal) << "CUtilityManager::SendStopTcpDumpToConfigurator";

	CConfigManagerApi configuratorApi;
	STATUS respStatus = configuratorApi.StopTcpDump();

	return respStatus;
}

#if 0 // According to Rachel, rename is unneccessary
STATUS CUtilityManager::SendRenameTcpDumpOutputToConfigurator(const string &startTimeStr)
{
	TRACESTR(eLevelInfoNormal) << "CUtilityManager::SendRenameTcpDumpOutputToConfigurator: startTimeStr "
		<< startTimeStr;

	CConfigManagerApi configuratorApi;
	STATUS respStatus = configuratorApi.RenameTcpDumpOutput(startTimeStr);

	return respStatus;
}
#endif

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnStartTcpDumpStatusInd(CSegment* pSeg)
{
	TCP_DUMP_CONFIG_IND_S* theMsg = new TCP_DUMP_CONFIG_IND_S; AUTO_DELETE(theMsg);
	pSeg->Get((BYTE*)theMsg, sizeof(TCP_DUMP_CONFIG_IND_S));

	TRACEINTO << "status:" << theMsg->eDumpStatus << " reason " << theMsg->uErrorReason;

	CTcpDumpEntityList* pTcpDumpEntityList = m_proc->GetTcpDumpEntityList();
	CTcpDumpStatus* pCTcpDumpStatus = m_proc->GetTcpDumpStatus();
	CTcpDumpEntityList* pTcpDumpStartList = m_proc->GetTcpDumpStartList();

	switch (theMsg->eDumpStatus)
	{
		case eTcpDumpOk:
		{
			pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Running);
			pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Running);

			StartTcpDumpDurationTimer(pTcpDumpStartList->GetMaxCaptureDuration());

			if (!m_proc->GetIsTcpDumpRunning())
			{
				// VNGR-21815
				AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
					AA_UTILITY_NETWORK_TRAFFIC_CAPTURE_ON,
					MAJOR_ERROR_LEVEL,
					"Network traffic capture is on.",
					true,
					true);

				m_proc->SetIsTcpDumpRunning(true);
				CStructTm curTime ;
				SystemGetTime(curTime);
				m_proc->SetTimeElapsed(curTime);
				TRACEINTO << "sec:" << curTime.m_sec << ", minutes:" << curTime.m_min;
			}

			break;
		}

		case eTcpDumpInternalError:
			if (theMsg->uErrorReason != eTcpDumpAlreadyRunning)
			{
				pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Failed);
				pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Failed, ERROR_TEMPLATE_FAILED_CAPTURE, "Media Card");
			}
			break;

		case eTcpDumpSystemError:
			pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Failed);
			pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Failed, ERROR_TEMPLATE_FAILED_CAPTURE, "Media Card");
			break;

		default:
			pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Failed);
			pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Failed, ERROR_TEMPLATE_FAILED_CAPTURE, "Media Card");
			break;
	}

	pTcpDumpEntityList->SetMaxCaptureDuration(pTcpDumpStartList->GetMaxCaptureDuration());
	pTcpDumpEntityList->SetMaxCaptureSize(pTcpDumpStartList->GetMaxCaptureSize());

	if (pTcpDumpEntityList->GetTcpDumpState() == e_TcpDumpState_Failed)
		OnStopTcpDump();
}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnStopTcpDumpStatusInd(CSegment* pSeg)
{
	TCP_DUMP_CONFIG_IND_S* theMsg = new TCP_DUMP_CONFIG_IND_S; AUTO_DELETE(theMsg);
	pSeg->Get( (BYTE*)theMsg, sizeof(TCP_DUMP_CONFIG_IND_S) );

	TRACEINTO << theMsg->eDumpStatus << " m_tcpdumpCalls " << m_tcpdumpCalls;

	// ===== 1. extract keycode from segment received
	CTcpDumpEntityList * pTcpDumpStartList = m_proc->GetTcpDumpStartList() ;


	CTcpDumpEntityList * pTcpDumpEntityList = m_proc->GetTcpDumpEntityList() ;
	CTcpDumpStatus * pCTcpDumpStatus = m_proc->GetTcpDumpStatus() ;

	switch (theMsg->eDumpStatus)
	{
		case eTcpDumpOk:
			pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Idle);
			pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Idle);

			DeleteTimer(UTILITY_START_TCP_DUMP_TIMER);
			TRACEINTO << "delete timer";

			pTcpDumpEntityList->SetMaxCaptureDuration(e_MaxCaptureDuration_none);
			pTcpDumpEntityList->SetMaxCaptureSize(e_MaxCaptureSize_none);
			break;

		case eTcpDumpInternalError:
			TRACEINTO << "eTcpDumpInternalError" ;
			pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Failed);
			pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Failed, ERROR_TEMPLATE_FAILED_CAPTURE, "Media Card");
			break;

		case eTcpDumpSystemError:
			TRACEINTO << "eTcpDumpSystemError" ;
			pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Failed);
			pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Failed, ERROR_TEMPLATE_FAILED_CAPTURE, "Media Card");
			break;

		default:
			TRACEINTO << "Unknown error";
			pTcpDumpEntityList->SetTcpDumpState(e_TcpDumpState_Failed);
			pCTcpDumpStatus->SetTcpDumpState(e_TcpDumpState_Failed, ERROR_TEMPLATE_FAILED_CAPTURE, "Media Card");
			break;
	}

	if (m_tcpdumpCalls > 0)
	{
		--m_tcpdumpCalls;

		if (!m_tcpdumpCalls)
			MoveToRunningState();
	}

#if 0 // According to Rachel, rename is unneccessary
	// VNGR-21782: trigger rename if all stopped
	if (m_tcpdumpCalls == 0)
		SendRenameTcpDumpOutputToConfigurator(m_startTimeStr);
#endif
}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnClearTcpDumpStorage(CSegment*)
{
	TRACEINTO;
	ClearTcpDumpStorage();
	m_proc->SetIsUiUpdateNeeded(true);
}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnStartTcpDumpTimer(CSegment*)
{
	DeleteTimer(UTILITY_START_TCP_DUMP_TIMER);
	TRACEINTO << "delete timer";

	CTcpDumpStatus* pCTcpDumpStatus = m_proc->GetTcpDumpStatus();
	OnStopTcpDump();
}

/////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnMediaIpParamsFromCsInd(CSegment* pSeg)
{
	DWORD numOfElem =0;
	*pSeg >> numOfElem;
	IP_INTERFACE_SHORT_S *pNewMediaIpParamsStruct = (IP_INTERFACE_SHORT_S*)pSeg->GetPtr(1);


	memcpy((BYTE*)&m_interfacesList, (BYTE*)pNewMediaIpParamsStruct, sizeof(m_interfacesList));

	TRACEINTO << "\nCCommCardService::OnMediaIpParamsFromCsInd numOfElem "<< numOfElem;

	for (int i =0;i<MAX_NUM_OF_BOARDS * MAX_NUM_OF_PQS;i++)
		{
			if (m_interfacesList[i].boardId !=0 && m_interfacesList[i].iPv4Address != 0)
			{
				char tmpIP[IP_ADDRESS_LEN];
				::SystemDWORDToIpString(m_interfacesList[i].iPv4Address , tmpIP);

				TRACEINTO << "\nCCommCardService::OnMediaIpParamsFromCsInd"
						      << "\nIndx: " << i
					          << "\nBoardId: " << m_interfacesList[i].boardId
					          << "\niPv4Address: " << tmpIP
					          << "\nipType: " << m_interfacesList[i].ipType
					          << "\npqId: " << m_interfacesList[i].pqId
					          <<"\n";


			}



		}
}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnDownloadFile(CSegment* seg)
{
	eProcessType target = eProcessTypeInvalid;
	OPCODE opcode = 0;

	std::string url, baseFolder, filePath;
	*seg >> target >> opcode >> url >> baseFolder >> filePath;

	TRACEINTO << "baseFolder:" << baseFolder << ", filePath:" << filePath;

	m_downloaderTask.enqueue(FileDownloadRequest(url, baseFolder, filePath, target, opcode));
}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnConvertSlide(CSegment* seg)
{
	eProcessType target = eProcessTypeInvalid;
	OPCODE opcode = 0;

	std::string url, outputPath,inputFile;
	
	bool isOnlyForH264 = false;
	
	if(eProductTypeSoftMCUMfw == CProcessBase::GetProcess()->GetProductType())
		isOnlyForH264 = true;
	
    int nConversionMethod = eIvrSlideLowHighRes;
    int nImageType = eIvrSlideImageJpg;
	*seg >> target >> opcode >> url >> outputPath >> inputFile  >> nConversionMethod >> nImageType;

    TRACEINTO << "url: " << url << "outputPath:" << outputPath << ", inputFile:" << inputFile 
        << ", isOnlyForH264:" << isOnlyForH264 << ", nConversionMethod:" << nConversionMethod 
        << ", nImageType:" << nImageType;

    m_slideConvertTask.enqueue(SlideConvertRequest(url, outputPath, inputFile, isOnlyForH264, nConversionMethod, nImageType, target, opcode));
}


////////////////////////////////////////////////////////////////////////////
void CUtilityManager::ForwardTraceToLogger(CSegment* pMsgSeg)
{
	WORD lineNum = 0;
	WORD level = 0;

	std::string file_name;
	std::string message;

	*pMsgSeg >> file_name >> lineNum >> level >> message;

	OutTraceMessage(file_name.c_str(), lineNum, level, NULL, message.c_str(), NULL);
}

////////////////////////////////////////////////////////////////////////////
void CUtilityManager::OnSystemCardsModeInd(CSegment* pMsg)
{
	DWORD tmp;
	*pMsg >> tmp;

	eSystemCardsMode mode = static_cast<eSystemCardsMode>(tmp);

	m_proc->SetSystemCardsMode(mode);
	TRACEINTO << "Mode received from Cards process: " << ::GetSystemCardsModeStr(mode);
}

/////////////////////////////////////////////////////////////////////////////////////

void  CUtilityManager::OnRtmLanAndIsdnSlotInd(CSegment* pMsg)
{

	memset(m_mfaBoardCnfArray,0,MAX_NUM_OF_SLOTS*sizeof(SLOTS_CONFIGURATION_S));

	pMsg->Get( (BYTE*)m_mfaBoardCnfArray, MAX_NUM_OF_SLOTS*sizeof(SLOTS_CONFIGURATION_S) );

	for (int i=0;i<5;i++)
	    	{

	    		TRACEINTO <<   "\nCUtilityManager::OnRtmLanAndIsdnSlotInd\nRTM LAN/ISDN  Params:\nMediaCompType" << m_mfaBoardCnfArray[i].mediaCompType
	    				<< "\nRtmCompType "  << m_mfaBoardCnfArray[i].rtmCompType
	    				<< "\nSubBoardId  "  << m_mfaBoardCnfArray[i].unSubBoardId
	    				<< "\nBoardId      "  << i;
	    	}
}

////////////////////////////////////////////////////////////////////////////

