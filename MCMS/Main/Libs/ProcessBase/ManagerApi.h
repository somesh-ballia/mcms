// ManagerApi.h: interface for the CManagerApi class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MANAGERAPI_H__)
#define _MANAGERAPI_H__



#include "TaskApi.h"

class CManagerApi : public CTaskApi  
{
CLASS_TYPE_1(CManagerApi,CTaskApi )
public:
	CManagerApi();
    CManagerApi(eProcessType process);
	virtual ~CManagerApi();
	virtual const char* NameOf() const { return "CManagerApi";}
    void InformHttpGetRequest(const std::string file_name);
    

};

#endif // !defined(_MANAGERAPI_H__)
