//+========================================================================+
//                   ContentBridgeInitParams.CPP                           |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       ContentBridgeInitParams.CPP                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  Nov-2008  | Description                                    |
//-------------------------------------------------------------------------|


#include "ContentBridgeInitParams.h"


// ------------------------------------------------------------
CContentBridgeInitParams::CContentBridgeInitParams()
{
	m_byCurrentContentProtocol = H264;
	m_byH264HighProfile = FALSE;  //HP content
}
// ------------------------------------------------------------
CContentBridgeInitParams::CContentBridgeInitParams (const CConf* pConf, const char*	pConfName,ConfRsrcID confRsrcId,
							const EBridgeImplementationTypes eBridgeImplementationType,BYTE ContentProtocol,BOOL iseExcludeContentMode, BYTE ContentHighProfile):
							CBridgeInitParams(pConf, pConfName,confRsrcId, eBridgeImplementationType),
							m_byCurrentContentProtocol(ContentProtocol), m_bIseExcludeContentMode(iseExcludeContentMode), m_byH264HighProfile(ContentHighProfile)  //HP content
{
}
// ------------------------------------------------------------
CContentBridgeInitParams::CContentBridgeInitParams (const CContentBridgeInitParams& rContentBridgeInitParams) :
	CBridgeInitParams(rContentBridgeInitParams)
{
	m_byCurrentContentProtocol = rContentBridgeInitParams.GetContentProtocol();
	m_byH264HighProfile = rContentBridgeInitParams.GetContentH264HighProfile();  //HP content
	SetIsExclusiveContentMode(rContentBridgeInitParams.GetIsExclusiveContentMode());
}

// ------------------------------------------------------------
CContentBridgeInitParams::~CContentBridgeInitParams ()
{

}

// ------------------------------------------------------------
const char* CContentBridgeInitParams::NameOf () const
{
	return "CContentBridgeInitParams";
}
// ------------------------------------------------------------


