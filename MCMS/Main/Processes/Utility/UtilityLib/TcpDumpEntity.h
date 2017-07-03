#ifndef TCP_DUMP_ENTITY_H__
#define TCP_DUMP_ENTITY_H__

#include "psosxml.h"
#include "DefinesGeneral.h"
#include "SerializeObject.h"
#include "InitCommonStrings.h"
#include "UtilityProcessDefines.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "StructTm.h"
#include "DefinesIpService.h"
#include "CommonStructs.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
class CIPEntity : public CSerializeObject
{
	CLASS_TYPE_1(CIPEntity, CSerializeObject)

	virtual const char* NameOf() const
	{ return "CIPEntity"; }

	virtual void Dump(std::ostream& msg) const;

public:

	CIPEntity();

	bool operator ==(const CIPEntity& rHnd)const;
	bool operator !=(const CIPEntity& other);

	void SerializeXml(CXMLDOMElement*& pFatherNode, DWORD ObjToken) const;
	int  DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError,const char* action);
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;

	CSerializeObject* Clone()
	{ return new CIPEntity(*this); }

	BYTE  GetChanged() const;
	DWORD GetUpdateCounter() const;

	void SetIpAddress(DWORD ipaddress)
	{
		m_ipAddress.addr.v4.ip = ipaddress;
		m_ipAddress.ipVersion = eIpVersion4;
	}

	void SetIpV6Address(char* ipV6addr);

	DWORD GetIpAddress() const
	{ return m_ipAddress.addr.v4.ip; }

	BYTE GetIsSelected() const
	{ return m_selected; }

	void InitMembers();
	void ClearMembers();

private:

	ipAddressStruct m_ipAddress;
	BYTE            m_selected;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
class CTcpDumpEntity : public CSerializeObject
{
	CLASS_TYPE_1(CTcpDumpEntity, CSerializeObject)

	virtual const char* NameOf() const
	{ return "CTcpDumpEntity";}

	virtual void Dump(std::ostream& msg) const;

public:

	CTcpDumpEntity();
	CTcpDumpEntity(const CTcpDumpEntity& other);
	CTcpDumpEntity& operator =(const CTcpDumpEntity& other);

	bool operator ==(const CTcpDumpEntity &rHnd)const;
	bool operator !=(const CTcpDumpEntity& other);

	virtual ~CTcpDumpEntity();

	void SerializeXml(CXMLDOMElement*& pFatherNode, DWORD ObjToken) const;
	int  DeSerializeXml(CXMLDOMElement* pActionNode, char *pszError, const char* action);
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;

	CSerializeObject* Clone()
	{ return new CTcpDumpEntity(*this); }

	BYTE  GetChanged() const;
	DWORD GetUpdateCounter() const;
	void  InitMembers();
	void  ClearMembers();

	void AddIpEntity(DWORD ipAddress);
	void AddIpV6Entity(char* ipV6Address);

	void SetEntityType(eEntityType type)
	{ m_entityType = type; }

	eEntityType GetEntityType() const
	{ return m_entityType; }

	void SetBoardId(DWORD boardId)
	{ m_boardId = boardId; }

	DWORD GetBoardId() const
	{ return m_boardId; }

	void SetIpType(eIpType ipType)
	{ m_ipType = ipType; }

	eIpType GetIpType() const
	{ return m_ipType; }

	char* GetFilter()
	{ return m_filter; }

	const std::string GetTcpDumpEntityName() const;
	void SetTcpDumpState(eTcpDumpState state);
	eTcpDumpState GetTcpDumpState();

public:

	CIPEntity*  m_pIPList[MAX_SPAN_NUMBER_IN_SERVICE];

private:

	DWORD         m_boardId;
	char          m_filter[ONE_LINE_BUFFER_LEN];
	eEntityType   m_entityType;
	eIpType       m_ipType;
	eTcpDumpState m_tcpDumpState;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
class CTcpDumpEntityList : public CSerializeObject
{
	friend bool operator ==(const CTcpDumpEntityList& first,const CTcpDumpEntityList& second);

