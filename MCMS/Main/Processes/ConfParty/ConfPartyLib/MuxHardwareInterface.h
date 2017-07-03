//+========================================================================+
//                     MuxHardwareInterface.h                              |
//		             Copyright 2005 Polycom, Inc                           |
//                       All Rights Reserved.                              |
//-------------------------------------------------------------------------|
// FILE:       MuxHardwareInterface.h                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Olga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  11-2007  | Description                                     |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _MUX_INTERFACE_H_
#define _MUX_INTERFACE_H_

#include "HardwareInterface.h"


class CCapH320;
class CSegment;

class CMuxHardwareInterface : public CHardwareInterface
{
	CLASS_TYPE_1(CMuxHardwareInterface, CHardwareInterface)
        
public:
    // constructors
    CMuxHardwareInterface();
    CMuxHardwareInterface(CRsrcParams& rsrcDesc);
    virtual ~CMuxHardwareInterface();
    
	virtual const char* NameOf() const;
	
	void  InitComm(WORD numChnl, WORD channelWidth, CCapH320 &localCap,
				   CSegment& initialXmitMode, CSegment& initialH230, WORD restrictMode);

	void  RestrictMode(WORD restrict_type);
	void  SetXmitRcvMode(CComMode& rcomMode, DWORD mode, WORD bitRateFlag = 0, BYTE isH239 = 0);
//	void  SendAudioComfortNoiseRequest(BYTE comfortNoiseOnOff);
	void  SendECS(CSegment& ECSString);
	void  SendH230(CSegment& h230String);
	void  SetRepeatedH230(CSegment& h230String);
	void  ExchangeCap(CCapH320& localCap, WORD IsH263_2000Cap);
	void  KillConnection();
	void  Destroy();

private:
    
    
    
};


#endif // _MUX_INTERFACE_H_
