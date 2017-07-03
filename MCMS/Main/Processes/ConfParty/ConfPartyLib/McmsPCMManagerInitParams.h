//+========================================================================+
//                   McmsPCMMngrInitParams.H                               |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       McmsPCMMngrInitParams.H                                     |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2005  | Description                                    |
//-------------------------------------------------------------------------|

#ifndef __MCMS_PCM_MNGR_INIT_PARAMS_H__
#define __MCMS_PCM_MNGR_INIT_PARAMS_H__

#include "ConfPartyDefines.h"

class CAudioBridgeInterface;
class CVideoBridgeInterface;
class CConfAppMngrInterface;
class CConf;

class CMcmsPCMMngrInitParams : public CPObject
{
CLASS_TYPE_1(CBridgeInitParams,CPObject)
public:
	CMcmsPCMMngrInitParams();
	CMcmsPCMMngrInitParams (const CConf* pConf, const char*	pConfName,
							const DWORD confRsrcId,
							CAudioBridgeInterface* pAudBrdgInterface,
							CVideoBridgeInterface* pVideoBridgeInterface,
							CConfAppMngrInterface* pConfAppMngrInterface
							);
				
	~CMcmsPCMMngrInitParams (){}
	const char* NameOf() const;

	const CConf* GetConf() const; 
	const char*  GetConfName() const;
	DWORD GetConfRsrcId() const;
	void SetConfRsrcId(DWORD confRsrcId);
	CAudioBridgeInterface*	GetAudioBridgeInterface() const;
	CVideoBridgeInterface*	GetVideoBridgeInterface() const;
	CConfAppMngrInterface*  GetConfAppMngrInterface() const;
	
protected:
	const CConf*			m_pConf;
	const char*				m_pConfName;
	DWORD					m_confRsrcId;
	CAudioBridgeInterface*	m_pAudioBridgeInterface;
	CVideoBridgeInterface*	m_pVideoBridgeInterface;
	CConfAppMngrInterface*  m_pConfAppMngrInterface;
	};

#endif //__MCMS_PCM_MNGR_INIT_PARAMS_H__


