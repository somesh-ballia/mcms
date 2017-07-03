// PlatformType.h: interface for the CPlatformType class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PLATFORMTYPE_H_
#define PLATFORMTYPE_H_


#include "PObject.h"
#include "psosxml.h"
#include "McuMngrStructs.h"



class CPlatformType : public CPObject
{
CLASS_TYPE_1(CPlatformType, CPObject)

public:
	CPlatformType ();
	CPlatformType (const CPlatformType& other);
	virtual ~CPlatformType ();
	const char* NameOf() const {return "CPlatformType";}
	CPlatformType& operator = (const CPlatformType &rOther);

  	void  SerializeXml(CXMLDOMElement* pParentNode);
	int	  DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError);


	ePlatformType	GetPlatformType() const;
	void			SetPlatformType(const ePlatformType theType);

    
protected:
	ePlatformType	m_platformType;

};





#endif /*PLATFORMTYPE_H_*/
