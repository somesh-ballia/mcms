#include <sstream>
#include <stdio.h>
#include "CDRUtils.h"
#include "H239Defines.h"
#include "NonStandardCaps.h"
#include "H221.h"
#include "Trace.h"
#include "CDRDefines.h"
#include "H263.h"
#include "H264.h"
#include "IpChannelParams.h"
#include "CapH263Annexes.h"
#include "PObject.h"
#include "Macros.h"
//#include "Capabilities.h"

//#include "ConfPartyH323Defines.h"

using namespace std;


#define NUMBER_OF_RESOLUTIONS    NUMBER_OF_H263_FORMATS
#define NUMBER_OF_BYTES    5
#define CUSTOM_1_BOUNDS     (NUMBER_OF_H263_FORMATS + CUSTOM_FORMAT_CAP_EQUAL_BOUNDS)
#define CUSTOM_2_BOUNDS    (NUMBER_OF_H263_FORMATS + CUSTOM_FORMAT_CAP_TWO_DISTINCT_BOUNDS)

struct H230_ENTRY  {
  BYTE        opcode;
  WORD        endMark;
  char*       opcodeStr;
  char*       descStr;
};


#define  ONH230(opcode,endmark,opcodestr,descstr) { opcode , endmark ,opcodestr , descstr } ,
#define  BEGIN_H230_TBL  static H230_ENTRY g_H230Entries[] = {
#define  END__H230_TBL { 0 , 1, "UNKNOWN" , "UNKNOWN" } };


#define    _MCC      Cancel_MCC
#define    _MIZ      Cancel_MIZ
#define    _MIS      Cancel_MIS
#define    _MCV      Cancel_MCV
#define    _MIV      Cancel_MIV
#define    _VCS      Cancel_VCS
#define    _MMS      Cancel_MMS


BEGIN_H230_TBL
                    // opcode related to audio
  ONH230(AIM | Attr000,0,"AIM",  "Audio indicate muted")
  ONH230(AIA | Attr000,0,"AIA",  "Audio indicate active")
  ONH230(ACE | Attr000,0,"ACE",  "Audio command equalize")
  ONH230(ACZ | Attr000,0,"ACZ",  "Audio zero delay")
                    // opcode related to video
  ONH230(VIS | Attr000,0,"VIS",  "Video indicate suppressed")
  ONH230(VIA | Attr000,0,"VIA",  "Video indicate active")
                    // opcode related to chair control
  ONH230(CIC | Attr010,0,"CIC",  "Chair control capbility")
  ONH230(CCA | Attr010,0,"CCA",  "Chair token acquire")
  ONH230(CIS | Attr010,0,"CIS",  "Chair token release")
  ONH230(CIR | Attr010,0,"CIR",  "Chair indicate releas/refuse terminal drop")
  ONH230(CIT | Attr010,0,"CIT",  "Chair indicate token")
  ONH230(CCR | Attr010,0,"CCR",  "Chair token releas/refuse")
  ONH230(CCD | Attr010,0,"CCD",  "Chair drop terminal")
  ONH230(CCK | Attr010,0,"CCK",  "Chair drop conference")
  ONH230(MIJ | Attr010,0,"MIJ",  "Multipoint Indicate Join Real Conference")
  ONH230(TIE | Attr010,0,"TIE",  "Terminal Indicate End of Listing")
  ONH230(TIF | Attr010,0,"TIF",  "Request for floor")
  ONH230(TCU | Attr001,0,"TCU",  "Update terminal list")
  ONH230(TIA | Attr001,0,"TIA",  "Assign terminal number")
  ONH230(TIN | Attr001,0,"TIN",  "Indicate terminal number")
  ONH230(TID | Attr001,0,"TID",  "Dropped  terminal number")
  ONH230(TCA | Attr001,0,"TCA",  "Token command association")
  ONH230(MIH | Attr001,0,"MIH",  "Multipoint Indicate Hierarchy")

  ONH230(VCB | Attr001,0,"VCB",  "Chair video command broadcast")
  ONH230(VCE | Attr001,0,"VCE",  "Chair video cancel command broadcast")
    // cancelVCB ?????
                    // opcodes related to video forces
  ONH230(MVC | Attr010,0,"MVC",  "Multipoint Visualization Capability")
  ONH230(MVA | Attr010,0,"MVA",  "Multipoint Visualization Achieved")
  ONH230(MVR | Attr010,0,"MVR",  "Multipoint Visualization Refused/Revoked")
                    // opcode related to data control
  ONH230(DCA_L | Attr010,0,"DCA_L",  "Lsd acquire token")
  ONH230(DCA_H | Attr010,0,"DCA_H",  "Hsd acquire token")
  ONH230(DIT_L | Attr010,0,"DIT_L",  "Lsd indicate token")
  ONH230(DIT_H | Attr010,0,"DIT_H",  "Hsd indicate token")
  ONH230(DIS_L | Attr010,0,"DIS_L",  "Lsd stop using token")
  ONH230(DIS_H | Attr010,0,"DIS_H",  "Hsd stop using token")
  ONH230(DCR_L | Attr010,0,"DCR_L",  "Lsd releas/refuse token")
  ONH230(DCR_H | Attr010,0,"DCR_H",  "Hsd releas/refuse token")
  ONH230(DCC_L | Attr010,0,"DCC_L",  "Lsd close channel")

                    // opcode related to terminal identification
  ONH230(TCI   | Attr000,0,"TCI",  "Terminal command identify")
  ONH230(TII   | Attr000,0,"TII",  "Terminal indicate identity")
  ONH230(TIS   | Attr000,0,"TIS",  "Terminal indicate identity stop")
  ONH230(TCS_0 | Attr011,0,"TCS_0","Terminal string reservrd")
  ONH230(TCS_1 | Attr011,0,"TCS_1","Terminal string password")
  ONH230(TCS_2 | Attr011,0,"TCS_2","Terminal string identity")
  ONH230(TCS_3 | Attr011,0,"TCS_3","Terminal string conf identify")
  ONH230(TCS_4 | Attr011,0,"TCS_4","Terminal extension address")
  ONH230(TCP_  | Attr011,0,"TCP_", "Terminal personal identify")
                    // opcode related to broudcast control
  ONH230(MCV  | Attr001,0,"MCV" ,"Multipoint Visualize Force")
  ONH230(_MCV | Attr001,0,"_MCV","Cancel Multipoint Visualize Force")
  ONH230(MIV  | Attr001,0,"MIV" ,"Multipoint Visualize Indication")
  ONH230(_MIV | Attr001,0,"_MIV","Cancel Multipoint Visualize Indication")
                    // opcode related to select control
  ONH230(VCS  | Attr001,0,"VCS" ,"Multipoint Visualize Force")
  ONH230(_VCS | Attr001,0,"_VCS","Cancel Multipoint Visualize Force")
  ONH230(VCR  | Attr001,0,"VCR","Reject video enforce")
                    // opcode related to cascading
  ONH230(MIL  | Attr001,0,"MIL" ,"Multipoint indicate loop")
  ONH230(MIM  | Attr001,0,"MIM" ,"Multipoint indicate master")
  ONH230(Cancel_MIM |Attr001,0,"Cancel_MIM","Cancel Multipoint indicate master")
  ONH230(RAN  | Attr001,0,"RAN" ,"Random number")
                    // opcode related to other
  ONH230(VIN | Attr001,0,"VIN","Indicate video src number")
  ONH230(MIZ | Attr001,0,"MIZ","Multipoint zero communication")
  ONH230(MIS | Attr001,0,"MIS","Multipoint secondery status")

  ONH230(_MCC | Attr001,0,"Cancel_MCC","Cancel multipoint command conference")
  ONH230(_MIZ | Attr001,0,"Cancel_MIZ","Cancel multipoint command symmetrical")
  ONH230(_MIS | Attr001,0,"Cancel_MIS","Cancel multipoint secondery status")
  ONH230(_MMS | Attr001,0,"Cancel_MMS","Multipoint command mode asymmetrize")

  ONH230(DCM  | Attr010,0,"DCM","Data command MLP")
  ONH230(MCS  | Attr001,0,"MCS","Multipoint command symmetrical")
  ONH230(MCC  | Attr001,0,"MCC","Multipoint command conference")
  ONH230(MMS  | Attr001,0,"MMS","Multipoint command mode symmetrize")
  ONH230(h239ControlCapability  | Attr101,0,"h239ControlCapability","h239 Control Capability")

END__H230_TBL


// AUDIO_CAP
    typedef struct
	{
        BYTE Is_Alaw;
        BYTE Is_Ulaw;
        BYTE Is_G711;
        BYTE Is_G722_1_24;
        BYTE Is_G722_1_32;
        BYTE Is_G722_48;
        BYTE Is_G722_64;
        BYTE Is_G728;
        BYTE Is_G722_1_C_24;
        BYTE Is_G722_1_C_32;
        BYTE Is_G722_1_C_48;
    } AUDIO_CAP;

// VIDEO_CAP
	typedef struct
	{
        BYTE Is_CIF;
        BYTE CIF_rate;
        BYTE Is_QCIF;
        BYTE QCIF_rate;
        BYTE Is_ISO;
    } VIDEO_CAP;

// nsCap
	typedef struct
	{
        BYTE msgLen;
        BYTE country1;
        BYTE country2;
        BYTE manufact1;
        BYTE manufact2;
        BYTE data[251];
    } nsCap;


/////////////////////////////////////////////////////////////////////////////
static char*  audXfer[32] = {
    "Neutral"                     ,//   0
    "A_Law"                       ,//   1
    "U_Law"                       ,//   2
    "G722_64"                     ,//   3
    "G722_48"                     ,//   4
    "Au_16k"                      ,//   5
    "Au_Iso"                      ,//   6
    "SM_comp"                     ,//   7
    "Xfer_Cap_128"                ,//   8
    "Xfer_Cap_192"                ,//   9
    "Xfer_Cap_256"                ,//   10
    "Xfer_Cap_320"                ,//   11
    "Xfer_Cap_512"                ,//   12
    "Xfer_Cap_768"                ,//   13
    "UNKNOWN"                     ,//   14
    "Xfer_Cap_1152"               ,//   15
    "Xfer_Cap_B"                  ,//   16
    "Xfer_Cap_2B"                 ,//   17
    "Xfer_Cap_3B"                 ,//   18
    "Xfer_Cap_4B"                 ,//   19
    "Xfer_Cap_5B"                 ,//   20
    "fer_Cap_6B"                  ,//   21
    "Xfer_Cap_Restrict"           ,//   22
    "Xfer_Cap_6B_H0_Comp"         ,//   23
    "Xfer_Cap_H0"                 ,//   24
    "Xfer_Cap_2H0"                ,//   25
    "Xfer_Cap_3H0"                ,//   26
    "Xfer_Cap_4H0"                ,//   27
    "Xfer_Cap_5H0"                ,//   28
    "Xfer_Cap_1472"               ,//   29
    "Xfer_Cap_H11"                ,//   30
    "Xfer_Cap_H12"                 //   31
};

/////////////////////////////////////////////////////////////////////////////
static char*  datVid[32] = {
    "Ter2_Var_Lsd"                ,//   0
    "Dxfer_Cap_300"               ,//   1
    "Dxfer_Cap_1200"              ,//   2
    "Dxfer_Cap_4800"              ,//   3
    "Dxfer_Cap_6400"              ,//   4
    "Dxfer_Cap_8000"              ,//   5
    "Dxfer_Cap_9600"              ,//   6
    "Dxfer_Cap_14400"             ,//   7
    "Dxfer_Cap_16k"               ,//   8
    "Dxfer_Cap_24k"               ,//   9
    "Dxfer_Cap_32k"               ,//   10
    "Dxfer_Cap_40k"               ,//   11
    "Dxfer_Cap_48k"               ,//   12
    "Dxfer_Cap_56k"               ,//   13
    "Dxfer_Cap_62_4k"             ,//   14
    "Dxfer_Cap_64k"               ,//   15
    "Dxfer_Cap_Mlp_4k"            ,//   16
    "Dxfer_Cap_Mlp_6_4k"          ,//   17
    "Var_Mlp"                     ,//   18
    "Mlp_Set_1"                   ,//   19
    "V_Qcif"                      ,//   20
    "V_Cif"                       ,//   21
    "V_1_29_97"                   ,//   22
    "V_2_29_97"                   ,//   23
    "V_3_29_97"                   ,//   24
    "V_4_29_97"                   ,//   25
    "H263_2000"                   ,//   26
    "Vid_Iso"                     ,//   27
    "Mlp_Set_2"                   ,//   28
    "Esc_Cf_R"                    ,//   29
    "Encryp_Cap"                  ,//   30
    "Mbe_Cap"                      //   31
};

/////////////////////////////////////////////////////////////////////////////
static char*  hsdHmlp[32] = {
    "HSD-off"                     ,//   0
    "var-HSD"                     ,//   1
    "H-MLP-62.4"                  ,//   2
    "H-MLP-64k"                   ,//   3
    "H-MLP-128k"                  ,//   4
    "H-MLP-192k"                  ,//   5
    "H-MLP-256k"                  ,//   6
    "H-MLP-320k"                  ,//   7
    "H-MLP-384k"                  ,//   8
    " "                           ,//   9
    " "                           ,//   10
    " "                           ,//   11
    "H-MLP-14.4k"                 ,//   12
    "var-H-MLP"                   ,//   13
    " "                           ,//   14
    " "                           ,//   15
    " "                           ,//   16
    "HSD-64k"                     ,//   17
    "HSD-128k"                    ,//   18
    "HSD-192k"                    ,//   19
    "HSD-256k"                    ,//   20
    "HSD-320k"                    ,//   21
    "HSD-384k"                    ,//   22
    "HSD-512k"                    ,//   23
    "HSD-768k"                    ,//   24
    "HSD-1152k"                   ,//   25
    "HSD-1536k"                   ,//   26
    " "                           ,//   27
    " "                           ,//   28
    " "                           ,//   29
    " "                           ,//   30
    " "                            //   31
};

/////////////////////////////////////////////////////////////////////////////
static char*  Mlp[32] = {
    "MLP-14.4k"                   ,//   0
    "MLP-22.4k"                   ,//   1
    "MLP-30.4k"                   ,//   2
    "MLP-38.4k"                   ,//   3
    "MLP-46.4k"                   ,//   4
    "(R)"                         ,//   5
    "MLP-62.4k"                   ,//   6
    "(R)"                         ,//   7
    "MLP-16k"                     ,//   8
    "MLP-24k"                     ,//   9
    "MLP-32k"                     ,//   10
    "MLP-40k"                     ,//   11
    "(R)"                         ,//   12
    "(R)"                         ,//   13
    "MLP-64k"                     ,//   14
    " "                           ,//   15
    " "                           ,//   16
    " "                           ,//   17
    " "                           ,//   18
    " "                           ,//   19
    " "                           ,//   20
    " "                           ,//   21
    " "                           ,//   22
    " "                           ,//   23
    " "                           ,//   24
    " "                           ,//   25
    " "                           ,//   26
    " "                           ,//   27
    " "                           ,//   28
    " "                           ,//   29
    " "                           ,//   30
    " "                            //   31
};


/////////////////////////////////////////////////////////////////////////////
static char*  restrict[32] = {
    "Restrict_L"                  ,//   0
    "Restrict_P"                  ,//   1
    "NoRestrict"                  ,//   2
    "G723_1"                      ,//   3
    "G729"                        ,//   4
    "G722.1/32"                   ,//   5
    "G722.1/24"                   ,//   6
    "(R)"                         ,//   7
    "(R)"                         ,//   8
    "(R)"                         ,//   9
    "(R)"                         ,//   10
    "(R)"                         ,//   11
    "(R)"                         ,//   12
    "(R)"                         ,//   13
    "(R)"                         ,//   14
    "(R)"                         ,//   15
    "(R)"                         ,//   16
    "(R)"                         ,//   17
    "(R)"                         ,//   18
    "(R)"                         ,//   19
    "(R)"                         ,//   20
    "(R)"                         ,//   21
    "(R)"                         ,//   22
    "(R)"                         ,//   23
    "(R)"                         ,//   24
    "(R)"                         ,//   25
    "(R)"                         ,//   26
    "(R)"                         ,//   27
    "(R)"                         ,//   28
    "(R)"                         ,//   29
    "ns_cap"                      ,//   30
    "ns_com"                       //   31
};

/////////////////////////////////////////////////////////////////////////////
static char*  LsdHsdMlp[32] = {
    "(R) ISO-SP baseline on LSD"  ,//   0
    "(R) ISO-SP baseline on HSD"  ,//   1
    "(R) ISO-SP spatial"          ,//   2
    "(R) ISO-SP progressive"      ,//   3
    "(R) ISO-SP arithmetic"       ,//   4
    "UNKNOWN"                     ,//   5
    "UNKNOWN"                     ,//   6
    "UNKNOWN"                     ,//   7
    "UNKNOWN"                     ,//   8
    "Still image (Rec. H.261) "   ,//   9
    "Graphics cursor (R)"         ,//   10
    "UNKNOWN "                    ,//   11
    "UNKNOWN "                    ,//   12
    "UNKNOWN "                    ,//   13
    "UNKNOWN "                    ,//   14
    "UNKNOWN "                    ,//   15
    "(R) Group 3 fax"             ,//   16
    "(R) Group 4 fax"             ,//   17
    "UNKNOWN"                     ,//   18
    "UNKNOWN"                     ,//   19
    "V.120 LSD"                   ,//   20
    "V.120 HSD"                   ,//   21
    "V.14_LSD"                    ,//   22
    "V.14_HSD"                    ,//   23
    "H.224_MLP"                   ,//   24
    "H.224_LSD"                   ,//   25
    "H.224_HSD"                   ,//   26
    "H.224-sim"                   ,//   27
    "T.120-cap"                   ,//   28
    "Nil_Data"                    ,//   29
    "H.224-token"                 ,//   30
    "UNKNOWN"                      //   31
};



typedef enum
{
	kRolePeople					= 0,
	kRoleContent				= 1,	//0001
	//structures with the following roles will be sent as extendedVideoCapability:
	kRolePresentation			= 2,	//0010
	kRoleContentOrPresentation	= 3,	//0011 // Don't change value!! kRoleContent | kRolePresentation.
	kRoleLive					= 4,	//0100
	kRoleLiveOrPresentation		= 6		//0110 // Don't change value!! kRoleLive | kRolePresentation.

}ERoleLabel;

static char* RoleLableArray[] = {
	"people",
	"content",
	"presentation",
	"content or presentation",
	"live",
	"live or presentation",
};

static char* FormatH323Array[] = {
	"QCif",
	"Cif",
	"4Cif",
	"16Cif",
	"VGA",
	"NTSC",
	"SVGA",
	"XGA",
	"SIF",
	"QVGA",
	"SD",
	"720p",
};

//for sip conferencing limitation
typedef enum {
		kNotAnAdvancedVideoConference = 0,
		kH264VswFixed,
		kEpcVswFixed,
		kAutoVsw,
		kSoftwareCp,
		kCop,
		kQuadView
}EAdvancedVideoConferenceType;

static char* FormatConferenceType[] = {
		"Unknown",
		"VSW set to H264 fixed",
		"VSW set to EPC fixed",
		"VSW set to auto",
		"Software CP",
		"COP",
		"QUAD view"
};



///////////////////////////////////////////////////////////////////////////////////////
static void PrintH323CustomOF(std::ostream& ostr, customPic_St * pCustomFormats, int index)
{
	int xMax, yMax, xMin, yMin;

	char * pChar = (char *)pCustomFormats->customPicPtr;
	pChar += index * sizeof(customPicFormatSt);
	customPicFormatSt * pSpecificCustomFormat = (customPicFormatSt *)pChar;

	xMax = pSpecificCustomFormat->maxCustomPictureWidth;
	yMax = pSpecificCustomFormat->maxCustomPictureHeight;
	xMin = pSpecificCustomFormat->minCustomPictureWidth;
	yMin = pSpecificCustomFormat->minCustomPictureHeight;

	if((xMax >= 256) && (yMax >= 192))
		ostr << "XGA  - ";
	else if ((xMax >= 200) && (yMax >= 150))
		ostr << "SVGA - ";
	else if((xMax >= 176) && (yMax >= 120))
		ostr << "NTSC - ";
	else if((xMax >= 160) && (yMax >= 120))
		ostr << "VGA  - ";
	else if((xMax >= 88) && (yMax >= 72))
		ostr << "PAL (CIF)  - ";
	else if((xMax >= 88) && (yMax >= 60))
		ostr << "1/4 NTSC  - ";

	if(xMax != xMin)
	{
		ostr << " max width: " <<  xMax << "\n";
		ostr << " min width: " <<  xMin << "\n";
	}
	if(yMax != yMin)
	{
		ostr << " max height: " <<  yMax << "\n";
		ostr << " min height: " <<  yMin << "\n";
	}
	else
	{
		if(pSpecificCustomFormat->standardMPI)
			ostr << " Standard MPI: " << (int)pSpecificCustomFormat->standardMPI << "\n";
		else
			ostr << " Custom MPI: " << (int)pSpecificCustomFormat->customMPI << "\n";
	}
}

