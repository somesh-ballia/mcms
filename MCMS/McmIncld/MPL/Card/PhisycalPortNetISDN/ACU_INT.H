
/*--------------------------------------------------------------------------*
 *                       M O D U L E   H E A D E R
 *
 * filename - ACU_INT.H
 *
 * $Revision:   6.88  $
 *
 * $Date:   Jun 29 2004 09:56:06  $
 *
 *		- Call Control Automatic Calling Unit Interface header compatible
 *        with the TR29 ACU_INT. For Analog and ISDN Call Control.
 *
 *
 * !!!!!!!!!!!!!!!!!!!!!!  IMPORTANT  NOTE   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !
 * ! This file can be used with the ISDN-BRICKS "Call-Control" entity (CC),
 * ! and with the analog "ACU" entity (Automatic Call Unit).
 * !
 * ! When used with the ISDN_BRICKS CC entity, the cc_int.h file should
 * ! be included prior to including acu_int.h, so that the ISDN-BRICKS CC
 * ! specific parameters are properly defined.
 * ! Not including cc_int.h prior to acu_int.h could lead to different
 * ! structures alignments between CC and the application.
 * !
 * ! The inclusion of cc_int.h is not required when only the analog "ACU"
 * ! entity is being used.
 * !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *-------------------------------------------------------------------------*/

/*[]---------------------------------------------------[]*/
/*|                                                     |*/
/*|    ISDN-BRICKS                                      |*/
/*|    Call Control (CC) Entity                         |*/
/*|                                                     |*/
/*|    Copyright (c) 1989-2004 by NETBRICKS SA          |*/
/*|    All Rights Reserved.                             |*/
/*|                                                     |*/
/*[]---------------------------------------------------[]*/

#ifndef ACU_INT
#	define ACU_INT

/*----------------------------------------------------------------------------
 *				Adressing constants
 *--------------------------------------------------------------------------*/

#define	ACU_SAPI					'a'			/* SAP for ACU interface primitives */
#define	ACU_SAPI_CED_CNG			'b'			/* SAP for CED/CNG signalling */
#define	ACU_SAPI_MGT				'A'			/* SAP for Management primitives */

#define ACU_SAPI_MULTI_CHAN			'X'			/* Multi-channel SAPI */

/*--------------------------------------------------------------------------*/

			/* Network-specific compilation parameters */

#define	ACU_MX_SZ_CALLING_NB		32			/* Maximum size for calling nb */
#define	ACU_MX_SZ_CALLING_SUB_ADD	20			/* Maximum size for calling sub address */
#define	ACU_MX_SZ_CALLED_NB      	35          /* Maximum size for called nb */
#define	ACU_MX_SZ_CALLED_SUB_ADD	20          /* Maximum size for called sub address */
#define ACU_MX_SZ_UUI				133			/* Maximum size for UUI */
#define ACU_MX_SZ_CALL_ID			4			/* Maximum size for called ID */
#define ACU_MX_SZ_USER_FAC			7			/* Maximum size for user facility information */
#define ACU_MX_SZ_FACILITY			250			/* Maximum size for facility information */
#define ACU_MX_SZ_DISPLAY			128			/* Maximum size for display information */
#define ACU_MX_SZ_PCS_USER			129			/* Maximum size for display information (128 + 1 for the leading 'protocol-discriminator' field */
#define	ACU_PREFIX_SIZE				5			/* Maximum length of a phone number prefix */
												/* (for accessing public or private telephone network, */
												/* international network, etc.) */

/*----------------------------------------------------------------------------
 * Default macro-definitions for compatibility purpose
 *--------------------------------------------------------------------------*/

#ifndef CC_F_DMS
#	define CC_F_DMS					OFF
#endif
#ifndef CC_F_NI2
#	define CC_F_NI2					OFF
#endif
#ifndef CC_F_E10
#	define CC_F_E10					OFF
#endif
#ifndef CC_F_A11
#	define	CC_F_A11				OFF
#endif
#ifndef CC_F_A17
#	define	CC_F_A17				OFF
#endif

#ifndef CC_TYPE_BRI
#	define CC_TYPE_BRI				OFF
#endif

#if (CC_F_A11 == ON) || (CC_F_A17 == ON)
#	define ARINC_NETWORKS 			ON
#else
#	define ARINC_NETWORKS 			OFF
#endif

#ifndef	ACU_PH_ENTITY
#	define	ACU_PH_ENTITY			ENT_PH_GSTN	/* For Analog ACU only */
#endif

#ifndef ACU_PAR_EXTENDED_PRIM
#	define ACU_PAR_EXTENDED_PRIM	OFF
#endif

#ifndef ACU_PAR_EXT_LLC
#	define ACU_PAR_EXT_LLC			OFF
#endif

#ifndef ACU_PAR_EXT_CUG
#	define ACU_PAR_EXT_CUG			OFF
#endif

#ifndef ACU_PAR_EXT_FAC_ASN1
#	define ACU_PAR_EXT_FAC_ASN1		OFF
#endif

#ifndef ACU_PAR_EXT_SPF_FAC_IE
#	define ACU_PAR_EXT_SPF_FAC_IE	OFF
#endif

#ifndef _CC_SPID_INFO
#	if (EQUIPMENT != TE) && (CC_TYPE_BRI == ON) && ((CC_F_DMS == ON) || (CC_F_NI2 == ON) || (CC_F_E10 == ON))
#		define 	_CC_SPID_INFO 		ON
#	else
#		define 	_CC_SPID_INFO 		OFF
#	endif
#endif

#ifndef ACU_PSTN_CLIP
#   define ACU_PSTN_CLIP            OFF
#endif


/*----------------------------------------------------------------------------
 *				Primitives sent and received to/from ACU by a user entity
 *--------------------------------------------------------------------------*/

#define	ACU_CONN_RQ			'A'		/* request establishment of an outgoing call */
#define	ACU_CONN_IN			'a'		/* indiacte an incoming call */
#define	ACU_CONN_RS			'B'		/* request answering to an incoming call */
#define	ACU_CONN_CO			'b'		/* indicate termination of an incoming or outgoing call */

#define	ACU_CLEAR_RQ		'C'		/* request release of the connection */
#define	ACU_CLEAR_IN		'c'		/* connection released indication */
#define	ACU_CLEAR_RS		'D'		/* response to the release of the connection */
#define	ACU_CLEAR_CO		'd'		/* connection released confirmation */

#define	ACU_ALERT_RQ		'E'		/* alert request */
#define	ACU_ALERT_IN		'e'		/* alert indication (ex: incoming ring) */

#define	ACU_PROGRESS_RQ		'F'		/* events for the progression of the call */
#define	ACU_PROGRESS_IN		'f'		/* events from the progressing outgoing call */

#define	ACU_INFO_RQ			'G'		/* information request */
#define	ACU_INFO_CO			'g'		/* information confirmation */

#define	ACU_INIT_RQ			'H'		/* initialization request */
#define	ACU_INIT_CO			'h'		/* initialization confirmation */

#define	ACU_SETPARM_RQ		'I'		/* set parameter request */
#define	ACU_SETPARM_CO		'i'		/* parameter confirmation */

#define	ACU_USER_INFO_RQ	'J'		/* request user to user information */
#define	ACU_USER_INFO_IN	'j'		/* indicate user to user information */

#define	ACU_SUSPEND_RQ		'K'		/* request suspend (TE-side) */
#define	ACU_SUSPEND_CO		'k'		/* confirm suspend (TE-side) */
#define	ACU_SUSPEND_IN		ACU_SUSPEND_RQ/* suspend indication (NT-side) */
#define	ACU_SUSPEND_RS		ACU_SUSPEND_CO/* suspend response (NT-side) */

#define	ACU_RESUME_RQ		'L'		/* request resume (TE-side) */
#define	ACU_RESUME_CO		'l'		/* confirm resume (TE-side) */
#define	ACU_RESUME_IN		ACU_RESUME_RQ/* resume indication (NT-side) */
#define	ACU_RESUME_RS		ACU_RESUME_CO/* resume response (NT-side) */

#define	ACU_TEST_RQ			'M'		/* request test */
#define	ACU_TEST_CO			'm'		/* confirm test */

#define	ACU_DIGIT_RQ		'N'		/* send several called nb digits in Overlap Sending mode */
#define	ACU_DIGIT_IN		'v'		/* receive several called nb digits in Overlap Receiving mode */
#define	ACU_DIGIT_CO		'n'		/* confirm end-of-called-nb-reached in Overlap Sending mode */

#define	ACU_FACILITY_RQ		'O'		/* request specific facility */
#define	ACU_FACILITY_IN		'o'		/* indicate specific facility */
#define	ACU_FACILITY_RS		'P'		/* response to specific facility */
#define	ACU_FACILITY_CO		'p'		/* confirm specific facility */

#define	ACU_SET_MODE_RQ		'Q'		/* request setting of a particular mode */
#define	ACU_SET_MODE_CO		'q'		/* confirm setting of a particular mode */

#define	ACU_RS_MODE_RQ		'R'		/* request response mode to incoming call */
#define	ACU_RS_MODE_CO		'r'		/* confirm response mode to incoming call */

#define	ACU_INFORMATION_RQ	'S'		/* request information */
#define	ACU_INFORMATION_IN	's'		/* indicate information */

#define ACU_SETUP_REPORT_IN	't'		/* indicate incompatible incoming call */

#define	ACU_CALL_PROC_RQ	'U'		/* events for the progression of the call */

#define	ACU_NOTIFY_RQ		'W'		/* NOTIFY message: events for the progression of the call */
#define	ACU_NOTIFY_IN		'w'		/* NOTIFY message: events for the progression of the call */

#define	ACU_ERR_IN			'?'		/* Error indication */

#define ACU_PDU_RQ			'Z'		/* request sending of a raw Q.931 PDU */

/*------------------------------------------------------
 * Type of the 'size' fields within the ACU primitives.
 *-----------------------------------------------------*/

typedef uchar acu_size_t;

/*----------------------------------------------------------------------------
 * Layer 1 information structure and fields values used in ACU_CONN_RQ/IN/RS
 * Used for V.110 and V.120 services only.
 *
 * Fields used by the physical drivers (available for all services):
 *   ph_rate:     physical rate
 *
 * Fields used for the ISDN coding (BC/LLC Information Elements):
 * (for ACU_V110_SERVICE and ACU_V120_SERVICE services only)
 *   ph_rate:     physical rate
 *   user_rate:   user rate
 *   syn_asyn:    synchronous/asynchronous
 *   stop_bits:   nb of stop bits
 *   data_bits:   nb of data bits
 *   parity:      parity
 *   l1_protocol: user information layer 1 protocol
 *
 *   ! When used in ACU_CONN_RQ or ACU_CONN_RS, the 'l1_protocol' field
 *   ! may only indicate the voice law (A or mu) associated to a
 *   ! voice-class service (ACU_VOICE_SERVICE or ACU_MODEM_SERVICE or
 *   ! ACU_FAX_SERVICE). A value of 0 or ACU_BYTE_VAL_NIL indicates that
 *   ! CC should use the value stored in the 'out_calls_behaviour'
 *   ! configuration field.
 *
 *--------------------------------------------------------------------------*/

struct layer_1_info {
	uchar	ph_rate;		/* physical rate */
	uchar	user_rate;		/* user rate */
	uchar	syn_asyn;		/* synchronous/asynchronous */
	uchar	stop_bits;		/* nb of stop bits */
	uchar	data_bits;		/* nb of data bits */
	uchar	parity;			/* parity */
	uchar	l1_protocol;	/* user information layer 1 protocol */
};

	/* Same values as CCITT */

#define ACU_BYTE_VAL_NIL		(uchar)(~0)				/* NIL byte value */

	/* User rates */

#define ACU_RA_E_BITS			G_5 (0,0,0,0,0)			/* Rate is indicated by E-bits specified in Recommendation I.460 (V.110) */
#define ACU_RA_50				G_5 (1,1,0,0,1)			/* 0.050 kbits/s CCITT V.6 and X.1 */
#define ACU_RA_75				G_5 (1,1,0,1,0) 		/* 0.075 kbits/s CCITT V.6 and X.1 */
#define ACU_RA_75_1200 			G_5 (1,0,1,1,1)			/* 0.075/1.2 kbits/s CCITT V.6 and x.1 */
#define ACU_RA_100				G_5 (1,0,1,1,0)			/* 0.100 kbits/s CCITT V.6 and X.1 */
#define ACU_RA_110				G_5 (1,1,0,1,1)			/* 0.110 kbits/s CCITT V.6 and X.1 */
#define ACU_RA_134				G_5 (1,0,1,0,1)			/* 0.1345 kbits/s CCITT X.1 */
#define ACU_RA_150				G_5 (1,1,1,0,0)			/* 0.150 kbits/s CCITT V.6 and X.1 */
#define ACU_RA_200				G_5 (1,1,1,0,1)			/* 0.200 kbits/s CCITT V.6 and X.1 */
#define ACU_RA_300				G_5 (1,1,1,1,0)			/* 0.300 kbits/s CCITT V.6 and X.1 */
#define ACU_RA_600				G_5 (0,0,0,0,1)			/* 0.6 kbits/s CCITT V.6 and x.1 */
#define ACU_RA_1200				G_5 (0,0,0,1,0)			/* 1.2 kbits/s CCITT V.6 */
#define ACU_RA_1200_75 			G_5 (1,1,0,0,0)			/* 1.2/0.075 kbits/s CCITT V.6 and x.1 */
#define ACU_RA_2400				G_5 (0,0,0,1,1)			/* 2.4 kbits/s CCITT V.6 and X.1 */
#define ACU_RA_3600				G_5 (0,0,1,0,0)			/* 3.6 kbits/s CCITT V.6 */
#define ACU_RA_4800				G_5 (0,0,1,0,1)			/* 4.8 kbits/s CCITT V.6 and x.1 */
#define ACU_RA_7200				G_5 (0,0,1,1,0)			/* 7.2 kbits/s CCITT V.6 */
#define ACU_RA_8000				G_5 (0,0,1,1,1)			/* 8 kbits/s CCITT I.460 */
#define ACU_RA_9600				G_5 (0,1,0,0,0)			/* 9.6 kbits/s  CCITT V.6 and x.1 */
#define ACU_RA_12000			G_5 (1,1,1,1,1)			/* 12 kbits/s CCITT V.6 */
#define ACU_RA_14400			G_5 (0,1,0,0,1)			/* 14.4 kbits/s CCITT V.6 */
#define ACU_RA_16000			G_5 (0,1,0,1,0)			/* 16 kbits/s CCITT I.460 */
#define ACU_RA_19200			G_5 (0,1,0,1,1)			/* 19.2 kbits/s CCITT V.6 */
#define ACU_RA_32000			G_5 (0,1,1,0,0)			/* 32 kbits/s CCITT I.460 */
#define ACU_RA_38400			G_5 (1,0,0,0,1)			/* 38.4 kbits/s extended V.14 */
#define ACU_RA_38400_NO_ETSI	G_5 (0,1,1,0,1)			/* 38.4 kbits/s extended V.14 (VN3) NON ETSI */
#define ACU_RA_48000			G_5 (0,1,1,1,0)			/* 48 kbits/s CCITT V.6 and X.1 */
#define ACU_RA_56000			G_5 (0,1,1,1,1)			/* 56 kbits/s CCITT V.6 */
#define ACU_RA_57600			G_5 (1,0,0,1,0)			/* 57.6 kbits/s extended V.14 (VN3) NON ETSI NON CCITT */
#define ACU_RA_64000			G_5 (1,0,0,0,0)			/* 64 kbits/s CCITT I.460 */

	/* Sync/Async */

#define ACU_SYN					G_1 (0)					/* synchronous */
#define ACU_ASYN				G_1 (1)					/* asynchronous */

	/* Stop bits */

#define ACU_STOP_BIT_1			G_2 (0,1)				/* 1 bit */
#define ACU_STOP_BIT_1_5		G_2 (1,0)				/* 1.5 bit */
#define ACU_STOP_BIT_2			G_2 (1,1)				/* 2 bits */

	/* Data bits */

#define ACU_DATA_BIT_5			G_2 (0,1)				/* 5 bits */
#define ACU_DATA_BIT_7			G_2 (1,0)				/* 7 bits */
#define ACU_DATA_BIT_8			G_2 (1,1)				/* 8 bits */

	/* Parity */

#define ACU_ODD					G_3 (0,0,0)				/* odd */
#define ACU_EVEN				G_3 (0,1,0)				/* even */
#define ACU_NO_PARITY			G_3 (0,1,1)				/* none */
#define ACU_FORCED_TO_0			G_3 (1,0,0)				/* forced to 0 (space) */
#define ACU_FORCED_TO_1			G_3 (1,0,1)				/* forced to 1 (mark) */

	/* l1_protocol (from Q.931 "User information layer 1 protocol" field) */

#define ACU_L1_PROTOCOL_V110	G_5 (0,0,0,0,1)			/* V.110/X.30 CCITT rate adaptation */
#define ACU_L1_PROTOCOL_MU_LAW	G_5 (0,0,0,1,0)			/* G.711 mu-law */
#define ACU_L1_PROTOCOL_A_LAW	G_5 (0,0,0,1,1)			/* G.711 A-law */
#define ACU_L1_PROTOCOL_I460	G_5 (0,0,1,0,0)			/* I.460 and G.721 */
#define ACU_L1_PROTOCOL_7KHZ	G_5 (0,0,1,0,1)			/* G.722 and G.725 7kHz audio */
#define ACU_L1_PROTOCOL_VIDEO	G_5 (0,0,1,1,0)			/* G.7xx 384 kbit/s video */
#define ACU_L1_PROTOCOL_NON_CCITT G_5 (0,0,1,1,1)		/* G.7xx 384 kbit/s video */
#define ACU_L1_PROTOCOL_V120	G_5 (0,1,0,0,0)			/* V.120 CCITT rate adaptation */
#define ACU_L1_PROTOCOL_X31		G_5 (0,1,0,0,1)			/* X.31 CCITT rate adaptation */

/*----------------------------------------------------------------------------
 * B channels ids structure used to report B channels to the user
 *--------------------------------------------------------------------------*/

#ifndef ACU_MX_CHANI
#	define ACU_MX_CHANI			1	/* default maximum number of B-channel ids in the lists inside the primitives */
#endif

struct chani_tab {
	nai_t   nai;			/* nai value for non-NFAS mode or Multiplexed NFAS mode, Interface_id value for Transparent NFAS mode */
	uchar   chan_id;		/* channel-id */
};

struct chani_list {
	uchar   nb;				/* number of channel ids in the list */
	uchar	chan_id_excl;	/* chan_ids are preferred (OFF) or exclusive (ON) */
	struct chani_tab tab[ACU_MX_CHANI];
};

/*----------------------------------------------------------------------------
 * Called number related information sub-structure
 *----------------------------------------------------------------------------*/

struct acu_called_nb {
	uchar		type;		/* called nb type (ACU_NB_TYPE_*) */
	uchar		plan;		/* called nb plan (ACU_NB_PLAN_*) */
	uchar		sa_type;	/* called sub-add type (ACU_SA_TYPE_*) */
	uchar		sa_format;	/* called sub-add format (ACU_SA_FORMAT_*) -- meaningful only when sa_type==ACU_SA_TYPE_NSAP */
	uchar		sa_odd;		/* called sub-add odd/even indicator (ACU_SA_ODD/EVEN) */
	acu_size_t	size;		/* size of the called number in bytes */
	uchar		digits_ie;	/* which IE is used: Called-nb or Keypad (ACU_DIGITS_IN_*) */
};

	/* Values for 'digits_ie' field:
	 * In ACU_CONN_RQ and ACU_DIGIT_RQ primitives:
	 * this field allows to select which IE should be used;
	 * 0 makes CC use the default IE according to the variant and
	 * configuration settings (CC_DIAL_WITH_KEYPAD bit of
	 * the 'out_calls_behaviour' field).
	 * In ACU_CONN_IN and ACU_DIGIT_IN primitives:
	 * this field indicates which IE was used.
	 */

#define ACU_DIGITS_IN_CALLED_NB	1	/* digits are stored in a Called-nb IE */
#define ACU_DIGITS_IN_KEYPAD	2	/* digits are stored in a Keypad IE */

/*----------------------------------------------------------------------------
 * Calling number related information sub-structure
 *----------------------------------------------------------------------------*/

struct acu_calling_nb {
	uchar		type;		/* calling nb type (ACU_NB_TYPE_*) */
	uchar		plan;		/* calling nb plan (ACU_NB_PLAN_*) */
	uchar		pres;		/* calling nb presentation (ACU_NB_PRES_*) */
	uchar		screen;		/* calling nb screening indicator (ACU_NB_SCREEN_*) */
	uchar		reason;		/* (NTT only) reason for non-notification of the calling-nb (ACU_NB_NON_NOTIF_*) */
	uchar		sa_type;	/* calling sub-add type (ACU_SA_TYPE_*) */
	uchar		sa_format;	/* calling sub-add format (ACU_SA_FORMAT_*) -- meaningful only when sa_type==ACU_SA_TYPE_NSAP */
	uchar		sa_odd;		/* calling sub-add odd/even indicator (ACU_SA_ODD/EVEN) */
	acu_size_t	size;		/* size of the calling number in bytes */
};

/*----------------------------------------------------------------------------
 * Connected number/sub-add related information sub-structure
 *----------------------------------------------------------------------------*/

struct acu_conn_nb {
	uchar		type;		/* conn nb type (ACU_NB_TYPE_*) */
	uchar		plan;		/* conn nb plan (ACU_NB_PLAN_*) */
	uchar		pres;		/* conn nb presentation (ACU_NB_PRES_*) */
	uchar		screen;		/* conn nb screening indicator (ACU_NB_SCREEN_*) */
	uchar		sa_type;	/* conn sub-add type (ACU_SA_TYPE_*) */
	uchar		sa_odd;		/* conn sub-add odd/even indicator (ACU_SA_ODD/EVEN) */
	acu_size_t	size;		/* size of the conn number*sub-add string in bytes */
};

/*----------------------------------------------------------------------------
 * Redirecting/tion number related information sub-structure
 * Used to carry the Redirecting_nb IE in the ACU_CONN_RQ/IN,
 * and Redirection_nb IE in other ACU primitives (ACU_ALERT_IN, ACU_CONN_CO).
 *----------------------------------------------------------------------------*/

struct acu_redir_nb {
	uchar		pres;		/* redirecting/tion nb presentation (ACU_NB_PRES_*) */
	uchar		type;		/* redirecting/tion nb type (ACU_NB_TYPE_*) */
	uchar		plan;		/* redirecting/tion nb plan (ACU_NB_PLAN_*) */
	uchar		screen;		/* redirecting/tion nb screening indicator (ACU_NB_SCREEN_*) */
	uchar		reason;		/* reason for redirection (ACU_NB_REASON_*) */
	acu_size_t	size;		/* size of the redirecting/tion number in bytes */
};

/*----------------------------------------------------------------------------
 * Progress-indicator IE related information sub-structure
 *----------------------------------------------------------------------------*/

#ifndef ACU_MX_PROGRESS_IND
#	define	ACU_MX_PROGRESS_IND		1
#endif

struct acu_progress_ind {

	uchar nb;					/* number of progress_ind elements in the table */

	struct {
		uchar	loc;			/* progress location (ACUPROGRESS_LOC_*) */
		uchar	descr;			/* progress description (ACUPROGRESS_DESCR_*) */
	} t [ACU_MX_PROGRESS_IND];
};

/*----------------------------------------------------------------------------
 * Display IE.
 * Note: in E10 variant, the acu_display_list structure contains the
 * "Display-Field" IE rather than the ETSI-like Display IE.
 * The 'type' field only applies to the E10 and DMS variants,
 * and is ignored for other variants.
 * E10 'type' values can be: 1=normal, 2=inspect, 3=misc-info, 4=msg-retrieval, 5=electronic-directory-query;
 * DMS 'type' values can be: 0=unknown, 1=calling-party, 2=Connected-party, 5=original-called-party;
 * This IE can be repeated only for the E10 variant (up to 9 times).
 *--------------------------------------------------------------------------*/

#ifndef ACU_MX_DISPLAY
#	define	ACU_MX_DISPLAY			5
#endif

struct acu_display_list {
	uchar 		nb;					/* Nb of present occurences */
	uchar		total_size;			/* total size of the stored strings */
	struct {
		uchar		string_offset;	/* offset of the Display string, replacing the pointer stored in the p_display field;
									 * It is the offset of the string based on the address of the first display string;
									 * p_display -> element.string_offset[0] is therefore always null.
									 */
		uchar 		size;			/* Size of the string */
		uchar 		type;			/* Type of display (for E10 variant's "Display-Field" IE only) */
		char FAR *	p_display;		/* address of the string, kept for compatibility */
	} element[ACU_MX_DISPLAY];
};

#define	Acu_display_nb					p_display -> nb
#define	Acu_display_total_size			p_display -> total_size
#define	Acu_display_type(xx)			p_display -> element[xx].type
#define	Acu_display_offset(xx)			p_display -> element[xx].string_offset
#define	Acu_display_p_display(xx)		p_display -> element[xx].p_display
#define	Acu_display_size(xx)			p_display -> element[xx].size

	/* Acu_display_write(p_acu_display_list,p_display_strings,p_str,str_siz,type):
	 * macro to be used to store one instance of the Display IE into an ACU primitive.
	 * Input:
	 * p_display: address of the acu_display_list field in the ACU primitive, i.e. Acu_xxx_a_display_list;
	 * p_display_strings: address of the first display string in the ACU primitive, i.e. Acu_xxx_a_display;
	 * p_str: (char FAR *)address of the string to be stored in the ACU primitive;
	 * str_siz: (uchar) size of the string to be stored, including the trailing \0 for regular C strings;
	 * type: (uchar) type of Display (for E10 variant's "Display-Field" IE only);
	 *
	 * Example: Acu_display_write(Acu_conn_rq_a_display_list,Acu_conn_rq_a_display,"Hello",6,type);
	 */
#define Acu_display_write(p_acu_display_list,p_display_strings,p_str,str_siz,type) \
	{	struct acu_display_list FAR *p_display = p_acu_display_list; \
		uchar __nb__ = Acu_display_nb; \
		uchar __siz__ = str_siz; \
		Acu_display_type(__nb__) = type; \
		Acu_display_size(__nb__) = __siz__; \
		Acu_display_offset(__nb__) = Acu_display_total_size; \
		Acu_display_p_display (__nb__) = (char FAR *)p_display_strings + Acu_display_total_size; \
		Memcpy_far ((char FAR *)Acu_display_p_display(__nb__),p_str,__siz__); \
		Acu_display_total_size += __siz__; \
		Acu_display_nb++; \
	}

#define Acu_conn_rq_write_display_string(p_string,type) \

/*----------------------------------------------------------------------------
 * Date/Time information
 *--------------------------------------------------------------------------*/

struct acu_date_time {
	uchar available;		/* ON if the information is available, else OFF */
	uchar year;				/* year */
	uchar month;			/* month */
	uchar day;				/* day */
	uchar hour;				/* hour */
	uchar minute;			/* minute */
	uchar second;			/* second */
};

/*----------------------------------------------------------------------------
 * Charging information
 *--------------------------------------------------------------------------*/

#define ACU_CHARGING_CURRENCY_SIZE	11

struct acu_charging_info {
	long  value;			/* charging value (nb of units) */
	uchar available;		/* charging info available (ON) or not (OFF) */
	uchar type;				/* charging type (ACUCHARG_TYPE_*) */
	uchar period;			/* charging period (ACUCHARG_PERIOD_*) */
	uchar multi;			/* charging period or value multiplier (ACUCHARG_MULTI_*) */
	char  currency[ACU_CHARGING_CURRENCY_SIZE];	/* currency string (1..10 IA5 characters, with \0 terminator) */
};

/*---------------------------------------------------------------------------
 * Voice supplementary services management.
 *--------------------------------------------------------------------------*/

struct voice_supp_service {
	uchar   button_type;    /* Unknown, call appear or feature */
	uchar   module_nb;      /* 0-7, call appear buttons are assigned to module 0 */
	uchar   button_nb;      /* 1-254, 0 and 255 are reserved */
	uchar	call_appear;    /* selected call appearence */
	uchar	switchhook;     /* switchhook indication */
	uchar	conn_id;        /* CONFERENCE : Conn_id of the active call */
};

/*---------------------------------------------------------------------------
 * SPID / ENDPOINT ID management.
 *--------------------------------------------------------------------------*/

#if _CC_SPID_INFO == ON

	struct acu_endpoint_id {
		uchar action;		/* Action */
		uchar usid;			/* User Id */
		uchar tid;			/* Terminal Id */
		uchar interpreter;	/* Intrepreter */
	};

#endif

/*----------------------------------------------------------------------------
 * ACU primitives "extended elements"
 *----------------------------------------------------------------------------*/

#if ACU_PAR_EXTENDED_PRIM == ON /* { */

	/* "Extended elements" are structures that are appended to the ACU "classic"
	 * structure and variable-length strings.
	 * They carry various elements that are not part of the "classic" ACU
	 * structures, such as ETSI structured-ASN.1 elements, or LLC detailed
	 * content.
	 *
	 * The "extended elements" are appended after the ACU "classic" structures
	 * and strings (e.g. called/calling-nb, display strings, etc),
	 * into the same message/buffer data area.
	 *
	 * Each ACU "classic" structure contains an "extension descriptor" structure
	 * describing the number of extended elements, the total size of these
	 * elements, and the position of the first element in the data area.
	 * Each extended element is a sub-structure that contains an
	 * "extension header" sub-structure giving the element id and its total size.
	 */

	/*------------------------------------------------------------------*/
	/* 1. Extension descriptor, placed in the ACU primitives structures */
	/*------------------------------------------------------------------*/

	/* This structure is placed at the beginning of each ACU "classic" structure
	 * (e.g. struct acu_conn_in_args).
	 * The extension descriptor structure contains the following fields:
	 *   nb:      number of extended elements
	 *   lgth:    total length of extended elements
	 *   address: address of the first extended element.
	 * Access macros are defined for easy reading/writing of the extension
	 * descriptor fields, through a pointer called 'p_data', which needs
	 * to be declared by the user as follows:
	 *   uchar FAR *p_data;
	 */
struct acu_ext_descr {
	ushort		nb;			/* number of extended elements */
	ushort		lgth;		/* total length of extended elements */
	uchar FAR *	address;	/* address of the first extended element */
};

	/* Access macros through 'p_data' pointing to the extension descriptor */

#define Acu_ext_descr_nb		((struct acu_ext_descr FAR *)p_data) -> nb
#define Acu_ext_descr_lgth		((struct acu_ext_descr FAR *)p_data) -> lgth
#define Acu_ext_descr_address	((struct acu_ext_descr FAR *)p_data) -> address

	/*---------------------------------------------------------------*/
	/* 2. Extension header placed in each extended element structure */
	/*---------------------------------------------------------------*/

	/* Each extended element starts with a "struct acu_ext_hdr" extension
	 * header sub-structure:
	 *   id:   extended element id (ACU_EXT_*)
	 *   lgth: size of the extended element.
	 * This sub-structure is part of the extended element structure itself,
	 * Access macros are defined for easy reading/writing of the extension
	 * header fields, through a pointer called 'p_ext_data', which needs to
	 * be declared by the user as follows:
	 *   uchar FAR *p_ext_data;
	 */
struct acu_ext_hdr {
	ushort		id;			/* identifier */
	ushort		lgth;		/* length in bytes of the contents */
};

	/* Access macros through 'p_ext_data' pointing to the extension header */

#define Acu_ext_id			((struct acu_ext_hdr FAR *)p_ext_data) -> id
#define Acu_ext_lgth		((struct acu_ext_hdr FAR *)p_ext_data) -> lgth

	/* Extended elements identifiers */

#define ACU_EXT_LLC					0x0001	/* low layer compatibility */

#define ACU_EXT_FAC_ASN1			0x0100	/* ASN1 Facility element */
#define ACU_EXT_QSI_FAC_ASN1		0x0400	/* ASN1 Q-SIG specific Facility element */
#define ACU_EXT_NI2_FAC_ASN1		0x0410	/* ASN1 NI specific Facility element */

#define ACU_EXT_FAC_RETURN_ERR		0x0101	/* Facility Return Error */
#define ACU_EXT_FAC_REJECT			0x0102	/* Facility Reject */

#define ACU_EXT_CUG					0x0201	/* Closed User Group */

#define	ACU_EXT_SPF_FAC_IE			0x0301	/* Network Specific IE */

	/*---------------------------------*/
	/* 3. Extended elements structures */
	/*---------------------------------*/

		/*-----------------------------------*/
		/* 3a. LOW LAYER COMPATIBILITY (LLC) */
		/*                                   */
		/*   id = ACU_EXT_LLC                */
		/* lgth = Acu_ext_llc_size           */
		/*-----------------------------------*/

		/* This extended element contains the detailed fields
		 * of the Q.931 LLC information element.
		 * The extended element descriptor structure contains the following
		 * values:
		 *   hdr.id   = ACU_EXT_LLC;
		 *   hdr.lgth = Acu_ext_llc_size
		 */
#if ACU_PAR_EXT_LLC == ON /* { */

	struct acu_ext_llc {
		struct acu_ext_hdr hdr;	/* extension header */

			/* octet 3 */

		uchar	coding_std;		/* 3: coding standard */
		uchar	trf_cap;		/* 3: information transfer capability */
		uchar	out_band_neg;	/* 3a: negotiation indicator */

			/* octets 4,4a,4b */

		uchar	trf_mode;		/* 4: transfer mode */
		uchar	trf_rate;		/* 4: information transfer rate */
		uchar	structure;		/* 4a: structure */
		uchar	config;			/* 4a: configuration */
		uchar	establish;		/* 4a: establishment */
		uchar	symmetry;		/* 4b: symmetry */
		uchar	dest_rate;		/* 4b: information transfer rate, dest->orig */

			/* octets 5,5a,5b,5c,5d: Layer-1 info */

		uchar	l1_protocol;	/* 5: user info L1 protocol */
		uchar	syn_asyn;		/* 5a: synchronous/asynchronous */
		uchar	negotiation;	/* 5a: negotiation */
		uchar	user_rate;		/* 5a: user rate */
				/* 5b for l1_protocol=V.110 */
		uchar	inter_rate;		/* 5b/V.110: intermediate rate */
		uchar	nic_tx;			/* 5b/V.110: Network Independent Clock on Tx */
		uchar	nic_rx;			/* 5b/V.110: Network Independent Clock on Rx */
		uchar	fc_tx;			/* 5b/V.110: Flow Control on Tx */
		uchar	fc_rx;			/* 5b/V.110: Flow Control on Rx */
				/* 5b for l1_protocol=V.120 */
		uchar	ra_hdr;			/* 5b/V.120: rate adaption header/no header */
		uchar	multi_frame;	/* 5b/V.120: multi-frame establishment support in data-link */
		uchar	op_mode;		/* 5b/V.120: mode of operation */
		uchar	lli_negot;		/* 5b/V.120: Logical Link Identifier negotiation */
		uchar	assign;			/* 5b/V.120: assignor/assignee */
		uchar	band_negot;		/* 5b/V.120: in-band/out-band negotiation */
				/* 5c (only if l1_protocol=V.110 or V.120) */
		uchar	stop_bits;		/* 5c: nb of stop bits */
		uchar	data_bits;		/* 5c: nb of data bits */
		uchar	parity;			/* 5c: parity */
				/* 5d (only if l1_protocol=V.110 or V.120) */
		uchar	duplex;			/* 5d: duplex mode */
		uchar	modem;			/* 5d: modem type */

			/* octets 6,6a: Layer-2 info */

		uchar	l2_protocol;	/* 6: user info L2 protocol */
		uchar	l2_option;		/* 6a: optional user specified L2 parameter */

			/* octets 7,7a: Layer-3 info */

		uchar	l3_protocol;	/* 7: user info L3 protocol */
		uchar	l3_option;		/* 7a: optional user specified L3 parameter */
	};

	/* Access macros through 'p_ext_data' pointing to the extended element */

