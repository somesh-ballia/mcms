//+========================================================================+
//                    SimCardRxSocket.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SimCardRxSocket.cpp                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include "QAAPIRxSocket.h"
#include "psosxml.h"
#include "HTTPPars.h"
#include "PartyIdTaskApi.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "EncodingConvertor.h"
#include "UnicodeDefines.h"
#include "SecuredSocketConnected.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CSimCardRxSocket
//
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CQAAPIRxSocket)
ONEVENT( SOCKET_WRITE, ANYCASE,  CStateMachine::NullActionFunction)
PEND_MESSAGE_MAP(CQAAPIRxSocket,CSocketRxTask);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void QAAPIRxEntryPoint(void* appParam)
{
	BYTE bSecured;
	CSegment* pSeg = (CSegment*)appParam;
	*pSeg >> (BYTE&)bSecured;
	pSeg->ResetRead();

	if (bSecured == TRUE)
	{
		FTRACEINTO << "\nCQAAPIRxSocket::QAAPIRxEntryPoint - secured connection";
		CQAAPIRxSocket*  pRxSocket = new CQAAPIRxSocket(new CSecuredSocketConnected);
		pRxSocket->Create(*(CSegment*)appParam);
	}
	else
	{
		FTRACEINTO << "\nCQAAPIRxSocket::QAAPIRxEntryPoint - not-secured connection";
		CQAAPIRxSocket*  pRxSocket = new CQAAPIRxSocket(new COsSocketConnected);
		pRxSocket->Create(*(CSegment*)appParam);
	}
}

/////////////////////////////////////////////////////////////////////////////
CQAAPIRxSocket::CQAAPIRxSocket(COsSocketConnected * pSocketDesc)
:CSocketRxTask(pSocketDesc), 
 CHTTPReceiver(TRUE) 	// constructor
{
	
}

/////////////////////////////////////////////////////////////////////////////
CQAAPIRxSocket::~CQAAPIRxSocket()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void CQAAPIRxSocket::InitTask()
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CQAAPIRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
const char* CQAAPIRxSocket::GetTaskName() const
{
	return "QAAPIRxSocketTask";
}


