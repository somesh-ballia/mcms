#include <iomanip>
#include "WrappersGK.h"
#include "SystemFunctions.h"
#include "StringsLen.h"

extern char* IpTypeToString(APIU32 ipType, bool caps = false);
extern char* AuthenticationProtocolToString(eAuthenticationProtocol authenticationProtocol);
/*-----------------------------------------------------------------------------
	class CGkManagerServiceParamsIndStructWrapper
-----------------------------------------------------------------------------*/
CGkManagerServiceParamsIndStructWrapper::CGkManagerServiceParamsIndStructWrapper(const GkManagerServiceParamsIndStruct &data)
:m_Data(data)
{}

CGkManagerServiceParamsIndStructWrapper::~CGkManagerServiceParamsIndStructWrapper()
{}

void CGkManagerServiceParamsIndStructWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "GkManagerServiceParamsIndStruct::Dump");

	os << std::setw(20) << "Service Id: "   		<< m_Data.serviceId 					<< std::endl;
	os << std::setw(20) << "Service Name: " 		<< m_Data.serviceName 					<< std::endl;
	os << std::setw(20) << "Ip addres type: " << ::IpTypeToString(m_Data.service_ip_protocol_types) << std::endl;
	os << std::setw(20) << "Is Gk In Service: " 	<< BOOL_TO_STRING(m_Data.bIsGkInService)<< std::endl;
	os << std::setw(20) << "Prefix Name: "          << m_Data.prefixName    				<< std::endl;
	os << std::setw(20) << "Is Reg As Gw: " 		<< BOOL_TO_STRING(m_Data.bIsRegAsGw) 	<< std::endl;
	os << std::setw(20) << "Is Avf: " 				<< BOOL_TO_STRING(m_Data.bIsAvf) 		<< std::endl;
	os << std::setw(20) << "RRQ Poling Interval: " 	<< m_Data.rrqPolingInterval 			<< std::endl;
	//for debug
	//os << std::setw(20) << "User Name: " 	        << m_Data.authenticationParams.user_name		<< std::endl;
	//os << std::setw(20) << "Password: " 	        << m_Data.authenticationParams.password		<< std::endl;
	os << std::setw(20) << "Authentication mode: " 	        << BOOL_TO_STRING(m_Data.authenticationParams.isAuthenticationEnabled) 		<< std::endl;
	os << std::setw(20) << "authenticationProtocol: " 	        << ::AuthenticationProtocolToString(m_Data.authenticationParams.authenticationProtocol)		<< std::endl;

	os << std::setw(20) << "Aliases: ";
	for(int i = 0 ; i < MAX_ALIAS_NAMES_NUM ; i++)
	{
		os << CAliasWrapper(m_Data.aliases[i]);
	}

	for (int i=0; i<TOTAL_NUM_OF_IP_ADDRESSES; i++)
	{
		os << "CS_IP " << i << ":" << std::endl;
		os << CIpAddrStructWrapper(m_Data.csIp[i], "CS IP");
	}

	os << CIpAddrStructWrapper(m_Data.gkIp, "GK IP");
	os << CIpAddrStructWrapper(m_Data.alternateGkIp, "Alt IP");

	os << std::endl;

	os << std::setw(20) << "GK Name: " 		<< m_Data.gkName << std::endl;
	os << std::setw(20) << "Alt Gk Name: "	<< m_Data.altGkName << std::endl;

}




