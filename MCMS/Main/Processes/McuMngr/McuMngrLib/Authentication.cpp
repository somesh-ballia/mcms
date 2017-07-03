// Authentication.cpp

#include <iostream>
#include <iomanip>

#include "SystemFunctions.h"
#include "Authentication.h"
#include "ObjString.h"
#include "McmsAuthentication.h"

extern const char* PlatformTypeToString(APIU32 platformType);


// ------------------------------------------------------------
CAuthentication::CAuthentication ()
{
	memset( &m_authenticationStruct, 0,	sizeof(MPL_AUTHENTICATION_S) );
}


// ------------------------------------------------------------
void  CAuthentication::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n\n"
		<< "Authentication::Dump\n"
		<< "--------------------\n";

	msg	<< std::setw(20) << "Serial Number: "  << m_authenticationStruct.serialNum << "\n"
	    << std::setw(20) << "Platform Type: ";
	
	const char* platformTypeStr = ::PlatformTypeToString(m_authenticationStruct.platformType);
	if (platformTypeStr)
	{
		msg << platformTypeStr << "\n";
	}
	else
	{
		msg << "(invalid: " << m_authenticationStruct.platformType << ")\n";
	}
	
	msg	<< "X_KeyCode: " << (char*)(m_authenticationStruct.cfs_X_KeyCode) << "\n"
	    << "U_KeyCode: " << (char*)(m_authenticationStruct.cfs_U_KeyCode) << "\n";
	
	msg	<< std::setw(20) << "Mcu Version: "    
		<< m_authenticationStruct.mcuVersion.ver_major    << "."
		<< m_authenticationStruct.mcuVersion.ver_minor    << "."
		<< m_authenticationStruct.mcuVersion.ver_release  << "."
		<< m_authenticationStruct.mcuVersion.ver_internal << "\n\n";
	
	msg << std::setw(20) << "Chassis Version: "
		<< m_authenticationStruct.chassisVersion.ver_major	<< "."
		<< m_authenticationStruct.chassisVersion.ver_minor	<< "."
		<< m_authenticationStruct.chassisVersion.ver_release<< "."
		<< m_authenticationStruct.chassisVersion.ver_internal << "\n\n";

}


// ------------------------------------------------------------
char* CAuthentication::GetSerialNumber ()
{
	return (char*)(m_authenticationStruct.serialNum);
}


// ------------------------------------------------------------
void CAuthentication::SetSerialNumber (const char* theNum)
{
	memcpy( m_authenticationStruct.serialNum,
	        theNum,
	        MPL_SERIAL_NUM_LEN );
}


// ------------------------------------------------------------
ePlatformType CAuthentication::GetPlatformType ()
{
	return (ePlatformType)(m_authenticationStruct.platformType);
}


// ------------------------------------------------------------
void CAuthentication::SetPlatformType (const ePlatformType type)
{
	m_authenticationStruct.platformType = type;
}


// ------------------------------------------------------------
VERSION_S CAuthentication::GetMcuVersionFromMpl ()
{
	return m_authenticationStruct.mcuVersion;
}


// ------------------------------------------------------------
void CAuthentication::SetMcuVersionFromMpl (const VERSION_S mcuVer)
{
	m_authenticationStruct.mcuVersion = mcuVer;
}

// ------------------------------------------------------------
VERSION_S CAuthentication::GetMcuChassisVersionFromMpl ()
{
	return m_authenticationStruct.chassisVersion;
}

// ------------------------------------------------------------
void CAuthentication::SetMcuChassisVersionFromMpl (const VERSION_S chassisVer)
{
	m_authenticationStruct.chassisVersion = chassisVer;
}

// ------------------------------------------------------------
const char* CAuthentication::Get_X_KeyCode () const
{
	return (char*)m_authenticationStruct.cfs_X_KeyCode;
}


// ------------------------------------------------------------
void CAuthentication::Set_X_KeyCode (const char* keyCode)
{
	strcpy_safe((char*)m_authenticationStruct.cfs_X_KeyCode,
						  ARRAYSIZE(m_authenticationStruct.cfs_X_KeyCode),
              keyCode);
}


// ------------------------------------------------------------
const char* CAuthentication::Get_U_KeyCode () const
{
	return (char*)m_authenticationStruct.cfs_U_KeyCode;
	//"U277-4C21-C310-0000-000A";	// ver 1.0, serial#=9251471
	//"U520-BB55-8CB0-0000-03E8";	// ver 100.0, serial#=9251471

	//"U65D-BF16-B5A0-0000-03E8";	// ver 100.0, serial#=0
}


// ------------------------------------------------------------
void CAuthentication::Set_U_KeyCode (const char* keyCode)
{
	strcpy_safe((char*)m_authenticationStruct.cfs_U_KeyCode,
              ARRAYSIZE(m_authenticationStruct.cfs_U_KeyCode),
              keyCode);
}


// ------------------------------------------------------------
void CAuthentication::SetData(const char *data)
{
	memcpy( &m_authenticationStruct, data, sizeof(MPL_AUTHENTICATION_S) );

	m_authenticationStruct.cfs_X_KeyCode[KEYCODE_LENGTH-1] = 0;
	m_authenticationStruct.cfs_U_KeyCode[KEYCODE_LENGTH-1] = 0;
}
// ------------------------------------------------------------
VERSION_S CAuthentication::GetkeyCodeVersionFromMpl ()
{
	return m_authenticationStruct.keyCodeVersion;
}

// ------------------------------------------------------------
APIU8     CAuthentication::GetIsNewCtrlGeneration ()
{
	return m_authenticationStruct.isNewCtrlGeneration;
}

