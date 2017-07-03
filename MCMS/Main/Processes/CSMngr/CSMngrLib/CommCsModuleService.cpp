// CommCsModuleService.cpp: implementation of the CCommCsModuleService class.
//
//////////////////////////////////////////////////////////////////////

#include "CommCsModuleService.h"
#include "MplMcmsProtocol.h"
#include "CSMngrMplMcmsProtocolTracer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommCsModuleService::CCommCsModuleService()
{

}

CCommCsModuleService::~CCommCsModuleService()
{

}

//////////////////////////////////////////////////////////////////////
// csId - is the service id. since service id already in used in cs side, we are using the csId parameter instead
STATUS CCommCsModuleService::SendToCsApi(OPCODE opcode, int destUnitId, const int dataLen, const char * data, const int csId)
{
	CMplMcmsProtocol        mplProt;
	mplProt.AddCommonHeader(opcode, 0, 0, 0, eCentral_signaling);
	mplProt.AddMessageDescriptionHeader();
	mplProt.AddCSHeader(csId, 0, destUnitId);
	mplProt.AddData(dataLen, data);
	STATUS res = mplProt.SendMsgToCSApiCommandDispatcher();

	char buff[256];
	sprintf(buff, "SignalingTask (csId %d) or CSMngr send message to CSApi", csId);

	m_CSMngrMplMcmsProtocolTracer->SetData(&mplProt);
	m_CSMngrMplMcmsProtocolTracer->TraceMplMcmsProtocol(buff, CS_API_TYPE);
	m_CSMngrMplMcmsProtocolTracer->SetData(NULL);

	return res;
}
