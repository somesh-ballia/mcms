/*
 * CSSimCluster.h
 *
 *  Created on: May 9, 2010
 *      Author: drabkin
 */

#ifndef CSSIMCLUSTER_H_
#define CSSIMCLUSTER_H_

#include "PObject.h"
#include "DataTypes.h"
#include "EpSimSipSubscription.h"

class CGKTaskApi;
class CProxyTaskApi;
class CCSSimTaskApi;
class COsQueue;

class CCSSimCluster : public CPObject
{
CLASS_TYPE_1(CCSSimCluster, CPObject)
public:
    CCSSimCluster(DWORD csID, DWORD numPorts,
                  const COsQueue& creator, const COsQueue& eps);
    ~CCSSimCluster(void);
    const char* NameOf(void) const;

    CEpSimSipSubscriptionsMngr& GetSipSubscriptions(void);
    DWORD GetCSID(void) const;
    DWORD GetNumPorts(void) const;
    bool HasCSID(DWORD csID) const;

private:
    // forbid those default operation
    CCSSimCluster(const CCSSimCluster& rhs);
    CCSSimCluster& operator=(const CCSSimCluster& rhs);

    DWORD          m_numPorts;
    CGKTaskApi*    m_pGKTaskApi;
    CProxyTaskApi* m_pProxyApi;
    CCSSimTaskApi* m_pCSApi;
    CEpSimSipSubscriptionsMngr m_sipSubscriptions;
};

#endif /* CSSIMCLUSTER_H_ */
