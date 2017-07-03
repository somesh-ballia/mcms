// XmlMiniParser.cpp: implementation of the CXmlMiniParser class.
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include "XmlMiniParser.h"
#include <OperatorDefines.h>
#include "../../../McmIncld/Common/AuditDefines.h"
#include "SharedDefines.h"
//////////////////////////////////////////////////////////////////////
// CXmlMiniParser class
//////////////////////////////////////////////////////////////////////


char* CXmlMiniParser::GetActionName(const char* pszSearchString, char** pszEndActionName)
{
	if(!pszSearchString || !pszEndActionName)
		return NULL;

	bool isActionTagExists = true;

	// ===== 1. get pointer to start Action
	char* pszStartActionName = (char*)strstr(pszSearchString,"<ACTION>");


	// 11.01.07: supported even if there is no '<ACTION>' tag

	if(pszStartActionName) // "<ACTION>" tag exists
	{
		pszStartActionName += strlen("<ACTION>");
	}
	else // "<ACTION>" tag does not exists; the Action comes right after common_params
	{
		isActionTagExists = false;

		char* afterCommonParamsTag = (char*)strstr(pszSearchString,"</TRANS_COMMON_PARAMS>");
		if(afterCommonParamsTag)
		{
			afterCommonParamsTag += strlen("</TRANS_COMMON_PARAMS>");

			pszStartActionName = strstr(afterCommonParamsTag, "<");
			if (!pszStartActionName)
				return NULL;
		}
		else // no </TRANS_COMMON_PARAMS> tag
		{
			return NULL;
		}
	}

	// ===== 2. get pointer to close Action
	char* pszStartActionClosingTag = 0;
	if (true == isActionTagExists)
	{
		pszStartActionClosingTag = strstr(pszStartActionName,"</ACTION>");
	}
	else // "<ACTION>" tag does not exists
	{
		// ----- a. 'actionName' will contain the Action name itself ("XXX") ['100' was randomly chosen]
		char actionName[100];
		memset(actionName, 0, 100);
		char* endOfName = strstr(pszStartActionName,">");

		if (endOfName)
		{
			// no content in the Action ("<XXX/>")
			char* itsClosingTag = endOfName - 1;
			if ('/' == *itsClosingTag)
			{
				pszStartActionClosingTag = strstr(itsClosingTag, "<");
			}

			// Action exists, without 'Action' tag ("<XXX>xxxx</XXX>")
			else
			{
				int strActionTagLen = 0;
				if ( (endOfName-(pszStartActionName+1)) < 100 )
					strActionTagLen =  endOfName - (pszStartActionName+1);
				else
					strActionTagLen = 99;

				const int maxStrLenForKW = std::min((int) sizeof(actionName) - 1, strActionTagLen);
				strncpy(actionName, pszStartActionName+1, maxStrLenForKW);
				actionName[maxStrLenForKW] = 0;

				// ----- b. 'sCloseActionTag' will contain the closing tag of the Action ("</XXX>")
				std::string sCloseActionTag = "</";
				sCloseActionTag += actionName;
				sCloseActionTag += ">";

				// ----- c. 'pszStartActionClosingTag' will contain the string starting at the open tag AFTER the closing tag of the Action
				char* afterActionClosingTag  = strstr( pszStartActionName, sCloseActionTag.c_str() );
				if (afterActionClosingTag)
				{
					afterActionClosingTag += sCloseActionTag.length();
					pszStartActionClosingTag = strstr(afterActionClosingTag, "<");
				}
			}
		}
	}

	if(!pszStartActionClosingTag)
		return NULL;

	while((pszStartActionName != pszStartActionClosingTag) &&
		  (*pszStartActionName != '<'))
	{
		pszStartActionName++;
	}

	if(pszStartActionName == pszStartActionClosingTag)
		return NULL;

	pszStartActionName++;

	char* pszEndActionNameTagCloser = strstr(pszStartActionName,">");
	char* pszEndActionNameSlash = strstr(pszStartActionName,"/");	// Action without contents

	if(pszEndActionNameTagCloser < 	pszEndActionNameSlash)
		*pszEndActionName = pszEndActionNameTagCloser;
	else
		*pszEndActionName = pszEndActionNameSlash;
	if( NULL == *pszEndActionName )
		return NULL;

	while((*pszEndActionName > pszStartActionName) && (*(*pszEndActionName-1) == ' '))
			(*pszEndActionName)--;

	if(!(*pszEndActionName))
		return NULL;

	return pszStartActionName;
}

