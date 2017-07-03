//+========================================================================+
//                            H320MediaCaps.CPP                            |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H320MediaCaps.CPP                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |  01/10/07  |                                                      |
//+========================================================================+


#include <iomanip>

#include "CDRUtils.h"
#include "ConfPartyApiDefines.h"
#include "ConfPartyGlobals.h"
#include "H221.h"
#include "H263.h"
#include "H320MediaCaps.h"
#include "Macros.h"
#include "Segment.h"
#include "VideoDefines.h"

using namespace std;

//const BYTE AMC_CAP_LEN = 0x03;

///////////////////////////////////////////////////////////////////////////////
CDataCap::CDataCap()
{
    m_dataCap = 0L;
    m_hsdHmplCap = 0L;
    m_mlpCap     = 0L;
}

CDataCap::CDataCap(const CDataCap& other) : CPObject(other)
{
    m_dataCap    = other.m_dataCap;
    m_hsdHmplCap = other.m_hsdHmplCap;
	m_mlpCap     = other.m_mlpCap;
}

const char* CDataCap::NameOf() const
{
  return "CDataCap";
}

void CDataCap::Dump(std::ostream& msg) const
{
    msg << "\n==================    CDataCap::Dump    ==================\n"
		<< setw(20) << "m_dataCap"    << (hex) << m_dataCap    << "\n"
		<< setw(20) << "m_hsdHmplCap" << (hex) << m_hsdHmplCap << "\n"
		<< setw(20) << "m_mlpCap"     << (hex) << m_mlpCap     << "\n";

    msg << "\nDATA CAP\n";
    msg << "-------------------\n";
    for (WORD i = 0 ; i < 32 ; i++ ) {
        if ( OnDataCap(i) )  {
            CCDRUtils::DumpCap(DATAVIDCAPATTR | i,msg);
            msg << "\n";
        }
    }
    msg << "\nHSD/H-MLP CAP\n";
    msg << "-------------------\n";
    for (WORD i = 0 ; i < 32 ; i++ ) {
        if ( OnHsdHmlpCap(i) )  {
            CCDRUtils::DumpCap(HSDHMPLCAPATTR | i,msg,2);
            msg << "\n";
        }
    }

    msg << "\nMLP CAP\n";
    msg << "-------------------\n";
    for (WORD i = 0 ; i < 32 ; i++ ) {
        if ( OnMlpCap(i) )  {
            CCDRUtils::DumpCap(MLPCAPATTR | i,msg,2); 
            msg << "\n";
        }
    }
}

void CDataCap::CreateDefault()
{
    SetDataCap(Mbe_Cap);
    SetDataCap(Encryp_Cap);
    SetHsdHmlpBas(Hxfer_Cap_64k);
}

WORD CDataCap::OnHsdHmlpCap(WORD cap) const
{
    DWORD  temp = 0L;

    temp = 1L << cap;
    if ( m_hsdHmplCap & temp )
        return 1;

    return 0;
}

WORD CDataCap::OnDataCap(WORD cap) const
{
    DWORD temp = 0L;
    temp = 1L << cap;
    if ( m_dataCap & temp )
        return 1;

    return 0;
}

WORD CDataCap::OnMlpCap(WORD cap) const
{
    DWORD  temp = 0L;

    temp = 1L << cap;
    if ( m_mlpCap & temp )
        return 1;

    return 0;
}

void CDataCap::SetHsdHmlpBas(WORD cap)
{
    DWORD  temp = 0L;

    temp = 1L << cap;
    m_hsdHmplCap |= temp;
}


void CDataCap::SetMlpBas(WORD cap)
{
    DWORD  temp = 0L;

    temp = 1L << cap;
    m_mlpCap |= temp;
}


void CDataCap::ResetDataCaps()
{
    m_dataCap    = 0L;
    m_hsdHmplCap = 0L;
    m_mlpCap     = 0L;
}

void CDataCap::SetDataCap(WORD cap)
{
    DWORD  temp = 0L;

    temp = 1L << cap;
    m_dataCap |= temp;
}

void CDataCap::RemoveDataCap(WORD cap)
{
	DWORD  temp = 0L;
	temp = 1L << cap;
    temp = ~(temp);
    m_dataCap &= temp;
}

