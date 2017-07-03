#include "CustomizeDisplaySettingForOngoingConfConfigurationDBGet.h"
#include "ConfPartyProcess.h"
#include "CustomizeDisplaySettingForOngoingConfConfiguration.h"

CCustomizeDisplaySettingForOngoingConfConfigurationDBGet::CCustomizeDisplaySettingForOngoingConfConfigurationDBGet()
{
	
}

CCustomizeDisplaySettingForOngoingConfConfigurationDBGet::~CCustomizeDisplaySettingForOngoingConfConfigurationDBGet()
{
}

CCustomizeDisplaySettingForOngoingConfConfigurationDBGet::CCustomizeDisplaySettingForOngoingConfConfigurationDBGet(const CCustomizeDisplaySettingForOngoingConfConfigurationDBGet &other)
:CSerializeObject(other)
{
	//*m_pFailoverConfiguration = *other.m_pFailoverConfiguration;

}

CCustomizeDisplaySettingForOngoingConfConfigurationDBGet& CCustomizeDisplaySettingForOngoingConfConfigurationDBGet::operator=(const CCustomizeDisplaySettingForOngoingConfConfigurationDBGet& other)
{
	
	return *this;
}


void CCustomizeDisplaySettingForOngoingConfConfigurationDBGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CCustomizeDisplaySettingForOngoingConfConfiguration* pCustomizeSetting =  ((CConfPartyProcess*) (CConfPartyProcess::GetProcess()))->GetCustomizeDisplaySettingForOngoingConfConfiguration();
	pCustomizeSetting->SerializeXml(pFatherNode);
}

int  CCustomizeDisplaySettingForOngoingConfConfigurationDBGet::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	return STATUS_OK;
}

