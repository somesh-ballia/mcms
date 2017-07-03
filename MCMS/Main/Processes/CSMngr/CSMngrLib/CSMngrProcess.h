// CSMngrProcess.h: interface for the CCSMngrProcess class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner
//========   ==============   =====================================================================

#if !defined(_CSMngrPROCESS_H__)
#define _CSMngrPROCESS_H__

#include "ProcessBase.h"
#include "SharedMcmsCardsStructs.h"
#include "ObjString.h"
#include "PingData.h"
#include "IpService.h"
#include "CsMngrInternalStructs.h"
#include "CSMngrDefines.h"
#include "Versions.h"

class CIPService;
class CIPServiceList;
class CIPServiceFullList;
class CCommConfService;
class CCommSnmpService;
class CSystemInterfaceList;
class CRequst;

enum eSignalingTaskStateType
{
	eSignalingTaskOk		= 0,
	eSignalingTaskZombie,

	NumOfSignalingTaskTypes
};

enum eSystemMode
{
	eSystemMode_Multiple_services = 0,
	eSystemMode_Jitc_v35,
	eSystemMode_None
};



class CCSMngrProcess : public CProcessBase
{
CLASS_TYPE_1(CCSMngrProcess,CProcessBase )
public:
	friend class CTestCSMngrProcess;

