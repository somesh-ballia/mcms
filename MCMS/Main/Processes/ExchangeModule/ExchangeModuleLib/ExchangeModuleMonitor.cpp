// ExchangeModuleMonitor.cpp: implementation of the ExchangeModuleMonitor class.
//
//////////////////////////////////////////////////////////////////////


#include "OpcodesMcmsCommon.h"
#include "ExchangeModuleMonitor.h"
#include "TaskApi.h"
#include "Trace.h"

#include "Macros.h"
#include "ExchangeModuleCfgGet.h"
#include "Request.h"
#include "OpcodesMcmsCommon.h"
#include "ExchangeModuleCfg.h"
#include "ExchangeModuleProcess.h"
#include "ExchangeClientCntl.h"
#include "ExchangeDataTypes.h"
#include "ExchangeModuleManager.h"
#include "OpcodesMcmsInternal.h"
#include "DummyEntry.h"
#include "Trace.h"
#include "TraceStream.h"
#include "EncodingConvertor.h"
#define MAX_DISPLAY_NAME_LEN		H243_NAME_LEN
#define MAX_CHAR_OPTIONS_UTF8_LEN	    3
////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////
/*EXCHNAGE_UPDATE_CONF_DETAILS_REQ*/

PBEGIN_MESSAGE_MAP(CExchangeModuleMonitor)
  ONEVENT( XML_REQUEST    ,IDLE    , CExchangeModuleMonitor::HandlePostRequest )
  ONEVENT( EXCHNAGE_UPDATE_CONF_DETAILS_REQ , IDLE , CExchangeModuleMonitor::OnConfPartyUpdateConfDetailsRequest )
  ONEVENT( EXCHNAGE_MONITOR_UPDATE_SET_CFG_PARAMS, IDLE , CExchangeModuleMonitor::OnSetLastSetExchangeCfgIndication )
