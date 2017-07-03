/*
 * CGetDynamicContentRateResTable.h
 *
 *  Created on: Nov 29, 2011
 *      Author: mhalfon
 */

#ifndef CGETDYNAMICCONTENTRATERESTABLE_H_
#define CGETDYNAMICCONTENTRATERESTABLE_H_

#include "SerializeObject.h"
#include "ConfPartyApiDefines.h"
#include "psosxml.h"
#include "StatusesGeneral.h"

class CGetContentRateResTable : public CSerializeObject
{
CLASS_TYPE_1(CGetContentRateResTable, CSerializeObject)
public:

	virtual const char* NameOf() const { return "CGetContentRateResTable"; }

	//CGetContentRateResTable();

	CGetContentRateResTable();

	virtual void   SerializeXml(CXMLDOMElement*& pFatherNode) const {}
	int    DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError);

	CSerializeObject* Clone() { return new CGetContentRateResTable(); }

protected:

	typedef map<eCascadeOptimizeResolutionEnum, BYTE> Resolution2RateMap;
	typedef map<eEnterpriseMode, Resolution2RateMap> RateResolutionMinValuesMap;

	bool m_isHighProfile;
};

//GET_DYNAMIC_CONTENT_RATE_TABLE / GET_DYNAMIC_HP_CONTENT_RATE_TABLE
class CGetDynamicContentRateResTable : public CGetContentRateResTable
{
CLASS_TYPE_1(CGetDynamicContentRateResTable, CGetContentRateResTable)
public:

	const char* NameOf() const { return "CGetDynamicContentRateResTable"; }

	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;

	CSerializeObject* Clone() { return new CGetDynamicContentRateResTable(); }

protected:

	void getAmcH264DynamicContentMinRate(
		RateResolutionMinValuesMap& amcH264DynamicContentRateResTableMinValues) const;

};

//GET_CUSTOMIZED_CONTENT_RATE_TABLE / GET_CUSTOMIZED_HP_CONTENT_RATE_TABLE
class CGetCustomizedContentRateResTable : public CGetContentRateResTable
{
CLASS_TYPE_1(CGetCustomizedContentRateResTable, CGetContentRateResTable)
public:

	virtual const char* NameOf() const { return "CGetCustomizedContentRateResTable"; }

	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;

	CSerializeObject* Clone() { return new CGetCustomizedContentRateResTable(); }
};

#endif /* CGETDYNAMICCONTENTRATERESTABLE_H_ */
