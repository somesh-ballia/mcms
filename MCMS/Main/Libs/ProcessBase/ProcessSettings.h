#ifndef __PROJECT_SETTINGS_H__
#define __PROJECT_SETTINGS_H__

#include <map>
#include <string>
using namespace std;

#include "SerializeObject.h"




typedef map<const string , string> CSettingMap;






class CProcessSettings : public CSerializeObject
{
public:
    CProcessSettings();
    virtual ~CProcessSettings();
    virtual CSerializeObject* Clone(){return new CProcessSettings(*this);}
    
	virtual const char* NameOf() const { return "CProcessSettings";}

    bool IsParamExist(const string &key)const;
    
    void SetSetting(const string & key, const string & data, bool isSaveToFile = true);
    void SetSettingDWORD(const string & key, DWORD data, bool isSaveToFile = true);
    void SetSettingBOOL(const string & key, BOOL data, bool isSaveToFile = true);
    
    bool GetSetting(const string & key, string & data);
    bool GetSettingDWORD(const string & key, DWORD & data);
    bool GetSettingBOOL(const string & key, BOOL & data);
    
    bool RemoveSetting(const string & key, bool isSaveToFile = true);
    
    STATUS WriteToFile();
    STATUS LoadFromFile();
    STATUS DeleteFile();


    
private:
    // disabled
    CProcessSettings& operator = (const CProcessSettings&);
    
    virtual void SerializeXml(CXMLDOMElement*& pXMLRootElement) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pXMLRootElement,char *pszError,const char* action);
    void CreateFileName(string &fileName)const;
    void CreateMainNodeName(string &mainNodeName)const;
    const char * GetSpecificProcessName()const;
    const string& GetFileName()const;
    const string& GetMainXmlNodeName()const;
        
    CSettingMap m_SettingMap;
    string m_FileName;
    string m_MainXmlNodeName;
};





#endif // !defined(__PROJECT_SETTINGS_H__)
