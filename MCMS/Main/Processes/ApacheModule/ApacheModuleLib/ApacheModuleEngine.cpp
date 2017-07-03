// ApacheModuleEngine.cpp

#include <iomanip>
#include <cctype>
#include <algorithm>

#include "http_protocol.h"
#include "ap_compat.h"
#include "ApacheModuleEngine.h"
#include "XmlMiniParser.h"
#include "HeaderPars.h"
#include "TaskApi.h"
#include "OpcodesMcmsCommon.h"
#include "XmlApi.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "http_core.h"
#include "apr_xml.h"
#include "InitCommonStrings.h"
#include "TraceStream.h"
#include "ZipWrapper.h"
#include "ApiStatuses.h"
#include "HlogApi.h"
#include "FaultsDefines.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ObjString.h"
#include "NStream.h"
#include "VirtualDirectory.h"
#include "ApacheStatuses.h"
#include "IConv.h"
#include "UnicodeDefines.h"
#include "EncodingConvertor.h"
#include "PostXmlHeader.h"
#include "OsFileIF.h"
#include "SegmentFuncForRequestHandler.h"
#include "InternalProcessStatuses.h"
#include "http_request.h"
#include "RvgwSslPorts.h"
#include "PrettyTable.h"
#include "test_char.h"
#include "ShelfXmlConvertor.h"

/* we assume the folks using this ensure 0 <= c < 256... which means
 * you need a cast to (unsigned char) first, you can't just plug a
 * char in here and get it to work, because if char is signed then it
 * will first be sign extended.
 */
#define TEST_CHAR(c, f)        (test_char_table[(unsigned)(c)] & (f))

extern char* ProcessTypeToString(eProcessType processType);

CApacheModuleProcess* CApacheModuleEngine::m_pProcess = NULL;
CConnectionList* CApacheModuleEngine::m_pConnectionList = NULL;
CActionRedirectionMap* CApacheModuleEngine::m_pActionRedirectionMap = NULL;
int CApacheModuleEngine::m_nKeepAliveTimeout = 15000;
int CApacheModuleEngine::m_nLastVD = -1;
CVirtualDirectory CApacheModuleEngine::m_VirtualDirectoryList[MAX_NUMBER_OF_VIRTUAL_DIRECTORIES];
std::string CApacheModuleEngine::m_rvgwAliasName ="";
int CApacheModuleEngine::m_Security_Loose_Revocation =0;
eProductType CApacheModuleEngine::m_curProductType = eProductTypeUnknown;

#define RETURN_AUTH_FAILED	if((nConnId == 0) || (m_pConnectionList->GetAuthorization(nConnId) == ANONYMOUS))\
							{\
								ap_note_basic_auth_failure(pRequestRec);\
								return HTTP_UNAUTHORIZED;\
							}\
							else\
								return OK;

#define TRANS_LOGIN				"<TRANS_MCU><TRANS_COMMON_PARAMS><MCU_TOKEN>%i</MCU_TOKEN><MCU_USER_TOKEN>0</MCU_USER_TOKEN></TRANS_COMMON_PARAMS><ACTION><LOGIN><MCU_IP><IP>127.0.0.1</IP><LISTEN_PORT>80</LISTEN_PORT></MCU_IP><USER_NAME>%s</USER_NAME><PASSWORD>%s</PASSWORD><STATION_NAME>%s</STATION_NAME></LOGIN></ACTION></TRANS_MCU>"
#define RESP_TRANS_LOGIN_START	"<RESPONSE_TRANS_MCU>"

#define TRANS_LOGOUT			"<TRANS_MCU><TRANS_COMMON_PARAMS><MCU_TOKEN>11</MCU_TOKEN><MCU_USER_TOKEN>11</MCU_USER_TOKEN><ASYNC><YOUR_TOKEN1>0</YOUR_TOKEN1><YOUR_TOKEN2>0</YOUR_TOKEN2></ASYNC><MESSAGE_ID>365</MESSAGE_ID></TRANS_COMMON_PARAMS><ACTION><LOGOUT/></ACTION></TRANS_MCU>"


#define FW_REQUEST_TO_SHM 600

#define APACHE_MODULE_LOADING_LOG "ApacheModuleLoad.log"



