// MediaIpConfig.h: interface for the CMediaIpConfig class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MediaIpConfig_H_
#define _MediaIpConfig_H_


#include "PObject.h"
#include "CardsStructs.h"



class CMediaIpConfig : public CPObject
{

CLASS_TYPE_1(CMediaIpConfig, CPObject)

public:
	CMediaIpConfig ();
	virtual const char* NameOf() const { return "CMediaIpConfig";}
	CMediaIpConfig (const CMediaIpConfig& rOther);
	virtual ~CMediaIpConfig ();
	virtual void Dump(std::ostream& msg) const;

	CMediaIpConfig& operator = (const CMediaIpConfig &other);
	friend WORD operator==(const CMediaIpConfig& lhs,const CMediaIpConfig& rhs);    
	friend bool operator<(const CMediaIpConfig& lhs,const CMediaIpConfig& rhs);    

	MEDIA_IP_CONFIG_S*  GetMediaIpConfigStruct ();

	eMediaIpConfigStatus  GetStatus ();
	void                  SetStatus (const eMediaIpConfigStatus status);

	DWORD  GetServiceId ();
	void   SetServiceId (const DWORD id);

	eIpType GetIpType ();
	void    SetIpType (const eIpType ipType);

	DWORD  GetIpV4Address ();
	void   SetIpV4Address (const DWORD address);

	void	GetIpV6Address(int idx, char* retStr);
	void	SetIpV6Address(const char* ipV6Address, int idx);

	BYTE   GetPqNumber ();
	void   SetPqNumber (const BYTE num);

    void   SetDefaultGatewayIPv6(const char* defaultGateway);
    char*  GetDefaultGatewayIPv6() const;

	void   SetData(const char *data);
	void   SetData(MEDIA_IP_CONFIG_S* mediaIpConfig);


protected:
	MEDIA_IP_CONFIG_S m_mediaIpConfigStruct;
};



#endif // _MediaIpConfig_H_
