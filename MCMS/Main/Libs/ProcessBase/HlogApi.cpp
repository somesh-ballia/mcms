// HlogApi.cpp

#include "HlogApi.h"
#include <stdio.h>

#include "DefinesGeneral.h"
#include "FaultsDefines.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "HlogElement.h"
#include "FaultDesc.h"
#include "OpcodesMcmsCommon.h"
#include "ManagerApi.h"
#include "Versions.h"
#include "FaultBlockQueue.h"
#include "TraceStream.h"
#include "StringsMaps.h"
#include "ObjString.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "AuditorApi.h"
#include "AlarmStrTable.h"

bool TestFaultValidity(CLogFltElement* pHlogElement)
{
    CXMLDOMElement *pFatherNode = new CXMLDOMElement;
    pHlogElement->SerializeXml(pFatherNode);
    PDELETE(pFatherNode);

    return true;
}

CHlogApi::CHlogApi()
{}

CHlogApi::~CHlogApi()
{}

void CHlogApi::TreatBadFaultType( BYTE subject,
                                  DWORD errorCode,
                                  BYTE errorLevel,
                                  const char *description,
                                  BOOL isFullOnly,
                                  DWORD boardId,
                                  DWORD unitId,
                                  WORD theType)
{
    CLargeString message;
    message << "Bad Fault Type Found\n"
            << "Subject :" << subject << '\n'
            << "Type : " << theType;
    FPASSERTMSG(TRUE, message.GetString());
}

bool CHlogApi::AddToDB_IsFlooding(CLogFltElement *pHlogElement)
{
    CTaskApp * pTask = CProcessBase::GetProcess()->GetCurrentTask();
    if (pTask == NULL)
    {
    	POBJDELETE(pHlogElement);	//judith - to prevent leak
        return false;
    }

    CFaultBlockQueue *faultBlockQueue = pTask->GetFaultBlockQueue();
    TICKS timeNow = SystemGetTickCount();
    DWORD tickNow = timeNow.GetIntegerPartForTrace();

    FaultBlockNode *pFBQNode = new FaultBlockNode(tickNow, pHlogElement);	//This struct delete pHlogElement
    FaultBlockNode *pFBQSimilar = faultBlockQueue->FindSimilar(pFBQNode);

    //The first time we have this error
    if(NULL == pFBQSimilar)
    {
        faultBlockQueue->Push(pFBQNode);
    }
    else
    {
        DWORD difference = tickNow - pFBQSimilar->GetTime();

        faultBlockQueue->Replace(pFBQSimilar, pFBQNode);

        const DWORD maxInterval = SECOND * 60 * 10;
        if(difference < maxInterval)
        {
            return false;
        }
    }
    return true;
}

eAuditable CHlogApi::IsAuditableFault(DWORD errorCode)
{
    switch(errorCode)
    {
        case AA_SIP_SECURED_COMMUNICATION_FAILED:
        case AA_PRODUCT_ACTIVATION_FAILURE:
        case AA_ILLEGAL_TIME:
        case AA_RUNNING_VERSION_MISMATCHES_CURRENT_VERSION:
        case AA_ILLEGAL_MCU_VERSION:
        case AA_NO_MUSIC_SOURCE:
        case BAD_FILE_SYSTEM:
        case RESET_MCU:
        case RESET_MCU_INTERNAL:
        case FILE_SYSTEM_FAILED_TO_SCAN:
        case FILE_SYSTEM_OVERFLOW:
        case AA_HIGH_SCHEDULER_LATENCY:
            return eAuditableFailure;

        case CFG_CHANGED:
        case RESTORING_DEFAULT_FACTORY:
        case IP_SERVICE_DELETED:
        case IP_SERVICE_CHANGED:
        case IP_SERVICE_ADDED:
        case AA_SSH_ENABLED:
        case RESET_MCU_BY_TERMINAL:
        case RESET_MCU_BY_OPERATOR:
        case RESET_MCU_DIAGNOSTICS:
        case NEW_VERSION_INSTALLED:
        case UPGRADE_RECEIVE_VERSION :
        case SHA1_VERSION:
        case NEW_ACTIVATION_KEY_LOADED:
        case ACTIVATION_KEY_IS_NEEDED:
        case AA_DEFAULT_USER_EXISTS:
        case MCU_RESTART:
        case AA_PATCHED_VERSION:
        case AA_PRIVATE_VERSION:
        case STARTUP_CONDITION_CONFIGURATION:
        case TEST_ERROR:
        case AA_HTTPS_DISABLED_IN_JITC:
        	return eAuditableNotification;

        default:
            return eAuditableNothing;
    }

    return eAuditableNothing;
}

