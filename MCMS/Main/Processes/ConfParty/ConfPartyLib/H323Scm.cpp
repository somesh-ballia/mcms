//+========================================================================+
//                            H323SCM.CPP                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323SCM.CPP                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 1/02/98    |                                                      |
//+========================================================================+


#include <ostream>
#include "H323Scm.h"
#include "Segment.h"

#include "Macros.h"
#include "ObjString.h"
#include "Trace.h"
#include "CommModeInfo.h"
#include "NStream.h"
#include "EncryptionKey.h"
#include "AllocateStructs.h"
#include "H264Util.h"

#include "H323Caps.h"
#include "CapClass.h"
#include "SipCaps.h"
#include "COP_video_mode.h"
#include "SipUtils.h"
#include "TipUtils.h"
#include "SysConfigKeys.h"
#include "IpCommon.h"
#include "Trace.h"
#include "EnumsToStrings.h"

extern DWORD CalculateAudioRate(DWORD call_rate);


/////////////////////////////////////////////////////////////////////////////
CComModeH323::CComModeH323()
{
	m_isEncrypted = Encryp_Off;
	m_bDisconnectOnEncryptionFailure = FALSE;
	m_encMediaTypeAlg = kUnKnownMediaType;
	m_halfKeyType = kHalfKeyUnKnownType;
	m_eConfType = kUnknownType;

	m_eTipContentMode = eTipCompatibleNone;

    m_confMediaType = eConfMediaType_dummy;
	m_callRate  = 0;
	m_bIsHd720Enabled = FALSE;
	m_bIsHd1080Enabled = FALSE;
	m_bIsHd720At60Enabled = FALSE;
	m_bIsHd1080At60Enabled = FALSE;
	m_flowControlRateConstraint = 0;
	m_flowControlRateConstraintForPresentation = 0;
	m_isLpr = 1;

	//LYNC2013_FEC_RED:
	m_isFec = 0;
	m_isRed = 0;

	m_TotalVideoRate = 0;
	m_HdVswResolution = eHD720Res;
	m_contentProtocol = eNoPresentation;
	m_declareContentRate = 0;
	m_copTxLevel = INVALID_COP_LEVEL;
	m_bShowContentAsVideo = NO;
	m_eTipMode = eTipModeNone;// TIP
	m_TipAuxFPS = eTipAuxNone;
	m_eOPPreset = eOPP_dummy;
	m_eIsUseOperationPointesPresets = eIsUseOPP_No;
    m_partyId = DEFAULT_PARTY_ID;
	m_isDtlsEncrypted = Encryp_Off; //_dtls_
	m_bDtlsAvailable = FALSE;
    m_partyMediaType = eAvcPartyType;
}

/////////////////////////////////////////////////////////////////////////////
CComModeH323::~CComModeH323()	{}

/////////////////////////////////////////////////////////////////////////////
CComModeH323::CComModeH323(const CComModeH323 &other) :
CPObject(other)
{

	m_audModeRcv		=	other.m_audModeRcv;
	m_audModeXmit		=	other.m_audModeXmit;

	m_vidModeRcv		=	other.m_vidModeRcv;
	m_vidModeXmit		=	other.m_vidModeXmit;

	m_vidContModeRcv	=	other.m_vidContModeRcv;
	m_vidContModeXmit	=	other.m_vidContModeXmit;

    m_dataModeRcv		=	other.m_dataModeRcv;
    m_dataModeXmit		=	other.m_dataModeXmit;

    m_bfcpModeRcv		=	other.m_bfcpModeRcv;
    m_bfcpModeXmit		=	other.m_bfcpModeXmit;

	m_eConfType			=	other.m_eConfType;
	m_eTipContentMode =   other.m_eTipContentMode;
    m_confMediaType         =   other.m_confMediaType;
	m_callRate			=   other.m_callRate;
	m_TotalVideoRate    =   other.m_TotalVideoRate;
	// Encryption
	m_isEncrypted		=   other.m_isEncrypted;
	m_bDisconnectOnEncryptionFailure		=   other.m_bDisconnectOnEncryptionFailure;
	m_encMediaTypeAlg	=   other.m_encMediaTypeAlg;
	m_halfKeyType		=   other.m_halfKeyType;

	m_bIsHd720Enabled      =   other.m_bIsHd720Enabled;
	m_bIsHd1080Enabled     =   other.m_bIsHd1080Enabled;
	m_bIsHd720At60Enabled    =   other.m_bIsHd720At60Enabled;
	m_bIsHd1080At60Enabled    =   other.m_bIsHd1080At60Enabled;

	m_flowControlRateConstraint=	other.m_flowControlRateConstraint;
	m_flowControlRateConstraintForPresentation = other.m_flowControlRateConstraintForPresentation;

	m_isLpr				= 	other.m_isLpr;

	//LYNC2013_FEC_RED:
	m_isFec = other.m_isFec;
	m_isRed = other.m_isRed;

	m_HdVswResolution   =  other.m_HdVswResolution;
	m_contentProtocol	= 	other.m_contentProtocol;
	m_declareContentRate    =   other.m_declareContentRate;
	
	m_bShowContentAsVideo = other.m_bShowContentAsVideo;
	m_copTxLevel		= other.m_copTxLevel;
	m_eTipMode 			= other.m_eTipMode;// TIP
	m_TipAuxFPS 		= other.m_TipAuxFPS;
	m_operationPoints   = other.m_operationPoints;
	m_eOPPreset			= other.m_eOPPreset;
	m_eIsUseOperationPointesPresets = other.m_eIsUseOperationPointesPresets;
	m_partyId           = other.m_partyId;
	m_isDtlsEncrypted		=   other.m_isDtlsEncrypted;
	m_bDtlsAvailable		=   other.m_bDtlsAvailable;
	m_partyMediaType 	= other.m_partyMediaType;
}
/*
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::Create(const CAudModeH323& newAudMode,
						  const CVidModeH323& newVidMode,const CVidModeH323 &newVidContMode,
						  const CDataModeH323& newDataMode,EConfType eConfType, DWORD isEncrypted,
						  EenMediaType encMediaType,EenHalfKeyType halfKeyType, BYTE bIsHd720Enabled, BYTE bIsHd1080Enabled, BYTE isLpr,EHDResolution hdVswResolution,
                          cmCapDirection direction)
{
	PASSERT_AND_RETURN(
		(!newAudMode.IsValidMode())||
		(!newVidMode.IsValidMode())||(!newVidContMode.IsValidMode())||
		(!newDataMode.IsValidMode()));

    if (direction & cmCapReceive)
    {
        m_audModeRcv     = newAudMode;
        m_vidModeRcv     = newVidMode;
		m_vidContModeRcv = newVidContMode;
        m_dataModeRcv    = newDataMode;
    }
    if (direction & cmCapTransmit)
    {
        m_audModeXmit     = newAudMode;
        m_vidModeXmit     = newVidMode;
		m_vidContModeXmit = newVidContMode;
        m_dataModeXmit    = newDataMode;
    }
	m_eConfType = eConfType;
	m_isEncrypted = isEncrypted;
	m_encMediaTypeAlg = encMediaType;
	m_halfKeyType = halfKeyType;
	m_bIsHd720Enabled = bIsHd720Enabled;
	m_isLpr = isLpr;
	m_HdVswResolution = hdVswResolution;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::Create(capBuffer *pNewAudMode,
						  capBuffer *pNewVidMode,capBuffer *pNewVidContMode,
						  capBuffer *pNewDataMode,EConfType eConfType, DWORD isEncrypted,
						  EenMediaType encMediaType,EenHalfKeyType halfKeyType, BYTE bIsHd720Enabled, BYTE bIsHd1080Enabled, BYTE isLpr,EHDResolution hdVswResolution,
						  cmCapDirection direction )
{
    if (direction & cmCapReceive)
    {
        m_audModeRcv.Create(pNewAudMode);
        m_vidModeRcv.Create(pNewVidMode);
        m_vidContModeRcv.Create(pNewVidContMode);
        m_dataModeRcv.Create(pNewDataMode);
    }
    if (direction & cmCapTransmit)
    {
        m_audModeXmit.Create(pNewAudMode);
        m_vidModeXmit.Create(pNewVidMode);
        m_vidContModeXmit.Create(pNewVidContMode);
        m_dataModeXmit.Create(pNewDataMode);
    }
	m_eConfType = eConfType;
	m_isEncrypted = isEncrypted;
	m_encMediaTypeAlg = encMediaType;
	m_halfKeyType = halfKeyType;
	m_bIsHd720Enabled = bIsHd720Enabled;
	m_bIsHd1080Enabled = bIsHd1080Enabled;
	m_isLpr = isLpr;
	m_HdVswResolution = hdVswResolution;
}


*/
/////////////////////////////////////////////////////////////////////////////
void  CComModeH323::Serialize(WORD format,CSegment& seg) const
{
    switch (format)
    {
    case SERIALEMBD :
        break;

    case NATIVE     :
        m_audModeRcv.Serialize(format,seg);
        m_audModeXmit.Serialize(format,seg);

        m_vidModeRcv.Serialize(format,seg);
        m_vidModeXmit.Serialize(format,seg);

        m_vidContModeRcv.Serialize(format,seg);
        m_vidContModeXmit.Serialize(format,seg);

        m_dataModeRcv.Serialize(format,seg);
        m_dataModeXmit.Serialize(format,seg);

        m_bfcpModeRcv.Serialize(format,seg);
        m_bfcpModeXmit.Serialize(format,seg);

        m_operationPoints.Serialize(seg);

		seg << (BYTE)m_eConfType;
        seg << (BYTE)m_confMediaType;
		seg << (DWORD)m_callRate;
		seg << (DWORD)m_isEncrypted;
		seg << (BYTE)m_encMediaTypeAlg;
		seg << (BYTE)m_halfKeyType;
		seg << (BYTE)m_bDisconnectOnEncryptionFailure;
		seg << (BYTE)m_bIsHd720Enabled;
		seg << (BYTE)m_bIsHd1080Enabled;
		seg << (BYTE)m_bIsHd720At60Enabled;
		seg << (BYTE)m_bIsHd1080At60Enabled;
		seg << (DWORD)m_flowControlRateConstraint;
		seg << (DWORD)m_flowControlRateConstraintForPresentation;
		seg << (BYTE)m_isLpr;
		seg << (BYTE)m_isFec;
		seg << (BYTE)m_isRed;
		seg << (BYTE) m_HdVswResolution;
		seg << (BYTE)m_contentProtocol;
		
		seg << (DWORD)m_declareContentRate;
		seg << (DWORD)m_TotalVideoRate;
		seg << (BYTE)m_bShowContentAsVideo;
		seg << (BYTE)m_copTxLevel;
		seg << (DWORD)m_partyId;
		seg << (BYTE)m_eTipMode;
		seg << (BYTE)m_TipAuxFPS;
		seg << (BYTE)m_eTipContentMode;
		seg << (DWORD)m_isDtlsEncrypted; //_dtls_
		seg << (BYTE)m_bDtlsAvailable; //_dtls_
		seg << (BYTE)m_eOPPreset;
		seg << (BYTE)m_eIsUseOperationPointesPresets;
        break;

    default :
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CComModeH323::DeSerialize(WORD format,CSegment& seg)
{
    switch ( format )
    {
    case SERIALEMBD :
        break;

    case NATIVE     :
        m_audModeRcv.DeSerialize(format, seg);
        m_audModeXmit.DeSerialize(format, seg);

        m_vidModeRcv.DeSerialize(format, seg);
        m_vidModeXmit.DeSerialize(format, seg);

        m_vidContModeRcv.DeSerialize(format, seg);
        m_vidContModeXmit.DeSerialize(format, seg);

        m_dataModeRcv.DeSerialize(format,seg);
        m_dataModeXmit.DeSerialize(format,seg);

        m_bfcpModeRcv.DeSerialize(format,seg);
        m_bfcpModeXmit.DeSerialize(format,seg);

        m_operationPoints.DeSerialize(seg);

		seg >> (BYTE&)m_eConfType;
        seg >> (BYTE&)m_confMediaType;
		seg >> (DWORD&)m_callRate;
		seg >> (DWORD&)m_isEncrypted;
		seg >> (BYTE&)m_encMediaTypeAlg;
		seg >> (BYTE&)m_halfKeyType;
		seg >> (BYTE&)m_bDisconnectOnEncryptionFailure;
		seg >> (BYTE&)m_bIsHd720Enabled;
		seg >> (BYTE&)m_bIsHd1080Enabled;
		seg >> (BYTE&)m_bIsHd720At60Enabled;
		seg >> (BYTE&)m_bIsHd1080At60Enabled;
		seg >> (DWORD&)m_flowControlRateConstraint;
		seg >> (DWORD&)m_flowControlRateConstraintForPresentation;
		seg >> (BYTE&)m_isLpr;
		seg >> (BYTE&)m_isFec;
		seg >> (BYTE&)m_isRed;
		seg >> (BYTE&)m_HdVswResolution;
		seg >> (BYTE&)m_contentProtocol;
		
		seg >> (DWORD&)m_declareContentRate;
		seg >> (DWORD&)m_TotalVideoRate;
		seg >> (BYTE&)m_bShowContentAsVideo;
		seg >> (BYTE&)m_copTxLevel;
		seg >> (DWORD&)m_partyId;
		seg >> (BYTE&)m_eTipMode;
		seg >> (BYTE&)m_TipAuxFPS;
		seg >> (BYTE&)m_eTipContentMode;
		seg >> (DWORD&)m_isDtlsEncrypted;
		seg >> (BYTE&)m_bDtlsAvailable;
		seg >> (BYTE&)m_eOPPreset;
		seg >> (BYTE&)m_eIsUseOperationPointesPresets;
        break;

    default :
        break;

    }
}

/////////////////////////////////////////////////////////////////////////////
void  CComModeH323::Serialize(CSegment& seg,cmCapDirection direction,BYTE bOperUse,CCapH323* pCaps) const
{
    switch (direction)
    {
    case cmCapReceive :
        Serialize(seg,m_audModeRcv,m_vidModeRcv,m_vidContModeRcv,m_dataModeRcv, m_bfcpModeRcv, bOperUse, pCaps, direction);
        break;

    case cmCapTransmit :
        Serialize(seg,m_audModeXmit,m_vidModeXmit,m_vidContModeXmit,m_dataModeXmit, m_bfcpModeXmit ,bOperUse, pCaps, direction);
        break;

    default :
        PTRACE(eLevelError,"CComModeH323::Serialize:  Direction is not valid.");
    }
    m_operationPoints.Serialize(seg);

	seg << (BYTE)m_eConfType;
    seg << (BYTE)m_confMediaType;
	seg << (DWORD)m_isEncrypted;
	seg << (BYTE)m_encMediaTypeAlg;
	seg << (BYTE)m_halfKeyType;
	seg << (BYTE)m_bDisconnectOnEncryptionFailure;
	seg << (BYTE)m_bIsHd720Enabled;
	seg << (BYTE)m_bIsHd1080Enabled;
	seg << (BYTE)m_bIsHd720At60Enabled;
	seg << (BYTE)m_bIsHd1080At60Enabled;
	seg << (DWORD)m_flowControlRateConstraint;
	seg << (DWORD)m_flowControlRateConstraintForPresentation;
	seg << (BYTE)m_isLpr;
	seg << (BYTE)m_isFec;
	seg << (BYTE)m_isRed;
	seg << (BYTE)m_HdVswResolution;
	seg << (BYTE)m_contentProtocol;
	seg << (BYTE)m_bShowContentAsVideo;
	seg << (BYTE)m_copTxLevel;
	seg << (DWORD)m_partyId;
	seg << (BYTE)m_eTipMode;// TIP
	seg << (BYTE)m_TipAuxFPS;
	seg << (BYTE)m_eTipContentMode;
	seg << (DWORD)m_isDtlsEncrypted;
	seg << (BYTE)m_bDtlsAvailable;
	seg << (BYTE)m_eOPPreset;
	seg << (BYTE)m_eIsUseOperationPointesPresets;
}

/////////////////////////////////////////////////////////////////////////////
void  CComModeH323::Serialize(CSegment &seg, const CAudModeH323 &audMode,
							  const CVidModeH323 &vidMode,const CVidModeH323 &vidContMode,
							  const CDataModeH323 &dataMode,const CBfcpModeH323 &bfcpMode, BYTE bOperUse,CCapH323* pCaps, cmCapDirection direction) const
{
	capBuffer *pAudio = NULL;
	capBuffer *pVideo = NULL;
	capBuffer *pVideoCont = NULL;
	capBuffer *pData = NULL;
	capBuffer *pBfcp = NULL;
	capBuffer *pLpr = NULL;

	WORD capArrSize = 0;
	if (pCaps)
		capArrSize = 5;
	else
		capArrSize = 4;

	WORD size = 0;
	// 4 = (audio + video + video content + t120)
	BYTE* capArray = new BYTE[capArrSize*(sizeof(capBufferBase) + MAX_MEDIA_CAP_LENGTH)];
	memset(capArray,0,(capArrSize*(sizeof(capBufferBase) + MAX_MEDIA_CAP_LENGTH)));
	pAudio = (capBuffer *)capArray;

	if( audMode.IsModeOff() )
		pVideo = (capBuffer *)capArray;
	else
	{
		audMode.CopyToCapBuffer(pAudio,bOperUse);
		size += sizeof(capBufferBase) + pAudio->capLength;
	    pVideo = (capBuffer *)(capArray + size);
	}

	if( vidMode.IsModeOff() )
		pVideoCont = (capBuffer *)pVideo;
	else
	{
		vidMode.CopyToCapBuffer(pVideo,bOperUse);
		size += sizeof(capBufferBase) + pVideo->capLength;
		pVideoCont = (capBuffer *)(capArray + size);
	}

	if( vidContMode.IsModeOff() )
		pData = (capBuffer *)pVideoCont;
	else
	{
		vidContMode.CopyToCapBuffer(pVideoCont,bOperUse);
		size += sizeof(capBufferBase) + pVideoCont->capLength;
		pData = (capBuffer *)(capArray + size);
	}

	if( dataMode.IsModeOff() )
		pBfcp = (capBuffer *)pData;
	else
	{
		dataMode.CopyToCapBuffer(pData,bOperUse);
		size += sizeof(capBufferBase) + pData->capLength;
		pBfcp = (capBuffer *)(capArray + size);
	}

	if( bfcpMode.IsModeOff() )
		pLpr = (capBuffer *)pBfcp;
	else
	{
		bfcpMode.CopyToCapBuffer(pBfcp,bOperUse);
		size += sizeof(capBufferBase) + pBfcp->capLength;

		if (pCaps)
			pLpr = (capBuffer *)(capArray + size);
	}

	if (pCaps)
	{
		CLprCap* pLprCap = NULL;
		pLprCap = pCaps->GetLprCapability(eLPRCapCode);
		if (pLprCap)
		{
			lprCapStruct lprCaps;
			pLpr->capTypeCode = eLPRCapCode;
			pLpr->capLength = sizeof(lprCapStruct);
			lprCaps.header.direction 		  	= direction;
			lprCaps.header.type 			  	= cmCapGeneric;
			lprCaps.header.roleLabel        	= kRolePeople;
			lprCaps.header.capTypeCode			= eLPRCapCode;
			lprCaps.header.xmlHeader.dynamicType = lprCaps.header.capTypeCode;
			lprCaps.header.xmlHeader.dynamicLength = sizeof(lprCapStruct);

			lprCaps.versionID = pLprCap->GetLprVersionID();
			lprCaps.minProtectionPeriod = pLprCap->GetLprMinProtectionPeriod();
			lprCaps.maxProtectionPeriod = pLprCap->GetLprMaxProtectionPeriod();
			lprCaps.maxRecoverySet = pLprCap->GetLprMaxRecoverySet();
			lprCaps.maxRecoveryPackets = pLprCap->GetLprMaxRecoveryPackets();
			lprCaps.maxPacketSize = pLprCap->GetLprMaxPacketSize();

			memcpy(pLpr->dataCap,&lprCaps,pLpr->capLength);
			size += sizeof(capBufferBase) + pLpr->capLength;

			pLprCap->FreeStruct();
			POBJDELETE(pLprCap);
		}
	}

    seg << size;
    for(WORD i=0; i<size; i++)
        seg << capArray[i];

    PDELETEA(capArray);

	seg << (BYTE)m_eConfType;
    seg << (BYTE)m_confMediaType;
	seg << (DWORD)m_isEncrypted;
	seg << (BYTE)m_encMediaTypeAlg;
	seg << (BYTE)m_halfKeyType;
	seg << (BYTE)m_bDisconnectOnEncryptionFailure;
	seg << m_bIsHd720Enabled;
	seg << m_bIsHd1080Enabled;
	seg << m_bIsHd720At60Enabled;
	seg << m_bIsHd1080At60Enabled;
	seg << (DWORD)m_flowControlRateConstraint;
	seg << (DWORD)m_flowControlRateConstraintForPresentation;
	seg << (BYTE)m_isLpr;
	seg << (BYTE)m_isFec;
	seg << (BYTE)m_isRed;
	seg << (BYTE)m_HdVswResolution;
	seg << (BYTE)m_contentProtocol;
	seg << (BYTE)m_bShowContentAsVideo;
	seg << (BYTE)m_copTxLevel;
	seg << (DWORD)m_partyId;
	seg << (BYTE)m_eTipMode;// TIP
	seg << (BYTE)m_TipAuxFPS;
	seg << (BYTE)m_eTipContentMode;
	seg << (DWORD)m_isDtlsEncrypted;
	seg << (BYTE)m_bDtlsAvailable;
	seg << (BYTE)m_eOPPreset;
	seg << (BYTE)m_eIsUseOperationPointesPresets;
}

/////////////////////////////////////////////////////////////////////////////
void  CComModeH323::Serialize(CSegment& seg,cmCapDirection direction,BYTE bOperUse,CSipCaps* pCaps) const
{
    switch (direction)
    {
    case cmCapReceive :
        Serialize(seg,m_audModeRcv,m_vidModeRcv,m_vidContModeRcv,m_dataModeRcv, m_bfcpModeRcv, bOperUse, pCaps, direction);
        break;

    case cmCapTransmit :
        Serialize(seg,m_audModeXmit,m_vidModeXmit,m_vidContModeXmit,m_dataModeXmit, m_bfcpModeXmit, bOperUse, pCaps, direction);
        break;

    default :
        PTRACE(eLevelError,"CComModeH323::Serialize:  Direction is not valid.");
    }
    m_operationPoints.Serialize(seg);
	seg << (BYTE)m_eConfType;
    seg << (BYTE)m_confMediaType;
	seg << (DWORD)m_isEncrypted;
	seg << (BYTE)m_encMediaTypeAlg;
	seg << (BYTE)m_halfKeyType;
	seg << (BYTE)m_bDisconnectOnEncryptionFailure;
	seg << m_bIsHd720Enabled;
	seg << m_bIsHd1080Enabled;
	seg << m_bIsHd720At60Enabled;
	seg << m_bIsHd1080At60Enabled;
	seg << (DWORD)m_flowControlRateConstraint;
	seg << (DWORD)m_flowControlRateConstraintForPresentation;
	seg << (BYTE)m_isLpr;
	seg << (BYTE)m_isFec;
	seg << (BYTE)m_isRed;
	seg << (BYTE)m_HdVswResolution;
	seg << (BYTE)m_contentProtocol;
	seg << (BYTE)m_bShowContentAsVideo;
	seg << (DWORD)m_partyId;
	seg << (BYTE)m_eTipMode;// TIP
	seg << (BYTE)m_TipAuxFPS;
	seg << (BYTE)m_eTipContentMode;
	seg << (DWORD)m_isDtlsEncrypted;
	seg << (BYTE)m_bDtlsAvailable;
	seg << (BYTE)m_eOPPreset;
	seg << (BYTE)m_eIsUseOperationPointesPresets;
}

/////////////////////////////////////////////////////////////////////////////
void  CComModeH323::Serialize(CSegment &seg, const CAudModeH323 &audMode,
							  const CVidModeH323 &vidMode,const CVidModeH323 &vidContMode,
							  const CDataModeH323 &dataMode, const CBfcpModeH323 &bfcpMode, BYTE bOperUse,CSipCaps* pCaps, cmCapDirection direction) const
{
	capBuffer *pAudio = NULL;
	capBuffer *pVideo = NULL;
	capBuffer *pVideoCont = NULL;
	capBuffer *pData = NULL;
	capBuffer *pBfcp = NULL;
	capBuffer *pLpr = NULL;

	WORD capArrSize = 0;
	if (pCaps)
		capArrSize = 5;
	else
		capArrSize = 4;

	WORD size = 0;
	// 4 = (audio + video + video content + t120)
	BYTE* capArray = new BYTE[capArrSize*(sizeof(capBufferBase) + MAX_MEDIA_CAP_LENGTH)];
	PASSERT_AND_RETURN(capArray == NULL);
	memset(capArray,0,(capArrSize*(sizeof(capBufferBase) + MAX_MEDIA_CAP_LENGTH)));
	pAudio = (capBuffer *)capArray;

	if( audMode.IsModeOff() )
		pVideo = (capBuffer *)capArray;
	else
	{
		audMode.CopyToCapBuffer(pAudio,bOperUse);
		size += sizeof(capBufferBase) + pAudio->capLength;

		CSdesCap* pSdesCap = audMode.GetSdesCap();
		if(pSdesCap) {
			pAudio = (capBuffer *)(capArray + size);
			audMode.CopySdesToCapBuffer(pAudio,bOperUse);
			size += sizeof(capBufferBase) + pAudio->capLength;
		}

		//_dtls_
		CDtlsCap* pDtlsCap = audMode.GetDtlsCap();
		if(pDtlsCap) {
			pAudio = (capBuffer *)(capArray + size);
			audMode.CopyDtlsToCapBuffer(pAudio,bOperUse);
			size += sizeof(capBufferBase) + pAudio->capLength;
		}

	    pVideo = (capBuffer *)(capArray + size);
	}

	if( vidMode.IsModeOff() )
		pVideoCont = (capBuffer *)pVideo;
	else
	{
		vidMode.CopyToCapBuffer(pVideo,bOperUse);
		size += sizeof(capBufferBase) + pVideo->capLength;

		CSdesCap* pSdesCap = vidMode.GetSdesCap();
		if(pSdesCap) {
			pVideo = (capBuffer *)(capArray + size);
			vidMode.CopySdesToCapBuffer(pVideo,bOperUse);
			size += sizeof(capBufferBase) + pVideo->capLength;
		}

		//_dtls_
		CDtlsCap* pDtlsCap = vidMode.GetDtlsCap();
		if(pDtlsCap) {
			pVideo = (capBuffer *)(capArray + size);
			vidMode.CopyDtlsToCapBuffer(pVideo,bOperUse);
			size += sizeof(capBufferBase) + pVideo->capLength;
		}

		pVideoCont = (capBuffer *)(capArray + size);
	}

	if( vidContMode.IsModeOff() )
		pData = (capBuffer *)pVideoCont;
	else
	{
		vidContMode.CopyToCapBuffer(pVideoCont,bOperUse);
		size += sizeof(capBufferBase) + pVideoCont->capLength;

		CSdesCap* pSdesCap = vidContMode.GetSdesCap();
		if(pSdesCap) {
			pVideoCont = (capBuffer *)(capArray + size);
			vidContMode.CopySdesToCapBuffer(pVideoCont,bOperUse);
			size += sizeof(capBufferBase) + pVideoCont->capLength;

		}

		//_dtls_
		CDtlsCap* pDtlsCap = vidContMode.GetDtlsCap();
		if(pDtlsCap) {
			pVideoCont = (capBuffer *)(capArray + size);
			vidContMode.CopyDtlsToCapBuffer(pVideoCont,bOperUse);
			size += sizeof(capBufferBase) + pVideoCont->capLength;

		}

		pData = (capBuffer *)(capArray + size);
	}

	if(dataMode.IsModeOff())
	{
		pBfcp = pData? (capBuffer *)pData : NULL;
	}
	else
	{
		dataMode.CopyToCapBuffer(pData,bOperUse);
		size += sizeof(capBufferBase) + pData->capLength;

		CSdesCap* pSdesCap = dataMode.GetSdesCap();
		if(pSdesCap) {
			pData = (capBuffer *)(capArray + size);
			dataMode.CopySdesToCapBuffer(pData,bOperUse);
			size += sizeof(capBufferBase) + pData->capLength;
		}

		//_dtls_
		CDtlsCap* pDtlsCap = dataMode.GetDtlsCap();
		if(pDtlsCap) {
			pData = (capBuffer *)(capArray + size);
			dataMode.CopyDtlsToCapBuffer(pData,bOperUse);
			size += sizeof(capBufferBase) + pData->capLength;
		}

		if (pCaps)
			pBfcp = (capBuffer *)(capArray + size);
	}

	if(bfcpMode.IsModeOff())
	{
		pLpr = pBfcp? (capBuffer *)pBfcp : NULL;
	}
	else
	{
        bfcpMode.CopyToCapBuffer(pBfcp,bOperUse);
        APIU16 bfcpCapLength = pBfcp ? pBfcp->capLength : 0 ;
        size += sizeof(capBufferBase) + bfcpCapLength;

        CSdesCap* pSdesCap = bfcpMode.GetSdesCap();
		if(pSdesCap) {
			pBfcp = (capBuffer *)(capArray + size);
			bfcpMode.CopySdesToCapBuffer(pBfcp,bOperUse);
			size += sizeof(capBufferBase) + bfcpCapLength;
		}

		//_dtls_
		CDtlsCap* pDtlsCap = bfcpMode.GetDtlsCap();
		if(pDtlsCap) {
			pBfcp = (capBuffer *)(capArray + size);
			bfcpMode.CopyDtlsToCapBuffer(pBfcp,bOperUse);
			size += sizeof(capBufferBase) + bfcpCapLength;
		}

        if (pCaps)
        	pLpr = (capBuffer *)(capArray + size);
	}

	if (pCaps)
	{
		CLprCap* pLprCap = NULL;
		ERoleLabel roleLabel = kRolePeople;
		pLprCap = pCaps->GetLprCap();
		if (!pLprCap)
		{
			pLprCap = pCaps->GetContentLprCap();
			roleLabel = kRolePresentation;
		}
		if (pLprCap && pLpr)
		{
			lprCapStruct lprCaps;
			pLpr->capTypeCode = eLPRCapCode;
			pLpr->capLength = sizeof(lprCapStruct);
			lprCaps.header.direction 		  	= direction;
			lprCaps.header.type 			  	= cmCapGeneric;
			lprCaps.header.roleLabel        	= roleLabel;
			lprCaps.header.capTypeCode			= eLPRCapCode;
			lprCaps.header.xmlHeader.dynamicType = lprCaps.header.capTypeCode;
			lprCaps.header.xmlHeader.dynamicLength = sizeof(lprCapStruct);

			lprCaps.versionID = pLprCap->GetLprVersionID();
			lprCaps.minProtectionPeriod = pLprCap->GetLprMinProtectionPeriod();
			lprCaps.maxProtectionPeriod = pLprCap->GetLprMaxProtectionPeriod();
			lprCaps.maxRecoverySet = pLprCap->GetLprMaxRecoverySet();
			lprCaps.maxRecoveryPackets = pLprCap->GetLprMaxRecoveryPackets();
			lprCaps.maxPacketSize = pLprCap->GetLprMaxPacketSize();

			memcpy(pLpr->dataCap,&lprCaps,pLpr->capLength);
			size += sizeof(capBufferBase) + pLpr->capLength;

			POBJDELETE(pLprCap);

		}
/*
		else
		{
			//LYNC2013_FEC_RED:  to check...!!!!!!!!!!!
			PTRACE(eLevelError,"CComModeH323::Serialize FEC Shira");
			CFecCap* pFecCap = NULL;
			pFecCap = pCaps->GetFecCap();
			if (pFecCap)
			{
				fecCapStruct fecCaps;
				pLpr->capTypeCode = eFECCapCode;
				pLpr->capLength = sizeof(fecCapStruct);
				fecCaps.header.direction 		  	= direction;
				fecCaps.header.type 			  	= cmCapGeneric;
				fecCaps.header.roleLabel        	= kRolePeople;
				fecCaps.header.capTypeCode			= eFECCapCode;
				fecCaps.header.xmlHeader.dynamicType = fecCaps.header.capTypeCode;
				fecCaps.header.xmlHeader.dynamicLength = sizeof(fecCapStruct);

				memcpy(pLpr->dataCap,&fecCaps,pLpr->capLength);
				size += sizeof(capBufferBase) + pLpr->capLength;

				POBJDELETE(pFecCap);
			}
		}

		//LYNC2013_FEC_RED:  to check... !!!!!!!!!!!!!!!!!!!!!!
		CRedCap* pRedCap = NULL;
		pRedCap = pCaps->GetRedCap();
		if (pRedCap)
		{
			PTRACE(eLevelError,"CComModeH323::Serialize RED Shira");

			redCapStruct redCaps;
			pLpr->capTypeCode = eREDCapCode;
			pLpr->capLength = sizeof(redCapStruct);
			redCaps.header.direction 		  	= direction;
			redCaps.header.type 			  	= cmCapGeneric;
			redCaps.header.roleLabel        	= kRolePeople;
			redCaps.header.capTypeCode			= eREDCapCode;
			redCaps.header.xmlHeader.dynamicType = redCaps.header.capTypeCode;
			redCaps.header.xmlHeader.dynamicLength = sizeof(redCapStruct);

			memcpy(pLpr->dataCap,&redCaps,pLpr->capLength);
			size += sizeof(capBufferBase) + pLpr->capLength;

			POBJDELETE(pRedCap);
		}*/

	}
    seg << size;
    for(WORD i=0; i<size; i++)
        seg << capArray[i];

    PDELETEA(capArray);

	seg << (BYTE)m_eConfType;
	seg << (DWORD)m_isEncrypted;
	seg << (BYTE)m_encMediaTypeAlg;
	seg << (BYTE)m_halfKeyType;
	seg << (BYTE)m_bDisconnectOnEncryptionFailure;
	seg << m_bIsHd720Enabled;
	seg << m_bIsHd1080Enabled;
	seg << m_bIsHd720At60Enabled;
	seg << m_bIsHd1080At60Enabled;
	seg << (DWORD)m_flowControlRateConstraint;
	seg << (DWORD)m_flowControlRateConstraintForPresentation;
	seg << (BYTE)m_isLpr;
	seg << (BYTE)m_isFec;
	seg << (BYTE)m_isRed;
	seg << (BYTE)m_HdVswResolution;
	seg << (BYTE)m_contentProtocol;
	seg << (BYTE)m_bShowContentAsVideo;
	seg << (DWORD)m_partyId;
	seg << (BYTE)m_eTipMode;// TIP
	seg << (BYTE)m_TipAuxFPS;
	seg << (BYTE)m_eTipContentMode;
	seg << (DWORD)m_isDtlsEncrypted;
	seg << (BYTE)m_bDtlsAvailable;
	seg << (BYTE)m_eOPPreset;
	seg << (BYTE)m_eIsUseOperationPointesPresets;
}

/////////////////////////////////////////////////////////////////////////////
////////////////////// 				API           ///////////////////////////
/////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsSecondary() const
{
	BYTE rVal = ((GetMediaBitRate(cmCapVideo, cmCapReceive, kRolePeople) == 0) && IsMediaOff(cmCapVideo, cmCapTransmit, kRolePeople));
	return rVal;
}

/////////////////////////////////////////////////////////////////////////////
WORD CComModeH323::GetMediaLength(cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole) const
{
    const CMediaModeH323& rMediaModeH323 = GetMediaMode(dataType, direction, eRole);
	return rMediaModeH323.GetLength();
}

///////////////////////////////////////////////////////////////////////////////
void CComModeH323::GetMediaParams(cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole,
							   CSecondaryParams &secParams,DWORD details) const
{
	const CMediaModeH323& rMediaModeH323 = GetMediaMode(dataType, direction, eRole);
	rMediaModeH323.GetMediaParams(secParams, details);
}

///////////////////////////////////////////////////////////////////////////////
void CComModeH323::GetDiffFromDetails(cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole,
							   CSecondaryParams &secParams,DWORD details) const
{
	const CMediaModeH323& rMediaModeH323  = GetMediaMode(dataType, direction, eRole);
	rMediaModeH323.GetDiffFromDetails(details, secParams);
}

///////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsMediaContaining(const CComModeH323& other, DWORD valuesToCompare, DWORD* pDetails,
									 cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole) const
{
	BYTE bRes = TRUE;
	if (direction & cmCapReceive)
	{
		BYTE bIsMediaOn      = IsMediaOn(dataType, cmCapReceive, eRole);
		BYTE bIsOtherMediaOn = other.IsMediaOn(dataType, cmCapReceive, eRole);
		if (bIsMediaOn && bIsOtherMediaOn)
		{
			const CMediaModeH323& rMediaModeH323      = GetMediaMode(dataType, cmCapReceive, eRole);
			const CMediaModeH323& rOtherMediaModeH323 = other.GetMediaMode(dataType, cmCapReceive, eRole);
			bRes &= rMediaModeH323.IsContaining(rOtherMediaModeH323, valuesToCompare, pDetails);
		}
		else if (bIsMediaOn || bIsOtherMediaOn)
			bRes = FALSE;
	}
	if (bRes && (direction & cmCapTransmit))
	{
		BYTE bIsMediaOn      = IsMediaOn(dataType, cmCapTransmit, eRole);
		BYTE bIsOtherMediaOn = other.IsMediaOn(dataType, cmCapTransmit, eRole);
		if (bIsMediaOn && bIsOtherMediaOn)
		{
			const CMediaModeH323& rMediaModeH323      = GetMediaMode(dataType, cmCapTransmit, eRole);
			const CMediaModeH323& rOtherMediaModeH323 = other.GetMediaMode(dataType, cmCapTransmit, eRole);
			bRes &= rMediaModeH323.IsContaining(rOtherMediaModeH323, valuesToCompare, pDetails);
		}
		else if (bIsMediaOn || bIsOtherMediaOn)
			bRes = FALSE;
	}
	return bRes;
}

///////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsMediaContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD* pDetails,
									 cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole) const
{
	BYTE bRes = TRUE;
	if (direction & cmCapReceive)
	{
		BYTE bIsMediaOn = IsMediaOn(dataType, cmCapReceive, eRole);
		if (bIsMediaOn)
		{
			const CMediaModeH323& rMediaModeH323 = GetMediaMode(dataType, cmCapReceive, eRole);
			bRes &= rMediaModeH323.IsContaining(other, valuesToCompare, pDetails);
		}
		else
			bRes = FALSE;
	}
	if (bRes && (direction & cmCapTransmit))
	{
		BYTE bIsMediaOn = IsMediaOn(dataType, cmCapTransmit, eRole);
		if (bIsMediaOn)
		{
			const CMediaModeH323& rMediaModeH323 = GetMediaMode(dataType, cmCapTransmit, eRole);
			bRes &= rMediaModeH323.IsContaining(other, valuesToCompare, pDetails);
		}
		else
			bRes = FALSE;
	}
	return bRes;
}


///////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsMediaEquals(const CComModeH323& other,cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole) const
{
	BYTE bRes = TRUE;
	if (direction & cmCapReceive)
	{
		const CMediaModeH323& rMediaModeH323 = GetMediaMode(dataType, cmCapReceive, eRole);
		const CMediaModeH323& rOtherMediaModeH323 = other.GetMediaMode(dataType, cmCapReceive, eRole);
		bRes &= (rMediaModeH323 == rOtherMediaModeH323);
	}
	if (bRes && (direction & cmCapTransmit))
	{
		const CMediaModeH323& rMediaModeH323 = GetMediaMode(dataType, cmCapTransmit, eRole);
		const CMediaModeH323& rOtherMediaModeH323 = other.GetMediaMode(dataType, cmCapTransmit, eRole);
		bRes &= (rMediaModeH323 == rOtherMediaModeH323);
	}
	return bRes;
}
///////////////////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsNewContentModeDiffersOnlyInRes(const CComModeH323& other, cmCapDirection direction) const
{
	BYTE bIsDiffMpiORResFromCurMode = FALSE;
	BYTE bIsDiffMpiORResFromCurModeRcv = FALSE;
	BYTE bIsDiffMpiORResFromCurModeTx = FALSE;

	if(direction & cmCapReceive && IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation))
	{
		const CMediaModeH323& rcvCurMode  = GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
		const CMediaModeH323& rcvNewMode  = other.GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
		DWORD currContentRateRcv = GetContentBitRate(cmCapReceive);
		DWORD newContentRateRcv  = other.GetContentBitRate(cmCapReceive);
		CapEnum curContentProtocolRcv = ((CapEnum)GetMediaType(cmCapVideo, cmCapReceive,kRoleContentOrPresentation));
		CapEnum newContentProtocolRcv = ((CapEnum)other.GetMediaType(cmCapVideo, cmCapReceive,kRoleContentOrPresentation));
		if(curContentProtocolRcv== eH264CapCode && curContentProtocolRcv == newContentProtocolRcv && currContentRateRcv == newContentRateRcv)
		{
		    bIsDiffMpiORResFromCurModeRcv = (!(rcvCurMode == rcvNewMode));
			if(bIsDiffMpiORResFromCurModeRcv)
			    PTRACE(eLevelInfoNormal,"CComModeH323::IsNewContentModeDiffersOnlyInRes, Diff Mpi Or Res in Rcv new Content mode");
		}
	}

	if(direction & cmCapTransmit && IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
	{

		const CMediaModeH323& transCurMode = GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
		const CMediaModeH323& tansNewMode  =other.GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
		DWORD currContentRateTx = GetContentBitRate(cmCapTransmit);
		DWORD newContentRateTx  = other.GetContentBitRate(cmCapTransmit);
		CapEnum curContentProtocolTx = ((CapEnum)GetMediaType(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation));
		CapEnum newContentProtocolTx = ((CapEnum)other.GetMediaType(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation));

		if(curContentProtocolTx== eH264CapCode && curContentProtocolTx == newContentProtocolTx && currContentRateTx == newContentRateTx)
		{
			bIsDiffMpiORResFromCurModeTx = (!(transCurMode == tansNewMode));
			if(bIsDiffMpiORResFromCurModeTx)
				PTRACE(eLevelInfoNormal,"CCComModeH323::IsNewContentModeDiffersOnlyInRes, Diff Mpi Or Res in Tx new Content mode Name - ");
		}
	}
	 if(bIsDiffMpiORResFromCurModeTx || bIsDiffMpiORResFromCurModeRcv)
		 bIsDiffMpiORResFromCurMode = TRUE;


	return bIsDiffMpiORResFromCurMode;
}


///////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsMediaEquals(const CBaseCap& other,DWORD valuesToCompare,cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole) const
{
	BYTE bRes = TRUE;
	if (direction & cmCapReceive)
	{
		const CMediaModeH323& rMediaMode = GetMediaMode(dataType, cmCapReceive, eRole);
		bRes &= rMediaMode.IsEquals(other,valuesToCompare);
	}
	if (bRes && (direction & cmCapTransmit))
	{
		const CMediaModeH323& rMediaMode = GetMediaMode(dataType, cmCapTransmit, eRole);
		bRes &= rMediaMode.IsEquals(other,valuesToCompare);
	}
	return bRes;
}

///////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsSdesMediaEquals(const CComModeH323& other,cmCapDataType dataType, cmCapDirection direction,ERoleLabel eRole) const
{
	BYTE bRes = TRUE;
	if (direction & cmCapReceive)
	{
		BYTE bIsMediaOn      = IsMediaOn(dataType, cmCapReceive, eRole);
		BYTE bIsOtherMediaOn = other.IsMediaOn(dataType, cmCapReceive, eRole);
		if (bIsMediaOn && bIsOtherMediaOn)
		{
			const CMediaModeH323& rMediaModeH323      = GetMediaMode(dataType, cmCapReceive, eRole);
			const CMediaModeH323& rOtherMediaModeH323 = other.GetMediaMode(dataType, cmCapReceive, eRole);
			bRes &= rMediaModeH323.IsSdesEquals(rOtherMediaModeH323);
		}
		else if (bIsMediaOn || bIsOtherMediaOn)
			bRes = FALSE;
	}
	if (bRes && (direction & cmCapTransmit))
	{
		BYTE bIsMediaOn      = IsMediaOn(dataType, cmCapTransmit, eRole);
		BYTE bIsOtherMediaOn = other.IsMediaOn(dataType, cmCapTransmit, eRole);
		if (bIsMediaOn && bIsOtherMediaOn)
		{
			const CMediaModeH323& rMediaModeH323      = GetMediaMode(dataType, cmCapTransmit, eRole);
			const CMediaModeH323& rOtherMediaModeH323 = other.GetMediaMode(dataType, cmCapTransmit, eRole);
			bRes &= rMediaModeH323.IsSdesEquals(rOtherMediaModeH323);
		}
		else if (bIsMediaOn || bIsOtherMediaOn)
			bRes = FALSE;
	}
	return bRes;
}
///////////////////////////////////////////////////////////////////////////////
const CMediaModeH323& CComModeH323::GetMediaMode(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole) const
{
	if (eRole == kRolePeople)
	{
		switch(type)
		{
		case cmCapAudio:
			if (direction == cmCapTransmit)
				return m_audModeXmit;
			else if (direction == cmCapReceive)
				return m_audModeRcv;
			break;
		case cmCapVideo:
			if (direction == cmCapTransmit)
				return m_vidModeXmit;
			else if (direction == cmCapReceive)
				return m_vidModeRcv;
			break;
		case cmCapData:
			if (direction == cmCapTransmit)
				return m_dataModeXmit;
			else if (direction == cmCapReceive)
				return m_dataModeRcv;
			break;
		case cmCapBfcp:
			if (direction == cmCapTransmit)
				return m_bfcpModeXmit;
			else if (direction == cmCapReceive)
				return m_bfcpModeRcv;
			break;

		default:
			break;
		}
	}
	else if (eRole & kRoleContentOrPresentation)
	{
		if (type == cmCapVideo)
        {
			if (direction == cmCapTransmit)
				return m_vidContModeXmit;
			else if (direction == cmCapReceive)
				return m_vidContModeRcv;
        }

	}
	// There was some error in the function parameters
	// therefore the return value will be arbitrarily m_audModeRcv!
	PASSERT(TRUE);
	PTRACE(eLevelInfoNormal, "CComModeH323::GetMediaMode: Direction is not valid");
	return m_audModeRcv;
}

///////////////////////////////////////////////////////////////////////////////
CMediaModeH323& CComModeH323::GetMediaMode(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	if (eRole == kRolePeople)
	{
		switch(type)
		{
		case cmCapAudio:
			if (direction == cmCapTransmit)
				return m_audModeXmit;
			else if (direction == cmCapReceive)
				return m_audModeRcv;
			break;
		case cmCapVideo:
			if (direction == cmCapTransmit)
				return m_vidModeXmit;
			else if (direction == cmCapReceive)
				return m_vidModeRcv;
			break;
		case cmCapData:
			if (direction == cmCapTransmit)
				return m_dataModeXmit;
			else if (direction == cmCapReceive)
				return m_dataModeRcv;
			break;
		case cmCapBfcp:
			if (direction == cmCapTransmit)
				return m_bfcpModeXmit;
			else if (direction == cmCapReceive)
				return m_bfcpModeRcv;
			break;
		default:;
		}
	}
	else if (eRole & kRoleContentOrPresentation)
	{
		if (type == cmCapVideo) {
			if (direction == cmCapTransmit)
				return m_vidContModeXmit;
			else if (direction == cmCapReceive)
				return m_vidContModeRcv;
		}
	}
	// There was some error in the function parameters
	// therefore the return value will be arbitrarily m_audModeRcv!
	PASSERT(TRUE);
	PTRACE(eLevelInfoNormal, "CComModeH323::GetMediaMode: Direction is not valid");
	return m_audModeRcv;
}

/////////////////////////////////////////////////////////////////////////////
WORD CComModeH323::GetMediaType(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole) const
{
    const CMediaModeH323& rMediaModeH323 = GetMediaMode(type,direction,eRole);
	return rMediaModeH323.GetType();
}

/////////////////////////////////////////////////////////////////////////////
APIS16 CComModeH323::GetFrameRate(EFormat eFormat,cmCapDirection direction,ERoleLabel eRole) const
{
	APIS16 res = -1;
	const CVidModeH323& rVidModeH323 = (const CVidModeH323 &)GetMediaMode(cmCapVideo, direction, eRole);
	res = rVidModeH323.GetFrameRate(eFormat);
	return res;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CComModeH323::GetFrameRateForRTV(cmCapDirection direction,ERoleLabel eRole) const
{
	DWORD res = -1;

	const CVidModeH323& rVidModeH323 = (const CVidModeH323 &)GetMediaMode(cmCapVideo, direction, eRole);

	if((CapEnum)(rVidModeH323.GetType()) == eRtvCapCode)
	{
		res = rVidModeH323.GetFrameRateForRTV();
		return res;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
APIS8 CComModeH323::GetFormatMpi(EFormat eFormat,cmCapDirection direction,ERoleLabel eRole) const
{
	APIS8 res = -1;
	const CVidModeH323& rVidModeH323 = (const CVidModeH323 &)GetMediaMode(cmCapVideo, direction, eRole);
	res = rVidModeH323.GetFormatMpi(eFormat);
	return res;
}

/////////////////////////////////////////////////////////////////////////////
EFormat CComModeH323::GetVideoFormat(cmCapDirection direction,ERoleLabel eRole) const
{
	EFormat res = kUnknownFormat;
	const CVidModeH323& rVidModeH323 = (const CVidModeH323 &)GetMediaMode(cmCapVideo, direction, eRole);
	res = rVidModeH323.GetFormat();
	return res;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CComModeH323::GetTotalBitRate(cmCapDirection direction) const
{
	const CAudModeH323& rAudModeH323  = (const CAudModeH323 &)GetMediaMode(cmCapAudio, direction);
	const CVidModeH323& rVidModeH323  = (const CVidModeH323 &)GetMediaMode(cmCapVideo, direction);

    DWORD totalRate = 0;

    if (rAudModeH323.IsModeOff() == FALSE)
    {
        DWORD audRate = 0;
        audRate = rAudModeH323.GetBitRate();
        totalRate += audRate*1000;
	}

	if (rVidModeH323.IsModeOff() == FALSE)
	{
		DWORD vidRate = 0;
		vidRate		  = rVidModeH323.GetBitRate();

		// in case of VSW we take the minimum between the constraint (flow control) and video rate (OLC)
		if ((kVideoSwitch == m_eConfType || kVSW_Fixed == m_eConfType) && (m_flowControlRateConstraint != 0))
		{
			vidRate = min(vidRate, m_flowControlRateConstraint);
		}

		totalRate	  +=(vidRate*100);
	}

    return totalRate;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CComModeH323::GetOngoingCallBitRate() const
{
	DWORD audioRcvRate    = GetMediaBitRate(cmCapAudio,cmCapReceive)*_K_;
	DWORD videoRcvRate    = GetMediaBitRate(cmCapVideo,cmCapReceive)*100;
	DWORD videoXmitRate   = GetMediaBitRate(cmCapVideo,cmCapTransmit)*100;
	DWORD contentRcvRate  = GetMediaBitRate(cmCapVideo,cmCapReceive,kRoleContentOrPresentation)*100; //the transmit isn't always updated, so we can't regard it
    DWORD totalRate = audioRcvRate + min(videoRcvRate,videoXmitRate) + contentRcvRate;
	return totalRate;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::CopyMediaData(BYTE data[],cmCapDataType type,cmCapDirection direction,ERoleLabel eRole) const
{
	const CMediaModeH323& rMediaModeH323  = GetMediaMode(type,direction,eRole);
    rMediaModeH323.CopyData(data);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::CopyMediaToCapBuffer(capBuffer* pCapBuffer,cmCapDataType type,cmCapDirection direction,ERoleLabel eRole,BYTE bOperUse) const
{
	const CMediaModeH323& rMediaModeH323  = GetMediaMode(type,direction,eRole);
    rMediaModeH323.CopyToCapBuffer(pCapBuffer,bOperUse);
}

/////////////////////////////////////////////////////////////////////////////
CBaseCap* CComModeH323::GetMediaAsCapClass(cmCapDataType type, cmCapDirection direction,ERoleLabel eRole) const
{
	const CMediaModeH323& rMediaModeH323  = GetMediaMode(type, direction,eRole);
    CBaseCap* pCapClass = rMediaModeH323.GetAsCapClass();
	return pCapClass;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::CopyMediaMode(const CComModeH323& newComMode,
					  cmCapDataType type, cmCapDirection direction,ERoleLabel eRole)
{
	if (direction & cmCapReceive)
	{
		const CMediaModeH323& rMediaMode = newComMode.GetMediaMode(type,cmCapReceive,eRole);
		SetMediaMode(rMediaMode,type,cmCapReceive,eRole);
	}
	if (direction & cmCapTransmit)
	{
		const CMediaModeH323& rMediaMode = newComMode.GetMediaMode(type,cmCapTransmit,eRole);
		SetMediaMode(rMediaMode,type,cmCapTransmit,eRole);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::CopyMediaModeToOppositeDirection(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	if (direction != cmCapReceiveAndTransmit)
	{
		cmCapDirection opposite = (direction == cmCapReceive)? cmCapTransmit: cmCapReceive;
		const CMediaModeH323& rMediaMode = GetMediaMode(type,direction,eRole);
		SetMediaMode(rMediaMode,type,opposite,eRole);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetMediaMode(const capBuffer* newMediaMode,
								cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, bool aKeepStreams)
{
    const std::list <StreamDesc> streamsDescList = GetStreamsListForMediaMode(type,direction,eRole);

	switch(type)
	{
	case cmCapAudio:
		if (direction & cmCapReceive)
			m_audModeRcv.Create(newMediaMode);
		if (direction & cmCapTransmit)
			m_audModeXmit.Create(newMediaMode);
		break;
	case cmCapVideo:
		if (eRole & kRoleContentOrPresentation)
		{
			if (direction & cmCapReceive)
				m_vidContModeRcv.Create(newMediaMode);
			if (direction & cmCapTransmit)
				m_vidContModeXmit.Create(newMediaMode);
		}
		else
		{
			if (direction & cmCapReceive)
				m_vidModeRcv.Create(newMediaMode);
			if (direction & cmCapTransmit)
				m_vidModeXmit.Create(newMediaMode);
		}
		break;
	case cmCapData:
		if (direction & cmCapReceive)
			m_dataModeRcv.Create(newMediaMode);
		if (direction & cmCapTransmit)
			m_dataModeXmit.Create(newMediaMode);
		break;
	case cmCapBfcp:
		if (direction & cmCapReceive)
			m_bfcpModeRcv.Create(newMediaMode);
		if (direction & cmCapTransmit)
			m_bfcpModeXmit.Create(newMediaMode);
		break;

	default:
		PTRACE(eLevelInfoNormal,"CComModeH323::SetMediaMode: cmCapDataType parameter is wrong");
	}

    if (aKeepStreams == true)
    {
        SetStreamsListForMediaMode(streamsDescList, type, direction, eRole);
    }

}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetMediaMode(const CMediaModeH323& newMediaMode,
								cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, bool aKeepStreams)
{
    const std::list <StreamDesc> streamsDescList = GetStreamsListForMediaMode(type,direction,eRole);

	switch(type)
	{
	case cmCapAudio:
		if (direction & cmCapReceive)
			m_audModeRcv  = (const CAudModeH323 &)newMediaMode;
		if (direction & cmCapTransmit)
			m_audModeXmit = (const CAudModeH323 &)newMediaMode;
		break;
	case cmCapVideo:
		if (eRole & kRoleContentOrPresentation)
		{
			if (direction & cmCapReceive)
				m_vidContModeRcv  = (const CVidModeH323 &)newMediaMode;
			if (direction & cmCapTransmit)
				m_vidContModeXmit = (const CVidModeH323 &)newMediaMode;
		}
		else
		{
			if (direction & cmCapReceive)
				m_vidModeRcv  = (const CVidModeH323 &)newMediaMode;
			if (direction & cmCapTransmit)
				m_vidModeXmit = (const CVidModeH323 &)newMediaMode;
		}
		break;
	case cmCapData:
		if (direction & cmCapReceive)
			m_dataModeRcv  = (const CDataModeH323 &)newMediaMode;
		if (direction & cmCapTransmit)
			m_dataModeXmit = (const CDataModeH323 &)newMediaMode;
		break;
	case cmCapBfcp:
		if (direction & cmCapReceive)
			m_bfcpModeRcv  = (const CBfcpModeH323 &)newMediaMode;
		if (direction & cmCapTransmit)
			m_bfcpModeXmit = (const CBfcpModeH323 &)newMediaMode;
		break;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
	}

    if (aKeepStreams == true)
    {
        SetStreamsListForMediaMode(streamsDescList, type, direction, eRole);
    }

}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetMediaMode(const CBaseCap *pNewMediaMode,
								cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, bool aKeepStreams)
{
	if (pNewMediaMode)
	{
		CapEnum eCapCode	= pNewMediaMode->GetCapCode();
		int    length		= pNewMediaMode->SizeOf();
		BYTE* data			= (BYTE *)pNewMediaMode->GetStruct();

		SetMediaMode(eCapCode,length,data,type,direction,eRole, aKeepStreams);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetMediaMode(WORD newType,WORD newDataLength,const BYTE newData[],
								cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, bool aKeepStreams)
{
    const std::list <StreamDesc> streamsDescList = GetStreamsListForMediaMode(type,direction,eRole);

    switch(type)
	{
	case cmCapAudio:
		if (direction & cmCapReceive)
			m_audModeRcv.Create(newType, newDataLength, newData);
		if (direction & cmCapTransmit)
			m_audModeXmit.Create(newType, newDataLength, newData);
		break;
	case cmCapVideo:
		if (eRole & kRoleContentOrPresentation)
		{
			if (direction & cmCapReceive)
				m_vidContModeRcv.Create(newType, newDataLength, newData);
			if (direction & cmCapTransmit)
				m_vidContModeXmit.Create(newType, newDataLength, newData);
		}
		else
		{
			if (direction & cmCapReceive)
				m_vidModeRcv.Create(newType, newDataLength, newData);
			if (direction & cmCapTransmit)
				m_vidModeXmit.Create(newType, newDataLength, newData);
		}
		break;
	case cmCapData:
		if (direction & cmCapReceive)
			m_dataModeRcv.Create(newType, newDataLength, newData);
		if (direction & cmCapTransmit)
			m_dataModeXmit.Create(newType, newDataLength, newData);
		break;
	case cmCapBfcp:
		if (direction & cmCapReceive)
			m_bfcpModeRcv.Create(newType, newDataLength, newData);
		if (direction & cmCapTransmit)
			m_bfcpModeXmit.Create(newType, newDataLength, newData);
		break;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
	}

    if (aKeepStreams == true)
    {
        SetStreamsListForMediaMode(streamsDescList, type, direction, eRole);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SwitchMediaDirections()
{
	// copy modes into temp objects
	CAudModeH323	audModeRcv	= m_audModeRcv;
	CVidModeH323	vidModeRcv	= m_vidModeRcv;
	CVidModeH323	contModeRcv = m_vidContModeRcv;
	CDataModeH323	dataModeRcv = m_dataModeRcv;
	CBfcpModeH323	bfcpModeRcv = m_bfcpModeRcv;

	m_audModeRcv		= m_audModeXmit;
	m_vidModeRcv		= m_vidModeXmit;
	m_vidContModeRcv	= m_vidContModeXmit;
	m_dataModeRcv		= m_dataModeXmit;
	m_bfcpModeRcv		= m_bfcpModeXmit;

	m_audModeXmit		= audModeRcv;
	m_vidModeXmit		= vidModeRcv;
	m_vidContModeXmit	= contModeRcv;
	m_dataModeXmit		= dataModeRcv;
	m_bfcpModeXmit		= bfcpModeRcv;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetAllModesOff(cmCapDirection direction)
{
    if (direction & cmCapReceive)
    {
        m_audModeRcv.SetModeOff();
        m_vidModeRcv.SetModeOff();
		m_vidContModeRcv.SetModeOff();
        m_dataModeRcv.SetModeOff();
        m_bfcpModeRcv.SetModeOff();
    }
    if (direction & cmCapTransmit)
    {
        m_audModeXmit.SetModeOff();
        m_vidModeXmit.SetModeOff();
		m_vidContModeXmit.SetModeOff();
		m_dataModeXmit.SetModeOff();
		m_bfcpModeXmit.SetModeOff();
    }
}

///////////////////////////////////////////////////////////////////////////
void CComModeH323::SetAudioFramePerPacket(int maxFramePP,int minFramePP,cmCapDirection direction)
{
    if (direction & cmCapReceive)
        m_audModeRcv.SetFramePP(maxFramePP,minFramePP);
    if (direction & cmCapTransmit)
      m_audModeXmit.SetFramePP(maxFramePP,minFramePP);
    else
        PTRACE(eLevelError,"CComModeH323::SetAudioFramePerPacke: Can't set value. audio is off.");
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetAudioWithDSHforAvMcu()
{
    m_audModeRcv.SetDSHforAvMcu();
    m_audModeXmit.SetDSHforAvMcu();
}


///////////////////////////////////////////////////////////////////////////
void CComModeH323::SetVideoBitRate(int newBitRate, cmCapDirection direction,ERoleLabel eRole)
{
	if (eRole & kRoleContentOrPresentation)
	{
		if (direction & cmCapReceive)
			m_vidContModeRcv.SetBitRate(newBitRate);
		if (direction & cmCapTransmit)
			m_vidContModeXmit.SetBitRate(newBitRate);
	}
	else
	{
		if (direction & cmCapReceive)
			m_vidModeRcv.SetBitRate(newBitRate);
		if (direction & cmCapTransmit)
			m_vidModeXmit.SetBitRate(newBitRate);
	}
}
////////////////////////////////////////////////////////////////////////////////
DWORD CComModeH323::GetVideoBitRate(cmCapDirection direction,ERoleLabel eRole) const
{
	if (eRole & kRoleContentOrPresentation)
	{
		if (direction & cmCapReceive)
			return (m_vidContModeRcv.GetBitRate());
		if (direction & cmCapTransmit)
			return(m_vidContModeXmit.GetBitRate());
	}
	else
	{
		if (direction & cmCapReceive)
			return (m_vidModeRcv.GetBitRate());
		if (direction & cmCapTransmit)
			return (m_vidModeXmit.GetBitRate());
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetDataBitRate(int newBitRate, cmCapDirection direction)
{
	if (direction & cmCapReceive)
		m_dataModeRcv.SetBitRate(newBitRate);
	if (direction & cmCapTransmit)
		m_dataModeXmit.SetBitRate(newBitRate);
}

///////////////////////////////////////////////////////////////////////////
void CComModeH323::SetFormatMpi(EFormat eFormat, int mpi, cmCapDirection direction, ERoleLabel eRole)
{
	if (eRole == kRolePeople)
	{
		if (direction & cmCapReceive)
			m_vidModeRcv.SetFormatMpi(eFormat,mpi);
		if (direction & cmCapTransmit)
			m_vidModeXmit.SetFormatMpi(eFormat,mpi);
	}
	else if (eRole & kRoleContentOrPresentation)
	{
		if (direction & cmCapReceive)
			m_vidContModeRcv.SetFormatMpi(eFormat,mpi);
		if (direction & cmCapTransmit)
			m_vidContModeXmit.SetFormatMpi(eFormat,mpi);
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsSupportErrorCompensation(cmCapDirection direction) const
{
	BYTE bRes = FALSE;
	const CVidModeH323& rVidModeH323 = (const CVidModeH323 &)GetMediaMode(cmCapVideo,direction,kRolePeople);
	bRes = rVidModeH323.IsSupportErrorCompensation();
	return bRes;
}

////////////////////////////////////////////////////////////////////////////
/*
BYTE CComModeH323::IsVidParamSupportedInCP(int maxPossibleFormatsMpi[kSIF + 1], cmCapDirection direction) const
{
	BYTE bRes = TRUE;
	const CVidModeH323& rVidModeH323 = (const CVidModeH323 &)GetMediaMode(cmCapVideo,direction,kRolePeople);
	bRes = rVidModeH323.IsVidParamSupportedInCP(maxPossibleFormatsMpi);
	return bRes;
}
*/

////////////////////////////////////////////////////////////////////////////
DWORD CComModeH323::GetMediaBitRate(cmCapDataType type, cmCapDirection direction,ERoleLabel eRole) const
{
	const CMediaModeH323& rMediaModeH323 = GetMediaMode(type,direction,eRole);
	return rMediaModeH323.GetBitRate();
}

////////////////////////////////////////////////////////////////////////////
CComModeH323& CComModeH323::operator=(const CComModeH323& other)
{
	if(this != &other)
	{
	m_audModeRcv  = other.m_audModeRcv;
	m_audModeXmit = other.m_audModeXmit;

	m_vidModeRcv  = other.m_vidModeRcv;
	m_vidModeXmit = other.m_vidModeXmit;

	m_vidContModeRcv  = other.m_vidContModeRcv;
	m_vidContModeXmit = other.m_vidContModeXmit;

	m_dataModeRcv  = other.m_dataModeRcv;
	m_dataModeXmit = other.m_dataModeXmit;

	m_bfcpModeRcv  = other.m_bfcpModeRcv;
	m_bfcpModeXmit = other.m_bfcpModeXmit;

    m_eConfType	   = other.m_eConfType;
    m_confMediaType    = other.m_confMediaType;
    m_callRate	   = other.m_callRate;
    m_TotalVideoRate = other.m_TotalVideoRate;

 	m_isEncrypted  = other.m_isEncrypted;
		m_encMediaTypeAlg   = other.m_encMediaTypeAlg;
		m_halfKeyType  		= other.m_halfKeyType;
		m_bDisconnectOnEncryptionFailure  = other.m_bDisconnectOnEncryptionFailure;

	m_bIsHd720Enabled = other.m_bIsHd720Enabled;
	m_bIsHd1080Enabled = other.m_bIsHd1080Enabled;
	m_bIsHd720At60Enabled = other.m_bIsHd720At60Enabled;
	m_bIsHd1080At60Enabled = other.m_bIsHd1080At60Enabled;

	m_flowControlRateConstraint = other.m_flowControlRateConstraint;
	m_flowControlRateConstraintForPresentation = other.m_flowControlRateConstraintForPresentation;

	m_isLpr				= other.m_isLpr;

	//LYNC2013_FEC_RED:
	m_isFec				= other.m_isFec;
	m_isRed				= other.m_isRed;

	m_contentProtocol	= other.m_contentProtocol;
       m_declareContentRate= other.m_declareContentRate;

	m_bShowContentAsVideo = other.m_bShowContentAsVideo;
	m_copTxLevel		= other.m_copTxLevel;
	m_partyId           = other.m_partyId;
	m_eTipMode			= other.m_eTipMode;// TIP
	m_TipAuxFPS			= other.m_TipAuxFPS;
	m_eTipContentMode   = other.m_eTipContentMode;
    m_operationPoints   = other.m_operationPoints;
 	m_isDtlsEncrypted  	= other.m_isDtlsEncrypted;
 	m_bDtlsAvailable	= other.m_bDtlsAvailable;
 	m_eOPPreset			= other.m_eOPPreset;
		m_eIsUseOperationPointesPresets=other.m_eIsUseOperationPointesPresets;
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
WORD operator== (const CComModeH323& first,const CComModeH323& second)
{
	WORD rval = 0;

	if ((first.m_audModeRcv   == second.m_audModeRcv)  &&
		(first.m_audModeXmit  == second.m_audModeXmit) &&
		(first.m_vidModeRcv   == second.m_vidModeRcv)  &&
		(first.m_vidModeXmit  == second.m_vidModeXmit) &&
		(first.m_vidContModeRcv  == second.m_vidContModeRcv)  &&
		(first.m_vidContModeXmit == second.m_vidContModeXmit) &&
		(first.m_dataModeRcv  == second.m_dataModeRcv) &&
		(first.m_dataModeXmit == second.m_dataModeXmit) &&
		(first.m_bfcpModeRcv  == second.m_bfcpModeRcv) &&
		(first.m_bfcpModeXmit == second.m_bfcpModeXmit))


		rval = 1;

	return rval;
}

/////////////////////////////////////////////////////////////////////////////
WORD operator!= (const CComModeH323& first,const CComModeH323& second)
{
	return !(first == second);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetMediaOff(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole)
{
	if (eRole == kRolePeople)
	{
		switch(type)
		{
		case cmCapAudio:
			if (direction & cmCapReceive)
				m_audModeRcv.SetModeOff();
			if (direction & cmCapTransmit)
				m_audModeXmit.SetModeOff();
			break;
		case cmCapVideo:
			if (direction & cmCapReceive)
				m_vidModeRcv.SetModeOff();
			if (direction & cmCapTransmit)
				m_vidModeXmit.SetModeOff();
			break;
		case cmCapData:
			if (direction & cmCapReceive)
				m_dataModeRcv.SetModeOff();
			if (direction & cmCapTransmit)
				m_dataModeXmit.SetModeOff();
			break;
		case cmCapBfcp:
			if (direction & cmCapReceive)
				m_bfcpModeRcv.SetModeOff();
			if (direction & cmCapTransmit)
				m_bfcpModeXmit.SetModeOff();
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
		}
	}
	else if (eRole & kRoleContentOrPresentation)
	{
		if (type == cmCapVideo)
		{
			if (direction & cmCapReceive)
				m_vidContModeRcv.SetModeOff();
			if (direction & cmCapTransmit)
				m_vidContModeXmit.SetModeOff();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsMediaOff(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole) const
{
    BYTE bRes = TRUE;
	
	if (direction & cmCapReceive)
	{
		const CMediaModeH323& rMediaModeH323 = GetMediaMode(type, cmCapReceive, eRole);
	    bRes &= rMediaModeH323.IsModeOff();
	}
	
	if (direction & cmCapTransmit)
	{
		const CMediaModeH323& rMediaModeH323 = GetMediaMode(type, cmCapTransmit, eRole);
	    bRes &= rMediaModeH323.IsModeOff();
	}
	
	return bRes;
}

///////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsMediaOn(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole)  const
{
	if(eRole == kRoleContent)//kRoleContentOrPresentation
		PTRACE(eLevelInfoNormal,"CComModeH323::IsMediaOn - asking for rule content");

	BYTE bRes = TRUE;
	if (direction & cmCapReceive)
	{
		const CMediaModeH323& rMediaModeH323 = GetMediaMode(type, cmCapReceive, eRole);
	    bRes &= rMediaModeH323.IsMediaOn();
	}
	if (direction & cmCapTransmit)
	{
		const CMediaModeH323& rMediaModeH323 = GetMediaMode(type, cmCapTransmit, eRole);
	    bRes &= rMediaModeH323.IsMediaOn();
	}
	return bRes;
//	return !IsMediaOff(type, direction, eRole);
}

/////////////////////////////////////////////////////////////////////////////
//void  CComModeH323::SetConfType(const CComMode &h320ComMode, BYTE bIsSoftCp,BYTE bIsQuad)
void  CComModeH323::SetConfType(BYTE bIsFreeVideoRate, BYTE bIsSoftCp, BYTE bIsQuad)
{
	if (bIsFreeVideoRate)
		m_eConfType = kCp;
	else if (bIsSoftCp)
		m_eConfType = kSoftCp;
	else if(bIsQuad)
		m_eConfType = kCpQuad;
	else
		m_eConfType = kVideoSwitch;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsVidModeRcvAndXmitMediaOn()
{
	BYTE bRes = FALSE;
	if (m_vidModeRcv.IsMediaOn() || m_vidModeXmit.IsMediaOn())
		bRes = TRUE;
	return bRes;
}


/////////////////////////////////////////////////////////////////////////////
void CComModeH323::Dump(const char* title, WORD level) const
{
	COstrStream msg;
    if(title != NULL)
        msg << title;
	Dump(msg);
	PTRACE(level,msg.str().c_str());
}


/////////////////////////////////////////////////////////////////////////////
void CComModeH323::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
    msg.setf(std::ios::showbase);

    this->m_operationPoints.Trace();
    msg << "\nPartyId: " << m_partyId << " ConfMediaType: " << ConfMediaTypeToString(m_confMediaType);

    //    msg << "\n--Audio Rcv:";
    m_audModeRcv.Dump(msg, "--Audio Rcv:");
//    msg << "\n--Audio Xmit:";
    m_audModeXmit.Dump(msg, "--Audio Xmit:");

	if (m_vidModeRcv.IsMediaOn())
	{
//		msg << "\n--Video Rcv:";
		m_vidModeRcv.Dump(msg, "--Video Rcv:");
	}
	if (m_vidModeXmit.IsMediaOn())
	{
//		msg << "\n--Video Xmit:";
		m_vidModeXmit.Dump(msg, "--Video Xmit:");
	}

	if (m_vidContModeRcv.IsMediaOn())
	{
//		msg << "\n--Video Content Rcv:";
		m_vidContModeRcv.Dump(msg, "--Video Content Rcv:");
	}
	if (m_vidContModeXmit.IsMediaOn())
	{
//		msg << "\n--Video Content Xmit:";
		m_vidContModeXmit.Dump(msg, "--Video Content Xmit:");
	}

	if (m_dataModeRcv.IsMediaOn())
	{
//		msg << "\n--Data Rcv:";
		m_dataModeRcv.Dump(msg, "--Data Rcv:");
	}
	if (m_dataModeXmit.IsMediaOn())
	{
//		msg << "\n--Data Xmit:";
		m_dataModeXmit.Dump(msg, "--Data Xmit:");
	}
	if (m_bfcpModeRcv.IsMediaOn())
	{
		enTransportType transportType = m_bfcpModeRcv.GetTransportType();

		if (transportType == eTransportTypeUdp)
			msg << "\n--BFCP Enabled - UDP";
		else if (transportType == eTransportTypeTcp)
			msg << "\n--BFCP Enabled - TCP";
		else
			msg << "\n--BFCP Enabled - Unknown type!!!";

		m_bfcpModeRcv.Dump(msg);
	}
	msg << "\n--Total video bit rate: " << m_TotalVideoRate;
	if (m_isLpr)
	{
		msg << "\n--LPR support";
	}
	if (m_isFec)
	{
		msg << "\n--FEC support";
	}
	if (m_isRed)
	{
		msg << "\n--RED support";
	}
	if (m_contentProtocol)
	{
		msg << "\n--Protocol mode is - " << m_contentProtocol;
	}
	msg << "\n--Declare content rate: " << m_declareContentRate;

	if(m_bShowContentAsVideo)
	{
		msg << "\n--Content as Video Enabled " ;
	}
	if(m_eConfType == kCop)
	{
		msg << "\n--Cop Tx level: " << (WORD)m_copTxLevel;
	}
	if (m_eTipMode)
	{
		msg << "\n--TIP mode: " << m_eTipMode;
	}
	if (m_TipAuxFPS != eTipAuxNone)
	{
		msg << "\n--" << ::GetTipAuxFPSStr(m_TipAuxFPS);
	}
	if (m_eTipContentMode)
	{
		msg << "\n--TIP Compatibility: " << (DWORD)m_eTipContentMode;
	}
	if (IsEncrypted())
	{
		msg << "\n--Encryption Enabled";
	}
	if (m_isDtlsEncrypted == Encryp_On)
	{
		msg << "\n--Dtls Encryption Enabled";
	}
	if (m_bDtlsAvailable)
	{
		msg << "\n--Dtls Encryption Available";
	}
	if (m_eOPPreset != eOPP_dummy)
	{
		msg << "\n--Operation points preset:" << m_eOPPreset;
	}
	if (eIsUseOPP_No != m_eIsUseOperationPointesPresets)
	{
		msg << "\n-- IsUseOperationPointesPresets: " << m_eIsUseOperationPointesPresets;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
WORD CComModeH323::GetChangeFromNewScmMode(CComModeH323* pNewScm,cmCapDirection direction,cmCapDataType dataType,ERoleLabel eRole) const
{
	DWORD details = 0;
	const CMediaModeH323 &rMediaModeH323 = GetMediaMode(dataType,direction,eRole);
	const CMediaModeH323 &newMediaModeH323 = pNewScm->GetMediaMode(dataType,direction,eRole);
	newMediaModeH323.IsContaining(rMediaModeH323,kBitRate|kFormat|kFrameRate|kH264Level|kH264Additional|kAnnexes,&details);

	return details;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::AddLowerResolutionsIfNeeded(cmCapDirection eDirection)
{
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.AddLowerResolutionsIfNeeded();
	if (eDirection & cmCapReceive)
		m_vidModeRcv.AddLowerResolutionsIfNeeded();
}


//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsContent(cmCapDirection eDirection) const
{
	BYTE bContent = FALSE;
	if (eDirection & cmCapTransmit)
	{
		bContent = m_vidContModeXmit.IsMediaOn();
		if(bContent == FALSE)
			return bContent;
	}

	if (eDirection & cmCapReceive)
	{
		bContent = m_vidContModeRcv.IsMediaOn();
	}
	return bContent;
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::RemoveContent(cmCapDirection eDirection)
{
	BYTE bRemove = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_vidContModeXmit.SetModeOff();
		bRemove = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		m_vidContModeRcv.SetModeOff();
		bRemove = TRUE;
	}
	return bRemove;
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::RemoveData(cmCapDirection eDirection)
{
	BYTE bRemove = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_dataModeXmit.SetModeOff();
		bRemove = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		m_dataModeRcv.SetModeOff();
		bRemove = TRUE;
	}
	return bRemove;
}
//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::SetHDContent(DWORD contentRate,cmCapDirection eDirection, EHDResolution eHDRes,BYTE HDMpi,BOOL isHighProfile)
{
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_vidContModeRcv.SetHDContent(contentRate,eHDRes,cmCapReceive,m_bShowContentAsVideo,HDMpi,isHighProfile);
		bSet = TRUE;
	}
	if (eDirection & cmCapReceive)
	{
		m_vidContModeXmit.SetHDContent(contentRate,eHDRes,cmCapTransmit,m_bShowContentAsVideo,HDMpi,isHighProfile);
		bSet = TRUE;
	}
	return bSet;

}
//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::SetTIPContent(DWORD contentRate, cmCapDirection eDirection, BYTE set264ModeAsTipContent)
{
	BYTE bSet = FALSE;

	if (eDirection & cmCapTransmit)
	{
		m_vidContModeXmit.SetTIPContent(contentRate, cmCapTransmit, set264ModeAsTipContent);
		bSet = TRUE;
	}
	if (eDirection & cmCapReceive)
	{
		m_vidContModeRcv.SetTIPContent(contentRate, cmCapReceive, set264ModeAsTipContent);
		bSet = TRUE;
	}

	if (set264ModeAsTipContent == FALSE)
		m_eTipContentMode = eTipCompatiblePreferTIP;
	else //just for TipCompatibility:video&content!
	{
		m_eTipContentMode = eTipCompatibleVideoAndContent;
	}

	return bSet;

}
//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::SetContent(DWORD contentRate, cmCapDirection eDirection, CapEnum Protocol, BOOL isHD1080, BYTE HDContentMpi, BOOL isHighProfile)
{
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_vidContModeRcv.SetContent(contentRate,Protocol,cmCapReceive,isHD1080,HDContentMpi,GetPartyMediaType(),isHighProfile);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		m_vidContModeXmit.SetContent(contentRate,Protocol,cmCapTransmit,isHD1080,HDContentMpi,GetPartyMediaType(),isHighProfile);
		bSet = TRUE;
	}

	m_eTipContentMode = eTipCompatibleNone;

	return bSet;
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::SetContentBitRate(DWORD contentRate, cmCapDirection eDirection)
{
	SetVideoBitRate(contentRate, eDirection, kRoleContentOrPresentation);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
DWORD  CComModeH323::GetContentBitRate(cmCapDirection eDirection) const
{
	DWORD contentRate = 0;
	contentRate = GetMediaBitRate(cmCapVideo, eDirection, kRoleContentOrPresentation);
	return contentRate;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetH264Scm(APIU16 profile, APIU8 level, long mbps, long fs, long dpb, long brAndCpb, long sar, long staticMB, cmCapDirection eDirection, APIU8 packatizationMode)
{
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_vidModeXmit.SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapTransmit, packatizationMode);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		m_vidModeRcv.SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive, packatizationMode);
		bSet = TRUE;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetVP8Scm(VP8VideoModeDetails VP8VideoDetails, cmCapDirection eDirection, APIS32 BitRate)
{//N.A. DEBUG VP8
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_vidModeXmit.SetVP8Scm(VP8VideoDetails, cmCapTransmit, BitRate);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		m_vidModeRcv.SetVP8Scm(VP8VideoDetails, cmCapReceive, BitRate);
		bSet = TRUE;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetSvcScm(APIU16 profile, APIU8 level, long mbps, long fs, long dpb, long brAndCpb, long sar, long staticMB, cmCapDirection eDirection)
{
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_vidModeXmit.SetSvcScm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		m_vidModeRcv.SetSvcScm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
		bSet = TRUE;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetSacScm(cmCapDirection eDirection)
{
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_audModeXmit.SetSacScm(cmCapTransmit);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		m_audModeRcv.SetSacScm(cmCapReceive);
		bSet = TRUE;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetIsSupportScp(BYTE bIsSupportScp)
{
	m_dataModeXmit.SetIsSupportScp(bIsSupportScp);
	m_dataModeRcv.SetIsSupportScp(bIsSupportScp);
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetOperationPointsAndRecvStreamsGroup(const CVideoOperationPointsSet* pOperationPoints)
{
    SetOperationPoints(pOperationPoints);

    CBaseCap* pTempCap = GetMediaAsCapClass(cmCapVideo, cmCapTransmit);
    CSvcVideoCap* pSvcCap  = (CSvcVideoCap* )pTempCap;
    if (pSvcCap)
    {
        pSvcCap->SetOperationPoints(pOperationPoints);

    }
    POBJDELETE(pSvcCap);

    pTempCap = GetMediaAsCapClass(cmCapVideo, cmCapReceive);
    pSvcCap  = (CSvcVideoCap* )pTempCap;
    if (pSvcCap)
    {
        pSvcCap->SetOperationPoints(pOperationPoints);
        pSvcCap->SetStreams(*(pSvcCap->GetOperationPoints()));
    }
    POBJDELETE(pSvcCap);
}
////////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateStreamsAndPayloadType(CSipCaps* pCapsSet, bool updatePayloadTypeOnly)
{
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		if (mediaType == cmCapBfcp)
			continue;

	   	for (int direction = cmCapReceive; direction < cmCapReceiveAndTransmit; direction++)
	   	{
	   		CMediaModeH323 &rMediaModeH323 = GetMediaMode(mediaType, (cmCapDirection)direction,eRole);
	   		CCapSetInfo capInfo = (CapEnum)rMediaModeH323.GetType();
	   		// find the specific cap in cap set
	   		CBaseCap *pCap = pCapsSet->GetCapSet(capInfo,0,eRole);
			if (pCap)
	    	{
				payload_en payload_type = pCapsSet->GetPayloadTypeByDynamicPreference(capInfo,pCap->GetProfile(),eRole);
				SetPayloadTypeForMediaMode(mediaType, (cmCapDirection)direction , eRole, payload_type);
				if (!updatePayloadTypeOnly)
				SetStreamsForMediaMode(pCap, mediaType,(cmCapDirection)direction , eRole,payload_type);
	    	    PDELETE(pCap);
	    	}

	    }
	}
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetStreamsForMediaMode(CBaseCap* pNewMediaMode, cmCapDataType type,cmCapDirection direction,ERoleLabel eRole,payload_en payload_type)
{
	if (pNewMediaMode)
	{
		CMediaModeH323 &rMediaModeH323 = GetMediaMode(type,direction,eRole);
		if (direction & cmCapReceive)
			rMediaModeH323.FillStreamsDescList(payload_type,pNewMediaMode->GetRecvStreamsGroup());
		else if (direction & cmCapTransmit)
			rMediaModeH323.FillStreamsDescList(payload_type,pNewMediaMode->GetSendStreamsGroup());
	}
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetPayloadTypeForMediaMode(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole, payload_en payload_type)
{
	CMediaModeH323 &rMediaModeH323 = GetMediaMode(type,direction,eRole);
	if (direction & cmCapReceive)
		rMediaModeH323.SetPayloadType(payload_type);
	else if (direction & cmCapTransmit)
		rMediaModeH323.SetPayloadType(payload_type);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetH264VideoParams(H264VideoModeDetails h264VidModeDetails, APIS32 sar, cmCapDirection direction, APIU8 packatizationMode)
{
	SetH264Scm(h264VidModeDetails.profileValue, h264VidModeDetails.levelValue, h264VidModeDetails.maxMBPS, h264VidModeDetails.maxFS, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR, sar, h264VidModeDetails.maxStaticMbps, direction, packatizationMode);
	if (h264VidModeDetails.videoModeType == eHD720Asymmetric)
		SetHd720Enabled(TRUE);
	else
		SetHd720Enabled(FALSE);

	if (h264VidModeDetails.videoModeType == eHD1080Asymmetric)
		SetHd1080Enabled(TRUE);
	else
		SetHd1080Enabled(FALSE);

	if (h264VidModeDetails.videoModeType == eHD1080At60Asymmetric)
		SetHd1080At60Enabled(TRUE);
	else
		SetHd1080At60Enabled(FALSE);

	if (h264VidModeDetails.videoModeType == eHD720At60Asymmetric)
		SetHd720At60Enabled(TRUE);
	else
		SetHd720At60Enabled(FALSE);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetVP8VideoParams(VP8VideoModeDetails vp8VideoDetails, cmCapDirection direction)
{//N.A. DEBUG VP8
	SetVP8Scm(vp8VideoDetails, direction);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetMsSvcScm(MsSvcVideoModeDetails MsSvcVidModeDetails,cmCapDirection eDirection,APIS32 BitRate)
{
	BYTE bSet = FALSE;
	PTRACE(eLevelInfoNormal,"CComModeH323::SetMsSvcScm ");
	if (eDirection & cmCapTransmit)
	{
			m_vidModeXmit.SetMsSvcScm(MsSvcVidModeDetails,cmCapTransmit,BitRate);
			bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		m_vidModeRcv.SetMsSvcScm(MsSvcVidModeDetails,cmCapReceive,BitRate);
		bSet = TRUE;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetRtvScm(APIS32 width, APIS32 Height,APIS32 FR ,cmCapDirection eDirection,APIS32 BitRate)
{
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_vidModeXmit.SetRtvScm(width, Height,FR,cmCapTransmit,BitRate);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		m_vidModeRcv.SetRtvScm(width, Height,FR,cmCapReceive,BitRate);
		bSet = TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetRtvVideoParams(RTVVideoModeDetails rtvVidModeDetails, cmCapDirection direction, APIS32 BitRate)
{
	SetRtvScm(rtvVidModeDetails.Width,rtvVidModeDetails.Height,rtvVidModeDetails.FR,direction, BitRate);

	if (rtvVidModeDetails.videoModeType == e_rtv_HD720Asymmetric)
		SetHd720Enabled(TRUE);
	else
		SetHd720Enabled(FALSE);

}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::GetRtvScm(long& mbps, long& fs, cmCapDirection eDirection) const
{
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.GetRtvScm(mbps, fs);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.GetRtvScm(mbps, fs);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::GetMsSvcScm(long& mbps, long& fs, cmCapDirection eDirection) const
{
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.GetMsSvcScm(mbps, fs);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.GetMsSvcScm(mbps, fs);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::GetH264Scm(APIU16& profile, APIU8& level, long& mbps, long& fs, long& dpb, long& brAndCpb, long& sar, long& staticMB, cmCapDirection eDirection) const
{
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsTIPContentEnableInH264Scm() const
{
	BOOL IsTIPContentEnable = FALSE;

	IsTIPContentEnable = m_vidContModeRcv.IsTIPContentEnableInH264Scm();
	if (IsTIPContentEnable)
		PTRACE(eLevelInfoNormal,"CComModeH323::IsTIPContentEnableInH264Scm TIPContent is Enable");

	return IsTIPContentEnable;

}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetScmToHdCp(BYTE hdResolution, cmCapDirection direction)
{
	int profile, level;
	long mbps, fs, dpb, brAndCpb, sar, staticMB;
	::GetH264VideoHdVswParam(profile, level, mbps, fs, dpb, brAndCpb, sar,staticMB, hdResolution);
	sar = H264_ALL_LEVEL_DEFAULT_SAR; // sar for cp is different than sar for vsw
	profile = H264_Profile_None; // save the exist profile
	SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, direction);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetScmToCpHD720At60(cmCapDirection direction)
{
	int level = H264_Level_3_1;
	long mbps = GetMaxMbpsAsDevision(H264_HD720_60_MBPS);
	long fs = -1;
	long dpb = -1;
	long brAndCpb = -1;
	long sar = H264_ALL_LEVEL_DEFAULT_SAR; // sar for cp is different than sar for vsw
	long staticMB = H264_ALL_LEVEL_DEFAULT_STATIC_MBPS; // sar for cp is different than sar for vsw
	APIU16 profile = H264_Profile_None; // save the exist profile
	SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, direction);
}

//////////////////////////////
void  CComModeH323::SetScmToCpHD1080At60(cmCapDirection direction)
{
	int level = H264_Level_3_1;
	// 1080p60debug - to be removed
	// long mbps = GetMaxMbpsAsDevision(H264_HD1080_60_MBPS);
	long mbps = GetMaxMbpsAsDevision(Get1080p60mbps());
	long fs = GetMaxFsAsDevision(H264_HD1080_FS);
	long dpb = -1;
	long brAndCpb = -1;
	long sar = H264_ALL_LEVEL_DEFAULT_SAR; // sar for cp is different than sar for vsw
	long staticMB = H264_ALL_LEVEL_DEFAULT_STATIC_MBPS; // sar for cp is different than sar for vsw
	APIU8 profile = H264_Profile_None; // save the exist profile
	SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, direction);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetSampleAspectRatio(APIS32 sar, cmCapDirection eDirection)
{
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.SetSampleAspectRatio(sar);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.SetSampleAspectRatio(sar);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetStaticMB(APIS32 staticMB, cmCapDirection eDirection)
{
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.SetStaticMB(staticMB);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.SetStaticMB(staticMB);
}

//////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType  CComModeH323::GetVideoPartyType(cmCapDirection eDirection, DWORD staticMB,BYTE IsRsrcByFs) const
{
	//check video open:
	if (IsMediaOff(cmCapVideo, eDirection))
		return eVideo_party_type_none;

	if (m_eConfType == kVideoSwitch || m_eConfType ==kVSW_Fixed)
	{
		return eVSW_video_party_type;
	}

	if (eDirection & cmCapTransmit)
	{
		if ((CapEnum)(m_vidModeXmit.GetType()) == eH263CapCode)
		{
		  BYTE is4CIF = NO;
		  if(m_vidModeXmit.GetFormat() == k4Cif){
			  is4CIF = YES;
		  }
		  return GetH261H263ResourcesPartyType(is4CIF);
		}
		else if (((CapEnum)(m_vidModeXmit.GetType()) == eH264CapCode) 	||
				((CapEnum)(m_vidModeXmit.GetType()) == eRtvCapCode) 	||
				((CapEnum)(m_vidModeXmit.GetType()) == eMsSvcCapCode)	||
				((CapEnum)(m_vidModeXmit.GetType()) == eSvcCapCode)		||
				((CapEnum)(m_vidModeXmit.GetType()) == eVP8CapCode))
			return m_vidModeXmit.GetVideoPartyTypeMBPSandFS(staticMB,IsRsrcByFs);
	}

	else if (eDirection & cmCapReceive)
	{
		if (((CapEnum)(m_vidModeRcv.GetType()) == eH264CapCode)		||
			((CapEnum)(m_vidModeRcv.GetType()) == eRtvCapCode) 		||
			((CapEnum)(m_vidModeRcv.GetType()) == eMsSvcCapCode)	||
			((CapEnum)(m_vidModeRcv.GetType()) == eSvcCapCode)		||
			((CapEnum)(m_vidModeXmit.GetType()) == eVP8CapCode))
			return m_vidModeRcv.GetVideoPartyTypeMBPSandFS(staticMB,0);

		else if ((CapEnum)(m_vidModeRcv.GetType()) == eH263CapCode)
		{
		   BYTE is4CIF = NO;
		   if(m_vidModeRcv.GetFormat() == k4Cif){
				is4CIF = YES;
			}
			return GetH261H263ResourcesPartyType(is4CIF);
		}
	}

	PTRACE2INT(eLevelInfoNormal, "CComModeH323::GetVideoPartyType - set h261 h263 cif party type ", eDirection);
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	if( systemCardsBasedMode == eSystemCardsMode_mpmrx &&( (eDirection & cmCapTransmit && (CapEnum)(m_vidModeXmit.GetType()) == eH261CapCode)   || ( eDirection & cmCapReceive && (CapEnum)(m_vidModeRcv.GetType()) == eH261CapCode ) )    )
	{
			return eCP_H261_CIF_equals_H264_HD1080_video_party_type;
	}

	return GetH261H263ResourcesPartyType(NO);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::GetFSandMBPS(cmCapDirection& eDirection, APIU16& profile, APIU8& level, long& fs, long& mbps, long& sar, long& staticMB,long& dpb) const
{	// * factor
	long brAndCpb = 0;

	//GetMbps()
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::GetRtvFSandMBPS(cmCapDirection& eDirection, long& fs, long& mbps) const
{
	PTRACE(eLevelInfoNormal, "CComModeH323::GetRtvFSandMBPS ");

	if (eDirection & cmCapTransmit)
		m_vidModeXmit.GetRtvScm( mbps, fs);

	if (eDirection & cmCapReceive)
		m_vidModeRcv.GetRtvScm( mbps, fs);

}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::GetMSSvcSpecificParams(cmCapDirection& eDirection, APIS32& Width, APIS32& Height, APIS32& aspectRatio, APIS32& maxFrameRate) const
{	// * factor

	//GetMbps()
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.GetMSSvcSpecificParams(Width, Height, aspectRatio, maxFrameRate);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.GetMSSvcSpecificParams(Width, Height, aspectRatio, maxFrameRate);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::GetVp8Scm(cmCapDirection& eDirection,APIS32& fs, APIS32& maxFrameRate) const
{	// * factor

	//GetMbps()
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.GetVp8Scm(fs, maxFrameRate);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.GetVp8Scm(fs, maxFrameRate);
}
//////////////////////////////////////////////////////////////////////////////////////////////
APIU16  CComModeH323::GetH264Profile(cmCapDirection eDirection) const
{
	APIU16 profile = 0;

	if (eDirection & cmCapTransmit)
		m_vidModeXmit.GetH264Profile(profile);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.GetH264Profile(profile);

	return profile;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetH264Profile(APIU16 profile, cmCapDirection eDirection)
{
	if (eDirection & cmCapTransmit)
		m_vidModeXmit.SetH264Profile(profile);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.SetH264Profile(profile);
}
//////////////////////////////////////////////////////////////////////////////////////////////
APIU8  CComModeH323::GetH264PacketizationMode(cmCapDirection eDirection) const
{
	APIU8 packetizationMode = 0;

	if (eDirection & cmCapTransmit)
		m_vidModeXmit.GetH264PacketizationMode(packetizationMode);
	if (eDirection & cmCapReceive)
		m_vidModeRcv.GetH264PacketizationMode(packetizationMode);

	return packetizationMode;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetAudioAlg(CapEnum alg,cmCapDirection eDirection,APIS32 bitrate)
{
	// BRIDGE-26, so the Audio algo will not be overwritten
	CapEnum currentAudioAlgo = ( (CapEnum)GetMediaType(cmCapAudio, cmCapReceive) );
    if ( CSacAudioCap::IsSacAudio(currentAudioAlgo) )
    {
        TRACEINTOFUNC << "SAC audio (audio algo: " <<  currentAudioAlgo << ").";
        return;
    }

	if (eDirection & cmCapTransmit) {
		m_audModeXmit.SetAudioAlg(alg,eDirection);
		m_audModeXmit.SetBitRate(bitrate);
	}
	if (eDirection & cmCapReceive) {
		m_audModeRcv.SetAudioAlg(alg,eDirection);
		m_audModeRcv.SetBitRate(bitrate);
	}
}

////////////////////////////////////////////////////////////////////////////
void  CComModeH323:: SetAudioAccordingToPartyName(const char* pPartyName)
{
	BOOL bIsForceG711A = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_FORCE_G711A, bIsForceG711A);

	if( (TRUE == bIsForceG711A) || (strstr(pPartyName, "##FORCE_MEDIA_AG711_A")!=NULL) )
		SetAudioAlg(eG711Alaw64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG711_U")!=NULL)
		SetAudioAlg(eG711Ulaw64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1_16K")!=NULL)
		SetAudioAlg(eG7221_16kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1_24K")!=NULL)
		SetAudioAlg(eG7221_24kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1_32K")!=NULL)
		SetAudioAlg(eG7221_32kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7231")!=NULL)
		SetAudioAlg(eG7231CapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG729")!=NULL)
		SetAudioAlg(eG729AnnexACapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG728")!=NULL)
		SetAudioAlg(eG728CapCode,cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN7_16K")!=NULL)
		SetAudioAlg(eSiren7_16kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14_48K")!=NULL)
		SetAudioAlg(eSiren14_48kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14_32K")!=NULL)
		SetAudioAlg(eSiren14_32kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14_24K")!=NULL)
		SetAudioAlg(eSiren14_24kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C_24K")!=NULL)
		SetAudioAlg(eG7221C_24kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C_32K")!=NULL)
		SetAudioAlg(eG7221C_32kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C_48K")!=NULL)
		SetAudioAlg(eG7221C_48kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1")!=NULL)
		SetAudioAlg(eG7221_24kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C")!=NULL)
		SetAudioAlg(eG7221C_24kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722STEREO")!=NULL)
	{
		BOOL isSLyncEnableG722Stereo = FALSE;
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		pSysConfig->GetBOOLDataByKey("LYNC2013_ENABLE_G722Stereo128k", isSLyncEnableG722Stereo);
		TRACEINTO << "LYNC_G722Stereo128k - isSLyncEnableG722Stereo:" << (DWORD)isSLyncEnableG722Stereo;
		if (isSLyncEnableG722Stereo == TRUE)
			SetAudioAlg(eG722Stereo_128kCapCode, cmCapReceiveAndTransmit);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722")!=NULL)// must be last because it can be confuse with the ##FORCE_MEDIA_AG7221C
		SetAudioAlg(eG722_64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_48K")!=NULL)
		SetAudioAlg(eSiren14Stereo_48kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_56K")!=NULL)
		SetAudioAlg(eSiren14Stereo_56kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_64K")!=NULL)
		SetAudioAlg(eSiren14Stereo_64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_96K")!=NULL)
		SetAudioAlg(eSiren14Stereo_96kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO")!=NULL)
		SetAudioAlg(eSiren14Stereo_48kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14")!=NULL)
			SetAudioAlg(eSiren14_24kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO_128K")!=NULL)
		SetAudioAlg(eSiren22Stereo_128kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO_96K")!=NULL)
		SetAudioAlg(eSiren22Stereo_96kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO_64K")!=NULL)
		SetAudioAlg(eSiren22Stereo_64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22_64K")!=NULL)
		SetAudioAlg(eSiren22_64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22_48K")!=NULL)
		SetAudioAlg(eSiren22_48kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22_32K")!=NULL)
		SetAudioAlg(eSiren22_32kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_32K")!=NULL)
		SetAudioAlg(eSirenLPR_32kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_48K")!=NULL)
		SetAudioAlg(eSirenLPR_48kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_64K")!=NULL)
		SetAudioAlg(eSirenLPR_64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_64K")!=NULL)
		SetAudioAlg(eSirenLPRStereo_64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_96K")!=NULL)
		SetAudioAlg(eSirenLPRStereo_96kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_128K")!=NULL)
		SetAudioAlg(eSirenLPRStereo_128kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR")!=NULL)
		SetAudioAlg(eSirenLPR_32kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_64K")!=NULL)
		SetAudioAlg(eG719_64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_48K")!=NULL)
		SetAudioAlg(eG719_48kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_32K")!=NULL)
		SetAudioAlg(eG719_32kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO_128K")!=NULL)
		SetAudioAlg(eG719Stereo_128kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO_96K")!=NULL)
		SetAudioAlg(eG719Stereo_96kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO_64K")!=NULL)
		SetAudioAlg(eG719Stereo_64kCapCode, cmCapReceiveAndTransmit);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AAC_LD")!=NULL) // TIP
		SetAudioAlg(eAAC_LDCapCode, cmCapReceiveAndTransmit);
    else if(strstr(pPartyName, "##FORCE_MEDIA_AILBC")!=NULL)
		SetAudioAlg(eiLBC_15kCapCode, cmCapReceiveAndTransmit);
    else if(strstr(pPartyName, "##FORCE_MEDIA_AOPUS_64")!=NULL)
		SetAudioAlg(eOpus_CapCode, cmCapReceiveAndTransmit, rate64K);
    else if(strstr(pPartyName, "##FORCE_MEDIA_AOPUSSTEREO_128")!=NULL)
		SetAudioAlg(eOpusStereo_CapCode, cmCapReceiveAndTransmit, rate128K);
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetAudioAlg(BOOL bIsSip, DWORD videoLineRate, const char* pPartyName, BYTE isReplace, BYTE bReduceAudioCodecs)
{
	PASSERTMSG_AND_RETURN((!CProcessBase::GetProcess() || !CProcessBase::GetProcess()->GetSysConfig()), "CComModeH323::SetAudioAlg - NULLs found in CProcessBase::GetProcess()->GetSysConfig()");

	// BRIDGE-26, so the Audio algo will not be overwritten
	CapEnum currentAudioAlgo = ( (CapEnum)GetMediaType(cmCapAudio, cmCapReceive) );
    if ( CSacAudioCap::IsSacAudio(currentAudioAlgo) )
    {
    	TRACEINTOFUNC << "SAC audio (audio algo: " <<  currentAudioAlgo << ").";
        return;
    }

	BOOL bIsForceG711A = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_FORCE_G711A, bIsForceG711A);

	//BRIDGE-12398
	//if( (TRUE == bIsForceG711A) || (strstr(pPartyName, "##FORCE_MEDIA_A") != NULL) )
	BOOL bIsForceSirenStero = videoLineRate==0 && strstr(pPartyName, "##FORCE_MEDIA_ASIREN")!=NULL && strstr(pPartyName, "STEREO")!=NULL; //no support for siren stereo on audio only calls
	BOOL bIsForceMedia 		= strstr(pPartyName, "##FORCE_MEDIA_A")!=NULL;

	if((bIsForceG711A || bIsForceMedia) && !bIsForceSirenStero)
	{
		SetAudioAccordingToPartyName(pPartyName);
		return;
	}

	//============================================================
	// Siren7 is dependent on configuration only and added first
	//============================================================
	if (bIsSip && IsFeatureSupportedBySystem(eFeatureSiren7))
	{
		BOOL siren7Allowed = FALSE;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ALLOW_SIREN7_CODEC, siren7Allowed);
		if (siren7Allowed)
		{
			SetAudioAlg(eSiren7_16kCapCode, cmCapReceiveAndTransmit);
			return;
		}
	}


	// set the preferred audio algorithm according to the SCM
	DWORD audioBitRate = GetMediaMode(cmCapAudio).GetBitRate() * _K_;
	if (!isReplace)
	{
		if(audioBitRate != 0)
			return;
	}
	DWORD confBitRate = GetCallRate() * 1000;
	DWORD confAudioBitRate = ::CalculateAudioRate(confBitRate);
	/*
	 if X = confRate - audio rate; if X = video rate, take confRate.
	else
	{
	if X > video rate  or if X < video rate, rounded video rate up to the next multiple of 64K and take audio rate according to this rate
	}
	*/
	DWORD calculatedConfRate = videoLineRate * 100;
	if((confBitRate - confAudioBitRate) != calculatedConfRate)
	{//because of the new rates of 96K and above the round up result is diferent then the conffference video rate
		// to make the calculated rate a multiple of 64k
		calculatedConfRate = (calculatedConfRate % rate64K) ? (calculatedConfRate/rate64K + 1) * rate64K : (calculatedConfRate/rate64K) * rate64K;
		if(calculatedConfRate)
			confBitRate = calculatedConfRate;
	}

	// default value is rate1024K
	DecideOnConfBitRateForAudioSelection(confBitRate);

//		confBitRate = rate256K;
	CCapSetInfo capInfo = eUnknownAlgorithemCapCode;
	BYTE setCodec = NO;
	PTRACE2INT(eLevelInfoNormal,"CComModeH323::SetAudioAlg: confBitRate = ",confBitRate);

	bool isiLBCFirst = (GetConfMediaType()==eMixAvcSvcVsw && confBitRate <= rate384K);
	APIS32 audioRate = confAudioBitRate; //CalculateAudioRate returned max 64K

	bool isOpusFirst = confAudioBitRate < rate24K;
	bool isOpusSet = false;

	// Set the selected mode first
	switch(confBitRate)
	{
	case rate64K:
	case rate96K:
		capInfo = CalculateAudioOpusCapEnum(audioRate);
		if(!isOpusFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
		{
			capInfo = eiLBC_15kCapCode;
			if(!isiLBCFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
			{
				capInfo = eG729AnnexACapCode;
				if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
				{
					capInfo = eG7231CapCode;
					if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
					{
						capInfo = eG7221_16kCapCode;
						if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
						{
							capInfo = eG7221C_24kCapCode;
							if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
							{
								capInfo = eG722_64kCapCode;
								if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
								{
									capInfo = CalculateAudioOpusCapEnum(audioRate);
									if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
									{
										capInfo = eG711Ulaw64kCapCode;
										if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
										{
											capInfo = eG711Alaw64kCapCode;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;

	case rate128K:
		capInfo = CalculateAudioOpusCapEnum(audioRate);
		if(!isOpusFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
		{
			capInfo = eiLBC_15kCapCode;
			if(!isiLBCFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
			{
				capInfo = eG719_32kCapCode;
				if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
				{
					capInfo = eSiren22_32kCapCode;
					if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
					{
						capInfo = eSirenLPR_32kCapCode;
						if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
						{
							capInfo = eG7221C_32kCapCode;
							if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
							{
								capInfo = eG7221C_24kCapCode;
									if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
									{
									capInfo = eSiren14_24kCapCode;
									if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
									{
										capInfo = eG7221_32kCapCode;
										if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
										{
											capInfo = eG7221_24kCapCode;
											if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
											{
												capInfo = eG7221_16kCapCode;
												if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
												{
													capInfo = eG728CapCode;
													if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
													{
														capInfo = eG729AnnexACapCode;
														if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
														{
															capInfo = eG722_64kCapCode;
															if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
															{
																capInfo = CalculateAudioOpusCapEnum(audioRate);
																if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
																{
																	capInfo = eG711Ulaw64kCapCode;
																	if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
																	{
																		capInfo = eG711Alaw64kCapCode;
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;

	case rate1024K:

		// TIP
		if (GetIsTipMode())
			capInfo = eAAC_LDCapCode;
		else
			capInfo = eG719Stereo_128kCapCode;

		if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
		{
			capInfo = eSiren22Stereo_128kCapCode;
			if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
			{
				capInfo = eSirenLPRStereo_128kCapCode;
				if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
				{
					//do nothing
					;
				}
				else
					setCodec = YES;
			}
			else
				setCodec = YES;
		}
		else
			setCodec = YES;
	case rate512K:
		if(setCodec)
			break;
		capInfo = CalculateAudioOpusCapEnum(audioRate);
		if(!isOpusFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
		{
			capInfo = eG719Stereo_96kCapCode;
			if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
			{
				capInfo = eSiren22Stereo_96kCapCode;
				if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
				{
					capInfo = eSiren14Stereo_96kCapCode;
					if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
					{
						capInfo = eSirenLPRStereo_96kCapCode;
						if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
						{
							//do nothing
							;
						}
						else
							setCodec = YES;
					}
					else
						setCodec = YES;
				}
				else
					setCodec = YES;
			}
			else
				setCodec = YES;
		}
		else
			setCodec = YES;
	case rate384K:
		if(setCodec)
			break;
		// Siren14 Stereo - Temp
		capInfo = CalculateAudioOpusCapEnum(audioRate);
		if(!isOpusFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
		{
			capInfo = eiLBC_15kCapCode;
			if(!isiLBCFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
			{
				capInfo = eG719Stereo_64kCapCode;
				if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
				{
					capInfo = eSiren22Stereo_64kCapCode;
					if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
					{
						capInfo = eSiren14Stereo_64kCapCode;
						if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
						{
							capInfo = eG719_64kCapCode;
							if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
							{
								capInfo = eSiren22_64kCapCode;
								if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
								{
									capInfo = eSirenLPR_64kCapCode;
									if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
									{
										capInfo = eSirenLPRStereo_64kCapCode;
										if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
										{
											//do nothing
											;
										}
										else
											setCodec = YES;
									}
									else
										setCodec = YES;
								}
								else
									setCodec = YES;
							}
							else
								setCodec = YES;
						}
						else
							setCodec = YES;
					}
					else
						setCodec = YES;
				}
				else
					setCodec = YES;
			}
			else
				setCodec = YES;
		}
		else
			setCodec = YES;
	case rate256K:
		if(setCodec)
			break;
		// Siren14 Stereo - Temp
		capInfo = CalculateAudioOpusCapEnum(audioRate);
		if(!isOpusFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
		{
			capInfo = eiLBC_15kCapCode;
			if(!isiLBCFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
			{
				capInfo = eSiren14Stereo_48kCapCode;
				if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
				{
					capInfo = eG719_48kCapCode;
					if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
					{
						capInfo = eSiren22_48kCapCode;
						if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
						{
							capInfo = eG7221C_48kCapCode;
							if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
							{
								capInfo = eSirenLPR_48kCapCode;
								if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
								{
									//do nothing
									;
								}
								else
									setCodec = YES;
							}
							else
								setCodec = YES;
						}
						else
							setCodec = YES;
					}
					else
						setCodec = YES;
				}
				else
					setCodec = YES;
			}
			else
				setCodec = YES;
		}
		else
			setCodec = YES;
	case rate192K:
		if(setCodec)
			break;
		capInfo = CalculateAudioOpusCapEnum(audioRate);
		if(!isOpusFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
		{
			capInfo = eiLBC_15kCapCode;
			if(!isiLBCFirst || capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
			{
				capInfo = eG719_32kCapCode;
				if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
				{
					capInfo = eSiren22_32kCapCode;
					if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
					{
						capInfo = eSirenLPR_32kCapCode;
						if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
						{
							capInfo = eG7221C_32kCapCode;
							if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
							{
								capInfo = eG7221C_24kCapCode;
								if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
								{
									capInfo = eG7221_32kCapCode;
									if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
									{
										capInfo = eG7221_24kCapCode;
										if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
										{
											capInfo = eG7221_16kCapCode;
											if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
											{
												capInfo = eG722_64kCapCode;
												if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
												{
													DWORD opusBitRate = (confBitRate < rate1024K) ? audioRate : rate128K;
													capInfo = CalculateAudioOpusCapEnum(opusBitRate);
													if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
													{
														capInfo = eG711Ulaw64kCapCode;
														if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
														{
															capInfo = eG711Alaw64kCapCode;
															if(capInfo.IsSupporedCap() == NO || (bReduceAudioCodecs && IsRemoveAudioCodec((CapEnum)capInfo)))
															{
																capInfo = eG729AnnexACapCode;
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	}

	BOOL isReduceCapsForRedcom = FALSE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("REDUCE_CAPS_FOR_REDCOM_SIP", isReduceCapsForRedcom);
	if(isReduceCapsForRedcom)
	{
		PTRACE(eLevelInfoNormal,"CComModeH323::SetAudioAlg REDUCE_CAPS_FOR_REDCOM_SIP" );
		SetAudioAlg(eSirenLPR_64kCapCode, cmCapReceiveAndTransmit);
		return;
	}

	SetAudioAlg(capInfo.GetIpCapCode(), cmCapReceiveAndTransmit, audioRate);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetFECCMode(CapEnum feccMode, DWORD feccRate, cmCapDirection eDirection)
{
	if (eDirection & cmCapTransmit)
		m_dataModeXmit.SetFECCMode(feccMode, feccRate, cmCapTransmit);
	if (eDirection & cmCapReceive)
		m_dataModeRcv.SetFECCMode(feccMode, feccRate, cmCapReceive);
}

//////////////////////////////////////////////////////////////////////////////////////////////
WORD CComModeH323::SetHighestH261ScmForCP(cmCapDirection eDirection, eVideoQuality videoQuality)
{
	WORD result = TRUE;
	BYTE bSet = FALSE;
	DWORD callRate = GetCallRate();

	if (eDirection & cmCapTransmit)
	{
		result &= m_vidModeXmit.SetHighestH261ScmForCP(callRate, videoQuality);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		if (bSet)
		{
			const CMediaModeH323& rMediaMode = GetMediaMode(cmCapVideo, cmCapTransmit);
		    SetMediaMode(rMediaMode, cmCapVideo, cmCapReceive);
		}
		else
			result &= m_vidModeRcv.SetHighestH261ScmForCP(callRate, videoQuality);
	}
	return result;
}


//////////////////////////////////////////////////////////////////////////////////////////////
WORD CComModeH323::SetHighestH263ScmForCP(cmCapDirection eDirection, eVideoQuality videoQuality)
{
	WORD result = TRUE;
	BYTE bSet = FALSE;
	DWORD callRate = GetCallRate();
	//modified by JasonZhu for NVGR-21708 begin
	DWORD audioRate = CalculateAudioRate((callRate*1000));
	DWORD videoRate = (callRate*10) - (audioRate/100);
	//GetVideoBitRate()
	PTRACE2INT(eLevelInfoNormal,"CComModeH323::SetHighestH263ScmForCP videoRate ",videoRate);
	if (eDirection & cmCapReceive)
	{

		//PTRACE2INT(eLevelInfoNormal,"CComModeH323::SetHighestH263ScmForCP audioRate ",audioRate);
		result &= m_vidModeRcv.SetHighestH263ScmForCP(videoRate, videoQuality);
		bSet = TRUE;
	}

	if (eDirection & cmCapTransmit)
	{
		if (bSet)
		{
			const CMediaModeH323& rMediaMode = GetMediaMode(cmCapVideo, cmCapReceive);
		    	SetMediaMode(rMediaMode, cmCapVideo, cmCapTransmit);
		}
		else
		{
			result &= m_vidModeXmit.SetHighestH263ScmForCP(videoRate, videoQuality);
		}
	}
	//modified by JasonZhu for NVGR-21708 end
	return result;
}


////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetHighestH263CapForVswAuto(cmCapDirection eDirection)
{
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		m_vidModeXmit.SetHighestH263CapForVswAuto();
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		if (bSet)
		{
			const CMediaModeH323& rMediaMode = GetMediaMode(cmCapVideo, cmCapTransmit);
		    SetMediaMode(rMediaMode, cmCapVideo, cmCapReceive);
		}
		else
			m_vidModeRcv.SetHighestH263CapForVswAuto();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
WORD CComModeH323::SetScmMpi(CapEnum protocol, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi, cmCapDirection eDirection)
{
	WORD result = TRUE;
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		result &= m_vidModeXmit.SetScmMpi(protocol, qcifMpi, cifMpi, cif4Mpi, cif16Mpi);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		if (bSet)
		{
			const CMediaModeH323& rMediaMode = GetMediaMode(cmCapVideo, cmCapTransmit);
		    SetMediaMode(rMediaMode, cmCapVideo, cmCapReceive);
			SetDirection(cmCapVideo, cmCapReceive);
		}
		else
			result &= m_vidModeRcv.SetScmMpi(protocol, qcifMpi, cifMpi, cif4Mpi, cif16Mpi);
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////
WORD CComModeH323::SetH263ScmPlus(BYTE bAnnexP, BYTE bAnnexT, BYTE bAnnexN, BYTE bAnnexI_NS,
								  char vga, char ntsc, char svga, char xga, char qntsc, cmCapDirection eDirection)
{
	WORD result = TRUE;
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		result &= m_vidModeXmit.SetH263ScmPlus(bAnnexP, bAnnexT, bAnnexN, bAnnexI_NS,
											  vga, ntsc, svga, xga, qntsc);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		if (bSet)
		{
			const CMediaModeH323& rMediaMode = GetMediaMode(cmCapVideo, cmCapTransmit);
		    SetMediaMode(rMediaMode, cmCapVideo, cmCapReceive);
		}
		else
			result &= m_vidModeRcv.SetH263ScmPlus(bAnnexP, bAnnexT, bAnnexN, bAnnexI_NS,
											       vga, ntsc, svga, xga, qntsc);
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////
WORD  CComModeH323::SetH263ScmInterlaced(WORD interlacedResolution, char qcifMpi, char cifMpi, cmCapDirection eDirection)
{
	WORD result = TRUE;
	BYTE bSet = FALSE;
	if (eDirection & cmCapTransmit)
	{
		result &= m_vidModeXmit.SetH263ScmInterlaced(interlacedResolution, qcifMpi, cifMpi);
		bSet = TRUE;
	}

	if (eDirection & cmCapReceive)
	{
		if (bSet)
		{
			const CMediaModeH323& rMediaMode = GetMediaMode(cmCapVideo, cmCapTransmit);
		    SetMediaMode(rMediaMode, cmCapVideo, cmCapReceive);
		}
		else
			result &= m_vidModeRcv.SetH263ScmInterlaced(interlacedResolution, qcifMpi, cifMpi);
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsAutoVideoResolution()
{
	return m_vidModeXmit.IsAutoResolution();
}

////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetAutoVideoResolution(BYTE bIsAuto)
{
	m_vidModeRcv.SetAutoResolution(bIsAuto);
	m_vidModeXmit.SetAutoResolution(bIsAuto);
}

////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsAutoVideoProtocol()
{
	return m_vidModeXmit.IsAutoProtocol();
}

////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetAutoVideoProtocol(BYTE bIsAuto)
{
	m_vidModeRcv.SetAutoProtocol(bIsAuto);
	m_vidModeXmit.SetAutoProtocol(bIsAuto);
}

////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetCallRate(DWORD rate)
{
	m_callRate = rate;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::CreateLocalComModeECS(EenMediaType encMediaType,EenHalfKeyType halfKeyType)
{
	m_encMediaTypeAlg = encMediaType;
	m_halfKeyType = halfKeyType;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetEncryption(BYTE bShouldEncrypted, BYTE bShouldDisconnectonEncryptFailure)
{
	m_isEncrypted = (bShouldEncrypted == TRUE || bShouldEncrypted == Encryp_On)? Encryp_On : Encryp_Off;
	m_bDisconnectOnEncryptionFailure = bShouldDisconnectonEncryptFailure;
}

///////////////////////////////////////////////////////////////////////////// //_dtls
void CComModeH323::SetDtlsEncryption(BYTE bShouldEncrypted)
{
	m_isDtlsEncrypted = (bShouldEncrypted == TRUE || bShouldEncrypted == Encryp_On)? Encryp_On : Encryp_Off;

	if(!bShouldEncrypted)
		SetDtlsAvailable(FALSE);
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
DWORD CComModeH323::GetIsDtlsEncrypted()
{
	return m_isDtlsEncrypted;
}

///////////////////////////////////////////////////////////////////////////// //_dtls
void CComModeH323::SetDtlsAvailable(BYTE bDtlsAvailable)
{
	if(bDtlsAvailable) {
		PASSERTMSG_AND_RETURN(GetIsDtlsEncrypted() != Encryp_On,"NO Dtls Encryption!");
	}

	m_bDtlsAvailable = bDtlsAvailable;
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
BYTE CComModeH323::GetIsDtlsAvailable()
{
	return m_bDtlsAvailable;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CComModeH323::GetIsEncrypted()
{
	return m_isEncrypted;
}
/////////////////////////////////////////////////////////////////////////////
BYTE	CComModeH323::GetIsDisconnectOnEncryptionFailure() const
{
	return m_bDisconnectOnEncryptionFailure;
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::CreateSipOptions(CCommConf* pCommConf,BYTE videoProtocol,BYTE partyEncryptionMode/* = AUTO*/)
{

	if(pCommConf->IsAudioConf())
	{
		SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit);
	}
	else
    {
		switch(videoProtocol)
	    {
	       case H264:
	       case AUTO:
		   {
			/*(Talya 29.11.05 Temp Patch)*///m_pUnifiedComMode->SetH264VidMode(H264_Level_2,-1,-1,-1,-1,m_pCommConf->GetVideoProtocol());
			//2000000/25000 = 80
			  SetH264Scm(H264_Profile_BaseLine, (APIU8)H264_Level_1_2,-1,-1,-1,80,-1, -1, cmCapReceiveAndTransmit);
			  break;
		   }
	       case H263:
		   {
		   	  CComModeInfo lComModeInfo((WORD)videoProtocol,(WORD)StartVideoCap);
		   	  CapEnum h323VideoCapCode = lComModeInfo.GetH323ModeType();
	          SetScmMpi(h323VideoCapCode,  1, 1, 0, 0,cmCapReceiveAndTransmit);
			  break;
		   }
	       case H261:
		   {
		   	 CComModeInfo lComModeInfo((WORD)videoProtocol,(WORD)StartVideoCap);
		   	 CapEnum h323VideoCapCode = lComModeInfo.GetH323ModeType();
			 SetScmMpi(h323VideoCapCode,pCommConf->GetQCIFframeRate(),pCommConf->GetCIFframeRate(),0,0,cmCapReceiveAndTransmit);
			 break;
		   }
	    }
    }
	// set xfer mode
	WORD confBitRate = pCommConf->GetConfTransferRate();
	CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
	DWORD h323Rate = lCapInfo.TranslateReservationRateToIpRate(confBitRate);
	SetCallRate(h323Rate);
	//set audio mode
	WORD audRate = pCommConf->GetAudioRate();
	CComModeInfo comModeInfo(audRate, StartAudioCap);
    CapEnum h323AudioCapCode = comModeInfo.GetH323ModeType();
	SetAudioAlg(h323AudioCapCode,cmCapReceiveAndTransmit);

	if(pCommConf->GetIsEncryption() && (partyEncryptionMode != NO))
	{
		CreateLocalComModeECS(kAES_CBC,kHalfKeyDH1024);
		CreateLocalSipComModeSdes(TRUE);
	}

}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
EenMediaType CComModeH323::GetEncryptionAlgType() const
{
	return m_encMediaTypeAlg;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetEncryptionAlgType(EenMediaType encAlgType)
{
	m_encMediaTypeAlg = encAlgType;
}

/////////////////////////////////////////////////////////////////////////////
EenHalfKeyType CComModeH323::GetEncryptionHalfKeyType()
{
	return m_halfKeyType;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetEncryptionHalfKeyType(EenHalfKeyType encHalfKeyType)
{
	m_halfKeyType = encHalfKeyType;
}

////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetHd720Enabled(BYTE bIsHd720Enabled)
{
	m_bIsHd720Enabled = bIsHd720Enabled;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsHd720Enabled() const
{
	BYTE bRes = (m_bIsHd720Enabled == TRUE);
	return bRes;
}
////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetHd1080Enabled(BYTE bIsHd1080Enabled)
{
	m_bIsHd1080Enabled = bIsHd1080Enabled;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsHd1080Enabled() const
{
	BYTE bRes = (m_bIsHd1080Enabled == TRUE);
	return bRes;
}
////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetHd720At60Enabled(BYTE bIsHd720At60Enabled)
{
	m_bIsHd720At60Enabled = bIsHd720At60Enabled;
}

////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetHd1080At60Enabled(BYTE bIsHd1080At60Enabled)
{
	m_bIsHd1080At60Enabled = bIsHd1080At60Enabled;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsHd720At60Enabled() const
{
	BYTE bRes = (m_bIsHd720At60Enabled == TRUE);
	return bRes;
}
////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsHd1080At60Enabled() const
{
	BYTE bRes = (m_bIsHd1080At60Enabled == TRUE);
	return bRes;
}
////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsH264HighProfile(cmCapDirection eDirection) const
{
	BYTE bRes = FALSE;
	if (GetH264Profile(eDirection) == H264_Profile_High)
		bRes = TRUE;
	return bRes;
}
////////////////////////////////////////////////////////////////////////////
BYTE  CComModeH323::IsH264HighProfileContent(cmCapDirection eDirection) const
{
	BYTE bRes = FALSE;
	APIU16 profile = 0;

	if (eDirection & cmCapTransmit)
		m_vidContModeXmit.GetH264Profile(profile);
	if (eDirection & cmCapReceive)
		m_vidContModeRcv.GetH264Profile(profile);

	if (profile == H264_Profile_High)
		bRes = TRUE;

	return bRes;
}
////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetFlowControlRateConstraint(DWORD flowControlRateConstraint)
{
	m_flowControlRateConstraint = flowControlRateConstraint;
}

////////////////////////////////////////////////////////////////////////////
DWORD CComModeH323::GetFlowControlRateConstraint()
{
	return m_flowControlRateConstraint;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetFlowControlRateConstraintForPresentation(DWORD flowControlRateConstraintForPresentation)
{
	m_flowControlRateConstraintForPresentation = flowControlRateConstraintForPresentation;
}

////////////////////////////////////////////////////////////////////////////
DWORD CComModeH323::GetFlowControlRateConstraintForPresentation()
{
	return m_flowControlRateConstraintForPresentation;
}

//////////////////////////////////////////////////////////////////////////////////////////////


void CComModeH323::CopyStaticAttributes(const CComModeH323& other)
{
	m_eConfType			= other.m_eConfType;
    m_confMediaType     = other.m_confMediaType;
	m_callRate			= other.m_callRate;
	m_encMediaTypeAlg	= other.m_encMediaTypeAlg ;
	m_halfKeyType		= other.m_halfKeyType;
	m_isEncrypted		= other.m_isEncrypted;
	m_bDisconnectOnEncryptionFailure		= other.m_bDisconnectOnEncryptionFailure;
	m_bIsHd720Enabled	= other.m_bIsHd720Enabled;
	m_bIsHd1080Enabled  =other.m_bIsHd1080Enabled;
	m_bIsHd720At60Enabled  =other.m_bIsHd720At60Enabled;
	m_bIsHd1080At60Enabled  =other.m_bIsHd1080At60Enabled;
	m_flowControlRateConstraint= other.m_flowControlRateConstraint;
	m_flowControlRateConstraintForPresentation= other.m_flowControlRateConstraintForPresentation;
	m_isLpr				= other.m_isLpr;

	//LYNC2013_FEC_RED:
	m_isFec				= other.m_isFec;
	m_isRed				= other.m_isRed;

	m_contentProtocol	= other.m_contentProtocol;
	m_declareContentRate= other.m_declareContentRate;
	m_TotalVideoRate    = other.m_TotalVideoRate;
	m_copTxLevel		= other.m_copTxLevel;
	m_partyId           = other.m_partyId;
	m_eTipMode			= other.m_eTipMode;// TIP
	m_TipAuxFPS			= other.m_TipAuxFPS;
	m_isDtlsEncrypted	= other.m_isDtlsEncrypted; //_dtls_
	m_bDtlsAvailable	= other.m_bDtlsAvailable; //_dtls_
	m_eTipContentMode   = other.m_eTipContentMode;
	m_operationPoints 	= other.m_operationPoints;
	m_eOPPreset			= other.m_eOPPreset;
	m_eIsUseOperationPointesPresets=other.m_eIsUseOperationPointesPresets;
	m_vidModeRcv.SetAutoProtocol(other.m_vidModeRcv.IsAutoProtocol());
	m_vidModeRcv.SetAutoResolution(other.m_vidModeRcv.IsAutoResolution());
	m_vidModeXmit.SetAutoProtocol(other.m_vidModeXmit.IsAutoProtocol());
	m_vidModeXmit.SetAutoResolution(other.m_vidModeXmit.IsAutoResolution());
	m_vidContModeRcv.SetAutoProtocol(other.m_vidContModeRcv.IsAutoProtocol());
	m_vidContModeRcv.SetAutoResolution(other.m_vidContModeRcv.IsAutoResolution());
	m_vidContModeXmit.SetAutoProtocol(other.m_vidContModeXmit.IsAutoProtocol());
	m_vidContModeXmit.SetAutoResolution(other.m_vidContModeXmit.IsAutoResolution());

	//SDES
	CSdesCap* pAudModeRcvSdesCap = other.m_audModeRcv.GetSdesCap();
	if(pAudModeRcvSdesCap)
	{
		m_audModeRcv.SetSdesCap(pAudModeRcvSdesCap);
	}


	CSdesCap* pAudModeXmitSdesCap = other.m_audModeXmit.GetSdesCap();
	if(pAudModeXmitSdesCap)
	{
		m_audModeXmit.SetSdesCap(pAudModeXmitSdesCap);
	}

	if( m_audModeXmit.IsMediaOn() && other.m_audModeXmit.IsMediaOn() )
	{
		m_audModeXmit.SetIvrSsrc( other.m_audModeXmit.GetIvrSsrc() );
	}

	CSdesCap* pVidModeRcvSdesCap = other.m_vidModeRcv.GetSdesCap();
	if(pVidModeRcvSdesCap)
	{
		m_vidModeRcv.SetSdesCap(pVidModeRcvSdesCap);
	}

	if( m_vidModeXmit.IsMediaOn() && other.m_vidModeXmit.IsMediaOn() )
	{
		m_vidModeXmit.SetIvrSsrc( other.m_vidModeXmit.GetIvrSsrc() ) ;
	}


	CSdesCap* pVidModeXmitSdesCap = other.m_vidModeXmit.GetSdesCap();
	if(pVidModeXmitSdesCap)
	{
		m_vidModeXmit.SetSdesCap(pVidModeXmitSdesCap);
	}

	CSdesCap* pVidContModeRcvSdesCap = other.m_vidContModeRcv.GetSdesCap();
	if(pVidContModeRcvSdesCap)
	{
		m_vidContModeRcv.SetSdesCap(pVidContModeRcvSdesCap);
	}

	CSdesCap* pVidContModeXmitSdesCap = other.m_vidContModeXmit.GetSdesCap();
	if(pVidContModeXmitSdesCap)
	{
		m_vidContModeXmit.SetSdesCap(pVidContModeXmitSdesCap);
	}

	CSdesCap* pDataModeRcvSdesCap = other.m_dataModeRcv.GetSdesCap();
	if(pDataModeRcvSdesCap)
	{
		m_dataModeRcv.SetSdesCap(pDataModeRcvSdesCap);
	}

	CSdesCap* pDataModeXmitSdesCap = other.m_dataModeXmit.GetSdesCap();
	if(pDataModeXmitSdesCap)
	{
		m_dataModeXmit.SetSdesCap(pDataModeXmitSdesCap);
	}

	//_dtls_
	CDtlsCap* pAudModeRcvDtlsCap = other.m_audModeRcv.GetDtlsCap();
	if(pAudModeRcvDtlsCap)
	{
		m_audModeRcv.SetDtlsCap(pAudModeRcvDtlsCap);
	}

	CDtlsCap* pAudModeXmitDtlsCap = other.m_audModeXmit.GetDtlsCap();
	if(pAudModeXmitDtlsCap)
	{
		m_audModeXmit.SetDtlsCap(pAudModeXmitDtlsCap);
	}

	CDtlsCap* pVidModeRcvDtlsCap = other.m_vidModeRcv.GetDtlsCap();
	if(pVidModeRcvDtlsCap)
	{
		m_vidModeRcv.SetDtlsCap(pVidModeRcvDtlsCap);
	}

	CDtlsCap* pVidModeXmitDtlsCap = other.m_vidModeXmit.GetDtlsCap();
	if(pVidModeXmitDtlsCap)
	{
		m_vidModeXmit.SetDtlsCap(pVidModeXmitDtlsCap);
	}

	CDtlsCap* pVidContModeRcvDtlsCap = other.m_vidContModeRcv.GetDtlsCap();
	if(pVidContModeRcvDtlsCap)
	{
		m_vidContModeRcv.SetDtlsCap(pVidContModeRcvDtlsCap);
	}

	CDtlsCap* pVidContModeXmitDtlsCap = other.m_vidContModeXmit.GetDtlsCap();
	if(pVidContModeXmitDtlsCap)
	{
		m_vidContModeXmit.SetDtlsCap(pVidContModeXmitDtlsCap);
	}

	CDtlsCap* pDataModeRcvDtlsCap = other.m_dataModeRcv.GetDtlsCap();
	if(pDataModeRcvDtlsCap)
	{
		m_dataModeRcv.SetDtlsCap(pDataModeRcvDtlsCap);
	}

	CDtlsCap* pDataModeXmitDtlsCap = other.m_dataModeXmit.GetDtlsCap();
	if(pDataModeXmitDtlsCap)
	{
		m_dataModeXmit.SetDtlsCap(pDataModeXmitDtlsCap);
	}

}

// this method is required where a media type is created out of a different direction type and direction is needed to be updated
void CComModeH323::SetDirection(cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole)
{
	CBaseCap* pBaseCapObj = NULL;
	// find which mode shall be updated
	switch(dataType)
	{
	case cmCapVideo:

		if (eRole & kRoleContentOrPresentation)
		{
			if (direction & cmCapReceive)
				pBaseCapObj = m_vidContModeRcv.GetAsCapClass();
			else if (direction & cmCapTransmit)
				pBaseCapObj = m_vidContModeXmit.GetAsCapClass();
		}
		else
		{
			if (direction & cmCapReceive)
				pBaseCapObj = m_vidModeRcv.GetAsCapClass();
			else if (direction & cmCapTransmit)
				pBaseCapObj = m_vidModeXmit.GetAsCapClass();
		}

		break;
	case cmCapAudio:
		if (direction & cmCapReceive)
			pBaseCapObj = m_audModeRcv.GetAsCapClass();
		else if (direction & cmCapTransmit)
			pBaseCapObj = m_audModeXmit.GetAsCapClass();
		break;
	case cmCapData:
		if (direction & cmCapReceive)
			pBaseCapObj = m_dataModeRcv.GetAsCapClass();
		else if (direction & cmCapTransmit)
			pBaseCapObj = m_dataModeXmit.GetAsCapClass();
		break;
	case cmCapBfcp:
		if (direction & cmCapReceive)
			pBaseCapObj = m_bfcpModeRcv.GetAsCapClass();
		else if (direction & cmCapTransmit)
			pBaseCapObj = m_bfcpModeXmit.GetAsCapClass();
		break;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
	}
	if(pBaseCapObj)
		pBaseCapObj->SetDirection(direction);

    POBJDELETE(pBaseCapObj);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetIsLpr(BYTE isLpr)
{
	m_isLpr = isLpr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::GetIsLpr() const
{
	return m_isLpr;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CComModeH323::SetIsFec(BYTE isFec)
{
	m_isFec = isFec;
}

//////////
BYTE CComModeH323::GetIsFec() const
{
	return m_isFec;
}
//////////
void CComModeH323::SetIsRed(BYTE isRed)
{
	m_isRed = isRed;
}

///////////
BYTE CComModeH323::GetIsRed() const
{
	return m_isRed;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetHdVswResolution(EHDResolution hdVswRes)
{
	m_HdVswResolution = hdVswRes;
}
EHDResolution CComModeH323::GetHdVswResolution()
{
	return m_HdVswResolution;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::DecideOnConfBitRateForAudioSelection(DWORD& confBitRate)
{
	// This is a temporary matrix - TBD by SRE
	if (confBitRate < rate96K)
		confBitRate = rate64K;
	else if (confBitRate < rate192K)
		confBitRate = rate128K;
	else if (confBitRate < rate256K)
		confBitRate = rate192K;
	else if (confBitRate < rate512K)
		confBitRate = rate384K;
	else if (confBitRate < rate1024K)
		confBitRate = rate512K;
	else
		confBitRate = rate1024K;
}
///////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::isModeIncludeStereo()
{
	if (GetCodecNumberOfChannels((CapEnum)m_audModeXmit.GetType()) == AUDIO_STEREO_NUM_OF_CHANNELS)
		return TRUE;

	if (GetCodecNumberOfChannels((CapEnum)m_audModeRcv.GetType()) == AUDIO_STEREO_NUM_OF_CHANNELS)
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::CreateLocalSipComModeSdes(BYTE bIsEncrypted, BOOL isTipCompatible)
{
	if(bIsEncrypted == FALSE) {
		m_isEncrypted = Encryp_Off;
		m_bDisconnectOnEncryptionFailure = FALSE;
		PTRACE(eLevelInfoNormal,"CComModeH323::CreateLocalSipComModeSdes: encryption is off");
		return;
	}

	BOOL bIsTipMode = (isTipCompatible)?TRUE:GetIsTipMode();
	m_isEncrypted = Encryp_On;
	PTRACE2INT(eLevelInfoNormal,"CComModeH323::CreateLocalSipComModeSdes: encryption is on, tip is ", bIsTipMode);


	m_audModeXmit.SetSdesDefaultTxData(cmCapAudio, bIsTipMode );
	m_audModeRcv.SetSdesDefaultTxData(cmCapAudio, bIsTipMode);

	if(m_vidModeRcv.IsMediaOn() == TRUE) {
		m_vidModeRcv.SetSdesDefaultTxData(cmCapVideo, bIsTipMode);
		m_vidModeXmit.SetSdesDefaultTxData(cmCapVideo, bIsTipMode);
	}

	if(m_dataModeRcv.IsMediaOn() == TRUE) {
		m_dataModeRcv.SetSdesDefaultTxData(cmCapData, bIsTipMode);
		m_dataModeXmit.SetSdesDefaultTxData(cmCapData, bIsTipMode);
	}

	if(m_vidContModeRcv.IsMediaOn() == TRUE) {
		m_vidContModeRcv.SetSdesDefaultTxData(cmCapVideo, bIsTipMode, kRolePresentation);
		m_vidContModeXmit.SetSdesDefaultTxData(cmCapVideo, bIsTipMode, kRolePresentation);
	}
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
void CComModeH323::CreateLocalSipComModeDtls(BYTE bIsEncrypted, BOOL isTipCompatible)
{
	if(bIsEncrypted == FALSE) {
		m_isDtlsEncrypted = Encryp_Off;
		//m_bDisconnectOnEncryptionFailure = FALSE;
		PTRACE(eLevelInfoNormal,"CComModeH323::CreateLocalSipComModeDtls: dtls encryption is off");
		return;
	}

	BOOL bIsTipMode = (isTipCompatible)?TRUE:GetIsTipMode();
	m_isDtlsEncrypted = Encryp_On;
	PTRACE2INT(eLevelInfoNormal,"CComModeH323::CreateLocalSipComModeDtls: dtls encryption is on, tip is ", bIsTipMode);

	m_audModeXmit.SetDtlsDefaultTxData(cmCapAudio, bIsTipMode);
	m_audModeRcv.SetDtlsDefaultTxData(cmCapAudio, bIsTipMode);

	if(m_vidModeRcv.IsMediaOn() == TRUE) {
		m_vidModeRcv.SetDtlsDefaultTxData(cmCapVideo, bIsTipMode);
		m_vidModeXmit.SetDtlsDefaultTxData(cmCapVideo, bIsTipMode);
	}

	if(m_dataModeRcv.IsMediaOn() == TRUE) {
		m_dataModeRcv.SetDtlsDefaultTxData(cmCapData, bIsTipMode);
		m_dataModeXmit.SetDtlsDefaultTxData(cmCapData, bIsTipMode);
	}

	if(m_vidContModeRcv.IsMediaOn() == TRUE) {
		m_vidContModeRcv.SetDtlsDefaultTxData(cmCapVideo, bIsTipMode, kRolePresentation);
		m_vidContModeXmit.SetDtlsDefaultTxData(cmCapVideo, bIsTipMode, kRolePresentation);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::CreateLocalSipComModeSdesForSpecficMedia(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole)
{
	BOOL bIsTipMode = GetIsTipMode();
	PTRACE2INT(eLevelInfoNormal,"CComModeH323::CreateLocalSipComModeSdesForSpecficMedia: tip is ", bIsTipMode);

	if (eRole == kRolePeople)
		{
			switch(type)
			{
			case cmCapAudio:
				if (direction & cmCapReceive)
					m_audModeRcv.SetSdesDefaultTxData(cmCapAudio, bIsTipMode);
				if (direction & cmCapTransmit)
					m_audModeXmit.SetSdesDefaultTxData(cmCapAudio, bIsTipMode);
				break;
			case cmCapVideo:
				if (direction & cmCapReceive)
					m_vidModeRcv.SetSdesDefaultTxData(cmCapVideo, bIsTipMode);
				if (direction & cmCapTransmit)
					m_vidModeXmit.SetSdesDefaultTxData(cmCapVideo, bIsTipMode);
				break;
			case cmCapData:
				if (direction & cmCapReceive)
					m_dataModeRcv.SetSdesDefaultTxData(cmCapData, bIsTipMode);
				if (direction & cmCapTransmit)
					m_dataModeXmit.SetSdesDefaultTxData(cmCapData, bIsTipMode);
				break;
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
			}
		}
		else if (eRole & kRoleContentOrPresentation)
		{
			if (type == cmCapVideo)
			{
				if (direction & cmCapReceive)
					m_vidContModeRcv.SetSdesDefaultTxData(cmCapVideo,bIsTipMode, kRolePresentation);
				if (direction & cmCapTransmit)
					m_vidContModeXmit.SetSdesDefaultTxData(cmCapVideo, bIsTipMode, kRolePresentation);
			}
		}
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetSdesMkiDefaultParams(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole, BYTE bIsNeedToSendMKI)
{
	if (eRole == kRolePeople)
	{
		switch(type)
		{
		case cmCapAudio:
			if (direction & cmCapReceive)
				m_audModeRcv.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
			if (direction & cmCapTransmit)
				m_audModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
			break;
		case cmCapVideo:
			if (direction & cmCapReceive)
				m_vidModeRcv.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
			if (direction & cmCapTransmit)
				m_vidModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
			break;
		case cmCapData:
			if (direction & cmCapReceive)
				m_dataModeRcv.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
			if (direction & cmCapTransmit)
				m_dataModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
		}
	}
	else if (eRole & kRoleContentOrPresentation)
	{
		if (type == cmCapVideo)
		{
			if (direction & cmCapReceive)
				m_vidContModeRcv.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
			if (direction & cmCapTransmit)
				m_vidContModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetSdesLifeTimeDefaultParams(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole, BYTE bLifeTimeInUse)
{
	if (eRole == kRolePeople)
	{
		switch(type)
		{
		case cmCapAudio:
			if (direction & cmCapReceive)
				m_audModeRcv.SetSdesLifeTimeDefaultParams(bLifeTimeInUse);
			if (direction & cmCapTransmit)
				m_audModeXmit.SetSdesLifeTimeDefaultParams(bLifeTimeInUse);
			break;
		case cmCapVideo:
			if (direction & cmCapReceive)
				m_vidModeRcv.SetSdesLifeTimeDefaultParams(bLifeTimeInUse);
			if (direction & cmCapTransmit)
				m_vidModeXmit.SetSdesLifeTimeDefaultParams(bLifeTimeInUse);
			break;
		case cmCapData:
			if (direction & cmCapReceive)
				m_dataModeRcv.SetSdesLifeTimeDefaultParams(bLifeTimeInUse);
			if (direction & cmCapTransmit)
				m_dataModeXmit.SetSdesLifeTimeDefaultParams(bLifeTimeInUse);
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
		}
	}
	else if (eRole & kRoleContentOrPresentation)
	{
		if (type == cmCapVideo)
		{
			if (direction & cmCapReceive)
				m_vidContModeRcv.SetSdesLifeTimeDefaultParams(bLifeTimeInUse);
			if (direction & cmCapTransmit)
				m_vidContModeXmit.SetSdesLifeTimeDefaultParams(bLifeTimeInUse);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetSdesFecDefaultParams(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole, BYTE bFecKeyInUse)
{
	if (eRole == kRolePeople)
	{
		switch(type)
		{
		case cmCapAudio:
			if (direction & cmCapReceive)
				m_audModeRcv.SetSdesFecDefaultParams(bFecKeyInUse);
			if (direction & cmCapTransmit)
				m_audModeXmit.SetSdesFecDefaultParams(bFecKeyInUse);
			break;
		case cmCapVideo:
			if (direction & cmCapReceive)
				m_vidModeRcv.SetSdesFecDefaultParams(bFecKeyInUse);
			if (direction & cmCapTransmit)
				m_vidModeXmit.SetSdesFecDefaultParams(bFecKeyInUse);
			break;
		case cmCapData:
			if (direction & cmCapReceive)
				m_dataModeRcv.SetSdesFecDefaultParams(bFecKeyInUse);
			if (direction & cmCapTransmit)
				m_dataModeXmit.SetSdesFecDefaultParams(bFecKeyInUse);
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
		}
	}
	else if (eRole & kRoleContentOrPresentation)
	{
		if (type == cmCapVideo)
		{
			if (direction & cmCapReceive)
				m_vidContModeRcv.SetSdesFecDefaultParams(bFecKeyInUse);
			if (direction & cmCapTransmit)
				m_vidContModeXmit.SetSdesFecDefaultParams(bFecKeyInUse);
		}
	}
}
///////////////////////////////////////////////////////////////////////////// //_dtls_
void CComModeH323::CreateLocalSipComModeDtlsForSpecficMedia(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole)
{
	BOOL bIsTipMode = GetIsTipMode();
	PTRACE2INT(eLevelInfoNormal,"CComModeH323::CreateLocalSipComModeDtlsForSpecficMedia: tip is ", bIsTipMode);

	if (eRole == kRolePeople)
		{
			switch(type)
			{
			case cmCapAudio:
				if (direction & cmCapReceive)
					m_audModeRcv.SetDtlsDefaultTxData(cmCapAudio, bIsTipMode);
				if (direction & cmCapTransmit)
					m_audModeXmit.SetDtlsDefaultTxData(cmCapAudio, bIsTipMode);
				break;
			case cmCapVideo:
				if (direction & cmCapReceive)
					m_vidModeRcv.SetDtlsDefaultTxData(cmCapVideo, bIsTipMode);
				if (direction & cmCapTransmit)
					m_vidModeXmit.SetDtlsDefaultTxData(cmCapVideo, bIsTipMode);
				break;
			case cmCapData:
				if (direction & cmCapReceive)
					m_dataModeRcv.SetDtlsDefaultTxData(cmCapData, bIsTipMode);
				if (direction & cmCapTransmit)
					m_dataModeXmit.SetDtlsDefaultTxData(cmCapData, bIsTipMode);
				break;
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
			}
		}
		else if (eRole & kRoleContentOrPresentation)
		{
			if (type == cmCapVideo)
			{
				if (direction & cmCapReceive)
					m_vidContModeRcv.SetDtlsDefaultTxData(cmCapVideo, bIsTipMode, kRolePresentation);
				if (direction & cmCapTransmit)
					m_vidContModeXmit.SetDtlsDefaultTxData(cmCapVideo, bIsTipMode, kRolePresentation);
			}
		}
}


/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateXmitSdesAudioTag(APIU32 tag)
{
	if(m_audModeXmit.IsMediaOn() == TRUE) {
		m_audModeXmit.SetSdesTag(tag);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateXmitSdesAudioTag: Media is off!!");
		return;
	}
	m_audModeRcv.SetSdesTag(tag);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateXmitSdesVideoTag(APIU32 tag)
{
	if(m_vidModeXmit.IsMediaOn() == TRUE) {
		m_vidModeXmit.SetSdesTag(tag);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateXmitSdesVideoTag: Media is off!!");
		return;
	}
	m_vidModeRcv.SetSdesTag(tag);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateXmitSdesDataTag(APIU32 tag)
{
	if(m_dataModeXmit.IsMediaOn() == TRUE) {
		m_dataModeXmit.SetSdesTag(tag);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateXmitSdesDataTag: Media is off!!");
		return;
	}
	m_dataModeRcv.SetSdesTag(tag);
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateXmitSdesContentTag(APIU32 tag)
{
	if(m_vidContModeXmit.IsMediaOn() == TRUE) {
		m_vidContModeXmit.SetSdesTag(tag);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateXmitSdesContentTag: Media is off!!");
		return;
	}
	m_vidContModeRcv.SetSdesTag(tag);
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateRxSdesAudioTag(APIU32 tag)
{
	if(m_audModeRcv.IsMediaOn() == TRUE) {
		m_audModeRcv.SetSdesTag(tag);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateRxSdesAudioTag: Media is off!!");
		return;
	}
	m_audModeXmit.SetSdesTag(tag);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateRxSdesAudioMkiInUse(BYTE bIsNeedToSendMKI)
{
	if(m_audModeRcv.IsMediaOn() == TRUE) {
		m_audModeRcv.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateRxSdesAudioMkiInUse: Media is off!!");
		return;
	}
	//m_audModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
}


/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateRxSdesVideoMkiInUse(BYTE bIsNeedToSendMKI)
{
	if(m_vidModeRcv.IsMediaOn() == TRUE) {
		m_vidModeRcv.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateRxSdesVideoMkiInUse: Media is off!!");
		return;
	}
	//m_vidModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
}


/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateRxSdesDataMkiInUse(BYTE bIsNeedToSendMKI)
{
	if(m_dataModeRcv.IsMediaOn() == TRUE) {
		m_dataModeRcv.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateRxSdesDataMkiInUse: Media is off!!");
		return;
	}
	//m_dataModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
}


/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateRxSdesContentMkiInUse(BYTE bIsNeedToSendMKI)
{
	if(m_vidContModeRcv.IsMediaOn() == TRUE) {
		m_vidContModeRcv.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateRxSdesContentMkiInUse: Media is off!!");
		return;
	}
	//m_vidContModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateTxSdesAudioMkiInUse(BYTE bIsNeedToSendMKI)
{
	if(m_audModeXmit.IsMediaOn() == TRUE) {
		m_audModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateTxSdesAudioMkiInUse: Media is off!!");
		return;
	}
}


/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateTxSdesVideoMkiInUse(BYTE bIsNeedToSendMKI)
{
	if(m_vidModeXmit.IsMediaOn() == TRUE) {
		m_vidModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateTxSdesVideoMkiInUse: Media is off!!");
		return;
	}
}


/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateTxSdesDataMkiInUse(BYTE bIsNeedToSendMKI)
{
	if(m_dataModeXmit.IsMediaOn() == TRUE) {
		m_dataModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateTxSdesDataMkiInUse: Media is off!!");
		return;
	}
}


/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateTxSdesContentMkiInUse(BYTE bIsNeedToSendMKI)
{
	if(m_vidContModeXmit.IsMediaOn() == TRUE) {
		m_vidContModeXmit.SetSdesMkiDefaultParams(bIsNeedToSendMKI);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateTxSdesContentMkiInUse: Media is off!!");
		return;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateRxSdesVideoTag(APIU32 tag)
{
	if(m_vidModeRcv.IsMediaOn() == TRUE) {
		m_vidModeRcv.SetSdesTag(tag);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateRxSdesVideoTag: Media is off!!");
		return;
	}
	m_vidModeXmit.SetSdesTag(tag);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateRxSdesDataTag(APIU32 tag)
{
	if(m_dataModeRcv.IsMediaOn() == TRUE) {
		m_dataModeRcv.SetSdesTag(tag);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateRxSdesDataTag: Media is off!!");
		return;
	}
	m_dataModeXmit.SetSdesTag(tag);
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateRxSdesContentTag(APIU32 tag)
{
	if(m_vidContModeRcv.IsMediaOn() == TRUE) {
		m_vidContModeRcv.SetSdesTag(tag);
	} else {
		PTRACE(eLevelInfoNormal, "CComModeH323::UpdateRxSdesContentTag: Media is off!!");
		return;
	}
	m_vidContModeXmit.SetSdesTag(tag);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateXmitSdesAudioMasterSaltBase64Key(char* key)
{
	m_audModeXmit.SetSdesBase64MasterSaltKey(key);
	m_audModeRcv.SetSdesBase64MasterSaltKey(key);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateXmitSdesVideoMasterSaltBase64Key(char* key)
{
	if(m_vidModeXmit.IsMediaOn() == TRUE) {
		m_vidModeXmit.SetSdesBase64MasterSaltKey(key);
	} else {
		return;
	}
	m_vidModeRcv.SetSdesBase64MasterSaltKey(key);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateXmitSdesDataMasterSaltBase64Key(char* key)
{
	if(m_dataModeXmit.IsMediaOn() == TRUE){
		m_dataModeXmit.SetSdesBase64MasterSaltKey(key);
	} else {
		return;
	}
	m_dataModeRcv.SetSdesBase64MasterSaltKey(key);
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::UpdateXmitSdesContentMasterSaltBase64Key(char* key)
{
	if(m_vidContModeXmit.IsMediaOn() == TRUE) {
		m_vidContModeXmit.SetSdesBase64MasterSaltKey(key);
	} else {
		return;
	}
	m_vidContModeRcv.SetSdesBase64MasterSaltKey(key);
}
/////////////////////////////////////////////////////////////////////////////
CSdesCap* CComModeH323::GetSipSdes(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	const CMediaModeH323& rMediaModeH323  = GetMediaMode(type,direction,eRole);
	return rMediaModeH323.GetSdesCap();
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetSipSdes(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, CSdesCap* pSdesCap)
{
	CMediaModeH323& rMediaModeH323  = GetMediaMode(type,direction,eRole);
	rMediaModeH323.SetSdesCap(pSdesCap);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::RemoveSipSdes(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	CMediaModeH323& rMediaModeH323  = GetMediaMode(type,direction,eRole);
	rMediaModeH323.RemoveSdesCap();
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
CDtlsCap* CComModeH323::GetSipDtls(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	const CMediaModeH323& rMediaModeH323  = GetMediaMode(type,direction,eRole);
	return rMediaModeH323.GetDtlsCap();
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
void CComModeH323::SetSipDtls(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, CDtlsCap* pDtlsCap)
{
	CMediaModeH323& rMediaModeH323  = GetMediaMode(type,direction,eRole);
	rMediaModeH323.SetDtlsCap(pDtlsCap);
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
void CComModeH323::RemoveSipDtls(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	CMediaModeH323& rMediaModeH323  = GetMediaMode(type,direction,eRole);
	rMediaModeH323.RemoveDtlsCap();
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
BYTE CComModeH323::IsDtlsChannelEnabled(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	const CMediaModeH323& rMediaModeH323  = GetMediaMode(type,direction,eRole);

	CDtlsCap* pDtlsCap = rMediaModeH323.GetDtlsCap();

	if (pDtlsCap)
		return TRUE;
	else
		return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// Change H264 parameters for force Party
void  CComModeH323::ChangeH264ScmForForceParty(const char* partyName)
{
	CapEnum protocol = (CapEnum)GetMediaType(cmCapVideo, cmCapTransmit);
	if (protocol != eH264CapCode)
			return;

	char* pPointerStr = NULL;
    APIU16 profile;
    APIU8 level;
    long mbps, fs, dpb, brAndCpb, sar, staticMB;

    pPointerStr = (char*)strstr(partyName, "##FORCE_1080_AT_60_SETUP");
    if(pPointerStr != NULL)
    {
		//===============================================================================
		// As 1080p60 affects both FS and MBPS for both directions (720p60 for receive)
		// it is done mutually exclusive from the other "force" actions
		//===============================================================================
		PTRACE1(eLevelInfoNormal,"Forcing 1080p60 setup");
		SetHd1080At60Enabled(YES);
		GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
		SetH264Scm(profile, level, GetMaxMbpsAsDevision(H264_HD720_60_MBPS), GetMaxFsAsDevision(H264_HD720_FS), dpb, brAndCpb, sar, staticMB, cmCapReceive);
		GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);
		// 1080p60debug - to be removed
		// SetH264Scm(profile, level, GetMaxMbpsAsDevision(H264_HD1080_60_MBPS), GetMaxFsAsDevision(H264_HD1080_FS), dpb, brAndCpb, sar, staticMB, cmCapTransmit);
		SetH264Scm(profile, level, GetMaxMbpsAsDevision(Get1080p60mbps()), GetMaxFsAsDevision(H264_HD1080_FS), dpb, brAndCpb, sar, staticMB, cmCapTransmit);
    }
    else
    {
	int iSar = 0, iFs = 0, iMbps = 0;
	pPointerStr = (char*)strstr(partyName, "##FORCE_ASPECT_RATIO_");
	if(pPointerStr != NULL)
	{
		iSar = atoi(pPointerStr + strlen("##FORCE_ASPECT_RATIO_"));
		if(iSar == 256)
			iSar = -1;
		PTRACE2INT(eLevelInfoNormal,"Force sar = ",iSar);
	}
	pPointerStr = (char*)strstr(partyName, "##FORCE_FS_");
	if(pPointerStr != NULL)
	{
		iFs = atoi(pPointerStr + strlen("##FORCE_FS_"));
		PTRACE2INT(eLevelInfoNormal,"Force fs = ",iFs);
	}
	pPointerStr = (char*)strstr(partyName, "##FORCE_MBPS_");
	if(pPointerStr != NULL)
	{
		iMbps = atoi(pPointerStr + strlen("##FORCE_MBPS_"));
		PTRACE2INT(eLevelInfoNormal,"Force mbps = ",iMbps);
	}
	APIU16 profile;
	APIU8 level;
	long mbps, fs, dpb, brAndCpb, sar, staticMB;
	if (iSar || iMbps || iFs )
	{
		GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
		SetH264Scm(profile, level, iMbps ? iMbps:mbps, iFs?iFs: fs, dpb, brAndCpb, iSar?iSar:sar, staticMB, cmCapReceive);
		GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);
		SetH264Scm(profile, level, iMbps ? iMbps:mbps, iFs?iFs: fs, dpb, brAndCpb, iSar?iSar:sar, staticMB, cmCapTransmit);
	}
    }
}
//////////////////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::IsRemoveAudioCodec(CapEnum codec)
{    // Temporary solution for reducing audio codecs number. In future it will be implemented by Re-Invite.
	int i = 0;
	while (g_SipDialOutRemovedAudioCodecs[i] != eUnknownAlgorithemCapCode)
	{
		if (g_SipDialOutRemovedAudioCodecs[i] == codec)
			return TRUE;
		i++;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
EResult CComModeH323::SetBfcp(enTransportType transType, cmCapDirection eDirection)
{
	EResult bRes = kFailure;

	PTRACE2INT(eLevelInfoNormal, "CComModeH323::SetBfcp - transport type:", transType);

	if (eDirection & cmCapTransmit)
	{
		bRes = m_bfcpModeXmit.SetBfcp(transType);
	}

	if (eDirection & cmCapReceive)
	{
		bRes = m_bfcpModeRcv.SetBfcp(transType);
	}

	return bRes;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetBfcpTransportType(enTransportType transType, cmCapDirection eDirection)
{
	if (eDirection & cmCapTransmit)
		m_bfcpModeXmit.SetTransportType(transType);
	if (eDirection & cmCapReceive)
		m_bfcpModeRcv.SetTransportType(transType);
}

//////////////////////////////////////////////////////////////////////////////////////////////
enTransportType CComModeH323::GetBfcpTransportType(cmCapDirection eDirection) const
{
	enTransportType transType = eUnknownTransportType;

	if (eDirection & cmCapTransmit)
		transType = m_bfcpModeXmit.GetTransportType();
	if (eDirection & cmCapReceive)
		transType = m_bfcpModeRcv.GetTransportType();

	return transType;
}

//////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetBfcpParameters(eBfcpSetup setup, eBfcpConnection connection, eBfcpFloorCtrl floorCtrl, eBfcpMStreamType mstreamType)
{
	m_bfcpModeXmit.SetBfcpParameters(setup, connection, floorCtrl, mstreamType);
	m_bfcpModeRcv.SetBfcpParameters(setup, connection, floorCtrl, mstreamType);
}

/////////////////////////////////////////////////////////////////////////
void CComModeH323::SetConfUserIdForBfcp(WORD confId, WORD userId)
{
	m_bfcpModeXmit.SetConfUserId(confId, userId);
	m_bfcpModeRcv.SetConfUserId(confId, userId);
}
/////////////////////////////////////////////////////////////////////////
void CComModeH323::SetFloorIdParamsForBfcp(char* pFloorid, char* pStreamid)
{
	m_bfcpModeXmit.SetFloorIdParams(pFloorid, pStreamid);
	m_bfcpModeRcv.SetFloorIdParams(pFloorid, pStreamid);
}
/////////////////////////////////////////////////////////////////////////
WORD CComModeH323::GetBfcpUserId()
{
	return m_bfcpModeXmit.GetUserId();
}
/////////////////////////////////////////////////////////////////////////
WORD CComModeH323::GetBfcpConfId()
{
	return m_bfcpModeXmit.GetConfId();
}
/////////////////////////////////////////////////////////////////////////
int	CComModeH323::CopTranslateConfResolutionToConfigureResolution(ECopDecoderResolution decoderResolution)
{
	switch (decoderResolution) {

		case COP_decoder_resolution_HD108030:
			return eCopLevelEncoderVideoFormat_HD1080p;
			break;
		case COP_decoder_resolution_HD720p50:
		case COP_decoder_resolution_HD720p25:
			return eCopLevelEncoderVideoFormat_HD720p;
			break;

		case COP_decoder_resolution_W4CIF25:
		case COP_decoder_resolution_4CIF25:
		case COP_decoder_resolution_4CIF50:
			return eCopLevelEncoderVideoFormat_4CIF_16_9;
			break;

  		case COP_decoder_resolution_CIF25:
  			return eCopLevelEncoderVideoFormat_CIF;
  			break;

		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	PTRACE2INT(eLevelInfoNormal, "CComModeH323::FindBestCopLevelAccordingToResolution - AN DEBUG, decoderResolution wasn't found", decoderResolution);
	DBGPASSERT(decoderResolution+1000);

	return -1;
}

/////////////////////////////////////////////////////////////////////////////
CapEnum	CComModeH323::CopFindBestLevelAccordingToResolution(ECopDecoderResolution& decoderResolution, CapEnum capCode,APIU16& capProfile ,CCOPConfigurationList* pCOPConfigurationList)
{
	int	encoderIndex;
	int copConfiguredRes;
	int	newCopRes;
	int	copConfiguredProtocol;
	int resindex;
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

	PTRACE2INT(eLevelInfoNormal, "CComModeH323::FindBestCopLevelAccordingToResolution - AN DEBUG, decoderResolution:", decoderResolution);

	//CCOPConfigurationList* 	pCOPConfigurationList = pConf->GetCommConf()->GetCopConfigurationList();
	CCopVideoParams* 		pCopLevelParams;

  	newCopRes = CopTranslateConfResolutionToConfigureResolution(decoderResolution);

  	PTRACE2INT(eLevelInfoNormal, "CComModeH323::FindBestCopLevelAccordingToResolution - AN DEBUG newCopRes:", newCopRes);

  	if (newCopRes == -1)
  		return capCode;
  	//first look for matching res with the same cap
  	//second look for same res with different protocol
  	//third look for lower res with the same protocol
  	//if else fail return lower res with different protocol
  	int FoundResWithTheDiffCap = -1;
  	int OriginalProtocol=GetProtocolFromCapAndProfile(capCode,capProfile);
  	for( resindex = newCopRes;resindex > 0 ;resindex--)
  	{
  		PTRACE2INT(eLevelInfoNormal, "CComModeH323::FindBestCopLevelAccordingToResolution - AN DEBUG resindex:", resindex);
  		FoundResWithTheDiffCap = -1;
  		for (encoderIndex = 0; encoderIndex < NUMBER_OF_COP_LEVELS; encoderIndex++)
  		{
  			PTRACE2INT(eLevelInfoNormal, "CComModeH323::FindBestCopLevelAccordingToResolution - AN DEBUG encoder index:", encoderIndex);
  			pCopLevelParams = pCOPConfigurationList->GetVideoMode(encoderIndex);

  			copConfiguredRes = pCopLevelParams->GetFormat();
  			copConfiguredProtocol = pCopLevelParams->GetProtocol();
  			BYTE FrameRateOfLevel = pCopLevelParams->GetFrameRate();
  			if (copConfiguredRes == resindex)
  			{
  				if(OriginalProtocol == copConfiguredProtocol)
  				{
  					PTRACE2INT(eLevelInfoNormal, "CComModeH323::FindBestCopLevelAccordingToResolution -found res with same cap this is cap", capCode);
  					decoderResolution = ConvertEncoderResToDecoderRes(copConfiguredRes,FrameRateOfLevel,capCode);
  					return capCode;
  				}
  				else
  				{
  					if( (copConfiguredProtocol == VIDEO_PROTOCOL_H263 ||copConfiguredProtocol == VIDEO_PROTOCOL_H261) && ( copConfiguredRes == eCopLevelEncoderVideoFormat_4CIF || copConfiguredRes == eCopLevelEncoderVideoFormat_4CIF_16_9) && systemCardsBasedMode == eSystemCardsMode_breeze  )
  						PTRACE(eLevelInfoNormal, "CComModeH323::FindBestCopLevelAccordingToResolution -found 4cif res but it is H2634CIF on mpmx and can't be use due to lack of resources.");
  					else
  						FoundResWithTheDiffCap = encoderIndex;
  				}

		    }
  		}
  		if(FoundResWithTheDiffCap != -1)
  		{
  			PTRACE(eLevelInfoNormal,"CComModeH323::CopFindBestLevelAccordingToResolution -found res with diff cap");
  			CCopVideoParams* chosenParams = pCOPConfigurationList->GetVideoMode(FoundResWithTheDiffCap);
  			int copChosenRes = chosenParams->GetFormat();
  			int ChosenFrameRate = chosenParams->GetFrameRate();
  			int ChosenProtocol = chosenParams->GetProtocol();
  			capCode = GetCapAndProfileFromProtocol(ChosenProtocol,capProfile);
  			decoderResolution = ConvertEncoderResToDecoderRes(copChosenRes,ChosenFrameRate,capCode);

  			return capCode;
  		}
	}

	return capCode;
}
///////////////////////////////////////////////////////////////////////////////
int	CComModeH323::GetProtocolFromCapAndProfile(CapEnum capCode,APIU16 profile)
{
	if (capCode == eH264CapCode && profile == H264_Profile_BaseLine)
		return VIDEO_PROTOCOL_H264;
	if(capCode == eH264CapCode && profile == H264_Profile_High)
		return VIDEO_PROTOCOL_H264_HIGH_PROFILE;
	if(capCode == eH263CapCode)
		return VIDEO_PROTOCOL_H263;
	else
	{
		DBGPASSERT((DWORD)capCode+1000);
		return -1;
	}

}
////////////////////////////////////////////////////////
CapEnum CComModeH323::GetCapAndProfileFromProtocol(int protocol,APIU16& profile)
{
	if(protocol == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
	{
		profile = H264_Profile_BaseLine;
		return eH264CapCode;
	}
	if(protocol == VIDEO_PROTOCOL_H264)
	{
		profile = H264_Profile_BaseLine;
		return eH264CapCode;
	}
	if(protocol == VIDEO_PROTOCOL_H263)
	{
		return eH263CapCode;
	}
	else
	{
		DBGPASSERT((DWORD)protocol+1000);
		return eH263CapCode;
	}

}
////////////////////////////////////////////////////
ECopDecoderResolution CComModeH323::ConvertEncoderResToDecoderRes(int EncoderRes,BYTE FrameRate,CapEnum capCode)
{
	if(EncoderRes == eCopLevelEncoderVideoFormat_HD1080p)
	{
		return COP_decoder_resolution_HD108030;

	}
	else if(EncoderRes == eCopLevelEncoderVideoFormat_HD720p)
	{
		if(FrameRate == ((BYTE)eCopVideoFrameRate_50))
			return COP_decoder_resolution_HD720p50;
		else
			return COP_decoder_resolution_HD720p25;
	}
	else if(EncoderRes == eCopLevelEncoderVideoFormat_4CIF_16_9)
	{
		return COP_decoder_resolution_W4CIF25;

	}
	else if(EncoderRes == eCopLevelEncoderVideoFormat_4CIF)
	{
		if(FrameRate == ((BYTE)eCopVideoFrameRate_50) && capCode ==  eH264CapCode)
			return COP_decoder_resolution_4CIF50;
		else
			return COP_decoder_resolution_4CIF25;
	}
	else if(EncoderRes == eCopLevelEncoderVideoFormat_CIF)
	{
		return COP_decoder_resolution_CIF25;
	}
	else if(EncoderRes == eCopLevelEncoderVideoFormat_QCIF)
	{
		DBGPASSERT(EncoderRes+1001);
		return COP_decoder_resolution_CIF25;
	}
	else{

		DBGPASSERT(EncoderRes+1000);
		return COP_decoder_resolution_CIF25;

	}
}

/////////////////////////////////////////////////////////////////////////////
void  CComModeH323::SetVideoRxModeAccordingDecoderResolution(ECopDecoderResolution decoderResolution, CapEnum capCode,APIU16 capProfile,CCOPConfigurationList* pCOPConfigurationList,
		BOOL bIsCascase, BOOL bIsPartyMasterOrSlaveLecturer)
{
	CCopVideoModeTable *pCopTable = new CCopVideoModeTable;

	PTRACE2INT (eLevelInfoNormal, "CComModeH323::SetVideoRxModeAccordingDecoderResolution - AN DEBUG, decoder resolution:", decoderResolution);
	PTRACE2INT (eLevelInfoNormal, "CComModeH323::SetVideoRxModeAccordingDecoderResolution - AN DEBUG, current capCode:", capCode);

	// ICBC - When resolution change, need to change also protocol, otherwise slave change to secondary
	// e.g. from HD to CIF. If conference levels has CIf only for H263 and master ask for H264CIF, slave change to secondary
	// cause it's level with CIF is H263 and not H264. This function will change also the protocol, not just resolution in case of COP, cascade and
	// master is changing the resolution, asuuming that both sides have same levels..
	//if (bIsCascase && bIsPartyMasterOrSlaveLecturer && pCOPConfigurationList)
	//	capCode = CopFindBestLevelAccordingToResolution(decoderResolution, capCode,capProfile ,pCOPConfigurationList);

	//PTRACE2INT (eLevelInfoNormal, "CComModeH323::SetVideoRxModeAccordingDecoderResolution - AN DEBUG, new capCode:", capCode);
	//PTRACE2INT (eLevelInfoNormal, "CComModeH323::SetVideoRxModeAccordingDecoderResolution - AN DEBUG, new profile:", capProfile);
	//PTRACE2INT (eLevelInfoNormal, "CComModeH323::SetVideoRxModeAccordingDecoderResolution - AN DEBUG, new decoder res:", decoderResolution);
 //in 4.7.1 the same profile asummption is not valid.
	if (capCode == eH264CapCode)
	{
		sCopH264VideoMode copH264VideoMode;
		WORD res = pCopTable->GetDecoderH264Mode(decoderResolution, copH264VideoMode);
		if (res == (WORD)-1)
			DBGPASSERT(decoderResolution+1000);
		SetH264Scm(capProfile, copH264VideoMode.levelValue, copH264VideoMode.maxMBPS, copH264VideoMode.maxFS, copH264VideoMode.maxDPB, copH264VideoMode.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, copH264VideoMode.maxStaticMbps, cmCapReceive);
	}
	else if(capCode == eH263CapCode)
	{
		int qcifMpi = -1, cifMpi = -1, cif4Mpi = -1, cif16Mpi = -1;
		pCopTable->GetDecoderH263Mode(decoderResolution, qcifMpi, cifMpi,cif4Mpi,cif16Mpi);
		SetScmMpi(capCode, qcifMpi, cifMpi, cif4Mpi, cif16Mpi, cmCapReceive);
	}
	else
		DBGPASSERT(capCode+1000);
	POBJDELETE(pCopTable);
}
//////////////////////////////////////////////////////////////////////////
DWORD CComModeH323::CalcCopMinFlowControlRate (const CCommConf* pCommConf, DWORD dwFlowControlRate,DWORD contentRate)
{
    PTRACE (eLevelInfoNormal, "CComModeH323::CalcCopMinFlowControlRate - gbbd");

	DWORD newBitRate = dwFlowControlRate*100;
    BYTE copLevel = GetCopTxLevel();
    if (copLevel != 0xFF && m_eConfType == kCop)
    {
        CCOPConfigurationList* pCOPConfigurationList = pCommConf->GetCopConfigurationList();
        BYTE copFormat = pCOPConfigurationList->GetVideoMode(copLevel)->GetFormat();
        BYTE copFrameRate = pCOPConfigurationList->GetVideoMode(copLevel)->GetFrameRate();
        BYTE copProtocol = pCOPConfigurationList->GetVideoMode(copLevel)->GetProtocol();
        int xferBitRate = pCOPConfigurationList->GetVideoMode(copLevel)->GetBitRate();
        CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
        DWORD copBitRate = lCapInfo.TranslateReservationRateToIpRate(xferBitRate);
        DWORD actualAudioRate = GetMediaBitRate(cmCapAudio ,cmCapTransmit) * _K_;
        DWORD copLevelThreshold = GetMinBitRateForCopLevel(copFormat, copFrameRate,copProtocol,copBitRate) * _K_ - actualAudioRate;
        //Don't limit to less than the threshold
     //   PTRACE2INT (eLevelInfoNormal, "CComModeH323::CalcCopMinFlowControlRate - COP level treshold ", copLevelThreshold);
       // PTRACE2INT (eLevelInfoNormal, "CComModeH323::CalcCopMinFlowControlRate - COP level flow control ", dwFlowControlRate*100);
        newBitRate = max(copLevelThreshold, dwFlowControlRate*100);
        //don't limit to less than what the channel was opened initially
        DWORD curBitRate = GetMediaBitRate(cmCapVideo ,cmCapTransmit) * 100 ;
     //   PTRACE2INT (eLevelInfoNormal, "CComModeH323::CalcCopMinFlowControlRate - COP current bit rate without content ", curBitRate);
        curBitRate = curBitRate - (contentRate*100);
        PTRACE2INT (eLevelInfoNormal, "CComModeH323::CalcCopMinFlowControlRate - COP  content rate ", contentRate*100);
     //   PTRACE2INT (eLevelInfoNormal, "CComModeH323::CalcCopMinFlowControlRate - updated current bit rate ", curBitRate);
        newBitRate = min(curBitRate, newBitRate);
        PTRACE2INT (eLevelInfoNormal, "CComModeH323::CalcCopMinFlowControlRate - COP rate ", newBitRate);
    }
    return newBitRate;
}

//////////////////////////////////////////////////////////////////////////
// TIP -------------------------------------------------------------------
void CComModeH323::SetTipMode(ETipMode eTipMode)
{
	m_eTipMode = eTipMode;
}
//////////////////////////////////////////////////////////////////////////
BOOL CComModeH323::GetIsTipMode() const
{
	return (m_eTipMode != eTipModeNone);
}
//////////////////////////////////////////////////////////////////////////
BOOL CComModeH323::IsTipNegotiated() const
{
	return (m_eTipMode == eTipModeNegotiated);
}
//////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::isHDContent1080Supported(cmCapDirection direction) const
{
	// Function returns Content Mpi
	BYTE hd1080ContentMpi = 0;
	CBaseVideoCap* pContentScmCap = (CBaseVideoCap*)GetMediaAsCapClass(cmCapVideo, direction,kRolePresentation);
	//BOOL bContentHD1080Enabled = FALSE;
	if(pContentScmCap)
	{
		if(pContentScmCap->GetCapCode() == eH264CapCode)
	    {
		   if(((CH264VideoCap *)pContentScmCap)->IsCapableOfHD1080())
		   {
				  //bContentHD1080Enabled = TRUE;
			   hd1080ContentMpi = ((CH264VideoCap*)pContentScmCap)->GetH264Mpi();
			   PTRACE2INT(eLevelInfoNormal, "CComModeH323::isHDContent1080Supported - HD 1080 Content Mpi: ", hd1080ContentMpi);
		   }
		}
	}
	else
		PTRACE(eLevelInfoNormal, "CComModeH323::isHDContent1080Supported - No Content H264 cap");

	POBJDELETE(pContentScmCap);
	return hd1080ContentMpi;

}
//////////////////////////////////////////////////////////////////////////
BYTE CComModeH323::isHDContent720Supported(cmCapDirection direction) const
{
	// Function returns Content Mpi
	BYTE hd720ContentMpi = 0;
	CBaseVideoCap* pContentScmCap = (CBaseVideoCap*)GetMediaAsCapClass(cmCapVideo, direction,kRolePresentation);
	if(pContentScmCap)
	{
		if(pContentScmCap->GetCapCode() == eH264CapCode)
	    {
		   if(((CH264VideoCap *)pContentScmCap)->IsCapableOfHD720(kRolePresentation))
		   {
				  //bContentHD1080Enabled = TRUE;
			   hd720ContentMpi = ((CH264VideoCap*)pContentScmCap)->GetH264Mpi();
			   PTRACE2INT(eLevelInfoNormal, "CComModeH323::isHDContent720Supported - HD 720 Content Mpi: ", hd720ContentMpi);
		   }
		}
	}
	else
		 PTRACE(eLevelInfoNormal, "CComModeH323::isHDContent720Supported - No Content H264 cap");

	POBJDELETE(pContentScmCap);
	return hd720ContentMpi;

}

////////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetRtcpFeedbackMask(DWORD rtcpFbMask, cmCapDirection direction, ERoleLabel eRole)
{
	if (eRole == kRolePeople)
	{
		if (direction & cmCapReceive)
			m_vidModeRcv.SetRtcpFeedbackMask(rtcpFbMask);
		if (direction & cmCapTransmit)
			m_vidModeXmit.SetRtcpFeedbackMask(rtcpFbMask);
	}
	else if (eRole & kRoleContentOrPresentation)
	{
		if (direction & cmCapReceive)
			m_vidContModeRcv.SetRtcpFeedbackMask(rtcpFbMask);
		if (direction & cmCapTransmit)
			m_vidContModeXmit.SetRtcpFeedbackMask(rtcpFbMask);
	}
}

////////////////////////////////////////////////////////////////////////////////
DWORD CComModeH323::GetRtcpFeedbackMask(cmCapDirection direction,ERoleLabel eRole) const
{
	if (eRole & kRoleContentOrPresentation)
	{
		if (direction & cmCapReceive)
			return (m_vidContModeRcv.GetRtcpFeedbackMask());
		if (direction & cmCapTransmit)
			return(m_vidContModeXmit.GetRtcpFeedbackMask());
	}
	else
	{
		if (direction & cmCapReceive)
			return (m_vidModeRcv.GetRtcpFeedbackMask());
		if (direction & cmCapTransmit)
			return (m_vidModeXmit.GetRtcpFeedbackMask());
	}
	return 0;
}





VIDEO_OPERATION_POINT_SET_S* CComModeH323::GetOperationPoints(cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole) const
{
	VIDEO_OPERATION_POINT_SET_S* pVideoOperationPointSt = NULL;

	CBaseCap *pCap = GetMediaAsCapClass(aMediaType,direction, eRole);
	if (pCap)
	{
		if(direction==cmCapReceive)
		{
			pVideoOperationPointSt = pCap->GetOperationPoints();
		}

	}

	POBJDELETE(pCap);

	return pVideoOperationPointSt;
}


void CComModeH323::SetStreamsGroup(STREAM_GROUP_S& rStreamGroup, cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole)
{
	CBaseCap *pCap = GetMediaAsCapClass(aMediaType,direction, eRole);
	if (pCap)
	{
		if(direction==cmCapReceive)
		{
			pCap->SetRecvStreamsGroup(rStreamGroup);
		}
		else
		{
			pCap->SetSendStreamsGroup(rStreamGroup);
		}
	}

	POBJDELETE(pCap);
}
STREAM_GROUP_S* CComModeH323::GetStreamsGroup(cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole) const
{
	STREAM_GROUP_S* pStreamGroup = NULL;
	//CapEnum capInfo = (CapEnum)GetMediaType(aMediaType, direction, eRole);
	CBaseCap *pCap = GetMediaAsCapClass(aMediaType,direction, eRole);
	if (pCap)
	{
		if(direction==cmCapReceive)
		{
			pStreamGroup = pCap->GetRecvStreamsGroup();
		}
		else
		{
			pStreamGroup = pCap->GetSendStreamsGroup();
		}
	}

	POBJDELETE(pCap);

	return pStreamGroup;
}
BOOL CComModeH323::SetSsrcIds(cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole, APIU32 *aSsrcIds, int aNumOfSsrcIds, bool aIsUpdate)
{
    if (aNumOfSsrcIds == 0 || aSsrcIds == NULL)
    {
    	PTRACE(eLevelInfoNormal,"CComModeH323::SetSsrcIds No SSRC IDs are given.");
        return FALSE;
    }

    if (aMediaType == cmCapAudio)
    {
        // check if SAC codec is part of the SipCaps
        CapEnum capInfo = (CapEnum)GetMediaType(cmCapAudio, cmCapReceive, kRolePeople);
        if (!CSacAudioCap::IsSacAudio(capInfo))
        {// no stream to update
        	PTRACE(eLevelInfoNormal,"CComModeH323::SetSsrcIds Audio - No SAC");
            return FALSE;
        }

        if (aNumOfSsrcIds > 1)
        {
        	PTRACE(eLevelInfoNormal,"CComModeH323::SetSsrcIds Audio: Too many SSRC Ids are given. Using only the first one.");
        }
        // update the stream id
        CSacAudioCap *pCap = (CSacAudioCap *)GetMediaAsCapClass(cmCapAudio, cmCapReceive, kRolePeople);
        if (!pCap)
        {
        	PTRACE(eLevelInfoNormal,"CComModeH323::SetSsrcIds Cannot get the audio cap");
            return FALSE;
        }
        // add support for more audio codecs - make sure SetRecvSsrcId & AddRecvStream are implemented
        BOOL res = FALSE;
        if (aIsUpdate)
        {
            res = pCap->SetRecvSsrcId(aSsrcIds[0]);
        }
        else
        {
            res = pCap->AddRecvStream(aSsrcIds[0]);
        }
        POBJDELETE(pCap);
        return res;
    }
    else if (aMediaType == cmCapVideo)
    {
        // check if SVC codec is part of the SipCaps
        CapEnum capInfo = (CapEnum)GetMediaType(cmCapVideo, cmCapReceive, kRolePeople);
        if (capInfo != eSvcCapCode)
        {
        	PTRACE(eLevelInfoNormal,"CComModeH323::SetSsrcIds capInfo != eSvcCapCode");
            return FALSE;
        }
        if (aNumOfSsrcIds > 3)
        {
        	PTRACE(eLevelInfoNormal,"CComModeH323::SetSsrcIds aMediaType aNumOfSsrcIds > 3");

        	PTRACE(eLevelInfoNormal,"Video: Too many SSRC Ids are given. Using only the first 3 ones.");
        }
        // support more audio codecs
        CSvcVideoCap *pCap = (CSvcVideoCap *)GetMediaAsCapClass(cmCapVideo, cmCapReceive, kRolePeople);
        if (!pCap)
        {
        	PTRACE(eLevelInfoNormal,"CComModeH323::SetSsrcIds aMediaType !pCap");

        	PTRACE(eLevelInfoNormal,"Cannot get the video cap");
            return FALSE;
        }
        BOOL res = FALSE;
//        if (aIsUpdate)
//        {
//        	PTRACE(eLevelInfoNormal,"CComModeH323::SetSsrcIds aMediaType aIsUpdate");
//
//        	// TBD - will be supported in the future
////            res = pCap->SetRecvSsrcId(aSsrcIds[0]);
//        	res = pCap->AddRecvStream(aSsrcIds,aNumOfSsrcIds);
//        }
//        else
//        {
//        	PTRACE(eLevelInfoNormal,"CComModeH323::SetSsrcIds calling pCap->AddRecvStream");
//
//            res = pCap->AddRecvStream(aSsrcIds,aNumOfSsrcIds);
//        }
        res = pCap->AddRecvStream(aSsrcIds, aNumOfSsrcIds, aIsUpdate);
        POBJDELETE(pCap);
        return res;



        // check how many streams are in the cap - @@@ to be done
        return TRUE; /* 3  */
    }
    return FALSE;
}

BOOL CComModeH323::GetSsrcIds(cmCapDataType aMediaType, cmCapDirection aDirection ,APIU32*& aSsrcIds, int* aNumOfSsrcIds)
{

    if (aMediaType == cmCapAudio)
    {
        // take the SSRC from the sreamDesc
        if (aDirection == cmCapReceive)
            m_audModeRcv.GetSsrcIds(aSsrcIds, *aNumOfSsrcIds);
        else
            m_audModeXmit.GetSsrcIds(aSsrcIds, *aNumOfSsrcIds);
        return TRUE;

    }
    else if (aMediaType == cmCapVideo)
    {
        if (aDirection == cmCapReceive)
            m_vidModeRcv.GetSsrcIds(aSsrcIds, *aNumOfSsrcIds);
        else
            m_vidModeXmit.GetSsrcIds(aSsrcIds, *aNumOfSsrcIds);
        return TRUE;
    }

    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////////////////////  CMediaModeH323 Mode H323///////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Constructor: Sets all values to zero.
//---------------------------------------------------------------------------
//CMediaModeH323::CMediaModeH323():m_type(eUnknownAlgorithemCapCode),m_dataLength(0),m_dataCap(NULL),m_isMute(NO){}
CMediaModeH323::CMediaModeH323()
{
	InitAllMediaModeMembers();
}

void CMediaModeH323::InitAllMediaModeMembers()
{
	m_type = eUnknownAlgorithemCapCode;
	m_dataLength = 0;
	m_dataCap = NULL;
	m_isMute = NO;
	m_pSdesData = NULL;
	m_payloadType = 0;
	m_streams.clear();
	m_pDtlsData = NULL; //_dtls_
	m_IvrSsrc = 0;
}

/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//Destructor: Deletes dataCap array.
//---------------------------------------------------------------------------
CMediaModeH323::~CMediaModeH323()
{
	PDELETEA(m_dataCap);
	if(m_pSdesData)
	{
		m_pSdesData->FreeStruct();
		POBJDELETE(m_pSdesData);
	}
	if(m_pDtlsData) //_dtls_
	{
		m_pDtlsData->FreeStruct();
		POBJDELETE(m_pDtlsData);
	}
	m_streams.clear();
}

/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//Copy-constructor:
//---------------------------------------------------------------------------
CMediaModeH323::CMediaModeH323(const CMediaModeH323& other) :
CPObject(other)
{
	m_type = other.m_type;
	m_payloadType = other.m_payloadType;
	SetLength(other.m_dataLength) ;

	if (other.m_dataLength == 0)
		m_dataCap = NULL;
	else
	{
		m_dataCap = new BYTE[other.m_dataLength];
		memcpy(m_dataCap, other.m_dataCap,other.m_dataLength);
	}
	m_isMute = other.m_isMute;

	m_pSdesData = NULL;
	if(other.m_pSdesData != NULL) {
		//m_pSdesData = new CSdesCap;
		sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE[other.m_pSdesData->SizeOf()]);
		memcpy (pStruct, other.m_pSdesData->GetStruct(), other.m_pSdesData->SizeOf());
		m_pSdesData = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pStruct);
	}

	//_dtls_
	m_pDtlsData = NULL;
	if(other.m_pDtlsData != NULL) {
		//m_pDtlsData = new CDtlsCap;
		sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE[other.m_pDtlsData->SizeOf()]);
		memcpy (pStruct, other.m_pDtlsData->GetStruct(), other.m_pDtlsData->SizeOf());
		m_pDtlsData = (CDtlsCap*)CBaseCap::AllocNewCap((CapEnum)eDtlsCapCode,pStruct);
	}

	m_payloadType = other.m_payloadType;
	m_streams.clear();
	m_streams.assign(other.m_streams.begin(), other.m_streams.end());
	m_IvrSsrc = other.m_IvrSsrc;
}

/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//Create: Sets new type, data and data length.
//---------------------------------------------------------------------------
void CMediaModeH323::Create(WORD newType, WORD newDataLength, const BYTE newData[], unsigned char payloadType)
{
	SetType (newType);
	AllocNewDataCap (newDataLength);
	SetDataCap (newData);

	//Amihay: MRM CODE
	SetPayloadType(payloadType);
}

/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//Create: Sets new type, data and data length from a capBuffer.
//---------------------------------------------------------------------------
void CMediaModeH323::Create(const capBuffer* newMode)
{
	if(newMode == NULL) {
		PASSERT_AND_RETURN(!newMode);
	}

	Create(newMode->capTypeCode, newMode->capLength, newMode->dataCap, newMode->sipPayloadType);
}


/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::Create(const CBaseCap* pCap)
{
	CapEnum eCapCode	= pCap->GetCapCode();
	int    length		= pCap->SizeOf();
	BYTE* data			= (BYTE *)pCap->GetStruct();
	Create(eCapCode, length, data);
}

/////////////////////////////////////////////////////////////////////////////
void  CMediaModeH323::Serialize(WORD format,CSegment& seg) const
{
	switch (format)
	{
		case SERIALEMBD:
			break;
		case NATIVE:
		{
			seg << m_type << m_dataLength;
			for(WORD i=0 ; i<m_dataLength ; i++)
				seg << m_dataCap[i];
			seg << m_isMute;
			if (m_pSdesData)
			{
				unsigned int len = (unsigned int) m_pSdesData->SizeOf();
				seg << len;
				sdesCapStruct* pStruct = (sdesCapStruct*)m_pSdesData->GetStruct();
				seg.Put((BYTE *)pStruct, len);
			}
			else
				seg << (unsigned int)0;

			//_dtls_
			if (m_pDtlsData)
			{
				unsigned int len = (unsigned int) m_pDtlsData->SizeOf();
				seg << len;
				sdesCapStruct* pStruct = (sdesCapStruct*)m_pDtlsData->GetStruct();
				seg.Put((BYTE *)pStruct, len);
			}
			else
				seg << (unsigned int)0;

			seg << m_payloadType;

			WORD numOfStreams = m_streams.size();
			seg << numOfStreams;
			std::list<StreamDesc>::const_iterator it;
			for (it = m_streams.begin();it != m_streams.end();it++)
				((StreamDesc)(*it)).Serialize(format, seg);

			seg << m_IvrSsrc;
			break;
		}
		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CMediaModeH323::DeSerialize(WORD format,CSegment& seg)
{
	switch (format)
	{
		case SERIALEMBD:
			break;
		case NATIVE :
			{
				WORD dataLength;
				seg >> m_type >> dataLength;
				AllocNewDataCap (dataLength);
				for(WORD i=0 ; i<m_dataLength ; i++)
					seg >> m_dataCap[i];
				seg >> m_isMute;
				unsigned int sdesLen = 0;
				seg >> sdesLen;
				if (sdesLen)
				{
					sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE [sdesLen]);
					seg.Get ((BYTE *)pStruct, sdesLen);
					//m_pSdesData = new CSdesCap;
					m_pSdesData = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pStruct);

				}
				else
				{
					if(m_pSdesData)
					{
						m_pSdesData->FreeStruct();
						POBJDELETE(m_pSdesData);
						m_pSdesData = NULL;
					}
				}

				//_dtls_
				unsigned int dtlsLen = 0;
				seg >> dtlsLen;
				if (dtlsLen)
				{
					sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE [dtlsLen]);
					seg.Get ((BYTE *)pStruct, dtlsLen);
					//m_pDtlsData = new CDtlsCap;
					m_pDtlsData = (CDtlsCap*)CBaseCap::AllocNewCap((CapEnum)eDtlsCapCode,pStruct);

				}
				else
				{
					if(m_pDtlsData) //_dtls_
					{
						m_pDtlsData->FreeStruct();
						POBJDELETE(m_pDtlsData);
						m_pDtlsData = NULL;
					}
				}
				seg >> m_payloadType;

				WORD numOfStreams = 0;
				seg >> numOfStreams;
				m_streams.clear();
				for (int i=0;i<numOfStreams;i++)
				{
					StreamDesc streamDesc;
					streamDesc.DeSerialize(format, seg);
					m_streams.push_back(streamDesc);
				}
				seg >> m_IvrSsrc;
				break;
			}
		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMediaModeH323::IsValidMode(void) const
{
	return (m_type < eUnknownAlgorithemCapCode);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMediaModeH323::IsModeOff(void) const
{
	return !m_dataLength;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMediaModeH323::IsMediaOn() const
{
	return (m_dataLength != 0);
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetModeOff(void)
{
    PDELETEA (m_dataCap);
	m_dataLength = 0;
    m_type = eUnknownAlgorithemCapCode;
}

/////////////////////////////////////////////////////////////////////////////
WORD CMediaModeH323::GetType(void) const
{
	return m_type;
}

/////////////////////////////////////////////////////////////////////////////
BYTE *CMediaModeH323::GetDataCap(void) const
{
	return m_dataCap;
}

/////////////////////////////////////////////////////////////////////////////
WORD CMediaModeH323::GetLength(void) const
{
	return m_dataLength;
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::CopyData(BYTE data[]) const
{
	PASSERT_AND_RETURN(data == NULL);
	if (m_dataLength != 0)
		memcpy(data, m_dataCap, m_dataLength);
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::CopyToCapBuffer(capBuffer* pCapBuffer,BYTE bOperUse) const
{
	PASSERT_AND_RETURN( !pCapBuffer );
	pCapBuffer->capTypeCode = m_type;
	pCapBuffer->capLength = m_dataLength;
	if (m_dataLength != 0)
		memcpy(pCapBuffer->dataCap,m_dataCap,m_dataLength);
/*	if (bOperUse)
	{
		CCapSetInfo capInfo = (CapEnum)m_type;
		pCapBuffer->capTypeCode = capInfo.GetOperCapCode();
		if (capInfo.IsCapCode(eG7231CapCode))
			pCapBuffer->capLength = (m_dataLength - sizeof(APIU16));
		else if (capInfo.GetCapType() == cmCapAudio)
			pCapBuffer->capLength = (m_dataLength - sizeof(APIU32));
	}*/
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::CopySdesToCapBuffer(capBuffer* pCapBuffer,BYTE bOperUse) const
{
	PASSERT_AND_RETURN( !pCapBuffer );
	pCapBuffer->capTypeCode = eSdesCapCode;
	pCapBuffer->capLength = m_pSdesData->SizeOf();
	if (m_pSdesData->SizeOf() != 0)
		memcpy(pCapBuffer->dataCap,m_pSdesData->GetStruct(),m_pSdesData->SizeOf());

}

///////////////////////////////////////////////////////////////////////////// //_dtls_
void CMediaModeH323::CopyDtlsToCapBuffer(capBuffer* pCapBuffer,BYTE bOperUse) const
{
	PASSERT_AND_RETURN( !pCapBuffer );
	pCapBuffer->capTypeCode = eDtlsCapCode;
	pCapBuffer->capLength = m_pDtlsData->SizeOf();
	if (m_pDtlsData->SizeOf() != 0)
		memcpy(pCapBuffer->dataCap,m_pDtlsData->GetStruct(),m_pDtlsData->SizeOf());

}

/////////////////////////////////////////////////////////////////////////////
CMediaModeH323& CMediaModeH323::operator=(const CMediaModeH323& other)
{
	if(this == &other)
		return *this;

	m_type = other.m_type;
	AllocNewDataCap (other.m_dataLength);
	SetDataCap (other.m_dataCap);
	m_isMute = other.m_isMute;

	m_pSdesData = NULL;
	if(other.m_pSdesData != NULL) {
		//m_pSdesData = new CSdesCap;
		sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE[other.m_pSdesData->SizeOf()]);
		memcpy (pStruct, other.m_pSdesData->GetStruct(), other.m_pSdesData->SizeOf());
		m_pSdesData = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pStruct);
	}

	//_dtls_
	m_pDtlsData = NULL;
	if(other.m_pDtlsData != NULL) {
		//m_pDtlsData = new CDtlsCap;
		sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE[other.m_pDtlsData->SizeOf()]);
		memcpy (pStruct, other.m_pDtlsData->GetStruct(), other.m_pDtlsData->SizeOf());
		m_pDtlsData = (CDtlsCap*)CBaseCap::AllocNewCap((CapEnum)eDtlsCapCode,pStruct);
	}

	m_payloadType = other.m_payloadType;
	m_streams.clear();
	m_streams.assign(other.m_streams.begin(), other.m_streams.end());
	m_IvrSsrc = other.m_IvrSsrc;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMediaModeH323::IsRole(ERoleLabel eRole) const
{
	CBaseCap* pCap  = CBaseCap::AllocNewCap((CapEnum)m_type,m_dataCap);
	BYTE bIsRole = FALSE;

	if (pCap)
		bIsRole = pCap->GetRole() & eRole;

	PDELETE(pCap);
	return bIsRole;
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::GetMediaParams(CSecondaryParams &secParams, DWORD details) const
{
	CBaseCap* pCap  = CBaseCap::AllocNewCap((CapEnum)m_type,m_dataCap);

	if (pCap)
		pCap->GetMediaParams(secParams, details);

	PDELETE(pCap);
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::GetDiffFromDetails(DWORD details, CSecondaryParams &secParams) const
{
	CBaseCap* pCap  = CBaseCap::AllocNewCap((CapEnum)m_type,m_dataCap);

	if (pCap)
		pCap->GetDiffFromDetails(details,secParams);

	PDELETE(pCap);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMediaModeH323::IsContaining(const CMediaModeH323& other, DWORD valuesToCompare, DWORD* pDetails) const
{
	BYTE bRes = FALSE;

	// Type are the same. Can compare parameters
	if (m_type == other.m_type)
	{
		CBaseCap* pThisCap  = CBaseCap::AllocNewCap((CapEnum)m_type,m_dataCap);
		CBaseCap* pOtherCap = CBaseCap::AllocNewCap((CapEnum)other.m_type,other.m_dataCap);

		if (pThisCap && pOtherCap)
		{
			if (valuesToCompare == kCapCode)
				bRes = TRUE; //the function was asked to compare only the protocol
			else
			{
				bRes = pThisCap->IsContaining(*pOtherCap,valuesToCompare, pDetails);

				if (bRes == FALSE)
				{
					CSmallString msg;
					cmCapDataType eType = pThisCap->GetType();
					DumpDetailsToStream(eType,*pDetails,msg);
					FPTRACE2(eLevelInfoNormal,"CMediaModeH323::IsContaining: ",msg.GetString());
				}

				if (bRes && (valuesToCompare & kNumOfStreamDesc))
				{
					const std::list <StreamDesc> thisStreamsDescList = GetStreams();
					const std::list <StreamDesc> otherStreamsDescList = other.GetStreams();
					TRACEINTO << "This streams=" << (int)thisStreamsDescList.size() << " other streams="<< (int)otherStreamsDescList.size();

					if (thisStreamsDescList.size() != otherStreamsDescList.size())
					{
						TRACEINTO << "Different number of streams. this streams=" << (int)thisStreamsDescList.size() << " other streams="<< (int)otherStreamsDescList.size();
						bRes = FALSE;
					}

				}

			}
		}

        PDELETE(pThisCap);
		PDELETE(pOtherCap);
	}
	else
	{
		FPTRACE(eLevelInfoNormal,"CMediaModeH323::IsContaining: Types are different!!!");
		*pDetails |= DIFFERENT_CAPCODE;
	}

	if (bRes)
		FPTRACE(eLevelInfoNormal,"CMediaModeH323::IsContaining: Returned TRUE");

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMediaModeH323::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD* pDetails) const
{
	BYTE bRes = FALSE;
	CBaseCap* pThisCap  = CBaseCap::AllocNewCap((CapEnum)m_type,m_dataCap);
	if (m_type == other.GetCapCode())
	{
		if (valuesToCompare == kCapCode)
			bRes = TRUE; //the function was asked to compare only the protocol
		else if( pThisCap )
			bRes = pThisCap->IsContaining(other,valuesToCompare,pDetails);
	}
	PDELETE(pThisCap);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMediaModeH323::IsEquals(const CBaseCap& other,DWORD valuesToCompare) const
{
	BYTE bRes = FALSE;
	CCapSetInfo otherCapInfo = other.GetCapCode();
	if (m_type == (CapEnum)otherCapInfo)
	{
		if (valuesToCompare != kCapCode)
		{
			CSmallString	msg;
			DWORD			details = 0;
			cmCapDataType	eType	= other.GetType();
			bRes = IsContaining(other,valuesToCompare,&details);
			if (bRes)
			{
				CBaseCap* pCap = GetAsCapClass();
				if (pCap)
				{
					bRes = other.IsContaining(*pCap,valuesToCompare,&details);
					if (bRes == FALSE)
					{
						DumpDetailsToStream(eType,details,msg);
						PTRACE2(eLevelInfoNormal,"CMediaModeH323::IsEquals, Other does not contain this: ",msg.GetString());
					}
				}
				POBJDELETE(pCap);
			}
			else
			{
				DumpDetailsToStream(eType,details,msg);
				PTRACE2(eLevelInfoNormal,"CMediaModeH323::IsEquals, This does not contain other: ",msg.GetString());
			}
		}
		else // kCapCode was already compared
			bRes = TRUE;
	}
	else
		PTRACE2(eLevelInfoNormal,"CMediaModeH323::IsEquals, Type is not equals: ",otherCapInfo.GetH323CapName());
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
CBaseCap* CMediaModeH323::GetAsCapClass() const
{
	CBaseCap* pCap = NULL;
	if (m_dataCap)
		pCap = CBaseCap::AllocNewCap((CapEnum)m_type,m_dataCap);
	return pCap;
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetType(WORD newType)
{
	if(newType >= eUnknownAlgorithemCapCode)
		PASSERTMSG(1,"CMediaModeH323::SetType newType >= eUnknownAlgorithmCapCode");
	m_type			=	newType;
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetLength(WORD newDataLength)
{
	if(newDataLength > MAX_MEDIA_CAP_LENGTH){
		PASSERTMSG(newDataLength,"CMediaModeH323::SetLength - newDataLength > MAX_MEDIA_CAP_LENGTH");
		// added to prevent exception where newDataLength > max
		m_dataLength = 0;
		return;
	}
	m_dataLength	=	newDataLength;
}

/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//AllocNewDataCap: Allocate new array with new size.
//				   if size is zero, dataCap will be NULL.
//---------------------------------------------------------------------------
void CMediaModeH323::AllocNewDataCap(WORD newDataLength)
{
	if (m_dataLength != newDataLength)
	{
        //delete and set to NULL
		PDELETEA (m_dataCap);
		if (newDataLength != 0)
        {
        	m_dataCap = new BYTE[newDataLength];
        	PASSERT_AND_RETURN (m_dataCap == NULL);
        	memset(m_dataCap, 0, newDataLength);
        }
    }
	//else: new length equals to allocated lenght, there is no need to alloc new memory.
	SetLength(newDataLength);
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetDataCap (const BYTE newData[])
{
	if (m_dataLength != 0)
	{
		PASSERT_AND_RETURN(newData == NULL);
		memcpy(m_dataCap,newData,m_dataLength);

	}
}

/////////////////////////////////////////////////////////////////////////////
/*void CMediaModeH323::Dump(void) const
{
	PTRACE(eLevelInfoNormal,"CMediaModeH323::Dump");

	WORD        msgBufSize = 4096;
	char*       msgStr = new char[msgBufSize];
	COstrStream msg(msgStr,msgBufSize);

	msg.setf(ios::left,ios::adjustfield);
	msg.setf(ios::showbase);

	DumpToStream(msg);

	PTRACE(eLevelInfoNormal,msg.str().c_str());
	PDELETEA(msgStr); */
//}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::Dump(std::ostream& msg, const char* title) const
{
	cmCapDataType type = GetDataType();
    msg << "\n" << title << "\n";
	msg << "["<< GetTypeStr(type) <<"]:";

	if (IsModeOff())
	{
		msg <<"\nMode is off.\n";
		return;
	}

	CComModeInfo comModeInfo = (CapEnum)m_type;

	if(comModeInfo.IsType(type))
	{
		CBaseCap* pBaseCap = CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

		if (pBaseCap)
		{
			pBaseCap->Dump(msg);
		}

		PDELETE(pBaseCap);

		if(m_pSdesData) {
			msg << "\nSDES cap:";
			m_pSdesData->Dump(msg);
		}

		//_dtls_
		if(m_pDtlsData) {
			msg << "\nDTLS cap:";
			m_pDtlsData->Dump(msg);
		}

		unsigned int unTmpPT=(unsigned int) m_payloadType;
		msg << "\nPayload:" << unTmpPT;
	}
	else
	{
		msg << "\n  HasError                :Unknown capability";
	}

	DumpStreamsList(msg, title);
	msg << "\nIVR SSRC:" << m_IvrSsrc;
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::DumpStreamsList(std::ostream& msg, const char* title) const
{
	if (m_streams.empty())
		return;

	msg << "\n- - - - Streams list : - - - - - - - -" << "\n";
    msg << "Number of streams (" << title << ") = " << m_streams.size() << "\n";
	std::list<StreamDesc>::const_iterator itr_streams;
	for (itr_streams = m_streams.begin();itr_streams != m_streams.end();itr_streams++)
	{
		msg << "ssrc:" << itr_streams->m_pipeIdSsrc;
		if (GetDataType() == cmCapVideo)
		{
			msg << " width:" << itr_streams->m_width;
			msg << " height:" << itr_streams->m_height;
			msg << " frame rate:" << itr_streams->m_frameRate;
		}
		msg << " m_bitRate:" << itr_streams->m_bitRate;
		if (itr_streams->m_specificSourceSsrc)
			msg << " specified ssrc:" << itr_streams->m_sourceIdSsrc;

		msg << " payload type:" << itr_streams->m_payloadType;
		msg << " priority: " << itr_streams->m_priority;
        msg << " isLegal: " << itr_streams->m_isLegal;
        msg << " isAvcToSvcVsw: " << itr_streams->m_isAvcToSvcVsw;
		msg << "\n";
	}

}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetTipDefaultsForEncrypt(BOOL& bIsNeedToSendMKI, BOOL& bLifeTimeInUse , BOOL& bFecKeyInUse)
{
	bIsNeedToSendMKI = FALSE;
	bLifeTimeInUse = FALSE;
	bFecKeyInUse = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetSdesDefaultTxData(cmCapDataType eMediaType, BOOL bIsTipMode, ERoleLabel eRole)
{
	BOOL bLifeTimeInUse   = TRUE;
	BOOL bIsNeedToSendMKI = FALSE;
	BOOL bFecKeyInUse	  = FALSE;
	//BRIDGE-10123
	//CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEND_SRTP_MKI, bIsNeedToSendMKI);
	bIsNeedToSendMKI = 	(CSipCaps::GetUseMkiEncrytionFlag() != eUseNonMkiKeyOnly);

	if(bIsTipMode)
		SetTipDefaultsForEncrypt(bIsNeedToSendMKI,bLifeTimeInUse ,bFecKeyInUse);
	m_pSdesData = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,NULL);

	int sh1Type = GetSha1Type();
	if(m_pSdesData) {
		EResult eResOfSet	= kSuccess;

		eResOfSet &= m_pSdesData->SetStruct(eMediaType, cmCapReceiveAndTransmit, eRole);
		m_pSdesData->SetSdesTag(1);
		m_pSdesData->SetSdesCryptoSuite(sh1Type);
		m_pSdesData->SetNumOfKeysParam(1);
		m_pSdesData->SetIsSdesUnencryptedSrtp(FALSE);
		m_pSdesData->SetIsSdesUnencryptedSrtcp(FALSE);
		m_pSdesData->SetIsSdesUnauthenticatedSrtp(FALSE);
		m_pSdesData->SetIsSdesKdrInUse(FALSE);
		m_pSdesData->SetSdesKdr(0);
		m_pSdesData->SetIsSdesWshInUse(FALSE);
		m_pSdesData->SetSdesWsh(0);
		m_pSdesData->SetIsSdesFecOrderInUse(FALSE);
		m_pSdesData->SetSdesFecOrder(eSrtpUnknownFec);
		m_pSdesData->SetIsSdesFecKeyInUse(bFecKeyInUse);
		m_pSdesData->SetSdesKeyMethod(0, eSdesInlineKeyMethod);
		m_pSdesData->SetSdesBase64KeySalt(0, "ffffffffffffffffffffffffffffffffffffffff");
		m_pSdesData->SetIsSdesLifeTimeInUse(0, bLifeTimeInUse);
		m_pSdesData->SetSdesLifeTime(0, DEFAULT_LIFE_TIME);
		if(bIsNeedToSendMKI)
		{
			m_pSdesData->SetIsSdesMkiInUse(0, TRUE);
			m_pSdesData->SetSdesMkiValue(0, 1);
			m_pSdesData->SetIsSdesMkiValueLenInUse(0, TRUE);
			m_pSdesData->SetSdesMkiValueLen(0,DEFAULT_MKI_VAL_LEN);
		}
		else
		{
			m_pSdesData->SetIsSdesMkiInUse(0, FALSE);
			m_pSdesData->SetSdesMkiValue(0, 0);
			m_pSdesData->SetIsSdesMkiValueLenInUse(0, FALSE);
			m_pSdesData->SetSdesMkiValueLen(0,0);
		}

	} else {
		PTRACE(eLevelInfoNormal,"CMediaModeH323::SetSdesDefaultTxData: m_pSdesData is NULL");
	}
}
/////////////////////////////////////////////////////////////////////////////
int CMediaModeH323::GetSha1Type()
{
	int Sh1Length = GetSdesCapEnumFromSystemFlag();
	PTRACE2INT(eLevelInfoNormal,"GetSdesCapEnumFromSystemFlag Sh1Length: ",Sh1Length);

	switch(Sh1Length)
	{
	case eSha1_length_80:
		return eAes_Cm_128_Hmac_Sha1_80;
		break;
	case eSha1_length_32:
		return eAes_Cm_128_Hmac_Sha1_32;
		break;
	case eSha1_length_80_32:
		return eAes_Cm_128_Hmac_Sha1_32;
		break;
	default:
		PTRACE(eLevelError,"CMediaModeH323 incorrect eTypeOfSha1Length ");
		break;
	}
	return eAes_Cm_128_Hmac_Sha1_32;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetDtlsDefaultTxData(cmCapDataType eMediaType, BOOL bIsTipMode , ERoleLabel eRole )
{
	BOOL bLifeTimeInUse   = TRUE;
	BOOL bIsNeedToSendMKI = FALSE;
	BOOL bFecKeyInUse	  = FALSE;
	//BRIDGE-10123
	//CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEND_SRTP_MKI, bIsNeedToSendMKI);
	bIsNeedToSendMKI = 	(CSipCaps::GetUseMkiEncrytionFlag() != eUseNonMkiKeyOnly);

	if(bIsTipMode)
		SetTipDefaultsForEncrypt(bIsNeedToSendMKI,bLifeTimeInUse ,bFecKeyInUse);

	m_pDtlsData = (CDtlsCap*)CBaseCap::AllocNewCap((CapEnum)eDtlsCapCode,NULL);

	if(m_pDtlsData) {
		EResult eResOfSet	= kSuccess;

		eResOfSet &= m_pDtlsData->SetStruct(eMediaType, cmCapReceiveAndTransmit, eRole);
		m_pDtlsData->SetSdesTag(1);
		m_pDtlsData->SetSdesCryptoSuite(DEFAULT_CRYPTO_SUITE);
		m_pDtlsData->SetNumOfKeysParam(1);
		m_pDtlsData->SetIsSdesUnencryptedSrtp(FALSE);
		m_pDtlsData->SetIsSdesUnencryptedSrtcp(FALSE);
		m_pDtlsData->SetIsSdesUnauthenticatedSrtp(FALSE);
		m_pDtlsData->SetIsSdesKdrInUse(FALSE);
		m_pDtlsData->SetSdesKdr(0);
		m_pDtlsData->SetIsSdesWshInUse(FALSE);
		m_pDtlsData->SetSdesWsh(0);
		m_pDtlsData->SetIsSdesFecOrderInUse(FALSE);
		m_pDtlsData->SetSdesFecOrder(eSrtpUnknownFec);
		m_pDtlsData->SetIsSdesFecKeyInUse(bFecKeyInUse);
		m_pDtlsData->SetSdesKeyMethod(0, eSdesInlineKeyMethod);
		m_pDtlsData->SetSdesBase64KeySalt(0, "dtlsDefaultValue");
		m_pDtlsData->SetIsSdesLifeTimeInUse(0, bLifeTimeInUse);
		m_pDtlsData->SetSdesLifeTime(0, DEFAULT_LIFE_TIME);
		if(bIsNeedToSendMKI)
		{
			m_pDtlsData->SetIsSdesMkiInUse(0, TRUE);
			m_pDtlsData->SetSdesMkiValue(0, 1);
			m_pDtlsData->SetIsSdesMkiValueLenInUse(0, TRUE);
			m_pDtlsData->SetSdesMkiValueLen(0,DEFAULT_MKI_VAL_LEN);
		}
		else
		{
			m_pDtlsData->SetIsSdesMkiInUse(0, FALSE);
			m_pDtlsData->SetSdesMkiValue(0, 0);
			m_pDtlsData->SetIsSdesMkiValueLenInUse(0, FALSE);
			m_pDtlsData->SetSdesMkiValueLen(0,0);
		}

	} else {
		PTRACE(eLevelInfoNormal,"CMediaModeH323::SetDtlsDefaultTxData: m_pDtlsData is NULL");
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetSdesTag(APIU32 tag)
{
	APIU32 tmpTag = 0;
	if(m_pSdesData) {
		m_pSdesData->SetSdesTag(tag);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetSdesBase64MasterSaltKey(char *key)
{
	if(m_pSdesData) {
		m_pSdesData->SetSdesBase64KeySalt(0,key);
	}
}

/////////////////////////////////////////////////////////////////////////////
CSdesCap* CMediaModeH323::GetSdesCap() const
{
	return m_pSdesData;
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
CDtlsCap* CMediaModeH323::GetDtlsCap() const
{
	return m_pDtlsData;
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetSdesCap(CSdesCap* pSdesCap)
{
	if(pSdesCap) {
		m_pSdesData = new CSdesCap;
		sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE[pSdesCap->SizeOf()]);
		memcpy (pStruct, pSdesCap->GetStruct(), pSdesCap->SizeOf());
		POBJDELETE(m_pSdesData);

		m_pSdesData = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pStruct);
	}
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
void CMediaModeH323::SetDtlsCap(CDtlsCap* pDtlsCap)
{
	if(pDtlsCap) {
		m_pDtlsData = new CDtlsCap;
		sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE[pDtlsCap->SizeOf()]);
		memcpy (pStruct, pDtlsCap->GetStruct(), pDtlsCap->SizeOf());
		POBJDELETE(m_pDtlsData);

		m_pDtlsData = (CDtlsCap*)CBaseCap::AllocNewCap((CapEnum)eDtlsCapCode,pStruct);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::RemoveSdesCap()
{
	if(m_pSdesData)
	{
		m_pSdesData->FreeStruct();
		POBJDELETE(m_pSdesData);
	}
}

///////////////////////////////////////////////////////////////////////////// //_dtls_
void CMediaModeH323::RemoveDtlsCap()
{
	if(m_pDtlsData)
	{
		m_pDtlsData->FreeStruct();
		POBJDELETE(m_pDtlsData);
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CMediaModeH323::IsSdesEquals(const CMediaModeH323& other ) const
{
	BYTE res = TRUE;

	CSdesCap* pThisCap   = GetSdesCap();
	CSdesCap* pOtherCap  = other.GetSdesCap();

	if(((!pThisCap) && pOtherCap) || ((!pOtherCap) && pThisCap) || ((!pThisCap) && (!pOtherCap)))
		return TRUE;

	if (pThisCap->GetSdesTag() != pOtherCap->GetSdesTag()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.Tag is different");
	}
	if (pThisCap->GetSdesCryptoSuite() != pOtherCap->GetSdesCryptoSuite()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.CryptoSuite is different");
	}
	if(pThisCap->GetIsSdesFecOrderInUse()!= pOtherCap->GetIsSdesFecOrderInUse()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.IsFecOrderInUse is different");
	}
	if (pThisCap->GetSdesFecOrder() != pOtherCap->GetSdesFecOrder()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.FecOrder is different");
	}
	if(pThisCap->GetIsSdesKdrInUse()!= pOtherCap->GetIsSdesKdrInUse()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.IsSdesKdrInUse is different");
	}
	if (pThisCap->GetSdesKdr() != pOtherCap->GetSdesKdr()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.GetSdesKdr is different");
	}
	if (pThisCap->GetIsSdesWshInUse() != pOtherCap->GetIsSdesWshInUse()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.IsSdesWshInUse is different");
	}
	if (pThisCap->GetSdesWsh() != pOtherCap->GetSdesWsh()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.SdesWsh is different");
	}
	if(pThisCap->GetIsSdesFecKeyInUse()!= pOtherCap->GetIsSdesFecKeyInUse()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.IsSdesFecKeyInUse is different");
	}
	if(pThisCap->GetIsSdesUnauthenticatedSrtp()!= pOtherCap->GetIsSdesUnauthenticatedSrtp()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.IsSdesUnauthenticatedSrtp is different");
	}
	if(pThisCap->GetIsSdesUnencryptedSrtcp()!= pOtherCap->GetIsSdesUnencryptedSrtcp()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.IsSdesUnencryptedSrtcp is different");
	}
	if(pThisCap->GetIsSdesUnencryptedSrtp()!= pOtherCap->GetIsSdesUnencryptedSrtp()) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.IsSdesUnencryptedSrtp is different");
	}

	APIU32 numOfThisKeys = pThisCap->GetSdesNumOfKeysParam();
	APIU32 numOfOtherKeys = pOtherCap->GetSdesNumOfKeysParam();
	if(numOfThisKeys != numOfOtherKeys) {
		res &= FALSE;
		PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.num of keys is different");
		return res;
	}

	for (APIU32 keyNumber = 0; keyNumber < numOfThisKeys; keyNumber++) {
		if (pThisCap->GetSdesKeyMethod(keyNumber) != pOtherCap->GetSdesKeyMethod(keyNumber)) {
			res &= FALSE;
			PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.SdesKeyMethod is different");
		}
		if(pThisCap->GetIsSdesLifeTimeInUse(keyNumber)!= pOtherCap->GetIsSdesLifeTimeInUse(keyNumber)) {
			res &= FALSE;
			PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.IsSdesLifeTimeInUse is different");
		}
		if (pThisCap->GetSdesLifeTime(keyNumber) != pOtherCap->GetSdesLifeTime(keyNumber)) {
			res &= FALSE;
			PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.SdesLifeTime is different");
		}
		if (pThisCap->GetIsSdesMkiInUse(keyNumber) != pOtherCap->GetIsSdesMkiInUse(keyNumber)) {
			res &= FALSE;
			PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.IsSdesMkiInUse is different");
		}
		if (pThisCap->GetSdesMkiValue(keyNumber) != pOtherCap->GetSdesMkiValue(keyNumber)) {
			res &= FALSE;
			PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.SdesMkiValue is different");
		}
		if (pThisCap->GetIsSdesMkiValueLenInUse(keyNumber) != pOtherCap->GetIsSdesMkiValueLenInUse(keyNumber)) {
			res &= FALSE;
			PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.SdesMkiValueLenInUse is different");
		}
		if (pThisCap->GetSdesMkiValueLen(keyNumber) != pOtherCap->GetSdesMkiValueLen(keyNumber)) {
			res &= FALSE;
			PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.SdesMkiValueLen is different");
		}
		if((pThisCap->GetSdesBase64KeySalt(keyNumber) == NULL ) || (pOtherCap->GetSdesBase64KeySalt(keyNumber) == NULL) )
		{
		    res &= FALSE;
		}
		else if ( (pThisCap->GetSdesBase64KeySalt(keyNumber) != NULL) && (pOtherCap->GetSdesBase64KeySalt(keyNumber) != NULL)
		        && strcmp(pThisCap->GetSdesBase64KeySalt(keyNumber), pOtherCap->GetSdesBase64KeySalt(keyNumber)))
		{
			res &= FALSE;
			PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.SdesBase64KeySalt is different");
			/*PTRACE2(eLevelInfoNormal, "This target cap key = ", pThisCap->GetSdesBase64KeySalt(keyNumber));
			PTRACE2(eLevelInfoNormal, "Other cur mode cap key = ", pOtherCap->GetSdesBase64KeySalt(keyNumber));*/
		} else {
			PTRACE(eLevelInfoNormal, "CMediaModeH323::IsSdesEquals.SdesBase64KeySalt is same");
			/*PTRACE2(eLevelInfoNormal, "This target cap key = ", pThisCap->GetSdesBase64KeySalt(keyNumber));
			PTRACE2(eLevelInfoNormal, "Other cur mode cap key = ", pOtherCap->GetSdesBase64KeySalt(keyNumber));*/
		}
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetSdesMkiDefaultParams(BYTE bIsNeedToSendMKI)
{
	if (m_pSdesData)
	{
		if (bIsNeedToSendMKI)
		{
			m_pSdesData->SetIsSdesMkiInUse(0, TRUE);
			m_pSdesData->SetSdesMkiValue(0, 1);
			m_pSdesData->SetIsSdesMkiValueLenInUse(0, TRUE);
			m_pSdesData->SetSdesMkiValueLen(0,DEFAULT_MKI_VAL_LEN);
		}
		else
		{
			m_pSdesData->SetIsSdesMkiInUse(0, FALSE);
			m_pSdesData->SetSdesMkiValue(0, 0);
			m_pSdesData->SetIsSdesMkiValueLenInUse(0, FALSE);
			m_pSdesData->SetSdesMkiValueLen(0,0);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetSdesLifeTimeDefaultParams(BYTE bLifeTimeInUse)
{
	if (m_pSdesData)
		m_pSdesData->SetIsSdesLifeTimeInUse(0, bLifeTimeInUse);
}
/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetSdesFecDefaultParams(BYTE bFecKeyInUse)
{
	if (m_pSdesData)
		m_pSdesData->SetIsSdesFecKeyInUse(bFecKeyInUse);
}
/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::FillStreamsDescList(payload_en payload_type, STREAM_GROUP_S* pStreamGroup)
{
	if(pStreamGroup == NULL)
		return;

	m_streams.clear();
	for(int i = 0;i < pStreamGroup->numberOfStreams; i++)
	{
		StreamDesc streamDesc;
		streamDesc.InitDefaults();
		streamDesc.m_pipeIdSsrc = pStreamGroup->streams[i].streamSsrcId;
		if (GetDataType() == cmCapVideo)
		{
			streamDesc.m_width = pStreamGroup->streams[i].frameWidth;
			streamDesc.m_height = pStreamGroup->streams[i].frameHeight;
			streamDesc.m_frameRate = pStreamGroup->streams[i].maxFrameRate;

		}

		// TODO - scp
		streamDesc.m_specificSourceSsrc = false; //is initiated only from scp request
		streamDesc.m_payloadType = payload_type;
		streamDesc.m_isLegal = true;
		streamDesc.m_isAvcToSvcVsw = false;

		PTRACE2INT(eLevelInfoNormal,"CMediaModeH323::FillStreamsDescList m_payloadType",m_payloadType);
		PTRACE2INT(eLevelInfoNormal,"CMediaModeH323::FillStreamsDescList m_payloadType",streamDesc.m_payloadType);
		m_streams.push_back(streamDesc);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////  Audio Mode H323///////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CAudModeH323::CAudModeH323() {}

/////////////////////////////////////////////////////////////////////////////
WORD CAudModeH323::operator==(const CMediaModeH323& other) const
{
	WORD res = FALSE;
	{
		if (IsModeOff() && other.IsModeOff())
			res = TRUE;
		else if (IsModeOff() || other.IsModeOff())
			res = FALSE;
		else
		{
			if (GetType() == other.GetType())
			{
				CBaseCap* pThisCap  = GetAsCapClass();
				CBaseCap* pOtherCap = other.GetAsCapClass();
				if (pThisCap && pOtherCap)
				{
					DWORD details = 0;
					res = pThisCap->IsContaining(*pOtherCap,kFrameRate,&details);
					if (res)
						res = pOtherCap->IsContaining(*pThisCap,kFrameRate,&details);
				}
				POBJDELETE(pThisCap);
				POBJDELETE(pOtherCap);
			}
		}
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////
void CAudModeH323::SetFramePP(int maxFramePP,int minFramePP)
{
	if (IsModeOff())
	{
		PTRACE(eLevelInfoNormal, "CAudModeH323::SetFramePP: Mode is off!!");
		return;
	}

	CComModeInfo comModeInfo = (CapEnum)m_type;

	if(comModeInfo.IsType(cmCapAudio))
	{
		CBaseAudioCap* pAudioCap = (CBaseAudioCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
		if (pAudioCap)
		{
			EResult eRes = pAudioCap->SetMaxFramePerPacket(maxFramePP);
			eRes &= pAudioCap->SetMinFramePerPacket(minFramePP);
			if (eRes == kFailure)
				PTRACE(eLevelInfoNormal, "CAudModeH323::SetFramePP: Couldn't set value!!");
		}
		PDELETE(pAudioCap);
	}
	else
		PTRACE(eLevelError,"CAudModeH323::SetFramePP: unknown algorithm was found in CComModeH323 object.");
}

//////////////////////////////////////////////////////////////////////////
void CAudModeH323::SetDSHforAvMcu()
{
	if (IsModeOff())
	{
		PTRACE(eLevelInfoNormal, "CAudModeH323::SetDSHforAvMcu: Mode is off!!");
		return;
	}

	CComModeInfo comModeInfo = (CapEnum)m_type;

	if(comModeInfo.IsType(cmCapAudio))
	{
		CBaseAudioCap* pAudioCap = (CBaseAudioCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
		if (pAudioCap)
		{
			EResult eRes = pAudioCap->SetRtcpMask(RTCP_MASK_MS_DSH);
			if (eRes == kFailure)
				PTRACE(eLevelInfoNormal, "CAudModeH323::SetDSHforAvMcu: Couldn't set value!!");
		}
		PDELETE(pAudioCap);
	}
	else
		PTRACE(eLevelError,"CAudModeH323::SetFramePP: unknown algorithm was found in CComModeH323 object.");
}

//////////////////////////////////////////////////////////////////////////
DWORD CAudModeH323::GetBitRate() const
{
	DWORD bitRate = 0;
	CComModeInfo comModeInfo = (CapEnum)m_type;

	if (IsModeOff())
	{
		PTRACE(eLevelInfoNormal, "CAudModeH323::GetBitRate: Mode is off!!");
		return bitRate;
	}

	if(comModeInfo.IsType(cmCapAudio))
	{
		bitRate = comModeInfo.GetBitRate(GetDataCap())/_K_;
	}
	else
		PTRACE(eLevelError,"CAudModeH323::GetBitRate: unknown algorithm was found in CComModeH323 object.");

	return bitRate;
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CAudModeH323::SetAudioAlg(CapEnum alg,cmCapDirection eDirection)
{
	CBaseAudioCap* pAudioCap = (CBaseAudioCap *)CBaseCap::AllocNewCap(alg, NULL);
	if (pAudioCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet &= pAudioCap->SetDefaults(eDirection);
		CCapSetInfo capInfo = alg;
		pAudioCap->SetMaxFramePerPacket(capInfo.GetMaxFramePerPacket());
        Create(pAudioCap);
        pAudioCap->FreeStruct();
		POBJDELETE(pAudioCap);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CAudModeH323::SetBitRate(DWORD bitRate)
{
	CBaseAudioCap* pAudioCap = (CBaseAudioCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
	if (pAudioCap) {
		pAudioCap->SetBitRate(bitRate);
		POBJDELETE(pAudioCap);
	}
}

//////////////////////////////////////////////////////////////////////////
size_t  CAudModeH323::SizeOf() const
{
	CBaseAudioCap	*pAudioCap	= (CBaseAudioCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

	size_t size = 0 ;

	if (pAudioCap)
	{
	    size =  pAudioCap->SizeOf();
	    PDELETE(pAudioCap);
	}

	return size;
}

//////////////////////////////////////////////////////////////////////////
WORD  CAudModeH323::GetBitRateFromAudioMode(BYTE audMode)
{
	WORD retVal = 0;
    switch ( audMode )
    {

        case G729_8k  :
        case G723_1_Command:
        {
            retVal = 8;
            break;
        }
        case G728  :
        case Au_Siren7_16k  :
        {
            retVal = 16;
            break;
        }

        // G722.1/24 support
        case Au_24k    :
        case Au_Siren7_24k  :
        case Au_Siren14_24k  :
        {  // G.722.1/24
            retVal = 24;
            break;
        }
        // G722.1/32 support
        case Au_32k    :
        case Au_Siren7_32k  :
        case Au_Siren14_32k  :
        {  // G.722.1/32
            retVal = 32;
            break;
        }

        case G722_m3  :
        case A_Law_48  :
        case U_Law_48  :
        case Au_Siren14_48k  :
        case Au_Siren14S_48k  :
        {  // g.711 only
            retVal = 48;
            break;
        }

        case G722_m2  :
        case A_Law_OF  :
        case U_Law_OF  :
        case Au_Siren14S_56k  :{  // g.711 only
            retVal = 56;
            break;
        }

        case G722_m1  :
        case A_Law_OU:
		case U_Law_OU:
		case Au_Siren14S_64k  :
		{  // voice call
            retVal = 64;
            break;
        }

        // Siren14S96 support
        case Au_Siren14S_96k  :
        case Au_Siren22S_96k  :
        case Au_SirenLPRS_96k  :
        case G719S_96k  :
        {  // Siren14S96
            retVal = 96;
            break;
        }
        // Siren14S96 support
        case Au_Siren22S_128k  :
        case Au_SirenLPRS_128k  :
        case G719S_128k  :
        {  // Siren22S96
            retVal = 128;
            break;
        }

        case 255  :  {    // auto
            retVal = 0;
            break;
        }

        default   :  {
            retVal = 0;
            break;
        }
    }
	return retVal;
}
////////////////////////////////////////////////////////////////////////////////////////
void  CAudModeH323::SetSacScm(cmCapDirection eDirection)
{
	CSacAudioCap* pSacCap  = (CSacAudioCap*) (CBaseCap::AllocNewCap(eSirenLPR_Scalable_48kCapCode, NULL));
	if (pSacCap)
	{
		EResult bRes = kSuccess;
		bRes &= pSacCap->SetDefaults(eDirection);
		if (bRes)
		{//Sac Audio only for softMSU


			WORD mixDepth = SFTMCU_AUDIO_MIX_DEPTH_DEFAULT; //Sac Audio only for softMSU
			pSacCap->SetMixDepth(mixDepth);


//			WORD maxRecvSsrc = 0;
//			pSacCap->SetMaxRecvSsrc(maxRecvSsrc);
//
//			WORD maxSendSsrc = AUDIO_MAX_SEND_STREAMS_DEFAULT;
//			pSacCap->SetMaxSendSsrc(maxSendSsrc);

			Create(pSacCap);
		}

		pSacCap->FreeStruct();
		POBJDELETE(pSacCap);
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////  Video Mode H323///////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CVidModeH323::CVidModeH323()
{
	InitAllVideoMediaModeMembers();
}
/////////////////////////////////////////////////////////////////////////////
CVidModeH323::CVidModeH323(BaseCapStruct *pCap,CapEnum type)
{
	InitAllVideoMediaModeMembers();
	CBaseVideoCap *pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(type, pCap);
	if (pVideoCap)
	{
	    Create(type, pVideoCap->SizeOf(),(const BYTE *)pCap);
	    PDELETE(pVideoCap);
	}
	else
	    FPTRACE(eLevelError,"CVidModeH323::CVidModeH323 - pVideoCap is NULL!!");
}
/////////////////////////////////////////////////////////////////////////////
void  CVidModeH323::InitAllVideoMediaModeMembers()
{
	InitAllMediaModeMembers();
	m_bIsAutoResolution	= YES;
	m_bIsAutoProtocol	= YES;
}
/////////////////////////////////////////////////////////////////////////////
void  CVidModeH323::Serialize(WORD format,CSegment& seg) const
{
	switch (format)
	{
		case SERIALEMBD:
			break;
		case NATIVE:
		{
			CMediaModeH323::Serialize(format, seg);
			seg << m_bIsAutoResolution << m_bIsAutoProtocol;
			break;
		}
		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CVidModeH323::DeSerialize(WORD format,CSegment& seg)
{
	switch (format)
	{
		case SERIALEMBD:
			break;
		case NATIVE :
			{
				CMediaModeH323::DeSerialize(format, seg);
				seg >> m_bIsAutoResolution >> m_bIsAutoProtocol;
				break;
			}
		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
CVidModeH323& CVidModeH323::operator=(const CVidModeH323& other)
{
	if(this == &other)
		return *this;

	(*(CMediaModeH323*)this) = (CMediaModeH323&)other;

	m_bIsAutoResolution		 = other.m_bIsAutoResolution;
	m_bIsAutoProtocol		 = other.m_bIsAutoProtocol;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
WORD CVidModeH323::operator==(const CMediaModeH323& other) const
{
	BYTE bRes = TRUE;

	// If one is off
	if (IsModeOff() != other.IsModeOff())
	{
		bRes = FALSE;
		//FPTRACE(eLevelInfoNormal,"CVidModeH323::operator==: one has video off!!");
	}

	// Both is on and type are the same. Should compare parameters
	else if (m_type == ((const CVidModeH323 &)other).m_type)
	{
		CBaseVideoCap* pFirstVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
		CBaseVideoCap* pSecondVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)((const CVidModeH323 &)other).m_type, ((const CVidModeH323 &)other).m_dataCap);

		if (pFirstVideoCap && pSecondVideoCap)
		{
			DWORD details = 0;
			BYTE bIsContainingRes;
			//CSmallString msg;

			bIsContainingRes = pSecondVideoCap->IsContaining(*pFirstVideoCap, kBitRate | kFormat | kFrameRate | kAnnexes | kH264Additional_MBPS | kH264Additional_FS | kH264Profile | kMaxFR | kH264Mode | kPacketizationMode, &details);
			if (bIsContainingRes == FALSE)
			{
				cmCapDataType eType = pSecondVideoCap->GetType();
				//DumpDetailsToStream(eType, details, msg);
				//FPTRACE2(eLevelInfoNormal,"CVidModeH323::operator==(first): ",msg.GetString());
				bRes = FALSE;
			}
			else
			{
				bIsContainingRes = pFirstVideoCap->IsContaining(*pSecondVideoCap, kBitRate | kFormat | kFrameRate | kAnnexes | kH264Additional_MBPS | kH264Additional_FS | kH264Profile | kMaxFR | kH264Mode | kPacketizationMode, &details);
				if (bIsContainingRes == FALSE)
				{
					cmCapDataType eType = pFirstVideoCap->GetType();
					//DumpDetailsToStream(eType, details, msg);
					//FPTRACE2(eLevelInfoNormal,"CVidModeH323::operator==(second): ",msg.GetString());
					bRes = FALSE;
				}
				else //Both true
				{
					if (m_type == eH263CapCode)
					{ //check annex I NS (can't be part of the IsContaining function!!)
						CH263VideoCap* pFirstH263Cap = (CH263VideoCap*)pFirstVideoCap;
						CH263VideoCap* pSecondH263Cap = (CH263VideoCap*)pSecondVideoCap;
						BYTE bFirstHasNsAnnexI = pFirstH263Cap->IsAnnex(typeAnnexI_NS);
						BYTE bSecondHasNsAnnexI = pSecondH263Cap->IsAnnex(typeAnnexI_NS);
						if (bFirstHasNsAnnexI != bSecondHasNsAnnexI)
						{
							//FPTRACE(eLevelInfoNormal,"CVidModeH323::operator==: Different in Annex I NS");
							bRes = FALSE;
						}
					}
				}
			}
		}

		PDELETE(pSecondVideoCap);
		PDELETE(pFirstVideoCap);
	}

	else
	{
		//FPTRACE(eLevelInfoNormal,"CVidModeH323::operator==: Video types are different!!");
		bRes = FALSE;
	}

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
void CVidModeH323::SetFormatMpi(EFormat eFormat, int mpi)
{
	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::SetFormatMpi: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapVideo))
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pVideoCap)
			{
				WORD resOfSet = kSuccess;
				resOfSet  &= pVideoCap->SetFormatMpi(eFormat,mpi);

				if (resOfSet == kFailure)
					PTRACE(eLevelInfoNormal,"CVidModeH323::SetFormatMpi: Set failed!!");
			}

			PDELETE(pVideoCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::SetFormatMpi: Unknown video cap type");
	}
}

/////////////////////////////////////////////////////////////////////////////
APIS8  CVidModeH323::GetFormatMpi(EFormat eFormat) const
{
	APIS8 res = -1;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetFormatMpi: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapVideo))
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pVideoCap)
				res  = pVideoCap->GetFormatMpi(eFormat);

			PDELETE(pVideoCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::GetFormatMpi: Unknown video cap type");
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////
APIS16  CVidModeH323::GetFrameRate(EFormat eFormat) const
{
	APIS16 res = -1;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetFrameRate: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapVideo))
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pVideoCap)
				res  = pVideoCap->GetFrameRate(eFormat);

			PDELETE(pVideoCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::GetFrameRate: Unknown video cap type");
	}

	return res;
}
/////////////////////////////////////////////////////////////////////////////
DWORD  CVidModeH323::GetFrameRateForRTV() const
{
	DWORD res = -1;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetFrameRateForRTV: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapVideo))
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pVideoCap)
				res  = ((CRtvVideoCap*)pVideoCap)->GetFrameRateForRTV();

			PDELETE(pVideoCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::GetFrameRateForRTV: Unknown video cap type");
	}

	return res;
}
/////////////////////////////////////////////////////////////////////////////
EFormat CVidModeH323::GetFormat() const
{
	EFormat res = kUnknownFormat;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetFormat: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapVideo))
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pVideoCap)
				res  = pVideoCap->GetFormat();

			PDELETE(pVideoCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::GetFormat: Unknown video cap type");
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////
void CVidModeH323::SetBitRate(int newBitRate)
{
	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::SetBitRate: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapVideo))
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pVideoCap)
			{
				EResult eResOfSet  = pVideoCap->SetBitRate(newBitRate);

				if (eResOfSet == kFailure)
					PTRACE(eLevelInfoNormal,"CVidModeH323::SetBitRate: Set failed!!");
			}

			PDELETE(pVideoCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::SetBitRate: Unknown video cap type");
	}
}

/////////////////////////////////////////////////////////////////////////////
void CVidModeH323::SetMaxFR(int newMaxFR)
{
	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::SetMaxFR: Mode is off");
	else
	{
		if (m_type == eH264CapCode)
		{
			CH264VideoCap* pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode,m_dataCap);

			if (pVideoCap)
			{
			    EResult eResOfSet = pVideoCap->SetMaxFR(newMaxFR);
			    if (eResOfSet == kFailure)
			        PTRACE(eLevelInfoNormal,"CVidModeH323::SetMaxFR: Set failed!!");
			    PDELETE(pVideoCap);
			}
			else
			    PTRACE(eLevelInfoNormal,"CVidModeH323::SetMaxFR - pVideoCap is NULL");
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::SetMaxFR: not 264 video cap type");
	}
}

/////////////////////////////////////////////////////////////////////////////
void CVidModeH323::SetH264mode(int newH264mode)
{
	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::SetH264mode: Mode is off");
	else
	{

		if (m_type == eH264CapCode)
		{

			CH264VideoCap* pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode,m_dataCap);

			if (pVideoCap)
			{
			    EResult eResOfSet  = pVideoCap->SetH264mode(newH264mode);
			    if (eResOfSet == kFailure)
			        PTRACE(eLevelInfoNormal,"CVidModeH323::SetH264mode: Set failed!!");
			    PDELETE(pVideoCap);
			}
			else
			    PTRACE(eLevelInfoNormal,"CVidModeH323::SetH264mode - pVideoCap is NULL");
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::SetH264mode: not 264 video cap type");
	}
}
/////////////////////////////////////////////////////////////////////////////
DWORD CVidModeH323::GetBitRate(void)const
{
	DWORD bitRate = 0;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetBitRate: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapVideo))
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pVideoCap)
				bitRate	= pVideoCap->GetBitRate();

			PDELETE(pVideoCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::GetBitRate: Unknown video cap type");
	}
	return bitRate;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CVidModeH323::GetMaxFR(void)const
{
	DWORD getMaxFR = 0;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetMaxFR: Mode is off");
	else
	{
		if (m_type == eH264CapCode)
		{
			CH264VideoCap* pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode,m_dataCap);
			if (pVideoCap)
			{
			    getMaxFR = pVideoCap->GetMaxFR();
			    PDELETE(pVideoCap);
			}
			else
			    PTRACE(eLevelInfoNormal,"CVidModeH323::GetMaxFR - pVideoCap is NULL");
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::GetMaxFR: not 264 video cap type");
	}
	return getMaxFR;
}

/////////////////////////////////////////////////////////////////////////////
APIU8 CVidModeH323::GetPacketizationMode(void)const
{
	APIU8 packetizationMode = 0;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetPacketizationMode: Mode is off");
	else
	{
		if (m_type == eH264CapCode)
		{
			CH264VideoCap* pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode,m_dataCap);
			if (pVideoCap)
			{
				packetizationMode = pVideoCap->GetPacketizationMode();
			    PDELETE(pVideoCap);
			}
			else
			    PTRACE(eLevelInfoNormal,"CVidModeH323::GetPacketizationMode - pVideoCap is NULL");
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::GetPacketizationMode: not 264 video cap type");
	}
	return packetizationMode;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CVidModeH323::GetH264mode(void)const
{
	DWORD getH264mode = H264_standard;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetH264mode: Mode is off");
	else
	{
		if (m_type == eH264CapCode)
		{
		    CH264VideoCap* pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode,m_dataCap);
		    if (pVideoCap)
		    {
		        getH264mode = pVideoCap->GetH264mode();
		        PDELETE(pVideoCap);
		    }
		    else
		        PTRACE(eLevelInfoNormal,"CVidModeH323::GetH264mode - pVideoCap is NULL");
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::GetH264mode: not 264 video cap type");
	}
	return getH264mode;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CVidModeH323::GetRtcpFeedbackMask(void)const
{
	DWORD geRtcpFeedbackMask = 0;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetRtcpFeedbackMask: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if (comModeInfo.IsType(cmCapVideo))
		{
		    CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
		    if (pVideoCap)
		    {
		        geRtcpFeedbackMask = pVideoCap->GetRtcpFeedbackMask();
		        PDELETE(pVideoCap);
		    }
		    else
		        PTRACE(eLevelInfoNormal,"CVidModeH323::GetRtcpFeedbackMask - pVideoCap is NULL");
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::GetRtcpFeedbackMask: Unknown video cap type");
	}
	return geRtcpFeedbackMask;
}

/////////////////////////////////////////////////////////////////////////////
void CVidModeH323::SetRtcpFeedbackMask(DWORD rtcpFbMask)
{
	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CVidModeH323::GetRtcpFeedbackMask: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if (comModeInfo.IsType(cmCapVideo))
		{
		    CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
		    if (pVideoCap)
		    {
		        EResult eResOfSet = pVideoCap->SetRtcpFeedbackMask(rtcpFbMask);
			 if (eResOfSet == kFailure)
				PTRACE(eLevelInfoNormal,"CVidModeH323::SetRtcpFeedbackMask: Set failed!!");
			 PDELETE(pVideoCap);
		    }
		    else
		        PTRACE(eLevelInfoNormal,"CVidModeH323::SetRtcpFeedbackMask - pVideoCap is NULL");
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::SetRtcpFeedbackMask: Unknown video cap type");
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CVidModeH323::IsSupportErrorCompensation() const
{
	BYTE bRes = FALSE;
	if (m_type == eH263CapCode)
	{
		CH263VideoCap* pVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,m_dataCap);
		if(pVideoCap)
			bRes = pVideoCap->IsErrorCompensation();
		PDELETE(pVideoCap);
	}
	return bRes;
}

////////////////////////////////////////////////////////////////////////////
/*
BYTE CVidModeH323::IsVidParamSupportedInCP(int maxPossibleFormatsMpi[kSIF + 1]) const
{
	BYTE bRes = TRUE;

	if (m_type == eH261CapCode)
	{
		CH261VideoCap* pVideoCap = (CH261VideoCap *)CBaseCap::AllocNewCap(eH261CapCode,m_dataCap);
		if(pVideoCap)
		{
			for (EFormat format = kQCif; format <= kCif; format++)
			{
				if (pVideoCap->IsFormat(format))
				{
					APIU8 currentMpi = pVideoCap->GetFormatMpi(format);
					if (currentMpi < maxPossibleFormatsMpi[format]) //mpi is 1,2,3,4...
					{
						CSmallString temptrace;
						temptrace << "CVidModeH323::IsVidParamSupportedInCP:261 format mpi is exceeded. ";
						temptrace <<"currentMpi = "<<currentMpi<<",maxPossibleFormatsMpi = "<<maxPossibleFormatsMpi[format];
						temptrace <<", format = "<<format;
						PTRACE(eLevelInfoNormal,temptrace.GetString());
						bRes = FALSE;
					}
				}
			}
			POBJDELETE(pVideoCap);
		}
	}
	else if (m_type == eH263CapCode)
	{
		CH263VideoCap* pVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,m_dataCap);
		if(pVideoCap)
		{
			if (pVideoCap->IsFormat(k16Cif))
			{
				PTRACE(eLevelInfoNormal,"CVidModeH323::IsVidParamSupportedInCP: 16Cif was opened.");
				bRes = FALSE;
			}
			else if (pVideoCap->IsAnnexes())
			{
				PTRACE(eLevelInfoNormal,"CVidModeH323::IsVidParamSupportedInCP: annexes were opened.");
				bRes = FALSE;
			}
			else
			{
				for (EFormat format = kQCif; format < kSIF; format++)
				{
					if (pVideoCap->IsFormat(format))
					{
						APIU8 currentMpi = pVideoCap->GetFormatMpi(format);
						if (currentMpi < maxPossibleFormatsMpi[format]) //mpi is 1,2,3,4...
						{
							CSmallString temptrace;
							temptrace << "CVidModeH323::IsVidParamSupportedInCP:263 format mpi is exceeded. ";
							temptrace <<"currentMpi = "<<currentMpi<<",maxPossibleFormatsMpi = "<<maxPossibleFormatsMpi[format];
							temptrace <<", format = "<<format;
							PTRACE(eLevelInfoNormal,temptrace.GetString());
							bRes = FALSE;
						}
					}
				}
			}

			POBJDELETE(pVideoCap);
		}
	}

	return bRes;
}
*/
////////////////////////////////////////////////////////////////////////////
void CVidModeH323::UpdateParams(CH263VideoCap *pFirstVideoCap)
{
	SetType(pFirstVideoCap->GetCapCode());
	AllocNewDataCap(pFirstVideoCap->SizeOf());
	SetDataCap((BYTE *)pFirstVideoCap->GetStruct());
}

////////////////////////////////////////////////////////////////////////////
size_t  CVidModeH323::SizeOf() const
{
    CBaseVideoCap	*pVideoCap	= (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
    size_t size =  0;

    if (pVideoCap)
    {
        size =  pVideoCap->SizeOf();
        PDELETE(pVideoCap);
    }

    return size;
}

////////////////////////////////////////////////////////////////////////////
void CVidModeH323::AddLowerResolutionsIfNeeded()
{
	if (IsModeOff())
	{
		PTRACE(eLevelInfoNormal,"CVidModeH323::AddLowerResolutionsIfNeeded: Mode is off");
		return;
	}

	CComModeInfo comModeInfo = (CapEnum)m_type;
	if(comModeInfo.IsType(cmCapVideo))
	{
		CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
		if (pVideoCap)
			pVideoCap->AddLowerResolutionsIfNeeded();
		POBJDELETE(pVideoCap);
	}
	else
		PTRACE(eLevelInfoNormal,"CVidModeH323::AddLowerResolutionsIfNeeded: Unknown video cap type");
}

////////////////////////////////////////////////////////////////////////////////////////
BYTE  CVidModeH323::SetHDContent(DWORD contentRate,EHDResolution eHDRes,cmCapDirection eDirection,BYTE bContentAsVideo,BYTE HDMpi,BOOL bHighProfile)
{
	EResult eResOfSet = kFailure;

	CCapSetInfo capVideoInfo = eH264CapCode;
	CCapSetInfo capH239Info  = eH239ControlCapCode;
////////////// Romem
   /* if(eHDRes == eHD1080Res && bContentAsVideo)
    {
    	PTRACE(eLevelInfoNormal,"CVidModeH323::SetHDContent: error-can not use content with Hd1080 when Legacy feature is enabled");
    	eHDRes = eHD720Res;
    }*/
	// If h323 mode type is video alg and the video mode is not off
	if (capVideoInfo.IsSupporedCap() && capH239Info.IsSupporedCap())
	{
		CBaseVideoCap * pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(capVideoInfo,NULL);

		if (pVideoCap)
		{
			eResOfSet = ((CH264VideoCap*)pVideoCap)->SetHDContent(kRolePresentation,eHDRes,eDirection,HDMpi,bHighProfile);


		//	eResOfSet = pVideoCap->SetContent(kRoleContent);
			DWORD maxContRate     = contentRate;//the conference pass with bit per second
			if(eResOfSet)
			{
				eResOfSet = pVideoCap->SetBitRate(maxContRate);
				PDELETEA(m_dataCap);//PDELETE(m_dataCap); // deletes old struct   PDELETEA
				m_type       = capVideoInfo.GetIpCapCode();
				m_dataLength = pVideoCap->SizeOf();
				m_dataCap    = (BYTE *)pVideoCap->GetStruct();
			}
			else
				pVideoCap->FreeStruct();
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::SetContent : Couldn't set struct!!");

		POBJDELETE(pVideoCap);
	}

	else if ((capVideoInfo.IsType(cmCapVideo)) && capVideoInfo.IsSupporedCap())
	{
		PTRACE(eLevelInfoNormal,"CVidModeH323::SetContent : video is off");
		SetModeOff();
	}
	return eResOfSet;
}

////////////////////////////////////////////////////////////////////////////////////////
BYTE  CVidModeH323::SetTIPContent(DWORD contentRate, cmCapDirection eDirection, BYTE set264ModeAsTipContent)
{
	EResult eResOfSet = kFailure;

	CCapSetInfo capVideoInfo = eH264CapCode;
//	CCapSetInfo capH239Info  = eH239ControlCapCode;
//
//    if(eHDRes == eHD1080Res && bContentAsVideo)
//    {
//    	PTRACE(eLevelInfoNormal,"CVidModeH323::SetHDContent: error-can not use content with Hd1080 when Legacy feature is enabled");
//    	eHDRes = eHD720Res;
//    }
//	// If h323 mode type is video alg and the video mode is not off
//	if (capVideoInfo.IsSupporedCap() && capH239Info.IsSupporedCap())
//	{
		CBaseVideoCap * pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(capVideoInfo,NULL);

		if (pVideoCap)
		{
			eResOfSet = ((CH264VideoCap*)pVideoCap)->SetTIPContent(kRolePresentation, eDirection, set264ModeAsTipContent);

			if(eResOfSet)
			{
				eResOfSet = pVideoCap->SetBitRate(contentRate);
				PDELETEA(m_dataCap);//PDELETE(m_dataCap); // deletes old struct   PDELETEA
				m_type       = capVideoInfo.GetIpCapCode();
				m_dataLength = pVideoCap->SizeOf();
				m_dataCap    = (BYTE *)pVideoCap->GetStruct();
			}
			else
			{
				pVideoCap->FreeStruct();
			}
			POBJDELETE(pVideoCap);
		}
//		else
//			PTRACE(eLevelInfoNormal,"CVidModeH323::SetContent : Couldn't set struct!!");
//
//		POBJDELETE(pVideoCap);
//	}
//	else if ((capVideoInfo.IsType(cmCapVideo)) && capVideoInfo.IsSupporedCap())
//	{
//		PTRACE(eLevelInfoNormal,"CVidModeH323::SetContent : video is off");
//		SetModeOff();
//	}
	return eResOfSet;
}
////////////////////////////////////////////////////////////////////////////////////////
BYTE  CVidModeH323::SetContent(DWORD contentRate, CapEnum Protocol, cmCapDirection eDirection, BOOL isHD1080, BYTE HDContentMpi, BYTE partyMediaType, BOOL isHighProfile)
{
	EResult eResOfSet = kFailure;
	PTRACE(eLevelInfoNormal,"CVidModeH323::SetContent");
	CCapSetInfo capVideoInfo = Protocol;
	CCapSetInfo capH239Info  = eH239ControlCapCode;
	CCapSetInfo capEPCInfo = ePeopleContentCapCode;

	// If h323 mode type is video alg and the video mode is not off
	if (capVideoInfo.IsSupporedCap() && capH239Info.IsSupporedCap())
	{
		CBaseVideoCap * pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(capVideoInfo,NULL);

		if (pVideoCap)
		{
			eResOfSet = pVideoCap->SetContent(kRolePresentation,eDirection,isHD1080,HDContentMpi,isHighProfile);

			if (partyMediaType == eSvcPartyType) {
				APIS32 rtcpFeedbackMask = pVideoCap->GetRtcpFeedbackMask();
				if (rtcpFeedbackMask & RTCP_MASK_IS_NOT_STANDARD_ENCODE)
					pVideoCap->SetRtcpFeedbackMask(rtcpFeedbackMask ^ RTCP_MASK_IS_NOT_STANDARD_ENCODE);
			}

		//	eResOfSet = pVideoCap->SetContent(kRoleContent);
			DWORD maxContRate     = contentRate;//the conference pass with bit per second
			if(eResOfSet)
			{
				eResOfSet = pVideoCap->SetBitRate(maxContRate);
				PDELETEA(m_dataCap);//PDELETE(m_dataCap); // deletes old struct   PDELETEA
				m_type       = capVideoInfo.GetIpCapCode();
				m_dataLength = pVideoCap->SizeOf();
				m_dataCap    = (BYTE *)pVideoCap->GetStruct();
			}
			else
				pVideoCap->FreeStruct();
		}
		else
			PTRACE(eLevelInfoNormal,"CVidModeH323::SetContent : Couldn't set struct!!");

		POBJDELETE(pVideoCap);
	}

	else if ((capVideoInfo.IsType(cmCapVideo)) && capVideoInfo.IsSupporedCap())
	{
		PTRACE(eLevelInfoNormal,"CVidModeH323::SetContent : video is off");
		SetModeOff();
	}
	return eResOfSet;
}

////////////////////////////////////////////////////////////////////////////////////////
void  CVidModeH323::SetH264Scm(APIU16 profile, APIU8 level, APIS32 mbps, APIS32 fs, APIS32 dpb, APIS32 brAndCpb, APIS32 sar, APIS32 staticMB, cmCapDirection eDirection, APIU8 packatizationMode)
{
	DWORD backUpRate         = GetBitRate();
	APIS32 backUpMaxFR       = GetMaxFR();
	APIS32 backUpH264mode    = GetH264mode();
	APIU8  backUpPM 		 = GetPacketizationMode();

	CH264VideoCap* pH264Cap  = (CH264VideoCap*) (CBaseCap::AllocNewCap(eH264CapCode, NULL));
	if (pH264Cap)
	{
		EResult bRes = kSuccess;
		bRes &= pH264Cap->SetDefaults(eDirection);
		if (bRes)
		{
			pH264Cap->SetLevelAndAdditionals(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
			pH264Cap->RemoveDefaultAdditionals();
			pH264Cap->SetBitRate(backUpRate);
			pH264Cap->SetMaxFR(backUpMaxFR);
			pH264Cap->SetH264mode(backUpH264mode);
			if(packatizationMode == H264_PACKETIZATION_MODE_UNSET)
			{
				pH264Cap->SetPacketizationMode(backUpPM);
			}
			if(packatizationMode == H264_SINGLE_NAL_PACKETIZATION_MODE)
			{
				pH264Cap->SetPacketizationMode(H264_SINGLE_NAL_PACKETIZATION_MODE);
			}
			Create(pH264Cap);
		}

		pH264Cap->FreeStruct();
		POBJDELETE(pH264Cap);
	}
}
////////////////////////////////////////////////////////////////////////////////////////
void  CVidModeH323::SetVP8Scm(VP8VideoModeDetails VP8VideoDetails,cmCapDirection eDirection,APIS32 BitRate)
{//N.A. DEBUG VP8

	CVP8VideoCap* pVP8Cap  = (CVP8VideoCap*) (CBaseCap::AllocNewCap(eVP8CapCode, NULL));
	PTRACE2INT(eLevelInfoNormal,"CVidModeH323::SetVP8Scm eDirection = ",(DWORD)eDirection);

	if (pVP8Cap)
	{
		EResult bRes = kSuccess;
		bRes &= pVP8Cap->SetDefaults(eDirection);
		if (bRes)
		{
			pVP8Cap->SetMaxFR(VP8VideoDetails.maxFrameRate);
			pVP8Cap->SetMaxFS(VP8VideoDetails.maxFS);
			pVP8Cap->SetBitRate(VP8VideoDetails.maxBitRate);

			PTRACE2INT(eLevelInfoNormal,"CVidModeH323::SetVP8Scm VP8VideoDetails.maxFrameRate = ",VP8VideoDetails.maxFrameRate);
			PTRACE2INT(eLevelInfoNormal,"CVidModeH323::SetVP8Scm VP8VideoDetails.maxFS = ",VP8VideoDetails.maxFS);


			//N.A. DEBUG - Add the rest of the sets

			Create(pVP8Cap);
		}

		pVP8Cap->FreeStruct();
		POBJDELETE(pVP8Cap);
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////
void  CVidModeH323::SetMsSvcScm(MsSvcVideoModeDetails MsSvcVidModeDetails,cmCapDirection eDirection,APIS32 BitRate)
{
	CMsSvcVideoCap* pMsSvcCap  = (CMsSvcVideoCap*) (CBaseCap::AllocNewCap(eMsSvcCapCode, NULL));
	PTRACE2INT(eLevelInfoNormal,"CVidModeH323::SetMsSvcScm ",(DWORD)eDirection);
	if (pMsSvcCap)
	{
		//PTRACE2INT(eLevelInfoNormal,"CVidModeH323::SetMsSvcScm - alloc ",(DWORD)eDirection);
		EResult bRes = kSuccess;
		bRes &= pMsSvcCap->SetDefaults(eDirection);
		if(bRes)
		{
			//PTRACE2INT(eLevelInfoNormal,"CVidModeH323::SetMsSvcScm - in setting ",(DWORD)eDirection);
			pMsSvcCap->SetAspectRatio(MsSvcVidModeDetails.aspectRatio);
			pMsSvcCap->SetHeight(MsSvcVidModeDetails.maxHeight);
			pMsSvcCap->SetWidth(MsSvcVidModeDetails.maxWidth);
			pMsSvcCap->SetBitRate(BitRate);
			pMsSvcCap->SetMaxBitRate(BitRate);
			pMsSvcCap->SetMaxFrameRate(MsSvcVidModeDetails.maxFrameRate);
			pMsSvcCap->SetMinBitRate(MsSvcVidModeDetails.minBitRate/100);
			pMsSvcCap->SetPacketizationMode(H264_NON_INTERLEAVED_PACKETIZATION_MODE);//?
			pMsSvcCap->SetMaxPixelNum(MsSvcVidModeDetails.maxNumOfPixels);
			COstrStream msg;
			pMsSvcCap->Dump(msg);
			PTRACE2(eLevelInfoNormal,"CVidModeH323::SetMsSvcScm, mssvccap : ", msg.str().c_str());
			//noa -what about min bit rate

			Create(pMsSvcCap);
		}
		pMsSvcCap->FreeStruct();
		POBJDELETE(pMsSvcCap);
	}

}
////////////////////////////////////////////////////////////////////////////////////////
void  CVidModeH323::SetSvcScm(APIU16 profile, APIU8 level, APIS32 mbps, APIS32 fs, APIS32 dpb, APIS32 brAndCpb, APIS32 sar, APIS32 staticMB, cmCapDirection eDirection)
{
	PTRACE(eLevelInfoNormal, " CVidModeH323::SetScvScm");
	DWORD backUpRate         = GetBitRate();
	APIS32 backUpMaxFR       = GetMaxFR();
	APIS32 backUpH264mode    = GetH264mode();
	CSvcVideoCap* pSvcCap  = (CSvcVideoCap*) (CBaseCap::AllocNewCap(eSvcCapCode, NULL));
	if (pSvcCap)
	{
		EResult bRes = kSuccess;
		bRes &= pSvcCap->SetDefaults(eDirection);
		if (bRes)
		{
			pSvcCap->SetLevelAndAdditionals(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
			pSvcCap->RemoveDefaultAdditionals();
			pSvcCap->SetBitRate(backUpRate);
			pSvcCap->SetMaxFR(backUpMaxFR);
			pSvcCap->SetH264mode(backUpH264mode);
		//	pSvcCap->SetPacketizationMode(1);

			APIS32 rtcpFeedbackMask = pSvcCap->GetRtcpFeedbackMask();
			if (rtcpFeedbackMask & RTCP_MASK_IS_NOT_STANDARD_ENCODE)
				pSvcCap->SetRtcpFeedbackMask(rtcpFeedbackMask ^ RTCP_MASK_IS_NOT_STANDARD_ENCODE);

			Create(pSvcCap);
		}

		pSvcCap->FreeStruct();
		POBJDELETE(pSvcCap);
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::GetH264Scm(APIU16& profile, APIU8& level, long& mbps, long& fs, long& dpb, long& brAndCpb, long& sar, long& staticMB) const
{
	if ((CapEnum)GetType() != eH264CapCode)
		return;

	CH264VideoCap* pH264Cap  = (CH264VideoCap*) (CBaseCap::AllocNewCap(eH264CapCode, m_dataCap));

	if (pH264Cap)
	{
	    profile = pH264Cap->GetProfile();
	    level = pH264Cap->GetLevel();
	    mbps  = pH264Cap->GetMbps();
	    fs    = pH264Cap->GetFs();
	    dpb   = pH264Cap->GetDpb();
	    brAndCpb = pH264Cap->GetBrAndCpb();
	    sar	  = pH264Cap->GetSampleAspectRatio();
	    staticMB = pH264Cap->GetStaticMB();
	    POBJDELETE(pH264Cap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::GetH264Scm - pH264Cap is NULL!");
}
/////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::GetVp8Scm(APIS32& fs, APIS32& maxFrameRate) const
{
	if ((CapEnum)GetType() != eVP8CapCode)
		return;

	CVP8VideoCap* pVp8Cap  = (CVP8VideoCap*) (CBaseCap::AllocNewCap(eVP8CapCode, m_dataCap));

	if (pVp8Cap)
	{
	    fs    		 = pVp8Cap->GetMaxFS();
	    maxFrameRate = pVp8Cap->GetMaxFR();
	    PTRACE2INT(eLevelError,"CVidModeH323::GetVp8Scm - fs", fs);
	    PTRACE2INT(eLevelError,"CVidModeH323::GetVp8Scm - maxFrameRate", maxFrameRate);

	    POBJDELETE(pVp8Cap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::GetVp8Scm - pVp8Cap is NULL!");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::GetMSSvcSpecificParams(APIS32& Width, APIS32& Height, APIS32& aspectRatio, APIS32& maxFrameRate) const
{
	if ((CapEnum)GetType() != eMsSvcCapCode)
		return;

	CMsSvcVideoCap* pMsSvcCap  = (CMsSvcVideoCap*) (CBaseCap::AllocNewCap(eMsSvcCapCode, m_dataCap));

	if (pMsSvcCap)
	{
		Width = pMsSvcCap->GetWidth();
		Height = pMsSvcCap->GetHeight();
		PTRACE2INT(eLevelError,"CVidModeH323::GetMSSvcSpecificParams - Height ",Height);
		aspectRatio = pMsSvcCap->GetAspectRatio();
		maxFrameRate = pMsSvcCap->GetMaxFrameRate();

		POBJDELETE(pMsSvcCap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::GetMSSvcSpecificParams - pMsSvcCap is NULL!");
}
/////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::GetRtvScm(long& mbps, long& fs) const
{
	if ((CapEnum)GetType() != eRtvCapCode)
		return;

	CRtvVideoCap* pRtvCap  = (CRtvVideoCap*) (CBaseCap::AllocNewCap(eRtvCapCode, m_dataCap));

	if (pRtvCap)
	{
	    pRtvCap->GetMbpsAndFsAsDevision(mbps,fs);
	    POBJDELETE(pRtvCap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::GetRtvScm - pRtvCap is NULL!");

}

/////////////////////////////////////////////////////////////////////////////////////
BYTE CVidModeH323::IsTIPContentEnableInH264Scm() const
{
	BOOL IsTIPContentEnable = FALSE;

	if ((CapEnum)GetType() != eH264CapCode)
		return IsTIPContentEnable;

	CH264VideoCap* pH264Cap  = (CH264VideoCap*) (CBaseCap::AllocNewCap(eH264CapCode, m_dataCap));

	if (!pH264Cap)
	{
	    PTRACE(eLevelInfoNormal,"CVidModeH323::IsTIPContentEnableInH264Scm - pH264Cap is NULL");
	    return IsTIPContentEnable;
	}

	if (pH264Cap->GetH264mode() == H264_tipContent)
	{
	    IsTIPContentEnable = TRUE;
	    PTRACE(eLevelInfoNormal,"CVidModeH323::IsTIPContentEnableInH264Scm TIPContentEnable");
	}

	POBJDELETE(pH264Cap);

	return IsTIPContentEnable;
}

/////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::SetH264Profile(APIU16 profile)
{
	if ((CapEnum)GetType() != eH264CapCode)
		return;

	CH264VideoCap* pH264Cap  = (CH264VideoCap*) (CBaseCap::AllocNewCap(eH264CapCode, m_dataCap));
	if (pH264Cap)
	{
	    pH264Cap->SetProfile(profile);
	    POBJDELETE(pH264Cap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::SetH264Profile - pH264Cap is NULL");
}
/////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::GetH264Profile(APIU16& profile) const
{
	if ((CapEnum)GetType() != eH264CapCode)
		return;

	CH264VideoCap* pH264Cap  = (CH264VideoCap*) (CBaseCap::AllocNewCap(eH264CapCode, m_dataCap));
	if (pH264Cap)
	{
	    profile = pH264Cap->GetProfile();
	    POBJDELETE(pH264Cap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::GetH264Profile - pH264Cap is NULL");
}
/////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::GetH264PacketizationMode(APIU8& packetizationMode) const
{
	if ((CapEnum)GetType() != eH264CapCode)
		return;

	CH264VideoCap* pH264Cap  = (CH264VideoCap*) (CBaseCap::AllocNewCap(eH264CapCode, m_dataCap));
	if (pH264Cap)
	{
		packetizationMode = pH264Cap->GetPacketizationMode();
	    POBJDELETE(pH264Cap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::GetH264PacketizationMode - pH264Cap is NULL");
}
//////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CVidModeH323::GetVideoPartyTypeMBPSandFS(DWORD staticMB,BYTE IsRsrcByFs) const
{
	eVideoPartyType videoPartyType = eVideo_party_type_none;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bSplitTIPVideoResources = FALSE;
	pSysConfig->GetBOOLDataByKey("TIP_SPLIT_VIDEO_RESOURCES", bSplitTIPVideoResources);

    CMedString str;

//	PTRACE2INT(eLevelError,"CVidModeH323::GetVideoPartyTypeMBPSandFS - bSplitTIPVideoResources",bSplitTIPVideoResources);
	if((CapEnum)GetType() == eRtvCapCode)
	{
        str << " == RTV CapCode\n";
		CRtvVideoCap* pRtvCap  = (CRtvVideoCap*) (CBaseCap::AllocNewCap(eRtvCapCode, m_dataCap));
		if (pRtvCap)
		    videoPartyType = pRtvCap->GetVideoPartyTypeMBPSandFS(staticMB);
		else
		{
            str << " == pRtvCap is NULL - set eVideo_party_type_none \n";
		}

		POBJDELETE(pRtvCap);
	}
	else if((CapEnum)GetType() == eH264CapCode)
	{
	    CH264VideoCap* pH264Cap  = (CH264VideoCap*) (CBaseCap::AllocNewCap(eH264CapCode, m_dataCap));
	    if (pH264Cap)
	    {
	        videoPartyType = pH264Cap->GetVideoPartyTypeMBPSandFS(staticMB,IsRsrcByFs);
	        APIU16 profile = 0;
	        GetH264Profile(profile);
            str << " == main returning hd1080 resource profile = " << profile << "\n == resource videoPartyType = " << videoPartyType << "\n";

	        if(bSplitTIPVideoResources && profile == H264_Profile_Main && videoPartyType > eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
	        {
                str << " == main returning hd1080 resource\n";
	            videoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
	        }
	        POBJDELETE(pH264Cap);
	    }
	    else
            str << " == pH264Cap is NULL\n";
	}
	else if((CapEnum)GetType() == eMsSvcCapCode)
	{
		 str << " == CMsSvcVideoCap CapCode\n";
		 CMsSvcVideoCap* pMsSvcCap  = (CMsSvcVideoCap*) (CBaseCap::AllocNewCap(eMsSvcCapCode, m_dataCap));
		if (pMsSvcCap)
			videoPartyType = pMsSvcCap->GetVideoPartyTypeMBPSandFS(staticMB);
		else
		{
			str << " == pMsSvcCap is NULL - set eVideo_party_type_none \n";
		}

		POBJDELETE(pMsSvcCap);
	}
	else if((CapEnum)GetType() == eVP8CapCode)
	{
		 str << " == eVP8CapCode CapCode\n";
		 CVP8VideoCap* pvp8Cap  = (CVP8VideoCap*) (CBaseCap::AllocNewCap(eVP8CapCode, m_dataCap));
		if (pvp8Cap)
			videoPartyType = pvp8Cap->GetVideoPartyTypeFRandFS(staticMB);
		else
		{
			str << " == eVP8CapCode is NULL - set eVideo_party_type_none \n";
		}

		POBJDELETE(pvp8Cap);
	}
	else if((CapEnum)GetType() == eSvcCapCode)
	{
		 str << " == eSvcCapCode CapCode\n";
		 CSvcVideoCap* pSvcCap  = (CSvcVideoCap*) (CBaseCap::AllocNewCap(eSvcCapCode, m_dataCap));
		if (pSvcCap)
			videoPartyType = pSvcCap->GetVideoPartyTypeMBPSandFS(staticMB);
		else
		{
			str << " == pSvcCap is NULL - set eVideo_party_type_none \n";
		}

		POBJDELETE(pSvcCap);
	}
	else
	{
        str << " == not H264 SVC or RTV\n";
	}
    str << " == videoPartyType =" << videoPartyType;
    PTRACE2(eLevelError, "CVidModeH323::GetVideoPartyTypeMBPSandFS", str.GetString());

	return videoPartyType;
}

/////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::SetSampleAspectRatio(APIS32 sar)
{
	if ((CapEnum)GetType() != eH264CapCode)
		return;

	CH264VideoCap* pH264Cap  = (CH264VideoCap*) (CBaseCap::AllocNewCap(eH264CapCode, m_dataCap));
	if (pH264Cap)
	{
	    pH264Cap->SetSampleAspectRatio(sar);
	    POBJDELETE(pH264Cap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::SetSampleAspectRatio - pH264Cap is NULL!!");
}

/////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::SetStaticMB(APIS32 staticMB)
{
	if ((CapEnum)GetType() != eH264CapCode)
		return;

	CH264VideoCap* pH264Cap  = (CH264VideoCap*) (CBaseCap::AllocNewCap(eH264CapCode, m_dataCap));
	if (pH264Cap)
	{
	    pH264Cap->SetStaticMB(staticMB);
	    POBJDELETE(pH264Cap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::SetStaticMB - pH264Cap is NULL!!");
}

//////////////////////////////////////////////////////////////////////////////////////////
void  CVidModeH323::SetHighestH263CapForVswAuto()
{
    if ((CapEnum)m_type != eH263CapCode)
	{
		PTRACE(eLevelError, "CVidModeH323::SetHighestH263CapForVswAuto - current cap code isn't 263");
		return;
	}

	DWORD backUpRate = GetBitRate();
	CH263VideoCap* pH263Cap  = (CH263VideoCap*)(CBaseCap::AllocNewCap(eH263CapCode, NULL));
	if (pH263Cap)
	{
		EResult bRes = kSuccess;
		bRes &= pH263Cap->SetHighestCapForVswFromScmAndCardValues();
		pH263Cap->SetBitRate(backUpRate);

		if (bRes)
			Create(pH263Cap);

		PTRACE(eLevelInfoNormal, "CVidModeH323::SetHighestH263CapForVswAuto - Dump");

		pH263Cap->FreeStruct();
		POBJDELETE(pH263Cap);

		return;
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////////////
WORD CVidModeH323::SetHighestH263ScmForCP(DWORD callRate, eVideoQuality	videoQuality)
{
	DWORD backUpRate = GetBitRate();
	CH263VideoCap* pVideoCap  = (CH263VideoCap*) (CBaseCap::AllocNewCap(eH263CapCode, NULL));
	if (pVideoCap)
	{
		EResult bRes = kSuccess;
		bRes &= pVideoCap->SetDefaults();
		if (bRes)
		{
			pVideoCap->SetHighestCapForCpFromScmAndCardValues(callRate, videoQuality);//instead of call rate. pass info in K bits
			pVideoCap->SetBitRate(backUpRate);
			//if(!bIsAnnexTAllowed)  //for content as video feature
			//	((CH263VideoCap*)pVideoCap)->RemoveAnAnnexFromMask(typeAnnexT);
			Create(pVideoCap);
		}

		pVideoCap->FreeStruct();
		POBJDELETE(pVideoCap);

		WORD result = bRes ? TRUE : FALSE;
		return result;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////
WORD CVidModeH323::SetHighestH261ScmForCP(DWORD callRate, eVideoQuality	videoQuality)
{
	DWORD backUpRate = GetBitRate();
	CH261VideoCap* pVideoCap  = (CH261VideoCap*) (CBaseCap::AllocNewCap(eH261CapCode, NULL));
	if (pVideoCap)
	{
		EResult bRes = kSuccess;
		bRes &= pVideoCap->SetDefaults();
		if (bRes)
		{
			APIS8 qcifMpi, cifMpi;
			::Get261VideoCardMPI(backUpRate/10, &qcifMpi, &cifMpi, videoQuality);

			pVideoCap->SetStandardFormatsMpi(qcifMpi, cifMpi);//set QCIF and CIF
			pVideoCap->SetBitRate(backUpRate);
			Create(pVideoCap);
		}

		pVideoCap->FreeStruct();
		POBJDELETE(pVideoCap);

		WORD result = bRes ? TRUE : FALSE;
		return result;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////
WORD  CVidModeH323::SetScmMpi(CapEnum protocol, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi)
{
	DWORD backUpRate = GetBitRate();
	CBaseVideoCap* pVideoCap  = (CBaseVideoCap*) (CBaseCap::AllocNewCap(protocol, NULL));
	if (pVideoCap)
	{
		EResult bRes = kSuccess;
		bRes &= pVideoCap->SetDefaults();
		if (bRes)
		{
			if (protocol == eH261CapCode)
				((CH261VideoCap*)pVideoCap)->SetStandardFormatsMpi(qcifMpi, cifMpi);
			else if (protocol == eH263CapCode)
				((CH263VideoCap*)pVideoCap)->SetStandardFormatsMpi(qcifMpi, cifMpi, cif4Mpi, cif16Mpi);
//			else
//				PTRACE2INT2(eLevelError, "CVidModeH323::SetScmMpi : invalid protocol ", protocol);
			pVideoCap->SetBitRate(backUpRate);
			Create(pVideoCap);
		}

		pVideoCap->FreeStruct();
		POBJDELETE(pVideoCap);

		WORD result = bRes ? TRUE : FALSE;
		return result;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
//The function replace the current 263+.
//The standard resolutions remain.
WORD  CVidModeH323::SetH263ScmPlus(BYTE bAnnexP, BYTE bAnnexT, BYTE bAnnexN, BYTE bAnnexI_NS,
								   char vga, char ntsc, char svga, char xga, char qntsc)
{
	if ((CapEnum)m_type != eH263CapCode)
	{
		PTRACE(eLevelError, "CVidModeH323::SetH263ScmPlus - current cap code isn't 263");
		return FALSE;
	}

	DWORD backUpRate = GetBitRate();
	CH263VideoCap* pH263Cap  = (CH263VideoCap*)(CBaseCap::AllocNewCap(eH263CapCode, NULL));
	if (pH263Cap)
	{
		EResult bRes = kSuccess;
		bRes &= pH263Cap->SetDefaults();

		CH263VideoCap* pOldVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode, m_dataCap);
		if (pOldVideoCap)
		{
		    bRes &= pH263Cap->CopyStandardFormatsMpi(pOldVideoCap);
		    POBJDELETE(pOldVideoCap);
		}
		else
		{
		    PTRACE(eLevelError, "CVidModeH323::SetH263ScmPlus - pOldVideoCap is NULL");
			pH263Cap->FreeStruct();
			POBJDELETE(pH263Cap);
		    return FALSE;
		}

		/* In the scm AnnexI_NS is set as a regular one, and in the caps we will translate
		it to an additional NS structure.
		The reason is that FX doesn't open the channel with AnnexI_NS, but only transmit
		it. So there won't be any problem in the "is containg"*/
		bRes &= pH263Cap->SetH263Plus(bAnnexP, bAnnexT, bAnnexN, bAnnexI_NS, vga, ntsc, svga, xga, qntsc);

		pH263Cap->SetBitRate(backUpRate);

		if (bRes)
			Create(pH263Cap);

		pH263Cap->FreeStruct();
		POBJDELETE(pH263Cap);

		WORD result = bRes ? TRUE : FALSE;
		return result;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//The function replace the current 263+.
//The standard resolutions remain.
WORD  CVidModeH323::SetH263ScmInterlaced(WORD interlacedResolution, char qcifMpi, char cifMpi)
{
	DWORD backUpRate = GetBitRate();
	CH263VideoCap* pH263Cap  = (CH263VideoCap*) (CBaseCap::AllocNewCap(eH263CapCode, NULL));
	if (pH263Cap)
	{
		EResult bRes = kSuccess;
		bRes &= pH263Cap->SetDefaults();
		bRes &= pH263Cap->SetH263Interlaced((EFormat)interlacedResolution, qcifMpi, cifMpi);

		if (bRes)
			Create(pH263Cap);

		pH263Cap->SetBitRate(backUpRate);
		pH263Cap->FreeStruct();
		POBJDELETE(pH263Cap);

		WORD result = bRes ? TRUE : FALSE;
		return result;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////
BYTE  CVidModeH323::IsAutoResolution() const
{
	return m_bIsAutoResolution;
}

///////////////////////////////////////////////////////////////////////////
void  CVidModeH323::SetAutoResolution(BYTE bIsAuto)
{
	m_bIsAutoResolution = bIsAuto;
}

///////////////////////////////////////////////////////////////////////////
BYTE  CVidModeH323::IsAutoProtocol() const
{
	return m_bIsAutoProtocol;
}

///////////////////////////////////////////////////////////////////////////
void  CVidModeH323::SetAutoProtocol(BYTE bIsAuto)
{
	m_bIsAutoProtocol = bIsAuto;
}

////////////////////////////////////////////////////////////////////////////////////////
void  CVidModeH323::SetRtvScm(APIS32 width, APIS32 Height,APIS32 FR ,cmCapDirection eDirection,APIS32 BitRate)
{
	DWORD backUpRate;
	if(BitRate)
		backUpRate = BitRate;
	else
		backUpRate = GetBitRate();

	CRtvVideoCap* pRtvCap  = (CRtvVideoCap*) (CBaseCap::AllocNewCap(eRtvCapCode, NULL));
	if (pRtvCap)
	{
		EResult bRes = kSuccess;
		bRes &= pRtvCap->SetDefaults(eDirection);
		if (bRes)
		{
			pRtvCap->SetRtvWidthAndHeight(width,Height,FR,backUpRate);
			Create(pRtvCap);
		}

		pRtvCap->FreeStruct();
		POBJDELETE(pRtvCap);
	}
}


///////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////  CDataModeH323 ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CDataModeH323::CDataModeH323() {}

/////////////////////////////////////////////////////////////////////////////
void CDataModeH323::SetBitRate(int newBitRate)
{
	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CDataModeH323::SetBitRate: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapData))
		{
			CBaseDataCap* pDataCap = (CBaseDataCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pDataCap)
			{
				EResult eResOfSet  = pDataCap->SetBitRate(newBitRate);

				if (eResOfSet == kFailure)
					PTRACE(eLevelInfoNormal,"CDataModeH323::SetBitRate: Set failed!!");
			}

			PDELETE(pDataCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CDataModeH323::SetBitRate: Unknown data cap type");
	}
}

/////////////////////////////////////////////////////////////////////////////
WORD CDataModeH323::operator==(const CMediaModeH323& other) const
{
	WORD res = FALSE;
	{
		if (IsModeOff() && other.IsModeOff())
			res = TRUE;
		else if (IsModeOff() || other.IsModeOff())
			res = FALSE;
		else
		{
			if (GetType() == other.GetType())
			{
				CBaseCap* pThisCap  = GetAsCapClass();
				CBaseCap* pOtherCap = other.GetAsCapClass();
				if (pThisCap && pOtherCap)
				{
					DWORD details = 0;
					res = pThisCap->IsContaining(*pOtherCap,kBitRate,&details);
					if (res)
						res = pOtherCap->IsContaining(*pThisCap,kBitRate,&details);
				}
				POBJDELETE(pThisCap);
				POBJDELETE(pOtherCap);
			}
		}
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CDataModeH323::GetBitRate(void)const
{
	DWORD bitRate = 0;

	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CDataModeH323::GetBitRate: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapData))
		{
			CBaseDataCap *pDataCap = (CBaseDataCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pDataCap)
				bitRate	= pDataCap->GetBitRate();

			POBJDELETE(pDataCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CDataModeH323::GetBitRate: Unknown video cap type");
	}
	return bitRate;
}

/////////////////////////////////////////////////////////////////////////////
void CDataModeH323::SetFECCMode(CapEnum feccMode, DWORD feccRate, cmCapDirection eDirection)
{
	CBaseDataCap* pDataCap = (CBaseDataCap *)CBaseCap::AllocNewCap(feccMode, NULL);
	if (pDataCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet &= pDataCap->SetDefaults(eDirection);
		eResOfSet &= pDataCap->SetBitRate(feccRate);
        Create(pDataCap);
        pDataCap->FreeStruct();
		POBJDELETE(pDataCap);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CDataModeH323::SetIsSupportScp(BYTE bIsSupportScp)
{
	if (IsModeOff())
		PTRACE(eLevelInfoNormal,"CDataModeH323::SetIsSupportScp: Mode is off");
	else
	{
		CComModeInfo comModeInfo = (CapEnum)m_type;

		if(comModeInfo.IsType(cmCapData))
		{
			CBaseDataCap* pDataCap = (CBaseDataCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

			if (pDataCap)
			{
				pDataCap->SetIsSupportScp(bIsSupportScp);
			}
			PDELETE(pDataCap);
		}
		else
			PTRACE(eLevelInfoNormal,"CDataModeH323::SetScpSupport: Unknown data cap type");
	}
}
//////////////////////////////////////////////////////////////////////////
size_t  CDataModeH323::SizeOf() const
{
	CBaseDataCap	*pDataCap	= (CBaseDataCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

	size_t size =  0;

	if (pDataCap)
	{
	    size =  pDataCap->SizeOf();
	    PDELETE(pDataCap);
	}

	return size;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////  CBfcpModeH323 ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CBfcpModeH323::CBfcpModeH323():m_transType(eUnknownTransportType) {}

/////////////////////////////////////////////////////////////////////////////
EResult CBfcpModeH323::SetBfcp(enTransportType transType)
{
	EResult eResOfSet = kFailure;
	PTRACE2INT(eLevelInfoNormal,"CBfcpModeH323::SetBfcp - transport type:", transType);
	CCapSetInfo capBfcpInfo = eBFCPCapCode;

	if (capBfcpInfo.IsSupporedCap())
	{
		CBfcpCap * pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(capBfcpInfo,NULL);

		if (pBfcpCap)
		{
			eResOfSet = pBfcpCap->SetBfcp(transType);

			if(eResOfSet)
			{
				PDELETEA(m_dataCap);//PDELETE(m_dataCap); // deletes old struct   PDELETEA
				m_type       = capBfcpInfo.GetIpCapCode();
				m_dataLength = pBfcpCap->SizeOf();
				m_dataCap    = (BYTE *)pBfcpCap->GetStruct();

				PTRACE2INT(eLevelInfoNormal,"CBfcpModeH323::SetBfcp - m_type:", m_type);
				PTRACE2INT(eLevelInfoNormal,"CBfcpModeH323::SetBfcp - m_dataLength:", m_dataLength);
			}
			else
				pBfcpCap->FreeStruct();
		}
		else
			PTRACE(eLevelInfoNormal,"CBfcpModeH323::SetBfcp : Couldn't set struct!!");

		POBJDELETE(pBfcpCap);
	}

	else if ((capBfcpInfo.IsType(cmCapVideo)) && capBfcpInfo.IsSupporedCap())
	{
		PTRACE(eLevelInfoNormal,"CBfcpModeH323::SetBfcp : bfcp is off");
		SetModeOff();
	}
	return eResOfSet;
}
/////////////////////////////////////////////////////////////////////////////
WORD CBfcpModeH323::operator==(const CMediaModeH323& other) const
{
	WORD res = FALSE;
	{
		if (IsModeOff() && other.IsModeOff())
			res = TRUE;
		else if (IsModeOff() || other.IsModeOff())
			res = FALSE;
		else
		{
			if (GetType() == other.GetType())
			{
				CBaseCap* pThisCap  = GetAsCapClass();
				CBaseCap* pOtherCap = other.GetAsCapClass();
				if (pThisCap && pOtherCap)
				{
					DWORD details = 0;
					res = pThisCap->IsContaining(*pOtherCap,m_transType,&details);
					if (res)
						res = pOtherCap->IsContaining(*pThisCap,m_transType,&details);
				}
				POBJDELETE(pThisCap);
				POBJDELETE(pOtherCap);
			}
		}
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
size_t  CBfcpModeH323::SizeOf() const
{
	CBfcpCap	*pBfcpCap	= (CBfcpCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

	size_t size =  0;

	if (pBfcpCap)
	{
	    size =  pBfcpCap->SizeOf();
	    PDELETE(pBfcpCap);
	}

	return size;
}
//////////////////////////////////////////////////////////////////////////
void CBfcpModeH323::SetTransportType(enTransportType transType)
{
	CBfcpCap	*pBfcpCap	= (CBfcpCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

	if (pBfcpCap)
	{
		pBfcpCap->SetTransportType(transType);
		PDELETE(pBfcpCap);
	}
}

//////////////////////////////////////////////////////////////////////////
enTransportType CBfcpModeH323::GetTransportType() const
{
	enTransportType transType = eUnknownTransportType;

	CBfcpCap	*pBfcpCap	= (CBfcpCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

	if (pBfcpCap)
	{
		transType = pBfcpCap->GetTransportType();
		PDELETE(pBfcpCap);
	}

	return transType;
}

/////////////////////////////////////////////////////////////////////////
void CBfcpModeH323::SetBfcpParameters(eBfcpSetup setup, eBfcpConnection connection, eBfcpFloorCtrl floorCtrl, eBfcpMStreamType mstreamType)
{
	CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

	if (pBfcpCap)
	{
		pBfcpCap->SetConnection(connection);
		pBfcpCap->SetSetup(setup);
		pBfcpCap->SetFloorCntl(floorCtrl);
		pBfcpCap->SetMStreamType(mstreamType);
		POBJDELETE(pBfcpCap);
	}
	else
		PTRACE(eLevelInfoNormal,"CBfcpModeH323::SetBfcpParameters - pBfcpCap is NULL");
}

/////////////////////////////////////////////////////////////////////////
void CBfcpModeH323::SetConfUserId(WORD confId, WORD userId)
{
	CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);

	if (pBfcpCap)
	{
		pBfcpCap->SetConfId(confId);
		pBfcpCap->SetUserId(userId);
		POBJDELETE(pBfcpCap);
	}
	else
		PTRACE(eLevelInfoNormal,"CBfcpModeH323::SetConfUserId - pBfcpCap is NULL");
}
/////////////////////////////////////////////////////////////////////////
void CBfcpModeH323::SetFloorIdParams(char* pFloorid, char* pStreamid)
{
	CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
	if (pBfcpCap)
	{
		pBfcpCap->SetFloorIdParams(0, pFloorid, pStreamid, NULL, NULL, NULL);
		POBJDELETE(pBfcpCap);
	}
	else
		PTRACE(eLevelInfoNormal,"CSipCaps::SetFloorIdParamsp - pBfcpCap is NULL");
}
/////////////////////////////////////////////////////////////////////////
WORD CBfcpModeH323::GetUserId()
{
	WORD userId = 0;

	CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
	if (pBfcpCap)
	{
		userId = AtoiBounded(pBfcpCap->GetUserId());
		POBJDELETE(pBfcpCap);
	}
	else
		PTRACE(eLevelInfoNormal,"CSipCaps::GetUserId - pBfcpCap is NULL");

	return userId;
}
/////////////////////////////////////////////////////////////////////////
WORD CBfcpModeH323::GetConfId()
{
	WORD confId = 0;

	CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap((CapEnum)m_type, m_dataCap);
	if (pBfcpCap)
	{
		confId = AtoiBounded(pBfcpCap->GetConfId());
		POBJDELETE(pBfcpCap);
	}
	else
		PTRACE(eLevelInfoNormal,"CSipCaps::GetConfId - pBfcpCap is NULL");

	return confId;
}
/////////////////////////////////////////////////////////////////////////////
// Amihay: MRM CODE


#define DEBUG(X)			PTRACE(eLevelInfoNormal, 	X)
#define WARN(X)				PTRACE(eLevelInfoNormal,	X)
#define ERROR(X)			PTRACE(eLevelError, 	X)
#define NO_TRACE(X)			PTRACE(eLevelInfoNormal, 	X)
#define DEBUG_F(X)			FPTRACE(eLevelInfoNormal, 	X)
#define WARN_F(X)			FPTRACE(eLevelInfoNormal, 	X)
#define ERROR_F(X)			FPTRACE(eLevelError,	X)
#define NO_TRACE_F(X)		FPTRACE(eLevelInfoNormal, 	X)

#define DEBUG2(X,Y)			PTRACE2(eLevelInfoNormal, 	X, Y)
#define WARN2(X,Y)			PTRACE2(eLevelInfoNormal, 	X, Y)
#define ERROR2(X,Y)			PTRACE2(eLevelError, X, Y)
#define NO_TRACE2(X,Y)		PTRACE2(eLevelInfoNormal, 	X, Y)
#define DEBUG_F2(X,Y)		FPTRACE2(eLevelInfoNormal, 	X, Y)
#define WARN_F2(X,Y)		FPTRACE2(eLevelInfoNormal, 	X, Y)
#define ERROR_F2(X,Y)		FPTRACE2(eLevelError,X, Y)
#define NO_TRACE_F2(X,Y)	FPTRACE2(eLevelInfoNormal, 	X, Y)

#define DEBUG2INT(X,Y)		PTRACE2INT(eLevelInfoNormal,	X, Y)
#define WARN2INT(X,Y)		PTRACE2INT(eLevelInfoNormal,	X, Y)
#define ERROR2INT(X,Y)		PTRACE2INT(eLevelError,	X, Y)
#define NO_TRACE2INT(X,Y)	PTRACE2INT(eLevelInfoNormal, 	X, Y)
#define DEBUG_F2INT(X,Y)	FPTRACE2INT(eLevelInfoNormal, 	X, Y)
#define WARN_F2INT(X,Y)		FPTRACE2INT(eLevelInfoNormal, 	X, Y)
#define ERROR_F2INT(X,Y)	FPTRACE2INT(eLevelError, X, Y)
#define NO_TRACE_F2INT(X,Y)	FPTRACE2INT(eLevelInfoNormal, 	X, Y)

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::SetStreamsList(const std::list <StreamDesc>&  rStreams, const DWORD aPartyId)
{
//    TRACEINTO << "PartyID: " << aPartyId << " Old size: " << m_streams.size() << " New size:" << rStreams.size();
	m_streams.clear();
	m_streams.assign(rStreams.begin(), rStreams.end());
}

/////////////////////////////////////////////////////////////////////////////
ERoleLabel CMediaModeH323::GetRole() const
{
	CBaseCap* pCap  = CBaseCap::AllocNewCap((CapEnum)GetType(),m_dataCap);
	ERoleLabel eRole = kRolePeople; //role people is the default

	if (pCap)
		eRole = pCap->GetRole();

	POBJDELETE(pCap);

	return eRole;
}

/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::IntersectStreams(const CSipCaps* pCapsSet, cmCapDirection direction, DWORD aPartyId, BYTE bIsMrcSlave)
{
	CCapSetInfo capInfo = (CapEnum)GetType();
	// find the specific cap in cap set
	CBaseCap *pCap = pCapsSet->GetCapSet(capInfo,0,GetRole());
	if (pCap)
   	{
		payload_en payload_type = pCapsSet->GetPayloadTypeByDynamicPreference(capInfo,pCap->GetProfile(),GetRole());
		if (direction == cmCapReceive)
			IntersectReceiveStreams(pCap,payload_type, aPartyId, bIsMrcSlave);
		else
			IntersectSendStreams(pCap,payload_type, aPartyId);
   	}
	POBJDELETE(pCap);
}

/////////////////////////////////////////////////////////////////////////////
std::list <StreamDesc>::const_iterator CMediaModeH323::FindStreamDesc(unsigned int SSRC) const
{
	std::list<StreamDesc>::const_iterator itr_streams = m_streams.begin();
	for(;itr_streams != m_streams.end() ; itr_streams++)
	{
		if(itr_streams->m_pipeIdSsrc == SSRC)
			break;
	}
	return itr_streams;
}

//////////////////////////////////////////////////////////////////////////
bool CMediaModeH323::UpdateStreamsScpNotificationParams(CScpNotificationWrapper *pScpNotifyInd)
{
	TRACEINTOFUNC << "Updating streams with new ScpNotification params; num of pipes: " << pScpNotifyInd->m_numOfPipes;

	bool bIsStreamExists = false;
	bool bRetVal = false;

	std::list<StreamDesc>::iterator itr_streams;
	for (itr_streams = m_streams.begin(); itr_streams != m_streams.end(); itr_streams++)
	{
		bIsStreamExists = pScpNotifyInd->FillPipeById( (*itr_streams).m_pipeIdSsrc, (*itr_streams).m_scpNotificationParams );
		if (true == bIsStreamExists)
			bRetVal = true;
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
APIU32 CAudModeH323::GetMixDepth() const
{
	APIU32 mix_depth = 0;
	CBaseAudioCap	*pAudioCap	= (CBaseAudioCap *)CBaseCap::AllocNewCap((CapEnum)GetType(), m_dataCap);
	if (pAudioCap)
	{
		mix_depth =  pAudioCap->GetMixDepth();
		POBJDELETE(pAudioCap);
	}
	return mix_depth;
}


////////////////////////////////////////////////////////////////////////////
//APIU32 CAudModeH323::GetMaxRecvSsrc() const
//{
//	APIU32 max_recv_streams = 0;
//	CBaseAudioCap	*pAudioCap	= (CBaseAudioCap *)CBaseCap::AllocNewCap((CapEnum)GetType(), m_dataCap);
//	if (pAudioCap)
//	{
//		max_recv_streams =  pAudioCap->GetMaxRecvSsrc();
//		POBJDELETE(pAudioCap);
//	}
//	return max_recv_streams;
//}
//
////////////////////////////////////////////////////////////////////////////
//APIU32 CAudModeH323::GetMaxSendSsrc() const
//{
//	APIU32 max_send_streams = 0;
//	CBaseAudioCap	*pAudioCap	= (CBaseAudioCap *)CBaseCap::AllocNewCap((CapEnum)GetType(), m_dataCap);
//	if (pAudioCap)
//	{
//		max_send_streams =  pAudioCap->GetMaxSendSsrc();
//		POBJDELETE(pAudioCap);
//	}
//	return max_send_streams;
//}

/////////////////////////////////////////////////////////////////////////////
void CAudModeH323::IntersectReceiveStreams(CBaseCap *pCap, payload_en payload_type, DWORD aPartyId, BYTE bIsMrcSlave)
{

	STREAM_GROUP_S* pStreamGroup = pCap->GetRecvStreamsGroup();

	if (pStreamGroup == NULL)
	{
		ERROR("pStreamGroup is null");
		return;
	}
	// only one side (MRM) declares on "Number of receive streams"
	// these streams should enter to the intersection --> stream list should remain the same

	if (pStreamGroup->numberOfStreams != 0)
	{
		ERROR2INT("number of streams in remote cap should be 0, actual:", pStreamGroup->numberOfStreams);
	}

}
/////////////////////////////////////////////////////////////////////////////
void CAudModeH323::IntersectSendStreams(CBaseCap *pCap, payload_en payload_type, DWORD aPartyId)
{

	STREAM_GROUP_S* pStreamGroup = pCap->GetSendStreamsGroup();

	if (pStreamGroup == NULL)
	{
		ERROR("strema group is NULL");
		return;
	}

	// only one side (Remote) declares on "Number of send streams"
    // these streams should enter to the intersect cap (just the number that is <= mixDepth)
	m_streams.clear();

	APIU32 mixDepth = GetMixDepth();
//	APIU32 maxSendSsrc = GetMaxSendSsrc();
//	TRACEINTO << "maxSendSsrc: " << maxSendSsrc;
//	if (maxSendSsrc==0)
//	{
//		maxSendSsrc = GetMaxRecvSsrc();
//		TRACEINTO << "maxSendSsrc: " << maxSendSsrc;
//	}

	for (unsigned int i = 0; i < mixDepth/*maxSendSsrc*/ && i < pStreamGroup->numberOfStreams; i++)
	{
		StreamDesc streamDesc;
		streamDesc.InitDefaults();
		streamDesc.m_pipeIdSsrc = pStreamGroup->streams[i].streamSsrcId;
		streamDesc.m_payloadType = payload_type;

		m_streams.push_back(streamDesc);

		TRACEINTO << "stream " <<  i << " added (mixDepth: " << mixDepth << ", pStreamGroup->numberOfStreams: " << (int)(pStreamGroup->numberOfStreams) << ")";
	}

	if(pStreamGroup->numberOfStreams>0 && mixDepth >0)//we use the SDP SSRC value -1 as the IVR SSRC
	{
		m_IvrSsrc = (pStreamGroup->streams[0].streamSsrcId) -1 ;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CAudModeH323::CopyStreamListToStreamGroup(const std::list <StreamDesc>&  rStreams, cmCapDirection direction)
{
	WORD numOfStreams = rStreams.size();
	TRACEINTO << "Number of streams:" <<  numOfStreams << "direction:" << direction;

	STREAM_GROUP_S* pStreamGroup = NULL;
	CBaseCap *pCap = GetAsCapClass();
	if (pCap)
	{
		if(direction==cmCapReceive)
			pStreamGroup = pCap->GetRecvStreamsGroup();
		else
			pStreamGroup = pCap->GetSendStreamsGroup();
	}

	if (pStreamGroup)
	{
		pStreamGroup->numberOfStreams = numOfStreams;
		unsigned int i = 0;
		std::list<StreamDesc>::const_iterator itr_streams;
		for (itr_streams = rStreams.begin();itr_streams != rStreams.end();itr_streams++)
		{
			pStreamGroup->streams[i].streamSsrcId = itr_streams->m_pipeIdSsrc;
			if (itr_streams->m_specificSourceSsrc)
				pStreamGroup->streams[i].requstedStreamSsrcId = itr_streams->m_sourceIdSsrc;
			i++;
		}
	}
	if (pCap)
		PDELETE(pCap);
}
/////////////////////////////////////////////////////////////////////////////
void CVidModeH323::IntersectReceiveStreams(CBaseCap *pCap, payload_en payload_type, DWORD aPartyId, BYTE bIsMrcSlave)
{
	STREAM_GROUP_S* pStreamGroup = pCap->GetRecvStreamsGroup();
	if (pStreamGroup == NULL)
	{
		ERROR("stream is NULL");
		return;
	}
	if (bIsMrcSlave)
	{
		TRACEINTO << "bIsMrcSlave";
		return;
	}

	unsigned int numOfRemoteStreams = pStreamGroup->numberOfStreams;
	std::list <StreamDesc> intersectStreamsList;
	for (unsigned int i = 0; i < numOfRemoteStreams;i++)
	{
		STREAM_S rmtStream = pStreamGroup->streams[i];
		//DEBUG("SSRC: " << rmtStream.streamSsrcId);
		std::list <StreamDesc>::const_iterator itr_streams = FindStreamDesc(rmtStream.streamSsrcId);
		if (itr_streams != m_streams.end())
		{
			StreamDesc intersectStream;
			intersectStream.InitDefaults();
			// in receive stream -> payload type is taken from local scm
			intersectStream.m_payloadType = itr_streams->m_payloadType;
			intersectStream.m_pipeIdSsrc = rmtStream.streamSsrcId;
			if (rmtStream.frameWidth > itr_streams->m_width)
			{
				WARN("high width value in remote cap");
				continue;
			}
			intersectStream.m_width = itr_streams->m_width;//rmtStream.frameWidth;

			if (rmtStream.frameHeight > itr_streams->m_height)
			{
				WARN("high height value in remote cap");
				continue;
			}
			intersectStream.m_height = itr_streams->m_height;//rmtStream.frameHeight;
			// todo - add check of height and width are containing in local receive stream values. (in IsContaing function).

			if (rmtStream.maxFrameRate == 0)
				intersectStream.m_frameRate = itr_streams->m_frameRate;
			else
			{
				if (rmtStream.maxFrameRate > itr_streams->m_frameRate)
				{
					WARN("high frame rate value in remote cap");
					continue;
				}
				intersectStream.m_frameRate = rmtStream.maxFrameRate; // todo need to take the frame rate from operation points.
			}

			intersectStreamsList.push_back(intersectStream);
		}
	}
	SetStreamsList(intersectStreamsList, aPartyId);
}
/////////////////////////////////////////////////////////////////////////////
void CVidModeH323::IntersectSendStreams(CBaseCap *pCap, payload_en payload_type, DWORD aPartyId)
{
	STREAM_GROUP_S* pStreamGroup = pCap->GetSendStreamsGroup();
	if (pStreamGroup == NULL)
	{
		ERROR("stream is NULL");
		return;
	}

	// only one side (Remote) declares on "Number of send streams"
    // these streams should enter to the intersect cap (just the number that is <= mixDepth)
	unsigned int numOfRemoteStreams = pStreamGroup->numberOfStreams;
	std::list <StreamDesc> intersectStreamsList;
	for (unsigned int i = 0;i < numOfRemoteStreams;i++)
	{
		StreamDesc intersectStream;
		intersectStream.InitDefaults();
		intersectStream.m_payloadType = payload_type;

		intersectStream.m_pipeIdSsrc = pStreamGroup->streams[i].streamSsrcId;
		intersectStream.m_width = pStreamGroup->streams[i].frameWidth;
		intersectStream.m_height = pStreamGroup->streams[i].frameHeight;
		intersectStream.m_frameRate = pStreamGroup->streams[i].maxFrameRate;

		intersectStreamsList.push_back(intersectStream);
	}
	SetStreamsList(intersectStreamsList, aPartyId);
	if(numOfRemoteStreams>0)//we use the SDP SSRC value -1 as the IVR SSRC
	{

		DWORD SSRC = pStreamGroup->streams[0].streamSsrcId;

		m_IvrSsrc = (pStreamGroup->streams[0].streamSsrcId) -1 ;

	}
}
/////////////////////////////////////////////////////////////////////////////
void CVidModeH323::CopyStreamListToStreamGroup(const std::list <StreamDesc>&  rStreams, cmCapDirection direction)
{
	WORD numOfStreams = rStreams.size();
	TRACEINTO << "Number of streams:" <<  numOfStreams << "direction:" << direction;

	STREAM_GROUP_S* pStreamGroup = NULL;
	CBaseCap *pCap = GetAsCapClass();
	if (pCap)
	{
		if(direction==cmCapReceive)
			pStreamGroup = pCap->GetRecvStreamsGroup();
		else
			pStreamGroup = pCap->GetSendStreamsGroup();
	}

	if (pStreamGroup)
	{
		pStreamGroup->numberOfStreams = numOfStreams;
		unsigned int i = 0;
		std::list<StreamDesc>::const_iterator itr_streams;
		for (itr_streams = rStreams.begin();itr_streams != rStreams.end();itr_streams++)
		{
			pStreamGroup->streams[i].streamSsrcId = itr_streams->m_pipeIdSsrc;
			pStreamGroup->streams[i].frameWidth = itr_streams->m_width;
			pStreamGroup->streams[i].frameHeight = itr_streams->m_height;
			pStreamGroup->streams[i].maxFrameRate = itr_streams->m_frameRate;
			if (itr_streams->m_specificSourceSsrc)
				pStreamGroup->streams[i].requstedStreamSsrcId = itr_streams->m_sourceIdSsrc;

			i++;
		}
	}

	if (pCap)
		PDELETE(pCap);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DumpDetailsToStream(cmCapDataType eType, WORD details, std::stringstream& msg)
{
	CObjString objString;
	DumpDetailsToStream(eType, details, objString);

	msg << objString;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::IntersectStreams(const CSipCaps* pCapsSet,cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, BYTE bIsMrcSlave)
{
	CMediaModeH323& rMediaMode = GetMediaModeSvc(type,direction,eRole);
	rMediaMode.IntersectStreams(pCapsSet,direction, m_partyId, bIsMrcSlave);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CComModeH323::CopyStreamListToStreamGroup(cmCapDataType type, ERoleLabel eRole, cmCapDirection sourceDirection, cmCapDirection targetMediaModeDirection)
{
	Dump("CComModeH323::CopyStreamListToStreamGroup", eLevelInfoNormal);
	CMediaModeH323& rMediaMode = GetMediaMode(type, targetMediaModeDirection, eRole);
	const std::list <StreamDesc> streamsDescList = GetStreamsListForMediaMode(type, sourceDirection, eRole);

	rMediaMode.CopyStreamListToStreamGroup(streamsDescList, sourceDirection);
	Dump("CComModeH323::CopyStreamListToStreamGroup - after", eLevelInfoNormal);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetStreamsListForMediaMode(const std::list <StreamDesc>&  rStreams,cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	CMediaModeH323& rMediaMode = GetMediaModeSvc(type,direction,eRole);
	rMediaMode.SetStreamsList(rStreams, m_partyId);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::RemoveStreamsListForMediaMode(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	std::list <StreamDesc> streamDescList;
	streamDescList.clear();
	SetStreamsListForMediaMode(streamDescList, type, direction, eRole);
}

/////////////////////////////////////////////////////////////////////////////
unsigned char CComModeH323::GetPayloadType(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole) const
{
	const CMediaModeH323& rMediaMode = GetMediaModeSvc(type,direction,eRole);
	return	rMediaMode.GetPayloadType();
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetMediaModeSvc(capBuffer* pCapBuffer, cmCapDataType mediaType, cmCapDirection direction, ERoleLabel eRole)
{
	switch(mediaType)
	{
	case cmCapAudio:
		if (direction & cmCapReceive)
			m_audModeRcv.Create(pCapBuffer);
		if (direction & cmCapTransmit)
			m_audModeXmit.Create(pCapBuffer);
		break;
	case cmCapVideo:
		if (eRole == kRolePeople)
		{
			if (direction & cmCapReceive)
				m_vidModeRcv.Create(pCapBuffer);
			if (direction & cmCapTransmit)
				m_vidModeXmit.Create(pCapBuffer);
		}
		else //if (eRole == kRoleContent)
		{
			if (direction & cmCapReceive)
				m_vidContModeRcv.Create(pCapBuffer);
			if (direction & cmCapTransmit)
				m_vidContModeXmit.Create(pCapBuffer);
		}
		break;
	case cmCapData:
		if (direction & cmCapReceive)
			m_dataModeRcv.Create(pCapBuffer);
		if (direction & cmCapTransmit)
			m_dataModeXmit.Create(pCapBuffer);
		break;
	default:
		WARN("cmCapDataType parameter is wrong");
	}
}

/////////////////////////////////////////////////////////////////////////////
const std::list <StreamDesc>&  CComModeH323::GetStreamsListForMediaMode(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole) const
{
	const CMediaModeH323& rMediaMode = GetMediaModeSvc(type,direction,eRole);
	return	rMediaMode.GetStreams();
}

/////////////////////////////////////////////////////////////////////////////
bool  CComModeH323::IsHdVswInMixMode() const
{
	this->Dump("CComModeH323::IsHdVswInMixMode");
	const std::list <StreamDesc> streamsDescList = GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
	std::list <StreamDesc>::const_iterator itr_streams;
	for (itr_streams = streamsDescList.begin();itr_streams != streamsDescList.end();itr_streams++)
	{
//		TRACEINTO << "ssrc:" << itr_streams->m_pipeIdSsrc
//				<< " width:" << itr_streams->m_width
//				<< " height:" << itr_streams->m_height
//				<< " frame rate:" << itr_streams->m_frameRate
//				<< " m_bitRate:" << itr_streams->m_bitRate
//				<< "m_isAvcToSvcVsw" << itr_streams->m_isAvcToSvcVsw;
		if (itr_streams->m_isAvcToSvcVsw)
		{
//			TRACEINTO << "return true";
			return true;
		}
	}
//	TRACEINTO << "return false";
	return false;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CComModeH323::GetIvrSsrc(cmCapDataType type)const
{
	DWORD ssrc = 0;
	const CMediaModeH323& rMediaMode = GetMediaModeSvc(type,cmCapTransmit);
	ssrc = rMediaMode.GetIvrSsrc();

	return	ssrc;
}
///////////////////////////////////////////////////////////////////////////////
const CMediaModeH323& CComModeH323::GetMediaModeSvc(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole) const
{
	switch(type)
	{
	case cmCapAudio:
		if (direction == cmCapTransmit)
			return m_audModeXmit;
		else if (direction == cmCapReceive)
			return m_audModeRcv;
		break;
	case cmCapVideo:
		if (eRole == kRolePeople)
		{
			if (direction == cmCapTransmit)
				return m_vidModeXmit;
			else if (direction == cmCapReceive)
				return m_vidModeRcv;
		}
		else //if (eRole == kRoleContent)
		{
			if (direction == cmCapTransmit)
				return m_vidContModeXmit;
			else if (direction == cmCapReceive)
				return m_vidContModeRcv;
		}
		break;
	case cmCapData:
		if (direction == cmCapTransmit)
			return m_dataModeXmit;
		else if (direction == cmCapReceive)
			return m_dataModeRcv;
		break;
	case cmCapBfcp:
		if (direction == cmCapTransmit)
			return m_bfcpModeXmit;
		else if (direction == cmCapReceive)
			return m_bfcpModeRcv;
		break;
	default:
		break;
	}

//	SystemCoreDump(FALSE);
	// There was some error in the function parameters
	// therefore the return value will be arbitrarily m_pAudModeRcv!
	ERROR( "CComModeH323::GetMediaModeSvc Invalid direction");
	return m_audModeRcv;
}

///////////////////////////////////////////////////////////////////////////////
CMediaModeH323& CComModeH323::GetMediaModeSvc(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	switch(type)
	{
	case cmCapAudio:
		if (direction == cmCapTransmit)
			return m_audModeXmit;
		else if (direction == cmCapReceive)
			return m_audModeRcv;
		break;
	case cmCapVideo:
		if (eRole == kRolePeople)
		{
			if (direction == cmCapTransmit)
				return m_vidModeXmit;
			else if (direction == cmCapReceive)
				return m_vidModeRcv;
		}
		else //if (eRole == kRoleContent)
		{
			if (direction == cmCapTransmit)
				return m_vidContModeXmit;
			else if (direction == cmCapReceive)
				return m_vidContModeRcv;
		}
		break;
	case cmCapData:
		if (direction == cmCapTransmit)
			return m_dataModeXmit;
		else if (direction == cmCapReceive)
			return m_dataModeRcv;
		break;
	case cmCapBfcp:
		if (direction == cmCapTransmit)
			return m_bfcpModeXmit;
		else if (direction == cmCapReceive)
			return m_bfcpModeRcv;
		break;

	default:
		break;
	}

	// There was some error in the function parameters
	// therefore the return value will be arbitrarily m_pAudModeRcv!
	ERROR( "CComModeH323::GetMediaModeSvc Invalid direction");
//	SystemCoreDump(FALSE);
	return m_audModeRcv;
}

/////////////////////////////////////////////////////////////////////////////
const VideoOperationPoint*  CComModeH323::GetHighestOperationPoint(DWORD partyId)
{
	return m_operationPoints.GetHighestOperationPoint(partyId);
}

/////////////////////////////////////////////////////////////////////////////
const VideoOperationPoint*  CComModeH323::GetLowestOperationPoint(DWORD partyId) const
{
	return m_operationPoints.GetLowestOperationPoint(partyId);
}

/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetToOperationPointsOnly(const CVideoOperationPointsSet* pOperationPoints)
{
	pOperationPoints->Trace();
	m_operationPoints.SetToOperationPointsOnly(pOperationPoints);
    //m_operationPoints = *pOperationPoints;
    m_operationPoints.Trace();
}
/////////////////////////////////////////////////////////////////////////////
void CComModeH323::SetOperationPoints(const CVideoOperationPointsSet* pOperationPoints)
{
    m_operationPoints = *pOperationPoints;
    m_operationPoints.Trace();
}
void CComModeH323::SetStreamsForPayloadType(CBaseCap* pNewMediaMode, cmCapDataType type,cmCapDirection direction,ERoleLabel eRole,payload_en payload_type)
{
	if (pNewMediaMode)
	{
		CMediaModeH323 &rMediaModeH323 = GetMediaMode(type,direction,eRole);
		if (direction & cmCapReceive)
			rMediaModeH323.FillStreamsDescPayloadType(payload_type);
		else if (direction & cmCapTransmit)
			rMediaModeH323.FillStreamsDescPayloadType(payload_type);
	}
}
void CComModeH323::SetStreamsForResolutionAndBitRate(CBaseCap* pNewMediaMode, cmCapDataType type,cmCapDirection direction,ERoleLabel eRole)
{
	if (pNewMediaMode)
	{
		CMediaModeH323 &rMediaModeH323 = GetMediaMode(type,direction,eRole);
		if (direction & cmCapReceive)  // void CMediaModeH323::FillStreamsDescResolutionAndBitrate(payload_en payload_type)
			rMediaModeH323.FillStreamsDescResolutionAndBitrate(pNewMediaMode);
		else if (direction & cmCapTransmit)
			rMediaModeH323.FillStreamsDescResolutionAndBitrate(pNewMediaMode);
	}
}
/*void CComModeH323::SetDirectionForAudio()
{

	CBaseAudioCap *pCap = (CBaseAudioCap *)GetMediaAsCapClass(cmCapAudio, cmCapReceive, kRolePeople);
    if (!pCap)
    {
//    	PTRACE(eLevelInfoNormal,"@@@!CComModeH323::SetDirectionForAudio Cannot get the cmCapReceive audio cap");

    }
    pCap->SetDefaults(cmCapReceive,kRolePeople);

	pCap = (CBaseAudioCap *)GetMediaAsCapClass(cmCapAudio, cmCapTransmit, kRolePeople);
    if (!pCap)
    {
//    	PTRACE(eLevelInfoNormal,"@@@!CComModeH323::SetDirectionForAudio Cannot get the cmCapTransmit audio cap");

    }
    pCap->SetDefaults(cmCapTransmit,kRolePeople);
}*/
void CComModeH323::UpdateStreamsPayloadType(CSipCaps* pCapsSet)
{
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		if (mediaType == cmCapBfcp)
			continue;

	   	for (int direction = cmCapReceive; direction < cmCapReceiveAndTransmit; direction++)
	   	{
	   		CMediaModeH323 &rMediaModeH323 = GetMediaMode(mediaType, (cmCapDirection)direction,eRole);
	   		CCapSetInfo capInfo = (CapEnum)rMediaModeH323.GetType();
	   		// find the specific cap in cap set
	   		CBaseCap *pCap = pCapsSet->GetCapSet(capInfo,0,eRole);
			if (pCap)
	    	{
				payload_en payload_type = pCapsSet->GetPayloadTypeByDynamicPreference(capInfo,pCap->GetProfile(),eRole);
				SetPayloadTypeForMediaMode(mediaType, (cmCapDirection)direction , eRole, payload_type);
				SetStreamsForPayloadType(pCap, mediaType,(cmCapDirection)direction , eRole,payload_type);


				//SetStreamsForMediaMode
	    	    PDELETE(pCap);
	    	}
	    }
	}
}




void CComModeH323::UpdateStreamsResolutionAndBitRate(CSipCaps* pCapsSet)
{
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		if (mediaType == cmCapBfcp)
			continue;

	   	for (int direction = cmCapReceive; direction < cmCapReceiveAndTransmit; direction++)
	   	{
	   		CMediaModeH323 &rMediaModeH323 = GetMediaMode(mediaType, (cmCapDirection)direction,eRole);
	   		CCapSetInfo capInfo = (CapEnum)rMediaModeH323.GetType();
	   		// find the specific cap in cap set
	   		CBaseCap *pCap = pCapsSet->GetCapSet(capInfo,0,eRole);
			if (pCap)
	    	{
				SetStreamsForResolutionAndBitRate(pCap, mediaType,(cmCapDirection)direction , eRole);


				//SetStreamsForMediaMode
	    	    PDELETE(pCap);
	    	}
	    }
	}
}

BOOL CComModeH323::CreateStreamsDescList(cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole, APIU32 *aSsrcIds, int aNumOfSsrcIds)
{
    if (aNumOfSsrcIds == 0 || aSsrcIds == NULL)
    {
    	PTRACE(eLevelInfoNormal,"CComModeH323::CreateStreamsDescList No SSRC IDs are given.");
        return FALSE;
    }
    if (aMediaType == cmCapAudio)
    {
    	if(direction==cmCapReceive)
    	{
//    		PTRACE(eLevelInfoNormal,"@@@! CComModeH323::CreateStreamsDescList Audio - m_audModeRcv.CreateStreamsDescList");
    		m_audModeRcv.CreateStreamsDescList(aSsrcIds, aNumOfSsrcIds);
    	}
    	else if(direction==cmCapTransmit)
    	{
 //   		PTRACE(eLevelInfoNormal,"@@@! CComModeH323::CreateStreamsDescList Audio - m_audModeXmit.CreateStreamsDescList");
    		m_audModeXmit.CreateStreamsDescList(aSsrcIds, aNumOfSsrcIds); /* for Xmit SSRCs are irrelevant  */
    	}
    }
    else if (aMediaType == cmCapVideo)
    {
    	if(direction==cmCapReceive)
    	{
    		if(eRole==kRolePeople)
    		{
//				PTRACE(eLevelInfoNormal,"@@@! CComModeH323::CreateStreamsDescList Video - m_vidModeRcv.CreateStreamsDescList");
				m_vidModeRcv.CreateStreamsDescList(aSsrcIds, aNumOfSsrcIds);
    		}
    		else if(eRole==kRolePresentation)
    		{
//				PTRACE(eLevelInfoNormal,"@@@! CComModeH323::CreateStreamsDescList Video - m_vidContModeRcv.CreateStreamsDescList");
				m_vidContModeRcv.CreateStreamsDescList(aSsrcIds, aNumOfSsrcIds);

    		}
    	}
    	else if(direction==cmCapTransmit)
    	{
    		if(eRole==kRolePeople)
    		{
//				PTRACE(eLevelInfoNormal,"@@@! CComModeH323::CreateStreamsDescList Video - m_vidModeXmit.CreateStreamsDescList");
				m_vidModeXmit.CreateStreamsDescList(aSsrcIds, aNumOfSsrcIds); /* for Xmit SSRCs are irrelevant  */
    		}
    		else if(eRole==kRolePresentation)
    		{
//				PTRACE(eLevelInfoNormal,"@@@! CComModeH323::CreateStreamsDescList Video - m_vidContModeXmit.CreateStreamsDescList");
				m_vidContModeXmit.CreateStreamsDescList(aSsrcIds, aNumOfSsrcIds); /* for Xmit SSRCs are irrelevant  */
    		}
    	}
    }
    else if(aMediaType ==cmCapData)
    {
    	if(direction==cmCapReceive)
    	{
//			PTRACE(eLevelInfoNormal,"@@@! CComModeH323::CreateStreamsDescList Video - m_dataModeRcv.CreateStreamsDescList");
			m_dataModeRcv.CreateStreamsDescList(aSsrcIds, aNumOfSsrcIds); /* for data SSRCs are irrelevant  */
    	}
    	else if(direction==cmCapTransmit)
    	{
//			PTRACE(eLevelInfoNormal,"@@@! CComModeH323::CreateStreamsDescList Video - m_dataModeXmit.CreateStreamsDescList");
			m_dataModeXmit.CreateStreamsDescList(aSsrcIds, aNumOfSsrcIds); /* for data SSRCs are irrelevant  */
    	}
    }
    return FALSE;
}
//BOOL CComModeH323::GetSsrcIds(cmCapDataType aMediaType,cmCapDirection direction ,APIU32*& aSsrcIds, int* aNumOfSsrcIds)
//{
//
//    if (aMediaType == cmCapAudio)
//    {
//        // check if SAC codec is part of the SipCaps
//        CapEnum capInfo = (CapEnum)GetMediaType(cmCapAudio, direction, kRolePeople);
//        if (!CSacAudioCap::IsSacAudio(capInfo))
//        {// no stream to update
//        	PTRACE(eLevelInfoNormal, "CComModeH323::GetSsrcIds -Cannot get the audio cap");
//            return FALSE;
//        }
//
//        // update the stream id
//        CSacAudioCap *pCap = (CSacAudioCap *)GetMediaAsCapClass(cmCapAudio,direction, kRolePeople);
//        if (!pCap)
//        {
//        	PTRACE(eLevelInfoNormal, "CComModeH323::GetSsrcIds -Cannot get the audio cap");
//            return FALSE;
//        }
//
//        bool res = FALSE;
//        res = pCap->GetSsrcId(direction, aSsrcIds, aNumOfSsrcIds);
//
//        POBJDELETE(pCap);
//        return res;
//    }
//    else if (aMediaType == cmCapVideo)
//    {
//        // check if SVC codec is part of the SipCaps
//        CapEnum capInfo = (CapEnum)GetMediaType(cmCapVideo, direction, kRolePeople);
//        if (capInfo != eSvcCapCode)
//        {
//        	PTRACE(eLevelInfoNormal, "CComModeH323::GetSsrcIds -capInfo != eSvcCapCode");
//            return FALSE;
//        }
//
//        CSvcVideoCap *pCap = (CSvcVideoCap *)GetMediaAsCapClass(cmCapVideo, direction, kRolePeople);
//        if (!pCap)
//        {
//        	PTRACE(eLevelInfoNormal, "CComModeH323::GetSsrcIds -Cannot get the video cap");
//            return FALSE;
//        }
//
//        bool res = FALSE;
//        if(direction==cmCapReceive)
//        {
//        	res=pCap->GetRecvSsrcId(aSsrcIds,aNumOfSsrcIds);
//        }
//        else
//        {
//        	res=pCap->GetSendSsrcId(aSsrcIds,aNumOfSsrcIds);
//        }
//
//        POBJDELETE(pCap);
//        return res;
//
//
//
//        // check how many streams are in the cap - @@@ to be done
//        return TRUE; /* 3  */
//    }
//
//    return FALSE;
//}
/////////////////////////////////////////////////////////////////////
void CComModeH323::SetVswRelayStreamDescAccordingToOperationPoint(int layerId,cmCapDirection direction,ERoleLabel role,DWORD ssrcVideo,bool update)
{
    StreamDesc streamDesc;
    std::list <StreamDesc> streamDescList;
    CVideoOperationPointsSet* vopS = GetOperationPoints();
    VideoOperationPoint videoOperationPoint;
    vopS->Trace();
    APIU32* aSsrcIds=NULL;
    int aNumOfSsrcIds;
    streamDesc.InitDefaults();
    streamDescList.clear();

    if(update==true)
    {
    	GetSsrcIds(cmCapVideo,direction,aSsrcIds, &aNumOfSsrcIds); // tbd; get the SSRC
    	if(aSsrcIds)
    	{
    		ssrcVideo=aSsrcIds[0];
    		delete []aSsrcIds;
    	}
    }

    streamDesc.m_pipeIdSsrc = ssrcVideo;
    streamDesc.m_payloadType = GetPayloadType(cmCapVideo, direction/*cmCapReceive*/, role/*kRolePeople*/);
    vopS->GetOperationPointFromList(layerId, videoOperationPoint);


    streamDesc.m_bitRate = videoOperationPoint.m_maxBitRate;
    streamDesc.m_frameRate = videoOperationPoint.m_frameRate;
    streamDesc.m_height = videoOperationPoint.m_frameHeight;
    streamDesc.m_width = videoOperationPoint.m_frameWidth;

    streamDescList.push_back(streamDesc);
    SetStreamsListForMediaMode(streamDescList, cmCapVideo, direction/*cmCapReceive*/,role /*kRolePeople*/);
}

// Decide whether to leave the HD VSW or not
void CComModeH323::UpdateHdVswForAvcInMixMode(CBaseCap *pBaseInCap)
{
    Dump("CComModeH323::UpdateHdVswForAvcInMixMode begin: pBestMode is:\n", eLevelInfoNormal);

	if (IsMediaOff(cmCapVideo, cmCapReceive, kRolePeople))
	{
		TRACEINTO << "Audio only. Do nothing!!!";
		return;
	}

    // first of all check if there is a VSW stream
	if (!IsHdVswInMixMode())
    {
        TRACEINTO << "No VSW stream.";
        return;
    }

    // get the T0 op. point for the 720p stream
    CVideoOperationPointsSet* pOperationPointsSet = GetOperationPoints();
    VideoOperationPoint videoOperationPoint;

    int numOfStreams = pOperationPointsSet->GetNumberOfStreams();
    bool bFound = false;
    for (int i = 0; i < numOfStreams; i++)
    {
        const VideoOperationPoint * pVideoOperationPoint = pOperationPointsSet->GetHighestOperationPointForStream(i);
        if (pVideoOperationPoint && (pVideoOperationPoint->m_rsrcLevel == eResourceLevel_HD720 || pVideoOperationPoint->m_rsrcLevel == eResourceLevel_HD1080)) //JasonTest
        {
            pOperationPointsSet->GetOperationPointFromListTidDid(i, 0, videoOperationPoint);
            bFound = true;
            break;
        }
    }
    if (!bFound)
    {
        TRACEINTO << "No HD720 operation point.";
        return;
    }

    // check if the local caps are higher than the T0 for the 720p stream, allow base profile
    DWORD details = 0;
//    DWORD videoValuesToCompare = kCapCode|kH264Profile|kH264Additional|kBitRate;
    DWORD videoValuesToCompare = kCapCode|kH264Additional|kBitRate;
    CBaseCap* pRxVideoCap = pBaseInCap;
    if (!pRxVideoCap)
    {
		pRxVideoCap = GetMediaAsCapClass(cmCapVideo,  cmCapReceive);
		if (!pRxVideoCap)
		{
			TRACEINTO << "No Video Cap in SCM.";
			return;
		}
    }


	if (pRxVideoCap->GetCapCode()==eH264CapCode &&  ((CH264VideoCap*)pRxVideoCap)->IsCapableOfHD720())
    {
        TRACEINTO << "Best mode supports at least HD720. No need to remove HD stream.";

        if (!pBaseInCap)
        	POBJDELETE(pRxVideoCap);
        return;
    }

    if (!pBaseInCap)
    	POBJDELETE(pRxVideoCap);

    // remove the highest stream
    TRACEINTO << "Best mode is not HD720. Remove the HD VSW stream.";
    std::list <StreamDesc> videoStreams = GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
    videoStreams.pop_back();
    SetStreamsListForMediaMode(videoStreams, cmCapVideo, cmCapReceive, kRolePeople);

    Dump("CComModeH323::UpdateHdVswForAvcInMixMode: pBestMode is:\n", eLevelInfoNormal);
}


/*
/////////////////////////////////////////////////////////////////////
bool CComModeH323::AddSupplementalInfo(CBaseCap* pCap,cmCapDataType mediaType, cmCapDirection direction,ERoleLabel eRole)
{
	bool bRetVal = false;

	CSmallString msg;

	CH264VideoCap* pH264VideoCap;
	APIS32 Mbps,Fs,bitRate;
	APIS16 frameRate;
	int layerId;

	//CBaseCap* pCap=GetMediaAsCapClass(mediaType,direction,eRole);
	if (mediaType== cmCapVideo && (pCap) &&
		( pCap->GetRole() == kRolePeople) &&
		(pCap->GetCapCode() == eH264CapCode) )
	{
		pH264VideoCap=(CH264VideoCap*)pCap;
		Mbps=pH264VideoCap->GetMbps();
		Fs=pH264VideoCap->GetFs();
		frameRate=pH264VideoCap->GetFrameRate();
		bitRate=pH264VideoCap->GetBitRate();

		layerId = m_operationPoints.GetBestLayerIdByMbpsAndFsEx(Mbps,Fs,bitRate); // eyaln9794 // should replace 20 with bitrate
		TRACEINTO << "Opposite direction: " << direction << ", layerId: " <<layerId;

		if (-1 != layerId)
		{
			SetVswRelayStreamDescAccordingToOperationPoint(layerId,direction,eRole,0,true);
			bRetVal = true;
		}
		else
		{
			bRetVal = false;
		}
	}

	return bRetVal;
}
*/
/////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::CreateStreamsDescList(APIU32 *aSsrcIds, int num)
{

	m_streams.clear();
	for(int i = 0;i < num; i++)
	{
		StreamDesc streamDesc;
		streamDesc.InitDefaults();
		streamDesc.m_pipeIdSsrc =aSsrcIds[i]; //pStreamGroup->streams[i].streamSsrcId;

		// TODO - scp
		streamDesc.m_specificSourceSsrc = false; //is initiated only from scp request
        streamDesc.m_isLegal = true;
        streamDesc.m_isAvcToSvcVsw = false;

//		streamDesc.m_payloadType = payload_type;
//		PTRACE2INT(eLevelInfoNormal,"CMediaModeH323::FillStreamsDescList m_payloadType",m_payloadType);
//		PTRACE2INT(eLevelInfoNormal,"CMediaModeH323::FillStreamsDescList m_payloadType",streamDesc.m_payloadType);
//		PTRACE(eLevelInfoNormal,"@@@! CComModeH323::CreateStreamsDescList m_streams.push_back(streamDesc);");
		m_streams.push_back(streamDesc);
	}
}
void CMediaModeH323::FillStreamsDescPayloadType(payload_en payload_type)
{
	int i=0;

	std::list <StreamDesc>::iterator itr_streams;
	for(itr_streams=m_streams.begin();itr_streams!=m_streams.end();itr_streams++)
	{
		itr_streams->m_payloadType=m_payloadType;
		i++;
	}
}

void CMediaModeH323::FillStreamsDescResolutionAndBitrate(CBaseCap* pNewMediaMode)
{
	int i=0;

	CH264VideoCap* pCap=(CH264VideoCap*)pNewMediaMode;
	std::list <StreamDesc>::iterator itr_streams;
	for(itr_streams=m_streams.begin();itr_streams!=m_streams.end();itr_streams++)
	{
		if (GetDataType() == cmCapVideo)
		{
			itr_streams->m_bitRate=89;//pCap->GetBitRate();
			itr_streams->m_frameRate=3840;//3840;
			itr_streams->m_height=320;
			itr_streams->m_width=180;
			i++;
		}
//		itr_streams->m_payloadType=pNewMediaMode->GetPam_payloadType;
	}
/*
 virtual APIS32  GetBitRate() const;
virtual APIU8   GetLevel() const;
virtual APIS32	GetMbps() const;
virtual	APIS32  GetFs() const;

 * */

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CMediaModeH323::GetSsrcIds(APIU32*& aSsrcId, int& num)
{
    num = 0;
    aSsrcId = NULL;

    if (!IsMediaOn()) // media is off
        return;

    std::list <StreamDesc>::iterator itr_streams;
    for (itr_streams = m_streams.begin(); itr_streams != m_streams.end(); itr_streams++)
    {
        num++;
    }

    aSsrcId = new APIU32[num];
    int i = 0;
    for (itr_streams = m_streams.begin(); itr_streams != m_streams.end(); itr_streams++, i++)
    {
        aSsrcId[i] = itr_streams->m_pipeIdSsrc;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
void CVidModeH323::GetMsSvcScm(long& mbps, long& fs) const
{
	if ((CapEnum)GetType() != eMsSvcCapCode)
		return;

	CMsSvcVideoCap* pMsSvcCap  = (CMsSvcVideoCap*) (CBaseCap::AllocNewCap(eMsSvcCapCode, m_dataCap));

	if (pMsSvcCap)
	{
	    pMsSvcCap->GetMbpsAndFsAsDevision(mbps,fs);
	    POBJDELETE(pMsSvcCap);
	}
	else
	    PTRACE(eLevelError,"CVidModeH323::GetMsSvcScm - pMsSvcCap is NULL!");

}

