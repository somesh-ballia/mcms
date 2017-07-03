
#include "MoveParams.h"
#include "string.h"

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CMoveIPImportParams::CMoveIPImportParams(CConf* pConf, CPartyCntl*	pImpConfPartyCntl, char* confName, DWORD sysConfId,
		CAudioBridgeInterface* pAudBridgeInterface, CVideoBridgeInterface* pVidBridgeInterface,
		CConfAppMngrInterface* pConfAppMngrInterface, CFECCBridge* pFECCBridge,CContentBridge* pContentBridge ,CTerminalNumberingManager* pTerminalNumberingManager, CCopVideoTxModes* pCopVideoTxModes):
		m_pConf(pConf),m_pImpConfPartyCntl(pImpConfPartyCntl),
		m_sysConfId(sysConfId),m_pAudBridgeInterface(pAudBridgeInterface), m_pVidBridgeInterface(pVidBridgeInterface),
		m_pConfAppMngrInterface(pConfAppMngrInterface), m_pFECCBridge(pFECCBridge),m_pContentBridge(pContentBridge) ,m_pTerminalNumberingManager(pTerminalNumberingManager), m_pCopVideoTxModes(pCopVideoTxModes)
{
	strncpy(m_name,confName ,H243_NAME_LEN-1);
	//PTRACE(eLevelInfoNormal,"CMoveIPImportParams::CMoveIPImportParams");
	//(CSipPartyCntl*)pImpConfPartyCntl->DumpUdpAddresses();
    m_name[H243_NAME_LEN -1]='\0';
    m_eLastAllocatedVideoPartyType = pImpConfPartyCntl->GetLastAllocated();
}
//////////////////////////////////////////////////////////////////////
CMoveIPImportParams::CMoveIPImportParams()
{
}
//////////////////////////////////////////////////////////////////////
CMoveIPImportParams::CMoveIPImportParams(const CMoveIPImportParams& otherMoveParams):CPObject(otherMoveParams)
{
	m_pConf = otherMoveParams.m_pConf;
    m_pImpConfPartyCntl = otherMoveParams.m_pImpConfPartyCntl;
    strncpy(m_name,otherMoveParams.m_name,H243_NAME_LEN-1);
    m_name[H243_NAME_LEN -1]='\0';
    m_sysConfId 				= otherMoveParams.m_sysConfId;
    m_pAudBridgeInterface		= otherMoveParams.m_pAudBridgeInterface;
    m_pVidBridgeInterface 		= otherMoveParams.m_pVidBridgeInterface;
    m_pConfAppMngrInterface 	= otherMoveParams.m_pConfAppMngrInterface;
    m_pFECCBridge				= otherMoveParams.m_pFECCBridge;
    m_pContentBridge 	        = otherMoveParams.m_pContentBridge;
    m_pTerminalNumberingManager = otherMoveParams.m_pTerminalNumberingManager;
    m_pCopVideoTxModes			= otherMoveParams.m_pCopVideoTxModes;
    m_eLastAllocatedVideoPartyType = otherMoveParams.m_eLastAllocatedVideoPartyType;
}
////////////////////////////////////////////////////////////////////
   CMoveIPImportParams& CMoveIPImportParams::operator=(CMoveIPImportParams& otherMoveParams)
{
	if(this == &otherMoveParams){
	    return *this;
	}

	m_pConf = otherMoveParams.m_pConf;
    m_pImpConfPartyCntl = otherMoveParams.m_pImpConfPartyCntl;
    strncpy(m_name,otherMoveParams.m_name,H243_NAME_LEN-1);
    m_name[H243_NAME_LEN -1]='\0';
    m_sysConfId 				= otherMoveParams.m_sysConfId;
    m_pAudBridgeInterface 		= otherMoveParams.m_pAudBridgeInterface;
    m_pVidBridgeInterface 		= otherMoveParams.m_pVidBridgeInterface;
    m_pConfAppMngrInterface 	= otherMoveParams.m_pConfAppMngrInterface;
    m_pFECCBridge				= otherMoveParams.m_pFECCBridge;
    m_pContentBridge            = otherMoveParams.m_pContentBridge;
    m_pTerminalNumberingManager = otherMoveParams.m_pTerminalNumberingManager;
    m_pCopVideoTxModes			= otherMoveParams.m_pCopVideoTxModes;
    m_eLastAllocatedVideoPartyType = otherMoveParams.m_eLastAllocatedVideoPartyType;
    
    return *this;    
}
//////////////////////////////////////////////////////////////////////
CMoveIPImportParams::~CMoveIPImportParams()
{
}
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////

