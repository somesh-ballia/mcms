//etTurnSsh.h: interface for the CSetTurnSsh class.
//
//
//Date         Updated By         Description
//
//26/12/06	  Yuri Ratner		Used in XML transaction. 
//========   ==============   =====================================================================



#include "SetTurnSsh.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"


CSetTurnSsh::CSetTurnSsh()
{
    m_IsSshOn = FALSE;
}

CSetTurnSsh::~CSetTurnSsh()
{

}

//////////////////////////////////////////////////////////////////////
void CSetTurnSsh::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	PASSERTMSG(1, "this function should not be called");
}

//////////////////////////////////////////////////////////////////////
int CSetTurnSsh::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;
    
    BOOL isSshOn = FALSE;
    GET_VALIDATE_CHILD(pActionNode, "ON", &isSshOn, _BOOL);
	m_IsSshOn = (TRUE == isSshOn);
	
	return nStatus;
}
