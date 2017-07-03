// SyncedElementRecordingLinks.h: interface for the SyncedElementRecordingLinks class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElementRecordingLinks_H_
#define _SyncedElementRecordingLinks_H_


#include "SyncedElement.h"
#include "FailoverDefines.h"



class CSyncedElementRecordingLinks : public CSyncedElement
{
CLASS_TYPE_1(CSyncedElementRecordingLinkse, CSyncedElement)

public:
	CSyncedElementRecordingLinks ();
	virtual ~CSyncedElementRecordingLinks () {}
	const char* NameOf() const {return "CSyncedElementRecordingLinks";}

	virtual void HandleChange(CXMLDOMElement *pRootElem);

protected:
	virtual void    SetElementFileds();
};



#endif // _SyncedElementRecordingLinks_H_
