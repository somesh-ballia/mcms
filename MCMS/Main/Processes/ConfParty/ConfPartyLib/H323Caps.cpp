//+========================================================================+
//                            H323Caps.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323Caps.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Michael / Matvey                                            |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 1/02/98    |                                                      |
//+========================================================================+
#include  <ostream>
#include  "H323Caps.h"

#include  "Segment.h"

#include  "Trace.h"
#include  "Macros.h"
#include  "H264Util.h"
#include  "CommModeInfo.h"
#include  "ConfPartyGlobals.h"
#include  "NStream.h"
#include  "SysConfig.h"
#include  "ProcessBase.h"
#include  "IpCommonUtilTrace.h"
#include  "H263VideoMode.h"
#include  "CopVideoTxModes.h"
#include "IpCommon.h"
#include "ServiceConfigList.h"
#include "UnifiedComMode.h"

using namespace std;


#define BUFSIZE(exp)    ( (((exp) / 1024) + 1) * 1024 )

#define RATE_8K	  8
#define RATE_16K  16
#define RATE_24K  24
#define RATE_32K  32
#define RATE_48K  48
#define RATE_56K  56
#define RATE_64K  64
#define RATE_192K 192
#define RATE_128K 128
#define RATE_256K 256
#define RATE_320K 320

#define RATE_512K 512000



/////////////////////////////////////////////////////////////////////////////
CCapH323::CCapH323()  	      // constructor
{
	m_numOfCaps				    = 0;
	m_size						= INITIAL_CAPS_LEN + sizeof(ctCapabilitiesBasicStruct);
	m_pCap						= (ctCapabilitiesStruct*) new BYTE[m_size];
	memset(m_pCap, 0, m_size);
	m_pCap->numberOfCaps		= 0;
	m_pCap->numberOfAlts		= 0;
	m_pCap->numberOfSim			= 0;
	m_offsetWrite			    = 0;
	WORD i = 0;
	for(i=0; i<MAX_CAP_TYPES; i++)
		m_capArray[i]		= 0;
	m_numOfAudioCap			= 0;
	m_numOfVideoCap			= 0;
	m_numOfContentCap		= 0;
	m_numOfDuoCap			= 0;
	m_numOfT120Cap			= 0;
	m_numOfFeccCap			= 0;
	m_numOfPeopContCap		= 0;
	m_numOfH239ControlCap	= 0;
	m_numOfLprCap			= 0;
	m_numOfDynamicPTRControlCap = 0;
	for(i=0; i<(2*MAX_CAP_TYPES); i++)
	{
		m_sortedCap[i].capTypeCode=0;
		m_sortedCap[i].pCapPtr=NULL;
	}
	ZeroingMatchListArray();
	m_dataMaxBitRate	 = 0;
	m_maxContRate		 = 0;
	m_maxContTdmRate	 = 0;
	m_is263Plus			 = 0;
	//bIsRestricted	   = 0;
	m_encAlg			 = kUnKnownMediaType;
	m_contentAltNumber	 = -1;
	m_peopleAltNumber	 = -1;
	m_bIsEPC			 = FALSE;
	m_bIsH239			 = FALSE;
	m_bIsPCversion0		 = FALSE;
	m_bIsDBC2		   	 = FALSE;
	m_h263_4CifMpi		 = -1;	// disable 4cif
	m_videoQuality		 = eVideoQualitySharpness;
	m_bIsLpr			 = FALSE;
	m_bIsDPTR            = FALSE;

	m_numOfNsCap		= 0;
	m_numOfEncrypCap	= 0;
}

/////////////////////////////////////////////////////////////////////////////
CCapH323::CCapH323(const CCapH323 &other)
: CPObject(other)
{
	m_numOfCaps				= other.m_numOfCaps;
	m_numOfAudioCap			= other.m_numOfAudioCap;
	m_numOfVideoCap			= other.m_numOfVideoCap;
	m_numOfContentCap       = other.m_numOfContentCap;
	m_numOfDuoCap		    = other.m_numOfDuoCap;
	m_numOfT120Cap			= other.m_numOfT120Cap;
	m_numOfFeccCap			= other.m_numOfFeccCap;
	m_numOfPeopContCap		= other.m_numOfPeopContCap;
	m_numOfH239ControlCap   = other.m_numOfH239ControlCap;
	m_numOfNsCap			= other.m_numOfNsCap;
	m_numOfEncrypCap		= other.m_numOfEncrypCap;
	m_numOfLprCap			= other.m_numOfLprCap;
	m_size					= other.m_size;
	m_numOfDynamicPTRControlCap = other.m_numOfDynamicPTRControlCap;
	m_pCap					= (ctCapabilitiesStruct*) new BYTE[m_size];
	memcpy(m_pCap, other.m_pCap, m_size);
	m_offsetWrite			= other.m_offsetWrite;
	WORD i = 0;
	for(i=0; i<MAX_CAP_TYPES; i++)
		m_capArray[i] = other.m_capArray[i];
	ZeroingMatchListArray();
	for(i=0;i < (2*MAX_CAP_TYPES);i++)
	{
		m_sortedCap[i].capTypeCode = 0;
		m_sortedCap[i].pCapPtr = NULL;
	}
	m_dataMaxBitRate		= other.m_dataMaxBitRate;
	m_maxContRate			= other.m_maxContRate;
	m_maxContTdmRate		= other.m_maxContTdmRate;

	m_is263Plus				= other.m_is263Plus;
	//bIsRestricted	   = other.bIsRestricted;
	m_encAlg				= other.m_encAlg;

	m_contentAltNumber		= other.m_contentAltNumber;
	m_peopleAltNumber		= other.m_peopleAltNumber;
	m_bIsEPC				= other.m_bIsEPC;
	m_bIsH239				= other.m_bIsH239;
	m_bIsPCversion0			= other.m_bIsPCversion0;
	m_bIsDBC2				= other.m_bIsDBC2;
	m_h263_4CifMpi			= other.m_h263_4CifMpi;
	m_videoQuality			= other.m_videoQuality;
	m_bIsLpr				= other.m_bIsLpr;
	m_bIsDPTR               = other.m_bIsDPTR;

	m_maxContTdmRate		= other.m_maxContTdmRate;
}

/////////////////////////////////////////////////////////////////////////////
/*CCapH323::CCapH323(const CCapH221& capH221, DWORD videoRate,DWORD confRate) :
m_numOfCaps(0),m_numOfAudioCap(0),m_numOfVideoCap(0),m_numOfContentCap(0),m_numOfDuoCap(0),m_numOfT120Cap(0),m_numOfFeccCap(0),
m_numOfPeopContCap(0), m_numOfH239ControlCap(0), m_offsetWrite(0),m_size(INITIAL_CAPS_LEN), m_pCap((ctCapabilitiesStruct*)new BYTE[m_size])
{
	for(WORD i=0; i<MAX_CAP_TYPES; i++)
		m_capArray[i] = 0;

	//There could be some key encryption alg, so I need to ask for each of them if we supported.
	BYTE  bIsAES128		= capH221.GetCapECS()->EncrypAlgCapIncludesAES128();
	if(bIsAES128)
		m_encAlg = kAES_CBC;

	CComMode *pScm = new CComMode;// added for the perpase of creating the preferences at the capability oreder
	Create(*pScm,capH221,videoRate,confRate);
	ZeroingMatchListArray();
	POBJDELETE(pScm);
	m_dataMaxBitRate    = 0;
	m_is263Plus			= 0;

	m_contentAltNumber = -1;
	m_peopleAltNumber  = -1;
	m_bIsEPC		   = FALSE;
	m_bIsH239		   = FALSE;
	m_bIsPCversion0	   = FALSE;
	m_bIsDBC2		   = FALSE;
}*/


/////////////////////////////////////////////////////////////////////////////
CCapH323::~CCapH323()        // destructor
{
	delete[] (BYTE *)m_pCap;
	ReleaseMatchList();
}

//////////////////////////////////////////////////////////////////////////
const char*  CCapH323::NameOf() const
{
	return "CCapH323";
}

/////////////////////////////////////////////////////////////////////////////

#ifdef __H323_SIM__
/////////////////////////////////////////////////////////////////////////////
// for simulation only
void CCapH323::SetSpecificCaps()
{
	WORD  numOfVidCap      = 0;
	WORD  numOfAudioCap    = 0;
	WORD  numOfT120Cap     = 0;
	WORD  numOfPeopContCap = 0;
	WORD  numOfContentCap  = 0;
	WORD  numOfDuoCap  = 0;
	WORD  numOfNsCap       = 0;
	WORD  numOfFeccCap = 0;
	WORD  numOfH239ControlCap = 0;
	WORD  numOfEncCap = 0;


/*	CCapH221 *pH221Cap = new CCapH221;

	pH221Cap->AddEnterprisePeopleContent(Xfer_384,0, 0);
	pH221Cap->SetMVCCap(YES);
	pH221Cap->AddH239Cap(Xfer_384,0, 0);
	pH221Cap->SetMVCCap(YES);	*/


	//------------------------  SET AUDIO  ----------------------------//
	numOfAudioCap += SetAudioCap(eG728CapCode);
	numOfAudioCap += SetAudioCap(eG7221_32kCapCode);
	numOfAudioCap += SetAudioCap(eG7221_24kCapCode);
	numOfAudioCap += SetAudioCap(eG7221_16kCapCode);
	numOfAudioCap += SetAudioCap(eG722_64kCapCode);
	numOfAudioCap += SetAudioCap(eG711Alaw64kCapCode);
	numOfAudioCap += SetAudioCap(eG711Ulaw64kCapCode);
	numOfAudioCap += SetAudioCap(eG7231CapCode);

	//------------------------  SET VIDEO  ----------------------------//
	WORD Mpi = 1;
	DWORD videoRate = 3680;
	numOfVidCap += SetVideoCap(eeH263CapCode, videoRate, Mpi);

	//------------------------  SET NS  Video (Annex I) ----------------------------//
	numOfVidCap += SetNonStandardAnnex();

	//------------------------  SET VIDEO  continue ----------------------------//
	numOfVidCap += SetVideoCap(eeH261CapCode, videoRate, Mpi);
	numOfVidCap += SetRoleLabelCapCode(LABEL_PEOPLE);


	//------------------------  SET CONTENT  ----------------------------//
	//numOfContentCap += SetContent(*pH221Cap);

	//------------------------  SET PEOPLE & CONTENT  ----------------------------//
	numOfPeopContCap	+=	SetContentProfile(2,cmCapReceiveAndTransmit);
	numOfH239ControlCap	+=	SetH239ControlCap(cmCapReceiveAndTransmit);

	//------------------------  END OF CAP CONSTRUCTION  ----------------------------//
	EndOfCapsConstruction(numOfAudioCap,numOfVidCap,numOfContentCap,numOfDuoCap,numOfT120Cap,
							numOfFeccCap,numOfPeopContCap,numOfH239ControlCap,numOfEncCap,numOfNsCap);

	SetPeopleContentAlt();
}

/////////////////////////////////////////////////////////////////////////////
// for simulation only
WORD CCapH323::SetVideoCap(CapEnum videoprotocol, DWORD videoRate, WORD Mpi)
{
	CCapSetInfo capInfo = videoprotocol;
	CBaseVideoCap* pVideoCap = NULL;
	EResult eResOfSet = kSuccess;

	switch(videoprotocol)
	{
	case eH261CapCode:
		{
			pVideoCap = (CH261VideoCap *)CBaseCap::AllocNewCap(videoprotocol,NULL);
			break;
		}
	case eH263CapCode:
		{
			pVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(videoprotocol,NULL);
			break;
		}
	case eH264CapCode:
		{
			pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(videoprotocol,NULL);
			pVideoCap->SetDefaults(cmCapReceive);
			eResOfSet &= pVideoCap->SetBitRate(videoRate);
			((CH264VideoCap*)pVideoCap)->SetMbps(20);
			((CH264VideoCap*)pVideoCap)->SetLevel(H264_Level_1_2);
			break;
		}
	}
	if(videoprotocol != eH264CapCode)
	{
		pVideoCap->SetDefaults(cmCapReceive);
		eResOfSet &= pVideoCap->SetBitRate(videoRate);
		eResOfSet &= pVideoCap->SetFormatMpi(kQCif, Mpi);
		eResOfSet &= pVideoCap->SetFormatMpi(kCif, Mpi);
		if(videoprotocol == eeH263CapCode)
			((CH263VideoCap*)pVideoCap)->SetH263Plus(1,0,0,0,-1,-1,-1,-1,-1);
	}

	int structSize = pVideoCap->SizeOf();
	if (eResOfSet && structSize)
	{
		AddCapToCapBuffer(capInfo,structSize, pVideoCap->GetStruct());
		m_capArray[(CapEnum)capInfo]++;
		m_numOfCaps++;
	}
	if (pVideoCap)
	{
		pVideoCap->FreeStruct();
		POBJDELETE(pVideoCap);
	}
	return 1;
}
#endif// __H323_SIM__

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Creates audio only cap for audio only conf or audio data conf.(G729, G7231, g711)
//---------------------------------------------------------------------------------------------------
//WORD  CCapH323::SetAudioOnlyCap(const CComMode& scm)
WORD  CCapH323::SetAudioOnlyCap(const CComModeH323* pScm, const char* pPartyName)
{
	WORD  numOfAudioCap		= 0;
	DWORD  audioBitRate     = pScm->GetMediaMode(cmCapAudio).GetBitRate();
	BOOL bIsForceG711A = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_FORCE_G711A, bIsForceG711A);

	//BRIDGE-12398
	//if( (FALSE == bIsForceG711A) && (strstr(pPartyName, "##FORCE_MEDIA_A")==NULL) )
	BOOL bIsForceSirenStero = strstr(pPartyName, "##FORCE_MEDIA_ASIREN")!=NULL && strstr(pPartyName, "STEREO")!=NULL; //no support for siren stereo on audio only calls
	BOOL bIsForceMedia 		= strstr(pPartyName, "##FORCE_MEDIA_A")!=NULL;

	if((!bIsForceG711A && !bIsForceMedia) || bIsForceSirenStero)
	{
		// Set the selected mode first
		switch(audioBitRate)
		{
		case RATE_8K:
			numOfAudioCap += SetAudioCap(eG729AnnexACapCode);
			numOfAudioCap += SetAudioCap(eG7231CapCode);
			break;
		case RATE_56K:
			numOfAudioCap += SetAudioCap(eG711Ulaw64kCapCode);
			break;
		}

		CCapSetInfo capInfo = FIRST_AUDIO_ONLY_CAP;
		for (int j = 0; j < MAX_AUDIO_ONLY_CAPS; j++)
		{
			//If it wasn't set before, set now.
			if (((CapEnum)capInfo != eUnknownAlgorithemCapCode) && (OnCap(capInfo)==FALSE))
				numOfAudioCap += SetAudioCap(capInfo);
			capInfo.SetNextAudioOnlyCap();
		}
	}
	else
	{
		numOfAudioCap = SetAudioAccordingToPartyName(pPartyName);
	}

	return numOfAudioCap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Creates audio and data caps for audio only participent (G729, G7231, g711 A/U and T120)
//There is no encryption for data cap!!!
//---------------------------------------------------------------------------------------------------
/*void  CCapH323::CreateAudioDataCap(const CComMode& scm,const CCapH221& h221Cap)
{
	WORD  numOfAudioCap    = 0;
	WORD  numOfT120Cap     = 0;
	m_pCap->numberOfAlts   = 0; //alt's in each Sim.
	m_pCap->numberOfSim	   = 0; //Sim in the Desc.

	//------------------------  SET AUDIO Caps----------------------------//
	numOfAudioCap			= SetAudioOnlyCap(scm);

	numOfT120Cap			+= SetT120(h221Cap);

	m_pCap->numberOfCaps	= m_numOfCaps;
	m_numOfAudioCap			= numOfAudioCap;
	m_numOfVideoCap			= 0;
	m_numOfContentCap       = 0;
	m_numOfDuoCap		    = 0;
	m_numOfT120Cap			= numOfT120Cap;
	m_numOfFeccCap			= 0;
	m_numOfPeopContCap		= 0;
	m_numOfH239ControlCap   = 0;
	m_numOfNsCap			= 0;
	m_numOfEncrypCap		= 0;

	// calculates number of Alt's in each Sim
	if(numOfAudioCap)
		m_pCap->numberOfAlts++;

	if(numOfT120Cap)
		m_pCap->numberOfAlts++;

	// number of Sim's in the Desc is now always = 1 !!!
    m_pCap->numberOfSim  = 1;

	//------------------------  END OF CAP CONSTRUCTION  ----------------------------//
	BuildCapMatrixFromSortedCapBuf();
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapH323::SetEncryptionAlgAccordingToScm(EenMediaType encMediaTypeAlg)
{
	m_encAlg = encMediaTypeAlg;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Creates audio only cap for audio only conf (G729, G7231, g711 A/U)
//---------------------------------------------------------------------------------------------------
//void  CCapH323::CreateAudioOnlyCap(const CComMode& scm)
void  CCapH323::CreateAudioOnlyCap(DWORD videoRate, const CComModeH323* pScm, const char* pPartyName)
{
	TRACEINTO <<" audio only";

	WORD  numOfAudioCap		= 0;
	m_pCap->numberOfAlts	= 0; //alt's in each Sim.
	m_pCap->numberOfSim		= 0; //Sim in the Desc.

	//------------------------  SET AUDIO Caps----------------------------//
	numOfAudioCap	+=	SetAudio(pScm, videoRate, pPartyName,FALSE,
								 FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,TRUE);//BRIDGE-12398 - remove siren stereo for audio only calls

	m_numOfAudioCap			= numOfAudioCap;
	m_numOfVideoCap			= 0;
	m_numOfContentCap       = 0;
	m_numOfDuoCap		    = 0;
	m_numOfT120Cap			= 0;
	m_numOfFeccCap			= 0;
	m_numOfPeopContCap		= 0;
	m_numOfH239ControlCap	= 0;
	m_numOfNsCap			= 0;
	m_numOfEncrypCap		= 0;
	m_numOfLprCap			= 0;
	m_numOfDynamicPTRControlCap = 0;

	EenMediaType encAlg     = pScm->GetEncryptionAlgType();
	SetEncryptionAlgAccordingToScm(encAlg);
	//If the conference is encrypted I need to update the encrypted caps via audio caps
	if(IsPartyEncrypted())
		m_numOfEncrypCap = SetEncryption();

	m_pCap->numberOfCaps	= m_numOfCaps;
	// calculates number of Alt's in each Sim
	if(numOfAudioCap)
		m_pCap->numberOfAlts++;

	// number of Sim's in the Desc is now always = 1 !!!
    m_pCap->numberOfSim  = 1;

	BuildCapMatrixFromSortedCapBuf();
}

/////////////////////////////////////////////////////////////////////////////
void  CCapH323::CreateAllButAudioAndVideo(const CComModeH323* pScm,
							  WORD& numOfT120Cap, WORD& numOfPeopContCap, WORD& numOfH239ControlCap,
							  WORD& numOfContentCap, WORD& numOfDuoCap,  WORD& numOfNsCap,
							  WORD& numOfFeccCap, WORD& numOfLprCap, WORD&numOfEncCap, WORD& numOfDptrCap, EenMediaType encAlg,
							  BYTE isRemoveH239, BYTE isFixContentProtocol, BYTE isRemoveAnnexQ, BYTE isRemoveRvFecc,
							  BYTE isRemoveLpr,BYTE isRemoveEPC, DWORD serviceId, BYTE isRemoveDPTR)
{

	if (pScm->IsTIPContentEnableInH264Scm())
		isRemoveEPC = TRUE;
	BYTE bContetnAsVideo = 0;
	bContetnAsVideo = pScm->GetIsShowContentAsVideo();
	//------------------------  SET CONTENT  ----------------------------//
	numOfContentCap += SetContent(pScm, isRemoveH239, isFixContentProtocol,isRemoveEPC/*,bContetnAsVideo*/);

	//------------------------  SET FECC  ----------------------------//
	numOfFeccCap	+=	SetFecc(pScm, isRemoveAnnexQ, isRemoveRvFecc, serviceId);

	//------------------------  SET PEOPLE & CONTENT  ----------------------------//
	numOfPeopContCap	+=	SetPeopleContent(pScm,isRemoveEPC,1,2);
	numOfH239ControlCap	+=	SetH239Control(pScm, isRemoveH239);

	//------------------------  SET LPR  ----------------------------//
	numOfLprCap			+=  SetLpr(pScm, isRemoveLpr);

	//PTRACE2INT(eLevelInfoNormal,"CCapH323::CreateAllButAudioAndVideo isRemoveDPTR ",isRemoveDPTR);

	numOfDptrCap			+=  SetDynamicPTRepControl(pScm, isRemoveDPTR);
	//------------------------  SET Encryption caps  ----------------------------//
	SetEncryptionAlgAccordingToScm(encAlg);
	if(IsPartyEncrypted())
		numOfEncCap		+= SetEncryption();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::CreateWithDefaultVideoCaps(DWORD videoRate, CComModeH323* pH323Scm, const char* pPartyName, eVideoQuality vidQuality, BYTE isRecordingLink, DWORD serviceId,ECopVideoFrameRate highestframerate, BYTE maxResolution)
{
	PTRACE(eLevelInfoNormal,"CCapH323::CreateWithDefaultVideoCaps");

	WORD  numOfVidCap		= 0;
	WORD  numOfAudioCap		= 0;
	WORD  numOfT120Cap		= 0;
	WORD  numOfPeopContCap	= 0;
	WORD  numOfH239ControlCap = 0;
	WORD  numOfContentCap	= 0;
	WORD  numOfDuoCap		= 0;
	WORD  numOfNsCap		= 0;
	WORD  numOfFeccCap	    = 0;
	WORD  numOfEncCap		= 0;
	WORD  numOfLprCap		= 0;
	WORD  numOfDptrCap		= 0;
	BYTE  isRemoveDPTR      = FALSE;


	//------------------------  SET AUDIO  ----------------------------//
	if(isRecordingLink == TRUE)
	{
		BYTE isRemoveGenericAudioCaps = FALSE;
		BYTE isRemoveG722 = FALSE;
		BYTE isRemoveG7221C = FALSE;
		BYTE isRemoveG729 = TRUE;
		BYTE isRemoveG723 = TRUE;
		numOfAudioCap	+=	SetAudio(pH323Scm, videoRate, pPartyName,FALSE, isRemoveGenericAudioCaps, isRemoveG722, isRemoveG7221C, isRemoveG729, isRemoveG723);
	}
	else
	{
		numOfAudioCap	+=	SetAudio(pH323Scm, videoRate, pPartyName,FALSE);
	}

	//------------------------  SET VIDEO  ----------------------------//
	// since 4cif is asymetric (having ability to send but not to receive), we don't declare on such cap.
	// this member indicates whether we have this capability to send 4cif video cap. if we have a frate will be entered into it. otherwise, it will be set with -1.

	//(video rate is in 100 bits per sec)
	m_videoQuality 			= vidQuality;

	//FROM 7.2 VIDEO RATE IS ACTUALLY CALL RATE WE NEED TO ADJUST THE RATE TO VIDEO RATE
	DWORD audioRate 		= CalculateAudioRate((videoRate*100));
	DWORD actualVideoRate 	= videoRate - (audioRate/100);

	PTRACE2INT(eLevelInfoNormal,"CCapH323::CreateWithDefaultVideoCaps ",actualVideoRate);

	if(pH323Scm->GetMediaType(cmCapVideo, cmCapTransmit) != eH261CapCode)
		m_h263_4CifMpi = CH263VideoMode::GetH263Cif4VideoCardMPI(actualVideoRate, vidQuality);

	PTRACE2INT(eLevelInfoNormal,"CCapH323::CreateWithDefaultVideoCaps -4CIF TRANS: ",m_h263_4CifMpi);
	/*BYTE bIsVSWAutoProtocol = (pH323Scm->GetConfType() == kVideoSwitch) && (pH323Scm->IsAutoVideoProtocol());
	numOfVidCap = CreateDefaultVideoCaps(pH323Scm, bIsVSWAutoProtocol, videoRate);*/
	BYTE bIsAutoProtocol = pH323Scm->IsAutoVideoProtocol();
	BYTE isRemoveContent = FALSE;
	numOfVidCap = CreateDefaultVideoCaps(pH323Scm, bIsAutoProtocol, videoRate, pPartyName, isRemoveContent,FALSE,FALSE,FALSE,FALSE,highestframerate, NULL, maxResolution);

	if (pH323Scm->IsHdVswInMixMode())
	{// update caps according to operation points
		TRACEINTO << "mix_mode: Update Caps for HD720 VSW";
		CVideoOperationPointsSet* pOperationPointsSet = pH323Scm->GetOperationPoints();
		const VideoOperationPoint *pVideoOperationPoint = pOperationPointsSet->GetHighestOperationPoint(pH323Scm->GetPartyId());
		UpdateCapsForHdVswInMixedMode(pH323Scm, pVideoOperationPoint);   //FSN-613: Dynamic Content for SVC/Mix Conf
	}

	// adding a role label cap at the end of video caps for EPC call
//	if (numOfVidCap && h221Cap.IsEnterprisePeopleContent())
//		numOfVidCap += SetRoleLabelCapCode(LABEL_PEOPLE);

	//------------------------  SET Rest of caps  ----------------------------//
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	//	CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();

	BOOL bisuseDPTR = 0;
	pSysConfig->GetBOOLDataByKey("H323_OLC_ACK_DYNAMIC_PAYLOAD_REPLACEMENT", bisuseDPTR);
	if(!bisuseDPTR)
	{
		isRemoveDPTR = TRUE;
		//PTRACE2INT(eLevelInfoNormal,"CCapH323::CreateWithDefaultVideoCaps inside !bisuseDPTR",isRemoveDPTR);
	}


	CreateAllButAudioAndVideo(pH323Scm, numOfT120Cap, numOfPeopContCap, numOfH239ControlCap,
		              numOfContentCap, numOfDuoCap, numOfNsCap, numOfFeccCap, numOfLprCap, numOfEncCap, numOfDptrCap,
		              pH323Scm->GetEncryptionAlgType(), isRemoveContent,FALSE/*isFixContentProtocol*/,FALSE/*isRemoveAnnexQ*/,FALSE/*isRemoveRvFecc*/,FALSE /*isRemoveLpr*/,FALSE/*isRemoveEPC*/,serviceId, isRemoveDPTR);


	//------------------------  END OF CAP CONSTRUCTION  ----------------------------//
	EndOfCapsConstruction(numOfAudioCap,numOfVidCap,numOfContentCap,numOfDuoCap,numOfT120Cap,
						  numOfFeccCap, numOfPeopContCap,numOfH239ControlCap, numOfEncCap,numOfNsCap,numOfLprCap, numOfDptrCap);

	SetPeopleContentAlt();

//	Dump("CCapH323::CreateWithDefaultVideoCaps - show build caps:", eLevelInfoNormal);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::BuildCapsWithSpecialCaps(DWORD videoRate, CComModeH323* pH323Scm, const char* pPartyName,
											BYTE isRemoveGenericAudioCaps, BYTE isRemoveH239, BYTE isFixContentProtocol, BYTE isRemoveAnnexQ, BYTE isRemoveRvFecc,
											BYTE isRemoveGenericVideoCap, BYTE isRemoveG722,
											BYTE isRemoveH264, BYTE isRemoveDBC2, BYTE isRemoveOtherThenQCif,
											BYTE isRemoveG7221C,BYTE isRemoteMgcWithLowRateConf, BYTE isRemoveLpr,
											BYTE isCascadeOrRecordingLink,BYTE isRemoveEPC,BYTE isRemoveG7231,BYTE isRemoveG719, DWORD serviceId,ECopVideoFrameRate highestframerate,CCopVideoTxModes* pCopVideoTxModes, BYTE isRemoveDPTR)
{
	PTRACE(eLevelInfoNormal,"CCapH323::BuildCapsWithSpecialCaps");
	if (pH323Scm->IsTIPContentEnableInH264Scm())
		isRemoveEPC = TRUE;
	WORD  numOfVidCap		= 0;
	WORD  numOfAudioCap		= 0;
	WORD  numOfT120Cap		= 0;
	WORD  numOfPeopContCap	= 0;
	WORD  numOfH239ControlCap = 0;
	WORD  numOfContentCap	= 0;
	WORD  numOfDuoCap		= 0;
	WORD  numOfNsCap		= 0;
	WORD  numOfFeccCap	    = 0;
	WORD  numOfEncCap		= 0;
	WORD  numOfLprCap		= 0;
	WORD  numOfDptrCap		= 0;
	//BYTE  isRemoveDPTR      = FALSE;



	//------------------------  SET AUDIO  ----------------------------//
    if(isCascadeOrRecordingLink == TRUE)
	{
		BYTE isRemoveGenericAudioCaps = FALSE;
		BYTE isRemoveG722 = FALSE;
		BYTE isRemoveG7221C = FALSE;
		BYTE isRemoveG729 = TRUE;
		BYTE isRemoveG723 = TRUE;
		numOfAudioCap	+=	SetAudio(pH323Scm, videoRate, pPartyName,isRemoteMgcWithLowRateConf, isRemoveGenericAudioCaps, isRemoveG722, isRemoveG7221C, isRemoveG729, isRemoveG723,isRemoveG719);
	}
    else
    {
    	BYTE isRemoveG729 = FALSE;
        numOfAudioCap	+=	SetAudio(pH323Scm, videoRate, pPartyName,isRemoteMgcWithLowRateConf, isRemoveGenericAudioCaps, isRemoveG722, isRemoveG7221C,isRemoveG729,isRemoveG7231,isRemoveG719);
    }

	//------------------------  SET VIDEO  ----------------------------//
	if(pH323Scm->IsMediaOn(cmCapVideo))
	{
		BYTE bIsAutoProtocol = pH323Scm->IsAutoVideoProtocol();
		numOfVidCap = CreateDefaultVideoCaps(pH323Scm, bIsAutoProtocol, videoRate, pPartyName, isRemoveH239, isRemoveGenericVideoCap, isRemoveH264, isRemoveOtherThenQCif, isRemoveDBC2, highestframerate,pCopVideoTxModes);


	//------------------------  SET Rest of caps  ----------------------------//

		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		BOOL bisuseDPTR = 0;
		pSysConfig->GetBOOLDataByKey("H323_OLC_ACK_DYNAMIC_PAYLOAD_REPLACEMENT", bisuseDPTR);
		if(!bisuseDPTR)
		{
			//PTRACE2INT(eLevelInfoNormal,"CCapH323::BuildCapsWithSpecialCaps inside !bisuseDPTR",isRemoveDPTR);
			isRemoveDPTR = TRUE;
		}
	CreateAllButAudioAndVideo(pH323Scm, numOfT120Cap, numOfPeopContCap, numOfH239ControlCap,
		              			numOfContentCap, numOfDuoCap, numOfNsCap, numOfFeccCap, numOfLprCap, numOfEncCap, numOfDptrCap, pH323Scm->GetEncryptionAlgType(),
		              			isRemoveH239, isFixContentProtocol, isRemoveAnnexQ, isRemoveRvFecc, isRemoveLpr,isRemoveEPC, serviceId, isRemoveDPTR);
	}
    else
    {
        SetEncryptionAlgAccordingToScm(pH323Scm->GetEncryptionAlgType());
        //If the conference is encrypted I need to update the encrypted caps via audio caps
        if(IsPartyEncrypted())
        	numOfEncCap = SetEncryption();
    }


	//------------------------  END OF CAP CONSTRUCTION  ----------------------------//
	EndOfCapsConstruction(numOfAudioCap,numOfVidCap,numOfContentCap,numOfDuoCap,numOfT120Cap,
						  numOfFeccCap, numOfPeopContCap,numOfH239ControlCap, numOfEncCap,numOfNsCap,numOfLprCap, numOfDptrCap);

	SetPeopleContentAlt();
}

/////////////////////////////////////////////////////////////////////////////
//Description: In case of VSW auto protocol: add default caps for lower protocol, than the scm protocol.
WORD  CCapH323::CreateDefaultVideoCaps(const CComModeH323* pH323Scm, BYTE bIsAutoProtocol, DWORD videoRate, const char* pPartyName, BYTE isRemoveContent,
										BYTE isRemoveGenericVideoCap, BYTE isRemoveH264, BYTE isRemoveOtherThenQCif, BYTE isRemoveDBC2,
										ECopVideoFrameRate highestframerate, CCopVideoTxModes* pCopVideoTxModes, BYTE maxResolution)
{
	WORD numOfVidCap = 0;
	CapEnum scmProtocol = (CapEnum)pH323Scm->GetMediaType(cmCapVideo, cmCapReceive);

	PTRACE2INT(eLevelInfoNormal,"CCapH323::CreateDefaultVideoCaps : scmProtocol = ", scmProtocol);

	if((scmProtocol == eH264CapCode) && (isRemoveH264 == TRUE))
		PTRACE(eLevelInfoNormal,"CCapH323::CreateDefaultVideoCaps - Remote can't support H264. Remove the cap from the caps list");
	else
		numOfVidCap += SetVideoCapsFromH323Scm(*pH323Scm, kRolePeople, TRUE, confTypeUnspecified, isRemoveGenericVideoCap, isRemoveOtherThenQCif,highestframerate, maxResolution);

	if (bIsAutoProtocol)
    {
		EConfType confType = pH323Scm->GetConfType();

		if (scmProtocol == eH264CapCode && pH323Scm->GetConfType() != kVSW_Fixed)
		{
			DWORD callRate = pH323Scm->GetCallRate();
			numOfVidCap += SetH263DefaultCap(videoRate, confType, callRate, isRemoveOtherThenQCif);
		}

		if(scmProtocol != eH261CapCode)
		{// we always add H261 caps.
			CCapSetInfo capInfo(eH261CapCode);
			if(capInfo.IsSupporedCap())
				numOfVidCap += SetH261DefaultCap(videoRate, pPartyName);
		}
	}

	if(pCopVideoTxModes)
			numOfVidCap += AddVidTxModesForCop(pCopVideoTxModes);



	// add role label people in case of EPC
	if(numOfVidCap)
	{// 1. check the SCM support content, 2. the content is not removed, 3. EPC is enabled.
		BOOL bSupportEPC = GetSystemCfgFlagInt<BOOL>("ENABLE_EPC");
		if(bSupportEPC)
		{
			if (pH323Scm->IsContent(cmCapReceiveAndTransmit) && (isRemoveContent == FALSE))
			{
				numOfVidCap += SetRoleLabelCapCode(LABEL_PEOPLE);
			}
		}
	}

	// add role label people in case of EPC
	if(numOfVidCap)
	{// 1. check the SCM support content, 2. the content is not removed, 3. EPC is enabled.
		BOOL bSupportEPC = GetSystemCfgFlagInt<BOOL>("ENABLE_EPC");
		if(bSupportEPC)
		{
			PTRACE(eLevelInfoNormal,"CCapH323::CreateDefaultVideoCaps Add Lable people");
			if (pH323Scm->IsContent(cmCapReceiveAndTransmit) && isRemoveContent == FALSE)
			{
				numOfVidCap += SetRoleLabelCapCode(LABEL_PEOPLE);
			}
		}
	}

	return numOfVidCap;
}


/////////////////////////////////////////////////////////////////////////////
//Description: Create caps, which contains only video caps, that we support in highest common mechanism
void CCapH323::CreateFromOtherSupportedVideo(const CCapH323* other, APIS32 minVideoRate)
{
	PTRACE(eLevelInfoNormal,"CCapH323::CreateFromOtherSupportedVideo");
	WORD numOfCaps = 0;
	capBuffer* pOtherCapBuffer = (capBuffer *) &other->m_pCap->caps;
	char *otherTempPtr    = (char *)pOtherCapBuffer;

	// this function set only people caps (we use later other->CheckRole(kRolePeople,j))
	m_peopleAltNumber = 0; // 0 => people
	for (int j=0; j<other->m_numOfCaps; j++)
	{
		CapEnum capCode = (CapEnum)pOtherCapBuffer->capTypeCode;
		if( (capCode == eH261CapCode) || (capCode == eH263CapCode) ||
			(capCode == eH264CapCode) || (capCode == eNonStandardCapCode) )
		{
			CBaseCap* pOtherCap = CBaseCap::AllocNewCap(capCode, pOtherCapBuffer->dataCap);
			if (pOtherCap)
			{
				if (pOtherCap->IsType(cmCapVideo) && other->CheckRole(kRolePeople,j,pOtherCap) )
				{
					if (pOtherCap->GetBitRate() >= minVideoRate)
					{
						int structSize = pOtherCap->SizeOf();
						AddCapToCapBuffer(capCode, structSize, pOtherCap->GetStruct());
						m_capArray[pOtherCapBuffer->capTypeCode]++;
						numOfCaps++;
						m_numOfCaps++;
					}
				}

				else if (capCode == eNonStandardCapCode)
				{
					CNonStandardCap* pNsCap = (CNonStandardCap*)CBaseCap::AllocNewCap(eNonStandardCapCode, pOtherCapBuffer->dataCap);
					if (pNsCap)
					{
						BYTE bIsAnnexI_NS = pNsCap->IsNonStandardAnnex(typeAnnexI_NS);
						if (bIsAnnexI_NS)
						{
							int structSize = pNsCap->SizeOf();
							AddCapToCapBuffer(eNonStandardCapCode, structSize, pNsCap->GetStruct());
							m_capArray[eNonStandardCapCode]++;
							numOfCaps++;
							m_numOfCaps++;
						}
					}
					POBJDELETE (pNsCap);
				}

				POBJDELETE (pOtherCap);
			}
		}
		otherTempPtr += sizeof(capBufferBase) + pOtherCapBuffer->capLength;
		pOtherCapBuffer = (capBuffer*)otherTempPtr;
	}

	EndOfCapsConstruction(0,numOfCaps,0,0,0, 0,0,0,0);
}

/////////////////////////////////////////////////////////////////////////////
// Function name:	SetRestric
//
// Description:		Setting restric mode from H320 restric mode
//
/////////////////////////////////////////////////////////////////////////////
/*void  CCapH323::SetRestric(const CComMode& scm)
{
	bIsRestricted = (scm.GetOtherRestrictMode() == Restrict) ? YES : NO;
}*/

/////////////////////////////////////////////////////////////////////////////
// Function name:	GetRestric
//
// Description:		Getting restric mode if there is.
//
/////////////////////////////////////////////////////////////////////////////
/*BYTE  CCapH323::GetRestric() const
{
	return bIsRestricted;
}*/
/////////////////////////////////////////////////////////////////////////////
// Function name:	IsPartyEncrypted
//
// Description:		Return TRUE if the encryption algorithm is different from unKnown.
//
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsPartyEncrypted() const
{
	if(m_encAlg != kUnKnownMediaType)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Function name:	IsEncrypedCap
//
// Description:		Return TRUE if the exact cap is encrypt.
//
/////////////////////////////////////////////////////////////////////////////
BOOL CCapH323::IsEncrypedCap(int index) const
{
	capBuffer		*pCapBuffer		= (capBuffer *) &m_pCap->caps;
	char			*tempPtr		= (char *) pCapBuffer;
	BaseCapStruct	*pCapBaseStruct = NULL;

	APIU16 entry = 0;
	for(APIU16 i=0; i < m_numOfCaps; i++)
	{
		pCapBaseStruct = (BaseCapStruct*)pCapBuffer->dataCap;
		if(pCapBaseStruct->header.type == cmCapH235)
		{
			CEncryptionCap *pCap = (CEncryptionCap *)CBaseCap::AllocNewCap(eEncryptionCapCode,pCapBuffer->dataCap);
			if(pCap)
			{
				entry = pCap->GetEntry();
				POBJDELETE(pCap);
				if(entry == index)
					return TRUE; //I found 235 (cap that encrypt another cap) that encrypt the cap that in the array cap.
			}
			else
				PTRACE(eLevelInfoNormal,"CCapH323::IsEncrypedCap pCap is NULL");
		}
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

	return FALSE; //we get out from the loop and did not find 235 cap that encrypt our cap
}

//////////////////////////////////////////////////////////////////////////
//Get the encrypted posion of the regular cap.
int CCapH323::GetEncryptedPosition(int position,capBuffer **ppCapBuffer) const
{
	BYTE		rval				= 0;
	char		*pTempPtr;
	APIU8		type;
	APIU16		entry;
	capBuffer   *pCapBuffer	= (capBuffer *) &m_pCap->caps;

	for(int i=0; i<m_numOfCaps; i++)
	{
		type	  = (( BaseCapStruct*)pCapBuffer->dataCap)->header.type;
		if(type == cmCapH235) //encrypted cap
		{
			entry = ((encryptionCapStruct *)pCapBuffer->dataCap)->entry;
			if(entry == position)
			{
				if(ppCapBuffer)
					*ppCapBuffer = pCapBuffer;
				return i; //index of the encrypted cap
			}
		}

		pTempPtr = (char*)pCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return -1; //not found!!
}

/////////////////////////////////////////////////////////////////////////////
// Function name:	UpdateEncryptedCaps
//
// Description:		Update the entries inside the 235 cap. We should do it before remove any cap from the capabilities.
//					we receive the position from there we should decrease the entry.
// Return value:	Type [WORD]:  Number of added audio caps
/////////////////////////////////////////////////////////////////////////////
void CCapH323::UpdateEncryptedCaps(int encryptedPosition,capBuffer* pCapBuffer)
{
	PTRACE(eLevelInfoNormal,"CCapH323::UpdateEncryptedCaps");

	char	*pTempPtr = (char*)pCapBuffer;
	APIU8	type;

	pTempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
	pCapBuffer = (capBuffer*)pTempPtr;

	for(int i=encryptedPosition+1; i<m_numOfCaps; i++)
	{
		type	  = (( BaseCapStruct*)pCapBuffer->dataCap)->header.type;
		if(type == cmCapH235) //encrypted cap
			((encryptionCapStruct *)pCapBuffer->dataCap)->entry--;

		pTempPtr = (char*)pCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapH323::FixEncryptedCapsBackward (int position)
{
	PTRACE2INT(eLevelInfoNormal, "CCapH323::FixEncryptedCapsBackward %d", position);
	capBuffer   *pCapBuffer	= (capBuffer *) &m_pCap->caps;
	char	*pTempPtr = (char*)pCapBuffer;
	APIU8	type;

	pTempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
	pCapBuffer = (capBuffer*)pTempPtr;

	if (!pCapBuffer)
		return;
	for(int i=0; i<m_numOfCaps; i++)
	{
		type = (( BaseCapStruct*)pCapBuffer->dataCap)->header.type;
		if(type == cmCapH235 && ((encryptionCapStruct *)pCapBuffer->dataCap)->entry > position) //encrypted cap
			((encryptionCapStruct *)pCapBuffer->dataCap)->entry--;

		pTempPtr = (char*)pCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
}
/////////////////////////////////////////////////////////////////////////////
// Function name:	SetAudio
//
// Description:		Setting H323 Audio Capabilities from H320 Capabilities object
//
// Return value:	Type [WORD]:  Number of added audio caps
/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetAudio(const CComModeH323* pScm, DWORD videoRate, const char* pPartyName,BYTE isRemoteMgcWithLowRateConf ,
						 BYTE isRemoveGenericAudioCaps, BYTE isRemoveG722, BYTE isRemoveG7221C, BYTE isRemoveG729, BYTE isRemoveG723,BYTE isRemoveG719, BYTE isRemoveSirenStereo) //BRIDGE-12398 isRemoveSirenStereo
{
	WORD numOfCaps = 0;
	BOOL bIsForceG711A = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_FORCE_G711A, bIsForceG711A);

	//BRIDGE-12398
	//if( (FALSE == bIsForceG711A) && (strstr(pPartyName, "##FORCE_MEDIA_A")==NULL) )
	BOOL bIsForceSirenStero = strstr(pPartyName, "##FORCE_MEDIA_ASIREN")!=NULL && strstr(pPartyName, "STEREO")!=NULL; //no support for siren stereo on audio only calls
	BOOL bIsForceMedia 		= strstr(pPartyName, "##FORCE_MEDIA_A")!=NULL;

	if((!bIsForceG711A && !bIsForceMedia) || (bIsForceSirenStero && isRemoveSirenStereo))
	{
        // set the prefered audio algorithm according to the Scm
		DWORD audioBitRate = pScm->GetMediaMode(cmCapAudio).GetBitRate() * _K_;
       // DWORD confBitRate  = scm.GetNumB0Chnl()*64;
		DWORD confBitRate  = pScm->GetCallRate() * 1000;

		BYTE isRemoveSirenLPR  = FALSE;

		//Patch - In Audio only calls we will remove SirenLPR from our caps so the HDX will open incoming audio channels.
		//if(!pScm->IsMediaOn(cmCapVideo, cmCapTransmit,kRolePeople))
			//isRemoveSirenLPR = TRUE;

		numOfCaps = SetAudioAccordingToRate(audioBitRate, confBitRate, videoRate * 100,isRemoteMgcWithLowRateConf, isRemoveGenericAudioCaps, isRemoveG722, isRemoveG7221C,isRemoveG729, isRemoveG723,isRemoveG719,isRemoveSirenLPR, isRemoveSirenStereo); //BRIDGE-12398 isRemoveSirenStereo
	}
	else
	{
		numOfCaps = SetAudioAccordingToPartyName(pPartyName, isRemoveGenericAudioCaps, isRemoveG722);
	}

	return numOfCaps;
}

////////////////////////////////////////////////////////////////////////////
WORD  CCapH323:: SetAudioAccordingToPartyName(const char* pPartyName, BYTE isRemoveGenericAudioCaps, BYTE isRemoveG722)
{
	WORD numOfCaps = 0;
	BYTE isRemoveAudioCap = FALSE;

	BOOL bIsForceG711A = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_FORCE_G711A, bIsForceG711A);

	if( (TRUE == bIsForceG711A) || (strstr(pPartyName, "##FORCE_MEDIA_AG711_A") != NULL) )
		numOfCaps += SetAudioCap(eG711Alaw64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG711_U")!=NULL)
		numOfCaps += SetAudioCap(eG711Ulaw64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1_16K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG7221_16kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1_24K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG7221_24kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1_32K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG7221_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7231")!=NULL)
		numOfCaps += SetAudioCap(eG7231CapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG729")!=NULL)
		numOfCaps += SetAudioCap(eG729AnnexACapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG728")!=NULL)
			numOfCaps += SetAudioCap(eG728CapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN7_16K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren7_16kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14_48K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren14_48kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14_32K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren14_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14_24K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren14_24kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_48K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren14Stereo_48kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_56K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren14Stereo_56kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_64K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren14Stereo_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_96K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren14Stereo_96kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO_128K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren22Stereo_128kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO_96K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren22Stereo_96kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO_64K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren22Stereo_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22_64K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren22_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22_48K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren22_48kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22_32K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSiren22_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_32K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSirenLPR_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_48K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSirenLPR_48kCapCode);
	}

	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_64K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSirenLPR_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_64K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSirenLPRStereo_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_96K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSirenLPRStereo_96kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_128K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eSirenLPRStereo_128kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_64K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG719_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_48K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG719_48kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_32K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG719_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO_128K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG719Stereo_128kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO_96K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG719Stereo_96kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO_64K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG719Stereo_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C_24K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG7221C_24kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C_32K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG7221C_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C_48K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG7221C_48kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_32K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG719_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_48K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG719_48kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_64K")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
			numOfCaps += SetAudioCap(eG719_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eG7221_16kCapCode);
			numOfCaps += SetAudioCap(eG7221_24kCapCode);
			numOfCaps += SetAudioCap(eG7221_32kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN7")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eSiren7_16kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eSiren14_24kCapCode);
			numOfCaps += SetAudioCap(eSiren14_32kCapCode);
			numOfCaps += SetAudioCap(eSiren14_48kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eSiren14Stereo_48kCapCode);
//			numOfCaps += SetAudioCap(eSiren14Stereo_56kCapCode);
			numOfCaps += SetAudioCap(eSiren14Stereo_64kCapCode);
			numOfCaps += SetAudioCap(eSiren14Stereo_96kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eSiren22Stereo_128kCapCode);
			numOfCaps += SetAudioCap(eSiren22Stereo_96kCapCode);
			numOfCaps += SetAudioCap(eSiren22Stereo_64kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22")!=NULL)
	{// keep this order due to strstr function
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eSiren22_64kCapCode);
			numOfCaps += SetAudioCap(eSiren22_48kCapCode);
			numOfCaps += SetAudioCap(eSiren22_32kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO")!=NULL)
	{// keep this order due to strstr function
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eSirenLPRStereo_64kCapCode);
			numOfCaps += SetAudioCap(eSirenLPRStereo_96kCapCode);
			numOfCaps += SetAudioCap(eSirenLPRStereo_128kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR")!=NULL)
	{// keep this order due to strstr function
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eSirenLPR_32kCapCode);
			numOfCaps += SetAudioCap(eSirenLPR_48kCapCode);
			numOfCaps += SetAudioCap(eSirenLPR_64kCapCode);
		}
	}

	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eG719Stereo_128kCapCode);
			numOfCaps += SetAudioCap(eG719Stereo_96kCapCode);
			numOfCaps += SetAudioCap(eG719Stereo_64kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719")!=NULL)
	{// keep this order due to strstr function
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eG719_64kCapCode);
			numOfCaps += SetAudioCap(eG719_48kCapCode);
			numOfCaps += SetAudioCap(eG719_32kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eG7221C_24kCapCode);
			numOfCaps += SetAudioCap(eG7221C_32kCapCode);
			numOfCaps += SetAudioCap(eG7221C_48kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719")!=NULL)
	{
		if(isRemoveGenericAudioCaps == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eG719_32kCapCode);
			numOfCaps += SetAudioCap(eG719_48kCapCode);
			numOfCaps += SetAudioCap(eG719_64kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722STEREO")!=NULL)
	{
		if(isRemoveG722 == TRUE)
			isRemoveAudioCap++;
		else
		{
			BOOL isSLyncEnableG722Stereo = FALSE;
			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			pSysConfig->GetBOOLDataByKey("LYNC2013_ENABLE_G722Stereo128k", isSLyncEnableG722Stereo);
			TRACEINTO << "LYNC_G722Stereo128k - isSLyncEnableG722Stereo:" << (DWORD)isSLyncEnableG722Stereo;
			if (isSLyncEnableG722Stereo == TRUE)
				numOfCaps += SetAudioCap(eG722Stereo_128kCapCode);
		}
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722")!=NULL)// must be last because it can be confuse with the ##FORCE_MEDIA_AG7221C
	{
		if(isRemoveG722 == TRUE)
			isRemoveAudioCap++;
		else
		{
			numOfCaps += SetAudioCap(eG722_64kCapCode);
		}
	}

	if(isRemoveAudioCap)
	{
		PTRACE2(eLevelInfoNormal,"CCapH323::SetAudioAccordingToPartyName - Remote can't support Force party name. Declare on G711U only.", pPartyName);
		numOfCaps += SetAudioCap(eG711Ulaw64kCapCode);
	}

	return numOfCaps;
}

////////////////////////////////////////////////////////////////////////////
// Audio rate should be bit/second
// Conf rate should be bit/second
// video rate should be bit/second
WORD  CCapH323:: SetAudioAccordingToRate(DWORD audioBitRate, DWORD confBitRate, DWORD videoRate,BYTE isRemoteMgcWithLowRateConf, BYTE isRemoveGenericAudioCaps, BYTE isRemoveG722, BYTE isRemoveG7221C, BYTE isRemoveG729, BYTE isRemoveG723,BYTE isRemoveG719,BYTE isRemoveSirenLPR , BYTE isRemoveSirenStereo)//BRIDGE-12398 isRemoveSirenStereo
{
	TRACEINTO
		<< "ConfRate:" << confBitRate
		<< ", AudioRate:" << audioBitRate
		<< ", VideoRate:" << videoRate
		<< ", isRemoteMgcWithLowRateConf:" << (int)isRemoteMgcWithLowRateConf
		<< ", isRemoveGenericAudioCaps:" << (int)isRemoveGenericAudioCaps
		<< ", isRemoveG722:" << (int)isRemoveG722
		<< ", isRemoveG7221C:" << (int)isRemoveG7221C
		<< ", isRemoveG722:" << (int)isRemoveG729
		<< ", isRemoveG7221C:" << (int)isRemoveG723
		<< ", isRemoveG722:" << (int)isRemoveG719
		<< ", isRemoveG7221C:" << (int)isRemoveSirenLPR;



	WORD numOfCaps = 0;
	uint32_t isSirenLPRSet = 0;
	bool isG719Set = false;
	bool isG719StereoSet = false;

	if(isRemoveSirenLPR)
		isSirenLPRSet = 1;

	// 1- Set the selected mode first
	if(isRemoteMgcWithLowRateConf==FALSE)
	{
		switch(audioBitRate)
		{
			case rate8K:
				if(isRemoveG729 == FALSE)
				{
					numOfCaps += SetAudioCap(eG729AnnexACapCode);
				}
				if(isRemoveG723 == FALSE)
				{
					numOfCaps += SetAudioCap(eG7231CapCode);
				}
				break;
			case rate16K:
				if(isRemoveGenericAudioCaps == FALSE)
					numOfCaps += SetAudioCap(eG7221_16kCapCode);
				numOfCaps += SetAudioCap(eG728CapCode);
				break;
			case rate24K:
				if(isRemoveGenericAudioCaps == FALSE)
				{
					if (isRemoveG7221C == FALSE)
						numOfCaps += SetAudioCap(eG7221C_24kCapCode);
					numOfCaps += SetAudioCap(eG7221_24kCapCode);
				}
				break;
			case rate32K:
				if(isRemoveGenericAudioCaps == FALSE)
				{
					if(isRemoveG719 == FALSE)
						numOfCaps += SetAudioCap(eG719_32kCapCode);
					numOfCaps += SetAudioCap(eSiren22_32kCapCode);
					if (isRemoveG7221C == FALSE)
						numOfCaps += SetAudioCap(eG7221C_32kCapCode);
					numOfCaps += SetAudioCap(eG7221_32kCapCode);
				}
				break;
			case rate48K:
				if(isRemoveGenericAudioCaps == FALSE)
				{
					if(isRemoveSirenStereo == FALSE) //BRIDGE-12398
						numOfCaps += SetAudioCap(eSiren14Stereo_48kCapCode);
					if(isRemoveG719 == FALSE)
						numOfCaps += SetAudioCap(eG719_48kCapCode);
					numOfCaps += SetAudioCap(eSiren22_48kCapCode);
					if (isRemoveG7221C == FALSE)
						numOfCaps += SetAudioCap(eG7221C_48kCapCode);
				}
				break;
			case rate56K:
				numOfCaps += SetAudioCap(eG711Ulaw64kCapCode);
				numOfCaps += SetAudioCap(eG711Alaw64kCapCode);
				break;
			case rate64K:
				if(isRemoveGenericAudioCaps == FALSE)
				{
					if(isRemoveG719 == FALSE)
						numOfCaps += SetAudioCap(eG719Stereo_64kCapCode);
					if(isRemoveSirenStereo == FALSE) //BRIDGE-12398
						numOfCaps += SetAudioCap(eSiren14Stereo_64kCapCode);

					numOfCaps += SetAudioCap(eSiren22_64kCapCode);
				}
				if(isRemoveG722 == FALSE)
					numOfCaps += SetAudioCap(eG722_64kCapCode);
				numOfCaps += SetAudioCap(eG711Ulaw64kCapCode);
				numOfCaps += SetAudioCap(eG711Alaw64kCapCode);
				break;
			case rate96K:
				if(isRemoveGenericAudioCaps == FALSE)
				{
					if(isRemoveG719 == FALSE)
						numOfCaps += SetAudioCap(eG719Stereo_96kCapCode);
				}
				break;
			case rate128K:
				if(isRemoveGenericAudioCaps == FALSE)
				{
					if(isRemoveG719 == FALSE)
						numOfCaps += SetAudioCap(eG719Stereo_128kCapCode);
				}
				if (!isRemoveGenericAudioCaps && !isRemoveG722)
				{
					BOOL isSLyncEnableG722Stereo = FALSE;
					CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
					pSysConfig->GetBOOLDataByKey("LYNC2013_ENABLE_G722Stereo128k", isSLyncEnableG722Stereo);
					TRACEINTO << "LYNC_G722Stereo128k - isSLyncEnableG722Stereo:" << (DWORD)isSLyncEnableG722Stereo;
					if (isSLyncEnableG722Stereo == TRUE)
						numOfCaps += SetAudioCap(eG722Stereo_128kCapCode);
				}
				break;
		}
	}

	// 2- Set the rest of the audio algorithems according to conf rate
	CapEnum capOrderArr[MAX_SUPPORTED_AUDIO_CAPS];
	int i = 0;

	DWORD bitRateForAudioAlgsOrder = confBitRate;

	if (videoRate)
	{// need to make the calculated rate a multiple of 64k
		DWORD calculatedVideoRate = videoRate;
		calculatedVideoRate = (calculatedVideoRate % rate64K) ? (calculatedVideoRate/rate64K + 1) * rate64K : (calculatedVideoRate/rate64K) * rate64K;
		bitRateForAudioAlgsOrder = calculatedVideoRate;
	}

	// Set the selected mode first
	PTRACE(eLevelInfoNormal,"CCapH323::SetAudioAccordingToRate - entering second stage");
	if (bitRateForAudioAlgsOrder <= rate192K && isRemoteMgcWithLowRateConf == FALSE)
	{
		switch(bitRateForAudioAlgsOrder)
		{
			case rate64K:
				if(isRemoveG729 == FALSE)
				{
					capOrderArr[i++] = eG729AnnexACapCode;
				}
				if(isRemoveG723 == FALSE)
				{
					capOrderArr[i++] = eG7231CapCode;
				}
				if(isRemoveGenericAudioCaps == FALSE)
				{
					capOrderArr[i++] = eG7221_16kCapCode;
				}
				capOrderArr[i++]= eG728CapCode;
				if(isRemoveGenericAudioCaps == FALSE)
				{
					if (isRemoveG7221C == FALSE)
						capOrderArr[i++] = eG7221C_24kCapCode;
				}
				if(isRemoveG722 == FALSE)
		    		capOrderArr[i++] = eG722_64kCapCode;
		    	capOrderArr[i++] = eG711Ulaw64kCapCode;
		    	capOrderArr[i++] = eG711Alaw64kCapCode;
				break;

			case rate128K:
				/*if (!isSirenLPRSet ) // Added by Efi
				{
					capOrderArr[i++] = eSirenLPR_32kCapCode;
					isSirenLPRSet = eSirenLPR_32kCapCode;
				} Waiting to fix bug in HDX (if this change is made, HDX doesn't choose eSirenLPR_32kCapCode. Is chooses eSirenLPRStereo_128kCapCode instead) */

				if(isRemoveGenericAudioCaps == FALSE)
				{
					if(isRemoveG719 == FALSE && !isG719Set)
					{
						capOrderArr[i++] = eG719_32kCapCode;
						isG719Set = true;
					}
					capOrderArr[i++] = eSiren22_32kCapCode;
					if (isRemoveG7221C == FALSE)
					{
						capOrderArr[i++] = eG7221C_32kCapCode;
						capOrderArr[i++] = eG7221C_24kCapCode;
					}
					capOrderArr[i++] = eG7221_32kCapCode;
					capOrderArr[i++] = eG7221_24kCapCode;
					capOrderArr[i++] = eG7221_16kCapCode;
				}
				capOrderArr[i++] = eG728CapCode;
				if(isRemoveG729 == FALSE)
				{
					capOrderArr[i++] = eG729AnnexACapCode;
				}
				if(isRemoveG722 == FALSE)
		    		capOrderArr[i++] = eG722_64kCapCode;
		    	capOrderArr[i++] = eG711Ulaw64kCapCode;
		    	capOrderArr[i++] = eG711Alaw64kCapCode;
		    	if(isRemoveG723 == FALSE)
				{
	    			capOrderArr[i++] = eG7231CapCode;
				}
				break;

			case rate192K:
				/*if (!isSirenLPRSet ) // Added by Efi
				{
					capOrderArr[i++] = eSirenLPR_32kCapCode;
					isSirenLPRSet = eSirenLPR_32kCapCode;
				} Waiting to fix bug in HDX (if this change is made, HDX doesn't choose eSirenLPR_32kCapCode. Is chooses eSirenLPRStereo_128kCapCode instead) */

				if(isRemoveGenericAudioCaps == FALSE)
				{
					if(isRemoveG719 == FALSE && !isG719Set)
					{
						capOrderArr[i++] = eG719_32kCapCode;
						isG719Set = true;
					}
					capOrderArr[i++] = eSiren22_32kCapCode;
					if (isRemoveG7221C == FALSE)
					{
						capOrderArr[i++] = eG7221C_32kCapCode;
						capOrderArr[i++] = eG7221C_24kCapCode;
					}
					capOrderArr[i++] = eG7221_32kCapCode;
					capOrderArr[i++] = eG7221_24kCapCode;
					capOrderArr[i++] = eG7221_16kCapCode;
				}
				capOrderArr[i++]= eG728CapCode;
				if(isRemoveG722 == FALSE)
			    	capOrderArr[i++] = eG722_64kCapCode;
		    	capOrderArr[i++] = eG711Ulaw64kCapCode;
		    	capOrderArr[i++] = eG711Alaw64kCapCode;
		    	if(isRemoveG729 == FALSE)
				{
		    		capOrderArr[i++] = eG729AnnexACapCode;
				}
				if(isRemoveG723 == FALSE)
				{
	    			capOrderArr[i++] = eG7231CapCode;
				}
				break;
		}
	}

	else if (isRemoteMgcWithLowRateConf == FALSE)
	{
		if(isRemoveGenericAudioCaps == FALSE)
		{
			if(bitRateForAudioAlgsOrder >= rate1024K)
			{
				if(!isSirenLPRSet && !isRemoveSirenStereo)//BRIDGE-12398
				{
					capOrderArr[i++] = eSirenLPRStereo_128kCapCode;
					isSirenLPRSet = eSirenLPRStereo_128kCapCode;
				}
				if(isRemoveG719 == FALSE && !isG719StereoSet)
				{
					capOrderArr[i++] = eG719Stereo_128kCapCode;
					isG719StereoSet = true;
				}

				if(!isRemoveSirenStereo)//BRIDGE-12398
					capOrderArr[i++] = eSiren22Stereo_128kCapCode;
			}
			if(bitRateForAudioAlgsOrder >= rate512K)
			{
				if (!isSirenLPRSet && !isRemoveSirenStereo)//BRIDGE-12398
				{
					capOrderArr[i++] = eSirenLPRStereo_96kCapCode;
					isSirenLPRSet = eSirenLPRStereo_96kCapCode;
				}			
				if(isRemoveG719 == FALSE && !isG719StereoSet)
				{
					capOrderArr[i++] = eG719Stereo_96kCapCode;
					isG719StereoSet = true;
				}

				if(!isRemoveSirenStereo)//BRIDGE-12398
				{
					capOrderArr[i++] = eSiren22Stereo_96kCapCode;
					capOrderArr[i++] = eSiren14Stereo_96kCapCode;
				}
			}
			if(bitRateForAudioAlgsOrder >= rate384K)
			{
				if (!isSirenLPRSet && !isRemoveSirenStereo)//BRIDGE-12398
				{
					capOrderArr[i++] = eSirenLPRStereo_64kCapCode;
					isSirenLPRSet = eSirenLPRStereo_64kCapCode;
				}
				if(isRemoveG719 == FALSE && !isG719StereoSet)
				{
					capOrderArr[i++] = eG719Stereo_64kCapCode;
					isG719StereoSet = true;
				}

				if(!isRemoveSirenStereo)//BRIDGE-12398
				{
					capOrderArr[i++] = eSiren22Stereo_64kCapCode;
					capOrderArr[i++] = eSiren14Stereo_64kCapCode;
				}

				if(isRemoveG719 == FALSE && !isG719Set)
				{
					capOrderArr[i++] = eG719_64kCapCode;
					isG719Set = true;
				}
				capOrderArr[i++] = eSiren22_64kCapCode;
			}
			if(bitRateForAudioAlgsOrder >= rate256K)
			{
				if (!isSirenLPRSet ) {
					capOrderArr[i++] = eSirenLPR_64kCapCode;
					isSirenLPRSet = eSirenLPR_64kCapCode;
				}

				if(!isRemoveSirenStereo)//BRIDGE-12398
					capOrderArr[i++] = eSiren14Stereo_48kCapCode;

				if(isRemoveG719 == FALSE && !isG719Set)
				{
					capOrderArr[i++] = eG719_48kCapCode;
					isG719Set = true;
				}
				capOrderArr[i++] = eSiren22_48kCapCode;
				if (isRemoveG7221C == FALSE)
					capOrderArr[i++] = eG7221C_48kCapCode;
			}
			if(bitRateForAudioAlgsOrder >= rate128K)
			{
				if(isRemoveG719 == FALSE && !isG719Set)
				{
					capOrderArr[i++] = eG719_32kCapCode;
					isG719Set = true;
				}
				if (!isSirenLPRSet ) {
					capOrderArr[i++] = eSirenLPR_32kCapCode;
					isSirenLPRSet = eSirenLPR_32kCapCode;
				}
				capOrderArr[i++] = eSiren22_32kCapCode;
				if (isRemoveG7221C == FALSE)
					capOrderArr[i++] = eG7221C_32kCapCode;
			}
			if (isRemoveG7221C == FALSE)
				capOrderArr[i++] = eG7221C_24kCapCode;
		}
		if(isRemoveG722 == FALSE)
    		capOrderArr[i++] = eG722_64kCapCode;
		if(isRemoveGenericAudioCaps == FALSE)
		{
			capOrderArr[i++] = eG7221_32kCapCode;
			capOrderArr[i++] = eG7221_24kCapCode;
			capOrderArr[i++] = eG7221_16kCapCode;
		}
		capOrderArr[i++]= eG728CapCode;
    	capOrderArr[i++] = eG711Ulaw64kCapCode;
    	capOrderArr[i++] = eG711Alaw64kCapCode;
    	if(isRemoveG729 == FALSE)
		{
    		capOrderArr[i++] = eG729AnnexACapCode;
		}
		if(isRemoveG723 == FALSE)
		{
    		capOrderArr[i++] = eG7231CapCode;
		}
	}
	else if ( isRemoteMgcWithLowRateConf)
	{
		PTRACE(eLevelInfoNormal,"CCapH323::SetAudioAccordingToRate - link to MGC in small rates");
		capOrderArr[i++] = eG728CapCode;
		if(isRemoveG729 == FALSE)
		{
			capOrderArr[i++] = eG729AnnexACapCode;
		}
		if(isRemoveGenericAudioCaps == FALSE)
		{
			if(isRemoveG723 == FALSE)
			{
				capOrderArr[i++] = eG7231CapCode;
			}
			capOrderArr[i++] = eG7221_24kCapCode;
			capOrderArr[i++] = eG7221_32kCapCode;
		}
		if(isRemoveG722 == FALSE)
		    capOrderArr[i++] = eG722_64kCapCode;
		capOrderArr[i++] = eG711Ulaw64kCapCode;
		capOrderArr[i++] = eG711Alaw64kCapCode;
	}

	if(isRemoveG722)
		PTRACE(eLevelInfoNormal,"CCapH323::SetAudioAccordingToRate - Remote can't support G722. Codec has removed from the lis");
	if(isRemoveGenericAudioCaps)
		PTRACE(eLevelInfoNormal,"CCapH323::SetAudioAccordingToRate - Remote can't support Generic Audio. Codec's have removed from the list (Siren14 and G7221)");
	if (isRemoveG7221C)
		PTRACE(eLevelInfoNormal,"CCapH323::SetAudioAccordingToRate - Remote can't support G7221C. Codec's have removed from the list ");
	if(isRemoveG723)
		PTRACE(eLevelInfoNormal,"CCapH323::SetAudioAccordingToRate - Remote can't support G7223. RSS does not support this codec");
	if(isRemoveG729)
   		PTRACE(eLevelInfoNormal,"CCapH323::SetAudioAccordingToRate - Remote can't support G729.  RSS does not support this codec");

	for (int j=0; j<i; j++)
	{	//If it wasn't set before, set by capOrderArr order.
		if (OnCap(capOrderArr[j])==FALSE)
			numOfCaps += SetAudioCap(capOrderArr[j]);
	}

	return numOfCaps;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CCapH323::GetPreferedAudioRateAccordingToVideo(DWORD vidRate)
{
	DWORD audRate = 0;
	CCapSetInfo capInfo;
	switch(vidRate)
	{
		case rate64K:
		{
			capInfo = eG729AnnexACapCode;
			audRate = capInfo.GetBitRate();
			break;
		}
		case rate128K:
		{
			capInfo = eSiren14_32kCapCode;
			audRate = capInfo.GetBitRate();
			break;
		}
		case rate192K:
		{
			capInfo = eSiren14_32kCapCode;
			audRate = capInfo.GetBitRate();
			break;
		}
		default :
		{
			capInfo = eSiren14_48kCapCode;
			audRate = capInfo.GetBitRate();
			break;
		}
	}

	return 	(audRate);

}
/////////////////////////////////////////////////////////////////////////////
// Function name:	SetVideo
//
// Description:		Setting H323 Video Capabilities from H320 Capabilities object
//
// Return value:	Type [WORD]:  Number of added video caps
/////////////////////////////////////////////////////////////////////////////
/*WORD  CCapH323::SetVideo(const CCapH221& h221Cap, DWORD videoRate,DWORD confRate, const CComMode& scm)
{
	WORD numOfVidCap = 0;

    WORD videoAlg   = scm.GetVidMode();
	DWORD upperLimit264 = ::GetpSystemCfg()->GetH264UpperLimit() * 1000;

	BOOL bConfType = VSW;
	if(scm.m_vidMode.IsAutoVidScm())
		bConfType = VSW_AUTO;
	if(scm.IsFreeVideoRate()) // video card conference (CP\TR)
		bConfType = CP;

    BYTE bAutoResolution = scm.m_vidMode.IsAutoVidScm();
	BYTE bOnlyOneResolution = (bAutoResolution == FALSE);

	// set the order of the Video capability
	if(videoAlg == H261)
	{
		if(::IsH261Cap())
            numOfVidCap += SetH261Cap(h221Cap, videoRate, bOnlyOneResolution, bConfType);

		if(h221Cap.IsH263())
			numOfVidCap	+= SetH263Cap(h221Cap, videoRate, bConfType, scm);

		if(h221Cap.IsH26L())
			numOfVidCap	+= SetH26LCap(h221Cap, videoRate, bConfType);
	}
	else if (videoAlg == H263)
	{
		//In case we in CP auto we want 264 to be in the caps but in case the rate is higher than upperLimit264 can't be
		//the first one, so the scm is 263 and the first cap should be 263 but the second should be 264

		//if((videoProtocol == AUTO) && (confRate >= upperLimit264) && (bConfType == CP)) -->263 26L 261
		if(h221Cap.IsH263())
			numOfVidCap += SetH263Cap(h221Cap, videoRate, bConfType, scm);

		if(h221Cap.IsH26L())
			numOfVidCap	+= SetH26LCap(h221Cap, videoRate, bConfType);

		if(::IsH261Cap())
			numOfVidCap += SetH261Cap(h221Cap, videoRate, bOnlyOneResolution, bConfType);

		if(::GetpSystemCfg()->IsDBC2())
			    numOfVidCap += SetDBC2Cap();
	}
	else if (videoAlg == H264)
	{
		if((confRate > upperLimit264) && (bConfType == CP))
		{
			if(h221Cap.IsH263())
				numOfVidCap	+= SetH263Cap(h221Cap, videoRate, bConfType, scm);

			if(h221Cap.IsH264())
				numOfVidCap += SetH264Cap(h221Cap, videoRate, bConfType);

			if(::IsH261Cap())
				numOfVidCap += SetH261Cap(h221Cap, videoRate, bOnlyOneResolution, bConfType);
		}
		else //confBitRate <= upperLimit264
		{
			if(h221Cap.IsH264())
				numOfVidCap += SetH264Cap(h221Cap, videoRate, bConfType);

			if(h221Cap.IsH263())
				numOfVidCap	+= SetH263Cap(h221Cap, videoRate, bConfType, scm);

			if(::IsH261Cap())
				numOfVidCap += SetH261Cap(h221Cap, videoRate, bOnlyOneResolution, bConfType);
		}
	}
	else if (videoAlg == H26L)
	{
		if(h221Cap.IsH26L())
			numOfVidCap	+= SetH26LCap(h221Cap, videoRate, bConfType);

		if(h221Cap.IsH263())
			numOfVidCap += SetH263Cap(h221Cap, videoRate, bConfType, scm);

		if(::IsH261Cap())
			numOfVidCap += SetH261Cap(h221Cap, videoRate, bOnlyOneResolution,bConfType);
	}
	else // if (videoAlg = Video_Off)
	{
		PTRACE(eLevelInfoNormal,"CCapH323::SetVideo: video is off");
	}

	// atara: adding a role label cap at the end of video caps for EPC call
	if (numOfVidCap && h221Cap.IsEnterprisePeopleContent())
		numOfVidCap += SetRoleLabelCapCode(LABEL_PEOPLE);

	if (numOfVidCap && h221Cap.IsFieldDrop())
		numOfVidCap	+= SetDropFieldCap(&h221Cap);

	return numOfVidCap;
}*/


/////////////////////////////////////////////////////////////////////////////
// Function name:	SetContent
// Description:		Setting H323 content explicit capabilities for version 1
// Return value:	Type [WORD]:  Number of added content caps
/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetContent(const CComModeH323* pScm, BYTE isRemoveH239, BYTE isFixContentProtocol,BYTE isRemoveEPC/*,BYTE bContetnAsVideo*/)
{
	WORD numOfCaps = 0;
	// PTRACE(eLevelInfoNormal,"CCapH323::SetContent");

	BOOL bSupportEPC = GetSystemCfgFlagInt<BOOL>("ENABLE_EPC");
	 // Content protocol h264 only
	BOOL bForceH264 = 0;
	BYTE bIsHighProfileContent = FALSE;

	if (pScm->IsTIPContentEnableInH264Scm())
		isRemoveEPC = TRUE;
	if (pScm->IsContent(cmCapReceiveAndTransmit) && isRemoveH239 == FALSE )
	{
		DWORD contentRate   = pScm->GetContentBitRate(cmCapReceive); //in 100 bit per second
		//1) H239
		ePresentationProtocol contentProtocolMode = pScm->GetContentProtocolMode();
		if(contentProtocolMode == eH264Fix || contentProtocolMode == eH264Dynamic)
		{
			ON(bForceH264);
			OFF(isFixContentProtocol);
		}
		CCapSetInfo capInfo = (CapEnum)pScm->GetMediaType(cmCapVideo, cmCapReceive, kRolePresentation);
		BYTE isSetH264Content = FALSE;
		BYTE isSetH263Content = FALSE;

		if(capInfo.GetIpCapCode() == eH264CapCode)
		{
			if(contentProtocolMode == ePresentationAuto || bForceH264)
			{
				if(isFixContentProtocol)
				{
					isSetH263Content = TRUE;
				}
				else
				{
					isSetH263Content = TRUE;
					if(bForceH264)
					{
						PTRACE(eLevelInfoNormal,"CCapH323::SetContent:remove H263 CONTETN this is a fixed H264 content conf! ");
						isSetH263Content = FALSE;
					}
					isSetH264Content = TRUE;
				}
			}
		}

		if(capInfo.GetIpCapCode() == eH263CapCode)
		{
			if(contentProtocolMode == ePresentationAuto || bForceH264)
			{
				isSetH263Content = TRUE;
			}
			if(contentProtocolMode == eH263Fix)
				isSetH263Content = TRUE;
			// if(contentProtocol == eH264Fix) its an error, don't set content protocols
		}

		if((isSetH264Content == FALSE) && (isSetH263Content == FALSE))
			PTRACE(eLevelInfoNormal,"CCapH323::SetContent: No content protocls to be set!");

		if (pScm->IsTIPContentEnableInH264Scm() == TRUE)
		{
			isSetH263Content = FALSE;
			//bSupportEPC= FALSE;
			//RemoveProtocolFromCapSet(eRoleLabelCapCode);
		}

		if(isSetH264Content)
		{
			CBaseVideoCap* pContentCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(eH264CapCode,NULL);
			if (pContentCap)
			{
				EResult eResOfSet = kSuccess;

				if ( pScm->IsTIPContentEnableInH264Scm() == TRUE) //just for TipCompatibility:video&content!
				{
					eResOfSet &= ((CH264VideoCap *)pContentCap)->SetTIPContent(kRolePresentation, cmCapReceive);
				}
				else if ( pScm->GetTipContentMode() == eTipCompatiblePreferTIP)
				{
					eResOfSet &= ((CH264VideoCap *)pContentCap)->SetTIPContent(kRolePresentation, cmCapReceive, FALSE);
				}
				else
				{
					BOOL bContentHD1080Enabled = FALSE;
					BYTE HDResMpi = 0;
					BYTE HDResMpiRcv = 0;
					BYTE HDResMpiTx = 0;
					bIsHighProfileContent = pScm->IsH264HighProfileContent(cmCapReceive);
					HDResMpiRcv = pScm->isHDContent1080Supported(cmCapReceive);
					HDResMpiTx  = pScm->isHDContent1080Supported(cmCapTransmit);

					if(HDResMpiRcv && HDResMpiTx)
					{
						bContentHD1080Enabled = TRUE;
						HDResMpi = HDResMpiRcv;
					}
					else
					{
						HDResMpiRcv = pScm->isHDContent720Supported(cmCapReceive);
						HDResMpiTx  = pScm->isHDContent720Supported(cmCapTransmit);
						if(HDResMpiRcv && HDResMpiTx)
						{
							HDResMpi = HDResMpiRcv;
						}
					}
					eResOfSet &= pContentCap->SetContent(kRolePresentation, cmCapReceive, bContentHD1080Enabled, HDResMpi, bIsHighProfileContent);
				}

				eResOfSet &= pContentCap->SetBitRate(contentRate);
	//			PTRACE2INT(eLevelInfoNormal," CCapH323::SetContent :  Content rate  - ",contentRate);

				if (eResOfSet && (contentRate > 0))// if content rate is equal to zero we should not declare on content at all.
				{
					int structSize = pContentCap->SizeOf();
					pContentCap->SetAdditionalXmlInfo();
					AddCapToCapBuffer(eH264CapCode,structSize, pContentCap->GetStruct());
					m_capArray[eH264CapCode]++;
					m_numOfCaps++;
					numOfCaps++;

					//HP content: Add baseline profile cap if needed:
					if (bIsHighProfileContent && contentProtocolMode != eH264Fix)
					{
						CBaseVideoCap* pContentCapBaseline = (CBaseVideoCap *)CBaseCap::AllocNewCap(eH264CapCode,NULL);
						if (pContentCapBaseline)
						{
							eCascadeOptimizeResolutionEnum eMaxResolution = e_res_dummy;
							BYTE HDMpi = 0;
							BYTE bPartySupportsContentHD1080 = TRUE;
							PTRACE2INT(eLevelInfoNormal,"CCapH323::SetContent :  Content rate: ", contentRate);
							eMaxResolution = (eCascadeOptimizeResolutionEnum)CUnifiedComMode::getMaxContentResolutionbyRateAndProfile(contentRate*100, FALSE);
							PTRACE2INT(eLevelInfoNormal,"CCapH323::SetContent :  Content eMaxResolution: ", eMaxResolution);

							if (eMaxResolution < e_res_1080_15fps)
							{
								OFF(bPartySupportsContentHD1080);
							}

							switch(eMaxResolution)
							 {
							 case e_res_720_5fps:
								 HDMpi = 10;
								 break;
							 case e_res_720_30fps:
								 HDMpi = 2;
								 break;
							 case e_res_1080_15fps:
								 HDMpi = 4;
								 break;
							 case e_res_1080_30fps:
								 HDMpi = 2;
								 break;
							 case e_res_1080_60fps:
								 HDMpi = 1;
								 break;
							 default:
								 PASSERT(eMaxResolution + 1);
								 HDMpi = 10;
							 }

							if (bPartySupportsContentHD1080 && HDMpi < 4) // 1080p30/60
							{
								if(!IsFeatureSupportedBySystem(eFeatureHD1080p30Content))
									HDMpi = 4;
								else if(HDMpi == 1 && !IsFeatureSupportedBySystem(eFeatureHD1080p60Content)) 
									HDMpi = 2;
							}

							PTRACE2INT(eLevelInfoNormal,"CCapH323::SetContent :  Content basline profile MPI: ",HDMpi);
							eResOfSet &= pContentCapBaseline->SetContent(kRolePresentation, cmCapReceive, bPartySupportsContentHD1080, HDMpi, FALSE);
							eResOfSet &= pContentCapBaseline->SetBitRate(contentRate);

							if (eResOfSet)
							{
								PTRACE(eLevelInfoNormal,"CCapH323::SetContent : Add baseline content cap");
								pContentCapBaseline->SetAdditionalXmlInfo();
								AddCapToCapBuffer(eH264CapCode,pContentCapBaseline->SizeOf(), pContentCapBaseline->GetStruct());
								m_capArray[eH264CapCode]++;
								m_numOfCaps++;
								numOfCaps++;
							}

							pContentCapBaseline->FreeStruct();
							POBJDELETE(pContentCapBaseline);
						}
					}
					
					if(bForceH264)
						numOfCaps += SetRoleLabelCapCode(LABEL_CONTENT);

				}
				else
					PTRACE(eLevelInfoNormal,"CCapH323::SetContent: Set struct has failed!!!");

				pContentCap->FreeStruct();
				POBJDELETE(pContentCap);
			}
			else PTRACE(eLevelInfoNormal,"CCapH323::SetContent - pContentCap is NULL");
		}

		if(isSetH263Content)
		{// Add H263 content cap
			CBaseVideoCap* pH263ContentCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(eH263CapCode,NULL);
			if (pH263ContentCap)
			{
				EResult eH263ResOfSet = kSuccess;
				eH263ResOfSet &= pH263ContentCap->SetContent(kRolePresentation, cmCapReceive/*,bContetnAsVideo*/);
				eH263ResOfSet &= pH263ContentCap->SetBitRate(contentRate);
				/*if(bContetnAsVideo)
				{
					PTRACE(eLevelInfoNormal,"CCapH323::SetContent: remove annex T - content as video enabled!!");
					((CH263VideoCap*)pH263ContentCap)->RemoveAnAnnexFromMask(typeAnnexT);
				}
				*/
	//			PTRACE2INT(eLevelInfoNormal," CCapH323::SetContent :  Content rate  - ",contentRate);

				if (eH263ResOfSet && (contentRate > 0))// if content rate is equal to zero we should not declare on content at all.
				{
					int structSize = pH263ContentCap->SizeOf();
					pH263ContentCap->SetAdditionalXmlInfo();
					AddCapToCapBuffer(eH263CapCode,structSize, pH263ContentCap->GetStruct());
					m_capArray[eH263CapCode]++;
					m_numOfCaps++;
					numOfCaps++;
				}
				else
					PTRACE(eLevelInfoNormal,"CCapH323::SetContent: Set struct has failed!!!");

				pH263ContentCap->FreeStruct();
				POBJDELETE(pH263ContentCap);
			}
			else PTRACE(eLevelInfoNormal,"CCapH323::SetContent - pH263ContentCap is NULL");
		}

		//2) EPC
		if((bSupportEPC)&& (isRemoveEPC == FALSE) && (!bForceH264 || contentProtocolMode == eH263Fix) )
		{
			CCapSetInfo epcCapInfo = eH263CapCode;
			CH263VideoCap* pEpcContentCap = (CH263VideoCap *)CBaseCap::AllocNewCap((CapEnum)epcCapInfo,NULL);
			if (pEpcContentCap)
			{
				EResult eResOfSet = kSuccess;
				eResOfSet &= pEpcContentCap->SetContent(kRoleContent, cmCapReceive);
				eResOfSet &= pEpcContentCap->SetBitRate(contentRate);

				if (eResOfSet)
				{
					int structSize = pEpcContentCap->SizeOf();
					pEpcContentCap->SetAdditionalXmlInfo();
					AddCapToCapBuffer((CapEnum)epcCapInfo,structSize,pEpcContentCap->GetStruct());
					m_capArray[(CapEnum)epcCapInfo]++;
					m_numOfCaps++;
					numOfCaps++;
					numOfCaps += SetRoleLabelCapCode(LABEL_CONTENT);
					PTRACE(eLevelInfoNormal,"CCapH323::SetContent: Add lable content and H263 capcode!");
				}
				else
					PTRACE(eLevelInfoNormal,"CCapH323::SetContent: Set struct has failed!!!");

				pEpcContentCap->FreeStruct();
				POBJDELETE(pEpcContentCap);
			}
		}
	}
	//In h239 we don't have RoleLabelCapCode od content at the end of content caps
    if (0 == numOfCaps)
    {
        pScm->Dump("CCapH323::SetContent: -num of contet caps is ZERO!", eLevelError);
    }


	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::SetRoleLabelCapCode(BYTE label)
{
	WORD numOfCaps = 0;

	CBaseAudioCap* pLabelCap = (CBaseAudioCap *)CBaseCap::AllocNewCap(eRoleLabelCapCode, NULL);
	if (pLabelCap)
	{
		EResult eRes = pLabelCap->SetRoleLabelStruct(label);
		if (eRes)
		{
			int structSize = pLabelCap->SizeOf();
			BaseCapStruct* pStruct = pLabelCap->GetStruct();
			AddCapToCapBuffer(eRoleLabelCapCode,structSize,pStruct);
			m_capArray[eRoleLabelCapCode]++;
			numOfCaps = 1;
			m_numOfCaps++;
		}
		pLabelCap->FreeStruct();
		POBJDELETE(pLabelCap);
	}

	return numOfCaps;
}


/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetFecc(const CComModeH323* pScm, BYTE isRemoveAnnexQ, BYTE isRemoveRvFecc, DWORD serviceId)
{
	WORD numOfCaps = 0;

	if (pScm->IsMediaOff(cmCapData, cmCapTransmit))
		return 0;

	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
//	CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
	BOOL bRvFecc, bAnnexQFecc;
	bRvFecc = bAnnexQFecc = 0;
	pSysConfig->GetBOOLDataByKey("FECC_RV", bRvFecc);
	pSysConfig->GetBOOLDataByKey("FECC_ANNEXQ", bAnnexQFecc);
//	pServiceSysCfg->GetBOOLDataByKey(serviceId, "FECC_RV", bRvFecc);
//	pServiceSysCfg->GetBOOLDataByKey(serviceId, "FECC_ANNEXQ", bAnnexQFecc);

	if (bAnnexQFecc)
	{
		if(isRemoveAnnexQ == FALSE)
			numOfCaps += SetFeccCap(pScm, eAnnexQCapCode);
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetFecc - Remove AnnexQ. EP doesn't support this cap");
	}
	if (bRvFecc)
	{
		if(isRemoveRvFecc == FALSE)
			numOfCaps += SetFeccCap(pScm, eRvFeccCapCode);
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetFecc - Remove RvFecc. EP doesn't support this cap");
	}

	return numOfCaps;
}


/////////////////////////////////////////////////////////////////////////////
// Function name:	SetPeopleContent
//
// Description:		Setting H323 People&Content Version 0/1 Capabilities from
//					H320 Capabilities object
//
// Return value:	Type [WORD]:  Number of added People&Content caps
/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetPeopleContent(const CComModeH323* pScm, BYTE isRemoveContent,int version, int profile)
{
	WORD numOfCaps = 0;
	BOOL bSupportEPC = GetSystemCfgFlagInt<BOOL>("ENABLE_EPC");
	if(bSupportEPC)
	{
		if (pScm->IsContent(cmCapReceiveAndTransmit) && (isRemoveContent == FALSE))
		{
		    if ((version == 1)&&(profile == 2))
			{
				PTRACE(eLevelInfoNormal,"CCapH323::SetPeopleContent - add ");
				// The PeopleAndContentCapStruct is the same as audioCapStructBase therefore we use
				// CBaseAudioCap to initialize this struct.
				// the caph323 will hold the maximum content rate that the mcu supports
				m_maxContRate = pScm->GetContentBitRate(cmCapReceive); //in 100 bit per second
				if(m_maxContRate > 0)
					numOfCaps += SetContentProfile(profile);
			}
		}
	}
	return numOfCaps;
}

//////////////////////////////////////////////////////////////////////////////
WORD CCapH323::SetContentProfile(int profile,cmCapDirection eDirection)
{
	WORD numOfCaps = 0;
	CBaseAudioCap* pPeopleContentCap = (CBaseAudioCap *)CBaseCap::AllocNewCap(ePeopleContentCapCode,NULL);
	if (pPeopleContentCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet &= pPeopleContentCap->SetContentProfile(profile,cmCapReceive);
		if (eResOfSet)
		{
			int structSize = pPeopleContentCap->SizeOf();
			AddCapToCapBuffer(ePeopleContentCapCode,structSize, pPeopleContentCap->GetStruct());
			m_capArray[ePeopleContentCapCode]++;
			numOfCaps = 1;
			m_numOfCaps++;

			if(profile == 2)
			{
				m_bIsEPC = TRUE;
			}
			else
				m_bIsPCversion0 = TRUE;
		}
		pPeopleContentCap->FreeStruct();
		POBJDELETE(pPeopleContentCap);
	}
	else PTRACE(eLevelInfoNormal,"CCapH323::SetContentProfile - eResOfSet is NULL");

	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetH239Control(const CComModeH323* pScm, BYTE isRemoveH239)
{
	WORD numOfCaps = 0;

    if (pScm->IsContent(cmCapReceiveAndTransmit) && isRemoveH239 == FALSE )
	{
		// the caph323 will hold the maximum content rate that the mcu supports
		m_maxContRate = pScm->GetContentBitRate(cmCapReceive); //in 100 bit per second
		if(m_maxContRate > 0)
			numOfCaps += SetH239ControlCap();
	}
	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetH239ControlCap(cmCapDirection eDirection)
{
	WORD numOfCaps = 0;
	CBaseAudioCap* pH239ControlCap = (CBaseAudioCap *)CBaseCap::AllocNewCap(eH239ControlCapCode, NULL);
	if (pH239ControlCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet &= pH239ControlCap->SetH239ControlCap(cmCapReceive);
		if (eResOfSet)
		{
			int structSize = pH239ControlCap->SizeOf();
			AddCapToCapBuffer(eH239ControlCapCode,structSize, pH239ControlCap->GetStruct());
			m_capArray[eH239ControlCapCode]++;
			numOfCaps = 1;
			m_numOfCaps++;

			m_bIsH239 = TRUE;
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetH239ControlCap: Set struct has failed!!!");

		pH239ControlCap->FreeStruct();
		POBJDELETE(pH239ControlCap);
	}

	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetDynamicPTRepControl(const CComModeH323* pScm, BYTE isRemoveDPTR)
{
	WORD numOfCaps = 0;
	//PTRACE2INT(eLevelInfoNormal,"CCapH323::SetDynamicPTRepControl",isRemoveDPTR);
    if (!isRemoveDPTR )
	{
			numOfCaps += SetDynamicPTRepControlCap();
	}

	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetDynamicPTRepControlCap(cmCapDirection eDirection)
{
	WORD numOfCaps = 0;
	CBaseAudioCap* pDynPTRepControlCap = (CBaseAudioCap *)CBaseCap::AllocNewCap(eDynamicPTRCapCode, NULL);
	if (pDynPTRepControlCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet &= pDynPTRepControlCap->SetDynamicPTRepControl(cmCapReceive); //?why trnasmit
		if (eResOfSet)
		{
			int structSize = pDynPTRepControlCap->SizeOf();
			AddCapToCapBuffer(eDynamicPTRCapCode,structSize, pDynPTRepControlCap->GetStruct());
			m_capArray[eDynamicPTRCapCode]++;
			numOfCaps = 1;
			m_numOfCaps++;

			m_bIsDPTR = TRUE;
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetDynamicPTRepControlCap: Set struct has failed!!!");
		pDynPTRepControlCap->FreeStruct();
		POBJDELETE(pDynPTRepControlCap);
	}

	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetLpr(const CComModeH323* pScm, BYTE isRemoveLpr)
{
	WORD numOfCaps = 0;

    if (pScm->GetIsLpr() && (isRemoveLpr == FALSE))
	{
		// the caph323 will hold the maximum content rate that the mcu supports
		numOfCaps += SetLprControlCap();
	}
	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetLprControlCap(cmCapDirection eDirection)
{
	WORD numOfCaps = 0;
	CLprCap* pLprCap = (CLprCap *)CBaseCap::AllocNewCap(eLPRCapCode, NULL);
	if (pLprCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet &= pLprCap->SetDefaults(cmCapReceive);
		if (eResOfSet)
		{
			int structSize = pLprCap->SizeOf();
			AddCapToCapBuffer(eLPRCapCode,structSize, pLprCap->GetStruct());
			m_capArray[eLPRCapCode]++;
			numOfCaps = 1;
			m_numOfCaps++;

			m_bIsLpr = TRUE;
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetLprControlCap: Set struct has failed!!!");

		pLprCap->FreeStruct();
		POBJDELETE(pLprCap);
	}
	else PTRACE(eLevelInfoNormal,"CCapH323::SetLprControlCap - eResOfSet is NULL");

	return numOfCaps;
}
/////////////////////////////////////////////////////////////////////////////
WORD CCapH323::SetDropFieldCap()
{
	WORD numOfCaps = 0;
	CGenericVideoCap* pGenericVideoDropField = (CGenericVideoCap *)CBaseCap::AllocNewCap(eGenericVideoCapCode, NULL);
	if (pGenericVideoDropField)
	{
		EResult eRes = pGenericVideoDropField->SetDefaults();
		eRes = pGenericVideoDropField->SetDropFieldCap();

		if (eRes)
		{
			int structSize = pGenericVideoDropField->SizeOf();
			BaseCapStruct* pStruct = pGenericVideoDropField->GetStruct();
			AddCapToCapBuffer(eGenericVideoCapCode, structSize, pStruct);
			m_capArray[eGenericVideoCapCode]++;
			numOfCaps++;
			m_numOfCaps++;
		}
		pGenericVideoDropField->FreeStruct();
		POBJDELETE(pGenericVideoDropField);
	}

	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetNonStandardAnnex(annexesListEn annex)
{
	WORD numOfCaps = 0;
	if (annex == typeAnnexI_NS) //right now we support only this one
	{
		CNonStandardCap* pNonStandardCap = (CNonStandardCap *)CBaseCap::AllocNewCap(eNonStandardCapCode,NULL);
		if (pNonStandardCap)
		{
			EResult eResOfSet = kSuccess;
			eResOfSet &= pNonStandardCap->SetAnnexI_NsStruct();

			if(eResOfSet)
			{
				int structSize = pNonStandardCap->SizeOf();
				AddCapToCapBuffer(eNonStandardCapCode,structSize, pNonStandardCap->GetStruct());
				m_capArray[eNonStandardCapCode]++;
				numOfCaps++;
				m_numOfCaps++;
			}
			else
				PTRACE(eLevelInfoNormal,"CCapH323::SetNonStandardAnnex: Set struct has failed!!!");

			pNonStandardCap->FreeStruct();
			POBJDELETE(pNonStandardCap);
		}
	}

	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////
// Function name:	SetEncryption
//
// Description:		Add encryption to the capabilities table.
//
// Return value:	Type [WORD]:  Number of added encryption caps
/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetEncryption()
{
    PTRACE (eLevelInfoNormal, "CCapH323::SetEncryption");
	APIU16 entry			= 1;
	capBuffer *pCapBuffer	= (capBuffer *) &m_pCap->caps;
    char      *pTempPtr		= (char*)pCapBuffer;
	WORD	  numOfCaps		= m_numOfCaps;
	WORD	  numOfEncCaps	= 0;

	for(WORD i=0; i < numOfCaps; i++)
	{
		if ((pCapBuffer->capTypeCode != ePeopleContentCapCode) && (pCapBuffer->capTypeCode != eRoleLabelCapCode)
		&& (pCapBuffer->capTypeCode != eH239ControlCapCode) && (pCapBuffer->capTypeCode != eDBC2CapCode)
		&& (pCapBuffer->capTypeCode != eLPRCapCode) && (pCapBuffer->capTypeCode != eDynamicPTRCapCode))
		{
			BaseCapStruct* pCapBaseStruct = (BaseCapStruct*)pCapBuffer->dataCap;
			CEncryptionCap* pEncryCap = (CEncryptionCap *)CBaseCap::AllocNewCap(eEncryptionCapCode,NULL);
			if (pEncryCap)
			{
				EResult eResOfSet = kSuccess;

				eResOfSet		&= pEncryCap->SetStruct(m_encAlg,entry);
				if(pCapBaseStruct->header.direction == cmCapTransmit)
			    {
					//PTRACE(eLevelInfoNormal,"CCapH323::SetEncryption: noa temp");
					pEncryCap->SetDirection(cmCapTransmit);
					pEncryCap->GetStruct()->header.direction = cmCapTransmit;

				}
				int structSize	= pEncryCap->SizeOf();


				if(eResOfSet && structSize)
				{
					AddCapToCapBuffer(eEncryptionCapCode,structSize, pEncryCap->GetStruct());
					m_capArray[eEncryptionCapCode]++;
					numOfEncCaps++;
					m_numOfCaps++;
				}
				else
					PTRACE(eLevelInfoNormal,"CCapH323::SetEncryption: Set struct has failed");

				pEncryCap->FreeStruct();
				POBJDELETE(pEncryCap);
			}

		}

		pTempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
		entry++;
	}

	return numOfEncCaps;
}

/////////////////////////////////////////////////////////////////////////////
// Function name:	EndOfCapsConstruction
//
// Description:		Setting following H323 Capabilities Parameters:
//						1. Number Of Alts
//						2. Number Of Sims
//						3. Cap Matrix (m_pCap->altMatrix)
//						4. Cap Array  (m_capArray)
//This function is for local capabilities ONLY!!!!
// Return value:	Type [void]:
/////////////////////////////////////////////////////////////////////////////
void  CCapH323::EndOfCapsConstruction(WORD numOfAudioCap,
									  WORD numOfVidCap,
									  WORD numOfContentCap,
									  WORD numOfDuoCap,
									  WORD numOfT120Cap,
									  WORD numOfFeccCap,
									  WORD numOfPeopContCap,
									  WORD numOfH239ControlCap,
									  WORD numOfEncCap,
									  WORD numOfNsCap,
									  WORD numOfLprCap,
									  WORD numOfDptrCap)
{
	m_pCap->numberOfAlts		= 0; //alt's in each Sim.
	m_pCap->numberOfSim			= 0; //Sim in the Desc.

	m_pCap->numberOfCaps	= m_numOfCaps;
	m_numOfAudioCap			= numOfAudioCap;
	m_numOfVideoCap			= numOfVidCap;
	m_numOfContentCap       = numOfContentCap;
	m_numOfDuoCap			= numOfDuoCap;
	m_numOfT120Cap			= numOfT120Cap;
	m_numOfFeccCap			= numOfFeccCap;
	m_numOfPeopContCap		= numOfPeopContCap;
	m_numOfH239ControlCap   = numOfH239ControlCap;
	m_numOfNsCap			= numOfNsCap;
	m_numOfLprCap			= numOfLprCap;
	m_numOfEncrypCap		= numOfEncCap;
	m_numOfDynamicPTRControlCap = numOfDptrCap;

	// calculate number of Alt's in each Sim
	if(numOfAudioCap)
		m_pCap->numberOfAlts++;
	if(numOfVidCap)
		m_pCap->numberOfAlts++;
	if (numOfContentCap)
		m_pCap->numberOfAlts++;
	if (numOfDuoCap)
		m_pCap->numberOfAlts++;
	if((numOfT120Cap) || (numOfFeccCap))
		m_pCap->numberOfAlts++;
    if(numOfPeopContCap)
        m_pCap->numberOfAlts++;
    if(numOfH239ControlCap)
        m_pCap->numberOfAlts++;
	if(numOfNsCap)
		m_pCap->numberOfAlts++;
	if (numOfLprCap)
		m_pCap->numberOfAlts++;
	if (m_numOfDynamicPTRControlCap)
		m_pCap->numberOfAlts++;

	// number of Sim's in the Desc is now always = 1 !!!
    m_pCap->numberOfSim  = 1;

	BuildCapMatrixFromSortedCapBuf();
}

/////////////////////////////////////////////////////////////////////////////
// Function name:  BuildCapMatrixFromSortedCapBuf           written by: atara
// Variables:      Non.
// Description:	   Build cap matrix from a sorted cap buffer. In case the caps are encrypted we need to build the matrix
//				   only with the encrypted caps.
// Return value:   Non.
/////////////////////////////////////////////////////////////////////////////
void CCapH323::BuildCapMatrixFromSortedCapBuf()
{
	//------------------------  START OF CAP-MATRIX CONSTRUCTION  ----------------------------//
	// set the matrix to zero and then refill it with the new cap
//	CAP_FD_ZERO(&(m_pCap->altMatrix));
	for (int k = 0; k < FD_SET_SIZE; k++)
		m_pCap->altMatrix.fds_bits[k] = 0;

	int currentAltNumber	= 1;
	int currentBitToBeSet	= 0;
	int currentEncrToBeSet	= 0;
	int i					= 0;

	capBuffer *pCapBuffer	= (capBuffer *) &m_pCap->caps;
    char      *pTempPtr		= (char*)pCapBuffer;

	if(m_numOfEncrypCap)
		currentEncrToBeSet = m_numOfCaps - m_numOfEncrypCap;

	if(m_numOfAudioCap)
	{
		int index;
		
		// set audio alt's bits in cap-matrix
		for( i = currentBitToBeSet; i< m_numOfAudioCap; i++ )
		{
			index = i + currentEncrToBeSet;

			if (index >= 0 && index < 64 * 32)
			{
				CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));
			}
			else
			{
				PASSERT(index);
			}
			pTempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)pTempPtr;
		}

		currentBitToBeSet = currentAltNumber * m_numOfCaps + m_numOfAudioCap;
		currentAltNumber++;
	}

	if(m_numOfVideoCap)
	{
		// set video alt's bits in cap-matrix
		//We shouldn't encrypted PeopleContentCapCode and RoleLabelCapCode
		for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfVideoCap); i++ )
		{
			if ((pCapBuffer->capTypeCode != ePeopleContentCapCode) && (pCapBuffer->capTypeCode != eRoleLabelCapCode)
					&& (pCapBuffer->capTypeCode != eDBC2CapCode))
			{
				if((i+currentEncrToBeSet)/CAP_NFDBITS < FD_SET_SIZE)	
					CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));
				else
					PASSERTMSG(1, "fds_bits[] index is exceeded"); 
			}
			else
			{
				if(i/CAP_NFDBITS < FD_SET_SIZE)
					CAP_FD_SET(i,&(m_pCap->altMatrix));
				else
					PASSERTMSG(1, "fds_bits[] index is exceeded"); 
				
				if(currentEncrToBeSet)
					currentEncrToBeSet--;
			}

			pTempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)pTempPtr;
		}

		currentBitToBeSet = currentAltNumber * m_numOfCaps + m_numOfAudioCap + m_numOfVideoCap;
		currentAltNumber++;
	}

	if (m_numOfContentCap)
	{
		// set content alt's bits in cap-matrix
		//We shouldn't encrypted PeopleContentCapCode and RoleLabelCapCode
		for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfContentCap); i++ )
		{
			if ((pCapBuffer->capTypeCode != ePeopleContentCapCode) && (pCapBuffer->capTypeCode != eRoleLabelCapCode))
				CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));
			else
			{
				CAP_FD_SET(i,&(m_pCap->altMatrix));
				if(currentEncrToBeSet)
					currentEncrToBeSet--;
			}
			pTempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)pTempPtr;
		}

		currentBitToBeSet = currentAltNumber * m_numOfCaps + m_numOfAudioCap + m_numOfVideoCap + m_numOfContentCap;
		currentAltNumber++;
	}

	if (m_numOfDuoCap)
	{
		// set content alt's bits in cap-matrix
		for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfDuoCap); i++ )
			CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));

		currentBitToBeSet = currentAltNumber * m_numOfCaps + m_numOfAudioCap + m_numOfVideoCap + m_numOfContentCap + m_numOfDuoCap;
		currentAltNumber++;
	}

	if(m_numOfT120Cap)  //There is no encryption for T120 caps!!!
	{
		// set T120 alt's bits in cap-matrix
		for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfT120Cap); i++ )
		{
			CAP_FD_SET(i,&(m_pCap->altMatrix));
			if(currentEncrToBeSet)
				currentEncrToBeSet--;
		}

		currentBitToBeSet = currentAltNumber * m_numOfCaps
			                + m_numOfAudioCap
							+ m_numOfVideoCap
							+ m_numOfContentCap
							+ m_numOfDuoCap
							+ m_numOfT120Cap;
		currentAltNumber++;
	}

	if (m_numOfFeccCap)
	{
		// set content alt's bits in cap-matrix
		for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfFeccCap); i++ )
			CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));

		currentBitToBeSet = currentAltNumber * m_numOfCaps
							+ m_numOfAudioCap
							+ m_numOfVideoCap
							+ m_numOfContentCap
							+ m_numOfDuoCap
							+ m_numOfT120Cap
							+ m_numOfFeccCap;
		currentAltNumber++;
	}

    // PeopleContent cap section start
    if(m_numOfPeopContCap)
    {
        // set PeopleContent alt's bits in cap-matrix
        for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfPeopContCap); i++ )
		{
			CAP_FD_SET(i,&(m_pCap->altMatrix));
			if(currentEncrToBeSet)
				currentEncrToBeSet--;
		}

		currentBitToBeSet	= currentAltNumber * m_numOfCaps
							+ m_numOfAudioCap
							+ m_numOfVideoCap
							+ m_numOfContentCap
							+ m_numOfDuoCap
							+ m_numOfT120Cap
							+ m_numOfFeccCap
							+ m_numOfPeopContCap;

		currentAltNumber++;
	}
	// PeopleContent cap section  end

    if (m_numOfH239ControlCap)
    {
        // set h239ControlCap alt's bits in cap-matrix
        for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfH239ControlCap); i++ )
 		{
            //CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));
			//07.09.2006 Changes by VK. There is no encryption on H239 control cap, so mark its original bit.
			CAP_FD_SET(i,&(m_pCap->altMatrix));
			if (currentEncrToBeSet)
				currentEncrToBeSet--;
		}

		currentBitToBeSet	= currentAltNumber * m_numOfCaps
							+ m_numOfAudioCap
							+ m_numOfVideoCap
							+ m_numOfContentCap
							+ m_numOfDuoCap
							+ m_numOfT120Cap
							+ m_numOfFeccCap
							+ m_numOfPeopContCap
							+ m_numOfH239ControlCap;

		currentAltNumber++;
	}



	// NonStandard (VisualConcert) cap section start
	if(m_numOfNsCap)
	{
		// set NonStandard alt's bits in cap-matrix
		for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfNsCap); i++ )
			CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));

		currentBitToBeSet	= currentAltNumber * m_numOfCaps
							+ m_numOfAudioCap
							+ m_numOfVideoCap
							+ m_numOfContentCap
							+ m_numOfDuoCap
							+ m_numOfT120Cap
							+ m_numOfFeccCap
							+ m_numOfPeopContCap
							+ m_numOfH239ControlCap
							+ m_numOfNsCap;

		currentAltNumber++;
	}
	// NonStandard (VisualConcert) cap section  end

	// LPR Capabilities start
   if (m_numOfLprCap)
    {
        // set h239ControlCap alt's bits in cap-matrix
        for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfLprCap); i++ )
 		{
            //CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));
			//07.09.2006 Changes by VK. There is no encryption on H239 control cap, so mark its original bit.
			CAP_FD_SET(i,&(m_pCap->altMatrix));
			if (currentEncrToBeSet)
				currentEncrToBeSet--;
		}

		currentBitToBeSet	= currentAltNumber * m_numOfCaps
							+ m_numOfAudioCap
							+ m_numOfVideoCap
							+ m_numOfContentCap
							+ m_numOfDuoCap
							+ m_numOfT120Cap
							+ m_numOfFeccCap
							+ m_numOfPeopContCap
							+ m_numOfH239ControlCap
							+ m_numOfNsCap
							+ m_numOfLprCap;

		currentAltNumber++;
	}
   // LPR Capabilities end

   /* Dynamic Payload type Capabilities start */
  if (m_numOfDynamicPTRControlCap)
   {
       // set h239ControlCap alt's bits in cap-matrix
       for( i = currentBitToBeSet; i < (currentBitToBeSet + m_numOfDynamicPTRControlCap); i++ )
		{
           //CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));
			CAP_FD_SET(i,&(m_pCap->altMatrix));
			if (currentEncrToBeSet)
				currentEncrToBeSet--;
		}

		currentBitToBeSet	= currentAltNumber * m_numOfCaps
							+ m_numOfAudioCap
							+ m_numOfVideoCap
							+ m_numOfContentCap
							+ m_numOfDuoCap
							+ m_numOfT120Cap
							+ m_numOfFeccCap
							+ m_numOfPeopContCap
							+ m_numOfH239ControlCap
							+ m_numOfNsCap
							+ m_numOfLprCap
							+ m_numOfDynamicPTRControlCap;

		currentAltNumber++;
	}
	  /* Dynamic Payload type Capabilities end */

   SetPeopleContentAlt();
}

/////////////////////////////////////////////////////////////////////////////
void  CCapH323::Serialize(WORD format,CSegment& seg)
{
	BYTE *cap = (BYTE *)&m_pCap->caps;
	WORD i = 0;

	switch (format)
	{
		case SERIALEMBD :
		case  NATIVE  :
		{
			DWORD  encAlg		= (DWORD)m_encAlg;
			DWORD  contentAlt	= (DWORD)m_contentAltNumber;
			DWORD  peopleAlt	= (DWORD)m_peopleAltNumber;


			for (i = 0; i<FD_SET_SIZE ; i++)
				seg << (DWORD)(m_pCap->altMatrix.fds_bits[i]);
			seg << ((WORD)(m_pCap->numberOfCaps));
			seg << (WORD)m_pCap->numberOfAlts;
			seg << (WORD)m_pCap->numberOfSim;

			seg << m_numOfCaps
				<< m_offsetWrite
				<< m_size
				<< m_numOfAudioCap
				<< m_numOfVideoCap
				<< m_numOfContentCap
				<< m_numOfDuoCap
				<< m_numOfT120Cap
				<< m_numOfFeccCap
				<< m_numOfPeopContCap
				<< m_numOfH239ControlCap
				<< m_numOfNsCap
				<< m_numOfLprCap
				<< m_numOfEncrypCap
				<< m_numOfDynamicPTRControlCap
				<< encAlg
				<< contentAlt
				<< peopleAlt;

			for(i=0;i<MAX_CAP_TYPES;i++)
				seg << m_capArray[i];

			for(i=0; i<m_offsetWrite; i++)
				seg << cap[i];
			for(i=0;i < m_pCap->numberOfCaps;i++)
			{
				seg << (DWORD)m_sortedCap[i].capTypeCode;
				seg << m_sortedCap[i].pCapPtr;
			}

			seg << m_dataMaxBitRate;

			seg << m_maxContRate;
			seg << m_maxContTdmRate;
			seg << m_bIsH239;
		    seg << m_bIsEPC;
			seg << m_bIsPCversion0;
			seg << m_bIsDBC2;

			seg << m_is263Plus;
			seg << (BYTE)m_h263_4CifMpi;
			seg << (BYTE)m_videoQuality;

			seg << m_bIsLpr;
			seg << m_bIsDPTR;
            break;
		}

	   default : { break;	}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CCapH323::DeSerialize(WORD format,CSegment  &seg)
{
	BYTE *cap = (BYTE *)&m_pCap->caps;
	WORD i = 0;
	WORD tmp;
	DWORD temp;

	switch (format)
	{
	case SERIALEMBD :
	case NATIVE     :
	{
		DWORD  encAlg;
		DWORD  contentAlt;
		DWORD  peopleAlt;

		for (i = 0; i<FD_SET_SIZE ; i++)
		{
			seg >> temp;
			m_pCap->altMatrix.fds_bits[i] = temp;
		}
		seg >> tmp;
		m_pCap->numberOfCaps = tmp;
		seg >> tmp;
		m_pCap->numberOfAlts = tmp;
		seg >> tmp;
		m_pCap->numberOfSim = tmp;

		seg >> m_numOfCaps
			>> m_offsetWrite
			>> m_size
			>> m_numOfAudioCap
			>> m_numOfVideoCap
			>> m_numOfContentCap
			>> m_numOfDuoCap
			>> m_numOfT120Cap
			>> m_numOfFeccCap
			>> m_numOfPeopContCap
			>> m_numOfH239ControlCap
			>> m_numOfNsCap
			>> m_numOfLprCap
			>> m_numOfEncrypCap
			<< m_numOfDynamicPTRControlCap
			>> encAlg
			>> contentAlt
			>> peopleAlt;

		 m_encAlg = (EenMediaType)encAlg;
		 m_contentAltNumber = (int)contentAlt;
		 m_peopleAltNumber = (int)peopleAlt;

		for(i=0; i<MAX_CAP_TYPES; i++)
			seg >> m_capArray[i];

		for(i=0; i<m_offsetWrite; i++)
			seg >> cap[i];
		for(i=0;i < m_pCap->numberOfCaps;i++)
		{
			seg >> temp;
			m_sortedCap[i].capTypeCode = temp;
			seg >> temp;
			m_sortedCap[i].pCapPtr = (capBuffer*)temp;
		}
		seg >> m_dataMaxBitRate;

		seg >> m_maxContRate;
		seg >> m_maxContTdmRate;
	    seg >> m_bIsH239;
		seg >> m_bIsEPC;
		seg >> m_bIsPCversion0;
		seg >> m_bIsDBC2;

		seg >> m_is263Plus;

		BYTE h263_4CifMpi;
		seg >> h263_4CifMpi;
		m_h263_4CifMpi = (APIS8)h263_4CifMpi;


		BYTE videoQuality;
		seg >> videoQuality;
		m_videoQuality = (eVideoQuality)videoQuality;

		seg >> m_bIsLpr;
		seg >> m_bIsDPTR;
        break;
	}
	default :  { break;	}

	}
}

/////////////////////////////////////////////////////////////////////////////
void  CCapH323::SerializeCapArrayOnly(CSegment& seg,BYTE bOperUse)
{
	WORD operatorOffsetWrite = m_offsetWrite;

	capBuffer* pCapBuffer	= &m_pCap->caps;
	// adjust the the offset write to operator structures
	int protocolRemoveCount = 0;
/*	if (bOperUse)
	{
		for(int i = 0; i < m_numOfCaps; i++)
		{
			CCapSetInfo capInfo	= (CapEnum)pCapBuffer->capTypeCode;

			if (capInfo.GetCapType() == cmCapAudio ||
				(CapEnum)capInfo == ePeopleContentCapCode || (CapEnum)capInfo == eRoleLabelCapCode)
				protocolRemoveCount ++;

			pCapBuffer = (capBuffer*)((BYTE*)pCapBuffer + (sizeof(capBufferBase) + pCapBuffer->capLength));
		}
//		operatorOffsetWrite -= (sizeof(APIS32) * protocolRemoveCount);
	}*/

		seg << operatorOffsetWrite;
	if (bOperUse)
	{
		BYTE*	pTemp			= new BYTE[m_offsetWrite];
		memset(pTemp,0,m_offsetWrite);
		int		tempOffset		= 0;
		pCapBuffer				= &m_pCap->caps;

		for(int i = 0; i < m_numOfCaps; i++)
		{
			CCapSetInfo capInfo			= (CapEnum)pCapBuffer->capTypeCode;
			WORD		capLen			= pCapBuffer->capLength;

//			if (capInfo.GetCapType() == cmCapAudio ||
//				(CapEnum)capInfo == ePeopleContentCapCode || (CapEnum)capInfo == eRoleLabelCapCode)
//				capLen -= sizeof(APIS32);

			int	capBufferLen	= sizeof(capBufferBase) + capLen;

			capBuffer* pCapTemp = (capBuffer*)(pTemp + tempOffset);
			memcpy((BYTE*)pCapTemp,(BYTE*)pCapBuffer,capBufferLen);
			pCapTemp->capTypeCode	= capInfo.GetIpCapCode();
			pCapTemp->capLength		= capLen;

			tempOffset += sizeof(capBufferBase) + pCapTemp->capLength;
			pCapBuffer = (capBuffer*)((BYTE*)pCapBuffer + (sizeof(capBufferBase) + pCapBuffer->capLength));
		}
		seg.Put(pTemp,m_offsetWrite);
		PDELETEA(pTemp);
	}
	else
	{
		BYTE* cap = (BYTE* )&m_pCap->caps;
		seg.Put(cap,m_offsetWrite);
	}
}

/////////////////////////////////////////////////////////////////////////
void  CCapH323::Initialize(int size)
{
	for(int i=0; i<MAX_CAP_TYPES; i++)
		m_capArray[i] = 0;

	delete[] (BYTE *)m_pCap;

	m_size = INITIAL_CAPS_LEN + sizeof(ctCapabilitiesBasicStruct);

	int newSize = size ? size : m_size;
	m_pCap = (ctCapabilitiesStruct*) new BYTE[newSize];
	memset(m_pCap,0,newSize);
	CAP_FD_ZERO(&(m_pCap->altMatrix));

	m_numOfCaps	= 0;
	m_offsetWrite = 0;
	m_contentAltNumber = -1;
	m_peopleAltNumber = -1;

/*
	// //Bug fix VNGR-9584 set all the H323Cap members to default
// 	WORD i = 0;
// 	for(i=0; i<(2*MAX_CAP_TYPES); i++)
// 	{
// 			m_sortedCap[i].capTypeCode=0;
// 			m_sortedCap[i].pCapPtr=NULL;
// 	}
// 	ZeroingMatchListArray();

// 	m_dataMaxBitRate	 = 0;
// 	m_maxContRate		 = 0;
// 	m_maxContTdmRate	 = 0;
// 	m_is263Plus			 = 0;

// 	m_encAlg			 = kUnKnownMediaType;
// 	m_bIsEPC			 = FALSE;
// 	m_bIsH239			 = FALSE;
// 	m_bIsPCversion0		 = FALSE;
// 	m_bIsDBC2		   	 = FALSE;
// 	m_h263_4CifMpi		 = -1;	// disable 4cif
// 	m_videoQuality		 = eVideoQualitySharpness;
*/
	m_bIsLpr			 = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void  CCapH323::SetNumOfCapsToZero()
{
	m_numOfAudioCap = 0;
    m_numOfVideoCap = 0;
	m_numOfContentCap = 0;
	m_numOfT120Cap = 0;
	m_numOfFeccCap			= 0;
	m_numOfPeopContCap		= 0;
	m_numOfH239ControlCap	= 0;
	m_numOfNsCap			= 0;
	m_numOfEncrypCap		= 0;
	m_numOfLprCap			= 0;
	m_numOfDynamicPTRControlCap = 0;

	//Bug fix VNGR-9584 set all the H323Cap members to default
	m_numOfDuoCap			= 0;

	m_bIsH239 = FALSE;
	m_bIsEPC  = FALSE;
	m_bIsDPTR  = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void  CCapH323::Create(ctCapabilitiesStruct* pCap, int size)
{
	Initialize(size);
	SetNumOfCapsToZero();

	m_pCap->numberOfCaps	= pCap->numberOfCaps;
	m_pCap->numberOfAlts	= pCap->numberOfAlts;
	m_pCap->numberOfSim		= pCap->numberOfSim;

	if (m_pCap->numberOfCaps != 0)
	{
		int i = 0;
		for( i = 0; i < FD_SET_SIZE; i++)
			m_pCap->altMatrix.fds_bits[i] = pCap->altMatrix.fds_bits[i];

		capBuffer* pCapBuffer;
		pCapBuffer = (capBuffer *) &pCap->caps;
		BuildRemoteCapabilities(pCapBuffer);

		BaseCapStruct* pCapBaseStruct = NULL;
		pCapBuffer					  = &m_pCap->caps;
		CapEnum capCode;

		// update the counter of each capability type.
		for(i = 0; i < m_numOfCaps; i++)
		{
			pCapBaseStruct = (BaseCapStruct*)pCapBuffer->dataCap;
			capCode        = (CapEnum)pCapBuffer->capTypeCode;

			if(pCapBaseStruct->header.type == cmCapAudio)
				m_numOfAudioCap++;
			else if(pCapBaseStruct->header.type == cmCapVideo)
			{
				if (pCapBaseStruct->header.roleLabel == kRolePeople)
					m_numOfVideoCap++;
				else
					m_numOfContentCap++;
			}
			else if(pCapBaseStruct->header.type == cmCapData)
			{
				if(pCapBaseStruct->header.capTypeCode == eT120DataCapCode)
					m_numOfT120Cap++;
				else if(pCapBaseStruct->header.capTypeCode == eAnnexQCapCode
					|| pCapBaseStruct->header.capTypeCode == eRvFeccCapCode)
					m_numOfFeccCap++;
			}
			else if(pCapBaseStruct->header.type == cmCapNonStandard)
				m_numOfNsCap++;
			else if (capCode == eH239ControlCapCode)
				m_numOfH239ControlCap++;
			else if (capCode == eDynamicPTRCapCode)
				m_numOfDynamicPTRControlCap++;
			else if ((capCode == ePeopleContentCapCode) || (pCapBaseStruct->header.roleLabel == kRoleContent))
				m_numOfPeopContCap++;
			else if (capCode == eLPRCapCode)
				m_numOfLprCap++;
			else if(pCapBaseStruct->header.type == cmCapH235)
				m_numOfEncrypCap++;

			pCapBuffer = (capBuffer *)((BYTE *)pCapBuffer + sizeof(capBufferBase) + pCapBuffer->capLength);
		}

		if (CheckProfile(2))
		{
			m_bIsEPC  = TRUE;
		}
        else
            m_bIsEPC  = FALSE;
		if(CheckProfile(0))
			m_bIsPCversion0 = TRUE;
        else
            m_bIsPCversion0 = FALSE;
		if (m_numOfH239ControlCap)
			m_bIsH239 = TRUE;
        else
            m_bIsH239 = FALSE;

		if (m_numOfLprCap)
			m_bIsLpr = TRUE;
        else
        	m_bIsLpr = FALSE;

		SetPeopleContentAlt();
		m_bIsDBC2 = CheckDBC2();
	}
}

/////////////////////////////////////////////////////////////////////////////
CCapH323& CCapH323::operator=(const CCapH323& other)
{
	m_numOfCaps					   = other.m_numOfCaps;
	m_numOfAudioCap				   = other.m_numOfAudioCap;
	m_numOfVideoCap				   = other.m_numOfVideoCap;
	m_numOfContentCap              = other.m_numOfContentCap;
	m_numOfDuoCap                  = other.m_numOfDuoCap;
	m_numOfT120Cap				   = other.m_numOfT120Cap;
	m_numOfFeccCap				   = other.m_numOfFeccCap;
	m_numOfPeopContCap			   = other.m_numOfPeopContCap;
	m_numOfH239ControlCap		   = other.m_numOfH239ControlCap;
	m_numOfNsCap				   = other.m_numOfNsCap;
	m_numOfEncrypCap			   = other.m_numOfEncrypCap;
	m_numOfLprCap				   = other.m_numOfLprCap;
	m_numOfDynamicPTRControlCap    = other.m_numOfDynamicPTRControlCap;
	m_size						   = other.m_size;
	memcpy(m_pCap, other.m_pCap, m_size);
	m_offsetWrite				   = other.m_offsetWrite;
	WORD i = 0;
	for( i =0; i<MAX_CAP_TYPES; i++)
		m_capArray[i] = other.m_capArray[i];
	for(i=0;i<(2*MAX_CAP_TYPES);i++)
	{
		m_sortedCap[i].capTypeCode = other.m_sortedCap[i].capTypeCode;
		m_sortedCap[i].pCapPtr     = other.m_sortedCap[i].pCapPtr;
	}

	m_dataMaxBitRate      = other.m_dataMaxBitRate;
	m_maxContRate         = other.m_maxContRate;
	m_maxContTdmRate	  = other.m_maxContTdmRate;
	m_is263Plus		      = other.m_is263Plus;
	m_encAlg			  = other.m_encAlg;
	m_contentAltNumber	  = other.m_contentAltNumber;
	m_peopleAltNumber     = other.m_peopleAltNumber;
	m_bIsEPC			  = other.m_bIsEPC;
	m_bIsH239		      = other.m_bIsH239;
	m_bIsPCversion0		  = other.m_bIsPCversion0;
	m_bIsDBC2			  = other.m_bIsDBC2;
	m_h263_4CifMpi		  = other.m_h263_4CifMpi;
	m_videoQuality		  = other.m_videoQuality;
	m_bIsLpr			  = other.m_bIsLpr;
	m_bIsDPTR             = other.m_bIsDPTR;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: The function is now supporting caps that the MCU can handle.
//             Need to check this function in any change in algorithem !!
//	           WARNING: at we transmit only parameters of video algorithems.
//             need to check if we don't need to pass also audio algorithems parameters
//---------------------------------------------------------------------------------------------------
/*void  CCapH323::GenerateH221Caps(CCapH221* pH221Cap, BYTE bGateWay, cmCapDirection direction) const
{
    DWORD maxAudioBitRate = 0;
	DWORD maxVideoBitRate = 0;
	WORD  NoOfVideoCapFailedGenerated = 0;
	// set the capabilities in H320 parameters according to the H323 capabilities

	capBuffer	*pCapBuffer			= (capBuffer *) &m_pCap->caps;
	char		*tempPtr			= (char *) pCapBuffer;
	BOOL		clearVidCap			= FALSE;
	BOOL		bIsPartyEncrypted	= IsPartyEncrypted();
	BOOL		bIsCapOk			= TRUE;
	BOOL		bIsDataCapSet		= FALSE;

	for(int i = 0; i < m_numOfCaps; i++)
	{
		CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;

		if(bIsPartyEncrypted)
			bIsCapOk = IsEncrypedCap(i+1);//In case the party is encrypt we should generate only encrypted caps.

		if(bIsCapOk)
		{
			// set audio
			if(capInfo.IsType(cmCapAudio))
			{
				DWORD currentBitRate = capInfo.GetBitRate();
				maxAudioBitRate  = max(maxAudioBitRate,currentBitRate);

				WORD h221CapCode = capInfo.GetH221CapCode();
				pH221Cap->SetAudXferCap(h221CapCode);

				// in case of g722 add this too
				if (capInfo.IsCapCode(g722_64kCapCode))
					pH221Cap->SetAudXferCap(G722_48);
			}

			// set video
			else if(capInfo.IsType(cmCapVideo))
			{
				if(clearVidCap == FALSE)
				{
					// First we need to clears video caps

					// Bits 20 up to 25 are null (video)
					DWORD vidCapOff = 0xfc0fffff;

					pH221Cap->SetDataVidCapOff(vidCapOff);
					clearVidCap = TRUE;
				}

				CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(capInfo,pCapBuffer->dataCap);

				if (pVideoCap)
				{
					if ((direction == cmCapReceiveAndTransmit || pVideoCap->IsDirection(direction)) && CheckRole(kRolePeople, i,(CapEnum)pCapBuffer->capTypeCode))
					{
						EResult eRes = pVideoCap->GenerateH221Caps(pH221Cap);
						if (eRes == kFailure)
						{
							PTRACE(eLevelError,"CCapH323::GenerateH221Caps: Generate Of One Video Cap Failed!!!");
							NoOfVideoCapFailedGenerated++;
						}
						DWORD currentBitRate = pVideoCap->GetBitRate();
						maxVideoBitRate = max(maxVideoBitRate,currentBitRate);
					}
				}

				POBJDELETE(pVideoCap);
			}

			// set data
			// Can be T120 OR fecc caps
			else if((capInfo.IsType(cmCapData)) && (!bIsDataCapSet))
			{
				CBaseDataCap *pDataCap = (CBaseDataCap *)CBaseCap::AllocNewCap(capInfo,pCapBuffer->dataCap);
				if (pDataCap)
				{
					DWORD bitRate = pDataCap->GetBitRate() * 100;
					if(bitRate)
					{
						CComModeInfo cmInfo(pDataCap->GetCapCode(),bitRate);
						WORD h320ModeType = cmInfo.GetH320ModeType();

						if(pDataCap->GetCapCode() == t120DataCapCode)
						{
							if (h320ModeType != (WORD)NA)
								pH221Cap->SetT120Cap(h320ModeType);
							else
								PTRACE(eLevelInfoNormal,"CCapH323::GenerateH221Caps : unknown T120 cap rate");

						}
						else//annexQ or rvFecc
						{
							//In case of GW this it the bit rate of the remote caps, and he can declare on ant bit rate
							//he want, so when we want to convert it to 320 type we can't find, so we pt 6.4.
							if(bGateWay)
								pH221Cap->SetLsdCap(LSD_6400);
							else
							{
								if (h320ModeType != (WORD)NA)
									pH221Cap->SetLsdCap(h320ModeType);
								else
									PTRACE(eLevelInfoNormal,"CCapH323::GenerateH221Caps : unknown FECC cap rate");
							}
						}

						bIsDataCapSet = TRUE;
					}
				}
				POBJDELETE(pDataCap);
			}

			// set non standard
			else if (capInfo.IsType(cmCapNonStandard))
			{
				if( IsVisualConcert(NS_CAP_H323_VISUAL_CONCERT_PC) )
				{
					pH221Cap->AddVisualConcertPC();
					pH221Cap->SetMVCCap(YES);
				}
				else if( IsVisualConcert(NS_CAP_H323_VISUAL_CONCERT_FX) )
				{
					pH221Cap->AddVisualConcertFX();
					pH221Cap->SetMVCCap(YES);
				}
				else
					PTRACE(eLevelInfoNormal,"CCapH323::GenerateH221Caps : unsupported nonStandard H323 capability!!!");
			}
		}

		//for PeopleContentCapCode we do not encrypt so in any case we should genarate
		if (capInfo.IsCapCode(PeopleContentCapCode) && m_bIsEPC)
		{
			pH221Cap->AddEnterprisePeopleContent(Xfer_384,0, 0);
			pH221Cap->SetMVCCap(YES);
		}
		else if (capInfo.IsCapCode(PeopleContentCapCode))
		{
			pH221Cap->AddPeopleContent();
			pH221Cap->SetMVCCap(YES);
		}
		else if(capInfo.IsCapCode(H239ControlCapCode))
		{
			pH221Cap->AddH239Cap(Xfer_384,0, 0);
			pH221Cap->SetMVCCap(YES);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

	// if there wes at least one video cap that failed to generate and no video cap succed to generate
	// then make debug assert ( it could have been SQCif that the MCU is not supported).
	if((NoOfVideoCapFailedGenerated != 0) && (maxVideoBitRate == 0))
		PTRACE(eLevelInfoNormal,"CCapH323::GenerateH221Caps : The EP got video cap but its video cap failed to be generated!!!");

	// In case of GW we calculate the XferMode at GW conf and not here.
	if(bGateWay == FALSE)
	{
		// Xfer -  [audio + video + FAS + BAS]
		DWORD fasWidth  = 800;
		DWORD basWidth  = 800;
		DWORD maxConfBitRate = maxVideoBitRate*100  + maxAudioBitRate;//video323 is in 100 bit/sec
		int   numOfBChannals = maxConfBitRate / (64*_K_);
		DWORD fullBitRate    = numOfBChannals*(fasWidth + basWidth) + maxConfBitRate;
		int   numOfBChannalsWithFasBas = fullBitRate / (64*_K_);

		if(numOfBChannalsWithFasBas > numOfBChannals)
			numOfBChannalsWithFasBas++;
		if((numOfBChannalsWithFasBas > 30)&&((maxVideoBitRate*100)/(64*_K_) > 30))
			PTRACE(eLevelInfoNormal,"CCapH323::GenerateH221Caps: Call ability is bigger than E1");  // not more than 30 b channals

		// this array holds in each entry a pair of numbers where the first is
		// the number of b channels with fas and bas and the second is the Xfer cap
		// to set.
		WORD BChannelsColumn	   = 1;
		WORD XferCapColumn		   = 2;
		WORD audXferCapUtilArr[12][2] =
		{
			{1, Xfer_Cap_B},
			{2, Xfer_Cap_2B},
			{3, Xfer_Cap_3B},
			{4, Xfer_Cap_4B},
			{5, Xfer_Cap_5B},
			{6, Xfer_Cap_6B},  // 6-7
			{8, Xfer_Cap_512}, // 8-11
			{12,Xfer_Cap_768}, //12-17
			{18,Xfer_Cap_1152},//18-22
			{23,Xfer_Cap_1472},
			{24,Xfer_Cap_4H0}, //24-29
			{30,Xfer_Cap_H12}, //30-?
		};

		if (numOfBChannalsWithFasBas > 0)
		{
			for (int j = 0; j<12; j++ )
			{
				if (numOfBChannalsWithFasBas < audXferCapUtilArr[j][BChannelsColumn - 1])
				{
					pH221Cap->SetAudXferCapUtil(audXferCapUtilArr[j-1][XferCapColumn - 1]);
					break;
				}
			}
		}
	}
}*/

/////////////////////////////////////////////////////////////////////////////
void  CCapH323::BuildRemoteCapabilities(capBuffer* pCapRmtBuf)
{
	capBuffer*	pCapBuffer = (capBuffer *) &m_pCap->caps;
	WORD i=0;
	for(i=0; i<m_pCap->numberOfCaps; i++)
	{
		m_capArray[pCapRmtBuf->capTypeCode]++;
		m_numOfCaps++;

 		if((pCapRmtBuf->capLength + sizeof(capBufferBase) + m_offsetWrite) < GetCurrentCapsBufSize())
		{
			pCapBuffer->capTypeCode = pCapRmtBuf->capTypeCode;
			pCapBuffer->capLength = pCapRmtBuf->capLength;
			memcpy(pCapBuffer->dataCap, pCapRmtBuf->dataCap, pCapRmtBuf->capLength);
			m_offsetWrite = m_offsetWrite + sizeof(capBufferBase) + pCapRmtBuf->capLength;
			pCapRmtBuf = (capBuffer *)((BYTE *)pCapRmtBuf + sizeof(capBufferBase) + pCapBuffer->capLength);
			pCapBuffer = (capBuffer *)((BYTE *)&m_pCap->caps + m_offsetWrite);
		}
		else
		{
			capBuffer*	pNewCapBuffer = (capBuffer *)new BYTE[INITIAL_CAPS_LEN];
			pNewCapBuffer->capTypeCode	= pCapRmtBuf->capTypeCode;
			pNewCapBuffer->capLength	= pCapRmtBuf->capLength;
			memcpy(pNewCapBuffer->dataCap, pCapRmtBuf->dataCap, pCapRmtBuf->capLength);
			AllocateNewBuffer(pNewCapBuffer, pCapRmtBuf->capLength + sizeof(capBufferBase));
			pCapBuffer = (capBuffer *)((BYTE *)&m_pCap->caps + m_offsetWrite);
			pCapRmtBuf = (capBuffer *)((BYTE *)pCapRmtBuf + sizeof(capBufferBase) + pNewCapBuffer->capLength);
			delete[] (BYTE *)pNewCapBuffer;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void CCapH323::InitCapArrayStruct(int capTypeCode, char* dataCap, capHeadStruct* pEntry)
{
	strcpy_safe(pEntry->capName, CapEnumToString((CapEnum)capTypeCode));
	pEntry->pCapPtr = (BaseCapStruct*)dataCap;
}

////////////////////////////////////////////////////////////////////////////////////////////
void CCapH323::AllocateNewBuffer(capBuffer* pCapBuffer, WORD len)
{
	int compLen = m_offsetWrite + len - GetCurrentCapsBufSize();

	if ( m_pCap != NULL )
	{
		BYTE* pTmpBuf = new BYTE[BUFSIZE(m_size + compLen)];
		PASSERTMSG_AND_RETURN(!pTmpBuf, "CCapH323::AllocateNewBuffer - TempBuff not valid");
		memcpy(pTmpBuf,m_pCap,m_size);
		delete[] (BYTE *)m_pCap;
		m_pCap = (ctCapabilitiesStruct*)pTmpBuf;
		memcpy((BYTE *)&m_pCap->caps + m_offsetWrite ,pCapBuffer,len);
		m_offsetWrite += len;
		m_size = BUFSIZE(m_size + compLen);
	}
	else
		PTRACE(eLevelError,"CCapH323::AllocateNewBuffer - m_pCap NULL POINTER !!!!");
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::GetCapTotalLength(void) const
{
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
    char* tempPtr = (char*)pCapBuffer;
	WORD   length = sizeof(ctCapabilitiesBasicStruct);

	for(BYTE i=0; i<m_numOfCaps; i++)
	{
		length += pCapBuffer->capLength + sizeof(capBufferBase);
		tempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return length;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::CheckProfile(int profile) const
{
	BYTE bRes = FALSE;
	capBuffer *pCapBuffer	= (capBuffer *) &m_pCap->caps;
	char *tempPtr			= (char*)pCapBuffer;

	for(int i = 0; i < m_numOfCaps; i++)
	{
		if (pCapBuffer->capTypeCode == ePeopleContentCapCode)
		{
			CBaseAudioCap* pPeopleContentCap = (CBaseAudioCap *)CBaseCap::AllocNewCap(ePeopleContentCapCode, pCapBuffer->dataCap);
			if (pPeopleContentCap)
			{
				if(pPeopleContentCap->GetContentProfile() == profile)
				{
					POBJDELETE(pPeopleContentCap);
					return TRUE;
				}
			}
			POBJDELETE(pPeopleContentCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

	return bRes;
}

////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CCapH323::IsDBC2() const
{
	return m_bIsDBC2;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::IsFECC() const
{
	BYTE bRes = FALSE;
	capBuffer *pCapBuffer = (capBuffer *) &m_pCap->caps;
    char      *tempPtr    = (char*)pCapBuffer;

	for(BYTE i=0; (i<m_numOfCaps) && (bRes == FALSE); i++)
	{
		if ((pCapBuffer->capTypeCode == eAnnexQCapCode) || (pCapBuffer->capTypeCode == eRvFeccCapCode))
			bRes = TRUE;

		tempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

	return bRes;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::OnTypeRole(cmCapDataType eType,ERoleLabel eRole) const
{
	BYTE bRes = FALSE;
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
    char*	   tempPtr    = (char*)pCapBuffer;

	for(BYTE i=0; (i<m_numOfCaps) && (bRes == FALSE); i++)
	{
		CBaseCap* pCurrentCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);
		if (pCurrentCap)
		{
			BYTE bIsRoleCorrect = TRUE;
			if(eType == cmCapVideo)
				bIsRoleCorrect = CheckRole(eRole,i,pCurrentCap);

			if (pCurrentCap->IsType(eType) && bIsRoleCorrect)
				bRes = TRUE;
		}
		POBJDELETE(pCurrentCap);

		tempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return bRes;
}

////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::OnType(cmCapDataType type) const
{
	BYTE bRes = FALSE;
	for (int i=0; i<eUnknownAlgorithemCapCode; i++)
	{
		CCapSetInfo capInfo = (CapEnum)i;
		if (capInfo.IsType(type))
		{
			if(OnCap(i))
			{
				bRes = TRUE;
				break;
			}
		}
	}
	return bRes;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::IsAdvanceAudioCodecSupported(WORD cap, BYTE implicitCheck) const
{// Siren14Stereo, Siren22 and Siren22Stereo are implemented at the EP by declaring the hishest rate cap only and not all the supported ones
	// so if the capabilities contained audio codec cap with higher rate, then its support also the same
	//codec with lower rate without the need to declare on it specificlly
	PTRACE2INT(eLevelInfoNormal,"CCapH323::IsAdvanceAudioCodecSupported - ", cap);
	switch(cap)
	{
    case eSiren14_24kCapCode:
         return OnCap(eSiren14_32kCapCode, implicitCheck);

    case eSiren14_32kCapCode:
         return OnCap(eSiren14_48kCapCode, implicitCheck);

	case eSiren14Stereo_48kCapCode:
		return OnCap(eSiren14Stereo_56kCapCode, implicitCheck);

	case eSiren14Stereo_56kCapCode:
		return OnCap(eSiren14Stereo_64kCapCode, implicitCheck);

	case eSiren14Stereo_64kCapCode:
		return OnCap(eSiren14Stereo_96kCapCode, implicitCheck);

	case eSiren22Stereo_64kCapCode:
		return OnCap(eSiren22Stereo_96kCapCode, implicitCheck);

	case eSiren22Stereo_96kCapCode:
		return OnCap(eSiren22Stereo_128kCapCode, implicitCheck);

	case eSiren22_32kCapCode:
		return OnCap(eSiren22_48kCapCode, implicitCheck);

	case eSiren22_48kCapCode:
		return OnCap(eSiren22_64kCapCode, implicitCheck);

	case eSirenLPR_32kCapCode:
		return OnCap(eSirenLPR_48kCapCode, implicitCheck);

	case eSirenLPR_48kCapCode:
		return OnCap(eSirenLPR_64kCapCode, implicitCheck);

	case eSirenLPR_64kCapCode:
		return OnCap(eSirenLPRStereo_64kCapCode, implicitCheck); // Added by Efi

	case eSirenLPRStereo_64kCapCode:
		return OnCap(eSirenLPRStereo_96kCapCode, implicitCheck);

	case eSirenLPRStereo_96kCapCode:
		return OnCap(eSirenLPRStereo_128kCapCode, implicitCheck);

	case eG719_32kCapCode:
		return OnCap(eG719_48kCapCode, implicitCheck);

	case eG719_48kCapCode:
		return OnCap(eG719_64kCapCode, implicitCheck);

	case eG719_64kCapCode:
		return OnCap(eG719_96kCapCode, implicitCheck);

	case eG719_96kCapCode:
		return OnCap(eG719_128kCapCode, implicitCheck);

	case eG719Stereo_64kCapCode:
		return OnCap(eG719Stereo_96kCapCode, implicitCheck);

	case eG719Stereo_96kCapCode:
		return OnCap(eG719Stereo_128kCapCode, implicitCheck);
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::OnCap(WORD cap, BYTE implicitCheck) const
{
	PASSERTMSG_AND_RETURN_VALUE(cap >= MAX_CAP_TYPES, 
		"CCapH323::OnCap - Cap >= MAX_CAP_TYPE", 0);

	if(m_capArray[cap] != 0)
		return 1;
	else if((cap == eG7221C_24kCapCode) ||  (cap == eG7221C_32kCapCode) || (cap == eG7221C_48kCapCode))
	{// G7221C can hold 3 CapEnum 24K, 32K and 48K
		capBuffer*	pCapBuffer = (capBuffer *) &m_pCap->caps;
	    char*	    tempPtr    = (char*)pCapBuffer;
		WORD i = 0;
		for(i=0; i < m_pCap->numberOfCaps; i++)
		{
			if(pCapBuffer->capTypeCode == eG7221C_CapCode)
			{
				BYTE rVal = 0;
				CG7221CAudioCap * pAudioCap = (CG7221CAudioCap *)CBaseCap::AllocNewCap(eG7221C_CapCode, pCapBuffer->dataCap);
				if (pAudioCap)
				{
					rVal = pAudioCap->isCapEnumSupported((CapEnum)cap);
					POBJDELETE(pAudioCap);
					if(rVal)
						return 1;
				}
			}
			tempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
		return 0;
	}
	else if(implicitCheck)
	{
		//PTRACE2INT(eLevelInfoNormal,"CCapH323::OnCap, cap code is  = calling advanced with:  ",(WORD)cap);
		return IsAdvanceAudioCodecSupported(cap, implicitCheck);
	}

	return 0;
}
/////////////////////////////////////////////////////////////
int CCapH323::GetMaxRateOfAdvanceAudioCodecSupported(WORD cap,BYTE implicitCheck) const
{
	//PTRACE2INT(eLevelInfoNormal,"CCapH323::GetReverseAdvanceAudioCodecSupported - ", cap);
		switch(cap)
		{
		case eSiren14Stereo_48kCapCode:
			return GetMaxAudioFramePerPacket(eSiren14Stereo_56kCapCode,(BYTE)TRUE);
			break;
		case eSiren14Stereo_56kCapCode:
			return GetMaxAudioFramePerPacket(eSiren14Stereo_64kCapCode,TRUE);
			break;
		case eSiren14Stereo_64kCapCode:
			return GetMaxAudioFramePerPacket(eSiren14Stereo_96kCapCode,TRUE);
			break;
		case eSiren22Stereo_64kCapCode:
			return GetMaxAudioFramePerPacket(eSiren22Stereo_96kCapCode,TRUE);
			break;
		case eSiren22Stereo_96kCapCode:
			return GetMaxAudioFramePerPacket(eSiren22Stereo_128kCapCode,TRUE);
			break;
		case eSiren22_32kCapCode:
			return GetMaxAudioFramePerPacket(eSiren22_48kCapCode,TRUE);
			break;
		case eSiren22_48kCapCode:
			return GetMaxAudioFramePerPacket(eSiren22_64kCapCode,TRUE);
			break;
		case eSirenLPR_32kCapCode:
			return GetMaxAudioFramePerPacket(eSirenLPR_48kCapCode,TRUE);
			break;
		case eSirenLPR_48kCapCode:
			return GetMaxAudioFramePerPacket(eSirenLPR_64kCapCode,TRUE);
			break;
		case eSirenLPRStereo_64kCapCode:
			return GetMaxAudioFramePerPacket(eSirenLPRStereo_96kCapCode,TRUE);
			break;
		case eSirenLPRStereo_96kCapCode:
			return GetMaxAudioFramePerPacket(eSirenLPRStereo_128kCapCode,TRUE);
			break;
		case eG719_32kCapCode:
			return GetMaxAudioFramePerPacket(eG719_48kCapCode,TRUE);
			break;
		case eG719_48kCapCode:
			return GetMaxAudioFramePerPacket(eG719_64kCapCode,TRUE);
			break;
		case eG719_64kCapCode:
			return GetMaxAudioFramePerPacket(eG719_96kCapCode,TRUE);
			break;
		case eG719_96kCapCode:
			return GetMaxAudioFramePerPacket(eG719_128kCapCode,TRUE);
			break;
		case eG719Stereo_64kCapCode:
			return GetMaxAudioFramePerPacket(eG719Stereo_96kCapCode,TRUE);
			break;
		case eG719Stereo_96kCapCode:
			return GetMaxAudioFramePerPacket(eG719Stereo_128kCapCode,TRUE);
			break;
		}

			return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
//returns the first audio cap in the capabilities
const BaseCapStruct* CCapH323::GetAudioDesiredMode() const
{
    DWORD rate = 0;
	capBuffer* pCapBuffer  = (capBuffer *) &m_pCap->caps;
	char*	   tempPtr     = (char*)pCapBuffer;
	BaseCapStruct* pStruct = NULL;

	for (int i = 0; (i < m_numOfCaps) && !pStruct; i++)
	{
		CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pCap)
		{
			if (pCap->IsType(cmCapAudio))
				pStruct = pCap->GetStruct();
			POBJDELETE(pCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return pStruct;
}

/////////////////////////////////////////////
//returns the rate of the first audio cap in the capabilities
DWORD CCapH323::GetAudioDesiredRate() const
{
    DWORD rate = 0;
	const BaseCapStruct* pStruct = GetAudioDesiredMode();
	if (pStruct)
	{
		CCapSetInfo capInfo((CapEnum)pStruct->header.capTypeCode);
		rate = capInfo.GetBitRate() / 1000;
	}
	return rate;
}

////////////////////////////////////////////////////////////////////////////////////////////
// in 100 bits
DWORD CCapH323::GetMaxContentBitRate() const
{
	DWORD maxContentRate = 0;
	CCapSetInfo capInfo = eH263CapCode;
	if (m_bIsH239 || m_bIsEPC)
	{
		maxContentRate = GetMaxVideoBitRate((CapEnum)capInfo,cmCapReceive,kRoleContentOrPresentation);
		capInfo = eH264CapCode;
		maxContentRate = max(maxContentRate, GetMaxVideoBitRate((CapEnum)capInfo,cmCapReceive,kRoleContentOrPresentation));
	}

/*	else if (m_bIsEPC)
	{
		maxContentRate = GetMaxVideoBitRate((CapEnum)capInfo,cmCapReceive,kRoleContent);
		//if the capabilities are implicit we take from the video
		if (maxContentRate == 0)
			maxContentRate = GetMaxVideoBitRate((CapEnum)capInfo,cmCapReceive,kRolePeople);
	}*/
	return maxContentRate;
}

////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsContentRateSupported(BYTE h320RateAMSC) const
{
	BYTE res = FALSE;
	CCapSetInfo capInfo       = eH263CapCode;
	DWORD maxContentRate      = GetMaxContentBitRate()*100;
	DWORD maxContentTDMRate   = CalculateTdmRate(maxContentRate);
	DWORD givenContentTDMRate = capInfo.GetEpcBitRate(h320RateAMSC/*,GetRestric()*/);
	res = (maxContentTDMRate >= givenContentTDMRate)? TRUE: FALSE;
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::GetMaxContentOrDuoRateInAMSCValue() const
{
	BYTE AMSCValue;
	for( AMSCValue=AMSC_1536k; AMSCValue>=AMSC_64k; AMSCValue-- )
	{
		if(IsContentRateSupported(AMSCValue))
			break;
	}
//	PTRACE2INT2(eLevelInfoNormal,"CCapH323::GetMaxContentOrDuoRateInAMSCValue: max %d",(int)AMSCValue);
	return AMSCValue;
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::GetMaxContentRateInAMSCValue() const
{
	BYTE AMSCValue;
	for( AMSCValue=AMSC_1536k; AMSCValue>=AMSC_64k; AMSCValue-- )
	{
		if(IsContentRateSupported(AMSCValue))
			break;
	}
//	PTRACE2INT2(eLevelInfoNormal,"CCapH323::GetMaxContentRateInAMSCValue: max %d",(int)AMSCValue);
	return AMSCValue;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Function name: SetVideoBitRate            written by: uri avni
// Variables:     vid_bitrate the new video rate of the video capabilities.
// Description:	  Set the video rate to the parameters one.
// Return value:  -1 - in case of no video caps. 0 - other
//////////////////////////////////////////////////////////////////////////////////////////////
int CCapH323::SetVideoBitRate(int newBitRate, ERoleLabel eRole, CapEnum protocolToChange)
{
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*	   tempPtr    = (char*)pCapBuffer;

	BYTE bChangeSpecificProtocol = protocolToChange != eUnknownAlgorithemCapCode;

	for(int i = 0; i < m_numOfCaps; i++)
	{
		CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;
		if (capInfo.IsType(cmCapVideo))
		{
			if (!bChangeSpecificProtocol || (bChangeSpecificProtocol && capInfo.IsCapCode(protocolToChange)) )
			{
				CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(capInfo,pCapBuffer->dataCap);
				if (pVideoCap)
				{
					BYTE bIsRoleCorrect = CheckRole(eRole,i,pVideoCap);
					if (bIsRoleCorrect)
					{
						EResult eResOfSet = pVideoCap->SetBitRate(newBitRate);
						if (eResOfSet == kFailure)
							PTRACE(eLevelInfoNormal,"CCapH323::SetVideoBitRate: Set struct has failed");
					}
				}
				POBJDELETE(pVideoCap);
			}
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
/*DWORD CCapH323::GetMaxVideoBitRateByH320Protocol(BYTE h320VideoProtocol, cmCapDirection eDirection,ERoleLabel eRole) const
{
	CComModeInfo comModeInfo(h320VideoProtocol, StartVideoCap);
	CapEnum h323CapCode = comModeInfo.GetH323ModeType();
	return GetMaxVideoBitRate(h323CapCode, eDirection, eRole);
}*/

////////////////////////////////////////////////////////////////////////////////////////////
// In 100 bits!!
// if eDirection is cmCapReceiveAndTransmit it will calculate all of the cap sets!!!
DWORD CCapH323::GetMaxVideoBitRate(CapEnum videoType, cmCapDirection eDirection,ERoleLabel eRole) const
{
	DWORD maxBitRate = 0;
	CCapSetInfo capInfo = videoType;
	if(capInfo.IsType(cmCapVideo)&&OnCap(videoType))
	{
		capBuffer *pCapBuffer = (capBuffer *) &m_pCap->caps;
		char      *tempPtr    = (char*)pCapBuffer;

		for(int i = 0; i < m_numOfCaps; i++)
		{
			if (pCapBuffer->capTypeCode == videoType)
			{
				CBaseVideoCap * pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(videoType,pCapBuffer->dataCap);
				if(pVideoCap)
				{
					BYTE bIsRoleCorrect = CheckRole(eRole,i,pVideoCap);
					if ((eDirection == cmCapReceiveAndTransmit || pVideoCap->IsDirection(eDirection)) && bIsRoleCorrect)
				{
					DWORD currentBitRate = pVideoCap->GetBitRate();
					maxBitRate = max(maxBitRate,currentBitRate);
				}
				}
				POBJDELETE(pVideoCap);
			}

			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal,
			"CCapH323::GetMaxVideoBitRate: This is not a video algorithm or not in capset!!!",capInfo.GetH323CapName());
	}
	return maxBitRate;
}


////////////////////////////////////////////////////////////////////////////////////////////
int CCapH323::GetMaxAudioFramePerPacket(CapEnum audioType,BYTE implicitCheck) const
{
	PTRACE2INT(eLevelInfoNormal,
				"CCapH323::GetMaxAudioFramePerPacket: cap id - ",(WORD)audioType);
	int framePerPacket = 0;
	CCapSetInfo capInfo = audioType;
	if(capInfo.IsType(cmCapAudio) && OnCap(audioType))
	{
		capBuffer *pCapBuffer = (capBuffer *) &m_pCap->caps;
		char      *tempPtr = (char*)pCapBuffer;

		for(int i = 0; i < m_numOfCaps; i++)
		{
			BYTE isGeneralG7221C_Cap = FALSE;
			if(pCapBuffer->capTypeCode == eG7221C_CapCode)
			{
				CG7221CAudioCap * pAudioCap = (CG7221CAudioCap *)CBaseCap::AllocNewCap(eG7221C_CapCode, pCapBuffer->dataCap);
				if (pAudioCap)
					isGeneralG7221C_Cap = pAudioCap->isCapEnumSupported(audioType);
				POBJDELETE(pAudioCap);
			}

			if ((pCapBuffer->capTypeCode == audioType) || isGeneralG7221C_Cap)
			{
				CBaseAudioCap * pAudioCap = (CBaseAudioCap *)CBaseCap::AllocNewCap(audioType,pCapBuffer->dataCap);
				if (pAudioCap)
				{
					// takes the higher frames per packet for this alg
					int currentFramePerPacket = pAudioCap->GetMaxFramePerPacket();
					if (currentFramePerPacket > 0)
						framePerPacket = max(framePerPacket,currentFramePerPacket);
				}
				POBJDELETE(pAudioCap);
			}

			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal,
			"CCapH323::GetMaxAudioFramePerPacket: not an audio algorithm or not in capset - ",capInfo.GetH323CapName());
		if(implicitCheck)
		{
			return GetMaxRateOfAdvanceAudioCodecSupported((WORD)audioType,implicitCheck);
		}
	}
	return framePerPacket;
}

////////////////////////////////////////////////////////////////////////////////////////////
int CCapH323::GetMinAudioFramePerPacket(CapEnum audioType) const
{
	int framePerPacket = 0;
	CCapSetInfo capInfo = audioType;
	if(capInfo.IsType(cmCapAudio)&&OnCap(audioType))
	{
		capBuffer *pCapBuffer = (capBuffer *) &m_pCap->caps;
		char      *tempPtr = (char*)pCapBuffer;

		for(int i = 0; i < m_numOfCaps; i++)
		{
			BYTE isGeneralG7221C_Cap = FALSE;
			if(pCapBuffer->capTypeCode == eG7221C_CapCode)
			{
				CG7221CAudioCap * pAudioCap = (CG7221CAudioCap *)CBaseCap::AllocNewCap(eG7221C_CapCode, pCapBuffer->dataCap);
				if (pAudioCap)
					isGeneralG7221C_Cap = pAudioCap->isCapEnumSupported(audioType);
				POBJDELETE(pAudioCap);
			}

			if ((pCapBuffer->capTypeCode == audioType) || isGeneralG7221C_Cap)
			{
				CBaseAudioCap* pAudioCap = (CBaseAudioCap *)CBaseCap::AllocNewCap(audioType,pCapBuffer->dataCap);
				if (pAudioCap)
				{
					// takes the higher frames per packet for this alg
					int currentFramePerPacket = pAudioCap->GetMinFramePerPacket();
					PTRACE2INT(eLevelInfoNormal,"CCapH323::GetMinAudioFramePerPacket: Set struct has failed ",currentFramePerPacket);
					if (currentFramePerPacket > 0)
						framePerPacket = min(framePerPacket,currentFramePerPacket);
				}
				POBJDELETE(pAudioCap);
			}

			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal,
			"CCapH323::GetMinAudioFramePerPacket: not an audio algorithm or not in capset - ",capInfo.GetH323CapName());
	}
	return framePerPacket;
}

////////////////////////////////////////////////////////////////////////////////////////////
// if eDirection is cmCapReceiveAndTransmit it will calculate all of the cap sets!!!
int CCapH323::GetFormatMpiAndLocation(CapEnum videoType, EFormat eFormat, cmCapDirection eDirection,
						   APIS8& formatMpi) const
{
	formatMpi = -1;
	int capNum = -1;
	CCapSetInfo capInfo = videoType;
	if(capInfo.IsType(cmCapVideo)&&OnCap(videoType))
	{
		capBuffer *pCapBuffer = (capBuffer *) &m_pCap->caps;
		char      *tempPtr = (char*)pCapBuffer;

		int i = 0;
		for(i = 0; i < m_numOfCaps; i++)
		{
			if (pCapBuffer->capTypeCode == videoType)
			{
				CBaseVideoCap *pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(videoType,pCapBuffer->dataCap);
				if (pVideoCap)
				{
					// Check if the direction is what we wanted
					if (eDirection == cmCapReceiveAndTransmit || pVideoCap->IsDirection(eDirection))
					{
						// takes the higher frame rate for this format
						APIU8 currentFormatMpi = pVideoCap->GetFormatMpi(eFormat);
						if (currentFormatMpi > 0)
						{
							if (formatMpi > 0) // not the first cap
							{
								//formatMpi = min(formatMpi,currentFormatMpi);
								if (formatMpi < currentFormatMpi)
								{
									formatMpi = currentFormatMpi;
									capNum = i;
								}
							}
							else // the first
							{
								formatMpi = currentFormatMpi;
								capNum = i;
							}
						}
					}
				}
				POBJDELETE(pVideoCap);
			}

			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal,
			"CCapH323::GetFormatMpi: %s is not a video algorithm or not in capset!!!",capInfo.GetH323CapName());
	}
	return capNum;
}

////////////////////////////////////////////////////////////////////////////////////////////
//Descriptions: Get the struct of the capset with the "best" abilities
// if eDirection is cmCapReceiveAndTransmit it will calculate all of the cap sets!!!
const BaseCapStruct* CCapH323::GetBestCapStruct(CapEnum videoType, cmCapDirection eDirection, EFormat minFormat, ERoleLabel eRole, BYTE bCheckProfile) const
{//right now we do it only for video
	CCapSetInfo capInfo = videoType;

	BaseCapStruct* pBestVideoSruct = NULL;

	if( capInfo.IsType(cmCapVideo) && OnCap(videoType) )
	{
		int capNum = -1;

		capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
		char* tempPtr = (char*)pCapBuffer;

		if (videoType == eH264CapCode)
			capNum = GetBestH264CapStructNum(eDirection, eRole, bCheckProfile);
		else
		{
			EFormat maxFormat = kUnknownFormat;
			BYTE   bMaxHasAnnex = FALSE;

			if (minFormat != kUnknownFormat)
			{
				if (IsVidImageFormat(videoType, minFormat))
				{
					maxFormat = minFormat;
					APIS8 tmpFormatMpi;
					capNum = GetFormatMpiAndLocation(videoType, minFormat, eDirection, tmpFormatMpi);
				}
				else
					return NULL;
			}

			capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
			char* tempPtr = (char*)pCapBuffer;

			for(int i = 0; i < m_numOfCaps; i++)
			{
				if (pCapBuffer->capTypeCode == videoType)
				{
					CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(videoType,pCapBuffer->dataCap);
					if (pVideoCap)
					{
						// Check if the direction is what we wanted
						BYTE bIsRoleCorrect = (CheckRole(eRole, i, pVideoCap));
						if(bIsRoleCorrect)
						{
							if (eDirection == cmCapReceiveAndTransmit || pVideoCap->IsDirection(eDirection))
							{
								if (pVideoCap->IsThisCapBetter(&maxFormat, &bMaxHasAnnex))
									capNum = i;
							}
						}
					}
					POBJDELETE(pVideoCap);
				}

				tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
				pCapBuffer = (capBuffer*)tempPtr;
			}
		}

		if (capNum != -1)
		{
			//now we need to get the CBaseVideoCap of capNum
			pCapBuffer = (capBuffer *) &m_pCap->caps;
			tempPtr = (char*)pCapBuffer;

			for(int j = 0; j < capNum; j++)
			{
				tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
				pCapBuffer = (capBuffer*)tempPtr;
			}

			CBaseVideoCap* pCurrentCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(videoType, pCapBuffer->dataCap);
			if (pCurrentCap)
				pBestVideoSruct = pCurrentCap->GetStruct();
			POBJDELETE(pCurrentCap);
		}
		else
			PTRACE(eLevelInfoNormal,	"CCapH323::GetBestCapStruct: cap struct wasn't found");
	}
	else
		PTRACE2(eLevelInfoNormal,	"CCapH323::GetBestCapStruct: %s is not a video algorithm or not in capset!!!",capInfo.GetH323CapName());

	return pBestVideoSruct;
}

////////////////////////////////////////////////////////////////////////////////////////////
// if eDirection is cmCapReceiveAndTransmit it will calculate all of the cap sets!!!
int CCapH323::GetBestH264CapStructNum(cmCapDirection eDirection, ERoleLabel eRole, BYTE bCheckProfile) const
{
	int   capNum      = -1;
	APIU16 maxProfile  = H264_Profile_BaseLine; //the lowest profile
	APIU8 maxLevel    = H264_Level_1; //the lowest level
	APIS32 maxMbps     = -1;
	APIS32 maxFs       = -1;
	APIS32 maxDpb      = -1;

	APIS32 maxBrAndCpb = -1;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char* tempPtr = (char*)pCapBuffer;

	for(int i = 0; i < m_numOfCaps; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			//if (capNum == -1) //if this is the first h264 cap struct in the capabilities
			//	capNum = i;

			CH264VideoCap* pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode,pCapBuffer->dataCap);
			if (pVideoCap)
			{
				// Check if the direction is what we wanted
				if ((eDirection == cmCapReceiveAndTransmit || pVideoCap->IsDirection(eDirection)) && CheckRole(eRole,i,pVideoCap))
				{
					if (pVideoCap->IsThisCapBetter(&maxProfile, &maxLevel, &maxMbps, &maxFs, &maxDpb, &maxBrAndCpb, bCheckProfile))
						capNum = i;
				}
			}
			POBJDELETE(pVideoCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

	return capNum;
}


////////////////////////////////////////////////////////////////////////////////////////
const capBuffer* CCapH323::GetFirstMediaCapBufferAccording2CapEnum(CapEnum protocol, ERoleLabel eRole) const
{//right now we do it only for video

	int capNum = -1;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char* tempPtr = (char*)pCapBuffer;

	for (int i = 0; (i < m_numOfCaps) && (capNum == -1); i++)
	{
		CBaseCap* pBaseCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);
		if (pBaseCap)
		{
			if(pBaseCap->GetCapCode() == protocol)
			{
				BYTE bIsRoleCorrect = CheckRole(eRole, i, pBaseCap);
				if (bIsRoleCorrect)
					capNum = i;
			}
		}
		POBJDELETE(pBaseCap);

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

	if (capNum != -1)
	{
		pCapBuffer = (capBuffer *) &m_pCap->caps;
		tempPtr = (char*)pCapBuffer;

		for (int j = 0; j < capNum; j++)
		{
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
		return pCapBuffer;
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CCapH323::GetFirstMediaCapBufferAccording2CapEnum: cap struct wasn't found");
		return NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
const capBuffer* CCapH323::GetFirstMediaCapBuffer(cmCapDataType dataType, ERoleLabel eRole) const
{//right now we do it only for video

	int capNum = -1;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char* tempPtr = (char*)pCapBuffer;
	CapEnum curProtocol;

	for (int i = 0; (i < m_numOfCaps) && (capNum == -1); i++)
	{
		curProtocol = (CapEnum)pCapBuffer->capTypeCode;
		CCapSetInfo capInfo = curProtocol;
		if (capInfo.IsType(dataType))
		{
			CBaseCap* pCurCap = CBaseCap::AllocNewCap(curProtocol, pCapBuffer->dataCap);
			if (pCurCap)
			{
				BYTE bIsRoleCorrect = CheckRole(eRole, i, pCurCap);
				if (bIsRoleCorrect)
					capNum = i;
			}
			POBJDELETE(pCurCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

	if (capNum != -1)
	{
		pCapBuffer = (capBuffer *) &m_pCap->caps;
		tempPtr = (char*)pCapBuffer;

		for (int j = 0; j < capNum; j++)
		{
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
		return pCapBuffer;
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CCapH323::GetFirstMediaStruct: cap struct wasn't found");
		return NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCapH323::GetFirstMaxVideoBitRate()
{
	const capBuffer* pCapBuffer = GetFirstMediaCapBuffer(cmCapVideo,kRolePeople);
	DWORD vidBitRate = 0;
	if (pCapBuffer)
		vidBitRate = GetMaxVideoBitRate((CapEnum)pCapBuffer->capTypeCode, cmCapReceive,kRolePeople);

	return vidBitRate;
}

////////////////////////////////////////////////////////////////////////////////////////////
/*WORD CCapH323::SetVideoCapsFromH320Scm(const CComMode &scm, DWORD videoRate)
{
	WORD numOfCaps = 0;
	CComModeInfo cmInfo(scm.m_vidMode.GetVidMode(), StartVideoCap);
	CapEnum      h323ModeType = cmInfo.GetH323ModeType();
	CBaseVideoCap * pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(h323ModeType,NULL);

	if (pVideoCap)
	{
		DWORD bitRateToSet = videoRate / 800;

		if (scm.m_vidMode.IsFreeBitRate() == VSW)
		{
			DWORD videoBitRateForVSW = 0;
			DWORD contentBitRate     = 0;
			DWORD temp               = 0;
			CComMode * pScm = const_cast<CComMode *>(&scm);
			pScm->GetMediaBitrate(temp,videoBitRateForVSW,temp,temp,temp,temp,temp);
			videoBitRateForVSW += contentBitRate; // max video bit rate (people + content)
			bitRateToSet = videoBitRateForVSW /800;
		}

		EResult eResOfSet = kSuccess;
		eResOfSet &= pVideoCap->SetFromComMode(scm);
		eResOfSet &= pVideoCap->SetBitRate(bitRateToSet);

		int structSize = pVideoCap->SizeOf();
		if (eResOfSet && structSize)
		{
			AddCapToCapBuffer(h323ModeType,structSize,(char*)pVideoCap->GetStruct());
			m_capArray[h323ModeType]++;
			numOfCaps++;
			m_numOfCaps++;
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH320Scm: Set struct has failed");
	}

	pVideoCap->FreeStruct();
	POBJDELETE(pVideoCap);

	return numOfCaps;
}
*/

/////////////////////////////////////////////////////////////////////////////
void CCapH323::Dump(const char* title, WORD level) const
{
	COstrStream msg;
    if(title != NULL)
        msg << title;
	Dump(msg);
	PTRACE(level,msg.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::Dump(std::ostream & msg) const
{
	WORD        msgBufSize = 8192;
	int			i;

	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n";

	msg <<"No. of Sim:"<< (DWORD)m_pCap->numberOfSim << "\n";
	msg <<"No. of alt:"<< (DWORD)m_pCap->numberOfAlts << "\n";
	msg <<"No. of caps::"<< m_numOfCaps << "\n";
	msg <<"No. of people alt::"<< m_peopleAltNumber << "\n";
	msg <<"No. of content alt::"<< m_contentAltNumber << "\n";
	msg <<"is H239::"<< m_bIsH239 << "\n";
	msg <<"is EPC::"<< m_bIsEPC << "\n";
	msg << "is m_bIsDPTR::"<<m_bIsDPTR<<"\n";
	msg << "\n----- ";
	msg <<"\n altMatrix = ";
	for( i = 0; i < FD_SET_SIZE; i++)
		msg << "(" << (std::hex) << m_pCap->altMatrix.fds_bits[i] << "), ";

	msg << (std::dec);
	capBuffer *pCapBuffer = (capBuffer *) &m_pCap->caps;
	char      *tempPtr = (char*)pCapBuffer;

	for(i = 0; i < m_numOfCaps; i++)
	{
		CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;
		msg << "\n" << i+1;
		msg << " ----- ";
		capInfo.Dump(msg);
        msg << "\nalt num: " << FindAltNumber(i, (CapEnum)pCapBuffer->capTypeCode) << "\n";
		msg << " -----\n";

		CBaseCap* pCap = CBaseCap::AllocNewCap(capInfo, pCapBuffer->dataCap);
		if (pCap)
			pCap->Dump(msg);

		POBJDELETE(pCap);


		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	msg << "-------------------------------\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::GetCurrentCapsBufSize() const
{
	return(m_size - sizeof(ctCapabilitiesBasicStruct));
}

//////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::SetFeccCap(const CComModeH323* pScm, CapEnum dataType)
{
	WORD        numOfCaps  = 0;
	CCapSetInfo capInfo    = dataType;
	BYTE        bSupported = capInfo.IsSupporedCap();

	if (capInfo.IsType(cmCapData) && bSupported)
	{
		CH224DataCap* pH224DataCap = (CH224DataCap *)CBaseCap::AllocNewCap(dataType,NULL);

		if (pH224DataCap)
		{
			EResult eResOfSet = pH224DataCap->SetFromIpScm(pScm);
			if (eResOfSet)
			{
				DWORD rate = pH224DataCap->GetBitRate();
				if (pH224DataCap->GetBitRate())
				{
					int structSize = pH224DataCap->SizeOf();
					AddCapToCapBuffer(dataType, structSize, pH224DataCap->GetStruct());
					m_capArray[dataType]++;
					m_numOfCaps++;
					numOfCaps++;
					m_dataMaxBitRate = (m_dataMaxBitRate > rate) ? m_dataMaxBitRate : rate;  // in 100 bit/sec
				}
			}
			else
				PTRACE(eLevelInfoNormal,"CCapH323::SetFeccCap: there is no feec in the conference.");

			pH224DataCap->FreeStruct();
			POBJDELETE(pH224DataCap);
		}

	}

	return numOfCaps;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::SetAudioCap(CapEnum audCapType)
{
	WORD        numOfCaps  = 0;
	CCapSetInfo capInfo    = audCapType;
    BYTE        bSupported = capInfo.IsSupporedCap();

    if (capInfo.IsType(cmCapAudio) && bSupported)
	{
		CBaseAudioCap* pAudioCap = (CBaseAudioCap *)CBaseCap::AllocNewCap(audCapType,NULL);
		if (pAudioCap)
		{
			EResult eResOfSet = kSuccess;
			int framePerPacket = capInfo.GetMaxFramePerPacket();
			eResOfSet &= pAudioCap->SetStruct(cmCapReceive,framePerPacket);
			int structSize = pAudioCap->SizeOf();
			if(audCapType == eG728CapCode)
				pAudioCap->SetMinFramePerPacket(4);
			if(eResOfSet && structSize)
			{
				AddCapToCapBuffer(audCapType,structSize,pAudioCap->GetStruct());
				m_capArray[audCapType]++;
				numOfCaps++;
				m_numOfCaps++;
			}
			else
				PTRACE(eLevelInfoNormal,"CCapH323::SetAudioCap: Set struct has failed");

			pAudioCap->FreeStruct();
			POBJDELETE(pAudioCap);
		}

	}

	return numOfCaps;
}

///////////////////////////////////////////////////////////////////////////////////////////////
capBuffer *CCapH323::GetAudioMatch(CCapH323* pRmtCap) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::GetAudioMatch");

	capBuffer	*pLocalCapBuffer;
	capBuffer	*pRmtCapBuffer;
	char		*pTempPtr;
	APIU8		localType;
	APIU8		rmtType;
	APIU8		rmtDirection;

	pLocalCapBuffer	= (capBuffer *) &m_pCap->caps;

	for(int j=0; j < m_numOfCaps; j++)
	{
		localType	= ((ctCapStruct*)(pLocalCapBuffer->dataCap))->type;

		pRmtCapBuffer = (capBuffer *)&pRmtCap->m_pCap->caps;

		for(int i=0; i < pRmtCap->m_numOfCaps; i++)
		{
			rmtType			= ((ctCapStruct*)pRmtCapBuffer->dataCap)->type;
			rmtDirection	= ((ctCapStruct*)pRmtCapBuffer->dataCap)->direction;

			if( (localType == cmCapAudio)		&&
				(rmtType == cmCapAudio)			&&
				(rmtDirection & cmCapReceive)	&&
				(pRmtCapBuffer->capTypeCode == pLocalCapBuffer->capTypeCode))
			{
//				PTRACE2INT2(eLevelInfoNormal,"CCapH323::GetAudioMatch:  %d",pLocalCapBuffer->capTypeCode);
				return pLocalCapBuffer;
			}

			pTempPtr = (char*)pRmtCapBuffer;
			pTempPtr += sizeof(capBufferBase) + pRmtCapBuffer->capLength;
			pRmtCapBuffer = (capBuffer*)pTempPtr;
		}
		pTempPtr = (char*)pLocalCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
		pLocalCapBuffer = (capBuffer*)pTempPtr;
	}

	PTRACE(eLevelInfoNormal,"CCapH323::GetAudioMatch: NO MATCH");
	return NULL; //No mutch
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Ensure the initiate audio mode match to the remote cap. We shouldn't be hard with the audio caps.
void CCapH323::EnsureAudioTargetMode(CAudModeH323 &pXmitMode323,CCapH323* pRmtCap) const
{
	capBuffer	*pMatchCapBuffer	= NULL;
	capBuffer	*pRmtCapBuffer		= (capBuffer *) &pRmtCap->m_pCap->caps;

	char		*pTempPtr;

	//find if the remote has the audio initiate mode
	for(int i=0; i < pRmtCap->m_numOfCaps; i++)
	{
		if((pXmitMode323.GetType() == pRmtCapBuffer->capTypeCode) &&
			(((ctCapStruct*)pRmtCapBuffer->dataCap)->direction & cmCapReceive))
			return;
		// check if its G7221C
		BYTE isGeneralG7221C_Cap = FALSE;
		if(pRmtCapBuffer->capTypeCode == eG7221C_CapCode)
		{
			CG7221CAudioCap * pAudioCap = (CG7221CAudioCap *)CBaseCap::AllocNewCap(eG7221C_CapCode, pRmtCapBuffer->dataCap);
			if (pAudioCap)
				isGeneralG7221C_Cap = pAudioCap->isCapEnumSupported((CapEnum)pXmitMode323.GetType());
			POBJDELETE(pAudioCap);
			if(isGeneralG7221C_Cap)
				return;
		}


		pTempPtr = (char*)pRmtCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pRmtCapBuffer->capLength;
		pRmtCapBuffer = (capBuffer*)pTempPtr;

	}

	//if we get out from the loop it means that we did not found match.
	//We need to find match between the remote and local and put it in the initiate mode.
	pMatchCapBuffer = GetAudioMatch(pRmtCap);
	if(pMatchCapBuffer)
		pXmitMode323.Create(pMatchCapBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Ensure the initiate Video protocol exist at the remote cap.
void CCapH323::EnsureVideoTargetMode(CVidModeH323 &pXmitMode323,CCapH323* pRmtCap) const
{
	if(pXmitMode323.IsMediaOn())
	{// check if remote has H264
		CapEnum targetCapCode = (CapEnum)pXmitMode323.GetType();
		if(targetCapCode == eH264CapCode)
		{
			if(pRmtCap->OnCap(eH264CapCode) == FALSE)
			{
				if(OnCap(eH263CapCode) && pRmtCap->OnCap(eH263CapCode))
				{
					const capBuffer *pCapBuffer =  GetFirstMediaCapBufferAccording2CapEnum(eH263CapCode);
					if(pCapBuffer !=  NULL)
						pXmitMode323.Create(pCapBuffer);
				}
				else if(OnCap(eH261CapCode) && pRmtCap->OnCap(eH261CapCode))
				{
					const capBuffer *pCapBuffer =  GetFirstMediaCapBufferAccording2CapEnum(eH261CapCode);
					if(pCapBuffer !=  NULL)
						pXmitMode323.Create(pCapBuffer);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Ensure the initiate Video protocol exist at the remote cap.
void CCapH323::EnsureContentTargetMode(CVidModeH323 &pXmitMode323,CCapH323* pRmtCap) const
{
	BYTE isCommonMode = FALSE;
	PTRACE(eLevelInfoNormal,"CCapH323::EnsureContentTargetMode");
	if(pXmitMode323.IsMediaOn())
	{// check if remote has H264
		if(pRmtCap->IsH239() == FALSE && pRmtCap->IsEPC() == FALSE)
			isCommonMode = FALSE;
		else
		{
			CapEnum targetCapCode = (CapEnum)pXmitMode323.GetType();
			if(targetCapCode == eH264CapCode)
			{
				APIS32 H264mode   = H264_standard;
				if (pXmitMode323.IsTIPContentEnableInH264Scm() == TRUE)
					H264mode = H264_tipContent;
				if(pRmtCap->AreCapsSupportProtocol(targetCapCode, cmCapVideo, kRoleContentOrPresentation, H264mode) == TRUE)
					isCommonMode = TRUE;
			}
			if(isCommonMode == FALSE)
			{

				ERoleLabel eRole = kRoleContentOrPresentation;
				if(pRmtCap->IsH239() == FALSE && pRmtCap->IsEPC() != FALSE)
				{
					eRole = kRoleContent;
					PTRACE(eLevelInfoNormal,"CCapH323::EnsureContentTargetMode - role content");
				}


				if(AreCapsSupportProtocol(eH263CapCode, cmCapVideo, eRole) && pRmtCap->AreCapsSupportProtocol(eH263CapCode, cmCapVideo, eRole))
				{
					//PTRACE(eLevelInfoNormal,"CCapH323::EnsureContentTargetMode caps support h263");
					isCommonMode = TRUE;
					const capBuffer *pCapBuffer =  GetFirstMediaCapBufferAccording2CapEnum(eH263CapCode, eRole);
					if((pCapBuffer !=  NULL) && (targetCapCode == eH264CapCode || eRole == kRoleContent))// if the target mode was H263 presentation no need to re-set it to H263 again.
					{
						PTRACE(eLevelInfoNormal,"CCapH323::EnsureContentTargetMode - re-set content mode");
						pXmitMode323.Create(pCapBuffer);
					}
				}
				else
				{
					PTRACE(eLevelInfoNormal,"CCapH323::EnsureContentTargetMode caps not support h263");
					isCommonMode = FALSE;
				}
			}
		}
	}

	if(isCommonMode == FALSE)
		pXmitMode323.SetModeOff();
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Ensure the initiate Video protocol exist at the remote cap.
void CCapH323::EnsureDataTargetMode(CDataModeH323 &pXmitMode323,CCapH323* pRmtCap) const
{
	if(pXmitMode323.IsMediaOn())
	{// check if remote has H264
		CapEnum targetCapCode = (CapEnum)pXmitMode323.GetType();
		if(targetCapCode == eAnnexQCapCode)
		{
			if(pRmtCap->OnCap(eAnnexQCapCode) == FALSE)
			{
				if(OnCap(eRvFeccCapCode) && pRmtCap->OnCap(eRvFeccCapCode))
				{
					const capBuffer *pCapBuffer =  GetFirstMediaCapBufferAccording2CapEnum(eRvFeccCapCode);
					if(pCapBuffer !=  NULL)
					{
						pXmitMode323.Create(pCapBuffer);
						PTRACE(eLevelInfoNormal,"CCapH323::EnsureDataTargetMode: FECC is set to eRvFeccCapCode");
					}
					else
						PTRACE(eLevelInfoNormal,"CCapH323::EnsureDataTargetMode: can't get FECC from eRvFeccCapCode type");
				}
			}
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::EnsureDataTargetMode: FECC is already set to eAnnexQCapCode");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
//In case of auto audio the conference scm is nullify, I need to update the target via the first audio cap
void	CCapH323::UpdateTargetMode(CAudModeH323 &pXmitMode323) const
{
	capBuffer	*pLocalCapBuffer = (capBuffer *) &m_pCap->caps;

	char		*pTempPtr;
	APIU8		type;

	for(int k=0; k < m_numOfCaps; k++)
	{
		type = ((ctCapStruct*)(pLocalCapBuffer->dataCap))->type;
		if(type == cmCapAudio)
		{
			pXmitMode323.Create(pLocalCapBuffer);
			return;
		}

		pTempPtr = (char*)pLocalCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
		pLocalCapBuffer = (capBuffer*)pTempPtr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::ImproveTargetMode(CVidModeH323 &pXmitMode323,cmCapDirection direction) const
{
	if (pXmitMode323.IsModeOff())
	{
		PTRACE(eLevelError,"CCapH323::ImproveTargetMode - media is off");
		return;
	}

	PTRACE(eLevelInfoNormal,"CCapH323::ImproveTargetMode ");
	// We are running here in the context of the local Cap!!!!!!!!!!!!

	capBuffer		*pLocalCapBuffer	= (capBuffer *) &m_pCap->caps;
	APIU8			xmitModeLabel		= ((BaseCapStruct*)pXmitMode323.GetDataCap())->header.roleLabel;
	APIU8			xmitModeType		= pXmitMode323.GetType();
	char			*pTempPtr;
	CVidModeH323	tmpVidMode;

	for(int k=0; k < m_numOfCaps; k++)
	{
		APIU8 localCapLabel		= ((BaseCapStruct*)pLocalCapBuffer->dataCap)->header.roleLabel;
		APIU8 localCapDirection = ((BaseCapStruct*)pLocalCapBuffer->dataCap)->header.direction;


		if((xmitModeType  == pLocalCapBuffer->capTypeCode) && (xmitModeLabel == localCapLabel))
		{
			//In case we found the exact direction we can get out from the function, else we save the mode and if we
			//do not find any cap that fit to the direction we should take the other one.
			//There are cases that there are no transmit caps only receive and the meaning is for the transmit.
			if(localCapDirection & direction)
			{
				pXmitMode323.Create(pLocalCapBuffer);
				return;
			}
			else
				tmpVidMode.Create(pLocalCapBuffer);
		}

		pTempPtr = (char*)pLocalCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
		pLocalCapBuffer = (capBuffer*)pTempPtr;
	}

	pXmitMode323 = tmpVidMode;

	return;
}

//////////////////////////////////////////////////////////////////
void CCapH323::ReleaseLinkedList(candidateList **ppHead) const
{
	candidateList* pNext = *ppHead;
	candidateList* pCurrent;

	while(pNext != NULL)
	{
		pCurrent = pNext;
		pNext = pNext->pNextCap;
		PDELETE(pCurrent->pCapPtr);
		PDELETE(pCurrent);
	}

	*ppHead = NULL;
}

//////////////////////////////////////////////////////////////////
void CCapH323::AddNewEntry(CVidModeH323 *pCapBuffer,candidateList **ppHead) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::AddNewTempEntry ");

 	candidateList  *pNewCap	= new candidateList;
	pNewCap->pNextCap		= NULL;
	pNewCap->pCapPtr		= new CVidModeH323(*((const CVidModeH323 *)pCapBuffer));

	if(*ppHead == NULL) // the first
	{
		*ppHead = pNewCap;
		pNewCap->count = 1;
	}
	else
	{
		candidateList  *pHead	= *ppHead;
		while( pHead->pNextCap != NULL)
			pHead = pHead->pNextCap;
		pHead->pNextCap = pNewCap;
		pHead->count	= pHead->count++;
	}

	return;
}

//////////////////////////////////////////////////////////////////
CMediaModeH323 *CCapH323::CheckTheBestCap(candidateList &pHead, int type) const
{
	candidateList	*pCurrentCap		= &pHead;
	CMediaModeH323	*pOneMod			= NULL;
	CMediaModeH323	*pSecondMod			= NULL;
	BYTE			rval				= 0;
	DWORD			details				= 0x00000000;

	if(pCurrentCap != NULL)
	{
		pOneMod		= pCurrentCap->pCapPtr;
		pCurrentCap	= pCurrentCap->pNextCap;
		if(pCurrentCap != NULL)
			pSecondMod	= pCurrentCap->pCapPtr;
	}

	while(pSecondMod != NULL)
	{
		rval = pOneMod->IsContaining(*pSecondMod,kFormat|kFrameRate|kAnnexes|kH264Level|kH264Additional|kBitRate,&details);

		if(!rval)//The second is not containing in the one so move on - second become one and the next become second
			pOneMod	= pSecondMod;

		pCurrentCap = pCurrentCap->pNextCap;
		if(pCurrentCap != NULL)
			pSecondMod	= pCurrentCap->pCapPtr;
		else
			pSecondMod = NULL;
	}

	return pOneMod;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int	CCapH323::LookVideo(BYTE audiosim,CComModeH323 *pInitial323,int altNumber,ERoleLabel label,int confKind) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookVideo");

	int				type			= pInitial323->GetMediaType(cmCapVideo,cmCapTransmit,label);
	capMatchList	*pCurrentCap	= NULL;

	candidateList	*pCandidateList	= NULL; //We hold the list of the candidate to union
	CVidModeH323	*pCandidateCap	= NULL;

	if (type < MAX_CAP_TYPES)
	{
		pCurrentCap	= m_capMatchList[type];
	}
	else
	{
		PASSERT(type);
	}

	while(pCurrentCap != NULL)
	{
		CComModeH323 tempInitiate	= *pInitial323;  //VNGFE-7531
		if((pCurrentCap->sim == audiosim) && (pCurrentCap->altNumber == altNumber))
		{
			switch(type)
			{
				case eH261CapCode:
					pCandidateCap	= LookForMatchingH261Pair(&tempInitiate,pCurrentCap->pCapPtr,label,confKind);
					break;

				case eH263CapCode:
				{
					pCandidateCap	= LookForMatchingH263Pair(&tempInitiate,pCurrentCap->pCapPtr,label,confKind);
					break;
				}

				case eH264CapCode:
					pCandidateCap	= LookForMatchingH264Pair(&tempInitiate,pCurrentCap->pCapPtr,label,confKind);
					break;

				case eH26LCapCode:
					pCandidateCap	= LookForMatchingH26LPair(&tempInitiate,pCurrentCap->pCapPtr,label,confKind);
					break;

				default:
					PTRACE(eLevelInfoNormal,"CCapH323::LookVideo - Unknown video type.");
			}

			if(pCandidateCap)
				AddNewEntry(pCandidateCap,&pCandidateList);
		}

		//POBJDELETE(pTempInitiate);
		pCandidateCap = NULL;
		pCurrentCap = pCurrentCap->pNextCap;
	}

	CVidModeH323 *pBestCap = NULL;
	if (pCandidateList)
		pBestCap = (CVidModeH323 *)CheckTheBestCap(*pCandidateList,type);

	if(pBestCap != NULL)
	{
		//pInitial323->SetMediaMode(type,pBestCap->SizeOf(),pBestCap->GetDataCap(),cmCapVideo,cmCapReceiveAndTransmit,label);
		pInitial323->SetMediaMode(type, pBestCap->SizeOf(), pBestCap->GetDataCap(), cmCapVideo, cmCapTransmit, label);
		ReleaseLinkedList(&pCandidateList);
		return 1;
	}
	else
	{
		//we dont have rmt cap that match to our initial (even we tried to change our initial)
		PTRACE(eLevelInfoNormal,"CCapH323::LookVideo - Not found");
		ReleaseLinkedList(&pCandidateList);
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
int	CCapH323::LookForVideoPair(BYTE audiosim,CComModeH323 *pInitial323,int confKind) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForVideoPair");
	int	    rval					= 0;
	BYTE	bIsFoundVidPeopleMatch	= FALSE;
	BYTE	bIsFoundVidContentMatch = FALSE;
	BYTE	bIsExplicit				= IsExplicit();
	BYTE	bCheckContent			= pInitial323->IsMediaOn(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation);
	BYTE	bIsEPC;
	BYTE	bIsH239;
	ERoleLabel eRole = kRolePeople;

	//Check if there is content.
	bIsEPC = (m_bIsEPC && bCheckContent);
	bIsH239 = (bCheckContent && m_bIsH239);
	if(!bIsH239 || !bIsEPC) //in case there in no content we shouldn't search for content matching.
		bIsFoundVidContentMatch = TRUE;

	bIsFoundVidPeopleMatch = LookVideo(audiosim,pInitial323,m_peopleAltNumber,kRolePeople,confKind);
	if(bIsH239)
	{
		bIsExplicit = TRUE;
		eRole = kRolePresentation;
	}
	else if(bIsEPC) //in case there is a content
	{
		if(bIsExplicit)
			eRole = kRoleContent;
	}

	if(!bIsFoundVidContentMatch)
	{
		if(bIsExplicit) //there is an alt for people and content
		{
			PTRACE(eLevelInfoNormal,"CCapH323::LookForVideoPair - explicit");
			bIsFoundVidContentMatch = LookVideo(audiosim,pInitial323,m_contentAltNumber,eRole,confKind);
		}
		else//the same alt is for people and content
		{
			PTRACE(eLevelInfoNormal,"CCapH323::LookForVideoPair - implicit");
			bIsFoundVidContentMatch = LookVideo(audiosim,pInitial323,m_peopleAltNumber,eRole,confKind);
		}
	}

	//We should return the success of finding match for people and content.
	rval = bIsFoundVidPeopleMatch & bIsFoundVidContentMatch;
    return rval;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int	CCapH323::LookForDataPair(BYTE audiosim,CComModeH323 *pInitial323) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForDataPair ");

	const CDataModeH323 &dataMode	= (const CDataModeH323 &)pInitial323->GetMediaMode(cmCapData,cmCapTransmit);
	capMatchList	*pCurrentCap	= NULL;
	WORD type = dataMode.GetType();

	if (type < MAX_CAP_TYPES)
	{
		pCurrentCap	= m_capMatchList[type];
	}
	else
	{
		PASSERT(type);
	}
	
	while(pCurrentCap != NULL)
	{
		if(pCurrentCap->sim == audiosim)
		{
			int maxBitRate = min(dataMode.GetBitRate(),pInitial323->GetMediaBitRate(cmCapData,cmCapTransmit));
			pInitial323->SetDataBitRate(maxBitRate,cmCapTransmit);
			PTRACE2(eLevelInfoNormal,"CCapH323::LookForDataPair - data algoritem found ", CapEnumToString((CapEnum)dataMode.GetType()));
			return 1;
		}
		pCurrentCap = pCurrentCap->pNextCap;
	}

	return 0; //If get out from the loop it means that we do not found T120 that match to our initial.
}

///////////////////////////////////////////////////////////////////////////////////////////////
BYTE	CCapH323::FindMatching(CComModeH323 *pTargetModeH323,WORD confKind,BYTE bIsDataOnly,BYTE bIsCheckVideoOnly) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::FindMatching ");

	BYTE audiosim   = 0;
	int audioFound	= -1;
	int videoFound	= -1;
	int dataFound	= -1;

	if (bIsCheckVideoOnly)
	{ // We only need to find the video pair
		PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - Video only case");
		CVidModeH323 *pVidMode		= &(CVidModeH323 &)pTargetModeH323->GetMediaMode(cmCapVideo,cmCapTransmit);
		capMatchList *pCurrentCap	= NULL;
		WORD type = pVidMode->GetType();

		if (type < MAX_CAP_TYPES)
		{
			pCurrentCap	= m_capMatchList[type];
		}
		else
		{
			PASSERT(type);
		}
	
		if (pCurrentCap == NULL)
		{
			PTRACE(eLevelInfoNormal,"CCapH323::FindMatching (video only) - m_capMatchList has no matching video cap");
			return NOCAP;
		}

		if(pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit))
			return NOCAP;

		videoFound = LookForVideoPair(pCurrentCap->sim,pTargetModeH323,confKind);
		if(videoFound > 0)
		{
			PTRACE(eLevelInfoNormal,"CCapH323::FindMatching (video only) - video found ");
			return FOUND_VIDEO;
		}
		else
		{
			pTargetModeH323->SetMediaOff(cmCapVideo,cmCapTransmit);
			if(videoFound == -1)
			//we did not find match between the remote and the target mode, so we nullify it and will'l try with the local cap.
			//If we found any local cap that match to the remote cap I will initiate the target.
				PTRACE(eLevelInfoNormal,"CCapH323::FindMatching (video only) - there is no video in the initial ");
			else//(videoFound == 0)
				PTRACE(eLevelInfoNormal,"CCapH323::FindMatching (video only) - video algoritem not found ");
			return NOCAP;
		}
	}


	CAudModeH323 *pAudMode		= &(CAudModeH323 &)pTargetModeH323->GetMediaMode(cmCapAudio,cmCapTransmit);
	capMatchList *pCurrentCap = NULL;
	if(pAudMode->IsValidMode())
		pCurrentCap = m_capMatchList[pAudMode->GetType()];
	

	int firstAudio = -1;
	while(pCurrentCap != NULL)
	{
		audiosim				= pCurrentCap->sim;
		audioFound				= 1;

		PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - audio algorithm found ");
		//In case we need to check only data we should get out from the loop here
		if(bIsDataOnly)
			break;

		// now look for video cap at the same sim as we found the audio cap.
		if(pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit))
			break;

		videoFound = LookForVideoPair(audiosim,pTargetModeH323,confKind);
		if(videoFound)
			break;
		pCurrentCap = pCurrentCap->pNextCap;
	}

   	if(audioFound > 0)
	{
		CDataModeH323& rDataXmit = (CDataModeH323&)pTargetModeH323->GetMediaMode(cmCapData,cmCapTransmit);
		if(rDataXmit.GetDataCap())
			dataFound = LookForDataPair(audiosim,pTargetModeH323);
	}

	//Check if the initial has the correct mode
	if(audioFound == -1)
	{
		PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - audio algorithm not found ");
		return NOCAP;
	}

	if(bIsDataOnly)
	{
		if(dataFound > 0)
		{
			PTRACE(eLevelInfoNormal,"CCapH323::FindMatching (data only) - data found ");
			return FOUND_DATA;
		}
		else
		{
			pTargetModeH323->SetMediaOff(cmCapData,cmCapTransmit);
			if(dataFound == -1)
			//we did not find match between the remote and the target mode, so we nullify it and will'l try with the local cap.
			//If we found any local cap that match to the remote cap I will initiate the target.
				PTRACE(eLevelInfoNormal,"CCapH323::FindMatching (data only) - there is no data in the initial ");
			else//(dataFound == 0)
				PTRACE(eLevelInfoNormal,"CCapH323::FindMatching (data only) - data algoritem not found ");
			return NOCAP;
		}
	}

	else
	{
		// check the video match
		if(videoFound > 0)
			PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - video found ");
		else
		{
			//we did not find match between the remote and the target mode, so we nullify it and will'l try with the local cap.
			//If we found any local cap that match to the remote cap I will initiate the target.
			pTargetModeH323->SetMediaOff(cmCapVideo,cmCapTransmit);
			if(videoFound == -1)
			{
				PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - there is no video in the initial ");
				videoFound = 1;// to return full cap, since the initial has no video the full match is audio and maybe data
			}
			else//(videoFound == 0)
			{
				if(GetNumOfVideoCap() == 0)// no video cap so the create commMode can ignore the result of the video search match.
				{
					videoFound = 1;
					PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - Remote has no video");
				}
				else
					PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - video algoritem not found ");
			}
		}

		// check the data match
		if(dataFound > 0)
			PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - data found ");
		else
		{
			pTargetModeH323->SetMediaOff(cmCapData,cmCapTransmit);
			if(dataFound == -1)
			{
			//we did not find match between the remote and the target mode, so we nullify it and will'l try with the local cap.
			//If we found any local cap that match to the remote cap I will initiate the target.
				PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - there is no data in the initial ");
				dataFound = 1;// to return full cap
			}
			else//(dataFound == 0)
			{
				if(GetNumOfFeccCap() == 0)// no FECC cap so the create commMode can ignore the result of the video search match.
				{
					dataFound = 1;
					PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - Remote has no data");
				}
				else
					PTRACE(eLevelInfoNormal,"CCapH323::FindMatching - data algoritem not found ");
			}
		}
	}

	if(audioFound && videoFound && dataFound)
		return FULL_MATCH;
	if(audioFound && videoFound && !dataFound)
		return AUDIO_VIDEO;
	if(audioFound && !videoFound && dataFound)
		return AUDIO_DATA;
	if(audioFound && !videoFound && !dataFound)
		return SECONDARY;
	if(!audioFound && !videoFound && !dataFound)
		return NOCAP;
	return NOCAP;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::IsExplicit() const
{
	BYTE bRes = FALSE;
	capBuffer *pCapBuffer = (capBuffer *) &m_pCap->caps;
    char      *tempPtr    = (char*)pCapBuffer;

	for(BYTE i=0; (i<m_numOfCaps) && (bRes == FALSE); i++)
	{
		if (pCapBuffer->capTypeCode == eRoleLabelCapCode)
			bRes = TRUE;

		tempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return bRes;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Find alt number in the matrix by cap position in the cap array
int	CCapH323::FindAltNumber(int position,CapEnum capCode) const
{
	BYTE		bIsEncryptedParty	= IsPartyEncrypted();
	int			capPosition	= -1;

	if(bIsEncryptedParty && (capCode != ePeopleContentCapCode) && (capCode != eRoleLabelCapCode) &&
							(capCode != eH239ControlCapCode) && (capCode != eDBC2CapCode) &&
							(capCode != eDynamicPTRCapCode))
	{
		capPosition = GetEncryptedPosition(position+1,NULL);
	}
	else
		capPosition = position;

	if(capPosition == -1)
		return -1;

	for(int j=0;j < m_pCap->numberOfSim; j++)
	{
		for(int altNumber=0; altNumber < m_pCap->numberOfAlts; altNumber++)
		{
			if(CAP_FD_ISSET(capPosition,&(m_pCap->altMatrix)))
				return altNumber;
			capPosition = capPosition+m_numOfCaps;
		}
	}

	return -1; //In case we did not find
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Find alt number: find the cap via capCode and label and then find it in the matrix
int	CCapH323::FindAltNumber(cmCapDataType capType,ERoleLabel label) const
{
	capBuffer		*pCapBuffer	= (capBuffer *) &m_pCap->caps;
	cmCapDataType	currCapType;
	ERoleLabel		currLabel;
	int				altNumber = -1;
	char			*pTempPtr;

	for(int k=0; k < m_numOfCaps; k++)
	{
		currCapType = (cmCapDataType)(( BaseCapStruct*)pCapBuffer->dataCap)->header.type;
		currLabel	= (ERoleLabel)(( BaseCapStruct*)pCapBuffer->dataCap)->header.roleLabel;

		if ((currCapType == capType) && (currLabel == label))
		{
			CapEnum capCode = (CapEnum)(( BaseCapStruct*)pCapBuffer->dataCap)->header.capTypeCode;
			altNumber = FindAltNumber(k,capCode);
			if(altNumber != -1)
				break;
		}

		pTempPtr = (char*)pCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return altNumber;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Find alt number: find the cap via capCode and label and then find it in the matrix
//!!!This Function can use only for capCode==RoleLabelCapCode when the label is content- because only then the role
//label is initialize!!!!!
int	CCapH323::FindAltNumber(CapEnum capCode,ERoleLabel label) const
{
	capBuffer	*pCapBuffer	= (capBuffer *) &m_pCap->caps;
	CapEnum		currCapCode;
	ERoleLabel	currLabel;
	int			altNumber = -1;
	char		*pTempPtr;

	for(int k=0; k < m_numOfCaps; k++)
	{
		currCapCode = (CapEnum)pCapBuffer->capTypeCode;
		currLabel	= (ERoleLabel)(( BaseCapStruct*)pCapBuffer->dataCap)->header.roleLabel;

		if ((currCapCode == capCode) && (currLabel == label))
		{
			altNumber = FindAltNumber(k,capCode);
			break;
		}

		pTempPtr = (char*)pCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return altNumber;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Check if the content caps are implicit or explicit.
//set content and people alt.
void CCapH323::SetPeopleContentAlt()
{
	if(m_bIsEPC)
	{
		if(IsExplicit()) //In case of explicit we need to know which alt is people and which content to
		{
			m_contentAltNumber	= FindAltNumber(eRoleLabelCapCode,kRoleContent);
			m_peopleAltNumber	= FindAltNumber(eRoleLabelCapCode,kRolePeople);
		}
		else
		{
			// in case of implicit the video caps are both for people and content, so for each cap we should compare it
			//with content and people. (people and content are in the same alt!)
			m_peopleAltNumber	 = FindAltNumber(cmCapVideo,kRolePeople);
			m_contentAltNumber   = m_peopleAltNumber;

		}
	}
	else if (m_bIsH239)
	{
		m_contentAltNumber = FindAltNumber(cmCapVideo, kRolePresentation);
		if (m_contentAltNumber == -1) //not found
			m_contentAltNumber = FindAltNumber(cmCapVideo, kRoleLiveOrPresentation);//we don't check only for roleLive, because we don't support it. Therefore, we ignore it.
		m_peopleAltNumber  = FindAltNumber(cmCapVideo, kRolePeople);
	}
	else//There is no EPC we just need the people alt.
	{
		m_peopleAltNumber  = FindAltNumber(cmCapVideo,kRolePeople);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//In case the capabilities (remote) are not contain the correct label in their header we should check trow the
//description if the cap is for content or for people.
// this function is used only for video, for any other type of media it should return TRUE!!
BYTE CCapH323::CheckRole(ERoleLabel eRole,int index,CBaseCap *pCap /*CapEnum capCode*/) const
{
	BYTE		bIsEPC				= FALSE;
	int			capAltNumber		= -1;
//	CCapSetInfo capInfo = capCode;
	// we check the role only for video type (it can be people or content), all other types are always people.
	if(pCap->IsType(cmCapVideo) == FALSE)
		return TRUE;

	capAltNumber = FindAltNumber(index,pCap->GetCapCode());

	if(eRole == kRolePeople)
	{
		if (capAltNumber == m_peopleAltNumber)
			return TRUE;
		else
			return FALSE; //capAltNumber is in the content alt
	}
	else //eRole == kRoleContent
	{
		if(eRole == kRoleContent)
				if((capAltNumber == m_contentAltNumber) && (pCap->GetRole() == eRole || pCap->GetRole() == kRolePeople)) // in EPC the content cap can be with role of people or content.
					return TRUE;																							// in H239 it can only have presentation role.
				else
					return FALSE;
		else if (capAltNumber == m_contentAltNumber)
			return TRUE;
		else
			return FALSE; //capAltNumber is in the people alt
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
BYTE	CCapH323::BuildComMode(CComModeH323 *pTargetModeH323,WORD confKind,BYTE bIsDataOnly,BYTE bIsCheckVideoOnly)
{	//confKind = 2 ==> CP
	//confkind = 0 ==> VSW
	//confKind = 1 ==> VSW_AUTO

	// We are running here in the context of the remote Cap!
	// pXmitMode323 is the desirable mode.
	PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode - From Rmt Caps & Target mode");

	capBuffer	*pRemoteCapBuffer	= (capBuffer *) &m_pCap->caps;
	BYTE		rval				= 0;
	BYTE		generalFound		= 0;
	char		*pTempPtr;
	APIU8		roleLabel;
	APIU8		direction;
	APIU8		type;
	BYTE		bIsEPC				= FALSE;
	BYTE		bIsH239				= FALSE;
	BYTE		bIsExplicit			= IsExplicit();

	BYTE bCheckAudio = pTargetModeH323->IsMediaOn(cmCapAudio, cmCapTransmit);
	BYTE bCheckVideo = 0;
	BYTE bCheckContent = 0;
	BYTE bCheckData  = 0;

	if(bIsDataOnly)
		bCheckData = pTargetModeH323->IsMediaOn(cmCapData,  cmCapTransmit);
	else
	{
		bCheckVideo = pTargetModeH323->IsMediaOn(cmCapVideo, cmCapTransmit,kRolePeople);
		bCheckContent = pTargetModeH323->IsMediaOn(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation);
		bCheckData  = pTargetModeH323->IsMediaOn(cmCapData,  cmCapTransmit);
	}

	bIsEPC = (bCheckContent && m_bIsEPC && pTargetModeH323->GetMediaMode(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation).IsRole(kRoleContent));
	bIsH239 = (bCheckContent && m_bIsH239 && pTargetModeH323->GetMediaMode(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation).IsRole(kRolePresentation));

	for(int k=0; k < m_numOfCaps; k++)
	{
		direction = (( BaseCapStruct*)pRemoteCapBuffer->dataCap)->header.direction;
		type	  = (( BaseCapStruct*)pRemoteCapBuffer->dataCap)->header.type;
		roleLabel = (( BaseCapStruct*)pRemoteCapBuffer->dataCap)->header.roleLabel;
		rval	  = 0;

		if(direction & cmCapReceive)
		{
			switch(type)
			{
			case cmCapAudio:
				if (!bIsCheckVideoOnly)
				{
					if (bCheckAudio)
					{
						if (pTargetModeH323->GetMediaType(cmCapAudio,cmCapTransmit)== pRemoteCapBuffer->capTypeCode)
							rval = 1;
						BYTE isGeneralG7221C_Cap = FALSE;
						if(pRemoteCapBuffer->capTypeCode == eG7221C_CapCode)
						{
							CG7221CAudioCap * pAudioCap = (CG7221CAudioCap *)CBaseCap::AllocNewCap(eG7221C_CapCode, pRemoteCapBuffer->dataCap);
							if (pAudioCap)
								isGeneralG7221C_Cap = pAudioCap->isCapEnumSupported((CapEnum)pTargetModeH323->GetMediaType(cmCapAudio,cmCapTransmit));
							POBJDELETE(pAudioCap);
							if(isGeneralG7221C_Cap)
								rval = 1;;
						}
					}
				}
				break;
			case cmCapVideo:
				if(bIsEPC) //In case we have P&C version 2, we need to check if the cap is content or people
				{
					if(bIsExplicit)
					{
						PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode : It's Explicit !!");
						int altNumber = FindAltNumber(k,(CapEnum)pRemoteCapBuffer->capTypeCode);

						if(m_peopleAltNumber == altNumber)
						{
							if (pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople) == pRemoteCapBuffer->capTypeCode)
							{
								rval = 1;
								PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode : Found people");
							}
						}
						else if(m_contentAltNumber == altNumber)
						{
							if (pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRoleContent) == pRemoteCapBuffer->capTypeCode)
								rval = 1;
						}
						else
						{
							PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode - can't find match to content or peaple cap.");
							PASSERT(altNumber);
						}
					}
					else //implicit - In this case content and people should be the same capCode.
					{
						if((pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople) == pRemoteCapBuffer->capTypeCode) &&
						   (pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRoleContent) == pRemoteCapBuffer->capTypeCode))
						{
							rval = 1;
							PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode : Found implicit");
						}
					}
				}
				else if(bIsH239 && (kRolePresentation == roleLabel))
				{// in case of H239 we need to check the role label as well as the cap code.
					if (pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation) == pRemoteCapBuffer->capTypeCode)
					{
						rval = 1;
						PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode : Found H239");
					}
				}
				else //we check for people match. We should also check that the
				{
					if (bCheckVideo)
						if (pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople) == pRemoteCapBuffer->capTypeCode)
						{
							PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode : people check");
							rval = 1;
						}
				}
				break;
			case cmCapData:
				if (!bIsCheckVideoOnly)
				{
					if (bCheckData)
						if (pTargetModeH323->GetMediaType(cmCapData,cmCapTransmit) == pRemoteCapBuffer->capTypeCode)
							rval = 1;
				}
				break;
			case cmCapH235:// encryption cap - do nothing
			case cmCapGeneric:
				break;
			default:
				PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode : unknown H323 capability type!");
			}

			if(rval)
				CheckMatrix(k,pRemoteCapBuffer->capTypeCode,pRemoteCapBuffer);
		}

		pTempPtr = (char*)pRemoteCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pRemoteCapBuffer->capLength;
		pRemoteCapBuffer = (capBuffer*)pTempPtr;
		generalFound += rval;
	}

	if(generalFound)
	{// we found at least one match
		rval = FindMatching(pTargetModeH323,confKind,bIsDataOnly,bIsCheckVideoOnly);
		ReleaseMatchList();
	}

	return rval;
}


//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsCapableOfHD720(ERoleLabel eRole) const
{
	BYTE 		bRes 		= FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps && !bRes; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
			if (pH264VideoCap && pH264VideoCap->IsCapableOfHD720(eRole))
				bRes = TRUE;
			POBJDELETE(pH264VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return bRes;
}
//////////////////////////////////////////////////
BYTE CCapH323::IsH264CapFound() const
{

	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps ; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
			if(pH264VideoCap && pH264VideoCap->GetRole() == kRolePeople)
			{
				POBJDELETE(pH264VideoCap);
				return TRUE;
			}
			POBJDELETE(pH264VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return FALSE;
}
///////////////////////////////////////////////////////
BYTE CCapH323::IsH263CapFound() const
{

	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps ; i++)
	{
		if (pCapBuffer->capTypeCode == eH263CapCode)
		{
			CH263VideoCap* pH263VideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode, pCapBuffer->dataCap);
			if(pH263VideoCap && pH263VideoCap->GetRole() == kRolePeople)
			{
				POBJDELETE(pH263VideoCap);
				return TRUE;
			}
			POBJDELETE(pH263VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CCapH323::GetCPVideoPartyType()
{
	eVideoPartyType resVideoPartyType = eVideo_party_type_none;
	eVideoPartyType tmpVideoPartyType = eVideo_party_type_none;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps ; i++)
	{
			CBaseCap* pVideoCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pVideoCap && pVideoCap->IsType(cmCapVideo) && CheckRole(kRolePeople, i,pVideoCap))
			{
			   // COstrStream msg;
			    //pVideoCap->Dump(msg);
				//PTRACE2(eLevelInfoNormal,"CCapH323::GetCPVideoPartyType -cap is ",msg.str().c_str());
				tmpVideoPartyType = ((CBaseVideoCap*)pVideoCap)->GetCPVideoPartyType();
				if(resVideoPartyType<tmpVideoPartyType)
					resVideoPartyType = tmpVideoPartyType;
				//PTRACE2INT(eLevelInfoNormal,"CCapH323::GetCPVideoPartyType type is  ",(WORD)tmpVideoPartyType);
			}
			POBJDELETE(pVideoCap);

		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return resVideoPartyType;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CCapH323::IsCapableOfHDContent1080() const
{
	// Function returns HD 1080 content  MPI
	BYTE 		hd1080Mpi = 0;
	BYTE bRes = FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
	char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps && !bRes; i++)
	{
	   if (pCapBuffer->capTypeCode == eH264CapCode)
	   {
		  CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
		  if ((pH264VideoCap && pH264VideoCap->GetRole() == kRolePresentation) && pH264VideoCap->IsCapableOfHD1080())
		  {
			hd1080Mpi = pH264VideoCap->GetH264Mpi();
			bRes = TRUE;
			POBJDELETE(pH264VideoCap);
			PTRACE2INT(eLevelError,"CCapH323::IsCapableOfHDContent1080 hd1080Mpi = ", hd1080Mpi);  //HP content
			break;
		  }
		  POBJDELETE(pH264VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return hd1080Mpi;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsCapableOfHD1080() const
{
	BYTE 		bRes 		= FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps && !bRes; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
			if (pH264VideoCap && pH264VideoCap->IsCapableOfHD1080())
				bRes = TRUE;
			POBJDELETE(pH264VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return bRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsCapableOfHD1080At60() const
{
	BYTE 		bRes 		= FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps && !bRes; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);

			if(pH264VideoCap)
			{
				if (pH264VideoCap->IsCapableOfHD1080At60())
					bRes = TRUE;
				POBJDELETE(pH264VideoCap);
			}
			else
				PASSERTMSG(NULL == pH264VideoCap, "AllocNewCap return NULL");
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return bRes;
}
//////////////////////////////////////////////////////////////////
BYTE CCapH323::IsCapableOfHD720At60() const
{
	BYTE 		bRes 		= FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps && !bRes; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
			////The threshold for HD720 60 asymmetric mode is HD720 at 50fps
			if (pH264VideoCap && pH264VideoCap->IsCapableOfHD720At50())
				bRes = TRUE;
			POBJDELETE(pH264VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return bRes;
}
//////////////////////////////////////////////////////////////////
BYTE CCapH323::IsCapableOfHDContent720() const
{
	// Function returns back HDContent 720 MPI
	BYTE 		hd720Mpi  = 0;
	BYTE bRes = FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps && !bRes; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
			////The threshold for HD720 60 asymmetric mode is HD720 at 50fps
			if (pH264VideoCap && pH264VideoCap->IsCapableOfHD720(kRolePresentation) && (pH264VideoCap->GetRole() == kRolePresentation))
			{
				//hd720Mpi = pH264VideoCap->GetH264Mpi(); (vngfe-5040):
				hd720Mpi = pH264VideoCap->GetH264Mpi(H264_HD720_FS);
				bRes = TRUE;
				POBJDELETE(pH264VideoCap);
				PTRACE2INT(eLevelError,"CCapH323::IsCapableOfHDContent720 hd720Mpi = ", hd720Mpi);  //HP content
				break;
			}
			POBJDELETE(pH264VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return hd720Mpi;
}

//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsCapableOf4CIF() const
{
	return (Get4CifMpi() != -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////
APIS8  CCapH323::Get4CifMpi() const
{
	// since 4cif is not being declared by local caps, 4cif (for local cap) is being set in seperate data member (m_h263_4CifMpi) on createure.
	// if 4cif data member is set with non -1 value, caps are not being checked (if they have 4cif ability).
	// if that data member is initilize with -1, there is still a posibility that these caps have 4cif ability (for instance, remote EP).
	// Therefore, caps are being checked as well.

	if(m_h263_4CifMpi != -1) return m_h263_4CifMpi;

	APIS8 bRes = -1;

	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i < m_numOfCaps && bRes == -1; i++)
	{
		if (pCapBuffer->capTypeCode == eH263CapCode)
		{
			CH263VideoCap* pH263VideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode, pCapBuffer->dataCap);
			// if role is peopel and 4Cif supported by this cap
			if (pH263VideoCap && pH263VideoCap->GetRole() == kRolePeople)
				bRes = pH263VideoCap->GetFormatMpi(k4Cif);
			POBJDELETE(pH263VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return bRes;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::SetLevelAndAdditionals(APIU16 profile, APIU8 level, APIS32 mbps, APIS32 fs, APIS32 dpb ,APIS32 brAndCpb, APIS32 sar, APIS32 staticMB, ERoleLabel eRole)
{
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps; i++)
	{
		CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pH264VideoCap && pH264VideoCap->GetCapCode() == eH264CapCode && CheckRole(eRole,i,pH264VideoCap))
		{
			pH264VideoCap->SetLevelAndAdditionals(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
		}
		POBJDELETE(pH264VideoCap);

		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::SetLevelAndAdditionals(H264VideoModeDetails& h264VidModeDetails, APIS32 sar, ERoleLabel eRole)
{
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps; i++)
	{
		CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pH264VideoCap && pH264VideoCap->GetCapCode() == eH264CapCode && CheckRole(eRole,i,pH264VideoCap))
		{
			pH264VideoCap->SetLevelAndAdditionals(h264VidModeDetails.profileValue, h264VidModeDetails.levelValue, h264VidModeDetails.maxMBPS, h264VidModeDetails.maxFS,
												  h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR, sar, h264VidModeDetails.maxStaticMbps);
		}
		POBJDELETE(pH264VideoCap);

		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::SetH263FormatMpi(EFormat format, APIS8 mpi, ERoleLabel eRole)
{
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;

	for(BYTE i=0; i<m_numOfCaps; i++)
	{
		CH263VideoCap* pH263VideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pH263VideoCap && pH263VideoCap->GetCapCode() == eH263CapCode && CheckRole(eRole,i,pH263VideoCap))
		{
			pH263VideoCap->SetFormatMpi(format, mpi);

		}
		POBJDELETE(pH263VideoCap);

		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
BYTE	CCapH323::BuildComMode(CCapH323* pLocalCapH323,WORD confKind, CComModeH323 * pTargetModeH323,BYTE bIsCheckVideoOnly)
{	//confKind = 1 ==> CP
	//confkind = 0 ==> VSW
	PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode From Rmt & Local Caps");
	// We are running here in the context of the remote Cap!!!!!!!!!!!!
	// pLocalCapH323 are the MCU caps
	BYTE localCapArray[MAX_CAP_TYPES]; // local cap array
	memcpy(localCapArray,pLocalCapH323->m_capArray,MAX_CAP_TYPES);

	capBuffer* pRemoteCapBuffer;
	pRemoteCapBuffer = (capBuffer *) &m_pCap->caps;

	BYTE numOfH261Found, numOf263VidCapFound,numOf263VidContCapFound, numOfH26LFound, numOfH264Found;
	numOfH261Found = numOf263VidCapFound = numOf263VidContCapFound = numOfH26LFound = numOfH264Found = 0;

	BYTE		bIsEPC				= (pLocalCapH323->IsEPC() && m_bIsEPC);
	BYTE		bIsExplicit			= IsExplicit();

	BYTE rval = 0;
	char* tempPtr;

	for(int k=0; k < m_numOfCaps; k++)
	{
		if(localCapArray[pRemoteCapBuffer->capTypeCode])
		{
			switch(pRemoteCapBuffer->capTypeCode)
			{
				case eG711Alaw64kCapCode :
				case eG711Ulaw64kCapCode :
				case eG711Alaw56kCapCode :
 				case eG711Ulaw56kCapCode :
				case eG722_64kCapCode :
				case eG722_56kCapCode :
				case eG722_48kCapCode :
				case eG722Stereo_128kCapCode :
				case eG728CapCode :
				case eG729CapCode :
				case eG729AnnexACapCode :
				case eG729wAnnexBCapCode :
				case eG729AnnexAwAnnexBCapCode :
				case eG7221_32kCapCode:
				case eG7221_24kCapCode:
				case eG7221_16kCapCode:
				case eSiren7_16kCapCode:
				case eSiren14_48kCapCode:
				case eSiren14_32kCapCode:
				case eSiren14_24kCapCode:
				case eG7221C_48kCapCode:
				case eG7221C_32kCapCode:
				case eG7221C_24kCapCode:
				case eSiren14Stereo_48kCapCode:
				case eSiren14Stereo_56kCapCode:
				case eSiren14Stereo_64kCapCode:
				case eSiren14Stereo_96kCapCode:
				case eSiren22Stereo_128kCapCode:
				case eSiren22Stereo_96kCapCode:
				case eSiren22Stereo_64kCapCode:
				case eSiren22_64kCapCode:
				case eSiren22_48kCapCode:
				case eSiren22_32kCapCode:
				case eG719_128kCapCode:
				case eG719_96kCapCode:
				case eG719_64kCapCode:
				case eG719_48kCapCode:
				case eG719_32kCapCode:
				case eG719Stereo_128kCapCode:
				case eG719Stereo_96kCapCode:
				case eG719Stereo_64kCapCode:
				{
					if (!bIsCheckVideoOnly)
					{
					    audioCapStructBase *pAudioCap = (audioCapStructBase *)pRemoteCapBuffer->dataCap;
						WORD sortedEntry;
						rval=LookForMatchingLocalAudioCap(pLocalCapH323,pAudioCap,sortedEntry);
						if(rval)
								CheckMatrix(k,pRemoteCapBuffer->capTypeCode,pRemoteCapBuffer);
					}
					break;
				}
				case eSirenLPR_32kCapCode:
				case eSirenLPR_48kCapCode:
				case eSirenLPR_64kCapCode:
				case eSirenLPRStereo_64kCapCode:
				case eSirenLPRStereo_96kCapCode:
				case eSirenLPRStereo_128kCapCode:
				{
					if (!bIsCheckVideoOnly)
					{
					    sirenLPR_CapStruct *pAudioCap = (sirenLPR_CapStruct *)pRemoteCapBuffer->dataCap;
						WORD sortedEntry;
						rval=LookForMatchingLocalSirenLPRCap(pLocalCapH323,pAudioCap,sortedEntry);
						if(rval)
								CheckMatrix(k,pRemoteCapBuffer->capTypeCode,pRemoteCapBuffer);
					}
					break;
				}
				case eG7221C_CapCode:
				{
					if (!bIsCheckVideoOnly)
					{
					    g7221C_CapStruct *pAudioCap = (g7221C_CapStruct *)pRemoteCapBuffer->dataCap;
						WORD sortedEntry;
						rval=LookForMatchingLocalG7221CCap(pLocalCapH323,pAudioCap,sortedEntry);
						if(rval)
								CheckMatrix(k,pRemoteCapBuffer->capTypeCode,pRemoteCapBuffer);
					}
					break;
				}
				case eG7231CapCode :
				{
					if (!bIsCheckVideoOnly)
					{
					    g7231CapStruct *pG723Cap = (g7231CapStruct *)pRemoteCapBuffer->dataCap;
						WORD sortedEntry;
						rval=LookForMatchingLocalG723Cap(pLocalCapH323,pG723Cap,sortedEntry);
						if(rval)
								CheckMatrix(k,pRemoteCapBuffer->capTypeCode,pRemoteCapBuffer);
					}
					break;
				}
				case eH261CapCode :
				{
				    h261CapStruct *pH261Cap = (h261CapStruct *)pRemoteCapBuffer->dataCap;
					WORD sortedEntry;
					rval = LookForMatchingLocalH261Cap(pLocalCapH323,pH261Cap,sortedEntry, numOfH261Found);
					if(rval)
						CheckMatrix(k,pRemoteCapBuffer->capTypeCode,pRemoteCapBuffer);
					break;
				}
				case eH263CapCode :
				{
					h263CapStruct *pH263Cap = (h263CapStruct *)pRemoteCapBuffer->dataCap;
					WORD sortedEntry;

					if(bIsEPC) //In case we have P&C version 2, we need to check if the cap is content or people
					{
						if(bIsExplicit)
						{
							int altNumber = FindAltNumber(k,(CapEnum)pRemoteCapBuffer->capTypeCode);
							if(m_peopleAltNumber == altNumber)
							{
								rval=LookForMatchingLocalH263Cap(pLocalCapH323,pH263Cap,sortedEntry,numOf263VidCapFound,kRolePeople);
							}
							else if(m_contentAltNumber == altNumber)
							{
								rval=LookForMatchingLocalH263Cap(pLocalCapH323,pH263Cap,sortedEntry,numOf263VidContCapFound,kRoleContent);
							}
							else
							{
								PTRACE(eLevelInfoNormal,"CCapH323::BuildComMode - can't find match to content or peaple cap.");
								PASSERT(altNumber);
							}
						}
						else //implicit - In this case content and people should be the same capCode.
						{
							rval = LookForMatchingLocalH263Cap(pLocalCapH323,pH263Cap,sortedEntry,numOf263VidCapFound,kRolePeople);
							rval &= LookForMatchingLocalH263Cap(pLocalCapH323,pH263Cap,sortedEntry,numOf263VidContCapFound,kRoleContent);
						}
					}
					else //if no EPC there is no content.
					{
						rval=LookForMatchingLocalH263Cap(pLocalCapH323,pH263Cap,sortedEntry,numOf263VidCapFound,kRolePeople);
					}

					if(rval)
						CheckMatrix(k,pRemoteCapBuffer->capTypeCode,pRemoteCapBuffer);

				break;
				}
				case eH264CapCode :
				{
					h264CapStruct *pH264Cap = (h264CapStruct *)pRemoteCapBuffer->dataCap;
					WORD sortedEntry;
					rval=LookForMatchingLocalH264Cap(pLocalCapH323, pH264Cap, sortedEntry, numOfH264Found);
					if(rval)
						CheckMatrix(k,pRemoteCapBuffer->capTypeCode,pRemoteCapBuffer);

					break;
				}
				case eT120DataCapCode :
				case eAnnexQCapCode:
				case eRvFeccCapCode:
				{
					if (!bIsCheckVideoOnly)
					{
						dataCapStructBase *pDataCap = (dataCapStructBase *)pRemoteCapBuffer->dataCap;
						WORD sortedEntry;
						rval=LookForMatchingLocalDataCap(pLocalCapH323,pDataCap,sortedEntry);
						if(rval)
								CheckMatrix(k,pRemoteCapBuffer->capTypeCode,pRemoteCapBuffer);
					}
					break;
				}
				case eEncryptionCapCode:
				case eH239ControlCapCode:
				case eDynamicPTRCapCode:
				{
					// case of encryption caps - do nothing
					/* In case of Dynamic Payload Type replacement code - do nothing */
					break;
				}
				default:
				{
					ALLOCBUFFER(s, 200);
					sprintf(s," - Unknown Capability is: %d ",pRemoteCapBuffer->capTypeCode);
					PTRACE2(eLevelInfoNormal,"CCapH323::BuildComMode : unknown H323 capability!!!",s);
					DEALLOCBUFFER(s);
					break;
				}
			}
		}
		tempPtr = (char*)pRemoteCapBuffer;
		tempPtr += sizeof(capBufferBase) + pRemoteCapBuffer->capLength;
		pRemoteCapBuffer = (capBuffer*)tempPtr;
	}

	rval = FindMatchingCap(pLocalCapH323,pTargetModeH323,confKind,bIsCheckVideoOnly);
	return rval;
}


/////////////////////////////////////////////////////////////////////
BYTE	CCapH323::IsVideoCapCodeMatch(CCapH323 *pRmtCap) const
{
	for (int i=eH261CapCode; i <= eGenericVideoCapCode; i++)
	{
		if (OnCap(i) && pRmtCap->OnCap(i))
			return TRUE;
	}

	return FALSE;  //If we get out from the function it means that we do not found match in the video cap code.
}

/////////////////////////////////////////////////////////////////////
CVidModeH323 *CCapH323::LookForMatchingH261Pair(CComModeH323 *pInitial323,DWORD *pRmtCap,ERoleLabel label,WORD confKind) const
{
	PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingH261Pair. Label=%s",GetRoleStr(label));

	const CVidModeH323 &xmitInitiateH261 = (const CVidModeH323 &)pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);

	DWORD			details				= 0;
	BYTE			bIsSuccess			= TRUE;

	h261CapStruct *pInitiateH261Cap = (h261CapStruct *)xmitInitiateH261.GetDataCap();
	h261CapStruct *pRemoteH261Cap	= (h261CapStruct *)pRmtCap;

	APIU32			maxBitRate			= min((APIS32)xmitInitiateH261.GetBitRate(),pRemoteH261Cap->maxBitRate);

	details = CompareParameters((BYTE*)pInitiateH261Cap,(BYTE*)pRemoteH261Cap,kFormat|kFrameRate, eH261CapCode);

	if ((confKind == CP) || (confKind == VSW)) //cp or vsw fix - make intersection
	{
		PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingH261Pair CP | VSW fixed - intersection.");
		IntersectionParameters(pInitial323,(BYTE*)pInitiateH261Cap,(BYTE*)pRemoteH261Cap, eH261CapCode,label);
		bIsSuccess = TRUE;
	}
	else //confKind == VSW_AUTO
	{
		//Make Union
		if ((details == HIGHER_FORMAT) || (details == HIGHER_FRAME_RATE)) //the remote is not Containing in the initial we need to add it.
		{
			PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingH261Pair VSW auto - union.");

			EFormat format = GetVideoFormatFromDetails(details);

			if(format == kCif)
				pInitial323->SetFormatMpi(format,pRemoteH261Cap->cifMPI,cmCapTransmit,label);
			else
				pInitial323->SetFormatMpi(format,pRemoteH261Cap->qcifMPI,cmCapTransmit,label);

		}
		else //The remote absolutely containing in the initial
		{
			PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingH261Pair - vsw auto - remote contain in the local.");
			pInitial323->SetMediaMode(eH261CapCode,sizeof(h261CapStruct),(BYTE*)pRemoteH261Cap,cmCapVideo,cmCapTransmit,label);
		}

		pInitial323->SetVideoBitRate(maxBitRate,cmCapTransmit,label);
		PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingInitialH261 - video algorithm found . Label ",GetRoleStr(label));
		bIsSuccess = TRUE;
	}

	if(bIsSuccess)
	{
		CMediaModeH323& rMedia = pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
		pInitial323->SetMediaMode(rMedia, cmCapVideo,cmCapReceive, label);
		return &(CVidModeH323 &)pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingInitialH261 - video algorithm not found. Label ",GetRoleStr(label));
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////
BYTE CCapH323::HandleH263Pair(CComModeH323* pComModeH323,h263CapStruct *pRemoteH263Cap,ERoleLabel label,WORD confKind,
							  APIS32 maxBitRate,h263CapStruct *pLocalH263Cap) const
{
	PTRACE2(eLevelInfoNormal,"CCapH323::HandleH263Pair. Label ",GetRoleStr(label));

	CVidModeH323	*pXmitInitiateH263	= &(CVidModeH323 &)pComModeH323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	h263CapStruct   *pInitiateH263Cap	= (h263CapStruct *)pXmitInitiateH263->GetDataCap();
	BYTE			bIsSuccess			= FALSE;
	BYTE			eRes				= FALSE;

	DWORD			compareRmtLocalCapsRes	= 0;
	DWORD			compareLocalRmtCapsRes	= 0;

	if ((confKind == CP) || (confKind == VSW)) //cp or vsw fix - make intersection
	{
		PTRACE(eLevelInfoNormal,"CCapH323::HandleH263Pair - CP/Vsw fixed - intersection ");
		pInitiateH263Cap = (h263CapStruct *)pXmitInitiateH263->GetDataCap();
		BYTE *pTempCap = NULL;
		if(pLocalH263Cap)
			pTempCap = (BYTE*)pLocalH263Cap;
		else
			pTempCap = (BYTE*)pInitiateH263Cap;

		if (label == kRolePresentation)
			PTRACE(eLevelInfoNormal,"CCapH323::HandleH263Pair - Don't intersec presentation Tx mode");
		else
 		IntersectionParameters(pComModeH323,pTempCap,(BYTE*)pRemoteH263Cap, eH263CapCode,label);
		bIsSuccess = TRUE;

	} //cp or VSW fixed

	else //confKind == VSW_AUTO
	{
		h263CapStruct *pTempCap = NULL;
		if(pLocalH263Cap)
			pTempCap = pLocalH263Cap;

		eRes = IntersectionH263Annex(pXmitInitiateH263,pRemoteH263Cap, pTempCap);
		pInitiateH263Cap	   = (h263CapStruct *)pXmitInitiateH263->GetDataCap();

		compareRmtLocalCapsRes = CompareParameters((BYTE*)pRemoteH263Cap,(BYTE*)pInitiateH263Cap,kFormat|kFrameRate,eH263CapCode);
		compareLocalRmtCapsRes = CompareParameters((BYTE*)pInitiateH263Cap,(BYTE*)pRemoteH263Cap,kFormat|kFrameRate,eH263CapCode);

		if(IS_SECONDS_CAP_HIGHER(compareLocalRmtCapsRes) == 0) //The remote absolutely containing in the initial
		{
			PTRACE(eLevelInfoNormal,"CCapH323::HandleH263Pair - Vsw auto - remote contain in the local ");
			CH263VideoCap *pTemp = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pRemoteH263Cap);
			CH263VideoCap *pInitalVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pXmitInitiateH263->GetDataCap());
			if (pInitalVideoCap)
			{
				pInitalVideoCap->CopyBaseQualities(*pTemp);
				bIsSuccess = TRUE;
				POBJDELETE(pInitalVideoCap);
			}
			else
				PTRACE(eLevelInfoNormal,"CCapH323::HandleH263Pair - pInitalVideoCap is NULL!");
			POBJDELETE(pTemp);
		}
		else if(IS_SECONDS_CAP_HIGHER(compareRmtLocalCapsRes) == 0) //The local absolutely contained in the remote
		{
			//Add to the local the remote caps
			PTRACE(eLevelInfoNormal,"CCapH323::HandleH263Pair - Vsw auto - local contain in the remote ");
			AddOneCapToOther(pXmitInitiateH263,pInitiateH263Cap,pRemoteH263Cap,eH263CapCode,label);
			bIsSuccess = TRUE;

		}
		else //Improve the local cap via the remote cap
		{
			PTRACE(eLevelInfoNormal,"CCapH323::HandleH263Pair - Vsw auto - improve local via the remote ");
			ImproveH263Parameters(pComModeH323,pInitiateH263Cap,pRemoteH263Cap,label);
			bIsSuccess = TRUE;
		}
	}

	if(bIsSuccess == TRUE)
	{
		pComModeH323->SetVideoBitRate(maxBitRate,cmCapTransmit,label);
		PTRACE2(eLevelInfoNormal,"CCapH323::HandleH263Pair - video algorithm found. Label ",GetRoleStr(label));
		return 1;
		//return &(CVidModeH323 &)pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CCapH323::HandleH263Pair - video algorithm not found. Label ",GetRoleStr(label));
		return 0;
		//return NULL;
	}
}

/////////////////////////////////////////////////////////////////////
CVidModeH323 *CCapH323::LookForMatchingH263Pair(CComModeH323 *pInitial323,DWORD *pRmtCap,ERoleLabel label,WORD confKind) const
{
	PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingH263Pair. Label ",GetRoleStr(label));

	CVidModeH323	*pXmitInitiateH263		= &(CVidModeH323 &)pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	h263CapStruct	*pRemoteH263Cap			= (h263CapStruct *)pRmtCap;
	BYTE			bIsSuccess				= FALSE;
	APIU32			maxBitRate				= min((APIS32)pXmitInitiateH263->GetBitRate(),pRemoteH263Cap->maxBitRate);

	bIsSuccess = HandleH263Pair(pInitial323,pRemoteH263Cap,label,confKind,maxBitRate);

	if (bIsSuccess)
	{
		CMediaModeH323& rMedia = pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
		pInitial323->SetMediaMode(rMedia, cmCapVideo,cmCapReceive, label);
		return &(CVidModeH323 &)pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	}

	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////
CVidModeH323 *CCapH323::LookForMatchingH26LPair(CComModeH323 *pInitial323,DWORD *pRmtCap,ERoleLabel label,WORD confKind) const
{
	PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingH26LPair. Label ",GetRoleStr(label));

	const CVidModeH323 &xmitInitiateH26L = (const CVidModeH323 &)pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	DWORD			details				= 0;
	BYTE			bIsSuccess			= TRUE;

	genericVideoCapStruct *pInitiateH26LCap = (genericVideoCapStruct *)xmitInitiateH26L.GetDataCap();
	genericVideoCapStruct *pRemoteH26LCap	= (genericVideoCapStruct *)pRmtCap;
	APIU32				  maxBitRate		= min((APIS32)xmitInitiateH26L.GetBitRate(),pRemoteH26LCap->maxBitRate);


	details = CompareParameters((BYTE*)pInitiateH26LCap,(BYTE*)pRemoteH26LCap,kRoleLabel|kFormat|kFrameRate,eH26LCapCode);

	PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingH26L - intersection");
	IntersectionParameters(pInitial323,(BYTE*)pInitiateH26LCap,(BYTE*)pRemoteH26LCap,eH26LCapCode,label);
	bIsSuccess = TRUE;

	if(bIsSuccess)
	{
		pInitial323->SetVideoBitRate(maxBitRate,cmCapTransmit,label);
		PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingInitialH26L - video algorithm found. Label ",GetRoleStr(label));
		return &(CVidModeH323 &)pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingH26L - video algorithm Not found. Label ",GetRoleStr(label));
		return NULL;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
CVidModeH323 *CCapH323::LookForMatchingH264Pair(CComModeH323 *pInitial323, DWORD *pRmtCap,ERoleLabel label, WORD confKind) const
{
	PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingH264Pair. Label ",GetRoleStr(label));

	const CVidModeH323 &xmitInitiateH264 = (const CVidModeH323 &)pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);

	BYTE bIsSuccess	= TRUE;

	h264CapStruct *pInitiateH264CapStruct = (h264CapStruct *)xmitInitiateH264.GetDataCap();
	h264CapStruct *pRemoteH264CapStruct   = (h264CapStruct *)pRmtCap;

	APIU32 maxBitRate = min((APIS32)xmitInitiateH264.GetBitRate(), pRemoteH264CapStruct->maxBitRate);

	//compareLocalRmtCapsRes contains if pInitiateH264Cap contains pRemoteH264Cap
	DWORD compareLocalRmtCapsRes	  = 0x00000000;
	compareLocalRmtCapsRes = CompareParameters((BYTE*)pInitiateH264CapStruct, (BYTE*)pRemoteH264CapStruct, kH264Level|kH264Additional | kH264Profile, eH264CapCode);

	if ((confKind == CP) || (confKind == VSW)) //cp or vsw fix - make intersection
	{
		PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingH264Pair CP | VSW fixed - intersection");
		int profileValueInt = pInitiateH264CapStruct->profileValue;
		int profileValueRmt = pRemoteH264CapStruct->profileValue;
		if( profileValueInt != profileValueRmt  )
		{
			PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingH264Pair CP | VSW fixed - intersection -differnt profiles");
			return NULL;
		}

		IntersectionParameters(pInitial323, (BYTE*)pInitiateH264CapStruct, (BYTE*)pRemoteH264CapStruct, eH264CapCode,label);
		bIsSuccess = TRUE;
	}
	else //confKind == VSW_AUTO
	{
		if(IS_SECONDS_CAP_HIGHER(compareLocalRmtCapsRes) == 0) //The remote is contained in the initial	scm
		{
			PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingH264Pair  - remote is contained in the initial scm");
			CVidModeH323 *pTemp = new CVidModeH323((BaseCapStruct*)pRemoteH264CapStruct, eH264CapCode); //in order to know the size of the remote cap.
			pInitial323->SetMediaMode(eH264CapCode, pTemp->SizeOf(), (BYTE*)pRemoteH264CapStruct, cmCapVideo, cmCapTransmit,label);
			bIsSuccess = TRUE;
			POBJDELETE(pTemp);
		}
		else
		{
			//compareRmtLocalCapsRes checks if pRemoteH264Cap contains pInitiateH264Cap
			DWORD compareRmtLocalCapsRes	= 0x00000000;
			compareRmtLocalCapsRes = CompareParameters((BYTE*)pRemoteH264CapStruct, (BYTE*)pInitiateH264CapStruct, kH264Level|kH264Additional, eH264CapCode);

			if(IS_SECONDS_CAP_HIGHER(compareRmtLocalCapsRes) == 0) //The initial scm is contained in the remote
			{//get the max remote caps, which are supported in VSW calls
				PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingH264Pair - Vse auto - initial scm is contained in the remote");

				CH264VideoCap* pRemoteH264Cap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pRemoteH264CapStruct);
				if (pRemoteH264Cap)
				{
					CVidModeH323* pNewMode = pRemoteH264Cap->CreateVidModeThatSupportedByMcu();
					if (pNewMode)
					{
						pInitial323->SetMediaMode(eH264CapCode, pNewMode->SizeOf(), (BYTE*)pNewMode->GetDataCap(),
												  cmCapVideo, cmCapTransmit,label);
						bIsSuccess = TRUE;
						POBJDELETE(pNewMode);
					}
					POBJDELETE(pRemoteH264Cap);
				}
			}

			else
			{//intersect
				PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingH264Pair - Vse auto - improve local via the remote ");
				IntersectionParameters(pInitial323, (BYTE*)pInitiateH264CapStruct, (BYTE*)pRemoteH264CapStruct, eH264CapCode,label);
				bIsSuccess = TRUE;
			}
		}
	}

	if(bIsSuccess == TRUE)
	{
		pInitial323->SetVideoBitRate(maxBitRate,cmCapTransmit,label);

	//	CMediaModeH323& rMedia = pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	//	pInitial323->SetMediaMode(rMedia, cmCapVideo,cmCapReceive, label); //yael:

		PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingInitialH264 - video algorithm found. Label ",GetRoleStr(label));
		return &(CVidModeH323 &)pInitial323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CCapH323::LookForMatchingInitialH264 - video algorithm not found. Label ",GetRoleStr(label));
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////
BYTE CCapH323::LookForMatchingLocalAudioCap(CCapH323* pLocalCapH323,
                                            audioCapStructBase* pAudioCap,
                                            WORD& sortedEntry) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingLocalAudioCap ");

	if (pAudioCap->header.direction & cmCapReceive)
	{
		char* tempPtr;
		capBuffer* pLocalCapBuffer	= (capBuffer *) &pLocalCapH323->GetCapStructPtr()->caps;
		WORD numOfCaps				= pLocalCapH323->GetNumOfCap();

		for(WORD i=0; i < numOfCaps; i++) //look in the whole structure
		{
			if(pLocalCapBuffer->capTypeCode == pAudioCap->header.capTypeCode)
			{
	//		  audioCapStructBase *pLocalAudioCap = (audioCapStructBase *)pLocalCapBuffer->dataCap;
				// value is the number of audio packets per second we send.
				if(/*pLocalAudioCap->value <= */pAudioCap->maxValue)
				{
					sortedEntry=i;
					return 1;
				}
				else
					return 0;
			}

			tempPtr = (char*)pLocalCapBuffer;
			tempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
			pLocalCapBuffer = (capBuffer*)tempPtr;
		}
	}
	return 0;
}

BYTE CCapH323::LookForMatchingLocalSirenLPRCap(CCapH323* pLocalCapH323,
											   sirenLPR_CapStruct* pAudioCap,
                                               WORD& sortedEntry) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingLocalSirenLPRCap ");

	if (pAudioCap->header.direction & cmCapReceive)
	{
		char* tempPtr;
		capBuffer* pLocalCapBuffer	= (capBuffer *) &pLocalCapH323->GetCapStructPtr()->caps;
		WORD numOfCaps				= pLocalCapH323->GetNumOfCap();

		for(WORD i=0; i < numOfCaps; i++) //look in the whole structure
		{
			if(pLocalCapBuffer->capTypeCode == pAudioCap->header.capTypeCode)
			{
	//		  audioCapStructBase *pLocalAudioCap = (audioCapStructBase *)pLocalCapBuffer->dataCap;
				// value is the number of audio packets per second we send.
				if(/*pLocalAudioCap->value <= */pAudioCap->maxValue)
				{
					sortedEntry=i;
					return 1;
				}
				else
					return 0;
			}

			tempPtr = (char*)pLocalCapBuffer;
			tempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
			pLocalCapBuffer = (capBuffer*)tempPtr;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////
BYTE CCapH323::LookForMatchingLocalG7221CCap(CCapH323* pLocalCapH323,
                                            g7221C_CapStruct* pAudioCap,
                                            WORD& sortedEntry) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingLocalG7221CCap ");

	if (pAudioCap->header.direction & cmCapReceive)
	{
		char* tempPtr;
		capBuffer* pLocalCapBuffer	= (capBuffer *) &pLocalCapH323->GetCapStructPtr()->caps;
		WORD numOfCaps				= pLocalCapH323->GetNumOfCap();

		for(WORD i=0; i < numOfCaps; i++) //look in the whole structure
		{
			if(pLocalCapBuffer->capTypeCode == eG7221C_48kCapCode)
			{
				if(pAudioCap->maxValue)
				{
					if(pAudioCap->capBoolMask & g7221C_Mask_Rate48K)
					{
						sortedEntry=i;
						return 1;
					}
				}
				return 0;
			}
			else if(pLocalCapBuffer->capTypeCode == eG7221C_32kCapCode)
			{
				if(pAudioCap->maxValue)
				{
					if(pAudioCap->capBoolMask & g7221C_Mask_Rate32K)
					{
						sortedEntry=i;
						return 1;
					}
				}
				return 0;
			}
			else if(pLocalCapBuffer->capTypeCode == eG7221C_24kCapCode)
			{
				if(pAudioCap->maxValue)
				{
					if(pAudioCap->capBoolMask & g7221C_Mask_Rate24K)
					{
						sortedEntry=i;
						return 1;
					}
				}
				return 0;
			}

			tempPtr = (char*)pLocalCapBuffer;
			tempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
			pLocalCapBuffer = (capBuffer*)tempPtr;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////
BYTE CCapH323::LookForMatchingLocalG723Cap(CCapH323* pLocalCapH323,
                                           g7231CapStruct* pG723Cap,
                                           WORD& sortedEntry) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingLocalG723Cap ");

	if (pG723Cap->header.direction & cmCapReceive)
	{
		char* tempPtr;
		capBuffer *pLocalCapBuffer	= (capBuffer *) &pLocalCapH323->GetCapStructPtr()->caps;
		WORD numOfCaps				= pLocalCapH323->GetNumOfCap();

		for(int i=0; i < numOfCaps; i++) //look in the whole structure
		{
			if(pLocalCapBuffer->capTypeCode==pG723Cap->header.capTypeCode)
			{
			    g7231CapStruct *pLocalAudioCap = (g7231CapStruct *)pLocalCapBuffer->dataCap;
				if(/*pLocalAudioCap->maxAl_sduAudioFrames <= */pG723Cap->maxAl_sduAudioFrames)
				{
					sortedEntry = i;
					return 1;
				}
				else
					return 0;
			}

			tempPtr = (char*)pLocalCapBuffer;
			tempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
			pLocalCapBuffer = (capBuffer*)tempPtr;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
BYTE CCapH323::LookForMatchingLocalH261Cap(CCapH323* pLocalCapH323,
                                           h261CapStruct* pH261Cap,
                                           WORD& sortedEntry, BYTE& numOfH261Found)
{
    PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingLocalH261Cap ");
	if (pH261Cap->header.direction & cmCapReceive)
    {
	    char* tempPtr;
	    BYTE numOfVidCapFound		= numOfH261Found;
	    capBuffer* pLocalCapBuffer	= (capBuffer *) &pLocalCapH323->GetCapStructPtr()->caps;
		WORD numOfCaps				= pLocalCapH323->GetNumOfCap();

		for(int i=0; i < numOfCaps; i++) //look in the whole structure
		{
			if (pLocalCapBuffer->capTypeCode == pH261Cap->header.capTypeCode)
			{
				if(numOfVidCapFound == 0) // First video cap found
				{
					numOfH261Found++;
					sortedEntry = i;
					return 1;
				}
				else
					numOfVidCapFound--;
			}
			tempPtr = (char*)pLocalCapBuffer;
			tempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
			pLocalCapBuffer = (capBuffer*)tempPtr;
		}
    }
	return 0;
}

//////////////////////////////////////////////////////////////////////////
BYTE CCapH323::LookForMatchingLocalH263Cap(CCapH323* pLocalCapH323,h263CapStruct* pH263Cap,WORD& sortedEntry,
										   BYTE& numOfH263Found,ERoleLabel label)
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingLocalH263Cap ");
	if (pH263Cap->header.direction & cmCapReceive)
	{
		char* tempPtr;
		BYTE numOfVidCapFound		= numOfH263Found;
		capBuffer *pLocalCapBuffer	= (capBuffer *) &pLocalCapH323->GetCapStructPtr()->caps;
		WORD numOfCaps				= pLocalCapH323->GetNumOfCap();

		for(int i=0; i < numOfCaps; i++) //look in the whole structure
		{
			if((((ctCapStruct*)pLocalCapBuffer->dataCap)->roleLabel == label) &&
			   (pLocalCapBuffer->capTypeCode == pH263Cap->header.capTypeCode))
			{
				if(numOfVidCapFound == 0) // First video cap found
				{
					numOfH263Found++;
					sortedEntry = i;
					return 1;
				}
				else
					numOfVidCapFound--;
			}
			tempPtr = (char*)pLocalCapBuffer;
			tempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
			pLocalCapBuffer = (capBuffer*)tempPtr;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
BYTE CCapH323::LookForMatchingLocalH264Cap(CCapH323* pLocalCapH323, h264CapStruct* pH264Cap, WORD& sortedEntry, BYTE& numOfH264Found)
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingLocalH264Cap ");
	if (pH264Cap->header.direction & cmCapReceive)
	{
		char* tempPtr;
		BYTE numOfVidCapFound		= numOfH264Found;
		capBuffer *pLocalCapBuffer	= (capBuffer *) &pLocalCapH323->GetCapStructPtr()->caps;
		WORD numOfCaps				= pLocalCapH323->GetNumOfCap();

		for(int i=0; i < numOfCaps; i++) //look in the whole structure
		{
			if(pLocalCapBuffer->capTypeCode == pH264Cap->header.capTypeCode )
			{
				if(numOfVidCapFound == 0) // First video cap found
				{
					numOfH264Found++;
					sortedEntry = i;
					return TRUE;
				}
				else
					numOfVidCapFound--;
			}
			tempPtr = (char*)pLocalCapBuffer;
			tempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
			pLocalCapBuffer = (capBuffer*)tempPtr;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
BYTE CCapH323::LookForMatchingLocalH26LCap(CCapH323* pLocalCapH323,genericVideoCapStruct* pH26LCap, WORD& sortedEntry, BYTE& numOfH26LFound)
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingLocalH26LCap ");
	if (pH26LCap->header.direction & cmCapReceive)
	{
		char* tempPtr;
		BYTE numOfVidCapFound		= numOfH26LFound;
		capBuffer *pLocalCapBuffer	= (capBuffer *) &pLocalCapH323->GetCapStructPtr()->caps;
		WORD numOfCaps				= pLocalCapH323->GetNumOfCap();

		for(int i=0; i < numOfCaps; i++) //look in the whole structure
		{
			if(pLocalCapBuffer->capTypeCode == pH26LCap->header.capTypeCode)
			{
				if(numOfVidCapFound == 0) // First video cap found
				{
					numOfH26LFound++;
					sortedEntry = i;
					return 1;
				}
				else
					numOfVidCapFound--;
			}
			tempPtr = (char*)pLocalCapBuffer;
			tempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
			pLocalCapBuffer = (capBuffer*)tempPtr;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////
BYTE CCapH323::LookForMatchingLocalDataCap(CCapH323* pLocalCapH323,dataCapStructBase* pDataCap, WORD& sortedEntry) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForMatchingLocalT120Cap ");

	if (pDataCap->header.direction & cmCapReceive)
	{
		char* tempPtr;
		capBuffer *pLocalCapBuffer	= (capBuffer *) &pLocalCapH323->GetCapStructPtr()->caps;
		WORD numOfCaps				= pLocalCapH323->GetNumOfCap();

		for(int i=0; i < numOfCaps; i++) //look in the whole structure
		{
			if(pLocalCapBuffer->capTypeCode == pDataCap->header.capTypeCode)
			{
				dataCapStructBase *pLocalDataCap = (dataCapStructBase *)pLocalCapBuffer->dataCap;
				if(pLocalDataCap->maxBitRate <= pDataCap->maxBitRate)
				{
					sortedEntry = i;
					return 1;
				}
				else
					return 0;
			}

			tempPtr = (char*)pLocalCapBuffer;
			tempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
			pLocalCapBuffer = (capBuffer*)tempPtr;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CCapH323::CheckMatrix(int position,int capTypeCode,capBuffer *pCapBuffer)
{
	PTRACE(eLevelInfoNormal,"CCapH323::CheckMatrix ");

	//In case this is encrypted call, we should check if the encrypted cap (that hold the regular cap - position)
	//is in the matrix)
	if(IsPartyEncrypted())
		position = GetEncryptedPosition(position+1);

	for(int j=0;j < m_pCap->numberOfSim; j++)
	{
		for(int i=0; i < m_pCap->numberOfAlts; i++)
		{
			if(position >= 0)
			{
				if(CAP_FD_ISSET(position,&(m_pCap->altMatrix)))
				{
					AddNewEntry(capTypeCode,(BYTE)j,i,pCapBuffer);
					return;
				}
				position=position+m_numOfCaps;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Function name:  BuildSortedCap          written by: atara
// Variables:      Non.
// Description:	   Sorts data caps in a buffer by groups in the following order:
//                 audio, video, t120 people-content and encryption . The sort keeps the priority order
//				   inside of each group.
// Return value:   Non
/////////////////////////////////////////////////////////////////////////////
void CCapH323::BuildSortedCap(void)
{
	PTRACE(eLevelInfoNormal,"CCapH323::BuildSortedCap ");


	capBuffer *pCapBuffer = (capBuffer *) &m_pCap->caps;
	char *tempPtr    = (char*)pCapBuffer;

	int audIndex  = 0;
	int vidIndex  = m_numOfAudioCap;
	int contIndex = m_numOfVideoCap + m_numOfAudioCap;
	int duoIndex  = m_numOfVideoCap + m_numOfAudioCap + m_numOfContentCap;
	int t120Index = m_numOfDuoCap + m_numOfContentCap + m_numOfVideoCap + m_numOfAudioCap;
	int feccIndex = m_numOfT120Cap + m_numOfDuoCap + m_numOfContentCap + m_numOfVideoCap + m_numOfAudioCap;
	int pcIndex   = m_numOfFeccCap + m_numOfT120Cap + m_numOfDuoCap + m_numOfContentCap + m_numOfVideoCap + m_numOfAudioCap;
	int h239Index = m_numOfPeopContCap + m_numOfFeccCap + m_numOfT120Cap + m_numOfDuoCap + m_numOfContentCap + m_numOfVideoCap + m_numOfAudioCap;
	int nsIndex   = m_numOfH239ControlCap + m_numOfPeopContCap + m_numOfFeccCap + m_numOfT120Cap + m_numOfDuoCap + m_numOfContentCap + m_numOfVideoCap + m_numOfAudioCap;
	int dptrIndex = nsIndex + m_numOfNsCap;
	int encIndex  = 0;

	BOOL	onlyEncryptedCaps = FALSE;
	//m_sortedCaps is only for cascade. In encrypted call we need to use only the encrypted caps.
	//there are encrypted caps, so we want to locate in the sortedCap only the encryption cap.
	if(m_numOfEncrypCap > 0)
		onlyEncryptedCaps = TRUE;

	for(int i = 0; i< m_numOfCaps; i++) //look in the whole structure
	{
		CapEnum capTypeCode = (CapEnum)(pCapBuffer->capTypeCode);
		CBaseCap * pCapBase = CBaseCap::AllocNewCap(capTypeCode,pCapBuffer->dataCap);

		if (pCapBase)
		{
			if(onlyEncryptedCaps == TRUE) //takes only encrypted cap
			{
				//encryption caps in case of encrypted conf.
				if(pCapBase->IsType(cmCapH235))
				{
					m_sortedCap[encIndex].capTypeCode	= pCapBuffer->capTypeCode;
					m_sortedCap[encIndex].pCapPtr		= pCapBuffer;
					encIndex++;
				}
				//people content: pc profile - for PeopleContentCapCode there is no encrypted cap
				else if ((capTypeCode == ePeopleContentCapCode))
				{
					m_sortedCap[pcIndex].capTypeCode	= capTypeCode;
					m_sortedCap[pcIndex].pCapPtr		= pCapBuffer;
					pcIndex++;
				}
				//video + duo profile does not have encrypted cap
				else if ((capTypeCode == eRoleLabelCapCode) && CheckRole(kRolePeople,i,pCapBase) )
				{
					m_sortedCap[vidIndex].capTypeCode = capTypeCode;
					m_sortedCap[vidIndex].pCapPtr	  = pCapBuffer;
					vidIndex++;
				}
				//content profile does not have encrypted cap
				else if ((capTypeCode == eRoleLabelCapCode) &&
					     (CheckRole(kRoleContent,i,pCapBase) || CheckRole(kRolePresentation,i,pCapBase) ))
				{
					m_sortedCap[contIndex].capTypeCode = capTypeCode;
					m_sortedCap[contIndex].pCapPtr	   = pCapBuffer;
					contIndex++;
				}
				else if (capTypeCode == eDynamicPTRCapCode)
				{
					/* Dynamic Payload Type Replacement control */
					m_sortedCap[dptrIndex].capTypeCode	= capTypeCode;
					m_sortedCap[dptrIndex].pCapPtr		= pCapBuffer;
					pcIndex++;
				}
			}
			else //regular call - no encrypted call
			{
				//audio
				if(pCapBase->IsType(cmCapAudio))
				{
					m_sortedCap[audIndex].capTypeCode = capTypeCode;
					m_sortedCap[audIndex].pCapPtr	  = pCapBuffer;
					audIndex++;
				}
				//video + duo
				else if ((pCapBase->IsType(cmCapVideo)||(capTypeCode == eRoleLabelCapCode)) &&
						 CheckRole(kRolePeople,i,pCapBase))
				{
					m_sortedCap[vidIndex].capTypeCode = capTypeCode;
					m_sortedCap[vidIndex].pCapPtr	  = pCapBuffer;
					vidIndex++;
				}
				//content
				else if ((pCapBase->IsType(cmCapVideo)||(capTypeCode == eRoleLabelCapCode)) &&
						 (CheckRole(kRoleContent,i,pCapBase) || CheckRole(kRolePresentation,i,pCapBase)))
				{
					m_sortedCap[contIndex].capTypeCode = capTypeCode;
					m_sortedCap[contIndex].pCapPtr	   = pCapBuffer;
					contIndex++;
				}
				//t120
				else if(pCapBase->IsType(cmCapData))
				{
					if(capTypeCode == eT120DataCapCode)
					{
						m_sortedCap[t120Index].capTypeCode = capTypeCode;
						m_sortedCap[t120Index].pCapPtr	   = pCapBuffer;
						t120Index++;
					}
					else
					{
						m_sortedCap[feccIndex].capTypeCode = capTypeCode;
						m_sortedCap[feccIndex].pCapPtr	   = pCapBuffer;
						feccIndex++;
					}
				}
				//people content: pc profile
				else if ((capTypeCode == ePeopleContentCapCode))
				{
					m_sortedCap[pcIndex].capTypeCode	= capTypeCode;
					m_sortedCap[pcIndex].pCapPtr		= pCapBuffer;
					pcIndex++;
				}
				//h239 control:
				else if (capTypeCode == eH239ControlCapCode)
				{
					m_sortedCap[h239Index].capTypeCode	= capTypeCode;
					m_sortedCap[h239Index].pCapPtr		= pCapBuffer;
					pcIndex++;
				}
				//Dynamic Payload Type Replacement control:
				else if (capTypeCode == eDynamicPTRCapCode)
				{
					m_sortedCap[dptrIndex].capTypeCode	= capTypeCode;
					m_sortedCap[dptrIndex].pCapPtr		= pCapBuffer;
					pcIndex++;
				}
				// non standard
				else if(pCapBase->IsType(cmCapNonStandard))
				{
					m_sortedCap[nsIndex].capTypeCode	= capTypeCode;
					m_sortedCap[nsIndex].pCapPtr		= pCapBuffer;
					nsIndex++;
				}
			}
		}
		POBJDELETE(pCapBase);

		tempPtr   += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void CCapH323::ZeroingMatchListArray()
{
	for(int i=0;i<MAX_CAP_TYPES;i++)
	{
		m_capMatchList[i]=NULL;
	}
}

//////////////////////////////////////////////////////////////////
void CCapH323::AddNewEntry(int capTypeCode, BYTE sim ,int altNumber,capBuffer *pCapBuffer)
{
	PTRACE(eLevelInfoNormal,"CCapH323::AddNewEntry ");

	capMatchList* pHead;
 	capMatchList* pNewCapMatchList	= new capMatchList;
	pNewCapMatchList->sim 			= sim;
	pNewCapMatchList->altNumber		= altNumber;
	pNewCapMatchList->pNextCap		= NULL;
	pNewCapMatchList->pCapPtr		= (DWORD*)pCapBuffer->dataCap;

	if(m_capMatchList[capTypeCode] == NULL) // the first
	{
		m_capMatchList[capTypeCode] = pNewCapMatchList;
		pNewCapMatchList->count = 1;
	}
	else
	{
		BYTE h263CustAnnexesCap = 0;
		pHead = m_capMatchList[capTypeCode];
		if(capTypeCode == eH263CapCode)
		{
			h263CapStruct *pEnteredH263Cap = (h263CapStruct *)pCapBuffer->dataCap;
			if(pEnteredH263Cap->annexesMask.fds_bits[0]) //if 263 Custume or annexes
				h263CustAnnexesCap = 1;
		}

		if(h263CustAnnexesCap) // put at the begining of the list
		{
			m_capMatchList[capTypeCode] = pNewCapMatchList;
			pNewCapMatchList->pNextCap  = pHead;
			pNewCapMatchList->count = 1;
		}
		else // put at the end of the list
		{
			while(pHead->pNextCap!=NULL)
				pHead = pHead->pNextCap;
			pHead->pNextCap = pNewCapMatchList;
			pNewCapMatchList->count = pHead->count++;
		}
	}
}

/////////////////////////////////////////////////////
void CCapH323::ReleaseMatchList()
{
	capMatchList* pNext;
    capMatchList* pCurrent;

	for(int i=0;i<MAX_CAP_TYPES;i++)
	{
		if(m_capMatchList[i]!= NULL)
		{
			pNext =	m_capMatchList[i]->pNextCap;
			pCurrent = m_capMatchList[i];
			while(pNext != NULL)
			{
				pCurrent = pNext;
				pNext = pNext->pNextCap;
				PDELETE(pCurrent);
			}
			PDELETE(m_capMatchList[i]);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
BYTE CCapH323::FindMatchingCap(CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323,WORD confKind,BYTE bIsCheckVideoOnly)
{
	PTRACE(eLevelInfoNormal,"CCapH323::FindMatchingCap ");
	BYTE audiosim   = 0;
	BYTE audioFound = 0;
	BYTE videoFound = 0;
	BYTE dataFound  = 0;
	BYTE currentCapToOpen = 0;
	capMatchList *pCurrentCap;

	localCap localSortedCap[MAX_CAP_TYPES];
	memcpy(localSortedCap,pLocalCapH323->GetSortedArray(),(sizeof(localCap)*MAX_CAP_TYPES));

	WORD numOfLocalAudioCap = pLocalCapH323->GetNumOfAudioCap();
	WORD numOfLocalVideoCap = pLocalCapH323->GetNumOfVideoCap();
	int firstAudio = -1;

	if (numOfLocalAudioCap > MAX_CAP_TYPES)
	{
		PASSERTSTREAM(1, "numOfLocalAudioCap has unexpected value " << numOfLocalAudioCap);
		numOfLocalAudioCap = MAX_CAP_TYPES;
	}

	if (bIsCheckVideoOnly)
		videoFound = LookForVideoPair(audiosim, pLocalCapH323, pTargetModeH323, confKind);
	else //check for audio + video
	{
		for(int audio = 0; audio < numOfLocalAudioCap; audio++)
		{
			if(m_capMatchList[localSortedCap[audio].capTypeCode] != NULL)
			{
				pCurrentCap = m_capMatchList[localSortedCap[audio].capTypeCode];
				while(pCurrentCap != NULL)
				{
					audiosim	     = pCurrentCap->sim;
					audioFound	     = 1;
					currentCapToOpen = audio;

					if (firstAudio == -1)
						firstAudio = audio;

					PTRACE(eLevelInfoNormal,"CCapH323::FindMatchingCap - audio algorithm found ");
					// now look for video cap at the same sim as we found the audio cap.
					videoFound = LookForVideoPair(audiosim,pLocalCapH323,pTargetModeH323,confKind);
					if(videoFound)
					{
						// if found video cap - get out of the "for"- loop
						audio = numOfLocalAudioCap;
						break;
					}
					pCurrentCap = pCurrentCap->pNextCap;
				}
			}
		}
	}

	if(!videoFound)
	{
		PTRACE(eLevelInfoNormal,"CCapH323::FindMatchingCap - No matching video cap found");
		if(firstAudio != -1)
			currentCapToOpen = firstAudio;
	}

   	if(audioFound)
	{
		dataFound = LookForDataPair(audiosim,pLocalCapH323,pTargetModeH323);

		// only if we found an audio algorithm insert it to the target mode
        pTargetModeH323->SetMediaMode(localSortedCap[currentCapToOpen].pCapPtr,cmCapAudio,cmCapTransmit);
	}

	if(audioFound && videoFound && dataFound)
		return FULL_MATCH;
	if(audioFound && videoFound && !dataFound)
		return AUDIO_VIDEO;
	if(audioFound && !videoFound && dataFound)
		return AUDIO_DATA;
	if(audioFound && !videoFound && !dataFound)
		return SECONDARY;
	if(!audioFound && !videoFound && !dataFound)
		return NOCAP;
	if (!audioFound && videoFound && !dataFound)
		return FOUND_VIDEO;
	return NOCAP;
}

//////////////////////////////////////////////////
BYTE CCapH323::LookFor261VideoPair(localCap localSortedCap[],int index,BYTE audiosim,CCapH323* pLocalCapH323,
								   CComModeH323* pTargetModeH323,WORD confKind)
{
	h261CapStruct *pLocalH261Cap	= (h261CapStruct *)localSortedCap[index].pCapPtr->dataCap;
	capMatchList* phead				= m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode];

	while(phead != NULL)
	{
		h261CapStruct *pRemoteH261Cap			= (h261CapStruct *)phead->pCapPtr;
		DWORD         compareRmtLocalCapsRes	= 0;

		compareRmtLocalCapsRes = CompareParameters((BYTE*)pRemoteH261Cap,(BYTE*)pLocalH261Cap,kFormat|kFrameRate, eH261CapCode);

		if (IS_SECONDS_CAP_HIGHER(compareRmtLocalCapsRes) == 0)
		{
			// in this case local's cap is not higher than remote's,
			// so if it's the same sim as audio cap we set local cap in the target mode
			if(audiosim == m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode]->sim)
			{
				capBuffer * pNewMode = localSortedCap[index].pCapPtr;
				PTRACE(eLevelInfoNormal,"CCapH323::LookFor261VideoPair - video algorithm found");
				UpdateComMode(pTargetModeH323, pNewMode->capTypeCode, pNewMode->capLength, pNewMode->dataCap);
				return 1;
			}
		}
		else
		{
			// at this point we know that remote's cap is lower than local's in format or frame rate,
			// BUT there is a possibility that remote's caps are higher than local's in others:
			// for instance, remote channel rate is 3200 bps and local's is 3840 bps (rmt < local)
			// AND remote's resolution is Cif and local's is QCif (rmt > local)
			// so in this case we will NOT set target mode with any cap (local's or remote's)

			DWORD compareLocalRmtCapsRes = 0;

			PTRACE(eLevelInfoNormal,
			"CCapH323::LookFor261VideoPair\n\
			calling CompareParameters when 'first' refers to the local's possible channel and 'second' to the remote cap set");

			compareLocalRmtCapsRes = CompareParameters((BYTE*)pLocalH261Cap, (BYTE*)pRemoteH261Cap,kFormat|kFrameRate,eH261CapCode);

			// if ((compareLocalRmtCapsRes & 0x0000007f)==0)
			if (IS_SECONDS_CAP_HIGHER(compareLocalRmtCapsRes)==0)
			{
				// in this case remote's caps are not higher than local's in any functionality
				// so if the local's conference type is CP we can set the remote's cap in the target mode
				if (((confKind == CP) || (confKind == VSW_AUTO)) &&
					(audiosim == m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode]->sim))
				{
					PTRACE(eLevelInfoNormal,"CCapH323::LookFor261VideoPair - video algorithm found");
					UpdateComMode(pTargetModeH323,eH261CapCode, sizeof(h261CapStruct),(BYTE*)phead->pCapPtr);
					return 1;
				}
			}

		}
		phead = phead->pNextCap;

	} //while

	return 0;
}

//////////////////////////////////////////////////
BYTE CCapH323::LookFor263VideoPair(localCap localSortedCap[],int index,BYTE audiosim,CCapH323* pLocalCapH323,
								   CComModeH323* pTargetModeH323,WORD confKind,int altNumber,ERoleLabel label)
{
	PTRACE2(eLevelInfoNormal,"CCapH323::LookFor263VideoPair. Label ",GetRoleStr(label));

	CVidModeH323  *pXmitInitiateH263		= &(CVidModeH323 &)pTargetModeH323->GetMediaMode(cmCapVideo,cmCapTransmit,label);
	h263CapStruct *pLocalH263Cap			= (h263CapStruct *)localSortedCap[index].pCapPtr->dataCap;
	capMatchList  *phead					= m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode];

	// copy data to pXmitInitiateH263 from pLocalH263Cap

	while(phead != NULL)
	{
		if(phead->altNumber == altNumber)
		{
			h263CapStruct *pRemoteH263Cap       = (h263CapStruct *)phead->pCapPtr;

			BYTE			bIsSuccess				= FALSE;
			APIU16			maxBitRate				= min(pLocalH263Cap->maxBitRate,pRemoteH263Cap->maxBitRate);

			bIsSuccess = HandleH263Pair(pTargetModeH323,pRemoteH263Cap,label,confKind,maxBitRate,pLocalH263Cap);

			if (bIsSuccess)
			{
			/*	CBaseCap* pTransmitScm = pTargetModeH323->GetMediaAsCapClass(cmCapVideo, cmCapTransmit, label);
				pTargetModeH323->SetMediaMode(pTransmitScm, cmCapVideo, cmCapReceive, label);

				CBaseCap* pReceiveScm = pTargetModeH323->GetMediaAsCapClass(cmCapVideo, cmCapReceive, label);
				pReceiveScm->SetDirection(cmCapReceive);
				pTargetModeH323->SetMediaMode(pReceiveScm, cmCapVideo, cmCapReceive, label);	*/ //yael

				return 1;
			}

			else
				return 0;
		}
		phead = phead->pNextCap;
	}//endwhile phead - h263

	return 0;
}

//////////////////////////////////////////////////
BYTE CCapH323::LookFor264VideoPair(localCap localSortedCap[],int index,BYTE audiosim,CCapH323* pLocalCapH323,
								   CComModeH323* pTargetModeH323,WORD confKind)
{
    h264CapStruct* pLocalH264Cap	= (h264CapStruct *)localSortedCap[index].pCapPtr->dataCap;
    capMatchList* phead				= m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode];

	while (phead != NULL)
	{
		h264CapStruct* pRemoteH264Cap = (h264CapStruct *)phead->pCapPtr;
		DWORD compareRmtLocalCapsRes   = 0;

		compareRmtLocalCapsRes = CompareParameters((BYTE*)pRemoteH264Cap, (BYTE*)pLocalH264Cap, kH264Level|kH264Additional, eH264CapCode);

		if (IS_SECONDS_CAP_HIGHER(compareRmtLocalCapsRes)==0)
		{	// in this case local's cap is not higher than remote's,
			// so if it's the same sim as audio cap we set local cap in the target mode
			if(audiosim == m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode]->sim)
			{
				capBuffer* pNewMode = localSortedCap[index].pCapPtr;
				PTRACE(eLevelInfoNormal,"CCapH323::LookFor264VideoPair - video algorithm found");
				UpdateComMode(pTargetModeH323, eH264CapCode, pNewMode->capLength, pNewMode->dataCap);
				return 1;
			}
		}

		else
		{	// at this point we know that remote's cap is lower than local's in one or more functionalities
			// BUT there is a possibility that remote's caps are higher than local's in others:
			// for instance, remote channel rate is 3200 bps and local's is 3840 bps (rmt < local)
			// AND remote's resolution is Cif and local's is QCif (rmt > local)
			// so in this case we will NOT set target mode with any cap (local's or remote's)

			DWORD compareLocalRmtCapsRes = 0;

			PTRACE(eLevelInfoNormal,"CCapH323::LookFor264VideoPair: calling CompareH264Parameters when 'first' refers to the local's possible channel and 'second' to the remote cap set");

			compareLocalRmtCapsRes = CompareParameters((BYTE*)pLocalH264Cap, (BYTE*)pRemoteH264Cap, kH264Level|kH264Additional, eH264CapCode);

			if (IS_SECONDS_CAP_HIGHER(compareLocalRmtCapsRes)==0)
			{	// in this case remote's caps are not higher than local's in any functionality
				// so if the local's conference type is CP we can set the remote's cap in the target mode
				if( ( (confKind == CP) || (confKind == VSW_AUTO) ) &&
					(audiosim == m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode]->sim))
				{
					PTRACE(eLevelInfoNormal,"CCapH323::LookFor264VideoPair - video algorithm found");
					UpdateComMode(pTargetModeH323, eH264CapCode, sizeof(h264CapStruct), (BYTE*)phead->pCapPtr);
					return 1;
				}
			}
		}
		phead = phead->pNextCap;

	}//endwhile phead - h264

	return 0;
}

//////////////////////////////////////////////////
BYTE CCapH323::LookFor26LVideoPair(localCap localSortedCap[],int index,BYTE audiosim,CCapH323* pLocalCapH323,
								   CComModeH323* pTargetModeH323,WORD confKind)
{
	genericVideoCapStruct * pLocalH26LCap	= (genericVideoCapStruct *)localSortedCap[index].pCapPtr->dataCap;
	capMatchList* phead						= m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode];

	while(phead != NULL)
	{
		genericVideoCapStruct * pRemoteH26LCap       = (genericVideoCapStruct *)phead->pCapPtr;
		DWORD           compareRmtLocalCapsRes = 0;

		compareRmtLocalCapsRes = CompareParameters((BYTE*)pRemoteH26LCap, (BYTE*)pLocalH26LCap,kFormat|kFrameRate|kAnnexes, eH26LCapCode);

		if (compareRmtLocalCapsRes)
		{
			// in this case local's cap is not higher than remote's,
			// so if it's the same sim as audio cap we set local cap in the target mode
			if(audiosim == m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode]->sim)
			{
				capBuffer * pNewMode = localSortedCap[index].pCapPtr;
				PTRACE(eLevelInfoNormal,"CCapH323::LookFor26LVideoPair - video algorithm found");
				UpdateComMode(pTargetModeH323, pNewMode->capTypeCode, pNewMode->capLength, pNewMode->dataCap);
				return 1;
			}
		}
		else
		{
			// at this point we know that remote's cap is lower than local's in one or more functionalities
			// BUT there is a possibility that remote's caps are higher than local's in others:
			// for instance, remote channel rate is 3200 bps and local's is 3840 bps (rmt < local)
			// AND remote's resolution is Cif and local's is QCif (rmt > local)
			// so in this case we will NOT set target mode with any cap (local's or remote's)

			DWORD compareLocalRmtCapsRes = 0;

			PTRACE(eLevelInfoNormal,
			"CCapH323::LookFor26LVideoPair:\n\
			calling CompareParameters when 'first' refers to the local's possible channel and 'second' to the remote cap set");

			compareLocalRmtCapsRes = CompareParameters((BYTE*)pLocalH26LCap, (BYTE*)pRemoteH26LCap,kFormat|kFrameRate|kAnnexes,eH26LCapCode);

			// if ((compareLocalRmtCapsRes & 0x0000007f)==0)
			if (IS_SECONDS_CAP_HIGHER(compareLocalRmtCapsRes)==0)
			{
				// in this case remote's caps are not higher than local's in any functionality
				// so if the local's conference type is CP we can set the remote's cap in the target mode
				if ((confKind == CP)&&(audiosim == m_capMatchList[localSortedCap[index].pCapPtr->capTypeCode]->sim))
				{
					PTRACE(eLevelInfoNormal,"CCapH323::LookFor26LVideoPair - video algorithm found");
					UpdateComMode(pTargetModeH323,eH26LCapCode, sizeof(genericVideoCapStruct),(BYTE*)phead->pCapPtr);
					return 1;
				}
			}
		}
		phead = phead->pNextCap;

	}//endwhile phead - h26L

	return 0;
}

//////////////////////////////////////////////////
BYTE CCapH323::LookForVideoPair(BYTE audiosim,CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323,WORD confKind)
{
    PTRACE(eLevelInfoNormal,"CCapH323::LookForVideoPair ");

    localCap localSortedCap[MAX_CAP_TYPES];
    memcpy(localSortedCap,pLocalCapH323->GetSortedArray(),(sizeof(localCap)*MAX_CAP_TYPES));

    WORD	numOfLocalAudioCap	= pLocalCapH323->GetNumOfAudioCap();
    WORD	numOfLocalVideoCap	= pLocalCapH323->GetNumOfVideoCap();
	WORD	numOfLocalContentCap= pLocalCapH323->GetNumOfContentCap();
	BYTE	bIsEPC				= (pLocalCapH323->IsEPC() && m_bIsEPC);
	BYTE	bIsExplicit			= IsExplicit();
	BYTE	rval				= 0;
	BYTE	bIsFoundVidPeopleMatch	= FALSE;
	BYTE	bIsFoundVidContentMatch = FALSE;
	int		lastVideoIndex		= numOfLocalAudioCap + numOfLocalVideoCap;
	int		lastContentIndex	= numOfLocalAudioCap + numOfLocalVideoCap + numOfLocalContentCap;
	BYTE	bCheckContent		= pLocalCapH323->IsEPC();
	int		i;

	if (lastVideoIndex > MAX_CAP_TYPES)
	{
		PASSERTSTREAM(1, 
			"lastVideoIndex has unexpected value " << lastVideoIndex
			<< ", numOfLocalAudioCap " << numOfLocalAudioCap
			<< ", numOfLocalVideoCap " << numOfLocalVideoCap);
		lastVideoIndex = MAX_CAP_TYPES;
	}
	
	if(!bIsEPC) //in case there in no content we shouldnt search for content matching.
		bIsFoundVidContentMatch = TRUE;

    for(i = numOfLocalAudioCap; ((i < lastVideoIndex) && (!bIsFoundVidPeopleMatch)); i++)
	{
		if(localSortedCap[i].pCapPtr == NULL)
			continue;
        switch(localSortedCap[i].pCapPtr->capTypeCode)
        {
        case eH261CapCode :
			bIsFoundVidPeopleMatch = LookFor261VideoPair(localSortedCap,i,audiosim,pLocalCapH323,pTargetModeH323,confKind);
			break;
        case eH263CapCode :
			bIsFoundVidPeopleMatch = LookFor263VideoPair(localSortedCap,i,audiosim,pLocalCapH323,pTargetModeH323,confKind,m_peopleAltNumber,kRolePeople);
			break;
        case eH264CapCode :
			bIsFoundVidPeopleMatch = LookFor264VideoPair(localSortedCap,i,audiosim,pLocalCapH323,pTargetModeH323,confKind);
			break;
        case eH26LCapCode :
			bIsFoundVidPeopleMatch = LookFor26LVideoPair(localSortedCap,i,audiosim,pLocalCapH323,pTargetModeH323,confKind);
			break;
        }//endswitch localsortedcap
    }//endforloop

    if(lastContentIndex <= MAX_CAP_TYPES)
    {
        for(i = lastVideoIndex; ((i < lastContentIndex) && (!bIsFoundVidContentMatch)); i++)
        {
		    if((localSortedCap[i].pCapPtr != NULL) && (localSortedCap[i].pCapPtr->capTypeCode == eH263CapCode))
		    	bIsFoundVidContentMatch =	LookFor263VideoPair(localSortedCap,i,audiosim,pLocalCapH323,pTargetModeH323,confKind,m_contentAltNumber,kRoleContent);
        }//endforloop
    }
    else
        PASSERTMSG(1, "lastContentIndex is larger than MAX_CAP_TYPES"); 		

	//We should return the success of finding match for people and content.
	rval = bIsFoundVidPeopleMatch & bIsFoundVidContentMatch;
    return rval;
}

/////////////////////////////////////////////////////////////////////////////
// Function name:  LookForDataPair          written by: atara
// Variables:      audiosim:        The sim that audio algorithm was found in.
//				   pLocalCapH323:   Mcu's caps.
//				   pTargetModeH323: target communication mode
//                                  (the mode we want to get for communication).
// Description:	   Looks for data algorithm at the same sim as the audio
//                 algorithm was found. Sets the data alg at the target mode.
// Return value:   Success (data alg was found) = 1, Failure = 0.
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::LookForDataPair(BYTE audiosim,CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323) const
{
	PTRACE(eLevelInfoNormal,"CCapH323::LookForDataPair ");
	capMatchList *pCurrentCap;

	localCap localSortedCap[MAX_CAP_TYPES];
	memcpy(localSortedCap,pLocalCapH323->GetSortedArray(),(sizeof(localCap)*MAX_CAP_TYPES));

	WORD numOfLocalAudioCap = pLocalCapH323->GetNumOfAudioCap();
	WORD numOfLocalVideoCap = pLocalCapH323->GetNumOfVideoCap();
	WORD numOfLocalContentCap = pLocalCapH323->GetNumOfContentCap();
	WORD numOfLocalDataCap	= 0;
	if(pLocalCapH323->GetNumOfT120Cap())
		numOfLocalDataCap  = pLocalCapH323->GetNumOfT120Cap();
	else if(pLocalCapH323->GetNumOfFeccCap())
		numOfLocalDataCap  = pLocalCapH323->GetNumOfFeccCap();
	else
		numOfLocalDataCap = 0;

	if (numOfLocalDataCap == 0)
		return 2;

	int dataStart = numOfLocalAudioCap + numOfLocalVideoCap + numOfLocalContentCap;
	int dataEnd   = numOfLocalAudioCap + numOfLocalVideoCap + numOfLocalContentCap + numOfLocalDataCap;

	if (dataEnd > MAX_CAP_TYPES)
	{
		PASSERTSTREAM(1, 
			"dataEnd has unexpected value " << dataEnd
			<< ", numOfLocalAudioCap " << numOfLocalAudioCap
			<< ", numOfLocalVideoCap " << numOfLocalVideoCap
			<< ", numOfLocalDataCap " << numOfLocalDataCap);
		dataEnd = MAX_CAP_TYPES;
	}
	
	for(int i = dataStart; i < dataEnd; i++)
	{
		CCapSetInfo capInfo = (CapEnum)localSortedCap[i].capTypeCode;

		if((localSortedCap[i].pCapPtr != NULL) && capInfo.IsType(cmCapData) &&
				(m_capMatchList[localSortedCap[i].capTypeCode] != NULL))
		{
			pCurrentCap = m_capMatchList[localSortedCap[i].capTypeCode];
			while(pCurrentCap != NULL)
			{
				// check if it's the same sim
                if(pCurrentCap->sim == audiosim)
                {
                  	localSortedCap[i].pCapPtr->capTypeCode = localSortedCap[i].capTypeCode;// because of VNGR-6232 (sometimes the pCapPtr->capTypeCode is not updated correctly).
					PTRACE2(eLevelInfoNormal,"CCapH323::LookForDataPair - data algoritem found ", CapEnumToString((CapEnum)localSortedCap[i].capTypeCode));

                    // only if we found a data algorithem insert it to the target mode
                    dataCapStructBase *localDataStruct = (dataCapStructBase*)localSortedCap[i].pCapPtr->dataCap;
                    dataCapStructBase *remoteDataStruct = (dataCapStructBase*)pCurrentCap->pCapPtr;
                    if(localDataStruct->maxBitRate < remoteDataStruct->maxBitRate)
                        pTargetModeH323->SetMediaMode(localSortedCap[i].pCapPtr,cmCapData,cmCapTransmit);
                    else
                        pTargetModeH323->SetMediaMode(localSortedCap[i].pCapPtr->capTypeCode,sizeof(dataCapStructBase),(BYTE*)pCurrentCap->pCapPtr,cmCapData,cmCapTransmit);

                    return 1;
                }
				pCurrentCap = pCurrentCap->pNextCap;
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::UpdateComModeCustomAnnexesMPI( CComModeH323* pTargetModeH323, capBuffer *pCapBuffer)
{
	// Right now this function and the code doent support 263+ cascade between CP conference and VSW
	// it was only written as a link between two VSW conference custom
	PTRACE(eLevelInfoNormal,"CCapH323::UpdateComModeCustomAnnexesMPI - video algorithm found h263+");
    pTargetModeH323->SetMediaMode(pCapBuffer,cmCapVideo);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CCapH323::UpdateComModeMPI(int localH263CapMpi,int remoteH263CapMpi, WORD confKind,
                                 capBuffer* pCapPtr, CComModeH323* pTargetModeH323, capMatchList* phead)
{
    PTRACE(eLevelInfoNormal,"CCapH323::UpdateComModeSqcifMPI - video algorithm found");
    if(confKind==CP) //CP
    {
        if(localH263CapMpi > remoteH263CapMpi)
            pTargetModeH323->SetMediaMode(pCapPtr, cmCapVideo);
        else
            pTargetModeH323->SetMediaMode(eH263CapCode,sizeof(h263CapStruct),(BYTE*)phead->pCapPtr, cmCapVideo);
        return 1;
    }
    else //VSW
    {
        if(localH263CapMpi == remoteH263CapMpi)
        {
            pTargetModeH323->SetMediaMode(pCapPtr,cmCapVideo);
            return 1;
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::UpdateComMode(CComModeH323* pTargetModeH323, WORD newType, WORD newDataLength, const BYTE newData[],ERoleLabel label)
{
    PTRACE(eLevelInfoNormal,"CCapH323::UpdateComMode - video algorithm found");
	pTargetModeH323->SetMediaMode(newType,newDataLength,newData,cmCapVideo, cmCapTransmit,label);
}


////////////////////////////////////////////////////////////////////////////////////////////////
// the function add capability to the cap buffer
// the capability is given by CapEnum, struct_size,
// and AlgorithmCapStruct that been cast to char*
// the function should replace all other function that doing the same
////////////////////////////////////////////////////////////////////////////////////////////////
void CCapH323::AddCapToCapBuffer(CapEnum algorithmCapCode, int structSize, BaseCapStruct* algorithmCapStructPtr)
{
	algorithmCapStructPtr->header.xmlHeader.dynamicType   = algorithmCapCode;
	algorithmCapStructPtr->header.xmlHeader.dynamicLength = structSize;

	if((structSize + m_offsetWrite) < GetCurrentCapsBufSize())
	{
		capBuffer* pCapBuffer = (capBuffer *)((BYTE *)&m_pCap->caps + m_offsetWrite);
		// XML setting
		pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
		pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + structSize;
		pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
		pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = structSize;

		pCapBuffer->capTypeCode					= algorithmCapCode;
		pCapBuffer->capLength					= structSize;
		memcpy(pCapBuffer->dataCap, (char*)algorithmCapStructPtr, structSize);
		m_offsetWrite = m_offsetWrite + sizeof(capBufferBase) + structSize;
	}
	else
	{	//not enough space in the buffer
		capBuffer*	pNewCapBuffer = (capBuffer *)new BYTE[INITIAL_CAPS_LEN];

		if(pNewCapBuffer)
		{
			// XML setting
			pNewCapBuffer->xmlHeader.dynamicType   = tblCapBuffer;
			pNewCapBuffer->xmlHeader.dynamicLength = sizeof(capBufferBase) + structSize;
			pNewCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
			pNewCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = structSize;

			pNewCapBuffer->capTypeCode			   = algorithmCapCode;
			pNewCapBuffer->capLength			   = structSize;
			memcpy(pNewCapBuffer->dataCap, (char*)algorithmCapStructPtr, structSize);
			AllocateNewBuffer(pNewCapBuffer, structSize + sizeof(capBufferBase));
			delete[] (BYTE *)pNewCapBuffer;
		}
		else
			PASSERTMSG(1, "CCapH323::AddCapToCapBuffer - pNewCapBuffer not valid");
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CCapH323::UpdateNumOfCaps(CapEnum algorithmCapCode,ERoleLabel eRole)
{
	CCapSetInfo capInfo = algorithmCapCode;

	// Update num of caps
	if (capInfo.IsType(cmCapAudio))
		m_numOfAudioCap--;
	else if (capInfo.IsType(cmCapVideo))
	{
		if (eRole == kRolePeople)
			m_numOfVideoCap--;
		else //kRoleContent or kRolePresentation
			m_numOfContentCap--;
	}
	else if (capInfo.IsType(cmCapData))
	{
		if(capInfo.GetIpCapCode() == eT120DataCapCode)
			m_numOfT120Cap--;
		else
			m_numOfFeccCap--;
	}
	else if (capInfo.IsType(cmCapNonStandard))
		m_numOfNsCap--;
	else if(capInfo.IsType(cmCapH235))
		m_numOfEncrypCap--;

	else if (capInfo.IsCapCode(ePeopleContentCapCode))
	{
		m_numOfPeopContCap--;
		if (m_numOfPeopContCap == 0)
		{
			m_bIsEPC = FALSE;
			m_bIsPCversion0 = FALSE;
		}
	}
	else if (capInfo.IsCapCode(eH239ControlCapCode))
	{
		m_numOfH239ControlCap--;
		if (m_numOfH239ControlCap == 0)
			m_bIsH239 = FALSE;
	}
	else if (capInfo.IsCapCode(eDynamicPTRCapCode))
	{
		m_numOfDynamicPTRControlCap--;
		if (m_numOfDynamicPTRControlCap == 0)
			m_bIsDPTR = FALSE;
	}
	else if (capInfo.IsCapCode(eRoleLabelCapCode))
	{
		if(eRole == kRolePeople)
			m_numOfVideoCap--;
		else
			m_numOfContentCap--;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Function name: RemoveEncryptedCapFromCapBuffer
// Variables:     index: it is the index of the capability that we want to find it's encrypted cap.
// Description:	  We should remove the encrypted cap (235 cap)
// Return value:  TRUE if we found specific 235Cap, False if we dont find.
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::RemoveEncryptedCapFromCapBuffer(int index)
{
	PTRACE(eLevelInfoNormal,"CCapH323::RemoveEncryptedCapFromCapBuffer");

	int			encryptedPosition	= -1;
	capBuffer  *pTmpCapBuffer		= NULL;
	BYTE		bIsRemove			= FALSE;

	encryptedPosition = GetEncryptedPosition(index+1,&pTmpCapBuffer);
	if(encryptedPosition != -1)//there is encrypt cat for the specific remove cap.
	{
		//in case the cap is encrypt remove the 235cap (encryptedPosition).
		//We should remove first the 235Cap before, because it comes after the cap that we want
		//to remove so we will not harm the remove cap's index

		//We should update the entries in the encrypted caps before remove any capability
		UpdateEncryptedCaps(encryptedPosition,pTmpCapBuffer);
		RemoveOneCapset(pTmpCapBuffer, encryptedPosition, eEncryptionCapCode);
		UpdateNumOfCaps(eEncryptionCapCode);
		bIsRemove = TRUE;
	}
	else
		FixEncryptedCapsBackward (index+1);


	return bIsRemove;
}

/////////////////////////////////////////////////////////////////////////////
// Function name: RemoveCapFromCapBuffer                    written by: atara
// Variables:     algorithmCapCode: Algorithm cap code to remove.
//                bH263plus: Indicates if the algorithm to be removed is h263+.
//                eRole: People or content on video caps
// Description:	  Remove cap (audio, data or h263+) or caps (video) from cap buffer.
// Return value:  The number of removed caps.
/////////////////////////////////////////////////////////////////////////////
WORD CCapH323::RemoveCapFromCapBuffer(CapEnum algorithmCapCode,ERoleLabel eRole,BOOL bH263plus,APIU16 profileToRemove )
{
	WORD		numOfRemovedCaps	= 0;
	BOOL		bIsNeedRestart		= TRUE;
	BYTE		bIsEncryptedParty	= IsPartyEncrypted();
	BOOL        IsRemoveonlySpecificProfie = FALSE;
	if(profileToRemove)
		IsRemoveonlySpecificProfie = TRUE;

	while(bIsNeedRestart) //On encryption after each remove we should restart the search
	{
		capBuffer*	pCapBuffer	= (capBuffer *) &m_pCap->caps;
		char*		tempPtr		= (char *)pCapBuffer;

		bIsNeedRestart			= FALSE; //only if there is encrypted cap we should restart the search.
		// Look for the algorithm to remove in the caps buffer
		for (int i=0; i<m_numOfCaps; i++)
		{
			BYTE bRemoveThisCap = FALSE;
			// in order to know if it's ok to remove this cap we first check if the cap code is equals,
			// then if the role is equals.
			if (algorithmCapCode == pCapBuffer->capTypeCode)
			{
				CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);

				if (pCap)
				{
					if(pCap->IsType(cmCapVideo) || (algorithmCapCode == eRoleLabelCapCode))
					{
						if (CheckRole(eRole,i,pCap))
						{//role presentation and role content are in the same alt
							if (eRole == kRolePresentation) //presentation has a explicit role in cap
								bRemoveThisCap = (pCap->GetRole() == kRolePresentation);
							else if (eRole == kRoleContent) //presentation doesn't have a explicit role in remote cap
								bRemoveThisCap = (pCap->GetRole() != kRolePresentation);
							else //people
								bRemoveThisCap = TRUE;
						}
						if(bRemoveThisCap && IsRemoveonlySpecificProfie && pCap->GetCapCode() == eH264CapCode)
						{
							APIU16 capProfile = ((CH264VideoCap*)pCap)->GetProfile();
							if(capProfile != profileToRemove)
								bRemoveThisCap = FALSE;

						}
					}
					else
						bRemoveThisCap = TRUE;

				}

				POBJDELETE(pCap);
			}

			// if the cap is for remove do the following -
			if (bRemoveThisCap)
			{
				//In case of encryption we should remove either the 235cap that encrypt the remove cap.
				if(bIsEncryptedParty)
				{
					//in case the cap is encrypt remove the 235cap (encryptedPosition).
					//We should remove first the 235Cap before, because it comes after the cap that we want
					//to remove so we will not harm the remove cap's index
					BYTE bIsRemoved;
					bIsRemoved = RemoveEncryptedCapFromCapBuffer(i);
					if(bIsRemoved)
						numOfRemovedCaps++;
				}

				RemoveOneCapset(pCapBuffer, i, algorithmCapCode);
				UpdateNumOfCaps(algorithmCapCode,eRole);
				numOfRemovedCaps++;


				// If one cap was removed, there is a need to update members and to build cap matrix from the start.
				//Build the matrix from the start.
				BuildCapMatrixFromSortedCapBuf(); //Since this function is been called only for local
				//caps and not for remote caps, we don't need to build the matrix from the start.
				BuildSortedCap();

				bIsNeedRestart = TRUE;
				//start from the begining - in case we want to remove the last cap and the previous one, if we
				//do not start from the begining we will not remove the last because we updated the num of caps
				//so we must start again.
				break;
			}

			// Move current pointer to the next cap. keep looking for the algorithm
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		} //for
	}//while

	return numOfRemovedCaps;
}

////////////////////////////////////////////////////////////////////////////
WORD CCapH323::RemoveTransmitCapsFromCapBuffer()
{
	WORD		numOfRemovedCaps	= 0;
	BOOL		bIsNeedRestart		= TRUE;
	BYTE		bIsEncryptedParty	= IsPartyEncrypted();
	BOOL        IsRemoveonlySpecificProfie = FALSE;


	while(bIsNeedRestart) //On encryption after each remove we should restart the search
	{
		capBuffer*	pCapBuffer	= (capBuffer *) &m_pCap->caps;
		char*		tempPtr		= (char *)pCapBuffer;

		bIsNeedRestart			= FALSE; //only if there is encrypted cap we should restart the search.
		// Look for the algorithm to remove in the caps buffer
		for (int i=0; i<m_numOfCaps; i++)
		{
			BYTE bRemoveThisCap = FALSE;
			// in order to know if it's ok to remove this cap we first check if the cap code is equals,
			// then if the role is equals.

			CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);

			if (pCap)
			{
				if(pCap->IsType(cmCapVideo) && pCap->GetRole() == kRolePeople && pCap->GetDirection() == cmCapTransmit)
				{
					bRemoveThisCap = TRUE;
				}
				else
					bRemoveThisCap = FALSE;



				POBJDELETE(pCap);
			}

			// if the cap is for remove do the following -
			if (bRemoveThisCap)
			{
				//In case of encryption we should remove either the 235cap that encrypt the remove cap.
				if(bIsEncryptedParty)
				{
					//in case the cap is encrypt remove the 235cap (encryptedPosition).
					//We should remove first the 235Cap before, because it comes after the cap that we want
					//to remove so we will not harm the remove cap's index
					BYTE bIsRemoved;
					bIsRemoved = RemoveEncryptedCapFromCapBuffer(i);
					if(bIsRemoved)
						numOfRemovedCaps++;
				}

				RemoveOneCapset(pCapBuffer, i, ((CapEnum)pCapBuffer->capTypeCode));
				UpdateNumOfCaps(((CapEnum)pCapBuffer->capTypeCode),kRolePeople);
				numOfRemovedCaps++;


				// If one cap was removed, there is a need to update members and to build cap matrix from the start.
				//Build the matrix from the start.
				BuildCapMatrixFromSortedCapBuf(); //Since this function is been called only for local
				//caps and not for remote caps, we don't need to build the matrix from the start.
				BuildSortedCap();

				bIsNeedRestart = TRUE;
				//start from the begining - in case we want to remove the last cap and the previous one, if we
				//do not start from the begining we will not remove the last because we updated the num of caps
				//so we must start again.
				break;
			}

			// Move current pointer to the next cap. keep looking for the algorithm
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		} //for
	}//while

	return numOfRemovedCaps;
}

//////////////////////////////////////////////////////////////////////////
void CCapH323::RemoveOneCapset(capBuffer* pCapBuffer, int capIndex, CapEnum algorithmCapCode)
{
	int currentCapLenght = pCapBuffer->capLength;

	// If the current cap is not the last cap in cap set remove this algorithm by copying rest of buffer over it.
	// If it is the last one, there is nothing to copy over it
	if (capIndex != (m_numOfCaps - 1))
	{
		// Find the current offset from start of cap sets
		DWORD lenghtFromStart = (DWORD)(pCapBuffer) - (DWORD)(&m_pCap->caps);
		DWORD lenghtTillEndOfCaps = m_offsetWrite - (lenghtFromStart + sizeof(capBufferBase) + currentCapLenght);
		capBuffer * pNextCapBuffer = (capBuffer *)((BYTE *)pCapBuffer + sizeof(capBufferBase) + currentCapLenght);
		ALLOCBUFFER(tempBuff, lenghtTillEndOfCaps);
		memcpy ((BYTE *)tempBuff,(BYTE *)pNextCapBuffer,lenghtTillEndOfCaps);
		memcpy ((BYTE *)pCapBuffer,(BYTE *)tempBuff,lenghtTillEndOfCaps);
		DEALLOCBUFFER(tempBuff);
	}

	// Decrease the algorithm's length from the offset
	m_offsetWrite -= currentCapLenght;
	m_capArray[algorithmCapCode]--;
	m_pCap->numberOfCaps--;
	m_numOfCaps--;
}


#define isCapInCapRange(_lowRange, _highRange, _cap) ((_cap <= _highRange) && (_cap >= _lowRange))
#define isSirenLPRCap(_cap) isCapInCapRange(eSirenLPR_32kCapCode, eSirenLPRStereo_128kCapCode, _cap)
#define isG719Cap(_cap) (isCapInCapRange(eG719_32kCapCode, eG719_128kCapCode, _cap) ||\
						 isCapInCapRange(eG719Stereo_64kCapCode, eG719Stereo_128kCapCode, _cap))

/////////////////////////////////////////////////////////////////////////////
// this function check if the parameters of the incoming channel is less or equal to our local abilities
BYTE CCapH323::IsChannelsParamsOK(const channelSpecificParameters *pParams, payload_en payloadType, CSecondaryParams &secParams,
								  const char *strChannelName,BYTE isAnnexes) const
{
	BYTE	bRes = FALSE;
	DWORD   details;

	CBaseCap *pChannelCap = CBaseCap::AllocNewCap((BYTE *)pParams);

	if (pChannelCap)
    {
		CCapSetInfo channelCapInfo = pChannelCap->GetCapCode();
		CBaseCap	*pTmpCap = NULL;	//for the reject string
		uint32_t 	isChannelSirenLPRCap = isSirenLPRCap(pChannelCap->GetCapCode());
		bool	 	isChannelG719Cap = isG719Cap(pChannelCap->GetCapCode());

		if(m_capArray[(CapEnum)channelCapInfo] || isChannelSirenLPRCap || isChannelG719Cap)
	    {
			CSmallString msg;

			BYTE bCheckWasDone = FALSE; // if we called the IsContaining function

		    capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
		    char* tempPtr = (char *)pCapBuffer;

	        for (int i = 0; (i < m_numOfCaps) && (bRes == FALSE); i++)
			{

				CCapSetInfo currentCapInfo = (CapEnum)pCapBuffer->capTypeCode;

	            if(((CapEnum)currentCapInfo == (CapEnum)channelCapInfo) ||
	            	(isSirenLPRCap((CapEnum)currentCapInfo) && isChannelSirenLPRCap && (CapEnum)currentCapInfo >= (CapEnum)channelCapInfo) ||
	            	(isG719Cap((CapEnum)currentCapInfo) && isChannelG719Cap && (CapEnum)currentCapInfo >= (CapEnum)channelCapInfo))
				{

					CBaseCap * pCurrentCap = CBaseCap::AllocNewCap((CapEnum)currentCapInfo,pCapBuffer->dataCap);

					// is channel params OK check if the parameters are part of the capability
					if (pCurrentCap && pCurrentCap->IsDirection(cmCapReceive))
					{

						uint32_t isLocalSirenLPRCap = isSirenLPRCap(pCurrentCap->GetCapCode());
						uint32_t isRemoteLocalSirenLPRCapMatch = (isChannelSirenLPRCap && isLocalSirenLPRCap && (pCurrentCap->GetCapCode() > pChannelCap->GetCapCode()));

						bool isLocalG719Cap = isG719Cap(pCurrentCap->GetCapCode());
						bool isRemoteLocalG719CapMatch = (isChannelG719Cap && isLocalG719Cap && (pCurrentCap->GetCapCode() > pChannelCap->GetCapCode()));

						if(((pCurrentCap->GetCapCode() == pChannelCap->GetCapCode()) || isRemoteLocalSirenLPRCapMatch || isRemoteLocalG719CapMatch) && CheckRole(pChannelCap->GetRole(),i,pCurrentCap))
						{
							details = 0x00000000;

							if ((pChannelCap->GetCapCode() == eH263CapCode))
								bRes = !((CH263VideoCap*)pChannelCap)->IsOnlySqcifMpi(); //we shell reject channel that has only sqcif resulation.
							else
								bRes = TRUE;

							if(isAnnexes)
								bRes = pCurrentCap->IsContaining(*pChannelCap,kBitRate|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS| kH264Additional_FS|kRoleLabel| kMaxFR | kH264Mode,&details);
							else
								bRes = pCurrentCap->IsContaining(*pChannelCap,kBitRate|kFrameRate|kH264Additional_MBPS| kH264Additional_FS|kRoleLabel| kMaxFR | kH264Mode,&details);
							bCheckWasDone = TRUE;
							cmCapDataType eType = pCurrentCap->GetType();
							DumpDetailsToStream(eType,details,msg);

							POBJDELETE(pTmpCap);
							pTmpCap		= CBaseCap::AllocNewCap((CapEnum)currentCapInfo,pCapBuffer->dataCap);

	//						if (bRes)
	//							PTRACE2INT2(eLevelInfoNormal,"CCapH323::IsChannelsParamsOK: Channel param OK. Contained in cap set #%d",i);
						}
					}

					POBJDELETE(pCurrentCap);
				}

	            tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
	            pCapBuffer = (capBuffer*)tempPtr;
			}

			// if the caps are not containing the incoming channel (we called the IsContaining function)
			// we will need to print the details
			if (bCheckWasDone && (bRes == FALSE))
			{
				PTRACE2(eLevelInfoNormal,"CCapH323::IsChannelsParamsOK: Failure details:\n",msg.GetString());
				if((pChannelCap->GetCapCode() >= eH261CapCode) && (pChannelCap->GetCapCode() < eT120DataCapCode))
				{
					pChannelCap->GetDiffFromDetails(details,secParams);
					if( pTmpCap )
						pTmpCap->GetMediaParams(secParams,details);
				}
			}
			POBJDELETE(pTmpCap);
	    }
    }
	POBJDELETE(pChannelCap);
	return bRes;
}

///////////////////////////////////////////////////////////////////////////////
void CCapH323::IntersectionParameters(CComModeH323 *pInitial323,BYTE *pFirstCap,BYTE *pSecondCap,CapEnum type,ERoleLabel label) const
{
	CBaseVideoCap	*pFirstVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(type,pFirstCap);
	CBaseVideoCap	*pSecondVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(type,pSecondCap);

	if (pFirstVideoCap && pSecondVideoCap && ((pFirstVideoCap->GetRole() == label) && (pSecondVideoCap->GetRole() == label)))
	{
		int sizeOfNewStruct = pFirstVideoCap->SizeOfBaseStruct();
		BYTE* pNewStruct	= new BYTE[sizeOfNewStruct];
		memset(pNewStruct,0,sizeOfNewStruct);
		CBaseVideoCap* pIntersectVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(type, pNewStruct);
		if (pIntersectVideoCap)
		{
			EResult eResOfSet = kSuccess;
			eResOfSet &= pIntersectVideoCap->SetDefaults(cmCapTransmit, pFirstVideoCap->GetRole());
			eResOfSet &= pIntersectVideoCap->CopyBaseQualities(*pFirstVideoCap);

			if (eResOfSet)
			{
				BaseCapStruct* pIntersectStruct = pIntersectVideoCap->GetStruct();
				BYTE bIsIntersect = pFirstVideoCap->Intersection(*pSecondVideoCap, (BYTE**)(&pIntersectStruct) );

				if(bIsIntersect)
				{
					BaseCapStruct* pPrevStruct = pIntersectVideoCap->GetStruct();
					if (pIntersectStruct != pPrevStruct)
					{
					//it happens in case of 263+, in which Intersection does reallocate to pIntersectStruct,
						pIntersectVideoCap->SetStruct(pIntersectStruct);
					}

					pInitial323->SetMediaMode(type, pIntersectVideoCap->SizeOf(), (BYTE*)pIntersectStruct, cmCapVideo,cmCapTransmit,label);
				}
			}
			pIntersectVideoCap->FreeStruct();
			POBJDELETE(pIntersectVideoCap);
		}
	}

	POBJDELETE(pSecondVideoCap);
	POBJDELETE(pFirstVideoCap);
}

///////////////////////////////////////////////////////////////////////////////
DWORD CCapH323::CompareParameters(BYTE *pFirstCap, BYTE *pSecondCap,DWORD valuesToCompare,CapEnum type) const
{
	BYTE	bRes	= FALSE;
	DWORD   details	= 0x00000000;
	CBaseVideoCap *pFirstVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(type, pFirstCap);
	CBaseVideoCap *pSecondVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(type, pSecondCap);

	if (pFirstVideoCap && pSecondVideoCap)
		bRes = pFirstVideoCap->IsContaining(*pSecondVideoCap,valuesToCompare,&details);

	POBJDELETE(pSecondVideoCap);
	POBJDELETE(pFirstVideoCap);

	return details;
}

///////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IntersectionH263Annex(CVidModeH323 *pXmitMode,h263CapStruct *pSecondH263Cap, h263CapStruct *pLocalH263Cap) const
{
	CH263VideoCap *pFirstVideoCap = NULL;
	if(pLocalH263Cap != NULL)
		pFirstVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pLocalH263Cap);
	else
		pFirstVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pXmitMode->GetDataCap());
	CH263VideoCap *pSecondVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pSecondH263Cap);

	DWORD	firstSec	= 0x00000000;
	DWORD	secFirst	= 0x00000000;
	BYTE	eRes		= TRUE;
	if (pFirstVideoCap && pSecondVideoCap)
	{
		//First we will check if both have the same annexes only if not i will do intersection.
		pFirstVideoCap->IsContaining(*pSecondVideoCap,kAnnexes|kFormat,&firstSec);
		pSecondVideoCap->IsContaining(*pFirstVideoCap,kAnnexes|kFormat,&secFirst);

		//if one is not nullify it means that there is problem with the annexes so we should do a intersection.
		if ((firstSec) || (secFirst))
		{
			//If we got here we know that there is asymmetric between the annexes so if we exit and we don't found a match the cap
			//are not even so we do not need to continue to compare.
	   	    int sizeOfNewStruct = sizeof(h263CapStruct);
			BYTE* pNewStruct	= new BYTE[sizeOfNewStruct];
		    CH263VideoCap* pIntersectVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode, pNewStruct);

			EResult eResOfSet = kSuccess;
			if (pIntersectVideoCap)
			{
				eResOfSet &= pIntersectVideoCap->SetDefaults(cmCapTransmit, pFirstVideoCap->GetRole());
				//We should copy the 263 base structure without the annexes and the custom format.
				eResOfSet &= pIntersectVideoCap->CopyBaseQualities(*pFirstVideoCap);

				if (eResOfSet)
				{
					h263CapStruct* pIntersectStruct = (h263CapStruct *)pIntersectVideoCap->GetStruct();
					pFirstVideoCap->IntersectionAnnexesAndCustomFormats(*pSecondVideoCap, &pIntersectStruct);
					h263CapStruct* pPrevStruct = (h263CapStruct *)pIntersectVideoCap->GetStruct();
					if (pIntersectStruct != pPrevStruct)
					{
					//it happens in case of 263+, in which Intersection does reallocate to pIntersectStruct,
						pIntersectVideoCap->SetStruct((BaseCapStruct*)pIntersectStruct);
					}

					pXmitMode->UpdateParams(pIntersectVideoCap);
				}
				pIntersectVideoCap->FreeStruct();
				POBJDELETE(pIntersectVideoCap);
			}
		}
		else if(pLocalH263Cap)//One containes the other so I should intiate the XmitMode
			pXmitMode->UpdateParams(pFirstVideoCap);
	}

	POBJDELETE(pSecondVideoCap);
	POBJDELETE(pFirstVideoCap);

	return eRes;
}

///////////////////////////////////////////////////////////////////////////////
void CCapH323::AddOneCapToOther(CVidModeH323 *pXmitInitiate,h263CapStruct *pFirstH263Cap,h263CapStruct *pSecondH263Cap,CapEnum type,ERoleLabel label) const
{
	CH263VideoCap	*pFirstVideoCap		= (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pFirstH263Cap);
	CH263VideoCap	*pSecondVideoCap	= (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pSecondH263Cap);

	int sizeOfNewStruct = sizeof(h263CapStruct);
	BYTE* pNewStruct	= new BYTE[sizeOfNewStruct];
	CH263VideoCap* pUnionCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode, pNewStruct);

	if (pUnionCap && pFirstVideoCap && pSecondVideoCap)
	{
		pFirstVideoCap->AddOneCapToOther(*pSecondVideoCap,(h263CapStruct *)pUnionCap->GetStruct());
		if(pXmitInitiate->GetDataCap() == NULL)
			pXmitInitiate->UpdateParams(pUnionCap);
		else
		{
			//we shouldn't copy the annexes because we already did an intersection with the annexes.
			CH263VideoCap *pInitalVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pXmitInitiate->GetDataCap());
			if (pInitalVideoCap)
			{
				pInitalVideoCap->CopyBaseQualities(*pUnionCap);
				POBJDELETE(pInitalVideoCap);
			}
			else
				PTRACE(eLevelInfoNormal,"CCapH323::AddOneCapToOther: pInitalVideoCap is NULL");
		}
	}

	POBJDELETE(pSecondVideoCap);
	POBJDELETE(pFirstVideoCap);

	if (pUnionCap)
	{
		pUnionCap->FreeStruct();
		POBJDELETE(pUnionCap);
	}
}

///////////////////////////////////////////////////////////////////////////////
void CCapH323::ImproveH263Parameters(CComModeH323 *pInitial323,h263CapStruct *pFirstH263Cap,h263CapStruct *pSecondH263Cap,ERoleLabel label) const
{
	CH263VideoCap	*pVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pSecondH263Cap);
	CVidModeH323	*pImproveCap = new CVidModeH323((BaseCapStruct*)pFirstH263Cap,eH263CapCode);

	if (pVideoCap)
	{
		h263CapStruct *pImproveDataCap = (h263CapStruct *)pImproveCap->GetDataCap();
		if (pImproveDataCap)
			pVideoCap->ImproveCap(pImproveDataCap);
		pInitial323->SetMediaMode(eH263CapCode,pImproveCap->SizeOf(),pImproveCap->GetDataCap(),cmCapVideo,cmCapTransmit,label);
	}

	POBJDELETE(pVideoCap);
	POBJDELETE(pImproveCap);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Builds the H261 capabilities structure according to 323 standard.
//             Returns the number of video caps it has found.
//---------------------------------------------------------------------------------------------------
/*WORD CCapH323::SetH261Cap(const CCapH221& h221Cap, DWORD videoRate, BOOL bOnlyOneResolution, BOOL ConfType)
{
	WORD numOfCaps = 0;
	CCapSetInfo capInfo = eeH261CapCode;
	CH261VideoCap* pVideoCap = (CH261VideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,NULL);
	if (pVideoCap)
	{
		EResult eResOfSet = kSuccess;
		if (bOnlyOneResolution == FALSE)
			eResOfSet &= pVideoCap->SetFromH221Cap(h221Cap);//set all the resolutions
		else
		{
			eResOfSet &= pVideoCap->SetDefaults(cmCapReceive);
			eResOfSet &= pVideoCap->SetFromH221Cap(h221Cap,kCif);
			if (eResOfSet == kFailure)
				eResOfSet = pVideoCap->SetFromH221Cap(h221Cap,kQCif);
		}

		if (eResOfSet)
		{
			eResOfSet &= pVideoCap->SetStillImageTransmission(TRUE);
			eResOfSet &= pVideoCap->SetBitRate(videoRate);
		}

		int structSize = pVideoCap->SizeOf();
		if (eResOfSet && structSize)
		{
			AddCapToCapBuffer(capInfo,structSize,(char*)pVideoCap->GetStruct());
			m_capArray[(CapEnum)capInfo]++;
			numOfCaps++;
			m_numOfCaps++;
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetH261Cap: Set struct has failed!!!");

		// only for 4X4!!
		if (ConfType == CP)
		{
			CH261VideoCap * p4x4VideoCap = (CH261VideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,NULL);
			if (p4x4VideoCap)
			{
				eResOfSet &= p4x4VideoCap->SetDefaults(cmCapTransmit);
				eResOfSet &= p4x4VideoCap->SetBitRate(videoRate);
				eResOfSet &= p4x4VideoCap->SetFormatMpi(kCif,h221Cap.m_qCifMpi - 21);
			}
			int structSize = pVideoCap->SizeOf();
			if (eResOfSet && structSize)
			{
				AddCapToCapBuffer(capInfo,structSize,(char*)p4x4VideoCap->GetStruct());
				m_capArray[(CapEnum)capInfo]++;
				numOfCaps++;
				m_numOfCaps++;
			}
			p4x4VideoCap->FreeStruct();
			POBJDELETE(p4x4VideoCap);
		}
	}
	pVideoCap->FreeStruct();
	POBJDELETE(pVideoCap);
	return numOfCaps;
}*/

/////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::SetH261DefaultCap(DWORD videoRate,const char* pPartyName)
{
	WORD numOfCaps = 0;
	CH261VideoCap* pVideoCap = (CH261VideoCap *)CBaseCap::AllocNewCap(eH261CapCode, NULL);
	if (pVideoCap)
	{
		APIS8 qcifMpi, cifMpi;
		::Get261VideoCardMPI(videoRate/10, &qcifMpi, &cifMpi, m_videoQuality);

		EResult eResOfSet = kSuccess;
		eResOfSet &= pVideoCap->SetDefaults(cmCapReceive);
		eResOfSet &= pVideoCap->SetFormatMpi(kQCif, qcifMpi);
		if(pPartyName && strstr(pPartyName, "##FORCE_MEDIA_VIDEO_H261_QCIF_ONLY") == NULL)
			eResOfSet &= pVideoCap->SetFormatMpi(kCif, cifMpi);

		if (eResOfSet)
		{
			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			BOOL bIsStillImageTransmissionSupported = 0;
			std::string key = "H261_StillImageTransmission";
			pSysConfig->GetBOOLDataByKey(key, bIsStillImageTransmissionSupported);

			eResOfSet &= pVideoCap->SetStillImageTransmission(bIsStillImageTransmissionSupported);
			eResOfSet &= pVideoCap->SetBitRate(videoRate);
		}

		int structSize = pVideoCap->SizeOf();
		if (eResOfSet && structSize)
		{
			AddCapToCapBuffer(eH261CapCode, structSize,pVideoCap->GetStruct());
			m_capArray[eH261CapCode]++;
			numOfCaps++;
			m_numOfCaps++;
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetH261DefaultCap: Set struct has failed!!!");

		pVideoCap->FreeStruct();
		POBJDELETE(pVideoCap);
	}

	return numOfCaps;
}

///////////////////////////////////////////////////////////////////////////////
/*WORD CCapH323::SetH263Cap(const CCapH221& h221Cap, DWORD videoRate,BOOL ConfType, const CComMode& scm)
{
	WORD numOfCaps = 0;
	CH263VideoCap* pVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eeH263CapCode,NULL);
	BYTE bIsQuad = ((ConfType == CP)&&
					 h221Cap.GetCapH263().IsVidImageFormat(kQCif) &&
					 (!(h221Cap.GetCapH263().IsVidImageFormat(kCif))));

	if (pVideoCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet &= pVideoCap->SetFromH221Cap(h221Cap,cmCapReceive,bIsQuad);
		eResOfSet &= pVideoCap->SetBitRate(videoRate);
		if (eResOfSet)
		{
			int structSize = pVideoCap->SizeOf();
			AddCapToCapBuffer(eeH263CapCode, structSize, (char*)pVideoCap->GetStruct());
			m_capArray[eeH263CapCode]++;
			numOfCaps++;
			m_numOfCaps++;

			if (pVideoCap->IsH263Plus())
			{
				eResOfSet = kSuccess;
				for(EFormat i = k16Cif;i != kUnknownFormat; i--)
					eResOfSet &= pVideoCap->SetFromH221Cap(h221Cap,i);
				eResOfSet &= pVideoCap->SetBitRate(videoRate);

				int structSize = pVideoCap->SizeOf();
				if (eResOfSet && structSize)
				{
					AddCapToCapBuffer(eeH263CapCode, structSize, (char*)pVideoCap->GetStruct());
					m_capArray[eeH263CapCode]++;
					numOfCaps++;
					m_numOfCaps++;
				}
			}

			// only for 4X4!!
			if(bIsQuad)
			{
				CH263VideoCap* p4x4VideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eeH263CapCode,NULL);
				if (p4x4VideoCap)
				{
					eResOfSet &= p4x4VideoCap->SetDefaults(cmCapTransmit);
					eResOfSet &= p4x4VideoCap->SetBitRate(videoRate);
					int qcifMpi = h221Cap.GetCapH263().GetFormatMPI(kQCif) + 1;

					if (scm.m_vidMode.GetVidFormat() == H263_CIF_4)
					{//add 4Cif
						WORD isdnMpi = scm.m_vidMode.GetH263Mpi();
						CCapSetInfo capInfo = eeH263CapCode;
						int h323Mpi	= capInfo.TranslateIsdnMpiToIp(isdnMpi);;
						eResOfSet &= p4x4VideoCap->SetFormatMpi(k4Cif, h323Mpi);
					}
					eResOfSet &= p4x4VideoCap->SetFormatMpi(kCif,qcifMpi);
				}
				int structSize = pVideoCap->SizeOf();
				if (eResOfSet && structSize)
				{
					AddCapToCapBuffer(eeH263CapCode,structSize,(char*)p4x4VideoCap->GetStruct());
					m_capArray[eeH263CapCode]++;
					numOfCaps++;
					m_numOfCaps++;
				}
				p4x4VideoCap->FreeStruct();
				POBJDELETE(p4x4VideoCap);
			}
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetH263Cap: Set struct has failed!!!");
	}

	pVideoCap->FreeStruct();
	POBJDELETE(pVideoCap);
	return numOfCaps;
}*/

///////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::SetH263DefaultCap(DWORD videoRate, EConfType confType, DWORD callRate, BYTE isRemoveOtherThenQCif)
{
	WORD numOfCaps = 0;
	CH263VideoCap* pVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,NULL);

	if (pVideoCap)
	{
		EResult eResOfSet = kSuccess;
		pVideoCap->SetDefaults(cmCapReceive);
		eResOfSet &= pVideoCap->SetBitRate(videoRate);

		//struct number 1:
		if (confType == kCp || confType == kCop)
		{
			//FROM 7.2 VIDEO RATE IS ACTUALLY CALL RATE WE NEED TO ADJUST THE RATE TO VIDEO RATE
			DWORD audioRate 		= CalculateAudioRate((videoRate*100));
			DWORD actualVideoRate 	= videoRate - (audioRate/100);
			pVideoCap->SetHighestCapForCpFromScmAndCardValues(actualVideoRate, m_videoQuality);//pass info in K bits
		}
		else //VSW
			pVideoCap->SetHighestCapForVswFromScmAndCardValues();

		if(isRemoveOtherThenQCif)
		{
			PTRACE(eLevelInfoNormal,"CCapH323::SetH263DefaultCap - Remote can't other resolution then QCIF");
			for (EFormat index = kCif; index <= kQVGA; index++)
			{
				INT8 currentFormat = pVideoCap->GetFormatMpi(index);

				if(currentFormat > 0)
					pVideoCap->SetFormatMpi(index, -1);
			}
		}

		//yael: add interlaced mode:
		/*if (pVideoCap->IsInterlaced())
			numOfCaps += SetDropFieldCap();*/

		if (eResOfSet)
		{
			int structSize = pVideoCap->SizeOf();
			AddCapToCapBuffer(eH263CapCode, structSize, pVideoCap->GetStruct());
			m_capArray[eH263CapCode]++;
			numOfCaps++;
			m_numOfCaps++;

			if (pVideoCap->IsH263Plus())
			{
				//struct number 2:
				CH263VideoCap* pSecondVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,NULL);
				if (pSecondVideoCap)
				{
					eResOfSet = kSuccess;
					pSecondVideoCap->SetDefaults(cmCapReceive);
					for(EFormat i = k16Cif;i != kUnknownFormat; i--)
						eResOfSet &= pSecondVideoCap->SetFormatMpi(i, 1);
					eResOfSet &= pSecondVideoCap->SetBitRate(videoRate);

					if(isRemoveOtherThenQCif)
					{
						PTRACE(eLevelInfoNormal,"CCapH323::SetH263DefaultCap - Remote can't other resolution then QCIF");
						for (EFormat index = kCif; index <= kQVGA; index++)
						{
							INT8 currentFormat = pVideoCap->GetFormatMpi(index);

							if(currentFormat > 0)
								pVideoCap->SetFormatMpi(index, -1);
						}
					}

					int structSize = pSecondVideoCap->SizeOf();
					if (eResOfSet && structSize)
					{
						AddCapToCapBuffer(eH263CapCode, structSize, pSecondVideoCap->GetStruct());
						m_capArray[eH263CapCode]++;
						numOfCaps++;
						m_numOfCaps++;
					}

					pSecondVideoCap->FreeStruct();
					POBJDELETE(pSecondVideoCap);
				}
			}
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetH263DefaultCap: Set struct has failed!!!");

		pVideoCap->FreeStruct();
		POBJDELETE(pVideoCap);
	}

	return numOfCaps;
}

///////////////////////////////////////////////////////////////////////////////
/*WORD CCapH323::SetH264Cap(const CCapH221& h221Cap, DWORD videoRate, BOOL ConfType)
{
	WORD numOfCaps = 0;
	CH264VideoCap* pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eeH264CapCode,NULL);

	if (pVideoCap)
	{
		EResult eResOfSet = kSuccess;

		CCapH264 h264Cap = h221Cap.GetCapH264();
		WORD numOfSets = h264Cap.GetNumberOfH264Sets();

		if (numOfSets)
		{
			for (WORD currSetNum = 0; currSetNum < numOfSets; currSetNum++)
			{//each isdn capSet is translated into ip structure
				eResOfSet &= pVideoCap->SetFromH221Cap(h221Cap, currSetNum, cmCapReceive);
				eResOfSet &= pVideoCap->SetBitRate(videoRate);

				int structSize = pVideoCap->SizeOf();
				if (eResOfSet && structSize)
				{
					AddCapToCapBuffer(eH264CapCode, structSize, (char*)pVideoCap->GetStruct());
					m_capArray[eeH264CapCode]++;
					numOfCaps++;
					m_numOfCaps++;
				}
			}

			// only for 4X4!! CP + level 15 equal to QCIF
			if((ConfType == CP) && (pVideoCap->GetLevel() == H264_Level_1))
			{
				CH264VideoCap* p4x4VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode,NULL);
				if (p4x4VideoCap)
				{
					eResOfSet &= p4x4VideoCap->SetDefaults(cmCapTransmit);
					eResOfSet &= p4x4VideoCap->SetBitRate(videoRate);
					p4x4VideoCap->SetLevel(H264_Level_1_2);
				}
				int structSize = pVideoCap->SizeOf();
				if (eResOfSet && structSize)
				{
					AddCapToCapBuffer(eH264CapCode,structSize,(char*)p4x4VideoCap->GetStruct());
					m_capArray[eH264CapCode]++;
					numOfCaps++;
					m_numOfCaps++;
				}
				p4x4VideoCap->FreeStruct();
				POBJDELETE(p4x4VideoCap);
			}

		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetH264Cap: No H264 Set in H320 caps");
	}

	else
		PTRACE(eLevelInfoNormal,"CCapH323::SetH264Cap: Set struct has failed!!!");

	pVideoCap->FreeStruct();
	POBJDELETE(pVideoCap);
	return numOfCaps;
}*/
/////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::SetH264DefaultCap(DWORD videoRate)
{
	WORD numOfCaps = 0;
	CH264VideoCap* pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode,NULL);

	if (pVideoCap)
	{
		EResult eResOfSet = kSuccess;

		eResOfSet &= pVideoCap->SetDefaults();
		eResOfSet &= pVideoCap->SetBitRate(videoRate);
		pVideoCap->SetLevel(MAX_H264_LEVEL_SUPPORTED_IN_VSW);//level 2
		pVideoCap->SetFs(GetMaxFsAsDevision(MAX_FS_SUPPORTED_IN_VSW));

		if (eResOfSet)
		{
			int structSize = pVideoCap->SizeOf();
			if (eResOfSet && structSize)
			{
				AddCapToCapBuffer(eH264CapCode, structSize, pVideoCap->GetStruct());
				m_capArray[eH264CapCode]++;
				numOfCaps++;
				m_numOfCaps++;
			}
		}

		pVideoCap->FreeStruct();
		POBJDELETE(pVideoCap);
	}

	else
		PTRACE(eLevelInfoNormal,"CCapH323::SetH264DefaultCap: Set struct has failed!!!");

	return numOfCaps;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Builds the H261 capabilities structure according to 323 standard.
//             Returns the number of video caps it has found.
//---------------------------------------------------------------------------------------------------
/*WORD CCapH323::SetH26LCap(const CCapH221& h221Cap, DWORD videoRate, BOOL ConfType)
{
	WORD numOfCaps = 0;
	CCapSetInfo capInfo = eH26LCapCode;
	CH26LVideoCap* pVideoCap = (CH26LVideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,NULL);
	// CGenericVideoCap
	if (pVideoCap)
	{
		EResult eResOfSet = kSuccess;
		int structSize	  = 0;
		eResOfSet &= pVideoCap->SetDefaults(cmCapReceive);

		for (EFormat i = k4Cif; i != kUnknownFormat; i--)
		{
			WORD h320FormatType = capInfo.GetH320Format(i);
			if(h320FormatType != (WORD)NA)
			{
				if(h221Cap.IsH26LMpi(h320FormatType))
				{// all the H26L has to be in the same structure.
					eResOfSet &= pVideoCap->SetFromH221Cap(h221Cap,i);
					eResOfSet &= pVideoCap->SetBitRate(videoRate);
					structSize = pVideoCap->SizeOf();
				}
			}
		}

		if (eResOfSet && structSize)
		{
			AddCapToCapBuffer(capInfo,structSize,(char*)pVideoCap->GetStruct());
			m_capArray[(CapEnum)capInfo]++;
			numOfCaps++;
			m_numOfCaps++;
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetH26LCap: Set struct has failed!!!");
	}
	pVideoCap->FreeStruct();
	POBJDELETE(pVideoCap);
	return numOfCaps;
}*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCapH323::CheckDBC2()
{
	capBuffer		*pCapBuffer	= (capBuffer *) &m_pCap->caps;
	char			*pTempPtr;
	BOOL			bIsDBC2 = FALSE;
	int				altNumber;

	CDBC2VideoCap *pDBC2VideoCap = NULL;
	for(int k=0; k < m_numOfCaps; k++)
	{
		CapEnum capCode = (CapEnum)((BaseCapStruct*)pCapBuffer->dataCap)->header.capTypeCode;

		if (capCode == eDBC2CapCode)
		{
			altNumber = FindAltNumber(k,capCode);
			if(altNumber != -1)
			{
				pDBC2VideoCap = (CDBC2VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,pCapBuffer->dataCap);
				if (pDBC2VideoCap)
				{
					genericVideoCapStruct *pDBC2Cap = (genericVideoCapStruct *)pDBC2VideoCap->GetStruct();
					if(pDBC2Cap->data && DBC2_MotionVectors_Options)
					{
						bIsDBC2 = TRUE;
						POBJDELETE(pDBC2VideoCap);
						break;
					}
					POBJDELETE(pDBC2VideoCap);
				}
			}
		}

		pTempPtr = (char*)pCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return bIsDBC2;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Builds the DBC2 capabilities structure according to 323 standard.
//             Returns the number of video caps it has found.
//---------------------------------------------------------------------------------------------------
WORD CCapH323::SetDBC2Cap()
{
	WORD numOfCaps = 0;
	CCapSetInfo capInfo = eDBC2CapCode;
	CDBC2VideoCap* pVideoCap = (CDBC2VideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,NULL);
	// CGenericVideoCap
	if (pVideoCap)
	{
		EResult eResOfSet = kSuccess;
		int structSize	  = 0;
		eResOfSet &= pVideoCap->SetDefaults(cmCapReceive);
		structSize = pVideoCap->SizeOf();
		m_bIsDBC2 = TRUE;

		if (eResOfSet && structSize)
		{
			AddCapToCapBuffer(capInfo,structSize, pVideoCap->GetStruct());
			m_capArray[(CapEnum)capInfo]++;
			numOfCaps++;
			m_numOfCaps++;
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetDBC2Cap: Set struct has failed!!!");

		pVideoCap->FreeStruct();
		POBJDELETE(pVideoCap);
	}

	return numOfCaps;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Adds the protocol to cap set. only g7221 for now
//---------------------------------------------------------------------------------------------------
//WORD  CCapH323::AddProtocolToCapSet(payload_en payloadType)
//{
//	WORD numOfAddedCap  = 0;
//	CCapSetInfo capInfo = payloadType;
//
//	PTRACE(eLevelInfoNormal,"CCapH323::AddProtocolToLocalCapSet");
//
//	// (in case of PicterTel EP (Stinger) add the G.7221 to the capability Set)
//	if (capInfo.IsPayloadType(_G7221) && capInfo.IsSupporedCap())
//	{
//		// --------------------------------------------------------------------
//		// Add G7221 (24K rate & 32K rate) at the begining of capability table.
//		// --------------------------------------------------------------------
//
//		// Add to the end of caps table
//		if ((m_isG7221_24wanted == FALSE)&&(m_isG7221_32wanted == FALSE))
//		{
//			// Au-24k
//			numOfAddedCap += SetAudioCap(g7221_24kCapCode);
//			// Au-32k
//			numOfAddedCap += SetAudioCap(g7221_32kCapCode);
//		}
//		// else: g7221 is wanted at the begining of cap table
//		else
//		{
//			ctCapabilitiesStruct * pOldCap;
//			WORD                   oldOffsetWrite;
//
//			// Keep old data
//			pOldCap         = m_pCap;
//			oldOffsetWrite  = m_offsetWrite;
//
//			// Check if there is enough space, if not enlarge the size
//			if((2*sizeof(audioCapStructBase) + oldOffsetWrite) >= GetCurrentCapsBufSize())
//				m_size += 2*(sizeof(audioCapStructBase) + sizeof(capBufferBase));
//
//			m_pCap	              = (ctCapabilitiesStruct*) new BYTE[m_size];
//			m_offsetWrite         = 0;
//
//			// Copy old data
//			m_pCap->bUseAAL5      = pOldCap->bUseAAL5;
//			m_pCap->maxNTUSize    = pOldCap->maxNTUSize;
//			m_pCap->trafficType   = pOldCap->trafficType;
//			m_pCap->transportType = pOldCap->transportType;
//			m_pCap->numberOfAlts  = pOldCap->numberOfAlts;
//			m_pCap->numberOfSim   = pOldCap->numberOfSim;
//			memcpy(m_pCap->filler,pOldCap->filler,sizeof(pOldCap->filler));
//
//			// Put g7221, 24K rate, first, then G7221 34K rate
//			if (m_isG7221_24wanted == TRUE)
//			{
//				// Au-24k
//				numOfAddedCap += SetAudioCap(g7221_24kCapCode);
//				m_capArray[g7221_24kCapCode] += numOfAddedCap;
//				// Au-32k
//				numOfAddedCap = 0;
//				numOfAddedCap += SetAudioCap(g7221_32kCapCode);
//				m_isG7221_24wanted = FALSE;
//			}
//			// Put g7221, 34K rate, first
//			else if (m_isG7221_32wanted == TRUE)
//			{
//				// Au-32k
//				numOfAddedCap += SetAudioCap(g7221_32kCapCode);
//				m_capArray[g7221_32kCapCode] += numOfAddedCap;
//				// Au-24k
//				numOfAddedCap = 0;
//				numOfAddedCap += SetAudioCap(g7221_24kCapCode);
//				capInfo = g7221_24kCapCode;
//				m_isG7221_32wanted = FALSE;
//			}
//
//			// Copy all old caps after g7221 and update offset and amount.
//			memcpy((BYTE *)&m_pCap->caps + m_offsetWrite,(BYTE *)&pOldCap->caps,oldOffsetWrite);
//			m_offsetWrite += oldOffsetWrite;
//
//			delete []pOldCap;
//		}
//	}
//
//	// If one cap was added, there is a need to update members
//	// and to build cap matrix and sorted cap array from the start.
//	if(numOfAddedCap != 0)
//	{
//		// Update cap array
//		m_capArray[(CapEnum)capInfo] += numOfAddedCap;
//
//
//		if (capInfo.IsType(cmCapAudio))
//			m_numOfAudioCap	   += numOfAddedCap;
//		else if (capInfo.IsType(cmCapVideo))
//			m_numOfVideoCap	   += numOfAddedCap;
//		else if (capInfo.IsType(cmCapData))
//		{
//			if(capInfo.GetH323CapCode() == t120DataCapCode)
//				m_numOfT120Cap--;
//			else
//				m_numOfFeccCap--;
//		}
//		else if (capInfo.IsType(cmCapNonStandard))
//			m_numOfNsCap       += numOfAddedCap;
//		else if (capInfo.IsCapCode(PeopleContentCapCode))
//			m_numOfPeopContCap += numOfAddedCap;
//
//        m_pCap->numberOfCaps = m_numOfCaps;
//
//        // Build the matrix and sorted cap array from the start
//		  BuildCapMatrixFromSortedCapBuf(); //Since this function is been called only for local
//        BuildSortedCap();
//    }
//
//	return numOfAddedCap;
//}
/////////////////////////////////////////////////////////////////////////////
void  CCapH323::RemoveOtherProtocols(CapEnum h323CapCode, ERoleLabel eRole)
{
//	PTRACE2INT2(eLevelInfoNormal,"CCapH323::RemoveOtherProtocols - CapCode = %d",h323CapCode);

	CCapSetInfo capInfo(h323CapCode);
	cmCapDataType requestedDataType = capInfo.GetCapType();

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char* tempPtr = (char *)pCapBuffer;

	// Look for the algorithm to remove in the caps buffer
	for (int i=0; i<m_numOfCaps; i++)
	{
		CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pCap)
		{
			CCapSetInfo currCapInfo = (CapEnum)pCapBuffer->capTypeCode;
			cmCapDataType currDataType = currCapInfo.GetCapType();
			if( requestedDataType == currDataType )
			{//same dataType and same role:
				BYTE bIsGoodRole = TRUE;
				if(currDataType == cmCapVideo)
					bIsGoodRole = CheckRole(eRole,i,pCap);

				if (bIsGoodRole && (h323CapCode != pCapBuffer->capTypeCode)) //different algorithm
				{
					payload_en currPayloayType = currCapInfo.GetPayloadType();
					RemoveProtocolFromCapSet(currPayloayType);
				}
			}
		}
		POBJDELETE(pCap);

		// Move current pointer to the next cap. keep looking for the algorithm
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Function name:  RemoveProtocolFromCapSet                 written by: atara
// Variables:      paloadType: The payload type to be removed.
// Description:	   Remove protocol form cap set and updates class members correspondently.
// Return value:   number of caps removed
/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::RemoveProtocolFromCapSet(payload_en payloadType)
{
//	PTRACE2INT2(eLevelInfoNormal,"CCapH323::RemoveProtocolFromCapSet - payload type = %d", payloadType);
	return RemoveProtocolFromCapSet(payloadType, eUnknownAlgorithemCapCode);
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::RemoveProtocolFromCapSet(CapEnum capEnum)
{
//	PTRACE2INT2(eLevelInfoNormal,"CCapH323::RemoveProtocolFromCapSet - cap enum = %d", capEnum);
	return RemoveProtocolFromCapSet(_ANY, capEnum);
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::RemoveProtocolFromCapSet(payload_en payloadType, CapEnum capEnum)
{
	WORD numOfRemovedCap	= 0;
	BYTE bH263Plus			= FALSE;

	// Update boolean variables
	if (payloadType == _H263_P)
		bH263Plus = TRUE;

	// if it's people and content we will have to remove the control form
	// as long with the explicit form (if there is one like in version 1) and the label cap.
	if (payloadType == _H323_P_C)
	{
		// this cap is in the people & content alt
		numOfRemovedCap += RemoveCapFromCapBuffer(ePeopleContentCapCode,kRoleContent);
		// these caps are in the content alt
		numOfRemovedCap += RemoveCapFromCapBuffer(eH263CapCode,kRoleContent);
		numOfRemovedCap += RemoveCapFromCapBuffer(eRoleLabelCapCode,kRoleContent);
		numOfRemovedCap += RemoveCapFromCapBuffer(eRoleLabelCapCode,kRolePeople);
	}
	if (payloadType == _H239)
	{
		// this cap is in the people & content alt
		numOfRemovedCap += RemoveCapFromCapBuffer(eH239ControlCapCode);
		// these caps are in the content alt
		numOfRemovedCap += RemoveCapFromCapBuffer(eH263CapCode,kRolePresentation);
	}
	if (payloadType == _DynamicPTRep)
	{
		/* This capability is for Dynamic Payload type replacement */
		numOfRemovedCap += RemoveCapFromCapBuffer(eDynamicPTRCapCode);
	}

	else if ((payloadType == _ANY) && (capEnum == eGenericVideoCapCode) )//genericVideoCapCode doesn't have a payloadType
		numOfRemovedCap += RemoveCapFromCapBuffer(eGenericVideoCapCode);

	else if(payloadType == _ANY)
		numOfRemovedCap += RemoveCapFromCapBuffer(capEnum);
	else // remove cap sets of people
	{
		// If supported algorithm, remove it from buffer.
		// (we don't want to try to remove algs that are not in cap set)
		CCapSetInfo tempCapInfo(payloadType,0); //since we change m_index of tempCapInfo in the
		//function SetNextH323CapCodeWithSamePayloadType, we need to work on a temporary object
		while ((CapEnum)tempCapInfo < eUnknownAlgorithemCapCode)
		{
			if(m_capArray[(CapEnum)tempCapInfo] > 0)
				numOfRemovedCap += RemoveCapFromCapBuffer(tempCapInfo,kRolePeople,bH263Plus);
			tempCapInfo.SetNextIpCapCodeWithSamePayloadType(); // for g7221 (24k or 32k)
		}
	}

	return numOfRemovedCap;
}

//////////////////////////////////////////////////////////////////////////////
void  CCapH323::RemoveOtherFormats(EFormat eWantedFormat, CapEnum h323CapCode, ERoleLabel eRole)
{
//	PTRACE2INT2(eLevelInfoNormal,"CCapH323::RemoveOtherFormats - format = %d",eWantedFormat);

	WORD	numOfRemovedCap  = 0;
	BYTE	bIsEncryptedCap = FALSE;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char* tempPtr = (char*)pCapBuffer;

	BYTE bFormatLeft;
	BYTE bRemoveInAllProtocols = (h323CapCode == eUnknownAlgorithemCapCode);
	CCapSetInfo capInfo;
	CapEnum capCode;

	for(int i = 0; i < m_numOfCaps; i++)
	{
		bFormatLeft = FALSE;
		capInfo = (CapEnum)pCapBuffer->capTypeCode;
		capCode = capInfo.GetIpCapCode();
		if ( (pCapBuffer->capTypeCode == h323CapCode) || (bRemoveInAllProtocols && capInfo.IsType(cmCapVideo) && (capCode != eH264CapCode) ) )
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(capCode , pCapBuffer->dataCap);
			if (pVideoCap && CheckRole(eRole,i,pVideoCap))
			{
				for (EFormat index = kQCif; index < kLastFormat; index++)
				{
					APIU8 currentFormat = pVideoCap->GetFormatMpi(index);

					if ( (index == eWantedFormat) && (currentFormat > 0) )
					{
						bFormatLeft = TRUE;
						continue;
					}

					if(currentFormat > 0)
						pVideoCap->SetFormatMpi(index, -1);
				}

				if (bFormatLeft == FALSE)
				{
					if(IsPartyEncrypted())
					{
						//in case the cap is encrypt remove the 235cap (encryptedPosition).
						//We should remove first the 235Cap before, because it comes after the cap that we want
						//to remove so we will not harm the remove cap's index

						bIsEncryptedCap = RemoveEncryptedCapFromCapBuffer(i);
						if(bIsEncryptedCap)
							numOfRemovedCap++;
					}

					numOfRemovedCap++;

					RemoveOneCapset(pCapBuffer, i, capCode);
					UpdateNumOfCaps(capCode, eRole);
					i--;//for the "for" loop, since we decresed m_numOfCaps

					// If one cap was removed, there is a need to update members and to build cap matrix from the start.
					// Build the matrix from the start
					BuildCapMatrixFromSortedCapBuf(); //Since this function is been called only for local
					//caps and not for remote caps, we don't need to build the matrix from the start.
					BuildSortedCap();
				}
			}

			POBJDELETE(pVideoCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
}
///////////////////////////////////////////////////////////////////////////
void CCapH323::SetFormatsMpi(CapEnum protocol, ERoleLabel eRole, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi)
{
	if ((qcifMpi==-1 && cifMpi==-1 && cif4Mpi==-1 && cif16Mpi==-1)
		|| ((protocol !=eH263CapCode) && (protocol !=eH261CapCode)))
	{
		PTRACE(eLevelInfoNormal,"CCapH323::SetFormatsMpi : illegal parameters");
		return;
	}
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char* tempPtr = (char*)pCapBuffer;
	CCapSetInfo capInfo;
	CapEnum capCode;
	for(int i = 0; i < m_numOfCaps; i++)
	{
		capInfo = (CapEnum)pCapBuffer->capTypeCode;
		capCode = capInfo.GetIpCapCode();
		if (capCode == protocol)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(capCode , pCapBuffer->dataCap);
			if (pVideoCap && CheckRole(eRole,i,pVideoCap))
			{
				pVideoCap->SetFormatMpi(kQCif, qcifMpi);
				pVideoCap->SetFormatMpi(kCif, cifMpi);
				pVideoCap->SetFormatMpi(k4Cif, cif4Mpi);
				pVideoCap->SetFormatMpi(k16Cif, cif16Mpi);
			}
			POBJDELETE(pVideoCap);
		}
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
}
///////////////////////////////////////////////////////////////////////////
void CCapH323::Remove4CifFromH263VideoCap()
{
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char* tempPtr = (char*)pCapBuffer;
	CCapSetInfo capInfo;
	CapEnum capCode;
	for(int i = 0; i < m_numOfCaps; i++)
	{
		capInfo = (CapEnum)pCapBuffer->capTypeCode;
		capCode = capInfo.GetIpCapCode();
		if (capCode == eH263CapCode)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(capCode , pCapBuffer->dataCap);
			if (pVideoCap && CheckRole(kRolePeople,i,pVideoCap))
				pVideoCap->SetFormatMpi(k4Cif, -1);
			POBJDELETE(pVideoCap);
		}
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

}
///////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsDropFieldCap()
{
	BYTE bFound = FALSE;
	capBuffer* pCapBuffer = (capBuffer *)&m_pCap->caps;
    char*	   pTempPtr   = (char*)pCapBuffer;

	for(BYTE i=0; (i<m_numOfCaps) && !bFound; i++)
	{
		if (pCapBuffer->capTypeCode == eGenericVideoCapCode)
		{
			CGenericVideoCap* pGenericVideoCap = (CGenericVideoCap *)CBaseCap::AllocNewCap(eGenericVideoCapCode, pCapBuffer->dataCap);
			if (pGenericVideoCap && pGenericVideoCap->IsDropField())
				bFound = TRUE;
			POBJDELETE(pGenericVideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	return bFound;
}

/////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsECS() const
{
	return (m_numOfCaps == 0);
}

/////////////////////////////////////////////////////////////////////////////////////
/*BYTE CCapH323::IsMediaContaining(CComModeH323* pScm, BYTE valuesToCompare,
					  cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole) const
{
	if (pScm->IsMediaOff(dataType, direction, eRole))
		return FALSE;

	BYTE bRes = TRUE;

	if (direction & cmCapReceive)
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapReceive,eRole);
		bRes &= IsContaining(rScmMediaMode, valuesToCompare, cmCapReceive, eRole);
	}
	if (bRes && (direction & cmCapTransmit))
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapTransmit,eRole);
		bRes &= IsContaining(rScmMediaMode, valuesToCompare, cmCapTransmit, eRole);
	}
	return bRes;
}*/


/////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::AreLocalCapsContaining(CComModeH323* pScm, DWORD valuesToCompare,
					  cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole) const
{
    if (0 == m_numOfCaps)
	    return FALSE;

	if (pScm->IsMediaOff(dataType, direction, eRole))
		return FALSE;

	BYTE bRes = TRUE;

	if (direction & cmCapReceive)
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapReceive,eRole);
        bRes &= IsContaining(rScmMediaMode, valuesToCompare, cmCapReceive, eRole);
	}
	if (bRes && (direction & cmCapTransmit))
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapTransmit,eRole);
        bRes &= IsContaining(rScmMediaMode, valuesToCompare, cmCapTransmit, eRole);
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::AreRemoteCapsContaining(CComModeH323* pScm, DWORD valuesToCompare,
					  cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole) const
{
   if (0 == m_numOfCaps)
	    return FALSE;

   cmCapDirection oppositeDirection = cmCapReceiveAndTransmit;
   if(direction == cmCapReceive)
	   oppositeDirection = cmCapTransmit;
   if(direction == cmCapTransmit)
	   oppositeDirection = cmCapReceive;

   if (pScm->IsMediaOff(dataType, oppositeDirection, eRole))
		return FALSE;

	BYTE bRes = TRUE;
    if (direction == cmCapReceiveAndTransmit)
    {
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapReceive,eRole);
		bRes &= IsContaining(rScmMediaMode, valuesToCompare, cmCapReceiveAndTransmit, eRole);
        if(bRes == FALSE)
        {
            const CMediaModeH323& tScmMediaMode = pScm->GetMediaMode(dataType,cmCapTransmit,eRole);
            bRes = IsContaining(tScmMediaMode, valuesToCompare, cmCapReceiveAndTransmit, eRole);
        }
        return bRes;
	}

    if (direction & cmCapReceive)
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapTransmit,eRole);
		bRes &= IsContaining(rScmMediaMode, valuesToCompare, cmCapReceive, eRole);
	}
	if (bRes && (direction & cmCapTransmit))
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapReceive,eRole);
		bRes &= IsContaining(rScmMediaMode, valuesToCompare, cmCapTransmit, eRole);
		if(bRes == FALSE)
			bRes = IsContaining(rScmMediaMode, valuesToCompare, cmCapReceive, eRole);
	}

	return bRes;
}


/////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsContaining(const CMediaModeH323& rScmMediaMode, DWORD valuesToCompare, cmCapDirection direction, ERoleLabel eRole) const
{
    WORD scmType = rScmMediaMode.GetType();

	capBuffer* pRemoteCapBuffer;
	pRemoteCapBuffer = (capBuffer*) &m_pCap->caps;

	DWORD details = 0x0000;
	char* tempPtr;
	CBaseCap* pThisCap;
	CLargeString msg;
	int length = rScmMediaMode.GetLength();
	BYTE* scmData = new BYTE[length];
	rScmMediaMode.CopyData(scmData);
	CBaseCap* pOtherCap = CBaseCap::AllocNewCap((CapEnum)scmType, scmData);
	if (pOtherCap == NULL)
	{
		PDELETEA(scmData);
		return FALSE;
	}

	BYTE bRes = FALSE;
	msg << "CCapH323::IsContaining. Number of caps -  " << m_numOfCaps << "\n";

	for(int k=0; k < m_numOfCaps; k++)
	{
		if (pRemoteCapBuffer->capTypeCode == scmType)
		{
			pThisCap = CBaseCap::AllocNewCap((CapEnum)scmType, pRemoteCapBuffer->dataCap);

			if (!pThisCap)
			{
				PTRACE2INT(eLevelError, "CCapH323::IsContaining failed to allocate new cap #", k);
			}
			else
			{
				BYTE bIsGoodRole = TRUE;
				if(pThisCap->IsType(cmCapVideo))
					bIsGoodRole = CheckRole(eRole,k,pThisCap);

				if (pThisCap && (direction == cmCapReceiveAndTransmit || pThisCap->IsDirection(direction)) && bIsGoodRole )
				{
					details = 0x00000000;
					bRes = pThisCap->IsContaining(*pOtherCap,valuesToCompare, &details);
					if (bRes)
					{
						POBJDELETE(pThisCap);
						PDELETEA(scmData);
						POBJDELETE(pOtherCap);
						return TRUE;
					}
					//else: continue to search in the other caps
					cmCapDataType eType = pThisCap->GetType();
					msg << "Object's cap number " << k << " doesn't contained other cap: ";
					DumpDetailsToStream(eType,details,msg);
				}
				else if(pThisCap->IsType(cmCapVideo))
				{
					msg << ", Video not check. Containing K number - " << k << "\n";
					msg << ", is_direction - " << (DWORD)(direction == cmCapReceiveAndTransmit || pThisCap->IsDirection(direction)) << "\n";
					msg << ", is_good_role - " << (DWORD)bIsGoodRole << "\n";
				}

				POBJDELETE(pThisCap);
			}
		}

		tempPtr = (char*)pRemoteCapBuffer;
		tempPtr += sizeof(capBufferBase) + pRemoteCapBuffer->capLength;
		pRemoteCapBuffer = (capBuffer*)tempPtr;
	}

	PTRACE2(eLevelInfoNormal,"",msg.GetString());
	PDELETEA(scmData);
	POBJDELETE(pOtherCap);
	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////
void CCapH323::FindBestVidTxModeForCop(CCopVideoTxModes* pCopVideoTxModes, CComModeH323* pScm, BYTE definedProtocol, DWORD definedRate) const
{
	if (!pCopVideoTxModes)
	{
		PTRACE(eLevelInfoNormal,"CCapH323::FindBestVidTxModeForCop pCopVideoTxModes is NULL");
		return;
	}

	pCopVideoTxModes->Dump("CCapH323::FindBestVidTxModeForCop",eLevelInfoNormal);
	DWORD valuesToCompare = kBitRate|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS| kH264Additional_FS|kRoleLabel| kMaxFR | kH264Mode;
	CVidModeH323* pVidMode;
	int modeIndex = 0;
	for (modeIndex=0;modeIndex<NUMBER_OF_COP_LEVELS;modeIndex++)
	{
		pVidMode = pCopVideoTxModes->GetVideoMode(modeIndex);
		if (pVidMode && pVidMode->IsMediaOn() && pCopVideoTxModes->IsValidForDefinedParams(modeIndex, definedProtocol, definedRate))
		{
			if (IsContaining(*pVidMode, valuesToCompare, cmCapReceive, kRolePeople))
			{
				COstrStream strBase;
				strBase << "Caps contain cop level index: " << modeIndex << "\n";
				pVidMode->Dump(strBase);
				PTRACE2(eLevelInfoNormal,"CCapH323::FindBestVidTxModeForCop: \n", strBase.str().c_str());
				pScm->SetMediaMode(*pVidMode, cmCapVideo, cmCapTransmit, kRolePeople);

				CBaseVideoCap* pIntersectCap = NULL;
				pIntersectCap = FindIntersectionBetweenCapsAndVideoScm(pScm, kRolePeople, TRUE);
				if (pIntersectCap)
				{
					pScm->SetMediaMode(pIntersectCap->GetCapCode(), pIntersectCap->SizeOf(), (BYTE*)pIntersectCap->GetStruct(), cmCapVideo, cmCapTransmit, kRolePeople);
					pScm->SetCopTxLevel(modeIndex);

					pIntersectCap->FreeStruct();
					POBJDELETE(pIntersectCap);
					return;
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CCapH323::FindBestVidTxModeForCopThatMatchesWithRemoteTxMode(CCopVideoTxModes* pCopVideoLocalTxModes,CCopVideoTxModes* pCopVideoRemoteTxModes, CComModeH323* pScm, BYTE definedProtocol, DWORD definedRate) const
{
	if (!pCopVideoLocalTxModes)
	{
			PTRACE(eLevelInfoNormal,"CCapH323::FindBestVidTxModeForCopThatMatchesWithRemoteTxMode pCopVideoLocalTxModes is NULL");
			return;
     }
	if(!pCopVideoRemoteTxModes)
	{
		PTRACE(eLevelInfoNormal,"CCapH323::FindBestVidTxModeForCopThatMatchesWithRemoteTxMode pCopVideoRemoteTxModes is NULL changing to regular match");
		FindBestVidTxModeForCop(pCopVideoLocalTxModes,pScm,definedProtocol,definedRate);
		return;
	}
	DWORD valuesToCompare = kBitRate|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS| kH264Additional_FS|kRoleLabel|kH264Profile;
	CVidModeH323* pVidModeLocal;
	CVidModeH323* pVidModeRemote;
	int modeIndex = 0;
	for (modeIndex=0;modeIndex<NUMBER_OF_COP_LEVELS;modeIndex++)
	{
		pVidModeLocal = pCopVideoLocalTxModes->GetVideoMode(modeIndex);


		if (pVidModeLocal && pVidModeLocal->IsMediaOn() && pCopVideoLocalTxModes->IsValidForDefinedParams(modeIndex, definedProtocol, definedRate))
		{
			if (IsContaining(*pVidModeLocal, valuesToCompare, cmCapReceive, kRolePeople))
			{

				BYTE modeRemoteIndex = 0;
				for(modeRemoteIndex=0;modeRemoteIndex<NUMBER_OF_COP_LEVELS;modeRemoteIndex++)
				{
					DWORD details = 0;
					pVidModeRemote = pCopVideoRemoteTxModes->GetVideoMode(modeRemoteIndex);
					if( pVidModeLocal->IsContaining(*pVidModeRemote,valuesToCompare,&details) && pVidModeRemote->IsContaining(*pVidModeLocal,valuesToCompare,&details) )
					{
						COstrStream strBase;
						strBase << "Caps equals to cop level index: " << modeIndex << "\n";
						pVidModeLocal->Dump(strBase);
						PTRACE2(eLevelInfoNormal,"CCapH323::FindBestVidTxModeForCopThatMatchesWithRemoteTxMode: \n", strBase.str().c_str());
						pScm->SetMediaMode(*pVidModeLocal, cmCapVideo, cmCapTransmit, kRolePeople);
						CBaseVideoCap* pIntersectCap = NULL;
						pIntersectCap = FindIntersectionBetweenCapsAndVideoScm(pScm, kRolePeople, TRUE);
						if (pIntersectCap)
						{
							pScm->SetMediaMode(pIntersectCap->GetCapCode(), pIntersectCap->SizeOf(), (BYTE*)pIntersectCap->GetStruct(), cmCapVideo, cmCapTransmit, kRolePeople);
							pScm->SetCopTxLevel(modeIndex);
							pIntersectCap->FreeStruct();
							POBJDELETE(pIntersectCap);
							return;
						}


					}

				}



			}
		}
	}
	PTRACE(eLevelError,"CCapH323::FindBestVidTxModeForCopThatMatchesWithRemoteTxMode no match between local Tx mode and remote Tx mode lecture cascade link feature will not work");

	//in this case no match was found we will try to connect but if this is lecture mode than on changing in to symmetric the party will drop secondary.
	FindBestVidTxModeForCop(pCopVideoLocalTxModes,pScm,definedProtocol,definedRate);

}
//////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::FindBestVidTxModeForCopLecturerLink(CCopVideoTxModes* pCopVideoTxModes, CComModeH323* pScm, WORD copLecturerLevelIndex,BYTE definedProtocol, DWORD definedRate) const
{
	if (!pCopVideoTxModes || copLecturerLevelIndex == INVALID_COP_LEVEL || copLecturerLevelIndex > 3)
	{
		PTRACE(eLevelInfoNormal,"CCapH323::FindBestVidTxModeForCop pCopVideoTxModes is NULL or no cop level is defined");
		return FALSE;
	}
	CVidModeH323* pVidMode;
	pVidMode = pCopVideoTxModes->GetVideoMode(copLecturerLevelIndex);
	if(pVidMode && pVidMode->IsMediaOn() && pCopVideoTxModes->IsValidForDefinedParams(copLecturerLevelIndex,definedProtocol,definedRate) )
	{
		COstrStream strBase;
		strBase << "this is lecturer link cop level: " << copLecturerLevelIndex << "\n";
		pVidMode->Dump(strBase);
		PTRACE2(eLevelInfoNormal,"CCapH323::FindBestVidTxModeForCop: \n", strBase.str().c_str());
		pScm->SetMediaMode(*pVidMode, cmCapVideo, cmCapTransmit, kRolePeople);
		CBaseVideoCap* pIntersectCap = NULL;
		pIntersectCap = FindIntersectionBetweenCapsAndVideoScm(pScm, kRolePeople, TRUE);
		if (pIntersectCap)
		{
					pScm->SetMediaMode(pIntersectCap->GetCapCode(), pIntersectCap->SizeOf(), (BYTE*)pIntersectCap->GetStruct(), cmCapVideo, cmCapTransmit, kRolePeople);
				//	pScm->SetCopTxLevel(modeIndex);

					pIntersectCap->FreeStruct();
					POBJDELETE(pIntersectCap);
					return TRUE;

		}


	}
	else
	{
		//the video mode not valid
		PTRACE(eLevelInfoNormal,"CCapH323::FindBestVidTxModeForCopLecturerLink lecturer level not valid");
		DBGPASSERT(103);
	}
	return FALSE;



}
///////////////////////////////////////////////////////////////////////////////
WORD CCapH323::AddVidTxModesForCop(CCopVideoTxModes* pCopVideoTxModes)
{

	CVidModeH323* pVidMode;
	int modeIndex = 0;
	WORD numOfVideoCaps = 0;
	PTRACE(eLevelInfoNormal,"CCapH323::AddVidTxModesForCop ");
	for (modeIndex=0;modeIndex<NUMBER_OF_COP_LEVELS;modeIndex++)
	{
		pVidMode = pCopVideoTxModes->GetVideoMode(modeIndex);
		if(pVidMode)
		{

			CComModeH323* pScm = new CComModeH323;
			pScm->SetMediaMode(*pVidMode, cmCapVideo, cmCapTransmit, kRolePeople);
			CBaseVideoCap* pScmCap = (CBaseVideoCap*)pScm->GetMediaAsCapClass(cmCapVideo, cmCapTransmit, kRolePeople);
			if(pScmCap)
			{
				if(pScmCap->GetCapCode() == eH264CapCode)
				{
					APIU16 profile = 0;
					APIU8 level=0;
					long mbps=0, fs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
					pVidMode->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
					PTRACE2INT(eLevelInfoNormal,"CCapH323::AddVidTxModesForCop sar, ",sar);
					((CH264VideoCap*)pScmCap)->SetSampleAspectRatio(sar);

				}

				//ADD pScmCap TO CAP SET

				int structSize = pScmCap->SizeOf();
				pScmCap->SetDirection(cmCapTransmit);
				AddCapToCapBuffer(pScmCap->GetCapCode(),structSize,pScmCap->GetStruct());
				m_capArray[pScmCap->GetCapCode()]++;
				m_numOfCaps++;
				numOfVideoCaps++;
				PTRACE(eLevelInfoNormal,"CCapH323::AddVidTxModesForCop add cap ");

				POBJDELETE(pScmCap);
			}
			else
				PTRACE2INT(eLevelError,"CCapH323::AddVidTxModesForCop no pScmCap created, ",modeIndex);


		}
		else
			PTRACE2INT(eLevelError,"CCapH323::AddVidTxModesForCop no video mode found, ",modeIndex);

	}


	//EndOfCapsConstruction(m_numOfAudioCap,m_numOfVideoCap,m_numOfContentCap,m_numOfDuoCap,m_numOfT120Cap,
	//		m_numOfFeccCap, m_numOfPeopContCap,m_numOfH239ControlCap, m_numOfEncrypCap,m_numOfNsCap,m_numOfLprCap);

	//SetPeopleContentAlt();
	return numOfVideoCaps;
}
////////////////////////////////////////////////////////////////////////////////////
void CCapH323::CopyNoneAudioCaps(CCapH323* pOldCap)
{
	CopyCaps(pOldCap, cmCapVideo, pOldCap->m_numOfVideoCap);
	if (pOldCap->m_capArray[eRoleLabelCapCode])
		CopyCaps(pOldCap, eRoleLabelCapCode, 1, kRolePeople);

	if (pOldCap->m_numOfContentCap)
	{
		CopyCaps(pOldCap, cmCapVideo, pOldCap->m_numOfContentCap, kRoleContentOrPresentation);
		if (pOldCap->m_capArray[eRoleLabelCapCode])
			CopyCaps(pOldCap, eRoleLabelCapCode, 1, kRoleContentOrPresentation);
	}

	if (pOldCap->m_numOfFeccCap)
		CopyCaps(pOldCap, cmCapData, pOldCap->m_numOfFeccCap);
	else
		CopyCaps(pOldCap, cmCapData, pOldCap->m_numOfT120Cap);

	if (m_numOfPeopContCap)
		SetContentProfile(2);

	if (m_numOfH239ControlCap)
		SetH239ControlCap();

	if (IsPartyEncrypted())
		SetEncryption();
}

////////////////////////////////////////////////////////////////////////////////////
// currently this function only handle adding of audio caps.
// audio rate, conf rate and video rate are all in bit/second values
void CCapH323::ReBuildCapsWithAddedSpecificCodecs(DWORD audioRate, DWORD confRate, DWORD videoRate, int numOfAddedCaps, ...)
{
	PTRACE(eLevelInfoNormal,"CCapH323::ReBuildCapsWithAddedSpecificCodecs");
	CCapH323* pOldCap = new CCapH323(*this);
	CapEnum *pTempCapEnumArray = (CapEnum*) new BYTE[numOfAddedCaps];

	// get the function varibels
	va_list			params;
	va_start(params, numOfAddedCaps);     // Initialize variable arguments.
	for (int i=0; i<numOfAddedCaps; i++)
	{
		pTempCapEnumArray[i]  = (CapEnum)va_arg(params, int);
	}
	va_end(params);               // Reset variable arguments.

	Initialize();
	m_pCap->numberOfAlts = 0;
	m_pCap->numberOfSim	 = 1; //One Sim in the Desc.

	WORD numOfAudioCaps = SetAudioAccordingToRate(audioRate, confRate, videoRate,FALSE);

	for (int j=0; j<numOfAddedCaps; j++)
	{	//If it wasn't set before, set by capOrderArr order.
		if (OnCap(pTempCapEnumArray[j])==FALSE)
			numOfAudioCaps += SetAudioCap(pTempCapEnumArray[j]);
	}

	CopyNoneAudioCaps(pOldCap);
	EndOfCapsConstruction(numOfAudioCaps, pOldCap->m_numOfVideoCap, pOldCap->m_numOfContentCap, pOldCap->m_numOfDuoCap, pOldCap->m_numOfT120Cap, pOldCap->m_numOfFeccCap, pOldCap->m_numOfPeopContCap, pOldCap->m_numOfH239ControlCap, pOldCap->m_numOfEncrypCap);
	SetPeopleContentAlt();
	POBJDELETE(pOldCap);
	PDELETEA(pTempCapEnumArray);
}

////////////////////////////////////////////////////////////////////////////////////
// audio rate, conf rate and video rate are all in bit/second values
void CCapH323::ReArrangeAudioCaps(DWORD audioRate, DWORD confRate, DWORD videoRate)
{
	CCapH323* pOldCap = new CCapH323(*this);

	Initialize();
	m_pCap->numberOfAlts = 0;
	m_pCap->numberOfSim	 = 1; //One Sim in the Desc.

	WORD numOfAudioCaps = SetAudioAccordingToRate(audioRate, confRate, videoRate,FALSE);

	CopyNoneAudioCaps(pOldCap);
	EndOfCapsConstruction(numOfAudioCaps, pOldCap->m_numOfVideoCap, pOldCap->m_numOfContentCap, pOldCap->m_numOfDuoCap, pOldCap->m_numOfT120Cap, pOldCap->m_numOfFeccCap, pOldCap->m_numOfPeopContCap, pOldCap->m_numOfH239ControlCap, pOldCap->m_numOfEncrypCap);
	SetPeopleContentAlt();
	POBJDELETE(pOldCap);
}

/////////////////////////////////////////////////////////////////////////////////////
void CCapH323::CopyCaps(CCapH323 *pCap, cmCapDataType dataType, WORD numOfCap, ERoleLabel role)
{
	CopyCaps(pCap, dataType, eUnknownAlgorithemCapCode, numOfCap, role);
}

/////////////////////////////////////////////////////////////////////////////////////
void CCapH323::CopyCaps(CCapH323 *pCap, CapEnum capEnum, WORD numOfCap, ERoleLabel role)
{
	CopyCaps(pCap, cmCapEmpty, capEnum, numOfCap, role);
}

/////////////////////////////////////////////////////////////////////////////////////
void CCapH323::CopyCaps(CCapH323 *pCap, cmCapDataType dataType, CapEnum capEnum, WORD numOfCap, ERoleLabel role)
{
	capBuffer		*pCapBuffer		= (capBuffer *) &(pCap->m_pCap->caps);
	char			*tempPtr		= (char *) pCapBuffer;
	BaseCapStruct	*pCapBaseStruct = NULL;

	BYTE bCheckDataType = dataType != cmCapEmpty;
	BYTE bCheckCapEnum  = capEnum  != eUnknownAlgorithemCapCode;

	for(int i=0; ((i < pCap->m_numOfCaps) && numOfCap); i++)
	{
		pCapBaseStruct = (BaseCapStruct*)pCapBuffer->dataCap;
		if ( (bCheckDataType && (pCapBaseStruct->header.type == dataType)) ||
			 (bCheckCapEnum && (pCapBaseStruct->header.capTypeCode == capEnum)))
		{
			if (((ERoleLabel)pCapBaseStruct->header.roleLabel == role) || ((ERoleLabel)pCapBaseStruct->header.roleLabel & role) )
			{
				CCapSetInfo capInfo = (CapEnum)pCapBaseStruct->header.capTypeCode;
				CBaseCap *pCap = CBaseCap::AllocNewCap((CapEnum)capInfo,pCapBuffer->dataCap);
				if (pCap)
				{
					int structSize = pCap->SizeOf();
					AddCapToCapBuffer(capInfo,structSize, pCap->GetStruct());
					m_capArray[(CapEnum)capInfo]++;
					m_numOfCaps++;

					numOfCap--;
					POBJDELETE(pCap);
				}
			}
		}
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
void CCapH323::BuildNewCapsFromComModeAndCaps(const CComModeH323& pNewScm, DWORD confType,WORD numOfFecc,CapEnum feccMedaiType,
											  const CCapH323* pOtherCaps, DWORD serviceId,ECopVideoFrameRate highestframerate)
{
	WORD  numOfVidCaps   = 0;
	WORD  numOfContCaps  = 0;
	WORD  numOfDuoCaps   = 0;
	WORD  numOfEncCaps   = 0;
	WORD  numOfFeccCaps  = 0;
	WORD  numOfT120Caps	 = 0;
	WORD  numOfPeopContCaps  = 0;
	WORD  numOfContOrDuoCaps = 0;

	CCapH323 *pTmpCap = new CCapH323(*this);

	Initialize();

	m_pCap->numberOfAlts = 0;
	m_pCap->numberOfSim	 = 1; //One Sim in the Desc.

	WORD numOfAudioCaps  = SetMediaCapsFromH323Scm(pNewScm, cmCapAudio);

    //right now we support creating new caps from other caps only in video
	BYTE bCanAddEPC = pTmpCap->IsEPC();
	if (pOtherCaps == NULL)
		numOfVidCaps = SetVideoCapsFromH323Scm(pNewScm, kRolePeople, bCanAddEPC, confType,FALSE,FALSE,highestframerate);
	else
		numOfVidCaps = SetVideoCapsFromAnotherCaps(pOtherCaps, kRolePeople, bCanAddEPC, confType);

	if (pTmpCap->IsEPC() || pTmpCap->IsH239())
	{ //if from the previous caps we removed this, we shouldn't declare it now.
		numOfContOrDuoCaps = SetVideoCapsFromH323Scm(pNewScm, kRoleContentOrPresentation, TRUE, confType);
		if (numOfContOrDuoCaps)
		{
			if (confType == confTypeDuoVideo)
				numOfDuoCaps  = numOfContOrDuoCaps;
			else
				numOfContCaps = numOfContOrDuoCaps;
		}
	}

	//In case we had fecc in the old cap we should remove them in case the new scm does not include them.
	//In case the ols one did not have fecc we can't include them even the new scm include them.
	if(numOfFecc)
	{
		BYTE bIsFecc = TRUE;
		if(numOfFecc == 1)
			numOfFeccCaps += SetMediaCapsFromH323Scm(pNewScm, cmCapData,bIsFecc,feccMedaiType);
		else if(numOfFecc == 2)
		{
			// get system.cfg values
			CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
			//CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			BOOL bRvFecc, bAnnexQFecc;
			bRvFecc = bAnnexQFecc = 0;
			std::string key = "FECC_RV";
//			pSysConfig->GetBOOLDataByKey(key, bRvFecc);
			if( pServiceSysCfg )
			    pServiceSysCfg->GetBOOLDataByKey(serviceId, key, bRvFecc);
			key = "FECC_ANNEXQ";
//			pSysConfig->GetBOOLDataByKey(key, bAnnexQFecc);
			if( pServiceSysCfg )
			    pServiceSysCfg->GetBOOLDataByKey(serviceId, key, bAnnexQFecc);

			if(bRvFecc)//::GetpSystemCfg()->GetFeccRVFlag())
				numOfFeccCaps += SetMediaCapsFromH323Scm(pNewScm, cmCapData,bIsFecc,eRvFeccCapCode);
			if(bAnnexQFecc)//::GetpSystemCfg()->GetFeccAnnexQFlag())
				numOfFeccCaps += SetMediaCapsFromH323Scm(pNewScm, cmCapData,bIsFecc,eAnnexQCapCode);
		}
//		else
//			PTRACE2INT2(eLevelInfoNormal,"CCapH323::BuildNewCapsFromComModeAndCaps: unexpected num of fecc  %d", numOfFecc);
	}
	else
		numOfT120Caps = SetMediaCapsFromH323Scm(pNewScm, cmCapData);

	if ((confType == confTypePPCVersion1) && numOfContCaps)
		numOfPeopContCaps += SetContentProfile(2);

	WORD numOfH239ControlCaps = 0;
	if ((confType == confTypeH239) && numOfContCaps)
		numOfH239ControlCaps += SetH239ControlCap();

	if(IsPartyEncrypted())
		numOfEncCaps += SetEncryption();

	EndOfCapsConstruction(numOfAudioCaps, numOfVidCaps, numOfContCaps, numOfDuoCaps, numOfT120Caps, numOfFeccCaps, numOfPeopContCaps, numOfH239ControlCaps, numOfEncCaps);
	SetPeopleContentAlt();
	POBJDELETE(pTmpCap);
}

////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::SetVideoCapsFromAnotherCaps(const CCapH323* pOtherCap, ERoleLabel eRole, BYTE bCanAddEPC, DWORD confType)
{
	WORD numOfCaps = 0;
	capBuffer* pOtherCapBuffer = (capBuffer *) &(pOtherCap->m_pCap->caps);
	char*	   pOtherTempPtr   = (char*)pOtherCapBuffer;
	BaseCapStruct* pOtherCapBaseStruct = NULL;
	CBaseVideoCap* pVideoCap	       = NULL;
	EResult eResOfSet;
	CapEnum mediaType;

	for(int i = 0; i < pOtherCap->m_numOfCaps; i++)
	{
		pOtherCapBaseStruct = (BaseCapStruct*)pOtherCapBuffer->dataCap;
		if(pOtherCapBaseStruct->header.type == cmCapVideo)
		{
			mediaType = (CapEnum)pOtherCapBuffer->capTypeCode;
			pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(mediaType, (BYTE*)pOtherCapBuffer->dataCap);
			if (pVideoCap && pOtherCap->CheckRole(eRole,i,pVideoCap))
			{
				int structSize = pVideoCap->SizeOf();
				eResOfSet = kSuccess;
				if (structSize)
				{
					if( (confType == confTypeDuoVideo) && (eRole == kRoleContent) )
					{
						eResOfSet &= pVideoCap->SetRole(kRolePeople);
						eResOfSet &= pVideoCap->SetBitRate(m_maxContRate);
					}

					if (eResOfSet)
					{
						AddCapToCapBuffer(mediaType, structSize, pVideoCap->GetStruct());
						m_capArray[mediaType]++;
						numOfCaps++;
						m_numOfCaps++;
					}
					//we don't need to add here the generic video caps of the droop field, because it's part of the "other" caps
				}
				else
					PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromAnotherCaps: Set struct has failed");
			}
			POBJDELETE(pVideoCap);
		}

		pOtherTempPtr += sizeof(capBufferBase) + pOtherCapBuffer->capLength;
		pOtherCapBuffer = (capBuffer*)pOtherTempPtr;
	}


	if(confType == confTypePPCVersion1)
	{
		if (numOfCaps && bCanAddEPC)
		{
			BYTE label = (eRole == kRolePeople) ? LABEL_PEOPLE : LABEL_CONTENT;
			numOfCaps += SetRoleLabelCapCode(label);
		}
	}

	return numOfCaps;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::SetVideoCapsFromH323Scm(const CComModeH323& rScm, ERoleLabel eRole, BYTE bCanAddEPC, DWORD confType, BYTE isRemoveGenericVideoCap, BYTE isRemoveOtherThenQCif,ECopVideoFrameRate highestframerate, BYTE maxResolution)
{
	PTRACE2INT(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm : eRole = ", eRole);
	if (rScm.IsMediaOff(cmCapVideo, cmCapReceive, eRole))
		return 0; //numOfCaps

	WORD numOfCaps = 0;
	BYTE bRemoveAnnexI_NS = FALSE;

	//in case we need to send re-caps, that means that we want to change the receive mode
	WORD length = rScm.GetMediaLength(cmCapVideo, cmCapReceive, eRole);
	capBuffer* pCapBuffer = (capBuffer *)new BYTE[length + sizeof(capBufferBase)];
	rScm.CopyMediaToCapBuffer(pCapBuffer ,cmCapVideo, cmCapReceive, eRole);
	CapEnum mediaType = (CapEnum)pCapBuffer->capTypeCode;
	CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(mediaType, (BYTE*)pCapBuffer->dataCap);
	PTRACE2INT(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm : mediaType = ",mediaType);
	if (pVideoCap)
	{
		int structSize = pVideoCap->SizeOf();
		EResult eResOfSet = kSuccess;
		if (structSize)
		{
			eResOfSet &= pVideoCap->SetDirection(cmCapReceive); //ensure direction

			if ((mediaType == eH263CapCode) && (eRole == kRolePeople))
			{
				if (((CH263VideoCap*)pVideoCap)->IsAnnex(typeAnnexI_NS))
				{
					if ((((CH263VideoCap*)pVideoCap)->RemoveAnAnnexFromMask(typeAnnexI_NS)) == kSuccess)
						bRemoveAnnexI_NS = TRUE;
				}
				if(isRemoveOtherThenQCif)
				{
					PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm - Remote can't other resolution then QCIF");
					for (EFormat index = kCif; index <= kQVGA; index++)
					{
						INT8 currentFormat = pVideoCap->GetFormatMpi(index);

						if(currentFormat > 0)
							pVideoCap->SetFormatMpi(index, -1);
					}
				}
				else// if we enable other caps than QCIF we also set the 4CIF
				{
					//adjust to video rate
					DWORD audioRate = CalculateAudioRate((pVideoCap->GetBitRate()*100));
					DWORD actualVideoRate = pVideoCap->GetBitRate() - (audioRate/100);
					m_h263_4CifMpi = CH263VideoMode::GetH263Cif4VideoCardMPI(actualVideoRate, m_videoQuality);
				}
			}
			else if(mediaType == eH261CapCode)
			{
				CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
				BOOL bIsStillImageTransmissionSupported = 0;
				std::string key = "H261_StillImageTransmission";
				pSysConfig->GetBOOLDataByKey(key, bIsStillImageTransmissionSupported);

				((CH261VideoCap*)pVideoCap)->SetStillImageTransmission(bIsStillImageTransmissionSupported);
			}

			if (eRole == kRoleContent)
			{//scm in both duo and H239 is with role content
				if (confType == confTypeDuoVideo)
				{
					eResOfSet &= pVideoCap->SetRole(kRolePeople);
					eResOfSet &= pVideoCap->SetBitRate(m_maxContRate);
				}
				else if (confType == confTypeH239)
					eResOfSet &= pVideoCap->SetRole(kRolePresentation);
			}
			else if (eRole & kRoleContentOrPresentation)
				eResOfSet &= pVideoCap->SetRole(kRolePresentation);

			if (eResOfSet)
			{
				AddCapToCapBuffer(mediaType, structSize, pVideoCap->GetStruct());
				m_capArray[mediaType]++;
				numOfCaps++;
				m_numOfCaps++;

				// Add baseline profile cap if needed:
				if ((mediaType == eH264CapCode) && (eRole == kRolePeople))
				{
					APIU16 capProfile = ((CH264VideoCap*)pVideoCap)->GetProfile();
					PTRACE2INT(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm capProfile=",capProfile);
					if (capProfile == H264_Profile_High && rScm.GetConfType() != kVSW_Fixed)
					{
						BYTE bBaselineCapAdded = FALSE;
						CH264VideoCap* pVideoCapBaseline = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, NULL);
						if (pVideoCapBaseline != NULL  )
						{
							DWORD callRate = rScm.GetCallRate();
							PTRACE2INT(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm rScm.GetCallRate=",callRate);
							PTRACE2INT(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm m_videoQuality=",m_videoQuality);
							DWORD valuesToCompare = kBitRate|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS| kH264Additional_FS|kRoleLabel| kMaxFR | kH264Mode;

							if(rScm.GetConfType() == kCop)
							{
								pVideoCapBaseline->SetDefaults(pVideoCap->GetDirection(),pVideoCap->GetRole());
								pVideoCapBaseline->CopyQualities(*pVideoCap);
								pVideoCapBaseline->SetBitRate(pVideoCap->GetBitRate());
								long currentFS = pVideoCapBaseline->GetFs();
								if (currentFS == -1 )
								{
									PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm -current fs =-1");
									CH264Details thisH264Details = pVideoCapBaseline->GetLevel();
									currentFS = thisH264Details.GetDefaultFsAsProduct();
								}
								else
								{
									currentFS = currentFS * CUSTOM_MAX_FS_FACTOR;
								}
								if( isNeedToChangeResOfBaselineAccordingToRate(callRate,currentFS) )
								{
									PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm -need to change baseline res");
									long levelValue ,maxMBPS,maxFS,maxDPB,maxBR,maxCPB,maxSAR,maxStaticMbps = 0;
									WORD encoderLevel = GetEncoderParamsForNewResOnH264BaseLineCap(callRate,highestframerate,levelValue,maxMBPS,maxFS,maxDPB,maxBR,maxCPB,maxSAR,maxStaticMbps);
									if(encoderLevel != (WORD)-1)
									{
										maxSAR = ((CH264VideoCap *)pVideoCap)->GetSampleAspectRatio();
										pVideoCapBaseline->SetLevelAndAdditionals(H264_Profile_BaseLine,levelValue,maxMBPS,maxFS,maxDPB,maxBR,maxSAR,maxStaticMbps);
										AddCapToCapBuffer(mediaType, pVideoCapBaseline->SizeOf(), pVideoCapBaseline->GetStruct());
										m_capArray[mediaType]++;
										numOfCaps++;
										m_numOfCaps++;
										bBaselineCapAdded = TRUE;
									}
									else
										PTRACE(eLevelError,"CCapH323::SetVideoCapsFromH323Scm -need to change baseline res -can't get incoder index -stay with res to high");
								}
								else
								{

									PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm -no need to change res acoordig to treshold");

								}
							}
							else
							{
								pVideoCapBaseline->SetDefaults(pVideoCap->GetDirection(),pVideoCap->GetRole());
								pVideoCapBaseline->CopyQualities(*pVideoCap);
								pVideoCapBaseline->SetBitRate(pVideoCap->GetBitRate());


								if( maxResolution == eAuto_Res )
								{
									Eh264VideoModeType resourceMaxVideoMode = IsFeatureSupportedBySystem(eFeatureHD1080p60Asymmetric) ? eHD1080At60Asymmetric : eHD1080At60Symmetric;
									H264VideoModeDetails h264VidModeDetails;
									GetH264VideoParams(h264VidModeDetails, callRate*1000 , m_videoQuality, resourceMaxVideoMode, FALSE);
									pVideoCapBaseline->SetLevelAndAdditionals(H264_Profile_BaseLine, h264VidModeDetails.levelValue, h264VidModeDetails.maxMBPS,h264VidModeDetails.maxFS, h264VidModeDetails.maxDPB,h264VidModeDetails.maxCPB,((CH264VideoCap *)pVideoCap)->GetSampleAspectRatio(),h264VidModeDetails.maxStaticMbps);
								}
								else
									pVideoCapBaseline->SetProfile(H264_Profile_BaseLine);

								DWORD details = 0;
								DWORD valuesToCompare = kBitRate|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS| kH264Additional_FS|kRoleLabel;
								if (pVideoCap->IsContaining(*pVideoCapBaseline, valuesToCompare, &details))
								{
									PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm : Add baseline cap according to decision matrix for baseline profile");
									AddCapToCapBuffer(mediaType, pVideoCapBaseline->SizeOf(), pVideoCapBaseline->GetStruct());
									m_capArray[mediaType]++;
									numOfCaps++;
									m_numOfCaps++;
									bBaselineCapAdded = TRUE;
								}

							}

							pVideoCapBaseline->FreeStruct();
							POBJDELETE(pVideoCapBaseline);
						}
						else
							DBGPASSERT(1);

						if (!bBaselineCapAdded)
						{
							PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm : Add the original cap as baseline cap");
							((CH264VideoCap *)pVideoCap)->SetProfile(H264_Profile_BaseLine);
							AddCapToCapBuffer(mediaType, structSize, pVideoCap->GetStruct());
							m_capArray[mediaType]++;
							numOfCaps++;
							m_numOfCaps++;
						}
					}

				}
			}

			if (pVideoCap->IsInterlaced())
			{
				if(isRemoveGenericVideoCap == FALSE)
					numOfCaps += SetDropFieldCap();
				else
					PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm - EP doesn't support drop field. Cap has removed from list");
			}

			if (bRemoveAnnexI_NS)
				numOfCaps += SetNonStandardAnnex();
		}
		else
			PTRACE(eLevelInfoNormal,"CCapH323::SetVideoCapsFromH323Scm: Set struct has failed");
	}
	else
		DBGPASSERT(1);

	PDELETEA(pCapBuffer);
	POBJDELETE(pVideoCap);

	if(confType == confTypePPCVersion1)
	{
		if (numOfCaps && bCanAddEPC)
		{
			BYTE label = (eRole == kRolePeople) ? LABEL_PEOPLE : LABEL_CONTENT;
			numOfCaps += SetRoleLabelCapCode(label);
		}
	}

	return numOfCaps;
}

////////////////////////////////////////////////////////////////////////////////////////////
//for audio or data
WORD CCapH323::SetMediaCapsFromH323Scm(const CComModeH323& rScm, cmCapDataType dataType,BYTE bIsFecc,CapEnum mediaCap)
{
	if( (dataType != cmCapAudio) && (dataType != cmCapData) )
	{
//		PTRACE2INT2(eLevelInfoNormal,"CCapH323::SetMediaCapsFromH323Scm: Illegal cmCapDataType - %d", dataType);
		return 0;
	}

	if (rScm.IsMediaOff(dataType, cmCapReceive))
		return 0; //numOfCaps

	WORD numOfCaps = 0;
	CapEnum mediaType = eUnknownAlgorithemCapCode;

	WORD length = rScm.GetMediaLength(dataType, cmCapReceive);
	capBuffer* pCapBuffer = (capBuffer *)new BYTE[length + sizeof(capBufferBase)];
	rScm.CopyMediaToCapBuffer(pCapBuffer ,dataType, cmCapReceive);

	if(bIsFecc)
		pCapBuffer->capTypeCode = mediaCap;

	mediaType = (CapEnum)pCapBuffer->capTypeCode;
	BaseCapStruct* algorithmCapStructPtr = NULL;
	int structSize = 0;

	CBaseCap* pCap = (CBaseCap *)CBaseCap::AllocNewCap(mediaType, (BYTE*)pCapBuffer->dataCap);
	if (pCap)
	{
		pCap->SetDirection(cmCapReceive); //ensure direction
		structSize = pCap->SizeOf();
		algorithmCapStructPtr = pCap->GetStruct();
		POBJDELETE(pCap);
	}

	if (structSize)
	{
		AddCapToCapBuffer(mediaType, structSize, algorithmCapStructPtr);
		m_capArray[mediaType]++;
		numOfCaps++;
		m_numOfCaps++;
	}
	else
		PTRACE(eLevelInfoNormal,"CCapH323::SetMediaCapsFromH323Scm: Set struct has failed");

	PDELETEA(pCapBuffer);

	return numOfCaps;
}

////////////////////////////////////////////////////////////////////////////////////////
CBaseVideoCap* CCapH323::FindIntersectionBetweenCapsAndVideoScm(CComModeH323* pScm, ERoleLabel eRole, BYTE bCheckRate) const
{
    if (IsECS() || 0 == GetNumOfVideoCap())
        return NULL;

	CBaseVideoCap* pScmCap = (CBaseVideoCap*)pScm->GetMediaAsCapClass(cmCapVideo, cmCapTransmit, eRole);

	if (!pScmCap)
	{
        PTRACE(eLevelError, "CCapH323::FindIntersectionBetweenCapsAndVideoScm - pScmCap is NULL");
        DBGPASSERT(1200);
	    return NULL;
	}

	CapEnum			scmProtocol = pScmCap->GetCapCode();
	CBaseVideoCap	*pIntersect	= NULL;

	BYTE bCheckProfile = FALSE;
	if ((scmProtocol == eH264CapCode) && (((CH264VideoCap*)pScmCap)->GetProfile() != H264_Profile_High))
		bCheckProfile = TRUE;

	const BaseCapStruct* pRemoteBestStruct = GetBestCapStruct(scmProtocol, cmCapReceive, kUnknownFormat, kRolePeople, bCheckProfile);
	if (pRemoteBestStruct)
	{
		CBaseVideoCap* pRemoteBestCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(scmProtocol,(BYTE *)pRemoteBestStruct);
		if (pRemoteBestCap)
		{
			if (!bCheckRate || (bCheckRate && (pScmCap->GetBitRate() <= pRemoteBestCap->GetBitRate())))
			{
				BYTE bIntersectRate = !bCheckRate;
				pIntersect = pScmCap->CreateIntersectBetweenTwoVidCaps((CBaseVideoCap*)pRemoteBestCap, cmCapTransmit, bIntersectRate, FALSE);
			}
			POBJDELETE(pRemoteBestCap);
		}
	}

	POBJDELETE(pScmCap);
	return pIntersect;
}

////////////////////////////////////////////////////////////////////////////////////////
//finds only 1 intersect between 2 caps
CBaseVideoCap* CCapH323::FindIntersectionBetweenTwoCaps(CCapH323* pOtherCaps, CapEnum protocol,
														cmCapDirection eDirection, ERoleLabel eRole, BYTE bCheckRate)
{
    if (IsECS() || pOtherCaps->IsECS() || 0 == GetNumOfVideoCap() || 0 == pOtherCaps->GetNumOfVideoCap())
        return NULL;

	//this
	capBuffer		*pCapBuffer = (capBuffer *)&m_pCap->caps;
	char			*tempPtr    = (char *)pCapBuffer;
	CBaseVideoCap	*pIntersect	= NULL;
	BYTE			bFound		= FALSE;
	BYTE bCheckProfile = FALSE;
	if (IsSupportH264HighProfile() == FALSE || pOtherCaps->IsSupportH264HighProfile() == FALSE)
		bCheckProfile = TRUE;

	for (int i=0; i<m_numOfCaps && !bFound; i++)
	{
		CBaseCap* pThisCap = (CBaseCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pThisCap)
		{
			if (pThisCap->IsType(cmCapVideo) && CheckRole(eRole, i, pThisCap) && ((pThisCap->GetCapCode() == protocol) || (protocol == eUnknownAlgorithemCapCode)))
			{
				CapEnum currentProtocol = protocol;
				if (currentProtocol == eUnknownAlgorithemCapCode) //don't care
					currentProtocol = pThisCap->GetCapCode();

				BYTE isDirection = FALSE;
				if (eDirection == cmCapReceiveAndTransmit)// direction receive and transmit for that function means its enough to fing one direction or more
					isDirection = pThisCap->IsDirection(cmCapReceive) || pThisCap->IsDirection(cmCapTransmit);// since we check the local cap for what we want to open in the outgoing channel
				else									// In RMX there is no special video conferences like QUAD, therefore all the in and the out resolutions are the same.
					isDirection = pThisCap->IsDirection(eDirection);

				if (isDirection)
				{
					const BaseCapStruct* pRemoteBestStruct = pOtherCaps->GetBestCapStruct(currentProtocol, cmCapReceive, kUnknownFormat, eRole, bCheckProfile);
					if (pRemoteBestStruct)
					{
						CBaseVideoCap* pRemoteBestCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(currentProtocol,(BYTE *)pRemoteBestStruct);
						if (pRemoteBestCap)
						{
							if (!bCheckRate || (bCheckRate && (pThisCap->GetBitRate() <= pRemoteBestCap->GetBitRate())))
							{
								BYTE bIntersectRate = !bCheckRate;
								pIntersect = ((CBaseVideoCap*)pThisCap)->CreateIntersectBetweenTwoVidCaps((CBaseVideoCap*)pRemoteBestCap, cmCapTransmit, bIntersectRate, FALSE);
								if (pIntersect && pIntersect->GetStruct())
									bFound = TRUE;
							}

							POBJDELETE(pRemoteBestCap);
						}
					}
				}

			}
			POBJDELETE(pThisCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return (pIntersect);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//In case remote doesn't have h264 we try to open something else (like h263)
BYTE CCapH323::FindSecondBestCap( CCapH323 *pRmtCapH323, CapEnum &h323OutCapCode, cmCapDataType eType)
{
    if (IsECS() || pRmtCapH323->IsECS())
        return FALSE;

	//AreCapsSupportVideoProtocolAndRate( protocol, (DWORD) 0, kRolePeople, TRUE)
	capBuffer		*pCapBuffer = (capBuffer *)&m_pCap->caps;
	char			*tempPtr    = (char *)pCapBuffer;
	BYTE 			bRightDirection = FALSE;
	CapEnum currentCap = eUnknownAlgorithemCapCode;

	for (int i=0; i < m_numOfCaps; i++)
	{
		CBaseCap* pThisCap = (CBaseCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pThisCap)
		{
			currentCap = pThisCap->GetCapCode();
			bRightDirection = pThisCap->IsDirection(cmCapReceive) && 1;//|| pThisCap->IsDirection(cmCapReceiveAndTransmit);
			POBJDELETE(pThisCap);
			if (bRightDirection && pRmtCapH323->AreCapsSupportProtocol(currentCap, eType, kRolePeople)) {
                h323OutCapCode = currentCap;
				return TRUE;
			}
		}
		else return FALSE;

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////
//finds only 1 intersect between 2 caps
CBaseVideoCap* CCapH323::FindIntersectionBetweenTwoCaps(CBaseVideoCap* pRmtCap, CapEnum protocol,
														cmCapDirection eDirection, ERoleLabel eRole, BYTE bCheckRate)
{
	//this
	capBuffer		*pCapBuffer = (capBuffer *)&m_pCap->caps;
	char			*tempPtr    = (char *)pCapBuffer;
	CBaseVideoCap	*pIntersect	= NULL;
	BYTE			bFound		= FALSE;

	for (int i=0; i<m_numOfCaps && !bFound; i++)
	{
		CBaseCap* pThisCap = (CBaseCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pThisCap)
		{
			if(pThisCap->IsType(cmCapVideo) && CheckRole(eRole,i,pThisCap) && (pThisCap->GetCapCode() == protocol))
			{
				BYTE isDirection = FALSE;
				if(eDirection == cmCapReceiveAndTransmit)// direction receive and transmit for that function means its enough to fing one direction or more
					isDirection = pThisCap->IsDirection(cmCapReceive) || pThisCap->IsDirection(cmCapTransmit);// since we check the local cap for what we want to open in the outgoing channel
				else									// In RMX there is no special video conferences like QUAD, therefore all the in and the out resolutions are the same.
					isDirection = pThisCap->IsDirection(eDirection);

				if (isDirection)
				{
					if (!bCheckRate || (bCheckRate && (pThisCap->GetBitRate() <= pRmtCap->GetBitRate())))
					{
						BYTE bIntersectRate = !bCheckRate;
						pIntersect = ((CBaseVideoCap*)pThisCap)->CreateIntersectBetweenTwoVidCaps(pRmtCap, cmCapTransmit, bIntersectRate, FALSE);
						if (pIntersect && pIntersect->GetStruct())
							bFound = TRUE;
					}
				}
			}
			POBJDELETE(pThisCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return (pIntersect);
}

////////////////////////////////////////////////////////////////////////////////////////
//intersect all intersects between 2 caps
void CCapH323::IntersectHighestCommon(const CCapH323& other, WORD& bIsHighestCommonParamsChanged, ERoleLabel eRole)
{
	BYTE bCurCapsIntersectedWithOtherCaps = 0; //as a result of intersect
	CCapH323* pResultsCaps = new CCapH323;
	pResultsCaps->m_pCap->numberOfAlts = 0;
	pResultsCaps->m_pCap->numberOfSim  = 1;

    //other
	capBuffer* pOtherCapBuffer;
	char* otherTempPtr;
	BYTE bFoundOtherHigherOrEqualToThis;
	int j;

	//this
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char* tempPtr = (char *)pCapBuffer;
	CapEnum thisAlgorithm;

	for (int i=0; i<m_numOfCaps; i++)
	{
		bFoundOtherHigherOrEqualToThis = FALSE;

		CBaseCap* pThisCap = (CBaseCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pThisCap)
		{
			/* video */
			if (pThisCap->IsType(cmCapVideo))
			{
			//1) Find if there is a cap in "other", that contains "this"
				thisAlgorithm = pThisCap->GetCapCode();

				pOtherCapBuffer = (capBuffer *) &other.m_pCap->caps;
				otherTempPtr    = (char *)pOtherCapBuffer;

				for (j=0; j<other.m_numOfCaps && !bFoundOtherHigherOrEqualToThis; j++)
				{
					CBaseCap* pOtherCap = CBaseCap::AllocNewCap((CapEnum)pOtherCapBuffer->capTypeCode, pOtherCapBuffer->dataCap);
					if (pOtherCap)
					{//other caps are full caps, so we can ask about the role, in contrast to this caps
						if (pOtherCap->IsType(cmCapVideo) && other.CheckRole(eRole,j,pOtherCap) &&
						    (thisAlgorithm == (CapEnum)pOtherCapBuffer->capTypeCode) )
						{
							DWORD tempDetails = 0;
							bFoundOtherHigherOrEqualToThis = pOtherCap->IsContaining(*pThisCap, kBitRate|kFormat|kFrameRate|kAnnexes|kH264Level|kH264Additional|kRoleLabel, &tempDetails);
						}
						POBJDELETE(pOtherCap);
					}

					otherTempPtr += sizeof(capBufferBase) + pOtherCapBuffer->capLength;
					pOtherCapBuffer = (capBuffer*)otherTempPtr;
				}

				//2) If found, copy it to the result, if it's not already contained there
					//in this case, the current caps are not changed
				if (bFoundOtherHigherOrEqualToThis)
					pResultsCaps->InsertToResultsCapsIfNotContained((CBaseVideoCap*)pThisCap);

				//3) If not, intersect "this" with all other's caps
				else
				{
					pOtherCapBuffer = (capBuffer *) &other.m_pCap->caps;
					otherTempPtr    = (char *)pOtherCapBuffer;

					for (j=0; j<other.m_numOfCaps; j++)
					{
						CBaseCap* pOtherCap = CBaseCap::AllocNewCap((CapEnum)pOtherCapBuffer->capTypeCode, pOtherCapBuffer->dataCap);
						if (pOtherCap)
						{
							if (pOtherCap->IsType(cmCapVideo) && other.CheckRole(eRole,j,pOtherCap))
							{
								if( (pThisCap->GetCapCode() == pOtherCap->GetCapCode()) &&
								    (pThisCap->GetBitRate() <= pOtherCap->GetBitRate()))//since we don't have highest common on the rate
								{
									//Intersect
									CBaseVideoCap* pIntersectionCap = ((CBaseVideoCap*)pThisCap)->CreateIntersectBetweenTwoVidCaps((CBaseVideoCap*)pOtherCap, cmCapReceive, FALSE);
									//Insert the intersect to the result caps, only if it's not contained there.
									if (pIntersectionCap && pIntersectionCap->GetStruct())
									{
										bCurCapsIntersectedWithOtherCaps |= pResultsCaps->InsertToResultsCapsIfNotContained(pIntersectionCap);
										//Changes by VK. VNGM-937 Memory leak fix
										pIntersectionCap->FreeStruct();			//
									}

									POBJDELETE(pIntersectionCap);
								}
							}
							POBJDELETE(pOtherCap);
						}

						otherTempPtr += sizeof(capBufferBase) + pOtherCapBuffer->capLength;
						pOtherCapBuffer = (capBuffer*)otherTempPtr;
					}
				}
			}

			/* Annex I NS */
			else if (pThisCap->IsType(cmCapNonStandard)/* && CheckRole(eRole,i)*/ )
			{
				BYTE bThisSupportAnnexI_NS = ((CNonStandardCap*)pThisCap)->IsNonStandardAnnex(typeAnnexI_NS);
				if (bThisSupportAnnexI_NS)
				{
					BYTE bOtherSupportAnnexI_NS = FALSE;

					pOtherCapBuffer = (capBuffer *) &other.m_pCap->caps;
					otherTempPtr    = (char *)pOtherCapBuffer;

					for (j=0; j<other.m_numOfCaps && !bOtherSupportAnnexI_NS; j++)
					{
						if ((CapEnum)pOtherCapBuffer->capTypeCode == eNonStandardCapCode)
						{
							CNonStandardCap* pOtherCap = (CNonStandardCap*)CBaseCap::AllocNewCap(eNonStandardCapCode, pOtherCapBuffer->dataCap);
							if (pOtherCap)
							{
								bOtherSupportAnnexI_NS = pOtherCap->IsNonStandardAnnex(typeAnnexI_NS);
								POBJDELETE(pOtherCap);
							}
						}

						otherTempPtr += sizeof(capBufferBase) + pOtherCapBuffer->capLength;
						pOtherCapBuffer = (capBuffer*)otherTempPtr;
					}

					//2) If found, copy it to the result, if it's not already contained there
					//in this case, the current caps aren't changed
					if (bOtherSupportAnnexI_NS)
						pResultsCaps->InsertToResultsCapsIfNotContained((CBaseVideoCap*)pThisCap);
				}
			}
			POBJDELETE(pThisCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}


	if (bCurCapsIntersectedWithOtherCaps)
		bIsHighestCommonParamsChanged = TRUE;
	else
	{//we have 2 options: 1- We just remove some caps from current caps
		//                2- We didn't change current caps at all!
		if (pResultsCaps->m_numOfCaps) //option 1
			bIsHighestCommonParamsChanged = (pResultsCaps->m_numOfCaps != m_numOfCaps);
		else // option 2
			bIsHighestCommonParamsChanged = FALSE;
	}

	if (pResultsCaps->m_numOfCaps)
	{
		pResultsCaps->m_peopleAltNumber = 0;
		pResultsCaps->EndOfCapsConstruction(0, pResultsCaps->m_numOfCaps, 0, 0, 0, 0, 0, 0, 0);

		*this = *pResultsCaps;
	}
	else// we stay with the old caps
		PTRACE(eLevelInfoNormal,"CCapH323::IntersectHighestCommon - No change");

	POBJDELETE(pResultsCaps);
	PTRACE(eLevelInfoNormal,"CCapH323::IntersectHighestCommon");
}

//////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::InsertToResultsCapsIfNotContained(CBaseCap* pCandidateCap)
{
	BYTE bFoundResultHigherOrEqualToCandidate = FALSE;

	CapEnum candidateCapCode = pCandidateCap->GetCapCode();

	capBuffer* pResultCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      resultTempPtr    = (char *)pResultCapBuffer;

	for (int i=0; i<m_numOfCaps && !bFoundResultHigherOrEqualToCandidate; i++)
	{
		if ((CapEnum)pResultCapBuffer->capTypeCode == candidateCapCode)
		{
			CBaseCap* pCurResultCap = (CBaseCap*)CBaseCap::AllocNewCap((CapEnum)pResultCapBuffer->capTypeCode, pResultCapBuffer->dataCap);
			if (pCurResultCap)
			{
				DWORD tempDetails = 0;
				bFoundResultHigherOrEqualToCandidate = pCurResultCap->IsContaining(*pCandidateCap, kBitRate|kFormat|kFrameRate|kAnnexes|kH264Level|kH264Additional|kRoleLabel, &tempDetails);
				POBJDELETE(pCurResultCap);
			}
		}
		resultTempPtr += sizeof(capBufferBase) + pResultCapBuffer->capLength;
		pResultCapBuffer = (capBuffer*)resultTempPtr;
	}

	if (!bFoundResultHigherOrEqualToCandidate)
	{
		int     structSize = pCandidateCap->SizeOf();
		CapEnum algorithm  = pCandidateCap->GetCapCode();
		AddCapToCapBuffer(algorithm, structSize, pCandidateCap->GetStruct());
		m_capArray[algorithm]++;
		m_numOfCaps++;
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*WORD CCapH323::IsHighestCommonConditionSupported(CHighestCommonCondition& hcCondition, ERoleLabel eRole)
{
	BYTE bResult = FALSE;

	WORD hcProtocol = hcCondition.GetVideoProtocol(H323_INTERFACE_TYPE);

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; i<m_numOfCaps; i++)
	{
		if (pCapBuffer->capTypeCode == hcProtocol)
		{
			CBaseVideoCap* pCap = (CBaseVideoCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
			if (pCap)
				bResult = pCap->IsHighestCommonConditionSupported(hcCondition);
			POBJDELETE(pCap);
			if (bResult)
				return TRUE; //found
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return FALSE;
}*/

//////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::GetMaxH264Level(ERoleLabel eRole)
{
	BYTE curlevel;
	BYTE maxLevel = 0;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; i<m_numOfCaps; i++)
	{
		CH264VideoCap* pCap = (CH264VideoCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pCap && pCap->GetCapCode() == eH264CapCode && CheckRole(eRole,i,pCap))
			{
				curlevel = pCap->GetLevel();
				if (curlevel > maxLevel)
					maxLevel = curlevel;
		}
		POBJDELETE(pCap);

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return maxLevel;
}

///////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::GetMaxH264CustomParameters(BYTE level, WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB, WORD& maxSAR, WORD& maxStaticMB, ERoleLabel eRole,DWORD profile)
{
	APIS32 mbps = 0, fs = 0, dpb = 0, brAndCpb = 0, sar = 0, staticMB = 0;
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; i<m_numOfCaps; i++)
	{
		CH264VideoCap* pCap = (CH264VideoCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pCap && pCap->GetCapCode() == eH264CapCode && CheckRole(eRole,i,pCap))
		{
			if (pCap->GetLevel() >= level)
			{
				pCap->GetAdditionalsAsExplicit(mbps, fs, dpb, brAndCpb, sar, staticMB);
				maxMBPS		= max((signed long)maxMBPS, mbps);
				maxFS		= max((signed long)maxFS,   fs);
				maxDPB		= max((signed long)maxDPB,  dpb);
				maxBRandCPB = max((signed long)maxBRandCPB, brAndCpb);
				maxSAR		= max((signed long)maxSAR, sar);
				maxStaticMB	= max((signed long)maxStaticMB, staticMB);
			}
		}
		POBJDELETE(pCap);

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
void  CCapH323::GetMaxH264StaticMB(WORD& maxStaticMB, ERoleLabel eRole)
{
	APIS32 staticMB, lMaxStaticMB;
	maxStaticMB = lMaxStaticMB = -1;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; i<m_numOfCaps; i++)
	{
		CH264VideoCap* pCap = (CH264VideoCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pCap && pCap->GetCapCode() == eH264CapCode && CheckRole(eRole,i,pCap))
			{
				staticMB = (APIS32)pCap->GetStaticMB();
				lMaxStaticMB	= max((signed long)maxStaticMB, staticMB);
		}
		POBJDELETE(pCap);

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	maxStaticMB = lMaxStaticMB;
}

///////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::GetMaxMpi(WORD protocol, WORD resolution, char& maxMpi, ERoleLabel eRole)
{
    BYTE bFoundResolution = FALSE;
	//BYTE curMpi;
	APIS8 curMpi;
	maxMpi = -1;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; i<m_numOfCaps; i++)
	{
		if (pCapBuffer->capTypeCode == protocol)
		{
			CBaseVideoCap* pCap = (CBaseVideoCap*)CBaseCap::AllocNewCap((CapEnum)protocol, pCapBuffer->dataCap);
			if (pCap && pCap->IsFormat((EFormat)resolution))
			{
				bFoundResolution = TRUE;
				curMpi = pCap->GetFormatMpi((EFormat)resolution);
				if ( (curMpi != -1) && ((curMpi < maxMpi) || (maxMpi == -1)) )
					maxMpi = curMpi;
			}
			POBJDELETE(pCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return bFoundResolution;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::GetMaxH263Annexes(WORD resolution, BYTE& bIsAnnexF, BYTE& bIsAnnexT, BYTE& bIsAnnexN, BYTE& bIsAnnexI_NS, ERoleLabel eRole)
{
    BYTE bFoundResolution = FALSE;
	bIsAnnexF = bIsAnnexT = bIsAnnexN = bIsAnnexI_NS = 0;
	BYTE bCurAnnexF, bCurAnnexT, bCurAnnexN;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; i<m_numOfCaps; i++)
	{
		if (pCapBuffer->capTypeCode == eH263CapCode)
		{
			CH263VideoCap* pCap = (CH263VideoCap*)CBaseCap::AllocNewCap(eH263CapCode, pCapBuffer->dataCap);
			if (pCap && pCap->GetAnnexesForFormat((EFormat)resolution, bCurAnnexF, bCurAnnexT, bCurAnnexN))
			{
				bFoundResolution = TRUE;
				if( (bCurAnnexF >= bIsAnnexF) && (bCurAnnexT >= bIsAnnexT) && (bCurAnnexN >= bIsAnnexN) )
				{//we can't replace only one of them, because they are per structure
					bIsAnnexF = bCurAnnexF;
					bIsAnnexT = bCurAnnexT;
					bIsAnnexN = bCurAnnexN;
				}
			}
			POBJDELETE(pCap);
		}
		else if((pCapBuffer->capTypeCode == eNonStandardCapCode) && !bIsAnnexI_NS)
		{
			CNonStandardCap* pCap = (CNonStandardCap*)CBaseCap::AllocNewCap(eNonStandardCapCode, pCapBuffer->dataCap);
			if (pCap)
				bIsAnnexI_NS = pCap->IsNonStandardAnnex(typeAnnexI_NS);
			POBJDELETE(pCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return bFoundResolution;
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD  CCapH323::IsNsAnnexI()
{
	WORD bIsAnnexI_NS = FALSE;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; (i<m_numOfCaps) && !bIsAnnexI_NS; i++)
	{
		if (pCapBuffer->capTypeCode == eNonStandardCapCode)
		{
			CNonStandardCap* pCap = (CNonStandardCap*)CBaseCap::AllocNewCap(eNonStandardCapCode, pCapBuffer->dataCap);
			if (pCap)
				bIsAnnexI_NS = pCap->IsNonStandardAnnex(typeAnnexI_NS);
			POBJDELETE(pCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return bIsAnnexI_NS;
}


////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CCapH323::IsAnnex(annexesListEn annex, ERoleLabel eRole)
{
	BYTE bIsAnnex = FALSE;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; (i<m_numOfCaps) && !bIsAnnex; i++)
	{
		if (pCapBuffer->capTypeCode == eH263CapCode)
		{
			CH263VideoCap* pCap = (CH263VideoCap*)CBaseCap::AllocNewCap(eH263CapCode, pCapBuffer->dataCap);
			if (pCap)
			{
				if (CheckRole(eRole, i, pCap))
					bIsAnnex = pCap->IsAnnex(annex);
			}
			POBJDELETE(pCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return bIsAnnex;
}

///////////////////////////////////////////////////////////////////////////////////////////
WORD CCapH323::IsVidImageFormat(WORD protocol, EFormat format, ERoleLabel eRole) const
{
    BYTE bFoundFormat = FALSE;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; (i<m_numOfCaps) && !bFoundFormat; i++)
	{
		if (pCapBuffer->capTypeCode == protocol)
		{
			CBaseVideoCap* pCap = (CBaseVideoCap*)CBaseCap::AllocNewCap((CapEnum)protocol, pCapBuffer->dataCap);
			if (pCap && pCap->IsFormat(format))
				bFoundFormat = TRUE;
			POBJDELETE(pCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return bFoundFormat;
}

///////////////////////////////////////////////////////////////////////////////////////////
APIU8 CCapH323::GetMpi(WORD protocol, EFormat format, ERoleLabel eRole)
{
    APIS8 mpi = -1;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;
	int i=0;
	for (i=0; (i<m_numOfCaps) && (mpi == -1); i++)
	{
		CBaseVideoCap* pCap = (CBaseVideoCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pCap && pCap->GetCapCode() == protocol && CheckRole(eRole,i,pCap))
		{
			if (pCap->IsFormat(format))
				mpi = pCap->GetFormatMpi(format);
		}
		POBJDELETE(pCap);

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return mpi;
}

////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::AreCapsSupportProtocol(CapEnum protocol,cmCapDataType eType,ERoleLabel role, APIS32 H264mode, APIU16 H264profile) const
{
	BYTE bFoundGood = FALSE;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; (i<m_numOfCaps) && !bFoundGood; i++)
	{
		CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pCap)
		{
			if(eType != cmCapVideo)
			{

				if( pCap->IsType(eType) && ((CapEnum)pCapBuffer->capTypeCode == protocol) )
					bFoundGood = TRUE;
				else if (isSirenLPRCap(protocol) && isSirenLPRCap(((CapEnum)pCapBuffer->capTypeCode)))
				{
					/* in case of SirenLPR we need to verify the local cap is in the range of the remote cap.
					 * So local sirenLPR_32k is included in remote sirenLPR_128k remote cap
					 **/
						if( pCap->IsType(eType) && (protocol <= (CapEnum)pCapBuffer->capTypeCode ) )
						{
							bFoundGood = TRUE;
						}
				}
			}
			else if (pCap->IsType(eType) && CheckRole(role,i,pCap ))
			{
				if (( (CapEnum)pCapBuffer->capTypeCode == protocol) &&
					( ((CapEnum)pCapBuffer->capTypeCode != eH264CapCode) || (/*(role & kRoleContentOrPresentation) &&*/ (((CH264VideoCap *)pCap)->GetH264mode() == H264mode)) ))
				{
					if (H264profile == 0)
						bFoundGood = TRUE;
					else
						if (((CH264VideoCap *)pCap)->GetProfile() == H264profile)
							bFoundGood = TRUE;
				}
			}
			POBJDELETE(pCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return bFoundGood;
}

////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::AreCapsSupportVideoProtocolAndRate(CapEnum protocol, DWORD rate, ERoleLabel role)
{
	BYTE bFoundGood = FALSE;

	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;

	for (int i=0; (i<m_numOfCaps) && !bFoundGood; i++)
	{
		CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pCap)
		{
			if (pCap->IsType(cmCapVideo) && CheckRole(role,i,pCap))
			{
				if( (protocol == eUnknownAlgorithemCapCode) || ((CapEnum)pCapBuffer->capTypeCode == protocol) )
				{
					if (pCap->GetBitRate() >= (APIS32)rate)
						bFoundGood = TRUE;
				}
			}
			POBJDELETE(pCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return bFoundGood;
}

///////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::AreLocalCapsEqual(CComModeH323* pScm, DWORD valuesToCompare,
							cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole) const
{
	if (pScm->IsMediaOff(dataType, direction, eRole))
		return FALSE;

	BYTE bRes = TRUE;

	if (direction & cmCapReceive)
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapReceive,eRole);
		bRes &= IsEqual(rScmMediaMode, valuesToCompare, cmCapReceive, eRole);
	}
	if (bRes && (direction & cmCapTransmit))
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapTransmit,eRole);
		bRes &= IsEqual(rScmMediaMode, valuesToCompare, cmCapTransmit, eRole);
		if(bRes == FALSE)
			bRes = IsEqual(rScmMediaMode, valuesToCompare, cmCapReceive, eRole);
	}
	return bRes;
}


////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::AreRemoteCapsEqual(CComModeH323* pScm, DWORD valuesToCompare,
					cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole) const
{
	cmCapDirection eOppositeDirection =  cmCapReceiveAndTransmit;
	if (direction == cmCapReceive)
		eOppositeDirection = cmCapTransmit;
	else if (direction == cmCapTransmit)
		eOppositeDirection =  cmCapReceive;

	if (pScm->IsMediaOff(dataType, eOppositeDirection, eRole))
		return FALSE;

	BYTE bRes = TRUE;

	if (direction & cmCapReceive)
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapTransmit,eRole);
		bRes &= IsEqual(rScmMediaMode, valuesToCompare, cmCapReceive, eRole);
	}
	if (bRes && (direction & cmCapTransmit))
	{
		const CMediaModeH323& rScmMediaMode = pScm->GetMediaMode(dataType,cmCapReceive,eRole);
		bRes &= IsEqual(rScmMediaMode, valuesToCompare, cmCapTransmit, eRole);
		if(bRes == FALSE)
			bRes = IsEqual(rScmMediaMode, valuesToCompare, cmCapReceive, eRole);
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsEqual(const CMediaModeH323& rScmMediaMode, DWORD valuesToCompare, cmCapDirection direction, ERoleLabel eRole) const
{
    WORD scmType = rScmMediaMode.GetType();

	capBuffer* pRemoteCapBuffer;
	pRemoteCapBuffer = (capBuffer*) &m_pCap->caps;

	char* tempPtr;
	DWORD details = 0x00000000;
	CBaseCap* pThisCap;

	int length = rScmMediaMode.GetLength();
	BYTE* scmData = new BYTE[length];
	rScmMediaMode.CopyData(scmData);
	CBaseCap* pOtherCap = CBaseCap::AllocNewCap((CapEnum)scmType, scmData);
	if (pOtherCap == NULL)
	{
		PDELETEA(scmData);
		return FALSE;
	}

	BYTE bRes = FALSE;

	for(int k=0; k < m_numOfCaps; k++)
	{
		if (pRemoteCapBuffer->capTypeCode == scmType)
		{
			pThisCap = CBaseCap::AllocNewCap((CapEnum)scmType, pRemoteCapBuffer->dataCap);

			if (!pThisCap)
			{
				DBGPASSERT(k);
				PTRACE2INT(eLevelError, "CCapH323::IsEqual failed to allocate new cap #%d", k);
			}
			else
			{
				BYTE bIsGoodRole = TRUE;
				if(pThisCap->IsType(cmCapVideo))
				{
					bIsGoodRole = CheckRole(eRole,k,pThisCap);
				}


				if (pThisCap && (direction == cmCapReceiveAndTransmit || pThisCap->IsDirection(direction)) && bIsGoodRole)
				{
                    APIS8 Mpi4CifOriginal = 0;
                    //H263 4cif for local caps
                    if(scmType == eH263CapCode &&  m_h263_4CifMpi != -1 && eRole == kRolePeople)
                    {
                        PTRACE2INT (eLevelInfoNormal, "CCapH323::IsEqual - H263 people compare in local - set 4cif to:",m_h263_4CifMpi);
                        Mpi4CifOriginal = pThisCap->GetFormatMpi (k4Cif);
                        ((CH263VideoCap*)pThisCap)->SetFormatMpi (k4Cif,m_h263_4CifMpi);
                    }
					details = 0x00000000;
					bRes = pThisCap->IsEquals(*pOtherCap,valuesToCompare);

                    if (bRes)
					{
                        if (Mpi4CifOriginal)
                            //Lior: The following line might look weird because we delete pThisCap
                            //but remember the struct is not deleted!
                            ((CH263VideoCap*)pThisCap)->SetFormatMpi (k4Cif,Mpi4CifOriginal);

                        POBJDELETE(pThisCap);
						PDELETEA(scmData);
						POBJDELETE(pOtherCap);
						return TRUE;
					}
					//else: continue to search in the other caps

					CSmallString msg;
					cmCapDataType eType = pThisCap->GetType();
					DumpDetailsToStream(eType,details,msg);
					PTRACE2(eLevelInfoNormal,"CCapH323::IsEqual: ",msg.GetString());
                    if (Mpi4CifOriginal)
                        //Lior: The following line might look weird because we delete pThisCap
                        //but remember the struct is not deleted!
                        ((CH263VideoCap*)pThisCap)->SetFormatMpi (k4Cif,Mpi4CifOriginal);
				}
				POBJDELETE(pThisCap);
			}
		}

		tempPtr = (char*)pRemoteCapBuffer;
		tempPtr += sizeof(capBufferBase) + pRemoteCapBuffer->capLength;
		pRemoteCapBuffer = (capBuffer*)tempPtr;
	}


	PDELETEA(scmData);
	POBJDELETE(pOtherCap);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
void CCapH323::GetVideoMode(CMediaModeH323& pXmitMode323, CapEnum mediaType, cmCapDirection direction)
{
	PTRACE(eLevelInfoNormal,"CCapH323::GetVideoMode ");

	capBuffer		*pLocalCapBuffer	= (capBuffer *) &m_pCap->caps;
	APIU8			xmitModeLabel		= kRolePeople;
	APIU8			xmitModeType		= mediaType;
	char			*pTempPtr;
	CVidModeH323	tmpVidMode;

	for(int k=0; k < m_numOfCaps; k++)
	{
		APIU8 localCapLabel		= ((BaseCapStruct*)pLocalCapBuffer->dataCap)->header.roleLabel;
		APIU8 localCapDirection = ((BaseCapStruct*)pLocalCapBuffer->dataCap)->header.direction;


		if((xmitModeType  == pLocalCapBuffer->capTypeCode) && (xmitModeLabel == localCapLabel))
		{
			//In case we found the exact direction we can get out from the function, else we save the mode and if we
			//do not find any cap that fit to the direction we should take the other one.
			//There are cases that there are no transmit caps only receive and the meaning is for the transmit.
			if(localCapDirection & direction)
			{
				pXmitMode323.Create(pLocalCapBuffer);
				return;
			}
			else
				tmpVidMode.Create(pLocalCapBuffer);
		}

		pTempPtr = (char*)pLocalCapBuffer;
		pTempPtr += sizeof(capBufferBase) + pLocalCapBuffer->capLength;
		pLocalCapBuffer = (capBuffer*)pTempPtr;
	}

	pXmitMode323 = tmpVidMode;

}


/////////////////////////////////////////////////////////////////////////////////////////
APIU32 CCapH323::GetPreferedAudioRate()
{
	capBuffer*	pCapBuffer	= (capBuffer *) &m_pCap->caps;
	char*		tempPtr		= (char *)pCapBuffer;
	APIU32 		rate 		= 0;
	int i = 0;
	for (i=0; i<m_numOfCaps; i++)
	{
		CCapSetInfo capInfo	= (CapEnum)pCapBuffer->capTypeCode;
		if(capInfo.IsType(cmCapAudio))
		{
			rate = capInfo.GetBitRate();
				break;
		}
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return rate;
}

/////////////////////////////////////////////////////////////////////////////////////////
DWORD CCapH323::UpdateCorrectVideoRateAfterRemovingGenericCap(DWORD confRate)
{
	PTRACE(eLevelInfoNormal,"CCapH323::UpdateCorrectVideoRateAfterRemovingGenericCap ");

	APIU32 AudRate = GetPreferedAudioRate();
	int newBitRate = (confRate - AudRate)/100;
	SetVideoBitRate(newBitRate, kRolePeople, eUnknownAlgorithemCapCode);
	return newBitRate;
}

/////////////////////////////////////////////////////////////////////////////////////////
void CCapH323::RemoveSpecificDataTypeFromCaps(cmCapDataType dataType)
{
	PTRACE2INT(eLevelInfoNormal,"CCapH323::RemoveSpecificDataTypeFromCaps - Data type = ",(DWORD)dataType);
	capBuffer*	pCapBuffer	= (capBuffer *) &m_pCap->caps;
	char*		tempPtr		= (char *)pCapBuffer;
	int i = 0;
	for (i=0; i<m_numOfCaps; i++)
	{
		CCapSetInfo capInfo	= (CapEnum)pCapBuffer->capTypeCode;
		if(capInfo.IsType(dataType))
			RemoveProtocolFromCapSet(capInfo.GetIpCapCode());

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

}
/////////////////////////////////////////////////////////////////////////////////////////
//26.12.2006 Changes by VK. Stress Test
void CCapH323::SetStressTestSpecificCaps(CapEnum eAudioCap, CapEnum eVideoCap, DWORD dwConfRate)
{
	WORD  numOfVidCap         = 0;
	WORD  numOfAudioCap       = 0;
	WORD  numOfT120Cap        = 0;
	WORD  numOfPeopContCap    = 0;
	WORD  numOfContentCap  	  = 0;
	WORD  numOfDuoCap  	  = 0;
	WORD  numOfNsCap          = 0;
	WORD  numOfFeccCap 	  = 0;
	WORD  numOfH239ControlCap = 0;
	WORD  numOfEncCap         = 0;

	PTRACE(eLevelInfoNormal, "Stress Test CCapH323::SetStressTestSpecificCaps - Build capability set");

	DWORD dwAudioRate = 0;
	DWORD dwVideoRate = 0;
	WORD Mpi = 1;
	if (eAudioCap != eUnknownAlgorithemCapCode)
	{
		numOfAudioCap += SetAudioCap(eAudioCap);
		CCapSetInfo capSetInfoObj(eAudioCap);
		dwAudioRate = capSetInfoObj.GetBitRate();
		PTRACE2INT(eLevelInfoNormal, "Stress Test CCapH323::SetStressTestSpecificCaps - Audio rate = ", dwAudioRate);
	}
	if (eVideoCap != eUnknownAlgorithemCapCode)
	{
		dwVideoRate = (dwConfRate - dwAudioRate) / 100;
		PTRACE2INT(eLevelInfoNormal, "Stress Test CCapH323::SetStressTestSpecificCaps - Video rate = ", dwVideoRate);
		numOfVidCap += SetVideoCapStressTest(eVideoCap, dwVideoRate, Mpi);
	}

	EndOfCapsConstruction(numOfAudioCap,numOfVidCap,numOfContentCap,numOfDuoCap,numOfT120Cap,
		numOfFeccCap,numOfPeopContCap,numOfH239ControlCap,numOfEncCap,numOfNsCap);
	SetPeopleContentAlt();
}
//26.12.2006 Changes by VK. Stress Test
WORD CCapH323::SetVideoCapStressTest(CapEnum videoprotocol, DWORD videoRate, WORD Mpi)
{
	CCapSetInfo capInfo = videoprotocol;
	CBaseVideoCap* pVideoCap = NULL;
	EResult eResOfSet = kSuccess;

	switch (videoprotocol)
	{
		case eH261CapCode:
			pVideoCap = (CH261VideoCap *)CBaseCap::AllocNewCap(videoprotocol,NULL);
			break;
		case eH263CapCode:
			pVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(videoprotocol,NULL);
			break;
		case eH264CapCode:
			pVideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(videoprotocol,NULL);
			if (!pVideoCap)
			{
				PASSERTMSG_AND_RETURN_VALUE(1, "CCapH323::SetVideoCapStressTest - AllocNewCap is Failed", 0);
			}
			pVideoCap->SetDefaults(cmCapReceive);
			eResOfSet &= pVideoCap->SetBitRate(videoRate);
			((CH264VideoCap*)pVideoCap)->SetMbps(20);
			((CH264VideoCap*)pVideoCap)->SetLevel(H264_Level_1_2);
			break;
		default:
		  PASSERTMSG_AND_RETURN_VALUE(1, "CCapH323::SetVideoCapStressTest - Failed, invalid video protocol", 0);
	}
	if (!pVideoCap)
	{
		PASSERTMSG_AND_RETURN_VALUE(1, "CCapH323::SetVideoCapStressTest - AllocNewCap is Failed", 0);
	}
	if (videoprotocol != eH264CapCode)
	{
		pVideoCap->SetDefaults(cmCapReceive);
		eResOfSet &= pVideoCap->SetBitRate(videoRate);
		eResOfSet &= pVideoCap->SetFormatMpi(kQCif, Mpi);
		eResOfSet &= pVideoCap->SetFormatMpi(kCif, Mpi);
		if(videoprotocol == eH263CapCode)
			((CH263VideoCap*)pVideoCap)->SetH263Plus(1,0,0,0,-1,-1,-1,-1,-1);
	}
	int structSize = pVideoCap->SizeOf();
	if (eResOfSet && structSize)
	{
		AddCapToCapBuffer(capInfo,structSize, pVideoCap->GetStruct());
		m_capArray[(CapEnum)capInfo]++;
		m_numOfCaps++;
	}
	if (pVideoCap)
	{
		pVideoCap->FreeStruct();
		POBJDELETE(pVideoCap);
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////
//(video rate is in 100 bits per sec)
void CCapH323::Set4CifMpi(DWORD videoRateIn100bits, eVideoQuality vidQuality)
{
	m_h263_4CifMpi = CH263VideoMode::GetH263Cif4VideoCardMPI(videoRateIn100bits, vidQuality);
}

////////////////////////////////////////////////////////////////////////////////////////
//finds only 1 intersect between 2 caps
CLprCap* CCapH323::GetLprCapability(CapEnum protocol)
{
	//this
	capBuffer		*pCapBuffer = (capBuffer *)&m_pCap->caps;
	char			*tempPtr    = (char *)pCapBuffer;
	CLprCap			*pLprCap	= NULL;
	BYTE			bFound		= FALSE;

	for (int i=0; i<m_numOfCaps && !bFound; i++)
	{
		CBaseCap* pThisCap = (CBaseCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
		if (pThisCap)
		{
			if ((pThisCap->GetCapCode() == protocol))
			{
				pLprCap = (CLprCap*) CBaseCap::AllocNewCap(protocol, NULL);
				if (pLprCap)
				{
					pLprCap->CopyQualities(*pThisCap);
					if(pLprCap->GetStruct())
						bFound = TRUE;
					else
						POBJDELETE(pLprCap);
				}
			}
			POBJDELETE(pThisCap);
		}

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	return (pLprCap);
}
////////////////////////////////////////////////////////////////////////////////////////
void CCapH323::SetSingleVideoProtocolIfNeeded (BYTE protocol)
{
    switch (protocol)
    {
        case(VIDEO_PROTOCOL_H261):
        {
            PTRACE (eLevelInfoNormal, "CCapH323::SetSingleVideoProtocolIfNeeded - H261");
            RemoveProtocolFromCapSet(_H264);
            RemoveProtocolFromCapSet(_H263);
            Set4CifMpi(-1);
            break;
        }
        case(VIDEO_PROTOCOL_H263):
        {
            PTRACE (eLevelInfoNormal, "CCapH323::SetSingleVideoProtocolIfNeeded - H263");
            RemoveProtocolFromCapSet(_H264);
            RemoveProtocolFromCapSet(_H261);
            break;
        }
        case(VIDEO_PROTOCOL_H264):
        {
            PTRACE (eLevelInfoNormal, "CCapH323::SetSingleVideoProtocolIfNeeded - H264");
            RemoveProtocolFromCapSet(_H263);
            RemoveProtocolFromCapSet(_H261);
            Set4CifMpi(-1);
            RemoveCapFromCapBuffer(eH264CapCode,kRolePeople,NO,H264_Profile_High);
            break;
        }
        case(VIDEO_PROTOCOL_H264_HIGH_PROFILE):
		{
        	PTRACE (eLevelInfoNormal, "CCapH323::SetSingleVideoProtocolIfNeeded - H264 -High profile");
        	RemoveProtocolFromCapSet(_H263);
        	RemoveProtocolFromCapSet(_H261);
            Set4CifMpi(-1);
            RemoveCapFromCapBuffer(eH264CapCode,kRolePeople,NO,H264_Profile_BaseLine);
            break;

        }
        case AUTO:
            //do nothing!
            break;
    }
}
////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsSupportH264HighProfile() const
{
	BYTE 		bRes 		= FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;
    int i=0;
	for(i=0; i<m_numOfCaps && !bRes; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
			if (pH264VideoCap && pH264VideoCap->GetProfile() == H264_Profile_High)
				bRes = TRUE;
			POBJDELETE(pH264VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////
BOOL CCapH323::IsH264HighProfileContent() const
{
	BOOL 		bRes 		= FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr   	= (char*)pCapBuffer;
    int i=0;
	for(i=0; i<m_numOfCaps && !bRes; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
			if (pH264VideoCap && (pH264VideoCap->GetRole() & kRoleContentOrPresentation) && pH264VideoCap->GetProfile() == H264_Profile_High)
				bRes = TRUE;
			POBJDELETE(pH264VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////
//HP content:
BOOL CCapH323::IsH264BaseProfileContent() const
{
	BOOL 		bRes 		= FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
    	char*	   	pTempPtr   	= (char*)pCapBuffer;
   	 int i=0;
	for(i=0; i<m_numOfCaps && !bRes; i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
			if (pH264VideoCap && (pH264VideoCap->GetRole() & kRoleContentOrPresentation) && pH264VideoCap->GetProfile() == H264_Profile_BaseLine)
				bRes = TRUE;
			POBJDELETE(pH264VideoCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsSupportAnnexQ() const
{
	BYTE 		bAnnexQSupported 		= FALSE;
	capBuffer* 	pCapBuffer 	= (capBuffer *)&m_pCap->caps;
	char*	   	pTempPtr   	= (char*)pCapBuffer;
	int i=0;
	for(i=0; i<m_numOfCaps && !bAnnexQSupported; i++)
	{
		if((CapEnum)pCapBuffer->capTypeCode == eAnnexQCapCode)
			bAnnexQSupported = TRUE;
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
	return bAnnexQSupported;
}
////////////////////////////////////////////////////////////////////////////////////////
void CCapH323::SetVideoCapsExactlyAccordingToScm(const CComModeH323* pScm)
{
	CapEnum capCode = (CapEnum)(pScm->GetMediaType(cmCapVideo, cmCapReceive));
	APIU16 profile = H264_Profile_BaseLine;
	if(capCode == eH264CapCode)
	{
		profile = pScm->GetH264Profile(cmCapReceive);

	}
	BYTE protocol = ::ConvertCapEnumToReservationProtocol(capCode,profile);
    DWORD videoRate = pScm->GetVideoBitRate(cmCapReceive, kRolePeople);

    SetSingleVideoProtocolIfNeeded(protocol);
	SetVideoBitRate (videoRate, kRolePeople);

	if (capCode == eH263CapCode || capCode == eH261CapCode)
	{
		int qcifMpi = -1, cifMpi = -1, cif4Mpi = -1, cif16Mpi = -1;
		qcifMpi = pScm->GetFormatMpi(kQCif, cmCapReceive, kRolePeople);
		cifMpi = pScm->GetFormatMpi(kCif, cmCapReceive, kRolePeople);
		cif4Mpi = pScm->GetFormatMpi(k4Cif, cmCapReceive, kRolePeople);
		cif16Mpi = pScm->GetFormatMpi(k16Cif, cmCapReceive, kRolePeople);
		SetFormatsMpi(capCode, kRolePeople, qcifMpi, cifMpi, cif4Mpi, cif16Mpi);
	}
	else if (capCode == eH264CapCode)
	{
		APIU16 profile = 0;
		APIU8 level=0;
		long mbps=0, fs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
		pScm->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
		profile = H264_Profile_None; // save profile as is
		SetLevelAndAdditionals(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, kRolePeople);
	}

}
//////////////////////////////////////////////////////////
void CCapH323::RemovePeopleCapSet(CapEnum capEnum)
{
	PTRACE(eLevelInfoNormal,"CCapH323::RemovePeopleCapSet");
	WORD numOfRemovedCap	= 0;
	BYTE bH263Plus			= FALSE;
	CCapSetInfo tempCapInfo(_ANY,0); //since we change m_index of tempCapInfo in the
	//function SetNextH323CapCodeWithSamePayloadType, we need to work on a temporary object
	//while ((CapEnum)tempCapInfo < eUnknownAlgorithemCapCode)
	//{
	//	if(m_capArray[(CapEnum)tempCapInfo] > 0)
	//		numOfRemovedCap += RemoveCapFromCapBuffer(tempCapInfo,kRolePeople,bH263Plus);
	//	tempCapInfo.SetNextIpCapCodeWithSamePayloadType();
	//}
	RemoveCapFromCapBuffer(capEnum,kRolePeople);
	//PTRACE(eLevelInfoNormal,"CCapH323::RemovePeopleCapSet - num of caps removed ",numOfRemovedCap);
}
////////////////////////////////////////////////////////
BYTE CCapH323::IsFoundOrH263H261()
{
		capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
		char*      tempPtr    = (char*)pCapBuffer;
		char*	   	pTempPtr   	= (char*)pCapBuffer;
		BYTE isH261 = FALSE;
		BYTE isH263 = FALSE;
		for (int i=0; i<m_numOfCaps; i++)
		{
			//PTRACE2INT(eLevelInfoNormal,"REMOVEH261H263 -this is the capcodetype - ",pCapBuffer->capTypeCode);
			CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);
			if (pCap && (CapEnum)pCapBuffer->capTypeCode == eH261CapCode )
			{
				isH261 = TRUE;
			}
			else if (pCap && (CapEnum)pCapBuffer->capTypeCode == eH263CapCode && pCap->GetRole()== kRolePeople )
			{
				isH263 = TRUE;
			}
			POBJDELETE (pCap);
			pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)pTempPtr;


		}
		if(isH263 || isH261)
			return TRUE;
		else
			return FALSE;
}
//////////////////////////////////////////////////////////////////////////
DWORD CCapH323::GetMaxFsAccordingToProfile(APIU16 profile)
{
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;
	char*	   	pTempPtr   	= (char*)pCapBuffer;
	DWORD maxFS = 0;
	for (int i=0; i<m_numOfCaps; i++)
	{
		//PTRACE2INT(eLevelInfoNormal,"REMOVEH261H263 -this is the capcodetype - ",pCapBuffer->capTypeCode);
		CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);
		if (pCap && (CapEnum)pCapBuffer->capTypeCode == eH264CapCode && ((CH264VideoCap *)pCap)->GetProfile() == profile )
		{
			//DWORD &fs =
			maxFS = ((CH264VideoCap *)pCap)->GetFs();
			PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetMaxFsAccordingToProfile - fs found is ,",maxFS);
	        if (maxFS == INVALID)
	        {
	           CH264Details thisH264Details = (WORD)(((CH264VideoCap *)pCap)->GetLevel());
	           maxFS = thisH264Details.GetDefaultFsAsDevision();
	        }
			POBJDELETE (pCap);
			break;
		}
		POBJDELETE (pCap);
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
	PTRACE2INT(eLevelInfoNormal,"CCapH323::GetMaxFsAccordingToProfile - fs is ,",maxFS);
	return maxFS;

}
//////////////////////////////////////////////////////////////////////////
long CCapH323::GetMaxMbpsAccordingToProfile(APIU16 profile)
{
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;
	char*	   	pTempPtr   	= (char*)pCapBuffer;
	DWORD maxMbps = 0;
	for (int i=0; i<m_numOfCaps; i++)
	{
		//PTRACE2INT(eLogLevelDEBUG,"REMOVEH261H263-this is the capcodetype - ",pCapBuffer->capTypeCode);
		CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);
		if (pCap && (CapEnum)pCapBuffer->capTypeCode == eH264CapCode && ((CH264VideoCap *)pCap)->GetProfile() == profile )
		{
			//DWORD &Mbps =
			maxMbps = ((CH264VideoCap *)pCap)->GetMbps();
			PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetMaxMbpsAccordingToProfile - Mbps found is ,",maxMbps);
	        if (maxMbps == INVALID)
	        {
	           CH264Details thisH264Details = (WORD)(((CH264VideoCap *)pCap)->GetLevel());
	           maxMbps = thisH264Details.GetDefaultMbpsAsDevision();
	        }
			POBJDELETE (pCap);
			break;
		}
		POBJDELETE (pCap);
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}
	PTRACE2INT(eLevelInfoNormal,"CCapH323::GetMaxMbpsAccordingToProfile - Mbps is ,",maxMbps);
	return maxMbps;

}

/////////////////////////////////////////////////////////////////////////
BYTE CCapH323::IsTipCompatibleContentSupported() const
{
	BYTE        bRes             = FALSE;
	APIS32      is264tipContent  = 1;
	capBuffer* 	pCapBuffer       = (capBuffer *)&m_pCap->caps;
    char*	   	pTempPtr     	 = (char*)pCapBuffer;

	for (int i = 0; ((i < m_numOfCaps) && (is264tipContent == 1)); i++)
	{
		if (pCapBuffer->capTypeCode == eH264CapCode)
		{
			CH264VideoCap* pContentCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pCapBuffer->dataCap);
			if (pContentCap)
				is264tipContent = pContentCap->GetH264mode();
			POBJDELETE(pContentCap);
		}
		pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTempPtr;
	}

	if (is264tipContent == H264_tipContent)
	{
		bRes = TRUE;
	}
	PTRACE2INT(eLevelInfoNormal,"CCapH323::IsTipCompatibleContentSupported ",bRes);
	return bRes;
}


////////////////////////////////////////////////////////////
WORD CCapH323::TransferRemoteCapsToRemoteTxModeAndRemove(CCopVideoTxModes* pCopRemoteVideoTxModes)
{
	WORD		numOfRemovedCaps	= 0;
	WORD        numOfRemovedCapsEncryption = 0;
		BOOL		bIsNeedRestart		= TRUE;
		BYTE		bIsEncryptedParty	= IsPartyEncrypted();
		BOOL        IsRemoveonlySpecificProfie = FALSE;
		BOOL        numOfTxModes = 0;
		capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
		char*      tempPtr    = (char*)pCapBuffer;
		char*	   	pTempPtr   	= (char*)pCapBuffer;
		for (int i=0; i<m_numOfCaps; i++)
		{
				//PTRACE2INT(eLevelInfoNormal,"REMOVEH261H263 -this is the capcodetype - ",pCapBuffer->capTypeCode);
				CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap);
				if (pCap)
				{
						if(pCap->IsType(cmCapVideo) && pCap->GetRole() == kRolePeople && pCap->GetDirection() == cmCapTransmit)
						{

							if(numOfTxModes < 4)
							{
								CVidModeH323* pVidMode = new CVidModeH323();
								pVidMode->Create(((CapEnum)pCapBuffer->capTypeCode),pCap->SizeOf() , ((BYTE *)pCap->GetStruct()));
								pCopRemoteVideoTxModes->SetVideoTxMode(numOfTxModes,pVidMode);
								POBJDELETE(pVidMode);
							}
							else
							{
								PTRACE2INT(eLevelError,"CCapH323::TransferRemoteCapsToRemoteTxModeAndRemove - more than 4 tx caps ,",numOfTxModes);
								DBGPASSERT(102);
							}
							numOfTxModes++;

						}



						POBJDELETE(pCap);
				}

				POBJDELETE (pCap);
				pTempPtr  += sizeof(capBufferBase) + pCapBuffer->capLength;
				pCapBuffer = (capBuffer*)pTempPtr;
		}



		return numOfRemovedCaps;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapH323::UpdateCapsForHdVswInMixedMode(const CComModeH323* pScm, const VideoOperationPoint*  pOperationPoint)
{
    this->Dump("CCapH323::UpdateCapsForHdVswInMixedMode begin:", eLevelInfoNormal);

    if (pOperationPoint==NULL)
    {
        DBGPASSERT(1);
        return;
    }

    // get Cap from op. point
	CH264VideoCap* pOpPointCap = (CH264VideoCap*)CBaseCap::AllocNewCap(eH264CapCode, NULL);

	if (pOpPointCap==NULL)
	{
		DBGPASSERT(1);
		return;
	}


	pOpPointCap->InitAccordingToOperationPoint(*pOperationPoint);

    // go over all caps and lower the HD720 caps if needed
    CH264VideoCap* pMaxLevelCap = NULL;
	capBuffer* pCapBuffer = (capBuffer *) &m_pCap->caps;
	char*      tempPtr    = (char*)pCapBuffer;
    DWORD details = 0;
    DWORD valuesToCompare = kCapCode|kH264Profile|kH264Additional|kBitRate;
    DWORD valuesNoProfileToCompare = kCapCode|kH264Additional|kBitRate;
    BYTE bIsAtleastHD720 = FALSE;
    DWORD localVideoRate = 0;

    bool flag = false;
	for (int i=0; i<m_numOfCaps; i++)
	{
		CH264VideoCap* pCap = (CH264VideoCap*)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, pCapBuffer->dataCap);
//		  COstrStream msg;
//		  pCap->Dump(msg);
//		  TRACEINTO << "CCapH323::ReplaceHighestVideoCapForHdVswInMixedMode Retrieved Cap \n" << msg.str().c_str();

		if (pCap && pCap->GetCapCode() == eH264CapCode && CheckRole(kRolePeople,i,pCap))
		{
//		  COstrStream msg;
//		  pCap->Dump(msg);
//		  TRACEINTO << "CCapH323::ReplaceHighestVideoCapForHdVswInMixedMode Check Cap \n" << msg.str().c_str();

                      //FSN-613: Dynamic Content for SVC/Mix Conf: check if need to limit videorate
			if (!bIsAtleastHD720 && pCap->IsCapableOfHD720At30())
			{
				bIsAtleastHD720 = TRUE;
				localVideoRate = pCap->GetBitRate();
				TRACEINTO << "localVideoRate = " << localVideoRate;
			}

			if (pCap->IsContaining(*pOpPointCap, valuesToCompare, &details))
			{// lower the cap
				TRACEINTO << "Update high profile cap #" << i;
				pCap->SetAccordingToOperationPoint(*pOperationPoint, true);
				flag = true;
			}
			else if (pCap->IsContaining(*pOpPointCap, valuesNoProfileToCompare, &details))
			{// lower the cap
				TRACEINTO << "Update base profile cap #" << i;
				pCap->SetAccordingToOperationPoint(*pOperationPoint, false);
				flag = true;
			}
		}
		POBJDELETE(pCap);

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}


	POBJDELETE(pOpPointCap);

	// Update the video bit rate for all caps
	if (flag || (bIsAtleastHD720 && localVideoRate > (pOperationPoint->m_maxBitRate * 10)))
	{
		int videoRate = (pOperationPoint->m_maxBitRate + this->GetAudioDesiredRate())*10;
		TRACEINTO << "GetAudioDesiredRate()=" << GetAudioDesiredRate() << " Setting videoBitRate to " << videoRate;
		
		//FSN-613: Dynamic Content for SVC/Mix Conf, since bitRate = 1232 in pOperationPoint for HD1080, while it is different from AVC requirement for HD1080. (1536)
		/*if (pOperationPoint->m_rsrcLevel == eResourceLevel_HD1080)
		{
		       videoRate = pScm->GetVideoBitRate(cmCapReceive, kRolePeople);
			//videoRate = videoRate +  this->GetAudioDesiredRate()*10;
			//videoRate = CResRsrcCalculator::GetRateThrshldBasedOnVideoModeType(GetSystemCardsBasedMode(), m_videoQuality, isHighProfile, eHD1080Symmetric); 
			TRACEINTO << "videoRate = " << videoRate;
		 }*/
		
		SetVideoBitRate(videoRate);
	}
	else
	{
        TRACEINTO << "Local caps are less than highest operation point. No need to lower them.";	
	}

       this->Dump("CCapH323::UpdateCapsForHdVswInMixedMode end:", eLevelInfoNormal);
}
