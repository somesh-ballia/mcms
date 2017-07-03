// SegmentFuncForRequestHandler.cpp

#include "SegmentFuncForRequestHandler.h"
#include "TaskApi.h"
#include "ApiStatuses.h"
#include "ZipWrapper.h"
#include "TraceStream.h"
#include "ActionRedirection.h"
#include "XmlMiniParser.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"

STATUS CSegmentFuncForRequestHandler::IsReadyToHandleRequests()
{
    CProcessBase *pProcess = CProcessBase::GetProcess();
    if(pProcess == NULL)
      {
	FTRACEINTO << "\nSegmentFuncForRequestHandler::IsReadyToHandleRequests: pProcess is NULL";
	return eProcessInvalid;
     }

    eProcessStatus processState = pProcess->GetProcessStatus();


    if(eProcessMinor != pProcess->GetProcessStatus() &&
       eProcessMajor != pProcess->GetProcessStatus() &&
       eProcessNormal != pProcess->GetProcessStatus())
    {
        FTRACEINTO << "\nSegmentFuncForRequestHandler::IsReadyToHandleRequests. Reject transaction, Process is not ready\n"
                   << "State = " << GetProcessStatusName(processState) << ", Process = " << (DWORD)pProcess;
        return STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP;
    }

    return STATUS_OK;
}

STATUS CSegmentFuncForRequestHandler::ValidateTransaction(CActionRedirectionMap *pActionRedirectionMap, char* pszNonZippedBuffer, char* pszStartActionName,
										int nConnId, ConnectionDetails_S &connDetails, CStringToProcessEntry& destProcessEntry, eOtherProcessQueueEntry& eDestTask,
										bool& bSetConnectionUponReturn, WORD authorization/* = SUPER*/, const char* loginName /*=""*/)
{
	char *pszEndActionName=NULL;
	ALLOCBUFFER(pTransName, 100);

	if(pActionRedirectionMap->IsValidTransName(pszNonZippedBuffer, &pTransName))
	{
		pszStartActionName = CXmlMiniParser::GetActionName(pszNonZippedBuffer, &pszEndActionName);

		if(!pszStartActionName)
		{
			DEALLOCBUFFER(pTransName);
			FTRACEINTO << "CSegmentFuncForRequestHandler::ValidateTransaction - Action Name missing: ";
      return STATUS_ACTION_NODE_IS_MISSING_OR_INVALID;
		}

		if(strstr(pszStartActionName,"LOGIN") == pszStartActionName)
		{
            // extract user name and station because this client still not in the connection list
            char strUserName[OPERATOR_NAME_LEN]={0};
            char strStation[MAX_AUDIT_WORKSTATION_NAME_LEN]={0};
            memset(strStation,'\0',MAX_AUDIT_WORKSTATION_NAME_LEN);
            bool userNameParseRes = CXmlMiniParser::GetUserName(pszStartActionName,strUserName);
            bool stationNameParseRes = CXmlMiniParser::GetStationName(pszStartActionName,strStation);

			int nMessageID = CXmlMiniParser::GetMessageId(pszNonZippedBuffer);
			if (nMessageID == -2 )
			{
				FTRACEINTOFUNC << "Station Name :" << strStation << " Status : " << STATUS_NODE_VALUE_INVALID;
				return STATUS_NODE_VALUE_INVALID;
			}
            if(userNameParseRes)
            {
                connDetails.userName = strUserName;
            }
            if(stationNameParseRes)
            {
                connDetails.workStation = strStation;
            }

			if((authorization != ANONYMOUS))
			{
				if(userNameParseRes)
				{
					if(strcmp(strUserName,loginName))
						bSetConnectionUponReturn = true;
					else
					{
						DEALLOCBUFFER(pTransName);
						FTRACEINTOFUNC << "Station Name :" << strStation << " Status : " << STATUS_LOGIN_EXISTS;
						return STATUS_LOGIN_EXISTS;
					}
				}
			}
			else
			{
				destProcessEntry = pActionRedirectionMap->GetDedicatedManager(pszNonZippedBuffer,pszStartActionName,pszEndActionName,eDestTask);
			    bSetConnectionUponReturn = true;
			}
		}
		else
		{
			if((authorization == ANONYMOUS))
			{
				FTRACESTR(eLevelInfoNormal) << "Failed finding user in connection list nConnId: " << nConnId;
				DEALLOCBUFFER(pTransName);
				return STATUS_FAIL_TO_LOCATE_USER;
			}
		}

		if(eProcessTypeInvalid == destProcessEntry.GetProcessType())
            destProcessEntry = pActionRedirectionMap->GetDedicatedManager(pszNonZippedBuffer,pszStartActionName,pszEndActionName,eDestTask);
	}
	else
	{
		if(strstr(pszNonZippedBuffer,"<REQUEST_") == NULL)
		{
			FTRACEINTO << "CSegmentFuncForRequestHandler::ValidateTransaction: Transaction Name missing or invalid ";
			DEALLOCBUFFER(pTransName);
			return STATUS_TRANSACTION_DOES_NOT_EXIST;
		}
        else
        {
        	destProcessEntry.SetProcessType(eProcessEndpointsSim);
            eDestTask = eManager;	// ??
        }
    }

	DEALLOCBUFFER(pTransName);
	return STATUS_OK;
}