#	define Acu_ext_llc(field)	(((struct acu_ext_llc FAR *)p_ext_data) -> field)
#	define Acu_ext_llc_size		Rnd_sizeof (struct acu_ext_llc)

#endif /* ACU_PAR_EXT_LLC == ON } */

		/*--------------------------------------*/
		/* 3b. ASN1 Facility element structure  */
		/*                                      */
		/*   id = ACU_EXT_FAC_ASN1 for ETSI     */
		/* 	   or ACU_EXT_QSI_FAC_ASN1 for QSIG */
		/*--------------------------------------*/

		/* This extended element contains the ETSI or QSIG ASN.1 Facility
		 * information element.
		 * The ASN.1 structures are transparently transferred from/to the
		 * NS entity, and are therefore identical the ASN.1 structures
		 * provided at the NS interface.
		 * The extended element descriptor structure contains the following
		 * values:
		 *   hdr.id   = Acu_ext_id = ACU_EXT_FAC_ASN1 for ETSI
		 *                        or ACU_EXT_QSI_FAC_ASN1 for QSIG
		 *                        or ACU_EXT_NI2_FAC_ASN1 for NI2;
		 *   hdr.lgth = Acu_ext_lgth = total size of all ASN.1 components stored
		 *              after this extended element structure.
		 *
		 * ASN.1 components are NOT part of this structure, but are appended
		 * to it, as multiple ASN.1 components may be present, with various
		 * sizes for each components.
		 * The extended element structure "struct acu_ext_fac_asn1" only
		 * specifies the number of following components.
		 * Each component is then accessible using the macros and structures
		 * defined in acu_ss.h.
		 * See acu_ss.h for details on the ASN.1 components structures and
		 * macros.
		 *
		 * When Acu_ext_id==ACU_EXT_FAC_ASN1, the ASN.1 components start at
		 *    p_ss_component = p_ext_data + Acu_ext_fac_asn1_size;
		 *
		 * This does not apply when Acu_ext_id==ACU_EXT_QSI_FAC_ASN1.
		 * See acu_ss.h for details.
		 */
	struct acu_ext_fac_asn1 {
		struct acu_ext_hdr hdr;	/* extension header */

		ushort	component_nb;	/* nb of ASN1 components in the following facility */

			/* followed by the ASN1 structures */
	};

	/* Access macros through 'p_ext_data' pointing to the extended element */

#	define Acu_ext_fac_asn1(field)		(((struct acu_ext_fac_asn1 FAR *)p_ext_data) -> field)
#	define Acu_ext_fac_asn1_size		Rnd_sizeof (struct acu_ext_fac_asn1)

		/*---------------------------------------*/
		/* 3c. Facility "Return Error" structure */
		/*                                       */
		/*   id = ACU_EXT_FAC_RETURN_ERR         */
		/* lgth = Acu_ext_fac_return_err_size    */
		/*---------------------------------------*/

		/* This extended element contains the ETSI or QSIG ASN.1 Facility
		 * "Return Error" component structure.
		 * The extended element descriptor structure contains the following
		 * values:
		 *   hdr.id   = ACU_EXT_FAC_RETURN_ERR
		 *   hdr.lgth = Acu_ext_fac_return_err_size
		 */
	struct acu_ext_fac_return_err {
		struct acu_ext_hdr hdr;	/* extension header */

		short	invoke_id;		/* service invoke reference id */
		uchar	error_id;		/* error identifier */
	};

	/* Access macros through 'p_ext_data' pointing to the extended element */

#	define Acu_ext_fac_return_err(field)	(((struct acu_ext_fac_return_err FAR *)p_ext_data) -> field)
#	define Acu_ext_fac_return_err_size		Rnd_sizeof (struct acu_ext_fac_return_err)

		/*---------------------------------*/
		/* 3d. Facility "Reject" structure */
		/*                                 */
		/*   id = ACU_EXT_FAC_REJECT       */
		/* lgth = Acu_ext_fac_reject_size  */
		/*---------------------------------*/

		/* This extended element contains the ETSI or QSIG ASN.1 Facility
		 * "Reject" component structure.
		 * The extended element descriptor structure contains the following
		 * values:
		 *   hdr.id   = ACU_EXT_FAC_REJECT
		 *   hdr.lgth = Acu_ext_fac_reject_size
		 */
	struct acu_ext_fac_reject {
		struct acu_ext_hdr hdr;	/* extension header */

		uchar	present;		/* invoke_id present ? */
		short	invoke_id;		/* service invoke reference id */
		short	pb_type;		/* problem type */
		uchar	pb_val;			/* problem value */
	};

	/* Access macros through 'p_ext_data' pointing to the extended element */

#	define Acu_ext_fac_reject(field)	(((struct acu_ext_fac_reject FAR *)p_ext_data) -> field)
#	define Acu_ext_fac_reject_size		Rnd_sizeof (struct acu_ext_fac_reject)

		/*---------------------------------------------------*/
		/* 3e. Facility - CLOSED USER GROUPS (CUG) component */
		/*                                                   */
		/*   id = ACU_EXT_CUG                                */
		/* lgth = Acu_ext_cug_size                           */
		/*---------------------------------------------------*/

		/* This extended element contains the ETSI ASN.1 Facility
		 * "Closed User Group" (CUG) component structure.
		 * It may be used in ETS, KOR and TR6 variants only.
		 * The extended element descriptor structure contains the following
		 * values:
		 *   hdr.id   = ACU_EXT_CUG
		 *   hdr.lgth = Acu_ext_cug_size
		 */
#if ACU_PAR_EXT_CUG == ON /* { */

	struct acu_ext_cug {
		struct acu_ext_hdr hdr;	/* extension header */

		short	invoke_id;		/* service invoke reference id */
		uchar	out_access_rq;	/* outgoingAccessRequest : TRUE/FALSE or 0xFF if not present */
		ushort	index;			/* cUGIndex : INTEGER (0..32767) or 0xFFFF if not present */
	};

	/* Access macros through 'p_ext_data' pointing to the extended element */

#	define Acu_ext_cug(field)	(((struct acu_ext_cug FAR *)p_ext_data) -> field)
#	define Acu_ext_cug_size		Rnd_sizeof (struct acu_ext_cug)

#endif /* ACU_PAR_EXT_CUG == ON } */

		/*--------------------------------*/
		/* 3f. AT4 Network Specific IE    */
		/*                                */
		/*   id = ACU_EXT_SPF_FAC_IE      */
		/* lgth = Acu_ext_spf_fac_ie_size */
		/*--------------------------------*/

		/* This extended element contains the AT4 (AT&T 4ess) Specific-Facility
		 * information element.
		 * The extended element descriptor structure contains the following
		 * values:
		 *   hdr.id   = ACU_EXT_SPF_FAC_IE
		 *   hdr.lgth = Acu_ext_spf_fac_ie_size
		 */
#if ACU_PAR_EXT_SPF_FAC_IE == ON /* { */

#	define	ACU_MX_SZ_NET_ID		5			/* network id array */

	struct acu_ext_spf_fac_ie {
		struct acu_ext_hdr hdr;					/* extension header */
		uchar		net_id_lgth;	 			/* length of network identification */
		uchar		net_id_type;	 			/* type of ID */
		uchar		net_id[ACU_MX_SZ_NET_ID];	/* network identification array */
		uchar		action;						/* parameterized/binary indicator */
		uchar		serv_feature;				/* service/feature indicator */
		uchar		facility_coding;			/* facility coding */
		uchar		param_fld;					/* parameterized field */
	};

	/* Access macros through 'p_ext_data' pointing to the extended element */

#	define Acu_ext_spf_fac_ie(field)		(((struct acu_ext_spf_fac_ie FAR *)p_ext_data) -> field)
#	define Acu_ext_spf_fac_ie_size			Rnd_sizeof (struct acu_ext_spf_fac_ie)

		/* net_id_type */

#	define ACU_USER_SPECIFIED_TYPE			0	/* User specified */
#	define ACU_NATIONAL_NETWORK_ID_TYPE		2	/* National network */

		/*  action */
#	define ACU_SPF_FAC_PARAMETER			0	/* Parameter are associated with the facility */
#	define ACU_SPF_FAC_BINARY				1	/* Binary facility */

		/* serv_feature */

#	define ACU_FAC_SERVICE					1	/* Service */
#	define ACU_FAC_FEATURE					0	/* Feature */

		/* facility coding */

#	define ACU_FAC_CPN_SID_PREFERRED		1	/* CPN (SID) preferred */
#	define ACU_FAC_BN_ANI_PREFERRED			2	/* BN (ANI) preferred */
#	define ACU_FAC_CPN_SID_ONLY				3	/* CPN (SID) only */
#	define ACU_FAC_BN_ANI_ONLY				4	/* BN (ANI) only */
#	define ACU_FAC_CALL_ASSOC_TSC			9	/* Call associated TSC */
#	define ACU_FAC_TSC_CLEAR_RU				10	/* Notification of call associated TSC clearing or resource unavailable */
#	define ACU_FAC_OPERATOR					5	/* Operator */
#	define ACU_FAC_PCCO						6	/* Pre-subscribed Common Carrier Operator */

#	define ACU_FAC_SDN						1	/* SDN including GSDN */
#	define ACU_FAC_MEGACOM_800				2	/* Acces to MEGACOM 800 */
#	define ACU_FAC_MEGACOM					3	/* Acces to MEGACOM */
#	define ACU_FAC_ACCUNET					6	/* ACCUNET */
#	define ACU_FAC_LONG_DISTANCE			7	/* International long distance */
#	define ACU_FAC_INTERNATIONAL_800		8	/* International 800 */
#	define ACU_FAC_ATT_MULTIQUEST			16	/* AT&T MultiQuest */

		/* param_fld */

#	define	ACU_FAC_VARI_A_BILL				6	/* Vari-A-Bill (flexible billing) */

#endif  /* ACU_PAR_EXT_SPF_FAC_IE == ON } */

#endif /* ACU_PAR_EXTENDED_PRIM == ON } */

/*----------------------------------------------------------------------------
 *		SM_SET_CONFIG_RQ - Analog ACU entity configuration structure
 *
 * This configuration structure is only used by the analog ACU entity,
 * not by the ISDN ACU entity.
 * The ISDN ACU configuration structure is defined in cc_int.h.
 *----------------------------------------------------------------------------*/

#ifndef	ACU_CONFIG_MX_NA
#	define	ACU_CONFIG_MX_NA	1
#endif

struct acu_config {		/* ACU entity configuration */

	struct confirm 	acu_confirm;		/* 'acu_confirm': confirmation structure */
										/* Cause codes are below: */
#		define	ACU_CFG_CAUSE_ALREADY_INITIALIZED	0x81	/* Entity already initialized. Diagnostic: line index in configuration structure */
#		define	ACU_CFG_CAUSE_TOO_MANY_LINES		0x82	/* Too many lines in configuration structure. Diagnostic: ACU_CONFIG_MX_NA */
															/* */
#		define	ACU_CFG_CAUSE_PID_VL				0x91	/* Heap (PID_VL) memory unavailable. Diagnostic: see source code */
#		define	ACU_CFG_CAUSE_PID_TIMER				0x92	/* Timer pool memory unavailable. Diagnostic: timer index */

	uchar			nlines;				/* 'nlines': number of managed network accesses / lines */

	ent_id_t		user_entity;		/* 'user_entity': default user entity */
										/*                Typical value: ENT_DTE_INT, ENT_APPLI */
	uchar			can_ph_num_max_size;/* 'can_ph_num_size': maximum size of a canonical phone number */
										/*                    Suggested value: 30 */
	uchar			cmd_max_num;		/* 'cmd_max_num': maximum size of a sequence of internal ACU commands */
										/*                Suggested value: 16 */

	struct acu_config_na {				/* Array of network accesses descriptions */
		nai_t			nai;					/* 'nai': network access identifier */

		struct {								/* modem chip / PH entity description */
			ent_id_t	ph_ent_id;				/* 'ph_ent_id': name of the PH entity. 0: use default, compiled value */
												/*				Suggested value: ENT_PH_GSTN */
												/*				Another typical value is ENT_PH_MODEM */
			sapi_t		ph_sapi;				/* 'ph_sapi': name of the PH sapi. 0: use default, compiled value */
												/*				Suggested value: 0 */
			uchar		detect_complete_ri_train;/* 'detect_complete_ri_train': Ring Indication detection */
												/*								ON : the modem chip (PH entity) signals the last RI train when the final silence has expired */
												/*								OFF: the modem chip signals the last RI train before, as soon as the signal has disappeared (at the beginning of the silence) */
												/*								For Rockwell RC9623: set to ON */
			uchar		ced_data_by_chip;		/* 'ced_data_by_chip': CED in data mode */
												/*					   ON : the modem chip (PH entity) sends CED automatically in data mode */
												/*					   OFF: the modem chip does not. CED transmisssion is managed by ACU */
												/*					   For Rockwell RC9623: set to ON */
			uchar		ced_fax_by_chip;		/* 'ced_fax_by_chip': CED in fax mode */
												/*					  ON : the modem chip (PH entity) sends CED automatically in fax mode */
												/*					  OFF: the modem chip does not. CED transmisssion is managed by ACU */
												/*					  For Rockwell RC9623: set to OFF */
		} ph;

		short		max_rings;				/* 'max_rings': default maximum number of rings before answering. */
											/*				0: no autoanswer. */
											/*				Suggested value: 0 */
		uchar       dial_mode;				/* 'dial_mode': default dial mode (ACU_TONE_DIAL, ACU_PULSE_DIAL) */
											/*				Suggested value: ACU_TONE_DIAL */
		uchar		no_check;				/* 'no_check': disable regulation checks */
											/*			   ON : no check of phone numbers against regulation */
											/* 					and also no check of parameters (regmax_call_delay, regmin_call_delay) against regulation */
											/*			   Suggested value: OFF */
		uchar		direct_seizing;			/* 'direct_seizing': direct line seizing */
											/*					 ON:  direct line seizing (bypass check of line current presence) */
											/*					 OFF: check line current before seizing line */
											/*					 Suggested value: OFF */
		uchar		direct_dialing;			/* 'direct_dialing': wait for proceed (dial) tone */
											/*					 ON : direct dialing (never wait for proceed tone) */
											/*					 OFF: wait for proceed tone before dialing */
											/*					 Suggested value: OFF */
		uchar		dial_if_no_proceed;		/* 'dial_if_no_proceed': processing to perform if proceed tone is not detected in time */
											/*						 ON : dial even if proceed tone is not detected in time */
											/*						 OFF: do not dial if proceed tone is not detected in time */
											/*						 Suggested value: OFF */

		timer_val_t	init_delay;				/* 'init_delay': duration of the initial delay (afetr configuration) before being able to establish a call */
												/* Suggested value: 300*M_SCD */
		timer_val_t	intercall_delay;		/* 'intercall_delay': minimum duration between two successive outgoing calls */
												/* Suggested value: 100*M_SCD */
		timer_val_t	before_seizing_delay;	/* 'before_seizing_delay': duration of the attempt to detect RI and Line Current before line seizing */
												/* Suggested value: 2*SECOND */
		timer_val_t	proceed_delay;			/* 'proceed_delay': duration of the attempt to detect proceed tone before dialing */
												/* Suggested value: 3800*M_SCD. Or 12*SCD - a value between 6 and 12 seconds. */
		timer_val_t	international_delay;	/* 'international_delay': duration of the attempt to detect international proceed tone when dialing */
												/* Suggested value: same value as proceed_delay. 3800*M_SCD */
		timer_val_t	interregional_delay;	/* 'interregional_delay': duration of the attempt to detect interregional proceed tone when dialing */
												/* Suggested value: same value as proceed_delay. 3800*M_SCD */
		timer_val_t	call_delay;				/* 'call_delay': after all digits have been dialled, duration of the attempt to detect CED */
												/* Note: when using the DTE_INT entity, the corresponding field 'dte_ch_config.s7' in the DTE_INT configuration structure must be set to the same value */
												/* This parameter corresponds to the AT S7 (S007) register */
												/* Suggested value: 60*SECOND */
		timer_val_t	dial_pause_delay;		/* 'dial_pause_delay': if not waiting for proceed tone (blind dialling) : duration before blind dialling */
												/* Suggested value: 4*SECOND */
		timer_val_t	comma_pause_delay;		/* 'comma_dial_pause_delay': 'comma' dial pause duration */
												/* Suggested value: 2*SECOND */
		timer_val_t	dtmf_duration;			/* 'dtmf_duration': DTMF duration - not used */
												/* Suggested value: 68*M_SCD */
		timer_val_t	watchdog_delay;			/* 'watchdog delay': duration during which we ACU expect an answer from PH */
												/* Suggested value: 180*SECOND */
		timer_val_t	no_ring_delay;			/* 'no_ring_delay': ring indication period (delay after which ring synchronization is lost) */
												/* Suggested value (for France): 5*SECOND */
		timer_val_t	before_ced_delay;		/* 'before_ced_delay': duration of the delay before sending CED after line seizing (incoming call) */
												/* Suggested value: 1*SECOND */
		timer_val_t	after_ced_delay;		/* 'after_ced_delay': duration of the delay to wait after having sent CED before signalling successful call establishment (incoming call) */
												/* Suggested value: 0*M_SCD */
		timer_val_t	regmax_call_delay;		/* 'regmax_call_delay': regulation-specified maximum 'call_delay' */
												/* Suggested value (for France): 60*SECOND */
		timer_val_t	regmin_dial_pause_delay;/* 'regmin_dial_pause_delay': regulation-specified minimum 'dial_pause_delay' */
												/* Suggested value (for France): 4*SECOND */
		timer_val_t	regmax_dial_pause_delay;/* 'regmax_dial_pause_delay': regulation-specified maximum 'dial_pause_delay' */
												/* Suggested value (for France): 7*SECOND */
		timer_val_t	regmax_comma_pause_delay;/* 'regmax_comma_pause_delay': regulation-specified maximum 'comma_pause_delay' */
												/* Suggested value (for France): 3*SECOND */
		timer_val_t	regmin_dtmf_duration;	/* 'regmin_dtmf_duration': regulation-specified minimum 'dtmf_duration' */
												/* Suggested value: 68*M_SCD */
		timer_val_t	regmax_dtmf_duration;	/* 'regmax_dtmf_duration': regulation-specified maximum 'dtmf_duration' */
												/* Suggested value: 5*60*SECOND */
		uchar		regmin_max_rings;		/* 'regmin_max_rings': regulation-specified minimum 'max_rings' */
												/* Suggested value (for France): 3 */
		uchar		regmax_max_rings;		/* 'regmax_max_rings': regulation-specified maximum 'max_rings' */
												/* Suggested value (for France): 5 */
		uchar		gstn_prefix			 [ACU_PREFIX_SIZE + 1];	/* 'gstn_prefix': (PABX) prefix to access the network */
												/* Suggested value: "" */
		uchar		international_prefix [ACU_PREFIX_SIZE + 1];	/* 'international_prefix': international prefix */
												/* Suggested value: "" */
		uchar		interregional_prefix [ACU_PREFIX_SIZE + 1];	/* 'interregional_prefix': interregional prefix */
												/* Suggested value: "" */

		timer_val_t	regmin_call_delay;		/* 'regmin_call_delay': regulation-specified maximum 'call_delay' */
											/*						Suggested value (for France): 20*SECOND */
		timer_val_t	flash_hook_duration;	/* 'flash_hook_duration': duration of the flash hook */
											/*						  0 means: flash hook is not implemented */
											/*						  Suggested value: 100*M_SCD */

#       if ACU_PSTN_CLIP == ON
            struct {
                uchar enable_clip;          /* ' enable_clip': != 0  -> enable CLIP detector (PH_GSTN_ACT_SIGNAL_RQ (PH_GSTN_TONE_CALLER_ID)) */
                                            /*                 0     -> enable RING detector (PH_GSTN_ACT_SIGNAL_RQ (PH_GSTN_TONE_RI))        */
            }clip_param;
#       endif

		timer_val_t	end_of_dial_delay;		/* 'end_of_dial_delay': duration of delay after a sequence of digits (DTMF or pulse) has been dialled */
											/* Suggested value: 0, or a short duration */
	} acu_config_na [ACU_CONFIG_MX_NA];
};

#define	_Acf_	P_acu_config->
#define	_Ana_	p_acu_config_na->

#define		Cfig_can_ph_num_max_size		(_Acf_ can_ph_num_max_size)
#define		Cfig_nlines						(_Acf_ nlines)
#define		Cfig_cmd_max_num				(_Acf_ cmd_max_num)
#define		Cfig_user_entity				(_Acf_ user_entity)

#define		Cfig_a_na_config(i)				(&(_Acf_ acu_config_na [i]))

#define		Cfig_nai						(_Ana_ nai)
#define		Cfig_ph_ent_id					(_Ana_ ph.ph_ent_id)
#define		Cfig_ph_sapi					(_Ana_ ph.ph_sapi)
#define		Cfig_detect_complete_ri_train	(_Ana_ ph.detect_complete_ri_train)
#define		Cfig_ced_data_by_chip			(_Ana_ ph.ced_data_by_chip)
#define		Cfig_ced_fax_by_chip			(_Ana_ ph.ced_fax_by_chip)

#define		Cfig_dial_mode					(_Ana_ dial_mode)
#define		Cfig_direct_dialing				(_Ana_ direct_dialing)
#define		Cfig_direct_seizing				(_Ana_ direct_seizing)
#define		Cfig_dial_if_no_proceed			(_Ana_ dial_if_no_proceed)
#define		Cfig_max_rings					(_Ana_ max_rings)
#define		Cfig_no_check					(_Ana_ no_check)

#define		Cfig_after_ced_delay			(_Ana_ after_ced_delay)
#define		Cfig_before_ced_delay			(_Ana_ before_ced_delay)
#define		Cfig_init_delay					(_Ana_ init_delay)
#define		Cfig_intercall_delay			(_Ana_ intercall_delay)
#define		Cfig_before_seizing_delay		(_Ana_ before_seizing_delay)
#define		Cfig_call_delay					(_Ana_ call_delay)
#define		Cfig_dial_pause_delay			(_Ana_ dial_pause_delay)
#define		Cfig_comma_pause_delay			(_Ana_ comma_pause_delay)
#define		Cfig_dtmf_duration				(_Ana_ dtmf_duration)
#define		Cfig_no_ring_delay				(_Ana_ no_ring_delay)
#define		Cfig_proceed_delay				(_Ana_ proceed_delay)
#define		Cfig_international_delay		(_Ana_ international_delay)
#define		Cfig_interregional_delay		(_Ana_ interregional_delay)
#define		Cfig_watchdog_delay				(_Ana_ watchdog_delay)
#define		Cfig_regmin_dial_pause_delay	(_Ana_ regmin_dial_pause_delay)
#define		Cfig_regmax_dial_pause_delay	(_Ana_ regmax_dial_pause_delay)
#define		Cfig_regmin_call_delay			(_Ana_ regmin_call_delay)
#define		Cfig_regmax_call_delay			(_Ana_ regmax_call_delay)
#define		Cfig_regmax_comma_pause_delay	(_Ana_ regmax_comma_pause_delay)
#define		Cfig_regmin_dtmf_duration		(_Ana_ regmin_dtmf_duration)
#define		Cfig_regmax_dtmf_duration		(_Ana_ regmax_dtmf_duration)
#define		Cfig_regmin_max_rings			(_Ana_ regmin_max_rings)
#define		Cfig_regmax_max_rings			(_Ana_ regmax_max_rings)

#define		Cfig_gstn_prefix				(_Ana_ gstn_prefix)
#define		Cfig_international_prefix		(_Ana_ international_prefix)
#define		Cfig_interregional_prefix		(_Ana_ interregional_prefix)
#define		Cfig_flash_hook_duration		(_Ana_ flash_hook_duration)

#if ACU_PSTN_CLIP == ON
#   define  Cfig_enable_clip                (_Ana_ clip_param.enable_clip)
#endif
#define		Cfig_end_of_dial_delay			(_Ana_ end_of_dial_delay)


/*----------------------------------------------------------------------------
 *		SM_RESET_CONFIG_RQ - Analog ACU reset configuration request
 *
 *	nai:	nai of the line to reset
 *  sapi:	SAPI_NIL
 *	add	:	ADD_NIL
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *		SM_RESET_CONFIG_CO - Analog ACU reset configuration confirm
 *
 *	nai:	nai of the line to reset
 *  sapi:	SAPI_NIL
 *	add	:	ADD_NIL
 *	arguments: struct confirm
 *----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 *	Cause codes:
 *----------------------------------------------------------------------------*/
#define	ACU_RESET_CFG_CAUSE_ENTITY_NOT_CONFIGURED	0xA1	/* Entity not configured. Diagnostic: 1, 2 */
#define	ACU_RESET_CFG_CAUSE_LINE_NOT_CONFIGURED		0xA2	/* Line not configured. Diagnostic: 0 */
#define	ACU_RESET_CFG_CAUSE_INVALID_LINE_ID			0xA3	/* Invalid line identifier. Diagnostic: line identifier */


#if ACU_PSTN_CLIP == ON

    /* Some comments about storage of Caller ID:                                                                                              */
    /* - Specifications: ETS 300 659-1:February 1997                                                                                          */
    /* - The CLIP raw message are stored at the end of data area of this structure                                                            */
    /* - Each field of CLIP is described by xxxx_size and xxxx_off                                                                            */
    /*   If field is absent from CLIP then xxxx_size = 0                                                                                      */
    /*   otherwise xxxx_off give offset into raw_msg of field contents                                                                        */
    /*                                                                                                                                        */
    /* Example:                                                                                                                               */
    /*                                                                                                                                        */
    /*                                                                                                                                        */
    /*        :<----------------------------- raw_msg_size ----------------------------------->:                                              */
    /*        :                                                                                :                                              */
    /*        msg_name                                                                         :                                              */
    /*        :                                                                                checksum                                       */
    /*        :  Length of CLIP Msg                                                            :                                              */
    /*        :  :<------------------------------------------------------------------------>:  :                                              */
    /*        :  :                                                                          :  :                                              */
    /*        80 19 01 08 30 35 32 34 31 38 32 39 02 0a 30 34 34 32 39 31 34 34 37 36 16 01 0a 89                                             */
    /*              :  :  :        :  :        :  :  :  :                          :  :  :  :                                                 */
    /*              :  :  :< date >:  :< time >:  :  :  :                          :  :  :  :                                                 */
    /*              :  :  :                    :  :  :  :                          :  :  :  :                                                 */
    /*              :  :  date_time_off        :  :  :  cli_off                    :  :  :  t_calling_user_off                                */
    /*              :  :                       :  :  :                             :  :  :                                                    */
    /*              :  Len (date_time_size)    :  :  Len (cli_size)                :  :  Len (t_calling_user_size)                            */
    /*              :                          :  :                                :  :                                                       */
    /*              :<--- Info data time ----->:  :<- Info Calling Line Identity ->:  : Info Type of calling user                             */
    /*                                                                                                                                        */
    /*                                                                                                                                        */
    /* So Acu_conn_in_raw_ms_size         = 0x21                                                                                              */
    /*    Acu_conn_in_a_raw_msg           = ptr on 80 19 01 ....                                                                              */
    /*                                                                                                                                        */
    /*    Acu_conn_in_msg_name            = 0x80                                                                                              */
    /*                                                                                                                                        */
    /*    Acu_conn_in_date_time_size      = 0x08                                                                                              */
    /*    Acu_conn_in_a_date_time         = Ptr on 30 35 32 34 31 38 32 39                                                                    */
    /*                                                                                                                                        */
    /*    Acu_conn_in_cli_size            = 0x0A                                                                                              */
    /*    Acu_conn_in_a_cli               = 30 34 34 32 39 31 34 34 37                                                                        */
    /*                                                                                                                                        */
    /*    Acu_conn_in_t_calling_user_size = 0x01                                                                                              */
    /*    Acu_conn_in_a_t_calling_user    = Ptr on 0x01                                                                                       */
    /*    Acu_conn_in_t_calling_user      = 0x01                                                                                              */
    /*                                                                                                                                        */

    /* Some comments about Caller ID reception:                                                                                               */
    /*                                                                                                                                        */
    /*  a) If CLIP is received before first ring                                                                                              */
    /*     then ACU_CONN_IN is sent to the upper layer with CLIP                                                                              */
    /*     and when the first ring is received then ACU_ALERT_IN is sent to the upper layer .                                                 */
    /*                                                                                                                                        */
    /*  b) If CLIP is received after first ring; when this first ring is received,                                                            */
    /*     ACU_CONN_IN is sent to the upper layer with CLIP infos and ACU_ALERT_IN is also sent.                                              */
    /*                                                                                                                                        */
    /*  c) If there is NO CLIP, when the first ring is received                                                                               */
    /*     then ACU_CONN_IN is sent to the upper layer (No CLIP info) and ACU_ALERT_IN is also sent.                                          */
    /*                                                                                                                                        */
    /*  d) If there is more CLIP info between rings then ACU_ALERT_IN is sent to the upper layer with CLIP info                               */
    /*     (ACU_CONN_IN has been previously sent).                                                                                            */
    /*                                                                                                                                        */
    /*  e) The clip message is presented to upper layer only if its length, checksum, .. are correct                                          */


    struct acu_clip__message {                      /*                                                                                        */
                                                    /*                                                                                        */
        acu_size_t raw_msg_size;                    /*                       0, No raw message, so no CLIP decoded                            */
                                                    /*                       Max length = 255 (Clip message with its checksum)                */
                                                    /*                                                                                        */
                                                    /*              Reference                                                                 */
                                                    /*              (Subclause)                                                               */
        uchar msg_name;                             /*                       Message name: Call Setup or Message waiting indicator            */
#       define ACU_PSTN_CLIP__CALL_SETUP       0x80 /*              [7.2.1]                                                                   */
#       define ACU_PSTN_CLIP__MSG_WAITING      0x82 /*              [7.2.2]                                                                   */
#       define ACU_PSTN_CLIP__ADVISE_OF_CHARGE 0x86 /*              [7.2.3]  Not yet defined                                                  */
                                                    /*                       Other values are unknown                                         */
                                                    /*                                                                                        */
                                                    /*                                                                                        */
                                                    /*                                                                                        */
                                                    /* Call  Msg                                                                              */
                                                    /* Setup Wait                                                                             */
                                                    /* +----+----+                                                                            */
        acu_size_t date_time_size;                  /* : xx : xx :  [7.4.1]  date time information                                            */
        acu_size_t date_time_off;                   /* :    :    :           Len  : 0 or 8                                                    */
                                                    /* +----+----+                                                                            */
        acu_size_t cli_size;                        /* : xx : xx :  [7.4.2]  Calling line identity                                            */
        acu_size_t cli_off;                         /* :    :    :           Len             : 0 to 20                                        */
                                                    /* :    :    :           Authorized chars: "0" to "9" "*" "#" Space "-" "(" ")" and ","   */
                                                    /* :    :    :                                                                            */
        acu_size_t reason_no_cli_size;              /* : xx : xx :  [7.4.4]  Reason of absence of Calling line identity                       */
        acu_size_t reason_no_cli_off;               /* :    :    :           Len  : 0, 1                                                      */
                                                    /* :    :    :           Value: "O"          -> Unavailable                               */
                                                    /* :    :    :                  "P"          -> Private (CLIR invloved)                   */
                                                    /* :    :    :                  0x80 to 0xFF -> Reserved for network operator use         */
                                                    /* +----+----+                                                                            */
        acu_size_t called_li_size;                  /* : xx :    :  [7.4.3]  Called line identity                                             */
        acu_size_t called_li_off;                   /* :    :    :           Len             : 0 to 20                                        */
                                                    /* :    :    :           Authorized chars: "0" to "9" "*" "#" Space "-" "(" ")" and ","   */
                                                    /* +----+----+                                                                            */
        acu_size_t cpn_size;                        /* : xx : xx :  [7.4.5]  Calling party name                                               */
        acu_size_t cpn_off;                         /* :    :    :           Len             : 0 to 50                                        */
                                                    /* :    :    :           Authorized chars: any ascii characters                           */
                                                    /* :    :    :                                                                            */
        acu_size_t reason_no_cpn_size;              /* : xx : xx :  [7.4.6]  Reason of absence of Calling party name                          */
        acu_size_t reason_no_cpn_off;               /* :    :    :           Len  : 0, 1                                                      */
                                                    /* :    :    :           Value: "O"          -> Unavailable                               */
                                                    /* :    :    :                  "P"          -> Private (name delivery has been blocked)  */
                                                    /* :    :    :                  0x80 to 0xFF -> Reserved for network operator use         */
                                                    /* +----+----+                                                                            */
        acu_size_t complementary_cli_size;          /* : xx :    :  [7.4.8]  Complementary calling line identity                              */
        acu_size_t complementary_cli_off;           /* :    :    :           Len             : 0 to 20                                        */
                                                    /* :    :    :           Authorized chars: "0" to "9" "*" "#" Space "-" "(" ")" and ","   */
                                                    /* +----+----+                                                                            */
        acu_size_t call_type_size;                  /* : xx :    :  [7.4.9]  Call type information                                            */
        acu_size_t call_type_off;                   /* :    :    :           Len  : 0, 1                                                      */
                                                    /* :    :    :           Value: 0x01         -> Voice call                                */
                                                    /* :    :    :                  0x02         -> CLI ring back when free call              */
                                                    /* :    :    :                  0x03         -> Calling name delivery                     */
                                                    /* :    :    :                  0x81         -> Message waiting call                      */
                                                    /* :    :    :                  0x82 to 0xFF -> Reserved for network operator use         */
                                                    /* +----+----+                                                                            */
        acu_size_t first_called_li_size;            /* : xx :    :  [7.4.10] First called line identity                                       */
        acu_size_t first_called_li_off;             /* :    :    :           Len             : 0 to 20                                        */
                                                    /* :    :    :           Authorized chars: "0" to "9" "*" "#" Space "-" "(" ")" and ","   */
                                                    /* +----+----+                                                                            */
        acu_size_t net_msg_sys_sts_size;            /* : xx : xx :  [7.4.11] Network message system status                                    */
        acu_size_t net_msg_sys_sts_off;             /* :    :    :           Len  : 0, 1                                                      */
                                                    /* :    :    :           Value: 0        -> No message                                    */
                                                    /* :    :    :                  1        -> 1 message or unspecified numder of message    */
                                                    /* :    :    :                              waiting                                       */
                                                    /* :    :    :                  2 to 255 -> Number of message waiting                     */
                                                    /* +----+----+                                                                            */
        acu_size_t t_frw_call_size;                 /* : xx :    :  [7.4.12] Type of forwarded call                                           */
        acu_size_t t_frw_call_off;                  /* :    :    :           Len  : 0, 1                                                      */
                                                    /* :    :    :           Value: 0x00         -> Unavailable or unknown forwarded call type*/
                                                    /* :    :    :                  0x01         -> Forwarded call on busy                    */
                                                    /* :    :    :                  0x02         -> Forwarded call on no reply                */
                                                    /* :    :    :                  0x03         -> Unconditionnal forwarded call             */
                                                    /* :    :    :                  0x04         -> Deflected call (after alerting)           */
                                                    /* :    :    :                  0x05         -> Deflected call (immediate)                */
                                                    /* :    :    :                  0x06         -> Forwarded call on inability to reach      */
                                                    /* :    :    :                                  mobile subscriber                         */
                                                    /* :    :    :                  0x80 to 0xFF -> Reserved for network operator use         */
                                                    /* +----+----+                                                                            */
        acu_size_t t_calling_user_size;             /* : xx :    :  [7.4.13] Type of calling user                                             */
        acu_size_t t_calling_user_off;              /* :    :    :           Len  : 0, 1                                                      */
                                                    /* :    :    :           Value: 0x00 -> Origination unknown or unavailable                */
                                                    /* :    :    :                  0x03 -> VPN (Virtual Private Network)                     */
                                                    /* :    :    :                  0x04 -> Mobile phone                                      */
                                                    /* :    :    :                  0x05 -> Mobile phone + VPN                                */
                                                    /* :    :    :                  0x0A -> Ordinary calling subscriber                       */
                                                    /* :    :    :                  0x0B -> Calling subscriber with priority                  */
                                                    /* :    :    :                  0x0C -> Data call                                         */
                                                    /* :    :    :                  0x0D -> Test call                                         */
                                                    /* :    :    :                  0x0F -> Payphone                                          */
                                                    /* +----+----+                                                                            */
        acu_size_t redirecting_nb_size;             /* : xx :    :  [7.4.14] Redirecting number                                               */
        acu_size_t redirecting_nb_off;              /* :    :    :           Len             : 0 to 20                                        */
                                                    /* :    :    :           Authorized chars: "0" to "9" "*" "#" Space "-" "(" ")" and ","   */
                                                    /* +----+----+                                                                            */
                                                    /* : xx :    :  [7.4.15] Charge parameter                                                 */
                                                    /* :    :    :           Not yet defined into ETS 300 659-1:February 1997                 */
                                                    /* +----+----+                                                                            */
        acu_size_t ext_for_op_use_size;             /* : xx : xx :  [7.4.16] Extension for network operator use                               */
        acu_size_t ext_for_op_use_off;              /* :    :    :           Len  : 0 or 10                                                   */
                                                    /* :    :    :           Digit: 1       -> First digit of country code                    */
                                                    /* :    :    :                  2       -> Second digit of country code or space (0x20)   */
                                                    /* :    :    :                  3       -> Third digit of country code or space (0x20)    */
                                                    /* :    :    :                  4 to 7  -> Network operator use                           */
                                                    /* :    :    :                  8 to 10 -> Version (operator coding)                      */
                                                    /* +----+----+                                                                            */
        acu_size_t visual_in_size;                  /* :    : xx :  [7.4.7]  Visual indicator                                                 */
        acu_size_t visual_in_off;                   /* :    :    :           Len  : 0, 1                                                      */
                                                    /* :    :    :           Value: 0x00         -> Deactivation (indicator off)              */
                                                    /* :    :    :                  0x01         -> Activation (indicator on)                 */
                                                    /* :    :    :                  0x80 to 0xFE -> Reserved for network operator use         */
                                                    /* +----+----+                                                                            */

        ushort     chk_alignment;                   /* Field used to check alignement of primitive (this field is always loaded               */
                                                    /* with ACU_CLIP__CHK_ALIGNMENT)                                                          */
#           define ACU_CLIP__CHK_ALIGNMENT  0x1234  /*                                                                                        */

    }; /* struct acu_clip__message */



