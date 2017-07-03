//+========================================================================+
//                            H264CAP.CPP                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       UCAPH264.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/10/07   |                                                      |
//+========================================================================+

#include <sstream>
#include <iomanip>

#include "CDRUtils.h"
#include "ConfPartyGlobals.h"
#include "H221.h"
//#include "H264.h"
#include "H264Cap.h"
#include "H264Util.h"
#include "H320ComMode.h"
#include "Macros.h"
#include "ObjString.h"
#include "Segment.h"
#include "IpCommon.h"



using namespace std;



CCapSetH264::CCapSetH264()
{
     CreateDefault();
}
/////////////////////////////////////////////////////////////////////////////
WORD CCapSetH264::operator==(const CCapSetH264& CapSetH264) const
{

   if(  (m_profileValue == CapSetH264.m_profileValue)  &&
	    (m_levelValue == CapSetH264.m_levelValue)  )
		/*&&
		(m_capH264CustomMaxMBPS == CapSetH264.m_capH264CustomMaxMBPS) &&
		(m_capH264CustomMaxFS == CapSetH264.m_capH264CustomMaxFS) &&
		(m_capH264CustomMaxDPB == CapSetH264.m_capH264CustomMaxDPB) &&
		(m_capH264CustomMaxBRandCPB == CapSetH264.m_capH264CustomMaxBRandCPB))*/
   	   return 1;
   else
       return 0;
}

