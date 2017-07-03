// SysConfig.h: interface for the CSysConfig class.
//
//
//Date         Updated By         Description
//
//27/10/05	  Yuri Ratner		System Configuration Manipulations
//========   ==============   =====================================================================



#ifndef __SYSCONFIG_H__
#define __SYSCONFIG_H__


#include <string>
#include <map>
#include <vector>


#include "PObject.h"
#include "DataTypes.h"
#include "SerializeObject.h"
#include "InitCommonStrings.h"




class CXMLDOMElement;




static const char * STR_WIN_NEW_LINE = "\r\n";

static const char * CFG_STR_YES 		= "YES";
static const char * CFG_STR_NO 			= "NO";
static const char * CFG_LANREDUNDANCY_DEFAULT 			= "NO";
static const BOOL CFG_VALUE_LANREDUNDANCY_DEFAULT 	= NO;

enum eCfgSections
{
	eCfgSectionMcmsUser,
	eCfgSectionMcmsDebug,
	eCfgSectionCSModule,

	NumOfCfgSections
};

static const char *GetCfgSectionName(eCfgSections sectionType)
{
    static const char * CfgSectionNames [] =
        {
            "MCMS_PARAMETERS_USER"	,
            "MCMS_PARAMETERS_DEBUG"	,
            "CS_MODULE_PARAMETERS"
        };

	const char *name = (eCfgSectionMcmsUser <= sectionType && sectionType < NumOfCfgSections
						?
						CfgSectionNames[sectionType] : "Invalid");
	return name;
}



struct CCfgPair;
typedef std::vector<CCfgPair*> CCfgPairVector;

class CCfgData;
typedef std::map<std::string, CCfgData*> CSysMap;

class CCfgData;
typedef vector<const CCfgData*> CCfgParamsVector;


enum eCfgParamType
{
	eCfgParamUser = 0,
	eCfgParamDebug,
	eCfgCustomParam,
	NumOfCfgTypes
};

enum eCfgParamDataType
{
	eCfgParamDataDefault = 0,
	eCfgParamDataBoolean,
	eCfgParamDataString,
	eCfgParamDataNumber,
	eCfgParamDataEnum,
	eCfgParamDataIpAddress,

	NumOfCfgDataTypes
};

#include <iostream>
static const char* GetCfgParamDataTypeName(eCfgParamDataType type)
{
    static char* CfgParamDataTypeNames [] =
        {
            "Default"	,
            "Boolean"	,
            "String"	,
            "Number"	,
	    "Enum"	,
            "IpAddress"
        };

    const DWORD namesLen = sizeof(CfgParamDataTypeNames) / sizeof(CfgParamDataTypeNames[0]);
	const char *name = (namesLen > (DWORD)type
						?
						CfgParamDataTypeNames[type] : "Invalid_Data_Type");
	return name;
}


enum eCfgParamVisibilityType
{
	eCfgParamVisible,
	eCfgParamNotVisible,
	eCfgParamInFileNonVisible
};

static const char* GetCfgParamVisibilityType(eCfgParamVisibilityType type)
{
    static char* CfgParamVisibilityTypeNames [] =
        {
            "Visible",
            "NonVisible",
            "NonVisibleSerialized"
        };

    const DWORD namesLen = sizeof(CfgParamVisibilityTypeNames) / sizeof(CfgParamVisibilityTypeNames[0]);
	const char *name = (namesLen > (DWORD)type
						?
						CfgParamVisibilityTypeNames[type] : "Invalid_Visibility_Type");
	return name;
}


enum eCfgParamResponsibilityGroup
{
	eCfgParamResponsibilityGroup_Illegal = 0,
	eCfgParamResponsibilityGroup_SwSysInfraApp,
	eCfgParamResponsibilityGroup_SwSysIp,
	eCfgParamResponsibilityGroup_McaConfParty,
	eCfgParamResponsibilityGroup_McaResource,

	MAX_NUM_OF_CFG_PARAM_RESPONSIBILITY_GROUPS
};

static const char* GetCfgParamResponsibilityGroupName(eCfgParamResponsibilityGroup group)
{
    static char* CfgParamResponsibilityGroupNames [] =
        {
    		"Illegal_Group",
            "SwSys_Infra&App",
            "SwSys_Ip",
            "MCA_ConfParty",
            "MCA_Resource"
        };

	const char *name = ( MAX_NUM_OF_CFG_PARAM_RESPONSIBILITY_GROUPS > group
						 ?
						 CfgParamResponsibilityGroupNames[group] : "Invalid_Responsibility_Group_Name" );
	return name;
}




