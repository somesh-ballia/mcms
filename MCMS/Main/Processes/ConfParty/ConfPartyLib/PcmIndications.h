//+========================================================================+
//                       PcmIndications.h                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PcmIndications.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Eitan                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  August - 2007  | Description                               |
//-------------------------------------------------------------------------|
//			
//+========================================================================++

#ifndef _PCMINDICATION_H_
#define _PCMINDICATION_H_

#include "PcmMessage.h"
#include <vector>


class CCommConf;
class CConfParty;
class CRsrvParty;
class CLayout;
class CVideoLayout;

using namespace std;

class CPcmControlKeyIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmControlKeyIndication,CPcmIndication)

public:
	CPcmControlKeyIndication(int termId = 0);
	virtual ~CPcmControlKeyIndication();
	CPcmControlKeyIndication(CPcmControlKeyIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	
	void SetKey(string key){m_key = key;}

protected:
	 	
	string m_key;

};
/////////////////////////////////////////////////////////////////////////////
class CPcmInitialMenuBufferIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmInitialMenuBufferIndication,CPcmIndication)

public:
	CPcmInitialMenuBufferIndication(int termId = 0);
	virtual ~CPcmInitialMenuBufferIndication();
	CPcmInitialMenuBufferIndication(CPcmInitialMenuBufferIndication& other);
	virtual const char*  NameOf() const;
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

protected:
};
/////////////////////////////////////////////////////////////////////////////
class CPcmMenuStateIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmMenuStateIndication,CPcmIndication)

public:
	CPcmMenuStateIndication(int termId = 0,bool isActive = true, bool isDisenable = true , bool isCopActive = true);
	virtual ~CPcmMenuStateIndication();
	CPcmMenuStateIndication(CPcmMenuStateIndication& other);
	virtual const char*  NameOf() const;
			
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

	void SetIsActive(bool isActive){m_isActive = isActive;}
	void SetIsDisenable(bool isDisenable){m_isDisenable = isDisenable;}
	void SetIsCopActive(bool isCopActive){m_isCopActive = isCopActive;}
	
	bool GetIsActive(){return m_isActive;}
	bool GetIsDisenable(){return m_isDisenable;}
	bool GetIsCopActive(){return m_isCopActive;}
	
protected:
	bool m_isActive;
	bool m_isDisenable;
	bool m_isCopActive;
};
/////////////////////////////////////////////////////////////////////////////
class CPcmImageSizeIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmImageSizeIndication,CPcmIndication)

public:
	CPcmImageSizeIndication(int termId = 0);
	virtual ~CPcmImageSizeIndication();
	CPcmImageSizeIndication(CPcmImageSizeIndication& other);
	virtual const char*  NameOf() const;
		
	void SetImageType(string type){m_type = type;}
	void SetImageRatio(string ratio){m_ratio = ratio;}
	
	string GetType(){return m_type;}
	string GetRatio(){return m_ratio;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

protected:
	string m_type;
	string m_ratio;
};

/////////////////////////////////////////////////////////////////////////////
class CPcmConfLayoutModeInfoIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmConfLayoutModeInfoIndication,CPcmIndication)

public:
	CPcmConfLayoutModeInfoIndication(int termId = 0);
	virtual ~CPcmConfLayoutModeInfoIndication();
	CPcmConfLayoutModeInfoIndication(CPcmConfLayoutModeInfoIndication& other);
	virtual const char*  NameOf() const;
		
	//int FillTerminalNamesVector(CCommConf* pCommConf);
	BYTE FillOngoingPartiesVector(CCommConf* pCommConf);
	void SetLayoutParams(CCommConf* pCommConf);
	void SetPrivateLayout(CVideoLayout* privateLayout);
	//int TranslateLayoutTypeToPcmApi(LayoutType mcmsLayoutType);
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

