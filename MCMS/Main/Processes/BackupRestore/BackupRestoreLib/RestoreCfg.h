#ifndef __RESTORE_CFG_H_
#define __RESTORE_CFG_H_

// RestoreCfg.h: interface for the CRestoreCfg class.
//            Used when Restore (for configuration) is called. 
//
//
//Date         Updated By         Description
//
//24/12/07	  Hillel K			Used in XML transaction. 
//========   ==============   =====================================================================



#include "SerializeObject.h"
#include "ObjString.h"

class CRestoreCfg : public CSerializeObject
{
public:
	CRestoreCfg();
	virtual ~CRestoreCfg(){}
	virtual const char*  NameOf() const {return "CRestoreCfg";}
	virtual CSerializeObject* Clone(){return new CRestoreCfg;}
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	const char* GetFileName();
   
private:
	CSmallString m_fileName;
	
};

#endif /*__RESTORE_CFG_H_*/
