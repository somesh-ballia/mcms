//+========================================================================+
//                            StructTm.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       StructTm.h                                                  |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Anatoly                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef _STRUCT_TM__
#define _STRUCT_TM__

#include <time.h>
#include "PObject.h"
#include "Segment.h"
#include "StructTm.h"

/////////////////////////////////////////////////////////////////////////////
// CStructTm

class CStructTm : public CPObject
{
CLASS_TYPE_1(CStructTm,CPObject )
public:
	   //Constructors
    CStructTm();
    CStructTm(const CStructTm &other);
    CStructTm(const tm &tm_other);
    CStructTm(int hour,
			  int min,
			  int sec);
    CStructTm(int day,
			  int mon,
			  int year,
			  int hour,
			  int min,
			  int sec = 0);

    virtual ~CStructTm();
    
    virtual const char* NameOf() const { return "CStructTm";} 
	// Implementation
	void  InitDefaults();
    void  DeSerialize(std::istream &m_ostr);
    void  Serialize(std::ostream &m_ostr);
    void  SerializeV2(std::ostream &m_ostr);
    void Serialize(WORD format,CSegment& seg);
    void DeSerialize(WORD format,CSegment& seg);
    void  LongSerialize(std::ostream &m_ostr);
    void SerializeBilling(std::ostream &m_ostr);
    void DeSerializeBilling(std::istream &m_istr);
    void SerializeCdr(std::ostream &m_ostr);
    void DeSerializeCdr(std::istream &m_istr);
    void SerializeSNMP(std::ostream &m_ostr);
//    void DeSerializeSNMP(std::istream &m_istr);

    void  DumpToBuffer(char *buffer) const;

    int    IsValid() const;
	int    IsValidForCdr() const;
    void   GetAsTm(tm& tm_other) const;
	BYTE   GetAndVerifyAsTm(tm& tm_other);
	void   AddReferenceTime(const CStructTm &referenceTime);

    CStructTm&   operator=(const CStructTm& other);
    int          operator<=(const CStructTm& other) const;
    int          operator>=(const CStructTm& other) const;

    CStructTm    GetTimeDelta(const CStructTm& other) const;

    friend int operator==(const CStructTm&,const CStructTm&) ;
    friend int operator!=(const CStructTm&,const CStructTm&) ;
    friend int operator<(const CStructTm&,const CStructTm&) ;
    friend int operator>(const CStructTm&,const CStructTm&) ;

    DWORD operator-(const CStructTm& other) const;

	operator time_t() const;

  	static size_t Sizeof()// should be the amount Serialized to a file
	{
	    return 6 * sizeof(int);
    }

	void Dump(std::ostream& msg) const ;
	friend std::ostream& operator<< (std::ostream& os, const CStructTm& obj );

	time_t GetAbsTime(BOOL isGMT = FALSE)const;
	void   SetAbsTime(time_t absTime);
	void   SetGmtTime() ;

	// Attributes
    int m_sec;   //0..59
    int m_min;   //0..59
    int m_hour;  //0..23
    int m_day;   //1..31
    int m_mon;   //0..11
    int m_year;  //Years since 1900

	static WORD IsPeriodOverLapping(
		const CStructTm& Time1Start,
		const CStructTm& Time1End,
		const CStructTm& Time2Start,
		const CStructTm& Time2End);

};

//CStructTm& operator-(const CStructTm& l,const CStructTm& r);
CStructTm operator+(const CStructTm& l,const CStructTm& r);

int operator==(const CStructTm&,const CStructTm&);
int operator!=(const CStructTm&,const CStructTm&);
int operator<(const CStructTm&,const CStructTm&);
int operator>(const CStructTm&,const CStructTm&);


#endif /* _STRUCT_TM__ */

