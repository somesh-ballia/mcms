#include <stdlib.h>
#include <stdio.h>
#include "ProcessSettings.h"
#include "ProcessBase.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ObjString.h"
#include "ApiStatuses.h"

const char *STR_BOOL_TRUE = "YES";
const char *STR_BOOL_FALSE = "NO";


bool atob(const  string & str, BOOL & outData)
{
    bool res = true;
    if(STR_BOOL_TRUE == str)
    {
        outData = TRUE;
    }
    else if(STR_BOOL_FALSE == str)
    {
        outData = FALSE;
    }
    else
    {
        res = false;
    }
    
    return res;
}

void btoa(BOOL bVal, string & outStrVal)
{
    outStrVal = (TRUE == bVal ? STR_BOOL_TRUE : STR_BOOL_FALSE);
}

void itoa(DWORD iVal, string & outStrVal)
{
    char buffer [32];
    sprintf(buffer, "%d", iVal);
    outStrVal = buffer;
}







//////////////////////////////////////////////////////////////////////
CProcessSettings::CProcessSettings()
{
    CreateFileName(m_FileName);
    CreateMainNodeName(m_MainXmlNodeName);
    
    LoadFromFile();
}


//////////////////////////////////////////////////////////////////////
CProcessSettings::~CProcessSettings()
{
    WriteToFile();
}

//////////////////////////////////////////////////////////////////////
void CProcessSettings::SetSetting(const string & key, const string & data, bool isSaveToFile)
{
    m_SettingMap[key] = data;
 
    if(isSaveToFile)
    {
        WriteToFile();
    }
}

//////////////////////////////////////////////////////////////////////
void CProcessSettings::SetSettingDWORD(const string & key, DWORD data, bool isSaveToFile)
{
    string strData;
    itoa(data, strData);
    SetSetting(key, strData, isSaveToFile);
}

//////////////////////////////////////////////////////////////////////
void CProcessSettings::SetSettingBOOL(const string & key, BOOL data, bool isSaveToFile)
{
    string strData;
    btoa(data, strData);
    SetSetting(key, strData, isSaveToFile);
}

//////////////////////////////////////////////////////////////////////
bool CProcessSettings::GetSetting(const string & key, string & data)
{
    bool isExist = IsParamExist(key);
    if(false == isExist)
    {
        return false;
    }
    data = m_SettingMap[key];
   
    return true;
}

//////////////////////////////////////////////////////////////////////
bool CProcessSettings::GetSettingDWORD(const string & key, DWORD & data)
{
    bool isExist = IsParamExist(key);
    if(false == isExist)
    {
        return false;
    }
    const string & tmp = m_SettingMap[key];
    data = atoi(tmp.c_str());
    return true;
}

//////////////////////////////////////////////////////////////////////
bool CProcessSettings::GetSettingBOOL(const string & key, BOOL & data)
{
    bool isExist = IsParamExist(key);
    if(false == isExist)
    {
        return false;
    }
    const string & tmp = m_SettingMap[key];
    bool res = atob(tmp.c_str(), data);
    return res;
}

//////////////////////////////////////////////////////////////////////
bool CProcessSettings::RemoveSetting(const string & key, bool isSaveToFile)
{
    bool isExist = IsParamExist(key);
    if(false == isExist)
    {
        return false;
    }
    m_SettingMap.erase(key);
    
    if(isSaveToFile)
    {
      DeleteFile();
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
STATUS CProcessSettings::WriteToFile()
{
    string fileName = GetFileName();
    STATUS status = WriteXmlFile(fileName.c_str());
    return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CProcessSettings::LoadFromFile()
{
    string fileName = GetFileName();
    STATUS status = ReadXmlFile(fileName.c_str());
    return status;
}


//////////////////////////////////////////////////////////////////////
STATUS CProcessSettings::DeleteFile()
{
    string fileName = GetFileName();
 
    if ( unlink(fileName.c_str()) != 0 )
         {
                PTRACE(eLevelError,"CProcessSettings::DeleteFile unlink fails");
                PASSERT(1);              
                return STATUS_FAIL;
         }
   return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
const string& CProcessSettings::GetFileName()const
{
    return m_FileName;
}

//////////////////////////////////////////////////////////////////////
const string& CProcessSettings::GetMainXmlNodeName()const
{
    return m_MainXmlNodeName;
}

//////////////////////////////////////////////////////////////////////
const char * CProcessSettings::GetSpecificProcessName()const
{
    CProcessBase *pProcess = CProcessBase::GetProcess();
    eProcessType processType = pProcess->GetProcessType();
    const char *processName = CProcessBase::GetProcessName(processType);
    return processName;
}

//////////////////////////////////////////////////////////////////////
void CProcessSettings::CreateMainNodeName(string &mainNodeName)const
{
    mainNodeName = GetSpecificProcessName();
    mainNodeName += "_Settings";
}

//////////////////////////////////////////////////////////////////////
void CProcessSettings::CreateFileName(string &fileName)const
{
    fileName = "Cfg/";
    fileName += GetSpecificProcessName();
    fileName += "_Settings.xml";
}

//////////////////////////////////////////////////////////////////////
void CProcessSettings::SerializeXml(CXMLDOMElement*& pXMLRootElement) const
{
    const string &mainNodeName = GetMainXmlNodeName();  
    if(NULL == pXMLRootElement)
	{
		pXMLRootElement =  new CXMLDOMElement();
		pXMLRootElement->set_nodeName(mainNodeName.c_str());
	}
	else
	{
		pXMLRootElement = pXMLRootElement->AddChildNode(mainNodeName.c_str());
	}

    CSettingMap::const_iterator iCurrent = m_SettingMap.begin();
    CSettingMap::const_iterator iEnd = m_SettingMap.end();
    
    for(; iCurrent != iEnd ; iCurrent++)
    {
        const string & key = iCurrent->first;
        const string & data = iCurrent->second;

        CXMLDOMElement *pPairElement = pXMLRootElement->AddChildNode("SETTING_PAIR");
        
        pPairElement->AddChildNode("KEY" , key.c_str());
		pPairElement->AddChildNode("DATA", data.c_str());
    }
}

//////////////////////////////////////////////////////////////////////
int CProcessSettings::DeSerializeXml(CXMLDOMElement *pXMLRootElement,char *pszError,const char* action)
{
    int nStatus = STATUS_OK;

    char buffKey			[256];
	char buffData			[256];

    CXMLDOMElement 	*pPairElement = NULL;
    GET_FIRST_CHILD_NODE(pXMLRootElement, "SETTING_PAIR", pPairElement);
    while(NULL != pPairElement)
    {
        GET_VALIDATE_CHILD(pPairElement, "KEY", buffKey	, ONE_LINE_BUFFER_LENGTH);
        GET_VALIDATE_CHILD(pPairElement, "DATA", buffData	, ONE_LINE_BUFFER_LENGTH);
        
        SetSetting(buffKey, buffData, false);
        
        GET_NEXT_CHILD_NODE(pXMLRootElement, "SETTING_PAIR", pPairElement);
    }
    return nStatus;
}

//////////////////////////////////////////////////////////////////////
bool CProcessSettings::IsParamExist(const string &key)const
{
	return m_SettingMap.end() != m_SettingMap.find(key);
}
