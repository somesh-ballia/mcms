/*
 * MccfCdrPackageResponse.h
 *
 *  Created on: Dec 17, 2012
 *      Author: sshafrir
 */

#ifndef __MCCFCDRPACKAGERESPONSE_H_
#define __MCCFCDRPACKAGERESPONSE_H_


#include <string>
#include "MscPolycomMixer.h"
#include "EventElement.h"
#include "ParticipantInfoNotify.h"
#include "Channels.h"
#include "Channel.h"
#include "PartyMonitor.h"
#include "IpChannelDetails.h"

//#include "MscIvr.h"
//#include "Event.h"
//#include "MccfIvrDialogManager.h"
//#include "IvrPackageStatusCodes.h"

class CMccfCdrPackageResponse
{

public:

    static void ResponseReportMsg(const COsQueue clientRspMbx, CPrtMontrBaseParams* pIpVideoCdrChannelMonitor[],CIpChannelDetails* m_pIpVideoCdrChannelDetails[],
    		                      /*std::string partyConnection,const char* strConfName,*/ const char* confId, const char* partyId);

};


#endif /* _MCCFCDRPACKAGERESPONSE_H_ */
