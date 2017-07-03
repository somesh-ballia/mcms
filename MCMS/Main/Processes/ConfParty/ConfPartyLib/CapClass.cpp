//+========================================================================+
//                            CAPCLASS.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CAPCLASS.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |09/07/02     |                                                     |
//+========================================================================+

#include <ostream>
#include  "CapClass.h"

#include "H264Util.h"
#include "H323Scm.h"
#include "NonStandardCaps.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"
#include "ConfPartyGlobals.h"
#include "IpCommon.h"
#include "H263VideoMode.h"
#include "RtvVideoMode.h"
#include "MsSvcMode.h"

/*#ifndef _HCPRIORITY_H_
#include <hcprior.h>
#endif*/

#include "ObjString.h"
#include "Trace.h"

#define CLOCK_CONVERSION_CODE_50_FILEDS_IP 1000+CLOCK_CONVERSION_CODE_50_FILEDS_ISDN
#define CLOCK_CONVERSION_CODE_60_FILEDS_IP 1000+CLOCK_CONVERSION_CODE_60_FILEDS_ISDN
#define CUSTOM_MPI_INDICATOR_IP 1+CUSTOM_MPI_INDICATOR_ISDN

using namespace std;


//                                                "QCif",   "Cif",    "4Cif",   "16Cif",    "VGA",   "NTSC",   "SVGA",    "XGA",   "QNTSC"
const int g_formatDimensions[kLastFormat][2] = {{176,144},{352,288},{704,576},{1408,1152},{640,480},{704,480},{800,600},{1024,768},{352,240}, {320,240}};

// Global functions
// template:
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Dumps the IsContaining function details into a stream
//---------------------------------------------------------------------------------------------------
void DumpDetailsToStream(cmCapDataType eType, DWORD details, CObjString& msg)
{
	// Gets the format out
	EFormat eFormat = GetVideoFormatFromDetails(details);
	// Gets the details without the format
	//details &= DETAILS_WITHOUT_FORMAT;


	CCapSetInfo capInfo;
	msg<<"IsContaining details:\n";
	if (details & HIGHER_BIT_RATE)
		msg<<"* Higher bit rate was found.\n";
	if (details & HIGHER_FORMAT)
		msg<<"* Higher format was found ("<<capInfo.GetFormatStr(eFormat)<<").\n";
	if (details & HIGHER_FRAME_RATE)
	{
		msg<<"* Higher frame rate was found";
		if (eType == cmCapVideo)
			msg <<" in "<<capInfo.GetFormatStr(eFormat);
		msg<<".\n";
	}
	if (details & LOWER_FRAME_RATE)
		msg<<"* Lower frame rate was found";
	if (details & NO_H263_PLUS)
		msg<<"* H263 plus was found.\n";
	if (details & DIFFERENT_ROLE)
		msg<<"* Different role was found.\n";
	if (details & DIFFERENT_CAPCODE)
		msg<<"* Different cap code was found.\n";
	if (details & HIGHER_LEVEL)
		msg<<"* Different H264 level was found.\n";
	if (details & HIGHER_MBPS)
	{
		//FPTRACE(eLevelInfoNormal,"DumpDetailsToStream - mbps PRINTS");
		msg<<"* Different H264 MBPS was found.\n";
	}
	if (details & HIGHER_FS)
		msg<<"* Different H264 FS was found.\n";
	if (details & HIGHER_DPB)
		msg<<"* Different H264 DPB was found.\n";
	if (details & HIGHER_BR_AND_CPB)
		msg<<"* Different H264 BrAndCpb was found.\n";
	if (details & DIFFERENT_H264MODE)
		msg<<"* Different H264 mode was found(TIP content).\n";
	if (details & HIGHER_MAXFR)
			msg<<"* Different maxFR was found(TIP content).\n";
	if (details & DIFF_PACKETIZATION_MODE)
		msg<<"Different Packetization Mode was found \n";

	if (details & ANNEXES_DETAILS)
	{
		for (annexesListEn i = typeAnnexB; i < H263_Annexes_Number; i++)
		{
			if (GetAnnexFromDetails(details) == i)
			{
				msg<<"* Annex ";
				::GetAnnexTypeName(i, msg);
				msg <<" was found.\n";
				break;
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Dumps the Non standard data parameters into a stream
//---------------------------------------------------------------------------------------------------
void DumpNonStandardData(std::ostream& msg, char *pDataArray, int dataArraySize)
{
	for( int i = 0; i < dataArraySize; i++ )
	{
		if( i%8 == 0 )
			msg << "\n        ";
		msg << " (" << (std::hex) << (int)pDataArray[i] <<")";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*void FlushMessage(std::ostream& msg)
{
	msg.seekp(std::beg);
//	memset(msg.str(), 0 ,CapClassMsgBufSize);
	msg << "Continue MCMS trace \n";
}*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Calculates format according to the x and y values
//---------------------------------------------------------------------------------------------------
EFormat CalculateFormat(WORD xWidth, WORD yHeight)
{
	for (EFormat i = kQCif; i < kLastFormat && i < MAX_VIDEO_FORMATS_INCLUDE_MONITORING; i++)
		if ((g_formatDimensions[i][WIDTH]  == xWidth )&& (g_formatDimensions[i][HEIGHT] == yHeight))
			return i;
	return kUnknownFormat;
}


//end of global

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CBaseCap
//==================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Allocates a new struct. Deletes old
//---------------------------------------------------------------------------------------------------
void CBaseCap::AllocStructBySize(size_t size)
{
	PDELETE(m_pCapStruct);
	if (size)
	{
		m_pCapStruct = (BaseCapStruct *) new BYTE[size];
		memset(m_pCapStruct,0,size);
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Set the structure header
//---------------------------------------------------------------------------------------------------
EResult CBaseCap::SetHeader(cmCapDataType eType,cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	if (m_pCapStruct)
	{
		m_pCapStruct->header.direction 		  = eDirection;
		m_pCapStruct->header.type 			  = eType;
		m_pCapStruct->header.roleLabel        = eRole;
		m_pCapStruct->header.xmlHeader.dynamicType = m_pCapStruct->header.capTypeCode;
		m_pCapStruct->header.xmlHeader.dynamicLength = SizeOf() + sizeof(BaseCapStruct);
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Set the structure role
//---------------------------------------------------------------------------------------------------
EResult CBaseCap::SetRole(ERoleLabel eRole)
{
	EResult eRes = kFailure;
	if (m_pCapStruct)
	{
		m_pCapStruct->header.roleLabel = eRole;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the cap type as capEnum
//---------------------------------------------------------------------------------------------------
EResult CBaseCap::SetCapCode(CapEnum eCapCode)
{
	EResult eRes = kFailure;
	if (m_pCapStruct)
	{
		m_pCapStruct->header.capTypeCode = (APIU8)eCapCode;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CBaseCap::SetDirection(cmCapDirection eDirection)
{
	EResult eRes = kFailure;
	if (m_pCapStruct)
	{
		m_pCapStruct->header.direction = (APIU8)eDirection;
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CBaseCap::SwitchDirection()
{
	EResult eRes = kFailure;
	if (m_pCapStruct)
	{
		if(m_pCapStruct->header.direction != cmCapReceiveAndTransmit)
			m_pCapStruct->header.direction = (m_pCapStruct->header.direction==cmCapReceive)? cmCapTransmit: cmCapReceive;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Set the structure values
//---------------------------------------------------------------------------------------------------

EResult CBaseCap::SetStruct(cmCapDataType  eType,
							cmCapDirection eDirection,
							ERoleLabel     eRole)
{
	EResult eRes = SetHeader(eType,eDirection,eRole);
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a object string
//---------------------------------------------------------------------------------------------------
void CBaseCap::Dump(CObjString* pMsg) const
{
	COstrStream msg;
	Dump(msg);
	*pMsg << msg.str().c_str();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CBaseCap::Dump(std::ostream& msg) const
{
	if (m_pCapStruct)
	{
		CCapSetInfo capInfo = (CapEnum)m_pCapStruct->header.capTypeCode;

		msg << "\nh.direction             = " << ::GetDirectionStr((cmCapDirection)m_pCapStruct->header.direction);
		msg << "\nh.type                  = " << ::GetTypeStr((cmCapDataType)m_pCapStruct->header.type);
		msg << "\nh.role                  = " << ::GetRoleStr((ERoleLabel)m_pCapStruct->header.roleLabel);
		msg << "\nh.capCode               = " << capInfo.GetH323CapName();
		msg << "\n";
	} else {
		PTRACE(eLevelInfoNormal,"CBaseCap::Dump m_pCapStruct is null");
	}
}

/////////////////////////////////////////////////////////////////////////////
void CBaseCap::Dump(const char* title, WORD level) const
{
	COstrStream msg;
	if(title != NULL)
		msg << title;
	Dump(msg);
	PTRACE(level,msg.str().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the direction
//---------------------------------------------------------------------------------------------------
cmCapDirection CBaseCap::GetDirection() const
{
	cmCapDirection eRes = cmCapReceive;
	if (m_pCapStruct)
		eRes = (cmCapDirection)m_pCapStruct->header.direction;
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CBaseCap::IsDirection(cmCapDirection eDirection) const
{
	BYTE bRes = FALSE;
	cmCapDirection eStructDirection = GetDirection();
	if (eDirection == cmCapReceiveAndTransmit)
		bRes = (eStructDirection == cmCapReceiveAndTransmit);
	else
		bRes = (eStructDirection & eDirection)? TRUE: FALSE;
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the type
//---------------------------------------------------------------------------------------------------
cmCapDataType CBaseCap::GetType() const
{
	cmCapDataType eRes = cmCapEmpty;
	if (m_pCapStruct)
		eRes = (cmCapDataType)m_pCapStruct->header.type;
	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the role label
//---------------------------------------------------------------------------------------------------
ERoleLabel CBaseCap::GetRole() const
{
	ERoleLabel eRes = kRolePeople;
	if (m_pCapStruct)
		eRes = (ERoleLabel)m_pCapStruct->header.roleLabel;
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the cap type as capEnum
//---------------------------------------------------------------------------------------------------
CapEnum CBaseCap::GetCapCode() const
{
	CapEnum eCapType = eUnknownAlgorithemCapCode;
	if (m_pCapStruct)
		eCapType = (CapEnum)m_pCapStruct->header.capTypeCode;
	return eCapType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets video cap setting
//---------------------------------------------------------------------------------------------------
void CBaseCap::GetMediaParams(CSecondaryParams &secParams, DWORD details) const
{
	CCapSetInfo capInfo = (CapEnum)m_pCapStruct->header.capTypeCode;
	secParams.m_capCode	= (CapEnum)capInfo.GetIpCapCode();
}

#define isCapInCapRange(_lowRange, _highRange, _cap) ((_cap <= _highRange) && (_cap >= _lowRange))
#define isSirenLPRCap(_cap) isCapInCapRange(eSirenLPR_32kCapCode, eSirenLPRStereo_128kCapCode, _cap)
#define isG719Cap(_cap) (isCapInCapRange(eG719_32kCapCode, eG719_128kCapCode, _cap) ||\
						 isCapInCapRange(eG719Stereo_64kCapCode, eG719Stereo_128kCapCode, _cap))
#define isiLBCCap(_cap) isCapInCapRange(eiLBC_13kCapCode, eiLBC_15kCapCode, _cap)
#define isOpusCap(_cap) isCapInCapRange(eOpus_CapCode, eOpusStereo_CapCode, _cap)

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first value that "other" object has and "this" object is lack of.
//             The first 5 bits hold value/s :
//             DIFFERENT_ROLE - Role label is different from "other"'s.
//             DIFFERENT_CAPCODE - capEnum is different from "other"'s.
//             The combination of values to compare can be only kFrameRate in that case
//---------------------------------------------------------------------------------------------------

BYTE CBaseCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x00000000;

	BaseCapStruct *pOtherCapStruct = other.GetStruct();
	if (m_pCapStruct && pOtherCapStruct)
	{
		if (valuesToCompare & kCapCode)
		{
			if (m_pCapStruct->header.capTypeCode != pOtherCapStruct->header.capTypeCode)
			{
				bRes = FALSE;
				*pDetails |= DIFFERENT_CAPCODE;
			}
		}
		if (bRes && (valuesToCompare & kRoleLabel))
		{
			if (m_pCapStruct->header.roleLabel != pOtherCapStruct->header.roleLabel)
			{
				bRes = FALSE;
				*pDetails |= DIFFERENT_ROLE;
			}
		}
	}
	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares to other cap and tells if equals
//---------------------------------------------------------------------------------------------------
BYTE CBaseCap::IsEquals(const CBaseCap& other, DWORD valuesToCompare) const
{
	BYTE bRes    = FALSE;
	DWORD details = 0;
	CSmallString str;
	cmCapDataType eType = GetType();

	bRes = IsContaining(other,valuesToCompare,&details);
	if (bRes)
	{
		bRes = other.IsContaining(*this,valuesToCompare,&details);
		if (bRes == FALSE)
		{
			DumpDetailsToStream(eType,details,str);
			PTRACE2(eLevelInfoNormal,"CBaseCap::IsEquals: Other cap set does not contain first one: ",str.GetString());
		}
	}
	else
	{
		DumpDetailsToStream(eType,details,str);
		PTRACE2(eLevelInfoNormal,"CBaseCap::IsEquals: First cap set does not contain other one: ",str.GetString());
	}

	return bRes;
}

//static

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Creates a base struct. If doesn't know what to create returns NULL.
//             There are 3 possibilities:
//             1) Use newCapCode to allocate a new struct inside the CBaseCap. In that case pStruct is NULL
//             2) Use pStruct->header.nameEnum to allocate a new CBaseCap.
//                In that case newCapCode is eUnknownAlgorithemCapCode.
//             3) Use newCapCode to allocate a new CBaseCap when pStruct is not NULL but it's data is not valid.
//---------------------------------------------------------------------------------------------------
CBaseCap* CBaseCap::AllocNewCap(CapEnum newCapCode, void *pStruct, size_t size)
{
	CBaseCap *pNewCap = NULL;

	if (pStruct)
	{
		// if we don't know the algorithm type but we have a struct, we can take the type from the struct.
		if (newCapCode == eUnknownAlgorithemCapCode)
		{
			CapEnum nameEnum = (CapEnum)((BaseCapStruct *)pStruct)->header.capTypeCode;
			newCapCode = nameEnum;
		}
	}
	CCapSetInfo capInfo = newCapCode;

	switch ((CapEnum)capInfo)
	{
		case eH261CapCode:
			pNewCap = new CH261VideoCap((h261CapStruct*)pStruct);
			break;
		case eH263CapCode:
			pNewCap = new CH263VideoCap((h263CapStruct*)pStruct);
			break;
		case eVP8CapCode: //N.A. DEBUG VP8
			pNewCap = new CVP8VideoCap((vp8CapStruct*)pStruct);
			break;
		case eH264CapCode:
			if (pStruct)
				CH264Details::AlignH264LevelsToKnownLevelIfNeeded((h264CapStruct*)pStruct);

			pNewCap = new CH264VideoCap((h264CapStruct*)pStruct);
			break;
		case eRtvCapCode:
			pNewCap = new CRtvVideoCap((rtvCapStruct*)pStruct);
			break;
		case eG7231CapCode:
			pNewCap = new CG7231AudioCap((g7231CapStruct*)pStruct);
			break;
		case eG7221C_CapCode:
			pNewCap = new CG7221CAudioCap((g7221C_CapStruct*)pStruct);
			break;
		case eT120DataCapCode:
			pNewCap = new CT120DataCap((t120DataCapStruct*)pStruct);
			break;
		case eAnnexQCapCode:
			pNewCap = new CH224DataCap((annexQDataCapStruct*)pStruct);
			break;
		case eRvFeccCapCode:
			pNewCap = new CH224DataCap((rvFeccDataCapStruct*)pStruct);
			break;
		case eNonStandardCapCode:
			pNewCap = new CNonStandardCap((ctNonStandardCapStruct*)pStruct);
			break;
		case eGenericCapCode:
			pNewCap = new CGenericCap((genericCapStruct*)pStruct);
			break;
		case eDBC2CapCode:
			pNewCap = new CDBC2VideoCap((genericVideoCapStruct*)pStruct);
			break;
		case eGenericVideoCapCode:
			pNewCap = new CGenericVideoCap((genericVideoCapStruct*)pStruct);
			break;
		case eEncryptionCapCode:
			pNewCap = new CEncryptionCap((encryptionCapStruct*)pStruct);
			break;
		case eLPRCapCode:
			pNewCap = new CLprCap((lprCapStruct*)pStruct);
			break;
		case eSdesCapCode:
			pNewCap = new CSdesCap((sdesCapStruct*)pStruct);
			break;
		case eDtlsCapCode: //_dtls_
			pNewCap = new CDtlsCap((sdesCapStruct*)pStruct);
			break;
		case eIcePwdCapCode:
			pNewCap = new CICEPwdCap((icePwdCapStruct*)pStruct);
			break;
		case eIceUfragCapCode:
			pNewCap = new CICEUfragCap((iceUfragCapStruct*)pStruct);
			break;
		case eIceCandidateCapCode:
			pNewCap = new CICECandidateCap((iceCandidateCapStruct*)pStruct);
			break;
		case eIceRemoteCandidateCapCode:
			pNewCap = new CICERemoteCandidateCap((iceRemoteCandidateCapStruct*)pStruct);
			break;
		case eRtcpCapCode:
			pNewCap = new CRtcpCap((rtcpCapStruct*)pStruct);
			break;
		case eBFCPCapCode:
			pNewCap = new CBfcpCap((bfcpCapStruct*)pStruct);
			break;
		case eFECCapCode:   //LYNC2013_FEC_RED
			pNewCap = new CFecCap((fecCapStruct*)pStruct);
			break;
		case eREDCapCode:   //LYNC2013_FEC_RED:
			pNewCap = new CRedCap((redCapStruct*)pStruct);
			break;

		case eSirenLPR_32kCapCode:
		case eSirenLPR_48kCapCode:
		case eSirenLPR_64kCapCode:
		case eSirenLPRStereo_64kCapCode:
		case eSirenLPRStereo_96kCapCode:
		case eSirenLPRStereo_128kCapCode:
			pNewCap = new SirenLPRAudioCap((sirenLPR_CapStruct*)pStruct);
			break;

		case eAAC_LDCapCode:
			pNewCap = new CAAC_LDCap((AAC_LDCapStruct*)pStruct);
			break;

		// Amihay: MRM CODE
		case eSirenLPR_Scalable_32kCapCode:
		case eSirenLPR_Scalable_48kCapCode:
		case eSirenLPR_Scalable_64kCapCode:
		case eSirenLPRStereo_Scalable_64kCapCode:
		case eSirenLPRStereo_Scalable_96kCapCode:
		case eSirenLPRStereo_Scalable_128kCapCode:
			pNewCap = new CSacAudioCap((sirenLPR_Scalable_CapStruct*)pStruct);
			break;

		case eOpus_CapCode:
		case eOpusStereo_CapCode:
				pNewCap = new COpusAudioCap((opus_CapStruct*)pStruct);
				break;

		case eSvcCapCode:
			if (pStruct)
					CH264Details::AlignH264LevelsToKnownLevelIfNeeded((h264CapStruct*)pStruct);
			pNewCap = new CSvcVideoCap((svcCapStruct*)pStruct);
			break;
		case eMsSvcCapCode:
				pNewCap = new CMsSvcVideoCap((msSvcCapStruct*)pStruct);
				break;

		default:
		{
			if (capInfo.IsType(cmCapAudio)||
				((CapEnum)capInfo == ePeopleContentCapCode)||
				((CapEnum)capInfo == eRoleLabelCapCode) ||
				((CapEnum)capInfo == eH239ControlCapCode) ||
				((CapEnum)capInfo == eDynamicPTRCapCode))
				pNewCap = new CBaseAudioCap((audioCapStructBase*)pStruct);
		}
	}

	if (pNewCap)
	{
		if (pStruct == NULL)
			pNewCap->AllocStruct(size);
		pNewCap->SetCapCode((CapEnum)capInfo);
	}

	return pNewCap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets thr reason from the details and return is as string
//---------------------------------------------------------------------------------------------------
void CBaseCap::GetDiffFromDetails(DWORD details, CSecondaryParams &secParams)
{
	details &= DETAILS_WITHOUT_FORMAT;// Gets the details without the format

	if(details == HIGHER_BIT_RATE)
	{
		secParams.m_problemParam		= LineRate;
		secParams.m_rmtProblemValue	= GetBitRate() / 10;
	}
	else if(details == HIGHER_FORMAT)
	{
		secParams.m_problemParam = Resolution;
		secParams.m_rmtProblemValue = GetFormat();

	}
	else if(details == HIGHER_FRAME_RATE)
	{
		EFormat eFormat = GetFormat();
		APIS8	mpi		= GetFormatMpi(eFormat);
		if(mpi > 0)
			mpi				= 30 / mpi;

		secParams.m_problemParam = FrameRate;
		secParams.m_rmtProblemValue = mpi;

	}
	else if(details == NO_H263_PLUS)
	{
		secParams.m_problemParam = Lacks263Plus;
	}
	else if(details == DIFFERENT_ROLE)
	{
		secParams.m_problemParam		= RoleLabel;
		secParams.m_rmtProblemValue	= m_pCapStruct->header.roleLabel;
	}
	else if (details == DIFFERENT_CAPCODE)
	{
		CCapSetInfo capInfo = (CapEnum)m_pCapStruct->header.capTypeCode;
		secParams.m_problemParam = CapCode;
		secParams.m_rmtProblemValue = capInfo.GetIpCapCode();
	}
	else if (details & ANNEXES_DETAILS)
	{
		secParams.m_problemParam = Annexes;

		for (annexesListEn i = typeAnnexB; i < H263_Annexes_Number; i++)
		{
			if (GetAnnexFromDetails(details) == i)
			{
				secParams.m_rmtProblemValue = i;
				break;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Copies all the given structure's data and role EXCEPT from the header and the name.
//             the structures should be allocated with the same size!!!
//---------------------------------------------------------------------------------------------------
EResult CBaseCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.m_pCapStruct)
	{
		eRes = kSuccess;
		int otherSizeWithoutBaseSize = otherCap.SizeOf() - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.m_pCapStruct) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, otherSizeWithoutBaseSize);
		//m_pCapStruct->header.roleLabel = otherCap.m_pCapStruct->header.roleLabel;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Creates a new cap buffer and set the cap class data into it.
// This function is used ONLY for SIP calls (and currently has a propriety SIP code problem solution
//---------------------------------------------------------------------------------------------------
capBuffer* CBaseCap::GetAsCapBuffer() const
{
	capBuffer *pCapBuffer = NULL;
	int size			= SizeOf();
	CCapSetInfo capInfo = GetCapCode();

	if (size)
	{
		pCapBuffer = (capBuffer *)new BYTE[sizeof(capBufferBase)+size];
		pCapBuffer->capTypeCode		= (CapEnum)capInfo;
		pCapBuffer->sipPayloadType  = _UnKnown;
		pCapBuffer->capLength		= size;
		memcpy(pCapBuffer->dataCap,m_pCapStruct,size);
	}
	return pCapBuffer;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CBaseAudioCap
//=======================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the Default structure values
//---------------------------------------------------------------------------------------------------
EResult CBaseAudioCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio,eDirection,eRole);
		pCapStruct->maxValue = NonFrameBasedFPP;
		pCapStruct->minValue = 0;
		//pCapStruct->rtcpFeedbackMask=0;
//		pCapStruct->rtcpFeedbackMask = RTCP_MASK_MS_DSH; //temp noa
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CBaseAudioCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(audioCapStructBase);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CBaseAudioCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(audioCapStructBase)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(audioCapStructBase));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the structure values
//---------------------------------------------------------------------------------------------------

EResult CBaseAudioCap::SetStruct(cmCapDirection eDirection,int maxValue,int minValue)
{
	EResult eRes;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);

	eRes = CBaseCap::SetStruct(cmCapAudio,eDirection);
	if (eRes == kSuccess)
	{
		pCapStruct->maxValue = maxValue;
		pCapStruct->minValue = minValue;
		pCapStruct->rtcpFeedbackMask=0;
//		pCapStruct->rtcpFeedbackMask= RTCP_MASK_MS_DSH;//noa temp
	}
	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CBaseAudioCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);
	CBaseAudioCap &hOtherAudioCap  = (CBaseAudioCap&)otherCap;

	if (GetCapCode() == hOtherAudioCap.GetCapCode())
	{
		pCapStruct->maxValue = hOtherAudioCap.GetMaxFramePerPacket();
		pCapStruct->minValue = hOtherAudioCap.GetMinFramePerPacket();;
		pCapStruct->rtcpFeedbackMask=hOtherAudioCap.GetRtcpMask();
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the structure max value (frame per packet)
//---------------------------------------------------------------------------------------------------

EResult CBaseAudioCap::SetMaxFramePerPacket(int newValue)
{
	EResult eRes = kFailure;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);
	if (pCapStruct)
	{
		pCapStruct->maxValue = newValue;
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the structure min value (frame per packet)
//---------------------------------------------------------------------------------------------------

EResult CBaseAudioCap::SetMinFramePerPacket(int newValue)
{
	EResult eRes = kFailure;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);
	if (pCapStruct)
	{
		pCapStruct->minValue = newValue;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the structure max value (frame per packet)
//---------------------------------------------------------------------------------------------------
int CBaseAudioCap::GetMaxFramePerPacket() const
{
	int res = NA;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);
	if (pCapStruct)
		res = pCapStruct->maxValue;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the structure min value (frame per packet)
//---------------------------------------------------------------------------------------------------
int CBaseAudioCap::GetMinFramePerPacket() const
{
	int res = NA;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);
	if (pCapStruct)
		res = pCapStruct->minValue;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// return the codec rate in bit/seconds
APIS32 CBaseAudioCap::GetBitRate() const
{
	APIS32 rate = -1;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);
	if (pCapStruct)
	{
		CCapSetInfo capInfo = (CapEnum)pCapStruct->header.capTypeCode;
		rate = capInfo.GetBitRate();
	}
	return rate;
}
////////////////////////////////////////////////////////////
APIS32 CBaseAudioCap::GetRtcpMask() const
{
	APIS32 rate = -1;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);
	if (pCapStruct)
	{

		return pCapStruct->rtcpFeedbackMask;
	}
	return 0;
}
EResult CBaseAudioCap::SetRtcpMask(APIS32 rtcpfeedback)
{
	EResult eRes;
	 eRes = kFailure;
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);

	if(pCapStruct)
	{
	   pCapStruct->rtcpFeedbackMask = rtcpfeedback;
	   eRes = kSuccess;

	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the bit rate.
//---------------------------------------------------------------------------------------------------
EResult CBaseAudioCap::SetBitRate(APIS32 rate)
{
	EResult eRes = kSuccess;

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CBaseAudioCap::SetH239ControlCap(cmCapDirection eDirection)
{
	EResult eRes = kSuccess;
	eRes &= CBaseCap::SetStruct(cmCapGeneric,eDirection);
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CBaseAudioCap::SetDynamicPTRepControl(cmCapDirection eDirection)
{
	EResult eRes = kSuccess;
	eRes &= CBaseCap::SetStruct(cmCapGeneric,eDirection);
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/*EResult CBaseAudioCap::SetLprControlCap(cmCapDirection eDirection)
{
	EResult eRes = kSuccess;
	lprCapStruct *pCapStruct = CAP_CAST(lprCapStruct);
	eRes &= CBaseCap::SetStruct(cmCapGeneric,eDirection);
	if (eRes == kSuccess)
	{
		pCapStruct->versionID = maxValue;
		pCapStruct->minValue = minValue;
	}
	return eRes;
}*/
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets a control content profile. Save the profile number in the frame per
//             packet field.
//---------------------------------------------------------------------------------------------------
EResult CBaseAudioCap::SetContentProfile(int profile,cmCapDirection eDirection)
{
	EResult eRes = kSuccess;
	eRes &= CBaseCap::SetStruct(cmCapGeneric,eDirection,kRoleContent);
	eRes &= SetMaxFramePerPacket(profile);
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the control content profile. The profile number is saved in the frame per
//             packet field.
//---------------------------------------------------------------------------------------------------
int CBaseAudioCap::GetContentProfile() const
{
	int profile = GetFramePerPacket();
	return profile;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets a role label struct. Save the role label in the frame per packet field.
//---------------------------------------------------------------------------------------------------
EResult CBaseAudioCap::SetRoleLabelStruct(BYTE label,cmCapDirection eDirection)
{
	EResult eRes = kSuccess;
	ERoleLabel eRole = (label == LABEL_CONTENT)? kRoleContent: kRolePeople;
	eRes &= CBaseCap::SetStruct(cmCapGeneric,eDirection,eRole);
	eRes &= SetMaxFramePerPacket(label);
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the label of a role label struct. The role label is saved in the frame per
//             packet field.
//---------------------------------------------------------------------------------------------------
BYTE CBaseAudioCap::GetLabelFromRoleLabelStruct() const
{
	BYTE label = GetFramePerPacket();
	return label;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CBaseAudioCap::Dump(std::ostream& msg) const
{
	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "Max frame per packet    = " << pCapStruct->maxValue << "\n";
		msg << "Min frame per packet    = " << pCapStruct->minValue << "\n";
		msg << "rtcp Feedback Mask      = " << pCapStruct->rtcpFeedbackMask << "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
#define isCapInCapRange(_lowRange, _highRange, _cap) ((_cap <= _highRange) && (_cap >= _lowRange))
#define isG719Cap(_cap) (isCapInCapRange(eG719_32kCapCode, eG719_128kCapCode, _cap) ||\
						 isCapInCapRange(eG719Stereo_64kCapCode, eG719Stereo_128kCapCode, _cap))

BYTE CBaseAudioCap::IsG719Match(const CBaseCap& other, DWORD *pDetails) const
{

	BYTE bRes  = TRUE;
	BaseCapStruct *pOtherCapStruct = other.GetStruct();

	bool isOtherG719Cap = isG719Cap(pOtherCapStruct->header.capTypeCode);
	bool isLocalG719Cap = isG719Cap(m_pCapStruct->header.capTypeCode);
	bool isG719Match = (isOtherG719Cap && isLocalG719Cap && (m_pCapStruct->header.capTypeCode <= pOtherCapStruct->header.capTypeCode));

	*pDetails = 0x0000;

	if (!isG719Match) {
		bRes = FALSE;
		*pDetails |= DIFFERENT_CAPCODE;
	}
	return bRes;
}

BYTE CBaseAudioCap::IsSirenLPRMatch(const CBaseCap& other, DWORD *pDetails) const
{

	BYTE bRes  = TRUE;
	BaseCapStruct *pOtherCapStruct = other.GetStruct();

	uint32_t isOtherSirenLPRCap = isSirenLPRCap(pOtherCapStruct->header.capTypeCode);
	uint32_t isLocalSirenLPRCap = isSirenLPRCap(m_pCapStruct->header.capTypeCode);
	uint32_t isSirenLPRMatch = (isOtherSirenLPRCap && isLocalSirenLPRCap && (m_pCapStruct->header.capTypeCode <= pOtherCapStruct->header.capTypeCode));

	*pDetails = 0x00000000;

	if (!isSirenLPRMatch) {
		bRes = FALSE;
		*pDetails |= DIFFERENT_CAPCODE;
	}
	return bRes;
}

BYTE CBaseAudioCap::IsiLBCMatch(const CBaseCap& other, DWORD *pDetails) const
{

	BYTE bRes  = TRUE;
	BaseCapStruct *pOtherCapStruct = other.GetStruct();

	uint32_t isOtheriLBCCap = isiLBCCap(pOtherCapStruct->header.capTypeCode);
	uint32_t isLocaliLBCCap = isiLBCCap(m_pCapStruct->header.capTypeCode);
	uint32_t isiLBCMatch = (isOtheriLBCCap && isLocaliLBCCap && (m_pCapStruct->header.capTypeCode <= pOtherCapStruct->header.capTypeCode));

	*pDetails = 0x00000000;

	if (!isiLBCMatch) {
		bRes = FALSE;
		*pDetails |= DIFFERENT_CAPCODE;
	}
	return bRes;
}

BYTE CBaseAudioCap::IsOpusMatch(const CBaseCap& other, DWORD *pDetails) const
{

	BYTE bRes  = TRUE;
	BaseCapStruct *pOtherCapStruct = other.GetStruct();

	uint32_t isOtherOpusCap = isOpusCap(pOtherCapStruct->header.capTypeCode);
	uint32_t isLocalOpusCap = isOpusCap(m_pCapStruct->header.capTypeCode);
	uint32_t isOpusMatch = (isOtherOpusCap && isLocalOpusCap);

	*pDetails = 0x00000000;

	if (!isOpusMatch) {
		bRes = FALSE;
		*pDetails |= DIFFERENT_CAPCODE;
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first value that "other" object has and "this" object is lack of.
//             The first 5 bits hold value/s :
//             HIGHER_FRAME_RATE - Frame per packet is higher in "other".
//             DIFFERENT_ROLE    - Different role (people or content)
//             The combination of values to compare can be only kFrameRate in that case
//---------------------------------------------------------------------------------------------------

BYTE CBaseAudioCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x00000000;

	audioCapStructBase *pCapStruct = CAP_CAST(audioCapStructBase);
	audioCapStructBase *pOtherCapStruct = (audioCapStructBase *)other.GetStruct();

	if (pCapStruct && pOtherCapStruct)
	{
		bRes = CBaseCap::IsContaining(other,valuesToCompare,pDetails);
		if (!bRes &&  (*pDetails & DIFFERENT_CAPCODE) && isSirenLPRCap(other.GetCapCode()))
		{
				bRes = IsSirenLPRMatch(other, pDetails);
		}
		else if (!bRes &&  (*pDetails & DIFFERENT_CAPCODE) && isG719Cap(other.GetCapCode()))
		{
			bRes = IsG719Match(other, pDetails);
		}
		else if (!bRes && isiLBCCap(other.GetCapCode()))
		{
			bRes = IsiLBCMatch(other, pDetails);
		}
		else if (!bRes && isOpusCap(other.GetCapCode()))
		{
			bRes = IsOpusMatch(other, pDetails);
		}

		if ((valuesToCompare & kFrameRate) && bRes)
		{
			if (pCapStruct->maxValue < pOtherCapStruct->maxValue)
			{
				if (pOtherCapStruct->maxValue <= 60) //we should be tolerate about it
					PTRACE2INT(eLevelError, "CBaseAudioCap::IsContaining - higher frame rate, but lower than 60 - ", pOtherCapStruct->maxValue);
				else
				{
					*pDetails |= HIGHER_FRAME_RATE;
					bRes = FALSE;
				}
			}
		}
	}
	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Description: Check validity of values according to valuesToCheck (mask of ECompareValue)
// and if bSetValidValueInstead is TRUE set valid values instead. Take values from defaultParams.
// The function will return NO if values were not valid.
//---------------------------------------------------------------------------------------------------
BYTE CBaseAudioCap::CheckValidationAndSetValidValuesIfNeeded(BYTE valuesToCheck,
															 BYTE bSetValidValueInstead,const CBaseCap *pDefaultParams)
{
	BYTE bRes = YES;
	const CBaseAudioCap & rDefault = (const CBaseAudioCap &)*pDefaultParams;

	bRes &= CBaseCap::CheckValidationAndSetValidValuesIfNeeded(valuesToCheck,bSetValidValueInstead,pDefaultParams);
	if (valuesToCheck & kFrameRate)
	{
		int value = GetFramePerPacket();
		if (value <= 0)
		{
			bRes &= NO;
			if (bSetValidValueInstead)
			{
				int defaultMaxValue = rDefault.GetMaxFramePerPacket();
				SetMaxFramePerPacket(defaultMaxValue);
				int defaultMinValue = rDefault.GetMinFramePerPacket();
				SetMinFramePerPacket(defaultMinValue);
//				SetRtcpMask(RTCP_MASK_MS_DSH);//noa temp
			}
		}
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Description: Alloc a new cap and set it's header from 'this' values.
//				Make the hightest common between 'this' and 'otherCap' and set the values in the new cap.
//---------------------------------------------------------------------------------------------------
CBaseCap* CBaseAudioCap::GetHighestCommon(const CBaseCap & otherCap) const
{
	CBaseAudioCap* pRes = NULL;

	uint32_t isOtherSirenLPRCap = isSirenLPRCap(((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode);
	uint32_t isLocalSirenLPRCap = isSirenLPRCap(m_pCapStruct->header.capTypeCode);
	uint32_t isSirenLPRMatch = (isOtherSirenLPRCap && isLocalSirenLPRCap && (m_pCapStruct->header.capTypeCode <= ((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode));

	bool isOtherG719Cap = isG719Cap(((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode);
	bool isLocalG719Cap = isG719Cap(m_pCapStruct->header.capTypeCode);
	bool isG719Match = (isOtherG719Cap && isLocalG719Cap && (m_pCapStruct->header.capTypeCode <= ((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode));

	bool isOtheriLBCCap = isiLBCCap(((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode);
	bool isLocaliLBCCap = isiLBCCap(m_pCapStruct->header.capTypeCode);
	bool isiLBCMatch = (isOtheriLBCCap && isLocaliLBCCap && (m_pCapStruct->header.capTypeCode <= ((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode));

	bool isOtherOpusCap = isOpusCap(((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode);
	bool isLocalOpusCap = isOpusCap(m_pCapStruct->header.capTypeCode);
	bool isOpusMatch = (isOtherOpusCap && isLocalOpusCap);

	if ((m_pCapStruct->header.capTypeCode == ((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode) || (isSirenLPRMatch) || (isG719Match) || (isiLBCMatch) || (isOpusMatch))
	{
		CapEnum capEnum = (CapEnum)m_pCapStruct->header.capTypeCode;
		if (isOpusMatch && (m_pCapStruct->header.capTypeCode > ((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode))
			capEnum = (CapEnum) ((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode;

		CCapSetInfo capInfo = capEnum;
		pRes = (CBaseAudioCap*)CBaseCap::AllocNewCap(capEnum, NULL);

		if (pRes)
		{
			pRes->SetHeader(cmCapAudio,(cmCapDirection)m_pCapStruct->header.direction,(ERoleLabel)m_pCapStruct->header.roleLabel);
//			pRes->SetRtcpMask(RTCP_MASK_MS_DSH); //temp noa
			int thisMaxFpp  = GetMaxFramePerPacket();
			if ((isSirenLPRMatch || isG719Match || isiLBCMatch || isOpusMatch) && thisMaxFpp == -1) {
				thisMaxFpp = FrameBasedFPP;
			}
			int otherMaxFpp = ((const CBaseAudioCap&)otherCap).GetMaxFramePerPacket();
			int maxFpp		= min(thisMaxFpp,otherMaxFpp);

			int thisMinFpp  = GetMinFramePerPacket();
			int otherMinFpp = ((const CBaseAudioCap&)otherCap).GetMinFramePerPacket();
			if (isG719Match) {
				thisMinFpp = FrameBasedFPP;
			}
			int minFpp		= max(thisMinFpp,otherMinFpp);

			maxFpp = capInfo.GetFramePerPacketQuotient(maxFpp);
			minFpp = capInfo.GetFramePerPacketQuotient(minFpp);

			if (isOpusMatch) {

				APIS32 bitrate = 0;
				APIS32 thisBitrate = ((const CBaseAudioCap *)this)->GetBitRate();
				APIS32 otherBitrate = ((const CBaseAudioCap *)(&otherCap))->GetBitRate();

				if (((const CBaseAudioCap&)otherCap).m_pCapStruct->header.capTypeCode == eOpusStereo_CapCode &&
					pRes->m_pCapStruct->header.capTypeCode == eOpus_CapCode)
					otherBitrate = rate64K;

				if (thisBitrate && otherBitrate)
					bitrate = (thisBitrate < otherBitrate) ? thisBitrate : otherBitrate;
				else if (thisBitrate)
					bitrate = thisBitrate;
				else
					bitrate = otherBitrate;

				pRes->SetBitRate(bitrate);
			}

			if (maxFpp >= minFpp)
			{
				pRes->SetMaxFramePerPacket(maxFpp);
				pRes->SetMinFramePerPacket(minFpp);
			}
			else
			{
				// no hights common
				pRes->FreeStruct();
				POBJDELETE(pRes);
			}
		}
	}

	return pRes;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



//  class COpusAudioCap
//=======================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t COpusAudioCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(opus_CapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  COpusAudioCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(opus_CapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(opus_CapStruct));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the Default structure values
//---------------------------------------------------------------------------------------------------
EResult COpusAudioCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	opus_CapStruct *pCapStruct = CAP_CAST(opus_CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio,eDirection,eRole);
		pCapStruct->maxValue = FrameBasedFPP;
		pCapStruct->minValue = 0;

		SetMaxAverageBitRateDefault();
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void COpusAudioCap::Dump(std::ostream& msg) const
{
	opus_CapStruct *pCapStruct = CAP_CAST(opus_CapStruct);

	if (pCapStruct)
	{
		CBaseAudioCap::Dump(msg);
		msg << "maxAverageBitrate       = " << pCapStruct->maxAverageBitrate << "\n";
		msg << "maxPtime                = " << pCapStruct->maxPtime << "\n";
		msg << "minPtime                = " << pCapStruct->minPtime << "\n";
		msg << "cbr                     = " << pCapStruct->cbr << "\n";
		msg << "useInbandFec            = " << pCapStruct->useInbandFec << "\n";
		msg << "useDtx                  = " << pCapStruct->useDtx << "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// return the codec highest rate possible rate in bit/seconds
//---------------------------------------------------------------------------------------------------
APIS32 COpusAudioCap::GetBitRate() const
{
	APIS32 rate = 0;

	opus_CapStruct *pCapStruct = CAP_CAST(opus_CapStruct);
	if (pCapStruct)
		rate = pCapStruct->maxAverageBitrate;

	return rate;
}

EResult COpusAudioCap::SetBitRate(APIS32 rate)
{
	EResult eRes = kFailure;
	opus_CapStruct *pCapStruct = CAP_CAST(opus_CapStruct);

	if (pCapStruct) {
		pCapStruct->maxAverageBitrate = rate;
		eRes = kSuccess;
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the Default max average bitrate
//---------------------------------------------------------------------------------------------------
void COpusAudioCap::SetMaxAverageBitRateDefault()
{
	opus_CapStruct *pCapStruct = CAP_CAST(opus_CapStruct);

	if (pCapStruct)
	{
		if (pCapStruct->header.capTypeCode == eOpus_CapCode)
			pCapStruct->maxAverageBitrate = 64000;
		else if (pCapStruct->header.capTypeCode == eOpusStereo_CapCode)
			pCapStruct->maxAverageBitrate = 128000;
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



//  class CG7231AudioCap
//=======================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CG7221CAudioCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(g7221C_CapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CG7221CAudioCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(g7221C_CapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(g7221C_CapStruct));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the Default structure values
//---------------------------------------------------------------------------------------------------
EResult CG7221CAudioCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	g7221C_CapStruct *pCapStruct = CAP_CAST(g7221C_CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio,eDirection,eRole);
		pCapStruct->maxValue = FrameBasedFPP;
		pCapStruct->minValue = 0;
		pCapStruct->capBoolMask = 0;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CG7221CAudioCap::Dump(std::ostream& msg) const
{
	g7221C_CapStruct *pCapStruct = CAP_CAST(g7221C_CapStruct);

	if (pCapStruct)
	{
		CBaseAudioCap::Dump(msg);
		msg << "G7221C 48K supported	= " << (pCapStruct->capBoolMask & g7221C_Mask_Rate48K ? "YES":"NO") << "\n";
		msg << "G7221C 32K supported	= " << (pCapStruct->capBoolMask & g7221C_Mask_Rate32K ? "YES":"NO") << "\n";
		msg << "G7221C 24K supported	= " << (pCapStruct->capBoolMask & g7221C_Mask_Rate24K ? "YES":"NO") << "\n";
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: check if the bundle of G7221C rates support a specific Cap Enum
//---------------------------------------------------------------------------------------------------
BYTE CG7221CAudioCap::isCapEnumSupported(CapEnum IpCapEnum) const
{
	BYTE eRes = FALSE;
	g7221C_CapStruct *pCapStruct = CAP_CAST(g7221C_CapStruct);

	if (pCapStruct)
	{
		if(IpCapEnum == eG7221C_48kCapCode)
			eRes = (pCapStruct->capBoolMask & g7221C_Mask_Rate48K) ? 1:0;
		else if(IpCapEnum == eG7221C_32kCapCode)
			eRes = (pCapStruct->capBoolMask & g7221C_Mask_Rate32K) ? 1:0;
		else if(IpCapEnum == eG7221C_24kCapCode)
			eRes = (pCapStruct->capBoolMask & g7221C_Mask_Rate24K) ? 1:0;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: check if the bundle of G7221C rates support a specific rate
//---------------------------------------------------------------------------------------------------
BYTE CG7221CAudioCap::isRateSupported(DWORD audioRate) const
{
	BYTE eRes = FALSE;
	g7221C_CapStruct *pCapStruct = CAP_CAST(g7221C_CapStruct);

	if (pCapStruct)
	{
		if(audioRate == rate48K)
			eRes = (pCapStruct->capBoolMask & g7221C_Mask_Rate48K) ? 1:0;
		else if(audioRate == rate32K)
			eRes = (pCapStruct->capBoolMask & g7221C_Mask_Rate32K) ? 1:0;
		else if(audioRate == rate24K)
			eRes = (pCapStruct->capBoolMask & g7221C_Mask_Rate24K) ? 1:0;
	}
	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// return the codec highest rate possible rate in bit/seconds
APIS32 CG7221CAudioCap::GetBitRate() const
{
	APIS32 rate = -1;

	if(isRateSupported(rate48K))
		return rate48K;
	if(isRateSupported(rate32K))
		return rate32K;
	if(isRateSupported(rate24K))
		return rate24K;
	return 0;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class SirenLPRAudioCap
//=======================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t SirenLPRAudioCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(sirenLPR_CapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  SirenLPRAudioCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(sirenLPR_CapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(sirenLPR_CapStruct));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the Default structure values
//---------------------------------------------------------------------------------------------------
EResult SirenLPRAudioCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	sirenLPR_CapStruct *pCapStruct = CAP_CAST(sirenLPR_CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio,eDirection,eRole);
		pCapStruct->maxValue = FrameBasedFPP;
		pCapStruct->minValue = 0;
		pCapStruct->sirenLPRMask = sirenLPRStereo;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void SirenLPRAudioCap::Dump(std::ostream& msg) const
{
	sirenLPR_CapStruct *pCapStruct = CAP_CAST(sirenLPR_CapStruct);

	if (pCapStruct)
	{
		CBaseAudioCap::Dump(msg);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// return the codec mask - Mono/Stereo
APIS32 SirenLPRAudioCap::GetSirenLPRMask() const
{
	sirenLPR_CapStruct *pCapStruct = CAP_CAST(sirenLPR_CapStruct);
	if (pCapStruct) {
		return pCapStruct->sirenLPRMask;
	}
	return 0;
}

/*****************************
 * set the sirenLPR mask value
 * 1- mono, 2 - stereo
 *****************************/
EResult SirenLPRAudioCap::SetSirenLPRMask(int mask)
{
	EResult eRes = kFailure;
	sirenLPR_CapStruct *pCapStruct = CAP_CAST(sirenLPR_CapStruct);
	if (pCapStruct)
	{
		pCapStruct->sirenLPRMask = mask;
		eRes = kSuccess;
	}
	return eRes;
}

//  class CG7231AudioCap
//=======================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CG7231AudioCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(g7231CapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CG7231AudioCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(g7231CapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(g7231CapStruct));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the Default structure values
//---------------------------------------------------------------------------------------------------
EResult CG7231AudioCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	g7231CapStruct *pCapStruct = CAP_CAST(g7231CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio,eDirection,eRole);
		pCapStruct->maxAl_sduAudioFrames = FrameBasedFPP;
		pCapStruct->capBoolMask = 0;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the structure values
//---------------------------------------------------------------------------------------------------
EResult CG7231AudioCap::SetStruct(cmCapDirection eDirection,int maxValue,int minValue)
{
	EResult eRes;
	g7231CapStruct *pCapStruct = CAP_CAST(g7231CapStruct);

	eRes = CBaseCap::SetStruct(cmCapAudio,eDirection);
	if (eRes == kSuccess)
	{
		pCapStruct->maxAl_sduAudioFrames = maxValue;
		pCapStruct->minAl_sduAudioFrames = minValue;
		pCapStruct->capBoolMask	= 0;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CG7231AudioCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	g7231CapStruct *pCapStruct = CAP_CAST(g7231CapStruct);
	CG7231AudioCap &hOtherAudioCap  = (CG7231AudioCap&)otherCap;

	if (GetCapCode() == hOtherAudioCap.GetCapCode())
	{
		eRes = kSuccess;
		pCapStruct->maxAl_sduAudioFrames = hOtherAudioCap.GetMaxFramePerPacket();
		pCapStruct->minAl_sduAudioFrames = hOtherAudioCap.GetMinFramePerPacket();;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the structure max frame per packet.
//---------------------------------------------------------------------------------------------------
EResult CG7231AudioCap::SetMaxFramePerPacket(int newValue)
{
	EResult eRes = kFailure;
	g7231CapStruct *pCapStruct = CAP_CAST(g7231CapStruct);
	if (pCapStruct)
	{
		pCapStruct->maxAl_sduAudioFrames = newValue;
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the structure min frame per packet.
//---------------------------------------------------------------------------------------------------
EResult CG7231AudioCap::SetMinFramePerPacket(int newValue)
{
	EResult eRes = kFailure;
	g7231CapStruct *pCapStruct = CAP_CAST(g7231CapStruct);
	if (pCapStruct)
	{
		pCapStruct->minAl_sduAudioFrames = newValue;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the max structure value (frame per packet)
//---------------------------------------------------------------------------------------------------
int CG7231AudioCap::GetMaxFramePerPacket() const
{
	int res = NA;
	g7231CapStruct *pCapStruct = CAP_CAST(g7231CapStruct);
	if (pCapStruct)
		res = pCapStruct->maxAl_sduAudioFrames;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the min structure value (frame per packet)
//---------------------------------------------------------------------------------------------------
int CG7231AudioCap::GetMinFramePerPacket() const
{
	int res = NA;
	g7231CapStruct *pCapStruct = CAP_CAST(g7231CapStruct);
	if (pCapStruct)
		res = pCapStruct->minAl_sduAudioFrames;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CG7231AudioCap::Dump(std::ostream& msg) const
{
	g7231CapStruct *pCapStruct = CAP_CAST(g7231CapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "maxAl_sduAudioFrames    = " << pCapStruct->maxAl_sduAudioFrames;
		msg << "\nminAl_sduAudioFrames    = " << pCapStruct->minAl_sduAudioFrames;
		msg << "\nsilenceSuppression      = " << (pCapStruct->capBoolMask & g7231_silenceSuppression?1:0) << "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first value that "other" object has and "this" object is lack of.
//             The first 5 bits hold value/s :
//             FRAME_RATE - Frame per packet is higher in "other".
//             DIFFERENT_ROLE    - Different role (people or content)
//             The combination of values to compare can be only kFrameRate in that case
//---------------------------------------------------------------------------------------------------

BYTE CG7231AudioCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x00000000;

	g7231CapStruct *pCapStruct = CAP_CAST(g7231CapStruct);
	g7231CapStruct *pOtherCapStruct = (g7231CapStruct *)other.GetStruct();

	if (pCapStruct && pOtherCapStruct)
	{
		bRes = CBaseCap::IsContaining(other,valuesToCompare,pDetails);
		if ((valuesToCompare & kFrameRate) && bRes)
		{
			if (pCapStruct->maxAl_sduAudioFrames < pOtherCapStruct->maxAl_sduAudioFrames)
			{
				bRes = FALSE;
				*pDetails |= HIGHER_FRAME_RATE;
			}
		}
	}
	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CBaseVideoCap
//=======================
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the frame rate (mpi).
//---------------------------------------------------------------------------------------------------
APIS16 CBaseVideoCap::GetFrameRate(EFormat eFormat) const
{
	APIS8 mpi = GetFormatMpi(eFormat);
	if (!mpi || mpi == -1)
		return -1;
	return 30/mpi;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Description: Check validity of values according to valuesToCheck (mask of ECompareValue)
// and if bSetValidValueInstead is TRUE set valid values instead. Take values from defaultParams.
// The function will return NO if values were not valid.
//---------------------------------------------------------------------------------------------------
BYTE CBaseVideoCap::CheckValidationAndSetValidValuesIfNeeded(BYTE valuesToCheck,
															 BYTE bSetValidValueInstead,const CBaseCap *pDefaultParams)
{
	BYTE bRes = YES;
	const CBaseVideoCap& rDefault = (const CBaseVideoCap &)*pDefaultParams;

	bRes &= CBaseCap::CheckValidationAndSetValidValuesIfNeeded(valuesToCheck,bSetValidValueInstead,pDefaultParams);
	if (valuesToCheck & kFrameRate)
	{
		BYTE bOneFrameRateValid = NO;
		for (EFormat i = kQCif ; i < kLastFormat; i++)//(EFormat i = --kLastFormat; i >= kQCif; i--)
		{
			int mpi = GetFormatMpi(i);
			bOneFrameRateValid |= (mpi > 0);
		}
		// if all frame rate were garbages set default cif 30.
		if (bOneFrameRateValid == NO)
		{
			bRes &= NO;
			if (bSetValidValueInstead)
			{
				for ( EFormat i = kQCif ; i < kLastFormat; i++)//(EFormat i = --kLastFormat; i >= kQCif; i--)
				{
					int defaultMpi = rDefault.GetFormatMpi(i);
					if (defaultMpi > 0)
						SetFormatMpi(i,defaultMpi);
				}
			}
		}
	}
	if (valuesToCheck & kBitRate)
	{
		APIS32 bitRate = GetBitRate();
		if (bitRate == -1)
		{
			bRes &= NO;
			if (bSetValidValueInstead)
			{
				APIS32 defaultBitRate = rDefault.GetBitRate();
				SetBitRate(defaultBitRate);
			}
		}
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets video cap setting
//---------------------------------------------------------------------------------------------------
void CBaseVideoCap::GetMediaParams(CSecondaryParams &secParams, DWORD details) const
{
	CCapSetInfo capInfo		= (CapEnum)m_pCapStruct->header.capTypeCode;
	secParams.m_capCode		= (CapEnum)capInfo.GetIpCapCode();

	secParams.m_resolution	= GetFormat();

	//APIS8	mpi				= GetFormatMpi((EFormat)secParams.m_resolution);
	//mpi						= 30 / mpi;

	secParams.m_frameRate		= GetFrameRate((EFormat)secParams.m_resolution); //mpi;
	secParams.m_lineRate		= GetBitRate() / 10;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CBaseVideoCap::IsThisCapBetter(EFormat* maxFormat, BYTE* bMaxHasAnnex) const
{
	EFormat curFormat = GetFormat();
	if (curFormat > *maxFormat)
	{
		*maxFormat = curFormat;
		return TRUE;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CBaseVideoCap::IsThisCapEqual(EFormat maxFormat) const
{
	EFormat curFormat = GetFormat();
	if (curFormat == maxFormat)
		return TRUE;
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Description: Alloc a new cap and set it's header from 'this' values.
//				Make the hightest common between 'this' and 'otherCap' and set the values in the new cap.
//---------------------------------------------------------------------------------------------------
CBaseCap* CBaseVideoCap::GetHighestCommon(const CBaseCap & otherCap) const	// alloc memory
{
	CBaseCap *pRes = NULL;

	if (m_pCapStruct->header.capTypeCode == ((const CBaseVideoCap &)otherCap).m_pCapStruct->header.capTypeCode)
	{
		int sizeOfNewStruct = min(SizeOf(),otherCap.SizeOf());
		BYTE *pNewStruct	= new BYTE[sizeOfNewStruct];
		memset(pNewStruct, 0, sizeOfNewStruct);
		pRes = CBaseCap::AllocNewCap((CapEnum)m_pCapStruct->header.capTypeCode,pNewStruct);

		if (pRes)
		{
			((CBaseVideoCap *)pRes)->SetHeader(cmCapVideo,(cmCapDirection)m_pCapStruct->header.direction,(ERoleLabel)m_pCapStruct->header.roleLabel);
			Intersection(otherCap, &pNewStruct);
			EFormat eBestFormat = ((CBaseVideoCap *)pRes)->GetFormat();
			if (eBestFormat != kUnknownFormat)
			{
				APIS32 thisRate  = GetBitRate();
				APIS32 otherRate = ((const CBaseVideoCap &)otherCap).GetBitRate();

				PTRACE2INT(eLevelInfoNormal,"CBaseVideoCap::GetHighestCommon: thisRate :",thisRate);
				PTRACE2INT(eLevelInfoNormal,"CBaseVideoCap::GetHighestCommon: otherRate :",otherRate);

				APIS32 rate	     = min(thisRate,otherRate);
				((CBaseVideoCap *)pRes)->SetBitRate(rate);
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CBaseVideoCap::GetHighestCommon: No common video format!");
				pRes->FreeStruct();
				POBJDELETE(pRes);
				return NULL;
			}

			if( (GetRtcpFeedbackMask() != (((const CBaseVideoCap &)otherCap).GetRtcpFeedbackMask()) ))
			{
				APIS32  intersectionOfFbMask = GetRtcpFeedbackMask() & (((const CBaseVideoCap &)otherCap).GetRtcpFeedbackMask());

				((CBaseVideoCap &)otherCap).SetRtcpFeedbackMask(intersectionOfFbMask);

				((CBaseVideoCap *)pRes)->SetRtcpFeedbackMask(intersectionOfFbMask);
				if ( IsSupportNonStandardEncode() )
					((CBaseVideoCap *)pRes)->SetRtcpFeedbackMask(intersectionOfFbMask | RTCP_MASK_IS_NOT_STANDARD_ENCODE );
			}
		}
	}
	return pRes;
}

////////////////////////////////////////////////////////////////////////////////////////
//intersect between this and pOtherCap
CBaseVideoCap* CBaseVideoCap::CreateIntersectBetweenTwoVidCaps(CBaseVideoCap* pOtherCap, cmCapDirection direction, BYTE bIntersectRate, BYTE comparePacketizationMode)
{
	if (!pOtherCap)
	{
		PTRACE (eLevelError, "CBaseVideoCap::CreateIntersectBetweenTwoVidCaps - no other cap!");
		return NULL;
	}

	if (GetCapCode() != pOtherCap->GetCapCode())
	{
		CSmallString str;
		str << "CBaseVideoCap::CreateIntersectBetweenTwoVidCaps - different cap codes. this capCode=" << GetCapCode()
		    << " other capCode = " <<  pOtherCap->GetCapCode();
		PTRACE (eLevelError, str.GetString());
		return NULL;
	}

	CBaseVideoCap* pIntersectionCap = (CBaseVideoCap*) CBaseCap::AllocNewCap((CapEnum)m_pCapStruct->header.capTypeCode, NULL);
	if (pIntersectionCap)
	{
		pIntersectionCap->SetDefaults(direction, (ERoleLabel)m_pCapStruct->header.roleLabel);
		BaseCapStruct* pIntersectStruct = pIntersectionCap->GetStruct();

		BYTE bIsEntersect = Intersection(*pOtherCap, (BYTE**)(&pIntersectStruct));

		if(bIsEntersect)
		{
			BaseCapStruct* pPrevStruct = pIntersectionCap->GetStruct();
			if (pIntersectStruct != pPrevStruct)
				//it happens in case of 263+, in which Intersection does reallocate to pIntersectStruct,
				pIntersectionCap->SetStruct(pIntersectStruct);

			APIS32 videoRate;
			if (bIntersectRate) //CP
				videoRate = min(GetBitRate(), pOtherCap->GetBitRate());
			else //VSW
				videoRate = GetBitRate();
			pIntersectionCap->SetBitRate(videoRate);
		}
		else
		{
			//Changes by VK. VNGM-937 Memory leak fix
			pIntersectionCap->FreeStruct();			//
			POBJDELETE(pIntersectionCap);
		}


	}
	return pIntersectionCap;
}

///////////////////////////////////////////////////////////////////////////////////
BYTE CBaseVideoCap::checkIsh263preffered(BYTE bIsCp, eVideoQuality vidQuality)
{
	BYTE res = FALSE;
	//In case the protocol is H264 and this is cp sharpness conf
	CSmallString str;

	if(bIsCp && (GetCapCode() == eH264CapCode) && (vidQuality == eVideoQualitySharpness))
	{
		PTRACE(eLevelInfoNormal,"CBaseVideoCap::checkIsh263preffered - Try to open H263 4CIF");
		DWORD mbps;
		DWORD fs;
		((CH264VideoCap*)this)->GetAdditionalsMbpsFsAsExplicit(mbps, fs);
		//Checking if H263 is prefered on H264 according to the intersect
		if(::IsH2634Cif15PreferedOverH264InSharpnessConf(fs, mbps))
		{
			PTRACE(eLevelInfoNormal,"CBaseVideoCap::checkIsh263preffered - H263 4CIF is Preffered on H264");
			res = TRUE;
		}
	}

	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CBaseVideoCap::GetToleraceRatePct(APIS32 maxBitRate) const
{
	static DWORD tolerancePct = 0xFFFFFFFF;
	if (tolerancePct == 0xFFFFFFFF)
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("VSW_RATE_TOLERANCE_PERECENT", tolerancePct);

	return maxBitRate*(100-tolerancePct)/100;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EFormat CBaseVideoCap::GetFormatAccordingToFS(APIS32 fs) const
{
	if ((fs >= 99) && (fs < 300) )
		return kQCif;
	else if ((fs >= 300) && (fs < 330) )
		return kQVGA;
	else if ((fs >= 330) && (fs < 396) )
		return kSIF;
	else if ((fs >= 396) && (fs < 660) )
		return kCif;
	else if ((fs >= 660) && (fs < 792) )
		return k2SIF;
	else if ((fs >= 792) && (fs < 1200) )
		return k2Cif;
	else if ((fs >= 1200) && (fs < 1320) )
		return kVGA;
	else if ((fs >= 1320) && (fs < 1350) )
		return k4SIF;
	else if ((fs >= 1350) && (fs < 1584) )
		return k525SD;
	else if ((fs >= 1584) && (fs < 1620) )
		return k4Cif;
	else if ((fs >= 1620) && (fs < 1900))
		return k625SD;
	else if ((fs >= 1900) && (fs < 3072) )
		return kSVGA;
	else if ((fs >= 3072) && (fs < 3600) )
		return kXGA;
	else if ((fs >= 3600) && (fs < 6336) )
		return k720p;
	else if ((fs >= 6336) && (fs < 8160) )
		return k16Cif;
	else if (fs >= 8160)
		return k1080p;
	else
		return kUnknownFormat;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CH261VideoCap
//=======================


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CH261VideoCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(h261CapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CH261VideoCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(h261CapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CH261VideoCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(h261CapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(h261CapStruct));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);
		pCapStruct->qcifMPI					= -1;
		pCapStruct->cifMPI					= -1;
		pCapStruct->capBoolMask				= 0;
		pCapStruct->rtcpFeedbackMask        = (RTCP_MASK_FIR | RTCP_MASK_TMMBR | RTCP_MASK_PLI );
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH261VideoCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	CH261VideoCap &hOtherVideoCap  = (CH261VideoCap&)otherCap;

	if (GetCapCode() == hOtherVideoCap.GetCapCode())
	{
		eRes = kSuccess;
		pCapStruct->maxBitRate	= hOtherVideoCap.GetBitRate();
		pCapStruct->qcifMPI		= hOtherVideoCap.GetFormatMpi(kQCif);
		pCapStruct->cifMPI		= hOtherVideoCap.GetFormatMpi(kCif);
		pCapStruct->rtcpFeedbackMask       = hOtherVideoCap.GetRtcpFeedbackMask();
	}
	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Copies the base structure without annexes  EXCEPT from the header and the name.
//             the structures should be allocated with the same size!!!
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::CopyBaseQualities(const CBaseVideoCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.GetStruct())
	{
		eRes = kSuccess;
		int sizeBaseWithAnnex = sizeof(h261CapStruct) - sizeof(APIU32);//APIU32 capBoolMask;
		int sizeWithoutBaseSize = sizeBaseWithAnnex - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.GetStruct()) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, sizeWithoutBaseSize);
	}
	return eRes;

}

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the h261 values from h221 cap set. All formats are set.
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::SetFromH221Cap(const CCapH221 & h221Cap,cmCapDirection eDirection,BYTE bIsQuad)
{
	EResult eRes = kSuccess;
	EResult eSetFormatResult = kFailure;
	CCapSetInfo capInfo = h261CapCode;

	eRes &= SetDefaults(eDirection);

	for (EFormat i = kCif; i != kUnknownFormat; i--)
		eSetFormatResult |= SetFromH221Cap(h221Cap,i);

	eRes &= eSetFormatResult;

	return eRes;
}
*/

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the h261 values from h221 cap set. Only one format is set.
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::SetFromH221Cap(const CCapH221 & h221Cap,EFormat eFormat)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	BYTE bIsParameterOK = (::isValidPObjectPtr(const_cast<CCapH221*> (&h221Cap)) && ((eFormat == kCif)||(eFormat == kQCif)));

	if (pCapStruct && bIsParameterOK)
	{
		CCapSetInfo capInfo = h261CapCode;

		eRes = kSuccess;

		WORD h320FormatType = capInfo.GetH320Format(eFormat);// V_Cif || V_Qcif
		BYTE bFormat = h221Cap.OnDataVidCap(h320FormatType);

		if (bFormat && eFormat == kCif)
			pCapStruct->cifMPI = h221Cap.m_cifMpi - 21;
		else if (bFormat && eFormat == kQCif)
			pCapStruct->qcifMPI = h221Cap.m_qCifMpi - 21;

		BYTE bIsFormatMpiSet = ((pCapStruct->cifMPI > 0)||(pCapStruct->qcifMPI > 0));
		eRes &= (EResult)bIsFormatMpiSet;
	}
	return eRes;
}
*/


/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the structure's values from a communication mode object
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::SetFromComMode(const CComMode & h320ComMode)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	BYTE bIsParameterOK = ::isValidPObjectPtr(const_cast<CComMode*> (&h320ComMode));

	if (pCapStruct && bIsParameterOK)
	{
		eRes = kSuccess;
		eRes &= SetDefaults();

		const CVidMode & vidMode = h320ComMode.m_vidMode;
		WORD       format    = vidMode.GetVidFormat();
		if (format == V_Cif)
			pCapStruct->cifMPI = vidMode.GetCifMpi() - 21;
		else if (format == V_Qcif)
			pCapStruct->qcifMPI = vidMode.GetQcifMpi() - 21;
		else
			pCapStruct->cifMPI = 4; //7.5
	}
	return eRes;
}
*/

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
//In quad we should initiate the receive qcif mode - The 320 initate the cif in usually.
EResult CH261VideoCap::SetFromComModeQuad(const CComMode & h320ComMode)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	BYTE bIsParameterOK = ::isValidPObjectPtr(const_cast<CComMode*> (&h320ComMode));

	if (pCapStruct && bIsParameterOK)
	{
		eRes = kSuccess;
		eRes &= SetDefaults();

		const CVidMode & vidMode = h320ComMode.m_vidMode;
		APIU8 mpi;
		WORD       format    = vidMode.GetVidFormat();
		if (format == V_Cif)
			mpi = vidMode.GetCifMpi() - 21;
		else if (format == V_Qcif)
			mpi = vidMode.GetQcifMpi() - 21;
		else
			mpi = 4; //7.5

		pCapStruct->qcifMPI = mpi;
	}
	return eRes;

}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets a content struct. For now we don't support h261 content so the return value
//             is kFailure.
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::SetContent(ERoleLabel eRole, cmCapDirection eDirection, BOOL isHD1080, BYTE HDMpi, BOOL isHighProfile)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	if (pCapStruct)
	{
		//atara: for now we don't support
		SetDefaults(eDirection, eRole);
	}
	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the still image transmission
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::SetStillImageTransmission(BYTE bStillImageTransmission)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		if (bStillImageTransmission)
			pCapStruct->capBoolMask |= h261_stillImageTransmission; //ON
		else
			pCapStruct->capBoolMask &= (~h261_stillImageTransmission); //OFF
	}

	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the bit rate.
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::SetBitRate(APIS32 rate)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);

	if (pCapStruct)
	{	// There is limitation on H261 rate - up to 1920k
		if(rate > 19200)
			pCapStruct->maxBitRate = 19200;
		else
			pCapStruct->maxBitRate = rate;

		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the bit rate for h261 without limitation of 1920
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::SetBitRateWithoutLimitation(APIS32 rate)
{
	TRACEINTO << "new rate = " << rate;

	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxBitRate = rate;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the mpi. Format can be kCif or kQCif
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::SetFormatMpi(EFormat eFormat, APIS8 mpi)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	BYTE bIsParameterOK = (eFormat == kCif)||(eFormat == kQCif);

	if (pCapStruct && bIsParameterOK)
	{
		if (eFormat == kCif)
			pCapStruct->cifMPI = mpi;
		else if (eFormat == kQCif)
			pCapStruct->qcifMPI = mpi;

		eRes = kSuccess;
	}
	return eRes;
}


/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Generates an h320 video mode
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::GenerateH320Mode(CVidMode *pH320VidMode) const
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	BYTE bIsParameterOK = (pH320VidMode != NULL);

	if (pCapStruct && bIsParameterOK)
	{
		WORD format  = 255;
		int  qcifMpi = 0;
		int  cifMpi  = 0;

		if (pCapStruct->cifMPI > (APIU8)-1)
		{
			format = V_Cif;
			cifMpi = pCapStruct->cifMPI + 21;
		}
		else if (pCapStruct->qcifMPI > (APIU8)-1)
		{
			format  = V_Qcif;
			qcifMpi = pCapStruct->qcifMPI + 21;
		}

		pH320VidMode->SetH261VidMode(0,format,qcifMpi,cifMpi);
		eRes = kSuccess;
	}
	return eRes;
}
*/


/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Generates an h221 video cap
//---------------------------------------------------------------------------------------------------
EResult CH261VideoCap::GenerateH221Caps(CCapH221* pH221Cap, BYTE bGenerateFromCaps) const
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	BYTE bIsParameterOK = (pH221Cap != NULL);

	if (pCapStruct && bIsParameterOK)
	{

		WORD format  = 255;
		int  qcifMpi = 0;
		int  cifMpi  = 0;

		if (pCapStruct->cifMPI > (APIU8)-1)
		{
			cifMpi  = pCapStruct->cifMPI + 21;
			qcifMpi = cifMpi;
			pH221Cap->SetVidCap(V_Cif, cifMpi, qcifMpi);
		}

		if (pCapStruct->qcifMPI > (APIU8)-1)
		{
			qcifMpi = pCapStruct->qcifMPI + 21;
			pH221Cap->SetVidCap( V_Qcif, qcifMpi );
		}
		// else video is off

		eRes = kSuccess;
	}
	return eRes;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CH261VideoCap::Dump(std::ostream& msg) const
{
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "qcifMPI                 = " << (int)pCapStruct->qcifMPI << "\n";
		msg << "cifMPI                  = " << (int)pCapStruct->cifMPI << "\n";
		msg << "tempTradeOffCapability  = " << (pCapStruct->capBoolMask & h261_temporalSpatialTradeOffCapability?1:0) << "\n";
		msg << "stillImageTransmission  = " << (pCapStruct->capBoolMask & h261_stillImageTransmission?1:0) << "\n";
		msg << "rtcpFeedbackMask        = " << pCapStruct->rtcpFeedbackMask << "\n";

		if (pCapStruct->maxBitRate > 0)
			msg << "maxBitRate              = " << pCapStruct->maxBitRate <<
			" (" << (pCapStruct->maxBitRate)*100 << " bps)" ;
		else
			msg << "maxBitRate              = Unknown";
	}
}

/*
 * pOtherCap  - is the local video cap.
 * pCapStruct - this is the remote.
 * pOtherCapStruct - Is the local 261 struct.
 * */
//---------------------------------------------------------------------------------------------------
BYTE CH261VideoCap::Intersection(const CBaseCap& other, BYTE **ppTemp, BYTE comparePacketizationMode) const
{
	//We need to do a Intersection and we want all the resolutions that initialize to take with MPi that
	//fit to both sides
	//We will start from the bigger resolution and save it's MPI and compare, we will take the maximum MPI.

#define		MAXRATE		99
	const CH261VideoCap *pOtherCap = (const CH261VideoCap *)(&other);

	BYTE				bIsSuccess			= FALSE;
	h261CapStruct		*pCapStruct			= CAP_CAST(h261CapStruct);
	h261CapStruct		*pOtherCapStruct	= (h261CapStruct *)other.GetStruct();
	h261CapStruct		*pTemp261			= (h261CapStruct *)*ppTemp;

	if (pOtherCap && pCapStruct && pOtherCapStruct)
	{
		APIS8	framRate		= 99;
		APIS8	otherFrameRate	= 99;
		APIS8* pStructMpi		= &(pCapStruct->cifMPI);
		APIS8* pOtherStructMpi	= &(pOtherCapStruct->cifMPI);
		APIS8* pTempStructMpi	= &(pTemp261->cifMPI);

		for (EFormat i = kCif; i >= kQCif; i--)
		{
			if(*pStructMpi > 0)
				framRate = *pStructMpi;
			if(*pOtherStructMpi > 0)
				otherFrameRate = *pOtherStructMpi;

			if ((*pStructMpi < 0) && (*pOtherStructMpi < 0)) //if both -1 the result is -1
				*pTempStructMpi = -1;
			else
			{
				if ((framRate == MAXRATE) || (otherFrameRate == MAXRATE)) //if only one side has resolution the intersection is -1
					*pTempStructMpi = -1;
				else
				{
					*pTempStructMpi = max(framRate,otherFrameRate);
					bIsSuccess = TRUE;
				}
			}

			pStructMpi--;
			pOtherStructMpi--;
			pTempStructMpi--;
		}

		pTemp261->rtcpFeedbackMask = GetRtcpFeedbackMask() & (pOtherCapStruct->rtcpFeedbackMask);
		if (GetRtcpFeedbackMask() & RTCP_MASK_IS_NOT_STANDARD_ENCODE)
		{
			pTemp261->rtcpFeedbackMask  |= RTCP_MASK_IS_NOT_STANDARD_ENCODE;
		}
	}

	return bIsSuccess;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first found value that "other" object has and "this" object is lack of. When such value
//             is found, the function stops checking other values (and the return value will be FALSE)
//             The comparison will be in the following order.
//             The first 4 bits hold value/s :
//             * HIGHER_BIT_RATE   - Other's bit rate is higher.
//             * HIGHER_FORMAT     - Other's format is higher.
//             * HIGHER_FRAME_RATE - Frame per packet is higher in "other".
//             * Annex type (according to annexesListEn enum) + DETAIL_ANNEXES_OFFSET.
//             * DIFFERENT_ROLE    - Different role (people or content)
//             Bits 4-7 of the comparison details are the video format (EFormat) that the
//			   values HIGHER_FORMAT, HIGHER_FRAME_RATE and annexes relate to.
//             The combination of values to compare can be from kBitRate, kFormat, kFrameRate or kAnnexes.
//             There is no posibility to compare kFrameRate without kFormat.
//---------------------------------------------------------------------------------------------------

BYTE CH261VideoCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x00000000;

	h261CapStruct* pCapStruct = CAP_CAST(h261CapStruct);
	h261CapStruct* pOtherCapStruct = (h261CapStruct *)other.GetStruct();

	if (pCapStruct && pOtherCapStruct)
	{
		// Starts the comparison. If in some stage res gets FALSE the comparison stops
		// and the value that "this" struct was lack of will be saved in the pDetails.

		bRes = CBaseCap::IsContaining(other,valuesToCompare,pDetails);

		// Saves the higher video format for the annex details (if needed)
		EFormat higherVideoFormat = kQCif;

		// If the last comparison did not fail check resolutions
		if ((valuesToCompare & kFormat) && bRes)
		{
			EFormat index;
			APIS8* pOtherStructMpi = &(pOtherCapStruct->qcifMPI);
			APIS8* pStructMpi;
			for (EFormat i = kQCif; i <= kCif; i++)
			{
				if (*pOtherStructMpi > 0)
				{
					pStructMpi = &(pCapStruct->qcifMPI);
					pStructMpi += i;
					for (index = i; index <= kCif; index++)//check only from the current resolution and higher
					{
						if (*pStructMpi > 0)
						{
							higherVideoFormat = max(higherVideoFormat,i);

							if (valuesToCompare & kFrameRate)
							{
								if (*pOtherStructMpi < *pStructMpi)
								{
									*pDetails |= HIGHER_FRAME_RATE;
									*pDetails |= ShiftVideoFormat(i);
									return FALSE;
								}
								else
									break; //appropriate parameters were found for the current resolution
							}
							break;
						}
						pStructMpi++;
					}

					if (index > kCif) //=> inappropriate parameters were found
					{
						bRes = FALSE;
						*pDetails |= HIGHER_FORMAT;
						*pDetails |= ShiftVideoFormat(i);
					}
				}
				pOtherStructMpi++;
			}
		}

		if ((valuesToCompare & kAnnexes)&&(bRes))
		{
			// get system.cfg values
			DWORD checkAnnexD = GetSystemCfgFlagInt<DWORD>(CFG_KEY_H323_CHECK_ANNEX_D);//::GetpSystemCfg()->IsCheck_ANNEX_D();

			if(checkAnnexD)
			{
				if ( (pOtherCapStruct->capBoolMask & h261_stillImageTransmission) &&
						!(pCapStruct->capBoolMask & h261_stillImageTransmission))
				{
					bRes = FALSE;
					*pDetails |= ShiftAnnexDetails(typeAnnexD);
					*pDetails |= ShiftVideoFormat(higherVideoFormat);
				}
			}
		}

		if ((valuesToCompare & kBitRate) && bRes)
		{

			if (pCapStruct->maxBitRate < GetToleraceRatePct(pOtherCapStruct->maxBitRate))
			{
				bRes = FALSE;
				*pDetails |= HIGHER_BIT_RATE;
			}
		}
		if ((valuesToCompare & kBitRateForCascade) && bRes)
		{
			if (pCapStruct->maxBitRate < (pOtherCapStruct->maxBitRate * CASCADE_BW_FACTOR) / 100)
			{
				bRes = FALSE;
				*pDetails |= HIGHER_BIT_RATE;
			}
		}
		if ((valuesToCompare & kBitRateWithoutTolerance) && bRes)
		{
			if (pCapStruct->maxBitRate < pOtherCapStruct->maxBitRate)
			{
				bRes = FALSE;
				*pDetails |= HIGHER_BIT_RATE;
			}
		}

	}
	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the video format. kCif or kQcif
//---------------------------------------------------------------------------------------------------
EFormat CH261VideoCap::GetFormat() const
{
	EFormat eRes = kUnknownFormat;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);

	if (pCapStruct)
	{
		if (pCapStruct->cifMPI != (APIS8)-1)
			eRes = kCif;
		else if (pCapStruct->qcifMPI != (APIS8)-1)
			eRes = kQCif;
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the mpi of a specific format. format can be kCif or kQcif
//---------------------------------------------------------------------------------------------------
APIS8 CH261VideoCap::GetFormatMpi(EFormat eFormat) const
{
	APIS8 mpi = -1;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	BYTE bIsParameterOK = (eFormat == kCif)||(eFormat == kQCif);

	if (pCapStruct && bIsParameterOK)
	{
		if (eFormat == kCif)
			mpi = pCapStruct->cifMPI;

		else if (eFormat == kQCif)
			mpi = pCapStruct->qcifMPI;
	}
	return mpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if the format is supported in the struct (if it's mpi > 0)
//---------------------------------------------------------------------------------------------------
BYTE CH261VideoCap::IsFormat(EFormat eFormat) const
{
	BYTE bRes = (GetFormatMpi(eFormat) > 0);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the bit rate
//---------------------------------------------------------------------------------------------------
APIS32 CH261VideoCap::GetBitRate() const
{
	APIS32 bitRate = -1;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);

	if (pCapStruct)
		bitRate = pCapStruct->maxBitRate;

	return bitRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH261VideoCap::AddLowerResolutionsIfNeeded()
{
	h261CapStruct* pCapStruct = CAP_CAST(h261CapStruct);
	if (pCapStruct)
	{
		if ( (pCapStruct->cifMPI > 0) && (pCapStruct->qcifMPI == -1) )
			pCapStruct->qcifMPI = pCapStruct->cifMPI;
	}
}

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH261VideoCap::IsHighestCommonConditionSupported(CHighestCommonCondition& hcCondition) const
{
	if (hcCondition.GetVideoProtocol(H323_INTERFACE_TYPE) != h261CapCode)
		return FALSE;

	BYTE bRes = FALSE;
	h261CapStruct* pCapStruct = CAP_CAST(h261CapStruct);
    if (pCapStruct)
	{
		EFormat hcResolution = (EFormat)hcCondition.GetResolution(H323_INTERFACE_TYPE);
		EFormat curFormat = GetFormat();
		if (hcResolution <= curFormat)
			bRes = TRUE;
	}
	return bRes;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH261VideoCap::SetStandardFormatsMpi(APIS8 qcifMpi, APIS8 cifMpi)
{
	h261CapStruct* pCapStruct = CAP_CAST(h261CapStruct);
	if (!pCapStruct)
		return;

	pCapStruct->qcifMPI  = qcifMpi;
	pCapStruct->cifMPI   = cifMpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////
EResult  CH261VideoCap::SetHighestCapForCpFromScmAndCardValues(DWORD videoRate,  eVideoQuality videoQuality)
{
/*
	DWORD rateInBitPerSec = videoRate * 1000;
	BYTE numberOf64s = (rateInBitPerSec % rate64K) ? (rateInBitPerSec/rate64K + 1) : rateInBitPerSec/rate64K;
	DWORD videoRateIn64bits = numberOf64s * 64;
*/

	EResult eRes = kSuccess;
	BYTE buffer[MAX_VIDEO_FORMATS];
	memset(buffer, -1, MAX_VIDEO_FORMATS);

	APIS8 qcifMpi, cifMpi;
	::Get261VideoCardMPI(videoRate/10, &qcifMpi, &cifMpi, videoQuality);

	EResult eResOfSet = kSuccess;
	eResOfSet &= SetFormatMpi(kQCif, qcifMpi);
	eResOfSet &= SetFormatMpi(kCif, cifMpi);

	if (eResOfSet)
	{
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		BOOL bIsStillImageTransmissionSupported = 0;
		std::string key = "H261_StillImageTransmission";
		pSysConfig->GetBOOLDataByKey(key, bIsStillImageTransmissionSupported);

		eResOfSet &= SetStillImageTransmission(bIsStillImageTransmissionSupported);
	}

	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CH261VideoCap::GetCPVideoPartyType() const
{
	eVideoPartyType eResVideoPartyType = eVideo_party_type_none;
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	if (pCapStruct)
	{
		if ((pCapStruct->cifMPI != (APIS8)-1) || (pCapStruct->qcifMPI != (APIS8)-1))
		{
			if( systemCardsBasedMode == eSystemCardsMode_breeze  )
				eResVideoPartyType = GetH261H263ResourcesPartyType(NO);
			else
			{
				BOOL bH261Support = GetSystemCfgFlagInt<BOOL>("H261_SUPPORT_ALWAYS");
				if( bH261Support && systemCardsBasedMode == eSystemCardsMode_mpmrx)
					eResVideoPartyType = eCP_H261_CIF_equals_H264_HD1080_video_party_type;
				else
					eResVideoPartyType = GetH261H263ResourcesPartyType(NO);
			}

		}
		else
		{
			DBGPASSERT(1);
			PTRACE(eLevelError, "CH261VideoCap::GetCPVideoPartyType() H261 cap with no CIF nor QCIF format");
		}
	}

	return eResVideoPartyType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH261VideoCap::IsSupportPLI() const
{
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_PLI);

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH261VideoCap::IsSupportFIR() const
{
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_FIR);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH261VideoCap::IsSupportTMMBR() const
{
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_TMMBR);

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH261VideoCap::IsSupportNonStandardEncode() const
{
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_IS_NOT_STANDARD_ENCODE);
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH261VideoCap::GetRtcpFeedbackMask() const
{
	APIS32 rtcpFeedbackMask = 0;
	h261CapStruct* pCapStruct = CAP_CAST(h261CapStruct);
	if (pCapStruct)
		rtcpFeedbackMask = pCapStruct->rtcpFeedbackMask;
	return rtcpFeedbackMask;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH261VideoCap::SetRtcpFeedbackMask(APIS32 rtcpFbMask)
{
	EResult eRes = kFailure;
	h261CapStruct *pCapStruct = CAP_CAST(h261CapStruct);

	if (pCapStruct)
	{
		pCapStruct->rtcpFeedbackMask = rtcpFbMask;
		eRes = kSuccess;
	}
	return eRes;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Copies the base structure without annexes and cusom format EXCEPT from the header and the name.
//             the structures should be allocated with the same size!!!
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::CopyBaseQualities(const CBaseVideoCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.GetStruct())
	{
		eRes = kSuccess;
		int sizeBaseWithAnnex = sizeof(h263CapStructBase) - sizeof(annexes_fd_set) - sizeof(xmlDynamicProperties) - sizeof(APIU16); //APIU16 capBoolMask
		int sizeWithoutBaseSize = sizeBaseWithAnnex - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.GetStruct()) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, sizeWithoutBaseSize);
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Calculates the structure size according to the mask
//---------------------------------------------------------------------------------------------------

size_t CH263VideoCap::SizeOf() const
{
	size_t structSize = 0;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		// if we have annexes or custom formats
		if (IsH263Plus())
		{
			int  annexes         = GetNumOfAnnexes();
			int  customFormats   = GetNumOfCustomFormats(); //from mask

			structSize += sizeof(h263CapStructBase);

			structSize += (annexes * sizeof(h263OptionsStruct));

			if (customFormats)
			{
				structSize += sizeof(customPic_StBase);
				structSize += (customFormats * sizeof(customPicFormatSt));
			}
		}
		else
			structSize += sizeof(h263CapStruct);
	}
	return structSize;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CH263VideoCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(h263CapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CH263VideoCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(h263CapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(h263CapStruct));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH263VideoCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	h263CapStruct *pOtherCapStruct = (h263CapStruct*)otherCap.GetStruct();
	CH263VideoCap &hOtherVideoCap  = (CH263VideoCap&)otherCap;

	if (GetCapCode() == hOtherVideoCap.GetCapCode())
	{
		eRes = kSuccess;
		pCapStruct->maxBitRate	= hOtherVideoCap.GetBitRate();
		pCapStruct->qcifMPI		= hOtherVideoCap.GetFormatMpi(kQCif);
		pCapStruct->cifMPI		= hOtherVideoCap.GetFormatMpi(kCif);
		pCapStruct->cif4MPI		= hOtherVideoCap.GetFormatMpi(k4Cif);
		pCapStruct->cif16MPI	= hOtherVideoCap.GetFormatMpi(k16Cif);
		pCapStruct->rtcpFeedbackMask  = hOtherVideoCap.GetRtcpFeedbackMask();

		// to be improve laterr
		pCapStruct->hrd_B 			= pOtherCapStruct->hrd_B;
		pCapStruct->bppMaxKb 		= pOtherCapStruct->bppMaxKb;
		pCapStruct->slowSqcifMPI 	= pOtherCapStruct->slowSqcifMPI;
		pCapStruct->slowQcifMPI 	= pOtherCapStruct->slowQcifMPI;
		pCapStruct->slowCifMPI 		= pOtherCapStruct->slowCifMPI;
		pCapStruct->slowCif4MPI 	= pOtherCapStruct->slowCif4MPI;
		pCapStruct->slowCif16MPI 	= pOtherCapStruct->slowCif16MPI;
//		pCapStruct->sqcifMPI 		= pOtherCapStruct->sqcifMPI;

		pCapStruct->capBoolMask 	= pOtherCapStruct->capBoolMask;
		pCapStruct->annexesMask 	= pOtherCapStruct->annexesMask;
		pCapStruct->xmlDynamicProps = pOtherCapStruct->xmlDynamicProps;

		if(IsH263Plus())//IsAnnexes() || IsCustomFormats())
		{// copy the annexes
//			pCapStruct = ReallocIfNeeded();
			memcpy(pCapStruct->annexesPtr, pOtherCapStruct->annexesPtr, SizeOf() - sizeof(h263CapStructBase));
		}
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if there are annexes in the struct
//---------------------------------------------------------------------------------------------------
BYTE CH263VideoCap::IsAnnexes() const
{
	BYTE bRes = FALSE;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct && pCapStruct->annexesMask.fds_bits[0])
	{
		annexes_fd_set tempAnnexesMask = pCapStruct->annexesMask;

		//Clears the custom formats bits
		for (int i = H263_Annexes_Number ;i < H263_Annexes_Number + H263_Custom_Number; i++)
			CAP_FD_CLR(i, &tempAnnexesMask);

		bRes = (tempAnnexesMask.fds_bits[0] != 0);
	}
	return bRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if there are custom formats in the struct
//---------------------------------------------------------------------------------------------------
BYTE CH263VideoCap::IsCustomFormats() const
{
	BYTE bRes = FALSE;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		long mask = pCapStruct->annexesMask.fds_bits[0];
		mask &= CUSTOM_FORMATS_ON_MASK;
		bRes = (mask != 0);
	}
	return bRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the number of annexes in the struct
//---------------------------------------------------------------------------------------------------
int CH263VideoCap::GetNumOfAnnexes()const
{
	int annexes = 0;

    if (IsH263Plus() && IsAnnexes())
	{
		for (annexesListEn i = typeAnnexB; i < H263_Annexes_Number - 1; i++)
		{//we do H263_Annexes_Number - 1, because be don't set typeAnnexI_NS, so we don't take it into acount.
		    //PTRACE(eLevelInfoNormal,"CH263VideoCap::GetNumOfAnnexes in loop");
			if (IsAnnex(i))
			{
			    //PTRACE(eLevelInfoNormal,"CH263VideoCap::GetNumOfAnnexes after calling IsAnnex");
				annexes++;
			}
		}
	}
	return annexes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the annex Enum value according to an indexnumber of annexes in the struct
//---------------------------------------------------------------------------------------------------
annexesListEn CH263VideoCap::GetAnnexEnAccordingToIndex(int i)const
{
	int annexes = 0;

	if (IsH263Plus() && IsAnnexes())
	{
		for (annexesListEn aEn = typeAnnexB; aEn < H263_Annexes_Number - 1; aEn++)
		{//we do H263_Annexes_Number - 1, because be don't set typeAnnexI_NS, so we don't take it into acount.
			if (IsAnnex(aEn))
			{
				if(annexes == i)
					return aEn;
				annexes++;
			}
		}
	}
	return typeCustomPic;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the number of custom formats in the struct (from mask)
//             The number is in the 22-31 bits of the mask:
//             * 1000... means 1
//             * 1100... means 2
//             * 1110... means 3
//             * 1111... means 4
//             And so on.
//---------------------------------------------------------------------------------------------------
int CH263VideoCap::GetNumOfCustomFormats() const
{
	int counter = 0;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	if (pCapStruct)
	{
		for (int i=0;i<H263_Custom_Number;i++)
		{
			if (CAP_FD_ISSET(H263_Annexes_Number+i, &(pCapStruct->annexesMask)))
				counter++;
		}
	}
	return counter;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH263VideoCap::IsSupportPLI() const
{
    h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
    if (pCapStruct)
        return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_PLI);
    return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH263VideoCap::IsSupportFIR() const
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_FIR);
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH263VideoCap::IsSupportTMMBR() const
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_TMMBR);

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH263VideoCap::IsSupportNonStandardEncode() const
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_IS_NOT_STANDARD_ENCODE);
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH263VideoCap::GetRtcpFeedbackMask() const
{
	APIS32 rtcpFeedbackMask = 0;
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	if (pCapStruct)
		rtcpFeedbackMask = pCapStruct->rtcpFeedbackMask;
	return rtcpFeedbackMask;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH263VideoCap::SetRtcpFeedbackMask(APIS32 rtcpFbMask)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		pCapStruct->rtcpFeedbackMask = rtcpFbMask;
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the bit rate.
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::SetBitRate(APIS32 rate)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	if (pCapStruct)
	{
		pCapStruct->maxBitRate = rate;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the mpi. Format can be k16Cif, k4Cif, kCif, kQCif or custom format
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::SetFormatMpi(EFormat eFormat, APIS8 mpi)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	BYTE bIsParameterOK = (eFormat < MAX_VIDEO_FORMATS);

	if (pCapStruct && bIsParameterOK)
	{
		// Regular resolutions
		if (eFormat <= k16Cif)
		{
			APIS8 *pStructMpi = &(pCapStruct->cif16MPI);
			for (int i = k16Cif; i >= kQCif; i--)
			{
				if (eFormat == i)
				{
					*pStructMpi = mpi;
					eRes = kSuccess;
					break;
				}
				pStructMpi--;
			}
		}

		// Custom formats
		else if (IsCustomFormats())
		{
			customPic_St *pH263CustomFormats = GetAPointerToCustomFormats();
			if (pH263CustomFormats)
			{
				customPicFormatSt *pCustomFormat = GetAPointerToASpecificCustomFormat(eFormat);
				if (pCustomFormat)
				{
					pCustomFormat->standardMPI = mpi;
					eRes = kSuccess;
				}

			}
		}
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets sqcif mpi to -1
//---------------------------------------------------------------------------------------------------
void	CH263VideoCap::NullifySqcifMpi()
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	pCapStruct->sqcifMPI = -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: return true if the cap has only sqcif mpi, if it has different then sqcif it return true.
//---------------------------------------------------------------------------------------------------
BYTE	CH263VideoCap::IsOnlySqcifMpi() const
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if(pCapStruct->sqcifMPI == -1)
		return FALSE;
	else
	{
		APIS8 *pStructMpi = &(pCapStruct->cif16MPI);
		for (int i = k16Cif; i >= kQCif; i--)
		{
			if(*pStructMpi > 0)
				return FALSE;
			pStructMpi--;
		}

		return TRUE; //if get out from the loop it means that the cap has only sqcif
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the default h263 values
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		pCapStruct->sqcifMPI						   = -1;
		pCapStruct->qcifMPI							   = -1;
		pCapStruct->cifMPI							   = -1;
		pCapStruct->cif4MPI							   = -1;
		pCapStruct->cif16MPI						   = -1;
		pCapStruct->hrd_B							   = 0;
		pCapStruct->bppMaxKb						   = 0;
		pCapStruct->slowSqcifMPI					   = -1;
		pCapStruct->slowQcifMPI						   = -1;
		pCapStruct->slowCifMPI						   = -1;
		pCapStruct->slowCif4MPI						   = -1;
		pCapStruct->slowCif16MPI					   = -1;
		pCapStruct->capBoolMask						   = 0;
		pCapStruct->annexesMask.fds_bits[0]			   = 0; // sets the annexes mask to zero
		pCapStruct->xmlDynamicProps.numberOfDynamicParts  = 0;
		pCapStruct->xmlDynamicProps.sizeOfAllDynamicParts = 0;
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);
		pCapStruct->rtcpFeedbackMask                    = (RTCP_MASK_FIR | RTCP_MASK_TMMBR | RTCP_MASK_PLI );
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets a content struct. For now we support the following hard coded values.
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::SetContent(ERoleLabel eRole, cmCapDirection eDirection,BOOL isHD1080, BYTE HDMpi, BOOL isHighProfile)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
    PTRACE(eLevelInfoNormal,"CH263VideoCap::SetContent");
	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= SetDefaults(eDirection, eRole);
		eRes &= SetFormatMpi(kQCif,1);
		eRes &= SetFormatMpi(kCif,1);
//		PASSERTMSG(1, "CH263VideoCap::SetContent - Yoellas` PATCH to Remove 263+  !!!!!!");

		eRes &= SetFormatMpi(k4Cif,2);

		eRes &= SetNumOfCustomFormatsInMask(3);
//		eRes &= SetAnAnnexInMask(typeAnnexI); // decision from 10.12.02
		BOOL bIsAnnexT = GetSystemCfgFlagInt<BOOL>(CFG_KEY_H263_ANNEX_T) ;//&& !(bContentAsVideo);
		if (bIsAnnexT)
		eRes &= SetAnAnnexInMask(typeAnnexT);
		pCapStruct = ReallocIfNeeded();
		if (pCapStruct) //allocation succeed
		{
			eRes &= SetAnnexesStructs();
			eRes &= SetCustomFormatsDefaults();
			APIU8 pixelAspectCode[] = {1};
			eRes &= SetASpecificCustomFormatStruct(0,kVGA ,2,pixelAspectCode);
			eRes &= SetASpecificCustomFormatStruct(1,kSVGA,3,pixelAspectCode);
			eRes &= SetASpecificCustomFormatStruct(2,kXGA ,4,pixelAspectCode);
		}
	}

	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if annex is set
//---------------------------------------------------------------------------------------------------
BYTE CH263VideoCap::IsAnnex(annexesListEn eAnnex) const
{
	BYTE bRes = FALSE;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	BYTE bIsParameterOK = (eAnnex < H263_Annexes_Number);

	if (pCapStruct && bIsParameterOK)
	{
		if (CAP_FD_ISSET(eAnnex, &(pCapStruct->annexesMask)))
			bRes = TRUE;

	}

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if custom format is set. Checks if the custom format is set in the array,
//             meaning that it's index is between 0 to 3.
//---------------------------------------------------------------------------------------------------
BYTE CH263VideoCap::IsCustomFormat(EFormat eFormat) const
{
	BYTE bRes = FALSE;
	customPicFormatSt *pCustomFormat = GetAPointerToASpecificCustomFormat(eFormat);
	bRes = (pCustomFormat != NULL);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
//Description
//Return the size of customs
/////////////////////////////////////////////////////////////////////////////
int CH263VideoCap::GetSizeOfCustoms(char *pStruct)
{
	customPic_St    *pCustomFormats = (customPic_St*)pStruct;
	int				size			= sizeof(customPic_StBase);
	int	numberOfCustomPic			= pCustomFormats->numberOfCustomPic;

	customPicFormatSt	*pCurrentCustomFormat = (customPicFormatSt *)pCustomFormats->customPicPtr;
	char				*pChar = (char *)pCustomFormats->customPicPtr;

	for(int j=0; j < numberOfCustomPic; j++)
	{
		if(pCurrentCustomFormat)
			size += sizeof(customPicFormatSt);

		pChar += sizeof(customPicFormatSt);
		pCurrentCustomFormat = (customPicFormatSt *)pChar;
	}
	return size;
}
/////////////////////////////////////////////////////////////////////////////
//Description
//Return the size of annexes
/////////////////////////////////////////////////////////////////////////////
int CH263VideoCap::GetSizeOfAnnexes(char *pStruct, BYTE **ppAnnexPtr)
{
	h263CapStruct	*pCapStruct = (h263CapStruct*)pStruct;
	BYTE			*pAnnexPtr		= (BYTE *)pCapStruct->annexesPtr;
	int				size			= 0;

	for (annexesListEn annex = typeAnnexB; annex < H263_Annexes_Number; annex++)
	{
		if (CAP_FD_ISSET(annex, &(pCapStruct->annexesMask)))
		{
			size += sizeof(h263OptionsStruct);
			pAnnexPtr += sizeof(h263OptionsStruct);
		}
	}

	*ppAnnexPtr						= pAnnexPtr;

	return size;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets an annex in the mask
//---------------------------------------------------------------------------------------------------

EResult CH263VideoCap::AddAnnexToCap(annexesListEn eAnnex)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	BYTE bIsParameterOK = (eAnnex < H263_Annexes_Number);
	annexes_fd_set newMask;
	annexes_fd_set oldMask;

	if (pCapStruct && bIsParameterOK)
	{
		BYTE *pAnnexPtr	  = (BYTE *)pCapStruct->annexesPtr;
		BYTE *pStartAnnex = (BYTE *)pCapStruct->annexesPtr;
		BYTE *pEndAnnex   = (BYTE *)pCapStruct->annexesPtr;
		oldMask.fds_bits[0] = pCapStruct->annexesMask.fds_bits[0];
		newMask.fds_bits[0] = 0;
		int annexIndex		= 0;

		int annexesSize = GetSizeOfAnnexes((char *)pCapStruct, &pEndAnnex);
		annexesSize += sizeof(h263OptionsStruct); //For the new annex.

		int customSize = 0;
		if(pCapStruct->annexesMask.fds_bits[0] & CUSTOM_FORMATS_ON_MASK)
			customSize = GetSizeOfCustoms((char *)pEndAnnex);

		BYTE *pNewAnnexPtr = new BYTE[annexesSize + customSize];
		
		memset(pNewAnnexPtr, 0, annexesSize + customSize);

		int index = 0;
		annexesListEn i;
		for (i = typeAnnexB; i < H263_Annexes_Number - 1; i++)
		{
			if(i == eAnnex)
			{
				h263OptionsStruct tempAnnex;
				memset(&tempAnnex,0,sizeof(h263OptionsStruct));
				CAP_FD_SET(i,&newMask);
				memcpy(&pNewAnnexPtr[index],&tempAnnex,sizeof(h263OptionsStruct));
				index += sizeof(h263OptionsStruct);
			}
			else if (IsAnnex(i))
			{
				CAP_FD_SET(i,&newMask);
				memcpy(&pNewAnnexPtr[index],pAnnexPtr,sizeof(h263OptionsStruct));
				pAnnexPtr += sizeof(h263OptionsStruct);
				index += sizeof(h263OptionsStruct);
			}
		}

		if(pCapStruct->annexesMask.fds_bits[0] & CUSTOM_FORMATS_ON_MASK)
		{
			memcpy(&pNewAnnexPtr[index],pAnnexPtr,sizeof(customPic_StBase));
			pAnnexPtr += sizeof(customPic_StBase);
			index += sizeof(customPic_StBase);

			i++; //move over to number 23 (we shouldn't check annex_INS - he does not have body)
			for(;i < H263_Annexes_Number + H263_Custom_Number;  i++)
			{
				if(CAP_FD_ISSET(i, &oldMask))
				{
					CAP_FD_SET(i,&newMask);
					memcpy(&pNewAnnexPtr[index],pAnnexPtr,sizeof(customPicFormatSt));
					pAnnexPtr += sizeof(customPicFormatSt);
					index += sizeof(customPicFormatSt);
				}
			}
		}

		pCapStruct->annexesMask.fds_bits[0] = newMask.fds_bits[0];
		memcpy(pCapStruct->annexesPtr,pNewAnnexPtr,index);
		delete [] pNewAnnexPtr;

	}
	return eRes;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets an annex in the mask
//---------------------------------------------------------------------------------------------------

EResult CH263VideoCap::SetAnAnnexInMask(annexesListEn eAnnex)
{
	EResult eRes = kFailure;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bIsAnnexSupported = FALSE;
	std::string key = "";
	if(eAnnex == typeAnnexT)
		key = "H263_ANNEX_T";
	else if(eAnnex == typeAnnexW)
		key = "H263_ANNEX_W";
	else if(eAnnex == typeAnnexN)
		key = "H263_ANNEX_N";
	else if(eAnnex == typeAnnexI)
		key = "H263_ANNEX_I";
	else if(eAnnex == typeAnnexF)
		key = "H263_ANNEX_F";
	else if(eAnnex == typeAnnexI_NS)
		key = "H263_ANNEX_I_NS";

	pSysConfig->GetBOOLDataByKey(key, bIsAnnexSupported);

	if(bIsAnnexSupported)
	{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	BYTE bIsParameterOK = (eAnnex < H263_Annexes_Number);

	if (pCapStruct && bIsParameterOK)
	{
		CAP_FD_SET(eAnnex, &(pCapStruct->annexesMask));
		eRes = kSuccess;
	}
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH263VideoCap::RemoveAnAnnexFromMask(annexesListEn eAnnex)
{
	EResult eRes = kFailure;
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	BYTE bIsParameterOK = (eAnnex < H263_Annexes_Number);

	if (pCapStruct && bIsParameterOK)
	{
		CAP_FD_CLR(eAnnex, &(pCapStruct->annexesMask));
		eRes = kSuccess;
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// This is a problem just to remove the annex from mask without remove its structure and do reallocate.
// We can do it only if this is the only H263+ element in the structure.
EResult CH263VideoCap::RemoveTheSingleAnnexFromMask(annexesListEn eAnnex)
{
	EResult eRes = kFailure;
	if (!IsCustomFormats() && (GetNumOfAnnexes() == 1) && IsAnnex(eAnnex) )
		RemoveAnAnnexFromMask(eAnnex);
	else
	{
		DBGPASSERT(eAnnex);
		PTRACE(eLevelError, "CH263VideoCap::RemoveTheSingleAnnexFromMask: This is not the only annex in mask.");
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the number of custom formats in the mask.
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::SetNumOfCustomFormatsInMask(DWORD num)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	BYTE bIsParameterOK = (num <= H263_Custom_Number);

	if (pCapStruct && bIsParameterOK)
	{
		// zeroing the bits (22-31) that indicate of the custom formats number
		pCapStruct->annexesMask.fds_bits[0] &= CUSTOM_FORMATS_OFF_MASK;

		DWORD startIndex = H263_Annexes_Number;

		for (DWORD maskIndex = startIndex; maskIndex < startIndex+num; maskIndex++)
			CAP_FD_SET(maskIndex, &(pCapStruct->annexesMask));

		eRes = kSuccess;
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Reallocates a new structure if h263+ (bigger size) or in case to reduce them (smaller)
//             Returns the new structure. Don't forget to set it instead of the old one
//---------------------------------------------------------------------------------------------------
h263CapStruct* CH263VideoCap::ReallocIfNeeded(BYTE reallocAnyWay)
{
	h263CapStruct *pResCapStruct = NULL;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		if ((IsH263Plus()) || (reallocAnyWay == TRUE))
		{
			size_t newSize = SizeOf();
			BYTE *pNewStruct = new BYTE[newSize];
			memset(pNewStruct,0,newSize);
			memcpy(pNewStruct,(BYTE*)pCapStruct,sizeof(h263CapStruct));
			FreeStruct();
			m_pCapStruct = (BaseCapStruct *)pNewStruct;
		}
		pResCapStruct = (h263CapStruct *)m_pCapStruct;
	}

	return pResCapStruct;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the annexes structures
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::SetAnnexesStructs()
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;

		int annexIndex  = 0;
		for (annexesListEn i = typeAnnexB; i < H263_Annexes_Number - 1; i++)
		{//we do H263_Annexes_Number - 1, because be don't set typeAnnexI_NS
			if (IsAnnex(i))
			{
				eRes &= SetAnAnnexStruct(i, annexIndex);
				annexIndex++;
			}
		}
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int CH263VideoCap::GetAnnexIndex(annexesListEn annex) const
{
	int annexIndex = -1;
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		for (annexesListEn i = typeAnnexB; i <= annex; i++)
		{
			if (IsAnnex(i))
				annexIndex++;
		}
	}
	return annexIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets a pointer to a specific annex struct.
//---------------------------------------------------------------------------------------------------
h263OptionsStruct* CH263VideoCap::GetAPointerToASpecificAnnex(int annexIndex) const
{
	h263OptionsStruct *pH263OptionsSt = NULL;

	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	BYTE bIsParameterOK = (annexIndex < GetNumOfAnnexes());

	if (pCapStruct && bIsParameterOK)
	{
		char *pChar = pCapStruct->annexesPtr;
		pChar += annexIndex * sizeof(h263OptionsStruct);
		pH263OptionsSt = (h263OptionsStruct *)pChar;
	}

	return pH263OptionsSt;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets a pointer to the custom formats struct.
//---------------------------------------------------------------------------------------------------
customPic_St* CH263VideoCap::GetAPointerToCustomFormats() const
{
	customPic_St  *pCustomFormats = NULL;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct && IsCustomFormats())
	{
		int numOfAnnexes = GetNumOfAnnexes();
		char *pChar = pCapStruct->annexesPtr;
		pChar += numOfAnnexes * sizeof(h263OptionsStruct);
		pCustomFormats = (customPic_St *)pChar;
	}

	return pCustomFormats;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets a pointer to a specific custom format struct.
//---------------------------------------------------------------------------------------------------
customPicFormatSt* CH263VideoCap::GetAPointerToASpecificCustomFormat(int customFormatIndex) const
{
	customPicFormatSt *pSpecificCustomFormat = NULL;
	customPic_St      *pCustomFormats = GetAPointerToCustomFormats();
	if (pCustomFormats && (customFormatIndex < H263_Custom_Number))
	{
		char *pChar = (char *)pCustomFormats->customPicPtr;
		pChar += customFormatIndex * sizeof(customPicFormatSt);
		pSpecificCustomFormat = (customPicFormatSt *)pChar;
	}
	return pSpecificCustomFormat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets a pointer to a specific custom format struct according to given format.
//---------------------------------------------------------------------------------------------------
customPicFormatSt* CH263VideoCap::GetAPointerToASpecificCustomFormat(EFormat eFormat) const
{
	customPicFormatSt *pSpecificCustomFormat = NULL;
	customPic_St *pCustomFormats  = GetAPointerToCustomFormats();
	BYTE bIsParameterOK = ((eFormat < MAX_VIDEO_FORMATS)&&((eFormat >= kVGA) || (eFormat == kCif)));


	if (pCustomFormats && bIsParameterOK)
	{

		BOOL CustomOk = (pCustomFormats->numberOfCustomPic < H263_Custom_Number) ? true : false;
		PASSERTSTREAM_AND_RETURN_VALUE(!CustomOk, "pCustomFormats->numberOfCustomPic is too large - will cause endless loop", pSpecificCustomFormat); //Core -Bridge 15824 protection - fix in CS

		DWORD i;
		for ( i = 0; i < pCustomFormats->numberOfCustomPic; i++)
		{
			customPicFormatSt *pCurrentCustomFormat = GetAPointerToASpecificCustomFormat(i);
			if (pCurrentCustomFormat)
			{
				DWORD xWidth  = pCurrentCustomFormat->maxCustomPictureWidth *4;
				DWORD yHeight = pCurrentCustomFormat->maxCustomPictureHeight*4;
				EFormat eCurrentFormat = ::CalculateFormat(xWidth,yHeight);
				if (eFormat == eCurrentFormat)
				{
					pSpecificCustomFormat = pCurrentCustomFormat;
					break;
				}
			}
		}
	}
	return pSpecificCustomFormat;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets a single annex structure.
//---------------------------------------------------------------------------------------------------

EResult CH263VideoCap::SetAnAnnexStruct(annexesListEn eAnnex, int annexIndex)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		h263OptionsStruct *pOptionsSt = GetAPointerToASpecificAnnex(annexIndex);

		if (pOptionsSt)
		{
			switch(eAnnex)
			{
				// set Bool to zero
				// its a mistake to set the bool with XOR
				pCapStruct->capBoolMask								= 0; // common to all annexes
				pOptionsSt->annexD.annexBoolMask					= 0; // since its enum and it located in the same place for all the annexes we can combine the set of this field.
				case typeAnnexB:
					pCapStruct->hrd_B								= 1;
					break;
				case typeAnnexD:
					pCapStruct->capBoolMask							|= h263_unrestrictedVector;
					pOptionsSt->annexD.annexBoolMask				|= annexD_unlimitedMotionVectors;
					break;
				case typeAnnexE:
					pCapStruct->capBoolMask							|= h263_arithmeticCoding;
					break;
				case typeAnnexF:
					pCapStruct->capBoolMask							|= h263_advancedPrediction;
					break;
				case typeAnnexG:
					pCapStruct->capBoolMask							|= h263_pbFrames;
					break;
				case typeAnnexH:
					break;
				case typeAnnexI:
					pOptionsSt->annexI.annexBoolMask				|= annexI_advancedIntraCodingMode;
					break;
				case typeAnnexJ:
					pOptionsSt->annexJ.annexBoolMask				|= annexJ_deblockingFilterMode;
					break;
				case typeAnnexK:
					pOptionsSt->annexK.annexBoolMask				|= annexK_slicesInOrder_NonRect;
					pOptionsSt->annexK.annexBoolMask				|= annexK_slicesInOrder_Rect;
					pOptionsSt->annexK.annexBoolMask				|= annexK_slicesNoOrder_NonRect;
					pOptionsSt->annexK.annexBoolMask				|= annexK_slicesNoOrder_Rect;
					break;
				case typeAnnexL:
					pOptionsSt->annexL.annexBoolMask				|= annexL_fullPictureFreeze;
//					pOptionsSt->annexL.annexBoolMask				^= annexL_partialPictureFreezeAndRelease;		//FALSE
//					pOptionsSt->annexL.annexBoolMask				^= annexL_resizingPartPicFreezeAndRelease;		//FALSE
//					pOptionsSt->annexL.annexBoolMask				^= annexL_fullPictureSnapshot;					//FALSE
//					pOptionsSt->annexL.annexBoolMask				^= annexL_partialPictureSnapshot;				//FALSE
//					pOptionsSt->annexL.annexBoolMask				^= annexL_videoSegmentTagging;					//FALSE
//					pOptionsSt->annexL.annexBoolMask				^= annexL_progressiveRefinement;				//FALSE
					break;
				case typeAnnexM:
					pOptionsSt->annexM.annexBoolMask				|= annexM_improvedPBFramesMode;
					break;
				case typeAnnexN:
					// The following values were taken by PCTL Stinger capabilities.
					pCapStruct->capBoolMask		|= h263_errorCompensation; // indicating support for videoNotDecodedMbs
					pOptionsSt->annexN.refPictureSelection.additionalPictureMemory.sqcifAdditionalPictureMemory	 = 1;
					pOptionsSt->annexN.refPictureSelection.additionalPictureMemory.qcifAdditionalPictureMemory	 = 1;
					pOptionsSt->annexN.refPictureSelection.additionalPictureMemory.cifAdditionalPictureMemory	 = 1;
					pOptionsSt->annexN.refPictureSelection.additionalPictureMemory.cif4AdditionalPictureMemory	 = 1;
					pOptionsSt->annexN.refPictureSelection.additionalPictureMemory.cif16AdditionalPictureMemory	 = 1;
					pOptionsSt->annexN.refPictureSelection.additionalPictureMemory.bigCpfAdditionalPictureMemory = 0;
					pOptionsSt->annexN.refPictureSelection.annexBoolMask										 &= (~refPic_videoMux); //=0
					pOptionsSt->annexN.refPictureSelection.videoBackChannelSend									 = VBnone;
					pOptionsSt->annexN.refPictureSelection.mpuHorizMBs											 = 0;
					pOptionsSt->annexN.refPictureSelection.mpuVertMBs											 = 0;
					break;
				case typeAnnexO:
				    break;
				case typeAnnexP:
					pOptionsSt->annexP.annexBoolMask						|= annexP_dynamicPictureResizingByFour;
//					pOptionsSt->annexP.annexBoolMask						^= annexP_dynamicPictureResizingSixteenthPel;	//FALSE
//					pOptionsSt->annexP.annexBoolMask						^= annexP_dynamicWarpingHalfPel;				//FALSE
//					pOptionsSt->annexP.annexBoolMask						^= annexP_dynamicWarpingSixteenthPel;			//FALSE
					break;
				case typeAnnexQ:
					pOptionsSt->annexQ.annexBoolMask						|= annexQ_reducedResolutionUpdate;
					break;
				case typeAnnexR:
					pOptionsSt->annexR.annexBoolMask						|= annexR_independentSegmentDecoding;
					break;
				case typeAnnexS:
					pOptionsSt->annexS.annexBoolMask						|= annexS_alternateInterVLCMode;
					break;
				case typeAnnexT:
					pOptionsSt->annexT.annexBoolMask						|= annexT_modifiedQuantizationMode;
					break;
				case typeAnnexU:
					// The following values are garbage.
					pOptionsSt->annexU.refPictureSelection.additionalPictureMemory.sqcifAdditionalPictureMemory  = (APIU8)256;
					pOptionsSt->annexU.refPictureSelection.additionalPictureMemory.qcifAdditionalPictureMemory	 = (APIU8)256;
					pOptionsSt->annexU.refPictureSelection.additionalPictureMemory.cifAdditionalPictureMemory	 = (APIU8)256;
					pOptionsSt->annexU.refPictureSelection.additionalPictureMemory.cif4AdditionalPictureMemory	 = (APIU8)256;
					pOptionsSt->annexU.refPictureSelection.additionalPictureMemory.cif16AdditionalPictureMemory	 = (APIU8)256;
					pOptionsSt->annexU.refPictureSelection.additionalPictureMemory.bigCpfAdditionalPictureMemory = 0;
					pOptionsSt->annexU.refPictureSelection.annexBoolMask										 &= (~refPic_videoMux); //=0
					pOptionsSt->annexU.refPictureSelection.videoBackChannelSend									 = VBnone;
					pOptionsSt->annexU.refPictureSelection.mpuHorizMBs											 = 0;
					pOptionsSt->annexU.refPictureSelection.mpuVertMBs											 = 0;
					break;
				case typeAnnexV:
					break;
				case typeAnnexW:
//					pOptionsSt->annexW.annexBoolMask						^= annexW_dataPartitionedSlices;	//FALSE
//					pOptionsSt->annexW.annexBoolMask						^= annexW_fixedPointIDCT0;			//FALSE
					pOptionsSt->annexW.annexBoolMask						|= annexW_interlacedFields;
//					pOptionsSt->annexW.annexBoolMask						^= annexW_currentPictureHeaderRepetition;//FALSE
//					pOptionsSt->annexW.annexBoolMask						^= annexW_previousPictureHeaderRepetition;//FALSE
//					pOptionsSt->annexW.annexBoolMask						^= annexW_nextPictureHeaderRepetition;	//FALSE
//					pOptionsSt->annexW.annexBoolMask						^= annexW_pictureNumber;				//FALSE
//					pOptionsSt->annexW.annexBoolMask						^= annexW_spareReferencePictures;		//FALSE
					break;
				default:
					break;
			}
			eRes = kSuccess;
		}
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values of custom formats
//---------------------------------------------------------------------------------------------------

EResult CH263VideoCap::SetCustomFormatsDefaults()
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		customPic_St *pCustomFormats = GetAPointerToCustomFormats();
		if (pCustomFormats)
		{
			int counter = 0;
			eRes = kSuccess;
			int amount = GetNumOfCustomFormats(); // from mask
			for (int i = 0; i < amount; i++)
			{
				customPicFormatSt *pCustomPicFormatSt = GetAPointerToASpecificCustomFormat(i);
				if (pCustomPicFormatSt)
				{
					pCustomPicFormatSt->maxCustomPictureWidth	= 0;
					pCustomPicFormatSt->maxCustomPictureHeight	= 0;
					pCustomPicFormatSt->minCustomPictureWidth	= 0;
					pCustomPicFormatSt->minCustomPictureHeight	= 0;
					pCustomPicFormatSt->standardMPI				= -1;
					pCustomPicFormatSt->clockConversionCode		= 0;
					pCustomPicFormatSt->clockDivisor			= 0;
					pCustomPicFormatSt->customMPI				= 0;
					pCustomPicFormatSt->pixelAspectCode[0]		= 1;
					counter++;
				}
				else
					eRes = kFailure;

			}
			if (counter)
			{
				// The following values were taken by PCTL Stinger capabilities.
				pCustomFormats->customPictureClockFrequency.clockConversionCode	= -1;
				pCustomFormats->customPictureClockFrequency.clockDivisor		= -1;
				pCustomFormats->customPictureClockFrequency.sqcifMPI			= -1;
				pCustomFormats->customPictureClockFrequency.qcifMPI				= -1;
				pCustomFormats->customPictureClockFrequency.cifMPI				= -1;
				pCustomFormats->customPictureClockFrequency.cif4MPI				= -1;
				pCustomFormats->customPictureClockFrequency.cif16MPI			= -1;
			}

			pCustomFormats->numberOfCustomPic = counter;
		}
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the amount of custom formats
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::SetCustomFormatsAmount(int amount)
{
	EResult eRes = kFailure;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		customPic_St *pCustomFormats = GetAPointerToCustomFormats();
		if (pCustomFormats)
		{
			pCustomFormats->numberOfCustomPic = amount;
			eRes = kSuccess;
		}
	}
	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets a specific custom format struct with the given values.
//---------------------------------------------------------------------------------------------------

EResult CH263VideoCap::SetASpecificCustomFormatStruct(int customFormatIndex,
										              APIU16 maxCustomPictureWidth,APIU16 maxCustomPictureHeight,
										              APIU16 minCustomPictureWidth,APIU16 minCustomPictureHeight,
													  APIU8 standardMPI,
													  APIU8 pixelAspectCode[],int length,
													  APIU16 clockConversionCode,
													  APIU8 clockDivisor,
													  APIU16 customMPI)
{
	EResult eRes = kFailure;
	customPicFormatSt *pCustomFormat = GetAPointerToASpecificCustomFormat(customFormatIndex);
	if (pCustomFormat)
	{
		eRes = kSuccess;
		pCustomFormat->maxCustomPictureWidth	= maxCustomPictureWidth;
		pCustomFormat->maxCustomPictureHeight	= maxCustomPictureHeight;
		pCustomFormat->minCustomPictureWidth	= minCustomPictureWidth;
		pCustomFormat->minCustomPictureHeight	= minCustomPictureHeight;
		pCustomFormat->standardMPI				= standardMPI;
		pCustomFormat->clockConversionCode = clockConversionCode;
		pCustomFormat->clockDivisor        = clockDivisor;
		pCustomFormat->customMPI           = customMPI;
		memcpy(pCustomFormat->pixelAspectCode,pixelAspectCode,length);
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets a specific custom format struct with the difault values.
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::SetASpecificCustomFormatStruct(int customFormatIndex,
									                  EFormat eFormat,
													  APIU8 standardMPI,
													  APIU8 pixelAspectCode[],int length,
													  APIU16 clockConversionCode,
													  APIU8 clockDivisor,
													  APIU16 customMPI)
{
	EResult eRes = kSuccess;
	eRes &= SetASpecificCustomFormatStruct(customFormatIndex,
										   g_formatDimensions[eFormat][WIDTH]/4,g_formatDimensions[eFormat][HEIGHT]/4,
										   g_formatDimensions[eFormat][WIDTH]/4,g_formatDimensions[eFormat][HEIGHT]/4,
										   standardMPI,pixelAspectCode,length,clockConversionCode,clockDivisor,customMPI);
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets a specific custom format dimensions with the difault values.
//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::SetASpecificCustomFormatDimensions(int customFormatIndex,EFormat eFormat)
{
	EResult eRes = kFailure;
	customPicFormatSt *pCustomFormat = GetAPointerToASpecificCustomFormat(customFormatIndex);
	if (pCustomFormat)
	{
		pCustomFormat->maxCustomPictureWidth   = g_formatDimensions[eFormat][WIDTH] /4;
		pCustomFormat->maxCustomPictureHeight  = g_formatDimensions[eFormat][HEIGHT]/4;
		pCustomFormat->minCustomPictureWidth   = g_formatDimensions[eFormat][WIDTH] /4;
		pCustomFormat->minCustomPictureHeight  = g_formatDimensions[eFormat][HEIGHT]/4;
	}

	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if the struct has h263 plus (annexes or custom formats)
//---------------------------------------------------------------------------------------------------
BYTE CH263VideoCap::IsH263Plus(BYTE isWithoutAnnexF) const
{
	BYTE bRes = FALSE;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		if(isWithoutAnnexF == FALSE)
			bRes = (pCapStruct->annexesMask.fds_bits[0] != 0);
		else
		{
			bRes = (pCapStruct->annexesMask.fds_bits[0] & WITHOUT_ANNEXF_MASK) ? 1 : 0;
//			bRes = (pCapStruct->annexesMask.fds_bits[0] & WITH_ANNEXF_MASK) ? 0 : 1  ;
		}
	}

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if the struct supports error compensation (the annex N flag).
//---------------------------------------------------------------------------------------------------
WORD CH263VideoCap::IsErrorCompensation() const
{
	WORD bRes = FALSE;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
		bRes = (pCapStruct->capBoolMask & h263_errorCompensation);

	return bRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the bit rate
//---------------------------------------------------------------------------------------------------
APIS32 CH263VideoCap::GetBitRate() const
{
	APIS32 bitRate = -1;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
		bitRate = pCapStruct->maxBitRate;

	return bitRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH263VideoCap::AddLowerResolutionsIfNeeded()
{
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	if (pCapStruct)
	{
		APIS8* pStructMpi = &(pCapStruct->cif16MPI);
		for (EFormat index = k16Cif; index >= kQCif; index--)
		{
			if (*pStructMpi > 0)
			{
				APIS8 mpiOfHigherResolution = *pStructMpi;
				for (EFormat i = index--; i >= kQCif; i--)
				{
					if (*pStructMpi == -1)
						*pStructMpi = mpiOfHigherResolution;
					pStructMpi--;
				}
				break;
			}
			else
				pStructMpi--;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the highest video format
//---------------------------------------------------------------------------------------------------
EFormat CH263VideoCap::GetFormat() const
{
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		EFormat i;
		// Custom formats
		if (IsCustomFormats())
		{
			for (i = kSIF; i >= kVGA; i--)
				if (IsFormat(i))
					return i;
		}

		//standard resolutions:
		APIS8 *pStructMpi = &(pCapStruct->cif16MPI);

		for (i = k16Cif; i >= kQCif; i--)
		{
			if (*pStructMpi > 0)
				return i;
			pStructMpi--;
		}
	}

	return kUnknownFormat;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the mpi of a specific format.
//             Format can be k16Cif, k4Cif, kCif, kQCif or custom format
//---------------------------------------------------------------------------------------------------
APIS8 CH263VideoCap::GetFormatMpi(EFormat eFormat) const
{
	APIS8 mpi = -1;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	BYTE bIsParameterOK = (eFormat != kUnknownFormat);

	if (pCapStruct && bIsParameterOK)
	{
		// Regular resolutions
		if (eFormat <= k16Cif)
		{
			APIS8 *pStructMpi = &(pCapStruct->cif16MPI);
			for (int i = k16Cif; i >= kQCif; i--)
			{
				if (eFormat == i)
				{
					mpi = *pStructMpi;
					break;
				}
				pStructMpi--;
			}
		}
		// Custom formats
		else if (IsCustomFormats())
		{
			customPic_St *pH263CustomFormats = GetAPointerToCustomFormats();
			if (pH263CustomFormats)
			{
				customPicFormatSt *pCustomFormat = GetAPointerToASpecificCustomFormat(eFormat);
				if (pCustomFormat)
				{
					mpi = pCustomFormat->standardMPI;
				}
			}
		}
	}

	return mpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if the format is supported in the struct (if it's mpi > 0)
//---------------------------------------------------------------------------------------------------
BYTE CH263VideoCap::IsFormat(EFormat eFormat) const
{
	BYTE bRes = (GetFormatMpi(eFormat) > 0);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the highest custom format. kSIF or kXGA or kSVGA or kNTSC or kVGA or cif (if it is PAL)
//---------------------------------------------------------------------------------------------------
EFormat CH263VideoCap::GetCustomFormat() const
{
	EFormat eRes = kUnknownFormat;
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		if (IsCustomFormats())
		{
			if(IsCustomFormat(kCif))
				return kCif;		//for PAL the custom format is cif

			// starts from xga
			for (EFormat i = kSIF; i >= kVGA; i--)
			{
				if (IsCustomFormat(i))
				{
					eRes = i;
					break;
				}
			}
		}
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Gets the mpi of the highest custom format
APIU8 CH263VideoCap::GetCustomFormatMpi() const
{
	h263CapStruct *pCapStruct		= CAP_CAST(h263CapStruct);
	APIU8			customFormatMpi	= 0;

	if (pCapStruct)
	{
		if (IsCustomFormats())
		{
			EFormat				customFormat	= GetCustomFormat();
			customPicFormatSt	*pCustomFormat	= GetAPointerToASpecificCustomFormat(customFormat);

			if(pCustomFormat != NULL)
			{
				if((customFormat == kCif) || (customFormat == kSIF))
					customFormatMpi = pCustomFormat->customMPI;
				else
					customFormatMpi	= pCustomFormat->standardMPI;
			}
		}
	}

	return customFormatMpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CH263VideoCap::GetCustomFormatMpi(EFormat	customFormat) const
{
	h263CapStruct *pCapStruct		= CAP_CAST(h263CapStruct);
	APIU8			customFormatMpi	= 0;

	if (pCapStruct)
	{
		if (IsCustomFormats())
		{
			customPicFormatSt *pCustomFormat = GetAPointerToASpecificCustomFormat(customFormat);

			if(pCustomFormat != NULL)
			{
				if((customFormat == kCif) || (customFormat == kSIF))
					customFormatMpi = pCustomFormat->customMPI;
				else
					customFormatMpi	= pCustomFormat->standardMPI;
			}
		}
	}

	return customFormatMpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CH263VideoCap::Dump(std::ostream& msg) const
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "sqcifMPI                = " << (int)pCapStruct->sqcifMPI;
		msg << "\nqcifMPI                 = " << (int)pCapStruct->qcifMPI;
		msg << "\ncifMPI                  = " << (int)pCapStruct->cifMPI;
		msg << "\ncif4MPI                 = " << (int)pCapStruct->cif4MPI;
		msg << "\ncif16MPI                = " << (int)pCapStruct->cif16MPI;
		msg << "\ntemTradeOffCapability   = " << (pCapStruct->capBoolMask & h263_temporalSpatialTradeOffCapability?1:0);
		msg << "\nrtcpFeedbackMask        = " << pCapStruct->rtcpFeedbackMask;

#ifdef UnimportantData
		msg << "\nbppMaxKb                = " << pCapStruct->bppMaxKb;
		msg << "\nslowSqcifMPI            = " << pCapStruct->slowSqcifMPI;
		msg << "\nslowQcifMPI             = " << pCapStruct->slowQcifMPI;
		msg << "\nslowCifMPI              = " << pCapStruct->slowCifMPI;
		msg << "\nslowCif4MPI             = " << pCapStruct->slowCif4MPI;
		msg << "\nslowCif16MPI            = " << pCapStruct->slowCif16MPI;
#endif
		if (pCapStruct->maxBitRate >= 0)
			msg << "\nmaxBitRate              = " << pCapStruct->maxBitRate <<
			" (" << (pCapStruct->maxBitRate)*100 << " bps)";
		else
			msg << "\nmaxBitRate              = Unknown";

		msg << "\n";

		if (IsH263Plus())
		{
			msg << "H263 Plus: ";
			if (IsAnnexes())
				DumpAnnexesDetailsToStream(msg);
			if (IsCustomFormats())
				DumpCustomFormatsToStream(msg);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the annexes and custom formats values into a stream
//---------------------------------------------------------------------------------------------------

void CH263VideoCap::DumpAnnexesDetailsToStream(std::ostream& msg) const
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		int annexIndex  = 0;
		for (annexesListEn i = typeAnnexB; i < H263_Annexes_Number; i++)
		{
			if (IsAnnex(i))
			{
				DumpAnAnnexDetailsToStream(i, annexIndex, msg);
				annexIndex++;
			}
		}
		msg << "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints only the annexes name into a stream
//---------------------------------------------------------------------------------------------------

void CH263VideoCap::DumpAnnexesNamesToStream(CObjString& msg, APIU32 hcAnnexes) const
{
	annexes_fd_set tempAnnexesMask;
	tempAnnexesMask.fds_bits[0] = hcAnnexes;

	for (annexesListEn eAnnex = typeAnnexB; eAnnex < H263_Annexes_Number; eAnnex++)
	{
		if (CAP_FD_ISSET(eAnnex, &tempAnnexesMask))
			::GetAnnexTypeName(eAnnex, msg);
			msg<<" was found.\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Get only the annexes name.
//---------------------------------------------------------------------------------------------------

annexesListEn CH263VideoCap::GetAnnex(APIU32 hcAnnexes) const
{
	annexes_fd_set tempAnnexesMask;
	tempAnnexesMask.fds_bits[0] = hcAnnexes;

	annexesListEn eAnnex;
	for (eAnnex = typeAnnexB; eAnnex < H263_Annexes_Number; eAnnex++)
	{
		if (CAP_FD_ISSET(eAnnex, &tempAnnexesMask))
			break;
	}

	return eAnnex;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets Annexes
//---------------------------------------------------------------------------------------------------
DWORD CH263VideoCap::GetAnnexes() const
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	DWORD annexes = 0;

	if (pCapStruct)
		annexes =  pCapStruct->annexesMask.fds_bits[0];

	return annexes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints a single annex's values into a stream
//---------------------------------------------------------------------------------------------------

void CH263VideoCap::DumpAnAnnexDetailsToStream(annexesListEn eAnnex, int annexIndex, std::ostream& msg) const
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		if (eAnnex == typeAnnexI_NS) //annex I_NS doesn't have a structure, so GetAPointerToASpecificAnnex won't bring it!
		{
			msg << " ............... AnnexI NS............" << "\n";
			return;
		}

		h263OptionsStruct *pH263OptionsSt = GetAPointerToASpecificAnnex(annexIndex);

		if (pH263OptionsSt)
		{
/*			if (CheckMsgSize(msg))
			{
				PTRACE(eLevelInfoNormal,msg.str());
				FlushMessage(msg);
			}
*/
			switch(eAnnex)
			{
			case	typeAnnexB:
				msg << " ............... Annex B ............." << "\n";
				msg << " hrd_B								= " << pCapStruct->hrd_B << "\n";
				break;
			case	typeAnnexD:
				msg << " ............... Annex D ............." << "\n";
				msg << " unrestrictedVector					= " <<  (pCapStruct->capBoolMask & h263_unrestrictedVector?1:0) << "\n";
				msg << " unlimitedMotionVectors				= " <<  (pH263OptionsSt->annexD.annexBoolMask & annexD_unlimitedMotionVectors?1:0) << "\n";
				break;
			case	typeAnnexE:
				msg << " ............... Annex E ............." << "\n";
				msg << " arithmeticCoding					= " <<  (pCapStruct->capBoolMask & h263_arithmeticCoding?1:0) << "\n";
				break;
			case	typeAnnexF:
				msg << " ............... Annex F ............." << "\n";
				msg << " advancedPrediction					= " <<  (pCapStruct->capBoolMask & h263_advancedPrediction?1:0) << "\n";
				break;
			case	typeAnnexG:
				msg << " ............... Annex G ............." << "\n";
				msg << " pbFrames							= " <<  (pCapStruct->capBoolMask & h263_pbFrames?1:0) << "\n";
				break;
			case	typeAnnexH:
				break;
			case	typeAnnexI:
				msg << " ............... Annex I ............." << "\n";
				msg << " advancedIntraCodingMode			= " <<  (pH263OptionsSt->annexI.annexBoolMask & annexI_advancedIntraCodingMode?1:0) << "\n";
				break;
			case	typeAnnexJ:
				msg << " ............... Annex J ............." << "\n";
				msg << " deblockingFilterMode				= " <<  (pH263OptionsSt->annexJ.annexBoolMask & annexJ_deblockingFilterMode?1:0) << "\n";
				break;
			case	typeAnnexK:
				msg << " ............... Annex K ............." << "\n";
				msg << " slicesInOrder_NonRect				= " <<  (pH263OptionsSt->annexK.annexBoolMask & annexK_slicesInOrder_NonRect?1:0) << "\n";
				msg << " slicesInOrder_Rect					= " <<  (pH263OptionsSt->annexK.annexBoolMask & annexK_slicesInOrder_Rect?1:0) << "\n";
				msg << " slicesNoOrder_NonRect				= " <<  (pH263OptionsSt->annexK.annexBoolMask & annexK_slicesNoOrder_NonRect?1:0) << "\n";
				msg << " slicesNoOrder_Rect					= " <<  (pH263OptionsSt->annexK.annexBoolMask & annexK_slicesNoOrder_Rect?1:0) << "\n";
				break;
			case	typeAnnexL:
				msg << " ............... Annex L ............." << "\n";
				msg << " fullPictureFreeze						= " <<  (pH263OptionsSt->annexL.annexBoolMask & annexL_fullPictureFreeze?1:0)<< "\n";
				msg << " partialPictureFreezeAndRelease		= " <<  (pH263OptionsSt->annexL.annexBoolMask & annexL_partialPictureFreezeAndRelease?1:0) << "\n";
				msg << " resizingPartPicFreezeAndRelease		= " <<  (pH263OptionsSt->annexL.annexBoolMask & annexL_resizingPartPicFreezeAndRelease?1:0) << "\n";
				msg << " fullPictureSnapshot				= " <<  (pH263OptionsSt->annexL.annexBoolMask & annexL_fullPictureSnapshot?1:0) << "\n";
				msg << " partialPictureSnapshot				= " <<  (pH263OptionsSt->annexL.annexBoolMask & annexL_partialPictureSnapshot?1:0) << "\n";
				msg << " videoSegmentTagging				= " <<  (pH263OptionsSt->annexL.annexBoolMask & annexL_videoSegmentTagging?1:0) << "\n";
				msg << " progressiveRefinement				= " <<  (pH263OptionsSt->annexL.annexBoolMask & annexL_progressiveRefinement?1:0) << "\n";
				break;
			case	typeAnnexM:
				msg << " ............... Annex M ............." << "\n";
				msg << " improvedPBFramesMode				= " <<  (pH263OptionsSt->annexM.annexBoolMask & annexM_improvedPBFramesMode?1:0) << "\n";
				break;
			case	typeAnnexN:
				msg << " ............... Annex N ............." << "\n";
				msg << " errorCompensation					= "<<  (pCapStruct->capBoolMask & h263_errorCompensation?1:0) << "\n";
				msg << " refPictureSelection.additionalPictureMemory: \n"
					<< " sqcifAdditionalPictureMemory		= "
					<<  (int)pH263OptionsSt->annexN.refPictureSelection.additionalPictureMemory.sqcifAdditionalPictureMemory << "\n";
				msg << " qcifAdditionalPictureMemory		= "
					<<  (int)pH263OptionsSt->annexN.refPictureSelection.additionalPictureMemory.qcifAdditionalPictureMemory << "\n";
				msg << " cifAdditionalPictureMemory			= "
					<<  (int)pH263OptionsSt->annexN.refPictureSelection.additionalPictureMemory.cifAdditionalPictureMemory << "\n";
				msg << " cif4AdditionalPictureMemory		= "
					<<  (int)pH263OptionsSt->annexN.refPictureSelection.additionalPictureMemory.cif4AdditionalPictureMemory << "\n";
				msg << " cif16AdditionalPictureMemory		= "
					<<  (int)pH263OptionsSt->annexN.refPictureSelection.additionalPictureMemory.cif16AdditionalPictureMemory << "\n";
				msg << " bigCpfAdditionalPictureMemory		= "
					<<  (int)pH263OptionsSt->annexN.refPictureSelection.additionalPictureMemory.bigCpfAdditionalPictureMemory << "\n";
				msg << " refPictureSelection: \n"
					<< " videoMux							= "
					<<  (pH263OptionsSt->annexN.refPictureSelection.annexBoolMask & refPic_videoMux?1:0) << "\n";
				msg << " videoBackChannelSend				= "
					<<  (int)pH263OptionsSt->annexN.refPictureSelection.videoBackChannelSend << "\n";
				msg << " mpuHorizMBs						= "
					<<  (int)pH263OptionsSt->annexN.refPictureSelection.mpuHorizMBs << "\n";
				msg << " mpuVertMBs							= "
					<<  (int)pH263OptionsSt->annexN.refPictureSelection.mpuVertMBs << "\n";
				break;
			case	typeAnnexO:
				break;
			case	typeAnnexP:
				msg << " ............... Annex P ............." << "\n";
				msg << " dynamicPictureResizingByFour		= " <<  (pH263OptionsSt->annexP.annexBoolMask & annexP_dynamicPictureResizingByFour?1:0) << "\n";
				msg << " dynamicPictureResizingSixteenthPel	= " <<  (pH263OptionsSt->annexP.annexBoolMask & annexP_dynamicPictureResizingSixteenthPel?1:0) << "\n";
				msg << " dynamicWarpingHalfPel				= " <<  (pH263OptionsSt->annexP.annexBoolMask & annexP_dynamicWarpingHalfPel?1:0) << "\n";
				msg << " dynamicWarpingSixteenthPel			= " <<  (pH263OptionsSt->annexP.annexBoolMask & annexP_dynamicWarpingSixteenthPel?1:0) << "\n";
				break;
			case	typeAnnexQ:
				msg << " ............... Annex Q ............." << "\n";
				msg << " reducedResolutionUpdate			= " <<  (pH263OptionsSt->annexQ.annexBoolMask & annexQ_reducedResolutionUpdate?1:0) << "\n";
				break;
			case	typeAnnexR:
				msg << " ............... Annex R ............." << "\n";
				msg << " independentSegmentDecoding			= " <<  (pH263OptionsSt->annexR.annexBoolMask & annexR_independentSegmentDecoding?1:0) << "\n";
				break;
			case	typeAnnexS:
				msg << " ............... Annex S ............." << "\n";
				msg << " alternateInterVLCMode				= " <<  (pH263OptionsSt->annexS.annexBoolMask & annexS_alternateInterVLCMode?1:0) << "\n";
				break;
			case	typeAnnexT:
				msg << " ............... Annex T ............." << "\n";
				msg << " modifiedQuantizationMode			= " <<  (pH263OptionsSt->annexT.annexBoolMask & annexT_modifiedQuantizationMode?1:0) << "\n";
				break;
			case	typeAnnexU:
				msg << " ............... Annex U ............." << "\n";
				msg << " refPictureSelection.additionalPictureMemory: \n"
					<< " sqcifAdditionalPictureMemory		= "
					<<  pH263OptionsSt->annexU.refPictureSelection.additionalPictureMemory.sqcifAdditionalPictureMemory << "\n";
				msg << " qcifAdditionalPictureMemory		= "
					<<  pH263OptionsSt->annexU.refPictureSelection.additionalPictureMemory.qcifAdditionalPictureMemory << "\n";
				msg << " cifAdditionalPictureMemory			= "
					<<  pH263OptionsSt->annexU.refPictureSelection.additionalPictureMemory.cifAdditionalPictureMemory << "\n";
				msg << " cif4AdditionalPictureMemory		= "
					<<  pH263OptionsSt->annexU.refPictureSelection.additionalPictureMemory.cif4AdditionalPictureMemory << "\n";
				msg << " cif16AdditionalPictureMemory		= "
					<<  pH263OptionsSt->annexU.refPictureSelection.additionalPictureMemory.cif16AdditionalPictureMemory << "\n";
				msg << " bigCpfAdditionalPictureMemory		= "
					<<  pH263OptionsSt->annexU.refPictureSelection.additionalPictureMemory.bigCpfAdditionalPictureMemory << "\n";
				msg << " refPictureSelection: \n"
					<< " videoMux							= "
					<<  (pH263OptionsSt->annexU.refPictureSelection.annexBoolMask & refPic_videoMux?1:0) << "\n";
				msg << " videoBackChannelSend				= "
					<<  pH263OptionsSt->annexU.refPictureSelection.videoBackChannelSend << "\n";
				msg << " mpuHorizMBs						= "
					<<  pH263OptionsSt->annexU.refPictureSelection.mpuHorizMBs << "\n";
				msg << " mpuVertMBs							= "
					<<  pH263OptionsSt->annexU.refPictureSelection.mpuVertMBs << "\n";
				break;
			case	typeAnnexV:
				msg << " ............... Annex V ............." << "\n";
				break;
			case	typeAnnexW:
				msg << " ............... Annex W ............." << "\n";
				msg << " dataPartitionedSlices				= " <<
						 (pH263OptionsSt->annexW.annexBoolMask & annexW_dataPartitionedSlices?1:0) << "\n";
				msg << " fixedPointIDCT0					= " <<
						 (pH263OptionsSt->annexW.annexBoolMask & annexW_fixedPointIDCT0?1:0) << "\n";
				msg << " interlacedFields					= " <<
						 (pH263OptionsSt->annexW.annexBoolMask & annexW_interlacedFields?1:0) << "\n";
				msg << " currentPictureHeaderRepetition		= " <<
						 (pH263OptionsSt->annexW.annexBoolMask & annexW_currentPictureHeaderRepetition?1:0) << "\n";
				msg << " previousPictureHeaderRepetition	= " <<
						 (pH263OptionsSt->annexW.annexBoolMask & annexW_previousPictureHeaderRepetition?1:0) << "\n";
				msg << " nextPictureHeaderRepetition		= " <<
						 (pH263OptionsSt->annexW.annexBoolMask & annexW_nextPictureHeaderRepetition?1:0) << "\n";
				msg << " pictureNumber						= " <<
						 (pH263OptionsSt->annexW.annexBoolMask & annexW_pictureNumber?1:0) << "\n";
				msg << " spareReferencePictures				= " <<
						 (pH263OptionsSt->annexW.annexBoolMask & annexW_spareReferencePictures?1:0) << "\n";
				break;
			case	typeAnnexI_NS:
				msg << " ............... typeAnnexI NS........" << "\n";
				break;
			default:
				char strAnnex[20];
				sprintf(strAnnex, "%d", eAnnex);
				msg << strAnnex << "\n";
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the custom formats values into a stream
//---------------------------------------------------------------------------------------------------

void CH263VideoCap::DumpCustomFormatsToStream(std::ostream& msg) const
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		customPic_St *pH263CustomFormats = GetAPointerToCustomFormats();

		if (pH263CustomFormats)
		{
			BYTE bAtLeastOne = FALSE;
			int  numOfCustomFormats = pH263CustomFormats->numberOfCustomPic;

			for (int i = 0; i < numOfCustomFormats; i++)
			{
				customPicFormatSt *pCurrentCustomFormat = GetAPointerToASpecificCustomFormat(i);

				if (pCurrentCustomFormat)
				{
					// The headline
					if (bAtLeastOne == FALSE)
					{
						msg << " ........... ";
						msg << numOfCustomFormats;
						msg << " Custom Picture Format .........." << "\n";

						bAtLeastOne = TRUE;
					}
/*
					if (CheckMsgSize(msg))
					{
						PTRACE(eLevelInfoNormal,msg.str());
						FlushMessage(msg);
					}
*/
					int xWidth  = pCurrentCustomFormat->maxCustomPictureWidth *4;
					int yHeight = pCurrentCustomFormat->maxCustomPictureHeight*4;
					EFormat eCurrentFormat = ::CalculateFormat(xWidth,yHeight);
					CCapSetInfo capInfo;
					msg << " Custom Picture Format Type: "<<capInfo.GetFormatStr(eCurrentFormat) <<  "\n";

					msg << " { "
						<<   pCurrentCustomFormat->maxCustomPictureWidth;
					msg << " , "
						<<   pCurrentCustomFormat->maxCustomPictureHeight;
					msg << " , "
						<<   pCurrentCustomFormat->minCustomPictureWidth;
					msg << " , "
						<<   pCurrentCustomFormat->minCustomPictureHeight;
					msg << " } " << "\n";


					msg << " standardMPI					= "
						<<   (int)pCurrentCustomFormat->standardMPI << "\n";
					msg << " clockConversionCode			= "
						<<   pCurrentCustomFormat->clockConversionCode << "\n";
					msg << " clockDivisor					= "
						<<   (int)pCurrentCustomFormat->clockDivisor << "\n";
					msg << " customMPI						= "
						<<   pCurrentCustomFormat->customMPI << "\n";
					msg << " pixelAspectCode[0]				= "
						<<   (int)pCurrentCustomFormat->pixelAspectCode[0] << "\n";
				}
			}

			if (bAtLeastOne)
			{
/*				if (CheckMsgSize(msg))
				{
					PTRACE(eLevelInfoNormal,msg.str());
					FlushMessage(msg);
				}
*/
				msg << " customPictureClockFrequency.clockConversionCode 	= "
					<<  (int)pH263CustomFormats->customPictureClockFrequency.clockConversionCode << "\n";
				msg << " customPictureClockFrequency.clockDivisor			= "
					<<  (int)pH263CustomFormats->customPictureClockFrequency.clockDivisor << "\n";
				msg << " customPictureClockFrequency.sqcifMPI				= "
					<<  pH263CustomFormats->customPictureClockFrequency.sqcifMPI << "\n";
				msg << " customPictureClockFrequency.qcifMPI				= "
					<<  pH263CustomFormats->customPictureClockFrequency.qcifMPI << "\n";
				msg << " customPictureClockFrequency.cifMPI					= "
					<<  pH263CustomFormats->customPictureClockFrequency.cifMPI << "\n";
				msg << " customPictureClockFrequency.cif4MPI				= "
					<<  pH263CustomFormats->customPictureClockFrequency.cif4MPI << "\n";
				msg << " customPictureClockFrequency.cif16MPI				= "
					<<  pH263CustomFormats->customPictureClockFrequency.cif16MPI << "\n";
			}
		}
	}
}
//---------------------------------------------------------------------------------------------------
void CH263VideoCap::ImproveCap(h263CapStruct *pTemp263) const
{
//Improve the cap - when the MPI is -1 we assume it is like the bigger resolution
#define		MAXRATE		99

	h263CapStruct		*pCapStruct = CAP_CAST(h263CapStruct);

	if (pCapStruct)
	{
		APIS8	frameRate		= 99;
		APIS8	*pTempStructMpi	= &(pTemp263->cif16MPI);
		APIS8	*pStructMpi		= &(pCapStruct->cif16MPI);

		for (EFormat i = k16Cif; i >= kQCif; i--)
		{
			if(*pStructMpi > 0)
			{
				*pTempStructMpi = *pStructMpi;
				frameRate		= *pStructMpi;
			}

			else //*pStruct < 0
			{
				if(frameRate != MAXRATE)
					*pTempStructMpi = frameRate;
				else
					*pTempStructMpi = -1;
			}

			pTempStructMpi--;
			*pStructMpi--;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
BYTE CH263VideoCap::Intersection(const CBaseCap& other, BYTE **ppTemp, BYTE comparePacketizationMode) const
{
	//We need to do a Intersection and we want all the resolutions that initialize to take with MPi that
	//fit to both sides
	//We will start from the bigger resolution and save it's MPI and compare, we will take the maximum MPI.

#define		MAXRATE		99
	const CH263VideoCap *pOtherCap = (const CH263VideoCap *)(&other);

	BYTE				bIsSuccess			= FALSE;
	h263CapStruct		*pCapStruct			= CAP_CAST(h263CapStruct);
	h263CapStruct		*pOtherCapStruct	= (h263CapStruct *)other.GetStruct();
	h263CapStruct		*pTemp263			= (h263CapStruct *)(*ppTemp);

	if (pOtherCap && pCapStruct && pOtherCapStruct)
	{
		APIS8	framRate		= 99;
		APIS8	otherFrameRate	= 99;
		APIS8* pStructMpi		= &(pCapStruct->cif16MPI);
		APIS8* pOtherStructMpi	= &(pOtherCapStruct->cif16MPI);
		APIS8* pTempStructMpi	= &(pTemp263->cif16MPI);

		for (EFormat i = k16Cif; i >= kQCif; i--)
		{
			if(*pStructMpi > 0)
				framRate = *pStructMpi;
			if(*pOtherStructMpi > 0)
				otherFrameRate = *pOtherStructMpi;

			if ((*pStructMpi < 0) && (*pOtherStructMpi < 0)) //if both -1 the result is -1
				*pTempStructMpi = -1;
			else
			{
				if ((framRate == MAXRATE) || (otherFrameRate == MAXRATE)) //if only one side has resolution the intersection is -1
					*pTempStructMpi = -1;
				else
				{
					*pTempStructMpi = max(framRate,otherFrameRate);
					bIsSuccess = TRUE;
				}
			}

			pStructMpi--;
			pOtherStructMpi--;
			pTempStructMpi--;
		}

	   //First we will check if both have the same annexes only if not i will do intersection.
		DWORD tempDetails = 0x00000000;
		BYTE eRes	  = TRUE;

		if (IsH263Plus() && pOtherCap->IsH263Plus() && bIsSuccess)
		{
			eRes &= IsContaining(*(CBaseCap*)pOtherCap,kAnnexes|kFormat,&tempDetails);
			if (eRes)
				eRes &= pOtherCap->IsContaining(*(CBaseCap*)this,kAnnexes|kFormat,&tempDetails);

			if (!eRes)
			{
				//because IntersectionAnnexesAndCustomFormats has reallocate, which changes the pointer
				//of the second argument, we can't send pTemp263
				IntersectionAnnexesAndCustomFormats(*pOtherCap, (h263CapStruct **)ppTemp);
			}
		}
		/* set intersect of rtcp feedback mask fir/pli/tmmbr */
		pTemp263->rtcpFeedbackMask = GetRtcpFeedbackMask() & (pOtherCapStruct->rtcpFeedbackMask);
		if (GetRtcpFeedbackMask() & RTCP_MASK_IS_NOT_STANDARD_ENCODE)
		{
			pTemp263->rtcpFeedbackMask  |= RTCP_MASK_IS_NOT_STANDARD_ENCODE;
		}

	}

	return bIsSuccess;
}

//---------------------------------------------------------------------------------------------------
EResult CH263VideoCap::AddAnnex(annexes_fd_set *pTempMask,annexesListEn eAnnex) const
{
	EResult eRes = kFailure;

	BYTE bIsParameterOK = (eAnnex < H263_Annexes_Number);

	if (bIsParameterOK)
	{
		CAP_FD_SET(eAnnex, pTempMask);
		eRes = kSuccess;
	}

	return eRes;
}

//---------------------------------------------------------------------------------------------------
void CH263VideoCap::SaveCustom(int numOfCustomFormats,customPicFormatSt intersectionCustomFormat[],EFormat customFormat,const CH263VideoCap *pOtherCap) const
{
	//Save intersection custom format to a list.
	customPicFormatSt	*pCustomFormat		= GetAPointerToASpecificCustomFormat(customFormat);
	customPicFormatSt	*pOtherCustomFormat	= pOtherCap->GetAPointerToASpecificCustomFormat(customFormat);
	int					index				= numOfCustomFormats - 1;
	customPicFormatSt	*pIntersection		= &intersectionCustomFormat[index];

	if (pCustomFormat)
	{
		pIntersection->maxCustomPictureWidth	= pCustomFormat->maxCustomPictureWidth;
		pIntersection->maxCustomPictureHeight	= pCustomFormat->maxCustomPictureHeight;
		pIntersection->minCustomPictureWidth	= pCustomFormat->minCustomPictureWidth;
		pIntersection->minCustomPictureHeight	= pCustomFormat->minCustomPictureHeight;
		pIntersection->standardMPI				= max(pCustomFormat->standardMPI,pOtherCustomFormat->standardMPI);
		pIntersection->clockConversionCode		= pCustomFormat->clockConversionCode;
		pIntersection->clockDivisor				= pCustomFormat->clockDivisor;
		pIntersection->customMPI				= pCustomFormat->customMPI;

		memcpy(pIntersection->pixelAspectCode,pCustomFormat->pixelAspectCode,1);
	}
	else
		PTRACE(eLevelInfoNormal,"CH263VideoCap::SaveCustom There is no pCustomFormat");
}

//---------------------------------------------------------------------------------------------------
void CH263VideoCap::AddCustomAndAnnex(h263CapStruct	**ppCapStruct,annexes_fd_set tempMask,int numOfCustomFormats,customPicFormatSt intersectionCustomFormat[])
{
	//Add intersection custom format to a cap set

	EResult				eRes = kFailure;
	customPicFormatSt	*pIntersection;

	(*ppCapStruct)->annexesMask.fds_bits[0] = tempMask.fds_bits[0];
	(*ppCapStruct)->capBoolMask				= 0;

	eRes					= SetNumOfCustomFormatsInMask(numOfCustomFormats);
	BYTE	reallocAnyWay	= TRUE; //We add or reduce annex so we need to realloc

	*ppCapStruct = ReallocIfNeeded(reallocAnyWay);
	if (*ppCapStruct) //allocation succeed
	{
		eRes &= SetAnnexesStructs();
		if(numOfCustomFormats)
		{
			eRes &= SetCustomFormatsDefaults();
			for(int i=0; i < numOfCustomFormats; i++)
			{
				pIntersection	= &intersectionCustomFormat[i];
				eRes &= SetASpecificCustomFormatStruct(
									i,
									pIntersection->maxCustomPictureWidth ,pIntersection->maxCustomPictureHeight,
									pIntersection->minCustomPictureWidth,pIntersection->minCustomPictureHeight,
									pIntersection->standardMPI,pIntersection->pixelAspectCode,1,
									pIntersection->clockConversionCode,pIntersection->clockDivisor,pIntersection->customMPI);
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------
void CH263VideoCap::NullifyArray(customPicFormatSt intersectionCustomFormat[]) const
{
	customPicFormatSt *pIntersection;
	for(int i=0;i<H263_Custom_Number;i++)
	{
		pIntersection	= &intersectionCustomFormat[i];

		pIntersection->maxCustomPictureWidth	= 0;
		pIntersection->maxCustomPictureHeight	= 0;
		pIntersection->minCustomPictureWidth	= 0;
		pIntersection->minCustomPictureHeight	= 0;
		pIntersection->standardMPI				= -1;
		pIntersection->clockConversionCode		= 0;
		pIntersection->clockDivisor				= 0;
		pIntersection->customMPI				= 0;
		pIntersection->pixelAspectCode[0]		= 1;
	}
}

//---------------------------------------------------------------------------------------------------
void CH263VideoCap::IntersectionAnnexesAndCustomFormats(const CBaseCap& other, h263CapStruct** ppIntersectStruct) const
{
	//If we got here we know that there is asymmetric between the annexes so if we exit and we don't found a match the cap
	//are not even
	const CH263VideoCap *pOtherCap = (const CH263VideoCap *)(&other);

	h263CapStruct		*pCapStruct = CAP_CAST(h263CapStruct);
	h263CapStruct		*pOtherCapStruct = (h263CapStruct *)other.GetStruct();

	CH263VideoCap       *pIntersectH263Cap = (CH263VideoCap*)CBaseCap::AllocNewCap(eH263CapCode, *ppIntersectStruct);

	annexes_fd_set		tempMask;

	EResult eRes = kSuccess;

	tempMask.fds_bits[0] = 0;
	int numOfCustomFormats = 0;
	customPicFormatSt  intersectionCustomFormat[H263_Custom_Number];
	NullifyArray(intersectionCustomFormat);


	if (pOtherCap && pCapStruct && pOtherCapStruct)
	{
		if (pOtherCap->IsH263Plus())
		{
			if(IsH263Plus())
			{
				// Check the annexes
				if (IsAnnexes() && pOtherCap->IsAnnexes())
				{
					for (annexesListEn i = typeAnnexB; i < H263_Annexes_Number; i++)
					{
						if (IsAnnex(i) && pOtherCap->IsAnnex(i))
						{
							eRes = AddAnnex(&tempMask,i);
							if(!eRes)
								break;
						}
					}
				}

				if(eRes)
				{
					//check the custom formats
					if (IsCustomFormats() && pOtherCap->IsCustomFormats())
					{
						EFormat customFormat = kCif; // in case its PAL the custom format is CIF.

						if (pOtherCap->IsCustomFormat(customFormat) && IsCustomFormat(customFormat))
						{
							numOfCustomFormats++;
							SaveCustom(numOfCustomFormats,intersectionCustomFormat,customFormat,pOtherCap);
						}
						else
						{
							for (customFormat = kVGA ;customFormat <= kSIF; customFormat++)
							{
								if (pOtherCap->IsCustomFormat(customFormat) && IsCustomFormat(customFormat))
								{
									numOfCustomFormats++;
									SaveCustom(numOfCustomFormats,intersectionCustomFormat,customFormat,pOtherCap);
								}
							}
						}
					}
				}
			}
			//else //remote has annex and local does not have - we do not find intersection.
			if (pIntersectH263Cap && (numOfCustomFormats || (tempMask.fds_bits[0] != 0)) )
				pIntersectH263Cap->AddCustomAndAnnex(ppIntersectStruct,tempMask,numOfCustomFormats,intersectionCustomFormat);
		}
	}

	POBJDELETE(pIntersectH263Cap);
}

//---------------------------------------------------------------------------------------------------
void CH263VideoCap::AddOneCapToOther(const CBaseCap& other, h263CapStruct *pTemp263) const
{
//The remote contain the local so I want to add to the local the caps that he is been missed.

#define		MAXRATE		99
	const CH263VideoCap *pOtherCap = (const CH263VideoCap *)(&other);

	h263CapStruct		*pCapStruct = CAP_CAST(h263CapStruct);
	h263CapStruct		*pOtherCapStruct = (h263CapStruct *)other.GetStruct();

	if (pOtherCap && pCapStruct && pOtherCapStruct)
	{
		*pTemp263				= *pCapStruct;

		APIS8	framRate		= 99;
		APIS8	otherFrameRate	= 99;
		APIS8* pStructMpi		= &(pCapStruct->cif16MPI);
		APIS8* pOtherStructMpi	= &(pOtherCapStruct->cif16MPI);
		APIS8* pTempStructMpi	= &(pTemp263->cif16MPI);

		for (EFormat i = k16Cif; i >= kQCif; i--)
		{
			if(*pOtherStructMpi > 0)
				*pTempStructMpi = *pOtherStructMpi;
			else
				*pTempStructMpi = *pStructMpi;

			pStructMpi--;
			pOtherStructMpi--;
			pTempStructMpi--;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first found value that "other" object has and "this" object is lack of. When such value
//             is found, the function stops checking other values (and the return value will be FALSE)
//             The comparison will be in the following order.
//             The first 4 bits hold value/s :
//             * HIGHER_BIT_RATE   - Other's bit rate is higher.
//             * HIGHER_FORMAT     - Other's format is higher.
//             * HIGHER_FRAME_RATE - Frame per packet is higher in "other".
//             * NO_H263_PLUS      - "Other" object is a h263 plus and "this" is not.
//             * DIFFERENT_ROLE    - "Other" object has a different type (people or content).
//             * Annex type (according to annexesListEn enum) + DETAIL_ANNEXES_OFFSET.
//             Bits 4-7 of the comparison details are the video format (EFormat) that the
//			   HIGHER_FORMAT, HIGHER_FRAME_RATE and annexes relate to.
//			   The 5 bits: 8-12 of the comparison details are the annexes.
//             The combination of values to compare can be from kBitRate, kFormat, kFrameRate or kAnnexes.
//             There is no posibility to compare kFrameRate without kFormat.
//---------------------------------------------------------------------------------------------------
BYTE CH263VideoCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x00000000;

	const CH263VideoCap* pOtherCap = (const CH263VideoCap *)(&other);

	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	h263CapStruct* pOtherCapStruct = (h263CapStruct *)other.GetStruct();

	if (pOtherCap && pCapStruct && pOtherCapStruct)
	{
		// Starts the comparison. If in some stage res gets FALSE the comparison stops
		// and the value that "this" struct was lack of will be saved in the pDetails.

		bRes = CBaseCap::IsContaining(other,valuesToCompare,pDetails);

		// Saves the higher video format for the annex details (if needed)
		EFormat higherVideoFormat = kQCif;

		/* 1) Annexes */
		if (bRes && valuesToCompare & kAnnexes) //maybe we don't care that there are no annexes (like if we check in conf3ctl if current <= initial in transmit. But since we open "best remote caps" in VSW, it can happen, and it's not a problem.)
		{
			if (pOtherCap->IsH263Plus() && pOtherCap->IsAnnexes())
			{//if other has annexes:
				if (IsH263Plus())
				{//if current also has annexes, we should compare:
					// If annexes masks are equal there in no need to enter the loop
					long mask      = pCapStruct->annexesMask.fds_bits[0];
					long otherMask = pOtherCapStruct->annexesMask.fds_bits[0];

					// Turns off the custom formats
					mask      &= CUSTOM_FORMATS_OFF_MASK;
					otherMask &= CUSTOM_FORMATS_OFF_MASK;

					if (mask != otherMask)
					{
						for (annexesListEn i = typeAnnexB; i < H263_Annexes_Number - 1; i++)
						{//we do H263_Annexes_Number - 1, because be don't check typeAnnexI_NS
							// If "other" cap has an annex that "this" is lack of
							if (pOtherCap->IsAnnex(i) && (IsAnnex(i)==FALSE))
							{
								bRes = FALSE;
								*pDetails |= ShiftAnnexDetails(i);
								*pDetails |= ShiftVideoFormat(higherVideoFormat);
								break;
							}
						}
					}
				}

				else //current doesn't have annexes but other has.
				{
					bRes = FALSE;
					*pDetails |= NO_H263_PLUS;
					*pDetails |= ShiftVideoFormat(higherVideoFormat);
				}
			}
			//else: if other doesn't have annexs => current contains other!
		}


		/* 2) Format & Mpi */
		// If the last comparison did not fail check resolutions
		if ((valuesToCompare & kFormat) && bRes)
		{
			EFormat index;
			APIS8* pOtherStructMpi = &(pOtherCapStruct->qcifMPI);
			APIS8* pStructMpi;
			for (EFormat i = kQCif; i <= k16Cif; i++)
			{
				if (*pOtherStructMpi > 0)
				{
					pStructMpi = &(pCapStruct->qcifMPI);
					pStructMpi += i;
					for (index = i; index <= k16Cif; index++)//check only from the current resolution and higher
					{
						if (*pStructMpi > 0)
						{
							higherVideoFormat = max(higherVideoFormat,i);

							if (valuesToCompare & kFrameRate)
							{
								if (*pOtherStructMpi < *pStructMpi)
								{
									*pDetails |= HIGHER_FRAME_RATE;
									*pDetails |= ShiftVideoFormat(i);
									return FALSE;
								}
								else
									break; //appropriate parameters were found for the current resolution
							}
							break;
						}
						pStructMpi++;
					}

					if (index > k16Cif) //=> inappropriate parameters were found
					{
						bRes = FALSE;
						*pDetails |= HIGHER_FORMAT;
						*pDetails |= ShiftVideoFormat(i);
					}
				}
				pOtherStructMpi++;
			}
		}


		// If the last comparison did not fail check custom format
		if ((valuesToCompare & kFormat) && bRes)
		{
			if (pOtherCap->IsH263Plus() && pOtherCap->IsCustomFormats())
			{//if other has custom format:
				if (IsH263Plus())
				{//if current also has custom format, we should compare:
					// in case its PAL the custom format is CIF.
					if (pOtherCap->IsCustomFormat(kCif))
					{
						if(!IsCustomFormats() || !IsCustomFormat(kCif)) // If "other" cap has a custom format that "this" is lack of.
						{
							bRes = FALSE;
							*pDetails |= HIGHER_FORMAT;
							*pDetails |= ShiftVideoFormat(kCif);
						}
						else if (valuesToCompare & kFrameRate)
						{
							if(pOtherCap->GetCustomFormatMpi(kCif) < GetCustomFormatMpi(kCif))//other standardMPI is higher then the local
							{
								bRes = FALSE;
								*pDetails |= HIGHER_FRAME_RATE;
								*pDetails |= ShiftVideoFormat(kCif);
							}
						}
					}
					else
					{
						for (EFormat i = kVGA ;i <= kSIF; i++)
						{
							if (pOtherCap->IsCustomFormat(i))
							{
								if(!IsCustomFormats() || !IsCustomFormat(i)) // If "other" cap has a custom format that "this" is lack of.
								{
									bRes = FALSE;
									*pDetails |= HIGHER_FORMAT;
									*pDetails |= ShiftVideoFormat(i);
									break;
								}
								else if (valuesToCompare & kFrameRate)
								{
									if(pOtherCap->GetCustomFormatMpi(i) < GetCustomFormatMpi(i)) //other standardMPI is higher then the local
									{
										bRes = FALSE;
										*pDetails |= HIGHER_FRAME_RATE;
										*pDetails |= ShiftVideoFormat(i);
										break;
									}
								}
							}
						}
					}
				}

				else //current doesn't have custom format but other has.
				{
					bRes = FALSE;
					*pDetails |= NO_H263_PLUS;
					*pDetails |= ShiftVideoFormat(higherVideoFormat);
				}
			}
			//else: if other doesn't have custom format => current contains other!
		}


		/* 3) rate */
		if ((valuesToCompare & kBitRate) && bRes)
		{
			APIS32 rmtBitrate = pOtherCapStruct->maxBitRate;
			if (GetRole() == kRolePresentation)
				rmtBitrate = (rmtBitrate * CASCADE_BW_FACTOR)/100;
			if (pCapStruct->maxBitRate < GetToleraceRatePct(rmtBitrate))
			{
				bRes = FALSE;
				*pDetails |= HIGHER_BIT_RATE;
			}
		}
		if ((valuesToCompare & kBitRateForCascade) && bRes)
		{
			APIS32 rmtBitrate = (pOtherCapStruct->maxBitRate * CASCADE_BW_FACTOR)/100;
			if (pCapStruct->maxBitRate < rmtBitrate)
			{
				bRes = FALSE;
				*pDetails |= HIGHER_BIT_RATE;
			}
		}
		if ((valuesToCompare & kBitRateWithoutTolerance) && bRes)
		{
			if (pCapStruct->maxBitRate < pOtherCapStruct->maxBitRate)
			{
				bRes = FALSE;
				*pDetails |= HIGHER_BIT_RATE;
			}
		}

	}

	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH263VideoCap::IsThisCapBetter(EFormat* maxFormat, BYTE* bMaxHasAnnex) const
{
	BYTE bIsHigher			= FALSE;
	BYTE bIsEqualFormat		= FALSE;

	bIsHigher = CBaseVideoCap::IsThisCapBetter(maxFormat, bMaxHasAnnex);

	bIsEqualFormat = CBaseVideoCap::IsThisCapEqual(*maxFormat);

	//annexes are stronger than custom picture formats
	if (IsAnnexes() && !*bMaxHasAnnex && (bIsHigher || bIsEqualFormat))
	{
		*bMaxHasAnnex = TRUE;
		*maxFormat    = GetFormat();
		return TRUE;
	}
	else
		return bIsHigher;
}

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH263VideoCap::IsHighestCommonConditionSupported(CHighestCommonCondition& hcCondition) const
{
	if (hcCondition.GetVideoProtocol(H323_INTERFACE_TYPE) != eH263CapCode)
		return FALSE;

	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	if (!pCapStruct)
		return FALSE;

	EFormat desiredResolution = (EFormat)hcCondition.GetResolution(H323_INTERFACE_TYPE);
	// 1) check interlaced mode
	if (hcCondition.IsInterlaced())
	{
		BYTE bInterlacedSupported = IsInterlaced(desiredResolution);
		return bInterlacedSupported;
	}

	// 2) check resolution (this includes custom formats)
	EFormat bestFormatInCap = GetFormat(); //Gets the highest video format
	if (desiredResolution <= bestFormatInCap)
		return TRUE;
	else
		return FALSE;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH263VideoCap::IsInterlaced(EFormat format) const
{
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	if (!pCapStruct)
		return FALSE;

	BYTE bInterlacedSupported = TRUE;

	// Check custom format
	if (IsCustomFormats())
	{
		if (format != kUnknownFormat)//we want to know if the cap is interlaced with a specific resolution
			bInterlacedSupported &= IsCustomFormat(format);
		else//we want to know if the cap is interlaced no matter what is the resolution
			bInterlacedSupported &= (IsCustomFormat(kCif) || IsCustomFormat(kSIF));
	}
	else
		bInterlacedSupported = FALSE;

	// Check Annexes N,W
	if (bInterlacedSupported)
	{
		if (IsAnnexes())
		{
			bInterlacedSupported &= IsAnnex(typeAnnexN);
			bInterlacedSupported &= IsAnnex(typeAnnexW);

			// Check drop field in annex W
			if (bInterlacedSupported)
			{
				int annexIndex = GetAnnexIndex(typeAnnexW);
				if (annexIndex != -1)
				{
					h263OptionsStruct* pOptionsSt = GetAPointerToASpecificAnnex(annexIndex);
					if (pOptionsSt)
					{
						BYTE bIsDropField = (pOptionsSt->annexW.annexBoolMask & annexW_interlacedFields) ? 1 : 0;
						bInterlacedSupported &= bIsDropField;
					}
					else
						bInterlacedSupported = FALSE;
				}
				else
					bInterlacedSupported = FALSE;
			}
		}
		else
			bInterlacedSupported = FALSE;
	}
	return bInterlacedSupported;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH263VideoCap::GetAnnexesForFormat(EFormat eFormat, BYTE& bIsAnnexF, BYTE& bIsAnnexT, BYTE& bIsAnnexN) const
{
	bIsAnnexF = bIsAnnexT = bIsAnnexN = 0;

	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	if (!pCapStruct)
		return FALSE;

	if (IsFormat(eFormat))
	{
		if (IsAnnexes())
		{
			bIsAnnexF = IsAnnex(typeAnnexF);
			bIsAnnexT = IsAnnex(typeAnnexT);
			bIsAnnexN = IsAnnex(typeAnnexN);
		}
		return TRUE;
	}
	else
		return FALSE; //requested format wasn't found
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH263VideoCap::SetStandardFormatsMpi(APIS8 qcifMpi, APIS8 cifMpi, APIS8 cif4Mpi, APIS8 cif16Mpi)
{
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	if (!pCapStruct)
		return;

	pCapStruct->qcifMPI  = qcifMpi;
	pCapStruct->cifMPI   = cifMpi;
	pCapStruct->cif4MPI  = cif4Mpi;
	pCapStruct->cif16MPI = cif16Mpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH263VideoCap::SetH263Plus(BYTE bAnnexF, BYTE bAnnexT, BYTE bAnnexN, BYTE bAnnexI_NS,
							APIS8 vgaMpi, APIS8 ntscMpi, APIS8 svgaMpi, APIS8 xgaMpi, APIS8 qntscMpi)
{
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	if (!pCapStruct)
		return kFailure;

	EResult eRes = kSuccess;
	DWORD numOfCustoms = 0;
	numOfCustoms += (vgaMpi   != -1);
	numOfCustoms += (ntscMpi  != -1);
	numOfCustoms += (svgaMpi  != -1);
	numOfCustoms += (xgaMpi   != -1);
	numOfCustoms += (qntscMpi != -1);

	if (numOfCustoms)
		eRes &= SetNumOfCustomFormatsInMask(numOfCustoms);

	if (bAnnexF)
		eRes &= SetAnAnnexInMask(typeAnnexF);
	if (bAnnexT)
		eRes &= SetAnAnnexInMask(typeAnnexT);
	if (bAnnexN)
		eRes &= SetAnAnnexInMask(typeAnnexN);

	pCapStruct = ReallocIfNeeded();
	if (pCapStruct) //allocation succeed
	{
		//Annex I NS is only in the mask and doesn't have a structure. Therefore, SetAnAnnexInMask(typeAnnexI_NS) is after the ReallocIfNeeded
		if (bAnnexI_NS)
			eRes &= SetAnAnnexInMask(typeAnnexI_NS);

		eRes &= SetAnnexesStructs();
		if (numOfCustoms)
		{
			eRes &= SetCustomFormatsDefaults();
			APIU8 pixelAspectCode[] = {1};
			int counter = 0;
			if (vgaMpi != -1)
			{
				eRes &= SetASpecificCustomFormatStruct(counter, kVGA,   vgaMpi,   pixelAspectCode);
				if (eRes)
					counter++;
			}
			if (ntscMpi != -1)
			{
				eRes &= SetASpecificCustomFormatStruct(counter, kNTSC,  ntscMpi,  pixelAspectCode);
				if (eRes)
					counter++;
			}
			if (svgaMpi != -1)
			{
				eRes &= SetASpecificCustomFormatStruct(counter, kSVGA,  svgaMpi,  pixelAspectCode);
				if (eRes)
					counter++;
			}
			if (xgaMpi != -1)
			{
				eRes &= SetASpecificCustomFormatStruct(counter, kXGA,   xgaMpi,   pixelAspectCode);
				if (eRes)
					counter++;
			}
			if (qntscMpi != -1)
			{
				eRes &= SetASpecificCustomFormatStruct(counter, kSIF, qntscMpi, pixelAspectCode);
				if (eRes)
					counter++;
			}
		}
	}
	else
		eRes = kFailure;

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult  CH263VideoCap::SetH263Interlaced(EFormat format, APIS8 qcifMpi, APIS8 cifMpi)
{
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	if (!pCapStruct)
		return kFailure;

	EResult eRes = kSuccess;

	//cif, qcif
	eRes &= SetFormatMpi(kQCif, qcifMpi);
	eRes &= SetFormatMpi(kCif,  cifMpi);

	//custom
	eRes &= SetNumOfCustomFormatsInMask(1);

	//annexes n,w
	eRes &= SetAnAnnexInMask(typeAnnexN);
	eRes &= SetAnAnnexInMask(typeAnnexW);

	pCapStruct = ReallocIfNeeded();
	if (pCapStruct) //allocation succeed
	{
		eRes &= SetAnnexesStructs();

		eRes &= SetCustomFormatsDefaults();

		APIU8 standardMPI = 0; //always in interlaced mode

		APIU16 clockConversionCode = (format == kCif) ? CLOCK_CONVERSION_CODE_50_FILEDS_IP : CLOCK_CONVERSION_CODE_60_FILEDS_IP;
		APIU8  clockDivisor        = (format == kCif) ? CLOCK_DEVISOR_50_FILEDS : CLOCK_DEVISOR_60_FILEDS;
		APIU16 customMPI           = CUSTOM_MPI_INDICATOR_IP;
		APIU8 pixelAspectCode[] = {1};
		pixelAspectCode[0] = ((format==kSIF) ||(format==kNTSC)) ? 3 : ((format==kCif) ? 2 : 1);

		eRes &= SetASpecificCustomFormatStruct(0, format, standardMPI, pixelAspectCode, 1,
										    	clockConversionCode, clockDivisor, customMPI);
	}
	else
		eRes = kFailure;

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult  CH263VideoCap::CopyStandardFormatsMpi(CH263VideoCap* pOther)
{
	h263CapStruct* pCapStruct = CAP_CAST(h263CapStruct);
	h263CapStruct* pOtherStruct = (h263CapStruct*)pOther->GetStruct();
	if (pCapStruct && pOtherStruct)
	{
		APIS8* pOtherStructMpi = &(pOtherStruct->qcifMPI);
		APIS8* pStructMpi = &(pCapStruct->qcifMPI);
		for (EFormat i = kQCif; i <= k16Cif; i++)
		{
			*pStructMpi = *pOtherStructMpi;
			pStructMpi++;
			pOtherStructMpi++;
		}
		return kSuccess;
	}
	else
		return kFailure;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult  CH263VideoCap::SetHighestCapForVswFromScmAndCardValues()
{
			// get system.cfg values
	EResult eRes = kSuccess;
	for(EFormat i = k16Cif;i != kUnknownFormat; i--)
		eRes &= SetFormatMpi(i, 1);

	//annexes:
	BOOL bAnnexF, bAnnexT, bAnnexN, bAnnexI_NS, bCustomFormat;
	bAnnexF = bAnnexT = bAnnexN = bAnnexI_NS = bCustomFormat = 0;
	bAnnexF = GetSystemCfgFlagInt<BOOL>(CFG_KEY_H263_ANNEX_F);
	bAnnexT = GetSystemCfgFlagInt<BOOL>(CFG_KEY_H263_ANNEX_T);
	bAnnexN = GetSystemCfgFlagInt<BOOL>(CFG_KEY_H263_ANNEX_N);
	bCustomFormat = GetSystemCfgFlagInt<BOOL>(CFG_KEY_HIGHEST_COMMON_CUSTOM_FORMATS);

	bAnnexI_NS = 0; // AnnexI_NS isn't a part of H263 structure.

	//custom picture formats (only the ones required by the DIA):
	APIS8 vgaMpi, ntscMpi, svgaMpi, xgaMpi, qntscMpi;
	if (bCustomFormat)
		vgaMpi = svgaMpi = xgaMpi = 1;
	else
		vgaMpi = svgaMpi = xgaMpi = -1;
	ntscMpi = qntscMpi = -1 ;

	eRes &= SetH263Plus(bAnnexF, bAnnexT, bAnnexN, bAnnexI_NS, vgaMpi, ntscMpi, svgaMpi, xgaMpi, qntscMpi);
	//yael: handle interlace mode according to the conf rate (to do in another function!!!)
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////
EResult  CH263VideoCap::SetHighestCapForCpFromScmAndCardValues(DWORD videoRateIn100Bits, eVideoQuality videoQuality)
{
	/*
	DWORD rateInBitPerSec = videoRate * 1000;
	BYTE numberOf64s = (rateInBitPerSec % rate64K) ? (rateInBitPerSec/rate64K + 1) : rateInBitPerSec/rate64K;
	DWORD videoRateIn64bits = numberOf64s * 64;
	*/

	EResult eRes = kSuccess;
	APIS8 buffer[MAX_VIDEO_FORMATS];
	memset(buffer, -1, MAX_VIDEO_FORMATS);
	CH263VideoMode::Get263VideoCardMPI(videoRateIn100Bits, buffer, videoQuality);

	CMedString mstr;
	mstr << "Video Quality[" <<  videoQuality << "] QCif[" << (int)(buffer[0]) << "] Cif[" << (int)(buffer[1]) << "]";
	PTRACE2(eLevelInfoNormal,"CH263VideoCap::SetHighestCapForCpFromScmAndCardValues: ",mstr.GetString());

	for(EFormat i = kQVGA;i != kUnknownFormat; i--)
		eRes &= SetFormatMpi(i, buffer[i]);
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Add additonal XML info for the XML API.
void  CH263VideoCap::SetAdditionalXmlInfo()
{
	h263CapStruct *pCapStruct = CAP_CAST(h263CapStruct);
	int numOfAnnexes = 0;
	int numOfCustoms = 0;

	if (pCapStruct)
	{
		numOfAnnexes = GetNumOfAnnexes();
		numOfCustoms = GetNumOfCustomFormats(); // from mask
		pCapStruct->xmlDynamicProps.numberOfDynamicParts = 0;
		if (numOfAnnexes > 0)
			pCapStruct->xmlDynamicProps.numberOfDynamicParts++;//numOfAnnexes + numOfCustoms;// number of annexes + number of custom format
		if (numOfCustoms > 0)
			pCapStruct->xmlDynamicProps.numberOfDynamicParts++;

		if 	(pCapStruct->xmlDynamicProps.numberOfDynamicParts > 0)
			pCapStruct->xmlDynamicProps.sizeOfAllDynamicParts = numOfAnnexes * sizeof(h263OptionsStruct) +
															sizeof(customPic_StBase) + numOfCustoms * sizeof(customPicFormatSt);// size of the dynamic parts only
		else
			pCapStruct->xmlDynamicProps.sizeOfAllDynamicParts = 0;

		// loop of all the annexes
		h263OptionsStruct *pH263OptionsSt = NULL;
		for(int i = 0; i < numOfAnnexes; i++)
		{
			// annexB is a custing since all the headers has the same xml header (should enter to the interface with the CS)
			pH263OptionsSt = GetAPointerToASpecificAnnex(i);
			if (pH263OptionsSt)
			{
				pH263OptionsSt->annexB.xmlHeader.dynamicType	= GetAnnexEnAccordingToIndex(i);// the annex type
				pH263OptionsSt->annexB.xmlHeader.dynamicLength = sizeof(h263OptionsStruct);// the annexes union size
			}
			else
				PTRACE2INT(eLevelInfoNormal,"CH263VideoCap::SetAdditionalXmlInfo pH263OptionsSt[i] is NULL, i=",i);
		}

		// loop of all the custom format resolutions
		if(IsCustomFormats())
		{
			customPic_St *pCustomFormats = GetAPointerToCustomFormats();
			if (pCustomFormats)
			{
				pCustomFormats->xmlHeader.dynamicType = tblCustomFormat;// custom format enum
				pCustomFormats->xmlHeader.dynamicLength = sizeof(customPic_StBase) + numOfCustoms * sizeof(customPicFormatSt);// size of all the custom format resolutions
				pCustomFormats->xmlDynamicProps.numberOfDynamicParts = GetNumOfCustomFormats();// number of custom format resolutions.
				pCustomFormats->xmlDynamicProps.sizeOfAllDynamicParts = numOfCustoms * sizeof(customPicFormatSt);// size of all the custom format resolutions.

				// loop of all the custom format
				for (int j = 0; j < numOfCustoms; j++)
				{
					customPicFormatSt *pCustomPicFormatSt = GetAPointerToASpecificCustomFormat(j);
					if (pCustomPicFormatSt)
					{
						pCustomPicFormatSt->xmlHeader.dynamicType	= typeCustomPic;// custom format enum
						pCustomPicFormatSt->xmlHeader.dynamicLength = sizeof(customPicFormatSt);//the custom format structure size
					}
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CH263VideoCap::GetCPVideoPartyType() const
{
	BYTE is4CIF = NO;
	if(IsFormat(k4Cif))
	{
		is4CIF = YES;
		PTRACE(eLevelInfoNormal,"CH263VideoCap::GetCPVideoPartyType Cap are with 4Cif");
	}
	eVideoPartyType eResVideoPartyType = GetH261H263ResourcesPartyType(is4CIF);
	return eResVideoPartyType;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CGenericVideoCap
//=======================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CGenericVideoCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(genericVideoCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CGenericVideoCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(genericVideoCapStruct);
	return size;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CGenericVideoCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(genericVideoCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(genericVideoCapStruct));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values
//---------------------------------------------------------------------------------------------------
EResult CGenericVideoCap::SetDefaults(cmCapDirection eDirection, ERoleLabel eRole)
{
	EResult eRes = kFailure;
	genericVideoCapStruct *pCapStruct = CAP_CAST(genericVideoCapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);
		pCapStruct->maxBitRate		 = 0;

		for(int i=0; i < CT_GenricVideo_Data_Len; i++)
			pCapStruct->data[i] = '\0';

	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Copies the base structure EXCEPT from the header and the name.
//             the structures should be allocated with the same size!!!
//---------------------------------------------------------------------------------------------------

EResult CGenericVideoCap::CopyBaseQualities(const CBaseVideoCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.GetStruct())
	{
		eRes = kSuccess;
		int sizeBaseWithAnnex = sizeof(genericVideoCapStruct);
		int sizeWithoutBaseSize = sizeBaseWithAnnex - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.GetStruct()) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, sizeWithoutBaseSize);
	}
	return eRes;

}

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the paramters of the generic audio according to the H221 values
//---------------------------------------------------------------------------------------------------
EResult CGenericVideoCap::SetFromH221Cap(const CCapH221 & h221Cap,cmCapDirection eDirection,BYTE bIsQuad)
{
	EResult eRes = kFailure;
	genericVideoCapStruct *pCapStruct = CAP_CAST(genericVideoCapStruct);

	if (pCapStruct)
	{
		eRes = SetDefaults(eDirection);
		BYTE genericType = h221Cap.GetFieldDropValue();
		if(genericType == NS_FIELD_DROP)
			pCapStruct->genericCodeType = eDropField;
		pCapStruct->data[1] = '\0';
	}
	return eRes;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CGenericVideoCap::SetDropFieldCap(cmCapDirection eDirection)
{
	EResult eRes = kFailure;
	genericVideoCapStruct* pCapStruct = CAP_CAST(genericVideoCapStruct);

	if (pCapStruct)
	{
		eRes = SetDefaults(eDirection);
		pCapStruct->genericCodeType = eDropField;
		pCapStruct->data[1] = '\0';
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CGenericVideoCap::IsDropField() const
{
	BYTE bRes = FALSE;
	genericVideoCapStruct* pCapStruct = CAP_CAST(genericVideoCapStruct);

	if (pCapStruct)
		if (pCapStruct->genericCodeType == eDropField)
			bRes = TRUE;

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the bit rate.
//---------------------------------------------------------------------------------------------------
EResult CGenericVideoCap::SetBitRate(APIS32 rate)
{
	EResult eRes = kFailure;
	genericVideoCapStruct *pCapStruct = CAP_CAST(genericVideoCapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxBitRate = rate;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the bit rate
//---------------------------------------------------------------------------------------------------
APIS32 CGenericVideoCap::GetBitRate() const
{
	APIS32 bitRate = -1;
	genericVideoCapStruct *pCapStruct = CAP_CAST(genericVideoCapStruct);

	if (pCapStruct)
		bitRate = pCapStruct->maxBitRate;

	return bitRate;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CGenericVideoCap::Dump(std::ostream& msg) const
{
	genericVideoCapStruct *pCapStruct = CAP_CAST(genericVideoCapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		if((pCapStruct->genericCodeType == eDropField) || (pCapStruct->genericCodeType == NS_FIELD_DROP))
		{
			msg << "Generic CapCode			= " << "eDropField";
		}

		msg << "\n ---- NonStandard Message end ----\n" << (std::dec);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if the format is supported in the struct (if it's mpi > 0)
//---------------------------------------------------------------------------------------------------
BYTE CGenericVideoCap::IsFormat(EFormat eFormat) const
{
	BYTE bRes = (GetFormatMpi(eFormat) > 0);
	return bRes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CDBC2VideoCap
//=======================
EResult CDBC2VideoCap::SetDefaults(cmCapDirection eDirection, ERoleLabel eRole)
{
	EResult eRes = kFailure;
	eRes = CGenericVideoCap::SetDefaults(eDirection, eRole);
	genericVideoCapStruct *pCapStruct = CAP_CAST(genericVideoCapStruct);

	if (pCapStruct && (eRes == kSuccess))
	{
		// set the Polycom H26L and the base profile, data field has to end with '\0'
		eRes = kSuccess;
		pCapStruct->genericCodeType				= eDBC2Code;

		// Set parameters type
		pCapStruct->data[1] = 0;
		pCapStruct->data[2] = '\0';

		pCapStruct->data[0]  = 0;
//		pCapStruct->data[0]  |= DBC2_motionVectors_Cap; //open the bit 6 motionVectors (there is no set of motion vector in capability exchange)
		pCapStruct->data[0]  |= DBC2_requiresEncapsulation_Cap; //open bit 4 requiresEncapsulation
		pCapStruct->data[0]  |= 2; //open bit 2

	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CDBC2VideoCap::Dump(std::ostream& msg) const
{
	genericVideoCapStruct *pCapStruct = CAP_CAST(genericVideoCapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		int data = 0;
		data = (BYTE)pCapStruct->genericCodeType;
		msg << "genericCodeType				= (" << (std::hex) << data <<")";
		if(data == eDBC2Code)
			msg << " -- DBC2Code";

		data = (BYTE)pCapStruct->data[0];
		msg << "\nParameter data			= (" << (std::hex) << data <<")";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values
//---------------------------------------------------------------------------------------------------

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CH264VideoCap
//=======================


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CH264VideoCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(h264CapStruct);
	return size;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CH264VideoCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(h264CapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CH264VideoCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(h264CapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(h264CapStruct));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values
//---------------------------------------------------------------------------------------------------
EResult CH264VideoCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);

		pCapStruct->customMaxMbpsValue          = -1;
		pCapStruct->customMaxFsValue            = -1;
		pCapStruct->customMaxDpbValue           = -1;
		pCapStruct->customMaxBrAndCpbValue      = -1;
		pCapStruct->profileValue		        = H264_Profile_BaseLine;
		pCapStruct->levelValue			        = 0;
		pCapStruct->maxStaticMbpsValue	        = -1;
		pCapStruct->sampleAspectRatiosValue     = -1;
		/* packetizationMode=1 only enabled only for mpmx cards.
		 * packetization mode 1 is mainly used in TIP
		 * */
		{
			BOOL packetizationModeSupport = true;
			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			if (pSysConfig)
	   		    pSysConfig->GetBOOLDataByKey("SIP_H264_PACKETIZATION_MODE", packetizationModeSupport);
			pCapStruct->packetizationMode = packetizationModeSupport && IsFeatureSupportedBySystem(eFeatureH264PacketizationMode);
		}

		pCapStruct->maxFR                   	= 0;
		pCapStruct->H264mode		        	= H264_standard;
		pCapStruct->rtcpFeedbackMask            = (RTCP_MASK_FIR | RTCP_MASK_TMMBR | RTCP_MASK_PLI);

	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH264VideoCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	h264CapStruct *pCapStruct = CAP_CAST(h264CapStruct);
	CH264VideoCap &hOtherVideoCap  = (CH264VideoCap&)otherCap;

	if (GetCapCode() == hOtherVideoCap.GetCapCode())
	{
		eRes = kSuccess;
		pCapStruct->maxBitRate			   = hOtherVideoCap.GetBitRate();
		pCapStruct->maxFR   			   = hOtherVideoCap.GetMaxFR();
		pCapStruct->H264mode			   = hOtherVideoCap.GetH264mode();
		pCapStruct->rtcpFeedbackMask       = hOtherVideoCap.GetRtcpFeedbackMask();
		pCapStruct->customMaxMbpsValue     = hOtherVideoCap.GetMbps();
		pCapStruct->customMaxFsValue       = hOtherVideoCap.GetFs();
		pCapStruct->customMaxDpbValue      = hOtherVideoCap.GetDpb();
		pCapStruct->customMaxBrAndCpbValue = hOtherVideoCap.GetBrAndCpb();
		pCapStruct->levelValue			   = hOtherVideoCap.GetLevel();
		pCapStruct->maxStaticMbpsValue	   = hOtherVideoCap.GetStaticMB();
		pCapStruct->sampleAspectRatiosValue = hOtherVideoCap.GetSampleAspectRatio();
		pCapStruct->profileValue			= hOtherVideoCap.GetProfile();

		/* For Non-mpmx cards, set packetizationMode to 0.*/
		if (IsFeatureSupportedBySystem(eFeatureH264PacketizationMode)) {
			pCapStruct->packetizationMode = hOtherVideoCap.GetPacketizationMode();
		} else {
			pCapStruct->packetizationMode = 0;
		}

	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Copies the base structure EXCEPT from the header and the name.
//             the structures should be allocated with the same size!!!
//---------------------------------------------------------------------------------------------------

EResult CH264VideoCap::CopyBaseQualities(const CBaseVideoCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.GetStruct())
	{
		eRes = kSuccess;
		int sizeBaseWithAnnex = sizeof(h264CapStruct);
		int sizeWithoutBaseSize = sizeBaseWithAnnex - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.GetStruct()) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, sizeWithoutBaseSize);
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets a content struct.
//---------------------------------------------------------------------------------------------------
EResult CH264VideoCap::SetContent(ERoleLabel eRole, cmCapDirection eDirection,BOOL isHD1080, BYTE HDMpi, BOOL isHighProfile)
{
	//DWORD dMbpsValue = 44;
	EResult eRes = kFailure;
	// The MPI concept is changed for supporting 60fps:
	// ->  MPI 1 = 60fps; MPI 2 = 30 fps; MPI 4 = 15fps; MPI 10 = 5fps
	if(HDMpi == 0)
	{
		//PASSERT(1);
		//return eRes;
		PTRACE(eLevelError,"Remote has mpi lower than 5");
		if (isHD1080 == TRUE)
			HDMpi = 4;
		else
			HDMpi = 10;
	}

	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		//for now we don't support
		SetDefaults(eDirection, eRole);
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);
		DWORD resolutionMbps = 0;
		if(isHD1080)
		{
			WORD levelValue = H264_Level_3_1;
			//HD1080p
			switch(HDMpi)
			{
				case 1:
				{
					resolutionMbps = H264_HD1080_60_MBPS;
					levelValue = H264_Level_4;
					break;
				}
				case 2:
				{
					resolutionMbps = H264_HD1080_30_MBPS;
					levelValue = H264_Level_3_2;
					break;
				}
				case 4:
				{
					resolutionMbps = H264_HD1080_15_MBPS;
					break;
				}
				default:
				{
					PASSERT(HDMpi);
					eRes = kFailure;
					return eRes;
				}
			}

			pCapStruct->customMaxFsValue       = GetMaxFsAsDevision(H264_HD1080_FS);
			pCapStruct->customMaxDpbValue      = -1;
			pCapStruct->customMaxMbpsValue     = GetMaxMbpsAsDevision(resolutionMbps);
			pCapStruct->customMaxBrAndCpbValue = -1;
			pCapStruct->profileValue		   = isHighProfile? H264_Profile_High : H264_Profile_BaseLine;
			pCapStruct->levelValue			   = levelValue;
			pCapStruct->maxStaticMbpsValue	   = -1;
			pCapStruct->sampleAspectRatiosValue = -1;
			pCapStruct->packetizationMode		= isHighProfile? H264_NON_INTERLEAVED_PACKETIZATION_MODE : 0;
		}
		else
		{  //HD720p
			switch(HDMpi)
			{
				case 1:
				case 2:
				{
					resolutionMbps = H264_HD720_30_MBPS;
					break;
				}
				case 4:
				{
					resolutionMbps = H264_HD720_15_MBPS;
					break;
				}
				case 10:
				{
					resolutionMbps = H264_HD720_5_MBPS;
					break;
				}
				case 8:
				{
					resolutionMbps = H264_HD720_5_MBPS;
					break;
				}
				default:
				{
					PASSERT(HDMpi);
					eRes = kFailure;
					return eRes;
				}
			}

			pCapStruct->customMaxFsValue       = GetMaxFsAsDevision(H264_HD720_FS);
			pCapStruct->customMaxDpbValue      = -1;
			pCapStruct->customMaxMbpsValue     = GetMaxMbpsAsDevision(resolutionMbps);;
			pCapStruct->customMaxBrAndCpbValue = -1;
			pCapStruct->profileValue		   = isHighProfile? H264_Profile_High : H264_Profile_BaseLine;
			pCapStruct->levelValue			   = H264_Level_2_2;
			pCapStruct->maxStaticMbpsValue	   = -1;
			pCapStruct->sampleAspectRatiosValue = -1;
			pCapStruct->packetizationMode		= isHighProfile? H264_NON_INTERLEAVED_PACKETIZATION_MODE : 0;
		}

		/* EP declaration for HD720
		profileValue                   = 64
		levelValue                     = 57
		customMaxMbpsValue             = 80 (40000 MB/s)
		customMaxFsValue               = 7 (1792 MBs)
		customMaxDpbValue              = -1
		customMaxBrAndCpbValue         = 205 (5125000 bits/s)
		maxStaticMbpsValue             = -1
		sampleAspectRatiosValue         = 13 H/V
		 */
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH264VideoCap::SetHDContent(ERoleLabel eRole,EHDResolution eHDRes,cmCapDirection eDirection,BYTE HDMpi,BOOL isHighProfile)
{
	//DWORD dMbpsValue = 44;
	//dMbpsValue = GetSystemCfgFlagInt<DWORD>(CFG_KEY_H264_CONTENT_MBPS_VALUE);
	DWORD resolutionMbps = 0;

	// The MPI concept is changed for supporting 60fps:
	// ->  MPI 1 = 60fps; MPI 2 = 30 fps; MPI 4 = 15fps; MPI 10 = 5fps
	EResult eRes = kFailure;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		//for now we don't support
		SetDefaults(eDirection, eRole);
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);

		if(eHDRes == eHD1080Res)
		{
			WORD levelValue = H264_Level_3_1;
			switch(HDMpi)
			{
				case 1:
				{
					resolutionMbps = H264_HD1080_60_MBPS;
					levelValue = H264_Level_4;
					break;
				}
				case 2:
				{
					resolutionMbps = H264_HD1080_30_MBPS;
					levelValue = H264_Level_3_2;
					break;
				}
				case 4:
				{
					resolutionMbps = H264_HD1080_15_MBPS;
					break;
				}
				default:
				{
					PASSERT(1);
					eRes = kFailure;
					return eRes;
				}
			}
			//HD1080p ///Need to Update/////
			pCapStruct->customMaxFsValue       = GetMaxFsAsDevision(H264_HD1080_FS);
			pCapStruct->customMaxDpbValue      = -1;
			pCapStruct->customMaxMbpsValue     = GetMaxMbpsAsDevision(resolutionMbps);
			pCapStruct->customMaxBrAndCpbValue = -1;
			pCapStruct->profileValue		   = isHighProfile? H264_Profile_High : H264_Profile_BaseLine;
			pCapStruct->levelValue			   = levelValue;
			pCapStruct->maxStaticMbpsValue	   = -1;
			pCapStruct->sampleAspectRatiosValue = -1;
			pCapStruct->packetizationMode		= isHighProfile? H264_NON_INTERLEAVED_PACKETIZATION_MODE : 0;
		}

		else if(eHDRes == eHD720Res)
		{
			switch(HDMpi)
			{
				case 1:
				case 2:
				{
					resolutionMbps = H264_HD720_30_MBPS;
					break;
				}
				case 4:
				{
					resolutionMbps = H264_HD720_15_MBPS;
					break;
				}
				case 10:
				{
					resolutionMbps = H264_HD720_5_MBPS;
					break;
				}
				case 8:
				{
					resolutionMbps = H264_HD720_5_MBPS;
					break;
				}
				default:
				{
					PASSERT(1);
					eRes = kFailure;
					return eRes;
				}
			}

			pCapStruct->customMaxFsValue       = 15;
			pCapStruct->customMaxDpbValue      = -1;
			pCapStruct->customMaxMbpsValue     = GetMaxMbpsAsDevision(resolutionMbps);
			pCapStruct->customMaxBrAndCpbValue = -1;
			pCapStruct->profileValue		   = isHighProfile? H264_Profile_High : H264_Profile_BaseLine;
			pCapStruct->levelValue			   = H264_Level_2_2;
			pCapStruct->maxStaticMbpsValue	   = -1;
			pCapStruct->sampleAspectRatiosValue = -1;
			pCapStruct->packetizationMode		= isHighProfile? H264_NON_INTERLEAVED_PACKETIZATION_MODE : 0;

		}
		//Olga - eHD720p60Re, eSDRes, eH263Res, eH264Res ?


		/* EP declaration for HD720
		profileValue                   = 64
		levelValue                     = 57
		customMaxMbpsValue             = 80 (40000 MB/s)
		customMaxFsValue               = 7 (1792 MBs)
		customMaxDpbValue              = -1
		customMaxBrAndCpbValue         = 205 (5125000 bits/s)
		maxStaticMbpsValue         H264_Level_3_1    = -1
		sampleAspectRatiosValue         = 13 H/V
		 */
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH264VideoCap::SetTIPContent(ERoleLabel eRole, cmCapDirection eDirection, BYTE bSet264ModeAsTipContent)
{
//	DWORD dMbpsValue = 44;
//	dMbpsValue = GetSystemCfgFlagInt<DWORD>(CFG_KEY_H264_CONTENT_MBPS_VALUE);

	EResult eRes = kFailure;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		//for now we don't support
		SetDefaults(eDirection, eRole);
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);

		pCapStruct->customMaxFsValue        = 12;// XGA =>  3072FS / 256FACTOR = 12
		pCapStruct->customMaxDpbValue       = -1;
		pCapStruct->customMaxMbpsValue      = 31; // (5FPS * 3072FS) / 500FACTOR = 30.72 => ~ 31
		pCapStruct->customMaxBrAndCpbValue  = -1;
		pCapStruct->profileValue		    = H264_Profile_BaseLine;
		pCapStruct->levelValue			    = H264_Level_1_3; // 5FPS * 3072FS = 15360MBPS and rate=0.5M
		pCapStruct->maxStaticMbpsValue	    = -1;
		pCapStruct->sampleAspectRatiosValue = -1;
		/* MPM/MPM+ cards do not support packetizationMode=1 */
		if (IsFeatureSupportedBySystem(eFeatureH264PacketizationMode))
		{
			pCapStruct->packetizationMode = 1;
		}
		else
		{
			pCapStruct->packetizationMode = 0;
			PTRACE(eLevelInfoNormal, "CH264VideoCap::SetTIPContent Setting packetization mode to 0, on MPM+ card");
		}

		if (bSet264ModeAsTipContent == TRUE) //just for TipCompatibility:video&content
		{
			pCapStruct->H264mode		        = H264_tipContent;
			pCapStruct->maxFR                   = 5;
		}
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CH264VideoCap::Dump(std::ostream& msg) const
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);

		msg << "profileValue            = " << (int)pCapStruct->profileValue;
		msg << "\nlevelValue              = " << (int)pCapStruct->levelValue;

		//Mbps:
		msg << "\ncustomMaxMbpsValue      = " << pCapStruct->customMaxMbpsValue;
		APIS32 mbps = pCapStruct->customMaxMbpsValue;
		if (mbps != -1)
		{
			if ((mbps / CUSTOM_MAX_MBPS_FACTOR) < 3)
				msg << " (" << pCapStruct->customMaxMbpsValue*CUSTOM_MAX_MBPS_FACTOR << " MB/s)";
			else
				msg << " MB/s";
		}

		//Fs
		msg << "\ncustomMaxFsValue        = " << pCapStruct->customMaxFsValue;
		APIS32 fs = pCapStruct->customMaxFsValue;
		if (fs != -1)
		{
			if ((fs / CUSTOM_MAX_FS_FACTOR) == 0)
				msg << " (" << pCapStruct->customMaxFsValue*CUSTOM_MAX_FS_FACTOR << " MBs)";
			else
				msg << " MBs";
		}

		//Dpb
		msg << "\ncustomMaxDpbValue       = " << pCapStruct->customMaxDpbValue;
		APIS32 dpb = pCapStruct->customMaxDpbValue;
		if (dpb != -1)
		{
			if ((dpb / CUSTOM_MAX_DPB_FACTOR) == 0)
				msg << " (" << pCapStruct->customMaxDpbValue*CUSTOM_MAX_DPB_FACTOR << " Bytes)";
			else
				msg << " Bytes";
		}

		//BrAndCpb
		msg << "\ncustomMaxBrAndCpbValue  = " << pCapStruct->customMaxBrAndCpbValue;
		APIS32 brAndCpb = pCapStruct->customMaxBrAndCpbValue;
		if (brAndCpb != -1)
		{
			if ((brAndCpb / CUSTOM_MAX_BR_FACTOR) == 0)
				msg << " (" << pCapStruct->customMaxBrAndCpbValue*CUSTOM_MAX_BR_FACTOR << " bits/s)";
			else
				msg << " bits/s";
		}

		// StaticMbps
		msg << "\nmaxStaticMbpsValue      = " << pCapStruct->maxStaticMbpsValue;
		APIS32 staticMbps = pCapStruct->maxStaticMbpsValue;
		if (staticMbps != -1)
		{
			if ((staticMbps / CUSTOM_MAX_MBPS_FACTOR) < 3)
				msg << " (" << pCapStruct->maxStaticMbpsValue*CUSTOM_MAX_MBPS_FACTOR << " MB/s)";
			else
				msg << " MB/s";
		}

		// SampleAspectRatio
		msg << "\nsampleAspectRatiosValue = " << pCapStruct->sampleAspectRatiosValue;

		if (pCapStruct->sampleAspectRatiosValue != -1)
			msg <<  " H/V";

		// Packetization Mode
		msg << "\npacketizationMode       = " << (int)pCapStruct->packetizationMode;

		if (pCapStruct->maxBitRate >= 0)
			msg << "\nmaxBitRate              = " <<pCapStruct->maxBitRate  <<
			" (" << (pCapStruct->maxBitRate)*100 << " Bps)";
		else
			msg << "\nmaxBitRate              = Unknown";

		if (pCapStruct->maxFR >= 0)
			msg << "\nmaxFR                   = " <<pCapStruct->maxFR  <<
			" (" << (pCapStruct->maxFR) << " frame rate per second)";
		else
			msg << "\nmaxFR                   = Unknown";

		if (pCapStruct->rtcpFeedbackMask >= 0)
			msg << "\nrtcpFeedbackMask        = " <<pCapStruct->rtcpFeedbackMask;
		else
			msg << "\nrtcpFeedbackMask        = Unknown";

		msg << "\nH264mode                = " <<pCapStruct->H264mode << "\n";
	}
}

//---------------------------------------------------------------------------------------------------
BYTE CH264VideoCap::Intersection(const CBaseCap& other, BYTE** ppIntersectData, BYTE comparePacketizationMode) const
{
	PTRACE(eLevelInfoNormal,"CH264VideoCap::Intersection");
	//We need to do a Intersection and we want all the resolutions that initialize to take with MPi that
	//fit to both sides
	//We will start from the bigger resolution and save it's MPI and compare, we will take the maximum MPI.

	BYTE bIsSuccess = FALSE;
	const CH264VideoCap *pOtherCap = (const CH264VideoCap *)(&other);

	h264CapStruct *pThisCapStruct	 = CAP_CAST(h264CapStruct);
	h264CapStruct *pOtherCapStruct	 = (h264CapStruct *)other.GetStruct();

	h264CapStruct *pIntersectStuct	 = (h264CapStruct *)*ppIntersectData;
	CH264VideoCap *pIntersectH264Cap = (CH264VideoCap*)CBaseCap::AllocNewCap(eH264CapCode, pIntersectStuct);
	if(pIntersectH264Cap == NULL)
	{
		PTRACE(eLevelInfoNormal, "CH264VideoCap::Intersection -  pIntersectH264Cap is NULL");
		return FALSE;
	}
	pIntersectH264Cap->SetDefaults();

	if (pOtherCap && pThisCapStruct && pOtherCapStruct)
	{
		BYTE bIntersectParams = IntersectH264Profile(pThisCapStruct, pOtherCapStruct, pIntersectH264Cap); // Intersect Profile
		if (bIntersectParams)
		{
		    if (pThisCapStruct->H264mode != pOtherCapStruct->H264mode)	// Compare H264 mode
		    {
		    	bIntersectParams = FALSE;
		    	CSmallString str;
				str << "H264mode is different.";
				PTRACE2(eLevelInfoNormal,"CH264VideoCap::Intersection : ",str.GetString());
		    }
		}
		if (bIntersectParams)
		{
			if(CProcessBase::GetProcess()->GetProductType()!=eProductTypeSoftMCUMfw)
			{
				 if (((pThisCapStruct->packetizationMode != pOtherCapStruct->packetizationMode) && comparePacketizationMode)) /* In case packetization mode comparison is set to FALSE (323 call) do not compare */
				{
					bIntersectParams = FALSE;
					CSmallString str;
					str << "packetization mode is different.";
					PTRACE2(eLevelInfoNormal,"CH264VideoCap::Intersection : ",str.GetString());
				}
			}
		}
		if (bIntersectParams)
		{
			//LEVEL
			APIU8 thisLevel      = pThisCapStruct->levelValue;
			APIU8 otherLevel     = pOtherCapStruct->levelValue;
			APIU8 intersectLevel = min(thisLevel, otherLevel);
			if(intersectLevel >= H264_Level_1)
			{
				pIntersectH264Cap->SetLevel(intersectLevel);
				bIsSuccess = TRUE;


				//MBPS
				APIS32 thisMpbs  = pThisCapStruct->customMaxMbpsValue;
				APIS32 otherMpbs = pOtherCapStruct->customMaxMbpsValue;
				APIS32 intersectMbps = IntersectCustomParam(CUSTOM_MAX_MBPS_CODE, intersectLevel, thisLevel, otherLevel, thisMpbs, otherMpbs);
				pIntersectH264Cap->SetMbps(intersectMbps);

				//FS
				APIS32 thisFs  = pThisCapStruct->customMaxFsValue;
				APIS32 otherFs = pOtherCapStruct->customMaxFsValue;
				APIS32 intersectFs = IntersectCustomParam(CUSTOM_MAX_FS_CODE, intersectLevel, thisLevel, otherLevel, thisFs, otherFs);
				pIntersectH264Cap->SetFs(intersectFs);

				//DPB
				APIS32 thisDpb  = pThisCapStruct->customMaxDpbValue;
				APIS32 otherDpb = pOtherCapStruct->customMaxDpbValue;
				APIS32 intersectDpb = IntersectCustomParam(CUSTOM_MAX_DPB_CODE, intersectLevel, thisLevel, otherLevel, thisDpb, otherDpb);
				pIntersectH264Cap->SetDpb(intersectDpb);

				//BR
				APIS32 thisBrAndCpb = pThisCapStruct->customMaxBrAndCpbValue;
				APIS32 otherBrAndCpb   = pOtherCapStruct->customMaxBrAndCpbValue;
				APIS32 thisBrDevision  = thisBrAndCpb;
				APIS32 otherBrDevision = otherBrAndCpb;
				APIS32 intersectBrDevision = IntersectCustomParam(CUSTOM_MAX_BR_CODE, intersectLevel, thisLevel, otherLevel, thisBrDevision, otherBrDevision);
				APIS32 intersectBrAndCpb   = intersectBrDevision;
				pIntersectH264Cap->SetBrAndCpb(intersectBrAndCpb);

				//SAR
				APIS32 thisSar  = pThisCapStruct->sampleAspectRatiosValue;
				APIS32 otherSar = pOtherCapStruct->sampleAspectRatiosValue;
				APIS32 intersectResult;
				if (thisSar == -1 || otherSar == -1)
					intersectResult = -1;
				else
					intersectResult = min(thisSar, otherSar);
				pIntersectH264Cap->SetSampleAspectRatio(intersectResult);

				//Static MB
				APIS32 thisStaticMB  = pThisCapStruct->maxStaticMbpsValue;
				APIS32 otherStaticMB = pOtherCapStruct->maxStaticMbpsValue;
				if (thisStaticMB == -1 || otherStaticMB == -1)
					intersectResult = -1;
				else
					intersectResult = min(thisStaticMB, otherStaticMB);
				pIntersectH264Cap->SetStaticMB(intersectResult);

				if (pThisCapStruct->H264mode == H264_tipContent)
				{
					APIS32 thisMaxFR  = pThisCapStruct->maxFR;
					APIS32 otherMaxFR = pOtherCapStruct->maxFR;

					if (thisMaxFR == -1 || otherMaxFR == -1)
						intersectResult = -1;
					else
						intersectResult = min(thisMaxFR, otherMaxFR);

					pIntersectH264Cap->SetMaxFR(intersectResult);
				}

				pIntersectH264Cap->SetPacketizationMode(pThisCapStruct->packetizationMode);
			}

			((CBaseVideoCap *)pIntersectH264Cap)->SetRtcpFeedbackMask(GetRtcpFeedbackMask() & (pOtherCapStruct->rtcpFeedbackMask));
			if (GetRtcpFeedbackMask() & RTCP_MASK_IS_NOT_STANDARD_ENCODE)
			{
				((CBaseVideoCap *)pIntersectH264Cap)->SetRtcpFeedbackMask((((CBaseVideoCap *)pIntersectH264Cap)->GetRtcpFeedbackMask())| RTCP_MASK_IS_NOT_STANDARD_ENCODE);
			}
		}

	}

	POBJDELETE(pIntersectH264Cap);
	return bIsSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IntersectH264Profile(h264CapStruct *pThisCapStruct, h264CapStruct *pOtherCapStruct, CH264VideoCap *pIntersectH264Cap) const
{
	BYTE bRet = FALSE;

	if (pThisCapStruct->profileValue == pOtherCapStruct->profileValue)
	{
		pIntersectH264Cap->SetProfile(pThisCapStruct->profileValue);
		bRet = TRUE;
	}
	else
	{
		CSmallString str;
		str << "Profiles different. This profile=" << pThisCapStruct->profileValue << " other profile=" << pOtherCapStruct->profileValue;
		PTRACE2(eLevelInfoNormal,"CH264VideoCap::Intersection : ",str.GetString());
	}

	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CH264VideoCap::IntersectCustomParam(BYTE customType, APIU8 intersectLevel,
		APIU8 firstLevel, APIU8 secondLevel,
		APIS32 firstParam, APIS32 secondParam) const
{
	APIS32 intersectResult = -1;
	DWORD firstStandardFs = 0;
	DWORD secondStandardFs = 0;
	DWORD intersectionStandardResult = 0;
	if( (firstParam == -1) && (secondParam == -1) )//both of then are the default for the level
		intersectResult = -1;

	else //at least one isn't the default
	{
		CH264Details firstDetails     = firstLevel;
		CH264Details seconfDetails    = secondLevel;
		CH264Details intersectDetails = intersectLevel;

		if (customType == CUSTOM_MAX_FS_CODE )
		{
			// In this case we will calculate the exact values of FS and not the default devision level
			if (firstParam == -1)
				firstStandardFs = firstDetails.GetDefaultFsAsProduct();
			else
				firstStandardFs = firstDetails.ConvertMaxFsToProduct(firstParam);

			if (secondParam == -1)
				secondStandardFs = seconfDetails.GetDefaultFsAsProduct();
			else
				secondStandardFs = seconfDetails.ConvertMaxFsToProduct(secondParam);

			intersectionStandardResult = min(firstStandardFs, secondStandardFs);
			intersectResult = min(firstParam, secondParam);

			if ( intersectDetails.IsParamSameAsDefaultForLevel(customType, intersectionStandardResult, eCustomProduct) )
				intersectResult = -1;
			else
				intersectResult = GetMaxFsAsDevision(intersectionStandardResult);

		}
		else
		{
			if (firstParam == -1)
				firstParam = firstDetails.GetParamDefaultValueAsDevisionForLevel(customType);

			if (secondParam == -1)
				secondParam = seconfDetails.GetParamDefaultValueAsDevisionForLevel(customType);

			intersectResult = min(firstParam, secondParam);

			if (intersectResult != -1)
				if ( intersectDetails.IsParamSameAsDefaultForLevel(customType, intersectResult, eCustomDevision) )
					intersectResult = -1;
		}
	}

	return intersectResult;
}

///////////////////////////////////////////////////////////////////////////////
CVidModeH323* CH264VideoCap::CreateVidModeThatSupportedByMcu() const
{
	CVidModeH323* pNewMode = NULL;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct->levelValue <= MAX_H264_LEVEL_SUPPORTED_IN_VSW)
	//copy as is
		pNewMode = new CVidModeH323((BaseCapStruct*)pCapStruct, eH264CapCode);

	else if (pCapStruct->levelValue > MAX_H264_LEVEL_SUPPORTED_IN_VSW)
	{//the minimum is the default values for that level
		CH264VideoCap* pNewCapStruct = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, NULL);
		if (pNewCapStruct)
		{
			pNewCapStruct->SetDefaults();
			pNewCapStruct->SetLevel(MAX_H264_LEVEL_SUPPORTED_IN_VSW);
			pNewMode = new CVidModeH323(pNewCapStruct->GetStruct(), eH264CapCode);
			delete pNewCapStruct;
		}
	}

	return pNewMode;
}
///////////////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsCapableOfHD720(ERoleLabel eRole) const
{
	BYTE bRes  = FALSE;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		WORD thisLevel = pCapStruct->levelValue;
		if (thisLevel >= H264_HD720_LEVEL)
			return TRUE;

		CH264Details thisH264Details = thisLevel;
		APIS32 thisFs = pCapStruct->customMaxFsValue;
		if (thisFs == -1)
			thisFs = thisH264Details.GetDefaultFsAsDevision();

		APIS32 thisMbps = pCapStruct->customMaxMbpsValue;
		if (thisMbps == -1)
			thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

		WORD hdMinimumFs   = GetMinimumHd720Fs();
		WORD hdMinimumMbps = 0;
		if(eRole == kRolePresentation)
			hdMinimumMbps = GetMinimumHd720At5Mbps();
		else
			hdMinimumMbps = GetMinimumHd720At15Mbps();
		if ((thisFs >= hdMinimumFs) && (thisMbps >= hdMinimumMbps))
			bRes = TRUE;
	}
	return bRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsCapableOfHD1080() const
{
	BYTE bRes  = FALSE;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		WORD thisLevel = pCapStruct->levelValue;
		if (thisLevel >= H264_HD1080_LEVEL)
			return TRUE;

		CH264Details thisH264Details = thisLevel;
		APIS32 thisFs = pCapStruct->customMaxFsValue;
		if (thisFs == -1)
			thisFs = thisH264Details.GetDefaultFsAsDevision();

		APIS32 thisMbps = pCapStruct->customMaxMbpsValue;
		if (thisMbps == -1)
			thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

		WORD hdMinimumFs   = GetMinimumHd1080Fs();
		WORD hdMinimumMbps = GetMinimumHd1080At15Mbps();

		if ((thisFs >= hdMinimumFs) && (thisMbps >= hdMinimumMbps))
			bRes = TRUE;
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsCapableOfHD720At50() const
{
	BYTE bRes  = FALSE;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		WORD thisLevel = pCapStruct->levelValue;
		if (thisLevel >= H264_Level_3_2)
			return TRUE;

		CH264Details thisH264Details = thisLevel;
		APIS32 thisFs = pCapStruct->customMaxFsValue;
		if (thisFs == -1)
			thisFs = thisH264Details.GetDefaultFsAsDevision();

		APIS32 thisMbps = pCapStruct->customMaxMbpsValue;
		if (thisMbps == -1)
			thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

		WORD hd720MinimumFs   = GetMinimumHd720Fs();
		WORD hd720At50MinimumMbps = GetMinimumHd720At50Mbps();

		if ((thisFs >= hd720MinimumFs) && (thisMbps >= hd720At50MinimumMbps))
			bRes = TRUE;
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsCapableOfHD1080At60() const
{
	BYTE bRes  = FALSE;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		WORD thisLevel = pCapStruct->levelValue;
		if (thisLevel >= H264_Level_4_2)
			return TRUE;

		CH264Details thisH264Details = thisLevel;
		APIS32 thisFs = pCapStruct->customMaxFsValue;
		if (thisFs == -1)
			thisFs = thisH264Details.GetDefaultFsAsDevision();

		APIS32 thisMbps = pCapStruct->customMaxMbpsValue;
		if (thisMbps == -1)
			thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

		WORD hd1080MinimumFs   = GetMinimumHd1080Fs();
		WORD hd1080At60MinimumMbps = GetMinimumHd1080At60Mbps();

		if ((thisFs >= hd1080MinimumFs) && (thisMbps >= hd1080At60MinimumMbps))
			bRes = TRUE;
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsCapableOfHD720At30() const
{
	BYTE bRes  = FALSE;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		WORD thisLevel = pCapStruct->levelValue;
		if (thisLevel >= H264_Level_3_2)
			return TRUE;

		CH264Details thisH264Details = thisLevel;
		APIS32 thisFs = pCapStruct->customMaxFsValue;
		if (thisFs == -1)
			thisFs = thisH264Details.GetDefaultFsAsDevision();

		APIS32 thisMbps = pCapStruct->customMaxMbpsValue;
		if (thisMbps == -1)
			thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

		WORD hd720MinimumFs   = GetMinimumHd720Fs();
		WORD hd720At30MinimumMbps = GetMinimumHd720At30Mbps();

		if ((thisFs >= hd720MinimumFs) && (thisMbps >= hd720At30MinimumMbps))
			bRes = TRUE;
	}
	return bRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CH264VideoCap::GetCPVideoPartyType() const
{
	// this function calling GetVideoPartyTypeMBPSandFS from the caps (local) so it can use without calculating staticMB as we implement it in V4.1
	DWORD staticMB = 0;
	eVideoPartyType videoPartyType = eVideo_party_type_dummy;
	if( GetProfile() == H264_Profile_Main ) //TIP party
		videoPartyType = GetTipVideoPartyTypeAccordingToVideoRate(GetBitRate());
	else
		videoPartyType = GetVideoPartyTypeMBPSandFS(staticMB);
	return videoPartyType;
}

////////////////////////////////////////////////////////////////////////////
eVideoPartyType CH264VideoCap::GetTipVideoPartyTypeAccordingToVideoRate(DWORD videoBitRate) const
{
	if (videoBitRate >= 30000)
		return eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;// when we'll support 1080 for TIP we need to set it to: eHD1080Symmetric;
	else if (videoBitRate >= 936 && videoBitRate < 30000)
		return eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;

	return eVideo_party_type_dummy;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsSupportPLI() const
{
	h264CapStruct *pCapStruct = CAP_CAST(h264CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_PLI);
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsSupportFIR() const
{
	h264CapStruct *pCapStruct = CAP_CAST(h264CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_FIR);
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsSupportTMMBR() const
{
	h264CapStruct *pCapStruct = CAP_CAST(h264CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_TMMBR);

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsSupportNonStandardEncode() const
{
	h264CapStruct *pCapStruct = CAP_CAST(h264CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_IS_NOT_STANDARD_ENCODE);
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetRtcpFeedbackMask() const
{
	APIS32 rtcpFeedbackMask = 0;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		rtcpFeedbackMask = pCapStruct->rtcpFeedbackMask;

	return rtcpFeedbackMask;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH264VideoCap::SetRtcpFeedbackMask(APIS32 rtcpFbMask)
{
	EResult eRes = kFailure;
	h264CapStruct *pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		pCapStruct->rtcpFeedbackMask = rtcpFbMask;
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType	CH264VideoCap::GetVideoPartyTypeMBPSandFS(DWORD staticMB,BYTE IsRsrcByFs) const
{
	APIU8 level = 0;
	long mbps = 0, fs = 0;
	eVideoPartyType videoPartyType = eVideo_party_type_none;

	level = GetLevel();
	mbps  = GetMbps();
	fs    = GetFs();

	if (mbps == -1)
	{
		CH264Details thisH264Details = level;
		mbps = thisH264Details.GetDefaultMbpsAsProduct();
	}
	else
		mbps *= CUSTOM_MAX_MBPS_FACTOR;
	// for resource calculation
	// we add staticMB mode in V4.1, should be change (take staticMB from the structure and not as a variable) when the feature will be fully supported in later versions
	mbps += (staticMB*CUSTOM_MAX_MBPS_FACTOR);

	if (fs == -1)
	{
		CH264Details thisH264Details = level;
		fs = thisH264Details.GetDefaultFsAsProduct();
	}
	else
		fs *= CUSTOM_MAX_FS_FACTOR;

	/*if(NewFRThreshold)
	{
		DWORD CurrentFR = mbps/fs;
		if(CurrentFR < NewFRThreshold)
		{
			newMbps = ((long)fs*NewFRThreshold);

			PTRACE2INT(NORMAL_TRACE, "CH264VideoCap::GetVideoPartyTypeMBPSandFS - Need to calc new MBPS",newMbps);
		}
	}
*/
	cmCapDirection eDirection = GetDirection();

//	if(newMbps && eDirection== cmCapTransmit)
//		videoPartyType = ::GetCPH264ResourceVideoPartyType((DWORD)fs, (DWORD)mbps,IsRsrcByFs);
//	else
		videoPartyType = ::GetCPH264ResourceVideoPartyType((DWORD)fs, (DWORD)mbps,FALSE,IsRsrcByFs);

	CSmallString msg;

	msg << "Direction: " << eDirection<< " fs: " << fs << " mbps: " << mbps 
		<< " IsRsrcByFs: "<< (WORD)IsRsrcByFs << " videoPartyType: " << eVideoPartyTypeNames[videoPartyType];
	PTRACE2(eLevelInfoNormal,"CH264VideoCap::GetVideoPartyTypeMBPSandFS(): ",msg.GetString());

	return videoPartyType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first found value that "other" object has and "this" object is lack of. When such value
//             is found, the function stops checking other values (and the return value will be FALSE)
//             The comparison will be in the following order.
//             The first 4 bits hold value/s :
//             * HIGHER_BIT_RATE   - Other's bit rate is higher.
//             * HIGHER_LEVEL     - Other's level is higher.
//             * HIGHER_MBPS/FS/DPB/BrAndCpb - additional parameter is higher in "other".
//             * DIFFERENT_ROLE    - Different role (people or content)
//             Bits 4-7 of the comparison details are the video format (EFormat) that the
//			   values HIGHER_FORMAT, HIGHER_FRAME_RATE related to.
//             The combination of values to compare can be from kBitRate, kH264Level, kH264Additional
//---------------------------------------------------------------------------------------------------
BYTE CH264VideoCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD* pDetails) const
{
	BYTE bRes = TRUE;
	*pDetails = 0x00000000;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);
	h264CapStruct* pOtherCapStruct = (h264CapStruct *)other.GetStruct();
/*
	TRACEINTO << "@#@";
  COstrStream msg;
  this->Dump(msg);
  TRACEINTO << "CH264VideoCap::IsContaining This Cap \n" << msg.str().c_str();

  COstrStream othermsg;
  other.Dump(othermsg);
  TRACEINTO << "CH264VideoCap::IsContaining other Cap \n" << othermsg.str().c_str();
*/

	if (pCapStruct && pOtherCapStruct)
	{
		// Starts the comparison. If in some stage res gets FALSE the comparison stops
		// and the value that "this" struct was lack of will be saved in the pDetails.
		bRes = CBaseCap::IsContaining(other, valuesToCompare, pDetails);
		if (bRes == FALSE)
			return bRes;

		APIS32 h264Mode = pCapStruct->H264mode;

		if ((valuesToCompare & kH264Mode) && bRes)
		{
			if (pCapStruct->H264mode != pOtherCapStruct->H264mode)
			{
				bRes = FALSE;
				*pDetails |= DIFFERENT_H264MODE;
			}
		}

		if ((h264Mode == H264_tipContent) && (valuesToCompare & kMaxFR) && bRes) //just for TipCompatibility:video&content!
		{
			if (pCapStruct->maxFR < pOtherCapStruct->maxFR)
			{
				bRes = FALSE;
				*pDetails |= HIGHER_MAXFR;
			}
		}

		if (h264Mode != H264_tipContent)
		{
			// If the last comparison did not fail check profile
			if (bRes && (valuesToCompare & kH264Profile))
			{
				if (pCapStruct->profileValue != pOtherCapStruct->profileValue)
				{
					bRes = FALSE;
					*pDetails |= DIFFERENT_PROFILE;
				}
			}

			CH264Details thisH264Details = pCapStruct->levelValue;
			CH264Details otherH264Details = pOtherCapStruct->levelValue;

			// If the last comparison did not fail check level
			if (bRes && (valuesToCompare == kH264Level)) //level is checked only if this is the only thing to check, because customs can upgrade the level.
			{
				if (pCapStruct->levelValue < pOtherCapStruct->levelValue)
				{
					bRes = FALSE;
					*pDetails |= HIGHER_LEVEL;
				}
			}

			// If the last comparison did not fail check additional parameters
			if (bRes && (valuesToCompare & kH264Additional_MBPS))
			{ //MBPS:
				if ((pCapStruct->customMaxMbpsValue != -1) || (pOtherCapStruct->customMaxMbpsValue != -1))
				{
					APIS32 thisMpbs = pCapStruct->customMaxMbpsValue;
					if (thisMpbs == -1)
						thisMpbs = thisH264Details.GetDefaultMbpsAsDevision();

					APIS32 otherMpbs = pOtherCapStruct->customMaxMbpsValue;
					if (otherMpbs == -1)
						otherMpbs = otherH264Details.GetDefaultMbpsAsDevision();
					if (thisMpbs < otherMpbs)
					{
						bRes = FALSE;
						*pDetails |= HIGHER_MBPS;
					}
				}
				else if (pCapStruct->levelValue < pOtherCapStruct->levelValue) //if both are -1: check agains level:
				{ //means that also this mbps < other mbps
				  //PTRACE2INT(eLevelInfoNormal,"CH264VideoCap::IsContaining level is ",pCapStruct->levelValue);
					if (!((pCapStruct->levelValue == 36 || pCapStruct->levelValue == 43) && (pOtherCapStruct->levelValue == 36 || pOtherCapStruct->levelValue == 43)))
					{
						bRes = FALSE;
						*pDetails |= HIGHER_MBPS;
					}
				}
			}

			if (bRes && (valuesToCompare & kH264Additional_FS))
			{ //FS:
				if ((pCapStruct->customMaxFsValue != -1) || (pOtherCapStruct->customMaxFsValue != -1))
				{
					APIS32 thisFs = pCapStruct->customMaxFsValue;
					if (thisFs == -1)
						thisFs = thisH264Details.GetDefaultFsAsDevision();

					APIS32 otherFs = pOtherCapStruct->customMaxFsValue;
					if (otherFs == -1)
						otherFs = otherH264Details.GetDefaultFsAsDevision();

					if (thisFs < otherFs)
					{
						bRes = FALSE;
						*pDetails |= HIGHER_FS;
					}
				}
				else if (pCapStruct->levelValue < pOtherCapStruct->levelValue) //if both are -1: check agains level:
				{ //means that also this fs < other fs
					if (!((pCapStruct->levelValue == 36 || pCapStruct->levelValue == 43) && (pOtherCapStruct->levelValue == 36 || pOtherCapStruct->levelValue == 43)))
					{
						bRes = FALSE;
						*pDetails |= HIGHER_FS;
					}
				}
			}

			if (bRes && (valuesToCompare & kH264Additional_DPB))
			{ //DPB:
				if ((pCapStruct->customMaxDpbValue != -1) || (pOtherCapStruct->customMaxDpbValue != -1))
				{
					APIS32 thisDpb = pCapStruct->customMaxDpbValue;
					if (thisDpb == -1)
						thisDpb = thisH264Details.GetDefaultDpbAsDevision();

					APIS32 otherDpb = pOtherCapStruct->customMaxDpbValue;
					if (otherDpb == -1)
						otherDpb = otherH264Details.GetDefaultDpbAsDevision();

					if (thisDpb < otherDpb)
					{
						bRes = FALSE;
						*pDetails |= HIGHER_DPB;
					}
				}
				else if (pCapStruct->levelValue < pOtherCapStruct->levelValue) //if both are -1: check agains level:
				{ //means that also this dpb < other dpb
					bRes = FALSE;
					*pDetails |= HIGHER_DPB;
				}
			}

			if (bRes && (valuesToCompare & kH264Additional_BR_AND_CPB))
			{ //BR & CPB:
				if ((pCapStruct->customMaxBrAndCpbValue != -1) || (pOtherCapStruct->customMaxBrAndCpbValue != -1))
				{ //BR:
					APIS32 thisBr = pCapStruct->customMaxBrAndCpbValue; //without the product, maxBrAndCpb = Br
					if (thisBr == -1)
						thisBr = thisH264Details.GetDefaultBrAsDevision();

					APIS32 otherBr = pOtherCapStruct->customMaxBrAndCpbValue;
					if (otherBr == -1)
						otherBr = otherH264Details.GetDefaultBrAsDevision();

					if (thisBr < otherBr)
						bRes = FALSE;
					if (bRes == FALSE)
						*pDetails |= HIGHER_BR_AND_CPB;
				}
				else if (pCapStruct->levelValue < pOtherCapStruct->levelValue) //if both are -1: check agains level:
				{ //means that also this br < other br
					bRes = FALSE;
					*pDetails |= HIGHER_BR_AND_CPB;
				}
			}

			APIS32 toleraceRatePct = GetToleraceRatePct(pOtherCapStruct->maxBitRate);
			TRACEINTO << "ToleraceBitRate:" << toleraceRatePct << ", MaxBitRate:" << pOtherCapStruct->maxBitRate;
			if ((valuesToCompare & kBitRate) && bRes)
			{
				if (pCapStruct->maxBitRate < toleraceRatePct)
				{
					bRes = FALSE;
					*pDetails |= HIGHER_BIT_RATE;
				}
			}
			if ((valuesToCompare & kBitRateForCascade) && bRes)
			{
				if (pCapStruct->maxBitRate < (pOtherCapStruct->maxBitRate * CASCADE_BW_FACTOR) / 100)
				{
					bRes = FALSE;
					*pDetails |= HIGHER_BIT_RATE;
				}
			}
			if ((valuesToCompare & kBitRateWithoutTolerance) && bRes)
			{
				if (pCapStruct->maxBitRate < pOtherCapStruct->maxBitRate)
				{
					bRes = FALSE;
					*pDetails |= HIGHER_BIT_RATE;
				}
			}
			if ((valuesToCompare & kPacketizationMode) && bRes)
			{
				if (CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw)
				{
					if (pCapStruct->packetizationMode != pOtherCapStruct->packetizationMode)
					{
						bRes = FALSE;
						*pDetails |= DIFF_PACKETIZATION_MODE;
					}
				}
			}
		}
	}
	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}
//////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsSameProfile(const CBaseCap& other) const
{
	BYTE bRes  = FALSE;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);
	h264CapStruct* pOtherCapStruct = (h264CapStruct *)other.GetStruct();
	if (pCapStruct && pOtherCapStruct)
	{
		if (pCapStruct->profileValue == pOtherCapStruct->profileValue)
		{
			bRes = TRUE;
		}
	}
	return bRes;

}



/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH264VideoCap::SetBitRate(APIS32 rate)
{
	EResult eRes = kFailure;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxBitRate = rate;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH264VideoCap::SetMaxFR(APIS32 maxfr)
{
	EResult eRes = kFailure;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxFR = maxfr;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH264VideoCap::SetH264mode(APIS32 h264mode)
{
	EResult eRes = kFailure;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		pCapStruct->H264mode = h264mode;
		eRes = kSuccess;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EFormat CH264VideoCap::GetFormat() const
{
	APIS32 fs = GetFs();

	if(fs == -1)
	{
		CH264Details details     = GetLevel();
		fs = details.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_FS_CODE);
	}
	else
		fs = fs * CUSTOM_MAX_FS_FACTOR;

	EFormat format = GetFormatAccordingToFS(fs);

	return format;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS8 CH264VideoCap::GetFormatMpi(EFormat eFormat) const
{
	return GetH264Mpi();
}

///////////////////////////////////////////////////////////////////////////////
//APIS8 CH264VideoCap::GetH264Mpi() const (vngfe5040):
APIS8 CH264VideoCap::GetH264Mpi(const APIS32 MaxFSForFrameRate) const
{
	APIS32 			mbps 	 = GetMbps();
	APIS32 			fs 		 = GetFs();
	CH264Details 	details  = GetLevel();
	APIS16   	    frameRate = 0;

	//Frame Size
	if (fs == -1)
		fs = details.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_FS_CODE);
	else
		fs *= CUSTOM_MAX_FS_FACTOR;
	GetExactFS(fs);
	//MacroBlock Processing per Second
	if(mbps == -1)
		mbps = details.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_MBPS_CODE);
	else
		mbps *= CUSTOM_MAX_MBPS_FACTOR;
	//multiplied by 10 to hold halfs
    //if (fs == -1 || fs == 0 || ((frameRate = mbps * 10 / fs) == 0)) (vngfe5040):
	if (fs == -1 || fs == 0 || ((frameRate = mbps * 10 / min(fs, MaxFSForFrameRate)) == 0))
			return -1;
	APIS8 mpi = RoundFrameRateAndTranslateToMpi(frameRate);
	return mpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::GetExactFS(APIS32& fs) const
{
	if	    ( 99    <= fs && fs < 396 )
		fs = 99;
	else if	( 396   <= fs && fs < 792 )
		fs = 396;
	else if	( 792   <= fs && fs < 1620 )
		fs = 792;
	else if	( 1620  <= fs && fs < 3600 )
		fs = 1620;
	else if	( 3600  <= fs && fs < 5120 )
		fs = 3600;
	else if	( 5120  <= fs && fs < 8192 )
		fs = 5120;
	else if	( 8192  <= fs && fs < 22080 )
		fs = 8192;
	else if	( 22080 <= fs && fs < 36864 )
		fs = 22080;
	else
		fs = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS8 CH264VideoCap::RoundFrameRateAndTranslateToMpi(APIS16 frameRate) const
{
	//rounding
	frameRate += 10;

	// 60   fps
	if( 600 <= frameRate )
		frameRate = 600;
	// 50   fps
	else if( 500 <= frameRate && frameRate < 600 )
		frameRate = 500;
	// 30   fps
	else if( 300 <= frameRate && frameRate < 500 )
		frameRate = 300;
	// 25   fps
	else if( 250 <= frameRate && frameRate < 300 )
		frameRate = 250;
	// 15   fps
	else if( 150 <= frameRate && frameRate < 250 )
		frameRate = 150;
	// 12.5 fps
	else if( 125 <= frameRate && frameRate < 150 )
		frameRate = 125;
	// 10   fps
	else if( 100 <= frameRate && frameRate < 125 )
		frameRate = 100;
	// 7.5  fps
	else if(  75 <= frameRate && frameRate < 100 )
		frameRate =  75;
	// 6    fps
	else if(  60 <= frameRate && frameRate <  75 )
		frameRate =  60;
	// 5    fps
	else if(  50 <= frameRate && frameRate <  60 )
		frameRate =  50;
	// 3    fps
	else if(  30 <= frameRate && frameRate <  50 )
		frameRate =  30;
	// wrong value
	else
		frameRate = -1;

	// The MPI concept is changed for supporting 60fps:
	// ->  MPI 1 = 60fps; MPI 2 = 30 fps; MPI 4 = 15fps; MPI 10 = 5fps
	APIS8 mpi = 600/frameRate;
	//PTRACE2INT(eLevelInfoNormal,"CH264VideoCap::RoundFrameRateAndTranslateToMpi mpi",mpi);
	return mpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetBitRate() const
{
	APIS32 bitRate = -1;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		bitRate = pCapStruct->maxBitRate;

	return bitRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetMaxFR() const
{
	APIS32 maxfr = -1;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		maxfr = pCapStruct->maxFR;

	return maxfr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetH264mode() const
{
	APIS32 h264mode = H264_standard;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		h264mode = pCapStruct->H264mode;

	return h264mode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CH264VideoCap::GetProfile() const
{
	APIU16 profileValue = 0;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		profileValue = pCapStruct->profileValue;

	return profileValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetProfile(APIU16 profileValue)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct && profileValue!=H264_Profile_None)
		pCapStruct->profileValue = profileValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CH264VideoCap::GetLevel() const
{
	APIU8 levelValue = 0;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		levelValue = pCapStruct->levelValue;

	return levelValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetLevel(APIU8 levelValue)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		pCapStruct->levelValue = levelValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetMbps() const
{
	APIS32 customMaxMbpsValue = 0;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		customMaxMbpsValue = pCapStruct->customMaxMbpsValue;

	return customMaxMbpsValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetMbps(APIS32 mbps)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		pCapStruct->customMaxMbpsValue = mbps;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetFs() const
{
	APIS32 customMaxFsValue = -1;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		customMaxFsValue = pCapStruct->customMaxFsValue;

	return customMaxFsValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetFs(APIS32 fs)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		pCapStruct->customMaxFsValue = fs;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetDpb() const
{
	APIS32 customMaxDpbValue = 0;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		customMaxDpbValue = pCapStruct->customMaxDpbValue;

	return customMaxDpbValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetDpb(APIS32 dpb)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		pCapStruct->customMaxDpbValue = dpb;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetBrAndCpb() const
{
	APIS32 customMaxBrAndCpbValue = 0;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		customMaxBrAndCpbValue = pCapStruct->customMaxBrAndCpbValue;

	return customMaxBrAndCpbValue;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetBrAndCpb(APIS32 brAndCpb)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		pCapStruct->customMaxBrAndCpbValue = brAndCpb;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetSampleAspectRatio() const
{
	APIS32 sampleAspectRatio = 0;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		sampleAspectRatio = pCapStruct->sampleAspectRatiosValue;

	return sampleAspectRatio;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetSampleAspectRatio(APIS32 sar)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		pCapStruct->sampleAspectRatiosValue = sar;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH264VideoCap::GetStaticMB() const
{
	APIS32 maxStaticMbpsValue = 0;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		maxStaticMbpsValue = pCapStruct->maxStaticMbpsValue;

	return maxStaticMbpsValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetStaticMB(APIS32 staticMB)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		pCapStruct->maxStaticMbpsValue = staticMB;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::GetAdditionals(APIS32& mbps, APIS32& fs, APIS32& dpb ,APIS32& brAndCpb, APIS32& sar, APIS32& staticMB)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		mbps	 = pCapStruct->customMaxMbpsValue;
		fs		 = pCapStruct->customMaxFsValue;
		dpb      = pCapStruct->customMaxDpbValue;
		brAndCpb = pCapStruct->customMaxBrAndCpbValue;
		sar		 = pCapStruct->sampleAspectRatiosValue;
		staticMB = pCapStruct->maxStaticMbpsValue;
	}
	else
		 mbps = fs = dpb = brAndCpb = sar = staticMB = -1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::GetAdditionalsMbpsFsAsExplicit(DWORD& mbps, DWORD& fs)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);
	mbps = fs =(DWORD) -1;
	if (pCapStruct)
	{

		CH264Details h264Details = pCapStruct->levelValue;
		mbps = pCapStruct->customMaxMbpsValue;
		if (mbps == (DWORD)-1)
			mbps = h264Details.GetDefaultMbpsAsProduct();
		else
			mbps *= CUSTOM_MAX_MBPS_FACTOR;

		fs = pCapStruct->customMaxFsValue;
		if (fs == (DWORD)-1)
			fs = h264Details.GetDefaultFsAsProduct();
		else
			fs *= CUSTOM_MAX_FS_FACTOR;

	}

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::GetAdditionalsAsExplicit(APIS32& mbps, APIS32& fs, APIS32& dpb ,APIS32& brAndCpb, APIS32& sar, APIS32& staticMB)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		CH264Details h264Details = pCapStruct->levelValue;
		mbps = pCapStruct->customMaxMbpsValue;
		if (mbps == -1)
			mbps = h264Details.GetDefaultMbpsAsDevision();

		fs = pCapStruct->customMaxFsValue;
		if (fs == -1)
			fs = h264Details.GetDefaultFsAsDevision();

		dpb = pCapStruct->customMaxDpbValue;
		if (dpb == -1)
			dpb = h264Details.GetDefaultDpbAsDevision();

		brAndCpb = pCapStruct->customMaxBrAndCpbValue;
		if (brAndCpb == -1)
			brAndCpb = h264Details.GetDefaultBrAsDevision();

		sar = pCapStruct->sampleAspectRatiosValue;
		staticMB = pCapStruct->maxStaticMbpsValue;
	}
	else
		 mbps = fs = dpb = brAndCpb = sar = staticMB = -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetLevelAndAdditionals(APIU16 profile, APIU8 level, APIS32 mbps, APIS32 fs, APIS32 dpb ,APIS32 brAndCpb, APIS32 sar, APIS32 staticMB)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		if (profile!=H264_Profile_None) // When profile is H264_Profile_None, save the exist profile.
			pCapStruct->profileValue	   = profile;
		pCapStruct->levelValue			   = level;
		pCapStruct->customMaxMbpsValue	   = mbps;
		pCapStruct->customMaxFsValue	   = fs;
		pCapStruct->customMaxDpbValue      = dpb;
		pCapStruct->customMaxBrAndCpbValue = brAndCpb;
		pCapStruct->sampleAspectRatiosValue = sar;
		pCapStruct->maxStaticMbpsValue = staticMB;
		//pCapStruct->packetizationMode = (IsFeatureSupportedBySystem(eFeatureH264PacketizationMode)) ? 1 : 0; --> removed as part of packetizationMode=0 support as additional capCode.
	}
}

void CH264VideoCap::SetAccordingToOperationPoint(const VideoOperationPoint &operationPoint, bool shouldUpdateProfile)
{
    APIU8 level;
    long mbps, fs;
    long opBitRate = operationPoint.m_maxBitRate*10;

    mbps = CalcMBPSforVswRelay(operationPoint);
    fs = CalcFSforVswRelay(operationPoint);
    ProfileToLevelTranslator plt;
    level = plt.ConvertResolutionAndRateToLevelEx(fs, mbps);

    SetDefaults(cmCapReceive);
    SetBitRate(opBitRate);
    SetLevel(level);
    SetMbps(mbps);
    SetFs(fs);
    if (shouldUpdateProfile)
    	SetProfile(ProfileToLevelTranslator::SvcProfileToH264(operationPoint.m_videoProfile));
}

void CH264VideoCap::InitAccordingToOperationPoint(const VideoOperationPoint &operationPoint, bool shouldUpdateProfile)
{
    SetDefaults(cmCapReceive);
    SetAccordingToOperationPoint(operationPoint, shouldUpdateProfile);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::RemoveDefaultAdditionals()
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
	{
		CH264Details h264Details = pCapStruct->levelValue;
		//in the structure it is in devision, and in the table it is in multiple
		if (h264Details.IsParamSameAsDefaultForLevel(CUSTOM_MAX_MBPS_CODE, ((unsigned long)pCapStruct->customMaxMbpsValue)*CUSTOM_MAX_MBPS_FACTOR, eCustomProduct))
			pCapStruct->customMaxMbpsValue = -1;

		if (h264Details.IsParamSameAsDefaultForLevel(CUSTOM_MAX_FS_CODE, ((unsigned long)pCapStruct->customMaxFsValue)*CUSTOM_MAX_FS_FACTOR, eCustomProduct))
			pCapStruct->customMaxFsValue   = -1;

		if (h264Details.IsParamSameAsDefaultForLevel(CUSTOM_MAX_DPB_CODE, (unsigned long)pCapStruct->customMaxDpbValue, eCustomDevision))
			pCapStruct->customMaxDpbValue  = -1;

		//DWORD structureMaxBr = (unsigned long)pCapStruct->customMaxBrAndCpbValue;//without the product, maxBrAndCpb = Br
		if (h264Details.IsParamSameAsDefaultForLevel(CUSTOM_MAX_BR_CODE, (unsigned long)pCapStruct->customMaxBrAndCpbValue/*structureMaxBr*/, eCustomDevision))
			pCapStruct->customMaxBrAndCpbValue = -1;



	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: The function gets H264 level and additional parameters, and return if this
// capability struct has higher values.
// Higher values for H264 is:
//		-	higher level
//      -   same level but higher additional parameters.

BYTE CH264VideoCap::IsThisCapBetter(APIU16* maxProfile, APIU8* maxLevel, APIS32* maxMbps, APIS32* maxFs, APIS32* maxDpb, APIS32* maxBrAndCpb, BYTE bIgnoreHighProfileCap) const
{
	BYTE bRes = FALSE;

	APIU16 curProfile = GetProfile();
	if (bIgnoreHighProfileCap && (curProfile != H264_Profile_BaseLine)) //we want to take the BaseLine
			return FALSE;
	if ((curProfile == H264_Profile_High) && (*maxProfile == H264_Profile_BaseLine))
	{
		*maxProfile = H264_Profile_High;
		bRes = TRUE;
		// if set to the better caps profile then have to set all its parameters as well.
		*maxLevel = GetLevel();
		*maxMbps = GetMbps();
		*maxFs = GetFs();
		*maxDpb = GetDpb();
		*maxBrAndCpb = GetBrAndCpb();
	}
	else
	{

		APIU8 curLevel = GetLevel();
		if (curLevel > *maxLevel)
		{
			*maxLevel = curLevel;
			bRes = TRUE;

			// if set to the better caps level then have to set all its parameters as well.
			*maxMbps = GetMbps();
			*maxFs = GetFs();
			*maxDpb = GetDpb();
			*maxBrAndCpb = GetBrAndCpb();
		}

		else if (curLevel == *maxLevel)
		{//According to the standard, if the additional parameters exist, they must be higher than
		//the default. Therefore, I didn't check if the current additional parameter >-1
			APIS32 currMbps = GetMbps();
			if (currMbps > *maxMbps)
			{
				*maxMbps = currMbps;
				bRes = TRUE;
			}

			APIS32 currFs = GetFs();
			if (currFs > *maxFs)
			{
				*maxFs = currFs;
				bRes = TRUE;
			}

			APIS32 currDpb = GetDpb();
			if (currDpb > *maxDpb)
			{
				*maxDpb = currDpb;
				bRes = TRUE;
			}

			APIS32 currBrAndCpb = GetBrAndCpb();
			if (currBrAndCpb > *maxBrAndCpb)
			{
				*maxBrAndCpb = currBrAndCpb;
				bRes = TRUE;
			}
		}
	}

	return bRes;
}

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VideoCap::IsHighestCommonConditionSupported(CHighestCommonCondition& hcCondition) const
{
	if (hcCondition.GetVideoProtocol(H323_INTERFACE_TYPE) != eH264CapCode)
		return FALSE;

	BYTE bRes = FALSE;
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);
    if (pCapStruct)
	{
		CHighestCommonConditionH264 hcConditionH264 = (CHighestCommonConditionH264&)hcCondition;
		BYTE hcLevel = hcConditionH264.GetLevel();
		WORD hcMBPS, hcFS, hcDPB, hcBRandCPB;
		hcConditionH264.GetCustomParams(hcMBPS, hcFS, hcDPB, hcBRandCPB);

		CH264VideoCap* pHcStruct = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, NULL);
		pHcStruct->SetDefaults();
		pHcStruct->SetLevelAndAdditionals(hcLevel, hcMBPS, hcFS, hcDPB, hcBRandCPB);

		WORD tempDetails;
		bRes = IsContaining(*pHcStruct, kH264Level|kH264Additional, &tempDetails);
		POBJDELETE(pHcStruct);
	}
	return bRes;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::GetMediaParams(CSecondaryParams &secParams, DWORD details) const
{//In h264, we don't fill all the params, just the cap code and the current param
	CCapSetInfo capInfo = eH264CapCode;
	secParams.m_capCode	= (CapEnum)capInfo.GetIpCapCode();
	secParams.m_lineRate	= GetBitRate() / 10;

	APIU32 problemParam = GetH264DiffFromDetails(details, secParams.m_currProblemValue);
	secParams.m_problemParam = problemParam;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::GetDiffFromDetails(DWORD details, CSecondaryParams &secParams)
{
	APIU32 problemParam = GetH264DiffFromDetails(details, secParams.m_rmtProblemValue);
	secParams.m_problemParam = problemParam;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
ESecondaryParam CH264VideoCap::GetH264DiffFromDetails(DWORD details, DWORD& problemValue) const
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	CH264Details h264Details = pCapStruct->levelValue;

	ESecondaryParam problemParam = UnKnown;

	switch (details)
	{
		case HIGHER_BIT_RATE:
			problemParam	= LineRate;
			problemValue	= GetBitRate() / 10;
			break;
		case DIFFERENT_ROLE:
			problemParam	= RoleLabel;
			problemValue	= m_pCapStruct->header.roleLabel;
			break;
		case DIFFERENT_CAPCODE:
			{
				CCapSetInfo capInfo = (CapEnum)m_pCapStruct->header.capTypeCode;
				problemParam	= CapCode;
				problemValue	= capInfo.GetIpCapCode();
			}
			break;
		case HIGHER_LEVEL:
			problemParam	= Level;
			problemValue	= pCapStruct->levelValue;
			break;
		case HIGHER_MBPS:
			problemParam	= MBPS;
			problemValue	= pCapStruct->customMaxMbpsValue;
			if (problemValue == (DWORD)-1)
				problemValue = h264Details.GetDefaultMbpsAsProduct();
			break;
		case HIGHER_FS:
			problemParam	= FS;
			problemValue	= pCapStruct->customMaxFsValue;
			if (problemValue == (DWORD)-1)
				problemValue = h264Details.GetDefaultFsAsProduct();
			break;
		case HIGHER_DPB:
			problemParam	= DPB;
			problemValue	= pCapStruct->customMaxDpbValue;
			if (problemValue == (DWORD)-1)
				problemValue = h264Details.GetDefaultDpbAsProduct();
			break;
		case HIGHER_BR_AND_CPB:
			problemParam	= BrAndCpb;
			problemValue	= pCapStruct->customMaxBrAndCpbValue;
			if (problemValue == (DWORD)-1)
			{
				DWORD brAndCpb = h264Details.GetDefaultBrAsProduct();
				problemValue = ConvertMaxBrToMaxBrAndCpb(brAndCpb);
			}
			break;
		case DIFF_PACKETIZATION_MODE:
			problemParam	= PacketizationMode;
			problemValue	= 0;
			break;
		default:
			problemParam	= Level;
			problemValue	= pCapStruct->levelValue;
			break;
	}
	return problemParam;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// TIP
APIU8 CH264VideoCap::GetPacketizationMode() const
{
	APIU8 packetizationMode = 1;

	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		packetizationMode = pCapStruct->packetizationMode;

	return packetizationMode;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VideoCap::SetPacketizationMode(APIU8 packetizationMode)
{
	h264CapStruct* pCapStruct = CAP_CAST(h264CapStruct);

	if (pCapStruct)
		pCapStruct->packetizationMode = packetizationMode;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CH264VideoCap::IsH264Video(CapEnum capCode)
{
	if (capCode == (CapEnum)eH264CapCode || capCode == (CapEnum)eSvcCapCode)
	{
		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

//N.A. DEBUG VP8
/////////////////////////////////////////////////////////////////////////////////////////////////////
//							 	 Start VP8VideoCap Function										  //
///////////////////////////////////////////////////////////////////////////////////////////////////


//------------------------------------
// Description: Returns the size of struct
//-----------------------------------

size_t CVP8VideoCap::SizeOf() const
{
	return (m_pCapStruct) ? sizeof(vp8CapStruct) : 0 ;
}

//------------------------------------
// Description: Allocates a new struct
//------------------------------------
void    CVP8VideoCap::AllocStruct(size_t size)
{

	if (size && (size > sizeof(vp8CapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(vp8CapStruct));
}

//------------------------------------
// Description: Set Default values for struct fields
// Return Value: success/failure result
//------------------------------------
EResult CVP8VideoCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	PTRACE(eLevelError,"CVP8VideoCap::SetDefaults");
	EResult eRes = kFailure;
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);

		pCapStruct->maxFR		= 30;
		pCapStruct->maxFS		= 1280;
		pCapStruct->maxBitRate	= 1920;

		if(!pCapStruct->rtcpFeedbackMask)
			pCapStruct->rtcpFeedbackMask            = (RTCP_MASK_FIR | RTCP_MASK_TMMBR);

		
		//Set the rest of the fields once we have them N.A.
	}
	return eRes;
}

//------------------------------------
// Description: does =
// Return Value: success/failure result
//------------------------------------
EResult CVP8VideoCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	vp8CapStruct *pCapStruct = CAP_CAST(vp8CapStruct);
	CVP8VideoCap &hOtherVideoCap  = (CVP8VideoCap&)otherCap;

	if (GetCapCode() == hOtherVideoCap.GetCapCode())
	{
		eRes = kSuccess;
		pCapStruct->maxFR				= hOtherVideoCap.GetMaxFR();
		pCapStruct->maxFS				= hOtherVideoCap.GetMaxFS();
		pCapStruct->maxBitRate  		= hOtherVideoCap.GetMaxFS();
		pCapStruct->rtcpFeedbackMask	= hOtherVideoCap.GetRtcpFeedbackMask();
		//Add  the rest of fields once we have them N.A.
	}
	return eRes;
}
//------------------------------------
// Description: Prints the object values into a stream
// Return Value: None
//------------------------------------
void CVP8VideoCap::Dump(std::ostream& msg) const
{

	/*

	 * typedef struct{
		ctCapStruct			header;
		APIS32				maxFR;
		APIS32				maxFS;
		APIS32              maxBitRate;

	 * */

	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		//msg << "\nCVP8VideoCap::Dump" ;

		if (pCapStruct->maxFR >= 0)
			msg << "maxFR                   = " <<pCapStruct->maxFR  <<" (" << (pCapStruct->maxFR) << " frame rate per second)" << "\n";
		else
			msg << "Unknown maxFR"  << "\n";

		if (pCapStruct->maxFS >= 0)
			msg << "maxFS                   = " <<pCapStruct->maxFS << "\n";
		else
			msg << "Unknown maxFS"  << "\n";

		if (pCapStruct->maxBitRate >= 0)
			msg << "maxBitRate              = " <<pCapStruct->maxBitRate << "\n";
		else
			msg << "Unknown maxBitRate"  << "\n";

		if (pCapStruct->rtcpFeedbackMask >= 0)
			msg << "rtcpFeedbackMask        = " <<pCapStruct->rtcpFeedbackMask << "\n";
		else
			msg << "Unknown rtcpFeedbackMask"  << "\n";
	}
}




//------------------------------------
// Sets...
//------------------------------------
EResult CVP8VideoCap::SetMaxFR(APIS32 maxfr)
{
	EResult eRes = kFailure;
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxFR = maxfr;
		PTRACE2INT(eLevelInfoNormal, "CVP8VideoCap::SetMaxFS pCapStruct->maxFR = ", pCapStruct->maxFR);
		eRes = kSuccess;
	}
	return eRes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EResult CVP8VideoCap::SetMaxFS(APIS32 maxfs)
{
	EResult eRes = kFailure;
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxFS = maxfs;
		PTRACE2INT(eLevelInfoNormal, "CVP8VideoCap::SetMaxFS pCapStruct->maxFS = ", pCapStruct->maxFS);
		eRes = kSuccess;
	}
	return eRes;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EResult CVP8VideoCap::SetBitRate(APIS32 rate)
{
	EResult eRes = kFailure;
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxBitRate = rate;
		eRes = kSuccess;
		PTRACE2INT(eLevelInfoNormal, "CVP8VideoCap::SetBitRate pCapStruct->maxBitRate = ", pCapStruct->maxBitRate);
	}

	return eRes;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EResult CVP8VideoCap::SetRtcpFeedbackMask(APIS32 rtcpFbMask)
{
	EResult eRes = kFailure;
	vp8CapStruct *pCapStruct = CAP_CAST(vp8CapStruct);

	if (pCapStruct)
	{
		pCapStruct->rtcpFeedbackMask = rtcpFbMask;
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------
// Gets...
//------------------------------------

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
APIS32 CVP8VideoCap::GetMaxFR() const
{
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	return (pCapStruct) ? pCapStruct->maxFR : -1 ;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
APIS32 CVP8VideoCap::GetMaxFS() const
{
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	return (pCapStruct) ? pCapStruct->maxFS : -1;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
APIS32 CVP8VideoCap::GetMaxBitRate() const
{
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	return (pCapStruct) ? pCapStruct->maxBitRate : 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
APIS32 CVP8VideoCap::GetBitRate() const
{
	APIS32 bitRate = -1;
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	if (pCapStruct)
		bitRate = pCapStruct->maxBitRate;

	PTRACE2INT(eLevelInfoNormal, "CVP8VideoCap::GetBitRate bitRate = ", bitRate);
	return bitRate;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
APIS32 CVP8VideoCap::GetRtcpFeedbackMask() const
{
	APIS32 rtcpFeedbackMask = 0;
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	if (pCapStruct)
		rtcpFeedbackMask = pCapStruct->rtcpFeedbackMask;

	return rtcpFeedbackMask;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CVP8VideoCap::SetContent(ERoleLabel eRole, cmCapDirection eDirection,BOOL isHD1080, BYTE HDMpi, BOOL isHighProfile)
{
	EResult eRes = kFailure;

	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		SetDefaults(eDirection, eRole);
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);

	}
	return eRes;
}

//------------------------------------
// Description: Intersection
// Return Value: success/failure result
//------------------------------------
BYTE CVP8VideoCap::Intersection(const CBaseCap& other, BYTE** ppIntersectData, BYTE comparePacketizationMode) const
{
	PTRACE(eLevelInfoNormal,"CVP8VideoCap::Intersection");
	//We need to do a Intersection and we want all the resolutions that initialize to take with MPi that
	//fit to both sides
	//We will start from the bigger resolution and save it's MPI and compare, we will take the maximum MPI.

	BYTE bIsSuccess = FALSE;
	const CVP8VideoCap *pOtherCap = (const CVP8VideoCap *)(&other);

	vp8CapStruct *pThisCapStruct	 = CAP_CAST(vp8CapStruct);
	vp8CapStruct *pOtherCapStruct	 = (vp8CapStruct *)other.GetStruct();

	vp8CapStruct *pIntersectStuct	 = (vp8CapStruct *)*ppIntersectData;
	CVP8VideoCap *pIntersectVP8Cap = (CVP8VideoCap*)CBaseCap::AllocNewCap(eVP8CapCode, pIntersectStuct);
	if(pIntersectVP8Cap == NULL)
	{
		PTRACE(eLevelInfoNormal, "CVP8VideoCap::Intersection -  pIntersectH264Cap is NULL");
		return FALSE;
	}
	pIntersectVP8Cap->SetDefaults();



	if (pOtherCap && pThisCapStruct && pOtherCapStruct)
	{

		if (CProcessBase::GetProcess()->GetProductType()== eProductTypeSoftMCU)
		{

			//FR
			APIS32 thisFr  = pThisCapStruct->maxFR;
			APIS32 otherFr = pOtherCapStruct->maxFR;
			APIS32 intersectFr = IntersectVP8CustomParam(CUSTOM_MAX_MBPS_CODE, thisFr, otherFr);
			pIntersectVP8Cap->SetMaxFR(intersectFr);

			//FS
			APIS32 thisFs  = pThisCapStruct->maxFS;
			APIS32 otherFs = pOtherCapStruct->maxFS;
			APIS32 intersectFs = IntersectVP8CustomParam(CUSTOM_MAX_FS_CODE, thisFs, otherFs);
			pIntersectVP8Cap->SetMaxFS(intersectFs);

			((CBaseVideoCap *)pIntersectVP8Cap)->SetRtcpFeedbackMask(GetRtcpFeedbackMask() & pOtherCapStruct->rtcpFeedbackMask );
		}


	POBJDELETE(pIntersectVP8Cap);
	return bIsSuccess;
	}
	return bIsSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CVP8VideoCap::IntersectVP8CustomParam(BYTE customType, APIS32 firstParam, APIS32 secondParam) const
{
	APIS32 intersectResult = -1;
	DWORD firstStandardFs = 0;
	DWORD secondStandardFs = 0;
	if( (firstParam == -1) && (secondParam == -1) )//both of then are the default for the level
		intersectResult = -1;
	else //at least one isn't the default
	{
		intersectResult = min(firstParam, secondParam);
	}

	return intersectResult;
}

///////////////////////////////////////////////////////////////////////////////
eVideoPartyType CVP8VideoCap::GetCPVideoPartyType() const
{

	// this function calling GetVideoPartyTypeMBPSandFS from the caps (local) so it can use without calculating staticMB as we implement it in V4.1
	DWORD staticMB = 0;
	eVideoPartyType videoPartyType = eVideo_party_type_dummy;

	videoPartyType = GetVideoPartyTypeFRandFS(staticMB);

	return videoPartyType;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType	CVP8VideoCap::GetVideoPartyTypeFRandFS(DWORD staticMB,BYTE IsRsrcByFs) const
{
	long mbps = 0, fs = 0, fr = 0;
	eVideoPartyType videoPartyType = eVideo_party_type_none;

	fr	  = GetMaxFR();
	fs    = GetMaxFS();

	mbps = (fs * fr);

	//fs *= CUSTOM_MAX_FS_FACTOR; //N.A. DEBUG - depends on the units we get from the EP - may need to remove

	//mbps *= CUSTOM_MAX_MBPS_FACTOR; //N.A. DEBUG - depends on the units we get from the EP - may need to remove
		// for resource calculation
		// we add staticMB mode in V4.1, should be change (take staticMB from the structure and not as a variable) when the feature will be fully supported in later versions
	//	mbps += (staticMB*CUSTOM_MAX_MBPS_FACTOR);


	cmCapDirection eDirection = GetDirection();
	videoPartyType = ::GetCPHVP8ResourceVideoPartyType((DWORD)fs, (DWORD)mbps, fr, IsRsrcByFs);

	CSmallString msg;

	msg << "Direction: " << eDirection<< " fs: " << fs << " mbps: " << mbps
		<< " IsRsrcByFs: "<< (WORD)IsRsrcByFs << " videoPartyType: " << eVideoPartyTypeNames[videoPartyType];
	PTRACE2(eLevelInfoNormal,"CVP8VideoCap::GetVideoPartyTypeFRandFS: ",msg.GetString());

	return videoPartyType;
}
///////////////////////////////////////////////////////////////////////////////
void CVP8VideoCap::SetVP8CapForCpFromVP8VideoType(VP8VideoModeDetails& Vp8Details)
{
	vp8CapStruct* pCapStruct = CAP_CAST(vp8CapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxFR 		= Vp8Details.maxFrameRate;
		pCapStruct->maxFS 		= Vp8Details.maxFS;
		pCapStruct->maxBitRate  = Vp8Details.maxBitRate/100;

		PTRACE2INT(eLevelInfoNormal,"CVP8VideoCap::SetVP8CapForCpFromVP8VideoType: pCapStruct->maxFS = ",pCapStruct->maxFS);

	}
}

///////////////////////////////////////////////////////////////////////////////
APIS8   CVP8VideoCap::GetFormatMpi(EFormat eFormat) const
{
	return 1;
}

/////////////////////////////////////////////////CVP8VideoCap::////////////////////////////////////////////////////
BYTE CVP8VideoCap::IsSupportPLI() const
{
	vp8CapStruct *pCapStruct = CAP_CAST(vp8CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_PLI);
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CVP8VideoCap::IsSupportFIR() const
{
	vp8CapStruct *pCapStruct = CAP_CAST(vp8CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_FIR);
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CVP8VideoCap::IsSupportTMMBR() const
{
	vp8CapStruct *pCapStruct = CAP_CAST(vp8CapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_TMMBR);

	return FALSE;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
//							 	    End VP8VideoCap Functions									  //
///////////////////////////////////////////////////////////////////////////////////////////////////


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CBaseDataCap
//=======================
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CBaseDataCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(dataCapStructBase);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CBaseDataCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(dataCapStructBase)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(dataCapStructBase));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CBaseDataCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	dataCapStructBase *pCapStruct = CAP_CAST(dataCapStructBase);
	CBaseDataCap &hOtherDataCap  = (CBaseDataCap&)otherCap;

	if (GetCapCode() == hOtherDataCap.GetCapCode())
	{
		eRes = kSuccess;
		pCapStruct->maxBitRate	= hOtherDataCap.GetBitRate();
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the bit rate.
//---------------------------------------------------------------------------------------------------
EResult CBaseDataCap::SetBitRate(APIS32 rate)
{
	EResult eRes = kFailure;
	dataCapStructBase *pCapStruct = CAP_CAST(dataCapStructBase);

	if (pCapStruct)
	{
		pCapStruct->maxBitRate = rate;
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the bit rate
//---------------------------------------------------------------------------------------------------
APIS32 CBaseDataCap::GetBitRate() const
{
	APIS32 bitRate = -1;
	dataCapStructBase *pCapStruct = CAP_CAST(dataCapStructBase);

	if (pCapStruct)
		bitRate = pCapStruct->maxBitRate;

	return bitRate;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values
//---------------------------------------------------------------------------------------------------
EResult CBaseDataCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	dataCapStructBase *pCapStruct = CAP_CAST(dataCapStructBase);

	if (pCapStruct)
		eRes = CBaseCap::SetStruct(cmCapData,eDirection,eRole);

	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CBaseDataCap::Dump(std::ostream& msg) const
{
	dataCapStructBase *pCapStruct = CAP_CAST(dataCapStructBase);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "maxBitRate				= " << pCapStruct->maxBitRate <<
			" (" << (pCapStruct->maxBitRate)*100 << " bps)\n";
		if(pCapStruct->mask & MrcScpMask)
			msg << "scp support" << "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CBaseCap* CBaseDataCap::GetHighestCommon(const CBaseCap & otherCap) const
{
	CBaseDataCap *pRes = NULL;

	if (m_pCapStruct->header.capTypeCode == ((const CBaseDataCap &)otherCap).m_pCapStruct->header.capTypeCode)
	{
		pRes = (CBaseDataCap*)CBaseCap::AllocNewCap((CapEnum)m_pCapStruct->header.capTypeCode,NULL);
		if (pRes)
		{
			pRes->SetHeader(cmCapData,(cmCapDirection)m_pCapStruct->header.direction,(ERoleLabel)m_pCapStruct->header.roleLabel);
			pRes->SetBitRate(min(GetBitRate(),((const CBaseDataCap&)otherCap).GetBitRate()));
		}
		else
		    PTRACE(eLevelError,"CBaseDataCap::GetHighestCommon - pRes is NULL");
	}

	return pRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBaseDataCap::IsSupportScp()	const
{
	dataCapStructBase *pCapStruct = CAP_CAST(dataCapStructBase);

	if (pCapStruct)
		return (pCapStruct->mask & MrcScpMask);

	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBaseDataCap::SetIsSupportScp(BYTE yesNo)
{
	dataCapStructBase *pCapStruct = CAP_CAST(dataCapStructBase);

	if (pCapStruct)
	{
		if ((yesNo))
			pCapStruct->mask |= MrcScpMask; //turn on the scp bit
		else
			pCapStruct->mask &= ~MrcScpMask; //turn off the scp bit
	}
}

/*
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CT120DataCap
//=======================
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the t120 values from h221 cap set.
//---------------------------------------------------------------------------------------------------
EResult CT120DataCap::SetFromH221Cap(const CCapH221 & h221Cap)
{
	EResult eRes = kFailure;
	t120DataCapStruct *pCapStruct = CAP_CAST(t120DataCapStruct);
	BYTE bIsParameterOK = ::isValidPObjectPtr(const_cast<CCapH221*> (&h221Cap));

	if (pCapStruct && bIsParameterOK)
	{
		eRes = kSuccess;
		CCapSetInfo capInfo = eT120DataCapCode;
		WORD t120MaxBitRate = 0;
		for (int i = 0; i < MAX_HMLP_CAPS; i++)
		{
			WORD currentMlpOrHmlpType = CCapSetInfo::g_T120dataCapQualityTbl[i].dataType;
			if (h221Cap.OnHsdHmlpCap(currentMlpOrHmlpType))
			{
				t120MaxBitRate = capInfo.GetT120DataBitRate(currentMlpOrHmlpType);
				i = MAX_MLP_HMLP_CAPS;
				break;
			}
		}
		for (;i < MAX_HMLP_CAPS + MAX_MLP_CAPS; i++)
		{
			WORD currentMlpOrHmlpType = CCapSetInfo::g_T120dataCapQualityTbl[i].dataType;
			if (h221Cap.OnMlpCap(currentMlpOrHmlpType))
			{
				t120MaxBitRate = capInfo.GetT120DataBitRate(currentMlpOrHmlpType);
				i = MAX_MLP_HMLP_CAPS;
				break;
			}
		}
		for (;i < MAX_MLP_HMLP_CAPS;i++)
		{
			WORD currentMlpOrHmlpType = CCapSetInfo::g_T120dataCapQualityTbl[i].dataType;
			if (h221Cap.OnDataVidCap(currentMlpOrHmlpType))
			{
				t120MaxBitRate = capInfo.GetT120DataBitRate(currentMlpOrHmlpType);
				break;
			}
		}
		eRes &= SetDefaults();
		eRes &= SetBitRate(t120MaxBitRate/100);
	}

	return eRes;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first value that "other" object has and "this" object is lack of.
//             The first 5 bits hold value/s :
//             HIGHER_BIT_RATE - Other's bit rate is higher.
//             The combination of values to compare can be only kBitRate in that case
//---------------------------------------------------------------------------------------------------

BYTE CT120DataCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//  class CH224DataCap
//=======================

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CH224DataCap::SetFromIpScm(const CComModeH323* pScm)
{
	EResult eRes = kFailure;
	dataCapStructBase *pCapStruct = CAP_CAST(dataCapStructBase);
	if (pCapStruct)
	{
		CCapSetInfo capInfo = (CapEnum)pCapStruct->header.capTypeCode;
		DWORD maxBitRate = pScm->GetMediaBitRate(cmCapData, cmCapTransmit);
		eRes = kSuccess;
		//In case we have 6.4 and 4.8 we initialize only the 6.4
		for (int i = 0; i < MAX_H224_CAPS; i++)
		{
			if(pCapStruct->header.capTypeCode == eAnnexQCapCode)
				eRes &= SetDefaults(cmCapReceiveAndTransmit);
			else
				eRes &= SetDefaults(cmCapReceive);

		//	eRes &= SetBitRate(maxBitRate/100);
			eRes &= SetBitRate(maxBitRate);
			i = MAX_H224_CAPS;
			break;
		}
	}
	return eRes;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CNonStandardCap
//=========================
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CNonStandardCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(ctNonStandardCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CNonStandardCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(ctNonStandardCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(ctNonStandardCapStruct));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CNonStandardCap::Dump(std::ostream& msg) const
{
	ctNonStandardCapStruct *pCapStruct = CAP_CAST(ctNonStandardCapStruct);

	if (pCapStruct)
	{
		BYTE bUSA		= FALSE;
		BYTE bNORWAY	= FALSE;
		BYTE bPolycom	= FALSE;

		CBaseCap::Dump(msg);
		msg <<   "t35CountryCode      = (" << (std::hex) << (int)pCapStruct->nonStandardData.info.t35CountryCode   << ")"
			<< "\nt35Extension        = (" << (std::hex) <<(int)pCapStruct->nonStandardData.info.t35Extension     << ")";

			if( pCapStruct->nonStandardData.info.t35CountryCode == NS_T35COUNTRY_CODE_USA &&
				pCapStruct->nonStandardData.info.t35Extension   == NS_T35EXTENSION_USA )
			{
				// if USA country code and extension
				bUSA = TRUE;
				msg << " -- USA\n";
			}
			else if( pCapStruct->nonStandardData.info.t35CountryCode == NS_T35COUNTRY_CODE_NORWAY &&
					 pCapStruct->nonStandardData.info.t35Extension   == NS_T35EXTENSION_NORWAY )
			{
				// if USA country code and extension
				bNORWAY = TRUE;
				msg << " -- NORWAY\n";
			}
			else
				msg << " -- UNKNOWN\n";

			msg	<< "\nmanufacturerCode = (" << (std::hex) <<(int)pCapStruct->nonStandardData.info.manufacturerCode << ")";

		if(bUSA && pCapStruct->nonStandardData.info.manufacturerCode == NS_MANUFACTURER_POLYCOM )
		{
			// if country is USA and Polycom manufacturer
			bPolycom = TRUE;
			msg << " -- Polycom;\n";
		}
		else if(bNORWAY && pCapStruct->nonStandardData.info.manufacturerCode == NS_MANUFACTURER_NORWAY )
		{
			// if country is NORWAY and Tandberg manufacturer
			msg << " -- Tandberg;\n";
		}
		else
			msg << " -- None Polycom (UNKNOWN);\n";

		for( int i = 0; i < CT_NonStandard_Data_Size; i++ )
		{
			if( i%8 == 0 )
				msg << "\n        ";
			msg << " (" << (std::hex) << (int)pCapStruct->nonStandardData.data[i] <<")";
			if(bPolycom)
			{
				switch( pCapStruct->nonStandardData.data[i] )
				{
				case NS_CAP_ACCORD_SENDER :          msg << " -- Polycom' MCU sender";    break;
				case NS_CAP_H323_VISUAL_CONCERT_PC : msg << " -- VisualConcertPC cap";    break;
				case NS_CAP_H323_VISUAL_CONCERT_FX : msg << " -- VisualConcertFX cap";    break;
				case NS_CAP_H323_H263_ANNEX_I:       msg << " -- H263_ANNEX_I";			  break;
				//case NS_CAP_H323_H263_QCIF_ANNEX_I : msg << " -- H263_QCif_AnnexI cap";   break;
				case NS_CAP_H323_H263_CIF_ANNEX_I :  msg << " -- H263_Cif_AnnexI cap";    break;
				case NS_CAP_H323_H263_4CIF_ANNEX_I : msg << " -- H263_4Cif_AnnexI cap";   break;
				case NS_CAP_H323_H263_QCIF_ANNEX_T : msg << " -- H263_QCif_AnnexT cap";   break;
				case NS_CAP_H323_H263_CIF_ANNEX_T :  msg << " -- H263_Cif_AnnexT cap";    break;
				case NS_CAP_H323_H263_4CIF_ANNEX_T : msg << " -- H263_4Cif_AnnexT cap";   break;
				case NS_CAP_H323_HIGH_CAPACITY :     msg << " -- High Video Capacity";    break;
				case NS_CAP_H323_VGA_800X600 :       msg << " -- VGA 800x600 Capacity";   break;
				case NS_CAP_H323_VGA_1024X768 :      msg << " -- VGA 1024x768 Capacity";  break;
				case NS_CAP_H323_VGA_1280X1024 :     msg << " -- VGA 1280x1024 Capacity"; break;
				case NS_CAP_H323_VIDEO_STREAMS_2 :   msg << " -- VideoStreams2 Capacity"; break;
				default: break;
				}
			}
			else
				msg << " -- ??";
		}
		msg << "\n ---- NonStandard Message end ----\n" << (std::dec);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first value that "other" object has and "this" object is lack of.
//             Since we don't support non standard channels we won't imply this function.
//---------------------------------------------------------------------------------------------------
BYTE CNonStandardCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD* pDetails) const
{//right now we check only AnnexI_NS
	BYTE bRes  = TRUE;
	*pDetails = 0x00000000;

	bRes = CBaseCap::IsContaining(other, valuesToCompare, pDetails);

	if (bRes && (valuesToCompare & kAnnexes))
	{
		BYTE bOtherHasAnnexI_NS = ((CNonStandardCap&)other).IsNonStandardAnnex(typeAnnexI_NS);
		if (bOtherHasAnnexI_NS)
			bRes = IsNonStandardAnnex(typeAnnexI_NS);
	}

	return bRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Set the structure values
//---------------------------------------------------------------------------------------------------
EResult CNonStandardCap::SetStruct(APIU8 t35CountryCode,APIU8 t35Extension,APIU16 manufacturerCode,char data[],int dataLength)
{
	EResult eRes = kFailure;
	ctNonStandardCapStruct *pCapStruct = CAP_CAST(ctNonStandardCapStruct);

	if (pCapStruct)
	{
		eRes = CBaseCap::SetStruct(cmCapNonStandard, cmCapReceive);
		pCapStruct->nonStandardData.info.objectLength     = 0;
		pCapStruct->nonStandardData.info.t35CountryCode   = t35CountryCode;
		pCapStruct->nonStandardData.info.t35Extension     = t35Extension;
		pCapStruct->nonStandardData.info.manufacturerCode = manufacturerCode;

		memset(pCapStruct->nonStandardData.data,0,CT_NonStandard_Data_Size);
		if (data && dataLength)
			memcpy(pCapStruct->nonStandardData.data,data,min(dataLength,CT_NonStandard_Data_Size));
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CNonStandardCap::SetAnnexI_NsStruct()
{
	EResult eRes = kFailure;
	ctNonStandardCapStruct* pCapStruct = CAP_CAST(ctNonStandardCapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		char data[CT_NonStandard_Data_Size];
		data[0] = NS_CAP_H323_HIGH_CAPACITY;
		data[1] = NS_CAP_H323_H263_ANNEX_I;

		int size = 2;

		eRes &= SetStruct(NS_T35COUNTRY_CODE_USA,NS_T35EXTENSION_USA,NS_MANUFACTURER_POLYCOM,data,size);
	}
	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the structure t35CountryCode
//---------------------------------------------------------------------------------------------------
APIU8 CNonStandardCap::GetT35CountryCode() const
{
	APIU8 res = 0;
	ctNonStandardCapStruct *pCapStruct = CAP_CAST(ctNonStandardCapStruct);
	if (pCapStruct)
		res = pCapStruct->nonStandardData.info.t35CountryCode;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the structure t35Extension
//---------------------------------------------------------------------------------------------------
APIU8 CNonStandardCap::GetT35Extension() const
{
	APIU8 res = 0;
	ctNonStandardCapStruct *pCapStruct = CAP_CAST(ctNonStandardCapStruct);
	if (pCapStruct)
		res = pCapStruct->nonStandardData.info.t35Extension;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the structure manufacturerCode
//---------------------------------------------------------------------------------------------------
APIU16 CNonStandardCap::GetManufacturerCode() const
{
	APIU16 res = 0;
	ctNonStandardCapStruct *pCapStruct = CAP_CAST(ctNonStandardCapStruct);
	if (pCapStruct)
		res = pCapStruct->nonStandardData.info.manufacturerCode;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the structure data in index i
//---------------------------------------------------------------------------------------------------
char CNonStandardCap::GetData(int i) const
{
	char res = 0;
	ctNonStandardCapStruct *pCapStruct = CAP_CAST(ctNonStandardCapStruct);
	BYTE bIsParameterOK = (i<CT_NonStandard_Data_Size);

	if (pCapStruct&&bIsParameterOK)
		res = pCapStruct->nonStandardData.data[i];

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Checks if data is in the non standard cap.
//---------------------------------------------------------------------------------------------------
BYTE CNonStandardCap::IsData(BYTE data) const
{
	BYTE bRes = FALSE;
	ctNonStandardCapStruct *pCapStruct = CAP_CAST(ctNonStandardCapStruct);
	if (pCapStruct)
	{
		for (int i = 0; i < CT_NonStandard_Data_Size; i++)
			if (pCapStruct->nonStandardData.data[i] == data)
			{
				bRes = TRUE;
				break;
			}
	}
	return bRes;
}

///////////////////////////////////////////////////////////////////////////////////////
BYTE CNonStandardCap::IsNonStandardAnnex(annexesListEn eAnnex) const
{
	BYTE bRes = FALSE;
	if (eAnnex == typeAnnexI_NS)
	{
		bRes = ((GetManufacturerCode() == NS_MANUFACTURER_POLYCOM) &&
				(IsData(NS_CAP_H323_H263_ANNEX_I)));
	}
	return bRes;
}

///////////////////////////////////////////////////////////////////////////////////////
EResult CNonStandardCap::SetDuoNS()
{
	EResult eRes = kFailure;
	ctNonStandardCapStruct *pCapStruct = CAP_CAST(ctNonStandardCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		char data[CT_NonStandard_Data_Size];
		int  size = 2;
		data[0] = 1; // According to Tandberg opcode
		data[1] = 1; // According to Tandberg opcode
		eRes &= SetStruct(NS_T35COUNTRY_CODE_NORWAY, NS_T35EXTENSION_NORWAY, NS_MANUFACTURER_NORWAY,data,size);
	}
	return eRes;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CGenericCap
//=========================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CGenericCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(genericCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CGenericCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(genericCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(genericCapStruct));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first value that "other" object has and "this" object is lack of.
//             Since we don't support generic channels we won't imply this function.
//---------------------------------------------------------------------------------------------------
BYTE CGenericCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x0000;
	return bRes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CEncryptionCap
//=========================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CEncryptionCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(encryptionCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CEncryptionCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(encryptionCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(encryptionCapStruct));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CEncryptionCap::Dump(std::ostream& msg) const
{
	encryptionCapStruct *pCapStruct = CAP_CAST(encryptionCapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);

		msg <<   "\nEncryption alg          = ";
		CMedString str;
		::GetEncryptionMediaTypeName(pCapStruct->type,str);
		msg << str.GetString();
		msg <<   "\nEntry to encrypted      = " << pCapStruct->entry;
		msg << "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the structure values
//---------------------------------------------------------------------------------------------------

EResult CEncryptionCap::SetStruct(EenMediaType encAlg,APIU16 entry)
{
	EResult eRes;
	encryptionCapStruct *pCapStruct = CAP_CAST(encryptionCapStruct);

	eRes = CBaseCap::SetStruct(cmCapH235,cmCapReceive);
	if (eRes == kSuccess)
	{
		pCapStruct->type	= (APIU16)encAlg;
		pCapStruct->entry	= entry;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Get entry
//---------------------------------------------------------------------------------------------------

APIU16 CEncryptionCap::GetEntry() const
{
	encryptionCapStruct *pCapStruct = CAP_CAST(encryptionCapStruct);

	return pCapStruct->entry;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CLprCap
//=======================


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CLprCap::SizeOf() const
{
	//PTRACE(eLevelInfoNormal,"SIZE OF IN LPR CAP");
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(lprCapStruct);
	//PTRACE2INT(eLevelInfoNormal,"SIZE OF IN LPR CAP size is",size);
	return size;
}
/////////////////////////////////////////
size_t CLprCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(lprCapStruct);
	return size;
}
////////////////////////////////////
EResult CLprCap::CopyBaseQualities(const CBaseVideoCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.GetStruct())
	{
		eRes = kSuccess;
		int sizeBaseWithAnnex = sizeof(genericVideoCapStruct);
		int sizeWithoutBaseSize = sizeBaseWithAnnex - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.GetStruct()) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, sizeWithoutBaseSize);
	}
	return eRes;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CLprCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(lprCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(lprCapStruct));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values
//---------------------------------------------------------------------------------------------------
EResult CLprCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		//eRes &= CBaseCap::SetStruct(cmCapGeneric, eDirection);
		eRes &= CBaseCap::SetStruct(cmCapGeneric, eDirection, eRole);
		CCapSetInfo capInfo = eLPRCapCode;
		pCapStruct->versionID = capInfo.g_lprCapSet[0].versionID;
		pCapStruct->minProtectionPeriod     = capInfo.g_lprCapSet[0].minProtectionPeriod;
		pCapStruct->maxProtectionPeriod     = capInfo.g_lprCapSet[0].maxProtectionPeriod;
		pCapStruct->maxRecoverySet			= capInfo.g_lprCapSet[0].maxRecoverySet;
		pCapStruct->maxRecoveryPackets      = capInfo.g_lprCapSet[0].maxRecoveryPackets;
		pCapStruct->maxPacketSize     	    = capInfo.g_lprCapSet[0].maxPacketSize;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values
//---------------------------------------------------------------------------------------------------
EResult CLprCap::SetDefaultsLpr(cmCapDirection eDirection,ERoleLabel eRole, cmCapDataType eType)
{
	EResult eRes = kFailure;
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(eType, eDirection, eRole);
		CCapSetInfo capInfo = eLPRCapCode;
		pCapStruct->versionID = capInfo.g_lprCapSet[0].versionID;
		pCapStruct->minProtectionPeriod     = capInfo.g_lprCapSet[0].minProtectionPeriod;
		pCapStruct->maxProtectionPeriod     = capInfo.g_lprCapSet[0].maxProtectionPeriod;
		pCapStruct->maxRecoverySet			= capInfo.g_lprCapSet[0].maxRecoverySet;
		pCapStruct->maxRecoveryPackets      = capInfo.g_lprCapSet[0].maxRecoveryPackets;
		pCapStruct->maxPacketSize     	    = capInfo.g_lprCapSet[0].maxPacketSize;
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CLprCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	lprCapStruct *pCapStruct = CAP_CAST(lprCapStruct);
	CLprCap &hOtherVideoCap  = (CLprCap&)otherCap;

	if (GetCapCode() == hOtherVideoCap.GetCapCode())
	{
		eRes = kSuccess;
		pCapStruct->versionID			   = hOtherVideoCap.GetLprVersionID();
		pCapStruct->minProtectionPeriod    = hOtherVideoCap.GetLprMinProtectionPeriod();
		pCapStruct->maxProtectionPeriod    = hOtherVideoCap.GetLprMaxProtectionPeriod();
		pCapStruct->maxRecoverySet	       = hOtherVideoCap.GetLprMaxRecoverySet();
		pCapStruct->maxRecoveryPackets	   = hOtherVideoCap.GetLprMaxRecoveryPackets();
		pCapStruct->maxPacketSize		   = hOtherVideoCap.GetLprMaxPacketSize();

	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CLprCap::Dump(std::ostream& msg) const
{
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);

		msg << "versionID               = " << pCapStruct->versionID;
		msg << "\nminProtectionPeriod     = " << pCapStruct->minProtectionPeriod;
		msg << "\nmaxProtectionPeriod     = " << pCapStruct->maxProtectionPeriod;
		msg << "\nmaxRecoverySet          = " << pCapStruct->maxRecoverySet;
		msg << "\nmaxRecoveryPackets      = " << pCapStruct->maxRecoveryPackets;
		msg << "\nmaxPacketSize           = " << pCapStruct->maxPacketSize;
		msg << "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CLprCap::GetLprVersionID() const
{
	APIU32 versionID = 0;
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		versionID = pCapStruct->versionID;

	return versionID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CLprCap::SetLprVersionID(APIU32 versionID)
{
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		pCapStruct->versionID = versionID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CLprCap::GetLprMinProtectionPeriod() const
{
	APIU32 minProtectionPeriod = 0;
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		minProtectionPeriod = pCapStruct->minProtectionPeriod;

	return minProtectionPeriod;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CLprCap::SetLprMinProtectionPeriod(APIU32 minProtectionPeriod)
{
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		pCapStruct->minProtectionPeriod = minProtectionPeriod;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CLprCap::GetLprMaxProtectionPeriod() const
{
	APIU32 maxProtectionPeriod = 0;
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		maxProtectionPeriod = pCapStruct->maxProtectionPeriod;

	return maxProtectionPeriod;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CLprCap::SetLprMaxProtectionPeriod(APIU32 maxProtectionPeriod)
{
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		pCapStruct->maxProtectionPeriod = maxProtectionPeriod;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CLprCap::GetLprMaxRecoverySet() const
{
	APIU32 maxRecoverySet = 0;
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		maxRecoverySet = pCapStruct->maxRecoverySet;

	return maxRecoverySet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CLprCap::SetLprMaxRecoverySet(APIU32 maxRecoverySet)
{
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		pCapStruct->maxRecoverySet = maxRecoverySet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CLprCap::GetLprMaxRecoveryPackets() const
{
	APIU32 maxRecoveryPackets = 0;
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		maxRecoveryPackets = pCapStruct->maxRecoveryPackets;

	return maxRecoveryPackets;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CLprCap::SetLprMaxRecoveryPackets(APIU32 maxRecoveryPackets)
{
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		pCapStruct->maxRecoveryPackets = maxRecoveryPackets;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CLprCap::GetLprMaxPacketSize() const
{
	APIU32 maxPacketSize = 0;
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		maxPacketSize = pCapStruct->maxPacketSize;

	return maxPacketSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CLprCap::SetLprMaxPacketSize(APIU32 maxPacketSize)
{
	lprCapStruct* pCapStruct = CAP_CAST(lprCapStruct);

	if (pCapStruct)
		pCapStruct->maxPacketSize = maxPacketSize;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CSdesCap
//=======================


/////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------------------
EResult CSdesCap::SetDefaults(cmCapDataType eType, cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	sdesCapStruct *pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(eType,eDirection,eRole);

	}
	return eRes;
}

EResult CSdesCap::SetStruct(cmCapDataType eType,cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kSuccess;

	sdesCapStruct *pCapStruct = CAP_CAST(sdesCapStruct);
	if (pCapStruct)
	{
		eRes = CBaseCap::SetStruct(eType,eDirection,eRole);
	}
	return eRes;
}


size_t CSdesCap::SizeOf() const
{
	size_t size = 0;
	sdesCapStruct *pCapStruct = CAP_CAST(sdesCapStruct);
	if (pCapStruct)
	{
		int numKeyParams = pCapStruct->numKeyParams;
		if(numKeyParams == 0)
			numKeyParams = 1;

		size += sizeof(sdesCapStruct);
		size += (numKeyParams * sizeof(xmlSdesKeyParamsStruct));

	}

	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
size_t CSdesCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(sdesCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CSdesCap::AllocStruct(size_t size)
{
	size_t tmpSize = 0;

	tmpSize = sizeof(sdesCapStruct) + sizeof(xmlSdesKeyParamsStruct) ;

	if (size && (size > tmpSize))
		AllocStructBySize(size);
	else
		AllocStructBySize(tmpSize);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
sdesCapStruct* CSdesCap::ReallocIfNeeded(BYTE reallocAnyWay)
{
	sdesCapStruct *pResCapStruct = NULL;
	sdesCapStruct *pCapStruct = CAP_CAST(sdesCapStruct);
	size_t tmpSize = 0;

	tmpSize = sizeof(sdesCapStruct) + sizeof(xmlSdesKeyParamsStruct) ;

	if (pCapStruct)
	{
		if (reallocAnyWay == TRUE ) {
			size_t newSize = SizeOf();
			BYTE *pNewStruct = new BYTE[newSize];
			memset(pNewStruct,0,newSize);
			memcpy(pNewStruct,(BYTE*)pCapStruct,tmpSize);
			FreeStruct();
			m_pCapStruct = (BaseCapStruct *)pNewStruct;
		}
		pResCapStruct = (sdesCapStruct *)m_pCapStruct;
	}

	return pResCapStruct;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CSdesCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	APIU32 i = 0;
	BYTE	reallocAnyWay	= TRUE;
	sdesCapStruct *pCapStruct = CAP_CAST(sdesCapStruct);
	sdesCapStruct *pOtherCapStruct = (sdesCapStruct*)otherCap.GetStruct();
	CSdesCap &hOtherSdesCap  = (CSdesCap&)otherCap;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	if (GetCapCode() == hOtherSdesCap.GetCapCode())
	{
		pCapStruct->tag				= hOtherSdesCap.GetSdesTag();
		pCapStruct->cryptoSuite		= hOtherSdesCap.GetSdesCryptoSuite();
		pCapStruct->numKeyParams	= hOtherSdesCap.GetSdesNumOfKeysParam();
		pCapStruct = ReallocIfNeeded(reallocAnyWay);
		if (pCapStruct)
		{
			eRes = kSuccess;

			pCapStruct->sessionParams.sdesUnencryptedSrtp =  hOtherSdesCap.GetIsSdesUnencryptedSrtp();
			pCapStruct->sessionParams.sdesUnencryptedSrtcp = hOtherSdesCap.GetIsSdesUnencryptedSrtcp();
			pCapStruct->sessionParams.sdesUnauthenticatedSrtp = hOtherSdesCap.GetIsSdesUnauthenticatedSrtp();
			pCapStruct->sessionParams.bIsKdrInUse = hOtherSdesCap.GetIsSdesKdrInUse();
			pCapStruct->sessionParams.sdesKdr =  hOtherSdesCap.GetSdesKdr();
			pCapStruct->sessionParams.bIsWshInUse =  hOtherSdesCap.GetIsSdesWshInUse();
			pCapStruct->sessionParams.sdesWsh =  hOtherSdesCap.GetSdesWsh();
			pCapStruct->sessionParams.bIsFecOrderInUse =  hOtherSdesCap.GetIsSdesFecOrderInUse();
			pCapStruct->sessionParams.sdesFecOrder =  hOtherSdesCap.GetSdesFecOrder();
			pCapStruct->sessionParams.bIsFecKeyInUse =  hOtherSdesCap.GetIsSdesFecKeyInUse();

			for (i = 0; i < pCapStruct->numKeyParams; i++) {
				pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (pCapStruct->keyParamsList + (i *sizeof(xmlSdesKeyParamsStruct)));
				pXmlSdesKeyParamsStruct->elem.keyMethod =  hOtherSdesCap.GetSdesKeyMethod(i);
				char *sdesBase64KeySalt = hOtherSdesCap.GetSdesBase64KeySalt(i) ;
				if (sdesBase64KeySalt)
				{
					strncpy(pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt,
							sdesBase64KeySalt,
							sizeof(pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt) - 1);
					pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt[sizeof(pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt) - 1] = '\0';
				}
				else
					PTRACE2INT(eLevelInfoNormal,"CSdesCap::CopyQualities CSdesCap::CopyQualities hOtherSdesCap.GetSdesBase64KeySalt(i) is NULL, i=",i);
				pXmlSdesKeyParamsStruct->elem.keyInfo.bIsLifeTimeInUse =  hOtherSdesCap.GetIsSdesLifeTimeInUse(i);
				pXmlSdesKeyParamsStruct->elem.keyInfo.lifetime = hOtherSdesCap.GetSdesLifeTime(i);
				pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiInUse = hOtherSdesCap.GetIsSdesMkiInUse(i);
				pXmlSdesKeyParamsStruct->elem.keyInfo.mkiValue = hOtherSdesCap.GetSdesMkiValue(i);
				pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiValueLenInUse = hOtherSdesCap.GetIsSdesMkiValueLenInUse(i);
				pXmlSdesKeyParamsStruct->elem.keyInfo.mkiValueLen = hOtherSdesCap.GetSdesMkiValueLen(i);
			}
		}
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CSdesCap::Dump(std::ostream& msg) const
{
	eLogLevel loggerlevel = CProcessBase::GetProcess()->GetMaxLogLevel();
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;
	sdesSessionParamsStruct	sessionParams;
	APIU32 numOfKeys = 0;
	APIU32 i = 0;

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		sessionParams = pCapStruct->sessionParams;


		msg << "tag                   	= " << pCapStruct->tag;
		msg << "\ncryptoSuite       		= " << pCapStruct->cryptoSuite;

		numOfKeys = pCapStruct->numKeyParams;
		msg << "\nnumOfKeys       		= " << numOfKeys;

		for (i = 0; i < numOfKeys; i++)
		{
			pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (pCapStruct->keyParamsList + (i *sizeof(xmlSdesKeyParamsStruct)));

			if (pXmlSdesKeyParamsStruct)
			{
				msg << "\nSdesKeyMethod 		= " << pXmlSdesKeyParamsStruct->elem.keyMethod;

				if(eLevelDebug == loggerlevel && !CSysConfigBase::IsUnderJITCState())
				msg << "\nSdesBase64Key 		= " << pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt;

				msg << "\nbIsLifeTimeInUse      = " << pXmlSdesKeyParamsStruct->elem.keyInfo.bIsLifeTimeInUse;
				if (pXmlSdesKeyParamsStruct->elem.keyInfo.bIsLifeTimeInUse)
				msg << "\nlifetime 				= " << pXmlSdesKeyParamsStruct->elem.keyInfo.lifetime;

				msg << "\nbIsMkiInUse           = " << pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiInUse;
				if (pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiInUse)
				msg << "\nmkiValue 				= " << pXmlSdesKeyParamsStruct->elem.keyInfo.mkiValue;

				msg << "\nbIsMkiValueLenInUse = " << pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiValueLenInUse;
				if (pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiValueLenInUse)
				msg << "\nmkiValueLen 			= " << pXmlSdesKeyParamsStruct->elem.keyInfo.mkiValueLen;
			}
		}

		msg << "\n";
	}
}

//Gets
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CSdesCap::GetSdesTag() const
{
	APIU32 tag = 0;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct)
		tag = pCapStruct->tag;

	return tag;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CSdesCap::GetSdesCryptoSuite() const
{
	APIU16 cryptoSuite = eUnknownSuiteParam;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct)
		cryptoSuite = pCapStruct->cryptoSuite;

	return cryptoSuite;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CSdesCap::GetSdesNumOfKeysParam() const
{
	APIU32 numKeyParams = 0;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct)
		numKeyParams = pCapStruct->numKeyParams;

	return numKeyParams;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesUnencryptedSrtp() const
{
	BOOL sdesUnencryptedSrtp = FALSE;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sdesUnencryptedSrtp = sessionParams.sdesUnencryptedSrtp;
	}
	return sdesUnencryptedSrtp;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesUnencryptedSrtcp() const
{
	BOOL sdesUnencryptedSrtcp = FALSE;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sdesUnencryptedSrtcp = sessionParams.sdesUnencryptedSrtcp;
	}
	return sdesUnencryptedSrtcp;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesUnauthenticatedSrtp() const
{
	BOOL sdesUnauthenticatedSrtp = FALSE;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sdesUnauthenticatedSrtp = sessionParams.sdesUnauthenticatedSrtp;
	}
	return sdesUnauthenticatedSrtp;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesKdrInUse() const
{
	BOOL bIsKdrInUse = FALSE;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		bIsKdrInUse = sessionParams.bIsKdrInUse;
	}
	return bIsKdrInUse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CSdesCap::GetSdesKdr() const
{
	APIU8 sdesKdr = 0;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sdesKdr = sessionParams.sdesKdr;
	}
	return sdesKdr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesWshInUse() const
{
	BOOL bIsWshInUse = FALSE;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		bIsWshInUse = sessionParams.bIsWshInUse;
	}
	return bIsWshInUse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CSdesCap::GetSdesWsh() const
{
	APIU16 sdesWsh = 0;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sdesWsh = sessionParams.sdesWsh;
	}
	return sdesWsh;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesFecOrderInUse() const
{
	BOOL bIsFecOrderInUse = FALSE;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		bIsFecOrderInUse = sessionParams.bIsFecOrderInUse;
	}
	return bIsFecOrderInUse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CSdesCap::GetSdesFecOrder() const
{
	APIU16 sdesFecOrder = eSrtpUnknownFec;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sdesFecOrder = sessionParams.sdesFecOrder;
	}
	return sdesFecOrder;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesFecKeyInUse() const
{
	BOOL bIsFecKeyInUse = FALSE;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		bIsFecKeyInUse = sessionParams.bIsFecKeyInUse;
	}
	return bIsFecKeyInUse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CSdesCap::GetSdesKeyMethod(int keyNumber) const
{
	APIU16	keyMethod = eSdesUnknownKeyMethod;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
		if (pXmlSdesKeyParamsStruct)
			keyMethod = pXmlSdesKeyParamsStruct->elem.keyMethod;
	}

	return keyMethod;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
char* CSdesCap::GetSdesBase64KeySalt(int keyNumber) const
{
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
		if (pXmlSdesKeyParamsStruct)
			return pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesLifeTimeInUse(int keyNumber) const
{
	BOOL bIsLifeTimeInUse = FALSE;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				bIsLifeTimeInUse = pXmlSdesKeyParamsStruct->elem.keyInfo.bIsLifeTimeInUse;
	}
	return bIsLifeTimeInUse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CSdesCap::GetSdesLifeTime(int keyNumber) const
{
	APIU32 lifetime = 0;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				lifetime = pXmlSdesKeyParamsStruct->elem.keyInfo.lifetime;
	}
	return lifetime;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesMkiInUse(int keyNumber) const
{
	BOOL bIsMkiInUse = FALSE;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				bIsMkiInUse = pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiInUse;
	}
	return bIsMkiInUse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CSdesCap::GetSdesMkiValue(int keyNumber) const
{
	APIU8 mkiValue = 0;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				mkiValue = pXmlSdesKeyParamsStruct->elem.keyInfo.mkiValue;
	}
	return mkiValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSdesCap::GetIsSdesMkiValueLenInUse(int keyNumber) const
{
	BOOL bIsMkiValueLenInUse = FALSE;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				bIsMkiValueLenInUse = pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiValueLenInUse;
	}
	return bIsMkiValueLenInUse;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CSdesCap::GetSdesMkiValueLen(int keyNumber) const
{
	APIU8 mkiValueLen = FALSE;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				mkiValueLen = pXmlSdesKeyParamsStruct->elem.keyInfo.mkiValueLen;
	}
	return mkiValueLen;
}

//Sets
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesXmlDynamicProps()
{
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);
	xmlSdesKeyParamsStruct	*pXmlElem;
	xmlDynamicHeader		*pXmlHeader;

	if (pCapStruct) {
		pCapStruct->xmlDynamicProps.numberOfDynamicParts = pCapStruct->numKeyParams;
		pCapStruct->xmlDynamicProps.sizeOfAllDynamicParts = (pCapStruct->numKeyParams) * sizeof(xmlSdesKeyParamsStruct);

		pXmlElem = (xmlSdesKeyParamsStruct *) &pCapStruct->keyParamsList;
		pXmlHeader = &pXmlElem->xmlHeader;
		pXmlHeader->dynamicType 	= eSdesCapCode;
		pXmlHeader->dynamicLength 	= sizeof(sdesKeyParamsStruct);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesTag(APIU32 sdesTag)
{
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct)
		pCapStruct->tag = sdesTag;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesCryptoSuite(APIU16 cryptoSuite)
{
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct)
		pCapStruct->cryptoSuite = cryptoSuite;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetNumOfKeysParam(APIU32 numKeyParams)
{
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pCapStruct->numKeyParams = numKeyParams;
		pCapStruct->xmlDynamicProps.numberOfDynamicParts = numKeyParams;
		pCapStruct->xmlDynamicProps.sizeOfAllDynamicParts = numKeyParams * sizeof(xmlSdesKeyParamsStruct);
	}
	if(numKeyParams != 0 && numKeyParams != 1)
		pCapStruct = ReallocIfNeeded(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesUnencryptedSrtp(BOOL bIsSdesUnencryptedSrtp)
{
	APIU32 bIsEnable = 0;
	sdesSessionParamsStruct	sessionParams;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsSdesUnencryptedSrtp)
		bIsEnable = 1;
	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sessionParams.sdesUnencryptedSrtp = bIsEnable;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesUnencryptedSrtcp(BOOL bIsSdesUnencryptedSrtcp)
{
	APIU32 bIsEnable = 0;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsSdesUnencryptedSrtcp)
		bIsEnable = 1;
	if (pCapStruct) {
		pCapStruct->sessionParams.sdesUnencryptedSrtcp = bIsEnable;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesUnauthenticatedSrtp(BOOL bIsSdesUnauthenticatedSrtp)
{
	APIU32 bIsEnable = 0;
	sdesSessionParamsStruct	sessionParams;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsSdesUnauthenticatedSrtp)
		bIsEnable = 1;
	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sessionParams.sdesUnauthenticatedSrtp = bIsEnable;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesKdrInUse(BOOL bIsKdrInUse)
{
	APIU32 bIsEnable = 0;
	sdesSessionParamsStruct	sessionParams;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsKdrInUse)
		bIsEnable = 1;
	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sessionParams.bIsKdrInUse = bIsEnable;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesKdr(APIU8 sdesKdr)
{
	sdesSessionParamsStruct	sessionParams;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sessionParams.sdesKdr = sdesKdr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesWshInUse(BOOL bIsWshInUse)
{
	APIU32 bIsEnable = 0;
	sdesSessionParamsStruct	sessionParams;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsWshInUse)
		bIsEnable = 1;
	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sessionParams.bIsWshInUse = bIsEnable;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesWsh(APIU16 sdesWsh)
{
	sdesSessionParamsStruct	sessionParams;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sessionParams.sdesWsh = sdesWsh;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesFecOrderInUse(BOOL bIsFecOrderInUse)
{
	APIU32 bIsEnable = 0;
	sdesSessionParamsStruct	sessionParams;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsFecOrderInUse)
		bIsEnable = 1;
	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sessionParams.bIsFecOrderInUse = bIsEnable;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesFecOrder(APIU16 sdesFecOrder)
{
	sdesSessionParamsStruct	sessionParams;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sessionParams.sdesFecOrder = sdesFecOrder;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesFecKeyInUse(BOOL bIsFecKeyInUse)
{
	APIU32 bIsEnable = 0;
	sdesSessionParamsStruct	sessionParams;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsFecKeyInUse)
		bIsEnable = 1;
	if (pCapStruct) {
		sessionParams = pCapStruct->sessionParams;
		sessionParams.bIsFecKeyInUse = bIsEnable;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesKeyMethod(int keyNumber, APIU16 keyMethod)
{
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);
	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
		if (pXmlSdesKeyParamsStruct)
			pXmlSdesKeyParamsStruct->elem.keyMethod = keyMethod;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesBase64KeySalt(int keyNumber, char* pBase64MasterSalt)
{
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;
	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
		if(pBase64MasterSalt) {
			APIU32 keyLen = strlen(pBase64MasterSalt);
			if (pXmlSdesKeyParamsStruct && keyLen != 0) {
				strncpy(pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt, pBase64MasterSalt, sizeof(pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt) - 1);
				pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt[sizeof(pXmlSdesKeyParamsStruct->elem.keyInfo.keySalt) - 1] ='\0';
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesLifeTimeInUse(int keyNumber, BOOL bIsLifeTimeInUse)
{
	APIU32 bIsEnable = 0;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsLifeTimeInUse)
		bIsEnable = 1;
	if (pCapStruct) {
		//pCapStruct = ReallocIfNeeded(FALSE);
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				pXmlSdesKeyParamsStruct->elem.keyInfo.bIsLifeTimeInUse = bIsEnable;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesLifeTime(int keyNumber,APIU32 lifetime)
{
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		//pCapStruct = ReallocIfNeeded(FALSE);
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				pXmlSdesKeyParamsStruct->elem.keyInfo.lifetime = lifetime;

	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesMkiInUse(int keyNumber, BOOL bIsMkiInUse)
{
	APIU32 bIsEnable = 0;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsMkiInUse)
		bIsEnable = 1;
	if (pCapStruct) {
		//pCapStruct = ReallocIfNeeded(FALSE);
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiInUse = bIsEnable;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetSdesMkiValue(int keyNumber, APIU8 mkiValue)
{
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		//pCapStruct = ReallocIfNeeded(FALSE);
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				pXmlSdesKeyParamsStruct->elem.keyInfo.mkiValue = mkiValue;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSdesCap::SetIsSdesMkiValueLenInUse(int keyNumber, BOOL bIsMkiValueLenInUse)
{
	APIU32 bIsEnable = 0;
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if(bIsMkiValueLenInUse)
		bIsEnable = 1;
	if (pCapStruct) {
		//pCapStruct = ReallocIfNeeded(FALSE);
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				pXmlSdesKeyParamsStruct->elem.keyInfo.bIsMkiValueLenInUse = bIsEnable;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CSdesCap::SetSdesMkiValueLen(int keyNumber, APIU8 mkiValueLen)
{
	xmlSdesKeyParamsStruct * pXmlSdesKeyParamsStruct = NULL;

	sdesCapStruct* pCapStruct = CAP_CAST(sdesCapStruct);

	if (pCapStruct) {
		//pCapStruct = ReallocIfNeeded(FALSE);
		pXmlSdesKeyParamsStruct = (xmlSdesKeyParamsStruct *) (&pCapStruct->keyParamsList + (keyNumber *sizeof(xmlSdesKeyParamsStruct)));
			if (pXmlSdesKeyParamsStruct)
				pXmlSdesKeyParamsStruct->elem.keyInfo.mkiValueLen = mkiValueLen;

	}
}
/////////////////////////////////////////////////////////////////////////////////////
BYTE CSdesCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x00000000;

	sdesCapStruct *pCapStruct 		= CAP_CAST(sdesCapStruct);
	sdesCapStruct *pOtherCapStruct 	= (sdesCapStruct *)other.GetStruct();

	PTRACE2INT(eLevelError, "CSdesCap::IsContaining - valuesToCompare - ", valuesToCompare);

	if (pCapStruct && pOtherCapStruct)
	{
		bRes = CBaseCap::IsContaining(other,valuesToCompare,pDetails);

		if ((valuesToCompare & kCryptoSuit) && bRes)
		{
			if (pCapStruct->cryptoSuite != pOtherCapStruct->cryptoSuite)
			{
				CLargeString str;
				str << "this cryptoSuite:" << pCapStruct->cryptoSuite << ", other cryptoSuite: " << pOtherCapStruct->cryptoSuite;

				PTRACE2(eLevelError, "CSdesCap::IsContaining - differnet crypto suit types - ", str.GetString());
				*pDetails |= DIFFERENT_CRYPTO_SUIT;
				bRes = FALSE;
			}
			else
				PTRACE2INT(eLevelError, "CSdesCap::IsContaining - same crypto suit type:", pCapStruct->cryptoSuite);
		}
	}
	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}
//  class CICEPwdCap
//=======================
EResult CICEPwdCap::SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr)
{
	EResult eRes = kSuccess;

	icePwdCapStruct *pCapStruct = CAP_CAST(icePwdCapStruct);
	if (pCapStruct)
	{
		if(DataStr)
		{
			strncpy(pCapStruct->icePwd, DataStr, sizeof(pCapStruct->icePwd) - 1);
			pCapStruct->icePwd[sizeof(pCapStruct->icePwd) - 1] = '\0';

			PASSERT(strlen(DataStr) > IcePwdLen);
		}
		eRes = CBaseCap::SetStruct(eType,eDirection);
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CICEPwdCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(icePwdCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CICEPwdCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(icePwdCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(icePwdCapStruct));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CICEPwdCap::Dump(std::ostream& msg) const
{
	icePwdCapStruct* pCapStruct = CAP_CAST(icePwdCapStruct);

		if (pCapStruct)
		{
			CBaseCap::Dump(msg);
			msg << "icePwd                   = " << pCapStruct->icePwd;
			msg << "\n";
		}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CICEUfragCap
//=======================
EResult CICEUfragCap::SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr)
{
	EResult eRes = kSuccess;
	iceUfragCapStruct *pCapStruct = CAP_CAST(iceUfragCapStruct);

	if (pCapStruct)
	{
		if(DataStr)
		{
			strncpy(pCapStruct->iceUfrag, DataStr, sizeof(pCapStruct->iceUfrag) - 1);
			pCapStruct->iceUfrag[sizeof(pCapStruct->iceUfrag) - 1] = '\0';

			PASSERT(strlen(DataStr) > IceUfragLen);
		}
		eRes = CBaseCap::SetStruct(eType,eDirection);
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CICEUfragCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(iceUfragCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CICEUfragCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(iceUfragCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(iceUfragCapStruct));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CICEUfragCap::Dump(std::ostream& msg) const
{
	iceUfragCapStruct* pCapStruct = CAP_CAST(iceUfragCapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "iceUfrag     = " << pCapStruct->iceUfrag;
		msg << "\n";
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CICECandidateCap
//=======================
EResult CICECandidateCap::SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr, APIS8 candidateType)
{
	EResult eRes = kSuccess;

	TRACEINTO << "dbg candidateType:" << candidateType;
	iceCandidateCapStruct *pCapStruct = CAP_CAST(iceCandidateCapStruct);
	if (pCapStruct)
	{
		if(DataStr)
		{
			strncpy(pCapStruct->candidate, DataStr, sizeof(pCapStruct->candidate) - 1);
			pCapStruct->candidate[sizeof(pCapStruct->candidate) - 1] = '\0';

			PASSERT(strlen(DataStr) > 512);
		}
		pCapStruct->candidateType = candidateType;
		eRes = CBaseCap::SetStruct(eType,eDirection);
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CICECandidateCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(iceCandidateCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CICECandidateCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(iceCandidateCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(iceCandidateCapStruct));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CICECandidateCap::Dump(std::ostream& msg) const
{
	iceCandidateCapStruct* pCapStruct = CAP_CAST(iceCandidateCapStruct);

		if (pCapStruct)
		{
			CBaseCap::Dump(msg);

			msg << "candidate                   = " << pCapStruct->candidate;
			msg << "\ncandidateType					= " << (DWORD)pCapStruct->candidateType;
			msg << "\n";
		}

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CICERemoteCandidateCap

//=======================
EResult CICERemoteCandidateCap::SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr)
{
	EResult eRes = kSuccess;

	iceRemoteCandidateCapStruct *pCapStruct = CAP_CAST(iceRemoteCandidateCapStruct);
	if (pCapStruct)
	{
		if(DataStr)
		{
			strncpy(pCapStruct->candidate, DataStr, sizeof(pCapStruct->candidate) - 1);
			pCapStruct->candidate[sizeof(pCapStruct->candidate) - 1] = '\0';

			PASSERT(strlen(DataStr) > 512);
		}

		eRes = CBaseCap::SetStruct(eType,eDirection);
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CICERemoteCandidateCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(iceRemoteCandidateCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CICERemoteCandidateCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(iceRemoteCandidateCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(iceRemoteCandidateCapStruct));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CICERemoteCandidateCap::Dump(std::ostream& msg) const
{
	iceRemoteCandidateCapStruct* pCapStruct = CAP_CAST(iceRemoteCandidateCapStruct);

		if (pCapStruct)
		{
			CBaseCap::Dump(msg);
			msg << "candidate                   = " << pCapStruct->candidate;
			msg << "\n";
		}

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CICERemoteCandidateCap
//=======================
EResult CRtcpCap::SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr)
{
	EResult eRes = kSuccess;

	rtcpCapStruct *pCapStruct = CAP_CAST(rtcpCapStruct);
	if (pCapStruct)
	{
		if(DataStr)
			pCapStruct->port = atoi(DataStr);

		eRes = CBaseCap::SetStruct(eType,eDirection);
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CRtcpCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(rtcpCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CRtcpCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(rtcpCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(rtcpCapStruct));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtcpCap::Dump(std::ostream& msg) const
{
	rtcpCapStruct* pCapStruct = CAP_CAST(rtcpCapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "port                   = " << pCapStruct->port;
		msg << "\n";
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CBfcpCap
//=======================

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult  CBfcpCap::SetBfcp(enTransportType transType)
{
	EResult eRes = kFailure;

	bfcpCapStruct *pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
	{
		SetDefaults();
		SetTransportType(transType);

		eRes = kSuccess;
	}

	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CBfcpCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(bfcpCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(bfcpCapStruct));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
size_t CBfcpCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(bfcpCapStruct);
	return size;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::Dump(std::ostream& msg) const
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "setup                   = " << GetBfcpSetupStr((eBfcpSetup)pCapStruct->setup);
		msg << "\nconnection              = " << GetBfcpConnectionStr((eBfcpConnection)pCapStruct->connection);
		msg << "\nfloor control           = " << GetBfcpFloorCtrlStr((eBfcpFloorCtrl)pCapStruct->floorctrl);
		msg << "\nconfid                  = " << pCapStruct->confid;
		msg << "\nuserid                  = " << pCapStruct->userid;
		DumpFloorId(msg, pCapStruct->floorid_0);
		DumpFloorId(msg, pCapStruct->floorid_1);
		DumpFloorId(msg, pCapStruct->floorid_2);
		DumpFloorId(msg, pCapStruct->floorid_3);
		msg << "\ninfo enabled            = " << (int)pCapStruct->xbfcp_info_enabled;
		msg << "\ninfo time               = " << (int)pCapStruct->xbfcp_info_time;
		msg << "\ntransport type          = " << (int)pCapStruct->transType;
		msg << "\n";
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::DumpFloorId(std::ostream& msg, bfcpFlooridStruct &hBfcpFlooridStruct) const
{
	if (hBfcpFlooridStruct.floorid[0])
	{
		msg << "\nFloorID                 = " << hBfcpFlooridStruct.floorid;
		if (hBfcpFlooridStruct.m_stream_0[0])
			msg << "\nStream0                 = " << hBfcpFlooridStruct.m_stream_0;
		if (hBfcpFlooridStruct.m_stream_1[0])
			msg << "\nStream1                 = " << hBfcpFlooridStruct.m_stream_1;
		if (hBfcpFlooridStruct.m_stream_2[0])
			msg << "\nStream2                 = " << hBfcpFlooridStruct.m_stream_2;
		if (hBfcpFlooridStruct.m_stream_3[0])
			msg << "\nStream3                 = " << hBfcpFlooridStruct.m_stream_3;
	}
	else
		msg << "\nEmpty FloorId Structure";
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CBfcpCap::SetDefaults(cmCapDirection eDirection, ERoleLabel eRole)
{
	EResult eRes = kFailure;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapBfcp, eDirection, eRole);

		pCapStruct->floorctrl =  bfcp_flctrl_s_only;
		memset(pCapStruct->confid, 0, BFCP_MAX_CONFID_LENGTH);
		memset(pCapStruct->userid, 0, BFCP_MAX_USERID_LENGTH);
		SetDefaultsFloorId(pCapStruct->floorid_0);
		SetDefaultsFloorId(pCapStruct->floorid_1);
		SetDefaultsFloorId(pCapStruct->floorid_2);
		SetDefaultsFloorId(pCapStruct->floorid_3);
		pCapStruct->setup = bfcp_setup_passive ; //bfcp_setup_null;
		pCapStruct->connection = bfcp_connection_new; //bfcp_connection_null;
		pCapStruct->xbfcp_info_enabled = FALSE;
		pCapStruct->xbfcp_info_time = 0;
		pCapStruct->mstreamType = bfcp_m_stream_None;
		pCapStruct->transType = eTransportTypeUdp;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetDefaultsFloorId(bfcpFlooridStruct &hBfcpFlooridStruct)
{
	memset(hBfcpFlooridStruct.floorid, 0, BFCP_MAX_FLOORID_LENGTH);
	memset(hBfcpFlooridStruct.m_stream_0, 0, BFCP_MAX_STREAM_LENGTH);
	memset(hBfcpFlooridStruct.m_stream_1, 0, BFCP_MAX_STREAM_LENGTH);
	memset(hBfcpFlooridStruct.m_stream_2, 0, BFCP_MAX_STREAM_LENGTH);
	memset(hBfcpFlooridStruct.m_stream_3, 0, BFCP_MAX_STREAM_LENGTH);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CBfcpCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	bfcpCapStruct *pCapStruct = CAP_CAST(bfcpCapStruct);
	CBfcpCap &hOtherBfcpCap  = (CBfcpCap&)otherCap;

	if (GetCapCode() == hOtherBfcpCap.GetCapCode())
	{
		eRes = kSuccess;
		bfcpCapStruct *pOtherCapStruct = (bfcpCapStruct*)otherCap.GetStruct();

		pCapStruct->floorctrl = pOtherCapStruct->floorctrl;
		strncpy(pCapStruct->confid, pOtherCapStruct->confid, BFCP_MAX_CONFID_LENGTH-1);
		pCapStruct->confid[BFCP_MAX_CONFID_LENGTH-1] = '\0';
		strncpy(pCapStruct->userid, pOtherCapStruct->userid, BFCP_MAX_USERID_LENGTH-1);
		pCapStruct->userid[BFCP_MAX_USERID_LENGTH-1] = '\0';
		CopyQualitiesFloorId(pCapStruct->floorid_0, pOtherCapStruct->floorid_0);
		CopyQualitiesFloorId(pCapStruct->floorid_1, pOtherCapStruct->floorid_1);
		CopyQualitiesFloorId(pCapStruct->floorid_2, pOtherCapStruct->floorid_2);
		CopyQualitiesFloorId(pCapStruct->floorid_3, pOtherCapStruct->floorid_3);
		pCapStruct->xbfcp_info_enabled = pOtherCapStruct->xbfcp_info_enabled;
		pCapStruct->xbfcp_info_time = pOtherCapStruct->xbfcp_info_time;
		pCapStruct->setup = pOtherCapStruct->setup;
		pCapStruct->connection = pOtherCapStruct->connection;
		pCapStruct->mstreamType = pOtherCapStruct->mstreamType;
		pCapStruct->transType = pOtherCapStruct->transType;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::CopyQualitiesFloorId(bfcpFlooridStruct &hBfcpFlooridStruct, bfcpFlooridStruct &hOtherBfcpFlooridStruct)
{
	strncpy(hBfcpFlooridStruct.floorid, hOtherBfcpFlooridStruct.floorid, BFCP_MAX_FLOORID_LENGTH-1);
	hBfcpFlooridStruct.floorid[BFCP_MAX_FLOORID_LENGTH-1] = '\0';
	strncpy(hBfcpFlooridStruct.m_stream_0, hOtherBfcpFlooridStruct.m_stream_0, BFCP_MAX_STREAM_LENGTH-1);
	hBfcpFlooridStruct.m_stream_0[BFCP_MAX_STREAM_LENGTH-1] = '\0';
	strncpy(hBfcpFlooridStruct.m_stream_1, hOtherBfcpFlooridStruct.m_stream_1, BFCP_MAX_STREAM_LENGTH-1);
	hBfcpFlooridStruct.m_stream_1[BFCP_MAX_STREAM_LENGTH-1] = '\0';
	strncpy(hBfcpFlooridStruct.m_stream_2, hOtherBfcpFlooridStruct.m_stream_2, BFCP_MAX_STREAM_LENGTH-1);
	hBfcpFlooridStruct.m_stream_2[BFCP_MAX_STREAM_LENGTH-1] = '\0';
	strncpy(hBfcpFlooridStruct.m_stream_3, hOtherBfcpFlooridStruct.m_stream_3, BFCP_MAX_STREAM_LENGTH-1);
	hBfcpFlooridStruct.m_stream_3[BFCP_MAX_STREAM_LENGTH-1] = '\0';
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetFloorCntl(eBfcpFloorCtrl bfcpFloorCtrl)
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
		pCapStruct->floorctrl = bfcpFloorCtrl;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
eBfcpFloorCtrl CBfcpCap::GetFloorCntl() const
{
	eBfcpFloorCtrl floorcntl = bfcp_flctrl_null;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct)
		floorcntl = (eBfcpFloorCtrl)pCapStruct->floorctrl;
	return floorcntl;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetConfId(char* pConfId)
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct && pConfId)
	{
		strncpy(pCapStruct->confid, pConfId, BFCP_MAX_CONFID_LENGTH-1);
		pCapStruct->confid[BFCP_MAX_CONFID_LENGTH-1] = '\0';
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* CBfcpCap::GetConfId() const
{
	char* pConfid = NULL;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct)
		pConfid = pCapStruct->confid;
	return pConfid;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetUserId(char* pUserId)
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct && pUserId)
	{
		strncpy(pCapStruct->userid, pUserId, BFCP_MAX_USERID_LENGTH-1);
		pCapStruct->userid[BFCP_MAX_USERID_LENGTH-1] = '\0';
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetConfId(WORD confId)
{
	char buffer[BFCP_MAX_CONFID_LENGTH];
	memset(buffer,0,BFCP_MAX_CONFID_LENGTH);
	sprintf(buffer,"%d",confId);
	SetConfId(buffer);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetUserId(WORD userId)
{
	char buffer[BFCP_MAX_USERID_LENGTH];
	memset(buffer,0,BFCP_MAX_USERID_LENGTH);
	sprintf(buffer,"%d",userId);
	SetUserId(buffer);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* CBfcpCap::GetUserId() const
{
	char* pUserid = NULL;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct)
		pUserid = pCapStruct->userid;
	return pUserid;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetInfoEnabled(BYTE bInfoEnabled)
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
		pCapStruct->xbfcp_info_enabled = bInfoEnabled;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CBfcpCap::GetInfoEnabled() const
{
	BYTE bInfoEnabled = FALSE;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct)
		bInfoEnabled = pCapStruct->xbfcp_info_enabled;
	return bInfoEnabled;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetInfoTime(APIU16 uInfoTime)
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
		pCapStruct->xbfcp_info_time = uInfoTime;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CBfcpCap::GetInfoTime() const
{
	BYTE uInfoTime = 0;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct)
		uInfoTime = pCapStruct->xbfcp_info_time;
	return uInfoTime;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetFloorIdParams(int floorIndex, char* pFloorId, char* pStreamId0, char* pStreamId1, char* pStreamId2, char* pStreamId3)
{
	bfcpFlooridStruct*	pFlooridStruct;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
	{
		if (floorIndex==0)
			pFlooridStruct = &pCapStruct->floorid_0;
		else if (floorIndex==1)
			pFlooridStruct = &pCapStruct->floorid_1;
		else if (floorIndex==2)
			pFlooridStruct = &pCapStruct->floorid_2;
		else if (floorIndex==3)
			pFlooridStruct = &pCapStruct->floorid_3;
		else
			return;

		if(pFloorId)
		{
			strncpy(pFlooridStruct->floorid, pFloorId, BFCP_MAX_FLOORID_LENGTH-1);
			pFlooridStruct->floorid[BFCP_MAX_FLOORID_LENGTH-1] = '\0';
		}
		if(pStreamId0)
		{
			strncpy(pFlooridStruct->m_stream_0, pStreamId0, BFCP_MAX_STREAM_LENGTH-1);
			pFlooridStruct->m_stream_0[BFCP_MAX_STREAM_LENGTH-1] = '\0';
		}
		if(pStreamId1)
		{
			strncpy(pFlooridStruct->m_stream_1, pStreamId1, BFCP_MAX_STREAM_LENGTH-1);
			pFlooridStruct->m_stream_1[BFCP_MAX_STREAM_LENGTH-1] = '\0';
		}
		if(pStreamId2)
		{
			strncpy(pFlooridStruct->m_stream_2, pStreamId2, BFCP_MAX_STREAM_LENGTH-1);
			pFlooridStruct->m_stream_2[BFCP_MAX_STREAM_LENGTH-1] = '\0';
		}
		if(pStreamId3)
		{
			strncpy(pFlooridStruct->m_stream_3, pStreamId3, BFCP_MAX_STREAM_LENGTH-1);
			pFlooridStruct->m_stream_3[BFCP_MAX_STREAM_LENGTH-1] = '\0';
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::GetFloorIdParams(int floorIndex, char* pFloorId, char* pStreamId0, char* pStreamId1, char* pStreamId2, char* pStreamId3)
{
	pFloorId = NULL;
	pStreamId0 = NULL;
	pStreamId1 = NULL;
	pStreamId2 = NULL;
	pStreamId3 = NULL;

	bfcpFlooridStruct*	pFlooridStruct;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
	{
		if (floorIndex==0)
			pFlooridStruct = &pCapStruct->floorid_0;
		else if (floorIndex==1)
			pFlooridStruct = &pCapStruct->floorid_1;
		else if (floorIndex==2)
			pFlooridStruct = &pCapStruct->floorid_2;
		else if (floorIndex==3)
			pFlooridStruct = &pCapStruct->floorid_3;
		else
			return;

		pFloorId = pFlooridStruct->floorid;
		pStreamId0 = pFlooridStruct->m_stream_0;
		pStreamId1 = pFlooridStruct->m_stream_1;
		pStreamId2 = pFlooridStruct->m_stream_2;
		pStreamId3 = pFlooridStruct->m_stream_3;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetSetup(APIU8 setup)
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
		pCapStruct->setup = setup;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CBfcpCap::GetSetup() const
{
	BYTE setup = (BYTE)bfcp_setup_null;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct)
		setup = pCapStruct->setup;
	return setup;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetConnection(APIU8 connection)
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
		pCapStruct->connection = connection;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CBfcpCap::GetConnection() const
{
	BYTE connection = (BYTE)bfcp_connection_null;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct)
		connection = pCapStruct->connection;
	return connection;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetMStreamType(eBfcpMStreamType mstreamType)
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
		pCapStruct->mstreamType = (APIU8)mstreamType;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CBfcpCap::SetTransportType(enTransportType transType)
{
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
		pCapStruct->transType = transType;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
enTransportType CBfcpCap::GetTransportType() const
{
	enTransportType transType = eUnknownTransportType;

	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);

	if (pCapStruct)
		transType = pCapStruct->transType;

	return transType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
eBfcpMStreamType CBfcpCap::GetMStreamType() const
{
	eBfcpMStreamType mstreamType = bfcp_m_stream_None;
	bfcpCapStruct* pCapStruct = CAP_CAST(bfcpCapStruct);
	if (pCapStruct)
		mstreamType = (eBfcpMStreamType)(pCapStruct->mstreamType);
	return mstreamType;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CBfcpCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x00000000;

	bfcpCapStruct *pCapStruct 		= CAP_CAST(bfcpCapStruct);
	bfcpCapStruct *pOtherCapStruct 	= (bfcpCapStruct *)other.GetStruct();

	PTRACE2INT(eLevelError, "CBfcpCap::IsContaining - valuesToCompare - ", valuesToCompare);

	if (pCapStruct && pOtherCapStruct)
	{
		bRes = CBaseCap::IsContaining(other,valuesToCompare,pDetails);

		if ((valuesToCompare & kTransportType) && bRes)
		{
			if (pCapStruct->transType != pOtherCapStruct->transType)
			{
				CLargeString str;
				str << "this transport type:" << pCapStruct->transType << ", other transport type: " << pOtherCapStruct->transType;

				PTRACE2(eLevelError, "CBfcpCap::IsContaining - differnet BFCP transport types - ", str.GetString());
				*pDetails |= DIFFERENT_TRANSPORT_TYPE;
				bRes = FALSE;
			}
			else
				PTRACE2INT(eLevelError, "CBfcpCap::IsContaining - same BFCP transport type:", pCapStruct->transType);
		}
	}
	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CAAC_LDCap
//=======================
// TIP
/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CAAC_LDCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(AAC_LDCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(AAC_LDCapStruct));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
size_t CAAC_LDCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(AAC_LDCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::Dump(std::ostream& msg) const
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
	{
//		pCapStruct->mode[MAX_AACLD_MODE] 		= '\0';
//		pCapStruct->config[MAX_AACLD_CONFIG] 	= '\0';

		CBaseCap::Dump(msg);

		msg << "profile-level-id		= " << pCapStruct->profileLevelId;
		msg << "\nstream type				= " << pCapStruct->streamType;
		msg << "\nmode					= " << pCapStruct->mode;
		msg << "\nconfig               	= " << pCapStruct->config;
		msg << "\nsize length				= " << pCapStruct->sizeLength;
		msg << "\nindex length			= " << pCapStruct->indexLength;
		msg << "\nindex delta length		= " << pCapStruct->indexDeltaLength;
		msg << "\nconstant duration		= " << pCapStruct->constantDuration;
		msg << "\nmaxBitRate				= " << pCapStruct->maxBitRate;
		msg << "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CAAC_LDCap::SetDefaults(cmCapDirection eDirection, ERoleLabel eRole)
{
	EResult eRes = kFailure;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio, eDirection, eRole);

		strncpy(pCapStruct->mimeType, "mpeg4-generic", MAX_AACLD_MIME_TYPE);
		pCapStruct->sampleRate			= 48000;
		pCapStruct->profileLevelId		= 16;
		pCapStruct->streamType			= 5;
		strncpy(pCapStruct->mode, "AAC-hbr", MAX_AACLD_MODE);
		strncpy(pCapStruct->config, "B98C00", MAX_AACLD_CONFIG);// or B98C00;
		pCapStruct->sizeLength			= 13;
		pCapStruct->indexLength			= 3;
		pCapStruct->indexDeltaLength	= 3;
		pCapStruct->constantDuration 	= 480;
		pCapStruct->maxBitRate			= 2560;// 256kbps *10
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CAAC_LDCap::SetStruct(cmCapDirection eDirection,int maxValue,int minValue)
{
	EResult eRes = kFailure;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio, eDirection);

		strncpy(pCapStruct->mimeType, "mpeg4-generic", MAX_AACLD_MIME_TYPE);
		pCapStruct->sampleRate			= 48000;
		pCapStruct->profileLevelId		= 16;
		pCapStruct->streamType			= 5;
		strncpy(pCapStruct->mode, "AAC-hbr", MAX_AACLD_MODE);
		strncpy(pCapStruct->config, "B98C00", MAX_AACLD_CONFIG);// or B98C00;
		pCapStruct->sizeLength			= 13;
		pCapStruct->indexLength			= 3;
		pCapStruct->indexDeltaLength	= 3;
		pCapStruct->constantDuration 	= 480;
		pCapStruct->maxBitRate			= 2560;// 64kbps *10
	}
	return eRes;


	// why not:
	// eRes &= CBaseCap::SetDefaults(eDirection);	// it is not compile
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CAAC_LDCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	AAC_LDCapStruct *pCapStruct = CAP_CAST(AAC_LDCapStruct);
	CAAC_LDCap &hOtherVideoCap  = (CAAC_LDCap&)otherCap;

	if (GetCapCode() == hOtherVideoCap.GetCapCode())
	{
		eRes = kSuccess;
		char *mimeType = hOtherVideoCap.GetMimeType();
		if (mimeType)
		{
			strncpy(pCapStruct->mimeType, mimeType, sizeof(pCapStruct->mimeType) - 1);
			pCapStruct->mimeType[sizeof(pCapStruct->mimeType) - 1] = '\0';
		}
		else
			PTRACE(eLevelInfoNormal,"CAAC_LDCap::CopyQualities hOtherVideoCap.GetMimeType() is NULL");
		pCapStruct->sampleRate    		= hOtherVideoCap.GetSampleRate();
		pCapStruct->profileLevelId    	= hOtherVideoCap.GetProfileLevelId();
		pCapStruct->streamType	       	= hOtherVideoCap.GetStreamType();
		char *mode =  hOtherVideoCap.GetMode();
		if ( mode )
			strncpy(pCapStruct->mode, mode, sizeof(pCapStruct->mode) - 1);
		else
			PTRACE(eLevelInfoNormal,"CAAC_LDCap::CopyQualities hOtherVideoCap.GetMode() is NULL");
		pCapStruct->mode[sizeof(pCapStruct->mode) - 1] = '\0';
		char *config = hOtherVideoCap.GetConfig();
		if ( config )
			strncpy(pCapStruct->config, config, sizeof(pCapStruct->config) - 1);
		else
			PTRACE(eLevelInfoNormal,"CAAC_LDCap::CopyQualities hOtherVideoCap.GetConfig() is NULL");
		pCapStruct->config[sizeof(pCapStruct->config) - 1] = '\0';

		pCapStruct->sizeLength			= hOtherVideoCap.GetSizeLength();
		pCapStruct->indexLength    		= hOtherVideoCap.GetIndexLength();
		pCapStruct->indexDeltaLength    = hOtherVideoCap.GetIndexDeltaLength();
		pCapStruct->constantDuration	= hOtherVideoCap.GetConstantDuration();
		pCapStruct->maxBitRate			= hOtherVideoCap.GetBitRate();
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CBaseCap* CAAC_LDCap::GetHighestCommon(const CBaseCap & otherCap) const
{
	CAAC_LDCap* pRes = NULL;

	if (m_pCapStruct->header.capTypeCode == ((const CAAC_LDCap&)otherCap).m_pCapStruct->header.capTypeCode)
	{
		pRes = (CAAC_LDCap*)CBaseCap::AllocNewCap((CapEnum)m_pCapStruct->header.capTypeCode,NULL);
		if(pRes)
			pRes->SetDefaults((cmCapDirection)m_pCapStruct->header.direction,(ERoleLabel)m_pCapStruct->header.roleLabel);
		else
		{
			PTRACE(eLevelInfoNormal,"CAAC_LDCap::GetHighestCommon: pRes is NULL!");
			return pRes;
		}

	}
	else
	{
		PTRACE(eLevelInfoNormal,"CAAC_LDCap::GetHighestCommon: No common AAC_LD format!");
	}

	return pRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the structure max frame per packet.
//---------------------------------------------------------------------------------------------------
//EResult CAAC_LDCap::SetMaxFramePerPacket(int newValue)
//{
//	EResult eRes = kFailure;
//	AAC_LDCapStruct *pCapStruct = CAP_CAST(AAC_LDCapStruct);
//	if (pCapStruct)
//	{
//		pCapStruct->maxValue = newValue;
//		eRes = kSuccess;
//	}
//	return eRes;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////
////Description: Sets the structure min frame per packet.
////---------------------------------------------------------------------------------------------------
//EResult CAAC_LDCap::SetMinFramePerPacket(int newValue)
//{
//	EResult eRes = kFailure;
//	AAC_LDCapStruct *pCapStruct = CAP_CAST(AAC_LDCapStruct);
//	if (pCapStruct)
//	{
//		pCapStruct->minValue = newValue;
//		eRes = kSuccess;
//	}
//	return eRes;
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetMimeType(char *psMimeType)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
	{
		strncpy(pCapStruct->mimeType, psMimeType, sizeof(pCapStruct->mimeType) - 1);
		pCapStruct->mimeType[sizeof(pCapStruct->mimeType) - 1] = '\0';
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* CAAC_LDCap::GetMimeType() const
{
	char* pMimeType = NULL;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		pMimeType = pCapStruct->mimeType;
	return pMimeType;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetSampleRate(APIU32 sampleRate)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
		pCapStruct->sampleRate = sampleRate;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CAAC_LDCap::GetSampleRate() const
{
	APIU32 sampleRate = 0;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		sampleRate = pCapStruct->sampleRate;
	return sampleRate;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetProfileLevelId(WORD profileLevelId)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
		pCapStruct->profileLevelId = profileLevelId;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CAAC_LDCap::GetProfileLevelId() const
{
	APIU16 profileLevelId = 0;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		profileLevelId = pCapStruct->profileLevelId;
	return profileLevelId;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetStreamType(APIU16 streamType)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
		pCapStruct->streamType = streamType;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CAAC_LDCap::GetStreamType() const
{
	APIU16 streamType = 0;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		streamType = pCapStruct->streamType;
	return streamType;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetMode(char *psMode)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
	{
		strncpy(pCapStruct->mode, psMode, sizeof(pCapStruct->mode) - 1);
		pCapStruct->mode[sizeof(pCapStruct->mode) - 1] = '\0';
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* CAAC_LDCap::GetMode() const
{
	char* pMode = NULL;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		pMode = pCapStruct->mode;
	return pMode;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetConfig(char *psConfig)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
	{
		strncpy(pCapStruct->config, psConfig, sizeof(pCapStruct->config) - 1);
		pCapStruct->config[sizeof(pCapStruct->config) - 1] = '\0';
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* CAAC_LDCap::GetConfig() const
{
	char* pConfig = NULL;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		pConfig = pCapStruct->config;
	return pConfig;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetSizeLength(APIU16 sizeLen)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
		pCapStruct->sizeLength = sizeLen;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CAAC_LDCap::GetSizeLength() const
{
	APIU16 sizeLen = 0;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		sizeLen = pCapStruct->sizeLength;
	return sizeLen;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetIndexLength(APIU16 indexLen)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
		pCapStruct->indexLength = indexLen;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CAAC_LDCap::GetIndexLength() const
{
	APIU16 indexLen = 0;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		indexLen = pCapStruct->indexLength;
	return indexLen;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetIndexDeltaLength(APIU16 indexDeltaLen)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
		pCapStruct->indexDeltaLength = indexDeltaLen;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU16 CAAC_LDCap::GetIndexDeltaLength() const
{
	APIU16 indexDeltaLen = 0;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		indexDeltaLen = pCapStruct->indexDeltaLength;
	return indexDeltaLen;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAAC_LDCap::SetConstantDuration(APIU32 constantDuration)
{
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
		pCapStruct->constantDuration = constantDuration;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CAAC_LDCap::GetConstantDuration() const
{
	APIU32 constantDuration = 0;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);
	if (pCapStruct)
		constantDuration = pCapStruct->constantDuration;
	return constantDuration;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CAAC_LDCap::SetBitRate(APIS32 rate)
{
	EResult eRes = kFailure;

	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxBitRate 	= rate;
		eRes 					= kSuccess;
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CAAC_LDCap::GetBitRate() const
{
	APIS32 maxBitRate = 0;
	AAC_LDCapStruct* pCapStruct = CAP_CAST(AAC_LDCapStruct);

	if (pCapStruct)
		maxBitRate = pCapStruct->maxBitRate;

	return maxBitRate;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CRtvVideoCap
//=======================


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CRtvVideoCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(rtvCapStruct);
	return size;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CRtvVideoCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(rtvCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CRtvVideoCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(rtvCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(rtvCapStruct));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values
//---------------------------------------------------------------------------------------------------
EResult CRtvVideoCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);

		pCapStruct->numOfItems = 0;

		for(int i=0;i<NumOfRtvItems;i++)
		{
			(pCapStruct->rtvCapItem[i]).capabilityID = 0;

			(pCapStruct->rtvCapItem[i]).widthVF = 0;
			(pCapStruct->rtvCapItem[i]).heightVF = 0;
			(pCapStruct->rtvCapItem[i]).fps = 0;
			(pCapStruct->rtvCapItem[i]).maxBitrateInBps = 0;

		}

	}
	return eRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::SetFpsToAllItems(cmCapDirection eDirection,ERoleLabel eRole, unsigned long FR)
{
	EResult eRes = kFailure;
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;

		WORD numOfItems = (int)pCapStruct->numOfItems;

		for(int i=0;i<numOfItems;i++)
		{

			(pCapStruct->rtvCapItem[i]).fps = FR;

		}

	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);

	CRtvVideoCap &hOtherVideoCap  = (CRtvVideoCap&)otherCap;
	rtvCapStruct *pOtherCapStruct	 = (rtvCapStruct *)otherCap.GetStruct();

	if (GetCapCode() == hOtherVideoCap.GetCapCode())
	{
		eRes = kSuccess;

		pCapStruct->numOfItems = pOtherCapStruct->numOfItems;

		for(int i=0;i<NumOfRtvItems;i++)
		{
			(pCapStruct->rtvCapItem[i]).capabilityID = (pOtherCapStruct->rtvCapItem[i]).capabilityID;
			(pCapStruct->rtvCapItem[i]).widthVF = (pOtherCapStruct->rtvCapItem[i]).widthVF;
			(pCapStruct->rtvCapItem[i]).heightVF = (pOtherCapStruct->rtvCapItem[i]).heightVF;
			(pCapStruct->rtvCapItem[i]).fps = (pOtherCapStruct->rtvCapItem[i]).fps;
			(pCapStruct->rtvCapItem[i]).maxBitrateInBps = (pOtherCapStruct->rtvCapItem[i]).maxBitrateInBps;
		}

	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CRtvVideoCap::Dump(std::ostream& msg) const
{
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);

	if (pCapStruct)
	{
		WORD numOfItems = (int)pCapStruct->numOfItems;

		if(numOfItems > 0)
		{
			CBaseCap::Dump(msg);

			msg << "numOfItems  = " << numOfItems;

			for(int i=0;(i<numOfItems && i<NumOfRtvItems);i++)
			{
				if((pCapStruct->rtvCapItem[i]).capabilityID != 0)
				{
					msg << "\nRtvItem                     = " << i;
					msg << "\ncapabilityID                = " << (pCapStruct->rtvCapItem[i]).capabilityID;
					msg << "\nwidthVF                     = " << (pCapStruct->rtvCapItem[i]).widthVF;
					msg << "\nheightVF                    = " << (pCapStruct->rtvCapItem[i]).heightVF;
					msg << "\nfps                         = " << (pCapStruct->rtvCapItem[i]).fps;
					msg << "\nmaxBitrateInBps             = " << (pCapStruct->rtvCapItem[i]).maxBitrateInBps <<
					" (" << ((pCapStruct->rtvCapItem[i]).maxBitrateInBps)*100 << " bps)";
				}
			}
		} else {
			PTRACE(eLevelInfoNormal,"CRtvVideoCap::Dump pCapStruct->numOfItems = 0");
		}
		msg << "\n";
	} else {
		PTRACE(eLevelInfoNormal,"CRtvVideoCap::Dump pCapStruct is null");
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CRtvVideoCap::GetBitRate() const
{

	APIS32 bitRate = -1;
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);

	if (pCapStruct)
	{
		rtvCapItemS RtvMaxCapItem;
		EResult eRes = FindMaxCapInCapSet(pCapStruct,RtvMaxCapItem);

		if(eRes )
			bitRate = RtvMaxCapItem.maxBitrateInBps;

	}
	return bitRate;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::GetBitRateAccordingToResolution(DWORD width, DWORD height, DWORD& bitRate) const
{
	//PTRACE(eLevelInfoNormal,"CRtvVideoCap::GetBitRateAccordingToResolution");
	//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetBitRateAccordingToResolution width", width);
	//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetBitRateAccordingToResolution height", height);

	EResult eRes = kFailure;
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);

	if (pCapStruct)
		eRes = GetBitRateAccordingToResolutionInCapSet(pCapStruct,width,height, bitRate);

	//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetBitRateAccordingToResolution bitRate", bitRate);

	TRACEINTO << "width:" << width << ", height:" << ", bitRate:" << bitRate;

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EFormat CRtvVideoCap::GetFormat() const
{
	EFormat format = kUnknownFormat;
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);
	if (pCapStruct)
	{
		rtvCapItemS RtvMaxCapItem;
		EResult eRes = FindMaxCapInCapSet(pCapStruct,RtvMaxCapItem);

		if(eRes )
		{
			APIS32 fs = ((RtvMaxCapItem.widthVF) * (RtvMaxCapItem.heightVF))/256;

			format = GetFormatAccordingToFS(fs);
		}

	}


	return format;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS8 CRtvVideoCap::GetFormatMpi(EFormat eFormat) const
{
	return 1;  // to check if we use this function in H264 and RTV.
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CRtvVideoCap::GetFrameRateForRTV() const
{
	EFormat format = kUnknownFormat;
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);
	if (pCapStruct)
	{
		rtvCapItemS RtvMaxCapItem;
		EResult eRes = FindMaxCapInCapSet(pCapStruct,RtvMaxCapItem);

		if(eRes )
		{
			DWORD FrameRate = RtvMaxCapItem.fps;
			PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetFrameRateForRTV - FrameRate",FrameRate);

			return FrameRate;

		}
	}
	return 0;


}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::SetContent(ERoleLabel eRole, cmCapDirection eDirection,BOOL isHD1080, BYTE HDMpi, BOOL isHighProfile)
{
	EResult eRes = kFailure;
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);
	if (pCapStruct)
	{
		//atara: for now we don't support
		SetDefaults(eDirection, eRole);
	}
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtvVideoCap::SetRtvWidthAndHeight(APIS32 width, APIS32 Height,APIS32 Fps,DWORD BitRate)
{
	CSmallString msg;
	msg << " Width: " << width << " ,Height: " << Height << " ,Fps: " << Fps << " ,BitRate: " << BitRate;
	PTRACE2(eLevelInfoNormal,"CRtvVideoCap::SetRtvWidthAndHeight ",msg.GetString());

	//There is a problem with this function - there can be only RtvItem here
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);

	if (pCapStruct)
	{
		pCapStruct->numOfItems = 0;

		for(int i=0;i<NumOfRtvItems;i++)
		{
			if((pCapStruct->rtvCapItem[i]).capabilityID == 0)
			{
				(pCapStruct->rtvCapItem[i]).capabilityID = i+1 ;
				(pCapStruct->rtvCapItem[i]).widthVF = width;
				(pCapStruct->rtvCapItem[i]).heightVF = Height;
				(pCapStruct->rtvCapItem[i]).fps = Fps;
				(pCapStruct->rtvCapItem[i]).maxBitrateInBps = BitRate;

				(pCapStruct->numOfItems)++;
				break;
			}

		}
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult  CRtvVideoCap::SetRtvCapForCpFromH264VideoType(Eh264VideoModeType H264VideoModeType,int videoLineRate,BYTE maxResolution)
{
	EResult eRes = kSuccess;
	RTVVideoModeDetails rtvVidModeDetails;

	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);
//	rtvCapItemS* capItem;

	if (pCapStruct)
	{
		pCapStruct->numOfItems = 0;

		for(int i=0;i<NumOfRtvItems;i++)
		{
			(pCapStruct->rtvCapItem[i]).capabilityID = 0;
			(pCapStruct->rtvCapItem[i]).widthVF = 0;
			(pCapStruct->rtvCapItem[i]).heightVF = 0;
			(pCapStruct->rtvCapItem[i]).fps = 0;
			(pCapStruct->rtvCapItem[i]).maxBitrateInBps = 0;
		}
	}

	// Shmulik: BRIDGE-275 patch for HD on 768kbps conf
	if ((eSD15 <= H264VideoModeType) && (H264VideoModeType <= eW4CIF30) && (7680 == videoLineRate) &&
				maxResolution != eCIF_Res && maxResolution != eSD_Res)
		{
			PTRACE(eLevelInfoNormal,"CRtvVideoCap::SetRtvCapForCpFromH264VideoType - setting RTV to HD on 768 rate !");
			H264VideoModeType = eHD720Asymmetric;
		}

	CRtvVideoMode::GetRtvVideoParams(rtvVidModeDetails,H264VideoModeType);

	TRACEINTO << "CRtvVideoCap::SetRtvCapForCpFromH264VideoTyp "
	          << "- H264VideoModeType:" << H264VideoModeType
	          << ", VideoModeType:"     << rtvVidModeDetails.videoModeType
	          << ", Width:"             << rtvVidModeDetails.Width
	          << ", Height:"            << rtvVidModeDetails.Height
	          << ", FrameRate:"         << rtvVidModeDetails.FR
	          << ", VideoLineRate:"     << videoLineRate;

	//In RTV we declare 3 video resolutions according to rate, HD720, SD30 , CIF30
	AddRTVcapsToCapset(H264VideoModeType, pCapStruct, videoLineRate);

	return eRes;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CRtvVideoCap::AddRTVcapsToCapset(Eh264VideoModeType MaxVideoModeType,rtvCapStruct* pCapStruct,int videoLineRate)
{
	PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::AddRTVcapsToCapset -  MaxVideoModeType: ",MaxVideoModeType);

	RTVVideoModeDetails rtvVidModeDetails;
	CRtvVideoMode::GetRtvVideoParams(rtvVidModeDetails,MaxVideoModeType);
	AddRTVcapsToCapsetByVideoMode(rtvVidModeDetails, videoLineRate, pCapStruct);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CRtvVideoCap::AddRTVcapsToCapsetByVideoMode(RTVVideoModeDetails rtvVidModeDetails,int videoLineRate,rtvCapStruct* pCapStruct)
{
	PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::AddRTVcapsToCapsetByVideoMode -  videoLineRate: ",videoLineRate);

	std::string max_rtv_protocol_str;
    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL res = pSysConfig->GetDataByKey(CFG_MAX_RTV_RESOLUTION, max_rtv_protocol_str);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetDataByKey: " << CFG_MAX_RTV_RESOLUTION);
	
	CSmallString msg;
	msg << "MAX_RTV_RESOLUTION sys flag is: " << max_rtv_protocol_str << " videoModeType: " << rtvVidModeDetails.videoModeType;
	PTRACE2(eLevelInfoNormal,"CRtvVideoCap::AddRTVcapsToCapsetByVideoMode: ",msg.GetString());

	if (rtvVidModeDetails.videoModeType >= eLastRtvVideoMode)
	{
		PTRACE2INT(eLevelError, "CRtvVideoCap::AddRTVcapsToCapsetByVideoMode invalid, setting to CIF, rtv videoModeType: ", rtvVidModeDetails.videoModeType);
		DBGPASSERT(1);
		rtvVidModeDetails.videoModeType = e_rtv_CIF30;
	}
	BOOL bForceVga = ("VGA" == max_rtv_protocol_str) || (rtvVidModeDetails.videoModeType <= e_rtv_SD30);;
	BOOL bForceCif = ("CIF" == max_rtv_protocol_str) || (rtvVidModeDetails.videoModeType <= e_rtv_CIF30);
	BOOL bForceQCif = ("QCIF" == max_rtv_protocol_str) || (rtvVidModeDetails.videoModeType <= e_rtv_QCIF30);;

	PTRACE2(eLevelInfoNormal,"CRtvVideoCap::AddRTVcapsToCapsetByVideoMode - max videoModeType: ", eVideoPartyTypeNames[rtvVidModeDetails.videoModeType]);

	DWORD rate = videoLineRate;
	if (videoLineRate > RTV_MAX_RATE_VGA && !bForceVga && !bForceCif && !bForceQCif)
	{
		PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::AddRTVcapsToCapset Add HD rate=",rate);
		CRtvVideoMode::GetRtvVideoParams(rtvVidModeDetails,eHD720Symmetric);
		SetRtvCap(rtvVidModeDetails,rate,pCapStruct);
	}
	else if (videoLineRate > RTV_MAX_RATE_CIF && !bForceCif && !bForceQCif)
	{
		rate = (videoLineRate > RTV_MAX_RATE_VGA && !bForceVga) ? RTV_MAX_RATE_VGA : videoLineRate;
		PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::AddRTVcapsToCapset Add eSD30 rate=",rate);
		CRtvVideoMode::GetRtvVideoParams(rtvVidModeDetails,eSD30);
		SetRtvCap(rtvVidModeDetails,rate,pCapStruct);
	}
	else if (videoLineRate > RTV_MAX_RATE_QCIF && !bForceQCif)
	{
		rate = (videoLineRate > RTV_MAX_RATE_CIF && !bForceCif) ? RTV_MAX_RATE_CIF : videoLineRate;
		PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::AddRTVcapsToCapset Add eCIF30 rate=",rate);
		CRtvVideoMode::GetRtvVideoParams(rtvVidModeDetails,eCIF30);
		SetRtvCap(rtvVidModeDetails,rate,pCapStruct);
	}
	else
	{
		rate = (videoLineRate > RTV_MAX_RATE_QCIF && !bForceQCif) ? RTV_MAX_RATE_QCIF : videoLineRate;
		PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::AddRTVcapsToCapset Add e_rtv_QCIF30 rate=",rate);
		CRtvVideoMode::GetRtvQcifVideoParams(rtvVidModeDetails);
		SetRtvCap(rtvVidModeDetails,rate,pCapStruct);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CRtvVideoCap::SetRtvCapForCpFromRtvVideoType(RTVVideoModeDetails rtvVidModeDetails,int videoLineRate)
{
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);
	AddRTVcapsToCapsetByVideoMode(rtvVidModeDetails, videoLineRate, pCapStruct);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::SetRtvCap(RTVVideoModeDetails rtvVidModeDetails,int videoLineRate,rtvCapStruct* pCapStruct)  //Set local caps
{
	EResult eRes = kFailure;

	DWORD rate = (videoLineRate > RTV_MAX_RATE_HD) ? RTV_MAX_RATE_HD :videoLineRate;
	PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::SetRtvCap rate=", rate);

	if (pCapStruct)
	{
		for(int i=0;i<NumOfRtvItems;i++)
		{
			if((pCapStruct->rtvCapItem[i]).capabilityID)
			{
				PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::SetRtvCap occupied i:", i);
			}
			else
			{
				//rtvCapItemS* capItem;
				PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::SetRtvCap Free i:", i);

				(pCapStruct->rtvCapItem[i]).capabilityID = i+1;
				(pCapStruct->rtvCapItem[i]).widthVF = rtvVidModeDetails.Width;
				(pCapStruct->rtvCapItem[i]).heightVF = rtvVidModeDetails.Height;
				(pCapStruct->rtvCapItem[i]).fps = rtvVidModeDetails.FR;

				if(videoLineRate)
					(pCapStruct->rtvCapItem[i]).maxBitrateInBps = rate;

				(pCapStruct->numOfItems)++;
				eRes = kSuccess;
				break;
			}
		}
//		(pCapStruct->rtvCapItem[0]).capabilityID = 1;
//		(pCapStruct->rtvCapItem[0]).widthVF = rtvVidModeDetails.Width;
//		(pCapStruct->rtvCapItem[0]).heightVF = rtvVidModeDetails.Height;
//		(pCapStruct->rtvCapItem[0]).fps = rtvVidModeDetails.FR;

	//	if(videoLineRate)
	//		(pCapStruct->rtvCapItem[0]).maxBitrateInBps = videoLineRate;

	//	(pCapStruct->numOfItems) = 1;
	//	eRes = kSuccess;

	}

	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::GetRtvCap(RTVVideoModeDetails& rtvVidModeDetails,DWORD& BitRate)
{
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);
	EResult eRes = kFailure;

	PTRACE(eLevelInfoNormal,"CRtvVideoCap::GetRtvCap ");

	if (pCapStruct)
	{
		rtvCapItemS RtvMaxCapItem;
		eRes = FindMaxCapInCapSet(pCapStruct,RtvMaxCapItem);

		if(eRes )
		{
			rtvVidModeDetails.Width = RtvMaxCapItem.widthVF;
			rtvVidModeDetails.Height = RtvMaxCapItem.heightVF;
			rtvVidModeDetails.FR = RtvMaxCapItem.fps;
			BitRate = RtvMaxCapItem.maxBitrateInBps;
			
			if (rtvVidModeDetails.Width*rtvVidModeDetails.Height >= 1280*720)
			{
				rtvVidModeDetails.videoModeType = e_rtv_HD720Symmetric;
			}
			else if (rtvVidModeDetails.Width*rtvVidModeDetails.Height >= 640*480)
			{
				rtvVidModeDetails.videoModeType = e_rtv_SD30;
			}
			else 
			{
				rtvVidModeDetails.videoModeType = e_rtv_CIF30;
			}
		}
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::GetRtvCapFRAccordingToFS(DWORD& FrameRate,DWORD FS)
{
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);
	EResult eRes = kFailure;
	FrameRate = 0;
	DWORD Resolution = 0;

	PTRACE(eLevelInfoNormal,"CRtvVideoCap::GetRtvCapAccordingToFS ");

	if (pCapStruct)
	{
		WORD numOfItems = pCapStruct->numOfItems;

		for(int i =0;i < numOfItems;i++)
		{
				if((pCapStruct->rtvCapItem[i]).capabilityID != 0)
				{
					Resolution = (((pCapStruct->rtvCapItem[i]).widthVF) * ((pCapStruct->rtvCapItem[i]).heightVF))/256;
					if(Resolution == FS)
						FrameRate = (pCapStruct->rtvCapItem[i]).fps;
				}
		}

		//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetRtvCapAccordingToFS - FrameRate",FrameRate);
		//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetRtvCapAccordingToFS - FS",FS);

		TRACEINTO << "FrameRate:" << FrameRate << ", FS:" << FS;

	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CRtvVideoCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{

	PTRACE(eLevelInfoNormal,"CRtvVideoCap::IsContaining");

	BYTE bRes  = TRUE;
	*pDetails = 0x0000;
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);
	rtvCapStruct* pOtherCapStruct = (rtvCapStruct *)other.GetStruct();

	DWORD thisResolution  = 0;
	DWORD otherResolution  = 0;

	WORD ThisFR = 0;
	WORD OtherFR = 0;

	DWORD ThisBitRate = 0;
	DWORD OtherBitRate = 0;

	if (pCapStruct && pOtherCapStruct && (pCapStruct->numOfItems > 0) && (pOtherCapStruct->numOfItems > 0))
	{
		PTRACE(eLevelInfoNormal,"CRtvVideoCap::IsContaining In 1");
		 // Starts the comparison. If in some stage res gets FALSE the comparison stops
		// and the value that "this" struct was lack of will be saved in the pDetails.
		bRes = CBaseCap::IsContaining(other,valuesToCompare,pDetails);
		if(bRes == FALSE)
			return bRes;
		PTRACE(eLevelInfoNormal,"CRtvVideoCap::IsContaining In 2");
		rtvCapItemS thisRtvMaxCapItem;
		EResult ethisRes =FindMaxCapInCapSet(pCapStruct,thisRtvMaxCapItem);

		rtvCapItemS otherRtvMaxCapItem;
		EResult eOtherRes =FindMaxCapInCapSet(pOtherCapStruct,otherRtvMaxCapItem);

		if(ethisRes && eOtherRes)
		{
			if (bRes && (valuesToCompare & kFormat))
			{
				thisResolution = (thisRtvMaxCapItem.widthVF) * (thisRtvMaxCapItem.heightVF);

				otherResolution = (otherRtvMaxCapItem.widthVF) * (otherRtvMaxCapItem.heightVF);

				//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::IsContaining thisResolution: ",thisResolution);
				//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::IsContaining otherResolution: ",otherResolution);

				TRACEINTO << "thisResolution:" << thisResolution << ", otherResolution:" << otherResolution;

				if (thisResolution < otherResolution)
				{
					PTRACE(eLevelInfoNormal,"CRtvVideoCap::IsContaining - HIGHER_FORMAT");
					bRes = FALSE;
					*pDetails |= HIGHER_FORMAT;
				}

			}

			if (bRes && (valuesToCompare & kFrameRate))
			{
				ThisFR = thisRtvMaxCapItem.fps;
				OtherFR = otherRtvMaxCapItem.fps;

				//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::IsContaining ThisFR: ",ThisFR);
				//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::IsContaining OtherFR: ",OtherFR);
				TRACEINTO << "ThisFR:" << ThisFR << ", OtherFR:" << OtherFR;

				if (ThisFR < OtherFR)
				{
					bRes = FALSE;
					*pDetails |= HIGHER_FRAME_RATE;
					PTRACE(eLevelInfoNormal,"CRtvVideoCap::IsContaining - HIGHER_FRAME_RATE");
				}

			}

			if ((valuesToCompare & kBitRate) && bRes)
			{
				ThisBitRate = thisRtvMaxCapItem.maxBitrateInBps;
				OtherBitRate = otherRtvMaxCapItem.maxBitrateInBps;

				//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::IsContaining ThisBitRate: ",ThisBitRate);
				//PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::IsContaining OtherBitRate: ",OtherBitRate);
				TRACEINTO << "ThisBitRate:" << ThisBitRate << ", OtherBitRate:" << OtherBitRate;

				if (ThisBitRate < OtherBitRate)
				{
					bRes = FALSE;
					*pDetails |= HIGHER_BIT_RATE;
					PTRACE(eLevelInfoNormal,"CRtvVideoCap::IsContaining - HIGHER_BIT_RATE");
				}
			}

		}
	}
	PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::IsContaining bRes ", bRes );
	return bRes;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::FindMaxCapInCapSet(rtvCapStruct* pCapStruct,rtvCapItemS& rtvCapItem) const
{

	EResult eRes = kFailure;
	DWORD Resolution = 0;
	DWORD MaxResolution = 25344; //QCIF (176*144)
	WORD  ResIndex = 0;
	WORD numOfItems = pCapStruct->numOfItems;

	for(int i =0;i < numOfItems;i++)
	{
		if((pCapStruct->rtvCapItem[i]).capabilityID != 0)
			Resolution = ((pCapStruct->rtvCapItem[i]).widthVF) * ((pCapStruct->rtvCapItem[i]).heightVF);

		if(Resolution >= MaxResolution )
		{
			MaxResolution = Resolution;
			ResIndex = i;
			eRes = kSuccess;
		}
	}

	if(eRes == kSuccess)
	{
		rtvCapItem.widthVF = (pCapStruct->rtvCapItem[ResIndex]).widthVF;
		rtvCapItem.heightVF = (pCapStruct->rtvCapItem[ResIndex]).heightVF;
		rtvCapItem.capabilityID = (pCapStruct->rtvCapItem[ResIndex]).capabilityID;
		rtvCapItem.fps = (pCapStruct->rtvCapItem[ResIndex]).fps;
		rtvCapItem.maxBitrateInBps = (pCapStruct->rtvCapItem[ResIndex]).maxBitrateInBps;
	}

	return eRes;

}
EResult CRtvVideoCap::GetBitRateAccordingToResolutionInCapSet(rtvCapStruct* pCapStruct,DWORD width, DWORD height, DWORD& bitRate) const
{
	//PTRACE(eLevelInfoNormal,"CRtvVideoCap::GetBitRateAccordingToResolutionInCapSet ");

	EResult eRes = kFailure;
	DWORD resolution = width * height;

	WORD numOfItems = pCapStruct->numOfItems;
	DWORD curResolution = 0;

	for(int i =0;i < numOfItems;i++)
	{
		if((pCapStruct->rtvCapItem[i]).capabilityID != 0)
			curResolution = ((pCapStruct->rtvCapItem[i]).widthVF) * ((pCapStruct->rtvCapItem[i]).heightVF);
		if(resolution >= curResolution)
		{
			bitRate = (pCapStruct->rtvCapItem[i]).maxBitrateInBps;
			eRes = kSuccess;
			break;
		}
	}
	PTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetBitRateAccordingToResolutionInCapSet bitRate ", bitRate );
	return eRes;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::SetRateInMaxCapInCapSet(rtvCapStruct* pCapStruct,APIS32 rate) const
{

	EResult eRes = kFailure;
	DWORD Resolution = 0;
	DWORD MaxResolution = 25344; //QCIF (176*144)
	WORD  ResIndex = 0;
	WORD numOfItems = pCapStruct->numOfItems;

	for(int i =0;i < numOfItems;i++)
	{
		if((pCapStruct->rtvCapItem[i]).capabilityID != 0)
			Resolution = ((pCapStruct->rtvCapItem[i]).widthVF) * ((pCapStruct->rtvCapItem[i]).heightVF);

		if(Resolution >= MaxResolution )
		{
			MaxResolution = Resolution;
			ResIndex = i;
			eRes = kSuccess;
		}
	}

	if(eRes == kSuccess)
	{
		(pCapStruct->rtvCapItem[ResIndex]).maxBitrateInBps = rate ;
	}

	return eRes;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CRtvVideoCap::Intersection(const CBaseCap& other, BYTE** ppIntersectData, BYTE comparePacketizationMode) const
{
	PTRACE(eLevelInfoNormal,"CRtvVideoCap::Intersection");
	//We need to do a Intersection and we want all the resolutions that initialize to take with MPi that
	//fit to both sides
	//We will start from the bigger resolution and save it's MPI and compare, we will take the maximum MPI.

	BYTE bIsSuccess = FALSE;
	const CRtvVideoCap *pOtherCap = (const CRtvVideoCap *)(&other);

	DWORD thisResolution  = 0;
	DWORD otherResolution  = 0;
	WORD ThisFR = 0;
	WORD OtherFR = 0;

	rtvCapStruct *pThisCapStruct	 = CAP_CAST(rtvCapStruct);
	rtvCapStruct *pOtherCapStruct	 = (rtvCapStruct *)other.GetStruct();
	rtvCapStruct *pIntersectStuct	 = (rtvCapStruct *)*ppIntersectData;

	if (pOtherCap && pThisCapStruct && pOtherCapStruct)
	{
		rtvCapItemS thisRtvMaxCapItem;
		EResult ethisRes = FindMaxCapInCapSet(pThisCapStruct,thisRtvMaxCapItem);

		rtvCapItemS otherRtvMaxCapItem;
		EResult eotherRes =FindMaxCapInCapSet(pOtherCapStruct,otherRtvMaxCapItem);


		//PTRACE2INT(eLevelInfoNormal, "CRtvVideoCap::Intersection start, ThisFR=",thisRtvMaxCapItem.fps);
		//PTRACE2INT(eLevelInfoNormal, "CRtvVideoCap::Intersection start, OtherFR=",otherRtvMaxCapItem.fps);
		TRACEINTO << "start, ThisFR:" << thisRtvMaxCapItem.fps << ", OtherFR:" << otherRtvMaxCapItem.fps;

		if (thisRtvMaxCapItem.fps == 13 || otherRtvMaxCapItem.fps == 13)
		{
			DWORD max_allowed_rtv_hd_fps = GetSystemCfgFlagInt<DWORD>(CFG_KEY_MAX_ALLOWED_RTV_HD_FRAME_RATE);
			if (max_allowed_rtv_hd_fps  < 13)
			{
				PTRACE(eLevelInfoNormal, "CRtvVideoCap::Intersection setting FPS to 30 !!! ");
				thisRtvMaxCapItem.fps = otherRtvMaxCapItem.fps = 30;
			}
		}

		if(ethisRes && eotherRes)
		{
			thisResolution = (thisRtvMaxCapItem.widthVF) * (thisRtvMaxCapItem.heightVF);

			otherResolution = (otherRtvMaxCapItem.widthVF) * (otherRtvMaxCapItem.heightVF);

			if (thisResolution == otherResolution)
			{
				(pIntersectStuct->rtvCapItem[0]).widthVF = thisRtvMaxCapItem.widthVF;
				(pIntersectStuct->rtvCapItem[0]).heightVF= thisRtvMaxCapItem.heightVF;

				bIsSuccess = TRUE;

				ThisFR = thisRtvMaxCapItem.fps;
				OtherFR = otherRtvMaxCapItem.fps;

				if(ThisFR == OtherFR)
					(pIntersectStuct->rtvCapItem[0]).fps = ThisFR;
				else
					(pIntersectStuct->rtvCapItem[0]).fps = min(ThisFR, OtherFR);

			}
			else
			{
				if(thisResolution > otherResolution)
				{
					(pIntersectStuct->rtvCapItem[0]).widthVF = otherRtvMaxCapItem.widthVF;
					(pIntersectStuct->rtvCapItem[0]).heightVF = otherRtvMaxCapItem.heightVF;
					(pIntersectStuct->rtvCapItem[0]).fps = otherRtvMaxCapItem.fps;

					bIsSuccess = TRUE;

				}
				else
				{
					(pIntersectStuct->rtvCapItem[0]).widthVF = thisRtvMaxCapItem.widthVF;
					(pIntersectStuct->rtvCapItem[0]).heightVF = thisRtvMaxCapItem.heightVF;

					bIsSuccess = TRUE;

					if(thisRtvMaxCapItem.fps > 30) //In case we want to open 60fps and RTV is limites to 30fps
						(pIntersectStuct->rtvCapItem[0]).fps = 30;
					else
						(pIntersectStuct->rtvCapItem[0]).fps = thisRtvMaxCapItem.fps;
				}
			}
		}
	}

	if(bIsSuccess)
	{
		pIntersectStuct->numOfItems = 1;
		(pIntersectStuct->rtvCapItem[0]).capabilityID = 1;
		(pIntersectStuct->rtvCapItem[0]).maxBitrateInBps = 0;
	}



	return bIsSuccess;
}
///////////////////////////////////////////////////////////////////////////////
BYTE CRtvVideoCap::IsCapableOfHD720(ERoleLabel eRole) const
{
	BYTE bRes  = FALSE;
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);

	DWORD thisFs  = 0;
	DWORD thisMbps  = 0;

	if (pCapStruct)
	{
		if(eRole == kRolePresentation)
			bRes = FALSE;
		else
		{
			rtvCapItemS thisRtvMaxCapItem;
			EResult ethisRes = FindMaxCapInCapSet(pCapStruct,thisRtvMaxCapItem);

			if(ethisRes)
			{
				thisFs = ((thisRtvMaxCapItem.widthVF) * (thisRtvMaxCapItem.heightVF))/256;
				thisMbps =  thisFs * thisRtvMaxCapItem.fps;

				WORD hd720MinimumFs   = GetMinimumHd720Fs();
				WORD hd720At50MinimumMbps = GetMinimumHd720At50Mbps();

				if((thisFs >= hd720MinimumFs)&& (thisMbps >= hd720At50MinimumMbps))
					bRes = TRUE;

				else
					bRes =  FALSE;
			}
		}
	}
	return bRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CRtvVideoCap::GetCPVideoPartyType() const
{
	BYTE bRes  = FALSE;
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);

	DWORD thisFs  = 0;
	DWORD thisMbps  = 0;

	eVideoPartyType videoPartyType = eVideo_party_type_dummy;

	if (pCapStruct)
	{
		rtvCapItemS thisRtvMaxCapItem;
		EResult ethisRes = FindMaxCapInCapSet(pCapStruct,thisRtvMaxCapItem);

		if(ethisRes)
		{
			thisFs = ((thisRtvMaxCapItem.widthVF) * (thisRtvMaxCapItem.heightVF))/256;
			thisMbps =  thisFs * thisRtvMaxCapItem.fps;
			DWORD rate = thisRtvMaxCapItem.maxBitrateInBps;
			videoPartyType = GetCPRtvResourceVideoPartyTypeByRate(rate);

			eVideoPartyType videoPartyTypeAccordingToCaps = ::GetCPH264ResourceVideoPartyType((DWORD)thisFs, (DWORD)thisMbps, true);

			videoPartyType = min(videoPartyType, videoPartyTypeAccordingToCaps);

			CSmallString msg, msg2;
			cmCapDirection eDirection = GetDirection();
			msg << "Direction: " << eDirection<< " fs: " << thisFs << " mbps: " << thisMbps << " videoPartyTypeAccordingToCaps: " << eVideoPartyTypeNames[videoPartyTypeAccordingToCaps];
			msg2 << "rate: " << rate << " videoPartyType: " << eVideoPartyTypeNames[videoPartyType];
			PTRACE2(eLevelInfoNormal,"CRtvVideoCap::GetCPVideoPartyType(): ",msg.GetString());
			PTRACE2(eLevelInfoNormal,"CRtvVideoCap::GetCPVideoPartyType(): ",msg2.GetString());
		}
	}
	return videoPartyType;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CRtvVideoCap::GetCPRtvResourceVideoPartyTypeByRate(DWORD rate)
{
	std::string max_rtv_protocol_str;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL res = pSysConfig->GetDataByKey(CFG_MAX_RTV_RESOLUTION, max_rtv_protocol_str);
	FPASSERTSTREAM_AND_RETURN_VALUE(!res, "CSysConfig::GetDataByKey: " << CFG_MAX_RTV_RESOLUTION,eCP_H264_upto_CIF_video_party_type);
	FPTRACE2(eLevelInfoNormal,"CRtvVideoCap::GetCPRtvResourceVideoPartyTypeByRate - MAX_RTV_RESOLUTION flag is: ",(max_rtv_protocol_str.c_str()));

	BOOL bForceVga = ("VGA" == max_rtv_protocol_str);
	BOOL bForceCif = ("CIF" == max_rtv_protocol_str);
	BOOL bForceQCif = ("QCIF" == max_rtv_protocol_str);

	if (rate > RTV_MAX_RATE_VGA && !bForceVga && !bForceCif && !bForceQCif)
	{
		FPTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetCPRtvResourceVideoPartyTypeByRate returning eCP_H264_upto_HD720_30FS_Symmetric_video_party_type on rate: ",rate);
		return eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
	}

	if (rate > RTV_MAX_RATE_CIF && !bForceCif && !bForceQCif)
	{
		FPTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetCPRtvResourceVideoPartyTypeByRate returning eCP_H264_upto_SD30_video_party_type on rate: ",rate);
		return eCP_H264_upto_SD30_video_party_type;
	}

	FPTRACE2INT(eLevelInfoNormal,"CRtvVideoCap::GetCPRtvResourceVideoPartyTypeByRate returning eCP_H264_upto_CIF_video_party_type on rate: ",rate);
	return eCP_H264_upto_CIF_video_party_type;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CRtvVideoCap::IsSupportPLI() const
{
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_PLI);
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CRtvVideoCap::IsSupportFIR() const
{
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_FIR);
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CRtvVideoCap::IsSupportTMMBR() const
{
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_TMMBR);

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CRtvVideoCap::IsSupportNonStandardEncode() const
{
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);
	if (pCapStruct)
		return (pCapStruct->rtcpFeedbackMask & RTCP_MASK_IS_NOT_STANDARD_ENCODE);
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CRtvVideoCap::GetRtcpFeedbackMask() const
{
	APIS32 rtcpFeedbackMask = 0;
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);

	if (pCapStruct)
		rtcpFeedbackMask = pCapStruct->rtcpFeedbackMask;

	return rtcpFeedbackMask;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::SetRtcpFeedbackMask(APIS32 rtcpFbMask)
{
	EResult eRes = kFailure;
	rtvCapStruct *pCapStruct = CAP_CAST(rtvCapStruct);

	if (pCapStruct)
	{
		pCapStruct->rtcpFeedbackMask = rtcpFbMask;
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::CopyBaseQualities(const CBaseVideoCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.GetStruct())
	{
		eRes = kSuccess;
		int sizeBaseWithAnnex = sizeof(rtvCapStruct);
		int sizeWithoutBaseSize = sizeBaseWithAnnex - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.GetStruct()) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, sizeWithoutBaseSize);
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRtvVideoCap::SetBitRate(APIS32 rate) //Need to check if this func is needed!!
{
	EResult eRes = kFailure;
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);

	if (pCapStruct)
	{
		eRes = SetRateInMaxCapInCapSet(pCapStruct,rate);
	}

	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtvVideoCap::GetMbpsAndFsAsDevision(APIS32 &DevMbps,APIS32 &DevFs) const
{
	rtvCapStruct* pCapStruct = CAP_CAST(rtvCapStruct);

	long Fs =0;
	long tmpFs = 0;
	long mbps = 0;


	if (pCapStruct)
	{
		rtvCapItemS thisRtvMaxCapItem;
		EResult ethisRes = FindMaxCapInCapSet(pCapStruct,thisRtvMaxCapItem);

		if(ethisRes)
		{
			Fs = ((thisRtvMaxCapItem.widthVF) * (thisRtvMaxCapItem.heightVF));
			tmpFs = GetMaxFsAsDevision(Fs);
			DevFs = GetMaxFsAsDevision(tmpFs);


			mbps =  tmpFs * thisRtvMaxCapItem.fps;
			DevMbps = GetMaxMbpsAsDevision(mbps);


			CSmallString msg;
			msg << "DevFs: " << DevFs<< " DevMbps: " << DevMbps;
			PTRACE2(eLevelInfoNormal,"CRtvVideoCap::GetMbpsAndFs(): ",msg.GetString());
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType	CRtvVideoCap::GetVideoPartyTypeMBPSandFS(DWORD staticMB) const
{
	eVideoPartyType videoPartyType = eVideo_party_type_none;

	long fs =0;
	long mbps = 0;

	GetMbpsAndFsAsDevision(mbps,fs);

	mbps *= CUSTOM_MAX_MBPS_FACTOR;

	mbps += (staticMB*CUSTOM_MAX_MBPS_FACTOR);

	fs *= CUSTOM_MAX_FS_FACTOR;

	videoPartyType = ::GetCPH264ResourceVideoPartyType((DWORD)fs, (DWORD)mbps,true);
	CSmallString msg;
	cmCapDirection eDirection = GetDirection();
	msg << "Direction: " << eDirection<< " fs: " << fs << " mbps: " << mbps << " videoPartyType: " << eVideoPartyTypeNames[videoPartyType];
	PTRACE2(eLevelInfoNormal,"CRtvVideoCap::GetVideoPartyTypeMBPSandFS(): ",msg.GetString());

	return videoPartyType;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtvVideoCap::CopyCapToOtherCap(CRtvVideoCap *pOtherCap)  // to check if not like CopyQualities
{

	rtvCapStruct *pThisCapStruct	 = CAP_CAST(rtvCapStruct);
	rtvCapStruct *pOtherCapStruct	 = (rtvCapStruct *)pOtherCap->GetStruct();

	pOtherCapStruct->numOfItems = pThisCapStruct->numOfItems;

	for(int i=0;i<NumOfRtvItems;i++)
	{
		(pOtherCapStruct->rtvCapItem[i]).capabilityID = (pThisCapStruct->rtvCapItem[i]).capabilityID;
		(pOtherCapStruct->rtvCapItem[i]).widthVF = (pThisCapStruct->rtvCapItem[i]).widthVF;
		(pOtherCapStruct->rtvCapItem[i]).heightVF = (pThisCapStruct->rtvCapItem[i]).heightVF;
		(pOtherCapStruct->rtvCapItem[i]).fps = (pThisCapStruct->rtvCapItem[i]).fps;
		(pOtherCapStruct->rtvCapItem[i]).maxBitrateInBps = (pThisCapStruct->rtvCapItem[i]).maxBitrateInBps;

	}

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////
//StreamDesc
StreamDesc::StreamDesc()
:m_type(eChannelTypeUndefined)
,m_payloadType(0)
,m_specificSourceSsrc(false)
,m_bitRate(0)
,m_frameRate(0)
,m_height(0)
,m_width(0)
,m_pipeIdSsrc(0)
,m_sourceIdSsrc(0)
,m_priority(0)
,m_isLegal(true)
,m_isAvcToSvcVsw(false)
{
	m_scpNotificationParams.InitDefaults();
}

///////////////////////////////////////////////////
StreamDesc::StreamDesc(const StreamDesc &other)
:m_type(other.m_type)
,m_payloadType(other.m_payloadType)
,m_specificSourceSsrc(other.m_specificSourceSsrc)
,m_bitRate(other.m_bitRate)
,m_frameRate(other.m_frameRate)
,m_height(other.m_height)
,m_width(other.m_width)
,m_pipeIdSsrc(other.m_pipeIdSsrc)
,m_sourceIdSsrc(other.m_sourceIdSsrc)
,m_priority(other.m_priority)
,m_isLegal(other.m_isLegal)
,m_isAvcToSvcVsw(other.m_isAvcToSvcVsw)
,m_scpNotificationParams(other.m_scpNotificationParams)
{
}

///////////////////////////////////////////////////
StreamDesc::~StreamDesc()
{
}

///////////////////////////////////////////////////
void StreamDesc::InitDefaults()
{
	m_type = eChannelTypeUndefined;
	m_payloadType = 0;
	m_specificSourceSsrc = false;
	m_bitRate = 0;
	m_frameRate = 0;
	m_height = 0;
	m_width = 0;
	m_pipeIdSsrc = 0;
	m_sourceIdSsrc = 0;
	m_priority = 0;
	m_isLegal = true;
	m_isAvcToSvcVsw = false;

	m_scpNotificationParams.InitDefaults();
}
/////////////////////////////////////////////////////////////////////////////
void  StreamDesc::Serialize(WORD format,CSegment& seg)
{
	switch (format)
	{
		case SERIALEMBD:
			break;
		case NATIVE:
		{
			seg <<(DWORD)m_type;
			seg <<(DWORD)m_payloadType;
			seg <<(DWORD)m_specificSourceSsrc;
			seg <<(DWORD)m_bitRate;
			seg <<(DWORD)m_frameRate;
			seg <<(DWORD)m_height;
			seg <<(DWORD)m_width;
			seg <<(DWORD)m_pipeIdSsrc;
			seg <<(DWORD)m_sourceIdSsrc;
            seg <<(DWORD)m_isLegal;
            seg <<(DWORD)m_isAvcToSvcVsw;

			m_scpNotificationParams.Serialize(format, seg);
		}
		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  StreamDesc::DeSerialize(WORD format,CSegment& seg)
{
	switch (format)
	{
		case SERIALEMBD:
			break;
		case NATIVE:
		{
			DWORD tmpSpecificSourceSsrc, tmpIsLegal, tmpIsAvcToSvcVsw = 0;

			seg >>(DWORD&)m_type;
			seg >>(DWORD&)m_payloadType;
			seg >>(DWORD&)tmpSpecificSourceSsrc;
			seg >>(DWORD&)m_bitRate;
			seg >>(DWORD&)m_frameRate;
			seg >>(DWORD&)m_height;
			seg >>(DWORD&)m_width;
			seg >>(DWORD&)m_pipeIdSsrc;
			seg >>(DWORD&)m_sourceIdSsrc;
            seg >>(DWORD&)tmpIsLegal;
            seg >>(DWORD&)tmpIsAvcToSvcVsw;

			m_scpNotificationParams.DeSerialize(format, seg);

			m_specificSourceSsrc = (bool)tmpSpecificSourceSsrc;
            m_isLegal = (bool)tmpIsLegal;
            m_isAvcToSvcVsw = (bool)tmpIsAvcToSvcVsw;
		}
		default:
			break;
	}

}
//StreamDesc
///////////////////////////////////////////////////



//  class CSacAudioCap
//=======================

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CSacAudioCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(sirenLPR_Scalable_CapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CSacAudioCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(sirenLPR_Scalable_CapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(sirenLPR_Scalable_CapStruct));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the Default structure values
//---------------------------------------------------------------------------------------------------
EResult CSacAudioCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio,eDirection,eRole);
		pCapStruct->maxValue = FrameBasedFPP;
		pCapStruct->minValue = 0;
		pCapStruct->sirenLPRMask = sirenLPRStereo;
		pCapStruct->mixDepth = SFTMCU_AUDIO_MIX_DEPTH_DEFAULT; //Sac Audio only in Soft MCU
		pCapStruct->sampleRate = 48000;
		pCapStruct->recvStreamsGroup.numberOfStreams = 0;
		pCapStruct->sendStreamsGroup.numberOfStreams = 0;
	}

	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CSacAudioCap::SetStruct(cmCapDirection eDirection,int maxValue,int minValue)
{
	EResult eRes = kFailure;
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio,eDirection);
		pCapStruct->maxValue = FrameBasedFPP;
		pCapStruct->minValue = 0;
		pCapStruct->sirenLPRMask = sirenLPRStereo;
		pCapStruct->mixDepth = SFTMCU_AUDIO_MIX_DEPTH_DEFAULT;					//Sac Audio only in Soft MCU
//		pCapStruct->maxRecvSsrc = 0;									//Sac Audio only in Soft MCU
//		pCapStruct->maxSendSsrc = AUDIO_MAX_SEND_SSRC_DEFAULT;		//Sac Audio only in Soft MCU
		pCapStruct->sampleRate = 48000;
		pCapStruct->recvStreamsGroup.numberOfStreams = 0;
		pCapStruct->sendStreamsGroup.numberOfStreams = 0;
	}

	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CSacAudioCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	CSacAudioCap &hOtherSacAudioCap  = (CSacAudioCap&)otherCap;

	if (GetCapCode() == hOtherSacAudioCap.GetCapCode())
	{
		pCapStruct->maxValue = hOtherSacAudioCap.GetMaxValue();
		pCapStruct->minValue = hOtherSacAudioCap.GetMinValue();
		pCapStruct->sirenLPRMask = hOtherSacAudioCap.GetSirenLPRMask();
		pCapStruct->sampleRate = hOtherSacAudioCap.GetSampleRate();
		pCapStruct->mixDepth = hOtherSacAudioCap.GetMixDepth();
//		pCapStruct->maxRecvSsrc = hOtherSacAudioCap.GetMaxRecvSsrc();
//		pCapStruct->maxSendSsrc = hOtherSacAudioCap.GetMaxSendSsrc();
		if (hOtherSacAudioCap.GetRecvStreamsGroup())
			pCapStruct->recvStreamsGroup = *(hOtherSacAudioCap.GetRecvStreamsGroup());
		if (hOtherSacAudioCap.GetSendStreamsGroup())
			pCapStruct->sendStreamsGroup = *(hOtherSacAudioCap.GetSendStreamsGroup());
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSacAudioCap::SetSampleRate(APIU32 sampleRate)
{
	sirenLPR_Scalable_CapStruct* pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);

	if (pCapStruct)
		pCapStruct->sampleRate = sampleRate;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//void CSacAudioCap::ReplaceSendRecvStreams()
//{
//	TRACEINTO;
//
//	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
//
//	// ===== save originals params
//	APIU32 origMaxSendSsrc = pCapStruct->maxSendSsrc;
//	APIU32 origMaxRecvSsrc = pCapStruct->maxRecvSsrc;
//
//	APIU32 origNumOfSendStreams = pCapStruct->sendStreamsGroup.numberOfStreams;
//	APIU32 origNumOfRecvStreams = pCapStruct->recvStreamsGroup.numberOfStreams;
//
//	STREAM_GROUP_S origSendGroup = pCapStruct->sendStreamsGroup;
//	STREAM_GROUP_S origRecvGroup = pCapStruct->recvStreamsGroup;
//
//	// ===== print before
//	CSuperLargeString str;
//	str << "\nmaxSendSsrc:      " << (int)(pCapStruct->maxSendSsrc)
//		<< "\nmaxRecvSsrc:      " << (int)(pCapStruct->maxRecvSsrc)
//		<< "\nnumOfSendStreams: " << (int)(pCapStruct->sendStreamsGroup.numberOfStreams)
//		<< "\nnumOfRecvStreams: " << (int)(pCapStruct->recvStreamsGroup.numberOfStreams);
//
//		int i=0;
//		for (i=0; i<(pCapStruct->sendStreamsGroup.numberOfStreams); i++)
//		{
//			str << "\nSend stream " << (i+1) << " - SSRC Id: " << pCapStruct->sendStreamsGroup.streams[i].streamSsrcId;
//		}
//		for (i=0; i<(pCapStruct->recvStreamsGroup.numberOfStreams); i++)
//		{
//			str << "\nRecv stream " << (i+1) << " - SSRC Id: " << pCapStruct->recvStreamsGroup.streams[i].streamSsrcId ;
//		}
//	TRACEINTO << "---cascade_slave_audio--- BEFORE replacing:" << str.GetString();
//
//
//	// ===== replace send/recv
//	memset( &(pCapStruct->sendStreamsGroup), 0, sizeof(pCapStruct->sendStreamsGroup) );
//	memset( &(pCapStruct->recvStreamsGroup), 0, sizeof(pCapStruct->recvStreamsGroup) );
//
//	pCapStruct->maxRecvSsrc = origMaxSendSsrc;
//	pCapStruct->maxSendSsrc = origMaxRecvSsrc;
//
//	pCapStruct->sendStreamsGroup.numberOfStreams = origNumOfRecvStreams;
//	pCapStruct->recvStreamsGroup.numberOfStreams = origNumOfSendStreams;
//
//	if (pCapStruct->sendStreamsGroup.numberOfStreams)
//		pCapStruct->sendStreamsGroup = origRecvGroup;
//
//	if (pCapStruct->recvStreamsGroup.numberOfStreams)
//		pCapStruct->recvStreamsGroup = origSendGroup;
//
//
//	// ===== print after
//	CSuperLargeString str1;
//	str1 << "\nmaxSendSsrc:      " << (int)(pCapStruct->maxSendSsrc)
//		 << "\nmaxRecvSsrc:      " << (int)(pCapStruct->maxRecvSsrc)
//		 << "\nnumOfSendStreams: " << (int)(pCapStruct->sendStreamsGroup.numberOfStreams)
//		 << "\nnumOfRecvStreams: " << (int)(pCapStruct->recvStreamsGroup.numberOfStreams);
//
//		for (i=0; i<(pCapStruct->sendStreamsGroup.numberOfStreams); i++)
//		{
//			str1 << "\nSend stream " << (i+1) << " - SSRC Id: " << pCapStruct->sendStreamsGroup.streams[i].streamSsrcId;
//		}
//		for (i=0; i<(pCapStruct->recvStreamsGroup.numberOfStreams); i++)
//		{
//			str1 << "\nRecv stream " << (i+1) << " - SSRC Id: " << pCapStruct->recvStreamsGroup.streams[i].streamSsrcId;
//		}
//	TRACEINTO << "---cascade_slave_audio--- AFTER replacing:" << str1.GetString();
//
//}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Prints the object values into a stream
//---------------------------------------------------------------------------------------------------
void CSacAudioCap::Dump(std::ostream& msg) const
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);

	if (pCapStruct)
	{
		CBaseAudioCap::Dump(msg);
		msg << "Sample Rate             = " <<  pCapStruct->sampleRate << "\n";
		msg << "Mix depth    = " << pCapStruct->mixDepth << "\n";
//		msg << "Max recvSsrc            = " << pCapStruct->maxRecvSsrc << "\n";
//		msg << "Max sendSsrc            = " << pCapStruct->maxSendSsrc << "\n";
		//  if (dumpStreams)
		{
			msg << "Number of receive streams    = " << (int)pCapStruct->recvStreamsGroup.numberOfStreams << "\n";
			for (int i = 0; i < pCapStruct->recvStreamsGroup.numberOfStreams; i++)
			{
				msg << "Stream " << (i+1) << ": SSRC Id: " << pCapStruct->recvStreamsGroup.streams[i].streamSsrcId << "\n";
			}
			msg << "Number of send streams    = " << (int)pCapStruct->sendStreamsGroup.numberOfStreams << "\n";
			for (int i = 0; i < pCapStruct->sendStreamsGroup.numberOfStreams; i++)
			{
				msg << "Stream " << (i+1) << ": SSRC Id: " << pCapStruct->sendStreamsGroup.streams[i].streamSsrcId << "\n";
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// return the codec mask - Mono/Stereo
APIU32 CSacAudioCap::GetSirenLPRMask() const
{
	APIU32 sirenLPRMask = 0;
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
		sirenLPRMask = pCapStruct->sirenLPRMask;

	return sirenLPRMask;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSacAudioCap::SetMixDepth(BYTE aMixDepth)
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
		pCapStruct->mixDepth = aMixDepth;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CSacAudioCap::GetMixDepth() const
{
	APIU32 mixDepth = 0;
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
		mixDepth = pCapStruct->mixDepth;

	return mixDepth;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//void CSacAudioCap::SetMaxRecvSsrc(BYTE aMaxRecvSsrc)
//{
//	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
//	if (pCapStruct)
//		pCapStruct->maxRecvSsrc = aMaxRecvSsrc;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//APIU32 CSacAudioCap::GetMaxRecvSsrc() const
//{
//	APIU32 maxRecvSsrc = 0;
//	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
//	if (pCapStruct)
//		maxRecvSsrc = pCapStruct->maxRecvSsrc;
//
//	return maxRecvSsrc;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//void CSacAudioCap::SetMaxSendSsrc(BYTE aMaxSendSsrc)
//{
//	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
//	if (pCapStruct)
//		pCapStruct->maxSendSsrc = aMaxSendSsrc;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//APIU32 CSacAudioCap::GetMaxSendSsrc() const
//{
//	APIU32 maxSendSsrc = 0;
//	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
//	if (pCapStruct)
//		maxSendSsrc = pCapStruct->maxSendSsrc;
//
//	return maxSendSsrc;
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CSacAudioCap::GetMaxValue() const
{
	APIS32 maxValue = 0;
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
		maxValue = pCapStruct->maxValue;
	return maxValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CSacAudioCap::GetMinValue() const
{
	APIS32 minValue = 0;
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
		minValue = pCapStruct->minValue;
	return minValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CSacAudioCap::GetSampleRate() const
{
	APIU32 sampleRate = 0;
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
		sampleRate = pCapStruct->sampleRate;
	return sampleRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
STREAM_GROUP_S* CSacAudioCap::GetRecvStreamsGroup() const
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
		return &pCapStruct->recvStreamsGroup;
	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
STREAM_GROUP_S* CSacAudioCap::GetSendStreamsGroup() const
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
		return &pCapStruct->sendStreamsGroup;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSacAudioCap::SetRecvStreamsGroup(const STREAM_GROUP_S &rStreamGroup)
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
	{
		pCapStruct->recvStreamsGroup = rStreamGroup;
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSacAudioCap::SetSendStreamsGroup(const STREAM_GROUP_S &rStreamGroup)
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
	{
		pCapStruct->sendStreamsGroup = rStreamGroup;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSacAudioCap::SetRecvSsrcId(APIU32 aSsrcId)
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct) {
		if (pCapStruct->recvStreamsGroup.numberOfStreams != 1)
		{
			DEBUG("For SAC, only one receive stream is supported.");
		}
		if (pCapStruct->recvStreamsGroup.numberOfStreams > 0)
		{
			pCapStruct->recvStreamsGroup.streams[0].streamSsrcId = aSsrcId;
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}
bool CSacAudioCap::GetRecvSsrcId(APIU32* aSsrcId)
{

	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct) {
		if (pCapStruct->recvStreamsGroup.numberOfStreams != 1)
		{
			DEBUG("For SAC, only one receive stream is supported.");
		}
		if (pCapStruct->recvStreamsGroup.numberOfStreams > 0)
		{
			aSsrcId=&(pCapStruct->recvStreamsGroup.streams[0].streamSsrcId);
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSacAudioCap::GetSsrcId(cmCapDirection eDirection, APIU32*& aSsrcId, int* num)
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	int i;
	if (pCapStruct) {

		STREAM_GROUP_S streamsGroup;
		if (eDirection == cmCapReceive)
			streamsGroup = pCapStruct->recvStreamsGroup;
		else
			streamsGroup = pCapStruct->sendStreamsGroup;
		*num=streamsGroup.numberOfStreams;
		aSsrcId=new APIU32[*num];
		for(i=0;i<*num;++i)
		{
			aSsrcId[i]=streamsGroup.streams[i].streamSsrcId;
		}
		return true;
	}

	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSacAudioCap::AddRecvStream(APIU32* aSsrcId,int num, bool isUpdate)
{
	if(isUpdate)
	{
		return SetRecvSsrcId(aSsrcId[0]);
	}
	else
	{
		return AddRecvStream(aSsrcId[0]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSacAudioCap::AddSendStream(APIU32* aSsrcId,int num, bool isUpdate)
{
	if(isUpdate)
	{
		WARN("update send stream is not supported for sac audio");
		return false;
	}
	else
	{
		return AddSendStream(aSsrcId[0]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSacAudioCap::AddRecvStream(APIU32 aSsrcId)
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct) {
		if (pCapStruct->recvStreamsGroup.numberOfStreams != 0)
		{
			DEBUG("For SAC, only one receive stream is supported. One exists, cannot add another one.");
			return false;
		}
		pCapStruct->recvStreamsGroup.numberOfStreams = 1;
		pCapStruct->recvStreamsGroup.streams[0].streamSsrcId = aSsrcId;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSacAudioCap::AddSendStream(APIU32 aSsrcId)
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
	{
		int numStreamsCurrent = pCapStruct->sendStreamsGroup.numberOfStreams;
		if (numStreamsCurrent >= MAX_NUM_STREAMS_PER_SET)
		{
			WARN("max send streams has reached, can't add another stream");
			return false;
		}
		pCapStruct->sendStreamsGroup.streams[numStreamsCurrent].streamSsrcId = aSsrcId;
		pCapStruct->sendStreamsGroup.numberOfStreams++;

	    return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
APIU32 CSacAudioCap::GetRecvSsrcId() const
{
	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct) {
		if (pCapStruct->recvStreamsGroup.numberOfStreams != 1)
		{
			DEBUG("For SAC, only one receive stream is supported.");
		}
		if (pCapStruct->recvStreamsGroup.numberOfStreams > 0)
		{
			return pCapStruct->recvStreamsGroup.streams[0].streamSsrcId;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
bool CSacAudioCap::IsSacAudio(CapEnum capCode)
{
    if (capCode >= eSirenLPR_Scalable_32kCapCode && capCode <= eSirenLPRStereo_Scalable_128kCapCode)
	{
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first value that "other" object has and "this" object is lack of.
//             The first 5 bits hold value/s :
//             HIGHER_MIX_DEPTH - Frame per packet is higher in "other".
//---------------------------------------------------------------------------------------------------
BYTE CSacAudioCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x0000;

	sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	sirenLPR_Scalable_CapStruct *pOtherCapStruct = (sirenLPR_Scalable_CapStruct *)other.GetStruct();

	if (pCapStruct && pOtherCapStruct)
	{
		bRes = CBaseAudioCap::IsContaining(other,valuesToCompare,pDetails);


		if (bRes && (valuesToCompare & kMixDepth))
		{
			if (pCapStruct->mixDepth < pOtherCapStruct->mixDepth)
			{
				*pDetails |= HIGHER_MIX_DEPTH;
				bRes = FALSE;
			}
		}

//		if (bRes && (valuesToCompare & kMaxAudioSsrc))
//		{
//			if ( (pCapStruct->maxRecvSsrc)		&&
//				 (pOtherCapStruct->maxRecvSsrc)	&&
//				 (pCapStruct->maxRecvSsrc < pOtherCapStruct->maxRecvSsrc) )
//			{
//				TRACEINTO << "pCapStruct->maxRecvSsrc: " << pCapStruct->maxRecvSsrc << ", pOtherCapStruct->maxRecvSsrc: " << pOtherCapStruct->maxRecvSsrc;
//
//				*pDetails |= HIGHER_MIX_DEPTH;
//				bRes = FALSE;
//			}
//		}
//
//		if (bRes && (valuesToCompare & kMaxAudioSsrc))
//		{
//			if ( (pCapStruct->maxSendSsrc)		&&
//				 (pOtherCapStruct->maxSendSsrc)	&&
//				 (pCapStruct->maxSendSsrc < pOtherCapStruct->maxSendSsrc) )
//			{
//				TRACEINTO << "pCapStruct->maxSendSsrc: " << pCapStruct->maxSendSsrc << ", pOtherCapStruct->maxSendSsrc: " << pOtherCapStruct->maxSendSsrc;
//
//				*pDetails |= HIGHER_MIX_DEPTH;
//				bRes = FALSE;
//			}
//		}

		/*
		if (bRes && (valuesToCompare & kMaxAudioSsrc))
		{
			if ( (pCapStruct->maxRecvSsrc)		&&
				 (pOtherCapStruct->maxSendSsrc)	&&
				 (pCapStruct->maxRecvSsrc < pOtherCapStruct->maxSendSsrc) )
			{
				TRACEINTO << "pCapStruct->maxRecvSsrc: " << pCapStruct->maxRecvSsrc << ", pOtherCapStruct->maxSendSsrc: " << pOtherCapStruct->maxSendSsrc;

				*pDetails |= HIGHER_MIX_DEPTH;
				bRes = FALSE;
			}
		}

		if (bRes && (valuesToCompare & kMaxAudioSsrc))
		{
			if ( (pCapStruct->maxSendSsrc)		&&
				 (pOtherCapStruct->maxRecvSsrc)	&&
				 (pCapStruct->maxSendSsrc < pOtherCapStruct->maxRecvSsrc) )
			{
				TRACEINTO << "pCapStruct->maxSendSsrc: " << pCapStruct->maxSendSsrc << ", pOtherCapStruct->maxRecvSsrc: " << pOtherCapStruct->maxRecvSsrc;

				*pDetails |= HIGHER_MIX_DEPTH;
				bRes = FALSE;
			}
		}
		*/

	}
	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Description: Alloc a new cap and set it's header from 'this' values.
//              Make the highest common between 'this' and 'otherCap' and set the values in the new cap.
//---------------------------------------------------------------------------------------------------
CBaseCap* CSacAudioCap::GetHighestCommon(const CBaseCap & otherCap) const
{
	CSacAudioCap* pNewCap = (CSacAudioCap *)CBaseAudioCap::GetHighestCommon(otherCap);
	if (pNewCap)
	{

		// set the mix depth
		int thisMixDepth  = GetMixDepth();
		int otherMixDepth = ((const CSacAudioCap&)otherCap).GetMixDepth();
		int mixDepth      = min(thisMixDepth,otherMixDepth);
		pNewCap->SetMixDepth(mixDepth);


//		// set the max recv
//		int thisMaxRecvSsrc  = GetMaxRecvSsrc();
//		int otherMaxRecvSsrc = ((const CSacAudioCap&)otherCap).GetMaxRecvSsrc();
//		int maxRecvSsrc      = min(thisMaxRecvSsrc, otherMaxRecvSsrc);
//		pNewCap->SetMaxRecvSsrc(maxRecvSsrc);
//		TRACEINTO << "thisMaxRecvSsrc: " << thisMaxRecvSsrc << ", otherMaxRecvSsrc: " << otherMaxRecvSsrc
//				  << ", maxRecvSsrc: " << maxRecvSsrc << ", pNewCap->GetMaxRecvSsrc: " << (int)pNewCap->GetMaxRecvSsrc();
//
//		// set the max send
//		int thisMaxSendSsrc  = GetMaxSendSsrc();
//		int otherMaxSendSsrc = ((const CSacAudioCap&)otherCap).GetMaxSendSsrc();
//		int maxSendSsrc      = min(thisMaxSendSsrc, otherMaxSendSsrc);
//		pNewCap->SetMaxSendSsrc(maxSendSsrc);
//		TRACEINTO << "thisMaxSendSsrc: " << thisMaxSendSsrc << ", otherMaxSendSsrc: " << otherMaxSendSsrc
//				  << ", maxSendSsrc: " << maxSendSsrc << ", pNewCap->GetMaxSendSsrc: " << (int)pNewCap->GetMaxSendSsrc();

		// set sample rate
		int thisSampleRate  = GetSampleRate();
		int otherSampleRate = ((const CSacAudioCap&)otherCap).GetSampleRate();
		if (thisSampleRate == otherSampleRate)
			pNewCap->SetSampleRate(thisSampleRate);
		TRACEINTO << "thisSampleRate: " << thisSampleRate << ", otherSampleRate: " << otherSampleRate
				  << ", pNewCap->GetSampleRate: " << (int)pNewCap->GetSampleRate();

		// stream intersection is done separately in SCM
	}

	return pNewCap;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CSvcVideoCap
//=======================


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CSvcVideoCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(svcCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
//---------------------------------------------------------------------------------------------------
size_t CSvcVideoCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(svcCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CSvcVideoCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(svcCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(svcCapStruct));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the defaults values
//---------------------------------------------------------------------------------------------------
EResult CSvcVideoCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{

	if (CH264VideoCap::SetDefaults(eDirection, eRole) == kFailure)
	{
		return kFailure;
	}

	EResult eRes = kFailure;
	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);
	if (!pCapStruct)
	{
		return eRes;
	}
	SetLevel(H264_Level_1_2);



	//    memset(&pCapStruct->operationPoints, 0, sizeof(pCapStruct->operationPoints));
	//    memset(&pCapStruct->recvStreamsGroup, 0, sizeof(pCapStruct->recvStreamsGroup));
	//    memset(&pCapStruct->sendStreamsGroup, 0, sizeof(pCapStruct->sendStreamsGroup));
	pCapStruct->recvStreamsGroup.numberOfStreams = 0;
	pCapStruct->sendStreamsGroup.numberOfStreams = 0;
	pCapStruct->scalableLayerId=0;
	pCapStruct->isLegacy=0;
	pCapStruct->operationPoints.numberOfOperationPoints=0;
	eRes = kSuccess;
	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CSvcVideoCap::CopyQualities(const CBaseCap & otherCap)
{

	EResult eRes = kFailure;
	if (CH264VideoCap::CopyQualities(otherCap) == kFailure)
	{
		return eRes;
	}



	svcCapStruct *pCapStruct = CAP_CAST(svcCapStruct);

	CSvcVideoCap &hOtherSvcVideoCap = (CSvcVideoCap&)otherCap;

	if (GetCapCode() == hOtherSvcVideoCap.GetCapCode())
	{
		if(hOtherSvcVideoCap.GetOperationPoints())
			pCapStruct->operationPoints = *(hOtherSvcVideoCap.GetOperationPoints());
		if (hOtherSvcVideoCap.GetRecvStreamsGroup())
			pCapStruct->recvStreamsGroup = *(hOtherSvcVideoCap.GetRecvStreamsGroup());
		if (hOtherSvcVideoCap.GetSendStreamsGroup())
			pCapStruct->sendStreamsGroup = *(hOtherSvcVideoCap.GetSendStreamsGroup());


		eRes = kSuccess;


	}


	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSvcVideoCap::ReplaceSendRecvStreams()
{
	TRACEINTO;

	svcCapStruct *pCapStruct = CAP_CAST(svcCapStruct);

	// ===== save originals params
	APIU32 origNumOfSendStreams = pCapStruct->sendStreamsGroup.numberOfStreams;
	APIU32 origNumOfRecvStreams = pCapStruct->recvStreamsGroup.numberOfStreams;

	STREAM_GROUP_S origSendGroup = pCapStruct->sendStreamsGroup;
	STREAM_GROUP_S origRecvGroup = pCapStruct->recvStreamsGroup;

	// ===== print before
	CSuperLargeString str;
	str << "\nnumOfSendStreams: " << (int)(pCapStruct->sendStreamsGroup.numberOfStreams)
		<< "\nnumOfRecvStreams: " << (int)(pCapStruct->recvStreamsGroup.numberOfStreams);

		int i=0;
		for (i=0; i<(pCapStruct->sendStreamsGroup.numberOfStreams); i++)
		{
			str << "\nSend stream " << (i+1) << " - SSRC Id: " << pCapStruct->sendStreamsGroup.streams[i].streamSsrcId;
		}
		for (i=0; i<(pCapStruct->recvStreamsGroup.numberOfStreams); i++)
		{
			str << "\nRecv stream " << (i+1) << " - SSRC Id: " << pCapStruct->recvStreamsGroup.streams[i].streamSsrcId ;
		}
	TRACEINTO << "---cascade_slave_video--- BEFORE replacing:" << str.GetString();


	// ===== replace send/recv
	memset( &(pCapStruct->sendStreamsGroup), 0, sizeof(pCapStruct->sendStreamsGroup) );
	memset( &(pCapStruct->recvStreamsGroup), 0, sizeof(pCapStruct->recvStreamsGroup) );

	pCapStruct->sendStreamsGroup.numberOfStreams = origNumOfRecvStreams;
	pCapStruct->recvStreamsGroup.numberOfStreams = origNumOfSendStreams;

	if (pCapStruct->sendStreamsGroup.numberOfStreams)
		pCapStruct->sendStreamsGroup = origRecvGroup;

	if (pCapStruct->recvStreamsGroup.numberOfStreams)
		pCapStruct->recvStreamsGroup = origSendGroup;

	// ===== print after
	CSuperLargeString str1;
	str1 << "\nnumOfSendStreams: " << (int)(pCapStruct->sendStreamsGroup.numberOfStreams)
		 << "\nnumOfRecvStreams: " << (int)(pCapStruct->recvStreamsGroup.numberOfStreams);

		for (i=0; i<(pCapStruct->sendStreamsGroup.numberOfStreams); i++)
		{
			str1 << "\nSend stream " << (i+1) << " - SSRC Id: " << pCapStruct->sendStreamsGroup.streams[i].streamSsrcId;
		}
		for (i=0; i<(pCapStruct->recvStreamsGroup.numberOfStreams); i++)
		{
			str1 << "\nRecv stream " << (i+1) << " - SSRC Id: " << pCapStruct->recvStreamsGroup.streams[i].streamSsrcId;
		}
	TRACEINTO << "---cascade_slave_video--- AFTER replacing:" << str1.GetString();

}


/****************************************************************\
 * Function: CopyBaseQualities
 * Description: Copies the base structure EXCEPT from the header and the name
 *              The structures should be allocated with the same size!!!
 * Params: otherCap [in] - the CAPS from which values are copied
 ****************************************************************/
EResult CSvcVideoCap::CopyBaseQualities(const CBaseVideoCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.GetStruct())
	{
		eRes = kSuccess;
		int sizeBaseWithAnnex = sizeof(svcCapStruct);
		int sizeWithoutBaseSize = sizeBaseWithAnnex - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.GetStruct()) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, sizeWithoutBaseSize);
	}
	return eRes;
}

/****************************************************************\
 * Function: Dump
 * Description: Prints the object values into a stream
 * Params: msg [in/out] - the stream to write to
 ****************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSvcVideoCap::Dump(std::ostream& msg) const
{
	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);

	CH264VideoCap::Dump(msg);
	if (pCapStruct)
	{
		int numOfOperationPoints = (int)pCapStruct->operationPoints.numberOfOperationPoints;
		msg << "Number of operation points = " << numOfOperationPoints << "\n";
		int i;
		for (i=0;i<numOfOperationPoints;i++)
		{
			VIDEO_OPERATION_POINT_S op = pCapStruct->operationPoints.tVideoOperationPoints[i];
			msg << "<" << (int)op.layerId << ", " << (int)op.Tid << ", " << (int)op.Did << ", " << (int)op.Qid << ", "
					<< (int)op.Pid << ", " << (int)op.profile << ", " << (int)op.level << ", " << (int)op.frameWidth << ", "
					<< (int)op.frameHeight << ", " << (int)op.frameRate << ", " << (int)op.maxBitRate << ">\n";
		}

		msg << "multi_line - Receive streams:\n";
		DumpStreamGroup(msg, pCapStruct->recvStreamsGroup);

		msg << "multi_line - Send streams:\n";
		DumpStreamGroup(msg, pCapStruct->sendStreamsGroup);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSvcVideoCap::DumpStreamGroup(std::ostream& msg, STREAM_GROUP_S &rStreamGroup) const
{
	int numOfStreams = (int)rStreamGroup.numberOfStreams;
	msg << "Number of streams = " << numOfStreams << "\n";
	int i;
	for (i=0;i<numOfStreams;i++)
	{
		STREAM_S stream = rStreamGroup.streams[i];
		msg << "ssrc="<< (int)stream.streamSsrcId << " width=" << (int)stream.frameWidth
				<< " height=" << (int)stream.frameHeight << " maxFR=" << (int)stream.maxFrameRate << "\n";

	}
}

/****************************************************************\
 * Function: SetOperationPoints
 * Description: Update the OperationPoints with the given value
 * Params: newPipeset [in] - the new pipe set value
 ****************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSvcVideoCap::SetOperationPoints(const VIDEO_OPERATION_POINT_SET_S &operationPoints)
{
	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);
	if (pCapStruct)
		pCapStruct->operationPoints = operationPoints;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::SetRecvStreamsGroup(const STREAM_GROUP_S &rStreamGroup)
{
	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);
	if (pCapStruct)
	{
		pCapStruct->recvStreamsGroup = rStreamGroup;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::SetSendStreamsGroup(const STREAM_GROUP_S &rStreamGroup)
{
	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);
	if (pCapStruct)
	{
		pCapStruct->sendStreamsGroup = rStreamGroup;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSvcVideoCap::SetOperationPoints(const CVideoOperationPointsSet* operationPoints)
{
	PTRACE(eLevelInfoNormal,"CSvcVideoCap::SetOperationPoints");
	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);

	if (pCapStruct)
	{
		std::list <VideoOperationPoint>::const_iterator itr;
		std::list <VideoOperationPoint> vopList = operationPoints->m_videoOperationPoints;
		int i=0;
		APIU8 highest_level = H264_Level_1;
		UINT16 highest_profile = H264_Profile_None;


		pCapStruct->operationPoints.numberOfOperationPoints=operationPoints->m_numberOfOperationPoints;
		pCapStruct->operationPoints.operationPointSetId=operationPoints->m_videoOperationPointsSetId;
		for(itr = vopList.begin();itr!=vopList.end();++itr)
		{
			pCapStruct->operationPoints.tVideoOperationPoints[i].layerId=itr->m_layerId;
			pCapStruct->operationPoints.tVideoOperationPoints[i].Tid=itr->m_tid;
			pCapStruct->operationPoints.tVideoOperationPoints[i].Did=itr->m_did;
			pCapStruct->operationPoints.tVideoOperationPoints[i].Qid=itr->m_qid;
			pCapStruct->operationPoints.tVideoOperationPoints[i].profile=itr->m_videoProfile;
			pCapStruct->operationPoints.tVideoOperationPoints[i].frameRate=itr->m_frameRate;
			pCapStruct->operationPoints.tVideoOperationPoints[i].frameWidth=itr->m_frameWidth;
			pCapStruct->operationPoints.tVideoOperationPoints[i].frameHeight=itr->m_frameHeight;
			pCapStruct->operationPoints.tVideoOperationPoints[i].maxBitRate=itr->m_maxBitRate;
			pCapStruct->operationPoints.tVideoOperationPoints[i].level=ProfileToLevelTranslator::ConvertResolutionAndRateToLevel(itr->m_frameHeight,itr->m_frameWidth,itr->m_frameRate);

			highest_level = std::max(highest_level,pCapStruct->operationPoints.tVideoOperationPoints[i].level);
			highest_profile = ProfileToLevelTranslator::GetHeighestProfile(highest_profile,pCapStruct->operationPoints.tVideoOperationPoints[i].profile);

			i++;
		}
		SetLevel(highest_level);
		SetProfile(highest_profile);
	}
	//    svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);
	//    operationPoints->Dump();
	//    if (pCapStruct)
		//    {
		//       	std::list <VideoOperationPoint>::const_iterator itr;
	//
	//       	std::list <VideoOperationPoint> vopList = operationPoints->GetOperationPointsList();
	//        int i=0;
	//        APIU8 highest_level = H264_Level_1;
	//        UINT16 highest_profile = H264_Profile_None;
	//
	//
	//        pCapStruct->operationPoints.numberOfOperationPoints=operationPoints->GetNumberOfOperationPoints();
	//        pCapStruct->operationPoints.operationPointSetId=operationPoints->GetSetId();
	//
	//       // for(itr = (operationPoints->GetOperationPointsList()).begin();itr!=(operationPoints->GetOperationPointsList()).end();++itr)
	//    	{
	//    		operationPoints->GetOperationPoint(pCapStruct->operationPoints);
	//
	////    		pCapStruct->operationPoints.tVideoOperationPoints[i].layerId=itr->m_layerId;
	////       		pCapStruct->operationPoints.tVideoOperationPoints[i].Tid=itr->m_tid;
	////       		pCapStruct->operationPoints.tVideoOperationPoints[i].Did=itr->m_did;
	////       		pCapStruct->operationPoints.tVideoOperationPoints[i].Qid=itr->m_qid;
	////       		pCapStruct->operationPoints.tVideoOperationPoints[i].profile=itr->m_videoProfile;
	////       		pCapStruct->operationPoints.tVideoOperationPoints[i].frameRate=itr->m_frameRate;
	////       		pCapStruct->operationPoints.tVideoOperationPoints[i].frameWidth=itr->m_frameWidth;
	////       		pCapStruct->operationPoints.tVideoOperationPoints[i].frameHeight=itr->m_frameHeight;
	////       		pCapStruct->operationPoints.tVideoOperationPoints[i].maxBitRate=itr->m_maxBitRate;
	//       	//	pCapStruct->operationPoints.tVideoOperationPoints[i].level=ProfileToLevelTranslator::ConvertResolutionAndRateToLevel(itr->m_frameHeight,itr->m_frameWidth,itr->m_frameRate);
	////    		for (int i=0;i<  pCapStruct->operationPoints.numberOfOperationPoints;i++)
	////    		{
	////    			highest_level = std::max(highest_level,pCapStruct->operationPoints.tVideoOperationPoints[i].level);
	////    			highest_profile = ProfileToLevelTranslator::GetHeighestProfile(highest_profile,pCapStruct->operationPoints.tVideoOperationPoints[i].profile);
	////    		}
	//
	//
	//    	}
	//    	SetLevel(ProfileToLevelTranslator::GetHeighestLevel(operationPoints));
	//    	SetProfile(ProfileToLevelTranslator::GetHeighestProfile(operationPoints));
	//    }

}

/****************************************************************\
 * Function: GetOperationPoints
 * Description: Update the OperationPoints with the given value
 * Params: newPipeset [in] - the new pipe set value
 ****************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////////////
int CSvcVideoCap::GetNumberOfOperationPoints() const
{
	VIDEO_OPERATION_POINT_SET_S* pVideoOperationPointsStruct = GetOperationPoints();
	if (pVideoOperationPointsStruct)
		return pVideoOperationPointsStruct->numberOfOperationPoints;
	else
	{
		ERROR("pVideoOperationPointsStruct is NULL");
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
VIDEO_OPERATION_POINT_SET_S* CSvcVideoCap::GetOperationPoints() const
{
	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);

	if (pCapStruct)
	{
		return &pCapStruct->operationPoints;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void  CSvcVideoCap::SetStreams(const VIDEO_OPERATION_POINT_SET_S &operationPoints)
{

	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);
	int i;
	APIU8       	Did;
	int num;
	if (pCapStruct)
	{
		num=0;
		Did=operationPoints.tVideoOperationPoints[0].Did;

		pCapStruct->recvStreamsGroup.streamGroupId=operationPoints.operationPointSetId;
		for(i=0;i<operationPoints.numberOfOperationPoints;++i)
		{

			//			DEBUG("did:"<<operationPoints.m_aVideoOperationPoints[i-1].Did<<" width:"<<operationPoints.tVideoOperationPoints[i].frameWidth<<" height:"<<operationPoints.tVideoOperationPoints[i].frameHeight<<" rate:"<<operationPoints.m_aVideoOperationPoints[i].frameRate);

			if(operationPoints.tVideoOperationPoints[i].Did!=Did)
			{
				if (num < MAX_NUM_STREAMS_PER_SET)
				{
					pCapStruct->recvStreamsGroup.streams[num].frameWidth=operationPoints.tVideoOperationPoints[i-1].frameWidth;
					pCapStruct->recvStreamsGroup.streams[num].frameHeight=operationPoints.tVideoOperationPoints[i-1].frameHeight;
					pCapStruct->recvStreamsGroup.streams[num].maxFrameRate=operationPoints.tVideoOperationPoints[i-1].frameRate;
					num++;

					Did=operationPoints.tVideoOperationPoints[i].Did;
				}
				else
				{
					ERROR2INT("", num);
				}
			}
		}

		if (num < MAX_NUM_STREAMS_PER_SET)
		{
			pCapStruct->recvStreamsGroup.streams[num].frameWidth=operationPoints.tVideoOperationPoints[i-1].frameWidth;
			pCapStruct->recvStreamsGroup.streams[num].frameHeight=operationPoints.tVideoOperationPoints[i-1].frameHeight;
			pCapStruct->recvStreamsGroup.streams[num].maxFrameRate=operationPoints.tVideoOperationPoints[i-1].frameRate;
			num++;
		}
		else
		{
			ERROR2INT("", num);
		}
		pCapStruct->recvStreamsGroup.numberOfStreams=num;
	}


}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compares the given combination of values to a given object ("other") and tells if
//             the current object ("this") contains the given one i.e this >= other.
//             If the current object does not contain the given one the details parameter will hold
//             the first value that "other" object has and "this" object is lack of.
//             The first 5 bits hold value/s :
//             HIGHER_MIX_DEPTH - Frame per packet is higher in "other".
//---------------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSvcVideoCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x0000;
	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);
	svcCapStruct* pOtherCapStruct = (svcCapStruct *)other.GetStruct();

	if (pCapStruct && pOtherCapStruct)
	{
		bRes = /*CH264VideoCap*/CBaseCap::IsContaining(other,valuesToCompare,pDetails);

		//		if (bRes && (valuesToCompare & kStreams))
		//		{
		//			if ((!IsContainStreams(pCapStruct->recvStreamsGroup, pOtherCapStruct->recvStreamsGroup))
		//				|| (!IsContainStreams(pCapStruct->sendStreamsGroup, pOtherCapStruct->sendStreamsGroup)))
		//			{
		//				DEBUG("Streams are different");
		//				bRes = FALSE;
		//			}
		//		}
	}
	else
	{
		ERROR("pCapStruct or pOtherCapStruct is NULL");
		bRes = FALSE;
	}
	NO_TRACE2INT("", (int)bRes);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSvcVideoCap::Intersection(const CBaseCap& other, BYTE** ppIntersectData, BYTE comparePacketizationMode) const
{
	PTRACE(eLevelInfoNormal,"CSvcVideoCap::Intersection");
	BYTE bIsSuccess;

	bIsSuccess = CH264VideoCap::Intersection(other, ppIntersectData, comparePacketizationMode); // intersection for H264 params section

	if (!bIsSuccess)
	{
		ERROR("no intersection for H264 params");
	// todo - add it:return FALSE;
	}

	const CSvcVideoCap *pOtherCap = (const CSvcVideoCap *)(&other);
	// TODO - Currently we assume this function is called from remote caps, so pOtherCap is the Local caps.
	//		  The right way to do it is to check which of the caps has the operation points.
	if (pOtherCap->GetNumberOfOperationPoints() < 1)
	{
		ERROR("No operation points in local Caps");
	//	todo - add it: return FALSE;
	}

	//svcCapStruct *pThisCapStruct	 = CAP_CAST(svcCapStruct);
	svcCapStruct *pOtherCapStruct	 = (svcCapStruct *)other.GetStruct();

	svcCapStruct *pIntersectStuct	 = (svcCapStruct *)*ppIntersectData;
	CSvcVideoCap *pIntersectSvcCap = (CSvcVideoCap*)CBaseCap::AllocNewCap(eSvcCapCode, pIntersectStuct);
	if (pIntersectSvcCap != NULL)
	{
		pIntersectSvcCap->SetOperationPoints(pOtherCapStruct->operationPoints);

	/* streams intersection is done separately in SCM    */
	///////////////////////////////////////////////////////

//		//pIntersectSvcCap->SetRecvStreamsGroup(IntersectReceiveStreams(*pOtherCapStruct, pThisCapStruct->recvStreamsGroup));
//		 pIntersectSvcCap->IntersectReceiveStreams(*pOtherCapStruct, pThisCapStruct->recvStreamsGroup);
//		//pIntersectSvcCap->SetSendStreamsGroup(IntersectSendStreams(pOtherCapStruct->operationPoints, pThisCapStruct->sendStreamsGroup));
//		pIntersectSvcCap->IntersectSendStreams(*pOtherCapStruct, pThisCapStruct->sendStreamsGroup);

		POBJDELETE(pIntersectSvcCap);
	}
	else
	{
		ERROR("pIntersectSvcCap is NULL");
	}
	return bIsSuccess;

}
////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSvcVideoCap::IntersectH264Profile(h264CapStruct *pThisCapStruct, h264CapStruct *pOtherCapStruct, CH264VideoCap *pIntersectH264Cap) const
{
	BYTE bRet = FALSE;

	if (true/*pThisCapStruct->profileValue == pOtherCapStruct->profileValue*/)
	{
		/*svc temp. should compare profile with profiles in operation point list*/
		pIntersectH264Cap->SetProfile(pThisCapStruct->profileValue);
		bRet = TRUE;
	}
	else
	{
		CSmallString str;
		str << "Profiles different. This profile=" << pThisCapStruct->profileValue << " other profile=" << pOtherCapStruct->profileValue;
		PTRACE2(eLevelInfoNormal,"CSvcVideoCap::Intersection : ",str.GetString());
	}

	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// This function find intersection for MRM's receive streams
void CSvcVideoCap::IntersectReceiveStreams(svcCapStruct &rLocalCapStruct, STREAM_GROUP_S &rRemoteReceiveStreams) const
{
	STREAM_GROUP_S localReceiveStreams = rLocalCapStruct.recvStreamsGroup;
	unsigned int numOfRemoteStreams = rRemoteReceiveStreams.numberOfStreams;
	DEBUG2INT("numOfRemoteStreams: ", numOfRemoteStreams);
	unsigned int numOfLocalStreams = localReceiveStreams.numberOfStreams;
	DEBUG2INT("numOfLocalStreams: ", numOfLocalStreams);
	unsigned int i,j;
	for (i=0;i<numOfRemoteStreams;i++)
	{
		STREAM_S rmtStream = rRemoteReceiveStreams.streams[i];
		NO_TRACE2INT("SSRC: ", rmtStream.streamSsrcId);
		for (j=0;j<numOfLocalStreams;j++)
		{
			NO_TRACE2INT("j: ", j);
			unsigned int width=0, hieght=0, frameRate=0;
			NO_TRACE2INT("localReceiveStreams.streams[j].streamSsrcId: ", localReceiveStreams.streams[j].streamSsrcId);
			if (rmtStream.streamSsrcId == localReceiveStreams.streams[j].streamSsrcId)
			{
				if (rRemoteReceiveStreams.streams[i].frameWidth > localReceiveStreams.streams[j].frameWidth)
					ERROR("high width value in remote cap");
				if (rRemoteReceiveStreams.streams[i].frameHeight > localReceiveStreams.streams[j].frameHeight)
					ERROR("high height value in remote cap");
				// todo - add check of height and width are containing in local receive stream values. (in IsContaing function).

				width = localReceiveStreams.streams[j].frameWidth;
				hieght = localReceiveStreams.streams[j].frameHeight;

				if (rmtStream.maxFrameRate == 0)
					frameRate = localReceiveStreams.streams[j].maxFrameRate;
				else
					frameRate = rmtStream.maxFrameRate;// * 256;

				AddRecvStreamAccordingToOperationPoints(rLocalCapStruct.operationPoints, rmtStream.streamSsrcId, width, hieght, frameRate);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This function find intersection for MRM's receive streams
void CSvcVideoCap::IntersectSendStreams(svcCapStruct &rLocalCapStruct, STREAM_GROUP_S &rRemoteSendStreams) const // MRM's send streams
{
	unsigned int numOfRemoteStreams = rRemoteSendStreams.numberOfStreams;
	DEBUG2INT("numOfRemoteStreams: ", numOfRemoteStreams);
	unsigned int i;
	for (i=0;i<numOfRemoteStreams;i++)
	{
		STREAM_S rmtStream = rRemoteSendStreams.streams[i];
		NO_TRACE2INT("SSRC: ", rmtStream.streamSsrcId);

		unsigned int width=0, hieght=0, frameRate=0;

		width = rmtStream.frameWidth;
		hieght = rmtStream.frameHeight;
		frameRate = rmtStream.maxFrameRate;// * 256;

		AddSendStreamAccordingToOperationPoints(rLocalCapStruct.operationPoints, rmtStream.streamSsrcId, width, hieght, frameRate);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::IsStreamsEqual(STREAM_GROUP_S &streams1, STREAM_GROUP_S &streams2) const
{
	if(streams1.numberOfStreams != streams2.numberOfStreams)
		return false;
	for (int i=0;i<streams1.numberOfStreams;i++)
	{
		if ((streams1.streams[i].streamSsrcId != streams2.streams[i].streamSsrcId)
			|| (streams1.streams[i].frameWidth != streams2.streams[i].frameWidth)
			|| (streams1.streams[i].frameHeight != streams2.streams[i].frameHeight)
			|| (streams1.streams[i].maxFrameRate != streams2.streams[i].maxFrameRate))
		{
			return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::IsStreamsEqual(const CBaseCap& other) const
{
	svcCapStruct* pCapStruct = CAP_CAST(svcCapStruct);
	svcCapStruct* pOtherCapStruct = (svcCapStruct *)other.GetStruct();
	if (pCapStruct && pOtherCapStruct)
	{
		if (!IsStreamsEqual(pCapStruct->recvStreamsGroup, pOtherCapStruct->recvStreamsGroup))
		{
			DEBUG("Receive streams not equal");
			return false;
		}
		if (!IsStreamsEqual(pCapStruct->sendStreamsGroup, pOtherCapStruct->sendStreamsGroup))
		{
			DEBUG("Send streams not equal");
			return false;
		}
	}
	else
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::AddRecvStream(unsigned int ssrc, unsigned int frameWidth, unsigned int frameHeight, unsigned int maxFrameRate) const
{
	bool bRet = false;
	svcCapStruct *pCapStruct = CAP_CAST(svcCapStruct);
	if (pCapStruct)
	{
		int numStreamsCurrent = pCapStruct->recvStreamsGroup.numberOfStreams;
		if (numStreamsCurrent >= MAX_NUM_STREAMS_PER_SET)
		{
			ERROR("max receive streams has reached, can't add another stream");
		}
		else
		{
			pCapStruct->recvStreamsGroup.streams[numStreamsCurrent].streamSsrcId = ssrc;
			pCapStruct->recvStreamsGroup.streams[numStreamsCurrent].frameWidth = frameWidth;
			pCapStruct->recvStreamsGroup.streams[numStreamsCurrent].frameHeight = frameHeight;
			pCapStruct->recvStreamsGroup.streams[numStreamsCurrent].maxFrameRate = maxFrameRate;
			pCapStruct->recvStreamsGroup.numberOfStreams++;
			bRet = true;
		}
	}
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::AddSendStream(unsigned int ssrc, unsigned int frameWidth, unsigned int frameHeight, unsigned int maxFrameRate) const
{
	bool bRet = false;
	svcCapStruct *pCapStruct = CAP_CAST(svcCapStruct);
	if (pCapStruct)
	{
		int numStreamsCurrent = pCapStruct->sendStreamsGroup.numberOfStreams;
		if (numStreamsCurrent >= MAX_NUM_STREAMS_PER_SET)
		{
			ERROR("max send streams has reached, can't add another stream");
		}
		else
		{
			pCapStruct->sendStreamsGroup.streams[numStreamsCurrent].streamSsrcId = ssrc;
			pCapStruct->sendStreamsGroup.streams[numStreamsCurrent].frameWidth = frameWidth;
			pCapStruct->sendStreamsGroup.streams[numStreamsCurrent].frameHeight = frameHeight;
			pCapStruct->sendStreamsGroup.streams[numStreamsCurrent].maxFrameRate = maxFrameRate;
			pCapStruct->sendStreamsGroup.numberOfStreams++;
			bRet = true;
		}
	}
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::AddRecvStreamAccordingToOperationPoints(VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int ssrc, unsigned int width, unsigned int hieght, unsigned int frameRate) const
{
	bool bRet = false;
	int i;
	for (i=0;i<rOperationPoints.numberOfOperationPoints;i++)
	{
		if ((rOperationPoints.tVideoOperationPoints[i].frameWidth == width)
			&& (rOperationPoints.tVideoOperationPoints[i].frameHeight == hieght)
			&& (rOperationPoints.tVideoOperationPoints[i].frameRate >= frameRate))
		{

			bRet = AddRecvStream(ssrc, width, hieght, rOperationPoints.tVideoOperationPoints[i].frameRate);
			break;
		}
	}
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::AddSendStreamAccordingToOperationPoints(VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int ssrc, unsigned int width, unsigned int hieght, unsigned int frameRate) const
{
	bool bRet = false;
	int i;
	for (i=rOperationPoints.numberOfOperationPoints-1;i>=0;i--)
	{
		if ((rOperationPoints.tVideoOperationPoints[i].frameWidth <= width)
			&& (rOperationPoints.tVideoOperationPoints[i].frameHeight <= hieght)
			&& ((rOperationPoints.tVideoOperationPoints[i].frameRate <= frameRate) || (frameRate == 0)))
		{

			bRet = AddSendStream(ssrc, width, hieght, rOperationPoints.tVideoOperationPoints[i].frameRate);
			break;
		}
	}
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::AddRecvStream(APIU32* aSsrcId,int num, bool isUpdate)
{
	svcCapStruct *pCapStruct = CAP_CAST(svcCapStruct);
	int i;

	if (pCapStruct)
	{
		int current = (isUpdate)? 0 : pCapStruct->recvStreamsGroup.numberOfStreams;
		if(num > MAX_RECV_STREAMS_FOR_VIDEO - current)
		{
			num = MAX_RECV_STREAMS_FOR_VIDEO - current;
			DEBUG2INT("For video, only - receive streams are supported ", MAX_RECV_STREAMS_FOR_VIDEO);

		}

		for(i=0;i<num;++i)
		{
			pCapStruct->recvStreamsGroup.streams[i+current].streamSsrcId = aSsrcId[i];

		}
		
		pCapStruct->recvStreamsGroup.numberOfStreams=current+i;

		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
STREAM_GROUP_S* CSvcVideoCap::GetRecvStreamsGroup() const
{
	svcCapStruct *pCapStruct = CAP_CAST(svcCapStruct);
	if (pCapStruct)
		return  &pCapStruct->recvStreamsGroup;
	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
STREAM_GROUP_S* CSvcVideoCap::GetSendStreamsGroup() const
{
	svcCapStruct *pCapStruct = CAP_CAST(svcCapStruct);
	if (pCapStruct)
		return  &pCapStruct->sendStreamsGroup;
	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::GetSendSsrcId(APIU32*& aSsrcId,int* num)
{
	svcCapStruct *pCapStruct = CAP_CAST(svcCapStruct);
	int i;

	//   	DEBUG("inside CSvcVideoCap::GetTransSsrcId");

	//    sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct)
	{
		*num=pCapStruct->sendStreamsGroup.numberOfStreams;
		aSsrcId=new APIU32[*num];
		for(i=0;i<*num;++i)
		{
			aSsrcId[i]=pCapStruct->sendStreamsGroup.streams[i].streamSsrcId;
		}
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSvcVideoCap::GetRecvSsrcId(APIU32*& aSsrcId,int* num)
{
	svcCapStruct *pCapStruct = CAP_CAST(svcCapStruct);
	int i;

	//   	DEBUG("inside CSvcVideoCap::GetRecvSsrcId");

	//    sirenLPR_Scalable_CapStruct *pCapStruct = CAP_CAST(sirenLPR_Scalable_CapStruct);
	if (pCapStruct) {
		*num=pCapStruct->recvStreamsGroup.numberOfStreams;
		aSsrcId=new APIU32[*num];
		for(i=0;i<*num;++i)
		{
			aSsrcId[i]=pCapStruct->recvStreamsGroup.streams[i].streamSsrcId;
		}
		return true;
	}

	return false;
}



// hard coded values for levels - not in use in MRM-V1
static MbpsFsPair LevelConvertEx[]=
{
	{1485,99,H264_Level_1,"1"},
	{3000,396,H264_Level_1_1,"1.1"},
	{6000,396,H264_Level_1_2,"1.2"},
	{11880,396,H264_Level_1_3,"1.3"},
	{11880,396,H264_Level_2,"2"},
	{19800,792,H264_Level_2_1,"2.1"},
	{20250,1620,H264_Level_2_2,"2.2"},
	{40500,1620,H264_Level_3,"3"},
	{108000,3600,H264_Level_3_1,"3.1"},
	{216000,5120,H264_Level_3_2,"3.2"},
	{245760,8192,H264_Level_4,"4"},
	{245760,8192,H264_Level_4_1,"4.1"},
	{491520,8192,H264_Level_4_2,"4.2"},
	{589824,22080,H264_Level_5,"5"},
	{983040,36864,H264_Level_5_1,"5.1"}
};

// hard coded values for resolutions/frame rate/levels - not in use in MRM-V1
//static WidthHeightRateTriad LevelConvert[]=
//{
//{1080,1920,1920,85},
//{1080,1920,3840,85},
//{1080,1920,7680,85},
//{1080,1920,15360,99},
//
//{720,1280,1920,71},
//{720,1280,3840,71},
//{720,1280,7680,71},
//{720,1280,15360,78},
//
//{540,960,1920,71},
//{540,960,3840,71},
//{540,960,7680,71},
//{540,960,15360,78},
//
//{480,848,1920,57},
//{480,848,3840,64},
//{480,848,7680,71},
//{480,848,15360,71},
//
//{360,640,1920,57},
//{360,640,3840,57},
//{360,640,7680,64},
//{360,640,15360,71},
//
//{270,480,1920,50},
//{270,480,3840,50},
//{270,480,7680,50},
//{270,480,15360,64},
//
//{240,424,1920,50},
//{240,424,3840,50},
//{240,424,7680,50},
//{240,424,15360,64},
//
//{180,320,1920,22},
//{180,320,3840,29},
//{180,320,7680,36},
//{180,320,15360,50},
//
//{90,160,1920,15},
//{90,160,3840,15},
//{90,160,7680,22},
//{90,160,15360,29}
//};

////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int ProfileToLevelTranslator::GetHeighestLevel(const CVideoOperationPointsSet* operationPoints)
{
	std::list <VideoOperationPoint>::const_iterator itr;
	//std::list <VideoOperationPoint> vopList = operationPoints->m_videoOperationPoints;

	unsigned int maxLevel = H264_Level_1;
	for(itr = operationPoints->GetOperationPointsList()->begin();itr!=operationPoints->GetOperationPointsList()->end();++itr)
	{
		unsigned int tmpLevel = ConvertResolutionAndRateToLevel(itr->m_frameHeight,itr->m_frameWidth,itr->m_frameRate);
		if(tmpLevel > maxLevel)
		{
			maxLevel = tmpLevel;
		}
	}
	return maxLevel;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT16 ProfileToLevelTranslator::GetHeighestProfile(const CVideoOperationPointsSet* operationPoints)
{
	std::list <VideoOperationPoint>::const_iterator itr;

	UINT16 maxProfile = H264_Profile_None;
	for(itr = operationPoints->GetOperationPointsList()->begin();itr!=operationPoints->GetOperationPointsList()->end();++itr)
	{
		maxProfile = GetHeighestProfile(itr->m_videoProfile,maxProfile);
	}
	return maxProfile;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int ProfileToLevelTranslator::GetHeighestBitRate(const CVideoOperationPointsSet* operationPoints)
{
	std::list <VideoOperationPoint>::const_iterator itr;

	unsigned int maxBitRate = 0;
	for(itr = operationPoints->GetOperationPointsList()->begin();itr!=operationPoints->GetOperationPointsList()->end();++itr)
	{
		if(itr->m_maxBitRate > maxBitRate)
		{
			maxBitRate=itr->m_maxBitRate;
		}
	}
	return maxBitRate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
UINT16 ProfileToLevelTranslator::GetHeighestProfile(UINT16 firstProfile, UINT16 secondProfile)
{
	if (firstProfile > H264_Profile_BaseLine)
	{
		if (secondProfile > H264_Profile_BaseLine)
		{ // both profile are SVC profiles - the smallest is the highest
			return ((firstProfile > secondProfile)? secondProfile : firstProfile);
		}
		else
		{ // first profile is SVC - other is H264
			return firstProfile;
		}
	}
	else //first profile is H264
	{
		if (secondProfile > H264_Profile_BaseLine)
		{	// first profile is H264 - other is SVC
			return secondProfile;
		}
		else
		{ // both profile are H264 profiles - the smallest is the highest( if not H264_Profile_None(0))
			if (firstProfile > secondProfile)
			{	//make sure the lower proflie is not H264_Profile_None(0)
				return ((secondProfile)? secondProfile : firstProfile);
			}
			else
			{   //make sure the lower proflie is not H264_Profile_None(0)
				return ((firstProfile)? firstProfile : secondProfile);
			}
		}

	}
}

unsigned int ProfileToLevelTranslator::ConvertResolutionAndRateToLevel(int frame_height, int frame_width, int frame_rate)
{

	int numLevels = sizeof(LevelConvertEx)/sizeof(MbpsFsPair);
	// Calculate Mbps and FS (H.264 params) from operation point resolution and frame rate
	int FS = (frame_width) * (frame_height) / 256;
	int MBPS=(FS * frame_rate) / 256;

	bool flag = true;
	unsigned int highestLevelIndex = 0;

	// The level value is the smallest level were the level's MaxFS>= the resolutions frame size
	// and the level's MaxMBPS >= resolution size*frame rate.
	for(int i = highestLevelIndex ;flag==true && i < numLevels;i++)
	{
		if(LevelConvertEx[i].FS>=FS)
		{
			flag=false;
			highestLevelIndex = i;
		}

	}
	if(flag)
	{
		ERROR_F2INT("could not find level with FS >= ", FS);
	}
	flag=true;
	for(int j = highestLevelIndex;flag==true && j < numLevels ;j++)
	{
		if(LevelConvertEx[j].MBPS>=MBPS)
		{
			highestLevelIndex = j;
			//printf("LEVEL: %s MBPS %d FS %d\n",LevelConvertEx[j].strLevel,MBPS,FS);
			flag=false;
		}
	}
	if(flag)
	{
		ERROR_F2INT("could not find level with MBPS >= ", MBPS);
	}

	return LevelConvertEx[highestLevelIndex].level;

}

unsigned int ProfileToLevelTranslator::ConvertResolutionAndRateToLevelEx(long FS, long MBPS)
{
/*
 remark - in case FS==0 , the lowest operation point will be chosen!!!
 */
	int numLevels = sizeof(LevelConvertEx)/sizeof(MbpsFsPair);
	// Calculate Mbps and FS (H.264 params) from operation point resolution and frame rate
//	int FS = (frame_width) * (frame_height) / 256;
//	int MBPS=(FS * frame_rate) / 256;
	FS *= CUSTOM_MAX_FS_FACTOR;
	MBPS *= CUSTOM_MAX_MBPS_FACTOR;


	bool flag = true;
	unsigned int highestLevelIndex=0;
	unsigned int highestLevelIndexFs = numLevels-1;
	unsigned int highestLevelIndexMbps=numLevels-1;
	CSmallString str;
	// The level value is the smallest level were the level's MaxFS>= the resolutions frame size
	// and the level's MaxMBPS >= resolution size*frame rate.
	for(int i = 0 ;flag==true && i < numLevels;i++)
	{


//		str<<"@@@! LevelConvertEx[i].FS"<<LevelConvertEx[i].FS<<"FS: "<<FS;
		//PTRACE2(eLogLevelDEBUG,": ",str.GetString());
//        PTRACE (eLogLevelERROR, str.GetString());
		str.Clear();

		if(LevelConvertEx[i].FS>FS)
		{

			//TRACEINTOFUNC<<"@@@! chosen LevelConvertEx[i].FS"<<LevelConvertEx[i].FS<<"FS: "<<FS;

			flag=false;
//	        PTRACE (eLogLevelERROR, str.GetString());
			if(i>0)
			{
				highestLevelIndexFs = i-1;
			}
			else
			{
				highestLevelIndexFs = i;
				TRACEINTOFUNC<<"avc_vsw_relay: problem: FS lower then the lowest level!!!";
			}
//			str<<"@@@! chosen LevelConvertEx[i].FS"<<LevelConvertEx[i].FS<<"FS: "<<FS<<" highestLevelIndexFs: "<<highestLevelIndexFs;
//	        PTRACE (eLogLevelERROR, str.GetString());
			str.Clear();

		}

	}
	if(flag)
	{
		ERROR_F2INT("avc_vsw_relay: could not find level with FS >= ", FS);
	}
	flag=true;
	for(int j = 0;flag==true && j < numLevels ;j++)
	{

//		str<<"@@@! LevelConvertEx[j].MBPS"<<LevelConvertEx[j].MBPS<<"MBPS: "<<MBPS;
//       PTRACE (eLogLevelERROR, str.GetString());
		str.Clear();

		if(LevelConvertEx[j].MBPS>MBPS)
		{
			if(j>0)
			{
				highestLevelIndexMbps = j-1;
			}
			else
			{
				highestLevelIndexMbps = j;
				TRACEINTOFUNC<<"avc_vsw_relay: problem: MBPS lower then the lowest level!!!";

			}
//			str<<"@@@!chosen  LevelConvertEx[j].MBPS"<<LevelConvertEx[j].MBPS<<"MBPS: "<<MBPS<<"highestLevelIndexMbps: " <<highestLevelIndexMbps;
//	        PTRACE (eLogLevelERROR, str.GetString());
			str.Clear();

			//printf("LEVEL: %s MBPS %d FS %d\n",LevelConvertEx[j].strLevel,MBPS,FS);
			flag=false;
		}
	}

	if(flag)
	{
		ERROR_F2INT("avc_vsw_relay: could not find level with MBPS >= ", MBPS);
	}

	if(highestLevelIndexMbps<=highestLevelIndexFs)
	{
		highestLevelIndex=highestLevelIndexMbps;
	}
	else
	{
		highestLevelIndex=highestLevelIndexFs;
	}

//	highestLevelIndex=highestLevelIndexFs;

//	TRACEINTOFUNC<<"avc_vsw_relay MBPS discarded";

	TRACEINTOFUNC<<"avc_vsw_relay initialization (from FS and to level): InMBPS:"<<MBPS<<" InFS:"<<FS<<" level:"<<LevelConvertEx[highestLevelIndex].level<<" OutMBPS:"<<LevelConvertEx[highestLevelIndex].MBPS<<" OutFs:"<<LevelConvertEx[highestLevelIndex].FS;
	TRACEINTO<<"avc_vsw_relay highestLevelIndex:"<<highestLevelIndex<<" actual level: "<<LevelConvertEx[highestLevelIndex].strLevel<<" actual FS:"<<LevelConvertEx[highestLevelIndex].FS;
	return LevelConvertEx[highestLevelIndex].level;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD ProfileToLevelTranslator::SvcProfileToH264(const WORD aVideoProfile)
{
    switch (aVideoProfile)
    {
    case SVC_Profile_BaseLine:
        return H264_Profile_BaseLine;
    case SVC_Profile_High:
        return H264_Profile_High;
    default:
        return H264_Profile_BaseLine;
    }

    return H264_Profile_BaseLine;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  class CMsSvcVideoCap 
//=======================
void CMsSvcVideoCap::Dump(std::ostream& msg) const
{
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
	msg << "CMsSvcVideoCap: " << "\n";
	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "\nWidth  = " << pCapStruct->width;
		msg << "\nHeight  = " << pCapStruct->height;
		msg << "\nMaxFrameRate  = " << pCapStruct->maxFrameRate;
		msg << "\nMaxRate  = " << pCapStruct->maxBitRate;
		msg << "\nMinRate  = " << pCapStruct->minBitRate;
		msg << "\nmaxpixelNum  = " << ((int)pCapStruct->maxPixelsNum);
		msg << "\npacketizationMode  = " << ((int)pCapStruct->packetizationMode);
		msg << "\naspectRatio  = " << pCapStruct->aspectRatio;
		if (pCapStruct->rtcpFeedbackMask >= 0)
				msg << "\nrtcpFeedbackMask                = " <<pCapStruct->rtcpFeedbackMask;

		msg << "\n";
	}
}
size_t CMsSvcVideoCap::SizeOf() const
{
	size_t structSize = 0;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		structSize += sizeof(msSvcCapStruct);
	}
	return structSize;
}
size_t CMsSvcVideoCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(msSvcCapStruct);
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CMsSvcVideoCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(msSvcCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(msSvcCapStruct));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CMsSvcVideoCap::CopyQualities(const CBaseCap & otherCap)
{
	EResult eRes = kFailure;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
	msSvcCapStruct *pOtherCapStruct = (msSvcCapStruct*)otherCap.GetStruct();
	CMsSvcVideoCap &hOtherVideoCap  = (CMsSvcVideoCap&)otherCap;

	if (GetCapCode() == hOtherVideoCap.GetCapCode())
	{
		eRes = kSuccess;
		pCapStruct->maxBitRate	= hOtherVideoCap.GetBitRate();
		pCapStruct->minBitRate	= hOtherVideoCap.GetMinBitRate();

		pCapStruct->rtcpFeedbackMask  = hOtherVideoCap.GetRtcpFeedbackMask();
		pCapStruct->aspectRatio = hOtherVideoCap.GetAspectRatio();
		pCapStruct->filler = hOtherVideoCap.GetFiller();
		pCapStruct->height = hOtherVideoCap.GetHeight();
		pCapStruct->width = hOtherVideoCap.GetWidth();
		pCapStruct->packetizationMode = hOtherVideoCap.GetPacketizationMode();
		pCapStruct->maxFrameRate = hOtherVideoCap.GetMaxFrameRate();
		pCapStruct->maxPixelsNum = hOtherVideoCap.GetMaxPixel();


	}
	return eRes;
}
//////////////////////////////////////////////////////////////////////////////////
EResult CMsSvcVideoCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	EResult eRes = kFailure;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapVideo, eDirection, eRole);
		pCapStruct->maxBitRate	= 0;
		pCapStruct->minBitRate	= 0;

		pCapStruct->aspectRatio = -1;
		pCapStruct->filler = 0;
		pCapStruct->height = 0;
		pCapStruct->width = 0;
		pCapStruct->packetizationMode = 1;//?
		pCapStruct->maxFrameRate =0;
		pCapStruct->maxPixelsNum =0;
		pCapStruct->rtcpFeedbackMask                    = (RTCP_MASK_FIR | RTCP_MASK_TMMBR | RTCP_MASK_PLI | RTCP_MASK_IS_NOT_STANDARD_ENCODE | RTCP_MASK_MS_SRC | RTCP_MASK_MS_XPLI);
	}

	return eRes;
}
//////////////////////////////////////////////////////////////////////////////////
BYTE CMsSvcVideoCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD* pDetails) const
{
	BYTE bRes = TRUE;
	PTRACE2INT(eLevelInfoNormal, "CMsSvcVideoCap::IsContaining  ", valuesToCompare);
	*pDetails = 0x00000000;
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);
	msSvcCapStruct* pOtherCapStruct = (msSvcCapStruct *)other.GetStruct();
	//return TRUE; // TEMPORARY DISABLE THIS MODE AVIHAY/Natalia

	if (pCapStruct && pOtherCapStruct)
	{
		PTRACE(eLevelInfoNormal, "CMsSvcVideoCap::IsContaining - inside ");
		// Starts the comparison. If in some stage res gets FALSE the comparison stops
		// and the value that "this" struct was lack of will be saved in the pDetails.
		bRes = CBaseCap::IsContaining(other, valuesToCompare, pDetails);
		if (bRes == FALSE)
		{
			PTRACE(eLevelInfoNormal, "CMsSvcVideoCap::IsContaining - ToleraceRate - msv svc is contating failed on base ");
			return bRes;
		}

		APIS32 toleraceRatePct = GetToleraceRatePct(pOtherCapStruct->maxBitRate);
		TRACEINTO << "ToleraceBitRate:" << toleraceRatePct << ", MaxBitRate:" << pOtherCapStruct->maxBitRate;

		if ((valuesToCompare & kBitRate) && bRes)
		{
			if (pCapStruct->maxBitRate < toleraceRatePct)
			{
				//bRes = FALSE; - tbd-noa
				*pDetails |= HIGHER_BIT_RATE;
			}
		}
		if ((valuesToCompare & kBitRateForCascade) && bRes)
		{
			if (pCapStruct->maxBitRate < (pOtherCapStruct->maxBitRate * CASCADE_BW_FACTOR) / 100)
			{
				//bRes = FALSE;-TBD-NOA
				*pDetails |= HIGHER_BIT_RATE;
			}
		}
		if ((valuesToCompare & kBitRateWithoutTolerance) && bRes)
		{
			if (pCapStruct->maxBitRate < pOtherCapStruct->maxBitRate)
			{
				//bRes = FALSE;TBD-NOA
				*pDetails |= HIGHER_BIT_RATE;
			}
		}
		if ((valuesToCompare & kPacketizationMode) && bRes)
		{
			if (pCapStruct->packetizationMode != pOtherCapStruct->packetizationMode)
			{
				//bRes = FALSE;TBD-NOA
				*pDetails |= DIFF_PACKETIZATION_MODE;
			}

		}
		if ((valuesToCompare & kMsSvcParams) && bRes)
		{
			if (pCapStruct->width != pOtherCapStruct->width || pCapStruct->height != pOtherCapStruct->height || pCapStruct->aspectRatio != pOtherCapStruct->aspectRatio || pCapStruct->maxFrameRate != pOtherCapStruct->maxFrameRate)
			{
				bRes = FALSE;
			}
		}
	}

	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}
///////////////////////////////////////////////////////////////////////////////////
BYTE CMsSvcVideoCap::Intersection(const CBaseCap& other, BYTE** ppIntersectData, BYTE comparePacketizationMode) const
{
	PTRACE(eLevelInfoNormal,"CMsSvcVideoCap::Intersection");
	//We need to do a Intersection and we want all the resolutions that initialize to take with MPi that
	//fit to both sides
	//We will start from the bigger resolution and save it's MPI and compare, we will take the maximum MPI.

	BYTE bIsSuccess = FALSE;
	const CMsSvcVideoCap *pOtherCap = (const CMsSvcVideoCap *)(&other);

	msSvcCapStruct  *pThisCapStruct	 = CAP_CAST(msSvcCapStruct);
	msSvcCapStruct  *pOtherCapStruct	 = (msSvcCapStruct*)other.GetStruct();

	msSvcCapStruct  *pIntersectStuct	 = (msSvcCapStruct*)*ppIntersectData;
	CMsSvcVideoCap *pIntersectMsSvcCap = (CMsSvcVideoCap*)CBaseCap::AllocNewCap(eMsSvcCapCode, pIntersectStuct);
	if(pIntersectMsSvcCap == NULL)
	{
		PTRACE(eLevelInfoNormal, "CMsSvcVideoCap::Intersection -  pIntersectMsSvcCap is NULL");
		return FALSE;
	}
	pIntersectMsSvcCap->SetDefaults();

	if (pOtherCap && pThisCapStruct && pOtherCapStruct)
	{

		MsSvcVideoModeDetails MsSvcDetailsIntersect;
		CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();
		MsSvcVideoModeDetails ThisCapDetailes;
		MsSvcVideoModeDetails OtherCapDetails;

		ThisCapDetailes.aspectRatio = pThisCapStruct->aspectRatio;
		ThisCapDetailes.maxFrameRate = pThisCapStruct->maxFrameRate;
		ThisCapDetailes.maxWidth = pThisCapStruct->width;
		ThisCapDetailes.maxHeight = pThisCapStruct->height;
		ThisCapDetailes.maxBitRate = (pThisCapStruct->maxBitRate*100);

		ThisCapDetailes.maxNumOfPixels = pThisCapStruct->maxPixelsNum;

		OtherCapDetails.aspectRatio = pOtherCapStruct->aspectRatio;
		OtherCapDetails.maxFrameRate = pOtherCapStruct->maxFrameRate;
		OtherCapDetails.maxWidth = pOtherCapStruct->width;
		OtherCapDetails.maxHeight = pOtherCapStruct->height;
		OtherCapDetails.maxBitRate = (pOtherCapStruct->maxBitRate*100);
		OtherCapDetails.maxNumOfPixels = pOtherCapStruct->maxPixelsNum;


		MsSvcVidMode->GetMsSvcModeIntersctionOfTwoCapsForBestMode(MsSvcDetailsIntersect,ThisCapDetailes,OtherCapDetails);
		if(MsSvcDetailsIntersect.videoModeType == eInvalidModeType)
			PTRACE(eLevelError, "CMsSvcVideoCap::Intersection -  pIntersectMsSvcCap is NULL-error in finding ms svc cap mode");
		else
		{
			PTRACE2INT(eLevelInfoNormal, "CMsSvcVideoCap::Intersection -  pIntersectMsSvcCap max bit rate = ",(MsSvcDetailsIntersect.maxBitRate /100));

			bIsSuccess = TRUE;
		//	pIntersectMsSvcCap->SetMaxBitRate((MsSvcDetailsIntersect.maxBitRate /100));
			pIntersectMsSvcCap->SetWidth(MsSvcDetailsIntersect.maxWidth);
			pIntersectMsSvcCap->SetHeight(MsSvcDetailsIntersect.maxHeight);
			pIntersectMsSvcCap->SetAspectRatio(MsSvcDetailsIntersect.aspectRatio);
			pIntersectMsSvcCap->SetMaxFrameRate(MsSvcDetailsIntersect.maxFrameRate);
			pIntersectMsSvcCap->SetMinBitRate(MsSvcDetailsIntersect.minBitRate /100);
			pIntersectMsSvcCap->SetMaxPixelNum(MsSvcDetailsIntersect.maxNumOfPixels);
			pIntersectMsSvcCap->SetRtcpFeedbackMask(GetRtcpFeedbackMask() & pOtherCapStruct->rtcpFeedbackMask );
		}

	}

	POBJDELETE(pIntersectMsSvcCap);
	return bIsSuccess;
}
///////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType	CMsSvcVideoCap::GetVideoPartyTypeMBPSandFS(DWORD staticMB) const
{
	BYTE bRes  = FALSE;
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	DWORD thisFs  = 0;
	DWORD thisMbps  = 0;

	eVideoPartyType videoPartyType = eVideo_party_type_dummy;

	if (pCapStruct)
	{

			thisFs = ((pCapStruct->width) * (pCapStruct->height))/256;
			thisMbps =  thisFs * pCapStruct->maxFrameRate;
			videoPartyType = ::GetCPH264ResourceVideoPartyType((DWORD)thisFs, (DWORD)thisMbps, FALSE);

			CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();
			Eh264VideoModeType h264videomode = MsSvcVidMode->MinVideoModeForRes(pCapStruct->width,pCapStruct->height);
			eVideoPartyType videoPTAccordingToMsSvc = translateVideoPartyTypeToH264VideoMode(h264videomode);
			videoPartyType = max(videoPartyType,videoPTAccordingToMsSvc);
			POBJDELETE(MsSvcVidMode);

			CSmallString msg;
			cmCapDirection eDirection = GetDirection();
			msg << "Direction: " << eDirection<< " fs: " << thisFs << " mbps: " << thisMbps << " videoPartyType: " << eVideoPartyTypeNames[videoPartyType] << " videoPTAccordingToMsSvc: " << eVideoPartyTypeNames[videoPTAccordingToMsSvc];
			PTRACE2(eLevelInfoNormal,"CMsSvcVideoCap::GetVideoPartyTypeMBPSandFS(): ",msg.GetString());


	}


	return videoPartyType;
}
///////////////////////////////////////////////////////////////////////////////////
EResult CMsSvcVideoCap::SetRtcpFeedbackMask(APIS32 rtcpFbMask)
{
	EResult eRes = kFailure;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->rtcpFeedbackMask = rtcpFbMask;
		eRes = kSuccess;
	}
	return eRes;
}


void CMsSvcVideoCap::SetWidth(APIS32 width)
{
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->width = width;
	}
}

APIS32 CMsSvcVideoCap::GetWidth()
{
	int width = NA;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		width = pCapStruct->width;
	return width;
}

void CMsSvcVideoCap::SetHeight(APIS32 height)
{
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->height = height;
	}
}

APIS32 CMsSvcVideoCap::GetHeight()
{
	int height = NA;
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		height = pCapStruct->height;
	return height;
}



void CMsSvcVideoCap::SetAspectRatio(APIS32 aspectRatio)
{
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->aspectRatio = aspectRatio;
	}

}
APIS32 CMsSvcVideoCap::GetAspectRatio()
{
	int aspectRatio = NA;
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		aspectRatio = pCapStruct->aspectRatio;
	return aspectRatio;
}

void CMsSvcVideoCap::SetMaxFrameRate(APIS32 maxFrameRate)
{
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxFrameRate = maxFrameRate;
	}

}
APIS32 CMsSvcVideoCap::GetMaxFrameRate() const
{
	int maxFrameRate = NA;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		maxFrameRate = pCapStruct->maxFrameRate;
	return maxFrameRate;
}

void CMsSvcVideoCap::SetMaxPixelNum(APIS32 maxPixelNum)
{
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxPixelsNum = maxPixelNum;
	}

}
APIS32 CMsSvcVideoCap::GetMaxPixel() const
{
	int maxPixelNum = NA;
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		maxPixelNum = pCapStruct->maxPixelsNum;
	return maxPixelNum;
}


void CMsSvcVideoCap::SetMaxBitRate(APIS32 maxBitRate)
{
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxBitRate = maxBitRate;
	}

}
////////////////////////////////////
EResult CMsSvcVideoCap::SetBitRate(APIS32 maxBitRate)
{
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->maxBitRate = maxBitRate;
		return kSuccess;
	}
	return kFailure;

}

//////////////////////////////////
APIS32 CMsSvcVideoCap::GetMaxBitRate()
{
	int maxBitRate = NA;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		maxBitRate = pCapStruct->maxBitRate;
	return maxBitRate;
}
APIS32  CMsSvcVideoCap::GetBitRate() const
{
	int maxBitRate = NA;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		maxBitRate = pCapStruct->maxBitRate;
	return maxBitRate;
}
////////////////////////////////////////////////////////////////////
APIS32 CMsSvcVideoCap::GetMinBitRate()
{
	int minBitRate = NA;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		minBitRate = pCapStruct->minBitRate;
	return minBitRate;
}
EResult CMsSvcVideoCap::SetMinBitRate(APIS32 mibBitRate)
{
	int minBitRate = NA;
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
	{
		pCapStruct->minBitRate = mibBitRate;
		return kSuccess;
	}
	return kFailure;

}
EResult CMsSvcVideoCap::CopyBaseQualities(const CBaseVideoCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.GetStruct())
	{
		eRes = kSuccess;
		int sizeBaseWithAnnex = sizeof(msSvcCapStruct);
		int sizeWithoutBaseSize = sizeBaseWithAnnex - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.GetStruct()) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, sizeWithoutBaseSize);
	}
	return eRes;
}



//void CMsSvcVideoCap::SetRtcpFeedbackMask(APIS32 rtcpFeedbackMask)
//{
//	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);
//
//	if (pCapStruct)
//	{
//		pCapStruct->rtcpFeedbackMask = rtcpFeedbackMask;
//	}
//}
//APIS32 CMsSvcVideoCap::GetRtcpFeedbackMask()
//{
//	int rtcpFeedbackMask = NA;
//	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
//	if (pCapStruct)
//		rtcpFeedbackMask = pCapStruct->rtcpFeedbackMask;
//	return rtcpFeedbackMask;
//}

void CMsSvcVideoCap::SetPacketizationMode(APIS8 packetizationMode)
{	
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->packetizationMode = packetizationMode;
	}

}

APIS8 CMsSvcVideoCap::GetPacketizationMode()
{
	int packetizationMode = NA;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		packetizationMode = pCapStruct->packetizationMode;
	return packetizationMode;
}

void CMsSvcVideoCap::SetFiller(APIS8 filler)
{
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->filler = filler;
	}

}
APIS8 CMsSvcVideoCap::GetFiller()
{
	int filler = NA;
	msSvcCapStruct *pCapStruct = CAP_CAST(msSvcCapStruct);
	if (pCapStruct)
		filler = pCapStruct->filler;
	return filler;
}
eVideoPartyType CMsSvcVideoCap::GetCPVideoPartyType() const
{
	BYTE bRes  = FALSE;
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	DWORD thisFs  = 0;
	DWORD thisMbps  = 0;

	eVideoPartyType videoPartyType = eVideo_party_type_dummy;

	if (pCapStruct)
	{

			thisFs = ((pCapStruct->width) * (pCapStruct->height))/256;
			thisMbps =  thisFs * pCapStruct->maxFrameRate;
			videoPartyType = ::GetCPH264ResourceVideoPartyType((DWORD)thisFs, (DWORD)thisMbps, FALSE);
			CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();
			Eh264VideoModeType h264videomode = MsSvcVidMode->MinVideoModeForRes(pCapStruct->width,pCapStruct->height);
			eVideoPartyType videoPTAccordingToMsSvc = translateVideoPartyTypeToH264VideoMode(h264videomode);
			videoPartyType = max(videoPartyType,videoPTAccordingToMsSvc);
			POBJDELETE(MsSvcVidMode);


			CSmallString msg;
			cmCapDirection eDirection = GetDirection();
			msg << "Direction: " << eDirection<< " fs: " << thisFs << " mbps: " << thisMbps <<" videoPTAccordingToMsSvc: " << eVideoPartyTypeNames[videoPTAccordingToMsSvc] <<" videoPartyType: " << eVideoPartyTypeNames[videoPartyType];
			PTRACE2(eLevelInfoNormal,"CMsSvcVideoCap::GetCPVideoPartyType(): ",msg.GetString());


	}
	return videoPartyType;

}
/////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CMsSvcVideoCap::GetRtcpFeedbackMask() const
{
	APIS32 rtcpFeedbackMask = 0;
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
		rtcpFeedbackMask = pCapStruct->rtcpFeedbackMask;

	return rtcpFeedbackMask;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CMsSvcVideoCap::SetMsSvcCapForCpFromMsSvcVideoType(MsSvcVideoModeDetails& MsSvcDetails) 
{
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	if (pCapStruct)
	{
		pCapStruct->aspectRatio = MsSvcDetails.aspectRatio;
		pCapStruct->maxBitRate = MsSvcDetails.maxBitRate/100;
		pCapStruct->minBitRate = MsSvcDetails.minBitRate/100;
		pCapStruct->height = MsSvcDetails.maxHeight;
		pCapStruct->width = MsSvcDetails.maxWidth;
		pCapStruct->maxFrameRate = MsSvcDetails.maxFrameRate;
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////
void CMsSvcVideoCap::GetMbpsAndFsAsDevision(APIS32 &DevMbps,APIS32 &DevFs) const
{
	msSvcCapStruct* pCapStruct = CAP_CAST(msSvcCapStruct);

	DWORD pixels =0;
	DWORD rawFs = 0;
	DWORD rawMbps = 0;

	if (pCapStruct)
	{

		//===========================
		// Calculating raw FS value
		//===========================
		pixels	= (pCapStruct->width) * (pCapStruct->height);
		rawFs	= GetMaxFsAsDevision(pixels);

		//==================================
		// So eW4CIF30 won't seem like CIF
		//==================================
		CSmallString msg;
		msg << "CMsSvcVideoCap::GetMbpsAndFsAsDevision raw FS: " << rawFs;
		PTRACE(eLevelInfoNormal,msg.GetString());
		CMsSvcVideoMode::AdjustFS(pCapStruct->width, pCapStruct->height, rawFs);
		rawMbps	= rawFs * pCapStruct->maxFrameRate;
		msg.Clear();
		msg << "CMsSvcVideoCap::GetMbpsAndFsAsDevision chosen FS/MBPS: " << rawFs << "/" << rawMbps;
		PTRACE(eLevelInfoNormal,msg.GetString());

		//=============================
		// Adjusting to H.245 factors
		//=============================
		DevFs = GetMaxFsAsDevision(rawFs);
		DevMbps = GetMaxMbpsAsDevision(rawMbps);
		msg.Clear();
		msg << "DevFs: " << DevFs<< " DevMbps: " << DevMbps;
		PTRACE2(eLevelInfoNormal,"CMsSvcVideoCap::GetMbpsAndFsAsDevision(): ",msg.GetString());
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~LYNC2013_FEC_RED:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//LYNC2013_FEC_RED: class CFecCap:
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
size_t CFecCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(fecCapStruct);
	return size;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CFecCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(fecCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(fecCapStruct));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CFecCap::Dump(std::ostream& msg) const
{
	fecCapStruct* pCapStruct = CAP_CAST(fecCapStruct);
	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "\n";
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
/*EResult CFecCap::SetStruct(cmCapDataType eType,cmCapDirection eDirection)
{
	EResult eRes = kSuccess;

	fecCapStruct *pCapStruct = CAP_CAST(fecCapStruct);
	if (pCapStruct)
	{
		eRes = CBaseCap::SetStruct(eType,eDirection);
	}
	return eRes;
}*/
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CFecCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	PTRACE(eLevelInfoNormal,"CFecCap::SetDefaults cmCapGeneric");
	EResult eRes = kFailure;
	fecCapStruct* pCapStruct = CAP_CAST(fecCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		//eRes &= CBaseCap::SetStruct(cmCapGeneric, eDirection);
		eRes &= CBaseCap::SetStruct(cmCapGeneric, eDirection, eRole);
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CFecCap::SetDefaultsFec(cmCapDirection eDirection,ERoleLabel eRole, cmCapDataType eType)
{
	PTRACE2INT(eLevelInfoNormal,"CFecCap::SetDefaults eType:",eType);
	EResult eRes = kFailure;
	fecCapStruct* pCapStruct = CAP_CAST(fecCapStruct);
	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(eType, eDirection, eRole);
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CFecCap::CopyBaseQualities(const CBaseVideoCap & otherCap)
{
	EResult eRes = kFailure;
	if (m_pCapStruct && otherCap.GetStruct())
	{
		eRes = kSuccess;
		int sizeBaseWithAnnex = sizeof(genericVideoCapStruct);
		int sizeWithoutBaseSize = sizeBaseWithAnnex - sizeof(BaseCapStruct);
		BYTE *pCapQualities = (BYTE*)(m_pCapStruct) + sizeof(BaseCapStruct);
		BYTE *pOtherCapQualities = (BYTE*)(otherCap.GetStruct()) + sizeof(BaseCapStruct);
		memcpy(pCapQualities, pOtherCapQualities, sizeWithoutBaseSize);
	}
	return eRes;

}
/////////////////////////////////////////
size_t CFecCap::SizeOfBaseStruct() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(fecCapStruct);
	return size;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED: class CRedCap:
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the size of struct.
size_t CRedCap::SizeOf() const
{
	size_t size = 0;
	if (m_pCapStruct)
		size = sizeof(redCapStruct);
	return size;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CRedCap::AllocStruct(size_t size)
{
	if (size && (size > sizeof(redCapStruct)))
		AllocStructBySize(size);
	else
		AllocStructBySize(sizeof(redCapStruct));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CRedCap::Dump(std::ostream& msg) const
{
	redCapStruct* pCapStruct = CAP_CAST(redCapStruct);
	if (pCapStruct)
	{
		CBaseCap::Dump(msg);
		msg << "\n";
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRedCap::SetStruct(cmCapDataType eType,cmCapDirection eDirection)
{
	EResult eRes = kSuccess;

	redCapStruct *pCapStruct = CAP_CAST(redCapStruct);
	if (pCapStruct)
	{
		eRes = CBaseCap::SetStruct(eType,eDirection);
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRedCap::SetDefaults(cmCapDirection eDirection,ERoleLabel eRole)
{
	PTRACE(eLevelInfoNormal,"CRedCap::SetDefaults cmCapGeneric");
	EResult eRes = kFailure;
	redCapStruct *pCapStruct = CAP_CAST(redCapStruct);

	if (pCapStruct)
	{
		eRes = kSuccess;
		eRes &= CBaseCap::SetStruct(cmCapAudio,eDirection,eRole);
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRedCap::CopyQualities(const CBaseCap & otherCap)
{
	PTRACE(eLevelInfoNormal,"CRedCap::CopyQualities");
	EResult eRes = kFailure;
	redCapStruct *pCapStruct = CAP_CAST(redCapStruct);
	CRedCap &hOtherAudioCap  = (CRedCap&)otherCap;

	if (GetCapCode() == hOtherAudioCap.GetCapCode())
	{
		eRes = kSuccess;
	}
	return eRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CRedCap::IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const
{
	BYTE bRes  = TRUE;
	*pDetails = 0x00000000;

	redCapStruct *pCapStruct = CAP_CAST(redCapStruct);
	redCapStruct *pOtherCapStruct = (redCapStruct *)other.GetStruct();

	if (pCapStruct && pOtherCapStruct)
	{
		bRes = CBaseCap::IsContaining(other,valuesToCompare,pDetails);
	}
	// If one of the objects is NULL the function returns FALSE
	// but the details will be empty.
	else
		bRes = FALSE;

	return bRes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
EResult CRedCap::SetStruct(cmCapDirection eDirection,int maxValue,int minValue)
{
	EResult eRes;
	redCapStruct *pCapStruct = CAP_CAST(redCapStruct);

	eRes = CBaseCap::SetStruct(cmCapAudio,eDirection);
	return eRes;
}
