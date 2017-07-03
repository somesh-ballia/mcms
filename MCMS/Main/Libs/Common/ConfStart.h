/*$Header: /MCMS/MAIN/subsys/mcmsoper/CONFSTRT.H 9     9/08/01 9:53 Hagai $*/
//+========================================================================+
//                            CONFSTRT.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CONFSTRT.H                                                  |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#ifndef _CONFSTRT
#define _CONFSTRT

/*
#ifdef __HIGHC__

#ifndef _POBJECT
#include  <pobject.h>
#endif

#ifndef _MCMSOPER
#include  <mcmsoper.h>
#include  <nreserv.h>
#endif
#include  <cdrevent.h>


#else

#include  "Mcmsoper.h"
#include  "Pobject.h"
#include  "cdrevent.h"
#endif

#include  <strstrea.h>
#include  <iostream.h>
*/


#include "PObject.h"
#include "CDREvent.h"
#include "LectureModeParams.h"
//#include "CDREvent.h"
#include <iostream>

//class COstrStream;
//class CIstrStream;
class CServicePhoneStr;
class CCardRsrsStruct;
class CAvMsgStruct;
class CLectureMode;
class CXMLDOMElement;

// CConfStart 
	 

class CConfStart  : public CPObject ,public ACCCDREventConfStart
{
CLASS_TYPE_1(CConfStart, CPObject)  //**check macro**
public:
	//Constructors
	CConfStart();
	//CConfStart(const CConfStart &other);
	~CConfStart();

	// Implementation

	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr);	
	void Serialize(WORD format, std::ostream &m_ostr,BYTE bilflag);	
	void DeSerialize(WORD format, std::istream &m_istr);		

//#ifdef __HIGHC__
	void SerializeXml(CXMLDOMElement* pFatherNode);
//#endif
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;
	void   SetStandBy(const BYTE stand_by);

	void   SetAutoTerminate(const BYTE autoterminate);               
	void   SetConfTransfRate(const BYTE conf_trans_rate);                
	void   SetRestrictMode(const BYTE restrict_mode);        
	void   SetAudioRate(const BYTE audio_rate);      
	void   SetVideoSession(const BYTE video_session);       
	void   SetVideoPicFormat(const BYTE video_pic_format);
	void   SetCifFrameRate (const BYTE cif_frame_rate);
	void   SetQcifFrameRate (const BYTE Qcif_frame_rate);           
	void   SetLsdRate(const BYTE lsd_rate);         
	void   SetHsdRate(const BYTE hsd_rate);
	void   SetT120BitRate (const BYTE t120_bit_rate);

		
};



class CConfStartCont1  : public CPObject ,public ACCCDREventConfStartCont1
{
	CLASS_TYPE_1(CConfStartCont1, CPObject)//**check macro**
public:
	//Constructors
	CConfStartCont1();
	//CConfStartCont1(const CConfStartCont1 &other);
	~CConfStartCont1();
	
	bool operator == (const CConfStartCont1 &other);
	
	// Implementation

	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr,DWORD apiNum);	
	void Serialize(WORD format, std::ostream &m_ostr,BYTE bilflag,DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr,DWORD apiNum);	

//#ifdef __HIGHC__
	void SerializeXml(CXMLDOMElement* pFatherNode);
//#endif
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;

	void SetAudioTone (const DWORD audioTone);
	void SetAlertToneTiming (const BYTE alertToneTiming);
	void SetTalkHoldTime (const WORD talkHoldTime);
	void SetAudioMixDepth (const BYTE  audioMixDepth);
	void SetOperatorConf(const BYTE	operatorConf);
	void SetVideoProtocol (const BYTE videoProtocol);	  
	void SetMeetMePerConf (const BYTE meetMePerConf);
	void SetNumServicePhone(const WORD num);
	int  AddServicePhone(const CServicePhoneStr &other);
	void SetConf_password (const char*  conf_password);
	void SetChairMode (const BYTE  chairMode);
	void SetCascadeMode (const BYTE  cascadeMode);
	void SetMasterName (const char*    masterName );
	void SetNumUndefParties (const WORD numUndefParties);
	void SetUnlimited_conf_flag(const BYTE  unlimited_conf_flag);
	void SetTime_beforeFirstJoin(const BYTE  time_beforeFirstJoin);
	void SetTime_afterLastQuit(const BYTE  time_afterLastQuit);
	void SetConfLockFlag(const BYTE  confLockFlag);
	void SetMax_parties(const WORD  max_parties);
	void SetCardRsrsStruct(const CCardRsrsStruct &CardRsrsStruct) ; 
	void SetAvMsgStruct(const CAvMsgStruct& otherAvMsgStruct);
	void SetLectureMode(const CLectureMode& otherLectureMode);
	void SetLectureMode(const CLectureModeParams& otherLectureMode);	
};
class CConfStartCont2  : public CPObject ,public ACCCDREventConfStartCont2
{
CLASS_TYPE_1(CConfStartCont2, CPObject)//**check macro**
public:

