/*$Header: /MCMS/MAIN/include/H221.H 14    25/12/01 20:10 Ron $*/
/*========================================================================
                              H221.H
              Copyright 1995 Pictel Technologies Ltd.
                     All Rights Reserved.
--------------------------------------------------------------------------
   NOTE: This software contains valuable trade secrets and proprietary
   information of Pictel Technologies Ltd. and is protected by law.
   It may not be copied or distributed in any form or medium, disclosed
   to third parties, reverse engineered or used in any manner without
   prior written authorization from Pictel Technologies Ltd.
--------------------------------------------------------------------------
   FILE:
   SUBSYSTEM:
   PROGRAMMER:
--------------------------------------------------------------------------
   Who | Date       | Description
--------------------------------------------------------------------------
   ari | 13-2-96    |  modified consts
=========================================================================*/



#ifndef H221_H
#define H221_H
/* History:
   xx-xxx-94 DD: Created:
   12-Mar-95 JO: Added  define of Aggrgat_Esc.
*/




/* basic_bas_attribute  */

#define    Audio_Coding_Command   0
#define    Transfer_Rate_Command  1
#define    Video_Oth_Command      2
#define    Data_Command           3
#define    Terminal_Cap_1         4
#define    Terminal_Cap_2         5
#define    Escape_Code            7

#define   AUDCMDATTR      (0x0 << 5)           /*  cmd audio attribute value */
#define   XFERCMDATTR     (0x1 << 5)           /*  cmd xfer attribute value */
#define   OTHRCMDATTR     (0x2 << 5)           /*  cmd other attribute value */
#define   LSDMLPCMDATTR   (0x3 << 5)           /*  cmd lsd/mlp attribute value */
#define   HSDHMLPCMDATTR  (0x3 << 5)           /*  cmd hsd/hmpl attribute value */
/* #define   ESCAPEATTR      (0x7 << 5)*/           /* cmd escape */
#define   ESCHSDATTR      (0xF0)                 /*  Esc hsd value */




#define     AUDRATECAPATTR         (0x4 << 5)           /*  audio bit rate attribute value */
#define     DATAVIDCAPATTR         (0x5 << 5)           /*  data video attribute value */
#define     OTHERCAPATTR           (0x6 << 5)           /*  restrict attribute value */
#define     ESCAPECAPATTR          (0x7 << 5)           /*  escape attribute value   */
#define     HSDHMPLCAPATTR         (0x5 << 5)           /*  hsd hmlp attribute value - table A.2*/
#define     MLPCAPATTR             (0x6 << 5)           /*  mlp attribute value - table A.2     */
#define     H230CODE001            (0x1 << 5)


/* audio_command values */

#define    Au_Neutral        0
#define    Capex             1
#define    A_Law_OU          4
#define    U_Law_OU          5
#define    G722_m1           6
#define    Au_Off_U          7
#define    G723_1_Command    10
#define    G729_8k           11
#define    G7221_AnnexC_48k  13
#define    G7221_AnnexC_32k  14
#define    G7221_AnnexC_24k  15
#define    Au_Iso_256        16
#define    Au_Iso_384        17
#define    A_Law_OF          18
#define    U_Law_OF          19
#define    A_Law_48          20
#define    U_Law_48          21
#define    G722_m2           24
#define    G722_m3           25
#define    Au_40k            26
#define    Au_32k            27                      /* G.722.1/32 cmd opcode */
#define    Au_24k            28                      /* G.722.1/24 cmd opcode */
#define    G728              29
#define    Au_8k             30
#define    G719_32k          31                      /* G719 audio algorithm at 32 kbps*/
#define    G719_48k          32                      /* G719 audio algorithm at 48 kbps*/
#define    G719_64k          33                      /* G719 audio algorithm at 64 kbps*/
#define    G719S_64k         34                      /* G719 Stereo audio algorithm at 64 kbps*/
#define    G719S_96k         35                      /* G719 Stereo audio algorithm at 96 kbps*/
#define    G719S_128k        36                      /* G719 Stereo audio algorithm at 128 kbps*/
#define    Au_Off_F          37

