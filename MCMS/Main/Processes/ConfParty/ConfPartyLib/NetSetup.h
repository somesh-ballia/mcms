
#ifndef _NETSETUP
#define _NETSETUP

#include "PObject.h"
#include "Segment.h"
#include "NetCallingParty.h"
#include "NetCalledParty.h"

class CNetSetup : public CPObject
{
CLASS_TYPE_1(CNetSetup,CPObject )
   public:             
								// Constructors
	CNetSetup();
	~CNetSetup(); 
	virtual const char* NameOf() const { return "CNetSetup";}
	virtual void  Serialize(WORD format,CSegment& seg) = 0;
	virtual void  DeSerialize(WORD format,CSegment& seg) = 0	;
	virtual void  copy(	const CNetSetup *rhs) = 0; 
		
	
	DWORD m_callType; 
};
#endif //_NETSETUP


