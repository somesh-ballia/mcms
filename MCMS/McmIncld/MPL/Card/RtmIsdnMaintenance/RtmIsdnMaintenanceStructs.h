// RtmIsdnMaintenanceStructs.h
//
//////////////////////////////////////////////////////////////////////

#ifndef RTMISDNMAINTENANCESTRUCTS_H_
#define RTMISDNMAINTENANCESTRUCTS_H_




//////////////////////////////
//       ENUMERATORS
//////////////////////////////
// ================================
// ======= eVLanEntityType ========
// ================================
enum eVLanEntityType
{
	eVLanEntity_Invalid			= 0,
	eVLanEntity_Rtm				= 1,
	
	NUM_OF_VLAN_ENTITY_TYPES	= 2
};
static const char *vLanEntityTypeStr[] = 
{
	"vLanEntity_Invalid",
	"vLanEntity_Rtm",
};




//////////////////////////////
//    API STRUCTURES
//////////////////////////////

// ====================================
//      RTM_ISDN_PARAMETERS_S
// ====================================
typedef struct
{
	APIU32		country_code;		// see codes below	- from system.cfg (see defaults in NET8_PARAMETERS section)
	APIU32		idle_code_T1;		// from system.cfg (see defaults in NET8_PARAMETERS section)
	APIU32		idle_code_E1;		// from system.cfg (see defaults in NET8_PARAMETERS section)
	APIU32		number_of_digits;	// from system.cfg (see defaults in NET8_PARAMETERS section)
	APIU32		isdn_clock;			// YES(isdn) / NO(internal) - from sysConfig

	APIU32		future_use1;
	APIU32		future_use2;
} RTM_ISDN_PARAMETERS_S;



// ====================================
//      RTM_ISDN_SPAN_CONFIG_REQ_S
// ====================================
typedef struct
{
    APIU32		span_id;
	APIU32		unit_type;			// unit_type_val
	APIU32		framing;			// framing_val
	APIU32		line_code;			// line_code_val
	APIU32		functional_group;	// func_group_val
	APIU32		signaling_mode;		// signal_mode_val
	APIU32		d_chanl_needed;		// YES (since no leased line)   
	APIU32		switchType;			// switch_type_val
	APIU32		userConfigClock;	// clock_val [for future use] master/backup/none (default: none)

	APIU32		future_use1;
	APIU32		future_use2;
} RTM_ISDN_SPAN_CONFIG_REQ_S;

// ====================================
//      RTM_ISDN_SPAN_DISABLE_REQ_S
// ====================================
typedef struct
{
    APIU32		span_id;

	APIU32		future_use1;
	APIU32		future_use2;
} RTM_ISDN_SPAN_DISABLE_REQ_S;


// ====================================
//      RTM_ISDN_VLAN_ID_S
// ====================================
typedef struct
{
    APIU32		vLan_entityType; // eVLanEntityType
    APIU32		vLan_id;

	APIU32		future_use1;
	APIU32		future_use2;
} RTM_ISDN_VLAN_ID_S;


// ====================================
//      RTM_ISDN_SPAN_CONFIG_IND_S
// ====================================
typedef struct
{
    APIU32		span_id;
 	APIU32		alarm_status;
	APIU32		d_chnl_status;
	APIU32		clocking_status;

	APIU32		future_use1;
	APIU32		future_use2;
} RTM_ISDN_SPAN_STATUS_IND_S;




/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                      VALUES
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// ===========================
//      country_code values
// ===========================
#define	AUSTRALIA           61
#define	AUSTRIA             43
#define	BELGIUM             32
#define	CANADA              1
#define CHILE               56
#define	DENMARK             45
#define EUROPE              1000
#define	FINLAND             358
#define	FRANCE              33
#define	GERMANY             49
#define	HONG_KONG           852
#define	INDIA               91
#define	IRLANDE             353
#define	ITALY               39
#define	JAPAN               81
#define	KOREA               82
#define	LUXEMBOURG          352
#define	NETHERLANDS         31
#define	NEW_ZEALAND         64
#define	NORWAY              47
#define	PORTUGAL            351
#define	SOUTH_AFRICA        27
#define	SPAIN               34
#define	SWEDEN              46
#define	SWITZERLAND         41
#define TAIWAN				886
#define	UK                  44
#define	USA                 1
#define	USSR                7
#define COUNTRY_NIL         9999



// ===========================
//      unit_type values
// ===========================
#define PCM_MODE_E1				'E'
#define PCM_MODE_T1				'T'
/* - old unit_type values
#define utE1_VAL				1        
#define utT1_VAL				2 
*/



// ===========================
//        framing values
// ===========================
/* E1 Frame Format */
#define E1_FRAMING_DDF				'a'		/* DoubleFrame Format */
#define E1_FRAMING_MFF_CRC4			'b'		/* CRC4 MutiFrame Format */
#define E1_FRAMING_MFF_CRC4_EXT		'c'		/* CRC4 MultiFrame Format Extended G.706B */

/* T1 Framing Format */
#define T1_FRAMING_F4				'A'		/* 4-Frame multiframe */
#define T1_FRAMING_F12				'B'		/* 12-Frame multiframe (D4) */
#define T1_FRAMING_ESF				'C'		/* Extended Superframe without CRC6 */
#define T1_FRAMING_ESF_CRC6			'D'		/* Extended Superframe with CRC6*/
#define T1_FRAMING_F72				'E'		/* 72-Frame multiframe (SLC96) */
#define T1_FRAMING_ESF_CRC6_JT		'F'		/* Extended Superframe with CRC6 JT G.706 for Japan */

