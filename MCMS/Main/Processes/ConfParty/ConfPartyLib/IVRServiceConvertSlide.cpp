// IVRServiceConvertSlide.cpp: implementation of the CIVRServiceConvertSlide class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Huizhao	Sun		  Class for Convert Slide XML IVR Service
//========   ==============   =====================================================================

#include "NStream.h"
#include "IVRServiceConvertSlide.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CIVRServiceConvertSlide::CIVRServiceConvertSlide()
{
	m_slideName[0] = '\0';
}


/////////////////////////////////////////////////////////////////////////////
CIVRServiceConvertSlide& CIVRServiceConvertSlide::operator = (const CIVRServiceConvertSlide &other)
{
	strncpy( m_slideName, other.m_slideName, H243_NAME_LEN );
    m_conversionMethod = other.m_conversionMethod;
    m_imageType = other.m_imageType;
    
	return *this;
}


/////////////////////////////////////////////////////////////////////////////
CIVRServiceConvertSlide::~CIVRServiceConvertSlide()
{
}


///////////////////////////////////////////////////////////////////////////
void CIVRServiceConvertSlide::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CIVRServiceConvertSlide::DeSerializeXml(CXMLDOMElement* pIVRServiceNode, char* pszError, const char * strAction)
{
	int nStatus = STATUS_OK;
	BYTE value = 0;
    
	GET_VALIDATE_CHILD(pIVRServiceNode, "SLIDE_NAME", m_slideName, _1_TO_AV_MSG_SERVICE_NAME_LENGTH);
    GET_VALIDATE_CHILD(pIVRServiceNode, "CONVERSION_METHOD", &value, IVR_SLIDE_CONVERSION_METHOD_ENUM);
    m_conversionMethod = static_cast<eIvrSlideConversionMethod>(value);
    GET_VALIDATE_CHILD(pIVRServiceNode, "IMAGE_TYPE", &value, IVR_SLIDE_IMAGE_TYPE_ENUM);
    m_imageType = static_cast<eIvrSlideImageType>(value);
    
	return nStatus;
}

//////////////////////////////////////////////////////////////////////////
const char* CIVRServiceConvertSlide::GetSlideName()
{
	return m_slideName;
}

//////////////////////////////////////////////////////////////////////////
eIvrSlideConversionMethod CIVRServiceConvertSlide::GetConversionMethod()
{
    return m_conversionMethod;
}

//////////////////////////////////////////////////////////////////////////
eIvrSlideImageType CIVRServiceConvertSlide::GetImageType()
{
    return m_imageType;
}

