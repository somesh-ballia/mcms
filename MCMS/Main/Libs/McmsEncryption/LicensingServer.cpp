// LicensingServer.cpp

#include "licensingServer.h"

#include <stdio.h>
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "McuMngrStructs.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "Versions.h"

// ------------------------------------------------------------
CLicensingServer::CLicensingServer ()
{
	m_primaryLicenseServer     = "";
	m_primaryLicenseServerPort = 0;
	m_LicenseChangedCounter = 0;
}


// ------------------------------------------------------------
CLicensingServer::CLicensingServer(const CLicensingServer &other):
CSerializeObject(other)
{
	m_primaryLicenseServer      = other.m_primaryLicenseServer;
	m_primaryLicenseServerPort  = other.m_primaryLicenseServerPort;
	m_LicenseChangedCounter = other.m_LicenseChangedCounter;
}


// ------------------------------------------------------------
CLicensingServer::~CLicensingServer ()
{
}


// ------------------------------------------------------------
void  CLicensingServer::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n\n"
	    << "CLicensingServer::Dump\n"
		<< "-------------\n";

	msg << "PrimaryLicenseServer: " << m_primaryLicenseServer;
	msg << "PrimaryLicenseServerPort: " << m_primaryLicenseServerPort;
	msg << "LicenseChangedCounter: " << m_LicenseChangedCounter;

}


// ------------------------------------------------------------
void CLicensingServer::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	TRACESTR(eLevelInfoNormal) << "inside CLicensingServer::SerializeXml";

	CXMLDOMElement* pLicensingServerNode = NULL;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("LICENSE_DATA");
		pLicensingServerNode = pFatherNode;
	}
	else
	{
		pLicensingServerNode = pFatherNode->AddChildNode("LICENSE_DATA");
	}

	pLicensingServerNode->AddChildNode( "PRIMARY_LICENSE_SERVER", m_primaryLicenseServer.c_str() );
	pFatherNode->AddChildNode( "PRIMARY_LICENSE_SERVER_PORT", m_primaryLicenseServerPort );

	TRACESTR(eLevelInfoNormal) << "inside CLicensingServer::SerializeXml  m_primaryLicenseServer " << m_primaryLicenseServer.c_str();


}


/* ------------------------------------------------------------
int	 CLicensingServer::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
    STATUS nStatus = STATUS_OK;

    CXMLDOMElement* pLicensingServerNode = NULL;

    char licensingServerStr[KEYCODE_LENGTH];
    memset(licensingServerStr,0,KEYCODE_LENGTH);

   // GET_FIRST_CHILD_NODE(pActionNode, "UPDATE_LICENSING_SERVER", pLicensingServerNode);

    if (pActionNode)
    {

    	GET_VALIDATE_CHILD(pActionNode ,"PRIMARY_LICENSE_SERVER",licensingServerStr,ONE_LINE_BUFFER_LENGTH);
    	m_primaryLicenseServer = licensingServerStr;
    	GET_VALIDATE_CHILD(pActionNode,"PRIMARY_LICENSE_SERVER_PORT", &m_primaryLicenseServerPort,_0_TO_DWORD);
    }

    return nStatus;
}*/

// ------------------------------------------------------------
int	 CLicensingServer::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	TRACESTR(eLevelInfoNormal) << "inside CLicensingServer::DeSerializeXml";

	bool isFromFile = true;

		if (action && strcmp(action, "UPDATE_LICENSING_SERVER") == 0)
		{
			isFromFile = false;
		}

		if (isFromFile)
		{
			return DeSerializeXmlFromFile(pActionNode, pszError);
		}
		else
		{
			return DeSerializeXmlFromEma(pActionNode, pszError);
		}


}