#endif /* #if ACU_PSTN_CLIP == ON */

/*----------------------------------------------------------------------------
 *			DETAILED DESCRIPTION OF PRIMITIVES SENT TO ACU ENTITY
 *--------------------------------------------------------------------------*/

		/* length of a 'size' field within a ACU primitive */

#define ACU_SIZE_FIELD_LGTH					sizeof (acu_size_t)

/*---------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_CONN_RQ			request establishment of an outgoing call.
 *
 *	Arguments :
 *	(*)	Acu_conn_rq_priority 			: phone call priority normal/urgent
 *	(*)	Acu_conn_rq_auto_dial 			: automatical dialing (ON) or
 *										  just go off hook (OFF)
 *										  | ACU_AUTODIAL_NO_BUSY_DETECTION: do not detect busy tone.
 *	(*)	Acu_conn_rq_service				: requested (tele)service
 *		Acu_conn_rq_sending_complete	: Sending-complete IE to be generated (ON/OFF)
 *	(*)	Acu_conn_rq_ph_rate 			: physical rate (for all services)
 *										  - for fax service using ACU: set to either
 *										    ACU_RA_56000 (use V.8) or 0 (do not use V.8).
 *  (*) Acu_conn_rq_l1_protocol         : ISDN ACU  : user information layer 1 protocol
 *                                        Analog ACU: A Law / Mu Law selection if ISDN interworking connection is established
 *                                                    (ACU_L1_PROTOCOL_MU_LAW / ACU_L1_PROTOCOL_A_LAW)
 *	(*)	Acu_conn_rq_no_v25 			    : (Used only by Analog ACU)
 *                                        Disable V.25 procedure before data modulation (service == ACU_MODEM_SERVICE)
 *                                        -> Calling tone (1300Hz) is not sent.
 *                                        -> But answer tone is always waited (V.25: 2100Hz, Bell 103: 2225Hz or Bell 212a: 2250Hz)
 *                                        This field must be reset (OFF) with ITU modulation (V.21, V.22, ...)
 *                                        This field must be set (ON) with Bell modulation (Bell 103 or Bell 212a)
 *                                        Rem: this field is ignored if service == ACU_FAX_SERVICE
 *		Acu_conn_rq_user_rate			: requested user rate (for V.110 and V.120 services only)
 *		Acu_conn_rq_syn_asyn 			: synchronous/asynchronous (for V.110 and V.120 services only)
 *		Acu_conn_rq_stop_bits   		: nb of stop bits (for V.110 and V.120 services only)
 *		Acu_conn_rq_data_bits   		: nb of data bits (for V.110 and V.120 services only)
 *		Acu_conn_rq_parity      		: parity (for V.110 and V.120 services only)
 *      Acu_conn_rq_data_chani_nb		: nb of elements in the chani_list.tab field
 *      Acu_conn_rq_data_chani  		: data channel to use (B1, B2, etc, D, or 0 for any)
 *		Acu_conn_rq_charging_rq			: charging info requested
 *		Acu_conn_rq_transfer_rq 		: transfer requested
 *		Acu_conn_rq_transfer_con_id 	: conn_id of the call to be transfered
 *		Acu_conn_rq_ident_denied_rq 	: identity denied requested
 *		Acu_conn_rq_identification_denied_rq : alias macro for Acu_conn_rq_ident_denied_rq
 *		Acu_conn_rq_calling_nb_type		: calling nb type (ACU_NB_TYPE_*)
 *		Acu_conn_rq_calling_nb_plan		: calling nb plan (ACU_NB_PLAN_*)
 *		Acu_conn_rq_calling_nb_pres		: calling nb presentation
 *		Acu_conn_rq_calling_nb_screen	: calling nb screening indicator (ACU_NB_SCREEN_*)
 *		Acu_conn_rq_calling_nb2_type	: 2nd calling nb type (ACU_NB_TYPE_*)
 *		Acu_conn_rq_calling_nb2_plan	: 2nd calling nb plan (ACU_NB_PLAN_*)
 *		Acu_conn_rq_calling_nb2_pres	: 2nd calling nb presentation
 *		Acu_conn_rq_calling_nb2_screen	: 2nd calling nb screening indicator
 *		Acu_conn_rq_called_nb_type		: called nb type (ACU_NB_TYPE_*)
 *		Acu_conn_rq_called_nb_plan		: called nb plan (ACU_NB_PLAN_*)
 *	(*)	Acu_conn_rq_called_nb_size		: called number size in bytes
 *	(*)	Acu_conn_rq_a_called_nb			: address of the called number.
 *		(Acu_conn_rq_ph_num_size		: alias for Acu_conn_rq_called_nb_size)
 *		(Acu_conn_rq_a_ph_num 			: alias for Acu_conn_rq_a_called_nb)
 *		Acu_conn_rq_calling_nb_size,
 *		Acu_conn_rq_a_calling_nb		: calling number.
 *		Acu_conn_rq_calling_nb2_size,
 *		Acu_conn_rq_a_calling_nb2		: 2nd calling number.
 *		Acu_conn_rq_redir_nb_pres,
 *		Acu_conn_rq_redir_nb_type,
 *		Acu_conn_rq_redir_nb_plan,
 *		Acu_conn_rq_redir_nb_screen,
 *		Acu_conn_rq_redir_nb_reason,
 *		Acu_conn_rq_redir_nb_size,
 *		Acu_conn_rq_a_redir_nb			: redirecting nb
 *		Acu_conn_rq_uui_size,
 *		Acu_conn_rq_a_uui				: ISDN user to User Information
 *		Acu_conn_rq_facility_size,
 *		Acu_conn_rq_a_facility			: ISDN facility
 *		Acu_conn_rq_pcs_user_size,
 *		Acu_conn_rq_a_pcs_user			: ISDN PCS-to-User/User-to-PCS Info.
 *		Acu_conn_rq_call_appear			: call_appearance
 *		Acu_conn_rq_progress_location	: progress description
 *		Acu_conn_rq_progress_description: progress location
 *		Acu_conn_rq_notif_ind			: notification indicator description
 *		Acu_conn_rq_notif_descr			: (alias)notify description
 *		Acu_conn_rq_mlpp_present		; Q.955 MLPP information fields present ON/OFF; when ON, the following MLPP fields are meaningful
 *		Acu_conn_rq_mlpp_prec_level		: Q.955 MLPP Precedence Level: 0:flash-override(highest) .. 4:Routine(lowest)
 *		Acu_conn_rq_mlpp_lfb_ind		: Q.955 MLPP LFB Indication:   0:allowed, 1:not-allowed, 2:path-reserved; -- set by the originating exchange
 *		Acu_conn_rq_mlpp_domain_size	: Q.955 MLPP Service Domain: size of the Service Domain string (max 5)
 *		Acu_conn_rq_a_mlpp_domain		: Q.955 MLPP Service Domain: 5-octets value (non-zero-terminated)
 *		Acu_conn_rq_transit_counter		: Transit Counter (QSIG only)
 *
 *		ARINC SPECIFIC :
 *		Acu_conn_rq_spf_fac_id			: spf facility Id
 *		Acu_conn_rq_spf_fac_val			: spf_facility value
 *		Acu_conn_rq_departure_year		: year of departure
 *		Acu_conn_rq_departure_month		: month of departure
 *		Acu_conn_rq_departure_day		: day of departure
 *		Acu_conn_rq_service_class_id	: service class Id
 *		Acu_conn_rq_nats_route_ind		: nats route
 *		Acu_conn_rq_satcom_route_ind	: stacom route
 *		Acu_conn_rq_tfts_route_ind		: tfts route
 *
 *		Acu_conn_rq_track_1_size		: size of the track #1
 *		Acu_conn_rq_track_2_size        : size of the track #2
 *		Acu_conn_rq_flight_op_id_size	: size of the fligth op Id
 *		Acu_conn_rq_flight_number_size	: size of the flight digits
 *
 *		Acu_conn_rq_a_track_1			: address of the track #1
 *		Acu_conn_rq_a_track_2           : address of the track #2
 *		Acu_conn_rq_a_flight_op_id		: address of op Id
 *		Acu_conn_rq_a_flight_number		: address of flight digits
 *
 *		Warning : when building this primitive, the phone number field must
 *		be filled BEFORE the CALLING_NB field.
 *
 *  (*) Only these fields are used by the Analog ACU, other fields are ignored by the Analog ACU.
 *--------------------------------------------------------------------------*/

struct acu_conn_rq_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	priority;					/* phone call priority */
	uchar	auto_dial;					/* for analog ACU. Bit mask of:				*/
										/*	(bit 0)	:	OFF: go off hook only;		*/
										/*				ON : dial number			*/
										/*	(bit 1) :	ACU_AUTODIAL_NO_BUSY_DETECTION	*/
										/*				bit 1 set : do not detect busy tone */
										/*				bit 1 not set : detect busy tone */
										/*	(bit 2) :	ACU_AUTODIAL_NO_UNOBTAINABLE_DETECTION */
										/*				bit 2 set : do not detect "number unobtainable" tone */
										/*				bit 2 not set : detect "number unobtainable" tone */
										/*	Typical value for this field is ON. */
#	define	ACU_AUTODIAL_NO_BUSY_DETECTION			0x02
#	define	ACU_AUTODIAL_NO_UNOBTAINABLE_DETECTION	0x04

	uchar	service;					/* requested service */

    uchar	no_v25;                     /* (Used only by Analog ACU) */
                                        /* ON:  Disable V.25 procedure before data modulation (service == ACU_MODEM_SERVICE) */
                                        /* OFF: Enable V.25 procedure */

	uchar	sending_complete;			/* Sending-complete information element generation */
	struct layer_1_info layer_1_info;	/* layer 1 information field */
	struct chani_list   chani_list;		/* list of channel ids to use for data transfer */
	uchar	charging_rq;				/* request charging ON or OFF */
	uchar	transfer_rq;				/* request transfer ON or OFF */
	add_t	conn_id;					/* transfer conn_id */
	uchar	ident_denied_rq;			/* request identity denied ON or OFF */
	uchar	call_appear;				/* Call appearence */

	struct acu_called_nb  called_nb;	/* called nb information */
	struct acu_calling_nb calling_nb;	/* calling nb information */
	struct acu_calling_nb calling_nb2;	/* second calling nb information */
	struct acu_redir_nb   redir_nb;		/* redirecting nb information */
	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* size of the ISDN User to User information field */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

	struct acu_progress_ind progress_ind;/* progress indicator */

	uchar		notif_ind_present;		/* notification indicator present (ON/OFF) */
	uchar		notif_ind;				/* notification indicator description */
	uchar		notif_ind_ext_size;		/* notification-indicator extension (QSIG) */

	uchar		mlpp_present;			/* Q.955 MLPP information fields present ON/OFF; when ON, the following MLPP fields are meaningful */
	uchar		mlpp_prec_level;		/* Q.955 MLPP Precedence Level: 0:flash-override(highest) .. 4:Routine(lowest) */
	uchar		mlpp_lfb_ind;			/* Q.955 MLPP LFB Indication:   0:allowed, 1:not-allowed, 2:path-reserved */
	uchar		mlpp_domain_size;		/* Q.955 MLPP Service Domain: size of the Service Domain string (max 5) */
	uchar		mlpp_domain[5];			/* Q.955 MLPP Service Domain: 5-octets value (any binary, non-zero-terminated) */

	uchar		transit_counter;		/* Transit counter (QSIG only) */

#	if ARINC_NETWORKS == ON

#		if CC_SPF_FAC == ON
			uchar	spf_fac_id;			/* facility identifier */
			uchar	spf_fac_val;		/* facility value */
#		endif

		uchar	departure_year;			/* departure year */
		uchar	departure_month;    	/* departure  month */
		uchar	departure_day;      	/* departure day */
		uchar	service_class_id;		/* service class identifier defined by each airline */

		uchar 	nats_route_ind;			/* nats route indication */
		uchar 	satcom_route_ind;		/* satcom route indication */
		uchar 	tfts_route_ind;			/* tfts route indication */

		acu_size_t track_1_size;		/* information of track 1 */
		acu_size_t track_2_size;		/* information of track 2 */

		acu_size_t flight_op_id_size;	/* flight operator identity size */
		acu_size_t flight_number_size;	/* flight number size */

#	endif

#	if _CC_SPID_INFO == ON
		struct acu_endpoint_id endpoint_id;	/* Endpoint Id */
#	endif

										/* phone number is just here */
										/* then the calling number */
										/* than second calling nb information */
										/* than redirecting nb information */
										/* then the UUI */
										/* then facility */
										/* then display strings */
										/* then pcs_user */
										/* than notification-indicator extension (QSIG) */
										/* then track 1 info */
										/* then track 2 info */
										/* then flight operator id */
										/* then flight number */
};

#define	Acu_conn_rq_					((struct acu_conn_rq_args FAR *)p_data) ->
#define	Acq_							Acu_conn_rq_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_conn_rq_ext_parms_nb		Acq_ ext_descr.nb
#	define Acu_conn_rq_ext_parms_lgth	Acq_ ext_descr.lgth
#	define Acu_conn_rq_a_ext_parms		Acq_ ext_descr.address
#endif

#define	Acu_conn_rq_priority			Acq_ priority
#define	Acu_conn_rq_auto_dial			Acq_ auto_dial

#define	Acu_conn_rq_service				Acq_ service
#define	Acu_conn_rq_sending_complete	Acq_ sending_complete

#define	Acu_conn_rq_no_v25 			    Acq_ no_v25

#define	Acu_conn_rq_a_layer_1_info		((struct layer_1_info FAR *)&Acq_ layer_1_info)
#define	Acu_conn_rq_ph_rate				Acq_ layer_1_info.ph_rate
#define	Acu_conn_rq_user_rate			Acq_ layer_1_info.user_rate
#define	Acu_conn_rq_syn_asyn			Acq_ layer_1_info.syn_asyn
#define	Acu_conn_rq_stop_bits			Acq_ layer_1_info.stop_bits
#define	Acu_conn_rq_data_bits			Acq_ layer_1_info.data_bits
#define	Acu_conn_rq_parity				Acq_ layer_1_info.parity
#define	Acu_conn_rq_l1_protocol			Acq_ layer_1_info.l1_protocol

#define Acu_conn_rq_a_chani_list        ((struct chani_list FAR *)&Acq_ chani_list)
#define Acu_conn_rq_data_chani_nb       Acq_ chani_list.nb
#define Acu_conn_rq_data_chani          Acq_ chani_list.tab[0].chan_id
#define Acu_conn_rq_data_chani_nai      Acq_ chani_list.tab[0].nai
#define Acu_conn_rq_data_chani_tab(i)   Acq_ chani_list.tab[i].chan_id
#define Acu_conn_rq_data_chani_tab_nai(i) Acq_ chani_list.tab[i].nai
#define Acu_conn_rq_data_chani_excl     Acq_ chani_list.chan_id_excl

#define	Acu_conn_rq_charging_rq			Acq_ charging_rq
#define	Acu_conn_rq_transfer_rq			Acq_ transfer_rq
#define	Acu_conn_rq_transfer_conn_id	Acq_ conn_id
#define	Acu_conn_rq_ident_denied_rq		Acq_ ident_denied_rq
#define	Acu_conn_rq_identification_denied_rq	Acu_conn_rq_ident_denied_rq

#define Acu_conn_rq_call_appear			Acq_ call_appear

#define	Acu_conn_rq_called_nb_type		Acq_ called_nb.type
#define	Acu_conn_rq_called_nb_plan		Acq_ called_nb.plan
#define Acu_conn_rq_called_sub_add_type	Acq_ called_nb.sa_type
#define Acu_conn_rq_called_sub_add_format Acq_ called_nb.sa_format
#define Acu_conn_rq_called_sub_add_odd	Acq_ called_nb.sa_odd
#define	Acu_conn_rq_called_nb_size		Acq_ called_nb.size
#define	Acu_conn_rq_called_nb_ie		Acq_ called_nb.digits_ie
#define	Acu_conn_rq_ph_num_size			Acu_conn_rq_called_nb_size

#define	Acu_conn_rq_calling_nb_type		Acq_ calling_nb.type
#define	Acu_conn_rq_calling_nb_plan		Acq_ calling_nb.plan
#define	Acu_conn_rq_calling_nb_pres		Acq_ calling_nb.pres
#define	Acu_conn_rq_calling_nb_screen	Acq_ calling_nb.screen
#define	Acu_conn_rq_calling_nb_reason	Acq_ calling_nb.reason
#define Acu_conn_rq_calling_sub_add_type Acq_ calling_nb.sa_type
#define Acu_conn_rq_calling_sub_add_format Acq_ calling_nb.sa_format
#define Acu_conn_rq_calling_sub_add_odd	Acq_ calling_nb.sa_odd
#define	Acu_conn_rq_calling_nb_size		Acq_ calling_nb.size

#define	Acu_conn_rq_calling_nb2_type	Acq_ calling_nb2.type
#define	Acu_conn_rq_calling_nb2_plan	Acq_ calling_nb2.plan
#define	Acu_conn_rq_calling_nb2_pres	Acq_ calling_nb2.pres
#define	Acu_conn_rq_calling_nb2_screen	Acq_ calling_nb2.screen
#define Acu_conn_rq_calling_sub_add2_type Acq_ calling_nb2.sa_type
#define Acu_conn_rq_calling_sub_add2_format Acq_ calling_nb2.sa_format
#define Acu_conn_rq_calling_sub_add2_odd  Acq_ calling_nb2.sa_odd
#define	Acu_conn_rq_calling_nb2_size	Acq_ calling_nb2.size

#define	Acu_conn_rq_redir_nb_pres		Acq_ redir_nb.pres
#define	Acu_conn_rq_redir_nb_type		Acq_ redir_nb.type
#define	Acu_conn_rq_redir_nb_plan		Acq_ redir_nb.plan
#define	Acu_conn_rq_redir_nb_screen		Acq_ redir_nb.screen
#define	Acu_conn_rq_redir_nb_reason		Acq_ redir_nb.reason
#define	Acu_conn_rq_redir_nb_size		Acq_ redir_nb.size

#define	Acu_conn_rq_uui_protocol		Acq_ uui_protocol
#define	Acu_conn_rq_uui_size			Acq_ uui_size
#define	Acu_conn_rq_facility_size		Acq_ facility_size
#define	Acu_conn_rq_display_size		Acu_conn_rq_x_display_total_size
#define Acu_conn_rq_pcs_user_size		Acq_ pcs_user_size
#define	Acu_conn_rq_notif_ind_ext_size	Acq_ notif_ind_ext_size

#define Acu_conn_rq_x_display_nb		Acq_ display.nb
#define Acu_conn_rq_x_display_total_size Acq_ display.total_size
#define Acu_conn_rq_x_display_offset(i) Acq_ display.element[i].string_offset
#define Acu_conn_rq_x_p_display(i) 		Acq_ display.element[i].p_display
#define Acu_conn_rq_x_display_type(i)	Acq_ display.element[i].type
#define Acu_conn_rq_x_display_size(i)	Acq_ display.element[i].size
#define Acu_conn_rq_a_display_list      &(Acq_ display)

#define	Acu_conn_rq_a_called_nb			((char FAR *)p_data        + Acu_conn_rq_size)
#define	Acu_conn_rq_a_ph_num			Acu_conn_rq_a_called_nb

#define	Acu_conn_rq_a_calling_nb		(Acu_conn_rq_a_called_nb   + Acu_conn_rq_called_nb_size + 1)
#define	Acu_conn_rq_a_calling_nb2		(Acu_conn_rq_a_calling_nb  + Acu_conn_rq_calling_nb_size + 1)
#define	Acu_conn_rq_a_redir_nb			(Acu_conn_rq_a_calling_nb2 + Acu_conn_rq_calling_nb2_size + 1)
#define	Acu_conn_rq_a_uui				(Acu_conn_rq_a_redir_nb    + Acu_conn_rq_redir_nb_size + 1)
#define	Acu_conn_rq_a_facility			(Acu_conn_rq_a_uui         + Acu_conn_rq_uui_size + 1)
#define	Acu_conn_rq_a_display			(Acu_conn_rq_a_facility    + Acu_conn_rq_facility_size + 1)
#define Acu_conn_rq_a_pcs_user			(Acu_conn_rq_a_display	   + Acu_conn_rq_display_size + 1)
#define	Acu_conn_rq_a_notif_ind_ext		(Acu_conn_rq_a_pcs_user    + Acu_conn_rq_pcs_user_size + 1)

#define	Acu_conn_rq_progress_ind_nb		Acq_ progress_ind.nb
#define	Acu_conn_rq_progress_location	Acu_conn_rq_progress_loc
#define	Acu_conn_rq_progress_description Acu_conn_rq_progress_descr
	/* aliases */
#define	Acu_conn_rq_progress_loc		Acq_ progress_ind.t[0].loc
#define	Acu_conn_rq_progress_descr		Acq_ progress_ind.t[0].descr
	/* access to other occurencies */
#define	Acu_conn_rq_progress_loc_x(i)	Acq_ progress_ind.t[i].loc
#define	Acu_conn_rq_progress_descr_x(i)	Acq_ progress_ind.t[i].descr

#define	Acu_conn_rq_notif_ind_present	Acq_ notif_ind_present
#define	Acu_conn_rq_notif_ind			Acq_ notif_ind

#define Acu_conn_rq_mlpp_present		Acq_ mlpp_present
#define Acu_conn_rq_mlpp_prec_level		Acq_ mlpp_prec_level
#define Acu_conn_rq_mlpp_lfb_ind		Acq_ mlpp_lfb_ind
#define Acu_conn_rq_mlpp_domain_size	&(Acq_ mlpp_domain_size)
#define Acu_conn_rq_a_mlpp_domain		&(Acq_ mlpp_domain)

#define	Acu_conn_rq_transit_counter		Acq_ transit_counter

#if ARINC_NETWORKS == ON
#	define	Acu_conn_rq_notif_descr			Acq_ notif_ind
#	define	Acu_conn_rq_spf_fac_id			Acq_ spf_fac_id
#	define	Acu_conn_rq_spf_fac_val			Acq_ spf_fac_val
#	define	Acu_conn_rq_departure_year		Acq_ departure_year
#	define	Acu_conn_rq_departure_month		Acq_ departure_month
#	define	Acu_conn_rq_departure_day		Acq_ departure_day
#	define	Acu_conn_rq_service_class_id	Acq_ service_class_id
#	define	Acu_conn_rq_nats_route_ind 		Acq_ nats_route_ind
#	define	Acu_conn_rq_satcom_route_ind 	Acq_ satcom_route_ind
#	define	Acu_conn_rq_tfts_route_ind 		Acq_ tfts_route_ind

#	define	Acu_conn_rq_track_1_size		Acq_ track_1_size
#	define	Acu_conn_rq_track_2_size		Acq_ track_2_size
#	define	Acu_conn_rq_flight_op_id_size	Acq_ flight_op_id_size
#	define	Acu_conn_rq_flight_number_size	Acq_ flight_number_size

#	define	Acu_conn_rq_a_track_1			(Acu_conn_rq_a_notif_ind_ext+ Acu_conn_rq_notif_ind_ext_size + 1)
#	define	Acu_conn_rq_a_track_2			(Acu_conn_rq_a_track_1		+ Acu_conn_rq_track_1_size + 1)
#	define	Acu_conn_rq_a_flight_op_id		(Acu_conn_rq_a_track_2		+ Acu_conn_rq_track_2_size + 1)
#	define	Acu_conn_rq_a_flight_number		(Acu_conn_rq_a_flight_op_id + Acu_conn_rq_flight_op_id_size + 1)
#	define	Acu_conn_rq_a_first_other		(Acu_conn_rq_a_flight_number + Acu_conn_rq_flight_number_size + 1)
#else
#	define	Acu_conn_rq_a_first_other		(Acu_conn_rq_a_notif_ind_ext + Acu_conn_rq_notif_ind_ext_size + 1)
#endif

#define Acu_conn_rq_a_end_strings			(uchar FAR *)Round((ulong)Acu_conn_rq_a_first_other)

#if _CC_SPID_INFO == ON
#	define	Acu_conn_rq_p_endpoint_id		Acq_ endpoint_id
#	define	Acu_conn_rq_endpoint_action		Acq_ endpoint_id.action
#	define	Acu_conn_rq_usid				Acq_ endpoint_id.usid
#	define	Acu_conn_rq_tid					Acq_ endpoint_id.tid
#	define	Acu_conn_rq_interpreter			Acq_ endpoint_id.interpreter
#endif

#define	Acu_conn_rq_size					(Rnd_sizeof (struct acu_conn_rq_args))

	/*--------------------------
	 *
	 *            =========================================
	 *             Specifying multiple 'called_nb' fields:
	 *            =========================================
	 *
	 * When calling with multiple B-channels, e.g. aggregated channels,
	 * the user might have to specify multiple called numbers.
	 * The first called number is stored using the
	 * 'Acu_conn_rq_called_nb_size' and 'Acu_conn_rq_a_called_nb' macros,
	 * as for a single-called-nb call.
	 * The additional 'called-nb' strings must be stored AFTER having stored
	 * the last variable-length field (i.e. 'facility', or 'flight_number'
	 * for ARINC networks).
	 * The format for each additional 'called_nb' is:
	 *     - one byte for the exact size of the string,
	 *     - immediately followed by the string itself.
	 *
	 * The additional called-numbers will appear in the data area as follows:
	 *
	 * |-----------|
	 * |-----------| (fixed-length fields)
	 * |-----------|
	 * |-----------|
	 * |           |
	 * |           | (variable-length fields, e.g. 'called_nb', 'calling_nb',
	 * |           | etc...)
	 * |           |
	 * |-----------|
	 * |   last    |
	 * | variable  | ('facility', or 'flight_number' for ARINC networks).
	 * |  length   |
	 * |  field    |
	 * |-----------|
	 * |  size #1  | (size of first additional called-nb string)
	 * |-----------|
	 * |           |
	 * |  called   | (digits, ended with a '\0' or not)
	 * |    nb     |
	 * |    #1     |
	 * |           |
	 * |-----------|
	 * |  size #2  | (size of second additional called-nb string)
	 * |-----------|
	 * |  called   |
	 * |    nb     | (digits, ended with a '\0' or not)
	 * |    #2     |
	 * |-----------|
	 * |     0     | (indicates the end of the called-nb strings list)
	 * -------------
	 *
	 *
	 * The address of the 'size #1' field is given by the
	 * 'Acu_conn_rq_a_first_other' macro.
	 *
	 * The 'size' fields are 1 byte long; their type is 'acu_size_t', their
	 * length is given by the 'ACU_SIZE_FIELDS_LENGTH' macro.
	 *
	 * If p points to an 'other' 'size' field, then one can use the following
	 * macro to determine the address of the next 'other' 'size' field:
	 *
	 *     Acu_a_next_other (p);
	 *
	 * ===============
	 * !!! WARNING !!!
	 * ===============
	 *
	 * If no 'other' called_nb is to be specified by the application, then
	 * the first 'size' field of the 'other' called-numbers MUST be set to 0.
	 *
	 *--------------------------*/

#define Acu_a_next_other(p_current_other) \
	p_current_other = (char FAR *)((char FAR *)(p_current_other) + ACU_SIZE_FIELD_LGTH + *(acu_size_t FAR *)(p_current_other))

#define Acu_conn_rq_other_called_nb_size(idx,size)          \
		{                                                   \
			char FAR *address;                              \
			Acu_conn_rq_a_other_called_nb (idx, address);   \
			size = *(address - ACU_SIZE_FIELD_LGTH);        \
		}

#define Acu_conn_rq_a_other_called_nb(idx,ret_address)      \
		{													\
			uchar i;                                        \
			ret_address = Acu_conn_rq_a_first_other;        \
			for (i = 1; i < idx; i++) {                     \
				Acu_a_next_other (ret_address);             \
			}                                               \
			ret_address += ACU_SIZE_FIELD_LGTH;             \
		}

