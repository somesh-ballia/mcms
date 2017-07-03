// BackupRestoreManager.cpp

#include "BackupRestoreManager.h"

#include "Trace.h"
#include "StatusesGeneral.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <list>
#include <vector>
#include <sys/signal.h>
#include <time.h>
#include <errno.h>

#include "TraceStream.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "DummyEntry.h"
#include "StatusesGeneral.h"
#include "TaskApi.h"
#include "Request.h"
#include "SystemFunctions.h"
#include "FaultsDefines.h"
#include "Versions.h"
#include "ApacheDefines.h"
#include "HlogApi.h"
#include "SysConfigKeys.h"
#include "TerminalCommand.h"
#include "OsFileIF.h"
#include "McmsDaemonApi.h"
#include "OsTask.h"
#include "IncludePaths.h"
#include "AlarmStrTable.h"
#include "QA_ApiStructs.h"
#include "ConfigManagerApi.h"
#include "Blowfish.h"
#include "ApiStatuses.h"
#include "RestoreCfg.h"
#include "SslFunc.h"
#include "FipsMode.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "AuditorApi.h"

#define TAR_CREATE					"tar chz "
#define TAR_PIZZA_CREATE			"tar cz " // don't take pointed files by link, since IVR is a SymLink
#define TAR_EXTRACT					"tar xzv "
#define OUT_ERR_REDIRECTION 		" 1>/dev/null 2>/dev/null"

//Backup is linked to MCU_DATA_DIR/backup
#define BACKUP_DIR				  	"Backup/"
//Restore is linked to MCU_DATA_DIR/restore
#define RESTORE_DIR					"Restore/"

//MajorVersion, MinorVersion, ReleaseVerstion, InternalVersion
//date(dd-mm-yy),time(hour-min-sec).bck
#define BACKUP_FILE_PREFIX			"Backup_%d-%d-%d-%d"//"Backup_%d-%d_%d-%d-%d_%d-%d-%d.bck"
#define BACKUP_FILE_SUFFIX			".bck"

#define VERSION_FILE				"Cfg/backup_data.txt"
#define CFG_DIR						" Cfg "
#define EMACFG_DIR					" EMACfg "
#define KEYS_DIR					" Keys "
#define MFW_CFG						"/mcu_custom_config/"
#define CERT_CREAT_IND_FILE			((std::string)(MCU_MCMS_DIR+"/Keys/cert_valid_fail.ind"))

// file flag to sign restore configuration was performed before startup
#define RESTORE_CONFIG_FLAG_FILE 			 "States/restore_config.flg"
#define RESTORE_CONFIG_SUCCESS				 "States/restore_config_succeeded.flg"
#define	RESTORE_CONFIG_FAILURE_TAR_TZ  "States/restore_config_failed_tar_tz.flg" // print
#define	RESTORE_CONFIG_FAILURE_TAR_XZ  "States/restore_config_failed_tar_xz.flg" // extract
#define	RESTORE_CONFIG_FAILURE_LS			 "States/restore_config_failed_ls.flg" // file doesn't exist
#define	RESTORE_CUSTOM_CONFIG_FILE_PATH			 "mcu_custom_config/custom.cfg"

PBEGIN_MESSAGE_MAP(CBackupRestoreManager)
  ONEVENT(XML_REQUEST,		 IDLE,     CBackupRestoreManager::HandlePostRequest )
  ONEVENT(BACKUP_TIMEOUT,    ANYCASE,  CBackupRestoreManager::OnBackupTimeout )
  ONEVENT(RESTORE_TIMEOUT,   ANYCASE,  CBackupRestoreManager::OnRestoreTimeout )
  ONEVENT(MCUMNGR_TO_BACKUPRESTORE_SYS_VERSION_IND, ANYCASE, CBackupRestoreManager::OnGetMcuVersionInd )
  ONEVENT(BACKUP_START, 	ANYCASE,   CBackupRestoreManager::OnBackUpStart )
  ONEVENT(INSTALLER_TO_BACKUP_RESTORE_START_IND,  ANYCASE, CBackupRestoreManager::OnInstallStartInd )
  ONEVENT(INSTALLER_TO_BACKUP_RESTORE_FINISH_IND, ANYCASE, CBackupRestoreManager::OnInstallFinishInd )
  ONEVENT(BACKUP_IDLE_TIMER,					  ANYCASE, CBackupRestoreManager::OnBackupIdleTimer)
  ONEVENT(INSTALLER_PROGRESS_TIMER, 			  ANYCASE, CBackupRestoreManager::OnBackupRestoreProgressTimeout)
