// EthernetSettingsConfigListGet.h: interface for the CEthernetSettingsConfigListGet class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ETHERNETSETTINGSCONFIGLIST_GET_H_
#define ETHERNETSETTINGSCONFIGLIST_GET_H_




#include "SerializeObject.h"
class CMcuMngrProcess;


class CEthernetSettingsConfigListGet : public CSerializeObject
{
CLASS_TYPE_1(CEthernetSettingsConfigListGet, CSerializeObject)
public:

	//Constructors
	CEthernetSettingsConfigListGet();
	CEthernetSettingsConfigListGet(const CEthernetSettingsConfigListGet &other);
	virtual ~CEthernetSettingsConfigListGet();
	CEthernetSettingsConfigListGet& operator = (const CEthernetSettingsConfigListGet& other);

	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CEthernetSettingsConfigListGet();}

	const char * NameOf() const {return "CEthernetSettingsConfigListGet";}
	
  
protected:
	CMcuMngrProcess* m_pProcess;
};




#endif /*ETHERNETSETTINGSCONFIGLIST_GET_H_*/
