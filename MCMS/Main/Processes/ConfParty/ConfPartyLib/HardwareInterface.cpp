#include "HardwareInterface.h"

#include "MplMcmsProtocolTracer.h"
#include "ConfPartyMplMcmsProtocolTracer.h"

#include "StatusesGeneral.h"
#include "OpcodesMcmsVideo.h"

#include "ProcessBase.h"

#include "Macros.h"

#include "Trace.h"
#include "TraceStream.h"

#include <sstream>

//--------------------------------------------------------------------------
CHardwareInterface::CHardwareInterface()
	: m_pRsrcParams(NULL)
{}

//--------------------------------------------------------------------------
CHardwareInterface::~CHardwareInterface()
{
	POBJDELETE(m_pRsrcParams);
}

//--------------------------------------------------------------------------
CHardwareInterface::CHardwareInterface(const CHardwareInterface& rhs)
                   :CInterface(rhs)
{
	m_pRsrcParams = NULL;

	if (rhs.m_pRsrcParams)
		m_pRsrcParams = new CRsrcParams(*rhs.m_pRsrcParams);
}

//--------------------------------------------------------------------------
void CHardwareInterface::Create(CRsrcParams* pRsrcParams)
{
	POBJDELETE(m_pRsrcParams);
	m_pRsrcParams = new CRsrcParams(*pRsrcParams);
}

//--------------------------------------------------------------------------
// Function Does Not delete pParams after sending - need to delete from outside func
DWORD CHardwareInterface::SendMsgToMPL(OPCODE opcode, CSegment* pParams, WORD new_card_board_id)
{
	if (!IsValidParams())
		return 0;

	CMplMcmsProtocol protocol;
	protocol.AddCommonHeader(opcode);
	protocol.AddMessageDescriptionHeader();

	protocol.AddPortDescriptionHeader(
		m_pRsrcParams->GetPartyRsrcId(),
		m_pRsrcParams->GetConfRsrcId(),
		m_pRsrcParams->GetConnectionId(),
		m_pRsrcParams->GetLogicalRsrcType(),
		0, 0, 0, 0, 0, m_pRsrcParams->GetRoomId());

	if (pParams)
	{
		const size_t size = pParams->GetWrtOffset() - pParams->GetRdOffset();
		protocol.AddData(size, (const char*)pParams->GetPtr(true));
	}

	if (new_card_board_id != WORD(-1))
		protocol.SetPhysicalInfoHeaderBoard_id(new_card_board_id);

	const CProcessBase* pProcess = CProcessBase::GetProcess();

	if (VIDEO_ENCODER_CHANGE_LAYOUT_REQ != opcode)
		CConfPartyMplMcmsProtocolTracer(protocol).TraceMplMcmsProtocol("***CHardwareInterface::SendMsgToMPL");
	else
		TRACEINTO << pProcess->GetOpcodeAsString(opcode);

	STATUS status = protocol.SendMsgToMplApiCommandDispatcher();

	// VNGR-24270 - move assert to trace - we have assert on socket block in lower level - this one is only informal, no treatment but assert
	if (STATUS_OK != status)
	{
		TRACEINTO
			<< "Failed"
			<< "\n  Status          :" << pProcess->GetStatusAsString(status) << " (#" << status << ")"
			<< "\n  Opcode          :" << pProcess->GetOpcodeAsString(opcode) << " (#" << opcode << ")"
			<< "\n  PartyRsrcId     :" << m_pRsrcParams->GetPartyRsrcId()
			<< "\n  ConfRsrcId      :" << m_pRsrcParams->GetConfRsrcId()
			<< "\n  ConnectionId    :" << m_pRsrcParams->GetConnectionId()
			<< "\n  LogicalRsrcType :" << m_pRsrcParams->GetLogicalRsrcType()
			<< "\n  RoomId          :" << m_pRsrcParams->GetRoomId()
			<< "\n  PhysicalBoardId :" << new_card_board_id;
	}

	return protocol.getMsgDescriptionHeaderRequest_id();
}

//--------------------------------------------------------------------------
DWORD CHardwareInterface::GetConfRsrcId() const
{
	return (m_pRsrcParams) ? m_pRsrcParams->GetConfRsrcId() : 0;
}

//--------------------------------------------------------------------------
DWORD CHardwareInterface::GetPartyRsrcId() const
{
	return (m_pRsrcParams) ? m_pRsrcParams->GetPartyRsrcId() : 0;
}

//--------------------------------------------------------------------------
DWORD CHardwareInterface::GetConnectionId() const
{
	return (m_pRsrcParams) ? m_pRsrcParams->GetConnectionId() : 0;
}

//--------------------------------------------------------------------------
void CHardwareInterface::SetPartyRsrcId(DWORD partyRsrcId)
{
	if (m_pRsrcParams)
		m_pRsrcParams->SetPartyRsrcId(partyRsrcId);
}

//--------------------------------------------------------------------------
void CHardwareInterface::SetConfRsrcId(DWORD confRsrcId)
{
	if (m_pRsrcParams)
		m_pRsrcParams->SetConfRsrcId(confRsrcId);
}

//--------------------------------------------------------------------------
void CHardwareInterface::SetRoomId(WORD roomID)
{
	if (m_pRsrcParams)
		m_pRsrcParams->SetRoomId(roomID);
}

//--------------------------------------------------------------------------
BOOL CHardwareInterface::IsValidParams()
{
	PASSERT_AND_RETURN_VALUE(!m_pRsrcParams, FALSE);
	PASSERT_AND_RETURN_VALUE(!m_pRsrcParams->GetRsrcDesc(), FALSE);
	return TRUE;
}

//--------------------------------------------------------------------------
void CHardwareInterface::UpdateRsrcParams(CRsrcParams* pRsrcParams)
{
	m_pRsrcParams = pRsrcParams;
}

