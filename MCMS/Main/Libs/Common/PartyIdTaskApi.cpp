//+========================================================================+
//                         PartyIdTaskApi.cpp                              |
//+========================================================================+

#include "PartyIdTaskApi.h"
#include "ProcessBase.h"
#include "McmsProcesses.h"

/////////////////////////////////////////////////////////////////////////////
CPartyIdTaskApi::CPartyIdTaskApi(DWORD partyId,eProcessType process  ) // constructor
{
    COsQueue queue(ePartyId, (int) partyId, process);
    CreateOnlyApi(queue);

}

/////////////////////////////////////////////////////////////////////////////
CPartyIdTaskApi::~CPartyIdTaskApi() // destructor
{
}

