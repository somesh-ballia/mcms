#ifndef GKTOCSINTERFACE_H_
#define GKTOCSINTERFACE_H_

#include "PObject.h"
#include "Segment.h"
#include "MplMcmsProtocol.h"


class CGKToCsInterface : public CPObject
{
CLASS_TYPE_1(CGKToCsInterface, CPObject)     
public: 
	// Constructors
	CGKToCsInterface();
	virtual ~CGKToCsInterface();  
	virtual const char* NameOf() const { return "CGKToCsInterface";}
		
	// Operations
	

    void  SendMsgToCS(OPCODE opcode, CSegment* pSeg, DWORD serviceId, APIS32 status, DWORD connId = 0, DWORD partyId = 0, 
	    				DWORD confId = 0, DWORD callIndex = 0, WORD csId = 0);
   
protected:

	
	// Attributes
	// Operations	
};


#endif /*GKTOCSINTERFACE_H_*/
