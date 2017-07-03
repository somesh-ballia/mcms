
#include "CustomizeDisplaySettingForOngoingConfConfiguration.h"

CCustomizeDisplaySettingForOngoingConfConfiguration::CCustomizeDisplaySettingForOngoingConfConfiguration()
{
	m_bObtainDisplayNamefromAddressBook = FALSE;
}

CCustomizeDisplaySettingForOngoingConfConfiguration::~CCustomizeDisplaySettingForOngoingConfConfiguration()
{
}

CCustomizeDisplaySettingForOngoingConfConfiguration& CCustomizeDisplaySettingForOngoingConfConfiguration::operator=(const CCustomizeDisplaySettingForOngoingConfConfiguration& other)
{
	if (this == &other)
		return *this;

	m_bObtainDisplayNamefromAddressBook = other.m_bObtainDisplayNamefromAddressBook;
	
	return *this;
}


void CCustomizeDisplaySettingForOngoingConfConfiguration::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pTempNode = NULL;

	pTempNode = pFatherNode->AddChildNode("CUSTOMIZE_SETUP_ONGOING_CONF");
	pTempNode->AddChildNode("OBTAIN_DISPLAY_NAME_FROM_ADDRESS_BOOK",m_bObtainDisplayNamefromAddressBook,_BOOL);
}

int  CCustomizeDisplaySettingForOngoingConfConfiguration::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	return DeSerializeXml(pActionNode, pszError);
}

int  CCustomizeDisplaySettingForOngoingConfConfiguration::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pCustomizeDspSettingForOngingConfNode =NULL;

	CXMLDOMElement *pEventListNode=NULL;
	CXMLDOMElement *pEventInfoNode=NULL;

	GET_CHILD_NODE(pActionNode, "CUSTOMIZE_SETUP_ONGOING_CONF", pCustomizeDspSettingForOngingConfNode);
	if (pCustomizeDspSettingForOngingConfNode)
	{
		GET_VALIDATE_CHILD(pCustomizeDspSettingForOngingConfNode,"OBTAIN_DISPLAY_NAME_FROM_ADDRESS_BOOK",&m_bObtainDisplayNamefromAddressBook,_BOOL);
	}
	else
	{
		nStatus = STATUS_FAIL;
	}

	return nStatus;
	
}

BOOL	CCustomizeDisplaySettingForOngoingConfConfiguration::IsObtainDsipalyNamefromAddressBook() const
{
	return m_bObtainDisplayNamefromAddressBook;
}

void	 CCustomizeDisplaySettingForOngoingConfConfiguration::SetObtainDisplayNamefromAddressBook(BOOL bDsp)
{
	m_bObtainDisplayNamefromAddressBook = bDsp;
}
