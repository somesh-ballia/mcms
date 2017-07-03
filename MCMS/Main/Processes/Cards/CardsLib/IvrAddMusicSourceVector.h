//+========================================================================+
//                     IvrAddMusicSourceVector.h                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IvrAddMusicSourceVector.h	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Haggai                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | August-2005  |                                                    |
//+========================================================================+

#ifndef _IvrAddMusicSourceVector_H__
#define _IvrAddMusicSourceVector_H__


#include <vector>
#include "PObject.h"
#include "IvrApiStructures.h"


typedef std::vector< SIVRAddMusicSource * > IVR_ADD_MUSIC_SOURCE_VECTOR;

class CIvrAddMusicSourceVector : public CPObject
{
CLASS_TYPE_1(CIvrAddMusicSourceVector,CPObject)
public: 
	
	// Constructors
	CIvrAddMusicSourceVector();
	virtual const char* NameOf() const { return "CIvrAddMusicSourceVector";}
	virtual ~CIvrAddMusicSourceVector(); 
	CIvrAddMusicSourceVector (const CIvrAddMusicSourceVector& rIvrAddMusicSourceVector);

	// Overloaded operators
	CIvrAddMusicSourceVector& operator= (const CIvrAddMusicSourceVector& rOther);
	
	// Operations   

	void   ClearAndDestroy(void);
	DWORD  Size(void);

	SIVRAddMusicSource*  Find(SIVRAddMusicSource* pIvrAddMusicSource);      
	SIVRAddMusicSource*  Find(DWORD srcId);      
	STATUS               Insert(SIVRAddMusicSource* pIvrAddMusicSource); 
	SIVRAddMusicSource*  Remove(SIVRAddMusicSource* pIvrAddMusicSource);
	SIVRAddMusicSource*  Remove(DWORD srcId);

	SIVRAddMusicSource*  At(DWORD index);
	SIVRAddMusicSource*	 GetFirst();
	SIVRAddMusicSource*	 GetNext();

	
private:	
	// Internal use operations             
	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator FindPosition(const SIVRAddMusicSource* pIvrAddMusicSource);
	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator FindPosition(const DWORD srcId);

	IVR_ADD_MUSIC_SOURCE_VECTOR		      m_ivrAddMusicSourceVector;
	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator m_ivrAddMusicSourceVectorIterator; //for GetFirst and GetNext functions
};


#endif /* _IvrAddMusicSrcVector_H__ */
