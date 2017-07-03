/*
 * SvcPartyIndParamsWrapper.h
 *
 *  Created on: Nov 5, 2012
 *      Author: asilver
 */

#ifndef _SVCPARTYINDPARAMSWRAPPER_H_
#define _SVCPARTYINDPARAMSWRAPPER_H_

#include <list>
#include "Segment.h"
#include "MrcStructs.h"
#include "AllocateStructs.h"
#include "RvCommonDefs.h"

class CSvcPartyIndParamsWrapper : public CPObject
{
    CLASS_TYPE_1(CSvcPartyIndParamsWrapper, CPObject)

public:
    CSvcPartyIndParamsWrapper();
    virtual ~CSvcPartyIndParamsWrapper() {}

    CSvcPartyIndParamsWrapper& operator= (const CSvcPartyIndParamsWrapper &other);
    CSvcPartyIndParamsWrapper& operator= (const SVC_PARTY_IND_PARAMS_S &other);

    virtual const char* NameOf() const { return "CSvcPartyIndParamsWrapper"; }
    void  Print(const char *title="") const;

    virtual void  Serialize(CSegment& seg) const;
    virtual void  DeSerialize(CSegment& seg);

    DWORD GetSsrcId(int ind, cmCapDataType aDataType);

//private:
    SVC_PARTY_IND_PARAMS_S m_SsrcIds;
};


#endif /* _SVCPARTYINDPARAMSWRAPPER_H_ */
