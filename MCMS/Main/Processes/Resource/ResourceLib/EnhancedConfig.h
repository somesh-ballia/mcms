#ifndef ENHANCEDCONFIG_H_
#define ENHANCEDCONFIG_H_

#include "PObject.h"
#include "EnumsAndDefines.h"
#include "SerializeObject.h"

////////////////////////////////////////////////////////////////////////////
//                        CEnhancedConfig
////////////////////////////////////////////////////////////////////////////
class CEnhancedConfig : public CSerializeObject
{
	CLASS_TYPE_1(CEnhancedConfig, CSerializeObject)

public:
	                          CEnhancedConfig();
	virtual                  ~CEnhancedConfig();
	CEnhancedConfig&          operator=(const CEnhancedConfig& other);
	const char*               NameOf() const { return "CEnhancedConfig"; }

	int                       GetConfiguration(ePartyResourceTypes type) const;
	void                      SetConfiguration(ePartyResourceTypes type, int configuration);
	int                       GetConfiguration(int type) const;
	void                      SetConfiguration(int type, int configuration);

	void                      ReadFromProcessSetting();
	void                      WriteToProcessSetting();

	CEnhancedConfig&          SetIpServicePartConfig(float service_factor, BOOL round_up);

	void                      DumpToTrace();

	// CSerializeObject overrides
	virtual CSerializeObject* Clone()        { return new CEnhancedConfig; }
	virtual void              SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int               DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action);

private:
	WORD                      m_Configuration[NUM_OF_PARTY_RESOURCE_TYPES];
};

#endif /*ENHANCEDCONFIG_H_*/
