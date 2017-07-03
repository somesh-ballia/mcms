#include <iostream>
#include <string>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
using namespace std;


#include "DataTypes.h"
#include "StatusesGeneral.h"

STATUS SystemPipedCommand(const string & system_command, string & output_string);
time_t GetTimeFromSwitch();
DWORD ConvertSecondsFromEpochToYear(time_t secondsFromEpoch);
void ConfigTimeInOS(time_t time);





int main (int argc, char *argv[])
{
    bool success = false;
    time_t secondsFromEpoch = 0;
    for(int i = 0 ; i < 60 ; i++)
    {
        cout << "Try : " << i << endl << endl;
        
        secondsFromEpoch = GetTimeFromSwitch();
        DWORD year = ConvertSecondsFromEpochToYear(secondsFromEpoch);
        if(2000 < year)
        {
            cout << "Year is good : " << year << endl;
            success = true;
            break;
        }
        sleep(1); // seconds
    }

    if(false == success)
    {
        perror("Failed to get time from Switch (60 iterations)");
        exit(1);
    }
    
    
    ConfigTimeInOS(secondsFromEpoch);    

    cout << "Success to get time from Switch and configure it in the OS" << endl;
    sleep(2); // seconds
    
    return 0;
}



time_t GetTimeFromLocal()
{
    struct timeval tv;
    memset(&tv, 0, sizeof(struct timeval));
    struct timezone tz;
    memset(&tz, 0, sizeof(struct timezone));
    int result =  gettimeofday(&tv, &tz);
    
    time_t secondsFromEpoch = tv.tv_sec;
    
    return secondsFromEpoch;
}

time_t GetTimeFromSwitch()
{
    string strSelfMngrIp = " http://169.254.128.16:80/ ";
    string strSimulatorIp = " http://172.22.192.65:8080/ ";

    string system_command = "/usr/bin/wget --tries=1 --timeout=1 --output-document=\"-\" --post-data=\"Control\\$Opcode=7\\$MsgID=511\"";
    system_command += strSelfMngrIp; //strSimulatorIp;
    system_command += "2> /dev/null";
    
    string output_string;
    STATUS status = SystemPipedCommand(system_command, output_string);
    if(STATUS_OK != status)
    {
        string message = "Failed to run command : ";
        message += system_command;
        perror(message.c_str());
        return 0;
    }

    const DWORD bufferLen = 1024;
    char buffer [bufferLen];
    strncpy(buffer, output_string.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = 0;

//  the answer :  "Control$Opcode=7$MsgID=511$Status=0$Description=STATUS_OK$SystemTime=45753ae40";

    const char *tag = "SystemTime=";
    char *strSystemTime = strstr(buffer, tag);
    if(NULL == strSystemTime)
    {
        string message = "Failed to parse the answer : ";
        message += buffer;
        perror(message.c_str());
        return 0;
    }
    strSystemTime += strlen(tag);
    
    long long seconds_from_epoch = 0;
    DWORD *secondsFromEpoch1 = (DWORD*)&seconds_from_epoch;
    DWORD *secondsFromEpoch2 = ((DWORD*)&seconds_from_epoch) + 1;
    
    sscanf(strSystemTime + 8, "%x", secondsFromEpoch1);
    strSystemTime[8] = '\0';
    sscanf(strSystemTime, "%x", secondsFromEpoch2);
    
    return (time_t)seconds_from_epoch;
}


DWORD ConvertSecondsFromEpochToYear(time_t secondsFromEpoch)
{
    struct tm sysTm;
    gmtime_r(&secondsFromEpoch, &sysTm);

	DWORD year = sysTm.tm_year;
    
    return 1900 + year;
}


void ConfigTimeInOS(time_t time)
{
    struct timeval tv;
    tv.tv_sec = time;             /* seconds */
    tv.tv_usec = 0;               /* microseconds */

    struct timezone tz;
    tz.tz_minuteswest = 0;       /* minutes W of Greenwich */
    tz.tz_dsttime = 0; //DST_NONE;     /* type of dst correction */
    
    int result = settimeofday(&tv , &tz);
    if(-1 == result)
    {
        perror("Failed to execute settimeofday : ");
        exit(result);
    }
}


STATUS SystemPipedCommand(const string & system_command, string & output_string)
{
    cout << "SystemPipedCommand" << endl;
    
    STATUS ret = STATUS_OK;
  
    output_string = "";
    char line[256];
    FILE *fpipe = NULL;
    if ( !(fpipe = (FILE*)popen(system_command.c_str(),"r")) )
    {
        ret =  STATUS_FAIL;
    }
    else
    {
        cout << "fopen OK" << endl;

        int i = 0;
        while ( fgets( line, sizeof line, fpipe))
        {
            cout << "fgets OK : " << i << endl;
            output_string += line;

            i++;
        }
        
        int exit_value = pclose(fpipe);
        ret = (-1 == exit_value
               ?
               STATUS_FAIL : exit_value);
        
        cout << "pclose( exit_value == " << exit_value << " ) : " << ( STATUS_OK == ret ? "OK" : "FAIL") << endl;
    }
    return ret;
}