void CSegmentFuncForRequestHandler::RemovePasswordFromString(const char* pszSearchString, char* pszNewString, const char* pszElementOpenTag, const char* pszElementCloseTag)
{
	char* pszStartPassword = (char*)strstr(pszSearchString,pszElementOpenTag);
	char *pszEndPassword;

	if (pszStartPassword)	//if password exist in the transaction
	{
		pszStartPassword += strlen(pszElementOpenTag);
		pszEndPassword = strstr(pszStartPassword,pszElementCloseTag);

		if(pszEndPassword)
		{
			strncpy(pszNewString, pszSearchString, pszStartPassword-pszSearchString);
			strncat(pszNewString, "*********", 9);
			char* pszStartPassword2 = strstr(pszEndPassword,pszElementOpenTag);	//look if the password appear twice
			if (pszStartPassword2)
			{
				pszStartPassword2 += strlen(pszElementOpenTag);
				char *pszEndPassword2 = strstr(pszStartPassword2,pszElementCloseTag);
				if (pszEndPassword && pszEndPassword2)
				{
					strncat(pszNewString, pszEndPassword, pszStartPassword2-pszEndPassword);
					strncat(pszNewString, "*********", 9);
					strcat(pszNewString, pszEndPassword2);
				}
			}
			else
				strcat(pszNewString, pszEndPassword);
		}
		else
			strcpy(pszNewString, pszSearchString);
	}
	else
		strcpy(pszNewString, pszSearchString);
}

