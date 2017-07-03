//+========================================================================+
//                         GkTaskApi.cpp                              |
//+========================================================================+

#include "GkTaskApi.h"
#include "ProcessBase.h"
#include "McmsProcesses.h"

/////////////////////////////////////////////////////////////////////////////
CGatekeeperTaskApi::CGatekeeperTaskApi(DWORD serviceId,eProcessType process  ) // constructor
{
    COsQueue queue(eServiceId, (int) serviceId, process);
    CreateOnlyApi(queue);

}

/////////////////////////////////////////////////////////////////////////////
CGatekeeperTaskApi::~CGatekeeperTaskApi() // destructor
{
}

