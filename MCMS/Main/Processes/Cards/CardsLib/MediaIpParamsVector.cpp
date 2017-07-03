//+========================================================================+
//                     MediaIpParameters.cpp                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MediaIpParameters.cpp	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Haggai                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+


#include "MediaIpParamsVector.h"
#include "StatusesGeneral.h"



/////////////////////////////////////////////////////////////////////////////
CMediaIpParamsVector::CMediaIpParamsVector()
{
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpParamsVector::~CMediaIpParamsVector()
{
	ClearAndDestroy();
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpParamsVector::CMediaIpParamsVector (const CMediaIpParamsVector& rMediaIpParamsVector)
    :CPObject(rMediaIpParamsVector),
    m_mediaIpParamsVector(rMediaIpParamsVector.m_mediaIpParamsVector)
{
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpParamsVector& CMediaIpParamsVector::operator= (const CMediaIpParamsVector& rOther)
{
	if (&rOther == this ) 
		return *this;

	m_mediaIpParamsVector = rOther.m_mediaIpParamsVector;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CMediaIpParamsVector::ClearAndDestroy(void)
{
	CMediaIpParameters* pErasedMediaIpParams = NULL;
	
	MEDIA_IP_PARAMS_VECTOR::iterator itr =  m_mediaIpParamsVector.begin();

	while (itr != m_mediaIpParamsVector.end())
	{
		pErasedMediaIpParams = (*itr);
		m_mediaIpParamsVector.erase(itr);
		POBJDELETE(pErasedMediaIpParams);
		itr =  m_mediaIpParamsVector.begin();
	}
}

///////////////////////////////////////////////////////////////////////////////
DWORD CMediaIpParamsVector::Size()
{
	return m_mediaIpParamsVector.size();
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpParameters* CMediaIpParamsVector::Find(CMediaIpParameters*  pMediaIpParams)
{

	MEDIA_IP_PARAMS_VECTOR::iterator itr =  FindPosition(pMediaIpParams);

	if ( itr != m_mediaIpParamsVector.end() )
		return (*itr);

	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMediaIpParamsVector::Insert(CMediaIpParameters* pMediaIpParams)
{
	if ( NULL == pMediaIpParams )
		return STATUS_FAIL;


	m_mediaIpParamsVector.push_back(pMediaIpParams);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpParameters* CMediaIpParamsVector::Remove(CMediaIpParameters* pMediaIpParams)
{
	if ( NULL == pMediaIpParams )
		return pMediaIpParams;


	CMediaIpParameters* pErasedMediaIpParams = NULL;
	
	MEDIA_IP_PARAMS_VECTOR::iterator itr = FindPosition(pMediaIpParams);

	if ( itr != m_mediaIpParamsVector.end() )
	{
		pErasedMediaIpParams = (*itr);
		m_mediaIpParamsVector.erase(itr);
	}
	
	return pErasedMediaIpParams;
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpParameters* CMediaIpParamsVector::Remove(DWORD serviceId)
{
	CMediaIpParameters* pErasedMediaIpParams = NULL;
	
	MEDIA_IP_PARAMS_VECTOR::iterator itr = FindPosition(serviceId);

	if ( itr != m_mediaIpParamsVector.end() )
	{
		pErasedMediaIpParams = (*itr);
		m_mediaIpParamsVector.erase(itr);
	}
	
	return pErasedMediaIpParams;
}

///////////////////////////////////////////////////////////////////////////////
CMediaIpParameters* CMediaIpParamsVector::At(DWORD index)
{
	if( index < m_mediaIpParamsVector.size() )
		return (m_mediaIpParamsVector[index]);

	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpParameters* CMediaIpParamsVector::GetFirst()
{
	m_mediaIpParamsVectorIterator = m_mediaIpParamsVector.begin();

	if(m_mediaIpParamsVectorIterator != m_mediaIpParamsVector.end())
		return (*m_mediaIpParamsVectorIterator);

	else
		return NULL;

}

/////////////////////////////////////////////////////////////////////////////
CMediaIpParameters* CMediaIpParamsVector::GetNext()
{
	//make sure not to increase iterator if it's already at end
	if(m_mediaIpParamsVectorIterator == m_mediaIpParamsVector.end())
		return NULL;

	m_mediaIpParamsVectorIterator++;
	
	if(m_mediaIpParamsVectorIterator != m_mediaIpParamsVector.end())
		return (*m_mediaIpParamsVectorIterator);

	else
		return NULL;
}

///////////////////////////////////////////////////////////////////////////////
MEDIA_IP_PARAMS_VECTOR::iterator CMediaIpParamsVector::FindPosition
                                               (const CMediaIpParameters* pMediaIpParams)
{
	MEDIA_IP_PARAMS_VECTOR::iterator itr =  m_mediaIpParamsVector.begin();

	while (itr != m_mediaIpParamsVector.end())
	{
		if ( *(*itr) == *pMediaIpParams ) 
			return itr;

		itr++;
	}

	return itr;
}

///////////////////////////////////////////////////////////////////////////////
MEDIA_IP_PARAMS_VECTOR::iterator CMediaIpParamsVector::FindPosition(const DWORD serviceId)
{
	MEDIA_IP_PARAMS_VECTOR::iterator itr =  m_mediaIpParamsVector.begin();

	while (itr != m_mediaIpParamsVector.end())
	{
		if ( (*(*itr)).GetServiceId() == serviceId ) 
			return itr;

		itr++;
	}

	return itr;
}
