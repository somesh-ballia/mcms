#ifndef __DST_XML_COMMON_H__
#define __DST_XML_COMMON_H__

#include "PObject.h"

#ifndef WIN32
#define __min(a, b)		((a) < (b) ? (a) : (b))
#define __iscsymf(c)	(('_' == c) || isalpha(c))
#define __iscsym(c)	(('_' == c) || isalnum(c))
#endif	//#ifndef WIN32

#define	DST_XML_PROTOCOL_ID_LEN		64
#define	DST_XML_VERSION_ID_LEN		16
#define	DST_XML_MESSAGE_NAME_LEN	32

typedef enum X_ERR
{
	DST_XML_SUCCESS		= 0,	// successful
	DST_XML_FAILED		= 1,	// parse failed
	DST_XML_MOREDATA,			// parse the data success, but don't use all data
} X_ERR;

class CDstXmlNode
{
public:
	CDstXmlNode();
	~CDstXmlNode();

public:
	char *				GetName();
	char *				GetValue();

	void				SetName(char * szName, int len = -1);
	void				SetValue(char * szValue, int len = -1);
	void				SetValue(int iValue);
	void				SetValue(unsigned int uiValue);

private:
	char				m_szName[DST_XML_MESSAGE_NAME_LEN];
	char *				m_pszValue;
};

class CDstXmlMessage
{
public:
	CDstXmlMessage();
	~CDstXmlMessage();

public:
	X_ERR				Parse(char * szData, int & len);

	char *				GetType();
	char *				GetId();

	int					GetNodeNum();
	CDstXmlNode *		GetNode(int index);

	void				SetType(char * szType, int len = 0);
	void				SetId(char * szId, int len = 0);

	CDstXmlNode *		AddNode();

	char *				GetValueByName(char * szName);
	bool				SetValueByName(char * szName, char* szValue);

private:
	char				m_szMessageType[DST_XML_MESSAGE_NAME_LEN];
	char				m_szMessageId[DST_XML_MESSAGE_NAME_LEN];

	CDstXmlNode **		m_ppNode;
	int					m_iNodeNum;
	int					m_iNodeRoom;

private:
	void				Release();

	int					SkipSpace(char * szData);
	int					GetString(char * szData);
	int					GetSymbol(char * szData);
};

#endif	//#ifndef __DST_XML_COMMON_H__
