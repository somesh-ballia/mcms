#include "AddressBook.h"
#include "StatusesGeneral.h"
#include <sys/stat.h>
#include "TraceStream.h"
#include "Trace.h"

CAddressBook* CAddressBook::m_pInstance = NULL;
//--------------------------------------------------------------------------
CAddressBook* CAddressBook::Instance()
{
	if (m_pInstance == NULL)
		m_pInstance = new CAddressBook();

	return m_pInstance;
}

////////////////////////////////////////////////////////////////////////////
//                        CAddressBook
////////////////////////////////////////////////////////////////////////////
CAddressBook::CAddressBook()
{
	m_tLastUpdate = 0;
	m_pPartiesSet = new multiset<CRsrvParty*, CompareByPartyName>;
	GetAddressBookPartiesSet();
}

//--------------------------------------------------------------------------
CAddressBook::~CAddressBook()
{
	ClearSet();

	POBJDELETE(m_pPartiesSet);
}

//--------------------------------------------------------------------------
void CAddressBook::ClearSet()
{
	multiset<CRsrvParty*, CompareByPartyName>::iterator it;
	for (it = m_pPartiesSet->begin(); it != m_pPartiesSet->end(); ++it)
	{
		CRsrvParty* pParty = *it;
		POBJDELETE(pParty);
	}

	m_pPartiesSet->clear();
}

//--------------------------------------------------------------------------
const char* CAddressBook::GetAddressBookFilePath()
{
	if (!m_sPath.empty())
		return m_sPath.c_str();

	std::string szPathArray[4] = {  (MCU_MCMS_DIR+"/EMACfg/EMA.DataObjects.OfflineTemplates.AddressbookContentStructured_.xml"),
			(MCU_MCMS_DIR+"/EMACfg/EMA.DataObjects.OfflineTemplates.AddressbookContent_.xml"),
			(MCU_TMP_DIR+"/EMACfg/EMA.DataObjects.OfflineTemplates.AddressbookContentStructured_.xml"),
			(MCU_TMP_DIR+"/EMACfg/EMA.DataObjects.OfflineTemplates.AddressbookContent_.xml")};

	for (int i = 0; i < 4; i++)
	{
		if (0 == access(szPathArray[i].c_str(), F_OK))
		{
			m_sPath = szPathArray[i];
			break;
		}
	}

	return m_sPath.c_str();
}

//--------------------------------------------------------------------------
// If a file was changed return last modified time, else - 0
time_t CAddressBook::IsFileChanged()
{
	struct stat stBuffer;
	int iRes = stat(GetAddressBookFilePath(), &stBuffer);
	if (iRes == 0)
	{
		if (stBuffer.st_mtim.tv_sec > m_tLastUpdate)
			return stBuffer.st_mtim.tv_sec;
	}

	return 0;
}

//--------------------------------------------------------------------------
multiset<CRsrvParty*, CompareByPartyName>* CAddressBook::GetAddressBookPartiesSet()
{
	time_t tLastModified = IsFileChanged();
	if (tLastModified != 0)
	{
		const char* sPath  = GetAddressBookFilePath();
		STATUS status = ReadXmlZipFile(sPath);
		if (status == STATUS_OK)
		{
			m_tLastUpdate = tLastModified;
			TRACEINTO << "Address Book data updated";
		}
	}

	return m_pPartiesSet;
}

//--------------------------------------------------------------------------
int CAddressBook::DeSerializeXml(CXMLDOMElement* pRoot, char* pszError, const char* action)
{
	int nStatus;
	CXMLDOMElement* pPartyNode = NULL;
	CXMLDOMElement* pChildNode = NULL;
	ClearSet();
	int numAction = 0;

	GET_CHILD_NODE(pRoot, "PARTIES", pChildNode);
	GET_FIRST_CHILD_NODE(pChildNode, "PARTY", pPartyNode);
	DWORD maxPartyId = 0;

	while (pPartyNode)
	{
		CRsrvParty* pParty = new CRsrvParty;

		nStatus = pParty->DeSerializeXml(pPartyNode, pszError, numAction);

		if (nStatus != 0)
		{
			POBJDELETE(pParty);
			return nStatus;
		}

		m_pPartiesSet->insert(pParty);

		GET_NEXT_CHILD_NODE(pChildNode, "PARTY", pPartyNode);
	}

	return 0;
}
//--------------------------------------------------------------------------
void CAddressBook::SerializeXml(CXMLDOMElement*&) const
{
}

//--------------------------------------------------------------------------
CSerializeObject* CAddressBook::Clone()
{
	return NULL;
}

