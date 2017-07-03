#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif
#ifndef __DISABLE_ICE__

//author: Victor
#ifndef _GIDEONSIM_ICE_MPL_MSG_SENDER_HXX_
#define _GIDEONSIM_ICE_MPL_MSG_SENDER_HXX_

#include "IceMplMsgSender.h"

class CMplMcmsProtocol;
class CGideonSimBarakLogical;

class GideonSimIceMplMsgSender : public IceMplMsgSender
{
public:
	GideonSimIceMplMsgSender(const CGideonSimBarakLogical &module);
	~GideonSimIceMplMsgSender() {};

	void SendForMplApi(CMplMcmsProtocol & rMplProtocol) const;	

private:
	const CGideonSimBarakLogical &m_GideonSim;
};

#endif

#endif	//__DISABLE_ICE__

