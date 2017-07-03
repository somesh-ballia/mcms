// SyncedElementIvrServices.h: interface for the SyncedElementIvrServices class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElementIvrServices_H_
#define _SyncedElementIvrServices_H_


#include "SyncedElement.h"
#include "FailoverDefines.h"



class CSyncedElementIvrServices : public CSyncedElement
{
CLASS_TYPE_1(CSyncedElementIvrServices, CSyncedElement)

public:
CSyncedElementIvrServices ();
	virtual ~CSyncedElementIvrServices () {}
	const char* NameOf() const {return "CSyncedElementIvrServices";}

	virtual void HandleChange(CXMLDOMElement *pRootElem);

protected:
	virtual void    SetElementFileds();
};



#endif // _SyncedElementIvrServices_H_
