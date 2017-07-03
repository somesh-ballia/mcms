#include <stdlib.h>
#include "ServiceConfig.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "TraceStream.h"
#include "ObjString.h"
#include "Segment.h"


//////////////////////////////////////////////////////////////////////
CServiceConfig::CServiceConfig()
{
	m_Id = 0;
	//ServiceFilePath= new char[50];
	ServiceFilePath="Cfg/ServiceCfg";

}

//////////////////////////////////////////////////////////////////////
CServiceConfig::~CServiceConfig()
{

}

/////////////////////////////////////////////////////////////////////


CServiceConfig::CServiceConfig(const CServiceConfig &other)
:CSysConfigBase(other)
{
	m_Id= other.m_Id;
//	strncpy(m_serviceName, other.m_serviceName, NET_SERVICE_PROVIDER_NAME_LEN);
}

CServiceConfig&  CServiceConfig::operator=( const CServiceConfig& other )
{
	CSysConfigBase::operator=(other);

	m_Id = other.m_Id;
//	strncpy(m_serviceName, other.m_serviceName, NET_SERVICE_PROVIDER_NAME_LEN);

	return *this;
}

///////////////////////////////////////////////////
const char* CServiceConfig::GetTmpFileName(eCfgParamType type)
{

	/*char * ServiceConfigPath = (char *)GetFileName(eCfgParamUser);
	char *pTmpPath = new char[256];
	strcpy(pTmpPath, ServiceConfigPath);
	char *ServiceConfigPathTmp = strtok(pTmpPath,".");
	strcat( ServiceConfigPathTmp,"Tmp");
	strcat(ServiceConfigPathTmp,".xml");

	return ServiceConfigPathTmp;*/

	std::ostringstream fname;
	fname << ServiceFilePath.c_str()
		      << "_"
		      << m_Id
		      << "Tmp.xml";

	m_sFileNameTmp = fname.str();

	TRACESTR(eLevelInfoNormal) << "CServiceConfig::GetTmpFileName - " << m_sFileNameTmp;

	return m_sFileNameTmp.c_str();

}




const char* CServiceConfig::GetFileName(eCfgParamType type)
{
	TRACESTR(eLevelInfoNormal) << "CServiceConfig::GetFileName";
	std::ostringstream fname;
	std::string extension;

	fname << ServiceFilePath.c_str()
	      << "_"
	      << m_Id
	      << ".xml";

	m_sFileName = fname.str();

//	sstr << m_Id;
//	std::string service_id = sstr.str();
	//sstr<< m_serviceName;
	//std::string service_name=sstr.str();
//	m_sFileName=ServiceFilePath + "_" +service_id + ".xml";


	TRACESTR(eLevelInfoNormal) << "CServiceConfig::GetFileName - " << m_sFileName;

	return m_sFileName.c_str();
}

//////////////////////////////////////////////////////////////////////
DWORD CServiceConfig::GetNumOfParams()const
{
	DWORD result = GetMap()->size();
	return result;
}





//////////////////////////////////////////////////////////////////////
BOOL CServiceConfig::GetCSData(const std::string &key, std::string &data)const
{
	CCfgData *cfgData = NULL;
	BOOL res = ReadUpdate(key, cfgData);
	if(TRUE == res)
	{
		data = cfgData->GetData();
	}
	else
	{
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		if (!pSysConfig->GetDataByKey(key,data))
		{
			TRACESTR(eLevelInfoNormal) << "CServiceConfig::GetCSData: Failed to read key : " << key;
			return FALSE;
		}
		else
			return TRUE;


	}

	return true;
}


//////////////////////////////////////////////////////////////////
BOOL CServiceConfig::GetStrDataByKey(const std::string &key, std::string &data)const
{
	CCfgData *cfgData = NULL;
	BOOL res = ReadUpdate(key, cfgData);
	if(TRUE == res)
	{
		data = cfgData->GetData();
	}

	return res;

}


