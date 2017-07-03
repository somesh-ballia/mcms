#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <string.h>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <ctime>
#include <list>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <ftw.h>
#include <dirent.h>
#include <limits.h>
#include <float.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>   // for pause()  
#include <signal.h>   // for signal()  
#include <sys/time.h> // struct itimeral. setitimer()  
#include <map>
#include <pthread.h>


using namespace std;

#define MAX_PROC_NAME_SIZE 64
#define MAX_SYSCALL_MSG_SIZE 512
#define SYS_CALL_REQ_MSG_TYPE 20
#define SYS_CALL_RES_MSG_TYPE 40
#define SYS_CALL_KEY 400

typedef unsigned int UINT32;
typedef char INT8;

typedef struct SSysCallReqMsg
{
	UINT32  msgType;
	UINT32  msgInd;
	UINT32  waitOnMsg;
	char msgStr[MAX_SYSCALL_MSG_SIZE];
    char procName[MAX_SYSCALL_MSG_SIZE];

}TSysCallReqMsg;

typedef struct SSysCallResMsg
{
	UINT32  msgType;
	UINT32  msgInd;
	UINT32  result;
}TSysCallResMsg;

enum eLightType
{
	eLightStatus = 0,
	eLightMS,
	
	NumOfLightType
};

enum eLightColor
{
	eLightRed = 0,
	eLightGreen,
	eLightBlue,
	NumOfLightColor
};
static const char* LightTypeStr[NumOfLightType] ={
    "LT:STATUS",
    "LT:M/S"
};


enum eLightAction
{
	eLightOn = 0,
	eLightOff,	
	eLightBlink,
	NumOfLightAction
};
static const char* LightColorStr[NumOfLightColor] ={
    "LC:RED",
    "LC:GREEN",
    "LC:BLUE"
};


typedef struct LightInfoStruct
{
    eLightType light_type;
    eLightColor light_color;
    eLightAction light_act;
    unsigned int t_timer;
public:
    LightInfoStruct()
	{
		light_type = NumOfLightType;
		light_color = NumOfLightColor;
		light_act = NumOfLightAction;
		t_timer = 0;
	}
	LightInfoStruct(eLightType l_type, eLightColor l_color, eLightAction l_action, unsigned int l_timer)
            :light_type(l_type), light_color(l_color), light_act(l_action), t_timer(l_timer)
    {}

}lightInfo;
#ifndef DWORD
typedef         unsigned long     DWORD; 
#endif
#endif

