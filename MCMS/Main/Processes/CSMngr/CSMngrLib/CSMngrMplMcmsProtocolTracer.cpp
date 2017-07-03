

#include "CSMngrMplMcmsProtocolTracer.h"
#include "IpCsOpcodes.h"
#include "OpcodesMcmsInternal.h"
#include "CsStructs.h"
#include "NStream.h"
#include "WrappersCS.h"
#include "ObjString.h"



CCSMngrMplMcmsProtocolTracer::CCSMngrMplMcmsProtocolTracer()
{
	InitDumpMethodMap();
}

CCSMngrMplMcmsProtocolTracer::CCSMngrMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt)
:CMplMcmsProtocolTracer(mplMcmsProt)
{
	InitDumpMethodMap();
}

CCSMngrMplMcmsProtocolTracer::~CCSMngrMplMcmsProtocolTracer()
{}

void CCSMngrMplMcmsProtocolTracer::InitDumpMethodMap()
{

	m_DumpMethodMap[CS_NEW_IND						] = &CCSMngrMplMcmsProtocolTracer::DumpCsNewInd;
	m_DumpMethodMap[CS_CONFIG_PARAM_IND				] = &CCSMngrMplMcmsProtocolTracer::DumpConfigParamsInd;
	m_DumpMethodMap[CS_END_CONFIG_PARAM_IND			] = &CCSMngrMplMcmsProtocolTracer::DumpEndConfigParamsInd;
	m_DumpMethodMap[CS_END_CS_STARTUP_IND			] = &CCSMngrMplMcmsProtocolTracer::DumpEndCsStartupInd;
	m_DumpMethodMap[CS_NEW_SERVICE_INIT_IND			] = &CCSMngrMplMcmsProtocolTracer::TraceCSBuffer;
	m_DumpMethodMap[CS_COMMON_PARAM_IND				] = &CCSMngrMplMcmsProtocolTracer::TraceCSBuffer;
	m_DumpMethodMap[CS_END_SERVICE_INIT_IND			] = &CCSMngrMplMcmsProtocolTracer::TraceCSBuffer;
	m_DumpMethodMap[CS_DEL_SERVICE_IND				] = &CCSMngrMplMcmsProtocolTracer::TraceCSBuffer;
	m_DumpMethodMap[CS_KEEP_ALIVE_IND				] = &CCSMngrMplMcmsProtocolTracer::DumpKeepAliveInd;


	m_DumpMethodMap[CS_PING_IND		        		] = &CCSMngrMplMcmsProtocolTracer::DumpPingInd;	
	m_DumpMethodMap[CS_NEW_REQ						] = &CCSMngrMplMcmsProtocolTracer::DumpCsNewReq;
	m_DumpMethodMap[CS_CONFIG_PARAM_REQ				] = &CCSMngrMplMcmsProtocolTracer::DumpConfigParamsReq;
	m_DumpMethodMap[CS_END_CONFIG_PARAM_REQ			] = &CCSMngrMplMcmsProtocolTracer::DumpEndConfigParamsReq;

	m_DumpMethodMap[CS_LAN_CFG_REQ					] = &CCSMngrMplMcmsProtocolTracer::DumpCsLanCfgReq;


	m_DumpMethodMap[CS_NEW_SERVICE_INIT_REQ			] = &CCSMngrMplMcmsProtocolTracer::TraceCSBuffer;//VNGR-10970 - prevent passwords to be sent into the log.
	m_DumpMethodMap[CS_COMMON_PARAM_REQ				] = &CCSMngrMplMcmsProtocolTracer::TraceCSBuffer;
	m_DumpMethodMap[CS_END_SERVICE_INIT_REQ			] = &CCSMngrMplMcmsProtocolTracer::TraceCSBuffer;
	m_DumpMethodMap[CS_DEL_SERVICE_REQ				] = &CCSMngrMplMcmsProtocolTracer::TraceCSBuffer;
    m_DumpMethodMap[CS_PING_REQ			        	] = &CCSMngrMplMcmsProtocolTracer::TracePingReq;
}