/* audio command values of non standard commands */
#define    Au_Siren7_16k               Au_Off_F + 1  /* Siren7 audio algorithm at 16 kbps */
#define    Au_Siren7_24k               Au_Off_F + 2  /* Siren7 audio algorithm at 24 kbps */
#define    Au_Siren7_32k               Au_Off_F + 3  /* Siren7 audio algorithm at 32 kbps */
#define    Au_Siren14_24k              Au_Off_F + 4  /* Siren 14 audio algorithm at 24 kbps*/
#define    Au_Siren14_32k              Au_Off_F + 5  /* Siren 14 audio algorithm at 32 kbps*/
#define    Au_Siren14_48k              Au_Off_F + 6  /* Siren 14 audio algorithm at 48 kbps*/
#define    Au_G7221_16k                Au_Off_F + 7  /* G.722.1-16k (Used only in SIP/H323) */
#define    Au_Siren14S_48k             Au_Off_F + 8  /* Siren 14 Stereo audio algorithm at 48 kbps*/
#define    Au_Siren14S_56k             Au_Off_F + 9  /* Siren 14 Stereo audio algorithm at 56 kbps*/
#define    Au_Siren14S_64k             Au_Off_F + 10 /* Siren 14 Stereo audio algorithm at 64 kbps*/
#define    Au_Siren14S_96k             Au_Off_F + 11 /* Siren 14 Stereo audio algorithm at 96 kbps*/
#define    Au_Siren22_32k              Au_Off_F + 12 /* Siren 22 audio algorithm at 32 kbps*/
#define    Au_Siren22_48k              Au_Off_F + 13 /* Siren 22 audio algorithm at 48 kbps*/
#define    Au_Siren22_64k              Au_Off_F + 14 /* Siren 22 audio algorithm at 64 kbps*/
#define    Au_Siren22S_64k             Au_Off_F + 15 /* Siren 22 Stereo audio algorithm at 64 kbps*/
#define    Au_Siren22S_96k             Au_Off_F + 16 /* Siren 22 Stereo audio algorithm at 96 kbps*/
#define    Au_Siren22S_128k            Au_Off_F + 17 /* Siren 22 Stereo audio algorithm at 128 kbps*/
//#define    Au_Siren22S_128k Au_Off_F+17  /* Siren 22 Stereo audio algorithm at 128 kbps*/
#define    Au_SirenLPR_32k             Au_Off_F + 18 /* Siren LPR audio algorithm at 32 kbps*/
#define    Au_SirenLPR_48k             Au_Off_F + 19 /* Siren LPR audio algorithm at 48 kbps*/
#define    Au_SirenLPR_64k             Au_Off_F + 20 /* Siren LPR audio algorithm at 64 kbps*/
#define    Au_SirenLPRS_64k            Au_Off_F + 21 /* Siren LPR Stereo audio algorithm at 64 kbps*/
#define    Au_SirenLPRS_96k            Au_Off_F + 22 /* Siren LPR Stereo audio algorithm at 96 kbps*/
#define    Au_SirenLPRS_128k           Au_Off_F + 23 /* Siren LPR Stereo audio algorithm at 128 kbps*/
// TIP
#define    Au_AAC_LD                   Au_Off_F + 24 /* AAC-LD audio algorithm at 64 kbps*/
// LPR Scalable
#define    Au_SirenLPR_Scalable_32k    Au_Off_F + 25 /* Siren LPR Scalable audio algorithm at 32 kbps*/
#define    Au_SirenLPR_Scalable_48k    Au_Off_F + 26 /* Siren LPR Scalable audio algorithm at 48 kbps*/
#define    Au_SirenLPR_Scalable_64k    Au_Off_F + 27 /* Siren LPR Scalable audio algorithm at 64 kbps*/
#define    Au_SirenLPRS_Scalable_64k   Au_Off_F + 28 /* Siren LPR Stereo Scalable audio algorithm at 64 kbps*/
#define    Au_SirenLPRS_Scalable_96k   Au_Off_F + 29 /* Siren LPR Stereo Scalable audio algorithm at 96 kbps*/
#define    Au_SirenLPRS_Scalable_128k  Au_Off_F + 30 /* Siren LPR Stereo Scalable audio algorithm at 128 kbps*/
// iLBC
#define    Au_iLBC_13k                 Au_Off_F + 31 /* iLBC 13k*/
#define    Au_iLBC_15k                 Au_Off_F + 32 /* iLBC 15k*/
// opus
#define    Au_Opus_64k 	Au_Off_F+33 /* Opus_64k */
#define    Au_OpusStereo_128k Au_Off_F+34 /* Opus_Stereo 128k*/
// G722 Stereo
#define    Au_G722_Stereo_128          Au_Off_F + 35

