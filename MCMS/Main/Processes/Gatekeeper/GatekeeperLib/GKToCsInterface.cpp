

#include "GKToCsInterface.h"
#include "Macros.h"
#include "GkMplMcmsProtocolTracer.h"


/////////////////////////////////////////////////////////////////////////////
CGKToCsInterface::CGKToCsInterface()  	// constructor
{ 
}

/////////////////////////////////////////////////////////////////////////////
CGKToCsInterface::~CGKToCsInterface()     // destructor
{	
}

/////////////////////////////////////////////////////////////////////////////
void  CGKToCsInterface::SendMsgToCS(OPCODE opcode, CSegment* pSeg, DWORD serviceId, APIS32 status, DWORD connId, DWORD partyId, 
									DWORD confId, DWORD callIndex, WORD csId)
{
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
		
	pMplMcmsProtocol->AddCommonHeader(opcode);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader(partyId, confId , connId);
	pMplMcmsProtocol->AddCSHeader(csId, 0/*src unit id*/, eGk, callIndex, serviceId, 0/*channel index*/, 0/* mc channel index*/, status);
	if(pSeg)
	{
		DWORD nMsgLen = pSeg->GetWrtOffset() - pSeg->GetRdOffset();
		BYTE* pMessage = new BYTE[nMsgLen];
		pSeg->Get(pMessage,nMsgLen);
		pMplMcmsProtocol->AddData(nMsgLen,(const char*)pMessage);
		PDELETEA(pMessage);
	}
    pMplMcmsProtocol->AddPayload_len(CS_API_TYPE);
	CGkMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CGKToCsInterface::SendMsgToCS ",CS_API_TYPE);
	STATUS stat = 0;
	stat = pMplMcmsProtocol->SendMsgToCSApiCommandDispatcher();
	
	POBJDELETE(pMplMcmsProtocol);
}
	
////////////////////////////////////////////////////////////////////////////

