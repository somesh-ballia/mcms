// IVRServiceConvertSlide.h: interface for the CIVRServiceConvertSlide class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Huizhao Sun 	  Class for Convert Slide XML IVR Service
//========   ==============   =====================================================================


#if !defined(_IVRSERVICECONVERTSLIDE_H__)
#define _IVRSERVICECONVERTSLIDE_H__

#include "SerializeObject.h"
#include "IVRService.h"



class CIVRServiceConvertSlide : public CSerializeObject
{
CLASS_TYPE_1(CIVRServiceConvertSlide, CSerializeObject)
public:

	//Constructors
	CIVRServiceConvertSlide();
	virtual const char* NameOf() const { return "CIVRServiceConvertSlide";}
    CIVRServiceConvertSlide(const CIVRServiceConvertSlide &other);
	CIVRServiceConvertSlide& operator = (const CIVRServiceConvertSlide& other);
	virtual ~CIVRServiceConvertSlide();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CIVRServiceConvertSlide;}
    
	const char * GetSlideName();
	eIvrSlideConversionMethod GetConversionMethod();
    eIvrSlideImageType GetImageType();
  
protected:

	char 	                        m_slideName[H243_NAME_LEN];
    eIvrSlideConversionMethod       m_conversionMethod;
    eIvrSlideImageType              m_imageType;

};

#endif // !defined(_IVRSERVICECONVERTSLIDE_H__)

