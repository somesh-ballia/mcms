#include <stdlib.h>
#include "DstXml.h"
#include "DstXmlWriter.h"

CDstXmlWriter::CDstXmlWriter()
{
	memset(m_szProtocol, 0, sizeof(m_szProtocol));
	memset(m_szVersion, 0, sizeof(m_szVersion));

	m_ppMessage    = NULL;
	m_iMessageNum  = 0;
	m_iMessageRoom = 0;
}

CDstXmlWriter::~CDstXmlWriter()
{
	Release();
}

void CDstXmlWriter::Release()
{
	memset(m_szProtocol, 0, sizeof(m_szProtocol));
	memset(m_szVersion, 0, sizeof(m_szVersion));

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

void CDstXmlWriter::SetProtocol(char * szProtocol, int len)
{
	memset(m_szProtocol, 0, sizeof(m_szProtocol));

	if (NULL == szProtocol)
	{
		return;
	}

	if (len <= 0)
	{
		len = (int)strlen(szProtocol);
	}

	memcpy(m_szProtocol, szProtocol, __min(DST_XML_PROTOCOL_ID_LEN - 1, len));
}

void CDstXmlWriter::SetVersion(char * szVersion, int len)
{
	memset(m_szVersion, 0, sizeof(m_szVersion));

	if (NULL == szVersion)
	{
		return;
	}

	if (len <= 0)
	{
		len = (int)strlen(szVersion);
	}

	memcpy(m_szVersion, szVersion, __min(DST_XML_VERSION_ID_LEN - 1, len));
}

CDstXmlMessage * CDstXmlWriter::CloneMessage(CDstXmlMessage* pMsgSrc)
{
	CDstXmlMessage* pMsg = AddMessage();

	if(pMsg)
	{
		pMsg->SetId(pMsgSrc->GetId());
		pMsg->SetType(pMsgSrc->GetType());
	}
	else
		FPASSERT_AND_RETURN_VALUE(1, NULL);

	for(int i=0; i<pMsgSrc->GetNodeNum(); i++)
	{
		CDstXmlNode* pNodeSrc = pMsgSrc->GetNode(i);
		if(pNodeSrc)
		{
			CDstXmlNode* pNode = pMsg->AddNode();
			if( pNode )
			{
				pNode->SetName(pNodeSrc->GetName());
				pNode->SetValue(pNodeSrc->GetValue());
			}
		}
	}

	return pMsg;
}

CDstXmlMessage * CDstXmlWriter::AddMessage()
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
			m_iMessageRoom -= 8;  //resore the size;
			FPASSERT_AND_RETURN_VALUE(1, NULL);
		}
	}

	m_ppMessage[m_iMessageNum] = new CDstXmlMessage();

	m_iMessageNum ++;

	return m_ppMessage[m_iMessageNum - 1];
}

X_ERR CDstXmlWriter::Write(std::string& szData)
{
	char* buffer = new char[BUFFER_SIZE];

	int len = BUFFER_SIZE;
	X_ERR nCode = Write(buffer, len);

	if(nCode == DST_XML_SUCCESS)
	{
		szData = buffer;
	}

	delete []buffer;
	return nCode;
}

X_ERR CDstXmlWriter::Write(char * szData, int & len)
{
	X_ERR	ret = DST_XML_SUCCESS;
	char	szTemp[32000];

	if (NULL == szData)
	{
		goto exitWriteError;
	}

	memset(szData, 0, len);

	if (strlen(m_szProtocol) <= 0)
	{
		goto exitWriteError;
	}

	if (strlen(m_szVersion) <= 0)
	{
		goto exitWriteError;
	}

	// write xml header
	snprintf(szTemp, sizeof(szTemp), "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n");
	if ((strlen(szData) + strlen(szTemp)) > (unsigned int)(len - 1))
	{
		goto exitWriteError;
	}

	strcat(szData, szTemp);

	// write protocol header
	snprintf(szTemp, sizeof(szTemp), "\t<%s version=\"%s\">\r\n", m_szProtocol, m_szVersion);
	if ((int)(strlen(szData) + strlen(szTemp)) > len - 1)
	{
		goto exitWriteError;
	}

	strcat(szData, szTemp);

	int k, l;
	for (k=0; k<m_iMessageNum; k++)
	{
		if (m_ppMessage && m_ppMessage[k])
		{
			CDstXmlMessage * pMsg = m_ppMessage[k];

			// write message header
			snprintf(szTemp, sizeof(szTemp), "\t\t<%s id=\"%s\">\r\n", pMsg->GetType(), pMsg->GetId());
			if ((int)(strlen(szData) + strlen(szTemp)) > len - 1)
			{
				goto exitWriteError;
			}

			strcat(szData, szTemp);

			for (l=0; l<pMsg->GetNodeNum(); l++)
			{
				CDstXmlNode * pNode = pMsg->GetNode(l);
				if (pNode)
				{
					// write node header
					snprintf(szTemp, sizeof(szTemp), "\t\t\t<%s value=\"%s\" />\r\n", pNode->GetName(), pNode->GetValue());
					if ((int)(strlen(szData) + strlen(szTemp)) > len - 1)
					{
						goto exitWriteError;
					}

					strcat(szData, szTemp);
				}
			}

			// write message close tag
			snprintf(szTemp, sizeof(szTemp), "\t\t</%s>\r\n", pMsg->GetType());
			if ((int)(strlen(szData) + strlen(szTemp)) > len - 1)
			{
				goto exitWriteError;
			}

			strcat(szData, szTemp);
		}
	}

	// write protocol close tag
	snprintf(szTemp, sizeof(szTemp), "\t</%s>\r\n", m_szProtocol);
	if ((int)(strlen(szData) + strlen(szTemp)) > len - 1)
	{
		goto exitWriteError;
	}

	strcat(szData, szTemp);

exitWrite:
	if (szData)
		len = (int)strlen(szData);

	return ret;

exitWriteError:
	ret = DST_XML_FAILED;
	goto exitWrite;
}
