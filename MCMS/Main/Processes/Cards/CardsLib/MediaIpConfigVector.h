//+========================================================================+
//                     MediaIpConfigVector.h                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MediaIpConfigVector.h	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Haggai                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | August-2005  |                                                    |
//+========================================================================+

#ifndef _MediaIpConfigVector_H__
#define _MediaIpConfigVector_H__


#include <vector>
#include "PObject.h"
#include "MediaIpConfig.h"


typedef std::vector< CMediaIpConfig * > MEDIA_IP_CONFIG_VECTOR;

class CMediaIpConfigVector : public CPObject
{
CLASS_TYPE_1(CMediaIpConfigVector,CPObject)
public: 
	
	// Constructors
	CMediaIpConfigVector();
	virtual const char* NameOf() const { return "CMediaIpConfigVector";}
	virtual ~CMediaIpConfigVector(); 
	CMediaIpConfigVector (const CMediaIpConfigVector& rMediaIpConfigVector);

	// Overloaded operators
	CMediaIpConfigVector&	operator= (const CMediaIpConfigVector& rOther);
	
	// Operations   

	void             ClearAndDestroy(void);
	DWORD            Size(void);

	CMediaIpConfig*  Find(CMediaIpConfig* pMediaIpConfig);      
	STATUS           Insert(CMediaIpConfig* pMediaIpConfig); 
	CMediaIpConfig*  Remove(CMediaIpConfig* pMediaIpConfig);
	CMediaIpConfig*  Remove(DWORD serviceId);

	CMediaIpConfig*  At(DWORD index);
	CMediaIpConfig*	 GetFirst();
	CMediaIpConfig*	 GetNext();

	
private:	
	// Internal use operations             
	MEDIA_IP_CONFIG_VECTOR::iterator FindPosition(const CMediaIpConfig* pMediaIpConfig);
	MEDIA_IP_CONFIG_VECTOR::iterator FindPosition(const DWORD serviceId);

	MEDIA_IP_CONFIG_VECTOR		     m_mediaIpConfigVector;
	MEDIA_IP_CONFIG_VECTOR::iterator m_mediaIpConfigVectorIterator; //for GetFirst and GetNext functions
};


#endif /* _MediaIpConfigVector_H__ */
