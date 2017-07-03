#include "SysConfig.h"

#include <vector>
#include <stdio.h>
#include <errno.h>
#include <ostream>
#include <iomanip>
#include <algorithm>
#include <functional>

#include "CSKeys.h"
#include "psosxml.h"
#include "OsFileIF.h"
#include "TraceStream.h"
#include "StringsMaps.h"
#include "ProcessBase.h"
#include "SysConfigKeys.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"
#include "InitCommonStrings.h"

///////////////////////////////////////////////////////////////////////////
struct CCfgPair
{
	CCfgPair(const std::string& key, size_t counter)
		: Key(key)
		, Counter(counter)
	{}

	std::string Key;
	size_t Counter;
};

///////////////////////////////////////////////////////////////////////////
bool SysCfgPairGreater(const CCfgPair* elem1, const CCfgPair* elem2)
{
	return elem1->Counter > elem2->Counter;
}

///////////////////////////////////////////////////////////////////////////
struct CSysCfgPairSelector : public std::unary_function<CCfgPair*, bool>
{
	CSysCfgPairSelector(size_t limit)
		: Limit(limit)
	{}

	bool operator ()(const CCfgPair* checked) const
	{ return Limit > checked->Counter; }

	size_t Limit;
};

///////////////////////////////////////////////////////////////////////////
static void VectorPairOut(std::ostream& os, CCfgPairVector& v)
{
	CCfgPairVector::iterator it = v.begin();
	CCfgPairVector::iterator end = v.end();

	os << '\n';
	while (end != it)
	{
		CCfgPair* pair = *it;
		os << pair->Counter << "\t:\t" << pair->Key << '\n';

		++it;
	}

	os << std::endl;
}

///////////////////////////////////////////////////////////////////////////
static void VectorPairFree(CCfgPairVector& v)
{
	CCfgPairVector::iterator it = v.begin();
	CCfgPairVector::iterator end = v.end();

	while (end != it)
	{
		delete *it;
		*it = NULL;

		++it;
	}
}

///////////////////////////////////////////////////////////////////////////
CSysConfig::CSysConfig()
	: m_isReady(true)
{ Init(); }

///////////////////////////////////////////////////////////////////////////
CSysConfig::CSysConfig(const CSysConfig& obj)
	: CSysConfigBase(obj)
{}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::Init()
{
	InitMapByDefaults();

	// the &= does *not* short circuit, so it's Ok
	m_isReady &= LoadFromFile(eCfgParamDebug);
	m_isReady &= LoadFromFile(eCfgParamUser);
	m_isReady &= SetMSEnviromentDefaultsIfNeeded();
	m_isReady &= LoadFromFile(eCfgCustomParam);
}

///////////////////////////////////////////////////////////////////////////
bool CSysConfig::GetDataByKey(const std::string& key, std::string& data) const
{
	CCfgData* cfgData = NULL;
	bool res = ReadUpdate(key, cfgData);

	if (res)
		data = cfgData->GetData();

	return res;
}

bool CSysConfig::GetDWORDDataByKey(const std::string& key, DWORD& data) const
{
	CCfgData* cfgData;
	bool res = ReadUpdate(key, cfgData);

	if (!res)
		return res;

	data = atoi(cfgData->GetData().c_str());
	return true;
}

///////////////////////////////////////////////////////////////////////////
bool CSysConfig::GetIntDataByKey(const std::string& key, int& data) const
{
	CCfgData* cfgData;
	bool res = ReadUpdate(key, cfgData);

	if (!res)
		return res;

	data = atoi(cfgData->GetData().c_str());
	return true;
}

///////////////////////////////////////////////////////////////////////////
bool CSysConfig::GetHexDataByKey(const std::string& key, DWORD& data) const
{
	CCfgData* cfgData;
	bool res = ReadUpdate(key, cfgData);

	if (!res)
		return res;

	sscanf(cfgData->GetData().c_str(), "%x", &data);
	return true;
}

