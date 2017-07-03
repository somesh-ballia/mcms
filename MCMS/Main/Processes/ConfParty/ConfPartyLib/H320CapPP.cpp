//+========================================================================+
//                            H320CapPP.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H320CapPP.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 29/10/07   |                                                      |
//+========================================================================+

#include <sstream>
#include <iomanip>

#include "Macros.h"
#include "Segment.h"
#include "ConfPartyDefines.h"
#include "ConfPartyApiDefines.h"
// #include "CDRUtils.h"
#include "H221.h"
#include "H263.h"
#include "H320CapPP.h"     
// #include "NStream.h"
#include "NonStandardCaps.h"
// #include "Trace.h"

 
using namespace std;
/////////////////////////////////////////////////////////////////////////////
/////////////////////////// CVideoStreamCap /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CVideoStreamCap::CVideoStreamCap(): m_pLabelList(NULL), m_labelListSize(0)
{
}
/////////////////////////////////////////////////////////////////////////////	
CVideoStreamCap::CVideoStreamCap(const CVideoStreamCap& other) : CPObject(other)
{
	PDELETEA(m_pLabelList);
    m_labelListSize = 0;
	
	if (other.m_labelListSize > 0) {
		m_labelListSize = other.m_labelListSize;
        m_pLabelList = new BYTE[m_labelListSize];
        
        if(m_pLabelList)
        {
            for (int i=0 ; i < m_labelListSize ; i++)
			    m_pLabelList[i] = other.m_pLabelList[i];
        }
        else
            PASSERT(1);
    }
	
	m_h263cap = other.m_h263cap;
	m_h261cap = other.m_h261cap;
}
/////////////////////////////////////////////////////////////////////////////	
CVideoStreamCap::~CVideoStreamCap()
{
	PDELETEA(m_pLabelList);
}
/////////////////////////////////////////////////////////////////////////////	
const char* CVideoStreamCap::NameOf() const
{
    return "CVideoStreamCap";
}
/////////////////////////////////////////////////////////////////////////////	
CVideoStreamCap& CVideoStreamCap::operator=(const CVideoStreamCap& other)
{
	if (this == &other) return *this;

	PDELETEA(m_pLabelList);
    m_labelListSize = 0;
	
	if (other.m_labelListSize > 0) {
		m_labelListSize = other.m_labelListSize;
        m_pLabelList = new BYTE[m_labelListSize];

        if(m_pLabelList)
        {
            for (int i=0 ; i < m_labelListSize ; i++)
			    m_pLabelList[i] = other.m_pLabelList[i];
        }
		else
            PASSERT(1);
    }
	
	m_h263cap = other.m_h263cap;
	m_h261cap = other.m_h261cap;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////	
void CVideoStreamCap::Dump(std::ostream& ostr) const
{
    ostr << "\nCVideoStreamCap::Dump\n"
		 << "---------------------"
		 << "\n  Labels Number : <"   << (dec) << (WORD)m_labelListSize  << ">";
	for (BYTE  i=0 ; i < m_labelListSize ; i++)
	  ostr << "\n  Label         : <"   << (dec)<< (WORD)i << "  =  " << (dec) << (WORD)m_pLabelList[i] << ">";
	
	ostr << "\n";
	m_h263cap.Dump(ostr);
	//m_h261cap.Dump(ostr);
}
/////////////////////////////////////////////////////////////////////////////	
void CVideoStreamCap::DeSerializeEmbededVideoCaps(BYTE dataLen, BYTE readBytesNum, CSegment& seg)
{
	BYTE capLen = 0;
	BYTE basCode = 0;
			//***** Start Video caps parsing
	while ( readBytesNum < dataLen ) {
		seg >> basCode;
			
		readBytesNum += 1;
			
		switch (basCode) {
		case ESCAPECAPATTR | Start_Mbe : {
			BYTE mbeLen=0;
			BYTE table2_H230Value;
			seg >> mbeLen;
			readBytesNum++;
				
			seg >> table2_H230Value;
			if( table2_H230Value == H262_H263 )
				m_h263cap.Create(seg,mbeLen);

			readBytesNum += mbeLen;				
			break;
		}
		case V_Cif | DATAVIDCAPATTR : {
			seg >> basCode;
			m_h261cap.SetCaps(V_Qcif, basCode^DATAVIDCAPATTR);
			seg >> basCode;
			m_h261cap.SetCaps(V_Cif, basCode^DATAVIDCAPATTR);
			readBytesNum += 2;
			break;
		}
		case V_Qcif | DATAVIDCAPATTR : {
			seg >> basCode;
			m_h261cap.SetCaps(V_Qcif, basCode^DATAVIDCAPATTR);
			readBytesNum++;
			break;
		}
		default: { // Unknown video caps
				
			// skip unknown msg bytes
			//for (BYTE i=0; i<(capLen-1); i++) {
			//	seg >> basCode;
			//	readBytesNum++;
			//}
		    PTRACE(eLevelError,"CVideoStreamCap::DeSerializeEmbededVideoCaps: Unknow Video Caps");//EXCEPTION_TRACE|EPC_TRACE
			DBGPASSERT(readBytesNum+1);
			break;
		}
		}
		if(readBytesNum > dataLen) {
			PASSERT_AND_RETURN( readBytesNum );
		}
	}
		//***** End Video caps parsing
}
/////////////////////////////////////////////////////////////////////////////	
void CVideoStreamCap::CreateVideoCaps()
{

	// Create H263 caps for Content - QCIF-30, CIF-30, 4CIF-15, VGA-15, SVGA-10, XGA-7.5, Annex_T.
	// In EP&C ee send Content video caps using PPXC VideoStream

	BYTE* pH263CapSetBuf = new BYTE[LengthOfH263Buffer];
	CCapSetH263* pCapSetH263 = NULL;
	BYTE i = 0;

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

// 	if(::GetpSystemCfg()->IsEnableAnnexT())	//olga?
// 		pH263CapSetBuf[Annex_T] = 1;

	// QCIF-30
	pH263CapSetBuf[MinPictureHeight] = 17; // 176 x 144
	pH263CapSetBuf[MinPictureWidth]  = 21;
	pH263CapSetBuf[VideoFormat] = H263_QCIF_SQCIF;
	pH263CapSetBuf[MPI] = MPI_1;
	
	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

// 	if(::GetpSystemCfg()->IsEnableAnnexT())	//olga?
// 		pH263CapSetBuf[Annex_T] = 1;

	// CIF-30
	pH263CapSetBuf[MinPictureHeight] = 35; // 352 x 288
	pH263CapSetBuf[MinPictureWidth]  = 43;
	pH263CapSetBuf[VideoFormat] = H263_CIF;
	pH263CapSetBuf[MPI] = MPI_1;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

// 	if(::GetpSystemCfg()->IsEnableAnnexT())	//olga?
// 		pH263CapSetBuf[Annex_T] = 1;

	// 4CIF-15
	pH263CapSetBuf[MinPictureHeight] = 71; // 704 x 576
	pH263CapSetBuf[MinPictureWidth]  = 87;
	pH263CapSetBuf[VideoFormat] = H263_CIF_4;
	pH263CapSetBuf[MPI] = MPI_2;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

// 	if(::GetpSystemCfg()->IsEnableAnnexT())	//olga?
// 		pH263CapSetBuf[Annex_T] = 1;

	// VGA-15
	pH263CapSetBuf[MinPictureHeight] = 59; // 640 x 480
	pH263CapSetBuf[MinPictureWidth]  = 79; 
	pH263CapSetBuf[VideoFormat] = H263_CUSTOM_FORMAT;
	pH263CapSetBuf[ClockDivisor] = 60;   
	pH263CapSetBuf[ClockConversionCode] = 1;
	pH263CapSetBuf[CustomMPIIndicator] = MPI_2;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

// 	if(::GetpSystemCfg()->IsEnableAnnexT())	//olga?
// 		pH263CapSetBuf[Annex_T] = 1;

	// SVGA-10
	pH263CapSetBuf[MinPictureHeight] = 74; // 800 x 600
	pH263CapSetBuf[MinPictureWidth]  = 99;
	pH263CapSetBuf[VideoFormat] = H263_CUSTOM_FORMAT;
	pH263CapSetBuf[ClockDivisor] = 60;   
	pH263CapSetBuf[ClockConversionCode] = 1;
	pH263CapSetBuf[CustomMPIIndicator] = MPI_3;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

// 	if(::GetpSystemCfg()->IsEnableAnnexT())	//olga?
// 		pH263CapSetBuf[Annex_T] = 1;

	// XGA-7.5
	pH263CapSetBuf[MinPictureHeight] = 95; // 1024 x 768
	pH263CapSetBuf[MinPictureWidth]  = 127;
	pH263CapSetBuf[VideoFormat] = H263_CUSTOM_FORMAT;
	pH263CapSetBuf[ClockDivisor] = 60;   
	pH263CapSetBuf[ClockConversionCode] = 1;
	pH263CapSetBuf[CustomMPIIndicator] = MPI_4;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	delete [] pH263CapSetBuf;	
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////// CCapAMSC /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CCapAMSC::CCapAMSC(): m_AMSC_MUX(0), m_AMSC_MUX64_RateByte1(0), m_AMSC_MUX64_RateByte2(0)
{
}

/////////////////////////////////////////////////////////////////////////////
CCapAMSC::CCapAMSC(const CCapAMSC& other) : CPObject(other)
{
	m_AMSC_MUX				= other.m_AMSC_MUX;
	m_AMSC_MUX64_RateByte1	= other.m_AMSC_MUX64_RateByte1;
	m_AMSC_MUX64_RateByte2	= other.m_AMSC_MUX64_RateByte2;
}

/////////////////////////////////////////////////////////////////////////////
CCapAMSC::~CCapAMSC()
{
}

/////////////////////////////////////////////////////////////////////////////
const char* CCapAMSC::NameOf() const
{
    return "CCapAMSC";
}

/////////////////////////////////////////////////////////////////////////////
void CCapAMSC::Serialize(WORD format,CSegment &seg)
{
	switch( format ) {
	case SERIALEMBD :{
		if(m_AMSC_MUX == AMSC_MUX_CAP){
			seg << (BYTE)AMSC_MUX_CAP;
		}
		if( IsAMSC_MUX64() ){
			seg << (BYTE)AMSC_MUX64_CAP;
			seg << (BYTE)m_AMSC_MUX64_RateByte1;
			seg << (BYTE)m_AMSC_MUX64_RateByte2;
		}
		
		break;
	}
	case NATIVE: {
		seg << (BYTE)m_AMSC_MUX << (BYTE)m_AMSC_MUX64_RateByte1 
								<< (BYTE)m_AMSC_MUX64_RateByte2;
		
		break;
	}		
	default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapAMSC::DeSerialize(WORD format,CSegment &seg)
{
	switch( format ) {
	case SERIALEMBD :{
		PASSERT_AND_RETURN(format);
					 }
	case NATIVE: {
		seg >> m_AMSC_MUX  >> m_AMSC_MUX64_RateByte1 >> m_AMSC_MUX64_RateByte2;
				 }
	default: {
		break;}
	}
}

/////////////////////////////////////////////////////////////////////////////
CCapAMSC& CCapAMSC::operator=(const CCapAMSC& other)
{
	if(this == &other)  return *this;  

	m_AMSC_MUX				= other.m_AMSC_MUX;
	m_AMSC_MUX64_RateByte1	= other.m_AMSC_MUX64_RateByte1;
	m_AMSC_MUX64_RateByte2	= other.m_AMSC_MUX64_RateByte2;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CCapAMSC::Dump(std::ostream& ostr) const
{
	ostr << "\n===================    CCapAMSC::Dump    ===================\n" ;
	ostr << "\n  AMSC_MUX    \t: <"				<< (dec) << (WORD)m_AMSC_MUX  << ">";
	ostr << "\n  AMSC_MUX64_RateByte1 \t: <"	<< (dec) << (WORD)m_AMSC_MUX64_RateByte1  << ">";
	ostr << "\n  AMSC_MUX64_RateByte2 \t: <"	<< (dec) << (WORD)m_AMSC_MUX64_RateByte2  << ">";	
}

/////////////////////////////////////////////////////////////////////////////
void CCapAMSC::SetAMSC_MUX64(BYTE rateByte1, BYTE rateByte2)
{
	// MSB of AMSC_MUX64 rateByte1 must be zero
	if ( rateByte1 & 0x80 ) {
		PASSERT_AND_RETURN(rateByte1);
	}

	// 1. MSB of AMSC_MUX64 rateByte2 must be zero 
	// 2. Bits 3-7 of of AMSC_MUX64 rateByte2 must be zero
	if ( rateByte2 & (~(BYTE)AllRatesAMSC64Byte_2) ) {
		PASSERT_AND_RETURN(rateByte2);
	}

	m_AMSC_MUX64_RateByte1 = rateByte1;
	m_AMSC_MUX64_RateByte2 = rateByte2;
}

/////////////////////////////////////////////////////////////////////////////
void CCapAMSC::SetRateAMSC_MUX64(BYTE rate)
{
	if (rate > AMSC_1536k) {
		PASSERT_AND_RETURN(rate);
	}

	const BYTE MSB_ON_mask = 0x80;

	switch(rate){
	case AMSC_0k: {
		break;
	}
	case AMSC_64k: {
		m_AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_64k);
		break;
	}
	case AMSC_128k:	{		
		m_AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_128k);
		break;
	}
	case AMSC_192k:	{		
		m_AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_192k);
		break;
	}
	case AMSC_256k:	{		
		m_AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_256k);
		break;
	}
	case AMSC_384k:	{		
		m_AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_384k);
		break;
	}
	case AMSC_512k:	{		
		m_AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_512k);
		break;
	}
	case AMSC_768k:	{		
		m_AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_768k);
		break;
	}
	case AMSC_1152k:	{		
		m_AMSC_MUX64_RateByte2 |= (MSB_ON_mask >> AMSC_64k);
		break;
	}
	case AMSC_1536k:	{		
		m_AMSC_MUX64_RateByte2 |= (MSB_ON_mask >> AMSC_128k);
		break;
	}
	default: 
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapAMSC::SetAllAMSC_MUX64RatesUpTo(BYTE rate)
{
	if ( (rate > AMSC_1536k) || (!IsValidAMSC64Rate(rate)) ) {
		PASSERT_AND_RETURN(rate);
	}

	if (rate == AMSC_0k)
		return;

	const BYTE MSB_ON_mask = 0x80;

	if (rate >= AMSC_768k) { // Set rates in m_AMSC_MUX64_RateByte1 and in m_AMSC_MUX64_RateByte2
		SetAllRatesForRateByte1();
		// Set the rest of the rates in m_AMSC_MUX64_RateByte2
		if (rate==AMSC_1536k) {
			m_AMSC_MUX64_RateByte2 = AllRatesAMSC64Byte_2;
		}
		else { 
			if (rate==AMSC_1152k)
				m_AMSC_MUX64_RateByte2 |= (MSB_ON_mask >> 1);
		}
	}
	else { // Set rates only in m_AMSC_MUX64_RateByte1
		for(BYTE nextRate=rate; nextRate>AMSC_0k; nextRate--)
			m_AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> nextRate);
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapAMSC::IsAMSC64RateSupported (BYTE rate) const
{
	const BYTE MSB_ON_mask = 0x80;

	// AMSC_MUX64 is sub-group of AMSC_MUX.
	// If AMSC_MUX64 doesn't defined explicitly, but AMSC_MUX is defined - 
	// it means that all AMSC_MUX64 rates are supported
	if( (!m_AMSC_MUX64_RateByte1) && (!m_AMSC_MUX64_RateByte2) )
		return IsAMSC_MUX();

	switch(rate){
	case AMSC_0k:
	  return TRUE;
	case AMSC_64k:
	  if(m_AMSC_MUX64_RateByte1 & (MSB_ON_mask >> AMSC_64k))
		return TRUE;
	  break;
	case AMSC_128k:
	  if(m_AMSC_MUX64_RateByte1 & (MSB_ON_mask >> AMSC_128k))
		return TRUE;
	  break;
	case AMSC_192k:
	  if(m_AMSC_MUX64_RateByte1 & (MSB_ON_mask >> AMSC_192k))
		return TRUE;
	  break;
	case AMSC_256k:
	  if(m_AMSC_MUX64_RateByte1 & (MSB_ON_mask >> AMSC_256k))
		return TRUE;
	  break;
	case AMSC_384k:
	  if(m_AMSC_MUX64_RateByte1 & (MSB_ON_mask >> AMSC_384k))
		return TRUE;
	  break;
	case AMSC_512k:
	  if(m_AMSC_MUX64_RateByte1 & (MSB_ON_mask >> AMSC_512k))
		return TRUE;
	  break;
	case AMSC_768k:
	  if(m_AMSC_MUX64_RateByte1 & (MSB_ON_mask >> AMSC_768k))
		return TRUE;
	  break;
	case AMSC_1152k:
	  if(m_AMSC_MUX64_RateByte2 & (MSB_ON_mask >> AMSC_64k)) // second byte
		return TRUE;
	  break;
	case AMSC_1536k:
	  if(m_AMSC_MUX64_RateByte2 & (MSB_ON_mask >> AMSC_128k)) // second byte
		return TRUE;
	  break;
	default:
	  break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapAMSC::GetMaxAMSC64Rate(void) const
{
	for(BYTE rate = AMSC_1536k; rate>= AMSC_64k; rate--)
		if(IsAMSC64RateSupported(rate))
			return rate;
		
	return AMSC_0k;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapAMSC::IsValidAMSC64Rate(BYTE rate) const
{
	switch (rate) {
	case AMSC_0k:
	case AMSC_64k:
	case AMSC_128k:
	case AMSC_192k:
	case AMSC_256k:
	case AMSC_384k:
	case AMSC_512k:
	case AMSC_768k:
	case AMSC_1152k:
	case AMSC_1536k:
		return TRUE;
	default :
		return FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapAMSC::GetGreatestAMSC64RateLessThen(BYTE AMSC64Rate) const
{
	if (!IsValidAMSC64Rate(AMSC64Rate)) return AMSC_0k;

	for (BYTE i=(AMSC64Rate-1); i>=AMSC_64k; i--) {
		if (IsAMSC64RateSupported(i)) 
			return i;
	}
	return AMSC_0k;
}

/////////////////////////////////////////////////////////////////////////////
void CCapAMSC::SetAllRatesForRateByte1(void)
{
	m_AMSC_MUX64_RateByte1 = AllRatesAMSC64Byte_1;
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////// CCapPCProfile //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CCapPCProfile::CCapPCProfile():	m_profileListMask(0)
{
}
/////////////////////////////////////////////////////////////////////////////
CCapPCProfile::CCapPCProfile(const CCapPCProfile& other) : CPObject(other)
{
	m_profileListMask = other.m_profileListMask;
}
/////////////////////////////////////////////////////////////////////////////
CCapPCProfile::~CCapPCProfile()
{
}
/////////////////////////////////////////////////////////////////////////////
const char* CCapPCProfile::NameOf() const
{
    return "CCapPCProfile";
}
/////////////////////////////////////////////////////////////////////////////
void CCapPCProfile::Serialize(WORD format,CSegment &seg)
{
	switch( format ) {
	case SERIALEMBD :{
		if( m_profileListMask ) { 

			seg << PC_PROFILE_CAP;
			
			for (BYTE profileNumber=0; profileNumber <= MAX_PC_PROFILE; profileNumber++)
				if( IsProfileSupported(profileNumber) )
					seg << profileNumber;
		}
		break;
	}
	case NATIVE: {
		seg << m_profileListMask;
		break;
	}
	default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapPCProfile::DeSerialize(WORD format,CSegment &seg)
{	                                       
	switch ( format ) {    
	case SERIALEMBD :{       		   
		PASSERT_AND_RETURN(format);
		break;
	}
	case NATIVE: {
		seg >> m_profileListMask;
		break;
	}
	default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapPCProfile::Dump(std::ostream& ostr) const
{
	ostr << "\n===================    CCapPCProfile::Dump    ===================\n" ;
	for (BYTE  i=0 ; i < MAX_PC_PROFILE ; i++) {
		if (m_profileListMask & (1L << i))
			ostr << "\n  Profile_"   << (dec)<< (WORD)i;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapPCProfile::SetProfile(BYTE profileNumber)
{
	if (profileNumber > MAX_PC_PROFILE) {
		PASSERT_AND_RETURN(profileNumber);
	}

	m_profileListMask = (1L << profileNumber);
}

/////////////////////////////////////////////////////////////////////////////	
BYTE CCapPCProfile::IsProfileSupported(BYTE profileNumber)
{
	if (profileNumber > MAX_PC_PROFILE) {
		PASSERT(profileNumber);
		return FALSE;
	}

	if ( m_profileListMask & (1L << profileNumber) )
		return TRUE;
	else
		return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
////////////////////////// CVideoStreamPPXC /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CVideoStreamPPXC::CVideoStreamPPXC(): m_pLabelList(NULL), m_labelListSize(0)
// {
// }
// /////////////////////////////////////////////////////////////////////////////	
// CVideoStreamPPXC::CVideoStreamPPXC(const CVideoStreamPPXC &other)
// {
// 	PDELETEA(m_pLabelList);
//     m_labelListSize = 0;
	
// 	if (other.m_labelListSize > 0) {
// 		m_labelListSize = other.m_labelListSize;
//         m_pLabelList = new BYTE[m_labelListSize];
//         PASSERT(m_pLabelList == NULL);
//         for (int i=0 ; i < m_labelListSize ; i++)
// 			m_pLabelList[i] = other.m_pLabelList[i];
//     }	
// 	m_h263cap = other.m_h263cap;
// 	m_h261cap = other.m_h261cap;
// }
// /////////////////////////////////////////////////////////////////////////////	
// CVideoStreamPPXC::~CVideoStreamPPXC()
// {
// 	PDELETEA(m_pLabelList);
// }
/////////////////////////////////////////////////////////////////////////////	
const char* CVideoStreamPPXC::NameOf()  const
{
    return "CVideoStreamPPXC";
}
/////////////////////////////////////////////////////////////////////////////	
void CVideoStreamPPXC::Serialize(WORD format, CSegment &seg)
{	
	switch(format){
	case SERIALEMBD : {

		/////////////////////////////////////////////////////////////////////////
		// NOTE: only H263 caps are now sent by MCU in VideoStreamPPXC message!!!
		// =====
		// PPXC is now sent only in EP&C calls and we support Content only at H263.
		/////////////////////////////////////////////////////////////////////////

		BYTE msgLength = 0;

		if( (!m_labelListSize) || (!m_h263cap.GetNumberOfH263Sets()) ) return;
		
		CSegment *segTemp = new CSegment;

		*segTemp << EXTENDED_PP_CAP_VIDEO_STREAM;		
		
		for(int i = 0; i < m_labelListSize; i++ ) {
			// Set terminator bit ON for the last label in the list
			if (i == (m_labelListSize-1))
				*segTemp << (BYTE)(m_pLabelList[i] | RoleLabelTerminatorMask);
			else
				*segTemp << m_pLabelList[i];
		}
		
		*segTemp << OPTIONAL_CAP_INDICATOR;		
		
		{
		CSegment *segH263 = new CSegment;
		BYTE capLenH263 = 0;
		m_h263cap.Serialize(format, *segH263);
		capLenH263 = (BYTE)segH263->GetWrtOffset();
		*segTemp << capLenH263 << *segH263;
		POBJDELETE(segH263);
		}
				
		msgLength = (BYTE)segTemp->GetWrtOffset();
		seg << msgLength << *segTemp;
		POBJDELETE(segTemp);
		
		break;
	}
	case NATIVE: {
		
		seg << m_labelListSize;

		if(m_labelListSize){
			for(int i = 0; i < m_labelListSize; i++ )
				seg << m_pLabelList[i];
		}
		
		m_h263cap.Serialize(format, seg);
		
		m_h261cap.Serialize(format, seg);
		
		break;		
	}
	}
}

/////////////////////////////////////////////////////////////////////////////	
void CVideoStreamPPXC::DeSerialize(WORD format, CSegment &seg, BYTE dataLen)
{	
	switch( format ) {
	case SERIALEMBD : {

		BYTE readBytesNum = 0, capLen = 0;
		BYTE basCode;
		seg >> basCode;
		readBytesNum++;

		//***** Start Label list parsing
		AddLabel(basCode);
		while( !(basCode & RoleLabelTerminatorMask) ) {
			seg >> basCode;
			readBytesNum++;
			AddLabel(basCode);
		}
		//***** End Label list parsing

		// After label list should go Optional capability indicator - 0x0C
		seg >> basCode;
		readBytesNum++;
		if (basCode != OPTIONAL_CAP_INDICATOR) {
			// skip all remained CVideoStreamPPXC msg bytes
			for (BYTE i=0; i<(dataLen-readBytesNum); i++)
				seg >> basCode;
			DBGPASSERT_AND_RETURN (readBytesNum+1);
		}
		seg >> capLen;
		readBytesNum++;
		DeSerializeEmbededVideoCaps(dataLen, readBytesNum, seg);

		break;
		}
		case NATIVE : {
			seg >> m_labelListSize;
			PDELETEA(m_pLabelList);
            if (m_labelListSize > 0) {
                m_pLabelList = new BYTE[m_labelListSize];
				PASSERT(m_pLabelList == NULL);
				
                for (int i=0 ; i < m_labelListSize ; i++)
                    seg >> m_pLabelList[i];
			}
			
			m_h263cap.DeSerialize(format, seg);
			
			m_h261cap.DeSerialize(format, seg);			
			
			break;
					  }			
		default : {
			break;
				  }
	}//switch	
}
/////////////////////////////////////////////////////////////////////////////	
// CVideoStreamPPXC& CVideoStreamPPXC::operator=(const CVideoStreamPPXC& other)
// {
// 	if (this == &other) return *this;

// 	PDELETEA(m_pLabelList);
//     m_labelListSize = 0;
	
// 	if (other.m_labelListSize > 0) {
// 		m_labelListSize = other.m_labelListSize;
//         m_pLabelList = new BYTE[m_labelListSize];
//         PASSERT(m_pLabelList == NULL);
//         for (int i=0 ; i < m_labelListSize ; i++)
// 			m_pLabelList[i] = other.m_pLabelList[i];
//     }
	
// 	m_h263cap = other.m_h263cap;
// 	m_h261cap = other.m_h261cap;

// 	return *this;
// }

/////////////////////////////////////////////////////////////////////////////	
void CVideoStreamPPXC::CreateContentVideoCapEPC(void)
{
	// Set Content label
	PDELETEA(m_pLabelList);

    m_labelListSize = 1;
	m_pLabelList = new BYTE[m_labelListSize];
	m_pLabelList[0] = (BYTE)(ContentLabel | RoleLabelTerminatorMask);

	CreateVideoCaps();
	/*
	// Create H263 caps for Content - QCIF-30, CIF-30, 4CIF-15, VGA-15, SVGA-10, XGA-7.5, Annex_T.
	// In EP&C ee send Content video caps using PPXC VideoStream

	BYTE* pH263CapSetBuf = new BYTE[LengthOfH263Buffer];
	CCapSetH263* pCapSetH263 = NULL;
	BYTE i = 0;

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

	pH263CapSetBuf[Annex_T] = 1;

	// QCIF-30
	pH263CapSetBuf[MinPictureHeight] = 17; // 176 x 144
	pH263CapSetBuf[MinPictureWidth]  = 21;
	pH263CapSetBuf[VideoFormat] = H263_QCIF_SQCIF;
	pH263CapSetBuf[MPI] = MPI_1;
	
	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

	pH263CapSetBuf[Annex_T] = 1;

	// CIF-30
	pH263CapSetBuf[MinPictureHeight] = 35; // 352 x 288
	pH263CapSetBuf[MinPictureWidth]  = 43;
	pH263CapSetBuf[VideoFormat] = H263_CIF;
	pH263CapSetBuf[MPI] = MPI_1;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

	pH263CapSetBuf[Annex_T] = 1;

	// 4CIF-15
	pH263CapSetBuf[MinPictureHeight] = 71; // 704 x 576
	pH263CapSetBuf[MinPictureWidth]  = 87;
	pH263CapSetBuf[VideoFormat] = H263_CIF_4;
	pH263CapSetBuf[MPI] = MPI_2;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

	pH263CapSetBuf[Annex_T] = 1;

	// VGA-15
	pH263CapSetBuf[MinPictureHeight] = 59; // 640 x 480
	pH263CapSetBuf[MinPictureWidth]  = 79; 
	pH263CapSetBuf[VideoFormat] = H263_CUSTOM_FORMAT;
	pH263CapSetBuf[ClockDivisor] = 60;   
	pH263CapSetBuf[ClockConversionCode] = 1;
	pH263CapSetBuf[CustomMPIIndicator] = MPI_2;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

	pH263CapSetBuf[Annex_T] = 1;

	// SVGA-10
	pH263CapSetBuf[MinPictureHeight] = 74; // 800 x 600
	pH263CapSetBuf[MinPictureWidth]  = 99;
	pH263CapSetBuf[VideoFormat] = H263_CUSTOM_FORMAT;
	pH263CapSetBuf[ClockDivisor] = 60;   
	pH263CapSetBuf[ClockConversionCode] = 1;
	pH263CapSetBuf[CustomMPIIndicator] = MPI_3;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	for(i = 0; i < LengthOfH263Buffer; i++)
		pH263CapSetBuf[i] = 0;

	pH263CapSetBuf[Annex_T] = 1;

	// XGA-7.5
	pH263CapSetBuf[MinPictureHeight] = 95; // 1024 x 768
	pH263CapSetBuf[MinPictureWidth]  = 127;
	pH263CapSetBuf[VideoFormat] = H263_CUSTOM_FORMAT;
	pH263CapSetBuf[ClockDivisor] = 60;   
	pH263CapSetBuf[ClockConversionCode] = 1;
	pH263CapSetBuf[CustomMPIIndicator] = MPI_4;

	pCapSetH263 = new CCapSetH263;
	pCapSetH263->Create(pH263CapSetBuf);
	m_h263cap.InsertH263CapSet(pCapSetH263);

	delete [] pH263CapSetBuf;
	*/
}

/////////////////////////////////////////////////////////////////////////////	
void CVideoStreamPPXC::AddLabel(BYTE newLabel)
{
	BYTE clearLabel = newLabel;
	// switch OFF terminator bit is it is ON
	if (newLabel & RoleLabelTerminatorMask)
		clearLabel = (newLabel & (~RoleLabelTerminatorMask));

	switch (clearLabel) {
	case PeopleLabel:
	case ContentLabel: {
		m_labelListSize++;
		BYTE *pNewLabelList = new BYTE[m_labelListSize];

		for (BYTE i=0; i<(m_labelListSize-1); i++)
			pNewLabelList[i] = m_pLabelList[i];

		pNewLabelList[m_labelListSize-1] = clearLabel;

		PDELETEA(m_pLabelList);

		m_pLabelList = pNewLabelList;
		break;
					   }
	default: {
		ALLOCBUFFER(string,10);
		sprintf(string,"%02x",clearLabel);
		PTRACE2(eLevelError,"CVideoStreamPPXC::AddLabel: Unknow Label: ",string);//EXCEPTION_TRACE|EPC_TRACE
		DEALLOCBUFFER(string);
	}
	}
}

/////////////////////////////////////////////////////////////////////////////	
//void CVideoStreamPPXC::Dump(std::ostream& ostr) const
//{
/*		ostr << "\n===================    CVideoStreamPPXC::Dump    ===================\n" ;

		ostr << "\n  Labels Number     \t: <"   << (dec) << (BYTE)m_labelListSize  << ">";
		for (BYTE  i=0 ; i < m_labelListSize ; i++)
             ostr << "\n  Label     \t: <"   << (dec)<< (BYTE)i << "  =  " << (dec) << (BYTE)m_labelList[i] << ">";

		ostr << "\n  Cap Indicator     \t: <"   << (hex) << (BYTE)m_optionalCapIndicator  << ">";
		ostr << "\n  Cap Length     \t: <"   << (dec) << (BYTE)m_CapLength  << ">";

		m_h263cap.Dump(ostr);

		m_vidModeCap.Dump(ostr);
		//ostr << "\n Video Cap\t: <"   << (hex) << (DWORD)m_h261cap << ">";
	
	
		m_nsVideoCap.Dump(ostr);
*/
//}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// CCapPPXC ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CCapPPXC::CCapPPXC() : m_numOfItemsVideoStreamPPXC(0), m_pVideoStreamPPXC(NULL)
{
}
/////////////////////////////////////////////////////////////////////////////	
CCapPPXC::CCapPPXC(const CCapPPXC &other) : CPObject(other)
{
	PDELETEA(m_pVideoStreamPPXC);
	m_numOfItemsVideoStreamPPXC = 0;
	
	if(other.m_numOfItemsVideoStreamPPXC){
		m_numOfItemsVideoStreamPPXC  = other.m_numOfItemsVideoStreamPPXC;
		
		m_pVideoStreamPPXC = new CVideoStreamPPXC[m_numOfItemsVideoStreamPPXC];
		
		for( int i=0; i<m_numOfItemsVideoStreamPPXC; i++ ) 
			m_pVideoStreamPPXC[i] = other.m_pVideoStreamPPXC[i];
	}
}

/////////////////////////////////////////////////////////////////////////////	
CCapPPXC::~CCapPPXC()
{
	PDELETEA(m_pVideoStreamPPXC);
}

/////////////////////////////////////////////////////////////////////////////	
/*CCapPPXC_Item* CCapPPXC::operator[]( const WORD index ) const
{
	if( index >= m_itemsNumber ) {
		PASSERT(index);
		return NULL;
	}

	return m_pCapPPXC_Items[index];
}
*/
/////////////////////////////////////////////////////////////////////////////	
const char* CCapPPXC::NameOf()  const
{
    return "CCapPPXC";
}

/////////////////////////////////////////////////////////////////////////////	
void CCapPPXC::Serialize(WORD format,CSegment &seg)
{	
	int i;
	
	switch( format ) {
	case SERIALEMBD : {
		for( i=0; i<m_numOfItemsVideoStreamPPXC; i++ ) {
			seg << PPXC_CAP;
			m_pVideoStreamPPXC[i].Serialize(format,seg);
		}	
		break;
	}
	case NATIVE : {
		seg << m_numOfItemsVideoStreamPPXC;
		for( i=0; i<m_numOfItemsVideoStreamPPXC; i++ ) {
			m_pVideoStreamPPXC[i].Serialize(format,seg);
		}
		break;
	}
	default :
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////	
void CCapPPXC::DeSerialize(WORD format,CSegment &seg)
{	
	switch( format ) {
	case SERIALEMBD : {
		BYTE PPXCmsgLen;
		BYTE basCode;

		seg >> PPXCmsgLen;
		seg >> basCode;

		// Only one PPXC code exist for now VideoStream
		if (basCode != EXTENDED_PP_CAP_VIDEO_STREAM) {
			// skip umknown PPXC msg bytes
			for (BYTE i=0; i<(PPXCmsgLen-1); i++)
				seg >> basCode;
			DBGPASSERT_AND_RETURN (basCode+1);
		}
		else { // parse VideoStrea PPXC msg

			// Recreate & initialize data members
			PDELETEA(m_pVideoStreamPPXC);

			m_numOfItemsVideoStreamPPXC = 1;
			m_pVideoStreamPPXC = new CVideoStreamPPXC[m_numOfItemsVideoStreamPPXC];

			m_pVideoStreamPPXC->DeSerialize(format, seg, PPXCmsgLen-1);			
		}

		break;
	}
	case NATIVE : {
		seg >> m_numOfItemsVideoStreamPPXC;

		PDELETEA(m_pVideoStreamPPXC);

		if( m_numOfItemsVideoStreamPPXC == 0 ) {
			break;
		}
				
		m_pVideoStreamPPXC = new CVideoStreamPPXC[m_numOfItemsVideoStreamPPXC];
		
		for(int i=0; i<m_numOfItemsVideoStreamPPXC; i++)
			m_pVideoStreamPPXC[i].DeSerialize(format,seg);

		break;
	}
	default :
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////	
void CCapPPXC::Dump(std::ostream& ostr) const
{
}

/////////////////////////////////////////////////////////////////////////////	
CCapPPXC& CCapPPXC::operator=(const CCapPPXC &other)
{
	if( this == &other )
		return *this;
	
	PDELETEA(m_pVideoStreamPPXC);
	m_numOfItemsVideoStreamPPXC = 0;
	
	if(other.m_numOfItemsVideoStreamPPXC){
		m_numOfItemsVideoStreamPPXC  = other.m_numOfItemsVideoStreamPPXC;
		
		m_pVideoStreamPPXC = new CVideoStreamPPXC[m_numOfItemsVideoStreamPPXC];
		
		for( int i=0; i<m_numOfItemsVideoStreamPPXC; i++ ) 
			m_pVideoStreamPPXC[i] = other.m_pVideoStreamPPXC[i];
	}
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CCapPPXC::CreateContentVideoCapEPC(void)
{
	// Now only one VideoStreamPPXC item can be allocated - EPC Content video caps.
	// That's why when creating new Content video caps we delete the previous one
	PDELETEA(m_pVideoStreamPPXC);
	m_numOfItemsVideoStreamPPXC = 0;

	CVideoStreamPPXC* pVideoStreamPPXC = new CVideoStreamPPXC;

	pVideoStreamPPXC->CreateContentVideoCapEPC();

	AddVideoStreamPPXCItem(pVideoStreamPPXC);

	PDELETE(pVideoStreamPPXC);
}

/////////////////////////////////////////////////////////////////////////////
void CCapPPXC::AddVideoStreamPPXCItem(CVideoStreamPPXC* pNewVideoStreamPPXC)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pNewVideoStreamPPXC));

	if ( m_numOfItemsVideoStreamPPXC == 0 ) {
		m_numOfItemsVideoStreamPPXC = 1;
		m_pVideoStreamPPXC = new CVideoStreamPPXC[1];
		m_pVideoStreamPPXC[m_numOfItemsVideoStreamPPXC-1] = *pNewVideoStreamPPXC;
	}
	else {
		m_numOfItemsVideoStreamPPXC++;

		CVideoStreamPPXC *pTempVideoStreamPPXC = new CVideoStreamPPXC[m_numOfItemsVideoStreamPPXC];

		// copy old VideoStreamPPXC items
		for (int i=0; i<(m_numOfItemsVideoStreamPPXC-1); i++)
			pTempVideoStreamPPXC[i] = m_pVideoStreamPPXC[i];

		// add new VideoStreamPPXC items
		pTempVideoStreamPPXC[m_numOfItemsVideoStreamPPXC-1] = *pNewVideoStreamPPXC;

		PDELETEA(m_pVideoStreamPPXC);

		m_pVideoStreamPPXC = pTempVideoStreamPPXC;
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// CCapPP //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CCapPP::CCapPP() : m_capMediaFlowControlH320(0), m_capPPh221EscapeTable(0)
{	
}
/////////////////////////////////////////////////////////////////////////////	
CCapPP::CCapPP(const CCapPP& other) : CPObject(other)
{
	m_capPCProfile				= other.m_capPCProfile;
	m_capAMSC					= other.m_capAMSC;	
	m_capPPXC					= other.m_capPPXC;
	m_capMediaFlowControlH320	= other.m_capMediaFlowControlH320;
	m_capPPh221EscapeTable		= other.m_capPPh221EscapeTable;	
}

/////////////////////////////////////////////////////////////////////////////	
CCapPP::~CCapPP()
{
}
/////////////////////////////////////////////////////////////////////////////	
const char* CCapPP::NameOf() const
{
    return "CCapPP";
}
/////////////////////////////////////////////////////////////////////////////	
void CCapPP::Serialize(WORD format,CSegment &seg)
{
	switch( format ) {
	case SERIALEMBD : {
		CSegment  *segTemp  = new CSegment;
		BYTE msgLen = 0;
		
		if(	m_capMediaFlowControlH320 == MEDIA_FLOW_CONTROL_CAP )
		{
			*segTemp << m_capMediaFlowControlH320;
		}
		if(m_capPPh221EscapeTable == PP_H221_ESCAPE_TABLE_CAP)	{
			*segTemp << m_capPPh221EscapeTable;
		}

		m_capPCProfile.Serialize(format,*segTemp);
		
		m_capAMSC.Serialize(format,*segTemp);
		
		m_capPPXC.Serialize(format,*segTemp);
		
		msgLen = (BYTE)segTemp->GetWrtOffset();
			
		// if message length more than 0 - there is data to send
		if(msgLen) {
			seg << (BYTE) ( ESCAPECAPATTR | Ns_Cap );
			
			seg << (BYTE) (4 + msgLen );// msgLen
			
			seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
				<< (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
			seg << (BYTE)( W_MANUFACT_CODE_PUBLIC_PP & 0xFF )         // manufacturer code junior byte
				<< (BYTE)( (W_MANUFACT_CODE_PUBLIC_PP >> 8) & 0xFF ); // manufacturer code senior byte
			
			seg << *segTemp;
		}

		POBJDELETE(segTemp);
		break;
	}	
	case NATIVE : {
		seg << m_capMediaFlowControlH320 << m_capPPh221EscapeTable;
		m_capPCProfile.Serialize(format,seg);
		m_capAMSC.Serialize(format,seg);
		m_capPPXC.Serialize(format,seg);
		break;
	}
	default : {
		break;
	}
	}
}

/////////////////////////////////////////////////////////////////////////////	
void CCapPP::DeSerialize(WORD format,CSegment &seg)
{	
	switch( format ) {
	case SERIALEMBD : {
		BYTE msgLen = 0;
		BYTE country1, country2, manufCode1, manufCode2;
		BYTE basCode; 
		
		seg >> msgLen ;// msgLen
		if(msgLen > NS_MSG_HEADER_LEN){
			msgLen -= NS_MSG_HEADER_LEN;
			seg >> country1    // country code junior byte
				>> country2;   // country code senior byte
			seg >> manufCode1  // manufacturer code junior byte
				>> manufCode2; // manufacturer code senior byte
			if( (country1 == (BYTE)COUNTRY_CODE_USA_BYTE_1) && 
				(country2 == (BYTE)COUNTRY_CODE_USA_BYTE_2) &&
				(manufCode1 ==  (BYTE)MANUFACT_CODE_PUBLIC_PP_BYTE_1) && 
				(manufCode2 == (BYTE)MANUFACT_CODE_PUBLIC_PP_BYTE_2) ) {

				while (msgLen>0 && !seg.EndOfSegment()) {
					seg >> basCode;
					msgLen--;
					switch (basCode) {
					case PC_PROFILE_CAP: {
						CSegment  *pCopySeg = new CSegment;
						BYTE pcProfileListLen = 0;
						*pCopySeg = seg;
						*pCopySeg >> basCode;

						while (basCode <= MAX_PC_PROFILE) {
							pcProfileListLen++;
							m_capPCProfile.SetProfile(basCode);
							if( (msgLen-pcProfileListLen) == 0 )
								break;
							*pCopySeg >> basCode;
						}

						// get out all profile bytes from the basic cap segment
						for (int j=0; j<pcProfileListLen; j++)
							seg >> basCode;

						msgLen -= pcProfileListLen;
						POBJDELETE(pCopySeg);
						break;
					}
					case AMSC_MUX64_CAP: {
						BYTE rateByte1, rateByte2;
						seg >> rateByte1 >> rateByte2;
						m_capAMSC.SetAMSC_MUX64(rateByte1, rateByte2);
						msgLen -= 2;
						break;
					}
					case AMSC_MUX_CAP: {
						m_capAMSC.SetAMSC_MUX();
						break;
					}
					case PPXC_CAP: {
						BYTE msgLenPPXC = 0;
						CSegment  *pCopySeg = new CSegment;
						*pCopySeg = seg;
						*pCopySeg >> msgLenPPXC;
						msgLen--;
						msgLen -= msgLenPPXC;
						POBJDELETE(pCopySeg);

						m_capPPXC.DeSerialize(format, seg);

						break;
					}
					case MEDIA_FLOW_CONTROL_CAP: {
						m_capMediaFlowControlH320 = MEDIA_FLOW_CONTROL_CAP; 
						break;
					}
					case PP_H221_ESCAPE_TABLE_CAP: {
						m_capPPh221EscapeTable = PP_H221_ESCAPE_TABLE_CAP; 
						break;
					}
					default: {
						ALLOCBUFFER(string,10);
						sprintf(string,"%02x",basCode);
						PTRACE2(eLevelError,"CCapPP::DeSerialize : \' UNKNOWN BAS\': ",string);//EXCEPTION_TRACE|EPC_TRACE
						DEALLOCBUFFER(string);
						break;
					}
					}
				}
			}
			else
				DBGPASSERT(country1);
		}
		else {
			ALLOCBUFFER(str,ONE_LINE_BUFFER_LEN);
			sprintf(str,"%d",(int)msgLen);
			PTRACE2(eLevelError,"CCapPP::DeSerialize - illegal MsgLen in NS_CAP msg - ",str);//EXCEPTION_TRACE
			DEALLOCBUFFER(str);
		}
		break;
	}
	case NATIVE : {
		seg >> m_capMediaFlowControlH320 >> m_capPPh221EscapeTable;
		m_capPCProfile.DeSerialize(format,seg);
		m_capAMSC.DeSerialize(format,seg);
		m_capPPXC.DeSerialize(format,seg);
		break;
	}
	default :
		break;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapPP::CreateCapEPC(WORD callRate, WORD isTranscoding,WORD isLSD,WORD ContentLevel)
{
	////////////////////////////////////////////////////////////////////////////////
	// Fixed common cap set is sent now by MCU for EP&C calls:
	// 1. Profile_2 
	// 2. AMSC_MUX64 with rates accoring to callRate and isTranscoding flag
	// 3. H263 caps for Content - QCIF-30, CIF-30, 4CIF-15, VGA-15, SVGA-10, XGA-7.5, 
	//							  Annex_T
	////////////////////////////////////////////////////////////////////////////////

	/******************   ContentRateControl Table:  ********************************
	________________________________________________________________________________
	|Conf bps |64 | 128| 192  | 256   | 320   | 384| 512| 768| 1152| 1472| 1536| 1920|
	|_________|___|____|______|_______|_______|____|____|____|_____|_____|_____|_____|
	|Graphics |0  |64/0| 64   | 64    | 128   | 128| 128| 256| 256 | 256 | 256 | 256 |  
	|_________|___|____|______|_______|_______|____|____|____|_____|_____|_____|_____|
	|Hi Res   |0  |64/0| 64   | 128   | 192   | 192| 256| 384| 384 | 512 | 512 | 512 |
	|Graphics |   |    |      |       |       |    |    |    |     |     |     |     |
	|_________|___|____|______|_______|_______|____|____|____|_____|_____|_____|_____|
	|LiveVideo|0  |64/0|128/64|192/128|256/192| 256| 384| 512| 512 | 768 | 768 | 768 |
	|_________|___|____|______|_______|_______|____|____|____|_____|_____|_____|_____|

  
     Graphics - the old division of content rate
 
	*********************************************************************************/

	PTRACE(eLevelInfoNormal,"CCapPP::CreateCapEPC ");

	WORD ContentRate = AMSC_0k;

	// Set pcProfile 2
	m_capPCProfile.SetProfile(2);
	
	switch (callRate) {
		case Xfer_128	:
			{ 
				if(!isLSD)
				ContentRate = AMSC_64k;
				break;
			}
		case Xfer_192	: 
			{ 
				if((ContentLevel == eLiveVideo) && (!isLSD))
					ContentRate = AMSC_128k;
				else 
					ContentRate = AMSC_64k;
			 
				break;
			}	
        case Xfer_256	:
			{ 
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_64k;
				}
				else
				{
					if(ContentLevel == eHiResGraphics)
					{
						ContentRate = AMSC_128k;
					}
					else
					{
						if(isLSD)
							ContentRate = AMSC_128k;
						else
							ContentRate = AMSC_192k;
					}
				}
				break;
			}
		case Xfer_320	:
			{ 
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_128k;
				}
				else
				{
					if(ContentLevel == eHiResGraphics)
					{
						ContentRate = AMSC_192k;
					}
					else
					{
						if(isLSD)
							ContentRate = AMSC_192k;
						else
							ContentRate = AMSC_256k;
					}
				}
				break;
			}
		case Xfer_384	:
			{ 
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_128k;
				}
				else
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMSC_192k;
					else
						ContentRate = AMSC_256k;
				}
				break;
			}
		case Xfer_512	: 
			{ 
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_128k;
				}
				else 
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMSC_256k;
					else
						ContentRate = AMSC_384k;
				}
				break;
			}
		case Xfer_768	: 
			{ 
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_256k;
				}
				else 
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMSC_384k;
					else
						ContentRate = AMSC_512k;
				}
				break;
			}
		case Xfer_1152	: 
			{
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_256k;
				}
				else 
				{
					if(ContentLevel ==eHiResGraphics)
						ContentRate = AMSC_384k;
					else
						ContentRate = AMSC_512k;
					
				}
				break;
			}
		case Xfer_1472	: 
		case Xfer_1536	:
		case Xfer_1920	:
			{ 
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_256k;
				}
				else 
				{
					if(ContentLevel ==eHiResGraphics)
						ContentRate = AMSC_512k;
					else
						ContentRate = AMSC_768k;					
				}
				break;
			}
        default	: { 
			if (callRate)	
				PASSERT(callRate);
			else
				PASSERT(101);
			break; 
				  }
	}

	if (isTranscoding)
	{
		// In transcoding we add all possible Content rates for the specified call rate.  
		m_capAMSC.SetAllAMSC_MUX64RatesUpTo(ContentRate);				
	}
	else
		m_capAMSC.SetRateAMSC_MUX64(ContentRate);		
		

 
	// Set H263 caps for Content - QCIF-30, CIF-30, 4CIF-15, VGA-15, SVGA-10, XGA-7.5, Annex_T.
	// We send Content video caps using PPXC VideoStream
	m_capPPXC.CreateContentVideoCapEPC();
}

/////////////////////////////////////////////////////////////////////////////	
CCapPP& CCapPP::operator=(const CCapPP& other)
{
	if( this == &other )
		return *this;

	m_capPCProfile				= other.m_capPCProfile;
	m_capAMSC					= other.m_capAMSC;	
	m_capPPXC					= other.m_capPPXC;
	m_capMediaFlowControlH320	= other.m_capMediaFlowControlH320;
	m_capPPh221EscapeTable		= other.m_capPPh221EscapeTable;	

	return *this;
}
/////////////////////////////////////////////////////////////////////////////	
BYTE CCapPP::IsCapEPC(void)
{
	return m_capPCProfile.IsProfileSupported(2);
}

/////////////////////////////////////////////////////////////////////////////	
/*
void CCapPP::AddEnterprisePeopleAndContent(WORD callRate, WORD isTranscoding)
{
	BYTE EPC_CapBuff[20];
	BYTE AMSC_MUX64_RateByte1 = 0;
	// AMSC_MUX64_RateByte2 is set to 0 because we currrently support only following Content rates:
	// AMSC_64k, AMSC_128k, AMSC_256k. All these rates are in AMSC_MUX64_RateByte1. 
	// ATTENTION!!! - When in future AMSC_MUX64_RateByte2 will have non-zero value, you can NOT
	//                use values from H221.H as bit position identifier. For example, value of
	//                AMSC_64k in H221.H is 1. And it is corresponds to bit position 1 in 
	//                AMSC_MUX64_RateByte1. So, we can use this value in this function for bit position
	//                operations. On the other hand, AMSC_1152k has value 8 in H221.H, but its
	//                bit position is 1 in AMSC_MUX64_RateByte2. Therefore we can not use it
	//                here for bit position operations.
	const BYTE AMSC_MUX64_RateByte2 = 0; 
	const BYTE MSB_ON_mask = 0x80;
	
	memset(EPC_CapBuff,0x00,20);

	switch (callRate) {
		case Xfer_192	: 
        case Xfer_256	: {
			AMSC_MUX64_RateByte1 = (MSB_ON_mask >> AMSC_64k);
			break;
							}
        case Xfer_320	: 
        case Xfer_512	: 
        case Xfer_384	: {
			AMSC_MUX64_RateByte1 = (MSB_ON_mask >> AMSC_128k);
			if (isTranscoding) {
				// In transcoding we add all possible Content rates for the specified call rate.  
				AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_64k);				
			}
			break;
							}
        case Xfer_768	: 
        case Xfer_1152	: 
        case Xfer_1472	: 
        case Xfer_1536	: 
        case Xfer_1920	: {
			AMSC_MUX64_RateByte1 = (MSB_ON_mask >> AMSC_256k);
			if (isTranscoding) {
				// In transcoding we add all possible Content rates for the specified call rate.  
				AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_64k);				
				AMSC_MUX64_RateByte1 |= (MSB_ON_mask >> AMSC_128k);				
			}

			break;
							}
        default	: { 
			if (callRate)	
				PASSERT(callRate);
			else
				PASSERT(1);
			break; 
				  }
	}

				EPC_CapBuff[0] = (BYTE)0x9a; // PC-Profile
				EPC_CapBuff[1] = (BYTE)0x2;  // Profile_2
				EPC_CapBuff[2] = (BYTE)0x9c; //0x9c - AMSC_MUX64
				EPC_CapBuff[3] = AMSC_MUX64_RateByte1;
				EPC_CapBuff[4] = AMSC_MUX64_RateByte2;

		
				EPC_CapBuff[5] = (BYTE)0x90 ;// 0x90	-	PPXP BAS code
				EPC_CapBuff[6] = (BYTE)0x0b;//0x08	-	PPXC length
				EPC_CapBuff[7] = (BYTE)0x8b ; //0x8B	-	VideoStream BAS code
				EPC_CapBuff[8] = (BYTE)0x22;//0x22	-	Content label with terminator bit set to 1
				EPC_CapBuff[9] = (BYTE)0x0c; //0x0C	-	Optional Capability  Indicator BAS code
				EPC_CapBuff[10] = (BYTE)0x07;//0x04	-	Optional Capability List Length
                EPC_CapBuff[11] = (BYTE)0xf9 ;//0xF9	- 	Start_Mbe BAS code
				EPC_CapBuff[12] = (BYTE)0x02; //0x02	- 	Mbe length
				EPC_CapBuff[13] = (BYTE)0x0a;//0x0A	- 	H230 <H262/H263> BAS code
				EPC_CapBuff[14] = (BYTE)0x8c;//0x8C	-	H263_4CIF at 15fps

				EPC_CapBuff[15] = DATAVIDCAPATTR | V_Cif;
				EPC_CapBuff[16] = DATAVIDCAPATTR | V_1_29_97;
				EPC_CapBuff[17] = DATAVIDCAPATTR | V_1_29_97;
				
		
//	AppendNsCap( W_COUNTRY_CODE_USA, W_MANUFACT_CODE_PUBLIC_PP, EPC_CapBuff,15 );
					AppendPPCap(  EPC_CapBuff,18 );

}
/////////////////////////////////////////////////////////////////////////////	
void CCapPP::AddPPcapEPC()
{
	BYTE	bytes[36];
	memset(bytes,0x00,36);


				bytes[0] = (BYTE)0x9a;
				bytes[1] = (BYTE)0x2;
				bytes[2] = (BYTE)0x9c; //0x9c
				bytes[3] = (BYTE)0x60;//0x60
				bytes[4] = (BYTE)0;//0x0

		
				bytes[5] = (BYTE)0x90 ;// 0x90	-	PPXP BAS code
				bytes[6] = (BYTE)0x0b;//0x08	-	PPXC length>>>>>>>
				bytes[7] = (BYTE)0x8b ; //0x8B	-	VideoStream BAS code
			
				bytes[8] = (BYTE)0x22;//0x22	-	Content label with terminator bit set to 1
				bytes[9] = (BYTE)0x0c; //0x0C	-	Optional Capability  Indicator BAS code
				bytes[10] =  (BYTE)0x07;//0x04	-	Optional Capability List Length  >>>>>>>>>>>>
	//			bytes[11] = (BYTE)0xb4; //DATAVIDCAPATTR | V_Cif
	//			bytes[12] = (BYTE)0xb6; // for ^  DATAVIDCAPATTR and  then V_1_29_97
                bytes[11] = (BYTE)0xf9 ;//0xF9	- 	Start_Mbe BAS code
				bytes[12] = (BYTE)0x02; //0x02	- 	Mbe length
				bytes[13] = (BYTE)0x0a;//0x0A	- 	H230 <H262/H263> BAS code
				bytes[14] = (BYTE)0x8c;//0x8C	-	H263_4CIF at 15fps
			
				bytes[15] = DATAVIDCAPATTR | V_Cif;
				bytes[16] = DATAVIDCAPATTR | V_1_29_97;
				bytes[17] = DATAVIDCAPATTR | V_1_29_97;
				//bytes[6] = (BYTE)0x08;//0x08	-	PPXC length
				//bytes[7] = (BYTE)0x8b ; //0x8B	-	VideoStream BAS code
				//bytes[8] = (BYTE)0x22;//0x22	-	Content label with terminator bit set to 1
				//bytes[9] = (BYTE)0x0c; //0x0C	-	Optional Capability  Indicator BAS code
				//bytes[10] =  (BYTE)0x04;//0x04	-	Optional Capability List Length
                //bytes[11] = (BYTE)0xf9 ;//0xF9	- 	Start_Mbe BAS code
				//bytes[12] = (BYTE)0x02; //0x02	- 	Mbe length
				//bytes[13] = (BYTE)0x0a;//0x0A	- 	H230 <H262/H263> BAS code
				//bytes[14] = (BYTE)0x8c;//0x8C	-	H263_4CIF at 15fps
		
				
				//bytes[0] = (BYTE)0x9a;
				//bytes[1] = (BYTE)0x2;
				//bytes[2] = (BYTE)0x9b; //0x9b

	AppendPPCap( bytes,18);


}
/////////////////////////////////////////////////////////////////////////////	
void CCapPP::RemovePPCap()
{
	m_pcProfile.RemoveCap();
	m_capAMSC.RemoveCap();
	m_CapPPXC.RemoveCap();

}
/////////////////////////////////////////////////////////////////////////////	
void CCapPP::AppendPPCap(const BYTE* capData,const BYTE msgLen)
{
	char	str[48];


	RemovePPCap();

//create new item
	 for( int iter=0; iter<msgLen ; iter++ ) {
                    switch( capData[iter] ) {
                         case PPXC_CAP : 										      
													 m_CapPPXC.CreateCapabilities(capData, &iter);
										        	break;
						case PP_H221_ESCAPE_TABLE_CAP : m_capPPh221EscapeTable = capData[iter]; break;
						case MEDIA_FLOW_CONTROL_CAP : m_capMediaFlowControlH320 = capData[iter]; break;
						case PC_PROFILE_CAP : 
												m_pcProfile.CreateCapabilities(capData, &iter);
												break;
						case AMSC_MUX_CAP : m_capAMSC.CreateCapabilities(capData, &iter); break;
	
						case AMSC_MUX64_CAP :  m_capAMSC.CreateCapabilities(capData, &iter); break;
	
					    default   : 
							sprintf(str,"%0x",capData[iter]);
							PTRACE2(eLevelError,"CCapPP::Create - illegal PP Capability byte - ",str);//EXCEPTION_TRACE
							break;
					}
				}//for
			

}
/////////////////////////////////////////////////////////////////////////////	
BYTE CCapPP::CheckCapEPC() const
{

	//all  multiple capabilities (base and extended) will be analyzed and 
	// whether at least one of the cap item is in the set, then positive answere will be return
	//1. 0x9a - pc-profile or 
	//2. 0x9c(0x9b)
	//3.0x94 - Esc table
	//4.0x99 - MediaFlwCntrl
	//5.0x90 - extended cap
		BYTE		 result = FALSE;
		
			//////////////////
			if(	m_capMediaFlowControlH320 == MEDIA_FLOW_CONTROL_CAP ){
				result = TRUE;
			}
			else if(m_capPPh221EscapeTable == PP_H221_ESCAPE_TABLE_CAP){
				result = TRUE;
			}
			else if(m_pcProfile.GetNumberProfiles()){
				result =TRUE;
			}
			else if(m_capAMSC.CheckAMSCMUX()){
				result = TRUE;
			}
			else if (m_CapPPXC.GetPPXCitemsNumber()){
						result = TRUE;

			}
		
			return result;


}
/////////////////////////////////////////////////////////////////////////////	
BYTE CCapPP::IsCapAMSCSupportRate(BYTE rate) const
{
	return m_capAMSC.IsCapAMSCSupportRate(rate);
}
/////////////////////////////////////////////////////////////////////////////	
BYTE CCapPP::isValidMinCapConfig()
{
	
	if(m_pcProfile.IsValidMinPcProfileConfig() &&
		m_capAMSC.IsValidMinAMSCConfig() )//&& isValidMinVideoCapConfig()    <===== will be used in the future versions
		return TRUE;

	return FALSE;
		
}
*/
/////////////////////////////////////////////////////////////////////////////	
void CCapPP::Dump(std::ostream& ostr) const
{

    ostr << "\n===================    CCapPP::Dump    ===================\n" ;
	
	ostr << "\n MediaFlowControlH320\t: <" << (dec) << (WORD)m_capMediaFlowControlH320 << ">";
	ostr << "\n PPh221EscapeTable   \t: <" << (dec) << (WORD)m_capPPh221EscapeTable    << ">";
	
	m_capPCProfile.Dump(ostr);
	m_capAMSC.Dump(ostr);
	m_capPPXC.Dump(ostr);

	ostr << "\n===============  CCapPP::Dump Finished!!!  ===============\n" ;
	ostr << "\n";
}
