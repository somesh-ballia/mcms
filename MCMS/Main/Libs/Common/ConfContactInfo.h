//+========================================================================+
//                           ConfContactInfo.H                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ConfContactInfo.H                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date  january-2006  | Description                                |
//-------------------------------------------------------------------------|
//			
//+========================================================================+


#ifndef _ConfContactInfo_H_
#define _ConfContactInfo_H_

#include "PObject.h"
#include "psosxml.h"
#include "ObjString.h"
#include "ConfPartyApiDefines.h"

#define MAX_CONF_INFO_ITEMS			3 

/////////////////////////////////////////////////////////////////////////////
// CConfContactInfo
class CConfContactInfo : public CPObject
{
CLASS_TYPE_1(CConfContactInfo,CPObject)
public:
	   //Constructors
	CConfContactInfo();         
    CConfContactInfo(const CConfContactInfo &other);
    virtual ~CConfContactInfo();

	const CConfContactInfo& operator =(const CConfContactInfo & other);

	   // Implementation	   
    void   Serialize(WORD format, std::ostream  &m_ostr);     
	void   DeSerialize(WORD format, std::istream &m_istr);
    void   SetConfId(const DWORD  confId);                 
    DWORD  GetConfId()  const;                
    void   SetContactInfo(const char* info,int ContactNumber);                 
    const  char*  GetContactInfo(int ContactNumber)  const;                
    const char*  NameOf() const;
	void   SerializeXml(CXMLDOMElement *pParent); 
	int    DeSerializeXml(CXMLDOMElement *pParent,char* pszError);	

protected:
	 // Attributes
    DWORD  m_dwConfId;
	CSmallString  m_ContactInfo[MAX_CONF_INFO_ITEMS];
};

#endif //_ConfContactInfo_H_
