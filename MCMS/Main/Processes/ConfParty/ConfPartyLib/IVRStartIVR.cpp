//+========================================================================+
//            Copyright 2005 Polycom Networking Ltd.                       |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Networking Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IVRStartIVR.cpp	                                   |
//+========================================================================+


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include "IVRStartIVR.h"

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
CIVRStartIVR::~CIVRStartIVR()
{
}


void CIVRStartIVR::Serialize( CSegment* seg )
{
	seg->Put((BYTE*)&m_startIVR, sizeof(SIVRStartIVRStruct));
}


void CIVRStartIVR::DeSerialize( CSegment* seg)
{
}

