// CIpPortRange.cpp: implementation of the CIpPortRange class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "IpPortRange.h"
#include "SystemFunctions.h"




// ------------------------------------------------------------
CIpPortRange::CIpPortRange ()
{
}


// ------------------------------------------------------------
CIpPortRange::CIpPortRange (const IP_PORT_RANGE_S ipPortRange)
{
	memcpy(&m_ipPortRangeStruct, &ipPortRange, sizeof(IP_PORT_RANGE_S));
}


// ------------------------------------------------------------
CIpPortRange::~CIpPortRange ()
{
}


// ------------------------------------------------------------
void  CIpPortRange::Dump(ostream& msg) const
{
	msg.setf(ios::left,ios::adjustfield);
	msg.setf(ios::showbase);
	
	msg << "\n\n"
		<< "IpPortRange::Dump\n"
		<< "-----------------\n";

//	CPObject::Dump(msg);

	msg	<< setw(20) << "Dynamic Port Allocation: " << m_ipPortRangeStruct.dynamicPortAllocation << "\n"
		<< setw(20) << "Signalling First Port: "   << m_ipPortRangeStruct.signallingFirstPort   << "\n"
		<< setw(20) << "Signalling Num Ports: "    << m_ipPortRangeStruct.signallingNumPorts    << "\n"
		<< setw(20) << "Control First Port: "      << m_ipPortRangeStruct.controlFirstPort      << "\n"
		<< setw(20) << "Control Num Ports: "       << m_ipPortRangeStruct.controlNumPorts       << "\n"
		<< setw(20) << "Audio First Port: "        << m_ipPortRangeStruct.audioFirstPort        << "\n"
		<< setw(20) << "Audio Num Ports: "         << m_ipPortRangeStruct.audioNumPorts         << "\n"
		<< setw(20) << "Video First Port: "        << m_ipPortRangeStruct.videoFirstPort        << "\n"
		<< setw(20) << "Video Num Ports: "         << m_ipPortRangeStruct.videoNumPorts         << "\n"
		<< setw(20) << "Content First Port: "         << m_ipPortRangeStruct.contentFirstPort         << "\n"
		<< setw(20) << "Content Num Ports: "          << m_ipPortRangeStruct.contentNumPorts          << "\n"
		<< setw(20) << "Fecc First Port: "         << m_ipPortRangeStruct.feccFirstPort         << "\n"
		<< setw(20) << "Fecc Num Ports: "          << m_ipPortRangeStruct.feccNumPorts          << "\n"
		<< setw(20) << "Num Intended Calls: "      << m_ipPortRangeStruct.numIntendedCalls      << "\n"
		<< setw(20) << "Enable Port Range: "       << m_ipPortRangeStruct.enablePortRange       << "\n";

	msg << "\n\n";
}


// ------------------------------------------------------------
CIpPortRange& CIpPortRange::operator = (const CIpPortRange &rOther)
{
	memcpy(&m_ipPortRangeStruct, &rOther.m_ipPortRangeStruct, sizeof(IP_PORT_RANGE_S));

	return *this;
}


// ------------------------------------------------------------
IP_PORT_RANGE_S CIpPortRange::GetIpPortRangeStruct()
{
	return m_ipPortRangeStruct;
}


