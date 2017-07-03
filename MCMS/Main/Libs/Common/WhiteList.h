/*
 * WhiteList.h
 *
 *  Created on: Aug 28, 2012
 *      Author: stanny
 */

#ifndef WHITELIST_H_
#define WHITELIST_H_
#include "psosxml.h"
#include "PObject.h"
#include <list>
#include "Segment.h"

#define SEG_IPV4   1
#define SEG_IPV6   2
#define MAX_WHITE_LIST_SIZE 100
class CWhiteList: public CPObject {
public:
	CLASS_TYPE_1(CWhiteList,CPObject )
	
	CWhiteList();
	virtual ~CWhiteList();
	//framework
	void  SerializeXml(CXMLDOMElement* pFatherNode);
	int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);	
	virtual const char* NameOf() const { return "CWhiteList";}
	CWhiteList& operator = (const CWhiteList &rOther);    
    bool operator==(const CWhiteList &rHnd)const;
    bool operator!=(const CWhiteList& other)const;
    // Access
    BOOL IsWhiteListEnable(){return m_enableWhiteList;}    
    void  SetWhiteListEnable(BOOL status){m_enableWhiteList = status;}
    void  WriteWhiteListToSegment(CSegment* pWhiteListSeg);
    virtual void Dump(ostream& msg) const;
    bool IsWhiteListValid();
private:    
    BYTE m_enableWhiteList;
    std::list<std::string> m_ipList;
    void  WriteListToSegment(std::list<std::string>& iplist , CSegment* pWhiteListSeg);
    bool IsIPRangeValid(std::string& ip);
};

#endif /* WHITELIST_H_ */
