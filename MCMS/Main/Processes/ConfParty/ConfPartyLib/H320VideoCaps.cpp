//+========================================================================+
//                            H320VideoCaps.CPP                            |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H320VideoCaps.CPP                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |  01/10/07  |                                                      |
//+========================================================================+


#include <iomanip>

#include "H221.h"
#include "Segment.h"
#include "H320VideoCaps.h"
#include "ConfPartyGlobals.h"
#include "TraceStream.h"
#include "ConfigHelper.h"

using namespace std;



CCapH261::CCapH261()
{
    m_dataVidCap = 0L;
	m_cifMpi     = 0;
	m_qCifMpi    = 0;
}

CCapH261::CCapH261(const CCapH261& other) : CPObject(other)
{
    m_dataVidCap = other.m_dataVidCap;
	m_cifMpi     = other.m_cifMpi;
	m_qCifMpi    = other.m_qCifMpi;
}

void CCapH261::Dump(std::ostream& msg) const
{
    msg << "\nCCapH261::Dump\n"
		<< "-----------\n"
		<< setw(20) << "m_dataVidCap" << (hex) << m_dataVidCap    << "  " << GetResolutionName()   << "\n"
		<< setw(20) << "m_cifMpi"     << (hex) << (WORD)m_cifMpi  << "  " << GetMpiName(m_cifMpi)  << "\n"
		<< setw(20) << "m_qCifMpi"    << (hex) << (WORD)m_qCifMpi << "  " << GetMpiName(m_qCifMpi) << "\n";
}

const char* CCapH261::GetResolutionName()const
{
    switch ( m_dataVidCap ) {
	    case V_Qcif:
		    return "V_Qcif";
	    case V_Cif:
		    return "V_Cif";
	}
	return "Unknown resolution";
}

const char* CCapH261::GetMpiName(BYTE mpi)const
{
    switch ( mpi )  {
        case V_1_29_97 :
            return "V_1_29_97";
        case V_2_29_97 :
		  return "V_2_29_97";
        case V_3_29_97 :
		  return "V_3_29_97";
        case V_4_29_97:
		  return "V_4_29_97";
	}
	return "Unknown mpi";
}

const char* CCapH261::NameOf() const
{
	return "CCapH261";
}


void CCapH261::SetCaps(WORD resolution, WORD mpi)
{
    if(mpi < V_1_29_97 || mpi > V_4_29_97) {
	    PTRACE(eLevelInfoNormal,"CCapH261::SetCaps - illegal mpi value, will not be set");
		return;
	}
	if(resolution == V_Qcif){
		// set qcif
        m_dataVidCap = V_Qcif;
		// set qcif mpi
		m_qCifMpi = mpi;
	}
	else if(resolution == V_Cif){
		// set cif
	    m_dataVidCap = V_Cif;
		// set cif mpi
		m_cifMpi = mpi;

		// set qcif mpi
		if(m_qCifMpi==0 || (m_qCifMpi>m_cifMpi && m_cifMpi!=0)){
			m_qCifMpi = mpi;
		}
	} else {
	    PTRACE(eLevelInfoNormal,"CCapH261::SetH261Caps illegal resolution");//|HC_PHASE2_TRACE
	}
	return;
}

void CCapH261::CreateDefault()
{
    m_dataVidCap = V_Cif;
	m_cifMpi = V_1_29_97;
	m_qCifMpi = V_1_29_97;
}

void CCapH261::ResetCaps()
{
    m_dataVidCap = 0L;
}

void CCapH261::Serialize(WORD format,CSegment& H221StringSeg)
{
    BYTE seriableVal = 0;
    switch(format) {
	case NATIVE:
	  {
		H221StringSeg << m_dataVidCap
					  << m_cifMpi
					  << m_qCifMpi;
	  } break;
	case SERIALEMBD:
	  {
		if ( IsH261VideoCap(V_Cif) )  {
		  seriableVal = V_Cif | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		  seriableVal = m_qCifMpi | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		  seriableVal = m_cifMpi | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;

		} else if ( IsH261VideoCap(V_Qcif) )  {
		  seriableVal = V_Qcif | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		  seriableVal = m_qCifMpi | DATAVIDCAPATTR;
		  H221StringSeg << seriableVal;
		}
	  } break;
	}
}

void CCapH261::DeSerialize(WORD format, CSegment& H221StringSeg)
{
     switch(format) {
	 case NATIVE:
	  {
		H221StringSeg >> m_dataVidCap
					  >> m_cifMpi
					  >> m_qCifMpi;
	  } break;
	 case SERIALEMBD:
	   PTRACE(eLevelInfoNormal, "CCapH261::DeSerialize - case SERIALEMBD - do nothing");
	   break;
	 }
}