//called upon SET_CDR_SETTINGS
int CLicensingServer::DeSerializeXmlFromEma(CXMLDOMElement* pXMLRootElement,char *pszError)
{

	TRACESTR(eLevelInfoNormal) << "inside CLicensingServer::DeSerializeXmlFromEma";
	CXMLDOMElement* pLicensingServerNode = NULL;
	int nStatus=STATUS_OK;
	char tmpData        [512];  memset(tmpData        , '\0', sizeof(tmpData        ));

	//GET_FIRST_CHILD_NODE(pXMLRootElement, "LICENSE_DATA", pLicensingServerNode);
	if (pXMLRootElement)
	{

		GET_VALIDATE_MANDATORY_CHILD(pXMLRootElement ,"PRIMARY_LICENSE_SERVER",tmpData,ONE_LINE_BUFFER_LENGTH);
		m_primaryLicenseServer = tmpData;
		GET_VALIDATE_MANDATORY_CHILD(pXMLRootElement,"PRIMARY_LICENSE_SERVER_PORT", &m_primaryLicenseServerPort,_0_TO_DWORD);

	}
	else
			TRACESTR(eLevelInfoNormal) << "inside CLicensingServer::DeSerializeXmlFromEma; pXMLRootElement is NULL";

	return STATUS_OK;
}


//called upon startup and Get LICENSING_SERVER
int CLicensingServer::DeSerializeXmlFromFile(CXMLDOMElement* pXMLRootElement,char *pszError)
{
	TRACESTR(eLevelInfoNormal) << "inside CLicensingServer::DeSerializeXmlFromFile;";

	//CXMLDOMElement* pLicensingServerNode = NULL;
	//GET_FIRST_CHILD_NODE(pXMLRootElement, "LICENSE_DATA", pLicensingServerNode);

	int nStatus=STATUS_OK;
	char tmpData        [512];  memset(tmpData        , '\0', sizeof(tmpData        ));

	if (pXMLRootElement)
	{

		GET_VALIDATE_CHILD(pXMLRootElement,"PRIMARY_LICENSE_SERVER",tmpData,ONE_LINE_BUFFER_LENGTH);
		m_primaryLicenseServer = tmpData;

		GET_VALIDATE_CHILD(pXMLRootElement, "PRIMARY_LICENSE_SERVER_PORT", &m_primaryLicenseServerPort,_0_TO_DWORD);

		TRACESTR(eLevelInfoNormal) << "inside CLicensingServer::DeSerializeXmlFromFile  m_primaryLicenseServer " << m_primaryLicenseServer.c_str();

	}
	else
		TRACESTR(eLevelInfoNormal) << "inside CLicensingServer::DeSerializeXmlFromFile; pXMLRootElement is NULL";

		return STATUS_OK;
}


// ------------------------------------------------------------
CLicensingServer& CLicensingServer::operator = (const CLicensingServer &rOther)
{
	m_primaryLicenseServer     = rOther.m_primaryLicenseServer;
	m_primaryLicenseServerPort = rOther.m_primaryLicenseServerPort;
	m_LicenseChangedCounter = rOther.m_LicenseChangedCounter;
    return *this;
}



// ------------------------------------------------------------
void CLicensingServer::SetPrimaryLicenseServer(std::string  primaryLicenseServer)
{
	m_primaryLicenseServer = primaryLicenseServer;
}


// ------------------------------------------------------------
std::string  CLicensingServer::GetPrimaryLicenseServer()
{
	return m_primaryLicenseServer;
}

// ------------------------------------------------------------
void CLicensingServer::SetPrimaryLicenseServerPort(DWORD  primaryLicenseServerPort)
{
	m_primaryLicenseServerPort = primaryLicenseServerPort;
}


// ------------------------------------------------------------
DWORD  CLicensingServer::GetPrimaryLicenseServerPort()
{
	return m_primaryLicenseServerPort;
}
DWORD  CLicensingServer::GetLicenseChangedCounter()
{
	return m_LicenseChangedCounter;
}
void  CLicensingServer::SetLicenseChangedCounter(DWORD updatedCounter)
{
	 m_LicenseChangedCounter = updatedCounter;
}
