#include <stdlib.h>
#include "DstXml.h"

CDstXmlNode::CDstXmlNode()
{
	memset(m_szName, 0, sizeof(m_szName));
	m_pszValue = NULL;
}

CDstXmlNode::~CDstXmlNode()
{
	memset(m_szName, 0, sizeof(m_szName));

	if (m_pszValue)
	{
		free(m_pszValue);
		m_pszValue = NULL;
	}
}

void CDstXmlNode::SetName(char * szName, int len)
{
	memset(m_szName, 0, sizeof(m_szName));

	if (NULL == szName)
	{
		return;
	}

	if (len < 0)
	{
		len = (int)strlen(szName);
	}

	memcpy(m_szName, szName, __min(DST_XML_MESSAGE_NAME_LEN - 1, len));

	if (m_pszValue)
	{
		free(m_pszValue);
		m_pszValue = NULL;
	}
}

void CDstXmlNode::SetValue(char * szValue, int len)
{
	if (m_pszValue)
	{
		free(m_pszValue);
		m_pszValue = NULL;
	}

	if (NULL == szValue)
	{
		return;
	}

	if (len < 0)
	{
		len = (int)strlen(szValue);
	}

	m_pszValue = (char *)malloc(len + 1);
	if(m_pszValue)
	{
		memset(m_pszValue, 0, len + 1);
		memcpy(m_pszValue, szValue, len);
	}
	else
		FPASSERTMSG(1, "malloc failed");
}

void CDstXmlNode::SetValue(int iValue)
{
	if (m_pszValue)
	{
		free(m_pszValue);
		m_pszValue = NULL;
	}

	m_pszValue = (char *)malloc(32);
	if(m_pszValue)
	{
		memset(m_pszValue, 0, 32);
		sprintf(m_pszValue, "%d", iValue);
	}
	else
		FPASSERTMSG(1, "malloc failed");
}

void CDstXmlNode::SetValue(unsigned int uiValue)
{
	if (m_pszValue)
	{
		free(m_pszValue);
		m_pszValue = NULL;
	}

	m_pszValue = (char *)malloc(32);
	if(m_pszValue)
	{
		memset(m_pszValue, 0, 32);
		sprintf(m_pszValue, "%u", uiValue);
	}
	else
		FPASSERTMSG(1, "malloc failed");
}

char * CDstXmlNode::GetName()
{
	return m_szName;
}

char * CDstXmlNode::GetValue()
{
	return m_pszValue;
}

CDstXmlMessage::CDstXmlMessage()
{
	m_ppNode    = NULL;
	m_iNodeNum  = 0;
	m_iNodeRoom = 0;
}

CDstXmlMessage::~CDstXmlMessage()
{
	Release();
}

void CDstXmlMessage::Release()
{
	memset(m_szMessageType, 0, sizeof(m_szMessageType));
	memset(m_szMessageId, 0, sizeof(m_szMessageId));

	if (m_ppNode)
	{
		for (int k=0; k<m_iNodeNum; k++)
		{
			if (m_ppNode[k])
			{
				delete m_ppNode[k];
				m_ppNode[k] = NULL;
			}
		}

		free(m_ppNode);
		m_ppNode = NULL;
	}

	m_iNodeNum  = 0;
	m_iNodeRoom = 0;
}