/* - old framing values:
// T1 framing definitions
#define frmESF					0 // extended superframe (ESF) default value
#define frmESF_ZBTSI			1 // ESF with ZBTSI
#define frmSF_SLC96 			2 // Superframe SLC96
#define frmSF					3 // Superframe 

// E1 framing definitions
#define frmFEBE					0 // crc4 multiframing with Si=FEBE. Default val
#define frmCRC4					1 // crc4 multiframing with Si=1.
#define frmBASIC				2 // basic framing with no crc4, Si=1
#define frmFEBE_CAS				3 // FEBE with TS16 channel associated signaling
#define frmCRC4_CAS				4 // crc4 eith TS16 channel associated signaling
#define frmBASIC_CAS			5 // basic with TS16 chanel associated signaling
#define frmDISABLED				6 // ask zigi ????
*/



// ===========================
//      line_code values
// ===========================
#define LINE_CODE_NRZ			'n'		/* NRZ optical interface */
#define LINE_CODE_CMI			'c'		/* CMI 1T2B + HDB3 optical interface */
#define LINE_CODE_AMI			'a'		/* AMI ternary or digital dual rail interface */
#define LINE_CODE_HDB3			'3'		/* HDB3 code ternary or digital rail interface */
#define LINE_CODE_B8ZS			'8'		/* B8ZS Code ternary or digital dual rail interface */

/* - old line_code values:
// T1 line_code definitions
#define codB8ZS					0 // Binary 8 Zero substitution,default value
#define codB7ZS					1 // Bit 7 Zero suppression
#define codTRANSPARENT			2 // AMI coding

// E1 line_code definitions
#define codHDB3					0 // Binary 3 Zero substitution , default value
#define codTRANSPARENT			2 // AMI coding
*/



// ===========================
//     signaling_mode values
// ===========================
#define	SIGNALLING_NIL			'N'
#define	SIGNALLING_CCS			'A' /* Common Channel Signalling */
/* - old signaling_mode values:
#define NO_SIGNALLING			0x0001 //LEASED LINE 30 FOR T1 OR 24 FOR E1
#define NO_SIGNALLING31			0x0002 //LEASED LINE 31
*/



// ===========================
//   functional_group values
// ===========================
#define FG_NT_NET				4		/* NT2 terminal side */	// according to Tomer
#define FG_NT_TE				8		/* NT2 network side */	// according to Tomer
/* - old functional_group values
#define cnfgUSER_SIDE_L2		0x00000400  // Run user side, level 2
#define cnfgNET_SIDE_L2			0x00000800  // Run network side, level 2
*/



// ============================================
//   userConfigClock, clocking_status values
// ============================================
#define CLOCK_NONE		0xff
#define TO_MASTER		0
#define TO_BACKUP		1
	   


// ===========================
//   alarm_status values
// ===========================
#define NO_ALARM		0xff
#define RED_ALARM		1
#define YELLOW_ALARM	2



// ===========================
//   d_chnl_status values
// ===========================
#define D_CHANNEL_ESTABLISHED		1
#define D_CHANNEL_NOT_ESTABLISHED	2


// ===========================
//   switch_type_val values
// ===========================
#define NT_DMS100				8  /* Northern Telecom DMS100 */
#define NTT						9  /* Nippon Telegraph Telephone */
#define ETSI					11  /* European ETSI Technical Comittee */
#define N_ISDN1					13  /* US National ISDN 1 */
#define AUSTEL_1				15  /* Australian Telecom 1 */
#define HK_TEL					17  /* Hong Kong Telephone */
#define N_ISDN2					20  /* US National ISDN 2 */
#define ATT_5E10				23  /* ARINC CTU to Bearer Systems (BS) -  ARINC 746 attachment 11 */
#define ATT_4ESS				24  /* AT&T 4ESS (AT&T TR41459, August 1995) */
#define NT_DMS250				50 /* Northern Telecom DMS250 */
 
 
/* Network-Specific Types of Calls (Call-by-Call feature) */ //(taken from MGC code - netint.h)
#define PRInsNULL				0x00 /* The default case */
#define PRInsATT_SDN			0x01 /* AT&T Software Defined Network */
#define PRInsATT_MEGACOM800		0x02 /* AT&T Megacom 800 service */
#define PRInsATT_MEGACOM		0x03 /* AT&T Megacom */
#define PRInsATT_ACCUNET		0x06 /* AT&T Accunet */
#define PRInsATT_I800			0x08 /* AT&T International 800 */
#define PRInsATT_MULTIQUEST		0x10 /* AT&T MultiQuest */
#define PRInsNTI_PRIVATE		0x01 /* Northern Tel Private Net */
#define PRInsNTI_INWATS			0x02 /* Northern Tel InWats */
#define PRInsNTI_OUTWATS		0x03 /* Northern Tel OutWats */
#define PRInsNTI_FX				0x04 /* Northern Tel Foreign Exchange */
#define PRInsNTI_TIE_TRUNK		0x05 /* Northern Tel Tie Trunk */
#define PRInsNTI_TRO			0x10 /* Northern Tel TRO call */

#endif /*RTMISDNMAINTENANCESTRUCTS_H_*/
