// SysConfigSet.h: implementation of the CSysConfigSet class.
//
//
//Date         Updated By         Description
//
//27/10/05	  Yuri Ratner		Used in XML transaction. 
//========   ==============   =====================================================================


#include "SysConfigSet.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"


CSysConfigSet::CSysConfigSet()
{
	m_FileType 	= eCfgParamUser;
	m_SysConfig = new CSysConfigEma;
}

CSysConfigSet::~CSysConfigSet()
{
	PDELETE(m_SysConfig);
}

//////////////////////////////////////////////////////////////////////
void CSysConfigSet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	PASSERTMSG(1, "this function should not be called");
}

//////////////////////////////////////////////////////////////////////
int CSysConfigSet::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;
	
	BYTE tmp = (BYTE)m_FileType;
	GET_VALIDATE_CHILD(pActionNode, "CFG_TYPE", &tmp, CFG_TYPE_ENUM);
	m_FileType = (eCfgParamType)tmp;
	
	CXMLDOMElement *cfgNode;
	GET_CHILD_NODE(pActionNode, "SYSTEM_CFG", cfgNode);
	m_SysConfig->SetCfgParamTypeState(m_FileType);
	nStatus = m_SysConfig->DeSerializeXml(cfgNode, pszError, action);
		
	return nStatus;
}
