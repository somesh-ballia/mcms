// SysConfigGet.h: interface for the CSysConfigGet class.
//
//
//Date         Updated By         Description
//
//27/10/05	  Yuri Ratner		Used in XML transaction. 
//========   ==============   =====================================================================



#ifndef SYSCONFIGGET_H_
#define SYSCONFIGGET_H_

#include "SerializeObject.h"
#include "SysConfigEma.h"


class CSysConfigGet : public CSerializeObject
{
public:
	CSysConfigGet();
	virtual ~CSysConfigGet();
	virtual CSerializeObject* Clone(){return new CSysConfigGet;}
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual const char* NameOf() const { return "CSysConfigGet";}
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	eCfgParamType GetFileType() {return m_FileType;}
	
private:
	// disabled
	CSysConfigGet(const CSysConfigGet&);
	CSysConfigGet& operator=(const CSysConfigGet&);
	
	eCfgParamType m_FileType;
};

#endif /*SYSCONFIGGET_H_*/