void CCapH261::SetVideoCapQcif(CSegment& seg)
{
    BYTE basMpi;
	CSegment  segMpi;
    //seg >> basMpi; //this causes the following assert "Segment.cpp:596: error(50)"
	BYTE byte_size = sizeof (BYTE);
	segMpi.Create(byte_size);
	seg >> segMpi;
	segMpi >> basMpi;

    basMpi ^= DATAVIDCAPATTR;

	SetCaps (V_Qcif, basMpi);

//     switch ( basMpi )  {
//         case V_1_29_97 : {
//             SetDataVidCap(V_1_29_97);
//             SetDataVidCap(V_2_29_97);
//             SetDataVidCap(V_3_29_97);
//             SetDataVidCap(V_4_29_97);
//             break;
//         }
//         case V_2_29_97 : {
//             SetDataVidCap(V_2_29_97);
//             SetDataVidCap(V_3_29_97);
//             SetDataVidCap(V_4_29_97);
//             break;
//         }
//         case V_3_29_97 : {
//             SetDataVidCap(V_3_29_97);
//             SetDataVidCap(V_4_29_97);
//             break;
//         }
//         case V_4_29_97: {
//             SetDataVidCap(V_4_29_97);
//             break;
//         }
//         default:
// 		  break;
// 	}
//   }
}

void CCapH261::SetVideoCapCif(CSegment &seg)
{
    BYTE    basMpi;
	CSegment segMpi;

    SetVideoCapQcif(seg);   // set qcif min pic interval

//     seg >> basMpi; //this causes the following assert "Segment.cpp:596: error(50)"
	BYTE byte_size = sizeof (BYTE);
	segMpi.Create(byte_size);
	seg >> segMpi;
	segMpi >> basMpi;   // set cif min pic interval

    basMpi ^= DATAVIDCAPATTR;

	SetCaps (V_Cif, basMpi);

//     switch ( basMpi )  {
//         case V_1_29_97 : {
//             SetDataVidCap(V_1_29_97);
//             SetDataVidCap(V_2_29_97);
//             SetDataVidCap(V_3_29_97);
//             SetDataVidCap(V_4_29_97);
//             break;
//         }
//         case V_2_29_97 : {
//             SetDataVidCap(V_2_29_97);
//             SetDataVidCap(V_3_29_97);
//             SetDataVidCap(V_4_29_97);
//             break;
//         }
//         case V_3_29_97 : {
//             SetDataVidCap(V_3_29_97);
//             SetDataVidCap(V_4_29_97);
//             break;
//         }
//         case V_4_29_97 : {
//             SetDataVidCap(V_4_29_97);
//             break;
//         }
//         default : {
//             break;
//         }
//     }
}



WORD CCapH261::GetH261CapMpi(WORD cap) const
{
    switch (cap) {
	case V_Cif:
	  return m_cifMpi;
	case V_Qcif:
	  return m_qCifMpi;
	}
	return 0;
}

BOOL CCapH261::IsH261VideoCap(WORD cap) const
{
	if ( (V_Cif  == cap &&  V_Cif == m_dataVidCap) ||
		 (V_Qcif == cap && (V_Cif == m_dataVidCap  ||
							V_Qcif == m_dataVidCap)) )
	  return TRUE;

    return FALSE;
}



///////////////////////////////////////////////////////////////////////////////

CVideoCap::CVideoCap()
{
	m_isMBE = Mbe_Cap;
//  m_isFailH263 = 0;
// 	m_isFailH264 = 0;
}

CVideoCap::CVideoCap(const CVideoCap& other) : CPObject(other)
{
    m_h261cap = other.m_h261cap;
	m_isMBE   = other.m_isMBE;
 	m_h263cap = other.m_h263cap;
	m_h264cap = other.m_h264cap;
// 	m_isFailH263 = other.m_isFailH263;
// 	m_isFailH264 = other.m_isFailH264;
}

const char* CVideoCap::NameOf() const
{
    return "CVideoCap";
}

CVideoCap& CVideoCap::operator= (const CVideoCap& other)
{
	m_h261cap 		= other.m_h261cap;
    m_h263cap       = other.m_h263cap;
    m_h264cap       = other.m_h264cap;
 	return *this;
}

void CVideoCap::Dump(std::ostream& msg) const
{
    msg << "\n==================    CVideoCap::Dump    ==================\n"
		<< setw(20) << "m_isMBE" << (dec) << (WORD)m_isMBE << (Mbe_Cap == m_isMBE ? " Mbe_Cap is set" : " Mbe_Cap is unset") << "\n";
// 		<< setw(20) << "m_isFailH263"	  << (hex) << m_isFailH263 << "\n"
// 		<< setw(20) << "m_isFailH264"	  << (hex) << m_isFailH264 << "\n";

    m_h261cap.Dump(msg);
	m_h263cap.Dump(msg);
    m_h264cap.Dump(msg);
}

