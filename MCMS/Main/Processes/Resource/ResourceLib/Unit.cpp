#include "Unit.h"
#include "ProcessBase.h"
#include "SystemResources.h"
#include "HelperFuncs.h"
#include "PrettyTable.h"
#include "CRsrcDetailGet.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "InternalProcessStatuses.h"

int CUnitMFA::m_MaxArtChannelsPerArt = MAX_ART_CHANNELS_PER_ART;

////////////////////////////////////////////////////////////////////////////
//                        CActivePort
////////////////////////////////////////////////////////////////////////////
CActivePort::CActivePort()
{
	m_portId                = 0;
	m_acceleratorId         = 0;
	m_confId                = 0;
	m_partyId               = 0;
	m_capacity              = 0;
	m_promilUtilized        = 0;
	m_type                  = ePhysical_res_none;
	m_utilizedBandWidth_in  = 0;
	m_utilizedBandWidth_out = 0;
	m_utilizedEncoderWeight = 0;
}

////////////////////////////////////////////////////////////////////////////
CActivePort::CActivePort(PortID portId, AcceleratorID acceleratorId, ConfRsrcID confId, PartyRsrcID partyId, DWORD capacity, float promil, eResourceTypes porType, DWORD utilizedBandWidth_in, DWORD utilizedBandWidth_out, float utilizedEncoderWeight)
{
	m_portId                = portId;
	m_acceleratorId         = acceleratorId;
	m_confId                = confId;
	m_partyId               = partyId;
	m_type                  = porType;
	m_capacity              = capacity;
	m_promilUtilized        = promil;
	m_utilizedBandWidth_in  = utilizedBandWidth_in;
	m_utilizedBandWidth_out = utilizedBandWidth_out;
	m_utilizedEncoderWeight = utilizedEncoderWeight;
}

////////////////////////////////////////////////////////////////////////////
CActivePort::~CActivePort()
{
}

////////////////////////////////////////////////////////////////////////////
CActivePort::CActivePort(const CActivePort& other)
	: CPObject(other)
{
	m_portId                = other.m_portId;
	m_acceleratorId         = other.m_acceleratorId;
	m_confId                = other.m_confId;
	m_partyId               = other.m_partyId;
	m_type                  = other.m_type;
	m_capacity              = other.m_capacity;
	m_promilUtilized        = other.m_promilUtilized;
	m_utilizedBandWidth_in  = other.m_utilizedBandWidth_in;
	m_utilizedBandWidth_out = other.m_utilizedBandWidth_out;
	m_utilizedEncoderWeight = other.m_utilizedEncoderWeight;
}

////////////////////////////////////////////////////////////////////////////
void CActivePort::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left, std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\nCActivePort::Dump:"
	    << "\n  Port Id                 = " << m_portId
	    << "\n  Accelerator Id          = " << m_acceleratorId
	    << "\n  Conf Id                 = " << m_confId
	    << "\n  Party Id                = " << m_partyId
	    << "\n  Port Type               = " << m_type
	    << "\n  Port Capacity (Kbytes)  = " << m_capacity
	    << "\n  Utilized Promillies     = " << m_promilUtilized
	    << "\n  Utilized BandWidth In   = " << m_utilizedBandWidth_in
	    << "\n  Utilized BandWidth Out  = " << m_utilizedBandWidth_out
	    << "\n  Utilized Encoder Weight = " << m_utilizedEncoderWeight;
}

////////////////////////////////////////////////////////////////////////////
WORD operator ==(const CActivePort& lhs, const CActivePort& rhs)
{
	return lhs.m_portId == rhs.m_portId;
}

////////////////////////////////////////////////////////////////////////////
bool operator <(const CActivePort& lhs, const CActivePort& rhs)
{
	return lhs.m_portId < rhs.m_portId;
}


////////////////////////////////////////////////////////////////////////////
//                        CUnitRsrc
////////////////////////////////////////////////////////////////////////////
CUnitRsrc::CUnitRsrc(WORD bId, WORD uId, WORD boxId, WORD subBoardId, CTaskApp* pOwnerTask) : CStateMachine(pOwnerTask)
{
	m_boardId            = bId;
	m_unitId             = uId;
	m_isEnabled          = TRUE;
	m_isDisabledManually = FALSE;
	m_isFatal            = FALSE;
	m_boxId              = boxId;
	m_SubBoardId         = subBoardId;
}

////////////////////////////////////////////////////////////////////////////
CUnitRsrc::CUnitRsrc(const CUnitRsrc& other) : CStateMachine(other)
{
	m_boardId            = other.m_boardId;
	m_unitId             = other.m_unitId;
	m_isEnabled          = other.m_isEnabled;
	m_isDisabledManually = other.m_isDisabledManually;
	m_isFatal            = other.m_isFatal;

	m_boxId      = other.m_boxId;
	m_SubBoardId = other.m_SubBoardId;

	CTaskApp* task = CProcessBase::GetProcess()->GetCurrentTask();
	RegisterInTask(task);
}

////////////////////////////////////////////////////////////////////////////
CUnitRsrc::~CUnitRsrc()
{
}

