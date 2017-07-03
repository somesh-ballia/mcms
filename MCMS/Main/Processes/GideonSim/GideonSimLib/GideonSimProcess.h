//+========================================================================+
//                    GideonSimProcess.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimProcess.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimProcess.h: interface for the CGideonSimProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_GideonSimPROCESS_H__)
#define _GideonSimPROCESS_H__

#include "ProcessBase.h"

class CGideonSimProcess : public CProcessBase  
{
CLASS_TYPE_1(CGideonSimProcess,CProcessBase )
public:
	friend class CTestGideonSimProcess;

	CGideonSimProcess();
	virtual const char* NameOf() const { return "CGideonSimProcess";}
	virtual ~CGideonSimProcess();
	virtual eProcessType GetProcessType() { return eProcessGideonSim; }
	virtual TaskEntryPoint GetManagerEntryPoint();
    virtual BOOL HasMonitorTask() { return FALSE; }
	virtual BOOL UsingSockets() { return YES; }
    virtual BOOL HasWatchDogTask() {
        eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
        if (eProductFamilyCallGenerator == curProductFamily)
        	return TRUE;
    	else
    		return FALSE;
    	}

    virtual int GetProcessAddressSpace() {return 0;}

};

#endif // !defined(_GideonSimPROCESS_H__)

