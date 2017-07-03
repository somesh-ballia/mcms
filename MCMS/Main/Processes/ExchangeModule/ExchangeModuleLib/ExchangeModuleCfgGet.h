// CMcuTimeGet.h: interface for the CMcuStateGet class.
//
//////////////////////////////////////////////////////////////////////

#ifndef EXCHANGE_MODULE_CONFIGURATION_GET_H_
#define EXCHANGE_MODULE_CONFIGURATION_GET_H_






#include "SerializeObject.h"
#include "ExchangeModuleProcess.h"
#include "psosxml.h"



class CExchangeModuleCfgGet : public CSerializeObject
{
CLASS_TYPE_1(CExchangeModuleCfgGet, CSerializeObject)
public:

	//Constructors
	CExchangeModuleCfgGet();
	CExchangeModuleCfgGet(const CExchangeModuleCfgGet &other);
	CExchangeModuleCfgGet& operator = (const CExchangeModuleCfgGet& other);
	virtual ~CExchangeModuleCfgGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CExchangeModuleCfgGet();}
	const char * NameOf() const {return "CMcuStateGet";}
	void SetIsForEMA(BYTE yesNo);
	BYTE GetIsForEMA();

	
protected:
	CExchangeModuleProcess* m_pProcess;
	BYTE m_bIsForEMA;
};












#endif /*MCUSTATEGET_H_*/
