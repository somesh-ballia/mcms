#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
//#include "mtd-abi.h"
#include "mtd-user.h"
#include "mtd-abi.h"
#include <getopt.h>
#include "DataTypes.h"
#include "mfa_board.h"
#include "DiagnosticsInfo.h"
#include <string>
//#include "Print.h"
#include "IpmcInt.h"

#define SECTOR_SIZE 256*1024
#define FLASH_DEV "/dev/hda2"

int FlashDevfd = -1;
char FlashParamsBuf[SECTOR_SIZE];
INT32 ResetUpdateFlags = 0;


void CheckFSlog(const char * file, dgnsTestResult *diagTestRes);


void CNTLCompactFlashReadWriteTest(dgnsTestResult *diagTestRes)//CF card is a read only device, don't do write op.
{
   struct mtd_info_user mtd;
   INT32 RetStatus;
   INT32 i;

	FILE * fp = fopen("/config/testRW", "wb");
	if (fp == NULL)
	{
		printf("Could not create file on config, errors in CF\n");
		sprintf(diagTestRes->errString, "Could not create file on config, errors in CF\n");
		diagTestRes->testResult = eStatFail;
		return;
	}
	char buff[1024];
	for (int i =0; i< 1024; i++)
	{
		buff[i] = 'a';
	}
	int result = fwrite(buff, sizeof(char), 1024, fp);
	if (result != 1024)
	{
		printf("Short write into CF card, only %d Byte write, errors in CF\n", result);
		sprintf(diagTestRes->errString, "Short write into CF card, only %d Byte write, errors in CF\n", result);
		diagTestRes->testResult = eStatFail;
		fclose(fp);
		return;
	}
	fclose(fp);
	fp = fopen("/config/testRW", "r");

	char rbuff[1024];
	if (fp == NULL)
	{
		printf("Could not open file on config, errors in CF\n");
		sprintf(diagTestRes->errString, "Could not open file on config, errors in CF\n");
		diagTestRes->testResult = eStatFail;
		return;
	}
	result = fread(rbuff, sizeof(char), 1024, fp);
	if (result != 1024)
	{
		printf("Short read into CF card, only %d Byte write, errors in CF\n", result);
		sprintf(diagTestRes->errString, "Short read into CF card, only %d Byte write, errors in CF\n", result);
		diagTestRes->testResult = eStatFail;
		fclose(fp);
		return;
	}
	for (int i = 0; i< 1024; i++)
	{
		if (rbuff[i] != 'a')
		{
			printf("read values error into CF card, unexpected char %c, errors in CF\n", rbuff[i]);
			sprintf(diagTestRes->errString, "read values error into CF card, unexpected char %c, errors in CF\n", rbuff[i]);
			diagTestRes->testResult = eStatFail;
			fclose(fp);
			return;
		}
	}

	if (0!=fclose(fp))
	{
		printf("error when closing file\n");
		sprintf(diagTestRes->errString, "error when closing file\n");
		diagTestRes->testResult = eStatFail;
		return;
	}
	else
	  system("rm -f /config/testRW");
	diagTestRes->testResult = eStatOk;

   	//return 0;
}


void CNTLCompactFlashFSCheck(dgnsTestResult *diagTestRes)
{
	std::stringstream command;
	std::string answer;
	//to check the CF card
	int stat = SystemPipedCommand("mount | grep /data | awk '{ print $1 }'", answer);

	if(0 != stat)
		{
			SLEEP(2000);
			stat = SystemPipedCommand("mount | grep /data | awk '{ print $1 }'", answer);
			if(0 != stat)
			{
				diagTestRes->testResult = eStatFail;
				strcpy(diagTestRes->errString,"CNTLCompactFlashFSCheck:SystemPipedCommand can not get CF card.");
				return;
			}
		}

	if(answer.find("/dev/hda")!= string::npos)
	{
		command << "e2fsck -c -n -v /dev/hda1 > /output/cffscheck.log";
	}
	else if (answer.find("/dev/sdb")!= string::npos)
	{
		command << "e2fsck -c -n -v /dev/sdb1 > /output/cffscheck.log";
	}
	else if (answer.find("/dev/sda")!= string::npos)
		{
			command << "e2fsck -c -n -v /dev/sda1 > /output/cffscheck.log";
		}
	else {
		strcpy(diagTestRes->errString,"Can't find the System (hda or sdb or sda)");
		diagTestRes->testResult = eStatFail;
		return;
	}
	fprintf(stderr,"CNTLHardDiskSmartCheck::, command = %s\n",(command.str()).c_str());
	printf("ret=%d\n", system((command.str()).c_str()));
	CheckFSlog("/output/cffscheck.log", diagTestRes);
	system("rm -f /output/cffscheck.log");

	return;
	
}

void CheckFSlog(const char * file, dgnsTestResult *diagTestRes)
{
	FILE * fp = fopen(file,"r");

    if (fp == NULL)
    {
    	printf("file system check log cannot be found.\n");
    	diagTestRes->testResult = eStatFail;
    	sprintf(diagTestRes->errString, "no log found.\n");
    	return;
	}

	char buff[1024];
    char num[1024];
    memset(buff, 0,1024);
    memset(num,0,1024);
    int count = fread(buff, sizeof(char), 1024, fp);

    if (count != 0)
    {
    	std::string str(buff, count);
    	size_t pos = str.find("bad blocks");
        if (pos != string::npos)
        {
        	size_t num_pos=str.rfind(" ", pos-2);
            if (num_pos == string::npos)
            {
            	sprintf(diagTestRes->errString, "Parsing log error.\n");
                diagTestRes->testResult = eStatFail;
            	fclose(fp);
                return;
            }

            for (int i = num_pos;i<(int)pos; i++)
            {
            	num[i-num_pos]=str[i];
            }

            if (num[1] != '0')
            {
            	snprintf(diagTestRes->errString, sizeof(diagTestRes->errString), "%s Bad Blocks found .\n", num);
                printf("%s Bad Blocks found .\n", num);
                diagTestRes->testResult = eStatFail;
            	fclose(fp);
                return;
            }
            else
            	printf("no Bad Blocks found\n");
		}

        if (str.find("zero-length")!=string::npos)
        {
        	diagTestRes->testResult = eStatFail;
            sprintf(diagTestRes->errString, "Short read, CF card might be damaged.\n");
        	fclose(fp);
            return;
        }

    }
	else
	{
    	sprintf(diagTestRes->errString, "Could not find log.\n");
        diagTestRes->testResult = eStatFail;
    	fclose(fp);
        return;
	}
	
    diagTestRes->testResult = eStatOk;
	fclose(fp);
	return;
	
}
void CNTLCompactFlashMD5Test(dgnsTestResult *diagTestRes)
{

	if (system("/output/Md5Check.sh") ==0)//only for test.
	{
		diagTestRes->testResult = eStatOk;
	}
	else if (system("/mcms/Scripts/Md5Check.sh")==0)
	{
		diagTestRes->testResult = eStatOk;
	}
	else
	{
		diagTestRes->testResult = eStatFail;
		sprintf(diagTestRes->errString,"MD5 Check failed.");
	}
}

