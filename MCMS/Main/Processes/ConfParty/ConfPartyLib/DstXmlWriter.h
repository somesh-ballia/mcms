#ifndef __DST_XML_WRITER_H__
#define __DST_XML_WRITER_H__

#define BUFFER_SIZE 1024 * 1024

class CDstXmlWriter
{
public:
	CDstXmlWriter();
	~CDstXmlWriter();

public:
	X_ERR				Write(char * szData, int & len);
	X_ERR				Write(std::string& szData);

	void				SetProtocol(char * szProtocol, int len = 0);
	void				SetVersion(char * szVersion, int len = 0);

	CDstXmlMessage *	AddMessage();
	CDstXmlMessage *	CloneMessage(CDstXmlMessage* pMsgSrc);

private:
	char				m_szProtocol[DST_XML_PROTOCOL_ID_LEN];
	char				m_szVersion[DST_XML_VERSION_ID_LEN];

	CDstXmlMessage **	m_ppMessage;
	int					m_iMessageNum;
	int					m_iMessageRoom;

private:
	void	Release();
};

#endif	//#ifndef __DST_XML_WRITER_H__
