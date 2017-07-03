//+========================================================================+
//                         H320COMMODE.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H320COMMODE.CPP                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 14/10/07   |                                                      |
//+========================================================================+

#include <sstream>
#include <iomanip>

#include "Segment.h"
#include "EncryptionKey.h"
#include "ObjString.h"
#include "Macros.h"
#include "CDRUtils.h"
#include "CommConf.h"
#include "NonStandardCaps.h"
#include "H221.h"
#include "H263.h"
#include "H264Util.h"
#include "H320Caps.h"
#include "H320CapPP.h"
#include "H320ComMode.h"
#include "TraceStream.h"
#include "ConfPartyGlobals.h"
#include "IpCommon.h"

// #include "ConfPartyDefines.h" //olga - H26L but it is commented
// #include "ConfPartyGlobals.h" //olga - IsT120Valid() but it is commented


using namespace std;

/////////////////////////////////////////////////////////////////////////////
#define STATUS_UNEXPECTED_T120_BIT_RATE  1018

#define ITU_STANDARD_MODE					0x00
#define PROPRIETARY_MODE					0x01
#define RESERVED_MODE						0x02

#define RESOLUTION_QCIF						176
#define RESOLUTION_CIF						352

#define FRAME_RATE_30						1
#define FRAME_RATE_15						2


/////////////////////////////////////////////////////////////////////////////


static WORD IsT120Valid(BYTE transferRate, BYTE audRate,BYTE restrictMode, BYTE  T120Rate)
{
    DWORD audio_bitrate;
	DWORD video_bitrate;
	DWORD mlp_bitrate = 0;
	DWORD hmlp_bitrate = 0;
	DWORD lsd_bitrate = 0;
	DWORD hsd_bitrate = 0;
	DWORD content_bitrate = 0;

	//T120 bit rate test
	if (T120Rate) {
	  switch(T120Rate){
       case MLP_6_4k:
       case Mlp_14_4:
       case Mlp_22_4:
       case Mlp_30_4:
       case Mlp_38_4:
       case Mlp_46_4:
       case Mlp_16:
       case Mlp_24:
       case Mlp_32:
       case Mlp_40:
	   case H_Mlp_Com_14_4:
	   case H_Mlp_Com_62_4:
	   case H_Mlp_Com_64:
	   case H_Mlp_Com_128:{break;}
       default : {
	     return STATUS_UNEXPECTED_T120_BIT_RATE;
	   }
	  }
	  if( transferRate==Xfer_64) {
		if (T120Rate==H_Mlp_Com_64 || T120Rate==H_Mlp_Com_62_4 || T120Rate==H_Mlp_Com_128)
		  return STATUS_UNEXPECTED_T120_BIT_RATE;
	  }
	  if((transferRate==Xfer_128 || transferRate==Xfer_2x64) && T120Rate==H_Mlp_Com_128)
		return STATUS_UNEXPECTED_T120_BIT_RATE;

      if(transferRate==Xfer_2x64 || transferRate==Xfer_3x64 ||
	     transferRate==Xfer_4x64 || transferRate==Xfer_5x64 ||
  	     transferRate==Xfer_6x64) {

		 if (T120Rate==H_Mlp_Com_64)
		   return STATUS_UNEXPECTED_T120_BIT_RATE;
	  }
	  else {
		if (T120Rate==H_Mlp_Com_62_4  || T120Rate == H_Mlp_Com_14_4)
		  return STATUS_UNEXPECTED_T120_BIT_RATE;
	  }
	}


	// Minimum left Video
	const WORD MINIMAL_VIDEO_BITRATE = 0;

	CComMode* TestedComMode = new CComMode;

	// set audio param
	CAudMode  audMode;
	audMode.SetAudMode((WORD)audRate);
	audMode.SetBitRate((WORD)audRate);
	TestedComMode->SetAudMode(audMode);

	// set restrict mode
	COtherMode  otherMode;
	otherMode.SetRestrictMode((WORD)restrictMode);
	TestedComMode->SetOtherMode(otherMode);

	// set transfer rate
	TestedComMode->SetXferMode((WORD)transferRate);

	// mlp

	CLsdMlpMode  lsdMlpScm;
	if(T120Rate && T120Rate >= MLP_4k){
      lsdMlpScm.SetMlpMode((WORD)T120Rate);
	}

	TestedComMode->SetLsdMlpMode(lsdMlpScm);

	// hmlp

	CHsdHmlpMode  hsdMlpScm;
	if(T120Rate && ((T120Rate >= H_Mlp_Com_62_4 && T120Rate <= H_Mlp_Com_384) || T120Rate == H_Mlp_Com_14_4)){
	    hsdMlpScm.SetHmlpMode((WORD)T120Rate);
	}
	TestedComMode->SetHsdHmlpMode(hsdMlpScm);

	// calculate left space for video

	TestedComMode->GetMediaBitrate(audio_bitrate, video_bitrate,lsd_bitrate,hsd_bitrate, mlp_bitrate,hmlp_bitrate,content_bitrate);

	POBJDELETE(TestedComMode);

	if(video_bitrate > MINIMAL_VIDEO_BITRATE)
	   return STATUS_OK;
	else
	   return 	 STATUS_UNEXPECTED_T120_BIT_RATE;

}

/////////////////////////////////////////////////////////////////////////////
void CComMode::Dump(WORD  switchFlag)
{
	std::ostringstream  msg;

    WORD CBitRate;
    msg <<"- - Audio  - -|- - Video- - -|- Transfer - -|- - Hsd  - - -|- - Hmlp - - -|\n    "
        << CCDRUtils::Get_Audio_Coding_Command_BitRate((BYTE)GetAudMode(),&CBitRate);

	if(GetVidMode() == H26L)
		msg << setw(15) << "H.264*";
	else
        msg << setw(15) << CCDRUtils::Get_Video_Oth_Command_BitRate((BYTE)GetVidMode(),&CBitRate);

	msg << setw(15) << CCDRUtils::Get_Transfer_Rate_Command_BitRate((BYTE)GetXferMode(),&CBitRate)
        << setw(15) << CCDRUtils::Get_Hsd_Hmlp_command_Bitrate((BYTE)GetHsdMode(),&CBitRate)
        << setw(15) << CCDRUtils::Get_Hsd_Hmlp_command_Bitrate((BYTE)GetHmlpMode(),&CBitRate) << "\n\n";

    msg << "- - Lsd- - - -|- - Mlp- - - -|\n    "
        << CCDRUtils::Get_Lsd_Mlp_Command_BitRate((BYTE)GetLsdMode(),&CBitRate)
        << setw(15)<< CCDRUtils::Get_Lsd_Mlp_Command_BitRate((BYTE)GetMlpMode(),&CBitRate)   << "\n\n";

    msg << "- H06Bcom- - -|- Encryption -|- Restrict - -|- Loop - - - -|\n   "
        << CCDRUtils::Get_Video_Oth_Command_BitRate((BYTE)GetOtherH06BcompMode(),&CBitRate)
        << setw(15) <<CCDRUtils::Get_Video_Oth_Command_BitRate((BYTE)GetOtherEncrypMode(),&CBitRate)
        << setw(15) <<CCDRUtils::Get_Video_Oth_Command_BitRate((BYTE)GetOtherRestrictMode(),&CBitRate)
        << setw(15) <<CCDRUtils::Get_Video_Oth_Command_BitRate((BYTE)GetOtherLoopMode(),&CBitRate) << "\n\n";


    msg << "H221 STRING DUMP\n";
    msg << "----------------\n";
    m_H221string.Dump(msg);

    msg << "\n";

    if (m_bIsHd720Enabled)
    	msg << "HD720 30fps is Enabled\n";

    if (m_bIsHd1080Enabled)
    	msg << "HD1080 is Enabled";

    if (m_bIsHd720At60Enabled)
        msg << "HD720 60fps is Enabled";

    if (m_bIsHd1080At60Enabled)
        msg << "HD1080 60fps is Enabled";

    PTRACE(eLevelInfoNormal, (char*)msg.str().c_str());//NTERACTIVE_TRACE
}

