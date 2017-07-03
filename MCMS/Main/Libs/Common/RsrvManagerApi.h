// RsrvManager.h: interface for the CRsrvManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RSRVMANAGER_H__239264C7_5764_49EE_B9D1_F693CD20F40B__INCLUDED_)
#define AFX_RSRVMANAGER_H__239264C7_5764_49EE_B9D1_F693CD20F40B__INCLUDED_

#include "ProcessBase.h"
#include "SingleToneApi.h"

////////////////////////////////////////////////////////////////////////////////////
class CRsrvManagerApi : public CSingleToneApi  
{
CLASS_TYPE_1(CRsrvManagerApi,CSingleToneApi)
public:
	CRsrvManagerApi();
	virtual ~CRsrvManagerApi();
    
	virtual const char* NameOf() const { return "CRsrvManagerApi";}
    static COsQueue *pRsrvMngrQueue;

};
/////////////////////////////////////////////////////////////////////////////////////
#endif// !defined(AFX_RSRVMANAGER_H__239264C7_5764_49EE_B9D1_F693CD20F40B__INCLUDED_)
