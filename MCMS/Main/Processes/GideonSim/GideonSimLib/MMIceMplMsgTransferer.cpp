#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#include "MMIceMplMsgTransferer.h"
#include "GideonSimBarakLogical.h"
#include "MplMcmsProtocol.h"


MMIceMplMsgTransferer::MMIceMplMsgTransferer(const CGideonSimBarakLogical &module)
	:m_GideonSim(module) {};

void MMIceMplMsgTransferer::SendForMplApi(CMplMcmsProtocol & rMplProtocol) const
{
	m_GideonSim.ForwardMsg2MediaMngr(rMplProtocol);
};

#endif	//__DISABLE_ICE__