//VTX
#define    G7222_0660		50		// Not supported by VTX100
#define    G7222_0885		51
#define    G7222_1265		52
#define    G7222_1425		53
#define    G7222_1585		54
#define    G7222_1825		55
#define    G7222_1985		56
#define    G7222_2305		57		// Not supported by VTX100
#define    G7222_2385		58		// Not supported by VTX100

/* xsfer_rate_command values */

#define    Xfer_64        0
#define    Xfer_2x64      1
#define    Xfer_3x64      2
#define    Xfer_4x64      3
#define    Xfer_5x64      4
#define    Xfer_6x64      5
#define    Xfer_384       6
#define    Xfer_2x384     7
#define    Xfer_3x384     8
#define    Xfer_4x384     9
#define    Xfer_5x384     10
#define    Xfer_1536      11
#define    Xfer_1920      12
#define    Xfer_128       13
#define    Xfer_192       14
#define    Xfer_256       15
#define    Xfer_320       16
#define    Loss_ic        17
#define    Channel_2      18
#define    Channel_3      19
#define    Channel_4      20
#define    Channel_5      21
#define    Channel_6      22
#define    Xfer_512       23
#define    Xfer_768       24
#define    Xfer_1152      26
#define    Xfer_1472      29
// Ip rates
#define    Xfer_96        32
#define    Xfer_1024      33
#define	   Xfer_4096	  34
#define	   Xfer_6144	  35
// New rates
#define	   Xfer_832		  36
#define	   Xfer_1728	  37
#define	   Xfer_2048	  38
#define	   Xfer_1280      39
#define	   Xfer_2560      40
#define	   Xfer_3072      41
#define	   Xfer_3584	  42
#define	   Xfer_8192      43

/* video_oth_command values */

#define    Video_Off      0
#define    H261           1
#define    H263           2
#define    Video_ISO      3
//#define    AV_ISO
#define    H264           4
#define    RTV			  5
#define    Encryp_On      6
#define    Encryp_Off     7
#define    SVC            8
#define    MS_SVC         9
#define    VP8            10 //N.A. DEBUG VP8
#define    Freeze_Pic     16
#define    Fast_Update    17
#define    Au_Loop        18
#define    Vid_Loop       19
#define    Dig_Loop       20
#define    Loop_Off       21
#define    SM_comp        23
#define    Not_SM_comp    24
#define    B6_H0_Comp     25
#define    Not_B6_H0      26
#define    Restrict       27
#define    Derestrict     28

/* data_command values */

#define    LSD_Off        0
#define    LSD_300        1
#define    LSD_1200       2
#define    LSD_4800       3
#define    LSD_6400       4
#define    LSD_8000       5
#define    LSD_9600       6
#define    LSD_14400      7
#define    LSD_16k        8
#define    LSD_24k        9
#define    LSD_32k        10
#define    LSD_40k        11
#define    LSD_48k        12
#define    LSD_56k        13
#define    LSD_62_4k      14
#define    LSD_64k        15
#define    MLP_Off        16
#define    MLP_4k         17
#define    MLP_6_4k       18
#define    Data_var_MLP   19
#define    Mlp_14_4       20
#define    DTI1R          21   /* in H221 Status */
#define    DTI2R          22   /* in H221 Status */
#define    DTI3R          23   /* in H221 Status */
#define    Mlp_22_4       21
#define    Mlp_30_4       22
#define    Mlp_38_4       23
#define    Mlp_46_4       24
#define    Mlp_16         25
#define    Mlp_24         26
#define    Mlp_32         27
#define    Mlp_40         28
#define    Mlp_62_4       29
#define    Mlp_64         30
#define    Data_Var_LSD   31

