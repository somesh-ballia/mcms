/*$Header: /MCMS/MAIN/subsys/mcmsoper/CALLPART.H 3     23/08/00 18:31 Michaelr $*/
//+========================================================================+
//                            CALLPART.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CALLPART.H                                                  |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#ifndef _CALLPART
#define _CALLPART

#include  <iostream>
#include "PObject.h"
#include "CDREvent.h"


class CXMLDOMElement;

///////////////////////////////////////////////////////////////////////
class CCallingParty : public CPObject, public ACCCallingParty
{
CLASS_TYPE_1(CCallingParty,CPObject )//**check macro**
public:
	//Constructors
	CCallingParty();
	CCallingParty(const ACCCallingParty &other);

	~CCallingParty();
		
// Implementation
	
	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr);	
	void Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag);	
	void DeSerialize(WORD format, std::istream &m_istr);			
	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	
	const char*  NameOf() const;	

	void   SetCallingNumType(const BYTE numtype);              
	void   SetCallingNumPlan(const BYTE numplan);            
	void   SetCallingPresentInd(const BYTE presentationind);        
	void   SetCallingScreenInd(const BYTE screeningind);           
	void   SetCallingPhoneNum(const char* phonenumber);        
	
};

///////////////////////////////////////////////////////////////////////
class CCalledParty  : public CPObject, public ACCCalledParty
{
CLASS_TYPE_1(CCalledParty, CPObject)//**check macro**
public:
	
	//Constructors
	CCalledParty();
	CCalledParty(const ACCCalledParty &other);

	~CCalledParty();
	
	// Implementation
	
	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr);	
	void Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag);	
	void DeSerialize(WORD format, std::istream &m_istr);	
	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	
	const char*  NameOf() const;	

	void   SetCalledNumType(const BYTE numtype);             
	void   SetCalledNumPlan(const BYTE numplan);          
	void   SetCalledPhoneNum(const char* phonenumber);
	
};

#endif /*_CALLPART */


