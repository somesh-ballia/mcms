// LogInConfirm.cpp: implementation of the CLogInConfirm class.
//
//////////////////////////////////////////////////////////////////////

#include "LogInConfirm.h"

#include "string.h"
#include "psosxml.h"
#include "XmlDefines.h"
#include "Transactions.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApacheDefines.h"
#include "ApiStatuses.h"
#include "StringsMaps.h"
#include "Trace.h"
#include "TraceStream.h"

#include "ProductType.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"

/*Begin:added by Richer for BRIDGE-14278 , 2014.7.29*/
#include "SystemFunctions.h"
/*End:added by Richer for BRIDGE-14278 , 2014.7.29*/

// colinzuo: we can modify ApiCom to add a new mode for meridian if needed, but for now I don't think we need
#ifndef eSystemCardsMode_meridian
#define eSystemCardsMode_meridian NUM_OF_SYSTEM_CARDS_MODES
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CLogInConfirm::CLogInConfirm(BYTE bMultipleServices, BYTE bV35JitcSupport)
{
	m_MCMS_Version.ver_major    = 0;
	m_MCMS_Version.ver_minor    = 0;
	m_MCMS_Version.ver_release  = 0;
	m_MCMS_Version.ver_internal = 0;
	m_MCU_Version.ver_major     = 0;
	m_MCU_Version.ver_minor     = 0;
	m_MCU_Version.ver_release   = 0;
	m_MCU_Version.ver_internal  = 0;

	m_compatibilityLevel = 0;
	m_apiNumber          = 0;
	m_reserved1          = 0;
	m_reserved2          = 0;

	m_loginConfirmProductType = CProcessBase::GetProcess()->GetProductType();

	m_numCpParties       = 0;
	m_numCopParties      = 0;
	m_productType        = CProcessBase::GetProcess()->GetProductType();

    // SAGI patch for working with old RMX2000 - EMA
//    if (m_productType == eProductTypeCallGenerator || eProductTypeSoftMCU == m_productType || eProductTypeSoftMCUMfw == m_productType)
//        m_productType = eProductTypeRMX2000;

#ifdef LINUX
	m_operating_system   = OS_LINUX;
#else
	m_operating_system   = OS_XPE;
#endif

	m_authorizationGroup = 0;
	m_loginInfoFlag      = 0;
	m_HttpPort           = 0;
	m_mcuToken           = 0;
	m_eMasterSlaveCurrentState = 0;

    /*Begin:added by Richer for BRIDGE-14278 , 2014.7.29*/
    m_bSimulationMode = false;
    
    if (true == IsRmxSimulation())
    {
        m_bSimulationMode = true;
    }
    /*End:added by Richer for BRIDGE-14278 , 2014.7.29*/

	m_rmxSystemCardsMode = GetRmxSystemCardsModeDefault();
	m_ramSize = GetRamSizeAccordingToTotalMemory("CLogInConfirm::CLogInConfirm");

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = CFG_KEY_JITC_MODE;
	BOOL bJITCMode;
	sysConfig->GetBOOLDataByKey(key, bJITCMode);
	m_bJITC_Mode		= bJITCMode;

	key = CFG_KEY_LAST_LOGIN_ATTEMPTS;
	BOOL res = sysConfig->GetBOOLDataByKey(key, m_bLastLoginAttempts);
	if (!res)
	{
	    PASSERTSTREAM(true, "Unable to find " << key);
	    m_bLastLoginAttempts = false;
	}

	key = CFG_KEY_SESSION_TIMEOUT_IN_MINUTES;
	DWORD sessionTimeout;
	sysConfig->GetDWORDDataByKey(key, sessionTimeout);
	m_SessionTimeoutInMinutes = sessionTimeout;

	DWORD cgfPwdExpirationWarningPeriod = 0;
	sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_EXPIRATION_WARNING_DAYS, cgfPwdExpirationWarningPeriod);
	m_pwdExpirationWarningPeriod = cgfPwdExpirationWarningPeriod;

	m_daysUntilPwdExpires = 0;

	BOOL isHidePsw = NO;
	std::string key_hide = "HIDE_CONFERENCE_PASSWORD";
	sysConfig->GetBOOLDataByKey(key_hide, isHidePsw);
	m_bHidePsw = isHidePsw;

	BOOL isNetworkSeparation = NO;
	std::string key_net_sep = CFG_KEY_SEPARATE_NETWORK;
	sysConfig->GetBOOLDataByKey(key_net_sep, isNetworkSeparation);
	m_bNetworkSeparation = isNetworkSeparation;
	m_bAudibleAlarmEnable = 0;
	//CAudibleAlarm *pCAudibleAlarm
    m_bMultipleServices = bMultipleServices;
    m_bV35JitcSupport = bV35JitcSupport;
	m_bMachineAccount = false;


	BOOL isVideoPreviewEnable = NO;
	std::string key_preview = ENABLE_VIDEO_PREVIEW;
	sysConfig->GetBOOLDataByKey(key_preview, isVideoPreviewEnable);
	m_bVideoPreviewEnable = isVideoPreviewEnable;

	bzero(&m_avcSvcCap, sizeof(m_avcSvcCap));
}

