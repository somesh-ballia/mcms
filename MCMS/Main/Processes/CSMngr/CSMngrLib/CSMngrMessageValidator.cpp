#include "CSMngrMessageValidator.h"
#include "StatusesGeneral.h"
#include "IpCsOpcodes.h"
#include "CsStructs.h"
#include "MplMcmsProtocol.h"
#include "ProcessBase.h"
#include "ObjString.h"



static CProcessBase *pProcess = NULL;

//////////////////////////////////////////////////////////////////////
CCSMngrMessageValidator::CCSMngrMessageValidator()
{
	pProcess = CProcessBase::GetProcess();
	
	InitOpcodeLenMap();
}

//////////////////////////////////////////////////////////////////////
CCSMngrMessageValidator::~CCSMngrMessageValidator()
{
}

//////////////////////////////////////////////////////////////////////
void CCSMngrMessageValidator::InitOpcodeLenMap()
{
	AddOpcodeLenPair(CS_NEW_IND				, sizeof(CS_New_Ind_S)			);
	AddOpcodeLenPair(CS_CONFIG_PARAM_IND	, sizeof(CS_Config_Ind_S)		);
	AddOpcodeLenPair(CS_END_CONFIG_PARAM_IND, sizeof(CS_End_Config_Ind_S)	);
	AddOpcodeLenPair(CS_END_CS_STARTUP_IND	, sizeof(CS_End_StartUp_Ind_S)	);
}

//////////////////////////////////////////////////////////////////////
STATUS CCSMngrMessageValidator::ValidateMessageLen(CMplMcmsProtocol &mplMcmsProtocol)
{
	STATUS status = STATUS_OK;
	
	OPCODE	opcode	= mplMcmsProtocol.getOpcode();	
	bool isExist = m_OpcodeSizeMap.IsOpcodeExist(opcode);
	if(true == isExist)
	{
		DWORD expectedLen = m_OpcodeSizeMap.GetLenByOpcode(opcode);
		status = mplMcmsProtocol.ValidateDataSize(expectedLen);
	}
	
	return status;
}

//////////////////////////////////////////////////////////////////////
void CCSMngrMessageValidator::AddOpcodeLenPair(OPCODE opcode, DWORD len)
{
	bool isExist = m_OpcodeSizeMap.IsOpcodeExist(opcode);
	if(false == isExist)
	{
		m_OpcodeSizeMap.AddOpcodeLen(opcode, len);
		return;
	}
	
	CLargeString message = "Double addition of pair<OPCODE, LEN>;\n";
	message << "Opcode : " << pProcess->GetOpcodeAsString(opcode).c_str() << ". Len : " << len;
		
	PASSERTMSG(TRUE, message.GetString());
}



