/*$Header: /MCMS/MAIN/subsys/mcms/NATIVE.H 3     30/04/01 16:39 Matvey $*/
//+========================================================================+
//                            NATIVE.H                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       NATIVE.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 8/15/95     |                                                     |
//+========================================================================+

#ifndef _NATIVE
#define _NATIVE

#include "StructTm.h"
#include "CDRShort.h"
#include "CDRDetal.h"

class CSegment;

class CStructTmDrv : public CStructTm
{
CLASS_TYPE_1(CStructTmDrv, CStructTm)
public:
	void DeSerialize(WORD format, CSegment& seg);
	void Serialize(WORD format, CSegment& seg);
};
class CCdrShortDrv : public CCdrShort
{
CLASS_TYPE_1(CCdrShortDrv, CCdrShort)
public:
	void DeSerialize(WORD format, CSegment& seg);
	void Serialize(WORD format, CSegment& seg);
};

class CCdrEventDrv : public CCdrEvent
{
CLASS_TYPE_1(CCdrEventDrv, CCdrEvent)
public:
	void DeSerialize(WORD format, CSegment& seg);
	void Serialize(WORD format, CSegment& seg);
};

#endif // _NATIVE
