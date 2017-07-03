#ifndef ENHANCEDConfigRESPONSE_H_
#define ENHANCEDConfigRESPONSE_H_

#include "PObject.h"
#include "EnumsAndDefines.h"
#include "SerializeObject.h"

////////////////////////////////////////////////////////////////////////////
//                        CEnhancedConfigResponseItem
////////////////////////////////////////////////////////////////////////////
class CEnhancedConfigResponseItem : public CPObject
{
	CLASS_TYPE_1(CEnhancedConfigResponseItem, CPObject)

public:
	            CEnhancedConfigResponseItem();
	virtual    ~CEnhancedConfigResponseItem();
	const char* NameOf() const                            { return "CEnhancedConfigResponseItem"; }

	void        SerializeXml(CXMLDOMElement*& parentNode, char* NodeName) const;

	void        SetSystemMaximum(WORD system_maximum)     { m_system_maximum = system_maximum; }
	void        SetCurrent(WORD current)                  { m_current = current; }
	void        SetOptionalMaximum(WORD optional_maximum) { m_optional_maximum = optional_maximum; }
	void        SetStep(WORD step)                        { m_step = step; }
	WORD        GetSystemMaximum()                        { return m_system_maximum; }
	WORD        GetCurrent()                              { return m_current; }
	WORD        GetOptionalMaximum()                      { return m_optional_maximum; }
	WORD        GetStep()                                 { return m_step; }

private:
	WORD        m_system_maximum;
	WORD        m_current;
	WORD        m_optional_maximum;
	WORD        m_step;
};


////////////////////////////////////////////////////////////////////////////
//                        CEnhancedConfigResponse
////////////////////////////////////////////////////////////////////////////
class CEnhancedConfigResponse : public CSerializeObject
{
	CLASS_TYPE_1(CEnhancedConfigResponse, CSerializeObject)

public:
	                             CEnhancedConfigResponse();
	virtual                     ~CEnhancedConfigResponse();
	const char*                  NameOf() const           { return "CEnhancedConfigResponse"; }

	CEnhancedConfigResponseItem* GetConfigItem(ePartyResourceTypes type);

	// CSerializeObject overrides
	virtual CSerializeObject*    Clone()                  { return new CEnhancedConfigResponse; }
	virtual void                 SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int                  DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action);

	void                         DumpToTrace();

private:
	CEnhancedConfigResponseItem  m_ConfigResponse[NUM_OF_PARTY_RESOURCE_TYPES];
};

#endif /*ENHANCEDConfigRESPONSE_H_*/
