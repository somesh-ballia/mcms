// SystemFunctions.cpp

#include "SystemFunctions.h"
#include "CpuTemperatureControl.h"

#include "InternalProcessStatuses.h"

#include <limits>
#include <fcntl.h>
#include <errno.h>
#include <ostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/vfs.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <math.h>
#include <stdarg.h>
//#include "IfStructs.h"
#include "IfConfig.h"

#include "StructTm.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"
#include "OsFileIF.h"
#include "McmsProcesses.h"
#include "OsTask.h"
#include "TaskApp.h"
#include "ProcessBase.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "ErrorHandlerTask.h"
#include "ApiStatuses.h"
#include "StringsLen.h"
#include "TraceClass.h"


int m_bogoMipsValue = -1;
int m_cpuSize = -1;
int m_totalMemory = -1;
extern const char* GetSystemRamSizeStr(eSystemRamSize theSize);

#define BIT6(a, l) \
  ((ntohl(a->s6_addr32[(l) / 32]) >> (31 - ((l) & 31))) & 1)

int ipv6_prefix_length(struct in6_addr* a)
{
  int l, i;
  for (l = 0; l < 128; l++)
  {
    if (BIT6(a, l) == 0)
      break;
  }

  for (i = l + 1; i < 128; i++)
  {
    if (BIT6(a, i) == 1)
      return -1;
  }

  return l;
}

void SystemSleep(TICKS ticks, BOOL free_sem)
{
  // Allows to run other tasks during the operation
  CTaskApp::Unlocker unlocker(free_sem);

  if (ticks.m_self.tv_sec)
    sleep(ticks.m_self.tv_sec);

  if (ticks.m_self.tv_usec)
    usleep(ticks.m_self.tv_usec);
}

TICKS SystemGetTickCount()
{
  struct timespec tv;
  clock_gettime(CLOCK_MONOTONIC, &tv);
  return TICKS(tv.tv_sec, tv.tv_nsec/1000);
}

