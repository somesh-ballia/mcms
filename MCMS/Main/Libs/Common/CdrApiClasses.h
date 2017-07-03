//+========================================================================+
//                            NRESERV.H                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       NRESERV.H                                                   |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Anatoly                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                     


#ifndef _CDR_API_CLASSES_H
#define _CDR_API_CLASSES_H


#include  <iostream>
#include  "PObject.h"
#include  "CDREvent.h"
#include "IVRAvMsgStruct.h"
////////////////////////////////////////////////////////////////
class CCardRsrsStruct  : public CPObject ,public CCardRsrsStructBase
{
CLASS_TYPE_1(CCardRsrsStruct, CPObject)  //**check macro**
public:
	//Constructors
	CCardRsrsStruct(){};
	//CCardRsrsStruct(const CCardRsrsStruct &other);
	~CCardRsrsStruct(){};
	
	// Implementation
	
	//char* Serialize(WORD format);	
	void Serialize(WORD format, std::ostream &m_ostr);	
	void DeSerialize(WORD format, std::istream &m_istr);			
	void SerializeXml(CXMLDOMElement* pFatherNode);
	int	 DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	void CdrSerialize(WORD format, std::ostream &m_ostr);
	void CdrSerialize(WORD format, std::ostream &m_ostr, BYTE bilflag);
	void CdrDeSerialize(WORD format, std::istream &m_istr);			
	
	const char*  NameOf() const;	
	void   SetAudioBoardId(const WORD audio_board_id);       
	void   SetAudioUnitId(const WORD audio_unit_id);      
	void   SetVideoBoardId(const WORD video_board_id);      
	void   SetVideoUnitId(const WORD video_unit_id);          
	void   SetDataBoardId(const WORD data_board_id);          
	void   SetDataUnitId(const WORD data_unit_id); 
	
	
	
};

/////////////////////////////////////////////////////////////////////////////
// CServicePhoneStr
class CServicePhoneStr : public CPObject
{
CLASS_TYPE_1(CServicePhoneStr,CPObject)
public:
	   //Constructors
	CServicePhoneStr();                   
	CServicePhoneStr(const CServicePhoneStr &other);                   
	CServicePhoneStr& operator = (const CServicePhoneStr& other);
	BYTE operator >= (const CServicePhoneStr& other);
	BYTE operator < (const CServicePhoneStr& other);
	virtual ~CServicePhoneStr();
	bool operator == (const CServicePhoneStr& other);
	
	
	   // Implementation
    void   Serialize(WORD format, std::ostream  &m_ostr);     
    void   DeSerialize(WORD format, std::istream &m_istr);    
    void   CdrSerialize(WORD format, std::ostream  &m_ostr);     
    void   CdrDeSerialize(WORD format, std::istream &m_istr);    
    void   CdrSerialize(WORD format, std::ostream  &m_ostr,DWORD apiNum);               
	void   SerializeXml(CXMLDOMElement* pFatherNode);
	int	   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	
    const char*  NameOf() const;                
	
	int    AddPhoneNumber(const char*  phoneNumber);                 
	int    CancelPhoneNumber(const char*  phoneNumber);                 
    int    FindPhoneNumber(const char*  phoneNumber)const;
	WORD   GetNumPhoneNumbers();
	Phone* GetFirstPhoneNumber() ;
	Phone* GetFirstPhoneNumber(int& nPos) ;
    Phone* GetNextPhoneNumber() ;
	Phone* GetNextPhoneNumber(int ind,int& nPos) ;
	
    void   SetNetServiceName(const char*  name);                 
    const char*  GetNetServiceName () const;

	BYTE IsUseServicePhonesAsRange()const;
	void SetUseServicePhonesAsRange(BYTE useServicePhonesAsRange);
	const char* GetPhonePrefixForward()const;
	void SetPhonePrefixForward(const char* phonePrefixForward);
	WORD GetPhoneNumDigitsForward()const;
	void SetPhoneNumDigitsForward(WORD phoneNumDigitsForward);
	
public:
	   // Attributes					
    char       m_netServiceName[NET_SERVICE_PROVIDER_NAME_LEN]; 
	WORD       m_numPhones; 
	Phone*     m_pPhoneNumberList[MAX_PHONE_NUMBERS_IN_CONFERENCE];
	
	BYTE    m_useServicePhonesAsRange;
	char    m_phonePrefixForward[SERVICE_PHONE_PREFIX_LEN];	//	WORD    m_phonePrefixForward;
	WORD    m_phoneNumDigitsForward;
private:
	WORD m_ind_phone;
};

/////////////////////////////////////////////////////////////////////////////
//  CLectureMode
class CLectureMode : public CPObject ,public ACCLectureMode
{
CLASS_TYPE_1(CLectureMode, CPObject)  //**check macro**
public:
	//Constructors
	CLectureMode(){};                   
	//CLectureMode(const  CLectureMode &other);                   
	virtual ~ CLectureMode(){};
//	CLectureMode&  operator = (const CLectureModeParams& other);
	// Implementation
	void   Serialize     (WORD format, std::ostream &m_ostr, DWORD apiNum);     
	void   DeSerialize   (WORD format, std::istream &m_istr, DWORD apiNum);    
	int    DeSerializeXml(CXMLDOMElement *pLectureModeNode,char *pszError);	
	void   SerializeXml(CXMLDOMElement *pFatherNode);
	void   CdrSerialize  (WORD format, std::ostream &m_ostr, DWORD apiNum);  
	void   CdrSerialize  (WORD format, std::ostream &m_ostr, DWORD apiNum, BYTE bilflag);               
	void   CdrDeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);    
	
	const char*  NameOf() const;    
	void    SetLectureModeOnOff(BYTE  OnOff);
	void    SetLecturerName(const char*  name);           
	void    SetLectureTimeInterval(WORD  TimeInterval);
	void    SetTimerOnOff(BYTE  timerOnOff);
	void    SetAudioActivated(BYTE  audioActivated);
	void    SetLecturerId(DWORD  lecturerId);

};


////////////////////////////////////////////////////////////////////////////
#endif /* _CDR_API_CLASSES_H */



