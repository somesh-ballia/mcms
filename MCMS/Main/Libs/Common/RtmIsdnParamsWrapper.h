#ifndef RTMISDNPARAMS_H_
#define RTMISDNPARAMS_H_



#include "PObject.h"
#include "RtmIsdnMaintenanceStructs.h"
#include "RtmIsdnMngrCommonMethods.h"

using namespace std;


class CRtmIsdnParamsWrapper : public CPObject
{

CLASS_TYPE_1(CRtmIsdnParamsWrapper, CPObject)

public:
	CRtmIsdnParamsWrapper ();
	virtual const char* NameOf() const { return "CRtmIsdnParamsWrapper";}
	virtual ~CRtmIsdnParamsWrapper ();
	virtual void Dump(ostream& msg);

	CRtmIsdnParamsWrapper& operator = (const CRtmIsdnParamsWrapper &other);

	RTM_ISDN_PARAMETERS_S* GetRtmIsdnParamsStruct();

	DWORD	GetCountryCode ();
	void		SetCountryCode (const DWORD countryCode);
	void		SetCountryCode (const string countryCode);

	DWORD	GetIdleCodeT1 ();
	void		SetIdleCodeT1 (const DWORD idleCode);

	DWORD	GetIdleCodeE1 ();
	void		SetIdleCodeE1 (const DWORD idleCode);

	DWORD	GetNumOfDigits ();
	void		SetNumOfDigits (const DWORD numOfDigits);

	DWORD	GetClocking ();
	void		SetClocking (const DWORD clocking);

	void		SetData(const char *data);
	
	string  PrintStructData();
	

protected:
	RTM_ISDN_PARAMETERS_S m_rtmIsdnParamsStruct;

	CRtmIsdnMngrCommonMethods m_rtmIsdnCommonMethods;
};




#endif /*RTMISDNPARAMS_H_*/