STATUS SystemAllocateSharedMemory(SM_HANDLE& Shared_Memory_Descriptor,
                                  BYTE** newMemory,
                                  DWORD size,
                                  const char* name,
                                  BOOL& isFirst)
{
  key_t       shared_memory_key;
  STATUS      stat = STATUS_OK;
  std::string shared_memory_name;
  shared_memory_name = name;
  shared_memory_name = MCU_TMP_DIR+"/shared_memory/" + shared_memory_name;

  if (-1 == open(shared_memory_name.c_str(), O_RDWR|O_CREAT, 0666))
  {
    perror("SystemAllocateSharedMemory:: Error Opening file");
    perror(shared_memory_name.c_str());
    FPASSERTMSG(1, "SystemAllocateSharedMemory:: Error Opening file");
    return STATUS_FAIL;
  }

  chmod(shared_memory_name.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

  if ((shared_memory_key = ftok(shared_memory_name.c_str(), 'm')) == -1)
  {
    perror("SystemAllocateSharedMemory - ftok failed");
    return STATUS_FAIL;
  }

  isFirst = TRUE;
  if ((Shared_Memory_Descriptor = shmget(shared_memory_key,
                                         size,
                                         IPC_CREAT|IPC_EXCL|0666)) == -1)
  {
    if (errno != EEXIST) // we have an error reaching the shared memory
    {
      perror("shmget failed");
      return STATUS_FAIL;
    }
    else
    {
      isFirst = FALSE;
      if ((Shared_Memory_Descriptor = shmget(shared_memory_key, size, 0)) == -1)
      {
        perror("shmget2 failed");
        return STATUS_FAIL;
      }
    }
  }

  if ((int)(*newMemory = (BYTE*)shmat(Shared_Memory_Descriptor, 0, 0)) == -1)
  {
    perror("SystemAllocateSharedMemory shmat cannot allocate memory");
    return STATUS_FAIL;
  }

  return stat;
}

STATUS SystemFreeSharedMemory(void* pShared_Memory,
                              SM_HANDLE Shared_Memory_Descriptor)
{
  int             number_of_attached_procs;
  struct shmid_ds temp_shm_struct;

  if (-1 == shmdt(pShared_Memory))
  {
    perror("SystemFreeSharedMemory shmdt failed");
    return STATUS_FAIL;
  }

  if (-1 == shmctl(Shared_Memory_Descriptor, IPC_STAT, &temp_shm_struct))
  {
    perror("SystemFreeSharedMemory shmctl get stat failed");
    return STATUS_FAIL;
  }

  number_of_attached_procs = temp_shm_struct.shm_nattch;
  if (number_of_attached_procs == 0)
    {
    if (-1 == shmctl(Shared_Memory_Descriptor, IPC_RMID, 0))
    {
      perror("SystemFreeSharedMemory shmctl IPC_RMID failed");
      return STATUS_FAIL;
    }
    }
  return STATUS_OK;
}

STATUS SystemGetTime(CStructTm& t)
{
  time_t now = time((time_t*)NULL);
  t.SetAbsTime(now);

  return STATUS_OK;
}

BOOL SystemIsBadReadPtr(const void* ptr, unsigned int size)
{
  return FALSE;
}

DWORD SystemIpStringToDWORD(const char* ipAddress,
                             eIpPresentationType type)
{
  DWORD ip = inet_addr(ipAddress);
  ip = (type == eHost ? htonl(ip) : ip);

  return ip;
}

void SystemDWORDToIpString(const DWORD ip, char* ipAddress)
{
//  the ip must be in little indian format : "1.2.3.4" -> value = 0x01020304
//                              memory= 04030201
  sprintf(ipAddress, "%d.%d.%d.%d", GET_LE_IP_BYTE_1(ip),
          GET_LE_IP_BYTE_2(ip),
          GET_LE_IP_BYTE_3(ip),
          GET_LE_IP_BYTE_4(ip)
          );
}

void SystemIpStringToIntArray(const char* ipAddress,
                              unsigned char* Octet_array)
{
  DWORD ip = SystemIpStringToDWORD(ipAddress);
  Octet_array[0] = GET_LE_IP_BYTE_1(ip);
  Octet_array[1] = GET_LE_IP_BYTE_2(ip);
  Octet_array[2] = GET_LE_IP_BYTE_3(ip);
  Octet_array[3] = GET_LE_IP_BYTE_4(ip);
}

eIpType IpVersionToIpType(enIpVersion ipVersion)
{
  switch (ipVersion)
  {
    case eIpVersion4:
      return eIpType_IpV4;

    case eIpVersion6:
      return eIpType_IpV6;

    default:
      break;
  }

  return eIpType_None;
}

enIpVersion IpTypeToIpVersion(eIpType ipType)
{
  switch (ipType)
  {
    case eIpType_IpV4:
      return eIpVersion4;

    case eIpType_IpV6:
      return eIpVersion6;

    default:
      break;
  }

  return eIpVersion4;
}

char* ipV4ToString(const APIU32 ipAddress, char* sIpAddress)
{
  unsigned char* ip = (unsigned char*)&ipAddress;
  sprintf(sIpAddress, "%d.%d.%d.%d", (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]);

  return sIpAddress;
}

char* ipV6ToString(const APIU8* ipAddress,
                   char* sIpAddress,
                   BOOL addBrackets)
{
  struct sockaddr_in6 sockAddrV6;
  char*               pIpAddress = sIpAddress;
  socklen_t cnt;

  if (addBrackets)
  {
    sIpAddress[0] = '[';
    pIpAddress++;
    cnt = IPV6_ADDRESS_LEN - 2;
  }
  else
  {
    cnt = IPV6_ADDRESS_LEN;
  }

  memcpy(&sockAddrV6.sin6_addr, ipAddress, sizeof(in6_addr));

  const char* dst = inet_ntop(AF_INET6,
                              &sockAddrV6.sin6_addr,
                              pIpAddress,
                              IPV6_ADDRESS_LEN);

  int errnoCode = errno;
  if (addBrackets)
    strcat(sIpAddress, "]");

  return sIpAddress;
}

char* ipV6ToStringProtected(const APIU8* ipAddress,
                   char* sIpAddress,
                   BOOL addBrackets,
                   size_t sizeOfAddr)
{
  struct sockaddr_in6 sockAddrV6;
  char*               pIpAddress = sIpAddress;
  socklen_t cnt;

  if (addBrackets)
  {
    sIpAddress[0] = '[';
    pIpAddress++;
    cnt = IPV6_ADDRESS_LEN - 2;
  }
  else
  {
    cnt = IPV6_ADDRESS_LEN;
  }

  memcpy(&sockAddrV6.sin6_addr, ipAddress, sizeof(in6_addr));

  const char* dst = inet_ntop(AF_INET6,
                              &sockAddrV6.sin6_addr,
                              pIpAddress,
                              IPV6_ADDRESS_LEN);

  int errnoCode = errno;
  if (addBrackets)
  {
	  if(strlen(sIpAddress) < sizeOfAddr -1) // B.S klocwork 2568
		  strcat(sIpAddress, "]");
  }
  return sIpAddress;
}

char* ipV6AndSubnetMaskToString(const APIU8* ipAddress,
								DWORD subNetMask,
								char* outFullIpAddress,
								BOOL addBrackets)
{
	const int MAX_SUBNET_LEN = 11;
	char subNetMaskStr[MAX_SUBNET_LEN];
	sprintf((char *)subNetMaskStr, "%d", (int)subNetMask);
	return ipV6AndSubnetMaskStrToString(ipAddress, (const char *)subNetMaskStr, outFullIpAddress, addBrackets);
}

char* ipV6AndSubnetMaskStrToString(const APIU8* ipAddress,
								const char* subNetMaskStr,
								char* outFullIpAddress, // FULL_IPV6_ADDRESS_LEN
								BOOL addBrackets)
{
	memset(outFullIpAddress, 0, IPV6_ADDRESS_LEN);

	char outIpAddress[IPV6_ADDRESS_LEN];
    memset((char *)outIpAddress, 0, IPV6_ADDRESS_LEN);
    ipV6ToString(ipAddress, outIpAddress, addBrackets);

    if (subNetMaskStr &&  strcmp("::", (const char *)outFullIpAddress) !=0)
    {
    		snprintf((char *)outFullIpAddress, IPV6_ADDRESS_LEN, "%s/%s", outIpAddress, subNetMaskStr);

    }
	else
	{
			memcpy((char *)outFullIpAddress, (const char *)outIpAddress, IPV6_ADDRESS_LEN);
	}

    return outFullIpAddress;
}

char* ipToString(mcTransportAddress ipAddress,
                 char* sIpAddress,
                 BOOL addBrackets)
{
  if (ipAddress.ipVersion == eIpVersion4)
  {
    SystemDWORDToIpString(ipAddress.addr.v4.ip, sIpAddress);
    return sIpAddress;
  }

  return ipV6ToString(ipAddress.addr.v6.ip, sIpAddress, addBrackets);
}

char* ipToString(ipAddressStruct ipAddress, char* sIpAddress, BOOL addBrackets)
{
  if (ipAddress.ipVersion == eIpVersion4)
  {
    SystemDWORDToIpString(ipAddress.addr.v4.ip, sIpAddress);
    return sIpAddress;
  }

  return ipV6ToString(ipAddress.addr.v6.ip, sIpAddress, addBrackets);
}

void stringToIpV4(mcTransportAddress* ipAddress,
                  char* sIpAddress,
                  eIpPresentationType type)
{
  ipAddress->ipVersion = eIpVersion4;
  DWORD ip = inet_addr(sIpAddress);
  ip = (type == eHost ? htonl(ip) : ip);
  ipAddress->addr.v4.ip = ip;
}

int stringToIpV6(mcTransportAddress* ipAddress, const char* sIpAddress)
{
	struct sockaddr_in6 sockAddrV6;
	const char* pIpAddress = sIpAddress;
	char* bracketsPtr = NULL;

	if (pIpAddress[0] == '[')
	{
		++pIpAddress;
		bracketsPtr = (char*)strchr(sIpAddress, ']');
		if (bracketsPtr) {
			bracketsPtr[0] = '\0';
		}
	}

	ipAddress->ipVersion = eIpVersion6;
	memset(&sockAddrV6, 0, sizeof(sockAddrV6));
	sockAddrV6.sin6_family = AF_INET6;

	int ret = inet_pton(AF_INET6, pIpAddress, &(sockAddrV6.sin6_addr));
	int errnoCode = errno;

	memcpy(ipAddress->addr.v6.ip, &sockAddrV6.sin6_addr, sizeof(in6_addr));

	if (bracketsPtr)
		bracketsPtr[0] = ']';

	return ret;
}

BOOL isIpV4Str(const char* sIpAddress)
{
  DWORD dIp = 0;
  if(sIpAddress)
	  dIp = SystemIpStringToDWORD(sIpAddress, eHost);

  // ===== 1. a 'good' address
  if (INADDR_NONE != dIp)
    return TRUE;

  // ===== 2. 0xFFFFFFFF (255.255.255.255, which is a valid address)
  else if ((INADDR_NONE == dIp) &&      // INADDR_NONE is usually -1
           ((sIpAddress) && (strcmp("255.255.255.255", sIpAddress) == 0)))
    return TRUE;

  // ===== 1. an invalid address
  return FALSE;
}

BOOL isIpV6Str(const char* sIpAddress)
{
  int index;
  int len = strlen(sIpAddress);

  if(0 == len){
	  return FALSE;
  }

  // If we add port-number to the IPv6, the IPv6 must be under [].
  if (sIpAddress[0] == '[')
    return TRUE;


  // In IPv6 one of the first 5 characters must be ':'
  for (index = 0; index < 5 && index < len; index++)
  {
    if (sIpAddress[index] == ':')
      return TRUE;
  }

  return FALSE;
}

void SplitIPv6AddressAndMask(char* pInAddress,
                             char* pOutAddress,
                             char* pOutMask)
{
  // init the strings, just for safety
  pInAddress[IPV6_ADDRESS_LEN-1] = '\0';
  memset(pOutAddress, 0, IPV6_ADDRESS_LEN);
  memset(pOutMask, 0, IPV6_ADDRESS_LEN);
  char* pCh;

  pCh = strtok(pInAddress, "/");
  if (pCh)
  {
    strncpy(pOutAddress, pCh, IPV6_ADDRESS_LEN);  // ---- address ----

    pCh = strtok(NULL, "/");  // look for the rest of the addressString (starting after the '/')
    if (pCh)
      strncpy(pOutMask, pCh, IPV6_ADDRESS_LEN); // ----   mask  ----
  }

  else // '/' was not found
  {
    strncpy(pOutAddress, pInAddress, IPV6_ADDRESS_LEN); // ---- address ----
    strcpy(pOutMask, "64"); // no mask - pOutMask will be default - 64      // ----   mask  ----
  }

  pOutAddress[IPV6_ADDRESS_LEN - 1] = '\0';

  return;
}

void stringToIp(mcTransportAddress* ipAddress,
                char* sIpAddress,
                eIpPresentationType type)
{
  if (isIpV6Str(sIpAddress))
    stringToIpV6(ipAddress, sIpAddress);
  else
    stringToIpV4(ipAddress, sIpAddress, type);
}

void stringToIpV4(ipAddressStruct* ipAddress,
                  const char* sIpAddress,
                  eIpPresentationType type)
{
  ipAddress->ipVersion = eIpVersion4;
  DWORD ip = inet_addr(sIpAddress);
  ip = (type == eHost ? htonl(ip) : ip);
  ipAddress->addr.v4.ip = ip;
}

void stringToIpV6(ipAddressStruct* ipAddress, char* sIpAddress)
{
  struct sockaddr_in6 sockAddrV6;
  char*               pIpAddress = sIpAddress;
  char*               bracketsPtr = NULL;

  if (pIpAddress[0] == '[')
  {
    pIpAddress++;
    bracketsPtr = strchr(sIpAddress, ']');
    if (bracketsPtr) {
      bracketsPtr[0] = '\0';
    }
  }

  ipAddress->ipVersion = eIpVersion6;
  memset(&sockAddrV6, 0, sizeof(sockAddrV6));
  sockAddrV6.sin6_family = AF_INET6;

  int ret = inet_pton(AF_INET6, pIpAddress, &(sockAddrV6.sin6_addr));
  int errnoCode = errno;

  memcpy(ipAddress->addr.v6.ip, &sockAddrV6.sin6_addr, sizeof(in6_addr));


  if (bracketsPtr)
    bracketsPtr[0] = ']';

}

void stringToIp(ipAddressStruct* ipAddress,
                char* sIpAddress,
                eIpPresentationType type)
{
  if (isIpV6Str(sIpAddress))
    stringToIpV6(ipAddress, sIpAddress);
  else
    stringToIpV4(ipAddress, sIpAddress, type);
}

// Description: check if Api ip address is Null
// Address.
// Return code: TRUE if null
// FALSE not null
int isApiTaNull(const mcTransportAddress* pApiAddr)
{
  if (pApiAddr->ipVersion == eIpVersion4)
    return IsIpNull(pApiAddr->addr.v4);

  else if (pApiAddr->ipVersion == eIpVersion6)     // May test port
    return IsIpNull(pApiAddr->addr.v6);

  return TRUE;
}

void IPv6RemoveBrackets(const char* strInWithBrackets,
                        char* strOutWithoutBrackets)
{
  // init the strings, just for safety
  memset(strOutWithoutBrackets, 0, IPV6_ADDRESS_LEN);

  if ('[' != strInWithBrackets[0]) // inAddress does not contain brackets

    strncpy(strOutWithoutBrackets, strInWithBrackets, IPV6_ADDRESS_LEN); // copy as is (no brackets anyway)

  else // inAddress containsw brackets
  {
    strncpy(strOutWithoutBrackets, strInWithBrackets+1, IPV6_ADDRESS_LEN-1);
    char* pCh = strchr(strOutWithoutBrackets, ']');
    if (pCh)
    {
      int bracketIdx = pCh-strOutWithoutBrackets;
      strOutWithoutBrackets[bracketIdx] = '\0';
    }
  }
}

bool IsIpNull(const ipAddressStruct* pAddr)
{
  if (pAddr->ipVersion == eIpVersion4)
    return IsIpNull(pAddr->addr.v4);

  else if (pAddr->ipVersion == eIpVersion6) // May test port
    return IsIpNull(pAddr->addr.v6);

  return true;
}

bool IsIpNull(ipAddressV4If addr)
{
  bool ret = true;

  if (0 != addr.ip)
    ret = false;

  return ret;
}

bool IsIpNull(ipAddressV6If addr)
{
  bool ret = true;

  static APIU8 ipNull[16] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                             0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

  if (memcmp(addr.ip, ipNull, sizeof(addr.ip)))
    ret = false;

  return ret;
}

int isIpTaNonValid(mcTransportAddress* pApiAddr)
{
	if (pApiAddr->ipVersion == eIpVersion4) {

    if (pApiAddr->addr.v4.ip == 0xFFFFFFFF)
      return TRUE;
		else
    return FALSE;

	} else if (pApiAddr->ipVersion == eIpVersion6) { // May test port

    static APIU8 ipFF[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

		if ( memcmp(
				pApiAddr->addr.v6.ip,
               ipFF,
               sizeof(pApiAddr->addr.v6.ip)))      // not equal to NULL return true
      return FALSE;
  }

  return TRUE;
}

int isIpAddressEqual(const mcTransportAddress* firstAddr,
                     const mcTransportAddress* secondAddr)
{
  if (firstAddr->ipVersion == secondAddr->ipVersion)
  {
    if (firstAddr->ipVersion == (APIU32)eIpVersion4)
    {
      if (firstAddr->addr.v4.ip == secondAddr->addr.v4.ip)
        return TRUE;
    }
    else if (firstAddr->ipVersion == (APIU32)eIpVersion6)
    {
		if (!memcmp(firstAddr->addr.v6.ip,
                  secondAddr->addr.v6.ip,
                  IPV6_ADDRESS_BYTES_LEN))
        return TRUE;
    }
  }

  return FALSE;
}

DWORD SystemInitSocket()
{
  return 0;
}

DWORD SystemCleanupSocket()
{
  return 0;
}

STATUS SystemRunProcess(int processNumber,
                        const char* arg1,
                        BOOL giveUpRoot,
                        int nice)
{
  int    res;
  std::string ProcessName = ProcessNames[processNumber];
  std::string killCommand = "killall -q ";
  killCommand += ProcessName;
  std::string killAnswer;
  SystemPipedCommand(killCommand.c_str(), killAnswer, TRUE, FALSE);

  std::string linkProcessName = "Links/" + ProcessName;
  if (IsFileExists(linkProcessName))
  {
    if (SystemRunCommand(linkProcessName.c_str(),
                         arg1,
                         NULL,
                         NULL,
                         NULL,
                         giveUpRoot,
                         nice) != STATUS_OK)
    {
      perror(linkProcessName.c_str());
      return STATUS_FAIL;
    }

    return STATUS_OK;
  }
  else
  {
    std::string binProcessName = "Bin/" + ProcessName;
    if (IsFileExists(binProcessName))
    {
      if (SystemRunCommand(binProcessName.c_str(),
                           arg1,
                           NULL,
                           NULL,
                           NULL,
                           giveUpRoot,
                           nice) != STATUS_OK)
      {
        perror(binProcessName.c_str());
        return STATUS_FAIL;
      }

      return STATUS_OK;
    }

    return STATUS_EXE_NOT_FOUND;
  }

  return STATUS_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
STATUS SystemRunCommand(const char * system_command, const char *arg1, const char *arg2, const char* arg3, const char* arg4, BOOL giveUpRoot,int nice)
{
	pid_t pid = fork();
	FPASSERTSTREAM_AND_RETURN_VALUE(pid < 0,
		"fork: " << system_command <<
		": " << strerror(errno) << " (" << errno << ")",
		STATUS_FAIL);

	FTRACECOND_AND_RETURN_VALUE(pid > 0,
		system_command << " was born with PID " << pid,
		STATUS_OK);

	// The code runs in new process

	// Changes MM process priority for Call Generator
	if (nice != 0xffff)
	  setpriority(PRIO_PROCESS, 0, nice);

	if (giveUpRoot)
	{
	  setgid(200);
	  setuid(200); // set user and group as mcms
	}

	std::ostringstream buf1;
	buf1 << "execl: " << system_command;
	WriteLocalLog(buf1.str().c_str());

	int res;
	errno = 0;
	if (NULL != arg4)
	  res = execl(system_command, system_command, arg1, arg2, arg3, arg4, (char*)0);
	else if (NULL != arg3)
	  res = execl(system_command, system_command, arg1, arg2, arg3, (char*)0);
	else if (NULL != arg2)
	  res = execl(system_command, system_command, arg1, arg2, (char*)0);
	else if (NULL != arg1)
	  res = execl(system_command, system_command, arg1, (char*)0);
	else
	  res = execl(system_command, system_command, (char*)0);

	std::ostringstream buf2;
	buf2 << "execl: " << system_command << ": end";
	if (res < 0)
	  buf2 << ": " << strerror(errno) << " (" << errno << ")";
	WriteLocalLog(buf2.str().c_str());

	return STATUS_OK;

}

int core_count = 0;

/////////////////////////////////////////////////////////////////////////////
void SystemCoreDump(BOOL continue_running,
                    BOOL only_if_in_debug)
{

	if (only_if_in_debug)
	{
		BOOL isDebugMode = FALSE;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE,
				isDebugMode);
		if (!isDebugMode)
			return;
	}

	if (continue_running == FALSE)
	{
		//signal(SIGABRT,SIG_DFL );
		abort();
	}
	else
	{
		if (core_count == 0)
		{
			pid_t child = fork();
			if (0 == child)
			{
				//signal(SIGABRT,SIG_DFL );
				abort();
			}
			else
			{
				waitpid(child, 0, 0);
			}
		}
	}
	core_count++;
}

void SystemSync()
{
	FILE*       fpipe = NULL;
  	const char* sync = "sync";

  	if (!(fpipe = (FILE*)popen(sync, "r")))
  	{
    	FPASSERTMSG(1, sync);
  	}
  	else
	{
		if(-1 == pclose(fpipe))
    	{
    		FTRACEINTO << "SystemSync: Failed to close pipe";
        }
	}
}

/*STATUS SystemPipedCommand(const char* cmd)
{
  std::string dummy;
  return SystemPipedCommand(cmd, dummy, TRUE, TRUE);
}
*/

STATUS SystemPipedCommand(const char* cmd,
                          std::string& out,
                          BOOL free_sem /* = TRUE */,
                          BOOL is_print /* = TRUE */,
                          BOOL is_need_answer /*TRUE*/)
{
	// Allows to run other tasks during the operation
	CTaskApp::Unlocker unlocker(free_sem);

	FILE* fpipe = popen(cmd, "r");
	FPASSERTSTREAM_AND_RETURN_VALUE(NULL == fpipe,
		"popen: " << cmd << ": " << strerror(errno) << " (" << errno << ")",
		STATUS_FAIL);

	out.clear();
	if (is_need_answer)
	{
	  	char line[256];
		while (fgets(line, ARRAYSIZE(line), fpipe))
		  out += line;
	}

	// Does not return pclose status because some system command fail wait4
	// man: The pclose function waits for the associated process to terminate and
	// returns the exit status of the command as returned by wait4
	// Does not check the return status because some system command fail wait4
	int rc = pclose(fpipe);
	FPASSERTSTREAM_AND_RETURN_VALUE(rc < 0,
		"pclose: " << cmd << ": " << strerror(errno) << " (" << errno << ")",
		STATUS_OK);

	FTRACECOND(is_print, cmd << ": " << out);

	return STATUS_OK;

}

STATUS GetCpuUsage(TICKS& user, TICKS& system, int who)
{
  struct rusage usage;
  int           ret;

  ret = getrusage(who, &usage);


    if (ret != 0)
    {
        return STATUS_FAIL;
    }

  user.m_self = usage.ru_utime;
  system.m_self = usage.ru_stime;
  return STATUS_OK;
}

STATUS GetSelfCpuUsage(TICKS& user, TICKS& system)
{
  return GetCpuUsage(user, system, RUSAGE_SELF);
}

STATUS GetChildrenCpuUsage(TICKS& user, TICKS& system)
{
  return GetCpuUsage(user, system, RUSAGE_CHILDREN);
}

STATUS DumpFile(const string& fname)
{
  std::string buf;
  STATUS stat = ReadFileToString(fname.c_str(),
                                 std::numeric_limits<unsigned long>::max(),
                                 buf);

  if (STATUS_OK == stat)
    FTRACEINTO << "Dump of " << fname << ":\n" << buf;

  return stat;
}

STATUS DumpSystemStatistics(std::ostream& ostr)
{
  TICKS selfUser, selfSystem, childUser, childSystem;
  TICKS sum;

  pid_t pid = getpid();

  STATUS res1 = GetSelfCpuUsage(selfUser, selfSystem);
  STATUS res2 = GetChildrenCpuUsage(childUser, childSystem);

  if (res1 == STATUS_OK && res2 == STATUS_OK)
  {
    sum = selfUser + childUser + selfSystem + childSystem;
    ostr << "PID: " << pid
         << " User: " <<  selfUser+childUser
         << " System: " << selfSystem+childSystem
         << " Total: " << sum;

    return STATUS_OK;
  }

  FPASSERT(res1);
  FPASSERT(res2);

  return STATUS_FAIL;
}

BOOL IsTarget()
{
	if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
		return FALSE;

	int uid = getuid();
	if (uid == 200 || uid == 0)
		return TRUE;

	return FALSE;
}


BOOL IsRmxSimulation()
{
	if (eProductFamilyRMX != CProcessBase::GetProcess()->GetProductFamily())
		return FALSE;

	int uid = getuid();
	if (uid == 200 || uid == 0)
		return FALSE;

	return TRUE;
}
static BOOL SkipOperation()
{
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    if(eProductTypeSoftMCU == curProductType || eProductTypeSoftMCUMfw == curProductType || eProductTypeEdgeAxis == curProductType)
    {
        return TRUE;
    }
    else if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
    {
        return FALSE;
    }

    int uid = getuid();
    if (uid == 200 || uid == 0)
    {
        return FALSE;
    }

    return TRUE;
}

STATUS GetCurrentTime(time_t& l_time)
{
  // ZERO INPUT VALUE l_time from calling function
  if (-1 == time(&l_time))
    return STATUS_FAIL;

  return STATUS_OK;
}

BOOL IsPatchedVersion()
{
  if (!IsTarget())
    return FALSE;

  std::vector<std::string> fstruct;
  std::ostringstream dummy;
  std::string dirname = MCU_MCMS_DIR+"/Links/";
  GetDirectoryContents(dirname.c_str(), fstruct, dummy);

  if (fstruct.size() > 0)
    return TRUE;

  char answer[256];
  memset(answer, '\0', sizeof(answer));
  int readlinkPos = readlink((MCU_CONFIG_DIR+MCU_CS_DIR).c_str(), answer, ARRAYSIZE(answer) -1);
  if (readlinkPos != -1)
  {
    if (strcmp(answer, (MCU_CS_DIR+"/version_cfg").c_str()) != 0)
      return TRUE;
  }
  else
  {
    FTRACEINTOFUNC << "IsPatchedVersion readlink: "<<MCU_CONFIG_DIR<<MCU_CS_DIR<<": "
                   << strerror(errno) << " (" << errno << ")";
  }

  return FALSE;
}

std::string StripVersionFromFileName(const std::string& fileName)
{
  int         index1 = fileName.find("\n");
  std::string noNewLine(fileName, 0, index1);

  int         index2 = noNewLine.find(".bin");
  std::string noBin(noNewLine, 0, index2);

  return noBin;
}

BOOL GetVersions(std::string& running,
                 std::string& current,
                 std::string& fallback,
                 std::string& factory)
{
  if (!IsTarget())
    return FALSE;

  char fname[1024];

  int res1 = readlink((MCU_DATA_DIR+"/current").c_str(), fname, ARRAYSIZE(fname) - 1);
  if (res1 != -1)
  {
    fname[res1] = '\0';
    current = StripVersionFromFileName(fname);
  }
  else
    FTRACEINTOFUNC << "readlink: "<<MCU_DATA_DIR<<"/current: "
                   << strerror(errno) << " (" << errno << ")";

  int res2 = readlink((MCU_DATA_DIR+"/fallback").c_str(), fname, ARRAYSIZE(fname) - 1);
  if (res2 != -1)
  {
    fname[res2] = '\0';
    fallback = StripVersionFromFileName(fname);
  }
  else
    FTRACEINTOFUNC << "readlink: "<<MCU_DATA_DIR<<"/fallback: "
                   << strerror(errno) << " (" << errno << ")";

  int res3 = readlink((MCU_DATA_DIR+"/factory").c_str(), fname, ARRAYSIZE(fname) - 1);
  if (res3 != -1)
  {
    fname[res3] = '\0';
    factory = StripVersionFromFileName(fname);
  }
  else
    FTRACEINTOFUNC << "readlink: "<<MCU_DATA_DIR+"/factory: "
                   << strerror(errno) << " (" << errno << ")";

  STATUS res = SystemPipedCommand("cat /version.txt", running);
  running = StripVersionFromFileName(running);

  return (res1 != -1 &&
          res2 != -1 &&
          res3 != -1 &&
          res == STATUS_OK);
}

// Causes blocking status
void SyncMedia(BOOL free_sem /* = TRUE */)
{
	// Allows to run other tasks during the operation
	CTaskApp::Unlocker unlocker(free_sem);

	sync();

}

BOOL SetAddressSpaceLimit(int limit)
{

	const struct rlimit buf = {limit, limit};
	int 				res = setrlimit(RLIMIT_AS, &buf);
	FPASSERTSTREAM_AND_RETURN_VALUE(res < 0,
		"setrlimit: " << limit << ": " << strerror(errno) << " (" << errno << ")",
		FALSE);

	return TRUE;
}

bool IsUnderValgrind(const char* pname)
{

  	FPASSERT_AND_RETURN_VALUE(NULL == pname, false);
    if (IsTarget())
  	{
		std::stringstream fname;
		fname << (MCU_MCMS_DIR+"/Links/").c_str() << pname;
		return IsFileExists(fname.str());
	}

	std::string       ans;
	std::stringstream cmd;
	cmd << "ps -ef | grep valgrind | grep " << pname << " | grep -v grep";

	STATUS stat = SystemPipedCommand(cmd.str().c_str(), ans);
	FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
	  "SystemPipedCommand: " << cmd.str() << ": " << ans,
	  false);

	return !ans.empty();

}

int GetUsedMemory(BOOL isToPrint /*=TRUE*/)
{
  std::stringstream command;
  std::string       answer;
  pid_t             pid = getpid();
    command <<"cat /proc/" <<pid << "/status | grep VmSize | awk '{ print $2 }'";

  SystemPipedCommand((command.str()).c_str(), answer, TRUE, isToPrint);
    return (atoi(answer.c_str()));
}

int GetUsedMemory()
{
  std::stringstream command;
  std::string       answer;
  command <<"cat /proc/meminfo | grep MemTotal | awk '{ print $2 }'";
  SystemPipedCommand((command.str()).c_str(), answer);
  return (atoi(answer.c_str()));
}

int GetTotalMemory()
{
  std::stringstream command;
  std::string       answer;
  command <<"cat /proc/meminfo | grep MemTotal | awk '{ print $2 }'";
  SystemPipedCommand((command.str()).c_str(), answer);
  return (atoi(answer.c_str()));
}

bool CheckCpuValueValidity(int cpusize)
{
	switch (cpusize)
	{
		case eSystemCPUSize_2:
		case eSystemCPUSize_4:
		case eSystemCPUSize_8:
		case eSystemCPUSize_12:
		case eSystemCPUSize_16:
		case eSystemCPUSize_24:
		case eSystemCPUSize_32:
			return true;
		default:
			return false;

	}

}

float GetCpuFactor()
{
        std::stringstream command;
        std::string       answer;
        float cpuSize=0;

        if (m_cpuSize != -1)
                cpuSize = (float)m_cpuSize;
        else
        {
                command.str("");
                command << "grep processor /proc/cpuinfo | wc -l";
                SystemPipedCommand((command.str()).c_str(), answer);
                cpuSize = atoi(answer.c_str());
                FTRACESTR(eLevelInfoNormal) << "\n GetCpuFactor - cpuProcessors:" << cpuSize;
        }

        eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
        int num_cores_limt = (eProductTypeEdgeAxis == curProductType ? MAX_CORES_LIMIT_EDGE : MAX_CORES_LIMIT);

        float cpuSizeFactor=cpuSize/num_cores_limt;
        FTRACESTR(eLevelInfoNormal) << "\n GetCpuFactor - cpuSize:" << cpuSize << ";cpuSizeFactor:" << cpuSizeFactor;

        return cpuSizeFactor;
}



void SetBogmipsValue(int val)
{
	m_bogoMipsValue=val;

}
void SetCpuSize(int val)
{
	m_cpuSize=val;

}
void SetTotalMemory(int val)
{
	m_totalMemory=val;

}

float GetMemoryFactor()
{
//print_f ("GetMemoryFactor!!! %d",m_bogoMipsValue);
	float ramInGB;
	if (m_totalMemory != -1)
		ramInGB = (float)m_totalMemory;
	else
		ramInGB = GetTotalMemory()/1000000;

	float memoryFactor;
	if (ramInGB < MAX_MEMORY_LIMIT)
		memoryFactor = ramInGB/MAX_MEMORY_LIMIT;
	else
		memoryFactor = 1;

	FTRACESTR(eLevelInfoNormal) << "\n GetMemoryFactor - ramInGB:" << ramInGB << ";memoryFactor:" << memoryFactor;

	return memoryFactor;
}

WORD GetMachineProfileGrade()//bool calculateHT)
{
//	if ( !IsEdgeAxisMcuInSimulation())
//		return 100;


	float cpuSizeFactor=GetCpuFactor();
	FPASSERTSTREAM(cpuSizeFactor <= 0 || cpuSizeFactor > MAX_CORE_SIZE_FACTOR, "GetCpuFactor - number of cores size is not valid");

	float bogMibsFactor=GetBogoMipsFactor();
	FPASSERTSTREAM(bogMibsFactor <= 0 || bogMibsFactor > MAX_BOGO_MIPS_FACTOR, "GetBogoMipsFactor - bogomips value is not valid");

	float memoryFactor=GetMemoryFactor();
	FPASSERTSTREAM(memoryFactor <= 0 || memoryFactor > MAX_MEMORY_FACTOR, "GetMemoryFactor - memory value is not valid");


	float machineGrade= (cpuSizeFactor*bogMibsFactor*memoryFactor*100);

	FTRACESTR(eLevelInfoNormal) << "\n GetMachineProfileGrade - machineGrade:" << machineGrade;



	if (machineGrade <= 0 || machineGrade > 200)
			FPASSERTMSG(true, "GetMachineProfileGrade - machineGrade value is not valid");


	return (WORD)ceil(machineGrade);
}

float GetBogoMipsFactor()
{
	std::stringstream command;
	std::string       answer;
	float bogoMipsValue;
	if (m_bogoMipsValue != -1)
		bogoMipsValue = (float)m_bogoMipsValue;
	else
	{
		command << "cat /proc/cpuinfo | grep 'bogomips' | head -1 | tr -d 'bogomips:'";
		SystemPipedCommand((command.str()).c_str(), answer);
		bogoMipsValue = atoi(answer.c_str());
	}
	float bogomipsFactor = bogoMipsValue/MAX_BOGO_MIPS_LIMIT;
	if (bogomipsFactor > 0.7)
		return 1;


	FTRACESTR(eLevelInfoNormal) << "\n GetBogoMipsFactor - bogoMipsValue:" << bogoMipsValue << ";bogomipsFactor:" << bogomipsFactor;

	return bogomipsFactor;
}

int GetCpuSizeInRange(float calcCPUSize)
{

	int cpuSize = eSystemCPUSize_illegal;
		if (calcCPUSize < eSystemCPUSize_2)
			cpuSize = eSystemCPUSize_illegal;
		else if (calcCPUSize >= eSystemCPUSize_2 && calcCPUSize < eSystemCPUSize_4)
			cpuSize = eSystemCPUSize_2;
		else if (calcCPUSize >= eSystemCPUSize_4 && calcCPUSize < eSystemCPUSize_8)
			cpuSize = eSystemCPUSize_4;
		else if (calcCPUSize >= eSystemCPUSize_8 && calcCPUSize < eSystemCPUSize_12)
			cpuSize = eSystemCPUSize_8;
		else if (calcCPUSize >= eSystemCPUSize_12 && calcCPUSize < eSystemCPUSize_16)
			cpuSize = eSystemCPUSize_12;
		else if (calcCPUSize >= eSystemCPUSize_16 && calcCPUSize < eSystemCPUSize_24)
			cpuSize = eSystemCPUSize_16;
		else if (calcCPUSize >= eSystemCPUSize_24 && calcCPUSize < eSystemCPUSize_32)
			cpuSize = eSystemCPUSize_24;
		else if (calcCPUSize >= eSystemCPUSize_32)
			cpuSize = eSystemCPUSize_32;
		else
			cpuSize = eSystemCPUSize_illegal;

		FTRACESTR(eLevelInfoNormal) << "\n int GetCpuSizeInRange" << cpuSize;
		return cpuSize;




}

eSystemRamSize GetRamSizeAccordingToTotalMemory(string theCaller)
{
  eSystemRamSize ramSize = eSystemRamSize_illegal;

  int totalMemory = GetTotalMemory();
  if (RAM_SIZE_HALF >= totalMemory)
    ramSize = eSystemRamSize_half;
  else if (RAM_SIZE_01GB >= totalMemory)
    ramSize = eSystemRamSize_full_1;
  else if (RAM_SIZE_02GB >= totalMemory)
    ramSize = eSystemRamSize_full_2;
  else if (RAM_SIZE_03GB >= totalMemory)
    ramSize = eSystemRamSize_full_3;
  else if (RAM_SIZE_04GB >= totalMemory)
    ramSize = eSystemRamSize_full_4;
  else if (RAM_SIZE_05GB >= totalMemory)
    ramSize = eSystemRamSize_full_5;
  else if (RAM_SIZE_06GB >= totalMemory)
    ramSize = eSystemRamSize_full_6;
  else if (RAM_SIZE_07GB >= totalMemory)
    ramSize = eSystemRamSize_full_7;
  else if (RAM_SIZE_08GB >= totalMemory)
    ramSize = eSystemRamSize_full_8;
  else if (RAM_SIZE_09GB >= totalMemory)
    ramSize = eSystemRamSize_full_9;
  else if (RAM_SIZE_10GB >= totalMemory)
    ramSize = eSystemRamSize_full_10;
  else if (RAM_SIZE_11GB >= totalMemory)
    ramSize = eSystemRamSize_full_11;
  else if (RAM_SIZE_12GB >= totalMemory)
    ramSize = eSystemRamSize_full_12;
  else if (RAM_SIZE_13GB >= totalMemory)
    ramSize = eSystemRamSize_full_13;
  else if (RAM_SIZE_14GB >= totalMemory)
    ramSize = eSystemRamSize_full_14;
  else if (RAM_SIZE_15GB >= totalMemory)
    ramSize = eSystemRamSize_full_15;
  else if (RAM_SIZE_16GB >= totalMemory)
    ramSize = eSystemRamSize_full_16;
  else if (RAM_SIZE_17GB >= totalMemory)
    ramSize = eSystemRamSize_full_17;
  else if (RAM_SIZE_18GB >= totalMemory)
    ramSize = eSystemRamSize_full_18;
  else if (RAM_SIZE_19GB >= totalMemory)
    ramSize = eSystemRamSize_full_19;
  else if (RAM_SIZE_20GB >= totalMemory)
    ramSize = eSystemRamSize_full_20;
  else if (RAM_SIZE_21GB >= totalMemory)
    ramSize = eSystemRamSize_full_21;
  else if (RAM_SIZE_22GB >= totalMemory)
    ramSize = eSystemRamSize_full_22;
  else if (RAM_SIZE_23GB >= totalMemory)
    ramSize = eSystemRamSize_full_23;
  else if (RAM_SIZE_24GB >= totalMemory)
    ramSize = eSystemRamSize_full_24;
  else if (RAM_SIZE_25GB >= totalMemory)
    ramSize = eSystemRamSize_full_25;
  else if (RAM_SIZE_26GB >= totalMemory)
    ramSize = eSystemRamSize_full_26;
  else if (RAM_SIZE_27GB >= totalMemory)
    ramSize = eSystemRamSize_full_27;
  else if (RAM_SIZE_28GB >= totalMemory)
    ramSize = eSystemRamSize_full_28;
  else if (RAM_SIZE_29GB >= totalMemory)
    ramSize = eSystemRamSize_full_29;
  else if (RAM_SIZE_30GB >= totalMemory)
    ramSize = eSystemRamSize_full_30;
  else if (RAM_SIZE_31GB >= totalMemory)
    ramSize = eSystemRamSize_full_31;
  else if (RAM_SIZE_32GB >= totalMemory)
    ramSize = eSystemRamSize_full_32;
  else
    ramSize = eSystemRamSize_illegal;

  FTRACEINTO << "Caller: " << theCaller
             << ", Total Memory: " << totalMemory
             << ", RamSize: " << ::GetSystemRamSizeStr(ramSize);

  return ramSize;
}

// This function calculates the following ( (FreePhysicalMemory+Cached)/TotalMemory )
// There is no paging in our system so the only available memory is the
// Physical one.
int GetFreeMemoryPercentage()
{
  std::string answer;
  std::string cmd = MCU_MCMS_DIR+"/Scripts/CalcFreeMemPercentage.sh";
  SystemPipedCommand(cmd.c_str(), answer);
  return ((int)atof(answer.c_str()));
}

BOOL IsHardDiskOk()
{
  std::string   hardDiskPath = (IsTarget() ? MCU_OUTPUT_DIR : ".");
  struct statfs buf;
  int           res = statfs(hardDiskPath.c_str(), &buf);
  int           errCode = errno;

  BOOL isHD = (-1 != res && buf.f_blocks != 0
               ?
               TRUE : FALSE);
  return isHD;
}

STATUS GetHDTemperature(std::string& temp)
{
  if (IsTarget())
  {
    std::stringstream command, trace;
    std::string       answer;
    std::string cmd;
    // Check if HD is mounted on /dev/hdb (PATA)
    cmd = "mount | grep "+MCU_OUTPUT_DIR+" | grep /dev/hdb ";
    STATUS stat =
      SystemPipedCommand(cmd.c_str(), answer, FALSE,
                         FALSE);
    if (answer != "")
      command <<"smartctl -a /dev/hdb | grep Temperature | awk '{ print $10 }'";
    else
      command <<
      "smartctl -a -d ata /dev/sda | grep Temperature | awk '{ print $10 }'";

    stat = SystemPipedCommand((command.str()).c_str(), answer, FALSE, FALSE);
    temp = answer;

    FTRACEINTO << "GetHDTemperature - " << answer << " " << stat;
  }
  else
    temp = "33";

  return STATUS_OK;
}

CCpuTemperatureControl*  InitializeCpuTemperatureControl(eProductType prodcutType,
														eCpuManufacturerType cpuManufacturerType, std::string cpuType)
{
	const string cpuTypeCorei3 = "Core(TM) i3";
	const string cpuTypeCorei5 = "Core(TM) i5 CPU";
	const string cpuTypeCelron2 = "P4505";

	if (cpuManufacturerType == eCpuManufacturerTypePortwell)
	{
		if((cpuType.find(cpuTypeCorei3) != string::npos) ||
			(cpuType.find(cpuTypeCelron2) != string::npos)||
			(cpuType.find(cpuTypeCorei5) != string::npos))
		{

			return new CCpuTemperatureControlCpuType(prodcutType, cpuManufacturerType);
		}
	}

	switch (prodcutType)
	{
		case eProductTypeRMX1500:
			return new CCpuTemperatureControlRMX1500(prodcutType, cpuManufacturerType);
		case eProductTypeRMX4000:
			return new CCpuTemperatureControlRMX4000(prodcutType, cpuManufacturerType);
		case eProductTypeRMX2000:
		case eProductTypeSoftMCU:
		case eProductTypeSoftMCUMfw:
		case eProductTypeGesher:
		case eProductTypeNinja:
		case eProductTypeEdgeAxis:
			return new CCpuTemperatureControlRMX2000(prodcutType, cpuManufacturerType);
		default:
			return new CCpuTemperatureControlUnknown(prodcutType, cpuManufacturerType);
	}
	return new CCpuTemperatureControlUnknown(prodcutType, cpuManufacturerType);

}

void GetBiosDate(std::string& answer)
{
  if (SkipOperation())
    return;

  SystemPipedCommand("echo -n `cat /sys/class/dmi/id/bios_date`", answer);
}

void GetBiosVersion(std::string& answer)
{
  if (SkipOperation())
    return;

  SystemPipedCommand("echo -n `cat /sys/class/dmi/id/bios_version`", answer);
}

void GetBiosVendor(std::string& answer)
{
  if (SkipOperation())
    return;

  SystemPipedCommand("echo -n `cat /sys/class/dmi/id/bios_vendor`", answer);
}

// Description: Get ipV4 address (first address configured)
// Return code:
// 0                   - success
// non zero value      - error
STATUS RetrieveIpAddressConfigured_IpV4(char* pOutAddress, std::string& retStr)
{
  int i;
  int status;
  int prioIndex   = 0;

  char addrBuf_ipv4[IP_ADDRESS_STR_LEN];

  memset(addrBuf_ipv4, 0, IP_ADDRESS_STR_LEN);

  std::string retStr_ipv4;

  struct sockaddr_in*  sin;
  struct sockaddr_in6* sin6;
  struct ifaddrs  ifAddrs;
  struct ifaddrs* pIfAddrs = &ifAddrs;
  struct ifaddrs* pIfa = NULL;

  status = getifaddrs(&pIfAddrs);
  if (status) {
    return STATUS_FAIL;
  }

  bool isIpFound = false;
  for (pIfa = pIfAddrs; pIfa; pIfa = pIfa->ifa_next)
  {
    sin = (struct sockaddr_in*) pIfa->ifa_addr;

    switch (sin->sin_family)
    {
      // ===== 1. IPV4
      case AF_INET:
      {
        if (strstr(pIfa->ifa_name, "eth0"))
        {
          isIpFound = true;
          strncpy(addrBuf_ipv4, inet_ntoa(
                    sin->sin_addr), sizeof(addrBuf_ipv4) - 1);
          addrBuf_ipv4[sizeof(addrBuf_ipv4) - 1] = 0;

          retStr_ipv4 =
            "\nRetrieveIpAddressConfigured_IpV4 - address has been reached";
          retStr_ipv4 += "\nAddress: ";
          retStr_ipv4 += inet_ntoa(sin->sin_addr);
          break;
        } // if eth0

        break;
      } // case AF_INET

      default:
      {
        break;
      }
    } // end switch (sin->sin_family)

    if (isIpFound)
      break;
  } // loop over 'pIfa's

  // ===== 3. prepare the returned address
  if ('\0' != addrBuf_ipv4[0])
  {
    retStr = retStr_ipv4;
    strncpy(pOutAddress, addrBuf_ipv4, IP_ADDRESS_STR_LEN);
  }

  freeifaddrs(pIfAddrs);

  return STATUS_OK;
}

// Description: Get ipV6 address (first Global address, if configured; otherwise - first Site address)
// Return code:
// 0                   - success
// non zero value      - error
STATUS RetrieveIpAddressConfigured_IpV6(IpV6AddressMaskS pOutAddress[],
                                        std::string& retStr,
                                        std::string nic_name)
{
  int i;
  int status;
  int prioIndex   = 0;

  BOOL bManualIPv6Config = FALSE;
  char dstBuf[IPV6_ADDRESS_LEN];
  char addrBuf_site[IPV6_ADDRESS_LEN];
  char addrBuf_global[IPV6_ADDRESS_LEN];
  char addrBuf_local[IPV6_ADDRESS_LEN];

  int globalMask = 64,
      siteMask = 64,
      localMask = 64;

  memset(dstBuf, 0, IPV6_ADDRESS_LEN);
  memset(addrBuf_site, 0, IPV6_ADDRESS_LEN);
  memset(addrBuf_global, 0, IPV6_ADDRESS_LEN);
  memset(addrBuf_local, 0, IPV6_ADDRESS_LEN);

  std::string retStr_site, retStr_global, retStr_local, retStrOther;

  struct sockaddr_in*  sin;
  struct sockaddr_in6* sin6;

  struct ifaddrs  ifAddrs;
  struct ifaddrs* pIfAddrs = &ifAddrs;
  struct ifaddrs* pIfa = NULL;


  status = getifaddrs(&pIfAddrs);
  if (status) {
    return STATUS_FAIL;
  }

  bool isGlobalIpFound = false;
  for (pIfa = pIfAddrs; pIfa; pIfa = pIfa->ifa_next)
  {
    if (bManualIPv6Config)
      break;

    sin = (struct sockaddr_in*) pIfa->ifa_addr;

    switch (sin->sin_family)
    {
      // ===== 2. IPV6
      case AF_INET6:
      {
        if (strstr(pIfa->ifa_name, nic_name.c_str()))
        {
          sin6 = (struct sockaddr_in6*) pIfa->ifa_addr;
          inet_ntop(AF_INET6, sin6->sin6_addr.s6_addr, dstBuf, IPV6_ADDRESS_LEN);

          char prefix[IPV6_ADDRESS_LEN];
          memset(prefix, '\0', IPV6_ADDRESS_LEN);
          struct sockaddr_in6* netmask =
            (struct sockaddr_in6*) pIfa->ifa_netmask;
          int len = ipv6_prefix_length(&netmask->sin6_addr);

          eIPv6AddressScope curAddressScope = GetIPv6AddressScope(dstBuf);

          // ---------- 'Local' priority address ----------
          if (eIPv6AddressScope_linkLocal == curAddressScope)
          {
            strncpy(addrBuf_local, dstBuf, IPV6_ADDRESS_LEN);
            localMask = len;

            retStr_local =
              "\nRetrieveIpAddressConfigured_IpV6 - Local priority address has been reached";
            retStr_local += "\nAddress: ";
            retStr_local += dstBuf;

            FTRACESTR(eLevelInfoNormal) << retStr_local;
            break;
          }

          // ---------- 'Site' priority addresses ----------
          else if (eIPv6AddressScope_siteLocal == curAddressScope)
          {
            strncpy(addrBuf_site, dstBuf, IPV6_ADDRESS_LEN);
            siteMask = len;

            retStr_site =
              "\nRetrieveIpAddressConfigured_IpV6 - Site priority address has been reached";
            retStr_site += "\nAddress: ";
            retStr_site += dstBuf;

            FTRACESTR(eLevelInfoNormal) << retStr_site;
            break;
          }

          // ---------- 'Global' priority addresses ----------
          else if ((eIPv6AddressScope_global == curAddressScope) ||
                   (eIPv6AddressScope_uniqueLocalUnicast == curAddressScope))
          {
            isGlobalIpFound = true;
            strncpy(addrBuf_global, dstBuf, IPV6_ADDRESS_LEN);
            globalMask = len;

            retStr_global =
              "\nRetrieveIpAddressConfigured_IpV6 - Global priority address has been reached";
            retStr_global += "\nAddress: ";
            retStr_global += dstBuf;
            retStr_global += "/";

            // Maximum value of len is 128
            char mask[4];
            snprintf(mask, ARRAYSIZE(mask), "%d", len);
            retStr_global += mask;

            FTRACESTR(eLevelInfoNormal) << retStr_global;
            break;
          }

          // ---------- Other type of address ----------
          else
          {
            retStrOther =
              "\nRetrieveIpAddressConfigured_IpV6 - address from type ";
            retStrOther += GetIPv6AddressScopeStr(curAddressScope);
            retStrOther += " is found";
            retStrOther += "\nAddress: ";
            retStrOther += dstBuf;

            FTRACESTR(eLevelInfoNormal) << retStrOther;
            break;
          }
        } // if eth0

        break;
      } // case AF_INET6

      default:
      {
        break;
      }
    } // end switch (sin->sin_family)
  } // loop over 'pIfa's

  // ===== 3. prepare the returned address
  if ('\0' != addrBuf_global[0])
  {
    retStr = retStr_global;
    pOutAddress[0].Set(addrBuf_global, globalMask);

    if ('\0' != addrBuf_site[0])
      pOutAddress[1].Set(addrBuf_site, siteMask);

    if ('\0' != addrBuf_local[0])
      pOutAddress[2].Set(addrBuf_local, localMask);
  }

  else if ('\0' != addrBuf_site[0])
  {
    retStr = retStr_site;
    pOutAddress[0].Set(addrBuf_site, siteMask);
  }

  else if ('\0' != addrBuf_local[0])
    retStr = retStr_local;

  else
    retStr = retStrOther;

  freeifaddrs(pIfAddrs);

  return STATUS_OK;
}

eIPv6AddressScope GetIPv6AddressScope(const char* pAddress)
{
  eIPv6AddressScope retAddressScope = eIPv6AddressScope_other;
  if (!pAddress)
  {
    FPASSERTMSG(true, "GetIPv6AddressScope - address is NULL");
    return retAddressScope;
  }

  // set the begin pointer for scanning the string
  DWORD dBeginScan = (DWORD)pAddress;
  if ('[' == pAddress[0]) // address with brackets

    dBeginScan++;   // advance to after the opening bracket

  char* pBeginScan = (char*)dBeginScan;

  // ===== Unique Local Unicast format: 1111 110...
  // f    c/d
  if (!strncmp(pBeginScan, "fc", 2) ||
      !strncmp(pBeginScan, "fd", 2))
    retAddressScope = eIPv6AddressScope_uniqueLocalUnicast;

  // ===== Link local format: 1111 1110 1000 0000 0000...
  // f    e    8    0
  else if (!strncmp(pBeginScan, "fe80::", 6))
    retAddressScope = eIPv6AddressScope_linkLocal;

  // ===== Site local format: 1111 1110 11...
  // f    e    c/d/e/f
  else if (!strncmp(pBeginScan, "fec", 3) ||
           !strncmp(pBeginScan, "fed", 3) ||
           !strncmp(pBeginScan, "fee", 3) ||
           !strncmp(pBeginScan, "fef", 3))
    retAddressScope = eIPv6AddressScope_siteLocal;

  // ===== Multicast format: 1111 1111...
  // f    f
  else if (!strncmp(pBeginScan, "ff", 2))
    retAddressScope = eIPv6AddressScope_multicast;

  // ===== Loopback format: 0000...
  // 0
  else if (!strncmp(pBeginScan, "0", 1))
    retAddressScope = eIPv6AddressScope_loopBack;

  // ===== Global format: 001...
  // 2/3
  // if ( !strncmp(pBeginScan, "2", 1) ||
  // !strncmp(pBeginScan, "3", 1) )
  else
    retAddressScope = eIPv6AddressScope_global;

  return retAddressScope;
}
eIPv6AddressScope ConvertenScopeIdToeIpv6AddressScope(enScopeId scopeIdType)
{
	eIPv6AddressScope curScope = eIPv6AddressScope_other;
	switch(scopeIdType)
	{
		case eScopeIdGlobal:	 curScope = eIPv6AddressScope_global;
								 break;
		case eScopeIdLink:		 curScope = eIPv6AddressScope_linkLocal;
								 break;
		case eScopeIdSite:		 curScope = eIPv6AddressScope_siteLocal;
								 break;
		default : 			curScope = eIPv6AddressScope_other;

	}
	return curScope;
}
// Old method for retrieving Addresses' scope
enScopeId getScopeId(char* pIPv6)
{
  enScopeId retScopeIdType = eScopeIdOther;

  eIPv6AddressScope curScope = GetIPv6AddressScope(pIPv6);
  switch (curScope)
  {
    case eIPv6AddressScope_global:
    {
      retScopeIdType = eScopeIdGlobal;
      break;
    }
    case eIPv6AddressScope_linkLocal:
    {
      retScopeIdType = eScopeIdLink;
      break;
    }
    case eIPv6AddressScope_siteLocal:
    {
      retScopeIdType = eScopeIdSite;
      break;
    }
    default:
    {
      retScopeIdType = eScopeIdOther;
      break;
    }
  } // switch

  return retScopeIdType;
}

std::string ConvertIpAddressToCIDRNotation(const std::string ipAddress,
                                           const DWORD mask)
{
  std::string result = "";

  const DWORD dipAddress = SystemIpStringToDWORD(ipAddress.c_str());
  return ConvertIpAddressToCIDRNotation(dipAddress, mask);
}

std::string ConvertIpAddressToCIDRNotation(const std::string ipAddress,
                                           const std::string mask)
{
  std::string result = "";

  const DWORD dipAddress = SystemIpStringToDWORD(ipAddress.c_str());
  const DWORD dwMask =  SystemIpStringToDWORD(mask.c_str());

  return ConvertIpAddressToCIDRNotation(dipAddress, dwMask);
}

std::string ConvertIpAddressToCIDRNotation(const DWORD dipAddress,
                                           const DWORD mask)
{
  std::string result = "";

  DWORD cidrAddress = dipAddress & mask;
  char  stCidrAddress[16] = "";
  SystemDWORDToIpString(cidrAddress, stCidrAddress);

  DWORD cidrMask = 0;
  DWORD temp = mask;
  DWORD msb = temp && 0x80000000;
  while (cidrMask < 32 && msb)
  {
    cidrMask++;
    temp = temp << 1;
    msb = temp && 0x80000000;
  }

  char stResult[24] = "";
  snprintf(stResult, sizeof(stResult), "%s/%d", stCidrAddress, cidrMask);

  result = stResult;
  return result;
}

pid_t CollectZombieChildProcesses()
{
  return waitpid(-1, 0, WNOHANG);
}

void RemoveSpaceAndHyphenFromString(char* str)
{
  if (str == NULL)
    return;

  char output[IP_STRING_LEN];
  memset(output, '\0', sizeof(output));

  char* p = output;
  char* q = str;
  while (*str != '\0')
  {
    if (*str == ' ' || *str == '-')
    {
      str++;
      continue;
    }

    *p++ = *str++;
  }

  int i = 0;
  while (output [i] != '\0')
    *q++ = output[i++];

  *q = '\0';
}

void PrintErrorToLocalFile(std::ostringstream& cmd, BOOL addDate)
{
	std::string strFileName = MCU_OUTPUT_TMP_DIR+"/LogBackup.txt";
	if (!IsTarget())
		strFileName = MCU_TMP_DIR+"/LogBackup.txt";
	CStructTm localTime;
	SystemGetTime(localTime);

	//////////////////////
	char    str_time [58];
	memset(str_time,0,58);

	snprintf(str_time,sizeof(str_time),"%02d.%02d %02d:%02d:%02d.%07u :",localTime.m_day, localTime.m_mon,localTime.m_hour,localTime.m_min,localTime.m_sec,0);
	std::string strLine = str_time;
	if(addDate)
	{
		strLine = str_time;
	}
	cmd<< "\n";
	strLine = strLine + cmd.str();
	if(GetFileSize(strFileName.c_str()) > 500*1024*1024)
	{
		std::string newFullName = MCU_OUTPUT_TMP_DIR+"/LogBackup.bck";

		RenameFile(strFileName, newFullName);
	}

    PrintErrorToLocalFile(strFileName,strLine.c_str());
}

void PrintErrorToLocalFile(std::string strFileName,std::string cmd)
{
    std::ofstream file;
    file.open(strFileName.c_str(), ios::app );
    if (file.is_open())
    {
          file << cmd.c_str() << flush;
          file.close();
    }
}

void PrintErrorToLocalFile(std::string strFileName,std::ostringstream& cmd)
{
    PrintErrorToLocalFile(strFileName,cmd.str());
}

int strcpy_safe_helper(char* dst, size_t size, const char* src)
{
  if (size < 1)
    return ERANGE;

  if (NULL == src)
  {
    dst[0] = '\0';
    return EINVAL;
  }

  strncpy(dst, src, size-1);
  dst[size-1] = '\0';

  return 0;
}

int strcpy_safe(char* dst, size_t size, const char* src)
{
  if (NULL == dst)
    return EINVAL;

  return strcpy_safe_helper(dst, size, src);
}

STATUS GetNICFromIpAddress(const char* pIpAddress, std::string& NicName)
{
	struct ifaddrs *addrs = NULL, *ifa = NULL;
	struct sockaddr_in *sa = NULL;
	struct sockaddr_in6 *sa6 = NULL;
	char buf[IPV6_ADDRESS_LEN] = {0};
	STATUS ret = STATUS_FAIL;

	if(NULL != pIpAddress && 0 != getifaddrs(&addrs)) goto done;

	for (ifa = addrs; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr){
			if(ifa->ifa_addr->sa_family == AF_INET) {
				sa = (struct sockaddr_in *)(ifa->ifa_addr);
				inet_ntop(ifa->ifa_addr->sa_family, (void *)&(sa->sin_addr), buf, sizeof(buf));
				if (!strcmp(pIpAddress, buf)) {
					NicName = ifa->ifa_name;
					ret = STATUS_OK;
				}
			}
			else if(ifa->ifa_addr->sa_family == AF_INET6) {
				sa6 = (struct sockaddr_in6 *)(ifa->ifa_addr);
				inet_ntop(ifa->ifa_addr->sa_family, (void *)&(sa6->sin6_addr), buf, sizeof(buf));
				if (!strcmp(pIpAddress, buf)) {
					NicName = ifa->ifa_name;
					ret = STATUS_OK;
				}
			}
		}
	}

done:

	if(NULL != addrs){
		freeifaddrs(addrs);
		addrs = NULL;
	}

	return ret;
}


std::string formatString(const char* format, ...) {
 char buffer[MAX_SIZE_FORMATED_STRING]={0};
 va_list vl;
 va_start(vl, format);
 vsnprintf(buffer, MAX_SIZE_FORMATED_STRING, format, vl);
 buffer[MAX_SIZE_FORMATED_STRING-1] =0;
 return string(buffer);
}

bool ValidateHexString(const char* pcsz)
{
	bool bValid = false;
	if ( pcsz && strlen(pcsz) > 2  &&  pcsz[0] == '0' && toupper(pcsz[1]) == 'X' )
	{
		bValid = true;
		for (unsigned int i = 2; i < strlen(pcsz) && bValid; i++ )
		{
			bValid = (isxdigit(pcsz[i]) != 0);
		}
	}
	return bValid;
}