void CHlogApi::SendFaultToAudit(CLogFltElement* pHlogElement)
{
    CFaultDesc *pDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(pHlogElement->GetDescription());
    if (NULL == pDesc)
        return;

    DWORD errorCode = pDesc->GetErrorCode();
    char * name = "Failure";
    char * desc = "A failure occurred in the system.";

    eAuditable audit = IsAuditableFault(errorCode);
    if(eAuditableNothing == audit)
    {
	// No Audit should be performed
	PDELETE(pDesc);
        return;
    }
    else if(eAuditableNotification == audit)
    {
        name = "Notification";
	desc = "A new event was audited";
        //PDELETE(pDesc);
        //return;
    }

    const char *strCode = GetAlarmName(errorCode);
    const char *strCodeDesc = GetAlarmDescription(errorCode);

    char bufferCode[256];
    snprintf(bufferCode, sizeof(bufferCode), "%s", strCode);

    const char *description = pDesc->GetDescription();

    CXMLDOMElement * pFatherNode = NULL;
    pHlogElement->SerializeXml(pFatherNode);

    char *eventSerializeXmlBuffer = NULL;
    pFatherNode->DumpDataAsLongStringEx(&eventSerializeXmlBuffer, TRUE);

    delete pFatherNode;

    AUDIT_EVENT_HEADER_S auditHdr;
    CAuditorApi::PrepareAuditHeader(auditHdr,
                                    "",
                                    eMcms,
                                    "",
                                    "",
                                    eAuditEventTypeInternal,
                                    eAuditEventStatusOk,
                                    name,
                                    desc,
                                    desc,
                                    strCodeDesc);
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "Fault_Details",
                                 eFreeDataTypeXml,
                                 eventSerializeXmlBuffer,
                                 "",
                                 eFreeDataTypeText,
                                 "");

    CAuditorApi api;
    api.SendEventMcms(auditHdr, freeData);

    PDELETEA(eventSerializeXmlBuffer);
    PDELETE(pDesc);
}

bool CHlogApi::SendMessage(const OPCODE opcode,CSegment* pMsgSeg)
{
	CManagerApi api(eProcessFaults);
	STATUS res = api.SendMsg(pMsgSeg, opcode);
	return res;
}

bool CHlogApi::SendFaultToLogger(CLogFltElement* pHlogElement,  const BOOL isFullHlogOnly/*=FALSE*/, BOOL alwaysSend /*= FALSE*/)
{
    CTaskApp * pTask = CProcessBase::GetProcess()->GetCurrentTask();
    if (pTask == NULL)
    {
    	//VNGFE-7331 - kobig , this scenario will lead to a serious leak since pHlogElement is not released
    	POBJDELETE(pHlogElement);
        return false;
    }

    if (pHlogElement == NULL)
    {
    	FTRACEINTO << "pHlogElement IS NULL \n";
        return false;
    }

	pHlogElement->SetMask(FAULTS_MASK);


	if(false == AddToDB_IsFlooding(pHlogElement))
	{
		FTRACEINTO << "\nFault Flooding : " << *pHlogElement << " alwaysSend" << alwaysSend;
		if (!alwaysSend)
		{
			return false;
		}
	}


    if(false == TestFaultValidity(pHlogElement))
    {
        FTRACEINTO << "\nNot valid Fault : " << *pHlogElement;
        return false;
    }

	OPCODE opcode = HLOG_LOGGER;
	if (TRUE == isFullHlogOnly)
	{
		opcode = HLOG_LOGGER_FULL_ONLY;
	}

    SendFaultToAudit(pHlogElement);

	CSegment*  seg = new CSegment;
	((CHlogElement*)pHlogElement)->Serialize(*seg);
	if (SendMessage(opcode,seg) != STATUS_OK)
	{
		FTRACEINTO << "Failed to send fault: " << *pHlogElement;

	}

	return true;
}

