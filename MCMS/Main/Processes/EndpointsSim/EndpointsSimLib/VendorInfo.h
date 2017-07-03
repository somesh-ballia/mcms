//+========================================================================+
//                     VendorInfo.h  						               |
//				Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.                    |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VendorInfo.h												   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef __VENDORINFO_H__
#define __VENDORINFO_H__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
#include "PObject.h"


//////////////////////////////////////////////////////////////////////////
class CVendorInfo : public CPObject
{
	CLASS_TYPE_1(CVendorInfo,CPObject)

public:
	static const WORD VENDOR_PRODUCT_ID_LEN = 256;
	static const WORD VENDOR_VERSION_ID_LEN = 256;
	virtual const char* NameOf() const { return "CVendorInfo";}

public:
		// constructors
	CVendorInfo();
	CVendorInfo(const char* pszManufacturerName);
	virtual ~CVendorInfo();

		// overrides

		// utils
	CVendorInfo& operator=(const CVendorInfo& other);
    friend BOOL operator==(const CVendorInfo& first, const CVendorInfo& second);

	// predefined sets of vendor details
	void ModeEndpointsSim();
	void ModeFX();
	void ModeVSX7000();
	void ModeIPSoftPhone();
	void ModeMXP();
	void ModeRPX();

	BYTE GetCountryCode() const   { return m_bCountryCode; }
	BYTE GetT35Extension() const  { return m_bT35Extension; }
	WORD GetManufacturerCode() const  { return m_wManufacturerCode; }

	const char* GetProductId() const   { return m_szProductId; }
	const char* GetVersionId() const   { return m_szVersionId; }
	
	void SetCountryCode(const BYTE code);
	void SetT35Extension(const BYTE code);
	void SetManufacturerCode(const WORD code);
	void SetProductId(const char* pszProduct);
	void SetVersionId(const char* pszVersion);

protected:
	BYTE	m_bCountryCode;
	BYTE	m_bT35Extension;
	WORD	m_wManufacturerCode;
	
	char m_szProductId[VENDOR_PRODUCT_ID_LEN];
	char m_szVersionId[VENDOR_VERSION_ID_LEN];
};


#endif // __VENDORINFO_H__ 





