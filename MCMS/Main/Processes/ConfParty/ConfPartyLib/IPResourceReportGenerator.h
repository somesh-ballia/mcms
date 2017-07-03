//+========================================================================+
//                IPResorceReportGenerator.h                               |
//            Copyright 2006 Polycom Ltd.                                  |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.                    |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Ltd.                           |
//-------------------------------------------------------------------------|
// FILE:       IPResorceReportGenerator.h                                  |                                    
// PROGRAMMER: Lior Baram                                                  |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 18.12.07   |                                                      |
//+========================================================================+ 

#ifndef _CIPRSRCGEN
#define _CIPRSRCGEN

#include "CommResApi.h"
#include "CommResAdd.h"
#include "CommRes.h"
#include "CommResShort.h"
#include "IPAdHocProfilesReport.h"
#include "PObject.h"

class CIPResourceGenerator : public CPObject
{
    CLASS_TYPE_1(CIPResourceGenerator,CPObject)
public:
    // Constructors
    CIPResourceGenerator() {};
    
	virtual const char* NameOf() const { return "CIPResourceGenerator";}
    //Destructor
    virtual ~CIPResourceGenerator() {};

    void GenerateAdHocProfilesReport (CIPAdHocProfilesReport * pAdhocProfilesResourceReport, ALLOC_REPORT_PARAMS_S* pResult, CCommResDB * pResDB, CCommResDB * pProfilesDB);
};


#endif // _CIPRSRCGEN
