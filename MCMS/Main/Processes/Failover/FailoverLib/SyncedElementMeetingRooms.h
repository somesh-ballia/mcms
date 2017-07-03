// CSyncedElementMeetingRooms.h : interface for the CSyncedElementMeetingRooms class: For MR, EQ, Sip factories, GW profiles
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElementMeetingRooms_H_
#define _SyncedElementMeetingRooms_H_

#include "SyncedElementBasicReservations.h"


class CSyncedElementMeetingRooms : public CSyncedElementBasicReservations
{
CLASS_TYPE_1(CSyncedElementMeetingRooms, CSyncedElementBasicReservations)

public:
	CSyncedElementMeetingRooms ();
	virtual ~CSyncedElementMeetingRooms () {}
	const char* NameOf() const {return "CSyncedElementMeetingRooms";}

protected:
	virtual void  SetElementFileds();
	virtual void  SetComplexElementFileds();
	virtual void  SetBasicReservationElementFileds();

};



#endif // _SyncedElementMeetingRooms_H_
