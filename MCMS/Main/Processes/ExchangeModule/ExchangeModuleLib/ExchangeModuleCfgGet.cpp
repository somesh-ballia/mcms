// CMcuStateGet.cpp: implementation of the CMcuStateGet class.
//////////////////////////////////////////////////////////////////////////


#include "ExchangeModuleCfgGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "ExchangeModuleCfg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CExchangeModuleCfgGet::CExchangeModuleCfgGet()
{
	m_pProcess = (CExchangeModuleProcess*)CExchangeModuleProcess::GetProcess();	
	m_updateCounter = 0;
	m_bIsForEMA = false;
}

/////////////////////////////////////////////////////////////////////////////
CExchangeModuleCfgGet& CExchangeModuleCfgGet::operator = (const CExchangeModuleCfgGet &other)
{
	if (&other == this)
		return *this;

	m_pProcess = (CExchangeModuleProcess*)CExchangeModuleProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
	m_bIsForEMA = other.m_bIsForEMA;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CExchangeModuleCfgGet::~CExchangeModuleCfgGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CExchangeModuleCfgGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CExchangeModuleCfg exchangeCfg;
	m_pProcess->GetMcuExchangeCfg(exchangeCfg);
	exchangeCfg.SerializeXml(pFatherNode,m_bIsForEMA);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CExchangeModuleCfgGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;	
	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CExchangeModuleCfgGet::SetIsForEMA(BYTE yesNo)
{
	m_bIsForEMA = yesNo;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CExchangeModuleCfgGet::GetIsForEMA()
{
	return m_bIsForEMA;
}

