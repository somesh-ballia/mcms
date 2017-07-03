#ifndef _DBG_CFG_H
#define _DBG_CFG_H


#define FILENAME_dbgCfg             "/mnt/mfa_cm_fs/dbgCfg.txt"
#define FILENAME_defaultLogFile     "/output/diagnostic/DiagLog.txt"
#define FILENAME_defaultPrevLogFile "/output/diagnostic/DiagLogPrev.txt"
#define PATHNAME_defaultLogPath     "/output/diagnostic/"
#define MAX_DBG_FILE_PATH_LEN       100
#define DBG_FILE_BUFFER_SIZE		2000


typedef enum
{
	MFA_DBG__IGNORE_DBG_CFG_FILE,
	MFA_DBG__RELEASE_DSP,
	MFA_DBG__PRINT_CM_INITIATOR,
	
	MFA_DBG__PRINT_DIAG,
	MFA_DBG__PRINT_DISPATCH,
	MFA_DBG__PRINT_E2PROM,
	MFA_DBG__PRINT_IPMI,
	MFA_DBG__PRINT_SHELFCOM,
	MFA_DBG__PRINT_LAN_SWITCH,
	MFA_DBG__PRINT_MCMS_COM,
	MFA_DBG__PRINT_NTP,
	MFA_DBG__PRINT_SHARED,
	MFA_DBG__PRINT_STARTUP,
	MFA_DBG__PRINT_TCP_SERVER,
	MFA_DBG__PRINT_USB,
	MFA_DBG__PRINT_TIMER,
	
	MFA_DBG__OUTPUT_DEST,
	MFA_DBG__LOGFILE_NAME,
	MFA_DBG__MAX_ENTRIES
	
}	MFA_DBG_ENTRY_ENUM;



typedef enum
{
	eTERMINAL,
	eLOG_FILE,
	eTCP_LOGGER,
	eSPECIFIED_TERMINAL

}	MFA_DBG_OUTPUT_OPTIONS;





typedef struct
{
	BOOL	bIgnoreDbgCfgFile;		//ignore dbg configuration file, use defaults instead
	BOOL 	bReleaseDsp;			// release DSP after load - YES / NO
	UINT32	ul_OutputDest;			// where to print to - TERMINAL / FILE
	FILE	*logFileHandle;			// if printing to file - file handle
	FILE	*DiagTerminalFileHandle;
	FILE	*terminalFileHandle;
	INT8	ac_LogFileName[MAX_DBG_FILE_PATH_LEN];	// if printing to file - name of file
	INT8	ac_PrevLogFileName[MAX_DBG_FILE_PATH_LEN];	// if printing to file - name of file
	
}	MFA_DBG_S;





extern void ParseMfaDbgCfg();
extern void InitMfaDbgInfo(INT8 *pInfoBuff);
extern void DumpDbgParams();


#endif

