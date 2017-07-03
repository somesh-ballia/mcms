#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#include "MMIceMplMsgSender.h"
#include "ConnectionsTask.h"
#include "MplMcmsProtocol.h"


MMIceMplMsgSender::MMIceMplMsgSender(const CConnectionsTask &module)
	:m_ConnTask(module) {};

void MMIceMplMsgSender::SendForMplApi(CMplMcmsProtocol & rMplProtocol) const
{
	m_ConnTask.SendToGideonSimForMplApi(rMplProtocol);
};

#endif	//__DISABLE_ICE__
