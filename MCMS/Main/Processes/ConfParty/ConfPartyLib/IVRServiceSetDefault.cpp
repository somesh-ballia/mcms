// IVRServiceSetDefault.cpp: implementation of the CIVRServiceSetDefault class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Setting Default IVR Service 
//========   ==============   =====================================================================

#include "NStream.h"
#include "IVRServiceSetDefault.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CIVRServiceSetDefault::CIVRServiceSetDefault()
{
	m_ivrServiceName[0] = '\0';
}
/////////////////////////////////////////////////////////////////////////////
CIVRServiceSetDefault& CIVRServiceSetDefault::operator = (const CIVRServiceSetDefault &other)
{
	strncpy( m_ivrServiceName, other.m_ivrServiceName, H243_NAME_LEN );
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CIVRServiceSetDefault::~CIVRServiceSetDefault()
{
}


///////////////////////////////////////////////////////////////////////////
void CIVRServiceSetDefault::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CIVRServiceSetDefault::DeSerializeXml(CXMLDOMElement* pIVRServiceNode, char* pszError, const char * strAction)
{
	int nStatus = STATUS_OK;
	
	m_NumAction = SET_DEFAULT_AV_MSG_SERVICE;
	GET_VALIDATE_CHILD(pIVRServiceNode, "SERVICE_NAME", m_ivrServiceName, _1_TO_AV_MSG_SERVICE_NAME_LENGTH);
	return nStatus;
}

//////////////////////////////////////////////////////////////////////////
const char* CIVRServiceSetDefault::GetIVRServiceName()
{
	return m_ivrServiceName;
}



