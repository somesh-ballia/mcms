// CertMngrManager.h

#ifndef _CERTMNGRMANAGER_H_
#define _CERTMNGRMANAGER_H_

#include "ManagerTask.h"
#include "Macros.h"
#include "CertMngrDefines.h"
#include "AuditDefines.h"

class CCertMngrProcess;
void CertMngrManagerEntryPoint(void* appParam);

class CCertMngrManager : public CManagerTask
{
CLASS_TYPE_1(CCertMngrManager, CManagerTask)
public:
	CCertMngrManager(void);
	virtual ~CCertMngrManager(void);
	virtual const char* NameOf(void) const;
	virtual TaskEntryPoint GetMonitorEntryPoint(void);

private:
    virtual void ManagerInitActionsPoint();
    virtual void ManagerPostInitActionsPoint();
    virtual void ManagerStartupActionsPoint();
    virtual void DeclareStartupConditions();

    static bool IsCTLFile(const char* fname);
    static bool IsCRLFile(const char* fname);
    static bool IsCSPFXFile(const char* name, string& subFolderName, string& fileName);

    static const char* AuditOperStr(eCertificateType type);
    static const char* AuditDescStr(eCertificateType type, eAuditEventStatus stat);
    static void ManLsCert(std::ostream& ans);
    static void ManRmCert(std::ostream& ans);
    static void ManAddCert(std::ostream& ans);
    static void ManVfyCert(std::ostream& ans);
	
	STATUS HandleCreateCertificateReq(CRequest* req);
	STATUS HandleSendCertificate(CRequest* req);
	STATUS HandleCreateCertificateReqForCS(CRequest* req);
	STATUS HandleSendCertificateForCS(CRequest* req);
	STATUS HandleSendCACertificate(CRequest* req);
	STATUS HandleDelete(CRequest* req, eCertificateType type);
	STATUS HandleDeleteCTL(CRequest* req);
	STATUS HandleDeleteCRL(CRequest* req);
	STATUS HandleFinishUploadCertificate(CRequest* req);
	STATUS HandleUpdateCertificateRepository(CRequest* req);
	STATUS AddCert(eCertificateType type, const char* fname);
	STATUS DelCert(eCertificateType type,
                   const char* issuer,
                   const char* serial);
	STATUS OnRemoveActiveAlarm(CSegment* pMsg);
    STATUS GetCertificateDetails(std::string& host_name,
                                 CStructTm& cert_start_date,
                                 CStructTm& cert_expiration_date,
                                 std::string& errror_message);
	STATUS  OnCsIpServiceParamsInd(CSegment* pMsg);
	STATUS  OnCsIpServiceParamsEndInd(CSegment* pMsg);
	STATUS  OnCsDeleteIpServiceInd(CSegment* pMsg);
	void 	SendIpServiceParamsReqToCS();
    void AACertRepositoryChanged(void);
    void checkForUpgradeConflicts(int serviceId);
    void pfxCleanUp(std::string folderName,bool success);
    void OnMultipleServicesInd(CSegment* pParam);
    void PrintServiceList();

	WORD GetCertificateRequest(std::string& cert_req,
                               BYTE bForCS=FALSE,
                               std::string folder_name = "");
	void SendCertificateUpdateIndToMcuMngr();
	void OnUpdateHostName(CSegment* pMsg);
    void OnGetCertDetails(CSegment* pMsg);
    void OnVerifyCertificate(CSegment* pMsg);
    void OnVerifyCSCertificate(CSegment* pMsg);

    void OnHttpdCertMngrLinkFile(CSegment* pSeg);

    STATUS HandleTerminalSelfSignCertificateRequest(CTerminalCommand& command,
                                                    std::ostream& answer);
    STATUS HandleTerminalCertDetails(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalListCert(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalRemoveCert(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalAddCert(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalVerifyCert(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalRemoveAACert(CTerminalCommand& cmd, std::ostream& ans);
    STATUS AuditHttpCertificatesOperations(eCertificateType type,
                                           eAuditEventStatus eventStatus);


    OPCODE TryPFX(std::string fname);
    STATUS OpenPFXFile(string& folderName, string& fileName,BOOL toUpdateList = TRUE) ;

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS
	
	std::string m_host_name;
	CCertMngrProcess* m_pProcess;
	BYTE m_bSystemMultipleServices;
	std::string m_certUploadStatus;
	DISALLOW_COPY_AND_ASSIGN(CCertMngrManager);
};

#endif  // _CERTMNGRMANAGER_H_
