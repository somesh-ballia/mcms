/*$Header: /MCMS/MAIN/subsys/mcmsoper/NTCHACON.H 7     23/08/00 18:31 Michaelr $*/
//+========================================================================+
//                            NTCHACON.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       NTCHACON.H                                                  |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#ifndef _NTCHACON
#define _NTCHACON


/*
#ifdef __HIGHC__

#ifndef _POBJECT
#include  <pobject.h>
#endif

#ifndef _MCMSOPER
#include  <mcmsoper.h>
#endif

#ifndef _NTIME
#include  <ntime.h> 
#endif
#include  <callpart.h> 




#else

#include  "Mcmsoper.h"
#include  "Pobject.h"
#include  "ntime.h"
#include  "callpart.h"
#include  "cdrevent.h"

#endif
#include  <cdrevent.h>
#include  <strstrea.h>
#include  <iostream.h>

class CStructTm;
class COstrStream;
class CIstrStream;
class CCallingParty;
class CCalledParty;
class CXMLDOMElement;

// CNetChanlCon 
*/

#include "PObject.h"
#include "CDREvent.h"
#include <iostream>


class CStructTm;
//class COstrStream;
//class CIstrStream;
class CCallingParty;
class CCalledParty;
class CXMLDOMElement;
class ACCCallingParty;
class ACCCalledParty;
		
class CNetChanlCon  : public CPObject,public ACCCDREventNetChannelConnect
{
CLASS_TYPE_1(CNetChanlCon, CPObject)//**check macro**
public:
	//Constructors

	CNetChanlCon();
	//CNetChanlCon(const CNetChanlCon &other);
	~CNetChanlCon();

	// Implementation
    bool operator == (const CNetChanlCon & rHnd);
    
	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
	void Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);

//#ifdef __HIGHC__
	void SerializeXml(CXMLDOMElement* pFatherNode);
//#endif
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;	
	void   SetPartyName(const char* h243partyname);                 
	void   SetPartyId(const DWORD partyid);                 
 	void   SetChanlId(const BYTE chanlid);
	void   SetChanlNum(const BYTE chanlNum);
	void   SetConctIniator(const BYTE conct_initiator);
	void   SetCallType(const DWORD calltype);
	void   SetNetSpec(const BYTE net_specific);
	void   SetPrfMode (const BYTE prf_mode);
	void   SetCallingParty( const ACCCallingParty &other);
	void   SetCalledParty(const ACCCalledParty &other);

	BYTE  ConvertConnectionTypeToConnectInitiatorType(BYTE connectionType) const;	
		
};

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/*class CATMChanlCon  : public CChanlCon
{
public:
	   //Constructors
		 
    CATMChanlCon();
	CATMChanlCon(const CNetChanlCon &other);
	~CATMChanlCon();
		
		// Implementation
			
  	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
 	void Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum);	
    void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);			

	const char* NameOf() const;	
    void  SetResrtrict(const DWORD resrtrict);
    const DWORD  GetResrtrict() const; 
    void  SetATMaddress(const char* ATMaddress);                 
    const char*  GetATMaddress() const;                 
	void  SetCalledParty(const CCalledParty &other);
    const CCalledParty*  GetCalledParty() const;                 
 
		
  	
protected:
    	DWORD m_resrtrict;
		char m_ATMaddress[20];
		CCalledParty  m_called_party;
		
		
};*/

///////////////////////////////////////////////////////////////////////////////////
class CMPIChanlCon  : public CPObject,public ACCCDREventMPIChannelConnect
{
CLASS_TYPE_1(CMPIChanlCon,CPObject )//**check macro**
public:
	//Constructors

	CMPIChanlCon();
	//CMPIChanlCon(const CMPIChanlCon &other);
	~CMPIChanlCon();

	// Implementation

	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
	void Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);			

	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	
	const char*  NameOf() const;	
	void   SetPartyName(const char* h243partyname);               
	void   SetPartyId(const DWORD partyid);                
	void   SetChanlId(const BYTE chanlid);                
	void   SetConctIniator(const BYTE conct_initiator);              
	void   SetResrict(const DWORD resrict);
	void   SetChanlNum(const BYTE chanlNum);               
	void   SetCalledParty(const ACCCalledParty &other);

	BYTE  ConvertConnectionTypeToConnectInitiatorType(BYTE connectionType) const;	
		
		
};


///////////////////////////////////////////////////////////////////////////////////
class CIpChanlCon  : public CPObject,public ACCCDREvent323ChannelConnect
{
CLASS_TYPE_1(CIpChanlCon, CPObject)//**check macro**
public:
	//Constructors

	CIpChanlCon();
	//C323ChanlCon(const C323ChanlCon &other);
	~CIpChanlCon();

	// Implementation

	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
	void Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);			

//#ifdef __HIGHC__
	void SerializeXml(CXMLDOMElement* pFatherNode,WORD nEventType);
//#endif
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;	
	void   SetPartyName(const char* h243partyname);                 
	void   SetPartyId(const DWORD partyid);              
	void   SetConctIniator(const BYTE conct_initiator);             
	void  SetIpMaxRate(DWORD rate)	{ m_IpmaxRate		 = rate;	}
	void  SetIpMinRate(DWORD rate)	{ m_IpminRate		 = rate;	}
	void  SetIpEndPointType(BYTE type){ m_IpendpointType = type;	}
	void  SetSrcPartyAddress(const char * srcPartyAddr); 
	void  SetDstPartyAddress(const char * dstPartyAddr); 

		
};
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
#endif /* _NTCHACON */		
