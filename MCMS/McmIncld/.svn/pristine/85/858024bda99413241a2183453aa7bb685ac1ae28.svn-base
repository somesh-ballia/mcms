
#ifndef  MUXINT_H
#define  MUXINT_H

// MCMS <--> ART RMX API 
// relevant field moved to top - start here 


//============================================================================================
//==== general data structs ====
//============================================================================================
typedef struct
{
    APIU32   number_of_bytes;
} BAS_CMD_DESC;  
//============================================================================================
typedef struct
{
  BAS_CMD_DESC comm_mode_bas;
} COMM_MODE_S;  
//============================================================================================
typedef struct
{
  BAS_CMD_DESC caps_bas;
} CAPABILITIES_S;  
//============================================================================================
typedef struct
{
  BAS_CMD_DESC h230_bas;
} H_230_S;  
//============================================================================================
typedef struct
{
  // channel number is always 0 in bonding, bit shift is irrelevant
  // -------------------------------------------------------------
  // APIU32 channel_number;  will probably be 0 for bonding 
  // APIU32 bit_shift;  do we need it at RMX? 
  APIU32 source; // 0 - local , 1 - remote 
}  H221_SYNC_S;
//============================================================================================
typedef struct
{
  APIU32   number_of_bytes;     
} BAD_REQUEST_INFO_DESC;
//===========================================================================================
typedef struct
{
  APIU32   number_of_bytes;     
} SNMP_INFO_DESC;


//============================================================================================
//==== setting of communication mode ====
//============================================================================================
// opcodes uses the struct: SET_XMIT_MODE 
// direction: mcms ==> mux   
// description:
//  Setting the transmit communication mode - mux signals the bas command to the remote, and adjusts itself to send the new mode.
 
//change from mgc: fields removed.
//-------------------------------------------------------------------------------------------
typedef struct
{
    COMM_MODE_S xmit_mode;
} SET_XMIT_MODE_S;
//============================================================================================
// opcodes uses the struct:SET_RCV_MODE  
// direction: mcms ==> mux  
// description:
// Setting the receive communication mode - mux adjusts itself to receive the new mode.
//
// change from mgc: add support for asymetric mode, was defined but not used in mgc
//--------------------------------------------------------------------------------------------
typedef struct
{
    COMM_MODE_S receive_mode;
} SET_RCV_MODE_S;
//============================================================================================
// opcodes uses the struct: REMOTE_XMIT_MODE   
// direction: mux ==> mcms    
// description:
// new comm mode received from remote, the new comm mode send to mcms only if changed.
//
// change from mgc: no magor change 
//--------------------------------------------------------------------------------------------
typedef struct
{
    COMM_MODE_S remote_xmit_mode;
} REMOTE_XMIT_MODE_S;


//============================================================================================
//==== capabilities exchange ====
//============================================================================================
// opcodes uses the struct: EXCHANGE_CAPS  
// direction: mcms ==> mux    
// description: mux signals the local capability BAS sequence to the remote
///// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
    CAPABILITIES_S local_caps;
} EXCHANGE_CAPS_S;
//============================================================================================
// opcodes uses the struct: REMOTE_BAS_CAPS  
// direction: mux ==> mcms    
// description: new capabilities set received from remote, mux send the new set to mcms
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
    CAPABILITIES_S remote_caps;
} REMOTE_BAS_CAPS_S;

//============================================================================================
// opcodes uses the struct: REMOTE_NS_CAPS  
// direction: mux ==> mcms   
// description: new capabilities set received from remote, mux send the new set to mcms
// change from mgc: will probably not be used, not be separated from REMOTE_BAS_CAPS_S as mgc 
//--------------------------------------------------------------------------------------------
typedef struct
{
    CAPABILITIES_S remote_ns_caps;
} REMOTE_NS_CAPS_S;
//============================================================================================

