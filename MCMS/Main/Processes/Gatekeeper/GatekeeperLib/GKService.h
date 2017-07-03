#ifndef GKSERVICE_H_
#define GKSERVICE_H_

#include <vector>

#include "PObject.h"
#include "DataTypes.h"
#include "DefinesIpService.h"
#include "GkAlias.h"
#include "GkCsReq.h"
#include "GkCsInd.h"
#include "GateKeeperCommonParams.h"
#include "ObjString.h"
#include "IpAddress.h"

#define CALL_SIGNAL_PORT 	1720
#define GK_RAS_PORT			1719
#define PN_IDENT 			"PN:"
#define PN_DMA_IDENT 		"PN:DMA:"

typedef enum
{
	eRegister ,    //after receiving RCF
	eNonRegistered, //after start up
	eUnRegistered,   //after URQ
}eRegistrationStatus;

typedef enum
{
	eNotInProcess,
	eStartFromOriginal,
	eSearchFromAltList,
	eSearchFromAltListWithRegistration,
	eFoundFromAltList,
	eSearchFromAltListAfterOneCycle
}eAltGkProcess;

typedef struct
{
	BYTE			numOfWaiting; 
	ipAddressStruct	primeGkIpFromDns;
	ipAddressStruct	altGkIpFromDns;
}DnsParamsSt;


class CGkAlt : public CPObject
{
	CLASS_TYPE_1(CGkAlt , CPObject)      
public:
	CGkAlt();
	virtual ~CGkAlt(){}
	
	virtual const char* NameOf() const { return "CGkAlt";}
	friend bool operator<(const CGkAlt&,const CGkAlt&);	
	
	alternateGkSt m_altGk;	
};


class CGkService : public CPObject
{
	CLASS_TYPE_1(CGkService , CPObject)      
public:
	CGkService(WORD ServiceId = 0);
	virtual ~CGkService();
	
	virtual const char* NameOf() const { return "CGkService";}
	void InitParam (GkManagerServiceParamsIndStruct *pServiceParamsIndSt);   
	
	gkReqRasGRQ* CreateGRQ(BOOL bIsAVFLicense, int *pStructSize);
	gkReqRasRRQ* CreateRRQ(int bIsPolling, int *pStructSize);
	gkReqRasURQ* CreateURQ(int *pStructSize);
	void CreateARQ(gkReqRasARQ *pPartyARQ);
	void CreateDRQ(gkReqRasDRQ *pPartyDRQ);
	void CreateBRQ (gkReqRasBRQ *pPartyBRQ);
	void CreateIRR(gkReqRasIRR *pPartyIRR);
	void CreateBRQResponse(gkReqBRQResponse *pPartyBRQ);
	void CreateDRQResponse(gkReqDRQResponse *pPartyDRQ);
	gkReqDRQResponse* CreateDRQResponse(gkIndDRQFromGk *pPartyDRQFromGk);
	gkReqURQResponse* CreateURQResponse();
	gkReqLRQResponse* CreateLRQResponse(int hsRas);
	
	void  CreateVendor(mcuVendor *pEndpVendor);
	
	void SetGkRasAddress(mcXmlTransportAddress *pGkRasAddress);
	
	// cs ips
	void SetTransportAddr(CIpAddressPtr csIp, mcXmlTransportAddress* pAddr, WORD port); // ras or signaling address
	void SetRasAddr(mcXmlTransportAddress* pAddr, WORD port);       // ipv4 or ipv6 cs addresses
	void SetSignalingAddresses(mcXmlTransportAddress* pAddr, WORD port, int arr_size); // all cs addresses list
	void SetSignalingAddr(mcXmlTransportAddress* pAddr, WORD port, int index); // first or second signaling address 
	void SetCsAddressesOrder(const ipAddressStruct& pAddr); // set cs addresses order where first in order should match the given address type
	int  GetFirstCsIpIndexAs(const CIpAddressPtr comparedIp) const;

	void SetGkIpToZero();	
	
	void InitAliasTypes(DWORD *pAliasType);
	
	//void  AddGkPrefixToAliasesList(CIPSpan* pIPspan);    //only in PN - in case yael
	BYTE IsGkPathNavigator();
	void ConvertGkIdFromBmpString(char* destBuf); 
	
	void InsertAlias(CGkAlias *pAlias);
	void SetFsParams(h460FsSt *pFs,BOOL bIsKeepAlive=FALSE);
	BYTE CompareToGkAddress(mcXmlTransportAddress *pAddr) const;
	
	// Attributes functions
	// --------------------
	DWORD GetServiceId() const;
	void SetSeviceId(DWORD serviceId); 
	
	const char* GetServiceName() const; 
	void SetServiceName (const char* name);
	
	const char* GetGkName() const; 
	void SetGkName (const char* name);
	
	const char* GetAltGkName() const; 
	void SetAltGkName (const char* name);
	
    const char* GetAliasList() const;
//	void SetAliasList(char *pAliasList);
	
