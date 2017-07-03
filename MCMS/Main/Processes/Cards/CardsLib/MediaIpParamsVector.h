//+========================================================================+
//                     MediaIpParamsVector.h                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MediaIpParamsVector.h	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Haggai                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | August-2005  |                                                    |
//+========================================================================+

#ifndef _MediaIpParamsVector_H__
#define _MediaIpParamsVector_H__


#include <vector>
#include "PObject.h"
#include "MediaIpParameters.h"


typedef std::vector< CMediaIpParameters * > MEDIA_IP_PARAMS_VECTOR;

class CMediaIpParamsVector : public CPObject
{
CLASS_TYPE_1(CMediaIpParamsVector,CPObject)
public: 
	
	// Constructors
	CMediaIpParamsVector();
	virtual const char* NameOf() const { return "CMediaIpParamsVector";}
	virtual ~CMediaIpParamsVector(); 
	CMediaIpParamsVector (const CMediaIpParamsVector& rMediaIpParamsVector);

	// Overloaded operators
	CMediaIpParamsVector&	operator= (const CMediaIpParamsVector& rOther);
	
	// Operations   

	void                 ClearAndDestroy(void);
	DWORD                Size(void);

	CMediaIpParameters*  Find(CMediaIpParameters* pMediaIpParams);      
	STATUS               Insert(CMediaIpParameters* pMediaIpParams); 
	CMediaIpParameters*  Remove(CMediaIpParameters* pMediaIpParams); 
	CMediaIpParameters*  Remove(DWORD serviceId);

	CMediaIpParameters*  At(DWORD index);
	CMediaIpParameters*	 GetFirst();
	CMediaIpParameters*	 GetNext();

	
private:	
	// Internal use operations             
	MEDIA_IP_PARAMS_VECTOR::iterator FindPosition(const CMediaIpParameters* pMediaIpParams);
	MEDIA_IP_PARAMS_VECTOR::iterator FindPosition(const DWORD serviceId);

	MEDIA_IP_PARAMS_VECTOR		     m_mediaIpParamsVector;
	MEDIA_IP_PARAMS_VECTOR::iterator m_mediaIpParamsVectorIterator; //for GetFirst and GetNext functions
};


#endif /* _MediaIpParamsVector_H__ */
