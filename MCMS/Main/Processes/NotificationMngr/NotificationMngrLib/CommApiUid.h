/*
 * CommApiUid.h
 *
 *  Created on: Jun 4, 2012
 *      Author: Vasily
 */

#ifndef __COMMAPIUID_H__
#define __COMMAPIUID_H__


#include "SerializeObject.h"
//#include "DataTypes.h"


class CommApiUid : public CSerializeObject
{
CLASS_TYPE_1(CommApiUid,CSerializeObject)
public:
	CommApiUid();
	CommApiUid(const CommApiUid& other);
	virtual ~CommApiUid();
	CommApiUid& operator =(const CommApiUid& other);
	virtual CSerializeObject* Clone() { return new CommApiUid; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	std::string GetUid() const { return m_sUid; }

protected:
	std::string  m_sUid;
};



#endif /* __COMMAPIUID_H__ */
