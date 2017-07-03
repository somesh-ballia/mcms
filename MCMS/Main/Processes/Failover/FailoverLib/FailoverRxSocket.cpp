/*
 * FailoverRxSocket.cpp
 *
 *  Created on: Sep 1, 2009
 *      Author: yael
 */

#include "FailoverRxSocket.h"
#include "HTTPPars.h"
#include "psosxml.h"
#include "TraceStream.h"
#include "EncodingConvertor.h"
#include "UnicodeDefines.h"
#include "SocketApi.h"
#include "OpcodesMcmsInternal.h"
#include "ManagerApi.h"
#include "FailDetectionApi.h"
#include "FailoverSyncApi.h"
#include "FailoverProcess.h"
#include "ZipWrapper.h"
#include "HTTPDefi.h"
#include "ApiStatuses.h"
#include "SecuredSocketConnected.h"

PBEGIN_MESSAGE_MAP(CFailoverRxSocket)
	ONEVENT( SOCKET_WRITE, ANYCASE,  CStateMachine::NullActionFunction)
PEND_MESSAGE_MAP(CFailoverRxSocket, CSocketRxTask);



/////////////////////////////////////////////////////////////////////////////
void FailoverRxEntryPoint(void* appParam)
{
	BYTE bSecured;
	CSegment* pSeg = (CSegment*)appParam;
	*pSeg >> (BYTE&)bSecured;
	pSeg->ResetRead();

	if (bSecured == TRUE)
	{
		CFailoverRxSocket*  pRxSocket = new CFailoverRxSocket(new CSecuredSocketConnected);
		pRxSocket->Create(*(CSegment*)appParam);
	}
	else
	{
		CFailoverRxSocket*  pRxSocket = new CFailoverRxSocket(new COsSocketConnected);
		pRxSocket->Create(*(CSegment*)appParam);
	}
}

/////////////////////////////////////////////////////////////////////////////
CFailoverRxSocket::CFailoverRxSocket(COsSocketConnected * pSocketDesc)
:CSocketRxTask(pSocketDesc),CHTTPReceiver(TRUE)
{
	m_pProcess = (CFailoverProcess*)(CProcessBase::GetProcess());
}


/////////////////////////////////////////////////////////////////////////////
CFailoverRxSocket::~CFailoverRxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverRxSocket::InitTask()
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CFailoverRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
const char* CFailoverRxSocket::GetTaskName() const
{
	return "CFailoverRxSocket";
}


