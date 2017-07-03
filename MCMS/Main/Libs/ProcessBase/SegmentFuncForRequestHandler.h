// SegmentFuncForRequestHandler.h

#if !defined(CSEGMENT_FUNC_FOR_REQUEST_HANDLER_H__)
#define CSEGMENT_FUNC_FOR_REQUEST_HANDLER_H__

#include <ostream>
#include "OperatorDefines.h"
#include "ProcessBase.h"

class CStringToProcessEntry;
class CActionRedirectionMap;

struct ConnectionDetails_S
{
    std::string userName;
    std::string workStation;
    std::string clientIp;
};

class CSegmentFuncForRequestHandler
{
public:
	static STATUS IsReadyToHandleRequests();
	static STATUS ValidateTransaction(CActionRedirectionMap *pActionRedirectionMap, char* pszNonZippedBuffer, char* pszStartActionName, int nConnId, ConnectionDetails_S &connDetails,
									  CStringToProcessEntry& destProcessEntry, eOtherProcessQueueEntry& eDestTask,
									  bool& bSetConnectionUponReturn, WORD authorization = SUPER, const char* loginName = "");
	static void RemovePasswordFromString(const char* pszSearchString, char* pszNewString, const char* pszElementOpenTag, const char* pszElementCloseTag);
	static STATUS PrepareUncompressedString(DWORD nLen,char* pszMsg, char** pszMsgToSend,
										  BYTE &bCompressedForSend, DWORD& nNewLen, BYTE bCompressed);
	static STATUS PrepareSegmentBeforeSend(BYTE bCompressedForSend, DWORD nNewLen, char* pszMsgToSend,
										  CSegment *pSegment, int nConnId, CStringToProcessEntry &destProcessEntry,
	                                      eOtherProcessQueueEntry eDestTask, int nPort,
	                                      ConnectionDetails_S &connDetails, BYTE bSHM,
	                                      const std::string & strCharSet /*= "UTF-8"*/,
	                                      WORD nAuthorization /*=SUPER*/, std::string strLogin /*=""*/,
	                                      const std::string & strClientCertificateSubj, DWORD ifNoneMatch);
	static void PrintRequestInTrace(const eOtherProcessQueueEntry eDestTask, const char* pszStartActionName, const char* pszNonZippedBuffer,const char* pMsgHeader=NULL);
	static void PrintSetRequestInTrace(const char* pszStartActionName, const char* pszNonZippedBuffer,const char* pMsgHeader=NULL);
	static BOOL IsApacheDebugMode();
	static void ReadSegmentResponse(CSegment **pResponseSegment, BYTE &bCompressed, const eOtherProcessQueueEntry eDestTask, char** pszResponse, DWORD &dwResponseLen, BYTE printResponse = YES);
	static void PrintResponseInTrace(const eOtherProcessQueueEntry eDestTask, char* pszResponse);
	static void PrintShadowPassword(char* pszNonZippedBuffer, const eOtherProcessQueueEntry eDestTask, BOOL isResponse);
	
};

#endif
