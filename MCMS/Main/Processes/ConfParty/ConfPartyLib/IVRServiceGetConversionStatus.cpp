// IVRServiceGetConversionStatus.cpp: implementation of the CIVRServiceGetConversionStatus class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Huizhao	Sun		  Class for GET_CONVERSION_STATUS XML IVR Service
//========   ==============   =====================================================================

#include "IVRServiceGetConversionStatus.h"
#include "CommResDB.h"
#include "psosxml.h"
#include "InitCommonStrings.h"


extern std::map<DWORD, eIvrSlideConversionStatus> & GetConnectConversionStatus();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CIVRServiceGetConversionStatus::CIVRServiceGetConversionStatus()
    : m_connectId(-1)
{

}


/////////////////////////////////////////////////////////////////////////////
CIVRServiceGetConversionStatus& CIVRServiceGetConversionStatus::operator = (const CIVRServiceGetConversionStatus &other)
{
    m_connectId = other.m_connectId;
	return *this;
}


/////////////////////////////////////////////////////////////////////////////
CIVRServiceGetConversionStatus::~CIVRServiceGetConversionStatus()
{
}


///////////////////////////////////////////////////////////////////////////
void CIVRServiceGetConversionStatus::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    std::map<DWORD, eIvrSlideConversionStatus> & mapConversionStatus = ::GetConnectConversionStatus();
    CXMLDOMElement * pConversionStatusNode = pFatherNode->AddChildNode("CONVERSION_STATUS");
    if (mapConversionStatus.end() != mapConversionStatus.find(m_connectId))
    {
        pConversionStatusNode->AddChildNode("CURRENT_CONVERSION_STATUS", eIvrSlideConversionStatusNames[mapConversionStatus[m_connectId]]);
        if (eIvrSlideConversionInProgress != mapConversionStatus[m_connectId])
        {
            // the success or failed status has been reported to EMA, so remove the session status
            mapConversionStatus.erase(m_connectId);
        }
    }
    else
    {
        pConversionStatusNode->AddChildNode("CURRENT_CONVERSION_STATUS", eIvrSlideConversionStatusNames[eIvrSlideConversionSuccess]);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CIVRServiceGetConversionStatus::DeSerializeXml(CXMLDOMElement* pIVRServiceNode, char* pszError, const char * strAction)
{
	int nStatus = STATUS_OK;
	
	return nStatus;
}