//////////////////////////////////////////////////////////////////////
void CFailoverRxSocket::ReceiveFromSocket()
{
	if (ReadHttp(*this))
    {
        char *pHttpHeader = GetHttpHeader();

        string charSet;
	    CHTTPHeaderParser::GetContentCharset(pHttpHeader, charSet);
	    if(false == charSet.empty())
	    {
		   bool isKnown = CEncodingConvertor::IsKnownEncoding(charSet);
		   if(false == isKnown)
		   {
			   // drop the packet
			   FTRACEINTO << "\nCFailoverRxSocket::ReceiveFromSocket\nDrop the packet\nthe encoding is unknown: " << charSet.c_str();
			   return;
		   }
	    }

	    char *pContent;
	    char *pTmpContent = GetHttpContent();

        if (CHTTPHeaderParser::GetContentEncoding(pHttpHeader) == CONTENT_ENCODING_ZIP)
        {
        	DWORD tmpContentLen;
        	CHTTPHeaderParser::GetContentLength(pHttpHeader, &tmpContentLen);

        	if (tmpContentLen <=0 )
			{
        		FTRACEINTO << "\nCFailoverRxSocket::ReceiveFromSocket tmpContentLen is not ok \n" << tmpContentLen;
				return;
			}

        	const int pszNonZippedBufferLen = (MAX_NON_ZIPPED*200) + 1;
			char* pszNonZippedBuffer = new char[pszNonZippedBufferLen];
			memset(pszNonZippedBuffer, 0, pszNonZippedBufferLen);

			if (DecompressedTheContent(pszNonZippedBuffer, pszNonZippedBufferLen, pTmpContent, tmpContentLen) != STATUS_OK)
			{
				PDELETEA(pszNonZippedBuffer);
				return;
			}
			pContent = new char[pszNonZippedBufferLen + 1];
	        strcpy(pContent, pszNonZippedBuffer);
	        pContent[pszNonZippedBufferLen] = 0;
	        PDELETEA(pszNonZippedBuffer);
        }

        else
        {
        	DWORD tmpContentLen = strlen(pTmpContent);
        	pContent = new char[strlen(pTmpContent) + 1];
			strcpy(pContent, pTmpContent);
			pContent[tmpContentLen] = 0;
        }

        // convert to UTF-8
        COstrStream statusString;
        string currentEncoding;
        STATUS statusUTF8Convert = CEncodingConvertor::ConvertValidate(MCMS_INTERNAL_STRING_ENCODE_TYPE,
                                                                       currentEncoding,
                                                                       pContent,
                                                                       statusString);
        if(STATUS_OK != statusUTF8Convert)
        {
            // drop the packet
            FTRACEINTO << "\nCFailoverRxSocket::ReceiveFromSocket Drop the packet\n"
                       << statusString.str().c_str() << "\n"
                       << pContent;
            PDELETEA(pContent);
            return;
        }

		int status=0;
		CHTTPHeaderParser::GetHTTPResponseStatus(m_pHttpHeader,&status);
		if (status==200 && pContent)
		{
			//FTRACESTR(eLevelInfoNormal) <<  "\nCFailoverRxSocket::ReceiveFromSocket - reply from server - content... " << pContent;
			CXMLDOMDocument *pDom = new CXMLDOMDocument;
            if (pDom->Parse((const char **)&pContent)==SEC_OK)
            {
                CXMLDOMElement* pRoot = pDom->GetRootElement();
                if(pRoot)
                {
                	char* nodeName = NULL;
                	pRoot->get_nodeName(&nodeName);
                	DispatchResponse(nodeName, pContent, pRoot);
                }
            }
            POBJDELETE(pDom);
		}
		if(status!=200)
		{
			FTRACESTR(eLevelInfoHigh) <<  "\nCFailoverRxSocket::ReceiveFromSocket - bad reply from server: status =  " << status;
			if(pContent)
			{
				FTRACESTR(eLevelInfoHigh) <<  "\nCFailoverRxSocket::ReceiveFromSocket - bad reply from server - content... " << pContent;
			}
		}

        PDELETEA(pContent);
    }

    else
    {
    	FTRACESTR(eLevelInfoHigh) <<  "\nCFailoverRxSocket::ReceiveFromSocket - bad reply from server. fail to read the response.";
    }
}

