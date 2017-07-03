//
// CommGideonSimSet.h: 
//
//
//Date       Description
//====		 ===========
//1/1/06	 Used in XML transaction. 
//
//========   =====================================================================



#ifndef __COMM_GIDEON_SIM_SET_H_
#define __COMM_GIDEON_SIM_SET_H_

#include "SerializeObject.h"



// Connect / Disconnect Sockets to MPL-API (Rx / Tx / Rx+Tx)
//////////////////////////////////////////////////////////////////////
class CCommSetDisconnectOrConnectSockets : public CSerializeObject
{
CLASS_TYPE_1(CCommSetDisconnectOrConnectSockets,CSerializeObject)
public:
	CCommSetDisconnectOrConnectSockets();
	CCommSetDisconnectOrConnectSockets(const CCommSetDisconnectOrConnectSockets& other);
	virtual ~CCommSetDisconnectOrConnectSockets();
	CCommSetDisconnectOrConnectSockets& operator =(const CCommSetDisconnectOrConnectSockets& other);
	virtual CSerializeObject* Clone() { return new CCommSetDisconnectOrConnectSockets; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

public:
	DWORD GetSlots() {return m_slots;}
	DWORD GetConnectOrDisconnect() {return m_connectOrDisconnect;}
	DWORD GetTxRx() { return m_txrxAction;}
	int IsSlotInList( DWORD slotNum );
	int IsTxAction();
	int IsRxAction();
	
protected:
	DWORD m_slots;					// 0x1 = 1, ox10=2, 0x100=3 etc.(e.g. 0x01101=1,3,4)
	DWORD m_connectOrDisconnect;	// 0=disconnect, 1=connect
	DWORD m_txrxAction;				// 0x1=rx, 0x10=tx, 0x11=rxtx
};

// Connect / Disconnect Sockets to MPL-API (Rx / Tx / Rx+Tx)
//////////////////////////////////////////////////////////////////////
class CSetUnitStatus : public CSerializeObject
{
CLASS_TYPE_1(CSetUnitStatus,CSerializeObject)
public:
	CSetUnitStatus();
	CSetUnitStatus(const CSetUnitStatus& other);
	virtual ~CSetUnitStatus();
	CSetUnitStatus& operator =(const CSetUnitStatus& other);
	virtual CSerializeObject* Clone() { return new CSetUnitStatus; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

public:
	DWORD GetSlots() {return m_slots;}
	DWORD GetUnit() {return m_unit;}
	DWORD GetUnitStatus() { return m_status;}
	
	
protected:
	DWORD m_slots;				
	DWORD m_unit;
	DWORD m_status;				
};

// Connect / Disconnect Sockets to MPL-API (Rx / Tx / Rx+Tx)
//////////////////////////////////////////////////////////////////////
class CCommSetCardAckStatus : public CSerializeObject
{
CLASS_TYPE_1(CCommSetCardAckStatus,CSerializeObject)
public:
	CCommSetCardAckStatus();
	CCommSetCardAckStatus(const CCommSetCardAckStatus& other);
	virtual ~CCommSetCardAckStatus();
	CCommSetCardAckStatus& operator =(const CCommSetCardAckStatus& other);
	virtual CSerializeObject* Clone() { return new CCommSetCardAckStatus; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

public:
	DWORD GetOpcode() const { return m_opcode; }
	BOOL  GetIsSendAck() const { return m_isSendAck; }
	DWORD GetStatus() const { return m_status; }
	
	
protected:
	DWORD	m_opcode;
	BOOL	m_isSendAck;
	DWORD	m_status;
};


// Update ISDN timers
//////////////////////////////////////////////////////////////////////
class CCommSetIsdnTimers : public CSerializeObject
{
CLASS_TYPE_1(CCommSetIsdnTimers,CSerializeObject)
public:
	CCommSetIsdnTimers();
	CCommSetIsdnTimers(const CCommSetIsdnTimers& other);
	virtual ~CCommSetIsdnTimers();
	CCommSetIsdnTimers& operator =(const CCommSetIsdnTimers& other);
	virtual const char*  NameOf() const { return "CCommSetIsdnTimers"; }
	virtual CSerializeObject* Clone() { return new CCommSetIsdnTimers; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

public:
	DWORD GetBndRmtLclAlignment() const { return m_isdnBndRemoteLocalAlignmentTimer; }
	
	
protected:
	DWORD	m_isdnBndRemoteLocalAlignmentTimer;
	
};
//////////////////////////////////////////////////////////////////////


// Update ISDN timers
//////////////////////////////////////////////////////////////////////
class CCommSetEnableDisableRtmPorts : public CSerializeObject
{
CLASS_TYPE_1(CCommSetEnableDisableRtmPorts,CSerializeObject)
public:
	CCommSetEnableDisableRtmPorts();
	CCommSetEnableDisableRtmPorts(const CCommSetEnableDisableRtmPorts& other);
	virtual ~CCommSetEnableDisableRtmPorts();
	CCommSetEnableDisableRtmPorts& operator =(const CCommSetEnableDisableRtmPorts& other);
	virtual const char*  NameOf() const { return "CCommSetEnableDisableRtmPorts"; }
	virtual CSerializeObject* Clone() { return new CCommSetEnableDisableRtmPorts; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

public:
	DWORD GetCard() const { return m_card; }
	DWORD GetSpan() const { return m_span; }
	DWORD GetFirstPort() const { return m_firstPort; }
	DWORD GetNumPorts() const { return m_numOfPorts; }
	DWORD GetAction() const { return m_action; }
	
	
protected:
	DWORD	m_card;
	DWORD	m_span;
	DWORD	m_firstPort;
	DWORD	m_numOfPorts;
	DWORD	m_action;
	
};
//////////////////////////////////////////////////////////////////////


// Insert Card Event
//////////////////////////////////////////////////////////////////////

/*typedef enum
{
	eInsertEvent = 0,
	eRemoveEvent,
} eEventStatus;*/

class CCommInsertCardEvent : public CSerializeObject
{
CLASS_TYPE_1(CCommInsertCardEvent,CSerializeObject)
public:
	CCommInsertCardEvent();
	CCommInsertCardEvent(const CCommInsertCardEvent& other);
	virtual ~CCommInsertCardEvent();
	CCommInsertCardEvent& operator =(const CCommInsertCardEvent& other);
	virtual const char*  NameOf() const { return "CCommInsertCardEvent"; }
	virtual CSerializeObject* Clone() { return new CCommInsertCardEvent; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

public:
	BYTE GetBoadrId() const { return m_boardId; }
	BYTE GetSubBoadrId() const { return m_subBoardId; }
	BYTE GetCardType() const { return m_cardType; }

	//void SetEventState(eEventStatus eventStatus) { m_event =  eventStatus; }
	
	
protected:
	BYTE	m_boardId;
	BYTE	m_subBoardId;
	BYTE    m_cardType;
	
	//eEventStatus  m_event;
	
};

//////////////////////////////////////////////////////////////////////

// Remove Card Event
//////////////////////////////////////////////////////////////////////


class CCommRemoveCardEvent : public CSerializeObject
{
CLASS_TYPE_1(CCommRemoveCardEvent,CSerializeObject)
public:
	CCommRemoveCardEvent();
	CCommRemoveCardEvent(const CCommRemoveCardEvent& other);
	virtual ~CCommRemoveCardEvent();
	CCommRemoveCardEvent& operator =(const CCommRemoveCardEvent& other);
	virtual const char*  NameOf() const { return "CCommRemoveCardEvent"; }
	virtual CSerializeObject* Clone() { return new CCommRemoveCardEvent; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

public:
	BYTE GetBoadrId() const { return m_boardId; }
	BYTE GetSubBoadrId() const { return m_subBoardId; }
	BOOL GetIsHandleRemove() const { return m_isHandleRemove; }
	
protected:
	BYTE	m_boardId;
	BYTE	m_subBoardId;
	BOOL    m_isHandleRemove;	
	
};



#endif /*__COMM_GIDEON_SIM_SET_H_*/
