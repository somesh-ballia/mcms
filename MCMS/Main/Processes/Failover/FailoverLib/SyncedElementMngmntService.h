// CSyncedElementMngmntService.h: interface for the CSyncedElementMngmntService class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElementMngmntService_H_
#define _SyncedElementMngmntService_H_


#include "SyncedElement.h"
#include "FailoverDefines.h"



class CSyncedElementMngmntService : public CSyncedElement
{
CLASS_TYPE_1(CSyncedElementMngmntService, CSyncedElement)

public:
	CSyncedElementMngmntService ();
	virtual ~CSyncedElementMngmntService () {}
	const char* NameOf() const {return "CSyncedElementMngmntService";}

	virtual void HandleChange(CXMLDOMElement *pRootElem);

protected:
	virtual void    SetElementFileds();
};



#endif // _SyncedElementMngmntService_H_
