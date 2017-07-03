//+========================================================================+
//                            H320VideoCaps.H                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H320VideoCaps.H                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 01.10.07   |                                                      |
//+========================================================================+


#ifndef _H320VIDEOCAPS_H
#define _H320VIDEOCAPS_H


#include "PObject.h"
#include "H263Cap.h"
#include "H264Cap.h"


class CSegment;


/* This is capability description for H261 video coding standard */
class CCapH261 : public CPObject
{
CLASS_TYPE_1(CCapH261,CPObject )
public:
    CCapH261();
    CCapH261(const CCapH261& other);

    virtual const char* NameOf() const;
	virtual void  Dump(std::ostream&) const;

    void Serialize(WORD format,CSegment& stringSeg);
    void DeSerialize(WORD format,CSegment& stringSeg);

    void CreateDefault();
	void SetCaps(WORD resolution,WORD mpi);
    void SetVideoCapQcif(CSegment &seg);
    void SetVideoCapCif(CSegment &seg);
	void ResetCaps();
	WORD GetH261CapMpi(WORD cap) const;
	BOOL IsH261VideoCap(WORD cap) const;

private:

	const char* GetResolutionName()const;  //used for Dump
	const char* GetMpiName(BYTE mpi)const; //used for Dump


	DWORD   m_dataVidCap;       // video capabilities
	BYTE    m_cifMpi;           // cif min pic interval
	BYTE    m_qCifMpi;          // qcif min pic interval
};


/* This class describes video capabilities */
class CVideoCap : public CPObject
{
CLASS_TYPE_1(CVideoCap, CPObject)
public:
    CVideoCap();
    CVideoCap(const CVideoCap& other);

    virtual const char* NameOf() const;
	virtual void Dump(std::ostream&) const;

    void  Serialize(WORD format, CSegment& stringSeg);
    void  DeSerialize(WORD format, CSegment& stringSeg);

 	CVideoCap& operator= (const CVideoCap& other);

	void  CreateDefault();
	void  ResetVideoCaps();
    void  SetVideoCapCif(WORD mpi, WORD mpiQcif);
	void  SetVideoCapQcif(WORD mpiQcif);
    void  SetVideoCapQcif(CSegment& seg);
    void  SetVideoCapCif(CSegment& seg);
	void  SetVideoMBECap(BYTE cap);
	void  SetVideoH263Cap(BYTE cap);
	void  RemoveVideoMBECap();
	WORD  GetH261CapMpi(WORD cap) const;
	BOOL  IsMBECap(BYTE cap)const;
	BOOL  IsH261VideoCap(WORD cap) const;
	WORD  IsH263() const;
	WORD  IsH264() const;
	CCapH263* GetCapH263();
	CCapH264* GetCapH264();
	void  InsertH263CapSet(CCapSetH263* pH263CapSetBuf);
	void  InsertH264CapSet(CCapSetH264* pH263CapSetBuf);
	void  CreateH263Cap(CSegment& seg, BYTE mbeLen);
	void  CreateH264Cap(CSegment& seg, DWORD len);
	void  SetOneH263Cap(BYTE format, int mpi);
    void  RemoveH263Caps() { m_h263cap.Remove(); }
    void  RemoveH264Caps() { m_h264cap.Remove(); }
	void  SendSecondAdditionalCap(WORD OnOff);
    BOOL  IsVideoCapSupported(WORD cap)const;

	BYTE  IsCapableOfVideo() const;
	BYTE  IsCapableOfHD720_15() const;
	BYTE  IsCapableOfHD1080_15() const;
	BYTE  IsCapableOfHD720_50() const;
	eVideoPartyType GetCPVideoPartyTypeAccordingToCapabilities(BYTE isH2634Cif15Supported);

	eVideoPartyType GetCPVideoPartyTypeAccordingToLocalCapabilities(BYTE isH2634Cif15Supported);
    void RemoveH261Caps();

private:
	CCapH261 m_h261cap;
	BYTE     m_isMBE;

	CCapH263 m_h263cap;         // caps for H263 video
	CCapH264 m_h264cap;         // caps for H264 video
};



#endif /* _H320VIDEOCAPS_H  */



