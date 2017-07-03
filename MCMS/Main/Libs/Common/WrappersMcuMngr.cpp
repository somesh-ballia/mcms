#include <iomanip>

#include "WrappersMcuMngr.h"
#include "WrappersCommon.h"
#include "McuMngrInternalStructs.h"
#include "DefinesIpServiceStrings.h"
#include "SystemFunctions.h"

extern char* IpTypeToString(APIU32 ipType, bool caps = false);
extern const char* IPv6ScopeIdToString(enScopeId theScopeId);

/*-----------------------------------------------------------------------------
	class CNumOfPortsIndWrapper
-----------------------------------------------------------------------------*/
CNumOfPortsIndWrapper::CNumOfPortsIndWrapper(const CSMNGR_LICENSING_S &data)
:m_Data(data)
{}

CNumOfPortsIndWrapper::~CNumOfPortsIndWrapper()
{}
	
void CNumOfPortsIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CSMNGR_LICENSING_S::Dump");
	
	os << std::setw(20) << "numOfCpPorts" << " : "  << m_Data.numOfCpPorts << std::endl;
}












/*-----------------------------------------------------------------------------
	class CDnsHostRegistrationIndWrapper
-----------------------------------------------------------------------------*/
CDnsHostRegistrationIndWrapper::CDnsHostRegistrationIndWrapper(const DNS_HOST_REGISTRATION_S &data)
:m_Data(data)
{}

CDnsHostRegistrationIndWrapper::~CDnsHostRegistrationIndWrapper()
{}

void CDnsHostRegistrationIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "DNS_HOST_REGISTRATION_S::Dump");
	
	const char *strServerStatus = (	m_Data.serverStatus <= eServerStatusOff
									?
									ServerStatusStr[m_Data.serverStatus] : "Invalid");
	const char *strIsregistrationAuto = BOOL_TO_STRING(m_Data.isRegistrationAuto);
	
	os 	<< std::setw(25) << "Registration Type" 	<< " : " << strServerStatus		  << std::endl
		<< std::setw(25) << "Is Registration Auto" 	<< " : " << strIsregistrationAuto << std::endl
		<< std::setw(25) << "Domain Name" 			<< " : " << m_Data.domainName 	  << std::endl
		<< std::setw(25) << "Host Name" 			<< " : " << m_Data.hostName 	  << std::endl;
}













/*-----------------------------------------------------------------------------
	class CGKLicensingWrapper
-----------------------------------------------------------------------------*/
CGKLicensingWrapper::CGKLicensingWrapper(const GK_LICENSING_S &data)
:m_Data(data)
{}

CGKLicensingWrapper::~CGKLicensingWrapper()
{}

void CGKLicensingWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "GK_LICENSING_S::Dump");

    const char *strIsPartnerAvaya = BOOL_TO_STRING(m_Data.partner_Avaya);
    os 	<< std::setw(25) << "Is Partner Avaya" 	<< " : " << strIsPartnerAvaya << std::endl;
}


/*-----------------------------------------------------------------------------
	class CUDPMCUMNGRPerPQWrapper
-----------------------------------------------------------------------------*/
CUDPMcuMngrPerPQWrapper::CUDPMcuMngrPerPQWrapper(const IP_SERVICE_UDP_MCUMNGR_PER_PQ_S &data)
:m_Data(data)
{

}

CUDPMcuMngrPerPQWrapper::~CUDPMcuMngrPerPQWrapper()
{}


void CUDPMcuMngrPerPQWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "IP_SERVICE_UDP_MCUMNGR_PER_PQ_S::Dump");



	os << std::setw(20) << "Port Alloc Type: "   << m_Data.portsAlloctype << std::endl;
	os << std::setw(20) << "Box Id: " << m_Data.boxId << std::endl;
	os << std::setw(20) << "Board Id: " 	<< m_Data.boardId << std::endl;

	os << std::setw(20) << "Sub Board Id: "   << m_Data.subBoardId << std::endl;
	os << std::setw(20) << "PQ Id: " << m_Data.PQid << std::endl;
	os << std::setw(20) << "Type: " 	<< m_Data.type << std::endl;

	os << std::setw(20) << "IpType: "		<< ::IpTypeToString(m_Data.IpType) << std::endl;
	os << std::setw(20) << "IpV4Addr: "		<< CIPV4Wrapper((ipAddressV4If&)(m_Data.IpV4Addr) ) << std::endl;
	os << std::setw(20) << "IpV6Addr: " 	<< CIPV6AraryWrapper( (ipv6AddressArray&)(*(m_Data.IpV6Addr)) ) << std::endl;

	// ipAddress                    IpAddr;

	//os << std::setw(20) << "Udp First Channel: "   << m_Data.UdpFirstPort << std::endl;
	//os << std::setw(20) << "Udp Last Channel: " << m_Data.UdpLastPort << std::endl;

}



/*-----------------------------------------------------------------------------
	class CUDPMCUMNGRWrapper
-----------------------------------------------------------------------------*/

CUDPMcuMngrWrapper::CUDPMcuMngrWrapper(const IP_SERVICE_MCUMNGR_S &data)
:m_Data(data)
{
}

CUDPMcuMngrWrapper::~CUDPMcuMngrWrapper()
{
}

void CUDPMcuMngrWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "IP_SERVICE_MCUMNGR_S::Dump");

	os << std::setw(20) << "Service Id: "   << m_Data.ServId << std::endl;
	//os << std::setw(20) << "Service Name: " << m_Data.ServName << std::endl;
	os << std::setw(20) << "Num of PQ: " 	<< m_Data.numPQSactual << std::endl;
	os << std::setw(20) << "DNS STATUS: " 	<< m_Data.dnsStatus << std::endl;

	for(int i = 0 ; i < MAX_NUM_PQS ; i++)
	{
		os << CUDPMcuMngrPerPQWrapper(m_Data.IPServUDPperPQList[i]) << std::endl;
	}
}






