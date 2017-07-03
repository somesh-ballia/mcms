//+========================================================================+
//                            UCAPNS.H                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       UCAPNS.H                                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 2/17/00    |                                                      |
//+========================================================================+

#ifndef _UCAPNS
#define _UCAPNS

// country codes here
const WORD	W_COUNTRY_CODE_USA			= 0x00B5;
const WORD	W_COUNTRY_CODE_ISRAEL		= 0x0058;

// manufacturer codes here
const WORD	W_MANUFACT_CODE_PICTURETEL	= 0x0100;
const WORD	W_MANUFACT_CODE_POLYCOM		= 0x3123;

#define COUNTRY_CODE_USA_BYTE_1			0xB5
#define COUNTRY_CODE_USA_BYTE_2			0x00

// Header length for NS_CAP and NS_COM messages: 
// (2 bytes for country code + 2 bytes for manufacturer code) = 4 bytes
#define NS_MSG_HEADER_LEN	4

// Polycom H.320 stack version number 
const BYTE	VERSION_2_0 = 0x14;
const BYTE	VERSION_2_1 = 0x15; 
const BYTE	VERSION_2_2 = 0x16; 
const BYTE	VERSION_2_3 = 0x17; 
 
//reseller number
const BYTE	POLYCOM   = 1; 
const BYTE	PLOYSPAN  = 2; 
const BYTE	M3        = 3; 
const BYTE	ACCORD    = 4; 

// Polycon NS cap opcodes
const BYTE	NS_CAP_H263_QCIF_ANNEX_I= 0x00; // NS H263 QCIF Annex I video possibility opcode
const BYTE	NS_CAP_H263_CIF_ANNEX_I	= 0x01; // NS H263 CIF Annex I video possibility opcode
const BYTE	NS_CAP_H263_4CIF_ANNEX_I= 0x02; // NS H263 4CIF Annex I video possibility opcode
const BYTE	NS_CAP_H263_QCIF_ANNEX_T= 0x04; // NS H263 QCIF Annex T video possibility opcode
const BYTE	NS_CAP_H263_CIF_ANNEX_T	= 0x05; // NS H263 CIF Annex T video possibility opcode
const BYTE	NS_CAP_H263_4CIF_ANNEX_T= 0x06; // NS H263 4CIF Annex T video  opcode
const BYTE	NS_CAP_VGA_800X600		= 0x18; // NS Support of 800x600 VGA mode
const BYTE	NS_CAP_VGA_1024X768		= 0x19; // NS Support of 1024x768 VGA mode
const BYTE	NS_CAP_VGA_1280X1024	= 0x1A; // NS Support of 1280x1024 VGA mode
const BYTE	NS_CAP_VIDEO_STREAM_2	= 0x2C; // Dual Video Streams possibility
const BYTE	NS_CAP_VIDEO_STREAM_3	= 0x2D; // Three Video Streams possibility
const BYTE	NS_CAP_VIDEO_STREAM_4	= 0x2E; // Four Video Streams possibility
const BYTE	NS_CAP_MERCURY			= 0x2F; // VisualConcertPC video possibility opcode
const BYTE	NS_CAP_R2D2				= 0x33; // VisualConcertFX video possibility opcode
const BYTE	NS_CAP_ACCORD2POLYCOM	= 0x6C; // Accord sender to Polycom cap byte

// PictureTel NS cap opcodes
const BYTE	NS_CAP_SIREN7			= 0x87; // all audio Siren7 algorithms (at 16/24/32 kbps)
const BYTE	NS_CAP_SIREN14			= 0x88; // all audio Siren14 algorithms (at 24/32/48 kbps)
const BYTE	NS_CAP_SIREN716			= 0x8D; // audio Siren716 algorithm (16 kbps)
const BYTE	NS_CAP_SIREN724			= 0x8E; // audio Siren724 algorithm (24 kbps)
const BYTE	NS_CAP_SIREN732			= 0x8F; // audio Siren732 algorithm (32 kbps)
const BYTE	NS_CAP_SIREN1424		= 0x91; // audio Siren1424 algorithm (24 kbps)
const BYTE	NS_CAP_SIREN1432		= 0x92; // audio Siren1432 algorithm (32 kbps)
const BYTE	NS_CAP_SIREN1448		= 0x93; // audio Siren1448 algorithm (48 kbps)
const BYTE	NS_CAP_PEOPLE_CONTENT	= 0x95; // PeopleContentVersion0 capability
const BYTE	NS_CAP_DBC2         	= 0x9D; // DBC2 capability
const BYTE	NS_CAP_H26L             = 0x9E; // H.26L capabilities
const BYTE	NS_FIELD_DROP           = 0x9F; // Field Drop Capability

