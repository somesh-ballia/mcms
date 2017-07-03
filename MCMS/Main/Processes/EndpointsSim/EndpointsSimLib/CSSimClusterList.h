/*
 * CSSimClusterList.h
 *
 *  Created on: May 25, 2010
 *      Author: drabkin
 */

#ifndef CSSIMCLUSTERLIST_H_
#define CSSIMCLUSTERLIST_H_

#include <list>
#include <ostream>
#include "PObject.h"
#include "DataTypes.h"

class COsQueue;
class CCSSimCluster;

class CCSSimClusterList: public CPObject
{
CLASS_TYPE_1(CCSSimClusterList, CPObject)
public:
    CCSSimClusterList(void);
    ~CCSSimClusterList(void);
    const char* NameOf(void) const;

    void Clear(void);
    int Add(DWORD csID, DWORD numPorts,
            const COsQueue& creator, const COsQueue& eps);
    int Remove(DWORD csID);
    CCSSimCluster* Get(DWORD csId) const;
    void PrintOut(std::ostream& out) const;

private:
    // forbid those default operation
    CCSSimClusterList(const CCSSimClusterList& rhs);
    CCSSimClusterList& operator=(const CCSSimClusterList& rhs);

    std::list<CCSSimCluster*>::const_iterator FindElement(DWORD csID) const;

    std::list<CCSSimCluster*> m_list;
};

#endif /* CSSIMCLUSTERLIST_H_ */