/////////////////////////////////////////////////////////////////////////////
WORD CCapSetH264::operator<=(const CCapSetH264& CapSetH264) const
{

	BYTE customMaxMBPSSmallerOrSame     = IsFirstCustomSmallerOrSameAsSeconed(m_capH264CustomMaxMBPS,
																			  CapSetH264.m_capH264CustomMaxMBPS);
	BYTE customMaxFSSmallerOrSame       = IsFirstCustomSmallerOrSameAsSeconed(m_capH264CustomMaxFS,
																			  CapSetH264.m_capH264CustomMaxFS);
	BYTE customMaxDPBSmallerOrSame      = IsFirstCustomSmallerOrSameAsSeconed(m_capH264CustomMaxDPB,
																			  CapSetH264.m_capH264CustomMaxDPB);
	BYTE customMaxBRandCPBSmallerOrSame = IsFirstCustomSmallerOrSameAsSeconed(m_capH264CustomMaxBRandCPB,
																			  CapSetH264.m_capH264CustomMaxBRandCPB);


	if (m_profileValue != CapSetH264.m_profileValue)
	{
		DBGPASSERT(1);
		return FALSE;
	}

	if(m_levelValue == CapSetH264.m_levelValue)
	{
		if (customMaxMBPSSmallerOrSame && customMaxFSSmallerOrSame && customMaxDPBSmallerOrSame
			&& customMaxBRandCPBSmallerOrSame)
			return TRUE;
		else return FALSE;
	}

	else if (m_levelValue < CapSetH264.m_levelValue)
	{
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CCapSetH264::operator<(const CCapSetH264& CapSetH264) const
{
	// Romem - 25.5.11: High profile compatability with HDX ISDN
	BOOL bCfgEnableHighfProfileInIsdn = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE_WITH_ISDN);
	BOOL bEnableHighfProfileInIsdn = FALSE;
	if( IsFeatureSupportedBySystem(eFeatureH264HighProfile) && bCfgEnableHighfProfileInIsdn )
	{
		bEnableHighfProfileInIsdn = 1;
	}
	if (m_profileValue != CapSetH264.m_profileValue && !bEnableHighfProfileInIsdn)
	{
		DBGPASSERT(1);
		return 0;
	}

	if(m_levelValue < CapSetH264.m_levelValue)
		return 1;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH264::IsFirstCustomSmallerOrSameAsSeconed(WORD firstCustom, WORD secondCustom)const
{
	BYTE retval = 0;

	if(firstCustom == secondCustom)
		retval = 1;

	else
	{
		if(firstCustom != 0xFFFF)
		{
			if (secondCustom != 0xFFFF)
			{
				if (firstCustom < secondCustom)
					retval = 1;
			}
			else retval = 0;
		}
		else //first is 0xFFFF
		{
			if (secondCustom!=0xFFFF)
				retval = 1;

		}
	}
	return retval;
}

/////////////////////////////////////////////////////////////////////////////
CCapSetH264::~CCapSetH264()
{
}
/////////////////////////////////////////////////////////////////////////////
const char*   CCapSetH264::NameOf() const
{
	return "CCapSetH264";
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH264::Dump(void) const
{
	std::ostringstream  msg;

	msg << "\nCCapSetH264::Dump\n"
      << "-----------\n"
      << setw(30) << "this"             << (hex) << (DWORD)this  << (dec) << "\n"
      << setw(30) << "m_profileValue "   << (WORD)m_profileValue  << "\n"
      << setw(30) << "m_levelValue "     << (WORD)m_levelValue    << "\n"
      << setw(30) << "m_capH264CustomMaxMBPS "	  << m_capH264CustomMaxMBPS     << "\n"
      << setw(30) << "m_capH264CustomMaxFS "		  << m_capH264CustomMaxFS       << "\n"
      << setw(30) << "m_capH264CustomMaxDPB " 	  << m_capH264CustomMaxDPB      << "\n"
      << setw(30) << "m_capH264CustomMaxBRandCPB " << m_capH264CustomMaxBRandCPB << "\n"
	  << setw(30) << "m_capH264CustomMaxSAR " 	  << (dec) << m_capH264CustomMaxSAR << "\n";
	msg << "\n";

	PTRACE(eLevelInfoNormal, (char*)msg.str().c_str());
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH264::Dump(std::ostream& ostr) const
{
   ostr << setw(20) << (WORD)m_profileValue;
   ostr << setw(20) << (WORD)m_levelValue ;
   ostr << setw(20) << m_capH264CustomMaxMBPS;
   ostr << setw(20) << m_capH264CustomMaxFS;
   ostr << setw(20) << m_capH264CustomMaxDPB;
   ostr << setw(20) << m_capH264CustomMaxBRandCPB;
   ostr << setw(20) << (dec) << m_capH264CustomMaxSAR;

   ostr << "\n";

}
/////////////////////////////////////////////////////////////////////////////
void  CCapSetH264::CreateDefault()
{
	m_profileValue = H264_Profile_BaseLine;
	m_levelValue = H264_Level_3;
	m_capH264CustomMaxMBPS = 0xFFFF;
	m_capH264CustomMaxFS = 0xFFFF;
	m_capH264CustomMaxDPB = 0xFFFF;
	m_capH264CustomMaxBRandCPB = 0xFFFF;
	m_capH264CustomMaxSAR = H264_ALL_LEVEL_DEFAULT_SAR;
}

////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH264::GetCapH264ProfileValue()const
{
  return m_profileValue;
}

////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH264::GetCapH264LevelValue()const
{
  return m_levelValue;
}

////////////////////////////////////////////////////////////////////////////
void CCapSetH264::SetCapH264ProfileValue(BYTE profile)
{
  m_profileValue = profile;
}

////////////////////////////////////////////////////////////////////////////
void CCapSetH264::SetCapH264LevelValue(BYTE value)
{
  m_levelValue = value;
}

/////////////////////////////////////////////////////////////////////////////
void CCapSetH264::SetCapH264CustomMaxMBPS(WORD mbps)
{
	m_capH264CustomMaxMBPS = mbps;
}

/////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapSetH264::GetCapH264CustomMaxMBPS() const
{
	return m_capH264CustomMaxMBPS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// BYTE CCapSetH264::CalcCustomParamFirstByte(WORD customParam)
// {
// 	//63 is equal to Bin number 111111
// 	//when we would like to send byte that is higher than 111111 we need 1 more byte
// 	//so we sign it with 1 in the highest BYTEs bit it means that the first Byte will be
// 	//10xxxxxx when x is 0/1
// BYTE x=(BYTE)customParam;
// 	if (customParam <= 63)
// 		 return x ;
// 	else //custom param > 63
// 		x = (x & (BYTE)63) | (BYTE)128;
// 	return x;
// }

// //////////////////////////////////////////////////////////////////////////////////////////////////////
// BYTE CCapSetH264::CalcCustomParamSecondByte(WORD customParam)
// {
// 	BYTE y=0;
// 	WORD num1 = 8128;
// 	//8126 is equal to Bin number 0001111111000000
// 	//when we would like to send byte that is higher than 111111 we need 1 more byte
// 	//so we sign it with 1 in the highest BYTEs bit it means that the first Byte will be
// 	//10xxxxxx when x is 0/1
// WORD x=  customParam & num1; //only 7 requierd bits remine

// x>>=6; //shift 6 right
// y=(BYTE)x;
// 	return y;
// }

// //////////////////////////////////////////////////////////////////////////////////////////////////////
// WORD CCapSetH264::CalcCustomParam(BYTE firstByte,BYTE secondByte)
// {

// 	WORD num1 = 63;
// 	//if (firstByte<=63) PASSERT(firstByte);
// 	WORD customParam = firstByte;
// 	if (customParam > 63)
// 		customParam = customParam & num1;

// 	WORD secondByteCastToWord = secondByte;
// 	secondByteCastToWord <<= 6;
// 	customParam |= secondByteCastToWord;

// 	return customParam;
// }
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////
// BYTE CCapSetH264::IsTwoBytesNumber(WORD m_capH264CustomMaxMBPS)
// {
// 	if(CalcH264CustomParamFirstByte(m_capH264CustomMaxMBPS)>63)
// 		return TRUE;
// 	else return FALSE;
// }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapSetH264::SetCapH264CustomMaxFS(WORD fs)
{
	m_capH264CustomMaxFS = fs;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapSetH264::GetCapH264CustomMaxFS() const
{
	return m_capH264CustomMaxFS;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapSetH264::SetCapH264CustomMaxDPB(WORD dpb)
{
	m_capH264CustomMaxDPB = dpb;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapSetH264::GetCapH264CustomMaxDPB() const
{
	return m_capH264CustomMaxDPB;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapSetH264::SetCapH264CustomMaxBRandCPB(WORD brcpb)
{
	m_capH264CustomMaxBRandCPB = brcpb;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapSetH264::GetCapH264CustomMaxBRandCPB() const
{
	return m_capH264CustomMaxBRandCPB;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapSetH264::SetCapH264CustomMaxSAR(WORD sar)
{
	m_capH264CustomMaxSAR = sar;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapSetH264::GetCapH264CustomMaxSAR() const
{
	return m_capH264CustomMaxSAR;
}
/////////////////////////////////////////////////////////////////////////////
void  CCapSetH264::Serialize(WORD format,CSegment& seg)
{
	switch ( format )
	{

		case SERIALEMBD :
		case NATIVE:
		{
			seg << m_profileValue;
			seg << m_levelValue;

			if (GetCapH264CustomMaxMBPS() != 0xFFFF)
			{
				seg << (BYTE)CUSTOM_MAX_MBPS_CODE;
				seg << CCDRUtils::CalcH264CustomParamFirstByte(m_capH264CustomMaxMBPS);
				if (CCDRUtils::IsH264TwoBytesNumber(m_capH264CustomMaxMBPS))
					seg << CCDRUtils::CalcH264CustomParamSecondByte(m_capH264CustomMaxMBPS);
			}

			if (GetCapH264CustomMaxFS() != 0xFFFF)
			{
				seg << (BYTE)CUSTOM_MAX_FS_CODE;
				seg << CCDRUtils::CalcH264CustomParamFirstByte(m_capH264CustomMaxFS);
				if (CCDRUtils::IsH264TwoBytesNumber(m_capH264CustomMaxFS))
					seg << CCDRUtils::CalcH264CustomParamSecondByte(m_capH264CustomMaxFS);
			}

			if (GetCapH264CustomMaxDPB() != 0xFFFF)
			{
				seg << (BYTE)CUSTOM_MAX_DPB_CODE;
				seg << CCDRUtils::CalcH264CustomParamFirstByte(m_capH264CustomMaxDPB);
				if (CCDRUtils::IsH264TwoBytesNumber(m_capH264CustomMaxDPB))
					seg << CCDRUtils::CalcH264CustomParamSecondByte(m_capH264CustomMaxDPB);
			}

			if (GetCapH264CustomMaxBRandCPB() != 0xFFFF)
			{
				seg << (BYTE)CUSTOM_MAX_BR_CODE;
				seg << CCDRUtils::CalcH264CustomParamFirstByte(m_capH264CustomMaxBRandCPB);
				if (CCDRUtils::IsH264TwoBytesNumber(m_capH264CustomMaxBRandCPB))
					seg << CCDRUtils::CalcH264CustomParamSecondByte(m_capH264CustomMaxBRandCPB);
			}

			seg << (BYTE)CUSTOM_SAR_CODE;
			seg << CCDRUtils::CalcH264CustomParamFirstByte(m_capH264CustomMaxSAR);
			if (CCDRUtils::IsH264TwoBytesNumber(m_capH264CustomMaxSAR))
					seg << CCDRUtils::CalcH264CustomParamSecondByte(m_capH264CustomMaxSAR);
			break;
		}

		default :{
			if(format)
				DBGPASSERT(format);
			else
				DBGPASSERT(101);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CCapSetH264::DeSerialize(WORD format,CSegment& seg)
{
	switch ( format )  {

	case SERIALEMBD :
	case NATIVE:

		{
			seg >> m_profileValue;
			seg >> m_levelValue;

			if(seg.EndOfSegment())  return;

			else
			{
				BYTE customCode;
				BYTE firstByteCustomValue;
				BYTE secondByteCustomValue;
				WORD customValue;



				while (!seg.EndOfSegment())
				{
					seg >> customCode;
					seg >> firstByteCustomValue;
					if (firstByteCustomValue & 128)
					{
						seg>>secondByteCustomValue; // custom_1
						customValue = CCDRUtils::CalcH264WordFromBytes(firstByteCustomValue,secondByteCustomValue);
					}
					else customValue = firstByteCustomValue;

					switch(customCode)
					{
					case CUSTOM_MAX_MBPS_CODE:
						{
							SetCapH264CustomMaxMBPS(customValue);
							break;
						}
					case CUSTOM_MAX_FS_CODE:
						{
							SetCapH264CustomMaxFS(customValue);
							break;
						}
					case CUSTOM_MAX_DPB_CODE:
						{
							SetCapH264CustomMaxDPB(customValue);
							break;
						}
					case CUSTOM_MAX_BR_CODE:
						{
							SetCapH264CustomMaxBRandCPB(customValue);
							break;
						}
					case CUSTOM_SAR_CODE:
						{
							SetCapH264CustomMaxSAR(customValue);
							break;
						}
					case CUSTOM_MAX_STATIC_MBPS:
					case CUSTOM_MAX_RCMD_NAL_UNIT_SIZE:
					case CUSTOM_MAX_NAL_UNIT_SIZE:
					case ADDITIONAL_MODES_SUPPORETD:
					case ADDITIONAL_DISPLAY_CAPABILITIES:
					{
						//unsupported cap in MCMS level
						break;
					}
					default:
						{
							DBGPASSERT(customCode);
							break;
						}
					}
				}//while
			}
		}
	default :
	  break;
	}
}
/////////////////////////////////////////////////////////////////////////////
CCapSetH264& CCapSetH264::operator=(const CCapSetH264 &other)
{
	m_profileValue = other.m_profileValue;
	m_levelValue = other.m_levelValue;
	m_capH264CustomMaxMBPS = other.m_capH264CustomMaxMBPS;
	m_capH264CustomMaxFS = other.m_capH264CustomMaxFS;
	m_capH264CustomMaxDPB = other.m_capH264CustomMaxDPB;
	m_capH264CustomMaxBRandCPB = other.m_capH264CustomMaxBRandCPB;
	m_capH264CustomMaxSAR = other.m_capH264CustomMaxSAR;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CCapSetH264::SetBasicCapsH264(BYTE profile,BYTE level)
{
	PASSERT(profile != H264_Profile_BaseLine );
	SetCapH264ProfileValue(profile);
	SetCapH264LevelValue(level);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2004	programer: Ron
// Description:		The function Intersect the the data members with "other" cap set with equal or higher level
// input:			const CCapH264& other - to intersect with.
// Returns:			is_changed -  data members changed.
// Remarks:			1. Created 01/2004 for Highest Common Phase 2 feature.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapSetH264::IntersectSameOrHigherLevel(const CCapSetH264& other, WORD& is_changed)
{
	is_changed = FALSE;
	// other must have same or higher level
	if(m_levelValue > other.m_levelValue){
		PTRACE(eLevelError,"CCapSetH264::IntersectSameOrHigherLevel - level is lower");
		return;
	}
	// if the set don't have custom parameter the the other set with equal or higher level will always contain it.
	// we intesect the set only if it has a custum parameter
	if(IsCustom())
	{
		WORD custom_change = FALSE;
		if(m_levelValue == other.m_levelValue){
			if(!other.IsCustom())
			{
				// if other doesn't have a custum parameter
				// we reset the custum parameters
				PTRACE(eLevelInfoNormal,"CCapSetH264::IntersectSameOrHigherLevel - other has no custom parameters - set to default");
				SetNotCustom();
				is_changed = TRUE;
			}else{
				// if other has a custum parameter
				// we take the minimum of the custum parameters
				PTRACE(eLevelInfoNormal,"CCapSetH264::IntersectSameOrHigherLevel - both this and ither has custom parameters - set minimum");
				SetMinCustomSameLevel(other,custom_change);
				if(custom_change){
					is_changed = TRUE;
				}
			}
		}else{
				// we take the minimum between the custum parameter
				// and the default(or custum) value of the higher level
				SetMinCustomSameOrHigherLevel(other,custom_change);
				if(custom_change){
					is_changed = TRUE;
				}
		}
	}else{
		PTRACE(eLevelInfoNormal,"CCapSetH264::IntersectSameOrHigherLevel - this has no custom parameters - leave the default");

	}
	return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2004	programer: Ron
// Description:		The function Intersect the the data members with "other" cap set with lower level
// input:			const CCapH264& other - to intersect with.
// Returns:			is_changed -  data members changed.
// Remarks:			1. Created 01/2004 for Highest Common Phase 2 feature.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapSetH264::IntersectLowerLevel(const CCapSetH264& other, WORD& is_changed)
{

	PTRACE(eLevelInfoNormal,"CCapSetH264::IntersectLowerLevel");
	is_changed = FALSE;
	// if others level is not (strongly) lower - we shouldn't got into this function
	if(m_levelValue <= other.m_levelValue){
		PTRACE(eLevelError,"CCapSetH264::IntersectLowerLevel - level is higher or equal");
		return;
	}

	// the cap set changed because the level is lower
	is_changed = TRUE;

	if(!other.IsCustom())
	{
		// if "other" have lower level with default parameters - we set it in "this" cap set
		m_levelValue = other.m_levelValue;
		SetNotCustom();
		is_changed = TRUE;
	}else{
		// if "other" have lower level with at list 1 custom parameter
		// we set the parameter value to minimum of the custom value
		// and the higher level default (or custom) value
		CH264Details H264Details(m_levelValue);
		// 1)mbps
		if(other.m_capH264CustomMaxMBPS==0xFFFF){
		    // no custom value
			m_capH264CustomMaxMBPS=0xFFFF;
		}else{
			if(m_capH264CustomMaxMBPS==0xFFFF){
				// default of higher level
				WORD CustomMaxMBPS = H264Details.GetDefaultMbpsAsDevision();
				if(other.m_capH264CustomMaxMBPS < CustomMaxMBPS){
					m_capH264CustomMaxMBPS = other.m_capH264CustomMaxMBPS;
				}
			}else{
				// both custom values
				if(other.m_capH264CustomMaxMBPS < m_capH264CustomMaxMBPS){ //bug fix 22344
					m_capH264CustomMaxMBPS = other.m_capH264CustomMaxMBPS;
				}
			}
		}
		// 2) fs
		if(other.m_capH264CustomMaxFS==0xFFFF){
		    // no custom value
			m_capH264CustomMaxFS=0xFFFF;
		}else{
			if(m_capH264CustomMaxFS==0xFFFF){
				// default of higher level
				WORD CustomMaxFS = H264Details.GetDefaultFsAsDevision();
				if(other.m_capH264CustomMaxFS < CustomMaxFS){
					m_capH264CustomMaxFS = other.m_capH264CustomMaxFS;
				}
			}else{
				// both custom values
				if(other.m_capH264CustomMaxFS < m_capH264CustomMaxFS){//bug fix 22344
					m_capH264CustomMaxFS = other.m_capH264CustomMaxFS;
				}
			}
		}
		// 3) dpb
		if(other.m_capH264CustomMaxDPB==0xFFFF){
		    // no custom value
			m_capH264CustomMaxDPB=0xFFFF;
		}else{
			if(m_capH264CustomMaxDPB==0xFFFF){
				// default of higher level
				WORD CustomMaxDPB = H264Details.GetDefaultDpbAsDevision();
				if(other.m_capH264CustomMaxDPB < CustomMaxDPB){
					m_capH264CustomMaxDPB = other.m_capH264CustomMaxDPB;
				}
			}else{
				// both custom values
				if(other.m_capH264CustomMaxDPB < m_capH264CustomMaxDPB){ //bug fix 22344
					m_capH264CustomMaxDPB = other.m_capH264CustomMaxDPB;
				}
			}
		}

		// 4) BRandCPB
		if(other.m_capH264CustomMaxBRandCPB==0xFFFF){
		    // no custom value
			m_capH264CustomMaxBRandCPB=0xFFFF;
		}else{
			if(m_capH264CustomMaxBRandCPB==0xFFFF){
				// default of higher level
				WORD CustomMaxBRandCPB = H264Details.GetDefaultBrAsDevision();
				if(other.m_capH264CustomMaxBRandCPB < CustomMaxBRandCPB){
					m_capH264CustomMaxBRandCPB = other.m_capH264CustomMaxBRandCPB;
				}
			}else{
				// both custom values
				if(other.m_capH264CustomMaxBRandCPB < m_capH264CustomMaxBRandCPB){//bug fix 22344
					m_capH264CustomMaxBRandCPB = other.m_capH264CustomMaxBRandCPB;
				}
			}
		}
		// set the level
		m_levelValue = other.m_levelValue;

	}

	return;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2004	programer: Ron
// Description:		the function check if 1 of the parameters is custom
// Remarks:			1. Created 01/2004 for Highest Common Phase 2 feature.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH264::IsCustom() const
{
	BYTE r_val = NO;
	if(m_capH264CustomMaxMBPS!=0xFFFF || m_capH264CustomMaxFS!=0xFFFF || m_capH264CustomMaxDPB!=0xFFFF || m_capH264CustomMaxBRandCPB!=0xFFFF ){
		r_val = YES;
	}
	return r_val;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2004	programer: Ron
// Description:		the function resets the custom parameters
// Remarks:			1. Created 01/2004 for Highest Common Phase 2 feature.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CCapSetH264::SetNotCustom()
{
	// reset parameters
	m_capH264CustomMaxMBPS = 0xFFFF;
	m_capH264CustomMaxFS = 0xFFFF;
	m_capH264CustomMaxDPB = 0xFFFF;
	m_capH264CustomMaxBRandCPB = 0xFFFF;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2004	programer: Ron
// Description:		the function set the data members to the minimum of each parameters
// input:			const CCapH264& other - cap set with the same level.
// Returns:			is_changed -  data members changed.
// Remarks:			1. Created 01/2004 for Highest Common Phase 2 feature.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CCapSetH264::SetMinCustomSameLevel(const CCapSetH264& other, WORD& is_changed)
{
	CMedString sstr;

	sstr << "this = " << m_capH264CustomMaxMBPS << " , " << m_capH264CustomMaxFS << " , " << m_capH264CustomMaxDPB << " , " << m_capH264CustomMaxBRandCPB <<"\n";
	sstr << "other = " << other.m_capH264CustomMaxMBPS << " , " << other.m_capH264CustomMaxFS << " , " << other.m_capH264CustomMaxDPB << " , " << other.m_capH264CustomMaxBRandCPB <<"\n";
	is_changed = FALSE;
	// if levels are different - we return
	if(m_levelValue != other.m_levelValue){
		PTRACE(eLevelError,"CCapSetH264::IntersectHigherLevel - level is not higher");
		return;
	}
	// default value is always minimum
	// 1st parameter
	if(m_capH264CustomMaxMBPS != 0xFFFF)
	{
		if(other.m_capH264CustomMaxMBPS==0xFFFF || other.m_capH264CustomMaxMBPS<m_capH264CustomMaxMBPS)
		{
			m_capH264CustomMaxMBPS = other.m_capH264CustomMaxMBPS;
			is_changed = TRUE;
		}
	}
	// 2nd parameter
	if(m_capH264CustomMaxFS != 0xFFFF)
	{
		if(other.m_capH264CustomMaxFS==0xFFFF || other.m_capH264CustomMaxFS<m_capH264CustomMaxFS)
		{
			m_capH264CustomMaxFS = other.m_capH264CustomMaxFS;
			is_changed = TRUE;
		}
	}
	// 3rd parameter
	if(m_capH264CustomMaxDPB != 0xFFFF)
	{
		if(other.m_capH264CustomMaxDPB==0xFFFF || other.m_capH264CustomMaxDPB<m_capH264CustomMaxDPB)
		{
			m_capH264CustomMaxDPB = other.m_capH264CustomMaxDPB;
			is_changed = TRUE;
		}
	}
	// 4th parameter
	if(m_capH264CustomMaxBRandCPB != 0xFFFF)
	{
		if(other.m_capH264CustomMaxBRandCPB==0xFFFF || other.m_capH264CustomMaxBRandCPB<m_capH264CustomMaxBRandCPB)
		{
			m_capH264CustomMaxBRandCPB = other.m_capH264CustomMaxBRandCPB;
			is_changed = TRUE;
		}
	}
	sstr << "intersected this = " << m_capH264CustomMaxMBPS << " , " << m_capH264CustomMaxFS << " , " << m_capH264CustomMaxDPB << " , " << m_capH264CustomMaxBRandCPB <<"\n";
	PTRACE2(eLevelInfoNormal,"CCapSetH264::SetMinCustomSameLevel:\n",sstr.GetString());

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2004	programer: Ron
// Description:		the function set the data members to the minimum of each parameters
// input:			const CCapH264& other - cap set with the same or higher level.
// Returns:			is_changed -  data members changed.
// Remarks:			1. Created 01/2004 for Highest Common Phase 2 feature.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CCapSetH264::SetMinCustomSameOrHigherLevel(const CCapSetH264& other, WORD& is_changed)
{
	is_changed = FALSE;
	// if not same or higher level - return
	if(m_levelValue > other.m_levelValue){
		PTRACE(eLevelError,"CCapSetH264::IntersectHigherLevel - level is not higher");
		return;
	}
	// init values to intersect from "other"s values
	WORD otherCapH264CustomMaxMBPS = other.m_capH264CustomMaxMBPS;
	WORD otherCapH264CustomMaxFS = other.m_capH264CustomMaxFS;
	WORD otherCapH264CustomMaxDPB = other.m_capH264CustomMaxDPB;
	WORD otherCapH264CustomMaxBRandCPB = other.m_capH264CustomMaxBRandCPB;
	// if other have higher level we set the default values to "other"s level default values
	if(m_levelValue<other.m_levelValue)
	{
		CH264Details H264Details(other.m_levelValue);
		if(otherCapH264CustomMaxMBPS==0xFFFF){
			otherCapH264CustomMaxMBPS = H264Details.GetDefaultMbpsAsDevision();
		}
		if(otherCapH264CustomMaxFS==0xFFFF){
			otherCapH264CustomMaxFS = H264Details.GetDefaultFsAsDevision();
		}
		if(otherCapH264CustomMaxDPB==0xFFFF){
			otherCapH264CustomMaxDPB = H264Details.GetDefaultDpbAsDevision();
		}
		if(otherCapH264CustomMaxBRandCPB==0xFFFF){
			otherCapH264CustomMaxBRandCPB = H264Details.GetDefaultBrAsDevision();
		}
	}
	// set the data members to minimum values (default value is always minimu)
	// 1st parameter
	if(m_capH264CustomMaxMBPS != 0xFFFF)
	{
		if(otherCapH264CustomMaxMBPS<m_capH264CustomMaxMBPS)
		{
			m_capH264CustomMaxMBPS = otherCapH264CustomMaxMBPS;
			is_changed = TRUE;
		}
	}
	// 2nd parameter
	if(m_capH264CustomMaxFS != 0xFFFF)
	{
		if(otherCapH264CustomMaxFS<m_capH264CustomMaxFS)
		{
			m_capH264CustomMaxFS = otherCapH264CustomMaxFS;
			is_changed = TRUE;
		}
	}
	// 3rd parameter
	if(m_capH264CustomMaxDPB != 0xFFFF)
	{
		if(otherCapH264CustomMaxDPB<m_capH264CustomMaxDPB)
		{
			m_capH264CustomMaxDPB = otherCapH264CustomMaxDPB;
			is_changed = TRUE;
		}
	}
	// 4th parameter
	if(m_capH264CustomMaxBRandCPB != 0xFFFF)
	{
		if(otherCapH264CustomMaxBRandCPB<m_capH264CustomMaxBRandCPB)
		{
			m_capH264CustomMaxBRandCPB = otherCapH264CustomMaxBRandCPB;
			is_changed = TRUE;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void CCapSetH264::SetCustomValuesToExplicit()
{
	CH264Details H264Details(m_levelValue);

	m_capH264CustomMaxMBPS = H264Details.GetDefaultMbpsAsDevision();
	m_capH264CustomMaxFS = H264Details.GetDefaultFsAsDevision();
	m_capH264CustomMaxDPB = H264Details.GetDefaultDpbAsDevision();
	m_capH264CustomMaxBRandCPB = H264Details.GetDefaultBrAsDevision();

}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
// void CCapSetH264::CreateVSWFixed(WORD MaxBitRate)
// {
// 	PTRACE(eLevelError,"CCapSetH264::CreateVSWFixed");

// 	m_capH264CustomMaxFS = 0xFFFF;
// 	m_capH264CustomMaxMBPS = 0xFFFF;
// 	m_capH264CustomMaxDPB = 0xFFFF;
// 	m_capH264CustomMaxBRandCPB = 0xFFFF;
// 	switch(MaxBitRate)
// 	{

// 		case 64:
// 		{
// 			m_levelValue = H264_Level_1;
// 			break;
// 		}
// 		case 128:
// 		case 192:
// 		case 256:
// 		case 320:
// 		{
// 			m_levelValue = H264_Level_1_2;
// 			break;
// 		}
// 		case 384:
// 		{
// 			m_levelValue = H264_Level_1_2;
// 			m_capH264CustomMaxMBPS = GetMaxMbpsAsDevision(10000);
// 			break;
// 		}

// 		case 512:
// 		{
// 			if(::GetpSystemCfg()->GetEnableHDSDInFixedMode() == NO)
// 			{
// 				m_levelValue = H264_Level_1_2;
// 				m_capH264CustomMaxMBPS = GetMaxMbpsAsDevision(10000);
// 				m_capH264CustomMaxBRandCPB = ConvertMaxBrToMaxBrAndCpb(512000); //512*1000 bits/s
// 			}
// 			else/// HHR - up to date VSX 8000 is the only EP that supports HHR.
// 				/// It signals level 1.3 with custom parameters CustomMaxMBPS =40,
// 				/// CustomMaxBRandCPB = 50 and CustomMaxFS = 7.
// 			{
// 				m_levelValue = H264_Level_2_1;

// 				// commented in order to adjust MGC behavior to RMX behavior in HD / SD (Eitan 02/2007)

// 				/*m_levelValue = H264_Level_1_3;////H264_Level_1_2;
// 				CH264Details h264_level_2_1_details(H264_Level_2_1);
// 				m_capH264CustomMaxMBPS = h264_level_2_1_details.GetDefaultMbpsAsDevision();
// 				m_capH264CustomMaxFS = h264_level_2_1_details.GetDefaultFsAsDevision();
// 				CH264Details h264_level_1_2_details(H264_Level_1_2);
// 				m_capH264CustomMaxDPB = h264_level_1_2_details.GetDefaultDpbAsDevision();
// 				m_capH264CustomMaxBRandCPB = GetMaxBrAsDevision(1250000);//GetMaxBrAsDevision(1000);
// 			*/
// 			}


// 			break;
// 		}

// 		case 768:
// 		case 1152:
// 		{
// 			if(::GetpSystemCfg()->GetEnableHDSDInFixedMode() == NO)
// 			{
// 				m_levelValue = H264_Level_1_2;
// 				m_capH264CustomMaxMBPS = GetMaxMbpsAsDevision(10000);
// 				m_capH264CustomMaxBRandCPB = ConvertMaxBrToMaxBrAndCpb(768000); //768*1000 bits/s
// 			}
// 			else //SD
// 			{
// 				m_levelValue = H264_Level_2_2;
// 				CH264Details h264_level_3_details(H264_Level_3);
// 				m_capH264CustomMaxMBPS = h264_level_3_details.GetDefaultMbpsAsDevision();

// 				// commented in order to adjust MGC behavior to RMX behavior in HD / SD (Eitan 02/2007)

// 				/*m_levelValue = H264_Level_2;
// 				CH264Details h264_level_3_details(H264_Level_3);
// 				m_capH264CustomMaxFS = h264_level_3_details.GetDefaultFsAsDevision();
// 				m_capH264CustomMaxMBPS = h264_level_3_details.GetDefaultMbpsAsDevision();
// 				m_capH264CustomMaxDPB = h264_level_3_details.GetDefaultDpbAsDevision();
// 				m_capH264CustomMaxBRandCPB = ConvertMaxBrToMaxBrAndCpb(768000);
// 				*/
// 			}

// 			break;
// 		}

// 		case 1472:
// 		case 1536:
// 		case 1920:
// 		{
// 			if(::GetpSystemCfg()->GetEnableHDSDInFixedMode() == NO)
// 			{
// 				m_levelValue = H264_Level_1_2;
// 				m_capH264CustomMaxMBPS = GetMaxMbpsAsDevision(10000);
// 				m_capH264CustomMaxBRandCPB = ConvertMaxBrToMaxBrAndCpb(768000); //up to date the EPS send BR that is less than 1472000 bit rate
// 			}
// 			else //HD
// 			{
// 				m_levelValue = H264_Level_3_1;
// 				// we should declare on custom FS explicitly because Level 3.1 default FS  = 3600 < HD resultion (3840).
// 				// default value by devision is 15 -> FS = 3840
// 				// if we will declare 0xFFFF -> FS = 3600 < HD
// 				CH264Details h264_level_3_1_details(H264_Level_3_1);
// 				m_capH264CustomMaxFS = h264_level_3_1_details.GetDefaultFsAsDevision();

// 				// commented in order to adjust MGC behavior to RMX behavior in HD / SD (Eitan 02/2007)

// 				/*m_levelValue = H264_Level_2;
// 				CH264Details h264_level_3_1_details(H264_Level_3_1);
// 				m_capH264CustomMaxFS = h264_level_3_1_details.GetDefaultFsAsDevision();
// 				m_capH264CustomMaxMBPS = h264_level_3_1_details.GetDefaultMbpsAsDevision();
// 				m_capH264CustomMaxDPB = h264_level_3_1_details.GetDefaultDpbAsDevision();
// 				m_capH264CustomMaxBRandCPB = ConvertMaxBrToMaxBrAndCpb(1472000);
// 				*/
// 			}
// 			break;
// 		}
// 		default:
// 		{
// 			m_levelValue = MAX_H264_LEVEL_SUPPORTED_IN_VSW;
// 			DBGPASSERT(1);
// 			break;
// 		}
// 	}
// }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapSetH264::GetAdditionalsAsDevision(WORD& mbps, WORD& fs, WORD& dpb ,WORD& br)
{
	CH264Details h264Details(m_levelValue);


	mbps = m_capH264CustomMaxMBPS;
	if (mbps == 0xFFFF)
		mbps = h264Details.GetDefaultMbpsAsDevision();

	fs = m_capH264CustomMaxFS;
	if (fs == 0xFFFF)
		fs = h264Details.GetDefaultFsAsDevision();

	dpb = m_capH264CustomMaxDPB;
	if (dpb == 0xFFFF)
		dpb = h264Details.GetDefaultDpbAsDevision();

	WORD brAndCpb = m_capH264CustomMaxBRandCPB;
	if (brAndCpb == 0xFFFF)
		br = h264Details.GetDefaultBrAsDevision();
	else
		br = brAndCpb;//GetMaxBrAsDevision(brAndCpb*CUSTOM_MAX_BR_FACTOR);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2007	programer: Eitan
// Description:		the function set the right level according to the custom parameters
//
// Remarks:			1. Created 01/2007
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapSetH264::Legalize()
{
	WORD Mbps, Fs, Dpb , Br;
	Mbps = Fs = Dpb = Br =  0;
	GetAdditionalsAsDevision(Mbps, Fs, Dpb, Br);

	CH264Details FirstLevelDetails(H264_Level_1_1);
	BYTE	bFoundMatchingLevel = FALSE;
	CSmallString details;
	details  << "CCapSetH264::Legalize, setting level to :";

	for (int i = eLevel_1_1; i< eLastLevel && !bFoundMatchingLevel; i++)
	{
		H264DetailsStruct tmpStruct = FirstLevelDetails.GetDetailsStructForLevel(i);
		CH264Details tmpLevelDetails(tmpStruct.levelValue);

		H264DetailsStruct prevTmpStruct = tmpLevelDetails.GetDetailsStructForLevel(i-1);


		if (Fs < tmpLevelDetails.GetDefaultFsAsDevision())
		{
			details << prevTmpStruct.levelValue << " reason: FS";
			bFoundMatchingLevel = TRUE;
		}
		else if(Mbps < tmpLevelDetails.GetDefaultMbpsAsDevision())
		{
			details << prevTmpStruct.levelValue << " reason: MBPS";
			bFoundMatchingLevel = TRUE;
		}
		else if(Dpb < tmpLevelDetails.GetDefaultDpbAsDevision())
		{
			details << prevTmpStruct.levelValue << " reason: DPB";
			bFoundMatchingLevel = TRUE;
		}
		else if(Br < tmpLevelDetails.GetDefaultBrAsDevision())
		{
			details << prevTmpStruct.levelValue << " reason: BR";
			bFoundMatchingLevel = TRUE;
		}


		if (bFoundMatchingLevel)
		{
			PTRACE(eLevelInfoNormal,details.GetString());
			Create(prevTmpStruct.levelValue,Mbps, Fs, Dpb, Br);
		}

	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2007	programmer: Eitan
// Description:		The function checks if the cap set is higher then the conf scm
// input:			CVidMode confVidMode - the conference video mode.
// Returns:			rVal -  (0 - false , 1 - true)
// Remarks:			1. Created 02/2007
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapSetH264::AreRmtCapsContainConfCaps(CVidMode confVidMode)
{

	WORD rVal = 0;
	WORD Mbps, Fs, Dpb , Br;
	Mbps = Fs = Dpb = Br =  0;
	GetAdditionalsAsDevision(Mbps, Fs, Dpb, Br);

	CH264VidMode confH264VidMode = confVidMode.GetH264VidMode();
	WORD confLevel = confH264VidMode.GetLevelValue();
	WORD confMbps = confH264VidMode.GetMaxMBPSasCustomWord();
	WORD confFs = confH264VidMode.GetMaxFSasCustomWord();

	CH264Details confDetails(confLevel);

	if (confMbps == 0xFFFF)
		confMbps = confDetails.GetDefaultMbpsAsDevision();
	if (confFs == 0xFFFF)
		confFs = confDetails.GetDefaultFsAsDevision();



	CMedString details;
	details << "CCapSetH264::AreCapsContainedInConfCaps? ";
	if (Mbps < confMbps)
	{
		rVal = 0;
		details << "NO! \n Reason: party Mbps " << Mbps << " < conf Mbps " << confMbps;
	}
	else if (Fs < confFs)
	{
		rVal = 0;
		details << "NO! \n Reason: party Fs " << Fs << " < conf Fs " << confFs;
	}
	else
	{
		rVal = 1;
		details << " YES!";
	}

	PTRACE(eLevelInfoNormal,details.GetString());
	return rVal;

}
/////////////////////////////////////////////////////////////////////////////
// constructor
CCapH264::CCapH264()
{
    m_capH264 = new std::multiset<CCapSetH264*, CompareByH264>;
}
/////////////////////////////////////////////////////////////////////////////
CCapH264::CCapH264(const CCapH264& other) : CPObject(other)
{
 	m_capH264 = new std::multiset<CCapSetH264*, CompareByH264>;

    CCapSetH264* pCapSetH264 = NULL;
    std::multiset<CCapSetH264*, CompareByH264>::iterator other_itr = other.m_capH264->begin();
	for ( ; other_itr !=  other.m_capH264->end(); ++other_itr)
	{
		 CCapSetH264* temp = (*other_itr);
		 pCapSetH264 = new CCapSetH264;
		 *pCapSetH264 = *temp;

		 InsertH264CapSet(pCapSetH264);
	}
}
/////////////////////////////////////////////////////////////////////////////
CCapH264::~CCapH264()	// destructor
{
    ClearAndDestroy();
	delete m_capH264;
}
/////////////////////////////////////////////////////////////////////////////
const char* CCapH264::NameOf()  const
{
	return "CCapH264";
}
/////////////////////////////////////////////////////////////////////////////
void CCapH264::Dump(void) const
{
	WORD length = m_capH264->size();
	if( length == 0 )
		return;

	std::ostringstream  msg;
    msg << "\n==================    CCapH264::Dump    ==================\n" ;
	int i=0;
    std::multiset<CCapSetH264*, CompareByH264>::iterator itr = m_capH264->begin();
	for ( ; itr != m_capH264->end(); ++itr, i++ )
	{
	    msg << "\n\tCCapSetH264::Dump ----- Set number " << i+1 << "\n" ;
		msg <<     "\t------------------------------------\n" ;
		CCapSetH264* pCapSetH264 = (*itr);
		pCapSetH264->Dump(msg);
	}
	msg << "\n=============== CCapH264::Dump Finished!!! ===============\n" ;
	msg << "\n";
	PTRACE(eLevelInfoNormal, (char*)msg.str().c_str());
}
/////////////////////////////////////////////////////////////////////////////
void CCapH264::Dump(std::ostream& ostr) const
{
	WORD length = m_capH264->size();
	if( length == 0 )
		return;

	ostr << "\n==================    CCapH264::Dump    ==================\n" ;
	int i=0;
    std::multiset<CCapSetH264*, CompareByH264>::iterator itr = m_capH264->begin();
	for ( ; itr != m_capH264->end(); ++itr, i++ )
	  {
	    ostr << "\n\t CCapSetH264::Dump ----- Set number " << i+1 << "\n" ;
		ostr <<     "\t------------------------------------\n" ;
		CCapSetH264* pCapSetH264 = (*itr);
		pCapSetH264->Dump(ostr);
	  }
	ostr << "\n=============== CCapH264::Dump Finished!!! ===============\n" ;
	ostr << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void  CCapH264::CreateDefault()
{
    ClearAndDestroy();
	// Add here your initialization actions
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CCapH264::Create(CSegment& seg,BYTE mbeLen)
{
	BYTE numberOfH264CapBytes,IsErrorEndOfSegment,readBytes = 0;
	BYTE cap = 1;
	OFF(IsErrorEndOfSegment);

	PASSERT_AND_RETURN(!mbeLen);	//wrong lenght MBE H264 message.
	numberOfH264CapBytes = mbeLen;

	// Romem - 25.5.11: High profile compatability with HDX ISDN
	BOOL bCfgEnableHighfProfileInIsdn = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE_WITH_ISDN);
	BOOL bEnableHighfProfileInIsdn = FALSE;
	if( IsFeatureSupportedBySystem(eFeatureH264HighProfile) && bCfgEnableHighfProfileInIsdn )
	{
		bEnableHighfProfileInIsdn = 1;
	}

	if(!bEnableHighfProfileInIsdn)
	{
	   ClearAndDestroy(); // Remove all old CCapSetH264.
	}

    CSegment* CopyOfseg = new CSegment(seg);
	CSegment* CapSetSeg = new CSegment;

	while( readBytes < numberOfH264CapBytes  )
	{
		if( seg.EndOfSegment() ) {  // Wrong message length.
			PTRACE(eLevelError,"CCapH264::Create - BAD H264 CAP SEGMENT RECEIVED.");
			ON(IsErrorEndOfSegment);
			break;
		}
		cap = 1;
		while((cap!= 0) && (readBytes < numberOfH264CapBytes)) // 0 says that a new set is followes
		{
			seg>>cap;
			// VNGFE 574
			// Tandberg Edge 95 Version F6 sends 2 capsets of h.264,
			// in the second capset the profile value is 0, we will put H264_Profile_Main instead of 0 (Eitan 5/07)

			if (cap != 0)
			{
				*CapSetSeg <<cap;
			}
			else if (cap == 0 && readBytes == 0)
			{
				*CapSetSeg << (BYTE)H264_Profile_Main;
				cap = 1;
			}
			readBytes++;
		}
		CCapSetH264* NewCCapSetH264 = new CCapSetH264;
		NewCCapSetH264->DeSerialize(NATIVE, *CapSetSeg);
		InsertH264CapSet(NewCCapSetH264);
		//m_capH264->insert(NewCCapSetH264); //insert does Not copy the object before
		//NewCCapSetH264 = NULL;
		CapSetSeg->Reset();
	}

	POBJDELETE(CapSetSeg);

	if(IsErrorEndOfSegment)
	{
		DBGPASSERT(mbeLen);
	    PrintH264CapSeg(*CopyOfseg,mbeLen);
	}

	POBJDELETE(CopyOfseg);
}
/////////////////////////////////////////////////////////////////////////////
void  CCapSetH264::Create(BYTE level,WORD maxMBPS, WORD maxFS, WORD maxDPB, WORD maxBRandCPB, BYTE maxSAR, BYTE profile)
{

	BOOL bEnableIsdnHighfProfile = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE_WITH_ISDN);
	if(bEnableIsdnHighfProfile)
	{
		m_profileValue = profile;
	}
	else
	{
	    m_profileValue = H264_Profile_BaseLine;
	}
	m_levelValue = level;
	m_capH264CustomMaxMBPS = maxMBPS;
	m_capH264CustomMaxFS = maxFS;
	m_capH264CustomMaxDPB = maxDPB;
	m_capH264CustomMaxBRandCPB = maxBRandCPB;

	// legelize explicit values to tefault values
	CH264Details h264_level_details(level);
	WORD DefaultCustomMaxMBPS = h264_level_details.GetDefaultMbpsAsDevision();
	if(maxMBPS == DefaultCustomMaxMBPS){
		m_capH264CustomMaxMBPS = 0xFFFF;
	}
	WORD DefaultCustomMaxFS = h264_level_details.GetDefaultFsAsDevision();
	if(maxFS == DefaultCustomMaxFS){
		m_capH264CustomMaxFS = 0xFFFF;
	}
	WORD DefaultCustomMaxDPB = h264_level_details.GetDefaultDpbAsDevision();
	if(maxDPB == DefaultCustomMaxDPB){
		m_capH264CustomMaxDPB = 0xFFFF;
	}
	WORD DefaultCustomMaxBRandCPB = h264_level_details.GetDefaultBrAsDevision();
	if(maxBRandCPB == DefaultCustomMaxBRandCPB){
		m_capH264CustomMaxBRandCPB = 0xFFFF;
	}
	SetCapH264CustomMaxSAR(maxSAR);
}

/////////////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH264::IsCapableOfHD720_15() const
{
	BYTE bRes  = FALSE;
	WORD thisLevel = m_levelValue;
	if (thisLevel >= H264_HD720_LEVEL)
	  return TRUE;

	CH264Details thisH264Details (thisLevel);
	APIS32 thisFs = m_capH264CustomMaxFS;
	if (thisFs == -1)
	  thisFs = thisH264Details.GetDefaultFsAsDevision();

	APIS32 thisMbps = m_capH264CustomMaxMBPS;
	if (thisMbps == -1)
	  thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

	WORD hd720MinimumFs   = GetMinimumHd720Fs();
	WORD hd720MinimumMbps = GetMinimumHd720At15Mbps();

	if ((thisFs >= hd720MinimumFs) && (thisMbps >= hd720MinimumMbps))
	  bRes = TRUE;

	return bRes;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH264::IsCapableOfHD1080_15() const
{
	BYTE bRes  = FALSE;
	WORD thisLevel = m_levelValue;
	if (thisLevel >= H264_HD1080_LEVEL)
	  return TRUE;

	CH264Details thisH264Details (thisLevel);
	APIS32 thisFs = m_capH264CustomMaxFS;
	if (thisFs == -1)
	  thisFs = thisH264Details.GetDefaultFsAsDevision();

	APIS32 thisMbps = m_capH264CustomMaxMBPS;
	if (thisMbps == -1)
	  thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

	WORD hd1080MinimumFs   = GetMinimumHd1080Fs();
	WORD hd1080MinimumMbps = GetMinimumHd1080At15Mbps();

	if ((thisFs >= hd1080MinimumFs) && (thisMbps >= hd1080MinimumMbps))
	  bRes = TRUE;

	return bRes;
}
///////////////////////////////////////////////////////////////
BYTE CCapSetH264::IsCapableOfHD720_50() const
{
	BYTE bRes  = FALSE;
	WORD thisLevel = m_levelValue;
	if (thisLevel >= H264_Level_3_2)
	  return TRUE;

	CH264Details thisH264Details (thisLevel);
	APIS32 thisFs = m_capH264CustomMaxFS;
	if (thisFs == -1)
	  thisFs = thisH264Details.GetDefaultFsAsDevision();

	APIS32 thisMbps = m_capH264CustomMaxMBPS;
	if (thisMbps == -1)
	  thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

	WORD hd720MinimumFs   = GetMinimumHd720Fs();
	WORD hd720At50MinimumMbps = GetMinimumHd720At50Mbps();

	if ((thisFs >= hd720MinimumFs) && (thisMbps >= hd720At50MinimumMbps))
	  bRes = TRUE;

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
eVideoPartyType CCapSetH264::GetCPVideoPartyTypeAccordingToCapabilities()
{
	eVideoPartyType videoPartyType = eVideo_party_type_none;
	WORD thisLevel = m_levelValue;
	CH264Details thisH264Details (thisLevel);
	APIS32 fs = m_capH264CustomMaxFS;
	APIS32 mbps = m_capH264CustomMaxMBPS;

	if (fs == 0xFFFF)
	  fs = thisH264Details.GetDefaultFsAsProduct();
	else
		fs*= CUSTOM_MAX_FS_FACTOR;


	if (mbps == 0xFFFF)
	  mbps = thisH264Details.GetDefaultMbpsAsProduct();
	else
		mbps *= CUSTOM_MAX_MBPS_FACTOR;

	DWORD maxFS = fs;
	DWORD maxMBPS = mbps;
	videoPartyType = GetCPH264ResourceVideoPartyType(maxFS,maxMBPS);

	CSmallString msg;
	msg <<" maxFS: " << maxFS << " maxMBPS: " << maxMBPS << " videoPartyType: " << eVideoPartyTypeNames[videoPartyType];
	PTRACE2(eLevelInfoNormal,"CCapSetH264::GetCPVideoPartyTypeAccordingToCapabilities(): ",msg.GetString());

	return videoPartyType;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH264::IsLevelValueLegal(BYTE levelValue) //static
{
	switch(levelValue)
	{
		case H264_Level_1   :
		case H264_Level_1_1 :
		case H264_Level_1_2 :
		case H264_Level_1_3 :
		case H264_Level_2   :
		case H264_Level_2_1 :
		case H264_Level_2_2 :
		case H264_Level_3   :
		case H264_Level_3_1 :
		case H264_Level_3_2 :
		case H264_Level_4   :
		case H264_Level_4_1 :
		case H264_Level_4_2 :
		case H264_Level_5   :
		case H264_Level_5_1 :
		    return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH264::IsProfileValueLegal(BYTE profileValue) //static
{
	switch(profileValue)
	{
		case H264_Profile_BaseLine:
		case H264_Profile_Main:
		case H264_Profile_Extended:
		case H264_Profile_High:
		    return TRUE;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
void CCapH264::PrintH264CapSeg(CSegment& seg,BYTE Length) const
{

	//DBGPASSERT(1);
	ALLOCBUFFER(Message,seg.GetLen() * 5 + 100);
	ALLOCBUFFER(Header,64);
	ALLOCBUFFER(MessageByte,16);
	BYTE cap;
	WORD Count = 0;

	sprintf(Header,"Message length = %d   OpCode = H262_H264 \n",Length);
	strcat(Message,Header);

	while(!seg.EndOfSegment())
	{
		seg >> cap;
		sprintf(MessageByte,"%02x ",cap);
		strcat(Message,MessageByte);
		Count++;
        if( !(Count%15) ) strcat(Message,"\n");
	}

	PTRACE2(eLevelInfoNormal,"***  WRONG MESSAGE OF H.264 CAPABILITIES  ***\n",Message);
	DEALLOCBUFFER(MessageByte);
	DEALLOCBUFFER(Header);
	DEALLOCBUFFER(Message);
}
/////////////////////////////////////////////////////////////////////////////
void  CCapH264::Serialize(WORD format,CSegment& seg)
{
	WORD length = m_capH264->size();
	if( length == (WORD)0 )
		return;

    seg << ((BYTE)(Start_Mbe | ESCAPECAPATTR)); // Insert "Start- MBE" opcode.

	// First build the H264 cap message in order to calculate it's length.
    CSegment* H264CapSeg = new CSegment;
	*H264CapSeg << (BYTE)H264_Mbe;  //H264_Mbe

    CCapSetH264* pCapSetH264 = NULL;
    WORD i = 0;
	// Initial part.
    std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator itr = m_capH264->rbegin();
	for ( ; itr != m_capH264->rend (); ++itr )// 	for (i = length ; i > 0 ; i--)
	  {
		if(itr != m_capH264->rbegin()) //if(length != i) // Before each CCapSetH263 - except from the first.
			*H264CapSeg << (BYTE)0;	     // 0 between two consecuensive sets
		pCapSetH264 = (*itr);//m_capH264->at(i-1);
        pCapSetH264->Serialize(format,*H264CapSeg);
	}

    // Calculate MBE Command length.
	BYTE H264CapSegLength = (BYTE)( (H264CapSeg->GetWrtOffset()) - (H264CapSeg->GetRdOffset()));

	seg << H264CapSegLength << *H264CapSeg;
	POBJDELETE(H264CapSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CCapH264::DeSerialize(WORD format,CSegment& seg)
{
	if(seg.EndOfSegment())  return;

	if( *(seg.GetPtr(1)) != ((BYTE)(Start_Mbe | ESCAPECAPATTR)) )
		return;

	BYTE temp,mbeCommandLen;
	seg >> temp; // Start_Mbe
	seg >> mbeCommandLen;
	seg >> temp; // H264
	if(temp != H264_Mbe) //H264_Mbe
	{
		if(temp)   PASSERT(temp);
        else       PASSERT(101);
	}

	Create(seg,mbeCommandLen-1);  //-1 since we took H264 from the segment (H264_Mbe)
}

/////////////////////////////////////////////////////////////////////////////
void CCapH264::InsertH264CapSet(CCapSetH264* pH264CapSetBuf)
{
    BYTE profile = pH264CapSetBuf->GetCapH264ProfileValue();
	BYTE level   = pH264CapSetBuf->GetCapH264LevelValue();

	if( !CCapSetH264::IsProfileValueLegal(profile) )
	{
	    DBGPASSERT(profile);
	    POBJDELETE(pH264CapSetBuf);
	}
	else if ( !CCapSetH264::IsLevelValueLegal(level) )
		{
	    	DBGPASSERT(level);
	    	POBJDELETE(pH264CapSetBuf);
		}

	else
	    m_capH264->insert(pH264CapSetBuf);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// returns the cap set with higer level (ron,01/04)
// the cap sets sorted in the RW by cap set operator < , and the last set is the biggest.
CCapSetH264* CCapH264::GetLastCapSet() const
{
	std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator itr = m_capH264->rbegin();
	CCapSetH264* retVal = (*itr);

	return retVal;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCapSetH264* CCapH264::GetH264HighProfileCapSet()
{
	CCapSetH264* pCurrentCapSetH264 = NULL;
	std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator itr = m_capH264->rbegin();
	for ( ; itr != m_capH264->rend (); ++itr )//for(size_t cap_set_index = length; cap_set_index > 0 ; cap_set_index--)
	{
		pCurrentCapSetH264 = (*itr);//m_capH264->at(cap_set_index-1);
		if(pCurrentCapSetH264->GetCapH264ProfileValue() == H264_Profile_High)
		{
		   break;
		}
	}
	return pCurrentCapSetH264;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH264::GetMaxH264Level()
{
	BYTE level = 0;
	CCapSetH264* capSetH264 = GetLastCapSet();
	if(capSetH264 != NULL){
		level = capSetH264->GetCapH264LevelValue();
	}
	return level;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH264::GetMaxH264CustomParameters(BYTE level, WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB)
{
	WORD length = m_capH264->size();

	// init parameters
	WORD level_found = FALSE;
	maxMBPS = 0;
	maxFS = 0;
	maxDPB = 0;
	maxBRandCPB = 0;

	// find same or higher level and cap set
	BYTE local_level = GetMaxH264Level();
	CCapSetH264* pLocalCapSetH264 = GetLastCapSet();

	// if we have more then 1 h.264 cap sets we look for equal level
	if( length > 1 )
	{
		CCapSetH264* pCurrentCapSetH264 = NULL;
		std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator itr = m_capH264->rbegin();
		for ( ; itr != m_capH264->rend (); ++itr )//for(size_t cap_set_index = length; cap_set_index > 0 ; cap_set_index--)
		{
			pCurrentCapSetH264 = (*itr);//m_capH264->at(cap_set_index-1);
			if(pCurrentCapSetH264->GetCapH264LevelValue() == level)
			{
				local_level = level;
				pLocalCapSetH264 = pCurrentCapSetH264;
			}
		}
	}
	if(local_level >= level)
	{
		level_found = TRUE;
	}

	// same or higher level found get custom parameters (explisit values)
	if(level_found == TRUE && pLocalCapSetH264 != NULL)
	{
		CH264Details h264ConstDetails(local_level);
		maxMBPS = pLocalCapSetH264->GetCapH264CustomMaxMBPS();
		if(maxMBPS == 0xFFFF){
			maxMBPS = h264ConstDetails.GetDefaultMbpsAsDevision();
		}
		maxFS = pLocalCapSetH264->GetCapH264CustomMaxFS();
		if(maxFS == 0xFFFF){
			maxFS = h264ConstDetails.GetDefaultFsAsDevision();
		}
		maxDPB = pLocalCapSetH264->GetCapH264CustomMaxDPB();
		if(maxDPB == 0xFFFF){
			maxDPB = h264ConstDetails.GetDefaultDpbAsDevision();
		}
		maxBRandCPB = pLocalCapSetH264->GetCapH264CustomMaxBRandCPB();
		if(maxBRandCPB == 0xFFFF){
			maxBRandCPB = h264ConstDetails.GetDefaultBrAsDevision();
		}
	}

	// dump if not found
	if(!level_found)
	{
		CSmallString sstr;
		sstr << "level = " << level;
		sstr << " , m_numberOfH264Sets = " << length;
// 		WORD dump_levels = length; //olga??
// 		if(dump_levels > 5){
// 			dump_levels = 5;
// 		}
// 		sstr << " , levels =";

		std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator itr = m_capH264->rbegin();
		for ( ; itr != m_capH264->rend (); ++itr )//for(size_t cap_set_index = dump_levels ; cap_set_index > 0 ; cap_set_index--)
		{
		    pLocalCapSetH264 = (*itr);//m_capH264->at(cap_set_index-1);
			sstr << " " << pLocalCapSetH264->GetCapH264LevelValue();
		}
		PTRACE2(eLevelInfoNormal,"CCapH264::GetMaxH264CustomParameters: level not found : ",sstr.GetString());
	}
	return level_found;
}


/////////////////////////////////////////////////////////////////////////////
void CCapH264::Remove()
{
	ClearAndDestroy();
}

/////////////////////////////////////////////////////////////////////////////
CCapSetH264* CCapH264::Find(CCapSetH264* pCapSetH264) const
{
	return (*m_capH264->find(pCapSetH264));
}
/////////////////////////////////////////////////////////////////////////////
// size_t CCapH264::Index(CCapSetH264* pCapSetH264) const
// {
// 	if(CPObject::IsValidPObjectPtr(pCapSetH264))
// 		return m_capH264->index(pCapSetH264);
// 	else
// 		return 0;
// }
/////////////////////////////////////////////////////////////////////////////
// returns CapSet with index from 0 to NumOf-1
// CCapSetH264* CCapH264::GetCapSetH264(WORD setNumber) const
// {
// 	return m_capH264->at(setNumber);
// }
/////////////////////////////////////////////////////////////////////////////
void CCapH264::ClearAndDestroy()
{
	std::multiset<CCapSetH264*, CompareByH264>::iterator itr = m_capH264->begin();
	for ( ; itr != m_capH264->end(); ++itr ) {// Remove all old CCapSetH264
	    CCapSetH264* temp = *itr;
	    POBJDELETE(temp);
	}
	m_capH264->clear();
}

/////////////////////////////////////////////////////////////////////////////
WORD CCapH264::GetNumberOfH264Sets(void) const
{
	return	m_capH264->size();
}

/////////////////////////////////////////////////////////////////////////////
CCapH264& CCapH264::operator=(const CCapH264 &other)
{
	// In order to prevent clearAndDestroy() to the same object during ExchangeCap() of the mux.
	if(this == &other)
	    return *this;

	ClearAndDestroy();

	CCapSetH264* pCapSetH264 = NULL;
    std::multiset<CCapSetH264*, CompareByH264>::iterator itr = other.m_capH264->begin();
	for ( ; itr != other.m_capH264->end(); ++itr)
	{
		 CCapSetH264* temp = (*itr);
		 pCapSetH264 = new CCapSetH264;
		 *pCapSetH264 = *temp;//*(other.m_capH264->at(i));

		 InsertH264CapSet(pCapSetH264);
	}

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
WORD CCapH264::operator<=(const CCapH264 &other) const
{
	CCapSetH264* pRemoteCapSetH264 = NULL;
	CCapSetH264* pLocalCapSetH264 = NULL;
	BYTE foundfforthisSet =0;

// 	WORD RemoteNumberOfH264Sets = other.GetNumberOfH264Sets();
// 	WORD localNumOfSets = m_capH264->size();

	std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator itr = m_capH264->rbegin();
	for ( ; itr != m_capH264->rend (); ++itr )// for( WORD i=localNumOfSets; i > 0 ; i-- )
	{
		foundfforthisSet = 0;
		pLocalCapSetH264 = (*itr);//m_capH264->at(i-1);

		std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator remoteitr = other.m_capH264->rbegin();
		for ( ; itr != other.m_capH264->rend (); ++itr )// for (WORD j = RemoteNumberOfH264Sets; j>0 ; j--)
		{
		    pRemoteCapSetH264 = (*remoteitr);//other.m_capH264->at(j-1);

			if  (*pLocalCapSetH264<=*pRemoteCapSetH264)
			{
				foundfforthisSet=1;
				break;
			}
		}
		if(foundfforthisSet==0)
			return FALSE;
	}
	return TRUE; // if you are here a cap was found for each cap set.
}

///////////////////////////////////////////////////////////////////////////////////////
/*QCIF -> returen value 99 QCIF
  CIF and up returen value 396 CIF */
WORD CCapH264::GetMaxResolution()
{
	BYTE level;
	WORD retResolution = H264_L1_DEFAULT_FS; //QCIF
	DWORD h264FS;

    CCapSetH264* pCapSetH264 = GetLastCapSet(); //the last set is with the highest level

	if(pCapSetH264)
	{
		level = pCapSetH264->GetCapH264LevelValue();
		CH264Details h264ConstDetails(level);

		if(pCapSetH264->GetCapH264CustomMaxFS() == 0xFFFF)
			h264FS = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_FS_CODE);

		else h264FS = CUSTOM_MAX_FS_FACTOR * pCapSetH264->GetCapH264CustomMaxFS();


		if (h264FS >= H264_L1_1_DEFAULT_FS) /*&& h264FS <= H264_L2_DEFAULT_FS*/
			retResolution = (WORD)H264_L1_1_DEFAULT_FS;   //H264_L1_1_DEFAULT_FS is CIF
	}

	return retResolution;
}

/////////////////////////////////////////////////////////////////////////////
// return TRUE only if we can find the same level in the cap.
BYTE CCapH264::IsVidLevel(WORD level) const
{
    std::multiset<CCapSetH264*, CompareByH264>::iterator itr = m_capH264->begin();
	for ( ; itr != m_capH264->end(); ++itr )
	    if((*itr)->GetCapH264LevelValue() == level)
		    return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// return TRUE only if we can find the same level in the cap.
CCapSetH264* CCapH264::GetVidLevelCapSet(const WORD level)
{
    std::multiset<CCapSetH264*, CompareByH264>::iterator itr = m_capH264->begin();
	for ( ; itr != m_capH264->end(); ++itr )
	    if((*itr)->GetCapH264LevelValue() == level )
		    return (*itr);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////
void CCapH264::SetOneH264Cap(BYTE level, DWORD mbps,DWORD fs,DWORD dpb,DWORD br)
{
	CCapSetH264* pLevelCapSetH264 = GetVidLevelCapSet(level);

	if( pLevelCapSetH264 ) { // if cap-set with level exists - modify it

		DWORD currentMaxMBPS = pLevelCapSetH264->GetCapH264CustomMaxMBPS();
		if (currentMaxMBPS == 0xFFFFFFFF) currentMaxMBPS = 0;
		DWORD currentMaxFS   = pLevelCapSetH264->GetCapH264CustomMaxFS();
		if (currentMaxFS == 0xFFFFFFFF) currentMaxFS = 0;
		DWORD currentMaxDPB  = pLevelCapSetH264->GetCapH264CustomMaxDPB();
		if (currentMaxDPB == 0xFFFFFFFF) currentMaxDPB = 0;
		DWORD currentMaxBR   = pLevelCapSetH264->GetCapH264CustomMaxBRandCPB();
		if (currentMaxBR == 0xFFFFFFFF) currentMaxBR = 0;
		if ((mbps > currentMaxMBPS) && (fs > currentMaxFS) && (dpb > currentMaxDPB) &&
			(br > currentMaxBR) ) //only if ALL the custom parameters are higher we want to replace the set
		{
			pLevelCapSetH264->SetCapH264CustomMaxMBPS(mbps);
			pLevelCapSetH264->SetCapH264CustomMaxFS(fs);
			pLevelCapSetH264->SetCapH264CustomMaxDPB(dpb);
			pLevelCapSetH264->SetCapH264CustomMaxBRandCPB(br);

		}
	}
	else
	{//insert a new one

		CCapSetH264* pCapSetH264 = new CCapSetH264;

		pCapSetH264->SetCapH264LevelValue(level);
		pCapSetH264->SetCapH264CustomMaxMBPS(mbps);
		pCapSetH264->SetCapH264CustomMaxFS(fs);
		pCapSetH264->SetCapH264CustomMaxDPB(dpb);
		pCapSetH264->SetCapH264CustomMaxBRandCPB(br);

		InsertH264CapSet(pCapSetH264);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2004	programer: Ron
// Description:		The function Intersect the the data members with "other" H264 caps
// input:			const CCapH264& other - to intersect with.
// Returns:			is_changed -  data members changed
// Remarks:			1. Created 01/2004 for Highest Common Phase 2 feature.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapH264::Intersect(const CCapH264& other, WORD& is_changed)
{
    PTRACE(eLevelInfoNormal,"CCapH264::Intersect");//|HC_PHASE2_TRACE
	is_changed = FALSE;

	CMedString sstr;

	if(m_capH264->size() == 0)
	{
		is_changed = FALSE;
		PTRACE(eLevelInfoNormal,"CCapH264::Intersect - this have no h264 caps");//|HC_PHASE2_TRACE
		return;
	}

	if(other.m_capH264->size() == 0)
	{
		ClearAndDestroy();
		is_changed = TRUE;
		PTRACE(eLevelInfoNormal,"CCapH264::Intersect - other have no h264 caps");//|HC_PHASE2_TRACE
		return;
	}
	// cap sets are sorted in the RW such that the last set is the higher level
	// and the first set is the lower level (sorted by operator < of the cap sets)
	size_t cap_set_index = m_capH264->size();
    std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator itr = m_capH264->rbegin();
	for ( ; itr != m_capH264->rend (); ++itr, cap_set_index-- )// for(size_t cap_set_index = m_capH264->size();
	                                                           // 	         cap_set_index > 0; cap_set_index--)
	{
		// get cap set
	    CCapSetH264* pCapSetH264 = (*itr);//m_capH264->at(cap_set_index-1);
		BYTE level_found = NO;
		sstr << "this cap set number " << (WORD)cap_set_index << " : \n";
		// search for set with equal or higher level

		std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator other_itr = other.m_capH264->rbegin();
		for ( ; itr != other.m_capH264->rend (); ++itr ) // for(size_t other_cap_set_index = other.m_capH264->size();
		                                                 //            other_cap_set_index > 0; other_cap_set_index--)

		{
		    CCapSetH264* pOtherCapSetH264 = (*other_itr);//other.m_capH264->at(cap_set_index-1);
			if(pCapSetH264->GetCapH264LevelValue() <= pOtherCapSetH264->GetCapH264LevelValue())
			{
				sstr << "found cap set with equal or higher level: this cap set level = "
					 << (WORD)pCapSetH264->GetCapH264LevelValue() << " , others cap set level = "
					 << (WORD)pOtherCapSetH264->GetCapH264LevelValue() << "\n";
				// found cap set with equal or higher level
				level_found = YES;
				// if the set don't have custom parameter the equal or higher level will always contain it.
				// we intesect the set only if it has a custum parameter
				if(pCapSetH264->IsCustom()){
					WORD is_cap_set_changed = FALSE;
					pCapSetH264->IntersectSameOrHigherLevel(*pOtherCapSetH264,is_cap_set_changed);
					if(is_cap_set_changed){
						is_changed = TRUE;
					}
				}else{
					sstr << "cap set has no custom parameter (cap set included in intersection). \n" ;
				}
			}
		}
		if(level_found == NO)
		{
			sstr << "cap set with equal or higher level not found : this cap set level = "
				 << (WORD)pCapSetH264->GetCapH264LevelValue() << "\n";
			if(cap_set_index == 1)
			{
				// all "this" sets are higher than all "others" sets

				// get "other" highest set
				size_t other_last_set_index = other.m_capH264->size();
				if(other_last_set_index > 0)
				{
				    CCapSetH264* pOtherCapSetH264 = (*other.m_capH264->rbegin());//at(other_last_set_index-1);
					is_changed = TRUE;
					WORD dummy_is_cap_set_changed;
					pCapSetH264->IntersectLowerLevel(*pOtherCapSetH264,dummy_is_cap_set_changed);
				}
			}else{
				// not found cap set with equal or higher level => remove the set
			    m_capH264->erase(*itr);//removeAt(cap_set_index-1);
				is_changed = TRUE;
			}
		}
	}
	PTRACE2(eLevelInfoNormal,"CCapH264::Intersect\n",sstr.GetString());//|HC_PHASE2_TRACE
}

///////////////////////////////////////////////////////////////////////////////////
WORD CCapH264::GetMaxCapH264FrameRate(WORD resolution)
{
//	PTRACE(eLevelInfoNormal,"CCapH264::GetMaxCapH264FrameRate");

	WORD RetFrameRate = 0;
	DWORD h264FS = 0;
	DWORD h264MBPS =0;

	CCapSetH264* pCapSetH264 = GetLastCapSet(); //the last set is with the highest level

	if(pCapSetH264)
	{
		CH264Details h264ConstDetails(pCapSetH264->GetCapH264LevelValue());

		// get default FS from level - CIF or QCIF
		h264FS = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_FS_CODE);

		if(pCapSetH264->GetCapH264CustomMaxMBPS() == 0xFFFF)
			// Get MBPS from level
			h264MBPS = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_MBPS_CODE);
		else
			// Get MBPS from remote cap
			h264MBPS = CUSTOM_MAX_MBPS_FACTOR * pCapSetH264->GetCapH264CustomMaxMBPS();
	}

	return RetFrameRate = h264MBPS/resolution;//h264FS;
}
///////////////////////////////////////////////////////////////////////////////////
BYTE CCapH264::IsResolutionCapIsHigherThanQCIF()
{
//	PTRACE(eLevelInfoNormal,"CCapH264::IsResolutionCapIsHigherThanQCIF");

	BYTE level;
	BYTE res = FALSE;
	WORD h264FS;

    CCapSetH264* pCapSetH264 = GetLastCapSet(); //the last set is with the highest level

	if(pCapSetH264)
	{
		level = pCapSetH264->GetCapH264LevelValue();
	    h264FS = pCapSetH264->GetCapH264CustomMaxFS();

		if(level >= H264_Level_1_1 ||
		   ((h264FS != 0xFFFF) && (h264FS >= H264_L1_1_DEFAULT_FS)))
			res = TRUE;
		else
			res = FALSE;
	}
	return res;
}
///////////////////////////////////////////////////////////////////////////////////
void CCapH264::Legalize()
{
    PTRACE(eLevelInfoNormal,"CCapH264::Legalize");//|HC_PHASE2_TRACE
	BYTE is_changed = FALSE;

	CMedString sstr;

	if(m_capH264->size() == 0)
	{
		is_changed = FALSE;
		PTRACE(eLevelInfoNormal,"CCapH264::Legalize - this have no h264 caps");//|HC_PHASE2_TRACE
		return;
	}

	// cap sets are sorted in the RW such that the last set is the higher level
	// and the first set is the lower level (sorted by operator < of the cap sets)
    std::multiset<CCapSetH264*, CompareByH264>::reverse_iterator itr = m_capH264->rbegin();
	for ( ; itr != m_capH264->rend (); ++itr )
	{	// get cap set
	    CCapSetH264* pCapSetH264 = (*itr);
		PTRACE(eLevelInfoNormal,"CCapH264::Legalize - Caps Before:");
		pCapSetH264->Dump();
		pCapSetH264->Legalize();
		PTRACE(eLevelInfoNormal,"CCapH264::Legalize - Caps After:");
		pCapSetH264->Dump();
	}
}

//////////////////////////////////////////////////////////////////////
BYTE CCapH264::IsCapableOfHD720_15() const
{
    std::multiset<CCapSetH264*, CompareByH264>::iterator itr = m_capH264->begin();
	for ( ; itr != m_capH264->end(); ++itr)
	{
		 CCapSetH264* temp = (*itr);
		 if(temp->IsCapableOfHD720_15())
		   return TRUE;
	}
    return FALSE;
}
//////////////////////////////////////////////////////////////////////
BYTE CCapH264::IsCapableOfHD1080_15() const
{
    std::multiset<CCapSetH264*, CompareByH264>::iterator itr = m_capH264->begin();
	for ( ; itr != m_capH264->end(); ++itr)
	{
		 CCapSetH264* temp = (*itr);
		 if(temp->IsCapableOfHD1080_15())
		   return TRUE;
	}
    return FALSE;
}
////////////////////////////////////////////////////////////////////////
BYTE CCapH264::IsCapableOfHD720_50() const
{
    std::multiset<CCapSetH264*, CompareByH264>::iterator itr = m_capH264->begin();
	for ( ; itr != m_capH264->end(); ++itr)
	{
		 CCapSetH264* temp = (*itr);
		 if(temp->IsCapableOfHD720_50())
		   return TRUE;
	}
    return FALSE;
}
////////////////////////////////////////////////////////////////////////
eVideoPartyType CCapH264::GetCPVideoPartyTypeAccordingToCapabilities()
{
	eVideoPartyType videoPartyType = eVideo_party_type_none;
	std::multiset<CCapSetH264*, CompareByH264>::iterator itr = m_capH264->begin();
	for ( ; itr != m_capH264->end(); ++itr)
	{
		 CCapSetH264* temp = (*itr);
		 eVideoPartyType tmpVideoPartyType= temp->GetCPVideoPartyTypeAccordingToCapabilities();
		 if (tmpVideoPartyType>videoPartyType)
			 videoPartyType = tmpVideoPartyType;

	}
	return videoPartyType;
}
