//+========================================================================+
//                             CapNS.H                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CapNS.H                                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 14.10.07   |                                                      |
//+========================================================================+


#ifndef _H320CAPNS_H
#define _H320CAPNS_H

#include "PObject.h"


#define MANUFACT_CODE_PUBLIC_PP_BYTE_1			0x50
#define MANUFACT_CODE_PUBLIC_PP_BYTE_2			0x50

class CSegment;


class CCapNonStandardItem : public CPObject
{
CLASS_TYPE_1(CCapNonStandardItem,CPObject )
public :
					// Constructors
	CCapNonStandardItem();
	CCapNonStandardItem(const CCapNonStandardItem& other);
	virtual ~CCapNonStandardItem();

					// Operations
	virtual const char*  NameOf() const;
	virtual void   Dump(std::ostream& ostr) const;

	void  Serialize(WORD format,CSegment &seg);
	void  DeSerialize(WORD format,CSegment &seg);

	WORD  Create(const CCapNonStandardItem& other);
	WORD  Create(CSegment& seg);
	WORD  Create(const BYTE msgLen,const BYTE country1,const BYTE country2,
				const BYTE manuf1,const BYTE manuf2,const BYTE* capData);
	WORD  operator==(const CCapNonStandardItem& Item) const;

	BYTE  GetMessageLen()   const { return m_msgLen; }
	WORD  GetCountryCode()  const { return m_countryCode; }
	WORD  GetManufactCode() const { return m_manufactCode; }
	BYTE* GetpCapDataBytes() const { return m_pCapDataBytes; }

	BYTE  RemoveBytesFromDataArray( const BYTE indexFrom, const BYTE howMany );
	BYTE  AppendBytesToDataArray( const BYTE* pDataBytes, const BYTE numBytes );

protected :

	BYTE  m_msgLen;          // message length
	WORD  m_countryCode;     // country code by T.35 standard
	WORD  m_manufactCode;    // manufacturer code
	BYTE* m_pCapDataBytes;   // data bytes
};


/* NonStandard capability description */
class CCapNS : public CPObject
{
CLASS_TYPE_1(CCapNS,CPObject )
public :
					// Constructors
	CCapNS();
	virtual ~CCapNS();

					// Operations
	virtual const char*  NameOf() const;
	virtual void   Dump(std::ostream& ostr) const;

	void  Serialize(WORD format,CSegment &seg);
	void  DeSerialize(WORD format,CSegment &seg);

	WORD  AddNSItem(CSegment& seg);
	WORD  ItemsNumber() const	{ return m_itemsNumber; }
	WORD  IsEmpty() const		{ return (m_itemsNumber==0)? 1 : 0; }
	void					operator= ( const CCapNS& other );
	CCapNonStandardItem*	operator[]( const WORD index ) const;

	//  Polycom Non-Standard capabilities support
	void  AddNScapVisualConcertPC();
	BYTE  OnVisualConcertPC()const;
	void  RemoveVisualConcertPC();
	void  AddNScapVisualConcertFX();
	BYTE  OnVisualConcertFX()const;
	void  RemoveVisualConcertFX();
	BYTE  IsAbleJoinFXconf()const;
	void  AddNSH26LVideoCap(BYTE Octet0,BYTE Octet1);
	void  AddNSDBC2Cap(BYTE Octet0);
	//  PictureTel Non-Standard capabilities support
	void  AddNScapSiren7();
	void  AddNScapSiren716();
	void  AddNScapSiren724();
	void  AddNScapSiren732();
	void  AddNScapSiren14();
	void  AddNScapSiren1424();
	void  AddNScapSiren1432();
	void  AddNScapSiren1448();
	void  AddNScapPeopleContent();
	void  AddNScapFieldDrop();   
	void  AddNScapVTX();
	BYTE  OnSiren7() const;
	BYTE  OnSiren716() const;
	BYTE  OnSiren724() const;
	BYTE  OnSiren732() const;
	BYTE  OnSiren14() const;
	BYTE  OnSiren1424() const;
	BYTE  OnSiren1432() const;
	BYTE  OnSiren1448() const;
	BYTE  OnPeopleContent() const;
	BYTE  OnH26L() const;
	BYTE  OnFieldDrop() const;
	BYTE  OnDBC2() const;
	BYTE  OnVTX() const;
	WORD  GetH26LCifMpiForOneVideoStream() const;
	WORD  GetH26L4CifMpiForOneVideoStream() const;
	WORD  GetH26LMpi(WORD Resolution,WORD NumberOfVideoStreams) const;	
	void  RemovePeopleContent();

protected :
					// Attributes
	WORD				m_itemsNumber;
	WORD				m_itemsMax;
	CCapNonStandardItem**   m_pCapItemsPArray;

					// Operations
	BYTE  OnPictureTelNSCapByte(BYTE capByte)const;
	BYTE  OnPolycomNSCapByte(BYTE capByte)const;
	BYTE  OnPolycomIsraelNSCapByte(BYTE capByte)const;

private:
	WORD AppendNsCap( const WORD countryCode, const WORD manufCode,
			const BYTE* pDataBytes, const BYTE numBytes );
};




#endif /* _H320CAPNS_H  */



