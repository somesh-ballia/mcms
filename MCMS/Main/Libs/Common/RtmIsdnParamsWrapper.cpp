
#include <iomanip>
#include <stdio.h>
#include <iostream>
#include "RtmIsdnParamsWrapper.h"
#include "ObjString.h"


//extern APIU32 CountryCodeToAPIU32(string countryCodeStr);
//extern string CountryCodeToString(DWORD countryCodeDword);


// ------------------------------------------------------------
CRtmIsdnParamsWrapper::CRtmIsdnParamsWrapper ()
{
	memset( &m_rtmIsdnParamsStruct, 0,	sizeof(RTM_ISDN_PARAMETERS_S) );
}


// ------------------------------------------------------------
CRtmIsdnParamsWrapper::~CRtmIsdnParamsWrapper ()
{
}


// ------------------------------------------------------------
void  CRtmIsdnParamsWrapper::Dump(ostream& msg)
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n\n"
		<< "RtmIsdnParamsWrapper::Dump\n"
		<< "--------------------------\n";

	string countryCodeStr = m_rtmIsdnCommonMethods.CountryCodeToString(m_rtmIsdnParamsStruct.country_code);

	msg	<< std::setw(20) << "Country Code: "	<< countryCodeStr.c_str() << "\n"
	    << std::setw(20) << "Idle Code T1: "	<< m_rtmIsdnParamsStruct.idle_code_T1 << "\n"
	    << std::setw(20) << "Idle Code E1: "	<< m_rtmIsdnParamsStruct.idle_code_E1 << "\n"
	    << std::setw(20) << "Number of digits: "<< m_rtmIsdnParamsStruct.number_of_digits << "\n";
	
	if (YES == m_rtmIsdnParamsStruct.isdn_clock)
	{
		msg	<< std::setw(20) << "ISDN Clocking: YES";
	}
	else
	{
		msg	<< std::setw(20) << "ISDN Clocking: NO";
	}
}


// ------------------------------------------------------------
CRtmIsdnParamsWrapper& CRtmIsdnParamsWrapper::operator = (const CRtmIsdnParamsWrapper &other)
{
	memcpy( &m_rtmIsdnParamsStruct,
		    &(other.m_rtmIsdnParamsStruct),
			sizeof(RTM_ISDN_PARAMETERS_S) );

	return *this;
}


// ------------------------------------------------------------
RTM_ISDN_PARAMETERS_S* CRtmIsdnParamsWrapper::GetRtmIsdnParamsStruct()
{
	return &m_rtmIsdnParamsStruct;
}

// ------------------------------------------------------------
DWORD CRtmIsdnParamsWrapper::GetCountryCode ()
{
	return m_rtmIsdnParamsStruct.country_code;
}


// ------------------------------------------------------------
void CRtmIsdnParamsWrapper::SetCountryCode (const DWORD countryCode)
{
	m_rtmIsdnParamsStruct.country_code = countryCode;
}


// ------------------------------------------------------------
void CRtmIsdnParamsWrapper::SetCountryCode (const string countryCode)
{
	APIU32 dCode = m_rtmIsdnCommonMethods.CountryCodeToAPIU32(countryCode);
	m_rtmIsdnParamsStruct.country_code = (DWORD)dCode;
}


// ------------------------------------------------------------
DWORD CRtmIsdnParamsWrapper::GetIdleCodeT1 ()
{
	return m_rtmIsdnParamsStruct.idle_code_T1;
}


// ------------------------------------------------------------
void CRtmIsdnParamsWrapper::SetIdleCodeT1 (const DWORD idleCode)
{
	m_rtmIsdnParamsStruct.idle_code_T1 = idleCode;
}


// ------------------------------------------------------------
DWORD CRtmIsdnParamsWrapper::GetIdleCodeE1 ()
{
	return m_rtmIsdnParamsStruct.idle_code_E1;
}


// ------------------------------------------------------------
void CRtmIsdnParamsWrapper::SetIdleCodeE1 (const DWORD idleCode)
{
	m_rtmIsdnParamsStruct.idle_code_E1 = idleCode;
}


// ------------------------------------------------------------
DWORD CRtmIsdnParamsWrapper::GetNumOfDigits ()
{
	return m_rtmIsdnParamsStruct.number_of_digits;
}


// ------------------------------------------------------------
void CRtmIsdnParamsWrapper::SetNumOfDigits (const DWORD numOfDigits)
{
	m_rtmIsdnParamsStruct.number_of_digits = numOfDigits;
}


// ------------------------------------------------------------
DWORD CRtmIsdnParamsWrapper::GetClocking ()
{
	return m_rtmIsdnParamsStruct.isdn_clock;
}


// ------------------------------------------------------------
void CRtmIsdnParamsWrapper::SetClocking (const DWORD clocking)
{
	m_rtmIsdnParamsStruct.isdn_clock = clocking;
}


// ------------------------------------------------------------
void CRtmIsdnParamsWrapper::SetData(const char *data)
{
	memcpy( &m_rtmIsdnParamsStruct, data, sizeof(RTM_ISDN_PARAMETERS_S) );
}


// ------------------------------------------------------------
string  CRtmIsdnParamsWrapper::PrintStructData()
{
	string retStr = "RtmIsdnParamsWrapper::PrintStaructData\n";
	retStr +=       "--------------------------------------";
	
	string countryCodeStr = m_rtmIsdnCommonMethods.CountryCodeToString(m_rtmIsdnParamsStruct.country_code);

	char sIdleCodeT1[40], sIdleCodeE1[40], sNumOfDigits[40];
	sprintf(sIdleCodeT1,  "%d", m_rtmIsdnParamsStruct.idle_code_T1);
	sprintf(sIdleCodeE1,  "%d", m_rtmIsdnParamsStruct.idle_code_E1);
	sprintf(sNumOfDigits, "%d", m_rtmIsdnParamsStruct.number_of_digits);

	retStr += "\nCountry Code:     ";
	retStr += countryCodeStr.c_str();
	retStr += "\nIdle Code T1:     ";
	retStr += sIdleCodeT1;
	retStr += "\nIdle Code E1:     ";
	retStr += sIdleCodeE1;
	retStr += "\nNumber of digits: ";
	retStr += sNumOfDigits;
	
	if (YES == m_rtmIsdnParamsStruct.isdn_clock)
	{
		retStr += "\nISDN Clocking:    YES";
	}
	else
	{
		retStr += "\nISDN Clocking:    NO";
	}

	retStr += "\n\n";

	return retStr;
}
