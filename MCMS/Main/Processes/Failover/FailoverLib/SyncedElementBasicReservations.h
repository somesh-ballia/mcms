// CSyncedElementBasicReservations.h: interface for the CSyncedElementBasicReservations class.
// For all the objects with transaction name "TRANS_RES_LIST", followed transaction TRANS_RES_2 and response object of RESERVATION
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElementBasicReservations_H_
#define _SyncedElementBasicReservations_H_


#include "SyncedComplexElement.h"


class CSyncedElementBasicReservations : public CSyncedComplexElement
{
CLASS_TYPE_1(CSyncedElementReservations, CSyncedComplexElement)

public:
	CSyncedElementBasicReservations ();
	virtual ~CSyncedElementBasicReservations () {}
	const char* NameOf() const {return "CSyncedElementBasicReservations";}

protected:
	virtual void  SetBasicReservationElementFileds() = 0;

	virtual void  HandleChangeIfNeeded(CXMLDOMElement *pRootElem, eChangeType changeType, int &numOfChanged);
	

	char* m_responseSummaryLsNodeName;
	char* m_responseSummaryNodeName;
};



#endif // _SyncedElementBasicReservations_H_
