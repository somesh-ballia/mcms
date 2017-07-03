// SoftwareLocation.cpp: implementation of the CSoftwareLocation class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "NStream.h"
#include "SoftwareLocation.h"
#include "SystemFunctions.h"
#include "ObjString.h"
#include "ProcessBase.h"



extern char* UrlTypeToString(APIU32 urlType);


// ------------------------------------------------------------
CSoftwareLocation::CSoftwareLocation ()
{
}


// ------------------------------------------------------------
CSoftwareLocation::~CSoftwareLocation ()
{
}


// ------------------------------------------------------------
void  CSoftwareLocation::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n\n"
		<< "SoftwareLocation::Dump\n"
		<< "----------------------\n";

	msg	<< std::setw(20) << "HostName: " << m_swLocationStruct.hostName                   << "\n"
		<< std::setw(20) << "Host Ip: "  << m_swLocationStruct.hostIp                     << "\n"
		<< std::setw(20) << "Location: " << m_swLocationStruct.location                   << "\n"
		<< std::setw(20) << "Url Type: ";
		
	char* urlTypeStr = ::UrlTypeToString(m_swLocationStruct.urlType);
	if (urlTypeStr)
	{
		msg << urlTypeStr << "\n";
	}
	else
	{
		msg << "(invalid: " << m_swLocationStruct.urlType << ")\n";
	}

	msg	<< std::setw(20) << "Username: " << m_swLocationStruct.userName                   << "\n"
		<< std::setw(20) << "Password: " << m_swLocationStruct.password                   << "\n"
		<< std::setw(20) << "vLan Id: "  << m_swLocationStruct.vLanId                     << "\n";
}


// ------------------------------------------------------------
CSoftwareLocation& CSoftwareLocation::operator = (const CSoftwareLocation &rOther)
{
	memcpy( &m_swLocationStruct,
		    &rOther.m_swLocationStruct,
			sizeof(MPL_SW_LOCATION_S) );

	return *this;
}


// ------------------------------------------------------------
BYTE* CSoftwareLocation::GetHostName ()
{
	return m_swLocationStruct.hostName;
}


// ------------------------------------------------------------
void CSoftwareLocation::SetHostName (const BYTE *name)
{
	memcpy(&m_swLocationStruct.hostName, name, NAME_LEN);
}


// ------------------------------------------------------------
DWORD CSoftwareLocation::GetHostIp ()
{
	return m_swLocationStruct.hostIp;
}


// ------------------------------------------------------------
void CSoftwareLocation::SetHostIp (const DWORD ip)
{
	m_swLocationStruct.hostIp = ip;
}


// ------------------------------------------------------------
BYTE* CSoftwareLocation::GetLocation ()
{
	return m_swLocationStruct.location;
}


// ------------------------------------------------------------
void CSoftwareLocation::SetLocation (const BYTE *location)
{
	memcpy(&m_swLocationStruct.location, location, URL_LOCATION_LEN);
}


// ------------------------------------------------------------
eUrlType CSoftwareLocation::GetUrlType ()
{
	return (eUrlType)(m_swLocationStruct.urlType);
}


// ------------------------------------------------------------
void CSoftwareLocation::SetUrlType (const eUrlType type)
{
	m_swLocationStruct.urlType = type;
}


// ------------------------------------------------------------
BYTE* CSoftwareLocation::GetUserName ()
{
	return m_swLocationStruct.userName;
}


// ------------------------------------------------------------
void CSoftwareLocation::SetUserName (const BYTE *name)
{
	memcpy(&m_swLocationStruct.userName, name, NAME_LEN);
}


// ------------------------------------------------------------
BYTE* CSoftwareLocation::GetPassword ()
{
	return m_swLocationStruct.password;
}


// ------------------------------------------------------------
void CSoftwareLocation::SetPassword (const BYTE *pwd)
{
	memcpy(&m_swLocationStruct.password, pwd, NAME_LEN);
}


// ------------------------------------------------------------
DWORD CSoftwareLocation::GetVLanId ()
{
	return m_swLocationStruct.vLanId;
}


// ------------------------------------------------------------
void CSoftwareLocation::SetVLanId (const DWORD id)
{
	m_swLocationStruct.vLanId = id;
}


// ------------------------------------------------------------
void CSoftwareLocation::SetData(const char *data)
{
	memcpy(&m_swLocationStruct, data, sizeof(MPL_SW_LOCATION_S));
	
	m_swLocationStruct.hostName[NAME_LEN-2]			= '\0';
	m_swLocationStruct.location[URL_LOCATION_LEN-2]	= '\0';
	m_swLocationStruct.userName[NAME_LEN-2]			= '\0';
	m_swLocationStruct.password[NAME_LEN-2]			= '\0';
}


// ------------------------------------------------------------
void CSoftwareLocation::ValidateStrings()
{
    CProcessBase *pProcess = CProcessBase::GetProcess();
    
    pProcess->TestStringValidity((char *)m_swLocationStruct.hostName,
                                 NAME_LEN,
                                 __PRETTY_FUNCTION__);
    pProcess->TestStringValidity((char *)m_swLocationStruct.location,
                                 URL_LOCATION_LEN,
                                 __PRETTY_FUNCTION__);
    pProcess->TestStringValidity((char *)m_swLocationStruct.userName,
                                 NAME_LEN,
                                 __PRETTY_FUNCTION__);
    pProcess->TestStringValidity((char *)m_swLocationStruct.password,
                                 NAME_LEN,
                                 __PRETTY_FUNCTION__);
}
