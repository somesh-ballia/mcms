//+========================================================================+
//                       PcmCommands.h                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PcmCommands.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Eitan                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  August - 2007  | Description                               |
//-------------------------------------------------------------------------|
//			
//+========================================================================++

#ifndef _PCM_COMMANDS_H_
#define _PCM_COMMANDS_H_

#include <vector>
#include <map>
#include "PcmMessage.h"


typedef map<string,int> strIntMap;

class CPcmPopMenuStatusCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmPopMenuStatusCommand,CPcmCommand)

public:
	CPcmPopMenuStatusCommand(int termId = 0);
	virtual ~CPcmPopMenuStatusCommand();
	CPcmPopMenuStatusCommand(CPcmPopMenuStatusCommand& other);
	virtual const char*  NameOf() const{ return "CPcmPopMenuStatusCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmPopMenuStatusCommand;}
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	
	virtual void Dump();
	int GetStatus(){return status;}

protected:
	int status;

};

class CPcmSetAllAudioMuteInCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetAllAudioMuteInCommand,CPcmCommand)

public:
	CPcmSetAllAudioMuteInCommand(int termId = 0);
	virtual ~CPcmSetAllAudioMuteInCommand();
	CPcmSetAllAudioMuteInCommand(CPcmSetAllAudioMuteInCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetAllAudioMuteInCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetAllAudioMuteInCommand;}
	
	bool GetValue(){return m_value;}
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
		
protected:
	
	bool m_value; //true=On false=Off
	

};

class CPcmSetFocusCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetFocusCommand,CPcmCommand)

public:
	CPcmSetFocusCommand(int termId = 0);
	virtual ~CPcmSetFocusCommand();
	CPcmSetFocusCommand(CPcmSetFocusCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetFocusCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetFocusCommand;}
	
	int GetFocusPos(){return m_focusPos;}
	void SetFocusPos(int focusPos){m_focusPos = focusPos;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
		
protected:
	int m_focusPos;

};

class CPcmSetCpLayoutCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetCpLayoutCommand,CPcmCommand)

public:
	CPcmSetCpLayoutCommand(int termId = 0);
	virtual ~CPcmSetCpLayoutCommand();
	CPcmSetCpLayoutCommand(CPcmSetCpLayoutCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetCpLayoutCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetCpLayoutCommand;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	
	BYTE GetApiLayoutType();
	int GetLayoutType(){return m_layoutType;}
	
protected:
	int m_layoutType;

};

class CPcmSetAudioMuteInCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetAudioMuteInCommand,CPcmCommand)

public:
	CPcmSetAudioMuteInCommand(int termId = 0);
	virtual ~CPcmSetAudioMuteInCommand();
	CPcmSetAudioMuteInCommand(CPcmSetAudioMuteInCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetAudioMuteInCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetAudioMuteInCommand;}
	
	string GetTermToMuteName(){return m_TermToMuteName;}
	bool GetValue(){return m_value;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
		
protected:
	
	string m_TermToMuteName;
	bool m_value;  //true=On false=Off

};

class CPcmSetAudioMuteOutCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetAudioMuteOutCommand,CPcmCommand)

public:
	CPcmSetAudioMuteOutCommand(int termId = 0);
	virtual ~CPcmSetAudioMuteOutCommand();
	CPcmSetAudioMuteOutCommand(CPcmSetAudioMuteOutCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetAudioMuteOutCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetAudioMuteOutCommand;}
	
	string GetTermToMuteName(){return m_TermToMuteName;}
	bool GetValue(){return m_value;}
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
			
protected:
	
	string m_TermToMuteName;
	bool m_value;  //true=On false=Off

};

class CPcmSetVideoMuteInCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetVideoMuteInCommand,CPcmCommand)

public:
	CPcmSetVideoMuteInCommand(int termId = 0);
	virtual ~CPcmSetVideoMuteInCommand();
	CPcmSetVideoMuteInCommand(CPcmSetVideoMuteInCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetVideoMuteInCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetVideoMuteInCommand;}
	
	string GetTermToMuteName(){return m_TermToMuteName;}
	bool GetValue(){return m_value;}
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
			
protected:
		
	string m_TermToMuteName;
	bool m_value;  //true=On false=Off	

};

class CPcmSetInviteTermCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetInviteTermCommand,CPcmCommand)

public:
	CPcmSetInviteTermCommand(int termId = 0);
	virtual ~CPcmSetInviteTermCommand();
	CPcmSetInviteTermCommand(CPcmSetInviteTermCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetInviteTermCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetInviteTermCommand;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	bool IsFromAddressBook(){ return m_fromAddressBook; }
	strIntMap* GetInvitedTermDetails() { return &m_invitedTermDetails;}
protected:
	
	strIntMap m_invitedTermDetails;
	bool m_fromAddressBook;
	
};

class CPcmSetDropTermCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetDropTermCommand,CPcmCommand)

public:
	CPcmSetDropTermCommand(int termId = 0);
	virtual ~CPcmSetDropTermCommand();
	CPcmSetDropTermCommand(CPcmSetDropTermCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetDropTermCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetDropTermCommand;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	
	vector<string>* GetNamesVector(){return &m_names_to_drop;}
	bool GetKickAll(){return m_kickAll;}
	
protected:
	
	vector<string> m_names_to_drop;
	bool m_kickAll;
	

};

class CPcmStopConfCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmStopConfCommand,CPcmCommand)

public:
	CPcmStopConfCommand(int termId = 0);
	virtual ~CPcmStopConfCommand();
	CPcmStopConfCommand(CPcmStopConfCommand& other);
	virtual const char*  NameOf() const{ return "CPcmStopConfCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmStopConfCommand;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
		
protected:
	

};

class CPcmFeccControlCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmFeccControlCommand,CPcmCommand)

public:
	CPcmFeccControlCommand(int termId = 0);
	virtual ~CPcmFeccControlCommand();
	CPcmFeccControlCommand(CPcmFeccControlCommand& other);
	virtual const char*  NameOf() const{ return "CPcmFeccControlCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmFeccControlCommand;}
	
	int GetPaneIndex(){return m_paneIndex;}
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
		
protected:
	
	int m_paneIndex; 
	

};

class CPcmSetConfLayoutTypeCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetConfLayoutTypeCommand,CPcmCommand)

public:
	CPcmSetConfLayoutTypeCommand(int termId = 0);
	virtual ~CPcmSetConfLayoutTypeCommand();
	CPcmSetConfLayoutTypeCommand(CPcmSetConfLayoutTypeCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetConfLayoutTypeCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetConfLayoutTypeCommand;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
		
	int GetLayoutMode(){return m_layoutMode;}
	int GetLectureRole(){return m_lectoreRole;}
	
protected:
	
	int m_layoutMode;
	int m_lectoreRole;
};

class CPcmRecordCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmRecordCommand,CPcmCommand)

public:
	CPcmRecordCommand(int termId = 0);
	virtual ~CPcmRecordCommand();
	CPcmRecordCommand(CPcmRecordCommand& other);
	virtual const char*  NameOf() const{ return "CPcmRecordCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmRecordCommand;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	
	string GetCmd(){return cmd;}
	DWORD GetCommandValue(string cmd);
	static bool InitMap();
	
protected:
	string cmd;
	static strIntMap cmdStrToMcmsOpcode;

};

class CPcmLocalAddrBookCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmLocalAddrBookCommand,CPcmCommand)

public:
	CPcmLocalAddrBookCommand(int termId = 0);
	virtual ~CPcmLocalAddrBookCommand();
	CPcmLocalAddrBookCommand(CPcmLocalAddrBookCommand& other);
	virtual const char*  NameOf() const{ return "CPcmLocalAddrBookCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmLocalAddrBookCommand;}

	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

	string GetMatchStr(){return m_matchStr;}
	string GetMatchStrEnd(){return m_matchStrEnd;}
	int GetCount(){return m_count;}

protected:

	string m_matchStr;
	string m_matchStrEnd;
	int m_count;
};

class CPcmSetDisplaySettingCommand : public CPcmCommand {
CLASS_TYPE_1(CPcmSetDisplaySettingCommand,CPcmCommand)

public:
	CPcmSetDisplaySettingCommand(int termId = 0);
	virtual ~CPcmSetDisplaySettingCommand();
	CPcmSetDisplaySettingCommand(CPcmSetDisplaySettingCommand& other);
	virtual const char*  NameOf() const{ return "CPcmSetDisplaySettingCommand"; }
	virtual CPcmCommand* Clone() {return new CPcmSetDisplaySettingCommand;}

	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

	string GetPaneType(){return m_paneType;}
	int GetPaneIndex(){return m_paneIndex;}

protected:

	string m_paneType;
	int m_paneIndex;
};

class CPcmCommandDummy : public CPcmCommand {
CLASS_TYPE_1(CPcmCommandDummy,CPcmCommand)

public:
	CPcmCommandDummy(int termId = 0);
	virtual ~CPcmCommandDummy();
	CPcmCommandDummy(CPcmCommandDummy& other);
	virtual const char*  NameOf() const{ return "CPcmCommandDummy"; }
	virtual CPcmCommand* Clone() {return new CPcmCommandDummy;}
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap

};

#endif //_PCM_COMMANDS_H_
