// CSyncedElementIpService.h: interface for the CSyncedElementIpService class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElementIpService_H_
#define _SyncedElementIpService_H_


#include "SyncedElement.h"
#include "FailoverDefines.h"



class CSyncedElementIpService : public CSyncedElement
{
CLASS_TYPE_1(CSyncedElementIpService, CSyncedElement)

public:
	CSyncedElementIpService ();
	virtual ~CSyncedElementIpService () {}
	const char* NameOf() const {return "CSyncedElementIpService";}

	virtual void HandleChange(CXMLDOMElement *pRootElem);

protected:
	virtual void    SetElementFileds();
};



#endif // _SyncedElementIpService_H_
