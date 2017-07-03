// SNMPProcessProcess.cpp

#include "SNMPProcessProcess.h"

#include "SystemFunctions.h"
#include "StringsMaps.h"
#include "SNMPDefines.h"

extern void SNMPProcessManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess()
{
  return new CSNMPProcessProcess;
}

TaskEntryPoint CSNMPProcessProcess::GetManagerEntryPoint()
{
  return SNMPProcessManagerEntryPoint;
}

CSNMPProcessProcess::CSNMPProcessProcess()
{ }

CSNMPProcessProcess::~CSNMPProcessProcess()
{ }

void   	CSNMPProcessProcess::SetSnmpData(const CSnmpData& data)
{
	m_snmpConfiguration = data;
	m_snmpConfiguration.UnSetIsFromEma();
}

void CSNMPProcessProcess::SetJitcMode(BOOL jitc)
{
	m_jitc = jitc;
}

BOOL CSNMPProcessProcess::GetJitcMode() const
{
	return m_jitc;
}

void CSNMPProcessProcess::AddExtraStringsToMap()
{
  CStringsMaps::AddItem(SNMP_VER_ENUM, eSnmpVer1Trap, "snmpv1");
  CStringsMaps::AddItem(SNMP_VER_ENUM, eSnmpVer2Trap, "snmpv2");
  CStringsMaps::AddItem(SNMP_VER_ENUM, eSnmpVer3Trap, "snmpv3");

  CStringsMaps::AddItem(COMMUNITY_PERMISSION_ENUM, 1, "snmp_no_permission");
  CStringsMaps::AddItem(COMMUNITY_PERMISSION_ENUM, 2, "snmp_read_only");
  CStringsMaps::AddItem(COMMUNITY_PERMISSION_ENUM, 3, "snmp_read_write");
  CStringsMaps::AddItem(COMMUNITY_PERMISSION_ENUM, 4, "snmp_write_create");

  CStringsMaps::AddItem(SNMP_AUTH_PROTOCOL, eSapNone, "none");
  CStringsMaps::AddItem(SNMP_AUTH_PROTOCOL, eSapMD5, "MD5");
  CStringsMaps::AddItem(SNMP_AUTH_PROTOCOL, eSapSHA, "SHA");

  CStringsMaps::AddItem(SNMP_PRIV_PROTOCOL, eSppNone, "none");
  CStringsMaps::AddItem(SNMP_PRIV_PROTOCOL, eSppDES, "DES");
  CStringsMaps::AddItem(SNMP_PRIV_PROTOCOL, eSppAES, "AES");

  CStringsMaps::AddItem(SNMP_SECURITY_LEVEL, eSslNone, "none");
  CStringsMaps::AddItem(SNMP_SECURITY_LEVEL, eSslNoAuth, "noauth");
  CStringsMaps::AddItem(SNMP_SECURITY_LEVEL, eSslAuth, "auth");
  CStringsMaps::AddItem(SNMP_SECURITY_LEVEL, eSslPriv, "priv");
}
