//+========================================================================+
//                EndpointsSimConfigConfig.h                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EndpointsSimConfig.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __ENDPOINTSSIMCONFIG_
#define   __ENDPOINTSSIMCONFIG_


#include "SerializeObject.h"

#define		ENDPOINTS_SIM_CONFIG_FILE_NAME		"Cfg/EP_SIM.XML"


class  CXMLDOMElement;


class  CEndpointsSimSystemCfg  : public CSerializeObject
{
CLASS_TYPE_1(CEndpointsSimSystemCfg,CSerializeObject)
public:

				// Constructors
	CEndpointsSimSystemCfg();
	virtual const char* NameOf() const { return "CEndpointsSimSystemCfg";}
	~CEndpointsSimSystemCfg();
	CEndpointsSimSystemCfg(const CEndpointsSimSystemCfg& other);

				// Initializations

				// Operations
	CEndpointsSimSystemCfg& operator =(const CEndpointsSimSystemCfg& reference);
	virtual CSerializeObject* Clone() { return NULL; }

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	void Serialize(CSegment& rSegment) const;
	void DeSerialize(CSegment& rSegment);
	BOOL IsEqual(const CEndpointsSimSystemCfg& rOther) const;
	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(ENDPOINTS_SIM_CONFIG_FILE_NAME); }
	void WriteXmlFile() { CSerializeObject::WriteXmlFile(ENDPOINTS_SIM_CONFIG_FILE_NAME,"ENDPOINTS_SIM_CONFIGURATION"); }

	const char* GetCsApiIpAddress() const	{ return (const char*)m_szCsApiIp; }
	WORD  GetCsApiPortNumber() const		{ return m_wCsApiPortNumber; }
	WORD  GetGuiPortNumber() const			{ return m_wGuiPortNumber; }
	BOOL  GetEncryptionDialIn() const		{ return m_bEncryptionDialIn; }
	BOOL  GetEncryptionDialOut() const		{ return m_bEncryptionDialOut; }
	void  SetEncryptionDialOut(BOOL bEncryptionDialOut)		{ m_bEncryptionDialOut = bEncryptionDialOut; }

	DWORD GetErrorBitRate() const			{ return m_errorBitRate; } 
	void  SetErrorBitRate( DWORD errorBitRate )		{ m_errorBitRate = errorBitRate; } 	// in KB

	DWORD GetRejectStatus() const			{ return m_rejectStatus; }
	const char* GetRedirectionSipAddress() const	{ return (const char*)m_szRedirectionSipAddress; }

	BOOL  GetDeleteDialOutAfterDisconnect() const { return m_bDeleteDialOut; }
	BOOL  GetIsAvayaGatekeeper() const { return m_bIsAvayaGatekeeper; }
	DWORD GetGatekeeperBrqTime() const { return m_gkBrqTime; }
	
	void SetCSIpAddress(const char* szCSIpAddress) { strncpy(m_szCSIpAddress, szCSIpAddress , IP_ADDRESS_STR_LEN - 1); m_szCSIpAddress[IP_ADDRESS_STR_LEN - 1] = 0;}
	const char* GetCSIpAddress() const	{ return (const char*)m_szCSIpAddress; }
	
	void SetCSIpV6Address(const char* szCSIpAddress) { strncpy(m_szCSIpV6Address, szCSIpAddress , IPV6_ADDRESS_LEN - 1); m_szCSIpV6Address[IPV6_ADDRESS_LEN - 1] = 0;}
	const char* GetCSIpV6Address() const	{ return (const char*)m_szCSIpV6Address; }

protected:
				// Operations

				// Attributes
	// address & port number for Socket connection with CS-API
	char		m_szCsApiIp[IP_ADDRESS_STR_LEN];
	WORD		m_wCsApiPortNumber;
	// Listen port for GUI
	WORD		m_wGuiPortNumber;
	// Encryption for DialIn calls
	BOOL		m_bEncryptionDialIn;
	BOOL		m_bEncryptionDialOut;
	DWORD 		m_errorBitRate;
	DWORD		m_rejectStatus;
	char		m_szRedirectionSipAddress[H243_NAME_LEN];
	BOOL		m_bDeleteDialOut;
	BOOL		m_bIsAvayaGatekeeper;
	DWORD		m_gkBrqTime;
	
	char		m_szCSIpAddress[IP_ADDRESS_STR_LEN];
	char		m_szCSIpV6Address[IPV6_ADDRESS_LEN];
};




#endif /* __ENDPOINTSSIMCONFIG_ */