/*---------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_CONN_IN		signal an incoming call.
 *
 *	Arguments :
 *	(*)	Acu_conn_in_service 		: service requested by the remote
 *	   	Acu_conn_in_service_list_id : service list id associated with the selected services list
 *		Acu_conn_in_ph_rate 		: physical rate (for all services)
 *		Acu_conn_in_user_rate		: user rate (for V.110 and V.120 services only)
 *		Acu_conn_in_syn_asyn  		: synchronous/asynchronous (for V.110 and V.120 services only)
 *		Acu_conn_in_stop_bits   	: nb of stop bits (for V.110 and V.120 services only)
 *		Acu_conn_in_data_bits   	: nb of data bits (for V.110 and V.120 services only)
 *		Acu_conn_in_parity      	: parity (for V.110 and V.120 services only)
 *	(*)	Acu_conn_in_interworking	: interworking indication (ON/OFF)                                  (Analog ACU: ON)
 *		Acu_conn_in_sending_complete: Sending-complete IE received (ON/OFF)
 *		Acu_conn_in_ces				: DL-Connection identifier for that call
 *      Acu_conn_in_data_chani_nb   : nb of elements in the chani_list field
 *     	Acu_conn_in_data_chani      : data channel to use (B1, B2, etc, D)
 *		Acu_conn_in_calling_nb_type	: calling nb type (ACU_NB_TYPE_*)
 *		Acu_conn_in_calling_nb_plan	: calling nb plan (ACU_NB_PLAN_*)
 *		Acu_conn_in_calling_nb_pres	: calling nb presentation
 *		Acu_conn_in_calling_nb_screen	: calling nb screening indicator (ACU_NB_SCREEN_*)
 *		Acu_conn_in_calling_nb_size	: calling number size in bytes
 *		Acu_conn_in_a_calling_nb	: address of the calling number.
 *		Acu_conn_in_calling_nb2_type: 2nd calling nb type (ACU_NB_TYPE_*)
 *		Acu_conn_in_calling_nb2_plan: 2nd calling nb plan (ACU_NB_PLAN_*)
 *		Acu_conn_in_calling_nb2_pres: 2nd calling nb presentation
 *		Acu_conn_in_calling_nb2_screen	: 2nd calling nb screening indicator
 *		Acu_conn_in_calling_nb2_size: 2nd calling number size in bytes
 *		Acu_conn_in_a_calling_nb2	: address of the 2nd calling number.
 *	   	Acu_conn_in_ph_num_size		: alias for Acu_conn_in_calling_nb_size
 *		Acu_conn_in_a_ph_num 		: alias for Acu_conn_in_a_calling_nb
 *		Acu_conn_in_called_nb_type	: called nb type (ACU_NB_TYPE_*)
 *		Acu_conn_in_called_nb_plan	: called nb plan
 *	   	Acu_conn_in_called_nb_size	: called number*sub-address size in bytes
 *		Acu_conn_in_a_called_nb 	: address of the called number*sub-address.
 *	   	Acu_conn_in_uui_size,
 *		Acu_conn_in_a_uui			: ISDN user to User Information
 *		Acu_conn_in_facility_size,
 *		Acu_conn_in_a_facility		: ISDN facility
 *		Acu_conn_in_display_size,
 *		Acu_conn_in_a_display		: ISDN display Information
 *		Acu_conn_in_pcs_user_size,
 *		Acu_conn_in_a_pcs_user		: ISDN PCS-to-User/User-to-PCS Info.
 *		Acu_conn_in_dest_call_appear: destination call_appearance
 *		Acu_conn_in_progress_location		: progress description
 *		Acu_conn_in_progress_description	: progress location
 *		Acu_conn_in_notif_ind				: notification indicator description
 *		Acu_conn_in_notif_descr				: (alias)notification indicator description
 *		Acu_conn_in_mlpp_present	; Q.955 MLPP information fields present ON/OFF; when ON, the following MLPP fields are meaningful
 *		Acu_conn_in_mlpp_prec_level	: Q.955 MLPP Precedence Level: 0:flash-override(highest) .. 4:Routine(lowest)
 *		Acu_conn_in_mlpp_lfb_ind	: Q.955 MLPP LFB Indication:   0:allowed, 1:not-allowed, 2:path-reserved
 *		Acu_conn_in_mlpp_domain_size: Q.955 MLPP Service Domain: size of the Service Domain string (max 5)
 *		Acu_conn_in_a_mlpp_domain	: Q.955 MLPP Service Domain: 5-octets value (non-zero-terminated)
 *		Acu_conn_in_transit_counter	: Transit Counter (QSIG only)
 *
 *		ARINC SPECIFIC :
 *
 *		Acu_conn_in_spf_fac_id				: spf facility Id
 *		Acu_conn_in_spf_fac_val				: spf_facility value
 *		Acu_conn_in_departure_year			: year of departure
 *		Acu_conn_in_departure_month			: month of departure
 *		Acu_conn_in_departure_day			: day of departure
 *		Acu_conn_in_service_class_id		: service class Id
 *		Acu_conn_in_nats_route_ind			: nats route
 *		Acu_conn_in_satcom_route_ind		: stacom route
 *		Acu_conn_in_tfts_route_ind			: tfts route
 *
 *		Acu_conn_in_track_1_size			: size of the track #1
 *		Acu_conn_in_track_2_size            : size of the track #2
 *		Acu_conn_in_flight_op_id_size		: size of the fligth op Id
 *		Acu_conn_in_flight_number_size		: size of the flight digits
 *
 *		Acu_conn_in_a_track_1				: address of the track #1
 *		Acu_conn_in_a_track_2               : address of the track #2
 *		Acu_conn_in_a_flight_op_id			: address of op Id
 *		Acu_conn_in_a_flight_number			: address of flight digits
 *
 *      ACU_PSTN_CLIP == ON :
 *
 *  (*) Acu_conn_in_a_raw_msg               : address of CLIP raw data
 *  (*) Acu_conn_in_raw_msg_size            : size of CLIP raw data
 *
 *  (*) Acu_conn_in_a_clip_msg              : Get ptr for Acu_clip__xxx macros
 *
 *  (*) Acu_clip__raw_msg_size(p)           : see sub-structure acu_clip__message
 *  (*) Acu_clip__msg_name(p)
 *  (*) Acu_clip__date_time_size(p)
 *  (*) Acu_clip__date_time_off(p)
 *  (*) Acu_clip__a_date_time(p)
 *  (*) .....
 *
 *
 *		Warning : when building this primitive, the phone number field must
 *		be filled BEFORE the UUI field.
 *
 *  (*) Only these fields are initialized by the Analog ACU, other
 *		fields are set to 0.
 *
 *--------------------------------------------------------------------------*/

struct acu_conn_in_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	service;					/* (tele)service requested by the remote */
	uchar	service_list_id;			/* service list id associated with the selected services list */
	struct layer_1_info layer_1_info;	/* layer 1 information field */
	uchar	interworking;				/* interworking indication (ON/OFF) */
	uchar	sending_complete;			/* Sending-complete information element reception */
	add_t	ces;						/* DL-Connection identifier for that call */
	struct chani_list chani_list;		/* list of channel ids to use for data transfer */
	struct acu_calling_nb calling_nb;	/* calling nb information */
	struct acu_calling_nb calling_nb2;	/* second calling nb information */
	struct acu_called_nb  called_nb;	/* called nb information */
	struct acu_redir_nb   redir_nb;		/* redirecting nb information */

	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* size of the ISDN User to User information field */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

	struct acu_progress_ind progress_ind;/* progress indicator */

	uchar		notif_ind_present;		/* notification indicator present (ON/OFF) */
	uchar		notif_ind;				/* notification indicator description */
	uchar		notif_ind_ext_size;		/* notification-indicator extension (QSIG) */

	uchar		mlpp_present;			/* Q.955 MLPP information fields present ON/OFF; when ON, the following MLPP fields are meaningful */
	uchar		mlpp_prec_level;		/* Q.955 MLPP Precedence Level: 0:flash-override(highest) .. 4:Routine(lowest) */
	uchar		mlpp_lfb_ind;			/* Q.955 MLPP LFB Indication:   0:allowed, 1:not-allowed, 2:path-reserved */
	uchar		mlpp_domain_size;		/* Q.955 MLPP Service Domain: size of the Service Domain string (max 5) */
	uchar		mlpp_domain[5];			/* Q.955 MLPP Service Domain: 5-octets value (any binary, non-zero-terminated) */

	uchar		transit_counter;		/* Transit counter (QSIG only) */

#	if ARINC_NETWORKS == ON

#		if CC_SPF_FAC == ON
			uchar	spf_fac_id;			/* facility identifier */
			uchar	spf_fac_val;		/* facility value */
#		endif

		uchar	departure_year;			/* departure year */
		uchar	departure_month;    	/* departure  month */
		uchar	departure_day;      	/* departure day */
		uchar	service_class_id;		/* service class identifier defined by each airline */
		uchar 	nats_route_ind;			/* nats route indication */
		uchar 	satcom_route_ind;		/* satcom route indication */
		uchar 	tfts_route_ind;			/* tfts route indication */

		acu_size_t track_1_size;		/* information of track 1 */
		acu_size_t track_2_size;		/* information of track 2 */

		acu_size_t flight_op_id_size;	/* flight operator identity size */
		acu_size_t flight_number_size;	/* flight number size */
#	endif

	uchar	dest_call_appear;			/* Destination call appearence */
	uchar	semi_permanent_circuit;		/* semi-permanent circuit indication (ON/OFF) */
										/* phone number is just here */
										/* then the UUI */
										/* then facility */
										/* then display strings */
										/* then pcs_user */
										/* then track 1 info */
										/* then track 2 info */
										/* then flight operator id */
										/* then flight number */

#   if ACU_PSTN_CLIP == ON
        struct acu_clip__message clip;  /* Calling Line Identification Presentation (Caller ID) */
#   endif

};

#define	Acu_conn_in_					((struct acu_conn_in_args FAR *)p_data) ->
#define	Aci_							Acu_conn_in_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_conn_in_ext_parms_nb		Aci_ ext_descr.nb
#	define Acu_conn_in_ext_parms_lgth	Aci_ ext_descr.lgth
#	define Acu_conn_in_a_ext_parms		Aci_ ext_descr.address
#endif

#define	Acu_conn_in_service				Aci_ service
#define Acu_conn_in_service_list_id		Aci_ service_list_id

#define	Acu_conn_in_a_layer_1_info		((struct layer_1_info FAR *)&Aci_ layer_1_info)
#define	Acu_conn_in_ph_rate				Aci_ layer_1_info.ph_rate
#define	Acu_conn_in_user_rate			Aci_ layer_1_info.user_rate
#define	Acu_conn_in_syn_asyn			Aci_ layer_1_info.syn_asyn
#define	Acu_conn_in_stop_bits			Aci_ layer_1_info.stop_bits
#define	Acu_conn_in_data_bits			Aci_ layer_1_info.data_bits
#define	Acu_conn_in_parity				Aci_ layer_1_info.parity
#define	Acu_conn_in_l1_protocol			Aci_ layer_1_info.l1_protocol

#define	Acu_conn_in_interworking		Aci_ interworking
#define	Acu_conn_in_sending_complete	Aci_ sending_complete
#define	Acu_conn_in_ces					Aci_ ces

#define Acu_conn_in_a_chani_list        ((struct chani_list FAR *)&Aci_ chani_list)
#define Acu_conn_in_data_chani_nb       Aci_ chani_list.nb
#define Acu_conn_in_data_chani          Aci_ chani_list.tab[0].chan_id
#define Acu_conn_in_data_chani_nai      Aci_ chani_list.tab[0].nai
#define Acu_conn_in_data_chani_tab(i)   Aci_ chani_list.tab[i].chan_id
#define Acu_conn_in_data_chani_tab_nai(i) Aci_ chani_list.tab[i].nai
#define Acu_conn_in_data_chani_excl     Aci_ chani_list.chan_id_excl

#define	Acu_conn_in_calling_nb_type		Aci_ calling_nb.type
#define	Acu_conn_in_calling_nb_plan		Aci_ calling_nb.plan
#define	Acu_conn_in_calling_nb_pres		Aci_ calling_nb.pres
#define	Acu_conn_in_calling_nb_screen	Aci_ calling_nb.screen
#define	Acu_conn_in_calling_nb_reason	Aci_ calling_nb.reason
#define Acu_conn_in_calling_sub_add_type Aci_ calling_nb.sa_type
#define Acu_conn_in_calling_sub_add_format Aci_ calling_nb.sa_format
#define Acu_conn_in_calling_sub_add_odd	Aci_ calling_nb.sa_odd
#define	Acu_conn_in_calling_nb_size		Aci_ calling_nb.size
#define	Acu_conn_in_ph_num_size			Acu_conn_in_calling_nb_size

#define	Acu_conn_in_calling_nb2_type	Aci_ calling_nb2.type
#define	Acu_conn_in_calling_nb2_plan	Aci_ calling_nb2.plan
#define	Acu_conn_in_calling_nb2_pres	Aci_ calling_nb2.pres
#define	Acu_conn_in_calling_nb2_screen	Aci_ calling_nb2.screen
#define	Acu_conn_in_calling_nb2_reason	Aci_ calling_nb2.reason
#define	Acu_conn_in_calling_nb2_size	Aci_ calling_nb2.size

#define	Acu_conn_in_called_nb_type		Aci_ called_nb.type
#define	Acu_conn_in_called_nb_plan		Aci_ called_nb.plan
#define Acu_conn_in_called_sub_add_type	Aci_ called_nb.sa_type
#define Acu_conn_in_called_sub_add_format Aci_ called_nb.sa_format
#define Acu_conn_in_called_sub_add_odd	Aci_ called_nb.sa_odd
#define	Acu_conn_in_called_nb_size		Aci_ called_nb.size
#define	Acu_conn_in_called_nb_ie		Aci_ called_nb.digits_ie

#define	Acu_conn_in_redir_nb_pres		Aci_ redir_nb.pres
#define	Acu_conn_in_redir_nb_type		Aci_ redir_nb.type
#define	Acu_conn_in_redir_nb_plan		Aci_ redir_nb.plan
#define	Acu_conn_in_redir_nb_screen		Aci_ redir_nb.screen
#define	Acu_conn_in_redir_nb_reason		Aci_ redir_nb.reason
#define	Acu_conn_in_redir_nb_size		Aci_ redir_nb.size

#define	Acu_conn_in_uui_protocol		Aci_ uui_protocol
#define	Acu_conn_in_uui_size			Aci_ uui_size
#define	Acu_conn_in_facility_size		Aci_ facility_size
#define	Acu_conn_in_display_size		Acu_conn_in_x_display_total_size
#define Acu_conn_in_pcs_user_size		Aci_ pcs_user_size
#define	Acu_conn_in_notif_ind_ext_size	Aci_ notif_ind_ext_size

#define Acu_conn_in_x_display_nb		Aci_ display.nb
#define Acu_conn_in_x_display_total_size Aci_ display.total_size
#define Acu_conn_in_x_display_offset(i) Aci_ display.element[i].string_offset
#define Acu_conn_in_x_p_display(i) 		Aci_ display.element[i].p_display
#define Acu_conn_in_x_display_type(i)	Aci_ display.element[i].type
#define Acu_conn_in_x_display_size(i)	Aci_ display.element[i].size
#define Acu_conn_in_a_display_list      &(Aci_ display)

#define	Acu_conn_in_a_calling_nb		((char FAR *)p_data       + Acu_conn_in_size)
#define	Acu_conn_in_a_ph_num			Acu_conn_in_a_calling_nb
#define	Acu_conn_in_a_calling_nb2		(Acu_conn_in_a_calling_nb + Acu_conn_in_calling_nb_size)
#define	Acu_conn_in_a_called_nb			(Acu_conn_in_a_calling_nb2+ Acu_conn_in_calling_nb2_size)
#define	Acu_conn_in_a_redir_nb			(Acu_conn_in_a_called_nb  + Acu_conn_in_called_nb_size)
#define	Acu_conn_in_a_uui				(Acu_conn_in_a_redir_nb   + Acu_conn_in_redir_nb_size)
#define	Acu_conn_in_a_facility			(Acu_conn_in_a_uui        + Acu_conn_in_uui_size)
#define	Acu_conn_in_a_display			(Acu_conn_in_a_facility   + Acu_conn_in_facility_size)
#define Acu_conn_in_a_pcs_user			(Acu_conn_in_a_display    + Acu_conn_in_display_size)
#define	Acu_conn_in_a_notif_ind_ext		(Acu_conn_in_a_pcs_user   + Acu_conn_in_pcs_user_size + 1)

#define	Acu_conn_in_progress_ind_nb		Aci_ progress_ind.nb
#define	Acu_conn_in_progress_location	Acu_conn_in_progress_loc
#define	Acu_conn_in_progress_description Acu_conn_in_progress_descr
	/* aliases */
#define	Acu_conn_in_progress_loc		Aci_ progress_ind.t[0].loc
#define	Acu_conn_in_progress_descr		Aci_ progress_ind.t[0].descr
	/* access to other occurencies */
#define	Acu_conn_in_progress_loc_x(i)	Aci_ progress_ind.t[i].loc
#define	Acu_conn_in_progress_descr_x(i)	Aci_ progress_ind.t[i].descr

#define	Acu_conn_in_notif_ind_present	Aci_ notif_ind_present
#define	Acu_conn_in_notif_ind			Aci_ notif_ind

#define Acu_conn_in_mlpp_present		Aci_ mlpp_present
#define Acu_conn_in_mlpp_prec_level		Aci_ mlpp_prec_level
#define Acu_conn_in_mlpp_lfb_ind		Aci_ mlpp_lfb_ind
#define Acu_conn_in_mlpp_domain_size	Aci_ mlpp_domain_size
#define Acu_conn_in_a_mlpp_domain		&(Aci_ mlpp_domain)

#define	Acu_conn_in_transit_counter		Aci_ transit_counter

#if ARINC_NETWORKS == ON
#	define	Acu_conn_in_notif_descr			Aci_ notif_ind
#	define	Acu_conn_in_spf_fac_id			Aci_ spf_fac_id
#	define	Acu_conn_in_spf_fac_val			Aci_ spf_fac_val
#	define	Acu_conn_in_departure_year		Aci_ departure_year
#	define	Acu_conn_in_departure_month		Aci_ departure_month
#	define	Acu_conn_in_departure_day		Aci_ departure_day
#	define	Acu_conn_in_service_class_id	Aci_ service_class_id
#	define	Acu_conn_in_nats_route_ind 		Aci_ nats_route_ind
#	define	Acu_conn_in_satcom_route_ind 	Aci_ satcom_route_ind
#	define	Acu_conn_in_tfts_route_ind 		Aci_ tfts_route_ind

#	define	Acu_conn_in_track_1_size		Aci_ track_1_size
#	define	Acu_conn_in_track_2_size		Aci_ track_2_size
#	define	Acu_conn_in_flight_op_id_size	Aci_ flight_op_id_size
#	define	Acu_conn_in_flight_number_size	Aci_ flight_number_size

#	define	Acu_conn_in_a_track_1			(Acu_conn_in_a_notif_ind_ext+ Acu_conn_in_notif_ind_ext_size)
#	define	Acu_conn_in_a_track_2			(Acu_conn_in_a_track_1		+ Acu_conn_in_track_1_size)
#	define	Acu_conn_in_a_flight_op_id		(Acu_conn_in_a_track_2		+ Acu_conn_in_track_2_size)
#	define	Acu_conn_in_a_flight_number		(Acu_conn_in_a_flight_op_id + Acu_conn_in_flight_op_id_size)
#endif

#define Acu_conn_in_dest_call_appear		Aci_ dest_call_appear
#define Acu_conn_in_semi_permanent_circuit	Aci_ semi_permanent_circuit

#if ACU_PSTN_CLIP == ON

#   if ARINC_NETWORKS == ON
#	    define Acu_conn_in_a_raw_msg            (Acu_conn_in_a_flight_number + Acu_conn_in_flight_number_size)
#   else
#	    define Acu_conn_in_a_raw_msg            (Acu_conn_in_a_notif_ind_ext + Acu_conn_in_notif_ind_ext_size)
#   endif
#   define	Acu_conn_in_raw_msg_size            Aci_ clip.raw_msg_size

#   define	Acu_conn_in_a_clip_msg              &(Aci_ clip)                    /* Get ptr for Acu_clip__xxx macros */

#   define	Acu_clip__raw_msg_size(p)           (p)-> raw_msg_size

#   define	Acu_clip__msg_name(p)               (p)-> msg_name

#   define	Acu_clip__date_time_size(p)         (p)-> date_time_size
#   define	Acu_clip__date_time_off(p)          (p)-> date_time_off
#   define	Acu_clip__a_date_time(p)            (Acu_conn_in_a_raw_msg + Acu_clip__date_time_off (p))

#   define	Acu_clip__cli_size(p)               (p)-> cli_size
#   define	Acu_clip__cli_off(p)                (p)-> cli_off
#   define	Acu_clip__a_cli(p)                  (Acu_conn_in_a_raw_msg + Acu_clip__cli_off (p))

#   define	Acu_clip__reason_no_cli_size(p)     (p)-> reason_no_cli_size
#   define	Acu_clip__reason_no_cli_off(p)      (p)-> reason_no_cli_off
#   define	Acu_clip__a_reason_no_cli(p)        (Acu_conn_in_a_raw_msg + Acu_clip__reason_no_cli_off (p))
#   define	Acu_clip__reason_no_cli(p)          *(Acu_clip__a_reason_no_cli (p))

#   define	Acu_clip__called_li_size(p)         (p)-> called_li_size
#   define	Acu_clip__called_li_off(p)          (p)-> called_li_off
#   define	Acu_clip__a_called_li(p)            (Acu_conn_in_a_raw_msg + Acu_clip__called_li_off (p))

#   define	Acu_clip__cpn_size(p)               (p)-> cpn_size
#   define	Acu_clip__cpn_off(p)                (p)-> cpn_off
#   define	Acu_clip__a_cpn(p)                  (Acu_conn_in_a_raw_msg + Acu_clip__cpn_off (p))

#   define	Acu_clip__reason_no_cpn_size(p)     (p)-> reason_no_cpn_size
#   define	Acu_clip__reason_no_cpn_off(p)      (p)-> reason_no_cpn_off
#   define	Acu_clip__a_reason_no_cpn(p)        (Acu_conn_in_a_raw_msg + Acu_clip__reason_no_cpn_off (p))
#   define	Acu_clip__reason_no_cpn(p)          *(Acu_clip__a_reason_no_cpn (p))

#   define	Acu_clip__complementary_cli_size(p) (p)-> complementary_cli_size
#   define	Acu_clip__complementary_cli_off(p)  (p)-> complementary_cli_off
#   define	Acu_clip__a_complementary_cli(p)    (Acu_conn_in_a_raw_msg + Acu_clip__complementary_cli_off (p))

#   define	Acu_clip__call_type_size(p)         (p)-> call_type_size
#   define	Acu_clip__call_type_off(p)          (p)-> call_type_off
#   define	Acu_clip__a_call_type(p)            (Acu_conn_in_a_raw_msg + Acu_clip__call_type_off (p))
#   define	Acu_clip__call_type(p)              *(Acu_clip__a_call_type (p))

#   define	Acu_clip__first_called_li_size(p)   (p)-> first_called_li_size
#   define	Acu_clip__first_called_li_off(p)    (p)-> first_called_li_off
#   define	Acu_clip__a_first_called_li(p)      (Acu_conn_in_a_raw_msg + Acu_clip__first_called_li_off (p))

#   define	Acu_clip__net_msg_sys_sts_size(p)   (p)-> net_msg_sys_sts_size
#   define	Acu_clip__net_msg_sys_sts_off(p)    (p)-> net_msg_sys_sts_off
#   define	Acu_clip__a_net_msg_sys_sts(p)      (Acu_conn_in_a_raw_msg + Acu_clip__net_msg_sys_sts_off (p))
#   define	Acu_clip__net_msg_sys_sts(p)        *(Acu_clip__a_net_msg_sys_sts (p))

#   define	Acu_clip__t_frw_call_size(p)        (p)-> t_frw_call_size
#   define	Acu_clip__t_frw_call_off(p)         (p)-> t_frw_call_off
#   define	Acu_clip__a_t_frw_call(p)           (Acu_conn_in_a_raw_msg + Acu_clip__t_frw_call_off (p))
#   define	Acu_clip__t_frw_call(p)             *(Acu_clip__a_t_frw_call (p))

#   define	Acu_clip__t_calling_user_size(p)    (p)-> t_calling_user_size
#   define	Acu_clip__t_calling_user_off(p)     (p)-> t_calling_user_off
#   define	Acu_clip__a_t_calling_user(p)       (Acu_conn_in_a_raw_msg + Acu_clip__t_calling_user_off (p))
#   define	Acu_clip__t_calling_user(p)         *(Acu_clip__a_t_calling_user (p))

#   define	Acu_clip__redirecting_nb_size(p)    (p)-> redirecting_nb_size
#   define	Acu_clip__redirecting_nb_off(p)     (p)-> redirecting_nb_off
#   define	Acu_clip__a_redirecting_nb(p)       (Acu_conn_in_a_raw_msg + Acu_clip__redirecting_nb_off (p))

#   define	Acu_clip__ext_for_op_use_size(p)    (p)-> ext_for_op_use_size
#   define	Acu_clip__ext_for_op_use_off(p)     (p)-> ext_for_op_use_off
#   define	Acu_clip__a_ext_for_op_use(p)       (Acu_conn_in_a_raw_msg + Acu_clip__ext_for_op_use_off (p))

#   define	Acu_clip__visual_in_size(p)         (p)-> visual_in_size
#   define	Acu_clip__visual_in_off(p)          (p)-> visual_in_off
#   define	Acu_clip__a_visual_in(p)            (Acu_conn_in_a_raw_msg + Acu_clip__visual_in_off (p))
#   define	Acu_clip__visual_in(p)              *(Acu_clip__a_visual_in (p))

#   define	Acu_clip__chk_alignment(p)          (p)-> chk_alignment

#endif /* ACU_PSTN_CLIP == ON */
#if ACU_PSTN_CLIP == OFF
#   if ARINC_NETWORKS == ON
#	    define	Acu_conn_in_a_end_strings		(uchar FAR *)Round((ulong)Acu_conn_in_a_flight_number + Acu_conn_in_flight_number_size)
#   else
#	    define	Acu_conn_in_a_end_strings		(uchar FAR *)Round((ulong)Acu_conn_in_a_pcs_user + Acu_conn_in_pcs_user_size)
#   endif
#else
#   define	Acu_conn_in_a_end_strings		    (uchar FAR *)Round((ulong)Acu_conn_in_a_raw_msg + Acu_conn_in_raw_msg_size)
#endif
#define	Acu_conn_in_size					(Rnd_sizeof (struct acu_conn_in_args))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_SETUP_REPORT_IN		signals an incompatible incoming call.
 *
 *	WARNING : this message is generated when an incompatible incoming call
 *			  has been received. It uses the ACU_SAPI_MGT sapi. Its structure
 *			  is the same as an ACU_CONN_IN. It is used as an information
 *			  report only, no action should be taken in order to answer the
 *			  call. It is only generated in an ISDN context.
 *
 *	Arguments : use the Acu_conn_in_... macros.
 *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_CONN_RS			request answering to an incoming call.
 *
 *	Arguments :
 *	(*)	Acu_conn_rs_priority		: answer priority (ACU_PHIGH, ACU_PLOW)
 *	(*)	Acu_conn_rs_service			: requested (tele)service
 *  (*)	Acu_conn_rs_ph_rate			: physical rate (for all services)
 *									  - for fax service using ACU: set to either
 *									    ACU_RA_56000 (use V.8) or 0 (do not use V.8).
 *  (*) Acu_conn_rs_l1_protocol     : ISDN ACU  : user information layer 1 protocol
 *                                    Analog ACU: A Law / Mu Law selection if ISDN interworking connection is established
 *                                                (ACU_L1_PROTOCOL_MU_LAW / ACU_L1_PROTOCOL_A_LAW)
 *	(*)	Acu_conn_rs_no_v25 			: (Used only by Analog ACU)
 *                                    Disable V.25 procedure before data modulation (service == ACU_MODEM_SERVICE)
 *                                    -> Don't send answer tone (2100Hz)
 *                                    This field must be reset (OFF) with ITU modulation (V.21, V.22, ...)
 *                                    This field must be set (ON) with Bell modulation (Bell 103 or Bell 212a)
 *                                    Rem: this field is ignored if service == ACU_FAX_SERVICE
 *                                    Rem: this field is ignored if acu_config..ced_data_by_chip = ON
 *		Acu_conn_rs_user_rate		: user rate (for V.110 and V.120 services only)
 *		Acu_conn_rs_syn_asyn  		: synchronous/asynchronous (for V.110 and V.120 services only)
 *		Acu_conn_rs_stop_bits   	: nb of stop bits (for V.110 and V.120 services only)
 *		Acu_conn_rs_data_bits   	: nb of data bits (for V.110 and V.120 services only)
 *		Acu_conn_rs_parity      	: parity (for V.110 and V.120 services only)
 *      Acu_conn_rs_data_chani_nb   : nb of elements in the chani_list field
 *      Acu_conn_rs_data_chani  	: data channel to use (B1, B2, etc, D, or 0 for any)
 *		Acu_conn_rs_year			: year
 *		Acu_conn_rs_month;			: month
 *		Acu_conn_rs_day;			: day
 *		Acu_conn_rs_hour;			: hour
 *		Acu_conn_rs_minute;			: minute
 *		Acu_conn_rs_second;			: second
 *		Acu_conn_rs_conn_nb_type	: connected nb type (ACU_NB_TYPE_*)
 *		Acu_conn_rs_conn_nb_plan	: connected nb plan
 *		Acu_conn_rs_conn_nb_pres	: connected nb pres
 *		Acu_conn_rs_conn_nb_screen	: connected nb screening indicator (ACU_NB_SCREEN_*)
 *		Acu_conn_rs_conn_sub_add_type: connected sub-add type
 *		Acu_conn_rs_conn_sub_add_odd: connected sub-add odd/even indicator (0=even,1=odd)
 *
 *		Acu_conn_rs_uui_size,
 *		Acu_conn_rs_a_uui			: ISDN user to User Information
 *		Acu_conn_rs_facility_size,
 *		Acu_conn_rs_a_facility		: ISDN facility
 *		Acu_conn_rs_pcs_user_size,
 *		Acu_conn_rs_a_pcs_user		: ISDN PCS-to-User/User-to-PCS Info
 *		Acu_conn_rs_conn_nb_size,
 *		Acu_conn_rs_a_conn_nb		: Connected number string
 *		Acu_conn_rs_signal_val 		: signal value
 *
 *  (*) Only these fields are used by the Analog ACU, other fields are ignored by the Analog ACU.
 *--------------------------------------------------------------------------*/

struct acu_conn_rs_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	priority;					/* phone call priority */
	uchar	service;					/* requested service */

    uchar	no_v25;                     /* (Used only by Analog ACU) */
                                        /* ON:  Disable V.25 procedure before data modulation (service == ACU_MODEM_SERVICE) */
                                        /* OFF: Enable V.25 procedure */

	struct layer_1_info layer_1_info;	/* layer 1 information field */
	struct chani_list   chani_list;		/* list of channel ids to use for data transfer */
	struct acu_charging_info charging_info;	/* charging information */
	struct acu_date_time date_time;		/* date and time */

	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* size of the ISDN User to User information field */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

	uchar	signal_val;					/* signal value */
	struct acu_progress_ind progress_ind;/* progress indicator */
	uchar		notif_ind_present;		/* notification indicator present (ON/OFF) */
	uchar		notif_ind;				/* notification indicator description */
	uchar		notif_ind_ext_size;		/* notification-indicator extension (QSIG) */

	struct acu_conn_nb conn_nb;			/* connected nb/sub_add */

										/* UUI is just here */
										/* then the facility */
										/* then the display strings */
										/* then the pcs_user */
										/* and finally the connected-nb string */
};

#define	Acu_conn_rs_					((struct acu_conn_rs_args FAR *)p_data) ->
#define	Acs_							Acu_conn_rs_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_conn_rs_ext_parms_nb		Acs_ ext_descr.nb
#	define Acu_conn_rs_ext_parms_lgth	Acs_ ext_descr.lgth
#	define Acu_conn_rs_a_ext_parms		Acs_ ext_descr.address
#endif

#define	Acu_conn_rs_priority			Acs_ priority
#define	Acu_conn_rs_service				Acs_ service

#define	Acu_conn_rs_no_v25 		        Acs_ no_v25

#define	Acu_conn_rs_a_layer_1_info		((struct layer_1_info FAR *)&Acs_ layer_1_info)
#define	Acu_conn_rs_ph_rate				Acs_ layer_1_info.ph_rate
#define	Acu_conn_rs_user_rate			Acs_ layer_1_info.user_rate
#define	Acu_conn_rs_syn_asyn			Acs_ layer_1_info.syn_asyn
#define	Acu_conn_rs_stop_bits			Acs_ layer_1_info.stop_bits
#define	Acu_conn_rs_data_bits			Acs_ layer_1_info.data_bits
#define	Acu_conn_rs_parity				Acs_ layer_1_info.parity
#define	Acu_conn_rs_l1_protocol			Acs_ layer_1_info.l1_protocol
#define Acu_conn_rs_a_chani_list		((struct chani_list FAR *)&Acs_ chani_list)
#define Acu_conn_rs_data_chani_nb       Acs_ chani_list.nb
#define Acu_conn_rs_data_chani          Acs_ chani_list.tab[0].chan_id
#define Acu_conn_rs_data_chani_nai      Acs_ chani_list.tab[0].nai
#define Acu_conn_rs_data_chani_tab(i)   Acs_ chani_list.tab[i].chan_id
#define Acu_conn_rs_data_chani_tab_nai(i) Acs_ chani_list.tab[i].nai
#define Acu_conn_rs_data_chani_excl     Acs_ chani_list.chan_id_excl
#define	Acu_conn_rs_charging_available	Acs_ charging_info.available
#define Acu_conn_rs_charging_value     	Acs_ charging_info.value
#define Acu_conn_rs_charging_type     	Acs_ charging_info.type
#define Acu_conn_rs_charging_period    	Acs_ charging_info.period
#define Acu_conn_rs_charging_multi     	Acs_ charging_info.multi
#define Acu_conn_rs_charging_a_currency &(Acs_ charging_info.currency[0])
#define Acu_conn_rs_charging	     	Acu_conn_rs_charging_value	/* alias */
#define	Acu_conn_rs_date_available		Acs_ date_time.available
#define	Acu_conn_rs_year				Acs_ date_time.year
#define	Acu_conn_rs_month				Acs_ date_time.month
#define	Acu_conn_rs_day					Acs_ date_time.day
#define	Acu_conn_rs_hour				Acs_ date_time.hour
#define	Acu_conn_rs_minute				Acs_ date_time.minute
#define	Acu_conn_rs_second				Acs_ date_time.second
#define	Acu_conn_rs_a_date_time			&(Acs_ date_time)

#define	Acu_conn_rs_uui_protocol		Acs_ uui_protocol
#define	Acu_conn_rs_uui_size			Acs_ uui_size
#define	Acu_conn_rs_facility_size		Acs_ facility_size
#define	Acu_conn_rs_display_size		Acu_conn_rs_x_display_total_size
#define Acu_conn_rs_pcs_user_size		Acs_ pcs_user_size
#define	Acu_conn_rs_notif_ind_ext_size	Acs_ notif_ind_ext_size
#define Acu_conn_rs_conn_nb_size		Acs_ conn_nb.size

#define Acu_conn_rs_x_display_nb		Acs_ display.nb
#define Acu_conn_rs_x_display_total_size Acs_ display.total_size
#define Acu_conn_rs_x_display_offset(i) Acs_ display.element[i].string_offset
#define Acu_conn_rs_x_p_display(i) 		Acs_ display.element[i].p_display
#define Acu_conn_rs_x_display_type(i)	Acs_ display.element[i].type
#define Acu_conn_rs_x_display_size(i)	Acs_ display.element[i].size
#define Acu_conn_rs_a_display_list      &(Acs_ display)

#define	Acu_conn_rs_a_uui				((char FAR *)p_data + Acu_conn_rs_size)
#define	Acu_conn_rs_a_facility			(Acu_conn_rs_a_uui  + Acu_conn_rs_uui_size + 1)
#define	Acu_conn_rs_a_display			(Acu_conn_rs_a_facility + Acu_conn_rs_facility_size + 1)
#define Acu_conn_rs_a_pcs_user			(Acu_conn_rs_a_display  + Acu_conn_rs_display_size  + 1)
#define	Acu_conn_rs_a_notif_ind_ext		(Acu_conn_rs_a_pcs_user + Acu_conn_rs_pcs_user_size + 1)
#define Acu_conn_rs_a_conn_nb			(Acu_conn_rs_a_notif_ind_ext + Acu_conn_rs_notif_ind_ext_size)

#define Acu_conn_rs_signal_val 			Acs_ signal_val

#define	Acu_conn_rs_progress_ind_nb		Acs_ progress_ind.nb
#define	Acu_conn_rs_progress_location	Acu_conn_rs_progress_loc
#define	Acu_conn_rs_progress_description Acu_conn_rs_progress_descr
	/* aliases */
#define	Acu_conn_rs_progress_loc		Acs_ progress_ind.t[0].loc
#define	Acu_conn_rs_progress_descr		Acs_ progress_ind.t[0].descr
	/* access to other occurencies */
#define	Acu_conn_rs_progress_loc_x(i)	Acs_ progress_ind.t[i].loc
#define	Acu_conn_rs_progress_descr_x(i) Acs_ progress_ind.t[i].descr

#define	Acu_conn_rs_notif_ind_present	Acs_ notif_ind_present
#define	Acu_conn_rs_notif_ind			Acs_ notif_ind