// PictureTel NS com opcodes
const BYTE	NS_COM_SIREN716_ON		= 0x87; // Siren716 audio ON cmd
const BYTE	NS_COM_SIREN724_ON		= 0x88; // Siren724 audio ON cmd
const BYTE	NS_COM_SIREN732_ON		= 0x89; // Siren732 audio ON cmd
const BYTE	NS_COM_SIREN1424_ON		= 0x8A; // Siren1424 audio ON cmd
const BYTE	NS_COM_SIREN1432_ON		= 0x8B; // Siren1432 audio ON cmd
const BYTE	NS_COM_SIREN1448_ON		= 0x8C; // Siren1432 audio ON cmd
const BYTE	NS_COM_CONTENT_ON		= 0x8E; // ContentVisualization ON cmd
const BYTE	NS_COM_DBC2_ON	    	= 0xA4;// DBC2 ON cmd
const BYTE	NS_COM_DBC2_OFF	    	= 0xA5;// DBC2 OFF cmd
const BYTE	NS_COM_H26L_ON	    	= 0xA6;// H.26L ON cmd

// ISRAEL/Polycom NS cap opcodes - for internal MCMS use
const BYTE  NS_CAP_VTX				= 0x00; //VTX Cap

// ISRAEL/Polycom NS com opcodes - for internal MCMS use
// VTX com opcodes
const BYTE   NS_COM_G7222_0660				= 0x00; // Not supported by VTX100
const BYTE   NS_COM_G7222_0885				= 0x01;
const BYTE   NS_COM_G7222_1265				= 0x02;
const BYTE   NS_COM_G7222_1425				= 0x03;
const BYTE   NS_COM_G7222_1585				= 0x04;
const BYTE   NS_COM_G7222_1825				= 0x05;
const BYTE   NS_COM_G7222_1985				= 0x06;
const BYTE   NS_COM_G7222_2305				= 0x07;	// Not supported by VTX100
const BYTE   NS_COM_G7222_2385				= 0x08;	// Not supported by VTX100

// Polycom Public Non-standard Command Listing
const BYTE   NS_COM_ROLE_LABEL					= 0x8F;
const BYTE   NS_COM_ROLE_TOKEN_ACQUIRE			= 0x90;
const BYTE   NS_COM_ROLE_TOKEN_ACQUIRE_ACK		= 0x91;
const BYTE   NS_COM_ROLE_TOKEN_ACQUIRE_NAK		= 0x92;
const BYTE   NS_COM_ROLE_TOKEN_RELEASE			= 0x93;
const BYTE   NS_COM_ROLE_TOKEN_RELEASE_ACK		= 0x94;
const BYTE   NS_COM_ROLE_TOKEN_WITHDRAW			= 0x95;
const BYTE   NS_COM_ROLE_TOKEN_WITHDRAW_ACK		= 0x96;
const BYTE   NS_COM_ROLE_PROVIDER_IDENTITY		= 0x97;
const BYTE   NS_COM_NO_ROLE_PROVIDER			= 0x98;
const BYTE   NS_COM_MEDIA_FLOW_CNTL_COM			= 0x99;
const BYTE   NS_COM_MEDIA_FLOW_CNTL_INDICATION	= 0x9A;
const BYTE   NS_COM_AMSC_ON						= 0x9B;
const BYTE   NS_COM_AMSC_RATE_CHANGE			= 0x9C;
const BYTE   NS_COM_AMSC_OFF					= 0x9D;
const BYTE   NS_COM_AMSC_H230_C_I				= 0x9E;
const BYTE   NS_COM_AMSC_H230_MBE				= 0x9F;

const BYTE   NS_COM_PP_H221_ESC_TBL_ON			= 0xA0;
const BYTE   NS_COM_PP_H221_ESC_TBL_OFF			= 0xA1;
const BYTE   NS_COM_MEDIA_PRODUCER_STATUS		= 0xA2;
const BYTE   NS_COM_REQUEST_RATE_CHANGE			= 0xA3;


#define H26L_CIF    1
#define H26L_CIF_4  2



#endif /* _UCAPNS  */
