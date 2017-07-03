
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <sstream>
#include "DiagnosticsInfo.h"
#include "IpmcInt.h"




#define	MAX_LINE	1024
#define	CNTL_USB_DIAG_TEST_FILE	"/tmp/cntl_usb_test"
#define	CNTL_USB_DIAG_TEST_DIR1	"/tmp/cntl_usb_dir1"
#define	CNTL_USB_DIAG_TEST_DIR2	"/tmp/cntl_usb_dir2"

pthread_mutex_t dgnsTestMutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * In this function,i am adding test to current TestSession (diagCurrentSession),
 * It will be linked list of tests to do.
 */

int dgnsTestAddTest(int testId,int unitOnSlot)
{
	dgnsCtrlTestInfo *newTest;

	newTest = (dgnsCtrlTestInfo*)malloc(sizeof(dgnsCtrlTestInfo));
	if (newTest)
	{

		//newTest->testStartTime = now;
		newTest->testId = testId;
		newTest->loopsDone = 0;
		newTest->successTests = 0;
		newTest->failTests = 0;
		newTest->isActiveNow = 0;
		newTest->unitOnSlot = unitOnSlot;
		newTest->duration = 0;
		newTest->nextTest = 0;

		pthread_mutex_lock (&dgnsTestMutex);

		if (diagCurrentSession.firstTest == 0)
		{
			//first test
			diagCurrentSession.firstTest = newTest;
			diagCurrentSession.lastTest = newTest;
		}
		else
		{
			diagCurrentSession.lastTest->nextTest = newTest;
			diagCurrentSession.lastTest = newTest;
		}
		pthread_mutex_unlock(&dgnsTestMutex);
		return 1;
	}
	else
		return -1; //couldn't allocate new test from some reason

}

/* Erasing entire test list*/
int dgnsClearTests()
{
	dgnsCtrlTestInfo *delTest;
	pthread_mutex_lock (&dgnsTestMutex);
	while (diagCurrentSession.firstTest)
	{
		delTest = diagCurrentSession.firstTest->nextTest;
		free(diagCurrentSession.firstTest);
		diagCurrentSession.firstTest = delTest;
	}
	diagCurrentSession.lastTest = 0; //it will be deleted in previous loop anyway
	pthread_mutex_unlock(&dgnsTestMutex);
	return 1;
}

/*This function receives index of test to find,and returns pointer to
 * test struct,or NULL, if no such index
 * */
int dgnsGetTestFromList(int testIndex,dgnsCtrlTestInfo **findTest)
{
	int i;
	//no tests!
	if (diagCurrentSession.firstTest == 0 )
		return -1;
	pthread_mutex_lock (&dgnsTestMutex);
	*findTest = diagCurrentSession.firstTest;

	for (i=0 ; i<testIndex; i++)
		if (*findTest)
			*findTest = (*findTest)->nextTest;

	pthread_mutex_unlock(&dgnsTestMutex);
	 //testid -> string test name.
	if (*findTest == 0) //didnt find our test
		return -1;
	else
		return 1;

}

dgnsTestInfo * testIdToTest(UINT32 TestId,dgnsTestInfo currentSystemTests[])
{
	int i=0;

	while (currentSystemTests[i].TestId != 0xffff)
	{
		if (currentSystemTests[i].TestId == TestId)
			break;
		i++;
	}
	//it will return test's name,or "Empty",if not found
	return &(currentSystemTests[i]);
}

/*
 * CNTL Hard Disk check implementation
 */
