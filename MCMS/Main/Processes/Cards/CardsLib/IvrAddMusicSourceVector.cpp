//+========================================================================+
//                     IvrAddMusicSourceVector.cpp                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IvrAddMusicSourceVector.cpp	                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Haggai                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+

#include "IvrAddMusicSourceVector.h"
#include "StatusesGeneral.h"



/////////////////////////////////////////////////////////////////////////////
CIvrAddMusicSourceVector::CIvrAddMusicSourceVector()
{
}

/////////////////////////////////////////////////////////////////////////////
CIvrAddMusicSourceVector::~CIvrAddMusicSourceVector()
{
	ClearAndDestroy();
}

/////////////////////////////////////////////////////////////////////////////
CIvrAddMusicSourceVector::CIvrAddMusicSourceVector (const CIvrAddMusicSourceVector& rIvrAddMusicSourceVector)
    :CPObject(rIvrAddMusicSourceVector),
    m_ivrAddMusicSourceVector(rIvrAddMusicSourceVector.m_ivrAddMusicSourceVector)
{
}

/////////////////////////////////////////////////////////////////////////////
CIvrAddMusicSourceVector& CIvrAddMusicSourceVector::operator= (const CIvrAddMusicSourceVector& rOther)
{
	if (&rOther == this ) 
		return *this;

	m_ivrAddMusicSourceVector = rOther.m_ivrAddMusicSourceVector;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CIvrAddMusicSourceVector::ClearAndDestroy(void)
{
	SIVRAddMusicSource* pErasedIvrAddMusicSource = NULL;
	
	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator itr =  m_ivrAddMusicSourceVector.begin();

	while (itr != m_ivrAddMusicSourceVector.end())
	{
		pErasedIvrAddMusicSource = (*itr);
		m_ivrAddMusicSourceVector.erase(itr);
		delete pErasedIvrAddMusicSource;

		itr =  m_ivrAddMusicSourceVector.begin();
	}
}

///////////////////////////////////////////////////////////////////////////////
DWORD CIvrAddMusicSourceVector::Size()
{
	return m_ivrAddMusicSourceVector.size();
}

/////////////////////////////////////////////////////////////////////////////
SIVRAddMusicSource* CIvrAddMusicSourceVector::Find(SIVRAddMusicSource*  pIvrAddMusicSource)
{

	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator itr =  FindPosition(pIvrAddMusicSource);

	if ( itr != m_ivrAddMusicSourceVector.end() )
		return (*itr);

	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////
SIVRAddMusicSource* CIvrAddMusicSourceVector::Find(DWORD srcId)
{
	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator itr =  FindPosition(srcId);

	if ( itr != m_ivrAddMusicSourceVector.end() )
		return (*itr);

	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIvrAddMusicSourceVector::Insert(SIVRAddMusicSource* pIvrAddMusicSource)
{
	if ( NULL == pIvrAddMusicSource )
		return STATUS_FAIL;


	m_ivrAddMusicSourceVector.push_back(pIvrAddMusicSource);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
SIVRAddMusicSource* CIvrAddMusicSourceVector::Remove(SIVRAddMusicSource* pIvrAddMusicSource)
{
	if ( NULL == pIvrAddMusicSource )
		return pIvrAddMusicSource;


	SIVRAddMusicSource* pErasedIvrAddMusicSource = NULL;
	
	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator itr = FindPosition(pIvrAddMusicSource);

	if ( itr != m_ivrAddMusicSourceVector.end() )
	{
		pErasedIvrAddMusicSource = (*itr);
		m_ivrAddMusicSourceVector.erase(itr);
	}
	
	return pErasedIvrAddMusicSource;
}

/////////////////////////////////////////////////////////////////////////////
SIVRAddMusicSource* CIvrAddMusicSourceVector::Remove(DWORD srcId)
{
	SIVRAddMusicSource* pErasedIvrAddMusicSource = NULL;
	
	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator itr = FindPosition(srcId);

	if ( itr != m_ivrAddMusicSourceVector.end() )
	{
		pErasedIvrAddMusicSource = (*itr);
		m_ivrAddMusicSourceVector.erase(itr);
	}
	
	return pErasedIvrAddMusicSource;
}

///////////////////////////////////////////////////////////////////////////////
SIVRAddMusicSource* CIvrAddMusicSourceVector::At(DWORD index)
{
	if( index < m_ivrAddMusicSourceVector.size() )
		return (m_ivrAddMusicSourceVector[index]);

	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////
SIVRAddMusicSource* CIvrAddMusicSourceVector::GetFirst()
{
	m_ivrAddMusicSourceVectorIterator = m_ivrAddMusicSourceVector.begin();

	if(m_ivrAddMusicSourceVectorIterator != m_ivrAddMusicSourceVector.end())
		return (*m_ivrAddMusicSourceVectorIterator);

	else
		return NULL;

}

/////////////////////////////////////////////////////////////////////////////
SIVRAddMusicSource* CIvrAddMusicSourceVector::GetNext()
{
	//make sure not to increase iterator if it's already at end
	if(m_ivrAddMusicSourceVectorIterator == m_ivrAddMusicSourceVector.end())
		return NULL;

	m_ivrAddMusicSourceVectorIterator++;
	
	if(m_ivrAddMusicSourceVectorIterator != m_ivrAddMusicSourceVector.end())
		return (*m_ivrAddMusicSourceVectorIterator);

	else
		return NULL;
}

///////////////////////////////////////////////////////////////////////////////
IVR_ADD_MUSIC_SOURCE_VECTOR::iterator CIvrAddMusicSourceVector::FindPosition
                                               (const SIVRAddMusicSource* pIvrAddMusicSource)
{
	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator itr =  m_ivrAddMusicSourceVector.begin();

	while (itr != m_ivrAddMusicSourceVector.end())
	{
		if ( (*(*itr)).sourceID == pIvrAddMusicSource->sourceID ) 
			return itr;

		itr++;
	}

	return itr;
}

///////////////////////////////////////////////////////////////////////////////
IVR_ADD_MUSIC_SOURCE_VECTOR::iterator CIvrAddMusicSourceVector::FindPosition(const DWORD srcId)
{
	IVR_ADD_MUSIC_SOURCE_VECTOR::iterator itr =  m_ivrAddMusicSourceVector.begin();

	while (itr != m_ivrAddMusicSourceVector.end())
	{
		if ( (*(*itr)).sourceID == srcId ) 
			return itr;

		itr++;
	}

	return itr;
}
