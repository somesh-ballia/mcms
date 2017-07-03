// IpServiceListManager: Manages Ip service list in the ConfParty process
//
//////////////////////////////////////////////////////////////////////
//#ifndef IpServiceListManager_H
//#define IpServiceListManager_H
#if !defined(_IpServiceListManager_H__)
#define _IpServiceListManager_H__


#include "ConfIpParameters.h"
#include "InterfaceType.h"
#include <vector>

//template class std::vector < CConfIpParameters*> ;
typedef std::vector< CConfIpParameters*> VECTOR_OF_CONF_IP_PARAMS;

class CIpServiceListManager:public CPObject
{
	CLASS_TYPE_1(CIpServiceListManager,CPObject)

public:
  typedef VECTOR_OF_CONF_IP_PARAMS::iterator VectIterator;
  
	CIpServiceListManager();
	~CIpServiceListManager();
	void Clear();
	WORD numberOfIpServices();
	virtual const char* NameOf() const { return "CIpServiceListManager";}
	STATUS insertIpService(CConfIpParameters* pConfIpParameters);
	STATUS updateIpService(CConfIpParameters* pConfIpParameters);
	CConfIpParameters* removeIpService(DWORD serviceID);
	CConfIpParameters* FindIpService(DWORD serviceID);
	CConfIpParameters* FindServiceByName( const char* serviceName);
	CConfIpParameters* FindServiceByIPAddress( const mcTransportAddress IPAddress);
	CConfIpParameters* FindServiceBySIPDomain(const char* pDestAddr);
	CConfIpParameters* GetFirstServiceInList();
	CConfIpParameters* GetRelevantService(const char* serviceName, BYTE netInterfaceType /*= H323_INTERFACE_TYPE*/);
	const char* FindServiceAndGetStringWithoutPrefix( const char* prefixPlusString, WORD prefixType, BOOL* pServiceMatched = NULL);
	
	VectIterator VectBegin(){return m_ipConfParamsVector->begin();}
	VectIterator VectEnd(){return m_ipConfParamsVector->end();}
	
	void SetDefaultIpServiceType(const char* defaultH323serv, const char* defaultSIPserv);
	//WORD  at(WORD index);
	//void  Dump();
	
protected:
	
	VECTOR_OF_CONF_IP_PARAMS*  m_ipConfParamsVector;
	
	
};

	

#endif // IpServiceListManager  
