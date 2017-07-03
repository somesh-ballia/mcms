#ifndef WRAPPERSMCUMNGR_H_
#define WRAPPERSMCUMNGR_H_


#include "WrappersCSBase.h"
#include "AllocateStructs.h"


struct CSMNGR_LICENSING_S;
struct DNS_HOST_REGISTRATION_S;
struct GK_LICENSING_S;



/*-----------------------------------------------------------------------------
	class CNumOfPortsIndWrapper
-----------------------------------------------------------------------------*/
class CNumOfPortsIndWrapper : public CBaseWrapper
{
public:
	CNumOfPortsIndWrapper(const CSMNGR_LICENSING_S &data);
	virtual ~CNumOfPortsIndWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CNumOfPortsIndWrapper";}
	
private:
	const CSMNGR_LICENSING_S &m_Data;		
};










/*-----------------------------------------------------------------------------
	class CDnsHostRegistrationIndWrapper
-----------------------------------------------------------------------------*/
class CDnsHostRegistrationIndWrapper : public CBaseWrapper
{
public:
	CDnsHostRegistrationIndWrapper(const DNS_HOST_REGISTRATION_S &data);
	virtual ~CDnsHostRegistrationIndWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CDnsHostRegistrationIndWrapper";}
	
private:
	const DNS_HOST_REGISTRATION_S &m_Data;		
};

/*class CIPV4Wrapper : public CBaseWrapper
{
public:
	CIPV4Wrapper(ipAddressV4If &data);
	virtual ~CIPV4Wrapper();

	virtual const char*  NameOf() const {return "CIPV4Wrapper";}
	virtual void Dump(std::ostream&) const;

	void NullData();
	void CopyData(ipAddressV4If &Data);

private:
	ipAddressV4If &m_Data;
};


class CIPV6AraryWrapper : public CBaseWrapper
{
public:
	CIPV6AraryWrapper(ipv6AddressArray &data);
	virtual ~CIPV6AraryWrapper();

	virtual const char*  NameOf() const {return "CIPV6AraryWrapper";}
	virtual void Dump(std::ostream&) const;

	void CopyData(const ipv6AddressArray& data);

private:
	ipv6AddressArray &m_Data;
};*/




/*-----------------------------------------------------------------------------
	class CGKLicensingWrapper
-----------------------------------------------------------------------------*/
class CGKLicensingWrapper : public CBaseWrapper
{
public:
	CGKLicensingWrapper(const GK_LICENSING_S &data);
	virtual ~CGKLicensingWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CGKLicensingWrapper";}
	
private:
	const GK_LICENSING_S &m_Data;		
};


class CUDPMcuMngrPerPQWrapper : public CBaseWrapper
{
public:
	CUDPMcuMngrPerPQWrapper(const IP_SERVICE_UDP_MCUMNGR_PER_PQ_S &data);
	virtual ~CUDPMcuMngrPerPQWrapper();

	virtual void Dump(std::ostream&) const;

private:
	const IP_SERVICE_UDP_MCUMNGR_PER_PQ_S &m_Data;
};




class CUDPMcuMngrWrapper : public CBaseWrapper
{
public:
	CUDPMcuMngrWrapper(const IP_SERVICE_MCUMNGR_S &data);
	virtual ~CUDPMcuMngrWrapper();

	virtual void Dump(std::ostream&) const;

private:
	const IP_SERVICE_MCUMNGR_S &m_Data;
};






#endif /*WRAPPERSMCUMNGR_H_*/