protected:
	int m_layoutMode;	   //0-lecture, 1-samelayout, 3-discuss
	bool m_enableCpLayout; //true
	int m_cpLayoutType;	
	//101 201 202 203 204
	//301 302 303 304 305
	//401 402 403 404 405
	//501 502 503
	//601 602 603 604 
	//701
	//801
	//901 902 903 904
	//1001
	//1301
	//1601

	int m_lectureRole;
	
	vector<CConfParty*> m_ongoingParties;
	
	
};
/////////////////////////////////////////////////////////////////////////////
class CPcmServiceSettingIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmServiceSettingIndication,CPcmIndication)

public:
	CPcmServiceSettingIndication(int termId = 0,int setting = 2);
	virtual ~CPcmServiceSettingIndication();
	CPcmServiceSettingIndication(CPcmServiceSettingIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	
	void SetServiceSetting(int setting){m_setting=setting;}
	int  GetServiceSettings(){return m_setting;}

protected:
	int m_setting; //0--H323 1--sip 2-all
};


/////////////////////////////////////////////////////////////////////////////
class CPcmTermListInfoIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmTermListInfoIndication,CPcmIndication)

public:
	CPcmTermListInfoIndication(int termId = 0,CCommConf* pCommConf = NIL(CCommConf));
	virtual ~CPcmTermListInfoIndication();
	CPcmTermListInfoIndication(CPcmTermListInfoIndication& other);
	virtual const char*  NameOf() const;
	
	BYTE FillOngoingPartiesVector(CCommConf* pCommConf); 
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

protected:
	vector<CConfParty*> m_ongoingParties;
};
/////////////////////////////////////////////////////////////////////////////
class CPcmAllAudioMuteStateIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmAllAudioMuteStateIndication,CPcmIndication)

public:
	CPcmAllAudioMuteStateIndication(int termId = 0);
	virtual ~CPcmAllAudioMuteStateIndication();
	CPcmAllAudioMuteStateIndication(CPcmAllAudioMuteStateIndication& other);
	virtual const char*  NameOf() const;
		
	void CalculateAllMuteFlag(CCommConf* pCommConf);
	void SetAllMuteButXFlag(BYTE onOff);
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

protected:
	bool m_AreAllMuted;
};
/////////////////////////////////////////////////////////////////////////////
class CPcmDirectLoginConfIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmDirectLoginConfIndication,CPcmIndication)

public:
	CPcmDirectLoginConfIndication(int termId = 0);
	virtual ~CPcmDirectLoginConfIndication();
	CPcmDirectLoginConfIndication(CPcmDirectLoginConfIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

	void InitInternalParams(string confName,int confId, string termName,bool isPresider = true);
	void SetConfName(string confName){m_confName = confName;}
	void SetConfId(int confId){m_confId = confId;}
	void SetTermName(string termName){m_termName = termName;}
	void SetIsPresider(bool isPresider){m_isPresider = isPresider;}
	
	string GetConfName(){return m_confName;}
	int GetConfId() {return m_confId;}
	string GetTermName(){return m_termName;}
	bool GetIsPresider(){return m_isPresider;}
	
protected:
	string m_confName;
	int m_confId;
	string m_termName;
	bool m_isPresider;
	

};
////////////////////////////////////////////////////////////////////////////
class CPcmConfInfoIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmConfInfoIndication,CPcmIndication)

public:
	CPcmConfInfoIndication(int termId = 0);
	virtual ~CPcmConfInfoIndication();
	CPcmConfInfoIndication(CPcmConfInfoIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

	void InitInternalParams(CCommConf* pCommConf);
	void SetConfName(string confName){m_confName = confName;}
	void SetConfId(int confId){m_confId = confId;}
			
	string GetConfName(){return m_confName;}
	int GetConfId() {return m_confId;}
	
		
protected:
	string m_confName;
	int m_confId;
	string m_confPassword;
	string m_presiderPassword;
	int m_participantsCount;
	bool m_isLocked;
	bool m_isEncrypted;
	string m_textInfo;
	bool m_isSwitching;
	int  m_skinType;
	bool m_enableRecord;

};
/////////////////////////////////////////////////////////////////////////////
class CPcmPresiderToParticipantIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmPresiderToParticipantIndication,CPcmIndication)