/*-----------------------------------------------------------------------------
	class CGkManagerUpdateServicePropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
CGkManagerUpdateServicePropertiesReqStructWrapper::CGkManagerUpdateServicePropertiesReqStructWrapper(const GkManagerUpdateServicePropertiesReqStruct &data)
:m_Data(data)
{}

CGkManagerUpdateServicePropertiesReqStructWrapper::~CGkManagerUpdateServicePropertiesReqStructWrapper()
{}

void CGkManagerUpdateServicePropertiesReqStructWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "GkManagerUpdateServicePropertiesReqStruct::Dump");

	os << std::setw(20) << "Service Id: "   		<< m_Data.serviceId 		<< std::endl;
	os << std::setw(20) << "Service Status: " 		<< m_Data.serviceStatus		<< std::endl;
	os << std::setw(20) << "Service Opcode: " 		<< GkFaultsAndServiceStatusNames[m_Data.eServiceOpcode]	<< std::endl;
	os << std::setw(20) << "GK Id: " 				<< m_Data.gkId 				<< std::endl;
	os << std::setw(20) << "RRQ Poling Interval: "	<< m_Data.rrqPolingInterval << std::endl;
	os << std::setw(20) << "GK Connection State: " 	<< GKConnectionStateNames[m_Data.eGkConnState] << std::endl;

    os << std::setw(20) << "QOS Precedence Audio: " << (WORD)m_Data.ipPrecedenceAudio << std::endl;
    os << std::setw(20) << "QOS Precedence Video: " << (WORD)m_Data.ipPrecedenceVideo << std::endl;
}







/*-----------------------------------------------------------------------------
	class CClearAltGkParamsFromPropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
CClearGkParamsFromPropertiesReqStructWrapper::CClearGkParamsFromPropertiesReqStructWrapper(const ClearGkParamsFromPropertiesReqStruct &data)
:m_Data(data)
{}

CClearGkParamsFromPropertiesReqStructWrapper::~CClearGkParamsFromPropertiesReqStructWrapper()
{}

void CClearGkParamsFromPropertiesReqStructWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "ClearAltGkParamsFromPropertiesReqStruct::Dump");

	os << std::setw(20) << "Service Id: "   	<< m_Data.serviceId 	<< std::endl;
	os << std::setw(20) << "Index To Clear: "	<< (int)m_Data.indexToClear 	<< std::endl;
}







/*-----------------------------------------------------------------------------
	class CSetAltGkNameInPropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
CSetGkNameInPropertiesReqStructWrapper::CSetGkNameInPropertiesReqStructWrapper(const SetGkNameInPropertiesReqStruct &data)
:m_Data(data)
{}

CSetGkNameInPropertiesReqStructWrapper::~CSetGkNameInPropertiesReqStructWrapper()
{}

void CSetGkNameInPropertiesReqStructWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "SetAltGkNameInPropertiesReqStruct::Dump");

	os << std::setw(20) << "Service Id: "   << m_Data.serviceId 	<< std::endl;
	os << std::setw(20) << "Index To Set: "	<< m_Data.indexToSet 	<< std::endl;
	os << std::setw(20) << "Alt Name: "		<< m_Data.gkName 		<< std::endl;
}






/*-----------------------------------------------------------------------------
	class CSetAltGkIdInPropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
CSetGkIdInPropertiesReqStructWrapper::CSetGkIdInPropertiesReqStructWrapper(const SetGkIdInPropertiesReqStruct &data)
:m_Data(data)
{}

CSetGkIdInPropertiesReqStructWrapper::~CSetGkIdInPropertiesReqStructWrapper()
{}

void CSetGkIdInPropertiesReqStructWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "SetAltGkIdInPropertiesReqStruct::Dump");

	os << std::setw(20) << "Service Id: "   << m_Data.serviceId 	<< std::endl;
	os << std::setw(20) << "Index To Set: "	<< m_Data.indexToSet 	<< std::endl;
	os << std::setw(20) << "Alt Id: "		<< m_Data.gkId 			<< std::endl;
}





/*-----------------------------------------------------------------------------
	class CSetGkIPInPropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
CSetGkIPInPropertiesReqStructWrapper::CSetGkIPInPropertiesReqStructWrapper(const SetGkIPInPropertiesReqStruct &data)
:m_Data(data)
{}

CSetGkIPInPropertiesReqStructWrapper::~CSetGkIPInPropertiesReqStructWrapper()
{}

void CSetGkIPInPropertiesReqStructWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "SetGkIPInPropertiesReqStruct::Dump");

	os << std::setw(20) << "Service Id: "   << m_Data.serviceId 	<< std::endl;
	os << std::setw(20) << "Index To Set: "	<< m_Data.indexToSet 	<< std::endl;

	os << CIpAddrStructWrapper(m_Data.gkIp, "GK IP");
}







/*-----------------------------------------------------------------------------
	class CIpAddrStructWrapper
-----------------------------------------------------------------------------*/
CIpAddrStructWrapper::CIpAddrStructWrapper(const ipAddressStruct &data, const std::string &title)
:m_Data(data), m_Title(title)
{
	if(true == m_Title.empty())
	{
		m_Title = "CIpAddrStructWrapper::Dump";
	}
}

CIpAddrStructWrapper::~CIpAddrStructWrapper()
{}

void CIpAddrStructWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, m_Title.c_str());

	std::string ipStr;
	if (eIpVersion4 == m_Data.ipVersion)
	{
		char v4Buf[IP_ADDRESS_LEN];
		SystemDWORDToIpString(m_Data.addr.v4.ip, v4Buf);
		ipStr = v4Buf;
	}
	else if (eIpVersion6 == m_Data.ipVersion)
	{
		char v6Buf[IPV6_ADDRESS_LEN];
		ipV6ToString(m_Data.addr.v6.ip, v6Buf, TRUE);
		ipStr = v6Buf;
	}
	else
	{
		ipStr = "Illegal ip address";
	}

	os << std::setw(20) << "Type: " << m_Data.ipVersion << std::endl;
	os << std::setw(20) << "Ip: " 	<< ipStr.c_str() << std::endl;
}









/*-----------------------------------------------------------------------------
	class CMngmntParamStructWrapper
-----------------------------------------------------------------------------*/
CMngmntParamStructWrapper::CMngmntParamStructWrapper(const MngmntParamStruct & data)
:m_Data(data)
{
}

CMngmntParamStructWrapper::~CMngmntParamStructWrapper()
{}

void CMngmntParamStructWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CMngmntParamStructWrapper");

	char buffer[128];
	SystemDWORDToIpString(m_Data.ipAddress, buffer);
	os << std::setw(20) << "Mngmnt Ip: " << buffer << std::endl;
}
