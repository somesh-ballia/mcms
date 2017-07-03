// CertMngrProcess.h

#ifndef CERT_MNGR_PROCESS_H_
#define CERT_MNGR_PROCESS_H_

#include <string>

#include "ProcessBase.h"
#include "AllocateStructs.h"
#include "CertificateList.h"
#include "CertMngrDefines.h"

class CCertMngrProcess : public CProcessBase
{
  CLASS_TYPE_1(CCertMngrProcess, CProcessBase)

  struct IpServiceDetails
  {
	  std::string ServName;
	  std::string HostName;
	  BOOL        IsSecured;
	  BOOL		  IsRequestPeerCertificate;
	  BYTE		  Revocation_Method;
	  std::string OcspUrl;
	  BYTE		  IsMsService;
  };

public:
  friend class CTestCertMngrProcess;

  CCertMngrProcess(void);
  virtual const char*     NameOf(void) const;
  virtual eProcessType    GetProcessType(void);
  virtual BOOL            UsingSockets(void);
  virtual TaskEntryPoint  GetManagerEntryPoint();
  virtual int             GetProcessAddressSpace(void);
  virtual DWORD           GetMaxTimeForIdle(void) const;

  void                    FillLists(void);
  const CCertificateList* ReadList(eCertificateType type) const;
  STATUS                  AddCertificate(eCertificateType type,
                                         const char* fname);
  STATUS                  DeleteCertificate(eCertificateType type,
                                            const char* issuer,
                                            const char* serial);

  void                    SetServiceName(int pos, std::string service_name,
		  	  	  	  	  	  	  	  	  BOOL isSecured,
		  	  	  	  	  	  	  	  	  BOOL isRequestPeerCertificate,
		  	  	  	  	  	  	  	  	  std::string host_name,
		  	  	  	  	  	  	  	  	  BYTE revocation_method,
		  	  	  	  	  	  	  	  	  std::string ocsp_url,
		  	  	  	  	  	  	  	  	  BYTE  isMSService);

  std::string             GetServiceName(int pos) const;
  int                     GetServiceId(std::string service_name) const;
  std::string             GetServiceHostName(std::string service_name) const;
  BOOL 					  GetServiceIsSecured(int pos) const;
  BOOL 				     GetIsRequestPeerCertificate (int pos) const;
  BYTE					 GetIsMsService(int pos ) const;

  STATUS                  RemoveActiveAlarm(std::string& out, BOOL withCSAlarms) ;

  STATUS 					VerifyCertificates(const char* host_name,
                                              const CStructTm& now,
                                              bool add_aa,
                                              bool ca_validation,
                                              BYTE revocation_method,
                                              std::string& out) const;


  STATUS 					VerifyCSServiceCertificates(const std::string serviceName,
		  	  	  	  	  	  	  	  	  	  const CStructTm& now,
											  bool add_aa,
											  bool ca_validation,
											  BYTE revocation_method,
											  BYTE isMSService,
											  std::string& out);


  int                     GetNumOfIpServices(){return m_numOfIpServices;}

  BYTE       			GetServiceRevocationMethod(int pos);

private:
  static STATUS           GetStatusByAlarm(const CERT_ALARM_VECTOR& aa);

  CCertificateList*       GetList(eCertificateType type);
  STATUS                  SaveCertificate(eCertificateType type);
  void                    FillList(eCertificateType type);
  void                    FillPersonalCertificateList();
  void                    FillCSPersonalCertificateList();

  std::string 				GetServiceNameFromFileName(const char*  fileName) const;

  CCertificateList m_CertificateTrustList;
  CCertificateList m_CertificatePersonalList;  // management list

  CCertificateList m_CertificateCSPersonalList;

  CCertificateList m_CertificateRevocationList;

  // according to each slot
  IpServiceDetails m_IPServiceList[MAX_NUMBER_OF_SERVICES_IN_RMX_4000];


  int m_numOfIpServices;
  BOOL m_alreayValidateCasForCS;

};

#endif