#define Acu_conn_rs_conn_nb_type		Acs_ conn_nb.type
#define Acu_conn_rs_conn_nb_plan		Acs_ conn_nb.plan
#define Acu_conn_rs_conn_nb_pres		Acs_ conn_nb.pres
#define Acu_conn_rs_conn_nb_screen		Acs_ conn_nb.screen
#define Acu_conn_rs_conn_sub_add_type	Acs_ conn_nb.sa_type
#define Acu_conn_rs_conn_sub_add_odd	Acs_ conn_nb.sa_odd

#define Acu_conn_rs_a_end_strings		(uchar FAR *)Round((ulong)Acu_conn_rs_a_conn_nb + Acu_conn_rs_conn_nb_size + 1)

#define	Acu_conn_rs_size				(Rnd_sizeof (struct acu_conn_rs_args))

/*---------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_CONN_CO		signal successful connection of an incoming or outgoing
 *					call.
 *
 *	Arguments:
 *	(*)	Acu_conn_co_service			: service finally agreed
 *		Acu_conn_co_data_chani_nb	: nb of elements in the chani_list field
 *		Acu_conn_co_data_chani		: data channel to use (B1, B2, etc, D)
 *		Acu_conn_co_ces				: DL-Connection identifier for that call
 *		Acu_conn_co_charging_value	: charging value (nb of units)
 *		Acu_conn_co_charging_type	: charging type
 *		Acu_conn_co_charging_period	: charging period
 *		Acu_conn_co_charging_multi	: charging multiplier
 *		Acu_conn_co_year			: year
 *		Acu_conn_co_month;			: month
 *		Acu_conn_co_day;			: day
 *		Acu_conn_co_hour;			: hour
 *		Acu_conn_co_minute;			: minute
 *		Acu_conn_co_second;			: second
 *		Acu_conn_co_conn_nb_type	: connected nb type (ACU_NB_TYPE_*)
 *		Acu_conn_co_conn_nb_plan	: connected nb plan
 *		Acu_conn_co_conn_nb_pres	: connected nb pres
 *		Acu_conn_co_conn_nb_screen	: connected nb screening indicator (ACU_NB_SCREEN_*)
 *		Acu_conn_co_conn_sub_add_type: connected sub-add type
 *		Acu_conn_co_conn_sub_add_odd: connected sub-add odd/even indicator
 *
 *		Acu_conn_co_uui_size,
 *		Acu_conn_co_a_uui			: ISDN user to User Information
 *		Acu_conn_co_facility_size,
 *		Acu_conn_co_a_facility		: ISDN facility
 *		Acu_conn_co_display_size,
 *		Acu_conn_co_a_display		: ISDN display Information
 *		Acu_conn_co_pcs_user_size,
 *		Acu_conn_co_a_pcs_user		: ISDN PCS-to-User/User-to-PCS Info.
 *		Acu_conn_co_signal_val 		: signal value
 *
 *  (*) Only these fields are initialied by the Analog ACU, other fields are set to 0.
 *--------------------------------------------------------------------------*/

struct acu_conn_co_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	service;					/* agreed (tele)service */
	struct chani_list    chani_list;    /* list of channel ids to use for data transfer */
	add_t	ces;						/* DL-Connection identifier for that call */
	struct acu_charging_info charging_info;	/* charging information */
	struct acu_date_time date_time;		/* date and time */
	struct acu_redir_nb   redir_nb;		/* redirection nb information */

	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* size of the ISDN User to User information field */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

	uchar	signal_val;	 				/* signal value */
	uchar	notif_ind_present;			/* notification indicator present (ON/OFF) */
	uchar	notif_ind;					/* notification indicator description */
	uchar	notif_ind_ext_size;			/* notification-indicator extension (QSIG) */

	struct acu_conn_nb conn_nb;			/* connected nb/sub_add */

										/* Redir num is just here */
										/* UUI is just here */
										/* then the facility */
										/* then the display strings */
										/* then the pcs_user */
										/* and finally the connected-nb string */
};

#define	Acu_conn_co_					((struct acu_conn_co_args FAR *)p_data) ->
#define	Aco_							Acu_conn_co_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_conn_co_ext_parms_nb		Aco_ ext_descr.nb
#	define Acu_conn_co_ext_parms_lgth	Aco_ ext_descr.lgth
#	define Acu_conn_co_a_ext_parms		Aco_ ext_descr.address
#endif

#define	Acu_conn_co_service				Aco_ service
#define Acu_conn_co_a_chani_list        ((struct chani_list FAR *)&Aco_ chani_list)
#define Acu_conn_co_data_chani_nb       Aco_ chani_list.nb
#define Acu_conn_co_data_chani          Aco_ chani_list.tab[0].chan_id
#define Acu_conn_co_data_chani_nai      Aco_ chani_list.tab[0].nai
#define Acu_conn_co_data_chani_tab(i)   Aco_ chani_list.tab[i].chan_id
#define Acu_conn_co_data_chani_tab_nai(i) Aco_ chani_list.tab[i].nai
#define Acu_conn_co_data_chani_excl     Aco_ chani_list.chan_id_excl
#define Acu_conn_co_ces					Aco_ ces
#define	Acu_conn_co_charging_available	Aco_ charging_info.available
#define Acu_conn_co_charging_value     	Aco_ charging_info.value
#define Acu_conn_co_charging_type     	Aco_ charging_info.type
#define Acu_conn_co_charging_period    	Aco_ charging_info.period
#define Acu_conn_co_charging_multi     	Aco_ charging_info.multi
#define Acu_conn_co_charging_a_currency &(Aco_ charging_info.currency[0])
#define Acu_conn_co_charging	     	Acu_conn_co_charging_value	/* alias */
#define	Acu_conn_co_date_available		Aco_ date_time.available
#define	Acu_conn_co_year				Aco_ date_time.year
#define	Acu_conn_co_month				Aco_ date_time.month
#define	Acu_conn_co_day					Aco_ date_time.day
#define	Acu_conn_co_hour				Aco_ date_time.hour
#define	Acu_conn_co_minute				Aco_ date_time.minute
#define	Acu_conn_co_second				Aco_ date_time.second
#define	Acu_conn_co_a_date_time			&(Aco_ date_time)
#define	Acu_conn_co_redir_nb_pres		Aco_ redir_nb.pres
#define	Acu_conn_co_redir_nb_type		Aco_ redir_nb.type
#define	Acu_conn_co_redir_nb_plan		Aco_ redir_nb.plan
#define	Acu_conn_co_redir_nb_screen		Aco_ redir_nb.screen
#define	Acu_conn_co_redir_nb_reason		Aco_ redir_nb.reason
#define	Acu_conn_co_redir_nb_size		Aco_ redir_nb.size

#define	Acu_conn_co_uui_protocol		Aco_ uui_protocol
#define	Acu_conn_co_uui_size			Aco_ uui_size
#define	Acu_conn_co_facility_size		Aco_ facility_size
#define	Acu_conn_co_display_size		Acu_conn_co_x_display_total_size
#define Acu_conn_co_pcs_user_size		Aco_ pcs_user_size
#define	Acu_conn_co_notif_ind_ext_size	Aco_ notif_ind_ext_size
#define Acu_conn_co_conn_nb_size		Aco_ conn_nb.size

#define Acu_conn_co_x_display_nb		Aco_ display.nb
#define Acu_conn_co_x_display_total_size Aco_ display.total_size
#define Acu_conn_co_x_display_offset(i) Aco_ display.element[i].string_offset
#define Acu_conn_co_x_p_display(i) 		Aco_ display.element[i].p_display
#define Acu_conn_co_x_display_type(i)	Aco_ display.element[i].type
#define Acu_conn_co_x_display_size(i)	Aco_ display.element[i].size
#define Acu_conn_co_a_display_list      &(Aco_ display)

#define	Acu_conn_co_a_redir_nb			((char FAR *)p_data + Acu_conn_co_size)
#define	Acu_conn_co_a_uui				(Acu_conn_co_a_redir_nb + Acu_conn_co_redir_nb_size)
#define	Acu_conn_co_a_facility			(Acu_conn_co_a_uui  + Acu_conn_co_uui_size)
#define	Acu_conn_co_a_display			(Acu_conn_co_a_facility + Acu_conn_co_facility_size)
#define Acu_conn_co_a_pcs_user			(Acu_conn_co_a_display + Acu_conn_co_display_size)
#define	Acu_conn_co_a_notif_ind_ext		(Acu_conn_co_a_pcs_user + Acu_conn_co_pcs_user_size + 1)
#define Acu_conn_co_a_conn_nb			(Acu_conn_co_a_notif_ind_ext + Acu_conn_co_notif_ind_ext_size)

#define	Acu_conn_co_signal_val 			Aco_ signal_val

#define	Acu_conn_co_notif_ind_present	Aco_ notif_ind_present
#define	Acu_conn_co_notif_ind			Aco_ notif_ind

#define Acu_conn_co_conn_nb_type		Aco_ conn_nb.type
#define Acu_conn_co_conn_nb_plan		Aco_ conn_nb.plan
#define Acu_conn_co_conn_nb_pres		Aco_ conn_nb.pres
#define Acu_conn_co_conn_nb_screen		Aco_ conn_nb.screen
#define Acu_conn_co_conn_sub_add_type	Aco_ conn_nb.sa_type
#define Acu_conn_co_conn_sub_add_odd	Aco_ conn_nb.sa_odd

#define Acu_conn_co_a_end_strings		(uchar FAR *)Round((ulong)Acu_conn_co_a_conn_nb + Acu_conn_co_conn_nb_size)

#define	Acu_conn_co_size				(Rnd_sizeof (struct acu_conn_co_args))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_DIGIT_RQ/ACU_DIGIT_IN
 *	Send/Receive digits in Overlap Sending mode.
 *
 *	Arguments :
 *		Acu_digit_rq_sending_complete: Sending-complete IE to be sent/received (ON/OFF)
 *		Acu_digit_rq_digit_size,
 *		Acu_digit_rq_a_digit		: digits string size/address.
 *		Acu_digit_rq_display_size,
 *		Acu_digit_rq_a_display		: ISDN display Information
 *
 *	ARINC SPECIFIC :
 *		Acu_digit_rq_dtmf_digit		: dtmf digit,
 *		Acu_digit_rq_up_down		: up/down indication,
 *		Acu_digit_rq_dtmf_duration	: dtmf duration in step of 40ms.
 *
 *--------------------------------------------------------------------------*/

struct acu_digit_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	sending_complete;			/* Sending-complete IE to be sent/received (ON/OFF) */

	struct acu_called_nb called_nb;		/* called nb information */
	struct acu_display_list display;	/* display list */

#	if ARINC_NETWORKS == ON
		uchar	dtmf_digit;				/* digit */
		uchar	up_down_flag;			/* Up/Down flag */
		uchar	dtmf_duration;			/* duration in step of 40ms */
#	endif

										/* digits are just here */
										/* and finally the display strings */
};

	/* structure name alias for compatibility */

#define acu_digit_rq_args acu_digit_args
#define acu_digit_in_args acu_digit_args

	/* Set of macros for ACU_DIGIT_RQ */

#define	Adg_							((struct acu_digit_rq_args FAR *)p_data) ->

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_digit_rq_ext_parms_nb	Adg_ ext_descr.nb
#	define Acu_digit_rq_ext_parms_lgth	Adg_ ext_descr.lgth
#	define Acu_digit_rq_a_ext_parms		Adg_ ext_descr.address
#endif

#define	Acu_digit_rq_sending_complete	Adg_ sending_complete

#define	Acu_digit_rq_called_nb_type		Adg_ called_nb.type
#define	Acu_digit_rq_called_nb_plan		Adg_ called_nb.plan
#define	Acu_digit_rq_called_nb_size		Adg_ called_nb.size
#define	Acu_digit_rq_called_nb_ie		Adg_ called_nb.digits_ie

#define	Acu_digit_rq_digit_size			Acu_digit_rq_called_nb_size

#define	Acu_digit_rq_display_size		Acu_digit_rq_x_display_total_size

#define Acu_digit_rq_x_display_nb		Adg_ display.nb
#define Acu_digit_rq_x_display_total_size Adg_ display.total_size
#define Acu_digit_rq_x_display_offset(i) Adg_ display.element[i].string_offset
#define Acu_digit_rq_x_p_display(i) 	Adg_ display.element[i].p_display
#define Acu_digit_rq_x_display_type(i)	Adg_ display.element[i].type
#define Acu_digit_rq_x_display_size(i)	Adg_ display.element[i].size
#define Acu_digit_rq_a_display_list     &(Adg_ display)

#define	Acu_digit_rq_a_called_nb		((char FAR *)p_data   + Acu_digit_rq_size)
#define	Acu_digit_rq_a_digit			Acu_digit_rq_a_called_nb
#define	Acu_digit_rq_a_display			(Acu_digit_rq_a_digit + Acu_digit_rq_digit_size + 1)

#if ARINC_NETWORKS == ON
#	define	Acu_digit_rq_dtmf_digit		(Adg_ dtmf_digit)
#	define	Acu_digit_rq_up_down		(Adg_ up_down_flag)
#	define	Acu_digit_rq_dtmf_duration	(Adg_ dtmf_duration)
#endif

#define Acu_digit_rq_a_end_strings		(uchar FAR *)Round((ulong)Acu_digit_rq_a_display + Acu_digit_rq_display_size + 1)

#define	Acu_digit_rq_size				(Rnd_sizeof (struct acu_digit_args))

	/* Set of macros for ACU_DIGIT_IN alias */

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_digit_in_ext_parms_nb	Adg_ ext_descr.nb
#	define Acu_digit_in_ext_parms_lgth	Adg_ ext_descr.lgth
#	define Acu_digit_in_a_ext_parms		Adg_ ext_descr.address
#endif

#define	Acu_digit_in_sending_complete	Adg_ sending_complete

#define	Acu_digit_in_called_nb_type		Adg_ called_nb.type
#define	Acu_digit_in_called_nb_plan		Adg_ called_nb.plan
#define	Acu_digit_in_called_nb_size		Adg_ called_nb.size
#define	Acu_digit_in_called_nb_ie		Adg_ called_nb.digits_ie

#define	Acu_digit_in_digit_size			Acu_digit_rq_digit_size

#define	Acu_digit_in_display_size		Acu_digit_rq_display_size

#define Acu_digit_in_x_display_nb		Adg_ display.nb
#define Acu_digit_in_x_display_total_size Adg_ display.total_size
#define Acu_digit_in_x_display_offset(i) Adg_ display.element[i].string_offset
#define Acu_digit_in_x_p_display(i) 	Adg_ display.element[i].p_display
#define Acu_digit_in_x_display_type(i)	Adg_ display.element[i].type
#define Acu_digit_in_x_display_size(i)	Adg_ display.element[i].size
#define Acu_digit_in_a_display_list     &(Adg_ display)

#define	Acu_digit_in_a_called_nb		Acu_digit_rq_a_called_nb
#define	Acu_digit_in_a_digit			Acu_digit_rq_a_digit
#define	Acu_digit_in_a_display			Acu_digit_rq_a_display

#if ARINC_NETWORKS == ON
#	define	Acu_digit_in_dtmf_digit		Adg_ dtmf_digit
#	define	Acu_digit_in_up_down		Adg_ up_down_flag
#	define	Acu_digit_in_dtmf_duration	Adg_ dtmf_duration
#endif

#define Acu_digit_in_a_end_strings		(uchar FAR *)Round((ulong)Acu_digit_in_a_display + Acu_digit_in_display_size + 1)

#define	Acu_digit_in_size				Acu_digit_rq_size

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_DIGIT_CO	Confirm end-of-called-nb-reached in Overlap Sending mode.
 *
 *	Arguments :
 *		Acu_digit_co_display_size,
 *		Acu_digit_co_a_display		: ISDN display Information
 *
 *--------------------------------------------------------------------------*/

struct acu_digit_co_args {
	struct acu_display_list display;	/* display list */

										/* display strings are just here */
};

#define	Acu_digit_co_					((struct acu_digit_co_args FAR *)p_data) ->
#define	Acu_digit_co_display_size		Acu_digit_co_x_display_total_size

#define Acu_digit_co_x_display_nb		Acu_digit_co_ display.nb
#define Acu_digit_co_x_display_total_size Acu_digit_co_ display.total_size
#define Acu_digit_co_x_display_offset(i) Acu_digit_co_ display.element[i].string_offset
#define Acu_digit_co_x_p_display(i) 	Acu_digit_co_ display.element[i].p_display
#define Acu_digit_co_x_display_type(i)	Acu_digit_co_ display.element[i].type
#define Acu_digit_co_x_display_size(i)	Acu_digit_co_ display.element[i].size
#define Acu_digit_co_a_display_list     &(Acu_digit_co_ display)

#define	Acu_digit_co_a_display			((char FAR *)p_data + Acu_digit_co_size)

#define	Acu_digit_co_size				(Rnd_sizeof (struct acu_digit_co_args))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_CALL_PROC_RQ		Call Proceeding Request
 *
 *	Arguments :
 *      Acu_call_proc_rq_data_chani_nb			: nb of elements in the chani_list.tab field
 *      Acu_call_proc_rq_data_chani   			: data channel to use (B1, B2, etc, D, or 0 for any)
 *		Acu_call_proc_rq_display_size,
 *		Acu_call_proc_rq_a_display    			: ISDN display Information
 *		Acu_call_proc_rq_pcs_user_size,
 *		Acu_call_proc_rq_a_pcs_user				: ISDN PCS-to-User/User-to-PCS Info.
 *		Acu_call_proc_rq_progress_location 		: progress location
 *		Acu_call_proc_rq_progress_description	: progress description
 *
 *--------------------------------------------------------------------------*/

struct acu_call_proc_rq_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	struct chani_list chani_list;		/* list of channel ids to use for data transfer */

	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

	struct acu_progress_ind progress_ind;/* progress indicator */

	uchar	notif_ind_present;			/* notification indicator present (ON/OFF) */
	uchar	notif_ind;					/* notification indicator description */

										/* display strings are here */
										/* and finally pcs_user */

};

#define	Acu_call_proc_rq_						((struct acu_call_proc_rq_args FAR *)p_data) ->
#define	Apq_									Acu_call_proc_rq_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_call_proc_rq_ext_parms_nb		Apq_ ext_descr.nb
#	define Acu_call_proc_rq_ext_parms_lgth		Apq_ ext_descr.lgth
#	define Acu_call_proc_rq_a_ext_parms			Apq_ ext_descr.address
#endif

#define Acu_call_proc_rq_a_chani_list			((struct chani_list FAR *)&Apq_ chani_list)
#define Acu_call_proc_rq_data_chani_nb			Apq_ chani_list.nb
#define Acu_call_proc_rq_data_chani				Apq_ chani_list.tab[0].chan_id
#define Acu_call_proc_rq_data_chani_nai			Apq_ chani_list.tab[0].nai
#define Acu_call_proc_rq_data_chani_tab(i)		Apq_ chani_list.tab[i].chan_id
#define Acu_call_proc_rq_data_chani_tab_nai(i)	Apq_ chani_list.tab[i].nai
#define Acu_call_proc_rq_data_chani_excl    	Apq_ chani_list.chan_id_excl

#define	Acu_call_proc_rq_display_size			Acu_call_proc_rq_x_display_total_size
#define Acu_call_proc_rq_pcs_user_size			Apq_ pcs_user_size

#define Acu_call_proc_rq_x_display_nb			Apq_ display.nb
#define Acu_call_proc_rq_x_display_total_size	Apq_ display.total_size
#define Acu_call_proc_rq_x_display_offset(i)	Apq_ display.element[i].string_offset
#define Acu_call_proc_rq_x_p_display(i) 		Apq_ display.element[i].p_display
#define Acu_call_proc_rq_x_display_type(i)		Apq_ display.element[i].type
#define Acu_call_proc_rq_x_display_size(i)		Apq_ display.element[i].size
#define Acu_call_proc_rq_a_display_list     	&(Apq_ display)

#define	Acu_call_proc_rq_a_display				((char FAR *)p_data + Acu_call_proc_rq_size)
#define Acu_call_proc_rq_a_pcs_user				(Acu_call_proc_rq_a_display + Acu_call_proc_rq_display_size + 1)

#define	Acu_call_proc_rq_progress_ind_nb		Apq_ progress_ind.nb
#define	Acu_call_proc_rq_progress_location		Acu_call_proc_rq_progress_loc
#define	Acu_call_proc_rq_progress_description	Acu_call_proc_rq_progress_descr
	/* aliases */
#define	Acu_call_proc_rq_progress_loc			Apq_ progress_ind.t[0].loc
#define	Acu_call_proc_rq_progress_descr			Apq_ progress_ind.t[0].descr
	/* access to other occurencies */
#define	Acu_call_proc_rq_progress_loc_x(i)		Apq_ progress_ind.t[i].loc
#define	Acu_call_proc_rq_progress_descr_x(i)	Apq_ progress_ind.t[i].descr

#define	Acu_call_proc_rq_notif_ind_present		Apq_ notif_ind_present
#define	Acu_call_proc_rq_notif_ind				Apq_ notif_ind

#define Acu_call_proc_rq_a_end_strings			(uchar FAR *)Round((ulong)Acu_call_proc_rq_a_pcs_user + Acu_call_proc_rq_pcs_user_size + 1)

#define	Acu_call_proc_rq_size					(Rnd_sizeof (struct acu_call_proc_rq_args))

/*----------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *	ACU_PROGRESS_IN: 		progress indication
 *
 *  Interface: Isdn
 *	ACU_PROGRESS_RQ: 		progress request
 *
 *	Arguments :
 *	(*)	Acu_progress_in_event      			: detected event
 *		Acu_progress_in_cause      			: cause value
 *		Acu_progress_in_progress_location 	: progress location
 *		Acu_progress_in_progress_description: progress description
 *      Acu_progress_in_data_chani_nb		: nb of elements in the chani_list field
 *      Acu_progress_in_data_chani  		: channel id
 *		Acu_progress_in_facility_size,
 *		Acu_progress_in_a_facility			: ISDN facility
 *		Acu_progress_in_display_size,
 *		Acu_progress_in_a_display			: ISDN display Information
 *		Acu_progress_in_pcs_user_size,
 *		Acu_progress_in_a_pcs_user			: ISDN PCS-to-User/User-to-PCS Info.
 *		Acu_progress_in_ces					: DL-Connection identifier for that call
 *		Acu_progress_in_signal_val 			: signal value
 *
 *  (*) (ACU_PROGRESS_IN) Only these fields are initialized by the Analog ACU, other fields are set to 0.
 *--------------------------------------------------------------------------*/

struct acu_progress_in_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar				event;			/* event parameter */

	uchar				cause;			/* cause value */

	struct acu_progress_ind progress_ind;/* progress indicator */

	uchar				notif_ind_present;/* notification indicator present (ON/OFF) */
	uchar				notif_ind;		/* notification indicator description */
	uchar				notif_ind_ext_size;	/* notification-indicator extension (QSIG) */

	add_t				ces;			/* DL-Connection identifier for that call */

	uchar				signal_val;		/* signal value */

	struct chani_list	chani_list;		/* list of channel ids to use for data transfer */

	acu_size_t	facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

										/* facility is just here */
										/* then display strings */
										/* and finally the pcs_user */
};

#define	Acu_progress_in_						((struct acu_progress_in_args FAR *)p_data) ->
#define	Agi_									Acu_progress_in_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_progress_in_ext_parms_nb			Agi_ ext_descr.nb
#	define Acu_progress_in_ext_parms_lgth		Agi_ ext_descr.lgth
#	define Acu_progress_in_a_ext_parms			Agi_ ext_descr.address
#endif

#define	Acu_progress_in_event					Agi_ event
#define	Acu_progress_in_cause					Agi_ cause

#define	Acu_progress_in_progress_ind_nb			Agi_ progress_ind.nb
#define	Acu_progress_in_progress_location		Acu_progress_in_progress_loc
#define	Acu_progress_in_progress_description	Acu_progress_in_progress_descr
	/* aliases */
#define	Acu_progress_in_progress_loc			Agi_ progress_ind.t[0].loc
#define	Acu_progress_in_progress_descr			Agi_ progress_ind.t[0].descr
	/* access to other occurencies */
#define	Acu_progress_in_progress_loc_x(i)		Agi_ progress_ind.t[i].loc
#define	Acu_progress_in_progress_descr_x(i)		Agi_ progress_ind.t[i].descr

#define	Acu_progress_in_notif_ind_present		Agi_ notif_ind_present
#define	Acu_progress_in_notif_ind				Agi_ notif_ind

#define Acu_progress_in_a_chani_list			((struct chani_list FAR *)&Agi_ chani_list)
#define Acu_progress_in_data_chani_nb   		Agi_ chani_list.nb
#define Acu_progress_in_data_chani      		Agi_ chani_list.tab[0].chan_id
#define Acu_progress_in_data_chani_nai  		Agi_ chani_list.tab[0].nai
#define Acu_progress_in_data_chani_tab(i)    	Agi_ chani_list.tab[i].chan_id
#define Acu_progress_in_data_chani_tab_nai(i)	Agi_ chani_list.tab[i].nai
#define Acu_progress_in_data_chani_excl 		Agi_ chani_list.chan_id_excl

#define	Acu_progress_in_facility_size			Agi_ facility_size
#define	Acu_progress_in_display_size			Acu_progress_in_x_display_total_size
#define Acu_progress_in_pcs_user_size			Agi_ pcs_user_size
#define	Acu_progress_in_notif_ind_ext_size		Agi_ notif_ind_ext_size

#define Acu_progress_in_x_display_nb			Agi_ display.nb
#define Acu_progress_in_x_display_total_size	Agi_ display.total_size
#define Acu_progress_in_x_display_offset(i)		Agi_ display.element[i].string_offset
#define Acu_progress_in_x_p_display(i) 			Agi_ display.element[i].p_display
#define Acu_progress_in_x_display_type(i)		Agi_ display.element[i].type
#define Acu_progress_in_x_display_size(i)		Agi_ display.element[i].size
#define Acu_progress_in_a_display_list          &(Agi_ display)

#define	Acu_progress_in_a_facility				((char FAR *)p_data + Acu_progress_in_size)
#define	Acu_progress_in_a_display				(Acu_progress_in_a_facility + Acu_progress_in_facility_size)
#define Acu_progress_in_a_pcs_user				(Acu_progress_in_a_display + Acu_progress_in_display_size)
#define	Acu_progress_in_a_notif_ind_ext			(Acu_progress_in_a_pcs_user + Acu_progress_in_pcs_user_size + 1)

#define	Acu_progress_in_ces						Agi_ ces
#define	Acu_progress_in_signal_val				Agi_ signal_val

#define Acu_progress_in_a_end_strings			(uchar FAR *)Round((ulong)Acu_progress_in_a_notif_ind_ext + Acu_progress_in_notif_ind_ext_size)

#define	Acu_progress_in_size					(Rnd_sizeof (struct acu_progress_in_args))

/*----------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_ALERT_RQ		alert request
 *
 *	Arguments :
 *		Acu_alert_rq_uui_size,
 *		Acu_alert_rq_a_uui		: ISDN facility
 *		Acu_alert_rq_display_size,
 *		Acu_alert_rq_a_display	: ISDN display Information
 *		Acu_alert_rq_pcs_user_size,
 *		Acu_alert_rq_a_pcs_user	: ISDN PCS-to-User/User-to-PCS Info.
 *		Acu_alert_rq_signal_val 			: signal value
 *		Acu_alert_rq_progress_location 		: progress location
 *		Acu_alert_rq_progress_description	: progress description
 *		Acu_alert_rq_mlpp_status: Q.955 MLPP Call Request Status:
 *		                          (0 when call is not an MLPP call)
 *		                          1:success, called user is an MLPP subscriber;
 *		                          2:success, called user is not an MLPP subscriber;
 *		                          3:failure case A: MLPP call from the calling user is a precedence call and cannot be completed;
 *		                          2:failure case B: MLPP call from the calling user or MLPP call between two MLPP subscribers experiences preemption;
 *
 *--------------------------------------------------------------------------*/

struct acu_alert_rq_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	struct chani_list chani_list;		/* list of channel ids to use for data transfer */

	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* uui size */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

	uchar		signal_val;					/* signal value */

	struct acu_progress_ind progress_ind;/* progress indicator */

	uchar		notif_ind_present;		/* notification indicator present (ON/OFF) */
	uchar		notif_ind;				/* notification indicator description */
	uchar		notif_ind_ext_size;		/* notification-indicator extension (QSIG) */

	uchar		mlpp_status;			/* Q.955 MLPP Call Request Status */

										/* UUI is just here */
										/* then display */
										/* and finally pcs_user */
};

#define	Acu_alert_rq_					((struct acu_alert_rq_args FAR *)p_data) ->
#define	Aaq_							Acu_alert_rq_


#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_alert_rq_ext_parms_nb			Aaq_ ext_descr.nb
#	define Acu_alert_rq_ext_parms_lgth			Aaq_ ext_descr.lgth
#	define Acu_alert_rq_a_ext_parms				Aaq_ ext_descr.address
#endif

#define Acu_alert_rq_a_chani_list				((struct chani_list FAR *)&Aaq_ chani_list)
#define Acu_alert_rq_data_chani_nb				Aaq_ chani_list.nb
#define Acu_alert_rq_data_chani					Aaq_ chani_list.tab[0].chan_id
#define Acu_alert_rq_data_chani_nai				Aaq_ chani_list.tab[0].nai
#define Acu_alert_rq_data_chani_tab(i)			Aaq_ chani_list.tab[i].chan_id
#define Acu_alert_rq_data_chani_tab_nai(i)		Aaq_ chani_list.tab[i].nai
#define Acu_alert_rq_data_chani_excl        	Aaq_ chani_list.chan_id_excl

#define Acu_alert_rq_x_display_nb				Aaq_ display.nb
#define Acu_alert_rq_x_display_total_size		Aaq_ display.total_size
#define Acu_alert_rq_x_display_offset(i)		Aaq_ display.element[i].string_offset
#define Acu_alert_rq_x_p_display(i) 			Aaq_ display.element[i].p_display
#define Acu_alert_rq_x_display_type(i)			Aaq_ display.element[i].type
#define Acu_alert_rq_x_display_size(i)			Aaq_ display.element[i].size
#define Acu_alert_rq_a_display_list				&(Aaq_ display)

#define	Acu_alert_rq_uui_protocol				Aaq_ uui_protocol
#define	Acu_alert_rq_uui_size					Aaq_ uui_size
#define	Acu_alert_rq_display_size				Acu_alert_rq_x_display_total_size
#define Acu_alert_rq_pcs_user_size				Aaq_ pcs_user_size
#define	Acu_alert_rq_notif_ind_ext_size			Aaq_ notif_ind_ext_size

#define	Acu_alert_rq_a_uui						((char FAR *)p_data + Acu_alert_rq_size)
#define	Acu_alert_rq_a_display					(Acu_alert_rq_a_uui + Acu_alert_rq_uui_size + 1)
#define Acu_alert_rq_a_pcs_user					(Acu_alert_rq_a_display + Acu_alert_rq_display_size + 1)
#define	Acu_alert_rq_a_notif_ind_ext			(Acu_alert_rq_a_pcs_user+ Acu_alert_rq_pcs_user_size + 1)

#define	Acu_alert_rq_signal_val					Aaq_ signal_val

#define	Acu_alert_rq_progress_ind_nb			Aaq_ progress_ind.nb
#define	Acu_alert_rq_progress_location			Acu_alert_rq_progress_loc
#define	Acu_alert_rq_progress_description		Acu_alert_rq_progress_descr
	/* aliases */
#define	Acu_alert_rq_progress_loc				Aaq_ progress_ind.t[0].loc
#define	Acu_alert_rq_progress_descr				Aaq_ progress_ind.t[0].descr
	/* access to other occurencies */
#define	Acu_alert_rq_progress_loc_x(i)			Aaq_ progress_ind.t[i].loc
#define	Acu_alert_rq_progress_descr_x(i)		Aaq_ progress_ind.t[i].descr

#define	Acu_alert_rq_notif_ind_present			Aaq_ notif_ind_present
#define	Acu_alert_rq_notif_ind					Aaq_ notif_ind

#define	Acu_alert_rq_mlpp_status				Aaq_ mlpp_status

#define Acu_alert_rq_a_end_strings				(uchar FAR *)Round((ulong)Acu_alert_rq_a_notif_ind_ext + Acu_alert_rq_notif_ind_ext_size + 1)

#define	Acu_alert_rq_size						(Rnd_sizeof (struct acu_alert_rq_args))

/*----------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_ALERT_IN		alert indication
 *
 *	Arguments :
 *	(*)	Acu_alert_in_alert 					: detected alert (Alert code)
 *		Acu_alert_in_a_facility				: ISDN facility
 *		Acu_alert_in_facility_size,
 *		Acu_alert_in_a_facility				: ISDN facility
 *		Acu_alert_in_display_size,
 *		Acu_alert_in_a_display				: ISDN display Information
 *		Acu_alert_in_pcs_user_size,
 *		Acu_alert_in_a_pcs_user				: ISDN PCS-to-User/User-to-PCS Info.
 *		Acu_alert_in_ces					: DL-Connection identifier for that call
 *		Acu_alert_in_signal_val 			: signal value
 *		Acu_alert_in_progress_location 		: progress location
 *		Acu_alert_in_progress_description	: progress description
 *		Acu_alert_in_mlpp_status: Q.955 MLPP Call Request Status:
 *		                          (0 when call is not an MLPP call)
 *		                          1:success, called user is an MLPP subscriber;
 *		                          2:success, called user is not an MLPP subscriber;
 *		                          3:failure case A: MLPP call from the calling user is a precedence call and cannot be completed;
 *		                          2:failure case B: MLPP call from the calling user or MLPP call between two MLPP subscribers experiences preemption;
 *
 *      ACU_PSTN_CLIP == ON :
 *
 *  (*) Acu_alert_in_a_raw_msg              : address of CLIP raw data
 *  (*) Acu_alert_in_raw_msg_size           : size of CLIP raw data
 *
 *  (*) Acu_alert_in_a_clip_msg             : Get ptr for Acu_clip__xxx macros
 *
 *  (*) Acu_clip__raw_msg_size(p)           : see sub-structure acu_clip__message
 *  (*) Acu_clip__msg_name(p)
 *  (*) Acu_clip__date_time_size(p)
 *  (*) Acu_clip__date_time_off(p)
 *  (*) Acu_clip__a_date_time(p)
 *  (*) .....
 *
 *  (*) Only these fields are initialized by the Analog ACU, other fields are set to 0.
 *--------------------------------------------------------------------------*/

struct acu_alert_in_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	alert;						/* event parameter */

	struct chani_list chani_list; 		/* list of channel ids to use for data transfer */
	struct acu_redir_nb   redir_nb;		/* redirection nb information */

	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* uui size */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

	add_t		ces;					/* DL-Connection identifier for that call */

	uchar		signal_val;				/* signal value */

	struct acu_progress_ind progress_ind;/* progress indicator */

	uchar		notif_ind_present;		/* notification indicator present (ON/OFF) */
	uchar		notif_ind;				/* notification indicator description */
	uchar		notif_ind_ext_size;		/* notification-indicator extension (QSIG) */

	uchar		mlpp_status;			/* Q.955 MLPP Call Request Status */

