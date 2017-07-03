//+========================================================================+
//                         BridgeMoveParams.CPP                        	   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgeMoveParams.CPP                                    	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Talya                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  November-2005  | Description                               |
//-------------------------------------------------------------------------|
//			
//+========================================================================+



#include "BridgeMoveParams.h"


///////////////////////////////////////////////////////////////////////////////////////
CBridgeMoveParams::CBridgeMoveParams()
{
	m_pAudioBridgePartyCntl = NULL;
	m_pVideoBridgePartyCntl = NULL;
}
/////////////////////////////////////////////////////////////////////////////
CBridgeMoveParams::CBridgeMoveParams(const CBridgeMoveParams &other)
:CPObject(other)
{
	m_pAudioBridgePartyCntl = other.m_pAudioBridgePartyCntl;
	m_pVideoBridgePartyCntl = other.m_pVideoBridgePartyCntl;
}
/////////////////////////////////////////////////////////////////////////////
CBridgeMoveParams& CBridgeMoveParams::operator = (const CBridgeMoveParams &other)
{
	if ( &other == this ) return *this;
	
	m_pAudioBridgePartyCntl = other.m_pAudioBridgePartyCntl;
	m_pVideoBridgePartyCntl = other.m_pVideoBridgePartyCntl;
	
	return *this;
}
///////////////////////////////////////////////////////////////////////////////////////
CBridgeMoveParams::~CBridgeMoveParams()
{
	
}
//////////////////////////////////////////////////////////////////////////////
void   CBridgeMoveParams::SetAudioBridgePartyCntlOnExport(CAudioBridgePartyCntl* pAudioBridgePartyCntl)
{
	m_pAudioBridgePartyCntl = pAudioBridgePartyCntl;
}
//////////////////////////////////////////////////////////////////////////////
void   CBridgeMoveParams::SetVideoBridgePartyCntlOnExport(CVideoBridgePartyCntl* pVideoBridgePartyCntl)
{
	m_pVideoBridgePartyCntl = pVideoBridgePartyCntl;
}
//////////////////////////////////////////////////////////////////////////////
CAudioBridgePartyCntl* CBridgeMoveParams::GetAndResetAudioBridgePartyCntlOnImport()
{
	CAudioBridgePartyCntl* pAudioBridgePartyCntl = m_pAudioBridgePartyCntl;
	m_pAudioBridgePartyCntl = NULL;
	return pAudioBridgePartyCntl;
}
//////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntl* CBridgeMoveParams::GetAndResetVideoBridgePartyCntlOnImport()
{
	CVideoBridgePartyCntl* pVideoBridgePartyCntl = m_pVideoBridgePartyCntl;
	m_pVideoBridgePartyCntl = NULL;
	return pVideoBridgePartyCntl;
}

//////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntl* CBridgeMoveParams::UnregisterBridgePartyCntlsInTask()
{
    if (m_pVideoBridgePartyCntl)
        m_pVideoBridgePartyCntl->UnregisterInTask();

    if (m_pAudioBridgePartyCntl)
        m_pAudioBridgePartyCntl->UnregisterInTask();
        
    return NULL;    
}

//////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntl* CBridgeMoveParams::RegisterBridgePartyCntlsInTask(CTaskApp * pTask)
{
    if (m_pVideoBridgePartyCntl)
        m_pVideoBridgePartyCntl->RegisterInTask();

    if (m_pAudioBridgePartyCntl)
        m_pAudioBridgePartyCntl->RegisterInTask();
        
     return NULL;   
        
}


