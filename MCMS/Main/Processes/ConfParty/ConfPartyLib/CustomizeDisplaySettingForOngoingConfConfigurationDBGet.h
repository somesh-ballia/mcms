
#ifndef _CCustomizeDisplaySettingForOngoingConfConfiguration_DB_GET_H_
#define _CCustomizeDisplaySettingForOngoingConfConfiguration_DB_GET_H_

#include "SerializeObject.h"
#include "CustomizeDisplaySettingForOngoingConfConfiguration.h"
class CConfPartyProcess;

class CCustomizeDisplaySettingForOngoingConfConfigurationDBGet : public CSerializeObject
{
CLASS_TYPE_1(CCustomizeDisplaySettingForOngoingConfConfigurationDBGet, CSerializeObject)
public:
	//Constructors
	CCustomizeDisplaySettingForOngoingConfConfigurationDBGet();
	CCustomizeDisplaySettingForOngoingConfConfigurationDBGet(const CCustomizeDisplaySettingForOngoingConfConfigurationDBGet &other);
	CCustomizeDisplaySettingForOngoingConfConfigurationDBGet& operator = (const CCustomizeDisplaySettingForOngoingConfConfigurationDBGet& other);
	virtual ~CCustomizeDisplaySettingForOngoingConfConfigurationDBGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CCustomizeDisplaySettingForOngoingConfConfigurationDBGet();}

	const char * NameOf() const {return "CCustomizeDisplaySettingForOngoingConfConfigurationDBGet";}

private:


};

#endif


