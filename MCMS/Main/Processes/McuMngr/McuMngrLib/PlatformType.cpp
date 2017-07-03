// PlatformType.cpp: implementation of the CPlatformType class.
//
//////////////////////////////////////////////////////////////////////


#include "PlatformType.h"
#include "StatusesGeneral.h"
//#include "Macros.h"
#include "InitCommonStrings.h"
//#include "McuMngrDefines.h"
//#include "StringsMaps.h"
#include "ApiStatuses.h"


// ------------------------------------------------------------
CPlatformType::CPlatformType ()
{
	m_platformType = eGideonLite;
}

// ------------------------------------------------------------
CPlatformType::CPlatformType (const CPlatformType& other)
:CPObject(other)
{
	m_platformType = other.m_platformType;
}

// ------------------------------------------------------------
CPlatformType::~CPlatformType ()
{
}

// ------------------------------------------------------------
CPlatformType& CPlatformType::operator = (const CPlatformType &rOther)
{
	m_platformType = rOther.m_platformType;
    
	return *this;
}

// ------------------------------------------------------------
void CPlatformType::SerializeXml(CXMLDOMElement* pParentNode)
{
	pParentNode->AddChildNode("PLATFORM_TYPE", (WORD)m_platformType);
	
	return;
}

// ------------------------------------------------------------
int	CPlatformType::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	WORD tmp = 0;
	GET_VALIDATE_CHILD(pActionNode, "PLATFORM_TYPE", &tmp, _0_TO_WORD);
	m_platformType = (ePlatformType)tmp;

	return STATUS_OK;
}

// ------------------------------------------------------------
ePlatformType CPlatformType::GetPlatformType() const
{
	return m_platformType;
}

// ------------------------------------------------------------
void CPlatformType::SetPlatformType(const ePlatformType theType)
{
	m_platformType = theType;
}
