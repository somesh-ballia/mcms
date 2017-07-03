// WrappersSnmp.cpp

#include "WrappersSnmp.h"

#include <iomanip>

#include "SNMPStructs.h"
#include "SystemFunctions.h"

CSnmpMngmntInfoWrapper::CSnmpMngmntInfoWrapper(const SNMP_MMGMNT_INFO_S& data)
  : m_Data(data)
{ }

CSnmpMngmntInfoWrapper::~CSnmpMngmntInfoWrapper()
{ }

void CSnmpMngmntInfoWrapper::Dump(std::ostream& os) const
{
	  
  DumpHeader(os, "SNMP_MMGMNT_INFO_S::Dump");

  char buffer[128];
  SystemDWORDToIpString(m_Data.mngmntIp, buffer);
  os << std::setw(20) << "Mngmnt Ip: " << buffer << " isMngmtIpv6Address " << (int)m_Data.isMngmtIpv6Address  << std::endl;

  SystemDWORDToIpString(m_Data.shelfIp, buffer);
  os << std::setw(20) << "Shelf Ip: " << buffer << " isShelfIpv6Address " << (int)m_Data.isShelfIpv6Address << std::endl;
}

CSnmpCSInfoWrapper::CSnmpCSInfoWrapper(const SNMP_CS_INFO_S& data)
  : m_Data(data)
{ }

CSnmpCSInfoWrapper::~CSnmpCSInfoWrapper()
{ }

void CSnmpCSInfoWrapper::Dump(std::ostream& os) const
{
  DumpHeader(os, "SNMP_CS_INFO_S::Dump");

  char buffer[128];
  SystemDWORDToIpString(m_Data.csIp, buffer);
  os << std::setw(20) << "CS Ip: " << buffer << std::endl;

  SystemDWORDToIpString(m_Data.gkIp, buffer);
  os << std::setw(20) << "GK Ip: " << buffer << std::endl;

  os << std::setw(20) << "IP Type: " << m_Data.type << std::endl;
}

CSnmpCardsInfoWrapper::CSnmpCardsInfoWrapper(const SNMP_CARDS_INFO_S& data)
  : m_Data(data)
{ }

CSnmpCardsInfoWrapper::~CSnmpCardsInfoWrapper()
{ }

void CSnmpCardsInfoWrapper::Dump(std::ostream& os) const
{
  DumpHeader(os, "SNMP_CARDS_INFO_S::Dump");

  for (int i = 0; i < MAX_MFA_NUM; i++)
  {
    char buffer[128];
    SystemDWORDToIpString(m_Data.mfaLinks[i].ipAddress, buffer);

    os << std::setw(20) << "Mfa " << i <<  " Ip: " << buffer << " boardId:  " << m_Data.mfaLinks[i].boardId <<  " portId:  " << m_Data.mfaLinks[i].portId << std::endl;
  }
}


/*-----------------------------------------------------------------------------
	class CSnmpLinkStatusWrapper
-----------------------------------------------------------------------------*/

CSnmpLinkStatusWrapper::CSnmpLinkStatusWrapper(const LINK_STATUS_S& data)
  : m_Data(data)
{ }

CSnmpLinkStatusWrapper::~CSnmpLinkStatusWrapper()
{ }

void CSnmpLinkStatusWrapper::Dump(std::ostream& os) const
{
  DumpHeader(os, "LINK_STATUS_S::Dump");

  os << std::setw(20) << "BoardId " << m_Data.boardId << std::endl;
  os << std::setw(20) << "PortId " << m_Data.portId << std::endl;
  os << std::setw(20) << "IsUp " << (int)m_Data.isUp << std::endl;
}


