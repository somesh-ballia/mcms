// AutoScanOrderDrv.h: interface for the CAutoScanOrderDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//11/09			Eitan			  Class for Update Auto Scan Params
//========   ==============   =====================================================================


#ifndef _AUTO_SCAN_ORDER_DRV__
#define _AUTO_SCAN_ORDER_DRV__

#include "AutoScanOrder.h"
#include "SerializeObject.h"


class CAutoScanOrderDrv : public CSerializeObject
{
CLASS_TYPE_1(CAutoScanOrderDrv, CSerializeObject)
public:

	//Constructors
	CAutoScanOrderDrv();
	CAutoScanOrderDrv(const CAutoScanOrderDrv &other);
	CAutoScanOrderDrv& operator = (const CAutoScanOrderDrv& other);
	virtual ~CAutoScanOrderDrv();

	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	
	CSerializeObject* Clone() {return new CAutoScanOrderDrv();}
		
	const char * NameOf() const {return "CAutoScanOrderDrv";}
    
	CAutoScanOrder*	GetAutoScanOrder();
	DWORD			GetConfID();

  
protected:

	DWORD				  m_ConfID;
	CAutoScanOrder*   	  m_pAutoScanOrder;
	

};

#endif // _AUTO_SCAN_ORDER_DRV__

