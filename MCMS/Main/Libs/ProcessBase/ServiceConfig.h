#ifndef SERVICECONFIG_H_
#define SERVICECONFIG_H_

#include "SysConfigBase.h"



class CObjString;


//static char* ServiceFilePath="Cfg/ServiceCfg.xml";

class CServiceConfig : public CSysConfigBase
{
public:
	CServiceConfig();
	CServiceConfig(const CServiceConfig &other);
	virtual ~CServiceConfig();
	CServiceConfig& operator=( const CServiceConfig& other );

	virtual const char*  NameOf() const {return "CServiceConfig";}
	virtual CSerializeObject* Clone(){return new CServiceConfig(*this);}
/*
	STATUS IsValidOrChanged(CObjString & strError)const;
	void CompleteSectionName();
	DWORD GetNumOfParams()const;*/
	//virtual const char* GetFileName(eCfgParamType type) = 0;
	BOOL GetCSData(const std::string &key, std::string &data)const;

	BOOL GetStrDataByKey(const std::string &key, std::string &data)const;
	BOOL GetDWORDDataByKey(const std::string &key, DWORD &data)const;
	BOOL GetHexDataByKey(const std::string &key, DWORD &data)const;
	BOOL GetBOOLDataByKey(const std::string &key, BOOL &data)const;

	DWORD GetId() const { return m_Id; }
    void SetId(DWORD id){m_Id = id;}
//    void SetServiceName(const char* name);
    DWORD  GetNumOfParams()const;
    virtual const char* GetFileName(eCfgParamType type);
    const char* GetTmpFileName(eCfgParamType type);
    void DeSerialize(CSegment *pSeg);
	void Serialize(WORD format,CSegment *pSeg);
	//SIP_PROXY_IP_PARAMS_S GetProxyParams(){return m_sipProxyIpParamsStruct;}
	void FillMap(CServiceConfig *pServiceConfig);
	void PrintServiceConfig();

protected:
    virtual CSysConfigBase* GetSysConfig();

private:

    BOOL ReadUpdate(const std::string &key, CCfgData *& cfgData)const;
    DWORD m_Id;
//    char  m_serviceName[NET_SERVICE_PROVIDER_NAME_LEN];
    std::string ServiceFilePath;
    std::string m_sFileName;
    std::string m_sFileNameTmp;
	virtual bool TakeCfgParam(const char *key, const char *data, const char *section, const eCfgParamResponsibilityGroup curGroup);
};

#endif /*SERVICECONFIG_H_*/
