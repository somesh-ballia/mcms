//+========================================================================+
//                            H320CapPP.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H320CapPP.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 29.10.07    |                                                      |
//+========================================================================+


#ifndef _H320CAPPP_H 
#define _H320CAPPP_H


#include "PObject.h"
#include "H320VideoCaps.h"


#define pcProfile_0		0x00000001
#define pcProfile_1		0x00000002
#define pcProfile_2		0x00000004

#define AllRatesAMSC64Byte_1	0x7F
#define AllRatesAMSC64Byte_2	0x60

// Mode Types (MT) values of <Control/ID> byte from AMSC-ON command
#define ITU_STANDARD_MODE						0x00
#define PROPRIETARY_MODE						0x01
#define RESERVED_MODE							0x02

const WORD  W_MANUFACT_CODE_PUBLIC_PP			= 0x5050;

#define MANUFACT_CODE_PUBLIC_PP_BYTE_1			0x50
#define MANUFACT_CODE_PUBLIC_PP_BYTE_2			0x50

// Polycom Public Base Capabilities
const BYTE  PPXC_CAP							= 0x90;
const BYTE  PP_H221_ESCAPE_TABLE_CAP			= 0x94;
const BYTE  MEDIA_FLOW_CONTROL_CAP				= 0x99;
const BYTE  PC_PROFILE_CAP						= 0x9A;
const BYTE  AMSC_MUX_CAP						= 0x9B;
const BYTE  AMSC_MUX64_CAP						= 0x9C;

// Polycom Public Extended Capabilities 
const BYTE  EXTENDED_PP_CAP_VIDEO_STREAM		= 0x8B;
const BYTE  OPTIONAL_CAP_INDICATOR				= 0x0C;



class CCapAMSC : public CPObject
{
CLASS_TYPE_1(CCapAMSC, CPObject)
public :
	CCapAMSC();
	CCapAMSC(const CCapAMSC& other);
    virtual ~CCapAMSC();

	virtual const char* NameOf() const;
	void Dump(std::ostream& ostr) const;

	void Serialize(WORD format,CSegment &seg);
	void DeSerialize(WORD format,CSegment &seg);

	void SetAMSC_MUX(void) { m_AMSC_MUX = AMSC_MUX_CAP; }
	void SetRateAMSC_MUX64(BYTE rate);
	void SetAMSC_MUX64(BYTE rateByte1, BYTE rateByte2);
	void SetAllAMSC_MUX64RatesUpTo(BYTE rate);
	void SetOffAMSC_MUX(void) { m_AMSC_MUX = 0; }
	void SetOffAMSC_MUX64(void) { m_AMSC_MUX64_RateByte1 = m_AMSC_MUX64_RateByte2 = 0; }
	BYTE IsAMSC_MUX(void) const {return m_AMSC_MUX;}
	BYTE IsAMSC_MUX64(void) {return (m_AMSC_MUX64_RateByte1 || m_AMSC_MUX64_RateByte2);}

	CCapAMSC& operator=(const CCapAMSC& other);

	BYTE IsAMSC64RateSupported(BYTE rate) const;
	BYTE GetMaxAMSC64Rate(void) const;
	BYTE IsValidAMSC64Rate(BYTE rate) const;
	BYTE GetGreatestAMSC64RateLessThen(BYTE AMSC64Rate) const;

protected :
	BYTE	m_AMSC_MUX;
	BYTE	m_AMSC_MUX64_RateByte1;
	BYTE	m_AMSC_MUX64_RateByte2;

protected :
	// Internal use
	void SetAllRatesForRateByte1(void);
};

//------------------------------------------------
class CCapPCProfile : public CPObject
{
CLASS_TYPE_1(CCapPCProfile, CPObject)
public :
	CCapPCProfile();
	CCapPCProfile(const CCapPCProfile& other);
    virtual ~CCapPCProfile();

	virtual const char*  NameOf() const;
	void Dump(std::ostream& ostr) const;

	void SetProfile(BYTE profileNumber);
	BYTE IsProfileSupported(BYTE profileNumber);

