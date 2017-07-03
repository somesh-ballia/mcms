#ifndef WRAPPERSGK_H_
#define WRAPPERSGK_H_

#include <string>

#include "WrappersCSBase.h"
#include "GKManagerStructs.h"



/*-----------------------------------------------------------------------------
	class CGkManagerServiceParamsIndStructWrapper
-----------------------------------------------------------------------------*/
class CGkManagerServiceParamsIndStructWrapper : public CBaseWrapper
{
public:
	CGkManagerServiceParamsIndStructWrapper(const GkManagerServiceParamsIndStruct &data);
	virtual ~CGkManagerServiceParamsIndStructWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CGkManagerServiceParamsIndStructWrapper";}
	
private:
	const GkManagerServiceParamsIndStruct &m_Data;	
};






/*-----------------------------------------------------------------------------
	class CGkManagerUpdateServicePropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
class CGkManagerUpdateServicePropertiesReqStructWrapper : public CBaseWrapper
{
public:
	CGkManagerUpdateServicePropertiesReqStructWrapper(const GkManagerUpdateServicePropertiesReqStruct &data);
	virtual ~CGkManagerUpdateServicePropertiesReqStructWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CGkManagerUpdateServicePropertiesReqStructWrapper";}
	
private:
	const GkManagerUpdateServicePropertiesReqStruct &m_Data;	
};






/*-----------------------------------------------------------------------------
	class CClearGkParamsFromPropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
class CClearGkParamsFromPropertiesReqStructWrapper : public CBaseWrapper
{
public:
	CClearGkParamsFromPropertiesReqStructWrapper(const ClearGkParamsFromPropertiesReqStruct &data);
	virtual ~CClearGkParamsFromPropertiesReqStructWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CClearGkParamsFromPropertiesReqStructWrapper";}
	
private:
	const ClearGkParamsFromPropertiesReqStruct &m_Data;	
};





/*-----------------------------------------------------------------------------
	class CSetAltGkNameInPropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
class CSetGkNameInPropertiesReqStructWrapper : public CBaseWrapper
{
public:
	CSetGkNameInPropertiesReqStructWrapper(const SetGkNameInPropertiesReqStruct &data);
	virtual ~CSetGkNameInPropertiesReqStructWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSetGkNameInPropertiesReqStructWrapper";}
	
private:
	const SetGkNameInPropertiesReqStruct &m_Data;	
};






/*-----------------------------------------------------------------------------
	class CSetAltGkIdInPropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
class CSetGkIdInPropertiesReqStructWrapper : public CBaseWrapper
{
public:
	CSetGkIdInPropertiesReqStructWrapper(const SetGkIdInPropertiesReqStruct &data);
	virtual ~CSetGkIdInPropertiesReqStructWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSetGkIdInPropertiesReqStructWrapper";}
	
private:
	const SetGkIdInPropertiesReqStruct &m_Data;	
};






/*-----------------------------------------------------------------------------
	class CSetGkIPInPropertiesReqStructWrapper
-----------------------------------------------------------------------------*/
class CSetGkIPInPropertiesReqStructWrapper : public CBaseWrapper
{
public:
	CSetGkIPInPropertiesReqStructWrapper(const SetGkIPInPropertiesReqStruct &data);
	virtual ~CSetGkIPInPropertiesReqStructWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSetGkIPInPropertiesReqStructWrapper";}
	
private:
	const SetGkIPInPropertiesReqStruct &m_Data;	
};







/*-----------------------------------------------------------------------------
	class CIpAddrStructWrapper
-----------------------------------------------------------------------------*/
class CIpAddrStructWrapper : public CBaseWrapper
{
public:
	CIpAddrStructWrapper(const ipAddressStruct &data, const std::string &title);
	virtual ~CIpAddrStructWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CIpAddrStructWrapper";}
	
private:
	const ipAddressStruct  &m_Data;
	std::string 			m_Title;
};





/*-----------------------------------------------------------------------------
	class CMngmntParamStructWrapper
-----------------------------------------------------------------------------*/
class CMngmntParamStructWrapper : public CBaseWrapper
{
public:
	CMngmntParamStructWrapper(const MngmntParamStruct &data);
	virtual ~CMngmntParamStructWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CMngmntParamStructWrapper";}
	
private:
	const MngmntParamStruct  &m_Data;
};


#endif /*WRAPPERSGK_H_*/
