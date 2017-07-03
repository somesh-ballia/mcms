#ifndef MODULECONTENT_H_
#define MODULECONTENT_H_

#include "ProcessList.h"

#define			MAX_LOGLEVEL_LENGHT 		32
#define			MAX_MODULE_NAME_LENGHT 		32

class CModuleContent : public CSerializeObject
{
CLASS_TYPE_1(CModuleContent,CSerializeObject)
public:

    //Constructors
    
	CModuleContent();                                                                            
	CModuleContent(const CModuleContent &other);
	CModuleContent& operator = (const CModuleContent& other);
	virtual ~CModuleContent();
	virtual CSerializeObject* Clone() {return new CModuleContent();}  
	virtual void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int    DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);

	const char*  NameOf() const {return "CModuleContent";}  

	void						SetEnabled(BYTE bEnabled);
	void						SetLogLevel(DWORD level);
	void						SetLModuleName(char* name);
	void						SetProcessList(CProcessList processList);

	char*					GetModuleName();
	DWORD					GetLogLevel();
	BYTE					IsEnabled();
	CProcessList&				GetProcessList();
	bool 					ValidateLogMsg(int IndexInAllRmxProcesses,int log4cxx_level_of_the_trace,int src_id);
	void						CopyValue(CModuleContent* pOther);
protected:

	CProcessList				m_processList;
	BYTE					m_isEnabled;
	DWORD					m_traceLevel;
	char						m_moduleName[MAX_MODULE_NAME_LENGHT];
  
};

/////////////////////////////////////////////////////////////////////////
#endif /*MODULECONTENT_H_*/



