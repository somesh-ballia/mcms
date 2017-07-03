// HlogApi.h

#ifndef __HLOGAPI_H_
#define __HLOGAPI_H_

#include <string>
#include "DataTypes.h"
#include "SharedDefines.h"
#include "FaultsDefines.h"

enum eAuditable
{
    eAuditableNotification,
    eAuditableFailure,
    eAuditableNothing
};

class CSegment;
class CLogFltElement;
class CFaultDesc;

class CHlogApi
{
public:
	static void TaskFault( BYTE subject, DWORD errorCode, BYTE errorLevel, const char *description,
						   BOOL isFullOnly, DWORD boardId = 0, DWORD unitId = 0, WORD theType = 0);

	static bool TaskFault(const char *faultDescStr, BOOL alwaysSend);
	static void TaskFault(CFaultDesc *faultDesc, BOOL alwaysSend);

	static void SystemStartup(const bool isFederal);
	static void SoftwareBug(const char* fileName,const WORD lineNum,const DWORD errCode, const char* pszMes="");  //Called from PAssert().
	static void SocketDisconnect(DWORD boardId);
	static void SocketDisconnectNoFault(DWORD boardId);
	static void SocketReconnect(DWORD boardId);
	static void CmReconnect(const WORD boardId, const WORD subBoardId, const WORD theType);
	static void EmbUserMsgFault(const WORD boardId, const WORD subBoardId, const std::string desc, const WORD theType);
	static void ReservationError(const WORD errorCode, const DWORD confId, const DWORD partyId,
	                             const char* confName, const char* partyName);
	static void AppServerInternalError(const char *error);
	static void BackupAppError(const DWORD dwFailure, const char *error);
	static void CardsConfigError(const DWORD errorCode,const BYTE  faultLevel);
	static void GateKeeperError(const WORD boardId, const BYTE  faultLevel, const char *str);
	static void GateKeeperMessage(const WORD boardId, const BYTE  faultLevel, const char *str);
	static void BadSpontaneousIndication(const WORD boardId ,const WORD m_unitId, const WORD status);
	static void ImproprietyInDecryption();
	static void TDMDeficiencyFault(const char *mes);
	static void ConflictInSystemConfiguration(const DWORD Error);
	static void CsRecoveryStatus(std::string errStr);
	static void XmlParseError(std::string errStr);
	static void TestError(std::string errStr);
	static void ServiceAttachedToSpanNotExists(std::string errStr);
	static void IsdnServiceInconsistentSpanType(std::string errStr);
	static void NumOfConfiguredIsdnSpansExceeded(std::string errStr);
	static void CsGeneralFaults(std::string errStr);
	static void SessionTimerFault(std::string errStr);

private:
	CHlogApi();
	~CHlogApi();

	static bool SendMessage(const OPCODE opcode,CSegment* pMsgSeg);
	static bool SendFaultToLogger(CLogFltElement* pHlogElement,  const BOOL isFullHlogOnly=FALSE, BOOL alwaysSend =FALSE);
	static void SendFaultToLogger(const WORD code, const char* p, const BOOL isFullHlogOnly=FALSE);

  static bool AddToDB_IsFlooding(CLogFltElement *pHlogElement);
  static void TreatBadFaultType( BYTE subject,
                                 DWORD errorCode,
                                 BYTE errorLevel,
                                 const char *description,
                                 BOOL isFullOnly,
                                 DWORD boardId,
                                 DWORD unitId,
                                 WORD theType);
  static void SendFaultToAudit(CLogFltElement* pHlogElement);
  static eAuditable IsAuditableFault(DWORD errorCode);
};

#endif
