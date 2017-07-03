#ifndef MFA_BOARD_H
#define MFA_BOARD_H

#define FiRST_LOAD_SYNCH_VALUE1 0xDEADB00B
#define FiRST_LOAD_SYNCH_VALUE2 0xBADFACE5

#define UBOOT_EMB_FALSE 0
#define UBOOT_EMB_TRUE 1
#define MAX_UPGRADE_RETRYS 3

#define ERROR_BUF_LEN 200
#define VERSION_NAME_LEN 100


/*.R.H - env settings */
#define MAX_ETH_ADDRS		2
typedef unsigned char EthAddr[6];
typedef unsigned char IpAddr[4] ;

/* This Structure is not word aligned !!! */
/*
typedef struct mfa_env
{
	EthAddr ethAddrs[MAX_ETH_ADDRS];   
	IpAddr   loaclip ;				   
	IpAddr   subNetMask ;
	IpAddr   defGateWay ;
	IpAddr  remoteServerIp ;
	char    userPas[40] ;
	char   kernelBootType ;
	char   Loading;
	char   LoadingEnded;
	char   RunningStarted;
	char   Status;
	unsigned int   RetryCounter;	// should stay aligned
	unsigned int   FirstLoadSyncValue1;
	unsigned int   FirstLoadSyncValue2;	
	char   CurrentVersion[VERSION_NAME_LEN];
	char   Seperator1[4];
	char   PreviousVersion[VERSION_NAME_LEN];
	char   Seperator2[4];
   	char   ErrorReportBuf[ERROR_BUF_LEN];
	char   Seperator3[4];
	char   VlanId[8];
	char   Seperator4[4];
	char   UbootVerName[32];
	char   Seperator5[4];
	unsigned int	diagFlags;		// Proxy(P.K): Flags filed for indication of Diagnostics & UBOOT POST test support
	unsigned int 	diagClockStat;	// Proxy(P.K): Diagnostic clock test status (autorun by UBOOT in diag mode)
	unsigned int 	diagClockVal;	// Proxy(P.K): Diagnostic clock test value in case of error (autorun by UBOOT in diag mode)
}MENV ;
*/

typedef struct mfa_env
{
	EthAddr ethAddrs[MAX_ETH_ADDRS];   
	IpAddr   loaclip ;				   
	IpAddr   subNetMask ;
	IpAddr   defGateWay ;
	IpAddr  remoteServerIp ;
	char    userPas[40] ;
	char   kernelBootType ;
	char   Loading;
	char   LoadingEnded;
	char   RunningStarted;
	char   Status;
	unsigned int   RetryCounter;	// should stay aligned
	unsigned int   FirstLoadSyncValue1;
	unsigned int   FirstLoadSyncValue2;	
	char   CurrentVersion[VERSION_NAME_LEN];	
	char   Seperator1[4];
	char   PreviousVersion[VERSION_NAME_LEN];
	char   Seperator2[4];
   	char   ErrorReportBuf[ERROR_BUF_LEN];
    char   Seperator3[4];
	char   VlanId[8];
	char   Seperator4[4];
	char   UbootVerName[32];
	char   Seperator5[4];
	unsigned int	diagFlags;		// Proxy(P.K): Flags filed for indication of Diagnostics & UBOOT POST test support
	unsigned int	diagFlagAllowPost;	//this should be manually set to some value
	unsigned int 	diagClockStat;	// Proxy(P.K): Diagnostic clock test status (autorun by UBOOT in diag mode)
	unsigned int 	diagClockVal;	// Proxy(P.K): Diagnostic clock test value in case of error (autorun by UBOOT in diag mode)
}MENV ;


// typedef struct mfa_eeprom_common_header
// {
// 	char chFormatVersion ;
// 	char chInternalUseOffset ;
// 	char chChassisInfoOffset ;
// 	char chBoardAreaOffset ;
// 	char chProductInfoOffset ;
// 	char chMultiRecordOffset ;
// 	char chPad ;
// 	char chCRC ;
// }Mfa_Eeprom_CH ;
// 
// #define EEPROM_DATA  200 
// #define HEX_FORMAT   0xAA
// #define DEC_FORMAT   0xDD
// #define SLOT_NUM_EEPROM_OFFSET  0x2000
// #define UKNOWN_BOOT_TYPE  0xFE
// #define MAX_EEPROM_BYTES_PER_READ 0x16

// extern unsigned int mfa_atoi(char *str,unsigned int asciiLength,unsigned int intFormat) ;


extern INT32 FlashOpenDev (INT8 *DevName,INT32 flags);
extern INT32 FlashReadDev(INT32 fd,void *buf,INT32 count);
extern INT32 FlashWriteDev(INT32 fd,void *buf,INT32 count);
extern void  FlashCleanupDev (INT32 fd);
extern INT32 FlashEraseDev(INT32 Fd, INT32 start, INT32 count, INT32 unlock);
extern INT32 HandleSystemFlags_MFA(INT8 cLoading, INT8 cLoadingEnded, INT8 *pPrevFileName); 
extern INT32 TestFlashReadWrite();
extern INT32 InitFdPtr (INT32 fd,char *devname, INT32 offset);
extern void DumpConfigParamsArea(MENV *mfaEnv);

#endif

