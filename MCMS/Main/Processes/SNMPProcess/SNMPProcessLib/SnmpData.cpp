#include "SnmpData.h"
#include "SNMPProcessProcess.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "StringsMaps.h"
#include "Trace.h"
#include "TraceStream.h"
#include "FipsMode.h"
#include "ProcessBase.h"
#include "FipsMode.h"

//////////////////////////////////////////////////////////////////////////
CSnmpData::CSnmpData() :
	m_version(eSnmpVer1)

{
	Init();
}

//////////////////////////////////////////////////////////////////////////
CSnmpData::~CSnmpData()
{
}

void CSnmpData::InitDefaults()
{
	m_Security.InitDefaults();
	Init();
}

void CSnmpData::Init()
{
	m_version = eSnmpVer1;
	m_enable_snmp=FALSE;
	m_engineID = "";
	m_mngmntIp = 0;
	m_bIsForEMA = false;
	m_location = "";
	m_systemName = "";
	m_systemName = "";
	m_curProductType = CProcessBase::GetProcess()->GetProductType();

	if(eProductTypeSoftMCU == m_curProductType)
	{
		Set_enable_snmp(TRUE);
		SetVersion(eSnmpVer2);
	}
	else if ( eProductTypeSoftMCUMfw == m_curProductType)
	{
		Set_enable_snmp(TRUE);
		m_enable_snmp=TRUE;
		SetVersion(eSnmpVer3);

		m_Security.m_community.m_snmpV3Params.SetAuthPassword("SameTime9");
		m_Security.m_community.m_snmpV3Params.SetPrivPassword("SameTime9");
		m_Security.m_community.m_snmpV3Params.SetUserName("SameTime9");

	}
	if (((CSNMPProcessProcess*)CProcessBase::GetProcess())->GetJitcMode() == TRUE)
	{
		SetVersion(eSnmpVer3);
	}
}
//////////////////////////////////////////////////////////////////////////
CSnmpData::CSnmpData(const CSnmpData &other)
        :CSerializeObject(other)
{
    *this = other;
}
//////////////////////////////////////////////////////////////////////////
CSnmpData& CSnmpData::operator = (const CSnmpData& other)
{
	m_enable_snmp = other.m_enable_snmp;
    m_Security = other.m_Security;
    m_location = other.m_location;
    m_contactName = other.m_contactName;
    m_systemName = other.m_systemName;
    m_version = other.m_version;
    m_engineID = other.m_engineID;
    m_mngmntIp = other.m_mngmntIp;
    m_bIsForEMA = other.m_bIsForEMA;
    return *this;
}
///////////////////////////////////////////////////////////////////////////
void CSnmpData::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pSnmpParamsNode = pFatherNode->AddChildNode("SNMP_DATA");
    
	pSnmpParamsNode->AddChildNode("LOCATION", m_location);
	pSnmpParamsNode->AddChildNode("CONTACT_NAME", m_contactName);
	pSnmpParamsNode->AddChildNode("SYSTEM_NAME", m_systemName);
	pSnmpParamsNode->AddChildNode("SNMP_VERSION", m_version, SNMP_VER_ENUM);

	m_Security.SerializeXml(pSnmpParamsNode,m_bIsForEMA);

    pSnmpParamsNode->AddChildNode("SNMP_ENABLED",m_enable_snmp,_BOOL);
    pSnmpParamsNode->AddChildNode("ENGINE_ID", m_engineID);


    if(!m_bIsForEMA)
    {
    	pSnmpParamsNode->AddChildNode("MNGMNT_IP", m_mngmntIp, IP_ADDRESS);
    }


}
///////////////////////////////////////////////////////////////////////////
int CSnmpData::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;



	if( pActionNode )
	{
		CXMLDOMElement *pSnmpDataNode;
		
		GET_CHILD_NODE(pActionNode, "SNMP_DATA", pSnmpDataNode);
		
		if(pSnmpDataNode != NULL)
		{
			BOOL isFipsMode = GetSNMPFipsMode();

            GET_VALIDATE_ASCII_CHILD(pSnmpDataNode,"LOCATION",m_location,_0_TO_SNMP_STRING_LENGTH);
			GET_VALIDATE_ASCII_CHILD(pSnmpDataNode,"CONTACT_NAME",m_contactName,_0_TO_SNMP_STRING_LENGTH);

			GET_VALIDATE_ASCII_CHILD(pSnmpDataNode,"SYSTEM_NAME",m_systemName,_0_TO_SNMP_STRING_LENGTH);

			WORD ver = (WORD)eSnmpVer1;
			if (eProductTypeSoftMCUMfw == m_curProductType)
			{
				ver = (WORD)eSnmpVer3;
			}

			GET_VALIDATE_CHILD(pSnmpDataNode,"SNMP_VERSION",&ver,SNMP_VER_ENUM);
			m_version = (eSnmpVersion) ver;

			nStatus = m_Security.DeSerializeXml(pSnmpDataNode, pszError,action);
            if(STATUS_OK != nStatus)
            {
                return nStatus;
            }
            
            GET_VALIDATE_CHILD(pSnmpDataNode,"SNMP_ENABLED",&m_enable_snmp,_BOOL);

            pSnmpDataNode->GetAndVerifyChildNodeValue("MNGMNT_IP",&m_mngmntIp, pszError, IP_ADDRESS);
			
            nStatus = pSnmpDataNode->GetAndVerifyChildNodeValue("ENGINE_ID", m_engineID, pszError,_0_TO_SNMP_STRING_LENGTH, true);

            if (nStatus == STATUS_NODE_MISSING)
            {
            	nStatus = STATUS_OK;
            }
            else {
            	if (nStatus != STATUS_OK &&
            			nStatus != STATUS_ENUM_VALUE_INVALID &&
            			nStatus != STATUS_NODE_LENGTH_TOO_SHORT &&
            			nStatus != STATUS_NODE_LENGTH_TOO_LONG &&
            			nStatus != STATUS_VALUE_OUT_OF_RANGE &&
            			nStatus != STATUS_IP_ADDRESS_INVALID)
            	{

            		return nStatus;
            	}
            }
		}
	}

	return nStatus;
}
///////////////////////////////////////////////////////////////////////////
const char* CSnmpData::NameOf() const
{
    return "CSnmpData";
}


