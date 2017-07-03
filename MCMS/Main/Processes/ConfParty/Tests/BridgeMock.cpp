//+========================================================================+
//                      BridgeMock.cpp                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgeMock.cpp	                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+



#include "BridgeMock.h"

#define MOCK_EVENT	1

PBEGIN_MESSAGE_MAP(CBridgeMock)
ONEVENT(MOCK_EVENT         ,ANYCASE         ,CStateMachine::NullActionFunction)
PEND_MESSAGE_MAP(CBridgeMock, CStateMachine);



