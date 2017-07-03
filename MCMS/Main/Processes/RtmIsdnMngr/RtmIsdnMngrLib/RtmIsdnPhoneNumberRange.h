#ifndef CISDNPHONENUMBERRANGE_H_
#define CISDNPHONENUMBERRANGE_H_




#include "SerializeObject.h"
#include "NStream.h"
#include "StringsLen.h"
#include "psosxml.h"
#include "RtmIsdnMngrInternalStructs.h"



/////////////////////////////////////////////////////////////////////////////
// CRtmIsdnPhoneNumberRange
class CRtmIsdnPhoneNumberRange : public CSerializeObject
{
CLASS_TYPE_1(CRtmIsdnPhoneNumberRange,CPObject)
public:
	   //Constructors
	CRtmIsdnPhoneNumberRange();
	virtual ~CRtmIsdnPhoneNumberRange();


	   // Implementation
    void   Serialize(WORD format, COstrStream  &m_ostr);     
    void   DeSerialize(WORD format, CIstrStream &m_istr);    
	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int	   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	CSerializeObject* Clone() {return new CRtmIsdnPhoneNumberRange();}
    const char*  NameOf() const;             
	
    WORD   GetDialInGroupId() const;                 
    void   SetDialInGroupId(const WORD dialInGroupId);    
	
    const char*  GetFirstPhoneNumber () const; 
    void  SetFirstPhoneNumber(const char* phoneNumber);

    const char*  GetLastPhoneNumber () const; 
    void  SetLastPhoneNumber(const char* phoneNumber);

    DWORD GetCategory() const;
    void  SetCategory(const DWORD  category);

    BYTE  GetFirstPortId() const;
    void  SetFirstPortId(const BYTE  portId);
    
    bool  IsMaxNumbersInRangeExceeded() const;

    void  SetParams(RTM_ISDN_PHONE_RANGE_S &phoneRangeStruct);


public:
	   // Attributes					
    WORD    m_dialInGroupId;
    char    m_firstPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN];  
    char    m_lastPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN];  
    DWORD   m_category;
    BYTE    m_firstPortId;

};






















#endif /*CISDNPHONENUMBERRANGE_H_*/
