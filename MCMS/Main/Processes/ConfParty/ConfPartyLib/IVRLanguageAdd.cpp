// IVRServiceDel.cpp: implementation of the CIVRServiceDel class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Delete XML IVR Service
//========   ==============   =====================================================================

#include "NStream.h"
#include "IVRLanguageAdd.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CIVRLanguageAdd::CIVRLanguageAdd()
{
	m_ivrLanguageName[0] = '\0';
}
/////////////////////////////////////////////////////////////////////////////
CIVRLanguageAdd& CIVRLanguageAdd::operator = (const CIVRLanguageAdd &other)
{
	strncpy( m_ivrLanguageName, other.m_ivrLanguageName, LANGUAGE_NAME_LEN );
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CIVRLanguageAdd::~CIVRLanguageAdd()
{
}


///////////////////////////////////////////////////////////////////////////
void CIVRLanguageAdd::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CIVRLanguageAdd::DeSerializeXml(CXMLDOMElement* pIVRLanguageNode, char* pszError, const char * strAction)
{
	int nStatus = STATUS_OK;
	
	m_NumAction = NEW_IVR_LANGUAGE;
	GET_VALIDATE_CHILD(pIVRLanguageNode, "LANGUAGE_NAME", m_ivrLanguageName, _1_TO_LANGUAGE_NAME_LENGTH);
	
	return nStatus;
}

//////////////////////////////////////////////////////////////////////////
const char* CIVRLanguageAdd::GetIVRLanguageName()
{
	return m_ivrLanguageName;
}

//////////////////////////////////////////////////////////////////////////
void CIVRLanguageAdd::SetIVRLanguageName(char* langName)
{
	strncpy(m_ivrLanguageName, langName, sizeof(m_ivrLanguageName) - 1);
	m_ivrLanguageName[sizeof(m_ivrLanguageName) - 1] = '\0';
}




