#ifndef CARDSDEFINES_H_
#define CARDSDEFINES_H_


#define FIRST_SUBBOARD_ID			1 // subBoardId starts from 1 - as agreed for GL1...
#define SWITCH_BBOARD_ID            5

#define CM_KEEP_ALIVE_RETRIES		60  //VNGR- 20510 changed the retries from 4 to 60
#define SM_KEEP_ALIVE_RETRIES		60  //VNGR- 20510 changed the retries from 4 to 60
//VNGR- 20510 changed the retries from 4 to 60
//VNGR- 26324 changed the retries from 60 to 4 by David Liang 
#define RTM_ISDN_KEEP_ALIVE_RETRIES	4  	
#define ALL_DSP_UNIT_ID				0xFFFFFFFF  //NINJA: Rescue Card

//#define MEDIA_RECORDING_PHYSICAL_FILE_PATH	((std::string)(MCU_OUTPUT_DIR+"/rec")
#define MEDIA_RECORDING_FILE_PATH	        ((std::string)(MCU_MCMS_DIR+"/MediaRecording"))

#define SYSTEM_CARDS_MODE_DIR_FULL_PATH       ((std::string)(MCU_MCMS_DIR+"/StaticStates/"))
#define SYSTEM_CARDS_MODE_FILE_FULL_PATH      ((std::string)(MCU_MCMS_DIR+"/StaticStates/SystemCardsMode.txt"))

#define SYSTEM_CARDS_MODE_MPM_VAL		"CARDS_MODE_MPM"
#define SYSTEM_CARDS_MODE_MPM_PLUS_VAL	"CARDS_MODE_MPM+"
#define SYSTEM_CARDS_MODE_MPMX_VAL	    "CARDS_MODE_MPMX"
#define SYSTEM_CARDS_MODE_MPMRX_VAL      "CARDS_MODE_MPMRX"
//#define SYSTEM_CARDS_MODE_MIXED_MODE_VAL      "CARDS_MODE_MIXED_MODE"
#define SYSTEM_CARDS_MODE_MPMRX_ONLY_VAL      "CARDS_MODE_MPMRX_ONLY"
#define SYSTEM_CARDS_MODE_DEFAULT_VAL	SYSTEM_CARDS_MODE_MPMRX_VAL

#define SYSTEM_CARDS_MODE_STR_LEN		25	// ("CARDS_MODE_MPM" or "CARDS_MODE_MPM+")

#define MPM_BLOCKING_ALERT_DESCRIPTION			"MPM cards are disabled with this version"
#define MPM_BLOCKING_ALERT_USER_ID				465 // an identifier for the Alert
#define SINGLE_MEDIA_SECOND_SLOT_ALERT_USER_ID	789 // an identifier for the Alert