BOOL CVideoCap::IsVideoCapSupported(WORD cap) const
{
    BOOL ret = m_h261cap.IsH261VideoCap(cap);//olga - what about H263_2000
	if (!ret)
	    m_h263cap.IsH263VideoCap(cap);
	return ret;
}

void CVideoCap::Serialize(WORD format,CSegment& stringSeg)
{
    m_h261cap.Serialize(format, stringSeg);
	m_h263cap.Serialize(format, stringSeg);
	m_h264cap.Serialize(format, stringSeg);
}

void CVideoCap::DeSerialize(WORD format, CSegment& stringSeg)
{
    m_h261cap.DeSerialize(format, stringSeg);
	m_h263cap.DeSerialize(format, stringSeg);
	m_h264cap.DeSerialize(format, stringSeg);
}

void CVideoCap::SetVideoCapCif(WORD mpi, WORD mpiQcif)
{
    m_h261cap.SetCaps(V_Cif, mpi);
}

void CVideoCap::SetVideoCapQcif(WORD mpiQcif)
{
    m_h261cap.SetCaps(V_Qcif, mpiQcif);
}

void CVideoCap::SetVideoCapQcif(CSegment& seg)
{
    m_h261cap.SetVideoCapQcif(seg);
}

void CVideoCap::SetVideoCapCif(CSegment& seg)
{
    m_h261cap.SetVideoCapCif(seg);
}

void CVideoCap::ResetVideoCaps()
{
    m_h261cap.ResetCaps();
}

void CVideoCap::CreateDefault()
{
    m_h261cap.CreateDefault();//SetVidCap(V_Cif,V_1_29_97,V_1_29_97);
	m_isMBE = Mbe_Cap;

	m_h263cap.CreateDefault();
 	m_h264cap.CreateDefault();
}

void CVideoCap::SetVideoMBECap(BYTE cap)
{
    PTRACE(eLevelInfoNormal, "CVideoCap::SetVideoMBECap is called");
    m_isMBE = cap;
}

void CVideoCap::SetVideoH263Cap(BYTE cap)
{
    if (H263_2000 == cap)
	    m_h263cap.SetCapH263_2000();
}

void CVideoCap::RemoveVideoMBECap()
{
    m_isMBE = 0;
}

WORD CVideoCap::GetH261CapMpi(WORD cap) const
{
    return m_h261cap.GetH261CapMpi(cap);
}

BOOL CVideoCap::IsMBECap(BYTE cap)const
{
    return (cap == m_isMBE);
}

BOOL CVideoCap::IsH261VideoCap(WORD cap) const
{
    return m_h261cap.IsH261VideoCap(cap);
}

CCapH263* CVideoCap::GetCapH263()
{
    return &m_h263cap;
}

WORD CVideoCap::IsH263() const
{
	WORD numOfSets = m_h263cap.GetNumberOfH263Sets();
	return numOfSets;
}

WORD CVideoCap::IsH264() const
{
	WORD numOfSets = m_h264cap.GetNumberOfH264Sets();
	return numOfSets;
}

CCapH264* CVideoCap::GetCapH264()
{
    return &m_h264cap;
}

void CVideoCap::CreateH264Cap(CSegment& seg, DWORD len)
{
    m_h264cap.Create(seg,len);
}

void CVideoCap::CreateH263Cap(CSegment& seg, BYTE mbeLen)
{
    m_h263cap.Create(seg, mbeLen);
}

void CVideoCap::InsertH263CapSet(CCapSetH263* pH263CapSetBuf)
{
    m_h263cap.InsertH263CapSet(pH263CapSetBuf);
}

void CVideoCap::InsertH264CapSet(CCapSetH264* pH264CapSetBuf)
{
    m_h264cap.InsertH264CapSet(pH264CapSetBuf);
}

///////////////////////////////////////////////////////////
BYTE CVideoCap::IsCapableOfHD720_15() const
{
    return m_h264cap.IsCapableOfHD720_15();
}
///////////////////////////////////////////////////////////
BYTE  CVideoCap::IsCapableOfHD1080_15() const
{
	return m_h264cap.IsCapableOfHD1080_15();
}
///////////////////////////////////////////////////////////
BYTE  CVideoCap::IsCapableOfHD720_50() const
{
	return m_h264cap.IsCapableOfHD720_50();
}
///////////////////////////////////////////////////////////

