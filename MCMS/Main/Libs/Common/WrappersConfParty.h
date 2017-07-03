#ifndef WRAPERSCONFPARTY_H_
#define WRAPERSCONFPARTY_H_

#include "WrappersCSBase.h"
#include "ConfStructs.h"




/*-----------------------------------------------------------------------------
	class CBaseSipServerWrapper
-----------------------------------------------------------------------------*/
class CBaseSipServerWrapper : public CBaseWrapper
{	
public:
	CBaseSipServerWrapper(const BASE_SIP_SERVER_S &data);
	virtual ~CBaseSipServerWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CBaseSipServerWrapper";}

private:
	const BASE_SIP_SERVER_S &m_Data;		
};





/*-----------------------------------------------------------------------------
	class CSipServerWrapper
-----------------------------------------------------------------------------*/
class CSipServerWrapper : public CBaseWrapper
{
public:
	CSipServerWrapper(const SIP_SERVER_S &data);
	virtual ~CSipServerWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSipServerWrapper";}

private:
	const SIP_SERVER_S &m_Data;		
};





/*-----------------------------------------------------------------------------
	class CQosWrapper
-----------------------------------------------------------------------------*/
class CQosWrapper : public CBaseWrapper
{	
public:
	CQosWrapper(const QOS_S &data);
	virtual ~CQosWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CQosWrapper";}

private:
	const QOS_S &m_Data;	
};





/*-----------------------------------------------------------------------------
	class CSipWrapper
-----------------------------------------------------------------------------*/
class CSipWrapper : public CBaseWrapper
{
public:
	CSipWrapper(const SIP_S &data);
	virtual ~CSipWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSipWrapper";}

private:
	const SIP_S &m_Data;		
};










/*-----------------------------------------------------------------------------
	class CConfIpParamWrapper
-----------------------------------------------------------------------------*/
class CConfIpParamWrapper : public CBaseWrapper
{	
public:
	CConfIpParamWrapper(const CONF_IP_PARAMS_S &data);
	virtual ~CConfIpParamWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CConfIpParamWrapper";}

private:
	const CONF_IP_PARAMS_S &m_Data;	
};

#endif /*WRAPERSCONFPARTY_H_*/