/* terminal_cap_1 values */

#define    Neutral        0
#define    A_Law          1
#define    U_Law          2
#define    G722_64        3
#define    G722_48        4
#define    Au_16k         5
#define    Au_Iso         6
#define    Sm_comp        7
#define    Xfer_Cap_128   8
#define    Xfer_Cap_192   9
#define    Xfer_Cap_256   10
#define    Xfer_Cap_320   11
#define    Xfer_Cap_512   12
#define    Xfer_Cap_768   13
#define    Xfer_Cap_1152  15
#define    Xfer_Cap_B     16
#define    Xfer_Cap_2B    17
#define    Xfer_Cap_3B    18
#define    Xfer_Cap_4B    19
#define    Xfer_Cap_5B    20
#define    Xfer_Cap_6B    21
#define    Xfer_Cap_Restrict    22
#define    Xfer_Cap_6B_H0_Comp  23
#define    Xfer_Cap_H0    24
#define    Xfer_Cap_2H0   25
#define    Xfer_Cap_3H0   26
#define    Xfer_Cap_4H0   27
#define    Xfer_Cap_5H0   28
#define    Xfer_Cap_1472  29
#define    Xfer_Cap_H11   30
#define    Xfer_Cap_H12   31

/* terminal_cap_2 values */

#define    Ter2_Var_Lsd              0
#define    Dxfer_Cap_300             1
#define    Dxfer_Cap_1200            2
#define    Dxfer_Cap_4800            3
#define    Dxfer_Cap_6400            4
#define    Dxfer_Cap_8000            5
#define    Dxfer_Cap_9600            6
#define    Dxfer_Cap_14400           7
#define    Dxfer_Cap_16k             8
#define    Dxfer_Cap_24k             9
#define    Dxfer_Cap_32k             10
#define    Dxfer_Cap_40k             11
#define    Dxfer_Cap_48k             12
#define    Dxfer_Cap_56k             13
#define    Dxfer_Cap_62_4k           14
#define    Dxfer_Cap_64k             15
#define    Dxfer_Cap_Mlp_4k          16
#define    Dxfer_Cap_Mlp_6_4k        17
#define    Var_Mlp                   18
#define    Mlp_Set_1                 19
#define    V_Qcif                    20
#define    V_Cif                     21
#define    V_1_29_97                 22
#define    V_2_29_97                 23
#define    V_3_29_97                 24
#define    V_4_29_97                 25
#define    H263_2000                 26
#define    Vid_Iso                   27
#define    Mlp_Set_2                 28
#define    Esc_Cf_R                  29
#define    Encryp_Cap                30
#define    Mbe_Cap                   31

/* other cap values */

#define   Restrict_L            0
#define   Restrict_P            1
#define   NoRestrict            2
#define   G723_1                3
#define   G729                  4
#define   G722_1_32             5  /* G.722.1/32 cap */
#define   G722_1_24             6  /* G.722.1/24 cap */
#define   G722_1_Annex_C_48     7  /* G.722.1 annex c 48k*/
#define   G722_1_Annex_C_32     8  /* G.722.1 annex c 32k*/
#define   G722_1_Annex_C_24     9  /* G.722.1 annex c 24k*/

/* h221 escape_codes */

#define    Aggrgat_Esc    15
#define    Hsd_Esc        16
#define    H230_Esc       17   /* table in h.230 + h243 for DCA_L,DCA_H*/
#define    Data_Apps_Esc  18   /* Table A.3 */
#define    H230_Sbe_Esc   19   /* see h243 for <M><T> */
#define    H230_T61_Esc   20   /* standard T.61 graphical characters */
#define    R_Sbe_1        21
#define    R_Sbe_2        22
#define    R_Sbe_3        23
#define    Cap_Mark       24
#define    Start_Mbe      25
#define    Ns_Cap         30
#define    Ns_Com         31

/* hsd_esc_bas_attribute */

#define HSD_ESC_COMMAND     3
#define HSD_ESC_CAPABILITIE 5

/* hsd_esc_capabilitie_values */

