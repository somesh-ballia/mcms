#ifndef WRAPPERSCARDS_H_
#define WRAPPERSCARDS_H_

#include "CardsStructs.h"
#include "WrappersCSBase.h"

/*-----------------------------------------------------------------------------
	class CMediaIpConfigWrapper
-----------------------------------------------------------------------------*/
class CMediaIpConfigWrapper : public CBaseWrapper
{	
public:
	CMediaIpConfigWrapper(const MEDIA_IP_CONFIG_S &data);
	virtual ~CMediaIpConfigWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CMediaIpConfigWrapper";}

private:
	const MEDIA_IP_CONFIG_S &m_Data;		
};


#endif /*WRAPPERSCARDS_H_*/
