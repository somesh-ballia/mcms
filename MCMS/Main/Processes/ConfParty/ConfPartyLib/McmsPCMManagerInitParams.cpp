#include "McmsPCMManagerInitParams.h"
#include "HostCommonDefinitions.h"

// ------------------------------------------------------------
	CMcmsPCMMngrInitParams::CMcmsPCMMngrInitParams() : 
	m_pConf(NULL), m_pConfName(NULL), m_confRsrcId(DUMMY_CONF_ID),m_pAudioBridgeInterface(NULL), m_pVideoBridgeInterface(NULL)
{

}

// ------------------------------------------------------------
CMcmsPCMMngrInitParams::CMcmsPCMMngrInitParams (const CConf* pConf, const char*	pConfName,
												const DWORD confRsrcId,
												CAudioBridgeInterface* pAudBrdgInterface,
												CVideoBridgeInterface* pVideoBridgeInterface,
												CConfAppMngrInterface* pConfAppMngrInterface):
	m_pConf(pConf), m_pConfName(pConfName), m_confRsrcId(confRsrcId), m_pAudioBridgeInterface(pAudBrdgInterface),m_pVideoBridgeInterface(pVideoBridgeInterface),m_pConfAppMngrInterface(pConfAppMngrInterface)
{

}
// ------------------------------------------------------------
const char* CMcmsPCMMngrInitParams::NameOf () const
{
	return "CMcmsPCMMngrInitParams";
}
// ------------------------------------------------------------
const CConf* CMcmsPCMMngrInitParams::GetConf() const
{
	return m_pConf;
}
// ------------------------------------------------------------
const char*  CMcmsPCMMngrInitParams::GetConfName() const
{
	return m_pConfName;
}
// ------------------------------------------------------------
DWORD CMcmsPCMMngrInitParams::GetConfRsrcId() const 
{
	return m_confRsrcId;
}
// ------------------------------------------------------------
void CMcmsPCMMngrInitParams::SetConfRsrcId(DWORD confRsrcId)
{
	m_confRsrcId = confRsrcId;
}
// ------------------------------------------------------------
CAudioBridgeInterface*	CMcmsPCMMngrInitParams::GetAudioBridgeInterface() const
{
	return m_pAudioBridgeInterface;
}
// ------------------------------------------------------------
CVideoBridgeInterface*	CMcmsPCMMngrInitParams::GetVideoBridgeInterface() const
{
	return m_pVideoBridgeInterface;
}
// ------------------------------------------------------------
CConfAppMngrInterface*  CMcmsPCMMngrInitParams::GetConfAppMngrInterface() const
{
	return m_pConfAppMngrInterface;
}