STATUS CSegmentFuncForRequestHandler::PrepareUncompressedString(DWORD nLen,char* pszMsg, char** pszMsgToSend, BYTE &bCompressedForSend, DWORD& nNewLen, BYTE bCompressed)
{
	nNewLen = nLen;

	if((!bCompressed) && (nNewLen > MAX_NON_ZIPPED))
	{
		CZipWrapper ZipWrapper;

		char *pszZippedBuffer = new char[nNewLen];

		if (!CSysConfigBase::IsUnderJITCState())
		{
		      FTRACEINTOFUNC << "Maman before zip:\n" << pszMsg;
		}

		int nZippedLen = ZipWrapper.Deflate((unsigned char*)pszMsg,nLen,(unsigned char*)pszZippedBuffer,nLen);

		if(nZippedLen != -1)
		{
			FTRACESTR(eLevelInfoNormal) << "CSegmentFuncForRequestHandler::PrepareUncompressedString Maman after zip";
			nNewLen = nZippedLen;
			bCompressedForSend = true;
			*pszMsgToSend = pszZippedBuffer;
		}
		else
		{
			FTRACESTR(eLevelInfoNormal) << "CSegmentFuncForRequestHandler::PrepareUncompressedString Maman - bad zip";
			DEALLOCBUFFER(pszZippedBuffer);
		}
	}

	if(nNewLen > (1024*1024 - 1))  // 1MB limit
	{
		if(*pszMsgToSend != pszMsg)
			DEALLOCBUFFER(pszMsgToSend);

		return STATUS_REQUEST_TOO_LONG;
	}
	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CSegmentFuncForRequestHandler::PrepareSegmentBeforeSend(BYTE bCompressedForSend, DWORD nNewLen, char* pszMsgToSend,
									  CSegment *pSegment, int nConnId, CStringToProcessEntry &destProcessEntry,
                                      eOtherProcessQueueEntry eDestTask, int nPort,
                                      ConnectionDetails_S &connDetails, BYTE bSHM,
                                      const string & strCharSet /*= "UTF-8"*/,
                                      WORD nAuthorization /*=SUPER*/, string strLogin /*=""*/,
                                      const string & strClientCertificateSubj, DWORD ifNoneMatch)
{
    CPostXmlHeader postXmlHeader(nNewLen,
    							 nAuthorization,
                                 bCompressedForSend,
                                 strCharSet,
                                 connDetails.workStation,
                                 connDetails.userName,
                                 connDetails.clientIp,
                                 destProcessEntry.GetIsAudit(),
                                 destProcessEntry.GetTransName(),
                                 destProcessEntry.GetTransDesc(),
                                 destProcessEntry.GetTransFailureDesc(),
                                 ifNoneMatch,nConnId);

	postXmlHeader.Serialize(*pSegment);
    pSegment->Put((BYTE*)pszMsgToSend,nNewLen);

	if((eProcessApacheModule == destProcessEntry.GetProcessType()) && (eDestTask == eManager))
	{
		*pSegment << (DWORD) nConnId;
		*pSegment << (DWORD) nPort;
		*pSegment << (BYTE)	bSHM;
	}

	if(eProcessConfParty == destProcessEntry.GetProcessType() ||
       eProcessAuthentication == destProcessEntry.GetProcessType() ||eProcessResource == destProcessEntry.GetProcessType())
	{
		*pSegment << (DWORD) strLogin.length();
		if( strLogin.length() )
			*pSegment << (char*) strLogin.c_str();
	}

	// send client certificate subject to ApacheModuleManager and then to AuthenticationManager
	if( ( eProcessApacheModule == destProcessEntry.GetProcessType() ||  eProcessAuthentication == destProcessEntry.GetProcessType() ) && (eDestTask == eManager))
	{
		*pSegment << (DWORD) strClientCertificateSubj.length();
		*pSegment << (char*) strClientCertificateSubj.c_str();
	}

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
void CSegmentFuncForRequestHandler::PrintRequestInTrace(const eOtherProcessQueueEntry eDestTask, const char* pszStartActionName, const char* pszNonZippedBuffer,const char* pMsgHeader)
{
	char *pszEndActionName=NULL;

    if( eManager == eDestTask ) //it means Set request
	{
    	if(!pszStartActionName)
    	{
    		pszStartActionName = CXmlMiniParser::GetActionName(pszNonZippedBuffer,
    		                                                           &pszEndActionName);
    	}
		PrintSetRequestInTrace(pszStartActionName, pszNonZippedBuffer,pMsgHeader);
	}
	else if( eMonitor == eDestTask )
	{
		BOOL is_apache_debug_mode = IsApacheDebugMode();

	    if( is_apache_debug_mode )
		   FTRACESTR(eLevelInfoNormal) << "CSegmentFuncForRequestHandler::PrintRequestInTrace - Get request received:"
						           << std::endl << pszNonZippedBuffer;
	}

}
//////////////////////////////////////////////////////////////////////
BOOL CSegmentFuncForRequestHandler::IsApacheDebugMode()
{
	BOOL is_apache_debug_mode = FALSE;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    std::string key = CFG_KEY_APACHE_PRINT;
    sysConfig->GetBOOLDataByKey(key, is_apache_debug_mode);

    return is_apache_debug_mode;
}
//////////////////////////////////////////////////////////////////////
void CSegmentFuncForRequestHandler::PrintSetRequestInTrace(const char* pszStartActionName, const char* pszNonZippedBuffer,const char* pMsgHeader)
{
	if (pszStartActionName)
	{
		if((strstr(pszStartActionName,"LOGIN") == pszStartActionName) || (strstr(pszStartActionName,"NEW_OPERATOR") == pszStartActionName))
		{
			ALLOCBUFFER(pszNonZippedBufferWithShadowPassword, strlen(pszNonZippedBuffer)+10);
			memset(pszNonZippedBufferWithShadowPassword, 0, strlen(pszNonZippedBuffer)+10);
			ALLOCBUFFER(pszNonZippedBufferWithShadowPassword2, strlen(pszNonZippedBuffer)+20);
			memset(pszNonZippedBufferWithShadowPassword2, 0, strlen(pszNonZippedBuffer)+20);

			RemovePasswordFromString(pszNonZippedBuffer, pszNonZippedBufferWithShadowPassword, "<PASSWORD>", "</PASSWORD>");
			RemovePasswordFromString(pszNonZippedBufferWithShadowPassword, pszNonZippedBufferWithShadowPassword2, "<NEW_PASSWORD>", "</NEW_PASSWORD>");

			FTRACESTR(eLevelInfoNormal) << "CSegmentFuncForRequestHandler::PrintSetRequestInTrace - (login\\new operator) Set request received:"
									<< std::endl << pszNonZippedBufferWithShadowPassword2;
			DEALLOCBUFFER(pszNonZippedBufferWithShadowPassword);
			DEALLOCBUFFER(pszNonZippedBufferWithShadowPassword2);
		}
		else if (strstr(pszStartActionName,"CHANGE_PASSWORD") == pszStartActionName)
		{
			ALLOCBUFFER(pszNonZippedBufferWithShadowPassword, strlen(pszNonZippedBuffer)+10);
			memset(pszNonZippedBufferWithShadowPassword, 0, strlen(pszNonZippedBuffer)+10);
			ALLOCBUFFER(pszNonZippedBufferWithShadowPassword2, strlen(pszNonZippedBuffer)+20);
			memset(pszNonZippedBufferWithShadowPassword2, 0, strlen(pszNonZippedBuffer)+20);

			RemovePasswordFromString(pszNonZippedBuffer, pszNonZippedBufferWithShadowPassword, "<OLD_PASSWORD>", "</OLD_PASSWORD>");
			RemovePasswordFromString(pszNonZippedBufferWithShadowPassword, pszNonZippedBufferWithShadowPassword2, "<NEW_PASSWORD>", "</NEW_PASSWORD>");

			FTRACESTR(eLevelInfoNormal) << "CSegmentFuncForRequestHandler::PrintSetRequestInTrace - Change Opertaor Set request received:"
									<< std::endl << pszNonZippedBufferWithShadowPassword2;
			DEALLOCBUFFER(pszNonZippedBufferWithShadowPassword);
			DEALLOCBUFFER(pszNonZippedBufferWithShadowPassword2);
		}
		else
		{
			if(pMsgHeader)
				FTRACESTR(eLevelInfoNormal) << "CSegmentFuncForRequestHandler::PrintSetRequestInTrace - Set request received: " << pMsgHeader
									<< std::endl << pszNonZippedBuffer;
			else
				FTRACESTR(eLevelInfoNormal) << "CSegmentFuncForRequestHandler::PrintSetRequestInTrace - Set request received: "
									<< std::endl << pszNonZippedBuffer;
		}
	}
	else
	{
		if (strstr(pszNonZippedBuffer,"<REQUEST_") != NULL)
		{
			ALLOCBUFFER(pszNonZippedBufferWithShadowPassword, strlen(pszNonZippedBuffer)+10);
			memset(pszNonZippedBufferWithShadowPassword, 0, strlen(pszNonZippedBuffer)+10);

			RemovePasswordFromString(pszNonZippedBuffer, pszNonZippedBufferWithShadowPassword, "<PASSWORD>", "</PASSWORD>");
			FTRACESTR(eLevelInfoNormal) << "CSegmentFuncForRequestHandler::PrintSetRequestInTrace - External DB Set request received:"
									<< std::endl << pszNonZippedBufferWithShadowPassword;
			DEALLOCBUFFER(pszNonZippedBufferWithShadowPassword);
		}
		else
			FTRACESTR(eLevelInfoNormal) << "CSegmentFuncForRequestHandler::PrintSetRequestInTrace - Unrecognized Set request received:"
									<< std::endl << pszNonZippedBuffer;
	}

}
//////////////////////////////////////////////////////////////////////
void CSegmentFuncForRequestHandler::ReadSegmentResponse(CSegment **pResponseSegment, BYTE &bCompressed, const eOtherProcessQueueEntry eDestTask, char** pszResponse, DWORD &dwResponseLen, BYTE printResponse)
{
    CPostXmlHeader postXmlHeader;
    postXmlHeader.DeSerialize(**pResponseSegment);

    dwResponseLen = postXmlHeader.GetLen();
    *pszResponse = new char[dwResponseLen + 1];
    memset(*pszResponse, 0, dwResponseLen + 1);
    (*pResponseSegment)->Get((BYTE*)*pszResponse, dwResponseLen);
    (*pszResponse)[dwResponseLen] = '\0';

    bCompressed = postXmlHeader.GetIsCompressed();

    if (printResponse)
        PrintResponseInTrace(eDestTask, *pszResponse);
}
////////////////////////////////////////////////////////////////////////////
void CSegmentFuncForRequestHandler::PrintResponseInTrace(const eOtherProcessQueueEntry eDestTask, char* pszResponse)
{
	if (eManager == eDestTask) //it means Set request
  {
    BOOL isHidePsw = NO;
    std::string key_hide = "HIDE_CONFERENCE_PASSWORD";
    CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);
    if (!isHidePsw)
    {
      FTRACESTR(eLevelInfoNormal)
          << "CSegmentFuncForRequestHandler::PrintResponseInTrace - Set response received:"
          << std::endl << pszResponse;
    }
  }
  else if (eMonitor == eDestTask)
  {
    if (IsApacheDebugMode())
      FTRACESTR(eLevelInfoNormal)
          << "CSegmentFuncForRequestHandler::PrintResponseInTrace - Get response received:"
          << std::endl << pszResponse;
  }
}
////////////////////////////////////////////////////////////////////////////
void CSegmentFuncForRequestHandler::PrintShadowPassword(char* pszNonZippedBuffer,
														const eOtherProcessQueueEntry eDestTask,
														BOOL isResponse)
{
	if (strstr(pszNonZippedBuffer,"<PASSWORD>") != NULL ||
		strstr(pszNonZippedBuffer,"<ENTRY_PASSWORD>") != NULL ||
		strstr(pszNonZippedBuffer,"<LEADER_PASSWORD>") != NULL)
	{
		ALLOCBUFFER(pszBufferWithShadowPassword, strlen(pszNonZippedBuffer)+30);
		memset(pszBufferWithShadowPassword, 0, strlen(pszNonZippedBuffer)+30);
		ALLOCBUFFER(pszBufferWithShadowPassword2, strlen(pszNonZippedBuffer)+40);
		memset(pszBufferWithShadowPassword2, 0, strlen(pszNonZippedBuffer)+40);
		ALLOCBUFFER(pszBufferWithShadowPassword3, strlen(pszNonZippedBuffer)+50);
		memset(pszBufferWithShadowPassword3, 0, strlen(pszNonZippedBuffer)+50);


		CSegmentFuncForRequestHandler::RemovePasswordFromString(pszNonZippedBuffer, pszBufferWithShadowPassword,
				"<PASSWORD>", "</PASSWORD>");
		CSegmentFuncForRequestHandler::RemovePasswordFromString(pszBufferWithShadowPassword, pszBufferWithShadowPassword2,
				"<ENTRY_PASSWORD>", "</ENTRY_PASSWORD>");
		CSegmentFuncForRequestHandler::RemovePasswordFromString(pszBufferWithShadowPassword2, pszBufferWithShadowPassword3,
				"<LEADER_PASSWORD>", "</LEADER_PASSWORD>");

		/*FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest - Set "

							<< ( isResponse ? "response" : "request" ) << " received: "
							<< std::endl << pszBufferWithShadowPassword3;
		 */
		CSegmentFuncForRequestHandler::PrintResponseInTrace(eDestTask,pszBufferWithShadowPassword3);


		DEALLOCBUFFER(pszBufferWithShadowPassword);
		DEALLOCBUFFER(pszBufferWithShadowPassword2);
		DEALLOCBUFFER(pszBufferWithShadowPassword3);
	}
	else
	{
		CSegmentFuncForRequestHandler::PrintResponseInTrace(eDestTask,pszNonZippedBuffer);
	}
}