void CHlogApi::SendFaultToLogger(const WORD code, const char* p, const BOOL isFullHlogOnly/*=FALSE*/)
{

    CLogFltElement* pHlogElement = new CLogFltElement(code, p);

    SendFaultToLogger(pHlogElement, isFullHlogOnly);

}


void CHlogApi::TaskFault( BYTE subject, DWORD errorCode, BYTE errorLevel, const char *description,
						  BOOL isFullOnly, DWORD boardId, DWORD unitId, WORD theType)
{
    char *pszMsg = FactoryFaultDesc::GetAllocatedFaultDesciption(subject,
                                                                 errorCode,
                                                                 errorLevel,
                                                                 description,
                                                                 boardId,
                                                                 unitId,
                                                                 theType);
    CLogFltElement* pHlogElement = new CLogFltElement(FAULT_TASK_ACTIVE_ALARM, pszMsg);
    SendFaultToLogger(pHlogElement, isFullOnly);

	PDELETEA(pszMsg);

}
bool CHlogApi::TaskFault(const char *faultDescStr, BOOL alwaysSend)
{
    CLogFltElement* pHlogElement = new CLogFltElement(FAULT_TASK_ACTIVE_ALARM, faultDescStr);
    bool bRet =  SendFaultToLogger(pHlogElement, false, alwaysSend);
    return bRet;
}

void CHlogApi::TaskFault(CFaultDesc *faultDesc, BOOL alwaysSend)
{
	COstrStream ostr;
	faultDesc->Serialize(ostr);

    CLogFltElement* pHlogElement = new CLogFltElement(FAULT_TASK_ACTIVE_ALARM, ostr.str().c_str());
    SendFaultToLogger(pHlogElement, false, alwaysSend);
}

void CHlogApi::SoftwareBug(const char* fileName,const WORD lineNum,const DWORD errCode,const char* pszMes)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();

	CFaultAssertDesc assertFault(FAULT_ASSERT_SUBJECT, SOFTWARE_ASSERT_FAILURE,
		                       MAJOR_ERROR_LEVEL,fileName,(DWORD)lineNum,errCode,pszMes,processType);
	char* pszMsg = assertFault.SerializeString(0/*short serialization*/);
	SendFaultToLogger(FAULT_SOFTWARE_BUG, pszMsg, TRUE);
	PDELETEA(pszMsg);
}

void CHlogApi::SystemStartup(const bool isFederal)
{
	CVersions version;
	std::string versionFilePath = VERSIONS_FILE_PATH;
	version.ReadXmlFile(versionFilePath.c_str());

	VERSION_S mcuVer = version.GetMcuVersion();
	VERSION_S mcmsVer = version.GetMcmsVersion();

	CLargeString message;
	if (!isFederal)
	{
			const char*  mcuBaseline = version.GetMcuBaseline();

		    message << "RMX Version : "
		            << mcuVer.ver_major << "."
		            << mcuVer.ver_minor << "."
		            << mcuVer.ver_release << "."
		            << mcuVer.ver_internal;

			if (mcmsVer.ver_major !=0 || mcmsVer.ver_minor!= 0 || mcmsVer.ver_release!= 0 || mcmsVer.ver_internal!= 0)
			{
				message << ", MCMS Version :"
						<< mcmsVer.ver_major << "."
					    << mcmsVer.ver_minor << "."
					    << mcmsVer.ver_release << "."
						<< mcmsVer.ver_internal;
			}
			if (mcuBaseline!=NULL && strlen(mcuBaseline)> 0)
			{
				message << ", MCU Build Version :"
					    << mcuBaseline;
			}
	}
	else
	{
		    message << "RMX Version : "
	            	<< mcuVer.ver_major << "."
	            	<< mcuVer.ver_minor << "."
	            	<< mcuVer.ver_release << ".J";
	}

  CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT,
                       	  MCU_RESTART,
                          STARTUP_ERROR_LEVEL,
                          message.GetString(),
                          CProcessBase::GetProcess()->GetProcessType());

  char* msg = fault.SerializeString(0);
  SendFaultToLogger(FAULT_EMPTY_SLOT, msg);
  PDELETEA(msg);
}