void CNTLHardDiskSmartCheck(dgnsTestResult *diagTestRes)
{
	fprintf(stderr,"CNTLHardDiskSmartCheck::begin\n");
	
	std::stringstream command, trace;
	std::string answer;

	//Check if HD is mounted on /dev/hdb (PATA)
	int stat = SystemPipedCommand("mount | grep /output | awk '{ print $1 }'", answer);
	if(0 != stat)
	{
		SLEEP(2000);
		stat = SystemPipedCommand("mount | grep /output | awk '{ print $1 }'", answer);
		if(0 != stat)
		{
			diagTestRes->testResult = eStatFail;
			strcpy(diagTestRes->errString,"CNTLHardDiskSmartCheck:SystemPipedCommand can not mount .");
			return;
		}
	}


	if(answer.find("/dev/hdb")!= string::npos)
	{
		command <<"smartctl -q errorsonly /dev/hdb";
	}
	else
	{
		command <<"smartctl -q errorsonly -d ata /dev/sda";
	}
	
	fprintf(stderr,"CNTLHardDiskSmartCheck::, command = %s\n",(command.str()).c_str());
	
	stat = SystemPipedCommand((command.str()).c_str(),answer);

	if(0 != stat)
	{
		SLEEP(2000);
		stat = SystemPipedCommand((command.str()).c_str(),answer);
		if(0 != stat)
		{
			diagTestRes->testResult = eStatFail;
			strcpy(diagTestRes->errString,"CNTLHardDiskSmartCheck:SystemPipedCommand can not run command .");
			return;
		}
	}

   if (answer.size() > 0 )
    {
		diagTestRes->testResult = eStatFail;
		strncpy(diagTestRes->errString,answer.c_str(),sizeof(diagTestRes->errString)-1);
		diagTestRes->errString[sizeof(diagTestRes->errString)-1] = '\0';
    }
    else if(0 != stat)
    {
	 	diagTestRes->testResult = eStatFail;
		strcpy(diagTestRes->errString,"smartctl can not check the hard disk.");
    }
	else
	{
		diagTestRes->testResult = eStatOk;
	}
		
	fprintf(stderr,"smart check,error string - %s\n",answer.c_str());
	return;
}

int SystemPipedCommand(const string & system_command, string & output_string)
{
    cout << "SystemPipedCommand" << endl;

	int STATUS_OK = 0;
	int STATUS_FAIL = -1;
	
    int ret = STATUS_OK;
  
    output_string = "";
    char line[256];
    FILE *fpipe = NULL;

    if ( !(fpipe = (FILE*)popen(system_command.c_str(),"r")) )
    {

            return  STATUS_FAIL;
    }


        cout << "fopen OK" << endl;

        int i = 0;
        while ( fgets( line, sizeof line, fpipe))
        {
            cout << "fgets OK : " << i << endl;
            output_string += line;

            i++;
        }
        
        // Does not return pclose status because some system command fail wait4
        	// man: The pclose function waits for the associated process to terminate and
        	// returns the exit status of the command as returned by wait4
        	// Does not check the return status because some system command fail wait4
        int exit_value = pclose(fpipe);
        ret = (-1 == exit_value
               ?
               STATUS_FAIL : exit_value);
        
        if (STATUS_OK != ret)
           cout << "pclose( exit_value == " << exit_value <<  endl;

    return STATUS_OK;
}

void CNTLHardDiskReadWriteCheck(dgnsTestResult *diagTestRes)
{
	unsigned char ch = 0x55;	//Pattern: 01010101
	char * szFileName = "/output/tempFile519";
	char * szMsgInfo  = "hard disk";
	ReadWriteTemplate(ch,szFileName,szMsgInfo,diagTestRes);
	return;
}