#   if ACU_PSTN_CLIP == ON
        struct acu_clip__message clip;  /* Calling Line Identification Presentation (Caller ID) */
#   endif
										/* UUI is just here */
										/* then the facility */
										/* then the display strings */
										/* then the pcs_user */
										/* and finally the clip message */
};

#define	Acu_alert_in_					((struct acu_alert_in_args FAR *)p_data) ->
#define	Aai_							Acu_alert_in_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_alert_in_ext_parms_nb			Aai_ ext_descr.nb
#	define Acu_alert_in_ext_parms_lgth			Aai_ ext_descr.lgth
#	define Acu_alert_in_a_ext_parms				Aai_ ext_descr.address
#endif

#define	Acu_alert_in_alert						Aai_ alert

#define Acu_alert_in_a_chani_list				((struct chani_list FAR *)&Aai_ chani_list)
#define Acu_alert_in_data_chani_nb				Aai_ chani_list.nb
#define Acu_alert_in_data_chani					Aai_ chani_list.tab[0].chan_id
#define Acu_alert_in_data_chani_nai				Aai_ chani_list.tab[0].nai
#define Acu_alert_in_data_chani_tab(i)			Aai_ chani_list.tab[i].chan_id
#define Acu_alert_in_data_chani_tab_nai(i)		Aai_ chani_list.tab[i].nai
#define Acu_alert_in_data_chani_excl        	Aai_ chani_list.chan_id_excl

#define	Acu_alert_in_redir_nb_pres				Aai_ redir_nb.pres
#define	Acu_alert_in_redir_nb_type				Aai_ redir_nb.type
#define	Acu_alert_in_redir_nb_plan				Aai_ redir_nb.plan
#define	Acu_alert_in_redir_nb_screen			Aai_ redir_nb.screen
#define	Acu_alert_in_redir_nb_reason			Aai_ redir_nb.reason
#define	Acu_alert_in_redir_nb_size				Aai_ redir_nb.size

#define	Acu_alert_in_uui_protocol				Aai_ uui_protocol
#define	Acu_alert_in_uui_size					Aai_ uui_size
#define	Acu_alert_in_facility_size				Aai_ facility_size
#define	Acu_alert_in_display_size				Acu_alert_in_x_display_total_size
#define Acu_alert_in_pcs_user_size				Aai_ pcs_user_size
#define	Acu_alert_in_notif_ind_ext_size			Aai_ notif_ind_ext_size

#define Acu_alert_in_x_display_nb				Aai_ display.nb
#define Acu_alert_in_x_display_total_size		Aai_ display.total_size
#define Acu_alert_in_x_display_offset(i)		Aai_ display.element[i].string_offset
#define Acu_alert_in_x_p_display(i) 			Aai_ display.element[i].p_display
#define Acu_alert_in_x_display_type(i)			Aai_ display.element[i].type
#define Acu_alert_in_x_display_size(i)			Aai_ display.element[i].size
#define Acu_alert_in_a_display_list   		  &(Aai_ display)

#define	Acu_alert_in_a_redir_nb					((char FAR *)p_data + Acu_alert_in_size)
#define	Acu_alert_in_a_uui						(Acu_alert_in_a_redir_nb + Acu_alert_in_redir_nb_size)
#define	Acu_alert_in_a_facility					(Acu_alert_in_a_uui + Acu_alert_in_uui_size)
#define	Acu_alert_in_a_display					(Acu_alert_in_a_facility + Acu_alert_in_facility_size)
#define Acu_alert_in_a_pcs_user					(Acu_alert_in_a_display + Acu_alert_in_display_size)
#define	Acu_alert_in_a_notif_ind_ext			(Acu_alert_in_a_pcs_user+ Acu_alert_in_pcs_user_size + 1)

#define	Acu_alert_in_ces						Aai_ ces
#define	Acu_alert_in_signal_val					Aai_ signal_val

#define	Acu_alert_in_progress_ind_nb			Aai_ progress_ind.nb
#define	Acu_alert_in_progress_location			Acu_alert_in_progress_loc
#define	Acu_alert_in_progress_description		Acu_alert_in_progress_descr
	/* aliases */
#define	Acu_alert_in_progress_loc				Aai_ progress_ind.t[0].loc
#define	Acu_alert_in_progress_descr				Aai_ progress_ind.t[0].descr
	/* access to other occurencies */
#define	Acu_alert_in_progress_loc_x(i)			Aai_ progress_ind.t[i].loc
#define	Acu_alert_in_progress_descr_x(i)		Aai_ progress_ind.t[i].descr

#define	Acu_alert_in_notif_ind_present			Aai_ notif_ind_present
#define	Acu_alert_in_notif_ind					Aai_ notif_ind

#define	Acu_alert_in_mlpp_status				Aai_ mlpp_status



#if ACU_PSTN_CLIP == ON

#   define  Acu_alert_in_a_raw_msg              (Acu_alert_in_a_notif_ind_ext + Acu_alert_in_notif_ind_ext_size)
#   define  Acu_alert_in_raw_msg_size           Aai_ clip.raw_msg_size

#   define	Acu_alert_in_a_clip_msg             &(Aai_ clip)                    /* Get ptr for Acu_clip__xxx macros */

/*  define	Acu_clip__raw_msg_size(p)           (p)-> raw_msg_size      */
/*  define	Acu_clip__msg_name(p)               (p)-> msg_name          */
/*  define	Acu_clip__date_time_size(p)         (p)-> date_time_size    */
/*  .... see ACU_CONN_IN primitve                                       */

#endif


#if ACU_PSTN_CLIP == OFF
#   define Acu_alert_in_a_end_strings			(uchar FAR *)Round((ulong)Acu_alert_in_a_notif_ind_ext + Acu_alert_in_notif_ind_ext_size)
#else
#   define Acu_alert_in_a_end_strings			(uchar FAR *)Round((ulong)Acu_alert_in_a_raw_msg + Acu_alert_in_raw_msg_size)
#endif

#define	Acu_alert_in_size						(Rnd_sizeof (struct acu_alert_in_args))

/*---------------------------------------------------------------------------
 * Interface: Isdn
 *
 *	ACU_USER_INFO_RQ/IN		User Information.
 *
 *--------------------------------------------------------------------------*/

struct acu_user_info {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar      notif_ind_present;		/* notification indicator present (ON/OFF) */
	uchar      notif_ind;				/* notification indicator description */

	uchar      fac_protocol_discr;		/* user facility protocol discriminator */
	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* uui size */
	acu_size_t fac_size;				/* user facility size */

										/* UUI is just here */
										/* and finally the facility */
};

#define	Acu_user_info_				((struct acu_user_info FAR *)p_data) ->
#define	Au_							Acu_user_info_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_user_info_ext_parms_nb	Au_ ext_descr.nb
#	define Acu_user_info_ext_parms_lgth	Au_ ext_descr.lgth
#	define Acu_user_info_a_ext_parms	Au_ ext_descr.address
#endif

#define	Acu_user_info_notif_ind_present	Au_ notif_ind_present
#define	Acu_user_info_notif_ind			Au_ notif_ind

#define	Acu_user_info_fac_protocol_discr	Au_ fac_protocol_discr

#define	Acu_user_info_uui_protocol	Au_ uui_protocol
#define	Acu_user_info_uui_size		Au_ uui_size
#define	Acu_user_info_fac_size		Au_ fac_size

#define	Acu_user_info_a_uui			((char FAR *)p_data  + Acu_user_info_size)
#define	Acu_user_info_a_fac			(Acu_user_info_a_uui + Acu_user_info_uui_size + 1)

#define Acu_user_info_a_end_strings	(uchar FAR *)Round((ulong)Acu_user_info_a_fac + (int)Acu_user_info_fac_size)

#define	Acu_user_info_size			(Rnd_sizeof (struct acu_user_info))

/*----------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_CLEAR_RQ			request call clearing.
 *
 *	Arguments :
 *	(*)	Acu_clear_rq_priority   : call clear priority normal/urgent (ACU_PHIGH/ACU_PLOW)
 *		Acu_clear_rq_cause		: call clearing cause value to be sent
 *								  (if 0, "Normal Clearing" (16) is sent).
 *		Acu_clear_rq_uui_size,
 *		Acu_clear_rq_a_uui		: ISDN user to User Information
 *		Acu_clear_rq_facility_size,
 *		Acu_clear_rq_facility	: facility
 *		Acu_clear_rq_pcs_user_size,
 *		Acu_clear_rq_a_pcs_user	: ISDN PCS-to-User/User-to-PCS Info.
 *		Acu_clear_rq_signal_val	: signal value
 *
 *  (*) Only these fields are used by the Analog ACU, other fields are ignored.
 *--------------------------------------------------------------------------*/

struct acu_clear_rq_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	priority;					/* phone call priority */
	uchar	cause;						/* clearing cause value */
	struct acu_charging_info charging_info;	/* charging information */

	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* size of the ISDN User to User information field */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */
	uchar      signal_val;				/* signal value */
	struct acu_progress_ind progress_ind;/* progress indicator */
	uchar      notif_ind_present;		/* notification indicator present (ON/OFF) */
	uchar      notif_ind;				/* notification indicator description */
	uchar      notif_ind_ext_size;		/* notification-indicator extension (QSIG) */

										/* UUI is just here */
										/* then the facility */
										/* then the display strings */
										/* and finally the pcs_user */
};

#define	Acu_clear_rq_					((struct acu_clear_rq_args FAR *)p_data) ->
#define	Akr_							Acu_clear_rq_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_clear_rq_ext_parms_nb	Akr_ ext_descr.nb
#	define Acu_clear_rq_ext_parms_lgth	Akr_ ext_descr.lgth
#	define Acu_clear_rq_a_ext_parms		Akr_ ext_descr.address
#endif

#define	Acu_clear_rq_priority			Akr_ priority
#define	Acu_clear_rq_cause				Akr_ cause

#define	Acu_clear_rq_charging_available	Akr_ charging_info.available
#define Acu_clear_rq_charging_value     Akr_ charging_info.value
#define Acu_clear_rq_charging_type     	Akr_ charging_info.type
#define Acu_clear_rq_charging_period    Akr_ charging_info.period
#define Acu_clear_rq_charging_multi     Akr_ charging_info.multi
#define Acu_clear_rq_charging_a_currency &(Akr_ charging_info.currency[0])
#define Acu_clear_rq_charging	     	Acu_clear_rq_charging_value	/* alias */
#define	Acu_clear_rq_total_cost			Acu_clear_rq_charging /* alias */

#define	Acu_clear_rq_uui_protocol		Akr_ uui_protocol
#define	Acu_clear_rq_uui_size			Akr_ uui_size
#define	Acu_clear_rq_facility_size		Akr_ facility_size
#define	Acu_clear_rq_display_size		Acu_clear_rq_x_display_total_size
#define Acu_clear_rq_pcs_user_size		Akr_ pcs_user_size
#define	Acu_clear_rq_notif_ind_ext_size	Akr_ notif_ind_ext_size

#define Acu_clear_rq_x_display_nb		Akr_ display.nb
#define Acu_clear_rq_x_display_total_size Akr_ display.total_size
#define Acu_clear_rq_x_display_offset(i) Akr_ display.element[i].string_offset
#define Acu_clear_rq_x_p_display(i) 	Akr_ display.element[i].p_display
#define Acu_clear_rq_x_display_type(i)	Akr_ display.element[i].type
#define Acu_clear_rq_x_display_size(i)	Akr_ display.element[i].size
#define Acu_clear_rq_a_display_list		&(Akr_ display)

#define	Acu_clear_rq_a_uui				((char FAR *)p_data + Acu_clear_rq_size)
#define	Acu_clear_rq_a_facility			(Acu_clear_rq_a_uui + Acu_clear_rq_uui_size + 1)
#define	Acu_clear_rq_a_display			(Acu_clear_rq_a_facility + Acu_clear_rq_facility_size + 1)
#define Acu_clear_rq_a_pcs_user			(Acu_clear_rq_a_display  + Acu_clear_rq_display_size  + 1)
#define	Acu_clear_rq_a_notif_ind_ext	(Acu_clear_rq_a_pcs_user+ Acu_clear_rq_pcs_user_size + 1)

#define	Acu_clear_rq_signal_val			Akr_ signal_val

#define	Acu_clear_rq_progress_ind_nb	Akr_ progress_ind.nb
#define	Acu_clear_rq_progress_location	Acu_clear_rq_progress_loc
#define	Acu_clear_rq_progress_description Acu_clear_rq_progress_descr
	/* aliases */
#define	Acu_clear_rq_progress_loc		Akr_ progress_ind.t[0].loc
#define	Acu_clear_rq_progress_descr		Akr_ progress_ind.t[0].descr
	/* access to other occurencies */
#define	Acu_clear_rq_progress_loc_x(i)	Akr_ progress_ind.t[i].loc
#define	Acu_clear_rq_progress_descr_x(i) Akr_ progress_ind.t[i].descr

#define	Acu_clear_rq_notif_ind_present	Akr_ notif_ind_present
#define	Acu_clear_rq_notif_ind			Akr_ notif_ind

#define Acu_clear_rq_a_end_strings		(uchar FAR *)Round((ulong)Acu_clear_rq_a_notif_ind_ext + Acu_clear_rq_notif_ind_ext_size + 1)

#define	Acu_clear_rq_size				(Rnd_sizeof (struct acu_clear_rq_args))

/*----------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_CLEAR_IN			call cleared indication.
 *
 *	Arguments :
 *  (*) Acu_clear_in_ret_code 		: return code (ACURC_BUSY, etc., described
 *								  		below)
 *		Acu_clear_in_charging_vlaue	: charging value (nb of units)
 *		Acu_clear_in_charging_type	: charging type
 *		Acu_clear_in_charging_period: charging period
 *		Acu_clear_in_charging_multi	: charging multiplier
 *		Acu_clear_in_total_cost     : alias for charging field
 *		Acu_clear_in_mlpp_circuit_reserved: Q.955 MLPP: circuit reserved for reuse indicator: OFF:no, ON:yes
 *	    Acu_clear_in_uui_size,
 *		Acu_clear_in_a_uui			: ISDN user to User Information
 *		Acu_clear_in_facility_size,
 *		Acu_clear_in_a_facility		: ISDN facility
 *		Acu_clear_in_display_size,
 *		Acu_clear_in_a_display		: ISDN display Information
 *		Acu_clear_in_pcs_user_size,
 *		Acu_clear_in_a_pcs_user		: ISDN PCS-to-User/User-to-PCS Info.
 *		Acu_clear_in_signal_val		: signal value
 *
 *  (*) Only these fields are initialized by the Analog ACU, other fields are set to 0.
 *--------------------------------------------------------------------------*/

struct acu_clear_in_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	return_code;				/* clear cause code */
	uchar	network_cause;				/* Network-provided clear cause value */
	struct acu_charging_info charging_info;	/* charging information */

	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* size of the ISDN User to User information field */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */
	uchar      signal_val;				/* signal value */
	struct acu_progress_ind progress_ind;/* progress indicator */
	uchar      notif_ind_present;		/* notification indicator present (ON/OFF) */
	uchar      notif_ind;				/* notification indicator description */
	uchar      notif_ind_ext_size;		/* notification-indicator extension (QSIG) */
	uchar      mlpp_circuit_reserved;	/* Q.955 MLPP: circuit reserved for reuse indicator: OFF:no, ON:yes */

										/* UUI is just here */
										/* then the facility */
										/* then the display strings */
										/* and finally the pcs_user */
};

#define	Acu_clear_in_					((struct acu_clear_in_args FAR *)p_data) ->
#define	Aki_							Acu_clear_in_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_clear_in_ext_parms_nb	Aki_ ext_descr.nb
#	define Acu_clear_in_ext_parms_lgth	Aki_ ext_descr.lgth
#	define Acu_clear_in_a_ext_parms		Aki_ ext_descr.address
#endif

#define	Acu_clear_in_ret_code			Aki_ return_code
#define	Acu_clear_in_network_cause		Aki_ network_cause

#define	Acu_clear_in_charging_available	Aki_ charging_info.available
#define Acu_clear_in_charging_value     Aki_ charging_info.value
#define Acu_clear_in_charging_type     	Aki_ charging_info.type
#define Acu_clear_in_charging_period    Aki_ charging_info.period
#define Acu_clear_in_charging_multi     Aki_ charging_info.multi
#define Acu_clear_in_charging_a_currency &(Aki_ charging_info.currency[0])
#define Acu_clear_in_charging	     	Acu_clear_in_charging_value	/* alias */
#define	Acu_clear_in_total_cost			Acu_clear_in_charging /* alias */

#define	Acu_clear_in_uui_protocol		Aki_ uui_protocol
#define	Acu_clear_in_uui_size			Aki_ uui_size
#define	Acu_clear_in_facility_size		Aki_ facility_size
#define	Acu_clear_in_display_size		Acu_clear_in_x_display_total_size
#define Acu_clear_in_pcs_user_size		Aki_ pcs_user_size
#define	Acu_clear_in_notif_ind_ext_size	Aki_ notif_ind_ext_size

#define Acu_clear_in_x_display_nb		Aki_ display.nb
#define Acu_clear_in_x_display_total_size Aki_ display.total_size
#define Acu_clear_in_x_display_offset(i) Aki_ display.element[i].string_offset
#define Acu_clear_in_x_p_display(i) 	Aki_ display.element[i].p_display
#define Acu_clear_in_x_display_type(i)	Aki_ display.element[i].type
#define Acu_clear_in_x_display_size(i)	Aki_ display.element[i].size
#define Acu_clear_in_a_display_list   	&(Aki_ display)

#define	Acu_clear_in_a_uui				((char FAR *)p_data + Acu_clear_in_size)
#define	Acu_clear_in_a_facility			(Acu_clear_in_a_uui + Acu_clear_in_uui_size)
#define	Acu_clear_in_a_display			(Acu_clear_in_a_facility + Acu_clear_in_facility_size)
#define Acu_clear_in_a_pcs_user			(Acu_clear_in_a_display  + Acu_clear_in_display_size)
#define	Acu_clear_in_a_notif_ind_ext	(Acu_clear_in_a_pcs_user+ Acu_clear_in_pcs_user_size + 1)

#define	Acu_clear_in_signal_val			Aki_ signal_val

#define	Acu_clear_in_progress_ind_nb	Aki_ progress_ind.nb
#define	Acu_clear_in_progress_location	Acu_clear_in_progress_loc
#define	Acu_clear_in_progress_description Acu_clear_in_progress_descr
	/* aliases */
#define	Acu_clear_in_progress_loc		Aki_ progress_ind.t[0].loc
#define	Acu_clear_in_progress_descr		Aki_ progress_ind.t[0].descr
	/* access to other occurencies */
#define	Acu_clear_in_progress_loc_x(i)	Aki_ progress_ind.t[i].loc
#define	Acu_clear_in_progress_descr_x(i) Aki_ progress_ind.t[i].descr

#define	Acu_clear_in_notif_ind_present	Aki_ notif_ind_present
#define	Acu_clear_in_notif_ind			Aki_ notif_ind

#define	Acu_clear_in_mlpp_circuit_reserved	Aki_ mlpp_circuit_reserved

#define Acu_clear_in_a_end_strings		(uchar FAR *)Round((ulong)Acu_clear_in_a_notif_ind_ext + Acu_clear_in_notif_ind_ext_size)

#define	Acu_clear_in_size				(Rnd_sizeof (struct acu_clear_in_args))

/*----------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_CLEAR_RS			call clearing response.
 *
 *	Arguments :
 *		Acu_clear_rs_priority   : primitive priority (ACU_PHIGH/ACU_PLOW)
 *		Acu_clear_rs_uui_size,
 *		Acu_clear_rs_a_uui		: ISDN user to User Information
 *		Acu_clear_rs_facility_size,
 *		Acu_clear_rs_facility	: facility
 *		Acu_clear_rs_pcs_user_size,
 *		Acu_clear_rs_a_pcs_user	: ISDN PCS-to-User/User-to-PCS Info.
 *
 *--------------------------------------------------------------------------*/

struct acu_clear_rs_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar      priority;				/* phone call priority */

	struct acu_charging_info charging_info;	/* charging information */
	uchar      notif_ind_present;		/* notification indicator present (ON/OFF) */
	uchar      notif_ind;				/* notification indicator description */

	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* size of the ISDN User to User information field */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

										/* UUI is just here */
										/* then the facility */
										/* then the display strings */
										/* and finally the pcs_user */
};

#define	Acu_clear_rs_					((struct acu_clear_rs_args FAR *)p_data) ->
#define	Aks_							Acu_clear_rs_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_clear_rs_ext_parms_nb	Aks_ ext_descr.nb
#	define Acu_clear_rs_ext_parms_lgth	Aks_ ext_descr.lgth
#	define Acu_clear_rs_a_ext_parms		Aks_ ext_descr.address
#endif

#define	Acu_clear_rs_priority			Aks_ priority

#define	Acu_clear_rs_charging_available	Aks_ charging_info.available
#define Acu_clear_rs_charging_value     Aks_ charging_info.value
#define Acu_clear_rs_charging_type     	Aks_ charging_info.type
#define Acu_clear_rs_charging_period    Aks_ charging_info.period
#define Acu_clear_rs_charging_multi     Aks_ charging_info.multi
#define Acu_clear_rs_charging_a_currency &(Aks_ charging_info.currency[0])
#define Acu_clear_rs_charging	     	Acu_clear_rs_charging_value	/* alias */

#define	Acu_clear_rs_notif_ind_present	Aks_ notif_ind_present
#define	Acu_clear_rs_notif_ind			Aks_ notif_ind

#define	Acu_clear_rs_uui_protocol		Aks_ uui_protocol
#define	Acu_clear_rs_uui_size			Aks_ uui_size
#define	Acu_clear_rs_facility_size		Aks_ facility_size
#define	Acu_clear_rs_display_size		Acu_clear_rs_x_display_total_size
#define Acu_clear_rs_pcs_user_size		Aks_ pcs_user_size

#define Acu_clear_rs_x_display_nb		Aks_ display.nb
#define Acu_clear_rs_x_display_total_size Aks_ display.total_size
#define Acu_clear_rs_x_display_offset(i) Aks_ display.element[i].string_offset
#define Acu_clear_rs_x_p_display(i) 	Aks_ display.element[i].p_display
#define Acu_clear_rs_x_display_type(i)	Aks_ display.element[i].type
#define Acu_clear_rs_x_display_size(i)	Aks_ display.element[i].size
#define Acu_clear_rs_a_display_list		&(Aks_ display)

#define	Acu_clear_rs_a_uui				((char FAR *)p_data + Acu_clear_rs_size)
#define	Acu_clear_rs_a_facility			(Acu_clear_rs_a_uui + Acu_clear_rs_uui_size + 1)
#define	Acu_clear_rs_a_display			(Acu_clear_rs_a_facility + Acu_clear_rs_facility_size + 1)
#define Acu_clear_rs_a_pcs_user			(Acu_clear_rs_a_display  + Acu_clear_rs_display_size  + 1)

#define Acu_clear_rs_a_end_strings		(uchar FAR *)Round((ulong)Acu_clear_rs_a_pcs_user + Acu_clear_rs_pcs_user_size + 1)

#define	Acu_clear_rs_size				(Rnd_sizeof (struct acu_clear_rs_args))

/*----------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_CLEAR_CO			call cleared confirmation.
 *
 *	Arguments :
 *	(*)	Acu_clear_co_ret_code 		: return code (ACURC_BUSY, etc., see ACU_CLEAR_IN)
 *	  	Acu_clear_co_network_cause	: clear cause, as provided by the network
 *		Acu_clear_co_charging		: charging (nb of units)
 *		Acu_clear_co_charging_type	: charging type
 *		Acu_clear_co_charging_period: charging period
 *		Acu_clear_co_charging_multi	: charging multiplier
 *		Acu_clear_co_total_cost     : alias for charging field
 *	   	Acu_clear_co_uui_size,
 *		Acu_clear_co_a_uui			: ISDN User to User Information
 *		Acu_clear_co_facility_size,
 *		Acu_clear_co_a_facility		: ISDN facility
 *		Acu_clear_co_display_size,
 *		Acu_clear_co_a_display		: ISDN display Information
 *		Acu_clear_co_pcs_user_size,
 *		Acu_clear_co_a_pcs_user		: ISDN PCS-to-User/User-to-PCS Info.
 *
 *	ARINC SPECIFIC :
 * 		Acu_clear_co_credit_card_status	: credit card status
 *
 *  (*) Only these fields are initialized by the Analog ACU, other fields are set to 0.
 *--------------------------------------------------------------------------*/

struct acu_clear_co_args {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	return_code;				/* clear cause code */
	uchar	network_cause;				/* Network-provided clear cause value */
	struct acu_charging_info charging_info;	/* charging information */
	uchar   notif_ind_present;			/* notification indicator present (ON/OFF) */
	uchar	notif_ind;					/* notification indicator description */

	uchar      uui_protocol;			/* UUI protocol-discriminator value */
	acu_size_t uui_size;				/* size of the ISDN User to User information field */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

#	if ARINC_NETWORKS == ON
		uchar	credit_card_status;		/* credit card status */
#	endif

										/* UUI is just here */
										/* then the facility */
										/* then the display strings */
										/* and finally the pcs_user */
};

#define	Acu_clear_co_					((struct acu_clear_co_args FAR *)p_data) ->
#define	Ako_							Acu_clear_co_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_clear_co_ext_parms_nb	Ako_ ext_descr.nb
#	define Acu_clear_co_ext_parms_lgth	Ako_ ext_descr.lgth
#	define Acu_clear_co_a_ext_parms		Ako_ ext_descr.address
#endif

#define	Acu_clear_co_ret_code			Ako_ return_code
#define	Acu_clear_co_network_cause		Ako_ network_cause

#define	Acu_clear_co_charging_available	Ako_ charging_info.available
#define Acu_clear_co_charging_value     Ako_ charging_info.value
#define Acu_clear_co_charging_type     	Ako_ charging_info.type
#define Acu_clear_co_charging_period    Ako_ charging_info.period
#define Acu_clear_co_charging_multi     Ako_ charging_info.multi
#define Acu_clear_co_charging_a_currency &(Ako_ charging_info.currency[0])
#define Acu_clear_co_charging	     	Acu_clear_co_charging_value	/* alias */
#define	Acu_clear_co_total_cost		 	Acu_clear_co_charging /* alias */

#define	Acu_clear_co_notif_ind_present	Ako_ notif_ind_present
#define	Acu_clear_co_notif_ind			Ako_ notif_ind

#define	Acu_clear_co_uui_protocol		Ako_ uui_protocol
#define	Acu_clear_co_uui_size			Ako_ uui_size
#define	Acu_clear_co_facility_size		Ako_ facility_size
#define	Acu_clear_co_display_size		Acu_clear_co_x_display_total_size
#define Acu_clear_co_pcs_user_size		Ako_ pcs_user_size

#define Acu_clear_co_x_display_nb		Ako_ display.nb
#define Acu_clear_co_x_display_total_size Ako_ display.total_size
#define Acu_clear_co_x_display_offset(i) Ako_ display.element[i].string_offset
#define Acu_clear_co_x_p_display(i) 	Ako_ display.element[i].p_display
#define Acu_clear_co_x_display_type(i)	Ako_ display.element[i].type
#define Acu_clear_co_x_display_size(i)	Ako_ display.element[i].size
#define Acu_clear_co_a_display_list   	&(Ako_ display)

#define	Acu_clear_co_a_uui				((char FAR *)p_data + Acu_clear_co_size)
#define	Acu_clear_co_a_facility			(Acu_clear_co_a_uui + Acu_clear_co_uui_size)
#define	Acu_clear_co_a_display			(Acu_clear_co_a_facility + Acu_clear_co_facility_size)
#define Acu_clear_co_a_pcs_user			(Acu_clear_co_a_display + Acu_clear_co_display_size)

#if ARINC_NETWORKS == ON
#	define Acu_clear_co_credit_card_status	Ako_ credit_card_status
#endif

#define Acu_clear_co_a_end_strings		(uchar FAR *)Round((ulong)Acu_clear_co_a_pcs_user + Acu_clear_co_pcs_user_size)

#define	Acu_clear_co_size				(Rnd_sizeof (struct acu_clear_co_args))

/*---------------------------------------------------------------------------
 *  Interface: Analog ACU
 *
 *	ACU_INFO_RQ		Get a Automatic Call Unit parameter
 *
 *	Arguments :
 *	(*)	Acu_info_rq_name : parameter reference (ACU_NRINGS, ACU_PULSE, etc)
 *
 *  (*) Only these fields are initialized by the Analog ACU.
 *--------------------------------------------------------------------------*/

struct acu_info_rq_args {
	uchar	name;		/* parameter name */
};

#define	Acu_info_rq_name			(((struct acu_info_rq_args FAR *)p_data) -> name)

#define	Acu_info_rq_size			(Rnd_sizeof (struct acu_info_rq_args))

/*---------------------------------------------------------------------------
 *  Interface: Analog ACU
 *
 *	ACU_INFO_CO		confirmation with informations.
 *
 *	Arguments :
 *	(*)	Acu_info_co_name	 : parameter name (ACU_NRINGS, ACU_PULSE, etc)
 *	(*)	Acu_info_co_value	 : parameter value (ushort).
 *
 *  (*) Only these fields are initialized by the Analog ACU.
 *--------------------------------------------------------------------------*/

struct acu_info_co_args {
	uchar			name;			/* parameter name */
	ushort			value;			/* parameter value */
	uchar			ratio;			/* pulse dialing break ratio, 0..100 */
	uchar			rate;			/* pulse dialing rate, 1..20 */
	uchar			p_map;			/* pulse dialing map, 0..2 */
	timer_val_t		interdigit;		/* pulse dialing interdigit duration */
};

#define	Acu_info_co_				((struct acu_info_co_args FAR *)p_data) ->
#define	Acu_info_co_name			Acu_info_co_ name
#define	Acu_info_co_value			Acu_info_co_ value

#define	Acu_info_co_ratio			Acu_info_co_ ratio
#define	Acu_info_co_rate			Acu_info_co_ rate
#define	Acu_info_co_p_map			Acu_info_co_ p_map
#define	Acu_info_co_inter_digit		Acu_info_co_ interdigit

#define Acu_info_co_size			(Rnd_sizeof (struct acu_info_co_args))

/*---------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_INIT_RQ			initialize a 'primitive path'.
 *
 *	Arguments :
 *	(*)	Acu_init_rq_priority 	: primitive priority normal/urgent
 *	(*)	Acu_init_rq_new_user 	: name of the new user entity on the Network Access
 *		Acu_init_rq_new_sapi 	: name of the new sapi on the Network Access
 *
 *  (*) Only these fields are used by the Analog ACU, other fields are ignored.
 *--------------------------------------------------------------------------*/

struct acu_init_rq_args {
	uchar			priority;		/* primitive priority */
	ent_id_t		new_user;		/* new user entity */
	sapi_t			new_sapi;		/* new sapi */
};

#define	Acu_init_rq_				((struct acu_init_rq_args FAR *)p_data) ->
#define	Acu_init_rq_priority		Acu_init_rq_ priority
#define	Acu_init_rq_new_user		Acu_init_rq_ new_user
#define	Acu_init_rq_new_sapi		Acu_init_rq_ new_sapi

#define	Acu_init_rq_size			(Rnd_sizeof (struct acu_init_rq_args))

/*---------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_INIT_CO		'primitive path' initialization confirmation
 *
 *	Arguments :
 *	(*)	Acu_init_co_ret_code	: return code (OK/NOK)
 *      Acu_init_co_diagnostic	: diagnostic
 *
 *  (*) Only these fields are initialied by the Analog ACU, other fields are ignored.
 *--------------------------------------------------------------------------*/

struct acu_init_co_args {
	struct confirm 		confirm;	/* confirmation structure */
};

#define	Acu_init_co_				((struct acu_init_co_args FAR *)p_data) ->
#define	Acu_init_co_ret_code		Acu_init_co_ confirm.return_code
#define	Acu_init_co_diagnostic		Acu_init_co_ confirm.diagnostic

#define	Acu_init_co_size			(Rnd_sizeof (struct acu_init_co_args))

/*---------------------------------------------------------------------------
 *  Interface: Isdn / Analog ACU
 *
 *	ACU_SETPARM_RQ		Set a Automatic Call Unit parameter (Analog ACU only).
 *
 *	Arguments :
 *	(*)	Acu_setparm_rq_name		: parameter name (ACU_NRINGS, ACU_TPULSE, etc).
 *	(*)	Acu_setparm_rq_value	: parameter value (ushort)
 *
 *  (*) Only these fields are used by the Analog ACU.
 *--------------------------------------------------------------------------*/

struct acu_setparm_rq_args {
	uchar			name;			/* parameter name */
	ushort			value;			/* parameter value */
	uchar			ratio;			/* pulse dialing break ratio, 0..100 */
	uchar			rate;			/* pulse dialing rate, 1..20 */
	uchar			p_map;			/* pulse dialing map, 0..2 */
	timer_val_t		inter_digit;	/* pulse dialing interdigit duration */
};

#define	Acu_setparm_rq_				((struct acu_setparm_rq_args FAR *)p_data) ->
#define	Acu_setparm_rq_name			Acu_setparm_rq_ name
#define	Acu_setparm_rq_value		Acu_setparm_rq_ value
#define	Acu_setparm_rq_ratio		Acu_setparm_rq_ ratio
#define	Acu_setparm_rq_rate			Acu_setparm_rq_ rate
#define	Acu_setparm_rq_p_map		Acu_setparm_rq_ p_map
#define	Acu_setparm_rq_inter_digit	Acu_setparm_rq_ inter_digit

#define	Acu_setparm_rq_size			(Rnd_sizeof (struct acu_setparm_rq_args))

/*---------------------------------------------------------------------------
 *  Interface: Analog ACU
 *
 *	ACU_SETPARM_CO		confirm that parameter has been set.
 *
 *  Arguments : none
 *
 *--------------------------------------------------------------------------*/

struct acu_setparm_co_args {
	uchar	dummy;
};

