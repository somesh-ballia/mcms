//+========================================================================+
//                            CSApiRxSocketMngr.h                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CSApiRxSocketMngr.h                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 04/04/2005 |                                                      |
//+========================================================================+



#include "OsQueue.h"
#include "MplMcmsProtocol.h"



class CCSApiMplMcmsProtocolTracer;


class CCSApiRxSocketMngr : public CPObject 
{
CLASS_TYPE_1(CCSApiRxSocketMngr,CPObject)
public:             
	
	// Constructors
	CCSApiRxSocketMngr();    
	virtual ~CCSApiRxSocketMngr();  
	
	// Initializations
	const char * GetTaskName() const ;
	const char * NameOf(void) const {return "CCSApiRxSocketMngr";}
	DWORD SendCSEventToTheAppropriateProcess(CMplMcmsProtocol& CSPrtcl);

	const COsQueue * GetDestinationProcessQueueByOpcode (DWORD opcode);
	
	
private:  
	
	CCSApiMplMcmsProtocolTracer *m_CSApiMplMcmsProtocolTracer;
};