void CHlogApi::SocketDisconnect(DWORD boardId)
{
	char msg[GENERAL_MES_LEN];
	snprintf(msg, GENERAL_MES_LEN, "Socket disconnect (board id: %d)", boardId);
	TaskFault(FAULT_GENERAL_SUBJECT, SOCKET_DISCONNECT, MAJOR_ERROR_LEVEL, msg, FALSE);
}

void CHlogApi::SocketDisconnectNoFault(DWORD boardId)
{
  FTRACEINTOFUNC << "Socket disconnect board id: " << boardId;
}

void CHlogApi::SocketReconnect(DWORD boardId)
{
	char msg[GENERAL_MES_LEN];
	//sprintf(msg,"Socket reconnect (board id: %d)", boardId);
	snprintf(msg, GENERAL_MES_LEN, "Card at slot %d was rebooted successfully", boardId);

	TaskFault(FAULT_GENERAL_SUBJECT, SOCKET_RECONNECT, MAJOR_ERROR_LEVEL, (const char *)msg, FALSE);

}

void CHlogApi::CmReconnect(const WORD boardId, const WORD subBoardId, const WORD theType)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultCardDesc cardFault(FAULT_CARD_SUBJECT, CARD_MANAGER_RECONNECT, MAJOR_ERROR_LEVEL,  boardId, 0, theType, "Card Manager reconnect", processType);
	char* msg = cardFault.SerializeString(0);
	SendFaultToLogger(FAULT_CARD_MANAGER_RECONNECT, msg);
	PDELETEA(msg);
}

void CHlogApi::EmbUserMsgFault(const WORD boardId, const WORD subBoardId, const string desc, const WORD theType)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultCardDesc cardFault(FAULT_CARD_SUBJECT, EXTERNAL_FAULT_EMB, MAJOR_ERROR_LEVEL,  boardId, 0, theType, desc.c_str(), processType);
	char* msg = cardFault.SerializeString(0);
	SendFaultToLogger(FAULT_EXTERNAL_FAULT_EMB, msg);
	PDELETEA(msg);
}

void CHlogApi::ReservationError(const WORD errorCode, const DWORD confId, const DWORD partyId,
								 const char* confName, const char* partyName)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultReservationDesc resFault(FAULT_RESERVATION_SUBJECT, errorCode, MAJOR_ERROR_LEVEL,
									confId, partyId, confName, partyName, processType);
	char* msg = resFault.SerializeString(0);
	SendFaultToLogger(FAULT_EMPTY_SLOT, msg);
	PDELETEA(msg);
}

void CHlogApi::AppServerInternalError(const char *error)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, APPLICATION_SERVER_INTERNAL_ERROR,
							MAJOR_ERROR_LEVEL,(char *)error, processType);
	char* msg = fault.SerializeString(0);
	SendFaultToLogger(FAULT_EMPTY_SLOT, msg);
	PDELETEA(msg);
}

void  CHlogApi::BackupAppError(const DWORD dwFailure, const char *error)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, dwFailure, MAJOR_ERROR_LEVEL,(char*)error, processType);
	char* msg = fault.SerializeString(0);
	SendFaultToLogger(FAULT_EMPTY_SLOT, msg);
	PDELETEA(msg);
}

void  CHlogApi::CardsConfigError(const DWORD errorCode,const BYTE  faultLevel)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, errorCode, faultLevel,"", processType);
	char* msg = fault.SerializeString(0);
	SendFaultToLogger(FAULT_EMPTY_SLOT, msg);
	PDELETEA(msg);
}

void CHlogApi::GateKeeperError(const WORD boardId, const BYTE  faultLevel, const char *str)
{
	char  mes[100];
	int   tempLen=0;
	sprintf(mes ,"Service:%d Reason: ",boardId);
	tempLen = strlen(mes);
	strncat(mes,str,99-tempLen);

	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, AA_GATE_KEEPER_ERROR, faultLevel, mes, processType);
	char* msg = fault.SerializeString(0);
	SendFaultToLogger(FAULT_EMPTY_SLOT, msg);
	PDELETEA(msg);
}

void CHlogApi::GateKeeperMessage(const WORD boardId, const BYTE  faultLevel, const char *str)
{
	char  mes[100];
	int   tempLen=0;
	sprintf(mes ,"Service:%d Reason: ",boardId);
	tempLen=strlen(mes);
	strncat(mes,str,99-tempLen);

	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, GATEKEEPER_MESSAGE, faultLevel, mes, processType);
	char* msg = fault.SerializeString(0);
	SendFaultToLogger(FAULT_EMPTY_SLOT, msg);
	PDELETEA(msg);
}

