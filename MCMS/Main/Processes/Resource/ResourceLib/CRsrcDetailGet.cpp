#include "CRsrcDetailGet.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ApiStatuses.h"
#include "HelperFuncs.h"
#include "SystemResources.h"
#include "StlUtils.h"
#include "PrettyTable.h"

////////////////////////////////////////////////////////////////////////////
//                        CCardRsrc
////////////////////////////////////////////////////////////////////////////
CCardRsrc::CCardRsrc()
{
	m_unitId = 0xFFFF;
	m_unitType = 0;
	m_unitCfg = 0xFF;
	m_unitStatus = 0;
	m_portsNumber = 0;
	m_activMask1 = 0;
	m_activMask2 = 0;

	strcpy(m_serviceName, "");

	m_utilization = 0xFFFF;
	m_currentType = 0xFF;
	m_UpdateCounter = 0;
}

////////////////////////////////////////////////////////////////////////////
CCardRsrc::CCardRsrc(const CCardRsrc &other) :
		CSerializeObject(other)
{
	m_unitId = other.m_unitId;
	m_unitType = other.m_unitType;
	m_unitCfg = other.m_unitCfg;
	m_unitStatus = other.m_unitStatus;
	m_portsNumber = other.m_portsNumber;
	m_activMask1 = other.m_activMask1;
	m_activMask2 = other.m_activMask2;

	strncpy(m_serviceName, other.m_serviceName, NET_SERVICE_PROVIDER_NAME_LEN);

	m_utilization = other.m_utilization;
	m_currentType = other.m_currentType;
	m_UpdateCounter = other.m_UpdateCounter;
}

////////////////////////////////////////////////////////////////////////////
CCardRsrc& CCardRsrc::operator =(const CCardRsrc &other)
{
	m_unitId = other.m_unitId;
	m_unitType = other.m_unitType;
	m_unitCfg = other.m_unitCfg;
	m_unitStatus = other.m_unitStatus;
	m_portsNumber = other.m_portsNumber;
	m_activMask1 = other.m_activMask1;
	m_activMask2 = other.m_activMask2;

	strncpy(m_serviceName, other.m_serviceName, NET_SERVICE_PROVIDER_NAME_LEN);

	m_utilization = other.m_utilization;
	m_currentType = other.m_currentType;
	m_UpdateCounter = other.m_UpdateCounter;
	return *this;
}

////////////////////////////////////////////////////////////////////////////
CCardRsrc::~CCardRsrc()
{
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pUnitNode = pFatherNode->AddChildNode("UNIT_RESOURCE_DESCRIPTOR");

	pUnitNode->AddChildNode("UNIT_NUMBER", m_unitId);
	pUnitNode->AddChildNode("UNIT_TYPE", m_unitType, UNIT_TYPE_ENUM);
	pUnitNode->AddChildNode("CONFIGURATION_DATA", m_unitCfg);
	pUnitNode->AddChildNode("NETWORK_SERVICE_NAME", m_serviceName);
	CXMLDOMElement* pChild = NULL;
	pChild = pUnitNode->AddChildNode("UNIT_STATUS");
	if (pChild)
	{
		pChild->AddChildNode("ID", m_unitStatus);

		char* pszTemp = new char[256];
		memset(pszTemp, 0, 256);
		if (IsAvailable())
			strcat(pszTemp, "Available;");
		if (IsActive())
			strcat(pszTemp, "Active;");
		if (IsDisabledByError())
			strcat(pszTemp, "Disabled(by error);");
		if (IsDisabledManually())
			strcat(pszTemp, "Disabled(manually);");
		if (IsDiagnostics())
			strcat(pszTemp, "Diagnostics;");

		pChild->AddChildNode("DESCRIPTION", pszTemp);
		PDELETEA(pszTemp);
	}
	pUnitNode->AddChildNode("PORTS_NUMBER", m_portsNumber);
	pUnitNode->AddChildNode("ACTIVE_MASK_1", m_activMask1);
	pUnitNode->AddChildNode("ACTIVE_MASK_2", m_activMask2);
	pUnitNode->AddChildNode("UTILIZATION", m_utilization);
	pUnitNode->AddChildNode("CURRENT_TYPE", m_currentType);

}