#define    Var_Hsd_Cap_R        1
#define    H_Mlp_Cap_62_4       2
#define    H_Mlp_Cap_64         3
#define    H_Mlp_Cap_128        4
#define    H_Mlp_Cap_192        5
#define    H_Mlp_Cap_256        6
#define    H_Mlp_Cap_320        7
#define    H_Mlp_Cap_384        8

#define    H_Mlp_Cap_14_4       12
#define    Var_H_Mlp_Cap_R      13
#define    Hxfer_Cap_64k        17
#define    Hxfer_Cap_128k       18
#define    Hxfer_Cap_192k       19
#define    Hxfer_Cap_256k       20
#define    Hxfer_Cap_320k       21
#define    Hxfer_Cap_384k       22
#define    Hxfer_Cap_512k_R     23
#define    Hxfer_Cap_768k_R     24
#define    Hxfer_Cap_1152k_R    25
#define    Hxfer_Cap_1536k_R    26

/* hsd_esc_command_values */

#define    Hsd_Com_Off         0
#define    Var_Hsd_Com_R       1
#define    H_Mlp_Com_62_4      2
#define    H_Mlp_Com_64        3
#define    H_Mlp_Com_128       4
#define    H_Mlp_Com_192       5
#define    H_Mlp_Com_256       6
#define    H_Mlp_Com_320       7
#define    H_Mlp_Com_384       8

#define    H_Mlp_Com_14_4      12
#define    Var_H_Mlp_Com_R     13
#define    H_Mlp_Off_Com       14
#define    Hxfer_Com_64k       17
#define    Hxfer_Com_128k      18
#define    Hxfer_Com_192k      19
#define    Hxfer_Com_256k      20
#define    Hxfer_Com_320k      21
#define    Hxfer_Com_384k      22
#define    Hxfer_Com_512k_R    23
#define    Hxfer_Com_768k_R    24
#define    Hxfer_Com_1152k_R   25
#define    Hxfer_Com_1536k_R   26

/* mlp_esc_capabilitie_values */

#define    Mlp_Cap_14_4         0
#define    Mlp_Cap_22_4         1
#define    Mlp_Cap_30_4         2
#define    Mlp_Cap_38_4         3
#define    Mlp_Cap_46_4         4
#define    Mlp_Cap_62_4         6
#define    Mlp_Cap_16           8
#define    Mlp_Cap_24           9
#define    Mlp_Cap_32          10
#define    Mlp_Cap_40          11
#define    Mlp_Cap_64          14

/* data_esc_bas_attribute */

#define DATA_ESC_COMMAND     3
#define DATA_ESC_CAPABILITIE 5

/* data_esc_command_values (010) */

#define    H224_MLP_Off        24
#define    H224_LSD_Off        25
#define    H224_HSD_Off        26

#define    T120_Off            28
#define    H224_Token_Off      30


/* data_esc_command_values (011) */

#define    Iso_Sp_On_Lsd        0
#define    Iso_Sp_On_Hsd        1
#define    Cursor_Data_Com_Lsd  10
#define    Fax_On_Lsd           16
#define    Fax_On_Hsd           17
#define    V120_Com_Lsd         20
#define    V120_Com_Hsd         21
#define    V14_Com_Lsd          22
#define    V14_Com_Hsd          23
#define    H224_MLP_On          24
#define    H224_LSD_On          25
#define    H224_HSD_On          26
#define    T120_ON              28

/* data_esc_capabilitie_values */

#define    Iso_Sp_Baseline_Lsd     0
#define    Iso_Sp_Baseline_Hsd     1
#define    Iso_Sp_Spatial          2
#define    Iso_Sp_Progressive      3
#define    Iso_Sp_Arithmetic       4
#define    Still_Image             9
#define    Cursor_Data_Cap_Lsd     10
#define    Group_3_Fax             16
#define    Group_4_Fax             17
#define    V120_Cap_Lsd            20
#define    V120_Cap_Hsd            21
#define    T120_Cap                28

/* h230_esc_bas_attributes  */

#define    H230_Code_000               0
#define    H230_Code_001               1
#define    H230_Code_010               2
#define    H230_Code_011               3
#define    H230_Code_101               5
#define    H230_Code_111_Forbiden      7


