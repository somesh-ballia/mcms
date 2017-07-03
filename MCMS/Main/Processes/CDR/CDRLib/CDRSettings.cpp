#include "CDRSettings.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "StructTm.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"
#include "PlcmCdrClientConfig.h"

/////////////////////////////////////////////////////////////////////////////
// CCDRSettings

const std::string CCDRSettings::CDR_SETTINGS_FILE_NAME = ((std::string)(MCU_MCMS_DIR+"/Cfg/CDRSettings.xml"));

CCDRSettings::CCDRSettings()
{
	m_IsLocalCdrServer = true;
	m_IsRemoteCdrServer = false;
	m_ip = "0.0.0.0";
	m_port = "8443";
	m_user = "";
	m_pwd = "";
	m_sourceId = "";
	m_isSerializeToEMA = TRUE;
	m_bChanged = false;

}


/////////////////////////////////////////////////////////////////////////////
CCDRSettings::CCDRSettings(const CCDRSettings &other)
:CSerializeObject(other)
{
//	printf("inside CPrecedenceSettings::CPrecedenceSettings\n");

	m_IsLocalCdrServer = true;
	m_IsRemoteCdrServer = other.m_IsRemoteCdrServer;
	m_ip = other.m_ip;
	m_port = other.m_port;
	m_user = other.m_user;
	m_pwd = other.m_pwd;
	m_sourceId = other.m_sourceId;
	m_bChanged = other.m_bChanged;
	m_updateCounter = other.m_updateCounter;

}

/////////////////////////////////////////////////////////////////////////////
CCDRSettings& CCDRSettings::operator = (const CCDRSettings& other)
{
	m_IsLocalCdrServer = true;
	m_IsRemoteCdrServer = other.m_IsRemoteCdrServer;
	m_ip = other.m_ip;
	m_port = other.m_port;
	m_user = other.m_user;
	m_pwd = other.m_pwd;
	m_sourceId = other.m_sourceId;
	m_bChanged = other.m_bChanged;
	m_updateCounter = other.m_updateCounter;


	return *this;
}

bool CCDRSettings::operator !=(const CCDRSettings& other) const
{
	if (m_IsRemoteCdrServer != other.m_IsRemoteCdrServer ||
		m_ip != other.m_ip ||
		m_port != other.m_port ||
		m_user != other.m_user ||
		m_pwd != other.m_pwd ||
		m_bChanged != other.m_bChanged)
		return true;
	else
		return false;
}


/////////////////////////////////////////////////////////////////////////////
CCDRSettings::~CCDRSettings()
{

}




////////////////////////////////////////////////////////////////////////////
void CCDRSettings::SerializeXml(CXMLDOMElement*& pXMLRootElement) const
{
	TRACESTR(eLevelInfoNormal) << "CCDRSettings::SerializeXml, m_apiFormat=" << m_apiFormat;

	CXMLDOMElement* pPCDRSettingsNode = NULL;

	if (m_apiFormat==eRestApi)
	{
		PlcmCdrClientConfig plcmCdr;
		TRACESTR(eLevelInfoNormal) << "CCDRSettings::SerializeXml, m_updateCounter= " << m_updateCounter;
		plcmCdr.m_entityTag = m_updateCounter;
		TRACESTR(eLevelInfoNormal) << "CCDRSettings::SerializeXml, plcmCdr.m_entityTag= " << plcmCdr.m_entityTag;
		plcmCdr.m_enableCdr = m_IsRemoteCdrServer;
		plcmCdr.m_loginName = m_user;
		plcmCdr.m_loginPassword = m_pwd;
		plcmCdr.m_serverPort = m_port;
		plcmCdr.m_serverAddress = m_ip;


		m_apiBaseObjRest = ApiBaseObjectPtr(&plcmCdr);
	}
	else
	{
		//todo: maybe don't need if
		if(!pXMLRootElement)
		{
			pXMLRootElement =  new CXMLDOMElement();
			pXMLRootElement->set_nodeName("CDR_SETTINGS");
			pPCDRSettingsNode = pXMLRootElement;
		}
		else
		{
			pPCDRSettingsNode = pXMLRootElement->AddChildNode("CDR_SETTINGS");
		}

		//pPCDRSettingsNode->AddChildNode("IS_LOCAL_CDR_SERVICE", m_IsLocalCdrServer, _BOOL);
		TRACESTR(eLevelInfoNormal) << "CCDRSettings::SerializeXml, m_updateCounter=" << m_updateCounter;
		pPCDRSettingsNode->AddChildNode("OBJ_TOKEN", m_updateCounter);
		pPCDRSettingsNode->AddChildNode("CHANGED", m_bChanged, _BOOL);
		pPCDRSettingsNode->AddChildNode("IS_REMOTE_CDR_SERVICE", m_IsRemoteCdrServer, _BOOL);
		pPCDRSettingsNode->AddChildNode("CDR_SERVICE_IP", m_ip.c_str() );
		pPCDRSettingsNode->AddChildNode("CDR_SERVICE_PORT", m_port.c_str());
		pPCDRSettingsNode->AddChildNode("CDR_SERVICE_USER", m_user.c_str());
		pPCDRSettingsNode->AddChildNode("CDR_SERVICE_PASSWORD", m_pwd.c_str());
		if (m_isSerializeToEMA == FALSE)
			pPCDRSettingsNode->AddChildNode("SOURCE_ID", m_sourceId.c_str());
	}
}


