#ifndef MCMSDAEMONAPI_H_
#define MCMSDAEMONAPI_H_

#include <string>
using namespace std;

#include "TaskApi.h"




class CMcmsDaemonApi : public CTaskApi
{
public:
	CMcmsDaemonApi();
	virtual ~CMcmsDaemonApi() {}


	virtual const char* NameOf() const { return "CMcmsDaemonApi";}
    STATUS SendResetReq(const string & desc);
    STATUS SendResetExternalReq(const string & desc);
    STATUS SendResetProcessReq(eProcessType processType);
    STATUS SendConfigApacheInd();
};

#endif /*MCMSDAEMONAPI_H_*/