#define    Attr000   (H230_Code_000  << 5)
#define    Attr001   (H230_Code_001  << 5)
#define    Attr010   (H230_Code_010  << 5)
#define    Attr011   (H230_Code_011  << 5)
#define    Attr101   (H230_Code_101  << 5)


/* h230_code_000 values */

#define    Audio_Sym_Res0         0
#define    Audio_Sym_Res1         1
#define    AIM                    2
#define    AIA                    3
#define    ACE                    4
#define    ACZ                    5
#define    Audio_Sym_Res6         6
#define    Audio_Sym_Res7         7
#define    TCI                    8
#define    TII                    9
#define    TIS                    10
#define    VIS                    16
#define    VIA                    17
#define    VIA2                   18
#define    VIA3                   19
#define    Video_Sym_Res20        20
#define    Video_Sym_Res21        21
#define    Video_Sym_Res22        22
#define    Video_Sym_Res23        23
#define    Video_Sym_Res24        24
#define    Video_Sym_Res25        25
#define    Video_Sym_Res26        26
#define    Video_Sym_Res27        27
#define    Video_Sym_Res28        28
#define    Video_Sym_Res29        29
#define    Video_Sym_Res30        30
#define    VIR                    31

/* h230_code_001 values */

#define    MCC                    0
#define    Cancel_MCC             1
#define    MIZ                    2
#define    Cancel_MIZ             3
#define    MIS                    4
#define    Cancel_MIS             5
#define    MIM                    6
#define    TIC                    7
#define    TIX                    8
#define    RAN                    9
#define    MIH                    10
#define    TIA                    11
#define    TIN                    12
#define    TID                    13
#define    TCU                    14
#define    TCA                    15
#define    MCV                    16
#define    Cancel_MCV             17
#define    MIV                    18
#define    Cancel_MIV             19
#define    MCS                    20
#define    MCN                    21
#define    VIN                    22
#define    VCB                    23
#define    VCE                    24
#define    VCS                    25
#define    Cancel_VCS             26
#define    VCR                    27
#define    MMS                    28
#define    Cancel_MMS             29
#define    Cancel_MIM             30
#define    MIL                    31
#define    MIL                    31

/* h230_code_010  values */

#define    CIC            0
#define    CCD            1
#define    CIR            2
#define    CCK            3
#define    CCA            4
#define    CIT            5
#define    CCR            6
#define    CIS            7
#define    TIF            8
#define    TIE            9
#define    MVC            12 /* Multipoint Visualization Capability      (defined in H.243 ver.6) */
#define    MVA            13 /* Multipoint Visualization Achieved        (defined in H.243 ver.6) */
#define    MVR            14 /* Multipoint Visualization Refused/Revoked (defined in H.243 ver.6) */
#define    MIJ            15
#define    DCA_L          16
#define    DIT_L          17
#define    DCR_L          18
#define    DIS_L          19
#define    DCC_L          20
#define    DCA_H          24
#define    DIT_H          25
#define    DCR_H          26
#define    DIS_H          27
#define    DCC_H          28
#define    DCM            31

/* h230_code_011 values */

#define    TCS_0          0
#define    TCS_1          1
#define    TCS_2          2
#define    TCS_3          3
#define    TCS_4          17
#define    TCP_           4

/* h230_code_100 values */

/* h230_code_101 values */

#define    h239ControlCapability	1
#define    AMC_open					2
#define    AMC_close				3


/* Mbe opcodes */
/* 00000000 , 00000001 reserved */
/* 00001010 to 11011111 reserved*/
/* 11100000 to 11111111 forbidden */
#define    TIL            2
#define    IIS            3
#define    TIR            4
#define    TIP            5
#define    NIA            6
#define    NIAP           7
#define    Au_MAP         8
/* #define    Au_COM           13  print error in standart 12.94 h230 */
#define    H262_H263     10
#define    MRQ           12
#define    VideoNotDecodedMBs  15
#define    H264_Mbe            22

#define		h239ExtendedVideoCapability 23
#define		H239_messsage				24
#define		AMC_cap						25
#define		AMC_CI						26
#define		IEC14496_3Capability		27 //MPEG-4 audio

/* dca_l_command values without bas atribute */

