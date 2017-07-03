//+========================================================================+
//                            H320CapNS.CPP                                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H320CapNS.CPP                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |  14/10/07  |                                                      |
//+========================================================================+


#include <iomanip>
#include <iostream>

#include "Macros.h"
#include "Segment.h"
#include "H221.h"
#include "NonStandardCaps.h"
#include "H320CapNS.h"

using namespace std;



/////////////////////////////////////////////////////////////////////////////
//                       CCapNonStandardItem  CLASS                        //
/////////////////////////////////////////////////////////////////////////////
// This class contents all data about set of NonStandard capabilities
// that was get from remote endpoint or will be send to it
// 

/////////////////////////////////////////////////////////////////////////////
CCapNonStandardItem::CCapNonStandardItem() :
			m_msgLen(0), m_countryCode(0x0000), m_manufactCode(0x0000)
{
	m_pCapDataBytes = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CCapNonStandardItem::CCapNonStandardItem(const CCapNonStandardItem& other) : CPObject(other),
			m_msgLen(0), m_countryCode(0x0000), m_manufactCode(0x0000)
{
	m_pCapDataBytes = NULL;
	Create(other);
}

/////////////////////////////////////////////////////////////////////////////
CCapNonStandardItem::~CCapNonStandardItem()
{
	delete [] m_pCapDataBytes;
}

/////////////////////////////////////////////////////////////////////////////
const char* CCapNonStandardItem::NameOf() const
{
	return "CCapNonStandardItem";
}

/////////////////////////////////////////////////////////////////////////////
void CCapNonStandardItem::Dump(std::ostream& ostr) const
{
	ostr << "\n Message Length\t: <" << (dec) << (DWORD)m_msgLen       << ">";
	ostr << "\n Country     \t: <"   << (hex) << (DWORD)m_countryCode  << ">";
	ostr << "\n Manufacturer\t: <"   << (hex) << (DWORD)m_manufactCode << ">";
	ostr << "\n Data bytes  \t:";
	for( int i=0; i<m_msgLen-4; i++ ) {
		ostr << "<" << (hex) << (DWORD)m_pCapDataBytes[i] << ">; ";
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapNonStandardItem::Serialize(WORD format,CSegment &seg)
{
	int i;

	switch( format ) {
		case SERIALEMBD : {
			seg << m_msgLen;
			seg << (BYTE)(m_countryCode & 0xFF)
				<< (BYTE)( (m_countryCode >> 8) & 0xFF );
			seg << (BYTE)(m_manufactCode & 0xFF)
				<< (BYTE)( (m_manufactCode >> 8) & 0xFF );
			for( i=0; i<m_msgLen-4; i++ ) {
				seg << m_pCapDataBytes[i];
			}
			break;
		}
		case NATIVE : {
			seg << m_msgLen;
			seg << m_countryCode;
			seg << m_manufactCode;
			for( i=0; i<m_msgLen-4; i++ ) {
				seg << m_pCapDataBytes[i];
			}
			break;
		}
		default : {
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapNonStandardItem::DeSerialize(WORD format,CSegment &seg)
{
	switch( format ) {
		case SERIALEMBD : {
			break;
		}
		case NATIVE : {
			seg >> m_msgLen;
			if(m_msgLen<5)
			{
				m_msgLen ? PASSERT(m_msgLen) : PASSERT(101);
				// clean the segment in case of error
				BYTE temp;
				for (int j=0;j<m_msgLen;j++)
					seg >> temp;
			}
			else
			{
				seg >> m_countryCode;
				seg >> m_manufactCode;
				if( m_pCapDataBytes != NULL )
					delete [] m_pCapDataBytes;
				m_pCapDataBytes = new BYTE [m_msgLen-4];
				for( int i=0; i<m_msgLen-4; i++ ) {
					seg >> m_pCapDataBytes[i];
				}
			}
			break;
		}
		default : {
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
WORD CCapNonStandardItem::Create(CSegment& seg)
{
	BYTE	msgLen;
	BYTE	count1, count2;
	BYTE	manuf1, manuf2;
	BYTE*	pBytes;
	ALLOCBUFFER(str,ONE_LINE_BUFFER_LEN);

	msgLen = count1 = count2 = manuf1 = manuf2 = 0;

	seg >> msgLen;
	if( msgLen < 5 ) {
		sprintf(str,"%d",(int)msgLen);
		PTRACE2(eLevelError,"CCapNonStandardItem::Create - illegal MsgLen in NS_CAP msg - ",str);
		DEALLOCBUFFER(str);
		return 0;
	}
	if( seg.GetWrtOffset() - seg.GetRdOffset() < msgLen ) {
		sprintf(str,"msgLen (%d), seg(%d)",(int)msgLen,(int)(seg.GetWrtOffset() - seg.GetRdOffset()));
		PTRACE2(eLevelError,"CCapNonStandardItem::Create - segment len less than MsgLen in NS_CAP msg - ",str);
		DEALLOCBUFFER(str);
		return 0;
	}

	seg >> count1;
	seg >> count2;
	seg >> manuf1;
	seg >> manuf2;

	pBytes = new BYTE [msgLen-4];
	for( int iter=0; iter<msgLen-4; iter++ ) {
		seg >> pBytes[iter];
	}

	WORD res = Create(msgLen,count1,count2,manuf1,manuf2,pBytes);
	delete [] pBytes;
	DEALLOCBUFFER(str);

	return res;
}

/////////////////////////////////////////////////////////////////////////////
WORD CCapNonStandardItem::Create(const CCapNonStandardItem& other)
{
	BYTE	msgLen;
	BYTE	count1, count2;
	BYTE	manuf1, manuf2;
	BYTE*	pBytes;

	msgLen = count1 = count2 = manuf1 = manuf2 = 0;

	msgLen	= other.GetMessageLen();
	count1	= (BYTE)(other.m_countryCode & 0xFF);
	count2	= (BYTE)((other.m_countryCode >> 8) & 0xFF);
	manuf1	= (BYTE)(other.m_manufactCode & 0xFF);
	manuf2	= (BYTE)((other.m_manufactCode >> 8) & 0xFF);

	pBytes = other.GetpCapDataBytes();

	return Create(msgLen,count1,count2,manuf1,manuf2,pBytes);
}

/////////////////////////////////////////////////////////////////////////////
WORD CCapNonStandardItem::Create(const BYTE msgLen,const BYTE country1,const BYTE country2,
								 const BYTE manuf1,const BYTE manuf2,const BYTE* capData)
{
	if( msgLen < 5 ) {
		DBGPASSERT(msgLen);
		return 0;
	}

	m_msgLen = msgLen;
	m_countryCode  = (country2 << 8) | country1;
	m_manufactCode = ( manuf2  << 8) | manuf1;

	if( m_pCapDataBytes != NULL )
		delete [] m_pCapDataBytes;
	m_pCapDataBytes = new BYTE [m_msgLen-4];
	memcpy(m_pCapDataBytes,capData,m_msgLen-4);

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
WORD CCapNonStandardItem::operator==(const CCapNonStandardItem& Item) const
{
	if( m_msgLen != Item.GetMessageLen() )
		return 0;
	if( m_countryCode != Item.m_countryCode )
		return 0;
	if( m_manufactCode != Item.m_manufactCode )
		return 0;

	for( int iter=0; iter<m_msgLen-4; iter++ )
		if( m_pCapDataBytes[iter] != Item.m_pCapDataBytes[iter] )
			return 0;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNonStandardItem::RemoveBytesFromDataArray(const BYTE indexFrom, const BYTE howMany)
{
	BYTE	arrayLen = m_msgLen - 4;

	if( indexFrom >= arrayLen )
		return 0;
	if( indexFrom + howMany > arrayLen )
		return 0;
	if( howMany == 0 )
		return 1;

	int		i, j;
	BYTE*	pBytes = new BYTE [arrayLen - howMany];

	for( i=0; i<indexFrom; i++ )
		pBytes[i] = m_pCapDataBytes[i];

	for( j=i+howMany; j<arrayLen && i<arrayLen-howMany; j++,i++ )
		pBytes[i] = m_pCapDataBytes[j];

	delete [] m_pCapDataBytes;
	m_pCapDataBytes = pBytes;

	m_msgLen -= howMany;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNonStandardItem::AppendBytesToDataArray( const BYTE* pDataBytes, const BYTE numBytes )
{
	BYTE*	pBytes = NULL;

	if( m_msgLen < 5 ) {
		DBGPASSERT(501);
		return 0;
	}
	if( (m_msgLen+numBytes) >= 256 ) { // 256 - max length of NS_CAP message
		DBGPASSERT(502);
		return 0;
	}
	if( (((int)m_msgLen)-4+((int)numBytes)) <= 0 ) {
		DBGPASSERT(503);
		return 0;
	}

	pBytes = new BYTE[m_msgLen-4+numBytes];
	if( !pBytes ) {
		DBGPASSERT(504);
		return 0;
	}

	if( m_pCapDataBytes != NULL )
		memcpy(pBytes,m_pCapDataBytes,m_msgLen-4);

	memcpy((pBytes+m_msgLen-4),pDataBytes,numBytes);
	if( m_pCapDataBytes != NULL )
		delete [] m_pCapDataBytes;
	else
		DBGPASSERT(505);
	m_pCapDataBytes = pBytes;
	m_msgLen += numBytes;
	return 1;
}


/////////////////////////////////////////////////////////////////////////////
//                               CNSCap  CLASS                             //
/////////////////////////////////////////////////////////////////////////////
// This class is container for CCapNonStandardItem objects. It contains all
// non-standard capabilities that we got from endpoint or we'll send to e.p
//

const BYTE   STEP2ADD = 2;


/////////////////////////////////////////////////////////////////////////////
CCapNS::CCapNS()
{
	m_itemsNumber = 0;
	m_itemsMax = 5;

	m_pCapItemsPArray = new CCapNonStandardItem* [m_itemsMax];
	for( int ind=0; ind<m_itemsMax; ind++ )
		m_pCapItemsPArray[ind] = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CCapNS::~CCapNS()
{
	for( int ind=0; ind<m_itemsMax; ind++ ) {
		if( m_pCapItemsPArray[ind] != NULL ) {
			POBJDELETE( m_pCapItemsPArray[ind] );
			m_pCapItemsPArray[ind] = NULL;
		}
	}
	delete [] m_pCapItemsPArray;
}

/////////////////////////////////////////////////////////////////////////////
const char* CCapNS::NameOf() const
{
	return "CCapNS";
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::Dump(std::ostream& ostr) const
{
    if( 0 == m_itemsNumber ) {
		ostr << "\n==================    CCapNS::Dump    ==================\n";
		ostr << "\n empty \n";
 		return;
	}
	ostr << "\n===================    CCapNS::Dump    ===================\n" ;
		for( int i=0; i<m_itemsNumber; i++ ) {
			if( i ) ostr << "--------------------------------------\n" ;
			m_pCapItemsPArray[i]->Dump(ostr);
		}
	ostr << "\n===============  CCapNS::Dump Finished!!!  ===============\n" ;
	ostr << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::Serialize(WORD format,CSegment &seg)
{
	int i;

	switch( format ) {
		case SERIALEMBD : {
			for( i=0; i<m_itemsNumber; i++ ) {
				seg << (BYTE)(ESCAPECAPATTR | Ns_Cap);
				m_pCapItemsPArray[i]->Serialize(format,seg);
			}
			break;
		}
		case NATIVE : {
			seg << m_itemsNumber;
			for( i=0; i<m_itemsNumber; i++ ) {
				m_pCapItemsPArray[i]->Serialize(format,seg);
			}
			break;
		}
		default : {
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::DeSerialize(WORD format,CSegment &seg)
{
	switch( format ) {
		case SERIALEMBD : {
			break;
		}
		case NATIVE : {
			int   i;
			WORD  itemsNum = 0;

			seg >> itemsNum;
			if( itemsNum <= 0 )
				break;

			// clear all cells of array
			for( i=0; i<m_itemsMax; i++ ) {
				if( m_pCapItemsPArray[i] != NULL )
					POBJDELETE( m_pCapItemsPArray[i] );
			}
			// reallocate array
			if( itemsNum > m_itemsMax ) {
				delete [] m_pCapItemsPArray;
				m_itemsMax = itemsNum;
				m_pCapItemsPArray = new CCapNonStandardItem* [m_itemsMax];
				for( i=0; i<m_itemsMax; i++ )
					m_pCapItemsPArray[i] = NULL;
			}
			m_itemsNumber = itemsNum;
			// fill array from segment
			for( i=0; i<m_itemsNumber; i++ ) {
				if( m_pCapItemsPArray[i] != NULL )
					POBJDELETE( m_pCapItemsPArray[i] );
				m_pCapItemsPArray[i] = new CCapNonStandardItem();
				m_pCapItemsPArray[i]->DeSerialize(format,seg);
			}
			break;
		}
		default : {
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::operator= (const CCapNS& other)
{
	if( this == &other )
		return;

	int i;

	// cleanup
	for( i=0; i<m_itemsNumber; i++ ) {
		if( m_pCapItemsPArray[i] != NULL )
			POBJDELETE(m_pCapItemsPArray[i]);
	}
	m_itemsNumber  = other.m_itemsNumber;
	// reallocate array
	if( m_itemsNumber > m_itemsMax ) {
		delete [] m_pCapItemsPArray;
		m_itemsMax  = m_itemsNumber;
		m_pCapItemsPArray = new CCapNonStandardItem* [m_itemsMax];
		for( i=0; i<m_itemsMax; i++ )
			m_pCapItemsPArray[i] = NULL;
	}
	// fill array
	for( i=0; i<m_itemsNumber; i++ ) {
		m_pCapItemsPArray[i] = new CCapNonStandardItem(*(other[i]));
	}
}

/////////////////////////////////////////////////////////////////////////////
CCapNonStandardItem* CCapNS::operator[]( const WORD index ) const
{
	if( index >= m_itemsNumber ) {
		DBGPASSERT(index);
		return NULL;
	}

	return m_pCapItemsPArray[index];
}

/////////////////////////////////////////////////////////////////////////////
// WARNING: Use this function when you receive NsCap from remote E.P. in the
//          remote cap sets, for local cap set use  AppendNsCap  function
WORD CCapNS::AddNSItem(CSegment& seg)
{
	CCapNonStandardItem  newItem;

	// create new NSCAP item (check for a param inside)
	if( newItem.Create(seg) != 1 )
		return 0;
	// if array has identic item, return
	int ind=0;
	for( ind=0; ind<m_itemsNumber; ind++ )
		if( newItem == *(m_pCapItemsPArray[ind]) )
			return 1;
	// reallocate storage if it full
	if( m_itemsNumber >= m_itemsMax ) {
		CCapNonStandardItem** pCapArray = new CCapNonStandardItem* [m_itemsMax+STEP2ADD];
		if( !pCapArray )
			return 0;
		for( ind=0; ind<m_itemsNumber; ind++ )
			pCapArray[ind] = m_pCapItemsPArray[ind];
		for( ; ind<m_itemsMax+STEP2ADD; ind++ )
			pCapArray[ind] = NULL;
		delete [] m_pCapItemsPArray;
		m_pCapItemsPArray = pCapArray;
		m_itemsMax += STEP2ADD;
	}
	// append new item
	m_pCapItemsPArray[m_itemsNumber] = new CCapNonStandardItem(newItem);
	m_itemsNumber++ ;
	return 1;
}


/////////////////////////////////////////////////////////////////////////////
// PictureTel Non-Standard capabilities support
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapSiren7()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_SIREN7;				// Siren7 - 16k/24k/32k

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapSiren716()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_SIREN716;				// Siren7-16k

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapSiren724()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_SIREN724;				// Siren7-24k

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapSiren732()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_SIREN732;				// Siren7-32k

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapSiren14()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_SIREN14;				// Siren14 - 24k/32k/48k

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapSiren1424()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_SIREN1424;				// Siren14-24k

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapSiren1432()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_SIREN1432;				// Siren14-32k

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapSiren1448()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_SIREN1448;				// Siren14-48k

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapPeopleContent()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_PEOPLE_CONTENT;		// PeopleContent
	bytes[1] = (BYTE) 0;							// version

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 2 );
}
/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNSH26LVideoCap(BYTE Octet0,BYTE Octet1)
{
	// PictureTel 
	BYTE  bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells
	
	

	bytes[0] = (BYTE) NS_CAP_H26L; // H.26L     
	bytes[1] = (BYTE) 0x1;	        // According to the last spec : Currently the only defined value is 1
	bytes[2] = (BYTE) Octet0;		// MPI for CIF/SIF + 4CIF/4SIF for one video stream
	bytes[3] = (BYTE) Octet1;		// MPI for CIF/SIF + 4CIF/4SIF for two video streams
    

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 4 );

}
/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNSDBC2Cap(BYTE Octet0)
{
	// PictureTel 
	BYTE  bytes[2];
	memset(bytes,0x00,2); // set 0 to all cells	

	bytes[0] = (BYTE) NS_CAP_DBC2; // DBC2     
	bytes[1] = (BYTE) Octet0;	   // MotionVectors,canInterleave,requiresEncapsulation,maxOverlap
	
	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 2 );

}
/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapFieldDrop()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_FIELD_DROP;	// Field Drop

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PICTURETEL, bytes, 1 );
}
/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapVTX()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_VTX;				// VTX Cap

	AppendNsCap( W_COUNTRY_CODE_ISRAEL, W_MANUFACT_CODE_POLYCOM, bytes, 1 );
}

///////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnPictureTelNSCapByte(BYTE capByte/*, BYTE paramByte*/)const
{
	BYTE	retCode = 0;
	CCapNonStandardItem*    pItem = NULL;

	for( int i=0; i<m_itemsNumber && !retCode; i++ ) {
		pItem = m_pCapItemsPArray[i];
		if( pItem->GetCountryCode() != W_COUNTRY_CODE_USA )
			continue;
		if( pItem->GetManufactCode() != W_MANUFACT_CODE_PICTURETEL )
			continue;
		for( int j=0; j<pItem->GetMessageLen()-4 && !retCode; j++ ) {
			switch( pItem->GetpCapDataBytes()[j] ) {
				case 0x80 :
				case 0x81 :
				case 0x82 :
				case 0x83 :
				case 0x84 :
				case 0x85 :
				case 0x86 :
				case NS_CAP_SIREN7    : // 0x87
				case NS_CAP_SIREN14   : // 0x88
				case 0x89 :
				case 0x8A :
				case NS_CAP_SIREN716  : // 0x8D
				case NS_CAP_SIREN724  : // 0x8E
				case NS_CAP_SIREN732  : // 0x8F
				case 0x90 :
				case NS_CAP_SIREN1424 : // 0x91
				case NS_CAP_SIREN1432 : // 0x92
				case NS_CAP_SIREN1448 : // 0x93
				case NS_FIELD_DROP    :  //0x9F
                {
					if( capByte == pItem->GetpCapDataBytes()[j] )
						retCode = 1;
					break;
				}
				case 0x8B :
				case 0x8C :
				case NS_CAP_DBC2:
				case NS_CAP_PEOPLE_CONTENT : { // 0x95
					/*
					capabilities with these opcodes uses 2 bytes,
					so we need to move pointer to one byte forward
					*/
					if( capByte   == pItem->GetpCapDataBytes()[j] ) 
						retCode = 1;
					j++;
					break;
											 }
				case NS_CAP_H26L: //0x9E
					{
					/*
					capabilities with these opcodes uses 4 bytes,
					so we need to move pointer to one byte forward
					*/
						if( capByte   == pItem->GetpCapDataBytes()[j] ) 
							retCode = 1;
						j += 3 ;
						break;
					}
					
				default : {
					break;
				}
			}
		}
	}
	return retCode;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnSiren7() const
{
	return OnPictureTelNSCapByte(NS_CAP_SIREN7);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnSiren716() const
{
	if( OnSiren7() )
		return 1;
	return OnPictureTelNSCapByte(NS_CAP_SIREN716);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnSiren724() const
{
	if( OnSiren7() )
		return 1;
	return OnPictureTelNSCapByte(NS_CAP_SIREN724);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnSiren732() const
{
	if( OnSiren7() )
		return 1;
	return OnPictureTelNSCapByte(NS_CAP_SIREN732);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnSiren14() const
{
	return OnPictureTelNSCapByte(NS_CAP_SIREN14);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnSiren1424() const
{
	if( OnSiren14() )
		return 1;
	return OnPictureTelNSCapByte(NS_CAP_SIREN1424);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnSiren1432() const
{
	if( OnSiren14() )
		return 1;
	return OnPictureTelNSCapByte(NS_CAP_SIREN1432);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnSiren1448() const
{
	if( OnSiren14() )
		return 1;
	return OnPictureTelNSCapByte(NS_CAP_SIREN1448);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnPeopleContent() const
{
	return OnPictureTelNSCapByte(NS_CAP_PEOPLE_CONTENT);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnFieldDrop() const
{
	return OnPictureTelNSCapByte(NS_FIELD_DROP);
}
///////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnPolycomIsraelNSCapByte(BYTE capByte)const
{
	BYTE	retCode = 0;
	CCapNonStandardItem*    pItem = NULL;

	for( int i=0; i<m_itemsNumber && !retCode; i++ ) {
		pItem = m_pCapItemsPArray[i];
		if( pItem->GetCountryCode() != W_COUNTRY_CODE_ISRAEL )
			continue;
		if( pItem->GetManufactCode() != W_MANUFACT_CODE_POLYCOM )
			continue;
		for( int j=0; j<pItem->GetMessageLen()-4 && !retCode; j++ ) {
			switch( pItem->GetpCapDataBytes()[j] ) {
				case NS_CAP_VTX :
                {
					if( capByte == pItem->GetpCapDataBytes()[j] )
						retCode = 1;
					break;
				}	
				default : {
					break;
				}
			}
		}
	}
	return retCode;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnVTX() const
{
	return OnPolycomIsraelNSCapByte(NS_CAP_VTX);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnDBC2() const
{
	return OnPictureTelNSCapByte(NS_CAP_DBC2);
}
/////////////////////////////////////////////////////////////////////////////
void CCapNS::RemovePeopleContent()
{
	if( !OnPeopleContent() )
		return;

	CCapNonStandardItem*    pItem = NULL;
	BYTE                    removeThisItem;
	int     i, j, k;

	for( i=0; i<m_itemsNumber; i++ ) {
		removeThisItem = 0;
		pItem = m_pCapItemsPArray[i];

		if( pItem->GetCountryCode() != W_COUNTRY_CODE_USA )
			continue;
		if( pItem->GetManufactCode() != W_MANUFACT_CODE_PICTURETEL )
			continue;
		for( j=0; j<pItem->GetMessageLen()-4 && !removeThisItem; j++ ) {
			switch( pItem->GetpCapDataBytes()[j] ) {
				case 0x8B :
				case 0x8C :
				case NS_CAP_DBC2:
				{
					/*
					capabilities with these opcodes uses 2 bytes,
					so we need to move pointer to one byte forward
					*/
					j++;
					break;
				}
				case NS_CAP_H26L : { // 0x9E
					/*
					this cap - H.26L - has 3 parameters
					*/
					j += 3;
					break;
				}
				case NS_CAP_PEOPLE_CONTENT : {
					/*
					capabilities with these opcodes uses 2 bytes,
					so we need to move pointer to one byte forward
					*/
					if( pItem->GetMessageLen()-4 == 2 ) {
						// only PeopleContentV0 cap in NsCap set
						removeThisItem = 1;
						j++;
					} else { // not only PeopleContent in NsCap set 
						// remove 2 bytes from array of bytes :
						// PeopleContent opcode and version parameter
						if( pItem->RemoveBytesFromDataArray(j,2) ) {
							// now check element with this index again
							j--;
						}
					}
					break;
				}
				default : {
					break;
				}
			}
		}
		if( removeThisItem ) {
			// delete current item
			POBJDELETE(m_pCapItemsPArray[i]);
			// move all pointers in array of pointers
			for( k=i; k<m_itemsNumber-1; k++ ) {
				m_pCapItemsPArray[k] = m_pCapItemsPArray[k+1];
				m_pCapItemsPArray[k+1] = NULL;
			}
			// decrease item number data member
			m_itemsNumber-- ;
			// now we need to check this element of array again
			i-- ;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Polycom Non-Standard capabilities support
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapVisualConcertPC()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_ACCORD2POLYCOM;		// Accord sender
	bytes[1] = (BYTE) NS_CAP_MERCURY;				// VisualConcertPC capability

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_POLYCOM, bytes, 2 );
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::AddNScapVisualConcertFX()
{
	BYTE	bytes[4];
	memset(bytes,0x00,4); // set 0 to all cells

	bytes[0] = (BYTE) NS_CAP_ACCORD2POLYCOM;		// Accord sender
	bytes[1] = (BYTE) NS_CAP_R2D2;					// VisualConcertFX capability
	bytes[2] = (BYTE) NS_CAP_H263_4CIF_ANNEX_I;		// NonStandard H263 4CIF Annex I
	bytes[3] = (BYTE) NS_CAP_H263_4CIF_ANNEX_T;		// NonStandard H263 4CIF Annex T

	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_POLYCOM, bytes, 4 );
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnPolycomNSCapByte(BYTE capByte)const
{
	BYTE                    retCode = 0;
	CCapNonStandardItem*    pItem   = NULL;

	for( int i=0; i<m_itemsNumber && !retCode; i++ ) {
		pItem = m_pCapItemsPArray[i];
		if( pItem->GetCountryCode() != W_COUNTRY_CODE_USA )
			continue;
		if( pItem->GetManufactCode() != W_MANUFACT_CODE_POLYCOM )
			continue;
		for( int j=0; j<pItem->GetMessageLen()-4 && !retCode; j++ ) {
			if( capByte == pItem->GetpCapDataBytes()[j] )
				retCode = 1;
		}
	}
	return retCode;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnH26L() const
{
	return OnPictureTelNSCapByte(NS_CAP_H26L);
}
/////////////////////////////////////////////////////////////////////////////
WORD CCapNS::GetH26LCifMpiForOneVideoStream() const
{
	return GetH26LMpi(H26L_CIF,1);
}
/////////////////////////////////////////////////////////////////////////////
WORD CCapNS::GetH26L4CifMpiForOneVideoStream() const
{
	return GetH26LMpi(H26L_CIF_4,1);
}
/////////////////////////////////////////////////////////////////////////////
WORD CCapNS::GetH26LMpi(WORD Resolution,WORD NumberOfVideoStreams) const
{
	BYTE                    retCode = 0;
	CCapNonStandardItem*    pItem   = NULL;
	BYTE                    Octet;

	for( int i=0; i<m_itemsNumber && !retCode; i++ ) 
	{
		pItem = m_pCapItemsPArray[i];
		if( pItem->GetCountryCode() != W_COUNTRY_CODE_USA )
			continue;
		if( pItem->GetManufactCode() != W_MANUFACT_CODE_PICTURETEL )
			continue;
		for( int j=0; j<pItem->GetMessageLen()-4 && !retCode; j++ )
		{			
			if( NS_CAP_H26L == pItem->GetpCapDataBytes()[j] )		
			{   
				++j;
				Octet = pItem->GetpCapDataBytes()[++j]; //Next Byte

				switch(Resolution)
				{
				case H26L_CIF:
					{
						retCode = ( (Octet & 0x70) >> 4); //CIF resolution
						break;
					}
				case H26L_CIF_4:
					{
						retCode = (Octet & 0x0F); //4CIF resolution
						break;
					}

			   default: 
				   { 
					   if(Resolution)
							PASSERT(Resolution);
					   else
							PASSERT(101); 
					   break; 
				   }
				}
			}
		}
	}
	return retCode;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnVisualConcertPC()const
{
	return OnPolycomNSCapByte(NS_CAP_MERCURY);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::OnVisualConcertFX()const
{
	return OnPolycomNSCapByte(NS_CAP_R2D2);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapNS::IsAbleJoinFXconf() const
{
	/*
	//  old condition of participation : ( R2D2 || (DUAL && VGA) )

	// if ns_cap set has NS_CAP_ACCORD2POLYCOM byte - it's a local capset
	if( OnPolycomNSCapByte(NS_CAP_ACCORD2POLYCOM) )
		return FALSE;
	// if ns_cap set has NS_CAP_R2D2 - supports VisualConcertFX
	if( OnPolycomNSCapByte(NS_CAP_R2D2) )
		return TRUE;
	// if ns_cap set has DualVideoStreams cap & VGA possibility - supports FX as the viewer
	if( OnPolycomNSCapByte(NS_CAP_VIDEO_STREAM_2) ) {
		if( OnPolycomNSCapByte(NS_CAP_VGA_800X600) )
			return TRUE;
		if( OnPolycomNSCapByte(NS_CAP_VGA_1024X768) )
			return TRUE;
		if( OnPolycomNSCapByte(NS_CAP_VGA_1280X1024) )
			return TRUE;
	}
	*/

	//  condition of participation : ( (R2D2 || DUAL) && AnnexI && AnnexT )

	// if ns_cap set has NS_CAP_ACCORD2POLYCOM byte - it's a local capset
//	if( OnPolycomNSCapByte(NS_CAP_ACCORD2POLYCOM) )
//		return FALSE;
	// if ns_cap set has any AnnexI cap
	if( !OnPolycomNSCapByte(NS_CAP_H263_QCIF_ANNEX_I) && 
		!OnPolycomNSCapByte(NS_CAP_H263_CIF_ANNEX_I) && 
		!OnPolycomNSCapByte(NS_CAP_H263_4CIF_ANNEX_I) )
			return FALSE;
	// if ns_cap set has any AnnexT cap
	if( !OnPolycomNSCapByte(NS_CAP_H263_QCIF_ANNEX_T) && 
		!OnPolycomNSCapByte(NS_CAP_H263_CIF_ANNEX_T) && 
		!OnPolycomNSCapByte(NS_CAP_H263_4CIF_ANNEX_T) )
			return FALSE;
	// if ns_cap set has NS_CAP_R2D2 - supports VisualConcertFX
	if( OnPolycomNSCapByte(NS_CAP_R2D2) )
		return TRUE;
	// if ns_cap set has DualVideoStreams cap & VGA possibility - supports FX as the viewer
	if( OnPolycomNSCapByte(NS_CAP_VIDEO_STREAM_2) || 
		OnPolycomNSCapByte(NS_CAP_VIDEO_STREAM_3) ||
		OnPolycomNSCapByte(NS_CAP_VIDEO_STREAM_4) )
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::RemoveVisualConcertPC()
{
	if( !OnVisualConcertPC() )
		return;

	CCapNonStandardItem*    pItem = NULL;
	BYTE                    removeThisItem;
	int     i, j, k;

	for( i=0; i<m_itemsNumber; i++ ) {
		removeThisItem = 0;
		pItem = m_pCapItemsPArray[i];

		if( pItem->GetCountryCode() != W_COUNTRY_CODE_USA )
			continue;
		if( pItem->GetManufactCode() != W_MANUFACT_CODE_POLYCOM )
			continue;
		for( j=0; j<pItem->GetMessageLen()-4 && !removeThisItem; j++ ) {
			switch( pItem->GetpCapDataBytes()[j] ) {
	// according to mail of Dave Hein from 28/07/01 we have to inform
	// Polycom e.p. who is sender of MercuryCap, 6C - Accord
				case NS_CAP_ACCORD2POLYCOM : { // 0x6C
					if( pItem->GetMessageLen()-4 == 2 && j==0 &&
							pItem->GetpCapDataBytes()[1] == NS_CAP_MERCURY ) {
						// only Polycom' VisualConcert PC cap in NsCap set
						removeThisItem = 1;
						j++;
					} else { // not only VisualConcert PC in NsCap set 
						// we are not remove AccordSender byte
						/*if( j+1<pItem->GetMessageLen()-4 &&
								pItem->GetpCapDataBytes()[j+1] == NS_CAP_MERCURY ) {
							// remove 2 bytes from array of bytes :
							// VisualConcert PC opcode
							if( pItem->RemoveBytesFromDataArray(j,2) ) {
								// now check element with this index again
								j--;
							}
						}*/
					}
					break;
				}
				case NS_CAP_MERCURY : {  // 0x2F
					if( pItem->GetMessageLen()-4 == 1 ) {
						// only Polycom' VisualConcert PC cap in NsCap set
						removeThisItem = 1;
						j++;
					} else { // not only VisualConcert PC in NsCap set 
						// remove 1 byte from array of bytes :
						// VisualConcert PC opcode
						if( pItem->RemoveBytesFromDataArray(j,1) ) {
							// now check element with this index again
							j--;
						}
					}
					break;
				}
				default : {
					break;
				}
			}
		}
		if( removeThisItem ) {
			// delete current item
			POBJDELETE(m_pCapItemsPArray[i]);
			// move all pointers in array of pointers
			for( k=i; k<m_itemsNumber-1; k++ ) {
				m_pCapItemsPArray[k] = m_pCapItemsPArray[k+1];
				m_pCapItemsPArray[k+1] = NULL;
			}
			// decrease item number data member
			m_itemsNumber-- ;
			// now we need to check this element of array again
			i-- ;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapNS::RemoveVisualConcertFX()
{
	if( !OnVisualConcertFX() )
		return;

	CCapNonStandardItem*	pItem = NULL;
	BYTE					removeThisItem;
	int			i, j, k;

	for( i=0; i<m_itemsNumber; i++ ) {
		removeThisItem = 0;
		pItem = m_pCapItemsPArray[i];

		if( pItem->GetCountryCode() != W_COUNTRY_CODE_USA )
			continue;
		if( pItem->GetManufactCode() != W_MANUFACT_CODE_POLYCOM )
			continue;
		for( j=0; j<pItem->GetMessageLen()-4 && !removeThisItem; j++ ) {
			switch( pItem->GetpCapDataBytes()[j] ) {
	// according to mail of Dave Hein from 28/07/01 we have to inform
	// Polycom e.p. who is sender of MercuryCap, 6C - Accord
				case NS_CAP_ACCORD2POLYCOM : { // 0x6C
					if( pItem->GetMessageLen()-4 == 2 && j==0 &&
							pItem->GetpCapDataBytes()[1] == NS_CAP_R2D2 ) {
						// only Polycom' VisualConcert FX cap in NsCap set
						removeThisItem = 1;
						j++;
					} else { // not only VisualConcert FX in NsCap set 
						// we are not remove AccordSender byte
						/*if( j+1<pItem->GetMessageLen()-4 &&
								pItem->GetpCapDataBytes()[j+1] == NS_CAP_R2D2 ) {
							// remove 2 bytes from array of bytes :
							// VisualConcert FX opcode
							if( pItem->RemoveBytesFromDataArray(j,2) ) {
								// now check element with this index again
								j--;
							}
						}*/
					}
					break;
				}
				case NS_CAP_R2D2 : {  // 0x33
					if( pItem->GetMessageLen()-4 == 1 ) {
						// only Polycom' VisualConcert FX cap in NsCap set
						removeThisItem = 1;
						j++;
					} else { // not only VisualConcert FX in NsCap set 
						// remove 1 byte from array of bytes :
						// VisualConcert FX opcode
						if( pItem->RemoveBytesFromDataArray(j,1) ) {
							// now check element with this index again
							j--;
						}
					}
					break;
				}
				default : {
					break;
				}
			}
		}
		if( removeThisItem ) {
			// delete current item
			POBJDELETE(m_pCapItemsPArray[i]);
			// move all pointers in array of pointers
			for( k=i; k<m_itemsNumber-1; k++ ) {
				m_pCapItemsPArray[k] = m_pCapItemsPArray[k+1];
				m_pCapItemsPArray[k+1] = NULL;
			}
			// decrease item number data member
			m_itemsNumber-- ;
			// now we need to check this element of array again
			i-- ;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// WARNING: Use this function when you add NsCaps in the local cap sets,
//          for caps, received from E.P. use  AddNSItem  function
WORD CCapNS::AppendNsCap( const WORD countryCode, const WORD manufCode,
						const BYTE *pDataBytes, const BYTE numBytes )
{
	const BYTE		STEP2ADD = 2;

	WORD			found  = 0;
	int				i, j;
	BYTE*			pTmpBytes;
	CCapNonStandardItem*	pItem;

	// 1. go for all existing items: search for existance
	for( i=0; i<m_itemsNumber && !found; i++ ) {

		pItem = m_pCapItemsPArray[i];

		// if another manufacturer - go to next
		if( countryCode != pItem->GetCountryCode() )
			continue;
		if( manufCode != pItem->GetManufactCode() )
			continue;

		// search for the same sequence in existing item
		if( pItem->GetMessageLen()-4 >= numBytes ) {
			pTmpBytes = pItem->GetpCapDataBytes();
			for( j=0; j<=pItem->GetMessageLen()-4-numBytes; j++ ) {
				if( memcmp(pTmpBytes,pDataBytes,numBytes) != 0 ) {
					pTmpBytes++;
				} else {
					// existing item contents new sequence
					found = 1;
					break;
				}
			}
		}
	}

	// 2. go for all existing items: add to existing
	for( i=0; i<m_itemsNumber && !found; i++ ) {

		pItem = m_pCapItemsPArray[i];

		// if another manufacturer - go to next
		if( countryCode != pItem->GetCountryCode() )
			continue;
		if( manufCode != pItem->GetManufactCode() )
			continue;

		// 256 - max data size for NS_CAP structure
		if( pItem->GetMessageLen() + numBytes < 256 ) {
			if( pItem->AppendBytesToDataArray(pDataBytes,numBytes) )
				found = 1;
		} // else go to next
	}

	if( found )
		return 1;
	// ns-cap not found between existing items or existings are full - create new one

	// 3. reallocate storage if it full
	if( m_itemsNumber >= m_itemsMax ) {
		CCapNonStandardItem** pCapArray = new CCapNonStandardItem* [m_itemsMax+STEP2ADD];
		if( !pCapArray )
			return 0;
		// copy all item's pointers to new array
		for( i=0; i<m_itemsNumber; i++ )
			pCapArray[i] = m_pCapItemsPArray[i];
		for( ; i<m_itemsMax+STEP2ADD; i++ )
			pCapArray[i] = NULL;
		delete [] m_pCapItemsPArray;
		m_pCapItemsPArray = pCapArray;
		m_itemsMax += STEP2ADD;
	}
	// 4. append new item
	m_pCapItemsPArray[m_itemsNumber] = new CCapNonStandardItem();

	m_pCapItemsPArray[m_itemsNumber]->Create( numBytes+4,
		(BYTE)(countryCode & 0xFF), (BYTE)((countryCode>>8) & 0xFF),
		(BYTE)(manufCode   & 0xFF), (BYTE)((manufCode  >>8) & 0xFF),
		pDataBytes);

	m_itemsNumber++ ;

	return 1;
}