// approved descriptions for user messages from Emb
#define EMB_USE_MSG_DESC_ILLEGAL				"USer Msg Emb - Illegal message code"
#define EMB_USE_MSG_DESC_TEST					"USer Msg Emb - Test"
#define EMB_USE_MSG_DESC_UBOOT_FLASH_FAILURE	"UBoot Flash Failure"
#define EMB_USE_MSG_DESC_FPGA_VERSION_LOAD_FAILURE "An older version was loaded to the FPGA. RMX is still usable, however, please contact support."
#define EMB_USE_MSG_DESC_FAN_SPEED_BELOW_MIN	"Fan speed is below the minimum"
#define EMB_USE_MSG_DESC_FSM_FAN_NO_POWER		"FSM power to fans problem"
#define EMB_USE_MSG_DESC_FSM_NO_CARD			"No FSM card"
#define FAILED_TO_LOAD_FSM_4000_SOFTWARE	            "Failed to load FSM 4000 software"
#define NO_LINK_BETWEEN_FSM_4000_AND_CARD_IN_SLOT_1	    "No link between FSM 4000 and card in slot 1"
#define NO_LINK_BETWEEN_FSM_4000_AND_CARD_IN_SLOT_2	    "No link between FSM 4000 and card in slot 2"
#define NO_LINK_BETWEEN_FSM_4000_AND_CARD_IN_SLOT_3 	"No link between FSM 4000 and card in slot 3"
#define NO_LINK_BETWEEN_FSM_4000_AND_CARD_IN_SLOT_4 	"No link between FSM 4000 and card in slot 4"
#define CARD_RECOVERY_COMPLETED                         "Card recovery completed"
#define CARD1_NOT_SUPPORTED                             "The card type in slot 1 is not compatible with RMX version, card will not be powered on."
#define CARD2_NOT_SUPPORTED                             "The card type in slot 2 is not compatible with RMX version, card will not be powered on."
#define CARD3_NOT_SUPPORTED                             "The card type in slot 3 is not compatible with RMX version, card will not be powered on."
#define CARD4_NOT_SUPPORTED                             "The card type in slot 4 is not compatible with RMX version, card will not be powered on."
#define CARD_NOT_SUPPORTED_OLD_PSU                      "Power Supply should be upgraded in order that the MPM RX card will be functional"
#define CARD_NOT_SUPPORTED_OLD_CNTL                     "CPU card should be upgraded in order that the MPM RX card will be functional"

 
// bitmasks for tedermining smCompProblem
#define SM_COMP_CLEAR_BITMASK                   0
#define SM_COMP_OTHER_BITMASK					0x1		// 0----------1
#define SM_COMP_VOLTAGE_BITMASK					0x2		// 0---------10
#define SM_COMP_TEMPERATURE_MAJOR_BITMASK		0x4		// 0--------100
#define SM_COMP_TEMPERATURE_CRITICAL_BITMASK	0x8		// 0-------1000
#define SM_COMP_FAILURE_BITMASK					0x10	// 0------10000
#define SM_COMP_POWER_OFF_BITMASK				0x20	// 0-----100000
#define SM_COMP_RTM_OR_ISDN_MISSING_BITMASK			0x40	// 0----1000000
#define MEDIA_CARD_LAN_PORT_1 1
#define MEDIA_CARD_LAN_PORT_2 2



enum eFailureType
{
	eFailureTypeUnitFailure = 0,
	eFailureTypeCardNoConnection
};

enum eShmComponentProblemType
{
	eShmCompVoltage		= 0,	// voltage is below/above the defined limits
	eShmCompTemperature	= 1,	// temperature  is below/above the defined limits
	eShmCompReset		= 2,	// MFA has gone through a reset
	eShmCompOtherProblem = 3,

	eMaxNumOfSmComponentProblemTypes
};

enum eMfaKeepAlivePhase
{
	eFirstKa		= 0,
	eChangedKa,
	eFirstKaToRsrc
};

enum eCSExtIntMsgState
{
	eStartConfig		= 0,
			eSendMsgWaitForReply,
			eConfigFailed,
			eDone,
			eIlegalState

};

enum eNGBSystemCardsMode
{
    eNGBSystemCardsMode_illegal = 0,
    eNGBSystemCardsMode_breeze_only,
    eNGBSystemCardsMode_mpmrx_only,
   // eNGBSystemCardsMode_mixed_mode,

    NUM_OF_NGB_SYSTEM_CARDS_MODES

};

static const char *sNGBSystemCardsModes[] =
{
    "illegal_mode",       // eNGBSystemCardsMode_illegal
    "mpmx",               // eNGBSystemCardsMode_breeze_only
    "mpmrx",              // eNGBSystemCardsMode_mpmrx_only
   // "mixed_mode",         // eNGBSystemCardsMode_mixed_mode
};

/*typedef struct
{
    DWORD boardID;
    DWORD unitID;
} RESCUE_CARD_REQ_S;*/

#endif /*CARDSDEFINES_H_*/
