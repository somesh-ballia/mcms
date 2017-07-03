/*$Header: /MCMS/MAIN/subsys/mcmsoper/CONFSTRT.CPP 21    2/20/02 1:16p Amirk $*/
//+========================================================================+
//                            CONFSTRT.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CONFSTRT.CPP                                                |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#include <stdio.h>
#include "ConfStart.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "CdrApiClasses.h"
#include "InternalProcessStatuses.h"

#define T120_NONE		0
#define HMLP_62		2
#define HMLP_64		3
#define HMLP_128		4
#define HMLP_192		5
#define HMLP_256		6
#define HMLP_320		7
#define HMLP_384		8
#define HMLP_14		12
#define VAR_HMLP		13
#define MLP_4K		17
#define MLP_6K		18
#define VAR_MLP		19
#define MLP_14K		20
#define MLP_22K		21
#define MLP_30K		22
#define MLP_38K		23
#define MLP_46K		24
#define MLP_16K		25
#define MLP_24K		26
#define MLP_32K		27
#define MLP_40K		28
#define MLP_62K		29
#define MLP_64K		30

////////////////////////////////////////////////////////////////////////////
// class  CConfStart

ACCCDREventConfStart::ACCCDREventConfStart()
{
	m_stand_by=0;
	m_auto_terminate=0;
	m_conf_transfer_rate=0;
	m_restrict_mode=0;
	m_audio_rate=0;
	m_video_session=0;
	m_video_pic_format=0;
	m_CIF_frame_rate=0;
	m_QCIF_frame_rate=0;
	m_LSD_rate=0;
	m_HSD_rate=0;
	m_T120_bit_rate=0;
		
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStart::ACCCDREventConfStart(const ACCCDREventConfStart &other)
{
	*this=other;

}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStart::~ACCCDREventConfStart()
{
}

ACCCDREventConfStart& ACCCDREventConfStart::operator = (const ACCCDREventConfStart& other)
{
	m_stand_by=other.m_stand_by;
	m_auto_terminate=other.m_auto_terminate;
	m_conf_transfer_rate=other.m_conf_transfer_rate;
	m_restrict_mode=other.m_restrict_mode;
	m_audio_rate=other.m_audio_rate;
	m_video_session=other.m_video_session;
	m_video_pic_format=other.m_video_pic_format;
	m_CIF_frame_rate=other.m_CIF_frame_rate;
	m_QCIF_frame_rate=other.m_QCIF_frame_rate;
	m_LSD_rate=other.m_LSD_rate;
	m_HSD_rate=other.m_HSD_rate;
	m_T120_bit_rate=other.m_T120_bit_rate;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
/*
char* CConfStart::Serialize(WORD format)
{
 // assuming format = OPERATOR_MCMS  
 
 COstrStream*     pOstr;  
 char* msg_info = new char[SIZE_STREAM];
 pOstr = new COstrStream(msg_info,SIZE_STREAM);    
 Serialize(format, *pOstr); 
 int b=pOstr->pcount();
 char* msg = new char[SIZE_RECORD];
 memset(msg,' ', SIZE_RECORD);
 memcpy(msg, msg_info,b); 
 msg[b]='\0';
 msg[SIZE_RECORD-1]='\n';
 delete msg_info;
 //delete m_pOstr;
 PDELETE(pOstr);
 return msg;
}
*/
/////////////////////////////////////////////////////////////////////////////
CConfStart::CConfStart()
{
}

/////////////////////////////////////////////////////////////////////////////
CConfStart::~CConfStart()
{
}

/////////////////////////////////////////////////////////////////////////////
void CConfStart::Serialize(WORD format, std::ostream &m_ostr)
{
    m_ostr << (WORD)m_stand_by << ",";
    m_ostr << (WORD)m_auto_terminate << ",";
    m_ostr << (WORD)m_conf_transfer_rate << ",";
	m_ostr << (WORD)m_restrict_mode << ",";
	m_ostr << (WORD)m_audio_rate << ",";
	m_ostr << (WORD)m_video_session << ",";
	m_ostr << (WORD)m_video_pic_format << ",";
    m_ostr << (WORD)m_CIF_frame_rate << ",";
	m_ostr << (WORD)m_QCIF_frame_rate << ",";
	m_ostr << (WORD)m_LSD_rate << ",";
	m_ostr << (WORD)m_HSD_rate << ",";
	m_ostr << (WORD)m_T120_bit_rate << ";\n";
} 
 /////////////////////////////////////////////////////////////////////////////
void CConfStart::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag)
{
		switch (m_stand_by) {
					case STAND_BY_NO :{
    										m_ostr << "stand by:NO" << "\n";
												break;
											}
					case STAND_BY_YES :{
    										m_ostr << "stand by:YES" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch standby											
		switch (m_auto_terminate) {
					case AUTO_TERMINATE_NO :{
    										m_ostr << "auto terminate:NO" << "\n";
												break;
											}
					case AUTO_TERMINATE_YES :{
    										m_ostr << "auto terminate:YES" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch auto_terminate											
		switch (m_conf_transfer_rate) {
					case Xfer_64 :{
    										m_ostr << "conf transfer rate:1B" << "\n";
												break;
											}
					case Xfer_2x64 :{
    										m_ostr << "conf transfer rate:2B" << "\n";
												break;
									  	}
					case Xfer_3x64 :{
    										m_ostr << "conf transfer rate:3B" << "\n";
												break;
											}
					case Xfer_4x64 :{
    										m_ostr << "conf transfer rate:4B" << "\n";
												break;
									  	}
					case Xfer_5x64 :{
    										m_ostr << "conf transfer rate:5B" << "\n";
												break;
									  	}
					case Xfer_6x64 :{
    										m_ostr << "conf transfer rate:6B" << "\n";
												break;
											}
					case Xfer_128 :{
    										m_ostr << "conf transfer rate:128 kbps" << "\n";
												break;
									  	}
					case Xfer_192 :{
    										m_ostr << "conf transfer rate:192 kbps" << "\n";
												break;
											}
					case Xfer_256 :{
    										m_ostr << "conf transfer rate:256 kbps" << "\n";
												break;
									  	}
					case Xfer_320 :{
    										m_ostr << "conf transfer rate:320 kbps" << "\n";
												break;
									  	}
					case Xfer_384 :{
    										m_ostr << "conf transfer rate:384 kbps" << "\n";
												break;
											}
					case Xfer_512 :{
    										m_ostr << "conf transfer rate:512 kbps" << "\n";
												break;
									  	}
					case Xfer_768 :{
    										m_ostr << "conf transfer rate:768 kbps" << "\n";
												break;
											}
					case Xfer_1152 :{
    										m_ostr << "conf transfer rate:1152 kbps" << "\n";
												break;
									  	}
					case Xfer_1472 :{
    										m_ostr << "conf transfer rate:1472 kbps" << "\n";
												break;
									  	}
					case Xfer_1536 :{
    										m_ostr << "conf transfer rate:1536 kbps" << "\n";
												break;
											}
					case Xfer_1920 :{
    										m_ostr << "conf transfer rate:1920 kbps" << "\n";
												break;
									  	}
					case Xfer_1024 :{
    										m_ostr << "conf transfer rate:1024 kbps" << "\n";
												break;
					                    }
	                case Xfer_96 :{
    										m_ostr << "conf transfer rate:96 kbps" << "\n";
												break;
									  	}
	                case Xfer_4096 :{
    										m_ostr << "conf transfer rate:4096 kbps" << "\n";
												break;
									  	}
					case Xfer_832 :{
    										m_ostr << "conf transfer rate:832 kbps" << "\n";
												break;
									  	}				  	
					case Xfer_1728 :{
    										m_ostr << "conf transfer rate:1728 kbps" << "\n";
												break;
									  	}				  	
					case Xfer_2048 :{
    										m_ostr << "conf transfer rate:2048 kbps" << "\n";
												break;
									  	}				  	
					case Xfer_2560 :{
					    				m_ostr << "conf transfer rate:2560 kbps" << "\n";
										break;
									}
					case Xfer_3072 :{
										m_ostr << "conf transfer rate:3072 kbps" << "\n";
										break;
									}
					case Xfer_3584 :{
										m_ostr << "conf transfer rate:3584 kbps" << "\n";
										break;
								  	}
									  	
					case Xfer_6144 :{
    										m_ostr << "conf transfer rate:6144 kbps" << "\n";
												break;
									  	}				  	
					case Xfer_8192 :{
    										m_ostr << "conf transfer rate:8192 kbps" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch conf transfer rate											
		switch (m_restrict_mode) {
					case RESTRICT :{
    										m_ostr << "restrict mode"<< "\n";
												break;
											}
					case NON_RESTRICT :{
    										m_ostr << "non restrict" << "\n";
												break;
									  	}
					case AUTO :{
    										m_ostr << "mode auto" << "\n";
												break;
											}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch restrict mode				
		switch (m_audio_rate) {
					case AUTO :{
    										m_ostr << "audio rate:auto"<<"\n";
												break;
											}
					case AUDIO_RATE_16KBPS :{
    										m_ostr << "audio rate:16kbps" << "\n";
												break;
									  	}
					case AUDIO_RATE_48KBPS_G722 :{
    										m_ostr << "audio rate:48kbps G-722" << "\n";
												break;
											}
					case AUDIO_RATE_56KBPS_G722 :{
    										m_ostr << "audio rate:56kbps G-722"<< "\n";
												break;
											}
					case AUDIO_RATE_48KBPS :{
    										m_ostr << "audio rate:48kbps G-711" << "\n";
												break;
									  	}
					case AUDIO_RATE_56KBPS_G711 :{
    										m_ostr << "audio rate:56kbps G-711" << "\n";
												break;
											}
			default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch restrict mode				
		switch (m_video_session) {
					case VIDEO_SWITCH :{
    										m_ostr << "video switch"<< "\n";
												break;
											}
					case VIDEO_TRANSCODING :{
    										m_ostr << "video transcoding" << "\n";
												break;
									  	}
					case CONTINUOUS_PRESENCE :{
    										m_ostr << "continuous presence" << "\n";
												break;
											}
			default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch video session			
		switch (m_video_pic_format) {
					case PIC_FORMAT_AUTO :{
    										m_ostr << "picture format:auto"<< "\n";
												break;
											}
					case QCIF :{
    										m_ostr << "picture format:QCIF" << "\n";
												break;
									  	}
					case CIF :{
    										m_ostr << "picture format:CIF" << "\n";
												break;
											}
			default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch video pic format			
		switch (m_CIF_frame_rate) {
					case AUTO_CIF_FRAME_RATE :{
    										m_ostr << "CIF frame rate:auto"<< "\n";
												break;
											}
					case CIF_FRAME_RATE_7 :{
    										m_ostr << "CIF frame rate:7.5 pics per sec" << "\n";
												break;
									  	}
					case CIF_FRAME_RATE_10 :{
    										m_ostr << "CIF frame rate:10 pics per sec" << "\n";
												break;
											}
					case CIF_FRAME_RATE_15 :{
    										m_ostr << "CIF frame rate:15 pics per sec" << "\n";
												break;
									  	}
					case CIF_FRAME_RATE_30 :{
    										m_ostr << "CIF frame rate:30 pics per sec" << "\n";
												break;
											}
			default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch cif frame rate			
			switch (m_QCIF_frame_rate) {
					case AUTO_QCIF_FRAME_RATE :{
    										m_ostr << "QCIF frame rate:auto"<< "\n";
												break;
											}
					case QCIF_FRAME_RATE_7 :{
    										m_ostr << "QCIF frame rate:7.5 pics per sec" << "\n";
												break;
									  	}
					case QCIF_FRAME_RATE_10 :{
    										m_ostr << "QCIF frame rate:10 pics per sec" << "\n";
												break;
											}
					case QCIF_FRAME_RATE_15 :{
    										m_ostr << "QCIF frame rate:15 pics per sec" << "\n";
												break;
									  	}
					case QCIF_FRAME_RATE_30 :{
    										m_ostr << "QCIF frame rate:30 pics per sec" << "\n";
												break;
											}
			default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch QCIF frame rate			
		switch (m_LSD_rate) {
					case DYNAMIC :{
    										m_ostr << "LSD rate:dynamic" << "\n";
												break;
											}
					case LSD_RATE_300 :{
    										m_ostr << "LSD rate:300 bps" << "\n";
												break;
									  	}
					case LSD_RATE_1200 :{
    										m_ostr << "LSD rate:1200 bps" << "\n";
												break;
											}
					case LSD_RATE_4800 :{
    										m_ostr << "LSD rate:4800 bps" << "\n";
												break;
									  	}
					case LSD_RATE_6400 :{
    										m_ostr << "LSD rate:6400 bps" << "\n";
												break;
									  	}
					case LSD_RATE_8000 :{
    										m_ostr << "LSD rate:8000 bps" << "\n";
												break;
											}
					case LSD_RATE_9600 :{
    										m_ostr << "LSD rate:9600 bps" << "\n";
												break;
									  	}
					case LSD_RATE_14400 :{
    										m_ostr << "LSD rate:14.4 kbps" << "\n";
												break;
											}
					case LSD_RATE_16K :{
    										m_ostr << "LSD rate:16 kbps" << "\n";
												break;
									  	}
					case LSD_RATE_24K :{
    										m_ostr << "LSD rate:24 kbps" << "\n";
												break;
									  	}
					case LSD_RATE_32K :{
    										m_ostr << "LSD rate:32 kbps" << "\n";
												break;
											}
					case LSD_RATE_40K :{
    										m_ostr << "LSD rate:40 kbps" << "\n";
												break;
									  	}
					case LSD_RATE_48K :{
    										m_ostr << "LSD rate:48 kbps" << "\n";
												break;
											}
					case LSD_RATE_56K :{
    										m_ostr << "LSD rate:56 kbps" << "\n";
												break;
									  	}
					case LSD_RATE_62K :{
    										m_ostr << "LSD rate:62.4 kbps" << "\n";
												break;
									  	}
					case LSD_RATE_64K :{
    										m_ostr << "LSD rate:64 kbps" << "\n";
												break;
											}
					case VAR_LSD :{
    										m_ostr << "var LSD " << "\n";
												break;
									  	}
                   	case 0       :{
    										m_ostr << "LSD rate:NONE" << "\n";
												break;
									  	}

					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch LSD rate											
			switch (m_HSD_rate) {
					case HSD_DYNAMIC :{
    										m_ostr << "HSD rate:dynamic" << "\n";
												break;
											}
					case VAR_HSD :{
    										m_ostr << "var HSD" << "\n";
												break;
									  	}
					case HSD_RATE_64K :{
    										m_ostr << "HSD rate:64 kbps" << "\n";
												break;
											}
					case HSD_RATE_128K :{
    										m_ostr << "HSD rate:128 kbps" << "\n";
												break;
									  	}
					case HSD_RATE_192K :{
    										m_ostr << "HSD rate:192 kbps" << "\n";
												break;
									  	}
					case HSD_RATE_256K :{
    										m_ostr << "HSD rate:256 kbps" << "\n";
												break;
											}
					case HSD_RATE_320K :{
    										m_ostr << "HSD rate:320 kbps" << "\n";
												break;
									  	}
					case HSD_RATE_384K :{
    										m_ostr << "HSD rate:384 kbps" << "\n";
												break;
											}
					case HSD_RATE_512K :{
    										m_ostr << "HSD rate:512 kbps" << "\n";
												break;
									  	}
					case HSD_RATE_768K :{
    										m_ostr << "HSD rate:768 kbps" << "\n";
												break;
									  	}
					case HSD_RATE_1152K :{
    										m_ostr << "HSD rate:1152 kbps" << "\n";
												break;
											}
					case HSD_RATE_1536K :{
    										m_ostr << "HSD rate:1536 kbps" << "\n";
											break;
									  	}
					case 0              :{   m_ostr<< "HSD rate:NONE" << "\n";
												break;
									  	}

					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch HSD rate											
		switch (m_T120_bit_rate) {
					case T120_NONE :{
    										m_ostr << "T120 rate:none" << ";\n\n";
												break;
											}
					case HMLP_62 :{
    										m_ostr << "T120 rate:HMLP 62.4" << ";\n\n";
												break;
									  	}
					case HMLP_64 :{
    										m_ostr << "T120 rate:HMLP 64" << ";\n\n";
												break;
											}
					case HMLP_128 :{
    										m_ostr << "T120 rate:HMLP 128" << ";\n\n";
												break;
									  	}
					case HMLP_192 :{
    										m_ostr << "T120 rate:HMLP 192" << ";\n\n";
												break;
									  	}
					case HMLP_256 :{
    										m_ostr << "T120 rate:HMLP 256" << ";\n\n";
												break;
											}
					case HMLP_320 :{
    										m_ostr << "T120 rate:HMLP 320" << ";\n\n";
												break;
									  	}
					case HMLP_384 :{
    										m_ostr << "T120 rate:HMLP 384" << ";\n\n";
												break;
											}
					case HMLP_14 :{
    										m_ostr << "T120 rate:HMLP 14.4K" << ";\n\n";
												break;
									  	}
					case VAR_HMLP :{
    										m_ostr << "T120 rate: var HMLP" << ";\n\n";
												break;
									  	}
					case MLP_4K :{
    										m_ostr << "T120 rate:MLP 4K" << ";\n\n";
												break;
											}
					case MLP_6K :{
    										m_ostr << "T120 rate:MLP 6.4K" << ";\n\n";
												break;
									  	}
					case VAR_MLP :{
    										m_ostr << "T120 rate: var MLP" << ";\n\n";
												break;
											}
					case MLP_14K :{
    										m_ostr << "T120 rate:MLP 14.4K" << ";\n\n";
												break;
									  	}
					case MLP_22K :{
    										m_ostr << "T120 rate:MLP 22.4K" << ";\n\n";
												break;
									  	}
					case MLP_30K :{
    										m_ostr << "T120 rate:MLP 30.4K" << ";\n\n";
												break;
											}
					case MLP_38K :{
    										m_ostr << "T120 rate:MLP 38.4K" << ";\n\n";
												break;
									  	}
					case MLP_46K :{
    										m_ostr << "T120 rate:MLP 46.4K" << ";\n\n";
												break;
											}
					case MLP_16K :{
    										m_ostr << "T120 rate:MLP 16K" << ";\n\n";
												break;
									  	}
					case MLP_24K :{
    										m_ostr << "T120 rate:MLP 24K" << ";\n\n";
												break;
											}
					case MLP_32K :{
    										m_ostr << "T120 rate:MLP 32K" << ";\n\n";
												break;
									  	}
					case MLP_40K :{
    										m_ostr << "T120 rate:MLP 40K" << ";\n\n";
												break;
									  	}
					case MLP_62K :{
    										m_ostr << "T120 rate:MLP 62.4K" << ";\n\n";
												break;
											}
					case MLP_64K :{
    										m_ostr << "T120 rate:MLP 64K" << ";\n\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<";\n\n";
												break;
											}
					}//endswitch T120 bit rate											
} 

/////////////////////////////////////////////////////////////////////////////
void CConfStart::DeSerialize(WORD format, std::istream &m_istr)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  m_istr >> tmp;
  m_stand_by=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_auto_terminate=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_conf_transfer_rate=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_restrict_mode=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_audio_rate=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_video_session=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_video_pic_format=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_CIF_frame_rate=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_QCIF_frame_rate=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_LSD_rate=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_HSD_rate=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_T120_bit_rate=(BYTE)tmp;
  m_istr.ignore(1);

} 

/////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void CConfStart::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pConfStartNode = pFatherNode->AddChildNode("CONF_START");
	pConfStartNode->AddChildNode("STAND_BY",m_stand_by,_BOOL);
	pConfStartNode->AddChildNode("TRANSFER_RATE",m_conf_transfer_rate,TRANSFER_RATE_ENUM);
//	pConfStartNode->AddChildNode("RESTRICT_MODE",m_restrict_mode,RESTRICT_MODE_ENUM);
	pConfStartNode->AddChildNode("AUDIO_RATE",m_audio_rate,AUDIO_RATE_ENUM);
	pConfStartNode->AddChildNode("VIDEO_SESSION",m_video_session,VIDEO_SESSION_ENUM);
	pConfStartNode->AddChildNode("VIDEO_FORMAT",m_video_pic_format,VIDEO_FORMAT_ENUM);
	pConfStartNode->AddChildNode("FRAME_RATE",m_CIF_frame_rate,FRAME_RATE_ENUM);
	pConfStartNode->AddChildNode("LSD_RATE",m_LSD_rate,LSD_RATE_ENUM);

	pConfStartNode->AddChildNode("T120_RATE",m_T120_bit_rate,T120_RATE_ENUM);

	CXMLDOMElement* pAutoTerminateNode = pConfStartNode->AddChildNode("AUTO_TERMINATE");
	pAutoTerminateNode->AddChildNode("ON",m_auto_terminate,_BOOL);
	pConfStartNode->AddChildNode("QCIF_FRAME_RATE",m_QCIF_frame_rate,FRAME_RATE_ENUM);
//	pConfStartNode->AddChildNode("HSD_RATE",m_HSD_rate,HSD_RATE_ENUM);

}
//#endif

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CConfStart::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"STAND_BY",&m_stand_by,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"TRANSFER_RATE",&m_conf_transfer_rate,TRANSFER_RATE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"RESTRICT_MODE",&m_restrict_mode,RESTRICT_MODE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_RATE",&m_audio_rate,AUDIO_RATE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"VIDEO_SESSION",&m_video_session,VIDEO_SESSION_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"VIDEO_FORMAT",&m_video_pic_format,VIDEO_FORMAT_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"FRAME_RATE",&m_CIF_frame_rate,FRAME_RATE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"LSD_RATE",&m_LSD_rate,LSD_RATE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"T120_RATE",&m_T120_bit_rate,T120_RATE_ENUM);

	// schema file name:  obj_reservation.xsd
	CXMLDOMElement* pChildNode = NULL;

	GET_CHILD_NODE(pActionNode, "AUTO_TERMINATE", pChildNode);
	if (pChildNode)
		GET_VALIDATE_CHILD(pChildNode,"ON",&m_auto_terminate,_BOOL);

	GET_VALIDATE_CHILD(pActionNode,"QCIF_FRAME_RATE",&m_QCIF_frame_rate,FRAME_RATE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"HSD_RATE",&m_HSD_rate,HSD_RATE_ENUM);

	pszError[0] = '\0';
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CConfStart::NameOf() const                
{
  return "CConfStart";
}
////////////////////////////////////////////////////////////////////////////
void   CConfStart::SetStandBy(const BYTE stand_by)                 
{
     m_stand_by=stand_by;
}

////////////////////////////////////////////////////////////////////////////
BYTE  ACCCDREventConfStart::GetStandBy() const
{
    return m_stand_by;
}

/////////////////////////////////////////////////////////////////////////////
void  CConfStart::SetAutoTerminate(const BYTE autoterminate)                  
{ 
   m_auto_terminate=autoterminate;
}
/////////////////////////////////////////////////////////////////////////////
BYTE  ACCCDREventConfStart::GetAutoTerminate() const
{
    return m_auto_terminate;
}


////////////////////////////////////////////////////////////////////////////
void   CConfStart::SetConfTransfRate(const BYTE conf_trans_rate)                 
{
     m_conf_transfer_rate=conf_trans_rate;
}

////////////////////////////////////////////////////////////////////////////
BYTE  ACCCDREventConfStart::GetConfTransfRate() const
{
    return m_conf_transfer_rate;
}

////////////////////////////////////////////////////////////////////////////
				
void CConfStart::SetRestrictMode(const BYTE restrict_mode)
{
   m_restrict_mode=restrict_mode;
}
//////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCDREventConfStart::GetRestrictMode() const
{
    return m_restrict_mode;
}
///////////////////////////////////////////////////////////////////////////////		               
void   CConfStart::SetAudioRate(const BYTE audio_rate)
{
  m_audio_rate=audio_rate;
}
///////////////////////////////////////////////////////////////////////////////	
BYTE ACCCDREventConfStart::GetAudioRate() const
{
   return m_audio_rate;
}
///////////////////////////////////////////////////////////////////////////////
void   CConfStart::SetVideoSession(const BYTE video_session)
{
    m_video_session=video_session;
}
////////////////////////////////////////////////////////////////////////////////		
BYTE  ACCCDREventConfStart::GetVideoSession() const
{
  return  m_video_session;
}
///////////////////////////////////////////////////////////////////////////////	                
void   CConfStart::SetVideoPicFormat(const BYTE video_pic_format)
{
  m_video_pic_format=video_pic_format;
}
////////////////////////////////////////////////////////////////////////////////	
BYTE  ACCCDREventConfStart::GetVideoPicFormat() const
{
   return m_video_pic_format;
}
////////////////////////////////////////////////////////////////////////////////	 			
void   CConfStart::SetCifFrameRate (const BYTE cif_frame_rate)
{
   m_CIF_frame_rate=cif_frame_rate;
}
/////////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCDREventConfStart::GetCifFrameRate() const
{
  return m_CIF_frame_rate;
}
///////////////////////////////////////////////////////////////////////////////		               
void   CConfStart::SetQcifFrameRate (const BYTE Qcif_frame_rate)
{
  m_QCIF_frame_rate=Qcif_frame_rate;
}
///////////////////////////////////////////////////////////////////////////////	
BYTE ACCCDREventConfStart::GetQcifFrameRate() const
{
   return m_QCIF_frame_rate;
}
///////////////////////////////////////////////////////////////////////////////
void   CConfStart::SetLsdRate(const BYTE lsd_rate)
{
    m_LSD_rate=lsd_rate;
}
////////////////////////////////////////////////////////////////////////////////		
BYTE  ACCCDREventConfStart::GetLsdRate() const
{
  return  m_LSD_rate;
}
///////////////////////////////////////////////////////////////////////////////	                
void   CConfStart::SetHsdRate(const BYTE hsd_rate)
{
  m_HSD_rate=hsd_rate;
}
////////////////////////////////////////////////////////////////////////////////	
BYTE  ACCCDREventConfStart::GetHsdRate() const
{
   return m_HSD_rate;
}
////////////////////////////////////////////////////////////////////////////////	 			
void   CConfStart::SetT120BitRate (const BYTE t120_bit_rate)
{
   m_T120_bit_rate=t120_bit_rate;
}
/////////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCDREventConfStart::GetT120BitRate() const
{
  return m_T120_bit_rate;
}
  ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// class  CConfStart

ACCCDREventConfStartCont1::ACCCDREventConfStartCont1()
{
    m_audioTone =0;
	m_alertToneTiming=0;
	m_talkHoldTime=0;
	m_audioMixDepth=0;
	m_operatorConf=0;
	m_videoProtocol=0;  
	m_meetMePerConf=0;  
	m_numServicePhoneStr=0;
	m_numServicePhone=0;
	//m_pServicePhoneStr = new CServicePhoneStr(); // 
    for ( int i=0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
        m_pServicePhoneStr[i] = NULL;//;new CServicePhoneStr()

    m_conf_password[0]='\0';
	m_chairMode=0;
	m_cascadeMode=0;
	m_masterName[0]='\0';     
	m_numUndefParties=0;
    m_unlimited_conf_flag=YES; 
	m_time_beforeFirstJoin=0;
    m_time_afterLastQuit=0;
    m_confLockFlag=0;    
	m_max_parties=0;
    m_ind_service_phone=0;
	m_pCardRsrsStruct =new CCardRsrsStruct;
	m_pAvMsgStruct=new CAvMsgStruct;
    m_pLectureMode= new CLectureMode;
	
}

  /////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont1::ACCCDREventConfStartCont1(const ACCCDREventConfStartCont1 &other)
:ACCCDREventConfStart(other)
{
    for ( int i=0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
        m_pServicePhoneStr[i] = NULL;//;new CServicePhoneStr()

	m_pCardRsrsStruct = NULL;
    m_pAvMsgStruct = NULL;
    m_pLectureMode = NULL; 

	*this=other;

                                     
   
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont1::~ACCCDREventConfStartCont1()
{
    for ( int i=0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
		PDELETE(m_pServicePhoneStr[i]);
	PDELETE(m_pAvMsgStruct);
	PDELETE(m_pLectureMode);
	PDELETE(m_pCardRsrsStruct);
 
}
ACCCDREventConfStartCont1& ACCCDREventConfStartCont1::operator = (const ACCCDREventConfStartCont1& other)
{
  if (&other == this)
  	return *this;

  m_audioTone=other.m_audioTone;
  m_alertToneTiming=other.m_alertToneTiming;
  m_talkHoldTime=other.m_talkHoldTime;
  m_audioMixDepth=other.m_audioMixDepth;
  m_operatorConf=other.m_operatorConf;
  m_videoProtocol = other.m_videoProtocol;
  m_meetMePerConf = other.m_meetMePerConf;
  m_numServicePhoneStr=other.m_numServicePhoneStr;
  m_numServicePhone=other.m_numServicePhone;
  //m_pServicePhoneStr= new CServicePhoneStr(); 
  for (int i=0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
  { 
	  PDELETE(m_pServicePhoneStr[i]);
	  if( other.m_pServicePhoneStr[i]==NULL){
			continue;
	  }else{
			m_pServicePhoneStr[i]= new CServicePhoneStr(*other.m_pServicePhoneStr[i]);	
	  }
  }

  strncpy(m_conf_password, other.m_conf_password,H243_NAME_LEN);
  m_chairMode=other.m_chairMode;
  m_cascadeMode=other.m_cascadeMode;
  strncpy(m_masterName, other.m_masterName,H243_NAME_LEN);

  m_numUndefParties = other.m_numUndefParties;
  m_unlimited_conf_flag = other.m_unlimited_conf_flag;
  m_time_beforeFirstJoin = other.m_time_beforeFirstJoin;
  m_time_afterLastQuit = other.m_time_afterLastQuit;

  m_confLockFlag = other.m_confLockFlag;
  m_max_parties	= other.m_max_parties;

  PDELETE(m_pCardRsrsStruct);
  m_pCardRsrsStruct = new CCardRsrsStructBase(*other.m_pCardRsrsStruct);
  PDELETE(m_pAvMsgStruct);
  m_pAvMsgStruct = new CAvMsgStruct(*other.m_pAvMsgStruct);
  PDELETE(m_pLectureMode);
  m_pLectureMode = new ACCLectureMode(*other.m_pLectureMode);
  m_ind_service_phone=other.m_ind_service_phone;
  return *this;
}
 /////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  BOOL isHidePsw = NO;
  std::string key_hide = "HIDE_CONFERENCE_PASSWORD";
  CProcessBase * currentP = CProcessBase::GetProcess();

  if (currentP)
  {    
      currentP->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);
  }
  
  m_ostr << (DWORD)m_audioTone   << ",";          
  m_ostr << (WORD)m_alertToneTiming  << ","; 
  m_ostr << m_talkHoldTime  << ",";          
  m_ostr << (WORD)m_audioMixDepth  << ",";   
  m_ostr << (WORD)m_operatorConf<< ",";
  m_ostr << (WORD)m_videoProtocol<< ",";
  m_ostr << (WORD)m_meetMePerConf<< ",";
   m_ostr <<  m_numServicePhone << "," ;
  for (int i=0;i<(int)m_numServicePhone;i++)
      m_pServicePhoneStr[i]->CdrSerialize(format, m_ostr);
   m_ostr << (isHidePsw ? "" : m_conf_password)  << ",";
   m_ostr << (WORD)m_chairMode<< ",";
   m_ostr << (WORD)m_cascadeMode<< ",";
   if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
	     m_ostr << m_masterName  << ",";
   else{
		 char tmp[H243_NAME_LEN_OLD];
         strncpy(tmp,m_masterName,H243_NAME_LEN_OLD-1);
         tmp[H243_NAME_LEN_OLD-1]='\0';
		 m_ostr << tmp  << ",";
		}
  	m_ostr << m_numUndefParties << ",";    
    m_ostr << (WORD)m_unlimited_conf_flag << ",";  
    m_ostr << (WORD)m_time_beforeFirstJoin << ",";  
    m_ostr << (WORD)m_time_afterLastQuit << ",";  
    m_ostr << (WORD)m_confLockFlag << ",";  
	if(apiNum >= 137 || format != OPERATOR_MCMS)
       m_ostr << m_max_parties << ",";  
	else
	   m_ostr << (WORD)m_max_parties << ",";  
	((CCardRsrsStruct*)m_pCardRsrsStruct)->CdrSerialize(format, m_ostr);
   ((CAvMsgStruct*)m_pAvMsgStruct)->CdrSerialize(format, m_ostr);
	((CLectureMode*)m_pLectureMode)->CdrSerialize(format, m_ostr, apiNum);
}
  
/////////////////////////////////////////////////////////////////////////////

CConfStartCont1::CConfStartCont1()
{
}

/////////////////////////////////////////////////////////////////////////////
CConfStartCont1::~CConfStartCont1()
{
}

/////////////////////////////////////////////////////////////////////////////
bool CConfStartCont1::operator == (const CConfStartCont1 &other)
{
	if(m_audioTone != other.m_audioTone)
	{
		return false;
	}
	if(m_alertToneTiming != other.m_alertToneTiming)
	{
		return false;
	}
	if(m_talkHoldTime != other.m_talkHoldTime)
	{
		return false;
	}
	if(m_audioMixDepth != other.m_audioMixDepth)
	{
		return false;
	}
	if(m_operatorConf != other.m_operatorConf)
	{
		return false;
	}
	if(m_videoProtocol != other.m_videoProtocol)
	{
		return false;
	}
	if(m_meetMePerConf != other.m_meetMePerConf)
	{
		return false;
	}
	if(m_numServicePhoneStr != other.m_numServicePhoneStr)
	{
		return false;
	}
	if(m_numServicePhone != other.m_numServicePhone)
	{
		return false;
	}
	if(m_chairMode != other.m_chairMode)
	{
		return false;
	}
	
	if(m_cascadeMode != other.m_cascadeMode)
	{
		return false;
	}
	if(m_numUndefParties != other.m_numUndefParties)
	{
		return false;
	}
	if(m_unlimited_conf_flag != other.m_unlimited_conf_flag)
	{
		return false;
	}
	if(m_time_beforeFirstJoin != other.m_time_beforeFirstJoin)
	{
		return false;
	}
	if(m_time_afterLastQuit != other.m_time_afterLastQuit)
	{
		return false;
	}
	
	if(m_confLockFlag != other.m_confLockFlag)
	{
		return false;
	}
	if(m_max_parties != other.m_max_parties)
	{
		return false;
	}
	if(m_ind_service_phone != other.m_ind_service_phone)
	{
		return false;
	}
	if(0 != strcmp(m_conf_password, other.m_conf_password))
	{
		return false;
	}
	if(0 != strcmp(m_masterName, other.m_masterName))
	{
		return false;
	}
	if(*m_pAvMsgStruct != *other.m_pAvMsgStruct)
	{
		return false;
	}
	for(int i = 0 ; i < MAX_NET_SERV_PROVIDERS_IN_LIST ; i++)
	{
		if(m_pServicePhoneStr[i] != other.m_pServicePhoneStr[i])
		{
			return false;
		}
	}
	if(*m_pCardRsrsStruct != *other.m_pCardRsrsStruct)
	{
		return false;
	}
	if(*m_pLectureMode != *other.m_pLectureMode)
	{
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag,DWORD apiNum)
{

  if((m_audioTone|0xfffffffe)==0xffffffff)
  m_ostr << "entry tone:ON"<< "\n";
  else
  m_ostr << "entry tone:OFF"<< "\n";
  
  
  if((m_audioTone|0xfffffffd)==0xffffffff)
  m_ostr << "exit tone:ON"<< "\n";
  else
  m_ostr << "exit tone:OFF"<< "\n";
  
  if((m_audioTone|0xfffffffb)==0xffffffff)
  {
  m_ostr << "end time alert tone:ON"<< "\n";
  m_ostr << "alert tone :"<<(WORD)m_alertToneTiming  <<" minutes before end time " << "\n";
  }
  else
  {
  m_ostr << "end time alert tone:OFF"<< "\n";
 // m_ostr << "alert tone :"<<(WORD)m_alertToneTiming  <<" minutes before end time " << "\n";
  }


 
  
  m_ostr << "talk hold time:"<<m_talkHoldTime  << "\n";          
  m_ostr << "audio mix depth:"<< (WORD)m_audioMixDepth  << "\n";
  switch (m_operatorConf) {
					case 0 :{
    						 m_ostr << "operator conf:NO" << "\n";
							 break;
							}
					case 1 :{
    						m_ostr << "operator conf:YES" << "\n";
							break;
							}
					default :{
							m_ostr<<"--"<<"\n";
							break;
							}
						}//endswitch operatorConf											
  
  switch (m_videoProtocol) {
					case VIDEO_PROTOCOL_H261 :{
    						m_ostr <<"video protocol:H261" << "\n";
							 break;
							}
					case VIDEO_PROTOCOL_H263 :{
    						m_ostr <<"video protocol:H263" << "\n";
							break;
							}
					case VIDEO_PROTOCOL_H264 :{
							m_ostr <<"video protocol:H264" << "\n";
							break;
							}
					case VIDEO_PROTOCOL_H26L :{
							m_ostr <<"video protocol:H264*" << "\n";
							break;
							}
					case VIDEO_PROTOCOL_H264_HIGH_PROFILE:{
						m_ostr <<"video protocol:H264-HP*" << "\n";
						break;
					      }
                    case 0xFF :
						    {
    						m_ostr <<"video protocol:AUTO" << "\n";
							break;
							}

					default :{
							m_ostr<<"--"<<"\n";
							break;
							}
						}//endswitch videoProtocol											
 
  switch (m_meetMePerConf) {
					case 0 :{
    						m_ostr << "meet me per conf:NO" << "\n";
							 break;
							}
					case 1 :{
    						m_ostr << "meet me per conf:YES" << "\n";
							break;
							}
					default :{
							m_ostr<<"--"<<"\n";
							break;
							}
						}//endswitch meetMePerConf											

   ////////////////////////////////////////////////////
  // m_ostr << "num Service Phone Str:" <<m_numServicePhone << "\n" ; 
   for (int i=0;i<(int)m_numServicePhone;i++)
          m_pServicePhoneStr[i]->CdrSerialize(format, m_ostr,apiNum);
 

  /////////////////////////////////////////////////////
   m_ostr << "conference password:"<<m_conf_password  << "\n";

   switch (m_chairMode) {
					case 0 :{
    						m_ostr << "chair mode:NONE" << "\n";
							 break;
							}
					case 0xFF :{
    						m_ostr << "chair mode:AUTO" << "\n";
							break;
							}
					default :{
							m_ostr<<"--"<<"\n";
							break;
							}
						}//endswitch m_chairMode											


  switch (m_cascadeMode) {

					case 1 :{
    						m_ostr << "cascade mode:MASTER" << "\n";
							 break;
							}
                    case 2 :{
    						m_ostr << "cascade mode:SLAVE" << "\n";
							 break;
							}
					case 0xFF :{
    						m_ostr << "cascade mode:AUTO" << "\n";
							break;
							}
					default :{
							m_ostr<<"--"<<"\n";
							break;
							}
						}//endswitch cascadeMode											
 
   	     m_ostr <<"masterName:" <<m_masterName  << "\n";
		 m_ostr <<"number of undefined parties:"<< m_numUndefParties << "\n";
  switch (m_unlimited_conf_flag) {
					case 0 :{
    						m_ostr << "unlimited reserv flag:NO" << "\n";
							 break;
							}
					case 1 :{
    						m_ostr << "unlimited reserv flag:YES" << "\n";
							break;
							}
					default :{
							m_ostr<<"--"<<"\n";
							break;
							}
						}//endswitch m_numUndefParties											

  m_ostr <<"time before first party Join:" << (WORD)m_time_beforeFirstJoin << "\n";  
  m_ostr <<"time after last party quit:"<< (WORD)m_time_afterLastQuit << "\n";  
  
  switch (m_confLockFlag) {
					case 0 :{
    						m_ostr << "conference lock flag:NO" << "\n";
							 break;
							}
					case 1 :{
    						m_ostr << "conference lock flag:YES" << "\n";
							break;
							}
					default :{
							m_ostr<<"--"<<"\n";
							break;
							}
						}//endswitch m_confLockFlag											
// if (m_max_parties==0xFFFF)
   if (m_max_parties==0xFF)
   m_ostr <<"maximum parties that conference can include:no limitation"<< "\n";  
 else
   m_ostr <<"maximum parties that conference can include:"<< (WORD)m_max_parties << "\n";  
  
   ((CCardRsrsStruct*)m_pCardRsrsStruct)->CdrSerialize(format, m_ostr,bilflag); 
   ((CAvMsgStruct*)m_pAvMsgStruct)->CdrSerialize(format, m_ostr,bilflag);
   ((CLectureMode*)m_pLectureMode)->CdrSerialize(format, m_ostr, apiNum,bilflag);


}
/////////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::DeSerialize(WORD format, std::istream &m_istr ,DWORD apiNum)
{
	int i;
  WORD tmp;
//  m_istr >> tmp;
  m_istr >> m_audioTone;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_alertToneTiming = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> m_talkHoldTime;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_audioMixDepth = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_operatorConf = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_videoProtocol = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_meetMePerConf = (BYTE)tmp;

  m_istr.ignore(1);
  m_istr >> m_numServicePhone;
  m_istr.ignore(1);
  for ( i=0;i<(int)m_numServicePhone;i++){
	  if( m_pServicePhoneStr[i] == NULL){
			m_pServicePhoneStr[i] = new CServicePhoneStr();
	  }
	   m_pServicePhoneStr[i]->CdrDeSerialize(format, m_istr);
  }
  // the rest m_pServicePhoneStr should be removed
  for ( i = m_numServicePhone; i<MAX_NET_SERV_PROVIDERS_IN_LIST; i++){
	  PDELETE(m_pServicePhoneStr[i]);
  }
  m_istr.getline(m_conf_password,H243_NAME_LEN+1,',');
  m_istr >> tmp;
  m_chairMode = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_cascadeMode = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr.getline(m_masterName,H243_NAME_LEN+1,',');
  m_istr >> m_numUndefParties;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_unlimited_conf_flag = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp; 
  m_time_beforeFirstJoin = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp; 
  m_time_afterLastQuit = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp; 
  m_confLockFlag = (BYTE)tmp;
  m_istr.ignore(1);

  if(apiNum >= 137 || format != OPERATOR_MCMS){
	  m_istr >> m_max_parties; 
  }
  else{
	  m_istr >> tmp; 
	  m_max_parties = (BYTE)tmp;
  }
  
	m_istr.ignore(1);
	
  ((CCardRsrsStruct*)m_pCardRsrsStruct)->CdrDeSerialize(format, m_istr);
  ((CAvMsgStruct*)m_pAvMsgStruct)->CdrDeSerialize(format, m_istr);
  ((CLectureMode*)m_pLectureMode)->CdrDeSerialize( format, m_istr, apiNum);
   m_istr.ignore(1);
} 

/////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void CConfStartCont1::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CServicePhoneStr* pServicePhone;

	CXMLDOMElement* pConfStart1Node = pFatherNode->AddChildNode("CONF_START_1");

	pConfStart1Node->AddChildNode("ENTRY_TONE",(m_audioTone & 0x00000001),_BOOL);
	pConfStart1Node->AddChildNode("EXIT_TONE", (m_audioTone & 0x00000002),_BOOL);
//	pConfStart1Node->AddChildNode("END_TIME_ALERT_TONE", (m_audioTone & 0x00000004),_BOOL);

	pConfStart1Node->AddChildNode("TALK_HOLD_TIME", m_talkHoldTime);
	pConfStart1Node->AddChildNode("AUDIO_MIX_DEPTH", m_audioMixDepth);
	pConfStart1Node->AddChildNode("OPERATOR_CONF", m_operatorConf,_BOOL);
	pConfStart1Node->AddChildNode("VIDEO_PROTOCOL",m_videoProtocol,VIDEO_PROTOCOL_ENUM);

	CXMLDOMElement* pMeetMeNode = pConfStart1Node->AddChildNode("MEET_ME_PER_CONF");
	pMeetMeNode->AddChildNode("ON",m_meetMePerConf,_BOOL);

	if(!m_unlimited_conf_flag)
	{
		pMeetMeNode->AddChildNode("AUTO_ADD",FALSE,_BOOL);
		pMeetMeNode->AddChildNode("MIN_NUM_OF_PARTIES",0);
	}
	else
	{
		pMeetMeNode->AddChildNode("AUTO_ADD",TRUE,_BOOL);
		pMeetMeNode->AddChildNode("MIN_NUM_OF_PARTIES",m_numUndefParties);
	}

	for(int i=0; i < m_numServicePhoneStr; i++)
	{
		pServicePhone = m_pServicePhoneStr[i];

		if(pServicePhone)
			pServicePhone->SerializeXml(pMeetMeNode);
	}


	pConfStart1Node->AddChildNode("PASSWORD",m_conf_password);
	pConfStart1Node->AddChildNode("CHAIR_MODE",m_chairMode,CHAIR_MODE_ENUM);

	CXMLDOMElement* pCascadeNode = pConfStart1Node->AddChildNode("CASCADE");
	pCascadeNode->AddChildNode("CASCADE_ROLE",m_cascadeMode,CASCADE_ROLE_ENUM);
	pCascadeNode->AddChildNode("MASTER_NAME",m_masterName);

	pConfStart1Node->AddChildNode("LOCK",m_confLockFlag,_BOOL);
	pConfStart1Node->AddChildNode("MAX_PARTIES",m_max_parties,MAX_PARTIES_ENUM);

	((CCardRsrsStruct*)m_pCardRsrsStruct)->SerializeXml(pConfStart1Node);
	((CAvMsgStruct*)m_pAvMsgStruct)->SerializeXml(pConfStart1Node);
	((CLectureMode*)m_pLectureMode)->SerializeXml(pConfStart1Node);

	pConfStart1Node->AddChildNode("TIME_BEFORE_FIRST_JOIN",m_time_beforeFirstJoin);
	pConfStart1Node->AddChildNode("TIME_AFTER_LAST_QUIT",m_time_afterLastQuit);

	CXMLDOMElement* pEndTimeAlertNode = pConfStart1Node->AddChildNode("END_TIME_ALERT_TONE_EX");
	pEndTimeAlertNode->AddChildNode("ON", (m_audioTone & 0x00000004),_BOOL);
	pEndTimeAlertNode->AddChildNode("TIME", m_alertToneTiming);

}
//#endif

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CConfStartCont1::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	BYTE  bDummy;
	m_audioTone = 0;
	m_alertToneTiming = 0;
	char *pszTempString = NULL;
	CXMLDOMElement *pChildNode = NULL;
	CXMLDOMElement *pServiceNode = NULL;

	PASSERTSTREAM_AND_RETURN_VALUE(pActionNode == NULL, "Input pActionNode is NULL.", STATUS_FAIL);

	GET_VALIDATE_CHILD(pActionNode,"ENTRY_TONE",&bDummy,_BOOL);
	if( bDummy )
		m_audioTone |= 0x00000001;

	GET_VALIDATE_CHILD(pActionNode,"EXIT_TONE",&bDummy,_BOOL);
	if( bDummy )
		m_audioTone |= 0x00000002;

	//for competability reasons. 
	GET_VALIDATE_CHILD(pActionNode,"END_TIME_ALERT_TONE",pszTempString,END_TIME_ALERT_TONE_ENUM);
	if (nStatus==STATUS_OK)
	{
		if (pszTempString && strcmp(pszTempString, "off")!=0)
		{
			m_audioTone |= 0x00000004;
			GET_VALIDATE_CHILD(pActionNode,"END_TIME_ALERT_TONE",&m_alertToneTiming,_0_TO_30_DECIMAL);
		}
	}

	GET_VALIDATE_CHILD(pActionNode,"TALK_HOLD_TIME",&m_talkHoldTime,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_MIX_DEPTH",&m_audioMixDepth,_0_TO_BYTE);
	GET_VALIDATE_CHILD(pActionNode,"OPERATOR_CONF",&m_operatorConf,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"VIDEO_PROTOCOL",&m_videoProtocol,VIDEO_PROTOCOL_ENUM);

	GET_CHILD_NODE(pActionNode, "MEET_ME_PER_CONF", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode,"ON",&m_meetMePerConf,_BOOL);
		GET_VALIDATE_CHILD(pChildNode,"AUTO_ADD",&m_unlimited_conf_flag,_BOOL);
		GET_VALIDATE_CHILD(pChildNode,"MIN_NUM_OF_PARTIES",&m_numUndefParties,_0_TO_60_DECIMAL);

		GET_FIRST_CHILD_NODE(pChildNode, "SERVICE", pServiceNode);

		while( pServiceNode )
		{
			CServicePhoneStr PhoneService;
			nStatus = PhoneService.DeSerializeXml(pServiceNode, pszError);
			if (nStatus != STATUS_OK)
				return nStatus;
			AddServicePhone(PhoneService);
			GET_NEXT_CHILD_NODE(pChildNode, "SERVICE", pServiceNode);
		}
	}

	GET_VALIDATE_CHILD(pActionNode,"PASSWORD",m_conf_password,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"CHAIR_MODE",&m_chairMode,CHAIR_MODE_ENUM);

	GET_CHILD_NODE(pActionNode, "CASCADE", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode,"CASCADE_ROLE",&m_cascadeMode,CASCADE_ROLE_ENUM);
		GET_VALIDATE_CHILD(pChildNode,"MASTER_NAME",m_masterName,_0_TO_H243_NAME_LENGTH);
	}

	GET_VALIDATE_CHILD(pActionNode,"LOCK",&m_confLockFlag,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MAX_PARTIES",&m_max_parties,MAX_PARTIES_ENUM);

	GET_CHILD_NODE(pActionNode, "RESOURCE_FORCE", pChildNode);
	if (pChildNode)
	{
		nStatus = ((CCardRsrsStruct*)m_pCardRsrsStruct)->DeSerializeXml(pChildNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	nStatus = ((CAvMsgStruct*)m_pAvMsgStruct)->DeSerializeXml(pActionNode, pszError);
	if (nStatus != STATUS_OK)
		return nStatus;

	GET_CHILD_NODE(pActionNode, "LECTURE_MODE", pChildNode);
	if (pChildNode)
	{
		nStatus = ((CLectureMode*)m_pLectureMode)->DeSerializeXml(pChildNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}
	GET_VALIDATE_CHILD(pActionNode,"TIME_BEFORE_FIRST_JOIN",&m_time_beforeFirstJoin,_1_TO_60_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"TIME_AFTER_LAST_QUIT",&m_time_afterLastQuit,_0_TO_60_DECIMAL);

	GET_CHILD_NODE(pActionNode, "END_TIME_ALERT_TONE_EX", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pActionNode,"ON",&bDummy,_BOOL);
		if( bDummy )
			m_audioTone |= 0x00000004;
		GET_VALIDATE_CHILD(pActionNode,"TIME",&m_alertToneTiming,_0_TO_30_DECIMAL);
	}

	pszError[0] = '\0';
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CConfStartCont1::NameOf() const                
{
  return "CConfStartCont1";
}
////////////////////////////////////////////////////////////////////////////
DWORD ACCCDREventConfStartCont1::GetAudioTone () const
{
 return m_audioTone;
}

////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetAudioTone (const DWORD audioTone)
{
 m_audioTone=audioTone;
}
///////////////////////////////////////////////////////////////////////////
BYTE  ACCCDREventConfStartCont1::  GetAlertToneTiming() const
{
 return m_alertToneTiming;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetAlertToneTiming (const BYTE alertToneTiming)
{
 m_alertToneTiming=alertToneTiming;
}
////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventConfStartCont1::GetTalkHoldTime() const
{
 return m_talkHoldTime;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetTalkHoldTime (const WORD talkHoldTime)
{
 m_talkHoldTime=talkHoldTime;
}
////////////////////////////////////////////////////////////////////////////
BYTE ACCCDREventConfStartCont1::GetAudioMixDepth() const
{
 return m_audioMixDepth;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetAudioMixDepth (const BYTE  audioMixDepth)
{
 m_audioMixDepth=audioMixDepth;
}
////////////////////////////////////////////////////////////////////////////
BYTE ACCCDREventConfStartCont1::GetOperatorConf() const
{
 return m_operatorConf;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetOperatorConf(const BYTE	operatorConf)
{
 m_operatorConf=operatorConf;
}
////////////////////////////////////////////////////////////////////////////
BYTE ACCCDREventConfStartCont1::GetVideoProtocol() const
{
 return m_videoProtocol;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetVideoProtocol (const BYTE videoProtocol)
{
 m_videoProtocol=videoProtocol;
}
////////////////////////////////////////////////////////////////////////////
BYTE ACCCDREventConfStartCont1::GetMeetMePerConf() const
{
 return m_meetMePerConf;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetMeetMePerConf (const BYTE meetMePerConf)
{
m_meetMePerConf=meetMePerConf;
}
/////////////////////////////////////////////////////////////////////////////
WORD  ACCCDREventConfStartCont1::GetNumServicePhone() const                 
{
  return m_numServicePhone;
}
/////////////////////////////////////////////////////////////////////////////
void  CConfStartCont1::SetNumServicePhone(const WORD num)               
{
  m_numServicePhone = num;
}
/////////////////////////////////////////////////////////////////////////////
/*CServicePhoneStr* CConfStartCont1::GetFirstServicePhone() 
{
   m_ind_service_phone=1;
   return m_pServicePhoneStr[0]; 
}
/////////////////////////////////////////////////////////////////////////////
CServicePhoneStr* CConfStartCont1::GetNextServicePhone() 
{
   if (m_ind_service_phone>=m_numServicePhoneStr) return NULL;
   return m_pServicePhoneStr[m_ind_service_phone++];                        
}
/////////////////////////////////////////////////////////////////////////////
CServicePhoneStr*  CConfStartCont1::GetFirstServicePhone(int& nPos)  
{           
   CServicePhoneStr* pServicePhone = GetFirstServicePhone(); 
   nPos = m_ind_service_phone;
   return pServicePhone;
}
*/
/////////////////////////////////////////////////////////////////////////////
const char* ACCCDREventConfStartCont1::GetConf_password() const
{
 return m_conf_password;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetConf_password (const char* conf_password )
{
	strncpy(m_conf_password,conf_password,sizeof(m_conf_password) - 1);
	m_conf_password[sizeof(m_conf_password) - 1] = '\0';
}

////////////////////////////////////////////////////////////////////////////
//void SetH243_password (const char*  H243_password);

BYTE ACCCDREventConfStartCont1::GetChairMode() const
{
 return m_chairMode;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetChairMode (const BYTE  chairMode)
{
 m_chairMode=chairMode;
}
////////////////////////////////////////////////////////////////////////////	
BYTE ACCCDREventConfStartCont1::GetCascadeMode() const
{
 return m_cascadeMode;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetCascadeMode (const BYTE  cascadeMode)
{
 m_cascadeMode=cascadeMode;
}
////////////////////////////////////////////////////////////////////////////
const char* ACCCDREventConfStartCont1::GetMasterName() const
{
 return m_masterName;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetMasterName (const char*  masterName )
{
	strncpy(m_masterName,masterName,sizeof(m_masterName) - 1);
	m_masterName[sizeof(m_masterName) - 1] = '\0';
 }
////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventConfStartCont1::GetNumUndefParties() const
{
 return m_numUndefParties;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetNumUndefParties (const WORD numUndefParties)
{
 m_numUndefParties=numUndefParties;
}
////////////////////////////////////////////////////////////////////////////
BYTE ACCCDREventConfStartCont1::GetUnlimited_conf_flag() const
{
 return m_unlimited_conf_flag;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetUnlimited_conf_flag(const BYTE  unlimited_conf_flag)
{
 m_unlimited_conf_flag=unlimited_conf_flag;
}
////////////////////////////////////////////////////////////////////////////
BYTE ACCCDREventConfStartCont1::GetTime_beforeFirstJoin() const
{
 return m_time_beforeFirstJoin;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetTime_beforeFirstJoin(const BYTE  time_beforeFirstJoin)
{
 m_time_beforeFirstJoin=time_beforeFirstJoin;
}
////////////////////////////////////////////////////////////////////////////
BYTE ACCCDREventConfStartCont1::GetTime_afterLastQuit() const
{
 return m_time_afterLastQuit;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetTime_afterLastQuit(const BYTE  time_afterLastQuit)
{
 m_time_afterLastQuit=time_afterLastQuit;
}
////////////////////////////////////////////////////////////////////////////
BYTE ACCCDREventConfStartCont1::GetConfLockFlag() const
{
 return m_confLockFlag;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetConfLockFlag(const BYTE  confLockFlag)
{
 m_confLockFlag=confLockFlag;
}
////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventConfStartCont1::GetMax_parties() const
{
 return m_max_parties;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetMax_parties(const WORD  max_parties)
{
 m_max_parties = max_parties;
}
////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
CCardRsrsStructBase* ACCCDREventConfStartCont1::GetpCardRsrsStruct()
{
  return m_pCardRsrsStruct;
}
//#else
//void ACCCDREventConfStartCont1::GetpCardRsrsStruct(ACCResourceForce &ResourceForce)
//{

//	ResourceForce=*((CCardRsrsStruct *)m_pCardRsrsStruct);

//}
//#endif
/////////////////////////////////////////////////////////////////////////////

void  CConfStartCont1::SetCardRsrsStruct(const CCardRsrsStruct &cardRsrsStruct)  
{ 
   *m_pCardRsrsStruct = cardRsrsStruct;
}
/////////////////////////////////////////////////////////////////////////////
CAvMsgStruct*	ACCCDREventConfStartCont1::GetpAvMsgStruct()
{
  return m_pAvMsgStruct;
}
///////////////////////////////////////////////////////////////////////////////
void   CConfStartCont1::SetAvMsgStruct(const CAvMsgStruct &otherAvMsgStruct)
{
   *m_pAvMsgStruct = otherAvMsgStruct;
}
/////////////////////////////////////////////////////////////////////////////
 ACCLectureMode*  ACCCDREventConfStartCont1::GetpLectureMode()
 {
    return m_pLectureMode;
 }
/////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetLectureMode(const CLectureMode& otherLectureMode)
{
    *m_pLectureMode=otherLectureMode;
}
/////////////////////////////////////////////////////////////////////////////
void CConfStartCont1::SetLectureMode(const CLectureModeParams& otherLectureMode)
{
    *m_pLectureMode=otherLectureMode;
}

/////////////////////////////////////////////////////////////////////////////
CServicePhoneStr* ACCCDREventConfStartCont1::GetFirstServicePhone() 
{
   m_ind_service_phone=1;
   return m_pServicePhoneStr[0]; 
}

/////////////////////////////////////////////////////////////////////////////
CServicePhoneStr* ACCCDREventConfStartCont1::GetNextServicePhone() 
{
   if (m_ind_service_phone>=m_numServicePhoneStr) return NULL;
   return m_pServicePhoneStr[m_ind_service_phone++];                        
}

/////////////////////////////////////////////////////////////////////////////
int   CConfStartCont1::AddServicePhone(const CServicePhoneStr &other)
{
	if (m_numServicePhoneStr>=MAX_NET_SERV_PROVIDERS_IN_LIST)
  	   return  STATUS_NUMBER_OF_SERVICE_PROVIDERS_EXCEEDED;

  //if (FindServicePhone(other)!=NOT_FIND) 
  //  return STATUS_SERVICE_PROVIDER_NAME_EXISTS;  
    
  m_pServicePhoneStr[m_numServicePhoneStr] = new CServicePhoneStr(other);

  m_numServicePhoneStr++;
  return STATUS_OK;  
}

/////////////////////////////////////////////////////////////////////////////
int   ACCCDREventConfStartCont1::FindServicePhone(const CServicePhoneStr &other)
{
  for (int i=0;i<(int)m_numServicePhoneStr;i++)
  {
   if (! strcmp(m_pServicePhoneStr[i]->GetNetServiceName(),other.GetNetServiceName()))
     return i; 
  }
  return NOT_FIND; 
}

//////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// class  CConfStart

ACCCDREventConfStartCont2::ACCCDREventConfStartCont2()
{
    m_webReserved=0;  
    m_webReservUId=0; 
    m_webDBId=0; 
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont2::ACCCDREventConfStartCont2(const ACCCDREventConfStartCont2 &other)
:ACCCDREventConfStart(other)
{
	*this=other;
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont2::~ACCCDREventConfStartCont2()
{
	
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont2& ACCCDREventConfStartCont2::operator = (const ACCCDREventConfStartCont2& other)
{
    m_webReserved=other.m_webReserved;  
    m_webReservUId=other.m_webReservUId; 
    m_webDBId=other.m_webDBId; 
  return *this;
}
 /////////////////////////////////////////////////////////////////////////////
void CConfStartCont2::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  
  m_ostr << (WORD)m_webReserved     << ","; 
  m_ostr << (DWORD)m_webReservUId   << ",";
  m_ostr << (DWORD)m_webDBId        << ";\n";
}
  
/////////////////////////////////////////////////////////////////////////////
CConfStartCont2::CConfStartCont2()
{
}
/////////////////////////////////////////////////////////////////////////////

CConfStartCont2::~CConfStartCont2()
{
}
/////////////////////////////////////////////////////////////////////////////

void CConfStartCont2::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag,DWORD apiNum)
{
	if (m_webReserved)
	{
		m_ostr << "reservation was reserved from the web"<< "\n";
	}
	else
		m_ostr << "reservation was not reserved from the web"<< "\n";

	if (m_webReservUId)
	m_ostr << "user Id of the reservation establisher:"<<m_webReservUId  << "\n";          

	if (m_webDBId)
	m_ostr << "database Id of the reservation establisher:"<<m_webDBId  << "\n";          
	m_ostr <<";\n";

}
/////////////////////////////////////////////////////////////////////////////////
void CConfStartCont2::DeSerialize(WORD format, std::istream &m_istr , DWORD apiNum)
{
  WORD tmp;
  m_istr >> tmp;
  m_webReserved = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> m_webReservUId;
  m_istr.ignore(1);
  m_istr >> m_webDBId;
  m_istr.ignore(1);
} 

/////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void CConfStartCont2::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pConfStart2Node = pFatherNode->AddChildNode("CONF_START_2");

	pConfStart2Node->AddChildNode("WEB_RESERVED",m_webReserved,_BOOL);
	pConfStart2Node->AddChildNode("WEB_RESERVED_UID",m_webReservUId);
	pConfStart2Node->AddChildNode("WEB_DB_ID",m_webDBId);
}
//#endif

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CConfStartCont2::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"WEB_RESERVED",&m_webReserved,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"WEB_RESERVED_UID",&m_webReservUId,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"WEB_DB_ID",&m_webDBId,_0_TO_DWORD);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CConfStartCont2::NameOf() const                
{
  return "CConfStartCont2";
}
///////////////////////////////////////////////////////////////////////////
BYTE  ACCCDREventConfStartCont2::  GetwebReserved() const
{
 return m_webReserved;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont2::SetwebReserved  (const BYTE webReserved)
{
 m_webReserved=webReserved;
}
////////////////////////////////////////////////////////////////////////////
DWORD ACCCDREventConfStartCont2::GetwebReservUId () const
{
 return m_webReservUId;
}

////////////////////////////////////////////////////////////////////////////
void CConfStartCont2::SetwebReservUId (const DWORD webReservUId)
{
 m_webReservUId=webReservUId;
}
////////////////////////////////////////////////////////////////////////////
DWORD ACCCDREventConfStartCont2::GetwebDBId () const
{
 return m_webDBId;
}

////////////////////////////////////////////////////////////////////////////
void CConfStartCont2::SetwebDBId      (const DWORD webDBId)
{
 m_webDBId=webDBId;
}


/////////////////////////////////////////////////////////////////////////////
// ACCCDREventConfStartCont3, CConfStartCont3 - add ConfRemarks

ACCCDREventConfStartCont3::ACCCDREventConfStartCont3()
{
	m_confRemarks[0]='\0';
}

/////////////////////////////////////////////////////////////////////////////

ACCCDREventConfStartCont3::ACCCDREventConfStartCont3(const ACCCDREventConfStartCont3 &other)
:ACCCDREventConfStart(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////

ACCCDREventConfStartCont3::~ACCCDREventConfStartCont3()
{

}

/////////////////////////////////////////////////////////////////////////////

ACCCDREventConfStartCont3& ACCCDREventConfStartCont3::operator = (const ACCCDREventConfStartCont3& other)
{
	strncpy(m_confRemarks, other.m_confRemarks, CONF_REMARKS_LEN);
	return *this;
}

/////////////////////////////////////////////////////////////////////////////

CConfStartCont3::CConfStartCont3()
{
}

/////////////////////////////////////////////////////////////////////////////

CConfStartCont3::~CConfStartCont3()
{
}

/////////////////////////////////////////////////////////////////////////////

void CConfStartCont3::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
	m_ostr << m_confRemarks << ";\n";
}

/////////////////////////////////////////////////////////////////////////////

void CConfStartCont3::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag,DWORD apiNum)
{
	if ( m_confRemarks[0] != '\0' )	// something is written in the confRemarks
	{
		m_ostr << "conf remarks: " << m_confRemarks << "\n";
	}
	else
		m_ostr << "no conf remarks for this conference "<< "\n";
	
	m_ostr <<";\n";
}

/////////////////////////////////////////////////////////////////////////////////

void CConfStartCont3::DeSerialize(WORD format, std::istream &m_istr ,DWORD apiNum)
{
	m_istr.getline(m_confRemarks,CONF_REMARKS_LEN+1,';');
} 

/////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void CConfStartCont3::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pConfStart3Node = pFatherNode->AddChildNode("CONF_START_3");
	pConfStart3Node->AddChildNode("REMARK",m_confRemarks);
}
//#endif

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CConfStartCont3::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"REMARK",m_confRemarks,REMARK_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

const char*  CConfStartCont3::NameOf() const                
{
	return "CConfStartCont3";
}

////////////////////////////////////////////////////////////////////////////

const char* ACCCDREventConfStartCont3::GetConfRemarks() const
{
	return m_confRemarks;
}

////////////////////////////////////////////////////////////////////////////

void CConfStartCont3::SetConfRemarks (const char* remarks)
{
	strncpy(m_confRemarks, remarks, sizeof(m_confRemarks) - 1);
	m_confRemarks[sizeof(m_confRemarks) - 1] = '\0';
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////// ACCCDREventConfStartCont4 ////////////////////

ACCCDREventConfStartCont4::ACCCDREventConfStartCont4()
{
    m_NumericConfId[0] = '\0'; 
	m_user_password[0] = '\0';
	m_chair_password[0] = '\0';
	
	
	m_ConfBillingInfo[0] = '\0';
	for(int i = 0 ; i < MAX_CONF_INFO_ITEMS ; i++)
	{
		m_ContactInfo[i][0] = '\0';
	}
}
////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont4:: ~ACCCDREventConfStartCont4()
{

}
////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont4::ACCCDREventConfStartCont4(const ACCCDREventConfStartCont4 &other)
{
	strncpy(m_NumericConfId, other.m_NumericConfId,NUMERIC_CONFERENCE_ID_LEN);
	strncpy(m_user_password, other.m_user_password,CONFERENCE_ENTRY_PASSWORD_LEN);
	strncpy(m_chair_password, other.m_chair_password,CONFERENCE_ENTRY_PASSWORD_LEN);
//	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)

//		m_ContactInfo[i] = other.m_ContactInfo[i];
//	m_ConfBillingInfo = other.m_ConfBillingInfo;
	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
		strncpy(m_ContactInfo[i], other.m_ContactInfo[i],CONF_INFO_ITEM_LEN);
	strncpy(m_ConfBillingInfo, other.m_ConfBillingInfo,CONF_BILLING_INFO_LEN);

}
////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont4& ACCCDREventConfStartCont4:: operator = (const ACCCDREventConfStartCont4& other)
{
	strncpy(m_NumericConfId, other.m_NumericConfId,NUMERIC_CONFERENCE_ID_LEN);
	strncpy(m_user_password, other.m_user_password,CONFERENCE_ENTRY_PASSWORD_LEN);
	strncpy(m_chair_password, other.m_chair_password,CONFERENCE_ENTRY_PASSWORD_LEN);

/*	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
		m_ContactInfo[i] = other.m_ContactInfo[i];
	m_ConfBillingInfo = other.m_ConfBillingInfo;*/

	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
		strncpy(m_ContactInfo[i], other.m_ContactInfo[i],CONF_INFO_ITEM_LEN);
	strncpy(m_ConfBillingInfo, other.m_ConfBillingInfo,CONF_BILLING_INFO_LEN);
	


	return *this;
}
////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventConfStartCont4::GetContactInfo(int ContactNumber)  const
{
	if(ContactNumber >= 0 && ContactNumber < MAX_CONF_INFO_ITEMS)
		return m_ContactInfo[ContactNumber];
	//	return m_ContactInfo[ContactNumber].GetString();

	return NULL;
}
////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventConfStartCont4::GetConfBillingInfo() const
{	
	//return m_ConfBillingInfo.GetString();	
	return m_ConfBillingInfo;	
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

///////////////////////  CConfStartCont4(UpdateConfContactInfo) ////////////////////////////
CConfStartCont4::CConfStartCont4()
{
}
////////////////////////////////////////////////////////////////////////////
CConfStartCont4::~CConfStartCont4()
{
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont4::Serialize(WORD format, std::ostream &m_ostr, BYTE flag, DWORD apiNum)
{
 
//char  m_NumericConfId[NUMERIC_CONFERENCE_ID_LEN]; 

   m_ostr << "conference numeric ID:"<<m_NumericConfId  << "\n";
   m_ostr << "user password:"<<m_user_password  << "\n";
   m_ostr << "chair password:"<<m_chair_password  << "\n";
/*	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
	{
	  m_ostr << "user defined: "<<i+1;
	  m_ContactInfo[i].SerializeForCDR(format,m_ostr);
	  m_ostr<< "\n";

	 }
	 m_ostr << "Billing info: ";
     m_ConfBillingInfo.SerializeForCDR(format,m_ostr);
	 m_ostr<< "\n";

*/
   	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
	{
	  m_ostr << "user defined"<<i+1<<":"<<m_ContactInfo[i]<< "\n";

	 }
	 m_ostr << "Billing info: "<<m_ConfBillingInfo << "\n;\n\n";


}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont4::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
    BOOL isHidePsw = NO;
	std::string key_hide = "HIDE_CONFERENCE_PASSWORD";
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);

	 m_ostr << m_NumericConfId  << ",";
	m_ostr << (isHidePsw ? "" : m_user_password)  << ",";
	m_ostr << (isHidePsw ? "" : m_chair_password) << ",";
  /*for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
	  m_ContactInfo[i].SerializeForCDR(format,m_ostr);
      m_ostr<< ","; 
      m_ConfBillingInfo.SerializeForCDR(format,m_ostr);
	  m_ostr<< ";\n"; */
	  
	   for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
	   {
			m_ostr <<m_ContactInfo[i]<< ","; 
	   }
	   
	 m_ostr <<m_ConfBillingInfo<< ";\n"; 
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont4::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{

	m_istr.getline(m_NumericConfId,NUMERIC_CONFERENCE_ID_LEN+1,',');
	m_istr.getline(m_user_password,CONFERENCE_ENTRY_PASSWORD_LEN+1,',');
	m_istr.getline(m_chair_password,CONFERENCE_ENTRY_PASSWORD_LEN+1,',');
	/*for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
	{
		m_ContactInfo[i].DeSerializeForCDR(format,m_istr);
		m_istr.ignore(1);
	}
	    m_ConfBillingInfo.DeSerializeForCDR(format,m_istr);*/

	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++)
	{
		m_istr.getline(m_ContactInfo[i],CONF_INFO_ITEM_LEN+1,',');
	}
	m_istr.getline(m_ConfBillingInfo,CONF_BILLING_INFO_LEN+1,';');
	   



}

/////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void CConfStartCont4::SerializeXml(CXMLDOMElement* pFatherNode)
{
	char szNodeName[20];

	CXMLDOMElement* pConfStart4Node = pFatherNode->AddChildNode("CONF_START_4");
	pConfStart4Node->AddChildNode("NUMERIC_ID",m_NumericConfId);
	pConfStart4Node->AddChildNode("ENTRY_PASSWORD",m_user_password);
	pConfStart4Node->AddChildNode("LEADER_PASSWORD",m_chair_password);

	CXMLDOMElement *pChild = pConfStart4Node->AddChildNode("CONTACT_INFO_LIST");

	strcpy(szNodeName, "CONTACT_INFO");
	for(int i = 0; i < MAX_CONF_INFO_ITEMS; i++) {
		if( strlen(m_ContactInfo[i]) )
		{
			if (i!=0)
				snprintf(szNodeName, sizeof(szNodeName), "%s_%d",szNodeName, i+1);
			pChild->AddChildNode(szNodeName,m_ContactInfo[i]);
			strcpy(szNodeName, "CONTACT_INFO");
		}
	}

	pConfStart4Node->AddChildNode("BILLING_DATA",m_ConfBillingInfo);
}
//#endif

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CConfStartCont4::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pChild = NULL;
	char szNodeName[20];

	GET_VALIDATE_CHILD(pActionNode,"NUMERIC_ID",m_NumericConfId,_0_TO_NUMERIC_CONFERENCE_ID_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"ENTRY_PASSWORD",m_user_password,_0_TO_CONFERENCE_ENTRY_PASSWORD_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"LEADER_PASSWORD",m_chair_password,_0_TO_CONFERENCE_ENTRY_PASSWORD_LENGTH);

	CXMLDOMElement *pNodeList = NULL;

	GET_CHILD_NODE(pActionNode, "CONTACT_INFO_LIST", pNodeList);
	if( pNodeList )
	{
		int i;
		// cleanup
		for( i=0; i<MAX_CONF_INFO_ITEMS; i++ )
			memset(m_ContactInfo[i],'\0',CONF_INFO_ITEM_LEN);

		strcpy(szNodeName, "CONTACT_INFO");
		GET_FIRST_CHILD_NODE(pNodeList, szNodeName, pChild);

		i = 0;
		while(  i < MAX_CONF_INFO_ITEMS  )
		{
			if (pChild)
			{
				GET_VALIDATE_CHILD(pChild,szNodeName,m_ContactInfo[i],_0_TO_CONF_INFO_ITEM_LENGTH);
			}
			else
				m_ContactInfo[i][0]='\0';
				
			i++;

			snprintf(szNodeName, sizeof(szNodeName), "%s_%d",szNodeName, i+1);
			GET_NEXT_CHILD_NODE(pNodeList, szNodeName, pChild);
			strcpy(szNodeName, "CONTACT_INFO");
		}
	}

	memset(m_ConfBillingInfo,'\0',CONF_INFO_ITEM_LEN);
	GET_VALIDATE_CHILD(pActionNode,"BILLING_DATA",m_ConfBillingInfo,_0_TO_CONF_INFO_ITEM_LENGTH);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void   CConfStartCont4::SetContactInfo(const char* info,int ContactNumber)
{
	if( info  &&  ContactNumber<MAX_CONF_INFO_ITEMS )
		strncpy(m_ContactInfo[ContactNumber], info,CONF_INFO_ITEM_LEN);
	 
}
////////////////////////////////////////////////////////////////////////////
void   CConfStartCont4::SetConfBillingInfo(const char* BillingInfo)
{
	if(BillingInfo)
	{
		strncpy(m_ConfBillingInfo, BillingInfo, sizeof(m_ConfBillingInfo) - 1);
		m_ConfBillingInfo[sizeof(m_ConfBillingInfo) - 1] = '\0';
	}
	  
}
////////////////////////////////////////////////////////////////////////////
const char*  CConfStartCont4::NameOf() const
{
	return "CConfStartCont4";
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
const char* ACCCDREventConfStartCont4::GetNumericConfId() const
{
 return m_NumericConfId;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont4::SetNumericConfId (const char* NumericConfId )
{
	strncpy(m_NumericConfId,NumericConfId,sizeof(m_NumericConfId) - 1);
	m_NumericConfId[sizeof(m_NumericConfId) - 1] = '\0';

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* ACCCDREventConfStartCont4::GetUser_password() const
{
 return m_user_password;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont4::SetUser_password (const char* user_password )
{
	strncpy(m_user_password,user_password,sizeof(m_user_password) - 1);
	m_user_password[sizeof(m_user_password) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* ACCCDREventConfStartCont4::GetChair_password() const
{
 return m_chair_password;
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont4::SetChair_password (const char* chair_password )
{
	strncpy(m_chair_password,chair_password, sizeof(m_chair_password) - 1);
	m_chair_password[sizeof(m_chair_password) - 1] = '\0';
}


////////////////////////////////////////////////////////////////////////////
/////////////////// ACCCDREventConfStartCont5 //////////////////////////////
////////////////////////////////////////////////////////////////////////////

ACCCDREventConfStartCont5::ACCCDREventConfStartCont5()
{
}
////////////////////////////////////////////////////////////////////////////

ACCCDREventConfStartCont5:: ~ACCCDREventConfStartCont5()
{
}
////////////////////////////////////////////////////////////////////////////

ACCCDREventConfStartCont5::ACCCDREventConfStartCont5(const ACCCDREventConfStartCont5 &other)
{
    m_encryption = other.m_encryption;
}
////////////////////////////////////////////////////////////////////////////

ACCCDREventConfStartCont5& ACCCDREventConfStartCont5:: operator = (const ACCCDREventConfStartCont5& other)
{
    if (this != &other)	{
		m_encryption = other.m_encryption;
	}
	return *this;
}
///////////////////////  CConfStartCont5 ////////////////////////////

CConfStartCont5::CConfStartCont5()
{}

////////////////////////////////////////////////////////////////////////////

CConfStartCont5::~CConfStartCont5()
{}

////////////////////////////////////////////////////////////////////////////

const char*  CConfStartCont5::NameOf() const
{
    return "CConfStartCont5";
}
////////////////////////////////////////////////////////////////////////////

void CConfStartCont5::Serialize(WORD format, std::ostream &ostr, DWORD apiNum)
{
    ostr << (WORD)m_encryption << ";\n";
}
////////////////////////////////////////////////////////////////////////////

void CConfStartCont5::Serialize(WORD format, std::ostream &ostr, BYTE flag, DWORD apiNum)
{
    if (m_encryption)
	    ostr << "Encryption: yes" << "\n;\n\n";
	else
	    ostr << "Encryption: no" << "\n;\n\n";
}
////////////////////////////////////////////////////////////////////////////

void CConfStartCont5::DeSerialize(WORD format, std::istream &istr, DWORD apiNum)
{
    WORD tmp;
	istr >> tmp;
	m_encryption=(BYTE)tmp;
	istr.ignore(1);
}
/////////////////////////////////////////////////////////////////////////////

void CConfStartCont5::SerializeXml(CXMLDOMElement* pFatherNode)
{
    CXMLDOMElement* pConfStart5Node = pFatherNode->AddChildNode("CONF_START_5");
	pConfStart5Node->AddChildNode("ENCRYPTION",m_encryption);
}
/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd

int CConfStartCont5::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
    int nStatus = STATUS_OK;
	GET_VALIDATE_CHILD(pActionNode,"ENCRYPTION",&m_encryption,_0_TO_BYTE);
	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////

void CConfStartCont5::SetIsEncryptedConf(const BYTE is_encryption)
{
    m_encryption = is_encryption;
}
////////////////////////////////////////////////////////////////////////////

BYTE ACCCDREventConfStartCont5::GetIsEncryptedConf() const
{
    return m_encryption;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////// ACCCDREventConfStartCont10 ///////////////////////

ACCCDREventConfStartCont10::ACCCDREventConfStartCont10()
{
    m_confDisplayName[0] = '\0'; 
	m_Avc_Svc = 0;
}
////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont10:: ~ACCCDREventConfStartCont10()
{

}
////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont10::ACCCDREventConfStartCont10(const ACCCDREventConfStartCont10 &other)
{
	strncpy(m_confDisplayName, other.m_confDisplayName,H243_NAME_LEN);
	m_Avc_Svc = other.m_Avc_Svc;
}
////////////////////////////////////////////////////////////////////////////
ACCCDREventConfStartCont10& ACCCDREventConfStartCont10:: operator = (const ACCCDREventConfStartCont10& other)
{
	strncpy(m_confDisplayName, other.m_confDisplayName,H243_NAME_LEN);
	m_Avc_Svc = other.m_Avc_Svc;

	return *this;
}
////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventConfStartCont10::GetConfDisplayName()  const
{
	return m_confDisplayName;
}
/////////////////////////////////////////////////////////////////////////////////
BYTE  ACCCDREventConfStartCont10::GetAvcSvc() const
{
  return m_Avc_Svc;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

///////////////////////  CConfStartCont10////////////////////////////
CConfStartCont10::CConfStartCont10()
{
}
////////////////////////////////////////////////////////////////////////////
CConfStartCont10::~CConfStartCont10()
{
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont10::Serialize(WORD format, std::ostream &m_ostr, BYTE flag, DWORD apiNum)
{
   m_ostr << "conference display name:"<<m_confDisplayName  << "\n";
   m_ostr << "conference media type:"<< m_Avc_Svc << "\n;\n\n";
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont10::Serialize(WORD format, std::ostream &ostr, DWORD apiNum)
{	   
	 ostr << m_confDisplayName << ",";
	 ostr << (WORD)m_Avc_Svc << ";\n";
}
////////////////////////////////////////////////////////////////////////////
void CConfStartCont10::DeSerialize(WORD format, std::istream &istr, DWORD apiNum)
{
	istr.getline(m_confDisplayName,H243_NAME_LEN, ',');
    WORD tmp;
	istr >> tmp;
	m_Avc_Svc=(BYTE)tmp;
	istr.ignore(1);
}

/////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void CConfStartCont10::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pConfStart10Node = pFatherNode->AddChildNode("CONF_START_10");
	pConfStart10Node->AddChildNode("DISPLAY_NAME",m_confDisplayName);
	pConfStart10Node->AddChildNode("CONF_MEDIA_TYPE", m_Avc_Svc, CONF_MEDIA_TYPE_ENUM);
}
//#endif

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CConfStartCont10::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"DISPLAY_NAME",m_confDisplayName,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"CONF_MEDIA_TYPE", &m_Avc_Svc, CONF_MEDIA_TYPE_ENUM);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
const char*  CConfStartCont10::NameOf() const
{
	return "CConfStartCont10";
}

////////////////////////////////////////////////////////////////////////////
void CConfStartCont10::SetConfDisplayName (const char* confDisplayName )
{
	strncpy(m_confDisplayName,confDisplayName,sizeof(m_confDisplayName) - 1);
	m_confDisplayName[sizeof(m_confDisplayName) - 1] = '\0';
}
////////////////////////////////////////////////////////////////////////////
void  CConfStartCont10::SetAvcSvc (const BYTE eAvcSvc)
{
	m_Avc_Svc = eAvcSvc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