//////////////////////////////////////////////////////////////////////
char* CXmlMiniParser::GetTransName(const char* pszSearchString, char** pszEndTransName)
{
	if(!pszSearchString || !pszEndTransName)
		return NULL;

	char* pszStartTransName = (char*)strstr(pszSearchString,"<TRANS");

	if(!pszStartTransName)
		return NULL;

	pszStartTransName++;

	*pszEndTransName = strstr(pszStartTransName,">");

	if(!(*pszEndTransName))
		return NULL;

	return pszStartTransName;
}

//////////////////////////////////////////////////////////////////////
char* CXmlMiniParser::GetElementStringValue(const char* pszSearchString,
											const char* pszElementOpenTag,
											const char* pszElementCloseTag,
											char** pszEndElementValue)
{
	char *pszStartElementValue;

	if(!pszSearchString || !pszElementOpenTag || !pszElementCloseTag || !pszEndElementValue)
		return NULL;

	pszStartElementValue = (char*)strstr(pszSearchString,pszElementOpenTag);

	if(!pszStartElementValue)
		return NULL;

	pszStartElementValue += strlen(pszElementOpenTag);

	*pszEndElementValue = strstr(pszStartElementValue,pszElementCloseTag);

	if(!(*pszEndElementValue))
		return NULL;

	return pszStartElementValue;
}

