#if !defined(_MoveParams_H__)
#define _MoveParams_H__

#include "PObject.h"
#include "ConfPartyDefines.h"
#include "DataTypes.h"
#include "PartyCntl.h"

class CConf;

  //char          m_name[H243_NAME_LEN];]
  //DWORD         m_sysConfId;
 class CAudioBridgeInterface;
 class CVideoBridgeInterface;
 class CConfAppMngrInterface;
 class CCopVideoTxModes;
  // CFECCBridge
  //class CIpComMode;
  //BYTE                   m_networkInterface;
 // class CCapH323;
  //DWORD  
    
class CMoveIPImportParams: public CPObject 
{
 CLASS_TYPE_1(CMoveIPImportParams,CPObject)
  public:

    CMoveIPImportParams();
    CMoveIPImportParams(CConf* pConf, CPartyCntl*	pImpConfPartyCntl, char* confName, DWORD sysConfId,
		CAudioBridgeInterface* pAudBridgeInterface, CVideoBridgeInterface* pVidBridgeInterface,
		CConfAppMngrInterface* pConfAppMngrInterface, CFECCBridge* pFECCBridge, CContentBridge*	pContentBridge,
		CTerminalNumberingManager* pTerminalNumberingManager, CCopVideoTxModes* pCopVideoTxModes);
   CMoveIPImportParams(const CMoveIPImportParams& otherMoveParams);   
   virtual ~CMoveIPImportParams();
  CMoveIPImportParams& operator=(CMoveIPImportParams& otherMoveParams);
  WORD GetInterfaceType() { return m_pImpConfPartyCntl->GetInterfaceType();};
  virtual const char* NameOf() const { return "CMoveIPImportParams";}
  // members
  CConf*        m_pConf;
  CPartyCntl*	m_pImpConfPartyCntl;
  char          m_name[H243_NAME_LEN];
  DWORD         m_sysConfId;
  CAudioBridgeInterface*     m_pAudBridgeInterface;
  CVideoBridgeInterface*     m_pVidBridgeInterface;
  CConfAppMngrInterface*     m_pConfAppMngrInterface;
  CFECCBridge*               m_pFECCBridge;
  CContentBridge*			 m_pContentBridge;	
  CTerminalNumberingManager* m_pTerminalNumberingManager;
  CCopVideoTxModes*			 m_pCopVideoTxModes;
  eVideoPartyType            m_eLastAllocatedVideoPartyType;
  //CIpComMode*            m_pIpPartyScm;
 // BYTE                   m_networkInterface;
  //CCapH323*              m_pCap; // temporary until enevlope will be defined
  //DWORD                  m_videoBitRate; 

};

#endif
