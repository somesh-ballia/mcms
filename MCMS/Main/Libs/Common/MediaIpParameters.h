// MediaIpParameters.h: interface for the CMediaIpParameters class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MediaIpParameters_H_
#define _MediaIpParameters_H_


#include "IpParameters.h"


//------------------------------------------------------------------
//              CMediaIpParameters  
//------------------------------------------------------------------
class CMediaIpParameters : public CPObject
{

CLASS_TYPE_1(CMediaIpParameters, CPObject)

public:
	CMediaIpParameters ();
	virtual const char* NameOf() const { return "CMediaIpParameters";}
	CMediaIpParameters (const CMediaIpParameters& rOther);
	CMediaIpParameters (const MEDIA_IP_PARAMS_S  mediaIpParams);
	virtual ~CMediaIpParameters ();
	virtual void Dump(std::ostream& msg) const;

	CMediaIpParameters& operator = (const CMediaIpParameters &rOther);
	friend WORD operator==(const CMediaIpParameters& lhs,const CMediaIpParameters& rhs);    
	friend bool operator<(const CMediaIpParameters& lhs,const CMediaIpParameters& rhs);    

	MEDIA_IP_PARAMS_S*  GetMediaIpParamsStruct ();
	
	IP_PARAMS_S&        GetIpParams ();
	void                SetIpParams (const IP_PARAMS_S *ipParams);

	void                SetServiceId(DWORD id);
	DWORD               GetServiceId();

	BYTE*               GetServiceName ();
	void                SetServiceName (const BYTE* name);

	void                SetBoardId(DWORD id);
	DWORD               GetBoardId();

	void                SetSubBoardId(DWORD id);
	DWORD               GetSubBoardId();
	
	void				SetIpTypeInAllInterfaces(eIpType theType);
	void				SetIpV6ConfigurationTypeInAllInterfaces(eV6ConfigurationType theType);


	void                SetMediaIpParamsPlatformType(ePlatformType theType);
	ePlatformType       GetMediaIpParamsPlatformType();

	void                SetData(const char *data);
	void                SetData(MEDIA_IP_PARAMS_S &mediaIpParams);
	void                SetData(CS_MEDIA_IP_PARAMS_S &csMediaIpParams);

	void                SetVlan(DWORD vlan);
	void                SetMask(DWORD mask);
	void                SetDefGW(DWORD defgw);



protected:
	MEDIA_IP_PARAMS_S   m_mediaIpParamsStruct;

	DWORD               m_boardId;
	DWORD               m_subBoardId;
};



//------------------------------------------------------------------
//              CCsMediaIpParameters  
//------------------------------------------------------------------
class CCsMediaIpParameters : public CPObject
{

CLASS_TYPE_1(CCsMediaIpParameters, CPObject)

public:
	CCsMediaIpParameters ();
	virtual const char* NameOf() const { return "CCsMediaIpParameters";}
	CCsMediaIpParameters (const CS_MEDIA_IP_PARAMS_S  *mediaIpParams);
	CCsMediaIpParameters (const CCsMediaIpParameters &other);
	virtual ~CCsMediaIpParameters ();
	virtual void Dump(std::ostream& msg) const;

	CCsMediaIpParameters& operator = (const CCsMediaIpParameters &rOther);
	friend WORD operator==(const CCsMediaIpParameters& lhs,const CCsMediaIpParameters& rhs);    
	friend bool operator<(const CCsMediaIpParameters& lhs,const CCsMediaIpParameters& rhs);    

	CS_MEDIA_IP_PARAMS_S*  GetMediaIpParamsStruct ();
	
	CS_IP_PARAMS_S&     GetIpParams ();
	void                SetIpParams (const CS_IP_PARAMS_S *ipParams);

	void                SetServiceId(DWORD id);
	DWORD               GetServiceId();

	BYTE*               GetServiceName ();
	void                SetServiceName (const BYTE* name);

	void                SetBoardId(DWORD id);
	DWORD               GetBoardId();

	void                SetSubBoardId(DWORD id);
	DWORD               GetSubBoardId();

	void                SetData(const char *data);
	void                SetData(CS_MEDIA_IP_PARAMS_S *mediaIpParams);


protected:
	CS_MEDIA_IP_PARAMS_S   m_mediaIpParamsStruct;

	DWORD               m_boardId;
	DWORD               m_subBoardId;
};


#endif // _MediaIpParameters_H_
