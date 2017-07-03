#ifndef _NETCALLEDPARTY
#define _NETCALLEDPARTY


#include "PObject.h"
#include "Segment.h"
#include "NetCallingParty.h"

class CNetCalledParty : public CPObject
{
	CLASS_TYPE_1(CNetCalledParty, CPObject)	
public:             
	// Constructors
	CNetCalledParty();
	~CNetCalledParty();  
	virtual const char* NameOf() const { return "CNetCalledParty";}
	// Initializations  
	
	// Serialization
	void Serialize(WORD format,CSegment& seg);
	void DeSerialize(WORD format,CSegment& seg);
	WORD Legalize(); // remove illegal characters from number
	void Trace(std::ostream& msg);
	
	// Attributes              
	
	BYTE 	m_numDigits;  //Number of digits.       
	BYTE    m_numType;
	BYTE  	m_numPlan;
	BYTE    m_digits[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
	    
};
#endif //_NETCALLEDPARTY
