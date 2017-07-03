//+========================================================================+
//                            H264Cap.H                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H264Cap.H                                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15.10.07   |                                                      |
//+========================================================================+


#ifndef _H264VIDEOCAP_H
#define _H264VIDEOCAP_H


#include <set>
#include "PObject.h"
#include "H264.h"
#include "AllocateStructs.h"

class CSegment;
class CVidMode;

#define	MAX_BR_SUPPORTED_IN_VSW	      		 H264_L3_1_DEFAULT_BR  //H264_L2_DEFAULT_BR
#define MAX_MBPS_SUPPORTED_IN_VSW			 H264_L3_1_DEFAULT_MBPS
#define MAX_DPB_SUPPORTED_IN_VSW			 H264_L3_1_DEFAULT_DPB
#define MAX_CPB_SUPPORTED_IN_VSW			 H264_L3_1_DEFAULT_CPB //H264_L2_DEFAULT_CPB


//----------------------------------------------------------------------------

class CCapSetH264 : public CPObject
{
CLASS_TYPE_1(CCapSetH264, CPObject)
public:

// Constructors
	CCapSetH264();
	virtual ~CCapSetH264();

// Initializations
	void  CreateDefault();
	void  Create( BYTE capBuffer[] );
	void  Create(BYTE level,WORD maxMBPS, WORD maxFS, WORD maxDPB, WORD maxBRandCPB, BYTE maxSAR = H264_ALL_LEVEL_DEFAULT_SAR,BYTE profile = H264_Profile_BaseLine);
/* 	void  CreateVSWFixed(WORD MaxBitRate); */

// Operations
	virtual const char*  NameOf() const;
	void Dump(void) const;
	void Dump(std::ostream& ostr) const;
	void Serialize(WORD format,CSegment& seg);
	void DeSerialize(WORD format,CSegment& seg);
	CCapSetH264& operator=(const CCapSetH264 &other);
	WORD operator<(const CCapSetH264& CapSetH264) const;
	WORD operator<=(const CCapSetH264& CapSetH264) const;
    WORD operator==(const CCapSetH264& CapSetH264) const;

	void SetCapH264ProfileValue (BYTE profile);
	BYTE GetCapH264ProfileValue() const;
	void SetCapH264LevelValue (BYTE level);
	BYTE GetCapH264LevelValue() const;

	void SetCapH264CustomMaxMBPS(WORD mbps);
	WORD GetCapH264CustomMaxMBPS() const;

    void SetCapH264CustomMaxFS(WORD fs);
	WORD GetCapH264CustomMaxFS() const;

	void SetCapH264CustomMaxDPB(WORD dpb);
	WORD GetCapH264CustomMaxDPB() const;

	void SetCapH264CustomMaxBRandCPB(WORD brcpb);
	WORD GetCapH264CustomMaxBRandCPB() const;

	void SetCapH264CustomMaxSAR(WORD sar);
	WORD GetCapH264CustomMaxSAR() const;
	//BYTE CalcCustomParamFirstByte(WORD customParam);
	//BYTE CalcCustomParamSecondByte(WORD customParam);
	//WORD CalcCustomParam(BYTE firstByte,BYTE secondByte);
	void SetBasicCapsH264(BYTE profile,BYTE level);
	//BYTE IsTwoBytesNumber(WORD m_capH264CustomMaxMBPS);
	BYTE IsAnotherByteFollows(BYTE byteNum);
	BYTE IsFirstCustomSmallerOrSameAsSeconed(WORD firstCustom, WORD secondCustom)const;

	void IntersectSameOrHigherLevel(const CCapSetH264& other, WORD& is_changed);
	void IntersectLowerLevel(const CCapSetH264& other, WORD& is_changed);
	BYTE IsCustom() const;

	void Legalize();
	void GetAdditionalsAsDevision(WORD& mbps, WORD& fs, WORD& dpb ,WORD& br);
	WORD AreRmtCapsContainConfCaps(CVidMode confVidMode);

	BYTE IsCapableOfHD720_15() const;
	BYTE IsCapableOfHD1080_15() const;
	BYTE IsCapableOfHD720_50() const;
	eVideoPartyType GetCPVideoPartyTypeAccordingToCapabilities();

	static BYTE IsLevelValueLegal(BYTE level);
	static BYTE IsProfileValueLegal(BYTE profile);

protected:

