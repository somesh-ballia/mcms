// SysConfigSet.h: interface for the CSysConfigSet class.
//
//
//Date         Updated By         Description
//
//27/10/05	  Yuri Ratner		Used in XML transaction. 
//========   ==============   =====================================================================



#ifndef SYSCONFIGSET_H_
#define SYSCONFIGSET_H_

#include "SerializeObject.h"
#include "SysConfigEma.h"


class CSysConfigSet : public CSerializeObject
{
public:
	CSysConfigSet();
	virtual ~CSysConfigSet();
	virtual CSerializeObject* Clone(){return new CSysConfigSet;}
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual const char* NameOf() const { return "CSysConfigSet";}
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	
	eCfgParamType GetFileType()		{return m_FileType;}
	CSysConfigEma* GetSystemCfg()	{return m_SysConfig;}
	
private:
	// disabled
	CSysConfigSet(const CSysConfigSet&);
	CSysConfigSet& operator=(const CSysConfigSet&);
	
	eCfgParamType   	m_FileType;
	CSysConfigEma 		*m_SysConfig;
};

#endif /*SYSCONFIGSET_H_*/
