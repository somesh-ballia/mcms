// ConfigParamService.h: interface for the CCommService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Base class for all classes who need to communicate with other processes
// 								within MCMS and outside modules like CS Module
//========   ==============   =====================================================================

#if !defined(__COMMSERVICE_H__)
#define __COMMSERVICE_H__


#include "PObject.h"





class CSegment;
class CIPService;
class CIPServiceList;
class CCSMngrProcess;

class CCommService : public CPObject
{
CLASS_TYPE_1(CCommService,CPObject )
public:
	CCommService();
	~CCommService();
	
	static int MoveNext(int & param);
	static int MoveBack(int & param);
	
	bool GetIsConnected();
	void SetIsConnected(bool val);
	
	void SetIpService(CIPService *);

protected:
	CIPServiceList* GetIpServiceList();
	CIPService *m_IpService;
	CCSMngrProcess *m_pCSProcess;
	
private:
		// disabled
	CCommService(const CCommService&);
	CCommService&operator=(const CCommService&);
	
	bool m_IsConnected;
};

#endif // !defined(__CONFIGPARAMSERVICE_H__)

