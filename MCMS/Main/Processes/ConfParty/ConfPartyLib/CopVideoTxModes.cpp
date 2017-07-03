//+========================================================================+
//                        CopVideoTxModes.cpp                              |
//               Copyright 1995 Pictel Technologies Ltd.                   |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CopVideoTxModes.cpp                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: 		                                                       |
//-------------------------------------------------------------------------|
// Who  | Date    | Description			                                   |
//-------------------------------------------------------------------------|
//                                                                         |
//+========================================================================+

#include "CopVideoTxModes.h"
#include "ConfPartyGlobals.h"
#include "CommModeInfo.h"
#include "H264.h"
#include "COP_video_mode.h"

/////////////////////////////////////////////////////////////////////////////
CCopVideoTxModes::CCopVideoTxModes()
{
	int i;
	for (int i=0;i<NUMBER_OF_COP_LEVELS;i++)
		m_pVideoModes[i] = new CVidModeH323;

}
/////////////////////////////////////////////////////////////////////////////
CCopVideoTxModes::~CCopVideoTxModes()
{
	int i;
	for (i=0;i<NUMBER_OF_COP_LEVELS;i++)
		POBJDELETE(m_pVideoModes[i]);
}
/////////////////////////////////////////////////////////////////////////////
CCopVideoTxModes::CCopVideoTxModes(const CCopVideoTxModes &other)
:CPObject(other)
{
	int i;
	for (i=0;i<NUMBER_OF_COP_LEVELS;i++)
	{
		m_pVideoModes[i] = new CVidModeH323;
		if (other.m_pVideoModes[i])
			*(m_pVideoModes[i]) = *(other.m_pVideoModes[i]);
	}
}
/////////////////////////////////////////////////////////////////////////////
CCopVideoTxModes& CCopVideoTxModes::operator=(const CCopVideoTxModes &other)
{
	if ( &other == this )
		return *this;

	int i;
	for (i=0;i<NUMBER_OF_COP_LEVELS;i++)
	{
		POBJDELETE(m_pVideoModes[i]);
		if (other.m_pVideoModes[i])
		{
			m_pVideoModes[i] = new CVidModeH323;
			*(m_pVideoModes[i]) = *(other.m_pVideoModes[i]);
		}
	}
	return *this;
}
//////////////////////////////////////////////////////////////////////////
void CCopVideoTxModes::SetVideoTxMode(BYTE index,CVidModeH323* vidMode)
{
	if(index < 4 && vidMode && CPObject::IsValidPObjectPtr(vidMode))
	{
		POBJDELETE(m_pVideoModes[index]);
		m_pVideoModes[index] = new CVidModeH323;
		*(m_pVideoModes[index]) = *(vidMode);
	}
	else
	{
		DBGPASSERT(104);
		PTRACE2INT(eLevelInfoNormal,"CCopVideoTxModes::SetVideoTxMode can't create mode mode index ",index);
	}

}
///////////////////////////////////////////////////////////////////////////
const char* CCopVideoTxModes::NameOf () const
{
	return "CCopVideoTxModes";
}
/////////////////////////////////////////////////////////////////////////////
void CCopVideoTxModes::Dump(const char* title, WORD level) const
{
	COstrStream msg;
    if(title != NULL)
        msg << title;
	Dump(msg);
	PTRACE(level,msg.str().c_str());
}
/////////////////////////////////////////////////////////////////////////////
void CCopVideoTxModes::Dump(std::ostream& msg) const
{
    msg.setf(std::ios::left,std::ios::adjustfield);
    msg.setf(std::ios::showbase);
    int i;
    for (i=0;i<NUMBER_OF_COP_LEVELS;i++)
    {
	    msg << "\n--Video Mode "<< i << ":";
	    if (m_pVideoModes[i])
	    	m_pVideoModes[i]->Dump(msg);
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CCopVideoTxModes::Serialize(WORD format,CSegment& seg) const
{
    switch (format)
    {
    case SERIALEMBD :
        break;

    case NATIVE     :
    {
    	int i;
        for (i=0;i<NUMBER_OF_COP_LEVELS;i++)
        	m_pVideoModes[i]->Serialize(format,seg);
        break;

    }
    default :
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////
void  CCopVideoTxModes::DeSerialize(WORD format,CSegment& seg)
{
    switch ( format )
    {
    case SERIALEMBD :
        break;

    case NATIVE     :
    {
       	int i;
        for (i=0;i<NUMBER_OF_COP_LEVELS;i++)
        	m_pVideoModes[i]->DeSerialize(format,seg);
        break;
    }
    default :
        break;

    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CVidModeH323* CCopVideoTxModes::GetVideoMode(int index) const
{
	if (index < NUMBER_OF_COP_LEVELS)
		return m_pVideoModes[index];
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CCopVideoTxModes::SetModesAccodingToCopParams(CCOPConfigurationList* pCOPConfigurationList)
{
	CCopVideoParams* pCopVideoParams;
	int i;
	for (i=0;i<NUMBER_OF_COP_LEVELS;i++)
	{
		pCopVideoParams = pCOPConfigurationList->GetVideoMode(i);
		CreateModeFromCopParams(pCopVideoParams, i);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CCopVideoTxModes::CreateModeFromCopParams(CCopVideoParams* pCopVideoParams, int modeIndex)
{
	PTRACE2INT(eLevelInfoNormal,"CCopVideoTxModes::CreateModeFromCopParams mode index ",modeIndex);

	if (!CPObject::IsValidPObjectPtr(pCopVideoParams))
	{
		DBGPASSERT(1);
		return;
	}

	POBJDELETE(m_pVideoModes[modeIndex]);
	m_pVideoModes[modeIndex] = new CVidModeH323;

	BYTE protocol = pCopVideoParams->GetProtocol();

	if (protocol == VIDEO_PROTOCOL_H264 || protocol == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
	{
		sCopH264VideoMode copH264VideoMode;
		CCopVideoModeTable* pCopTable = new CCopVideoModeTable;
		APIU16 profile = H264_Profile_BaseLine;
		if(protocol == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
		{
			profile = H264_Profile_High;

		}
		if(!IsFeatureSupportedBySystem(eFeatureH264HighProfile) && protocol == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
			DBGPASSERT(protocol+1000);

	    pCopTable->GetSignalingH264ModeAccordingToReservationParams(pCopVideoParams, copH264VideoMode, TRUE);
	    m_pVideoModes[modeIndex]->SetH264Scm(profile, copH264VideoMode.levelValue, copH264VideoMode.maxMBPS, copH264VideoMode.maxFS, copH264VideoMode.maxDPB,
											 copH264VideoMode.maxBR, copH264VideoMode.maxSAR, copH264VideoMode.maxStaticMbps, cmCapTransmit);
		POBJDELETE(pCopTable);
	}
	else if (protocol == VIDEO_PROTOCOL_H263 || protocol == VIDEO_PROTOCOL_H261)
	{
		int qcifMpi = -1, cifMpi = -1, cif4Mpi = -1, cif16Mpi = -1;
		CCopVideoModeTable* pCopTable = new CCopVideoModeTable;
		pCopTable->GetSignalingH263H261ModeAccordingToReservationParams(pCopVideoParams, TRUE,qcifMpi,cifMpi,cif4Mpi,cif16Mpi);

		CComModeInfo lComModeInfo(protocol,(WORD)StartVideoCap);
	    CapEnum h323VideoCapCode = lComModeInfo.GetH323ModeType();
	    m_pVideoModes[modeIndex]->SetScmMpi(h323VideoCapCode, qcifMpi, cifMpi, cif4Mpi, cif16Mpi);
	    POBJDELETE(pCopTable);
	}
	else
		DBGPASSERT(protocol+1000);

	// Set Bit rate:
	int levelRate = pCopVideoParams->GetBitRate();
	CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
	DWORD levelBitRate = lCapInfo.TranslateReservationRateToIpRate(levelRate);
	levelBitRate *= 1000;
	DWORD audioRate = CalculateAudioRate(levelBitRate);
	DWORD videoRate = levelBitRate - audioRate;
	videoRate /= 100; // system units
	m_pVideoModes[modeIndex]->SetBitRate(videoRate);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCopVideoTxModes::GetMatchingIndex(CVidModeH323* pVideoMode)
{
	WORD modeIndex;
	for (modeIndex=0;modeIndex<NUMBER_OF_COP_LEVELS;modeIndex++)
	{
		if (*(m_pVideoModes[modeIndex]) == *pVideoMode)
			return modeIndex;
	}
	return 0xFFFF;
}
////////////////////////////////////////////////////////////////////////////////////////
BYTE CCopVideoTxModes::IsValidForDefinedParams(int modeIndex, BYTE definedProtocol, DWORD definedRate)
{
	BYTE ret = TRUE;

	CapEnum videoType = (CapEnum)m_pVideoModes[modeIndex]->GetType();
	if (((definedProtocol == VIDEO_PROTOCOL_H261) && (videoType != eH261CapCode))
		|| ((definedProtocol == VIDEO_PROTOCOL_H263) && (videoType != eH263CapCode))
		|| ((definedProtocol == VIDEO_PROTOCOL_H264) && (videoType != eH264CapCode)))
		ret = FALSE;
	else if ((definedRate != 0xFFFFFFFF) && (m_pVideoModes[modeIndex]->GetBitRate() > definedRate*10))
		ret = FALSE;

	if (ret==FALSE)
		PTRACE2INT(eLevelInfoNormal, "CCopVideoTxModes::IsValidForDefinedParams : not valid. index=", modeIndex);
	return ret;
}