// ------------------------------------------------------------
void CIpPortRange::SetIpPortRangeStruct(CIpPortRange ipPortRange)
{
	memcpy(&m_ipPortRangeStruct, &ipPortRange.m_ipPortRangeStruct, sizeof(IP_PORT_RANGE_S));
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetDynamicPortAllocation ()
{
	return m_ipPortRangeStruct.dynamicPortAllocation;
}

// ------------------------------------------------------------
void CIpPortRange::SetDynamicPortAllocation (const DWORD allocation)
{
	m_ipPortRangeStruct.dynamicPortAllocation = allocation;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetSignallingFirstPort ()
{
	return m_ipPortRangeStruct.signallingFirstPort;
}

// ------------------------------------------------------------
void CIpPortRange::SetSignallingFirstPort (const DWORD sgnlngFirstPort)
{
	m_ipPortRangeStruct.signallingFirstPort = sgnlngFirstPort;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetSignallingNumPorts ()
{
	return m_ipPortRangeStruct.signallingNumPorts;
}

// ------------------------------------------------------------
void CIpPortRange::SetSignallingNumPorts (const DWORD sgnlngNumPorts)
{
	m_ipPortRangeStruct.signallingNumPorts = sgnlngNumPorts;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetControlFirstPort ()
{
	return m_ipPortRangeStruct.controlFirstPort;
}

// ------------------------------------------------------------
void CIpPortRange::SetControlFirstPort (const DWORD cntrlFirstPort)
{
	m_ipPortRangeStruct.controlFirstPort = cntrlFirstPort;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetControlNumPorts ()
{
	return m_ipPortRangeStruct.controlNumPorts;
}

// ------------------------------------------------------------
void CIpPortRange::SetControlNumPorts (const DWORD cntrlNumPorts)
{
	m_ipPortRangeStruct.controlNumPorts = cntrlNumPorts;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetAudioFirstPort ()
{
	return m_ipPortRangeStruct.audioFirstPort;
}

// ------------------------------------------------------------
void CIpPortRange::SetAudioFirstPort (const DWORD audFirstPort)
{
	m_ipPortRangeStruct.audioFirstPort = audFirstPort;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetAudioNumPorts ()
{
	return m_ipPortRangeStruct.audioNumPorts;
}

// ------------------------------------------------------------
void CIpPortRange::SetAudioNumPorts (const DWORD audNumPorts)
{
	m_ipPortRangeStruct.audioNumPorts = audNumPorts;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetVideoFirstPort ()
{
	return m_ipPortRangeStruct.videoFirstPort;
}

// ------------------------------------------------------------
void CIpPortRange::SetVideoFirstPort (const DWORD vidFirstPort)
{
	m_ipPortRangeStruct.videoFirstPort = vidFirstPort;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetVideoNumPorts ()
{
	return m_ipPortRangeStruct.videoNumPorts;
}

// ------------------------------------------------------------
void CIpPortRange::SetVideoNumPorts (const DWORD vidNumPorts)
{
	m_ipPortRangeStruct.videoNumPorts = vidNumPorts;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetContentFirstPort ()
{
	return m_ipPortRangeStruct.contentFirstPort;
}

// ------------------------------------------------------------
void CIpPortRange::SetContentFirstPort (const DWORD content_FirstPort)
{
	m_ipPortRangeStruct.contentFirstPort = content_FirstPort;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetContentNumPorts ()
{
	return m_ipPortRangeStruct.contentNumPorts;
}

// ------------------------------------------------------------
void CIpPortRange::SetContentNumPorts (const DWORD content_NumPorts)
{
	m_ipPortRangeStruct.contentNumPorts = content_NumPorts;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetFeccFirstPort ()
{
	return m_ipPortRangeStruct.feccFirstPort;
}

// ------------------------------------------------------------
void CIpPortRange::SetFeccFirstPort (const DWORD fecc_FirstPort)
{
	m_ipPortRangeStruct.feccFirstPort = fecc_FirstPort;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetFeccNumPorts ()
{
	return m_ipPortRangeStruct.feccNumPorts;
}

// ------------------------------------------------------------
void CIpPortRange::SetFeccNumPorts (const DWORD fecc_NumPorts)
{
	m_ipPortRangeStruct.feccNumPorts = fecc_NumPorts;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetNumIntendedCalls ()
{
	return m_ipPortRangeStruct.numIntendedCalls;
}

// ------------------------------------------------------------
void CIpPortRange::SetNumIntendedCalls (const DWORD numIntndCalls)
{
	m_ipPortRangeStruct.numIntendedCalls = numIntndCalls;
}

// ------------------------------------------------------------
DWORD CIpPortRange::GetEnablePortRange ()
{
	return m_ipPortRangeStruct.enablePortRange;
}

// ------------------------------------------------------------
void CIpPortRange::SetEnablePortRange (const DWORD enableRange)
{
	m_ipPortRangeStruct.enablePortRange = enableRange;
}

// ------------------------------------------------------------
void CIpPortRange::SetData(IP_PORT_RANGE_S ipPortRange)
{
	memcpy(&m_ipPortRangeStruct, &ipPortRange, sizeof(IP_PORT_RANGE_S));
}

// ------------------------------------------------------------
