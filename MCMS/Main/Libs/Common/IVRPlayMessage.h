//+========================================================================+
//                        IVRPlayMessage.H				                   |
//					      Copyright 2005 Polycom                           |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IVRPlayMessage.H							                   |
// SUBSYSTEM:  MCMS                                                        |
//+========================================================================+

#ifndef __IVR_PLAY_MESSAGE_H__
#define __IVR_PLAY_MESSAGE_H__

#include "IvrApiStructures.h"

class CSegment;

class CIVRPlayMessage
{
public:
	CIVRPlayMessage();
	~CIVRPlayMessage();
	void Serialize( CSegment* seg );
	void DeSerialize( CSegment* seg );
public:
	SIVRPlayMessageStruct	play;
};


#endif	// __IVR_PLAY_MESSAGE_H__