//
//============================================================================================
//==== H230 C&I  ====
//============================================================================================
// opcodes uses the struct:SEND_H_230   
// direction: mcms ==> mux    
// description:Send over to the remote H.230 commands 
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  H_230_S h230_command;
} SEND_H_230_S;
//============================================================================================
// opcodes uses the struct: REMOTE_H_230
// direction: mux ==> mcms   
// description: any H.230 sequence received from remote. filtering defined in HLD 
// change from mgc: not in use at RMX 
//--------------------------------------------------------------------------------------------
//
typedef struct
{
  H_230_S h230_indication;
} REMOTE_H_230_S;
//
//============================================================================================
// opcodes uses the struct: REMOTE_CI_S   
// direction: mux ==> mcms    
// description: A CI was received from the remote. filtering defined in HLD 
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  H_230_S h230_indication;
} REMOTE_CI_S;
//===========================================================================================


//============================================================================================
//==== init connection  ====
//============================================================================================
// opcodes uses the struct: H221_INIT_COMM 
// direction: mcms ==> mux    
// description: open connection and sync, send capbilities, comm mode , and h230 to the remote initial transmit mode
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  CAPABILITIES_S        local_caps;
  COMM_MODE_S           initial_xmit_mode;
  H_230_S               initial_h230;

  APIU32                  channel_width;    
  APIU32                  restrict_type;
  APIU32    			  additional_flags;//indication for encryption 
} H221_INIT_COMM_S;

//============================================================================================
// opcodes uses the struct: END_INIT_COMM 
// direction: mux ==> mcms    
// description: initial remote capabilities received
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  CAPABILITIES_S        remote_caps;
  APIU32                restrict_type; 
} END_INIT_COMM_S;
//------------------------------------------------------------------------


//============================================================================================
//==== H221 sync ====
//============================================================================================
// opcodes uses the struct: LOCAL_MFRAME_SYNC  
// direction: mux ==> mcms   
// description: initial local h221 sync
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  H221_SYNC_S  sync_info;
} LOCAL_MFRAME_SYNC_S;
//============================================================================================
// opcodes uses the struct: REMOTE_MFRAME_SYNC 
// direction: mux ==> mcms    
// description: initial remote h221 sync
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  H221_SYNC_S  sync_info;
} REMOTE_MFRAME_SYNC_S;
//============================================================================================
// opcodes uses the struct:SYNCH_LOST   
// direction: mux ==> mcms    
// description: local h221 sync lost
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  H221_SYNC_S  sync_info;
} SYNC_LOST_S;
//============================================================================================
//==== encryption signaling ====
//============================================================================================
// opcodes uses the struct:  
// direction: mcms ==> mux   
// description:Send over to the remote, ECS channel encryption command 
// change from mgc: mcms encode the message in asn1 format
//--------------------------------------------------------------------------------------------
typedef struct
{
  APIU32  p_opcode; // this is the P opcode, in order to inform the mux without parsing the asn1_message
  APIU32  len; // length of asn1_message followed 
  APIU8*  asn1_message;// this is a buffer of bytes contains encoded Asn1 format message 
}SET_ECS_S;
//============================================================================================
// opcodes uses the struct:   
// direction: mux ==> mcms    
// description: Any Encryption Commands sequence received from remote. The MUX does not filter these msgs 
// change from mgc: mux pass the message as received in Asn1 format
//--------------------------------------------------------------------------------------------
typedef struct
{
  APIU32  len; //length of asn1_message followed 
  APIU8*  asn1_message; //this is a buffer of bytes contains encoded Asn1 format message 
} REMOTE_ECS_SE;
//============================================================================================
// opcodes uses the struct:   
// direction: mcms ==> mux    
// description: shared secret
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  APIU8  msginfo[32];
} SHARED_SECRET_INFO_S;
//============================================================================================
// opcodes uses the struct:   
// direction: mcms ==> mux    
// description: shared secret 
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  APIU8  xmtKey[16];
  APIU8  rcvKey[16];
  
} ENC_KEYS_INFO_S;


