#include "WrappersCS.h"
#include "Macros.h"
#include <iomanip>
#include "PingData.h"

extern char* CsCompTypeToString(compTypes compType);
extern char* CsCompStatusToString(compStatuses compStatus);


/*-----------------------------------------------------------------------------
	class CVersionWrapper
-----------------------------------------------------------------------------*/
CVersionWrapper::CVersionWrapper(const VERSION_S &data)
:m_Data(data)
{}

CVersionWrapper::~CVersionWrapper()
{}

void CVersionWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "VERSION_S::Dump");

	os << std::setw(20) << "Major Version: "  	<< m_Data.ver_major << std::endl;
	os << std::setw(20) << "Minor Version: "  	<< m_Data.ver_minor << std::endl;
	os << std::setw(20) << "Release Version: "  << m_Data.ver_release << std::endl;
	os << std::setw(20) << "Internal Version: " << m_Data.ver_internal << std::endl;
}





/*-----------------------------------------------------------------------------
	class CNewIndWrapper
-----------------------------------------------------------------------------*/
CNewIndWrapper::CNewIndWrapper(const CS_New_Ind_S &data)
:m_Data(data)
{}

CNewIndWrapper::~CNewIndWrapper()
{}


void CNewIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_New_Ind_S::Dump");

	os << std::setw(20) << "Id: "  << m_Data.id << std::endl;
	os << CVersionWrapper(m_Data.version);
}







/*-----------------------------------------------------------------------------
	class CNewReqWrapper
-----------------------------------------------------------------------------*/
CNewReqWrapper::CNewReqWrapper(const CS_New_Req_S &data)
:m_Data(data)
{}

CNewReqWrapper::~CNewReqWrapper()
{}

void CNewReqWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_New_Req_S::Dump");

	os << std::setw(20) << "Num H323 Ports: "  << m_Data.num_h323_ports << std::endl;
	os << std::setw(20) << "Num Sip Ports: "   << m_Data.num_sip_ports << std::endl;
}







/*-----------------------------------------------------------------------------
	class CConfigReqWrapper
-----------------------------------------------------------------------------*/
CConfigReqWrapper::CConfigReqWrapper(const CS_Config_Req_S &data)
:m_Data(data)
{}

CConfigReqWrapper::~CConfigReqWrapper()
{}

void CConfigReqWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_Config_Req_S::Dump");

	os << std::setw(20) << "Section: "  << m_Data.section << std::endl;
	os << std::setw(20) << "key: "   << m_Data.key << std::endl;
	os << std::setw(20) << "data: "   << m_Data.data << std::endl;
}






/*-----------------------------------------------------------------------------
	class CConfigIndWrapper
-----------------------------------------------------------------------------*/
CConfigIndWrapper::CConfigIndWrapper(const CS_Config_Ind_S &data)
:m_Data(data)
{}

CConfigIndWrapper::~CConfigIndWrapper()
{}

void CConfigIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_Config_Ind_S::Dump");

	os << std::setw(20) << "Section Name: "  << m_Data.str_section_name << std::endl;
	os << std::setw(20) << "Key Name: "   << m_Data.str_key_name << std::endl;
	os << std::setw(20) << "Result: "   << m_Data.result << std::endl;
	os << std::setw(20) << "Error Reason: "   << m_Data.str_error_reason << std::endl;
}












/*-----------------------------------------------------------------------------
	class CEndConfigReqWrapper
-----------------------------------------------------------------------------*/
CEndConfigReqWrapper::CEndConfigReqWrapper(const CS_End_Config_Req_S &data)
:m_Data(data)
{}

CEndConfigReqWrapper::~CEndConfigReqWrapper()
{}

void CEndConfigReqWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_End_Config_Req_S::Dump");
}










/*-----------------------------------------------------------------------------
	class CEndConfigIndWrapper
-----------------------------------------------------------------------------*/
CEndConfigIndWrapper::CEndConfigIndWrapper(const CS_End_Config_Ind_S &data)
:m_Data(data)
{}

CEndConfigIndWrapper::~CEndConfigIndWrapper()
{}

void CEndConfigIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_End_Config_Ind_S::Dump");

	os << std::setw(20) << "Result: "  << m_Data.result << std::endl;
	os << std::setw(20) << "Error Reason: "   << m_Data.str_error_reason << std::endl;
}














/*-----------------------------------------------------------------------------
	class CLanCfgReqWrapper
-----------------------------------------------------------------------------*/
CLanCfgReqWrapper::CLanCfgReqWrapper(const CS_Lan_Cfg_Req_S &data)
:m_Data(data)
{}

CLanCfgReqWrapper::~CLanCfgReqWrapper()
{}

void CLanCfgReqWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_Lan_Cfg_Req_S::Dump");
}











/*-----------------------------------------------------------------------------
	class CEndStartupIndWrapper
-----------------------------------------------------------------------------*/
CEndStartupIndWrapper::CEndStartupIndWrapper(const CS_End_StartUp_Ind_S &data)
:m_Data(data)
{}

CEndStartupIndWrapper::~CEndStartupIndWrapper()
{}

void CEndStartupIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_End_StartUp_Ind_S::Dump");

	os << std::setw(20) << "Result: "  << m_Data.result << std::endl;
	os << std::setw(20) << "Error Reason: "   << m_Data.str_error_reason << std::endl;
}






















/*-----------------------------------------------------------------------------
	class CReconnectReqWrapper
-----------------------------------------------------------------------------*/
CReconnectReqWrapper::CReconnectReqWrapper(const CS_Reconnect_Req_S &data)
:m_Data(data)
{}

CReconnectReqWrapper::~CReconnectReqWrapper()
{}

void CReconnectReqWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_Reconnect_Req_S::Dump");
}













/*-----------------------------------------------------------------------------
	class CReconnectIndWrapper
-----------------------------------------------------------------------------*/
CReconnectIndWrapper::CReconnectIndWrapper(const CS_Reconnect_Ind_S &data)
:m_Data(data)
{}

CReconnectIndWrapper::~CReconnectIndWrapper()
{}

void CReconnectIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CS_Reconnect_Ind_S::Dump");
}













/*-----------------------------------------------------------------------------
	class CCSBufferIndWrapper
-----------------------------------------------------------------------------*/
CCSBufferIndWrapper::CCSBufferIndWrapper(const char *data, const int dataLen)
{
	m_Data = new char[dataLen + 1];
	memcpy(m_Data, data, dataLen);
	m_Data[dataLen] = '\0';
}

CCSBufferIndWrapper::~CCSBufferIndWrapper()
{
	PDELETEA(m_Data);
}

void CCSBufferIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "Buffer from CS::Dump");

	os << m_Data << std::endl;
}











/*-----------------------------------------------------------------------------
	class CCSKeepAliveIndWrapper
-----------------------------------------------------------------------------*/
CCSKeepAliveIndWrapper::CCSKeepAliveIndWrapper(const csKeepAliveSt &data)
:m_Data(data)
{}

CCSKeepAliveIndWrapper::~CCSKeepAliveIndWrapper()
{}

void CCSKeepAliveIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "Keep Alive Ind::Dump");

	os << "Components' Statuses (in GL1 only " << NumOfComponentsOnGL1 << " components are relevant):" << std::endl;
	for (int i=0; i < NumOfComponentsOnGL1/*NumOfComponents*/; i++) // print only 3 first components (for improving the logger)
	{
		os << "Component " << i;

		char* csCompTypeStr   = ::CsCompTypeToString(m_Data.componentTbl[i].type);
		char* csCompStatusStr = ::CsCompStatusToString(m_Data.componentTbl[i].status);
		if (csCompTypeStr)
		{
			os << " - type: "  << csCompTypeStr;
		}
		else
		{
			os << " - type: (invalid: "  << m_Data.componentTbl[i].type << ")";
		}

		if (csCompStatusStr)
		{
			os << ", status: " << csCompStatusStr;
		}
		else
		{
			os << ", status: (invalid: " << m_Data.componentTbl[i].status << ")";
		}

		os << std::endl;
	}
}


/*-----------------------------------------------------------------------------
	class CPingReqWrapper
-----------------------------------------------------------------------------*/
CPingReqWrapper::CPingReqWrapper(const CS_Ping_req_S &data)
:m_Data(data)
{}

CPingReqWrapper::~CPingReqWrapper()
{}

void CPingReqWrapper::Dump(std::ostream &os) const
{
    DumpHeader(os, "Ping Request::Dump");
    CPingData pingData((ePingIpType)m_Data.ipType, m_Data.destination);
    pingData.DumpCsMsg ("Send this Ping To CS", eLevelInfoNormal);
}



/*-----------------------------------------------------------------------------
	class CPingIndWrapper
-----------------------------------------------------------------------------*/
CPingIndWrapper::CPingIndWrapper(const CS_Ping_ind_S &data)
:m_Data(data)
{}

CPingIndWrapper::~CPingIndWrapper()
{}

void CPingIndWrapper::Dump(std::ostream &os) const
{
    DumpHeader(os, "Ping Indication::Dump");
    os << "\n Ping Status from CS = ";
    if (ePingStatus_ok == m_Data.pingStatus)
        os << "ePingStatus_ok";
    else
        os << "ePingStatus_fail";
}