////////////////////////////////////////////////////////////////////////////
void* CUnitRsrc::GetMessageMap()
{
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
const char* CUnitMFA::NameOf() const
{
	return "CUnitMFA";
}

////////////////////////////////////////////////////////////////////////////
void CUnitRsrc::HandleEvent (CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

////////////////////////////////////////////////////////////////////////////
void CUnitRsrc::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left, std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n\n"
	    << "CUnitRsrc::Dump\n"
	    << "-----------------------\n";

	msg << std::setw(20) << "Box Id: " << m_boxId << "\n"
	    << std::setw(20) << "Board Id: " << m_boardId << "\n"
	    << std::setw(20) << "SubBoard Id: " << m_SubBoardId << "\n"
	    << std::setw(20) << "Unit Id: " << m_unitId << "\n"
	    << std::setw(20) << "Is Enabled: " << m_isEnabled << "\n"
	    << std::setw(20) << "Is Disabled Manually: " << m_isDisabledManually << "\n"
	    << std::setw(20) << "Is Fatal: " << m_isFatal << "\n";

	msg << "\n\n";
}

////////////////////////////////////////////////////////////////////////////
CBoard* CUnitRsrc::GetBoard() const
{
	CSystemResources* pResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(pResources == NULL, NULL);

	CBoard* pBoard = pResources->GetBoard(m_boardId);
	if (pBoard == NULL)
	{
		string buff = "Board is null : ";
		buff += m_boardId;
		PASSERTMSG(1, buff.c_str());
	}
	return pBoard;
}

////////////////////////////////////////////////////////////////////////////
BYTE CUnitRsrc::GetIsEnabled() const
{
	return (m_isEnabled && !m_isDisabledManually && !m_isFatal);
}

////////////////////////////////////////////////////////////////////////////
void CUnitRsrc::SetDisabledManually(BYTE isDisable)
{
	TRACEINTO << "boardId = " << m_boardId << ", unitId = " << m_unitId << " was DisabledManually = " << (WORD)isDisable;
	m_isDisabledManually = isDisable;
}

////////////////////////////////////////////////////////////////////////////
WORD operator ==(const CUnitRsrc& lhs, const CUnitRsrc& rhs)
{
	if ((lhs.m_boardId == rhs.m_boardId) && (lhs.m_unitId == rhs.m_unitId))
		return TRUE;  // if needed (lhs.m_UnitType == rhs.m_UnitType) - later;
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
bool operator <(const CUnitRsrc& lhs, const CUnitRsrc& rhs)
{
	if (lhs.m_boardId != rhs.m_boardId)
		return lhs.m_boardId < rhs.m_boardId;

	return lhs.m_unitId < rhs.m_unitId;
}


PBEGIN_MESSAGE_MAP(CUnitMFA)

	ONEVENT(RECOVERY_UNITS_TIMER,  ANYCASE,  CUnitMFA::OnRecoveryUnitTimer)

PEND_MESSAGE_MAP(CUnitMFA, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CUnitMFA
////////////////////////////////////////////////////////////////////////////
CUnitMFA::CUnitMFA(WORD bId, WORD uId, eUnitType unitType) : CUnitRsrc(bId, uId)
{
	//m_boxId    = 1;   phase2, 1 by default
	m_connId      = 0;
	m_UnitType    = unitType;
	m_isAllocated = FALSE;
	for (WORD curr_accelerator = 0; curr_accelerator < ACCELERATORS_PER_UNIT_NETRA; curr_accelerator++)
	{
		m_FreeCapacity[curr_accelerator] = 1000;
	}
	m_FreeEncoderWeight = MAX_ENCODER_WEIGHT_PER_UNIT;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (unitType == eUnitType_Video && pSystemResources && eProductTypeNinja == pSystemResources->GetProductType())
	{
		m_maxUnitNumPorts = MAX_PORTS;
	}
	else if (pSystemResources && CHelperFuncs::IsSoftMCU(pSystemResources->GetProductType()))
	{
		m_maxUnitNumPorts = (eProductTypeSoftMCUMfw == pSystemResources->GetProductType()) ? MAX_REQUIRED_PORTS_SOFTMFW_UNIT : MAX_REQUIRED_PORTS_SOFTMCU_UNIT;
	}
	else
		m_maxUnitNumPorts = MAX_PORTS;

	m_pPortsList             = new CPortAllocTypeVector(PORT_FREE, m_maxUnitNumPorts);
	m_FPGA_Index             = 0;
	m_OccupiedARTChannels    = 0;
	m_FipsStat               = TRUE;
	m_isRecovery             = FALSE;
	m_isRecoveryReserved     = FALSE;
	m_physicalUnitId         = uId;
	m_numAllocatedTipScreens = 0;

	VALIDATEMESSAGEMAP;
}

////////////////////////////////////////////////////////////////////////////
CUnitMFA::CUnitMFA( const CUnitMFA& other) : CUnitRsrc(other),
	m_pPortsList(new CPortAllocTypeVector(*(other.m_pPortsList)))
{
	m_connId      = other.m_connId;
	m_UnitType    = other.m_UnitType;
	m_isAllocated = other.m_isAllocated;
	for (WORD curr_accelerator = 0; curr_accelerator < ACCELERATORS_PER_UNIT_NETRA; curr_accelerator++)
	{
		m_FreeCapacity[curr_accelerator] = other.m_FreeCapacity[curr_accelerator];
	}
	m_FipsStat               = other.m_FipsStat;
	m_FPGA_Index             = other.m_FPGA_Index;
	m_OccupiedARTChannels    = other.m_OccupiedARTChannels;
	m_isRecovery             = other.m_isRecovery;
	m_isRecoveryReserved     = other.m_isRecoveryReserved;
	m_physicalUnitId         = other.m_physicalUnitId;
	m_FreeEncoderWeight      = other.m_FreeEncoderWeight;
	m_pActivePorts           = other.m_pActivePorts;
	m_maxUnitNumPorts        = other.m_maxUnitNumPorts;
	m_numAllocatedTipScreens = other.m_numAllocatedTipScreens;

	VALIDATEMESSAGEMAP;
}

////////////////////////////////////////////////////////////////////////////
CUnitMFA::~CUnitMFA()
{
	if (m_pPortsList) //bitVec
		POBJDELETE( m_pPortsList);
}

////////////////////////////////////////////////////////////////////////////
void* CUnitMFA::GetMessageMap()
{
	return (void*)m_msgEntries;
}

////////////////////////////////////////////////////////////////////////////
size_t CUnitMFA::GetPortNumber() const
{
	size_t num_ports = m_pActivePorts.size();
	return num_ports;
}

////////////////////////////////////////////////////////////////////////////
DWORD CUnitMFA::GetUtilizedBandwidth(BOOL bIn) const
{
	DWORD bandWidth = 0;
	CActivePortsList::iterator portItr;
	for (portItr = m_pActivePorts.begin(); portItr != m_pActivePorts.end(); portItr++)
	{
		bandWidth += portItr->GetUtilizedBandWidth(bIn);
	}

	return bandWidth;
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::SetUnitType(eUnitType newType)
{
	if (m_UnitType != newType)
	{
		m_UnitType = newType;
		if (m_OccupiedARTChannels != 0)
		{
			PASSERTMSG(1, "CUnitMFA::SetUnitType - m_OccupiedARTChannels != 0");
			m_OccupiedARTChannels = 0;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::UpdateUnitIDToFPGAIndex(WORD uId, eCardType cardType )
{
	if (eMpmx_80 == cardType || eMpmx_40 == cardType || eMpmx_20 == cardType)
	{
		if (uId <= 10)         // Unit Id is 1-based
			SetFPGAIndex(0);
		else if (uId <= 20)
			SetFPGAIndex(1);
		else if (uId <= 30)
			SetFPGAIndex(2);
		else
			SetFPGAIndex(3);
	}
	else if (eMpmRx_Half == cardType || eMpmRx_Full == cardType ||
	         eMpmx_Soft_Half == cardType || eMpmx_Soft_Full == cardType ||
	         eMpmRx_Ninja == cardType)
	{
		SetFPGAIndex(0);
	}
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::SetRecovery(BYTE isRecovery, WORD newPhysicalUnitId)
{
	m_isRecovery = isRecovery;
	if (isRecovery)
	{
		SetPhysicalUnitId( newPhysicalUnitId );

		TRACEINTO << " UnitMFA::SetRecovery = TRUE : StartTimer for unit = " << GetUnitId() << ", physicalUnitId = " << m_physicalUnitId;
		StartTimer(RECOVERY_UNITS_TIMER, SECONDS_FOR_RECOVERY_UNITS_TIMER * SECOND);
	}
	else
	{
		TRACEINTO << " UnitMFA::SetRecovery = FALSE : DeleteTimer for unit = " << GetUnitId();
		if (IsValidTimer(RECOVERY_UNITS_TIMER))
			DeleteTimer(RECOVERY_UNITS_TIMER);
	}
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::OnRecoveryUnitTimer(CSegment* pMsg)
{
	std::ostringstream ostr;
	ostr << " UnitMFA::OnRecoveryUnitTimer : unit = " << GetUnitId();
	if (m_isRecovery)
	{
		m_isRecovery = FALSE;
		SetEnabled(TRUE);
		ostr << " was under recovery - moved to be enabled !!!";
	}
	TRACEINTO << ostr.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
WORD CUnitMFA::GetPhysicalUnitId() const
{
	return m_physicalUnitId;
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::SetPhysicalUnitId(WORD unitId)
{
	m_physicalUnitId = unitId;
	CBoard* pBoard = GetBoard();
	if (pBoard)
		UpdateUnitIDToFPGAIndex( unitId, pBoard->GetCardType());

	TRACEINTO << "board_id = " << GetBoardId() << ", unit_id = " << GetUnitId() << ", physical unit_id = " << m_physicalUnitId;
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::IncreaseNumAllocatedTipScreens(WORD numOfTipScreens)
{
	m_numAllocatedTipScreens += numOfTipScreens;

	TRACEINTO << "board_id: " << GetBoardId()
	          << ", unit_id: " << GetUnitId()
	          << ", numOfTipScreens: " << numOfTipScreens
	          << ", m_numAllocatedTipScreens(after increase): " << m_numAllocatedTipScreens;

	WORD maxNumOfTipScreensPerArt = 0;

	if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART_SOFTMCU;
	}
	else
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART;
	}

	if (m_numAllocatedTipScreens > maxNumOfTipScreensPerArt)
	{
		TRACEINTO << "CUnitMFA::IncreaseNumAllocatedTipScreens OUT OF RANGE! - m_numAllocatedTipScreens=" << m_numAllocatedTipScreens << "\n";
		PASSERTMSG(m_numAllocatedTipScreens, "m_numAllocatedTipScreens is out of range!");
	}
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::DecreaseNumAllocatedTipScreens(WORD numOfTipScreens)
{
	if (numOfTipScreens > m_numAllocatedTipScreens)
	{
		m_numAllocatedTipScreens = 0;
	}
	else
	{
		m_numAllocatedTipScreens -= numOfTipScreens;
	}

	TRACEINTO << "board_id: " << GetBoardId()
	          << ", unit_id: " << GetUnitId()
	          << ", numOfTipScreens: " << numOfTipScreens
	          << " , m_numAllocatedTipScreens(after decrease): " << m_numAllocatedTipScreens;

	WORD maxNumOfTipScreensPerArt = 0;

	if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART_SOFTMCU;
	}
	else
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART;
	}

	if (m_numAllocatedTipScreens > maxNumOfTipScreensPerArt)
	{
		TRACEINTO << "CUnitMFA::DecreaseNumAllocatedTipScreens OUT OF RANGE! - m_numAllocatedTipScreens=" << m_numAllocatedTipScreens << "\n";
		PASSERTMSG(m_numAllocatedTipScreens, "m_numAllocatedTipScreens is out of range!");
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CUnitMFA::GetRsrcDetail(CRsrcDetailElement* pDetail)
{
	STATUS status = STATUS_OK;

	pDetail->m_numPorts       = m_pActivePorts.size();
	pDetail->m_numActivePorts = pDetail->m_numPorts;

	WORD num_ports = pDetail->m_numPorts;
	pDetail->m_activePorts = new CDetailActivePort*[num_ports];
	pDetail->m_slotNumber  = this->GetBoardId();
	pDetail->m_cardType    = 0; //BoardType2CardType(m_type);

	int activeIndex   = 0;
	DWORD monitorConfId = 0xFFFFFFFF,  monitorPartyId = 0xFFFFFFFF;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB)
	{
		for (WORD i = 0; i < m_maxUnitNumPorts; i++)
		{
			if (PORT_OCCUPIED == (*m_pPortsList)[i])
			{
				CActivePort port(i);
				CActivePortsList::iterator p_port;
				p_port = m_pActivePorts.find(port);
				if (p_port == m_pActivePorts.end()) //no such active port, some mistake.
				{
					PASSERT(1);
					return STATUS_FAIL;
				}

				pDetail->m_activePorts[activeIndex] = new CDetailActivePort;

				CBoard* pBoard = GetBoard();
				// Tsahi - update utilized promiles for Netra unit, maybe add 'ceil'
				if (pBoard && CHelperFuncs::IsMpmRxOrNinja(pBoard->GetCardType()) && this->GetUnitType() == eUnitType_Video)
				{
					pDetail->m_activePorts[activeIndex]->m_promilUtilized = (WORD)(p_port->GetPromilUtilized() / ACCELERATORS_PER_UNIT_NETRA);
				}
				else
				{
					pDetail->m_activePorts[activeIndex]->m_promilUtilized = (WORD)(p_port->GetPromilUtilized());
				}
				pDetail->m_activePorts[activeIndex]->m_portId = p_port->GetPortId();
				DWORD i1 = p_port->GetConfId();
				DWORD i2 = p_port->GetPartyId();

				pConfRsrcDB->GetMonitorIdsRsrcIds( i1, i2, monitorConfId, monitorPartyId);

				pDetail->m_activePorts[activeIndex]->m_confId  = monitorConfId;
				pDetail->m_activePorts[activeIndex]->m_partyId = monitorPartyId;
				pDetail->m_activePorts[activeIndex]->m_porType = 0;      // while. After it has to be add
				pDetail->m_activePorts[activeIndex]->m_connectionId = 0; //while
				pDetail->m_activePorts[activeIndex]->m_isActive     = TRUE;
				activeIndex++;
			} // if (!FreePorts()[i])
		} // for
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CUnitMFA::GetRsrcDetailNinja(CRsrcDetailElement* pDetail)
{
	STATUS status = STATUS_OK;
	DSPMonitorPortList portlist;
	DWORD curBdid = 0, curUnitId = 0;
	curBdid   = this->GetBoardId();
	curUnitId = this->GetUnitId();
	if (5 < curUnitId)
	{
		PTRACE2INT(eLevelError, " CUnitMFA::GetRsrcDetailNinja -DSP  unit id out of range:", curUnitId);
		return STATUS_FAIL;
	}
	status = GetDspMonitorStatus(portlist, curBdid, curUnitId);
	int num_ports = portlist.len;
	DWORD confMonitorID  = 0xFFFFFFFF;
	DWORD partyMonitorID = 0xFFFFFFFF;
	pDetail->m_numPorts       = num_ports;
	pDetail->m_numActivePorts = pDetail->m_numPorts;
	pDetail->m_activePorts    = new CDetailActivePort*[num_ports];
	pDetail->m_cardType       = 0;
	pDetail->m_slotNumber     = this->GetBoardId() + DSP_CARD_SLOT_ID_0;
	CConfRsrcDB*       pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	for (int i = 0; i < num_ports; ++i)
	{
		pDetail->m_activePorts[i]                   = new CDetailActivePort;
		pDetail->m_activePorts[i]->m_promilUtilized = (WORD)(portlist.status[i].percentOccupied * 10);
		pDetail->m_activePorts[i]->m_portId         = portlist.status[i].portId;
		if (pConfRsrcDB)
		{
			pConfRsrcDB->GetMonitorIdsRsrcIds(portlist.status[i].confRsrcID, portlist.status[i].partyRsrcID, confMonitorID, partyMonitorID);
		}
		pDetail->m_activePorts[i]->m_porType      = 0;
		pDetail->m_activePorts[i]->m_connectionId = 0;
		pDetail->m_activePorts[i]->m_isActive     = portlist.status[i].isActive ? TRUE : FALSE;

		if (TRUE == pDetail->m_activePorts[i]->m_isActive)
		{
			pDetail->m_activePorts[i]->m_confId  = confMonitorID;
			pDetail->m_activePorts[i]->m_partyId = partyMonitorID;
		}
		else
		{
			pDetail->m_activePorts[i]->m_confId  = 0xFFFFFFFF;
			pDetail->m_activePorts[i]->m_partyId = 0xFFFFFFFF;
		}
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::FreeAllActivePorts()
{
	std::ostringstream ostr;
	ostr << "unit = " << GetUnitId() << " on board = " << GetBoardId() << " has active ports : \n";

	CActivePortsList::iterator portItr  = m_pActivePorts.begin();
	WORD port_num = 0;
	while (portItr != m_pActivePorts.end())
	{
		WORD portId = portItr->GetPortId();
		ostr << "  portId = " << portId;
		if (port_num > m_maxUnitNumPorts)
		{
			PASSERT(port_num);
			break;
		}
		port_num++;

		FreeActivePort(portId);
		(*m_pPortsList)[portId] = PORT_FREE;

		portItr = m_pActivePorts.begin();
	}

	for (WORD curr_accelerator = 0; curr_accelerator < ACCELERATORS_PER_UNIT_NETRA; curr_accelerator++)
	{
		m_FreeCapacity[curr_accelerator] = 1000;
	}
	m_isAllocated       = FALSE;
	m_FreeEncoderWeight = MAX_ENCODER_WEIGHT_PER_UNIT;
	TRACEINTO << ostr.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::DumpAllActivePorts(WORD min_num_of_potrs)
{
	WORD num_of_active_ports = m_pActivePorts.size();

	std::ostringstream msg;

	msg << "UnitId:" << GetUnitId() << ", BoardId:" << GetBoardId() << ", NumOfActivePorts:" << num_of_active_ports;
	if (num_of_active_ports > min_num_of_potrs)
	{
		CActivePortsList::iterator _itr, _end = m_pActivePorts.end();
		for (_itr = m_pActivePorts.begin(); _itr != _end; ++_itr)
			msg << "\n  PortId:" << _itr->GetPortId() << ", ConfId:" << _itr->GetConfId() << ", PartyId:" << _itr->GetPartyId();
	}
	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
bool CUnitMFA::Is1080p60SplitEncoderAllocated() const
{
	bool found1080p60SplitEncoder = false;

	TRACEINTO << " CUnitMFA::Is1080p60SplitEncoderAllocated start ";

	WORD port_index = 0;

	CActivePortsList::iterator portItr;
	for (portItr = m_pActivePorts.begin(); portItr != m_pActivePorts.end(); portItr++)
	{
		eResourceTypes resourceTypes = portItr->GetPortType();
		if (ePhysical_video_encoder != resourceTypes)
		{
			continue;
		}

		DWORD confRsrcId  = portItr->GetConfId();
		DWORD partyRsrcId = portItr->GetPartyId();
		CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
		if (!pConfRsrcDB)
		{
			PTRACE(eLevelError, "CUnitMFA::Is1080p60SplitEncoderAllocated - pConfRsrcDB is NULL");
			return false;
		}
		const CConfRsrc*  pConfRsrc = pConfRsrcDB->GetConfRsrcByRsrcConfId(confRsrcId);
		if (!pConfRsrc)
		{
			PTRACE2INT(eLevelError, "CUnitMFA::Is1080p60SplitEncoderAllocated - cant find conf, confRsrcId = ", (DWORD)confRsrcId);
			return false;
		}
		const CPartyRsrc* pPartyRsrc = pConfRsrc->GetPartyRsrcByRsrcPartyId(partyRsrcId);
		if (!pPartyRsrc)
		{
			PTRACE2INT(eLevelError, "CUnitMFA::Is1080p60SplitEncoderAllocated - cant find party, partyRsrcId = ", (DWORD)partyRsrcId);
			return false;
		}
		eVideoPartyType   videoPartyType = pPartyRsrc->GetVideoPartyType();

		TRACEINTO << " CUnitMFA::Is1080p60SplitEncoderAllocated port = " << port_index << " , partyRsrcId = " << partyRsrcId << " ,confRsrcId = " << confRsrcId << " , videoPartyType = " << (DWORD)videoPartyType;
		port_index++;
		if (eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type == videoPartyType)
		{
			found1080p60SplitEncoder = true;
			break;
		}
	}
	return found1080p60SplitEncoder;
}

////////////////////////////////////////////////////////////////////////////
DWORD CUnitMFA::GetAverageAllocatedCapacity()
{
	return (m_pActivePorts.size() > 0) ? GetTotalAllocatedCapacity() / m_pActivePorts.size() : 0;
}

////////////////////////////////////////////////////////////////////////////
DWORD CUnitMFA::GetTotalAllocatedCapacity()
{
	DWORD allocatedCapacity = 0;
	for (CActivePortsList::iterator i = m_pActivePorts.begin(); i != m_pActivePorts.end(); ++i)
		allocatedCapacity += i->GetCapacity();

	return allocatedCapacity;
}

////////////////////////////////////////////////////////////////////////////
STATUS CUnitMFA::AddActivePort(WORD portId, WORD acceleratorId, DWORD confId, DWORD partyId, DWORD patyCapacity, float promilUtilized, eResourceTypes type, DWORD utilizedBandWidth_in, DWORD utilizedBandWidth_out, float utilizedEncoderWeight)
{
	CActivePort port(portId, acceleratorId, confId, partyId, patyCapacity, promilUtilized, type, utilizedBandWidth_in, utilizedBandWidth_out, utilizedEncoderWeight);

	if (m_pActivePorts.find(port) != m_pActivePorts.end())
	{
		PASSERT(portId);  // port already exists in list? - some error
		return STATUS_FAIL;
	}

	m_pActivePorts.insert(port);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CUnitMFA::FreeActivePort(WORD portId)
{
	CActivePort port(portId);

	if (m_pActivePorts.erase(port) == 0)  //returns number of members erased
	{
		PASSERT(portId);
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CUnitMFA::FindPortToOccupy(eLogicalResourceTypes type, WORD num_ports_per_unit, ePortAllocTypes oldPortStatus, WORD acceleratorId)
{
	PASSERT_AND_RETURN_VALUE(acceleratorId >= ACCELERATORS_PER_UNIT_NETRA, -1);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	eProductType product_type = (NULL != pSystemResources) ? pSystemResources->GetProductType() : eProductTypeRMX2000;

	DWORD firstPortId = acceleratorId * NUM_PORTS_PER_ACCELERATOR_NETRA;

	for (DWORD i = firstPortId; i < firstPortId + num_ports_per_unit; i++)
	{
		if (CHelperFuncs::IsLogicalVideoEncoderType(type))
		{  //video encoder should always be on a even unit id
			if (i % 2 != 0)
				continue;
		}
		else if (CHelperFuncs::IsLogicalVideoDecoderType(type))
		{  //video decoder should always be on a odd unit id
			if (i % 2 == 0)
				continue;
		}

		if (oldPortStatus == (*m_pPortsList)[i])
		{
			(*m_pPortsList)[i] = PORT_OCCUPIED; //found 1, take it
			return i;
		}
	}

	return -1; //not found
}

////////////////////////////////////////////////////////////////////////////
int CUnitMFA::FindPortToOccupyCOP(eLogicalResourceTypes type, WORD num_ports_per_unit, ePortAllocTypes oldPortStatus)
{
	for (int i = 0; i < num_ports_per_unit; i++) // req. from emb: start portId from 0;
	{
		if (CHelperFuncs::IsLogicalVideoEncoderType(type))
		{                          // 1. video encoder should always be on a even unit id
			if (i % 2 != 0 || i > 6) // 2. COP conf can have maximum 4 encoders per DSP, i.e. port id's 0,2,4,6 (Olga check?)
				continue;
		}
		else if (CHelperFuncs::IsLogicalVideoDecoderType(type))
		{ //video decoder should always be on a odd unit id
			if (i % 2 == 0)
				continue;
		}

		if (oldPortStatus == (*m_pPortsList)[i])
		{
			(*m_pPortsList)[i] = PORT_OCCUPIED; //found 1, take it
			return i;
		}
	}
	TRACEINTO << "ResourceType:" << type << ", NumPortsPerUnit:" << num_ports_per_unit << ", OldPortStatus:" << oldPortStatus;
	return -1; //not found
}

////////////////////////////////////////////////////////////////////////////
int CUnitMFA::FindPortToOccupySoftMCU(eLogicalResourceTypes type, WORD num_ports_per_unit, ePortAllocTypes oldPortStatus, int last_allocated_port)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	eProductType product_type = (NULL != pSystemResources) ? pSystemResources->GetProductType() : eProductTypeRMX2000;

	int num_avc_encoders = 0;
	if (eProductTypeCallGeneratorSoftMCU == product_type)
		num_avc_encoders = MAX_NUMBER_AVC_PARTICIPANTS_SOFT_CG * 2;
	else
		num_avc_encoders = MAX_NUMBER_AVC_PARTICIPANTS_SOFT_MCU * 2;
	//Gesher: first 40 for regular encoders, additional encoders (in pairs) to support mix participants

	int i_start_index = 0, increment = 1;
	if (eLogical_relay_avc_to_svc_video_encoder_1 == type)
	{
		i_start_index = num_avc_encoders;
		increment = 4; //additional encoders (in pairs) to support mix participants
	}
	else if (eLogical_relay_avc_to_svc_video_encoder_2 == type)
		i_start_index = last_allocated_port;

	for (int i = i_start_index; i < num_ports_per_unit; i += increment)
	{
		if (CHelperFuncs::IsLogicalVideoEncoderType(type))
		{
			if (i >= MAX_REQUIRED_PORTS_SOFTMCU_UNIT)
				return -1;
			if (i % 2 != 0) //video encoder should always be an even id,
				continue;
			if (i < num_avc_encoders && CHelperFuncs::IsLogicalSoftMixVideoEncoderType(type))
				continue;
			if (i >= num_avc_encoders && !CHelperFuncs::IsLogicalSoftMixVideoEncoderType(type))
				continue;

			// IMPORTANT: need to add protection for the additional MIX encoders:
			// the ports should be adjacent numbers (20 and 22, 24 and 26,...) because each pair writes to the same address!

		}
		else if (CHelperFuncs::IsLogicalVideoDecoderType(type))
		{
			if (i % 2 == 0) //video decoder should always be on a odd unit id
				continue;
		}

		if (oldPortStatus == (*m_pPortsList)[i])
		{
			if (eLogical_relay_avc_to_svc_video_encoder_2 == type && (i != (last_allocated_port + 2)))
			{
				TRACESTRFUNC(eLevelError) << "wrong encoder port : lrt=" << type << ", found port=" << i << ", last_allocated_port=" << last_allocated_port;
			}
			(*m_pPortsList)[i] = PORT_OCCUPIED; //found 1, take it
			return i;
		}
	}
	return -1; //not found
}

////////////////////////////////////////////////////////////////////////////
float CUnitMFA::GetFreeVideoPortsCapacity(WORD acceleratorId) const
{
	CBoard* pBoard = GetBoard();
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, 0);

	WORD num_ports_per_accelerator = 8; // in DaVinci each unit has 8 ports, in Netra each unit has 3 accelerators with 16 (8) ports on each one.
	float cif_port_promilles = VID_TOTAL_CIF_PROMILLES_BREEZE / 2;
	float free_ports_capacity = 0;
	eCardType cardType = pBoard->GetCardType();

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources && eProductTypeCallGeneratorSoftMCU == pSystemResources->GetProductType())
	{
		num_ports_per_accelerator = MAX_REQUIRED_PORTS_SOFTMCU_UNIT;
		cif_port_promilles = (float)VID_TOTAL_CIF_SD_PROMILLES_CG / 2;
	}
	else if (CHelperFuncs::IsMpmRxOrNinja(cardType))
	{
		num_ports_per_accelerator = NUM_PORTS_PER_ACCELERATOR_NETRA;
		cif_port_promilles = VID_TOTAL_CIF_PROMILLES_MPMRX / 2;
	}

	for (int i = (num_ports_per_accelerator * acceleratorId); i < (num_ports_per_accelerator * (acceleratorId + 1)); i++)
	{
		if (PORT_FREE == (*m_pPortsList)[i])
			free_ports_capacity += cif_port_promilles;
	}

	// In case of MPM-Rx/Ninja, reduce 1000 promilles since each accelerator on the Netra has 16 ports,
	// but only 8 are usable each time. Odd ports are for decoders, even ports are for encoders.
	if (CHelperFuncs::IsMpmRxOrNinja(cardType))
	{
		free_ports_capacity -= 1000;
	}

	return free_ports_capacity;
}

////////////////////////////////////////////////////////////////////////////
WORD CUnitMFA::GetFreeEncoderPorts(WORD acceleratorId, BOOL includeFreeDisabled) const
{
	CBoard* pBoard = GetBoard();
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, 0);

	// in DaVinci each unit has 8 ports, in Netra each unit has 3 accelerators with 16 (8) ports (NUM_PORTS_PER_ACCELERATOR_NETRA) on each one.
	WORD num_ports_per_accelerator = 8;
	WORD free_encoder_ports = 0;
	eCardType cardType = pBoard->GetCardType();

	if (CHelperFuncs::IsSoftCard(cardType))
	{
		num_ports_per_accelerator = MAX_REQUIRED_PORTS_SOFTMCU_UNIT;
	}
	else if (CHelperFuncs::IsMpmRxOrNinja(cardType))
	{
		num_ports_per_accelerator = NUM_PORTS_PER_ACCELERATOR_NETRA;
	}
	else
		num_ports_per_accelerator = 8;

	for (int i = (num_ports_per_accelerator * acceleratorId); i < (num_ports_per_accelerator * (acceleratorId + 1)); i++)
	{
		if (i % 2 == 0 && (PORT_FREE == (*m_pPortsList)[i] || (includeFreeDisabled && PORT_FREE_DISABLED == (*m_pPortsList)[i])))
			++free_encoder_ports;
	}

	return free_encoder_ports;
}

////////////////////////////////////////////////////////////////////////////
WORD CUnitMFA::GetFreeDecoderPorts(WORD acceleratorId, BOOL includeFreeDisabled) const
{
	CBoard* pBoard = GetBoard();
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, 0);

	// in DaVinci each unit has 8 ports, in Netra each unit has 3 accelerators with 16 (8) ports (NUM_PORTS_PER_ACCELERATOR_NETRA) on each one.
	WORD num_ports_per_accelerator = 8;
	WORD free_decoder_ports = 0;
	eCardType cardType = pBoard->GetCardType();

	if (CHelperFuncs::IsSoftCard(cardType))
	{
		num_ports_per_accelerator = MAX_REQUIRED_PORTS_SOFTMCU_UNIT;
	}
	else if (CHelperFuncs::IsMpmRxOrNinja(cardType))
	{
		num_ports_per_accelerator = NUM_PORTS_PER_ACCELERATOR_NETRA;
	}
	else
		num_ports_per_accelerator = 8;

	for (int i = (num_ports_per_accelerator * acceleratorId); i < (num_ports_per_accelerator * (acceleratorId + 1)); i++)
	{
		if (i % 2 != 0 && (PORT_FREE == (*m_pPortsList)[i] || (includeFreeDisabled && PORT_FREE_DISABLED == (*m_pPortsList)[i])))
			++free_decoder_ports;
	}

	return free_decoder_ports;
}

////////////////////////////////////////////////////////////////////////////
WORD CUnitMFA::GetFreeArtPorts(BOOL includeFreeDisabled) const
{
	CBoard* pBoard = GetBoard();
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, 0);

	eUnitType unitType = GetUnitType();
	PASSERTMSG_AND_RETURN_VALUE(unitType != eUnitType_Art && unitType != eUnitType_Art_Control, "GetArtPorts should be used only for ART units!", 0);

	WORD free_ports = 0;
	WORD num_ports_per_unit = 0;

	float full_capacity = 1000;
	float needed_promilles = pBoard->CalculateNeededPromilles(PORT_ART, 0 /*partyCapacity not relevant here*/, TRUE /* for 300 SVC */, TRUE, eCP_H264_upto_CIF_video_party_type);
	if (needed_promilles > 0)
		num_ports_per_unit = (WORD)(full_capacity / needed_promilles);

	for (int i = 0; i < num_ports_per_unit; i++)
	{
		if ((PORT_FREE == (*m_pPortsList)[i] || (includeFreeDisabled && PORT_FREE_DISABLED == (*m_pPortsList)[i])))
			++free_ports;
	}

	return free_ports;
}

////////////////////////////////////////////////////////////////////////////
int CUnitMFA::AllocatePorts(ConfRsrcID confId, PartyRsrcID partyId, DWORD partyCapacity, MediaUnit& mediaUnit, eVideoPartyType videoPartyType, WORD numOfTipScreens)
{
	CBoard* pBoard = GetBoard();
	PASSERT_AND_RETURN_VALUE(!pBoard, -1);

	float needed_promilles = mediaUnit.GetTotalNeededCapacity();
	float needed_encoder_weight = mediaUnit.GetTotalNeededEncoderWeight();
	eResourceTypes rsrcPhysicalType = mediaUnit.m_MediaPortsList[0].GetPhysicalResourceType();
	//assume that all ports in m_MediaPortsList are allocated on the same accelerator
	WORD acceleratorId = mediaUnit.m_MediaPortsList[0].m_acceleratorId;
	BOOL isMpmRx = CHelperFuncs::IsMpmRx(pBoard->GetCardType());
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	BYTE bRestrictArtAllocation = YES;
	BOOL isTrafficShapingEnabled = NO;

	// If traffic shaping is enabled in the system, art must be restricted to 100%
	sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING, isTrafficShapingEnabled);
	if (!isTrafficShapingEnabled)
		sysConfig->GetBOOLDataByKey("RESTRICT_ART_ALLOCATION_ACCORDING_TO_WEIGHT", bRestrictArtAllocation);

	std::ostringstream msg;
	msg
		<< "\n  ConfId              :" << confId
		<< "\n  PartyId             :" << partyId
		<< "\n  PartyCapacity       :" << partyCapacity
		<< "\n  BoardId             :" << GetBoardId()
		<< "\n  UnitId              :" << GetUnitId()
		<< "\n  VideoPartyType      :" << eVideoPartyTypeNames[videoPartyType]
		<< "\n  FreePromilles       :" << m_FreeCapacity[acceleratorId]
		<< "\n  NeededPromilles     :" << needed_promilles
		<< "\n  FreeEncoderWeight   :" << m_FreeEncoderWeight
		<< "\n  NeededEncoderWeight :" << needed_encoder_weight
		<< "\n  ResourceType        :" << ResourceTypeToString(rsrcPhysicalType);

	if (GetIsEnabled())
	{
		if (m_FreeCapacity[acceleratorId] < needed_promilles)/*all taken*/
		{
			if (ePhysical_art == rsrcPhysicalType && isMpmRx && (NO == bRestrictArtAllocation))
			{
				PTRACE(eLevelInfoNormal, "CUnitMFA::AllocatePorts, For MPM-Rx ART,continue to allocate ports even no free promilles when system flag is NO.");
			}
			else
			{
				PTRACE(eLevelInfoNormal, msg.str().c_str());
				return -1;
			}
		}
	}
	else
	{
		TRACEINTO << "Unit is disabled" << msg.str().c_str();
		return -1;
	}

	bool isSW_MCU = CHelperFuncs::IsSoftCard(pBoard->GetCardType());	//OLGA - Soft MCU
	eProductType prodType = eProductTypeRMX2000;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, -1);

	prodType = pSystemResources->GetProductType();

	WORD num_ports_per_unit = 0;
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	switch (rsrcPhysicalType)
	{
		case ePhysical_mrmp:
		{
			max_media_ports = 1; // For Physical_art only the first element has meaning
			// For mrmp, update needed_promilles since it is 0
			needed_promilles = pBoard->CalculateNeededPromilles(PORT_VIDEO, partyCapacity);
			float full_capacity = 1000;
			if (needed_promilles)
				num_ports_per_unit = (WORD)(full_capacity / needed_promilles);
			break;
		}
		case ePhysical_art: //all ports have same promilles
		{
			max_media_ports = 1; // For Physical_art only the first element has meaning
			float full_capacity = 1000;

			// Tsahi - TODO: check if we need to take the previous value, or maybe update the original param.
			float needed_promilles = pBoard->CalculateNeededPromilles(PORT_ART, 0 /*partyCapacity not relevant here*/, TRUE /* for 300 SVC */, TRUE, videoPartyType);

			if (needed_promilles)
				num_ports_per_unit = (WORD)(full_capacity / needed_promilles);
			break;
		}
		case ePhysical_video_encoder:
		case ePhysical_video_decoder:
		{
			if (isSW_MCU) //OLGA - Soft MCU
				num_ports_per_unit = MAX_REQUIRED_PORTS_SOFTMCU_UNIT;
			else if (mediaUnit.GetTotalNeededCapacity() == 1000) //full unit
				num_ports_per_unit = 2;
			else if (CHelperFuncs::IsMpmRxOrNinja(pBoard->GetCardType())) // video ports have different promilles, but the maximum is 8 for MPMx, 16 for MPM-Rx
				num_ports_per_unit = NUM_PORTS_PER_ACCELERATOR_NETRA;
			else
				// video ports have different promilles, but the maximum is 8
				num_ports_per_unit = 8;
			break;
		}
		default:
		{
			PTRACE(eLevelInfoNormal, msg.str().c_str());
			PASSERT(rsrcPhysicalType); // while only art and video types
		}
	}

	msg
		<< "\n  NumPortsInUnit      :" << num_ports_per_unit
		<< "\n  MaxMediaPorts       :" << max_media_ports;

	int last_allocated_port = 0;
	for (WORD portIndex = 0; portIndex < max_media_ports && portIndex < ARRAYSIZE(mediaUnit.m_MediaPortsList); ++portIndex)
	{
		// For Physical_art only the first element has the meaning:
		//  The value of mediaUnit.m_MediaPortsList[0].m_needed_capacity_promilles = needed_capacity;
		//  The value of mediaUnit.m_MediaPortsList[0].m_type = eLogical_audio_encoder;
		if (mediaUnit.m_MediaPortsList[portIndex].m_type != eLogical_res_none)
		{
			eLogicalResourceTypes rsrcType = mediaUnit.m_MediaPortsList[portIndex].m_type;
			int portId = -1;
			if (isSW_MCU && (eProductTypeSoftMCU == prodType || eProductTypeGesher == prodType || eProductTypeEdgeAxis == prodType))
				portId = FindPortToOccupySoftMCU(rsrcType, num_ports_per_unit, PORT_FREE, last_allocated_port);
			else
				portId = FindPortToOccupy(rsrcType, num_ports_per_unit, PORT_FREE, acceleratorId); //first try to find free port
			if (portId == -1) //all enabled ports busy
			{
				if (isSW_MCU && (eProductTypeSoftMCU == prodType || eProductTypeGesher == prodType || eProductTypeEdgeAxis == prodType || eProductTypeCallGeneratorSoftMCU == prodType))
					portId = FindPortToOccupySoftMCU(rsrcType, num_ports_per_unit, PORT_FREE_DISABLED, last_allocated_port);
				else
					portId = FindPortToOccupy(rsrcType, num_ports_per_unit, PORT_FREE_DISABLED, acceleratorId); // try to find disabled ports(due to unsuccessful open/close port)
				if (portId == -1) //still not found
				{
					msg << "\n  PortIndex           :" << portIndex << "there is no free/disable port for allocation";
					PTRACE(eLevelInfoNormal, msg.str().c_str());
					PASSERT(1 + rsrcType);
					return -1;
				}
				else
				{
					msg << "\n  PortIndex           :" << portIndex << "will be allocated on disabled port, PortId:" << portId;
				}
			}
			mediaUnit.m_MediaPortsList[portIndex].m_portId = portId;
			last_allocated_port = portId;
		}
	}
	TRACEINTO << msg.str().c_str();

	AddActivePorts(confId, partyId, partyCapacity, mediaUnit);

	m_FreeCapacity[acceleratorId] -= needed_promilles;
	m_FreeEncoderWeight -= needed_encoder_weight;

	if (ePhysical_art == rsrcPhysicalType && 0 != numOfTipScreens)
	{
		IncreaseNumAllocatedTipScreens(numOfTipScreens);
	}

	if (videoPartyType != eVideo_party_type_none && videoPartyType != eCOP_party_type) //Video participant
	{
		switch (rsrcPhysicalType)
		{
			case ePhysical_art:
				pBoard->AddOneVideoParticipantsWithArtOnThisBoard();
				break;

			case ePhysical_video_decoder: //assumption: there is one and only one decoder port per video participant
				pBoard->AddOneVideoParticipantsWithVideoOnThisBoard();
				break;

			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
	}
	return 0; //STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CUnitMFA::AllocateVideoPortsCOP(DWORD rsrcConfId, MediaUnit& mediaUnit)
{
	CBoard* pBoard = GetBoard();
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, -1);

	float capacity = mediaUnit.GetTotalNeededCapacity();
	if (m_FreeCapacity[0] < capacity) //all taken
		return -1;

	if (GetIsEnabled() == FALSE) //span in failure
		return -1;

	WORD num_ports_per_unit = 0;
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
	int portId;
	eResourceTypes type = mediaUnit.m_MediaPortsList[0].GetPhysicalResourceType();
	if (ePhysical_art == type)
	{
		PASSERT(1);
	}
	else if (ePhysical_video_encoder == type || ePhysical_video_decoder == type)
	{
		int port_count = 0;
		for (WORD portIndex = 0; portIndex < max_media_ports && portIndex < ARRAYSIZE(mediaUnit.m_MediaPortsList); ++portIndex)
		{
			if (mediaUnit.m_MediaPortsList[portIndex].m_type != eLogical_res_none)  //Olga - check with Sergey
				port_count++;
			else
				break;
		}
		if (mediaUnit.GetTotalNeededCapacity() == 1000 && port_count < 3) //full unit. For future, please note
		{
			num_ports_per_unit = 2;
		}
		else
		{
			num_ports_per_unit = 8;
		}
	}
	else
		PASSERT(1); // while only art and video types

	for (WORD portIndex = 0; portIndex < max_media_ports && portIndex < ARRAYSIZE(mediaUnit.m_MediaPortsList); ++portIndex)
	{
		if (mediaUnit.m_MediaPortsList[portIndex].m_type != eLogical_res_none)
		{
			//if(mediaUnit.m_MediaPortsList[portIndex].m_needed_capacity_promilles )
			portId = FindPortToOccupyCOP(mediaUnit.m_MediaPortsList[portIndex].m_type, num_ports_per_unit, PORT_FREE); //first try to find free port
			if (portId == -1) //all enabled ports busy
			{   // To check
				portId = FindPortToOccupyCOP(mediaUnit.m_MediaPortsList[portIndex].m_type, num_ports_per_unit, PORT_FREE_DISABLED);  // try to find disabled ports(due to unsuccessful open/close port)
				if (portId == -1) //still not found
				{
					PASSERT(1);
					return -1;
				}
				else
				{
					PTRACE2INT(eLevelInfoNormal, "CUnitMFA::AllocateVideoPortsCOP, trying to allocate disabled port Nr. =  ", portId);
					PTRACE2INT(eLevelInfoNormal, "/n For rsrcPartyId  - ", mediaUnit.m_MediaPortsList[portIndex].m_rsrcEntityId);
					PTRACE2INT(eLevelInfoNormal, "/n For rsrcConfId   - ", rsrcConfId);
				}
			}
			mediaUnit.m_MediaPortsList[portIndex].m_portId = portId;
		}
	}

	for (WORD portIndex = 0; portIndex < max_media_ports && portIndex < ARRAYSIZE(mediaUnit.m_MediaPortsList); ++portIndex)
	{
		if (mediaUnit.m_MediaPortsList[portIndex].m_type != eLogical_res_none)
		{
			AddActivePort(mediaUnit.m_MediaPortsList[portIndex].m_portId, mediaUnit.m_MediaPortsList[portIndex].m_acceleratorId, rsrcConfId, mediaUnit.m_MediaPortsList[portIndex].m_rsrcEntityId, 0, mediaUnit.m_MediaPortsList[portIndex].m_needed_capacity_promilles, mediaUnit.m_MediaPortsList[portIndex].GetPhysicalResourceType(), mediaUnit.m_MediaPortsList[portIndex].m_needed_bandwidth_in, mediaUnit.m_MediaPortsList[portIndex].m_needed_bandwidth_out, mediaUnit.m_MediaPortsList[portIndex].m_needed_encoder_weight);
		}
	}

	m_isAllocated = TRUE;

	m_FreeCapacity[0] -= capacity;  // To check - we could zeroise it

	return 0; //STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::AllocateARTChannels(WORD needed_ART_channels)
{
	PASSERTMSG(needed_ART_channels + m_OccupiedARTChannels > m_MaxArtChannelsPerArt, "needed_ART_channels + m_OccupiedARTChannels > m_MaxArtChannelsPerArt");
	m_OccupiedARTChannels += needed_ART_channels;

	TRACEINTO << "OccupiedARTChannels:" << m_OccupiedARTChannels << ", AllocatedArtChannels:" << needed_ART_channels << ", BoardId:" << GetBoardId() << ", UnitId:" << GetUnitId();
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::DeAllocateARTChannels(WORD needed_ART_channels)
{
	if (m_OccupiedARTChannels >= needed_ART_channels)
	{
		m_OccupiedARTChannels -= needed_ART_channels;
	}
	else
	{
		TRACEINTO << "\nCUnitMFA::DeAllocateARTChannels - m_OccupiedARTChannels - needed_ART_channels < 0" << "\n";
		m_OccupiedARTChannels = 0;
	}

	TRACEINTO << "\nDeAllocateARTChannels: m_OccupiedARTChannels = " << m_OccupiedARTChannels << " deallocated_ART_channels= " << needed_ART_channels << ", board = " << GetBoardId() << ", unit = " << GetUnitId() << "\n";
}

////////////////////////////////////////////////////////////////////////////
STATUS CUnitMFA::ReAllocateARTChannels(WORD new_ART_channels, WORD old_ART_channels)
{
	STATUS status = STATUS_OK;
	CSmallString sstr;
	sstr << "m_OccupiedARTChannels before = " << m_OccupiedARTChannels << "\n";
	sstr << "old_ART_channels             = " << old_ART_channels << "\n";
	sstr << "new_ART_channels             = " << new_ART_channels << "\n";

	if (m_OccupiedARTChannels >= old_ART_channels)
	{
		m_OccupiedARTChannels -= old_ART_channels;
	}
	else
	{
		sstr << "m_OccupiedARTChannels set to 0 (old_ART_channels>m_OccupiedARTChannels) \n";
		m_OccupiedARTChannels = 0;
	}

	if (new_ART_channels + m_OccupiedARTChannels > m_MaxArtChannelsPerArt)
	{
		status = STATUS_FAIL;
		DBGPASSERT(new_ART_channels + m_OccupiedARTChannels);
		sstr << "m_OccupiedARTChannels set to m_MaxArtChannelsPerArt = " << m_MaxArtChannelsPerArt << " \n";
		m_OccupiedARTChannels = m_MaxArtChannelsPerArt;
	}
	else
	{
		m_OccupiedARTChannels += new_ART_channels;
	}

	sstr << "m_OccupiedARTChannels after = " << m_OccupiedARTChannels << "\n";
	PTRACE2(eLevelInfoNormal, "CUnitMFA::ReAllocateARTChannels : \n", sstr.GetString());

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CUnitMFA::AddActivePorts(DWORD rsrcConfId, WORD rsrcPartyId, DWORD partyCapacity, MediaUnit& mediaUnit)
{
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
	for (WORD portIndex = 0; portIndex < max_media_ports && portIndex < ARRAYSIZE(mediaUnit.m_MediaPortsList); ++portIndex)
		if (mediaUnit.m_MediaPortsList[portIndex].m_type != eLogical_res_none)
			AddActivePort(mediaUnit.m_MediaPortsList[portIndex].m_portId, mediaUnit.m_MediaPortsList[portIndex].m_acceleratorId, rsrcConfId, rsrcPartyId, partyCapacity, mediaUnit.m_MediaPortsList[portIndex].m_needed_capacity_promilles, mediaUnit.m_MediaPortsList[portIndex].GetPhysicalResourceType(), mediaUnit.m_MediaPortsList[portIndex].m_needed_bandwidth_in, mediaUnit.m_MediaPortsList[portIndex].m_needed_bandwidth_out, mediaUnit.m_MediaPortsList[portIndex].m_needed_encoder_weight);

	m_isAllocated = TRUE;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CUnitMFA::DeAllocatePort(WORD portId, eVideoPartyType videoPartyType, BYTE dsbl, WORD acceleratorId)
{
	CActivePort port(portId);

	CActivePortsList::iterator i;

	i = m_pActivePorts.find(port);

	if (i == m_pActivePorts.end()) //no such active port, some mistake.
	{
		PASSERT(portId);
		return -1;
	}

	float capacity = i->GetPromilUtilized();
	eResourceTypes type = i->GetPortType();
	float utilized_encoder_weight = i->GetUtilizedEncoderWeight();
	if (acceleratorId >= ACCELERATORS_PER_UNIT_NETRA)
	{
		PASSERT((WORD )acceleratorId);
		return -1;
	}
	//diff. types error handle!!!
	if (capacity + m_FreeCapacity[acceleratorId] > MAX_FREE_UNIT_CAPACITY + SMALL_ERROR) //some mistake - capacity
	{
		PASSERT((WORD )capacity);
		return -1;
	}

	if (portId == m_maxUnitNumPorts /*|| portId == 0 */) // wrong parameter
	{
		PASSERT((WORD )capacity);
		return -1;
	}

	FreeActivePort(portId);

	if (!dsbl)
		(*m_pPortsList)[portId] = PORT_FREE; ////////////has to be inserted in the ver.
	else
		(*m_pPortsList)[portId] = PORT_FREE_DISABLED;

	m_FreeCapacity[acceleratorId] += capacity;

	eProductType prodType = eProductTypeRMX2000;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		prodType = pSystemResources->GetProductType();

	m_FreeEncoderWeight += utilized_encoder_weight;

	BOOL isUnitOccupied = FALSE;
	for (WORD j = 0; j < ACCELERATORS_PER_UNIT_NETRA; j++)
	{
		if (m_FreeCapacity[j] != MAX_FREE_UNIT_CAPACITY)
			isUnitOccupied = TRUE;
	}

	if (!isUnitOccupied) //all unit is free
	{
		m_isAllocated = FALSE;
		if (m_OccupiedARTChannels != 0)
		{
			//this shouldn't happen, because first we remove the chaneels, then we deallocate the port
			TRACEINTO << "m_OccupiedARTChannels is not 0, although unit is not allocated any more;  board = " << GetBoardId() << ", unit = " << GetUnitId();
			m_OccupiedARTChannels = 0;
		}
	}

	CBoard* pBoard = GetBoard();
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, -1);

	if (videoPartyType != eVideo_party_type_none && videoPartyType != eCOP_party_type) //Video participant
	{
		if (ePhysical_art == type)
		{
			pBoard->RemoveOneVideoParticipantsWithArtOnThisBoard();
		}
		else if (ePhysical_video_decoder == type) //assumption: there is one and only one decoder port per video participant
		{
			pBoard->RemoveOneVideoParticipantsWithVideoOnThisBoard();
		}
	}

	return 0; //success
}

////////////////////////////////////////////////////////////////////////////
int CUnitMFA::DeAllocatePort2C(WORD portId, BYTE dsbl)
{
	CActivePort port(portId);
	CActivePortsList::iterator i;

	i = m_pActivePorts.find(port);

	if (i == m_pActivePorts.end()) //no such active port, some mistake.
	{
		//PASSERT(portId);
		TRACEINTO << " CUnitMFA::DeAllocatePort2C : unit = " << GetUnitId() << " , board = " << GetBoardId() << ", port =" << portId;
		return -1;
	}

	float capacity = i->GetPromilUtilized();

	//diff. types error handle!!!
	if (capacity + m_FreeCapacity[0] > MAX_FREE_UNIT_CAPACITY) //some mistake - capacity
	{
		PASSERT((WORD )capacity);
		return -1;
	}

	if (portId == m_maxUnitNumPorts /*|| portId == 0 */) // wrong parameter
	{
		PASSERT((WORD )capacity);
		return -1;
	}

	FreeActivePort(portId);

	if (!dsbl)
		(*m_pPortsList)[portId] = PORT_FREE; ////////////has to be inserted in the ver.
	else
		(*m_pPortsList)[portId] = PORT_FREE_DISABLED;

	m_FreeCapacity[0] += capacity;

	if (m_FreeCapacity[0] == MAX_FREE_UNIT_CAPACITY) //all free
	{
		m_isAllocated = FALSE;
	}

	return 0; //success
}

////////////////////////////////////////////////////////////////////////////
int CUnitMFA::DeAllocatePortCOP(WORD portId, float capacity, BYTE dsbl)
{
	CActivePort port(portId);
	CActivePortsList::iterator i;

	if (portId == m_maxUnitNumPorts /*|| portId == 0 */) // wrong parameter
	{
		PASSERT((WORD )capacity);
		return -1;
	}

	TRACEINTO << " CUnitMFA::DeAllocatePort2C : unit = " << GetUnitId() << " , board = " << GetBoardId() << ", port =" << portId << ", capacity =" << capacity << ", m_FreeCapacity[0] =" << m_FreeCapacity[0];

	if (capacity + m_FreeCapacity[0] > MAX_FREE_UNIT_CAPACITY) //some mistake - capacity
	{
		//PASSERT((WORD)capacity);
		TRACEINTO << " CUnitMFA::DeAllocatePort2C : capacity + m_FreeCapacity[0] > MAX_FREE_UNIT_CAPACITY ";
		return -1;
	}

	if (!dsbl)
		(*m_pPortsList)[portId] = PORT_FREE; ////////////has to be inserted in the ver.
	else
		(*m_pPortsList)[portId] = PORT_FREE_DISABLED;

	m_FreeCapacity[0] += capacity;

	if (m_FreeCapacity[0] == MAX_FREE_UNIT_CAPACITY) //all free
	{
		m_isAllocated = FALSE;
	}

	i = m_pActivePorts.find(port);

	if (i == m_pActivePorts.end()) //no such active port, some mistake.
	{
		//PASSERT(portId);
		TRACEINTO << " CUnitMFA::DeAllocatePort2C : unit = " << GetUnitId() << " , board = " << GetBoardId() << ", port =" << portId;
		return -1;
	}

	FreeActivePort(portId);

	return 0; //success
}

////////////////////////////////////////////////////////////////////////////
STATUS CUnitMFA::UpdateActivePort(WORD portId, DWORD rsrcConfId, WORD rsrcPartyId)
{
	CActivePort port(portId);
	CActivePortsList::iterator p_port;

	p_port = m_pActivePorts.find(port);

	if (p_port == m_pActivePorts.end()) //no such active port, some mistake.
	{
		PASSERT(portId);
		return STATUS_FAIL;
	}

	CActivePort* p_updPort = new CActivePort(*p_port);

	if (m_pActivePorts.erase(port) == 0) //returns number of members erased
	{
		PDELETE(p_updPort);
		PASSERT(portId);
		return STATUS_FAIL;
	}
	p_updPort->SetConfId(rsrcConfId);
	p_updPort->SetPartyId(rsrcPartyId);

	m_pActivePorts.insert(*p_updPort);

	PDELETE(p_updPort);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::SetAllFreePortsToFreeDisable()
{
	PTRACE(eLevelInfoNormal, "CUnitMFA::SetAllFreePortsToFreeDisable");
	for (WORD i = 0; i < m_maxUnitNumPorts; i++)
	{
		if (PORT_FREE == (*m_pPortsList)[i])
		{
			(*m_pPortsList)[i] = PORT_FREE_DISABLED;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CUnitMFA::SetFreePortsToFreeDisable(WORD port)
{
	PTRACE(eLevelInfoNormal, "CUnitMFA::SetFreePortsToFreeDisable");

	//first, check that there isn't an overflow
	if (port <= m_pPortsList->size())
	{
		if ((PORT_FREE == (*m_pPortsList)[port]))
		{
			(*m_pPortsList)[port] = PORT_FREE_DISABLED;
		}
	}
}


////////////////////////////////////////////////////////////////////////////
//                        CSpanRTM
////////////////////////////////////////////////////////////////////////////
const char* CSpanRTM::NameOf() const
{
	return "CSpanRTM";
}

////////////////////////////////////////////////////////////////////////////
CSpanRTM::CSpanRTM(WORD bId, WORD uId) : CUnitRsrc(bId, uId)
{
	// Span created at first when service still not configured

	memset(m_serviceName, '\0', sizeof(m_serviceName));

	m_spanType                = SPAN_GENERIC;
	m_numPorts                = 0;
	m_numFreePorts            = 0;
	m_numDialOutReservedPorts = 0;
	m_pVirtualPorts           = NULL;
	m_isAllocated             = FALSE;
	m_pActivePorts            = new CActivePortsList;
}

////////////////////////////////////////////////////////////////////////////
CSpanRTM::CSpanRTM(const CSpanRTM& other) : CUnitRsrc(other), m_pActivePorts(new std::set<CActivePort>(*(other.m_pActivePorts)))
{
	strcpy_safe(m_serviceName, other.m_serviceName);

	m_spanType                = other.m_spanType;
	m_numPorts                = other.m_numPorts;
	m_numFreePorts            = other.m_numFreePorts;
	m_numDialOutReservedPorts = other.m_numDialOutReservedPorts;

	if (other.m_pVirtualPorts == NULL || m_numPorts == 0)
		m_pVirtualPorts = NULL;

	else
	{
		m_pVirtualPorts = new ePortAllocTypes[m_numPorts];
		for (int i = 0; i < m_numPorts; i++)
			m_pVirtualPorts[i] = other.m_pVirtualPorts[i];
	}

	m_isAllocated = other.m_isAllocated;
}

////////////////////////////////////////////////////////////////////////////
CSpanRTM::~CSpanRTM()
{
	if (m_pVirtualPorts)
	{
		delete [] m_pVirtualPorts;
		m_pVirtualPorts = NULL;
	}

	m_pActivePorts->clear();
	PDELETE(m_pActivePorts);
	m_pActivePorts = 0;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSpanRTM::ConfigureServiceOnSpan(char* serviceName, eRTMSpanType type, BYTE isEnabled)
{
	if (type == SPAN_GENERIC || serviceName == NULL)
		return STATUS_FAIL;

	if (m_serviceName[0] != '\0') // service already defined on this Span
	{
		if (strcmp(serviceName, m_serviceName) == 0)
		{
			TRACEINTO << "ServiceName:" << serviceName << " - Service already configured on Span";
			return STATUS_OK;
		}
		else
		{
			TRACEINTO << "ServiceName:" << serviceName << " - Failed, Another service already configured on Span";
			return STATUS_FAIL;
		}
	}

	strcpy_safe(m_serviceName, serviceName);

	m_spanType = type;

	if (type == TYPE_SPAN_T1)
		m_numPorts = m_numFreePorts = NUM_T1_PORTS;
	else if (type == TYPE_SPAN_E1)
		m_numPorts = m_numFreePorts = NUM_E1_PORTS;

	if (m_pVirtualPorts)
		delete [] m_pVirtualPorts;

	m_pVirtualPorts = new ePortAllocTypes[m_numPorts];

	for (int i = 0; i < m_numPorts; i++)
		m_pVirtualPorts[i] = PORT_FREE;

	SetEnabled(isEnabled);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CSpanRTM::SetNullConfiguration()
{
	m_spanType                = SPAN_GENERIC;
	m_numPorts                = 0;
	m_numFreePorts            = 0;
	m_numDialOutReservedPorts = 0;
	m_isAllocated             = FALSE;

	memset(m_serviceName, '\0', sizeof(m_serviceName));

	if (m_pVirtualPorts)
	{
		delete [] m_pVirtualPorts;
		m_pVirtualPorts = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSpanRTM::RemoveReservedPort(WORD portId)
{
	if (m_pVirtualPorts[portId] == PORT_RESERVED) // it might have been forced
		m_pVirtualPorts[portId] = PORT_FREE;

	PASSERT_AND_RETURN_VALUE(m_numDialOutReservedPorts < 1, STATUS_FAIL);

	m_numDialOutReservedPorts -= 1;
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSpanRTM::MovePortFromReservedToOccupied(WORD& portId, WORD acceleratorId, ConfRsrcID confId, PartyRsrcID partyId)
{
	std::ostringstream msg;
	msg << "PortId:" << portId << ", ConfId:" << confId << ", PartyId:" << partyId;

	PASSERTSTREAM_AND_RETURN_VALUE(m_numDialOutReservedPorts < 1, msg.str().c_str(), STATUS_FAIL);
	PASSERTSTREAM_AND_RETURN_VALUE(m_numFreePorts < 1, msg.str().c_str(), STATUS_FAIL);

	m_isAllocated = TRUE;

	if (m_pVirtualPorts[portId] == PORT_OCCUPIED) // if it is already occupied, we will have to find another one
	{
		STATUS status = FindFreePort(portId, TRUE);
		TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, msg.str().c_str() << " - Failed to find free port", status);
	}

	m_numFreePorts--;
	m_numDialOutReservedPorts--;
	m_pVirtualPorts[portId] = PORT_OCCUPIED;

	AddActivePort(portId, acceleratorId, confId, partyId, 0xFFFF, ePhysical_rtm);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSpanRTM::GetRsrcDetail(CRsrcDetailElement* pDetail)
{
	pDetail->m_numPorts       = m_numPorts; // 23 or 30 ports
	pDetail->m_numActivePorts = m_pActivePorts->size();

	//FreeActivePorts(pDetail);//Avoid leaks this caused core dump
	pDetail->m_activePorts = new CDetailActivePort*[m_numPorts];
	pDetail->m_slotNumber  = this->GetBoardId();
	pDetail->m_cardType    = 0;             // BoardType2CardType(m_type);

	ConfMonitorID  monitorConfId  = 0xFFFFFFFF;
	PartyMonitorID monitorPartyId = 0xFFFFFFFF;

	CConfRsrcDB*   pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB)
	{
		// Initiate the list of ports with default values
		for (int i = 0; i < m_numPorts; i++)
		{
			pDetail->m_activePorts[i]           = new CDetailActivePort;
			pDetail->m_activePorts[i]->m_portId = i;
		}

		// Update the active ports (the inactive ports stay with default values)
		CActivePortsList::iterator portItr;
		for (portItr = m_pActivePorts->begin(); portItr != m_pActivePorts->end(); portItr++)
		{
			WORD portId = portItr->GetPortId();

			if (m_pVirtualPorts[portId] != PORT_OCCUPIED)
			{
				// In active ports list there should be only active ports
				PASSERTSTREAM(1, "PortId:" << portId << " - In active ports list there should be only active ports");
				return STATUS_FAIL;
			}

			if (portId >= m_numPorts)
			{
				PASSERTSTREAM(1, "PortId:" << portId << " - The portId from GetPortId exceed num_ports");
				continue;
			}

			pDetail->m_activePorts[portId]->m_promilUtilized = (WORD)(portItr->GetPromilUtilized());
			pDetail->m_activePorts[portId]->m_portId         = portItr->GetPortId();

			DWORD i1 = portItr->GetConfId();
			DWORD i2 = portItr->GetPartyId();

			pConfRsrcDB->GetMonitorIdsRsrcIds(i1, i2, monitorConfId, monitorPartyId);

			pDetail->m_activePorts[portId]->m_confId       = monitorConfId;
			pDetail->m_activePorts[portId]->m_partyId      = monitorPartyId;
			pDetail->m_activePorts[portId]->m_porType      = 0;      // while. After it has to be add
			pDetail->m_activePorts[portId]->m_connectionId = 0;      // while
			pDetail->m_activePorts[portId]->m_isActive     = TRUE;
		}
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CSpanRTM::FreeActivePorts(CRsrcDetailElement* pDetail)
{
	TRACEINTO << "m_numPorts: " << m_numPorts;

	for (int i = 0; i < m_numPorts; i++)
	{
		TRACEINTO << "pDetail->m_activePorts[i]: " << pDetail->m_activePorts[i];
		POBJDELETE(pDetail->m_activePorts[i]);
	}

	TRACEINTO << "pDetail->m_activePorts: " << pDetail->m_activePorts;
	delete[] pDetail->m_activePorts;
}

////////////////////////////////////////////////////////////////////////////
void CSpanRTM::Dump(std::ostream& msg)
{
	msg.setf(std::ios::left, std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\nCSpanRTM::Dump----";
	msg << "\nBoardId          :" << GetBoardId();
	msg << "\nSubBoardId       :" << GetSubBoardId();
	msg << "\nUnitId           :" << GetUnitId();
	msg << "\nServiceName      :" << m_serviceName;
	msg << "\nSpanType         :";
	switch (m_spanType)
	{
	case SPAN_GENERIC: msg << "SPAN_GENERIC"; break;
	case TYPE_SPAN_T1: msg << "TYPE_SPAN_T1"; break;
	case TYPE_SPAN_E1: msg << "TYPE_SPAN_E1"; break;
	default: msg << "Unknown"; break;
	}

	msg << "\nNumPorts         :" << m_numPorts;
	msg << "\nNumFreePorts     :" << m_numFreePorts;
	msg << "\nNumReservedPorts :" << m_numDialOutReservedPorts;
	msg << "\nActive ports list:" << (int)m_pActivePorts->size();
	DumpActivePorts(msg);
}

////////////////////////////////////////////////////////////////////////////
void CSpanRTM::DumpActivePorts(std::ostream& msg)
{
	msg.setf(std::ios::left, std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	CPrettyTable<WORD, ConfRsrcID, PartyRsrcID> tbl("PortId", "ConfId", "PartyId");
	CActivePortsList::iterator portItr;
	for (portItr = m_pActivePorts->begin(); portItr != m_pActivePorts->end(); portItr++)
		tbl.Add(portItr->GetPortId(), portItr->GetConfId(), portItr->GetPartyId());
	msg << tbl.Get();
}

////////////////////////////////////////////////////////////////////////////
STATUS CSpanRTM::FindFreePort(WORD& portId, BOOL bPortCanAlsoBeReserved)
{
	std::ostringstream msg;
	msg << "BoardId:" << GetBoardId() << ", UnitId:" << GetUnitId() << ", NumPorts:" << GetNumPorts() << ", NumFreePorts:" << GetNumFreePorts() << ", IsEnabled:" << (int)GetIsEnabled() << ", IsDisabledManually:" << (int)GetIsDisabledManually() << ", IsFatal:" << (int)GetIsFatal() << ", PortCanBeReserved:" << (int)bPortCanAlsoBeReserved;

	if (!m_numFreePorts || !GetIsEnabled())
	{
		TRACEINTO << msg.str().c_str() << " - Failed, No free ports or not enabled";
		return STATUS_INSUFFICIENT_RTM_RSRC;
	}

	int reservedPortId = -1;
	int i;
	for (i = 0; i < m_numPorts; i++)
	{
		if (m_pVirtualPorts[i] == PORT_FREE)
		{
			portId = i;
			break;
		}
		if (m_pVirtualPorts[i] == PORT_RESERVED && reservedPortId == -1)
		{
			reservedPortId = i;
		}
	}

	if (i == m_numPorts) // not found specific free port
	{
		if (bPortCanAlsoBeReserved == FALSE) // we can not take a reserved port
		{
			TRACEINTO << msg.str().c_str() << " - Failed, Free port not found";
			return STATUS_INSUFFICIENT_RTM_RSRC;
		}

		if (reservedPortId == -1)            // reserved free port not found
		{
			TRACEINTO << msg.str().c_str() << " - Failed, Reserved free port not found";
			return STATUS_INSUFFICIENT_RTM_RSRC;
		}

		portId = reservedPortId;
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSpanRTM::AllocatePort(ConfRsrcID confId, PartyRsrcID partyId, eResourceTypes type, WORD& portId, BOOL bMoveToReserved)
{
	BOOL bPortCanAlsoBeReserved;
	if (bMoveToReserved == TRUE)
		bPortCanAlsoBeReserved = FALSE;   // when reserving a port, we can't force it over an already reserved port
	else
		bPortCanAlsoBeReserved = TRUE;    // when occupying a port we can "force" over a reserved port

	STATUS status = FindFreePort(portId, bPortCanAlsoBeReserved);
	if (status != STATUS_OK)
	{
		TRACEINTO << "ConfId:" << confId << ", PartyId:" << partyId << ", ResourceType:" << type << " - Failed find free port";
		return status;
	}

	if (bMoveToReserved)
	{
		m_pVirtualPorts[portId] = PORT_RESERVED;
		m_numDialOutReservedPorts++;
	}
	else
	{
		m_pVirtualPorts[portId] = PORT_OCCUPIED;
		m_numFreePorts--;
		AddActivePort(portId, 0, confId, partyId, 0xFFFF, type);
		m_isAllocated = TRUE;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSpanRTM::DeAllocatePort(WORD portId, BOOL bIsUpdated)
{
	if (portId > m_numPorts)
		return STATUS_FAIL;

	BOOL bShouldFreeActivePort = TRUE;

	if (m_pVirtualPorts[portId] == PORT_FREE)
	{
		bShouldFreeActivePort = FALSE;
		if (bIsUpdated == FALSE) // we're freeing a reserved port, that meanwhile has been forced and released
		{
			TRACEINTO << "PortId:" << portId << " - Freeing already free port that has been forced";
			m_numDialOutReservedPorts--;
		}
		else
		{
			TRACEINTO << "PortId:" << portId << " - Trying to free already free RTM port";
		}
	}
	else
	{
		if (m_pVirtualPorts[portId] == PORT_RESERVED)
		{
			TRACEINTO << "PortId:" << portId << " - Trying to free reserved RTM port";
			m_numDialOutReservedPorts--;
			bShouldFreeActivePort = FALSE;
		}
		else
		{
			m_numFreePorts++;
		}

		m_pVirtualPorts[portId] = PORT_FREE;
	}

	if (m_numPorts == m_numFreePorts)
		m_isAllocated = FALSE;

	if (bShouldFreeActivePort)
		FreeActivePort(portId);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CSpanRTM::AddActivePort(WORD portId, WORD acceleratorId, ConfRsrcID confId, PartyRsrcID partyId, WORD promilUtilized, eResourceTypes type)
{
	CActivePort port(portId, acceleratorId, confId, partyId, 0, promilUtilized, type);

	if (m_pActivePorts->find(port) != m_pActivePorts->end())
	{
		// some error, port exists
		PASSERT(portId);
		return -1; // ESTATUS???
	}

	m_pActivePorts->insert(port);
	return 0; // STATUS_OK
}

////////////////////////////////////////////////////////////////////////////
int CSpanRTM::FreeActivePort(WORD portId)
{
	CActivePort port(portId);

	if (m_pActivePorts->erase(port) == 0) // returns number of members erased
	{
		PASSERT(portId);
		return -1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSpanRTM::UpdateActivePort(WORD portId, ConfRsrcID confId, PartyRsrcID partyId)
{
	CActivePort port(portId);

	CActivePortsList::iterator p_port = m_pActivePorts->find(port);
	if (p_port == m_pActivePorts->end()) // no such active port, some mistake.
	{
		PASSERTSTREAM(1, "PortId:" << portId << " - No such active port");
		return STATUS_FAIL;
	}

	CActivePort* p_newPort = new CActivePort(*p_port);

	if (m_pActivePorts->erase(port) == 0) // returns number of members erased
	{
		PDELETE(p_newPort);
		PASSERT(portId);
		return STATUS_FAIL;
	}

	p_newPort->SetConfId(confId);
	p_newPort->SetPartyId(partyId);

	m_pActivePorts->insert(*p_newPort);

	PDELETE(p_newPort);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
//                        CRecordingJunction
////////////////////////////////////////////////////////////////////////////
CRecordingJunction::CRecordingJunction(WORD bId, WORD uId, size_t portId, DWORD confId, DWORD partyId)
	: CUnitRsrc(bId, uId), CActivePort(portId, confId, partyId)
{
	junctionId = 0;
}

////////////////////////////////////////////////////////////////////////////
CRecordingJunction::~CRecordingJunction()
{
}

////////////////////////////////////////////////////////////////////////////
const char*   CRecordingJunction::NameOf() const
{
	return "CRecordingJunction";
}

////////////////////////////////////////////////////////////////////////////
WORD operator ==(const CRecordingJunction& lhs, const CRecordingJunction& rhs)
{
	return (lhs.junctionId == rhs.junctionId); //temp? tbd?
}

////////////////////////////////////////////////////////////////////////////
bool operator <(const CRecordingJunction& lhs, const CRecordingJunction& rhs)
{
	return (lhs.junctionId < rhs.junctionId);  //temp? tbd?
}
