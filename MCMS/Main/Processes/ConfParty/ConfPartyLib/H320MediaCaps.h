//+========================================================================+
//                            H320MediaCaps.H                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H320MediaCaps.H                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 01.10.07   |                                                      |
//+========================================================================+


#ifndef _H320MEDIACAPS_H 
#define _H320MEDIACAPS_H


#include "PObject.h"
#include "H263Cap.h"
#include "H320CapPP.h"
#include "H239Defines.h"

/*
const BYTE RoleLabelParamIdent					= 0x01;
const BYTE LiveRoleLabel						= 0x02;
const BYTE PresentationRoleLabel				= 0x01;


///////////////H239
const BYTE RoleLabelParamIdent					= 0x01;
const BYTE BitRateParamIdent					= 0x29;
const BYTE ChannelIdParamIdent					= 0x2A;
const BYTE SymmetryBreakingParamIdent			= 0x2B;
const BYTE TerminalLabelParamIdent				= 0x2C;
const BYTE AcknowledgeParamIdent				= 0x7E;
const BYTE RejectParamIdent						= 0x7F;

const BYTE PresentationRoleLabel				= 0x01;
const BYTE LiveRoleLabel						= 0x02;

const BYTE AMC_CAP_LEN							= 0x03;

//H.239-message Sub Message Identifiers
const BYTE FlowControlReleaseRequest			= 0x01;
const BYTE FlowControlReleaseResponse			= 0x02;
const BYTE PresentationTokenRequest				= 0x03;
const BYTE PresentationTokenResponse			= 0x04;
const BYTE PresentationTokenRelease				= 0x05;
const BYTE PresentationTokenIndicateOwner		= 0x06;

//H.239-message AMC Channel ID values
const BYTE MainVideoChannel						= 0x01;
const BYTE SecondVideoChannel					= 0x02;

*/

////////////////////////////////////////////////////////////////////////
////////////////////////// Data capabilities ///////////////////////////
////////////////////////////////////////////////////////////////////////

class CDataCap : public CPObject
{
CLASS_TYPE_1(CDataCap, CPObject)
public:
    CDataCap();
    CDataCap(const CDataCap& other);

    virtual const char* NameOf() const; 
	virtual void  Dump(std::ostream&) const;

    void  Serialize(WORD format, CSegment& stringSeg);
    void  DeSerialize(WORD format, CSegment& stringSeg);

    void  CreateDefault();
	void  ResetDataCaps();
	void  SetDataCap(WORD cap);
	void  RemoveDataCap(WORD cap);
    void  SetHsdHmlpCap(WORD cap) { m_hsdHmplCap = cap; }
    void  SetMlpCap(WORD cap) { m_mlpCap = cap; }
    void  SetHsdHmlpBas(WORD bas);
    void  SetMlpBas(WORD bas);

	WORD  OnDataCap(WORD cap) const;
    WORD  OnHsdHmlpCap(WORD cap) const;
    WORD  OnMlpCap(WORD cap) const;

	DWORD GetHsdHmlpBas() const { return m_hsdHmplCap; }
	DWORD GetMlpBas() const { return m_mlpCap; }

private:
	DWORD m_dataCap;          // data capabilities
	DWORD m_hsdHmplCap;       // hsd + mlp capabilities
	DWORD m_mlpCap;           // mlp  capabilities
};


////////////////////////////////////////////////////////////////////////
/////////////////////// Content capabilities ///////////////////////////
////////////////////////////////////////////////////////////////////////

class CCapAMC : public CPObject
{
CLASS_TYPE_1(CCapAMC, CPObject)
public:
	CCapAMC();
    CCapAMC(const CCapAMC &other);
    virtual ~CCapAMC();
	virtual const char*  NameOf() const {return "CCapAMC";}	
	void  Dump(std::ostream& ostr) const;
	void  Serialize(WORD format, CSegment &seg);
	void  DeSerialize(WORD format, CSegment &seg);
	CCapAMC& operator=(const CCapAMC &other);
	void SetRateAMC(BYTE rate);
	void SetAllAMCRatesWeSupportUpTo(BYTE rate);
	BYTE IsAMCRateSupported (BYTE rate) const;
	BYTE GetMaxContentRate(void) const;
	BYTE GetGreatestAMCRateLessThen(BYTE AMCRate) const;

protected:
	BYTE			m_optionByte1;
	BYTE			m_optionByte2;
};


class CCapH239ExtendedVideo : public CVideoStreamCap
{
CLASS_TYPE_1(CCapH239ExtendedVideo,CVideoStreamCap )
public:
	CCapH239ExtendedVideo(){};
	virtual ~CCapH239ExtendedVideo(){};
	virtual const char*  NameOf() const {return "CCapH239ExtendedVideo";}
	void  Serialize(WORD format,CSegment &seg);
	void  DeSerialize(WORD format,CSegment &seg, BYTE dataLen = 0);
	void  CreateExtendedVideoCapH239(void);

protected:
	void DeserializeRoleLabelByteToList(BYTE roleLabelByte);
	BYTE SerializeRoleLabelListToByte();
};


class CCapH239 : public CPObject
{
CLASS_TYPE_1(CCapH239, CPObject)
public:
	CCapH239();
    CCapH239(const CCapH239 &other);
    virtual ~CCapH239();
	CCapH239& operator=(const CCapH239 &other);
	
	virtual const char*  NameOf() const {return "CCapH239";}	
	void  Dump(std::ostream&) const;
	void  Serialize(WORD format,CSegment &seg);
	void  DeSerialize(WORD format,CSegment &seg);
	
	void  CreateCapH239(WORD callRate, WORD isTranscoding,WORD isLSD,WORD ContentLevel);

	void  SetAMCCaps(CSegment &seg,BYTE len);
	void  SetH239ExtendedVideoCaps(CSegment &seg,BYTE len);
	void  SetOnlyExtendedVideoCaps(const CCapH239 &other);
	
	BYTE  GetMaxContentRate(void);
	BYTE  IsAMCRateSupported(BYTE rate) const;
	BYTE  GetGreatestAMCRateLessThen(BYTE AMCRate) const;
	void  SerializeH239ExtendedVidCaps(WORD format,CSegment &seg);
	void  SerializeAmcCaps(WORD format,CSegment &seg);

protected:
	CCapAMC					m_capAMC;
	CCapH239ExtendedVideo	m_capH239ExtendedVideo;
};


class CContentCap : public CPObject
{
CLASS_TYPE_1(CContentCap,CPObject )
public:
    CContentCap();
    CContentCap(const CContentCap& other);
    ~CContentCap();

	virtual const char* NameOf() const {return "CContentCap";}	
	virtual void  Dump(std::ostream&) const;

    void  Serialize(WORD format, CSegment& stringSeg);
    void  DeSerialize(WORD format, CSegment& stringSeg);

	CContentCap&  operator= (const CContentCap& other);

	void  SetH239ExtendedVideoCaps(CSegment &seg,BYTE len);
    void  SetAMCCaps(CSegment &seg,BYTE len);
	void  RemoveH239Caps();
	void  SetOnlyExtendedVideoCaps(const CCapH239* pCapH239);

	const CCapH239* GetH239Caps() { return m_pCapH239; }
	BOOL  IsCapH239()const { return (m_pCapH239 ? TRUE : FALSE); }
	
	
private:
	CCapH239* m_pCapH239;
	
};



#endif /* _H320MEDIACAPS  */



