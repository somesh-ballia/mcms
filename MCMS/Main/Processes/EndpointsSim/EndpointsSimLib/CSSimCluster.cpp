/*
 * CSSimCluster.cpp
 *
 *  Created on: May 9, 2010
 *      Author: drabkin
 */

#include "CSSimCluster.h"

#include "TraceStream.h"
#include "EndpointsSim.h"
#include "EndpointsSimConfig.h"
#include "CSSimTaskApi.h"
#include "ProxyTaskApi.h"
#include "GKTaskApi.h"
#include "OpcodesMcmsInternal.h"

extern CEndpointsSimSystemCfg* GetEpSystemCfg();
extern "C" void epSimCSEntryPoint(void* appParam);
extern "C" void epSimProxyEntryPoint(void* appParam);
extern "C" void epSimGKeeperEntryPoint(void* appParam);

CCSSimCluster::CCSSimCluster(DWORD csID, DWORD numPorts,
                             const COsQueue& creator, const COsQueue& eps) :
        m_numPorts(numPorts),
        m_pGKTaskApi(new CGKTaskApi(csID)),
        m_pProxyApi(new CProxyTaskApi(csID)),
        m_pCSApi(new CCSSimTaskApi(csID))

{
    TRACEINTO << "CS["
              << GetCSID()
              << "] cluster created";

    m_pGKTaskApi->Create(epSimGKeeperEntryPoint, creator);
    m_pProxyApi->Create(epSimProxyEntryPoint, creator);
    m_pCSApi->Create(epSimCSEntryPoint, creator);

    m_sipSubscriptions.Init(m_pCSApi);

    CSegment* msg1 = new CSegment;
    eps.Serialize(*msg1);
    m_pCSApi->SendMsg(msg1, UPDATE_ENDPS_MBX);

    CSegment* msg2 = new CSegment;
    *msg2 << ::GetEpSystemCfg()->GetCsApiIpAddress()
          << ::GetEpSystemCfg()->GetCsApiPortNumber();
    m_pCSApi->SendMsg(msg2, CONNECT_CS_SOCKET);
}

CCSSimCluster::~CCSSimCluster(void)
{
    TRACEINTO << "CS["
              << GetCSID()
              << "] cluster removed";

    m_pGKTaskApi->Destroy();
    delete m_pGKTaskApi;

    m_pProxyApi->Destroy();
    delete m_pProxyApi;

    m_pCSApi->Destroy();
    delete m_pCSApi;
}

const char* CCSSimCluster::NameOf(void) const
{
    return GetCompileType();
}

CEpSimSipSubscriptionsMngr& CCSSimCluster::GetSipSubscriptions(void)
{
    return m_sipSubscriptions;
}

DWORD CCSSimCluster::GetCSID(void) const
{
    return m_pCSApi->GetCSID();
}

DWORD CCSSimCluster::GetNumPorts(void) const
{
    return m_numPorts;
}

bool CCSSimCluster::HasCSID(DWORD csID) const
{
    return GetCSID() == csID;
}
