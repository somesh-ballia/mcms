

#ifndef _NETCALLINGPARTY
#define _NETCALLINGPARTY

#define PRI_LIMIT_PHONE_DIGITS_LEN 24
#define PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER (PRI_LIMIT_PHONE_DIGITS_LEN+1)

#include "PObject.h"
#include "Segment.h"

class CNetCallingParty : public CPObject
{
	CLASS_TYPE_1(CNetCallingParty, CPObject)	
public:             
	// Constructors
	CNetCallingParty();
	~CNetCallingParty();  
	virtual const char* NameOf() const { return "CNetCallingParty";}
	// Initializations  
	
	// Serialization
	void Serialize(WORD format,CSegment& seg);
	void DeSerialize(WORD format,CSegment& seg);
	
	void Trace(std::ostream& msg);
	
	// Attributes              
	
	BYTE 	m_numDigits;  //Number of digits.       
	BYTE    m_numType;
	BYTE  	m_numPlan;
	BYTE    m_presentationInd; 
	BYTE    m_screeningInd; 
	BYTE    m_digits[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
	
};
#endif//_NETCALLINGPARTY


