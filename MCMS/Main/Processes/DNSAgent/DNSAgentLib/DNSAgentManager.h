// DNSAgentManager.h

#ifndef DNS_AGENT_MANAGER_H_
#define DNS_AGENT_MANAGER_H_

#include "ManagerTask.h"
#include "Macros.h"
#include "NStream.h"
#include "DefinesIpService.h"
#include "SipDefinitions.h"

#define MAX_QUERIES         20
#define COMMON_REQ_BUF_SIZE 255
#define STR_LEN             256

// CDNSQuery Manager Conf states
const WORD DNS_IDLE          = 1;
const WORD DNS_RESOLVING     = 2;
const WORD DNS_VALID         = 3;

typedef enum
{
  eResolveQuery = 0,
  eAllQuery,
  eServiceQuery,
  eResolveGkQuery,
  eResolvePartyQuery,
  eServicePartyQuery,
  eResolveDummy
} enQueryType;

void DNSAgentManagerEntryPoint(void* appParam);

class CDNSQuery : public CStateMachine
{
  CLASS_TYPE_1(CDNSQuery, CStateMachine)
public:
  CDNSQuery();
  CDNSQuery(const CDNSQuery& other);
  virtual ~CDNSQuery();
  CDNSQuery& operator =(const CDNSQuery& other);

  virtual void     HandleEvent(CSegment* pMsg);
  virtual void     HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

  virtual int      Create(WORD serviceID, WORD index, char* pDomainName,
                          COsQueue* pRcvMbx,
                          eIPProtocolType protocolType, DWORD transportType,
                          eProcessType processType,
                          DWORD connId,
                          DWORD partyId);

  WORD             GetServiceId();
  const char*      GetDomainName() const;
  const char*      GetServiceName() const;
  ipAddressStruct* GetAdressList();
  DWORD            GetPort();
  ipAddressStruct* GetIpAddress(WORD index);
  eIPProtocolType  GetProtocolType();
  eProcessType     GetProcessType();
  DWORD            GetTransportType();
  WORD             GetQueryState();
  enQueryType      GetQueryType();
  DWORD            GetTtl();
  void             setAddressesToZero(ipAddressStruct* resolvedAddresses);

  virtual void     Retry();
  virtual STATUS   SendToCsApi(OPCODE opcode,
                               const int dataLen,
                               const char* data);

  virtual void     OnResolveToutRESOLVING(CSegment* pParam);
  virtual void     OnResolveIndRESOLVING(CSegment* pParam);
  virtual void     OnRefreshTimerVALID(CSegment* pParam);
  virtual void     OnResolveToutVALID(CSegment* pParam);
  virtual void     OnResolveIndVALID(CSegment* pParam);

protected:
  enQueryType     m_queryType;
  WORD            m_serviceID;
  char            m_strService[NET_SERVICE_PROVIDER_NAME_LEN];
  WORD            m_index;
  char*           m_strDomainName;
  eIPProtocolType m_protocolType;
  eProcessType    m_processType;
  DWORD           m_connId;
  DWORD           m_partyId;
  DWORD           m_transportType;
  WORD            m_resolvedPort;
  ipAddressStruct m_resolvedAddresses[TOTAL_NUM_OF_IP_ADDRESSES];
  WORD            m_timerRefresh;
  WORD            m_timerResolve;
  COsQueue*       m_pDNSMngrRcvMbx;
  DWORD           m_ttl;

  PDECLAR_MESSAGE_MAP;
};

class CDNSDummyQuery : public CDNSQuery
{
  CLASS_TYPE_1(CDNSDummyQuery, CDNSQuery)
public:
  virtual void OnResolveIndRESOLVING(CSegment* pParam);
  virtual void OnResolveIndVALID(CSegment* pParam);

protected:
  PDECLAR_MESSAGE_MAP
};

class CDNSServiceQuery : public CDNSQuery
{
  CLASS_TYPE_1(CDNSServiceQuery, CDNSQuery)
public:
  CDNSServiceQuery();
  ~CDNSServiceQuery();

  virtual int  Create(WORD serviceID, WORD index, char* pDomainName,
                      COsQueue* pRcvMbx,
                      eIPProtocolType protocolType, DWORD transportType,
                      eProcessType processType,
                      DWORD connId,
                      DWORD partyId);

  virtual void Retry();

  virtual void OnResolveIndRESOLVING(CSegment* pParam);
  virtual void OnResolveToutRESOLVING(CSegment* pParam);
  virtual void OnRefreshTimerVALID(CSegment* pParam);
  virtual void OnResolveIndVALID(CSegment* pParam);

protected:
  PDECLAR_MESSAGE_MAP
};

class CDNSAgentManager : public CManagerTask
{
  CLASS_TYPE_1(CDNSAgentManager, CManagerTask)
public:
  CDNSAgentManager();
  virtual ~CDNSAgentManager();

  void           Create(CSegment& appParam);
  virtual void   HandleEvent(CSegment* pMsg);

  void           ManagerPostInitActionsPoint();
  void           Dump(COstrStream& msg) const;

  TaskEntryPoint GetMonitorEntryPoint();

protected:
  int            IsQueryExist(enQueryType queryType, eProcessType processType,
                              WORD serviceID, char* pDomainName,
                              WORD index = (WORD)-1);
  int            IsQueryExist(enQueryType queryType, char* pDomainName,
                              WORD index = (WORD)-1);

  WORD           GetQueryState(int indDB);
  void           AddQuery(enQueryType queryType, WORD serviceID,
                          char* pDomainName, eIPProtocolType protocolType =
                            eIPProtocolType_SIP,
                          DWORD transportType = 0,
                          eProcessType proceesType = eProcessTypeInvalid,
                          DWORD connId = 0,
                          DWORD partyId = 0);

  void           OnMcuMngrConfigurationStatusInd(CSegment* pParam);
  void           OnCSApi_Msg(CSegment* pSeg);
  void           OnResolveDomainReq(CSegment* pMsg);
  void           OnResolveDomainReq2(CSegment* pParam);
  void           OnResolveInd(CSegment* pParam);
  void           OnServiceReq(CSegment* pParam);
  void           OnServiceInd(CSegment* pParam);
  void           OnClearQueries(CSegment* pParam = NULL);
  void           OnStatusUpdate(CSegment* pParam);
  void           RemoveServiceIdFromQueries(CSegment* pSeg);

  STATUS         HandleTerminalDumpAgent  (CTerminalCommand& command, std::ostream& answer);
  //-S-PLCM_DNS----------------------------------------------------------------------------//
  STATUS         HandleTerminalDumpRecords(CTerminalCommand& command, std::ostream& answer);
  STATUS         HandleTerminalDnsResolve (CTerminalCommand& command, std::ostream& answer);
  STATUS         HandleTerminalDnsConf    (CTerminalCommand& command, std::ostream& answer);

  void           OnRecvDnsResponse(CSegment* pParam); 
  void           OnCsMngrDnsConfig(CSegment* pParam);
  void           OnCsMngrCsConfig(CSegment* pParam);
  //-E-PLCM_DNS----------------------------------------------------------------------------//
 
  void           CreateDummyResolution();
  void 			 OnMcuMngrConfigurationStatus(WORD flag);

protected:
  CDNSQuery* m_pDNSQueryEnteties[MAX_QUERIES];

  PDECLAR_MESSAGE_MAP;
  PDECLAR_TRANSACTION_FACTORY;
  PDECLAR_TERMINAL_COMMANDS;

private:
  virtual void DeclareStartupConditions();
  virtual void ManagerStartupActionsPoint();
};

#endif
