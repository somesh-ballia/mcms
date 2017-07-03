#include <stdlib.h>
#include "DstXml.h"
#include "DstXmlParser.h"

CDstXmlParser::CDstXmlParser()
{
	memset(m_szProtocol, 0, sizeof(m_szProtocol));
	memset(m_szVersion, 0, sizeof(m_szVersion));
	m_ulVersion = 0;

	m_ppMessage    = NULL;
	m_iMessageNum  = 0;
	m_iMessageRoom = 0;
}

CDstXmlParser::~CDstXmlParser()
{
	Release();
}

void CDstXmlParser::Release()
{
	memset(m_szProtocol, 0, sizeof(m_szProtocol));
	memset(m_szVersion, 0, sizeof(m_szVersion));
	m_ulVersion = 0;

	if (m_ppMessage)
	{
		for (int k=0; k<m_iMessageNum; k++)
		{
			if (m_ppMessage[k])
			{
				delete m_ppMessage[k];
				m_ppMessage[k] = NULL;
			}
		}

		free(m_ppMessage);
		m_ppMessage = NULL;
	}

	m_iMessageNum  = 0;
	m_iMessageRoom = 0;
}

X_ERR CDstXmlParser::Parse(const std::string& szData)
{
	int nLen = (int)(szData.length());

	char* buffer = new char[nLen+1];
	memset(buffer, 0, nLen+1);

	memcpy(buffer, szData.c_str(), nLen);
	X_ERR nCode = Parse(buffer, nLen);

	delete []buffer;
	return nCode;
}

X_ERR CDstXmlParser::Parse(char * szData, int & len)
{
	X_ERR	ret = DST_XML_SUCCESS;
	X_ERR	r;
	int		lenParsed = 0;
	int		l;

	Release();	

	szData[len] = 0x0;

	// <?xml version="1.0" encoding="UTF-8" ?>
	r = ParseHeader(szData + lenParsed, l);
	if (DST_XML_SUCCESS != r)
	{
		lenParsed += l;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	lenParsed += l;

	// 
	r = ParseProtocol(szData + lenParsed, l);
	if (DST_XML_SUCCESS != r)
	{
		lenParsed += l;
		ret = DST_XML_FAILED;
		goto exitParse;
	}

	lenParsed += l;

	if (lenParsed != len)
	{
		ret = DST_XML_MOREDATA;
	}

exitParse:
	len = lenParsed;
	return ret;
}

char * CDstXmlParser::GetProtocol()
{
	return m_szProtocol;
}

char * CDstXmlParser::GetVersion()
{
	return m_szVersion;
}

unsigned long CDstXmlParser::GetVersionNum()
{
	return m_ulVersion;
}

int CDstXmlParser::GetMessageNum()
{
	return m_iMessageNum;
}

CDstXmlMessage * CDstXmlParser::GetMessageEx(int index)
{
	if (NULL == m_ppMessage)
	{
		return NULL;
	}

	if (index < 0 || index >= m_iMessageNum)
	{
		return NULL;
	}

	return m_ppMessage[index];
	
}

CDstXmlMessage * CDstXmlParser::GetMessageByNodeType(const char* nodeType)
{
	for(int i=0; i<m_iMessageNum; i++)
	{
		CDstXmlMessage* pMsg = GetMessageEx(i);
		if(strcmp(pMsg->GetType(), nodeType)==0)
		{
			return pMsg;
		}
	}

	return NULL;
}

int CDstXmlParser::SkipSpace(char * szData)
{
	int l = 0;

	while (isspace(szData[l]))
	{
		l ++;
	}

	while (1)
	{
		if (0 != strncmp((szData + l), "<!--", strlen("<!--")))
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

int CDstXmlParser::GetString(char * szData)
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

int CDstXmlParser::GetSymbol(char * szData)
{
	int l = 0;

	if (!__iscsymf(szData[0]))
	{
		return 0;
	}

	while (!isspace(szData[l])) 
	{
		if( szData[l]=='>')		// wangjj. 2005.8.22
		{
			break;
		}
		if (!__iscsym(szData[l]))
		{
			return 0;
		}

		l ++;
	}

	return l;
}

X_ERR CDstXmlParser::ParseHeader(char * szData, int & len)
{
	X_ERR	ret = DST_XML_SUCCESS;
	int		lenParsed = 0;
	int		l;

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "<?xml ", strlen("<?xml ")))
	{
		lenParsed += 5;
		ret = DST_XML_FAILED;
		goto exitParseHeader;
	}

	lenParsed += (int)strlen("<?xml ");

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "version", strlen("version")))
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseHeader;
	}

	lenParsed += (int)strlen("version");

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "=", strlen("=")))
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseHeader;
	}

	lenParsed += (int)strlen("=");

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	l = GetString(szData + lenParsed);
	if (l <= 0)
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseHeader;
	}

	lenParsed += l;

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "encoding", strlen("encoding")))
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseHeader;
	}

	lenParsed += (int)strlen("encoding");

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "=", strlen("=")))
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseHeader;
	}

	lenParsed += (int)strlen("=");

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	l = GetString(szData + lenParsed);
	if (l <= 0)
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseHeader;
	}
	else
	{
		char szTemp[1024];
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, szData + lenParsed + 1, l - 2);
#ifndef WIN32
		if (0 != strncasecmp(szTemp, "UTF-8", l - 2))
