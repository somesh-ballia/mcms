////////////////////////////////////////////////////////////////////////////////////////
//
//   Date         Created By         Description
//
//  11/7/2005	    ERAN		wrapper class for Delete-IPService	   
//  ========   ==============   ========================================================
///////////////////////////////////////////////////////////////////////////////////////


#include "CIPServiceDel.h"
#include "CSMngrProcess.h"
#include <string>
#include "StatusesGeneral.h"




//////////////////////////////////////////////////////////////////////////////
CIPServiceDel::CIPServiceDel()
{
}
/////////////////////////////////////////////////////////////////////////////
CIPServiceDel::~CIPServiceDel()
{

}
/////////////////////////////////////////////////////////////////////////////
CIPServiceDel::CIPServiceDel( const CIPServiceDel &other )
:CSerializeObject(other)
{
	m_IPServiceName = other.m_IPServiceName;
}

///////////////////////////////////////////////////////////////////////////
void CIPServiceDel::SerializeXml(CXMLDOMElement*& thisNode) const
{
		
}
/////////////////////////////////////////////////////////////////////////////
int CIPServiceDel::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	char buff[NET_SERVICE_PROVIDER_NAME_LENGTH];
	memset( buff, 0, NET_SERVICE_PROVIDER_NAME_LENGTH);
	
	GET_VALIDATE_CHILD(pNode,"NAME", buff,NET_SERVICE_PROVIDER_NAME_LENGTH);
	m_IPServiceName = buff;
	
	return nStatus;
	
}
///////////////////////////////////////////////////////////////////////////////////////////////