void CDataCap::Serialize(WORD format, CSegment& H221StringSeg)
{
    BYTE seriableVal = 0;
    switch(format) {
	case NATIVE:
	  {
 		H221StringSeg << m_dataCap
 					  << m_hsdHmplCap
 					  << m_mlpCap;
	  } break;
	case SERIALEMBD:
	  {
		WORD i, mlpOn = 0;
		// serialize data HSD  / Hmlp
		for ( i = 0 ; i < 32 ; i++ ) {
		  if ( OnHsdHmlpCap(i) )  {
			mlpOn = 1;
			seriableVal = (BYTE)Hsd_Esc | (BYTE)ESCAPECAPATTR ;
			H221StringSeg << seriableVal;
			seriableVal =  (BYTE)i | HSDHMPLCAPATTR; 
			H221StringSeg << seriableVal;
		  }
		}
		// serialize data LSD
		if ( OnDataCap(Ter2_Var_Lsd) )  {
		  H221StringSeg << seriableVal;
		  seriableVal = Ter2_Var_Lsd | DATAVIDCAPATTR;
		}
		if ( OnDataCap(Dxfer_Cap_300) )  {
		  seriableVal = Dxfer_Cap_300 | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_1200) )   {
		  seriableVal = Dxfer_Cap_1200 | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_4800) )   {
		  seriableVal = Dxfer_Cap_4800 | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_6400) )   {
		  seriableVal = Dxfer_Cap_6400 | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if (OnDataCap(Dxfer_Cap_4800) || OnDataCap(Dxfer_Cap_6400)) {
		  //declare h.224 caps
		  seriableVal = Data_Apps_Esc |  ESCAPECAPATTR ;
		  H221StringSeg << seriableVal;
		  seriableVal =  H224_LSD_On | Attr101;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_8000) )  {
		  seriableVal = Dxfer_Cap_8000 | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_9600) )   {
		  seriableVal = Dxfer_Cap_9600 | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_14400) )   {
		  seriableVal = Dxfer_Cap_14400 | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_16k) )   {
		  seriableVal = Dxfer_Cap_16k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_24k) )   {
		  seriableVal = Dxfer_Cap_24k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_32k) )  {
		  seriableVal = Dxfer_Cap_32k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_40k) )  {
		  seriableVal = Dxfer_Cap_40k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_48k) )  {
		  seriableVal = Dxfer_Cap_48k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_56k) )  {
		  seriableVal = Dxfer_Cap_56k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_62_4k) )  {
		  seriableVal = Dxfer_Cap_62_4k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_64k) )  {
		  seriableVal = Dxfer_Cap_64k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_Mlp_4k) )  {
		  mlpOn = 1;
		  seriableVal = Dxfer_Cap_Mlp_4k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Dxfer_Cap_Mlp_6_4k) )  {
		  mlpOn = 1;
		  seriableVal = Dxfer_Cap_Mlp_6_4k | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
		if ( OnDataCap(Var_Mlp) )  {
		  mlpOn = 1;
		  seriableVal = Var_Mlp | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}

		// serialize data MLP  
		for ( i = 0 ; i < 32 ; i++ ) {
		  if ( OnMlpCap(i) )  {
			mlpOn = 1;
			seriableVal = (BYTE)Hsd_Esc | (BYTE)ESCAPECAPATTR ;
			H221StringSeg << seriableVal;
			seriableVal =  (BYTE)i | MLPCAPATTR; 
			H221StringSeg << seriableVal;
		  }
		}
		if ( mlpOn )//to ask Ron
		  H221StringSeg << (BYTE)(Data_Apps_Esc | ESCAPECAPATTR) << (BYTE)((28 /*t120 cap */) | (5<<5) ); // A3 table / h221    
	  } break;
	}
}

