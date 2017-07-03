// CommStartupService.cpp: implementation of the CCommStartupService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with CS Module. 
//								Startup (it sends/receives structs)
//========   ==============   =====================================================================


#include "CommStartupService.h"
#include "IpCsOpcodes.h"
#include "ProcessBase.h"
#include "CSMngrProcess.h"
#include "SysConfigKeys.h"
#include "StatusesGeneral.h"
#include "CSMngrStatuses.h"
#include "TraceStream.h"
#include "WrappersCS.h"
#include "McuMngrInternalStructs.h"
#include "WrappersMcuMngr.h"
#include "CSKeys.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommStartupService::CCommStartupService()
{
	CSysConfig *sysConfig = m_pCSProcess->GetSysConfig();
	sysConfig->GetAllParamsBySection(eCfgSectionCSModule, m_CommonParams);

	const CCfgData *cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_H323_CS_ERROR_HANDLE_FIRST_TIMER_VAL);
	if(true == CCfgData::TestValidity(cfgData))
	{
		m_CommonParams.push_back(cfgData);
	}
	
	cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_H323_CS_ERROR_HANDLE_SECOND_TIMER_VAL);
	if(true == CCfgData::TestValidity(cfgData))
	{
		m_CommonParams.push_back(cfgData);
	}
	
	cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_DEBUG_MODE);
	if(true == CCfgData::TestValidity(cfgData))
	{
		m_CommonParams.push_back(cfgData);
	}

	cfgData = sysConfig->GetCfgEntryByKey(CS_KEY_H323_RAS_IPV6);
	if(true == CCfgData::TestValidity(cfgData))
	{
		m_CommonParams.push_back(cfgData);
	}
}

//////////////////////////////////////////////////////////////////////
CCommStartupService::~CCommStartupService()
{

}

//////////////////////////////////////////////////////////////////////
STATUS CCommStartupService::SendToCsApi(OPCODE opcode, const int dataLen, const char * data)
{
	STATUS status = CCommCsModuleService::SendToCsApi(opcode, eStartUp, dataLen, data, m_csId);

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommStartupService::SendNewReq(DWORD numPorts)
{
	CS_New_Req_S newReq;
	newReq.num_h323_ports = (WORD)(numPorts);
	newReq.num_sip_ports  = (WORD)(numPorts);
//	newReq.lan_cfg_bla_bla_bla

	TRACEINTO << CNewReqWrapper(newReq);

	STATUS res = SendToCsApi(CS_NEW_REQ, sizeof(CS_New_Req_S), (char*)&newReq);
	
	return res;	
}

//////////////////////////////////////////////////////////////////////
STATUS CCommStartupService::SendConfigParam(bool isFirstTime)
{
	static CCfgParamsVector::iterator iTer = m_CommonParams.end();
	static CCfgParamsVector::iterator iEnd = m_CommonParams.end();
	
	if(isFirstTime)
	{
		iTer = m_CommonParams.begin();
		iEnd = m_CommonParams.end();
	}
	
	while(iTer != iEnd)
	{
		CS_Config_Req_S req;
		const CCfgData *pCfgEntry = *iTer;
		FillConfigParam(req, *pCfgEntry);
		
//		TRACEINTO << CConfigReqWrapper(req);

		static const DWORD CS_Config_Req_SLen = sizeof(CS_Config_Req_S);

		STATUS status = SendToCsApi(	CS_CONFIG_PARAM_REQ, 
										CS_Config_Req_SLen, 
										(char*)&req
									);
		iTer++;
	
		if(STATUS_OK == status)
		{
			return status;
		}
	}
	
	return STATUS_END_CONFIG;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommStartupService::SendEndConfigParamReq()
{
	CS_End_Config_Req_S param;

//	TRACEINTO << CEndConfigReqWrapper(param);
	
	static const DWORD CS_End_Config_Req_SLen = sizeof(CS_End_Config_Req_S);
	
	STATUS res = SendToCsApi(	CS_END_CONFIG_PARAM_REQ, 
								CS_End_Config_Req_SLen,
								(char*)&param);
		
	return res;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommStartupService::SendConfigParamSingle(const CCfgData *pCfgEntry)
{
	CS_Config_Req_S req;
	memset(&req, 0, sizeof(CS_Config_Req_S));
	FillConfigParam(req, *pCfgEntry);

	STATUS status = SendToCsApi(	CS_CONFIG_PARAM_REQ, 
									sizeof(CS_Config_Req_S), 
									(char*)&req
								);
	return status;
}

//////////////////////////////////////////////////////////////////////
//TBD - jud - sending nothing to CS. is it needed???
STATUS CCommStartupService::SendLanCfgReq()
{
	CS_Lan_Cfg_Req_S param;

//	TRACEINTO << CLanCfgReqWrapper(param);

	static const DWORD CS_Lan_Cfg_Req_SLen = sizeof(CS_Lan_Cfg_Req_S);
	
	STATUS res = SendToCsApi(CS_LAN_CFG_REQ, CS_Lan_Cfg_Req_SLen, (char*)&param);
		
	return res;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommStartupService::ReceiveNumOfPortsInd(CSMNGR_LICENSING_S *param)
{
	TRACEINTO << CNumOfPortsIndWrapper(*param);
	
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommStartupService::ReceiveNewInd(CS_New_Ind_S *param)
{
//	TRACEINTO << CNewIndWrapper(*param);	
	
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommStartupService::ReceiveConfigInd(CS_Config_Ind_S *param)
{
//	TRACEINTO << CConfigIndWrapper(*param);
	
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommStartupService::ReceiveEndConfigInd(CS_End_Config_Ind_S *param)
{
//	TRACEINTO << CEndConfigIndWrapper(*param);
	
	return STATUS_OK;
}
	
//////////////////////////////////////////////////////////////////////	
STATUS CCommStartupService::ReceiveEndStartupInd(CS_End_StartUp_Ind_S *param)
{
//	TRACEINTO << CEndStartupIndWrapper(*param);
	
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CCommStartupService::FillConfigParam(CS_Config_Req_S &req, const CCfgData &cfgEntry)const
{
  strncpy(req.section, cfgEntry.GetSection().c_str(), sizeof(req.section)-1);
  req.section[sizeof(req.section)-1] = '\0';

  strncpy(req.key, cfgEntry.GetKey().c_str(), sizeof(req.key)-1);
  req.key[sizeof(req.key)-1] = '\0';

  strncpy(req.data, cfgEntry.GetData().c_str(), sizeof(req.data)-1);
  req.data[sizeof(req.data)-1] = '\0';
}

//////////////////////////////////////////////////////////////////////
void CCommStartupService::SetCsId(WORD id)
{
	m_csId = id;
}
//////////////////////////////////////////////////////////////////////
