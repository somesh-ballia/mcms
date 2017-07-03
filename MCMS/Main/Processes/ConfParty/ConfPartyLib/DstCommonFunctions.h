#include <map>
#include <string>
#include <list>
#include "DstXml.h"
#include "DstXmlWriter.h"
#include "DstXmlParser.h"

using namespace std;

typedef map<string,string> strmap;

#define PACK_LEN	1024 * 1024	

class CDstCommonFunctions
{
public:
	
	CDstCommonFunctions(void);
	~CDstCommonFunctions(void);
	
	string itoa(int n);
	string boolToString(bool b);
	bool stringToBool(string str);
	string	FindValueByKeyFromString(const string szString, const string szKey);
	bool XmlMsgToMapById(const char* szFile, const char* szMsgId, const char* szMsgName, strmap& mp);
	bool ParseFormatedStringToMap(const string& szText, strmap& strmap);
	bool ParseMapToFormatedString(strmap &mpSrc, string &szText);
	void ReadXmlFileToMap(string szFileName, map<string, strmap>& mpConfig);

	void GetMapFiledValue(strmap &mapDealing, string szFiled);
	string LoadFromFile(string fileName);
	bool ParseFormatedStringToList(const string& szText, list<string>& lst);
	string GetValueOfKeyFromFormatedString(const string& szText, const string& szKey);
	string LoadFromFileToXml(string fileName);
	bool StrmapToXmlMsg(strmap& mp, CDstXmlMessage* pMsg);
	bool XmlMsgToMap(CDstXmlMessage* pMsg, strmap& mp);
	bool XmlMsgToMap(const char* szFile, const char* szMsgName,const char* szMsgId,strmap& mp);
	bool XmlMsgToMap(const char* szFile,const char* szMsgId, strmap& mp);

	
	string MapToXmlStr(strmap& mp, string szRootName="ROOT", string szRootVer="1.0.0.0");

	void ChangeStringFromSubString(string &strTotal,string key,string strSub);



	
	string xml_MapToXmlStr(strmap& mp, string& szRootName, string& szRootVer);
	bool xml_XmlStrToMap(string szXml, strmap& mp);
	
	
	string	m_szLangFlag;
	
};