    DWORD* GetAliasType();
	void SetAliasType(DWORD *pAliasType,int len);
	
	BYTE GetIsGkInService();
	void SetIsGkInService(BYTE bIsGkInService);

	BYTE GetIsRegAsGw();
	void SetIsRegAsGw(BYTE bIsRegAsGw);
	
	char *GetPrefix();
	void  SetPrefix(char* pPrefix);
	
	BOOL GetMultcast();
	void SetMultcast(BOOL multcast);
	
	BYTE AreParamsReady() const {return (m_bAreParamsReady == TRUE);}
	void SetParamsReadiness(BYTE bAreParamsReady) {m_bAreParamsReady = bAreParamsReady;}
	
	CIpAddressPtr GetCsRasIp() const;						
	CIpAddressPtr GetCsIp(int index) const;					// index must be between 0-2  
	const ipAddressStruct* GetCsIpsSt() const;				// returns pointer to cs list ip addresses
	void SetCsIpsSt(ipAddressStruct* pAddr, int arr_size);	// sets cs list ip addresses
	
	
	CIpAddressPtr GetGkIp() const;	
	BYTE  IsIpGk() const;
	const ipAddressStruct& GetGkIpSt() const;
	void SetGkIpSt(ipAddressStruct* pAddr);
	void SetGkIpSt(mcXmlTransportAddress* pAddr);
	
	CIpAddressPtr GetGkIpConfigured() const;
	const ipAddressStruct& GetGkIpConfiguredSt() const;	
	void SetGkIpStConfigured(ipAddressStruct *pAddr);
	const char *GetNameGkConfigured() const;
	void SetNameGkConfigured(char *pName);	
	
	CIpAddressPtr GetAltGkIpConfigured() const;
	void SetAltGkIpConfigured(ipAddressStruct* pAddr);
	ipAddressStruct GetAltGkIpStConfigured() const;
	void SetAltGkIpStConfigured(ipAddressStruct *pAddr);
	
	const char *GetNameAltGkConfigured() const;
	void SetNameAltGkConfigured(char *pName);
	
	char *GetGkIdent();
	void SetGkIdent(char *gkIdent,int len);
	
	WORD GetGkIdentLen() const;
	void SetGkIdentLen(WORD len);
	
	char *GetEpIdent();
	void SetEpIdent(char *epIdent,int len);

	WORD GetEpIdentLen() const;
	void SetEpIdentLen(WORD len);

	eRegistrationStatus GetRegStatus() const;
	void SetRegStatus(eRegistrationStatus status);
	
	WORD GetRRJCounter() const;
	void SetRRJCounter(WORD counter);
	
	BOOL GetDiscovery() const;
	void SetDiscovery(BOOL discovery);
	
	WORD GetTimeToLive() const;
	void SetTimeToLive(WORD timeToLive);
    WORD GetConfiguredTimeToLive();
	
	int GetURQhsRas() const;
	void SetURQhsRas(int hsRas);
	
	BOOL GetIsAvaya() const;
	void SetIsAvaya(BOOL bIsAvaya);
	
	eAltGkProcess GetAltGkProcess() const;
	void SetAltGkProcess(eAltGkProcess process);
	
	BYTE IsAltGkPermanent() const;
	void SetAltGkPermanent(BYTE value);
	
	int GetCurrAltGkToSearch() const;
	void SetCurrAltGkToSearch(int curAltGk);

	CIpAddressPtr GetCurAltGkIp() const;
	alternateGkSt *GetCurAltGkSt();
	void SetCurAltGkSt(alternateGkSt *pAltGkSt);

	DWORD GetTriggerOpcode() const;
	void SetTriggerOpcode(DWORD opcode);

	DWORD GetTriggerConnId() const;
	void SetTriggerConnId(DWORD connId);

	BYTE GetHoldRRQAfterAltEnd() const;
	void SetHoldRRQAfterAltEnd(BYTE value);

	altGksListSt *GetGkUrqAltList() const;
	void SetGkUrqAltList(altGksListSt *pAltGkListSt);
	
	void SetQosParameters(WORD audioDscp, WORD videoDscp);
	void GetQosParameters(BYTE& audioDscp, BYTE& videoDscp);

	//DNS
	void SetNumOfDnsWaiting(BYTE num);
	BYTE GetNumOfDnsWaiting();
	void IncreaseNumOfDnsWaiting();
	void DecreaseNumOfDnsWaiting();
	void SetPrimeGkIpFromDns(ipAddressStruct* pGkAddr);
	void SetAltGkIpFromDns(ipAddressStruct* pAltGkAddr);

	//  functions for Alternate Gk
	CIpAddressPtr GetAltGkIp(int index);
	void SetAltGkParamsToZero();
	void InitAltGkListFromReject(rejectInfoSt* pRejectInfo);
	void InitAltGkList(altGksListSt* pListStFromGk);
	void InsertAltGkInServiceToAltList(bool is_prim=false);
	BYTE IsDiscoveryRequiredToAltGk();
	void SortAndInsertToAlGkList(alternateGkSt *pAltGkSt);
	BYTE IsNeedToRegisterToAltGk();
	void SetAltGkAsPermanent();
	void RestoreOriginalGkParams();
	
