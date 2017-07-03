// SyncedElementReservations.h : interface for the CSyncedElementReservations class
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElementReservations_H_
#define _SyncedElementReservations_H_

#include "SyncedElementBasicReservations.h"


class CSyncedElementReservations : public CSyncedElementBasicReservations
{
CLASS_TYPE_1(CSyncedElementReservations, CSyncedElementBasicReservations)

public:
	CSyncedElementReservations ();
	virtual ~CSyncedElementReservations () {}
	const char* NameOf() const {return "SyncedElementReservations";}

protected:
	virtual void  SetElementFileds();
	virtual void  SetComplexElementFileds();
	virtual void  SetBasicReservationElementFileds();

};



#endif // _SyncedElementReservations_H_
