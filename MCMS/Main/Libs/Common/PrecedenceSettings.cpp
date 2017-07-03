#include "PrecedenceSettings.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "StructTm.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"



/////////////////////////////////////////////////////////////////////////////
// CPrecedenceSettings

const char* CPrecedenceSettings::PRECEDENCE_SETTINGS_FILE_NAME = "Cfg/PrecedenceSettings.xml";

CPrecedenceSettings::CPrecedenceSettings()
{
	m_pUsePrecedence = 0;
	m_SingleDomains[0].m_DomainName = "uc-000000";
	m_SingleDomains[1].m_DomainName = "dsn-000000";
	for(int i=0; i<NUM_SINGLE_DOMAINS; i++)
	{
		m_SingleDomains[i].m_DomainId = i+1;
		m_SingleDomains[i].m_SignalingDSCP = 40;

		m_SingleDomains[i].m_PrecedenceLevels[0].precedenceLevelType = eRoutine;
		m_SingleDomains[i].m_PrecedenceLevels[0].r_priority = 0;
		m_SingleDomains[i].m_PrecedenceLevels[0].audio_dscp = 51;
		m_SingleDomains[i].m_PrecedenceLevels[0].video_dscp = 51;

		m_SingleDomains[i].m_PrecedenceLevels[1].precedenceLevelType = ePriority;
		m_SingleDomains[i].m_PrecedenceLevels[1].r_priority = 2;
		m_SingleDomains[i].m_PrecedenceLevels[1].audio_dscp = 39;
		m_SingleDomains[i].m_PrecedenceLevels[1].video_dscp = 39;

		m_SingleDomains[i].m_PrecedenceLevels[2].precedenceLevelType = eImmediate;
		m_SingleDomains[i].m_PrecedenceLevels[2].r_priority = 4;
		m_SingleDomains[i].m_PrecedenceLevels[2].audio_dscp = 37;
		m_SingleDomains[i].m_PrecedenceLevels[2].video_dscp = 37;

		m_SingleDomains[i].m_PrecedenceLevels[3].precedenceLevelType = eFlash;
		m_SingleDomains[i].m_PrecedenceLevels[3].r_priority = 6;
		m_SingleDomains[i].m_PrecedenceLevels[3].audio_dscp = 35;
		m_SingleDomains[i].m_PrecedenceLevels[3].video_dscp = 35;

		m_SingleDomains[i].m_PrecedenceLevels[4].precedenceLevelType = eFlashOverride;
		m_SingleDomains[i].m_PrecedenceLevels[4].r_priority = 8;
		m_SingleDomains[i].m_PrecedenceLevels[4].audio_dscp = 33;
		m_SingleDomains[i].m_PrecedenceLevels[4].video_dscp = 33;

		m_SingleDomains[i].m_PrecedenceLevels[5].precedenceLevelType = eFlashOverridePlus;
		m_SingleDomains[i].m_PrecedenceLevels[5].r_priority = 9;
		m_SingleDomains[i].m_PrecedenceLevels[5].audio_dscp = 33;
		m_SingleDomains[i].m_PrecedenceLevels[5].video_dscp = 33;

	}


}