	CCSMngrProcess();
	virtual ~CCSMngrProcess();
	const char * NameOf(void) const {return "CCSMngrProcess";}
	virtual eProcessType GetProcessType() {return eProcessCSMngr;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
	virtual void AddExtraStringsToMap();

	void AddNewIpService(CIPService &ipService);
	CIPServiceList* GetIpServiceListStatic()const	{return m_pIpServiceListStatic;}
	CIPServiceList* GetIpServiceListDynamic()const	{return m_pIpServiceListDynamic;}
	CIPServiceFullList* GetXMLWraper()const			{return m_XMLWraper;}
	DWORD GetLicensingMaxNumOfParties()const   		{return m_LicensingMaxNumOfParty;}
	void SetLicensingMaxNumOfParties(DWORD num);
	bool IsLicensingReceived()const;

	static void TraceToLogger(const char *location, const char *action, STATUS status);

    STATUS FixPortRange(CIPService &ipService);
//     bool FixTcpPortRange(CIPServiceList &ipServiceList);

    BOOL GetIsDebugMode() const;
    DWORD GetCsIp()const;
    DWORD GetGkIp()const;
    virtual bool IsFailoverBlockTransaction_SlaveMode(string sAction);

    void					SetSysIpType(eIpType theType);
    eIpType					GetSysIpType();
    void                    SetIpTypeReceivedStatus(BOOL status){m_IpTypeReceivedStatus = status;};
    BOOL                    GetIpTypeReceivedStatus(){return m_IpTypeReceivedStatus;};
    void					SetSysIPv6ConfigType(eV6ConfigurationType theType);
    eV6ConfigurationType	GetSysIPv6ConfigType();

    void					SetMngmntAddress_IPv4(DWORD theAddress);
    DWORD					GetMngmntAddress_IPv4();
    void					SetMngmntAddress_IPv6(int idx, string theAddress);
    string					GetMngmntAddress_IPv6(int idx);

	STATUS ValidateDuplicateSpanDefinition(CIPServiceList *pIpServiceList, CIPService *pNewService, CIPService *pOldService = NULL);
	STATUS ValidateDuplicateIpAddrCS(CIPService *pService, CObjString &errorMsg);
	STATUS ValidateV35GwAlreadyConfigured(string servName);

    virtual int GetProcessAddressSpace() {return 96 * 1024 * 1024;}
    //Ping
    void SetPing (CPingData* newPingData);
    CPingData* GetPing ();
    void DeletePing();

    static WORD GetMaxNumOfCSTasks(void);
    static WORD CheckAndFixCSID(WORD csID, OPCODE opcode);

	COsQueue* GetSignalingMbx(WORD index);
	eSignalingTaskStateType GetSignalingTaskState(WORD index);
	void AddToSignalingTasksList(COsQueue* mbx, WORD index);
	void TurnSignalingTaskToZombie(COsQueue* mbx);


	void RetrieveIPv6AddressesInAutoMode(eIpType ipType);

	eCsMultipleServiceMode GetIsMultipleServices();
	void SetIsMultipleServices(eCsMultipleServiceMode isMultipleServices);
	BYTE GetIsV35JITCSupport();

	void GetSignalingMasterBoardId(const DWORD csId, DWORD& board_id, DWORD& sub_board_id);
	eConfigInterfaceType GetSignalingInterfaceType(const DWORD csId);

	void SetIpv6Params( DWORD ipv4Add,
					    string ipv6Add_0,
						string ipv6Add_1,
						string ipv6Add_2,
						string ipv6_defGw,
						DWORD ipv6Mask_0,
						DWORD ipv6Mask_1,
						DWORD ipv6Mask_2,
						DWORD ipv6Mask_defGw,
						CCommConfService *pCommConfService,
						CCommSnmpService *pCommSnmpService,
						BOOL bForceDefGwUpdate);

	DWORD GetServiceIdFromDynamicList(DWORD board_id, DWORD sub_board_id);

	virtual DWORD GetMaxTimeForIdle(void) const
	{
	  return 12000;
	}

	void SetMngmntDefaultGatewayMaskIPv6(const DWORD mask);
	void SetMngmntDefaultGatewayIPv6(const string Address);

	const string GetMngmntDefaultGatewayIPv6();
	DWORD GetMngmntDefaultGatewayMaskIPv6();

	BYTE GetIpServiceFileNames(std::string &ipServiceFileName, std::string &ipServiceFileNameTmp);

	void        SetCSIpConfigMasterBoardId(int serviceId,DWORD boardId);
	 DWORD GetCSIpConfigMasterBoardId(int serviceId) const;
	void        SetCSIpConfigMasterPqId(int serviceId,DWORD pqId);
	 DWORD GetCSIpConfigMasterPqId(int serviceId) const;

	 CSystemInterfaceList* GetSystemInterfaceList();//const; 
     //{ return m_pSysInterfaceList; }

	 void ReadSystemInterfaceList();

	 STATUS ValidateSwMcuFields(CIPService *pUpdatedService);
	 STATUS ValidateMFWFields(CIPService* pUpdateService, CRequest *pRequest);

	 STATUS ValidateRouterDetails(CIPService *pService, CIPService *pUpdatedService);
	 STATUS ValidateDnsDetails(CIPService *pService, CIPService *pUpdatedService);
	 STATUS ValidateIpAddressAndInterface(DWORD ipv4Address, DWORD UpdatedIpv4Address, string interface, string UpdatedInterface);
	 STATUS ValidateSubnetMaskDetails(DWORD netMask, DWORD updatedNetmask);
	 STATUS ValidateV35Details(CIPService *pService, CIPService *pUpdatedService);
	 void SetMngmntDnsIpV4Address(char ipAddressStr[IP_ADDRESS_LEN]);
	 void SetMngmntDnsIpV6Address(char ipAddressStr[IPV6_ADDRESS_LEN]);
	 void SetMngmntDnsStatus(BOOL dnsStatus);
	 BOOL GetDnsStatus()const {return m_dnsStatus;}
	 char* GetDnsIpV4Address() {return m_ipv4AddressStr;}
	 char* GetDnsIpV6Address() {return m_ipv6AddressStr;}
	 VERSION_S GetMcuVersion() {CVersions ver; std::string versionFilePath = VERSIONS_FILE_PATH; ver.ReadXmlFile(versionFilePath.c_str()); return ver.GetMcuVersion();}


protected:
	virtual void SetUpProcess();
	virtual void AddExtraOpcodesStrings();
	virtual void AddExtraStatusesStrings();



private:
	void TraceMessage(const char *location, const char *action, STATUS status);

	void SetMFWIpServiceExtDiscription(STATUS nStatus, std::string strServiceName, CRequest *pRequest, CIPSpan *pIpSpan);

//	void RemoveFromSignalingTasksList(WORD csId);
//	void RemoveFromSignalingTasksList(COsQueue* mbx);



	CIPServiceList 		*m_pIpServiceListStatic;
	CIPServiceList 		*m_pIpServiceListDynamic;
	CIPServiceFullList	*m_XMLWraper; // contains m_update_cnt for dynamic part of services
	DWORD m_LicensingMaxNumOfParty;
    BOOL                  m_IpTypeReceivedStatus;
	eIpType					m_sysIpType;
	eV6ConfigurationType	m_sysIPv6ConfigType;
	DWORD					m_MngmntAddress_IPv4;
	string					m_MngmntAddress_IPv6[MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES];
	string					m_MngmntDefaultGatewayIPv6;
	DWORD					m_MngmntDefaultGatewayMaskIPv6;


	COsQueue*          		m_pSignalingTasksMbxs_List[MAX_NUMBER_OF_SERVICES_IN_RMX_4000];
	eSignalingTaskStateType m_SignalingTasksState_List[MAX_NUMBER_OF_SERVICES_IN_RMX_4000];

	eCsMultipleServiceMode					m_isMultipleServices;

    //current ping data
    CPingData* 				m_pCurrentPing;//[MAX_NUMBER_OF_SERVICES_IN_RMX_4000];

    CS_IP_CONFIG_MS_PARAMS_S         m_MasterBoardIdPerService[MAX_NUM_OF_IP_SERVICES+1];

    CSystemInterfaceList*	m_pSysInterfaceList;


    char m_ipv4AddressStr[IP_ADDRESS_LEN];
    BOOL m_dnsStatus;
    char m_ipv6AddressStr[IPV6_ADDRESS_LEN];

};

#endif // !defined(_CSMngrPROCESS_H__)
