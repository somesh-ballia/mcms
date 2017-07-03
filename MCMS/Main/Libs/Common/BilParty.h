/*$Header: /MCMS/MAIN/subsys/mcmsoper/BILPARTY.H 7     1/17/02 3:09p Guy $*/
//+========================================================================+
//                            BILPARTY.H                                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BILPARTY.H                                               |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#ifndef _PARTY
#define _PARTY


#include  <iostream>
#include  <list>

class	CH221Str;


class CXMLDOMElement;


#include  "PObject.h"
#include "CDREvent.h"
  
class CRemoteComMode  : public CPObject ,public ACCCDREventRemoteCommMode
{
	CLASS_TYPE_1(CRemoteComMode,CPObject ) //**check macro**
public:
	//Constructors

	CRemoteComMode();
	//CRemoteComMode(const CRemoteComMode &other);
	~CRemoteComMode();

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
	void   SetRemoteCommMode(const CH221Str &other);
		 
};
////////////////////////////////////////////////////////////////////
class CPartyConnected  : public ACCCDREventPartyConnected
{
//CLASS_TYPE_1(CPartyConnected, ACCCDREventPartyConnected)//**check macro**
public:
	//Constructors

	CPartyConnected();
	~CPartyConnected();
	CPartyConnected(const CPartyConnected &other);
	CPartyConnected& operator=(const CPartyConnected &other);
	bool operator == (const CPartyConnected &rHnd);

	// Implementation

	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
	void Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);			
    void  Serialize323(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum);


	void SerializeXml(CXMLDOMElement* pFatherNode,WORD nEventType);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;
	void   SetPartyName(const char* h243partyname);                 
	void   SetPartyId(const DWORD partyid);                 
	void   SetPartyState(const DWORD partystate);
	void   SetSecondaryCause(const BYTE second_cause);
	void   SetCapabilities(const CH221Str &other);
	void   SetRemoteCommMode(const CH221Str &other);

	
};

class CSvcSipPartyConnected : public CPObject
{
	CLASS_TYPE_1(CSvcSipPartyConnected, CPObject)
public:
	CSvcSipPartyConnected();
	CSvcSipPartyConnected(const CSvcSipPartyConnected& other);
	virtual ~CSvcSipPartyConnected();
	virtual const char* NameOf() const { return "CSvcSipPartyConnected";}

	CSvcSipPartyConnected&			operator= (const CSvcSipPartyConnected& other);
//	bool							operator==(const CSvcSipPartyConnected& other);

	void							Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);
	void							Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum);
	void							DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);
	void							SerializeXml(CXMLDOMElement* pFatherNode,WORD nEventType);
	int								DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	void							SetPartyName(const char* h243partyname);
	void							SetPartyId(const DWORD partyid);
	void							SetPartyState(const DWORD partystate);
	void							SetAudioCodec(const DWORD codec);
	void							SetBitRateOut(const DWORD dwBitRateOut);
	void							SetBitRateIn(const DWORD dwBitRateIn);
	void							SetStreams(const std::list<SvcStreamDesc>* pStreams);

	const char*						GetPartyName() const;
	DWORD							GetPartyId() const;
	DWORD							GetPartyState() const;
	DWORD							GetAudioCodec() const;
	DWORD							GetBitRateOut() const;
	DWORD							GetBitRateIn() const;
	const std::list<SvcStreamDesc>*	GetStreams()const;

protected:
	char							m_h243party_name[H243_NAME_LEN];
	DWORD							m_party_Id;
	DWORD							m_party_state;
	DWORD							m_audioCodec;
	DWORD							m_bitRateOut;
	DWORD							m_bitRateIn;
	std::list<SvcStreamDesc>		m_listStreams;
};


class CPartyDisconnected  : public CPObject,public ACCCDREventPartyDisconnected 
{
CLASS_TYPE_1(CPartyDisconnected, CPObject)//**check macro**
public:
	//Constructors

	CPartyDisconnected();
	~CPartyDisconnected();

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
	void   SetDisconctCause(const DWORD disconctcause);
	void   SetQ931DisonctCause(const DWORD q931discocause);

};
///////////////////////////////////////////////////////////////////////////////////
//CPartyErrors

class CPartyErrors  : public CPObject,public ACCCDRPartyErrors
{
CLASS_TYPE_1(CPartyErrors, CPObject)//**check macro**
public:
   //Constructors

	CPartyErrors();
	~CPartyErrors();
		
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
	void   SetResrcFailure(const BYTE rsrcfailure);
	void   SetNumH221SyncLoss(const DWORD h221syncloss);
	void   SetDurationH221SyncLoss(const DWORD durationsyncloss);
	void   SetNumH221SyncRLoss(const DWORD h221syncRloss);
	void   SetDurationH221SyncRLoss(const DWORD durationsynRcloss);
 	void   SetNumberRemoteVcu(const DWORD remote_vcu);
 	
};
//////////////////////////////////////////////////////////////////////////

class CPartyDisconnectedCont1  : public CPObject,public ACCCDREventPartyDisconnectedCont1
{
CLASS_TYPE_1(CPartyDisconnectedCont1,CPObject )//**check macro**
public:
	//Constructors
 
