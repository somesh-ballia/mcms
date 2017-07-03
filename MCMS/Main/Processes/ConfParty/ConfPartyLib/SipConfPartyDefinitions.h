/*======================================================================== 
//                            SipConfPartyDefinitions.H                                          
//            Copyright 1995 Polycom Technologies Ltd.                       
//                   All Rights Reserved.                                  
//------------------------------------------------------------------------ 
// NOTE: This software contains valuable trade secrets and proprietary     
// information of Polycom Technologies Ltd. and is protected by law.        
// It may not be copied or distributed in any form or medium, disclosed    
// to third parties, reverse engineered or used in any manner without      
// prior written authorization from Polycom Technologies Ltd.               
//------------------------------------------------------------------------ 
// FILE:       SipConfPartyDefinitions.h                                                             
// SUBSYSTEM:  MCMS                                                            
// PROGRAMMER: Uri A                                                            
//------------------------------------------------------------------------ 
// Who | Date       | Description                                          
//------------------------------------------------------------------------ 
//     | 13 - 7 - 05|  modified consts                                     
//=======================================================================*/
#ifndef __SIPCONFPARTYDEFINES_H___
#define __SIPCONFPARTYDEFINES_H___


//for sip conferencing limitation
typedef enum {
		kNotAnAdvancedVideoConference = 0,
		kH264VswFixed,
		kEpcVswFixed,
		kAutoVsw,
//		kSoftwareCp,
//		kCop,
//		kQuadView
}EAdvancedVideoConferenceType;

static char* FormatConferenceType[] = {
		"Unknown",
		"VSW set to H264 fixed",
		"VSW set to EPC fixed",
		"VSW set to auto",
//		"Software CP",
//		"COP",
//		"QUAD view"	
};

typedef enum
{
	kNone = 0,
	kLocal,
	kRelay,
	kFirewall,
}EIceConnectionType;


#endif //_SIPCONFPARTYDEFINES_H__