void CNTLUSBReadWriteCheck(dgnsTestResult *diagTestRes)
{
	//SystemPipedCommand("fdisk -l | grep 'Disk /dev/sd[bcdefghijklmnopqrstuvwxyz]' |awk '{ print $2 }' ",answer);
	char szLine[MAX_LINE];
	char szCommand[200];

	list<string> lstUSBDev;
	list<string> lstDevName;
	string szUSBDev[2];

	SLEEP(2000);

	//find USB disk	
	//sprintf(szCommand,"ls /dev/ |grep sd.1 > %s",CNTL_USB_DIAG_TEST_FILE);
	sprintf(szCommand,"ls /dev/ | grep -v usb_stick |grep sd > %s",CNTL_USB_DIAG_TEST_FILE);
	system(szCommand);

	FILE * fp;
	fp = fopen(CNTL_USB_DIAG_TEST_FILE,"r");

	if(NULL == fp)
	{
	 	diagTestRes->testResult = eStatFail;
		sprintf(diagTestRes->errString, "Open %s file error!\n", CNTL_USB_DIAG_TEST_FILE);
		fprintf(stderr,"CNTLUSBReadWriteCheck:Open %s file error!\n",CNTL_USB_DIAG_TEST_FILE);
		return;
	}

	//this file only contains max number of line is 2.
 	while (fgets(szLine, MAX_LINE, fp) != NULL)
	{
		fprintf(stderr,"%s",szLine);
		string szTemp = szLine;

		string::size_type nPosition = szTemp.find("\n");
		if(string::npos != nPosition)
		{
			szTemp.erase(nPosition,1);
		}
		
		lstDevName.push_back(szTemp);
	}
	fclose(fp);

	//remove hard disk
	std::string answer;
	int stat = SystemPipedCommand("df | grep '/output' | awk -F ' ' '{ print $1;}'  | cut -c 6-9 | tr -d '\r\n'", answer);

	if(0 != stat)
	{
		SLEEP(2000);
		 stat = SystemPipedCommand("df | grep '/output' | awk -F ' ' '{ print $1;}'  | cut -c 6-9 | tr -d '\r\n'", answer);
		if(0 != stat)
		{
			diagTestRes->testResult = eStatFail;
			strcpy(diagTestRes->errString,"CNTLUSBReadWriteCheck:SystemPipedCommand can not get the hard disk.");
			return;
		}
	}
	fprintf(stderr,"Get hard disk name %s\n",answer.c_str());

	string szMatch = answer;
	string szDevName;
	if(FindDevName(szMatch,lstDevName,szDevName))
	{
		DelDevName(szDevName,lstDevName);
	}

	//remove CF card
	stat = SystemPipedCommand("df | grep '/data' | awk -F ' ' '{ print $1;}'  | cut -c 6-8 | tr -d '\r\n'", answer);

	if(0 != stat)
	{
		SLEEP(2000);
		stat = SystemPipedCommand("df | grep '/data' | awk -F ' ' '{ print $1;}'  | cut -c 6-8 | tr -d '\r\n'", answer);

		if(0 != stat)
		{
			diagTestRes->testResult = eStatFail;
			strcpy(diagTestRes->errString,"CNTLUSBReadWriteCheck:SystemPipedCommand can not get CF card.");
			return;
		}
	}
	fprintf(stderr,"Get CF card name %s\n",answer.c_str());

	szMatch = answer;
	if(FindDevName(szMatch,lstDevName,szDevName))
	{
		DelDevName(szDevName,lstDevName);
	}
	
	szMatch = answer + "1";
	if(FindDevName(szMatch,lstDevName,szDevName))
	{
		DelDevName(szDevName,lstDevName);
	}

	szMatch = answer + "2";
	if(FindDevName(szMatch,lstDevName,szDevName))
	{
		DelDevName(szDevName,lstDevName);
	}

	szMatch = answer + "3";
	if(FindDevName(szMatch,lstDevName,szDevName))
	{
		DelDevName(szDevName,lstDevName);
	}
	

	szMatch = "1"; //sd.1
	while(FindDevName(szMatch,lstDevName,szDevName))
	{
		DelDevName(szDevName,lstDevName);
		
		string szTemp = szDevName.substr(0,3);
		DelDevName(szTemp,lstDevName);
		
		lstUSBDev.push_back(szDevName);
	}

	//combine two string
	list<string>::iterator itName;
	for(itName = lstDevName.begin();itName!= lstDevName.end();itName++)
	{
		lstUSBDev.push_back(*itName);
	}

	for(itName = lstUSBDev.begin();itName!= lstUSBDev.end();itName++)
	{
		fprintf(stderr,"CNTLUSBReadWriteCheck:USB device Name = %s\n", (*itName).c_str());
	}
	
	int iCounter = lstUSBDev.size();

	if(0 == iCounter)
	{
	 	diagTestRes->testResult = eStatFail;
		strcpy(diagTestRes->errString,"Can not find USB disk.");
		fprintf(stderr,"CNTLUSBReadWriteCheck:Can not find USB disk\n");
		return;
	}

	int nResult1,nResult2;

	if(1 == iCounter)
	{
		diagTestRes->testResult = eStatFail;
		strcpy(diagTestRes->errString,"This test case need to insert two USB keys before performing this test.");
		fprintf(stderr,"CNTLUSBReadWriteCheck:This test case need to insert two USB keys before performing this test.\n");
	}
	else if(2 == iCounter)
	{
		list<string>::iterator it= lstUSBDev.begin();
		for(int i =0 ;i<2;i++)
		{
			szUSBDev[i] = *it;
			it++;
		}
		string szDir1 = "/tmp/usb_stick_";
		szDir1 += szUSBDev[0];

		string szDir2 = "/tmp/usb_stick_";
		szDir2 += szUSBDev[1];
		
		nResult1 = USBUnitTest(szUSBDev[0],szDir1,diagTestRes);	
		nResult2 = USBUnitTest(szUSBDev[1],szDir2,diagTestRes);
		//successful
		if((0 == nResult1)&&(0 == nResult2))
		{
			diagTestRes->testResult = eStatOk;
			fprintf(stderr,"CNTLUSBReadWriteCheck:USB check pass.\n");			
		}
		else
		{
			diagTestRes->testResult = eStatFail;
			strcpy(diagTestRes->errString,"The test of USB failed.");
			fprintf(stderr,"CNTLUSBReadWriteCheck:The test of USB failed..\n");
		}
	}
	else
	{
		diagTestRes->testResult = eStatFail;
		strcpy(diagTestRes->errString,"There are more that 2 USB keys.");
		fprintf(stderr,"CNTLUSBReadWriteCheck:There are more that 2 USB keys..\n");
	}
	
	return;
}

