// CommGideonSimSet.cpp: 
//
//
// Date         Description
// ========   =====================================================================
// 1/1/06     Used in XML transaction. 
//
// ========   =====================================================================


#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CommGideonSimSet.h"
#include "DummyEntry.h"
#include "ApiStatuses.h"
#include "CardsStructs.h"






//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//			CCommSetDisconnectOrConnectSockets - Start				//
//																	//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//  VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV		//

//////////////////////////////////////////////////////////////////////
CCommSetDisconnectOrConnectSockets::CCommSetDisconnectOrConnectSockets()
{
	m_slots = 0;					// 0x1 = 1, ox10=2, 0x100=3 etc.(e.g. 0x01101=1,3,4)
	m_connectOrDisconnect = 0;		// 0=disconnect, 1=connect
	m_txrxAction = 0;				// 0x1=rx, 0x10=tx, 0x11=rxtx
}

//////////////////////////////////////////////////////////////////////
CCommSetDisconnectOrConnectSockets::CCommSetDisconnectOrConnectSockets(const CCommSetDisconnectOrConnectSockets& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetDisconnectOrConnectSockets::~CCommSetDisconnectOrConnectSockets()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetDisconnectOrConnectSockets& CCommSetDisconnectOrConnectSockets::operator= (const CCommSetDisconnectOrConnectSockets& other)
{
	if( this == &other )
		return *this;

	m_slots = other.m_slots;
	m_connectOrDisconnect = other.m_connectOrDisconnect;
	m_txrxAction = other.m_txrxAction;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetDisconnectOrConnectSockets::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetDisconnectOrConnectSockets::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode,	"BOARD_ID", &m_slots, 				_0_TO_DWORD);
//	GET_VALIDATE_CHILD(pActionNode,	"GIDEON_SIM_YESNO", &m_connectOrDisconnect, _BOOL);
//	GET_VALIDATE_CHILD(pActionNode,	"GIDEON_SIM_RXTX", 	&m_txrxAction, 			_0_TO_DWORD);
		
	return nStatus;
}

///////////////////////////////////////////////////////////////////////
int CCommSetDisconnectOrConnectSockets::IsSlotInList( DWORD slotNum )
{
	// slotNum = 0-14
	if ((slotNum == 0) && (m_slots == 0))	// request for slot 0
		return 1;
	if (slotNum > 14)
		return 0;
		
	DWORD bitFlag[16]={ 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000 };
	DWORD result = m_slots & bitFlag[slotNum - 1];	// slot# = 1...14
	if (result)
		return 1;
	return 0;
}

//////////////////////////////////////////////////////////////////////
int CCommSetDisconnectOrConnectSockets::IsRxAction()
{
	DWORD result = 0x1 & m_txrxAction;
	if (result)
		return 1;
	return 0;
}

//////////////////////////////////////////////////////////////////////
int CCommSetDisconnectOrConnectSockets::IsTxAction()
{
	DWORD result = 0x10 & m_txrxAction;
	if (result)
		return 1;
	return 0;
}
//	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//																	//
//			CCommSetDisconnectOrConnectSockets - End				//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//			CSetUnitStatus - Start                  				//
//																	//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//  VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV		//

//////////////////////////////////////////////////////////////////////
CSetUnitStatus::CSetUnitStatus()
{
	m_slots = 0;					// 
	m_unit = 0;		// 
	m_status = 0;				// 
}

//////////////////////////////////////////////////////////////////////
CSetUnitStatus::CSetUnitStatus(const CSetUnitStatus& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CSetUnitStatus::~CSetUnitStatus()
{
}

/////////////////////////////////////////////////////////////////////////////
CSetUnitStatus& CSetUnitStatus::operator= (const CSetUnitStatus& other)
{
	if( this == &other )
		return *this;

	m_slots  = other.m_slots;
	m_unit   = other.m_unit;
	m_status = other.m_status;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CSetUnitStatus::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CSetUnitStatus::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode,	"BOARD_ID",&m_slots, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,	"UNIT_ID", &m_unit, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,	"STATUS",  &m_status, _0_TO_DWORD);
	
	return nStatus;
}