/////////////////////////////////////////////////////////////////////////////
CLogInConfirm::~CLogInConfirm()
{
}

int CLogInConfirm::IsPlatformGesherNinja() const
{
    return (0
            || (eProductTypeGesher==m_loginConfirmProductType)
            || (eProductTypeNinja==m_loginConfirmProductType)
           );
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SerializeXml(CXMLDOMElement*& pXMLResponse) const
{
  	CXMLDOMElement *pVersionListNode,*pVersionNode, *pLoginRecords;
	pVersionListNode=pXMLResponse->AddChildNode("VERSION_LIST");

	if (pVersionListNode)
	{

		pVersionNode=pVersionListNode->AddChildNode("MCU_VERSION");
		if (pVersionNode)
		{
			pVersionNode->AddChildNode("MAIN",m_MCU_Version.ver_major);
			pVersionNode->AddChildNode("MAJOR",m_MCU_Version.ver_minor);
			pVersionNode->AddChildNode("MINOR",m_MCU_Version.ver_release);
			if(m_bJITC_Mode)
			{
				//pVersionNode->AddChildNode("INTERNAL","");
				pVersionNode->AddChildNode("PRIVATE_DESCRIPTION","J");
			}else
			{
				pVersionNode->AddChildNode("INTERNAL",m_MCU_Version.ver_internal);
				pVersionNode->AddChildNode("PRIVATE_DESCRIPTION",m_mcuPrivateDesc);
				pVersionNode->AddChildNode("DESCRIPTION",m_mcuDesc);
			}
		}

		pVersionNode=pVersionListNode->AddChildNode("MCMS_VERSION");
		if (pVersionNode)
		{
			pVersionNode->AddChildNode("MAIN",m_MCMS_Version.ver_major);
			pVersionNode->AddChildNode("MAJOR",m_MCMS_Version.ver_minor);
			pVersionNode->AddChildNode("MINOR",m_MCMS_Version.ver_release);
			if(m_bJITC_Mode)
			{
				//pVersionNode->AddChildNode("INTERNAL","");
				pVersionNode->AddChildNode("PRIVATE_DESCRIPTION","J");
			}else
			{
				pVersionNode->AddChildNode("INTERNAL",m_MCMS_Version.ver_internal);
				pVersionNode->AddChildNode("PRIVATE_DESCRIPTION",m_mcmsPrivateDesc);
			}
		}
	}

	pXMLResponse->AddChildNode("AUTHORIZATION_GROUP",m_authorizationGroup,AUTHORIZATION_GROUP_ENUM);
	pXMLResponse->AddChildNode("API_NUMBER",API_NUMBER);
	pXMLResponse->AddChildNode("PRODUCT_TYPE",(WORD)m_loginConfirmProductType,PRODUCT_TYPE_ENUM);
	pXMLResponse->AddChildNode("HTTP_PORT",m_HttpPort);
//	pXMLResponse->AddChildNode("OPERATING_SYSTEM",m_operating_system,OPERATING_SYSTEM_ENUM);
	pXMLResponse->AddChildNode("ENTRY_QUEUE_ROUTING",m_loginInfoFlag & NUMERIC_ID_ROUTING,ENTRY_QUEUE_ROUTING_ENUM);

	if(CMcmsAuthentication::IsForceStrongPassword() && m_pwdExpirationWarningPeriod > 0)
		{
		    pXMLResponse->AddChildNode("PASSWORD_EXPIRATION_DAYS_LEFT", m_daysUntilPwdExpires);		//supported since 4.5
		}

	pXMLResponse->AddChildNode("SYSTEM_CARDS_MODE",m_rmxSystemCardsMode, SYSTEM_CARDS_MODE_ENUM);
	pXMLResponse->AddChildNode("SYSTEM_RAM_SIZE",m_ramSize, SYSTEM_RAM_SIZE_ENUM);
	pXMLResponse->AddChildNode("JITC_MODE",m_bJITC_Mode, _BOOL);
	pXMLResponse->AddChildNode("SESSION_TIMEOUT_IN_MINUTES",m_SessionTimeoutInMinutes);

	if(m_bJITC_Mode)
	    pXMLResponse->AddChildNode("PASSWORD_EXPIRATION_WARNING_DAYS", m_pwdExpirationWarningPeriod);

	if (m_bLastLoginAttempts)
	    m_loginHistory.SerializeXml(pXMLResponse);

	pXMLResponse->AddChildNode("HIDE_CONFERENCE_PASSWORD", m_bHidePsw, _BOOL);
	pXMLResponse->AddChildNode("SEPARATED_MANAGEMENT_NETWORK", m_bNetworkSeparation, _BOOL);
	pXMLResponse->AddChildNode("AUDIBLE_ALARM_ENABLE", m_bAudibleAlarmEnable, _BOOL);
	pXMLResponse->AddChildNode("HOTBACKUP_ACTUAL_TYPE", m_eMasterSlaveCurrentState, FAILOVER_MASTER_SLAVE_STATE);
	pXMLResponse->AddChildNode("TOTAL_NUMBER_OF_PARTICIPANTS",m_numCpParties);
	pXMLResponse->AddChildNode("TOTAL_NUMBER_OF_EVENT_MODE_PARTICIPANTS",m_numCopParties);
    pXMLResponse->AddChildNode("MULTIPLE_SERVICES", m_bMultipleServices, _BOOL);
    pXMLResponse->AddChildNode("V35_FOR_JITC", m_bV35JitcSupport, _BOOL);
	pXMLResponse->AddChildNode("MACHINE_ACCOUNT", m_bMachineAccount, _BOOL);
	pXMLResponse->AddChildNode("VIDEO_PREVIEW_ENABLE", m_bVideoPreviewEnable, _BOOL);

	// MCU capacity matrix depends on product type
	pXMLResponse->AddChildNode("MCU_AVC_CP_SUPPORT", m_avcSvcCap.supportAvcCp, _BOOL);
	pXMLResponse->AddChildNode("MCU_AVC_VSW_SUPPORT", m_avcSvcCap.supportAvcVsw, _BOOL);
	pXMLResponse->AddChildNode("MCU_SVC_SUPPORT", m_avcSvcCap.supportSvc, _BOOL);
	pXMLResponse->AddChildNode("MCU_MIXED_CP_SUPPORT", m_avcSvcCap.supportMixedCp, _BOOL);
	pXMLResponse->AddChildNode("MCU_MIXED_VSW_SUPPORT", m_avcSvcCap.supportMixedVsw, _BOOL);

	//pXMLResponse->AddChildNode("MCU_CASCADE_AVC", m_avcSvcCap.supportCascadeAvc, _BOOL);
	pXMLResponse->AddChildNode("MCU_CASCADE_SVC", m_avcSvcCap.supportCascadeSvc, _BOOL);
	//pXMLResponse->AddChildNode("MCU_SRTP_AVC", m_avcSvcCap.supportSrtpAvc, _BOOL);
	//pXMLResponse->AddChildNode("MCU_SRTP_SVC", m_avcSvcCap.supportSrtpSvc, _BOOL);
	pXMLResponse->AddChildNode("MCU_TIP", m_avcSvcCap.supportTip, _BOOL);
	pXMLResponse->AddChildNode("AVC_CIF_PLUS_ENABLED", m_avcSvcCap.supportAvcCifPlus, _BOOL);

	pXMLResponse->AddChildNode("MCU_ITP", m_avcSvcCap.supportItp, _BOOL);
	pXMLResponse->AddChildNode("MCU_AUDIO_ONLY_CONF", m_avcSvcCap.supportAudioOnlyConf, _BOOL);
	pXMLResponse->AddChildNode("MCU_HIGH_PROFILE_CONTENT", m_avcSvcCap.supportHighProfileContent, _BOOL);
	pXMLResponse->AddChildNode("MCU_PRODUCT_NAME", ProductTypeToRPCSString(m_loginConfirmProductType));
	pXMLResponse->AddChildNode("MAX_LINE_RATE", m_avcSvcCap.maxLineRate);

    /*Begin:added by Richer for BRIDGE-14278 , 2014.7.29*/
    pXMLResponse->AddChildNode("SIMULATION", m_bSimulationMode, _BOOL);
    /*End:added by Richer for BRIDGE-14278 , 2014.7.29*/

    if (m_avcSvcCap.licenseMode != (int)eLicenseMode_none )
    	pXMLResponse->AddChildNode("LICENSE_MODE",m_avcSvcCap.licenseMode, LICENSE_MODE_ENUM);
    else
    {

    	if ((eProductTypeEdgeAxis == m_loginConfirmProductType) && (CProcessBase::GetProcess()->IsFlexeraLicenseInSysFlag() == true))
    		pXMLResponse->AddChildNode("LICENSE_MODE",(int)eLicenseMode_flexera, LICENSE_MODE_ENUM);

    	else
    		pXMLResponse->AddChildNode("LICENSE_MODE",(int)eLicenseMode_cfs, LICENSE_MODE_ENUM);
    }

    TRACEINTOFUNC << "ser m_avcSvcCap.licenseMode " << m_avcSvcCap.licenseMode
    		<<"\nIsFlexeraLicenseInSysFlag " <<(int)CProcessBase::GetProcess()->IsFlexeraLicenseInSysFlag();



}

/////////////////////////////////////////////////////////////////////////////

int CLogInConfirm::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
  	CXMLDOMElement *pVersionListNode = NULL, *pVersionNode = NULL;

	GET_CHILD_NODE(pActionNode, "VERSION_LIST", pVersionListNode);
	if (pVersionListNode)
	{
		GET_CHILD_NODE(pVersionListNode, "MCU_VERSION", pVersionNode);
		if (pVersionNode)
		{
			GET_VALIDATE_CHILD(pVersionNode,"MAIN",&(m_MCU_Version.ver_major),_0_TO_DWORD);
			GET_VALIDATE_CHILD(pVersionNode,"MAJOR",&(m_MCU_Version.ver_minor),_0_TO_DWORD);
			GET_VALIDATE_CHILD(pVersionNode,"MINOR",&(m_MCU_Version.ver_release),_0_TO_DWORD);
			GET_VALIDATE_CHILD(pVersionNode,"INTERNAL",&(m_MCU_Version.ver_internal),_0_TO_DWORD);
			GET_VALIDATE_CHILD(pVersionNode,"PRIVATE_DESCRIPTION",m_mcuPrivateDesc,_0_TO_PRIVATE_VERSION_DESC_LENGTH);
			GET_VALIDATE_CHILD(pVersionNode,"DESCRIPTION",m_mcuDesc,DESCRIPTION_LENGTH);
		}
		GET_CHILD_NODE(pVersionListNode, "MCMS_VERSION", pVersionNode);
		if (pVersionNode)
		{
			GET_VALIDATE_CHILD(pVersionNode,"MAIN",&(m_MCMS_Version.ver_major),_0_TO_DWORD);
			GET_VALIDATE_CHILD(pVersionNode,"MAJOR",&(m_MCMS_Version.ver_minor),_0_TO_DWORD);
			GET_VALIDATE_CHILD(pVersionNode,"MINOR",&(m_MCMS_Version.ver_release),_0_TO_DWORD);
			GET_VALIDATE_CHILD(pVersionNode,"INTERNAL",&(m_MCMS_Version.ver_internal),_0_TO_DWORD);
			GET_VALIDATE_CHILD(pVersionNode,"PRIVATE_DESCRIPTION",m_mcmsPrivateDesc,_0_TO_PRIVATE_VERSION_DESC_LENGTH);
		}
	}

	GET_VALIDATE_CHILD(pActionNode,"MCU_TOKEN",&m_mcuToken,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"AUTHORIZATION_GROUP",&m_authorizationGroup,AUTHORIZATION_GROUP_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"API_NUMBER",&m_apiNumber,_0_TO_DWORD);

	WORD typeTmp = 0;
	GET_VALIDATE_CHILD(pActionNode,"PRODUCT_TYPE",&typeTmp,PRODUCT_TYPE_ENUM);
	m_loginConfirmProductType = (eProductType)typeTmp;


	GET_VALIDATE_CHILD(pActionNode,"HTTP_PORT",&m_HttpPort,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"OPERATING_SYSTEM",&m_operating_system,OPERATING_SYSTEM_ENUM);

	BYTE bTmp = 0;
	GET_VALIDATE_CHILD(pActionNode,"ENTRY_QUEUE_ROUTING",&bTmp,ENTRY_QUEUE_ROUTING_ENUM);


	if(CMcmsAuthentication::IsForceStrongPassword())
	    GET_VALIDATE_CHILD(pActionNode,"PASSWORD_EXPIRATION_DAYS_LEFT",&m_daysUntilPwdExpires,_0_TO_999_DECIMAL);

	if(nStatus == STATUS_OK)
	{
		if(bTmp)
			m_loginInfoFlag |= NUMERIC_ID_ROUTING;
		else
			m_loginInfoFlag &= (0xffffffff ^ NUMERIC_ID_ROUTING);
	}

    DWORD tmpCardsMode = 0;
    GET_VALIDATE_CHILD(pActionNode,"SYSTEM_CARDS_MODE",&tmpCardsMode,SYSTEM_CARDS_MODE_ENUM);
    m_rmxSystemCardsMode = (eSystemCardsMode)tmpCardsMode;

    DWORD tmpRamSize = 0;
    GET_VALIDATE_CHILD(pActionNode,"SYSTEM_RAM_SIZE",&tmpRamSize,SYSTEM_RAM_SIZE_ENUM);
    m_ramSize = (eSystemRamSize)tmpRamSize;

    GET_VALIDATE_CHILD(pActionNode,"JITC_MODE",&m_bJITC_Mode,_BOOL);

	if(m_bJITC_Mode)
	{ //olga
	    GET_VALIDATE_CHILD(pActionNode,"PASSWORD_EXPIRATION_WARNING_DAYS",&m_pwdExpirationWarningPeriod,_7_TO_90_DECIMAL);
	    GET_VALIDATE_CHILD(pActionNode,"SESSION_TIMEOUT_IN_MINUTES",&m_SessionTimeoutInMinutes,_5_TO_60_DECIMAL);
	}
	else
		GET_VALIDATE_CHILD(pActionNode,"SESSION_TIMEOUT_IN_MINUTES",&m_SessionTimeoutInMinutes,_0_TO_999_DECIMAL);

	if (m_bLastLoginAttempts)
	{
	    STATUS stat = m_loginHistory.DeSerializeXml(pActionNode, pszError, action);
	    PASSERTMSG(stat != STATUS_OK, "Unable to deserialize login history");
	}

	GET_VALIDATE_CHILD(pActionNode,"HIDE_CONFERENCE_PASSWORD",&m_bHidePsw,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"SEPARATED_MANAGEMENT_NETWORK",&m_bNetworkSeparation,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AUDIBLE_ALARM_ENABLE", &m_bAudibleAlarmEnable, _BOOL);
    GET_VALIDATE_CHILD(pActionNode,"HOTBACKUP_ACTUAL_TYPE", &m_eMasterSlaveCurrentState, FAILOVER_MASTER_SLAVE_STATE);

	GET_VALIDATE_CHILD(pActionNode,"TOTAL_NUMBER_OF_PARTICIPANTS",&m_numCpParties,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"TOTAL_NUMBER_OF_EVENT_MODE_PARTICIPANTS",&m_numCopParties,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"MULTIPLE_SERVICES",&m_bMultipleServices, _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"VIDEO_PREVIEW_ENABLE",&m_bVideoPreviewEnable, _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"V35_FOR_JITC",&m_bV35JitcSupport, _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MACHINE_ACCOUNT", &m_bMachineAccount, _BOOL);

	GET_VALIDATE_CHILD(pActionNode,"MCU_AVC_CP_SUPPORT", &(m_avcSvcCap.supportAvcCp), _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MCU_AVC_VSW_SUPPORT", &(m_avcSvcCap.supportAvcVsw), _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MCU_SVC_SUPPORT", &(m_avcSvcCap.supportSvc), _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MCU_MIXED_CP_SUPPORT", &(m_avcSvcCap.supportMixedCp), _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MCU_MIXED_VSW_SUPPORT", &(m_avcSvcCap.supportMixedVsw), _BOOL);

	//GET_VALIDATE_CHILD(pActionNode,"MCU_CASCADE_AVC", &(m_avcSvcCap.supportCascadeAvc), _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MCU_CASCADE_SVC", &(m_avcSvcCap.supportCascadeSvc), _BOOL);
	//GET_VALIDATE_CHILD(pActionNode,"MCU_SRTP_AVC", &(m_avcSvcCap.supportSrtpAvc), _BOOL);
	//GET_VALIDATE_CHILD(pActionNode,"MCU_SRTP_SVC", &(m_avcSvcCap.supportSrtpSvc), _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MCU_TIP", &(m_avcSvcCap.supportTip), _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"AVC_CIF_PLUS_ENABLED", &(m_avcSvcCap.supportAvcCifPlus), _BOOL);

	GET_VALIDATE_CHILD(pActionNode,"MCU_ITP", &(m_avcSvcCap.supportItp), _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MCU_AUDIO_ONLY_CONF", &(m_avcSvcCap.supportAudioOnlyConf), _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MCU_HIGH_PROFILE_CONTENT", &(m_avcSvcCap.supportHighProfileContent), _BOOL);
	//GET_VALIDATE_CHILD(pActionNode,"MCU_PRODUCT_NAME", string);
	GET_VALIDATE_CHILD(pActionNode,"MAX_LINE_RATE", &(m_avcSvcCap.maxLineRate), _0_TO_DWORD);


	GET_VALIDATE_CHILD(pActionNode,"LICENSE_MODE",&(m_avcSvcCap.licenseMode),LICENSE_MODE_ENUM);
	TRACEINTOFUNC << "des m_avcSvcCap.licenseMode " << m_avcSvcCap.licenseMode ;




	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CLogInConfirm::GetCompatibilityLevel () const
{
    return m_compatibilityLevel;
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetCompatibilityLevel(const WORD  level)
{
	m_compatibilityLevel=level;
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetIsAudibleAlarmEnable(WORD bAudibleAlarmEnable)
{
		m_bAudibleAlarmEnable= bAudibleAlarmEnable;

}

/////////////////////////////////////////////////////////////////////////////
const VERSION_S  CLogInConfirm::GetMCMS_Version () const
{
    return m_MCMS_Version;
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetMCMS_Version(const VERSION_S version)
{
	m_MCMS_Version=version;
}


/////////////////////////////////////////////////////////////////////////////
const VERSION_S  CLogInConfirm::GetMCU_Version () const
{
    return m_MCU_Version;
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetMCU_Version(const VERSION_S version)
{
	m_MCU_Version=version;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CLogInConfirm::GetApiNumber () const
{
    return m_apiNumber;
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetApiNumber(const DWORD  api_number)
{
	m_apiNumber=api_number;
}

/////////////////////////////////////////////////////////////////////////////
eProductType  CLogInConfirm::GetLoginConfirmProductType() const
{
	return m_loginConfirmProductType;
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetLoginConfirmProductType (const eProductType theType)
{
	m_loginConfirmProductType = theType;
}

//////////////////////////////////////////////////////////////////////////////
WORD  CLogInConfirm::GetProductType() const
{
	return m_productType;
}


/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetProductType(const WORD type)
{
	m_productType = type;

}


/////////////////////////////////////////////////////////////////////////////
DWORD CLogInConfirm::GetNumCpParties() const
{
	return m_numCpParties;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetNumCpParties(const DWORD numCpParties)
{
	m_numCpParties = numCpParties;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CLogInConfirm::GetNumCopParties() const
{
	return m_numCopParties;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetNumCopParties(const DWORD numCopParties)
{
	m_numCopParties = numCopParties;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetAvcSvcCap(AvcSvcCap const & cap)
{
	m_avcSvcCap = cap;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CLogInConfirm::GetJITC_Mode() const
{
	return m_bJITC_Mode;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CLogInConfirm::GetSessionTimeoutInMinutes() const
{
	return m_SessionTimeoutInMinutes;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CLogInConfirm::GetOperatingSystem() const
{
	return m_operating_system;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetOperatingSystem(const DWORD os)
{
	m_operating_system = os;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CLogInConfirm::GetAuthorization () const
{
    return m_authorizationGroup;
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetAuthorization(const WORD  group)
{
	m_authorizationGroup=group;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CLogInConfirm::GetHttpPort() const
{
	return m_HttpPort;
}

void CLogInConfirm::SetHttpPort(DWORD dwPort)
{
	m_HttpPort = dwPort;
}
/////////////////////////////////////////////////////////////////////////////
const std::string & CLogInConfirm::GetMcmsPrivateDesc() const
{
	return m_mcmsPrivateDesc;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetMcmsPrivateDesc(const std::string & privateDesc)
{
	m_mcmsPrivateDesc = privateDesc;
}

/////////////////////////////////////////////////////////////////////////////
const std::string & CLogInConfirm::GetMcuPrivateDesc() const
{
	return m_mcuPrivateDesc;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetMcuPrivateDesc(const std::string & privateDesc)
{
	m_mcuPrivateDesc = privateDesc;
}

/////////////////////////////////////////////////////////////////////////////
const std::string & CLogInConfirm::GetMcuDesc() const
{
	return m_mcuDesc;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetMcuDesc(const std::string & desc)
{
	m_mcuDesc = desc;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetLoginInfoFlag(const DWORD InfoFlag)
{
	m_loginInfoFlag = InfoFlag;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CLogInConfirm::GetLoginInfoFlag() const
{
	return m_loginInfoFlag;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetRmxSystemCardsMode(const eSystemCardsMode curMode)
{
	m_rmxSystemCardsMode = curMode;

	if ((m_rmxSystemCardsMode == eSystemCardsMode_mpmrx) && (eProductTypeNinja == m_loginConfirmProductType))
	{
		m_rmxSystemCardsMode = eSystemCardsMode_meridian;
	}

	// EMA should not show 'illegal'; instead, it should display the default
	if (eSystemCardsMode_illegal == m_rmxSystemCardsMode)
		m_rmxSystemCardsMode = GetRmxSystemCardsModeDefault();
}

/////////////////////////////////////////////////////////////////////////////
eSystemCardsMode CLogInConfirm::GetRmxSystemCardsMode() const
{
    return m_rmxSystemCardsMode;
}

/////////////////////////////////////////////////////////////////////////////
eSystemCardsMode CLogInConfirm::GetRmxSystemCardsModeDefault() const
{
	return eSystemCardsMode_mpm;
}

/////////////////////////////////////////////////////////////////////////////
eSystemRamSize CLogInConfirm::GetRmxSystemRamSize() const
{
	return m_ramSize;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CLogInConfirm::GetMcuToken() const
{
	return m_mcuToken;
}

/////////////////////////////////////////////////////////////////////////////
CStructTm  CLogInConfirm::GetLastLogin() const
{
    return m_loginHistory.GetLastLogin();
}
/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetLastLogin(const CStructTm last_login)
{
	m_loginHistory.SetLastLogin(last_login);
}
/////////////////////////////////////////////////////////////////////////////
std::string CLogInConfirm::GetLastLoginIPaddress() const
{
	return m_loginHistory.GetLastLoginIPaddress();
}
/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetLastLoginIPaddress(const std::string address)
{
  //memcpy(&m_lastLogin.ip_address, address, IPV6_ADDRESS_LEN);
    m_loginHistory.SetLastLoginIPaddress(address);
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetFailedLoginInfo(const CStructTm login, const std::string address)
{
    m_loginHistory.SetFailedLoginInfo(login, address);
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetLoginHistory(const CLogInHistory& loginHistory)
{
    m_loginHistory = loginHistory;
}


/////////////////////////////////////////////////////////////////////////////
void  CLogInConfirm::SetDaysUntilPwdExpires(DWORD days)
{
	m_daysUntilPwdExpires = days;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CLogInConfirm::GetDaysUntilPwdExpires()
{
	return m_daysUntilPwdExpires;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CLogInConfirm::GetPwdExpirationWarningPeriod()
{
	return m_pwdExpirationWarningPeriod;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CLogInConfirm::IsMachineAccount() const
{
	return m_bMachineAccount;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInConfirm::SetMachineAccount(const BYTE bMachineAccount)
{
	m_bMachineAccount = bMachineAccount;
}


