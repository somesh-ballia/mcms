//author: Victor
#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#ifndef _MM_ICE_MPL_MSG_TRANS_HXX_
#define _MM_ICE_MPL_MSG_TRANS_HXX_

#include "IceMplMsgSender.h"

class CMplMcmsProtocol;
class CGideonSimBarakLogical;


class MMIceMplMsgTransferer: public IceMplMsgSender
{
public:
	MMIceMplMsgTransferer(const CGideonSimBarakLogical &module);
	
	~MMIceMplMsgTransferer() {};

	void SendForMplApi(CMplMcmsProtocol & rMplProtocol) const;

private:
	const CGideonSimBarakLogical &m_GideonSim;
};

#endif

#endif	//__DISABLE_ICE__

