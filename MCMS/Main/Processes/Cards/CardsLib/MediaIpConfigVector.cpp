//+========================================================================+
//                     MediaIpConfigVector.cpp                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MediaIpConfigVector.cpp	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Haggai                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+


#include "MediaIpConfigVector.h"
#include "StatusesGeneral.h"



/////////////////////////////////////////////////////////////////////////////
CMediaIpConfigVector::CMediaIpConfigVector()
{
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpConfigVector::~CMediaIpConfigVector()
{
	ClearAndDestroy();
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpConfigVector::CMediaIpConfigVector (const CMediaIpConfigVector& rMediaIpConfigVector)
    :CPObject(rMediaIpConfigVector),
    m_mediaIpConfigVector(rMediaIpConfigVector.m_mediaIpConfigVector)
{
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpConfigVector& CMediaIpConfigVector::operator= (const CMediaIpConfigVector& rOther)
{
	if (&rOther == this ) 
		return *this;

	m_mediaIpConfigVector = rOther.m_mediaIpConfigVector;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CMediaIpConfigVector::ClearAndDestroy(void)
{
	CMediaIpConfig* pErasedMediaIpConfig = NULL;
	
	MEDIA_IP_CONFIG_VECTOR::iterator itr =  m_mediaIpConfigVector.begin();

	while (itr != m_mediaIpConfigVector.end())
	{
		pErasedMediaIpConfig = (*itr);
		m_mediaIpConfigVector.erase(itr);
		POBJDELETE(pErasedMediaIpConfig);
		itr =  m_mediaIpConfigVector.begin();
	}
}

///////////////////////////////////////////////////////////////////////////////
DWORD CMediaIpConfigVector::Size()
{
	return m_mediaIpConfigVector.size();
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpConfig* CMediaIpConfigVector::Find(CMediaIpConfig*  pMediaIpConfig)
{

	MEDIA_IP_CONFIG_VECTOR::iterator itr =  FindPosition(pMediaIpConfig);

	if ( itr != m_mediaIpConfigVector.end() )
		return (*itr);

	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMediaIpConfigVector::Insert(CMediaIpConfig* pMediaIpConfig)
{
	if ( NULL == pMediaIpConfig )
		return STATUS_FAIL;


	m_mediaIpConfigVector.push_back(pMediaIpConfig);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpConfig* CMediaIpConfigVector::Remove(CMediaIpConfig* pMediaIpConfig)
{
	if ( NULL == pMediaIpConfig )
		return pMediaIpConfig;


	CMediaIpConfig* pErasedMediaIpConfig = NULL;
	
	MEDIA_IP_CONFIG_VECTOR::iterator itr = FindPosition(pMediaIpConfig);

	if ( itr != m_mediaIpConfigVector.end() )
	{
		pErasedMediaIpConfig = (*itr);
		m_mediaIpConfigVector.erase(itr);
	}
	
	return pErasedMediaIpConfig;
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpConfig* CMediaIpConfigVector::Remove(DWORD serviceId)
{
	CMediaIpConfig* pErasedMediaIpConfig = NULL;
	
	MEDIA_IP_CONFIG_VECTOR::iterator itr = FindPosition(serviceId);

	if ( itr != m_mediaIpConfigVector.end() )
	{
		pErasedMediaIpConfig = (*itr);
		m_mediaIpConfigVector.erase(itr);
	}
	
	return pErasedMediaIpConfig;
}

///////////////////////////////////////////////////////////////////////////////
CMediaIpConfig* CMediaIpConfigVector::At(DWORD index)
{
	if( index < m_mediaIpConfigVector.size() )
		return (m_mediaIpConfigVector[index]);

	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CMediaIpConfig* CMediaIpConfigVector::GetFirst()
{
	m_mediaIpConfigVectorIterator = m_mediaIpConfigVector.begin();

	if(m_mediaIpConfigVectorIterator != m_mediaIpConfigVector.end())
		return (*m_mediaIpConfigVectorIterator);

	else
		return NULL;

}

/////////////////////////////////////////////////////////////////////////////
CMediaIpConfig* CMediaIpConfigVector::GetNext()
{
	//make sure not to increase iterator if it's already at end
	if(m_mediaIpConfigVectorIterator == m_mediaIpConfigVector.end())
		return NULL;

	m_mediaIpConfigVectorIterator++;
	
	if(m_mediaIpConfigVectorIterator != m_mediaIpConfigVector.end())
		return (*m_mediaIpConfigVectorIterator);

	else
		return NULL;
}

///////////////////////////////////////////////////////////////////////////////
MEDIA_IP_CONFIG_VECTOR::iterator CMediaIpConfigVector::FindPosition
                                               (const CMediaIpConfig* pMediaIpConfig)
{
	MEDIA_IP_CONFIG_VECTOR::iterator itr =  m_mediaIpConfigVector.begin();

	while (itr != m_mediaIpConfigVector.end())
	{
		if ( *(*itr) == *pMediaIpConfig ) 
			return itr;

		itr++;
	}

	return itr;
}

///////////////////////////////////////////////////////////////////////////////
MEDIA_IP_CONFIG_VECTOR::iterator CMediaIpConfigVector::FindPosition(const DWORD serviceId)
{
	MEDIA_IP_CONFIG_VECTOR::iterator itr =  m_mediaIpConfigVector.begin();

	while (itr != m_mediaIpConfigVector.end())
	{
		if ( (*(*itr)).GetServiceId() == serviceId ) 
			return itr;

		itr++;
	}

	return itr;
}
