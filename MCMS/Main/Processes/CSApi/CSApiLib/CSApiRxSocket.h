//+========================================================================+
//                            CSApiRxSocket.h                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CSApiRxSocket.h                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 07/06/05     |                                                    |
//+========================================================================+


#ifndef _CSAPIRXSOCKET_H__
#define _CSAPIRXSOCKET_H__

#include "SocketRxTask.h"
#include "MplMcmsProtocol.h"
#include "DataTypes.h"
#include "CSApiDefines.h"

extern "C" void CSApiSocketRxEntryPoint(void* appParam);



class CCSApiRxSocket : public CSocketRxTask
{
CLASS_TYPE_1(CCSApiRxSocket,CSocketRxTask )
public:
	CCSApiRxSocket();
	virtual ~CCSApiRxSocket();
	const char * NameOf(void) const {return "CCSApiRxSocket";}
	const char * GetTaskName() const {return "CSApiRxSocket";}
	void InitTask();
	BOOL  IsSingleton() const {return NO;}

	void HandleDisconnect();
	void ReceiveFromSocket();

private:
	BOOL  IsKnownCsID();
	void  SetKnownCsID(BOOL knownCsID);
	STATUS ReadMainBuff		(CMplMcmsProtocol &CSPrtcl, DWORD MainBufLen);
	STATUS XmlToBinary		(char *Mainbuf, char *& BinaryMainbuf, int xmlMsgLen, int &binaryMsgSize)const;
	void UpdateCsId(CMplMcmsProtocol &csPrtcl);
	eCommProtocol GetCurrentProtocol(char testChar);
	bool IsXTimeHere(WORD time);
	virtual void AddFilterOpcodePoint();
	
	BOOL m_IsKnownCsID;
};


#endif /* _CSAPIRXSOCKET_H__ */