void CCSMngrMplMcmsProtocolTracer::TraceContent(CObjString* pContentStr, eProcessType processType)
{
	if(m_pMplMcmsProt == NULL || m_pMplMcmsProt->getDataLen() == 0)
	{
		*pContentStr << "\nCONTENT: No Content ";
		return;
	}

	OPCODE opcode = m_pMplMcmsProt->getCommonHeaderOpcode();
	const char *contentStr = m_pMplMcmsProt->getpData();
	COstrStream ostr;

	DumpMethod method = m_DumpMethodMap[opcode];
	if(NULL != method)
	{
		(this->*method)(ostr, contentStr);
		*pContentStr << ostr.str().c_str();
	}
	else
	{
		*pContentStr << "\nCONTENT: Unrecognized";
	}
}


void CCSMngrMplMcmsProtocolTracer::DumpCsNewInd(std::ostream & ostr, const char *contentStr)
{
	const CS_New_Ind_S *pContent = (const CS_New_Ind_S *)contentStr;
	CNewIndWrapper(*pContent).Dump(ostr);
}


void CCSMngrMplMcmsProtocolTracer::DumpConfigParamsInd(std::ostream & ostr, const char *contentStr)
{
	const CS_Config_Ind_S *pContent = (const CS_Config_Ind_S *)contentStr;
	CConfigIndWrapper(*pContent).Dump(ostr);
}


void CCSMngrMplMcmsProtocolTracer::DumpEndConfigParamsInd(std::ostream & ostr, const char *contentStr)
{
	const CS_End_Config_Ind_S *pContent = (const CS_End_Config_Ind_S *)contentStr;
	CEndConfigIndWrapper(*pContent).Dump(ostr);
}


void CCSMngrMplMcmsProtocolTracer::DumpEndCsStartupInd(std::ostream & ostr, const char *contentStr)
{
	const CS_End_StartUp_Ind_S *pContent = (const CS_End_StartUp_Ind_S *)contentStr;
	CEndStartupIndWrapper(*pContent).Dump(ostr);;
}


void CCSMngrMplMcmsProtocolTracer::TraceCSBuffer(std::ostream & ostr, const char *contentStr)
{
	static DWORD DWORDlen = sizeof(DWORD);

	DWORD dataLen = *((DWORD*)contentStr);
	const char *data = contentStr + DWORDlen;

	CCSBufferIndWrapper(data, dataLen).Dump(ostr);
}


void CCSMngrMplMcmsProtocolTracer::DumpCsNewReq(std::ostream & ostr, const char *contentStr)
{
	const CS_New_Req_S *pContent = (const CS_New_Req_S *)contentStr;
	CNewReqWrapper(*pContent).Dump(ostr);
}


void CCSMngrMplMcmsProtocolTracer::DumpConfigParamsReq(std::ostream & ostr, const char *contentStr)
{
	const CS_Config_Req_S *pContent = (const CS_Config_Req_S *)contentStr;
	CConfigReqWrapper(*pContent).Dump(ostr);
}


void CCSMngrMplMcmsProtocolTracer::DumpEndConfigParamsReq(std::ostream & ostr, const char *contentStr)
{
	const CS_End_Config_Req_S *pContent = (const CS_End_Config_Req_S *)contentStr;
	CEndConfigReqWrapper(*pContent).Dump(ostr);
}


void CCSMngrMplMcmsProtocolTracer::DumpCsLanCfgReq(std::ostream & ostr, const char *contentStr)
{
	const CS_Lan_Cfg_Req_S *pContent = (const CS_Lan_Cfg_Req_S *)contentStr;
	CLanCfgReqWrapper(*pContent).Dump(ostr);
}


void CCSMngrMplMcmsProtocolTracer::DumpKeepAliveInd(std::ostream & ostr, const char *contentStr)
{
	const csKeepAliveSt *pContent = (const csKeepAliveSt*)contentStr;
	CCSKeepAliveIndWrapper(*pContent).Dump(ostr);
}


void CCSMngrMplMcmsProtocolTracer::DumpPingInd(std::ostream & ostr, const char *contentStr)
{
    const CS_Ping_ind_S *pContent = (const CS_Ping_ind_S*)contentStr;
    CPingIndWrapper(*pContent).Dump(ostr);
}

void CCSMngrMplMcmsProtocolTracer::TracePingReq(std::ostream & ostr, const char *contentStr)
{
    const CS_Ping_req_S *pContent = (const CS_Ping_req_S*)contentStr;
    CPingReqWrapper(*pContent).Dump(ostr);
}
