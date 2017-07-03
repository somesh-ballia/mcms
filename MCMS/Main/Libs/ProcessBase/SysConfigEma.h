#ifndef SYSCONFIGEMA_H_
#define SYSCONFIGEMA_H_

#include "SysConfigBase.h"



class CObjString;



static const char* GetEmaCfgPath(eCfgParamType type)
{
    static char* EmaFilePaths [] =
        {
            "Cfg/SystemCfgUserTmp.xml",
            "Cfg/SystemCfgDebugTmp.xml",
            "/mcu_custom_config/custom.cfg"
        };
	const char *path = ((eCfgParamUser <= type) && (type < NumOfCfgTypes)
						?
						EmaFilePaths[type] : "Invalide_Tmp_Path");
	return path;
}


static const char* GetCfgTypeName(eCfgParamType type)
{
    static char* EmaFileTypeNames [] =
        {
            "user" ,
            "debug",
            "custom"
        };
	const char *name = ((eCfgParamUser <= type) && (type <= eCfgParamDebug)
						?
						EmaFileTypeNames[type] : "Invalide_File_Type");
	return name;
}




class CSysConfigEma : public CSysConfigBase
{
public:
	CSysConfigEma();
	CSysConfigEma(const CSysConfigEma &rHnd);
	virtual ~CSysConfigEma();

	virtual const char* NameOf() const { return "CSysConfigEma";}
	virtual CSerializeObject* Clone(){return new CSysConfigEma(*this);}

	STATUS IsValidOrChanged(CObjString & strError)const;
	STATUS CheckIfKeyChanged(const std::string key, std::string &data) const;
	void   CompleteSectionName();
	DWORD  GetNumOfParams()const;

private:
	virtual const char* GetFileName(eCfgParamType type) {return GetEmaCfgPath(type);}
	virtual bool TakeCfgParam(const char *key, const char *data, const char *section, const eCfgParamResponsibilityGroup curGroup);
};

#endif /*SYSCONFIGEMA_H_*/