//////////////////////////////////////////////////////////////////////
int CXmlMiniParser::GetConnectionId(const char* pszSearchString)
{
	char *pszStartConnId, *pszEndConnId;

	pszStartConnId = GetElementStringValue(pszSearchString,"<MCU_USER_TOKEN>",
										   "</MCU_USER_TOKEN>",&pszEndConnId);

	if(pszStartConnId)
	{
		if(!isdigit(pszStartConnId[0]))
			return 0;

		return atoi(pszStartConnId);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
int CXmlMiniParser::GetYourToken1(const char* pszSearchString)
{
	char *pszStartYourToken1, *pszEndYourToken1;

	pszStartYourToken1 = GetElementStringValue(pszSearchString,"<YOUR_TOKEN1>",
										   "</YOUR_TOKEN1>",&pszEndYourToken1);

	if(pszStartYourToken1)
	{
		if(!isdigit(pszStartYourToken1[0]))
			return -1;

		return atoi(pszStartYourToken1);
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////
int CXmlMiniParser::GetYourToken2(const char* pszSearchString)
{
	char *pszStartYourToken2, *pszEndYourToken2;

	pszStartYourToken2 = GetElementStringValue(pszSearchString,"<YOUR_TOKEN2>",
										   "</YOUR_TOKEN2>",&pszEndYourToken2);

	if(pszStartYourToken2)
	{
		if(!isdigit(pszStartYourToken2[0]))
			return -1;

		return atoi(pszStartYourToken2);
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////
int CXmlMiniParser::GetMessageId(const char* pszSearchString)
{
	char *pszStartMessageId, *pszEndMessageId;

	pszStartMessageId = GetElementStringValue(pszSearchString,"<MESSAGE_ID>",
											  "</MESSAGE_ID>",&pszEndMessageId);

	if(pszStartMessageId)
	{
		if(!isdigit(pszStartMessageId[0]))
			return -1;

		return atoi(pszStartMessageId);
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////
int CXmlMiniParser::GetResponseStatus(const char* pszSearchString)
{
	char* pszRespStatusId = (char*)strstr(pszSearchString,"<RETURN_STATUS>");

	if(pszRespStatusId == NULL)
		return -1;
	pszRespStatusId = strstr(pszRespStatusId, "<ID>");

	if(pszRespStatusId)
	{
		pszRespStatusId += strlen("<ID>");
		return atoi(pszRespStatusId);
	}
	else
		return -1;
}

// //////////////////////////////////////////////////////////////////////
// char* CXmlMiniParser::GetStationName(const char* pszSearchString, char** pszEndStationName)
// {
// 	return GetElementStringValue(pszSearchString,"<STATION_NAME>","</STATION_NAME>",
// 								 pszEndStationName);
// }


//////////////////////////////////////////////////////////////////////
char* CXmlMiniParser::GetFilenameInURI(const char* pszURI)
{
	if((pszURI == NULL) || (strlen(pszURI) == 0))
		return NULL;

	char* pszFilename = (char*)(pszURI + strlen(pszURI));

	while((pszFilename > pszURI) && (*(pszFilename-1) != '/') &&
		  (*(pszFilename-1) != '\\'))
		{
				pszFilename--;
		}

	if((pszFilename == (pszURI + strlen(pszURI))) || (pszFilename == pszURI))
		return NULL;

	return pszFilename;
}

int CXmlMiniParser::GetAuthorization(const char* pszSearchString)
{
	char *pszStartAuthorization, *pszEndAuthorization;

	pszStartAuthorization = GetElementStringValue(pszSearchString,
												  "<AUTHORIZATION_GROUP>",
												  "</AUTHORIZATION_GROUP>",
												  &pszEndAuthorization);

	if(pszStartAuthorization)
	{
		if(strstr(pszStartAuthorization,"administrator") != NULL)
			return SUPER;
		else if(strstr(pszStartAuthorization,"operator") != NULL)
			return ORDINARY;
		else if(strstr(pszStartAuthorization,"recording_user") != NULL)
			return RECORDING_USER;
		else if(strstr(pszStartAuthorization,"recording_administrator") != NULL)
			return RECORDING_ADMIN;
		else if(strstr(pszStartAuthorization,"attendant") != NULL)
			return AUTH_OPERATOR;
        else if(strstr(pszStartAuthorization,"auditor") != NULL)
            return AUDITOR;
        else if(strstr(pszStartAuthorization,"administrator_readonly") != NULL)
            return ADMINISTRATOR_READONLY;
	}

	return GUEST;
}

bool CXmlMiniParser::GetUserName(const char* pszLoginAction, char* pszUserName)
{
	if(!pszLoginAction || !pszUserName)
		return false;

	pszUserName[0] = 0;

	char* pszStartUserName = (char*)strstr(pszLoginAction,"<USER_NAME>");

	if(!pszStartUserName)
		return false;

	pszStartUserName += strlen("<USER_NAME>");

	char* pszEndUserName = (char*)strstr(pszLoginAction,"</USER_NAME>");

	if(!pszEndUserName || (pszEndUserName <= (pszStartUserName+1)))
		return false;


	int size = (int) std::min(OPERATOR_NAME_LEN,(pszEndUserName - pszStartUserName));
	strncpy(pszUserName,pszStartUserName, size-1);
	pszUserName[size-1] = 0;

	return true;
}

bool CXmlMiniParser::GetStationName(const char* pszLoginAction, char* pszStationName)
{
	if(!pszLoginAction || !pszStationName)
		return false;

	pszStationName[0] = 0;

	char* pszStartStationName = (char*)strstr(pszLoginAction,"<STATION_NAME>");

	if(!pszStartStationName)
		return false;

	pszStartStationName += strlen("<STATION_NAME>");


	char* pszEndStationName = (char*)strstr(pszLoginAction,"</STATION_NAME>");

	if(!pszEndStationName || (pszEndStationName <= (pszStartStationName+1)))
		return false;
	int size = (int) std::min(MAX_AUDIT_WORKSTATION_NAME_LEN,(pszEndStationName - pszStartStationName));
	strncpy(pszStationName, pszStartStationName, size-1);
	pszStationName[size-1] = 0;

	return true;
}





