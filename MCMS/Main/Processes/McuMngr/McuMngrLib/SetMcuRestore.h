#ifndef SETMCURESTORE_H_
#define SETMCURESTORE_H_


#include "SerializeObject.h"
#include "McuMngrStructs.h"
#include "McuMngrDefines.h"

static char *McuRestoreNames [] = 
{
	"none",
	"standard",
	"extensive"		,
	"basic"
};
static const char * GetMcuRestoreName(eMcuRestoreType type)
{
	const char *name = (eMcuRestoreNone <= type && type < NumOfMcuRestoreTypes
						?
						McuRestoreNames[type] : "Invalid");
	return name;
}







class CSetMcuRestore : public CSerializeObject
{
public:
	CSetMcuRestore();
	virtual ~CSetMcuRestore();
	
	virtual CSerializeObject* Clone(){return new CSetMcuRestore;}
	virtual const char* NameOf() const { return "CSetMcuRestore";}
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int	 DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action);
	
	eMcuRestoreType GetMcuRestoreType()const{return m_McuRestoreType;}
	
	
	
	
private:
	// disabled
	CSetMcuRestore(const CSetMcuRestore&);
	CSetMcuRestore&operator=(const CSetMcuRestore&);
	
	
	eMcuRestoreType m_McuRestoreType;
};




#endif /*SETMCURESTORE_H_*/