PEND_MESSAGE_MAP(CBackupRestoreManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CBackupRestoreManager)
	ON_TRANS("TRANS_MCU",	"BACKUP_CONFIG_START",		CDummyEntry,	CBackupRestoreManager::HandleBackUpStart)
	ON_TRANS("TRANS_MCU",	"BACKUP_CONFIG_FINISH",		CDummyEntry,	CBackupRestoreManager::HandleBackUpFinish)
	ON_TRANS("TRANS_MCU",	"RESTORE_CONFIG_START",		CDummyEntry,	CBackupRestoreManager::HandleRestoreStart)
	ON_TRANS("TRANS_MCU",	"RESTORE_CONFIG_FINISH",	CRestoreCfg,	CBackupRestoreManager::HandleRestoreFinish)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CBackupRestoreManager)
	ONCOMMAND("enc_dec", CBackupRestoreManager::HandleTerminalEncryptionDecryption,"test encryption and decryption")
	ONCOMMAND("decrypt", CBackupRestoreManager::HandleTerminalDecrypt, "decrypt a file")
END_TERMINAL_COMMANDS

extern void BackupRestoreMonitorEntryPoint(void* appParam);

void BackupRestoreManagerEntryPoint(void* appParam)
{
	CBackupRestoreManager * pBackupRestoreManager = new CBackupRestoreManager;
	pBackupRestoreManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CBackupRestoreManager::GetMonitorEntryPoint()
{
	return BackupRestoreMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CBackupRestoreManager::CBackupRestoreManager()
{
	m_processAction = eBRIdle;
	m_installInProgress = false;
	m_strBackupFileName = "";
	memset(&m_mcuVer, 0, sizeof(m_mcuVer));
}

////////////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::ManagerStartupActionsPoint()
{
    TestAndEnterFipsMode();

    AskMcuMngrForSysVersion();

	//check for restore succeeded/failed file
	string dump ="Restore configuration succeeded";
	DWORD code = 0;

	if(IsFileExists(RESTORE_CONFIG_SUCCESS))
	{
		code = AA_RESTORE_SUCCEEDED;
//		DeleteFile(RESTORE_CONFIG_SUCCESS);		//VNGR-13707 - we will delete this file after we update the switch with the restore management
	}
	else if(IsFileExists(RESTORE_CONFIG_FAILURE_TAR_TZ))
	{
		code = AA_RESTORE_FAILED;
		dump = "Restore configuration failed: Failed to print restore file content.";
		DeleteFile(RESTORE_CONFIG_FAILURE_TAR_TZ);
	}
	else if(IsFileExists(RESTORE_CONFIG_FAILURE_TAR_XZ))
	{
		code = AA_RESTORE_FAILED;
		dump = "Restore configuration failed: Failed to untar restore file.";
		DeleteFile(RESTORE_CONFIG_FAILURE_TAR_XZ);
	}
	else if(IsFileExists(RESTORE_CONFIG_FAILURE_LS))
	{
		code = AA_RESTORE_FAILED;
		dump = "Restore configuration failed: File or directory doesn't exist.";
		DeleteFile(RESTORE_CONFIG_FAILURE_LS);
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::ManagerStartupActionsPoint"
							   << "\nNo Restore flag file was found. <Normal case>";

	}

	if(0 != code)
	{
		TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::ManagerStartupActionsPoint"
		   					   << "\n" << dump.c_str();

		if(AA_RESTORE_FAILED == code)
		{
			AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
							code,
							MAJOR_ERROR_LEVEL,
							dump.c_str(),
							true,
							true
						);
		}

		//Audit should be added
	}
}


//////////////////////////////////////////////////////////////////////
BYTE CBackupRestoreManager::IsJitcMode()
{
	BYTE bJitcMode = FALSE;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

	return bJitcMode;
}

////////////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::AskMcuMngrForSysVersion()
{
	TRACESTR(eLevelInfoNormal) << "\nBackupRestoreManager::AskMcuMngrForSysVersion";
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pMcuMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr, eManager);

	STATUS res = pMcuMngrMbx->Send(pRetParam, BACKUPRESTORE_TO_MCUMNGR_SYS_VERSION_REQ);
}

//////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::OnInstallStartInd()
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::OnInstallStartInd";
	m_installInProgress = true;

	StartTimer(INSTALLER_PROGRESS_TIMER, (TIMEOUT_INSTALATION+10*SECOND));
}

//////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::OnInstallFinishInd()
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::OnInstallFinishInd";
	m_installInProgress = false;
}