const char*  CCDRUtils::Get_Audio_Coding_Command_BitRate(BYTE command,unsigned short* pCBitRate)
{
	// if it's the audio parameters of H323 party remove the H323 offset for this trace function.
	if (command > H323IFTypeAddition)
		command -= H323IFTypeAddition;

	switch (command)
	{
		case Au_Neutral                : { * pCBitRate = 0   ; return "Au_Neutral";}
		case Capex                     : { * pCBitRate = 0   ; return "Capex";}
		case A_Law_OU                  : { * pCBitRate = 640 ; return "A_Law_OU";}
		case U_Law_OU                  : { * pCBitRate = 640 ; return "U_Law_OU";}
		case G722_m1                   : { * pCBitRate = 640 ; return "G722_m1";}
		case Au_Off_U                  : { * pCBitRate = 0   ; return "Au_Off_U";}
		case A_Law_OF                  : { * pCBitRate = 560 ; return "A_Law_OF";}
		case U_Law_OF                  : { * pCBitRate = 560 ; return "U_Law_OF";}
		case A_Law_48                  : { * pCBitRate = 480 ; return "A_Law_48";}
		case U_Law_48                  : { * pCBitRate = 480 ; return "U_Law_48";}
		case G722_m2                   : { * pCBitRate = 560 ; return "G722_m2";}
		case G722_m3                   : { * pCBitRate = 480 ; return "G722_m3";}
		case Au_40k                    : { * pCBitRate = 400 ; return "Au_40k";}
		case Au_32k                    : { * pCBitRate = 320 ; return "Au_32k";}
		case Au_24k                    : { * pCBitRate = 240 ; return "Au_24k";}
		case Au_G7221_16k              : { * pCBitRate = 160 ; return "Au_G7221_16";}
		case G728                      : { * pCBitRate = 160 ; return "G728";}
		case Au_8k                     : { * pCBitRate = 80  ; return "Au_8k";}
		case Au_Off_F                  : { * pCBitRate = 0   ; return "Au_Off_F";}
		/* non-standard audio mode - start */
		case Au_Siren7_16k             : { *pCBitRate = 160  ; return "Au_Siren7_16k"; }
		case Au_Siren7_24k             : { *pCBitRate = 240  ; return "Au_Siren7_24k"; }
		case Au_Siren7_32k             : { *pCBitRate = 320  ; return "Au_Siren7_32k"; }
		case Au_Siren14_24k            : { *pCBitRate = 240  ; return "Au_Siren14_24k"; }
		case Au_Siren14_32k            : { *pCBitRate = 320  ; return "Au_Siren14_32k"; }
		case Au_Siren14_48k            : { *pCBitRate = 480  ; return "Au_Siren14_48k"; }

		case Au_Siren14S_48k           : { *pCBitRate = 480  ; return "Au_Siren14S_48k"; }
		case Au_Siren14S_56k           : { *pCBitRate = 560  ; return "Au_Siren14S_56k"; }
		case Au_Siren14S_64k           : { *pCBitRate = 640  ; return "Au_Siren14S_64k"; }
		case Au_Siren14S_96k           : { *pCBitRate = 960  ; return "Au_Siren14S_96k"; }

		case Au_Siren22_32k            : { *pCBitRate = 320  ; return "Au_Siren22_32k"; }
		case Au_Siren22_48k            : { *pCBitRate = 480  ; return "Au_Siren22_48k"; }
		case Au_Siren22_64k            : { *pCBitRate = 640  ; return "Au_Siren22_64k"; }
		case Au_Siren22S_64k           : { *pCBitRate = 640  ; return "Au_Siren22S_64k"; }
		case Au_Siren22S_96k           : { *pCBitRate = 960  ; return "Au_Siren22S_96k"; }
		case Au_Siren22S_128k          : { *pCBitRate = 1280 ; return "Au_Siren22S_128k"; }

		case Au_SirenLPR_32k           : { *pCBitRate = 320  ; return "Au_SirenLPR_32k"; }
		case Au_SirenLPR_48k           : { *pCBitRate = 480  ; return "Au_SirenLPR_48k"; }
		case Au_SirenLPR_64k           : { *pCBitRate = 640  ; return "Au_SirenLPR_64k"; }
		case Au_SirenLPRS_64k          : { *pCBitRate = 640  ; return "Au_SirenLPRS_64k"; }
		case Au_SirenLPRS_96k          : { *pCBitRate = 960  ; return "Au_SirenLPRS_96k"; }
		case Au_SirenLPRS_128k         : { *pCBitRate = 1280 ; return "Au_SirenLPRS_128k"; }
		// TIP
		case Au_AAC_LD                 : { *pCBitRate = 2560 ; return "AAC_LD";}

		// Scalable audio
		case Au_SirenLPR_Scalable_32k  : { *pCBitRate = 320  ; return "Au_SAC_32k"; }
		case Au_SirenLPR_Scalable_48k  : { *pCBitRate = 480  ; return "Au_SAC_48k"; }
		case Au_SirenLPR_Scalable_64k  : { *pCBitRate = 640  ; return "Au_SAC_64k"; }
		case Au_SirenLPRS_Scalable_64k : { *pCBitRate = 640  ; return "Au_SACStereo_64k"; }
		case Au_SirenLPRS_Scalable_96k : { *pCBitRate = 960  ; return "Au_SACStereo_96k"; }
		case Au_SirenLPRS_Scalable_128k: { *pCBitRate = 1280 ; return "Au_SACStereo_128k"; }

		// iLBC
		case Au_iLBC_13k               : { *pCBitRate = 133  ; return "Au_iLBC_13k"; }
		case Au_iLBC_15k               : { *pCBitRate = 152  ; return "Au_iLBC_15k"; }

	        // Opus
		case Au_Opus_64k				: { *pCBitRate = 640; return "Au_Opus_64k"; }
		case Au_OpusStereo_128k			: { *pCBitRate = 1280; return "Au_OpusStereo_128k"; }

		/* non-standard audio mode - end */
		case G7221_AnnexC_24k          : { *pCBitRate = 240  ; return "G7221_AnnexC_24k"; }
		case G7221_AnnexC_32k          : { *pCBitRate = 320  ; return "G7221_AnnexC_32k"; }
		case G7221_AnnexC_48k          : { *pCBitRate = 480  ; return "G7221_AnnexC_48k"; }
		case G719_32k                  : { *pCBitRate = 320  ; return "G719_32k"; }
		case G719_48k                  : { *pCBitRate = 480  ; return "G719_48k"; }
		case G719_64k                  : { *pCBitRate = 640  ; return "G719_64k"; }
		case G719S_64k                 : { *pCBitRate = 640  ; return "G719S_64k"; }
		case G719S_96k                 : { *pCBitRate = 960  ; return "G719S_96k"; }
		case G719S_128k                : { *pCBitRate = 1280 ; return "G719S_128k"; }
		case G729_8k                   : { *pCBitRate = 80   ; return "G729_8k";}
		case Au_G722_Stereo_128        : { *pCBitRate = 1280 ; return "G722Stereo_128k";}

		//       case Au_Iso_64 :
		//       case Au_Iso_128 :
		//       case Au_Iso_192 :
		case Au_Iso_256 :
		case Au_Iso_384 :
		default:
		{
			FPTRACE(eLevelInfoNormal,"::Get_Audio_Coding_Command_BitRate : illegal bit rate");
			DBGFPASSERT(1);
			return "";
		}
	}
}


//=============================================================================
const char*  CCDRUtils::Get_Transfer_Rate_Command_BitRate(BYTE command,unsigned short* pCBitRate)
{
	switch(command)
	{
		case Xfer_64   : { * pCBitRate = 640 ; return "Xfer_64";}
		case Xfer_2x64 : { * pCBitRate = 2*640 ; return "Xfer_2x64";}
		case Xfer_3x64 : { * pCBitRate = 3*640 ; return "Xfer_3x64";}
		case Xfer_4x64 : { * pCBitRate = 4*640 ; return "Xfer_4x64";}
		case Xfer_5x64 : { * pCBitRate = 5*640 ; return "Xfer_5x64";}
		case Xfer_6x64 : { * pCBitRate = 6*640 ; return "Xfer_6x64";}
		case Xfer_384  : { * pCBitRate = 3840 ; return "Xfer_384";}
		case Xfer_2x384: { * pCBitRate = 2*3840 ; return "Xfer_2x384";}
		case Xfer_3x384: { * pCBitRate = 3*3840 ; return "Xfer_3x384";}
		case Xfer_4x384: { * pCBitRate = 4*3840 ; return "Xfer_4x384";}
		case Xfer_5x384: { * pCBitRate = 5*3840 ; return "Xfer_5x384";}
		case Xfer_1536 : { * pCBitRate = 15360 ; return "Xfer_1536";}
		case Xfer_1920 : { * pCBitRate = 19200 ; return "Xfer_1920";}
		case Xfer_128  : { * pCBitRate = 1280 ; return "Xfer_128";}
		case Xfer_192  : { * pCBitRate = 1920 ; return "Xfer_192";}
		case Xfer_256  : { * pCBitRate = 2560 ; return "Xfer_256";}
		case Xfer_320  : { * pCBitRate = 3200 ; return "Xfer_320";}
		case Loss_ic   : { * pCBitRate = 0 ; return "Loss_ic";}
		case Channel_2 : { * pCBitRate = 0 ; return "Channel_2";}
		case Channel_3 : { * pCBitRate = 0 ; return "Channel_3";}
		case Channel_4 : { * pCBitRate = 0 ; return "Channel_4";}
		case Channel_5 : { * pCBitRate = 0 ; return "Channel_5";}
		case Channel_6 : { * pCBitRate = 0 ; return "Channel_6";}
		case Xfer_512  : { * pCBitRate = 5120 ; return "Xfer_512";}
		case Xfer_768  : { * pCBitRate = 7680 ; return "Xfer_768";}
		case Xfer_1152 : { * pCBitRate = 11520 ; return "Xfer_1152";}
		case Xfer_1280 : { * pCBitRate = 12800 ; return "Xfer_1280";}
		case Xfer_1472 : { * pCBitRate = 14720 ; return "Xfer_1472";}
		case Xfer_1024 : { * pCBitRate = 10240 ; return "Xfer_1024";}
		case Xfer_96   : { * pCBitRate = 960 ; return "Xfer_96";}
		case Xfer_4096 : { * pCBitRate = 40960 ; return "Xfer_4096";}
		case Xfer_6144 : { * pCBitRate = 61440 ; return "Xfer_6144";}
		case Xfer_832  : { * pCBitRate = 8320 ; return "Xfer_832";}
		case Xfer_1728 : { * pCBitRate = 17280 ; return "Xfer_1728";}
		case Xfer_2048 : { * pCBitRate = 20480 ; return "Xfer_2048";}
		case Xfer_2560 : { * pCBitRate = 25600 ; return "Xfer_2560";}
		case Xfer_3072 : { * pCBitRate = 30720 ; return "Xfer_3072";}
		case Xfer_3584 : { * pCBitRate = 35840 ; return "Xfer_3584";}
		case Xfer_8192 : { * pCBitRate = 65535 ; return "Xfer_8192";}//pCBitRate = 81920 too much big for short
		default:
		{
			FPTRACE(eLevelInfoNormal,"Unknown Bitrate");
			DBGFPASSERT(1);
			return "ASSERTION FAILED";
		}
	}
}

//=============================================================================
const char*  CCDRUtils::Get_Video_Oth_Command_BitRate(BYTE command,unsigned short* pCBitRate)
{
  switch(command){
    case Video_Off : { * pCBitRate = 0 ; return "Video_Off";}
    case H261 : { * pCBitRate = 0 ; return "H261";}
	case H263 : { * pCBitRate = 0 ; return "H263";}
	case H264 : { * pCBitRate = 0 ; return "H264";}
    case Video_ISO : { * pCBitRate = 0 ; return "Video_ISO";}
  //  case AV_ISO : { * pCBitRate = 0 ; return "AV_ISO";}
    case Encryp_On : { * pCBitRate = 0 ; return "Encryp_On";}
    case Encryp_Off : { * pCBitRate = 0 ; return "Encryp_Off";}
    case Freeze_Pic : { * pCBitRate = 0 ; return "Freeze_Pic";}
    case Fast_Update : { * pCBitRate = 0 ; return "Fast_Update";}
    case Au_Loop : { * pCBitRate = 0 ; return "Au_Loop";}
    case Vid_Loop : { * pCBitRate = 0 ; return "Vid_Loop";}
    case Dig_Loop : { * pCBitRate = 0 ; return "Dig_Loop";}
    case Loop_Off : { * pCBitRate = 0 ; return "Loop_Off";}
    case SM_comp : { * pCBitRate = 0 ; return "SM_comp";}
    case Not_SM_comp : { * pCBitRate = 0 ; return "Not_SM_comp";}
    case B6_H0_Comp : { * pCBitRate = 0 ; return "B6_H0_Comp";}
    case Not_B6_H0 : { * pCBitRate = 0 ; return "Not_B6_H0";}
    case Restrict : { * pCBitRate = 0 ; return "Restrict";}
    case Derestrict : { * pCBitRate = 0 ; return "Derestrict";}
    default:
		{
		FPTRACE(eLevelInfoNormal,"Unknown Bitrate");
		DBGFPASSERT(1);
		return "ASSERTION FAILED";
		}
  }
  return 0;

}

//=============================================================================

const char*  CCDRUtils::Get_Lsd_Mlp_Command_BitRate(BYTE command,unsigned short* pCBitRate)
{
  switch(command){
    // LSD consts
    case LSD_Off : { * pCBitRate = 0 ; return "LSD_Off";}
    case LSD_300 : { * pCBitRate = 3 ; return "LSD_300";}
    case LSD_1200 : { * pCBitRate = 12 ; return "LSD_1200";}
    case LSD_4800 : { * pCBitRate = 48 ; return "LSD_4800";}
    case LSD_6400 : { * pCBitRate =  64 ; return "LSD_6400";}
    case LSD_8000 : { * pCBitRate = 80 ; return "LSD_8000";}
    case LSD_9600 : { * pCBitRate =  96 ; return "LSD_9600";}
    case LSD_14400 : { * pCBitRate =  144 ; return "LSD_14400";}
    case LSD_16k : { * pCBitRate = 160 ; return "LSD_16k";}
    case LSD_24k : { * pCBitRate = 240 ; return "LSD_24k";}
    case LSD_32k : { * pCBitRate = 320 ; return "LSD_32k";}
    case LSD_40k : { * pCBitRate = 400 ; return "LSD_40k";}
    case LSD_48k : { * pCBitRate = 480 ; return "LSD_48k";}
    case LSD_56k : { * pCBitRate = 560 ; return "LSD_56k";}
    case LSD_62_4k : { * pCBitRate =  624 ; return "LSD_62_4k";}
    case LSD_64k : { * pCBitRate = 640 ; return "LSD_64k";}
    case Data_Var_LSD : { * pCBitRate = 0xFFFF ; return "Data_Var_LSD";}

    // MLP consts
    case MLP_Off : { * pCBitRate = 0 ; return "MLP_Off";}
    case MLP_4k : { * pCBitRate = 40 ; return "MLP_4k";}
    case MLP_6_4k : { * pCBitRate =  64 ; return "MLP_6_4k";}
    case Data_var_MLP : { * pCBitRate = 0xFFFF ; return "Data_var_MLP";}
    case Mlp_14_4 : { * pCBitRate =  144 ; return "Mlp_14_4";}

    case Mlp_22_4 : { * pCBitRate =  224 ; return "Mlp_22_4";}
    case Mlp_30_4 : { * pCBitRate =  304 ; return "Mlp_30_4";}
    case Mlp_38_4 : { * pCBitRate =  384 ; return "Mlp_38_4";}
    case Mlp_46_4 : { * pCBitRate =  464 ; return "Mlp_46_4";}
    case Mlp_16 : { * pCBitRate = 160 ; return "Mlp_16";}
    case Mlp_24 : { * pCBitRate = 240 ; return "Mlp_24";}
    case Mlp_32 : { * pCBitRate = 320 ; return "Mlp_32";}
    case Mlp_40 : { * pCBitRate = 400 ; return "Mlp_40";}
    case Mlp_62_4 : { * pCBitRate =  624 ; return "Mlp_62_4";}
    case Mlp_64 : { * pCBitRate = 640 ; return "Mlp_64";}
    default:
		{

			FPTRACE(eLevelInfoNormal,"Unknown Bitrate");
			DBGFPASSERT(1);
			return "ASSERTION FAILED";
		}
  }
}

//=============================================================================
const char*  CCDRUtils::Get_Lsd_Hsd_Mlp_Command(BYTE h221Array)
{
    BYTE command = h221Array & 0x1F;

	if ((h221Array & 0xE0) == OTHRCMDATTR) // attr 010
	{
		switch(command)
		{
		case H224_MLP_Off : { return "H224_MLP_Off";}
		case H224_LSD_Off : { return "H224_LSD_Off";}
		case H224_HSD_Off : { return "H224_HSD_Off";}
		case T120_Off : { return "T120_Off";}
		case H224_Token_Off : { return "H224_Token_Off";}

		default:
			{
		     FPTRACE(eLevelInfoNormal,"Unknown Command");
		     DBGFPASSERT(1);
		      return "ASSERTION FAILED";
			}
		}
	}

	if ((h221Array & 0xE0) == LSDMLPCMDATTR) // attr 011
	{
		switch(command)
		{
		case Iso_Sp_On_Lsd : { return "Iso_Sp_On_Lsd";}
		case Iso_Sp_On_Hsd : { return "Iso_Sp_On_Hsd";}
		case Cursor_Data_Com_Lsd : { return "Cursor_Data_Com_Lsd";}
		case Fax_On_Lsd : { return "Fax_On_Lsd";}
		case Fax_On_Hsd : { return "Fax_On_Hsd";}
		case V120_Com_Lsd : { return "V120_Com_Lsd";}
		case V120_Com_Hsd : { return "V120_Com_Hsd";}
		case V14_Com_Lsd : {  return "V14_Com_Lsd";}
		case V14_Com_Hsd : { return "V14_Com_Hsd";}
		case H224_MLP_On : { return "H224_MLP_On";}
		case H224_LSD_On : { return "H224_LSD_On";}
		case H224_HSD_On : { return "H224_HSD_On";}
		case T120_ON : { return "T120_ON";}
		default:
			{
		    FPTRACE(eLevelInfoNormal,"Unknown Command");
		    DBGFPASSERT(1);
		    return "ASSERTION FAILED";
			}
		}
	}

	return "Unknown!!!";
}

//=============================================================================
const char*  CCDRUtils::Get_Content_command_Bitrate(BYTE command,unsigned short* pCBitRate)
{
	switch(command) {
	case  AMSC_0k	: { *pCBitRate = 0; return "0k";}
	case  AMSC_64k	: { *pCBitRate = 640; return "64k";}
	case AMSC_128k : { *pCBitRate = 1280; return "128k";}
	case AMSC_192k : { *pCBitRate = 1920; return "192k";}
	case AMSC_256k : { *pCBitRate = 2560; return "256k";}
	case AMSC_384k : { *pCBitRate = 3840; return "384k";}
	case AMSC_512k : { *pCBitRate = 5120; return "512k";}
	case AMSC_768k : { *pCBitRate = 7680; return "768k";}
	case AMSC_1152k : { *pCBitRate = 11520; return "1152k";}
	case AMSC_1536k  : { *pCBitRate = 15360; return "1536k";}
		 default:
		{
			FPTRACE(eLevelInfoNormal,"::Get_Content_command_Bitrate - Unknown Content Rate!");
			DBGFPASSERT(1);

		}

	}
    return "Unknown Content Rate";
}

const char*  CCDRUtils::Get_Content_command_BitrateH239(BYTE command,unsigned short* pCBitRate)
{
	switch(command) {
	case AMC_0k	  : { *pCBitRate = 0; return "0k";}
	case AMC_40k  : { *pCBitRate = 400; return "40k";}
	case AMC_64k  : { *pCBitRate = 640; return "64k";}
	case AMC_96k  : { *pCBitRate = 960; return "96k";}
	case AMC_128k : { *pCBitRate = 1280; return "128k";}
	case AMC_192k : { *pCBitRate = 1920; return "192k";}
	case AMC_256k : { *pCBitRate = 2560; return "256k";}
	case AMC_384k : { *pCBitRate = 3840; return "384k";}
	case AMC_512k : { *pCBitRate = 5120; return "512k";}
	case AMC_768k : { *pCBitRate = 7680; return "768k";}
		 default:
		{

			FPTRACE(eLevelInfoNormal,"::Get_Content_command_BitrateH239 - Unknown Content Rate!");
			DBGFPASSERT(1);
		}

	}
    return "Unknown Content Rate";
}

const char*  CCDRUtils::Get_Hsd_Hmlp_command_Bitrate(BYTE command,unsigned short* pCBitRate)
{
  switch(command){
    // HSD consts:
    case Hsd_Com_Off : { * pCBitRate = 0 ; return "Hsd_Com_Off";}
    case Var_Hsd_Com_R : { * pCBitRate = 0xFFFF ; return "Var_Hsd_Com_R";}
    case Hxfer_Com_64k : { * pCBitRate = 640 ; return "Hxfer_Com_64k";}
    case Hxfer_Com_128k : { * pCBitRate = 1280 ; return "Hxfer_Com_128k";}
    case Hxfer_Com_192k : { * pCBitRate = 1920 ; return "Hxfer_Com_192k";}
    case Hxfer_Com_256k : { * pCBitRate = 2560 ; return "Hxfer_Com_256k";}
    case Hxfer_Com_320k : { * pCBitRate = 3200 ; return "Hxfer_Com_320k";}
    case Hxfer_Com_384k : { * pCBitRate = 3840 ; return "Hxfer_Com_384k";}
    case Hxfer_Com_512k_R : { * pCBitRate = 5120 ; return "Hxfer_Com_512k_R";}
    case Hxfer_Com_768k_R : { * pCBitRate = 7680 ; return "Hxfer_Com_768k_R";}
    case Hxfer_Com_1152k_R : { * pCBitRate = 11520 ; return "Hxfer_Com_1152k_R";}
    case Hxfer_Com_1536k_R : { * pCBitRate = 15360 ; return "Hxfer_Com_1536k_R";}
      // HMLP consts:

    case H_Mlp_Com_62_4 : { * pCBitRate =  624 ; return "H_Mlp_Com_62_4";}
    case H_Mlp_Com_64 : { * pCBitRate = 640 ; return "H_Mlp_Com_64";}
    case H_Mlp_Com_128 : { * pCBitRate = 1280 ; return "H_Mlp_Com_128";}
    case H_Mlp_Com_192 : { * pCBitRate = 1920 ; return "H_Mlp_Com_192";}
    case H_Mlp_Com_256 : { * pCBitRate = 2560 ; return "H_Mlp_Com_256";}
    case H_Mlp_Com_320 : { * pCBitRate = 3200 ; return "H_Mlp_Com_320";}
    case H_Mlp_Com_384 : { * pCBitRate = 3840 ; return "H_Mlp_Com_384";}
    case H_Mlp_Com_14_4 : { * pCBitRate =  144 ; return "H_Mlp_Com_14_4";}
    case Var_H_Mlp_Com_R : { * pCBitRate = 0xFFFF; return "Var_H_Mlp_Com_R";}
    case H_Mlp_Off_Com : { * pCBitRate = 0 ; return "H_Mlp_Off_Com";}
    default:
		{
			FPASSERTMSG(TRUE,"Unknown Bitrate");
			DBGFPASSERT(1);
			return "ASSERTION FAILED";
		}
  }
}

