// SystemFunctions.h

#ifndef SYSTEM_FUNCTIONS_H_
#define SYSTEM_FUNCTIONS_H_

#include <string>
#include "DataTypes.h"
#include "SystemTick.h"
#include "SharedDefines.h"
#include "IpAddressDefinitions.h"
#include "CommonStructs.h"
#include "DefinesGeneral.h"
#include "McuMngrInternalStructs.h"


class CCpuTemperatureControl;
#define  MAX_SHM_NAME 100
#define MAX_SIZE_FORMATED_STRING 2048

typedef struct
{
    UINT32      validFlag;
    UINT32      dataLen;
    OPCODE      opcode;
} MESSAGE_FILE_HEADER_S;


// Inside MCMS IPs must be stored inside DWORDs in a little endian (LE) format.
// for socket's functions use big indian format.
enum eIpPresentationType
{
	eNetwork,   // Big Indian
				//	"1.2.3.4" -> value = 0x04030201
				//			    memory= 01020304
	eHost		// Little Indian
				// "1.2.3.4" -> value = 0x01020304
				//			    memory= 04030201
};

enum eHashMethod
{
	eHashMethodSHA1,
	eHashMethodSHA256
};

// IPv6AddressScope
enum eIPv6AddressScope
{
	eIPv6AddressScope_linkLocal,
	eIPv6AddressScope_siteLocal,
	eIPv6AddressScope_global,
	eIPv6AddressScope_multicast,
	eIPv6AddressScope_loopBack,
	eIPv6AddressScope_uniqueLocalUnicast,
	eIPv6AddressScope_other,

	NUM_OF_IPV6_ADDRESS_SCOPES
};

enum eCpuManufacturerType
{
	eCpuManufacturerTypeUnknown     = -1, // use this for uninitialized data members
	eCpuManufacturerTypePortwell = 0,
	eCpuManufacturerTypeFintec = 1
};

static const char *ipv6AddressScopeStr[] =
{
	"linkLocal",
	"siteLocal",
	"global",
	"multicast",
	"loopBack",
	"uniqueLocalUnicast",
	"other"
};

static const char *GetIPv6AddressScopeStr(eIPv6AddressScope theScope)
{
  const char* name =
    (((0 <= theScope) && (theScope < NUM_OF_IPV6_ADDRESS_SCOPES))
						?
						ipv6AddressScopeStr[theScope] : "Invalid IPv6 address scope");

	return name;
}

#define GET_LE_IP_BYTE_1(ip) (*(((BYTE*)&ip) + 3))
#define GET_LE_IP_BYTE_2(ip) (*(((BYTE*)&ip) + 2))
#define GET_LE_IP_BYTE_3(ip) (*(((BYTE*)&ip) + 1))
#define GET_LE_IP_BYTE_4(ip) (*(((BYTE*)&ip) + 0))

class CStructTm;

void SetBogmipsValue(int val);
void SetTotalMemory(int val);
void SetCpuSize(int val);
void SystemSleep(TICKS,BOOL freeTaksSemaphore = TRUE);
TICKS SystemGetTickCount();
STATUS SystemAllocateSharedMemory(SM_HANDLE & Shared_memory_Descriptor,
								  BYTE **  newMemory,
								  DWORD size,
								  const char * name,
								  BOOL &isFirst);

STATUS SystemFreeSharedMemory(void *pShared_Memory,SM_HANDLE Shared_Memory_Handle);
STATUS SystemGetTime(CStructTm& t);
BOOL SystemIsBadReadPtr(const void * ptr, unsigned int size);
DWORD SystemIpStringToDWORD(const char * ipAddress, eIpPresentationType type = eHost);
void SystemDWORDToIpString(const DWORD ip, char* ipAddress);

// IpV4-6 New library conversion functions
eIpType IpVersionToIpType(enIpVersion ipVersion);
enIpVersion IpTypeToIpVersion(eIpType ipType);

char* ipV4ToString(const APIU32 ipAddress, char *sIpAddress);
char* ipV6ToString(const APIU8 *ipAddress, char *sIpAddress,BOOL addBrackets);
char* ipV6ToStringProtected(const APIU8* ipAddress, char* sIpAddress, BOOL addBrackets, size_t sizeOfAddr); // // B.S klocwork 2568
char* ipToString(mcTransportAddress	ipAddress,char	*sIpAddress,BOOL addBrackets);
void   stringToIpV4(mcTransportAddress* ipAddress,
                    char* sIpAddress,
                    eIpPresentationType type = eHost);
int  stringToIpV6(mcTransportAddress *ipAddress, const char* sIpAddress);
void   stringToIp(mcTransportAddress* ipAddress,
                  char* sIpAddress,
                  eIpPresentationType type = eHost);
BOOL  isIpV4Str(const char *sIpAddress);
BOOL  isIpV6Str(const char *sIpAddress);
void   SplitIPv6AddressAndMask(char* pInAddress,
                               char* pOutAddress,
                               char* pOutMask);
char* ipV6AndSubnetMaskToString(const APIU8* ipAddress,
								DWORD subNetMask,
								char* outFullIpAddress,
								BOOL addBrackets);

char* ipV6AndSubnetMaskStrToString(const APIU8* ipAddress,
								const char* subNetMaskStr,
								char* outFullIpAddress, // FULL_IPV6_ADDRESS_LEN
								BOOL addBrackets);
