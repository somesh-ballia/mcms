

#ifndef _AUDIO_BRIDGE_INTERFACE_MOCK_H__
#define _AUDIO_BRIDGE_INTERFACE_MOCK_H__

#include "AudioBridgeInterface.h"
#include "Bridge.h"

class CAudioBridgeInterfaceMock : public CAudioBridgeInterface
{
public:
	CAudioBridgeInterfaceMock(){}
	virtual ~CAudioBridgeInterfaceMock (){}
	CAudioBridge*	MockGetBridgeImplementation(){ return GetBridgeImplementation();}
};

#endif /* _AUDIO_BRIDGE_INTERFACE_MOCK_H__*/

