//+========================================================================+
//                        IVRStartIVR.H				                   |
//					      Copyright 2005 Polycom                           |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IVRStartIVR.H							                   |
// SUBSYSTEM:  MCMS                                                        |
//+========================================================================+

#ifndef __IVR_START_IVR_H__
#define __IVR_START_IVR_H__

#include "Segment.h"
#include "IvrApiStructures.h"


class CIVRStartIVR
{
public:
	~CIVRStartIVR();
	void Serialize( CSegment* seg );
	void DeSerialize( CSegment* seg );
public:
	SIVRStartIVRStruct	m_startIVR;
};


#endif	// __IVR_START_IVR_H__
