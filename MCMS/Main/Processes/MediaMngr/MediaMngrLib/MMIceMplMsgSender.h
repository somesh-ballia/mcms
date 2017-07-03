//author: Victor

#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#ifndef _MM_ICE_MPL_MSG_SENDER_HXX_
#define _MM_ICE_MPL_MSG_SENDER_HXX_

#include "IceMplMsgSender.h"

class CMplMcmsProtocol;
class CConnectionsTask;


class MMIceMplMsgSender: public IceMplMsgSender
{
public:
	MMIceMplMsgSender(const CConnectionsTask &module);
	
	~MMIceMplMsgSender() {};

	void SendForMplApi(CMplMcmsProtocol & rMplProtocol) const;

private:
	const CConnectionsTask &m_ConnTask;
};

#endif
#endif	//__DISABLE_ICE__

