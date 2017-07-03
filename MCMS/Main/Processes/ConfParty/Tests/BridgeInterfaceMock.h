//+========================================================================+
//                   BridgeInterfaceMock.h                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgeInterfaceMock.h	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+


#ifndef _BRIDGE_INTERFACE_MOCK_H__
#define _BRIDGE_INTERFACE_MOCK_H__

#include "BridgeInterface.h"
#include "BridgeMock.h"
#include "AudioBridge.h"
class CBridgeInterfaceMock : public CBridgeInterface
{
CLASS_TYPE_1(CBridgeInterfaceMock,CBridgeInterface)
public:
	virtual ~CBridgeInterfaceMock () {}
	CBridgeInterfaceMock () : CBridgeInterface() {}
	CBridgeInterfaceMock (const CBridgeInterfaceMock& rBridgeInterface): CBridgeInterface(rBridgeInterface) {}
	CBridgeInterfaceMock&	operator= (const CBridgeInterfaceMock& rOther) { (CBridgeInterface&)(*this) = (CBridgeInterface&)rOther; return *this; }

	virtual const char*	NameOf () const {return "CBridgeInterfaceMock";}
	
	CBridge* GetBridgeImplementation() {return CBridgeInterface::GetBridgeImplementation();}
	
	void	HandleEvent(CSegment* pMsg) {}

	virtual void	CreateImplementation(const CBridgeInitParams* pBridgeInitParams)
	{
		if (m_pBridgeImplementation)
			POBJDELETE(m_pBridgeImplementation);
	
		switch(pBridgeInitParams->GetBridgeImplementationType())
		{
		case eAudio_Bridge_V1:  
			  {
				  // temporary... audio bridge new to be called
				  //CAudioBridgeV1* pAudioBridgeV1 = new CAudioBridgeV1;
				  m_pBridgeImplementation = new CBridge; //CAudioBridge;
				  //m_pBridgeImplementation = new CBridge;
				  break;
			  }
		default: 
			{
				PASSERT(1);
				return;
			}
	
		}
	
		m_pBridgeImplementation->Create(pBridgeInitParams);
	}
  
};

#endif /* _BRIDGE_INTERFACE_MOCK_H__ */