public:
	CPcmPresiderToParticipantIndication(int termId = 0);
	virtual ~CPcmPresiderToParticipantIndication();
	CPcmPresiderToParticipantIndication(CPcmPresiderToParticipantIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

protected:
};
/////////////////////////////////////////////////////////////////////////////
class CPcmParticipantToPresiderIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmParticipantToPresiderIndication,CPcmIndication)

public:
	CPcmParticipantToPresiderIndication(int termId = 0);
	virtual ~CPcmParticipantToPresiderIndication();
	CPcmParticipantToPresiderIndication(CPcmParticipantToPresiderIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

protected:
};
/////////////////////////////////////////////////////////////////////////////
class CPcmInviteResultIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmInviteResultIndication,CPcmIndication)

public:
	CPcmInviteResultIndication(int termId = 0);
	virtual ~CPcmInviteResultIndication();
	CPcmInviteResultIndication(CPcmInviteResultIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	
	void SetResult(bool res){m_res = res;}
protected:
	bool m_res;
};
/////////////////////////////////////////////////////////////////////////////
class CPcmFeccEndIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmFeccEndIndication,CPcmIndication)

public:
	CPcmFeccEndIndication(int termId = 0);
	virtual ~CPcmFeccEndIndication();
	CPcmFeccEndIndication(CPcmFeccEndIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

protected:
};
/////////////////////////////////////////////////////////////////////////////
class CPcmRecordStateIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmRecordStateIndication,CPcmIndication)

public:
	CPcmRecordStateIndication(int termId = 0);
	virtual ~CPcmRecordStateIndication();
	CPcmRecordStateIndication(CPcmRecordStateIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

	string GetRecordingState(){return m_recordingState;}
	void SetRecordingStateStrFromWord(WORD recordingState);
	
protected:
	string m_recordingState;
};
/////////////////////////////////////////////////////////////////////////////
class CPcmTerminalEndCallIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmTerminalEndCallIndication,CPcmIndication)

public:
	CPcmTerminalEndCallIndication(int termId = 0);
	virtual ~CPcmTerminalEndCallIndication();
	CPcmTerminalEndCallIndication(CPcmTerminalEndCallIndication& other);
	virtual const char*  NameOf() const;
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

protected:
};
/////////////////////////////////////////////////////////////////////////////
class CPcmLanguageSettingIndication : public CPcmIndication {
CLASS_TYPE_1(CPcmLanguageSettingIndication,CPcmIndication)

public:
	CPcmLanguageSettingIndication(int termId = 0);
	virtual ~CPcmLanguageSettingIndication();
	CPcmLanguageSettingIndication(CPcmLanguageSettingIndication& other);
	virtual const char*  NameOf() const;

	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

	string GetLanguage(){return m_lang;}
	void SetLanguageFromSysConfig();

protected:
	string m_lang;  //  "English" "ChineseSimplified" "Japanese" "German" "French"  "ChineseTraditional"  "Spanish"  "Korean"  "Portuguese"   "Italian"  "Russian"  "Norwegian"
};

class CPcmLocalAddressBookIndication : public CPcmIndication
{
CLASS_TYPE_1(CPcmLocalAddressBookIndication,CPcmIndication)

public:
	CPcmLocalAddressBookIndication(int termId = 0);
	virtual ~ CPcmLocalAddressBookIndication();
	CPcmLocalAddressBookIndication(CPcmLocalAddressBookIndication& other);
	virtual const char*  NameOf() const;

	virtual void SerializeXmlToStr(strmap& mpMsg, string& str);       // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);                       // build the object from strmap

	void SetIsFinished(bool isFinished){m_isFinished = isFinished;}
	void SetSequenceNum(int sequence){m_sequence = sequence;}
	void AddPartyToVector(CRsrvParty* pParty){m_parties.push_back(pParty);}
	void SetCount(int count){m_count = count;}

private:
	bool m_isFinished;
	int  m_sequence;
	int  m_count;

	vector<CRsrvParty*> m_parties;

};

#endif //_PCMINDICATION_H_
