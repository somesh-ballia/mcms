#ifndef WRAPPERSCS_H_
#define WRAPPERSCS_H_

#include "WrappersCSBase.h"
#include "CsStructs.h"
#include "McuMngrInternalStructs.h"

#define NumOfComponentsOnGL1	4

/*-----------------------------------------------------------------------------
	class CVersionWrapper
-----------------------------------------------------------------------------*/
class CVersionWrapper : public CBaseWrapper
{
public:
	CVersionWrapper(const VERSION_S &data);
	virtual ~CVersionWrapper();

	const char * NameOf(void) const {return "CVersionWrapper";}
	
	virtual void Dump(std::ostream&) const;
	
private:
	const VERSION_S &m_Data;		
};






/*-----------------------------------------------------------------------------
	class CNewIndWrapper
-----------------------------------------------------------------------------*/
class CNewIndWrapper : public CBaseWrapper
{
public:
	CNewIndWrapper(const CS_New_Ind_S &data);
	virtual ~CNewIndWrapper();
	
	const char * NameOf(void) const {return "CNewIndWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_New_Ind_S &m_Data;	
};







/*-----------------------------------------------------------------------------
	class CNewReqWrapper
-----------------------------------------------------------------------------*/
class CNewReqWrapper : public CBaseWrapper
{
public:
	CNewReqWrapper(const CS_New_Req_S &data);
	virtual ~CNewReqWrapper();

	const char * NameOf(void) const {return "CNewReqWrapper";}
	
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_New_Req_S &m_Data;	
};








/*-----------------------------------------------------------------------------
	class CConfigReqWrapper
-----------------------------------------------------------------------------*/
class CConfigReqWrapper : public CBaseWrapper
{
public:
	CConfigReqWrapper(const CS_Config_Req_S &data);
	virtual ~CConfigReqWrapper();
	
	const char * NameOf(void) const {return "CConfigReqWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_Config_Req_S &m_Data;		
};



/*-----------------------------------------------------------------------------
	class CConfigIndWrapper
-----------------------------------------------------------------------------*/
class CConfigIndWrapper : public CBaseWrapper
{
public:
	CConfigIndWrapper(const CS_Config_Ind_S &data);
	virtual ~CConfigIndWrapper();
	
	const char * NameOf(void) const {return "CConfigIndWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_Config_Ind_S &m_Data;		
};







/*-----------------------------------------------------------------------------
	class CEndConfigReqWrapper
-----------------------------------------------------------------------------*/
class CEndConfigReqWrapper : public CBaseWrapper
{
public:
	CEndConfigReqWrapper(const CS_End_Config_Req_S &data);
	virtual ~CEndConfigReqWrapper();
	
	const char * NameOf(void) const {return "CEndConfigReqWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_End_Config_Req_S &m_Data;		
};









/*-----------------------------------------------------------------------------
	class CEndConfigIndWrapper
-----------------------------------------------------------------------------*/
class CEndConfigIndWrapper : public CBaseWrapper
{
public:
	CEndConfigIndWrapper(const CS_End_Config_Ind_S &data);
	virtual ~CEndConfigIndWrapper();
	
	const char * NameOf(void) const {return "CEndConfigIndWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_End_Config_Ind_S &m_Data;		
};









/*-----------------------------------------------------------------------------
	class CLanCfgReqWrapper
-----------------------------------------------------------------------------*/
class CLanCfgReqWrapper : public CBaseWrapper
{
public:
	CLanCfgReqWrapper(const CS_Lan_Cfg_Req_S &data);
	virtual ~CLanCfgReqWrapper();
	
	const char * NameOf(void) const {return "CLanCfgReqWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_Lan_Cfg_Req_S &m_Data;		
};










/*-----------------------------------------------------------------------------
	class CEndStartupIndWrapper
-----------------------------------------------------------------------------*/
class CEndStartupIndWrapper : public CBaseWrapper
{
public:
	CEndStartupIndWrapper(const CS_End_StartUp_Ind_S &data);
	virtual ~CEndStartupIndWrapper();
	
	const char * NameOf(void) const {return "CEndStartupIndWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_End_StartUp_Ind_S &m_Data;		
};


























/*-----------------------------------------------------------------------------
	class CReconnectReqWrapper
-----------------------------------------------------------------------------*/
class CReconnectReqWrapper : public CBaseWrapper
{
public:
	CReconnectReqWrapper(const CS_Reconnect_Req_S &data);
	virtual ~CReconnectReqWrapper();
	
	const char * NameOf(void) const {return "CReconnectReqWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_Reconnect_Req_S &m_Data;		
};












/*-----------------------------------------------------------------------------
	class CReconnectIndWrapper
-----------------------------------------------------------------------------*/
class CReconnectIndWrapper : public CBaseWrapper
{
public:
	CReconnectIndWrapper(const CS_Reconnect_Ind_S &data);
	virtual ~CReconnectIndWrapper();
	
	const char * NameOf(void) const {return "CReconnectIndWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_Reconnect_Ind_S &m_Data;		
};






















/*-----------------------------------------------------------------------------
	class CCSBufferIndWrapper
-----------------------------------------------------------------------------*/
class CCSBufferIndWrapper : public CBaseWrapper
{
public:
	CCSBufferIndWrapper(const char *data, const int dataLen);
	virtual ~CCSBufferIndWrapper();
	
	const char * NameOf(void) const {return "CCSBufferIndWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	char *m_Data;		
};









/*-----------------------------------------------------------------------------
	class CCSKeepAliveIndWrapper
-----------------------------------------------------------------------------*/
class CCSKeepAliveIndWrapper : public CBaseWrapper
{
public:
	CCSKeepAliveIndWrapper(const csKeepAliveSt &data);
	virtual ~CCSKeepAliveIndWrapper();
	
	const char * NameOf(void) const {return "CCSKeepAliveIndWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const csKeepAliveSt &m_Data;		
};



/*-----------------------------------------------------------------------------
	class CPingReqWrapper
-----------------------------------------------------------------------------*/
class CPingReqWrapper : public CBaseWrapper
{
public:
	CPingReqWrapper (const CS_Ping_req_S &data);
	virtual ~CPingReqWrapper();
	
	const char * NameOf(void) const {return "CPingReqWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_Ping_req_S &m_Data;		
};



/*-----------------------------------------------------------------------------
	class CPingReqWrapper
-----------------------------------------------------------------------------*/
class CPingIndWrapper : public CBaseWrapper
{
public:
	CPingIndWrapper (const CS_Ping_ind_S &data);
	virtual ~CPingIndWrapper();
	
	const char * NameOf(void) const {return "CPingIndWrapper";}
	virtual void Dump(std::ostream&) const;
	
private:
	const CS_Ping_ind_S &m_Data;		
};

#endif /*WRAPPERSCS_H_*/
