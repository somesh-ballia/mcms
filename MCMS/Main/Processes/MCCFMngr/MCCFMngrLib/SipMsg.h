#ifndef SIPMSG_H__
#define SIPMSG_H__

#include "Segment.h"
#include "ChannelParams.h"
#include "IpCsOpcodes.h"
#include "MplMcmsProtocol.h"
#include "SipDefinitions.h"
#include "StatusesGeneral.h"
#include "SipUtils.h"

//////////////////////////////////////////////////////////////////////
#define CFW_ID_LEN 128 //_mccf_

typedef char CFWID[CFW_ID_LEN];

//////////////////////////////////////////////////////////////////////
typedef DWORD ConfRsrcID;
typedef DWORD PartyRsrcID;
typedef DWORD ConnectionID;

//////////////////////////////////////////////////////////////////////
struct SipCallID
{
	SipCallID() : callID(0), serviceID(0), unitID(0) {}

	explicit SipCallID(const CMplMcmsProtocol& protocol);

	SipCallID& operator =(const CMplMcmsProtocol& protocol);

	APIU32 callID;
	APIU32 serviceID;
	APIU16 unitID;
};

//////////////////////////////////////////////////////////////////////
bool operator <(const SipCallID& a, const SipCallID& b);

//////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const SipCallID& obj);
CSegment& operator >>(CSegment& seg, SipCallID& obj);

//////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const SipCallID& obj);

std::ostream& operator <<(std::ostream& ostr, const mcTransportAddress& a);

//////////////////////////////////////////////////////////////////////
bool ParseCsInviteIndEvent(const CMplMcmsProtocol& protocol, CFWID cfwID, mcTransportAddress& appServerAddress, SipCallID& callID);
bool ParseCsByeIndEvent(const CMplMcmsProtocol& protocol , DWORD& outStatus, SipCallID& callID);
bool ParseCsBye200OKIndEvent(const CMplMcmsProtocol& protocol, DWORD& outStatus, SipCallID& callID);
bool ParseCsInviteAckIndEvent(const CMplMcmsProtocol& protocol, DWORD& outStatus, SipCallID& callID);
bool ParseCsTransportErrorIndEvent(const CMplMcmsProtocol& protocol, DWORD& outStatus, const char*& szErrorMessage, SipCallID& callID);
bool ParseCsBadStatusIndEvent(const CMplMcmsProtocol& protocol, DWORD& outFailOpcode, const char*& szErrorMessage, SipCallID& callID);

bool BuildSipInviteResponseReq(const CMplMcmsProtocol& protocol, enSipCodes inStatus, const mcTransportAddress& hostIpAddress, CSegment& segOut, APIU32& opcode);
bool BuildSipBye200OKReq(CSegment& segOut, APIU32& opcode);
bool BuildSipByeReq(CSegment& segOut, APIU32& opcode);

void SendSipMsgToCS(OPCODE opcode, CSegment& seg, const SipCallID& callID);

//////////////////////////////////////////////////////////////////////
#endif // SIPMSG_H__
