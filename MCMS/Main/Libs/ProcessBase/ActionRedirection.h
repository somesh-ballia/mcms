// XmlMiniParser.h: interface for the CXmlMiniParser class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(_ActionRedirection_H__)
#define _ActionRedirection_H__

#include <string>
#include <map>

#define ACTION_REDIRECTION_F ((std::string)(MCU_MCMS_DIR + "/StaticCfg/ActionRedirection.xml"))

using namespace std;

#include "McmsProcesses.h"
#include "SerializeObject.h"
#include "ProcessBase.h"


class CStringToProcessEntry : public CPObject
{
	CLASS_TYPE_1(CStringToProcessEntry, CPObject)
public:
	CStringToProcessEntry(eProcessType proc, bool isAud, const char* name, const char* desc, const char* faultDesc);
	CStringToProcessEntry(const CStringToProcessEntry& other);
	virtual ~CStringToProcessEntry();
	virtual const char* NameOf() const {return "CStringToProcessEntry"; }

	CStringToProcessEntry& operator = (const CStringToProcessEntry& other);

	eProcessType GetProcessType();
	bool         GetIsAudit();
	char*        GetTransName();
	char*        GetTransDesc();
	char*        GetTransFailureDesc();

	void SetProcessType(const eProcessType process_type);

private:
	eProcessType m_processType;
	bool         m_isAudit;
	char       * m_transName;
	char       * m_transDesc;
	char       * m_transFailureDesc;
};

typedef std::map<std::string, CStringToProcessEntry*> CStringToProcessTypeMap;

class CActionRedirectionMap : public CSerializeObject
{
	CLASS_TYPE_1(CActionRedirectionMap, CSerializeObject)

	DISALLOW_COPY_AND_ASSIGN(CActionRedirectionMap);

public:
	CActionRedirectionMap();
	virtual ~CActionRedirectionMap();
	virtual const char*       NameOf() const {return "CActionRedirectionMap"; }
	virtual CSerializeObject* Clone()        {return new CActionRedirectionMap; }

	virtual void             SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int              DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	bool                     LoadFromFile();
	bool                     IsValidTransName(char* pszSearchString, char** pTransName);
	void                     DumpActionRedirectionMap(std::ostream& ostr);
	CStringToProcessEntry    GetDedicatedManager(char* pszRequest, char* pszStartActionName, char* pszEndActionName, eOtherProcessQueueEntry& eDestTask);
	CStringToProcessTypeMap* GetMap();
	void                     ClearMap();
private:
	CStringToProcessTypeMap* m_pMap;


	void         OnMissingAuditAttr(const char* action, const char* desc, const char* detail);
	eProcessType ProcNameToProcType(char* pszProcName);
	void         MapFree();
};

#endif // !defined(_ActionRedirection_H__)
