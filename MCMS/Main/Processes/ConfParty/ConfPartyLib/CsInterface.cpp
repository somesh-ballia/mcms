// rh323.cpp : implementation of the CH323 class
//
#include "TraceStream.h"
#include "CsInterface.h"
#include "Trace.h"
#include "Macros.h"
#include "ConfPartyMplMcmsProtocolTracer.h"
#include "MplMcmsProtocolTracer.h"
                                       
/////////////////////////////////////////////////////////////////////////////
CCsInterface::CCsInterface()  	// constructor
{ 
	m_pRsrcParams = NULL;

}
/////////////////////////////////////////////////////////////////////////////
CCsInterface::~CCsInterface()     // destructor
{
	POBJDELETE (m_pRsrcParams);   


}
/////////////////////////////////////////////////////////////////////////////
CCsInterface::CCsInterface(const CCsInterface& rhs)
:CInterface(rhs)
{
	m_pRsrcParams = NULL;
	
	if(rhs.m_pRsrcParams)
		m_pRsrcParams = new CRsrcParams(*rhs.m_pRsrcParams);
}
/////////////////////////////////////////////////////////////////////////////
void  CCsInterface::Create(CRsrcParams* pRsrcParams)
{   
	POBJDELETE(m_pRsrcParams);
	m_pRsrcParams = new CRsrcParams(*pRsrcParams);	

}
/////////////////////////////////////////////////////////////////////////////
void  CCsInterface::SendMsgToCS(OPCODE opcode, CSegment* pseg1,WORD csId, DWORD csServiceId, WORD csDestUnitId, DWORD callIndex, DWORD channelIndex, DWORD mcChannelIndex, APIS32 status  )
{
	if(!m_pRsrcParams)
	{
		PASSERTMSG((DWORD)opcode,"Rsrc params not valid"); 
		return;
	}
	FPTRACE2INT(eLevelInfoNormal, "CCsInterface::SendMsgToCS ",callIndex);

	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
		
	pMplMcmsProtocol->AddCommonHeader(opcode);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader(m_pRsrcParams->GetPartyRsrcId(),m_pRsrcParams->GetConfRsrcId(), m_pRsrcParams->GetConnectionId());
	pMplMcmsProtocol->AddCSHeader(csId,0,csDestUnitId,callIndex,csServiceId,channelIndex,mcChannelIndex,status);
	if(pseg1)
	{
		DWORD nMsgLen = pseg1->GetWrtOffset() - pseg1->GetRdOffset();
		BYTE* pMessage = new BYTE[nMsgLen];
		pseg1->Get(pMessage,nMsgLen);
		pMplMcmsProtocol->AddData(nMsgLen,(const char*)pMessage);
		PDELETEA(pMessage);
	}
    pMplMcmsProtocol->AddPayload_len(CS_API_TYPE);

	CConfPartyMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("***CCsInterface::SendMsgToCS ",CS_API_TYPE);
	
	
	STATUS stat = 0;
	stat = pMplMcmsProtocol->SendMsgToCSApiCommandDispatcher();
//	TRACEINTO << "traceCCsInterface::SendMsgToCS Status of sending: " << stat; 
	
	POBJDELETE(pMplMcmsProtocol);
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CCsInterface::GetConfRsrcId() const
{                                     
	if(m_pRsrcParams)
		return(m_pRsrcParams->GetConfRsrcId());
	else
		return 0;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CCsInterface::GetPartyRsrcId() const
{                                     
	if(m_pRsrcParams)
		return(m_pRsrcParams->GetPartyRsrcId());
	else
		return 0;
}
/////////////////////////////////////////////////////////////////////////////
void CCsInterface::SetPartyRsrcId(DWORD partyRsrcId)
{
	if(m_pRsrcParams)
		m_pRsrcParams->SetPartyRsrcId(partyRsrcId);
}
/////////////////////////////////////////////////////////////////////////////
void CCsInterface::SetConfRsrcId(DWORD confRsrcId)
{
	if(m_pRsrcParams)
		m_pRsrcParams->SetConfRsrcId(confRsrcId);
}

/////////////////////////////////////////////////////////////////////////////
void CCsInterface::UpdateRsrcParams(CRsrcParams* pRsrcParams)
{
	m_pRsrcParams = pRsrcParams;
}

/////////////////////////////////////////////////////////////////////////////
void CCsInterface::SetTddMockInterface(CMplMcmsProtocol* pMockMplMcmsProtocol)
{
//	POBJDELETE (m_pMplMcmsProtocol);
	//m_pMplMcmsProtocol = pMockMplMcmsProtocol;
}


