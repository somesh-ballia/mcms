// RsrvManager.cpp: implementation of the CRsrvManager class.
#include "RsrvManagerApi.h"

class COsQueue;


COsQueue * CRsrvManagerApi::pRsrvMngrQueue = NULL;

///////////////////////////////////////////////////////////////////////////////////
CRsrvManagerApi::CRsrvManagerApi()
								 :CSingleToneApi(eProcessResource,
                                                 "RsrvManager",
                                                 pRsrvMngrQueue)
								 
{	
	m_pRcvMbx->m_process = eProcessResource;

    if (pRsrvMngrQueue == NULL)
        pRsrvMngrQueue = new COsQueue(*m_pRcvMbx);
    
}


///////////////////////////////////////////////////////////////////////////////////
CRsrvManagerApi::~CRsrvManagerApi()
{

}
                  
////////////////////////////////////////////////////////////////////////////////////