X_ERR CDstXmlMessage::Parse(char * szData, int & len)
{
	X_ERR	ret = DST_XML_SUCCESS;
	int		lenParsed = 0;
	int		l;

	Release();

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if ('<' != szData[lenParsed])
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	lenParsed += 1;

	l = GetSymbol(szData + lenParsed);
	if (l <= 0)
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	memset(m_szMessageType, 0, sizeof(m_szMessageType));
	memcpy(m_szMessageType, szData + lenParsed, __min(DST_XML_MESSAGE_NAME_LEN - 1, l));

	lenParsed += l;

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "id", (int)strlen("id")))
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	lenParsed += (int)strlen("id");

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "=", (int)strlen("=")))
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	lenParsed += (int)strlen("=");

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	l = GetString(szData + lenParsed);
	if (l <= 0)
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	memset(m_szMessageId, 0, sizeof(m_szMessageId));
	memcpy(m_szMessageId, szData + lenParsed + 1, __min(DST_XML_MESSAGE_NAME_LEN - 1, l - 2));

	lenParsed += l;

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if ('>' != szData[lenParsed])
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	char szTemp[DST_XML_MESSAGE_NAME_LEN + 4];
	memset(szTemp, 0, sizeof(szTemp));
	snprintf(szTemp, sizeof(szTemp), "</%s", m_szMessageType);

	char *	pClose;
	pClose = strstr(szData, szTemp);
	if (NULL == pClose)
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	lenParsed += 1;

	int iRemainMessageData;
	iRemainMessageData = (int)(pClose - szData) - lenParsed;

	while (iRemainMessageData > 0)
	{
		l = SkipSpace(szData + lenParsed);
		lenParsed += l;
		iRemainMessageData -= l;

		if (iRemainMessageData <= 0)
		{
			break;
		}

		if (m_iNodeNum + 1 > m_iNodeRoom)
		{
			int iOldRoom = m_iNodeRoom;
			m_iNodeRoom += 8;
			CDstXmlNode ** ppTemp = (CDstXmlNode **)malloc(sizeof(CDstXmlNode *) * m_iNodeRoom);

			if (ppTemp == NULL) 
			{
				ret = DST_XML_FAILED;
				goto exitParse;
			}
			memset(ppTemp, 0, sizeof(CDstXmlNode *) * m_iNodeRoom);
			if (m_ppNode)
			{
				memcpy(ppTemp, m_ppNode, sizeof(CDstXmlNode *) * iOldRoom);
				free(m_ppNode);
			}
			m_ppNode = ppTemp;
		}

		m_ppNode[m_iNodeNum] = new CDstXmlNode();
		m_iNodeNum ++;

		// Get Node Name and Value
		{
			if ('<' != szData[lenParsed])
			{
				lenParsed += 0;
				ret = DST_XML_FAILED;
				goto exitParse;
			}

			lenParsed += 1;
			iRemainMessageData -= 1;

			l = GetSymbol(szData + lenParsed);
			if (l <= 0)
			{
				lenParsed += 0;
				ret = DST_XML_FAILED;
				goto exitParse;
			}

			m_ppNode[m_iNodeNum - 1]->SetName(szData + lenParsed, l);

			lenParsed += l;
			iRemainMessageData -= l;

			l = SkipSpace(szData + lenParsed);
			lenParsed += l;
			iRemainMessageData -= l;

			// wangjj. 2005.8.15
			bool bDstFormat = true;
			if(szData[lenParsed] == '>')
				bDstFormat = false;
			if(bDstFormat)
			{
				if (0 != strncmp((szData + lenParsed), "value", (int)strlen("value")))
				{
					lenParsed += 0;
					ret = DST_XML_FAILED;
					goto exitParse;
				}

				lenParsed += (int)strlen("value");
				iRemainMessageData -= (int)strlen("value");

				l = SkipSpace(szData + lenParsed);
				lenParsed += l;
				iRemainMessageData -= l;

				if (0 != strncmp((szData + lenParsed), "=", (int)strlen("=")))
				{
					lenParsed += 0;
					ret = DST_XML_FAILED;
					goto exitParse;
				}

				lenParsed += (int)strlen("=");
				iRemainMessageData -= (int)strlen("=");

				l = SkipSpace(szData + lenParsed);
				lenParsed += l;
				iRemainMessageData -= l;

				l = GetString(szData + lenParsed);
				if (l <= 0)
				{
					lenParsed += 0;
					ret = DST_XML_FAILED;
					goto exitParse;
				}

				m_ppNode[m_iNodeNum - 1]->SetValue(szData + lenParsed + 1, l - 2);

				lenParsed += l;
				iRemainMessageData -= l;

				l = SkipSpace(szData + lenParsed);
				lenParsed += l;
				iRemainMessageData -= l;

				if (0 != strncmp((szData + lenParsed), "/>", (int)strlen("/>")))
				{
					lenParsed += 0;
					ret = DST_XML_FAILED;
					goto exitParse;
				}

				lenParsed += (int)strlen("/>");
				iRemainMessageData -= (int)strlen("/>");		
			}
			else
			{
				int l = 0;
				while(szData[lenParsed+l] != '<')		l++;

				m_ppNode[m_iNodeNum - 1]->SetValue(szData + lenParsed + 1, l-1);

				lenParsed += l;
				iRemainMessageData -= l;	

				char szEnd[100];
				char* szName = m_ppNode[m_iNodeNum - 1]->GetName();
				snprintf(szEnd, sizeof(szEnd), "</%s>", szName);

				if (0 != strncmp((szData + lenParsed), szEnd, (int)strlen(szEnd)))
				{
					lenParsed += 0;
					ret = DST_XML_FAILED;
					goto exitParse;
				}

				lenParsed += (int)strlen(szEnd);
				iRemainMessageData -= (int)strlen(szEnd);	
			}


		}
	}

	if (iRemainMessageData < 0)
	{
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	memset(szTemp, 0, sizeof(szTemp));
	snprintf(szTemp, sizeof(szTemp), "</%s", m_szMessageType);
	lenParsed += (int)strlen(szTemp);

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if ('>' != szData[lenParsed])
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	lenParsed += 1;

exitParse:
	len = lenParsed;
	return ret;
}

