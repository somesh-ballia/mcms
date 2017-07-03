//+========================================================================+
//                            CsHeader.H                                   |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       CsHeader.h                                                  |
// SUBSYSTEM:  CS                                                          |
// PROGRAMMER: Guy D,													   |
// Date : 20/4/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        
/*

This header file contains information regarding all ipAddr + general inner headers related to 
ConfParty + GK + CS entities within the Carmel.
In this document we set the common header structures.

*/

#ifndef __CSHEADER_H__
#define __CSHEADER_H__

#include "DataTypes.h"

/******* XML Interface **********/

typedef struct
{
	APIU16			dynamicType;		// Enc, G711, H263 Etc.
	APIU16			dynamicLength;		// Dynamic part length.
} xmlDynamicHeader;

typedef struct
{
	APIU16			numberOfDynamicParts;
	APIU16			sizeOfAllDynamicParts;
}xmlDynamicProperties;

typedef struct 
{
	int		unionType;
	int		unionSize;

} xmlUnionPropertiesSt;

typedef enum {
	kBalancer		= 1,
	kConfigurator	= 2,
	kServiceManager = 3,
}CsEntity;

#endif
