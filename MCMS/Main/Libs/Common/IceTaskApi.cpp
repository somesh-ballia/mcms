//+========================================================================+
//                         IceTaskApi.cpp                              |
//+========================================================================+

#include "IceTaskApi.h"
#include "ProcessBase.h"
#include "McmsProcesses.h"

/////////////////////////////////////////////////////////////////////////////
CIceTaskApi::CIceTaskApi(DWORD serviceId,eProcessType process  ) // constructor
{
    COsQueue queue(eServiceId, (int) serviceId, process);
    CreateOnlyApi(queue);

}

/////////////////////////////////////////////////////////////////////////////
CIceTaskApi::~CIceTaskApi() // destructor
{
}

