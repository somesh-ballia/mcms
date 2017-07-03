
#ifndef __DST_XML_PARSER_H__
#define __DST_XML_PARSER_H__

//#include <string.h>
#include "DstXml.h"

// DST XML


class CDstXmlParser
{
public:
	CDstXmlParser();
	~CDstXmlParser();

public:
	X_ERR				Parse(const std::string& szData);
	X_ERR				Parse(char * szData, int & len);

	char *				GetProtocol();
	char *				GetVersion();
	unsigned long		GetVersionNum();

	int					GetMessageNum();
	CDstXmlMessage *	GetMessageEx(int index);
	CDstXmlMessage *	GetMessageByNodeType(const char* nodeType);
private:
	char				m_szProtocol[DST_XML_PROTOCOL_ID_LEN];
	char				m_szVersion[DST_XML_VERSION_ID_LEN];
	unsigned long		m_ulVersion;

	CDstXmlMessage **	m_ppMessage;
	int					m_iMessageNum;
	int					m_iMessageRoom;

public:
	void				Release();

	int					SkipSpace(char * szData);
	int					GetString(char * szData);
	int					GetSymbol(char * szData);

	X_ERR				ParseHeader(char * szData, int & len);
	X_ERR				ParseProtocol(char * szData, int & len);
};

#endif	//#ifndef __DST_XML_PARSER_H__
