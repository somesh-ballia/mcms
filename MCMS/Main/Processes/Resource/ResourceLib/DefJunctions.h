#ifndef  __DSP_CM_JUNCTIONS_H_
#define  __DSP_CM_JUNCTIONS_H_


/***************************************************************************************/
/* Junctions                                                                           */
/***************************************************************************************/

/*-------------------------------------------------------------------------------------*/
/*                                                                                     */
/*  Each type of port / entity has a certain amount of incoming and outgoing streams.  */
/*  Each Stream carries a differnt type of data, the type of date is refered to as     */
/*  junctions.																		   */
/*                                                                                     */
/*-------------------------------------------------------------------------------------*/

typedef enum {

/***************************************************************************************/
/* Ports Junctions                                                                     */
/***************************************************************************************/
	CM_JUNCTION_DUMMY								= 0,			//size=whocares
/*-------------------------------------------------------------------------------------*/
/* ART Light Port Junctions                                                            */
/*-------------------------------------------------------------------------------------*/


	E_CM_JUNCTION_LAN_INCOMING_AUDIO					= 51,     	//size = 8K, Lobby
	E_CM_JUNCTION_INCOMING_AC_COMMAND,								//size = 4K, Audio Rx
	E_CM_JUNCTION_INCOMING_COMPRESSED_AUDIO,						//size = 4K, ISDN interface
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO1,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO2,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO3,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO4,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO5,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO6,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO7,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO8,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO9,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO10,					//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_PUBLIC_AUDIO_IVR,						//size = 4K, Audio Tx
	E_CM_JUNCTION_INCOMING_PRIVATE_AUDIO_IVR,						//size = 4K, Audio Tx
	E_CM_JUNCTION_INCOMING_PUBLIC_MUSIC,							//size = 4K, Audio Tx
	E_CM_JUNCTION_INCOMING_PRIVATE_MUSIC,							//size = 4K, Audio Tx
	E_CM_JUNCTION_INCOMING_MRMP_AUDIO,								//size = 8K, Audio Tx

	E_CM_JUNCTION_LAN_OUTGOING_AUDIO					= 71,		//size = 4K, Audio Tx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO1,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO2,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO3,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO4,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO5,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO6,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO7,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO8,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO9,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO10,					//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_ROLL_CALL,								//size = 4K, Audio Rx
	E_CM_JUNCTION_OUTGOING_COMPRESSED_AUDIO,						//size = 4K, Audio Tx
	E_CM_JUNCTION_ART_OUTGOING_INTERNAL_RECORDING1,					//size = 4K, Audio Rx
	E_CM_JUNCTION_ART_OUTGOING_INTERNAL_RECORDING2,					//size = 4K, Audio Tx
	E_CM_JUNCTION_OUTGOING_MRMP_AUDIO,								//size = 8K, Audio Rx


/*-------------------------------------------------------------------------------------*/
/* ART Port Junctions                                                                  */
/*-------------------------------------------------------------------------------------*/


	E_CM_JUNCTION_LAN_INCOMING_VIDEO					= 91,		//size = 20K, Lobby
	E_CM_JUNCTION_LAN_INCOMING_CONTENT,								//size = 20K, Lobby
	E_CM_JUNCTION_LAN_INCOMING_FECC,								//size = 8K,  Lobby
	E_CM_JUNCTION_FABRIC_INCOMING_VIDEO1,							//size = 16K, Video Tx
	E_CM_JUNCTION_FABRIC_INCOMING_VIDEO2,							//size = 16K, Video Tx
	E_CM_JUNCTION_FABRIC_INCOMING_VIDEO3,							//size = 16K, Video Tx
	E_CM_JUNCTION_FABRIC_INCOMING_CONTENT,							//size = 20K, Lobby
	E_CM_JUNCTION_FABRIC_INCOMING_FECC,								//size = 8K , Lobby
	E_CM_JUNCTION_INCOMING_VC_VSW_COMMAND,							//size = 4K , Video Rx
	E_CM_JUNCTION_INCOMING_PRIVATE_VIDEO_IVR,						//size = 16K, Video Tx
	E_CM_JUNCTION_MRMP_INCOMING_FECC,								//size = 4K,  FECC Tx
	E_CM_JUNCTION_MRMP_INCOMING_VIDEO,								//size = 64K, Video Tx

	E_CM_JUNCTION_LAN_OUTGOING_VIDEO					= 111,		//size = 16K, Video Tx
	E_CM_JUNCTION_LAN_OUTGOING_CONTENT,								//size = 16K, Content Tx
	E_CM_JUNCTION_LAN_OUTGOING_FECC,								//size = 4K , FECC Tx
	E_CM_JUNCTION_FABRIC_OUTGOING_VIDEO1,							//size = 16K, Video Rx
	E_CM_JUNCTION_FABRIC_OUTGOING_VIDEO2,							//size = 16K, Video Rx
	E_CM_JUNCTION_FABRIC_OUTGOING_VIDEO3,							//size = 16K, Video Rx
	E_CM_JUNCTION_FABRIC_OUTGOING_CONTENT,							//size = 20K, Content Rx
	E_CM_JUNCTION_FABRIC_OUTGOING_FECC,								//size = 8K , FECC RX
	E_CM_JUNCTION_MRMP_OUTGOING_FECC,								//size = 4K,  FECC Rx
	E_CM_JUNCTION_MRMP_OUTGOING_VIDEO,								//size = 64K, Video Rx


/*-------------------------------------------------------------------------------------*/
/* Video Decoder Port Junctions                                                        */
/*-------------------------------------------------------------------------------------*/

	E_CM_JUNCTION_INCOMING_COMPRESSED_VIDEO				= 131,		//size = 32K, Video Decoder

	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_4CIF		= 151,		//size = 1280K, Video Decoder
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_CIF,					//size = 320K, Video Decoder
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_QCIF,					//size = 80K, Video Decoder
	E_CM_JUNCTION_VIDEO_DECODER_OUTGOING_INTERNAL_RECORDING1,		//size = 4K, Video Decoder
	E_CM_JUNCTION_VIDEO_DECODER_OUTGOING_INTERNAL_RECORDING2,		//size = 4K, Video Decoder



/*-------------------------------------------------------------------------------------*/
/* Video Encoder Port Junctions                                                        */
/*-------------------------------------------------------------------------------------*/
/* Incoming Uncompressed Video streams are depended on the layout used. According to   */
/* the current design of the Video Encoder port, there might be up to 16 differnt      */
/* Uncompressed Video streams. Each Uncompressed Video stream will have the same       */
/* junction type                                                                       */
/*-------------------------------------------------------------------------------------*/


	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_1			= 171,		//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_2,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_3,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_4,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_5,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_6,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_7,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_8,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_9,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_11,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_12,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_13,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_14,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_15,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_16,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_PUBLIC_GRAPHIC_OBJECTS,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_PRIVATE_GRAPHIC_OBJECTS,					//size = 80K, Video Encoder

	E_CM_JUNCTION_OUTGOING_COMPRESSED_VIDEO				= 191,		//size = 16K, Video Encoder
	E_CM_JUNCTION_VIDEO_ENCODER_OUTGOING_INTERNAL_RECORDING1,		//size = 4K, Video Encoder
	E_CM_JUNCTION_VIDEO_ENCODER_OUTGOING_INTERNAL_RECORDING2,		//size = 4K, Video Encoder



/*-------------------------------------------------------------------------------------*/
/* Audio Controller Junctions  (Currently running in DSP)                              */
/*-------------------------------------------------------------------------------------*/
/* Audio controller streams are divided to startup streams (Audio Info streams, and    */
/* competition result streams) and run-time Multicast AC command streams.              */
/* These Multicast Command streams, are created for each conference conducted on that  */
/* MFA. Each such stream will have the same junction type.                             */
/*-------------------------------------------------------------------------------------*/


//	E_CM_JUNCTION_INCOMING_AUDIO_INFO_FROM_DSP			= 211,		//moved to startup
//	E_CM_JUNCTION_INCOMING_COMPETITION_RESULTS,						//moved to startup

	E_CM_JUNCTION_OUTGOING_AC_COMMAND					= 231,		//size = 4K, ART DSP chip
//	E_CM_JUNCTION_OUTGOING_COMPETITION_RESULTS,						//moved to startup


/*-------------------------------------------------------------------------------------*/
/* DSP Startup Junctions                                                               */
/*-------------------------------------------------------------------------------------*/


	E_CM_JUNCTION_INCOMING_CONTROL_FROM_CM				= 11,		//size = 32K
	E_CM_JUNCTION_INCOMING_SPECIAL_PARAMS_FROM_CM,					//size = 4K
	E_CM_JUNCTION_INCOMING_CLOCK_SYNC,								//size = 4K
	E_CM_JUNCTION_INCOMING_RTCP_FROM_IP_MEDIA,						//size = 32K		//relevant only to ART / ART Light DSPs
	E_CM_JUNCTION_INCOMING_AUDIO_INFO_FROM_DSP,						//size = 4K			//relevant only to AC DSPs
	E_CM_JUNCTION_INCOMING_COMPETITION_RESULTS,						//size = 32K		//relevant only to AC DSPs

	E_CM_JUNCTION_OUTGOING_CONTROL_TO_CM				= 31,		//size = 32K
	E_CM_JUNCTION_OUTGOING_BACKUP_TO_CM,							//size = 32K
	E_CM_JUNCTION_OUTGOING_TO_CM_SPECIAL_PARAMS,					//size = 4K
	E_CM_JUNCTION_OUTGOING_LOGGER_TO_CM,							//size = 32K
	E_CM_JUNCTION_OUTGOING_DEBUG_INTERNAL_RECORDING,				//size = 16K
	E_CM_JUNCTION_OUTGOING_AUDIO_INFO_TO_AC,						//size = 4K			//relevant only to ART / ART Light DSPs
	E_CM_JUNCTION_OUTGOING_RTCP_TO_IP_MEDIA,						//size = 32K		//relevant only to ART / ART Light DSPs
	E_CM_JUNCTION_OUTGOING_MUSIC,									//size = 4K			//relevant only to RTM DSPs
	E_CM_JUNCTION_OUTGOING_COMPETITION_RESULTS,						//size = 32K		//relevant only to AC DSPs

/***************************************************************************************/
/* PQ3 Junctions.                                                                      */
/***************************************************************************************/

/*-------------------------------------------------------------------------------------*/
/* CM Startup Junctions                                                                */
/*-------------------------------------------------------------------------------------*/


	E_CM_JUNCTION_INCOMING_CONTROL_FROM_PROC			= 10101,	//size = 32K
	E_CM_JUNCTION_INCOMING_BACKUP_FROM_PROC,						//size = 32K
	E_CM_JUNCTION_INCOMING_CM_SPECIAL_PARAMS_FROM_PROC,				//size = 4K
	E_CM_JUNCTION_INCOMING_LOGGER_FROM_PROC,						//size = 32K
	E_CM_JUNCTION_INCOMING_CONTROL_FROM_OTHER_CM,					//size = 32K

	E_CM_JUNCTION_OUTGOING_CONTROL_TO_PROC				= 10201,	//size = 32K
	E_CM_JUNCTION_OUTGOING_SPECIAL_PARAMS_TO_PROC,					//size = 4K
	E_CM_JUNCTION_OUTGOING_CLOCK_SYNC,								//size = 4K
	E_CM_JUNCTION_OUTGOING_CONTROL_TO_OTHER_CM,						//size = 32K
	E_CM_JUNCTION_OUTGOING_DSP_DOWNLOAD,							//size = 32K
	E_CM_JUNCTION_OUTGOING_DATABASE_UPDATE,							//size = ?K



/*-------------------------------------------------------------------------------------*/
/* PQ3 Startup Junctions                                                               */
/*-------------------------------------------------------------------------------------*/
/* Relevant to all PQ3, usually for IP / Media and Local IVR    					   */
/*-------------------------------------------------------------------------------------*/


	E_CM_JUNCTION_RTCP_FROM_FABRIC						= 10301,	//size = 32K
	E_CM_JUNCTION_PQ3_INCOMING_CONTROL_FROM_CM,						//size = 32K
	E_CM_JUNCTION_PQ3_INCOMING_SPECIAL_PARAMS_FROM_CM,				//size = 4K
	E_CM_JUNCTION_PQ3_INCOMING_CLOCK_SYNC,							//size = 4K
	E_CM_JUNCTION_INCOMING_DATABASE_UPDATE,							//size = ?K
	E_CM_JUNCTION_INCOMING_MUSIC,									//size = 4K
	E_CM_JUNCTION_INCOMING_DEBUG_INTERNAL_RECORDING,				//size = 16K

	E_CM_JUNCTION_RTCP_TO_FABRIC						= 10401,	//size = 32K
	E_CM_JUNCTION_PQ3_OUTGOING_CONTROL_TO_CM,						//size = 32K
	E_CM_JUNCTION_PQ3_OUTGOING_BACKUP_TO_CM,						//size = 32K
	E_CM_JUNCTION_PQ3_OUTGOING_TO_CM_SPECIAL_PARAMS,				//size = 4K
	E_CM_JUNCTION_PQ3_OUTGOING_LOGGER_TO_CM,						//size = 32K



/*-------------------------------------------------------------------------------------*/
/* IP / MEDIA Junctions per port                                                       */
/*-------------------------------------------------------------------------------------*/
/* IP / Media is connected to each ART / ART Light port in its MFA through unicast     */
/* streams. These unicast streams are created for each PORT at startup, once it is     */
/* known which DSPs are dedicated for ART ports. Each set of streams per port will     */
/* have the same set of junction types listed.                                         */
/*-------------------------------------------------------------------------------------*/
	E_CM_JUNCTION_TO_IP_MEDIA							= 10500,


	E_CM_JUNCTION_FROM_FABRIC_AUDIO						= 10501,	//size = 4K
	E_CM_JUNCTION_FROM_MRMP_AUDIO,
	E_CM_JUNCTION_FROM_FABRIC_VIDEO,								//size = 16K
	E_CM_JUNCTION_FROM_MRMP_VIDEO,
	E_CM_JUNCTION_FROM_FABRIC_CONTENT,								//size = 16K
	E_CM_JUNCTION_FROM_FABRIC_FECC,									//size = 4K
	E_CM_JUNCTION_FROM_MRMP_FECC,


	E_CM_JUNCTION_TO_FABRIC_AUDIO						= 10601,	//size = 8K
	E_CM_JUNCTION_TO_MRMP_AUDIO,
	E_CM_JUNCTION_TO_FABRIC_VIDEO,									//size = 20K
	E_CM_JUNCTION_TO_MRMP_VIDEO,
	E_CM_JUNCTION_TO_FABRIC_CONTENT,								//size = 20K
	E_CM_JUNCTION_TO_FABRIC_FECC,									//size = 8K
	E_CM_JUNCTION_TO_MRMP_FECC,


/*-------------------------------------------------------------------------------------*/
/* Video Controller Junctions per conference                                           */
/*-------------------------------------------------------------------------------------*/
/* Video controller streams are all run-time Multicast VC VSW command streams.         */
/* These Multicast Command streams, are created for each conference conducted on that  */
/* MFA. Each such stream will have the same junction type.                             */
/*                                                                                     */
/*-------------------------------------------------------------------------------------*/

	E_CM_JUNCTION_INCOMING_REMOTE_VC_VSW_COMMAND		= 10701,	//size = 4K

	E_CM_JUNCTION_OUTGOING_VC_VSW_COMMAND				= 10801,	//size = 4K


/*-------------------------------------------------------------------------------------*/
/* IVR Controller Junctions per port                                                   */
/*-------------------------------------------------------------------------------------*/
/* IVR controller OUTGOIGN streams are all run-time IVR/MUSIC/SLIDES/Graphic_Objects   */
/* streams. These streams, are created for each conference / port conducted on that    */
/* MFA.                                                                                */
/*-------------------------------------------------------------------------------------*/
	E_CM_JUNCTION_TO_IVR								= 10900,

	E_CM_JUNCTION_RECORD_INCOMING_COMPRESSED_AUDIO		= 10901,	//size = 4K
	E_CM_JUNCTION_RECORD_INCOMING_COMPRESSED_VIDEO,					//size = 4K
	E_CM_JUNCTION_INCOMING_ROLL_CALL,								//size = 4K
	E_CM_JUNCTION_ART_INCOMING_INTERNAL_RECORDING1,					//size = 4K
	E_CM_JUNCTION_ART_INCOMING_INTERNAL_RECORDING2,					//size = 4K
	E_CM_JUNCTION_VIDEO_DECODER_INCOMING_INTERNAL_RECORDING1,		//size = 4K
	E_CM_JUNCTION_VIDEO_DECODER_INCOMING_INTERNAL_RECORDING2,		//size = 4K
	E_CM_JUNCTION_VIDEO_ENCODER_INCOMING_INTERNAL_RECORDING1,		//size = 4K
	E_CM_JUNCTION_VIDEO_ENCODER_INCOMING_INTERNAL_RECORDING2,		//size = 4K

	E_CM_JUNCTION_OUTGOING_PUBLIC_MUSIC					= 11001,	//size = 4K
	E_CM_JUNCTION_OUTGOING_PRIVATE_MUSIC,							//size = 4K
	E_CM_JUNCTION_OUTGOING_PUBLIC_AUDIO_IVR,						//size = 4K
	E_CM_JUNCTION_OUTGOING_PRIVATE_AUDIO_IVR,						//size = 4K
	E_CM_JUNCTION_OUTGOING_PRIVATE_VIDEO_IVR,						//size = 16K
	E_CM_JUNCTION_OUTGOING_PUBLIC_GRAPHIC_OBJECTS,					//size = 80K
	E_CM_JUNCTION_OUTGOING_PRIVATE_GRAPHIC_OBJECTS,					//size = 80K

	//--------------------------------------------------------------------
	// RTM
	//--------------------------------------------------------------------

	E_CM_JUNCTION_NETWORK_RTM_OUTGOING					= 15001,	//size = 4K
	E_CM_JUNCTION_NETWORK_IPMEDIA_TO_RTM					= 15002,

	//---------------------------------
	// MUX
	//--------------------------------------------------------------------

	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL01					= 15101,	//size = 4k
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL02,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL03,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL04,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL05,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL06,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL07,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL08,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL09,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL10,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL11,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL12,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL13,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL14,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL15,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL16,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL17,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL18,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL19,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL20,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL21,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL22,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL23,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL24,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL25,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL26,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL27,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL28,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL29,
	E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL30,

	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL01					= 15201,	//size = 4k
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL02,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL03,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL04,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL05,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL06,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL07,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL08,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL09,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL10,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL11,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL12,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL13,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL14,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL15,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL16,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL17,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL18,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL19,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL20,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL21,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL22,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL23,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL24,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL25,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL26,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL27,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL28,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL29,
	E_CM_JUNCTION_INCOMING_ISDN_CHANNEL30,

	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL01				= 15301,	//size = 4k
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL02,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL03,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL04,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL05,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL06,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL07,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL08,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL09,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL10,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL11,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL12,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL13,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL14,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL15,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL16,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL17,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL18,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL19,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL20,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL21,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL22,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL23,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL24,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL25,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL26,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL27,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL28,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL29,
	E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL30,

	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL01					= 15401,	//size = 4k
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL02,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL03,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL04,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL05,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL06,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL07,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL08,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL09,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL10,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL11,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL12,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL13,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL14,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL15,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL16,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL17,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL18,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL19,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL20,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL21,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL22,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL23,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL24,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL25,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL26,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL27,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL28,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL29,
	E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL30,

/*-------------------------------------------------------------------------------------*/
	E_CM_JUNCTION_UNKNOWN								= 0xFFFE,
	E_CM_JUNCTION_LAST									= 0xFFFF

} ECmJunctions;

#endif	// ___DSP_CM_JUNCTIONS_H_











