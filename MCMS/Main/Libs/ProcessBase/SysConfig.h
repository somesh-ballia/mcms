#ifndef SYS_CONFIG_H__
#define SYS_CONFIG_H__

///////////////////////////////////////////////////////////////////////////
#include "SysConfigBase.h"

///////////////////////////////////////////////////////////////////////////
static const char* GetCfgPath(eCfgParamType type)
{
	static std::string CfgFilePaths[NumOfCfgTypes] =
	{

			(std::string)GET_MCU_HOME_DIR + "/mcms/Cfg/SystemCfgUser.xml",
			(std::string)GET_MCU_HOME_DIR + "/mcms/Cfg/SystemCfgDebug.xml",
		"/mcu_custom_config/custom.cfg"
	};

	const char* path = (eCfgParamUser <= type && type < NumOfCfgTypes) ? CfgFilePaths[type].c_str(): "Invalide_Path";

	return path;
}

///////////////////////////////////////////////////////////////////////////
class CSysConfig : public CSysConfigBase
{
	CLASS_TYPE_1(CSysConfig, CSysConfigBase)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

	virtual void Dump(std::ostream& ostr) const;

	virtual CSerializeObject* Clone()
	{ return new CSysConfig(*this); }

public:

	static bool SwitchCfgFiles(const char* mcmsFile, const char* tmpFile, const char* mcmsFileNoMulSer, const char* tmpFileNoMulSer, bool bAssert);
	static void SwitchCfgFiles(const char* mcmsFile, const char* tmpFile);

public:

	CSysConfig();

	CSysConfig(const CSysConfig&);
	CSysConfig& operator =(const CSysConfig&);

public:

	void DumpByKey(std::ostream&, const string& key) const;
	void DumpByCfgType(std::ostream& ostr, eCfgParamType type, eCfgSections section) const;

	bool GetDataByKey(const std::string& key, std::string& data) const;

	bool GetDWORDDataByKey(const std::string& key, DWORD& data) const;
	bool GetIntDataByKey(const std::string& key, int& data) const;
	bool GetHexDataByKey(const std::string& key, DWORD& data) const;

	bool GetBOOLDataByKey(const std::string& key, bool& data) const;
	bool GetBOOLDataByKey(const std::string& key, BOOL& data) const;

	void FillCnt(std::ostream& answer, int limit) const;

	void SetStartCSLogsParam() const;
	void SetStopCSLogsParam() const;

	bool TakeCfgParam(const char* key, const char* data, const char* section, eCfgParamResponsibilityGroup curGroup);

	bool IsReady() const
	{ return m_isReady; }

	bool ReadUpdate(const std::string& key, CCfgData*& cfgData) const;

	bool OverWriteParam(const std::string& key, const std::string& data, const std::string& section = "") const;

private:

	virtual bool IsSuppressedValidity(const char* key) const;

private:

	void Init();
	void InitMapByDefaults() const;

	void ConvertMapToPairVector(CCfgPairVector& v) const;

	virtual const char* GetFileName(eCfgParamType type)
	{ return GetCfgPath(type); }

	void InitUserParams() const;
	void InitUserJITCParams() const;
	void InitCSModuleParams() const;
	void InitDebugParams() const;
	void InitCustomParams() const;

	bool IsUnderJITCState() const;
	bool SetMSEnviromentDefaultsIfNeeded();

	void PrintToMcuMngrTrace(const string& buff);
	void PrintAllParams() const;

private:

	bool m_isReady;
};

#endif