	CLASS_TYPE_1(CTcpDumpEntityList, CSerializeObject)

	const char* NameOf() const
	{ return "CTcpDumpEntityList"; }

	virtual void Dump(std::ostream& msg) const;

public:

	CTcpDumpEntityList();
	CTcpDumpEntityList(const CTcpDumpEntityList& other);

	virtual ~CTcpDumpEntityList();

	CTcpDumpEntityList& operator =(const CTcpDumpEntityList& rOther);

	virtual CSerializeObject* Clone()
	{ return new CTcpDumpEntityList; }

	virtual void SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);

	void InitMembers();
	void ClearMembers();

	void SetTcpDumpState(eTcpDumpState state)
	{ m_tcpDump_State = state; }

	eTcpDumpState GetTcpDumpState() const
	{ return m_tcpDump_State; }

	void SetMaxCaptureDuration(eMaxCaptureDurationType type)
	{ m_max_capture_duration = type; }

	eMaxCaptureDurationType GetMaxCaptureDuration() const
	{ return m_max_capture_duration; }

	void SetMaxCaptureSize(eMaxCaptureSizeType type)
	{ m_max_capture_size = type; }

	eMaxCaptureSizeType GetMaxCaptureSize() const
	{ return m_max_capture_size; }

	void SetTimeElapsed(CStructTm time)
	{ m_time_elapsed = time; }

	CStructTm GetTimeElapsed() const
	{ return m_time_elapsed; }

	void SetStorageInUsed(WORD size)
	{ m_storage_in_used = size; }

	WORD GetStorageInUsed() const
	{ return m_storage_in_used; }

public:

	CTcpDumpEntity* m_pTcpDumpEntities[MAX_NUM_OF_ENTITIES];

private:

	eTcpDumpState           m_tcpDump_State;
	BYTE                    m_cyclic_storage;
	CStructTm               m_time_elapsed;
	WORD                    m_storage_in_used;
	eMaxCaptureSizeType     m_max_capture_size;
	eMaxCaptureDurationType m_max_capture_duration;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
class CTcpDumpStatus : public CSerializeObject
{
	CLASS_TYPE_1(CTcpDumpStatus, CSerializeObject)

	const char* NameOf() const
	{ return "CTcpDumpStatus"; }

	virtual void Dump(std::ostream& msg) const;

public:

	CTcpDumpStatus();
	CTcpDumpStatus(const CTcpDumpStatus& other);

	CTcpDumpStatus& operator =(const CTcpDumpStatus& rOther);

	virtual CSerializeObject* Clone()
	{ return new CTcpDumpStatus; }

	virtual void SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);

	friend bool operator ==(const CTcpDumpStatus& first, const CTcpDumpStatus& second);

	void InitMembers();
	void ClearMembers();

	void SetTcpDumpState(eTcpDumpState state, const char* descTemplate = NULL, const char* entityName = NULL);

	eTcpDumpState GetTcpDumpState() const
	{ return m_tcpDump_State; }

	void SetTimeElapsed(CStructTm time)
	{ m_time_elapsed = time; }

	CStructTm GetTimeElapsed() const
	{ return m_time_elapsed; }

	void SetIsStartOn(bool isStart)
	{ m_isStartOn = isStart; }

	bool GetIsStartOn() const
	{ return m_isStartOn; }

	void SetStorageInUsed(WORD size)
	{ m_storage_in_used = size; }

	WORD GetStorageInUsed() const
	{ return m_storage_in_used; }

	void SetDescription(const char* desc);

	const char* GetDescription() const
	{ return m_description; }

private:

	eTcpDumpState  m_tcpDump_State;
	char           m_description[ONE_LINE_BUFFER_LEN];
	CStructTm      m_time_elapsed;
	WORD           m_storage_in_used;
	bool           m_isStartOn;
};

#endif // TCP_DUMP_ENTITY_H__
