//
//	OpcodesMcmsCardMngrICE.h
//

#ifndef __OPCODES_MCMS_CARD_MNGR_ICE_h___
#define __OPCODES_MCMS_CARD_MNGR_ICE_h___


// =======================================
// ============= CARDS range =============
// =======================================
//#define CARDS_CARDMNGR_ICE_FIRST_OPCODE			5060000
#define ICE_INIT_REQ						   		5060010			// Init ICE stack

#define ICE_INIT_IND								5060030			// Return init ICE stack status

#define ICE_STATUS_IND								5060050

//#define CARDS_CARDMNGR_ICE_LAST_OPCODE			5069999



// =======================================
// =========== ConfParty range ===========
// =======================================
//#define CONF_PARTY_ICE_FIRST_OPCODE				1090000

#define ICE_MAKE_OFFER_REQ                          1090001			// use before sending INVITE in dial out
#define ICE_PROCESS_ANSWER_REQ                      1090002			// use when receiving OK for INVITE
#define ICE_MAKE_ANSWER_REQ                         1090003			// use when receiving INVITE - dial in
#define ICE_CLOSE_SESSION_REQ                       1090004
#define ICE_MODIFY_SESSION_ANSWER_REQ				1090005 		// Remote change the SDP or when receiving Re-INVITE from remote
#define ICE_MODIFY_SESSION_OFFER_REQ				1090006 		// Local RMX change the SDP


#define ICE_MAKE_OFFER_IND                          1090021			// return SDP with candidates for INVITE (RMX initialize INVITE)
#define ICE_PROCESS_ANSWER_IND                      1090022			// return status of process request
#define ICE_MAKE_ANSWER_IND                         1090023			// return SDP with candidates for OK (RMX receive INVITE or Re-INVITE)
#define ICE_CLOSE_SESSION_IND                       1090024			// return status of close request
#define ICE_MODIFY_SESSION_ANSWER_IND				1090025 		// Remote change the SDP
#define ICE_MODIFY_SESSION_OFFER_IND				1090026 		// Local RMX change the SDP
#define ICE_REINVITE_IND                            1090027			// used when ICE stack initiate re-INVITE event
#define ICE_ERR_IND                                 1090028			// used to send errors when occurs  during calls
#define ICE_CHECK_COMPLETE_IND						1090029
#define ICE_BANDWIDTH_EVENT_IND						1090030			// used when ICE stack returns BW event in middle of the call
#define ICE_INSUFFICIENT_BANDWIDTH_IND				1090031			// used when ICE stack returns INSUFFICIENT BW event in setting of the call
#define ICE_SESSION_INDEX_IND						1090032         // use to notify the MCMS with new session index for it to send it in case of cancel request

//#define CONF_PARTY_ICE_LAST_OPCODE   				1090999



#endif	// __OPCODES_MCMS_CARD_MNGR_ICE_h___

