// SysConfig.cpp: implementation of the CSysConfigBase class.
//
//
//Date         Updated By         Description
//
//27/10/05	  Yuri Ratner		System Configuration Base Manipulations
//========   ==============   =====================================================================

#include <ostream>
#include <stdio.h>
#include <iomanip>
#include <stdlib.h>
#include <algorithm>

#include "SysConfigBase.h"
#include "psosxml.h"
#include "OsFileIF.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ObjString.h"
#include "ProcessBase.h"
#include "ObjString.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "SysConfigKeys.h"

/*-------------------------------------------------------------------------------
	CCfgData used in CFG Map. This is a main structure for CFG parameters
-------------------------------------------------------------------------------*/
CCfgData::CCfgData(const std::string &section, 
		   const std::string &key,
		   const std::string &data,
		   eCfgParamType cfgType,
		   int cfgTypeValidator,
		   eCfgParamDataType cfgDataType,
		   eCfgParamVisibilityType cfgParamVisibilityType,
		   eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
		   int counter):
m_Key(key), 
m_cfgTypeValidator(cfgTypeValidator),
m_CfgType(cfgType),
m_CfgParamDataType(cfgDataType), 
m_CfgParamVisibilityType(cfgParamVisibilityType),
m_CfgParamResponsibilityGroup(cfgParamResponsibilityGroup)
{
	m_Section			= section;
	m_Data 				= data;
	m_Counter 			= counter;
	m_IsReset 			= true;
	m_ProcessType 		= eProcessTypeInvalid;
}

CCfgData::CCfgData(const std::string &section,
		   const std::string &key,
		   const std::string &data,
		   eCfgParamType cfgType,
		   int cfgTypeValidator,
		   bool isReset,
		   eCfgParamDataType cfgDataType,
		   eCfgParamVisibilityType cfgParamVisibilityType,
		   eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
		   eProcessType processType,
		   bool isAllProcesses,
		   int counter):
  m_Key(key),
  m_cfgTypeValidator(cfgTypeValidator),
  m_IsReset(isReset),
  m_CfgType(cfgType),
  m_CfgParamDataType(cfgDataType),
  m_CfgParamVisibilityType(cfgParamVisibilityType),
  m_CfgParamResponsibilityGroup(cfgParamResponsibilityGroup),
  m_ProcessType(processType),
  m_IsAllProcesses(isAllProcesses)
{
	m_Section			= section;
	m_Data 				= data;
	m_Counter 			= counter;
}

CCfgData::CCfgData(const CCfgData &other)
	: CPObject(other)
	, m_Section(other.m_Section)
	, m_Key(other.m_Key)
	, m_Data(other.m_Data)
	, m_IsReset(other.m_IsReset)
	, m_CfgType(other.m_CfgType)
	, m_CfgParamDataType(other.m_CfgParamDataType)
	, m_CfgParamVisibilityType(other.m_CfgParamVisibilityType)
	, m_CfgParamResponsibilityGroup(other.m_CfgParamResponsibilityGroup)
	, m_ProcessType(other.m_ProcessType)
	, m_IsAllProcesses(other.m_IsAllProcesses)
{
	m_cfgTypeValidator  = other.m_cfgTypeValidator;
	m_Counter 			= other.m_Counter;
}

void CCfgData::Dump(std::ostream &ostr) const
{
	ostr << std::setw(20) <<  m_Data;
}

