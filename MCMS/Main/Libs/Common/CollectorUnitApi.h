// CollectorUnit.h: interface for the CCollectorUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLLECTORUNIT_H__239264C7_5764_49EE_B9D1_F693CD20F40B__INCLUDED_)
#define AFX_COLLECTORUNIT_H__239264C7_5764_49EE_B9D1_F693CD20F40B__INCLUDED_

#include "ProcessBase.h"
#include "SingleToneApi.h"

////////////////////////////////////////////////////////////////////////////////////
class CCollectorUnitApi : public CSingleToneApi  
{
CLASS_TYPE_1(CCollectorUnitApi,CSingleToneApi)
public:
	CCollectorUnitApi();
	virtual ~CCollectorUnitApi();

	virtual const char* NameOf() const { return "CCollectorUnitApi";}
private:

};
/////////////////////////////////////////////////////////////////////////////////////
#endif// !defined(AFX_RSRVMANAGER_H__239264C7_5764_49EE_B9D1_F693CD20F40B__INCLUDED_)
