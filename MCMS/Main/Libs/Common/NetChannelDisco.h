/*$Header: /MCMS/MAIN/subsys/mcmsoper/NETDISCO.H 7     18/02/01 19:06 Shlomit $*/
//+========================================================================+
//                            NETDISCO.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       NETDISCO.H                                                  |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef _NETDISCO
#define _NETDISCO


#include "PObject.h"
#include "CDREvent.h"
#include "CDRUtils.h"
#include <iostream>


//class CStructTm;
class CCallingParty;
class CCalledParty;
class CXMLDOMElement;
class ACCCallingParty;
class ACCCalledParty;


class CNetChannelDisco  :public CPObject ,public ACCCDREventNetChannelDisconnect 
{
CLASS_TYPE_1(CNetChannelDisco, CPObject)//**check macro**
public:
	//Constructors
	CNetChannelDisco();
	//CNetChannelDisco(const CNetChannelDisco &other);
	~CNetChannelDisco();

	// Implementation

	char*Serialize(WORD format);  	
	void Serialize       (WORD format, std::ostream &m_ostr ,DWORD apiNum);
	void ShortSerialize(WORD format, std::ostream &m_ostr ,DWORD apiNum);
	void Short323Serialize(WORD format, std::ostream &m_ostr ,DWORD apiNum);
	void Serialize(WORD format, std::ostream &m_ostr,BYTE fullformat ,DWORD apiNum);	
	void ShortSerialize(WORD format, std::ostream &m_ostr,BYTE fullformat ,DWORD apiNum);	
	void Short323Serialize(WORD format, std::ostream &m_ostr,BYTE fullformat ,DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr);		
	void ShortDeSerialize(WORD format, std::istream &m_istr ,DWORD apiNum);			
	void Short323DeSerialize(WORD format, std::istream &m_istr ,DWORD apiNum);
	void SerializeXml(CXMLDOMElement* pFatherNode,WORD nEventType);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	const char*  NameOf() const;	
	void   SetNetDiscoPartyName(const char* partyname);          
	void   SetNetDiscoPartyId(const DWORD partyid);            
	void   SetNetDiscoChannelId(const BYTE channelid);         
	void   SetNetDiscoInitiator(const BYTE disco_init);          
	void   SetNetDiscoQ931(const ACCCDREventDisconnectCause &q931_cause);
  	
protected:

	//CDisconctCause m_q931_cause; 
};

	 
#endif /* _NETDISCO */

