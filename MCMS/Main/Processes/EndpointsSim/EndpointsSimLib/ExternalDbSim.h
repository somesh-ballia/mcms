//////////////////////////////////////////////////////////////////////
//
// ExternalDbSim.h: interface for external Database classes.
//
//////////////////////////////////////////////////////////////////////

#ifndef __EXTERNALDBSIM_H__
#define __EXTERNALDBSIM_H__


#include "SerializeObject.h"

#define		ENDPOINTS_SIM_EXTDB_FILE_NAME		"Cfg/EP_SIM_EXTERNAL_DB.XML"


class CXMLDOMElement;
class CCommSetExternalDbCreate;
class CCommSetExternalDbAdd;
class CCommSetExternalDbUser;


const int MAX_ETX_DB_RECORDS = 8;



/////////////////////////////////////////////////////////////////////////////
class CExtDbRecord : public CSerializeObject
{
CLASS_TYPE_1(CExtDbRecord,CSerializeObject)
public:
	CExtDbRecord();
	virtual ~CExtDbRecord();

	virtual const char* NameOf() const { return "CExtDbRecord";}
	virtual CSerializeObject* Clone() { return NULL; }

	void Serialize(CSegment& segment) const;

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);

	const char* GetNumericId() const { return m_szNumericId; }
	const char* GetName() const { return m_szName; }
	const char* GetChairPwd() const { return m_szChairPassword; }
	const char* GetEntryPwd() const { return m_szEntryPassword; }
	const char* GetBillingData() const { return m_szBillingData; }
	const char* GetOwner() const { return m_szOwner; }
	const char* GetPhone1() const { return m_szPhone1; }
	const char* GetPhone2() const { return m_szPhone2; }
	const char* GetInfo(const int ind) const { return (ind>=0 && ind<4) ? m_aszInfo[ind] : "" ; }
	const char* GetDisplayName() const { return m_szDisplayName;}
	WORD GetMinParties() const { return m_wMinParties; }
	WORD GetMaxParties() const { return m_wMaxParties; }
	BOOL GetIsLeader() const { return m_bIsLeader; }
	BOOL GetIsGuest() const { return m_bIsGuest; }
	BOOL GetIsVip() const { return m_bIsVip; }

	void Danny();
	void Amir();
	void Yuri();
	void Anat();
	void Vasily();
    void Da_UTF8();
    void ExAscii();
    void Judith();

protected:

	char	m_szNumericId[H243_NAME_LEN];
	char	m_szName[H243_NAME_LEN];
	char    m_szDisplayName[H243_NAME_LEN];
	WORD	m_wMinParties;
	WORD	m_wMaxParties;
	char	m_szChairPassword[H243_NAME_LEN];
	char	m_szEntryPassword[H243_NAME_LEN];
	BOOL	m_bIsLeader;
	BOOL	m_bIsGuest;
	BOOL	m_bIsVip;
	char	m_szBillingData[H243_NAME_LEN];
	char	m_szOwner[H243_NAME_LEN];
	char	m_szPhone1[H243_NAME_LEN];
	char	m_szPhone2[H243_NAME_LEN];
	char	m_aszInfo[4][H243_NAME_LEN];
};


/////////////////////////////////////////////////////////////////////////////
class CExtDbSimulator : public CSerializeObject
{
CLASS_TYPE_1(CExtDbSimulator,CSerializeObject)
public:
	CExtDbSimulator();
	virtual ~CExtDbSimulator();

	virtual const char* NameOf() const { return "CExtDbSimulator";}
	virtual CSerializeObject* Clone() { return NULL; }

	void Serialize(CSegment& segment) const;

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(ENDPOINTS_SIM_EXTDB_FILE_NAME); }
	void WriteXmlFile() { CSerializeObject::WriteXmlFile(ENDPOINTS_SIM_EXTDB_FILE_NAME,"ENDPOINTS_SIM_EXTERNAL_DB"); }

	STATUS ProcessRequest(CCommSetExternalDbCreate* pReqCreate) const;
	STATUS ProcessRequest(CCommSetExternalDbAdd* pReqAdd) const;
	STATUS ProcessRequest(CCommSetExternalDbUser* pReqUser) const;
	void KeepAlive();
	void KeepAliveTimer();

	DWORD GetConnectionStatus() const { return m_connectionStatus; }

protected:
	DWORD			m_connectionStatus;
	CExtDbRecord	m_records[MAX_ETX_DB_RECORDS];
};





#endif /* __EXTERNALDBSIM_H__ */