	void  Serialize(WORD format,CSegment &seg);
	void  DeSerialize(WORD format,CSegment &seg);

protected :
	DWORD	m_profileListMask;
};

//------------------------------------------------

class CVideoStreamCap : public CPObject //will be inherited by CVideoStreamPPXC and CCapH239ExtendedVideo for EPC and H239
{
CLASS_TYPE_1(CVideoStreamCap,CPObject )
public :
	CVideoStreamCap();
	CVideoStreamCap(const CVideoStreamCap &other);
    virtual ~CVideoStreamCap();

	virtual const char* NameOf() const;
	virtual void  Dump(std::ostream& ostr) const;

	virtual void  Serialize(WORD format, CSegment &seg) = 0;
	virtual void  DeSerialize(WORD format, CSegment &seg, BYTE dataLen=0) = 0;

	CVideoStreamCap& operator=(const CVideoStreamCap& other);

protected :
	void		DeSerializeEmbededVideoCaps(BYTE dataLen, BYTE readBytesNum, CSegment &seg);
	void		CreateVideoCaps();
	BYTE*		m_pLabelList;
	BYTE		m_labelListSize;
	CCapH263	m_h263cap;
	CCapH261	m_h261cap;
};

//------------------------------------------------
class CVideoStreamPPXC : public CVideoStreamCap
{
CLASS_TYPE_1(CVideoStreamPPXC, CVideoStreamCap)
public :
	CVideoStreamPPXC() {}
    virtual ~CVideoStreamPPXC() {}

	virtual const char* NameOf() const;

	virtual void  Serialize(WORD format, CSegment &seg);
	virtual void  DeSerialize(WORD format, CSegment &seg, BYTE dataLen=0);

	void CreateContentVideoCapEPC(void);
	void AddLabel(BYTE newLabel);

protected :
};

//------------------------------------------------
class CCapPPXC : public CPObject
{
CLASS_TYPE_1(CCapPPXC, CPObject)
public :
	CCapPPXC();
    CCapPPXC(const CCapPPXC &other);
    virtual ~CCapPPXC();

	virtual const char*  NameOf() const;
	void Dump(std::ostream& ostr) const;

	void  Serialize(WORD format,CSegment &seg);
	void  DeSerialize(WORD format,CSegment &seg);

	CCapPPXC& operator=(const CCapPPXC &other);

	void CreateContentVideoCapEPC(void);

protected :
	BYTE				m_numOfItemsVideoStreamPPXC;
	CVideoStreamPPXC*	m_pVideoStreamPPXC;

	// Internal use
	void AddVideoStreamPPXCItem(CVideoStreamPPXC* pVideoStreamPPXC);
};

//------------------------------------------------

class CCapPP : public CPObject
{
CLASS_TYPE_1(CCapPP, CPObject)
public :
    CCapPP();
    CCapPP(const CCapPP& other);
	virtual ~CCapPP();

	void CreateCapEPC(WORD callRate, WORD isTranscoding,WORD isLSD,WORD ContentLevel);
	
	virtual const char*  NameOf() const;
	void Dump(std::ostream& ostr) const;

	void  Serialize(WORD format,CSegment &seg);
	void  DeSerialize(WORD format,CSegment &seg);

	CCapPP& operator=(const CCapPP& other);
	BYTE IsCapEPC(void);
	BYTE GetMaxAMSC64Rate(void) {return m_capAMSC.GetMaxAMSC64Rate();}
	BYTE IsAMSCRateSupported(BYTE rate) const { return m_capAMSC.IsAMSC64RateSupported(rate); }
	BYTE GetGreatestAMSC64RateLessThen(BYTE AMSC64Rate) const { return m_capAMSC.GetGreatestAMSC64RateLessThen(AMSC64Rate); }


protected :
	CCapPCProfile	m_capPCProfile;
	CCapAMSC		m_capAMSC;
	CCapPPXC		m_capPPXC;
	BYTE			m_capMediaFlowControlH320;
	BYTE			m_capPPh221EscapeTable;
};






#endif /* _H320CAPPP_H  */



