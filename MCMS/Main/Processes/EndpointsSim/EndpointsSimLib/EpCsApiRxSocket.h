//+========================================================================+
//                    EpCsApiRxSocket.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpCsApiRxSocket.h                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __EPCSAPIRXSOCKET_H__
#define   __EPCSAPIRXSOCKET_H__


// base class definition
#include "SocketRxTask.h"
#include "SocketApi.h"

//class CMplMcmsProtocol;


// task entry point
extern "C" void EpCsApiRxEntryPoint(void* appParam);


class CEpCsApiRxSocket : public CSocketRxTask
{
CLASS_TYPE_1(CEpCsApiRxSocket,CSocketRxTask )
public:

				// Constructors
	CEpCsApiRxSocket();
	virtual const char* NameOf() const { return "CEpCsApiRxSocket";}
	virtual ~CEpCsApiRxSocket();
	
				// Initializations
	void* GetMessageMap(); 

				// Operations
	const char * GetTaskName() const;

	virtual void ReceiveFromSocket();

protected:
				// base overriding
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();

				// Operations

				// Action functions

				// Attributes

	PDECLAR_MESSAGE_MAP
};



#endif /* __EPCSAPIRXSOCKET_H__ */