////////////////////////////////////////////////////////////////////////////
int CCardRsrc::DeSerializeXml(CXMLDOMElement *pActionNode, char* pszError, const char* action)
{
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetUnitId(const WORD unitId)
{
	m_unitId = unitId;
}

////////////////////////////////////////////////////////////////////////////
WORD CCardRsrc::GetUnitId() const
{
	return m_unitId;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetUnitType(const BYTE unitType)
{
	m_unitType = unitType;
}

////////////////////////////////////////////////////////////////////////////
BYTE CCardRsrc::GetUnitType() const
{
	return m_unitType;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetUnitCfg(const BYTE unitCfg)
{
	m_unitCfg = unitCfg;
}

////////////////////////////////////////////////////////////////////////////
BYTE CCardRsrc::GetUnitCfg() const
{
	return m_unitCfg;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetUnitStatus(const DWORD unitStatus)
{
	m_unitStatus = unitStatus;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardRsrc::GetUnitStatus() const
{
	return m_unitStatus;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetActive(WORD bl)
{
	if (bl == FALSE)
		m_unitStatus &= 0xFFFFFFFE;
	else
		m_unitStatus |= 0x00000001;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetDisabledByError(WORD bl)
{
	if (bl == FALSE)
		m_unitStatus &= 0xFFFFFFFD;
	else
		m_unitStatus |= 0x00000002;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetDisabledManually(WORD bl)
{
	if (bl == FALSE)
		m_unitStatus &= 0xFFFFFFFB;
	else
		m_unitStatus |= 0x00000004;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetDiagnostics(WORD bl)
{
	if (bl == FALSE)
		m_unitStatus &= 0xFFFFFFF7;
	else
		m_unitStatus |= 0x00000008;
}

////////////////////////////////////////////////////////////////////////////
WORD CCardRsrc::IsActive() const
{
	if ((m_unitStatus & 0x00000001) == 1)
		return TRUE;
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CCardRsrc::IsDisabledByError() const
{
	if ((m_unitStatus & 0x00000002) == 2)
		return TRUE;
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CCardRsrc::IsDisabledManually() const
{
	if ((m_unitStatus & 0x00000004) == 4)
		return TRUE;
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CCardRsrc::IsDiagnostics() const
{
	if ((m_unitStatus & 0x00000008) == 8)
		return TRUE;
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CCardRsrc::IsAvailable() const
{
	if (m_unitStatus == 0)
		return TRUE;
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetPortsNumber(const WORD portsNum)
{
	m_portsNumber = portsNum;
}

////////////////////////////////////////////////////////////////////////////
WORD CCardRsrc::GetPortsNumber() const
{
	return m_portsNumber;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetActivMask1(const DWORD activMask)
{
	m_activMask1 = activMask;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardRsrc::GetActivMask1() const
{
	return m_activMask1;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetActivMask2(const DWORD activMask)
{
	m_activMask2 = activMask;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardRsrc::GetActivMask2() const
{
	return m_activMask2;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetServiceName(const char* serviceName)
{
	if (serviceName)
	{
		strncpy(m_serviceName, serviceName, sizeof(m_serviceName) - 1);
		m_serviceName[sizeof(m_serviceName) - 1] = '\0';
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetUtilization(const DWORD promil)
{
	m_utilization = promil;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardRsrc::GetUtilization() const
{
	return m_utilization;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::SetCurrentType(const BYTE type)
{
	m_currentType = type;
}

////////////////////////////////////////////////////////////////////////////
BYTE CCardRsrc::GetCurrentType() const
{
	return m_currentType;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardRsrc::GetUpdateCounter() const
{
	return m_UpdateCounter;
}

////////////////////////////////////////////////////////////////////////////
void CCardRsrc::IncreaseUpdateCounter()
{
	m_UpdateCounter++;
}



////////////////////////////////////////////////////////////////////////////
//                        CCommDynCard
////////////////////////////////////////////////////////////////////////////
CCommDynCard::CCommDynCard()
{
	m_numb_of_units = 0;
	m_BoardID = 0;

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
		m_pCardRsrc[i] = NULL;

	m_rsrc_ind = 0;
	m_updateCounter = 0;
	m_bChanged = FALSE;

}

////////////////////////////////////////////////////////////////////////////
CCommDynCard::CCommDynCard(const WORD slotNumber)
{
	m_numb_of_units = 0;
	m_BoardID = slotNumber;

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
		m_pCardRsrc[i] = NULL;

	m_rsrc_ind = 0;
	m_updateCounter = 0;
	m_bChanged = FALSE;
}

////////////////////////////////////////////////////////////////////////////
CCommDynCard& CCommDynCard::operator =(const CCommDynCard &other)
{
	m_numb_of_units = other.m_numb_of_units;
	m_BoardID = other.m_BoardID;

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		PDELETE(m_pCardRsrc[i]);

		if (other.m_pCardRsrc[i] != NULL)
			m_pCardRsrc[i] = new CCardRsrc(*other.m_pCardRsrc[i]);
	}
	m_rsrc_ind = other.m_rsrc_ind;

	m_updateCounter = other.m_updateCounter;
	m_bChanged = other.m_bChanged;
	return *this;
}

////////////////////////////////////////////////////////////////////////////
void CCommDynCard::SetBoardId(WORD boardID)
{
	m_BoardID = boardID;
}

////////////////////////////////////////////////////////////////////////////
CCommDynCard::CCommDynCard(const CCommDynCard &other) :
		CSerializeObject(other)
{
	m_numb_of_units = 0;
	m_BoardID = 0;

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
		m_pCardRsrc[i] = NULL;
	m_rsrc_ind = 0;
	m_updateCounter = 0;
	m_bChanged = FALSE;

	*this = other;
}

////////////////////////////////////////////////////////////////////////////
CCommDynCard::~CCommDynCard()
{
	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
		PDELETE(m_pCardRsrc[i]);
}

////////////////////////////////////////////////////////////////////////////
void CCommDynCard::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pCardNode = pFatherNode->AddChildNode("CARD_DETAILS");
	pCardNode->AddChildNode("OBJ_TOKEN", 1);
	pCardNode->AddChildNode("CHANGED", TRUE, _BOOL);

	STATUS status = STATUS_OK;

	CXMLDOMElement* pDescNode = pCardNode->AddChildNode("CARD_SUMMARY_DESCRIPTOR");

	CXMLDOMElement* pCommonNode = pDescNode->AddChildNode("CARD_COMMON_DATA");

	pCommonNode->AddChildNode("SLOT_NUMBER", m_BoardID, BOARD_ENUM);

	for (int i = 0; i < (int)m_numb_of_units; i++)
		m_pCardRsrc[i]->SerializeXml(pDescNode);
}

////////////////////////////////////////////////////////////////////////////
int CCommDynCard::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
BYTE CCommDynCard::GetChanged() const
{
	return m_bChanged;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCommDynCard::GetUpdateCounter() const
{
	return m_updateCounter;
}

////////////////////////////////////////////////////////////////////////////
void CCommDynCard::IncreaseUpdateCounter()
{
	m_updateCounter++;
	if (m_updateCounter == 0xFFFFFFFF)
		m_updateCounter = 0;
}

////////////////////////////////////////////////////////////////////////////
int CCommDynCard::AddRsrc(const CCardRsrc& other)
{
	if (m_numb_of_units >= MAX_NUM_OF_UNITS)
		return UNKNOWN_ACTION;

	if (FindRsrc(other) != NOT_FIND)
		return NOT_FIND;

	m_pCardRsrc[m_numb_of_units] = new CCardRsrc(other);

	m_numb_of_units++;
	IncreaseUpdateCounter();
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CCommDynCard::FindRsrc(const CCardRsrc& other)
{
	for (int i = 0; i < (int)m_numb_of_units; i++)
	{
		if (m_pCardRsrc[i] != NULL)
		{
			if (m_pCardRsrc[i]->GetUnitId() == other.GetUnitId())
				return i;
		}
	}
	return NOT_FIND;
}

////////////////////////////////////////////////////////////////////////////
int CCommDynCard::DelRsrc(const CCardRsrc& other)
{
	int ind = FindRsrc(other);
	if (ind == NOT_FIND || ind >= MAX_NUM_OF_UNITS)
		return NOT_FIND;

	PASSERT_AND_RETURN_VALUE(m_numb_of_units > MAX_NUM_OF_UNITS, NOT_FIND);

	PDELETE(m_pCardRsrc[ind]);
	int i = 0;
	int j = 0;
	for (i = 0; i < (int)m_numb_of_units; i++)
	{
		if (m_pCardRsrc[i] == NULL)
			break;
	}
	for (j = i; j < (int)m_numb_of_units - 1; j++)
	{
		m_pCardRsrc[j] = m_pCardRsrc[j + 1];
	}
	m_pCardRsrc[m_numb_of_units - 1] = NULL;
	m_numb_of_units--;
	IncreaseUpdateCounter();
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
CCardRsrc* CCommDynCard::GetFirstRsrc()
{
	m_rsrc_ind = 1;
	return m_pCardRsrc[0];
}

////////////////////////////////////////////////////////////////////////////
CCardRsrc* CCommDynCard::GetNextRsrc()
{
	if (m_rsrc_ind >= m_numb_of_units)
		return NULL;
	return m_pCardRsrc[m_rsrc_ind++];
}

////////////////////////////////////////////////////////////////////////////
CCardRsrc* CCommDynCard::GetFirstRsrc(int& nPos)
{
	CCardRsrc* pRsrc = GetFirstRsrc();
	nPos = m_rsrc_ind;
	return pRsrc;
}


////////////////////////////////////////////////////////////////////////////
CCardRsrc* CCommDynCard::GetNextRsrc(int& nPos)
{
	m_rsrc_ind = nPos;
	CCardRsrc* pRsrc = GetNextRsrc();
	nPos = m_rsrc_ind;
	return pRsrc;
}

////////////////////////////////////////////////////////////////////////////
CCardRsrc* CCommDynCard::GetCurrentRsrc(const WORD unitId)
{
	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		if (m_pCardRsrc[i] != NULL)
		{
			if (m_pCardRsrc[i]->GetUnitId() == unitId)
				return m_pCardRsrc[i];
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
void CCommDynCard::SetDisabledByError(WORD bl)
{
	for (int i = 0; i < (int)m_numb_of_units; i++)
		m_pCardRsrc[i]->SetDisabledByError(bl);
	IncreaseUpdateCounter();
}

////////////////////////////////////////////////////////////////////////////
void CCommDynCard::SetDisabledManually(WORD bl)
{
	for (int i = 0; i < (int)m_numb_of_units; i++)
		m_pCardRsrc[i]->SetDisabledManually(bl);
	IncreaseUpdateCounter();
}


////////////////////////////////////////////////////////////////////////////
//                        CRsrcDetailGet
////////////////////////////////////////////////////////////////////////////
CRsrcDetailGet::CRsrcDetailGet()
{
	m_DynCard = new CCommDynCard;
	m_BoardID = 0;
	m_SubBoardID = 0;
}

////////////////////////////////////////////////////////////////////////////
CRsrcDetailGet::CRsrcDetailGet(const CRsrcDetailGet &other) : CSerializeObject(other)
{
	m_DynCard = NULL;
	*this = other;
}

////////////////////////////////////////////////////////////////////////////
CRsrcDetailGet& CRsrcDetailGet::operator=(const CRsrcDetailGet &other)
{
	m_updateCounter = other.m_updateCounter;
	m_BoardID = other.m_BoardID;
	m_SubBoardID = other.m_SubBoardID;
	if (other.m_DynCard)
	{
		m_DynCard = new CCommDynCard;
		*m_DynCard = *other.m_DynCard;
	}

	return *this;
}

////////////////////////////////////////////////////////////////////////////
CRsrcDetailGet::~CRsrcDetailGet()
{
	POBJDELETE(m_DynCard);
}

////////////////////////////////////////////////////////////////////////////
int CRsrcDetailGet::DeSerializeXml(CXMLDOMElement *pResNode, char *pszError, const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	DeSerializeXml(pResNode, pszError, numAction);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CRsrcDetailGet::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, int nAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode, "BOARD_NUMBER", &m_BoardID, BOARD_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "SUB_BOARD_ID", &m_SubBoardID, SUB_BOARD_ENUM);

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcDetailGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	STATUS status = STATUS_OK;

	if (m_DynCard)
		m_DynCard->SerializeXml(pFatherNode);
}

////////////////////////////////////////////////////////////////////////////
WORD CRsrcDetailGet::GetBoardId()
{
	return m_BoardID;
}

////////////////////////////////////////////////////////////////////////////
DWORD CRsrcDetailGet::GetSubBoardId()
{
	return m_SubBoardID;
}

////////////////////////////////////////////////////////////////////////////
CCommDynCard* CRsrcDetailGet::GetDynCard()
{
	return m_DynCard;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcDetailGet::SetDynCard(CCommDynCard* p_dynCard)
{
	*m_DynCard = *p_dynCard;
}


////////////////////////////////////////////////////////////////////////////
//                        CUnitRsrcDetailGet
////////////////////////////////////////////////////////////////////////////
CUnitRsrcDetailGet::CUnitRsrcDetailGet()
{
	m_BoardID = 0;
	m_SubBoardID = 0;
	m_UnitID = 0;
	m_DetElmnt = new CRsrcDetailElement;
	m_iClassType = CLASS_TYPE_NO_UNIT_ACTION;

}

////////////////////////////////////////////////////////////////////////////
CUnitRsrcDetailGet::CUnitRsrcDetailGet(const CUnitRsrcDetailGet &other) :
		CSerializeObject(other)
{
	m_updateCounter = other.m_updateCounter;
	m_BoardID = other.m_BoardID;
	m_SubBoardID = other.m_SubBoardID;
	m_UnitID = other.m_UnitID;
	m_iClassType = other.m_iClassType;

	if (other.m_iClassType == CLASS_TYPE_NO_UNIT_ACTION)
	{
		m_DetElmnt = new CRsrcDetailElement();
		*m_DetElmnt = *other.m_DetElmnt;
	}
	else
	{
		m_DetElmnt = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////
CUnitRsrcDetailGet& CUnitRsrcDetailGet::operator=(const CUnitRsrcDetailGet &other)
{
	m_updateCounter = other.m_updateCounter;
	m_BoardID = other.m_BoardID;
	m_SubBoardID = other.m_SubBoardID;
	m_UnitID = other.m_UnitID;
	m_iClassType = other.m_iClassType;
	if (m_iClassType == CLASS_TYPE_NO_UNIT_ACTION)
		*m_DetElmnt = *other.m_DetElmnt;

	return *this;
}

////////////////////////////////////////////////////////////////////////////
CUnitRsrcDetailGet::~CUnitRsrcDetailGet()
{
	POBJDELETE(m_DetElmnt);
}

////////////////////////////////////////////////////////////////////////////
int CUnitRsrcDetailGet::DeSerializeXml(CXMLDOMElement *pResNode, char *pszError, const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	DeSerializeXml(pResNode, pszError, numAction);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CUnitRsrcDetailGet::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, int nAction)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement* pUnitNode = NULL;
	if (nAction == RESET_UNIT_ACTION)
		pUnitNode = pActionNode;
	else
	{
		GET_VALIDATE_CHILD(pActionNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD);
		GET_MANDATORY_CHILD_NODE(pActionNode, "UNIT_POSITION", pUnitNode);
	}

	GET_VALIDATE_CHILD(pUnitNode, "BOARD_NUMBER", &m_BoardID, BOARD_ENUM);
	GET_VALIDATE_CHILD(pUnitNode, "UNIT_NUMBER", &m_UnitID, UNIT_ENUM);
	GET_VALIDATE_CHILD(pUnitNode, "SUB_BOARD_ID", &m_SubBoardID, SUB_BOARD_ENUM);

	if (nStatus)
		return nStatus;

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////
void CUnitRsrcDetailGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	STATUS status = STATUS_OK;

	if (m_DetElmnt)
		m_DetElmnt->SerializeXml(pFatherNode);
}

////////////////////////////////////////////////////////////////////////////
WORD CUnitRsrcDetailGet::GetBoardId()
{
	return m_BoardID;
}

////////////////////////////////////////////////////////////////////////////
DWORD CUnitRsrcDetailGet::GetSubBoardId()
{
	return m_SubBoardID;
}

////////////////////////////////////////////////////////////////////////////
WORD CUnitRsrcDetailGet::GetUnitId()
{
	return m_UnitID;
}

////////////////////////////////////////////////////////////////////////////
CRsrcDetailElement* CUnitRsrcDetailGet::GetDetElmnt()
{
	return m_DetElmnt;
}

////////////////////////////////////////////////////////////////////////////
void CUnitRsrcDetailGet::SetNClassType(int iClassType)
{
	m_iClassType = iClassType;
}


////////////////////////////////////////////////////////////////////////////
//                        CUnitListRsrcAction
////////////////////////////////////////////////////////////////////////////
CUnitListRsrcAction::CUnitListRsrcAction()
{
	m_numb_of_units = 0;
	for (int i = 0; i < MAX_UNITS_IN_LIST; i++)
		m_CUnitRsrcDetailGet[i] = NULL;
}

////////////////////////////////////////////////////////////////////////////
CUnitListRsrcAction& CUnitListRsrcAction::operator =(const CUnitListRsrcAction &other)
{
	m_numb_of_units = other.m_numb_of_units;
	for (int i = 0; i < MAX_UNITS_IN_LIST; i++)
	{
		PDELETE(m_CUnitRsrcDetailGet[i]);

		if (other.m_CUnitRsrcDetailGet[i] == NULL)
			m_CUnitRsrcDetailGet[i] = NULL;
		else
		{
			m_CUnitRsrcDetailGet[i] = new CUnitRsrcDetailGet(*other.m_CUnitRsrcDetailGet[i]);
			m_CUnitRsrcDetailGet[i]->SetNClassType(CLASS_TYPE_UNIT_ACTION);
		}
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////
CUnitListRsrcAction::~CUnitListRsrcAction()
{
	for (int i = 0; i < MAX_UNITS_IN_LIST; i++)
	{
		PDELETE(m_CUnitRsrcDetailGet[i]);
	}
}

////////////////////////////////////////////////////////////////////////////
int CUnitListRsrcAction::DeSerializeXml(CXMLDOMElement *pResNode, char *pszError, const char * strAction)
{
	int numAction = RESET_UNIT_ACTION;
	DeSerializeXml(pResNode, pszError, numAction);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CUnitListRsrcAction::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, int nAction)
{
	int nStatus = STATUS_OK;
	m_numb_of_units = 0;

	for (int i = 0; i < MAX_UNITS_IN_LIST; i++)
		POBJDELETE(m_CUnitRsrcDetailGet[i]);

	CXMLDOMElement *pChild = NULL;
	CXMLDOMElement *pUnitsListNode = NULL;
	pActionNode->getChildNodeByName(&pUnitsListNode, "UNITS_LIST");
	GET_FIRST_MANDATORY_CHILD_NODE(pUnitsListNode, "UNIT_POSITION", pChild);	// at least one should present

	while (pChild && m_numb_of_units < MAX_UNITS_IN_LIST)
	{
		m_CUnitRsrcDetailGet[m_numb_of_units] = new CUnitRsrcDetailGet;
		nStatus = m_CUnitRsrcDetailGet[m_numb_of_units]->DeSerializeXml(pChild, pszError, nAction);
		if (nStatus)
		{
			POBJDELETE(m_CUnitRsrcDetailGet[m_numb_of_units]);
			return nStatus;
		}
		m_numb_of_units++;
		GET_NEXT_CHILD_NODE(pUnitsListNode, "UNIT_POSITION", pChild);
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
WORD CUnitListRsrcAction::GetNumbOfUnits()
{
	return m_numb_of_units;
}

////////////////////////////////////////////////////////////////////////////
CUnitRsrcDetailGet* CUnitListRsrcAction::GetUnitRsrcDetailGet(int nPos)
{
	PASSERTSTREAM_AND_RETURN_VALUE(nPos >= MAX_UNITS_IN_LIST, "nPos has invalid value " << nPos, NULL);

	if (nPos > m_numb_of_units)
		return NULL;
	return m_CUnitRsrcDetailGet[nPos];
}

////////////////////////////////////////////////////////////////////////////
void CUnitListRsrcAction::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pUnitsListNode = pFatherNode->AddChildNode("UNITS_LIST");

	for (int i = 0; i < m_numb_of_units; i++)
		m_CUnitRsrcDetailGet[i]->SerializeXml(pUnitsListNode);
}


////////////////////////////////////////////////////////////////////////////
//                        CDetailActivePort
////////////////////////////////////////////////////////////////////////////
CDetailActivePort::CDetailActivePort()
{
	m_connectionId = 0;
	m_isActive = FALSE;
	m_portId = 0;
	m_confId = 0;
	m_partyId = 0;
	m_promilUtilized = 0xFFFF;
	m_porType = 0; //TYPE_NONE; // may be obsolete
}

////////////////////////////////////////////////////////////////////////////
CDetailActivePort::CDetailActivePort(const CDetailActivePort& other) : CSerializeObject(other)
{
	*this = other;
}

////////////////////////////////////////////////////////////////////////////
CDetailActivePort::~CDetailActivePort()
{
}

////////////////////////////////////////////////////////////////////////////
CDetailActivePort& CDetailActivePort::operator=(const CDetailActivePort& other)
{
	m_connectionId   = other.m_connectionId;
	m_isActive       = other.m_isActive;
	m_portId         = other.m_portId;
	m_confId         = other.m_confId;
	m_partyId        = other.m_partyId;
	m_promilUtilized = other.m_promilUtilized;
	m_porType        = other.m_porType;

	return *this;
}

////////////////////////////////////////////////////////////////////////////
void CDetailActivePort::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pPort1Node = pFatherNode->AddChildNode("ACTIVE_PORT_DETAILS");

	CXMLDOMElement* pPort2Node = pPort1Node->AddChildNode("PORT_SUMMARY");

	if (!pPort2Node)
		return;

	pPort2Node->AddChildNode("PORT_ID", m_portId);
	pPort2Node->AddChildNode("CONF_ID", m_confId);
	pPort2Node->AddChildNode("PARTY_ID", m_partyId);
	pPort2Node->AddChildNode("UTILIZATION", m_promilUtilized);
	pPort2Node->AddChildNode("PORT_TYPE", m_porType, COP_PORT_TYPE_ENUM);

	pPort1Node->AddChildNode("CONNECTION_ID", m_connectionId);
	pPort1Node->AddChildNode("IS_ACTIVE", m_isActive, _BOOL);
}

////////////////////////////////////////////////////////////////////////////
int CDetailActivePort::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CDetailActivePort::DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action)
{
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CRsrcDetailElement
////////////////////////////////////////////////////////////////////////////
CRsrcDetailElement::CRsrcDetailElement()
{
	m_activePorts = 0;
	m_numActivePorts = 0;
	m_numPorts = 0;
	m_cardType = 0;
	m_slotNumber = 0;

}

////////////////////////////////////////////////////////////////////////////
CRsrcDetailElement::CRsrcDetailElement(const CRsrcDetailElement& other) : CSerializeObject(other)
{
	m_cardType = other.m_cardType;
	m_slotNumber = other.m_slotNumber;
	m_numPorts = other.m_numPorts;
	m_numActivePorts = other.m_numActivePorts;

	m_activePorts = new CDetailActivePort*[m_numPorts];
	for (WORD i = 0; i < m_numPorts; i++)
	{
		if (other.m_activePorts[i])
			m_activePorts[i] = new CDetailActivePort(*other.m_activePorts[i]);
		else
			m_activePorts[i] = new CDetailActivePort(); //Avoid Exception in Copy Constructor
	}
}

////////////////////////////////////////////////////////////////////////////
CRsrcDetailElement& CRsrcDetailElement::operator=(const CRsrcDetailElement& other)
{
	m_cardType = other.m_cardType;
	m_slotNumber = other.m_slotNumber;
	m_numPorts = other.m_numPorts;
	WORD i = 0;
	for (i = 0; i < m_numPorts; i++)
	{
		if (m_activePorts)
			delete m_activePorts[i];
	}
	if (m_activePorts)
		delete m_activePorts;

	m_numActivePorts = other.m_numActivePorts;
	m_activePorts = new CDetailActivePort*[m_numPorts];
	for (i = 0; i < m_numPorts; i++)
		m_activePorts[i] = new CDetailActivePort(*other.m_activePorts[i]);
	return *this;
}

////////////////////////////////////////////////////////////////////////////
CRsrcDetailElement::~CRsrcDetailElement()
{
	for (WORD i = 0; i < m_numPorts; i++)
		PDELETE(m_activePorts[i]);
	PDELETEA(m_activePorts);
}

////////////////////////////////////////////////////////////////////////////
WORD CRsrcDetailElement::getNumPort()
{
	return m_numPorts;
}

////////////////////////////////////////////////////////////////////////////
WORD CRsrcDetailElement::getNumActivePort()
{
	return m_numActivePorts;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcDetailElement::setNumPort(WORD num_port)
{
	m_numPorts = num_port;
	m_numActivePorts = num_port;
}

////////////////////////////////////////////////////////////////////////////
int CRsrcDetailElement::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CRsrcDetailElement::DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action)
{
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcDetailElement::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pUnitNode = pFatherNode->AddChildNode("UNIT_REPORT_LIST");
	pUnitNode->AddChildNode("OBJ_TOKEN", m_updateCounter);
	pUnitNode->AddChildNode("CHANGED", TRUE, _BOOL);

	CXMLDOMElement* pUnit2Node = pUnitNode->AddChildNode("UNIT_REPORT");

	if (!pUnit2Node)
		return;

	pUnit2Node->AddChildNode("SLOT_NUMBER", m_slotNumber, BOARD_ENUM);
	pUnit2Node->AddChildNode("PORTS_NUMBER", m_numPorts);

	WORD realActivePorts = 0;
	for (int i = 0; i < m_numPorts; i++)
	{
		if (m_activePorts[i])
		{
			m_activePorts[i]->SerializeXml(pUnit2Node);
			realActivePorts++;
		}
	}
	if (realActivePorts != m_numPorts)
		PASSERT(realActivePorts + 1);
}


////////////////////////////////////////////////////////////////////////////
//                        CRsrcReport
////////////////////////////////////////////////////////////////////////////
CRsrcReport::CRsrcReport()
{
	memset(m_numParties, 0, sizeof(m_numParties));
	m_availablePPM = 0;
}

////////////////////////////////////////////////////////////////////////////
CRsrcReport::CRsrcReport(const CRsrcReport& other) : CSerializeObject(other)
{
	*this = other;
}

////////////////////////////////////////////////////////////////////////////
CRsrcReport& CRsrcReport::operator=(const CRsrcReport& other)
{
	memcpy(m_numParties, other.m_numParties, sizeof(m_numParties));
	m_availablePPM = other.m_availablePPM;

	return *this;
}

////////////////////////////////////////////////////////////////////////////
CRsrcReport::~CRsrcReport()
{
}

////////////////////////////////////////////////////////////////////////////
static const char * FixedModeResourceReportNames[] = { "audio", "CIF", "SD", "HD720", "HD1080" };
static const char * AutoModeResourceReportNames[] = { "audio", "video" };
static const char * AutoModeResourceReportHDNames[] = { "audio", "HD720p30_video" };
static const char * AutoModeResourceReportNamesCOP[] = { "audio_video", "audio", "video", };

////////////////////////////////////////////////////////////////////////////
void CRsrcReport::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	// Legacy resource report: audio + video
	CXMLDOMElement* pReportNode = pFatherNode->AddChildNode("RSRC_REPORT_RMX_LIST");
	CXMLDOMElement* pNumPartiesNode;

	DWORD portGauge = pSystemResources->GetPortGauge();

	const char** reportNames;
	int numOfTypes = 0;

	if (CHelperFuncs::IsFixedModeAllocationType())
	{
		numOfTypes = NUM_OF_PARTY_RESOURCE_TYPES; //all types
		reportNames = FixedModeResourceReportNames;
	}
	else
	{
		if (CHelperFuncs::IsMode2C())
		{
			numOfTypes = 1; //audio
			reportNames = AutoModeResourceReportNamesCOP;
		}
		else
		{
			numOfTypes = 2; //audio + video
			reportNames = AutoModeResourceReportNames;
		}
	}

	eProductType productType = pSystemResources->GetProductType();
	int numItems = (CHelperFuncs::IsSoftMCU(productType) && eProductTypeNinja != productType) ? 1 : 0; //BRIDGE-2644 / 4165 (audio-only counter has no meaning in SoftMCU except for Ninja)
	for (int i = numItems; i < numOfTypes; i++)
	{
		pNumPartiesNode = pReportNode->AddChildNode("RSRC_REPORT_RMX");

		pNumPartiesNode->AddChildNode("RSRC_REPORT_ITEM", reportNames[i]);
		pNumPartiesNode->AddChildNode("TOTAL", m_numParties[i][TYPE_TOTAL]);
		pNumPartiesNode->AddChildNode("OCCUPIED", m_numParties[i][TYPE_OCCUPIED]);
		pNumPartiesNode->AddChildNode("RESERVED", m_numParties[i][TYPE_RESERVED]);
		pNumPartiesNode->AddChildNode("FREE", m_numParties[i][TYPE_FREE]);
	}

	pReportNode->AddChildNode("PORT_GAUGE_VALUE", portGauge);

	if (!CHelperFuncs::IsMode2C())
	{
		// New Resource Report in HD ports: audio + HD720p30_video
		pReportNode = pFatherNode->AddChildNode("RSRC_REPORT_RMX_LIST_HD");

		ePartyResourceTypes typesArray[NUM_OF_PARTY_RESOURCE_TYPES] = { e_Audio, e_Cif, e_SD30, e_HD720, e_HD1080p30 };

		if (CHelperFuncs::IsFixedModeAllocationType())
		{
			numOfTypes = NUM_OF_PARTY_RESOURCE_TYPES; //all types
			reportNames = FixedModeResourceReportNames;
		}
		else
		{
			numOfTypes = 2; //audio + HD720p30_video
			typesArray[1] = e_HD720; // Change only second type to HD720, because the first one remains the same
			reportNames = AutoModeResourceReportHDNames;
		}

		for (int i = numItems; i < numOfTypes; i++)
		{
			pNumPartiesNode = pReportNode->AddChildNode("RSRC_REPORT_RMX");

			pNumPartiesNode->AddChildNode("RSRC_REPORT_ITEM", reportNames[i]);
			pNumPartiesNode->AddChildNode("TOTAL", m_numParties[typesArray[i]][TYPE_TOTAL]);
			pNumPartiesNode->AddChildNode("OCCUPIED", m_numParties[typesArray[i]][TYPE_OCCUPIED]);
			pNumPartiesNode->AddChildNode("RESERVED", m_numParties[typesArray[i]][TYPE_RESERVED]);
			pNumPartiesNode->AddChildNode("FREE", m_numParties[typesArray[i]][TYPE_FREE]);
			if (e_HD720 == typesArray[i])
				pNumPartiesNode->AddChildNode("AVAILABLE_PORTION_PPM", m_availablePPM);
		}
		pReportNode->AddChildNode("PORT_GAUGE_VALUE", portGauge);
	}
}

////////////////////////////////////////////////////////////////////////////
std::string CRsrcReport::GetFloatValueAsString(const float fValue) const
{
	std::string strVal = CStlUtils::ValueToString(fValue);
	if (strVal == "nan" || strVal == "")
	{
		PTRACE(eLevelInfoNormal, "CRsrcReport::GetFloatValueAsString strVal is invalid value nan or empty!!!");
		return "NaN";
	}
	return strVal;
}

////////////////////////////////////////////////////////////////////////////
int CRsrcReport::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
	PASSERTMSG(TRUE, "CRsrcReport::DeSerializeXml - Should not be called"); //only for sending to API
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const CRsrcReport& val)
{
	CPrettyTable<const char*, WORD, WORD, WORD, WORD, WORD> tbl("Type", "Audio", "CIF", "SD30", "HD720", "HD1080");

	for (int i = TYPE_TOTAL; i < NUM_RPRT_TYPES; ++i)
		tbl.Add(to_string((eRPRTtypes)i), val.m_numParties[e_Audio][i], val.m_numParties[e_Cif][i], val.m_numParties[e_SD30][i], val.m_numParties[e_HD720][i], val.m_numParties[e_HD1080p30][i]);

	os << tbl.Get() << "\nPPM:" << val.m_availablePPM;
	return os;
}


////////////////////////////////////////////////////////////////////////////
//                        CIpServiceRsrcReport
////////////////////////////////////////////////////////////////////////////
CIpServiceRsrcReport::CIpServiceRsrcReport(const char* pServiceName, WORD service_id) :
		m_service_id(service_id)
{
	SetName(pServiceName);
}

////////////////////////////////////////////////////////////////////////////
CIpServiceRsrcReport::CIpServiceRsrcReport(const CRsrcReport& rRsrcReport, const char* pServiceName, WORD service_id) :
		CRsrcReport(rRsrcReport), m_service_id(service_id)
{
	SetName(pServiceName);
}

////////////////////////////////////////////////////////////////////////////
CIpServiceRsrcReport::CIpServiceRsrcReport(const CIpServiceRsrcReport& other) :
		CRsrcReport(other), m_service_id(other.GetServiceId())
{
	SetName(other.GetServiceName());
}

////////////////////////////////////////////////////////////////////////////
void CIpServiceRsrcReport::SetName(const char* pServiceName)
{
	strcpy_safe(m_serviceName, pServiceName);
}

////////////////////////////////////////////////////////////////////////////
CIpServiceRsrcReport::~CIpServiceRsrcReport()
{
}

////////////////////////////////////////////////////////////////////////////
void CIpServiceRsrcReport::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pReportNode = pFatherNode->AddChildNode("RSRC_SERVICES_REPORT");
	CRsrcReport::SerializeXml(pReportNode);
	pReportNode->AddChildNode("SERVICE_NAME", m_serviceName);
}

////////////////////////////////////////////////////////////////////////////
int CIpServiceRsrcReport::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
	PASSERTMSG(TRUE, "CIpServiceRsrcReport::DeSerializeXml - Should not be called"); //only for sending to API
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
bool operator==(const CIpServiceRsrcReport& ser1, const CIpServiceRsrcReport& ser2)
{
	return (ser1.GetServiceId() == ser2.GetServiceId());
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CIpServiceRsrcReport& ser1, const CIpServiceRsrcReport& ser2)
{
	return (ser1.GetServiceId() < ser2.GetServiceId());
}


////////////////////////////////////////////////////////////////////////////
//                        CServicesRsrcReport
////////////////////////////////////////////////////////////////////////////
CServicesRsrcReport::CServicesRsrcReport()
{
	m_pIpServicesReports = new std::set<CIpServiceRsrcReport>;
}

////////////////////////////////////////////////////////////////////////////
CServicesRsrcReport::CServicesRsrcReport(const CServicesRsrcReport& other) : CSerializeObject(other)
{
	m_pIpServicesReports = new std::set<CIpServiceRsrcReport>;
	other.Copy(*this);
}

////////////////////////////////////////////////////////////////////////////
CServicesRsrcReport::~CServicesRsrcReport()
{
	m_pIpServicesReports->clear();
	PDELETE(m_pIpServicesReports);
}

////////////////////////////////////////////////////////////////////////////
void CServicesRsrcReport::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	WORD num_of_services = m_pIpServicesReports->size();
	TRACEINTO << "NumIpServices:" << num_of_services;
	if (num_of_services == 0)
	{
		PTRACE(eLevelInfoNormal, "CServicesRsrcReport::SerializeXml, num_of_services = 0, do nothing");
		return;
	}

	CXMLDOMElement* pReportNode = pFatherNode->AddChildNode("RSRC_SERVICES_REPORT_RMX_LIST");
	WORD isConfigured = FALSE;
	std::set<CIpServiceRsrcReport>::iterator ser_itr;
	for (ser_itr = m_pIpServicesReports->begin(); ser_itr != m_pIpServicesReports->end(); ser_itr++)
	{
		ser_itr->SerializeXml(pReportNode);
	}
}

////////////////////////////////////////////////////////////////////////////
int CServicesRsrcReport::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
	PASSERTMSG(TRUE, "CServicesRsrcReport::DeSerializeXml - Should not be called"); //only for sending to API
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CServicesRsrcReport::AddIpServiceReport(const CIpServiceRsrcReport& ipServiceRsrcReport)
{
	m_pIpServicesReports->insert(ipServiceRsrcReport);
}

////////////////////////////////////////////////////////////////////////////
WORD CServicesRsrcReport::GetNumOfIpServiceReport() const
{
	return m_pIpServicesReports->size();
}

////////////////////////////////////////////////////////////////////////////
void CServicesRsrcReport::Copy(CServicesRsrcReport& target) const
{
	std::set<CIpServiceRsrcReport>::iterator _itr, _end = m_pIpServicesReports->end();
	for (_itr = m_pIpServicesReports->begin(); _itr != _end; ++_itr)
		target.AddIpServiceReport(*_itr);
}
