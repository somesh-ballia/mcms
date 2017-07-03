//+========================================================================+
//                            ResourceManagerApi.H                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                    All Rights Reserved.                                 |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       RESOURCEMANAGERAPI.H                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:			                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//+========================================================================+

#ifndef RESOURCEMANAGERAPI_H_
#define RESOURCEMANAGERAPI_H_

#include "ManagerApi.h"


class CResourceManagerApi : public CManagerApi
{
	CLASS_TYPE_1(CResourceManagerApi,CManagerApi )
public:

	CResourceManagerApi();
	virtual ~CResourceManagerApi();

	virtual const char*  NameOf() const;

};

#endif /* RESOURCEMANAGERAPI_H_ */
