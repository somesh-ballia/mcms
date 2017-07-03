// CommStartupService.h: interface for the CCommStartupService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with CS Module. 
//								Startup (it sends/receives structs)
//========   ==============   =====================================================================


#ifndef __COMMSTARTUPSERVICE_H__
#define __COMMSTARTUPSERVICE_H__

#include "CommCsModuleService.h"
#include "CsStructs.h"
#include "SysConfig.h"


class CCfgKeyData;
struct CSMNGR_LICENSING_S;





class CCommStartupService : public CCommCsModuleService  
{
public:
	CCommStartupService();
	virtual ~CCommStartupService();
	const char * NameOf(void) const {return "CCommStartupService";}

	void InitCommonParams();
		
	// startup state
	STATUS SendNewReq(DWORD numPorts);
	STATUS SendConfigParam(bool isFirstTime);
	STATUS SendEndConfigParamReq();

	// ready state
	STATUS SendLanCfgReq();
	STATUS SendReconnectReq();
	STATUS SendConfigParamSingle(const CCfgData *pCfgEntry);
	
	
	STATUS ReceiveNumOfPortsInd(CSMNGR_LICENSING_S *param);
	STATUS ReceiveNewInd(CS_New_Ind_S *param);
	STATUS ReceiveConfigInd(CS_Config_Ind_S *param);
	STATUS ReceiveEndConfigInd(CS_End_Config_Ind_S *param);
	STATUS ReceiveEndStartupInd(CS_End_StartUp_Ind_S *param);	
	
	void   SetCsId(WORD id);

	
private:
	void FillConfigParam(CS_Config_Req_S &req, const CCfgData &cfgEntry)const;
	virtual STATUS SendToCsApi(OPCODE opcode, const int dataLen, const char * data);


	CCfgParamsVector m_CommonParams;
	
	int m_csId;
};

#endif // __COMMSTARTUPSERVICE_H__
