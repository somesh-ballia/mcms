#ifndef OPCODESMCMSBONDING_H_
#define OPCODESMCMSBONDING_H_



// =======================================
// =========== CONFPARTY range ===========
// =======================================

//#define CONF_PARTY_BONDING_FIRST_OPCODE	1080000

/* Requests to BONDING. See file bonding.h */

#define BND_CONNECTION_INIT	        1080010  /* BndConnInit */
#define BND_ADD_CHANNEL		        1080011  /* BndAddChan  */
#define BND_CHANEL_DISCONNECT	    1080012	 /* Bond Channel Disconnect - currently not in use at RMX */
#define BND_CHANEL_REJECT		    1080013	 /* Bond Channel Reject - currently not in use at RMX */
#define BND_ACK_PARAMS		        1080014  /* BndAckParams - currently not in use at RMX */
#define BND_DISCONNECT_CALL	        1080015  /* BndDiscReq   */
#define BND_ABORT_CALL		        1080016  /* BndAbortInd  */

/* Indications from BONDING. See file bonding.h */

#define BND_REQ_PARAMS			    1080020  /* BndReqParams  - currently not in use at RMX */
#define BND_END_NEGOTIATION		    1080021  /* BndInitInd		*/
#define BND_REMOTE_LOCAL_ALIGNMENT  1080022  /* BndRLAlign		*/
#define BND_CALL_FAIL			    1080023  /* BndCallFail	   */
#define BND_END_DISCONNECT  	    1080024  /* BndDiscInd		*/


//#define CONF_PARTY_BONDING_LAST_OPCODE	1080999

/* #define BND_DBG_MSG			    1080017  toggle: on off - removed on RMX*/
/* #define BND_PROFILER			    1080025   BndProfiler - removed on RMX	   */


#endif /*OPCODESMCMSBONDING_H_*/
