// IVRServiceDel.cpp: implementation of the CIVRServiceDel class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Delete XML IVR Service
//========   ==============   =====================================================================

#include "NStream.h"
#include "IVRServiceDel.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CIVRServiceDel::CIVRServiceDel()
{
	m_ivrServiceName[0] = '\0';
}
/////////////////////////////////////////////////////////////////////////////
CIVRServiceDel& CIVRServiceDel::operator = (const CIVRServiceDel &other)
{
	strncpy( m_ivrServiceName, other.m_ivrServiceName, H243_NAME_LEN );
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CIVRServiceDel::~CIVRServiceDel()
{
}


///////////////////////////////////////////////////////////////////////////
void CIVRServiceDel::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CIVRServiceDel::DeSerializeXml(CXMLDOMElement* pIVRServiceNode, char* pszError, const char * strAction)
{
	int nStatus = STATUS_OK;
	
	m_NumAction = DEL_AV_MSG;
	GET_VALIDATE_CHILD(pIVRServiceNode, "SERVICE_NAME", m_ivrServiceName, _1_TO_AV_MSG_SERVICE_NAME_LENGTH);
	return nStatus;
}

//////////////////////////////////////////////////////////////////////////
const char* CIVRServiceDel::GetIVRServiceName()
{
	return m_ivrServiceName;
}



