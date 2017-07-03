// SysConfigGet.h: implementation of the CSysConfigGet class.
//
//
//Date         Updated By         Description
//
//27/10/05	  Yuri Ratner		Used in XML transaction. 
//========   ==============   =====================================================================



#include "SysConfigGet.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
CSysConfigGet::CSysConfigGet()
{
	m_FileType = eCfgParamUser;
}

//////////////////////////////////////////////////////////////////////
CSysConfigGet::~CSysConfigGet()
{
	
}

//////////////////////////////////////////////////////////////////////
void CSysConfigGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	PASSERTMSG(1, "this function should not be called");
}

//////////////////////////////////////////////////////////////////////
int CSysConfigGet::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;
	
	BYTE tmp = (BYTE)m_FileType;
	GET_VALIDATE_CHILD(pActionNode, "CFG_TYPE", &tmp, CFG_TYPE_ENUM);
	m_FileType = (eCfgParamType)tmp;
	
	return nStatus;
}