//////////////////////////////////////////////////////////////////////
void CQAAPIRxSocket::ReceiveFromSocket()
{
    if(ReadHttp(*this))
    {
        const char *pHttpHeader = GetHttpHeader();
        string charSet;
        CHTTPHeaderParser::GetContentCharset(pHttpHeader, charSet);
        if(false == charSet.empty())
        {
            bool isKnown = CEncodingConvertor::IsKnownEncoding(charSet);
            if(false == isKnown)
            {
                // drop the packet
                FTRACEINTO << "\nCQAAPIRxSocket::ReceiveFromSocket\nDrop the packet\nthe encoding is unknown: " << charSet.c_str();
                return;
            }
        }
        
		char *pTmpContent = GetHttpContent();
        DWORD tmpContentLen = strlen(pTmpContent);
        char *pContent = new char[strlen(pTmpContent) + 1];
        strcpy(pContent, pTmpContent);
        pContent[tmpContentLen] = 0;
        
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
            FTRACEINTO << "\nCQAAPIRxSocket::ReceiveFromSocket Drop the packet\n"
                       << statusString.str().c_str() << "\n"
                       << pContent;
            PDELETEA(pContent);
            return;
        }
        
		int status=0;
		CHTTPHeaderParser::GetHTTPResponseStatus(m_pHttpHeader,&status);
		if (status==200 && pContent)
		{
			FTRACESTR(eLevelInfoHigh) << "CQAAPIRxSocket::ReceiveFromSocket - reply from external server - content... " << pContent;
            CXMLDOMDocument *pDom=new CXMLDOMDocument;
            if (pDom->Parse((const char **)&pContent)==SEC_OK)
            {
                CXMLDOMNode * pNode     = NULL;
				CXMLDOMElement * pActionNode     = NULL;
                CXMLDOMElement *pRoot=pDom->GetRootElement();
                if(pRoot)
                {
					pRoot->getChildNodeByName(&pActionNode,"ACTION");
					if (pActionNode)
					{
						
						pActionNode->get_firstChild(&pNode);
						if (pNode)
						{
							char *pName=NULL;
							pNode->get_nodeName(&pName);
							if(pName)
							{
								if (strcmp(pName,"KEEP_ALIVE")==0)
								{
									const COsQueue*pQ=CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessQAAPI, eManager);
									
									CSegment* pMsg = new CSegment; // pMsg will delete inside
									*pMsg  << (DWORD)strlen(pContent);
									*pMsg  << pContent;
									
									
									CTaskApi api;
									api.CreateOnlyApi(*pQ);
									api.SendMsg(pMsg,EXT_DB_RESPONSE);
									api.DestroyOnlyApi();
								}
								else if (strcmp(pName,"ADD")==0)
								{
									int token;
									FTRACESTR(eLevelInfoNormal) << "CQAAPIRxSocket::ReceiveFromSocket in add";
									if (pRoot->getChildNodeDecValueByName("TOKEN",&token)==SEC_OK)
									{
										CSegment* pMsg = new CSegment; // pMsg will delete inside
										*pMsg  << (DWORD)EXT_DB_PWD_CONFIRM;
										*pMsg  << (DWORD)strlen(pContent);
										*pMsg  << pContent;
										
										CPartyIdTaskApi api(token,eProcessConfParty);
										api.SendMsg(pMsg,XML_EVENT);
										api.DestroyOnlyApi();
										FTRACESTR(eLevelInfoNormal) << "CQAAPIRxSocket::ReceiveFromSocket in add , send";
									}
									else
									{
										FTRACESTR(eLevelInfoHigh) << "CQAAPIRxSocket::ReceiveFromSocket - token is missing - content... " << pContent;
									}
								}
								else if (strcmp(pName,"CREATE")==0)
								{
									int token;
									FTRACESTR(eLevelInfoNormal) << "CQAAPIRxSocket::ReceiveFromSocket in CREATE";
									if (pRoot->getChildNodeDecValueByName("TOKEN",&token)==SEC_OK)
									{
										CSegment* pMsg = new CSegment; // pMsg will delete inside
										*pMsg  << (DWORD)EXT_DB_CONF_CONFIRM;
										*pMsg  << (DWORD)strlen(pContent);
										*pMsg  << pContent;
										
										CPartyIdTaskApi api(token,eProcessConfParty);
										api.SendMsg(pMsg,XML_EVENT);
										api.DestroyOnlyApi();
										FTRACESTR(eLevelInfoNormal) << "CQAAPIRxSocket::ReceiveFromSocket in Create , send";
									}
									else
									{
										FTRACESTR(eLevelInfoHigh) << "CQAAPIRxSocket::ReceiveFromSocket - token is missing - content... " << pContent;
									}
								}
								else if (strcmp(pName,"AUTHENTICATE")==0)
								{
									int token;
									if (pRoot->getChildNodeDecValueByName("TOKEN",&token)==SEC_OK)
									{
										CSegment* pMsg = new CSegment; // pMsg will delete inside
										*pMsg  << (DWORD)strlen(pContent);
										*pMsg  << pContent;
										
										CTaskApi api;
										const COsQueue* pAuthenticationManager = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessAuthentication,eManager);
										api.CreateOnlyApi(*pAuthenticationManager);
										//ResponedClientRequest(STATUS_OK,pMsg);
										api.SendMsg(pMsg,EXT_DB_USER_LOGIN_CONFIRM);
										//api.DestroyOnlyApi();
									}
									else
									{
										FTRACESTR(eLevelInfoNormal) << "CQAAPIRxSocket::ReceiveFromSocket - token is missing - content... " << pContent;
									}
								}	
							}		
						}
					}
                }
            }
            delete pDom;			
		}
		if(status!=200)
		{	
			FTRACESTR(eLevelInfoHigh) << "CQAAPIRxSocket::ReceiveFromSocket - bad reply from external server: status =  " << status;
			if(pContent)
				FTRACESTR(eLevelInfoHigh) << "CQAAPIRxSocket::ReceiveFromSocket - bad reply from external server - content... " << pContent;
		}

        PDELETEA(pContent);
    }
    else
    	FTRACESTR(eLevelInfoHigh) << "CQAAPIRxSocket::ReceiveFromSocket - bad reply from external server. fail to read the response.";
}











