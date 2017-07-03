#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif
#ifndef __DISABLE_ICE__
//author: Victor
#include "GideonSimIceMplMsgSender.h"
#include "GideonSimBarakLogical.h"
#include "MplMcmsProtocol.h"


GideonSimIceMplMsgSender::GideonSimIceMplMsgSender(const CGideonSimBarakLogical &module)
	:m_GideonSim(module)
{
}

void GideonSimIceMplMsgSender::SendForMplApi(CMplMcmsProtocol & rMplProtocol) const
{
	m_GideonSim.SendToCmForMplApi(rMplProtocol);
}

#endif	//__DISABLE_ICE__