//============================================================================================
//==== error handling ====
//============================================================================================
// opcodes uses the struct:   
// direction: mcms ==> mux   
// description:kill mux connection
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  APIU32  cause; // may not be used at start 
} H221_KILL_CONNECTION_S;
//============================================================================================
// opcodes uses the struct:   
// direction: mcms ==> mux    
// description: sending comfort noise 
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  APIU32  audio_stream_direction;  // 1 - from TDM (send to remote), 2 - to TDM (send to AUDIO card), 0 - both from & to TDM *
  APIU32  status;  // 1 - start sending comfort noise, 0 - stop sending comfort noise 
} AUDIO_COMFORT_NOISE_REQ_S;
//============================================================================================
// opcodes uses the struct:   
// direction: mux ==> mcms    
// description: mux received wrong request from mcms
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  APIU32             msg_opcode;
  APIU32             cause;
  BAD_REQUEST_INFO_DESC bad_reques_data; //for example if illegal comm mode - will return the comm mode
} BAD_REQUEST_S;
//============================================================================================
// opcodes uses the struct:   
// direction: mcms ==> mux   
// description:Used set mux loop,The MUX xmits back all frames received. It does not synch..
//
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
typedef struct
{
  APIU32 cause;
} LOOP_BACK_S;
//-------------------------------------------------------------------------
typedef struct
{
  APIU32 cause;
} STOP_LOOP_BACK_S;
//============================================================================================

//============================================================================================
//==== SNMP ====
//============================================================================================
// opcodes uses the struct:   
// direction: mcms ==> mux    
// change from mgc: no major change 
//--------------------------------------------------------------------------------------------
// //   Description SHOULD DEFINE A STRUCT !! :
// When the mux processor gets SNMP_STATS_REQ it start to send per participant, the connection 
// state information of its active channels.
// The connecion state information of one channel contains the following fields :
//   ·	h221StatsInFrames	             // The number of H.221 frames received.
//   ·	h221StatsOutFrames	             // The number of H.221 frames sent.
//   ·	h221StatsInFrameErrs	         // The number of error frames received.
//   ·	h221StatsFrameAlignmentErrs	     // The number of times frame alignment is lost. I.e.,  three consecutive frame alignment words were  received with an error after the establishment  of frame alignment.
//   ·	h221StatsMultiFrameAlignmentErrs //	The number of times multi-frame alignment is lost. I.e.,  three consecutive multi-frame alignment words were  received with an error after the establishment  of frame alignment.
//   ·	h221StatsErrorPerformance        // The quality of the 64 Kbps connection can be monitored  by counting the number of  CRC blocks in error (E-bit = 1)  within a period of one second (50 blocks). The following indicates the mapping between the number of CRC blocks  in error and the equivalent line error rate.  
//   ·	h221StatsBASErrs	             // The number of BAS codes received with ECC errors
//   ·	h221StatsCRC4Err	             // The number of frames received with CRC4 errors
//   ·	h221StatsInEBit	                 // The number of frames received with the E bit set
//   ·	h221StatsInopportuneBAS	         // The number of BAS codes received at inappropriate times (e.g. commands received in the middle of capabilities exchange.
//
//  The longest SNMP_CONNECTION_STATS_S message is of a participant with line rate of 6B.  
//
typedef struct  
{
     SNMP_INFO_DESC  connection_state_information;   // The segment descriptor which contains the information.
                                                  
} SNMP_CONNECTION_STATS_S;


//============================================================================================
//==== probably not in use - will be removed after checking with mux ====
//============================================================================================


