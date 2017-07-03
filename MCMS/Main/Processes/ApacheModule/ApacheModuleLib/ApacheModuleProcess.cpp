// ApacheModuleProcess.cpp: implementation of the CApacheModuleProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "ApacheModuleProcess.h"
#include "CardsStructs.h"
#include "LogOutRequest.h"
#include "StringsMaps.h"
#include "TraceStream.h"

extern void ApacheModuleManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CApacheModuleProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CApacheModuleProcess::GetManagerEntryPoint()
{
	return ApacheModuleManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
CApacheModuleProcess::CApacheModuleProcess()
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	m_pAuthenticationStruct = new MCMS_AUTHENTICATION_S;
	m_pAuthenticationStruct->productType		= (DWORD)curProductType;
	m_pAuthenticationStruct->switchBoardId		= 0;
	m_pAuthenticationStruct->switchSubBoardId	= 1;
	m_pAuthenticationStruct->rmxSystemCardsMode	= (DWORD)( GetRmxSystemCardsModeDefault() );

	m_pLicensingStruct = new APACHEMODULE_LICENSING_S;
	m_pLicensingStruct->num_cop_parties	= 0;
	m_pLicensingStruct->num_cp_parties	= 0;

	bzero(&m_pLicensingStruct->avcSvcCap, sizeof(m_pLicensingStruct->avcSvcCap));

	m_isAuthenticationStructureAlreadyReceived	= NO;
	m_bWaitingLicensingInd = TRUE;
	m_bBlockRequests = FALSE;
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleProcess::AddExtraStringsToMap()
{
	CStringsMaps::AddItem(DISCONNECT_REASON_ENUM,logoutNormal,"normal");
    CStringsMaps::AddItem(DISCONNECT_REASON_ENUM,logoutSessionExpired,"session_expired");
}


//////////////////////////////////////////////////////////////////////
CApacheModuleProcess::~CApacheModuleProcess()
{
	delete m_pAuthenticationStruct;
	delete m_pLicensingStruct;
}

////////////////////////////////////////////////////////////////////////////
void CApacheModuleProcess::SetAuthenticationStruct(MCMS_AUTHENTICATION_S* authentStruct)
{
	m_pAuthenticationStruct->productType = authentStruct->productType;
}

////////////////////////////////////////////////////////////////////////////
MCMS_AUTHENTICATION_S*  CApacheModuleProcess::GetAuthenticationStruct() const
{
	return m_pAuthenticationStruct;
}

////////////////////////////////////////////////////////////////////////////
void CApacheModuleProcess::SetLicensingStruct(APACHEMODULE_LICENSING_S* licensingStruct)
{
	m_pLicensingStruct->num_cop_parties	= licensingStruct->num_cop_parties;
	m_pLicensingStruct->num_cp_parties	= licensingStruct->num_cp_parties;
}

void CApacheModuleProcess::SetLicensingStructLicenseMode(int LicenseMode)
{
	m_pLicensingStruct->avcSvcCap.licenseMode = LicenseMode;

}

////////////////////////////////////////////////////////////////////////////
APACHEMODULE_LICENSING_S*  CApacheModuleProcess::GetLicensingStruct() const
{

	return m_pLicensingStruct;
}

////////////////////////////////////////////////////////////////////////////
void CApacheModuleProcess::SetIsAuthenticationStructureAlreadyReceived(BOOL isReceived)
{
	m_isAuthenticationStructureAlreadyReceived = isReceived;
}

////////////////////////////////////////////////////////////////////////////
BOOL CApacheModuleProcess::GetIsAuthenticationStructureAlreadyReceived()
{
	return m_isAuthenticationStructureAlreadyReceived;
}

////////////////////////////////////////////////////////////////////////////
BOOL CApacheModuleProcess::IsFederalOn()
{
	BOOL result = FALSE;
	std::string file = MCU_MCMS_DIR+"/JITC_MODE.txt";
	char szFileLine[5] = "";

	FILE* pFile = fopen(file.c_str(), "r");
	if(pFile)
	{
		fgets(szFileLine,4,pFile);
        fclose(pFile);
        
	}

	if(!strncmp(szFileLine, "YES", 3))
	{
		result = TRUE;
	}

	return result;

}

////////////////////////////////////////////////////////////////////////////
void CApacheModuleProcess::SetWaitingLicensingInd(BOOL bWaitingLicensingInd)
{
	m_bWaitingLicensingInd = bWaitingLicensingInd;
}
////////////////////////////////////////////////////////////////////////////
BOOL CApacheModuleProcess::GetWaitingLicensingInd()
{
	return m_bWaitingLicensingInd;
}