bool FindDevName(const string szMatch,const list<string> lstDevName,string &szDevName)
{
	//fprintf(stderr,"Enter FindDevName()!\n");		
	
	bool bFind = false;
	for(list<string>::const_iterator itName = lstDevName.begin();itName!= lstDevName.end();itName++)
	{
		string::size_type nPosition = itName->find(szMatch);
		//fprintf(stderr,"FindDevName(), nPosition = %d!\n",nPosition);		
		
		if(string::npos != nPosition)
		{
			bFind = true;
			szDevName = *itName;
			break;
		}
	}

	return bFind;
}

bool DelDevName(const string szDevName,list<string> &lstDevName)
{
	bool bFind = false;
	for(list<string>::iterator itName = lstDevName.begin();itName!= lstDevName.end();itName++)
	{
		string::size_type nPosition = itName->find(szDevName);				
		if(string::npos != nPosition)
		{
			lstDevName.remove(*itName);
			bFind = true;
			break;
		}
	}

	return bFind;
}


//result 0 ---- sucessful
//       1 ---- USB failed
//       2 ---- mount failed
int USBUnitTest(string szUSBDev,string szMountDir,dgnsTestResult *diagTestRes)
{
	char szCommand[200];
	//string answer;

	//try to make directory
	snprintf(szCommand, sizeof(szCommand), "test -d %s || mkdir %s",szMountDir.c_str(),szMountDir.c_str());
	system(szCommand);

	//umount USB directory
	//sprintf(szCommand,"umount %s",szMountDir.c_str());
	//system(szCommand);

	//mount USB directory
	//sprintf(szCommand,"mount /dev/%s %s",szUSBDev.c_str(),szMountDir.c_str());
	//fprintf(stderr,"USBUnitTest:mount /dev/%s,%s\n",szUSBDev.c_str(),szMountDir.c_str());
	//SystemPipedCommand(szCommand,answer);
	//fprintf(stderr,"USBUnitTest:mount result = %s\n",answer.c_str());
	
	//if("" == answer)
	//{
		unsigned char ch = 0x55;	//Pattern: 01010101
		char szFileNameUSB[100];
		snprintf(szFileNameUSB, sizeof(szFileNameUSB), "%s/tempFile519",szMountDir.c_str());
		char * szMsgInfo  = "USB port";
		int nResult = ReadWriteTemplate(ch,szFileNameUSB,szMsgInfo,diagTestRes);

		//umount USB directory
		//sprintf(szCommand,"umount %s",szMountDir.c_str());
		//system(szCommand);
		
		if(0 == nResult)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	//}
	//else
	//{
	//	return 2;
	//}
}
	
