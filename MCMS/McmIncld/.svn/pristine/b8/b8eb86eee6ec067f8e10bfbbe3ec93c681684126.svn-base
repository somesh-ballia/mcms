//+                                                                        +
//                       IpMngrOpcodes.h                                   |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IpMngrOpcodes.h                                             |
// SUBSYSTEM:  CS/GateKeeper                                               |
// PROGRAMMER: Guy D,													   |
// Date : 15/5/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+                                                                        +                        
/*

This header file contains all opcodes between CS and GateKeeper processes 
This will include all RAS + GK Mngr + DNS Mngr + PROXI Mngr Opcodes

  12000001	- 12666666: requests from GateKeeper to CS		   
  12666667	- 12999999: indication from CS to GateKeeper			   

*/

#ifndef  __IPMNGROPCODES_H__
#define  __IPMNGROPCODES_H__


////////////////////////////////////////////////
////////////   GKmngr <-----> CS   /////////////////
////////////////////////////////////////////////

/// Req
#define PROCESS_GATE_KEEPER_MNGR_FIRST_OPCODE_IN_RANGE			12000001
#define GATE_KEEPER_MNGR_TO_CS_FIRST_OPCODE_IN_RANGE			12000002
#define H323_CS_RAS_FIRST_REQ							12000003

#define H323_CS_RAS_GRQ_REQ										12000003 // H323_RAS_GRQ_GKMANAGER_REQ
#define H323_CS_RAS_RRQ_REQ										12000004 // H323_RAS_RRQ_GKMANAGER_REQ
#define H323_CS_RAS_URQ_REQ										12000005 // H323_RAS_URQ_GKMANAGER_REQ
#define H323_CS_RAS_URQ_RESPONSE_REQ							12000006 // H323_RAS_URQ_RESPONSE_GKMANAGER_REQ
#define H323_CS_RAS_DRQ_RESPONSE_REQ							12000007 // H323_RAS_DRQ_RESPONSE_GKMANAGER_REQ
#define H323_CS_RAS_LRQ_REQ										12000008 // H323_RAS_LRQ_GKMANAGER_REQ
#define H323_CS_RAS_ARQ_REQ										12000009 // H323_RAS_ARQ_GKMANAGER_REQ
#define H323_CS_RAS_BRQ_REQ										12000010 // H323_RAS_BRQ_GKMANAGER_REQ
#define H323_CS_RAS_IRR_REQ										12000011 // H323_RAS_IRR_GKMANAGER_REQ
#define H323_CS_RAS_IRR_RESPONSE_REQ							12000012 // H323_RAS_IRR_RESPONSE_GKMANAGER_REQ
#define H323_CS_RAS_DRQ_REQ										12000013 // H323_RAS_DRQ_GKMANAGER_REQ
#define H323_CS_RAS_LRQ_RESPONSE_REQ							12000014 // H323_RAS_LRQ_FROM_GK_GKMANAGER_REQ
#define H323_CS_RAS_RAI_REQ										12000015 // H323_RAS_RAI_GKMANAGER_REQ
#define H323_CS_RAS_REMOVE_GK_CALL_REQ							12000016 // H323_RAS_REMOVE_GK_CALL_REQ
#define H323_CS_RAS_BRQ_RESPONSE_REQ							12000017 // H323_RAS_BRQ_RESPONSE_GKMANAGER_REQ
#define H323_CS_RAS_LAST_REQ						     12000030

#define GATE_KEEPER_MNGR_TO_CS_LAST_OPCODE_IN_RANGE             12666666

/// Ind
#define CS_TO_GATE_KEEPER_MNGR_FIRST_OPCODE_IN_RANGE            12666667

#define H323_CS_RAS_FIRST_IND							        12666668
#define H323_CS_RAS_GRQ_IND										12666668 // H323_RAS_GRQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_RRQ_IND										12666669 // H323_RAS_RRQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_URQ_IND										12666670 // H323_RAS_URQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_LRQ_IND										12666671 // H323_RAS_LRQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_ARQ_IND										12666672 // H323_RAS_ARQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_BRQ_IND										12666673 // H323_RAS_BRQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_DRQ_IND										12666674 // H323_RAS_DRQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_FAIL_IND									12666675 // H323_RAS_FAIL_GATEKEEPER_IF_IND
#define H323_CS_RAS_TIMEOUT_IND									12666676 // H323_RAS_TIMEOUT_GATEKEEPER_IF_IND
#define H323_CS_RAS_GKURQ_IND									12666677 // H323_RAS_GKURQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_GKDRQ_IND									12666678 // H323_RAS_GKDRQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_GKBRQ_IND									12666679 // H323_RAS_GKBRQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_GKLRQ_IND									12666680 // H323_RAS_LRQ_FROM_GK_GATEKEEPER_IF_IND
#define H323_CS_RAS_RAC_IND										12666681 // H323_RAS_RAC_GATEKEEPER_IF_IND
#define H323_CS_RAS_GKIRQ_IND									12666682 // H323_RAS_GKIRQ_GATEKEEPER_IF_IND
#define H323_CS_RAS_LAST_IND							 12666690
#define CS_TO_GATE_KEEPER_MNGR_LAST_OPCODE_IN_RANGE             12999999
#define PROCESS_GATE_KEEPER_MNGR_LAST_OPCODE_IN_RANGE           13000000

#endif
