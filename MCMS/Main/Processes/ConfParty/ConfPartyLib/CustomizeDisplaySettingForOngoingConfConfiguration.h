
/*
 * CustomizeDisplaySettingForOngoingConfConfiguration.h
 *
 *  Created on: Oct 27, 2010
 *      Author: yiye
 */

#ifndef CustomizeDisplaySettingForOngoingConfConfiguration_H_
#define CustomizeDisplaySettingForOngoingConfConfiguration_H_

#include "SerializeObject.h"
#include "ConfPartyProcess.h"
#include "ConfPartyDefines.h"

class CCustomizeDisplaySettingForOngoingConfConfiguration : public CSerializeObject
{
	CLASS_TYPE_1(CCustomizeDisplaySettingForOngoingConfConfiguration, CSerializeObject)
public:

	CCustomizeDisplaySettingForOngoingConfConfiguration();
	~CCustomizeDisplaySettingForOngoingConfConfiguration();
	CCustomizeDisplaySettingForOngoingConfConfiguration& operator=(const CCustomizeDisplaySettingForOngoingConfConfiguration& other);
	virtual const char*  NameOf() const {return "CCustomizeDisplaySettingForOngoingConfConfiguration";}
	CSerializeObject* Clone() {return new CCustomizeDisplaySettingForOngoingConfConfiguration();}
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	BOOL			IsObtainDsipalyNamefromAddressBook() const;
	void			SetObtainDisplayNamefromAddressBook(BOOL bDsp);

private:
	WORD			m_bObtainDisplayNamefromAddressBook;
};
#endif
