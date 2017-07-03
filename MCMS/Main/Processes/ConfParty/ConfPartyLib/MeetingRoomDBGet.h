#ifndef _MEETING_ROOM_DB_GET_H_
#define _MEETING_ROOM_DB_GET_H_

#include "SerializeObject.h"


class CMeetingRoomDBGet : public CSerializeObject
{
CLASS_TYPE_1(CMeetingRoomDBGet, CSerializeObject)
public:
	//Constructors
	CMeetingRoomDBGet();
	CMeetingRoomDBGet(const CMeetingRoomDBGet &other);
	virtual const char* NameOf() const { return "CMeetingRoomDBGet";}
	CMeetingRoomDBGet& operator = (const CMeetingRoomDBGet& other);
	virtual ~CMeetingRoomDBGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CMeetingRoomDBGet();}
		
protected:
};

#endif 