#else
		if (0 != strncmp(_strupr(szTemp), "UTF-8", l - 2))
#endif	//#ifndef WIN32
		{
			lenParsed += 1;
			ret = DST_XML_FAILED;
			goto exitParseHeader;
		}
	}

	lenParsed += l;

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "?>", strlen("?>")))
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseHeader;
	}

	lenParsed += (int)strlen("?>");

exitParseHeader:
	len = lenParsed;
	return ret;
}

X_ERR CDstXmlParser::ParseProtocol(char * szData, int & len)
{
	X_ERR	ret = DST_XML_SUCCESS;
	int		lenParsed = 0;
	int		l;

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if ('<' != szData[lenParsed])
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseProtocol;
	}

	lenParsed += 1;

	l = GetSymbol(szData + lenParsed);
	if (l <= 0)
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseProtocol;
	}

	memset(m_szProtocol, 0, sizeof(m_szProtocol));
	memcpy(m_szProtocol, szData + lenParsed, __min(DST_XML_PROTOCOL_ID_LEN - 1, l));

	lenParsed += l;

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "version", strlen("version")))
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseProtocol;
	}

	lenParsed += (int)strlen("version");

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if (0 != strncmp((szData + lenParsed), "=", strlen("=")))
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseProtocol;
	}

	lenParsed += (int)strlen("=");

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	l = GetString(szData + lenParsed);
	if (l <= 0)
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseProtocol;
	}

	memset(m_szVersion, 0, sizeof(m_szVersion));
	memcpy(m_szVersion, szData + lenParsed + 1, __min(DST_XML_VERSION_ID_LEN - 1, l - 2));

	lenParsed += l;

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if ('>' != szData[lenParsed])
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseProtocol;
	}

	char szTemp[DST_XML_PROTOCOL_ID_LEN + 4];
	memset(szTemp, 0, sizeof(szTemp));
	snprintf(szTemp, sizeof(szTemp), "</%s", m_szProtocol);

	char *	pClose;
	pClose = strstr(szData, szTemp);
	if (NULL == pClose)
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseProtocol;
	}

	lenParsed += 1;

	int iRemainProtocolData;
	iRemainProtocolData = (int)(pClose - szData) - lenParsed;

	while (iRemainProtocolData > 0)
	{
		if (m_iMessageNum + 1 > m_iMessageRoom)
		{
			int iOldRoom = m_iMessageRoom;
			m_iMessageRoom += 8;
			CDstXmlMessage ** ppTemp = (CDstXmlMessage **)malloc(sizeof(CDstXmlMessage *) * m_iMessageRoom);
			if(ppTemp)
			{
				memset(ppTemp, 0, sizeof(CDstXmlMessage *) * m_iMessageRoom);
				if (m_ppMessage)
				{
					memcpy(ppTemp, m_ppMessage, sizeof(CDstXmlMessage *) * iOldRoom);
					free(m_ppMessage);
				}
				m_ppMessage = ppTemp;
			}
			else
			{
				m_iMessageRoom -= 8;  //restore the size
				FPASSERTMSG(NULL == ppTemp, "malloc return NULL !!!");
			}
		}

		m_ppMessage[m_iMessageNum] = new CDstXmlMessage();
		m_iMessageNum ++;

		l = SkipSpace(szData + lenParsed);
		lenParsed += l;
		iRemainProtocolData -= l;

		if (iRemainProtocolData <= 0)
		{
			break;
		}

		X_ERR	r;
		r = m_ppMessage[m_iMessageNum - 1]->Parse(szData + lenParsed, l);
		if (DST_XML_SUCCESS != r)
		{
			lenParsed += l;
			ret = DST_XML_FAILED;
			goto exitParseProtocol;
		}

		lenParsed += l;
		iRemainProtocolData -= l;

		l = SkipSpace(szData + lenParsed);
		lenParsed += l;
		iRemainProtocolData -= l;
	}

	if (iRemainProtocolData < 0)
	{
		ret = DST_XML_FAILED;
		goto exitParseProtocol;
	}

	memset(szTemp, 0, sizeof(szTemp));
	snprintf(szTemp, sizeof(szTemp), "</%s", m_szProtocol);
	lenParsed += (int)strlen(szTemp);

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

	if ('>' != szData[lenParsed])
	{
		lenParsed += 0;
		ret = DST_XML_FAILED;
		goto exitParseProtocol;
	}

	lenParsed += 1;

	l = SkipSpace(szData + lenParsed);
	lenParsed += l;

exitParseProtocol:
	len = lenParsed;
	return ret;
}
