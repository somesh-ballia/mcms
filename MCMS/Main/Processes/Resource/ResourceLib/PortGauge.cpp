#include "PortGauge.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "ObjString.h"
#include "StringsMaps.h"
#include "HelperFuncs.h"
#include "TraceStream.h"

CPortGauge::CPortGauge()
{
	m_portGauge = 80;
}

///////////////////////////////////////////////////////////////////////
CPortGauge::~CPortGauge()
{
}

///////////////////////////////////////////////////////////////////////
void CPortGauge::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("PORT_GAUGE_VALUE", m_portGauge);

	TRACEINTO << "CPortGauge:" << m_portGauge;
}

///////////////////////////////////////////////////////////////////////
int CPortGauge::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "PORT_GAUGE_VALUE", &m_portGauge, _0_TO_DWORD);

	TRACEINTO << "CPortGauge:" << m_portGauge;

	return STATUS_OK;
}