int  CCDRSettings::DeSerializeXml(CXMLDOMElement* pXMLRootElement,char *pszError,const char* action)
{

	bool isFromFile = true;

	if (action && strcmp(action, "SET_CDR_SETTINGS") == 0)
	{
		isFromFile = false;
	}

	if (isFromFile)
	{
		return DeSerializeXmlFromFile(pXMLRootElement, pszError);
	}
	else
	{
		return DeSerializeXmlFromEma(pXMLRootElement, pszError);
	}

}

//called upon SET_CDR_SETTINGS
int CCDRSettings::DeSerializeXmlFromEma(CXMLDOMElement* pXMLRootElement,char *pszError)
{
//	printf("CPrecedenceSettings::DeSerializeXmlFromEma\n");
	//TRACESTR(eLevelInfoNormal) << "inside CCDRSettings::DeSerializeXmlFromEma";
	CXMLDOMElement* pCDRSettingsNode = NULL;
	int nStatus=STATUS_OK;
	char tmpData        [512];  memset(tmpData        , '\0', sizeof(tmpData        ));

	GET_FIRST_CHILD_NODE(pXMLRootElement, "CDR_SETTINGS", pCDRSettingsNode);
	if (pCDRSettingsNode)
	{
		//GET_VALIDATE_MANDATORY_CHILD(pCDRSettingsNode,"IS_LOCAL_CDR_SERVICE",&m_IsLocalCdrServer,_BOOL);

		GET_VALIDATE_MANDATORY_CHILD(pCDRSettingsNode,"IS_REMOTE_CDR_SERVICE",&m_IsRemoteCdrServer,_BOOL);
		GET_VALIDATE_MANDATORY_CHILD(pCDRSettingsNode, "CDR_SERVICE_IP", tmpData, ONE_LINE_BUFFER_LENGTH);
		m_ip = tmpData;
		GET_VALIDATE_MANDATORY_CHILD(pCDRSettingsNode, "CDR_SERVICE_PORT", tmpData, ONE_LINE_BUFFER_LENGTH);
		m_port = tmpData;
		GET_VALIDATE_MANDATORY_CHILD(pCDRSettingsNode, "CDR_SERVICE_USER", tmpData, ONE_LINE_BUFFER_LENGTH);
		m_user = tmpData;
		GET_VALIDATE_MANDATORY_CHILD(pCDRSettingsNode, "CDR_SERVICE_PASSWORD", tmpData, ONE_LINE_BUFFER_LENGTH);
		m_pwd = tmpData;
	}

	return STATUS_OK;
}


