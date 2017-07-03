// ApacheModuleEngine.h: interface for the CApacheModuleEngine class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(_ApacheModuleEngine_H__)
#define _ApacheModuleEngine_H__

#include "httpd.h"

#include "ApacheModuleProcess.h"
#include "ConnectionList.h"
#include "HeaderParams.h"
#include "ActionRedirection.h"
#include "OpcodesMcmsInternal.h"
#include "AuditorApi.h"

#define MAX_NUMBER_OF_VIRTUAL_DIRECTORIES 50
#define MAX_NUMBER_OF_NESTED_VIRTUAL_DIRECTORIES 2

class CVirtualDirectory;
class CHeaderParser;
struct ConnectionDetails_S;
struct apr_xml_elem;
struct apr_xml_attr;

enum eDestProccessValidity {
	pDestprocValid = 0 ,
	pDestprocInValid = 1,
	pDestprocInValidUnderJitc = 2
};

class CApacheModuleEngine {

public:

	static void Initialize(apr_pool_t *pConfigPool, server_rec *pServerRec);
	static void Clean(void *pConfigPool);

	static int HandleResponsePhase(request_rec *pRequestRec);
	static int HandleLogTransaction(request_rec *pRequestRec);

	static int HandleFatalException(int signal);


	static STATUS SendSyncMessage(COsQueue* pMbxSend, COsQueue* pMbxRcv, char* pszMsg,
								  CMessageHeader &header, CStringToProcessEntry &destProcessEntry,
								  eOtherProcessQueueEntry eDestTask, BYTE bCompressed,
                                  const string & strCharset,
								  int nLen, int nConnId, int nPort,
                                  ConnectionDetails_S & connDetails,
                                  const char* pszClientCertificateSubj = NULL,
                                  BYTE bSHM = FALSE);
	
	static DWORD GetNextMsgSeqNum(request_rec *pRequestRec);

	static void Clean();
	static void* InitSkeletonThread(void* pDummy);

	static apr_status_t CleanConnection(apr_pool_t *pConnPool);
	static void InitConnection(conn_rec *pConnRec);
	static CConnectionList* GetConnectionList() { return m_pConnectionList; };
	static int  GetKeepAliveTimeout();
	static bool GetPhysicalPath(std::string strVirtualPath, std::string& strPhysicalPath);
	static bool FillActionRedirectionMap();
	static int BasicAuthentication(request_rec *pRequestRec);
	static void SetPhysicalPath(char* pszVirtual, const char* pszPhysicalPath);
	static void SetDirectoryAccess(const char* pszPhysicalPath, char* pszDirectoryAccess);
	static int  GetVirtualDirectoryPermission(char* file_name);
	static bool IsLegalLinkedFile(char* file_name);
	static void SetCreateDirectory(const char* pszPhysicalPath, char* pszCreateDir);
    static void SetAuditabilityOfDirectory(const char* pszPhysicalPath, char* pszDirectoryAuditability);
    static void SetInformProcessOfDirectory(const char* pszPhysicalPath,
                                            char* pszProcessName);
	static int GetVirtualDirectoryPos(char* file_name);
	static int GetCreateDirectoryPermission(char* file_name);
	static void TraceHeader(request_rec *pRequestRec);
    static void DumpActionRedirectionMap(std::ostream& answer);
    static void DumpVirtualDir(std::ostream& answer);
    static BOOL IsExtensionAllowed(const std::string &url);
    static int m_Security_Loose_Revocation;
    static void SetRvgwAliasName(std::string &aliasname){m_rvgwAliasName = aliasname;};
    static eProductType GetProductType();
private:

	static BOOL AllowedForAdminstratorReadonly(int nAuthorization, const char* fileName);
	static BOOL RestrictedForAdminstratorReadonly(int nAuthorization, const char* fileName);
	static CApacheModuleProcess *m_pProcess;
	static CConnectionList *m_pConnectionList;
	static CActionRedirectionMap *m_pActionRedirectionMap;
	static int m_nKeepAliveTimeout;
	static CVirtualDirectory m_VirtualDirectoryList[MAX_NUMBER_OF_VIRTUAL_DIRECTORIES];
	static int m_nLastVD;
	static std::string m_rvgwAliasName;
	static eProductType m_curProductType;
	static apr_thread_mutex_t* m_pAccessMutexEngine;//VNGFE-7713

private:

	static int ReadRequestContent(request_rec *pRequestRec, char **pszRequest, FILE* pFile);
	static int HandleGetRequest(request_rec *pRequestRec, int nConnId, int nAuthorization);
	static int HandlePostRequest(request_rec *pRequestRec, int nConnId, BYTE bSHM=FALSE, BYTE bLogin=FALSE);
	static int HandlePutRequest(request_rec *pRequestRec, int nConnId, int nAuthorization);
	static int HandleDeleteRequest(request_rec *pRequestRec, int nConnId, int nAuthorization);
	static int GetClientConnId(request_rec *pRequestRec);
	static void SetupResponseHeader(request_rec *pRequestRec, CHeaderParams& HeaderParams,
									bool bPragmaOnly=false);
	static void AddAnonymousConnection(request_rec *pRequestRec, int &nConnId);

	static int GetValidConnId(request_rec *pRequestRec);

    static STATUS SendSyncMessageToCertMngr(request_rec* pRequestRec, const char* strPhysicalPath);
	static STATUS SendResponse(request_rec *pRequestRec, int nConnId, int nMessageId,
							 char* pszResponse, int nResponseLen, const char* pszContentType, BYTE bCompressed);

	static void SendGeneralResponse(request_rec *pRequestRec, char* pszRequestContent,
								    int nStatus, int nConnI, int nMessageIdInHeader,
                                    const string & charSet);

	static eProcessType ProcNameToProcType(char* pszProcName);

	static STATUS ExtractEncodingFromHttpHeader(CHeaderParser HeaderParser,
												request_rec *pRequestRec,
												int nConnId,
                                                int nMessageId,
                                                string & outStrCharset);


	static STATUS DecompressedTheContent(char* pszNonZippedBuffer, int pszNonZippedBufferLen,
                                         request_rec *pRequestRec,
										 char* pszRequestContent, int nContentLen,
										 BYTE& bCompressed, int nConnId,int nMessageId,
										 const string & strCharset);

	static STATUS ExtractEncodingFromComment(char* pszNonZippedBuffer,
											 request_rec *pRequestRec, int nConnId,
											 int nMessageId,
                                             string & outStrCharset);

    static void OnMissingAuditAttr(const char *action, const char *desc, const char *detail);
	static void SendEventToAuditor(request_rec *pRequestRec, int nConnId, const string & action, const string & description, const string & failure_description);
	static void SendEventToAuditorThreadSafe(CFreeData& freeData,AUDIT_EVENT_HEADER_S& outAuditHdr);
    static void ExtractClientIp(request_rec *pRequestRec, string & outClientIp);
    static void ExtractConnectionDetails(request_rec *pRequestRec, ConnectionDetails_S & outConnectionDetails);

    static void GetHttpStatusDescription(int httpStatus, string & outStatusDesc);

    static bool ParseTransactionAttributes(const apr_xml_elem *pActionElement,
                                           bool & outIsAuditable,
                                           string & outTransName,
                                           string & outTransDesc,
                                           string & outErrorMessage);

    static bool TakeAttrAuditable(const apr_xml_attr *elemAttr,
                                  bool & outIsAuditable,
                                  string & outErrorMessage);
    static bool TakeAttrName(const apr_xml_attr *elemAttr,
                             string & outTransName,
                             string & outErrorMessage);
    static bool TakeAttrDescription(const apr_xml_attr *elemAttr,
                                    string & outTransDesc,
                                    string & outErrorMessage);

    static void WaitForInitSkeletonThread(DWORD maxNumIter);

    static BOOL IsFileModified(request_rec *pRequestRec, const string & filename);

    static void BuildXmlLoginRequest(request_rec *pRequestRec, char** szLoginRequest);
    static void BuildXmlLogoutRequest(char** szLogoutRequest);

	static void PrintShadowPassword(char* pszNonZippedBuffer, const eOtherProcessQueueEntry eDestTask, BOOL isResponse = FALSE);
	static bool IsLogin(request_rec *pRequestRec);

	static bool	IsCertFile(const char* name);

	static bool IsRvgwSslPorts();
	static bool CheckIfRvgwRequest(request_rec *pRequestRec);
	static bool CheckIfRestAPIRequest(request_rec *pRequestRec);
	static bool IsRvgwUseActiveX();
	static eDestProccessValidity IsDestProccessValid(eProcessType procType);
	static bool CheckSecurityToken(const char *pSecurityToken,request_rec *pRequestRec);
	static bool CheckLogOutSecurityToken(const char *pSecurityToken,request_rec *pRequestRec);
	static char * my_escape_urlencoded_buffer(char *copy, const char *buffer);
	static void DumpLoadingMessage(const char *pszMessage);
	static bool IsPlatformOfNewArch();
};



#endif // !defined(_ApacheModuleEngine_H__)