void CBackupRestoreManager::OnBackupIdleTimer()
{
	SendProgressTypeToMcuMngr(BACKUP_INPROGRESS_REQ, eBackup_Idle);
}
////////////////////////////////////////////////////////////////////////////
STATUS CBackupRestoreManager::OnBackupRestoreProgressTimeout()
{
	m_installInProgress = false;
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CBackupRestoreManager::FinishAction(OPCODE actionOP, const char * dump,
										   BYTE stateType,  char * dirToRemove)
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::FinishAction"
	                       << "\n" << dump << ", errno = " << errno;

	//enable further backup/restore operations
	SetProcessAction(eBRIdle);
	SendProgressTypeToMcuMngr(actionOP, stateType);
	SendMsgToInstaller(BACKUP_RESTORE_TO_INSTALLER_FINISH_IND);

	// timer to set backup failure to idle
	if(BACKUP_INPROGRESS_REQ == actionOP)
	{
		StartTimer(BACKUP_IDLE_TIMER, 10*SECOND);
	}

	//remove dirToRemove content
	if(NULL != dirToRemove)
	{
		//delete backup file
		string del = "rm -f ";
			   del += dirToRemove;
			   del += "/";
			   del += "*";
			   //del += OUT_ERR_REDIRECTION;

	   string output_string;
	   STATUS status = SystemPipedCommand(del.c_str(), output_string);
       if(STATUS_OK != status)
       {
    	   TRACESTR(eLevelInfoNormal) << "\nFailed to execute command: " << del.c_str();
       }
	}

	CConfigManagerApi api;
    // disable R/W access
	api.RemountVersionPartition(FALSE);

	/*
	DWORD errCode = AA_BACKUP_FAILED;
	if(RESTORE_INPROGRESS_REQ == actionOP)
	{
		errCode = AA_RESTORE_FAILED;
	}

	AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
					errCode,
					MAJOR_ERROR_LEVEL,
					dump,
					true,
					true
				);
	*/

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::OnGetMcuVersionInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nBackupRestoreManager::OnGetMcuVersionInd";

	pSeg->Get((BYTE*)&m_mcuVer, sizeof(m_mcuVer));
}

/////////////////////////////////////////////////////////////////////////////
STATUS CBackupRestoreManager::SendProgressTypeToMcuMngr(OPCODE action, BYTE type)
{
	//Send to set McuState to restoreInProgress
	CSegment*  pMsg = new CSegment();
    *pMsg << type;

    const COsQueue* pMcuMngrMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr,eManager);
	STATUS res = pMcuMngrMbx->Send(pMsg, action);

	return res;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CBackupRestoreManager::SendMsgToInstaller(OPCODE action, BYTE type)
{
	//Send to notify installer that backup/restore starts/ends
	CSegment*  pMsg = new CSegment();

	if(action != eBRIdle)
	{
    	*pMsg << type;
	}

    const COsQueue* pInstallerMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessInstaller,eManager);
	STATUS res = pInstallerMbx->Send(pMsg, action);

	return res;
}


/////////////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::GetTime(char * timeStr, int timeLen, char * format)
{
	time_t curTime;
	time(&curTime);

	struct tm* lt = localtime(&curTime);
	if(lt)
		strftime(timeStr, timeLen-1, format, lt);
	else
		TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::GetTime is lt NULL!!";
}

/////////////////////////////////////////////////////////////////////////////
// By value, instead of creating new object into body
bool CBackupRestoreManager::CreateBackupVersionFile()
{
	errno = 0;

	ofstream os;
    os.open(VERSION_FILE);

    if(!os.good())
    {
    	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::CreateBackupVersionFile"
    	                       << "\nVersion file: " << VERSION_FILE << " cannot be opened for writing"
    	                       << ", errno = " << errno;
    	return false;
    }

    char dateStr[128];
    GetTime(dateStr, sizeof(dateStr));

    os << "Date:  " <<  dateStr << "  \n"
       << "Version:  major: " << m_mcuVer.ver_major << ", minor: " << m_mcuVer.ver_minor
       << ", release: " << m_mcuVer.ver_release << ", internal: " << m_mcuVer.ver_internal << "\n";

    os.close();

    return true;
}

/*
 *  Backup_%d-%d-%d-%d _%d-%d-%d_%d-%d-%d.bck
 */