bool CApacheModuleEngine::FillActionRedirectionMap()
{

	if(NULL == m_pActionRedirectionMap)
	{
		m_pActionRedirectionMap = new CActionRedirectionMap;
	}
	else
    {
        m_pActionRedirectionMap->ClearMap();
    }

    bool bRet = true;

	char szCfgFileFullPath[100] = {0};

    bRet = m_pActionRedirectionMap->LoadFromFile();

	return bRet;
}
/////////////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::BasicAuthentication(request_rec *pRequestRec)
{
	if(!pRequestRec)
	{
		FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::BasicAuthentication. pRequestRec = NULL";
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	if(m_pProcess!=NULL && m_pProcess->IsBlockRequests())
			return HTTP_SERVICE_UNAVAILABLE;
	int nConnId=0, nRes;
	const char* pszPasswd;
	bool bSendLogin = false;

	const  char *pLoginLoose = apr_table_get(pRequestRec->notes,"Login-loose-revocation");

	 std::string revMethod = "empty";
	 if(pLoginLoose)
	 {
		    revMethod = pLoginLoose;
	    	if(revMethod.find("ocsp") !=std::string::npos)
	    	{
	    		revMethod ="Failed to use OCSP revocation method";
	    		m_Security_Loose_Revocation = 1;
	    	}
	    	else
	    	{
	    		revMethod ="Failed to use CRL revocation method";
	    		m_Security_Loose_Revocation = 2;
	    	}
	}
	 else
	 {
		 m_Security_Loose_Revocation = 0;
	 }

	nConnId = GetValidConnId(pRequestRec);
	if(!nConnId)
	{
		AddAnonymousConnection(pRequestRec,nConnId);
	}

	if(pRequestRec->method_number == M_GET)
	{
		int bSubRequest = false;
    	apr_pool_userdata_get((void**)&bSubRequest,"SubRequest",pRequestRec->connection->pool);

		if(bSubRequest == true)
			return OK;
	}

	if(pRequestRec->method_number == M_POST)
	{
		apr_pool_userdata_setn((const void*)true,"SubRequest",NULL,pRequestRec->connection->pool);
		return OK;
	}

	m_pConnectionList->SetConnectionAlive(nConnId);

	if((nRes = ap_get_basic_auth_pw(pRequestRec,&pszPasswd)) != OK) {
		RETURN_AUTH_FAILED;
	}

	if(m_pConnectionList->GetAuthorization(nConnId) == ANONYMOUS )
		bSendLogin = true;
	else
	{
		if(pRequestRec->user && strlen(pRequestRec->user) && strcmp(pRequestRec->user,m_pConnectionList->GetLogin(nConnId).c_str()))
			bSendLogin = true;
	}

	FTRACEINTO << "CApacheModuleEngine::BasicAuthentication. File: " << pRequestRec->filename << "; Method: " << pRequestRec->method_number << "; ConnId: " << nConnId << "; Authorization: " << m_pConnectionList->GetAuthorization(nConnId) << "Login-loose-revocation  = " << pLoginLoose << "method: "<< revMethod;

	if(bSendLogin)
	{
		char szLoginRequest[700];
		COsQueue *pMbxSend = NULL, *pMbxRcv = NULL;
		CMessageHeader Header;

		snprintf(szLoginRequest, sizeof(szLoginRequest), TRANS_LOGIN,nConnId ,pRequestRec->user, pszPasswd,pRequestRec->useragent_ip);

		//apr_pool_userdata_get((void**)&pMbxSend,"MbxSend",pRequestRec->connection->pool);
		//apr_pool_userdata_get((void**)&pMbxRcv,"MbxRcv",pRequestRec->connection->pool);
		m_pConnectionList->GetConnectionQueues(nConnId,&pMbxRcv,&pMbxSend);
		Header.m_RspMsgSeqNum = GetNextMsgSeqNum(pRequestRec);
		Header.m_msgType = eSyncMessage;

        ConnectionDetails_S connDetails;
        ExtractConnectionDetails(pRequestRec, connDetails);
        CStringToProcessEntry destProcessEntry(eProcessApacheModule, true, "Authentication", "Apache user authentication succeeded.", "Apache failed to authenticate user.");

		int nSendStatus = SendSyncMessage(pMbxSend,pMbxRcv,szLoginRequest,Header,destProcessEntry,eManager,
										  false, "",
                                          strlen(szLoginRequest),nConnId,pRequestRec->server->port,
                                          connDetails);

		if(nSendStatus == STATUS_OK)
		{
			DWORD dwResponseLen;
			CXmlMiniParser XmlMiniParser;

			CSegment *pResponseSegment = Header.m_segment;

			CPostXmlHeader postXmlHeader;
			postXmlHeader.DeSerialize(*pResponseSegment);

			dwResponseLen = postXmlHeader.GetLen();

			char *pszResponse = new char[dwResponseLen+1];//(char*)ap_pcalloc(pRequestRec->pool, dwResponseLen+1);
			if(!pszResponse)
				return HTTP_INTERNAL_SERVER_ERROR;
			else
			{
				memset(pszResponse, 0, dwResponseLen+1);
				pResponseSegment->Get((BYTE*)pszResponse,dwResponseLen);
				pszResponse[dwResponseLen] = '\0';

				if((dwResponseLen <= 0) || (pszResponse == NULL)) 
				{
					delete[] pszResponse;
					
					RETURN_AUTH_FAILED;
				}

				if(strstr(pszResponse,RESP_TRANS_LOGIN_START) != NULL)
				{
					if(XmlMiniParser.GetResponseStatus(pszResponse) == STATUS_OK)
					{
						/*if(nConnId != 0)
						{
							FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::BasicAuthentication : Connection changed";
							//m_pConnectionList->RemoveConnection(nConnId);
						}*/

						nConnId = XmlMiniParser.GetConnectionId(pszResponse);
						apr_pool_userdata_setn((const void*)nConnId,"ConnId",NULL,pRequestRec->connection->pool);
					}
					else
					{
						FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::BasicAuthentication : Login rejected";
						SendEventToAuditor(pRequestRec, nConnId, "BasicAuthentication", "Client tried to authenticate", "Client failed to authenticate (BasicAuthentication) or did not send authentication information");
						RETURN_AUTH_FAILED;
					}
				}
				else
				{
					FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::BasicAuthentication : Illegal response";
					RETURN_AUTH_FAILED;
				}
				PDELETEA(pszResponse);
			}
		}
		else
		{
			FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::BasicAuthentication : Failed communicating with Auth process";
			RETURN_AUTH_FAILED;
		}
	}

	FTRACEINTO << "CApacheModuleEngine::BasicAuthentication success. ConnId: " << nConnId;

	return OK;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CApacheModuleEngine::GetNextMsgSeqNum(request_rec *pRequestRec)
{
	DWORD msg_seq_num = 0;
	apr_pool_userdata_get((void**)&msg_seq_num,"MsgSeqNum",pRequestRec->connection->pool);

	msg_seq_num++;

	apr_pool_userdata_setn((const void*)msg_seq_num,"MsgSeqNum", NULL, pRequestRec->connection->pool);

	return msg_seq_num;
}

/////////////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::ReadRequestContent(request_rec *pRequestRec,
                                            char **pszRequest, FILE* pFile)
{
	if(!pRequestRec)
	{
		FTRACESTR(eLevelInfoNormal) <<
            "CApacheModuleEngine::ReadRequestContent. No request object";
		return 0;
	}

	if (pFile==NULL && !pszRequest)
	{
		FTRACESTR(eLevelInfoNormal) <<
            "CApacheModuleEngine::ReadRequestContent. No string to read content";
		return 0;
	}

    if (ap_setup_client_block(pRequestRec, REQUEST_CHUNKED_DECHUNK) != OK)
    {
    	FTRACESTR(eLevelInfoNormal) <<
            "CApacheModuleEngine::ReadRequestContent. ap_setup_client_block failed";
        return 0;
    }

	int nPos = 0;

    if (ap_should_client_block(pRequestRec))
	{
		char szContentBlock[HUGE_STRING_LEN];
		int nRead, nCopy;

		long nContentLen = pRequestRec->remaining;
		long bytesWritten=0;
		if(nContentLen)
		{
			if (pFile==NULL)	//if we don't read a file
			{
			   *pszRequest = new char[nContentLen + 1];//(char*)ap_pcalloc(pRequestRec->pool, nContentLen + 1);
			   memset(*pszRequest, 0, nContentLen + 1);
			}

		   	while ((nRead = ap_get_client_block(pRequestRec,
                                                szContentBlock,
                                                sizeof(szContentBlock))) > 0)
		   	{
				if ((nPos + nRead) > nContentLen)
					nCopy = nContentLen - nPos;
				else
					nCopy = nRead;

				if (pFile)	
						//if we read a file
				{
					bytesWritten = (long) fwrite((const void*)szContentBlock,1,nCopy,pFile);
					if(bytesWritten != nCopy)
						return nPos;
				}
				else
					memcpy((char*)*pszRequest + nPos, szContentBlock, nCopy);
				nPos += nCopy;
		   	}
		}
		else if (pszRequest != NULL)
			*pszRequest = NULL;
    }
    else
    	FTRACESTR(eLevelInfoNormal) <<
            "CApacheModuleEngine::ReadRequestContent. ap_should_client_block failed";

    return nPos;
}

//////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::GetValidConnId(request_rec *pRequestRec)
{
	int nConnId=0, nConnIdInUserData=0;

	if(!pRequestRec)
	{
		FTRACESTR(eLevelInfoNormal) <<
            "CApacheModuleEngine::GetValidConnId. pRequestRec = NULL";
		return -1;
	}

	apr_pool_userdata_get((void**)&nConnIdInUserData,"ConnId",pRequestRec->connection->pool);

	if(m_pConnectionList->IsValidConnection(nConnIdInUserData))
		nConnId = nConnIdInUserData;

	return nConnId;
}

STATUS CApacheModuleEngine::SendResponse(request_rec* pRequestRec,
                                         int nConnId,
                                         int nMessageId,
									     char* pszResponse,
                                         int nResponseLen,
									     const char* pszContentType,
                                         BYTE bCompressed)
{
	if (pRequestRec == NULL)
		return STATUS_ILLEGAL_REQUEST;

	CHeaderParams HeaderParams;
	std::vector<unsigned char> buf;
	int nLenForResponse = nResponseLen;
	BYTE bCompressedForResponse = bCompressed;
	const void* pszResponseForSend = pszResponse;

	if (pszResponse != NULL)
	{
		bool bSupportCompression = m_pConnectionList->GetSupportCompression(nConnId);
		if (bCompressed && !bSupportCompression)
		{
		    int rc = CZipWrapper::InflateByChunk(reinterpret_cast<unsigned char*>(pszResponse),
                                                 nResponseLen,
                                                 buf);

		    FPASSERTSTREAM_AND_RETURN_VALUE(Z_OK != rc,
		        "CZipWrapper::InflateByChunk: " << nResponseLen << " bytes: "
		            << CZipWrapper::StrError(rc),
		        STATUS_RESPONSE_TOO_LONG);

		    pszResponseForSend = &buf[0];
		    nLenForResponse = buf.size();
		    bCompressedForResponse = false;

		    FTRACEINTOFUNC << "Inflated original " << nResponseLen
                           << " to " << nLenForResponse << " bytes";
		}

		HeaderParams.m_nContentLength = nLenForResponse;
		HeaderParams.m_bZip = bCompressedForResponse;
	}

	HeaderParams.m_nAuthorization = m_pConnectionList->GetAuthorization(nConnId);
	HeaderParams.m_nMessageId = nMessageId;

	SetupResponseHeader(pRequestRec, HeaderParams);

	if ((HeaderParams.m_nContentLength > 0)	&& (pszContentType != NULL))
		pRequestRec->content_type = pszContentType;
	else
		pRequestRec->content_type = NULL;

	ap_send_http_header(pRequestRec);

	if (HeaderParams.m_nContentLength > 0)
		ap_rwrite(pszResponseForSend, nLenForResponse, pRequestRec);

	//otherwise, if the next message will arrive immediately and cause delay, the answer to the
	//current message can be returned only after the delay
	ap_rflush(pRequestRec);

	return STATUS_OK;
}

void CApacheModuleEngine::SendGeneralResponse(request_rec *pRequestRec,
                                              char* pszRequestContent,
											  int nStatus,
                                              int nConnId,
                                              int nMessageIdInHeader,
                                              const string & charSet)
{
		int nYourToken1, nYourToken2, nMessageId = nMessageIdInHeader;
		char *pszStartActionName, *pszEndActionName, szActionName[50], *pszResponse;
		CXMLDOMElement *pGenResponseElement;
		WORD nAction;

		if(!pRequestRec || !pszRequestContent)
			return;

		nYourToken1 = CXmlMiniParser::GetYourToken1(pszRequestContent);
		nYourToken2 = CXmlMiniParser::GetYourToken2(pszRequestContent);

		szActionName[0] = 0;
		pszStartActionName = CXmlMiniParser::GetActionName(pszRequestContent,
                                                           &pszEndActionName);

		if(pszStartActionName)
		{
			if(strstr(pszStartActionName,"GET") == pszStartActionName)
				nAction = GET_REQUEST;
			else if(pszStartActionName != NULL)
				nAction = SET_REQUEST;
		}
		else
			nAction = UNKNOWN_REQUEST;

		if(pszStartActionName)
		{
			const int maxStrSizeForKW = min((int) sizeof(szActionName) - 1, pszEndActionName-pszStartActionName);
			strncpy(szActionName,pszStartActionName,maxStrSizeForKW);
			szActionName[maxStrSizeForKW] = 0;
		}

		if(nMessageId == -1)
			nMessageId = CXmlMiniParser::GetMessageId(pszRequestContent);

		string strStatus = CProcessBase::GetProcess()->GetStatusAsString(nStatus);
		pGenResponseElement = BuildGeneralResponse( nStatus,
													strStatus.c_str(),
													NULL,
													nMessageId,
												   	nAction,
												   	nYourToken1,
												   	nYourToken2);

		pGenResponseElement->DumpDataAsLongStringEx(&pszResponse);

        string strContentType = "application/xml";
        if(false == charSet.empty())
        {
            strContentType += "; charset=";
            strContentType += charSet;
        }

		SendResponse(pRequestRec,
                     nConnId,
                     nMessageId,
                     pszResponse,
                     strlen(pszResponse),
                     strContentType.c_str(),
                     false);


		DEALLOCBUFFER(pszResponse);

		delete pGenResponseElement;
}

////////////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::HandlePostRequest(request_rec *pRequestRec, int nConnId, BYTE bSHM/*=FALSE*/, BYTE bLogin/*=FALSE*/)
{
	//DEBUG
	//Below trace doing too much noise every 1 msec !!
//	if( pRequestRec->plcm_ssl_client_s_dn == NULL )
//	{
//		FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest. plcm_ssl_client_s_dn is empty";
//	}
//	else
//	{
//		FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest. plcm_ssl_client_s_dn is " << pRequestRec->plcm_ssl_client_s_dn;
//	}

	CHeaderParser HeaderParser;
    bool isZip = HeaderParser.IsZippedContent(pRequestRec);
	int nMessageId = HeaderParser.GetMessageId(pRequestRec);
	int ifNoneMatche = HeaderParser.GetIfNoneHeader(pRequestRec);
    if(CSegmentFuncForRequestHandler::IsReadyToHandleRequests() == STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP)
    {
        FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest. return STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP";
        SendGeneralResponse(pRequestRec,"", STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP, nConnId,nMessageId, "");
        TraceHeader(pRequestRec);
		return OK;
    }

	char *pszRequestContent=NULL, *pszResponse, *pszNonZippedBuffer=NULL, *pszShelfMsgResponse = NULL;
	eOtherProcessQueueEntry eDestTask = eManager;
	COsQueue *pMbxSend = NULL, *pMbxRcv = NULL;
	CMessageHeader Header;
	char *pszStartActionName=NULL;
	bool bSetConnectionUponReturn = false;
	BYTE bCompressed = false;
	int nContentLen = 0;
	string strCharset;
	ConnectionDetails_S connDetails;
    CStringToProcessEntry destProcessEntry(eProcessTypeInvalid, true, "cucu-lulu name", "cucu-lulu description", "cucu-lulu fault description");
    bool bShelfMsg = false;
    unsigned int ulOpcode = 0, ulMsgId = 0;
    int nSlotId = -1;
    int retShelfMsg = STATUS_OK;

    ExtractClientIp(pRequestRec, connDetails.clientIp);

	apr_pool_userdata_setn((const void*)false,"SubRequest",NULL,pRequestRec->connection->pool);

    // Extract encoding type from the HTTP header
    // if no charset is provided no default value will be given.
    // the charset will be defined by comment. it will be done after unzip.
    if (ExtractEncodingFromHttpHeader(HeaderParser, pRequestRec, nConnId, nMessageId, strCharset)!=STATUS_OK)
		return OK;

    if (bSHM)
    {
    	if (bLogin)	//for login request to SHM
    		BuildXmlLoginRequest(pRequestRec, &pszRequestContent);
    	else
    		BuildXmlLogoutRequest(&pszRequestContent);

    	if (pszRequestContent)
    	{
    		nContentLen = strlen(pszRequestContent);
			pszNonZippedBuffer = pszRequestContent;
    	}

		if ((pszRequestContent==NULL) || (nContentLen == 0))
		{
	        FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest. Request content is empty";
			TraceHeader(pRequestRec);
			SendGeneralResponse(pRequestRec,"",STATUS_ILLEGAL_REQUEST,nConnId,nMessageId, strCharset);
			return OK;
		}

    }
    else
    {
		nContentLen = ReadRequestContent(pRequestRec,&pszRequestContent, NULL);
        // SAGI - this is not working - I removed this trace.
		//BOOL isHidePsw = NO;
		//std::string key_hide = "HIDE_CONFERENCE_PASSWORD";
		//CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);
		//if(!isHidePsw )
		//{
		//}

		if ((pszRequestContent==NULL) || (nContentLen == 0))
		{
	            //FPASSERTMSG(1,"CApacheModuleEngine::HandlePostRequest. Request content is empty");
	        	FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest. Request content is empty";
			TraceHeader(pRequestRec);
			SendGeneralResponse(pRequestRec,"",STATUS_ILLEGAL_REQUEST,nConnId,nMessageId, strCharset);
			return OK;
		}

		if (isZip)
		{
	        const int pszNonZippedBufferLen = ( nContentLen*100 /*MAX_NON_ZIPPED*200*/) + 1;
	            pszNonZippedBuffer = new char[pszNonZippedBufferLen];

	            memset(pszNonZippedBuffer, 0, pszNonZippedBufferLen);

			if (DecompressedTheContent(pszNonZippedBuffer, pszNonZippedBufferLen,
	                                   pRequestRec, pszRequestContent, nContentLen,
	                                   bCompressed, nConnId, nMessageId, strCharset) != STATUS_OK)
			{
				PDELETEA(pszRequestContent);
				PDELETEA(pszNonZippedBuffer);
				return OK;
			}
		}
		else
        {
            pszNonZippedBuffer = pszRequestContent;
        }

		// Extract encoding type from the comment <?xml version="1.0" encoding="utf-8"?>
	    // check for conflict with HTTP header
		if (ExtractEncodingFromComment(pszNonZippedBuffer, pRequestRec, nConnId, nMessageId, strCharset)!=STATUS_OK)
		{
			PDELETEA(pszRequestContent);
			if (isZip)
				PDELETEA(pszNonZippedBuffer);

			return OK;
		}
    }

	int ValidationStatus = STATUS_FAIL;
	if(pszNonZippedBuffer)
	{
          if(IsPlatformOfNewArch())
          {
                const  char *pUserAgent = apr_table_get(pRequestRec->headers_in, "user-agent");
                if(pUserAgent && strstr(pUserAgent, "PolycomEmaShelfMgr") > 0)
                {
                    //msg is from 3rd API and ask for Hardware info.
                    //FTRACESTR(eLevelInfoNormal) << "pRequestRec->headers_in - has PolycomEmaShelfMgr : " << pszNonZippedBuffer;
                    
                    if(isZip && (pszRequestContent !=pszNonZippedBuffer)) PDELETEA(pszRequestContent);
                    pszRequestContent = NULL;

                    //if (pszRequestContent)  fix BRIDGE-9563 do not check pszRequestContent. pszRequestContent will point to allocated buffer in CShelfXmlConvertor::StringStripper.
                    	CShelfXmlConvertor::StringStripper(pszNonZippedBuffer, strlen(pszNonZippedBuffer), &pszRequestContent, ulOpcode, ulMsgId, nSlotId);

                	if (pszRequestContent)
                	{
                		nContentLen = strlen(pszRequestContent);
                        PDELETEA(pszNonZippedBuffer);
            			pszNonZippedBuffer = pszRequestContent;
            			isZip = false;
                          if(1 != ulOpcode) FTRACESTR(eLevelInfoNormal) << "Change to EmaShelfMsg : " << pszRequestContent;
                	}

            		if ((pszRequestContent == NULL) || (nContentLen == 0))
            		{
            	             FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest. pszShelfXmlRequestContent content is empty";
            			TraceHeader(pRequestRec);
            			SendGeneralResponse(pRequestRec,"",STATUS_ILLEGAL_REQUEST,nConnId,nMessageId, strCharset);

                        if(pszRequestContent)
                        	PDELETEA(pszRequestContent);
                        if (isZip)
                        	PDELETEA(pszNonZippedBuffer);
            			return OK;
            		}

                    bShelfMsg = true;
                }
                else
                {
                    //normal msg. do nothing.
                }
          }

		ValidationStatus = CSegmentFuncForRequestHandler::ValidateTransaction(m_pActionRedirectionMap,
								pszNonZippedBuffer, pszStartActionName, nConnId, connDetails,
								destProcessEntry, eDestTask, bSetConnectionUponReturn,
								m_pConnectionList->GetAuthorization(nConnId),
								m_pConnectionList->GetLogin(nConnId).c_str());
	}
	else
	{
		FPASSERTMSG(1, "pszNonZippedBuffer is NULL!"); 
		
	}

	//Judith - no need to print all the POST (set and get requests) - SET are enough
	if ((eDestTask == eManager) && pszNonZippedBuffer)
	{
		 int res = m_pConnectionList->GetConnectionDetails(nConnId,connDetails);
		 if(res  != -1)
		{
			 std::string msg = "ClientIP : " + connDetails.clientIp + " ,User : " + connDetails.userName + " ,WorkStation : " + connDetails.workStation;
			 CSegmentFuncForRequestHandler::PrintRequestInTrace(eDestTask, pszStartActionName, pszNonZippedBuffer,msg.c_str());
		}
		else
		  CSegmentFuncForRequestHandler::PrintRequestInTrace(eDestTask, pszStartActionName, pszNonZippedBuffer,NULL);
	}
	if (ValidationStatus!=STATUS_OK)
	{

		if (ValidationStatus == STATUS_ACTION_NODE_IS_MISSING_OR_INVALID || ValidationStatus == STATUS_TRANSACTION_DOES_NOT_EXIST)
			TraceHeader(pRequestRec);

		SendGeneralResponse(pRequestRec,
                            pszNonZippedBuffer,
                            ValidationStatus,
                            nConnId,
                            nMessageId,
                            strCharset);

		PDELETEA(pszRequestContent);
		if (isZip)
			PDELETEA(pszNonZippedBuffer);
		return OK;
	}
	eDestProccessValidity destProcValidity =IsDestProccessValid( destProcessEntry.GetProcessType() );
	if(pDestprocValid == destProcValidity )
	{

		m_pConnectionList->GetConnectionQueues(nConnId,&pMbxRcv,&pMbxSend);
    	//apr_pool_userdata_get((void**)&pMbxSend,"MbxSend",pRequestRec->connection->pool);
		//apr_pool_userdata_get((void**)&pMbxRcv,"MbxRcv",pRequestRec->connection->pool);

		Header.m_RspMsgSeqNum = GetNextMsgSeqNum(pRequestRec);
		Header.m_msgType = eSyncMessage;
		Header.m_ifNoneMatch = ifNoneMatche;

		int nSendStatus = SendSyncMessage(pMbxSend,pMbxRcv,pszRequestContent,
                                          Header,destProcessEntry,eDestTask,
										  bCompressed, strCharset,
                                          nContentLen,nConnId,pRequestRec->server->port,
                                          connDetails, pRequestRec->plcm_ssl_client_s_dn, bSHM);

		bCompressed = false;

     	CSegment *pResponseSegment = NULL;
		if(nSendStatus == STATUS_OK)
		{
			DWORD dwResponseLen;

			pResponseSegment = Header.m_segment;

			CSegmentFuncForRequestHandler::ReadSegmentResponse(&pResponseSegment, bCompressed, eDestTask, &pszResponse, dwResponseLen);
			if(destProcessEntry.GetProcessType() == eProcessApacheModule)
			{
				if(destProcessEntry.GetTransName() != NULL && strcmp(destProcessEntry.GetTransName(),"Logout")==0)
				{
						CConnectionList* pConnList = CApacheModuleEngine::GetConnectionList();
						pConnList->RemoveConnection(nConnId);
				}
			}


 			if (bSHM)	//for SHM login\logout responses. If the status is STAUS_OK -we pass the request to the SHM, as it is
 			{			//otherwise - we return the response to the client in the regular way.
				//if(CXmlMiniParser::GetResponseStatus(pszResponse) == STATUS_OK)
 				int status = CXmlMiniParser::GetResponseStatus(pszResponse);

 				if (status == STATUS_OK)
 				{
 					PDELETEA(pszResponse);
 					PDELETEA(pszRequestContent);
 					if (isZip)
 						PDELETEA(pszNonZippedBuffer);
					PDELETE(pResponseSegment);
 					return FW_REQUEST_TO_SHM;
 				}
				else
				{
					FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest : MCMS response to SM request";

					ostringstream strShelfResponse;
					strShelfResponse <<  "Control$Opcode=";
					strShelfResponse <<	((bLogin) ? "1" : "2");
					strShelfResponse <<	"$MsgID=" << nMessageId;
					strShelfResponse <<	"$Status=" << status;
					strShelfResponse <<	"$Description=" << CProcessBase::GetProcess()->GetStatusAsString(status);

					int len = strShelfResponse.str().size();
					PDELETEA(pszResponse);
					pszResponse = new char[len + 1];
					memset(pszResponse, 0, len + 1);
					strcpy(pszResponse, strShelfResponse.str().c_str());
				}
			}
        	     if(bShelfMsg)
                 {
                        //The response to XMA should be in string format instead of xml
                        pszShelfMsgResponse = NULL;
                        retShelfMsg = CShelfXmlConvertor::StringWrapper(pszResponse, &pszShelfMsgResponse, ulOpcode, ulMsgId, nSlotId);
                        if(NULL != pszShelfMsgResponse)
                        {
                            	PDELETEA(pszResponse);
                            	pszResponse = pszShelfMsgResponse;                            
                        }
                        FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest : MCMS response shelf msg: \n" << pszResponse;
                  }

			int nLen = dwResponseLen;

			if(!bCompressed)
				nLen = strlen(pszResponse);

            string strContentType = "application/xml";
            if(false == strCharset.empty())
            {
                strContentType += "; charset=";
                strContentType += strCharset;
            }

            if (bSHM || bShelfMsg)
            	strContentType = "application/text";

			STATUS bSendResponseStatus = SendResponse(pRequestRec, nConnId, nMessageId, pszResponse, nLen,
                         							  strContentType.c_str(), bCompressed);

			if (bSendResponseStatus != STATUS_OK)
			{
				SendGeneralResponse(pRequestRec,
	                                pszNonZippedBuffer,
	                                bSendResponseStatus,
	                                nConnId,
	                                nMessageId,
	                                strCharset);
			}

			PDELETE(pResponseSegment);
			PDELETEA(pszResponse);
		}
		else
		{
            if(STATUS_INVALID_INPUT == nSendStatus)
            {
                nSendStatus = STATUS_ILLEGAL_REQUEST;
            }
            else if(STATUS_REQUEST_TOO_LONG == nSendStatus)
            {
            }
            else
            {
                nSendStatus = STATUS_INTERNAL_COMMUNICATION_PROBLEM;
            }

			SendGeneralResponse(pRequestRec,
                                pszNonZippedBuffer,
                                nSendStatus,
                                nConnId,
                                nMessageId,
                                strCharset);

			string errStr = "CApacheModuleEngine::HandlePostRequest: Failed sending request to ";
			errStr += ProcessTypeToString(destProcessEntry.GetProcessType());
			errStr += " process";

			FPASSERTMSG(1, errStr.c_str());

			if (nSendStatus != STATUS_REQUEST_TOO_LONG)
			{
			    // screen a password in JITC mode
			    std::string buf = pszNonZippedBuffer;
			    if (CApacheModuleProcess::IsFederalOn())
			    {
			        const char pat1[] = "<PASSWORD>";
			        const char pat2[] = "</PASSWORD>";
			        size_t beg = buf.find(pat1);
			        size_t end = buf.find(pat2);
			        if(std::string::npos != beg &&
			           std::string::npos != end)
			        {
			            beg += sizeof pat1 / sizeof *pat1 - 1;
			            for (size_t i = beg; i < end; ++i)
			                buf[i] = '*';
			        }
			    }

			    FTRACEINTO << errStr << ":\n" << buf;
			}
		}
	}
	else
	{
		STATUS xmlStatus = STATUS_ACTION_NODE_IS_MISSING_OR_INVALID;
		if(pDestprocInValidUnderJitc == destProcValidity )
		{
			xmlStatus = STATUS_ILLEGAL_REQUEST;
		}
		SendGeneralResponse(pRequestRec,pszNonZippedBuffer,xmlStatus,nConnId,nMessageId, strCharset);
		if (pDestprocInValid == destProcValidity )
		{
			FTRACEINTO << "CApacheModuleEngine::HandlePostRequest: Action Name invalid: " << pszNonZippedBuffer;
			TraceHeader(pRequestRec);
		}
	}

    if(retShelfMsg == STATUS_CONTROL_LOGIN_INVALID && nConnId != 0)
	{
	     FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandlePostRequest : ShelfMsg Login invalid. Close Connection";
	     m_pConnectionList->RemoveConnection(nConnId);
	}

	PDELETEA(pszRequestContent);
	if (isZip)
		PDELETEA(pszNonZippedBuffer);

	return OK;
}

int CApacheModuleEngine::HandlePutRequest(request_rec* pRequestRec,
                                          int nConnId,
                                          int nAuthorization)
{
  std::string strURI;
  CHeaderParser HeaderParser;
  int nMessageId = HeaderParser.GetMessageId(pRequestRec);
  HeaderParser.GetURIInHeader(pRequestRec->the_request, strURI);

  char pstrURI[4096];
  FPASSERTSTREAM_AND_RETURN_VALUE(strURI.length() > ARRAYSIZE(pstrURI) - 1,
      "Unsupported length of request " << strURI.length(),
      HTTP_BAD_REQUEST);


  strncpy(pstrURI, strURI.c_str(), ARRAYSIZE(pstrURI) - 1);
  pstrURI[ARRAYSIZE(pstrURI) - 1] = '\0';

  int bUnescaped = ap_unescape_url(pstrURI);
  COstrStream errorString;
  if (bUnescaped == OK)
  {
    bool isAscii = true;
    if (isAscii)
    {
      strURI = pstrURI;

      BOOL allowed_ext = IsExtensionAllowed(strURI);
      if (!strURI.length())
        pRequestRec->status = HTTP_NOT_FOUND;

      std::string strPhysicalPath;
      if(IsRmxSimulation())
      {
    	  int ind = strURI.find("http://");
    	  if(ind != -1)
    	  {
    		  int end = strURI.find_first_of("/",strlen("http://"));
    		  if(end != -1)
    			  strURI = strURI.substr(end);
    	  }
      }
      if (!GetPhysicalPath(strURI, strPhysicalPath))
        pRequestRec->status = HTTP_NOT_FOUND;

      int nLastSlashPos = strPhysicalPath.find_last_of('/');

      if (nLastSlashPos < 0)
        pRequestRec->status = HTTP_NOT_FOUND;

      if(pRequestRec->status != HTTP_NOT_FOUND)
      {
      std::string strDir = strPhysicalPath.substr(0, nLastSlashPos);
      std::string strFile = strPhysicalPath.substr(nLastSlashPos);
      std::replace(strFile.begin(), strFile.end(), ' ', '_');
      strPhysicalPath = strDir + strFile;

      if (!ap_is_directory(pRequestRec->pool, strDir.c_str()))
        pRequestRec->status = HTTP_NOT_FOUND;
      }
      if (pRequestRec->status != HTTP_NOT_FOUND)
      {
        // find the permission for this virtual directory
        int permission = GetVirtualDirectoryPermission((char*)strPhysicalPath.c_str());

        if (nAuthorization > permission || allowed_ext == FALSE)
        {
          pRequestRec->status = HTTP_FORBIDDEN;
          FTRACEINTOFUNC << "Permission denied. File: "
                         << pRequestRec->filename << "; ConnId: " << nConnId
                         << "; Permission required: " << permission
                         << "; Authorization: " << nAuthorization;
        }
        else
        {
          // add anonymous connection
         // if (nConnId == 0)
          //  AddAnonymousConnection(pRequestRec, nConnId);

          int headerContenLen = HeaderParser.GetContentLength(pRequestRec);
          FTRACEINTOFUNC << "HeaderContenLen = " << headerContenLen;

          std::string strTempPath = "";
          strTempPath += strPhysicalPath;
          strTempPath += "_tmp";

          FILE* pFile = fopen(strTempPath.c_str(), "wb");
          if (pFile != NULL)
          {
            int nContentLen = ReadRequestContent(pRequestRec, NULL, pFile);
            fclose(pFile);

            if ((nContentLen == 0) || (headerContenLen != nContentLen))
            {
              pRequestRec->status = HTTP_BAD_REQUEST;
              FTRACEINTOFUNC << "Failed reading content to file. File: "
                             << strPhysicalPath;
              unlink(strTempPath.c_str());
            }
            else
            {
              int status = rename(strTempPath.c_str(), strPhysicalPath.c_str());
              if (status != 0)
              {
                pRequestRec->status = HTTP_FORBIDDEN;
                FTRACEINTOFUNC << "Failed rename the file. File: "
                               << strPhysicalPath;
                unlink(strTempPath.c_str());
              }
              else
              {
                FTRACEINTOFUNC << "PUT file: " << strPhysicalPath
                               << "  Size:" << nContentLen;

                if (IsCertFile(strURI.c_str()))
                {
                  pRequestRec->status =
                    SendSyncMessageToCertMngr(pRequestRec,strPhysicalPath.c_str());
                }
                else
                  pRequestRec->status = HTTP_OK;

                FTRACEINTOFUNC << "PUT file: " << strPhysicalPath
                               << "  Size:" << nContentLen;
              }
            }

            SyncMedia(FALSE);
          }
          else
          {
            pRequestRec->status = HTTP_FORBIDDEN;
            FTRACEINTOFUNC << "Failed to open the file for writing. File: "
                           << strTempPath.c_str();
          }
        }
      }
      else
      {
        FTRACEINTOFUNC << "Invalid path in header. Header: "
                       << pRequestRec->the_request;
      }
    }
    else
    {
      FTRACEINTOFUNC << "Path not in ASCII. Header: " << pstrURI;
      pRequestRec->status = HTTP_BAD_REQUEST;
    }
  }
  else
  {
    pRequestRec->status = HTTP_BAD_REQUEST;
    FTRACEINTOFUNC << "Failed unescaping the URL. Header: "
                   << pRequestRec->the_request;
  }

  SendEventToAuditor(pRequestRec, nConnId, "Put file",
                     "The RMX Web Client sent a file to the RMX",
                     "The RMX Web Client failed to sent a file to the RMX");

  SendResponse(pRequestRec, nConnId, nMessageId, NULL, 0, NULL, false);

  return OK;
}
void  CApacheModuleEngine::SendEventToAuditorThreadSafe(CFreeData& freeData,AUDIT_EVENT_HEADER_S& outAuditHdr)
{
	if(m_pAccessMutexEngine)
		apr_thread_mutex_lock(m_pAccessMutexEngine);

	CAuditorApi api;
	api.SendEventMcms(outAuditHdr, freeData);

	if(m_pAccessMutexEngine)
		apr_thread_mutex_unlock(m_pAccessMutexEngine);

}

void CApacheModuleEngine::SendEventToAuditor(request_rec *pRequestRec,
											 int nConnId,
                                             const string & action,
                                             const string & description,
                                             const string & failure_description)
{
    ConnectionDetails_S connDetails;
    if(-1 == m_pConnectionList->GetConnectionDetails(nConnId, connDetails))
    {
        // not found
    }
    ExtractClientIp(pRequestRec, connDetails.clientIp);

    eAuditEventStatus status = (HTTP_OK == pRequestRec->status
                                ?
                                eAuditEventStatusOk : eAuditEventStatusFail);
    string statusDesc;
    GetHttpStatusDescription(pRequestRec->status, statusDesc);

    CHeaderParser HeaderParser;
    string strURI;
    HeaderParser.GetURIInHeader(pRequestRec->the_request,strURI);
    string descEx = "File name: "+ strURI;

    AUDIT_EVENT_HEADER_S outAuditHdr;
    CAuditorApi::PrepareAuditHeader(outAuditHdr,
                                   connDetails.userName,
                                   eMcms,
                                   connDetails.workStation,
                                   connDetails.clientIp,
                                   eAuditEventTypeHttp,
                                   status,
                                   action,
                                   description,
                                   failure_description,
                                   descEx);
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "Target file name",
                                 eFreeDataTypeText,
                                 strURI,
                                 "",
                                 eFreeDataTypeText,
                                 "");

    SendEventToAuditorThreadSafe(freeData,outAuditHdr);
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::GetHttpStatusDescription(int httpStatus, string & outStatusDesc)
{
#define STATUS_TO_STR(code) #code

    const char *desc = NULL;

    switch(httpStatus)
    {
        case HTTP_CONTINUE                      : desc = STATUS_TO_STR(HTTP_CONTINUE); break;
        case HTTP_SWITCHING_PROTOCOLS           : desc = STATUS_TO_STR(HTTP_SWITCHING_PROTOCOLS); break;
        case HTTP_PROCESSING                    : desc = STATUS_TO_STR(HTTP_PROCESSING); break;
        case HTTP_OK                            : desc = STATUS_TO_STR(HTTP_OK); break;
        case HTTP_CREATED                       : desc = STATUS_TO_STR(HTTP_CREATED); break;
        case HTTP_ACCEPTED                      : desc = STATUS_TO_STR(HTTP_ACCEPTED); break;
        case HTTP_NON_AUTHORITATIVE             : desc = STATUS_TO_STR(HTTP_NON_AUTHORITATIVE); break;
        case HTTP_NO_CONTENT                    : desc = STATUS_TO_STR(HTTP_NO_CONTENT); break;
        case HTTP_RESET_CONTENT                 : desc = STATUS_TO_STR(HTTP_RESET_CONTENT); break;
        case HTTP_PARTIAL_CONTENT               : desc = STATUS_TO_STR(HTTP_PARTIAL_CONTENT); break;
        case HTTP_MULTI_STATUS                  : desc = STATUS_TO_STR(HTTP_MULTI_STATUS); break;
        case HTTP_MULTIPLE_CHOICES              : desc = STATUS_TO_STR(HTTP_MULTIPLE_CHOICES); break;
        case HTTP_MOVED_PERMANENTLY             : desc = STATUS_TO_STR(HTTP_MOVED_PERMANENTLY); break;
        case HTTP_MOVED_TEMPORARILY             : desc = STATUS_TO_STR(HTTP_MOVED_TEMPORARILY); break;
        case HTTP_SEE_OTHER                     : desc = STATUS_TO_STR(HTTP_SEE_OTHER); break;
        case HTTP_NOT_MODIFIED                  : desc = STATUS_TO_STR(HTTP_NOT_MODIFIED); break;
        case HTTP_USE_PROXY                     : desc = STATUS_TO_STR(HTTP_USE_PROXY); break;
        case HTTP_TEMPORARY_REDIRECT            : desc = STATUS_TO_STR(HTTP_TEMPORARY_REDIRECT); break;
        case HTTP_BAD_REQUEST                   : desc = STATUS_TO_STR(HTTP_TEMPORARY_REDIRECT); break;
        case HTTP_UNAUTHORIZED                  : desc = STATUS_TO_STR(HTTP_BAD_REQUEST); break;
        case HTTP_PAYMENT_REQUIRED              : desc = STATUS_TO_STR(HTTP_PAYMENT_REQUIRED); break;
        case HTTP_FORBIDDEN                     : desc = STATUS_TO_STR(HTTP_FORBIDDEN); break;
        case HTTP_NOT_FOUND                     : desc = STATUS_TO_STR(HTTP_NOT_FOUND); break;
        case HTTP_METHOD_NOT_ALLOWED            : desc = STATUS_TO_STR(HTTP_METHOD_NOT_ALLOWED); break;
        case HTTP_NOT_ACCEPTABLE                : desc = STATUS_TO_STR(HTTP_NOT_ACCEPTABLE); break;
        case HTTP_PROXY_AUTHENTICATION_REQUIRED : desc = STATUS_TO_STR(HTTP_PROXY_AUTHENTICATION_REQUIRED); break;
        case HTTP_REQUEST_TIME_OUT              : desc = STATUS_TO_STR(HTTP_REQUEST_TIME_OUT); break;
        case HTTP_CONFLICT                      : desc = STATUS_TO_STR(HTTP_CONFLICT); break;
        case HTTP_GONE                          : desc = STATUS_TO_STR(HTTP_GONE); break;
        case HTTP_LENGTH_REQUIRED               : desc = STATUS_TO_STR(HTTP_LENGTH_REQUIRED); break;
        case HTTP_PRECONDITION_FAILED           : desc = STATUS_TO_STR(HTTP_PRECONDITION_FAILED); break;
        case HTTP_REQUEST_ENTITY_TOO_LARGE      : desc = STATUS_TO_STR(HTTP_REQUEST_ENTITY_TOO_LARGE); break;
        case HTTP_REQUEST_URI_TOO_LARGE         : desc = STATUS_TO_STR(HTTP_REQUEST_URI_TOO_LARGE); break;
        case HTTP_UNSUPPORTED_MEDIA_TYPE        : desc = STATUS_TO_STR(HTTP_UNSUPPORTED_MEDIA_TYPE); break;
        case HTTP_RANGE_NOT_SATISFIABLE         : desc = STATUS_TO_STR(HTTP_RANGE_NOT_SATISFIABLE); break;
        case HTTP_EXPECTATION_FAILED            : desc = STATUS_TO_STR(HTTP_EXPECTATION_FAILED); break;
        case HTTP_UNPROCESSABLE_ENTITY          : desc = STATUS_TO_STR(HTTP_UNPROCESSABLE_ENTITY); break;
        case HTTP_LOCKED                        : desc = STATUS_TO_STR(HTTP_LOCKED); break;
        case HTTP_FAILED_DEPENDENCY             : desc = STATUS_TO_STR(HTTP_FAILED_DEPENDENCY); break;
        case HTTP_UPGRADE_REQUIRED              : desc = STATUS_TO_STR(HTTP_UPGRADE_REQUIRED); break;
        case HTTP_INTERNAL_SERVER_ERROR         : desc = STATUS_TO_STR(HTTP_INTERNAL_SERVER_ERROR); break;
        case HTTP_NOT_IMPLEMENTED               : desc = STATUS_TO_STR(HTTP_NOT_IMPLEMENTED); break;
        case HTTP_BAD_GATEWAY                   : desc = STATUS_TO_STR(HTTP_BAD_GATEWAY); break;
        case HTTP_SERVICE_UNAVAILABLE           : desc = STATUS_TO_STR(HTTP_SERVICE_UNAVAILABLE); break;
        case HTTP_GATEWAY_TIME_OUT              : desc = STATUS_TO_STR(HTTP_GATEWAY_TIME_OUT); break;
        case HTTP_VERSION_NOT_SUPPORTED         : desc = STATUS_TO_STR(HTTP_VERSION_NOT_SUPPORTED); break;
        case HTTP_VARIANT_ALSO_VARIES           : desc = STATUS_TO_STR(HTTP_VARIANT_ALSO_VARIES); break;
        case HTTP_INSUFFICIENT_STORAGE          : desc = STATUS_TO_STR(HTTP_INSUFFICIENT_STORAGE); break;
        case HTTP_NOT_EXTENDED                  : desc = STATUS_TO_STR(HTTP_NOT_EXTENDED); break;

        default:
            desc = "invalid status, description was not found"; break;
            break;
    }
    outStatusDesc = desc;
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::ExtractClientIp(request_rec *pRequestRec, string & outClientIp)
{
//     char ipBuffer[64];
//     SystemDWORDToIpString(htonl(pRequestRec->connection->remote_addr->sa.sin.sin_addr.s_addr),
//                           ipBuffer);
//    outClientIp = ipBuffer;

    outClientIp = pRequestRec->useragent_ip;
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::ExtractConnectionDetails(request_rec *pRequestRec, ConnectionDetails_S & outConnectionDetails)
{
    outConnectionDetails.clientIp = pRequestRec->useragent_ip;
    outConnectionDetails.workStation = pRequestRec->useragent_ip;
    outConnectionDetails.userName = pRequestRec->user;
}

char * CApacheModuleEngine::my_escape_urlencoded_buffer(char *copy, const char *buffer)
{
    const unsigned char *s = (const unsigned char *)buffer;
    unsigned char *d = (unsigned char *)copy;
    unsigned c;

    while ((c = *s)) {
        if (TEST_CHAR(c, T_ESCAPE_URLENCODED)) {
            d = c2x(c, '%', d);
        }
        else if (c == ' ') {
            *d++ = '+';
        }
        else {
            *d++ = c;
        }
        ++s;
    }
    *d = '\0';
    return copy;
}
BOOL CApacheModuleEngine::AllowedForAdminstratorReadonly(int nAuthorization, const char* fileName)
{
	if (nAuthorization != ADMINISTRATOR_READONLY)
	{
		return false;
	}
	std::string fname = MCU_MCMS_DIR+"/Backup/";
	if( strstr(fileName, fname.c_str()))
	{
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
bool CApacheModuleEngine::CheckSecurityToken(const char *pSecurityToken,request_rec *pRequestRec)
{

    std::string clientIp;
    std::string v35GateWayUser;
    std::string v35GateWayPassword;
    TICKS v35Securityticket;

    bool isConExist = m_pConnectionList->GetConnectionV35Details(pSecurityToken, clientIp, v35GateWayUser, v35GateWayPassword, v35Securityticket);

	 if(isConExist)
	 {
		 		 
		 TICKS CurTime = SystemGetTickCount();
		 TICKS AbsTicketTime = v35Securityticket + SECURITY_TOKEN_REQUEST_TIME_OUT;
		 if(AbsTicketTime > CurTime)
		 {
			 FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckSecurityToken - valid security token";
			 if(strcmp(clientIp.c_str(), pRequestRec->connection->client_ip)==0)
			 {				 				 
				 FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckSecurityToken - IP's are equal create url and redirect it";
				 // encode characters outside the ASCII set for user name and password
				 char UserBuffer[120]={0};
				 char PassBuffer[120]={0};				 
				 std:string params = v35GateWayUser +std::string("&rvp=") + v35GateWayPassword;
				 params = my_escape_urlencoded_buffer(UserBuffer, v35GateWayUser.c_str()) +
						 std::string("&rvp=") +my_escape_urlencoded_buffer(PassBuffer, v35GateWayPassword.c_str());
				 // create url and redirect it 
				 std::string url =pRequestRec->uri;				 				 
				 url += std::string("/goform/loginCgi?rvu=") + params;				 
				 ap_internal_redirect(url.c_str(), pRequestRec);
				 return true;	 
			 }					 
			 FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckSecurityToken - error connection Client IP is notequel to request ";
			 return false;	 
		 }
		 FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckSecurityToken - error security token is Invalid";
		 return false;
	 }
	 else
	 {
		 FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckSecurityToken - error no connection was found";
		 return false;
	 }
	 
}
//////////////////////////////////////////////////////////////////////////////////////////////
bool CApacheModuleEngine::CheckLogOutSecurityToken(const char *pSecurityToken,request_rec *pRequestRec)
{
    std::string clientIp;
    std::string v35GateWayUser;
    std::string v35GateWayPassword;
    TICKS v35Securityticket;

    bool isConExist = m_pConnectionList->GetConnectionV35Details(pSecurityToken, clientIp, v35GateWayUser, v35GateWayPassword, v35Securityticket);


	 if(isConExist)
	 {
			 if(strcmp(clientIp.c_str(), pRequestRec->connection->client_ip)==0)
			 {				 				 
				 FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckLogOutSecurityToken - IP's are equal create url and redirect it";
				 // encode characters outside the ASCII set for user name and password
				 char UserBuffer[120]={0};				 			
				 std:string params = v35GateWayUser;
				 params = my_escape_urlencoded_buffer(UserBuffer, v35GateWayUser.c_str());
				 // create url and redirect it 
				 std::string url =pRequestRec->uri;				 				 
				 url += std::string("/goform/logoutCgi?rvu=") + params;				 
				 ap_internal_redirect(url.c_str(), pRequestRec);
				 return true;	 
			 }					 
			 FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckLogOutSecurityToken - error connection Client IP is notequel to request ";
			 return false;	 		 
	 }
	 else
	 {
		 FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckLogOutSecurityToken - error no connection was found";
		 return false;
	 }
	 
}
BOOL CApacheModuleEngine::RestrictedForAdminstratorReadonly(int nAuthorization, const char* fileName)
{
	if (nAuthorization != ADMINISTRATOR_READONLY)
	{
		return false;
	}
	return false;
}

bool CApacheModuleEngine::CheckIfRestAPIRequest(request_rec *pRequestRec)
{
	if(pRequestRec->uri == NULL)
		return false;

	if ( strstr(pRequestRec->uri, "/plcm/mcu/api") || strstr(pRequestRec->uri, "/api/rest"))
	{
		return true;
	}
	return false;

}

bool CApacheModuleEngine::CheckIfRvgwRequest(request_rec *pRequestRec)
{
	// this happens when  scripts are runned.
	if(pRequestRec->parsed_uri.port_str == NULL)
		return false;
	//check if it is for poxy
	for(int i=0;i<MAX_RVGW_PORTS;i++)
	{
		if ( strstr(pRequestRec->uri,rvgwAliasNames[i]))
		{
					if(IsRvgwUseActiveX())
						return true;
					else
					{
						char *pStr;	
						if(pRequestRec->args !=NULL)
						{
							if ((pStr=strstr(pRequestRec->args, "LogOutSecurityToken")))
							{
								pStr  += strlen("LogOutSecurityToken=");
								std::string  sToken(pStr);							
								FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckIfRvgwRequest - Logout security token Id ="<<sToken;
								return CheckLogOutSecurityToken(sToken.c_str(),pRequestRec);	
							}
							if ((pStr=strstr(pRequestRec->args, "SecurityToken")))
							{
								pStr  += strlen("SecurityToken=");
								std::string  sToken(pStr);							
								FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckIfRvgwRequest - security token Id ="<<sToken;
								return CheckSecurityToken(sToken.c_str(),pRequestRec);	
							}
							
							
						}
						
						return true;
					}
		}
	}
	if(IsRvgwSslPorts())
	{
		for(int i=0;i<MAX_RVGW_PORTS;i++)
		{
			if(strcmp(rvgwSslPorts[i],pRequestRec->parsed_uri.port_str)==0)
			{
			    std::string url = std::string("/") + std::string(rvgwAliasNames[i]) ;//+std::string("/");
				url +=pRequestRec->uri;
				if(pRequestRec->args !=NULL)
				{
					url += std::string("?") +std::string(pRequestRec->args);
				}
				FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckIfRvgwRequest - redirect to" << url.c_str();
				ap_internal_redirect(url.c_str(), pRequestRec);
				return true;
			}

		}
		return false;
	}
	else
	{
		bool isRvgw = false;
		if(m_rvgwAliasName =="")
			return false;
		if ( strstr(pRequestRec->uri, m_rvgwAliasName.c_str()))
			return true;

		if ( strstr(pRequestRec->uri, "/plcm/mcu/api") || strstr(pRequestRec->uri, "/api/rest"))
		{			
			isRvgw = true;	
		}

		if ( strstr(pRequestRec->uri, "goform"))
		{
			isRvgw = true;
		}
		if ( strstr(pRequestRec->uri, "admin"))
		{
			isRvgw = true;
		}
		if ( strstr(pRequestRec->uri, "js"))
		{
			isRvgw = true;
		}
		if ( strstr(pRequestRec->uri, "Images"))
		{
			isRvgw = true;
		}
		if ( strstr(pRequestRec->uri, "css"))
		{
			isRvgw = true;
		}
		if(isRvgw)
		{
			std::string url = std::string("/") + m_rvgwAliasName;
			url +=pRequestRec->uri;
			if(pRequestRec->args !=NULL)
			{
					url += std::string("?") +std::string(pRequestRec->args);
			}
			FTRACESTR(eLevelInfoNormal) <<"CApacheModuleEngine::CheckIfRvgwRequest - redirect to" << url.c_str();
			ap_internal_redirect(url.c_str(), pRequestRec);
			return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::HandleGetRequest(request_rec *pRequestRec, int nConnId, int nAuthorization)
{
	//find the permission for this virtual directory

	CHeaderParams HeaderParams;
	CHeaderParser HeaderParser;

	HeaderParams.m_nMessageId = HeaderParser.GetMessageId(pRequestRec);
	HeaderParams.m_nAuthorization = m_pConnectionList->GetAuthorization(nConnId);

	SetupResponseHeader(pRequestRec,HeaderParams,true);

    pRequestRec->status = HTTP_OK;


    if(CheckIfRvgwRequest(pRequestRec))
    {
    	return DECLINED;
    }
	if(strstr(pRequestRec->the_request,"EMACfg/") != NULL)
		pRequestRec->content_type = NULL;

	if (!IsFileExists(pRequestRec->filename))
	{
		pRequestRec->status = HTTP_NOT_FOUND;
		FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandleGetRequest: File " << pRequestRec->filename << " not found";
		SendEventToAuditor(pRequestRec, nConnId, "Get file", "The RMX Web Client requested a file to the RMX", "The RMX Web Client failed - file not found");
	}
	else
    {
        int permission = GetVirtualDirectoryPermission(pRequestRec->filename);

        if((nAuthorization > permission && !AllowedForAdminstratorReadonly(nAuthorization, pRequestRec->filename)) ||  RestrictedForAdminstratorReadonly(nAuthorization, pRequestRec->filename))
        {
            pRequestRec->status = HTTP_FORBIDDEN;
            FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandleGetRequest: permission denied. File: " << pRequestRec->filename << "; ConnId: " << nConnId <<  "; Permission required: " << permission << "; Authorization: " << nAuthorization;
            SendEventToAuditor(pRequestRec, nConnId, "Get file", "The RMX Web Client requested a file to the RMX", "The RMX Web Client failed - permission denied, Authorization failed");
        }
        else
        {
          //  if (nConnId == 0)
            //    AddAnonymousConnection(pRequestRec, nConnId);

            int bSubRequest = false;
            apr_pool_userdata_get((void**)&bSubRequest,"SubRequest",pRequestRec->connection->pool);
            //FTRACEINTO << "CApacheModuleEngine::HandleGetRequest: subrequest = " << bSubRequest;

            apr_table_set(pRequestRec->headers_out,"Cache-Control","max-age=0");

            if(!IsFileModified(pRequestRec, pRequestRec->filename))
            {
                pRequestRec->status = HTTP_NOT_MODIFIED;
                //FTRACEINTO << "\nCApacheModuleEngine::HandleGetRequest: " << pRequestRec->filename << " was not modified";
            }
        }
    }

    //int virtual_directory_pos = GetVirtualDirectoryPos(pRequestRec->filename);
   // if (virtual_directory_pos != -1)
    //{

        //eProcessType process_type = m_VirtualDirectoryList[virtual_directory_pos].GetInformProcess();
        //if (process_type != eProcessTypeInvalid)
        //{
        //    CManagerApi api(process_type);
        //    api.InformHttpGetRequest(std::string(pRequestRec->filename));
        // }

//        if(m_VirtualDirectoryList[virtual_directory_pos].GetAuditability())
//        {

            // no GETs for auditor in version 3. 7/1/08
//            SendEventToAuditor(pRequestRec, nConnId, "Get file", "The RMX Web Client retrieved a file from the RMX");
//        }
  //  }

    // DECLINED - Ok, apache will proceed with the request.
    // OK - Bad, apache will not proceed with the request
    int ret = (HTTP_OK == pRequestRec->status
               ?
               DECLINED : OK);

    if (HTTP_OK == pRequestRec->status)
    {
    	SendEventToAuditor(pRequestRec, nConnId, "Get file", "The RMX Web Client requested a file to the RMX", "File retrieval succeeded");
    }
    else
    {
        FTRACESTR(eLevelInfoNormal) << "The RMX Web Client requested a file to the RMX, File retrieval failed";        
    	SendEventToAuditor(pRequestRec, nConnId, "Get file", "The RMX Web Client requested a file to the RMX", "File retrieval failed");
    }

//    FTRACEINTO << "\nCApacheModuleEngine::HandleGetRequest: " << pRequestRec->filename << " ; return " << (DECLINED == ret ? "DECLINED" : "OK");

	return ret;
}

//////////////////////////////////////////////////////////////////////
BOOL CApacheModuleEngine::IsFileModified(request_rec *pRequestRec, const string & filename)
{
    CHeaderParser HeaderParser;
    time_t httpTime = -1;
    if(!HeaderParser.GetLastModify(pRequestRec, httpTime))
    {
        return TRUE;
    }

    time_t fileTime = GetLastModified(filename);
    if(-1 == fileTime)
    {
        FPASSERTMSG(TRUE, "Failed to get file last modified time ");
        return TRUE;
    }

    return (fileTime > httpTime ? TRUE : FALSE);
}

//////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::GetVirtualDirectoryPos(char* file_name)
{
	int virtual_directory_pos = -1;
	char* last_str = NULL, *strVirtualPath = NULL;

	if(!file_name)
		return -1;

	//check if the file name has permission in the virtual directory list
	for (int i=0; i<= m_nLastVD; i++)
	{
	  char* str = NULL;
		char *pPhysicalPath = m_VirtualDirectoryList[i].GetPhysicalPath();
		if(pPhysicalPath !=NULL)
		{
			str = strstr(file_name,pPhysicalPath);
		}
		else
		{
			FTRACEINTO << "CApacheModuleEngine::GetVirtualDirectoryPos " << file_name
                 << " GetPhysicalPath couldn't be found";
		}

		if (str)
		{
			if (last_str==NULL || str < last_str)	//for first time found or when position is before the previous virtual directory that we found
			{
				last_str = str;
				virtual_directory_pos = i;
			}
		}
	}

	return virtual_directory_pos;
}
//////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::GetVirtualDirectoryPermission(char* file_name)
{
	bool isLegalLinkedFile = IsLegalLinkedFile(file_name);
	if (true == isLegalLinkedFile)
		return ANONYMOUS; // Since followSimLinks was removed due to JITC Demands, only the 'legal' linked files are able to be retrieved by Anonymous Connection

	int virtual_directory_pos = GetVirtualDirectoryPos(file_name);

	if (virtual_directory_pos!=-1)
		return m_VirtualDirectoryList[virtual_directory_pos].GetDirectoryAccess();

	return virtual_directory_pos;
}

bool CApacheModuleEngine::IsLegalLinkedFile(char* fname)
{
  if (NULL == fname)
    return false;

  // According to JITC restrictions ('FollowSimLinks' was removed),
  // only the following files may be retrieved by Anonymous Connection
  const std::string files[] = {
      (MCU_TMP_DIR+"/EMAProductType.txt"),
      (MCU_TMP_DIR+"/JITC_MODE.txt"),
      (std::string)(MCU_CONFIG_DIR+"/ema/InternalConfigSet_Customized.xml"),
      (MCU_TMP_DIR+"/Versions.xml"),
      (MCU_TMP_DIR+"/ClientCfg.xml")
  };

  for (const std::string* it = files; it < ARRAYEND(files); ++it)
  {
    if (0 == strcmp((*it).c_str(), fname))
      return true;
  }

	return false;
}

int CApacheModuleEngine::GetCreateDirectoryPermission(char* file_name)
{
	int virtual_directory_pos = GetVirtualDirectoryPos(file_name);

	if (virtual_directory_pos!=-1)
		return m_VirtualDirectoryList[virtual_directory_pos].GetCreateDirectory();

	return virtual_directory_pos;
}
//////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::AddAnonymousConnection(request_rec *pRequestRec, int &nConnId)
{

	const char* pszStationName = ap_get_remote_host(pRequestRec->connection,pRequestRec->per_dir_config,REMOTE_NAME, NULL);
	if((pszStationName == NULL) || (strlen(pszStationName) == 0))
			pszStationName = pRequestRec->useragent_ip;

	CConnection connection;
	connection.SetAuthorization(ANONYMOUS);
	connection.SetClientIp(pRequestRec->useragent_ip);
	connection.SetStationName(pszStationName);

	COsQueue* pMbxRcv  = new COsQueue;
	COsQueue* pMbxSend = new COsQueue;
	ConnectionResource *pConnRes = new ConnectionResource(pMbxSend,pMbxRcv);
	nConnId = m_pConnectionList->AddConnection(connection,pConnRes);

	apr_pool_userdata_setn((const void*)nConnId,"ConnId",NULL,pRequestRec->connection->pool);
	//apr_pool_userdata_setn((const void*)pMbxRcv , "MbxRcv"    , NULL, con->pool);
	//apr_pool_userdata_setn((const void*)pMbxSend, "MbxSend"   , NULL, con->pool);
	FTRACEINTO << "CApacheModuleEngine::AddAnonymousConnection: Connection added. ConnId: " << nConnId ;
}
//////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::HandleDeleteRequest(request_rec *pRequestRec, int nConnId, int nAuthorization)
{
	CHeaderParser HeaderParser;

    int nMessageId = HeaderParser.GetMessageId(pRequestRec);

	int permission = GetVirtualDirectoryPermission(pRequestRec->filename);

	// IT 2/4/09
	// VNGR-10006 : Non-privileged accounts can access and delete Audit files.
	// -----------------------------------------------------------------------
	pRequestRec->status = HTTP_FORBIDDEN;

	FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandleDeleteRequest: operation blocked. File: " << pRequestRec->filename << "; ConnId: " << nConnId;

	SendEventToAuditor(pRequestRec, nConnId, "Delete file", "A file in the RMX was deleted", "Failed to delete a file in the RMX - operation blocked.");

	SendResponse(pRequestRec,nConnId,nMessageId,NULL,0,NULL,false);

	return OK;
	// -----------------------------------------------------------------------

	/*
	if(nAuthorization > permission)
	{
	   pRequestRec->status = HTTP_FORBIDDEN;
	   FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::HandleDeleteRequest: permission denied. File: " << pRequestRec->filename << "; ConnId: " << nConnId <<  "; Permission required: " << permission << "; Authorization: " << nAuthorization;
	}
	else
	{
		if (nConnId == 0)
			AddAnonymousConnection(pRequestRec, nConnId);

		if((pRequestRec->filename != NULL) && (strlen(pRequestRec->filename) > 0))
		{
			if(unlink(pRequestRec->filename) != -1)
				pRequestRec->status = HTTP_OK;
			else
			{
				switch(errno)
				{
					case EACCES:
						pRequestRec->status = HTTP_FORBIDDEN;
						break;

					default:
						pRequestRec->status = HTTP_NOT_FOUND;
				}
			}
		}
		else
			pRequestRec->status = HTTP_NOT_FOUND;
	}

    SendEventToAuditor(pRequestRec, nConnId, "Delete file", "A file in the RMX was deleted", "Failed to delete a file in the RMX");

	SendResponse(pRequestRec,nConnId,nMessageId,NULL,0,NULL,false);

	return OK;
	*/
}
//////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::HandleFatalException(int signal)
{
    CMedString description = "ApacheModuleException - ";
    description << "signal : " << signal;

    FPASSERTMSG(1, description.GetString());

  	BOOL isFullOnly = TRUE;
    CHlogApi::TaskFault( FAULT_GENERAL_SUBJECT,
                         APACHE_MODULE_EXCEPTION,
                         MAJOR_ERROR_LEVEL,
                         description.GetString(),
                         isFullOnly );

    SystemCoreDump(FALSE);

    return TRUE;
}
//////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::HandleResponsePhase(request_rec *pRequestRec)
{
	int nConnId = GetValidConnId(pRequestRec);
	int nAuthorization = ANONYMOUS;
	BYTE bSHM = FALSE, bLogin=FALSE , bRVGW = FALSE;

	int returnStatus = DECLINED;

	if(m_pProcess!=NULL && m_pProcess->IsBlockRequests())
				return HTTP_SERVICE_UNAVAILABLE;
	// if ApacheModule didn't receive licensing info from McuMngr - don't accept connections until timer will arise
	if ( m_pProcess!=NULL && m_pProcess->GetWaitingLicensingInd() )
	{
		FTRACEINTO << "\nCApacheModuleEngine::GetWaitingLicensingInd - connection rejected because waiting for licensing information";
		return HTTP_SERVICE_UNAVAILABLE;
	}
	if(-1 == nConnId)
		return DECLINED;

	if(nConnId != 0)
	{
		m_pConnectionList->SetConnectionAlive(nConnId);
		m_pConnectionList->IncreaseRequestCounter(nConnId,pRequestRec->method_number);

		nAuthorization = m_pConnectionList->GetAuthorization(nConnId);
	}
	else
		AddAnonymousConnection(pRequestRec,nConnId);

    if(CheckIfRvgwRequest(pRequestRec))
    {
    	return DECLINED;
    }

    if(CheckIfRestAPIRequest(pRequestRec))
    {
    	return DECLINED;
    }

	if (strstr(pRequestRec->uri, "SHM")>0 && pRequestRec->method_number == M_POST)
	{
		bSHM = TRUE;

		CHeaderParser HeaderParser;
		bool isZip = HeaderParser.IsZippedContent(pRequestRec);

		char *pstrActionName=NULL;

		HeaderParser.GetAction(pRequestRec, &pstrActionName);

		if (nConnId == 0 || (pstrActionName && (strcmp(pstrActionName, "LOGIN")==0 || strcmp(pstrActionName, "LOGOUT")==0)))	//login ot logout request
		{
			if (pstrActionName)
			{
				if (strcmp(pstrActionName, "LOGIN")==0)
					FTRACEINTO << "CApacheModuleEngine::HandleResponsePhase - First connection to shelf manager";
				else
					FTRACEINTO << "CApacheModuleEngine::HandleResponsePhase - logout from shelf manager";

				if (strcmp(pstrActionName, "LOGIN")==0)
					bLogin = TRUE;
				PDELETEA(pstrActionName);
			}
		}
		else
		{
			delete[] pstrActionName;
			
			return DECLINED;
		}
	}
	else		//all other proxy requests should be transfer directly to proxy
	{
		if (strstr(pRequestRec->filename, "proxy") > 0)
		{
			return DECLINED;
		}
	}

	switch(pRequestRec->method_number)
	{
		case M_GET:
			returnStatus = HandleGetRequest(pRequestRec,nConnId,nAuthorization);
			break;

		case M_POST:
			returnStatus = HandlePostRequest(pRequestRec,nConnId, bSHM, bLogin);
			if (strstr(pRequestRec->uri, "SHM")>0 )	//only post requests will be transferred to SHM
			{
				if (returnStatus == FW_REQUEST_TO_SHM)
				{
					FTRACEINTO << "CApacheModuleEngine::HandleResponsePhase - the request have been FW to SHM";
					returnStatus = DECLINED;
				}
				else
				{
					FTRACEINTO << "CApacheModuleEngine::HandleResponsePhase - the request have been failed, the request wasn't FW to SHM";
					returnStatus = DONE;
				}
			}
			break;

		case M_PUT:
			returnStatus = HandlePutRequest(pRequestRec,nConnId,nAuthorization);
			break;

		case M_DELETE:
			returnStatus = HandleDeleteRequest(pRequestRec,nConnId, nAuthorization);
			break;

		default:
			;
	}

	return returnStatus;
}


bool CApacheModuleEngine::IsLogin(request_rec *pRequestRec)
{
	char *pstrActionName=NULL;
	CHeaderParser HeaderParser;

	HeaderParser.GetAction(pRequestRec, &pstrActionName);
	if(pstrActionName && (strcmp(pstrActionName, "LOGIN")==0))
	{
		delete[] pstrActionName;
		
		return true;
	}

	if (pstrActionName)
	{
		delete[] pstrActionName;
	}

	return false;
}
//////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::HandleLogTransaction(request_rec *pRequestRec)
{
    if (pRequestRec->method_number == M_GET && pRequestRec->status == 200)
    {
        int virtual_directory_pos = GetVirtualDirectoryPos(pRequestRec->filename);
        if (virtual_directory_pos != -1)
        {
            eProcessType process_type = m_VirtualDirectoryList[virtual_directory_pos].GetInformProcess();
            if (process_type != eProcessTypeInvalid)
            {
                CManagerApi api(process_type);
                api.InformHttpGetRequest(std::string(pRequestRec->filename));
            }
        }

    }

	return DECLINED;
}


/////////////////////////////////////////////////////////////////////////////////////////////////

void CApacheModuleEngine::BuildXmlLoginRequest(request_rec* pRequestRec, char** szLoginRequest)
{
	FTRACESTR(eLevelInfoNormal) << "\nBuildXmlLoginRequestStr";
	std::string pstrUserName, pstrPassword, pstrStationName;
	CHeaderParser HeaderParser;
	HeaderParser.GetLoginDetail(pRequestRec, &pstrUserName, &pstrPassword, &pstrStationName);
	int len = strlen(TRANS_LOGIN)+ pstrUserName.length() + pstrPassword.length() + pstrStationName.length();
	*szLoginRequest = new char[len+1];
	sprintf(*szLoginRequest,TRANS_LOGIN,0,pstrUserName.c_str(),pstrPassword.c_str(),pstrStationName.c_str());

	FTRACESTR(eLevelInfoNormal) << "\nBuildXmlLoginRequestStr szLoginRequest = " << *szLoginRequest;

}
////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::BuildXmlLogoutRequest(char** szLogoutRequest)
{
	*szLogoutRequest = new char[strlen(TRANS_LOGOUT)+1];

	sprintf(*szLogoutRequest,TRANS_LOGOUT);
}

////////////////////////////////////////////////////////////////////////////////
STATUS CApacheModuleEngine::SendSyncMessage(COsQueue* pMbxSend, COsQueue* pMbxRcv, char* pszMsg,
                                            CMessageHeader &header, CStringToProcessEntry &destProcessEntry,
                                            eOtherProcessQueueEntry eDestTask, BYTE bCompressed,
                                            const string & strCharSet,
                                            int nLen, int nConnId, int nPort,
                                            ConnectionDetails_S & connDetails,
                                            const char* pszClientCertificateSubj,
                                            BYTE bSHM )
{
	BYTE bCompressedForSend = bCompressed;
	char* pszMsgToSend = pszMsg;

	TICKS timeout = REQUEST_TIMEOUT_SHORT;
	if (eProcessInstaller == destProcessEntry.GetProcessType()
			|| eProcessCertMngr == destProcessEntry.GetProcessType()
			|| eProcessCDR == destProcessEntry.GetProcessType())
	{
		timeout = REQUEST_TIMEOUT_LONG;
	}

	if(!pszMsg || (!bCompressed && !strlen(pszMsg)))
		return STATUS_INVALID_INPUT;

	if(!pMbxSend || !pMbxRcv )
	{
		FPASSERTMSG(1,"COsQueue pMbxSend  or pMbxRcv  is NULL !!! ");
		return STATUS_INVALID_INPUT;
	}


	DWORD nNewLen=0;

	STATUS nStatus = CSegmentFuncForRequestHandler::PrepareUncompressedString(nLen,pszMsg, &pszMsgToSend, bCompressedForSend, nNewLen, bCompressed);

	if (nStatus!=STATUS_OK)
		return nStatus;

	CSegment *pSegment = new CSegment;
	pMbxSend->Serialize(*pSegment);

    m_pConnectionList->GetConnectionDetails(nConnId, connDetails);

    string sClientCertificateSubj = "";
    if( pszClientCertificateSubj != NULL )
    	sClientCertificateSubj = pszClientCertificateSubj;
    //DEBUG
	//FTRACESTR(eLevelInfoNormal) << "\nCApacheModuleEngine::SendSyncMessage: VVVVVVV: [" << (char*)(sClientCertificateSubj.c_str()) << "]";

	nStatus = CSegmentFuncForRequestHandler::PrepareSegmentBeforeSend(bCompressedForSend, nNewLen, pszMsgToSend,
                                       		 pSegment, nConnId,
                                             destProcessEntry, eDestTask, nPort,
                                       		 connDetails, bSHM, strCharSet, (WORD) m_pConnectionList->GetAuthorization(nConnId),
											 GetConnectionList()->GetLogin(nConnId),
											 sClientCertificateSubj,
											 header.m_ifNoneMatch);

	if (nStatus!=STATUS_OK)
	{
		delete pSegment;
		return nStatus;
	}

	const COsQueue *managerTransectionsQueue = m_pProcess->GetOtherProcessQueue(destProcessEntry.GetProcessType(),
                                                                                eDestTask);

	CTaskApi api;

	api.CreateOnlyApi(*managerTransectionsQueue, NULL, NULL, 0, header.m_RspMsgSeqNum);


	DWORD original_msg_seq_num = header.m_RspMsgSeqNum;

	if((nStatus = api.SendMsg(pSegment,XML_REQUEST)) != STATUS_OK)
	{
		BOOL isFullOnly = TRUE;
        string description = "SendSyncMessage: Sending failed to process - ";
        description += ProcessTypeToString(destProcessEntry.GetProcessType());
        CHlogApi::TaskFault( FAULT_GENERAL_SUBJECT,
                             FAILED_TO_SEND_SYNC_MESSAGE,
                             MAJOR_ERROR_LEVEL,
                             description.c_str(),
                             isFullOnly );

		FTRACEINTOFUNC << "Sending failed: " << description.c_str()
					   << ": " << CProcessBase::GetProcess()->GetStatusAsString(nStatus).c_str();
	}
	else
	{
		TICKS  AbsTimeOut = SystemGetTickCount() + timeout;
        TICKS CurTime = 0;
		do
		{
			nStatus = pMbxRcv->Receive(header,timeout);
			if (STATUS_QUEUE_ID_INVALID==nStatus)
				break;
			if(nStatus != STATUS_OK)
			{
				BOOL isFullOnly = TRUE;
				string description = "SendSyncMessage: Receiving failed from process - ";
				description += ProcessTypeToString(destProcessEntry.GetProcessType());
				CHlogApi::TaskFault( FAULT_GENERAL_SUBJECT,
	                                 FAILED_TO_RECEIVE_SYNC_MESSAGE,
	                                 MAJOR_ERROR_LEVEL,
	                                 description.c_str(),
	                                 isFullOnly );

				FTRACEINTOFUNC << "Receiving failed: " << description.c_str()
							   << ": " << CProcessBase::GetProcess()->GetStatusAsString(nStatus).c_str()
							   << ": destination " << ProcessTypeToString(pMbxRcv->m_process);
			}
			else
			{
				if (original_msg_seq_num == header.m_RspMsgSeqNum)
				{
					break;
				}
				else
				{
					FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::SendSyncMessage: original_msg_seq_num =  " << original_msg_seq_num << " but the sequence number that we got is " << header.m_RspMsgSeqNum;
				}
			}
			CurTime = SystemGetTickCount();
			timeout = AbsTimeOut - CurTime;

			FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::SendSyncMessage: Try again to get response for seq number: " << original_msg_seq_num;
			SystemSleep(50, FALSE); // 0.5 of seconds
		}
		while (AbsTimeOut > CurTime);
	}

	if(bCompressedForSend && (pszMsgToSend != pszMsg))
		DEALLOCBUFFER(pszMsgToSend);

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////
apr_status_t CApacheModuleEngine::CleanConnection(apr_pool_t *pConnPool)
{
	/*COsQueue *pMbxRcv, *pMbxSend;

	apr_pool_userdata_get((void**)&pMbxRcv,"MbxRcv",pConnPool);
	apr_pool_userdata_get((void**)&pMbxSend,"MbxSend",pConnPool);


    if(pMbxSend)
	{
		pMbxSend->Delete();
		delete pMbxSend;
		apr_pool_userdata_setn(NULL,"MbxSend",NULL,pConnPool);
	}

    if(pMbxRcv)
	{
        pMbxRcv->Flush();
		pMbxRcv->Delete();
		delete pMbxRcv;

		apr_pool_userdata_setn(NULL,"MbxRcv",NULL,pConnPool);
	}
*/

	int nConnId = 0;

	apr_pool_userdata_get((void**)&nConnId,"ConnId",pConnPool);

	if(m_pConnectionList->IsValidConnection(nConnId))
	{
		// VNGR-10773
		// If the last connection is removed from map, it is not audited on Connection Garbage Collector, so I added
		// an audit here.
		// -------------------------------------------------------------------------------------------------
		CConnectionList* pConnList = GetConnectionList();
		if (pConnList != NULL)
		{

			string	strLoginName;
			string	strStationName;
			string	clientIp;

			bool isConExist = pConnList->GetConnectionAuditDetails(nConnId, strLoginName, strStationName, clientIp);
			if (isConExist)
			{
				AUDIT_EVENT_HEADER_S outAuditHdr;
				CAuditorApi::PrepareAuditHeader(outAuditHdr,
												strLoginName,
												eMcms,
												strStationName,
												clientIp,
												eAuditEventTypeInternal,
												eAuditEventStatusOk,
												"Session timed out",
												"RMX Web Client session timed out due to no user activity.",
												"",
												"Garbage collector cleaned expired connection");
				CFreeData freeData;
				SendEventToAuditorThreadSafe(freeData,outAuditHdr);
			}
		}
		// -------------------------------------------------------------------------------------------------
		m_pConnectionList->RemoveConnection(nConnId);
	}

    return APR_SUCCESS;
}

void CApacheModuleEngine::InitConnection(conn_rec* con)
{
	if(m_pProcess!=NULL && m_pProcess->IsBlockRequests())
		return;
	FPASSERT_AND_RETURN(NULL == con);

	CPrettyTable<const char*, const char*> tbl("name", "value");

        std::ostringstream buf;
        buf << "httpd conn " << con->id;
	{
	  // Binds the temporary to a const reference.
          const std::string& cap = buf.str();
	  tbl.SetCaption(cap.c_str());
        }

	tbl.Add("client_ip"     , con->client_ip      ? con->client_ip      : "no");
	tbl.Add("remote_host"   , con->remote_host    ? con->remote_host    : "no");
	tbl.Add("remote_logname", con->remote_logname ? con->remote_logname : "no");
	tbl.Add("local_ip"      , con->local_ip       ? con->local_ip       : "no");
	tbl.Add("local_host"    , con->local_host     ? con->remote_host    : "no");
	FTRACEINTO << tbl.Get();


	int nConnId = 0;
	//AddAnonymousConnection(con,nConnId);
	apr_pool_userdata_setn((const void*)nConnId,"ConnId",NULL,con->pool);
	apr_pool_userdata_setn((const void*)false   , "SubRequest", NULL, con->pool);
	apr_pool_userdata_setn((const void*)0       , "MsgSeqNum" , NULL, con->pool);
}

// used as initialization flag for httpd.
// it should wait until ApacheModule inits its internal structure
static volatile bool CApacheModuleEngine_init = false;

////////////////////////////////////////////////////////////////////////////////
void* CApacheModuleEngine::InitSkeletonThread(void* pDummy)
{
	m_pProcess = new CApacheModuleProcess;
	m_pProcess->SetUp();

    CApacheModuleEngine_init = true;

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::WaitForInitSkeletonThread(DWORD maxNumIter)
{
    for(DWORD i = 0 ; i < maxNumIter ; i++)
    {
//         FTRACEINTO << "CApacheModuleEngine::WaitForInitSkeletonThread : "
//                   << "iter : " << i
//                   << ", status : " << (CApacheModuleEngine_init ? "Finished" : "Not Fininished");

        if(CApacheModuleEngine_init)
        {
            break;
        }

        SystemSleep(10, FALSE); // 0.1 of seconds
    }
}
apr_thread_mutex_t* CApacheModuleEngine::m_pAccessMutexEngine = NULL;
////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::Initialize(apr_pool_t *pConfigPool, server_rec *pServerRec)
{
	m_nKeepAliveTimeout = pServerRec->keep_alive_timeout / 1000;

	if(!m_pProcess)
	{
		pthread_t thread;

		pthread_create(&thread,NULL,CApacheModuleEngine::InitSkeletonThread,NULL);
        WaitForInitSkeletonThread(20);
	}

	if(!m_pConnectionList)
		m_pConnectionList = new CConnectionList(pConfigPool);
	if(pConfigPool)
			apr_thread_mutex_create(&m_pAccessMutexEngine,APR_THREAD_MUTEX_NESTED,pConfigPool);
	CShelfXmlConvertor::Initialize();
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::Clean()
{
	if(m_pProcess)
	{
		m_pProcess->TearDown();
		SystemSleep(50,FALSE);
		delete m_pProcess;
		m_pProcess = NULL;
	}

	if(m_pConnectionList)
		delete m_pConnectionList;

	if(m_pActionRedirectionMap)
		delete m_pActionRedirectionMap;
	if(m_pAccessMutexEngine)
		apr_thread_mutex_destroy(m_pAccessMutexEngine);
	CShelfXmlConvertor::Clean();
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::SetupResponseHeader(request_rec *pRequestRec, CHeaderParams& HeaderParams, bool bPragmaOnly)
{
	if(!pRequestRec)
		return;

	char szHeaderLine[50] = "", szContentLength[10];
	const char *pszAuthorization;

	if(CStringsMaps::GetDescription(AUTHORIZATION_GROUP_ENUM,HeaderParams.m_nAuthorization,&pszAuthorization))
    {
		sprintf(szHeaderLine+strlen(szHeaderLine),"AUTHORIZATION=%s",pszAuthorization);
    }
    else
    {
        FTRACEINTO << "\nCould not insert authorization val: " << HeaderParams.m_nAuthorization;
    }

	if(HeaderParams.m_nMessageId != -1)
    {
		sprintf(szHeaderLine+strlen(szHeaderLine),";MESSAGE_ID=%d",HeaderParams.m_nMessageId);
    }
    else
    {
        //FTRACEINTO << "\nCould not insert message id (-1)";
    }

	apr_table_set(pRequestRec->headers_out,"Pragma",szHeaderLine);

	apr_table_set(pRequestRec->headers_out,"Accept-Ranges","none");
         
	char buff[200];
        memset(buff,0,sizeof(buff));
	sprintf(buff, "CP=%cALL IND DSP COR ADM CONo CUR CUSo IVAo IVDo PSA PSD TAI TELo OUR SAMo CNT COM INT NAV ONL PHY PRE PUR UNI%c", '"','"');
	apr_table_set(pRequestRec->headers_out,"P3P",buff);

	if(!bPragmaOnly)
	{
		snprintf(szContentLength,sizeof(szContentLength), "%d",HeaderParams.m_nContentLength);
		apr_table_set(pRequestRec->headers_out,"Content-Length",szContentLength);

		if(HeaderParams.m_bZip)
			apr_table_set(pRequestRec->headers_out,"Content-Encoding","zip");
	}
}

////////////////////////////////////////////////////////////////////////////////
eProcessType CApacheModuleEngine::ProcNameToProcType(char* pszProcName)
{
	if(!pszProcName)
		return eProcessTypeInvalid;

	for(int i=0; i < NUM_OF_PROCESS_TYPES; i++)
	{
		if(!strcmp(ProcessNames[i],pszProcName))
			return (eProcessType)i;
	}

	return eProcessTypeInvalid;
}

////////////////////////////////////////////////////////////////////////////////
int CApacheModuleEngine::GetKeepAliveTimeout()
{
	return m_nKeepAliveTimeout;
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::SetPhysicalPath(char* pszVirtual, const char* pszPhysicalPath)
{
	if (pszVirtual && pszPhysicalPath)
	{
		if (m_nLastVD + 1 < MAX_NUMBER_OF_VIRTUAL_DIRECTORIES)
		{
			m_nLastVD++;
			m_VirtualDirectoryList[m_nLastVD].SetPhysicalPath(pszVirtual, pszPhysicalPath);
		}
		else
		{
			// Vasily 5.08.13: At this stage Apache is starting up and we can't send
			// message to Logger or add assert, but we can write message to startup log
			//   --> MCU_TMP_DIR/startup_logs/ApacheModuleLoad.log

			std::stringstream sstr;
			sstr << "CApacheModuleEngine::SetPhysicalPath: no space for physical path ["
				<< pszPhysicalPath << "], nLastVD=[" << m_nLastVD << "].";
			DumpLoadingMessage(sstr.str().c_str());
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::SetDirectoryAccess(const char* pszPhysicalPath, char* pszDirectoryAccess)
{
	int access = -1;
	int i;

	if (pszPhysicalPath && pszDirectoryAccess)
	{
		int accessLen = strlen(pszDirectoryAccess);
		for (i=0; i<accessLen; i++)
			pszDirectoryAccess[i] = tolower(pszDirectoryAccess[i]);

		if (strcmp(pszDirectoryAccess, "administrator")==0)
			access = SUPER;
        else if (strcmp(pszDirectoryAccess, "administrator_readonly")==0)
            access = ADMINISTRATOR_READONLY;
		else if (strcmp(pszDirectoryAccess, "authenticated_user")==0)
			access = GUEST;
		else if (strcmp(pszDirectoryAccess, "anonymous")==0)
			access = ANONYMOUS;
        else if(strcmp(pszDirectoryAccess, "auditor") == 0)
        {
        	BOOL bJitcMode = CApacheModuleProcess::IsFederalOn();
        	if(bJitcMode == TRUE)
        		access = SUPER;
        	else
            access = AUDITOR;
        }

		for (i=m_nLastVD; i > -1; i--)
		{
			if (m_VirtualDirectoryList[i].GetPhysicalPath()==NULL)
				continue;
			if (strcmp(pszPhysicalPath, m_VirtualDirectoryList[i].GetPhysicalPath())==0)
			{
				m_VirtualDirectoryList[i].SetDirectoryAccess(access);
				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::SetCreateDirectory(const char* pszPhysicalPath,
                                             char* pszCreateDir)
{
	int create_dir = -1;
	int i;

	if (pszPhysicalPath && pszCreateDir)
	{
		int create_dir_Len = strlen(pszCreateDir);
		for (i=0; i<create_dir_Len; i++)
			pszCreateDir[i] = tolower(pszCreateDir[i]);

		if (strcmp(pszCreateDir, "none")==0)
			create_dir = CREATE_DIR_NONE;
		else if (strcmp(pszCreateDir, "all")==0)
			create_dir = CREATE_DIR_ALL;
		else if (strcmp(pszCreateDir, "administrator")==0)
			create_dir = CREATE_DIR_ADMIN;
		for (i=m_nLastVD; i > -1; i--)
		{
			if (strcmp(pszPhysicalPath, m_VirtualDirectoryList[i].GetPhysicalPath())==0)
			{
				m_VirtualDirectoryList[i].SetCreateDirectory(create_dir);
				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::SetAuditabilityOfDirectory(const char* pszPhysicalPath,
                                                     char* pszAuditability)
{
    if(NULL == pszPhysicalPath || NULL == pszAuditability)
    {
        return;
    }

    CObjString::ToUpper(pszAuditability);

    bool isAuditable = true;
    if (strcmp(pszAuditability, "YES") == 0)
    {
        isAuditable = true;
    }
    else if(strcmp(pszAuditability, "NO") == 0)
    {
        isAuditable = false;
    }

    for (int i = m_nLastVD; i > -1; i--)
    {
        if (strcmp(pszPhysicalPath, m_VirtualDirectoryList[i].GetPhysicalPath()) == 0)
        {
            m_VirtualDirectoryList[i].SetAuditability(isAuditable);
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::SetInformProcessOfDirectory(const char* pszPhysicalPath,
                                                      char* pszProcessName)
{
    if(NULL == pszPhysicalPath || NULL == pszProcessName)
    {
        return;
    }


    for (int i = m_nLastVD; i > -1; i--)
    {
        if (strcmp(pszPhysicalPath,
                   m_VirtualDirectoryList[i].GetPhysicalPath()) == 0)
        {
            m_VirtualDirectoryList[i].SetInformProcess(pszProcessName);
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
bool CApacheModuleEngine::GetPhysicalPath(std::string strVirtualPath,
                                          std::string& strPhysicalPath)
{
	char *pVirtualDirectory;
    char *original_buffer;

	pVirtualDirectory = new char[strlen(strVirtualPath.c_str())+1];
    original_buffer = pVirtualDirectory;

	memset(pVirtualDirectory, 0, strlen(strVirtualPath.c_str())+1);

	unsigned int pos=0, posCopy=0, iNestedDir = 0;

	char szFailureDetails[150];
	//strVirtualPath = m_VirtualDirectoryList[i].GetVirtualPath();
	snprintf(szFailureDetails,149,"%s", strVirtualPath.c_str());
	FTRACESTR(eLevelInfoNormal) << "CApacheModuleEngine::VirtualPath: " << szFailureDetails;

	while(pos<(strlen(strVirtualPath.c_str())))
	{
		iNestedDir++;

		if(iNestedDir > MAX_NUMBER_OF_NESTED_VIRTUAL_DIRECTORIES) //to avoid user's abuse of this code
		{
			break;
		}

		if(posCopy > 0)
		{
			pVirtualDirectory[posCopy] = '/';
			posCopy++;
			pos++;
		}

		for (; pos<(strlen(strVirtualPath.c_str())); pos++)
		{
			if (strVirtualPath.c_str()[pos] != '/')
				pVirtualDirectory[posCopy++] = strVirtualPath.c_str()[pos];
			else
			{
				if((pos > 0) || (strlen(strVirtualPath.c_str()) == 1))
				{
					pVirtualDirectory[posCopy] = '\0';
					break;
				}
			}
		}

		if (pos==strlen(strVirtualPath.c_str()))
			pVirtualDirectory[posCopy] = '\0';


		for (int i=0; i<=m_nLastVD; i++)
		{
			if (strcmp(pVirtualDirectory, m_VirtualDirectoryList[i].GetVirtualPath())==0)
			{
				strPhysicalPath = m_VirtualDirectoryList[i].GetPhysicalPath();
				strcpy(pVirtualDirectory, strVirtualPath.c_str());
				if (pVirtualDirectory[pos]=='/')
					pVirtualDirectory++;
				pVirtualDirectory += pos;
				strPhysicalPath+= pVirtualDirectory;
				FTRACEINTO << "CApacheModuleEngine::GetPhysicalPath - strPhysicalPath found (final) = " << strPhysicalPath.c_str();
	            delete [] original_buffer;
	            original_buffer = NULL;


				return true;
			}
		}
	}

	FTRACEINTO << "CApacheModuleEngine::GetPhysicalPath - cant find strPhysicalPath for " << strVirtualPath.c_str();
    delete [] original_buffer;
    original_buffer = NULL;
	return false;
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::TraceHeader(request_rec *pRequestRec)
{
	FTRACEINTO << "Http Header received:";

	const apr_array_header_t *pHeaderArray;
	const apr_table_entry_t *elts;

	pHeaderArray = apr_table_elts(pRequestRec->headers_in);
	elts = (const apr_table_entry_t*)pHeaderArray->elts;

	FTRACEINTO << pRequestRec->the_request;

	for(int i=0; i < pHeaderArray->nelts; i++)
	{
        if (elts[i].key == NULL || strcmp(elts[i].key, "Authorization") == 0)
            continue;

		FTRACEINTO << elts[i].key << ":" << elts[i].val;
	}
}

////////////////////////////////////////////////////////////////////////////////
STATUS CApacheModuleEngine::ExtractEncodingFromHttpHeader(CHeaderParser HeaderParser,
														  request_rec *pRequestRec,
														  int nConnId, int nMessageId,
                                                          string & outStrCharset)
{
    HeaderParser.GetContentEncoding(pRequestRec, outStrCharset);
    if(false == outStrCharset.empty())
    {
        CObjString::ToUpper((char*)outStrCharset.c_str());

        bool isKnownEncoding = CEncodingConvertor::IsKnownEncoding(outStrCharset);
        if(false == isKnownEncoding)
        {
            FTRACEINTO << "CApacheModuleEngine::ExtractEncodingFromHttpHeader: Encoding(in HTTP header) is not supported: "
                       << outStrCharset.c_str();
            SendGeneralResponse(pRequestRec,"", STATUS_UNKNOWN_CHARSET_ENCODING, nConnId, nMessageId, outStrCharset);
            return STATUS_UNKNOWN_CHARSET_ENCODING;
        }
    }
    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
STATUS CApacheModuleEngine::DecompressedTheContent(char* pszNonZippedBuffer,
                                                   int pszNonZippedBufferLen,
												   request_rec *pRequestRec,
												   char* pszRequestContent,
												   int nContentLen,
												   BYTE& bCompressed,
                                                   int nConnId, int nMessageId,
                                                   const string & strCharset)
{
	CZipWrapper ZipWrapper;
	int nUnZippedLen;

	nUnZippedLen = ZipWrapper.Inflate((unsigned char*)pszRequestContent,nContentLen,
                                      (unsigned char*)pszNonZippedBuffer, pszNonZippedBufferLen);

	bCompressed = true;

	if(nUnZippedLen <= 0)
	{
		FPASSERTMSG(1,"CApacheModuleEngine::DecompressedTheContent: Decompression failed");
		TraceHeader(pRequestRec);
		SendGeneralResponse(pRequestRec,"",STATUS_DECOMPRESSION_FAILED,nConnId,nMessageId, strCharset);
		return STATUS_DECOMPRESSION_FAILED;
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
STATUS CApacheModuleEngine::ExtractEncodingFromComment(char* pszNonZippedBuffer,
													   request_rec *pRequestRec,
													   int nConnId, int nMessageId,
                                                       string & outStrCharset)
{
    string strCharSetComment;
    COstrStream statusString;
    bool resParseComment = CEncodingConvertor::GetEncodingType(pszNonZippedBuffer,
                                                               strCharSetComment,
                                                               statusString);
    // the encoding was found, the value of the strCharSetComment was taken from comment.
    if(resParseComment)
    {
        // check for conflict between HTTP header and comment
        if(false == outStrCharset.empty() && strCharSetComment != outStrCharset)
        {
            FTRACEINTO << "CApacheModuleEngine::HandleEncoding: Conflict between HTTP header and comment: "
                       << "\nHTTP Header : " << outStrCharset.c_str()
                       << "\nComment : " << strCharSetComment.c_str()
                       << "\n"
                       << pszNonZippedBuffer;
            SendGeneralResponse(pRequestRec,
                                "",
                                STATUS_CHARSET_ENCODING_UNDETERMINABLE,
                                nConnId,
                                nMessageId,
                                outStrCharset);
            return STATUS_CHARSET_ENCODING_UNDETERMINABLE;
        }
    }
    if(outStrCharset.empty())
    {
        outStrCharset = strCharSetComment;
    }
    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::DumpActionRedirectionMap(std::ostream& ostr)
{
	m_pActionRedirectionMap->DumpActionRedirectionMap(ostr);
}

////////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::OnMissingAuditAttr(const char *action, const char *desc, const char *detail)
{
    string message = "ERROR during parsing action-redirection map\n";
    message += action;
    message += " has problem with auditable attribute, check ActionRedirection.xml\n";
    message += desc;
    message += ", ";
    message += detail;

    FPASSERTMSG(TRUE, message.c_str());
}

////////////////////////////////////////////////////////////////////////////
//./srclib/apr-util/include/apr_xml.h:148
// struct apr_xml_attr {
//     /** attribute name */
//     const char *name;
//     /** index into namespace array */
//     int ns;

//     /** attribute value */
//     const char *value;

//     /** next attribute */
//     struct apr_xml_attr *next;
// };
bool CApacheModuleEngine::ParseTransactionAttributes(const apr_xml_elem *pActionElement,
                                                     bool & outIsAuditable,
                                                     string & outTransName,
                                                     string & outTransDesc,
                                                     string & outErrorMessage)
{
    bool isAuditAttrrExist = false;
    bool isTransNameExist = false;
    bool isTransDescExist = false;


    apr_xml_attr *elemAttr = pActionElement->attr;
    while(NULL != elemAttr)
    {
        if(0 == strcmp(elemAttr->name, "auditable"))
        {
            if(!TakeAttrAuditable(elemAttr, outIsAuditable, outErrorMessage))
            {
                return false;
            }
            isAuditAttrrExist = true;
        }
        else if(0 == strcmp(elemAttr->name, "name"))
        {
            if(!TakeAttrName(elemAttr, outTransName, outErrorMessage))
            {
                return false;
            }
            isTransNameExist = true;
        }
        else if(0 == strcmp(elemAttr->name, "description"))
        {
            if(!TakeAttrDescription(elemAttr, outTransDesc, outErrorMessage))
            {
                return false;
            }
            isTransDescExist = true;
        }

        elemAttr = elemAttr->next;
    }

    // auditable attribute must exist
    if(!isAuditAttrrExist)
    {
        outErrorMessage = "attribute auditable was not found";
        return false;
    }

    // transaction is auditable but name or description do not exist
    // outIsAuditable | isTransNameExist  | isTransDescExist
    //      1         |       0           |       0
    if(outIsAuditable && (!isTransNameExist || !isTransDescExist))
    {
        outErrorMessage = "attribute name or description were not found";
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool CApacheModuleEngine::TakeAttrAuditable(const apr_xml_attr *elemAttr,
                                            bool & outIsAuditable,
                                            string & outErrorMessage)
{
    if(0 == strcmp(elemAttr->value, "no"))
    {
        outIsAuditable = false;
    }
    else if(0 == strcmp(elemAttr->value, "yes"))
    {
        outIsAuditable = true;
    }
    else
    {
        outErrorMessage = "value of ";
        outErrorMessage += elemAttr->name;
        outErrorMessage += " is not [yes,no] : ";
        outErrorMessage += elemAttr->value;
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool CApacheModuleEngine::TakeAttrName(const apr_xml_attr *elemAttr,
                                       string & outTransName,
                                       string & outErrorMessage)
{
    outTransName = elemAttr->value;
    if(outTransName.empty())
    {
        outErrorMessage = "attribute ";
        outErrorMessage += elemAttr->name;
        outErrorMessage += " can't be empty";
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool CApacheModuleEngine::TakeAttrDescription(const apr_xml_attr *elemAttr,
                                              string & outTransDesc,
                                              string & outErrorMessage)
{
    outTransDesc = elemAttr->value;
    if(outTransDesc.empty())
    {
        outErrorMessage = "attribute ";
        outErrorMessage += elemAttr->name;
        outErrorMessage += " can't be empty";
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::DumpVirtualDir(std::ostream& answer)
{
	answer.setf(std::ios::left,std::ios::adjustfield);
	answer.setf(std::ios::showbase);

	answer << " --> MAX_NUMBER_OF_VIRTUAL_DIRECTORIES=" << (int)MAX_NUMBER_OF_VIRTUAL_DIRECTORIES << "\n"
		   << " --> m_nLastVD=" << m_nLastVD << "\n";
	answer << "Dump of virtual directory's table (i <= m_nLastVD)\n"
		   << "--------------------------------------------------------------------\n";

	for(int i = 0 ; i <= m_nLastVD ; i++)
	{
		CVirtualDirectory & refCurrentVirtDer = m_VirtualDirectoryList[i];
		answer << std::setw(20) << refCurrentVirtDer.GetPhysicalPath() << ", "
			   << std::setw(10) << refCurrentVirtDer.GetVirtualPath() << ", "
			   << std::setw(4)  << refCurrentVirtDer.GetDirectoryAccess() << ", "
			   << refCurrentVirtDer.GetCreateDirectory() << ", "
			   << (refCurrentVirtDer.GetAuditability() ? "Auditable" : "Not Auditable")
			   << "\n";
	}

	answer << "\n\nDump of virtual directory's table(i < MAX_NUMBER_OF_VIRTUAL_DIRECTORIES)\n"
		   << "--------------------------------------------------------------------\n";

	for(int i = 0 ; i < MAX_NUMBER_OF_VIRTUAL_DIRECTORIES ; i++)
	{
		CVirtualDirectory & refCurrentVirtDer = m_VirtualDirectoryList[i];
		answer << std::setw(20) << refCurrentVirtDer.GetPhysicalPath() << ", "
			   << std::setw(10) << refCurrentVirtDer.GetVirtualPath() << ", "
			   << std::setw(4)  << refCurrentVirtDer.GetDirectoryAccess() << ", "
			   << refCurrentVirtDer.GetCreateDirectory() << ", "
			   << (refCurrentVirtDer.GetAuditability() ? "Auditable" : "Not Auditable")
			   << "\n";
	}
}

static bool has_extension(const char* name, const char* ext[])
{
  if (NULL == ext)
    return true;

  if (NULL == name)
    return false;

  std::string str(name);

  // find extension position
  size_t dot = str.rfind(".");
  if (dot == std::string::npos)
    return false;

  // transform extension to low case
  std::transform(str.begin() + dot + 1, str.end(),
                 str.begin() + dot + 1, ::tolower);

  while (*ext != NULL)
  {
    // compare extension of the name to pattern
    if (0 == str.compare(dot + 1, str.length() - dot - 1, *ext))
      return true;

    ++ext;
  }

  return false;
}

// Static
bool CApacheModuleEngine::IsCertFile(const char* name)
{
  static const char* ext[] = {
    "pem",
    "der",
    "p7b",
    "pfx",
    NULL
  };

  return has_extension(name, ext);
}

BOOL CApacheModuleEngine::IsExtensionAllowed(const std::string& url)
{
  static const char* ext[] = {
    "bin",
    "wav",
    "bmp",
    "vid",
    "slide",
    "jpg",
    "jpeg",
    "xml",
    "bck",
    "txt",
    "pem",
    "der",
    "p7b",
    "pfx",
    "tgz",
    "zip",
    NULL
  };

  return has_extension(url.c_str(), ext) ? TRUE : FALSE;
}

// Static
STATUS CApacheModuleEngine::SendSyncMessageToCertMngr(request_rec* pRequestRec,
                                                      const char* strPhysicalPath)
{
  CMessageHeader Header;
  Header.m_msgType = eSyncMessage;
  Header.m_RspMsgSeqNum = GetNextMsgSeqNum(pRequestRec);

  COsQueue* pMbxSend = NULL, * pMbxRcv = NULL;
  int nConnId =-1;
  nConnId = GetValidConnId(pRequestRec);
  m_pConnectionList->GetConnectionQueues(nConnId,&pMbxRcv,&pMbxSend);
  /*apr_pool_userdata_get((void**)&pMbxSend, "MbxSend",
                        pRequestRec->connection->pool);
  apr_pool_userdata_get((void**)&pMbxRcv, "MbxRcv",
                        pRequestRec->connection->pool);
*/
   if(!pMbxSend || !pMbxRcv )
   {
		FPASSERTMSG(1,"COsQueue pMbxSend  or pMbxRcv  is NULL !!! ");
		return STATUS_INVALID_INPUT;
   }

  CSegment* pSeg = new CSegment();



  pMbxSend->Serialize(*pSeg);
  *pSeg << strPhysicalPath;

  CTaskApi        api;
  const COsQueue* certMngrQueue =
    m_pProcess->GetOtherProcessQueue(eProcessCertMngr, eManager);
  api.CreateOnlyApi(*certMngrQueue, NULL, NULL, 0, Header.m_RspMsgSeqNum);

  STATUS stat = api.SendMsg(pSeg, HTTPD_TO_CERTMNGR_LINK_FILE);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
    "Failed to send HTTPD_TO_CERTMNGR_LINK_FILE to CertMngr: "
    << m_pProcess->GetStatusAsString(stat),
    HTTP_OK);

  TICKS CurTime = 0;
  TICKS timeout = REQUEST_TIMEOUT_LONG;
  TICKS AbsTimeOut = SystemGetTickCount() + timeout;
  DWORD original_msg_seq_num = Header.m_RspMsgSeqNum;

  do
  {
    if ((stat = pMbxRcv->Receive(Header, timeout)) != STATUS_OK)
    {
      BOOL isFullOnly = TRUE;
      CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
                          FAILED_TO_RECEIVE_SYNC_MESSAGE,
                          MAJOR_ERROR_LEVEL,
                          "HandlePutRequest: Receiving failed from CertMngr",
                          isFullOnly);

      FPASSERTSTREAM(true,
                     "Receiving failed: " << m_pProcess->GetStatusAsString(stat));
    }
    else
    {
      // Exists on last piece
      if (original_msg_seq_num == Header.m_RspMsgSeqNum)
        break;

      FTRACEINTOFUNC << "original_msg_seq_num =  " << original_msg_seq_num
                     << " but the sequence number that we got is "
                     << Header.m_RspMsgSeqNum;
    }

    CurTime = SystemGetTickCount();
    timeout = AbsTimeOut - CurTime;

    FTRACEINTOFUNC << "Try again to get response for seq number: "
                   << original_msg_seq_num;
  }
  while (AbsTimeOut > CurTime);

  if (STATUS_LINK_FILE_SUCCEEDED != Header.m_opcode)
  {
    FTRACEINTOFUNC << "Failed to link a certificate file";
    POBJDELETE(Header.m_segment);
    return HTTP_FORBIDDEN;
  }

  FTRACEINTOFUNC << "Succeeded to link a certificate file ";

  POBJDELETE(Header.m_segment);
  return HTTP_OK;
}

bool CApacheModuleEngine::IsRvgwSslPorts()
{
	 BOOL isSSLSockets = NO;
	 CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	 FPASSERTMSG_AND_RETURN_VALUE(sysConfig == NULL, "IsRvgwSslPorts: sysConfig is null", isSSLSockets);

	 sysConfig->GetBOOLDataByKey("V35_USE_SSL_PORTS", isSSLSockets);

	 return isSSLSockets;
}

bool CApacheModuleEngine::IsRvgwUseActiveX()
{
	BOOL isActiveX = NO;

	 CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	 if (sysConfig != NULL)
	 {
		 sysConfig->GetBOOLDataByKey("VIEW_RVGW_ACTIVEX", isActiveX);
	 }
	 else
	 {
		 //BRIDGE-6701  sysConfig was null when running smoke test
		 FTRACESTR(eLevelError) << "IsRvgwUseActiveX: sysConfig is null";
	 }
	 return isActiveX;
}
//This function identify and later Blocks API messages to proccesses that are not valid or Up
//in Jitc some proccesses are not launched - see CMcmsDaemonProcess::IsLaunched
eDestProccessValidity CApacheModuleEngine::IsDestProccessValid(eProcessType procType)
{

	if ( eProcessTypeInvalid == procType )
		return pDestprocInValid;
	BOOL bJitcMode = CApacheModuleProcess::IsFederalOn();
	if (bJitcMode && ( eProcessExchangeModule == procType  ||  eProcessFailover == procType ) )
		return pDestprocInValidUnderJitc;
	return pDestprocValid;
}


// static
////////////////////////////////////////////////////////////////////////////
void CApacheModuleEngine::DumpLoadingMessage(const char *pszMessage)
{
	std::string cmd, info, answer;
	info = pszMessage;

	cmd = "mkdir -p ";
	cmd += OS_STARTUP_LOGS_PATH;
	SystemPipedCommand(cmd.c_str(), answer);

	std::string fileName =  OS_STARTUP_LOGS_PATH + APACHE_MODULE_LOADING_LOG;
	std::string dump = "echo \"" + info + "\" >> "+ fileName;
	SystemPipedCommand(dump.c_str(), answer);

	SystemPipedCommand(("/bin/chmod a+w "+ fileName).c_str(), answer);
}

bool CApacheModuleEngine::IsPlatformOfNewArch()
{
    if( eProductTypeUnknown == m_curProductType)
    {
        m_curProductType = CProcessBase::GetProcess()->GetProductType();
    }
    
    if(eProductTypeGesher == m_curProductType 
       || eProductTypeNinja == m_curProductType)
    {
        return true;
    }

    return false;
}

eProductType CApacheModuleEngine::GetProductType()
{
    return m_curProductType;
}

