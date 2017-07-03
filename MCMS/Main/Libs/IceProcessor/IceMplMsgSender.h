//author: Victor


#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#ifndef _ICE_MPL_MSG_SENDER_HXX_
#define _ICE_MPL_MSG_SENDER_HXX_

class CMplMcmsProtocol;

class IceMplMsgSender
{
public:
	virtual ~IceMplMsgSender() {};

	virtual void SendForMplApi(CMplMcmsProtocol & rMplProtocol) const=0;	
};

#endif

#endif	//__DISABLE_ICE__