	CConfStartCont2();
	~CConfStartCont2();

	// Implementation

	char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr,DWORD apiNum);	
	void Serialize(WORD format, std::ostream &m_ostr,BYTE bilflag,DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr,DWORD apiNum);		

//#ifdef __HIGHC__
	void SerializeXml(CXMLDOMElement* pFatherNode);
//#endif
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;

	
	void SetwebReserved  (const BYTE webReserved);
	void SetwebReservUId (const DWORD webReservUId);
	void SetwebDBId      (const DWORD webDBId);
};

class CConfStartCont3  : public CPObject ,public ACCCDREventConfStartCont3
{
CLASS_TYPE_1(CConfStartCont3, CPObject)//**check macro**
public:
	CConfStartCont3();
	~CConfStartCont3();
	
	// Implementation
	
	char* Serialize(WORD format);	
	void  Serialize(WORD format, std::ostream &m_ostr,DWORD apiNum);	
	void  Serialize(WORD format, std::ostream &m_ostr,BYTE bilflag,DWORD apiNum);	
	void  DeSerialize(WORD format, std::istream &m_istr,DWORD apiNum);

//#ifdef __HIGHC__
	void SerializeXml(CXMLDOMElement* pFatherNode);
//#endif
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char* NameOf() const;
	
	void SetConfRemarks (const char* remarks);
};

////////////////////////////////////////////////////////////////////
class CConfStartCont4  : public CPObject , public ACCCDREventConfStartCont4
{
CLASS_TYPE_1(CConfStartCont4, CPObject)//**check macro**
public:
	//Constructors

	CConfStartCont4();
	~CConfStartCont4();

	// Implementation
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
	void Serialize(WORD format, std::ostream &m_ostr, BYTE flag, DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);	
    const char*  NameOf() const;	

//#ifdef __HIGHC__
	void SerializeXml(CXMLDOMElement* pFatherNode);
//#endif
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

    void SetContactInfo(const char* info,int ContactNumber);                 
	void SetNumericConfId (const char* NumericConfId );
	void SetUser_password (const char* user_password );
	void SetChair_password (const char* chair_password );
	void SetConfBillingInfo(const char* BillingInfo);

};

////////////////////////////////////////////////////////////////////
class CConfStartCont5  : public CPObject, public ACCCDREventConfStartCont5
{
CLASS_TYPE_1(CConfStartCont5, CPObject)//**check macro**
public:
    //Constructors
    CConfStartCont5();
    ~CConfStartCont5(); 

	// Implementation
	void Serialize(WORD format, std::ostream& ostr, DWORD apiNum);    
	void Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum);
	void DeSerialize(WORD format, std::istream& istr, DWORD apiNum);   
	const char*  NameOf() const;
	
	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError);

	void SetIsEncryptedConf(const BYTE is_encryption);
};


////////////////////////////////////////////////////////////////////
class CConfStartCont10  : public CPObject , public ACCCDREventConfStartCont10
{
CLASS_TYPE_1(CConfStartCont10, CPObject)//**check macro**
public:
	//Constructors

	CConfStartCont10();
	~CConfStartCont10();

	// Implementation
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);	
	void Serialize(WORD format, std::ostream &m_ostr, BYTE flag, DWORD apiNum);	
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);	
    const char*  NameOf() const;	

//#ifdef __HIGHC__
	void SerializeXml(CXMLDOMElement* pFatherNode);
//#endif
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

    void SetConfDisplayName(const char* confDisplayName);                 
    void SetAvcSvc (const BYTE eAvcSvc);
};
////////////////////////////////////////////////////////////////////
#endif /* _CONFSTRT */