	//m_pAltGkList function
	int GetSizeAltGkList();	
	alternateGkSt* GetAltGkMember();	
	
	void UpdateCardPropGkIp(ipAddressStruct *pAltGkAddr, int index = 0);
	void UpdateCardPropAltGkIp(ipAddressStruct *pAltGkAddr,int index);
	void UpdateCardPropPrimaryGkConfiguredName();
	void UpdateCardPropAltGkConfiguredName(int index);
	void UpdateCardPropGkName(int index, CSmallString gkName);
	void UpdateCardPropAltGkIdent(alternateGkSt *pAltGkSt,int index);
	void UpdateCardPropGkIdent(alternateGkSt *pAltGkSt,int index);
	void UpdateAltGkList(altGksListSt* pListStFromGk);
	void ClearAltGkList(int index);
	//void HandleEmptyAltGkList();

    //RAI
    // It's OK to send RAI after RAC has recieved for the previous RAI or time expired
    // for the previous RAI, this function set a flag to update the service state in this aspect
    BYTE IsOKToSendRAI() {return m_bOKToSendRAI; }
    void ReceivedRAC () {m_bOKToSendRAI = TRUE;}
    void SentRAI () {m_bOKToSendRAI = FALSE;}
    
    // IpV6
    void SetServiceIpTypes(BYTE ipVerType);
    eIpType GetServiceIpTypes();

    void   Set_EncryptMethodRequired(int par_EncryptionMethodRequired);

    // H.235
    void   Get_H235Params( GkH235AuthParam   *  par_pH235Params);    
    void   Set_H235Params( GkH235AuthParam   *  par_pH235Params);  
    int    Get_LastRejectReason();
    void   Set_LastRejectReason(int par_nLastRejectReason);

private:
	
    /* service Attributes */
	DWORD				m_ServiceId;
	char				m_ServiceName[ NET_SERVICE_PROVIDER_NAME_LEN ];
	char				m_pAliasList[MaxAddressListSize];
	DWORD				m_pAliasesTypes[MaxNumberOfAliases];
	BYTE				m_bIsGkInService;
	BYTE				m_bIsRegAsGw;
	
	char				m_prefix[PHONE_NUMBER_DIGITS_LEN + 1];
	BOOL				m_multcast;
	// Cs IPs (3):     for ipv4, ipv6 site, ipv6 global
	// first position (index 0) holds the ras address
	// the other two adresses (if exists) positioned at other two positions
	ipAddressStruct		m_CsIps[TOTAL_NUM_OF_IP_ADDRESSES];
	BYTE				m_bAreParamsReady;
	BOOL				m_bIsAvaya;				//this variable should be set by GKManager 
												//according to authetication procedure
	
	/* GK Attributes */
		//current:
	char				m_gkName[H243_NAME_LEN];
	char				m_altGkName[H243_NAME_LEN];
	ipAddressStruct		m_GkIp; 	
	char				m_gkIdent[MaxIdentifierSize];
	WORD				m_gkIdentLength;
	char				m_epIdent[MaxIdentifierSize];
	WORD				m_epIdentLength;
		// from service:
	ipAddressStruct		m_gkIpConfigured; 
	CSmallString		m_gkNameConfigured;
	ipAddressStruct		m_alternateGkIpConfigured; 
	CSmallString		m_altGkNameConfigured;
	WORD				m_RegistrationTimeConfigured;
	
	
	eRegistrationStatus m_regStatus;
	WORD				m_RRJCounter;
	BOOL				m_discovery; // TRUE => discovery+registration
	WORD				m_timeToLive;
	int					m_gkURQhsRas; 
	// Initial values for these variables are 0 because it's mean no settings
	BYTE				m_audioDscp;
	BYTE				m_videoDscp;
	
	/* Alternate Gk Params */
	BYTE				m_bAltGkPermanent;
	eAltGkProcess		m_eAltGkProcess;
	int					m_currAltGkToSearch;
	alternateGkSt*		m_pCurAltStruct;
	DWORD				m_triggerOpcode;     //trigger num 1
	DWORD				m_triggerConnId;   //trigger num 2
	BYTE				m_holdRRQAfterAltEnd;
	altGksListSt* 		m_pUrqAltGkList;
	
	std::vector<alternateGkSt>    *m_pAltGkList;
	
	/* DNS */
	DnsParamsSt  		m_dnsParams;
    BYTE                m_bOKToSendRAI;
    eIpType				m_service_ip_protocol_types;
    BYTE m_SystemCfgChecked;

    /* H.235 Gk Authentication */
    GkH235AuthParam     m_H235Params ;
    int                 m_nLastRejectReason;
};
#endif /*GKSERVICE_H_*/


