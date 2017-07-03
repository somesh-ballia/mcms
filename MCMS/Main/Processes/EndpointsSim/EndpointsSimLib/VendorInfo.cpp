//+========================================================================+
//                     VendorInfo.cpp									   |
//				Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.                    |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VendorInfo.cpp											   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include "VendorInfo.h"
#include "TraceStream.h"
#include "Macros.h"

#include <string>


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CVendorInfo::CVendorInfo()
{
	/*xxx
	 * m_pszProductId = new char [VENDOR_PRODUCT_ID_LEN];
	memset(m_pszProductId, 0, VENDOR_PRODUCT_ID_LEN);
	
	m_pszVersionId = new char [VENDOR_VERSION_ID_LEN];
	memset(m_pszVersionId, 0, VENDOR_VERSION_ID_LEN);*/
	
	ModeEndpointsSim();
}

/////////////////////////////////////////////////////////////////////////////
CVendorInfo::CVendorInfo(const char* pszManufacturerName)
{
	/*xxx
	m_pszProductId = new char [VENDOR_PRODUCT_ID_LEN];
	memset(m_pszProductId, 0, VENDOR_PRODUCT_ID_LEN);
	
	m_pszVersionId = new char [VENDOR_VERSION_ID_LEN];
	memset(m_pszVersionId, 0, VENDOR_VERSION_ID_LEN);			
	*/
	
	ModeEndpointsSim(); // default mode

	if( pszManufacturerName == NULL )
		return;

	if( 0 == strcmp(pszManufacturerName,"VSX 7000") )
		ModeVSX7000();
	else if( 0 == strcmp(pszManufacturerName,"IP Softphone") )
		ModeIPSoftPhone();
	else if( 0 == strcmp(pszManufacturerName,"FX") )
		ModeFX();
	else if( 0 == strcmp(pszManufacturerName,"Tandberg MXP") )
		ModeMXP();
	else if( 0 == strcmp(pszManufacturerName,"Polycom RPX") )
		ModeRPX();
	else if( 0 == strcmp(pszManufacturerName,"EpSim") )
		;
	else
		TRACESTR(eLevelError) << " CVendorInfo::CVendorInfo - unknown manufacturer name <" << pszManufacturerName << ">.";
}
/////////////////////////////////////////////////////////////////////////////
CVendorInfo::~CVendorInfo()
{
	//TRACESTR(eLevelError) << "xxx" << m_pszProductId;
	//xxxdelete [] m_pszProductId;
		
	//TRACESTR(eLevelError) << "yyy" << m_pszVersionId;	
	//xxxdelete [] m_pszVersionId;	
}

/////////////////////////////////////////////////////////////////////////////
CVendorInfo& CVendorInfo::operator=(const CVendorInfo& other)
{
	if( this == &other )
		return *this;

	m_bCountryCode = other.m_bCountryCode;
	m_bT35Extension = other.m_bT35Extension;
	m_wManufacturerCode = other.m_wManufacturerCode;

	void* ptr = memcpy(m_szProductId, other.m_szProductId, VENDOR_PRODUCT_ID_LEN);
	ptr = memcpy(m_szVersionId, other.m_szVersionId, VENDOR_VERSION_ID_LEN);

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
BOOL operator==(const CVendorInfo& first, const CVendorInfo& second)
{
	if( first.m_bCountryCode != second.m_bCountryCode )
		return FALSE;
	if( first.m_bT35Extension != second.m_bT35Extension )
		return FALSE;
	if( first.m_wManufacturerCode != second.m_wManufacturerCode )
		return FALSE;
	if( 0 != strcmp(first.m_szProductId, second.m_szProductId) )
		return FALSE;
	if( 0 != strcmp(first.m_szVersionId, second.m_szVersionId) )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::SetCountryCode(const BYTE code)
{
	m_bCountryCode = code;
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::SetT35Extension(const BYTE code)
{
	m_bT35Extension = code;
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::SetManufacturerCode(const WORD code)
{
	m_wManufacturerCode = code;
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::SetProductId(const char* pszProduct)
{
	if( pszProduct != NULL )
	{		
		char* ptr = strncpy(m_szProductId, pszProduct, VENDOR_PRODUCT_ID_LEN - 1);
		m_szProductId[VENDOR_PRODUCT_ID_LEN - 1] = '\0';
	}
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::SetVersionId(const char* pszVersion)
{
	if( pszVersion != NULL )
	{
		char* ptr = strncpy(m_szVersionId, pszVersion, VENDOR_VERSION_ID_LEN - 1);
		m_szVersionId[VENDOR_VERSION_ID_LEN - 1] = '\0';
	}
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::ModeEndpointsSim()
{
	m_bCountryCode = 181;
	m_bT35Extension = 0;
	m_wManufacturerCode = 9009;
	strcpy(m_szProductId, "EndpointsSim");
	strcpy(m_szVersionId, "version 2.0");
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::ModeFX()
{
	m_bCountryCode = 181;
	m_bT35Extension = 0;
	m_wManufacturerCode = 9009;
	strcpy(m_szProductId, "ViewStation FX");
	strcpy(m_szVersionId, "Release 6.0.5");
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::ModeMXP()
{
	m_bCountryCode = 130;
	m_bT35Extension = 1;
	m_wManufacturerCode = 256;
	strcpy(m_szProductId,"Tandberg MXP");
	strcpy(m_szVersionId,"67");
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::ModeVSX7000()
{
	m_bCountryCode = 181;
	m_bT35Extension = 0;
	m_wManufacturerCode = 9009;
	strcpy(m_szProductId, "VSX 7000");
	strcpy(m_szVersionId, "Release 8.0.3");
}

/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::ModeIPSoftPhone()
{
	m_bCountryCode = 181;
	m_bT35Extension = 0;
	m_wManufacturerCode = 19540;
	strcpy(m_szProductId, "IP_Soft");
	strcpy(m_szVersionId, "5.2438");
}
/////////////////////////////////////////////////////////////////////////////
void CVendorInfo::ModeRPX()
{
	m_bCountryCode = 181;
	m_bT35Extension = 0;
	m_wManufacturerCode = 9009;
	strcpy(m_szProductId, "Polycom RPX");
	strcpy(m_szVersionId, "RPX version 2.0.3.7");
}
/////////////////////////////////////////////////////////////////////////////
