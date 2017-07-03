#ifndef IPCOMMONUTILTRACE_H_
#define IPCOMMONUTILTRACE_H_

#include "DataTypes.h"
#include "IpChannelParams.h"
#include "ICEApiDefinitions.h"

class CObjString;

void GetChannelTypeName(APIU32 channelType,CObjString& msg);
void GetAnnexTypeName(APIU32 annexType,CObjString& msg);
void GetChannelDirectionName(APIU32 channelDirection, CObjString& msg);
void GetMediaModeName(APIU32 mediaMode, CObjString& msg);
void GetGenericCodeName(APIU32 genericCode, CObjString& msg);
void GetEncryptionMediaTypeName(APIU32 encryptionMediaType, CObjString& msg);
char* GetTransportTypeName(APIU32 enTransportType);
char* GetConnectionTransportModeName(APIU32 enConnectionTransportMode);

void GetEndpointTypeName(APIU32 endpointType, CObjString& msg);
void GetAliasTypeName(APIU32 aliasType, CObjString& msg);
void GetCallTypeName(APIU32 callType, CObjString& msg);
void GetRasRejectReasonTypeName(APIU32 rasRejectReasonType, CObjString& msg);
void GetConferenceGoalTypeName(APIU32 conferenceGoalType, CObjString& msg);
void GetRejectReasonTypeName(APIU32 rejectReasonType, CObjString& msg);
void GetCallModelTypeName(APIU32 callModelType, CObjString& msg);
void GetCallCloseStateModeName(APIU32 callCloseState, CObjString& msg);

void GetICEErrorAsString(DWORD Err,CObjString& msg);
void getIceStatusErrorText(iceServersStatus status,  CObjString& msg);
void getIceFwType(firewallTypes type,  CObjString& msg);
void getIceEnv(eIceEnvTypes env,  CObjString& msg);

void GetBFCPTransactionType(APIU32 type, CObjString& msg);
void GetBFCPFloorStatusType(APIU32 type, CObjString& msg);
void GetMsgTypeName(APIU32 MsgType, CObjString &msg);
void GetFeccTokenResponseName(APIU32 feccTokenResponse,CObjString& msg);

#endif /*IPCOMMONUTILTRACE_H_*/