/*-------------------------------------------------------------------------------
	CCfgData used in CFG Map. This is a main structure for CFG parameters
-------------------------------------------------------------------------------*/
class CCfgData : public CPObject
{
CLASS_TYPE_1(CCfgData, CPObject)
public:
    CCfgData(const std::string &section, const std::string &key, const std::string &data,
            eCfgParamType cfgType, int cfgTypeValidator, eCfgParamDataType cfgDataType,
            eCfgParamVisibilityType cfgParamVisibilityType,
            eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
            int counter = 0);

    CCfgData(const std::string &section, const std::string &key, const std::string &data,
                eCfgParamType cfgType, int cfgTypeValidator, bool isReset, eCfgParamDataType cfgDataType,
                eCfgParamVisibilityType cfgParamVisibilityType,
                eCfgParamResponsibilityGroup cfgParamResponsibilityGroup, eProcessType processType, bool isAllProcesses,
                int counter = 0);

    CCfgData(const CCfgData &other);
    virtual ~CCfgData(){}
    virtual void Dump(std::ostream &ostr) const;
    virtual const char* NameOf() const { return "CCfgData";}

    DWORD IncrementCnt(){m_Counter++; return m_Counter;}

    const std::string &	GetSection		()const	{return m_Section;	}
    const std::string &	GetKey			()const	{return m_Key;		}
    const std::string &	GetData			()const	{return m_Data;		}
    DWORD 				GetCounter		()const	{return m_Counter;	}
    eCfgParamType		GetCfgType		()const	{return m_CfgType;	}
    int 				GetTypeValidator()const {return m_cfgTypeValidator; }
    bool				GetIsReset		()const {return m_IsReset;}
    eCfgParamDataType	GetCfgDataType	()const	{return m_CfgParamDataType;	}
    eCfgParamVisibilityType GetCfgParamVisibilityType()const {return m_CfgParamVisibilityType;}
    eCfgParamResponsibilityGroup GetCfgParamResponsibilityGroup()const {return m_CfgParamResponsibilityGroup;}
    eProcessType		GetProcessType	()const {return m_ProcessType; }
    bool				GetIsAllProcesses() const {return m_IsAllProcesses;}



    void SetSection(std::string section)  	{m_Section = section;	}
//	void SetKey(std::string key)  	  		{m_Key = key;			}
    void SetData(std::string data)  	  	{m_Data = data;			}
    void SetIsReset(bool b)					{m_IsReset = b;			}
    void SetProcessType(eProcessType process)	{m_ProcessType = process;}
    void SetTypeValidator(int val){m_cfgTypeValidator = val;}
    void SetCfgParamVisibilityType(eCfgParamVisibilityType val){m_CfgParamVisibilityType = val;}
    static bool TestValidity(const CCfgData *obj);

private:
    CCfgData& operator=(const CCfgData &other);

    std::string 		m_Section;
    const std::string 	m_Key;
    std::string 		m_Data;
    DWORD 				m_Counter;
    int					m_cfgTypeValidator; // see initCommonStrings.h
    bool				m_IsReset;
    const eCfgParamType 				m_CfgType;
    const eCfgParamDataType 			m_CfgParamDataType;
    eCfgParamVisibilityType 		m_CfgParamVisibilityType;
    const eCfgParamResponsibilityGroup	m_CfgParamResponsibilityGroup;
    eProcessType 						m_ProcessType;
    bool								m_IsAllProcesses;

};

static bool SysCfgEntryCmpBySection (const CCfgData *elem1, const CCfgData *elem2 )
{
   return elem1->GetSection() > elem2->GetSection();
}

/*-------------------------------------------------------------------------------
	CSysConfigBase : Base clas for SysConfig and SysConfigEma
-------------------------------------------------------------------------------*/
class CSysConfigBase : public CSerializeObject
{
CLASS_TYPE_1(CSysConfigBase,CSerializeObject)
public:
    static bool IsUnderJITCState(void);

	CSysConfigBase();
	CSysConfigBase(const CSysConfigBase&);
	virtual ~CSysConfigBase();
	virtual const char* NameOf() const { return "CSysConfigBase";}
	CSysConfigBase& operator=( const CSysConfigBase& other );