void   stringToIpV4(ipAddressStruct* ipAddress,
                   const char* sIpAddress,
                   eIpPresentationType type = eHost);
void  stringToIpV6(ipAddressStruct *ipAddress, char *sIpAddress);
void   stringToIp(ipAddressStruct* ipAddress,
                  char* sIpAddress,
                  eIpPresentationType type = eHost);
void   IPv6RemoveBrackets(const char* strInWithBrackets,
                          char* strOutWithoutBrackets);
char*  ipToString(ipAddressStruct ipAddress,
                  char* sIpAddress,
                  BOOL addBrackets);
int   isApiTaNull(const mcTransportAddress* pApiAddr);
bool   IsIpNull(const ipAddressStruct* pAddr);
bool   IsIpNull(ipAddressV4If addr);
bool   IsIpNull(ipAddressV6If addr);
int    isIpAddressEqual(const mcTransportAddress* firstAddr,
                        const mcTransportAddress* secondAddr);
int   isIpTaNonValid(mcTransportAddress* pApiAddr);
DWORD SystemInitSocket();
void   SystemIpStringToIntArray(const char* ipAddress,
                                unsigned char* Octet_array);
DWORD SystemCleanupSocket();
STATUS SystemRunProcess(int processNumber,
                        const char* processParams = NULL,
                        BOOL giveUpRoot = TRUE,
                        int nice = 0xffff);
STATUS SystemRunCommand(const char* system_command,
                        const char* arg1 = NULL,
                        const char* arg2 = NULL,
                        const char* arg3 = NULL,
                        const char* arg4 = NULL,
                        BOOL giveUpRoot = FALSE,
                        int nice = 0xffff);
void   SystemCoreDump(BOOL continue_running, BOOL only_if_in_debug = FALSE);
void SystemSync();
STATUS SystemPipedCommand(const char* cmd,
                          std::string& out,
                          BOOL free_sem = TRUE,
                          BOOL is_print = TRUE,
                          BOOL is_need_answer = TRUE);

// Dumps file content into trace file
STATUS DumpFile(const std::string & fileName);
STATUS DumpSystemStatistics(std::ostream&);
STATUS GetSelfCpuUsage(TICKS & user, TICKS & system);
STATUS GetChildrenCpuUsage(TICKS & user, TICKS & system);
STATUS GetCurrentTime(time_t &l_time);
BOOL IsTarget();
BOOL IsRmxSimulation();
BOOL IsPatchedVersion();
BOOL SetAddressSpaceLimit(int Limit);
BOOL GetVersions(std::string & running,
                 std::string & current,
                 std::string & fallback,
                 std::string & factory);
void SyncMedia(BOOL freeTaskSemaphre = TRUE);
bool IsUnderValgrind(const char* pname);
int GetUsedMemory(BOOL isToPrint =TRUE);
int GetUsedMemory();
int GetTotalMemory();
int GetFreeMemoryPercentage();
BOOL IsHardDiskOk();

STATUS GetHDTemperature(std::string & temp);


BOOL IsStartupFinished();

CCpuTemperatureControl*  InitializeCpuTemperatureControl(eProductType prodcutType,
														eCpuManufacturerType cpuManufacturerType, std::string cpuType);

void GetBiosVersion(std::string &answer);
void GetBiosVendor(std::string &answer);
void GetBiosDate(std::string &answer);
STATUS RetrieveIpAddressConfigured_IpV4(char *pOutAddress, std::string &retStr);
STATUS RetrieveIpAddressConfigured_IpV6(IpV6AddressMaskS pOutAddress[],
                                        std::string &retStr,
                                        std::string nic_name = "eth0");
eSystemRamSize GetRamSizeAccordingToTotalMemory(std::string theCaller);
bool CheckCpuValueValidity(int cpusize);
WORD GetMachineProfileGrade();
float GetCpuFactor();
float GetBogoMipsFactor();
int GetCpuSizeInRange(float calcCPUSize);
eIPv6AddressScope GetIPv6AddressScope(const char *pAddress);
enScopeId  getScopeId(char* pIPv6); // old method
eIPv6AddressScope ConvertenScopeIdToeIpv6AddressScope(enScopeId scopeIdType);
std::string ConvertIpAddressToCIDRNotation(const std::string ipAddress,
                                           const std::string mask);
std::string ConvertIpAddressToCIDRNotation(const std::string ipAddress,
                                           const DWORD mask);
std::string ConvertIpAddressToCIDRNotation(const DWORD dipAddress,
                                           const DWORD mask);
void	RemoveSpaceAndHyphenFromString(char* str);
pid_t	CollectZombieChildProcesses();
void PrintErrorToLocalFile(std::ostringstream& cmd, BOOL addDate = true);
void PrintErrorToLocalFile(std::string strFileName,std::string cmd);

int strcpy_safe_helper(char* dst, size_t size, const char* src);
int strcpy_safe(char* dst, size_t size, const char* src);
template <size_t size> int strcpy_safe(char (&dst)[size], const char* src)
{
  return strcpy_safe_helper(dst, size, src);
}

std::string formatString(const char* format, ...);

STATUS GetNICFromIpAddress(const char* pIpAddress, std::string& NicName);

bool ValidateHexString(const char* pcsz);
#endif
