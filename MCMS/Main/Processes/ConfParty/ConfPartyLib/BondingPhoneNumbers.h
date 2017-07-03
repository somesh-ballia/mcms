//+========================================================================+
//                   BondingCntl.H                                     |
//		     Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       BondingCntl.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Ron                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  10-2007  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _BONDING_PHONE_NUMBERS_H_
#define _BONDING_PHONE_NUMBERS_H_


//==============================================================================================================//
#include "PObject.h"
#include "Bonding.h"
#include <vector>

class BondingPhoneNumber;
class CSegment;


template class std::vector < BondingPhoneNumber* > ;
typedef std::vector< BondingPhoneNumber*> BondingPhonesVector;

//==============================================================================================================//

class BondingPhoneNumber : public CPObject
{
    CLASS_TYPE_1(BondingPhoneNumber,CPObject)
public:
// constructors
    BondingPhoneNumber();
    explicit BondingPhoneNumber(char* phone_number);
    BondingPhoneNumber(const BondingPhoneNumber& other);
    BondingPhoneNumber& operator=(const BondingPhoneNumber& other);
    
    
    virtual ~BondingPhoneNumber();
// CPObject pure virtual
    const char*   NameOf() const;
    WORD CopyToBuffer(char* Buffer)const;
   void Dump(std::ostringstream& str)const;

    void Serialize(WORD format, CSegment &seg);
    void Deserialize(WORD format, CSegment &seg);

    void Emb2String();
    void String2Emb();
    

private:
// attributes
    char m_digits[BND_MAX_PHONE_LEN+1];
};

//==============================================================================================================//

class BondingPhoneNumbersList : public CPObject
{
    CLASS_TYPE_1(BondingPhoneNumbersList,CPObject)
        public:
// constructors
    BondingPhoneNumbersList();
    virtual ~BondingPhoneNumbersList();
// CPObject pure virtual
    const char*   NameOf() const;

// api functions
	void CleanPhoneList();//rons
    DWORD AddPhoneNumber(char* phone_number,int emb2str = 0);
    DWORD GetNumOfPhoneNumbers()const;
    BondingPhoneNumber* GetFirstOfPhoneNumbers()const;
    //const BondingPhoneNumber*& operator[](size_t index)const;
    
// serialize and dump functions    
    void Serialize(WORD format, CSegment &seg);
    void Deserialize(WORD format, CSegment &seg);
    void Dump(std::ostringstream& str)const;

    
// attributes
    // we use the vector methods, so we keep the data member public
    BondingPhonesVector m_phone_numbers_vector;
    
};

//==============================================================================================================//

#endif // _BONDING_PHONE_NUMBERS_H_
