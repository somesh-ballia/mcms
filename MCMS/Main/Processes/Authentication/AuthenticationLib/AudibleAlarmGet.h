#ifndef AUDIBLE_ALARM_GET_H_
#define AUDIBLE_ALARM_GET_H_

#include "SerializeObject.h"
#include <string>
#include "string.h"
#include "InitCommonStrings.h"

class CXMLDOMElement;


class CAudibleAlarmGet : public CSerializeObject

{

public:

	CAudibleAlarmGet();

	CAudibleAlarmGet(const CAudibleAlarmGet &other);

	CAudibleAlarmGet& operator= (const CAudibleAlarmGet &other);

	~CAudibleAlarmGet();

	const char * NameOf(void) const {return "CAudibleAlarmGet";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CAudibleAlarmGet(*this);}

private:
    std::string m_user_name;
};

#endif /*AUDIBLE_ALARM_GET_H_*/
