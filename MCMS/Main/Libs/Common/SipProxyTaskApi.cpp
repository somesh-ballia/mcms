//+========================================================================+
//                         GkTaskApi.cpp                              |
//+========================================================================+

#include "SipProxyTaskApi.h"
#include "ProcessBase.h"
#include "McmsProcesses.h"

/////////////////////////////////////////////////////////////////////////////
CSipProxyTaskApi::CSipProxyTaskApi(DWORD serviceId,eProcessType process  ) // constructor
{
    COsQueue queue(eServiceId, (int) serviceId, process);
    CreateOnlyApi(queue);

}

/////////////////////////////////////////////////////////////////////////////
CSipProxyTaskApi::~CSipProxyTaskApi() // destructor
{
}