PEND_MESSAGE_MAP(CExchangeModuleMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CExchangeModuleMonitor)
	ON_TRANS("TRANS_MCU", "GET_MCU_EXCHANGE_CONFIG_PARAMS", CExchangeModuleCfgGet, CExchangeModuleMonitor::OnGetExchangeCfg)
	ON_TRANS("TRANS_MCU", "GET_LAST_SET_MCU_EXCHANGE_CONFIG_INDICATION", CDummyEntry, CExchangeModuleMonitor::OnGetLastSetExchangeCfgIndication)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void ExchangeModuleMonitorEntryPoint(void* appParam)
{
	CExchangeModuleMonitor *monitorTask = new CExchangeModuleMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExchangeModuleMonitor::CExchangeModuleMonitor()
{
	m_nLastSetExchangeCfgIndication = STATUS_IN_PROGRESS;
	m_LastSetExchnageStatusDesc = "";
}

CExchangeModuleMonitor::~CExchangeModuleMonitor()
{

}

/////////////////////////////////////////////////////////////////////////////
STATUS CExchangeModuleMonitor::OnGetExchangeCfg(CRequest* pGetRequest)
{
	CExchangeModuleCfgGet* pExchangeCfgGet = new CExchangeModuleCfgGet();
	*pExchangeCfgGet  = *(CExchangeModuleCfgGet*)pGetRequest->GetRequestObject();
	pExchangeCfgGet->SetIsForEMA(true);
	pGetRequest->SetConfirmObject(pExchangeCfgGet);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CExchangeModuleMonitor::OnGetLastSetExchangeCfgIndication(CRequest* pGetRequest)
{
	PTRACE(eLevelInfoNormal,"CExchangeModuleMonitor::OnGetLastSetExchangeCfgIndication");
	pGetRequest->SetConfirmObject(new CDummyEntry());
	pGetRequest->SetStatus(m_nLastSetExchangeCfgIndication);
	pGetRequest->SetExDescription(m_LastSetExchnageStatusDesc.c_str());
	m_nLastSetExchangeCfgIndication = STATUS_IN_PROGRESS; //after retriving the last status gets it back to IN_PROGRESS
	m_LastSetExchnageStatusDesc = "";
	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////
STATUS CExchangeModuleMonitor::OnSetLastSetExchangeCfgIndication(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CExchangeModuleMonitor::OnSetLastSetExchangeCfgIndication");
	*pParam >> (DWORD&)m_nLastSetExchangeCfgIndication;
    *pParam >> m_LastSetExchnageStatusDesc;
	ResponedClientRequest(0,NULL);
	return STATUS_OK;
}
void   CExchangeModuleMonitor::OnSpcialCharExceedBufferDisplayName(CCommResApi* pRsrv,const char* pDisplayName)
{
	char tmp[MAX_DISPLAY_NAME_LEN];
    COstrStream statusString;
    strcpy_safe(tmp,pDisplayName);
    STATUS statusFormat = CEncodingConvertor::ValidateString("UTF-8",tmp ,statusString);
    if(STATUS_OK != statusFormat)
    {
        	TRACESTR(eLevelInfoNormal) << "failure  encoding after copying string attempt to recover string ";
        	int new_lengths[] ={MAX_DISPLAY_NAME_LEN -2,MAX_DISPLAY_NAME_LEN -3,MAX_DISPLAY_NAME_LEN -4};
        	int new_len=0;
        	for(int i =0; i < MAX_CHAR_OPTIONS_UTF8_LEN ; i++)
        	{
        		new_len = new_lengths[i];
        	    tmp[new_len] = '\0';
        	    statusFormat = CEncodingConvertor::ValidateString("UTF-8",tmp ,statusString);
        	    if(STATUS_OK == statusFormat)
        	    {
        	    	pRsrv->SetDisplayName(tmp);
        	    	break;
        	     }
        	}

        	       if(STATUS_OK != statusFormat)
        	       {
        	    	   TRACESTR(eLevelInfoNormal) << "failure to restore string do valid format do default copy";
        	       }
    }
   else
       pRsrv->SetDisplayName(pDisplayName);
}
////////////////////////////////////////////////////////////////////////////
STATUS CExchangeModuleMonitor::OnConfPartyUpdateConfDetailsRequest(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CExchangeModuleMonitor::OnConfPartyUpdateConfDetailsRequest");
	const CCalendarItem* pCalendarItem= NULL;
	CExchangeModuleProcess* pProcess = (CExchangeModuleProcess*)CProcessBase::GetProcess();

	STATUS status = STATUS_OK;

	CCommResApi* pRsrv= new CCommResApi;
	pRsrv->DeSerialize(NATIVE,*pParam);

	// if Res is Ad-hoc, get NumericId and find appointment by NumericId
	if( YES == pRsrv->IsAdHocConf() )
	{
		PTRACE2(eLevelInfoNormal,"CExchangeModuleMonitor::OnConfPartyUpdateConfDetailsRequest, find by NumericConfId=",pRsrv->GetNumericConfId());
		pCalendarItem = pProcess->GetCalendarItemList()->FindItemByNumericId(pRsrv->GetNumericConfId());
	}
	// else if Res has appointment id find by AppointmentId
	else if( strlen(pRsrv->GetAppointmentId()) > 0 )
	{
		pCalendarItem = pProcess->GetCalendarItemList()->FindItemByAppointmentId(pRsrv->GetAppointmentId());
		PTRACE2(eLevelInfoNormal,"CExchangeModuleMonitor::OnConfPartyUpdateConfDetailsRequest, find by AppointmentId=",pRsrv->GetAppointmentId());
	}

	if( NULL != pCalendarItem )
	{
		// handle special characters in DisplayName
		string sDisplayName = pCalendarItem->GetAppointmentSubject();
		int len = NeededSizeForNewArray(sDisplayName.c_str());
		ALLOCBUFFER(sDisplayNameSpecialChars, len+1);
		memset(sDisplayNameSpecialChars, '\0', len+1);
        ChangeSpecialChar(sDisplayName.c_str(), FALSE, sDisplayNameSpecialChars);

        TRACESTR(eLevelInfoNormal) << "\nCExchangeModuleMonitor::OnConfPartyUpdateConfDetailsRequest"
        					   << "\nsDisplayName:               " << sDisplayName
        					   << "\nsDisplayNameNoSpecialChars: " << sDisplayNameSpecialChars
        					   << "\n Length (with out null) : " << len;
        //VNGFE-9037
        if(len > MAX_DISPLAY_NAME_LEN -1)
        {
        	OnSpcialCharExceedBufferDisplayName(pRsrv,sDisplayNameSpecialChars);
        }
        else
        	pRsrv->SetDisplayName(sDisplayNameSpecialChars);

        DEALLOCBUFFER(sDisplayNameSpecialChars);
		
		// changed due to a reqest from RSS group (PCTCRSS-224)
        	///pRsrv->SetAppointmentId(pCalendarItem->GetItemId().c_str());
	        pRsrv->SetAppointmentId(pCalendarItem->GetItemUId().c_str());

		pRsrv->SetMeetingOrganizer(pCalendarItem->GetOrganizer().c_str());
		pRsrv->SetDuration(pCalendarItem->GetDuration());
		pRsrv->SetEnableRecording(pCalendarItem->GetRecording());
		CStructTm dt = pCalendarItem->GetExchangeConfStartTime();
//		char buff[128] = "";
//		dt.DumpToBuffer(buff);
//		PTRACE2(eLevelInfoNormal,"CExchangeModuleMonitor::OnConfPartyUpdateConfDetailsRequest, pCalendarItem->GetExchangeConfStartTime() = ", buff);
		pRsrv->SetExchangeConfStartTime( pCalendarItem->GetExchangeConfStartTime() );
		pRsrv->SetGatheringEnabled(true); //gathering will always be enabled if calendaring is is involved
		if( TRUE == pCalendarItem->GetRecording() )
		{
			pRsrv->SetStartRecordingPolicy(START_RECORDING_IMMEDIATELY);
		}
		pRsrv->SetEntryPassword(pCalendarItem->GetMeetingPassword().c_str());
		pRsrv->SetH243Password(pCalendarItem->GetChairPassword().c_str());
		pRsrv->SetIsStreaming(pCalendarItem->GetIsStreaming());
		pRsrv->SetLanguageFromString(pCalendarItem->GetLanguage().c_str());

		//need to change the parameter m_terminateConfAfterChairDroppedOnOff before changing m_startConfRequiresLeaderOnOff parameter
		if ((FALSE==pCalendarItem->GetIsChairPasswordRequired())&&(TRUE==pRsrv->GetStartConfRequiresLeaderOnOff()))
		{
			pRsrv->SetTerminateConfAfterChairDroppedOnOff(FALSE);
		}

		pRsrv->SetStartConfRequiresLeaderOnOff(pCalendarItem->GetIsChairPasswordRequired());

		std::string strIpAccessNumber = pCalendarItem->GetDialInPrefix() + pCalendarItem->GetNumericId();
		if( strIpAccessNumber.length() > 0 )
			pRsrv->SetIpNumberAccess(strIpAccessNumber.c_str());

		if( pCalendarItem->GetAudioNumber1().length() > 0 )
			pRsrv->SetNumberAccess_1(pCalendarItem->GetAudioNumber1().c_str());
		if( pCalendarItem->GetAudioNumber2().length() > 0 )
			pRsrv->SetNumberAccess_2(pCalendarItem->GetAudioNumber2().c_str());

//		puts("\n**start************");
//		printf("RSRV new data: Record(%d), EntryPwd(%s), 243Pwd(%s)",
//				pCalendarItem->GetRecording(),pCalendarItem->GetMeetingPassword().c_str(),pCalendarItem->GetChairPassword().c_str());
//		puts("\n**end**************\n");
	}
	else
	{
		status = STATUS_NOT_FOUND;
		PTRACE(eLevelInfoNormal,"CExchangeModuleMonitor::OnConfPartyUpdateConfDetailsRequest, appointment not found");
	}


	COstrStream str;
	pRsrv->Serialize(NATIVE, str);
	str << (WORD)status;
	CSegment*  pRspMsg = new CSegment;
  	*pRspMsg << str.str().c_str();
	ResponedClientRequest(0,pRspMsg);
	POBJDELETE(pRsrv);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////////

// Copied from psosxml file for VNGR-14306

//The size of the new array, with all the special characters.
int CExchangeModuleMonitor::NeededSizeForNewArray(const char* org_str, BOOL isAllocSpaceForSpecialChar)
{
	int new_length = 0;

    if(isAllocSpaceForSpecialChar)
    {
        DWORD len = strlen(org_str);
        for (DWORD i=0; i<len; i++)
        {
            if (org_str[i]=='&')
                new_length+=5;
            else if(org_str[i]=='<')
                new_length+=4;
            else if(org_str[i]=='>')
                new_length+=4;
            else if(org_str[i]=='\'')
                new_length+=6;
            else if(org_str[i]=='"')
                new_length+=6;
            else
                new_length++;
        }
    }
    else
    {
        new_length = strlen(org_str);
    }

    return new_length;
}

//////////////////////////////////////////////////////////////////////////////////
void CExchangeModuleMonitor::ChangeSpecialChar(const char* org_str, BYTE bRemoveSpecialChar, char* new_string)
{
	if (bRemoveSpecialChar)
	{
		DWORD textPointer = strlen(new_string);
		DWORD len = strlen(org_str);
		for (DWORD i=0; i<len; i++)
		{
			if (org_str[i]=='&')
			{
				strcat(new_string, "&amp;");
				textPointer+=5;
			}
			else if(org_str[i]=='<')
			{
				strcat(new_string, "&lt;");
				textPointer+=4;
			}
			else if(org_str[i]=='>')
			{
				strcat(new_string, "&gt;");
				textPointer+=4;
			}
			else if(org_str[i]=='\'')
			{
				strcat(new_string, "&apos;");
				textPointer+=6;
			}
			else if(org_str[i]=='"')
			{
				strcat(new_string, "&quot;");
				textPointer+=6;
			}
			else
			{
				new_string[textPointer] = org_str[i];
				new_string[textPointer+1]='\0';
				++textPointer;
			}
		}
	}
	else
	{
		DWORD textPointer = strlen(new_string);
		DWORD len = strlen(org_str);
		for (DWORD i=0; i<len; i++)
		{
			if (strncmp(org_str+i, "&amp;", 5)==0)
			{
				new_string[textPointer] = '&';
				++textPointer;
				i+=4;
			}
			else if(strncmp(org_str+i, "&lt;", 4)==0)
			{
				new_string[textPointer] = '<';
				++textPointer;
				i+=3;
			}
			else if(strncmp(org_str+i, "&gt;", 4)==0)
			{
				new_string[textPointer] = '>';
				++textPointer;
				i+=3;
			}
			else if(strncmp(org_str+i, "&apos;", 6)==0)
			{
				new_string[textPointer] = '\'';
				++textPointer;
				i+=5;
			}
			else if(strncmp(org_str+i, "&quot;", 6)==0)
			{
				new_string[textPointer] = '\"';
				++textPointer;
				i+=5;
			}
			else
			{
				new_string[textPointer] = org_str[i];
				new_string[textPointer+1]='\0';
				++textPointer;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////