void  CHlogApi::BadSpontaneousIndication(const WORD boardId ,const WORD m_unitId, const WORD status)
{
	char  mes[100];
	int   tempLen=0;
	sprintf(mes ,"Board ID: %d, Unit ID: %d, Reason: %d. ",boardId,m_unitId,status);
	tempLen = strlen(mes);

	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT,BAD_SPONTANEOUS_INDICATION, MAJOR_ERROR_LEVEL,mes, processType);
	char* msg = fault.SerializeString(0);
	SendFaultToLogger(FAULT_EMPTY_SLOT, msg, TRUE);
	PDELETEA(msg);
}

void  CHlogApi::ConflictInSystemConfiguration(const DWORD Error)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultDesc fault(FAULT_STARTUP_SUBJECT,Error,STARTUP_ERROR_LEVEL, processType);
	char* msg = fault.SerializeString(0);
	SendFaultToLogger(FAULT_EMPTY_SLOT, msg);
	PDELETEA(msg);
}

void  CHlogApi::CsRecoveryStatus(string errStr)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault( FAULT_GENERAL_SUBJECT, CS_RECOVERY_STATUS, MAJOR_ERROR_LEVEL, errStr.c_str(), processType );
	char* msg = fault.SerializeString(0);

	SendFaultToLogger(FAULT_FATAL_FAIURE, msg);
	PDELETEA(msg);
}

void CHlogApi::XmlParseError(string errStr)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault( FAULT_GENERAL_SUBJECT, XML_PARSE_ERROR, MAJOR_ERROR_LEVEL, errStr.c_str(), processType );
	char* msg = fault.SerializeString(0);

	SendFaultToLogger(FAULT_XML_PARSE, msg);
	PDELETEA(msg);
}

void CHlogApi::TestError(string errStr)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault( FAULT_GENERAL_SUBJECT, TEST_ERROR, MAJOR_ERROR_LEVEL, errStr.c_str(), processType );
	char* msg = fault.SerializeString(0);

	SendFaultToLogger(FAULT_EMPTY_SLOT/*no difference*/, msg);
	PDELETEA(msg);
}

void CHlogApi::ServiceAttachedToSpanNotExists(string errStr)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, ISDN_SPAN_ATTACHED_TO_NONEXISTING_SERVICE, MAJOR_ERROR_LEVEL, errStr.c_str(), processType);
	char* msg = fault.SerializeString(0);

	SendFaultToLogger(FAULT_EMPTY_SLOT/*no difference*/, msg);
	PDELETEA(msg);
}

void CHlogApi::IsdnServiceInconsistentSpanType(string errStr)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, INCONSISTENT_SPAN_TYPE, MAJOR_ERROR_LEVEL, errStr.c_str(), processType);
	char* msg = fault.SerializeString(0);

	SendFaultToLogger(FAULT_EMPTY_SLOT/*no difference*/, msg);
	PDELETEA(msg);
}

void CHlogApi::NumOfConfiguredIsdnSpansExceeded(string errStr)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, NUM_OF_CONFIGURED_SPANS_EXCEEDED, MAJOR_ERROR_LEVEL, errStr.c_str(), processType);
	char* msg = fault.SerializeString(0);

	SendFaultToLogger(FAULT_EMPTY_SLOT/*no difference*/, msg);
	PDELETEA(msg);
}

void CHlogApi::CsGeneralFaults(string errStr)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, EXTERNAL_FAULT_CS, MAJOR_ERROR_LEVEL, errStr.c_str(), processType);
	char* msg = fault.SerializeString(0);

	SendFaultToLogger(FAULT_EMPTY_SLOT/*no difference*/, msg);
	PDELETEA(msg);
}

void CHlogApi::SessionTimerFault(string errStr)
{
	eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	CFaultGeneralDesc fault(FAULT_GENERAL_SUBJECT, SESSION_TIMER_FAULT, MAJOR_ERROR_LEVEL, errStr.c_str(), processType);
	char* msg = fault.SerializeString(0);

	SendFaultToLogger(FAULT_EMPTY_SLOT/*no difference*/, msg);
	PDELETEA(msg);
}

