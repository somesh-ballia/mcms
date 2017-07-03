#ifndef OPCODESMCMSCARDMNGRIPMEDIA_H_
#define OPCODESMCMSCARDMNGRIPMEDIA_H_

// =======================================
// =========== ConfParty range ===========
// =======================================
//#define CONF_PARTY_CARDMNGR_MEDIA_FIRST_OPCODE	1070000	

#define CONFPARTY_CM_OPEN_UDP_PORT_REQ						  	1070001
#define CONFPARTY_CM_UPDATE_UDP_ADDR_REQ						1070002
#define CONFPARTY_CM_CLOSE_UDP_PORT_REQ						  	1070003
#define CONFPARTY_CM_INIT_ON_LYNC_CALL_REQ                    	1070004
#define CONFPARTY_CM_INIT_ON_AVMCU_CALL_REQ                   	1070005
#define CONFPARTY_CM_MUX_ON_AVMCU_CALL_REQ                    	1070006
#define CONFPARTY_CM_DMUX_ON_AVMCU_CALL_REQ                   	1070007


//#define CONF_PARTY_CARDMNGR_MEDIA_LAST_OPCODE   1079999


// =======================================
// =========== RSRCALLOC range ===========
// =======================================
//#define RESOURCE_CARDMNGR_IP_MEDIA_FIRST_OPCODE	9030000
#define  KILL_UDP_PORT_REQ	 						9030010

//#define RESOURCE_CARDMNGR_IP_MEDIA_LAST_OPCODE	9039999




#endif /*OPCODESMCMSCARDMNGRIPMEDIA_H_*/
