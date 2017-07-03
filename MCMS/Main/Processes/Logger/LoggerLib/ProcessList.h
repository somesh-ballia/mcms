#ifndef PROCESSLIST_H_
#define PROCESSLIST_H_


#include <vector>
#include <string>
#include "ProcessItemData.h"

using namespace std;
class CProcessList : public CSerializeObject
{
CLASS_TYPE_1(CProcessList,CSerializeObject)
public:

    //Constructors
    
	CProcessList();                                                                            
	CProcessList(const CProcessList &other);
	CProcessList& operator = (const CProcessList& other);
	virtual ~CProcessList();
	virtual CSerializeObject* Clone() { return new CProcessList(); }
	virtual void   SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);

	const char*  NameOf() const { return "CProcessList"; }

	//void							SetProcessInfo(vector<CProcessItemData*> processInfo);
	vector<CProcessItemData*>&	GetProcessInfo();

	void						AddProcessItem(CProcessItemData* processItem);
	bool						IsSpecifiedProcessChecked(int SpecifiedProcessInd, const char* moduleName,int src_id);
	void 						CopyValue(CProcessList* pOther);
	CProcessItemData*			GetProcessItemByProcessIndex(int processIndex);
	
protected:
	
	vector<CProcessItemData*>	m_processItemDataVec;
  	
};

/////////////////////////////////////////////////////////////////////////
#endif 



