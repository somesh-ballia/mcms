#ifndef _NETCAUSE
#define _NETCAUSE

#include "PObject.h"
#include "Segment.h"

class CNetCause : public CPObject
{
CLASS_TYPE_1(CNetCause, CPObject)
public:             
								// Constructors
	CNetCause();
	~CNetCause();  
	virtual const char* NameOf() const { return "CNetCause";}
								// Initializations  

	                            // Serialization
	void Serialize(WORD format,CSegment& seg);
	void DeSerialize(WORD format,CSegment& seg);
	void Trace(std::ostream& msg);


	
                                // Attributes              

	BYTE m_codingStandard;
	BYTE m_location;
	BYTE m_causeVal;
	    
protected:
};

#endif //_NETCAUSE

