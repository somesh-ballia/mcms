// CSyncedElementOngoingConfs.h: interface for the CSyncedElementOngoingConfs class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElementOngoingConfs_H_
#define _SyncedElementOngoingConfs_H_

#include "SyncedComplexElement.h"


class CSyncedElementOngoingConfs : public CSyncedComplexElement
{
CLASS_TYPE_1(CSyncedElementOngoingConfs, CSyncedComplexElement)

public:
	CSyncedElementOngoingConfs ();
	virtual ~CSyncedElementOngoingConfs () {}
	const char* NameOf() const {return "CSyncedElementOngoingConfs";}

protected:
	virtual void    SetElementFileds();
	virtual void    SetComplexElementFileds();
	virtual void	HandleChangeIfNeeded(CXMLDOMElement *pRootElem, eChangeType changeType, int &numOfChangedConfs);
};



#endif // _SyncedElementOngoingConfs_H_