	CPartyDisconnectedCont1();
	//CPartyDisconnectedCont1(const CPartyDisconnectedCont1 &other);
	~CPartyDisconnectedCont1();
		
	// Implementation
			
	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
	void Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);


	void SerializeXml(CXMLDOMElement* pFatherNode);

	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;

	void   SetL_syncLostCounter(const WORD num);
	void   SetR_syncLostCounter(const WORD num);
	void   SetL_videoSyncLostCounter(const WORD num);
	void   SetR_videoSyncLostCounter(const WORD num);
	void   SetMuxBoardId(const WORD boardId);
	void   SetMuxUnitId(const WORD unitId);
	void   SetAudioCodecBoardId(const WORD boardId);
	void   SetAudioCodecUnitId(const WORD unitId);
	void   SetAudioBrgBoardId(const WORD boardId);
	void   SetAudioBrgUnitId(const WORD unitId);
	void   SetVideoBoardId(const WORD boardId);
	void   SetVideoUnitId(const WORD unitId);
	void   SetT120BoardId(const WORD boardId);
	void   SetT120UnitId(const WORD unitId);
	void   SetMCSBoardId(const WORD boardId);
	void   SetMCSUnitId(const WORD unitId);
	void   SetH323BoardId(const WORD boardId);
	void   SetH323UnitId(const WORD unitId);
 	

   
};
///////////////////////////////////////////////////////////////////////////////////
class CPartyAddBillingCode  : public CPObject, public ACCCDREventPartyAddBillingCode
{
CLASS_TYPE_1(CPartyAddBillingCode, CPObject)//**check macro**

public:
	//Constructors

	CPartyAddBillingCode();
	~CPartyAddBillingCode();

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
	void   SetBillingData(const char* pBillingData);       

	
};

////////////////////////////////////////////////////////////////////
class CGkInfo  : public CPObject, public CCDRPartyGkInfo
{
	CLASS_TYPE_1(CGkInfo, CPObject)//**check macro**
public:
	//Constructors

	CGkInfo();
	~CGkInfo();
	CGkInfo& operator=(const CGkInfo &other);
	bool operator == (const CGkInfo &rHnd);

	// Implementation
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);			

	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;
	void   SetPartyName(const char* h243partyname);                 
	void   SetPartyId(const DWORD partyid);                 
	void   SetGkCallId(const BYTE *gkCallId);
	
};
///////////////////////////////////////////////////////////////////////
//CCDRPartyNewRateInfo
class CNewRateInfo  : public CPObject, public CCDRPartyNewRateInfo
{
	CLASS_TYPE_1(CNewRateInfo, CPObject)//**check macro**
public:
	//Constructors
	

	CNewRateInfo();
	~CNewRateInfo();
	CNewRateInfo& operator=(const CNewRateInfo &other);
	bool operator == (const CNewRateInfo &rHnd);

	// Implementation
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);			

	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;
	void   SetPartyName(const char* h243partyname);                 
	void   SetPartyId(const DWORD partyid);                 
	void   SetPartyCurrentRate(const DWORD currentRate);
	
	
	
};

////////////////////////////////////////////////////////////////////
class CCDRPartyChairPerson  : public CPObject,public ACCCDREventPartyChairPerson
{
CLASS_TYPE_1(CPartyChairPerson, CPObject)//**check macro**
public:
	//Constructors

	CCDRPartyChairPerson();
	~CCDRPartyChairPerson();

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
	void   SetIsChairPerson(const bool bChair);

};
//////////////////////////////////////////////////////////////////////
//CCDRPartyCallInfo
class CCallInfo  : public CPObject, public CCDRPartyCallInfo
{
	CLASS_TYPE_1(CCallInfo, CPObject)//**check macro**
public:
	//Constructors
	CCallInfo();
	~CCallInfo();
	CCallInfo& operator=(const CCallInfo &other);
	bool operator == (const CCallInfo &rHnd);

	// Implementation
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);

	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;
	void   SetPartyName(const char* h243partyname);
	void   SetPartyId(const DWORD partyid);
	void   SetPartyMaxBitRate(const DWORD maxBitRate);
	void   SetPartyMaxResolution(const char *maxResolution);
	void   SetPartyMaxFrameRate(const char *maxFrameRate);
	//void   SetPartyMaxFrameRate(const WORD maxFrameRate);
	void   SetPartyAddress(const char* address);
};
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//                        PartyCorrelationData
////////////////////////////////////////////////////////////////////////////
class CPartyCorrelationData  : public CPObject, public CCDRPartyCorrelationDataInfo
{
	CLASS_TYPE_1(CPartyCorrelationData, CPObject)
public:
	//Constructors
	CPartyCorrelationData();
	//CPartyCorrelationData(const CPartyCorrelationData &other);
	~CPartyCorrelationData();
	CPartyCorrelationData& operator=(const CPartyCorrelationData &other);
	bool operator == (const CPartyCorrelationData &rHnd);

	// Implementation
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);

	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;
	void   SetPartyName(const char* h243partyname);
	void   SetPartyId(const DWORD partyid);
	void  SetSigUuid(const char*  sigUuid);
};

#endif /* _PARTY */		