/////////////////////////////////////////////////////////////////////////////
CComMode::CComMode()          // constructor
{
	m_bShouldDisconnectOnEncryptionFailure = TRUE;
	m_bIsHd720Enabled = FALSE;
	m_bIsHd1080Enabled = FALSE;
	m_bIsHd720At60Enabled = FALSE;
	m_bIsHd1080At60Enabled = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CComMode::~CComMode()        // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
const char*   CComMode::NameOf()  const
{
    return "CComMode";
}
/////////////////////////////////////////////////////////////////////////////
CComMode::CComMode( CComMode& other ) : CPObject(other)
{
	m_audMode = other.m_audMode;
	m_vidMode = other.m_vidMode;
	m_xferMode = other.m_xferMode;
	m_hsdHmlpMode= other.m_hsdHmlpMode;
	m_lsdMlpMode = other.m_lsdMlpMode;
	m_otherMode = other.m_otherMode;
	m_nsMode = other.m_nsMode;
	m_contentMode = other.m_contentMode;
	m_H221string = other.m_H221string;
	m_ECSMode = other.m_ECSMode;
	m_bIsHd720Enabled = other.m_bIsHd720Enabled;
	m_bIsHd1080Enabled = other.m_bIsHd1080Enabled;
	m_bIsHd720At60Enabled = other.m_bIsHd720At60Enabled;
	m_bShouldDisconnectOnEncryptionFailure = other.m_bShouldDisconnectOnEncryptionFailure;
	m_bIsHd1080At60Enabled = other.m_bIsHd1080At60Enabled;
}
/////////////////////////////////////////////////////////////////////////////
WORD CComMode::GetAudMode() const
{
	return m_audMode.GetAudMode();
}
/////////////////////////////////////////////////////////////////////////////
WORD CComMode::GetVidMode() const
{
	return m_vidMode.GetVidMode();
}
/////////////////////////////////////////////////////////////////////////////
WORD CComMode::GetXferMode() const
{
	return m_xferMode.GetXferMode();
}

/////////////////////////////////////////////////////////////////////////////
WORD CComMode::IsFreeVideoRate() const
{
	return m_vidMode.IsFreeBitRate();
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::SetFullBitRateAudioOnly()
{
	m_vidMode.SetVidMode(Video_Off);
	m_lsdMlpMode.SetMlpMode(MLP_Off);
	m_lsdMlpMode.SetLsdMode(LSD_Off);
	m_hsdHmlpMode.SetHsdMode(Hsd_Com_Off);
	m_hsdHmlpMode.SetHmlpMode(H_Mlp_Off_Com);
	m_contentMode.SetOpcode(0);
}

/////////////////////////////////////////////////////////////////////////////
void CComMode::Zero(WORD restrict)
{
	if ( restrict )
		m_otherMode.SetRestrictMode(Restrict);
	m_vidMode.SetVidMode(Video_Off);
	m_xferMode.Zero();
	m_lsdMlpMode.SetMlpMode(MLP_Off);
	m_lsdMlpMode.SetLsdMode(LSD_Off);
	m_hsdHmlpMode.SetHsdMode(Hsd_Com_Off);
	m_hsdHmlpMode.SetHmlpMode(H_Mlp_Off_Com);
	m_contentMode.SetOpcode(0);
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::SetAudMode(const CAudMode& raudMode)
{
    m_audMode = raudMode;
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::SetVidMode(const CVidMode& rvidMode)
{
    m_vidMode = rvidMode;
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::SetXferMode(const CXferMode& rxferMode)
{
    m_xferMode = rxferMode;
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::SetOtherMode(const COtherMode& rotherMode)
{
    m_otherMode = rotherMode;
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::SetLsdMlpMode(const CLsdMlpMode& rlsdMlpMode)
{
    m_lsdMlpMode = rlsdMlpMode;
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::SetHsdHmlpMode(const CHsdHmlpMode& rhsdHmlpMode)
{
    m_hsdHmlpMode = rhsdHmlpMode;
}

/////////////////////////////////////////////////////////////////////////////
WORD CComMode::VideoOn() const
{
    WORD  rc = 0;

    if ( m_vidMode.GetVidMode() != Video_Off ) rc = 1;
    return rc;
}

/////////////////////////////////////////////////////////////////////////////
WORD CComMode::HsdOn() const
{
    WORD  rc = 0;
    if ( m_hsdHmlpMode.GetHsdMode() != Hsd_Com_Off ) rc = 1;
    return rc;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CComMode::LsdOn() const
{
    WORD  rc = 0;
    if ( m_lsdMlpMode.GetLsdMode() != LSD_Off ) rc = 1;
    return rc;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CComMode::MlpOn() const
{
    WORD  rc = 0;
    if ( m_lsdMlpMode.GetMlpMode() != MLP_Off ) rc = 1;
    return rc;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CComMode::HmlpOn() const
{
    WORD  rc = 0;
    if ( m_hsdHmlpMode.GetHmlpMode() != H_Mlp_Off_Com ) rc = 1;
    return rc;
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::SetXferMode(WORD chnlWidth,WORD numChnl)
{
    WORD    xferMode = Xfer_64;

    switch ( chnlWidth ) {

        case B0 :  {

            switch ( numChnl ) {
                case 1  :  {
                    xferMode = Xfer_64;
                    break;
                }
                case 2  :  {
                    xferMode = Xfer_2x64;
                    break;
                }
                case 3  :  {
                    xferMode = Xfer_3x64;
                    break;
                }
                case 4  :  {
                    xferMode = Xfer_4x64;
                    break;
                }
                case 5  :  {
                    xferMode = Xfer_5x64;
                    break;
                }
                case 6  :  {
                    xferMode = Xfer_6x64;
                    break;
                }
                default  :  {
                    PTRACE(eLevelError,"CComMode::SetXferMode : illegal channel number!!!");
                    break;
                }
            }
        break;
        }

        case H0     :  {
            switch ( numChnl ) {
                case 1  :  {
                    xferMode = Xfer_384;
                    break;
                }
                case 2  :  {
                    xferMode = Xfer_2x384;
                    break;
                }
                case 3  :  {
                    xferMode = Xfer_3x384;
                    break;
                }
                case 4  :  {
                    xferMode = Xfer_4x384;
                    break;
                }
                case 5  :  {
                    xferMode = Xfer_5x384;
                    break;
                }
                default :  {
                    PTRACE(eLevelError,"CComMode::SetXferMode : illegal channel number!!!");
                    break;
                }
            }
            break;
        }

        case  B0*2     :  xferMode = Xfer_128; break;
        case  B0*3     :  xferMode = Xfer_192; break;
        case  B0*4     :  xferMode = Xfer_256; break;
        case  B0*5     :  xferMode = Xfer_320; break;
        case  B0*8     :  xferMode = Xfer_512; break;
        case  B0*12    :  xferMode = Xfer_768; break;
        case  B0*18    :  xferMode = Xfer_1152; break;
        case  B0*23    :  xferMode = Xfer_1472; break;
        case  B0*24    :  xferMode = Xfer_1536; break;
        case  B0*30    :  xferMode = Xfer_1920; break;
        default        :  {
            PTRACE(eLevelError,"CComMode::SetXferMode : illegal channel width!!!");
            break;
        }
    }

    m_xferMode.SetXferMode(xferMode);
}


/////////////////////////////////////////////////////////////////////////////
void CComMode::SetXferMode(WORD chnlWidth, WORD numChnl, WORD aggragation)
{
	WORD xferMode = Xfer_64;

	switch (numChnl)
	{
		case 1 : { xferMode = Xfer_64  ; break; }
		case 2 : { xferMode = aggragation ? Xfer_128 : Xfer_2x64; break; }
		case 3 : { xferMode = aggragation ? Xfer_192 : Xfer_3x64; break; }
		case 4 : { xferMode = aggragation ? Xfer_256 : Xfer_4x64; break; }
		case 5 : { xferMode = aggragation ? Xfer_320 : Xfer_5x64; break; }
		case 6 : { xferMode = aggragation ? Xfer_384 : Xfer_6x64; break; }
		case 8 : { xferMode = Xfer_512 ; break; }
		case 12: { xferMode = Xfer_768 ; break; }
		case 18: { xferMode = Xfer_1152; break; }
		case 23: { xferMode = Xfer_1472; break; }
		case 24: { xferMode = Xfer_1536; break; }
		case 30: { xferMode = Xfer_1920; break; }
		default:
		{
			TRACESTR(eLevelError) << "NumOfChannels:" << numChnl << " - Illegal channel number";
			break;
		}
	}

	m_xferMode.SetXferMode(xferMode);
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::SetXferMode(WORD xferMode)
{
    m_xferMode.SetXferMode(xferMode);
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::DeSerialize(WORD format,CSegment& seg)
{
    BYTE  bas = 0;
    CSegment copy_of_seg(seg);
    BYTE  bas2 = 0;
    CSegment copy2(seg);
    char s1[16];
    ALLOCBUFFER( s, 4096 );
    s[0] = '\0';
    WORD counter = 0;

    switch ( format )  {
        case SERIALEMBD : {

            m_H221string.DeSerialize(format,copy_of_seg);

            PTRACE2(eLevelInfoNormal,"CComMode::Deserialize: segment:  \n",s);
            seg.DumpHex();

            while ( ! seg.EndOfSegment() )  {
                seg >> bas;
                HandleBas(bas,seg);
            }
            PTRACE(eLevelInfoNormal,"CComMode::Deserialize, Total segment is:");
            while ( ! copy2.EndOfSegment() )  {
                copy2 >> bas2;
                sprintf(s1," %02x",bas2);
                strcat(s,s1);
                counter++;
                if(!(counter%8)) strcat(s,"\n");
                if(counter > 100) break;
            }
            PTRACE2(eLevelInfoNormal,"CComMode::Deserialize: BAS DUMP:  \n",s);

            break;
        }
        case NATIVE     : {
            m_xferMode.DeSerialize(format,seg);
            m_audMode.DeSerialize(format,seg);
            m_vidMode.DeSerialize(format,seg);
            m_otherMode.DeSerialize(format,seg);
            m_lsdMlpMode.DeSerialize(format,seg);
            m_hsdHmlpMode.DeSerialize(format,seg);
            m_nsMode.DeSerialize(format,seg);
			m_contentMode.DeSerialize(format,seg);
			m_ECSMode.DeSerialize(format,seg);
            m_H221string.DeSerialize(format,seg);
            seg >> (BYTE&)m_bIsHd720Enabled;
            seg >> (BYTE&)m_bIsHd1080Enabled;
            seg >> (BYTE&)m_bIsHd720At60Enabled;
            seg >> (BYTE&)m_bShouldDisconnectOnEncryptionFailure;
            seg >> (BYTE&)m_bIsHd1080At60Enabled;
            break;
        }
        default : {
            break;
        }
    }
	DEALLOCBUFFER(s);
}

/////////////////////////////////////////////////////////////////////////////
WORD  CComMode::GetNumB0Chnl() const
{
    return m_xferMode.GetNumChnl() * m_xferMode.GetChnlWidth();
}

/////////////////////////////////////////////////////////////////////////////
WORD  CComMode::GetNumVideoChnl() const
{
    WORD numOccupiedChnl = 0;
    WORD hmlpMode;
    WORD mlpMode;
    WORD audMode;
    WORD transferRate;
    WORD contentChannels;
    WORD restrictMode=0;
    if(!HsdOn() && !LsdOn() && !MlpOn() && !HmlpOn() && !IsContentOn() )
        return GetNumB0Chnl();
        // hmlp
    if(HmlpOn()){
        hmlpMode = GetHmlpMode();
        switch(hmlpMode){
            case H_Mlp_Com_62_4:
            case H_Mlp_Com_64 :{
                numOccupiedChnl = 1;
                break;
            }
            case H_Mlp_Com_128:{
                numOccupiedChnl = 2;
                break;
            }
        }
    }
        // mlp
    if(MlpOn()){
        transferRate = GetXferMode();
        audMode = GetAudMode();
        mlpMode =  GetMlpMode();
        restrictMode = GetOtherRestrictMode() ;
        if(::IsT120Valid((BYTE)transferRate,(BYTE)audMode ,(BYTE)restrictMode,(BYTE)mlpMode))
            numOccupiedChnl = 1;
    }

	if( IsContentOn() ) {
		contentChannels = m_contentMode.GetNumberOfB0ChannelsByEpcContentRate(m_contentMode.GetContentRate());
		numOccupiedChnl += contentChannels;
	}

    return (GetNumB0Chnl() - numOccupiedChnl);
}



/////////////////////////////////////////////////////////////////////////////
void CComMode::HandleBas(BYTE bas,CSegment& seg)
{
    BYTE  basVal;
    BYTE  basAttr;

    basVal = bas & 0x1F;
    basAttr = bas & ( 0x7 << 5);

    switch ( basAttr ) {
        case AUDCMDATTR  :  {
            m_audMode.SetAudMode(basVal);
            break;
        }
        case XFERCMDATTR  :  {
            m_xferMode.SetXferMode(basVal);
            break;
        }
        case OTHRCMDATTR  :  {
            if ( basVal == Video_Off || basVal == H261 || basVal == H263 || basVal == H264)
                m_vidMode.SetVidMode(basVal);
            else
                m_otherMode.SetMode(basVal);
            break;
        }
        case LSDMLPCMDATTR  :  {
            m_lsdMlpMode.SetMode(basVal);
            break;
        }
        case ESCAPECAPATTR  :  {     // set hsd hmlp command

            // if Ns_Com getted
            if( bas == (ESCAPECAPATTR | Ns_Com) ) {
                HandleNsComBas(seg);
                break;
            }
            if( bas == (Data_Apps_Esc |  ESCAPECAPATTR) ) {
                seg >> bas;  //Table A4 : we don't use this information at CComMode.
                break;
            }
			if( bas == (H230_Esc | ESCAPECAPATTR) ) {
				HandleH230EscComBas(seg);
			}
            PTRACE(eLevelInfoNormal,"CComMode::HandleBas : Other ESC attribute was  detected!!!");
            //no action is taken if the last Bas in the stream is an escape-cap
            if (! seg.EndOfSegment()) {
                seg >> bas;
                basVal = bas & 0x1F;
                m_hsdHmlpMode.SetMode(basVal);
            }
            break;
        }
        default:  {
            PTRACE(eLevelError,"CComMode::HandleBas : unknown bas command!!!");
            break;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::HandleH230EscComBas(CSegment& seg)
{
	if( seg.EndOfSegment() ) {
		DBGPASSERT_AND_RETURN(101);
	}

	BYTE bas = 0;
	seg >> bas;
	if(bas == (Attr101 | AMC_open))
	{
		m_contentMode.SetContentModeFromH239(seg, AMC_open, GetNumB0Chnl());
	}
	else if(bas == (Attr101 | AMC_close))
	{
		m_contentMode.SetContentModeFromH239(seg, AMC_close, GetNumB0Chnl());
	}
	else
	{
		PTRACE(eLevelError,"CComMode::HandleH230EscComBas : unknown bas command!!!");
	}

}
/////////////////////////////////////////////////////////////////////////////
void CComMode::HandleNsComBas(CSegment& seg)
{
	if( seg.EndOfSegment() ) {
		DBGPASSERT_AND_RETURN(101);
	}

	BYTE	msgLen;
	BYTE	dataLen;
	BYTE    contentMsgLen = 0;
	BYTE	code1, code2;
	WORD	w_country /* country code */, w_manufacturer /* manufacturer code */;
	BYTE*	dump;
	int		iter, i;

	seg >> msgLen;
	dataLen = msgLen - NS_MSG_HEADER_LEN;
	if( msgLen == 0 ) {
		DBGPASSERT_AND_RETURN(102);
	}

	DBGPASSERT_AND_RETURN(msgLen < 5);

	if( seg.EndOfSegment() ) {
		DBGPASSERT_AND_RETURN(103);
	}

	seg >> code1;
	if( seg.EndOfSegment() ) {
		DBGPASSERT_AND_RETURN(104);
	}

	seg >> code2;
	w_country  = (code2 << 8) | code1;

	if( seg.EndOfSegment() ) {
		DBGPASSERT_AND_RETURN(105);
	}

	seg >> code1;
	if( seg.EndOfSegment() ) {
		DBGPASSERT_AND_RETURN(106);
	}

	seg >> code2;
	w_manufacturer  = (code2 << 8) | code1;

	dump = new BYTE [dataLen]; AUTO_DELETE_ARRAY(dump);
	for( iter=0; iter<dataLen; iter++ ) {
		if( seg.EndOfSegment() )
			break;
		seg >> dump[iter];
	}
	if( iter < dataLen ) { // part of Ns_Com msg not received
		ALLOCBUFFER(str,(dataLen*7+150));
		char	temp[16];
		sprintf( str, "\nmsgLen <%d>, country <0x%x>, manufact <0x%x>",
			msgLen, w_country, w_manufacturer );
		strcat(str,"\ndata bytes : ");
		for( i=0; i<iter; i++ ) {
			sprintf(temp," <0x%x>",dump[i]);
			strcat(str,temp);
		}
		PTRACE2(eLevelError,"CComMode::HandleNsComBas - illegal Ns_Com :",str);
		DEALLOCBUFFER(str);
	}
	for( i=0; i<iter; i++ ) {
		if( w_country == W_COUNTRY_CODE_USA ) { // USA country code
			if( w_manufacturer == W_MANUFACT_CODE_PICTURETEL ) { // PictureTel manufacturer code
				switch( dump[i] ) {
					case NS_COM_SIREN716_ON : {
						m_audMode.SetAudMode(Au_Siren7_16k);
						break;
					}
					case NS_COM_SIREN724_ON : {
						m_audMode.SetAudMode(Au_Siren7_24k);
						break;
					}
					case NS_COM_SIREN732_ON : {
						m_audMode.SetAudMode(Au_Siren7_32k);
						break;
					}
					case NS_COM_SIREN1424_ON : {
						m_audMode.SetAudMode(Au_Siren14_24k);
						break;
					}
					case NS_COM_SIREN1432_ON : {
						m_audMode.SetAudMode(Au_Siren14_32k);
						break;
					}
					case NS_COM_SIREN1448_ON : {
						m_audMode.SetAudMode(Au_Siren14_48k);
						break;
					}
					case NS_COM_CONTENT_ON : {// PeopleContentVersion0
						PTRACE(eLevelInfoNormal,"CComMode::HandleNsComBas - ERROR : PeopleContentsV0!");
						break;
					}
                    case NS_COM_H26L_ON : {
						m_vidMode.SetVidMode(H26L);
						break;
					}
				} // end switch
			} // end PictureTel manufacturer
			else if	(w_manufacturer ==W_MANUFACT_CODE_PUBLIC_PP){ // Public PP manufacturer code
				switch( dump[i] ) {
				case NS_COM_ROLE_LABEL: {
					if ( !(dump[dataLen-1] & RoleLabelTerminatorMask) )
					    PTRACE(eLevelError,"CComMode::HandleNsComBas - No terminator bit in the end of label list");//|EPC_TRACE
					for (BYTE j=i+1; j<dataLen; j++) {
						BYTE ClearTerminatorBitMask = (0xFF ^ RoleLabelTerminatorMask);
						BYTE currentLabel = (dump[j] & ClearTerminatorBitMask);
						switch (currentLabel) {
						case ContentLabel: {
						    PTRACE(eLevelInfoNormal,"CComMode::HandleNsComBas - Content Label");//|EPC_TRACE
							break;
										   }
						case PeopleLabel:  {
						    PTRACE(eLevelInfoNormal,"CComMode::HandleNsComBas - People Label");//|EPC_TRACE
							break;
										   }
						default: {
						    PTRACE(eLevelInfoNormal,"CComMode::HandleNsComBas - Unknown Label!!!");//|EPC_TRACE
							break;
								 }

						}
						i++;
					}
					break;
										}
				case NS_COM_AMSC_ON:
				case NS_COM_AMSC_OFF:{
					//according to EPC dump will be sent from i index
					m_contentMode.SetContentMode( (dump + i), dataLen, &contentMsgLen);
					// NS_COM_AMSC_ON takes 5 byte in the data,NS_COM_AMSC_OFF takes 2 byte in the data
					i = i + contentMsgLen;
					break;
									 }
				default:
					{
						DBGPASSERT_AND_RETURN(107);
						break;
					}
				}// end switch
			}// endPublic PP manufacturer code

		} // end USA country
		if( w_country == W_COUNTRY_CODE_ISRAEL ) { // ISRAEL country code - Internal Use Only
			if( w_manufacturer == W_MANUFACT_CODE_POLYCOM ) { // Polycom manufacturer code
				switch( dump[i] ) {
					case NS_COM_G7222_0660 : {
						m_audMode.SetAudMode(G7222_0660);
						break;
					}
					case NS_COM_G7222_0885 : {
						m_audMode.SetAudMode(G7222_0885);
						break;
					}
					case NS_COM_G7222_1265 : {
						m_audMode.SetAudMode(G7222_1265);
						break;
					}
					case NS_COM_G7222_1425 : {
						m_audMode.SetAudMode(G7222_1425);
						break;
					}
					case NS_COM_G7222_1585 : {
						m_audMode.SetAudMode(G7222_1585);
						break;
					}
					case NS_COM_G7222_1825 : {
						m_audMode.SetAudMode(G7222_1825);
						break;
					}
					case NS_COM_G7222_1985 : {
						m_audMode.SetAudMode(G7222_1985);
						break;
					}
					case NS_COM_G7222_2305: {
						m_audMode.SetAudMode(G7222_2305);
						break;
					}
					case NS_COM_G7222_2385 : {
						m_audMode.SetAudMode(G7222_2385);
						break;
					}
				} // end switch
			} // end PictureTel manufacturer
		} //end Israel
	}
}
///////////////////////////////////////////////////////////////////////////
void CComMode::GetNextNSCommand(CSegment& seg)

{/*
	if( seg.EndOfSegment() )
		DBGPASSERT_AND_RETURN(111);
	BYTE	msgLen;
	BYTE    contentMsgLen = 0;
	BYTE	code1, code2;
	WORD	w_country;
	BYTE*	dump;
	int		iter, i;
	BYTE code;
				//get next command

	seg >> code;
	if(code == (BYTE)(ESCAPECAPATTR | Ns_Com) ) {

		seg >> msgLen;
		if( msgLen == 0 )
			DBGPASSERT_AND_RETURN(112);
		DBGPASSERT_AND_RETURN(msgLen < 5);

		if( seg.EndOfSegment() )
			DBGPASSERT_AND_RETURN(113);
		seg >> code1;
		if( seg.EndOfSegment() )
			DBGPASSERT_AND_RETURN(114);
		seg >> code2;
		w_country  = (code2 << 8) | code1;

		if( seg.EndOfSegment() )
			DBGPASSERT_AND_RETURN(115);
		seg >> code1;
		if( seg.EndOfSegment() )
			DBGPASSERT_AND_RETURN(116);
		seg >> code2;
		w_manufacturer  = (code2 << 8) | code1;

		dump = new BYTE [msgLen - 4];
		for(  iter=0; iter<msgLen-4; iter++ ) {
			if( seg.EndOfSegment() )
				break;
			seg >> dump[iter];
		}
		if( iter < msgLen-4 ) { // part of Ns_Com msg not received
			char	str[256];
			char	temp[16];
			sprintf( str, "\nmsgLen <%d>, country <0x%x>, manufact <0x%x>",
				msgLen, w_country, w_manufacturer );
			strcat(str,"\ndata bytes : ");
			for( i=0; i<iter; i++ ) {
				sprintf(temp," <0x%x>",dump[i]);
				strcat(str,temp);
			}
			PTRACE2(eLevelError,"CComMode::GetNextNSCommand - illegal Ns_Com :",str);
		}
		//for( i=0; i<iter; i++ )
		i = 0;
		if( w_country == W_COUNTRY_CODE_USA &&
			w_manufacturer ==W_MANUFACT_CODE_PUBLIC_PP) { // USA country code
			switch( dump[i] ) {
			case NS_COM_AMSC_ON:
			case NS_COM_AMSC_OFF:{

				m_contentMode.SetNsContentMode( dump , (msgLen - 4 ), &contentMsgLen);
				// NS_COM_AMSC_ON takes 5 byte in the data,NS_COM_AMSC_OFF takes 2 byte in the data
				//	i = i +contentMsgLen;
				break;
								 }
			default:
				{
					DBGPASSERT_AND_RETURN(117);
					break;
				}
			}//switch
		}//if USA country code
		else{
			PTRACE(eLevelError,"CComMode::CheckNextCommand - illegal Country/Manuf code :");
			DBGPASSERT_AND_RETURN(118);

		}
	}//if  (ESCAPECAPATTR | Ns_Com)
	else
	{
		 PTRACE(eLevelError,"CComMode::CheckNextCommand - illegal Com :");
		 DBGPASSERT_AND_RETURN(119);
	}*/
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::Serialize(WORD format,CSegment& seg, BYTE isH239)
{
	switch ( format )  {

        case SERIALEMBD : {
		m_xferMode.Serialize(format,seg);
		m_audMode.Serialize(format,seg);
		m_vidMode.Serialize(format,seg);
		m_otherMode.Serialize(format,seg);
		m_lsdMlpMode.Serialize(format,seg);
		m_hsdHmlpMode.Serialize(format,seg);
		m_nsMode.Serialize(format,seg);
		m_contentMode.Serialize(format,seg, isH239);
		break;
						  }
		default :{
		m_xferMode.Serialize(format,seg);
		m_audMode.Serialize(format,seg);
		m_vidMode.Serialize(format,seg);
		m_otherMode.Serialize(format,seg);
		m_lsdMlpMode.Serialize(format,seg);
		m_hsdHmlpMode.Serialize(format,seg);
		m_nsMode.Serialize(format,seg);
		m_contentMode.Serialize(format,seg);
		m_ECSMode.Serialize(format,seg);
		m_H221string.Serialize(format,seg);
		seg << m_bIsHd720Enabled;
		seg << m_bIsHd1080Enabled;
		seg << m_bIsHd720At60Enabled;
		seg << m_bShouldDisconnectOnEncryptionFailure;
		seg << m_bIsHd1080At60Enabled;

		break;
				 }
	}
}

/////////////////////////////////////////////////////////////////////////////
WORD    CComMode::operator<= (const CCapH320& cap) const
{
    WORD    rval = 0;

    if ( m_vidMode.IsFreeBitRate() )
        return 1;

    if( m_audMode     <= cap &&
        m_vidMode     <= cap &&
        m_xferMode    <= cap &&
        m_hsdHmlpMode <= cap &&
        m_lsdMlpMode  <= cap &&
        m_otherMode   <= cap )
            rval = 1;
    return rval;
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::operator=(const CComMode& other)
{
	if ( &other == this ) return;

    m_audMode       = other.m_audMode;
    m_vidMode       = other.m_vidMode;
    m_xferMode      = other.m_xferMode;
    m_hsdHmlpMode   = other.m_hsdHmlpMode;
    m_lsdMlpMode    = other.m_lsdMlpMode;
    m_otherMode     = other.m_otherMode;
    m_H221string    = other.m_H221string;
    m_nsMode        = other.m_nsMode;
    m_contentMode   = other.m_contentMode;
	m_ECSMode		= other.m_ECSMode;

	m_bIsHd720Enabled = other.m_bIsHd720Enabled;
	m_bIsHd1080Enabled = other.m_bIsHd1080Enabled;
	m_bIsHd720At60Enabled = other.m_bIsHd720At60Enabled;
	m_bShouldDisconnectOnEncryptionFailure = other.m_bShouldDisconnectOnEncryptionFailure;
	m_bIsHd1080At60Enabled = other.m_bIsHd1080At60Enabled;
}

/////////////////////////////////////////////////////////////////////////////
WORD  operator==(const CComMode& rComMode_1,const CComMode& rComMode_2)
{
    WORD    rval = 0;

    if ( rComMode_1.m_audMode     == rComMode_2.m_audMode     &&
         rComMode_1.m_vidMode     == rComMode_2.m_vidMode     &&
         rComMode_1.m_xferMode    == rComMode_2.m_xferMode    &&
         rComMode_1.m_hsdHmlpMode == rComMode_2.m_hsdHmlpMode &&
         rComMode_1.m_lsdMlpMode  == rComMode_2.m_lsdMlpMode  &&
         rComMode_1.m_otherMode   == rComMode_2.m_otherMode   &&
         rComMode_1.m_nsMode      == rComMode_2.m_nsMode &&
         rComMode_1.m_contentMode == rComMode_2.m_contentMode)
        rval = 1;

    return rval;
}
/////////////////////////////////////////////////////////////////////////////
WORD CComMode::IsDifferInXferOnly(const CComMode& rComMode_2) const
{
    WORD    rval = 0;

    if ( m_audMode     == rComMode_2.m_audMode     &&
         m_vidMode     == rComMode_2.m_vidMode     &&
         !(m_xferMode    == rComMode_2.m_xferMode)    &&
         m_hsdHmlpMode == rComMode_2.m_hsdHmlpMode &&
         m_lsdMlpMode  == rComMode_2.m_lsdMlpMode  &&
         m_otherMode   == rComMode_2.m_otherMode   &&
         m_nsMode      == rComMode_2.m_nsMode &&
         m_contentMode == rComMode_2.m_contentMode)
        rval = 1;

    return rval;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CComMode::IsDifferInContentRateOnly(const CComMode& rComMode_2) const
{
	BYTE	rval = 0;
	if (m_audMode == rComMode_2.m_audMode &&
		m_vidMode == rComMode_2.m_vidMode &&
		m_xferMode == rComMode_2.m_xferMode &&
		m_hsdHmlpMode == rComMode_2.m_hsdHmlpMode &&
		m_lsdMlpMode == rComMode_2.m_lsdMlpMode &&
		m_otherMode == rComMode_2.m_otherMode &&
		m_nsMode == rComMode_2.m_nsMode &&
		m_contentMode.GetOpcode() == rComMode_2.m_contentMode.GetOpcode() &&
		m_contentMode.GetContentRate() != rComMode_2.m_contentMode.GetContentRate() )
		rval = 1;

	return  rval;

}

/////////////////////////////////////////////////////////////////////////////
WORD CComMode::IsClosureOfVideoOnly(const CComMode& rComMode_2) const
{
    WORD    rval = 0;
    // There are two options for change mode:
    // a) e.p. opened video and MCU accepted it but now wants to close it.
    // b) No change mode in any media is needed, but a request for change mode arrives
    // with video Off in both target/current communication mode. This is an indication
    // that MCMS should ask the e.p. to close its video because it opened a different video from
    // MCMS request.
    if ( m_audMode      == rComMode_2.m_audMode     &&
       ((m_vidMode.GetVidMode() == Video_Off  &&  rComMode_2.m_vidMode.GetVidMode() != Video_Off ) ||
        ( m_vidMode.GetVidMode() == Video_Off &&  rComMode_2.m_vidMode.GetVidMode() == Video_Off  ))  &&
        m_xferMode      == rComMode_2.m_xferMode    &&
        m_hsdHmlpMode   == rComMode_2.m_hsdHmlpMode &&
        m_lsdMlpMode    == rComMode_2.m_lsdMlpMode  &&
        m_otherMode     == rComMode_2.m_otherMode  &&
        m_contentMode   == rComMode_2.m_contentMode )
       rval = 1;

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
WORD CComMode::IsClosureOfVideoAndContentOnly(const CComMode& rComMode_2) const
{
    WORD    rval = 0;
    // Change to Secondary because content change mode failed - we are closing both video and content
    if ( m_audMode      == rComMode_2.m_audMode     &&
       ((m_vidMode.GetVidMode() == Video_Off  &&  rComMode_2.m_vidMode.GetVidMode() != Video_Off ) ||
        ( m_vidMode.GetVidMode() == Video_Off &&  rComMode_2.m_vidMode.GetVidMode() == Video_Off  ))  &&
        m_xferMode      == rComMode_2.m_xferMode    &&
        m_hsdHmlpMode   == rComMode_2.m_hsdHmlpMode &&
        m_lsdMlpMode    == rComMode_2.m_lsdMlpMode  &&
        m_otherMode     == rComMode_2.m_otherMode  &&
        (m_contentMode.GetOpcode()==NS_COM_AMSC_OFF && rComMode_2.m_contentMode.GetOpcode()==NS_COM_AMSC_ON))
       rval = 1;

    return rval;
}
/////////////////////////////////////////////////////////////////////////////
WORD CComMode::IsAsymmetricVideoProtocolOnly(const CComMode& rComMode_2) const
{
    WORD    rval = 0;

	if ( m_audMode      == rComMode_2.m_audMode     &&
       ((m_vidMode.GetVidMode() == H263  &&  rComMode_2.m_vidMode.GetVidMode() == H261 )||
        ( m_vidMode.GetVidMode() == H263 &&  rComMode_2.m_vidMode.GetVidMode() == H264 )||
        ( m_vidMode.GetVidMode() == H261 &&  rComMode_2.m_vidMode.GetVidMode() == H263 )||
        ( m_vidMode.GetVidMode() == H261 &&  rComMode_2.m_vidMode.GetVidMode() == H264 )||
        ( m_vidMode.GetVidMode() == H264 &&  rComMode_2.m_vidMode.GetVidMode() == H261 )||
        ( m_vidMode.GetVidMode() == H264 &&  rComMode_2.m_vidMode.GetVidMode() == H263  ))  &&
        m_xferMode      == rComMode_2.m_xferMode    &&
        m_hsdHmlpMode   == rComMode_2.m_hsdHmlpMode &&
        m_lsdMlpMode    == rComMode_2.m_lsdMlpMode  &&
        m_otherMode     == rComMode_2.m_otherMode  &&
        m_contentMode   == rComMode_2.m_contentMode )
       rval = 1;

    return rval;
}
/////////////////////////////////////////////////////////////////////////////
WORD CComMode::IsAsymmetricContentModeOnly(const CComMode& rComMode_2) const
{
    WORD    rval = 0;

	if ( m_audMode      == rComMode_2.m_audMode     &&
		 m_vidMode      == rComMode_2.m_vidMode     &&
        m_xferMode      == rComMode_2.m_xferMode    &&
        m_hsdHmlpMode   == rComMode_2.m_hsdHmlpMode &&
        m_lsdMlpMode    == rComMode_2.m_lsdMlpMode  &&
        m_otherMode     == rComMode_2.m_otherMode  &&
		m_contentMode.IsAssymetricContentMode(rComMode_2.m_contentMode))

       rval = 1;

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
WORD  operator<=(const CComMode& rComMode_1,const CComMode& rComMode_2)
{
    WORD    rval = 0;
    WORD    rvid_1,rvid_2;

    rvid_1 = rComMode_1.m_vidMode.GetVidMode();
    rvid_2 = rComMode_2.m_vidMode.GetVidMode();
    if ( rComMode_1.m_audMode      == rComMode_2.m_audMode     &&
         //rComMode_1.m_vidMode.GetVidMode() <= rComMode_2.m_vidMode.GetVidMode()   &&
         rComMode_1.m_xferMode     == rComMode_2.m_xferMode    &&
         rComMode_1.m_hsdHmlpMode  == rComMode_2.m_hsdHmlpMode &&
         rComMode_1.m_lsdMlpMode   == rComMode_2.m_lsdMlpMode  &&
         rComMode_1.m_otherMode    == rComMode_2.m_otherMode   &&
         rComMode_1.m_contentMode  == rComMode_2.m_contentMode  )
        rval = 1;

    // Operator 'CComMode <= CComMode' is used in only one place:
    // in function CMuxCntl::OnMuxRmtXmitModeChangeMode.
    // In this function, in case when target Scm is set to H263 video, we want
    // to wait for one of the events:
    //      1. Time-out for H263 video connection in ChangeModeCntl
    //           (in this case we try to connect with H261 video)
    //      2. Remote opened H263 video.
    // Therefore, when target Scm is H263 and remote Scm is H261 (none of the 2 event
    // is happened) operator 'CComMode <= CComMode' should return FALSE in order to
    // prevent ending of change mode in function CMuxCntl::OnMuxRmtXmitModeChangeMode.
    if ( (rvid_1 == H264 && rvid_2 == H263)||( rvid_1 == H264 && rvid_2 == H261)||( rvid_1 == H263 && rvid_2 == H261) )
        rval = 0;

    // In case the TargetComMode was with Video_on and the RmtXmitComMode
    // was with Video_off return FALSE in order to prevent ending of change mode.
    if ( rvid_1 != Video_Off && rvid_2 == Video_Off)
        rval = 0;

	if ( rvid_1 == H26L && (rvid_2 != H26L) )
        rval = 0;

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
void  CComMode::UpdateH221string(BYTE isH239)
{
    CSegment seg;

    //serialize ccommode in H221 format on seg
    Serialize(SERIALEMBD,seg,isH239);
    //deserialize the H221 string from the seg
    m_H221string.DeSerialize(SERIALEMBD,seg);
}

/////////////////////////////////////////////////////////////////////////////
BYTE *CComMode::GetH221str() const
{
    return m_H221string.GetPtr();
}

/////////////////////////////////////////////////////////////////////////////
WORD CComMode::GetH221StrLen() const
{
    return m_H221string.GetLen();
}
/////////////////////////////////////////////////////////////////////////////
eVideoPartyType CComMode::GetCPResourceVideoPartyType()
{
	eVideoPartyType videoPartyType = m_vidMode.GetCPResourceVideoPartyType();
	return videoPartyType;
}

/////////////////////////////////////////////////////////////////////////////
CAudMode::CAudMode()          // constructor
{
//    m_audMode = A_Law_OF;    // can be also U_Law_OF . see h242 (3/93) pg 23 tab/2
    m_audMode = 0;
	m_bitRate = 0;
}

/////////////////////////////////////////////////////////////////////////////
CAudMode::~CAudMode()        // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
const char*   CAudMode::NameOf()  const
{
    return "CAudMode";
}

/////////////////////////////////////////////////////////////////////////////
void  CAudMode::SetAudMode(WORD audMode)
{
    m_audMode  = audMode;
}

/////////////////////////////////////////////////////////////////////////////
void  CAudMode::SetBitRate(WORD audMode)
{
    switch ( audMode ) {

        case G729_8k  :  {
            m_bitRate = 8;
            m_audMode = G729_8k;
            break;
        }
        case G723_1_Command:  {  // voice call
            m_bitRate = 8;
            m_audMode = G723_1_Command;
            break;
        }
        case G728  :  {
            m_bitRate = 16;
            m_audMode = G728;
            break;
        }
        // G722.1/24 support
        case Au_24k    :  {  // G.722.1/24
            m_bitRate = 24;
            m_audMode = Au_24k;
            break;
        }
        // G722.1/32 supportPTRACE(eLevelInfoNormal, (char*)msg.str().c_str());
        case Au_32k    :  {  // G.722.1/32
            m_bitRate = 32;
            m_audMode = Au_32k;
            break;
        }

		case A_Law_48  :  {  // g.711 only
            m_bitRate = 48;
			m_audMode = A_Law_48;
            break;
        }

        case U_Law_48  :  {  // g.711 only
            m_bitRate = 48;
			m_audMode = U_Law_48;
            break;
        }

        case A_Law_OF  :  {  // g.711 only
            m_bitRate = 56;
			m_audMode = A_Law_OF;
            break;
        }

        case U_Law_OF  :  {  // g.711 only
            m_bitRate = 56;
			m_audMode = U_Law_OF;
            break;
        }

        case G722_m1  :  {  // g.711 || g.722
            m_bitRate = 64;
            m_audMode = G722_m1;
            break;
        }

        case G722_m2  :  {  // g.711 || g.722
            m_bitRate = 56;
            m_audMode = G722_m2;
            break;
        }

        case G722_m3  :  {  // g.711 || g.722
            m_bitRate = 48;
            m_audMode = G722_m3;
            break;
        }

        case A_Law_OU:  {  // voice call
            m_bitRate = 64;
			m_audMode = A_Law_OU;
            break;
        }

		case U_Law_OU:  {  // voice call
            m_bitRate = 64;
			m_audMode = U_Law_OU;
            break;
        }

        // Siren716 support
        case Au_Siren7_16k  :  {  // Siren716
            m_bitRate = 16;
            m_audMode = Au_Siren7_16k;
            break;
        }
        // Siren724 support
        case Au_Siren7_24k  :  {  // Siren724
            m_bitRate = 24;
            m_audMode = Au_Siren7_24k;
            break;
        }
        // Siren732 support
        case Au_Siren7_32k  :  {  // Siren732
            m_bitRate = 32;
            m_audMode = Au_Siren7_32k;
            break;
        }
        // Siren1424 support
        case Au_Siren14_24k  :  {  // Siren1424
            m_bitRate = 24;
            m_audMode = Au_Siren14_24k;
            break;
        }
        // Siren1432 support
        case Au_Siren14_32k  :  {  // Siren1432
            m_bitRate = 32;
            m_audMode = Au_Siren14_32k;
            break;
        }
        // Siren1448 support
        case Au_Siren14_48k  :  {  // Siren1448
            m_bitRate = 48;
            m_audMode = Au_Siren14_48k;
            break;
        }
		//G722.1 AnnexC support
	    case G7221_AnnexC_48k:  {
            m_bitRate = 48;
            m_audMode = G7221_AnnexC_48k;
            break;
        }
	    case G7221_AnnexC_32k:  {
            m_bitRate = 32;
            m_audMode = G7221_AnnexC_32k;
            break;
        }
	    case G7221_AnnexC_24k:  {
		    m_bitRate = 24;
			m_audMode = G7221_AnnexC_24k;
			break;
        }
        case 0 :
        case 255  :  {    // auto
            m_bitRate = 0;
            break;
        }

        default:  {
            std::ostringstream  msg;
            msg << "CAudMode::SetBitRate :unknown audio algorithm = " << audMode;
            PTRACE(eLevelError, (char*)msg.str().c_str());
			break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
WORD  CAudMode::GetAudMode() const
{
    return m_audMode;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CAudMode::GetBitRate() const
{
    return m_bitRate;
}

/////////////////////////////////////////////////////////////////////////////
void  CAudMode::DeSerialize(WORD format,CSegment& seg)
{
    switch ( format )  {
        case SERIALEMBD : {
            break;
        }
        case NATIVE     : {
            seg >> m_audMode >> m_bitRate;
            break;
        }
        default :{
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CAudMode::Serialize(WORD format,CSegment& seg)
{
    BYTE    audCmd = AUDCMDATTR;

    switch ( format )  {

        case SERIALEMBD : {
            if( m_audMode <= Au_Off_F ) {
                audCmd |= m_audMode;
                seg << audCmd;
            } else { // non-standard audio algorithm
                switch( m_audMode ) {
                    case Au_Siren7_16k : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_PICTURETEL & 0xFF )         // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_PICTURETEL >> 8) & 0xFF ); // manufacturer code senior byte
                        seg << (BYTE) NS_COM_SIREN716_ON;                          // Siren7 16 kbps audio algorithm ON
                        break;
                    }
                    case Au_Siren7_24k : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_PICTURETEL & 0xFF )         // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_PICTURETEL >> 8) & 0xFF ); // manufacturer code senior byte
                        seg << (BYTE) NS_COM_SIREN724_ON;                          // Siren7 24 kbps audio algorithm ON
                        break;
                    }
                    case Au_Siren7_32k : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_PICTURETEL & 0xFF )         // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_PICTURETEL >> 8) & 0xFF ); // manufacturer code senior byte
                        seg << (BYTE) NS_COM_SIREN732_ON;                          // Siren7 32 kbps audio algorithm ON
                        break;
                    }
                    case Au_Siren14_24k : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_PICTURETEL & 0xFF )         // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_PICTURETEL >> 8) & 0xFF ); // manufacturer code senior byte
                        seg << (BYTE) NS_COM_SIREN1424_ON;                         // Siren14 24 kbps audio algorithm ON
                        break;
                    }
                    case Au_Siren14_32k : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_PICTURETEL & 0xFF )         // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_PICTURETEL >> 8) & 0xFF ); // manufacturer code senior byte
                        seg << (BYTE) NS_COM_SIREN1432_ON;                         // Siren14 32 kbps audio algorithm ON
                        break;
                    }
                    case Au_Siren14_48k : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_PICTURETEL & 0xFF )         // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_PICTURETEL >> 8) & 0xFF ); // manufacturer code senior byte
                        seg << (BYTE) NS_COM_SIREN1448_ON;                         // Siren14 48 kbps audio algorithm ON
                        break;
                    }
					case G7222_0660 :{
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_ISRAEL & 0xFF )              // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_ISRAEL >> 8) & 0xFF );      // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_POLYCOM & 0xFF )			   // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_POLYCOM >> 8) & 0xFF );	   // manufacturer code senior byte
                        seg << (BYTE) NS_COM_G7222_0660;						   // VTX Audio Alg
                        break;
                    }
					case G7222_0885 : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_ISRAEL & 0xFF )              // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_ISRAEL >> 8) & 0xFF );      // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_POLYCOM & 0xFF )			   // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_POLYCOM >> 8) & 0xFF );	   // manufacturer code senior byte
                        seg << (BYTE) NS_COM_G7222_0885;						   // VTX Audio Alg
                        break;
                    }
					case G7222_1265 : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_ISRAEL & 0xFF )              // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_ISRAEL >> 8) & 0xFF );      // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_POLYCOM & 0xFF )			   // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_POLYCOM >> 8) & 0xFF );	   // manufacturer code senior byte
                        seg << (BYTE) NS_COM_G7222_1265;						   // VTX Audio Alg
                        break;
                    }
					case G7222_1425 : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_ISRAEL & 0xFF )              // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_ISRAEL >> 8) & 0xFF );      // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_POLYCOM & 0xFF )			   // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_POLYCOM >> 8) & 0xFF );	   // manufacturer code senior byte
                        seg << (BYTE) NS_COM_G7222_1425;					  	   // VTX Audio Alg
                        break;
                    }
					case G7222_1585 : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_ISRAEL & 0xFF )              // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_ISRAEL >> 8) & 0xFF );      // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_POLYCOM & 0xFF )			   // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_POLYCOM >> 8) & 0xFF );	   // manufacturer code senior byte
                        seg << (BYTE) NS_COM_G7222_1585;						   // VTX Audio Alg
                        break;
                    }
					case G7222_1825 : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_ISRAEL & 0xFF )              // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_ISRAEL >> 8) & 0xFF );      // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_POLYCOM & 0xFF )			   // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_POLYCOM >> 8) & 0xFF );	   // manufacturer code senior byte
                        seg << (BYTE) NS_COM_G7222_1825;						   // VTX Audio Alg
                        break;
                    }
					case G7222_1985 : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_ISRAEL & 0xFF )              // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_ISRAEL >> 8) & 0xFF );      // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_POLYCOM & 0xFF )			   // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_POLYCOM >> 8) & 0xFF );	   // manufacturer code senior byte
                        seg << (BYTE) NS_COM_G7222_1985;						   // VTX Audio Alg
                        break;
                    }
					case G7222_2305 : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_ISRAEL & 0xFF )              // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_ISRAEL >> 8) & 0xFF );      // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_POLYCOM & 0xFF )			   // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_POLYCOM >> 8) & 0xFF );	   // manufacturer code senior byte
                        seg << (BYTE) NS_COM_G7222_2305;						   // VTX Audio Alg
                        break;
                    }
					case G7222_2385 : {
                        seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                        seg << (BYTE) 5;                                           // msgLen
                        seg << (BYTE)( W_COUNTRY_CODE_ISRAEL & 0xFF )              // country code junior byte
                            << (BYTE)( (W_COUNTRY_CODE_ISRAEL >> 8) & 0xFF );      // country code senior byte
                        seg << (BYTE)( W_MANUFACT_CODE_POLYCOM & 0xFF )			   // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_POLYCOM >> 8) & 0xFF );	   // manufacturer code senior byte
                        seg << (BYTE) NS_COM_G7222_2385;						   // VTX Audio Alg
                        break;
                    }
                    default : {
                        break;
                    }
                }
            }
            break;
        }
        case NATIVE     : {
            seg << m_audMode << m_bitRate;
            break;
        }
        default :{
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
WORD CAudMode::operator<=(const CCapH320& cap) const
{
    WORD    rval = 0;

    switch ( m_audMode ) {

        case A_Law_OF  :
        case A_Law_48  :{
            rval = cap.OnAudioCap(e_A_Law);
            break;
        }
        case U_Law_OF  :
        case U_Law_48  : {
            rval = cap.OnAudioCap(e_U_Law);
            break;
        }
        case G722_m1   :  {
            rval = cap.OnAudioCap(e_G722_64);
            break;
        }
        case G722_m2   :  {
            rval = cap.OnAudioCap(e_G722_48);
            break;
        }
        case G722_m3   :  {
            rval = cap.OnAudioCap(e_G722_48);
            break;
        }
        case Au_32k    :  {
		    rval = cap.OnAudioCap(e_G722_1_32);//IsOtherCapSupported(G722_1_32);
            break;
        }
        case Au_24k    :  {
		    rval = cap.OnAudioCap(e_G722_1_24); //IsOtherCapSupported(G722_1_24);
            break;
        }
//         case G729_8k	:  { //IP only
//             rval = cap.OnAudioCap(e_G729);
//             break;
//         }
        case G728      :  {
            rval = cap.OnAudioCap(e_Au_16k);
            break;
        }
//         case G723_1_Command  :  { //IP only
// 		  rval = cap.IsOtherCapSupported(G723_1);
//             break;
//         }
        // non-standard audio cap start
        case Au_Siren7_16k : {
            rval = cap.GetNSCap()->OnSiren716();
            break;
        }
        case Au_Siren7_24k : {
            rval = cap.GetNSCap()->OnSiren724();
            break;
        }
        case Au_Siren7_32k : {
            rval = cap.GetNSCap()->OnSiren732();
            break;
        }
        case Au_Siren14_24k : {
            rval = cap.GetNSCap()->OnSiren1424();
            break;
        }
        case Au_Siren14_32k : {
            rval = cap.GetNSCap()->OnSiren1432();
            break;
        }
        case Au_Siren14_48k : {
            rval = cap.GetNSCap()->OnSiren1448();
            break;
        }
        // non-standard audio cap end
        default        :  {
            // automatic audio selection
            rval = 1;
            break;
        }
    }
    return rval;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CAudMode::GetBitRateClass() const
{
    //this function returns the audio bitrate class as follow:
    WORD bitrate = 0;

    switch (m_audMode) {
        case G729_8k       :{
            bitrate = 8;
            break;
        }
        case G723_1_Command : {
            bitrate = 8;
            break;
        }
        case G728          :
        case Au_Siren7_16k : {
            bitrate = 16;
            break;
        }
        case G7221_AnnexC_24k :
        case Au_24k         :
        case Au_Siren14_24k :
        case Au_Siren7_24k  : {
            bitrate = 24;
            break;
        }
        case G7221_AnnexC_32k :
        case Au_32k         :
        case Au_Siren14_32k :
        case Au_Siren7_32k  : {
            bitrate = 32;
            break;
        }
        case G722_m3        :
        case G7221_AnnexC_48k :
        case Au_Siren14_48k :
        case A_Law_48       :
        case U_Law_48       : {
            bitrate = 48;
            break;
        }
        case G722_m2  :
        case A_Law_OF :
        case U_Law_OF : {
            bitrate = 56;
            break;
        }
        case A_Law_OU :
        case U_Law_OU : {
            bitrate = 64;
            break;
        }
        case Au_Neutral :
        case Au_Off_U   :
        case Au_Off_F   : {
            bitrate = 0; //no audio algorithem has been chosen yet
            PTRACE(eLevelInfoNormal,"CAudMode::GetBitRateClass : audio is off");
            break;
        }
        default : {
            PTRACE(eLevelError,"CAudMode::GetBitRateClass : unknown audio algorithm");
            break;
        }
    }
    return bitrate;
}

/////////////////////////////////////////////////////////////////////////////
WORD CAudMode::IsAudioOpen() const
{
    WORD rc = 0;
    //this is the sub-set of the H221 audio modes
    //that we currently support
    switch(m_audMode) {
        case A_Law_OU :
        case U_Law_OU :
        case G722_m1  :
        case A_Law_OF :
        case U_Law_OF :
        case A_Law_48 :
        case U_Law_48 :
        case G722_m2  :
        case G722_m3  :
        case G723_1_Command :
        case Au_24k   : // G.722.1/24
        case Au_32k   : // G.722.1/32
        case Au_Siren7_16k :
        case Au_Siren7_24k :
        case Au_Siren7_32k :
        case Au_Siren14_24k :
        case Au_Siren14_32k :
        case Au_Siren14_48k :
        case G7221_AnnexC_48k :
        case G7221_AnnexC_32k :
        case G7221_AnnexC_24k :
        case G728           :
        case G729_8k        :{
            rc =1;
            break;
        }
    }
    return (rc);
}

/////////////////////////////////////////////////////////////////////////////
WORD  operator==(const CAudMode& rAud_1,const CAudMode& rAud_2)
{
    WORD    rval = 0;

    if ( rAud_1.m_audMode == rAud_2.m_audMode )
        rval = 1;

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
CXferMode::CXferMode()                // constructor
{
    m_xferMode = 0xff;//Xfer_64;
}

/////////////////////////////////////////////////////////////////////////////
CXferMode::~CXferMode()        // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
const char*   CXferMode::NameOf()  const
{
    return "CXferMode";
}

/////////////////////////////////////////////////////////////////////////////
void  CXferMode::SetXferMode(WORD mode)
{
    m_xferMode = mode;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CXferMode::GetXferMode() const
{
    return m_xferMode;
}

/////////////////////////////////////////////////////////////////////////////
void  CXferMode::Zero()
{
    switch ( m_xferMode ) {

        case Xfer_64    :
        case Xfer_2x64  :
        case Xfer_3x64  :
        case Xfer_4x64  :
        case Xfer_5x64  :
        case Xfer_6x64  :  {
            m_xferMode = Xfer_64;
            break;
        }
        case Xfer_128   :  {
            m_xferMode = Xfer_128;
            break;
        }
        case Xfer_384   :
        case Xfer_2x384 :
        case Xfer_3x384 :
        case Xfer_4x384 :
        case Xfer_5x384 :  {
            m_xferMode = Xfer_384;
            break;
        }

        default         :  {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
WORD  CXferMode::GetNumChnl() const
{
    WORD   numChnl = 0;

    switch ( m_xferMode ) {

        case Xfer_2x64  :
        case Xfer_2x384 :  { numChnl = 2; break; }

        case Xfer_3x64  :
        case Xfer_3x384 :  { numChnl = 3; break; }

        case Xfer_4x64  :
        case Xfer_4x384 :  { numChnl = 4; break; }

        case Xfer_5x64  :
        case Xfer_5x384 :  { numChnl = 5; break; }

        case Xfer_6x64  :  { numChnl = 6; break; }

        default         :  { numChnl = 1; break; }
    }
    return numChnl;
}

WORD CXferMode::GetXferModeByNumChannels(WORD numChnl)
{
    WORD xferMode = 0;
    switch ( numChnl )
	  {
	  case 2   :{ xferMode=Xfer_128; break;}
	  case 3   :{ xferMode=Xfer_192; break;}
	  case 4   :{ xferMode=Xfer_256; break;}
	  case 5   :{ xferMode=Xfer_320; break;}
	  case 6   :{ xferMode=Xfer_384; break;}
	  case 8   :{ xferMode=Xfer_512; break;}
	  case 12  :{ xferMode=Xfer_768; break;}
	  case 18  :{ xferMode=Xfer_1152; break;}
	  case 23  :{ xferMode=Xfer_1472; break;}
	  case 24  :{ xferMode=Xfer_1536; break;}
	  case 30  :{ xferMode=Xfer_1920; break;}
    }
	return xferMode;
}
/////////////////////////////////////////////////////////////////////////////
WORD  CXferMode::GetChnlWidth() const
{
    WORD    chnlWidth = B0;

    switch ( m_xferMode ) {

        case Xfer_64      :
        case Xfer_2x64    :
        case Xfer_3x64    :
        case Xfer_4x64    :
        case Xfer_5x64    :
        case Xfer_6x64    : { chnlWidth = B0;      break; }

        case Xfer_384     :
        case Xfer_2x384   :
        case Xfer_3x384   :
        case Xfer_4x384   :
        case Xfer_5x384   : { chnlWidth = H0;      break; }

        case Xfer_128     : { chnlWidth = B0 * 2;  break; }
        case Xfer_192     : { chnlWidth = B0 * 3;  break; }
        case Xfer_256     : { chnlWidth = B0 * 4;  break; }
        case Xfer_320     : { chnlWidth = B0 * 5;  break; }

        case Xfer_512     : { chnlWidth = B0 * 8;  break; }
        case Xfer_768     : { chnlWidth = B0 * 12; break; }
        case Xfer_1152    : { chnlWidth = B0 * 18; break; }
        case Xfer_1472    : { chnlWidth = B0 * 23; break; }
        case Xfer_1536    : { chnlWidth = B0 * 24; break; }
        case Xfer_1920    : { chnlWidth = B0 * 30; break; }

        default           : { break; }
    }
    return chnlWidth;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CXferMode::GetH221NetChnlWidth() const
{
    WORD    chnlWidth = Xfer_64;

    switch ( m_xferMode ) {

        case Xfer_64      :
        case Xfer_2x64    :
        case Xfer_3x64    :
        case Xfer_4x64    :
        case Xfer_5x64    :
        case Xfer_6x64    :  { chnlWidth = Xfer_64;    break; }

        case Xfer_384     :
        case Xfer_2x384   :
        case Xfer_3x384   :
        case Xfer_4x384   :
        case Xfer_5x384   :  {  chnlWidth = Xfer_384;  break; }

        case Xfer_128     :
        case Xfer_192     :
        case Xfer_256     :
        case Xfer_320     :
        case Xfer_512     :
        case Xfer_768     :
        case Xfer_1152    :
        case Xfer_1472    :
        case Xfer_1536    :
        case Xfer_1920    :  { chnlWidth = m_xferMode; break; }

        default           :  { break; }
    }
    return chnlWidth;
}

/////////////////////////////////////////////////////////////////////////////
void  CXferMode::DeSerialize(WORD format,CSegment& seg)
{
    switch ( format )  {

        case SERIALEMBD : {
            break;
        }
        case NATIVE     : {
            seg >> m_xferMode;
            break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CXferMode::Serialize(WORD format,CSegment& seg)
{
    BYTE    xferCmd = XFERCMDATTR;

    switch ( format )  {
        case SERIALEMBD : {
            xferCmd |= m_xferMode;
            seg << xferCmd;
            break;
        }
        case NATIVE     : {
            seg << m_xferMode;
            break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
WORD CXferMode::operator<=(const CCapH320& cap) const
{
    WORD    rval = 0;

    switch ( m_xferMode ) {
        case Xfer_64    : {
            rval = cap.OnXferCap(e_Xfer_Cap_B);
            break;
        }
        case Xfer_2x64  : {
            rval = cap.OnXferCap(e_Xfer_Cap_2B);
            break;
        }
        case Xfer_3x64  : {
            rval = cap.OnXferCap(e_Xfer_Cap_3B);
            break;
        }
        case Xfer_4x64  : {
            rval = cap.OnXferCap(e_Xfer_Cap_4B);
            break;
        }
        case Xfer_5x64  : {
            rval = cap.OnXferCap(e_Xfer_Cap_5B);
            break;
        }
        case Xfer_6x64  : {
            rval = cap.OnXferCap(e_Xfer_Cap_6B);
            break;
        }
        case Xfer_384   : {
            rval = cap.OnXferCap(e_Xfer_Cap_H0);
            break;
        }
        case Xfer_2x384 : {
            rval = cap.OnXferCap(e_Xfer_Cap_2H0);
            break;
        }
        case Xfer_3x384 : {
            rval = cap.OnXferCap(e_Xfer_Cap_3H0);
            break;
        }
        case Xfer_4x384 : {
            rval = cap.OnXferCap(e_Xfer_Cap_4H0);
            break;
        }
        case Xfer_5x384 : {
            rval = cap.OnXferCap(e_Xfer_Cap_5H0);
            break;
        }
        case Xfer_128   : {
            rval = cap.OnXferCap(e_Xfer_Cap_128);
            break;
        }
        case Xfer_192   : {
            rval = cap.OnXferCap(e_Xfer_Cap_192);
            break;
        }
        case Xfer_256   : {
            rval = cap.OnXferCap(e_Xfer_Cap_256);
            break;
        }
        case Xfer_320   : {
            rval = cap.OnXferCap(e_Xfer_Cap_320);
            break;
        }
        case Xfer_512   : {
            rval = cap.OnXferCap(e_Xfer_Cap_512);
            break;
        }
        case Xfer_768   : {
            rval = cap.OnXferCap(e_Xfer_Cap_768);
            break;
        }
        case Xfer_1152  : {
            rval = cap.OnXferCap(e_Xfer_Cap_1152);
            break;
        }
        case Xfer_1472  : {
            rval = cap.OnXferCap(e_Xfer_Cap_1472);
            break;
        }
        case Xfer_1536  : {
            rval = cap.OnXferCap(e_Xfer_Cap_H11);
            break;
        }
        case Xfer_1920  : {
            rval = cap.OnXferCap(e_Xfer_Cap_H12);
            break;
        }
        default         : {
            break;
        }
    }

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
WORD  operator==(const CXferMode& rXferMode_1,const CXferMode& rXferMode_2)
{
    WORD    rval = 0;

    if ( rXferMode_1.m_xferMode == rXferMode_2.m_xferMode )
        rval = 1;

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
WORD  operator<=(const CXferMode& rXferMode_1,const CXferMode& rXferMode_2)
{
    WORD    rval = 0;

    if ( rXferMode_2.m_xferMode != 0xFF && rXferMode_1.m_xferMode <= rXferMode_2.m_xferMode )
        rval = 1;

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
CVidMode::CVidMode()          // constructor
{
    m_vidMode        = Video_Off;
    m_isAutoVidScm   = 0;     // ????
    m_isVideoSwitchAutoVideoProtocol_H261_H263_H264 = 0;
    m_cifMpi         = V_1_29_97;
    m_qcifMpi        = V_1_29_97;
    m_vidImageFormat = V_Cif;
    m_isFreeBitRate  = TRUE;
    m_h263Mpi        = MPI_30;
}

/////////////////////////////////////////////////////////////////////////////
CVidMode::~CVidMode()        // destructor
{
}

////////////////////////////////////////////////////////////////////////////
const char*   CVidMode::NameOf()  const
{
    return "CVidMode";
}
/////////////////////////////////////////////////////////////////////////////
WORD  CVidMode::IsVideoSwitchAutoVideoProtocol() const
{
	return m_isVideoSwitchAutoVideoProtocol_H261_H263_H264;
}

/////////////////////////////////////////////////////////////////////////////
void  CVidMode::Dump() const
{
//     WORD         msgBufSize = 2048;
//     char*        msgStr = new char[msgBufSize];
//     std::ostream  msg(msgStr,msgBufSize);
	std::ostringstream  msg;

    msg << " Selected Communication Mode is :\n";
    msg << " --------------------------------\n";

    if(m_isFreeBitRate)
        msg << " Transcoding \n";
    else {
        msg << " Video Switching ";
        if(m_isVideoSwitchAutoVideoProtocol_H261_H263_H264)
            msg << "with Auto Video Protocol(H261 <--> H263 <--> H264) \n";
		if(m_isAutoVidScm && !m_isVideoSwitchAutoVideoProtocol_H261_H263_H264)
	         msg << "with Auto Video Scm \n";
        if(!m_isAutoVidScm)
            msg << "with Fixed Video Scm \n";
    }

    if( m_vidMode == Video_Off )
        msg << " Video Protocol: " << "Video Off" << "\n";

    if( m_vidMode == H261 ) {
        WORD MPI;
        msg << " Video Protocol: " << "H261" << "\n";
        if( m_vidImageFormat == V_Cif ) {
            MPI = 30 / (m_cifMpi - 21); // according to Table A1 [H221]
            msg << " Video Format: "   << "  CIF" << "\n"
                << " Frame Rate:     " << MPI << " fps \n";
        }
        if( m_vidImageFormat == V_Qcif ) {
            MPI = 30 / (m_qcifMpi - 21); // according to Table A1 [H221]
            msg << " Video Format: "   << "  QCIF" << "\n"
                << " Frame Rate:     " << MPI << " fps \n";
        }
    }

    if( m_vidMode == H263 ) {
        msg << " Video Protocol: "  << "H263" << "\n"
            << " Video Format:   "  << h263_cap_Format[(int)m_vidImageFormat] << "\n"
            << " Frame Rate:     "  << h263_cap_MPI[(int)m_h263Mpi];
    }


    if( m_vidMode == H264 ) {
		DWORD tmp = 0;
        msg << " Video Protocol:  "  << "H264" << "\n"
			<< " Level:           "  << m_H264VidMode.GetLevelAsString() << "\n";

	    if (m_H264VidMode.GetMaxMBPS() != 0xFFFFFFFF)
			msg << " MaxMBPS:         "  << m_H264VidMode.GetMaxMBPS() << "  (MB/s)\n";
		else msg << " MaxMBPS:         Default for Level.       " << "\n";

		if (m_H264VidMode.GetMaxFS() != 0xFFFFFFFF)
			msg << " MaxFS:           "  << m_H264VidMode.GetMaxFS() << "  (MBs)\n";
		else msg << " MaxFS:           Default for Level.         " << "\n";

		if (m_H264VidMode.GetMaxDPB() != 0xFFFFFFFF)
			msg << " MaxDPB:          "  << m_H264VidMode.GetMaxDPB() << "  (Bytes)\n";
		else msg << " MaxDPB:          Default for Level.        " << "\n";

		if (m_H264VidMode.GetMaxBR() != 0xFFFFFFFF)
			msg << " MaxBR:           "  << m_H264VidMode.GetMaxBR() << "  (bits/s)\n";
		else msg  << " MaxBR:           Default for Level.         " << "\n";

		if (m_H264VidMode.GetMaxBR() != 0xFFFFFFFF)
			msg << " MaxCPB:          "  << m_H264VidMode.GetMaxCPB() << "  (bits)\n"; //*level Factor is missing
		else msg  << " MaxCPB:          Default for Level.         " << "\n";

		msg  << " Sample Aspect Ratio: " << m_H264VidMode.GetSAR() << "\n";
    }

	if( m_vidMode == H26L )
	{
        msg << " Video Protocol: "  << "H264*" << "\n";

		int index;

		if(m_NSCVidMode.GetH26LCifMpiForOneVideoStream()) // Is CIF ?
		{
				index = (int)m_NSCVidMode.GetH26LCifMpiForOneVideoStream() - 1;
				
				msg << " Video Format:   "  << "CIF" << "\n"
				    << " Frame Rate:     ";

				if (index >= 0)
				{
					msg << h263_cap_MPI[index] << "\n";
				}
				else
				{
					msg << "Unexpected Error" << "\n";
					PASSERT(1);
				}
		}

		if(m_NSCVidMode.GetH26L4CifMpiForOneVideoStream()) // Is 4CIF ?
		{
				index = (int)m_NSCVidMode.GetH26L4CifMpiForOneVideoStream() - 1;
				
				msg << " Video Format:   "  << "4CIF" << "\n"
				    << " Frame Rate:     ";

				if (index >= 0)
				{
					msg << h263_cap_MPI[index] << "\n";
				}
				else
				{
					msg << "Unexpected Error" << "\n";
					PASSERT(1);
				}
		}
    }

    msg << "\n";

    PTRACE(eLevelInfoNormal, (char*)msg.str().c_str());
//     PDELETEA(msgStr);
}

/////////////////////////////////////////////////////////////////////////////
void  CVidMode::SetH261VidMode(WORD vidBitRate,WORD h261vidImageFormat,
                               WORD qcifMpi,WORD cifMpi)
{
    m_isAutoVidScm     = vidBitRate;  //????
    m_vidMode        = H261;
    if ( h261vidImageFormat == 255 )  { // auto video scm selection
        m_vidImageFormat = V_Cif;   // max video scm
        m_cifMpi         = V_1_29_97;
        m_qcifMpi        = V_1_29_97;
        ON(m_isAutoVidScm);
    }  else  {
        m_vidImageFormat = h261vidImageFormat;
        m_cifMpi         = cifMpi;
        m_qcifMpi        = qcifMpi;
    }
}

/////////////////////////////////////////////////////////////////////////////
void CVidMode::SetH263VidMode(WORD vidBitRate,WORD h263VidImageFormat,WORD h263Mpi)
{
    m_isAutoVidScm   = vidBitRate;
    m_vidMode        = H263;
    if ( h263VidImageFormat == 255 )  { // auto video scm selection
        m_vidImageFormat = H263_CIF;   // max MCU video scm
        m_h263Mpi        = MPI_1;
        ON(m_isAutoVidScm);
    }
	else  {
        m_vidImageFormat = h263VidImageFormat;
        SetH263Mpi(h263Mpi);
    }
}
/////////////////////////////////////////////////////////////////////////////
void  CVidMode::SetH263Mpi(WORD H263Mpi)
{
	// value range check
	if(H263Mpi > MPI_30){
		CSmallString sstr;
		sstr << " illegal mpi value = " << H263Mpi << " , m_h263Mpi sets to MPI_1";
		if(H263Mpi > 15)
		{
			// if mpi value > 15 exception aqure when using h263_cap_MPI[(int)m_h263Mpi] in dump file
			// we have definition static char*  h263_cap_MPI[16] in include/h263.h
			DBGPASSERT(H263Mpi);
		}
		m_h263Mpi = MPI_1;
		PTRACE2(eLevelError,"CVidMode::SetH263Mpi - ",sstr.GetString());
		return;
	}
	m_h263Mpi = H263Mpi;
}

/////////////////////////////////////////////////////////////////////////////
void CVidMode::SetH264VidMode(WORD isAutoVidScm,BYTE h264Level, DWORD maxBR, DWORD maxMBPS,
							  DWORD maxFS, DWORD maxDPB, BYTE maxSAR )
{
	DWORD maxCPB;

	m_isAutoVidScm   = isAutoVidScm;
    m_vidMode        = H264;
	m_H264VidMode.SetProfileValue(H264_Profile_BaseLine);
	//WORD H264_level =  m_H264VidMode.ConvertBitRateToLevel(confTransferRate);
	m_H264VidMode.SetLevelValue(h264Level); //VideoSwitch (FOR CP put switch case and set according)
	m_H264VidMode.SetMaxMBPS(maxMBPS);
	m_H264VidMode.SetMaxFS(maxFS);
	m_H264VidMode.SetMaxDPB(maxDPB);
	m_H264VidMode.SetMaxBR(maxBR);


	if (maxBR != 0xFFFFFFFF)
	{

		CH264Details h264ConstDetails(h264Level);
		DWORD defaultCPBForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_CPB_CODE);
		DWORD defaultBRForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_BR_CODE);

		maxCPB = (maxBR*defaultCPBForLevel)/defaultBRForLevel;
		if(((maxBR*defaultCPBForLevel) % defaultBRForLevel)>0)
			maxCPB+=1;
		m_H264VidMode.SetMaxCPB(maxCPB);
	}

	m_H264VidMode.SetSAR(maxSAR);

}
/////////////////////////////////////////////////////////////////////////////
void CVidMode::SetH264DefaultVswVidMode(WORD isAutoVidScm,BYTE h264Level, DWORD maxBR, DWORD maxMBPS,
							  DWORD maxFS, DWORD maxDPB )
{
	SetH264VidMode(isAutoVidScm,h264Level,maxBR,maxMBPS,maxFS,maxDPB );
}
/////////////////////////////////////////////////////////////////////////////
void CVidMode::SetH264VidMode(const CCapSetH264& h264CapSet)
{
	m_vidMode = H264;
	m_H264VidMode.SetProfileValue((BYTE)h264CapSet.GetCapH264ProfileValue());
	m_H264VidMode.SetLevelValue((BYTE)h264CapSet.GetCapH264LevelValue());
	m_H264VidMode.SetMaxMBPS(h264CapSet.GetCapH264CustomMaxMBPS()); //set with word parameter
	m_H264VidMode.SetMaxFS(h264CapSet.GetCapH264CustomMaxFS());
	m_H264VidMode.SetMaxDPB(h264CapSet.GetCapH264CustomMaxDPB());
	m_H264VidMode.SetMaxBR(h264CapSet.GetCapH264CustomMaxBRandCPB());
	m_H264VidMode.SetMaxCPB(h264CapSet.GetCapH264CustomMaxBRandCPB());
	m_H264VidMode.SetSAR(h264CapSet.GetCapH264CustomMaxSAR());

}

/////////////////////////////////////////////////////////////////////////////
void CVidMode::UpdateH264VidMode(CH264VidMode newH264 )
{
	DWORD maxCPB;

    m_vidMode        = H264;
	m_H264VidMode.SetProfileValue(H264_Profile_BaseLine);
	m_H264VidMode.SetLevelValue(newH264.GetLevelValue()); //VideoSwitch (FOR CP put switch case and set according)
	m_H264VidMode.SetMaxMBPS(newH264.GetMaxMBPS());
	m_H264VidMode.SetMaxFS(newH264.GetMaxFS());
	m_H264VidMode.SetMaxDPB(newH264.GetMaxDPB());
	m_H264VidMode.SetMaxBR(newH264.GetMaxBR());
	if (newH264.GetMaxBR() != 0xFFFFFFFF)
	{

		CH264Details h264ConstDetails(newH264.GetLevelValue());
		DWORD defaultCPBForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_CPB_CODE);
		DWORD defaultBRForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_BR_CODE);

		maxCPB = (newH264.GetMaxBR()*defaultCPBForLevel)/defaultBRForLevel;
		if(((newH264.GetMaxBR()*defaultCPBForLevel) % defaultBRForLevel)>0)
			maxCPB+=1;
		m_H264VidMode.SetMaxCPB(maxCPB);
	}

	m_H264VidMode.SetSAR(newH264.GetSAR());


}


/////////////////////////////////////////////////////////////////////////////
BYTE CVidMode::IsH264HigherThanQcifResolution()
{
	if(m_H264VidMode.GetLevelValue() > H264_Level_1 ||
		(m_H264VidMode.GetMaxFS()!= 0xFFFFFFFF && m_H264VidMode.GetMaxFS() > H264_L1_DEFAULT_FS)) //VideoSwitch (FOR CP put switch case and set according)

		return TRUE;
	else return FALSE;

}

////////////////////////////////////////////////////////////////////////////
BYTE CVidMode::IsH264CIFResolution()
{
	if( ((m_H264VidMode.GetLevelValue() >= H264_Level_1_1) && (m_H264VidMode.GetLevelValue() < H264_Level_2_1) &&
		(m_H264VidMode.GetMaxFS()== 0xFFFFFFFF)	)||
		( (m_H264VidMode.GetMaxFS()!= 0xFFFFFFFF) && (m_H264VidMode.GetMaxFS() == H264_L1_1_DEFAULT_FS)) ) //VideoSwitch (FOR CP put switch case and set according)

		return TRUE;
	else return FALSE;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVidMode::GetH264ResolutionFrameRateAndDPB(WORD& res, WORD& frameRate , DWORD& dpb )
{

	if(GetVidMode() != H264 )
		return;

	BYTE  h264level = GetH264VidMode().GetLevelValue();
	DWORD h264MBPS = GetH264VidMode().GetMaxMBPS();
	DWORD h264FS = GetH264VidMode().GetMaxFS();

	DWORD frame_rate = 0;
	DWORD frame_rate_for_level = 0;

	CH264Details h264ConstDetails(h264level);

	//Ability for level
	DWORD h264MBPS_for_level = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_MBPS_CODE);
	DWORD h264FS_for_level   = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_FS_CODE);

	//Ability level+custom
	if(h264MBPS == 0xFFFFFFFF)
		h264MBPS = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_MBPS_CODE);
	if(h264FS == 0xFFFFFFFF)
		h264FS = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_FS_CODE);

	if( (h264MBPS > 0) && (h264FS > 0))
		   frame_rate = h264MBPS/h264FS;
	else
	   DBGPASSERT(1);

	if( (h264MBPS_for_level > 0) && (h264FS_for_level > 0))
		 frame_rate_for_level = h264MBPS_for_level/h264FS_for_level;
	else
		DBGPASSERT(1);

	//the maximum that the EP decleare is the max between the level Frame Rate and the Custom Frame Rate
	frame_rate = max(frame_rate,frame_rate_for_level);

	//if a limit exits , we set the maximum mpi according to the codec encoder capabilitys
	if (h264FS >= H264_RESOLUTION_CIF)
	{
		res = RESOLUTION_CIF;  // this is used for OpenPort for the Encoder params
		if( frame_rate > 0  )
		{
			if(frame_rate == 25)
				frameRate = 25;
			else
				frameRate = 30/(MAX(frame_rate,FRAME_RATE_15));   //CIF15
		}
		else frameRate = 30/FRAME_RATE_15;
		PTRACE2INT(eLevelInfoNormal,"CVidMode::GetH264ResolutionFrameRateAndDPB : H264 Resolution is %d MB/s",res );//|VIDBRDG_TRACE
		PTRACE2INT(eLevelInfoNormal,"CVidMode::GetH264ResolutionFrameRateAndDPB : H264 framRate is %d ",frameRate );//|VIDBRDG_TRACE
    }

    else
	{
	  res = RESOLUTION_QCIF;
	  if (frame_rate > 0)
	  {
	  	  if(frame_rate == 25)
			frameRate = 25;
		  else
			frameRate = 30/(MAX(frame_rate,FRAME_RATE_30));   //QCIF30
	  }
	  else frameRate = 30/FRAME_RATE_30;
	  PTRACE2INT(eLevelInfoNormal,"CVidMode::GetH264ResolutionFrameRateAndDPB : H264 Resolution is %d MB/s",res );//|VIDBRDG_TRACE
	  PTRACE2INT(eLevelInfoNormal,"CVidMode::GetH264ResolutionFrameRateAndDPB : H264 framRate is %d ",frameRate );//|VIDBRDG_TRACE
    }

	if (dpb != 0)
	{
		dpb = GetH264VidMode().GetMaxDPB();
		if(dpb == 0xFFFFFFFF)
		{
			dpb = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_DPB_CODE);
			PTRACE2INT(eLevelInfoNormal,"CVidMode::GetH264ResolutionFrameRateAndDPB : H264 dpb is %d ",dpb );//|VIDBRDG_TRACE
		}
	}

	return;
}
/////////////////////////////////////////////////////////////////////////////
void CVidMode::SetH26LVidMode(WORD isAutoVidScm,WORD vidImageFormat,WORD NumB0Chnl)
{
    m_isAutoVidScm   = isAutoVidScm;
    m_vidMode        = H26L;

    if ( vidImageFormat == AUTO )
	{
		m_NSCVidMode.SetH26LMpi(NumB0Chnl);
        ON(m_isAutoVidScm);
    }
	else //Only auto mode is implemented.
	{
		if(vidImageFormat)
			PASSERT(vidImageFormat);
		else
			PASSERT(101);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CVidMode::SetH26LVidMode(WORD cif4Mpi,WORD cifMpi)
{
    m_isAutoVidScm     = 0;  //????
    m_vidMode		   = H26L;
	m_NSCVidMode.SetH26LCifMpi(cifMpi);
	m_NSCVidMode.SetH26L4CifMpi(cif4Mpi);
}

/////////////////////////////////////////////////////////////////////////////
WORD CVidMode::GetH26LMpi(WORD h320FormatType) const
{
	if(h320FormatType == H26L_CIF)
		return m_NSCVidMode.GetH26LCifMpiForOneVideoStream();
	else if(h320FormatType == H26L_CIF_4)
		return m_NSCVidMode.GetH26L4CifMpiForOneVideoStream();
	else
		return 0;
}

/////////////////////////////////////////////////////////////////////////////
void CVidMode::Create(CCapH320* pCap,WORD vidBitRate,WORD vidMode,WORD RequestedFormat)
{
    m_isAutoVidScm = vidBitRate;

    m_vidMode      = Video_Off;

    // If vidMode is H264 and pCap doesn't have capabilities for H264
    // the CVidMode must be set to H263 according to pCap H263 caps.
    // If pCap doesn't H263 caps too, the CVidMode is set to h261 if not stayed 'video_off'.

	// If vidMode is H26 and pCap doesn't have capabilities for H263
    // the CVidMode must be set to H261 according to pCap H261 caps.
    // If pCap doesn't H261 caps too, the CVidMode is set to h261 if not stayed 'video_off'.

    switch (vidMode) {

		case H264 : {
            // in case of h264 caps
            if ( pCap->IsH264() )
			{
                CCapH264 capH264 = *pCap->GetCapH264();
                m_vidMode        = H264;
                // set max H264 Level
				CCapSetH264* pCapSetH264 = NULL;
				// Romem - 25.5.11: High profile compatability with HDX ISDN
				BOOL bEnableHighfProfileInIsdn = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE_WITH_ISDN);
				if( IsFeatureSupportedBySystem(eFeatureH264HighProfile) && bEnableHighfProfileInIsdn )
				{
					pCapSetH264 = capH264.GetH264HighProfileCapSet();
				}
				else
				{
					pCapSetH264 = capH264.GetLastCapSet(); //the last set is with the highest level
				}
				if(pCapSetH264 == NULL)
				{
				   pCapSetH264 = capH264.GetLastCapSet(); //the last set is with the highest level
				}

			    	//Set the CH264VidMode parameters according to the Capabilities
			    	m_H264VidMode.SetProfileValue((BYTE)pCapSetH264->GetCapH264ProfileValue());
			    	m_H264VidMode.SetLevelValue((BYTE)pCapSetH264->GetCapH264LevelValue());
			    	m_H264VidMode.SetMaxMBPS(pCapSetH264->GetCapH264CustomMaxMBPS()); //set with word parameter
			    	m_H264VidMode.SetMaxFS(pCapSetH264->GetCapH264CustomMaxFS());
			    	m_H264VidMode.SetMaxDPB(pCapSetH264->GetCapH264CustomMaxDPB());
			    	m_H264VidMode.SetMaxBR(pCapSetH264->GetCapH264CustomMaxBRandCPB());
			    	m_H264VidMode.SetMaxCPB(pCapSetH264->GetCapH264CustomMaxBRandCPB());
			    	m_H264VidMode.SetSAR(pCapSetH264->GetCapH264CustomMaxSAR());


				break;
			}

			//continue to set the H263
         }



        case H263 : {
            // in case of h263 caps or no H264caps
            if ( pCap->IsH263() ) {
                CCapH263 capH263 = *pCap->GetCapH263();
                m_vidMode        = H263;
                // set max H263 Image format & MPI (Only standard formats)
				CCapSetH263* pCapSetH263 = NULL;
				WORD numberOfH263Sets = capH263.GetNumberOfH263Sets();
				WORD I = 0;

				if( RequestedFormat == AUTO)
				{
					for(I = numberOfH263Sets ; I > 0 ; I -- )
					{
						pCapSetH263 = capH263.GetCapSetH263(I);
						if(pCapSetH263 && pCapSetH263->GetVidImageFormat() <= H263_CIF_16)
						{
							m_vidImageFormat = (WORD) pCapSetH263->GetVidImageFormat();
							SetH263Mpi((WORD) pCapSetH263->GetMPI());
							return;
						}
					}
				}
				else //The MCMS searches form the lowest resolution to the highest.
				{
					for(I = 1 ; I <= numberOfH263Sets ; I ++ )
					{
						pCapSetH263 = capH263.GetCapSetH263(I);

						if( pCapSetH263 && pCapSetH263->GetVidImageFormat() <= H263_CIF_16) {
							// The relevant resolution - is the resolution itself or the first
							// higher resolution (in case the requested resolution was not
							// explicitly declared).
							if( pCapSetH263->GetVidImageFormat() >= RequestedFormat)
							{
								m_vidImageFormat = (WORD) pCapSetH263->GetVidImageFormat();
								SetH263Mpi((WORD) pCapSetH263->GetMPI());
								return;
							}
						}
					}
				}

				// The E.P did not declare explicitly on standard formats.
				pCapSetH263 = capH263.GetCapSetH263(capH263.GetNumberOfH263Sets());
				if(!pCapSetH263)
				{
					PTRACE(eLevelInfoNormal,"CVidMode::Create ipCapSetH263 s NULL!");
					break;
				}

				BYTE MinCustomPictureHeight = pCapSetH263->GetMinCustomPictureHeight();
				BYTE MinCustomPictureWidth = pCapSetH263->GetMinCustomPictureWidth();

				if(MinCustomPictureHeight >= 143 && MinCustomPictureWidth >= 175 )    // H263_CIF_16
					m_vidImageFormat = H263_CIF_16;
				else if(MinCustomPictureHeight >= 71 && MinCustomPictureWidth >= 87 ) // H263_CIF_4
					m_vidImageFormat = H263_CIF_4;
				else if(MinCustomPictureHeight >= 35 && MinCustomPictureWidth >= 43 ) // H263_CIF
					m_vidImageFormat = H263_CIF;
				else if(MinCustomPictureHeight >= 17 && MinCustomPictureWidth >= 21 ) // H263_QCIF_SQCIF
					m_vidImageFormat = H263_QCIF_SQCIF;


				SetH263Mpi((WORD) pCapSetH263->GetMPI()); // Set Custom format Mpi.
                break;
            }
        }
        case H261 : { //this case is also used if party has no H263 caps
            if ( pCap->IsH261VideoCap(V_Qcif) )  {
                m_vidMode        = H261;
                m_vidImageFormat = V_Qcif;
                m_qcifMpi        = pCap->GetH261CapMpi(V_Qcif);//m_qCifMpi;
            }
            if ( pCap->IsH261VideoCap(V_Cif) )  {
                m_vidMode        = H261;
                m_vidImageFormat = V_Cif;
                m_cifMpi         = pCap->GetH261CapMpi(V_Cif);//m_cifMpi;
                m_qcifMpi        = pCap->GetH261CapMpi(V_Qcif);//m_qCifMpi;
            }
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date: 01/2004	programer: Eitan
// Description:		The function intersects other's capabilities with local video mode
//
// input:			CCapH320& other capabilities to intersect
// Returns:			return value: CVidMode = this.m_vidMode /\ remoteCap
// Remarks:			1.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVidMode::Intersect(CCapH320* pCap, WORD isAutoVidScm,CVidMode& IntersectVidMode)
{

	CMedString cstr;
	WORD H263VidFormat = this->GetVidFormat(); 
	cstr << "CVidMode::Intersect\n";

	switch (m_vidMode)
	{
		case H264:
		{
			IntersectVidMode.Create(pCap,isAutoVidScm,H261);
			// in case the local video mode does not contain H.263
			// we should not consider H263 in intersect
			if (H263VidFormat < NUMBER_OF_H263_FORMATS)
				IntersectVidMode.Create(pCap,isAutoVidScm,H263,H263VidFormat);
			IntersectVidMode.Create(pCap,isAutoVidScm,H264);

			cstr << "local video mode is 264 \n";
			switch(IntersectVidMode.GetVidMode())
			{
				case H264:
				{
					//we have 264 and remote has 264 -> find the highest common level, FS, Mbps
					if (m_H264VidMode.IsLevelValueIsLegal())
					{
						cstr << "remote video mode is 264 -> intersect\n";
						IntersectVidMode.m_H264VidMode.CalcAndSetCommonLevelAndParam(m_H264VidMode);
											
					}
					else
					{
						cstr << "local H264Video mode is illegal -> Intersect = cap";
					}
					break;
				}

				case H263:
				{
					//we have 264 and remote has 263 -> find the highest common FrameRate, Resolution
					cstr << "remote video mode is 263 -> Intersect Local H263 VidMode with remote video mode \n";
					SetVidMode(H263);
					IntersectVidMode.SetCommonVidMode(*this);
					break;
				}
				case H261:
				{
					cstr << "remote video mode is 261 -> Set IntersectVidMode to remote video mode \n";
					break;
				}
				default:
				{
					cstr << "***remote has no video mode, setting video to Video_off  \n";
					IntersectVidMode.SetVidMode(Video_Off);
					break;
				}
			}
			break;
		}//local vid mode = 264
		case H263:
		{
			IntersectVidMode.Create(pCap,isAutoVidScm,H261);
			IntersectVidMode.Create(pCap,isAutoVidScm,H263,H263VidFormat);

			cstr << "local video mode is 263 \n";
			switch(IntersectVidMode.GetVidMode())
			{
				case H263:
				{
					if (IsH263FormatAndMpiAreLegal())
					{
						cstr << "remote video mode is 263 -> Intersect\n";
						IntersectVidMode.SetCommonVidMode(*this);
					}
					else
					{
						cstr << "local H263 Video mode is illegal -> Intersect = cap";
					}
					break;
				}
				case H261:
				{
					cstr << "remote has 261 -> Set IntersectVidMode to remote video mode \n";
					break;
				}
				default:
				{
					cstr << "***remote has no video mode or remote video mode > local video mode, setting video to Video_off  \n";
					IntersectVidMode.SetVidMode(Video_Off);
					break;
				}
			}
			break;
		}//local vid mode = 263
		case H261:
		{
			IntersectVidMode.Create(pCap,isAutoVidScm,H261);
			cstr << "local video mode is 261 \n";
			switch(IntersectVidMode.GetVidMode())
			{
				case H261:
				{
					cstr << "remote has 261 -> Intersect\n";
					IntersectVidMode.SetCommonVidMode(*this);
					break;
				}
				default:
				{
					cstr << "***remote has no video mode or remote video mode > local video mode, setting video to Video_off  \n";
					IntersectVidMode.SetVidMode(Video_Off);
					break;
				}
			}
			break;
		}//local vid mode = 261
	}//switch (m_vidMode)

	if (IntersectVidMode.GetVidMode() == H263 && IntersectVidMode.m_vidImageFormat > H263_CIF_4)
	{
		cstr << "Intersect video mode is H263 and vidImageFormat > H263_CIF_4  \n";
		cstr << "Set to H263 CIF30 \n";
		IntersectVidMode.SetH263VidMode(isAutoVidScm,AUTO);
	}
	PTRACE(eLevelInfoNormal,cstr.GetString());


}
////////////////////////////////////////////////////////////////////////////
BYTE  CVidMode::IsH263FormatAndMpiAreLegal()
{
	BYTE vidFormatLegal = (m_vidImageFormat < NUMBER_OF_H263_FORMATS )? TRUE : FALSE;
	BYTE vidMpiLegal = (m_h263Mpi <= MPI_30 )? TRUE : FALSE;

	return (vidFormatLegal & vidMpiLegal);
}

////////////////////////////////////////////////////////////////////////////
void  CVidMode::SetVidMode(WORD mode)
{
    m_vidMode = mode;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CVidMode::GetVidMode() const
{
    return m_vidMode;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CVidMode::GetVidFormat() const
{
    return m_vidImageFormat;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CVidMode::GetCifMpi() const
{
    return m_cifMpi;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CVidMode::GetQcifMpi() const
{
    return m_qcifMpi;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CVidMode::GetH263Mpi() const
{
    return m_h263Mpi;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CVidMode::GetOctet0() const
{
	return m_NSCVidMode.GetOctet0();
}
/////////////////////////////////////////////////////////////////////////////
BYTE CVidMode::GetOctet1() const
{
	return m_NSCVidMode.GetOctet1();
}
/////////////////////////////////////////////////////////////////////////////
void  CVidMode::DeSerialize(WORD format,CSegment& seg)
{
    switch ( format )  {

        case SERIALEMBD : {
            break;
        }
        case NATIVE     : {
            seg >> m_vidMode
                >> m_isAutoVidScm
                >> m_isVideoSwitchAutoVideoProtocol_H261_H263_H264
                >> m_cifMpi
                >> m_qcifMpi
                >> m_vidImageFormat
                >> m_isFreeBitRate
                >> m_h263Mpi;

			m_NSCVidMode.DeSerialize(format,seg);
			m_H264VidMode.DeSerialize(format,seg);

            break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CVidMode::Serialize(WORD format,CSegment& seg)
{
    BYTE    vidCmd = OTHRCMDATTR;

    switch ( format )  {
        case SERIALEMBD : {

			if(m_vidMode == H26L)
			{
				 seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
                 seg << (BYTE) 5;                                           // msgLen
                 seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
                     << (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
                 seg << (BYTE)( W_MANUFACT_CODE_PICTURETEL & 0xFF )         // manufacturer code junior byte
                            << (BYTE)( (W_MANUFACT_CODE_PICTURETEL >> 8) & 0xFF ); // manufacturer code senior byte
                 seg << (BYTE) NS_COM_H26L_ON;
				// H.26L algorithm ON
			}
			else // H261/H263/H264
			{
				vidCmd |= m_vidMode;
				seg << vidCmd;
			}

            break;
        }
        case NATIVE     : {
            seg << m_vidMode
                << m_isAutoVidScm
                << m_isVideoSwitchAutoVideoProtocol_H261_H263_H264
                << m_cifMpi
                << m_qcifMpi
                << m_vidImageFormat
                << m_isFreeBitRate
                << m_h263Mpi;

			m_NSCVidMode.Serialize(format,seg);
			m_H264VidMode.Serialize(format,seg);
            break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
WORD    CVidMode::operator<=(const CCapH320& cap) const
{
    WORD    rval = 1;

	// IMPORTANT : This operator <= is not enough for H263 Annexes Because the information about Annexes
	// is not exist at CVidMode.Thus in addition to this we have to operate   CCapH263 <= CCapH263.

    if ( m_vidMode == H261 && !m_isAutoVidScm /* no auto scm */ )  {
	    rval = cap.IsH261VideoCap(m_vidImageFormat);
        if ( ! rval )
            return rval;
        // in case of cif capbilty two mpi values are send
        if ( m_vidImageFormat == V_Cif )  {
            if ( m_cifMpi < cap.GetH261CapMpi(V_Cif))//m_cifMpi )
                rval = 0;
            //      rval = cap.OnDataVidCap(m_cifMpi);
            if ( ! rval )
                return rval;
        }
        if ( m_vidImageFormat == V_Qcif )  {
        //    rval = cap.OnDataVidCap(m_qcifMpi);
		    if ( m_qcifMpi < cap.GetH261CapMpi(V_Qcif))//m_qCifMpi )
                rval = 0;
            if ( ! rval )
                return rval;
        }
    }

    if ( m_vidMode == H263 && !cap.IsH263() )
        return 0;

	if ( m_vidMode == H264 && !cap.IsH264() )
        return 0;

	//H.26L
	if ( m_vidMode == H26L)
	{
		rval = m_NSCVidMode <= cap;
    }

	/*if ( m_vidMode == H263 && !m_isAutoVidScm ) {
        rval = cap.OnH263Format(m_vidImageFormat);
        if ( ! rval ) return rval;
        if ( m_h263Mpi < (WORD)(cap.GetCapH263()).GetFormatMPI(m_vidImageFormat) )
            rval = 0;
    }*/

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CVidMode::DiffMpiFormat(const CVidMode& other) const
{
	if(m_vidMode != other.m_vidMode)
	{
		if(other.m_vidMode)
			PASSERT(other.m_vidMode);
		else
			PASSERT(101);
	}

    WORD    rval = 0;
    if( m_vidMode == H261 ) {
        if ( ( m_vidImageFormat != other.m_vidImageFormat ) ||
             ( m_cifMpi         != other.m_cifMpi )         ||
             ( m_qcifMpi        != other.m_qcifMpi ) )
            rval =1;
    }
    if( m_vidMode == H263 ) {
        if ( (m_vidImageFormat != other.m_vidImageFormat ) ||
             (m_h263Mpi        != other.m_h263Mpi ) )
            rval =1;
    }

	if( m_vidMode == H26L ) {
        if ( (m_NSCVidMode.GetOctet0() != other.m_NSCVidMode.GetOctet0() ) ||
             (m_NSCVidMode.GetOctet1() != other.m_NSCVidMode.GetOctet1()) )
            rval =1;
    }

    return rval;
}
/////////////////////////////////////////////////////////////////////////////

WORD  CVidMode::IsDiffH264LevelOrParam(const CVidMode& other) const
{
	WORD    rval = 0;
	if(m_vidMode == H264 && other.m_vidMode == H264)
	{
		if(m_H264VidMode.IsDiffLevelOrParam(other.m_H264VidMode))
		{
			rval = 1;
		}
	}
	return rval;
}
//////////////////////////////////////////////////////////////////////////////
WORD  CVidMode::IsDiffPLFLevelOrParam(const CVidMode& other) const
{
	WORD    rval = 0;
	if(m_vidMode == H264 && other.m_vidMode == H264)
	{
		if(m_H264VidMode.IsDiffPLF(other.m_H264VidMode))
		{
			rval = 1;
		}
	}
	return rval;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CVidMode::IsCapableOfHD720_15fps()
{
	BYTE bRes = NO;
	if(m_vidMode == H264)
	{
		bRes = m_H264VidMode.IsCapableOfHD720_15fps();
	}
	return bRes;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CVidMode::IsCapableOfHD1080_15fps()
{
	BYTE bRes = NO;
	if(m_vidMode == H264)
	{
		bRes = m_H264VidMode.IsCapableOfHD1080_15fps();
	}
	return bRes;

}
/////////////////////////////////////////////////////////////////////////////
BYTE CVidMode::IsCapableOfHD720_50fps()
{
	BYTE bRes = NO;
	if(m_vidMode == H264)
	{
		bRes = m_H264VidMode.IsCapableOfHD720_50fps();
	}
	return bRes;

}
/////////////////////////////////////////////////////////////////////////////
void  CVidMode::SetCommonVidMode(const CVidMode& other)
{
  //select the highest commaon mode of this VidMode
  //and the other VidMode passed as a parameter

  //default common video mode
    WORD vidMode        = Video_Off;
    WORD cifMpi         = V_4_29_97;
    WORD qcifMpi        = V_4_29_97;
    WORD vidImageFormat = V_Qcif;
    WORD isFreeBitRate  = 0;
    WORD h263Mpi        = MPI_1;

    if ( m_vidMode == H261 && other. m_vidMode == H261 )  vidMode = H261;
    if ( m_vidMode == H263 && other. m_vidMode == H263 )  vidMode = H263;
    if ( m_isFreeBitRate && other. m_isFreeBitRate )  isFreeBitRate  = 1;

    if( vidMode == H261 ) {
        if  (( m_vidImageFormat  == V_Qcif) || (other.m_vidImageFormat == V_Qcif))  {
            // the common format is QCIF
            vidImageFormat = V_Qcif;
            if ( m_qcifMpi < other.m_qcifMpi )
                qcifMpi = other.m_qcifMpi;
            else
                qcifMpi = m_qcifMpi;
        }  else  {
            //the common format is CIF
            vidImageFormat = V_Cif;
            if ( m_cifMpi < other.m_cifMpi )
                cifMpi = other.m_cifMpi;
            else
                cifMpi = m_cifMpi;

            if ( m_qcifMpi < other.m_qcifMpi )
                qcifMpi = other.m_qcifMpi;
            else
                qcifMpi = m_qcifMpi;
        }
    }
    if ( vidMode == H263 ) {
        if( m_vidImageFormat <= other.m_vidImageFormat ) {
            vidImageFormat = m_vidImageFormat;
            h263Mpi = m_h263Mpi;
            /*
             * if(m_h263Mpi >= other.m_h263Mpi)
                h263Mpi = m_h263Mpi;
            else
                h263Mpi = other.m_h263Mpi;
                */
        }  else  {
            vidImageFormat = other.m_vidImageFormat;
            h263Mpi = other.m_h263Mpi;
            /*
            if(other.m_h263Mpi >= m_h263Mpi)
                h263Mpi = other.m_h263Mpi;
            else
                h263Mpi = m_h263Mpi;
                */
        }
    }

//set common parmeters
    m_vidMode        = vidMode;
    m_cifMpi         = cifMpi;
    m_qcifMpi        = qcifMpi;
    m_vidImageFormat = vidImageFormat;
    m_isFreeBitRate  = isFreeBitRate;
    SetH263Mpi(h263Mpi);
}
/////////////////////////////////////////////////////////////////////////////
void  CVidMode::SetCommonVidFormat(const CVidMode& other)
{
  //select the highest common Format of this VidMode
  //and the other VidMode passed as a parameter

  //default common video Format
    WORD vidMode        = Video_Off;
    WORD vidImageFormat = V_Qcif;
    WORD isFreeBitRate  = 0;

    if ( m_vidMode == H261 && other. m_vidMode == H261 )  vidMode = H261;
    if ( m_vidMode == H263 && other. m_vidMode == H263 )  vidMode = H263;
    if ( m_isFreeBitRate && other. m_isFreeBitRate )  isFreeBitRate  = 1;

    if( vidMode == H261 ) {
        if  (( m_vidImageFormat  == V_Qcif) || (other.m_vidImageFormat == V_Qcif))  {
            // the common format is QCIF
            vidImageFormat = V_Qcif;

        }  else  {
            //the common format is CIF
            vidImageFormat = V_Cif;
        }
    }
    if ( vidMode == H263 ) {
        if( m_vidImageFormat <= other.m_vidImageFormat ) {
            vidImageFormat = m_vidImageFormat;
        }  else  {
            vidImageFormat = other.m_vidImageFormat;
        }
    }

//set common parmeters
    m_vidMode        = vidMode;
    m_vidImageFormat = vidImageFormat;
    m_isFreeBitRate  = isFreeBitRate;

}
/////////////////////////////////////////////////////////////////////////////


void  CVidMode::SetCommonMPI(CCapH320* pCap)
{
	WORD newMPI=0;
	    if( m_vidMode == H261 )
		{
			switch ( m_vidImageFormat ) {
			case V_Qcif  :  {
			    newMPI = pCap->GetH261CapMpi(V_Qcif);//m_qCifMpi;
				if ( m_qcifMpi < newMPI )
					m_qcifMpi = newMPI;
				break;
					}
			case V_Cif  :  {
				newMPI = pCap->GetH261CapMpi(V_Cif);//m_cifMpi;
				if ( m_cifMpi < newMPI )
					m_cifMpi = newMPI;

				WORD new_h261QcifMpi;
				new_h261QcifMpi = pCap->GetH261CapMpi(V_Qcif);//m_qCifMpi;
				if ( m_qcifMpi < new_h261QcifMpi )
					m_qcifMpi = new_h261QcifMpi;

				break;
				   }
			default   :  {
				break;
				 }
			}
		}
		if ( m_vidMode == H263 )
		{
			newMPI = pCap->GetCapH263()->GetFormatMPI(m_vidImageFormat);
			if ( m_h263Mpi < newMPI )
				SetH263Mpi(newMPI);
		}

}
/////////////////////////////////////////////////////////////////////////////
WORD  operator==(const CVidMode& rVidMode_1,const CVidMode& rVidMode_2)
{
    WORD    rval = 0;
    /**
    if ( rVidMode_1.m_vidMode  == rVidMode_2.m_vidMode       &&
            rVidMode_1.m_vidImageFormat == rVidMode_2.m_vidImageFormat )
        rval = 1;
    **/

    if ( rVidMode_1.m_vidMode == rVidMode_2.m_vidMode  )
	    rval = 1;

    return rval;
}
/////////////////////////////////////////////////////////////////////////////
WORD  operator!=(const CVidMode& rVidMode_1,const CVidMode& rVidMode_2)
{
    WORD    rval = 1;

    if ( rVidMode_1.m_vidMode == rVidMode_2.m_vidMode  )
    {
    	switch(rVidMode_1.m_vidMode)
    	{
    		case H264:
    		{
    			CH264VidMode  rH264VidMode_1 = rVidMode_1.m_H264VidMode;
    			CH264VidMode  rH264VidMode_2 = rVidMode_2.m_H264VidMode;

    			rval = rH264VidMode_1.IsDiffLevelOrParam(rH264VidMode_2);
    			break;
    		}
    		case H263:
    		{
    			rval = (rVidMode_1.m_h263Mpi != rVidMode_2.m_h263Mpi) ||
    			       (rVidMode_1.m_vidImageFormat != rVidMode_2.m_vidImageFormat);
    			break;
    		}
    		case H261:
    		{
    			rval = (rVidMode_1.m_cifMpi != rVidMode_2.m_cifMpi) ||
    			       (rVidMode_1.m_qcifMpi != rVidMode_2.m_qcifMpi);
    			break;
    		}
    	}
    }


    return rval;
}
/////////////////////////////////////////////////////////////////////////////
void  CVidMode::CreateSipOptions(CCommConf* pCommConf,BYTE videoProtocol)
{
	// set free bit rate
	BYTE video_session = pCommConf->GetVideoSession();
	BYTE is_entry_q = pCommConf->GetEntryQ();
	BYTE eq_video_session = pCommConf->GetEQVideoSession();
	m_isFreeBitRate = 1;
	if(video_session==VIDEO_SWITCH || (is_entry_q && eq_video_session==VIDEO_SWITCH))
	{
		m_isFreeBitRate = 0;
	}

	BYTE isAutoScm = 0;
	BYTE confVideoFormat = pCommConf->GetVideoPictureFormat();
	if(videoProtocol == AUTO || confVideoFormat == AUTO)
	{
		isAutoScm = 1;
	}

	BYTE resrvationVideoProtocol = pCommConf->GetVideoProtocol();
	if(videoProtocol==AUTO && resrvationVideoProtocol!= AUTO)
	{
		videoProtocol=resrvationVideoProtocol;
	}

	BYTE h263Format = TranslateFormatValueFromH261toH263(confVideoFormat);
	BYTE h263mpi = pCommConf->GetQCIFframeRate() - 22;

	switch(videoProtocol) {
	case H261:{
		SetH261VidMode(isAutoScm,pCommConf->GetVideoPictureFormat(),pCommConf->GetQCIFframeRate(),pCommConf->GetCIFframeRate());
		break;
			  }
	case H263:{
		// we set h263 video mode as in fuction SetH263FormatAndMpiFromConfReservation(....)
		// with AUTO video format
		SetH263VidMode(isAutoScm,h263Format,h263mpi);
		break;
			  }
	case H264:
		{
			if(m_isFreeBitRate == 0)
			{
				SetH264DefaultVswVidMode();
			}else{
				// for transcode
				SetH264VidMode(1, H264_CP_TR_DEC_CIF_LEVEL, MAX_BR_IN_H264_CP);
			}
			break;
		}
	case AUTO:
		{
			// set from reservation
			SetH261VidMode(m_isAutoVidScm,V_Cif,V_1_29_97,V_1_29_97);
			break;
		}
	default:{
		PTRACE(eLevelError,"CreateSipOptions: VideoProtocol value is not specified vid mode set to H261");
		//set max video scm (cif/qcif 30)
		SetH261VidMode(m_isAutoVidScm,V_Cif,V_1_29_97,V_1_29_97);
		break;
			}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CVidMode::TranslateFormatValueFromH261toH263(BYTE h261VideoFormat)
{
	BYTE h263Format = H263_CIF;
	switch(h261VideoFormat)
	{
		case V_Qcif :		{  h263Format = H263_QCIF_SQCIF;	break;	}
		case V_Cif :		{  h263Format = H263_CIF;			break;	}
		case H263_CIF_4 :	{  h263Format = H263_CIF_4;			break;	}
		case H263_CIF_16 :	{  h263Format = H263_CIF_16;		break;	}
		case VGA :			{  h263Format = VGA;				break;	}
		case NTSC :			{  h263Format = NTSC;				break;	}
		case SVGA :			{  h263Format = SVGA;				break;	}
		case XGA :			{  h263Format = XGA;				break;	}
		case AUTO :			{  h263Format = AUTO;				break; }
		default :			{  PASSERT(h261VideoFormat);		break;  }
	}
	return h263Format;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CVidMode::GetCPResourceVideoPartyType()
{
	eVideoPartyType videoPartyType = eVideo_party_type_none;
	if(m_isFreeBitRate)
	{
		if(m_vidMode == H264)
		{
			videoPartyType = m_H264VidMode.GetCPH264ResourceVideoPartyType();
		}
		if(m_vidMode == H263)
		{
		  BYTE is4CIF = NO;
			if(m_vidImageFormat>=H263_CIF_4)
			{
			  is4CIF = YES;
			  //	videoPartyType = GetCpH2634CifVideoPartyType();
			}
			videoPartyType = GetH261H263ResourcesPartyType(is4CIF);	
		}

	}
	else //VSW
	{
		PTRACE(eLevelError,"CVidMode::GetResourceCPVideoPartyTyp not CP call");

	}
	return videoPartyType;
}
/////////////////////////////////////////////////////////////////////////////
CH264VidMode::CH264VidMode()          // constructor
{
   m_profileValue = 0xFF;
   m_levelValue = 0xFF;

   m_MaxMBPS = 0xFFFFFFFF;
   m_MaxFS   = 0xFFFFFFFF;
   m_MaxDPB  = 0xFFFFFFFF;
   m_MaxBR   = 0xFFFFFFFF;
   m_MaxCPB  = 0xFFFFFFFF;
   m_Sar	 = H264_ALL_LEVEL_DEFAULT_SAR;
}

/////////////////////////////////////////////////////////////////////////////
CH264VidMode::~CH264VidMode()        // destructor
{
}

////////////////////////////////////////////////////////////////////////////
const char*   CH264VidMode::NameOf()  const
{
    return "CH264VidMode";
}

/////////////////////////////////////////////////////////////////////////////
void  CH264VidMode::DeSerialize(WORD format,CSegment& seg)
{
    DWORD tmp;

	switch ( format )  {

        case SERIALEMBD : {
            break;
        }
        case NATIVE     : {
            seg >> m_profileValue
				>> m_levelValue;
			seg >> tmp;
			if (tmp!= 0xFFFFFFFF)
			  m_MaxMBPS = tmp*CUSTOM_MAX_MBPS_FACTOR;
			else
			  m_MaxMBPS = tmp;

			seg >> tmp;
			if (tmp!= 0xFFFFFFFF)
			  m_MaxFS = tmp*CUSTOM_MAX_FS_FACTOR;
			else
			  m_MaxFS = tmp;

			seg >> tmp;
			if (tmp!= 0xFFFFFFFF)
			  m_MaxDPB = tmp*CUSTOM_MAX_DPB_FACTOR;
			else
			  m_MaxDPB = tmp;

			seg >> tmp;
			if (tmp!= 0xFFFFFFFF)
			  m_MaxBR = tmp*CUSTOM_MAX_BR_FACTOR;
			else
			  m_MaxBR = tmp;

			seg >> tmp;
			if (tmp!= 0xFFFFFFFF)
			{

			   CH264Details h264ConstDetails(m_levelValue);
		       DWORD defaultCPBForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_CPB_CODE);
		       DWORD defaultBRForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_BR_CODE);
			  if(defaultBRForLevel == 0)
			  {
				  TRACESTR(eLevelInfoNormal) << "CH264VidMode::DeSerialize - wrong parameters\n"
				  						 << "level: " << m_levelValue << "\n"
				  						 << "profile" << m_profileValue;
				  m_MaxCPB = tmp;
			  }
			  else
			  {
				  m_MaxCPB = (tmp*CUSTOM_MAX_BR_FACTOR*defaultCPBForLevel)/defaultBRForLevel;
				  if(((tmp*CUSTOM_MAX_BR_FACTOR*defaultCPBForLevel) % defaultBRForLevel) >0 )
					  m_MaxCPB+=1;
			  }
			}
			else
			  m_MaxCPB = tmp;

			WORD tmpWord;
			seg >> tmpWord;
			m_Sar = tmpWord;

            break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CH264VidMode::Serialize(WORD format,CSegment& seg)
{

	switch ( format )  {
        case SERIALEMBD : {

		    break;
        }
        case NATIVE     : {

				 seg << m_profileValue;
                 seg << m_levelValue;
				 if (m_MaxMBPS!=0xFFFFFFFF)
					seg << m_MaxMBPS/CUSTOM_MAX_MBPS_FACTOR;
				 else seg << m_MaxMBPS;

				 if (m_MaxFS!=0xFFFFFFFF)
					seg << m_MaxFS/CUSTOM_MAX_FS_FACTOR;
				 else seg << m_MaxFS;

				 if (m_MaxDPB!=0xFFFFFFFF)
					seg << m_MaxDPB/CUSTOM_MAX_DPB_FACTOR;
				 else seg << m_MaxDPB;

				 if (m_MaxBR!=0xFFFFFFFF)
					seg << m_MaxBR/CUSTOM_MAX_BR_FACTOR;
				 else seg << m_MaxBR;

				 if (m_MaxCPB!=0xFFFFFFFF)
				 {

					CH264Details h264ConstDetails(m_levelValue);
					DWORD defaultCPBForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_CPB_CODE);
					DWORD defaultBRForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_BR_CODE);
			    	DWORD cpb;
					DWORD cpbMod;

			        cpb = m_MaxCPB * defaultBRForLevel / (CUSTOM_MAX_BR_FACTOR*defaultCPBForLevel);
					cpbMod = (m_MaxCPB * defaultBRForLevel) % (CUSTOM_MAX_BR_FACTOR*defaultCPBForLevel);
					if (cpbMod > 0) cpb+=1;

					seg << cpb;
				 }
				 else seg << m_MaxCPB;

				 seg << (WORD)m_Sar;
				 break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
BYTE CH264VidMode::operator<=(const CH264VidMode& vidMode) const
{
    BYTE    rval = 0;

	if (m_levelValue <= vidMode.GetLevelValue()) //Should be preformed on otherparams too???
		rval = 1;


    return rval;
}
/*
/////////////////////////////////////////////////////////////////////////////
BYTE CH264VidMode::operator==(const CH264VidMode& other) const
{

	if ((m_levelValue == other.GetLevelValue()) ||
	   (m_MaxMBPS == other.GetMaxMBPS()) ||
	   (m_MaxFS == other.GetMaxFS()) ||
	   (m_MaxDPB == other.GetMaxDPB()) ||
	   (m_MaxBR == other.GetMaxBR())||
	   (m_MaxCPB == other.GetMaxCPB()) )

	return YES;

	else return NO;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VidMode::IsDiffLevelOrParam(CH264VidMode other) const
{

	if ((m_levelValue != other.GetLevelValue()) ||
	   (m_MaxMBPS != other.GetMaxMBPS()) ||
	   (m_MaxFS != other.GetMaxFS()) ||
	   (m_MaxDPB != other.GetMaxDPB()) ||
	   (m_MaxBR != other.GetMaxBR()) ||
	   (m_MaxCPB != other.GetMaxCPB()) )

	return YES;

	else return NO;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VidMode::IsDiffPLF(CH264VidMode other) const
{

	if ((m_levelValue != other.GetLevelValue()) ||
	   (m_MaxMBPS != other.GetMaxMBPS()) ||
	   (m_MaxFS != other.GetMaxFS()))

	return YES;

	else return NO;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
char*  CH264VidMode::GetLevelAsString() const
{
	switch(m_levelValue)
	{
		case H264_Level_1   :	return "H264_Level_1";
		case H264_Level_1_1 :	return "H264_Level_1_1";
		case H264_Level_1_2 :	return "H264_Level_1_2";
		case H264_Level_1_3 :	return "H264_Level_1_3";
		case H264_Level_2   :	return "H264_Level_2";
		case H264_Level_2_1   :	return "H264_Level_2_1";
		case H264_Level_2_2   :	return "H264_Level_2_2";
		case H264_Level_3   :	return "H264_Level_3";
		case H264_Level_3_1   :	return "H264_Level_3_1";
		case H264_Level_3_2   :	return "H264_Level_3_2";
		case H264_Level_4   :	return "H264_Level_4";
		case H264_Level_4_1   :	return "H264_Level_4_1";
		case H264_Level_4_2   :	return "H264_Level_4_2";
		case H264_Level_5   :	return "H264_Level_5";
		case H264_Level_5_1   :	return "H264_Level_5_1";

		default:
			{
				DBGPASSERT(m_levelValue);
				return "Unexpected Level";
			}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH264VidMode::IsLevelValueIsLegal() const
{
	switch(m_levelValue)
	{
		case H264_Level_1   :
		case H264_Level_1_1 :
		case H264_Level_1_2 :
		case H264_Level_1_3 :
		case H264_Level_2   :
		case H264_Level_2_1 :
		case H264_Level_2_2 :
		case H264_Level_3   :
		case H264_Level_3_1 :
		case H264_Level_3_2 :
		case H264_Level_4   :
		case H264_Level_4_1 :
		case H264_Level_4_2 :
		case H264_Level_5   :
		case H264_Level_5_1 :	return TRUE;

		default:
		{
			return FALSE;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CH264VidMode::CalcAndSetCommonLevelAndParam(CH264VidMode other)
{
  //select the highest common Level and Custom Params of this VidMode
  //and the other VidMode that passed as a parameter

	PTRACE(eLevelInfoNormal,"CH264VidMode::CalcAndSetCommonLevelAndParam");
	BYTE original_m_levelValue = m_levelValue;

	if( m_levelValue == other.m_levelValue )
	{

		m_MaxMBPS     = CalcCommonParamSameLevel(m_MaxMBPS,other.m_MaxMBPS);
		m_MaxFS       = CalcCommonParamSameLevel(m_MaxFS,other.m_MaxFS);
		m_MaxDPB      = CalcCommonParamSameLevel(m_MaxDPB,other.m_MaxDPB);
		m_MaxBR       = CalcCommonParamSameLevel(m_MaxBR,other.m_MaxBR);
		m_MaxCPB      = CalcCommonParamSameLevel(m_MaxCPB,other.m_MaxCPB);
	}

	else if ( m_levelValue != other.m_levelValue )
	{

		if (other.m_levelValue < m_levelValue)
			m_levelValue = other.m_levelValue;

		{

			DWORD maxMBPS = CalcCommonParamDiffLevel(m_MaxMBPS,original_m_levelValue,other.m_MaxMBPS,other.m_levelValue,CUSTOM_MAX_MBPS_CODE);
			CH264Details h264ConstDetails(m_levelValue);
			if (h264ConstDetails.IsParamSameAsDefaultForLevel(CUSTOM_MAX_MBPS_CODE, maxMBPS, eCustomProduct))
				m_MaxMBPS = 0xFFFFFFFF;
			else m_MaxMBPS = maxMBPS;

			DWORD maxFS = CalcCommonParamDiffLevel(m_MaxFS,original_m_levelValue,other.m_MaxFS,other.m_levelValue,CUSTOM_MAX_FS_CODE);
			if (h264ConstDetails.IsParamSameAsDefaultForLevel(CUSTOM_MAX_FS_CODE, maxFS, eCustomProduct))
				m_MaxFS = 0xFFFFFFFF;
			else m_MaxFS = maxFS;

			DWORD maxDPB = CalcCommonParamDiffLevel(m_MaxDPB,original_m_levelValue,other.m_MaxDPB,other.m_levelValue,CUSTOM_MAX_DPB_CODE);
			if (h264ConstDetails.IsParamSameAsDefaultForLevel(CUSTOM_MAX_DPB_CODE, maxDPB, eCustomProduct))
				m_MaxDPB = 0xFFFFFFFF;
			else m_MaxDPB = maxDPB;

			DWORD maxBR = CalcCommonParamDiffLevel(m_MaxBR,original_m_levelValue,other.m_MaxBR,other.m_levelValue,CUSTOM_MAX_BR_CODE);
			if (h264ConstDetails.IsParamSameAsDefaultForLevel(CUSTOM_MAX_BR_CODE, maxBR, eCustomProduct))
				m_MaxBR = 0xFFFFFFFF;
			else m_MaxBR = maxBR;

			DWORD maxCPB = CalcCommonParamDiffLevel(m_MaxCPB,original_m_levelValue,other.m_MaxCPB,other.m_levelValue,CUSTOM_MAX_CPB_CODE);
			if (h264ConstDetails.IsParamSameAsDefaultForLevel(CUSTOM_MAX_CPB_CODE, maxCPB, eCustomProduct))
				m_MaxCPB = 0xFFFFFFFF;
			else m_MaxCPB = maxCPB;
		}
	}

	m_Sar = min(m_Sar, other.m_Sar);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH264VidMode::CalcCommonParamSameLevel(DWORD firstParam, DWORD secondParam)
{
	DWORD retval = 0xFFFFFFFF;

	if ((firstParam == 0xFFFFFFFF) || (secondParam == 0xFFFFFFFF))
		retval = 0xFFFFFFFF;

	else
	{
		if (secondParam < firstParam)
			retval = secondParam;

		else retval = firstParam;
	}

	return retval;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH264VidMode::GetCustomFactor(BYTE customType)
{

	switch(customType)
	{
		case CUSTOM_MAX_MBPS_CODE :	return (DWORD)CUSTOM_MAX_MBPS_FACTOR;
		case CUSTOM_MAX_FS_CODE : return (DWORD)CUSTOM_MAX_FS_FACTOR;
		case CUSTOM_MAX_DPB_CODE: return (DWORD)CUSTOM_MAX_DPB_FACTOR;
		case CUSTOM_MAX_BR_CODE: return (DWORD)CUSTOM_MAX_BR_FACTOR;
		default:
		{
			return 1;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH264VidMode::CalcCommonParamDiffLevel(DWORD firstParam,BYTE firstLevel,
											DWORD secondParam,BYTE secondLevel,
											BYTE type)
{
	DWORD retval = 0xFFFFFFFF;
	DWORD firstParamConverted = 0;
	DWORD secondParamConverted = 0;


	if (firstParam == 0xFFFFFFFF)
	{
		CH264Details h264ConstDetailsFirstLevel(firstLevel);
		firstParamConverted = h264ConstDetailsFirstLevel.GetParamDefaultValueAsProductForLevel(type);
		 //firstParamConverted = GetDefaultParamForLevel(firstLevel,type);
	}
	else firstParamConverted = firstParam;

	if (secondParam == 0xFFFFFFFF)
	{
		CH264Details h264ConstDetailsSecondLevel(secondLevel);
		secondParamConverted = h264ConstDetailsSecondLevel.GetParamDefaultValueAsProductForLevel(type);
		//secondParamConverted = GetDefaultParamForLevel(secondLevel,type);
	}
	else secondParamConverted = secondParam;


	if (secondParamConverted < firstParamConverted)
		retval = secondParamConverted;

	else retval = firstParamConverted;

	return retval;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VidMode::GetProfileValue() const
{
	return m_profileValue;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetProfileValue(BYTE profile)
{
	m_profileValue = profile;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH264VidMode::GetLevelValue() const
{
	return m_levelValue;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetLevelValue(BYTE level)
{
	m_levelValue = level;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH264VidMode::GetMaxMBPS() const
{
	return m_MaxMBPS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CH264VidMode::GetMaxMBPSasCustomWord() const
{
	 return GetMaxMbpsAsDevision(m_MaxMBPS);
}
/*
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::ConvertMaxMBPS(DWORD mbps)
{
	if (mbps == 0xFFFFFFFF)
		m_MaxMBPS = 0xFFFFFFFF;
	else
		m_MaxMBPS = mbps*CUSTOM_MAX_MBPS_FACTOR;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxMBPS(WORD mbps)
{
	DWORD dw_mbps = ConvertMaxMbpsToProduct(mbps);
	m_MaxMBPS = dw_mbps;

}
////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxMBPS(DWORD mbps)
{
	m_MaxMBPS = mbps;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH264VidMode::GetMaxFS() const
{
	return m_MaxFS;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CH264VidMode::GetMaxFSasCustomWord() const
{
	 return GetMaxFsAsDevision(m_MaxFS);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CH264VidMode::GetCPH264ResourceVideoPartyType()
{
	eVideoPartyType videoPartyType = eVideo_party_type_none;

	WORD thisLevel = m_levelValue;
	CH264Details thisH264Details (thisLevel);
	DWORD thisFS = m_MaxFS;
	DWORD thisMBPS = m_MaxMBPS;
	if (thisFS == 0xFFFFFFFF)
		thisFS = thisH264Details.GetDefaultFsAsProduct();
	if (thisMBPS == 0xFFFFFFFF)
		thisMBPS = thisH264Details.GetDefaultMbpsAsProduct();

	videoPartyType = ::GetCPH264ResourceVideoPartyType(thisFS, thisMBPS);
	PTRACE2(eLevelInfoNormal,"CH264VidMode::GetCPH264ResourceVideoPartyType videoPartyType : ",eVideoPartyTypeNames[videoPartyType] );

	return videoPartyType;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void CH264VidMode::ConvertMaxFS(DWORD fs)
{
	if (fs == 0xFFFFFFFF)
		m_MaxFS = 0xFFFFFFFF;
	else
		m_MaxFS = fs*CUSTOM_MAX_FS_FACTOR;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxFS(WORD fs)
{
	DWORD dw_fs = 0xFFFFFFFF;

	if (fs != 0xFFFF)
		dw_fs = (DWORD)fs;

	ConvertMaxFS(dw_fs);
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxFS(WORD fs)
{

	DWORD dw_fs = ConvertMaxFsToProduct(fs);
	m_MaxFS = dw_fs;

}


////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxFS(DWORD fs)
{
	m_MaxFS = fs;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH264VidMode::GetMaxDPB() const
{
	return m_MaxDPB;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CH264VidMode::GetMaxDPBasCustomWord() const
{
 return GetMaxDpbAsDevision(m_MaxDPB);
}
/*
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::ConvertMaxDPB(DWORD dpb)
{
	if (dpb == 0xFFFFFFFF)
		m_MaxDPB = 0xFFFFFFFF;
	else
		m_MaxDPB = dpb*CUSTOM_MAX_DPB_FACTOR;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxDPB(WORD dpb)
{
	DWORD dw_dpb = 0xFFFFFFFF;

	if (dpb != 0xFFFF)
		dw_dpb = (DWORD)dpb;

	ConvertMaxDPB(dw_dpb);
}
*/
void CH264VidMode::SetMaxDPB(WORD dpb)
{

	DWORD dw_dpb = ConvertMaxDpbToProduct(dpb);
	m_MaxDPB = dw_dpb;

}
///////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxDPB(DWORD dpb)
{
	m_MaxDPB = dpb;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH264VidMode::GetMaxBR() const
{
	return m_MaxBR;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CH264VidMode::GetMaxBRasCustomWord() const
{

 return GetMaxBrAsDevision(m_MaxBR);

}
/*
WORD CH264VidMode::GetMaxBRasCustomWord() const
{

	WORD w_br = 0xFFFF;
	if (m_MaxBR != 0xFFFFFFFF)
	{
		w_br = m_MaxBR/CUSTOM_MAX_BR_FACTOR;
		if ((m_MaxBR % CUSTOM_MAX_BR_FACTOR) > 0)
			w_br+=1;
	}
	return w_br;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::ConvertMaxBR(DWORD br)
{
	if (br == 0xFFFFFFFF)
		m_MaxBR = 0xFFFFFFFF;
	else
		m_MaxBR = br*CUSTOM_MAX_BR_FACTOR;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxBR(WORD br)
{
	DWORD dw_br = 0xFFFFFFFF;

	if (br != 0xFFFF)
		dw_br = (DWORD)br;

	ConvertMaxBR(dw_br);
}
*/
void CH264VidMode::SetMaxBR(WORD br)
{
	DWORD dw_br = ConvertMaxBrAndCpbToMaxBrProduct(br);
	m_MaxBR = dw_br;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxBR(DWORD br)
{
	m_MaxBR = br;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH264VidMode::GetMaxCPB() const
{
	return m_MaxCPB;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetSAR(WORD sar)
{
	m_Sar = sar;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CH264VidMode::GetSAR() const
{
	return m_Sar;
}


/*
We don`t need it since it does not go to the capabilieties
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CH264VidMode::GetMaxCPBasCustomWord() const
{
	WORD w_cpb = 0xFFFF;
	if (m_MaxCPB != 0xFFFFFFFF)
		w_cpb = m_MaxCPB/CUSTOM_MAX_CPB_FACTOR;

	return w_cpb;
}*/

/*
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::ConvertMaxCPB(DWORD br)
{
	if (br == 0xFFFFFFFF)
		m_MaxCPB = 0xFFFFFFFF;
	else
	{

		CH264Details h264ConstDetails(m_levelValue);
		DWORD defaultCPBForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_CPB_CODE);
		DWORD defaultBRForLevel = h264ConstDetails.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_BR_CODE);
		//DWORD defaultCPBForLevel = GetDefaultParamForLevel(m_levelValue,CUSTOM_MAX_CPB_CODE);
		//DWORD defaultBRForLevel = GetDefaultParamForLevel(m_levelValue,CUSTOM_MAX_BR_CODE);

		m_MaxCPB = (br*CUSTOM_MAX_BR_FACTOR*defaultCPBForLevel)/defaultBRForLevel;
		if(((br*CUSTOM_MAX_BR_FACTOR*defaultCPBForLevel) % defaultBRForLevel)>0)
			m_MaxCPB+=1;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxCPB(WORD cpb)
{
	DWORD dw_cpb = 0xFFFFFFFF;

	if (cpb != 0xFFFF)
		dw_cpb = (DWORD)cpb;

	ConvertMaxCPB(dw_cpb);
}
*/

void CH264VidMode::SetMaxCPB(WORD cpb)
{

  CH264Details h264constDetails(m_levelValue);
  DWORD dw_cpb = h264constDetails.ConvertMaxBrAndCpbToMaxCpbProduct(cpb);
  m_MaxCPB = dw_cpb;

}

////////////////////////////////////////////////////////////////////////////////////////////////
void CH264VidMode::SetMaxCPB(DWORD cpb)
{
	m_MaxCPB = cpb;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BYTE CH264VidMode::ConvertBitRateToLevel(BYTE confTransferRate)
// {
//     switch (confTransferRate) {

//         case Xfer_64      : return H264_Level_1;
//         case Xfer_2x64    :
//         case Xfer_3x64    : return H264_Level_1_1;
//         case Xfer_4x64    :
//         case Xfer_5x64    :
//         case Xfer_6x64    :
//         case Xfer_384     : return H264_Level_1_2;
//         case Xfer_2x384   : return H264_Level_1_3;
//         case Xfer_3x384   :
//         case Xfer_4x384   :
//         case Xfer_5x384   : return H264_Level_2;
//         case Xfer_128     :
//         case Xfer_192     : return H264_Level_1_1;
//         case Xfer_256     :
//         case Xfer_320     : return H264_Level_1_2;
//         case Xfer_512     :
//         case Xfer_768     : return H264_Level_1_3;
//         case Xfer_1152    :
//         case Xfer_1472    :
//         case Xfer_1536    :
//         case Xfer_1920    : return H264_Level_2;
//         default : {
//           	ALLOCBUFFER(Mess,ONE_LINE_BUFFER_LEN);
//             sprintf(Mess,"CH264VidMode::ConvertBitRateToLevel - BAD ARGUMENT <%d>",confTransferRate);
//             PTRACE(eLevelError,Mess);
// 			DEALLOCBUFFER(Mess);
// 			if(confTransferRate)
// 				PASSERT(confTransferRate);
// 			else
// 				PASSERT(101);
// 			return H264_Level_1;
//         }
//     }
// }

///////////////////////////////////////////////////////////////////////////////////////////////////
// void  CH264VidMode::SetCommonLevel(CCapH320* pCap)
// {
// 	for(WORD i=0;i<pCap->GetCapH264().GetNumberOfH264Sets();i++)
// 	{
// 		BYTE newLevel = pCap->GetCapH264().GetCapSetH264(i)->GetCapH264LevelValue();
// 		if ( m_levelValue < newLevel )
// 				m_levelValue = newLevel;
// 	}
// }


/////////////////////////////////////////////////////////////////////////////
COtherMode::COtherMode()              // constructor
{
    m_H06Bcomp      = Not_B6_H0;
    m_encryption    = Encryp_Off;
    m_restrict      = Derestrict;
    m_loop          = Loop_Off;
}


/////////////////////////////////////////////////////////////////////////////
COtherMode::~COtherMode()        // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
const char*   COtherMode::NameOf()  const
{
    return "COtherMode";
}

/////////////////////////////////////////////////////////////////////////////
void  COtherMode::SetH06BMode(WORD mode)
{
    m_H06Bcomp = mode;
}

/////////////////////////////////////////////////////////////////////////////
WORD  COtherMode::GetH06BCompMode() const
{
    return m_H06Bcomp;
}

/////////////////////////////////////////////////////////////////////////////
void  COtherMode::SetEncrypMode(WORD mode)
{
    m_encryption = mode;
}

/////////////////////////////////////////////////////////////////////////////
WORD  COtherMode::GetEncrypMode() const
{
    return m_encryption;
}

/////////////////////////////////////////////////////////////////////////////
void  COtherMode::SetRestrictMode(WORD mode)
{
    m_restrict = mode;
}
/////////////////////////////////////////////////////////////////////////////
WORD  COtherMode::GetRestrictMode( ) const
{
    return m_restrict;
}
/////////////////////////////////////////////////////////////////////////////
void  COtherMode::SetLoopMode(WORD mode)
{
    m_loop = mode;
}

/////////////////////////////////////////////////////////////////////////////
WORD  COtherMode::GetLoopMode() const
{
    return m_loop;
}

/////////////////////////////////////////////////////////////////////////////
void  COtherMode::SetMode(WORD mode)
{
    switch ( mode ) {
        case Not_SM_comp  :
        case B6_H0_Comp   :  {
            m_H06Bcomp = mode;
            break;
        }
        case Au_Loop   :
        case Vid_Loop  :
        case Dig_Loop  :
        case Loop_Off  :  {
            m_loop = mode;
            break;
        }
        case Restrict    :
        case Derestrict  :  {
            m_restrict = mode;
            break;
        }
        case Encryp_On  :
        case Encryp_Off :  {
            m_encryption = mode;
            break;
        }
        default   :  {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void  COtherMode::DeSerialize(WORD format,CSegment& seg)
{
    switch ( format )  {

        case SERIALEMBD :{
            break;
        }
        case NATIVE     :{
            seg >> m_H06Bcomp
                >> m_encryption
                >> m_restrict
                >> m_loop;
            break;
        }
        default : {
            break;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
void  COtherMode::Serialize(WORD format,CSegment& seg)
{
    BYTE  H06BcompCmd   = OTHRCMDATTR;
    BYTE  encryptionCmd = OTHRCMDATTR;
    BYTE  restrictCmd   = OTHRCMDATTR;
    BYTE  loopCmd       = OTHRCMDATTR;

    switch ( format )  {

        case SERIALEMBD :{
            H06BcompCmd   = OTHRCMDATTR | m_H06Bcomp;
            encryptionCmd = OTHRCMDATTR | m_encryption;
            restrictCmd   = OTHRCMDATTR | m_restrict;
            loopCmd       = OTHRCMDATTR | m_loop;
            seg << H06BcompCmd
                << encryptionCmd
                << restrictCmd;
                //<< loopCmd;    loop command is considerd as h230 c&i
            break;
        }
        case NATIVE     :{
            seg << m_H06Bcomp
                << m_encryption
                << m_restrict
                << m_loop;
            break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
WORD  COtherMode::operator<=(const CCapH320& cap) const
{
    WORD    rval = 1;

    if ( m_encryption == Encryp_On )  {
        rval = cap.IsDataCap(Encryp_Cap);
        if ( ! rval )
            return rval;
    }

    if ( m_H06Bcomp == B6_H0_Comp )  {
        rval = cap.OnXferCap(e_Xfer_Cap_6B_H0_Comp);
        if ( ! rval )
            return rval;
    }

    if ( m_restrict == Derestrict )  {
        rval = cap.OnXferCap(e_Xfer_Cap_Restrict); // remote can only work at restricted
        if ( rval )
            return 0;
        else
            rval = 1;
    }
    return rval;
}


/////////////////////////////////////////////////////////////////////////////
WORD  operator==(const COtherMode& rOtherMode_1,const COtherMode& rOtherMode_2)
{
    WORD    rval = 0;

    if (    rOtherMode_1.m_H06Bcomp     == rOtherMode_2.m_H06Bcomp   &&
            rOtherMode_1.m_encryption   == rOtherMode_2.m_encryption &&
            rOtherMode_1.m_restrict     == rOtherMode_2.m_restrict   &&
            rOtherMode_1.m_loop         == rOtherMode_2.m_loop )
        rval = 1;

    return rval;
}


/////////////////////////////////////////////////////////////////////////////
CLsdMlpMode::CLsdMlpMode()            // constructor
{
    m_mlp = MLP_Off;
    m_lsd = LSD_Off;
	m_lsdCap = LSD_NONE;
}
/////////////////////////////////////////////////////////////////////////////
CLsdMlpMode::~CLsdMlpMode()        // destructor
{
}


/////////////////////////////////////////////////////////////////////////////
const char*   CLsdMlpMode::NameOf()  const
{
    return "CLsdMlpMode";
}

/////////////////////////////////////////////////////////////////////////////
void  CLsdMlpMode::SetLsdMode(WORD mode)
{
    m_lsd = mode;
}

 /////////////////////////////////////////////////////////////////////////////
void  CLsdMlpMode::SetMlpMode(WORD mode)
{
    m_mlp = mode;
}
/////////////////////////////////////////////////////////////////////////////
void  CLsdMlpMode::SetlsdCap(WORD Cap)
{
    m_lsdCap = Cap;
}
/////////////////////////////////////////////////////////////////////////////
WORD  CLsdMlpMode::GetLsdMode() const
{
    return m_lsd;
}

 /////////////////////////////////////////////////////////////////////////////
WORD  CLsdMlpMode::GetMlpMode() const
{
    return m_mlp;
}
/////////////////////////////////////////////////////////////////////////////
WORD  CLsdMlpMode::GetlsdCap() const
{
    return m_lsdCap;
}
/////////////////////////////////////////////////////////////////////////////
void  CLsdMlpMode::SetMode(WORD mode)
{
    if ( mode < MLP_Off || mode == Data_Var_LSD )
        m_lsd = mode;
    else
        m_mlp = mode;
}

/////////////////////////////////////////////////////////////////////////////
void  CLsdMlpMode::DeSerialize(WORD format,CSegment& seg)
{
    switch ( format )  {
        case SERIALEMBD : {
            break;
        }
        case NATIVE     : {
            seg >> m_lsd
                >> m_mlp
			    >> m_lsdCap;
            break;
        }
        default : {
            break;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
void  CLsdMlpMode::Serialize(WORD format,CSegment& seg)
{
    BYTE  lsdCmd = LSDMLPCMDATTR;
    BYTE  mlpCmd = LSDMLPCMDATTR;

    switch ( format )  {
        case SERIALEMBD : {

	        BYTE    seriableVal = 0;

            lsdCmd = LSDMLPCMDATTR | m_lsd;
            mlpCmd = LSDMLPCMDATTR | m_mlp;

            seg << lsdCmd
                << mlpCmd;

            if(m_lsd == LSD_Off){
/*  because the problem with all of VTEL endpoints
    we don't send H224_LSD_Off command
                seriableVal = (BYTE)Data_Apps_Esc |  (BYTE)ESCAPECAPATTR ;
                seg << seriableVal;
                seriableVal =  (BYTE)(H224_LSD_Off | Attr010);
                seg << seriableVal;
*/
            } else {
                seriableVal = (BYTE)Data_Apps_Esc |  (BYTE)ESCAPECAPATTR ;
                seg << seriableVal;
                seriableVal =  (BYTE)(H224_LSD_On | Attr011);
                seg << seriableVal;
            }
            break;
        }
        case NATIVE     : {
            seg << m_lsd
                << m_mlp
				<< m_lsdCap;
            break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
WORD    CLsdMlpMode::operator<=(const CCapH320& cap) const
{
    WORD  rval = 1;     // m_lsd = LSD_Off && m_mlp = MLP_Off

    /*if ( m_lsd != LSD_Off && m_lsd != Data_Var_LSD )  {
        for ( WORD i = m_lsd ; i <= LSD_64k ; i++ ) {   // we assume that data bitrate capbilty is ordered by value
            rval = cap.OnDataVidCap(i);
            if ( rval )
                i = LSD_64k + 1;
        }
    }*/

    if ( m_lsd != LSD_Off && m_lsd != Data_Var_LSD )
        rval = cap.IsDataCap(m_lsd);

    if ( ! rval )
        return rval;

    if ( m_lsd == Data_Var_LSD )
        rval = cap.IsDataCap(Ter2_Var_Lsd);

    if ( ! rval )
        return rval;

    switch ( m_mlp ) {

        case MLP_Off  :  {
            rval = 1;
            break;
        }

        case MLP_4k :  {
            rval = cap.IsDataCap(Dxfer_Cap_Mlp_4k);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_1);
            break;
        }

        case Data_var_MLP   :  {
            rval = cap.IsDataCap(Var_Mlp);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case MLP_6_4k   :  {
            rval = cap.IsDataCap(Dxfer_Cap_Mlp_6_4k);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_1);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_14_4   :  {
            rval = cap.IsMlpCap(Mlp_Cap_14_4);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_1);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_32   : {
            rval = cap.IsMlpCap(Mlp_Cap_32);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_1);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_40   :  {
            rval = cap.IsMlpCap(Mlp_Cap_40);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_1);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_22_4  : {
            rval = cap.IsMlpCap(Mlp_Cap_22_4);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_30_4  : {
            rval = cap.IsMlpCap(Mlp_Cap_30_4);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_38_4  : {
            rval = cap.IsMlpCap(Mlp_Cap_38_4);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_46_4  : {
            rval = cap.IsMlpCap(Mlp_Cap_46_4);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_16   : {
            rval = cap.IsMlpCap(Mlp_Cap_16);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_24   : {
            rval = cap.IsMlpCap(Mlp_Cap_24);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        case Mlp_62_4   : {
            rval = cap.IsMlpCap(Mlp_Cap_62_4);
            if ( ! rval )
                rval = cap.IsDataCap(Mlp_Set_2);
            break;
        }
        default  :  {
            break;
        }
    }
    return rval;
}

/////////////////////////////////////////////////////////////////////////////
WORD  operator==(const CLsdMlpMode& rLsdMlpMode_1,const CLsdMlpMode& rLsdMlpMode_2)
{
    WORD    rval = 0;

    if ( rLsdMlpMode_1.m_mlp == rLsdMlpMode_2.m_mlp &&
         rLsdMlpMode_1.m_lsd == rLsdMlpMode_2.m_lsd  )
        rval = 1;

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
CHsdHmlpMode::CHsdHmlpMode()          // constructor
{
    m_hMlp = H_Mlp_Off_Com;
    m_hsd  = Hsd_Com_Off;
}


/////////////////////////////////////////////////////////////////////////////
CHsdHmlpMode::~CHsdHmlpMode()        // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
const char*   CHsdHmlpMode::NameOf()  const
{
    return "CHsdHmlpMode";
}

/////////////////////////////////////////////////////////////////////////////
void  CHsdHmlpMode::SetHmlpMode(WORD mode)
{
    m_hMlp = mode;
}

/////////////////////////////////////////////////////////////////////////////
void  CHsdHmlpMode::SetHsdMode(WORD mode)
{
    m_hsd = mode;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CHsdHmlpMode::GetHmlpMode() const
{
    return m_hMlp;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CHsdHmlpMode::GetHsdMode() const
{
    return m_hsd;
}

/////////////////////////////////////////////////////////////////////////////
void  CHsdHmlpMode::SetMode(WORD mode)
{
    if ( mode > H_Mlp_Off_Com || mode < H_Mlp_Com_62_4)
        m_hsd = mode;
    else
        m_hMlp = mode;
}

/////////////////////////////////////////////////////////////////////////////
void  CHsdHmlpMode::DeSerialize(WORD format,CSegment& seg)
{
    switch ( format )  {
        case SERIALEMBD : {
            break;
        }
        case NATIVE     : {
            seg >> m_hsd
                >> m_hMlp;
            break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CHsdHmlpMode::Serialize(WORD format,CSegment& seg)
{
    BYTE  hsdCmd  = HSDHMLPCMDATTR;
    BYTE  hMlpCmd = HSDHMLPCMDATTR;
    BYTE  escHsd  = ESCAPECAPATTR;

    switch ( format )  {
        case SERIALEMBD : {
            hsdCmd  =  HSDHMLPCMDATTR | m_hsd;
            hMlpCmd =  HSDHMLPCMDATTR | m_hMlp;
            escHsd  |= Hsd_Esc;
            seg << escHsd
                << hsdCmd
                << escHsd
                << hMlpCmd;
            break;
        }
        case NATIVE     : {
            seg << m_hsd
                << m_hMlp;
            break;
        }
        default : {
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
WORD    CHsdHmlpMode::operator<=(const CCapH320& cap) const
{
    WORD    rval = 1;  // m_hsd = HSD-off && m_hMlp = H_Mlp_Off_Com

    if ( m_hsd != Hsd_Com_Off )
        rval = cap.IsHsdHmlpCap(m_hsd);

    if ( ! rval )
        return rval;

    if ( m_hMlp != H_Mlp_Off_Com )
        rval = cap.IsHsdHmlpCap(m_hMlp);

    return rval;
}

/////////////////////////////////////////////////////////////////////////////
WORD  operator==(const CHsdHmlpMode& rHsdHmlpMode_1,const CHsdHmlpMode& rHsdHmlpMode_2)
{
    WORD    rval = 0;

    if ( rHsdHmlpMode_1.m_hMlp == rHsdHmlpMode_2.m_hMlp &&
         rHsdHmlpMode_1.m_hsd  == rHsdHmlpMode_2.m_hsd  )
        rval = 1;

    return rval;
}


/////////////////////////////////////////////////////////////////////////////
CNonStandardMode::CNonStandardMode() :
	m_isContentEnabled(0), m_isVisualConcertPC(0), m_isVisualConcertFX(0),
	m_isDuoVideo(0)
{
}

/////////////////////////////////////////////////////////////////////////////
CNonStandardMode::CNonStandardMode( const WORD isContentEnabled,
		const WORD isVisualConcertPc, const WORD isVisualConcertFx,
		const WORD isDuoVideo )
{
	SetPeopleContent(isContentEnabled);
	SetVisualConcertPC(isVisualConcertPc);
	SetVisualConcertFX(isVisualConcertFx);
	SetDuoVideo(isDuoVideo);
}

/////////////////////////////////////////////////////////////////////////////
CNonStandardMode::~CNonStandardMode()
{
}

/////////////////////////////////////////////////////////////////////////////
const char*	CNonStandardMode::NameOf() const
{
	return "CNonStandardMode";
}

/////////////////////////////////////////////////////////////////////////////
void CNonStandardMode::SetPeopleContent(const WORD mode)
{
	m_isContentEnabled = (mode)? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
void CNonStandardMode::SetVisualConcertPC(const WORD newVisualConcertPcMode)
{
	m_isVisualConcertPC = (newVisualConcertPcMode)? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
void CNonStandardMode::SetVisualConcertFX(const WORD newVisualConcertFxMode)
{
	m_isVisualConcertFX = (newVisualConcertFxMode)? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
void CNonStandardMode::SetDuoVideo(const WORD newDuoVideo)
{
	m_isDuoVideo = (newDuoVideo)? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
void CNonStandardMode::DeSerialize(WORD format,CSegment& seg)
{
	WORD  cap = 0x0000;

	switch ( format )  {
		case SERIALEMBD :{
			break;
		}
		case NATIVE     :{
			seg >> cap;
			SetPeopleContent(cap);
			seg >> cap;
			SetVisualConcertPC(cap);
			seg >> cap;
			SetVisualConcertFX(cap);
			seg >> cap;
			SetDuoVideo(cap);
			break;
		}
		default : {
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CNonStandardMode::Serialize(WORD format,CSegment& seg)
{
	switch ( format )  {
		case SERIALEMBD :{
			break;
		}
		case NATIVE     :{
			seg << m_isContentEnabled
				<< m_isVisualConcertPC
				<< m_isVisualConcertFX
				<< m_isDuoVideo;
			break;
		}
		default : {
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
WORD operator==(const CNonStandardMode& rNsMode1,const CNonStandardMode& rNsMode2)
{
    if( rNsMode1.m_isContentEnabled == rNsMode2.m_isContentEnabled )
//        if( rNsMode1.m_isDualStreams == rNsMode2.m_isDualStreams )
        return 1;
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
BYTE CComMode::GetVideoBitrateInB(WORD channelWidthForCalc)
{
    DWORD audio_bitrate, video_bitrate, lsd_bitrate;
    DWORD hsd_bitrate, mlp_bitrate, hmlp_bitrate;
    DWORD content_bitrate;

    const DWORD FAS_WIDTH=800;
    const DWORD BAS_WIDTH=800;

    DWORD callWidth;
    DWORD FasBasWidth;

    //the data bitrates are currently not supported
    lsd_bitrate = 0;
    hsd_bitrate = 0;
    mlp_bitrate = 0;
    hmlp_bitrate = 0;
    content_bitrate = 0;

    DWORD chnlWidth;
    long firstChnlLeftWidth;
    DWORD other_bitrate;
	DWORD EcsWidth = 0;

	if (channelWidthForCalc)
	{
		callWidth = channelWidthForCalc * 64 * 1000;
		FasBasWidth = (FAS_WIDTH + BAS_WIDTH);
	}
	else
	{
		callWidth = m_xferMode.GetNumChnl() * m_xferMode.GetChnlWidth() * 64 * 1000;
		FasBasWidth = m_xferMode.GetNumChnl() * (FAS_WIDTH + BAS_WIDTH);
	}

    chnlWidth = 64 * 1000;

    //update the bitrate field in m_audMode according to the audio algoritem
    m_audMode.SetBitRate(m_audMode.GetAudMode());
    audio_bitrate = m_audMode.GetBitRate() * 1 * 1000;
    CCDRUtils::Get_Hsd_Hmlp_command_Bitrate((BYTE)GetHmlpMode(),(WORD *)&hmlp_bitrate);
    hmlp_bitrate *= 100;
    CCDRUtils::Get_Lsd_Mlp_Command_BitRate((BYTE)GetLsdMode(),(WORD *)&lsd_bitrate);
    lsd_bitrate *= 100;
    CCDRUtils::Get_Lsd_Mlp_Command_BitRate((BYTE)GetMlpMode(),(WORD *)&mlp_bitrate);
    mlp_bitrate *= 100;
    CCDRUtils::Get_Content_command_Bitrate(GetContentModeContentRate(),(WORD*)&content_bitrate);
    content_bitrate *= 100;

    if (m_otherMode.GetRestrictMode() == Restrict ) {
        chnlWidth = (chnlWidth * 7) / 8;
        callWidth = (callWidth * 7) / 8;
        if( audio_bitrate == 56000 )
            audio_bitrate = 48000;
        content_bitrate = (content_bitrate * 7) / 8;
    }
	if(m_otherMode.GetEncrypMode()==Encryp_On)
		EcsWidth = 800;
    // mlp - check validity on the first channel
    if( mlp_bitrate ) {
        firstChnlLeftWidth = chnlWidth - (FAS_WIDTH + BAS_WIDTH) - audio_bitrate  - mlp_bitrate - EcsWidth;
        if( firstChnlLeftWidth < 0 ) {
            video_bitrate = 0;
            return 0;
        }
    }

    other_bitrate = FasBasWidth + audio_bitrate
        + lsd_bitrate + hsd_bitrate + mlp_bitrate + hmlp_bitrate + content_bitrate + EcsWidth;

    if( other_bitrate > callWidth )
        video_bitrate = 0;
    else
        video_bitrate = callWidth - other_bitrate;

    BYTE  videoRateInB =
        ( video_bitrate%64000 ) ? video_bitrate/64000+1 : video_bitrate/64000;

	ALLOCBUFFER(s,ONE_LINE_BUFFER_LEN);
    sprintf(s,"videoRate in B channels <%d>",videoRateInB);
    PTRACE2(eLevelInfoNormal,"CComMode::GetVideoBitrateInB : ",s);
	DEALLOCBUFFER(s);
    return videoRateInB;
}

/////////////////////////////////////////////////////////////////////////////
void CComMode::GetMediaBitrate(  DWORD& audio_bitrate,DWORD& video_bitrate,
						DWORD& lsd_bitrate,DWORD& hsd_bitrate,
						DWORD& mlp_bitrate,DWORD& hmlp_bitrate,
						DWORD& content_bitrate,WORD channelWidthForCalc)
{
    const DWORD FAS_WIDTH=800;
    const DWORD BAS_WIDTH=800;

    DWORD callWidth;
    DWORD FasBasWidth;

    //the data bitrates are currently not supported
    lsd_bitrate = 0;
    hsd_bitrate = 0;
    mlp_bitrate = 0;
    hmlp_bitrate = 0;
    content_bitrate = 0;

    DWORD chnlWidth;
    long firstChnlLeftWidth;
    DWORD other_bitrate;
	DWORD EcsWidth = 0;

	if (channelWidthForCalc != 0)
	{
		callWidth = channelWidthForCalc * 64 * 1000;
	    FasBasWidth = (FAS_WIDTH + BAS_WIDTH);
	}
	else
	{
		callWidth = m_xferMode.GetNumChnl() * m_xferMode.GetChnlWidth() * 64 * 1000;
		FasBasWidth = m_xferMode.GetNumChnl() * (FAS_WIDTH + BAS_WIDTH);
	}

    chnlWidth = 64 * 1000;

    //update the bitrate field in m_audMode according to the audio algoritem
    m_audMode.SetBitRate(m_audMode.GetAudMode());
    audio_bitrate = m_audMode.GetBitRate() * 1 * 1000;
    CCDRUtils::Get_Hsd_Hmlp_command_Bitrate((BYTE)GetHmlpMode(),(WORD *)&hmlp_bitrate);
    hmlp_bitrate *= 100;
    CCDRUtils::Get_Lsd_Mlp_Command_BitRate((BYTE)GetLsdMode(),(WORD *)&lsd_bitrate);
    lsd_bitrate *= 100;
    CCDRUtils::Get_Lsd_Mlp_Command_BitRate((BYTE)GetMlpMode(),(WORD *)&mlp_bitrate);
    mlp_bitrate *= 100;
	BYTE contentRateOpcode = AMSC_0k;
	if( m_contentMode.IsContentModeOn() )
		contentRateOpcode = m_contentMode.GetContentRate();
    CCDRUtils::Get_Content_command_Bitrate((BYTE)contentRateOpcode,(WORD*)&content_bitrate);
    content_bitrate *= 100;
    if (m_otherMode.GetRestrictMode() == Restrict )  {
        chnlWidth = (chnlWidth * 7)/8;
        callWidth = (callWidth * 7) / 8;
        if (audio_bitrate == 56000)
            audio_bitrate = 48000;
		content_bitrate = (content_bitrate * 7) / 8;
    }
	if(m_otherMode.GetEncrypMode()==Encryp_On) {
	  // isdn_encryption
	  //PTRACE(eLevelInfoNormal,"CComMode::GetMediaBitrate: Encryp_On found - temp we don't support it (EcsWidth should be 800) !!!");
	    EcsWidth = 800;
	}
        // mlp - check validity on the first channel
    if(mlp_bitrate){
        firstChnlLeftWidth = chnlWidth - (FAS_WIDTH + BAS_WIDTH) - audio_bitrate  - mlp_bitrate - content_bitrate - EcsWidth;
        if( firstChnlLeftWidth < 0 ) {
            video_bitrate = 0;
            return;
        }
    }

    other_bitrate = FasBasWidth + audio_bitrate
                   + lsd_bitrate + hsd_bitrate + mlp_bitrate + hmlp_bitrate + content_bitrate + EcsWidth;

    if(other_bitrate > callWidth)
        video_bitrate = 0;
    else
        video_bitrate = callWidth - other_bitrate;


	ALLOCBUFFER(s,TEN_LINE_BUFFER_LEN);
	sprintf(s,"\ncallWidth %u\nFasBasWidth %u\naudio_bitrate %u\nvideo_bitrate %u\n,mlp_bitrate %u\n,hmlp_bitrate %u\n,lsd_bitrate %u\n,content_bitrate %u\n,Ecs_width %u\n",
        callWidth,FasBasWidth,audio_bitrate,video_bitrate,mlp_bitrate,hmlp_bitrate,lsd_bitrate, content_bitrate,EcsWidth);
    PTRACE2(eLevelInfoNormal,"CComMode::GetMediaBitrate : ",s);//INTO_MCMS_TRACE
	DEALLOCBUFFER(s);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CComMode::GetMediaBitrate(const eMediaType mediaType, WORD channelWidthForCalc)
{
	DWORD retValue = 0;
	DWORD aud_bitrate, vid_bitrate, lsd_bitrate, hsd_bitrate, mlp_bitrate,
			hmlp_bitrate, content_bitrate;

	aud_bitrate = vid_bitrate = lsd_bitrate = hsd_bitrate = mlp_bitrate =
			hmlp_bitrate = content_bitrate = 0;

	GetMediaBitrate(aud_bitrate,vid_bitrate,lsd_bitrate,hsd_bitrate,mlp_bitrate,
		hmlp_bitrate,content_bitrate,channelWidthForCalc);

	switch( mediaType ) {
		case eMediaTypeAudio    :  retValue = aud_bitrate;      break;
		case eMediaTypeVideo    :  retValue = vid_bitrate;      break;
		case eMediaTypeLsd      :  retValue = lsd_bitrate;      break;
		case eMediaTypeHsd      :  retValue = hsd_bitrate;      break;
		case eMediaTypeMlp      :  retValue = mlp_bitrate;      break;
		case eMediaTypeHmlp     :  retValue = hmlp_bitrate;     break;
		case eMediaTypeContent  :  retValue = content_bitrate;  break;
		default : DBGPASSERT(1); break;
	}
	return retValue;
}

/////////////////////////////////////////////////////////////////////////////
WORD CComMode::IsSameAudioBitrate(const CComMode &other) const
{
    WORD audio_mode_1 = m_audMode.GetAudMode();
    WORD audio_mode_2 = other.m_audMode.GetAudMode();
    WORD rc =0;

    //handle audio in restrict mode
    if ((GetOtherRestrictMode() == Restrict) && (audio_mode_1 == A_Law_OF))
        audio_mode_1=A_Law_48;
    if ((GetOtherRestrictMode() == Restrict) && (audio_mode_1 == U_Law_OF))
        audio_mode_1=U_Law_48;
    if ((other.GetOtherRestrictMode() == Restrict) && (audio_mode_2 == A_Law_OF))
        audio_mode_2=A_Law_48;
    if ((other.GetOtherRestrictMode() == Restrict) && (audio_mode_2 == U_Law_OF))
        audio_mode_2=U_Law_48;

    //check audio bitrate groups
    if (m_audMode.GetBitRateClass() == other.m_audMode.GetBitRateClass() )
        rc =1;

    //if transcoded , all audio bitrates are acceptable
    //if (1);

    return rc;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CComMode::SetMaxMlpRate(const CCapH320& rCap)
{
    WORD mlpmode=m_lsdMlpMode.GetMlpMode();
    WORD init_mode=m_lsdMlpMode.GetMlpMode();
    static WORD mlpRates[] = {Mlp_46_4,  Mlp_40, Mlp_38_4, Mlp_32, Mlp_30_4, Mlp_24, Mlp_22_4, Mlp_16, Mlp_14_4};
    static WORD mlpCaps[] = {Mlp_Cap_46_4, Mlp_Cap_40, Mlp_Cap_38_4, Mlp_Cap_32, Mlp_Cap_30_4, Mlp_Cap_24,
                               Mlp_Cap_22_4, Mlp_Cap_16 , Mlp_Cap_14_4 };

    const WORD numMlpRate = 9;
    WORD initIndex = 0;

    WORD transferRate = GetXferMode();
    WORD audMode = GetAudMode();
    WORD restrictMode = GetOtherRestrictMode() ;
    for (WORD i =0; i< numMlpRate; i++){
        if (mlpRates[i] == init_mode){
            initIndex = i+1;
            break;
        }
    }

    for(WORD k = initIndex; k < numMlpRate;k++){
        WORD isT120NotValid = ::IsT120Valid((BYTE)transferRate,(BYTE)audMode ,(BYTE)restrictMode,(BYTE)mlpRates[k]);
        if(!isT120NotValid && rCap.IsMlpCap(mlpCaps[k])){
            mlpmode = mlpRates[k];
            break;
        }
    }

    if(mlpmode==init_mode) {
        if(rCap.IsDataCap(Dxfer_Cap_Mlp_6_4k))
            mlpmode = MLP_6_4k;
        else
            mlpmode = MLP_Off;
    }
    m_lsdMlpMode.SetMlpMode(mlpmode);
    return mlpmode;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CComMode::SetMaxHmlpRate(const CCapH320& rCap)
{
    WORD mlpCap;
    WORD mlpMode;
    WORD hmlpmode=m_hsdHmlpMode.GetHmlpMode();
    WORD init_mode=m_hsdHmlpMode.GetHmlpMode();
    WORD finalMode = 0;
    WORD transferRate = GetXferMode();
    WORD audMode = GetAudMode();
    WORD restrictMode = GetOtherRestrictMode() ;
    WORD isT120NotValid  = 0;

    switch(init_mode){
        case H_Mlp_Com_62_4:{
            hmlpmode = H_Mlp_Com_64;
            isT120NotValid = ::IsT120Valid((BYTE)transferRate,(BYTE)audMode ,(BYTE)restrictMode,(BYTE)hmlpmode);
            if(!isT120NotValid && rCap.IsHsdHmlpCap(hmlpmode) )finalMode = hmlpmode;
            break;
        }
        case H_Mlp_Com_64:{
            hmlpmode = H_Mlp_Com_62_4;
            isT120NotValid = ::IsT120Valid((BYTE)transferRate,(BYTE)audMode ,(BYTE)restrictMode,(BYTE)hmlpmode);
            if(!isT120NotValid && rCap.IsHsdHmlpCap(hmlpmode) )finalMode = hmlpmode;
            break;
        }
        default:
            break;
    }

    if(init_mode != H_Mlp_Com_14_4 && !finalMode){
        const WORD maximalT120Rate = H_Mlp_Com_128;
        WORD start_index = (init_mode < maximalT120Rate) ? init_mode -1 : maximalT120Rate;
        for(int i=start_index ;i >= H_Mlp_Com_62_4;i--){
            WORD isT120NotValid = ::IsT120Valid((BYTE)transferRate,(BYTE)audMode ,(BYTE)restrictMode,(BYTE)hmlpmode);
            if(hmlpmode >= init_mode || isT120NotValid)
                if(rCap.IsHsdHmlpCap(i ) &&  i != H_Mlp_Com_14_4) {
                    finalMode = hmlpmode = i;
                }
        }
    }

    if(!finalMode)
        finalMode = hmlpmode = H_Mlp_Off_Com;
    m_hsdHmlpMode.SetHmlpMode(hmlpmode);
    if(hmlpmode  ==  H_Mlp_Off_Com){
        if(init_mode == H_Mlp_Com_14_4){
            mlpMode = Mlp_14_4;
            mlpCap = Mlp_Cap_14_4;
        }  else  {
            mlpMode = Mlp_30_4;
            mlpCap = Mlp_Cap_30_4;
        }
        m_lsdMlpMode.SetMlpMode(mlpMode);
        WORD rc = ::IsT120Valid((BYTE)transferRate,(BYTE)audMode ,(BYTE)restrictMode,(BYTE)mlpMode);
        if(rc || !(rCap.IsDataCap(Mlp_Set_2) || rCap.IsMlpCap(mlpCap)))
            SetMaxMlpRate(rCap);
    }
    return hmlpmode;
}


/////////////////////////////////////////////////////////////////////////////
void  CComMode::SetNsMode(const CNonStandardMode& rNsMode)
{
    m_nsMode = rNsMode;
}

////////////////////////////////////////////////////////////////////////////
//////////////////////// CAMSCMode /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
BYTE CAMSCMode::GetOpcode(void) const
{
	return m_opcode;
}

////////////////////////////////////////////////////////////////////////////
BYTE CAMSCMode::GetControlID(void) const
{
	return m_ControlID;
}

////////////////////////////////////////////////////////////////////////////
BYTE CAMSCMode::GetStartSubChannel(void) const
{
	return m_StartSubChannel;
}

////////////////////////////////////////////////////////////////////////////
BYTE CAMSCMode::GetEndSubChannel(void) const
{
	return m_EndSubChannel;
}

////////////////////////////////////////////////////////////////////////////
//////////////////////// CContentMode //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
CContentMode::CContentMode()
{
	m_opcode =0;
	m_ControlID =0;
	m_StartSubChannel =0;
	m_EndSubChannel =0;
	m_MediaModeSize =0;
	m_pMediaMode = NULL;
	m_ContentRate = 0;
	m_ContentLevel = eGraphics;
}

/////////////////////////////////////////////////////////////////////////////
CContentMode::CContentMode( CContentMode& other ) : CAMSCMode(other)
{
	m_pMediaMode = NULL;
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
CContentMode::CContentMode( const BYTE  opcode, const BYTE ControlID,
				const BYTE StartSubChannel ,const BYTE  EndSubChannel,
				const BYTE mediaModeLen, const BYTE* pMediaMode,const WORD ContentLavel)
{
	BYTE ContentRate;

	SetOpcode (opcode);
	SetControlID (ControlID);
	SetStartSubChannel ( StartSubChannel);
	SetEndSubChannel ( EndSubChannel);
	SetMediaMode(mediaModeLen,pMediaMode);
	if(opcode == NS_COM_AMSC_ON){
		SetContentLevel(ContentLavel);
		ContentRate = CalculateContentRate(StartSubChannel, EndSubChannel);
		if(ContentRate <= AMSC_1536k)
				SetContentRate(ContentRate);
	}

}

/////////////////////////////////////////////////////////////////////////////
CContentMode::~CContentMode()
{
	PDELETEA(m_pMediaMode);
}

/////////////////////////////////////////////////////////////////////////////
const char*   CContentMode::NameOf() const
{
	return "CContentMode";
}

///////////////////////////////////////////////////////////////////////////////
void  	CContentMode::SetOpcode (const BYTE opcode)
{
	m_opcode = opcode;

	if (m_opcode == 0 || m_opcode==NS_COM_AMSC_OFF)
		m_ContentRate = AMSC_0k;
}

/////////////////////////////////////////////////////////////////////////////
void CContentMode::SetControlID(const BYTE ControlID)
{
	m_ControlID = ControlID;

}

/////////////////////////////////////////////////////////////////////////////
void CContentMode::SetStartSubChannel(const BYTE StartSubChannel)
{
	m_StartSubChannel = StartSubChannel;

}

/////////////////////////////////////////////////////////////////////////////
void CContentMode::SetEndSubChannel(const BYTE EndSubChannel)
{
	m_EndSubChannel = EndSubChannel;

}

/////////////////////////////////////////////////////////////////////////////
void CContentMode::SetMediaMode(const BYTE mediaModeLen, const BYTE* pMediaMode)
{
	PDELETEA(m_pMediaMode);
	m_MediaModeSize = mediaModeLen;
	if( m_MediaModeSize ) {
		m_pMediaMode = new BYTE[m_MediaModeSize];
		memcpy(m_pMediaMode,pMediaMode,m_MediaModeSize);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CContentMode::SetContentRate(const BYTE ContentRate)
{
	m_ContentRate = ContentRate;

}
//////////////////////////////////////////////////////////////////////////////
void CContentMode::SetContentLevel(const WORD Contentlevel)
{
	m_ContentLevel = Contentlevel;
}
//////////////////////////////////////////////////////////////////////////////
void CContentMode::Create(const BYTE opcode, const BYTE ControlID, const BYTE StartSubChannel,const BYTE EndSubChannel,
						const BYTE mediaModeLen, const BYTE* pMediaMode)
{
	BYTE ContentRate;

	SetOpcode (opcode);
	SetControlID (ControlID);
	if(opcode == NS_COM_AMSC_ON)
	{
		SetStartSubChannel ( StartSubChannel);
		SetEndSubChannel ( EndSubChannel);
		SetMediaMode(mediaModeLen,pMediaMode);
		ContentRate = CalculateContentRate(StartSubChannel, EndSubChannel);
		if(ContentRate <= AMSC_1536k)
			SetContentRate(ContentRate);
	}
}
//////////////////////////////////////////////////////////////////////////////
void CContentMode::CreateWithContentLevel(const BYTE opcode, const BYTE ControlID, const BYTE StartSubChannel,const BYTE EndSubChannel,
						const BYTE mediaModeLen, const BYTE* pMediaMode,const WORD ContentLevel)
{
	Create(opcode,ControlID,StartSubChannel,EndSubChannel,mediaModeLen,pMediaMode);

	if(opcode == NS_COM_AMSC_ON)
		SetContentLevel(ContentLevel);
}
//////////////////////////////////////////////////////////////////////////////
void CContentMode::CreateMinimal(const WORD ContentLevel)
{
	BYTE*  pMedia = new BYTE[1];
	pMedia[0] = H263;// | OTHRCMDATTR;
	CreateWithContentLevel(NS_COM_AMSC_ON,2,0,0,1,pMedia,ContentLevel);
	PDELETEA(pMedia);
}

/////////////////////////////////////////////////////////////////////////////
void CContentMode::DeSerialize(WORD format,CSegment& seg)
{

	switch ( format )  {
		case SERIALEMBD :{
			break;
		}
		case NATIVE     :{
			seg >> m_opcode
				>> m_ControlID
				>> m_StartSubChannel
				>> m_EndSubChannel
				>> m_ContentLevel
			/*according to MT field in the ControlID will be obtained the mediaMode
				but its only BYTE in the first version of P&C*/
				>> m_MediaModeSize;

			PDELETEA(m_pMediaMode);
			if( m_MediaModeSize ) {
				m_pMediaMode = new BYTE[m_MediaModeSize];

				for (int i = 0;i < m_MediaModeSize; i++)
					seg >> m_pMediaMode[i];
			}

			m_ContentRate = CalculateContentRate(m_StartSubChannel, m_EndSubChannel);
			if(m_ContentRate > AMSC_1536k)
				m_ContentRate = AMSC_0k;
			break;
		}
		default : {
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CContentMode::Serialize(WORD format,CSegment& seg, BYTE isH239)
{
	switch ( format )  {
		case SERIALEMBD :{
		if(isH239)
		{
			SerializeToH239(seg);
			break;
		}
		else
		{
			BYTE Flag = 1;// TODO

			if(m_opcode == NS_COM_AMSC_ON)
			{
				if(Flag)
					SerializeRoleLabel(seg);

				seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
				seg << (BYTE) (NS_MSG_HEADER_LEN + 4 + m_MediaModeSize );// msgLen
			}
			else if(m_opcode == NS_COM_AMSC_OFF)
			{
				if(Flag)
					SerializeRoleLabel(seg);

				seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
				seg << (BYTE) (NS_MSG_HEADER_LEN + 2);//msgLen
			}
			else
				return;

			seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
				<< (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
			seg << (BYTE)( W_MANUFACT_CODE_PUBLIC_PP & 0xFF )         // manufacturer code junior byte
				<< (BYTE)( (W_MANUFACT_CODE_PUBLIC_PP >> 8) & 0xFF ); // manufacturer code senior byte

			seg << m_opcode << m_ControlID;
			if(m_opcode == NS_COM_AMSC_ON)
			{
				seg << m_StartSubChannel << m_EndSubChannel  ;

										// seg <<m_pMediaMode[0];
				for( int i = 0; i<m_MediaModeSize; i++ )
					seg << (BYTE)( m_pMediaMode[i] | OTHRCMDATTR );
			}

			break;
		}
	}
		case NATIVE     :{
			seg << m_opcode << m_ControlID;
			// for any opcode seg will be filled ( NS_COM_AMSC_ON or  NS_COM_AMSC_OFF)

			seg << m_StartSubChannel << m_EndSubChannel << m_ContentLevel
					/*according to MT field in the ControlID will be sent the mediaMode
					but its only BYTE in the first version of P&C*/
				<< m_MediaModeSize;
			for (int i = 0;i < m_MediaModeSize; i++)
				seg << m_pMediaMode[i];

			break;
		}
		default : {
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Function should be called ONLY in SERIALEMBD serialize for parties that are
// connected with Content H239.
/////////////////////////////////////////////////////////////////////////////
void CContentMode::SerializeToH239(CSegment& seg)
{
	BYTE seriableVal =0, msgLength = 0, channelID = 0;

	channelID = (m_ControlID & ID_ONLY_BIT_MASK); //H239 only ID. eliminating 2 highest bits = control

	if(m_opcode == NS_COM_AMSC_ON)
	{
		seg << (BYTE)(H230_Esc | ESCAPECAPATTR);
		seg << (BYTE)(AMC_open | Attr101);
		seg << (BYTE)(H230_Sbe_Esc | ESCAPECAPATTR);
		//AMCOpenByte1
		seriableVal = PRESENTAION_ROLE_LABEL_AMCOpenByte1;
		seriableVal |= channelID;
		seg << seriableVal;
		seg << (BYTE)(H230_Sbe_Esc | ESCAPECAPATTR);
		//AMCOpenByte2
		seriableVal = GetNumberOfB0ChannelsByEpcContentRate(m_ContentRate); //subTimeslotCount
		seriableVal *= SUB_TIME_SLOTS_IN_BO_CHANNEL;
		seg << seriableVal;

	}
	else if(m_opcode == NS_COM_AMSC_OFF)
	{
		seg << (BYTE)(H230_Esc | ESCAPECAPATTR);
		seg << (BYTE)(AMC_close | Attr101);
		seg << (BYTE)(H230_Sbe_Esc | ESCAPECAPATTR);
		//AMCCloseByte1
		seg << channelID;
	}
	else
		return;

}

/////////////////////////////////////////////////////////////////////////
void CContentMode::	SerializeRoleLabel(CSegment& seg )
{
		seg << (BYTE) ( ESCAPECAPATTR | Ns_Com );
		seg << (BYTE) (4 + 2 );

			seg << (BYTE)( W_COUNTRY_CODE_USA & 0xFF )                 // country code junior byte
				<< (BYTE)( (W_COUNTRY_CODE_USA >> 8) & 0xFF );         // country code senior byte
			seg << (BYTE)( W_MANUFACT_CODE_PUBLIC_PP & 0xFF )         // manufacturer code junior byte
				<< (BYTE)( (W_MANUFACT_CODE_PUBLIC_PP >> 8) & 0xFF ); // manufacturer code senior byte

		seg << NS_COM_ROLE_LABEL<<(BYTE)(ContentLabel | RoleLabelTerminatorMask);

}
//////////////////////////////////////////////////////////////////////////
void  CComMode::SetContentMode(const CContentMode& rContentMode)
{
	m_contentMode = rContentMode;
}
void CComMode::SetContentFromComMode(const CComMode& rComMode)
{
	m_contentMode = rComMode.m_contentMode;
}
//////////////////////////////////////////////////////////////////////////
void CComMode::CreateLocalComModeECS()
{
	m_ECSMode.CreateLocalComModeECS();
}

//////////////////////////////////////////////////////////////////////////
void   CContentMode::operator=(const CContentMode& other)
{
	if ( &other == this ) return;

	m_opcode          = other.m_opcode;
	m_ControlID       = other.m_ControlID;
	m_StartSubChannel = other.m_StartSubChannel;
	m_EndSubChannel   = other.m_EndSubChannel;
	m_ContentRate     = other.m_ContentRate;
	m_ContentLevel	  = other.m_ContentLevel;
	m_MediaModeSize   = other.m_MediaModeSize;
	PDELETEA(m_pMediaMode);
	if( m_MediaModeSize ) {
		m_pMediaMode = new BYTE[m_MediaModeSize];
		memcpy(m_pMediaMode,other.m_pMediaMode,m_MediaModeSize);
	}
}
////////////////////////////////////////////////////////////////////////////
BYTE CContentMode::IsAssymetricContentMode(const CContentMode& rContentMode_2) const
{

	if(!m_opcode &&  rContentMode_2.m_opcode && rContentMode_2.m_ContentRate == AMSC_0k )
		return TRUE;
	else
		return FALSE;

}

/////////////////////////////////////////////////////////////////////////////
WORD  operator==(const CContentMode& rContentMode1,const CContentMode& rContentMode2)
{
	if( rContentMode1.m_opcode == 0  &&  rContentMode2.m_opcode == 0 )
		return 1;

	if( rContentMode1.m_opcode == NS_COM_AMSC_OFF  &&  rContentMode2.m_opcode == NS_COM_AMSC_OFF )
		return 1;

	if( rContentMode1.m_opcode == NS_COM_AMSC_OFF  &&  rContentMode2.m_opcode == 0 )
		return 1;

	if( rContentMode1.m_opcode == 0  &&  rContentMode2.m_opcode == NS_COM_AMSC_OFF )
		return 1;

	if ((	rContentMode1.m_opcode == rContentMode2.m_opcode &&
// Vasily 5/02/03: patch for PictureTel e.p returns. Sometimes iPower send to us ControlID=1,
//      and that bother to think that content is opened with right rate
//			rContentMode1.m_ControlID == rContentMode2.m_ControlID &&
			rContentMode1.m_StartSubChannel == rContentMode2.m_StartSubChannel &&
			rContentMode1.m_EndSubChannel == rContentMode2.m_EndSubChannel  &&
			rContentMode1.m_ContentRate == rContentMode2.m_ContentRate ) /*&&
			!( memcmp(rContentMode1.m_pMediaMode,rContentMode2.m_pMediaMode,rContentMode1.m_MediaModeSize) )*/

		)
	{
		if(rContentMode1.m_pMediaMode && rContentMode2.m_pMediaMode)
		{
			if(!( memcmp(rContentMode1.m_pMediaMode,rContentMode2.m_pMediaMode,rContentMode1.m_MediaModeSize) ))
				return 1;
		}
		else
			return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
void CContentMode::SetContentMode(BYTE * dump, BYTE msgSize, BYTE * contentMsgLen)
{
	BYTE    count = 0;
	BYTE    modeType;

	if(dump[count]== NS_COM_AMSC_ON)
	{
		if(msgSize < 5)
		{
			if(msgSize) {
				DBGPASSERT_AND_RETURN(msgSize);
			}

			DBGPASSERT_AND_RETURN(101);
		}


		m_opcode = NS_COM_AMSC_ON;
		count++;
		m_ControlID =  dump[count];

		modeType = m_ControlID;
		modeType = modeType >> 6;
		if(!modeType )
			modeType = ITU_STANDARD_MODE;
		else if(modeType == PROPRIETARY_MODE)
			modeType =PROPRIETARY_MODE;
		else
			modeType = RESERVED_MODE;

		count++;

		m_StartSubChannel = dump[count];
		count++;

		m_EndSubChannel = dump[count];
		count++;

		char* pMsg = new char[ONE_LINE_BUFFER_LEN];
		sprintf(pMsg,"REMOTE CONTENT MODE: start (0x%x), end (0x%x).",m_StartSubChannel,m_EndSubChannel);
		PTRACE2(eLevelInfoNormal,"CContentMode::SetContentMode : ",pMsg);
		PDELETEA(pMsg);

		//calculate Content Rate by m_StartSubChannel and m_EndSubChannel
		m_ContentRate = CalculateContentRate(m_StartSubChannel, m_EndSubChannel);
		if(m_ContentRate > AMSC_1536k)
			m_ContentRate = 0;

		m_MediaModeSize = msgSize - 4;//  4 for opcode + id + start + end

		PDELETEA(m_pMediaMode);
		BYTE mediaMode;
		if( m_MediaModeSize ) {
			m_pMediaMode = new BYTE[m_MediaModeSize];
			for (int i = 0;i < m_MediaModeSize; i++,count++) {
				mediaMode = dump[count] ^ OTHRCMDATTR;
				if( mediaMode == Video_Off || mediaMode == H261 || mediaMode == H263 )
					m_pMediaMode[i] = mediaMode;
				else
					m_pMediaMode[i] = dump[count];
			}
		}

	}
	else if(dump[count]== NS_COM_AMSC_OFF)
	{
		if(msgSize < 2)
		{
			if(msgSize) {
				DBGPASSERT_AND_RETURN(msgSize);
			}

			DBGPASSERT_AND_RETURN(101);
		}

		m_opcode = NS_COM_AMSC_OFF;
		count++;
		m_ControlID =  dump[count];
	}
	*contentMsgLen = count ;
}

//////////////////////////////////////////////////////////////////////////
BYTE CContentMode::CalculateContentRate() const
{
	return CalculateContentRate(m_StartSubChannel,m_EndSubChannel);
}

////////////////////////////////////////////////////////////////////////////
void CContentMode::SetContentModeFromH239(CSegment& seg, BYTE opcode, WORD confChanNum)
{
	BYTE tempByte = 0;
	BYTE roleLabel = 0;
	BYTE subTimeSlotCount = 0;
	BYTE sbe;
	CSmallString Msg;

	if(opcode == AMC_open)
	{
		m_opcode = NS_COM_AMSC_ON;
		seg >> sbe;
		if ( sbe != (H230_Sbe_Esc | ESCAPECAPATTR)) {
                  PTRACE(eLevelError,"CContentMode::SetContentModeFromH239 : invalid AMCOpenByte1 sbe");
                return;
        }
		seg >> tempByte;
		m_ControlID = (tempByte & FOUR_LEAST_SIGNIFICANT_BITS);		//AMCOpenByte1 4 least significant bits = channelID
		roleLabel = ((tempByte & FOUR_MOST_SIGNIFICANT_BITS) >> 4);	//AMCOpenByte1 4 most significant bits = roleLabel
		if(roleLabel!=0x2)
		{
			PASSERT(101);
		}
		seg >> sbe;
		if ( sbe != (H230_Sbe_Esc | ESCAPECAPATTR)) {
                  PTRACE(eLevelError,"CContentMode::SetContentModeFromH239 : invalid AMCOpenByte2 sbe");
                return;
        }
		seg >> subTimeSlotCount;
		subTimeSlotCount = subTimeSlotCount/SUB_TIME_SLOTS_IN_BO_CHANNEL;
		m_ContentRate = GetEpcContentRateByNumberOfB0Channels(subTimeSlotCount);
		CalculateSartEndDummyForH239(confChanNum);

		Msg << "REMOTE CONTENT MODE H239:AMC_open start (0x";
		Msg << "%x" << m_StartSubChannel;
		Msg << "), end (0x"<<m_EndSubChannel << ").";
		PTRACE2(eLevelInfoNormal,"CContentMode::SetContentModeFromH239 : ",Msg.GetString());

	}
	else if(opcode == AMC_close)
	{
		m_opcode = NS_COM_AMSC_OFF;
		seg >> sbe;
		if ( sbe != (H230_Sbe_Esc | ESCAPECAPATTR)) {
                  PTRACE(eLevelError,"CParty::GetMcuTerminalNum : invalid AMCCloseByte1 sbe");
                return;
        }
		seg >> tempByte;
		m_ControlID = tempByte;
		Msg << "REMOTE CONTENT MODE H239:AMC_close start (0x";
		Msg << "%x" << m_StartSubChannel;
		Msg << "), end (0x"<<m_EndSubChannel << ").";
		PTRACE2(eLevelInfoNormal,"CContentMode::SetContentModeFromH239 : ",Msg.GetString());

	}
	else
	{
		Msg << "Unrecognized Opcode : " << opcode;
		PTRACE2(eLevelInfoNormal,"CContentMode::SetContentModeFromH239 : ",Msg.GetString());
	}

}
//////////////////////////////////////////////////////////////////////////
void CContentMode::CalculateSartEndDummyForH239(WORD  confChanNum)
{
	BYTE  startSubChannel, endSubChannel, partyContentNumB0chan;

	partyContentNumB0chan = GetNumberOfB0ChannelsByEpcContentRate(m_ContentRate);

	// currently content channels always last
	startSubChannel = (BYTE)( (0<<5) + (confChanNum - partyContentNumB0chan + 1) );

	endSubChannel   = (BYTE)( (7<<5) + (confChanNum) );

	if( partyContentNumB0chan == 0 )
		startSubChannel = endSubChannel = 0;

	SetStartSubChannel(startSubChannel);
	SetEndSubChannel(endSubChannel);

}
//////////////////////////////////////////////////////////////////////////
BYTE CContentMode::CalculateContentRate(const BYTE StartSubChannel, const  BYTE EndSubChannel) const
{
	if(StartSubChannel == EndSubChannel)
		return (BYTE)AMSC_0k;

	BYTE     startBitNum, startTimeslot;
	BYTE     endBitNum, endTimeslot;

	startBitNum = StartSubChannel >>5;
	startTimeslot = StartSubChannel & 0x1F;

	endBitNum = EndSubChannel >> 5;
	endTimeslot = EndSubChannel & 0x1F;

	// endBitNum == 6 when restricted call
	if( (startBitNum == 0)  &&  (endBitNum == 7 || endBitNum == 6) )// only this case is supported now
	{

		//only timeslot values will be used currently for calculating ContentRate
		if((endTimeslot - startTimeslot) >= 0)
		{
			switch(endTimeslot - startTimeslot)
			{
				case 0: {
					return (BYTE)AMSC_64k;
				}
				case 1: {
					return (BYTE)AMSC_128k;
				}
				case 2: {
					return (BYTE)AMSC_192k;
				}
				case 3: {
					return (BYTE)AMSC_256k;
				}
				case 5: {
					return (BYTE)AMSC_384k;
				}
				case 7: {
					return (BYTE)AMSC_512k;
				}
				case 11: {
					return (BYTE)AMSC_768k;
				}
				case 17: {
					return (BYTE)AMSC_1152k;
				}
				case 23: {
					return (BYTE)AMSC_1536k;
				}
				default :{
					PTRACE(eLevelError,"CContentMode::CalculateContentRate : not valid rate! ");
					DBGPASSERT(endTimeslot - startTimeslot);
					return NOT_VALID_CONTENT_RATE;
				}
			}
		}
		else
		{
			PTRACE(eLevelError,"CContentMode::CalculateContentRate : not valid rate! ");
			DBGPASSERT(startBitNum);
			return NOT_VALID_CONTENT_RATE;
		}
	}
	else
	{
		ALLOCBUFFER(Mess,ONE_LINE_BUFFER_LEN);
		sprintf(Mess,"Start(0x%x),End(0x%x)",startBitNum,endBitNum);
		PTRACE2(eLevelError,"CContentMode::CalculateContentRate : not valid bit number - ",Mess);
		DEALLOCBUFFER(Mess);
		if((endBitNum - startBitNum))
			DBGPASSERT((endBitNum - startBitNum) );
		else if(endBitNum)
			DBGPASSERT(endBitNum);
		else if(startBitNum)
			DBGPASSERT(startBitNum);
		return NOT_VALID_CONTENT_RATE;
	}

}

///////////////////////////////////////////////////////////////////////
BYTE CContentMode::IsContentModeOn () const
{
	if (m_opcode == NS_COM_AMSC_ON)
		return TRUE;
	return FALSE;
}

///////////////////////////////////////////////////////////////////////
// line rate in opcodes of H.221
BYTE CContentMode::IsLineRateValidForEpc(const WORD wLineRate, const WORD wIsLSD)
{
	switch( wLineRate )
	{
		case Xfer_128   :
			{
				if(wIsLSD)
					return NO;
			}
		case Xfer_192	:
		case Xfer_256	:
		case Xfer_320	:
		case Xfer_384	:
		case Xfer_512	:
		case Xfer_768	:
		case Xfer_1152	:
		case Xfer_1472	:
		case Xfer_1536	:
		case Xfer_1920	: {
			return YES;
						  }
		default	: {
			return NO;
				  }
	}
}

///////////////////////////////////////////////////////////////////////
// line rate in opcodes of H.221
BYTE CContentMode::GetEpcContentRateByLineRate(const WORD wLineRate, const WORD isLsd,const WORD ContentLevel)
{

	/******************   ContentRateControl Table:  ********************************
	_________________________________________________________________________________
	|Conf bps |64 | 128| 192  | 256   | 320   | 384| 512| 768| 1152| 1472| 1536| 1920|
	|_________|___|____|______|_______|_______|____|____|____|_____|_____|_____|_____|
	|Graphics |0  |64/0| 64   | 64    | 128   | 128| 128| 256| 256 | 256 | 256 | 256 |
	|_________|___|____|______|_______|_______|____|____|____|_____|_____|_____|_____|
	|Hi Res   |0  |64/0| 64   | 128   | 192   | 192| 256| 384| 384 | 512 | 512 | 512 |
	|Graphics |   |    |      |       |       |    |    |    |     |     |     |     |
	|_________|___|____|______|_______|_______|____|____|____|_____|_____|_____|_____|
	|LiveVideo|0  |64/0|128/64|192/128|256/192| 256| 384| 512| 512 | 768 | 768 | 768 |
	|_________|___|____|______|_______|_______|____|____|____|_____|_____|_____|_____|


     Graphics - the old division of content rate

	*********************************************************************************/

	BYTE ContentRate = AMSC_0k;

	switch (wLineRate)
	{
		case Xfer_128	:
			{
				if(!isLsd)
				ContentRate = AMSC_64k;
				break;
			}
		case Xfer_192	:
			{
				if((ContentLevel == eLiveVideo) && (!isLsd))
					ContentRate = AMSC_128k;
				else
					ContentRate = AMSC_64k;

				break;
			}
        case Xfer_256	:
			{
				if(ContentLevel == eGraphics)  // (default)
				{
					ContentRate = AMSC_64k;
				}
				else
				{
					if(ContentLevel == eHiResGraphics)
					{
						ContentRate = AMSC_128k;
					}
					else
					{
						if(isLsd)
							ContentRate = AMSC_128k;
						else
							ContentRate = AMSC_192k;
					}
				}
				break;
			}
		case Xfer_320	:
			{
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_128k;
				}
				else
				{
					if(ContentLevel == eHiResGraphics)
					{
						ContentRate = AMSC_192k;
					}
					else
					{
						if(isLsd)
							ContentRate = AMSC_192k;
						else
							ContentRate = AMSC_256k;
					}
				}
				break;
			}
		case Xfer_384	:
			{
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_128k;
				}
				else
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMSC_192k;
					else
						ContentRate = AMSC_256k;
				}
				break;
			}
		case Xfer_512	:
			{
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_128k;
				}
				else
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMSC_256k;
					else
						ContentRate = AMSC_384k;
				}
				break;
			}
		case Xfer_768	:
			{
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_256k;
				}
				else
				{
					if(ContentLevel == eHiResGraphics)
						ContentRate = AMSC_384k;
					else
						ContentRate = AMSC_512k;
				}
				break;
			}
		case Xfer_1152	:
			{
				if(ContentLevel == eGraphics)
				{
					ContentRate = AMSC_256k;
				}
				else
				{
					if(ContentLevel ==eHiResGraphics)
						ContentRate = AMSC_384k;
					else
						ContentRate = AMSC_512k;

				}
				break;
			}
		case Xfer_1472	:
		case Xfer_1536	:
		case Xfer_1920	:
			{
				if(ContentLevel == eGraphics)
					ContentRate = AMSC_256k;
				else
				{
					if(ContentLevel ==eHiResGraphics)
						ContentRate = AMSC_512k;
					else
						ContentRate = AMSC_768k;
				}
				break;
			}

       default	: {
			break;
				  }
	}

	return ContentRate;
}

///////////////////////////////////////////////////////////////////////
BYTE CContentMode::GetNumberOfB0ChannelsByEpcContentRate(const BYTE byEpcContentRate)
{
	BYTE  byNumberOfB0Channels = 0;

	switch( byEpcContentRate )
	{
		case AMSC_0k    : { byNumberOfB0Channels =  0; break; }
		case AMSC_64k   : { byNumberOfB0Channels =  1; break; }
		case AMSC_128k  : { byNumberOfB0Channels =  2; break; }
		case AMSC_192k  : { byNumberOfB0Channels =  3; break; }
		case AMSC_256k  : { byNumberOfB0Channels =  4; break; }
		case AMSC_384k  : { byNumberOfB0Channels =  6; break; }
		case AMSC_512k  : { byNumberOfB0Channels =  8; break; }
		case AMSC_768k  : { byNumberOfB0Channels = 12; break; }
		case AMSC_1152k : { byNumberOfB0Channels = 18; break; }
		case AMSC_1536k : { byNumberOfB0Channels = 24; break; }
	}
	return byNumberOfB0Channels;
}
///////////////////////////////////////////////////////////////////////
BYTE CContentMode::TranslateRateToH239(const BYTE byEpcContentRate)
{
	BYTE  byH239ContentRate = AMC_0k;;

	switch( byEpcContentRate )
	{
		case AMSC_0k    : { byH239ContentRate =  AMC_0k; break; }
		case AMSC_64k   : { byH239ContentRate =  AMC_64k; break; }
		case AMSC_128k  : { byH239ContentRate =  AMC_128k; break; }
		case AMSC_192k  : { byH239ContentRate =  AMC_192k; break; }
		case AMSC_256k  : { byH239ContentRate =  AMC_256k; break; }
		case AMSC_384k  : { byH239ContentRate =  AMC_384k; break; }
		case AMSC_512k  : { byH239ContentRate =  AMC_512k; break; }
		case AMSC_768k  : { byH239ContentRate =  AMC_768k; break; }
		case AMSC_1152k : { byH239ContentRate =  AMC_1152k;  break; }
		case AMSC_1536k : { byH239ContentRate =  AMC_1536k;  break; }
	}
	return byH239ContentRate;
}
///////////////////////////////////////////////////////////////////////
BYTE CContentMode::TranslateRateFromH239ToEPC(const BYTE byH239ContentRate)
{
	BYTE  byEpcContentRate = AMSC_0k;

	switch( byH239ContentRate )
	{
		case AMC_0k    : { byEpcContentRate =  AMSC_0k; break; }
		case AMC_64k   : { byEpcContentRate =  AMSC_64k; break; }
		case AMC_128k  : { byEpcContentRate =  AMSC_128k; break; }
		case AMC_192k  : { byEpcContentRate =  AMSC_192k; break; }
		case AMC_256k  : { byEpcContentRate =  AMSC_256k; break; }
		case AMC_384k  : { byEpcContentRate =  AMSC_384k; break; }
		case AMC_512k  : { byEpcContentRate =  AMSC_512k; break; }
		case AMC_768k  : { byEpcContentRate =  AMSC_768k; break; }
		case AMC_1152k : { byEpcContentRate =  AMSC_1152k;  break; }
		case AMC_1536k : { byEpcContentRate =  AMSC_1536k;  break; }
	}
	return byEpcContentRate;
}
///////////////////////////////////////////////////////////////////////
BYTE CContentMode::GetEpcContentRateByNumberOfB0Channels(const BYTE byNumberOfB0Channels)
{
	BYTE  byEpcContentRate = 0;

	switch( byNumberOfB0Channels )
	{
		case 0  : { byEpcContentRate =	AMSC_0k;	break; }
		case 1  : { byEpcContentRate =  AMSC_64k;	break; }
		case 2  : { byEpcContentRate =  AMSC_128k;	break; }
		case 3  : { byEpcContentRate =  AMSC_192k;	break; }
		case 4  : { byEpcContentRate =  AMSC_256k;	break; }
		case 6  : { byEpcContentRate =  AMSC_384k;	break; }
		case 8  : { byEpcContentRate =  AMSC_512k;	break; }
		case 12 : { byEpcContentRate =	AMSC_768k;	break; }
		case 18 : { byEpcContentRate =	AMSC_1152k; break; }
		case 24 : { byEpcContentRate =	AMSC_1536k; break; }
	}
	return byEpcContentRate;
}

/////////////////////////////////////////////////////////////////////////////
//                CNSVidMode
/////////////////////////////////////////////////////////////////////////////
CNSVidMode::CNSVidMode()          // constructor
{
	octet0 = 0;
	octet1 = 0;
}
/////////////////////////////////////////////////////////////////////////////
CNSVidMode::~CNSVidMode()
{
}
/////////////////////////////////////////////////////////////////////////////
const char*   CNSVidMode::NameOf() const
{
    return "CNSVidMode";
}
/////////////////////////////////////////////////////////////////////////////
void CNSVidMode::SetH26LMpi(WORD NumB0Chnl)  //Currently, it used as auto mode only.
{
	if(NumB0Chnl <= 2 ) // Call rate <= 128
	{
		SetH26LCifMpi(MPI_1);
		SetH26L4CifMpi(MPI_4);
	}
	else
	{
		SetH26LCifMpi(MPI_1);
		SetH26L4CifMpi(MPI_4);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CNSVidMode::SetH26LCifMpi(WORD Mpi)
{
	if(Mpi > 7)
		PASSERT(Mpi);

	Mpi++; //In order to fit the non-standard MPI.

	octet0 &= 0x0F;
	octet0 |= (Mpi << 4);
}
/////////////////////////////////////////////////////////////////////////////
WORD CNSVidMode::GetH26LCifMpiForOneVideoStream() const
{
	return (octet0 >> 4);
}
/////////////////////////////////////////////////////////////////////////////
void CNSVidMode::SetH26L4CifMpi(WORD Mpi)
{
	if(Mpi > 7)
		PASSERT(Mpi);

	Mpi++; //In order to fit the non-standard MPI.

	octet0 &= 0xF0;
	octet0 |= Mpi;
}
/////////////////////////////////////////////////////////////////////////////
WORD CNSVidMode::GetH26L4CifMpiForOneVideoStream() const
{
	return (octet0 & 0x0F);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CNSVidMode::GetOctet0() const
{
	return octet0;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CNSVidMode::GetOctet1() const
{
	return octet1;
}
/////////////////////////////////////////////////////////////////////////////
void  CNSVidMode::DeSerialize(WORD format,CSegment& seg)
{
    switch ( format )
	{
	case SERIALEMBD :
		{
			break;
		}

	case NATIVE     :
		{
			seg >> octet0
				>> octet1;
			break;
		}

	default :
		{
			PTRACE(eLevelError,"CNSVidMode::DeSerialize : format not valid!");
			if(format!=0)
				PASSERT(format);
			else
				PASSERT(101);
			break;
		}
    }
}
/////////////////////////////////////////////////////////////////////////////
void  CNSVidMode::Serialize(WORD format,CSegment& seg)
{
    switch ( format )
	{
	case SERIALEMBD :
		{
            break;
        }

	case NATIVE     :
		{
            seg << octet0
                << octet1;
			break;
        }

	default :
		{
			PTRACE(eLevelError,"CNSVidMode::Serialize : format not valid!");
			if(format!=0)
				PASSERT(format);
			else
				PASSERT(101);
			break;
		}
    }
}
/////////////////////////////////////////////////////////////////////////////
WORD  CNSVidMode::operator<= (const CCapH320& cap) const
{
	WORD    ScmMpi = 0,CapMpi = 0;

	//CIF when one video stream is active
	ScmMpi = GetH26LCifMpiForOneVideoStream();

	if(ScmMpi)// CIF is not supported.
	{
		CapMpi = cap.GetH26LCifMpiForOneVideoStream();

		if(CapMpi == 0)//CIF resolution is not supported
			return FALSE;
		else
		{
		   if(ScmMpi < CapMpi)
			   return FALSE;
		}
	}

	//4CIF when one video stream is active
//	ScmMpi = GetH26L4CifMpiForOneVideoStream();

/*	if(ScmMpi)// 4CIF is not supported.
	{
		CapMpi = cap.GetH26L4CifMpiForOneVideoStream();

		if(CapMpi == 0)//4CIF resolution is not supported
			return FALSE;
		else
		{
		   if(ScmMpi < CapMpi)
			   return FALSE;
		}
	}*/

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
CECSMode::CECSMode()          // constructor
{
	m_pXmitRcvKey = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CECSMode::~CECSMode()        // destructor
{
	PDELETEA(m_pXmitRcvKey);
}
/////////////////////////////////////////////////////////////////////////////
CECSMode::CECSMode( CECSMode& other ) : CPObject(other)
{
	PDELETEA(m_pXmitRcvKey);

	m_xmitEncrpAlg = other.m_xmitEncrpAlg;

	if(other.m_pXmitRcvKey)//isValidPtr(other.m_pXmitRcvKey))
	{
		m_pXmitRcvKey = new BYTE[LengthXmitRcvKey];
		for(int i=0;i<LengthXmitRcvKey;i++)
			m_pXmitRcvKey[i] = other.m_pXmitRcvKey[i];
	}
}
/////////////////////////////////////////////////////////////////////////////
CECSMode&   CECSMode::operator=(const CECSMode& other)
{
	if ( &other == this ) return *this;

	m_xmitEncrpAlg = other.m_xmitEncrpAlg;

	if(other.m_pXmitRcvKey)//isValidPtr(other.m_pXmitRcvKey))
	{
	    if(!m_pXmitRcvKey)//isValidPtr(m_pXmitRcvKey))
			m_pXmitRcvKey = new BYTE[LengthXmitRcvKey];
		for(int i=0;i<LengthXmitRcvKey;i++)
			m_pXmitRcvKey[i] = other.m_pXmitRcvKey[i];
	}
	else if(m_pXmitRcvKey)//isValidPtr(m_pXmitRcvKey))
	{
		PDELETEA(m_pXmitRcvKey);
	}
	return *this;


}
/////////////////////////////////////////////////////////////////////////////
const char*   CECSMode::NameOf()  const
{
    return "CECSMode";
}
/////////////////////////////////////////////////////////////////////////////
void CECSMode::CreateLocalComModeECS()
{
	m_xmitEncrpAlg.Create(AES_PARAM,AES_128_IDENTIFIER,AES_MEDIA);
}
/////////////////////////////////////////////////////////////////////////////
void CECSMode::SetEncrpAlg(BYTE params, BYTE alg, BYTE media)
{
	m_xmitEncrpAlg.Create(params,alg,media);
}
/////////////////////////////////////////////////////////////////////////////
void CECSMode::Serialize(WORD format,CSegment& seg)
{
	switch( format ) {

		case SERIALEMBD : {
		    seg << (BYTE)P9_Identifier;      //P9
			seg << (BYTE)P9_CONTENT_LENGTH;
			m_xmitEncrpAlg.Serialize(format,seg);

			break;
		}
		case NATIVE     :{
			m_xmitEncrpAlg.Serialize(format, seg);
			break;
		}
		default : {
			DBGPASSERT_AND_RETURN(102);
			break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CECSMode::DeSerialize(WORD format,CSegment& seg)
{
	BYTE opcode, length = 0;

	switch ( format )  {
		case SERIALEMBD :{
		seg >> opcode;
		if(opcode==P9_Identifier)
		{
			seg >> length;
			DBGPASSERT_AND_RETURN(length!=P9_CONTENT_LENGTH);
			m_xmitEncrpAlg.DeSerialize(format,seg);
		}
		break;
		}
		case NATIVE     :{
			m_xmitEncrpAlg.DeSerialize(format, seg);
			break;
		}
		default : {
			DBGPASSERT_AND_RETURN(104);
			break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CECSMode::SaveECSP9(CSegment* pParam)
{
	BYTE length =0;
	BYTE alg = 0;
	BYTE params = 0;
	BYTE media = 0;

	*pParam >> length;

	if(length!=P9_CONTENT_LENGTH)	// P9 - will always be length == 3
	{
		CSmallString str;
		str <<"CECSMode::SaveECSP9 :  Illegal Length !! "<< length;
		PTRACE(eLevelError,str.GetString());
		for(int i=0;i<length;i++)
			*pParam >> params;
		return;
	}

	*pParam >> media;
	*pParam >> alg;
	*pParam >> params;

	m_xmitEncrpAlg.Create(params,alg,media);
}
/////////////////////////////////////////////////////////////////////////////
void CECSMode::SaveXmitRcvKey(CSegment* pParam)
{
    if(!m_pXmitRcvKey)//isValidPtr(m_pXmitRcvKey))
		m_pXmitRcvKey = new BYTE[LengthXmitRcvKey];
	for(int i=0;i<LengthXmitRcvKey;i++)
		*pParam >> m_pXmitRcvKey[i];
}
/////////////////////////////////////////////////////////////////////////////
BYTE* CECSMode::GetXmitRcvKey()
{
    if(m_pXmitRcvKey)//isValidPtr(m_pXmitRcvKey))
		return m_pXmitRcvKey;
	else
		return NULL;
}
/////////////////////////////////////////////////////////////////////////////
CEncrypAlg& CECSMode::GetXmitEncrpAlg()
{
	return m_xmitEncrpAlg;
}


void CComMode::SetVidModeOff()
{
	m_vidMode.SetVidMode(Video_Off);
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::SetH264Scm(BYTE CommonLevel,WORD CommonMaxMBPS, WORD CommonMaxFS, WORD CommonMaxDPB, WORD CommonMaxBRandCPB)
{
	CCapSetH264 capSetH264;
	capSetH264.Create(CommonLevel,CommonMaxMBPS,CommonMaxFS,CommonMaxDPB,CommonMaxBRandCPB);
	m_vidMode.SetH264VidMode(capSetH264);
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::SetH263InterlacedScm(WORD qcif_mpi_h221_val,WORD cif_mpi_h221_val,WORD resolution_h221)
{
    PTRACE(eLevelInfoNormal,"CComMode::SetH263InterlacedScm");// |HC_PHASE2_TRACE
	m_vidMode.SetVidMode(H263);
	m_vidMode.SetVidImageFormat(resolution_h221);

	// to be implemented
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::SetH263Scm(WORD resolution,BYTE mpi)
{
	CSmallString sstr;
	sstr << "resolution = " << resolution << ", mpi = " << mpi;
 	PTRACE2(eLevelInfoNormal,"CComMode::SetH263Scm: ",sstr.GetString());// |HC_PHASE2_TRACE

	m_vidMode.SetVidMode(H263);
	m_vidMode.SetVidImageFormat(resolution);
	m_vidMode.SetH263Mpi(mpi);
	return;
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::SetH261Scm(WORD resolution, WORD qcif_mpi, WORD cif_mpi)
{
	//m_vidMode.SetVideoSwitchAutoVideoProtocol();
	m_vidMode.SetVidMode(H261);
	m_vidMode.SetVidImageFormat(resolution);
	m_vidMode.SetQcifMpi(qcif_mpi);
	m_vidMode.SetCifMpi(cif_mpi);
	return;
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::CreateSipOptions(CCommConf* pCommConf,BYTE videoProtocol,BYTE partyEncryptionMode/* = AUTO*/)
{
	// set xfer mode
	WORD confBitRate = pCommConf->GetConfTransferRate();
	SetXferMode(confBitRate);
	//set audio mode
	WORD audRate = pCommConf->GetAudioRate();
	m_audMode.SetBitRate(audRate);
	// set video mode
	if(pCommConf->IsAudioConf()){
		m_vidMode.SetVidMode(Video_Off);
	}else{
		m_vidMode.CreateSipOptions(pCommConf,videoProtocol);
	}
	// set other mode
	m_otherMode.SetRestrictMode(Derestrict);// no restrict in SIP
	if(pCommConf->GetIsEncryption() && (partyEncryptionMode != NO))
	{
		m_otherMode.SetEncrypMode(Encryp_On);
		CreateLocalComModeECS();
	}
	else
	{
		m_otherMode.SetEncrypMode(Encryp_Off);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::SetH264VideoMode(BYTE h264Level, DWORD maxBR, DWORD maxMBPS,
								DWORD maxFS, DWORD maxDPB, WORD isAutoVidScm)
{
	m_vidMode.SetH264VidMode(isAutoVidScm, h264Level, maxBR, maxMBPS, maxFS, maxDPB);
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::SetH264VideoParams(H264VideoModeDetails h264VidModeDetails, long sar,WORD isAutoVidScm)
{
	CCapSetH264 h264CapSet;
	h264CapSet.Create(h264VidModeDetails.levelValue, h264VidModeDetails.maxMBPS, h264VidModeDetails.maxFS, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR, sar,h264VidModeDetails.profileValue);
	m_vidMode.SetIsAutoVidScm(isAutoVidScm);
	m_vidMode.SetH264VidMode(h264CapSet);

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
////////////////////////////////////////////////////////////////////////////
void CComMode::SetHd720Enabled(BYTE bIsHd720Enabled)
{
	m_bIsHd720Enabled = bIsHd720Enabled;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CComMode::IsHd720Enabled() const
{
	BYTE bRes = (m_bIsHd720Enabled == TRUE);
	return bRes;
}
////////////////////////////////////////////////////////////////////////////
void CComMode::SetHd1080Enabled(BYTE bIsHd1080Enabled)
{
	m_bIsHd1080Enabled = bIsHd1080Enabled;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CComMode::IsHd1080Enabled() const
{
	BYTE bRes = (m_bIsHd1080Enabled == TRUE);
	return bRes;
}
/////////////////////////////////////////////////////////////////////////////
void CComMode::SetHd720At60Enabled(BYTE bIsHd720At60Enabled)
{
	m_bIsHd720At60Enabled = bIsHd720At60Enabled;
}

/////////////////////////////////////////////////////////////////////////////
void CComMode::SetHd1080At60Enabled(BYTE bIsHd1080At60Enabled)
{
	m_bIsHd1080At60Enabled = bIsHd1080At60Enabled;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CComMode::IsHd720At60Enabled() const
{
	BYTE bRes = (m_bIsHd720At60Enabled == TRUE);
	return bRes;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CComMode::IsHd1080At60Enabled() const
{
	BYTE bRes = (m_bIsHd1080At60Enabled == TRUE);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
/*void CComMode::SetScmMpi(WORD protocol, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi,WORD IsAutoVidScm)
{
	switch (protocol)
	{
		case H263:
		{
			switch(m_vidMode.GetVidFormat())
			{
				ca
			}
		}
	}
}
*/
/////////////////////////////////////////////////////////////////////////////
void CComMode::TestComMode(BYTE* caps, DWORD num)
{
    std::ostringstream strm;
    strm << "\nTestComMode:  num = " << num;//bas = "  << caps[i];
    PTRACE(eLevelInfoNormal,strm.str().c_str());
    CSegment seg;
    for (WORD i=0; i<num; i++)
    {
	seg << caps[i];
    }
    this->DeSerialize(SERIALEMBD, seg);
    this->Dump(0);
}
/////////////////////////////////////////////////////////////////////////////

void CComMode::Dump (ostringstream& str)const
{
    str <<  "=====================\n";
    str <<  "CComMode::Dump\n";
    str <<  "=====================\n";
    m_audMode.Dump(str);
    str <<  "---------------------\n";
    m_vidMode.Dump(str);
    str <<  "---------------------\n";
    m_xferMode.Dump(str);
    str <<  "---------------------\n";
    m_otherMode.Dump(str);
    str <<  "---------------------\n";
    m_contentMode.Dump(str);

//   CHsdHmlpMode      m_hsdHmlpMode;
//   CLsdMlpMode       m_lsdMlpMode;
//   COtherMode        m_otherMode;
//   CNonStandardMode  m_nsMode;


//   CH221strComDrv    m_H221string;

//   CECSMode			m_ECSMode;

//   BYTE				m_bIsHdEnabled;
    str <<  "=====================\n";
    PTRACE(eLevelInfoNormal,str.str().c_str());

}
/////////////////////////////////////////////////////////////////////////////
void  CAudMode::Dump(ostringstream& str)const
{
    str << "CAudMode::Dump : \n" << "m_audMode = ";

    switch(m_audMode) {
        case A_Law_OU :
            str << "A_Law_OU";
            break;
        case U_Law_OU :
            str << "U_Law_OU";
            break;
        case G722_m1  :
            str << "G722_m1";
            break;
        case A_Law_OF :
            str << "A_Law_OF";
            break;
        case U_Law_OF :
            str << "U_Law_OF";
            break;
        case A_Law_48 :
            str << "A_Law_48";
            break;
        case U_Law_48 :
            str << "U_Law_48";
            break;
        case G722_m2  :
            str << "G722_m2";
            break;
        case G722_m3  :
            str << "G722_m3";
            break;
        case G723_1_Command :
            str << "G723_1_Command";
            break;
        case Au_24k   : // G.722.1/24
            str << "Au_24k (G.722.1/24)";
            break;
        case Au_32k   : // G.722.1/32
             str << "Au_32k (G.722.1/32)";
            break;
       case Au_Siren7_16k :
            str << "Au_Siren7_16k";
            break;
        case Au_Siren7_24k :
            str << " Au_Siren7_24k";
            break;
        case Au_Siren7_32k :
            str << "Au_Siren7_32k";
            break;
        case Au_Siren14_24k :
            str << "Au_Siren14_24k";
            break;
        case Au_Siren14_32k :
             str << "Au_Siren14_32k";
            break;
       case Au_Siren14_48k :
            str << "Au_Siren14_48k";
            break;
        case G728           :
             str << "G728";
            break;
       case G729_8k        :{
            str << "G729_8k";
            break;
        }
       case G7221_AnnexC_48k        :{
            str << "G7221_AnnexC_48k";
            break;
        }
       case G7221_AnnexC_32k        :{
            str << "G7221_AnnexC_32k";
            break;
        }
       case G7221_AnnexC_24k        :{
            str << "G7221_AnnexC_24k";
            break;
        }
        default:
        {
            str << m_audMode << " (unknown)";
            break;
        }
    }

    str << "\n";
    str << "m_bitRate = " << m_bitRate  << "k\n";
}
/////////////////////////////////////////////////////////////////////////////
void  CXferMode::Dump (ostringstream& str)const
{
    str << "CXferMode::Dump : \n" << "m_xferMode = ";

    switch(m_xferMode) {
        case Xfer_64 :
            str << "Xfer_64";
            break;
        case Xfer_2x64 :
            str << "Xfer_2x64";
            break;
        case Xfer_3x64 :
            str << "Xfer_3x64";
            break;
        case Xfer_4x64 :
            str << "Xfer_4x64";
            break;
        case Xfer_5x64 :
            str << "Xfer_5x64";
            break;
        case Xfer_6x64 :
            str << "Xfer_6x64";
            break;
        case Xfer_384 :
            str << "Xfer_384";
            break;
        case Xfer_2x384 :
            str << "Xfer_2x384";
            break;
        case Xfer_3x384 :
            str << "Xfer_3x384";
            break;
        case Xfer_4x384 :
            str << "Xfer_4x384";
            break;
        case Xfer_5x384 :
            str << "Xfer_5x384";
            break;
        case Xfer_128 :
            str << "Xfer_128";
            break;
        case Xfer_192 :
            str << "Xfer_192";
            break;
        case Xfer_256 :
            str << "Xfer_256";
            break;
        case Xfer_320 :
            str << "Xfer_320";
            break;
        case Xfer_512 :
            str << "Xfer_512";
            break;
        case Xfer_768 :
            str << "Xfer_768";
            break;
        case Xfer_1152 :
            str << "Xfer_1152";
            break;
        case Xfer_1472 :
            str << "Xfer_1472";
            break;
        case Xfer_1536 :
            str << "Xfer_1536";
            break;
        case Xfer_1920 :
            str << "Xfer_1920";
            break;
        default:
        {
            str << m_xferMode << " (unknown)";
            break;
        }
    }
    str << "\n";
}
/////////////////////////////////////////////////////////////////////////////
void  CVidMode::Dump (ostringstream& str)const
{
    str << "CVidMode::Dump : \n" << "m_vidMode = ";
    switch(m_vidMode) {
        case Video_Off :
            str << "Video_Off";
            break;
        case H261 :
            str << " H261\n";
             str << "m_cifMpi = " << m_cifMpi <<"\n";
             str << "m_qcifMpi = " << m_qcifMpi <<"\n";
            break;
        case H263 :
            str << " H263\n";
            str << "m_vidImageFormat = " << m_vidImageFormat <<"\n";
            str << "m_h263Mpi = " << m_h263Mpi <<"\n";
            break;
        case H264 :
            str << " H264\n";
            m_H264VidMode.Dump(str);
            break;
        default:
        {
            str << m_vidMode << " (unknown)\n";
            break;
        }
    }
    str << "\n";
    if(m_isFreeBitRate)
    {
        str << "m_isFreeBitRate True (CP)\n";
    }else{
        str << "m_isFreeBitRate False (VSW)\n";
    }
    if(m_isAutoVidScm){
        str << "m_isAutoVidScm True\n";
    }
    else{
        str << "m_isAutoVidScm False\n";
    }
 }
/////////////////////////////////////////////////////////////////////////////
void  CH264VidMode::Dump (ostringstream& str)const
{
//    str << "CH264VidMode::Dump : \n";
    str << "m_profileValue = " << (unsigned int)m_profileValue << "\n";
    str << "m_levelValue   = " << (unsigned int)m_levelValue << "\n";
    if(m_MaxMBPS == (DWORD)-1){
        str << "m_MaxMBPS = " << "-1\n";
    }else{
        str << "m_MaxMBPS = " << m_MaxMBPS << "\n";
    }
    if(m_MaxFS == (DWORD)-1){
        str << "m_MaxFS = " << "-1\n";
    }else{
        str << "m_MaxFS   = " << m_MaxFS << "\n";
    }
    if(m_MaxDPB == (DWORD)-1){
        str << "m_MaxDPB = " << "-1\n";
    }else{
        str << "m_MaxDPB  = " << m_MaxDPB << "\n";
    }
    if(m_MaxBR == (DWORD)-1){
        str << "m_MaxBR = " << "-1\n";
    }else{
        str << "m_MaxBR   = " << m_MaxBR << "\n";
    }
    if(m_MaxBR == (DWORD)-1){
        str << "m_MaxCPB = " << "-1\n";
    }else{
        str << "m_MaxCPB  = " << m_MaxCPB << "\n";
    }

    str << "m_Sar = " << m_Sar << "\n";
}
/////////////////////////////////////////////////////////////////////////////
BYTE CH264VidMode::IsCapableOfHD720_15fps()
{
	BYTE bRes  = FALSE;
	WORD thisLevel = m_levelValue;
	if (thisLevel >= H264_HD720_LEVEL)
	  return TRUE;

	CH264Details thisH264Details (thisLevel);
	APIS32 thisFs = m_MaxFS;
	if (thisFs == -1)
	  thisFs = thisH264Details.GetDefaultFsAsDevision();

	APIS32 thisMbps = m_MaxMBPS;
	if (thisMbps == -1)
	  thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

	WORD hd720MinimumFs   = GetMinimumHd720Fs();
	WORD hd720MinimumMbps = GetMinimumHd720At15Mbps();

	if ((thisFs >= hd720MinimumFs) && (thisMbps >= hd720MinimumMbps))
	  bRes = TRUE;
	return bRes;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CH264VidMode::IsCapableOfHD1080_15fps()
{
	BYTE bRes  = FALSE;
	WORD thisLevel = m_levelValue;
	if (thisLevel >= H264_HD1080_LEVEL)
		return TRUE;
	CH264Details thisH264Details (thisLevel);
	APIS32 thisFs = m_MaxFS;
	if (thisFs == -1)
	  thisFs = thisH264Details.GetDefaultFsAsDevision();

	APIS32 thisMbps = m_MaxMBPS;
	if (thisMbps == -1)
	  thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

	WORD hd1080MinimumFs   = GetMinimumHd1080Fs();
	WORD hd1080MinimumMbps = GetMinimumHd1080At15Mbps();

	if ((thisFs >= hd1080MinimumFs) && (thisMbps >= hd1080MinimumMbps))
	  bRes = TRUE;
	return bRes;
}
/////////////////////////////////////////////////////////////
BYTE CH264VidMode::IsCapableOfHD720_50fps()
{
	BYTE bRes  = FALSE;
	WORD thisLevel = m_levelValue;
	if (thisLevel >= H264_Level_3_2)
		return TRUE;
	CH264Details thisH264Details (thisLevel);
	APIS32 thisFs = m_MaxFS;
	if (thisFs == -1)
	  thisFs = thisH264Details.GetDefaultFsAsDevision();

	APIS32 thisMbps = m_MaxMBPS;
	if (thisMbps == -1)
	  thisMbps = thisH264Details.GetDefaultMbpsAsDevision();

	WORD hd720MinimumFs   = GetMinimumHd720Fs();
	WORD hd720_50MinimumMbps = GetMinimumHd720At50Mbps();

	if ((thisFs >= hd720MinimumFs) && (thisMbps >= hd720_50MinimumMbps))
	  bRes = TRUE;
	return bRes;
}
/////////////////////////////////////////////////////////////////////////////
void  COtherMode::Dump (ostringstream& str)const
{
    str << "COtherMode::Dump : \n";
    if(m_H06Bcomp == B6_H0_Comp){
        str << "B6_H0_Comp\n";
    }else if(m_H06Bcomp == Not_B6_H0){
        str << "Not_B6_H0\n";
    }else{
        str << "m_H06Bcomp = (unknown) " << m_H06Bcomp << "\n";
    }

    if(m_encryption == Encryp_On){
        str << "Encryp_On\n";
    }else if(m_encryption == Encryp_Off){
        str << "Encryp_Off\n";
    }else{
        str << "m_encryption = (unknown) " << m_encryption << "\n";
    }

    if(m_restrict == Restrict){
        str << "Restrict\n";
    }else if(m_restrict == Derestrict){
        str << "Derestrict\n";
    }else{
        str << "m_restrict = (unknown) " << m_restrict << "\n";
    }

    if(m_loop == Au_Loop){
        str << "Au_Loop\n";
    }else if(m_loop == Vid_Loop){
        str << "Vid_Loop\n";
    }else if(m_loop == Dig_Loop){
        str << "Dig_Loop\n";
    }else if(m_loop == Loop_Off){
        str << "Loop_Off\n";
    }else{
        str << "m_loop = (unknown) " << m_loop << "\n";
    }
}
/////////////////////////////////////////////////////////////////////////////
void  CContentMode::Dump (ostringstream& str)const
{
    str << "CContentMode::Dump : \n";
    str << "m_ContentRate   = " << (unsigned int)m_ContentRate << "\n";
    str << "m_ContentLevel  = " << (unsigned int)m_ContentLevel << "\n";
    str << "CAMSCMode : \n";
    str << "m_opcode = " << (unsigned int)m_opcode << "\n";
    str << "m_ControlID = " << (unsigned int)m_ControlID << "\n";
    str << "m_StartSubChannel = " << (unsigned int)m_StartSubChannel << "\n";
    str << "m_EndSubChannel = " << (unsigned int)m_EndSubChannel << "\n";
    str << "m_MediaModeSize = " << (unsigned int)m_MediaModeSize << "\n";
    if(m_MediaModeSize > 0)
    {
        str << "m_pMediaMode: ";
        for(int mode_index=0;mode_index<m_MediaModeSize;mode_index++){
            str << (unsigned int)m_pMediaMode[mode_index];
        }
    }

    str << "\n";
}
/////////////////////////////////////////////////////////////////////////////
