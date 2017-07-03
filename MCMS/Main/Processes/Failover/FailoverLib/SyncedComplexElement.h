// CSyncedComplexElement.h: interface for the CSyncedComplexElement class.
// For elements that has a followed transaction
//////////////////////////////////////////////////////////////////////

#ifndef _SyncedComplexElement_H_
#define _SyncedComplexElement_H_


#include "SyncedElement.h"
#include "FailoverDefines.h"
#include "McmsProcesses.h"


class CSyncedComplexElement : public CSyncedElement
{
CLASS_TYPE_1(CSyncedComplexElement, CSyncedElement)

public:
	CSyncedComplexElement ();
	virtual ~CSyncedComplexElement () {}
	const char* NameOf() const {return "CSyncedComplexElement";}

	void	HandleChange(CXMLDOMElement *pRootElem);
	void	HandleResponseTrans2(CXMLDOMElement *pRootElem);

protected:

	virtual void    SetComplexElementFileds() = 0;
	virtual void	HandleChangeIfNeeded(CXMLDOMElement *pRootElem, eChangeType changeType, int &numOfChangedConfs) = 0;
	virtual void	SendTrans2(DWORD id);
	virtual void    PrepareTrans2Str(DWORD id, char* &transStr);
	virtual void    SendDelInd(DWORD id);
	virtual void    SendAddOrUpdateInd(CXMLDOMElement *pElemNode);

	char* m_trans2TransName;
	char* m_trans2ActionName;
	char* m_trans2ResponseObjectName;

	DWORD m_addOrUpdateOpcode;
	DWORD m_delOpcode;
	eProcessType m_eDestProcess;
};



#endif // _SyncedComplexElement_H_