BYTE CVideoCap::IsCapableOfVideo() const
{

	return (IsH264() || IsH263() || IsH261VideoCap(V_Qcif) || IsH261VideoCap(V_Cif));
}
///////////////////////////////////////////////////////////
eVideoPartyType CVideoCap::GetCPVideoPartyTypeAccordingToCapabilities(BYTE isH2634Cif15Supported)
{
	eVideoPartyType videoPartyType = eVideo_party_type_none;
	if(IsCapableOfVideo())
	{
		videoPartyType = GetH261H263ResourcesPartyType(0); // basic cif resource
		eVideoPartyType h264VideoPartyType = eVideo_party_type_none;
		eVideoPartyType h263VideoPartyType = eVideo_party_type_none;
		if(IsH264())
		{
			h264VideoPartyType = m_h264cap.GetCPVideoPartyTypeAccordingToCapabilities();
			TRACESTR(eLevelInfoNormal) << "CVideoCap::GetCPVideoPartyTypeAccordingToCapabilities , h264VideoPartyType = " << eVideoPartyTypeNames[h264VideoPartyType];
		}
		if(IsH263())
		{
			h263VideoPartyType = m_h263cap.GetCPVideoPartyTypeAccordingToCapabilities(isH2634Cif15Supported);
			TRACESTR(eLevelInfoNormal) << "CVideoCap::GetCPVideoPartyTypeAccordingToCapabilities , h263VideoPartyType = " << eVideoPartyTypeNames[h263VideoPartyType];
		}
		if((h264VideoPartyType!=eVideo_party_type_none) || (h263VideoPartyType!=eVideo_party_type_none))
		{
			if( h263VideoPartyType == eCP_H261_H263_upto_CIF_video_party_type && h264VideoPartyType == eCP_H264_upto_CIF_video_party_type )
			{
					videoPartyType = h264VideoPartyType;	//VNGFE-3451- prefer 264 in case of CIF even though the 263 is bigger.
															//(eCP_H261_H263_upto_CIF_video_party_type > eCP_H264_upto_CIF_video_party_type in breeze CIF).
			}
			else
			{
				if (h264VideoPartyType >= h263VideoPartyType)
					videoPartyType = h264VideoPartyType;
				else
					videoPartyType = h263VideoPartyType;
			}
		}

        eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
        BOOL bH261Support = GetSystemCfgFlagInt<BOOL>("H261_SUPPORT_ALWAYS");
		if( bH261Support && systemCardsBasedMode == eSystemCardsMode_mpmrx)
        {      
            if ((h264VideoPartyType == eVideo_party_type_none) && (h263VideoPartyType == eVideo_party_type_none))
					videoPartyType = eCP_H261_CIF_equals_H264_HD1080_video_party_type;
        }
                
	}
  
	TRACESTR(eLevelInfoNormal) << "CVideoCap::GetCPVideoPartyTypeAccordingToCapabilities , videoPartyType = " << eVideoPartyTypeNames[videoPartyType];
	return videoPartyType;
}
eVideoPartyType CVideoCap::GetCPVideoPartyTypeAccordingToLocalCapabilities(BYTE isH2634Cif15Supported)
{
  eVideoPartyType videoPartyType = eVideo_party_type_none;
  if(IsCapableOfVideo())
    {
      videoPartyType = GetH261H263ResourcesPartyType(0); // basic cif resource
      eVideoPartyType h264VideoPartyType = eVideo_party_type_none;
      eVideoPartyType h263VideoPartyType = eVideo_party_type_none;
      if(IsH264())
	{
	  h264VideoPartyType = m_h264cap.GetCPVideoPartyTypeAccordingToCapabilities();
	  TRACESTR(eLevelInfoNormal) << "CVideoCap::GetCPVideoPartyTypeAccordingToLocalCapabilities , h264VideoPartyType = " << eVideoPartyTypeNames[h264VideoPartyType];
	}
      if(IsH263())
	{
	  h263VideoPartyType = m_h263cap.GetCPVideoPartyTypeAccordingToCapabilities(isH2634Cif15Supported);
	  TRACESTR(eLevelInfoNormal) << "CVideoCap::GetCPVideoPartyTypeAccordingToLocalCapabilities , h263VideoPartyType = " << eVideoPartyTypeNames[h263VideoPartyType];
	}
      if((h264VideoPartyType!=eVideo_party_type_none) || (h263VideoPartyType!=eVideo_party_type_none))
	{
	  if (h264VideoPartyType>=h263VideoPartyType)
	    videoPartyType = h264VideoPartyType;
	  else
	    videoPartyType = h263VideoPartyType;
	}
    }

    

TRACESTR(eLevelInfoNormal) << "CVideoCap::GetCPVideoPartyTypeAccordingToLocalCapabilities , videoPartyType = " << eVideoPartyTypeNames[videoPartyType];
return videoPartyType;
}
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
void CVideoCap::SetOneH263Cap(BYTE format, int mpi)
{
    m_h263cap.SetOneH263Cap(format, mpi);
}

void CVideoCap::SendSecondAdditionalCap(WORD OnOff)
{
    m_h263cap.SendSecondAdditionalCap(OnOff);
}

void CVideoCap::RemoveH261Caps()
{
    m_h261cap.ResetCaps();
}
