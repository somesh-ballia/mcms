//+========================================================================+
//                GideonSimCardAckStatusList.h                             |
//-------------------------------------------------------------------------|
// FILE:       GideonSimCardAckStatusList.h                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 21.03.06   |                                                      |
//+========================================================================+


#ifndef __GIDEON_SIM_CARD_ACK_STATUS_LIST_H_
#define __GIDEON_SIM_CARD_ACK_STATUS_LIST_H_


#include <vector>
#include "PObject.h"


class CGideonSimCardAckStatus;

template class std::vector < CGideonSimCardAckStatus* > ;



////////////////////////////////////////////////////////////////////////////
//  List of statuses
////////////////////////////////////////////////////////////////////////////
class CGideonSimCardAckStatusList : public CPObject
{
	CLASS_TYPE_1(CGideonSimCardAckStatusList,CPObject)
public: 
	
	// Constructors
	CGideonSimCardAckStatusList();
	virtual const char* NameOf() const { return "CGideonSimCardAckStatusList";}
	virtual ~CGideonSimCardAckStatusList();
	// Operations
	

	void NewStatusReceived(const DWORD opcode,const BOOL isSendAck,const DWORD status);
	const CGideonSimCardAckStatus* GetAckStatus(const DWORD opcode) const;

protected:

		// utilities
	STATUS	AddElement(CGideonSimCardAckStatus* pStatusElement);
	void	ClearFirstElemFromVector();
	void	ClearVector();

		// Attributes
	std::vector<CGideonSimCardAckStatus*>*		m_pVector;
};


////////////////////////////////////////////////////////////////////////////
//  Element of list
////////////////////////////////////////////////////////////////////////////
class CGideonSimCardAckStatus : public CPObject
{
	CLASS_TYPE_1(CGideonSimCardAckStatus,CPObject)
public: 
	
	// Constructors
	CGideonSimCardAckStatus(const DWORD opcode,const BOOL isSendAck,const DWORD status);
	virtual const char* NameOf() const { return "CGideonSimCardAckStatus";}
	CGideonSimCardAckStatus(const CGideonSimCardAckStatus& other);
	virtual ~CGideonSimCardAckStatus();
	CGideonSimCardAckStatus& operator= (const CGideonSimCardAckStatus& other);

	// Operations

	// Get / Set
	DWORD	GetOpcode() const { return m_opcode; }
	BOOL	GetIsSendAck() const { return m_isSendAck; }
	DWORD	GetStatus() const { return m_status; }

protected:
		// Attributes
	DWORD	m_opcode;
	BOOL	m_isSendAck;
	DWORD	m_status;
};




#endif // __GIDEON_SIM_CARD_ACK_STATUS_LIST_H_


