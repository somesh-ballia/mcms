// CSyncedElement.h: interface for the CSyncedElement class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedElement_H_
#define _SyncedElement_H_


#include "PObject.h"
#include "McuMngrStructs.h"
#include "psosxml.h"

using namespace std;

class CFailoverProcess;


//---------------------------------------------------------------------
//  Parent class
//---------------------------------------------------------------------
class CSyncedElement : public CPObject
{

CLASS_TYPE_1(CSyncedElement, CPObject)

public:
	CSyncedElement ();
	virtual ~CSyncedElement ();
	const char* NameOf() const {return "CSyncedElement";}

	void SendBasicGetTrans();
	void SendTrans(char* strToSend);
	void SetObjToken(int objToken);
	int  GetObjToken();

	virtual void HandleChange(CXMLDOMElement *pRootElem) = 0; // a pure virtual method - must be implemented by any derived class
	virtual void HandleResponseTransConf(CXMLDOMElement *pRootElem);

protected:
	virtual void  PrepareBasicGetTransStr();
	virtual void  SetElementFileds() = 0;

protected:
	CFailoverProcess	*m_pProcess;
	int					m_objToken;
	char*				m_basicGetTransStr;
	char*       		m_transName;
	char*		        m_actionName;
};


#endif // _SyncedElement_H_
