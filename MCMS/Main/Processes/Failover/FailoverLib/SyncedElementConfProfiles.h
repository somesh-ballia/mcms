// SyncedElementConfProfiles.h : interface for the SyncedElementConfProfiles class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElementConfProfiles_H_
#define _SyncedElementConfProfiles_H_

#include "SyncedElementBasicReservations.h"


class CSyncedElementConfProfiles : public CSyncedElementBasicReservations
{
CLASS_TYPE_1(CSyncedElementConfProfiles, CSyncedElementBasicReservations)

public:
	CSyncedElementConfProfiles ();
	virtual ~CSyncedElementConfProfiles () {}
	const char* NameOf() const {return "CSyncedElementConfProfiles";}

protected:
	virtual void  SetElementFileds();
	virtual void  SetComplexElementFileds();
	virtual void  SetBasicReservationElementFileds();

};



#endif // _SyncedElementConfProfiles_H_
