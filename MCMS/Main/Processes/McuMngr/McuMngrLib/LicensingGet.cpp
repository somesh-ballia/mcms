// LicensingGet.cpp: implementation of the CLicensingGet class.
//////////////////////////////////////////////////////////////////////////


#include "LicensingGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SystemTime.h"
#include "Licensing.h"
#include "TraceStream.h"

#include "ApiStatuses.h"

#include "PlcmLicenseAuthorityConfig.h"
#include "PlcmLicenseStatus.h"
#include "LicenseStatusApiEnums.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLicensingGet::CLicensingGet()
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CLicensingGet& CLicensingGet::operator = (const CLicensingGet &other)
{
	if(this == &other){
	    return *this;
	}

	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
	m_apiFormat = other.m_apiFormat;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CLicensingGet::~CLicensingGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CLicensingGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	if (m_pProcess == NULL) return;
	CLicensing* pLicense = m_pProcess->GetLicensing();
	TRACESTR(eLevelInfoNormal) << "CLicensingGet::SerializeXml, m_apiFormat=" << m_apiFormat;
	if (pLicense && m_apiFormat==eRestApi)
	{
		//todo: instead of using PlcmLicenseAuthorityConfig use a new object PlcmLicenseStatus

		PlcmLicenseStatus plcmLicenseStatus;

		plcmLicenseStatus.m_connectionStatus = pLicense->ConvertConnectionStatus();
		if (plcmLicenseStatus.m_connectionStatus == eConnectionStatus_CONNECTIONNOTATTEMPTED)
			plcmLicenseStatus.m_licensingStatus = eLicensingStatus_UNKNOWN;

		else plcmLicenseStatus.m_licensingStatus    = pLicense->ConvertLicenseStatus();

		plcmLicenseStatus.m_lastSuccessfulDate = pLicense->GetLastSuccesfulDateAsStr();
		plcmLicenseStatus.m_lastAttemptDate = pLicense->GetLastAttemptDateAsStr();

		//plcmLicenseStatus.

		plcmLicenseStatus.m_entityTag = pLicense->GetLicenseServerParams()->GetLicenseChangedCounter();
		plcmLicenseStatus.m_plcmLicenseAuthorityConfig.m_mainServerAddress.m_host = pLicense->GetLicenseServerParams()->GetPrimaryLicenseServer();
		plcmLicenseStatus.m_plcmLicenseAuthorityConfig.m_mainServerAddress.m_portNumber = pLicense->GetLicenseServerParams()->GetPrimaryLicenseServerPort();
		plcmLicenseStatus.m_plcmLicenseAuthorityConfig.m_useAutoDiscovery = false;
		plcmLicenseStatus.m_plcmLicenseAuthorityConfig.m_currentStatus = "";
		//plcmLicense.m_connectionStatus = eConnectionStatus_NotAttempt;//convertConnectionStatus(pLicense->GetLicenseStatus());
		//todo: set new fields to plcmLicense.
		//check right fields


		for (int i=0 ; i< MAX_NUM_OF_FEATURES;i++)
		{

			PlcmLicenseFeature item;

            item.m_name = ::GetLicensingFeaturesStr((E_FLEXERA_LICENSE_FEATURES)i);



			if (i >= MAX_NUM_OF_FEATURES ) break;

			if (plcmLicenseStatus.m_connectionStatus == eConnectionStatus_CONNECTIONNOTATTEMPTED)
				item.m_plcmLicenseFeatureStatus = eLicenseFeatureStatus_NOTSTARTED;
			else
			    item.m_plcmLicenseFeatureStatus = m_pProcess->ConvertFeatureStatus(m_pProcess->GetFlexeraCapabilityStatus(i));

			if (RPCS_MAX_PORTS == i)
				item.m_counted = true;
			else
				item.m_counted = false;

			item.m_count = m_pProcess->GetFlexeraCapabilityCounted(i);

			item.m_plcmLicenseFeatureVersion.m_majorId = 1;  //its hard coded. it should be changed for EE-490
			item.m_plcmLicenseFeatureVersion.m_minorId = 0;

			plcmLicenseStatus.m_plcmLicenseFeatureList.m_plcmLicenseFeature.push_back(item);

		}







		m_apiBaseObjRest = ApiBaseObjectPtr(&plcmLicenseStatus);
	}
	else if (pLicense)
	{
		pLicense->SerializeXml(pActionsNode);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CLicensingGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	//GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD); Not implemented yet
	WORD tmp = 0;
	GET_VALIDATE_CHILD(pNode,"API_FORMAT",&tmp,API_FORMAT_TYPE_ENUM);
	if (tmp)
	{
		TRACESTR(eLevelInfoNormal) << "CLicensingGet::DeSerializeXml, tmp=" << tmp;
		m_apiFormat = (eApiFormat)tmp;
		TRACESTR(eLevelInfoNormal) << "CLicensingGet::DeSerializeXml, m_apiFormat=" << m_apiFormat;
	}
	return STATUS_OK;
}