void CDataCap::DeSerialize(WORD format, CSegment& H221StringSeg)
{
    switch(format) {
	case NATIVE:
	  {
		H221StringSeg >> m_dataCap
					  >> m_hsdHmplCap
					  >> m_mlpCap;
	  } break;
	case SERIALEMBD:
	  PTRACE(eLevelInfoNormal, "CDataCap::DeSerialize - case SERIALEMBD - do nothing");
	  break;
	}
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////// CCapH239ExtendedVideo ////////////////////////////
/////////////////////////////////////////////////////////////////////////////	
void CCapH239ExtendedVideo::Serialize(WORD format, CSegment &seg)
{	
	switch( format ) {
	case SERIALEMBD :{
		
		if(!m_labelListSize && !m_pLabelList)
			return;
		
		seg << ((BYTE)(Start_Mbe | ESCAPECAPATTR));
	
		BYTE msgLength = 0;
		BYTE roleLabelValue = SerializeRoleLabelListToByte();
		
		CSegment *segTemp = new CSegment;
		*segTemp << (BYTE)h239ExtendedVideoCapability;
		SerializeGenericParameter(RoleLabelParamIdent, roleLabelValue ,segTemp);
		*segTemp << (BYTE)(0x0);
		
		m_h261cap.Serialize(format, *segTemp);
		m_h263cap.Serialize(format, *segTemp);
		
		msgLength = (BYTE)segTemp->GetWrtOffset();
		seg<<msgLength;
		seg<< *segTemp;
		
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
	default: {
		break;
			 }
	}

}
/////////////////////////////////////////////////////////////////////////////	
void CCapH239ExtendedVideo::DeSerialize(WORD format, CSegment &seg, BYTE dataLen)
{	
	switch( format ) {
	case SERIALEMBD : {
		BYTE readBytesNum =0;
		BYTE PID, roleLabelByte;
		seg >> PID >> roleLabelByte;
		DeserializeRoleLabelByteToList(roleLabelByte);
		BYTE tempByte = 0;
		seg >> tempByte;
		if(tempByte)
		{
			PTRACE(eLevelError, "Illegal value - Byte should equal zero.");
			PASSERT(tempByte);
		}
		readBytesNum+=3;
				
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
	default :	{
		break;
				}
	}

}	
/////////////////////////////////////////////////////////////////////////////		
void CCapH239ExtendedVideo::DeserializeRoleLabelByteToList(BYTE roleLabelByte)
{
	PDELETEA(m_pLabelList);
	m_labelListSize = 0;
	if((roleLabelByte&PresentationRoleLabel) && (roleLabelByte&LiveRoleLabel) )
	{
		m_labelListSize = 2;
		m_pLabelList = new BYTE[m_labelListSize];
		m_pLabelList[0] = (roleLabelByte&PresentationRoleLabel);
		m_pLabelList[0] = (roleLabelByte&LiveRoleLabel);
	}
	else if(roleLabelByte&PresentationRoleLabel)
	{
		m_labelListSize = 1;
		m_pLabelList = new BYTE[m_labelListSize];
		m_pLabelList[0] = roleLabelByte;
	}
	else if(roleLabelByte&LiveRoleLabel)
	{
		m_labelListSize = 1;
		m_pLabelList = new BYTE[m_labelListSize];
		m_pLabelList[0] = roleLabelByte;
	}

}
/////////////////////////////////////////////////////////////////////////////		
BYTE CCapH239ExtendedVideo::SerializeRoleLabelListToByte()
{
	BYTE roleLabelByte = 0;
	if(m_labelListSize && m_pLabelList)
	{
		for(int i=0; i<m_labelListSize;i++)
			roleLabelByte|= m_pLabelList[i];
	}
	return roleLabelByte;
}
/////////////////////////////////////////////////////////////////////////////
void CCapH239ExtendedVideo::CreateExtendedVideoCapH239(void)
{
	// Set Content label
	PDELETEA(m_pLabelList);

    m_labelListSize = 1;
	m_pLabelList = new BYTE[m_labelListSize];
	m_pLabelList[0] = (BYTE)(PresentationRoleLabel);

	CreateVideoCaps();
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////// CCapAMC //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CCapAMC::CCapAMC(): m_optionByte1(0), m_optionByte2(0)
{}


CCapAMC::CCapAMC(const CCapAMC& other) : CPObject(other)
{
	m_optionByte1 = other.m_optionByte1;
	m_optionByte2 = other.m_optionByte2;
}

CCapAMC::~CCapAMC()
{}

void CCapAMC::Serialize(WORD format,CSegment &seg)
{
	switch( format ) {
	case SERIALEMBD:{
		if(!m_optionByte1 && !m_optionByte2)
			return;
	    seg << (BYTE)(Start_Mbe | ESCAPECAPATTR); // Insert "Start- MBE" opcode.
		seg << (BYTE)AMC_CAP_LEN;
		seg << (BYTE)AMC_cap;
		seg << (BYTE)m_optionByte1;
		seg << (BYTE)m_optionByte2;
		break;
	}
	case NATIVE: {
		seg << (BYTE)m_optionByte1 << (BYTE)m_optionByte2;
		break;
	}		
	default:
	  break;
	}
}

void CCapAMC::DeSerialize(WORD format,CSegment &seg)
{
	seg >> m_optionByte1  >> m_optionByte2;
}

CCapAMC& CCapAMC::operator=(const CCapAMC& other)
{
	if(this == &other)
	  return *this;  

	m_optionByte1 = other.m_optionByte1;
	m_optionByte2 = other.m_optionByte2;

	return *this;
}

void CCapAMC::Dump(std::ostream& ostr) const
{
	ostr << "\nCCapAMC::Dump\n"
		 << "-------------"
		 << "\n  OPTION BYTE 1    \t: <"<< (hex) << (WORD)m_optionByte1  << ">"
		 << "\n  OPTION BYTE 2    \t: <"<< (hex) << (WORD)m_optionByte2  << ">" << "\n";
}

void CCapAMC::SetRateAMC(BYTE rate)
{
	if (rate > AMC_1536k) {
		PASSERT_AND_RETURN(rate);
	}

	const BYTE MSB_ON_mask = 0x80;

	switch(rate){
	case AMC_0k: {
		break;
	}
	case AMC_40k: {
		m_optionByte1 |= (MSB_ON_mask >> AMC_40k);
		break;
	}
	case AMC_64k: {
		m_optionByte1 |= (MSB_ON_mask >> AMC_64k);
		break;
	}
	case AMC_96k: {		
		m_optionByte1 |= (MSB_ON_mask >> AMC_96k);
		break;
	}
	case AMC_128k: {		
		m_optionByte1 |= (MSB_ON_mask >> AMC_128k);
		break;
	}
	case AMC_192k: {		
		m_optionByte1 |= (MSB_ON_mask >> AMC_192k);
		break;
	}
	case AMC_256k: {		
		m_optionByte1 |= (MSB_ON_mask >> AMC_256k);
		break;
	}
	case AMC_384k: {		
		m_optionByte1 |= (MSB_ON_mask >> AMC_384k);
		break;
	}
	case AMC_512k: {		
		m_optionByte2 |= (MSB_ON_mask >> AMC_40k);
		break;
	}
	case AMC_768k: {		
		m_optionByte2 |= (MSB_ON_mask >> AMC_64k);
		break;
	}
	default: 
		break;
	}
}

void CCapAMC::SetAllAMCRatesWeSupportUpTo(BYTE rate)
{
	if (rate > AMC_HSDCap) {
		PASSERT_AND_RETURN(rate);
	}

	switch (rate) {
        case AMC_HSDCap: {
		    SetRateAMC(AMC_HSDCap);
			
		}
		case AMC_768k: {
			SetRateAMC(AMC_768k);
			
		}
		case AMC_512k: {
			SetRateAMC(AMC_512k);
			
		}
		case AMC_384k: {
			SetRateAMC(AMC_384k);
			
		}
		case AMC_256k: {
			SetRateAMC(AMC_256k);
			
		}
		case AMC_192k: {
			SetRateAMC(AMC_192k);
			
		}
        case AMC_128k: {
			SetRateAMC(AMC_128k);
			
		}
	//	case AMC_96k: {
	//		SetRateAMC(AMC_96k);
	//  }
		case AMC_64k: {
			SetRateAMC(AMC_64k);
			break;
		}
	//	case AMC_40k: {
	//		SetRateAMC(AMC_40k);
	//		break;
	//  }
        default: { 
			if (rate)	
				PASSERT(rate);
			else
				PASSERT(101);
			break; 
		}
	}
}

BYTE CCapAMC::GetMaxContentRate(void) const
{
	for(BYTE rate = AMC_768k; rate>= AMC_40k; rate--)
		if(IsAMCRateSupported(rate))
			return rate;
		
	return AMC_0k;
}

BYTE CCapAMC::IsAMCRateSupported (BYTE rate) const
{
	const BYTE MSB_ON_mask = 0x80;

	switch(rate){
	case AMC_0k:
	  return TRUE;
	case AMC_40k:
	  if(m_optionByte1 & (MSB_ON_mask >> AMC_40k))
		return TRUE;
	  break;
	case AMC_64k:
	  if(m_optionByte1 & (MSB_ON_mask >> AMC_64k))
		return TRUE;
	  break;
	case AMC_96k:
	  if(m_optionByte1 & (MSB_ON_mask >> AMC_96k))
		return TRUE;
	  break;
	case AMC_128k:
	  if(m_optionByte1 & (MSB_ON_mask >> AMC_128k))
		return TRUE;
	  break;
	case AMC_192k:
	  if(m_optionByte1 & (MSB_ON_mask >> AMC_192k))
		return TRUE;
	  break;
	case AMC_256k:
	  if(m_optionByte1 & (MSB_ON_mask >> AMC_256k))
		return TRUE;
	  break;
	case AMC_384k:
	  if(m_optionByte1 & (MSB_ON_mask >> AMC_384k))
		return TRUE;
	  break;
	case AMC_512k:
	  if(m_optionByte2 & (MSB_ON_mask >> AMC_40k))
		return TRUE;
	  break;
	case AMC_768k:
	  if(m_optionByte2 & (MSB_ON_mask >> AMC_64k))
		return TRUE;
	  break;
	default:
	  break;
	}
	return FALSE;
}

BYTE CCapAMC::GetGreatestAMCRateLessThen(BYTE AMCRate) const
{
	for (BYTE i=(AMCRate-1); i>=AMC_40k; i--) {
		if (IsAMCRateSupported(i)) 
			return i;
	}
	return AMC_0k;
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////// CCapH239 /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CCapH239::CCapH239()
{}
/////////////////////////////////////////////////////////////////////////////
CCapH239::CCapH239(const CCapH239& other) : CPObject(other)
{
	m_capAMC = other.m_capAMC;
 	m_capH239ExtendedVideo = other.m_capH239ExtendedVideo;
}
/////////////////////////////////////////////////////////////////////////////
CCapH239::~CCapH239()
{
}
/////////////////////////////////////////////////////////////////////////////
void CCapH239::Serialize(WORD format,CSegment &seg)
{
	switch( format ) {
	case SERIALEMBD :{
 		PTRACE(eLevelError,"CCapH239::Serialize: Does not suit SERIALEMBD Format - Because Extended Video Cap should be sent outside the cap mark!!!");
		PASSERT(101);
		break;
					 }
	case NATIVE: {
		m_capAMC.Serialize(format,seg);
		m_capH239ExtendedVideo.Serialize(format,seg);
		break;
	}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapH239::DeSerialize(WORD format,CSegment &seg)
{
	switch( format ) {
	case SERIALEMBD: {
		PTRACE(eLevelError,"CCapH239::DeSerialize: Does not suit SERIALEMBD Format");
		PASSERT_AND_RETURN(101);
	}
	case NATIVE: {
		m_capAMC.DeSerialize(format,seg);
 		m_capH239ExtendedVideo.DeSerialize(format,seg);
		break;
	}
	default:
	  break;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapH239::SetOnlyExtendedVideoCaps(const CCapH239& other)
{
	m_capH239ExtendedVideo = other.m_capH239ExtendedVideo;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH239::GetMaxContentRate(void)
{
    return m_capAMC.GetMaxContentRate();
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH239::IsAMCRateSupported(BYTE rate) const
{
    return m_capAMC.IsAMCRateSupported(rate);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH239::GetGreatestAMCRateLessThen(BYTE AMCRate) const
{
    return m_capAMC.GetGreatestAMCRateLessThen(AMCRate);
}
/////////////////////////////////////////////////////////////////////////////
void CCapH239::SerializeH239ExtendedVidCaps(WORD format,CSegment &seg)
{
    m_capH239ExtendedVideo.Serialize(format, seg);
}
/////////////////////////////////////////////////////////////////////////////
void CCapH239::SerializeAmcCaps(WORD format,CSegment &seg)
{
    m_capAMC.Serialize(format, seg);
}
/////////////////////////////////////////////////////////////////////////////
CCapH239& CCapH239::operator=(const CCapH239& other)
{
	if(this == &other)
	  return *this;  
	m_capAMC = other.m_capAMC;
 	m_capH239ExtendedVideo = other.m_capH239ExtendedVideo;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CCapH239::Dump(std::ostream& ostr) const
{	
	ostr << "\n===================    CCapH239::Dump    ===================\n" ;
	
	m_capAMC.Dump(ostr);
 	m_capH239ExtendedVideo.Dump(ostr);

	ostr << "\n===============  CCapH239::Dump Finished!!!  ===============\n" ;
	ostr << "\n";	
}
// ///////////////////////////////////////////////////////////////////////////
void CCapH239::SetAMCCaps(CSegment &seg,BYTE len)
{
	if(len!=AMC_CAP_LEN)
	{
		if(len)
		{
			PASSERT_AND_RETURN(len);
		}
		else
		{
			PASSERT_AND_RETURN(101);
		}
 	}	
	m_capAMC.DeSerialize(SERIALEMBD, seg);		
}
// ///////////////////////////////////////////////////////////////////////////
void CCapH239::SetH239ExtendedVideoCaps(CSegment &seg,BYTE len)
{	
    m_capH239ExtendedVideo.DeSerialize(SERIALEMBD, seg, len);		
}

// ///////////////////////////////////////////////////////////////////////////
void CCapH239::CreateCapH239(WORD callRate, WORD isTranscoding,WORD isLSD,WORD ContentLevel)
{
	
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

    PTRACE(eLevelInfoNormal,"CCapH239::CreateCapH239");

	WORD ContentRate = AMC_0k;
	
	switch (callRate) {
		case Xfer_128:
			{ 
				if(!isLSD)
				ContentRate = AMC_64k;
				break;
			}
		case Xfer_192: 
			{ 
				if((ContentLevel == eLiveVideo) && (!isLSD))
					ContentRate = AMC_128k;
				else 
					ContentRate = AMC_64k;
			
				break;
			}	
		case Xfer_256:
			{ 
				if(ContentLevel == eGraphics)
					ContentRate = AMC_64k;
				else
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMC_128k;
					else
					{
						if(isLSD)
							ContentRate = AMC_128k;
						else
							ContentRate = AMC_192k;
					}
				}
				break;
			}
		case Xfer_320:
			{ 
				if(ContentLevel == eGraphics)
					ContentRate = AMC_128k;
				else
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMC_192k;
					else
					{
						if(isLSD)
							ContentRate = AMC_192k;
						else
							ContentRate = AMC_256k;
					}
				}
				break;
			}
		case Xfer_384:
			{ 
				if(ContentLevel == eGraphics)
					ContentRate = AMC_128k;
				else
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMC_192k;
					else
						ContentRate = AMC_256k;
				}
				break;
			}
		case Xfer_512: 
			{ 
				if(ContentLevel == eGraphics)
					ContentRate = AMC_128k;
				else 
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMC_256k;
					else
						ContentRate = AMC_384k;
				}
				break;
			}
		case Xfer_768: 
			{ 
				if(ContentLevel == eGraphics)
					ContentRate = AMC_256k;
				else 
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMC_384k;
					else
						ContentRate = AMC_512k;
				}
				break;
			}
		case Xfer_1152: 
			{
				if(ContentLevel == eGraphics)
					ContentRate = AMC_256k;
				else 
				{
					if(ContentLevel ==eHiResGraphics)
						ContentRate = AMC_384k;
					else
						ContentRate = AMC_512k;
				}
				break;
			}
		case Xfer_1472: 
		case Xfer_1536:
		case Xfer_1920:
			{ 
				if(ContentLevel == eGraphics)
					ContentRate = AMC_256k;
				else 
				{
					if(ContentLevel ==eHiResGraphics)
						ContentRate = AMC_512k;
					else
						ContentRate = AMC_768k;
				}
				break;
			}

		default: {
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
		m_capAMC.SetAllAMCRatesWeSupportUpTo(ContentRate);
	}
	else
	{
		m_capAMC.SetRateAMC(ContentRate);
		m_capAMC.SetRateAMC(AMC_64k);  //AMC-64k Mandatory by standard
	}

	m_capH239ExtendedVideo.CreateExtendedVideoCapH239();
}

/////////////////////////////////////////////////////////////////////////////
//////////////////CContentCap Class//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CContentCap::CContentCap()
{
    m_pCapH239 = NIL(CCapH239);
}
/////////////////////////////////////////////////////////////////////////////
CContentCap::CContentCap(const CContentCap& other) : CPObject(other)
{
    if (other.m_pCapH239) {
      m_pCapH239 = new CCapH239;
      *m_pCapH239 = *(other.m_pCapH239);
    }
    else
      m_pCapH239 = NIL(CCapH239);
}
/////////////////////////////////////////////////////////////////////////////
CContentCap::~CContentCap()
{
    POBJDELETE(m_pCapH239);
}
/////////////////////////////////////////////////////////////////////////////
CContentCap& CContentCap::operator= (const CContentCap& other)
{
	if (m_pCapH239)
		POBJDELETE(m_pCapH239);

	if (other.m_pCapH239) {
		m_pCapH239 = new CCapH239;
		*m_pCapH239 = *(other.m_pCapH239);
	}
	else
		m_pCapH239 = NIL(CCapH239);

	return *this;
}
/////////////////////////////////////////////////////////////////////////////
void CContentCap::Dump(std::ostream& msg) const
{
    msg << "\n==================    CContentCap::Dump    ==================\n";
    if (m_pCapH239)
 		m_pCapH239->Dump(msg);
	else
	    msg << "\n empty \n";
}
/////////////////////////////////////////////////////////////////////////////
void CContentCap::Serialize(WORD format, CSegment& H221StringSeg)
{
	switch( format ) {
	
	case NATIVE:
	  {
		if (m_pCapH239) {
		    H221StringSeg << (BYTE)1; // is H239 Caps
		    m_pCapH239->Serialize(format, H221StringSeg);
		}
		else
		    H221StringSeg << (BYTE)0; // is H239 Caps	
	  } break;
	case SERIALEMBD:
	  {
		if(m_pCapH239)
		{
		    m_pCapH239->SerializeAmcCaps(format,H221StringSeg);
		}
	  } break;
	}
}
/////////////////////////////////////////////////////////////////////////////

void CContentCap::DeSerialize(WORD format, CSegment& H221StringSeg)
{
	switch( format ) {
	
	case NATIVE:
	  {
		BYTE isH239Cap;
		H221StringSeg >> isH239Cap;
		if (isH239Cap) {
		    if (!m_pCapH239)
			    m_pCapH239 = new CCapH239;

			m_pCapH239->DeSerialize(format, H221StringSeg);
		}
	  } break;
	case SERIALEMBD:
	  PTRACE(eLevelInfoNormal, "CContentCap::DeSerialize - case SERIALEMBD - do nothing");
	  break;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CContentCap::SetH239ExtendedVideoCaps(CSegment &seg,BYTE len)
{
	if(!m_pCapH239)
		m_pCapH239 = new CCapH239;
		
	m_pCapH239->SetH239ExtendedVideoCaps(seg,len);
}
/////////////////////////////////////////////////////////////////////////////
void CContentCap::SetAMCCaps(CSegment &seg,BYTE len)
{
	if(!m_pCapH239)
		m_pCapH239 = new CCapH239;
	
	m_pCapH239->SetAMCCaps(seg,len);
}
/////////////////////////////////////////////////////////////////////////////
void CContentCap::RemoveH239Caps()
{
	if (m_pCapH239)
	{
		POBJDELETE(m_pCapH239)
		m_pCapH239 = NIL(CCapH239);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CContentCap::SetOnlyExtendedVideoCaps(const CCapH239* pCapH239)
{
	if (!m_pCapH239)
		m_pCapH239 = new CCapH239(*pCapH239);
	else
		m_pCapH239->SetOnlyExtendedVideoCaps(*pCapH239);
}