////////////////////////////////////////////////////////////////////////////
STATUS CFailoverRxSocket::DecompressedTheContent(char* pszNonZippedBuffer,
                                                   int pszNonZippedBufferLen,
												   char* pszRequestContent,
												   int nContentLen)
{
	CZipWrapper ZipWrapper;
	int nUnZippedLen;

	nUnZippedLen = ZipWrapper.Inflate((unsigned char*)pszRequestContent,nContentLen,
                                      (unsigned char*)pszNonZippedBuffer, pszNonZippedBufferLen);

	if (nUnZippedLen <= 0)
	{
		FPASSERTMSG(1,"CFailoverRxSocket::DecompressedTheContent: Decompression failed");
		return STATUS_DECOMPRESSION_FAILED;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CFailoverRxSocket::DispatchResponse(const char* nodeName, const char *pContent, CXMLDOMElement* pRoot)
{
	string sNodeName = nodeName;

	if ("RESPONSE_TRANS_MCU" == sNodeName)
	{
		TreatResponseTransMcu(pRoot, pContent);
	}

	else
	{
		TreatResponseOther(pRoot, pContent);		
	}	
}

//////////////////////////////////////////////////////////////////////
void CFailoverRxSocket::TreatResponseTransMcu(CXMLDOMElement* pRootElement, const char *pContent)
{
	CSegment* pMsg = new CSegment; // pMsg will delete inside
	*pMsg  << (DWORD)strlen(pContent);
	*pMsg  << pContent;

	CXMLDOMElement *pActionNode=NULL,
				   *pSpecActionNode=NULL;

	pRootElement->getChildNodeByName(&pActionNode,"ACTION");
	if (pActionNode)
	{
		pActionNode->getChildNodeByName(&pSpecActionNode,"LOGIN");

		if (pSpecActionNode) // action is 'LOGIN' - send to Manager task
		{
			FTRACESTR(eLevelInfoHigh) << "\nCFailoverRxSocket::TreatResponseTransMcu"
								 << "\nLOGIN transaction is sent to Manager task";

			// send to Manager task
			CManagerApi api(eProcessFailover);
			api.SendMsg(pMsg, FAILOVER_SOCKET_RCV_IND);
			return;
		}
		else
		{
			pActionNode->getChildNodeByName(&pSpecActionNode,"LOGOUT");

			if (pSpecActionNode) // action is 'LOGOUT' - send to Manager task
			{
				FTRACESTR(eLevelInfoHigh) << "\nCFailoverRxSocket::TreatResponseTransMcu"
									 << "\nLOOUT transaction is sent to Manager task";

				// send to Manager task
				CManagerApi api(eProcessFailover);
				api.SendMsg(pMsg, FAILOVER_SOCKET_RCV_IND);
				return;
			}
			else  // action is not 'LOGIN'
			{
				pActionNode->getChildNodeByName(&pSpecActionNode,"RMX_GET_STATE_EX");

				if (pSpecActionNode) // action is 'GET_STATE' - send to FailDetect task
				{
					COsQueue* pMbx = m_pProcess->GetFailDetectionTaskMbx();
					if (pMbx)
					{
						FTRACESTR(eLevelInfoHigh) << "\nCFailoverRxSocket::TreatResponseTransMcu"
											 << "\nRMX_GET_STATE_EX transaction is sent to FailDetection task";

						// send to FailDetect task
						CFailDetectionApi fdApi;
						fdApi.CreateOnlyApi(*pMbx);
						fdApi.SendMsg(pMsg, FAILOVER_SOCKET_RCV_IND);
						return;
					}

					else
					{
						FTRACESTR(eLevelInfoHigh) << "\nCFailoverRxSocket::TreatResponseTransMcu"
											 << "\nNo Mbx for FailDetection task - GET_STATE transaction is not sent";
					}
				} // end if action is 'GET_STATE'

				else // action is neither 'LOGIN' nor 'GET_STATE'
				{
					FTRACESTR(eLevelInfoHigh) << "\nCFailoverRxSocket::TreatResponseTransMcu"
										 << "\nAction is neither LOGIN nor GET_STATE";
				}
			} // end if action is not 'LOGIN'


		} // end 'ACTION' node
	}

	else
	{
		FTRACESTR(eLevelInfoHigh) << "\nCFailoverRxSocket::TreatResponseTransMcu"
							 << "\nAction node is NULL";
	}

	POBJDELETE(pMsg);
}

//////////////////////////////////////////////////////////////////////
void CFailoverRxSocket::TreatResponseOther(CXMLDOMElement* pRootElement, const char *pContent)
{
	CSegment* pMsg = new CSegment; // pMsg will delete inside
	*pMsg  << (DWORD)strlen(pContent);
	*pMsg  << pContent;

	COsQueue* pMbx = m_pProcess->GetFailoverSyncTaskMbx();
	if (pMbx)
	{
		FTRACESTR(eLevelInfoHigh) << "\nCFailoverRxSocket::TreatResponseOther"
							 << "\nThe transaction is sent to FailoverSync task";

		// send to FailoverSync task
		CFailoverSyncApi fsApi;
		fsApi.CreateOnlyApi(*pMbx);
		fsApi.SendMsg(pMsg, FAILOVER_SOCKET_RCV_IND);
	}

	else
	{
		FTRACESTR(eLevelInfoHigh) << "\nCFailoverRxSocket::TreatResponseOther"
							 << "\nNo Mbx for FailoverSync task - the transaction is not sent";					
	}
}