#define    X300bs_Lsd        1
#define    X1200bs_Lsd       2
#define    X4800bs_Lsd       3
#define    X6400bs_Lsd       4
#define    X8000bs_Lsd       5
#define    X9600bs_Lsd       6
#define    X14400bs_Lsd      7
#define    X16kb_Lsd         8
#define    X24kb_Lsd         9
#define    X32kb_Lsd         10
#define    X40kb_Lsd         11
#define    X48kb_Lsd         12
#define    X56kb_Lsd         13
#define    X62_4kb_Lsd       14
#define    X64kb_Lsd         15
#define    Dcal_Var_Lsd      31
#define    Highest_Common_Rate_L   32
#define    Lowest_Common_Rate_L    33
#define    Current_Channel_Rate_L  34

/* all the values between 35-255 are reserved */

/* dca_h_command values without bas atribute  */

#define    Var_Hsd                   1
#define    XX64kb_Hsd               17
#define    XX128kb_Hsd              18
#define    XX192kb_Hsd              19
#define    XX256kb_Hsd              20
#define    XX320kb_Hsd              21
#define    XX384kb_Hsd              22
#define    XX512kb_Hsd              23
#define    XX768kb_Hsd              24
#define    XX1152kb_Hsd             25
#define    XX1536kb_Hsd             26

#define    Highest_Common_Rate_H    32
#define    Lowest_Common_Rate_H     33
#define    Current_Channel_Rate_H   34

 /* all the values between 35-255 are reserved */


/* AMSC-MUX64 values (Non-standard) */
#define    AMSC_0k			0
#define    AMSC_64k			1
#define    AMSC_128k		2
#define    AMSC_192k		3
#define    AMSC_256k		4
#define    AMSC_384k		5
#define    AMSC_512k		6
#define    AMSC_768k		7
#define    AMSC_1024k		8
#define    AMSC_1152k		9
#define    AMSC_1280k		10
#define    AMSC_1536k		11
#define    AMSC_2048k		12
#define    AMSC_2560k		13
#define    AMSC_3072k		14
#define    AMSC_4096k		15

/* AMC Cap values */
///option Byte 1
#define	   AMC_0k			0
#define    AMC_40k			1
#define    AMC_64k			2
#define    AMC_96k			3
#define    AMC_128k			4
#define    AMC_192k			5
#define    AMC_256k			6
#define    AMC_384k			7
//option Byte 2
#define    AMC_512k			8
#define    AMC_768k			9
#define    AMC_HSDCap		10
#define	   AMC_1024k		11	// not supported in H239 - Only for translation between EPC and H239
#define	   AMC_1152k		12	// not supported in H239 - Only for translation between EPC and H239
#define	   AMC_1280k		13	// not supported in H239 - Only for translation between EPC and H239
#define	   AMC_1536k		14  // not supported in H239 - Only for translation between EPC and H239
#define	   AMC_2048k		15  // not supported in H239 - Only for translation between EPC and H239
#define	   AMC_2560k		16  // not supported in H239 - Only for translation between EPC and H239
#define	   AMC_3072k		17  // not supported in H239 - Only for translation between EPC and H239
#define	   AMC_4096k		18  // not supported in H239 - Only for translation between EPC and H239

#define    MAX_AMC_RATE             AMC_4096k
#define	   MAX_AMC_FOR_1080P15_ONLY AMC_1536k
#define    MAX_AMC_FOR_1080P30_ONLY AMC_2048k

#define    MAX_PC_PROFILE   2

/* H.233 and H.234 P msgs Identifiers */
#define P0_Identifier	(0x80)
#define P1_Identifier	(0x81)
#define P2_Identifier	(0x82)
#define P3_Identifier	(0xA3)
#define P4_Identifier	(0x84)
#define P5_Identifier	(0xA5)
#define P6_Identifier	(0xA6)
#define P8_Identifier	(0xC0)
#define P9_Identifier	(0xC1)
#define P11_Identifier	(0xAB)
#define NULL_Identifier	(0xDF)

/*MUX->MCMS local msg Identifiers */
#define P3_First_Half_Identifier  (0xA7)
#define P3_Second_Half_Identifier (0xAB)

#endif


