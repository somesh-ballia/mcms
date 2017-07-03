#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "DstCommonFunctions.h"
#include "Macros.h"



CDstCommonFunctions::CDstCommonFunctions()
{
}

//////////////////////////////////////////////////////////////////////////////////////
CDstCommonFunctions::~CDstCommonFunctions()
{
}

//////////////////////////////////////////////////////////////////////////////////////
string CDstCommonFunctions::itoa(int n)
{
	char szTmp[50];
	snprintf(szTmp, sizeof(szTmp), "%d", n);
	return szTmp;
}
//////////////////////////////////////////////////////////////////////////////////////
string CDstCommonFunctions::boolToString(bool b)
{
	if (b == true)
		return "true";
	else
		return "false";
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::stringToBool(string str)
{
	if (str == "true")
		return true;
	else
		return false;
}
//////////////////////////////////////////////////////////////////////////////////////
void CDstCommonFunctions::ChangeStringFromSubString(string &strTotal,string key,string strSub)
{
	string::size_type itbegin = strTotal.find(key);
	if(itbegin!=string::npos)
	{
		itbegin += key.length() +2 ;
		string::size_type itEnd = strTotal.find("\'",itbegin);

		if(itEnd!=string::npos)
		{
			strTotal.replace(itbegin, itEnd-itbegin, strSub);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////
string	CDstCommonFunctions::FindValueByKeyFromString(const string szString, const string szKey)
{
	string::size_type	sizePos;
	string				szFind;
	string				szSourceStr;

	szSourceStr = szString;
	szFind = szKey + "=";
	sizePos = szSourceStr.find(szFind);
	if (sizePos != string::npos)
	{
		szSourceStr.erase(0, sizePos + szFind.length());
		sizePos = szSourceStr.find(";");
		if (sizePos != string::npos)
		{
			szSourceStr.erase(sizePos);
		}
		else
		{
			szSourceStr = "";
		}
	}
	else
	{
		szSourceStr = "";
	}

	return szSourceStr;
}
//////////////////////////////////////////////////////////////////////////////////////
void CDstCommonFunctions::GetMapFiledValue(strmap &mapDealing, string szFiled)
{
	strmap::iterator	it;
	string::size_type	iPos;
	//string				szFieldStr = szFiled + "=\"";
	string				szFieldStr = szFiled + "='";

	for (it=mapDealing.begin(); it!=mapDealing.end(); it++)
	{
		string &szVal = it->second;
		iPos = szVal.find(szFieldStr);
		if(iPos != string::npos)
		{
			szVal.erase(0, iPos + szFieldStr.length());
			//iPos = szVal.find("\"|");
			iPos = szVal.find("'|");
			if (iPos != string::npos)
			{
				szVal.erase(iPos);
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////
string CDstCommonFunctions::LoadFromFile(string fileName)
{
	ifstream	f;
	string		len="", result="";
	f.open(		fileName.c_str());

	if(f.is_open())
	{
		while(!f.eof())
		{
			getline(f, len);
			result += len;
		}
		f.close();
	}

	return result;
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::XmlMsgToMapById(const char* szFile, const char* szMsgId, const char* szMsgName, strmap& mp)
{
	string szFull = LoadFromFile(szFile);

	int length = szFull.length();

	if (szFull.length()>0)
	{
		CDstXmlParser parse;
		if (DST_XML_FAILED != parse.Parse(szFull))
		{
			for (int i=0; i<parse.GetMessageNum(); i++)
			{
				CDstXmlMessage* pMsg = parse.GetMessageEx(i);
				if (pMsg && 0 == strcmp(szMsgId, pMsg->GetId()))
				{
					if (0 != strlen(szMsgName))
					{
						if (0 == strcmp(szMsgName, pMsg->GetType()))
						{
							XmlMsgToMap(pMsg, mp);
							return true;
						}
					}
					else
					{
						XmlMsgToMap(pMsg, mp);
						return true;
					}
				}
			}
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::ParseFormatedStringToList(const string& szText, list<string>& lst)
{
	if(szText.size()==0)		return false;

	string szVal = szText;
	while(1)
	{
		string::size_type it = szVal.find("|", 0);
		if(it != string::npos)		
		{
			string szValue = szVal.substr(0,it);

			lst.push_back(szValue);

			szVal.erase(0,it+1);
		}
		else
		{
			break;
		}

	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////
string CDstCommonFunctions::GetValueOfKeyFromFormatedString(const string& szText, const string& szKey)
{
	strmap mapText;

	ParseFormatedStringToMap(szText, mapText);
	return mapText[szKey];
}
//////////////////////////////////////////////////////////////////////////////////////
string CDstCommonFunctions::LoadFromFileToXml(string fileName)
{
	ifstream	f;
	string		len="", result="";
	f.open(		fileName.c_str());

	if(f.is_open())
	{
		while(!f.eof())
		{
			getline(f, len);
			result += len;
		}
		f.close();
	}

	return result;
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::StrmapToXmlMsg(strmap& mp, CDstXmlMessage* pMsg)
{
	char szKey[200], szValue[32000];

	for(strmap::iterator it=mp.begin(); it!= mp.end(); it++)
	{
		snprintf(szKey, sizeof(szKey), "%s", it->first.c_str());
		snprintf(szValue, sizeof(szValue), "%s", it->second.c_str());

		if(strcmp(szKey, "_xml_msg_name")==0)
		{
			pMsg->SetType(szValue);
		}
		else if(strcmp(szKey, "_xml_msg_id")==0)
		{
			pMsg->SetId(szValue);
		}
		else
		{
			CDstXmlNode* pNode = pMsg->AddNode();
			if(pNode)
			{
				pNode->SetName(szKey);
				pNode->SetValue(szValue);
			}
			else
			{
				FPASSERTMSG(1, "AddNode return NULL"); 
			}
		}
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::XmlMsgToMap(CDstXmlMessage* pMsg, strmap& mp)
{
	char* szId = pMsg->GetId();
	mp["_xml_msg_id"] = szId==NULL ? "":szId;

	char* szName = pMsg->GetType();
	mp["_xml_msg_name"] = szName;

	for(int i=0; i<pMsg->GetNodeNum(); i++)
	{
		CDstXmlNode* pNode = pMsg->GetNode(i);
		if( pNode )
			mp[pNode->GetName()] = pNode->GetValue();
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////
void CDstCommonFunctions::ReadXmlFileToMap(string szFileName, map<string, strmap>& mpConfig)
{
	string szConfig =	LoadFromFile(szFileName);

	if(szConfig.length()>0)	
	{
		CDstXmlParser parse;
		int nResult			= parse.Parse(szConfig);
		if(nResult			==DST_XML_FAILED)	
		{
			//AfxMessageBox(FormatString("Parse file failed(%s)!",szFileName.c_str()).c_str());
			exit(0);
		}

		for(int i=0; i<parse.GetMessageNum(); i++)
		{
			strmap mp;
			CDstXmlMessage* pMsg = parse.GetMessageEx(i);
			if( pMsg )
			{
				XmlMsgToMap(pMsg, mp);
				mpConfig[pMsg->GetId()] = mp;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::XmlMsgToMap(const char* szFile,  const char* szMsgName, const char* szMsgId,strmap& mp)
{
	string szFull = LoadFromFile(szFile);

	int length = szFull.length();

	if (szFull.length()>0)
	{
		CDstXmlParser parse;
		if (DST_XML_FAILED != parse.Parse(szFull))
		{
			for (int i=0; i<parse.GetMessageNum(); i++)
			{
				CDstXmlMessage* pMsg = parse.GetMessageEx(i);
				if (pMsg && 0 == strcmp(szMsgId, pMsg->GetId()))
				{
					if (0 != strlen(szMsgName))
					{
						if (0 == strcmp(szMsgName, pMsg->GetType()))
						{
							XmlMsgToMap(pMsg, mp);
							return true;
						}
					}
					else
					{
						XmlMsgToMap(pMsg, mp);
						return true;
					}
				}
			}
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::XmlMsgToMap(const char* szFile,const char* szMsgID, strmap& mp)
{
	string szFull = LoadFromFile(szFile);

	int length = szFull.length();

	if (szFull.length()>0)
	{
		CDstXmlParser parse;
		if (DST_XML_FAILED != parse.Parse(szFull))
		{
			for (int i=0; i<parse.GetMessageNum(); i++)
			{
				CDstXmlMessage* pMsg = parse.GetMessageEx(i);

				if (0 != strlen(szMsgID))
				{
					//if (0 == strcmp(szMsgName, pMsg->GetType()))
					if (pMsg && 0 == strcmp(szMsgID, pMsg->GetId()))
					{
						XmlMsgToMap(pMsg, mp);
						return true;
					}
				}
			}
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::ParseFormatedStringToMap(const string& szText, strmap& strmap)
{
	if(szText.size()==0)		return false;

	string szVal = szText;
	while(1)
	{
		string::size_type it = szVal.find("|", 0);
		if(it != string::npos)		
		{
			string::size_type it2 = szVal.find("=", 0);
			if(it2 != string::npos)
			{
				string szKey = szVal.substr(0, it2);

				//				string::size_type it3 = szVal.find("\"", it2);
				string::size_type it3 = szVal.find("'", it2);
				if(it3 != string::npos)
				{
					//					string::size_type it4 = szVal.find("\"", it3+1);
					string::size_type it4 = szVal.find("'", it3+1);
					if(it4!=string::npos && it4<it)
					{
						string szValue = szVal.substr(it3+1, it4-it3-1);
						strmap[szKey] = szValue;

						szVal.erase(0, it+1);
						continue;
					}
				}
			}
		}

		break;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::ParseMapToFormatedString(strmap &mpSrc, string &szText)
{
	strmap::iterator				it;

	if (0 == mpSrc.size())
	{
		return false;
	}

	szText = "";
	for (it=mpSrc.begin(); mpSrc.end()!=it; it++)
	{
		szText = szText + it->first + "='" + it->second + "'|";
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////
string CDstCommonFunctions::MapToXmlStr(strmap& mp, string szRootName, string szRootVer)
{
	CDstXmlMessage msg;
	StrmapToXmlMsg(mp, &msg);

	CDstXmlWriter wrt;
	wrt.CloneMessage(&msg);
	wrt.SetVersion("1.0.0.0");
	wrt.SetProtocol("TVUI_API");
//	char szTmp[200];
//	sprintf(szTmp,"%s",szRootVer.c_str());
//	wrt.SetVersion(szTmp);

//	sprintf(szTmp,"%s",szRootName.c_str());
//	wrt.SetProtocol(szTmp);

//	char buffer[PACK_LEN]; 
//	memset(buffer,0,PACK_LEN);
//	int len = PACK_LEN;
	int len = PACK_LEN;
	ALLOCBUFFER(buffer,PACK_LEN)
	memset(buffer,'\0',PACK_LEN);

	int nResult	= wrt.Write(buffer, len);
	//if(nResult	== DST_XML_FAILED)		
	//{
	//	return false;
	//}
	string res(buffer);
	DEALLOCBUFFER(buffer);
	
	return res;
}
//////////////////////////////////////////////////////////////////////////////////////
string CDstCommonFunctions::xml_MapToXmlStr(strmap& mp, string& szRootName, string& szRootVer)
{
	CDstXmlMessage* msg = new CDstXmlMessage;
	StrmapToXmlMsg(mp, msg);

	CDstXmlWriter wrt;
	wrt.CloneMessage(msg);
	//wrt.SetVersion("1.0.0.0");
	//wrt.SetProtocol("TVUI_API");
	char szTmp[200];
	snprintf(szTmp, sizeof(szTmp), "%s",szRootVer.c_str());
	wrt.SetVersion(szTmp);

	snprintf(szTmp, sizeof(szTmp), "%s",szRootName.c_str());
	wrt.SetProtocol(szTmp);

	//char buffer[PACK_LEN];
	int len = PACK_LEN;
	ALLOCBUFFER(buffer,PACK_LEN)
	memset(buffer,'\0',PACK_LEN);
	

	int nResult	= wrt.Write(buffer, len);
	//if(nResult	== DST_XML_FAILED)		
	//{
	//	return false;
	//}
	string res(buffer);
	DEALLOCBUFFER(buffer);
	PDELETE(msg);
	
	return res;
}
//////////////////////////////////////////////////////////////////////////////////////
bool CDstCommonFunctions::xml_XmlStrToMap(string szXml, strmap& mp)
{
	int length = szXml.length();

	if (szXml.length()>0)
	{
		CDstXmlParser parse;
		if (DST_XML_FAILED != parse.Parse(szXml))
		{
			for (int i=0; i<parse.GetMessageNum(); i++)
			{
				CDstXmlMessage* pMsg = parse.GetMessageEx(i);
				if( pMsg )
				{
	
					XmlMsgToMap(pMsg, mp);
					return true;
				}
			}
		}
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////