bool CCfgData::TestValidity(const CCfgData *obj)
{
	bool result = true;
	const char *errorStr = "OK";
	if(NULL == obj)
	{
		errorStr = "Error in CCfgData object; NULL == obj";
		result = false;
	}
	else if(FALSE == IsValidPObjectPtr(obj))
	{
		errorStr = "Error in CCfgData object; FALSE == IsValidPObjectPtr(obj)";
		result = false;
	}
	else if(obj->GetCfgParamVisibilityType() == eCfgParamVisible && obj->GetCfgType() != eCfgParamUser)
	{
		errorStr = "Error in CCfgData object; m_CfgParamVisibilityType == eCfgParamVisible && m_CfgType != eCfgParamUser";
		result = false;
	}

	FPASSERTMSG(false == result, errorStr);

	return result;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction/Virtuals
//////////////////////////////////////////////////////////////////////

CSysConfigBase::CSysConfigBase()
{
    m_Map = new CSysMap;
    m_SerializeVisibleOnly = false;

    m_IsServiceCFG = false;
    m_eCfgParamType = eCfgParamUser;
   // m_ActiveAlarmErrorBuf[0] = '\0';
   // m_FaultErrorBuf[0] = '\0';
    memset(m_ActiveAlarmErrorBuf, '\0', sizeof(m_ActiveAlarmErrorBuf));
    memset(m_FaultErrorBuf, '\0', sizeof(m_FaultErrorBuf));
}

//////////////////////////////////////////////////////////////////////

static void copy_map(const CSysMap& src, CSysMap& dst)
{
	//std::cerr << "XXX Map len: " << src.size() << std::endl;
	for (CSysMap::const_iterator i = src.begin(); i != src.end(); ++i)
	{
		//std::cerr << "XXX First: " << i->first << " Second: " << i->second << std::endl;
		dst[i->first] = new CCfgData(*i->second);
	}
}

CSysConfigBase::CSysConfigBase(const CSysConfigBase &other)
:CSerializeObject(other)
{
	m_Map = new CSysMap;
	copy_map(*other.m_Map, *m_Map);

	m_SerializeVisibleOnly = other.m_SerializeVisibleOnly;
	m_IsServiceCFG = other.m_IsServiceCFG;
	m_eCfgParamType = other.m_eCfgParamType;
	strncpy(m_ActiveAlarmErrorBuf, other.m_ActiveAlarmErrorBuf, ERROR_MESSAGE_LEN - 1);
	strncpy(m_FaultErrorBuf, other.m_ActiveAlarmErrorBuf, ERROR_MESSAGE_LEN - 1);
}

CSysConfigBase& CSysConfigBase::operator=( const CSysConfigBase& other )
{
	if(this == &other){
		return *this;
	}

	CSerializeObject::operator=(other);

	MapFree();
	m_Map = new CSysMap;

	copy_map(*other.m_Map, *m_Map);

	m_SerializeVisibleOnly = other.m_SerializeVisibleOnly;
	m_IsServiceCFG = other.m_IsServiceCFG;
	m_eCfgParamType = other.m_eCfgParamType;
	strncpy(m_ActiveAlarmErrorBuf, other.m_ActiveAlarmErrorBuf, ERROR_MESSAGE_LEN - 1);
	strncpy(m_FaultErrorBuf, other.m_ActiveAlarmErrorBuf, ERROR_MESSAGE_LEN - 1);

	return *this;
}

//////////////////////////////////////////////////////////////////////
CSysConfigBase::~CSysConfigBase()
{
	MapFree();
}

//////////////////////////////////////////////////////////////////////
bool CSysConfigBase::LoadFromFile(eCfgParamType type)
{
	STATUS status = STATUS_OK;

	static eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
	string fileName = GetFileName(type);
	TRACESTR(eLevelInfoNormal) << "CSysConfigBase::LoadFromFile - " << fileName;

	int size = GetFileSize(fileName);
	if (0 < size)
	{
		eFailReadingFileActiveAlarmType aaPolicy 	= eNoActiveAlarm;
		eFailReadingFileOperationType operationType = eNoAction;
		if(eProcessMcuMngr == processType)
		{
			aaPolicy = eActiveAlarmExternal;
			operationType = eRenameFile;
		}
		SetCfgParamTypeState(type);
		status = ReadXmlFile(fileName.c_str(), aaPolicy, operationType);
		TRACESTR(eLevelInfoNormal) << "CSysConfigBase::LoadFromFile -status " << status << " filename " << fileName;
	}

	return (STATUS_OK == status || STATUS_CFG_PARAM_NOT_EXIST == status); //kobig , when an Unknown flag will be identified as result of Remooved flag or new unknown visible flag from other version we will not set an active alaram
}

//////////////////////////////////////////////////////////////////////
void CSysConfigBase::SaveToFile(eCfgParamType type)
{
	WriteXmlFile(GetFileName(type));
}

//////////////////////////////////////////////////////////////////////
void CSysConfigBase::SaveToFile(const char *fileName)
{
	TRACESTR(eLevelInfoNormal) << "CSysConfigBase::SaveToFile - " << fileName;
	//m_SerializeVisibleOnly =
	WriteXmlFile(fileName);
}


//////////////////////////////////////////////////////////////////////
void CSysConfigBase::SerializeXml(CXMLDOMElement*& pXMLRootElement) const
{
	CXMLDOMElement *pCfgElement 		= NULL;
	CXMLDOMElement *pSectionElement 	= NULL;
	CXMLDOMElement *pPairElement 		= NULL;

	CCfgParamsVector vect;
	ConvertMapToVector(vect);

	std::string rootStr;
	if (m_IsServiceCFG)
		rootStr="SERVICE_CFG";
	else
		rootStr="SYSTEM_CFG";

	if(NULL == pXMLRootElement)
	{
		pXMLRootElement = new CXMLDOMElement;
		pXMLRootElement->set_nodeName("SYSTEM_CFG");

		// VNGR-18489
		pCfgElement = pXMLRootElement;
	}
	else
	{
		if (m_IsServiceCFG)
			pCfgElement = pXMLRootElement->AddChildNode("SERVICE_CFG");
		else
			pCfgElement = pXMLRootElement->AddChildNode("SYSTEM_CFG");

	}

	std::string currentSection = "";
    bool isCSModuleSectionAppear		= false,
         isMcmsUserModuleSectionAppear	= false;

	CCfgParamsVector::iterator iTer = vect.begin();
	CCfgParamsVector::iterator iEnd = vect.end();
	for(; iEnd != iTer ; iTer++)
	{
		const CCfgData *pCfgEntry = *iTer;


        if(m_eCfgParamType != NumOfCfgTypes && m_eCfgParamType != pCfgEntry->GetCfgType())
        {
            continue;
        }
        if(true == m_SerializeVisibleOnly && eCfgParamNotVisible == pCfgEntry->GetCfgParamVisibilityType())
		{
			continue;
		}

		if(currentSection != pCfgEntry->GetSection())
		{
			currentSection = pCfgEntry->GetSection();
			pSectionElement = pCfgElement->AddChildNode("CFG_SECTION");
			pSectionElement->AddChildNode("NAME", currentSection.c_str());

            if(currentSection == GetCfgSectionName(eCfgSectionCSModule))
            {
                isCSModuleSectionAppear = true;
            }
            else if(currentSection == GetCfgSectionName(eCfgSectionMcmsUser))
            {
                isMcmsUserModuleSectionAppear = true;
            }
		}

		if(NULL == pSectionElement)
		{
			pSectionElement = pCfgElement->AddChildNode("CFG_SECTION");
			pSectionElement->AddChildNode("NAME", currentSection.c_str());
		}

		pPairElement = pSectionElement->AddChildNode("CFG_PAIR");
		pPairElement->AddChildNode("KEY" , pCfgEntry->GetKey().c_str());


		switch(pCfgEntry->GetCfgDataType())
		{
			case eCfgParamDataBoolean:
			{
				WORD boolData = (CFG_STR_YES == pCfgEntry->GetData() ? 1 : 0);
				pPairElement->AddChildNode("DATA", boolData, pCfgEntry->GetTypeValidator());
//				cout << "Bool| Key: " << pCfgEntry->GetKey().c_str()
//					 << ", Data: " << boolData << endl;
				break;
			}
			case eCfgParamDataNumber:
			{
				string cfgIntData = pCfgEntry->GetData();
				int intData = atoi(cfgIntData.c_str());

				// if it's a hexa number format
				int found = cfgIntData.find("0x");
				if(0 == found)
				{
					sscanf(cfgIntData.c_str(), "%x", &intData);
				}


				pPairElement->AddChildNode("DATA", intData, pCfgEntry->GetTypeValidator());
				break;
			}
			case eCfgParamDataEnum:
			default: //eCfgParamDataString
				pPairElement->AddChildNode("DATA", pCfgEntry->GetData().c_str());

//				cout << "Default| Key: " << pCfgEntry->GetKey().c_str()
//			 	 	 << ", Data: " << pCfgEntry->GetData().c_str() << endl;
				break;

		}

		//pPairElement->AddChildNode("DATA", pCfgEntry->GetData().c_str());//, pCfgEntry->GetTypeValidator());
	}

    if(eCfgParamUser == m_eCfgParamType)
    {
        if(false == isMcmsUserModuleSectionAppear)
        {
            pSectionElement = pCfgElement->AddChildNode("CFG_SECTION");
            pSectionElement->AddChildNode("NAME", GetCfgSectionName(eCfgSectionMcmsUser));
        }
        if(false == isCSModuleSectionAppear)
		{
        	pSectionElement = pCfgElement->AddChildNode("CFG_SECTION");
			pSectionElement->AddChildNode("NAME", GetCfgSectionName(eCfgSectionCSModule));
		}
    }
}

CSysConfigBase* CSysConfigBase::GetSysConfig()
{
	CProcessBase *pProcess = CProcessBase::GetProcess();
	CSysConfigBase *realSysConfig = (CSysConfigBase*)pProcess->GetSysConfig();
	return realSysConfig;

}

//////////////////////////////////////////////////////////////////////
int CSysConfigBase::DeSerializeXml(CXMLDOMElement* pXMLRootElement, char* pszError, const char* action)
{
  int nStatus   = STATUS_OK;
  int retStatus = STATUS_OK;

  CXMLDOMElement* pSectionElement  = NULL;
  CXMLDOMElement* pPairElement     = NULL;

  // will get the flag's type validator.
  int val = -1;

  char buffSectionName[256];  memset(buffSectionName, '\0', sizeof(buffSectionName));
  char buffKey        [256];  memset(buffKey        , '\0', sizeof(buffKey        ));
  char buffData       [512];  memset(buffData       , '\0', sizeof(buffData       ));
  char tmpData        [512];  memset(tmpData        , '\0', sizeof(tmpData        ));

  DWORD  intData = 0;
  string buff = "";
  string AAbuff = "", faultBuff = "";
  bool   isValid = true;
  bool   isFromEma = false;

  CProcessBase*   pProcess = CProcessBase::GetProcess();
  CSysConfigBase* realSysConfig = (CSysConfigBase*)pProcess->GetSysConfig();

  if (!realSysConfig)
    realSysConfig = this;

  // if it's from EMA, no assert will be sent.
  if (!this->GetMap() || (0 == this->GetMap()->size()))
    isFromEma = true;

  bool              isMcmsSection = true;
  eCfgParamDataType cfgParamDataType = eCfgParamDataString;

  GET_FIRST_CHILD_NODE(pXMLRootElement, "CFG_SECTION", pSectionElement);
  while (NULL != pSectionElement)
  {
    buffSectionName[0] = '\0';
    GET_VALIDATE_CHILD(pSectionElement, "NAME", buffSectionName, ONE_LINE_BUFFER_LENGTH);

    if (0 == strcmp(buffSectionName, GetCfgSectionName(eCfgSectionCSModule)))
      isMcmsSection = false;
    else
      isMcmsSection = true;

    GET_FIRST_CHILD_NODE(pSectionElement, "CFG_PAIR", pPairElement);
    while (NULL != pPairElement)
    {
      memset(buffKey, '\0', sizeof(buffKey));
      string strBuf = "";

      GET_VALIDATE_CHILD(pPairElement, "KEY", buffKey, ONE_LINE_BUFFER_LENGTH);

      val = ONE_LINE_BUFFER_LENGTH;
      bool isParamExist = realSysConfig->IsParamExist(buffKey);
      bool isParamJitcSupported = realSysConfig->IsParamSupportedInJitcMode(buffKey);
      bool isSuppress = IsSuppressedValidity(buffKey);

      cfgParamDataType = eCfgParamDataString; // reset data type

      if ((true == IsUnderJITCState()) && (false == isParamJitcSupported))
      {
        nStatus = STATUS_CFG_PARAM_UNSUPPORTED_JITC_MODE;
        strBuf = "Flag is not supported: ";
        strBuf += buffKey;
        if (isValid)
        {
          retStatus = nStatus;
          isValid = false;
          buff = strBuf;
        }
      }
      else if (isParamExist || !isMcmsSection)
      {
        CCfgData* cfgData = NULL;

        if (isParamExist)
        {
          cfgData = realSysConfig->GetCfgEntryByKey(buffKey);
          if (!cfgData)
            continue;

          strncpy(buffData, cfgData->GetData().c_str(), sizeof(buffData));
          buffData[sizeof(buffData)-1] = '\0';          

          //update visibility type according to SystemCfgUser.xml only
          bool isSectionUser = false;
          if (0 == strcmp(buffSectionName, GetCfgSectionName(eCfgSectionMcmsUser)))
        	  isSectionUser = true;
		  val = cfgData->GetTypeValidator();
		  cfgParamDataType = cfgData->GetCfgDataType();
		  if (cfgData->GetKey() == "trace1level" ||cfgData->GetKey() == "trace2level" ||cfgData->GetKey() == "trace3level"
			  || cfgData->GetKey()=="JITC_MODE")
			  cfgData->SetCfgParamVisibilityType(eCfgParamInFileNonVisible);
		  else if (isSectionUser == true)
			  cfgData->SetCfgParamVisibilityType(eCfgParamVisible);

        }

        switch (cfgParamDataType)
        {
          case eCfgParamDataEnum:
          {
            GET_VALIDATE_CHILD(pPairElement, "DATA", &intData, val);
            if (STATUS_OK == nStatus)
            {
              GET_VALIDATE_CHILD(pPairElement, "DATA", tmpData, ONE_LINE_BUFFER_LENGTH);
              strcpy(buffData, tmpData);
            }
            break;
          }

          case eCfgParamDataNumber:
          {
          	int dataVal = (int)intData;
            GET_VALIDATE_CHILD(pPairElement, "DATA", &dataVal, val);
            if (STATUS_OK == nStatus)
            {
                sprintf(buffData, "%d", dataVal);
            }
            else // check if it's a hex number
            {
              STATUS tmpStatus = nStatus;
              GET_VALIDATE_CHILD(pPairElement, "DATA", tmpData, ONE_LINE_BUFFER_LENGTH)

              // if it's a hex number format, and the it's a number
              if ((tmpData[0] == '0' && 'x' == tmpData[1]) && (CObjString::IsNumeric(tmpData+2)))
            	  sscanf(tmpData, "%x", &dataVal);
              else
                nStatus = tmpStatus;
            }
            break;
          }

          case eCfgParamDataBoolean:
          {
            GET_VALIDATE_CHILD(pPairElement, "DATA", &intData, val);
            if (STATUS_OK == nStatus)
            {
              strncpy(buffData, (intData == 1 ?  CFG_STR_YES : CFG_STR_NO), sizeof(buffData)-1);
              buffData[sizeof(buffData)-1] = '\0';
            }
            break;
          }

          case eCfgParamDataIpAddress:
          case eCfgParamDataString:
          {
            GET_VALIDATE_CHILD(pPairElement, "DATA", tmpData, val);
            if (STATUS_OK == nStatus)
              strcpy(buffData, tmpData);
            break;
          }

          default:
          {
            TRACESTR(eLevelInfoNormal) << "DataType for " << buffKey << " wasn't initialized\n";
          }
        } // end switch/case

        if (STATUS_OK == nStatus)
        {
          buffData[sizeof(buffData)-1] = '\0';
          nStatus = CheckSpecialVerification(buffKey, buffData);
          if (STATUS_OK != nStatus)
          {
            // restore the default value
            strncpy(buffData, cfgData->GetData().c_str(), sizeof(buffData));
          }
        }

        eCfgParamResponsibilityGroup curGroup;

        if (!isMcmsSection)
        {
          nStatus = STATUS_OK;
          curGroup = eCfgParamResponsibilityGroup_SwSysIp;
        }
        else
          curGroup = cfgData->GetCfgParamResponsibilityGroup();

        buffData[sizeof(buffData)-1] = '\0';
        //overwrite flag data in memory
        TakeCfgParam(buffKey, buffData, buffSectionName, curGroup);
      }
      else // if flag doesn't exist
      {
        nStatus = STATUS_CFG_PARAM_NOT_EXIST;
        strBuf = "Flag does not exist: ";
        strBuf += buffKey;
        if (isValid)
        {
	      if (!isSuppress)
          retStatus = nStatus;
          isValid = false;
          buff = strBuf;
          faultBuff = strBuf;
        }
      }

      if (STATUS_OK != nStatus)
      {
        // Store the nStatus, since after next GET_VALIDATE_CHILD,
        // nStatus, will be assigned by STATUS_OK
        if (isValid && !isSuppress)
          retStatus = nStatus;

        if ((STATUS_CFG_PARAM_NOT_EXIST != nStatus) && (STATUS_CFG_PARAM_UNSUPPORTED_JITC_MODE != nStatus))
        {
          GET_VALIDATE_CHILD(pPairElement, "DATA", tmpData, ONE_LINE_BUFFER_LENGTH)

          strBuf = "\"";
          strBuf += tmpData;
          strBuf += "\" isn't a valid value for the flag:  ";
          strBuf += buffKey;
        }

        if (!isSuppress && isValid)
        {
          isValid = false;

          buff = strBuf;
          AAbuff = strBuf;
        }

        if (!isSuppress && !isFromEma && (eProcessMcuMngr == pProcess->GetProcessType()))
        {
          FPASSERTMSG(TRUE, strBuf.c_str());
        }
        else
        {
          TRACEINTO << strBuf.c_str();
        }
      }
      GET_NEXT_CHILD_NODE(pSectionElement, "CFG_PAIR", pPairElement);
    }
    GET_NEXT_CHILD_NODE(pXMLRootElement, "CFG_SECTION", pSectionElement);
  }

  if (!isValid)
  {
    memset(m_ActiveAlarmErrorBuf, '\0', sizeof(m_ActiveAlarmErrorBuf));
    strcpy(pszError, AAbuff.c_str());
    int len = MIN_(strlen(pszError)+1, sizeof(m_ActiveAlarmErrorBuf));
    strncpy(m_ActiveAlarmErrorBuf, pszError, len);
    m_ActiveAlarmErrorBuf[ERROR_MESSAGE_LEN-1] = '\0';

    memset(m_FaultErrorBuf, '\0', sizeof(m_FaultErrorBuf));
    strcpy(pszError, faultBuff.c_str());
    len = MIN_(strlen(pszError)+1, sizeof(m_FaultErrorBuf));
    strncpy(m_FaultErrorBuf, pszError, len);
    m_FaultErrorBuf[ERROR_MESSAGE_LEN-1] = '\0';
  }

  return retStatus;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool CSysConfigBase::IsSuppressedValidity(const char*) const
{
    // do not suppress validity at base class
    return false;
}

////////////////////////////////////////////////////////////////////////////////
STATUS CSysConfigBase::CheckSpecialVerification(const string &key, const string &data)
{
	STATUS status = STATUS_OK;
	if(key == CFG_KEY_MCU_DISPLAY_NAME)
	{
		status = VerifyMcuDispName(data.c_str(), data.length());
	}

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CSysConfigBase::VerifyMcuDispName(const char *data, int len)
{

	for(int i = 0; i < len; ++i)
	{
		if( isalnum(data[i]) ||
		    ('-' == data[i])  ||
		    ('.' == data[i])  ||
		    ('!' == data[i])  ||
		    ('%' == data[i])  ||
		    ('*' == data[i])  ||
		    ('_' == data[i])  ||
		    (' ' == data[i])  ||
		    ('+' == data[i])  ||
		    ('`' == data[i])  ||
		    ('\'' == data[i])  ||
		    ('~' == data[i])  )
			continue;

		return STATUS_ILLEGAL_CHARS_IN_FLAG;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
// Static
bool CSysConfigBase::IsUnderJITCState(void)
{
	bool result = false;

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
		result = true;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////
bool CSysConfigBase::IsParamSupportedInJitcMode(const std::string &key)const
{
	bool isSupported = true;
	if ( 0 == key.find("EXTERNAL_CONTENT_") )
	{
		isSupported = false; // CMA AddressBook flags ("EXTERNAL_CONTENT_...") are illegal in JITC mode
	}

	return isSupported;
}

//////////////////////////////////////////////////////////////////////
bool CSysConfigBase::IsParamExist(const std::string &key)const
{
	return m_Map->end() != m_Map->find(key);
}

//////////////////////////////////////////////////////////////////////
void CSysConfigBase::RemoveParamsInFileNonVisible()
{
	CSysMap::iterator iTer = m_Map->begin();
	CSysMap::iterator iEnd = m_Map->end();
	while(iEnd != iTer)
	{
		const CCfgData* cfgData = iTer->second;
		if (eCfgParamInFileNonVisible == cfgData->GetCfgParamVisibilityType())
		{
			m_Map->erase(iTer++);
		}
		else
		{
			iTer++;
		}
	}
}



//////////////////////////////////////////////////////////////////////
void CSysConfigBase::AddParamNotVisible(const std::string &section, const std::string &key,const std::string &data,
										eCfgParamType cfgType, int validatorType,
										eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
										eCfgParamDataType dataType)const
{
	AddParam(section, key, data, cfgType, validatorType, dataType, eCfgParamNotVisible, cfgParamResponsibilityGroup);
}

void CSysConfigBase::AddParamNotVisible(const std::string &section, const std::string &key,const std::string &data,
										 eCfgParamType cfgType, int validatorType, bool isReset,
										 eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
										 eCfgParamDataType dataType,
										 eProcessType processType,
										 bool isAllProcesses)const
{
	AddParam(section, key, data, cfgType, validatorType, isReset, dataType, eCfgParamNotVisible, cfgParamResponsibilityGroup, processType, isAllProcesses);
}

void CSysConfigBase::AddParamInFileNonVisible(const std::string &section, const std::string &key,const std::string &data,
										eCfgParamType cfgType, int validatorType,
										eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
										eCfgParamDataType dataType)const
{
	AddParam(section, key, data, cfgType, validatorType, dataType, eCfgParamInFileNonVisible, cfgParamResponsibilityGroup);
}

//////////////////////////////////////////////////////////////////////
void CSysConfigBase::AddParamVisible(const std::string &section, const std::string &key,const std::string &data,
									 eCfgParamType cfgType, int validatorType,
									 eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
									 eCfgParamDataType dataType)const
{
	AddParam(section, key, data, cfgType, validatorType, dataType, eCfgParamVisible, cfgParamResponsibilityGroup);
}

void CSysConfigBase::AddParamVisible(const std::string &section, const std::string &key,const std::string &data,
									 eCfgParamType cfgType, int validatorType, bool isReset,
									 eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
									 eCfgParamDataType dataType,
									 eProcessType processType,
									 bool isAllProcesses)const
{
	AddParam(section, key, data, cfgType, validatorType, isReset, dataType, eCfgParamVisible, cfgParamResponsibilityGroup, processType, isAllProcesses);
}



//////////////////////////////////////////////////////////////////////
void CSysConfigBase::AddParam(const std::string &section, const std::string &key,const std::string &data,
  							  eCfgParamType cfgType, int validatorType, eCfgParamDataType dataType,
  							  eCfgParamVisibilityType cfgParamVisibilityType,
  							  eCfgParamResponsibilityGroup cfgParamResponsibilityGroup)const
{
	if(false == IsParamExist(key))
	{
		CCfgData *cfgData = new CCfgData( section, key, data, cfgType, validatorType,
										  dataType, cfgParamVisibilityType, cfgParamResponsibilityGroup);
		(*m_Map)[key] = cfgData;
	}
	else
	{

		CMedString errorMessage = "Parameter already exist: <";
		errorMessage << key.c_str() << " = " << data.c_str();
		PASSERTMSG(1, errorMessage.GetString());
	}
}


void CSysConfigBase::AddParam(const std::string &section, const std::string &key,const std::string &data,
  							  eCfgParamType cfgType, int validatorType, bool isReset, eCfgParamDataType dataType,
  							  eCfgParamVisibilityType cfgParamVisibilityType,
  							  eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
  							  eProcessType processType,
  							  bool isAllProcesses)const
{
	if(false == IsParamExist(key))
	{
		CCfgData *cfgData = new CCfgData( section, key, data, cfgType, validatorType, isReset,
										  dataType, cfgParamVisibilityType, cfgParamResponsibilityGroup, processType, isAllProcesses);
		(*m_Map)[key] = cfgData;
	}
	else
	{

		CMedString errorMessage = "Parameter already exist: <";
		errorMessage << key.c_str() << " = " << data.c_str();
		PASSERTMSG(1, errorMessage.GetString());
	}
}



//////////////////////////////////////////////////////////////////////
BOOL CSysConfigBase::GetAllParamsBySection(eCfgSections eSection, CCfgParamsVector &params)
{
	const string sectionCsName = GetCfgSectionName(eSection);

	CSysMap::iterator iTer = m_Map->begin();
	CSysMap::iterator iEnd = m_Map->end();
	while(iEnd != iTer)
	{
		const CCfgData *cfgData = iTer->second;
		bool res = CCfgData::TestValidity(cfgData);
		if(false == res)
		{
			return FALSE;
		}

		if(sectionCsName == cfgData->GetSection())
		{
			params.push_back(cfgData);
		}

		iTer++;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
void CSysConfigBase::MapFree()
{
	PASSERTMSG_AND_RETURN(!m_Map, "Wrong program flow");

	CSysMap::iterator iTer = m_Map->begin();
	CSysMap::iterator iEnd = m_Map->end();
	while(iTer != iEnd)
	{
		PDELETE(iTer->second);
		m_Map->erase(iTer);
		iTer = m_Map->begin();
	}

	PDELETE(m_Map);
}

// Return whether first element is greater than the second
bool CfgGreater (const CCfgData *cfgData1, const CCfgData *cfgData2)
{
	return cfgData1->GetSection() > cfgData2->GetSection();
}

//////////////////////////////////////////////////////////////////////
void CSysConfigBase::ConvertMapToVector(CCfgParamsVector &vect)const
{
	vect.reserve(m_Map->size());

	CSysMap::iterator iTer = m_Map->begin();
	CSysMap::iterator iEnd = m_Map->end();
	while(iEnd != iTer)
	{
		const CCfgData *cfgData = iTer->second;
		if(false == CCfgData::TestValidity(cfgData))
		{
			return;
		}

		vect.push_back(cfgData);

		iTer++;
	}

	// sort the vector by section name
	sort(vect.begin(), vect.end( ), &CfgGreater);

}

/////////////////////////////////////////////////////////////////////////////
eCfgParamDataType CSysConfigBase::GetCfgDataTypeByData(const string &data)const
{
	eCfgParamDataType cfgDataType = eCfgParamDataString;
	if(IsBoolean(data.c_str()))
	{
		cfgDataType = eCfgParamDataBoolean;
	}
	else if(CObjString::IsNumeric(data.c_str()))
	{
		cfgDataType = eCfgParamDataNumber;
	}
// 	else if(IsIpAddress4(data.c_str()))
// 	{
// 		cfgDataType = eCfgParamDataIpAddress;
// 	}
	return cfgDataType;
}

/////////////////////////////////////////////////////////////////////////////
bool CSysConfigBase::IsBoolean(const char *strTested)const
{
	return  (0 == strcmp(strTested, "YES") || 0 == strcmp(strTested, "NO"));
}


/////////////////////////////////////////////////////////////////////////////
bool CSysConfigBase::IsIpAddress4(const char *strTested)const
{
	const char 	*token = ".";
	char  		source[128];
	strncpy(source, strTested, sizeof(source) - 1);
	source[sizeof(source) - 1] = 0;

	int cnt = 0;
	char *tmp  = strtok(source, token);
	while(NULL != tmp)
	{
		if(false == CObjString::IsNumeric(tmp))
		{
			break;
		}

        DWORD numericTmp = atoi(tmp);
        if(numericTmp > 255)
        {
            break;
        }


		tmp  = strtok(NULL, token);
		cnt++;
	}

	return (4 == cnt);
}


