#ifndef WRAPPERSCOMMON_H_
#define WRAPPERSCOMMON_H_

#include "WrappersCSBase.h"
#include "MplMcmsStructs.h"
#include "AuditDefines.h"
#include "AllocateStructs.h"


/*-----------------------------------------------------------------------------
	class CCommonHeaderWrapper
-----------------------------------------------------------------------------*/
class CCommonHeaderWrapper : public CBaseWrapper
{
public:
	CCommonHeaderWrapper(const COMMON_HEADER_S &data);
	virtual ~CCommonHeaderWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const COMMON_HEADER_S &m_Data;	
};






/*-----------------------------------------------------------------------------
	class CTraceHeaderWrapper
-----------------------------------------------------------------------------*/
class CTraceHeaderWrapper : public CBaseWrapper
{
public:
	CTraceHeaderWrapper(const TRACE_HEADER_S &data);
	virtual ~CTraceHeaderWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const TRACE_HEADER_S &m_Data;	
};




/*-----------------------------------------------------------------------------
	class CAuditHeaderWrapper
-----------------------------------------------------------------------------*/
class CAuditHeaderWrapper : public CBaseWrapper
{
public:
	CAuditHeaderWrapper(const AUDIT_EVENT_HEADER_S &data);
	virtual ~CAuditHeaderWrapper();
	
	virtual const char*  NameOf() const {return "CAuditHeaderWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const AUDIT_EVENT_HEADER_S &m_Data;	
};




/*-----------------------------------------------------------------------------
	class CTPKTHeaderWrapper
-----------------------------------------------------------------------------*/
class CTPKTHeaderWrapper : public CBaseWrapper
{
public:
	CTPKTHeaderWrapper(const TPKT_HEADER_S &data);
	virtual ~CTPKTHeaderWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const TPKT_HEADER_S &m_Data;	
};






class CIPV4Wrapper : public CBaseWrapper
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
};

#endif /*WRAPPERSCOMMON_H_*/
