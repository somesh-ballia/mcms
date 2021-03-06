// *********************************************************************************
// Copyright (C) 2011 Polycom Israel Ltd.
// This file contains confidential information proprietary to POLYCOM ISRAEL Ltd.
// The use or disclosure of any information contained In this file without the
// written consent of an officer of POLYCOM ISRAEL Ltd is expressly
// *********************************************************************************

// *********************************************************************************
// General Description:  File "802_1xApiDefinitions.h" 
// Generated By: inaveh      Date: 10/2012
// *********************************************************************************

#ifndef _802_1xAPI_IND_H_
#define _802_1xAPI_IND_H_

#include "802_1xApiDefinitions.h"

//the IND for the new conf req. when we finish the wpa_supplicant process we answer
//MCMS with this struct to indicate the status of the operation.
typedef struct {
	E_802_1xConnState eEth0En;
	E_802_1xConnState eEth1En;
	E_802_1xConnState eEth2En;
	E_802_1xConnState eEth3En;
} s802_1x_NEW_CONFIG_IND;

//the IND for the status query from MCMS regarding a specific NIC
typedef struct {
	E_802_1xEth ulNicId;
	APIU32 ulEnabled;
	E_802_1xSuppPortStatus eSuppPortStatus;
} s802_1x_CONNECTION_STATUS_IND;

//this is the unsolicited event we raise upon a change in 802.1x conn state during runtime
typedef struct {
	E_802_1xEth ulNicId;
	E_802_1xConnState eConnStatus;
} s802_1x_CONNECTION_STATUS_UNSOLICITED_IND;






#endif //_802_1xAPI_IND_H_
