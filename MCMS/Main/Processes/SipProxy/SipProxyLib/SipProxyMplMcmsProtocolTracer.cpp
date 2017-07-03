
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////				Sip Proxy Prints          /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
#include "SipProxyMplMcmsProtocolTracer.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "IpCsOpcodes.h"

///////////////////////////////////////////////////////////////////////////////////////////
CSipProxyMplMcmsProtocolTracer::CSipProxyMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt)
:CMplMcmsProtocolTracer(mplMcmsProt)
{
}

///////////////////////////////////////////////////////////////////////////////////////////
CSipProxyMplMcmsProtocolTracer::~CSipProxyMplMcmsProtocolTracer()
{
}

///////////////////////////////////////////////////////////////////////////////////////////
void CSipProxyMplMcmsProtocolTracer::TraceContent(CObjString* pContentStr, eProcessType processType)
{
	OPCODE opcode = m_pMplMcmsProt->getCommonHeaderOpcode();
	switch (opcode)
	{
	// SipProxy -> CS
	
	case SIP_CS_PROXY_REGISTER_REQ: {
		TraceProxyRegisterReq(pContentStr);
		break;  }

	// CS -> SipProxy
	
	case SIP_CS_PROXY_REGISTER_RESPONSE_IND:{
		TraceProxyRegisterResponseInd(pContentStr);
		break;  }
	
	case SIP_CS_PROXY_TRACE_INFO_IND:{
		TraceSipTraceInfoInd(pContentStr);
		break;  }

	case SIP_CS_PROXY_SEND_CRLF_REQ:{
	  TraceProxyMsKeepAlive(pContentStr);
    break;  }

	case SIP_CS_PROXY_CRLF_ERR_IND:{
    TraceProxyMsKeepAliveErrInd(pContentStr);
    break;  }

  	default:	{
    	CMplMcmsProtocolTracer::TraceContent(pContentStr, processType);
			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////
void CSipProxyMplMcmsProtocolTracer::TraceSipTraceInfoInd(CObjString* pContentStr)
{	
	mcIndTraceInfo * pStruct = (mcIndTraceInfo *)m_pMplMcmsProt->getpData(); 
	*pContentStr << "\nCONTENT: SIP_CS_PROXY_TRACE_INFO_IND: \n "  ;  
	*pContentStr << "SIP Trace info Indication" << "\n";
	*pContentStr << "Error: " << pStruct->sErrMsg << "\n";		
}


////////////////////////////////////////////////////////////////////////////////////////////
void CSipProxyMplMcmsProtocolTracer::TraceProxyRegisterReq(CObjString* pContentStr)
{
	mcReqRegister* p = (mcReqRegister*)m_pMplMcmsProt->getpData();

	*pContentStr << "\nCONTENT: SIP_CS_PROXY_REGISTER_REQ:\n ";
	*pContentStr << "Sip Proxy register request" << "\n"
				<< "Id					: " << p->id << "\n"
				<< "Expires				: " << p->expires << "\n"
				<< "Proxy Address		: " << "\n"; 
	TraceTransportAddrSt(pContentStr, p->proxyTransportAddr);	
	*pContentStr<< "Registrar Address	: " << "\n";
	TraceTransportAddrSt(pContentStr, p->registrarTransportAddr);	
	sipMessageHeaders* pHeaders = (sipMessageHeaders *)(&(p->sipHeaders));
	CSipHeaderList headers(*pHeaders);
	headers.DumpToStream(pContentStr);
	*pContentStr << "\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
void CSipProxyMplMcmsProtocolTracer::TraceProxyRegisterResponseInd(CObjString* pContentStr)
{
	mcIndRegisterResp* p = (mcIndRegisterResp*)m_pMplMcmsProt->getpData();
	
	*pContentStr << "\nCONTENT: SIP_CS_PROXY_REGISTER_RESPONSE_IND:\n ";
	*pContentStr << "Sip Proxy register response indication" << "\n"
					<< "Status			: " << p->status;
	if (p->status)
		*pContentStr << " " << ::GetRejectReasonStr((enSipCodes)p->status);
	*pContentStr 	<< "\n"
					<< "Id				: " << p->id << "\n"
					<< "Expires			: " << p->expires << "\n";
	sipMessageHeaders* pHeaders = (sipMessageHeaders *)(&(p->sipHeaders));
	CSipHeaderList headers(*pHeaders);
	headers.DumpToStream(pContentStr);
	*pContentStr << "\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
void CSipProxyMplMcmsProtocolTracer::TraceProxyMsKeepAlive(CObjString* pContentStr)
{
  mcReqSendCrlf* p = (mcReqSendCrlf*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: SIP_CS_PROXY_SEND_CRLF_REQ:\n ";
  *pContentStr << "Sip Proxy send crlf request" << "\n"
  << "dwMsKepAliveTimeOut_Sec       : " << p->dwMsKepAliveTimeOut_Sec << "\n"
  << "Registrar Address : " << "\n";
  TraceTransportAddrSt(pContentStr, p->registrarTransportAddr);
}

////////////////////////////////////////////////////////////////////////////////////////////
void CSipProxyMplMcmsProtocolTracer::TraceProxyMsKeepAliveErrInd(CObjString* pContentStr)
{
  mcIndProxyCrlfError* p = (mcIndProxyCrlfError*)m_pMplMcmsProt->getpData();

  *pContentStr << "\nCONTENT: SIP_CS_SIG_CRLF_ERR_IND:\n ";
  *pContentStr << "Sip Proxy crlf error indication" << "\n"
  << "Error code       : " << p->eCrlfSendingErrorCode << "\n"
  << "Conf Id       : " << p->nConfId << "\n";
}