//called upon startup and GET_CDR_SETTINGS
int CCDRSettings::DeSerializeXmlFromFile(CXMLDOMElement* pXMLRootElement,char *pszError)
{
	//TRACESTR(eLevelInfoNormal) << "inside CCDRSettings::DeSerializeXmlFromFile;";

	int nStatus=STATUS_OK;
	char tmpData        [512];  memset(tmpData        , '\0', sizeof(tmpData        ));

	if (pXMLRootElement)
	{
		//GET_VALIDATE_CHILD(pXMLRootElement,"IS_LOCAL_CDR_SERVICE",&m_IsLocalCdrServer,_BOOL);
		GET_VALIDATE_CHILD(pXMLRootElement,"IS_REMOTE_CDR_SERVICE",&m_IsRemoteCdrServer,_BOOL);
		GET_VALIDATE_CHILD(pXMLRootElement, "CDR_SERVICE_IP", tmpData, ONE_LINE_BUFFER_LENGTH);
		m_ip = tmpData;
		GET_VALIDATE_CHILD(pXMLRootElement, "CDR_SERVICE_PORT", tmpData, ONE_LINE_BUFFER_LENGTH);
		m_port = tmpData;
		GET_VALIDATE_CHILD(pXMLRootElement, "CDR_SERVICE_USER", tmpData, ONE_LINE_BUFFER_LENGTH);
		m_user = tmpData;
		GET_VALIDATE_CHILD(pXMLRootElement, "CDR_SERVICE_PASSWORD", tmpData, ONE_LINE_BUFFER_LENGTH);
		m_pwd = tmpData;
		GET_VALIDATE_CHILD(pXMLRootElement, "SOURCE_ID", tmpData, ONE_LINE_BUFFER_LENGTH);
		m_sourceId = tmpData;
	}

		return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////
void CCDRSettings::Serialize(WORD format,CSegment& pSeg)
{
//	pSeg << (WORD)m_pUsePrecedence ;
//	for(int i=0; i<NUM_SINGLE_DOMAINS; i++)
//	{
//		pSeg << m_SingleDomains[i].m_DomainId;
//		pSeg << m_SingleDomains[i].m_DomainName;
//		pSeg << m_SingleDomains[i].m_SignalingDSCP;
//
//		for(int j=0; j<NUM_PRECEDENCE_LEVELS; j++)
//		{
//			pSeg << (WORD)(m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType);
//			pSeg << m_SingleDomains[i].m_PrecedenceLevels[j].r_priority;
//			pSeg << m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp;
//			pSeg << m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp;
//		}
//
//	}

}

/////////////////////////////////////////////////////////////////////////////
void CCDRSettings::DeSerialize(WORD format,CSegment& rSeg)
{
//	WORD tmpWord;
//	BYTE tmpByte;
//	std::string tmpString;
//	rSeg >> tmpWord;
//	m_pUsePrecedence = (int)tmpWord;
//
//	for(int i=0; i<NUM_SINGLE_DOMAINS; i++)
//	{
//		rSeg >> tmpByte;
//		m_SingleDomains[i].m_DomainId = tmpByte;
//
//		rSeg >> tmpString;
//		m_SingleDomains[i].m_DomainName = tmpString;
//
//		rSeg >> tmpByte;
//		m_SingleDomains[i].m_SignalingDSCP = tmpByte;
//
//		for(int j=0; j<NUM_PRECEDENCE_LEVELS; j++)
//		{
//			rSeg >> tmpWord;
//			m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType = (ePrecedenceLevelType)tmpWord;
//
//			rSeg >> tmpByte;
//			m_SingleDomains[i].m_PrecedenceLevels[j].r_priority = tmpByte;
//
//			rSeg >> tmpByte;
//			m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp = tmpByte;
//
//			rSeg >> tmpByte;
//			m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp = tmpByte;
//		}
//
//	}

}






void CCDRSettings::PrintToConsole()
{

//	printf("m_pUsePrecedence: %d\n",m_pUsePrecedence);
//	for (int i=0; i<NUM_SINGLE_DOMAINS; i++)
//	{
//		printf("m_DomainId: %d\n",m_SingleDomains[i].m_DomainId);
//		printf("m_DomainName: %s\n",m_SingleDomains[i].m_DomainName.c_str());
//		printf("m_SignalingDSCP: %d\n",m_SingleDomains[i].m_SignalingDSCP);
//		for (int j=0; j<NUM_PRECEDENCE_LEVELS; j++)
//		{
//			printf("PRECEDENCE_LEVEL_TYPE: %s\n",GetPrecedenceSettingsName(m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType));
//			printf("R_PRIORITY: %d\n",m_SingleDomains[i].m_PrecedenceLevels[j].r_priority);
//			printf("AUDIO_DSCP: %d\n",m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp);
//			printf("R_PRIORITY: %d\n",m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp);
//		}
//		printf("\n");
//	}

}