/////////////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::CreateBackupName(char * name, int len)
{
	sprintf(name, BACKUP_FILE_PREFIX, m_mcuVer.ver_major, m_mcuVer.ver_minor, m_mcuVer.ver_release, m_mcuVer.ver_internal);

	strcat(name, "_");

	int versionLen = strlen(name);
	GetTime(name+versionLen, len-versionLen);

	strcat(name, BACKUP_FILE_SUFFIX);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CBackupRestoreManager::HandleBackUpStart(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleBackUpStart";

	// Send to set backup state for success (for not being monitored wrongl, by EMA)
	//SendProgressTypeToMcuMngr(BACKUP_INPROGRESS_REQ, eBackup_Success);

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	short action = GetProcessAction();
	if(m_installInProgress || (eBRIdle != action))
	{
		TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleBackUpStart"
		                       << "\n" << strBackupRestoreAction[action] << " already in progress";

		STATUS set = STATUS_BACKUP_IN_PROGRESS;
		if(eRestore == action)
		{
			set = STATUS_RESTORE_IN_PROGRESS;
		}
		else if (m_installInProgress )
		{
			set = STATUS_INSTALLATION_IN_PROGRESS;
		}

		pRequest->SetStatus(set);

		return STATUS_OK;
	}

	//Send to set McuState to restoreInProgress
	SendProgressTypeToMcuMngr(BACKUP_INPROGRESS_REQ, eBackup_InProgress);

	SendMsgToInstaller(BACKUP_RESTORE_TO_INSTALLER_START_IND, eBackup);

	SetProcessAction(eBackup);

    const COsQueue* pBackupRestoreMbx
    	= CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessBackupRestore,eManager);
	STATUS res = pBackupRestoreMbx->Send(NULL, BACKUP_START);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CBackupRestoreManager::OnBackUpStart()
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::OnBackUpStart";
	errno = 0;

	StartTimer( BACKUP_TIMEOUT, BACKUP_RESTORE_TIMEOUT );

	//create version file
	CreateBackupVersionFile();

	// Create backup file name
	char fileName[256];
	memset(fileName, '\0', sizeof(fileName));
	CreateBackupName(fileName, sizeof(fileName));

	CConfigManagerApi api;
    // enable R/W access (for adding&getting the backup file)
    api.RemountVersionPartition(TRUE);

	m_strBackupFileName = BACKUP_DIR;
	m_strBackupFileName += fileName;

	// Prepare encrypted tar file in a certain place, use encryption
	string tarCmd = TAR_CREATE;

	// prepare file name with current dateTime+version
	tarCmd += CFG_DIR;
	tarCmd += KEYS_DIR;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if(IsTarget() || eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)  // Fix BRIDGE-2440
	{
		tarCmd += EMACFG_DIR;
	}
	else
	{
		if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
		{
			string output;
			string touchCmd = "touch ";
			touchCmd += CERT_CREAT_IND_FILE;
			SystemPipedCommand(touchCmd.c_str(), output);
		}

	}
        if(eProductTypeSoftMCUMfw == curProductType)
        {
        	    TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::OnBackUpStart : backup custom.cfg";
        	    tarCmd += " ";
                tarCmd += MFW_CFG;
                tarCmd += " ";
        }

	//tarCmd += " ";
	//tarCmd += VERSION_FILE; Version file is already added to Tar, since it's created on Cfg dir
	//tarCmd += " | ";
	//tarCmd += OPENSSL;
	//tarCmd += " -e"; //Encryption
	tarCmd += " > ";
	tarCmd += m_strBackupFileName;
//	tarCmd += " -f ";
//	tarCmd += backupFile;
//	tarCmd += OUT_ERR_REDIRECTION;

	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::OnBackUpStart"
	                       << "\nExecute command: " << tarCmd.c_str() << endl;

	string output_string;
	STATUS status = SystemPipedCommand(tarCmd.c_str(), output_string);

	//delete version file
    string del = "rm -f ";
    del += VERSION_FILE;
    SystemPipedCommand(del.c_str(), output_string);

    std::string cmd;
    std::string answer;
    cmd = "sync";
    SystemPipedCommand(cmd.c_str(), answer, TRUE, TRUE, FALSE);

    //VNGR-10668
	status = EncryptBackupFile(m_strBackupFileName.c_str());

    if(STATUS_OK != status)
    {
        string message = "Failed to execute system call : ";
        message += tarCmd;

        return FinishAction(BACKUP_INPROGRESS_REQ, message.c_str(), eBackup_FailureTar, BACKUP_DIR);
    }

    SendProgressTypeToMcuMngr(BACKUP_INPROGRESS_REQ, eBackup_Success);

	// return done
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CBackupRestoreManager::HandleBackUpFinish(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleBackUpFinish";

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	errno = 0;

	short action = GetProcessAction();
	if(eBackup != action)
	{
		TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleBackUpFinish"
							   << strBackupRestoreAction[action] << " in progress...";

		STATUS set = STATUS_RESTORE_IN_PROGRESS;
		if(eBRIdle == action)
		{
			set = STATUS_ILLEGAL;
		}

		pRequest->SetStatus(set);

		return STATUS_OK;
	}



	//Delete timer
	DeleteTimer(BACKUP_TIMEOUT);

	TRACESTR(eLevelInfoNormal) << "\nRemove Backup file";

	//delete backup folder
	string del = "rm -f ";
		   del += BACKUP_DIR;
		   del += "/";
		   del += "*";
		   //del += OUT_ERR_REDIRECTION;

	system(del.c_str());

    CConfigManagerApi api;
    // disable R/W access
    api.RemountVersionPartition(FALSE);

    //enable further backup/restore operations
	SetProcessAction(eBRIdle);

	SendProgressTypeToMcuMngr(BACKUP_INPROGRESS_REQ, eBackup_Idle);

	SendMsgToInstaller(BACKUP_RESTORE_TO_INSTALLER_FINISH_IND);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CBackupRestoreManager::EncryptBackupFile(const char * fileName)
{
	const int iChunkSize = 32;
	unsigned char outSHA256[iChunkSize];
	memset(outSHA256, '\0', iChunkSize);
	unsigned char firstChunk[iChunkSize];
	// Create backup file name
	CSslFunctions::SHA256_Encryption(fileName, outSHA256);

	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::XXXXBackupFile BackupFileName="
						   << fileName;

	FILE* f = fopen(fileName,"rb+");

	if(f == NULL)
	{
        FPASSERTMSG(1,"CBackupRestoreManager::XXXXBackupFile: Backup file failed - file not found");
    	return STATUS_FAIL;
	}

	int length = 0;
	length = fread(firstChunk, 1, iChunkSize, f); //maybe should be fread(firstChunk, iChunkSize, 1, f)

	if(length != iChunkSize)
	{
        FPASSERTMSG(1,"CBackupRestoreManager::XXXXBackupFile: Backup file failed length != iChunkSize");
    	fclose(f);
        return STATUS_FAIL;
	}

	//Seek to beginning
	if(fseek(f, 0L, SEEK_SET) == -1)
	{
        FPASSERTMSG(1,"CBackupRestoreManager::XXXXBackupFile: Backup file failed, fseek 1 failed");
    	fclose(f);
        return STATUS_FAIL;
	}

	//write the SHA32 to first 32 chars
	if(fwrite(outSHA256, 1, iChunkSize, f) == 0)
	{
        FPASSERTMSG(1,"CBackupRestoreManager::XXXXBackupFile: Backup file failed, fwrite 1 failed");
    	fclose(f);
        return STATUS_FAIL;
	}
	//Seek to end (append) the firstChunk
	if(fseek(f, 0L, SEEK_END) == -1)
	{
        FPASSERTMSG(1,"CBackupRestoreManager::XXXXBackupFile: Backup file failed, fseek 2 failed");
    	fclose(f);
        return STATUS_FAIL;
	}

	//append
	if(fwrite(firstChunk, 1, iChunkSize, f) == 0)
	{
        FPASSERTMSG(1,"CBackupRestoreManager::XXXXBackupFile: Backup file failed, fwrite 2 failed");
    	fclose(f);
        return STATUS_FAIL;
	}

	fclose(f);

	return STATUS_OK;
}

STATUS CBackupRestoreManager::DecryptBackupFile(const char* fileName,bool isSHA256Dec)
{
  FILE* f = fopen(fileName, "rb+");
  PASSERTSTREAM_AND_RETURN_VALUE(f == NULL,
      "fopen: " << fileName << ": " << strerror(errno) << " (" << errno << ")",
      STATUS_FAIL);

  const int iChunkSize = 32;
  unsigned char fileSHA256[iChunkSize + 1];
  fileSHA256[iChunkSize] = '\0';
  if (fread(fileSHA256, 1, iChunkSize, f) == 0)
  {
    PASSERTSTREAM(true,
        "fread: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // Seek to end - iChunkSize
  if (fseek(f, -iChunkSize, SEEK_END) == -1)
  {
    PASSERTSTREAM(true,
        "fseek: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // copy the end of the file
  unsigned char firstChunk[iChunkSize];
  if (fread(firstChunk, 1, iChunkSize, f) == 0)
  {
    PASSERTSTREAM(true,
        "fread: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // Seek to beginning
  if (fseek(f, 0L, SEEK_SET) == -1)
  {
    PASSERTSTREAM(true,
        "fseek: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // write the firstChucnk back to it's original place
  if (fwrite(firstChunk, 1, iChunkSize, f) == 0)
  {
    PASSERTSTREAM(true,
        "fwrite: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // close the file - we are done with it
  fclose(f);

  // cut the file
  struct stat st;
  stat(fileName, &st);
  if (truncate(fileName, st.st_size - iChunkSize) != 0)
  {
    PASSERTSTREAM(true,
        "truncate: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    return STATUS_FAIL;
  }

  // compare the SHA1 values
  unsigned char calculatedSHA256[iChunkSize + 1];
  calculatedSHA256[iChunkSize] = '\0';

  CSslFunctions::SHA256_Encryption(fileName, calculatedSHA256);
  for (int i = 0; i < iChunkSize; i++)
  {
   /* PASSERTSTREAM_AND_RETURN_VALUE(calculatedSHA256[i] != fileSHA256[i],
      "File " << fileName << " has been modified outside the server, chunk #" << i,
      STATUS_FAIL);*/
	  if(calculatedSHA256[i] != fileSHA256[i])
	  {
		  return STATUS_FAIL;
	  }
  }

  TRACEINTOFUNC << "Backup file " << fileName << " was verified";

  return STATUS_OK;
}

STATUS CBackupRestoreManager::DecryptBackupFileSha1(const char* fileName)
{
  FILE* f = fopen(fileName, "rb+");
  PASSERTSTREAM_AND_RETURN_VALUE(f == NULL,
      "fopen: " << fileName << ": " << strerror(errno) << " (" << errno << ")",
      STATUS_FAIL);

  const int iChunkSize = 20;
  unsigned char fileSHA1[iChunkSize + 1];
  fileSHA1[iChunkSize] = '\0';
  if (fread(fileSHA1, 1, iChunkSize, f) == 0)
  {
    PASSERTSTREAM(true,
        "fread: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // Seek to end - iChunkSize
  if (fseek(f, -iChunkSize, SEEK_END) == -1)
  {
    PASSERTSTREAM(true,
        "fseek: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // copy the end of the file
  unsigned char firstChunk[iChunkSize];
  if (fread(firstChunk, 1, iChunkSize, f) == 0)
  {
    PASSERTSTREAM(true,
        "fread: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // Seek to beginning
  if (fseek(f, 0L, SEEK_SET) == -1)
  {
    PASSERTSTREAM(true,
        "fseek: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // write the firstChucnk back to it's original place
  if (fwrite(firstChunk, 1, iChunkSize, f) == 0)
  {
    PASSERTSTREAM(true,
        "fwrite: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    fclose(f);
    return STATUS_FAIL;
  }

  // close the file - we are done with it
  fclose(f);

  // cut the file
  struct stat st;
  stat(fileName, &st);
  if (truncate(fileName, st.st_size - iChunkSize) != 0)
  {
    PASSERTSTREAM(true,
        "truncate: " << fileName << ": " << strerror(errno) << " (" << errno << ")");
    return STATUS_FAIL;
  }

  // compare the SHA1 values
  unsigned char calculatedSHA1[iChunkSize + 1];
  calculatedSHA1[iChunkSize] = '\0';

  CSslFunctions::SHA1_Encryption(fileName, calculatedSHA1);
  for (int i = 0; i < iChunkSize; i++)
  {
    PASSERTSTREAM_AND_RETURN_VALUE(calculatedSHA1[i] != fileSHA1[i],
      "File " << fileName << " has been modified outside the server: "
      << fileSHA1[i] << " != " << calculatedSHA1[i] << ", chunk #" << i,
      STATUS_FAIL);
  }

  TRACEINTOFUNC << "Backup file " << fileName << " was verified";

  return STATUS_OK;
}

STATUS CBackupRestoreManager::HandleRestoreStart(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleRestoreStart";

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	errno = 0;

	// if back to factory defaults was done, and system has not been yet reset,
	// disallow restore operation
	if(IsFileExists(DEFAULT_RESTORE_FILE))
	{
		TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleRestoreStart"
							   << "\nBack to factory defaults has been performed without reset.";
		pRequest->SetStatus(STATUS_BACK_TO_FACTORY_DEFAULTS_IN_PROGRESS);
		return STATUS_OK;
	}

	short action = GetProcessAction();
	if(m_installInProgress || (eBRIdle != action))
	{
		TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleRestoreStart"
							   << "\n" << strBackupRestoreAction[action] << " already in progress";

		STATUS set = STATUS_BACKUP_IN_PROGRESS;
		if(eRestore == action)
		{
			set = STATUS_RESTORE_IN_PROGRESS;
		}
		else if (m_installInProgress)
		{
			set = STATUS_INSTALLATION_IN_PROGRESS;
		}
		pRequest->SetStatus(set);
		return STATUS_OK;
	}

	SendMsgToInstaller(BACKUP_RESTORE_TO_INSTALLER_START_IND, eRestore);

	SetProcessAction(eRestore);

	SendProgressTypeToMcuMngr(RESTORE_INPROGRESS_REQ, eRestore_InProgress);

	StartTimer( RESTORE_TIMEOUT, BACKUP_RESTORE_TIMEOUT );

	CConfigManagerApi api;
    // enable R/W access (for putting&retrieving the restore file)
    api.RemountVersionPartition(TRUE);

	//return OK, folder mounted
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CBackupRestoreManager::HandleRestoreFinish(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleRestoreFinish";

	pRequest->SetConfirmObject(new CDummyEntry);
	if (pRequest->GetAuthorization() != SUPER )
	{
	    FPTRACE(eLevelInfoNormal,"CBackupRestoreManager::HandleRestoreFinish: No permission to restore configuration");
	    pRequest->SetStatus(STATUS_NO_PERMISSION);
	    return STATUS_NO_PERMISSION;
	}
	pRequest->SetStatus(STATUS_OK);

	errno = 0;

	short action = GetProcessAction();
	if(eRestore != action)
	{
		TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleRestoreFinish: "
							   << strBackupRestoreAction[action] << " in progress...";

		STATUS set = STATUS_BACKUP_IN_PROGRESS;
		if(eBRIdle == action)
		{
			set = STATUS_ILLEGAL;
		}
		pRequest->SetStatus(set);

		return STATUS_OK;
	}

	// Was moved to HandleRestoreStart for preventing incorrect McuState
	//SendProgressTypeToMcuMngr(RESTORE_INPROGRESS_REQ, eRestore_InProgress);

	CRestoreCfg* fileName = (CRestoreCfg*)pRequest->GetRequestObject();

	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleRestoreFinish: "
						  << "Restore file name arrived = " << fileName->GetFileName() << endl;

	string restoreFile = RESTORE_DIR;
		   restoreFile += fileName->GetFileName();

    STATUS status = STATUS_OK;

    string restoreFileSha1 = RESTORE_DIR;
    restoreFileSha1 += fileName->GetFileName();
    restoreFileSha1 += ".copy";
    string cp = "cp ";
    cp += restoreFile.c_str();//
    cp += " ";
    cp += restoreFileSha1.c_str();

    string output_str;
    STATUS statusCopy = SystemPipedCommand(cp.c_str(), output_str);
    if(STATUS_OK != statusCopy)
    {
    	string message = "Failed to execute system call : ";
    	message += cp;
    	pRequest->SetStatus(status);
    	return FinishAction(RESTORE_INPROGRESS_REQ, message.c_str(), eRestore_FailureUntar, RESTORE_DIR);
    }

    //Decrypt here
    STATUS statusSha256 = DecryptBackupFile (restoreFile.c_str());
	if(statusSha256 != STATUS_OK && IsJitcMode() == FALSE)
		status = DecryptBackupFileSha1(restoreFileSha1.c_str());
	if(status !=  STATUS_OK)
	{

		DeleteTimer(RESTORE_TIMEOUT);

		CConfigManagerApi api;
	    // disable W access
	    api.RemountVersionPartition(FALSE);

	    pRequest->SetStatus(status);

        string message = "Failed to check the file before untar";
		return FinishAction(RESTORE_INPROGRESS_REQ, message.c_str(), eRestore_FailureUntar, RESTORE_DIR);
	}


    //VNGR-10830
	//delete the old IVR, MeetingRooms, Profiles, RecordLink, Reservations, Templates
	string del;
	del = "rm -Rf "+MCU_MCMS_DIR+"/Cfg/IVR/* "+MCU_MCMS_DIR+"/Cfg/AudibleAlarms/* "+MCU_MCMS_DIR+"/Cfg/MeetingRooms/* "+MCU_MCMS_DIR+"/Cfg/Profiles/* "+MCU_MCMS_DIR+"/Cfg/RecordLink/* "+MCU_MCMS_DIR+"/Cfg/Reservations/* "+MCU_MCMS_DIR+"/Cfg/Templates/*";

	string output_string;
	status = SystemPipedCommand(del.c_str(), output_string);
	if(STATUS_OK != status)
	{
		TRACESTR(eLevelInfoNormal) << "\nFailed to execute command: " << del.c_str();
	}


	DeleteTimer(RESTORE_TIMEOUT);

	CConfigManagerApi api;
    // disable W access
    api.RemountVersionPartition(FALSE);

	//extract configuration to validate tar checksum
	string  untarCmd = "cat ";
	if(statusSha256 == STATUS_OK)
		untarCmd += restoreFile;
	else
		untarCmd += restoreFileSha1;
			//untarCmd += " | ";
			//untarCmd += OPENSSL;
			//untarCmd += " -d ";
			untarCmd += " | ";
			untarCmd += TAR_EXTRACT;

	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::HandleRestoreFinish"
	                       << "\nExecute command: " << untarCmd << endl;

	status = SystemPipedCommand(untarCmd.c_str(), output_string);
    if(STATUS_OK != status)
    {
        string message = "Failed to execute system call : ";
        message += untarCmd;

        pRequest->SetStatus(status);

        return FinishAction(RESTORE_INPROGRESS_REQ, message.c_str(), eRestore_FailureUntar, RESTORE_DIR);
    }

    // indicate by a file, that restore was performed
    // so on startup, the system notices and replaces configuration
    CreateFile(RESTORE_CONFIG_FLAG_FILE);

	SendProgressTypeToMcuMngr(RESTORE_INPROGRESS_REQ, eRestore_Success);

	//SetProcessAction(eBRIdle);

	AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
					AA_RESTORE_SUCCEEDED,
					MAJOR_ERROR_LEVEL,
					"Restore operation completed. Reset the MCU",
					true,
					true
					);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::OnBackupTimeout()
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::OnBackupTimeout";
	SendAuditEvent("Backup operation failed",
				   "Backup operation Timed Out");

	FinishAction(BACKUP_INPROGRESS_REQ, "Backup timed out", eBackup_FailureTimeout, BACKUP_DIR);
}

//////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::OnRestoreTimeout()
{
	TRACESTR(eLevelInfoNormal) << "\nCBackupRestoreManager::OnRestoreTimeout";
	FinishAction(RESTORE_INPROGRESS_REQ, "Restore timed out", eRestore_FailureTimeout, RESTORE_DIR);
}

STATUS CBackupRestoreManager::HandleTerminalEncryptionDecryption(CTerminalCommand & command, std::ostream& answer)
{
	string strFileName = MCU_MCMS_DIR+"/Restore/enc_dec_test.txt";
	string strFileNameEnc = MCU_MCMS_DIR+"/Restore/enc_dec_test.txt_enc";
	string strFileNameDec = MCU_MCMS_DIR+"/Restore/enc_dec_test.txt_dec";
	struct stat st;
	FILE* f = NULL;

	CConfigManagerApi api;
    // enable R/W access (for adding&getting the backup file)
    api.RemountVersionPartition(TRUE);

    f = fopen(strFileName.c_str(), "wb+");

	if(f == NULL)
	{
        FPASSERTMSG(1,"CBackupRestoreManager::HandleTerminalEncryptionDecryption: test file could not be created or opened");
        api.RemountVersionPartition(FALSE);
    	return STATUS_FAIL;
	}

	//a dummy file creation
	fputs("It is not advisable to mix calls to output functions from the stdio library with low - level calls  to  write()  for  the  file "
       "descriptor associated with the same output stream; the results will be undefined and very probably not what you want", f);

	fclose(f);



	CopyFile(strFileName.c_str(), strFileNameEnc.c_str());

	EncryptBackupFile(strFileNameEnc.c_str());

	CopyFile(strFileNameEnc.c_str(), strFileNameDec.c_str());

	DecryptBackupFile(strFileNameDec.c_str());

	//compare files
	long sizeBefore =0 , sizeAfter=0;
	stat(strFileName.c_str(), &st);
	sizeBefore = st.st_size;
	stat(strFileNameDec.c_str(), &st);
	sizeAfter = st.st_size;

	if(sizeAfter != sizeBefore)
		answer << "EncryptionDecryption: files sizes are different";
	else
		answer << "EncryptionDecryption: files sizes are OK";

    api.RemountVersionPartition(FALSE);
	return STATUS_OK;
}

STATUS CBackupRestoreManager::HandleTerminalDecrypt(CTerminalCommand& cmd,
                                                    std::ostream& ans)
{
  std::string fname = cmd.GetToken(eCmdParam1);
  if (0 == fname.compare("Invalide Token"))
  {
      ans << "Unable to get fname\n";
      return STATUS_OK;
  }

  STATUS stat = DecryptBackupFile(fname.c_str());

  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN_VALUE(NULL == proc, STATUS_OK);

  ans << proc->GetStatusAsString(stat);

  return STATUS_OK;
}

STATUS CBackupRestoreManager::CopyFile(const char * fileNameSource, const char * fileNameDestination)
{
    FILE* f_from;		/* stream of source file. */
    FILE* f_to;			/* stream of target file. */

    /* open the source and the target files. */
	f_from = fopen(fileNameSource, "r");
	if (!f_from) {
		return STATUS_FAIL;
	}
	f_to = fopen(fileNameDestination, "w+");
	if (!f_to) {
		fclose(f_from);
		return STATUS_FAIL;
	}

	/* copy source file to target file, line by line. */
	int currentChar;
	while ((currentChar = fgetc(f_from)) != EOF) {
		if (fputc(currentChar, f_to) == EOF) {  /* error writing data */
			fclose(f_from);
			fclose(f_to);
			return STATUS_FAIL;
		}
	}

	fclose(f_from);
	fclose(f_to);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CBackupRestoreManager::SendAuditEvent(const std::string strEvent, const std::string strDescription) const
{
	//Audit
	AUDIT_EVENT_HEADER_S outAuditHdr;
	CAuditorApi::PrepareAuditHeader(outAuditHdr,
									"",
									eMcms,
									"",
									"",
									eAuditEventTypeInternal,
									eAuditEventStatusOk,
									strEvent,
									strDescription,
									"",
									"");
	CFreeData freeData;
	CAuditorApi::PrepareFreeData(freeData,
								 "",
								 eFreeDataTypeXml,
								 "",
								 "",
								 eFreeDataTypeXml,
								 "");
	CAuditorApi api;
	api.SendEventMcms(outAuditHdr, freeData);
}
