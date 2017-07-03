//+========================================================================+
//                GideonSimCardAckStatusList.cpp                           |
//-------------------------------------------------------------------------|
// FILE:       GideonSimCardAckStatusList.cpp                              |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 21.03.06   |                                                      |
//+========================================================================+


#include "StatusesGeneral.h"
#include "TraceStream.h"
#include "GideonSim.h"
#include "GideonSimCardAckStatusList.h"


////////////////////////////////////////////////////////////////////////////
CGideonSimCardAckStatusList::CGideonSimCardAckStatusList()
{
	m_pVector = new std::vector<CGideonSimCardAckStatus*>;
	::SetCardAckStatusList(this);
}

////////////////////////////////////////////////////////////////////////////
CGideonSimCardAckStatusList::~CGideonSimCardAckStatusList()
{
	::SetCardAckStatusList(NULL);
	ClearVector();
    PDELETE(m_pVector);
}

////////////////////////////////////////////////////////////////////////////
void CGideonSimCardAckStatusList::ClearVector()
{
	while (!m_pVector->empty())
	{
		ClearFirstElemFromVector();
	}
	m_pVector->clear();
}

////////////////////////////////////////////////////////////////////////////
void CGideonSimCardAckStatusList::ClearFirstElemFromVector()
{
	std::vector<CGideonSimCardAckStatus*>::iterator itr = m_pVector->begin();
//	CGideonSimCardAckStatus *pElem = *itr;

	m_pVector->erase(itr);
}

////////////////////////////////////////////////////////////////////////////
STATUS CGideonSimCardAckStatusList::AddElement(CGideonSimCardAckStatus* pStatusElement)
{
	if( pStatusElement == NULL )
		return STATUS_FAIL;

	if( !CPObject::IsValidPObjectPtr(pStatusElement) )
		return STATUS_FAIL;

	TRACEINTO << " CGideonSimCardAckStatusList::AddElement - ADD:\n"
			<< "opcode <" << (int)pStatusElement->GetOpcode() << ">, "
			<< "to send <" << ((pStatusElement->GetIsSendAck()==TRUE)? "yes": "no") << ">, "
			<< "status <" << (int)pStatusElement->GetStatus() << ">.";

	m_pVector->push_back(pStatusElement);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CGideonSimCardAckStatusList::NewStatusReceived( const DWORD opcode,
		const BOOL isSendAck, const DWORD status )
{
	std::vector<CGideonSimCardAckStatus*>::iterator itr = m_pVector->begin();
	std::vector<CGideonSimCardAckStatus*>::iterator itrEnd = m_pVector->end();

		// if element with this opcode exists - remove it from list
	while( itr != itrEnd )
	{
		CGideonSimCardAckStatus* pElem = *itr;
		if( pElem->GetOpcode() == opcode )
		{
			TRACEINTO << " CGideonSimCardAckStatusList::NewStatusReceived - REMOVE OLD:\n"
					<< "opcode <" << (int)pElem->GetOpcode() << ">, "
					<< "to send <" << ((pElem->GetIsSendAck()==TRUE)? "yes": "no") << ">, "
					<< "status <" << (int)pElem->GetStatus() << ">.";
			m_pVector->erase(itr);
			break;
		}
		itr++;
	}

		// add new element to list
	if( TRUE != isSendAck || STATUS_OK != status )
	{
		CGideonSimCardAckStatus* pElem = new CGideonSimCardAckStatus(opcode,isSendAck,status);
		AddElement(pElem);
	}
	else
	{
			TRACEINTO << " CGideonSimCardAckStatusList::NewStatusReceived - NOT TO ADD:\n"
					<< "opcode <" << (int)opcode << ">, "
					<< "to send <" << ((isSendAck==TRUE)? "yes": "no") << ">, "
					<< "status <" << (int)status << ">.";
	}
	// else - not add to list (regular case)
}

////////////////////////////////////////////////////////////////////////////
const CGideonSimCardAckStatus* CGideonSimCardAckStatusList::GetAckStatus(const DWORD opcode) const
{
	std::vector<CGideonSimCardAckStatus*>::iterator itr = m_pVector->begin();
	std::vector<CGideonSimCardAckStatus*>::iterator itrEnd = m_pVector->end();

		// if element with this opcode exists - return
	while( itr != itrEnd )
	{
		CGideonSimCardAckStatus* pElem = *itr;
		if( pElem->GetOpcode() == opcode )
			return pElem;
		itr++;
	}
	return NULL;
}


////////////////////////////////////////////////////////////////////////////
CGideonSimCardAckStatus::CGideonSimCardAckStatus(const DWORD opcode,const BOOL isSendAck,const DWORD status)
	: m_opcode(opcode), m_isSendAck(isSendAck), m_status(status)
{
}

//////////////////////////////////////////////////////////////////////
CGideonSimCardAckStatus::CGideonSimCardAckStatus(const CGideonSimCardAckStatus& other) : CPObject(other)
{
	*this = other;
}

////////////////////////////////////////////////////////////////////////////
CGideonSimCardAckStatus::~CGideonSimCardAckStatus()
{
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimCardAckStatus& CGideonSimCardAckStatus::operator= (const CGideonSimCardAckStatus& other)
{
	if( this == &other )
		return *this;

	m_opcode    = other.m_opcode;
	m_isSendAck = other.m_isSendAck;
	m_status    = other.m_status;

	return *this;
}


