#ifndef ALLOCATIONMODEDETAILS_H_
#define ALLOCATIONMODEDETAILS_H_

#include "PObject.h"
#include "SerializeObject.h"
#include "ConfPartyApiDefines.h"

#define USE_FLIXIBILE_RESOURCE_CAPACITY_ONLY YES

////////////////////////////////////////////////////////////////////////////
//                        CAllocationModeDetails
////////////////////////////////////////////////////////////////////////////
class CAllocationModeDetails: public CSerializeObject
{
	CLASS_TYPE_1(CAllocationModeDetails,CSerializeObject)

public:
	                    CAllocationModeDetails();
	virtual            ~CAllocationModeDetails();
	const char *        NameOf() const { return "CAllocationModeDetails"; }

	virtual             CSerializeObject* Clone() { return new CAllocationModeDetails; }
	virtual void        SerializeXml(CXMLDOMElement*& thisNode ) const;
	virtual int         DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action);

	void                SetModes(eAllocationModeType mode, eAllocationModeType futureMode);
	eAllocationModeType GetMode() { return m_Mode; }

	STATUS              ReadFromProcessSetting();
	STATUS              WriteToProcessSetting();

private:
	eAllocationModeType m_Mode;
	eAllocationModeType m_FutureMode;
};

#endif /*ALLOCATIONMODEDETAILS_H_*/
