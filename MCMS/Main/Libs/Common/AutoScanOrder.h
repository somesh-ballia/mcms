//+========================================================================+
//                           AutoScanOrder.CPP                        	   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       AutoScanOrder.CPP                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Eitan                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  Nov-2009 	 | Description                                 |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _CAutoScanOrder_H_
#define _CAutoScanOrder_H_

#include "PObject.h"
#include "ConfPartyApiDefines.h"
#include <map>

class CXMLDOMElement;
class CSegment;

using namespace std;

typedef map<int,int> AutoScanMap;

class CAutoScanOrder : public CPObject 
{
CLASS_TYPE_1(CAutoScanOrder,CPObject)
public:
	//Constructors
	CAutoScanOrder();             
	CAutoScanOrder(const  CAutoScanOrder &other);                   
	virtual ~ CAutoScanOrder();
	CAutoScanOrder& operator = (const CAutoScanOrder& other);
	void Dump() const;

	const   char*   NameOf() const;
	
	void    Serialize(WORD format, CSegment& seg);
   	void    DeSerialize(WORD format, CSegment& seg);
   	void    Serialize     (WORD format, std::ostream &ostr);    
	void    DeSerialize   (WORD format, std::istream &istr);
	int     DeSerializeXml(CXMLDOMElement *pFatherNode,char *pszError);	
	void    SerializeXml(CXMLDOMElement *pFatherNode);
	void    RemoveItem(int key);
	
	AutoScanMap* GetAutoScanOrderMap(){return m_autoScanPartiesOrder;}
	
private:
	AutoScanMap* m_autoScanPartiesOrder;
};

#endif //_CAutoScanOrder_H_
