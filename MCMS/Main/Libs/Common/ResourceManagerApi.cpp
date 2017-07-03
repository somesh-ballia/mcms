//+========================================================================+
//                         ResourceManagerApi.cpp                                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                    All Rights Reserved.                                 |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ResourceManagerApi.cpp                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:		                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//+========================================================================+

#include "ResourceManagerApi.h"

/////////////////////////////////////////////////////////////////////////////
CResourceManagerApi::CResourceManagerApi() // constructor
        :CManagerApi(eProcessResource)
{
}

/////////////////////////////////////////////////////////////////////////////
CResourceManagerApi::~CResourceManagerApi() // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
const char* CResourceManagerApi::NameOf()  const
{
  return "CResourceManagerApi";
}

