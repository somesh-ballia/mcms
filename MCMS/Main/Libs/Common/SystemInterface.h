
//+========================================================================+
//                                                             |
// FILE:      SystemInterface.h                                                   |                                              |
// PROGRAMMER: Judith Maman                                                      |
//-------------------------------------------------------------------------|
// Who | Date      | Description                                           |
//-------------------------------------------------------------------------|
//     | 11.9.12   | created                                               |
//+========================================================================+

#ifndef __SYSTEM_INTERFACE_H_
#define __SYSTEM_INTERFACE_H_

#include <vector>
#include "SerializeObject.h"
#include "psosxml.h"

//temp - check what needed
/*#include "StringsMaps.h"
#include "DefinesIpService.h"
#include "InitCommonStrings.h"
#include "ObjString.h"
#include "McuMngrStructs.h"
#include "DataTypes.h"
#include "H323Alias.h"
#include "DynIPSProperties.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"
#include "AllocateStructs.h"
#include "ServiceConfig.h"*/


//#define MAX_NUMBER_OF_INTERFACES 10

class CIPSpan;

//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class IpService                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////
class CSystemInterface : public CPObject
{
CLASS_TYPE_1(CSystemInterface, CPObject)
public:
	   //Constructors
	CSystemInterface();
	CSystemInterface( const CSystemInterface &other );
	CSystemInterface&  operator=( const CSystemInterface& other );
    bool operator==(const CSystemInterface &rHnd)const;
    bool operator!=(const CSystemInterface& other)const;

    virtual ~CSystemInterface();

    virtual const char* NameOf() const { return "CSystemInterface";}

// Implementation
    void   SerializeXml(CXMLDOMElement *pFatherNode);
    int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

    void SetSystemInterfaceName(std::string interface);
    void SetSystemInterfaceIp(DWORD interface_ip);
    void SetSystemIpType(DWORD ip_type);
    void SetSystemInterfaceIpv6_global(std::string strIpv6_global);
    void SetSystemInterfaceIpv6_site(std::string strIpv6_site);
    void SetSystemInterfaceIpv6_link(std::string strIpv6_link);
    void SetSystemInterfaceType(std::string interfaceType);
    void SetSystemSubNetMask(DWORD subnet_mask);

    std::string GetSystemInterfaceName() { return m_InterfaceName; }
    std::string GetSystemInterfaceType() { return m_InterfaceType; }
    DWORD GetSystemInterfaceIp() { return m_InterfaceIp; }
    DWORD GetSystemSubNetMask() { return m_SubNetMask; }
    std::string GetSystemInterfaceIpv6_global() { return m_IPv6_global; }
    std::string GetSystemInterfaceIpv6_site() { return m_IPv6_site; }
    std::string GetSystemInterfaceIpv6_link() { return m_IPv6_link; }



private:
	std::string m_InterfaceName;
	std::string m_InterfaceType;
	DWORD m_InterfaceIp;
	DWORD m_SubNetMask;
	DWORD m_IpType;
	std::string m_IPv6_global;
	std::string m_IPv6_site;
	std::string m_IPv6_link;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class CSystemInterfaceList                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////
class CSystemInterfaceList : public CSerializeObject
{
CLASS_TYPE_1(CSystemInterfaceList,CSerializeObject)
public:
	   //Constructors
	CSystemInterfaceList();
	CSystemInterfaceList( const CSystemInterfaceList &other );
 	CSystemInterfaceList&  operator=( const CSystemInterfaceList& other );
    
    virtual ~CSystemInterfaceList() ;
    virtual const char* NameOf() const { return "CSystemInterfaceList";}

	void    SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int     DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	CSerializeObject* Clone(){return new CSystemInterfaceList;}

	void  Add(DWORD interface_ip, DWORD subnet_mask, std::string interface,
				std::string iPv6_gloabl_str, std::string iPv6_site_str, std::string iPv6_link_str, std::string interfaceType="Ethernet");
	CSystemInterface* GetSystemInterfaceByName(std::string interface);
	bool GetInterfacesFromSystem();
	bool IsContainSysInterfaces() { return m_bHasGottenInterfaces; }
    CSystemInterface* GetFirstAvailableInterface();

	void Dump();
	void CleanUpList();
	STATUS ValidateMFWSPANField(CIPSpan* pIpSpan, eIpType  ipType);

	std::string GetValidatedResult() { return m_strValidatedResult; }
protected:

	bool GetInterfaceTypeFromSystem(CSystemInterface* pInterface);
	void GetInterfaceIpv6FromSystem(CSystemInterface* pInterface);

private:
	   // Attributes
	//CSystemInterface*    m_pSystemInterface[MAX_NUMBER_OF_INTERFACES];
	//int m_number_of_interfaces;
	std::vector<CSystemInterface*> m_vSystemInterfaces;
	bool m_bHasGottenInterfaces;

	std::string m_strValidatedResult;
};


#endif // __SYSTEM_INTERFACE_H_