#define	Acu_setparm_co_size			(Rnd_sizeof (struct acu_setparm_co_args))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *  ACU_SET_MODE_RQ/CO  request/confirm setting of a particular mode.
 *
 *  (uses the ACU_SAPI_MGT sapi) (ISDN only).
 *
 *  Arguments:
 *
 *  ACU_SET_MODE_RQ:
 *
 *      Acu_set_mode_action = { ACU_ACTION_ACTIVATE, ACU_ACTION_CLEAR, ACU_ACTION_ENQUIRY }
 *
 *      Acu_set_mode_code   = {
 *
 *          - ACU_MODE_ALL_NA_OUT_OF_ORDER: allow/forbid calls on all NAIs.
 *
 *          - ACU_MODE_NA_OUT_OF_ORDER: allow/forbid calls on NAI given by
 *              R_msg_nai.
 *
 *          - ACU_MODE_CALL_OUT_OF_ORDER: allow/forbid calls on NAI/CONN_ID
 *              given by R_msg_nai/R_msg_conn_id.
 *
 *          - ACU_MODE_CHANI_OUT_OF_ORDER: allow/forbid usage of the
 *              B-channel given by Acu_set_mode_data_chani.
 *
 *  ACU_SET_MODE_CO:
 *
 *      Acu_set_mode_action: { ACU_ACTION_ACK, ACU_ACTION_REJ }
 *
 *      - ACU_MODE_CHANI_OUT_OF_ORDER:
 *          - for ACU_ACTION_ACTIVATE request: the channels listed in ACU_SET_MODE_CO
 *            are the ones that could not be disabled, either because they were invalid,
 *            or because they were still in use by a call;
 *            Acu_set_mode_data_chani_nb is set to 0 when all requested channels are disabled.
 *          - for ACU_ACTION_CLEAR request: the channels listed in ACU_SET_MODE_CO
 *            are the ones that could not be enabled, because they were invalid;
 *            Acu_set_mode_data_chani_nb is set to 0 when all requested channels are enabled.
 *          - for ACU_ACTION_ENQUIRY request: the channels listed in ACU_SET_MODE_CO
 *            are the ones that are disabled;
 *            Acu_set_mode_data_chani_nb is set to 0 when all requested channels are enabled.
 *
 *--------------------------------------------------------------------------*/

struct acu_set_mode {
	uchar 	action;
	uchar	code;

	struct chani_list   chani_list;		/* list of channel ids -- for ACU_MODE_CHANI_OUT_OF_ORDER code only */
};

#define	Acu_set_mode_					((struct acu_set_mode FAR *)p_data) ->
#define	Acu_set_mode_action				Acu_set_mode_ action
#define	Acu_set_mode_code				Acu_set_mode_ code

#define Acu_set_mode_data_chani_nb		Acu_set_mode_ chani_list.nb
#define Acu_set_mode_data_chani			Acu_set_mode_ chani_list.tab[0].chan_id
#define Acu_set_mode_data_chani_tab(i)	Acu_set_mode_ chani_list.tab[i].chan_id

#define	Acu_set_mode_size				(Rnd_sizeof (struct acu_set_mode))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_SUSPEND_RQ/CO	Suspend a call (TE-side)
 *	ACU_SUSPEND_IN/RS	Suspend a call (NT-side)
 *
 *--------------------------------------------------------------------------*/

struct acu_suspend {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	return_code;				/* return code: OK or NOK (_RS/CO only) */
	uchar	network_cause;				/* Network-provided reject cause value (_RS/CO only) */
	uchar   notif_ind_present;			/* notification indicator present (ON/OFF) (_RS/_CO only) */
	uchar	notif_ind;					/* notification indicator description (_RS/_CO only) */
	char	call_id[ACU_MX_SZ_CALL_ID+1];/* call identity (_RQ/_IN only) */
	struct acu_display_list display;	/* display list (_RS/_CO only) */

										/* display strings are just here */
};

#define	Acu_suspend_					((struct acu_suspend FAR *)p_data) ->

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_suspend_ext_parms_nb		Acu_suspend_ ext_descr.nb
#	define Acu_suspend_ext_parms_lgth	Acu_suspend_ ext_descr.lgth
#	define Acu_suspend_a_ext_parms		Acu_suspend_ ext_descr.address
#endif

#define	Acu_suspend_ret_code			Acu_suspend_ return_code
#define	Acu_suspend_network_cause		Acu_suspend_ network_cause
#define	Acu_suspend_notif_ind_present	Acu_suspend_ notif_ind_present
#define	Acu_suspend_notif_ind			Acu_suspend_ notif_ind
#define	Acu_suspend_a_call_id	   		Acu_suspend_ call_id

#define	Acu_suspend_display_size		Acu_suspend_x_display_total_size

#define Acu_suspend_x_display_nb		Acu_suspend_ display.nb
#define Acu_suspend_x_display_total_size Acu_suspend_ display.total_size
#define Acu_suspend_x_display_offset(i) Acu_suspend_ display.element[i].string_offset
#define Acu_suspend_x_p_display(i) 		Acu_suspend_ display.element[i].p_display
#define Acu_suspend_x_display_type(i)	Acu_suspend_ display.element[i].type
#define Acu_suspend_x_display_size(i)	Acu_suspend_ display.element[i].size
#define Acu_suspend_a_display_list   	&(Acu_suspend_ display)

#define	Acu_suspend_a_display			((char FAR *)p_data + Acu_suspend_size)

#define Acu_suspend_a_first_other		(Acu_suspend_a_display + Acu_suspend_display_size + 1)
#define Acu_suspend_a_end_strings		(uchar FAR *)Round((ulong)Acu_suspend_a_first_other)

#define	Acu_suspend_size				(Rnd_sizeof (struct acu_suspend))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_RESUME_RQ/CO	Resume a call (TE-side)
 *	ACU_RESUME_IN/RS	Resume a call (NT-side)
 *
 *--------------------------------------------------------------------------*/

struct acu_resume {
	uchar	return_code;				/* return code: OK or NOK (_RS/CO only) */
	uchar	network_cause;				/* Network-provided reject cause value (_RS/CO only) */
	uchar	service;					/* service to be used for (_RQ/_IN only) */
	uchar	data_chani;					/* B channel id used (_RS/_CO only) */
	uchar	chan_id_excl;				/* chan_id preferred (OFF) or exclusive (ON) (_RS/_CO only) */
	uchar   notif_ind_present;			/* notification indicator present (ON/OFF) (_RS/_CO only) */
	uchar	notif_ind;					/* notification indicator description (_RS/_CO only) */
	add_t	ces;						/* DL-Connection identifier for that call (_IN/_CO only) */
	char	call_id[ACU_MX_SZ_CALL_ID+1];/* call identity (_RQ/_IN only) */
	struct acu_display_list display;	/* display list (_RS/_CO only) */

										/* display strings are just here */
};

#define	Acu_resume_						((struct acu_resume FAR *)p_data) ->

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_resume_ext_parms_nb		Acu_resume_ ext_descr.nb
#	define Acu_resume_ext_parms_lgth	Acu_resume_ ext_descr.lgth
#	define Acu_resume_a_ext_parms		Acu_resume_ ext_descr.address
#endif

#define	Acu_resume_ret_code				Acu_resume_ return_code
#define	Acu_resume_network_cause		Acu_resume_ network_cause
#define	Acu_resume_service				Acu_resume_ service
#define	Acu_resume_data_chani			Acu_resume_ data_chani
#define Acu_resume_data_chani_excl  	Acu_resume_ chan_id_excl
#define	Acu_resume_notif_ind_present	Acu_resume_ notif_ind_present
#define	Acu_resume_notif_ind			Acu_resume_ notif_ind
#define	Acu_resume_ces					Acu_resume_ ces
#define	Acu_resume_a_call_id	    	Acu_resume_ call_id
#define	Acu_resume_display_size			Acu_resume_x_display_total_size

#define Acu_resume_x_display_nb			Acu_resume_ display.nb
#define Acu_resume_x_display_total_size Acu_resume_ display.total_size
#define Acu_resume_x_display_offset(i)	Acu_resume_ display.element[i].string_offset
#define Acu_resume_x_p_display(i) 		Acu_resume_ display.element[i].p_display
#define Acu_resume_x_display_type(i)	Acu_resume_ display.element[i].type
#define Acu_resume_x_display_size(i)	Acu_resume_ display.element[i].size
#define Acu_resume_a_display_list   	&(Acu_resume_ display)

#define	Acu_resume_a_display			((char FAR *)p_data + Acu_resume_size)

#define Acu_resume_a_first_other		(Acu_resume_a_display + Acu_resume_display_size + 1)
#define Acu_resume_a_end_strings		(uchar FAR *)Round((ulong)Acu_resume_a_first_other)

#define	Acu_resume_size					(Rnd_sizeof (struct acu_resume))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_FACILITY_RQ/IN/RS/CO	Facility.
 *
 *--------------------------------------------------------------------------*/

struct acu_facility {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	service;					/* service id for CFU Activation/Deactivation/Enquiry (request+result) */

	uchar 	action;						/* action code: ACU_ACTION_... */
	uchar	code;						/* facility code: ACU_FAC_... */
	uchar	network_cause;				/* Network-provided facility reject cause value */
	uchar	notif_ind_present;			/* notification indicator present (ON/OFF) */
	uchar	notif_ind;					/* notification indicator description */

	char	called_address[ACU_MX_SZ_CALLED_NB + 1 + ACU_MX_SZ_CALLED_SUB_ADD + 1];	/* called address */
	acu_size_t facility_size;			/* size of the facility */
	struct acu_calling_nb calling_nb;	/* calling nb information */
	struct acu_calling_nb calling_nb2;	/* second calling nb information */
	struct acu_redir_nb   redir_nb;		/* redirecting nb information */
	struct acu_display_list display;	/* display list */
	acu_size_t pcs_user_size;			/* size of the PCS-to-User or User-to-PCS information */

	struct voice_supp_service voice;	/* voice supplementary services management */
	uchar	data_chani;					/* B channel to be used */

	nai_t	other_nai;					/* NAI value of the other PRI for TBCT Supp Service */
	add_t	other_conn_id;				/* CONN_ID value of the other call for TBCT Supp Service */

										/* facility is just here */
										/* then the calling nb */
										/* then the second calling nb */
										/* then the display strings */
										/* and finally the pcs_user */
};

#define	Af_								((struct acu_facility FAR *)p_data) ->

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_facility_ext_parms_nb	Af_ ext_descr.nb
#	define Acu_facility_ext_parms_lgth	Af_ ext_descr.lgth
#	define Acu_facility_a_ext_parms		Af_ ext_descr.address
#endif

#define	Acu_facility_service			Af_ service
#define	Acu_facility_action				Af_ action
#define	Acu_facility_code				Af_ code
#define	Acu_facility_network_cause		Af_ network_cause
#define	Acu_facility_notif_ind_present	Af_ notif_ind_present
#define	Acu_facility_notif_ind			Af_ notif_ind
#define	Acu_facility_a_called_address	(&(Af_ called_address[0]))
#define	Acu_facility_data_chani			Af_ data_chani
#define	Acu_facility_other_nai			Af_ other_nai
#define	Acu_facility_other_conn_id		Af_ other_conn_id

#define	Acu_facility_calling_nb_type	Af_ calling_nb.type
#define	Acu_facility_calling_nb_plan	Af_ calling_nb.plan
#define	Acu_facility_calling_nb_pres	Af_ calling_nb.pres
#define	Acu_facility_calling_nb_screen	Af_ calling_nb.screen

#define	Acu_facility_calling_nb2_type	Af_ calling_nb2.type
#define	Acu_facility_calling_nb2_plan	Af_ calling_nb2.plan
#define	Acu_facility_calling_nb2_pres	Af_ calling_nb2.pres
#define	Acu_facility_calling_nb2_screen	Af_ calling_nb2.screen

#define	Acu_facility_redir_nb_pres		Af_ redir_nb.pres
#define	Acu_facility_redir_nb_type		Af_ redir_nb.type
#define	Acu_facility_redir_nb_plan		Af_ redir_nb.plan

#define	Acu_facility_facility_size		Af_ facility_size
#define	Acu_facility_calling_nb_size	Af_ calling_nb.size
#define	Acu_facility_calling_nb2_size	Af_ calling_nb2.size
#define	Acu_facility_redir_nb_size		Af_ redir_nb.size
#define	Acu_facility_display_size		Acu_facility_x_display_total_size
#define Acu_facility_pcs_user_size		Af_ pcs_user_size

#define Acu_facility_x_display_nb		Af_ display.nb
#define Acu_facility_x_display_total_size Af_ display.total_size
#define Acu_facility_x_display_offset(i) Af_ display.element[i].string_offset
#define Acu_facility_x_p_display(i) 	Af_ display.element[i].p_display
#define Acu_facility_x_display_type(i)	Af_ display.element[i].type
#define Acu_facility_x_display_size(i)	Af_ display.element[i].size
#define Acu_facility_a_display_list   	&(Af_ display)

#define	Acu_facility_a_facility			((char FAR *)p_data + Acu_facility_size)
#define	Acu_facility_a_calling_nb		(Acu_facility_a_facility + Acu_facility_facility_size + 1)
#define	Acu_facility_a_calling_nb2		(Acu_facility_a_calling_nb + Acu_facility_calling_nb_size + 1)
#define	Acu_facility_a_redir_nb			(Acu_facility_a_calling_nb2 + Acu_facility_calling_nb2_size + 1)
#define	Acu_facility_a_display			(Acu_facility_a_redir_nb + Acu_facility_redir_nb_size + 1)
#define Acu_facility_a_pcs_user			(Acu_facility_a_display + Acu_facility_display_size + 1)

#define	Acu_facility_button_type		Af_ voice.button_type
#define	Acu_facility_module_nb			Af_ voice.module_nb
#define	Acu_facility_button_nb			Af_ voice.button_nb
#define	Acu_facility_call_appear		Af_ voice.call_appear
#define	Acu_facility_switchhook			Af_ voice.switchhook
#define	Acu_facility_conn_id			Af_ voice.conn_id

#define Acu_facility_a_first_other		(Acu_facility_a_pcs_user + Acu_facility_pcs_user_size + 1)

#define Acu_facility_a_end_strings		(uchar FAR *)Round((ulong)Acu_facility_a_first_other)

#define	Acu_facility_size				(Rnd_sizeof (struct acu_facility))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_TEST_RQ/CO	:	(with ACU_SAPI_MGT)
 *
 * Input fields (in ACU_TEST_RQ):
 * ------------------------------
 * Acu_test_action: action to be applied:
 *    ACU_ACTION_ACTIVATE: activate the test indicated in 'Acu_test_code';
 *    ACU_ACTION_CLEAR:    clear the test indicated in 'Acu_test_code';
 *    ACU_ACTION_ENQUIRY:  ask action state (activated or cleared);
 *
 * Acu_test_code values:
 *    ACU_TEST_B_CHAN_LOOP_BACK: loopback the B channel for Physical tests;
 *    ACU_TEST_B_CHAN_SWITCH:    switch the B-channel ON or OFF;
 *
 * Acu_test_b_chan or Acu_test_b_chan_loop_back_nb:
 *    B-channel on which to apply the action;
 *
 * Acu_test_b_chan_mode: mode to be applied to the B-channel (ACU_TEST_B_CHAN_SWITCH only)
 * (requires to include "ph_int.h" along with "acu_int.h")
 *    B_MODE_VOICE:     voice mode
 *    B_MODE_AUDIO:     audio mode
 *    B_MODE_DATA_HDLC: data mode
 *    B_MODE_DATA_V110: V.110 mode
 *
 * Output fields (in ACU_TEST_CO):
 * -------------------------------
 * Acu_test_action: action status: success or failure:
 *    ACU_ACTION_ACK: the requested action succeeded;
 *    ACU_ACTION_REJ: the requested action failed;
 *
 * Acu_test_response_code: response for the requested action (ACU_TEST_B_CHAN_LOOP_BACK only):
 *        ON:  B-chan is looped back;     Acu_test_b_chan_mode is set to B_MODE_LOOP_BACK_ON;
 *        OFF: B-chan is not looped back; Acu_test_b_chan_mode is set to B_MODE_NIL;
 *
 * Acu_test_b_chan or Acu_test_b_chan_loop_back_nb: echoed from ACU_TEST_RQ;
 *
 * Acu_test_b_chan_mode: returns the new B-channel mode;
 *
 *--------------------------------------------------------------------------*/

struct acu_test {
	uchar 	action;						/* Action code */
	uchar	code;						/* test code: ACU_TEST_* */
	uchar	response_code;				/* response code */
	uchar	b_chan_nb;					/* B-channel id */
	uchar	b_chan_mode;				/* B-channel mode (ACU_TEST_B_CHAN_SWITCH only) */
};

#define	Acu_test_						((struct acu_test FAR *)p_data) ->
#define	Acu_test_action					Acu_test_ action
#define	Acu_test_code					Acu_test_ code
#define	Acu_test_response_code			Acu_test_ response_code
#define	Acu_test_b_chan					Acu_test_ b_chan_nb
#define	Acu_test_b_chan_loop_back_nb	Acu_test_ b_chan_nb
#define	Acu_test_b_chan_mode			Acu_test_ b_chan_mode

#define	Acu_test_size					(Rnd_sizeof (struct acu_test))

	/* Acu_test_code values */
#define ACU_TEST_B_CHAN_LOOP_BACK		'L'	/* loop back B channel */
#define ACU_TEST_B_CHAN_SWITCH			'S'	/* switch B channel ON or OFF */

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_INFORMATION_RQ/IN	Facility
 * Arguments :
 *
 * 	Acu_information_charging		 		: charging
 * 	Acu_information_end_of_three_party		: end-of-three-party
 * 	Acu_information_display_size			: display size
 * 	Acu_information_keypad_size				: keypad size
 *	Acu_information_ces						: DL-Connection identifier for that call (ACU_INFORMATION_IN only)
 *	Acu_information_signal_val				: signal value
 *
 *	ARINC SPECIFIC :
 * 		Acu_information_credit_card_status	: credit card status
 * 		Acu_information_nats_avail			: nats_avail
 * 		Acu_information_satcom_avail		: satcom_avail
 * 		Acu_information_tfts_avail			: tfts_avail
 * 		Acu_information_nats_pos			: nats_position
 * 		Acu_information_satcom_pos			: satcom_position
 * 		Acu_information_tfts_pos			: tfts_position
 *		Acu_information_track_1_size		: size of the track #1
 *		Acu_information_track_2_size        : size of the track #2
 *
 *	US-BRI Network side specific :
 * 		Acu_information_ces					; Connection Endpoint Suffix on which the SPID was received. (_IN)
 * 		Acu_information_tid					: Terminal Endpoint Id (_RQ)
 * 		Acu_information_usid				: User Id (_RQ)
 * 		Acu_information_interpreter			: Interpreter (_RQ)
 *--------------------------------------------------------------------------*/

struct acu_information {

#	if ACU_PAR_EXTENDED_PRIM == ON
		struct acu_ext_descr ext_descr;	/* extended parameters descriptor */
#	endif

	uchar	end_of_three_party;				/* end of three party : ON or OFF */
	struct acu_charging_info charging_info;	/* charging information */

	struct acu_display_list display;		/* display list */
	acu_size_t keypad_size;					/* keypad size */

	add_t	ces;							/* DL-Connection identifier for that call (ACU_INFORMATION_IN only) */
	uchar	signal_val;						/* signal value */
	uchar   notif_ind_present;				/* notification indicator present (ON/OFF) */
	uchar	notif_ind;						/* notification indicator description */

#	if ARINC_NETWORKS == ON
		uchar	credit_card_status;			/* credit card status */
		uchar	nats_avail;					/* nats bearer system types available */
		uchar	satcom_avail;				/* satcom bearer system types available */
		uchar	tfts_avail;					/* tfts bearer system types available */
		uchar	nats_position; 				/* queue position of nats */
		uchar	satcom_position;			/* queue position of satcom */
		uchar	tfts_position; 				/* queue position of tfts */
		acu_size_t track_1_size;			/* information of track 1 */
		acu_size_t track_2_size;			/* information of track 2 */
#	endif

#	if _CC_SPID_INFO == ON
		struct 	acu_endpoint_id endpoint_id;/* Endpoint Id information */
		uchar	spid_size;					/* SPID size */
		uchar	cause;						/* cause value, provided by the NT when SPID negociation fails */

#	endif

#	if EQUIPMENT != TE
		  uchar	call_appear;				/* Call appearence */
#	endif
											/* display strings are just here */
											/* then the keypad */
											/* then the track 1 info */
											/* then the track 2 info */
											/* then the SPID info */

};

#define	Acu_information_					((struct acu_information FAR *)p_data) ->
#define	Ai_									Acu_information_

#if ACU_PAR_EXTENDED_PRIM == ON
#	define Acu_information_ext_parms_nb		Ai_ ext_descr.nb
#	define Acu_information_ext_parms_lgth	Ai_ ext_descr.lgth
#	define Acu_information_a_ext_parms		Ai_ ext_descr.address
#endif

#define	Acu_information_end_of_three_party	Ai_ end_of_three_party

#define	Acu_information_charging_available	Ai_ charging_info.available
#define Acu_information_charging_value     	Ai_ charging_info.value
#define Acu_information_charging_type     	Ai_ charging_info.type
#define Acu_information_charging_period    	Ai_ charging_info.period
#define Acu_information_charging_multi     	Ai_ charging_info.multi
#define Acu_information_charging_a_currency &(Ai_ charging_info.currency[0])
#define Acu_information_charging	     	Acu_information_charging_value	/* alias */

#define	Acu_information_display_size		Acu_information_x_display_total_size
#define	Acu_information_keypad_size			Ai_ keypad_size

#define Acu_information_x_display_nb		Ai_ display.nb
#define Acu_information_x_display_total_size Ai_ display.total_size
#define Acu_information_x_display_offset(i) Ai_ display.element[i].string_offset
#define Acu_information_x_p_display(i) 		Ai_ display.element[i].p_display
#define Acu_information_x_display_type(i)	Ai_ display.element[i].type
#define Acu_information_x_display_size(i)	Ai_ display.element[i].size
#define Acu_information_a_display_list   	&(Ai_ display)
#define	Acu_information_a_display			((char FAR *)p_data + Acu_information_size)
#define	Acu_information_a_keypad			(Acu_information_a_display + Acu_information_display_size + 1)

#define	Acu_information_ces					Ai_ ces
#define	Acu_information_signal_val			Ai_ signal_val
#define	Acu_information_notif_ind_present	Ai_ notif_ind_present
#define	Acu_information_notif_ind			Ai_ notif_ind

#if ARINC_NETWORKS == ON
#	define	Acu_information_notif_desc		Ai_ notif_ind
#	define	Acu_information_credit_card_status	Ai_ credit_card_status
#	define	Acu_information_nats_avail		Ai_ nats_avail
#	define	Acu_information_satcom_avail	Ai_ satcom_avail
#	define	Acu_information_tfts_avail		Ai_ tfts_avail
#	define	Acu_information_nats_pos		Ai_ nats_position
#	define	Acu_information_satcom_pos		Ai_ satcom_position
#	define	Acu_information_tfts_pos		Ai_ tfts_position
#	define	Acu_information_track_1_size	Ai_ track_1_size
#	define	Acu_information_track_2_size	Ai_ track_2_size

#	define	Acu_information_a_track_1		((char FAR *)p_data       + Acu_information_size)
#	define	Acu_information_a_track_2		(Acu_information_a_track_1+ Acu_information_track_1_size + 1)

#	if _CC_SPID_INFO == ON
#		define	Acu_information_spid_size	Ai_ spid_size
#		define	Acu_information_a_spid		(Acu_information_a_track_2 + Acu_information_track_2_size + 1)
#	endif

#else
		/* ARINC_NETWORKS OFF */

#	if _CC_SPID_INFO == ON
#		define	Acu_information_spid_size	Ai_ spid_size
#		define	Acu_information_a_spid		(Acu_information_a_keypad + Acu_information_keypad_size + 1)
#	endif

#endif

#if _CC_SPID_INFO == ON
#	define	Acu_information_p_endpoint_id	Ai_ endpoint_id
#	define	Acu_information_tid				Ai_ endpoint_id.tid
#	define	Acu_information_usid	   		Ai_ endpoint_id.usid
#	define	Acu_information_interpreter		Ai_ endpoint_id.interpreter
#	define	Acu_information_endpoint_action	Ai_ endpoint_id.action
#	define	Acu_information_cause			Ai_ cause
#endif

#if EQUIPMENT != TE
#	define	Acu_information_call_appear		Ai_ call_appear
#endif

#define Acu_information_a_first_other		(Acu_information_a_keypad + Acu_information_keypad_size + 1)

#define Acu_information_a_end_strings		(uchar FAR *)Round((ulong)Acu_information_a_first_other)

#define	Acu_information_size				(Rnd_sizeof (struct acu_information))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_NOTIFY_RQ/IN	Notify message.
 *
 *--------------------------------------------------------------------------*/

struct acu_notify {
	uchar   notif_ind_present;			/* notification indicator present (ON/OFF) */
	uchar	notif_ind;					/* notification indicator description */
	uchar	notif_ind_ext_size;			/* notification-indicator extension (QSIG) */
	struct acu_display_list display;	/* display list */
	struct acu_redir_nb   redir_nb;		/* redirection nb information */

										/* display strings are just here */
};

#define	Acu_notify_						((struct acu_notify FAR *)p_data) ->
#define Anf_							Acu_notify_

#define	Acu_notify_notif_ind_present	Anf_ notif_ind_present
#define	Acu_notify_notif_ind			Anf_ notif_ind
#define	Acu_notify_notif_ind_ext_size	Anf_ notif_ind_ext_size

#define	Acu_notify_redir_nb_pres		Anf_ redir_nb.pres
#define	Acu_notify_redir_nb_type		Anf_ redir_nb.type
#define	Acu_notify_redir_nb_plan		Anf_ redir_nb.plan

#define	Acu_notify_display_size			Acu_notify_x_display_total_size
#define	Acu_notify_redir_nb_size		Anf_ redir_nb.size

#define Acu_notify_x_display_nb			Anf_ display.nb
#define Acu_notify_x_display_total_size Anf_ display.total_size
#define Acu_notify_x_display_offset(i)	Anf_ display.element[i].string_offset
#define Acu_notify_x_p_display(i) 		Anf_ display.element[i].p_display
#define Acu_notify_x_display_type(i)	Anf_ display.element[i].type
#define Acu_notify_x_display_size(i)	Anf_ display.element[i].size
#define Acu_notify_a_display_list   	&(Anf_ display)

#define	Acu_notify_a_notif_ind_ext		((char FAR *)p_data + Acu_notify_size)
#define	Acu_notify_a_display			(Acu_notify_a_notif_ind_ext+ Acu_notify_notif_ind_ext_size + 1)
#define	Acu_notify_a_redir_nb			(Acu_notify_a_display + Acu_notify_display_size + 1)

#define Acu_notify_a_first_other		(Acu_notify_a_redir_nb + Acu_notify_redir_nb_size + 1)

#define Acu_notify_a_end_strings		(uchar FAR *)Round((ulong)Acu_notify_a_first_other)

#define	Acu_notify_size					(Rnd_sizeof (struct acu_notify))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_ERR_IN	Error indication.
 *
 *	Arguments:
 *
 *		Acu_err_in_cause:      error cause;
 *		Acu_err_in_diagnostic: error diagnostic;
 *
 *		See "DATA TYPES" section for symbolic values used in these fields.
 *
 *--------------------------------------------------------------------------*/

struct acu_err_in {
	uchar	cause;					/* error cause */
	uchar	diagnostic;				/* diagnostic */
};

#define	Acu_err_in_					((struct acu_err_in FAR *)p_data) ->
#define	Acu_err_in_cause			Acu_err_in_ cause
#define	Acu_err_in_diagnostic		Acu_err_in_ diagnostic

#define	Acu_err_in_size				(Rnd_sizeof (struct acu_err_in))

/*---------------------------------------------------------------------------
 *  Interface: Isdn
 *
 *	ACU_PDU_RQ	Request sending of a raw Q.931 PDU to NS.
 *
 *	No particular structure is defined for this primitive.
 *	The data area of the primitive must contain the raw Q.931 PDU,
 *	starting with the Q.931 message-type field (eg 0x05 for a SETUP PDU),
 *	and followed by the Q.931-coded Information Elements.
 *	NS will automatically add the Protocol-discriminator and the
 *	Call-Reference fields accoring to the message's conn_id value.
 *
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 *		MISCELLANEOUS DATA TYPES AND CONSTANTS EXCHANGED IN PRIMITIVES
 *-------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 *	ACU parameters names (ACU_SETPARM_RQ, ACU_INFO_RQ)
 *	Not used in ISDN interface
 *-------------------------------------------------------------------------*/

#define	ACU_NRINGS						'a'	/* number of rings before answering (ATS0) */
#define	ACU_TPULSE						'b'	/* tone or pulse dialing (ATD, ATP) */
#define	ACU_WB4_BLINDDIAL				'c'	/* delay to wait before blind dialing (AT S6) */
#define	ACU_CONN_DEL					'd'	/* max delay between the completion of dialing */
											/* and the establishment of the connection (AT S7) */
#define	ACU_PAUSE_DEL					'e'	/* pause duration (AT S8) */
#define	ACU_SIGDIGIT_DUR				'f'	/* DTMF duration (AT S11) */
#define	ACU_NRINGS_DET					'g'	/* Number of rings detected (AT S1) */
#define	ACU_PULSE_PARM					'h'	/* ACU pulse parameters */
#define	ACU_DIAL_NO_PROCEED				'i'	/* dial without waiting for proceed tone (AT X0) */
#define	ACU_DIAL_PROCEED				'j'	/* dial, waiting for proceed tone (AT X4, etc.) */
#define	ACU_PROCEED_DEL					'k'	/* maximum delay to wait for proceed tone (dial tone) before rejecting the outgoing call (ATS14, S014) */
#define	ACU_FLASH_HOOK_DEL				'l'	/* flash hook duration (ATS29?, S029). 0 means: flash hook is not implemented.*/
#if ACU_PSTN_CLIP == ON
#   define ACU_CLIP   			        'm'	/* enable / disable caller id detection (if caller id is disabled then only Ring detection is performed) */
                                            /* field "value" = ON or OFF.                                                                            */
#endif

/*--------------------------------------------------------------------------
 *	Primitive priority (ACU_CONN_RQ/RS, ACU_CLEAR_RQ/RS, ACU_INIT_RQ)
 *	Not used in ISDN interface
 *-------------------------------------------------------------------------*/

#define	ACU_PHIGH						'H'	/* urgent */
#define	ACU_PLOW						'L'	/* normal */

/*--------------------------------------------------------------------------
 *	Incoming or outgoing (tele)service (ACU_CONN_IN/RQ/RS)
 *-------------------------------------------------------------------------*/

#define	ACU_FAX_SERVICE					'F'	/* call for G3 facsimile service */
#define	ACU_FAX_4_SERVICE				'4'	/* call for G4 facsimile service */
#define	ACU_DATA_SERVICE				'D'	/* call for 'data' service */
#define	ACU_DATA_GCI_SERVICE			ACU_DATA_SERVICE	/* call for 'data' service on GCI bus */
#define	ACU_DATA_56KBS_SERVICE			'K'	/* call for 'data at 56 KBS' service */
#define	ACU_RAW_DATA_SERVICE			'!' /* call for 'raw data' service on GCI bus: no MPH_B_INIT_RQ is generated (i.e. no B-channel driver is associated) */
#define	ACU_DATA_TRANS_SERVICE			'T'	/* call for transparent data service */
#define	ACU_MODEM_SERVICE				'M'	/* call for modem data service */
#define	ACU_AUDIO_7_SERVICE				'7'	/* call for 7kHz Audio service */
#define	ACU_X25_SERVICE					'X'	/* call for X.25 circuit-mode service */
#define	ACU_X25_PACKET_SERVICE			'P'	/* call for X.25 packet-mode service */
#define	ACU_VOICE_SERVICE				'V'	/* call for voice service */
#define	ACU_VOICE_GCI_SERVICE			'G'	/* call for voice service on GCI bus */
#define	ACU_RAW_TELEPHONY_SERVICE		':' /* call for 'raw telephony' service on GCI bus: no MPH_B_INIT_RQ is generated (i.e. no B-channel driver is associated) */
#define	ACU_VOICE_TRANS_SERVICE			'W'	/* call for transparent voice service */
#define	ACU_V110_SERVICE				'1' /* call for V.110 service */
#define	ACU_V120_SERVICE				'2' /* call for V.120 service */
#define	ACU_VIDEO_SERVICE				'O' /* call for VIDEO service */
#define	ACU_TDD_SERVICE					'Q' /* call for TDD service */
#define	ACU_DATA_H0_SERVICE				'0' /* call for Data using H0  (384kbs)  channel service (PRI only) */
#define	ACU_DATA_H11_SERVICE			'H' /* call for Data using H11 (1536kbs) channel service (PRI only) */
#define	ACU_DATA_H12_SERVICE			'I' /* call for Data using H12 (1536kbs) channel service (PRI only) */
#define	ACU_DATA_MULTIRATE_SERVICE		'R' /* call for Data using Multirate (2..30*64kbs) channel service (PRI only) */
#define	ACU_DATA_128KBS_SERVICE			'S' /* call for Data using 2*64kbs channel service (BRI only) */
#define	ACU_NO_B_CHAN_SERVICE			'N' /* call for no B-channel service (bearer-independent calls - QSIG only) */
#define ACU_PPP_SERVICE					'Y' /* call for PPP service */
#define ACU_ML_PPP_SERVICE				'Z' /* call for ML-PPP service */

	/* Services for use with 'Physical Relay' process */

#define	ACU_FAX_RELAY_SERVICE			'f'	/* call for G3 facsimile service */
#define	ACU_DATA_RELAY_SERVICE			'd'	/* call for 'data' service */
#define	ACU_DATA_56KBS_RELAY_SERVICE	'k'	/* call for 'data at 56 KBS' service */
#define	ACU_DATA_TRANS_RELAY_SERVICE	't'	/* call for 'data transparent' service */
#define	ACU_MODEM_RELAY_SERVICE			'm'	/* call for modem data service */
#define	ACU_X25_RELAY_SERVICE			'x'	/* call for X.25 circuit-mode service */
#define	ACU_VOICE_RELAY_SERVICE			'v'	/* call for voice service */
#define	ACU_VOICE_GCI_RELAY_SERVICE		'g' /* call for voice service on GCI bus */

#define	ACU_NO_SERVICE					'U'	/* undefined service */

/*--------------------------------------------------------------------------
 *	Calling number presentation (ACU_CONN_RQ/IN, 'calling_nb_pres' field)
 *-------------------------------------------------------------------------*/

#define	ACU_NB_PRES_ALLOWED				G_2 (0,0)	/* presentation allowed */
#define	ACU_NB_PRES_RESTRICTED			G_2 (0,1)	/* presentation restricted */
#define	ACU_NB_PRES_NOT_AVAILABLE		G_2 (1,0)	/* number not available due to interworking */

/*--------------------------------------------------------------------------
 *	Calling(ed) number type (ACU_CONN_RQ/IN, 'calling(ed)_nb_type' field)
 *-------------------------------------------------------------------------*/

#define	ACU_NB_TYPE_UNKNOWN				G_3 (0,0,0)	/* unknown */
#define	ACU_NB_TYPE_INTERNATIONAL		G_3 (0,0,1)	/* international number */
#define	ACU_NB_TYPE_NATIONAL			G_3 (0,1,0)	/* national number */
#define	ACU_NB_TYPE_NET_SPF				G_3 (0,1,1)	/* network specific number */
#define	ACU_NB_TYPE_SUBSCRIBER			G_3 (1,0,0)	/* subscriber number */
#define	ACU_NB_TYPE_LOCAL				G_3 (1,0,0)	/* AT5 - local (directory) number */
#define	ACU_NB_TYPE_ABBREVIATED			G_3 (1,1,0)	/* abbreviated number */

/*--------------------------------------------------------------------------
 *	Called Number plan (ACU_CONN_RQ/IN, 'called_nb_plan' field)
 *-------------------------------------------------------------------------*/

#define ACU_NB_PLAN_UNKNOWN				G_4 (0,0,0,0) /* unknown */
#define ACU_NB_PLAN_ISDN				G_4 (0,0,0,1) /* ISDN/telephony numbering plan (CCITT E.164/E.163) */
#define ACU_NB_PLAN_TELEPHONE			G_4 (0,0,1,0) /* telephony - not in CEPT */
#define ACU_NB_PLAN_DATA				G_4 (0,0,1,1) /* data numbering plan (CCITT X.121) */
#define ACU_NB_PLAN_TELEX				G_4 (0,1,0,0) /* telex numbering plan (CCITT F.69) */
#define ACU_NB_PLAN_NATIONAL			G_4 (1,0,0,0) /* national standard numbering plan */
#define ACU_NB_PLAN_PRIVATE				G_4 (1,0,0,1) /* private numbering plan */
#define ACU_NB_PLAN_RESERVED			G_4 (1,1,1,1) /* reserved for extension */

/*--------------------------------------------------------------------------
 *	Calling number screening indicator (ACU_CONN_RQ/IN, 'calling_nb_screen'
 *  and 'calling_nb2_screen' fields)
 *-------------------------------------------------------------------------*/

#define ACU_NB_SCREEN_USER_PROVIDED		G_2 (0,0)	/* user-provided, not screened */
#define ACU_NB_SCREEN_USER_PASSED		G_2 (0,1)	/* user-provided, verified and passed */
#define ACU_NB_SCREEN_USER_FAILED		G_2 (1,0)	/* user-provided, verified and failed */
#define ACU_NB_SCREEN_NETWORK_PROVIDED 	G_2 (1,1)	/* network provided */

/*--------------------------------------------------------------------------
 *	Redirecting/tion number reason (ACU_CONN_IN/CO, ACU_ALERT_IN, 'redir_nb_reason' field)
 *-------------------------------------------------------------------------*/

#define ACU_NB_REASON_UNKNOWN							G_4 (0,0,0,0)	/* unknown (VN6) */
#define ACU_NB_REASON_CALL_FORWARDING_BUSY				G_4 (0,0,0,1)	/* call forwarding busy (VN6) */
#define ACU_NB_REASON_CALL_FORWARDING_NO_REPLY			G_4 (0,0,1,0)	/* call forwarding no reply (VN6) */
#define ACU_NB_REASON_CALL_FORWARDING_DTE_OUT			G_4 (1,0,0,1)	/* call forwarding DTE out of order */
#define ACU_NB_REASON_CALL_FORWARDING_BY_CALLED			G_4 (1,0,1,0)	/* call forwarding by the called equipment (VN6) */
#define ACU_NB_REASON_CALL_TRANSFER						G_4 (1,1,0,1)	/* call transfer */
#define ACU_NB_REASON_CALL_PICKUP						G_4 (1,1,1,0)	/* call pickup */
#define ACU_NB_REASON_CALL_FORWARDING_UNCONDITIONAL		G_4 (1,1,1,1)	/* call forwarding unconditional (VN6) */

/*--------------------------------------------------------------------------
 *	Calling_nb number: reason for non-notification (ACU_CONN_RQ/IN, NTT only)
 *-------------------------------------------------------------------------*/

#define ACU_NB_NON_NOTIF_UNAVAILABLE					0	/* unavailable */
#define ACU_NB_NON_NOTIF_REJECT_BY_USER					1	/* rejected by user */
#define ACU_NB_NON_NOTIF_INTERACTION					2	/* interaction with other Services */
#define ACU_NB_NON_NOTIF_COIN_LINE						3	/* coin line */
#define ACU_NB_NON_NOTIF_NIL							0xff/* meaningless */

/*--------------------------------------------------------------------------
 *	Called/calling/connected sub-address type, format, odd/even
 *	(ACU_CONN_RQ/IN/RS/CO, 'sa_type' and 'sa_format' fields)
 *-------------------------------------------------------------------------*/

#define ACU_SA_TYPE_NSAP								G_3 (0,0,0)		/* NSAP Sub-address */
#define ACU_SA_TYPE_USER								G_3 (0,1,0)		/* User specified Sub-address */

	/* Subaddress format -- meaningful only when the subaddress type is ACU_SA_TYPE_NSAP;
	 * Other format values are possible, but unspecified.
	 */
#define ACU_SA_FORMAT_IA5								0x50			/* ('P') International Alphabet No 5 */
#define ACU_SA_FORMAT_NIL								0xFF			/* null format */

	/* Sub-address odd/even indicator */
#define ACU_SA_EVEN										G_1 (0)			/* Even number of address signals */
#define ACU_SA_ODD										G_1 (1)			/* Odd number of address signals */

/*--------------------------------------------------------------------------
 *	Network-provided clearing causes (ACU_CLEAR_IN/CO, ACU_SUSPEND_RQ/CO,
 *	ACU_RESUME_RQ/CO, ACU_FACILITY_RQ/CO, 'network_cause' field)
 *	The first nibble is the 'cause-class',
 *	the second nibble is the value in the class.
 *-------------------------------------------------------------------------*/

	/* ACU-specific class (=0xF) values */

#define	ACU_CAUSE_ACU_BAD_ADDRESS		0xF0	/* value 0: bad context addressing info, or no free context available */
#define	ACU_CAUSE_ACU_BAD_SERVICE		0xF1	/* value 1: bad ACU service value */
#define	ACU_CAUSE_ACU_COLLISION			0xF2	/* value 2: incoming call collision */
#define	ACU_CAUSE_ACU_FAC_REJECTED		0xF3	/* value 3: Facility request rejected by ACU */
#define	ACU_CAUSE_CLEARED_BY_NOTIFY		0xF4	/* value 4: call cleared due to a NOTIFY [User-Suspended] -- see description of the CC_CLEAR_IF_NOTIFY_SUSPENDED configuration bit */
#define	ACU_NETWORK_CAUSE_NIL			0xFF	/* value F: unspecified */

/*--------------------------------------------------------------------------
 *	Alert codes (ACU_ALERT_IN)
 *-------------------------------------------------------------------------*/

#define	ACUAC_RING						'R'	/* ring detected */
#define	ACUAC_REMOTE_ALERTED			'A'	/* Remote equipment alerted */

/*--------------------------------------------------------------------------
 *	Progress events (ACU_PROGRESS_RQ/IN)
 *-------------------------------------------------------------------------*/

#define	ACUEV_DIAL_TONE					'T'	/* Received B-channel id on outgoing calls (Dial tone available in channel, received Setup-Ack or Call-Proceeding message) */
#define	ACUEV_CALL_PROC					'C'	/* Received Call-Proceeding message */
#define	ACUEV_INTERWORKING				'I'	/* Received Progress message (interworking) */

/*--------------------------------------------------------------------------
 *	Clear codes (ACU_CLEAR_RQ/IN/RS/CO)
 *-------------------------------------------------------------------------*/

#define	ACURC_BUSY						'b'	/* busy */
#define	ACURC_NOPROCEED					'p'	/* no proceed indication (dial tone) */
#define	ACURC_NOANSWER					'a'	/* no answer */
#define	ACURC_NOAUTOANSWER				't'	/* no auto-answer tone detected */
#define	ACURC_CONGESTED					'c'	/* GSTN or System is congested */
#define	ACURC_INCOMING					'i'	/* incoming call detected while try to dial */
#define	ACURC_NOLINE					'l'	/* (analog) line is seized by another equipment */
											/* (ISDN) Wrong Addressing info, or context already used */
#define	ACURC_ERRNUM					'e'	/* errored number */
#define	ACURC_INHNUM					'n'	/* inhibited number */
#define	ACURC_2MNUM						'2'	/* too many errored/inhibited numbers */
#define	ACURC_HUNGUP					'h'	/* remote has hung up or incident on connection */
#define	ACURC_NETWORK_ERROR				'N'	/* (ISDN) network disconnected us */
#define	ACURC_TIMEOUT					'T'	/* time-out error */
#define	ACURC_BAD_SERVICE				'S'	/* Bad Service-id in ACU_CONN_RQ/RS */
#define	ACURC_INTERNAL					'I'	/* other internal error */

	/*
	 *	The following codes are never sent in an ACU_CLEAR_xx message
	 */

#define	ACURC_OK						0	/* No error */
#define	ACURC_BL_TIMEOUT				1	/* BL time-out error */
#define	ACURC_IN_CALL					2	/* BL error, still in call */
#define	ACURC_CLEAR_RQ					3	/* User entity requested CLEAR */

/*--------------------------------------------------------------------------
 *	Dialing modes
 *-------------------------------------------------------------------------*/

#define	ACU_PULSE_DIAL					'P'	/* loop disconnect pulse dialing */
#define	ACU_TONE_DIAL					'T'	/* DTMF tone dialing */

/*--------------------------------------------------------------------------
 * 	Exchange time codes (not used)
 *-------------------------------------------------------------------------*/

#define	UUI_ALERT						'a'	/* to transmit at ALERT time */
#define	UUI_CLEAR						'b'	/* to transmit at CLEAR time */
#define	UUI_USER_INFORMATION    		'c'	/* immediatly in a user information message */

/*--------------------------------------------------------------------------
 *	Response mode codes (ACU_RS_MODE_RQ/CO)
 *-------------------------------------------------------------------------*/

#define RS_MODE_ALERT					'a'	/* ALERT   requested on incoming call */
#define RS_MODE_CONNECT					'b'	/* CONNECT requested on incoming call */
#define RS_MODE_CLEAR					'c'	/* CLEAR   requested on incoming call */

/*--------------------------------------------------------------------------
 *	Facility codes (ACU_FACILITY_RQ/IN/CO)
 *-------------------------------------------------------------------------*/

#define ACU_FAC_CALL_FORWARDING			21	/* (0x15) call forwarding unconditional (CFU) */
#define ACU_FAC_CALL_DEFLECTION			22	/* (0x16) call deflection (CD) */
#define ACU_FAC_CHARGING				23	/* (0x17) advice of charge (AOC-D) */
#define ACU_FAC_CHARGING_TOTAL			24	/* (0x18) advice of charge - total cost (AOC-E) */
#define ACU_FAC_HOLD					25	/* (0x19) call hold (HOLD) */
#define ACU_FAC_RETRIEVE				26	/* (0x1a) call retrieve  */
#define ACU_FAC_ALTERNATE				27	/* (0x1b) "toggle" between active and held call */
#define ACU_FAC_TRF						28	/* (0x1c) Explicit Call Transfer (ECT), implicit invocation only -- for explicit invocation, use ACU_FAC_EXPLICIT_ECT */
#define ACU_FAC_CONFERENCE				29	/* (0x1d) conferencing, 3-party (CONF,3PTY) */
#define ACU_FAC_THREE_PARTY				ACU_FAC_CONFERENCE /* alias */
#define ACU_FAC_MALICIOUS_CALL_ID		32	/* (0x20) malicious call identification (MCID) */
#define ACU_FAC_CFB						33	/* (0x21) Call Forwarding Busy (CFB) */
#define ACU_FAC_CFNR					34	/* (0x22) Call Forwarding No Reply (CFNR) */
#define ACU_FAC_TBCT					35	/* (0x23) Two-B-Channel Transfer (TBCT, Enhanced-Explicit-Call-Transfer) */
#define ACU_FAC_MWI						36	/* (0x24) Message-Wainting Indication (MWI) */
#define ACU_FAC_EXPLICIT_ECT			37	/* (0x25) ExplicitECT invocation -- for implicit ECT, use ACU_FAC_TRF */
#define ACU_FAC_TBCT_DCIRQ				38	/* (0x26) !!Reserved for CC private use only!! for TBCT: D-Channel-Id-Request */
#define ACU_FAC_EXPLICIT_ECT_LINKIDRQ	39	/* (0x27) !!Reserved for CC private use only!! for ExplicitECT: ECTLinkIdRequest */

#define ACU_FAC_RECALL					50	/* (0x32) Recall facility (ETSI) */

#define ACU_FAC_TRANSPARENT				100	/* (0x64) transparent facility accross CC&NS */

#define ACU_FAC_CALL_APPEAR				200	/* (0xC8) (North-America) Call appearence request */
#define ACU_FAC_FEATURE_ACT				201	/* (0xC9) (North-America) Feature activation request */
#define ACU_FAC_DROP					202	/* (0xCA) (North-America) Drop call */
#define ACU_FAC_FEATURE_IND				203	/* (0xCB) (North-America) Feature indication (NT->TE only) */
#define ACU_FAC_VOICE_MAIL				204	/* (0xCD) (North-America) Voice-Mail feature (NI2) */

#define ACU_FAC_PCS_USER_ONLY			250	/* (0xFA) (VN6) send PCS-User information only, \
											 * with no other facility request. \
											 */

/*--------------------------------------------------------------------------
 *	Action codes (ACU_FACILITY_RQ/IN/RS/CO, ACU_TEST_RQ/CO, ACU_SET_MODE_RQ/CO)
 *-------------------------------------------------------------------------*/

	/* ACU_FACILITY_RQ (TE side),
	 * ACU_FACILITY_IN (NT side),
	 * ACU_TEST_RQ,
	 * ACU_SET_MODE_RQ
	 */
#define ACU_ACTION_ACTIVATE				'A'	/* activate or register action */
#define ACU_ACTION_CLEAR				'C'	/* deactivate or clear action */
#define ACU_ACTION_ENQUIRY				'E'	/* enquiry action */

	/* ACU_FACILITY_IN (TE side) */

#define ACU_ACTION_TX					't'	/* incoming Facility indication (TE side) */

	/* ACU_FACILITY_RS (NT side),
	 * ACU_FACILITY_CO (TE side),
	 * ACU_TEST_CO,
	 * ACU_SET_MODE_CO
	 */
#define ACU_ACTION_ACK					'a'	/* acknowledgement */
#define ACU_ACTION_REJ					'r'	/* reject */

	/* aliases */
#define ACU_RQ_ACTIVATE					ACU_ACTION_ACTIVATE	/* activate or register action */
#define ACU_RQ_CLEAR					ACU_ACTION_CLEAR	/* deactivate or clear action */
#define ACU_RQ_ENQUIRY					ACU_ACTION_ENQUIRY	/* enquiry action */

#define ACU_IN_TX						ACU_ACTION_TX	/* incoming Facility indication (TE side) */

#define ACU_CO_ACK						ACU_ACTION_ACK	/* acknowledgement */
#define ACU_CO_REJ						ACU_ACTION_REJ	/* reject */

/*--------------------------------------------------------------------------
 *	ACU_SET_MODE_RQ/CO (codes)
 *-------------------------------------------------------------------------*/

#define ACU_MODE_NA_OUT_OF_ORDER		'a' /* allow/forbid calls on NAI given by R_msg_nai. */
#define ACU_MODE_CALL_OUT_OF_ORDER		'b' /* allow/forbid calls on NAI/CONN_ID given by R_msg_nai/R_msg_conn_id. */
#define ACU_MODE_CHANI_OUT_OF_ORDER		'c' /* allow/forbid usage of the B-channel given by Acu_set_mode_data_chani. */
#define ACU_MODE_ALL_NA_OUT_OF_ORDER	'd' /* allow/forbid calls on all NAIs. */

/*--------------------------------------------------------------------------
 *	ACU_ERR_IN cause codes
 *-------------------------------------------------------------------------*/

#define ACUER_PRIMITIVE_CODE			'a'	/* unknown primitive code */
#define ACUER_PARAM_VAL					'b'	/* invalid parameter */
#define ACUER_MANDATORY_PARAM_MISSING	'c'	/* mandatory parameter missing */
#define ACUER_PARAM_TYPE				'd'	/* incorrect parameter type */
#define ACUER_PARAM_LGTH				'e'	/* incorrect parameter length */
#define ACUER_UNEXPECTED_PRIMITIVE		'f'	/* unexpected primitive */
#define ACUER_PRIMITIVE_NOT_IMPLEMENTED 'j'	/* SSDU primitive not implemented */
#define ACUER_NO_TIMER_AVAILABLE		'q'	/* no more space to allocate new timer cells */
#define ACUER_CONGESTION				'm'	/* resource congestion */

/*--------------------------------------------------------------------------
 *	ACU_ERR_IN cause diagnostics
 *-------------------------------------------------------------------------*/

	/*
		For ACUER_PRIMITIVE_CODE
		and ACUER_PRIMITIVE_NOT_IMPLEMENTED
		errors:

		the diagnostic is the ACU primitive code that has generated the error.
	*/

	/*
		For ACUER_PARAM_VAL,
			ACUER_MANDATORY_PARAM_MISSING,
			ACUER_PARAM_TYPE,
		and ACUER_PARAM_LGTH,
		errors:
	*/

#define ACUERDIAG_CHAN_ID				0x18	/* Chan_id Information */
#define ACUERDIAG_DISPLAY				0x28	/* Display Information */
#define ACUERDIAG_KEYPAD				0x2c	/* Keypad Information */
#define ACUERDIAG_CALLING_NB			0x6c	/* Calling number Information */
#define ACUERDIAG_CALLING_SUB_ADD		0x6d	/* Calling sub-address Information */
#define ACUERDIAG_CALLED_NB				0x70	/* Called number Information */
#define ACUERDIAG_CALLED_SUB_ADD		0x71	/* Called sub-address Information */
#define ACUERDIAG_UUI					0x7e	/* UUI Information */

	/*
		For ACUER_NO_TIMER_AVAILABLE and ACUER_CONGESTION errors:
		No diagnostic.
	*/

/*--------------------------------------------------------------------------
 *	Button management (voice supplementary services)
 *-------------------------------------------------------------------------*/

		/* Button type */

#define ACU_BUTTON_TYPE_UNKNOWN			0
#define ACU_CALL_APPEAR_BUTTON			1
#define ACU_FEATURE_BUTTON				2
#define ACU_NON_APPLICABLE				3

		/* Module number */

#define ACU_MODULE_0					0
#define ACU_MODULE_1					1
#define ACU_MODULE_2					2
#define ACU_MODULE_3					3
#define ACU_MODULE_4					4
#define ACU_MODULE_5					5
#define ACU_MODULE_6					6
#define ACU_MODULE_7					7

		/* Switchhook */

#define ACU_HOOK_ON						0		/* on-hook */
#define ACU_HOOK_OFF					1		/* off-hook */

/*--------------------------------------------------------------------------
 *	DTMF dialing up/down flag (ACU_DIGIT_RQ/IN)
 *-------------------------------------------------------------------------*/

#define	ACUKEY_DOWN						0x00	/* key down */
#define	ACUKEY_UP						0x01	/* key up */

/*--------------------------------------------------------------------------
 *	Notify description
 *-------------------------------------------------------------------------*/

#define	ACUNO_DESCRIPTION				0xFF	/* Empty field */
#define	ACUUSER_RESUMED					0x01	/* User resumed */

#define ACUNOTIF_SUSPENDED				0x00	/* User suspended */
#define ACUNOTIF_RESUMED				0x01	/* User resumed */

#define ACUNOTIF_CONF_BRIDGE_FULL		0x2d	/* conference bridge full (DMS) */

/*--------------------------------------------------------------------------
 *	SPF facility Id (ACU_CONN_RQ/IN)
 *-------------------------------------------------------------------------*/

#define ACUSPF_FAC_ABSENT				0xff	/* No SPF FAC */
#define ACUSPF_FAC_GES_PRIV_NET_SELECT	0x00	/* GES private network selection */
#define ACUSPF_FAC_GES_SPECIAL_SERVICE	0x01	/* GES special services */
#define ACUSPF_FAC_GES_SELECTION		0x03	/* GES selection */


/*--------------------------------------------------------------------------
 *	Progress description (ACU_CONN_RQ/IN, ACU_ALERT_IN, ACU_PROGRESS_IN)
 *-------------------------------------------------------------------------*/

#define	ACUNO_PROGRESS_DESCR					0xff	/* no progress info to be stored */

#define	ACUPROGRESS_DESCR_NON_END_TO_END_ISDN	0x01	/* call is not end-to-end ISDN, further call progress information may be available */
#define	ACUPROGRESS_DESCR_NON_ISDN_DEST			0x02	/* destination address is non-ISDN */
#define	ACUPROGRESS_DESCR_NON_ISDN_ORIG			0x03	/* origination address is non-ISDN */
#define	ACUPROGRESS_DESCR_CALL_RETURNED_ISDN	0x04	/* call has returned to the ISDN */
#define	ACUPROGRESS_DESCR_SERVICE_CHANGE		0x05	/* interworking has occurred and has resulted in a telecommunication service change */
#define	ACUPROGRESS_DESCR_IN_BAND_NOW			0x08	/* in-band information or appropriate pattern now available */
			/* QSIG specific description (To be used with CODING_STD_ECMA) */
#define	ACUPROGRESS_DESCR_INTW_PU_NET			0x10	/* Interworking with a public network */
#define	ACUPROGRESS_DESCR_INTW_NO_REL			0x11	/* Interworking with a network unable to supply a release signal */
#define	ACUPROGRESS_DESCR_INTW_NO_REL_BA		0x12	/* Interworking with a network unable to supply a release signal before answer */
#define	ACUPROGRESS_DESCR_INTW_NO_REL_AF		0x13	/* Interworking with a network unable to supply a release signal after answer */
#define	ACUPROGRESS_DESCR_INTW_PR_NET			0x14	/* Interworking with another private network */

/*--------------------------------------------------------------------------
 *	Progress location (ACU_CONN_RQ/IN, ACU_ALERT_IN, ACU_PROGRESS_IN)
 *-------------------------------------------------------------------------*/

#define	ACUPROGRESS_LOC_USER					0x00	/* user */
#define	ACUPROGRESS_LOC_PR_LOCAL_USER			0x01	/* private network serving the local user */
#define	ACUPROGRESS_LOC_PU_LOCAL_USER			0x02	/* public network serving the local user */
#define	ACUPROGRESS_LOC_TRANSIT_NET				0x03	/* transit network */
#define	ACUPROGRESS_LOC_PU_REMOTE_USER			0x04	/* public network serving the remote user */
#define	ACUPROGRESS_LOC_PR_REMOTE_USER			0x05	/* private network serving the remote user */
#define	ACUPROGRESS_LOC_INTERNATIONAL			0x07	/* international network */
#define	ACUPROGRESS_BEYONG_IWP					0x0a	/* network beyong interworking point */

/*--------------------------------------------------------------------------
 *	UUI protocol discriminator:
 *  0 defaults to 'IA5' characters.
 *  The values defined here are the ones defined by the standards,
 *  incremented by 1 to reserve the value '0' for compatibility.
 *  One can use the following macros to translate the values from
 *  Q.931 format into ACU format and vice-versa:
 *  - Acu_uui_acu_protocol_from_q931 (q931_protocol)
 *  - Acu_uui_q931_protocol_from_acu (acu_protocol)
 *-------------------------------------------------------------------------*/

#define ACU_UUI_USER_SPF						0x01		/* User specific coding */
#define ACU_UUI_OSI								0x02		/* OSI */
#define ACU_UUI_X244							0x03		/* rec. X.244 */
#define ACU_UUI_SYSTEM_MANAGEMENT				0x04		/* reserved for system management convergence function */
#define ACU_UUI_IA5								0x05		/* IA5 characters (ASCII) */
#define ACU_UUI_V120							0x08		/* rec. V.120 */
#define ACU_UUI_Q931							0x09		/* rec. Q.931 */

	/* translation macros */

#define Acu_uui_acu_protocol_from_q931(q931_protocol)	(q931_protocol+1)
#define Acu_uui_q931_protocol_from_acu(acu_protocol)	(acu_protocol-1)

/*--------------------------------------------------------------------------
 *	pcs_user protocol discriminator (first byte of the pcs_user string)
 *  (ACU_CONN_RQ/IN/RS/CO, ACU_ALERT_IN/RQ, ACU_PROGRESS_IN/RQ, ACU_CLEAR_RQ/IN/RS/CO)
 *-------------------------------------------------------------------------*/

#define	ACUPCS_USER_TRANSGROUP					0x44		/* Transgroupe */
#define	ACUPCS_USER_PUBLIPHONE					0x45		/* Publiphone */
#define	ACUPCS_USER_CALL_ROUTING				0x46		/* Call-routing */

/*--------------------------------------------------------------------------
 *	Charging_info (ACU_CONN_RS/CO, ACU_CLEAR_RQ/IN/RS/CO, ACU_INFORMATION_RQ/IN)
 *  (implemented in VN6 only for the time being, further extensions to
 *  ETSI-like variants are to be done in the future)
 *-------------------------------------------------------------------------*/

	/* 'charging_info.type' field */

#define	ACUCHARG_TYPE_FREE_OF_CHARGE			0			/* free of charge (N/A in VN6/FRANCE) */
#define	ACUCHARG_TYPE_CHARGE_ON_DURATION		1			/* charge on duration (N/A in VN6/FRANCE) */
#define	ACUCHARG_TYPE_CHARGE_ON_ANSWER			2			/* charge on answer (N/A in VN6/FRANCE) */
#define	ACUCHARG_TYPE_CHARGE_INCREMENT			3			/* charge increment (N/A in VN6/FRANCE) */
#define	ACUCHARG_TYPE_SUB_TOTAL_CHARGE			4			/* sub-total charge */
#define	ACUCHARG_TYPE_TOTAL_CHARGE				5			/* total charge */

	/* 'charging_info.period' field: (N/A in VN6/FRANCE)
	 *
	 * - optional
	 * - only used if the 'charging_info.type' is either
	 *   ACUCHARG_TYPE_CHARGE_ON_DURATION or ACUCHARG_TYPE_CHARGE_ON_ANSWER.
	 */

#define	ACUCHARG_PERIOD_0_1_S					8			/* 0.1 second */
#define	ACUCHARG_PERIOD_1_S						9			/* 1 second */
#define	ACUCHARG_PERIOD_10_S					10			/* 10 seconds */
#define	ACUCHARG_PERIOD_60_S					11			/* 1 minute */

	/* 'charging_info.multi' field: (N/A in VN6/FRANCE)
	 *
	 * - if the 'charging_info.type' is either
	 *   ACUCHARG_TYPE_CHARGE_ON_DURATION or ACUCHARG_TYPE_CHARGE_ON_ANSWER,
	 *   then the 'multi' field specifies the charging PERIOD multiplier;
	 *   If this field is coded ACU_BYTE_VAL_NIL or 0 (i.e. the field is
	 *   not provided), then the charging period is in seconds.
	 *
	 * - if the 'charging_info.type' is either
	 *   ACUCHARG_TYPE_SUB_TOTAL_CHARGE or ACUCHARG_TYPE_TOTAL_CHARGE,
	 *   then the 'multi' field specifies the charging VALUE multiplier as
	 *   defined below;
	 *   The ACU interface values are defined as the values in the
	 *   ETS 300 182-1 spec + 1.
	 *   If the 'multi' field is 0, then no 'currency' string is provided;
	 *   Otherwize the 'currency' field provides the currency unit as a
	 *   null-terminated IA5 string (max 10+1 bytes).
	 */

#define	ACUCHARG_MULTI_NIL						0			/* no multiplier provided */
#define	ACUCHARG_MULTI_0_001					(0+1)		/* oneThousandth */
#define	ACUCHARG_MULTI_0_01						(1+1)		/* oneHundredth */
#define	ACUCHARG_MULTI_0_1						(2+1)		/* oneTenth */
#define	ACUCHARG_MULTI_1						(3+1)		/* one */
#define	ACUCHARG_MULTI_10						(4+1)		/* ten */
#define	ACUCHARG_MULTI_100						(5+1)		/* hundred */
#define	ACUCHARG_MULTI_1000						(6+1)		/* thousand */

#if ARINC_NETWORKS == ON	/* { */
/*--------------------------------------------------------------------------
 *	Route selection for SATCOM, TFTS and NATS (ACU_CONN_RQ/IN)
 *-------------------------------------------------------------------------*/
	/* NATS */

#define	ACUNATS_REMOVE_USER						0x00		/* remove user from NATS queue */
#define	ACUNATS_REQUEST_Q_USER					0x01		/* Request NATS queue */

	/* SATCOM */

#define	ACUSATCOM_REMOVE_USER					0x00		/* remove user from SATCOM queue */
#define	ACUSATCOM_REQUEST_Q_USER                0x01		/* Request SATCOM queue */

	/* TFTS */

#define	ACUTFTS_REMOVE_USER                    	0x00		/* remove user from TFTS queue */
#define	ACUTFTS_REQUEST_Q_USER                  0x01		/* Request TFTS queue */

/*--------------------------------------------------------------------------------------
 *	Credit card status (ACU_INFORMATION_RQ/IN) - ARINC networks only
 *-------------------------------------------------------------------------*/

#define	ACUCARD_NB_ACCEPTED						0x00		/* credit card number accepted */
#define	ACUCARD_TYPE_INVALID					0x01		/* credit card type invalid */
#define	ACUCARD_NB_INVALID_LUHN_CHECK			0x02		/* credit card number invalid based on Luhn check */
#define	ACUCARD_START_DATE_INVALID				0x03		/* credit card start date invalid */
#define	ACUCARD_EXPIRATION_DATE_INVALID			0x06		/* credit card expiration date invalid */
#define	ACUCARD_VENDOR_RESERVED_1				0x07		/* vendor reserved 1 */
#define	ACUCARD_VENDOR_RESERVED_2				0x08		/* vendor reserved 2 */
#define	ACUCARD_REJECT							0x09		/* credit card reject no specific cause */

/*--------------------------------------------------------------------------------------
 *	Queue information (ACU_INFORMATION_RQ/IN) - ARINC networks only
 *-------------------------------------------------------------------------*/

#define	ACUNATS_AVAILABLE						0x01		/* available */
#define	ACUNATS_NOT_AVAILABLE					0x00		/* not available */
#define	ACUSATCOM_AVAILABLE						0x01		/* available */
#define	ACUSATCOM_NOT_AVAILABLE					0x00		/* not available */
#define	ACUTFTS_AVAILABLE						0x01		/* available */
#define	ACUTFTS_NOT_AVAILABLE					0x00		/* not available */

#define	ACUNATS_QUEUED							0x01		/* queued */
#define	ACUNATS_NOT_QUEUED						0x00		/* not queued */
#define	ACUSATCOM_QUEUED						0x02		/* queued */
#define	ACUSATCOM_NOT_QUEUED					0x00		/* not queued */
#define	ACUTFTS_QUEUED							0x04		/* queued */
#define	ACUTFTS_NOT_QUEUED						0x00		/* not queued */

#endif	/* ARINC_NETWORKS */	/* } */

/*--------------------------------------------------------------------------------------
 *	Signal values (ACU_CONN_RS/CO, ACU_ALERT_RQ/IN, ACU_PROGRESS_IN,
 *  ACU_CLEAR_RQ/IN, ACU_INFORMATION_RQ/IN)
 *-------------------------------------------------------------------------*/

#define ACUDIAL_ON								0x00		/* dial tone on */
#define ACURING_BACK_ON							0x01		/* ring back tone on */
#define ACUINTERCEPT_ON							0x02		/* intercept tone on */
#define ACUNET_CONGEST_ON						0x03		/* network congest tone on */
#define ACUBUSY_ON								0x04		/* busy tone on */
#define ACUCONFIRM_ON							0x05		/* confirm tone on */
#define ACUANSWER_ON							0x06		/* answer tone on*/
#define ACUCALL_WAITING_ON						0x07		/* call waiting tone on */
#define ACUOFF_HOOK_WARNING_ON					0x08		/* off-hook warning tone on */
#define ACUTONES_OFF							0x3f		/* tones off */
#define ACUALERTING_OFF							0x4f		/* alerting off */
#define ACUPATTERN_0							0x40		/* alerting on - pattern 0 */
#define ACUPATTERN_1							0x41		/* alerting on - pattern 1 */
#define ACUPATTERN_2							0x42		/* alerting on - pattern 2 */
#define ACUPATTERN_3							0x43		/* alerting on - pattern 3 */
#define ACUPATTERN_4							0x44		/* alerting on - pattern 4 */
#define ACUPATTERN_5							0x45		/* alerting on - pattern 5 */
#define ACUPATTERN_6							0x46		/* alerting on - pattern 6 */
#define ACUPATTERN_7							0x47		/* alerting on - pattern 7 */
#define ACUUNSPECIFIED_TONE						0x7f		/* unspecified tone */
	/* ARINC-specific signal values: */
#define	ACUCREDIT_CARD_TONE_ON					0x40		/* credit card tone ON */
#define	ACUOUT_OF_RANGE_TONE_ON					0x41		/* out of range tone ON */
#define	ACUQUEUING_TONE_ON						0x42		/* queuing tone ON */
#define	ACUAUDIBLE_TONE_ON						0x44		/* audible tone ON */
#define	ACUVISUAL_ALERT_1_ON					0x45		/* visual alert #1 ON */
#define	ACUVISUAL_ALERT_2_ON					0x46		/* visual alert #2 ON */
#define	ACUSATCOM_CALL_NOTIFY					0x47		/* SATCOM call notify */


/*-------------------------------------------------------------------------
 *	Endpoint Id (ACU_INFORMATION_RQ/IN and ACU_CONN_RQ)
 *------------------------------------------------------------------------*/

#if _CC_SPID_INFO == ON

		/* Action field */

#	define ACU_ACTION_NO_ENDPOINT_ID			0			/* Don't send any Endpoint Id (ACU_CONN_RQ) */
#	define ACU_ACTION_SND_USID					1			/* SPID was OK, send the Endpoint Id info */
#	define ACU_ACTION_NO_SPID_NEGOTIATION		2			/* SPID negotiation is not supported */
#	define ACU_ACTION_INVALID_SPID				3			/* The received SPID is wrong */
#	define ACU_ACTION_PROMPT_INFO				4			/* Need a SPID from the terminal */

		/* Interpreter field */

#	define ACU_INTERPRETER_0					0			/* Terminal is selected if it is assigned the indicated TID and USID value */
#	define ACU_INTERPRETER_1					1			/* Terminal is selected if it is NOT assigned the indicated TID but is assigned the indicated USID value */

#endif

/*-------------------------------------------------------------------------
 *	ACU and CC entities entry point prototypes
 *------------------------------------------------------------------------*/

	uchar	cc						Fct_proto((struct message FAR *p_msg));
	void	cc_var_init				Fct_proto((void));
	uchar	acu						Fct_proto((struct message FAR *p_msg));
	void	acu_var_init			Fct_proto((void));

/*-------------------------------------------------------------------------
 *		Simulation routines prototypes
 *------------------------------------------------------------------------*/

#if (SIMULATION == ON) || (SIM_ANALYSER == ON)
	uchar 	ana_sdu_acu 			Fct_proto((struct message FAR *p_msg));
	uchar	build_sdu_dte_to_acu 	Fct_proto((void));
	uchar	build_sdu_acu_to_dte	Fct_proto((void));
#endif

/*------------------------------------------------------------------------*/

#endif /* ACU_INT */

/*EOF*/