//	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//																	//
//			CSetUnitStatus - End	                    			//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//		CCommSetCardAckStatus
//////////////////////////////////////////////////////////////////////
CCommSetCardAckStatus::CCommSetCardAckStatus()
{
	m_opcode    = 0;
	m_isSendAck = TRUE;
	m_status    = STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
CCommSetCardAckStatus::CCommSetCardAckStatus(const CCommSetCardAckStatus& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetCardAckStatus::~CCommSetCardAckStatus()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetCardAckStatus& CCommSetCardAckStatus::operator= (const CCommSetCardAckStatus& other)
{
	if( this == &other )
		return *this;

	m_opcode    = other.m_opcode;
	m_isSendAck = other.m_isSendAck;
	m_status    = other.m_status;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetCardAckStatus::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetCardAckStatus::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD( pActionNode, "REQUEST_OPCODE", &m_opcode, _0_TO_DWORD );
	GET_VALIDATE_CHILD( pActionNode, "ACK_TO_SEND", &m_isSendAck, _BOOL );
	GET_VALIDATE_CHILD( pActionNode, "ACK_STATUS", &m_status, _0_TO_DWORD );

	return STATUS_OK;//nStatus;
}


//	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//			CCommSetIsdnTimers - Start                				//
//																	//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//  VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV		//


CCommSetIsdnTimers::CCommSetIsdnTimers()
{
	m_isdnBndRemoteLocalAlignmentTimer = 0;
}



//////////////////////////////////////////////////////////////////////
CCommSetIsdnTimers::CCommSetIsdnTimers(const CCommSetIsdnTimers& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetIsdnTimers::~CCommSetIsdnTimers()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetIsdnTimers& CCommSetIsdnTimers::operator= (const CCommSetIsdnTimers& other)
{
	if( this == &other )
		return *this;

	m_isdnBndRemoteLocalAlignmentTimer    = other.m_isdnBndRemoteLocalAlignmentTimer;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetIsdnTimers::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetIsdnTimers::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD( pActionNode, "BONDING_TIMER", &m_isdnBndRemoteLocalAlignmentTimer, _0_TO_DWORD );

	return STATUS_OK;
}


//	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//			CCommSetIsdnTimers - End                				//
//																	//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//  VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV		//




//	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//			CCommSetEnableDisableRtmPorts - Start      				//
//																	//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//  VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV		//


CCommSetEnableDisableRtmPorts::CCommSetEnableDisableRtmPorts()
{
	m_card = 0;
	m_span = 0;
	m_firstPort = 0;
	m_numOfPorts = 0;
	m_action = 0;
}



//////////////////////////////////////////////////////////////////////
CCommSetEnableDisableRtmPorts::CCommSetEnableDisableRtmPorts(const CCommSetEnableDisableRtmPorts& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommSetEnableDisableRtmPorts::~CCommSetEnableDisableRtmPorts()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommSetEnableDisableRtmPorts& CCommSetEnableDisableRtmPorts::operator= (const CCommSetEnableDisableRtmPorts& other)
{
	if( this == &other )
		return *this;

	m_card			= other.m_card;
	m_span 			= other.m_span;
	m_firstPort 	= other.m_firstPort;
	m_numOfPorts 	= other.m_numOfPorts;
	m_action 		= other.m_action;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommSetEnableDisableRtmPorts::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommSetEnableDisableRtmPorts::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD( pActionNode, "RTM_CARD", 		&m_card, 		_0_TO_DWORD );
	GET_VALIDATE_CHILD( pActionNode, "RTM_SPAN", 		&m_span, 		_0_TO_DWORD );
	GET_VALIDATE_CHILD( pActionNode, "RTM_FIRST_PORT", 	&m_firstPort, 	_0_TO_DWORD );
	GET_VALIDATE_CHILD( pActionNode, "RTM_NUM_PORTS", 	&m_numOfPorts, 	_0_TO_DWORD );
	GET_VALIDATE_CHILD( pActionNode, "RTM_ACTION", 		&m_action, 		_0_TO_DWORD );
	
	return STATUS_OK;
}


//	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//			CCommSetEnableDisableRtmPorts - End        				//
//																	//
//	|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||		//
//  VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV		//







//////////////////////////////////////////////////////////////////////
//		CCommInsertCardEvent
//////////////////////////////////////////////////////////////////////
CCommInsertCardEvent::CCommInsertCardEvent()
{
	m_boardId = 0;
	m_subBoardId = 1;
	m_cardType = eMfa_26;
}

//////////////////////////////////////////////////////////////////////
CCommInsertCardEvent::CCommInsertCardEvent(const CCommInsertCardEvent& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommInsertCardEvent::~CCommInsertCardEvent()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommInsertCardEvent& CCommInsertCardEvent::operator= (const CCommInsertCardEvent& other)
{
	if( this == &other )
		return *this;
		
	m_boardId = other.m_boardId;
	m_subBoardId = other.m_subBoardId;	
	m_cardType = other.m_cardType;
	
	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommInsertCardEvent::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommInsertCardEvent::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD( pActionNode, "BOARD_ID", &m_boardId, _0_TO_BYTE );
	GET_VALIDATE_CHILD( pActionNode, "SUB_BOARD_ID", &m_subBoardId, _0_TO_BYTE );
	GET_VALIDATE_CHILD( pActionNode, "CARD_TYPE", &m_cardType, _0_TO_BYTE );

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
//		CCommRemoveCardEvent
//////////////////////////////////////////////////////////////////////
CCommRemoveCardEvent::CCommRemoveCardEvent()
{
	m_boardId = 0;
	m_subBoardId = 1;
	m_isHandleRemove = TRUE;
}

//////////////////////////////////////////////////////////////////////
CCommRemoveCardEvent::CCommRemoveCardEvent(const CCommRemoveCardEvent& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CCommRemoveCardEvent::~CCommRemoveCardEvent()
{
}

/////////////////////////////////////////////////////////////////////////////
CCommRemoveCardEvent& CCommRemoveCardEvent::operator= (const CCommRemoveCardEvent& other)
{
	if( this == &other )
		return *this;
		
	m_boardId = other.m_boardId;
	m_subBoardId = other.m_subBoardId;
	m_isHandleRemove = other.m_isHandleRemove;
	
	return *this;
}

//////////////////////////////////////////////////////////////////////
void CCommRemoveCardEvent::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////
int CCommRemoveCardEvent::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD( pActionNode, "BOARD_ID", &m_boardId, _0_TO_BYTE );
	GET_VALIDATE_CHILD( pActionNode, "SUB_BOARD_ID", &m_subBoardId, _0_TO_BYTE );
	GET_VALIDATE_CHILD( pActionNode, "IS_HANDLE_REMOVE", &m_isHandleRemove, _BOOL );
	
	return nStatus;
}








