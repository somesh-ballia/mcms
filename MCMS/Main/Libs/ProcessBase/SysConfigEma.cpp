#include <vector>
#include <algorithm>
#include <ostream>
#include <functional>


#include "SysConfigEma.h"
#include "SysConfig.h"
#include "OsFileIF.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "ProcessBase.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "ObjString.h"


//////////////////////////////////////////////////////////////////////
CSysConfigEma::CSysConfigEma()
{
}

//////////////////////////////////////////////////////////////////////
CSysConfigEma::CSysConfigEma(const CSysConfigEma &rHnd)
:CSysConfigBase(rHnd)
{

}

//////////////////////////////////////////////////////////////////////
CSysConfigEma::~CSysConfigEma()
{
}

//////////////////////////////////////////////////////////////////////
bool CSysConfigEma::TakeCfgParam(const char *key, const char *data, const char *section, const eCfgParamResponsibilityGroup curGroup)
{
	bool isValid = false;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	CSysConfigBase *pRealSysConfig = pProcess->GetSysConfig();
	CCfgData *pFlag = NULL;

	int typeValidator = ONE_LINE_BUFFER_LENGTH;
	eCfgParamDataType dataType = eCfgParamDataString;
	eCfgParamVisibilityType visibilityType = eCfgParamNotVisible;

	if(pRealSysConfig)
	{
		if(pRealSysConfig->IsParamExist(key))
		{
			pFlag = pRealSysConfig->GetCfgEntryByKey(key);
			if(pFlag)
			{
				typeValidator = pFlag->GetTypeValidator();
				dataType = pFlag->GetCfgDataType();
				visibilityType = pFlag->GetCfgParamVisibilityType();
				isValid = true;
			}
		}
		else if (0 == strcmp(section, GetCfgSectionName(eCfgSectionCSModule)))
		{
			isValid = true;
		}
	}

	if(isValid)
	{
		if (eCfgParamInFileNonVisible != visibilityType)
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




//////////////////////////////////////////////////////////////////////
/*
1. Parameters must be exist in defaults, CSModule section is an exception.
2. Type(User, Debug) must match.
3. Section Name must be the same.

returns: STATUS_OK         : valid and NOT different from the memory.
         STATUS_OK_WARNING : valid and different from the memory.

         STATUS_CFG_PARAM_TYPE_DONT_MATCH         : not valid.
         STATUS_CFG_PARAM_DATA_TYPE_DONT_MATCH
         STATUS_CFG_PARAM_DATA_TYPE_DONT_MATCH
         STATUS_CFG_PARAM_SECTION_DONT_MATCH
*/


/*
 Iterate through new flags and check if flags are valid (exist in old map, isLegalAsci, correct section, etc.).
 Set return status (if flag value has changed then STATUS_OK_WARNING).
 */
STATUS CSysConfigEma::IsValidOrChanged(CObjString & strError)const
{
	STATUS status = STATUS_OK;

	CProcessBase *pProcess = CProcessBase::GetProcess();
	//get map with old system cfg params from memory
	CSysConfigBase *realSysConfig = pProcess->GetSysConfig();
	//get iterators for map with new cfg params (for example: if received from transaction SET_CFG)
	CSysMap::iterator iTer = GetMap()->begin();
	CSysMap::iterator iEnd = GetMap()->end();
	while(iTer != iEnd)
	{
		const std::string &strKey 	= iTer->first;
		CCfgData 		*pData 	= iTer->second;
        const string &strData = pData->GetData();
        const string &section = pData->GetSection();

        eStringValidityStatus isAscii = CObjString::IsLegalAsciiString(const_cast<char *>(strKey.c_str()),
                                                                       strKey.length(),
                                                                       false);



        //EMA does not allow to input these params, so we never enter the if statement
        if (strKey.compare("trace1level") == 0 || strKey.compare("trace2level") == 0 || strKey.compare("trace3level") == 0)
        {
        	strError = "Key is not valid \n";
        	            strError << strKey;
        	            strError << " : ";
        	            strError << strData;

        	            status = STATUS_CFG_PARAM_TRY_ADD_CS_LOGS_TRACE;
        	            break;
        }

        if(eStringValid != isAscii)
        {
            strError = "Key is not a valid ASCII\n";
            strError << strKey;
            strError << " : ";
            strError << strData;

            status = STATUS_NODE_VALUE_NOT_ASCII;
            break;
        }

        isAscii = CObjString::IsLegalAsciiString(const_cast<char *>(strData.c_str()),
                                                 strData.length(),
                                                 false);
        if(eStringValid != isAscii)
        {
            strError = "Data is not a valid ASCII\n";
            strError << strKey;
            strError << " : ";
            strError << strData;

            status = STATUS_NODE_VALUE_NOT_ASCII;
            break;
        }
        //check if flag exists in old map
        bool isParamExist = realSysConfig->IsParamExist(strKey);
        bool isParamJitcSupported = realSysConfig->IsParamSupportedInJitcMode(strKey);
		if(false == isParamExist)
		{
			if(pData->GetSection() != GetCfgSectionName(eCfgSectionCSModule))
			{
				strError = "Param does not exist : ";
				strError << strKey;
				status = STATUS_CFG_PARAM_NOT_EXIST;
				break;
			}
            status = STATUS_OK_WARNING;
		}
		else if( (true  == IsUnderJITCState()) && (false == isParamJitcSupported) )
		{
			if(pData->GetSection() != GetCfgSectionName(eCfgSectionCSModule))
			{
				strError = "Param is not supported : ";
				strError << strKey;
				status = STATUS_CFG_PARAM_UNSUPPORTED_JITC_MODE;
				break;
			}
            status = STATUS_OK_WARNING;
		}
		else
		{
			//get new flag
			CCfgData *realParam = realSysConfig->GetCfgEntryByKey(strKey);
			if(pData->GetCfgType() != realParam->GetCfgType())
			{
				strError = "Type(Debug/User) does not match : ";
				strError << strKey;
				strError << '\n';
				strError << "Expected : ";
				strError << GetCfgTypeName(realParam->GetCfgType());
				strError << " - ";
				strError << "Got : ";
				strError << GetCfgTypeName(pData->GetCfgType());

				status = STATUS_CFG_PARAM_TYPE_DONT_MATCH;
				break;
			}

			if(section != realParam->GetSection())
			{
				strError = "Section does not match : ";
				strError << strKey;
				strError << '\n';
				strError << "Expected : ";
				strError << realParam->GetSection();
				strError << " - ";
				strError << "Got : ";
				strError << pData->GetSection();

				status = STATUS_CFG_PARAM_SECTION_DONT_MATCH;
				break;
			}

			//check if flag value has indeed changed
            if(status == STATUS_OK && realParam->GetData() != pData->GetData())
            {
                status = STATUS_OK_WARNING;
            }
		}

		iTer++;
	}

	TRACEINTO << (eLevelInfoNormal)   << "\nCSysConfigEma::TestValidity : "  << '\n'
              << "Status : "      << pProcess->GetStatusAsString(status) << '\n'
              << "description : " << strError.GetString();

	return status;
}


//check if data of strKey in EMA is different than in memory and set newData to value from EMA
STATUS CSysConfigEma::CheckIfKeyChanged(const std::string strKey, std::string &newData) const
{
	STATUS result = STATUS_FAIL;
	//get value from current systemCfg (from memory)
	CProcessBase *pProcess = CProcessBase::GetProcess();
	CSysConfigBase *memorySysConfig = pProcess->GetSysConfig();
	CCfgData *memoryData = memorySysConfig->GetCfgEntryByKey(strKey);

	//compare to value from EMA
	CSysMap::iterator iTer = GetMap()->begin();
	CSysMap::iterator iEnd = GetMap()->end();
	while(iTer != iEnd)
	{
		const std::string &emaStrKey 	= iTer->first;
		CCfgData 		*pData 	= iTer->second;
		const string &emaData = pData->GetData();
		if(strKey == emaStrKey)
		{
			//if key exists in memory map then compare
			if(memoryData)
			{
				newData = emaData;
				if(memoryData->GetData() != emaData)
				{
					result = STATUS_OK;
					break;
				}
			}
			//if new key then value changed
			else
			{
				newData = emaData;
				result = STATUS_OK;
				break;
			}
		}
		iTer++;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////
void CSysConfigEma::CompleteSectionName()
{
	CSysConfigBase *realSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	CSysMap 	   *pMap = GetMap();

	CSysMap::iterator iTer = pMap->begin();
	CSysMap::iterator iEnd = pMap->end();
	while(iTer != iEnd)
	{
		const std::string &key 	= iTer->first;
		CCfgData *pData 		= iTer->second;
		if(pData->GetSection().empty())
		{
			CCfgData *pRealData = realSysConfig->GetCfgEntryByKey(key);
			pData->SetSection(pRealData->GetSection());
		}

		iTer++;
	}
}

//////////////////////////////////////////////////////////////////////
DWORD CSysConfigEma::GetNumOfParams()const
{
	DWORD result = GetMap()->size();
	return result;
}