//////////////////////////////////////////////////////////////////////////
const CSnmpSecurity & CSnmpData::GetSecurityInfo() const
{
    return m_Security;
}


//////////////////////////////////////////////////////////////////////////
/*int CSnmpData::TestValidity() const
{
	ESTATUS status = STATUS_OK;
	if((m_trapInterval > 3600) || (m_trapInterval < 1))
		status = STATUS_INCONSISTENT_PARAMETERS;
    return status;
}
*/
//////////////////////////////////////////////////////////////////////////

void CSnmpData::SetLocation(const string &place)
{
    m_location = place;
}
/////////////////////////////////////////////////////////////////////////////

const string & CSnmpData::GetLocation() const
{
    return  m_location;
}
/////////////////////////////////////////////////////////////////////////////

void CSnmpData::SetContactName(const string & name)
{
    m_contactName = name;
}
/////////////////////////////////////////////////////////////////////////////

const string &  CSnmpData::GetEngineID() const
{
	return m_engineID;
}
/////////////////////////////////////////////////////////////////////////////

DWORD  CSnmpData::GetMngmntIp() const
{
	return m_mngmntIp;
}

/////////////////////////////////////////////////////////////////////////////


const string & CSnmpData::GetContactName() const
{
    return m_contactName;
}
/////////////////////////////////////////////////////////////////////////////

void CSnmpData::SetSystemName(const string & name)
{
     m_systemName =  name;
}

/////////////////////////////////////////////////////////////////////////////
const string &  CSnmpData::GetSystemName() const
{
    return m_systemName;
}
////////////////////////////////////////////////////////////////////////////
WORD   CSnmpData::Is_enable_snmp()const
{
    return m_enable_snmp;
}

/////////////////////////////////////////////////////////////////////////
void   CSnmpData::Set_enable_snmp(const WORD yesNo)
{
    m_enable_snmp = yesNo;
}

void CSnmpData::SetVersion(eSnmpVersion ver)
{
	m_version = ver;
}

eSnmpVersion CSnmpData::GetVersion(void) const
{
	return m_version;
}


/////////////////////////////////////////////////////////////////////////////

void  CSnmpData::SetMngmntIp(DWORD mngmntIp)
{
	m_mngmntIp = mngmntIp;

}



//////////////////////////////////////////////
void CSnmpData::WriteSNMPXmlFile() const
{
	std::string ScriptsFile = "Cfg/Snmp.xml";
	WriteXmlFile( ScriptsFile.c_str(), "SNMP_DATA");
}
/////////////////////////////////////////////////////////////////////////////
void CSnmpData::SetIsForEMA(BYTE yesNo)
{
	m_bIsForEMA = yesNo;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSnmpData::GetIsForEMA()
{
	return m_bIsForEMA;
}

void CSnmpData::UnSetIsFromEma() const
{
	m_Security.UnSetIsFromEma();

}