int ReadWriteTemplate(unsigned char ch,char *szFileName,char * szMsgInfo,dgnsTestResult *diagTestRes)
{
	const int bufLength = 1024;
	char * buff = (char *)malloc(bufLength);

	if(buff)
		memset(buff,ch,bufLength);
	else
	{
		printf("Malloc failed for the buffer\n");
		snprintf(diagTestRes->errString, sizeof(diagTestRes->errString),
				"Malloc failed for the buffer\n");
		diagTestRes->testResult = eStatFail;
		return -1;
		
	}
	
	FILE * fp = fopen(szFileName, "wb");
	if (fp == NULL)
	{
		printf("Could not create file on %s, errors in %s\n",szMsgInfo,szMsgInfo);
		snprintf(diagTestRes->errString, sizeof(diagTestRes->errString),
				"Could not create file on %s, errors in %s\n",szMsgInfo,szMsgInfo);
		diagTestRes->testResult = eStatFail;
		free(buff);
		
		return -1;
	}

	int result = fwrite(buff, sizeof(char), bufLength, fp);
	if (result != bufLength)
	{
		printf("Short write into %s, only %d Byte write, errors in %s\n",szMsgInfo,result,szMsgInfo);
		snprintf(diagTestRes->errString, sizeof(diagTestRes->errString),
				"Short write into %s, only %d Byte write, errors in %s\n", szMsgInfo,result,szMsgInfo);
		diagTestRes->testResult = eStatFail;
		fclose(fp);
		free(buff);
		
		return -1;
	}
	fclose(fp);
	
	fp = fopen(szFileName, "r");
	if (fp == NULL)
	{
		printf("Could not open file on %s, errors in %s\n",szMsgInfo,szMsgInfo);
		snprintf(diagTestRes->errString, sizeof(diagTestRes->errString),
				"Could not open file on %s, errors in %s\n",szMsgInfo,szMsgInfo);
		diagTestRes->testResult = eStatFail;
		free(buff);
		
		return -1;
	}
	
	memset(buff,0,bufLength);
	result = fread(buff, sizeof(char), bufLength, fp);
	if (result != bufLength)
	{
		printf("Short read into %s, only %d Byte write, errors in %s\n",szMsgInfo,result,szMsgInfo);
		snprintf(diagTestRes->errString, sizeof(diagTestRes->errString),
				"Short read into %s, only %d Byte write, errors in %s\n",szMsgInfo,result,szMsgInfo);
		diagTestRes->testResult = eStatFail;
		fclose(fp);	
		free(buff);
		
		return -1;
	}
	
	for (int i = 0; i< bufLength; i++)
	{
		if (buff[i] != ch)
		{
			printf("read values error into %s, unexpected char %c, errors in %s\n",szMsgInfo,buff[i],szMsgInfo);
			snprintf(diagTestRes->errString, sizeof(diagTestRes->errString),
					"read values error into %s, unexpected char %c, errors in %s\n",szMsgInfo,buff[i],szMsgInfo);
			diagTestRes->testResult = eStatFail;
			fclose(fp);	
			free(buff);
			
			return -1;
		}
	}
	
	if (0!=fclose(fp))
	{
		printf("error when closing file\n");
		snprintf(diagTestRes->errString, sizeof(diagTestRes->errString), "error when closing file\n");
		diagTestRes->testResult = eStatFail;
		free(buff);
		
		return -1;
	}
	
	char szTemp[100];
	snprintf(szTemp, sizeof(szTemp), "rm -rf %s",szFileName);
	system(szTemp);
	
	diagTestRes->testResult = eStatOk;

	free(buff);

	fprintf(stderr,"ReadWriteTemplate:reading and writing is successful\n");	

	return 0;
}