/////////////////////////////////////////////////////////////////////////////
CPrecedenceSettings::CPrecedenceSettings(const CPrecedenceSettings &other)
:CSerializeObject(other)
{
//	printf("inside CPrecedenceSettings::CPrecedenceSettings\n");

	m_pUsePrecedence = other.m_pUsePrecedence;
	for(int i=0; i<NUM_SINGLE_DOMAINS; i++)
	{
//		m_SingleDomains[i].m_DomainName = new char[100];
//		memset(m_SingleDomains[i].m_DomainName, '\0', 100);
//		strcpy(m_SingleDomains[i].m_DomainName, other.m_SingleDomains[i].m_DomainName);
		m_SingleDomains[i].m_DomainName = other.m_SingleDomains[i].m_DomainName;

		m_SingleDomains[i].m_DomainId = other.m_SingleDomains[i].m_DomainId;
		m_SingleDomains[i].m_SignalingDSCP = other.m_SingleDomains[i].m_SignalingDSCP;
		for(int j=0; j<NUM_PRECEDENCE_LEVELS; j++)
		{
			m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType = other.m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType;
			m_SingleDomains[i].m_PrecedenceLevels[j].r_priority = other.m_SingleDomains[i].m_PrecedenceLevels[j].r_priority;
			m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp = other.m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp;
			m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp = other.m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp;
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
CPrecedenceSettings& CPrecedenceSettings::operator = (const CPrecedenceSettings &other)
{
//	printf("inside CPrecedenceSettings& CPrecedenceSettings::operator =\n");
	TRACESTR(eLevelInfoNormal) << "inside CPrecedenceSettings& CPrecedenceSettings::operator =j" ;

	m_pUsePrecedence = other.m_pUsePrecedence;
	for(int i=0; i<NUM_SINGLE_DOMAINS; i++)
	{
//		m_SingleDomains[i].m_DomainName = new char[100];
//		memset(m_SingleDomains[i].m_DomainName, '\0', 100);
//		strcpy(m_SingleDomains[i].m_DomainName, other.m_SingleDomains[i].m_DomainName);

		m_SingleDomains[i].m_DomainName = other.m_SingleDomains[i].m_DomainName;

		m_SingleDomains[i].m_DomainId = other.m_SingleDomains[i].m_DomainId;
		m_SingleDomains[i].m_SignalingDSCP = other.m_SingleDomains[i].m_SignalingDSCP;
		for(int j=0; j<NUM_PRECEDENCE_LEVELS; j++)
		{
			m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType = other.m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType;
			m_SingleDomains[i].m_PrecedenceLevels[j].r_priority = other.m_SingleDomains[i].m_PrecedenceLevels[j].r_priority;
			m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp = other.m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp;
			m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp = other.m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp;
		}
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CPrecedenceSettings::~CPrecedenceSettings()
{

}




////////////////////////////////////////////////////////////////////////////
void CPrecedenceSettings::SerializeXml(CXMLDOMElement*& pXMLRootElement) const
{
//	printf("CPrecedenceSettings::SerializeXml\n");
	CXMLDOMElement* pPrecedenceSettingsNode = NULL;
	CXMLDOMElement* pSingleDomain = NULL;
	CXMLDOMElement* pLevel = NULL;

	//todo: maybe don't need if
	if(!pXMLRootElement)
	{
		pXMLRootElement =  new CXMLDOMElement();
		pXMLRootElement->set_nodeName("PRECEDENCE_SETTINGS");
		pPrecedenceSettingsNode = pXMLRootElement;
	}
	else
	{
		pPrecedenceSettingsNode = pXMLRootElement->AddChildNode("PRECEDENCE_SETTINGS");
	}

	pPrecedenceSettingsNode->AddChildNode("USE_PRECEDENCE", m_pUsePrecedence, _BOOL);

	for(int i=0; i<NUM_SINGLE_DOMAINS; i++)
	{
		pSingleDomain = pPrecedenceSettingsNode->AddChildNode("PRECEDENCE_SETTINGS_SINGLE_DOMAIN");
//		printf("m_SingleDomains[i].m_DomainId: %d\n", m_SingleDomains[i].m_DomainId);
		pSingleDomain->AddChildNode("PRECEDENCE_DOMAIN_ID", m_SingleDomains[i].m_DomainId);
		//printf("m_SingleDomains[i].m_DomainName: %s\n", m_SingleDomains[i].m_DomainName);
		pSingleDomain->AddChildNode("PRECEDENCE_DOMAIN_NAME", m_SingleDomains[i].m_DomainName.c_str());
		pSingleDomain->AddChildNode("SIGNALING_DSCP", m_SingleDomains[i].m_SignalingDSCP);

		for(int j=0; j<NUM_PRECEDENCE_LEVELS; j++)
		{
			pLevel = pSingleDomain->AddChildNode("PRECEDENCE_LEVEL");
			pLevel->AddChildNode("PRECEDENCE_LEVEL_TYPE", m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType, PRECEDENCE_LEVEL_TYPE_ENUM);
			pLevel->AddChildNode("R_PRIORITY", m_SingleDomains[i].m_PrecedenceLevels[j].r_priority);
			pLevel->AddChildNode("AUDIO_DSCP", m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp);
			pLevel->AddChildNode("VIDEO_DSCP", m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp);
		}

	}



}


int  CPrecedenceSettings::DeSerializeXml(CXMLDOMElement* pXMLRootElement,char *pszError,const char* action)
{
	bool isFromFile = true;

	if (action && strcmp(action, "SET_PRECEDENCE_SETTINGS") == 0)
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

//function is called upon SET_PRECEDENCE_SETTINGS
int CPrecedenceSettings::DeSerializeXmlFromEma(CXMLDOMElement* pXMLRootElement,char *pszError)
{
//	printf("CPrecedenceSettings::DeSerializeXmlFromEma\n");
	CXMLDOMElement* pPrecedenceSettingsNode = NULL;
	CXMLDOMElement* pSingleDomain = NULL;
	CXMLDOMElement* pLevel = NULL;
	int nStatus=STATUS_OK;
	char tmpData        [512];  memset(tmpData        , '\0', sizeof(tmpData        ));

	GET_FIRST_CHILD_NODE(pXMLRootElement, "PRECEDENCE_SETTINGS", pPrecedenceSettingsNode);
	if (pPrecedenceSettingsNode)
	{
		GET_VALIDATE_MANDATORY_CHILD(pPrecedenceSettingsNode,"USE_PRECEDENCE",&m_pUsePrecedence,_BOOL);

		//Single Domain
		GET_FIRST_CHILD_NODE(pPrecedenceSettingsNode, "PRECEDENCE_SETTINGS_SINGLE_DOMAIN", pSingleDomain);

		for (int i=0; pSingleDomain && (i < NUM_SINGLE_DOMAINS); i++)
		{
			GET_VALIDATE_MANDATORY_CHILD(pSingleDomain, "PRECEDENCE_DOMAIN_ID", &(m_SingleDomains[i].m_DomainId), PRECEDENCE_DOMAIN_ID_ENUM);
			GET_VALIDATE_MANDATORY_CHILD(pSingleDomain, "PRECEDENCE_DOMAIN_NAME", tmpData, ONE_LINE_BUFFER_LENGTH);
			m_SingleDomains[i].m_DomainName = tmpData;
			GET_VALIDATE_MANDATORY_CHILD(pSingleDomain, "SIGNALING_DSCP", &(m_SingleDomains[i].m_SignalingDSCP), _0_TO_63_DECIMAL);

			//Precedence Levels
			GET_FIRST_CHILD_NODE(pSingleDomain, "PRECEDENCE_LEVEL", pLevel);
			for (int j=0; pLevel && (j < NUM_PRECEDENCE_LEVELS); j++)
			{
				WORD tmp;
				GET_VALIDATE_MANDATORY_CHILD(pLevel,"PRECEDENCE_LEVEL_TYPE",&tmp,PRECEDENCE_LEVEL_TYPE_ENUM);
				m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType = (ePrecedenceLevelType)tmp;
				GET_VALIDATE_MANDATORY_CHILD(pLevel, "R_PRIORITY", &(m_SingleDomains[i].m_PrecedenceLevels[j].r_priority), _0_TO_255_DECIMAL);
				GET_VALIDATE_MANDATORY_CHILD(pLevel, "AUDIO_DSCP", &(m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp), _0_TO_63_DECIMAL);
				GET_VALIDATE_MANDATORY_CHILD(pLevel, "VIDEO_DSCP", &(m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp), _0_TO_63_DECIMAL);

				GET_NEXT_CHILD_NODE(pSingleDomain, "PRECEDENCE_LEVEL", pLevel);
			}

			GET_NEXT_CHILD_NODE(pPrecedenceSettingsNode, "PRECEDENCE_SETTINGS_SINGLE_DOMAIN", pSingleDomain);
		}

	}

	return STATUS_OK;
}


//function is called upon GET_PRECEDENCE_SETTINGS
int CPrecedenceSettings::DeSerializeXmlFromFile(CXMLDOMElement* pXMLRootElement,char *pszError)
{
//	printf("CPrecedenceSettings::DeSerializeXmlFromFile\n");
	CXMLDOMElement* pSingleDomain = NULL;
	CXMLDOMElement* pLevel = NULL;
	int nStatus=STATUS_OK;
	char tmpData        [512];  memset(tmpData        , '\0', sizeof(tmpData        ));


	if (pXMLRootElement)
	{
//		printf("got pXMLRootElement\n");
		GET_VALIDATE_CHILD(pXMLRootElement,"USE_PRECEDENCE",&m_pUsePrecedence,_BOOL);
//		printf("m_pUsePrecedence: %d\n", m_pUsePrecedence);
		//Single Domain
		GET_FIRST_CHILD_NODE(pXMLRootElement, "PRECEDENCE_SETTINGS_SINGLE_DOMAIN", pSingleDomain);

		for (int i=0; pSingleDomain && (i < NUM_SINGLE_DOMAINS); i++)
		{
			GET_VALIDATE_CHILD(pSingleDomain, "PRECEDENCE_DOMAIN_ID", &(m_SingleDomains[i].m_DomainId), PRECEDENCE_DOMAIN_ID_ENUM);
//			printf("got m_DomainId: %d\n", m_SingleDomains[i].m_DomainId);
			GET_VALIDATE_CHILD(pSingleDomain, "PRECEDENCE_DOMAIN_NAME", tmpData, ONE_LINE_BUFFER_LENGTH);
			m_SingleDomains[i].m_DomainName = tmpData;
//			printf("m_DomainName: %s\n", m_SingleDomains[i].m_DomainName.c_str());
			GET_VALIDATE_CHILD(pSingleDomain, "SIGNALING_DSCP", &(m_SingleDomains[i].m_SignalingDSCP), _0_TO_63_DECIMAL);
//			printf("m_SignalingDSCP: %d\n", m_SingleDomains[i].m_SignalingDSCP);

			//Precedence Levels
			GET_FIRST_CHILD_NODE(pSingleDomain, "PRECEDENCE_LEVEL", pLevel);
			for (int j=0; pLevel && (j < NUM_PRECEDENCE_LEVELS); j++)
			{
				WORD tmp;
				GET_VALIDATE_CHILD(pLevel,"PRECEDENCE_LEVEL_TYPE",&tmp,PRECEDENCE_LEVEL_TYPE_ENUM);
				m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType = (ePrecedenceLevelType)tmp;
//				printf("CPrecedenceSettings::DeSerializeXmlFromFile, m_DomainName: %s\n",m_SingleDomains[i].m_DomainName.c_str());

				//GET_VALIDATE_CHILD(pLevel, "PRECEDENCE_LEVEL_TYPE", &(m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType), PRECEDENCE_LEVEL_TYPE_ENUM);
//				printf("precedenceLevelType: %d, nStatus: %d\n", m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType, nStatus);
				GET_VALIDATE_CHILD(pLevel, "R_PRIORITY", &(m_SingleDomains[i].m_PrecedenceLevels[j].r_priority), _0_TO_255_DECIMAL);
//				printf("r_priority: %d\n", m_SingleDomains[i].m_PrecedenceLevels[j].r_priority);
				GET_VALIDATE_CHILD(pLevel, "AUDIO_DSCP", &(m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp), _0_TO_63_DECIMAL);
//				printf("audio_dscp: %d\n", m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp);
				GET_VALIDATE_CHILD(pLevel, "VIDEO_DSCP", &(m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp), _0_TO_63_DECIMAL);
//				printf("video_dscp: %d\n", m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp);

				GET_NEXT_CHILD_NODE(pSingleDomain, "PRECEDENCE_LEVEL", pLevel);
			}

			GET_NEXT_CHILD_NODE(pXMLRootElement, "PRECEDENCE_SETTINGS_SINGLE_DOMAIN", pSingleDomain);
		}

	}

	return STATUS_OK;
}



/////////////////////////////////////////////////////////////////////////////
void CPrecedenceSettings::Serialize(WORD format,CSegment& pSeg)
{
	pSeg << (WORD)m_pUsePrecedence ;
	for(int i=0; i<NUM_SINGLE_DOMAINS; i++)
	{
		pSeg << m_SingleDomains[i].m_DomainId;
		pSeg << m_SingleDomains[i].m_DomainName;
		pSeg << m_SingleDomains[i].m_SignalingDSCP;

		for(int j=0; j<NUM_PRECEDENCE_LEVELS; j++)
		{
			pSeg << (WORD)(m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType);
			pSeg << m_SingleDomains[i].m_PrecedenceLevels[j].r_priority;
			pSeg << m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp;
			pSeg << m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp;
		}

	}

}

/////////////////////////////////////////////////////////////////////////////
void CPrecedenceSettings::DeSerialize(WORD format,CSegment& rSeg)
{
	WORD tmpWord;
	BYTE tmpByte;
	std::string tmpString;
	rSeg >> tmpWord;
	m_pUsePrecedence = (int)tmpWord;

	for(int i=0; i<NUM_SINGLE_DOMAINS; i++)
	{
		rSeg >> tmpByte;
		m_SingleDomains[i].m_DomainId = tmpByte;

		rSeg >> tmpString;
		m_SingleDomains[i].m_DomainName = tmpString;

		rSeg >> tmpByte;
		m_SingleDomains[i].m_SignalingDSCP = tmpByte;

		for(int j=0; j<NUM_PRECEDENCE_LEVELS; j++)
		{
			rSeg >> tmpWord;
			m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType = (ePrecedenceLevelType)tmpWord;

			rSeg >> tmpByte;
			m_SingleDomains[i].m_PrecedenceLevels[j].r_priority = tmpByte;

			rSeg >> tmpByte;
			m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp = tmpByte;

			rSeg >> tmpByte;
			m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp = tmpByte;
		}

	}

}




//void CPrecedenceSettings::SaveToFile()
//{
//	TRACESTR(eLevelInfoNormal) << "CPrecedenceSettings::SaveToFile - " << PRECEDENCE_SETTINGS_FILE;
//	WriteXmlFile(PRECEDENCE_SETTINGS_FILE);
//}
//
//
////////////////////////////////////////////////////////////////////////
//bool CPrecedenceSettings::LoadFromFile()
//{
//	STATUS status = STATUS_OK;
//
////	int size = GetFileSize(PATH);
////	if(0 < size)
////	{
//		eFailReadingFileActiveAlarmType aaPolicy 	= eNoActiveAlarm;
//		eFailReadingFileOperationType operationType = eNoAction;
//
//		status = ReadXmlFile(PRECEDENCE_SETTINGS_FILE, aaPolicy, operationType);
//		printf("CPrecedenceSettings::LoadFromFile -status: %d, filename: %s\n", status, PRECEDENCE_SETTINGS_FILE);
//		TRACESTR(eLevelInfoNormal) << "CPrecedenceSettings::LoadFromFile -status " << status << " filename " << PRECEDENCE_SETTINGS_FILE << "\n";
////	}
//
//	return STATUS_OK == status;
//}

void CPrecedenceSettings::PrintToConsole()
{

	printf("m_pUsePrecedence: %d\n",m_pUsePrecedence);
	for (int i=0; i<NUM_SINGLE_DOMAINS; i++)
	{
		printf("m_DomainId: %d\n",m_SingleDomains[i].m_DomainId);
		printf("m_DomainName: %s\n",m_SingleDomains[i].m_DomainName.c_str());
		printf("m_SignalingDSCP: %d\n",m_SingleDomains[i].m_SignalingDSCP);
		for (int j=0; j<NUM_PRECEDENCE_LEVELS; j++)
		{
			printf("PRECEDENCE_LEVEL_TYPE: %s\n",GetPrecedenceSettingsName(m_SingleDomains[i].m_PrecedenceLevels[j].precedenceLevelType));
			printf("R_PRIORITY: %d\n",m_SingleDomains[i].m_PrecedenceLevels[j].r_priority);
			printf("AUDIO_DSCP: %d\n",m_SingleDomains[i].m_PrecedenceLevels[j].audio_dscp);
			printf("R_PRIORITY: %d\n",m_SingleDomains[i].m_PrecedenceLevels[j].video_dscp);
		}
		printf("\n");
	}

}



const char* CPrecedenceSettings::GetPrecedenceSettingsName(ePrecedenceLevelType precedenceLevelType)
{
   switch (precedenceLevelType)
   {
      case eRoutine: return "ROUTINE";
      case ePriority: return "PRIORITY";
      case eImmediate: return "IMMEDIATE";
      case eFlash: return "FLASH";
      case eFlashOverride: return "FLASH_OVERRIDE";
      case eFlashOverridePlus: return "FLASH_OVERRIDE_PLUS";
	  default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
   }

   return "";
}

//////////////////////////////////////////////////////////////////////


// GetDomainId - Retrieving domain id for a domain name
// Returns NUM_SINGLE_DOMAINS if domain not found
//====================================================================
BYTE CPrecedenceSettings::GetDomainId(const char* szDomainName) const
{
	CMedString log;
	BYTE precedNdx = NUM_SINGLE_DOMAINS;
	log << "CPrecedenceSettings::GetDomainId - for Domain[" << szDomainName << "]";
	PTRACE(eLevelInfoNormal, log.GetString());
	if (szDomainName)
	{
		for(precedNdx = 0; precedNdx < NUM_SINGLE_DOMAINS && strcmp(szDomainName, m_SingleDomains[precedNdx].m_DomainName.c_str()); ++precedNdx)
		{}
	}

	if (precedNdx < NUM_SINGLE_DOMAINS)
	{
		PTRACE2INT(eLevelInfoNormal, "CPrecedenceSettings::GetDomainId - Matched domain ID: ", precedNdx);
	}
	else
	{
		PTRACE(eLevelError, "CPrecedenceSettings::GetDomainId - Domain not found");
	}

	return precedNdx;
}

// GetRPrioForPrecedenceLevel - Retrieving R-priority for precedenceLevel
// Returns DEFAULT_PRECEDENCE_R_PRIORITY for input errors
//====================================================================
BYTE CPrecedenceSettings::GetRPrioForPrecedenceLevel(BYTE domainId, BYTE precedenceLevel) const
{
	BYTE RPrio = DEFAULT_PRECEDENCE_R_PRIORITY;

	if (domainId < NUM_SINGLE_DOMAINS && precedenceLevel < NUM_PRECEDENCE_LEVELS)
	{
		RPrio = m_SingleDomains[domainId].m_PrecedenceLevels[precedenceLevel].r_priority;
	}

	return RPrio;
}

// GetPrecedenceLevelForRPrio - Retrieving precedenceLevel for R-priority
// Returns NUM_PRECEDENCE_LEVELS for input errors
//====================================================================
BYTE CPrecedenceSettings::GetPrecedenceLevelForRPrio(BYTE domainId, BYTE RPrio) const
{
	BYTE precedLevel;

	if (domainId < NUM_SINGLE_DOMAINS && RPrio != DEFAULT_PRECEDENCE_R_PRIORITY)
	{
		for(precedLevel = 0; precedLevel < NUM_PRECEDENCE_LEVELS &&
			m_SingleDomains[domainId].m_PrecedenceLevels[precedLevel].r_priority != RPrio;
			++precedLevel)
		{}
	}
	else
	{
		precedLevel = NUM_PRECEDENCE_LEVELS;
	}

	return precedLevel;

}
