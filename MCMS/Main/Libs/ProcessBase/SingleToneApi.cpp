// SingleToneApi.cpp: implementation of the CSingleToneApi class.
//
//////////////////////////////////////////////////////////////////////

#include "SingleToneApi.h"
#include "StatusesGeneral.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSingleToneApi::CSingleToneApi(const eProcessType processType,
                               const char * taskName,
                               COsQueue * pQueue)
{	
	m_pRspMbx       = NULL;
	m_pLocalRcvMbx  = NULL;
	m_pTaskApp      = NULL;
	m_lastSendError = STATUS_OK;

    if (pQueue == NULL)
    {
        m_pRcvMbx = new COsQueue;
        m_pRcvMbx->CreateWrite(processType,taskName);
        m_pRcvMbx->m_process = (eProcessType) -1;
    }
    else
    {
        m_pRcvMbx = new COsQueue(*pQueue);
    }
}

CSingleToneApi::~CSingleToneApi()
{

}

void CSingleToneApi::CleanUp()
{
	m_pRcvMbx->Delete();
}