	void SetNotCustom();
	void SetMinCustomSameLevel(const CCapSetH264& other, WORD& is_changed);
	void SetMinCustomSameOrHigherLevel(const CCapSetH264& other, WORD& is_changed);

//	void SetCustomValuesToExplicit();



private:

	// Attributes
  BYTE m_profileValue;   // VideoConference bit 2 is 1 / Cable,DigitalTV,DVD...
  BYTE m_levelValue;

  //Aditional Parameters (Optional)
  WORD m_capH264CustomMaxMBPS;  // For higher processing rate capability(Rate in MBPS)
  WORD m_capH264CustomMaxFS;    // The decoder can decode larger picture(frame) sizes(Resolution MB)
						        // m_CustomMaxMBPS/m_customMaxFS --> FrameRate 1/sec
  WORD m_capH264CustomMaxDPB;   // The decoder has additional decoded picture buffer memory.
  WORD m_capH264CustomMaxBRandCPB; // The decoder can decode higher video bit rate and larger coded picture buffer

  WORD m_capH264CustomMaxSAR;	//Support of Table E-1/H.264 Extended_SAR
};


//----------------------------------------------------------------------------
/* CompareByH264 is comparison class: A class that takes two arguments
   of the same CCapSetH264 type (container's elements) and returns true if p1 less then p2 */
struct CompareByH264
{
  bool operator()(const CCapSetH264* p1, const CCapSetH264* p2) const
  {
    return (*p1) < (*p2);
  }
};

//----------------------------------------------------------------------------

/* This is capability description for H264 video coding standard */
class CCapH264 : public CPObject
{
CLASS_TYPE_1(CCapH264, CPObject)
public:

// Constructors
	CCapH264();
	CCapH264(const CCapH264&);
	virtual ~CCapH264();

// Initializations
	void  CreateDefault();
	void  Create(CSegment& seg,BYTE mbeLen);

// Operations
	virtual const char*  NameOf() const;
	void Dump(void) const;;
	void Dump(std::ostream& ostr) const;
	void SmartDump(std::ostream& ostr) const;
	void DumpCapH264(std::ostream& ostr) const;
	void Serialize(WORD format,CSegment& seg);
	void DeSerialize(WORD format,CSegment& seg);
	CCapH264& operator=(const CCapH264 &other);
	WORD operator<=(const CCapH264 &other) const;
	void InsertH264CapSet(CCapSetH264* pH264CapSetBuf);
	void SetHighestCommonVideoCapabilities(CCapH264* pCapH264,
										   WORD isAutoVidScm);
	void SetCommonCapabilities(CCapSetH264* pLocalCapSetH264,
							   CCapSetH264* pRemoteCapSetH264,
		                       CCapH264* pCapH264,
							   CCapSetH264& pNewCommonCapSetH264,
							   WORD isAutoVidScm);
	void SetOneH264Cap(BYTE level,
					   DWORD mbps,
					   DWORD fs,
					   DWORD dpb,
					   DWORD br);
	void  PrintH264CapSeg(CSegment& seg,BYTE Length) const;
    WORD GetNumberOfH264Sets(void) const ;
/* 	CCapSetH264* GetCapSetH264(WORD setNumber) const; */

	void Remove();
/*     size_t Index(CCapSetH264* pCapSetH264) const; */
	CCapSetH264* Find(CCapSetH264* pCapSetH264) const;
	BYTE IsVidLevel(WORD level) const;
	CCapSetH264* GetVidLevelCapSet(const WORD level);
	WORD GetMaxResolution();


	void ClearAndDestroy();

	void Intersect(const CCapH264& other, WORD& is_changed);
	CCapSetH264* GetLastCapSet() const;

	BYTE GetMaxH264Level();
	CCapSetH264* GetH264HighProfileCapSet();
	WORD GetMaxH264CustomParameters(BYTE level,
									WORD& maxMBPS,
									WORD& maxFS,
									WORD& maxDPB,
									WORD& maxBRandCPB);
	// PLF - inga
	WORD GetMaxCapH264FrameRate(WORD resolution);
	BYTE IsResolutionCapIsHigherThanQCIF();

	void Legalize();

	BYTE IsCapableOfHD720_15() const;
	BYTE IsCapableOfHD1080_15() const;
	BYTE IsCapableOfHD720_50() const;
	eVideoPartyType GetCPVideoPartyTypeAccordingToCapabilities();

private:

	// Attributes
    std::multiset<CCapSetH264*, CompareByH264>*  m_capH264;
};


#endif /* _H264VIDEOCAPS_H  */




