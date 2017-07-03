// ApacheModuleManager.h

#if !defined(_ApacheModuleMANAGER_H__)
#define _ApacheModuleMANAGER_H__

#include "ManagerTask.h"
#include "GateWaySecurityToken.h"

struct ConnectionDetails_S;
class CConnection;

struct RvgwCredential;



class CApacheModuleManager : public CManagerTask
{
CLASS_TYPE_1(CApacheModuleManager,CManagerTask )
public:
	CApacheModuleManager();
	virtual ~CApacheModuleManager();
	const char * NameOf(void) const {return "CApacheModuleManager";}

	void SelfKill();

	virtual BOOL HasWatchDogTask() {return FALSE;}

	TaskEntryPoint GetMonitorEntryPoint();

	virtual void ReceiveAdditionalParams(CSegment* pSeg);
	virtual void SendAdditionalParams(CSegment* pSeg);
	virtual void ResetAdditionalParams();
	virtual BOOL selfKillOnMemoryExhaustion(){ return TRUE; }
	void SendEventToAuditor(ConnectionDetails_S connDetails,char* pszRequestXmlString);

protected:
	// terminal commands
	STATUS HandleTerminalConnectionsList(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalShowTransMap(CTerminalCommand & command, std::ostream& answer);
  STATUS HandleTerminalShowVirtualDirMap(CTerminalCommand & command, std::ostream& answer);

private:
	STATUS HandleOperLogin(CRequest *pRequest);
	STATUS HandleOperLogout(CRequest *pRequest);
	STATUS HandleCreateDirectory(CRequest *pRequest);
	STATUS HandleRemoveDirectory(CRequest *pRequest);
	STATUS HandleRemoveDirectoryContent(CRequest *pRequest);
	STATUS HandleRename(CRequest *pRequest);
	STATUS HandleSetRvgwAlias(CRequest *pRequest);
	STATUS HandleCreateSecurityToken(CRequest *pRequest);
	void   OnBlockRequests(CSegment* pSeg);
	void   ConnectionGarbageCollector(CSegment* pMsg);
	void   OnMcuMngrAuthenticationStruct(CSegment* pSeg);
	void   OnMultipleServicesInd(CSegment* pParam);
	void   OnMcuMngrApacheModuleLicensingInd(CSegment* pSeg);
	void   OnHttpdSeucreLoginInd(CSegment* pSeg);
	void   OnLicensingInitialisationTimeout(CSegment* pSeg);
	void   SendAuthenticationStructAndLicensingReqToMcuMngr();
	virtual void ManagerPostInitActionsPoint();
	void SendAuditEventDelConnection(CConnection *pConn);

	DWORD m_dwConnId;
	DWORD m_dwPort;
	BYTE  m_bSHMRequest;
	char* m_pszClientCertificateSubj;

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS
	
	BYTE m_bSystemMultipleServices;
	BYTE m_bV35JitcSupport;

    list<RvgwCredential> m_rvgwCredentials;
};

#endif // !defined(_ApacheModuleMANAGER_H__)

