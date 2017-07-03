/*
 * IpmcTest.cpp
 *
 *  Created on: Jul 2, 2010
 *      Author: pctc_liangd
 */

#include <iostream>
#include <string.h>
#include "DataTypes.h"
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include "DiagnosticsInfo.h"
#include "IpmcInt.h"


// CNTL IPMC connection test implementation
void CNTLIPMCConntionTest(dgnsTestResult *diagTestRes)
{
	INT32 slotId;

	slotId = IpmcGetHWSlotId();

	if (slotId != -1 ) {
		diagTestRes->testResult = eStatOk;
	} else {
		diagTestRes->testResult = eStatFail;
		sprintf(diagTestRes->errString, "Fail to get the slot ID through IPMC\n");
	}
	return;

}