//============================================================================================
//==== default capabilities ====*/
//============================================================================================
// opcodes uses the struct:   
// direction: mcms ==> mux    
// description:  Local caps are the capabilities the MUX sends over to the remote in 
// the  initialization procedure.  When no caps are given in INIT_COMM msg,
// the MUX uses the default local caps.
//
// change from mgc: no major change 
//-------------------------------------------------------------------------------------------
typedef struct
{
  CAPABILITIES_S local_caps;
} SET_DEFAULT_LOCAL_CAPS_S;
//===========================================================================================
// opcodes uses the struct:   
// direction: mcms ==> mux   
// description:Local caps are the capabilities the MUX sends over to the remote in 
// the  initialization procedure.
//
// change from mgc: no major change 
//-------------------------------------------------------------------------------------------
typedef struct
{
  CAPABILITIES_S local_caps;
} SET_CONN_LOCAL_CAPS_S;
//===========================================================================================

//===========================================================================================
//==== H221 aggregation / restrict / unframed related commands ====
//===========================================================================================
// opcodes uses the struct:   
// direction: mux ==> mcms   
// description:Local caps are the capabilities the MUX sends over to the remote in 
//the  initialization procedurealignment of additional channel.
//
//change from mgc: no major change 
//------------------------------------------------------------------------------------------
typedef struct
{
    APIS32  channel_number;
}   CHANNEL_ALIGNED_S;
//===========================================================================================
// opcodes uses the struct:   
// direction: mcms ==> mux    
// description:Used upon incomming ring, in order to identify the channel.
//
// change from mgc: no major change 
//-------------------------------------------------------------------------------------------
typedef struct
{
    APIS32  channel_number;
}   H221_IDENTIFY_CHANNEL_S;
//==========================MUX -> MCMS MSGS================================
// END_IDENTIFY_CHANNEL:
// ---------------------
// Description: look h221_IDENTIFY_CHANNEL.
///

typedef struct
{
    APIS32                  framed;            // 1 - framed, 0 - not framed.
    APIS32                  initial_channel;   // 1 - initial channel,0 - additional channel.
    APIS32                  restricted;        // 1 - restricted,0 - not restricted.    
    APIS32                  channel_number;    // For additional channels. 
    APIS32                  Tix;               // TIX, if given, for additional channel.
    APIS32                  padd;
} END_IDENTIFY_CHANNEL_S;
//------------------------------------------------------------------------
// RESTRICT_MODE_S
// -----------------
// Description:
// Used when a restricted call is connected.
// Action:
// If connection do not exist - an error.
// If connection exist, the mux will start to communicate with the remote
// using the requested restrict mode.
// Response:
// none.
//

typedef struct
{
   APIS32 restrict_type;
} RESTRICT_MODE_S;

//-------------------------------------------------------------------------
// H221_INIT_COMM_UNFRAMED_S
// -----------------
// Description:
// Used when an unframed channel is connected.
// Action:
// If connection exist - an error.
// If connection does no exist, a new connection is created.
// The MUX connects the channel and wait for the audio TS.
// response:
// there is NO response back to the MCU for this command.
//
typedef struct
{
  APIU32                  channel_width;    // = 1
  APIU32                 restrict_type;    // = 0      
  //STREAM_DESC           net_stream; 
} H221_INIT_COMM_UNFRAMED_S;
//-------------------------------------------------------------------------
//
// KILL_UNFRAMED_CONNECTION_REQ_S
// -----------------
// Description:
// Used when the MCMS asks the mux to terminate an unframed connection.
// Action:
// The MUX kills the connection.
// response:
// there is no response back to the MCU for this command.
//
//#define  KILL_UNFRAMED_CONNECTION_REQ_S  INT_COMMON_HEADER_S - opcode only at RMX 
//-------------------------------------------------------------------------

//============================================================================================
//==== SMART RECOVERY ====
//============================================================================================
// opcodes uses the struct:  SMART_RECOVERY_UPDATE 
// direction: mux ==> mcms    
//description: this opcode is being sent when a dsp recovery is performed on 
// the dsp which the mux unit is located on.

//change from mgc: new opcode 
//--------------------------------------------------------------------------------------------
typedef struct
{
  APIU32     unDspNum;
  APIU32     unPortNum;     
} SMART_RECOVERY_UPDATE_S;

#endif