///////////////////////////////////////////////////////////////////////////
bool CSysConfig::GetBOOLDataByKey(const std::string& key, BOOL& data) const
{
	CCfgData* cfgData;
	bool res = ReadUpdate(key, cfgData);

	if (!res)
		return res;

	if (CFG_STR_YES == cfgData->GetData())
	{
		data = true;
		return true;
	}

	if (CFG_STR_NO == cfgData->GetData())
	{
		data = false;
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////
bool CSysConfig::GetBOOLDataByKey(const std::string& key, bool& data) const
{
	CCfgData* cfgData;
	bool res = ReadUpdate(key, cfgData);
	if (!res)
		return res;

	if (CFG_STR_YES == cfgData->GetData())
	{
		data = true;
		return true;
	}

	if (CFG_STR_NO == cfgData->GetData())
	{
		data = false;
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::FillCnt(std::ostream& os, int limit) const
{
	CCfgPairVector vect;
	vect.reserve(GetMap()->size());
	ConvertMapToPairVector(vect);

	CSysCfgPairSelector selector(limit);
	vect.erase(std::remove_if(vect.begin(), vect.end(), selector), vect.end());
	std::sort(vect.begin(), vect.end(), SysCfgPairGreater);

	VectorPairOut(os, vect);
	VectorPairFree(vect);
}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::DumpByKey(std::ostream& os, const string& key) const
{
	os.setf(std::ios::left, std::ios::adjustfield);
	os.setf(std::ios::showbase);

	bool isExist = IsParamExist(key);
	if (false == isExist) {
		os << "Param does not exist in cfg table: " << key;
		return;
	}

	CCfgData* cfgData = GetCfgEntryByKey(key);
	bool res = CCfgData::TestValidity(cfgData);
	if (false == res) {
		os << "Error in Cfg Table";
		return;
	}

	eCfgParamDataType cfgDataType = cfgData->GetCfgDataType();
	eCfgParamVisibilityType cfgVisibilityType =	cfgData->GetCfgParamVisibilityType();
	eCfgParamResponsibilityGroup cfgResponsibilityGroup =	cfgData->GetCfgParamResponsibilityGroup();

	os << std::setw(50) << key.c_str() << '\t';
	cfgData->Dump(os);
	os << '\t' << std::setw(15) << GetCfgParamVisibilityType(cfgVisibilityType);
	os << '\t' << std::setw(15) << GetCfgParamDataTypeName(cfgDataType);
	os << '\t' << std::setw(20) << GetCfgParamResponsibilityGroupName(cfgResponsibilityGroup) << STR_WIN_NEW_LINE;
}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::Dump(std::ostream& ostr) const
{
	ostr << endl;

	DumpByCfgType(ostr, eCfgParamUser, eCfgSectionCSModule);

	ostr << endl << endl;

	DumpByCfgType(ostr, eCfgParamUser, eCfgSectionMcmsUser);

	ostr << endl << endl;

	DumpByCfgType(ostr, eCfgParamDebug, eCfgSectionMcmsDebug);

	ostr << endl << endl;

	DumpByCfgType(ostr, eCfgCustomParam, eCfgSectionMcmsDebug);

	ostr << endl;
}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::DumpByCfgType(std::ostream& os, eCfgParamType type, eCfgSections section) const
{
	const char* pSectionName = GetCfgSectionName(section);

	os
		<< STR_WIN_NEW_LINE
		<< pSectionName << STR_WIN_NEW_LINE
		<< "------------------------" << STR_WIN_NEW_LINE
		<< std::setw(56) << "KEY"
		<< std::setw(24) << "DATA"
		<< std::setw(16) << "VISIBLE(?)"
		<< std::setw(16) << "TYPE"
		<< std::setw(21) << "RESPONSIBILITY" << STR_WIN_NEW_LINE
		<< std::setw(56) << "---"
		<< std::setw(24) << "----"
		<< std::setw(16) << "----------"
		<< std::setw(16) << "----"
		<< std::setw(21) << "--------------" << STR_WIN_NEW_LINE;

	CSysMap::iterator it = GetMap()->begin();
	CSysMap::iterator end = GetMap()->end();

	for ( ; end != it; ++it)
	{
		CCfgData* cfgData = it->second;
		bool res = CCfgData::TestValidity(cfgData);

		if (!res)
		{
			os << "Error in Cfg Table";
			return;
		}

		if (type != cfgData->GetCfgType())
			continue;

		const std::string& currentSection = cfgData->GetSection();

		if (currentSection != pSectionName)
			continue;

		const std::string& key = it->first;
		DumpByKey(os, key);
	}
}

bool CSysConfig::OverWriteParam(const std::string& key, const std::string& data, const std::string& section) const
{
	CCfgData* cfgData = NULL;
	bool res = ReadUpdate(key, cfgData);

	if (res)
	{
		if (!section.empty())
			cfgData->SetSection(section);

		cfgData->SetData(data);
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::PrintToMcuMngrTrace(const string& buff)
{
	CProcessBase* pProcess = CProcessBase::GetProcess();

	if (!pProcess) // no process no traces
		return;

	eProcessType procType = pProcess->GetProcessType();
	PASSERTMSG(eProcessMcuMngr == procType, buff.c_str());
}

///////////////////////////////////////////////////////////////////////////
bool CSysConfig::TakeCfgParam(const char* key, const char* data, const char* section, eCfgParamResponsibilityGroup curGroup)
{
	const eCfgParamType currentCfgParamType = GetCfgParamTypeState();

	if (!IsParamExist(key))
	{
		if (0 == strcmp(section, GetCfgSectionName(eCfgSectionCSModule)))
			AddParamNotVisible(section, key, data, currentCfgParamType, ONE_LINE_BUFFER_LENGTH, curGroup, eCfgParamDataString);

		else
		{
			string buff = "file ";
			buff += GetFileName(currentCfgParamType);
			buff += " contains a new param ";
			buff += key;
			buff += ". it must exist in defaults ";
			PrintToMcuMngrTrace(buff);
			return false;
		}
	}
	else
	{
		CCfgData* cfgData = NULL;
		ReadUpdate(key, cfgData);

		if (cfgData->GetCfgType() != currentCfgParamType)
		{
			string buff = "file ";
			buff += GetFileName(currentCfgParamType);
			buff += " contains an illegal param(type does not match) ";
			buff += key;
			buff += " : ";
			buff += data;
			PrintToMcuMngrTrace(buff);
			return false;
		}

		OverWriteParam(key, data, section);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::InitMapByDefaults() const
{
	InitUserParams();
	InitUserJITCParams();
	InitCSModuleParams();
	InitDebugParams();
	InitCustomParams();
}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::PrintAllParams() const
{
	COstrStream os;
	CSysMap* map = GetMap();
	CSysMap::iterator it = map->begin();

	while (it != map->end())
	{
		CCfgData* cfgData = (CCfgData*) it->second;
		os
			<< cfgData->GetKey() << "|" << cfgData->GetData() << "|"
			<< cfgData->GetSection() << "|"
			<< cfgData->GetCfgParamVisibilityType() << '\n';

		++it;
	}

	TRACEINTO << "\n" << os.str() << '\n';
}

///////////////////////////////////////////////////////////////////////////
bool CSysConfig::IsUnderJITCState() const
{
	static bool ret;
	static bool first_time = true;

	// Static return value stays false in case of the error
	if (!first_time)
		return ret;

	first_time = false;

	static std::string fname = MCU_MCMS_DIR+"/JITC_MODE.txt";
	if (!IsFileExists(fname))
		return false;

	FILE* fp = fopen(fname.c_str(), "r");
	PASSERTSTREAM_AND_RETURN_VALUE(NULL == fp,
		"fopen: " << fname << ": " << strerror(errno) << " (" << errno << ")",
		false);

	char buf[4];
	fgets(buf, ARRAYSIZE(buf), fp);
	fclose(fp);

	ret = (0 == strncmp(buf, "YES", ARRAYSIZE(buf)));
	return ret;
}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::InitUserParams() const
{
	const char* sec = GetCfgSectionName(eCfgSectionMcmsUser);
	const eCfgParamType type = eCfgParamUser;

	const eProductFamily productFamily = CProcessBase::GetProcess()->GetProductFamily();
	const eProductType productType = CProcessBase::GetProcess()->GetProductType();

	AddParamNotVisible(sec, "ENABLE_WHITE_LIST", "YES", type,_YES_NO, false, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean,eProcessMcuMngr);

	AddParamVisible(sec, "DISABLE_IPMC_USAGE", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	AddParamNotVisible(sec, "UNIT_DB_SYNC", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	AddParamNotVisible(sec, "802_1X_SKIP_CERTIFICATE_VALIDATION", "YES", type, _YES_NO,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "802_1X_CRL_MODE", "DISABLED", type, CRL_MODE_802_1X_ENUM,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataEnum);
	AddParamNotVisible(sec, "802_1X_CERTIFICATE_MODE", "ONE_CERTIFICATE", type, CERTIFICATE_MODE_802_1X_ENUM,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataEnum);
	AddParamNotVisible(sec, "802_FIPS_MODE", "NO", type, _YES_NO,false,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean,eProcessMcuMngr);

	AddParamNotVisible(sec, CFG_KEY_IP_RESPONSE_ECHO, "YES", type, _YES_NO,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	AddParamNotVisible(sec, CFG_KEY_DUPLICATE_IP_DETECTION, "YES", type,_YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);

	// default number in Linux 2.2 is 15
	AddParamNotVisible(sec, CFG_KEY_TCP_RETRANSMISSION_TIMEOUT, "15", type, _0_TO_30_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);

	//In order to be compatible with 7.0.2C added by huiyu.
	AddParamVisible(sec, "ALLOW_NON_ENCRYPT_PARTY_IN_ENCRYPT_CONF", "NO", type,	_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataBoolean);
	AddParamInFileNonVisible(sec, "JITC_MODE", "NO", type, _YES_NO,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamVisible(sec, "IVR_ROLL_CALL_USE_TONES_INSTEAD_OF_VOICE", "NO",	type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	AddParamVisible(sec, "ALLOW_NON_ENCRYPT_RECORDING_LINK_IN_ENCRYPT_CONF","NO", type, _YES_NO, false,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,	eProcessConfParty);
	AddParamNotVisible(	sec,"FORCE_ENCRYPTION_FOR_UNDEFINED_PARTICIPANT_IN_WHEN_AVAILABLE_MODE","NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G728_ISDN", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G728_IP", "NO", type, _YES_NO,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// external db
	AddParamNotVisible(sec, "ENABLE_EXTERNAL_DB_ACCESS", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "EXTERNAL_DB_DIRECTORY", "", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataString);
	AddParamNotVisible(sec, "EXTERNAL_DB_IP", "", type, ONE_LINE_BUFFER_LENGTH,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataString);
	AddParamNotVisible(sec, "EXTERNAL_DB_LOGIN", "POLYCOM", type,	ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataString);
	AddParamNotVisible(sec, CFG_KEY_ENABLE_EXTERNAL_DB_CONNECTIONLESS, "NO", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	AddParamNotVisible(sec, "EXTERNAL_DB_PORT", "5005", type, _0_TO_WORD,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "EXTERNAL_DB_PASSWORD", "POLYCOM", type,	ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataString);
	AddParamNotVisible(sec, "RMX_MANAGEMENT_SECURITY_PROTOCOL", "TLSV1",	type, TLSV_ENUM, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataEnum);
	AddParamNotVisible(sec, "ENABLE_ACTIVE_ALARM_NO_LAN", "YES", type, _YES_NO,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamVisible(sec, "NUMERIC_CONF_ID_LEN", "5", type, _0_TO_WORD,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamVisible(sec, "NUMERIC_CONF_ID_MIN_LEN", "4", type, _0_TO_WORD,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamVisible(sec, "NUMERIC_CONF_ID_MAX_LEN", "16", type, _0_TO_WORD,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// VNFE-2507 the display name flag default value changed to ""
	AddParamVisible(sec, "MCU_DISPLAY_NAME", "", type, ONE_LINE_BUFFER_LENGTH,	false, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataString, eProcessTypeInvalid, true);
	//VNGFE-8231 - seperate between mcu display name and product-id for GK registration.
	AddParamNotVisible(sec, "MCU_H323_PRODUCT_ID", "", type, ONE_LINE_BUFFER_LENGTH,	false, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataString, eProcessTypeInvalid, true);

	AddParamVisible(sec, "ENABLE_AUTO_EXTENSION", "YES", type, _YES_NO, false,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,	eProcessConfParty);
	//AddParamVisible(sec, "TERMINATE_CONF_AFTER_CHAIR_DROPPED", "NO", type,	_YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataBoolean, eProcessConfParty);
	AddParamVisible(sec, "HD_THRESHOLD_BITRATE", "768", type, HD_RATES_ENUM,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);

	// MS RTV
	AddParamNotVisible(sec, "MAX_ALLOWED_RTV_HD_FRAME_RATE", "0", type,	_0_TO_30_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataNumber);
	AddParamNotVisible(sec, "MAX_RTV_RESOLUTION", "AUTO", type,	MAX_RTV_RESOLUTION_ENUM, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, "MS_CLIENT_AUDIO_CODEC", "AUTO", type,	MS_CLIENT_AUDIO_CODEC_ENUM,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);
	AddParamNotVisible(sec, "FORCE_AUDIO_CODEC_FOR_MS_SINGLE_CORE", "G711A",	type, MS_CLIENT_AUDIO_CODEC__FOR_SINGLE_CORE_ENUM,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);
	AddParamNotVisible(sec, "ENCODE_RTV_B_FRAME", "NEVER", type,	ENCODE_RTV_B_FRAME_ENUM, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, "MS_CAC_AUDIO_MIN_BR", "30", type,	_0_TO_384_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataNumber);
	AddParamNotVisible(sec, "MS_CAC_VIDEO_MIN_BR", "40", type,	_0_TO_384_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataNumber);
	AddParamNotVisible(sec, "ENABLE_MS_FEC", "AUTO", type, MS_FEC_ENUM,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);
	AddParamNotVisible(sec, "ENABLE_SIP_LYNC2013_FEC", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);  //LYNC2013_FEC_RED
	AddParamNotVisible(sec, "ENABLE_SIP_LYNC2013_RED", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);  //LYNC2013_FEC_RED

	//debug - FEC
	AddParamNotVisible(sec, "RTCP_VSR_QRH_INDEX_FOR_FEC", "0", type, _0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);
	
	AddParamNotVisible(sec, "LYNC_AVMCU_1080p30_ENCODE_RESOLUTION", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean); //LYNC_AVMCU_1080p30
	AddParamNotVisible(sec, "ALLOCATE_HD_ENCODER_LYNC_AV_MCU_CASCADE_RESOURCE_OPTIMIZE", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean); //LYNC_AVMCU_1080p30
	AddParamNotVisible(sec, "LYNC2013_ENABLE_G722Stereo128k", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean); //LYNC_G722Stereo128k

	AddParamNotVisible(sec, "ENABLE_LYNC_RTCP_INTRA", "NO", type, _YES_NO,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// debug flags for bridge-11167
	AddParamNotVisible(sec, "ENABLE_LYNC_RTCP_INTRA_AVMCU", "YES", type, _YES_NO, false,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, "SEND_PREFERENCE_REQUEST_TO_AVMCU2010", "YES", type, _YES_NO, false,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);

	AddParamNotVisible(sec, "BLOCK_NEW_LYNC2013_FUNCTIONALITY", "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty, true);
	AddParamNotVisible(sec, "MAX_MS_SVC_RESOLUTION", "AUTO", type, MAX_MSSVC_RESOLUTION_ENUM, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, CFG_KEY_IGNORE_ICE_FOR_LYNC2013, "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty, true);

	AddParamNotVisible(sec, "RTV_MAX_BIT_RATE_FOR_FORCE_CIF_PARTICIPANT", "192", type, _0_TO_DWORD, false,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty, true);

	AddParamNotVisible(sec, "VSW_CIF_BL_THRESHOLD_BITRATE", "64", type,	_64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, "VSW_SD_BL_THRESHOLD_BITRATE", "256", type,	_64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, "VSW_HD_720p30_BL_THRESHOLD_BITRATE", "832", type,	_64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, "VSW_HD_720p50_60_BL_THRESHOLD_BITRATE", "1232",	type, _64_TO_8192_DECIMAL,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);
	AddParamNotVisible(sec, "VSW_HD_1080p_BL_THRESHOLD_BITRATE", "1728", type,	_64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, "VSW_HD_1080p60_BL_THRESHOLD_BITRATE", "3072", type, _64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);
	AddParamNotVisible(sec, "VSW_CIF_HP_THRESHOLD_BITRATE", "64", type,	_64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, "VSW_SD_HP_THRESHOLD_BITRATE", "128", type,	_64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, CFG_KEY_VSW_HD_720p30_HP_THRESHOLD_BITRATE, "512", type,	_64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, CFG_KEY_VSW_HD_720p50_60_HP_THRESHOLD_BITRATE, "832",	type, _64_TO_8192_DECIMAL,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);
	AddParamNotVisible(sec, CFG_KEY_VSW_HD_1080p_HP_THRESHOLD_BITRATE, "1024", type,	_64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataEnum);
	AddParamNotVisible(sec, CFG_KEY_VSW_HD_1080p60_HP_THRESHOLD_BITRATE, "1728", type, _64_TO_8192_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);
	AddParamNotVisible(sec, "DELAYED_IVR_FOR_SPECIFIC_EP", "0", type,	_0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataNumber);
	AddParamNotVisible(sec, "DELAYED_AUDIO_IVR_FOR_SPECIFIC_EP", "5", type,	_0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataNumber);
	AddParamVisible(sec, "RV_GW_VIDEO_RATE_REDUCTION_PERCENTAGE", "10", type,	_0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataNumber, eProcessConfParty);
	AddParamVisible(sec, "ENABLE_CASCADED_LINK_TO_JOIN_WITHOUT_PASSWORD", "NO",type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);

	// MAX_CP_RESOLUTION Flag
	// For MPM Based system: CIF, SD15, SD30, HD
	// For MPM+ Based system: CIF, SD30, HD720,HD1080
	AddParamVisible(sec, "MAX_CP_RESOLUTION", "HD1080", type,CONF_CP_RESOLUTION_ENUM, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum);
	AddParamNotVisible(sec, "MAX_ISDN_BITRATE_MPMX", "768", type,TRANSFER_RATE_ENUM, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum);
	AddParamVisible(sec, "ISDN_RESOURCE_POLICY", "LOAD_BALANCE", type,ISDN_RESOURCE_POLICY_ENUM, eCfgParamResponsibilityGroup_Illegal,eCfgParamDataEnum);
	AddParamVisible(sec, "ISDN_COUNTRY_CODE", "COUNTRY_NIL", type,COUNTRY_CODE_EXTERNAL_ENUM,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataEnum);
	AddParamVisible(sec, "ISDN_IDLE_CODE_T1", "0x13", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);
	AddParamVisible(sec, "ISDN_IDLE_CODE_E1", "0x54", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);
	AddParamVisible(sec, "ISDN_NUM_OF_DIGITS", "9", type, _0_TO_DWORD,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "ISDN_CLOCK", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "CONF_END_ALERT_TONE", "5", type, _0_TO_10_DECIMAL,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "ENCRYPTION", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SITE_NAMES_ALWAYS_ON", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty);
	AddParamVisible(sec, "HIDE_SITE_NAMES", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty);
	AddParamNotVisible(sec, "CP_REGARD_TO_INCOMING_SETUP_RATE", "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H323_ENABLE_CONFERENCE_DIALIN_IDENTIFY", "YES",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIP_IMS", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIP_PSI_SOURCE", "", type, ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
	AddParamNotVisible(sec, "SIP_PSI_URI", "", type, ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
	AddParamNotVisible(sec, "ENABLE_IP_REDIAL_FOR_NOT_FINISH_CAPS_EXCHANGE", "YES",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "NUMBER_OF_REDIAL", "3", type, _0_TO_WORD,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "ENABLE_ISDN_REDIAL", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ITP_CERTIFICATION", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamVisible(sec, "MIN_TIP_COMPATIBILITY_LINE_RATE", "1024", type,_0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec,"OFFER_SHORT_AUDIO_CODEC_LIST_AFTER_AUDIO_CALL_OFFERLESS_INVITE", "NO",type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty); //N.A. VNGFE-7854


	//WebRTC //N.A. DEBUG VP8
	AddParamNotVisible(sec,"ENABLE_WEBRTC_SUPPORT", "NO",type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty); //N.A. - set to no reset at the moment!





//	AddParamVisible(sec, "MAX_TIP_COMPATIBILITY_LINE_RATE", "2500", type,
//			_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,
//			eCfgParamDataNumber);

	// Hot backup:
	AddParamNotVisible(sec, "HOT_BACKUP_REDIAL_INTERVAL_IN_SECONDS", "10",type, _2_TO_120_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "HOT_BACKUP_NUMBER_OF_REDIAL_ATTEMPTS", "11", type,_3_TO_50_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "HOT_BACKUP_FAILURE_DETECTION_IN_SECONDS", "10",type, _6_TO_240_DECIMAL,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	//AddParamNotVisible(sec, "QOS_IP_AUDIO", "0x88", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);
	//AddParamNotVisible(sec, "QOS_IP_VIDEO", "0x88", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);
	AddParamNotVisible(sec, "QOS_IP_AUDIO", "0x31", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);
	AddParamNotVisible(sec, "QOS_IP_VIDEO", "0x31", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);


	//QOS_MANAGEMENT
	AddParamNotVisible(sec, "QOS_MANAGEMENT_NETWORK", "0x10", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataString, eProcessMcuMngr);
	// ENABLE/DISABLE H239+EPC
	AddParamNotVisible(sec, "ENABLE_H239", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ENABLE_EPC", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamVisible(sec, "RRQ_WITHOUT_GRQ", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "GAIN_AUDIO_FOR_DTMF_FORWARDING", "4", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "SILNECE_DURATION_FOR_DTMF_FORWARDING", "20", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "ENABLE_CISCO_GK", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	AddParamNotVisible(sec, "REMOVE_H323_LPR_CAP_TO_NON_POLYCOM_VENDOR", "NO",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "REMOVE_H323_EPC_CAP_TO_NON_POLYCOM_VENDOR", "NO",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec,	"REMOVE_H323_HIGH_PROFILE_CAP_TO_NON_POLYCOM_VENDOR", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec,	"REMOVE_H323_HIGH_QUALITY_AUDIO_CAP_TO_NON_POLYCOM_VENDOR", "NO",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// if 'POLYCOM' user exists in Users list, then an Alert is(/not) produced
	AddParamNotVisible(sec, "DEFAULT_USER_ALERT", "YES", type, _YES_NO,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	// Flag to free/not free video resources in case of undefined call that has only audio capabilities.
	// we can use the option not to free video resources in case of Avaya or Cisco
	AddParamNotVisible(sec, "H323_FREE_VIDEO_RESOURCES", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H323_OLC_ACK_DYNAMIC_PAYLOAD_REPLACEMENT", "NO",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ENABLE_SIRENLPR", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty);
	AddParamNotVisible(sec, "ENABLE_SIRENLPR_SIP_ENCRYPTION", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	AddParamNotVisible(sec, "G722_1", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G722_1_16K", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	/* The flag set SIP SDP 264 cap packetization-mode param to 0/1 depends on the value of the flag */
	AddParamNotVisible(sec, "SIP_H264_PACKETIZATION_MODE", "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	/* RTCP system flags */
	AddParamNotVisible(sec, "RTCP_FLOW_CONTROL_TMMBR_INTERVAL", "180", type,_5_TO_999_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "RTCP_FLOW_CONTROL_TMMBR_ENABLE", "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "RTCP_FIR_ENABLE", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "RTCP_PLI_ENABLE", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	/* field 7091 Send the RTCP according to the remote caps */
	AddParamNotVisible(sec, "RTCP_SEND_BY_RM_CAPS", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "RTCP_FLOWCONTROL_SEND_BY_RM_CAPS", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);


	/* DMA tags - X-Plcm-Require header system flags - in case flag is set, RMX will ignore if the Tags doesn't appear in the header */
	AddParamNotVisible(sec, "X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "X_PLCM_REQUIRE_FECC_ENABLE_TAG", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);


	// Flag to free/not free video resources in case of undefined call that has
	// only audio capabilities. we can use the option not to free video
	// resources in case of SIP calls
	AddParamNotVisible(sec, "SIP_FREE_VIDEO_RESOURCES", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIP_CHANGE_AUDIO_CAPS_LIST_ACCORDING_TO_VENDOR","YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIP_ALWAYS_USE_PARTIAL_AUDIO_CAPS", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ENABLE_CONF_HIGH_CALL_RATE", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// ENABLE/DISABLE FECC.
	AddParamNotVisible(sec, "FECC", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "FECC_RV", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "FECC_ANNEXQ", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// Enable/Disable Fecc in sip call
	AddParamNotVisible(sec, "SIP_ENABLE_FECC", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIP_FLIP_INTRA_ENV", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIP_FLIP_INTRA_EP", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIP_FAST_UPDATE_INTERVAL_ENV", "0", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "SIP_FAST_UPDATE_INTERVAL_EP", "0", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "SIP_REGISTER_ONLY_ONCE", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "FADE_IN_FADE_OUT", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SITE_NAME_TRANSPARENCY", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "MTU_SIZE", "1120", type, _0_TO_WORD,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	//AddParamNotVisible(sec, "TIP_MTU_SIZE", "1400", type, _0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "MINIMUM_FRAME_RATE_TRESHOLD_FOR_SD", "15", type,_0_TO_30_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,
			eCfgParamDataNumber);
	AddParamNotVisible(sec, "MTU_SIZE_DURING_LPR", "1440", type, _0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "MEDIA_NIC_MTU_SIZE", "1500", type, _0_TO_DWORD,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "SKIP_PROMPT", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	//  AddParamVisible(sec, "H263_ANNEX_T", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H264_CONTENT_MBPS_VALUE", "44", type, _0_TO_DWORD,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_SYSTEM_NORMAL_WITH_SINGLE_CLOCK_SOURCE,"NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H239_FORCE_CAPABILITIES", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H323_FORCE_25FPS_ON_HDX", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamVisible(sec, "FORCE_CIF_PORT_ALLOCATION","iPower;PolycomVVX;VVX 1500", type, ONE_LINE_BUFFER_LENGTH, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString,eProcessConfParty);
	AddParamVisible(sec, "FORCE_CIF_PORT_ALLOCATION_MPMX","iPower;PolycomVVX;VVX 1500", type, ONE_LINE_BUFFER_LENGTH, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString,	eProcessConfParty);
	//802.1x
	AddParamNotVisible(sec, "CSR_SANS_PREFIX", "otherName:1.3.6.1.4.1.311.20.2.3;UTF8:", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);

	AddParamNotVisible(sec, "REDUCE_RATE_MPMX", "NONE", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);
	AddParamNotVisible(sec, "DO_NOT_LIMIT_TO_HD720_MPMX", "ANY", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);
	AddParamNotVisible(sec, "USE_SMART_SWITCH_PRODUCT_NAME","HDX 800;HDX 700;HDX 600;HDX 400", type, ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);

	// params for cascade between RMX & MGC in HD (1920) conf
	// maxsimum one of them may be set to YES
	AddParamNotVisible(sec, "MIX_LINK_ENVIRONMENT", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "IP_LINK_ENVIRONMENT", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "RECORDING_LINK_DISCONNECTION_TIMER_DURATION","10", type, _0_TO_30_DECIMAL,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "RECORDING_LINK_START_RECORDING_TIMER_DURATION","10", type, _0_TO_30_DECIMAL,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "MAXIMUM_RECORDING_LINKS", "20", type,_0_TO_100_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	//  AddParamVisible(sec, "IVR_MESSAGE_VOLUME", "2", type, _0_TO_10_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	//  AddParamVisible(sec, "IVR_MUSIC_VOLUME", "2", type, _0_TO_10_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	//  AddParamVisible(sec, "IVR_ROLL_CALL_VOLUME", "4", type, _0_TO_10_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamVisible(sec, "IVR_MESSAGE_VOLUME", "2", type, _0_TO_10_DECIMAL,false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamVisible(sec, "IVR_MUSIC_VOLUME", "2", type, _0_TO_10_DECIMAL,false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamVisible(sec, "IVR_ROLL_CALL_VOLUME", "4", type, _0_TO_10_DECIMAL,false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamVisible(sec, "FORCE_SYSTEM_BROADCAST_VOLUME", "NO", type, _YES_NO,false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);
	AddParamVisible(sec, "FORCE_SYSTEM_LISTENING_VOLUME", "NO", type, _YES_NO,false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);
	AddParamVisible(sec, "SYSTEM_BROADCAST_VOLUME", "5", type, _0_TO_DWORD, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);
	AddParamVisible(sec, "SYSTEM_LISTENING_VOLUME", "5", type, _0_TO_DWORD, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, "SUPPORT_VSW_FLOW_CONTROL", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "VSW_RATE_TOLERANCE_PERECENT", "0", type,_0_TO_75_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_0", "CP_LAYOUT_1X1", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_1", "CP_LAYOUT_1X1", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_2", "CP_LAYOUT_1X1", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_3", "CP_LAYOUT_1x2", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_4", "CP_LAYOUT_2X2", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_5", "CP_LAYOUT_2X2", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_6", "CP_LAYOUT_1P5", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_7", "CP_LAYOUT_1P5", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_8", "CP_LAYOUT_1P7", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_9", "CP_LAYOUT_1P7", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_10", "CP_LAYOUT_2P8", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_11", "CP_LAYOUT_2P8", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "PREDEFINED_AUTO_LAYOUT_12", "CP_LAYOUT_1P12", type, FULL_LAYOUT_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);

	AddParamNotVisible(sec, "FORCE_1X1_LAYOUT_ON_CASCADED_LINK_CONNECTION","YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ENABLE_TEXTUAL_CONFERENCE_STATUS", "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// COP intra suppression
	AddParamVisible(sec, "EVENT_MODE_MAX_INTRA_REQUESTS_PER_INTERVAL", "3",type, _0_TO_DWORD, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
	AddParamVisible(sec, "EVENT_MODE_INTRA_SUPPRESSION_DURATION_IN_SECONDS","10", type, _0_TO_DWORD, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
	AddParamVisible(sec, "VSW_MAX_INTRA_REQUESTS_PER_INTERVAL", "7", type,_0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamVisible(sec, "VSW_INTRA_SUPPRESSION_DURATION_IN_SECONDS", "10",type, _0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamVisible(sec, "MAX_INTRA_REQUESTS_PER_INTERVAL_CONTENT", "3", type,_0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamVisible(sec, "MAX_INTRA_SUPPRESSION_DURATION_IN_SECONDS_CONTENT","10", type, _0_TO_DWORD, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
	AddParamVisible(sec, "CONTENT_SLAVE_LINKS_INTRA_SUPPRESSION_IN_SECONDS","30", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataNumber);
	AddParamVisible(sec, "EVENT_MODE_HANDLE_NOISY_PARTICIPANT", "NO", type,	_YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);
	AddParamVisible(sec, "COP_ENCODER_IGNORE_INTRA_DURATION_IN_SECONDS", "10",type, _0_TO_DWORD, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
	AddParamVisible(sec, "CONTENT_SPEAKER_INTRA_SUPPRESSION_IN_SECONDS", "3",type, _0_TO_DWORD, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
	AddParamVisible(sec, "LEVEL_RATE_REDUCTION_PERCENTAGE", "10", type,_0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamVisible(sec, "SITE_NAMES_LOCATION", "DOWN_CENTER", type,SITE_NAMES_LOCATION_ENUM, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_PCM_FECC, "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// not in use for V4.6
	AddParamNotVisible(sec, "PCM_LANGUAGE", "ENGLISH", type, PCM_LANGUAGE_ENUM,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);
	AddParamVisible(sec, CFG_KEY_INTERNAL_SCHEDULER, "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaResource, eCfgParamDataBoolean);
	AddParamVisible(sec, CFG_KEY_CHANGE_AD_HOC_CONF_DURATION, "60", type,_CHANGE_AD_HOC_CONF_DURATION, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum,eProcessConfParty);
	AddParamNotVisible(sec, "BONDING_DIALING_METHOD", "SEQUENTIAL", type,BONDING_DIALING_METHOD_ENUM,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum);

	// "BY_TIMERS" = wait BONDING_CHANNEL_DELAY between channels and BONDING_GROUP_DELAY between groups
	// "SEQUENTIAL" = dial first BONDING_NUM_CHANNELS_IN_GROUP (wait BONDING_CHANNEL_DELAY between channels)
	// and after every channel connection dial next channel (until all channel dialed)
	AddParamNotVisible(sec, "BONDING_NUM_CHANNELS_IN_GROUP", "6", type,_1_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamVisible(sec, "BONDING_CHANNEL_DELAY", "50", type, _1_TO_DWORD,false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, "BONDING_GROUP_DELAY", "500", type, _1_TO_DWORD,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	AddParamNotVisible(sec, "ISDN_AUDIO_G7221_C_ADVANCED", "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G711_ALAW", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G711_ULAW", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G722_64k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G722_56k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G722_48k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G722_1_32k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G722_1_24k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G722_1_C_48k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G722_1_C_32k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_G722_1_C_24k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_SIREN_14_48k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_SIREN_14_32k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_SIREN_14_24k", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ISDN_ZERO_MODE", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "DISCONNECT_H320_PARTY_SYNC_LOSS_TIMER_SECONDS","30", type, _0_TO_3600_DECIMAL,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// THIS FLAGS SHOULD DETERMINE THE GAP BETWEEN DIAL OUT PARTIES (TIME IN MILISECOND)
	AddParamNotVisible(sec, "DELAY_BETWEEN_DIAL_OUT_PARTY", "500", type,_0_TO_100000_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
	AddParamNotVisible(sec, "DELAY_BETWEEN_AUDIO_DIAL_OUT_PARTY", "200", type,_0_TO_100000_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
	AddParamNotVisible(sec, "DELAY_BETWEEN_H320_DIAL_OUT_PARTY", "1000", type,_0_TO_100000_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
	AddParamNotVisible(sec, "AUDIO_DECODER_COMPRESSED_DELAY", "0", type,_0_TO_3000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "MAX_TRANSMITED_FRAME_RATE_FROM_DECODER", "35",type, _0_TO_99_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamVisible(sec, "IGNORE_AIM", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ALLOW_IN_VIDEO_FROM_IPVCR", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// Microsoft Master falg (set defaults to a list of flags regarding MS enviroment
	AddParamVisible(sec, "MS_ENVIRONMENT", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// sip register delay
	AddParamNotVisible(sec, "SIP_REGISTER_DELAY_MILLI_SEC", "300", type,_0_TO_100000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamVisible(sec, CFG_KEY_MULTIPLE_SERVICES, "NO", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamVisible(sec, CFG_KEY_RMX2000_RTM_LAN, "NO", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_LAN_REDUNDANCY, CFG_STR_NO, type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "DISABLE_GW_OVERLAY_INDICATION", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "GW_OVERLAY_TYPE", "7", type, _0_TO_99_DECIMAL,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "GW_OVERLAY_TIMEOUT", "100", type,_0_TO_100000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamVisible(sec, "LEGACY_EP_CONTENT_DEFAULT_LAYOUT","CP_LAYOUT_1P4VER", type, FULL_LAYOUT_ENUM, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum,eProcessConfParty);
	AddParamVisible(sec, "ISDN_LEGACY_EP_CLOSE_CONTENT_FORCE_H263", "NO", type,_YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);
	AddParamVisible(sec, "RESTRICT_CONTENT_BROADCAST_TO_LECTURER", "YES", type,_YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, "FORCE_STATIC_MB_ENCODING", "MXP;TANDBERG/", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);
	AddParamNotVisible(sec, "MXP_FORCE_HD_FR", "10", type, _0_TO_20_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	//example for SIP_CONTACT_OVERRIDE_STR: rmx24022.reg10.ent@reg10.ent;opaque=srvr:videorouting:Tggd9yw0FFyNjnRqleNLwQAA;gruu
	AddParamNotVisible(sec, "SIP_CONTACT_OVERRIDE_STR","", type, ONE_LINE_BUFFER_LENGTH, true,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString,eProcessConfParty);
	AddParamNotVisible(sec, "MS_ICE_VERSION", "0x00000000", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);
	/* Override the SVC license bit to improve Lync users capacity for MPMX cards - In case Flag = NO, MPMX cards will not accept SVC calls */
	AddParamNotVisible(sec, CFG_KEY_PREFER_SVC_OVER_LYNC_CAPACITY, "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);


	// list of products names for EPs that we will force the encoder to use
	// static MB ("LifeSize Room;Tandberg MXP")

	// Is regard Sip bad characters for conference names:
	AddParamNotVisible(sec, "CHECK_SIP_CHARS_IN_CONF_NAME", "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ENABLE_CLOSED_CAPTION", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ENABLE_SIP_LPR", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "REDUCE_CAPS_FOR_REDCOM_SIP", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
    AddParamNotVisible(sec, "SIP_FORMAT_GW_HEADERS_FOR_REDCOM", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamVisible(sec, "CPU_TCP_KEEP_ALIVE_TIME_SECONDS", "7200", type,_600_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
	AddParamVisible(sec, "CPU_TCP_KEEP_INTERVAL_SECONDS", "75", type,_10_720_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
	AddParamNotVisible(sec, "SEND_WIDE_RES_TO_ISDN", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SEND_WIDE_RES_TO_IP", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "DISABLE_WIDE_RES_TO_SIP_DIAL_OUT", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "FORCE_RESOLUTION", "", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);
	AddParamVisible(sec, "PAL_NTSC_VIDEO_OUTPUT", "AUTO", type, FPS_MODE_ENUM,false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum, eProcessConfParty);
	AddParamNotVisible(sec, "HIDE_CONFERENCE_PASSWORD", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	// Default Adhoc calls
	AddParamNotVisible(sec, CFG_KEY_ENABLE_DEFAULT_ADHOC_CALLS, "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// FIPS flags
	// Possible values - "INACTIVE", "FAIL_DETERMINISTIC_TEST"
	AddParamNotVisible(sec, "FIPS140_SIMULATE_CARD_PROCESS_ERROR", "INACTIVE",type, FIPS140_SIMULATE_CARD_PROCESS_ENUM,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataEnum);

	// Possible values - "INACTIVE", "FAIL_DETERMINISTIC_TEST", "FAIL_POOL_GENERATION_TEST"
	AddParamNotVisible(sec, "FIPS140_SIMULATE_ENCRYPTION_PROCESS_ERROR","INACTIVE", type, FIPS140_SIMULATE_ENCRYPTION_PROCESS_ENUM,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataEnum);

	// Possible values - "INACTIVE", "FAIL_CIPHER_TEST"
	AddParamNotVisible(sec, "FIPS140_SIMULATE_CONFPARTY_PROCESS_ERROR","INACTIVE", type, FIPS140_SIMULATE_CONFPARTY_PROCESS_ENUM,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataEnum);
	AddParamVisible(sec, "STAR_DELIMITER_ALLOWED", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_PERMANENT_CP_CONF_RESOURCE_ALLOC, "YES",type, _YES_NO, eCfgParamResponsibilityGroup_McaResource,eCfgParamDataBoolean);

	// New Busy Indication flag
	AddParamNotVisible(sec, "SEND_SIP_BUSY_UPONRESOURCE_THRESHOLD", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "PORT_GAUGE_ALARM", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessResource);

	{
		// System: CPU load average and thresholds
		const char* threshold = (eProductFamilySoftMcu != productFamily) ? "480" : "180";

		AddParamNotVisible(sec, CFG_KEY_LOGGER_SYSTEM_LOAD_AVERAGE_THRESHOLD,threshold, type, _5_TO_999_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
		AddParamNotVisible(sec, CFG_KEY_LOAD_AVERAGE_ALARM_LEVEL, threshold, type, _5_TO_999_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	}
	AddParamNotVisible(sec, CFG_KEY_MFA_SYSTEM_LOAD_AVERAGE_THRESHOLD,"500", type, _5_TO_999_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_CM_HIGH_CPU_USAGE_INTERVAL_IN_MINUTES, "5", type,_1_TO_15_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_LOAD_AVERAGE_MINUTES, "1", type,_1_TO_15_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);

	 AddParamNotVisible(sec, "UNMUTE_MEDIA_AFTER_RESUME_IN_SEC", "1", type, _0_TO_100_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// Possible values:
	// "t"   eLevelTrace
	// "d"   eLevelDebug
	// "n"    eLevelInfoNormal
	// "i"    eLevelInfoHigh
	// "w"    eLevelWarn
	// "e"   eLevelError
	// "f"   eLevelFatal
	// "o"     eLevelOff
	AddParamNotVisible(sec, CFG_KEY_LOGGER_MAX_TRACE_LEVEL, "n", type,ONE_LINE_BUFFER_LENGTH, false,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataString,eProcessTypeInvalid, true);

	if((eProductTypeSoftMCUMfw == productType) || (eProductTypeSoftMCU == productType) || (eProductTypeGesher == productType)
		|| (eProductTypeEdgeAxis == productType) || (eProductTypeCallGeneratorSoftMCU == productType))
	{
		AddParamNotVisible(sec, "MIX_AVC_SVC_DYNAMIC_ALLOCATION", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaResource, eCfgParamDataBoolean);
	}
	
	else
	{
		AddParamNotVisible(sec, "MIX_AVC_SVC_DYNAMIC_ALLOCATION", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaResource, eCfgParamDataBoolean);
	}

	// Adding HD(720) VSW "path" to Avc Svc MixMode
	AddParamVisible(sec, "ENABLE_HIGH_VIDEO_RES_AVC_TO_SVC_IN_MIXED_MODE", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty);
	AddParamVisible(sec, "ENABLE_HIGH_VIDEO_RES_SVC_TO_AVC_IN_MIXED_MODE", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty);

	// --AGC
	AddParamVisible(sec, "ENABLE_AGC", "YES", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty);

	// --Gathering
	AddParamNotVisible(sec, CONF_GATHERING_DURATION_SECONDS, "180", type,_0_TO_3600_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, PARTY_GATHERING_DURATION_SECONDS, "15", type,_0_TO_3600_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	// PSTN dial in
	AddParamVisible(sec, "USE_GK_PREFIX_FOR_PSTN_CALLS", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);

	// Possible values - empty string, string to add after sip address
	AddParamVisible(sec, "SIP_AUTO_SUFFIX_EXTENSION", "", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString, eProcessConfParty);

	// Possible values - decimal number, in range of SIP error messages
	AddParamNotVisible(sec, "SIP_BUSY_UPONRESOURCE_ERROR_ID", "486", type,_100_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	// Version 6 flags
	AddParamNotVisible(sec, "SET_AUDIO_PLC", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SET_AUDIO_CLARITY", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SET_AUTO_BRIGHTNESS", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// G711Alaw flag
	AddParamNotVisible(sec, "PREFERRED_G711_MODE", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// Reduce audio codecs in Sip Dial Out call:
	AddParamNotVisible(sec, "SIP_REDUCE_AUDIO_CODECS_DECLARATION", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_ENABLE_FLOW_CONTROL_REINVITE, "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	// LPR - Time Out
	AddParamNotVisible(sec, "LPR_ACTIVITY_MAX_DURATION_IN_SECONDS", "0", type,_0_TO_3600_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_SIP_PREVENT_OVERFLOW, "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "RTCP_QOS_IS_EQUAL_TO_RTP", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "RESOURCE_PRIORITY_IN_REQUIRE", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_ENABLE_DSP_RECOVERY_ON_KILL_PORT, "NO",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, H264_MB_INTRA_REFRESH, "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SUPPORT_HIGH_PROFILE", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SUPPORT_HIGH_PROFILE_WITH_ISDN", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ENABLE_NO_VIDEO_RESOURCES_AUDIO_ONLY_MESSAGE","YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SPREADING_VIDEO_PORTS", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaResource, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "BURN_BIOS", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "MPMX_MAX_ART_PORTS_PER_UNIT", "9", type,_0_TO_3600_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	// Traffic shaping:
	AddParamNotVisible(sec, CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING, "NO", type, _YES_NO, true, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_TRAFFIC_SHAPING_WINDOW_SIZE, "100", type,_50_TO_1000_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_TRAFFIC_SHAPING_MTU_FACTOR, "800", type, _0_TO_5000_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_TRAFFIC_SHAPING_MTU_MIN,    "410", type, _0_TO_360000_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_VIDEO_BIT_RATE_REDUCTION_PERCENT, "5", type,_0_TO_60_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);

	// ISDN DTMF FORWARDING
	AddParamNotVisible(sec, CFG_KEY_DTMF_FORWARD_ANY_DIGIT_TIMER_SECONDS, "60",type, _0_TO_360000_DECIMAL,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamVisible(sec, "EXT_DB_IVR_PROV_TIME_SECONDS", "300", type,_0_TO_100000_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);

	// VIDEO traffic shaping
	AddParamNotVisible(sec, "MAX_HD_VIDEO_TX_BITS_PER_10_MILLI_SECONDS","80000", type, _10000_TO_1000000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "MAX_VIDEO_TX_BITS_PER_10_MILLI_SECONDS", "64000",type, _10000_TO_1000000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_MAX_SINGLE_TRANSFER_FRAME_SIZE_BITS, "320000", type, _10000_TO_1000000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// Recurrent intra request - in minutes
	AddParamNotVisible(sec, "ENCODER_RECURRENT_INTRA_REQ_MINUTES", "7", type,_0_TO_120_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "DECODER_RECURRENT_INTRA_REQ_MINUTES", "60", type,_0_TO_120_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	// Recurrent intra request for TIP - in minutes
	AddParamNotVisible(sec, "ENCODER_TIP_RECURRENT_INTRA_REQ_MINUTES", "10",type, _0_TO_120_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	// redial parameters
	AddParamNotVisible(sec, "REDIAL_INTERVAL_IN_SECONDS", "30", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	// IBM environment
	AddParamVisible(sec, "IBM_ENVIRONMENT", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	//Standart_ice
	AddParamVisible(sec, "ENABLE_STANDART_ICE","NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// Pstn Echo delay
	AddParamNotVisible(sec, "PSTN_ECHO_DELAY_MILLI_SECOND", "20", type,_2_TO_160_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "OPERATOR_ASSISTANCE_THROUGH_REQUEST_ONLY", "NO",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// SIP PPC
	AddParamNotVisible(sec, "ENABLE_SIP_PEOPLE_PLUS_CONTENT", (eProductTypeSoftMCUMfw == productType) ? "NO" : "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	AddParamNotVisible(sec, "ENABLE_SIP_PPC_FOR_ALL_USER_AGENT", "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "DISABLE_SIP_PPC_VIDEO_RATE_REDUCTION", "YES",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// VNGR-17782 - To Enable disable H323 round trip keep alive
	AddParamNotVisible(sec, "IP_ENABLE_ROUNDTRIPDELAY", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	//VNGFE-8577
	AddParamNotVisible(sec, "SEND_RECAP_ATER_REALLOC_FOR_H323", "YES",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// Cascade video lopp back
	AddParamNotVisible(sec, "AVOID_VIDEO_LOOP_BACK_IN_CASCADE", "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	// Session timer values
	AddParamNotVisible(sec, "SIP_SESSION_EXPIRES", "90", type, _0_TO_DWORD,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "SIP_MIN_SEC", "90", type, _0_TO_DWORD,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	AddParamVisible(sec, CFG_KEY_MAX_KEEP_ALIVE_REQUESTS, "0", type,_0_TO_DWORD, false, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessMcuMngr);
	AddParamNotVisible(sec, CFG_KEY_RESTORE_FACTORY_DEFAULTS_LOG_FILES, "NO",type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "V35_USE_SSL_PORTS", "YES",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_V35_MULTIPLE_SERVICES, "NO",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// Traffic Control
	AddParamNotVisible(sec, CFG_KEY_ENABLE_TC_PACKAGE, "NO", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_TC_LATENCY_SIZE, "500", type,_1_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_TC_BURST_SIZE, "10", type,_0_TO_30_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_ENFORCE_SAFE_UPGRADE, "YES", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "HD_WATCHDOG_TIMEOUT", "60", type, _0_TO_DWORD,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);

	// Flow control on content in VSW
	AddParamNotVisible(sec, "VSW_CONTENT_VIDEO_RATE_REDUCTION", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// Soft Intra (Big Budget)
	AddParamNotVisible(sec, "H263_HIGH_BIT_BUDGET_INTRA", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// vngr-20510 - HIGH CPU USAGE PATCH
	AddParamNotVisible(sec, CFG_KEY_SYNC_DESTROY_SOCKET, "YES", type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	// AddParamNotVisible(sec, CFG_KEY_SHOW_CPU_USAGE_ALARM, CFG_STR_NO, type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamVisible(sec, CFG_KEY_MONITORING_PACKET, CFG_STR_YES, type, _YES_NO,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	// VNGFE-3587
	AddParamNotVisible(sec, "ALWAYS_FORWARD_DTMF_IN_GW_SESSION_TO_ISDN", "NO",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// SESSION_TIMER_EXPIRED_ALERT flag
	AddParamNotVisible(sec, "ENABLE_SESSION_TIMER_EXPIRED_ALERT", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// Play blip sound when auto cascade link is connected
	AddParamNotVisible(sec, "CASCADE_LINK_PLAY_TONE_ON_CONNECTION", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "ROUTE_NAME_CHANGE_INVALID_CHAR_TO_UNDERSCORE","NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// ICE - indicates whether to preserve ICE within a local call
	AddParamNotVisible(sec,	CFG_KEY_SIP_PRESERVE_ICE_CHANNEL_IN_CASE_OF_LOCAL_MODE, CFG_STR_NO,type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, "INITIATE_TIP_CONTENT_INTRA", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_SPLIT_TIP_VIDEO_RESOURCES, "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// Flag for CAC
	AddParamNotVisible(sec, "CAC_ENABLE", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// On non RMX1500Q system (with MPMX) - CIF/SD promilles changed to 30%/40%
	// - VNGR-21581 On RMX1500Q - CIF/SD promiles are 25%/50% but we still
	// allocate 3 CIF per video DSP, and only after all DSPs are full with 3 CIF
	// then we allocate a forth CIF on them (in order to meet 25CIF req).
	// When 3CIF_PORTS_ON_DSP=YES we don't allocate the forth CIF
	// (so max CIF = 21 = 3*7).
	AddParamNotVisible(sec, "3CIF_PORTS_ON_DSP", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// Encryption
	AddParamNotVisible(sec, CFG_KEY_DELAY_BETWEEN_GENERATE_ENC_KEY, "50", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
	AddParamNotVisible(sec, "GK_IDENTIFIER", "", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
	AddParamVisible(sec, "H263_ANNEX_T", "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty);

	//Cancel for DUMMY REGISTRATION. Yes|NO. Default: NO
	AddParamNotVisible(sec, "DISABLE_DUMMY_REGISTRATION", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	if (eProductTypeRMX1500 == productType || eProductTypeSoftMCUMfw == productType/* || eProductTypeNinja == productType*/)
	{
		AddParamNotVisible(sec, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_POLYCOM", "100", type, _0_TO_WORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
		AddParamNotVisible(sec, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_ISDN", "100", type, _0_TO_WORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
		AddParamNotVisible(sec, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_IP", "100", type, _0_TO_WORD, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber);
	}
	else if (eProductTypeGesher == productType || eProductTypeSoftMCU == productType || eProductTypeEdgeAxis == productType || eProductTypeCallGeneratorSoftMCU == productType)
	{
			AddParamNotVisible(sec, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_POLYCOM",	"60", type, _0_TO_WORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
			AddParamNotVisible(sec, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_ISDN", "60", type, _0_TO_WORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
			AddParamNotVisible(sec, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_IP", "60", type, _0_TO_WORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	}
	else
	{
		AddParamNotVisible(sec, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_POLYCOM", "200", type, _0_TO_WORD,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
		AddParamNotVisible(sec, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_ISDN", "200", type, _0_TO_WORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
		AddParamNotVisible(sec, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_IP", "200", type, _0_TO_WORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	}

	AddParamNotVisible(sec, "H264_HD_GRAPHICS_MIN_CONTENT_RATE", "128", type,_0_TO_1536_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum);
	AddParamNotVisible(sec, "H264_HD_HIGHRES_MIN_CONTENT_RATE", "256", type,_0_TO_1536_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum);
	AddParamNotVisible(sec, "H264_HD_LIVEVIDEO_MIN_CONTENT_RATE", "384", type,_0_TO_1536_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum);
	AddParamNotVisible(sec, CFG_KEY_SET_DTMF_SOURCE_DIFF_IN_SEC, "120", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_SET_DTMF_SOURCE_DIFF_IN_SEC_VOIP, "120", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	AddParamNotVisible(sec, CFG_KEY_ACCEPT_H323_DTMF_TYPE, "0", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_ACCEPT_VOIP_DTMF_TYPE, "0", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	AddParamNotVisible(sec, "FORCE_CONTENT_TO_H264_HD", "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// VNGFE-4652
	AddParamNotVisible(sec, "ISDN_CASCADE_LONG_RMT_AUDIO_NEGOTIATION", "NO",type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	// VNGFE-3587
	AddParamNotVisible(sec, "ICE_RETRY_TIMER_IN_SECONDS", "5", type,_0_TO_100_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	// VNGFE-4753 - We allow by default to simulate REMOTE_XMIT_MODE in ISDN in all cases.
	// When this flag set to NO, we disable the simulation of REMOTE_XMIT_MODE when changing between H263 and H264. - (Done for VNGR-21260)
	AddParamNotVisible(sec, "SIMULATE_REMOTE_XMIT_MODE_ISDN_H263_TO_H264","YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// VNGR-23595: SEND_SRTP_MKI flag
	//AddParamNotVisible(sec, "SEND_SRTP_MKI", CFG_STR_YES, type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SEND_SRTP_MKI", "AUTO", type, ONE_LINE_BUFFER_LENGTH, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString,eProcessConfParty); //BRIDGE-10123
	AddParamNotVisible(sec, CFG_KEY_DTMF_IVR_OPERATION_DELAY, "100", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	//Add for EXT-4321
	AddParamNotVisible(sec, CFG_KEY_ETH_INACTIVITY_DURATION, "30", type,_20_TO_120_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);

	// EXT-4632, VNGR-20237
	AddParamVisible(sec, CFG_KEY_GK_MANDATORY_FOR_CALLS_OUT, "NO", type,_YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);
	AddParamVisible(sec, CFG_KEY_GK_MANDATORY_FOR_CALLS_IN, "NO", type,_YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_ENABLE_MULTI_PART_CDR, CFG_STR_NO, type,_YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_EXCLUSIVE_CONTENT_MODE, "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// 448p feature (for tandberg EPs, V7.6.1)
	AddParamNotVisible(sec, "USE_INTERMEDIATE_SD_RESOLUTION", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// For Ntp synchronization  VNGR-22433
	AddParamNotVisible(sec, CFG_KEY_NTP_LOCAL_SERVER_STRATUM, "14", type,_1_TO_15_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);

	// Flag to disable conference cleanup feature when a conf task gets fault. Enabled by default.
	AddParamNotVisible(sec, "ENABLE_CONFERENCE_CLEANUP_ON_FAULT", "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);

	// Maximum number of video parties in conf - MPMX
	AddParamNotVisible(sec, CFG_KEY_MPMX_MAX_NUMBER_OF_VIDEO_PARTIES_IN_CONF,"180", type, _180_TO_360_DECIMAL,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// Flag to switch redial on wrong number feature
	AddParamNotVisible(sec, CFG_KEY_ENABLE_REDIAL_ON_WRONG_NUMBER, "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_WRONG_NUMBER_DIAL_RETRIES, "3", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	//HOMOLOGATION
	AddParamNotVisible(sec, "BRQ_ON_CALL_START", "0", type, _0_TO_120_DECIMAL,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// Indication on Layout
	AddParamNotVisible(sec, CFG_KEY_DISABLE_CELLS_NETWORK_IND, "YES", type,_YES_NO, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_CELL_IND_LOCATION, "TOP_RIGHT", type,ICON_LOCATION_ENUM, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum); // eLocationTopRight
	AddParamNotVisible(sec, CFG_KEY_NETWORK_IND_MAJOR_PERCENTAGE, "1", type,_0_TO_100_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_NETWORK_IND_CRITICAL_PERCENTAGE, "5", type,_0_TO_100_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	//CRLF
	AddParamNotVisible(sec, "SOCKET_ACTIVITY_TIMEOUT", "300", type,_0_TO_360000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	//BFCP
	AddParamNotVisible(sec, CFG_KEY_SIP_BFCP_DIAL_OUT_MODE, "AUTO", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString);
	AddParamVisible(sec, CFG_KEY_BFCP_HELLO_KEEP_ALIVE_COUNT, "0", type,_0_TO_10_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamVisible(sec, CFG_KEY_BFCP_REQUEST_RETRY_TIMES, "3", type,_1_TO_7_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	//VNGR-25962
	AddParamNotVisible(sec, "PSTN_RINGING_DURATION_SECONDS", "45", type,_5_TO_120_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamVisible(sec, CFG_KEY_ENABLE_WRITE_LOG_TO_FILE, "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean, eProcessLogger);

	AddParamNotVisible(sec, "CPU_BONDING_MODE", "balance-alb", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataString);
	AddParamNotVisible(sec, "CPU_BONDING_LINK_MONITORING_FREQUENCY", "100", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	// 1080p60
	AddParamNotVisible(sec, "1080_60_FPS", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// Flag for Replacing the media IP with other IP (useful for Cloud IP):
	AddParamNotVisible(sec, "CLOUD_IP", "", type, ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);

	// Flag for blocking audio protocols other than G711
	AddParamVisible(sec, CFG_KEY_FORCE_G711A, "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

//	// Flag for SVC AVC Mix Mode
//	AddParamNotVisible(sec, CFG_KEY_SVC_TO_AVC_SUPPORTS, "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	AddParamNotVisible(sec, CFG_KEY_AVC_TO_SVC_INTRA_DELAY, "20", type,_0_TO_60_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, "SVC_INTRA_SUPPRESSION_DURATION_IN_MILLISECONDS", "1000", type,_0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);

//
//	// Flag for blocking AVC to SVC Mix Mode
//	AddParamNotVisible(sec, CFG_KEY_AVC_TO_SVC_SUPPORTS, "NO", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
//	AddParamNotVisible(sec, "IVR_FOR_SVC", "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	AddParamVisible(sec, CFG_KEY_MIN_SYSTEM_DISK_SPACE_TO_ALERT, "2048", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);

//	AddParamNotVisible(sec, CFG_KEY_VSW_RELAY_SUPPORT, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_ENABLE_1080_SVC, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	//VNGR-26449 - unencrypted conference message
	AddParamNotVisible(sec, "DISPLAY_UNENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE", "0", type, _MINUS_1_TO_300_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "DISPLAY_ENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE", "0", type, _MINUS_1_TO_300_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// Amdocs feature - VNGR-26821
	// Reserve last free units on MPM+ card according to the flag.
	// We do this in order to allow max allocation of 400 audio parties per card (25 * 16 = 400)
	// Range values: 0 to 7, default: 0.
	AddParamNotVisible(sec, CFG_KEY_LOW_PRIORITY_UNITS_ON_MPM_PLUS, "0", type, _0_TO_10_DECIMAL, eCfgParamResponsibilityGroup_McaResource, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_NUM_OF_PARTICIPANTS_TO_CHANGE_MRMP_LOG_LEVEL, "30", type, _0_TO_90_DECIMAL, eCfgParamResponsibilityGroup_McaResource, eCfgParamDataNumber);

	AddParamNotVisible(sec, FORCE_LEGACY_EP_CONTENT_LAYOUT_ON_TELEPRESENCE, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, BLOCK_CONTENT_LEGACY_FOR_LYNC, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	//Media encryption authentication
	AddParamNotVisible(sec, CFG_KEY_SRTP_SRTCP_HMAC_SHA_LENGTH, "80", type, SRTP_SRTCP_HMAC_SHA_LENGTH_ENUM, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataEnum);

	//DTLS / SDES / AUTO / NONE
	AddParamNotVisible(sec, CFG_KEY_SIP_ENCRYPTION_KEY_EXCHANGE_MODE, "AUTO", type,SIP_ENCRYPTION_KEY_EXCHANGE_MODE_ENUM, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum);

	//system flag for Deloytte requirements
	AddParamNotVisible(sec, "PRESERVE_PARTY_CELL_ON_FORCE_LAYOUT", "NO",type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);


	//CS < - > MCMS communication mode
	AddParamNotVisible(sec, "CS_API_XML_MODE", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	//H261 new support mode
	AddParamNotVisible(sec, "H261_SUPPORT_ALWAYS", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	AddParamNotVisible(sec, CFG_KEY_SIP_DETECT_DISCONNECT_TIMER, "20", type, _0_TO_300_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_H323_DETECT_DISCONNECT_TIMER, "20", type, _0_TO_300_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);

	AddParamNotVisible(sec, "DELAY_BETWEEN_UPDATE_CONTENT_ON_DISCONNECTION", "15", type, _1_TO_60_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);

	// CFG_KEY_REMOVE_EP_FROM_LAYOUT_ON_NO_VIDEO_TIMER - values below 20 disable the mechanism
	AddParamNotVisible(sec, CFG_KEY_REMOVE_EP_FROM_LAYOUT_ON_NO_VIDEO_TIMER, "20", type, _0_TO_300_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_RETURN_EP_TO_LAYOUT_ON_NO_VIDEO_TIMER, "10", type, _7_TO_90_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);

	AddParamNotVisible(sec, CFG_KEY_ALLOW_SIREN7_CODEC, "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);

	//MFW calls from DMA only
	// TODO: uncomment next three strings before GA
	//if (CProcessBase::GetProcess()->GetProductType()==eProductTypeSoftMCUMfw)
	//	AddParamNotVisible(sec, "CALLS_ONLY_FROM_DMA", "YES", type, _YES_ONLY, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	//else
		AddParamNotVisible(sec, "CALLS_ONLY_FROM_DMA", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	//_t_p_
	AddParamNotVisible(sec, CFG_KEY_FORCE_720P_2048_FOR_PLCM_TIP, "FORCE_720P_2048_FOR_PLCM_TIP", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
	AddParamNotVisible(sec, CFG_KEY_TIP_PACKET_LOSS_SEND_INTRA_INTERVAL_SEC , "3", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_TIP_SEND_INTRA_FOR_PLI, "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "MANAGE_TELEPRESENCE_ROOM_SWITCH_LAYOUTS", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean,eProcessConfParty);
	AddParamNotVisible(sec, "ENABLE_CODIAN_CASCADE", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	AddParamNotVisible(sec, "AS_SIP_CONTENT_ADD_DELAY", "5", type,_0_TO_10_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	//NGB weighted ART
	AddParamNotVisible(sec, "RESTRICT_ART_ALLOCATION_ACCORDING_TO_WEIGHT", "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaResource, eCfgParamDataBoolean, eProcessResource);

	//Sip Proxy: add/remove alarm for faulty registration
	AddParamNotVisible(sec, "ENABLE_SIP_SERVER_OR_REGISTRAR_ERROR_ACTIVE_ALARM", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	//AddParamNotVisible(sec, "SUPPORT_MULTIPLE_LYNC_USERS", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	//For AT&T - in delivery to main stream need to change the default to YES
	AddParamNotVisible(sec, CFG_KEY_ENABLE_COREDUMP_NOTIFICATIONS, "YES", type, _YES_NO, false,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean,eProcessMcuMngr);
	AddParamNotVisible(sec, "REMOVE_IP_IF_NUMBER_EXISTS", "YES", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);

    AddParamNotVisible(sec, CFG_KEY_MINIMUM_DELAY_SNMP_READY_TIMER,	"20", type, _2_TO_120_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_TIP_RESOLUTION_ACCORDING_TO_CONF_VID_QUALITY, "YES", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);

	AddParamNotVisible(sec, CFG_KEY_APACHE_WAITING_LICENSING_TIME_SECONDS, "60", type, _1_TO_90_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_APACHE_LOG_LEVEL, "notice", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataString);

	//FSN-128 - Selective Mixing
	AddParamNotVisible(sec, CFG_KEY_ENABLE_SELECTIVE_MIXING, "YES", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);

	AddParamNotVisible(sec, "REPEAT_INTRA_NUMBER", "5", type, _0_TO_100_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	AddParamNotVisible(sec, CFG_KEY_WAIT_BEFORE_DISPATCH_MCCF, "2", type, _0_TO_2_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber); // in seconds

	AddParamNotVisible(sec, CFG_KEY_SLIDES_ENV, "GENERIC", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);

	AddParamNotVisible(sec, CFG_KEY_FORCE_AVP_ON_ENCRYPT_WHEN_POSSIBLE, "YES", type, ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataString, eProcessConfParty);

	// for BRIDGE-9818 (audio leak between TIP EP screens)
		AddParamNotVisible(sec, "ACTIVE_SPEAKER_PREFERENCE", "0", type, _0_TO_MAX_ACTIVE_SPEAKER_PREFERENCE, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

	// shut down system when temperature reach upper critical
	AddParamNotVisible(sec, "DISABLE_TEMPERATURE_CONTROL", "NO", type,_YES_NO, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean,eProcessMcuMngr);

	AddParamNotVisible(sec, CFG_MCMS_NETWORK_FINISH_TIMEOUT, "60", type, _0_TO_300_DECIMAL,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_IPV6_AUTO_CONFIG_TIMEOUT, "40", type, _0_TO_120_DECIMAL,eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	// Amdocs Encoder/Decoder Gain
	AddParamNotVisible(sec, CFG_KEY_AUDIO_DECODER_GAIN_G729, "100", type, _0_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_AUDIO_ENCODER_GAIN_G729, "100", type, _0_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_AUDIO_DECODER_GAIN_G711, "100", type, _0_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_AUDIO_ENCODER_GAIN_G711, "100", type, _0_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_AUDIO_DECODER_GAIN_G722, "100", type, _0_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_AUDIO_ENCODER_GAIN_G722, "100", type, _0_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_AUDIO_DECODER_GAIN_G722_1, "100", type, _0_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_AUDIO_ENCODER_GAIN_G722_1, "100", type, _0_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	AddParamNotVisible(sec, CFG_KEY_ENBLE_OS_LATENCY_DETECTION, (productType == eProductTypeEdgeAxis) ? "YES" : "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	AddParamNotVisible(sec, CFG_KEY_OS_LATENCY_HIGH_THRESHOLD, "8000" , type, _0_TO_100000_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_OS_LATENCY_LOW_THRESHOLD,  "6000" , type, _0_TO_100000_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_RTCP_LYNC_PREFERENCE_REQ_INTERVAL, "3", type, _0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_SIP_OMIT_DOMAIN_FROM_PARTY_NAME, "YES", type,_YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);

	// for BRIDGE-5085: move from Debug section to User section (after consulted with SRE)
	AddParamNotVisible(sec, "RFC2833_DTMF", "YES", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);

	// Lync2013 testing
	AddParamNotVisible(sec, "ENABLE_MSFT_CAP", "0", type, _0_TO_2_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
	AddParamNotVisible(sec, "LYNC2013_PATCH_NUM_OF_LAYERS", "2",type, _2_TO_3_DECIMAL,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_MAX_NUM_OF_MS_IN_SLAVES, "4", type, _0_TO_4_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_DISABLE_MS_IN_SLAVES_DYNAMIC_REMOVAL, "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, "DISABLE_LYNC_AV_MCU_128_192_KBPS", "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);

	AddParamNotVisible(sec, CFG_KEY_CDR_SERVICE_QUEUE_SIZE, (eProductTypeNinja == productType || eProductTypeRMX4000 == productType) ? "2" : "1", type, _1_TO_5_DECIMAL, true, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber, eProcessCDR);

	AddParamNotVisible(sec, CFG_KEY_CDR_SERVICE_QUEUE_FULL_THRESHOLD, "80", type, _70_TO_95_DECIMAL,false,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber,eProcessCDR);
	AddParamNotVisible(sec, CFG_KEY_CDR_SERVICE_RETRY_TIME, "5", type, _1_TO_60_DECIMAL,false,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber,eProcessCDR);
	AddParamNotVisible(sec, "DISPLAY_CDR_SETTINGS", "NO", type,_YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessMcuMngr);

	//add a debug sys flag to disable msslave party monitoring for development purpose.
	//If it is YES all the slaves will be seen(as before your change). Default value is YES
	//If it is NO then the "real" monitoring will happen.
	//AddParamNotVisible(sec, "MS_DEBUG_MONITORING", "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, "MS_AV_MCU_MONITORING","MAIN_AND_IN_SLAVE", type, ONE_LINE_BUFFER_LENGTH, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString,eProcessConfParty);
	AddParamNotVisible(sec, "MS_DEBUG_CONNECT_WITHOUT_DMA_AS_WITH", "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, "MS_DEBUG_FOR_FEDERATE", "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, "GK_DELAY_BETWEEN_MESSAGES_TO_CS", "10", type, _0_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber); // in seconds

	//eFeatureRssDialin
	AddParamNotVisible(sec, CFG_KEY_ENABLE_RECORDING_CONTROL_VIA_SIPINFO, "YES", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);

	AddParamNotVisible(sec, "RESET_SDES_FOR_VENDOR","Cisco", type, ONE_LINE_BUFFER_LENGTH, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString,eProcessConfParty);
	
        AddParamNotVisible(sec, CFG_KEY_LICENSE_VALIDATION_INTERVAL, "60", type, _1_TO_1000_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
    	AddParamNotVisible(sec, "LICENSE_MODE", "flexera", type, LICENSE_MODE_ENUM,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataEnum);


    AddParamNotVisible(sec, CFG_KEY_CONF_TASK_RECEIVE_BUFFER_SIZE, "1", type, _1_TO_10_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
    AddParamNotVisible(sec, CFG_KEY_CONF_TASK_SEND_BUFFER_SIZE, "1", type, _1_TO_10_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);

	//FSN-489
	if(eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
		AddParamNotVisible(sec, CFG_KEY_FOLLOW_SPEAKER_ON_1X1, "NO", type, FOLLOW_SPEAKER_ON_1X1_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum, eProcessConfParty);
	else
		AddParamNotVisible(sec, CFG_KEY_FOLLOW_SPEAKER_ON_1X1, "AUTO", type, FOLLOW_SPEAKER_ON_1X1_ENUM, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataEnum, eProcessConfParty);
		
		
	//cropping
	AddParamVisible(sec, "ITP_CROPPING", "ITP", type, ITP_CROPPING_ENUM, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum,eProcessConfParty);
   	AddParamNotVisible(sec, "CROPPING_PERCENTAGE_THRESHOLD_GENERAL", "-1", type, _MINUS_1_TO_100_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);
   	AddParamNotVisible(sec, "CROPPING_PERCENTAGE_THRESHOLD_PANORAMIC", "-1", type, _MINUS_1_TO_100_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);

   	AddParamNotVisible(sec, "ENABLE_DTMF_NUMBER_WO_DELIMITER", "YES", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_LIMIT_CIF_SD_PORTS_PER_MPMX_CARD, "NO", type, _YES_NO, true, eCfgParamResponsibilityGroup_McaResource, eCfgParamDataBoolean, eProcessResource);
   	AddParamNotVisible(sec, CFG_KEY_ENABLE_CONTENT_IN_PREFER_TIP_FOR_CALL_RATES_LOWER_THAN_768K, "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty); //BRIDGE-13154

	AddParamNotVisible(sec, CFG_KEY_SPREAD_VIDEO_ALLOCATION_ON_NETRA_UNITS, (eProductTypeNinja == productType) ? "YES" : "NO", type, _YES_NO, true, eCfgParamResponsibilityGroup_McaResource, eCfgParamDataBoolean, eProcessResource);
	AddParamNotVisible(sec, CFG_KEY_NUM_OF_PCM_IN_MPMX, "1", type, _0_OR_1_OR_4_DECIMAL,true,	eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber,eProcessCards);

	// VNGFE-8204, no reset require.
	AddParamNotVisible(sec, "LPR_CONTENT_RATE_ADJUST_WEAK_LPR", "NO", type, _YES_NO, false,	eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, "ENABLE_CONTENT_OF_768_FOR_1024_LV", "NO", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);

	AddParamNotVisible(sec, "SET_CHAIR_PERMISSIONS_TO_H323_GW_WITHOUT_PASSWORD", "YES", type, _YES_NO, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataBoolean, eProcessConfParty);
	AddParamNotVisible(sec, CFG_KEY_RETRIEVE_OCCUPIED_UDP_PORTS_TIMER, "60", type,_0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
}

bool CSysConfig::IsSuppressedValidity(const char* key) const
{
	PASSERT_AND_RETURN_VALUE(NULL == key, false);

	static const char* keysJitc[] = {
		"SEPARATE_MANAGEMENT_NETWORK",
		"MIN_PWD_CHANGE_FREQUENCY_IN_DAYS",
		"FORCE_STRONG_PASSWORD_POLICY",
		"MIN_PASSWORD_LENGTH",
		"NUM_OF_LOWER_CASE_ALPHABETIC",
		"NUM_OF_UPPER_CASE_ALPHABETIC",
		"NUM_OF_SPECIAL_CHAR",
		"NUM_OF_NUMERIC",
		"MAX_PASSWORD_REPEATED_CHAR",
		"PASSWORD_EXPIRATION_DAYS",
		"PASSWORD_EXPIRATION_DAYS_MACHINE",
		"PASSWORD_EXPIRATION_WARNING_DAYS",
		"PASSWORD_HISTORY_SIZE",
		"DISABLE_INACTIVE_USER",
		"ENABLE_CYCLIC_FILE_SYSTEM_ALARMS",
		"USER_LOCKOUT",
		"USER_LOCKOUT_WINDOW_IN_MINUTES",
		"USER_LOCKOUT_DURATION_IN_MINUTES",
		"LAST_LOGIN_ATTEMPTS",
		"SESSION_TIMEOUT_IN_MINUTES",
		"PASSWORD_FAILURE_LIMIT",
		"MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM",
		"MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_USER",
		"NUMERIC_CONF_PASS_MIN_LEN",
		"NUMERIC_CHAIR_PASS_MIN_LEN",
		NUMERIC_CONF_PASS_DEFAULT_LEN,
		NUMERIC_CHAIR_PASS_DEFAULT_LEN,
		NUMERIC_CONF_PASS_MAX_LEN,
		NUMERIC_CHAIR_PASS_MAX_LEN,
		"APACHE_KEEP_ALIVE_TIMEOUT",
		"JITC_MODE",
		CFG_KEY_ENFORCE_SAFE_UPGRADE,
		"MAX_CONF_PASSWORD_REPEATED_DIGITS"
	};

	//if not in JITC and flags were decided to be Removed from being VISIBLE and remooved totally , Please add them to this list , Otherwize we will get Active Alarms on SysConfig Validity
	 static const char* keysNonJitc[] = {
		"TERMINATE_CONF_AFTER_CHAIR_DROPPED",
		"MAX_TIP_COMPATIBILITY_LINE_RATE",
		"SITE_NAME_TRANSPARENCY",
		"TIP_MTU_SIZE",
		"MTU_SIZE_DURING_LPR",
		"SITE_NAMES_LOCATION",
		"SITE_NAMES_ALWAYS_ON",
		CFG_KEY_SHOW_CPU_USAGE_ALARM,
		"MIN_CHANGED_CHAR",
		"SYSTEM_MANAGEMENT_SESSION_LIMIT",
		"TALK_HOLD_TIME",
		"EM_TALK_HOLD_TIME",
		"HIDE_SITE_NAMES",
		"IVR_ROLL_CALL_SUPPRESS_OPERATOR",
		"SITE_NAMES_LOCATION",
		"VSW_RELAY_SUPPORT"
	};

	// Does not suppress in not-JITC mode
	if (!IsUnderJITCState())
	{
		for (const char** it = keysNonJitc; it < ARRAYEND(keysNonJitc); ++it)
		{
			if (0 == strcmp(*it, key))
			{
				TRACEINTOFUNC << "Validation error for " << key << " is suppressed";
				return true;
			}
		}
	}
	else
	{
		for (const char** it = keysJitc; it < ARRAYEND(keysJitc); ++it)
		{
			if (0 == strcmp(*it, key))
			{
				TRACEINTOFUNC << "Validation error for " << key << " is suppressed";
				return true;
			}
		}
	}

	return false;
}

void CSysConfig::InitUserJITCParams() const
{
	const char* sec = GetCfgSectionName(eCfgSectionMcmsUser);
	const eCfgParamType type = eCfgParamUser;
	const eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	const bool JITC = IsUnderJITCState();

	if (JITC)
	{
		AddParamNotVisible(sec, "ULTRA_SECURE_MODE", "YES", type, _YES_ONLY,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);

		if (eProductTypeRMX1500 == curProductType || eProductTypeRMX4000 == curProductType)
			AddParamVisible(sec, "SEPARATE_MANAGEMENT_NETWORK", "NO", type,		_NO_ONLY, eCfgParamResponsibilityGroup_SwSysInfraApp,		eCfgParamDataBoolean);
		else
			AddParamVisible(sec, "SEPARATE_MANAGEMENT_NETWORK", "NO", type,		_YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp,		eCfgParamDataBoolean);

        if (eProductTypeEdgeAxis == curProductType)
            AddParamNotVisible(sec, "SEPARATE_MANAGEMENT_NETWORK", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);

		AddParamVisible(sec, "MIN_PWD_CHANGE_FREQUENCY_IN_DAYS", "1", type,	_1_TO_7_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);

		AddParamVisible(sec, "FORCE_STRONG_PASSWORD_POLICY", "YES", type,_YES_ONLY, false, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean, eProcessAuthentication);

		AddParamNotVisible(sec, "MIN_PASSWORD_LENGTH", "15", type,	_15_TO_20_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamNotVisible(sec, "NUM_OF_LOWER_CASE_ALPHABETIC", "2", type,	_1_TO_2_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);

		AddParamNotVisible(sec, "NUM_OF_UPPER_CASE_ALPHABETIC", "2", type,	_1_TO_2_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);

		AddParamNotVisible(sec, "NUM_OF_NUMERIC", "2", type, _1_TO_2_DECIMAL,	false, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamNotVisible(sec, "NUM_OF_SPECIAL_CHAR", "2", type, _1_TO_2_DECIMAL,	false, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamNotVisible(sec, "MAX_PASSWORD_REPEATED_CHAR", "2", type,	_1_TO_4_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);

		AddParamVisible(sec, "MAX_CONF_PASSWORD_REPEATED_DIGITS", "2", type,_1_TO_4_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty); //BRIDGE-4400
		AddParamVisible(sec, "PASSWORD_EXPIRATION_DAYS", "60", type,	_7_TO_90_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "PASSWORD_EXPIRATION_DAYS_MACHINE", "365", type,	_1_TO_999_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "PASSWORD_EXPIRATION_WARNING_DAYS", "7", type,	_7_TO_14_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "PASSWORD_HISTORY_SIZE", "10", type,	_10_TO_16_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "DISABLE_INACTIVE_USER", "30", type,	_30_TO_90_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "ENABLE_CYCLIC_FILE_SYSTEM_ALARMS", "YES", type,	_YES_ONLY, false, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataBoolean, eProcessTypeInvalid, true);

		AddParamVisible(sec, "USER_LOCKOUT", "YES", type, _YES_NO, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataBoolean, eProcessTypeInvalid, true);
		AddParamVisible(sec, "USER_LOCKOUT_WINDOW_IN_MINUTES", "60", type,	_0_TO_45000_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "USER_LOCKOUT_DURATION_IN_MINUTES", "0", type,	_0_TO_480_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "LAST_LOGIN_ATTEMPTS", "YES", type, _YES_NO,	false, eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataBoolean, eProcessTypeInvalid, true);
		AddParamVisible(sec, "SESSION_TIMEOUT_IN_MINUTES", "10", type,_5_TO_60_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessTypeInvalid, true);
		AddParamVisible(sec, "PASSWORD_FAILURE_LIMIT", "3", type,_2_TO_10_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM","80", type, _4_TO_80_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessApacheModule);
		AddParamVisible(sec, "MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_USER","20", type, _4_TO_80_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessApacheModule);

		AddParamVisible(sec, "NUMERIC_CONF_PASS_MIN_LEN", "9", type,_9_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, "NUMERIC_CHAIR_PASS_MIN_LEN", "9", type,_9_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, NUMERIC_CONF_PASS_DEFAULT_LEN, "9", type,_9_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, NUMERIC_CHAIR_PASS_DEFAULT_LEN, "9", type,_9_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, NUMERIC_CONF_PASS_MAX_LEN, "16", type,_9_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, NUMERIC_CHAIR_PASS_MAX_LEN, "16", type,_9_TO_16_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);

		if (IsRmxSimulation())   //this change gives EMA developers an option to debug EMA during Simulation Tests
			AddParamNotVisible(sec, "APACHE_KEEP_ALIVE_TIMEOUT", "120", type,_1_TO_999_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
		else
			AddParamNotVisible(sec, "APACHE_KEEP_ALIVE_TIMEOUT", "15", type,_1_TO_999_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
		AddParamNotVisible(sec, "V35_ULTRA_SECURED_SUPPORT", "NO", type,_YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);

		AddParamNotVisible(sec, ENABLE_VIDEO_PREVIEW, "NO", type, _NO_ONLY,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
		AddParamNotVisible(sec, "EXCHANGE_MODULE_CURLOPT_TIMEOUT", "5", type,_5_TO_60_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
		AddParamNotVisible(sec, "ANAT_IP_PROTOCOL", "AUTO", type, ANAT_IP_PROTOCOL_ENUM, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum,eProcessConfParty);

		if (eProductTypeSoftMCUMfw == curProductType)
		{
			AddParamNotVisible(sec, CFG_SNMP_FIPS_MODE, "YES", type, _YES_ONLY, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
		}
		else
		{
			AddParamNotVisible(sec, CFG_SNMP_FIPS_MODE, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
		}
		
		AddParamNotVisible(sec, CFG_OCSP_NO_NONCE, "YES", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
		AddParamNotVisible(sec, CFG_OCSP_RESPONDER_TIMEOUT, "3", type,_1_TO_20_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
		AddParamNotVisible(sec, CFG_KEY_VIEW_RVGW_ACTIVEX, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

		//IPMC
		AddParamNotVisible(sec, CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
		AddParamNotVisible(sec, CFG_KEY_ENABLE_ACCEPTING_ICMP_REDIRECT, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

		AddParamNotVisible(sec, CFG_KEY_ENABLE_DHCPV6, "YES", type,_YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);
	}
	else
	{
		AddParamNotVisible(sec, "ULTRA_SECURE_MODE", "NO", type, _YES_NO,	eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);

        if (eProductTypeEdgeAxis == curProductType)
		    AddParamNotVisible(sec, "SEPARATE_MANAGEMENT_NETWORK", "NO", type, _YES_NO,	eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);
        else
            AddParamVisible(sec, "SEPARATE_MANAGEMENT_NETWORK", "NO", type,_NO_ONLY, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);

        AddParamVisible(sec, "MIN_PWD_CHANGE_FREQUENCY_IN_DAYS", "0", type,_0_TO_7_DECIMAL, false, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "FORCE_STRONG_PASSWORD_POLICY", "NO", type,_YES_NO, false, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean, eProcessAuthentication);
		AddParamNotVisible(sec, "MIN_PASSWORD_LENGTH", "0", type,_0_TO_20_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamNotVisible(sec, "NUM_OF_LOWER_CASE_ALPHABETIC", "0", type,_0_TO_2_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamNotVisible(sec, "NUM_OF_UPPER_CASE_ALPHABETIC", "0", type,_0_TO_2_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamNotVisible(sec, "NUM_OF_NUMERIC", "0", type, _0_TO_2_DECIMAL,false, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamNotVisible(sec, "NUM_OF_SPECIAL_CHAR", "0", type, _0_TO_2_DECIMAL,false, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamNotVisible(sec, "MAX_PASSWORD_REPEATED_CHAR", "0", type,	_0_TO_4_DECIMAL, false,	eCfgParamResponsibilityGroup_SwSysInfraApp,	eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "MAX_CONF_PASSWORD_REPEATED_DIGITS", "0", type,_0_TO_4_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty); //BRIDGE-4400
		AddParamVisible(sec, "PASSWORD_EXPIRATION_DAYS", "0", type,_0_TO_90_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "PASSWORD_EXPIRATION_DAYS_MACHINE", "0", type,_0_TO_999_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "PASSWORD_EXPIRATION_WARNING_DAYS", "0", type,_0_TO_14_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "PASSWORD_HISTORY_SIZE", "0", type,_0_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "DISABLE_INACTIVE_USER", "0", type,_0_TO_90_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "ENABLE_CYCLIC_FILE_SYSTEM_ALARMS", "NO", type,_YES_NO, false, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean, eProcessTypeInvalid, true);
		AddParamVisible(sec, "USER_LOCKOUT", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean, eProcessTypeInvalid, true);
		AddParamVisible(sec, "USER_LOCKOUT_WINDOW_IN_MINUTES", "60", type,_0_TO_45000_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "USER_LOCKOUT_DURATION_IN_MINUTES", "0", type,_0_TO_480_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "LAST_LOGIN_ATTEMPTS", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean, eProcessTypeInvalid, true);

		AddParamVisible(sec, "SESSION_TIMEOUT_IN_MINUTES", "0", type,	_0_TO_999_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessTypeInvalid, true);

		AddParamVisible(sec, "PASSWORD_FAILURE_LIMIT", "3", type,_2_TO_10_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessAuthentication);
		AddParamVisible(sec, "MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM","80", type, _4_TO_80_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessApacheModule);
		AddParamVisible(sec, "MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_USER","20", type, _4_TO_80_DECIMAL, false,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber, eProcessApacheModule);
		AddParamVisible(sec, "NUMERIC_CONF_PASS_MIN_LEN", "0", type,_0_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, "NUMERIC_CHAIR_PASS_MIN_LEN", "0", type,_0_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, NUMERIC_CONF_PASS_DEFAULT_LEN, "0", type,_0_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, NUMERIC_CHAIR_PASS_DEFAULT_LEN, "0", type,_0_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, NUMERIC_CONF_PASS_MAX_LEN, "16", type,_0_TO_16_DECIMAL, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber,eProcessConfParty);
		AddParamVisible(sec, NUMERIC_CHAIR_PASS_MAX_LEN, "16", type,_0_TO_16_DECIMAL, false, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber, eProcessConfParty);
                if (IsRmxSimulation())   //this change gives EMA developers an option to debug EMA during Simulation Tests
                        AddParamNotVisible(sec, "APACHE_KEEP_ALIVE_TIMEOUT", "120", type,_1_TO_999_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
                else
                        AddParamNotVisible(sec, "APACHE_KEEP_ALIVE_TIMEOUT", "15", type,_1_TO_999_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);
  

		AddParamNotVisible(sec, ENABLE_VIDEO_PREVIEW, "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
		AddParamNotVisible(sec, "EXCHANGE_MODULE_CURLOPT_TIMEOUT", "5", type,_5_TO_60_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);

		// flags that are not supported in JITC mode
		AddParamNotVisible(sec, "EXTERNAL_CONTENT_DIRECTORY","/PlcmWebServices", type, ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
		AddParamNotVisible(sec, "EXTERNAL_CONTENT_IP","" /*http://www.nasdaq.com*/, type, ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
		AddParamNotVisible(sec, CFG_KEY_EXTERNAL_CONTENT_PORT, "80", type,_0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);
		AddParamNotVisible(sec, CFG_KEY_EXTERNAL_CONTENT_POLLING_INTREVAL, "1",	type, _0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty,eCfgParamDataNumber);

		AddParamNotVisible(sec, "EXTERNAL_CONTENT_USER", "DOMAIN/USER", type,ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
		AddParamNotVisible(sec, "EXTERNAL_CONTENT_PASSWORD", "PASSWORD", type,ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
		AddParamNotVisible(sec, "V35_ULTRA_SECURED_SUPPORT", "NO", type,_NO_ONLY, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);

		if (eProductTypeSoftMCUMfw == curProductType)
		{
			AddParamNotVisible(sec, CFG_SNMP_FIPS_MODE, "YES", type, _YES_ONLY, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
		}
		else
		{
			AddParamNotVisible(sec, CFG_SNMP_FIPS_MODE, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
		}

		AddParamNotVisible(sec, "ANAT_IP_PROTOCOL", "DISABLED", type, ANAT_IP_PROTOCOL_ENUM, false,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataEnum,eProcessConfParty);

		AddParamNotVisible(sec, CFG_OCSP_NO_NONCE, "YES", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
		AddParamNotVisible(sec, CFG_OCSP_RESPONDER_TIMEOUT, "3", type,_1_TO_20_DECIMAL, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataNumber);

		AddParamNotVisible(sec, "DTLS_TIMEOUT", "5", type,	_0_TO_75_DECIMAL, false,	eCfgParamResponsibilityGroup_McaConfParty,	eCfgParamDataNumber, eProcessConfParty);
		AddParamNotVisible(sec, CFG_KEY_VIEW_RVGW_ACTIVEX, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
		//IPMC
		AddParamNotVisible(sec, CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE, "YES", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
		AddParamNotVisible(sec, CFG_KEY_ENABLE_ACCEPTING_ICMP_REDIRECT, "YES", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

		AddParamNotVisible(sec, CFG_KEY_ENABLE_DHCPV6, "YES", type,_YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataBoolean);
	}

	const bool enableMCCF = !JITC; // CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw;

	AddParamNotVisible(sec, CFG_KEY_ENABLE_CDR_FOR_MCCF, enableMCCF ? "YES" : "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_ENABLE_MCCF, enableMCCF ? "YES" : "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
}

void CSysConfig::InitCSModuleParams() const
{
	const char* sec = GetCfgSectionName(eCfgSectionCSModule);
	const eCfgParamType type = eCfgParamUser;

	AddParamNotVisible(sec, "CS_MS_ENVIRONMENT", "NO", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "SIP_VIDEO_DRAFT", "5", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_REMOVE_ROUTE_HDR", "NO", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_NORTEL", "NO", type, ONE_LINE_BUFFER_LENGTH, false,eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_CAP_DTMF_BY_INFO", "YES", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_DEBUG_REDUCE_RATE", "0", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_DEBUG_FORCE_RATE", "-1", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_DEBUG_QCIF_ONLY", "NO", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);

	AddParamNotVisible(sec, "REQUEST_PEER_CERTIFICATE", "NO", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "OCSP_GLOBAL_RESPONDER_URI", "", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "USE_RESPONDER_OCSP_URI", "NO", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "ALLOW_INCOMPLETE_REV_CHECK", "NO", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "SKIP_VALIDATE_OCSP_CERT", "NO", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "REVOCATION_METHOD", "none", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);



	// SIP Party <-> CS keep alive mechanism.
	AddParamNotVisible(sec, CFG_KEY_SIP_MSG_TIMEOUT, "50", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, CFG_KEY_SIP_IS_KEEPALIVE_ENABLE, "YES", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, CFG_KEY_SIP_KEEPALIVE_SECOND_TIMEOUT, "10", type,ONE_LINE_BUFFER_LENGTH,  eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "H245_TUNNELING", "NO", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "CALL_CLOSE_TIMEOUT", "2500", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "SEND_CALL_IDLE_TIMEOUT", "4500", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "RV_OID_BUG_BYPASS", "NO", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "VCON_NON_STD_DTMF", "YES", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "DISABLE_AUTHEN", "YES", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "NO_H245ADDRESS", "NO", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "STOP_PROCESSORS", "NO", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "CS_ENABLE_EPC", "NO", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "ICE_TYPE", "NONE", type, ONE_LINE_BUFFER_LENGTH, false,eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, CS_KEY_H323_RAS_IPV6, "YES", type, _YES_NO,eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	//---Sasha Sh. Homoloation.
	//Index of SIP Timers strategy. 0: "POLYCOM" timers strategy |  1: RFC-3261 recommendesecfault: 0
	//See: vob/Carmel_IP/CS/modules/siptask/SipInterface.c : SET_PARAMETERS_STRATEGY
	AddParamNotVisible(sec, "SIP_TIMERS_SET_INDEX", "0", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);

	//---Cancel/Insert parameter proxy=replace into "Contact:" header
	AddParamNotVisible(sec, "MS_PROXY_REPLACE", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataBoolean);
	//---change or no  ADDRESS and(or) PORT into "Route:" header. 0-NO, 1-PORT
	AddParamNotVisible(sec, "SIP_TCP_PORT_ADDR_STRATEGY", "0", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString);

	//-S------MultiPart SDP NEW ----------------------------------------//
	AddParamNotVisible(sec, "SDP_PART_SELECTION_ALGORITHM", "1", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString);
	//-F------MultiPart SDP NEW ----------------------------------------//
	//---SIP TCP Keep Alive
	AddParamNotVisible(sec, "SIP_TCP_KEEP_ALIVE_TYPE"    , "4", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString);
	AddParamNotVisible(sec, "SIP_TCP_KEEP_ALIVE_BEHAVIOR", "0", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString);
	//AddParamNotVisible(sec, "MS_KEEP_ALIVE_ENABLE", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataBoolean, eProcessCsModule);
	//Added new CS Flags Non Visible that were skipped
	//AddParamNotVisible(sec, "QOS_IP_SIGNALING", "0", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);

	AddParamNotVisible(sec, "SIP_TCP_TLS_TIMERS" , "1,5,4,500000,5", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString);
	//see: G_sSipTransportTimers

	AddParamNotVisible(sec, "SIP_RECV_DELAY" , "50,0", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString);
	//see: G_ForcedDelay_Duration; G_ForcedDelay_Mask in CS

	AddParamNotVisible(sec, "QOS_IP_SIGNALING", "0x28", type,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString);
	AddParamNotVisible(sec, "STOP_ALL_PROCESSORS", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataBoolean, eProcessCsModule);

	AddParamNotVisible(sec, "H460_AVAYA_IND_SIM", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "AVAYA_BOARD_HUNT_SIM", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "AUTHEN_PRINTS", "NO", type, _YES_NO, false,eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "BRD_ROUND_TRIP_REQUEST", "YES", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_ST_ENFORCE_VAL", "500", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_TLS_RECV_RETRY", "4", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_PARSE_MULTIPART_SDP", "YES", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_CS_OPTION_MODE", "0", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIMULATE_ACK_OR_491", "YES", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_DUAL_DIRECTION_TCP_CON", "NO", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_DUAL_DIRECTION_TLS_CON", "NO", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "H323_TIMERS_SET_INDEX", "0", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_TO_TAG_CONFLICT", "YES", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	AddParamNotVisible(sec, "SIP_ENABLE_CCCP", "YES", type,ONE_LINE_BUFFER_LENGTH, false, eCfgParamResponsibilityGroup_SwSysIp,eCfgParamDataString, eProcessCsModule);
	//Please do not add flags here add above
	AddParamInFileNonVisible(sec, "trace1level","all.StackMsgs.logfileDebugLevel.0", eCfgParamUser,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);
	AddParamInFileNonVisible(sec, "trace2level","all.XmlTrace.logfileDebugLevel.0", eCfgParamUser,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);
	AddParamInFileNonVisible(sec, "trace3level","siptask.SipMsgsTrace.logfileDebugLevel.0", eCfgParamUser,ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);

	AddParamNotVisible(sec, "MCU_VERSION_ID", "V100.0.0", type, ONE_LINE_BUFFER_LENGTH, false,eCfgParamResponsibilityGroup_SwSysIp, eCfgParamDataString, eProcessCsModule);
}

bool CSysConfig::ReadUpdate(const std::string& key, CCfgData*& cfgData) const
{
	cfgData = GetCfgEntryByKey(key);

	if (!cfgData)
	{
		std::string buff = "CSysConfig::ReadUpdate: Failed to read key : ";
		buff += key;
		//PASSERTMSG(1, buff.c_str());
		return false;
	}

	cfgData->IncrementCnt();
	return true;
}

void CSysConfig::InitCustomParams() const
{
	const char* sec = GetCfgSectionName(eCfgSectionMcmsDebug);
	const eCfgParamType type = eCfgCustomParam;

	AddParamNotVisible(sec, "REST_API_PORT", "443", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "XML_API_PORT", "80", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "XML_API_HTTPS_PORT", "443", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);

	AddParamNotVisible(sec, "STUN_SERVER_PORT", DEFAULT_ICE_STUN_TURN_SERVER_PORT_STR, type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "TURN_SERVER_PORT", DEFAULT_ICE_STUN_TURN_SERVER_PORT_STR, type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);

	AddParamNotVisible(sec, "CUSTOM_USER_LOGIN", "POLYCOM", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataString);
	AddParamNotVisible(sec, "CUSTOM_USER_PASSWD", "POLYCOM", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataString);
	AddParamNotVisible(sec, "FORCE_LOW_MEMORY_USAGE", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "DEFAULT_NETWORK_INTERFACE", "eth0", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataString);
}

void CSysConfig::InitDebugParams() const
{
	const char* sec = GetCfgSectionName(eCfgSectionMcmsDebug);
	const eCfgParamType type = eCfgParamDebug;

	AddParamNotVisible(sec, "RESET_HISTORY_TIME_INTERVAL", "30000", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_DEBUG_MODE, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_KEY_H323_CS_ERROR_HANDLE_FIRST_TIMER_VAL, "35", type, _0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_H323_CS_ERROR_HANDLE_SECOND_TIMER_VAL, "10", type, _0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "NTP_SYNC_PERIOD", "2", type, _1_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "NTP_SERVERS_STATUS_PERIOD", "2", type, _1_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "NTP_PEER_STATUS_PERIOD", "2", type, _1_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "CFS_KEYCODE_VERSION_NUM_IGNORED", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "DHCP_ENABLED", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "CS_XML_CONVERTER", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "MAX_EXTENSION_TIME", "604800", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "EXTENSION_TIME_INTERVAL", "10", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "LECTURE_MODE_TIMER_INTERVAL", "15", type, _0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// resolution degradation tolerance percentage
	AddParamNotVisible(sec, "VIDEO_RESOLUTION_DECREASE_THRESHOLD_PERCENT", "5", type, _0_TO_100_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);


	char* defaultStartupTime = "12000"; // 2 minutes
	switch (CProcessBase::GetProcess()->GetProductFamily())
	{
		case eProductFamilyCallGenerator:
			defaultStartupTime = "3000";
			break;

		case eProductFamilyRMX:
			// 11.8.10 VNGR 16885 - Haggai & Rachel increase the startup timer from
			// 12000 to 42000 (7 minutes)
			defaultStartupTime = "42000";
			break;

		case eProductFamilySoftMcu:
			defaultStartupTime = "3000"; // 30 seconds
			break;

		default:
			PASSERT(1);
	}


	AddParamNotVisible(sec, "MAX_STARTUP_TIME", defaultStartupTime, type, _1_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "TALK_HOLD_TIME", "300", type, _0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "EM_TALK_HOLD_TIME", "500", type, _0_TO_WORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_APACHE_PRINT, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "IS_DOUBLE_DSP", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaResource, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "MAX_FAULTS_IN_LIST", "1000", type, _1_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "PLAY_TONE_SILENCE_DURATION", "20", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "PLAY_TONE_GAIN", "4", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "SLEEP_VALUE", "10", type, _0_TO_DWORD, false, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber, eProcessConfParty);
	AddParamNotVisible(sec, "IS_COP_SMART_SWITCH_FOR_ASYM_ON", "YES", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "COP_SMART_SWITCH_TOUT", "320", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "IS_COP_SMART_SWITCH_WAIT_DECODER_SYNC", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "IS_COP_DSP_SMART_SWITCH_ON", "YES", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// "PARTY_MONITORING";
	// Default value: YES
	// YES= show party monitoring red fault ('!') indication in MGC manager
	// NO= don't show the above indication
	AddParamNotVisible(sec, "IP_MONITORING_SHOW_FAULTY_MARK", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "PARTY_MONITORING_REFRESH_PERIOD", "2", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "IP_SET_ACTUAL_BITRATE_TO_FLOAT", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// 100 = enables threshhold_bitrate
	// 0 = enables threshhold_bitrate
	AddParamNotVisible(sec, "THRESHOLD_BITRATE", "100", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// 300 = Enables THRESHOLD_LATENCY
	// 0 = Disables THRESHOLD_LATENCY
	AddParamNotVisible(sec, "THRESHOLD_LATENCY", "300", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// 5 = Enables the flags
	// 0 = Disables the flags
	AddParamNotVisible(sec, "THRESHOLD_FRACTION_LOSS", "5", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "THRESHOLD_ACCUMULATE_PACKET_LOSS", "5", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "THRESHOLD_INTERVAL_PACKET_LOSS", "5", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "THRESHOLD_ACCUMULATE_OUT_OF_ORDER", "5", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "THRESHOLD_INTERVAL_OUT_OF_ORDER", "5", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "THRESHOLD_ACCUMULATE_FRAGMENTED", "5", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "THRESHOLD_INTERVAL_FRAGMENTED", "5", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// 80 = Enables THRESHOLD_JITTER
	// 0 = Disables THRESHOLD_JITTER
	AddParamNotVisible(sec, "THRESHOLD_JITTER", "80", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// 200 = Enables THRESHOLD_FRAME_RATE
	// 0 = Disables THRESHOLD_FRAME_RATE
	AddParamNotVisible(sec, "THRESHOLD_FRAME_RATE", "0", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "ENABLE_IP_PARTY_MONITORING", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// "IP_PARTY";
	// enables anyone to choose a specific algorithm to work with (declaration wise).
	AddParamNotVisible(sec, "IP_DEBUG_STATUS", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "CHANGE_MODE_IN_VSW_FIXED", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "IP_UNEXPECTED_MESSAGE", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "IP_RV_TEST_APPLICATION", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "IP_PARTY_CONN_TIME_TILL_SIGNALING", "60", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	// receive values 0 - 128 if its different from those values it is set to zero.
	// 52 is for Ericsson only, the default should be "0"
	AddParamNotVisible(sec, "IP_MOBILE_PHONE_RATE", "0", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	// To identify 3G beyond GW's, like SIP 3G in IMS environsment.
	AddParamNotVisible(sec, "IP_MOBILE_IDENTIFIER", "", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
	// "H323";
	// enable H323 dial out and in simulation for party control and conf levels only without network interfireiance.
	AddParamNotVisible(sec, "H323_PARTY_LEVEL_SIMULATION", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// enable identify cascade in the dial in side, with the name of the conf at the dial out side
	AddParamNotVisible(sec, "ENABLE_H460", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// "SIP";
	AddParamNotVisible(sec, "SIP_REINVITE", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIP_CIF_IN_CP_FROM_RATE", "0", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "SIP_FPS_FIX_TO", "0", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "SIP_USER_AGENT_FPS_FIX", "", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
	AddParamNotVisible(sec, "SIP_USER_AGENT_FLOW_CONTROL", "", type, ONE_LINE_BUFFER_LENGTH, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataString);
	AddParamNotVisible(sec, "SIP_USER_AGENT_FLOW_CONTROL_RATE", "0", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "SIP_RESET_CHANNEL_ON_HOLD", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIP_USER_AGENT_INTRA_INTERVAL", "0", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// enable SIP dial out and in simulation for party control and conf levels
	// only without network interfireiance.
	AddParamNotVisible(sec, "SIP_PARTY_LEVEL_SIMULATION", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H261_StillImageTransmission", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H261", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H263", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H264", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H323_CHECK_ANNEX_D", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H263_ANNEX_F", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "H263_ANNEX_N", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "HIGHEST_COMMON_CUSTOM_FORMATS", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G711", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G711_1", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G722", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G722_Stereo", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	AddParamNotVisible(sec, "G7221_C", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G7231", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G729", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIREN7", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIREN14", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIREN14_Stereo", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIREN22", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SIREN22_Stereo", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G719", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G719_Stereo", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	//  AddParamNotVisible(sec, "RFC2833_DTMF", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "G728", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// "IP_MEDIA";


	// PSTN Debug Flags
	AddParamNotVisible(sec, "SET_NETWORK_TYPE_TO_E1", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// "GATE_KEEPER";
	// enable list to LRQ messages:
	AddParamNotVisible(sec, "GK_PSEUDO_MODE", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// enable moving to alternate gk:
	AddParamNotVisible(sec, "ENABLE_ALT_GK", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	// "EXTERNAL";
	// YES = USERS MAY USE THE LEADER PASSWORD IN EQ TO ACCESS THE CONFERNCE
	// In this case, the NID (Numeric ID) is absolutely ignored. From the Entry Queue, the user should
	// enter the conf-PW or leader-PW. There is no AD-Hoc,There is no External DB, There is no NID
	// NO = the NID the only way to enter to the target conference.The AD-Hoc is active The External DB is active
	// There is no option to enter to the target conference from the Entry Queue via conf-PW or leader-PW
	// AddParamNotVisible(sec, "QUICK_LOG_IN_VIA_ENTRY_QUEUE", "NO", type);,

	// ENABLE/DISABLE ROLL CALL RECORDING CONFIRMATION (DURING IVR).
	AddParamNotVisible(sec, "VISUAL_NAME_CONVENTION", "0", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// IF THIS FLAG IS YES THE MCU WILL FORWARD DTMF TONES OVER CASCADE LINKS
	AddParamNotVisible(sec, "DTMF_FORWARDING", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);

	// Valid values are 0 till 5
	// (0 - no noise detection, 5 - highest sensitivity of noise detection)
	// AddParamNotVisible(sec, "NOISE_LINE_DETECTION", "3", type);,

	// First party in conference hears music until another party connects
	// AddParamNotVisible(sec, "SINGLE_PARTY_HEARS_MUSIC", "YES", type);,

	// The bridge will create new passwords with the above length value
	// Valid values MIN_PASSWORD_LENGTH - MAX_PASSWORD_LENGTH
	// AddParamNotVisible(sec, "DEFAULT_PASSWORD_LENGTH", "4", type);,

	// The bridge will not accept reservations with passwords longer than the above value (through API or DTMF)
	// Valid values 1-16 (should be >= MIN_PASSWORD_LENGTH)
	// AddParamNotVisible(sec, "MAX_PASSWORD_LENGTH", "16", type);,

	// The bridge will not accept reservations with passwords shorter than the above value (through API or DTMF)
	// Valid values 1-16 (should be <= MAX_PASSWORD_LENGTH)
	// AddParamNotVisible(sec, "MIN_PASSWORD_LENGTH", "4", type);,

	// The time we wait between DTMF before we stop the PLC feature
	// Valid values 3-15 (seconds)
	AddParamNotVisible(sec, "PERSONAL_LAYOUT_TIMER", "5", type, _0_TO_BYTE, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);

	// The volume of Entry & Exit Roll Call announcements
	// Valid values 1-10
	AddParamNotVisible(sec, "ROLL_CALL_AUDIO_MIX_PERCENT", "5", type, _0_TO_10_DECIMAL, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataNumber);
	AddParamNotVisible(sec, "KEEP_ALIVE_RECEIVE_PERIOD", "40", type, _1_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "CS_KEEP_ALIVE_RECEIVE_PERIOD", "50", type, _1_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "PERIOD_TRACE", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "MAX_TIME_BETWEEN_PROCESS_CRUSH", "1800", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "MAX_TIME_BETWEEN_TASK_CRUSH", "1800", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "SSH", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "SKIP_NID", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_McaConfParty, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "QAAPI_KEEP_ALIVE_PERIOD", "600", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "QAAPI_RECONNECT_DELAY", "1", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, CFG_KEY_CARDS_MODE_CHANGEABLE, "YES", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "TTL", "60", type, _1_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
	AddParamNotVisible(sec, "OPENSSL_ENC_FUNC", "YES", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, "REPORT_DSP_CRASH", "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);
	AddParamNotVisible(sec, CFG_PRINT_MESSAGE_QUEUE_STATISTICS, "NO", type, _YES_NO, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataBoolean);

	// This Cfg Param Value in seconds defines the number of seconds between each
	// statisics Print the System will do for the Message , Queues , Sizes of
	// each proccess. Zero means No prints, default value = 86400 = 60Sec*60*24 (24 Hours) , this Cfg is
	// internal and only for System Performarnce - replaces the previous timer of "stat" at every 30 Minutes
	AddParamNotVisible(sec, "PROCESSBASE_DUMP_STATISTICS_TIMER_INTERVAL", "86400", type, _0_TO_DWORD, eCfgParamResponsibilityGroup_SwSysInfraApp, eCfgParamDataNumber);
}

void CSysConfig::ConvertMapToPairVector(CCfgPairVector& v) const
{
	CSysMap::iterator it = GetMap()->begin();
	CSysMap::iterator end = GetMap()->end();

	while (end != it)
	{
		const std::string& key = it->first;
		CCfgData* cfgData = it->second;

		v.push_back(new CCfgPair(key, cfgData->GetCounter()));

		++it;
	}
}

///////////////////////////////////////////////////////////////////////////
bool CSysConfig::SwitchCfgFiles(const char* mcmsFile, const char* tmpFile, const char* mcmsFileNoMulSer, const char* tmpFileNoMulSer, bool bAssert)
{
	if ((!(IsFileExists(mcmsFile))) && (!(IsFileExists(tmpFile))))
	{
		if (IsFileExists(tmpFileNoMulSer))
		{
			CopyFile(tmpFileNoMulSer, mcmsFile);
			CopyFile(tmpFileNoMulSer, tmpFile);
			return true;
		}
		else if (IsFileExists(mcmsFileNoMulSer))
		{
			CopyFile(mcmsFileNoMulSer, mcmsFile);
			CopyFile(mcmsFileNoMulSer, tmpFile);
			return true;
		}

		FPASSERT(bAssert);
		return false;
	}

	if (IsFileExists(tmpFile))
		CopyFile(tmpFile, mcmsFile);

	else if (IsFileExists(mcmsFile))
		CopyFile(mcmsFile, tmpFile);

	return false;
}

///////////////////////////////////////////////////////////////////////////
void CSysConfig::SwitchCfgFiles(const char* mcmsFile, const char* tmpFile)
{
	if (IsFileExists(tmpFile))
		CopyFile(tmpFile, mcmsFile);

	else if (IsFileExists(mcmsFile))
		CopyFile(mcmsFile, tmpFile);
}

///////////////////////////////////////////////////////////////////////////
bool CSysConfig::SetMSEnviromentDefaultsIfNeeded()
{
	bool ret = true;
	bool bMSEnviroment = false;

	GetBOOLDataByKey("MS_ENVIRONMENT", bMSEnviroment);

	if (bMSEnviroment)
	{
		// Add the flag for the CS to use
		const char* csSectionName = GetCfgSectionName(eCfgSectionCSModule);
		const eCfgParamType type = eCfgParamUser;

		OverWriteParam("CS_MS_ENVIRONMENT", "YES", csSectionName);

		const char* mcmsSectionName = GetCfgSectionName(eCfgSectionMcmsUser);
		OverWriteParam("SIP_REGISTER_ONLY_ONCE", "YES", mcmsSectionName);

		DWORD mtuSize = 1120;
		GetDWORDDataByKey("MTU_SIZE", mtuSize);

		if (mtuSize > 1120)
			OverWriteParam("MTU_SIZE", "1120", mcmsSectionName);

		ret = LoadFromFile(eCfgParamUser);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////