//////////////////////////////////////////////////////////////////////
BOOL CServiceConfig::GetDWORDDataByKey(const std::string &key, DWORD &data)const
{
	CCfgData *cfgData;
	BOOL res = ReadUpdate(key, cfgData);
	if(FALSE == res)
	{
		return res;
	}

	data = atoi(cfgData->GetData().c_str());

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
BOOL CServiceConfig::GetHexDataByKey(const std::string &key, DWORD &data)const
{
	CCfgData *cfgData;
	BOOL res = ReadUpdate(key, cfgData);
	if(FALSE == res)
	{
		return res;
	}
	sscanf(cfgData->GetData().c_str(),"%x", &data);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
BOOL CServiceConfig::GetBOOLDataByKey(const std::string &key, BOOL &data)const
{
	CCfgData *cfgData;
	BOOL res = ReadUpdate(key, cfgData);
	if(FALSE == res)
	{
		return res;
	}

    if (CFG_STR_YES == cfgData->GetData())
	{
		data = TRUE;
		return TRUE;
	}

	if(CFG_STR_NO == cfgData->GetData())
	{
		data = FALSE;
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
BOOL CServiceConfig::ReadUpdate(const std::string &key, CCfgData *& cfgData)const
{
	//TRACESTR(eLevelInfoNormal) << "CServiceConfig::ReadUpdate : " << key;
	if (IsParamExist(key))
	{
		cfgData = GetCfgEntryByKey(key);
		//TRACESTR(eLevelInfoNormal) << "CServiceConfig::ReadUpdate: IsParamExist(key) : " << key;
		return TRUE;

	}
//	if(NULL == cfgData)
//	{
//		TRACESTR(eLevelInfoNormal) << "CServiceConfig::ReadUpdate: Failed to read key : " << key;
//		return FALSE;
//	}
	//cfgData->IncrementCnt();

	return FALSE;
}



/////////////////////////////////////////////////////////////////////////////
/*void  CServiceConfig::SetServiceName( const char* name )
{
	TRACESTR(eLevelInfoNormal) << "CServiceConfig::SetServiceName - "<<name;
    int len=strlen( name );
    strncpy( m_serviceName, name, NET_SERVICE_PROVIDER_NAME_LEN );
    if(len>NET_SERVICE_PROVIDER_NAME_LEN )
        m_serviceName[NET_SERVICE_PROVIDER_NAME_LEN-1] = '\0';
}*/

//////////////////////////////////////////////////////////////////////
CSysConfigBase* CServiceConfig::GetSysConfig()
{
	TRACESTR(eLevelInfoNormal) << "CServiceConfig::GetSysConfig - ";
	return this;
}

//////////////////////////////////////////////////////////////////////
bool CServiceConfig::TakeCfgParam(const char *key, const char *data, const char *section, const eCfgParamResponsibilityGroup curGroup)
{
	bool isValid = false;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	CSysConfigBase *pRealSysConfig = pProcess->GetSysConfig();
	CCfgData *pFlag = NULL;

	int typeValidator = ONE_LINE_BUFFER_LENGTH;
	eCfgParamDataType dataType = eCfgParamDataString;

	if(pRealSysConfig)
	{
		if(pRealSysConfig->IsParamExist(key))
		{
			pFlag = pRealSysConfig->GetCfgEntryByKey(key);
			if(pFlag)
			{
				typeValidator = pFlag->GetTypeValidator();
				dataType = pFlag->GetCfgDataType();
				isValid = true;
				TRACESTR(eLevelInfoNormal) << "CServiceConfig::TakeCfgParam - IsParamExist";

			}
		}
		else if (0 == strcmp(section, GetCfgSectionName(eCfgSectionCSModule)))
		{
			TRACESTR(eLevelInfoNormal) << "CServiceConfig::TakeCfgParam - strcmp(section, GetCfgSectionName(eCfgSectionCSModule)) ";
			isValid = true;
		}
	}

	if(isValid)
	{
		AddParamNotVisible(section, key, data, GetCfgParamTypeState(), typeValidator, curGroup, dataType);
		//pRealSysConfig
	}
	else
	{
		string errMsg = "key: ";
			   errMsg += key;
			   errMsg += ", wasn't found !";
		TRACESTR(eLevelInfoNormal) << errMsg.c_str() << endl;
	}

	return isValid;

}
///////////////////////////////////////////
void CServiceConfig::FillMap(CServiceConfig *pServiceConfig)
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	CSysMap *sysMap=pSysConfig->GetMap();
	CSysMap::iterator iTer = sysMap->begin();
	CSysMap::iterator iEnd = sysMap->end();
	std::string strTemp;
	while(iEnd != iTer)
	{
		const CCfgData *cfgData = iTer->second;
		if(false == IsParamExist(cfgData->GetKey()))// && cfgData->GetSection() == GetCfgSectionName(eCfgSectionCSModule))
		{
			pServiceConfig->GetCSData(cfgData->GetKey(),strTemp);
			CCfgData *newCfgData = new CCfgData( cfgData->GetSection(), cfgData->GetKey(), strTemp, cfgData->GetCfgType(), cfgData->GetTypeValidator(),
					cfgData->GetCfgDataType(), cfgData->GetCfgParamVisibilityType(), cfgData->GetCfgParamResponsibilityGroup());
			(*m_Map)[cfgData->GetKey()] = newCfgData;

			//FTRACESTR(eLevelInfoNormal) << "CServiceConfig::FillMap - param name" << newCfgData->GetKey() << ",data:"<< newCfgData->GetData();
		}
		//TRACESTR(eLevelInfoNormal) << "CCommIPSListService::SendIpServiceList()--before endIpServiceParamInd(service);!!!!!!!!!!!!!";
		iTer++;
	}


//	pServiceConfig->GetCSData("SIP_REGISTER_DELAY_MILLI_SEC",sipRegisterDelayMiliSecData);
//	AddParam("CS_MODULE_PARAMETERS", "SIP_REGISTER_DELAY_MILLI_SEC", sipRegisterDelayMiliSecData, eCfgParamUser, _0_TO_100000_DECIMAL,eCfgParamDataNumber,eCfgParamNotVisible, eCfgParamResponsibilityGroup_McaConfParty);

}


/////////////////////////////////////////////////////////////
void CServiceConfig::Serialize(WORD format,CSegment *pSeg)
{

	//TRACESTR(eLevelInfoNormal) << "CServiceConfig::Serialize -map size:" << m_Map->size();;
	DWORD mapSize=m_Map->size();
	*pSeg << mapSize;
	CSysMap::iterator iTer = m_Map->begin();
	CSysMap::iterator iEnd = m_Map->end();
	*pSeg << m_Id;
	while(iEnd != iTer)
	{

		const CCfgData *cfgData = iTer->second;
		string key = cfgData->GetKey();
		*pSeg << key;
//		switch(cfgData->GetCfgDataType())
//		{
//			case eCfgParamDataBoolean:
//			{
//				WORD boolData = (CFG_STR_YES == cfgData->GetData() ? 1 : 0);
//				*pSeg << boolData;
//
//				break;
//			}
//			case eCfgParamDataNumber:
//			{
//				string cfgIntData = cfgData->GetData();
//				DWORD intData = atol(cfgIntData.c_str());
//
//				// if it's a hexa number format
//				int found = cfgIntData.find("0x");
//				if(0 == found)
//				{
//					sscanf(cfgIntData.c_str(), "%x", &intData);
//				}
//				TRACESTR(eLevelInfoNormal) << "CServiceConfig::Serialize- intData" << intData;
//				*pSeg << intData;
//				break;
//			}
//			case eCfgParamDataEnum:
//			default:
				std::string sCfgData = cfgData->GetData();
				//char * cfgData;
				//strcpy(cfgData,sCfgData.c_str());
				//TRACESTR(eLevelInfoNormal) << "CServiceConfig::Serialize - param data" << cfgData->GetData();
				*pSeg << sCfgData;
//				break;
//		}
		iTer++;
		//TRACESTR(eLevelInfoNormal) << "CCommProxyService::SendIpServiceParamInd - param name" << cfgData->GetData();
	}

}

///////////////////////////////////////////////////////
void CServiceConfig::DeSerialize(CSegment *pSeg)
{
	//pSeg->Get((BYTE*)&m_sipProxyIpParamsStruct, sizeof(SIP_PROXY_IP_PARAMS_S));
	char buffData			[512];
	memset(buffData, 		'\0', sizeof(buffData));

	CProcessBase *pProcess = CProcessBase::GetProcess();
	CSysConfigBase *realSysConfig = (CSysConfigBase*)pProcess->GetSysConfig();
	DWORD mapSize=0;
	*pSeg >>mapSize;
	*pSeg >> m_Id;
	eCfgParamDataType cfgParamDataType = eCfgParamDataString;
	for (int i = 0; i < (int)mapSize; i++)
	{
		string key="";
		*pSeg >> key;
		//TRACESTR(eLevelInfoNormal) << "CServiceConfig::DeSerialize :key:" << key;
		bool isParamExist = realSysConfig->IsParamExist(key);
		cfgParamDataType = eCfgParamDataString; // reset data type
		if(isParamExist)
		{
			CCfgData *cfgData = NULL;

			cfgData = realSysConfig->GetCfgEntryByKey(key);
			if(!cfgData)
				continue;

			strncpy(buffData, cfgData->GetData().c_str(), sizeof(buffData) - 1);
			buffData[sizeof(buffData) - 1] = 0;

			//val = cfgData->GetTypeValidator();
			//cfgParamDataType = cfgData->GetCfgDataType();
			std::string sData="";
			*pSeg >>sData;


			AddParam(cfgData->GetSection(),key,sData,cfgData->GetCfgType(),cfgData->GetTypeValidator(),cfgData->GetCfgDataType(),cfgData->GetCfgParamVisibilityType(),cfgData->GetCfgParamResponsibilityGroup());

/*
			switch( cfgParamDataType )
			{
				case eCfgParamDataEnum:*/
		}
	}
}

////////////////////////////////////////////////////////////////////////
void CServiceConfig::PrintServiceConfig()
{
	string print_string = "\nCServiceConfig::PrintServiceConfigList";
  
	DWORD mapSize=m_Map->size();
	
	CSysMap::iterator iTer = m_Map->begin();
	CSysMap::iterator iEnd = m_Map->end();
	
	print_string += "\nService Id:     ";
	char id[2];
	snprintf(id, sizeof(id), "%02d", m_Id);

	print_string += id;

	while(iEnd != iTer)
	{
		const CCfgData *cfgData = iTer->second;
		print_string += "\nKey = ";
		print_string += cfgData->GetKey(); 
		print_string += " ;Data = ";
		print_string += cfgData->GetData();
		iTer++;
	}
	
    TRACESTR(eLevelInfoNormal) << print_string.c_str();

}

////////////////////////////////////////////////////////////////////////
//void CServiceConfig::AddParamToCfgMap(const std::string &section, const std::string &key,const std::string &data,
//							  eCfgParamType cfgType, int validatorType, eCfgParamDataType dataType,
//							  eCfgParamVisibilityType cfgParamVisibilityType,
//							  eCfgParamResponsibilityGroup cfgParamResponsibilityGroup)const
//{
//	if((m_Map->end() == m_Map->find(key)))
//	{
//		CCfgData *cfgData = new CCfgData( section, key, data, cfgType, validatorType, dataType, cfgParamVisibilityType, cfgParamResponsibilityGroup);
//		(*m_Map)[key] = cfgData;
//	}
//	else
//	{
//		//CMedString errorMessage = "Parameter already exist: <";
//		//errorMessage << key.c_str() << " = " << data.c_str();
//		//PASSERTMSG(1, errorMessage.GetString());
//
//	}
//}