char * CDstXmlMessage::GetType()
{
	return m_szMessageType;
}

char * CDstXmlMessage::GetId()
{
	return m_szMessageId;
}

int CDstXmlMessage::GetNodeNum()
{
	return m_iNodeNum;
}

CDstXmlNode * CDstXmlMessage::GetNode(int index)
{
	if (NULL == m_ppNode)
	{
		return NULL;
	}

	if (index < 0 || index >= m_iNodeNum)
	{
		return NULL;
	}

	return m_ppNode[index];
}

void CDstXmlMessage::SetType(char * szType, int len)
{
	memset(m_szMessageType, 0, sizeof(m_szMessageType));

	if (NULL == szType)
	{
		return;
	}

	if (len <= 0)
	{
		len = (int)strlen(szType);
	}

	memcpy(m_szMessageType, szType, __min(DST_XML_MESSAGE_NAME_LEN - 1, len));
}

void CDstXmlMessage::SetId(char * szId, int len)
{
	memset(m_szMessageId, 0, sizeof(m_szMessageId));

	if (NULL == szId)
	{
		return;
	}

	if (len <= 0)
	{
		len = (int)strlen(szId);
	}

	memcpy(m_szMessageId, szId, __min(DST_XML_MESSAGE_NAME_LEN - 1, len));
}

CDstXmlNode * CDstXmlMessage::AddNode()
{
	if (m_iNodeNum + 1 > m_iNodeRoom)
	{
		int iOldRoom = m_iNodeRoom;
		m_iNodeRoom += 8;
		CDstXmlNode ** ppTemp = (CDstXmlNode **)malloc(sizeof(CDstXmlNode *) * m_iNodeRoom);
		if(ppTemp)
		{
			memset(ppTemp, 0, sizeof(CDstXmlNode *) * m_iNodeRoom);
			if (m_ppNode)
			{
				memcpy(ppTemp, m_ppNode, sizeof(CDstXmlNode *) * iOldRoom);
				free(m_ppNode);
			}
			m_ppNode = ppTemp;
		}
		else
		{
			m_iNodeRoom -= 8;  //retore the size
			FPASSERT_AND_RETURN_VALUE(1, NULL);  
		}
			
	}

	m_ppNode[m_iNodeNum] = new CDstXmlNode();
	m_iNodeNum ++;

	return m_ppNode[m_iNodeNum - 1];
}

char * CDstXmlMessage::GetValueByName(char * szName)
{
	char * pValue = NULL;

	if (NULL == szName)
	{
		return pValue;
	}

	for (int k=0; k<GetNodeNum(); k++)
	{
		CDstXmlNode* pNode = GetNode(k);
		if ( pNode && 0 == strcmp(szName, pNode->GetName()))
		{
			pValue = pNode->GetValue();
		}
	}
	return pValue;
}

// wangjj. 2005.8.25
bool CDstXmlMessage::SetValueByName(char * szName, char* szValue)
{
	char * pValue = NULL;

	if (NULL == szName || NULL == szValue)
	{
		return false;
	}

	for (int k=0; k<GetNodeNum(); k++)
	{
		CDstXmlNode* pNode = GetNode(k);
		if ( pNode && 0 == strcmp(szName, pNode->GetName()))
		{
			pNode->SetValue(szValue);
			return true;
		}
	}
	return false;
}

int CDstXmlMessage::SkipSpace(char * szData)
{
	int l = 0;

	while (isspace(szData[l]))
	{
		l ++;
	}

	while (1)
	{
		if (0 != strncmp((szData + l), "<!--", (int)strlen("<!--")))
		{
			break;
		}

		char * pCloseRemark = strstr((szData + l + 3), "-->");
		if (NULL == pCloseRemark)
		{
			break;
		}
		else
		{
			l = (int)(pCloseRemark - szData) + 3;

			while (isspace(szData[l]))
			{
				l ++;
			}
		}
	}

	return l;
}

int CDstXmlMessage::GetString(char * szData)
{
	if ('"' != szData[0])
	{
		return 0;
	}

	char * p = strchr((szData + 1), '"');
	if (p)
	{
		return (int)(p - szData) + 1;
	}
	else
	{
		return 0;
	}
}

int CDstXmlMessage::GetSymbol(char * szData)
{
	int l = 0;

	if (!__iscsymf(szData[0]))
	{
		return 0;
	}

	while (!isspace(szData[l]))
	{
		if (!__iscsym(szData[l]))
		{
			if(szData[l] == '>')		return l;		// wangjj, 2005.8.15
			else						return 0;
		}

		l ++;
	}

	return l;
}
