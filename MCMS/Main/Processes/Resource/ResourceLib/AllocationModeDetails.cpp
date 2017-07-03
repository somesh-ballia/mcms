#include "AllocationModeDetails.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "ObjString.h"
#include "StringsMaps.h"
#include "HelperFuncs.h"
#include "TraceStream.h"
#include "ProcessSettings.h"

#define ALLOCATION_MODE_STRING "allocation_mode"
static const char * AllocationModeNames[] =
{
	"none",
	"auto",
	"fixed",
};

////////////////////////////////////////////////////////////////////////////
//                        CAllocationModeDetails
////////////////////////////////////////////////////////////////////////////
CAllocationModeDetails::CAllocationModeDetails()
{
	m_Mode = m_FutureMode = eAllocationModeNone;
}

////////////////////////////////////////////////////////////////////////////
CAllocationModeDetails::~CAllocationModeDetails()
{
}

////////////////////////////////////////////////////////////////////////////
void CAllocationModeDetails::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pAllocationNode = pFatherNode->AddChildNode("ALLOCATION_MODE");
	pAllocationNode->AddChildNode("ALLOCATION_MODE_CURRENT", m_Mode, ALLOCATION_MODE_ENUM);
	pAllocationNode->AddChildNode("ALLOCATION_MODE_FUTURE", m_FutureMode, ALLOCATION_MODE_ENUM);
}

////////////////////////////////////////////////////////////////////////////
int CAllocationModeDetails::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;
	CXMLDOMElement *pChildNode = NULL;

	DWORD temp = LAST_ALLOCATION_MODE_TYPE;
	GET_VALIDATE_CHILD(pActionNode, "SELECTED_ALLOCATION_MODE", &temp, ALLOCATION_MODE_ENUM);
	if (temp < LAST_ALLOCATION_MODE_TYPE)
		m_Mode = (eAllocationModeType)temp;
	else
		TRACEINTO << "AllocationMode:" << temp << " - Illegal allocation mode";

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CAllocationModeDetails::SetModes(eAllocationModeType mode, eAllocationModeType futureMode)
{
	m_Mode = mode;
	m_FutureMode = futureMode;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationModeDetails::ReadFromProcessSetting()
{
	m_Mode = eAllocationModeAuto;
	CProcessSettings *pProcessSettings = CHelperFuncs::GetProcessSettings();
	PASSERT_AND_RETURN_VALUE(pProcessSettings == NULL, STATUS_FAIL);

	string temp;
	if (pProcessSettings->GetSetting(ALLOCATION_MODE_STRING, temp))
	{
		BOOL bFound = FALSE;
		for (int i = 0; i < LAST_ALLOCATION_MODE_TYPE; i++)
		{
			if (temp == AllocationModeNames[i])
			{
				m_Mode = (eAllocationModeType)i;

				bFound = TRUE;
				if (CHelperFuncs::IsMode2C())
				{
					m_Mode = eAllocationModeAuto;
					TRACEINTO << "AllocationMode:auto - Setting default";
					bFound = FALSE;
				}

				// Since V8.1 Fixed Mode is not supported.
#if USE_FLIXIBILE_RESOURCE_CAPACITY_ONLY
				if (m_Mode == eAllocationModeFixed)
				{
					m_Mode = eAllocationModeAuto;
					TRACEINTO << "AllocationMode:auto - Since V8.1 'fixed' is not supported";
					bFound = FALSE;
				}
#endif
				break;
			}
		}
		if (bFound == FALSE)
		{
			TRACEINTO << "AllocationMode:" << temp << " - Illegal value";
			WriteToProcessSetting();
		}
		else
			TRACEINTO << "AllocationMode:" << temp << ", Mode:" << m_Mode;
	}
	else
	{
		TRACEINTO << "No allocation mode in file. Setting default of " << m_Mode << ", AllocationModeNames[m_Mode]:" << AllocationModeNames[m_Mode];
		WriteToProcessSetting();
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationModeDetails::WriteToProcessSetting()
{
	CProcessSettings *pProcessSettings = CHelperFuncs::GetProcessSettings();
	PASSERT_AND_RETURN_VALUE(pProcessSettings == NULL, STATUS_FAIL);
	PASSERT_AND_RETURN_VALUE(LAST_ALLOCATION_MODE_TYPE == m_Mode, STATUS_FAIL);

	if (m_Mode > eAllocationModeNone && m_Mode < eAllocationModeNone)
		pProcessSettings->SetSetting(ALLOCATION_MODE_STRING, AllocationModeNames[m_Mode]);

	return STATUS_OK;
}
