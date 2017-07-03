#ifndef CONFIG_HELPER_H__
#define CONFIG_HELPER_H__

///////////////////////////////////////////////////////////////////////////
#include "SysConfig.h"

#include "ServiceConfigList.h"

#include "ProcessBase.h"

#include "TraceStream.h"

///////////////////////////////////////////////////////////////////////////
// *** deprecated: for backward compatibility only! ***
#define GetSystemCfgFlagInt GetSystemCfgFlag
#define GetSystemCfgFlagStr GetSystemCfgFlag

///////////////////////////////////////////////////////////////////////////
struct SystemCfgHelper
{
	static bool get(const CSysConfig* pCfg, const std::string& key, bool& value)
	{ return pCfg->GetBOOLDataByKey(key, value); }

	static bool get(const CSysConfig* pCfg, const std::string& key, BOOL& value)
	{ return pCfg->GetBOOLDataByKey(key, value); }

	static bool get(const CSysConfig* pCfg, const std::string& key, size_t& value)
	{ return pCfg->GetDWORDDataByKey(key, value); }

	static bool get(const CSysConfig* pCfg, const std::string& key, int& value)
	{ return pCfg->GetIntDataByKey(key, value); }

	static bool get(const CSysConfig* pCfg, const std::string& key, std::string& value)
	{ return pCfg->GetDataByKey(key, value); }
};

///////////////////////////////////////////////////////////////////////////
struct ServiceCfgHelper
{
	static bool get(const CServiceConfigList* pCfg, size_t svcId, const std::string& key, bool& value)
	{ return pCfg->GetBOOLDataByKey(svcId, key, (BOOL&)value); }

	static bool get(const CServiceConfigList* pCfg, size_t svcId, const std::string& key, BOOL& value)
	{ return pCfg->GetBOOLDataByKey(svcId, key, value); }

	static bool get(const CServiceConfigList* pCfg, size_t svcId, const std::string& key, size_t& value)
	{ return pCfg->GetDWORDDataByKey(svcId, key, value); }

	static bool get(const CServiceConfigList* pCfg, size_t svcId, const std::string& key, std::string& value)
	{ return pCfg->GetStrDataByKey(svcId, key, value); }
};

///////////////////////////////////////////////////////////////////////////
template<class T>
T GetSystemCfgFlag(const char* key)
{
	T value;

	if (!SystemCfgHelper::get(CProcessBase::GetProcess()->GetSysConfig(), std::string(key), value))
		FTRACEINTO << "Key:'" << key << "' - System flag is not defined, default value will be used";

	return value;
}

///////////////////////////////////////////////////////////////////////////
template<class T>
T GetSystemCfgFlag(size_t serviceId, const char* key)
{
	CServiceConfigList* pCfg = CProcessBase::GetProcess()->GetServiceConfigList();

	T value;

	if (!pCfg || !ServiceCfgHelper::get(pCfg, serviceId, std::string(key), value))
		FTRACEINTO << "Key:'" << key << "' - System flag is not defined, default value will be used";

	return value;
}

///////////////////////////////////////////////////////////////////////////
template<class T>
inline T GetSystemCfgFlagHex(const char* key)
{
	DWORD value = 0;

	if (!CProcessBase::GetProcess()->GetSysConfig()->GetHexDataByKey(std::string(key), value))
		FTRACEINTO << "Key:'" << key << "' - System flag is not defined, default value will be used";

	return value;
}

///////////////////////////////////////////////////////////////////////////
#endif // CONFIG_HELPER_H__