/* try to remove */
/////////////////////////////////////////////////////////////////////////////
//dump of ONLY VIDEO from H221 capabilits vector
void CCDRUtils::DumpVideoCap(BYTE *h221str,WORD length,std::ostream& ostr)
{
    BYTE capAttr,cap;
    WORD i=0;
    WORD Dump_Flag = 0;
    ostr << "\n";

    while ( i < length )  {
        cap = h221str[i++];
        capAttr = cap & 0xE0;

        switch ( capAttr ) {
            case DATAVIDCAPATTR  :  {
				if((cap ^ capAttr) < 32)
                	ostr << datVid[cap ^ capAttr];
                break;
            }
            case ESCAPECAPATTR  :  {
                if(i < length)  {
                    if( cap == (Start_Mbe | ESCAPECAPATTR) ){
                        BYTE len = 0;
                        int h263CapsStartPosition = 0;
                        BYTE table2_H230Value;
                        len = h221str[i++]-1;
                        table2_H230Value = h221str[i++];
                        h263CapsStartPosition = i;

						if(table2_H230Value == H262_H263 || table2_H230Value == H264_Mbe )//H264_Mbe
						{
							//In case of mux /net sync,which causes a corrupted message, we reset the
							//length of Mbe message to the Left Length Of H221 Cap message.
							WORD LeftLenghOfH221Cap = length - h263CapsStartPosition;
							if(len > LeftLenghOfH221Cap )
							{
							  #ifdef __HIGHC__
								FPTRACE(eLevelInfoNormal,"::DumpVideoCap - Mbe 263/264 message length is corrupted!!! \'");
								DBGFPASSERT(1);
							  #endif

								len = LeftLenghOfH221Cap;
							}

							//The table2_H230Value is correct therefore we dump the message.
							if(table2_H230Value == H262_H263)
								DumpCapH263( ostr, len, (h221str + h263CapsStartPosition) );
							else if(table2_H230Value == H264_Mbe) //H264_Mbe
								DumpCapH264( ostr, len, (h221str + h263CapsStartPosition) );
						}
						else
						{
						  #ifdef __HIGHC__
							FPTRACE(eLevelInfoNormal,"::DumpVideoCap - Mbe H263/H264 message is corrupted!!!");
							DBGFPASSERT(1);
						  #endif

							ostr << "\n" << "Mbe H263/H264 message is corrupted!!!" << "\n" ;
						}

						i = i + len;
                    }

                }
                break;
            }

            default   :  {

#ifdef __HIGHC__
//                FPASSERT(1);
                ON(Dump_Flag);
                //FPTRACE(eLevelInfoHigh,"::DumpCap - \'unknown capability !!! \'");
                ostr << "unknown VIDEO capability (" << (hex) << (int) cap << ")";
#else
                ostr << "unknown VIDEO capability (" << (hex) << (int) cap << ")";
#endif
                break;
            }
        }
        ostr << "\n";
    }
// print entire array of caps in case of an unknown capability

    if(Dump_Flag && length < 100) {
        char s[16];
        WORD index;
        ostr << "Capabilities dump:" << "\n";
        WORD j  = length/8;
        if(length%8)  j++;
        for(WORD k =0; k < j; k++) {
            for(WORD l = 0; l < 8; l++) {
                index = 8*k + l;
                if(index >= length)break;
                BYTE capi = h221str[index];
                sprintf(s," %02x",capi);
                ostr << s;
            }
            ostr <<  "\n" ;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
char* CCDRUtils::GetH264ProfileAsString(WORD profileValue)
{
	char* profileString;

	switch(profileValue)
	{
		case H264_Profile_BaseLine:
		{
			profileString = "BaseLine Profile";
			break;
		}
		case H264_Profile_Main:
		{
			profileString = "Main Profile";
			break;
		}
		case H264_Profile_Extended:
		{
			profileString = "Extended Profile";
			break;
		}
		case H264_Profile_High:
		{
			profileString = "High Profile";
			break;
		}
		default :
		{
			profileString = "Unknown Profile ";
			break;
		}
	}
	return profileString;
}

/////////////////////////////////////////////////////////////////////
char* CCDRUtils::GetH264LevelAsString(BYTE levelValue)
{
	char* levelString;

	switch(levelValue)
	{
		case H264_Level_1:
		{
			levelString = "Level 1";
			break;
		}
		case H264_Level_1_1:
		{
			levelString = "Level 1.1";
			break;
		}
		case H264_Level_1_2:
		{
			levelString = "Level 1.2";
			break;
		}
		case H264_Level_1_3:
		{
			levelString = "Level 1.3";
			break;
		}
		case H264_Level_2:
		{
			levelString = "Level 2";
			break;
		}
		case H264_Level_2_1:
		{
			levelString = "Level 2.1";
			break;
		}
		case H264_Level_2_2:
		{
			levelString = "Level 2.2";
			break;
		}
		case H264_Level_3:
		{
			levelString = "Level 3";
			break;
		}
		case H264_Level_3_1:
		{
			levelString = "Level 3.1";
			break;
		}
		case H264_Level_3_2:
		{
			levelString = "Level 3.2";
			break;
		}
		case H264_Level_4:
		{
			levelString = "Level 4";
			break;
		}
		case H264_Level_4_1:
		{
			levelString = "Level 4.1";
			break;
		}
		case H264_Level_4_2:
		{
			levelString = "Level 4.2";
			break;
		}
		case H264_Level_5:
		{
			levelString = "Level 5";
			break;
		}
		case H264_Level_5_1:
		{
			levelString = "Level 5.1";
			break;
		}

			default :
		{
			levelString = "Level X By Default";
			break;
		}

	}
	return levelString;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCDRUtils::GetNewValueFromByte(BYTE byte,WORD first_bit,WORD last_bit)
{
	if((first_bit == (WORD)0) || (last_bit == (WORD)0))
	{
		if(first_bit != (WORD)0)
			FPASSERT(first_bit);
		else if(last_bit != (WORD)0)
			FPASSERT(last_bit);
		else
			FPASSERT(101);
	}
	if(first_bit > last_bit)
		FPASSERT( first_bit);

	BYTE result=0;
	BYTE check=0;
	int current_bit_number = (int)last_bit;

	for(WORD i=0;i<=(last_bit-first_bit);i++){
		check = (BYTE)1 << (8-current_bit_number);
		if( byte & check )
			result += (BYTE)1 << i;
		current_bit_number--;
	}
	return (BYTE)result;
}

/////////////////////////////////////////////////////////////////////////////
//dump of a single H221 cap value
void CCDRUtils::DumpCap(BYTE cap,std::ostream& ostr,BYTE Table)
{
    BYTE capAttr = cap & 0xE0;
	if((cap ^ capAttr) >= 32)
	{
	    FPTRACE(eLevelInfoNormal,"::DumpCap - the cap index exceed 32.");
        DBGFPASSERT(1);
	    return;
	}
	
    switch(Table){
        case 1: {// table A.1
            switch ( capAttr ) {

                case AUDRATECAPATTR  :  {
                  ostr << audXfer[cap ^ capAttr];
                  break;
                }

                case DATAVIDCAPATTR  :  {
                  ostr << datVid[cap ^ capAttr];
                  break;
                }

                case OTHERCAPATTR :  {
                  ostr << restrict[cap ^ capAttr];
                  break;
                }

                default   :  {
//                    FPASSERT(1);
                    FPTRACE(eLevelInfoNormal,"::DumpCap - unknown capability in table 1.");
                    DBGFPASSERT(1);
                    break;
                }
            }
            break;
        }

        case 2: {// table A.2
            switch ( capAttr ) {

                case HSDHMPLCAPATTR :  {
                    ostr << hsdHmlp[cap ^ capAttr];
                    break;
                }

                case MLPCAPATTR :  {
                    ostr << Mlp[cap ^ capAttr];
                    break;
                }
                default   :  {
//                    FPASSERT(1);
                    FPTRACE(eLevelInfoNormal,"::DumpCap - unknown capability in table 2.");
                    DBGFPASSERT(1);
                    break;
                }
            }
            break;
        }
        default : {
//            FPASSERT(1);
            FPTRACE(eLevelInfoNormal,"::DumpCap - unknown table.");
            DBGFPASSERT(1);
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
//smart dump of H221 capabilits vector
//creates a report of audio and video capabilites
void CCDRUtils::SmartDumpCap(BYTE *h221str,WORD length,std::ostream& ostr)
{
    BYTE capAttr,cap;
    WORD i=0;
    WORD restrict_type = 0;
    enum restrictType {NO_RES,RES_L,RES_P,RES_LP};
    enum MODE {INIT,LOOK_FOR_CIF_MPI,LOOK_FOR_QCIF_MPI};
    char *video_rate_str[4] = {"30 fps","15 fps","10 fps","7.5 fps"};

	AUDIO_CAP* audiocap = NULL;
	audiocap = new AUDIO_CAP;
	audiocap->Is_Alaw = FALSE;
	audiocap->Is_Ulaw = FALSE;
	audiocap->Is_G711 = FALSE;
	audiocap->Is_G722_1_24 = FALSE;
	audiocap->Is_G722_1_32 = FALSE;
	audiocap->Is_G722_48 = FALSE;
	audiocap->Is_G722_64 = FALSE;
	audiocap->Is_G728 = FALSE;
    audiocap->Is_G722_1_C_24 = FALSE;
    audiocap->Is_G722_1_C_32 = FALSE;
    audiocap->Is_G722_1_C_48 = FALSE;


    VIDEO_CAP* vidcap = NULL;
	vidcap = new VIDEO_CAP;
	vidcap->Is_CIF = FALSE;
	vidcap->CIF_rate = 0;
	vidcap->Is_QCIF = FALSE;
	vidcap->QCIF_rate = 0;
	vidcap->Is_ISO = FALSE;

    BYTE len = 0;
    int h263CapsStartPosition = 0;
    int h264CapsStartPosition = 0;
	int AMCCapsStartPosition = 0;
    int tempCapsStartPosition = 0;
	BYTE table2_H230Value = 0;
	WORD LeftLenghOfH221Cap;


    BYTE nsNum = 0;
    const BYTE nsCapMax = 10;

	nsCap* nsCapArray = new nsCap[nsCapMax];

	BYTE h239ControlCapfound = FALSE;
    BYTE restrict_found = FALSE;
    BYTE norestrictonly_found = FALSE;
	BYTE  h263found = NO, h264found = NO, AMCfound = NO;
	BYTE len263 = 0, len264 = 0, lenAMC = 0;

    enum MODE state = INIT ;

    while ( i < length )  {
        cap = h221str[i++];
        capAttr = cap & 0xE0;
        switch ( capAttr ) {

            case ESCAPECAPATTR : {
                if( (cap == (Start_Mbe | ESCAPECAPATTR))  && (state == INIT) ) {
                    len = h221str[i++]-1;
                    table2_H230Value = h221str[i++];
                    tempCapsStartPosition = i;

					//In case of mux /net sync,which causes a corrupted message, we reset the
					//length of H263 message to the Left Length Of H221 Cap message.
					LeftLenghOfH221Cap = length - tempCapsStartPosition;
					if(len > LeftLenghOfH221Cap )
					{

						FPTRACE(eLevelInfoNormal,"::SmartDumpCap - H263 or H264 message length is corrupted!!! \'");
						DBGFPASSERT(1);

						len = LeftLenghOfH221Cap;
					}
					if( table2_H230Value == H262_H263 ) {
						h263found = YES;
						h263CapsStartPosition = tempCapsStartPosition;
						len263 = len;
					} else if( table2_H230Value == H264_Mbe ) {
						h264found = YES;
						h264CapsStartPosition = tempCapsStartPosition;
						len264 = len;
					} else if(table2_H230Value == AMC_cap) {
						AMCfound = YES;
						AMCCapsStartPosition = tempCapsStartPosition;
						lenAMC = len;
					}

                    i = i + len;

                }
				else if( (cap == (ESCAPECAPATTR | H230_Esc))  && (state == INIT) )
				{
				    BYTE h230Bas;
					h230Bas = h221str[i++];
					if(h230Bas == (Attr101 | h239ControlCapability))
					{
						h239ControlCapfound = TRUE;
					}
				}
				else if( (cap == (ESCAPECAPATTR | Ns_Cap)) && (state == INIT) ) {
                    if( (h221str[i] < 5) || (nsNum >= nsCapMax ) ) { // if msg len < 5 skip msg or max
                        i += h221str[i] + 1;
                        break;
                    }
                    nsCapArray[nsNum].msgLen    = h221str[i++];
                    nsCapArray[nsNum].country1  = h221str[i++];
                    nsCapArray[nsNum].country2  = h221str[i++];
                    nsCapArray[nsNum].manufact1 = h221str[i++];
                    nsCapArray[nsNum].manufact2 = h221str[i++];
                    memcpy(nsCapArray[nsNum].data,h221str+i,min(nsCapArray[nsNum].msgLen-4,251));
                    for(int ind=0; ind<nsCapArray[nsNum].msgLen-4 && ind < 251; ind++ ) {
                        nsCapArray[nsNum].data[ind] = h221str[i++];
                    }
                    nsNum++;
                }  else
                    i++; //skip next cap attribute
                break;
            }
            case OTHERCAPATTR  :  {
                if (state == INIT)
                    switch (cap ^ capAttr) {
                        case NoRestrict : {
                            norestrictonly_found = TRUE;
                            break;
                        }
                        case Restrict_P : {
                            restrict_type |= RES_P;
                            break;
                        }
                        case Restrict_L : {
                            restrict_type |= RES_L;
                            break;
                        }
                        case G722_1_24 : {
                            audiocap->Is_G722_1_24 = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }
                        case G722_1_32 : {
                            audiocap->Is_G722_1_32 = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }
                        case G722_1_Annex_C_48 : {
                            audiocap->Is_G722_1_C_48 = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }
                        case G722_1_Annex_C_32 : {
                            audiocap->Is_G722_1_C_32 = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }
                        case G722_1_Annex_C_24 : {
                            audiocap->Is_G722_1_C_24 = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }


                    }
                break;
            }
            case AUDRATECAPATTR  :  {

                if (state == INIT)
                    switch (cap ^ capAttr) {

                        case  Xfer_Cap_Restrict : {
                            restrict_found = TRUE;
                            break;
                        }

                        case A_Law : {
                            audiocap->Is_Alaw = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }
                        case U_Law : {
                            audiocap->Is_Ulaw = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }
                        case G722_64 : {
                            audiocap->Is_G722_64 = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }
                        case G722_48 : {
                            audiocap->Is_G722_48 = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }
                        case Au_16k : { //G.728
                            audiocap->Is_G728 = TRUE;
                            audiocap->Is_G711 = TRUE;
                            break;
                        }
                    }
                    break;
                }

            case DATAVIDCAPATTR  :  {
                if (state == INIT)
                    switch (cap ^ capAttr) {

                        case V_Qcif : {
                            vidcap->Is_QCIF = TRUE;
                            vidcap->Is_CIF = FALSE;
                            state = LOOK_FOR_QCIF_MPI;
                            break;
                        }
                        case V_Cif : {
                            vidcap->Is_QCIF = TRUE;
                            vidcap->Is_CIF = TRUE;
                            state = LOOK_FOR_QCIF_MPI;
                            break;
                        }
                        case Vid_Iso : {
                            vidcap->Is_ISO = TRUE;
                            break;
                        }
                    }

                if (state == LOOK_FOR_QCIF_MPI)
                    switch (cap ^ capAttr) {

                        case V_1_29_97 :
                        case V_2_29_97 :
                        case V_3_29_97 :
                        case V_4_29_97 : {
                            vidcap->QCIF_rate = (cap ^ capAttr) - (V_1_29_97);
                            if (vidcap->Is_CIF == TRUE) {
                                state = LOOK_FOR_CIF_MPI;
                                continue;
                            }  else
                                state = INIT;
                            break;
                        }
                    }

                if (state == LOOK_FOR_CIF_MPI)
                    switch (cap ^ capAttr) {

                        case V_1_29_97 :
                        case V_2_29_97 :
                        case V_3_29_97 :
                        case V_4_29_97 : {
                            vidcap->CIF_rate = (cap ^ capAttr) - (V_1_29_97);
                            state = INIT;
                            break;
                        }
                    }
                break;
            }
        }
    }

    ostr << "\n---Audio---------\n";
    if (!(audiocap->Is_Alaw) && !(audiocap->Is_Ulaw)) audiocap->Is_G711=TRUE;
    if ((audiocap->Is_G711)) {
        ostr << "G.711 ";
        if (!(audiocap->Is_Alaw) && !(audiocap->Is_Ulaw)) ostr << "A-law/ -law\n";
        if ((audiocap->Is_Alaw) && (audiocap->Is_Ulaw))   ostr << "A-law/ -law\n";
        if ((audiocap->Is_Alaw)&& !(audiocap->Is_Ulaw))   ostr << "A-law only\n";
        if ((audiocap->Is_Ulaw)&& !(audiocap->Is_Alaw))   ostr << "-law only\n";
    }
    if ((audiocap->Is_G722_48))  ostr << "G.722 (64/48/56)\n";
    if ((audiocap->Is_G722_64))  ostr << "G.722 (64)\n";
    if ((audiocap->Is_G722_1_32))  ostr << "G.722.1/32\n";
    if ((audiocap->Is_G722_1_24))  ostr << "G.722.1/24\n";
    if ((audiocap->Is_G728))     ostr << "G.728\n";
    if ((audiocap->Is_G722_1_C_48))  ostr << "G.722.1_annex_C/48\n";
    if ((audiocap->Is_G722_1_C_32))  ostr << "G.722.1_annex_C/32\n";
    if ((audiocap->Is_G722_1_C_24))  ostr << "G.722.1_annex_C/24\n";

    ostr << "---Video---------\n";
    if (!(vidcap->Is_CIF) && !(vidcap->Is_QCIF))  ostr << "NONE\n";
    else ostr << "  [H.261]:\n" ;
    if ((vidcap->Is_QCIF) && !(vidcap->Is_CIF))
        ostr << "QCIF at " << video_rate_str[vidcap->QCIF_rate] << "\n";
    if ((vidcap->Is_CIF))     {
        ostr << "CIF  at " << video_rate_str[vidcap->CIF_rate]  << "\n";
        ostr << "QCIF at " << video_rate_str[vidcap->QCIF_rate] << "\n";
    }
    if ((vidcap->Is_ISO)) ostr << "ISO/IEC\n";

    if( len != 0 )
	{
		if(table2_H230Value == H262_H263 || table2_H230Value == H264_Mbe || table2_H230Value==AMC_cap || table2_H230Value==IEC14496_3Capability)//H264_Mbe
		{
			if( h263found/*table2_H230Value == H262_H263*/ )
			{
			//The table2_H230Value is correct therefore we dump the message.
				ostr << "  [H.263]:\n";
				ostr << "--Initial capabilities--: " << "\n" ;
				SmartDumpH263( ostr, len263, (h221str + h263CapsStartPosition) );
			}
			if( h264found/*table2_H230Value == H264_Mbe*/ )
			{
				ostr << "  [H.264]:\n";
				ostr << "---Capabilities---: " << "\n" ;
				//SmartDumpH264( ostr, len, (h221str + h263CapsStartPosition) );
				DumpCapH264( ostr, len264, (h221str + h264CapsStartPosition) );
			}

		}
		else
		{

			FPTRACE(eLevelInfoNormal,"::SmartDumpCap - Mbe Unknown Table 2 Value!!!");
			DBGFPASSERT(1);
			ostr << "\n" << "Mbe Unknown Table 2 Value (" << (WORD)table2_H230Value << ") !!!\n" ;
		}
	}

	if(h239ControlCapfound || AMCfound)
	{
		ostr << "---H.239---------\n";
		if (h239ControlCapfound)  ostr << "H239 Control Capability"<<"\n";
		if(AMCfound && len!=0)
		{
			BYTE optionByte1, optionByte2;
			optionByte1 = h221str[AMCCapsStartPosition];
			optionByte2 = h221str[AMCCapsStartPosition+1];
			GetAMCCaps(optionByte1, optionByte2, ostr);
		}
	}

    if( nsNum != 0 ) {
        BYTE  item2print = 0;
        for( int ind=0; ind<nsNum; ind++ ) {
            ostr << "---NonStandard caps------\n";
            item2print = 0;
            if( nsCapArray[ind].country1 == 0xB5  &&
                nsCapArray[ind].country2 == 0x00  &&
                nsCapArray[ind].manufact1 == 0x00 &&
                nsCapArray[ind].manufact2 == 0x01 ) { // country - USA , manufact - PictureTel

                for( int iter=0; iter<nsCapArray[ind].msgLen-4 && iter<251; iter++ ) {
                    switch( nsCapArray[ind].data[iter] ) {
                        case 0x87: { ostr << " * Siren7 16/24/32kbps\n"; break; }
                        case 0x88: { ostr << " * Siren14 24/32/48kbps\n"; break; }
                        case 0x8D: { ostr << " * Siren7 16kbps\n";       break; }
                        case 0x8E: { ostr << " * Siren7 24kbps\n";       break; }
                        case 0x8F: { ostr << " * Siren7 32kbps\n";       break; }
                        case 0x91: { ostr << " * Siren14 24kbps\n";       break; }
                        case 0x92: { ostr << " * Siren14 32kbps\n";       break; }
                        case 0x93: { ostr << " * Siren14 48kbps\n";       break; }
						case 0x9F: { ostr << " * Field Drop\n";       break; }
                        case 0x95: {
                            ostr << " * PeopleContentV0 ver."
                                 << (WORD)nsCapArray[ind].data[++iter] << "\n";
                            break;
                        }
                        case 0x9E: {

							ostr << " * H.264* Capability\n";

							BYTE profile = nsCapArray[ind].data[++iter];
							ostr << "H.264* profile number = " << (WORD)profile  << "\n";

							BYTE octet = nsCapArray[ind].data[++iter];

							// Octet 0
							if(GetNewValueFromByte(octet,2,4))// CIF cap
								ostr << "CIF at " << h263_cap_MPI[(GetNewValueFromByte(octet,2,4)-1)]
								<< " (when one video stream is active)"  << "\n";

							if(GetNewValueFromByte(octet,5,8))// 4CIF cap
								ostr << "4CIF at " << h263_cap_MPI[(GetNewValueFromByte(octet,5,8)-1)]
								<< " (when one video stream is active)"  << "\n";


							// Octet 1
							if(iter < nsCapArray[ind].msgLen-4)
							{
								octet = nsCapArray[ind].data[++iter];

								if(GetNewValueFromByte(octet,2,4))// CIF cap
									ostr << "CIF at " << h263_cap_MPI[(GetNewValueFromByte(octet,2,4)-1)]
									<< " (when 2 video streams are active)"  << "\n";

								if(GetNewValueFromByte(octet,5,8))// 4CIF cap
									ostr << "4CIF at " << h263_cap_MPI[(GetNewValueFromByte(octet,5,8)-1)]
									<< " (when 2 video streams are active)"  << "\n";

							}

							break;
								   }
                        default :  { item2print = 1;                  break; }
                    }
                }
            } else
            if( nsCapArray[ind].country1 == 0xB5  &&
                nsCapArray[ind].country2 == 0x00  &&
                nsCapArray[ind].manufact1 == 0x23 &&
                nsCapArray[ind].manufact2 == 0x31 ) { // country - USA, manufact - Polycom

                for( int iter=0; iter<nsCapArray[ind].msgLen-4 && iter<251; iter++ ) {
                    switch( nsCapArray[ind].data[iter] ) {
                        case 0x6C: { ostr << " * Polycom' MCU sender\n"; break; }
                        case 0x2F: { ostr << " * VisualConcertPC\n"; break; }
                        case 0x33: { ostr << " * VisualConcertFX\n"; break; }
                        case 0x00: { ostr << " * H263_QCif_AnnexI\n"; break; }
                        case 0x01: { ostr << " * H263_Cif_AnnexI\n"; break; }
                        case 0x02: { ostr << " * H263_4Cif_AnnexI\n"; break; }
                        case 0x04: { ostr << " * H263_QCif_AnnexT\n"; break; }
                        case 0x05: { ostr << " * H263_Cif_AnnexT\n"; break; }
                        case 0x06: { ostr << " * H263_4Cif_AnnexT\n"; break; }
                        case 0x18: { ostr << " * VGA 800x600 Capacity\n"; break; }
                        case 0x19: { ostr << " * VGA 1024x768 Capacity\n"; break; }
                        case 0x1A: { ostr << " * VGA 1280x1024 Capacity\n"; break; }
                        case 0x2C: { ostr << " * VideoStreams2 Capacity\n"; break; }
                        case 0x2D: { ostr << " * VideoStreams3 Capacity\n"; break; }
                        case 0x2E: { ostr << " * VideoStreams4 Capacity\n"; break; }



                        default :  { item2print = 1;          break; }
                    }
                }
            } else
				if( nsCapArray[ind].country1 == 0xB5  &&
					nsCapArray[ind].country2 == 0x00  &&
					nsCapArray[ind].manufact1 == 0x50 &&
					nsCapArray[ind].manufact2 == 0x50 ) { // country - USA, manufact - PP public(EPC)
					BYTE data;
					ostr <<  "  [Polycom Public]:\n";
					for( int iter=0; iter<nsCapArray[ind].msgLen-4 && iter<251; iter++ ) {
						switch( nsCapArray[ind].data[iter] ) {
						case 0x90 :
							GetPPXCCapabilities(nsCapArray[ind].data, &iter,  ostr);
							break;
						case 0x94 : ostr <<  "PP H.221 Escape Table\n";
							break;
						case 0x98 : ostr <<  "roleLabel\n";//0x98 belongs to H.323 therefore label list dumping wont be  used
							break;
						case 0x99 : ostr <<  "H320 MediaFlowControl\n";
							break;
						case 0x9a : ostr << "pc-Profiles: ";
							iter++; data = nsCapArray[ind].data[iter];
							while (data <= MAX_PC_PROFILE)
							{
								ostr  << (dec) << "#" << (WORD)data <<" ";
								iter++; data = nsCapArray[ind].data[iter];
							}
							ostr << "\n";
							iter--;
							break;
						case 0x9b : ostr << "AMSC_MUX\n"; break;
						case 0x9c : {
							BYTE rateByte1=0, rateByte2=0;
							ostr << "AMSC_MUX64: ";
							iter++; rateByte1 = nsCapArray[ind].data[iter];
							iter++; rateByte2 = nsCapArray[ind].data[iter];
							GetAMSC64Rates(rateByte1, rateByte2, ostr);
							break;
									}
						default   : ostr << " Unknown PP\n"; break;
						}
					}
				}else
                item2print = 1;
            if( item2print == 1 ) {
                ostr << "==--==--==--==--==--==--==\n";
                ostr << "Msg len  : (" << (dec) << (DWORD)nsCapArray[ind].msgLen << ")\n";
                ostr << "Country   : (" << (hex) << (DWORD)nsCapArray[ind].country1
                     << "::"             << (hex) << (DWORD)nsCapArray[ind].country2  << ")\n";
                ostr << "Manufact : (" << (hex) << (DWORD)nsCapArray[ind].manufact1
                     << "::"             << (hex) << (DWORD)nsCapArray[ind].manufact2 << ")\n";
                ostr << "Data     : ";
                for( int iter=0; iter<nsCapArray[ind].msgLen-4 && iter<251; iter++ )
                    ostr << "(" << (hex) << (DWORD)nsCapArray[ind].data[iter] << ") ";
                ostr <<"\n";
            }
        }
        ostr << "\n";
    }

    ostr << "---Restrict Cap-----\n";
    if (restrict_found == TRUE)
        ostr << "Restrict Only\n";
    else if(norestrictonly_found == TRUE)
        ostr << "Non-Restrict Only";
    else
        ostr << "Restrict or Non-Restrict\n";

    switch(restrict_type) {
        case RES_L : {
            ostr <<  "Restrict_L\n";
            break;
        }
        case RES_P : {
            ostr <<  "Restrict_P\n";
            break;
        }
        case RES_LP : {
            ostr <<  "Restrict_L and Restrict_P\n";
            break;
        }
    }

    ostr << "\n";

	PDELETE (audiocap);
	PDELETE (vidcap);
	PDELETEA (nsCapArray);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCDRUtils::GetPPXCCapabilities(BYTE * CapArray, int *iter, std::ostream& ostr)
{
	BYTE   cap;
	const BYTE RoleLabelTerminatorMask = 0x20;

	 (*iter)++; cap = CapArray[*iter];
	 int msgLen = cap;
	 (*iter)++;

	cap = CapArray[*iter];
	(*iter)++;
	switch (cap) {
	case 0x8b: { // VideoStream parsing
		cap = CapArray[*iter];
		(*iter)++;

		//***** Start Label list parsing
		{
		ostr << "Video {";
		BYTE label=0;
		while( !(label & RoleLabelTerminatorMask) ) {
			if (label) // NOT first iteration in the loop
				ostr << ",";
			label = cap;
			switch ((label & (~RoleLabelTerminatorMask))) {
			case 0x01: { // People label
				ostr << "People";
						break;
					   }
			case 0x02: { // Content label
				ostr << "Content";
				break;
						}
			default: {
						break;
		   }
	  }
			cap = CapArray[*iter];
			(*iter)++;
		}
		ostr << "}:\n";
}
		//***** End Label list parsing

		// After label list should go Optional capability indicator - 0x0C
		if (cap != 0x0C) {
			break;
		}

		// PPXC VideoStream cap length
		cap = CapArray[*iter];
		(*iter)++;

		// PPXC VideoStream cap data
		cap = CapArray[*iter];
		(*iter)++;

		if (cap == (Start_Mbe | ESCAPECAPATTR )) {
			BYTE len = 0;
			int h263CapsStartPosition = 0;
			BYTE table2_H230Value;
			len = CapArray[(*iter)++]-1;
			//ostr << " (" << (hex) << (WORD)(len +1)<< ")" <<" *	Mbe length;";
			table2_H230Value = CapArray[(*iter)++];
			//ostr << " (" << (hex) << (WORD)table2_H230Value << ")" <<" * H230 ";
			switch(table2_H230Value)
			{
			case 0x00: case 0x01 : 	/*ostr << "* R;";*/ break;  // 0 and 1
			case 0x02: /*ostr << "* TIL;";*/ break;				// 2
			case 0x03: /*ostr << "* IIS;";*/ break;				// 3
			case 0x04: /*ostr << "* TIR;";*/ break;				// 4
			case 0x05: /*ostr << "* TIP;";*/ break;				// 5
			case 0x06: /*ostr << "* NIA;";*/ break;				// 6
			case 0x07: /*ostr << "* NIAP;";*/ break;			// 7
			case 0x08: /*ostr << "* AU_MAP;";*/ break;			// 8
			case 0x09: /*ostr << "* AU_COM;";*/ break;			// 9
			case 0x0a: ostr << "   H.263"; break;			// 10
			}
			h263CapsStartPosition = (*iter);
			ostr << (dec) <<"\n";
			SmartDumpH263( ostr, len, (CapArray + h263CapsStartPosition) );
		}

		break;
			   }
	default: {
		ostr << "Unknown PPXC\n";
		*iter += msgLen;
		return;
			 }
	}

	*iter += msgLen;
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CCDRUtils::GetAMSC64Rates(BYTE rateByte1, BYTE rateByte2, std::ostream& ostr)
{
	if( (!rateByte1) && (!rateByte2) ) {
		ostr << "0k\n";
		return;
	}

	BYTE RatePrinted = 0;
	const BYTE MSB_ON_mask = 0x80;

	if(rateByte1 & (MSB_ON_mask >> AMSC_64k)) {
		ostr << "64k";
		RatePrinted=1;
	}
	if(rateByte1 & (MSB_ON_mask >> AMSC_128k)) {
		if (RatePrinted) 	ostr << ",";
		ostr << "128k";
		RatePrinted=1;
	}
	if(rateByte1 & (MSB_ON_mask >> AMSC_192k)) {
		if (RatePrinted) 	ostr << ",";
		ostr << "192k";
		RatePrinted=1;
	}
	if(rateByte1 & (MSB_ON_mask >> AMSC_256k)) {
		if (RatePrinted) 	ostr << ",";
		ostr << "256k";
		RatePrinted=1;
	}
	if(rateByte1 & (MSB_ON_mask >> AMSC_384k)) {
		if (RatePrinted) 	ostr << ",";
		ostr << "384k";
		RatePrinted=1;
	}
	if(rateByte1 & (MSB_ON_mask >> AMSC_512k)) {
		if (RatePrinted) 	ostr << ",";
		ostr << "512k";
		RatePrinted=1;
	}
	if(rateByte1 & (MSB_ON_mask >> AMSC_768k)) {
		if (RatePrinted) 	ostr << ",";
		ostr << "768k";
		RatePrinted=1;
	}
	if(rateByte2 & (MSB_ON_mask >> AMSC_64k)) {  // second rate byte
		if (RatePrinted) 	ostr << ",";
		ostr << "1152k";
		RatePrinted=1;
	}
	if(rateByte2 & (MSB_ON_mask >> AMSC_128k)) { // second rate byte
		if (RatePrinted) 	ostr << ",";
		ostr << "1536k";
		RatePrinted=1;
	}

	ostr << "\n";
}
/////////////////////////////////////////////////////////////////
void CCDRUtils::GetAMCCaps(BYTE optionByte1, BYTE optionByte2, std::ostream& ostr)
{
	const BYTE MSB_ON_mask = 0x80;
	unsigned short cntntKb;
	ostr << "AMC: ";
	int i = 0;
	for(i=AMC_0k;i<=AMC_384k; i++)
	{
		if(optionByte1 & (MSB_ON_mask >> i))
			ostr << Get_Content_command_BitrateH239(i,&cntntKb) <<", ";
	}
	for(i=AMC_512k;i<=AMC_768k; i++)
	{
		if(optionByte2 & (MSB_ON_mask >> (i-7)))
			ostr << Get_Content_command_BitrateH239(i,&cntntKb) <<", ";
	}
	ostr << "\n";
}
/////////////////////////////////////////////////////////////////
//dump of H221 capabilits vector
void CCDRUtils::DumpCap(BYTE *h221str,WORD length,std::ostream& ostr)
{
    BYTE capAttr,cap;
    WORD i=0;
    WORD Dump_Flag = 0;
    SmartDumpCap(h221str,length,ostr); //report audio+video cap
    ostr << "---capabilities----\n";

    while ( i < length )  {
        cap = h221str[i++];
        capAttr = cap & 0xE0;

        switch ( capAttr ) {
            case AUDRATECAPATTR  :  {
				if((cap ^ capAttr ) < 32)
                    ostr << audXfer[cap ^ capAttr];
                break;
            }

            case DATAVIDCAPATTR  :  {
				if((cap ^ capAttr ) < 32)
                    ostr << datVid[cap ^ capAttr];
                break;
            }

            case OTHERCAPATTR  :  {
				if((cap ^ capAttr ) < 32)
                    ostr << restrict[cap ^ capAttr];
                break;
            }

            case ESCAPECAPATTR  :  {
                if(i < length)  {
                    if ( cap == (Hsd_Esc | ESCAPECAPATTR) )  {
                        //ostr << "HSD/HMLP or MLP(" << (hex) << (int) cap << ",";
                        cap = h221str[i++]; // skip cap value
                        BYTE tab2Attr = cap & 0xE0;
                        BYTE tab2Val  = cap & 0x1F;
                        switch( tab2Attr ) {
                            case HSDHMPLCAPATTR : {
								if((cap ^ tab2Attr ) < 32)
                                    ostr << hsdHmlp[cap ^ tab2Attr];
                                break;
                            }
                            case MLPCAPATTR : {
								if((cap ^ tab2Attr ) < 32)
                                    ostr << Mlp[cap ^ tab2Attr];
                                break;
                            }
                        }
                        //ostr << (hex) << (int) cap << ")";
                    }
                    if ( cap == (H230_Esc | ESCAPECAPATTR) )  {
                        cap = h221str[i++];
                        if ( cap == (CIC | Attr010) ) ostr << "CHAIR";
                        if ( cap == (TIC | Attr001) ) ostr << "TIC";
                        if ( cap == (MIH | Attr001) ) ostr << "MIH";
                        if ( cap == (MVC | Attr010) ) ostr << "MVC";
						if ( cap == (h239ControlCapability | Attr101) ) ostr << "H239ControlCapability";
                    }
                    else if ( cap == (Data_Apps_Esc | ESCAPECAPATTR) )  {
                        cap = h221str[i++];
                        if ( cap == ((28 /*t120 cap */) | (5<<5) ) ) ostr << "T120-Cap";
                        if ( cap == (H224_LSD_On | Attr101)) {
                            BYTE tab4Attr = cap & 0xE0;
                            ostr << LsdHsdMlp[cap ^ tab4Attr];   // H.224_LSD
                        }
                    }
                    else if( cap == (Start_Mbe | ESCAPECAPATTR) ){
                        BYTE len = 0;
                        int h263CapsStartPosition = 0;
                        BYTE table2_H230Value;
                        len = h221str[i++]-1;
                        table2_H230Value = h221str[i++];
                        h263CapsStartPosition = i;

						if(table2_H230Value == H262_H263 || table2_H230Value == H264_Mbe )//H264_Mbe
						{
							//In case of mux /net sync,which causes a corrupted message, we reset the
							//length of Mbe message to the Left Length Of H221 Cap message.
							WORD LeftLenghOfH221Cap = length - h263CapsStartPosition;
							if(len > LeftLenghOfH221Cap )
							{

								FPTRACE(eLevelInfoNormal,"::DumpCap - Mbe 263/264 message length is corrupted!!! \'");
								DBGFPASSERT(1);

								len = LeftLenghOfH221Cap;
							}

							//The table2_H230Value is correct therefore we dump the message.
							if(table2_H230Value == H262_H263)
								DumpCapH263( ostr, len, (h221str + h263CapsStartPosition) );
							else if(table2_H230Value == H264_Mbe) //H264_Mbe
								DumpCapH264( ostr, len, (h221str + h263CapsStartPosition) );
						}
						else if(table2_H230Value==AMC_cap)
						{
							BYTE optionByte1, optionByte2;
							optionByte1 = h221str[i];
							optionByte2 = h221str[i+1];
							GetAMCCaps(optionByte1, optionByte2, ostr);
						}
						else if (table2_H230Value==IEC14496_3Capability)
						{
						  ostr << "ISO/IEC14496-3Capability (MPEG-4 audio) - unsupported cap!\n";
						}
						else
						{
						    std::ostringstream  dumpstring;
							dumpstring << "::DumpCap - Mbe Unknown Table 2 Value (" << (WORD)table2_H230Value << ") !!!\n" ;
						 	FPTRACE(eLevelInfoNormal,(char*)dumpstring.str().c_str());
						 	DBGFPASSERT(1);
						 	ostr << "\n" << "Mbe Unknown Table 2 Value (" << (WORD)table2_H230Value << ") !!!\n" ;
						}

						i = i + len;
                    }

                    else if( cap == (ESCAPECAPATTR | Ns_Cap) ) {
                        BYTE  msgLen, co1, co2, ma1, ma2, data;
                        BYTE  isPictureTel = 0, isPolycom = 0, isEPC = 0;

                        msgLen = co1 = co2 = ma1 = ma2 = data = 0xFF;
                        ostr << "---NonStandard caps----";
                        msgLen = h221str[i++]; // Msg Len

                        ostr << "\nMsg len  : <" << (dec) << (int)msgLen << ">";
                        if( msgLen < 5 ) ostr << "- WRONG !!!";

                        if( msgLen >= 1 ) co1 = h221str[i++];
                        if( msgLen >= 2 ) co2 = h221str[i++];
                        ostr << "\nCountry  : <" << (hex) << (WORD)co1 << "::"
                                                 << (hex) << (WORD)co2 << ">";
                        if( co1 == 0xB5 && co2 == 0x00 ) // USA country code
                            ostr << " - USA country code";

                        if( msgLen >= 3 ) ma1 = h221str[i++];
                        if( msgLen >= 4 ) ma2 = h221str[i++];
                        ostr << "\nManufact : <" << (hex) << (WORD)ma1 << "::"
                                                 << (hex) << (WORD)ma2 << ">";
                        if( co1 == 0xB5 && co2 == 0x00 && ma1 == 0x00 && ma2 == 0x01 ) {// PictureTel manufacturer code
                            ostr << " - PictureTel manufacturer code";
                            isPictureTel = 1;
                        } else if(co1 == 0xB5 && co2 == 0x00 && ma1 == 0x23 && ma2 == 0x31) {//Polycom
                            ostr << " - Polycom manufacturer code";
                            isPolycom = 1;
                        } else if(co1 == 0xB5 && co2 == 0x00 && ma1 == 0x50 && ma2 == 0x50) {//PP Public
                            ostr << " - PP  manufacturer code";
                            isEPC = 1;
                        }
                        if( msgLen >= 5 ) {
                            ostr << "\nData     :";
                            for( int ind=0; ind<msgLen-4; ind++ ) {
                                data = h221str[i++];
                                ostr << " <" << (hex) << (WORD)data << ">";
                                if( isPictureTel ) {
                                    switch ( data ) {
                                        case 0x87 : ostr << "- Siren7 16/24/32k;"; break;
                                        case 0x88 : ostr << "- Siren14 24/32/48k;"; break;
                                        case 0x8D : ostr << "- Siren7 16k;"; break;
                                        case 0x8E : ostr << "- Siren7 24k;"; break;
                                        case 0x8F : ostr << "- Siren7 32k;"; break;
                                        case 0x91 : ostr << "- Siren14 24k;"; break;
                                        case 0x92 : ostr << "- Siren14 32k;"; break;
                                        case 0x93 : ostr << "- Siren14 48k;"; break;
										case 0x9F : ostr << "- Field Drop\n"; break;
                                        case 0x95 : ostr << "- PeopleContentV0 ver.<" << (hex) << (WORD)h221str[i] << ">;";
                                            i++; ind++; break;
                                        case 0x9E:
											{
												ostr << " - H.264* Capability\n";

												data = h221str[i++];
												ind++;
												// H.264 profile
												ostr << "           "; // Alignment
												ostr << "<" << (hex) << (WORD)data << ">"
													 << " - H.264* profile number\n";

												data = h221str[i++];
												ind++;

												// Octet 0
												ostr << "           "; // Alignment
												ostr << "<" << (hex) << (WORD)data << ">"
													 << " - MPI for CIF/SIF and 4CIF/4SIF (when one video stream is active)\n";

												if(ind < msgLen-4) // Is Octet 1?
												{
													data = h221str[i++];
													ind++;

													// Octet 1
													ostr << "           "; // Alignment
													ostr << "<" << (hex) << (WORD)data << ">"
														 << " - MPI for CIF/SIF and 4CIF/4SIF (when two video streams are active)\n";
												}

												break;
											}
                                        default   : ostr << "- Unsupported;"; break;
                                    }
                                } else if( isPolycom ) {
                                    switch ( data ) {
                                        case 0x6C : ostr << "- Polycom' MCU sender;"; break;
                                        case 0x2F : ostr << "- VisualConcertPC;"; break;
                                        case 0x33 : ostr << "- VisualConcertFX;"; break;
                                        case 0x00 : ostr << "- H263_QCif_AnnexI;"; break;
                                        case 0x01 : ostr << "- H263_Cif_AnnexI;"; break;
                                        case 0x02 : ostr << "- H263_4Cif_AnnexI;"; break;
                                        case 0x04 : ostr << "- H263_QCif_AnnexT;"; break;
                                        case 0x05 : ostr << "- H263_Cif_AnnexT;"; break;
                                        case 0x06 : ostr << "- H263_4Cif_AnnexT;"; break;
                                        case 0x18: ostr << "- VGA 800x600;"; break;
                                        case 0x19: ostr << "- VGA 1024x768;"; break;
                                        case 0x1A: ostr << "- VGA 1280x1024;"; break;
                                        case 0x2C: ostr << "- VideoStreams2;"; break;
                                        case 0x2D: ostr << "- VideoStreams3;"; break;
                                        case 0x2E: ostr << "- VideoStreams4;"; break;
                                        default   : ostr << "- Unsupported;"; break;
                                    }
                                } else if(isEPC){
									switch(data) {
										 case 0x90 : ostr << "- PPXC;";

													 GetPPXCAllParameters(h221str, &i, &ind, ostr);

											break;
										case 0x94 : ostr << "- PP H.221 Escape Table;"; break;
										case 0x98 : ostr << "- roleLabel;";//0x98 belongs to H.323 therefore label list dumping wont be  used
											break;
										case 0x99 : ostr << "- MediaFlowControl;"; break;
										case 0x9a : ostr << "- Pc-Profile;";
															ind++; data = h221str[i++];
															while(data <=2)
															{
																ostr << " <" << (hex) << (WORD)data << ">" <<"- profile;";
																ind++; data = h221str[i++];
															}
															ind--; i--;
											break;
										case 0x9b : ostr << "- AMSC_MUX;"; break;
										case 0x9c : ostr << "- AMSC_MUX64;";
													ind++; data = h221str[i++];
													ostr << " <" << (hex) << (WORD)data << ">" <<"- rate byte 1;";
										        	ind++; data = h221str[i++];
													ostr << " <" << (hex) << (WORD)data << ">" <<"- rate byte 2;";
											break;
									    default   : ostr << "- Unsupported;"; break;
									}
                                } else
                                    ostr << "UNKNOWN!!!";
                            }
                            ostr << "\n";
                        }
                    }
                }
                break;
            }

            default   :  {


//                FPASSERT(1);
                ON(Dump_Flag);
                //FPTRACE(eLevelInfoHigh,"::DumpCap - \'unknown capability !!! \'");
                ostr << "unknown capability (" << (hex) << (int) cap << ")";

                break;
            }
        }
        ostr << "\n";
    }
// print entire array of caps in case of an unknown capability

    if(Dump_Flag && length < 100) {
        char s[16];
        WORD index;
        ostr << "Capabilities dump:" << "\n";
        WORD j  = length/8;
        if(length%8)  j++;
        for(WORD k =0; k < j; k++) {
            for(WORD l = 0; l < 8; l++) {
                index = 8*k + l;
                if(index >= length)break;
                BYTE capi = h221str[index];
                sprintf(s," %02x",capi);
                ostr << s;
            }
            ostr <<  "\n" ;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////
void CCDRUtils::GetPPXCAllParameters(BYTE *CapArray, WORD *i, int *ind, std::ostream& ostr)
{

	BYTE   cap;
	const BYTE RoleLabelTerminatorMask = 0x20;

	(*ind)++; cap = CapArray[(*i)++];
	ostr << " <" << (hex) << (WORD)cap << ">" <<"- PPXC len;";
	int msgLen = cap;
	WORD iter = (*i);
	*i += msgLen;
    *ind += msgLen;

	cap = CapArray[iter];
	(iter)++;
	switch (cap) {
	case 0x8b: { // VideoStream parsing
        ostr << " <" << (hex) << (WORD)cap << ">";
		ostr << "- VideoStream;";
		cap = CapArray[iter];
		iter++;

		//***** Start Label list parsing
		{
			BYTE label=0;
			while( !(label & RoleLabelTerminatorMask) ) {
				ostr << " <" << (hex) << (WORD)cap << "> - ";
				if (label) // NOT first iteration in the loop
					ostr << ",";
				label = cap;
				switch ((label & (~RoleLabelTerminatorMask))) {
				case 0x01: { // People label
					ostr << "People label";
					break;
						   }
				case 0x02: { // Content label
					ostr << "Content label";
					break;
						   }
				default: {
					break;
						 }
				}
				cap = CapArray[iter];
				iter++;
			}
			ostr << ";";
		}
		//***** End Label list parsing

		// After label list should go Optional capability indicator - 0x0C
		if (cap != 0x0C) {
			break;
		}
		else {
			ostr << " <" << (hex) << (WORD)cap << ">";
			ostr <<"- Optional Cap Indicator;";
		}

		// PPXC VideoStream cap length
		cap = CapArray[iter];
		iter++;
		ostr << " <" << (hex) << (WORD)cap << ">" <<"- Cap Length;";

		// PPXC VideoStream cap data
		cap = CapArray[iter];
		iter++;

		if (cap == (Start_Mbe | ESCAPECAPATTR )) {
			BYTE len = 0;
			int h263CapsStartPosition = 0;
			BYTE table2_H230Value;
			len = CapArray[iter++]-1;
			//ostr << " (" << (hex) << (WORD)(len +1)<< ")" <<" *	Mbe length;";
			table2_H230Value = CapArray[iter++];
			//ostr << " (" << (hex) << (WORD)table2_H230Value << ")" <<" * H230 ";
			switch(table2_H230Value)
			{
			case 0x00: case 0x01 : 	ostr << "* R;"; break;  // 0 and 1
			case 0x02: ostr << "* TIL;"; break;				// 2
			case 0x03: ostr << "* IIS;"; break;				// 3
			case 0x04: ostr << "* TIR;"; break;				// 4
			case 0x05: ostr << "* TIP;"; break;				// 5
			case 0x06: ostr << "* NIA;"; break;				// 6
			case 0x07: ostr << "* NIAP;"; break;			// 7
			case 0x08: ostr << "* AU_MAP;"; break;			// 8
			case 0x09: ostr << "* AU_COM;"; break;			// 9
			case 0x0a: ostr << " <H262/H263>"; break;			// 10
			}
			h263CapsStartPosition = (iter);
			for (BYTE j=0; j<len; j++ ) {
				ostr << " <" << (hex) << (WORD)(CapArray[iter++]) << ">" <<"- H263 cap";
			}
			//SmartDumpH263( ostr, len, (CapArray + h263CapsStartPosition) );
		}

		break;
			   }
	default: {
		ostr << "Unknown PPXC\n";
		return;
			 }
	}

	return;
}
/////////////////////////////////////////////////////////////////////////////
//dump of H221 capabilits vector
void CCDRUtils::FullDumpCap(BYTE *h221str,WORD length,std::ostream& ostr,BYTE capflag)
{
    BYTE capAttr,cap;
    WORD i=0;

    if(capflag==0)
        ostr <<"\n**** capabilities ****\n";
    else
        ostr << "**** remote comm mode ****\n";
    SmartDumpCap(h221str,length,ostr); //report audio+video cap

    while ( i < length )  {
        cap = h221str[i++];
        capAttr = cap & 0xE0;
        switch ( capAttr ) {

            case AUDRATECAPATTR  :  {
				if((cap ^ capAttr) < 32)
                    ostr << audXfer[cap ^ capAttr];
                break;
            }

            case DATAVIDCAPATTR  :  {
				if((cap ^ capAttr) < 32)
                    ostr << datVid[cap ^ capAttr];
                break;
            }

            case OTHERCAPATTR  :  {
				if((cap ^ capAttr) < 32)
                    ostr << restrict[cap ^ capAttr];
                break;
            }

            case ESCAPECAPATTR  :  {
                if(i < length)  {
                    if ( cap == (Hsd_Esc | ESCAPECAPATTR) )  {
                        //ostr << "HSD/HMLP or MLP(" << (hex) << (int) cap << ",";
                        cap = h221str[i++]; // skip cap value
                        BYTE tab2Attr = cap & 0xE0;
                        BYTE tab2Val  = cap & 0x1F;
                        switch(tab2Attr) {
                            case HSDHMPLCAPATTR : {
								if((cap ^ tab2Attr) < 32)
                                    ostr << hsdHmlp[cap ^ tab2Attr];
                                break;
                            }
                            case MLPCAPATTR : {
								if((cap ^ tab2Attr) < 32)
                                    ostr << Mlp[cap ^ tab2Attr];
                                break;
                            }
                        }
                        //ostr << (hex) << (int) cap << ")";
                    }
                    if ( cap == (H230_Esc | ESCAPECAPATTR) )  {
                        cap = h221str[i++];
                        if ( cap == (CIC | Attr010) ) ostr << "CHAIR";
                        if ( cap == (TIC | Attr001) ) ostr << "TIC";
                        if ( cap == (MIH | Attr001) ) ostr << "MIH";
                        if ( cap == (MVC | Attr010) ) ostr << "MVC";
                    }
                    if ( cap == (Data_Apps_Esc | ESCAPECAPATTR) )  {
                        cap = h221str[i++];
                        if ( cap == ((28 /*t120 cap */) | (5<<5) ) ) ostr << "T120-Cap";
                    }
                    if( cap == (Start_Mbe | ESCAPECAPATTR) ) {
                        BYTE len = 0;
                        int h263CapsStartPosition = 0;
                        BYTE table2_H230Value;
                        len = h221str[i++]-1;
                        table2_H230Value = h221str[i++];
                        h263CapsStartPosition = i;

						if(table2_H230Value == H262_H263 || table2_H230Value == H264_Mbe )//H264_Mbe
						{
							//In case of mux /net sync,which causes a corrupted message, we reset the
							//length of H263 message to the Left Length Of H221 Cap message.
							WORD LeftLenghOfH221Cap = length - h263CapsStartPosition;
							if(len > LeftLenghOfH221Cap )
							{

								FPTRACE(eLevelInfoNormal,"::FullDumpCap - H263 message length is corrupted!!! \'");
								DBGFPASSERT(1);

								len = LeftLenghOfH221Cap;
							}

							//The table2_H230Value is correct therefore we dump the message.
							if(table2_H230Value == H262_H263)
								DumpCapH263( ostr, len, (h221str + h263CapsStartPosition) );
							else if(table2_H230Value == H264_Mbe) //H264_Mbe
								DumpCapH264( ostr, len, (h221str + h263CapsStartPosition) );
						}
						else
						{

							FPTRACE(eLevelInfoNormal,"::DumpCap - Mbe H263/H264 message is corrupted!!!");
							DBGFPASSERT(1);

							ostr << "\n" << "Mbe H263 or H264 message is corrupted!!!" << "\n" ;
						}

						i = i + len;
                    }

                    if( cap == (ESCAPECAPATTR | Ns_Cap) ) {
                        BYTE  msgLen, co1, co2, ma1, ma2, data;
                        msgLen = co1 = co2 = ma1 = ma2 = data = 0xFF;
                        ostr << "***NonStandard Caps*****";
                        msgLen = h221str[i++]; // Msg Len
                        ostr << "\nMsg len  : (" << (dec) << (int)msgLen << ")";
                        if( msgLen < 5 ) ostr << "- WRONG !!!";
                        if( msgLen >= 1 ) co1 = h221str[i++];
                        if( msgLen >= 2 ) co2 = h221str[i++];
                        ostr << "\nCountry  : (" << (hex) << (WORD)co1 << "::"
                                                 << (hex) << (WORD)co2 << ")";
                        if( msgLen >= 3 ) ma1 = h221str[i++];
                        if( msgLen >= 4 ) ma2 = h221str[i++];
                        ostr << "\nManufact : (" << (hex) << (WORD)ma1 << "::"
                                                 << (hex) << (WORD)ma2 << ")";
                        if( msgLen >= 5 ) {
                            ostr << "\nData     : ";
                            for( int ind=0; ind<msgLen-4; ind++ ) {
                                data = h221str[i++];
                                ostr << " (" << (hex) << (WORD)data << ")";
                            }
                            ostr << "\n";
                        }
                    }
                }
                break;
            }

            default   :  {


//                FPASSERT(1);
                if(capflag==0)
                    FPTRACE(eLevelInfoNormal,"::DumpCap - \'unknown capability !!! \'");
                else
                    FPTRACE(eLevelInfoNormal,"::DumpCap - \'unknown remote mode !!! \'");
                DBGFPASSERT(1);
                break;
            }
        }
        ostr << "\n";
    }
}

/////////////////////////////////////////////////////////////////////////////
int CCDRUtils::GetBitValue(BYTE byte,int bitNumber)
{
	int res = 0;
	BYTE temp = (BYTE)1;
	if(byte & (temp << (8-bitNumber)))
		res = 1;
	return res;
}



/////////////////////////////////////////////////////////////////////////////
void CCDRUtils::SmartDumpH263(std::ostream& ostr, int numberOfH263Bytes, BYTE h221str[])
{
BYTE h263Cap;
BYTE h263ImageFormat;
BYTE h263Mpi;
int readBytes = 0;
int optional = 0;
int HighestStandardResolution = H263_QCIF_SQCIF;
int IsAnnexF = 0;
WORD ResolutionFlag = 0;

while( readBytes < numberOfH263Bytes )
{
	h263Cap = h221str[readBytes++];		// baseline h263 caps

  //	if( h263Cap == (BYTE)0x7f )
		//FPTRACE(eLevelInfoNormal,"Nh221.cpp - Extension Code 01111111 (0x7f) was found !");

	if( h263Cap == (BYTE)0x7f )
	{
		SmartDumpH263AdditionalCap(ostr,numberOfH263Bytes - readBytes ,(h221str + readBytes),HighestStandardResolution,IsAnnexF);
		return;
	}

	h263ImageFormat = GetNewValueFromByte(h263Cap,6,7);
	if( h263ImageFormat > (BYTE)H263_CIF_16 ) {
		// FPASSERT(1);
		h263ImageFormat = FORBIDDEN_IMAGE_FORMAT_H263;
	}

	h263Mpi = GetNewValueFromByte(h263Cap,2,5);
	if( h263Mpi > (BYTE)MPI_30 ) {
		//FPASSERT(1);
		h263Mpi = FORBIDDEN_MPI_H263;
	}

	if(ResolutionFlag == 0)
	{
	    HighestStandardResolution = h263ImageFormat;
		ON(ResolutionFlag);
	}

	ostr << h263_cap_Format[(int)h263ImageFormat]
			 << " at " << h263_cap_MPI[(int)h263Mpi] ;
			 // << "\n";
	if( GetBitValue(h263Cap,8) ){	// if there are optional caps
			h263Cap = h221str[readBytes++];		// optional h263 caps byte

			if( h263Cap == (BYTE)0x7f )
				FPTRACE(eLevelInfoNormal,"Nh221.cpp - Extension Code 01111111 (0x7f) was found !");

			if( h263Cap == (BYTE)0x7f )
				return;

			if(GetBitValue(h263Cap,3))  { ostr << " + UMV "; } //optional=1;
			if(GetBitValue(h263Cap,4))  { ostr << " + Annex F "; IsAnnexF = 1;} //optional=1;
			if(GetBitValue(h263Cap,5))  { ostr << " + AC ";  } //optional=1;
			if(GetBitValue(h263Cap,6))  { ostr << " + PB";   } //optional=1;
			// if( optional ) { ostr << "\n"; optional = 0; }

				// if specified HRD-B or BPPmaxKB
			if( GetBitValue(h263Cap,7) || GetBitValue(h263Cap,8) )
			{
				h263Cap = h221str[readBytes++];		// HRD-B BPPmaxKB byte

				if( h263Cap == (BYTE)0x7f )
					return;
				BYTE tmp = GetNewValueFromByte(h263Cap,1,4);
				if(tmp < 16)
					ostr << h263_cap_HRD_B_Size[tmp];
				
				tmp = GetNewValueFromByte(h263Cap,5,8);
				if(tmp < 16)
					ostr << " + " <<h263_cap_BPPmaxKB[tmp];
				ostr << "\n";
			}
	}
    ostr << "\n";
}
}
void CCDRUtils::SetResolutionBounds(BYTE* resolutionBounds, WORD size, WORD index, BYTE value)
{
	if (index < size)
	{
		*(resolutionBounds+index) = value;
		return;
	}
	FPASSERT(index);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCDRUtils::GetResolutionBounds(BYTE* resolutionBounds, WORD size, WORD index)
{
	if (index < size)
		return *(resolutionBounds+index);
	FPASSERT(index);
	return 0;
}
void CCDRUtils::SmartDumpH263AdditionalCap(std::ostream& ostr, int numberOfAdditionalH263Bytes, BYTE h221str[], int highestStandardResolution, int isAnnexF)
{
	BYTE AdditionalH263Cap=0,Option1=0,Option2=0;
	BYTE h263Cap = 0;
    BYTE readBytes = 0;

	//---------------------------------------------------------------------
	//This mechanism stores the bounds of resolution in order to exhabit
	//this information in the second additinal cap.
	//Each resolution has 5 bytes
	//First - 0xFF (= not valid) / H263_QCIF_SQCIF ... PAL_50_FIELDS
	//Second - X1, third - X2, forth - Y1, fifth - Y2
	BYTE btarrResolutionBounds[NUMBER_OF_RESOLUTIONS * NUMBER_OF_BYTES];
	memset(btarrResolutionBounds, 0xFF, sizeof(btarrResolutionBounds));

	WORD Bytes = 0;
	WORD numberOfResolutions = 0;
	//----------------------------------------------------------------------

	ostr << "\n" << "--Additional capabilities--: \n";
	while( readBytes < numberOfAdditionalH263Bytes)
	{
		ostr << "\n";
		AdditionalH263Cap = h221str[readBytes++];

		if( AdditionalH263Cap == (BYTE)0x7f )
		{
			SmartDumpH263SecondAdditionalCap(ostr, numberOfAdditionalH263Bytes - readBytes, (h221str + readBytes), btarrResolutionBounds);
			return;
		}

		Bytes = 0;

		//ostr << FormatIndicator[GetNewValueFromByte(AdditionalH263Cap,1,2)] << (dec);
		switch(GetNewValueFromByte(AdditionalH263Cap,1,2))
		{
			case STANDARD_FORMAT_ADDITIONAL_CAP:   // Do nothing
			{
				if (highestStandardResolution >= 0)
					ostr << NumberOfFormat[highestStandardResolution];
				else
					ostr << "(FORBIDDEN)";

				SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), numberOfResolutions * NUMBER_OF_BYTES, highestStandardResolution);
				numberOfResolutions++;
				highestStandardResolution--;
				break;
			}

			case CUSTOM_FORMAT_CAP_EQUAL_BOUNDS:
			{
				WORD resolutionNumber;
				WORD y = h221str[readBytes++];
				WORD x = h221str[readBytes++];

				const char* resolution = Get_Resolution(x, y, resolutionNumber);
				if (resolution)
				{
					ostr << resolution;
					ostr << " [" << ((x + 1) * 8) << " X " << ((y + 1) * 8) << "]";
					SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), numberOfResolutions * NUMBER_OF_BYTES, resolutionNumber);
				}
				else
				{
					ostr << "Custom format" << " [" << ((x + 1) * 8) << " X " << ((y + 1) * 8) << "]";;
					SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), ((numberOfResolutions * NUMBER_OF_BYTES) + Bytes++), CUSTOM_1_BOUNDS);
					SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), ((numberOfResolutions * NUMBER_OF_BYTES) + Bytes++), x);
					SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), ((numberOfResolutions * NUMBER_OF_BYTES) + Bytes++), y);

				}
				numberOfResolutions++;
				break;
			}

			case CUSTOM_FORMAT_CAP_TWO_DISTINCT_BOUNDS:
			{
				WORD y1 = (h221str[readBytes++]);
				WORD x1 = (h221str[readBytes++]);
				WORD y2 = (h221str[readBytes++]);
				WORD x2 = (h221str[readBytes++]);

				ostr << "Custom format";
				ostr << " [" << (int)((x2 + 1) * 8) << " - " << (int)((x1 + 1) * 8) << "]" << " X";
				ostr << " [" << (int)((y2 + 1) * 8) << " - " << (int)((y1 + 1) * 8) << "]";

				SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), ((numberOfResolutions * NUMBER_OF_BYTES) + Bytes++), CUSTOM_2_BOUNDS);

				SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), ((numberOfResolutions * NUMBER_OF_BYTES) + Bytes++), x1);

				SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), ((numberOfResolutions * NUMBER_OF_BYTES) + Bytes++), x2);

				SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), ((numberOfResolutions * NUMBER_OF_BYTES) + Bytes++), y1);

				SetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), ((numberOfResolutions * NUMBER_OF_BYTES) + Bytes++), y2);

				numberOfResolutions++;
				break;
			}

			default:
			{
				ostr << " FORBIDDEN ";   // Standard : Value of 1 is not allowed.
			}
		}
		//ostr << "\n";

        if(GetBitValue(AdditionalH263Cap,4))
		{
			h263Cap = h221str[readBytes++];  // CustomPCF_1
			WORD ClockDivisor = (int)GetNewValueFromByte(h263Cap,1,7);
			WORD ClockConversionCode = ((int)GetBitValue(h263Cap,8) + 1000);

			if (ClockDivisor == 0 || ClockConversionCode == 0)
			{
				FPTRACE(eLevelError, "CCDRUtils::SmartDumpH263AdditionalCap - Illegal ClockDivisor and/or ClockConversionCode");
				return;
			}

			h263Cap = h221str[readBytes++];  // CustomPCF_2
			WORD  CustomMPIIndicator = (int)GetNewValueFromByte(h263Cap,1,6);

			WORD CustomClockFrequency;
			if(ClockConversionCode == 1001)
			    CustomClockFrequency = (1800000 / (ClockDivisor * ClockConversionCode)) +1;
			else
				CustomClockFrequency =  1800000 / (ClockDivisor * ClockConversionCode);


       	    WORD FramePerSecond = CustomClockFrequency / (CustomMPIIndicator + 1);

			if(FramePerSecond == 7)
				ostr << " at " << "7.5" << " fps";
			else
            	ostr << " at " << FramePerSecond << " fps";



			if(GetNewValueFromByte(h263Cap,7,8)) // HRDBPPMaxKB
			{
				BYTE b263_1_4 = GetNewValueFromByte(h263Cap,1,4);
				BYTE b263_5_8 = GetNewValueFromByte(h263Cap,5,8);
				if(b263_1_4 < 16)
					ostr << h263_cap_HRD_B_Size[b263_1_4];
				if(b263_5_8 < 16)
					ostr << " , " << h263_cap_BPPmaxKB[b263_5_8] << "\n";
			}
		}

		ostr << "\n";

		if(GetBitValue(AdditionalH263Cap,5))
		{
			// We do not exhabit this detailed information at this level.
			// This information is keeped in the CCapH263 class.

			//ostr << "CustomPixelWidth - " << (int)h221str[readBytes++]
            //     << " , " << "CustomPixelHeight - " << (int)h221str[readBytes++] << "\n";

			readBytes += 2;//increase the readBytes by 2.
		}

		BYTE b263_6_8 = GetNewValueFromByte(AdditionalH263Cap,6,8);
		if(b263_6_8 < 8)
			ostr << h263_OptionsIndicator_short[b263_6_8];

		//According to the standard - initial annexes are inherit from the
		//immediately smaller standard format.