	virtual void SerializeXml(CXMLDOMElement*& pXMLRootElement) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pXMLRootElement,char *pszError,const char* action);

	STATUS CheckSpecialVerification(const string &key, const string &data);
	STATUS VerifyMcuDispName(const char *data, int len);

	bool LoadFromFile(eCfgParamType type);
	void SaveToFile(eCfgParamType type);
	void SaveToFile(const char *fileName);

	void RemoveParamsInFileNonVisible();

	bool IsParamSupportedInJitcMode(const std::string &key)const;
	bool IsParamExist(const std::string &key)const;

	bool IsActiveAlarmErrorExist()const{return (m_ActiveAlarmErrorBuf[0] != '\0');}
	const char *GetActiveAlarmErrorMsg()const{return m_ActiveAlarmErrorBuf;}

	bool IsFaultErrorExist()const{return (m_FaultErrorBuf[0] != '\0');}
	const char *GetFaultErrorMsg()const{return m_FaultErrorBuf;}

	BOOL GetAllParamsBySection(eCfgSections eSection, CCfgParamsVector &keys);

	CCfgData *GetCfgEntryByKey(const string &key)const
	{
		CSysMap::iterator it = m_Map->find(key);
		return (it != m_Map->end()) ? it->second : NULL;
	}

	eCfgParamType GetCfgParamTypeState()const{return m_eCfgParamType;}
	void SetCfgParamTypeState(eCfgParamType type){m_eCfgParamType = type;}

    void SetIsSerializeVisibleOnly(bool val){m_SerializeVisibleOnly = val;}
  	bool GetIsSerializeVisibleOnly()const {return m_SerializeVisibleOnly;}
  	void SetIsServiceCFG(bool val){m_IsServiceCFG = val;}
  	CSysMap *GetMap()const{return m_Map;}

	void AddParamInFileNonVisible(const std::string &section,
							const std::string &key,
							const std::string &data,
							eCfgParamType cfgType,
							int validatorType,
							eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
							eCfgParamDataType dataType = eCfgParamDataString)const;

	void AddParamVisible(const std::string &section,
						 const std::string &key,
						 const std::string &data,
						 eCfgParamType cfgType,
						 int validatorType,
						 eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
						 eCfgParamDataType dataType = eCfgParamDataString)const;


protected:
	void AddParamNotVisible(const std::string &section,
							const std::string &key,
							const std::string &data,
							eCfgParamType cfgType,
							int validatorType,
							eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
							eCfgParamDataType dataType = eCfgParamDataString)const;

	void AddParamNotVisible(const std::string &section, const std::string &key,const std::string &data,
							 eCfgParamType cfgType, int validatorType, bool isReset,
							 eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
							 eCfgParamDataType dataType,
							 eProcessType processType,
							 bool isAllProcesses = false)const;


	void AddParamVisible(const std::string &section,
							 const std::string &key,
							 const std::string &data,
							 eCfgParamType cfgType,
							 int validatorType,
							 bool isReset,
							 eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
							 eCfgParamDataType dataType = eCfgParamDataString,
							 eProcessType processType = eProcessTypeInvalid,
							 bool isAllProcesses = false)const;


	void ConvertMapToVector(CCfgParamsVector &vect)const;

	virtual const char* GetFileName(eCfgParamType type) = 0;
	virtual bool TakeCfgParam(const char *key, const char *data, const char *section, const eCfgParamResponsibilityGroup curGroup) = 0;
	virtual CSysConfigBase* GetSysConfig();
	eCfgParamDataType GetCfgDataTypeByData(const string &data)const;
	CSysMap *m_Map;

	void AddParam(const std::string &section,
					  const std::string &key,
					  const std::string &data,
					  eCfgParamType cfgType,
					  int validatorType,
					  eCfgParamDataType dataType,
					  eCfgParamVisibilityType cfgParamVisibilityType,
					  eCfgParamResponsibilityGroup cfgParamResponsibilityGroup)const;


	void AddParam(const std::string &section,
						  const std::string &key,
						  const std::string &data,
						  eCfgParamType cfgType,
						  int validatorType,
						  bool isReset,
						  eCfgParamDataType dataType,
						  eCfgParamVisibilityType cfgParamVisibilityType,
						  eCfgParamResponsibilityGroup cfgParamResponsibilityGroup,
						  eProcessType processType,
						  bool isAllProcesses)const;


    virtual bool IsSuppressedValidity(const char* key) const;

private:
	void MapFree();
	bool IsIpAddress4(const char *strTested)const;
	bool IsBoolean(const char *strTested)const;

    bool m_SerializeVisibleOnly;
    bool m_IsServiceCFG;
	eCfgParamType m_eCfgParamType;

	char m_ActiveAlarmErrorBuf[ERROR_MESSAGE_LEN];
	char m_FaultErrorBuf[ERROR_MESSAGE_LEN];
};

#endif // !defined(_SYSCONFIG_H__)
