#ifndef __RESTORE_CFG_CPP_
#define __RESTORE_CFG_CPP_

#include "RestoreCfg.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"

CRestoreCfg::CRestoreCfg()
{
	m_fileName="";
}

void CRestoreCfg::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	PASSERTMSG(1, "this function should not be called");
}

int  CRestoreCfg::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	TRACEINTO << "CRestoreCfg::DeSerializeXml";
	STATUS nStatus = STATUS_OK;
    
    GET_VALIDATE_CHILD(pActionNode, "RESTORE_CONFIG_FILE", m_fileName, ONE_LINE_BUFFER_LENGTH);
		
	return nStatus;
}
	
const char* CRestoreCfg::GetFileName() 
{
	return m_fileName.GetString();
}

#endif /*__RESTORE_CFG_CPP_*/