// annex f dump is wrong, it dump in all formats even if it set only for the lower formats
// dump removed (ron, 04/04 for highest common phase 2)
//		if(GetNewValueFromByte(AdditionalH263Cap,6,8) == 0) // detailed annexes
//		   if(IsAnnexF) ostr << "F,";

		if(!GetNewValueFromByte(AdditionalH263Cap,6,8))
		{
			BYTE IndividualOptionIndicator = h221str[readBytes++];
			if(GetBitValue(IndividualOptionIndicator,2)) // Options 1 Flag
			{
				Option1 = h221str[readBytes++];
				if(GetBitValue(Option1,2))  ostr << "I,";
				if(GetBitValue(Option1,3))  ostr << "J,";
			    if(GetBitValue(Option1,4))  ostr << "L(4),";
				if(GetBitValue(Option1,5))  ostr << "T,";
				if(GetBitValue(Option1,6))  ostr << "D,";
				if(GetBitValue(Option1,7))  ostr << "P,";
				if(GetBitValue(Option1,8))  ostr << "N,";
			}

			if(GetBitValue(IndividualOptionIndicator,3)) // Options 2 Flag
			{
				ostr << "\n";
                Option2 = h221str[readBytes++];
				if(GetBitValue(Option2,2))  ostr << "K,";
				if(GetBitValue(Option2,3))  ostr << "R,";
			    if(GetBitValue(Option2,4))  ostr << "Q,";
				if(GetBitValue(Option2,5))  ostr << "L,";
				if(GetBitValue(Option2,6))  ostr << "M,";
				if(GetBitValue(Option2,7))  ostr << "L(Partial Picture Freeze And Release),";
				if(GetBitValue(Option2,8))  ostr << "S,";
			}

			if(GetBitValue(IndividualOptionIndicator,4)) // Options 3 Flag
			{
				ostr << "\n";
                h263Cap = h221str[readBytes++];
                BYTE h263_1_2 = GetNewValueFromByte(h263Cap,1,2);
				if(h263_1_2 && h263_1_2 < 4)
					ostr << h263_DynamicWarping[h263_1_2] << "  ";

				if(GetBitValue(h263Cap,3))  ostr << "Full Picture Snapshot ";
			    if(GetBitValue(h263Cap,4))  ostr << "Partial Picture Snapshot ";
				if(GetBitValue(h263Cap,5))  ostr << "Video Segment Tagging ";
				if(GetBitValue(h263Cap,6))  ostr << "Progressive Refinement ";
				if(GetBitValue(h263Cap,7))  ostr << "Dynamic Picture Resizing Sixteenth Pel ";
				if(GetBitValue(h263Cap,8))  ostr << "Temporal Spatial Trade Off Capability ";
			}

			if(GetBitValue(Option1,8) || GetBitValue(Option2,2))
			{
				// We do not exhabit this detailed information at this level.
				// This information is keeped in the CCapH263 class.

				//ostr << "\n";
                h263Cap = h221str[readBytes++];


			}

			if(GetBitValue(IndividualOptionIndicator,5))
			{
				// We do not exhabit this detailed information at this level.
				// This information is keeped in the CCapH263 class.

				ostr << "O,";

				//ostr << "\n";
                h263Cap = h221str[readBytes++];
				BYTE NumberOfScalableLayers = GetNewValueFromByte(h263Cap,1,4) + 1;

				for(int i = 0; i < NumberOfScalableLayers; i++)
				{
				    //ostr << "\n";
                    h263Cap = h221str[readBytes++];

				}
			}

			if(GetBitValue(IndividualOptionIndicator,6)) // Error Compensation
			{
				ostr << "Error Compensation";
			}

		}

		ostr << "\n" <<"----------------------------------";
	 }

	 if(readBytes != numberOfAdditionalH263Bytes)  //extension codeword was encountered again.
	 {

			 FPTRACE(eLevelError,"SmartDumpH263AdditionalCap - extension codeword was encountered again !!!!");
			 //DBGFPASSERT(1);
	 }

	 ostr << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void CCDRUtils::SmartDumpH263SecondAdditionalCap(std::ostream& ostr, int numberOfSecondAdditionalH263Bytes, BYTE h221str[], BYTE btarrResolutionBounds[])
{
	BYTE secondAdditionalH263Cap;
	BYTE readBytes = 0;
	WORD resolutionNumber = 0;
	WORD resolutionValve = 0;
	BYTE bytes;
	WORD highestStandardResolution = H263_QCIF_SQCIF;

	ostr << "\n" << "--Second Additional capabilities--: \n\n";
	while( readBytes < numberOfSecondAdditionalH263Bytes)
	{
		bytes = 1;
		WORD resolutionIndex = resolutionNumber * NUMBER_OF_BYTES;
		resolutionValve = GetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), resolutionIndex);

		switch(resolutionValve)
		{
		case H263_QCIF_SQCIF:
		case H263_CIF:
		case H263_CIF_4:
		case H263_CIF_16:
			{
				ostr << "\n" << NumberOfFormat[resolutionValve] << "\n";
				highestStandardResolution = resolutionValve;
				break;
			}
        case VGA :
		case NTSC:
		case SVGA:
		case XGA : { ostr << "\n" << NumberOfFormat[resolutionValve] << "\n"; break; }

		case CUSTOM_1_BOUNDS:
			{
				WORD x = (WORD)GetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), resolutionIndex + bytes++);
				WORD y = (WORD)GetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), resolutionIndex + bytes++);

				ostr << "\n" << "Custom format"
                     << " [" << ((int)(x + 1) * 8) << " X " << ((int)(y + 1) * 8) << "]" << "\n";
			       	 break;
			}

		case CUSTOM_2_BOUNDS:
			{
				WORD x1 = (WORD)GetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), resolutionIndex + bytes++);
				WORD x2 = (WORD)GetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), resolutionIndex + bytes++);
				WORD y1 = (WORD)GetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), resolutionIndex + bytes++);
				WORD y2 = (WORD)GetResolutionBounds(btarrResolutionBounds, sizeof(btarrResolutionBounds), resolutionIndex + bytes++);

				ostr << "\n" << "Custom format";

				ostr << " [" << (int)((x2 + 1) * 8) << " - " << (int)((x1 + 1) * 8) << "]" << " X ";
				ostr << "[" << (int)((y2 + 1) * 8) << " - " << (int)((y1 +1) * 8) << "]" << "\n";

				break;
			}

		case 0xFF:
			{
				if(highestStandardResolution)
				{
					highestStandardResolution--;
					ostr << "\n" << NumberOfFormat[highestStandardResolution] << "\n";
				}

				break;
			}
		}


		resolutionNumber++;

		secondAdditionalH263Cap = h221str[readBytes++];

        if(secondAdditionalH263Cap == 0x40)
		{
			ostr << "No second capabilities are supported";
			ostr << "\n";
		}
		else
		{
			BYTE h263_1_2 = GetNewValueFromByte(secondAdditionalH263Cap,1,2);
			if(h263_1_2 != 1 && h263_1_2 < 4)// 1 means not capable.
			{
				ostr << h263_EnhancedReferencePicSelectShort[h263_1_2];
				ostr << "\n";
			}

			if(GetBitValue(secondAdditionalH263Cap,4))  ostr << "Capable of using dataPartionedSlices";
			if(GetBitValue(secondAdditionalH263Cap,5))  ostr << "Capable of using FixedPointIDCT0";
			if(GetBitValue(secondAdditionalH263Cap,6))  ostr << "Capable of using interlaced Fields\n";
			if(GetBitValue(secondAdditionalH263Cap,7))  ostr << "Capable of using currentPictureHeaderRepetition";
			if(GetBitValue(secondAdditionalH263Cap,8))  ostr << "secondOptionExtByte follows";
		}

		ostr <<"----------------------------------" << "\n";
	}

	ostr << "\n\n";
}
/////////////////////////////////////////////////////////////////////////////
void CCDRUtils::DumpCapH263(std::ostream& ostr, int numberOfH263Bytes, BYTE h221str[])
{

BYTE h263Cap;
BYTE h263ImageFormat;
BYTE h263Mpi;
int readBytes = 0;
int optional = 0;

while( readBytes < numberOfH263Bytes )
{
	h263Cap = h221str[readBytes++];		// baseline h263 caps

	if( h263Cap == (BYTE)0x7f )
	{
		//SmartDumpH263AdditionalCap(ostr,numberOfH263Bytes - readBytes ,(h221str + readBytes));
		return;
	}
	h263ImageFormat = GetNewValueFromByte(h263Cap,6,7);
	if( h263ImageFormat > (BYTE)H263_CIF_16 ) {
		//FPASSERT(1);
		h263ImageFormat = FORBIDDEN_IMAGE_FORMAT_H263;
	}

	h263Mpi = GetNewValueFromByte(h263Cap,2,5);
	if( h263Mpi > (BYTE)MPI_30 ) {
		//FPASSERT(1);
		h263Mpi = FORBIDDEN_MPI_H263;
	}

	ostr << "H263_"
		     << h263_cap_Format[(int)h263ImageFormat]
		     << " "
			 << h263_cap_MPI[(int)h263Mpi]
			 << ",";
	if( GetBitValue(h263Cap,8) ){	// if there are optional caps
			h263Cap = h221str[readBytes++];		// optional h263 caps byte

			if( h263Cap == (BYTE)0x7f )
				FPTRACE(eLevelInfoNormal,"Nh221.cpp - Extension Code 01111111 (0x7f) was found !");

			if( h263Cap == (BYTE)0x7f )
				return;

			ostr << GetBitValue(h263Cap,3)  /* UMV */
				 << GetBitValue(h263Cap,4)	/* AMP */
				 << GetBitValue(h263Cap,5)	/* AC  */
				 << GetBitValue(h263Cap,6); /* PB  */

			ostr << "\n";

				// if specified HRD-B or BPPmaxKB
			if( GetBitValue(h263Cap,7) || GetBitValue(h263Cap,8) ){
				h263Cap = h221str[readBytes++];		// HRD-B BPPmaxKB byte
				if( h263Cap == (BYTE)0x7f )
					return;
				BYTE b263_1_4 = GetNewValueFromByte(h263Cap,1,4);
				if(b263_1_4 < 16)
					ostr << h263_cap_HRD_B_Size[b263_1_4]  << "\n";

				BYTE b263_5_8 = GetNewValueFromByte(h263Cap,5,8);
				if(b263_5_8 < 16)
					ostr << h263_cap_BPPmaxKB[b263_5_8] << "\n";
			}
	}
	else{
			ostr << 0   /* UMV */
				 << 0	/* AMP */
				 << 0	/* AC  */
				 << 0;  /* PB  */

			ostr << "\n";
	}
}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCDRUtils::DumpCapH264(std::ostream& ostr, int numberOfH264Bytes, BYTE h221str[])
{

	BYTE h264Cap = 1;
	int  readBytes = 0;

	while( readBytes < numberOfH264Bytes )
	{
		BYTE h264CapArray[MAX_NUMBER_OF_BYTES_IN_H264_CAP_SET];    //20 BYTES

		for (int i=0;i<MAX_NUMBER_OF_BYTES_IN_H264_CAP_SET;i++)
			h264CapArray[i] = 0xFF;

		int index=0;

		while ((readBytes < numberOfH264Bytes ) && (index<MAX_NUMBER_OF_BYTES_IN_H264_CAP_SET))
		{
			h264Cap = h221str[readBytes++];
			if (h264Cap == 0)
				break;
			else
			{
				h264CapArray[index] = h264Cap;
				index++;
			}
		}
		ostr << "H264_"
			<< GetH264ProfileAsString(h264CapArray[H264_PROFILE])
			<< "\n"
			<< GetH264LevelAsString(h264CapArray[H264_LEVEL])
			<< "\n";

		BYTE offset1st = 0;

		while(h264CapArray[H264_CUSTOM_1+offset1st]!=0xFF  &&  offset1st < MAX_NUMBER_OF_BYTES_IN_H264_CAP_SET)
		{
			switch(h264CapArray[H264_CUSTOM_1+offset1st])
			{
				case CUSTOM_MAX_MBPS_CODE:
					{
						ostr << "CustomMaxMBPS(500 Mb/s) ";
						break;
					}
				case CUSTOM_MAX_FS_CODE:
					{
						ostr << "CustomMaxFS(256 Mb) ";
						break;
					}
				case CUSTOM_MAX_DPB_CODE:
					{
						ostr << "CustomMaxDPB(32768 Bytes) ";
						break;
					}
				case CUSTOM_MAX_BR_CODE:
					{
						ostr << "CustomMaxBRandCPB(25000 bits/s) ";
						break;
					}
				case CUSTOM_MAX_STATIC_MBPS:
					{
						ostr << "CustomMaxStaticMbps(UnSupported) ";
						break;
					}
				case CUSTOM_MAX_RCMD_NAL_UNIT_SIZE:
					{
						ostr << "CustomMaxRcmdNalUnitSize(UnSupported) ";
						break;
					}
				case CUSTOM_MAX_NAL_UNIT_SIZE:
					{
						ostr << "CustomMaxNalUnitSize(UnSupported) ";
						break;
					}
				case CUSTOM_SAR_CODE:
					{
						ostr << "CustomSarCode ";
						break;
					}
				case ADDITIONAL_MODES_SUPPORETD:
					{
						ostr << "AdditionalModesSupported(UnSupported) ";
						break;
					}
				case ADDITIONAL_DISPLAY_CAPABILITIES:
					{
						ostr << "AdditionalDisplayCapabilities(UnSupported) ";
						break;
					}
				default:
					{
						if(H264_CUSTOM_1+offset1st < MAX_NUMBER_OF_BYTES_IN_H264_CAP_SET)
							ostr << "H264 Custom parameters coruppted (" << h264CapArray[H264_CUSTOM_1+offset1st] <<") ";
						break;
					}
			}
			offset1st++;

			if(h264CapArray[H264_CUSTOM_1+offset1st] != 0xFF)
			{
				h264Cap = h264CapArray[H264_CUSTOM_1+offset1st];
				if (h264Cap & 128)
				{
					offset1st++;
					if(h264CapArray[H264_CUSTOM_1+offset1st] != 0xFF)
					{
						BYTE h264CapSecondByte = h264CapArray[H264_CUSTOM_1+offset1st];
						ostr << CalcH264WordFromBytes(h264Cap,h264CapSecondByte)
							 << "\n";
						offset1st++;
					}
				}
				else
				{
					ostr << (WORD)h264Cap << "\n";
					offset1st++;
				}

			}
		}

	}
}

/////////////////////////////////////////////////////////////////
const char* CCDRUtils::Get_Resolution(WORD x,WORD y,WORD& ResolutionNumber)
{
    if(x == 21 && y == 17)         { ResolutionNumber = H263_QCIF_SQCIF;return "QCIF/SQCIF";}
	else if(x == 43 && y == 35)    { ResolutionNumber = H263_CIF;return "CIF";}
	else if(x == 87 && y == 71)    { ResolutionNumber = H263_CIF_4;return "4CIF";}
	else if(x == 175 && y == 143)  { ResolutionNumber = H263_CIF_16;return "16CIF";}
	else if(x == 79 && y == 59)    { ResolutionNumber = VGA;return "VGA"; }
	else if(x == 87 && y == 59)    { ResolutionNumber = NTSC;return "NTSC"; }
	else if(x == 99 && y == 74)    { ResolutionNumber = SVGA;return "SVGA";}
	else if(x == 127 && y == 95)   { ResolutionNumber = XGA;return "XGA"; }
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCDRUtils::IsH264TwoBytesNumber(WORD capH264Custom)
{
	if(CalcH264CustomParamFirstByte(capH264Custom)>63)
		return TRUE;
	else return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCDRUtils::CalcH264CustomParamFirstByte(WORD customParam)
{
	//63 is equal to Bin number 111111
	//when we would like to send byte that is higher than 111111 we need 1 more byte
	//so we sign it with 1 in the highest BYTEs bit it means that the first Byte will be
	//10xxxxxx when x is 0/1
	BYTE x=(BYTE)customParam;
	if (customParam <= 63)
		 return x ;
	else //custom param > 63
		x = (x & (BYTE)63) | (BYTE)128;
	return x;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCDRUtils::CalcH264CustomParamSecondByte(WORD customParam)
{
	BYTE y=0;
	WORD num1 = 8128;
	//8126 is equal to Bin number 0001111111000000
	//when we would like to send byte that is higher than 111111 we need 1 more byte
	//so we sign it with 1 in the highest BYTEs bit it means that the first Byte will be
	//10xxxxxx when x is 0/1
	WORD x=  customParam & num1; //only 7 requierd bits remine

	x>>=6; //shift 6 right
	y=(BYTE)x;
	return y;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CCDRUtils::CalcH264WordFromBytes(BYTE firstByte, BYTE secondByte)
{

	WORD num1 = 63;
	//if (firstByte<=63) PASSERT(firstByte);
	WORD customParam = firstByte;
	if (customParam & 128)
		customParam = customParam & num1;

	WORD secondByteCastToWord = secondByte;
	secondByteCastToWord <<= 6;
	customParam |= secondByteCastToWord;

	return customParam;
}


/////////////////////////////////////////////////////////////////////////////
//dump of H221 BAS commands vector
void CCDRUtils::DumpH221Stream(std::ostream& msg, WORD len,BYTE* h221Array)
{
    WORD bitRate;
    BYTE msb;
    for( WORD i = 0 ; i < len ; i++ ) {
        msb = h221Array[i] & 0xE0;
        switch( msb ) {
            case AUDCMDATTR : { // 000
                msg << "Audio: " << Get_Audio_Coding_Command_BitRate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
                break;
            }
            case XFERCMDATTR : { // 001
                msg << "Transfer: " << Get_Transfer_Rate_Command_BitRate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
                break;
            }
            case OTHRCMDATTR : { // 010
                msg << "Other: " << Get_Video_Oth_Command_BitRate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
                break;
            }
            case LSDMLPCMDATTR : { // 011
                msg << "LSD/MLP: " << Get_Lsd_Mlp_Command_BitRate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
                break;
            }
            case ESCAPECAPATTR : { // 111
                if ((h221Array[i] == (Data_Apps_Esc |  ESCAPECAPATTR))&&(i<len-1)) {
                    i++;
                    msg << "H224 STATUS: " << Get_Lsd_Hsd_Mlp_Command(h221Array[i]) << " \n";
                    break;
                }
				if ((h221Array[i] == (H230_Esc |  ESCAPECAPATTR))&&(i<len-1)) {
                    BYTE opcode = h221Array[++i];
					if((opcode == (AMC_open | Attr101))&&(i<len-4))
					{
						BYTE AMCOpenByte1, AMCOpenByte2, sbe;
						unsigned short cntntKb;
						sbe = h221Array[++i];
						AMCOpenByte1 = h221Array[++i];
						sbe = h221Array[++i];
						AMCOpenByte2 = h221Array[++i];
						BYTE ContentRate;
						// calculate rate
						ContentRate = CalculateH239Rate(AMCOpenByte2);
						if((AMCOpenByte1&0xF0) == (0x2 << 4))
						{
							msg << "H.239 Role: Presentation\n";
							msg << "AMC Rate: "<< Get_Content_command_BitrateH239(ContentRate,&cntntKb) <<"  \n";

						}
					}
                    else if((opcode == (AMC_close | Attr101))&&(i<len-2))
					{
						BYTE AMCCloseByte, sbe;
						sbe = h221Array[++i];
						AMCCloseByte = h221Array[++i];
                        msg << "AMC Rate: AMC-Off\n";
					}
                    break;
                }
            // esc : discard this byte and look at nxt bye for hsd command
                if ((h221Array[i] == ESCHSDATTR)&&(i<len-1)){
                    i++;
                    msg << "HSD/HMLP: " << Get_Hsd_Hmlp_command_Bitrate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
                } else if(( h221Array[i] == (ESCAPECAPATTR | Ns_Com) )&&(i<len-5)) {
                    // non-standard commands section
		 BYTE msgLen = h221Array[++i];                          // message Lenght
                    BYTE count1 = h221Array[++i], count2 = h221Array[++i]; // country code
                    BYTE manuf1 = h221Array[++i], manuf2 = h221Array[++i]; // manufacturer code                   
		 BYTE found = 0;
                    for( int ind=0; ind<msgLen-4 && i<len-1; ind++ ) {
                        BYTE data = h221Array[++i];
                        if( found )
                            continue;
                        if( count1 == 0xB5  &&  count2 == 0x00 ) {     // USA country code
                            if( manuf1 == 0x00  &&  manuf2 == 0x01 ) { // PictureTel manufacturer code
                                switch( data ) {
                                    case 0x87 : {
                                        msg << "Audio: (NonStandard) Au_Siren7_16k\n";
                                        found = 1;
                                        break;
                                    }
                                    case 0x88 : {
                                        msg << "Audio: (NonStandard) Au_Siren7_24k\n";
                                        found = 1;
                                        break;
                                    }
                                    case 0x89 : {
                                        msg << "Audio: (NonStandard) Au_Siren7_32k\n";
                                        found = 1;
                                        break;
                                    }
                                    case 0x8A : {
                                        msg << "Audio: (NonStandard) Au_Siren14_24k\n";
                                        found = 1;
                                        break;
                                    }
                                    case 0x8B : {
                                        msg << "Audio: (NonStandard) Au_Siren14_32k\n";
                                        found = 1;
                                        break;
                                    }
                                    case 0x8C : {
                                        msg << "Audio: (NonStandard) Au_Siren14_48k\n";
                                        found = 1;
                                        break;
                                    }
                                    case 0xA6:{
										msg << "other: (NonStandard) H264*\n";
                                        found = 1;
                                        break;
                                    }

                                    default : {
                                        msg << "NonStandard PictureTel's unknown command\n";
                                        break;
                                    }
                                }
                            }
							else  if( manuf1 == 0x50  &&  manuf2 == 0x50 ) { // Public PP manufacturer code
								BYTE prevCounter = i;
								switch( data ) {
                                    case 0x9B : {
					if(i<len-4)
					{
						BYTE ContentRate;
						unsigned short cntntKb;
						BYTE ControlID					  = h221Array[++i];
						BYTE StartSubChannel		  = h221Array[++i];
						BYTE EndSubChannel           = h221Array[++i];
						BYTE Mode                          = h221Array[++i];

						// calculate rate
						 ContentRate = CalculateRate(StartSubChannel, EndSubChannel);
		                                    msg << "AMSC Rate: "<< Get_Content_command_Bitrate(ContentRate,&cntntKb) <<"  \n";

		                                    found = 1;
				}
                                        break;
                                    }
				case 0x9D : {
					if(i<len-1)
					{
						BYTE ControlID					  = h221Array[++i];
		                                     msg << "AMSC Rate: AMSC-Off\n";
		                                     found = 1;
				  	}
                                        break;
                                    }
			case 0x8F:{
				if(i<len-1)
				{
					BYTE ContentLabel					  = h221Array[++i];
					BYTE count;
					 msg << "People+Content Role: Content\n";
	                                      found = 1;
					i++;
					GetNextNSCom(msg,(h221Array + i),&count);
					i = i + count;
				}
				break;
			}
			default :{
                                        msg << "NonStandard Public PP  unknown command\n";
                                        break;
                                    }

								}//switch
								ind = ind + (i - prevCounter);
							} // else if
                        }
                    }
                }
                else
                {
                    FPTRACE(eLevelInfoNormal,"::DumpH221Stream - unknown escape attr.");
                    DBGFPASSERT(1);
                }
                break;
            }
            case 100 :
            case 101 : {

                FPTRACE(eLevelInfoNormal,"::DumpH221Stream : capabilities should  not appear here!!!");
                DBGFPASSERT(1);

            }
            default : {

                FPTRACE(eLevelInfoNormal,"::DumpH221Stream : Unknown MSB!!!");
                DBGFPASSERT(1);

                break;
            }
        } //switch(msb)

    }
    msg <<"\n";
}
/////////////////////////////////////////////////////////////////////////////
static void OnMuxH230Bas(BYTE opcode, std::ostream& ostr)
{
  switch ( opcode ) {
    case (Freeze_Pic  | OTHRCMDATTR)  :  {
      ostr << "  VCF";
      break;
    }
    case (Fast_Update | OTHRCMDATTR)  :  {
      ostr << "  VCU";
      break;
    }
    case (Au_Loop     | OTHRCMDATTR)  :  {
      ostr << "  Au_Loop";
      break;
    }
    case (Vid_Loop    | OTHRCMDATTR)  :  {
      ostr << "  Vid_Loop";
      break;
    }
    case (Dig_Loop    | OTHRCMDATTR)  :  {
      ostr << "  Dig_Loop";
      break;
    }
    case (Loop_Off    | OTHRCMDATTR)  :  {
      ostr << "  Loop_Off";
      break;
    }
    case (Au_Neutral  | AUDCMDATTR)	  :	 {
      ostr << "  Neutral";
      break;
    }
    default   :  {
      ostr << "  " << (hex) << (DWORD)opcode << "  UNKNOWN H230 BAS byte!!!";
      break;
    }
  }
}
/////////////////////////////////////////////////////////////////////////////
static void OnH239Message(CSegment* pParam, std::ostream& ostr)
{
	BYTE subMsgIdentifier = 0, PID = 0, acknowledge_reject = 0;
	int channelID, bitRate,terminalLabel, symmetryBreaking;
	*pParam >> subMsgIdentifier;
	switch( subMsgIdentifier )
	{

		case FlowControlReleaseRequest:{

			ostr << "=Flow Control Release Request";
			channelID = MbeBytesToInt(pParam);
			bitRate = MbeBytesToInt(pParam);
			ostr	<< "  ChannelID="<< channelID
					<< "  BitRate=" << bitRate;
			break;
										 }
		case FlowControlReleaseResponse: {

			ostr << "=Flow Control Release Response";
			*pParam >> acknowledge_reject;
			channelID = MbeBytesToInt(pParam);

			if(acknowledge_reject==126)
				ostr << " acknowledge";
			else
				ostr << " reject";
			ostr	<< "  Channel ID="<< channelID;
			break;
											}
		case PresentationTokenRequest: {

			ostr << "=Presentation Token Request";
			terminalLabel = MbeBytesToInt(pParam);
			channelID = MbeBytesToInt(pParam);
			symmetryBreaking = MbeBytesToInt(pParam);

			ostr <<" Terminal Label=" << terminalLabel << " Channel ID=" << channelID  << " Symmetry Breaking=" << symmetryBreaking ;
			break;
											}
		case PresentationTokenResponse: {

			ostr << "=Presentation Token Response";

			*pParam >> acknowledge_reject;
			terminalLabel = MbeBytesToInt(pParam);
			channelID = MbeBytesToInt(pParam);
			if(acknowledge_reject==126)
				ostr << " acknowledge";
			else
				ostr << " reject";
			ostr	<< " Terminal Label=" << terminalLabel <<"  Channel ID="<< channelID;
			break;
										}
		case PresentationTokenRelease: {

			ostr << "=Presentation Token Release";
			terminalLabel = MbeBytesToInt(pParam);
			channelID = MbeBytesToInt(pParam);

			ostr <<" Terminal Label=" << terminalLabel << " Channel ID=" << channelID ;
			break;
										}
		case PresentationTokenIndicateOwner: {

			ostr << "=Presentation Token Indicate Owner";
			terminalLabel = MbeBytesToInt(pParam);
			channelID = MbeBytesToInt(pParam);

			ostr <<" Terminal Label=" << terminalLabel << " Channel ID=" << channelID ;
			break;
									}
		default   :  {
			ostr << "  UNKNOWN H239 Sub Msg Identifier!!!";
			break;
					}

	}//switch
}
/////////////////////////////////////////////////////////////////////////////
static void OnMuxH230Mbe(CSegment* pParam, std::ostream& ostr)
{
  BYTE   msgLen   = 0;
  BYTE   opcode   = 0;
  BYTE   mcuNum   = 0;
  BYTE   terNum   = 0;
  *pParam >> msgLen >> opcode;
  ostr << "Mbe Message " << "Msg Len = " << (WORD)msgLen <<"  Opcode =";
  switch ( opcode ) {
    case TIL  :  {

	  *pParam>> mcuNum;
	   ostr << "  TIL "<<"\n";
	   ostr <<": mcu id - "<<(WORD)(mcuNum) <<"\n";
      for (WORD i=0;i<(msgLen-2);i++)
      {
        *pParam>> terNum;
		 ostr<< (WORD)(terNum) << "\n";
	  }
      break;
    }

    case IIS  :{
      ostr << "  IIS "<<"\n";
      BYTE  tcs_n = 0;
      *pParam >> tcs_n;
	  switch ( tcs_n ) {
       case 1:{ // password
        ostr << "Password  ";
        char  password[30];
		WORD i = 0;
        for ( ;  i <  msgLen - 2 && i<29 ; i++ )
			*pParam >> (BYTE &)password[i];

        password[i] = '\0';
        ostr << password;
		break;
       }
	   case 2:{
	     ostr << "Ident Name ";
         char  IdentName[PARTY_EXTENSION_LENGTH];
		 WORD i = 0;
         for ( ;  i <  msgLen - 2 && i<PARTY_EXTENSION_LENGTH ; i++ )
			*pParam >> (BYTE &)IdentName[i];
         IdentName[i] = '\0';
         ostr << IdentName;
		 break;
	   }
       case 4:{
	     ostr << "extension address : ";
         char  extension_address[30];
		 WORD i = 0;
         for ( ; i <  msgLen - 2 && i<30 ; i++ )
			*pParam >> (BYTE &)extension_address[i];
         if(i < 30)
        	 extension_address[i] = '\0'; // Last character
         ostr << extension_address;
		 break;
	   }
	   default:{
	    break;
	   }
	  }
	  break;
    }
    case TIR  :  {
      BYTE Number[6];
	  BYTE index =0;
	  
	  for(int i = 0; i< 6; i++)
	  	Number[i] = 0;
	  
	  while(!pParam->EndOfSegment() && index<6 )
	         *pParam >> Number[index++];

	  if(index == 6) // Right length.
         ostr << "  TIR" <<"\n";
	  else
         ostr << "  WRONG 'TIR' COMMAND " <<"\n";

      ostr<<" LSD : mcu id - " << (WORD)Number[0] <<"    term id - "<<(WORD)Number[1] <<"\n";
	  ostr<<" HSD : mcu id - " << (WORD)Number[2] <<"    term id - "<<(WORD)Number[3] <<"\n";
      ostr<<" CHAIR : mcu id - " << (WORD)Number[4] <<"    term id - "<<(WORD)Number[5] <<"\n";
      break;
    }
    case TIP  :  {

      *pParam>> mcuNum>>terNum;
	   ostr << "  TIP "<<"\n";
	   ostr<<": mcu id - "<<(WORD)(mcuNum) <<"\n";
	   ostr<<": term id - "<<(WORD)terNum<<"\n";
	   char  IdentName[30];
	   IdentName[0]='\0';
	   WORD i = 0;
       for ( ;  i <  msgLen - 3 && i<29 ; i++ )
	    	*pParam >> (BYTE &)IdentName[i];
       IdentName[i] = '\0';
       ostr<<": ident name - "<<IdentName<<"\n";
      break;
    }
    case NIA  :  {
      ostr << "  NIA";
      break;
    }
    case NIAP  :  {
      ostr << "  NIAP";
      break;
    }
    case Au_MAP  :  {
      ostr << "  Au_MAP";
      break;
    }
    case MRQ  :  {
      ostr << "  MRQ";
      break;
    }
	case VideoNotDecodedMBs  :  {

    ostr << " VideoNotDecodedMBs" <<"\n";

	BYTE bytes[6];
	for ( BYTE i = 0 ;  i < 6 ; i++ ) // Message must include 6 bytes
         *pParam >> bytes[i];

	ostr << /*setw(20) <<*/ "firstMBByte1 : " << (WORD)bytes[0] << /*setw(25)     <<*/ "   firstMBByte2 : " << (WORD)bytes[1]  <<"\n";
	ostr << /*setw(20) <<*/ "numberofMBsByte1 : " << (WORD)bytes[2] << /*setw(25) <<*/ "   numberOfMBsByte2 : " << (WORD)bytes[3]  <<"\n";
	ostr << /*setw(20) <<*/ "trByte1 : " << (WORD)bytes[4] << /*setw(25) <<*/ "   trByte2 : " << (WORD)bytes[5]  <<"\n";
    break;
    }
	case AMC_CI  :  {
		ostr << " AMC C&I" <<"\n";

		BYTE channelID, opcode;
		*pParam >> channelID >> opcode;

		ostr << "  Channel ID=" << (hex) << (int)channelID ;
		if(opcode == (Freeze_Pic  | OTHRCMDATTR))
			ostr <<"  VCF";
		else if(opcode ==(Fast_Update | OTHRCMDATTR ))
			ostr <<"  VCU";
		else if(opcode ==(H261 | OTHRCMDATTR))
			ostr << " H261" ;
		else if(opcode ==(H263 | OTHRCMDATTR))
			ostr << " H263" ;
		else if(opcode ==(H264 | OTHRCMDATTR))
			ostr << " H264" ;
		else if(opcode == (H230_Esc | ESCAPECAPATTR))
		{
			*pParam >> opcode;
			if(opcode == (VIS | Attr000))
				ostr << " VIS ";
			else if(opcode == (VIA | Attr000))
				ostr << " VIA ";
			else if(opcode == (MCS | Attr001))
				ostr << " MCS ";
			else
				ostr << " Unknown AMC C&I, H230_Esc" ;
		}

		else
			ostr << " Unknown AMC C&I" ;
		break;
    }
	case H239_messsage : {
        OnH239Message(pParam,ostr);
		break;
	}
	case h239ExtendedVideoCapability : {
		BYTE PID, roleLabel, zero;
		*pParam >>PID;
		*pParam >>roleLabel;
		*pParam >>zero ;
		ostr << "h239 Extended Video Capability: \nRole Label = "<< (WORD)roleLabel;
		if(zero)
			{FPASSERT(zero);}
		ostr << "\nVideo Caps = ";
		BYTE * tempArray = new BYTE[(msgLen-4)];
		for(int i =0; i<(msgLen-4); i++)
			*pParam >>tempArray[i];
		CCDRUtils::DumpVideoCap(tempArray, (msgLen-4),ostr);
		delete [] tempArray; //bug 21946 memory leak
		break;
	}
	case AMC_cap : {
		BYTE optionByte1, optionByte2;
		*pParam >>optionByte1;
		*pParam >>optionByte2;
		ostr << "AMC_cap: "<<(hex) <<(WORD)optionByte1<<" "<<(hex)<<(WORD)optionByte2;
		break;
	}
    default   :  {
      ostr << "  UNKNOWN H230 Mbe!!!" <<"  Opcode = "<< (WORD)opcode ;

	  BYTE TmpByte;

	  if((pParam ->GetRdOffset()+ msgLen-1)> pParam ->GetWrtOffset())
	  {
	     BYTE   TmpLen;
		 TmpLen = pParam ->GetWrtOffset() - pParam ->GetRdOffset();
         for( BYTE i=0; i < TmpLen; i++ )
			*pParam >> TmpByte;
	  }
	  else
		  for( BYTE i=0; i<msgLen-1; i++ )
			*pParam >> TmpByte;

 	  break;
    }
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int ComputeRate( BYTE StartSubChannel,   BYTE EndSubChannel)
{
	if(StartSubChannel == EndSubChannel)
		return 0;
		 BYTE     startBitNum, startTimeslot;
		 BYTE     endBitNum, endTimeslot;

		 startBitNum = StartSubChannel >>5; // for future needes
		 startTimeslot = StartSubChannel & 0x1F;

		 endBitNum = EndSubChannel >> 5; // for future needes
		 endTimeslot = EndSubChannel &0x1F;

		  if((startBitNum == 0)    && (endBitNum == 7))// only this case is supported now
		  {

		 //only timeslot values will be used currently for calculating ContentRate

		 if((endTimeslot - startTimeslot) >= 0)
		 {
			 switch(endTimeslot - startTimeslot)
			 {
				 case 0: {
					  return 64;
				 }
				 case 1: {
					  return 128;
						}
				 case 2: {
					  return 192;
						}
				 case 3: {
					  return 256;
						}
				 case 4: {
					  return 384;
						}
				 case 5: {
					  return 512;
						}
				 case 6: {
					  return 768;
						}
				 case 7: {
					  return 1152;
						}
				 case 8: {
					  return 1536;
						  }
				  default :
					  { return -1; }
				  }
			  }
			  else
			  {
				  return -1;
			  }

		  }
		  else
		  {
			  return -1;
		  }


 }
/////////////////////////////////////////////////////////////////////////////
static void OnMuxH230(CSegment* pParam, std::ostream& ostr)
{
    BYTE   opcode = 0;

    *pParam >> opcode;

    switch ( opcode ) {
    	case (Data_Apps_Esc | ESCAPECAPATTR): {
            *pParam >>  opcode;
            ostr << "  T120 Message: ";
            if(opcode == (T120_ON | Attr011))
                ostr << "  T120_ON ";
            else
                ostr << "  UNKNOWN!!!";
            break;
        }

        case (Ns_Com | ESCAPECAPATTR):{
            BYTE  msgLen, co1, co2, ma1, ma2, dump, isPictureTel = 0;
			BYTE publicPP = 0;
            *pParam >> msgLen >> co1 >> co2 >> ma1 >> ma2 ;
            ostr << "  NS_Comm  Message! \n" ;

            if( co1 == 0xB5 && co2 == 0x00 && ma1 == 0x00 && ma2 == 0x01 )
                isPictureTel = 1;
			else if(co1 == 0xB5 && co2 == 0x00 && ma1 == 0x50 && ma2 == 0x50)
				publicPP = 1;

            ostr << "    < " << (int)msgLen << "; " << (hex) << (DWORD)co1 << "," << (hex) << (DWORD)co2 << "; "
                 << (hex) << (DWORD)ma1 << "," << (hex) << (DWORD)ma2 << "; " ;

            for( BYTE i=0; i<msgLen-4; i++ ) {
                *pParam >> dump;
                ostr << (hex) << (DWORD)dump;// << "; ";

                // PictureTel commands
                if( isPictureTel ) {
                    switch( dump )
                    {
                        case NS_COM_SIREN716_ON : {
                            ostr << "(PictureTel' Siren7-16 kbps ON)";
                            break;
                        }
                        case NS_COM_SIREN724_ON : {
                            ostr << "(PictureTel' Siren7-24 kbps ON)";
                            break;
                        }
                        case NS_COM_SIREN732_ON : {
                            ostr << "(PictureTel' Siren7-32 kbps ON)";
                            break;
                        }
                        case NS_COM_SIREN1424_ON : {
                            ostr << "(PictureTel' Siren14-24 kbps ON)";
                            break;
                        }
                        case NS_COM_SIREN1432_ON : {
                            ostr << "(PictureTel' Siren14-32 kbps ON)";
                            break;
                        }
                        case NS_COM_SIREN1448_ON : {
                            ostr << "(PictureTel' Siren14-48 kbps ON)";
                            break;
                        }
                        case NS_COM_CONTENT_ON : {
                            ostr << "(PictureTel' ContentVisualization)";
                            break;
                        }
						case NS_COM_DBC2_ON : {
							BYTE p1_Id,p2_RR,p3_interLeave,p4_options,p5_mpiLimit;
							*pParam >> p1_Id >> p2_RR >> p3_interLeave >> p4_options >> p5_mpiLimit;
                            ostr << "\n(PictureTel' DBC2 ON) \n"
								<< "  AMSC ID     = " << (hex) << (DWORD)p1_Id
								<< "  RefreshRate = " << (hex) << (DWORD)p2_RR
								<< "  InterLeave  = " << (hex) << (DWORD)p3_interLeave
								<< "  Options     = " << (hex) << (DWORD)p4_options
								<< "  MpiLimit    = " << (hex) << (DWORD)p5_mpiLimit;
							i= i+5;
                            break;
                        }
						case NS_COM_DBC2_OFF : {
							BYTE p1_Id;
							*pParam >> p1_Id;
                            ostr << "(PictureTel' DBC2 OFF) /n" << "AMSC ID" << (hex) << (DWORD)p1_Id;
							i=i+1;
                            break;
                        }
                        case NS_COM_H26L_ON: {
                            ostr << "(PictureTel' H26L ON)";
                            break;
                        }
                        default : {
                            ostr << "(PictureTel' unknown)";
                            break;
                        }
                    }
                }//isPictureTel


				// public PP commands
				if( publicPP ) {
					switch( dump )
					{
						case NS_COM_AMSC_ON : {
							BYTE  ControlID, StartSubChannel, EndSubChannel, Mode;

							*pParam >> ControlID >> StartSubChannel >> EndSubChannel >> Mode;

							ostr << "  controlID =" << (hex) << (int)ControlID  << " startSubChannel =  " << (hex) << (int)StartSubChannel
								 << " endSubChannl = " << (hex) << (int)EndSubChannel << (dec) << "   => content Rate = AMSC_"  << (int)ComputeRate(StartSubChannel, EndSubChannel) <<"k "
								 << " media mode = "  << (hex) << (int)Mode  ;

							i= i+4;

							break;
											  }
						case NS_COM_AMSC_OFF:{
							BYTE ControlID;

							*pParam >> ControlID ;
							ostr << "  controlID =" <<(hex) << (int)ControlID  ;
							i = i+1;
							break;
											 }
						case NS_COM_ROLE_LABEL:{
								BYTE ContentLabel;

								*pParam >> ContentLabel ;
								 ostr << "  ContentLabel" <<(hex) << (int) ContentLabel ;
								 i = i+1;
								 //GetNextNSCom( pParam, ostr);
							break;
											   }
						case NS_COM_AMSC_H230_C_I:{

							ostr << "=AMSC_H230_C_I";

							BYTE ControlID, code;

							*pParam >> ControlID ;
							ostr << "  controlID=" << (hex) << (int)ControlID ;

							*pParam >> code;
							if(code == (Freeze_Pic  | OTHRCMDATTR))
								ostr <<"  VCF";
							else if(code ==(Fast_Update | OTHRCMDATTR ))
								ostr <<"  VCU";
							else
								ostr << " Unknown transmit opcode" ;
							i = i+2;
							break;
												  }
						case NS_COM_AMSC_H230_MBE:{

							ostr << "=AMSC_H230_MBE";

							BYTE ControlID, code;

							*pParam >> ControlID ;
							ostr << "  controlID=" << (hex) << (int)ControlID ;

							*pParam >> code;
							if(code == (Freeze_Pic  | OTHRCMDATTR))
								ostr <<"  VCF";
							else if(code ==(Fast_Update | OTHRCMDATTR ))
								ostr <<"  VCU";
							else
								ostr << " Unknown transmit opcode" ;
							i = i+2;
							break;
												  }
						case NS_COM_AMSC_RATE_CHANGE:{

							ostr << "=AMSC_RATE_CHANGE";

							BYTE ParamlID, startID, endID;

							*pParam >> ParamlID >> startID >> endID ;
							ostr << "  ParamlID=" << (hex) << (int)ParamlID << "  startID=" <<(hex) << (int) startID << "  endID=" << (hex) << (int)endID ;
							i = i+3;
							break;
													 }
						case NS_COM_MEDIA_PRODUCER_STATUS:{

							ostr << "=MEDIA_PRODUCER_STATUS";

							BYTE ChannelID, status;

							*pParam >> ChannelID >> status ;
							ostr << "  ChannelID=" << (hex) << (int)ChannelID << " status=" <<(hex) << (int) status ;
							i = i+2;
							break;
														  }
						case NS_COM_ROLE_TOKEN_ACQUIRE:{

							ostr << "=ROLE_TOKEN_ACQUIRE";

							BYTE MCUNumber, terminalNumber, randomNumber, label;

							*pParam >> MCUNumber >> terminalNumber >> randomNumber >> label;
							ostr << "  <M>="<< (hex) << (int)MCUNumber
								 << "  <T>=" <<(hex) << (int) terminalNumber  << "  RandomNumber=" << (hex) << (int)randomNumber
								 << "  label=" << (hex) << (int)label ;
							i = i+4;
							break;
												   }
						case NS_COM_ROLE_TOKEN_ACQUIRE_ACK: {

							ostr << "=ROLE_TOKEN_ACQUIRE_ACK";

							BYTE MCUNumber, terminalNumber,  label;

							*pParam >> MCUNumber >> terminalNumber >> label;
							ostr <<" <M>=" << (hex) << (int)MCUNumber << "  <T>=" <<(hex) << (int) terminalNumber  << " label=" <<(hex) << (int) label ;
							i = i+3;
							break;
															}
						case NS_COM_ROLE_TOKEN_ACQUIRE_NAK: {

							ostr << "=ROLE_TOKEN_ACQUIRE_NAK";

							BYTE MCUNumber, terminalNumber,  label;

							*pParam >> MCUNumber >> terminalNumber >> label;
							ostr <<" <M>=" << (hex) << (int)MCUNumber << "  <T>=" <<(hex) << (int) terminalNumber  << " label=" <<(hex) << (int) label ;
							i = i+3;
							break;
															}
						case NS_COM_ROLE_TOKEN_RELEASE: {

							ostr << "=ROLE_TOKEN_RELEASE";

							BYTE MCUNumber, terminalNumber,  label;

							*pParam >> MCUNumber >> terminalNumber >> label;
							ostr <<" <M>=" << (hex) << (int)MCUNumber << "  <T>=" <<(hex) << (int) terminalNumber  << " label=" <<(hex) << (int) label ;
							i = i+3;
							break;
														}
						case NS_COM_ROLE_TOKEN_RELEASE_ACK: {

							ostr << "=ROLE_TOKEN_RELEASE_ACK";

							BYTE MCUNumber, terminalNumber,  label;

							*pParam >> MCUNumber >> terminalNumber >> label;
							ostr <<" <M>=" << (hex) << (int)MCUNumber << "  <T>=" <<(hex) << (int) terminalNumber  << " label=" <<(hex) << (int) label ;
							i = i+3;
							break;
															}
						case NS_COM_ROLE_TOKEN_WITHDRAW: {

							ostr << "=ROLE_TOKEN_WITHDRAW";

							BYTE MCUNumber, terminalNumber,  label;

							*pParam >> MCUNumber >> terminalNumber >> label;
							ostr <<" <M>=" << (hex) << (int)MCUNumber << "  <T>=" <<(hex) << (int) terminalNumber  << " label=" <<(hex) << (int) label ;
							i = i+3;
							break;
														 }
						case NS_COM_ROLE_TOKEN_WITHDRAW_ACK: {

							ostr << "=ROLE_TOKEN_WITHDRAW_ACK";

							BYTE MCUNumber, terminalNumber,  label;

							*pParam >> MCUNumber >> terminalNumber >> label;
							ostr <<" <M>=" << (hex) << (int)MCUNumber << "  <T>=" <<(hex) << (int) terminalNumber  << " label=" <<(hex) << (int) label ;
							i = i+3;
							break;
															 }
						case NS_COM_ROLE_PROVIDER_IDENTITY:{

							ostr << "=ROLE_PROVIDER_IDENTITY";

							BYTE  MCUNumber, terminalNumber, label;

							*pParam >> MCUNumber >> terminalNumber >> label;

							//BYTE tempSize = msgLen - 8; // 4 for headers: country*2 + manuf*2  and 4 :  opcode + mcunum + termnum + label*
							ostr <<" <M>=" << (hex) << (int)MCUNumber << " <T>=" <<(hex) << (int) terminalNumber  << " label=" <<(hex) << (int) label ;
							//	<< "role specific parameters:" << *pParam;
							i = i+3;
							break;
														   }
						case NS_COM_NO_ROLE_PROVIDER:{

							ostr << "=NO_ROLE_PROVIDER";

							BYTE ContentLabel;

							*pParam >> ContentLabel ;
							ostr << "  ContentLabel=" <<(hex) << (int) ContentLabel ;
							i = i+1;
							break;
													 }
						case NS_COM_REQUEST_RATE_CHANGE:{

							ostr << "=REQUEST_RATE_CHANGE";

							BYTE  ChannelID, MaxBitRateL, MaxBitRateH;

							*pParam >> ChannelID >> MaxBitRateL >> MaxBitRateH;
							ostr <<" ChannelID=" << (hex) << (int)ChannelID << "  MaxBitRateL=" <<(hex) << (int) MaxBitRateL  << " MaxBitRateH=" <<(hex) << (int) MaxBitRateH ;
							i = i+3;
							break;
													 }
						default:{
							ostr << "(Public PP' unknown)";
							break;
								}
					}//switch
				} // if( publicPP )
                ostr << "; ";
            }//for

            ostr << ">";// closes bytes string
            break;
        } //case (Ns_Com | ESCAPECAPATTR)

        case (Start_Mbe | ESCAPECAPATTR)  :  {
            OnMuxH230Mbe(pParam,ostr);
            break;
        }

        case (H230_Esc | ESCAPECAPATTR) :  {
            WORD i = 0;
            H230_ENTRY*  pEntry = g_H230Entries;;
            BYTE opcode;
            *pParam >> opcode;
            while ( pEntry->endMark != 1 ) {
                if ( pEntry->opcode == opcode )  {
                    ostr << "  " << pEntry->opcodeStr << "   "  << pEntry->descStr;
                    switch ( opcode ) {
                        case TIA | Attr001 :  {
                            BYTE  mcu,term,sbe;
                            *pParam >> sbe >> mcu >> sbe >> term;
                            ostr << "  TIA "<<"\n";
                            ostr << ": mcu id - " << (WORD)mcu
                                 << " terminal id - " << (WORD)term;
                            break;
                        }

                        case DCA_L | Attr010 :  {
                            BYTE  bitRate, sbe;
                            *pParam >> sbe >> bitRate;
                            ostr << "\nLSD bit Rate - " <<  (WORD)bitRate ;
                            break;
                        }
                        case TIN | Attr001 :  {
                            BYTE  mcu,term,sbe;
                            *pParam >> sbe >> mcu >> sbe >> term;
                            ostr << "  TIN "<<"\n";
                            ostr << ": mcu id - " << (WORD)mcu
                                 << " terminal id - " << (WORD)term;
                            break;
                        }
                        case TID| Attr001 :  {
                            BYTE  mcu,term,sbe;
                            *pParam >> sbe >> mcu >> sbe >> term;
                            ostr << "  TID "<<"\n";
                            ostr << ": mcu id - " << (WORD)mcu
                                 << " terminal id - " << (WORD)term;
                            break;
                        }
                        case VCB| Attr001 :  {
                            BYTE  mcu,term,sbe;
                            *pParam >> sbe >> mcu >> sbe >> term;
                            ostr << "  VCB "<<"\n";
                            ostr << ": mcu id - " << (WORD)mcu
                                 << " terminal id - " << (WORD)term;
                            break;
                        }
                        case TIF | Attr010 :  {
                            BYTE  mcu,term,sbe;
                            *pParam >> sbe >> mcu >> sbe >> term;
                            ostr << "  TIF "<<"\n";
                            ostr << ": mcu id - " << (WORD)mcu
                                 << " terminal id - " << (WORD)term;
                            break;
                        }
                        case TII| Attr000 :{
                            BYTE character,sbeStorage;
                            *pParam >> sbeStorage>>character;
                            ostr<< " TII "<<"\n";
                            ostr<<" the character is: "<<character;
                            break;
                        }
                        case TCP_  | Attr011:{
                            BYTE term,mcu,sbe;
                            *pParam>>sbe >>mcu>> sbe >>term;
                            ostr << " TCP "<<"\n";
                            ostr << ": mcu id - " << (WORD)mcu
                                 << " terminal id - " << (WORD)term;
                            break;
                        }
                        case CCD | Attr010:{
                            BYTE term,mcu,sbe;
                            *pParam >> sbe >> mcu >> sbe >> term;
                            ostr<<" CCD "<<"\n";
                            ostr << ": mcu id - " << (WORD)mcu
                                 << " terminal id - " << (WORD)term;
                            break;
                        }
                        case VIN|Attr001:{
                            BYTE term,mcu,sbe;
                            *pParam >> sbe >> mcu >> sbe >> term;
                            ostr << " VIN " << "\n";
                            ostr << ": mcu id - " << (WORD)mcu
                                 << " terminal id - " << (WORD)term;
                            break;
                        }
                        case VCS | Attr001 :  {
                            BYTE  mcu,term,sbe;
                            *pParam >> sbe >> mcu >> sbe >> term;
                            ostr << "  VCS " << "\n";
                            ostr << ": mcu id - " << (WORD)mcu
                                 << " terminal id - " << (WORD)term;
                            break;
                        }
						case AMC_open | Attr101 :  {
                            BYTE  AMCopenByte1, AMCopenByte2, sbe;
                            *pParam >> sbe >> AMCopenByte1 >> sbe >> AMCopenByte2;
                            ostr << "  AMC-open " << "\n";
                            ostr << ": AMCopenByte1 - " << (WORD)AMCopenByte1
                                 << " SubTimeSlotCount - " << (WORD)AMCopenByte2;
                            break;
                        }
						case AMC_close | Attr101 :  {
                            BYTE  AMCcloseByte1, sbe;
                            *pParam >> sbe >> AMCcloseByte1;
                            ostr << "  AMC-close " << "\n";
                            ostr << ": AMCcloseByte1 - " << (WORD)AMCcloseByte1;
                            break;
                        }

                        default   :  {
                            break;
                        }
                    }

                    break;
                }
                pEntry++;
            }
            if ( pEntry->endMark == 1 )  {
                ostr << "  " << (hex) << (DWORD) opcode << " - UNKNOWN H230 opcode";
            }
            break;
        }
        default   :  {  // bas h230 opcodes
            OnMuxH230Bas(opcode,ostr);
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CCDRUtils::Dump230Opcode(CSegment& h221seg, std::ostream& ostr)
{
    DWORD segDesc;
	h221seg >> segDesc;  // skip segment descriptor
	while ( ! h221seg.EndOfSegment() )  {
	    OnMuxH230(&h221seg,ostr);
		ostr << "\n";
	}
 	h221seg.ResetRead();
}
///////////////////////////////////////////////////////////////////////////
void CCDRUtils::GetNextNSCom(std::ostream& msg,BYTE* h221Array, BYTE * count)
{
	WORD i = 0;
	if( h221Array[i] == (ESCAPECAPATTR | Ns_Com) ) {

            BYTE msgLen = h221Array[++i];                          // message Lenght
            BYTE count1 = h221Array[++i], count2 = h221Array[++i]; // country code
            BYTE manuf1 = h221Array[++i], manuf2 = h221Array[++i]; // manufacturer code

            //for( int ind=0; ind<msgLen-4 && i<len; ind++ ) {
             BYTE data = h221Array[++i];

                if( count1 == 0xB5  &&  count2 == 0x00  &&    // USA country code
				     manuf1 == 0x50  &&  manuf2 == 0x50 ) { // Public PP manufacturer code
						switch( data ) {
                            case 0x9B : {
								BYTE ContentRate;
								unsigned short cntntKb;
								BYTE ControlID					  = h221Array[++i];
								BYTE StartSubChannel		  = h221Array[++i];
								BYTE EndSubChannel           = h221Array[++i];
								BYTE Mode                          = h221Array[++i];

								// calculate rate
								 ContentRate = CalculateRate(StartSubChannel, EndSubChannel);

                                msg << "AMSC Rate: "<< Get_Content_command_Bitrate(ContentRate,&cntntKb) <<"  \n";

                                break;
                            }
							case 0x9D : {
								BYTE ControlID					  = h221Array[++i];
                                msg << "AMSC Rate: AMSC-Off\n";
                                break;
                            }

							default :{
                                msg << "NonStandard Public PP  unknown command\n";
                                break;
                            }

						}//switch
				*count = i;
				}
				else
				{
                           msg << "NonStandard country or manuf code \n";

				}
			//}
	}
	else
	{
		  msg << "unknown escape attr (" << (hex) << (int) h221Array[i] << ")";
	}

}
////////////////////////////////////////////////////////////////////////////
BYTE CCDRUtils::CalculateRate( BYTE StartSubChannel, BYTE EndSubChannel )
{
	if(StartSubChannel == EndSubChannel)
		return (BYTE)AMSC_0k;

	BYTE	startBitNum, startTimeslot;
	BYTE	endBitNum, endTimeslot;

	startBitNum = StartSubChannel >>5; // for future needes
	startTimeslot = StartSubChannel & 0x1F;

	endBitNum = EndSubChannel >> 5; // for future needes
	endTimeslot = EndSubChannel &0x1F;

	// endBitNum == 6  for restricted call
	if( (startBitNum == 0)  &&  (endBitNum == 7 || endBitNum == 6) )// only this case is supported now
	{

		//only timeslot values will be used currently for calculating ContentRate
		if((endTimeslot - startTimeslot) >= 0)
		{
			switch(endTimeslot - startTimeslot)
			{
				case 0: {
					return (BYTE)AMSC_64k;
				}
				case 1: {
					return (BYTE)AMSC_128k;
						}
				case 2: {
					return (BYTE)AMSC_192k;
						}
				case 3: {
					return (BYTE)AMSC_256k;
						}
				case 4: {
					return (BYTE)AMSC_384k;
						}
				case 5: {
					return (BYTE)AMSC_512k;
						}
				case 6: {
					return (BYTE)AMSC_768k;
						}
				case 7: {
					return (BYTE)AMSC_1152k;
						}
				case 8: {
					return (BYTE)AMSC_1536k;
						}
				default :{
					return 100;
						 }
			}
		}
		else
			return 100;
	}
	else
		return 100;
}
////////////////////////////////////////////////////////////////////////////
BYTE CCDRUtils::CalculateH239Rate( WORD subTimeSlotCount )
{
	subTimeSlotCount*=8;

	switch(subTimeSlotCount)
	{
		case 0: {
			return (BYTE)AMC_0k;
			}
		case 40: {
			return (BYTE)AMC_40k;
				}
		case 64: {
			return (BYTE)AMC_64k;
				}
		case 96: {
			return (BYTE)AMC_96k;
				}
		case 128: {
			return (BYTE)AMC_128k;
				}
		case 192: {
			return (BYTE)AMC_192k;
				}
		case 256: {
			return (BYTE)AMC_256k;
				}
		case 384: {
			return (BYTE)AMC_384k;
				}
		case 512: {
			return (BYTE)AMC_512k;
				}
		case 768: {
			return (BYTE)AMC_768k;
				}
		default :{
			DBGFPASSERT(subTimeSlotCount);
				 }
	}
	return (BYTE)AMC_0k;
}
/////////////////////////////////////////////////////////////////////////////
void CCDRUtils::PrintAnnexType(std::ostream& ostr, int annex)
{
	switch(annex)
	{
	case typeAnnexB:
		ostr << "Annex B" << "\n";
		break;
	case typeAnnexD:
		ostr << "Annex D" << "\n";
		break;
	case typeAnnexE:
		ostr << "Annex E" << "\n";
		break;
	case typeAnnexF:
		ostr << "Annex F" << "\n";
		break;
	case typeAnnexG:
		ostr << "Annex G" << "\n";
		break;
	case typeAnnexH:
		ostr << "Annex H" << "\n";
		break;
	case typeAnnexI:
		ostr << "Annex I " << "\n";
		break;
	case typeAnnexJ:
		ostr << "Annex J" << "\n";
		break;
	case typeAnnexK:
		ostr << "Annex K" << "\n";
		break;
	case typeAnnexL:
		ostr << "Annex L" << "\n";
		break;
	case typeAnnexM:
		ostr << "Annex M" << "\n";
		break;
	case typeAnnexN:
		ostr << "Annex N " << "\n";
		break;
	case typeAnnexO:
		ostr << "Annex O" << "\n";
		break;
	case typeAnnexP:
		ostr << "Annex P" << "\n";
		break;
	case typeAnnexQ:
		ostr << "Annex Q" << "\n";
		break;
	case typeAnnexR:
		ostr << "Annex R" << "\n";
		break;
	case typeAnnexS:
		ostr << "Annex S" << "\n";
		break;
	case typeAnnexT:
		ostr << "Annex T" << "\n";
		break;
	case typeAnnexU:
		ostr << "Annex U" << "\n";
		break;
	case typeAnnexV:
		ostr << "Annex V" << "\n";
		break;
	case typeAnnexW:
		ostr << "Annex W" << "\n";
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
//dump of H221 capabilits vector
void CCDRUtils::CdrDumpH323Cap(BYTE *h221str, WORD length, std::ostream& ostr, BYTE capflag)
{
    BYTE capAttr,cap;
    WORD i=0;

    if(capflag==0)
        ostr <<"\n**** capabilities ****\n";
    else
        ostr << "**** remote comm mode ****\n";
    SmartDumpCap(h221str,length,ostr); //report audio+video cap

    while ( i < length )  {
        cap = h221str[i++];
        capAttr = cap & 0xE0;
        switch ( capAttr ) {

            case AUDRATECAPATTR  :  {
				if((cap ^ capAttr) < 32)
                    ostr << audXfer[cap ^ capAttr];
                break;
            }

            case DATAVIDCAPATTR  :  {
				if((cap ^ capAttr) < 32)
                    ostr << datVid[cap ^ capAttr];
                break;
            }

            case OTHERCAPATTR  :  {
				if((cap ^ capAttr) < 32)
                    ostr << restrict[cap ^ capAttr];
                break;
            }

            case ESCAPECAPATTR  :  {
                if(i < length)  {
                    if ( cap == (Hsd_Esc | ESCAPECAPATTR) )  {
                        //ostr << "HSD/HMLP or MLP(" << (hex) << (int) cap << ",";
                        cap = h221str[i++]; // skip cap value
                        BYTE tab2Attr = cap & 0xE0;
                        BYTE tab2Val  = cap & 0x1F;
                        switch(tab2Attr) {
                            case HSDHMPLCAPATTR : {
								if((cap ^ tab2Attr) < 32)
                                    ostr << hsdHmlp[cap ^ tab2Attr];
                                break;
                            }
                            case MLPCAPATTR : {
								if((cap ^ tab2Attr) < 32)
                                    ostr << Mlp[cap ^ tab2Attr];
                                break;
                            }
                        }
                        //ostr << (hex) << (int) cap << ")";
                    }
                    if ( cap == (H230_Esc | ESCAPECAPATTR) )  {
                        cap = h221str[i++];
                        if ( cap == (CIC | Attr010) ) ostr << "CHAIR";
                        if ( cap == (TIC | Attr001) ) ostr << "TIC";
                        if ( cap == (MIH | Attr001) ) ostr << "MIH";
                        if ( cap == (MVC | Attr010) ) ostr << "MVC";
                    }
                    if ( cap == (Data_Apps_Esc | ESCAPECAPATTR) )  {
                        cap = h221str[i++];
                        if ( cap == ((28 ) | (5<<5) ) ) ostr << "T120-Cap";
                    }
                    if( cap == (Start_Mbe | ESCAPECAPATTR) ) {
                        BYTE len = 0;
                        int h263CapsStartPosition = 0;
                        BYTE table2_H230Value;
                        len = h221str[i++]-1;
                        table2_H230Value = h221str[i++];
                        h263CapsStartPosition = i;

						if(table2_H230Value == H262_H263 || table2_H230Value == H264_Mbe )//H264_Mbe
						{
							//In case of mux /net sync,which causes a corrupted message, we reset the
							//length of H263 message to the Left Length Of H221 Cap message.
							WORD LeftLenghOfH221Cap = length - h263CapsStartPosition;
							if(len > LeftLenghOfH221Cap )
							{

								FPTRACE(eLevelInfoNormal,"::FullDumpCap - H263 message length is corrupted!!! \'");
								DBGFPASSERT(1);

								len = LeftLenghOfH221Cap;
							}

							//The table2_H230Value is correct therefore we dump the message.
							if(table2_H230Value == H262_H263)
								DumpCapH263( ostr, len, (h221str + h263CapsStartPosition) );
							else if(table2_H230Value == H264_Mbe) //H264_Mbe
								DumpCapH264( ostr, len, (h221str + h263CapsStartPosition) );
						}
						else
						{

							FPTRACE(eLevelInfoNormal,"::DumpCap - Mbe H263/H264 message is corrupted!!!");
							DBGFPASSERT(1);

							ostr << "\n" << "Mbe H263 or H264 message is corrupted!!!" << "\n" ;
						}

						i = i + len;
                    }

                    if( cap == (ESCAPECAPATTR | Ns_Cap) ) {
                        BYTE  msgLen, co1, co2, ma1, ma2, data;
                        msgLen = co1 = co2 = ma1 = ma2 = data = 0xFF;
                        ostr << "***NonStandard Caps*****";
                        msgLen = h221str[i++]; // Msg Len
                        ostr << "\nMsg len  : (" << (dec) << (int)msgLen << ")";
                        if( msgLen < 5 ) ostr << "- WRONG !!!";
                        if( msgLen >= 1 ) co1 = h221str[i++];
                        if( msgLen >= 2 ) co2 = h221str[i++];
                        ostr << "\nCountry  : (" << (hex) << (WORD)co1 << "::"
                                                 << (hex) << (WORD)co2 << ")";
                        if( msgLen >= 3 ) ma1 = h221str[i++];
                        if( msgLen >= 4 ) ma2 = h221str[i++];
                        ostr << "\nManufact : (" << (hex) << (WORD)ma1 << "::"
                                                 << (hex) << (WORD)ma2 << ")";
                        if( msgLen >= 5 ) {
                            ostr << "\nData     : ";
                            for( int ind=0; ind<msgLen-4; ind++ ) {
                                data = h221str[i++];
                                ostr << " (" << (hex) << (WORD)data << ")";
                            }
                            ostr << "\n";
                        }
                    }
                }
                break;
            }

            default   :  {
                if(capflag==0)
                    FPTRACE(eLevelError, "::DumpCap - \'unknown capability !!! \'");
                else
                    FPTRACE(eLevelError, "::DumpCap - \'unknown remote mode !!! \'");
                break;
            }
        }
        ostr << "\n";
    }
}
/////////////////////////////////////////////////////////////////////////////

//dump of H221 BAS commands vector
void CCDRUtils::CdrDumpH221Stream(std::ostream& msg, WORD len,BYTE* h221Array)
{

  WORD bitRate;
	BYTE msb;
	msg << "\n**** remote comm mode ****\n";
  for (WORD i = 0 ; i < len ; i++){
		msb = h221Array[i] & 0xE0;
    switch(msb){
    case AUDCMDATTR:{ // 000
      msg << "Audio:" << Get_Audio_Coding_Command_BitRate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
      break;
		    }
    case XFERCMDATTR:{ // 001
      msg << "Transfer:" << Get_Transfer_Rate_Command_BitRate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
      break;
		     }
    case OTHRCMDATTR:{ // 010
      msg << "other:" << Get_Video_Oth_Command_BitRate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
      break;
		      }
    case LSDMLPCMDATTR:{ // 011
      msg << "LSD/MLP:" << Get_Lsd_Mlp_Command_BitRate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
      break;
		       }
    case ESCAPECAPATTR:{ // 111
        if (h221Array[i] == (Data_Apps_Esc |  ESCAPECAPATTR)) {
            i++;
            msg << "    H224 STATUS: " << Get_Lsd_Hsd_Mlp_Command(h221Array[i]) << " \n";
            break;
      }
      // esc : discard this byte and look at nxt bye for hsd command
      if (h221Array[i] == ESCHSDATTR){
	i++;
	msg << "HSD/HMLP:" << Get_Hsd_Hmlp_command_Bitrate(h221Array[i] & 0x1F ,&bitRate ) << " \n";
      }
      else
      {
    	  FPTRACE(eLevelInfoNormal,"::DumpH221Stream - unknown escape attr.");
    	  DBGFPASSERT(1);
      }
      break;
		       }
    case 100:
    case 101:
    {
    	FPTRACE(eLevelInfoNormal,"::DumpH221Stream : capabilities should  not appear here!!!");
    	DBGFPASSERT(1);
	}
    default:{

      FPTRACE(eLevelInfoNormal,"::DumpH221Stream : Unknown MSB!!!");
      DBGFPASSERT(1);

      break;
	    }
    } //switch(msb)

  }
  //msg <<"\n";
}




const char* CCDRUtils::GetQ931CauseAsString(int cause)
{
  switch (cause) {

    case 0: {
      return("default cause or not available (0)");
    }
    case 1: {
      return("unallocated number (1)");
    }
    case 2: {
      return("no route to specified transit net (2)");
    }
    case 3: {
      return("no route to destination (3)");
    }
    case 6: {
      return("channel unacceptable (6)");
    }
    case 16: {
      return("normal call clearing procedures (16)");
    }
    case 17: {
      return("called party was busy (17)");
    }
    case 18: {
      return("no user responding (18)");
    }
    case 19: {
      return("no answer from user (user alerted) (19)");
    }
    case 21: {
      return("call rejected (21)");
    }
    case 22: {
      return("number has been changed (22)");
    }
    case 26: {
      return("non-selected user clearing (26)");
    }
    case 27: {
      return("destination out of order (27)");
    }
    case 28: {
      return("invalid number format (28)");
    }
    case 29: {
      return("facility rejected (29)");
    }
    case 30: {
      return("response to status enquiry (30)");
    }
    case 31: {
      return("normal unspecified (31)");
    }
    case 34: {
      return("no circuit/channel available (34)");
    }
    case 38: {
      return("network out of order (38)");
    }
    case 41: {
      return("temporary failure (41)");
    }
    case 42: {
      return("switching equipment congestion (42)");
    }
    case 43: {
      return("access information discarded (43)");
    }
    case 44: {
      return("requested circuit/channel not available (44)");
    }
    case 45: {
      return("preempted (AT&T special defined) (45)");
    }
    case 47: {
      return("resources unavailable, unspecified (47)");
    }
    case 49: {
      return("quality of service unavailable (49)");
    }
    case 50: {
      return("requested facility not subscribed (50)");
    }
    case 52: {
      return("outgoing calls bared (52)");
    }
    case 54: {
      return("incoming calls bared (54)");
    }
    case 57: {
      return("bearer capability not authorized (57)");
    }
    case 58: {
      return("bearer capability not available (58)");
    }
    case 63: {
      return("service/option not available, unspecified (63)");
    }
    case 65: {
      return("bearer service not implemented (65)");
    }
    case 66: {
      return("channel type not implemented (66)");
    }
    case 69: {
      return("requested facility not implemented (69)");
    }
    case 70: {
      return("only restricted digital information bearer capability is available (70)");
    }
    case 79: {
      return("service/option not available, unspecified (79)");
    }
    case 81: {
      return("invalid call reference value (81)");
    }
    case 82: {
      return("identified channel does not exist (82)");
    }
    case 83: {
      return("a suspended call exists, burt this call identity does not (83)");
    }
    case 84: {
      return("call identity in use (84)");
    }
    case 85: {
      return("no call suspended (85)");
    }
    case 86: {
      return("call having the requested call identity has been cleared (86)");
    }
    case 88: {
      return("incompatible destination (88)");
    }
    case 91: {
      return("invalid transit network selection (91)");
    }
    case 95: {
      return("invalid message, unspecified (95)");
    }
    case 96: {
      return("mandatory information element missing (96)");
    }
    case 97: {
      return("message type is non-existent/not implemented (97)");
    }
    case 98: {
      return("message no good for the state of call (98)");
    }
    case 99: {
      return("information element non-existent or not implemented (99)");
    }
    case 100: {
      return("invalid information element contents (100)");
    }
    case 101: {
      return("message not compatible with call state (101)");
    }
    case 102: {
      return("recovery on timer expiration (102)");
    }
    case 111: {
      return("protocol error, unspecified (111)");
    }
    case 127: {
      return("interworking, unspecified (127)");
    }
    default: {
  	   return("UNKNOWN CAUSE");
    }

  } //end switch (cause)
}
