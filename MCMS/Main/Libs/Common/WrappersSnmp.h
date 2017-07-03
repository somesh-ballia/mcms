#ifndef WRAPPERSSNMP_H_
#define WRAPPERSSNMP_H_

#include "WrappersCSBase.h"

#include "SNMPStructs.h"

struct SNMP_MMGMNT_INFO_S;
struct SNMP_CS_INFO_S;
struct SNMP_CARDS_INFO_S;




/*-----------------------------------------------------------------------------
	class CSnmpMngmntInfoWrapper
-----------------------------------------------------------------------------*/
class CSnmpMngmntInfoWrapper : public CBaseWrapper
{	
public:
	CSnmpMngmntInfoWrapper(const SNMP_MMGMNT_INFO_S &data);
	virtual ~CSnmpMngmntInfoWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSnmpMngmntInfoWrapper";}

private:
	const SNMP_MMGMNT_INFO_S &m_Data;		
};








/*-----------------------------------------------------------------------------
	class CSnmpCSInfoWrapper
-----------------------------------------------------------------------------*/
class CSnmpCSInfoWrapper : public CBaseWrapper
{	
public:
	CSnmpCSInfoWrapper(const SNMP_CS_INFO_S &data);
	virtual ~CSnmpCSInfoWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSnmpCSInfoWrapper";}

private:
	const SNMP_CS_INFO_S &m_Data;		
};










/*-----------------------------------------------------------------------------
	class CSnmpCardsInfoWrapper
-----------------------------------------------------------------------------*/
class CSnmpCardsInfoWrapper : public CBaseWrapper
{	
public:
	CSnmpCardsInfoWrapper(const SNMP_CARDS_INFO_S &data);
	virtual ~CSnmpCardsInfoWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSnmpCardsInfoWrapper";}

private:
	const SNMP_CARDS_INFO_S &m_Data;		
};


/*-----------------------------------------------------------------------------
	class CSnmpLinkStatusWrapper
-----------------------------------------------------------------------------*/



class CSnmpLinkStatusWrapper : public CBaseWrapper
{
public:
	CSnmpLinkStatusWrapper(const LINK_STATUS_S &data);
	virtual ~CSnmpLinkStatusWrapper();

	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSnmpLinkStatusWrapper";}

private:
	const LINK_STATUS_S &m_Data;
};


#endif /*WRAPPERSSNMP_H_*/
