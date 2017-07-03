#ifndef __MCCFIVRPACKAGERESPONSE_H_
#define __MCCFIVRPACKAGERESPONSE_H_

//////////////////////////////////////////////////////////////////////
#include "DataTypes.h"
#include "IvrPackageStatusCodes.h"

#include "Event.h"

#include <string>

//////////////////////////////////////////////////////////////////////
class ApiBaseObject;
class MscIvr;

class CSegment;

struct DialogState;

//////////////////////////////////////////////////////////////////////
class CMccfIvrPackageResponse
{
public:

	static MccfIvrErrorCodesEnum ResponseReportMsg(const DialogState& state, MccfIvrErrorCodesEnum status, bool isUpdateReport= false);
	static void                  ResponseControlMsg(const DialogState& state);
	static Event*                BuildControlMsg(MscIvr* mscIvr, const std::string& dialogID, unsigned int status);

private:

	static CSegment*             CreateMccfResponse(HANDLE appServerID, const ApiBaseObject& response, size_t timeout = 0,bool isUpdateReport = false, DWORD seqNum = 1);

private:

	CMccfIvrPackageResponse();
};

//////////////////////////////////////////////////////////////////////
#endif // __MCCFIVRPACKAGERESPONSE_H_
