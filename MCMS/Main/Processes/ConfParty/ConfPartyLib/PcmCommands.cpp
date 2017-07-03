#include <stdlib.h>
#include "PcmCommands.h"
#include "ObjString.h"
#include "ConfPartyDefines.h"

WORD GetOldLayoutType(const LayoutType layoutType);

////////////////////////////////////////////////////////////////////////
// 						CPcmPopMenuStatusCommand
////////////////////////////////////////////////////////////////////////
CPcmPopMenuStatusCommand::CPcmPopMenuStatusCommand(int termId):CPcmCommand(termId)
{
	action_name = "pop_menu_status";
	status = 0;
}
////////////////////////////////////////////////////////////////////////
CPcmPopMenuStatusCommand::CPcmPopMenuStatusCommand(CPcmPopMenuStatusCommand& other):CPcmCommand(other)
{
	status = other.status;
}
////////////////////////////////////////////////////////////////////////
CPcmPopMenuStatusCommand::~CPcmPopMenuStatusCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmPopMenuStatusCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	mpMsg["STATUS"]=itoa(status);
	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmPopMenuStatusCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	status = atoi(mpMsg["STATUS"].c_str());
}
////////////////////////////////////////////////////////////////////////
void CPcmPopMenuStatusCommand::Dump() // build the object from strmap
{
	CLargeString cstr;
	cstr << "CPcmPopMenuStatusCommand::Dump\n";
	cstr << "SOURCE_GUID:" << source_guid << "\n";
	cstr << "TARGET_GUID:" << target_guid << "\n";
	cstr << "TERM_ID:" << term_id << "\n";
	cstr << "_xml_msg_name:" << MsgTypeToString(type) << "\n";
	cstr << "_xml_msg_id:" << action_name.c_str() << "\n";
	cstr << "STATUS:" << status;
	
	PTRACE(eLevelInfoNormal,cstr.GetString());
}
////////////////////////////////////////////////////////////////////////
// 						CPcmSetAllAudioMuteInCommand
////////////////////////////////////////////////////////////////////////
CPcmSetAllAudioMuteInCommand::CPcmSetAllAudioMuteInCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_all_audio_mute_in";
	m_value = false;
}
////////////////////////////////////////////////////////////////////////
CPcmSetAllAudioMuteInCommand::CPcmSetAllAudioMuteInCommand(CPcmSetAllAudioMuteInCommand& other):CPcmCommand(other)
{
	
}
////////////////////////////////////////////////////////////////////////
CPcmSetAllAudioMuteInCommand::~CPcmSetAllAudioMuteInCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmSetAllAudioMuteInCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmSetAllAudioMuteInCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	m_value = GetNodeValueBool(mpMsg,"VALUE");

}
////////////////////////////////////////////////////////////////////////
// 						CPcmSetFocusCommand
////////////////////////////////////////////////////////////////////////
CPcmSetFocusCommand::CPcmSetFocusCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_focus";
	m_focusPos = 0;
}
////////////////////////////////////////////////////////////////////////
CPcmSetFocusCommand::CPcmSetFocusCommand(CPcmSetFocusCommand& other):CPcmCommand(other)
{
	m_focusPos = other.m_focusPos;
}
////////////////////////////////////////////////////////////////////////
CPcmSetFocusCommand::~CPcmSetFocusCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmSetFocusCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmSetFocusCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	m_focusPos = GetNodeValueInteger(mpMsg,"FOCUS_POS");
	
}
////////////////////////////////////////////////////////////////////////
// 						CPcmSetCpLayoutCommand
////////////////////////////////////////////////////////////////////////
CPcmSetCpLayoutCommand::CPcmSetCpLayoutCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_cp_layout";
	m_layoutType = 101;
}
////////////////////////////////////////////////////////////////////////
CPcmSetCpLayoutCommand::CPcmSetCpLayoutCommand(CPcmSetCpLayoutCommand& other):CPcmCommand(other)
{
	m_layoutType = other.m_layoutType;
}
////////////////////////////////////////////////////////////////////////
CPcmSetCpLayoutCommand::~CPcmSetCpLayoutCommand()
{
}
////////////////////////////////////////////////////////////////////////
BYTE CPcmSetCpLayoutCommand::GetApiLayoutType()
{
	LayoutType mcmsLayoutType = TranslatePcmApiLayoutTypeToMcmsLayoutType(m_layoutType);
	if (mcmsLayoutType != CP_NO_LAYOUT)
		return (::GetOldLayoutType(mcmsLayoutType));
	else
		return CP_NO_LAYOUT;
		
}
////////////////////////////////////////////////////////////////////////
void CPcmSetCpLayoutCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmSetCpLayoutCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	m_layoutType = GetNodeValueInteger(mpMsg,"VALUE");
}
////////////////////////////////////////////////////////////////////////
// 						CPcmSetAudioMuteInCommand
////////////////////////////////////////////////////////////////////////
CPcmSetAudioMuteInCommand::CPcmSetAudioMuteInCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_audio_mute_in";
	m_TermToMuteName ="";
	m_value = false;
}
////////////////////////////////////////////////////////////////////////
CPcmSetAudioMuteInCommand::CPcmSetAudioMuteInCommand(CPcmSetAudioMuteInCommand& other):CPcmCommand(other)
{
	m_TermToMuteName = other.m_TermToMuteName;
	m_value = other.m_value;
}
////////////////////////////////////////////////////////////////////////
CPcmSetAudioMuteInCommand::~CPcmSetAudioMuteInCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmSetAudioMuteInCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmSetAudioMuteInCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	m_value = GetNodeValueBool(mpMsg,"VALUE");
	m_TermToMuteName = GetNodeValueString(mpMsg,"TERM_NAME");
	
}
////////////////////////////////////////////////////////////////////////
// 						CPcmSetAudioMuteOutCommand
////////////////////////////////////////////////////////////////////////
CPcmSetAudioMuteOutCommand::CPcmSetAudioMuteOutCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_audio_mute_out";  //block audio
	m_TermToMuteName ="";
	m_value = false;
}
////////////////////////////////////////////////////////////////////////
CPcmSetAudioMuteOutCommand::CPcmSetAudioMuteOutCommand(CPcmSetAudioMuteOutCommand& other):CPcmCommand(other)
{
	m_TermToMuteName = other.m_TermToMuteName;
	m_value = other.m_value;
}
////////////////////////////////////////////////////////////////////////
CPcmSetAudioMuteOutCommand::~CPcmSetAudioMuteOutCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmSetAudioMuteOutCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmSetAudioMuteOutCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	m_value = GetNodeValueBool(mpMsg,"VALUE");
	m_TermToMuteName = GetNodeValueString(mpMsg,"TERM_NAME");
}
////////////////////////////////////////////////////////////////////////
// 						CPcmSetVideoMuteInCommand
////////////////////////////////////////////////////////////////////////
CPcmSetVideoMuteInCommand::CPcmSetVideoMuteInCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_video_mute_in";
	m_TermToMuteName ="";
	m_value = false;
}
////////////////////////////////////////////////////////////////////////
CPcmSetVideoMuteInCommand::CPcmSetVideoMuteInCommand(CPcmSetVideoMuteInCommand& other):CPcmCommand(other)
{
	m_TermToMuteName = other.m_TermToMuteName;
	m_value = other.m_value;
}
////////////////////////////////////////////////////////////////////////
CPcmSetVideoMuteInCommand::~CPcmSetVideoMuteInCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmSetVideoMuteInCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmSetVideoMuteInCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	m_value = GetNodeValueBool(mpMsg,"VALUE");
	m_TermToMuteName = GetNodeValueString(mpMsg,"TERM_NAME");
}
////////////////////////////////////////////////////////////////////////
// 						CPcmSetInviteTermCommand
////////////////////////////////////////////////////////////////////////
CPcmSetInviteTermCommand::CPcmSetInviteTermCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_invite_term";
	m_fromAddressBook = false;
}
////////////////////////////////////////////////////////////////////////
CPcmSetInviteTermCommand::CPcmSetInviteTermCommand(CPcmSetInviteTermCommand& other):CPcmCommand(other)
{
	
}
////////////////////////////////////////////////////////////////////////
CPcmSetInviteTermCommand::~CPcmSetInviteTermCommand()
{
}

void CPcmSetInviteTermCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}

void CPcmSetInviteTermCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	
	m_fromAddressBook = GetNodeValueBool(mpMsg,"INVITE_FROM_ADDRESSBOOK");
	int invitedTermLen = GetNodeValueInteger(mpMsg,"TERM_NAME_COUNT");

	std::string tmpKeyName,tmpValueName,tmpKeyType;
	int tmpValueType;
	for (int i=1;i<=invitedTermLen;i++)
	{
		tmpKeyName = "TERM_NAME_" + itoa(i);
		tmpKeyType = "CALL_TYPE_" + itoa(i);
		tmpValueName = GetNodeValueString(mpMsg,tmpKeyName);
		tmpValueType = GetNodeValueInteger(mpMsg,tmpKeyType);
		
		m_invitedTermDetails.insert(strIntMap::value_type(tmpValueName,tmpValueType));
		
	}
}

CPcmSetDropTermCommand::CPcmSetDropTermCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_drop_term";
	m_kickAll = false;
	//m_names_to_drop = new vector<string>;
	
}
////////////////////////////////////////////////////////////////////////
CPcmSetDropTermCommand::CPcmSetDropTermCommand(CPcmSetDropTermCommand& other):CPcmCommand(other)
{
//	delete(m_names_to_drop);
}
////////////////////////////////////////////////////////////////////////
CPcmSetDropTermCommand::~CPcmSetDropTermCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmSetDropTermCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmSetDropTermCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	
	m_kickAll = GetNodeValueBool(mpMsg,"KICKALL");
	int namesToKickLen = GetNodeValueInteger(mpMsg,"TERM_NAME_COUNT");

	std::string tmpKey,tmpValue;
	for (int i=1;i<=namesToKickLen;i++)
	{
		tmpKey = "TERM_NAME_"+itoa(i);
		tmpValue = GetNodeValueString(mpMsg,tmpKey);
		m_names_to_drop.push_back(tmpValue);
	}
	
	
}

CPcmStopConfCommand::CPcmStopConfCommand(int termId):CPcmCommand(termId)
{
	action_name = "stop_conf";
}
////////////////////////////////////////////////////////////////////////
CPcmStopConfCommand::CPcmStopConfCommand(CPcmStopConfCommand& other):CPcmCommand(other)
{
	
}
////////////////////////////////////////////////////////////////////////
CPcmStopConfCommand::~CPcmStopConfCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmStopConfCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmStopConfCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	
}
////////////////////////////////////////////////////////////////////////
// 						CPcmFeccControlCommand
////////////////////////////////////////////////////////////////////////
CPcmFeccControlCommand::CPcmFeccControlCommand(int termId):CPcmCommand(termId)
{
	action_name = "fecc_control";
	m_paneIndex = -1;
}
////////////////////////////////////////////////////////////////////////
CPcmFeccControlCommand::CPcmFeccControlCommand(CPcmFeccControlCommand& other):CPcmCommand(other)
{
	
}
////////////////////////////////////////////////////////////////////////
CPcmFeccControlCommand::~CPcmFeccControlCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmFeccControlCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmFeccControlCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	m_paneIndex = GetNodeValueInteger(mpMsg,"PANE_INDEX");
	
}
////////////////////////////////////////////////////////////////////////
// 						CPcmSetConfLayoutTypeCommand
////////////////////////////////////////////////////////////////////////
CPcmSetConfLayoutTypeCommand::CPcmSetConfLayoutTypeCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_conf_layout_mode";
	
	m_layoutMode  = -1;
	m_lectoreRole = -1;
	
}
////////////////////////////////////////////////////////////////////////
CPcmSetConfLayoutTypeCommand::CPcmSetConfLayoutTypeCommand(CPcmSetConfLayoutTypeCommand& other):CPcmCommand(other)
{
	m_layoutMode = other.m_layoutMode;
	m_lectoreRole = other.m_lectoreRole;
}
////////////////////////////////////////////////////////////////////////
CPcmSetConfLayoutTypeCommand::~CPcmSetConfLayoutTypeCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmSetConfLayoutTypeCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmSetConfLayoutTypeCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	
	m_layoutMode = GetNodeValueInteger(mpMsg,"LAYOUT_MODE");
	
	string lectureRoleTemp = GetNodeValueString(mpMsg,"LECTURE_ROLE");
	if (lectureRoleTemp == "000000000000000000000000000000ff")
		m_lectoreRole = -1;
	else
		m_lectoreRole = atoi(lectureRoleTemp.c_str());

}
////////////////////////////////////////////////////////////////////////
// 						CPcmRecordCommand
////////////////////////////////////////////////////////////////////////
strIntMap CPcmRecordCommand::cmdStrToMcmsOpcode;

bool CPcmRecordCommand::InitMap()
{
	cmdStrToMcmsOpcode["start"] = SET_START_RECORDING;
	cmdStrToMcmsOpcode["pause"] = SET_PAUSE_RECORDING;
	cmdStrToMcmsOpcode["stop"] = SET_STOP_RECORDING;
	return true;
}
/////////////////////////////////////////////////////
CPcmRecordCommand::CPcmRecordCommand(int termId):CPcmCommand(termId)
{
	action_name = "record";
	cmd = "unknown";
	static bool one_time_call = InitMap(); 
}
////////////////////////////////////////////////////////////////////////
CPcmRecordCommand::CPcmRecordCommand(CPcmRecordCommand& other):CPcmCommand(other)
{
	cmd = other.cmd;
}
////////////////////////////////////////////////////////////////////////
CPcmRecordCommand::~CPcmRecordCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmRecordCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmRecordCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	
	cmd = GetNodeValueString(mpMsg,"CMD");
}
////////////////////////////////////////////////////////////////////////
DWORD CPcmRecordCommand::GetCommandValue(string cmd)
{
	if (cmdStrToMcmsOpcode.find(cmd) != cmdStrToMcmsOpcode.end())
		return cmdStrToMcmsOpcode[cmd];
	else
		return (DWORD)-1;
}
////////////////////////////////////////////////////////////////////////
// 						CPcmLocalAddrBookCommand
////////////////////////////////////////////////////////////////////////
CPcmLocalAddrBookCommand::CPcmLocalAddrBookCommand(int termId):CPcmCommand(termId)
{
	action_name = "local_addr_book";

	m_matchStr  = "";
	m_matchStrEnd = "";
	m_count = -1;

}
////////////////////////////////////////////////////////////////////////
CPcmLocalAddrBookCommand::CPcmLocalAddrBookCommand(CPcmLocalAddrBookCommand& other):CPcmCommand(other)
{
	m_matchStr = other.m_matchStr;
	m_matchStrEnd = other.m_matchStrEnd;
	m_count = other.m_count;
}
////////////////////////////////////////////////////////////////////////
CPcmLocalAddrBookCommand::~CPcmLocalAddrBookCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmLocalAddrBookCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);

	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmLocalAddrBookCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);

	m_matchStr = GetNodeValueString(mpMsg,"MATCH_STR");
	m_matchStrEnd = GetNodeValueString(mpMsg,"MATCH_STR_END");
	m_count = GetNodeValueInteger(mpMsg,"COUNT");

}
////////////////////////////////////////////////////////////////////////
// 						CPcmSetDisplaySettingCommand
////////////////////////////////////////////////////////////////////////
CPcmSetDisplaySettingCommand::CPcmSetDisplaySettingCommand(int termId):CPcmCommand(termId)
{
	action_name = "set_display_setting";

	m_paneType  = "";
	m_paneIndex = -1;  //<!-- 0-Voice Active,1-Polling,2-force polling ,string-name of terminal -->

}
////////////////////////////////////////////////////////////////////////
CPcmSetDisplaySettingCommand::CPcmSetDisplaySettingCommand(CPcmSetDisplaySettingCommand& other):CPcmCommand(other)
{
	m_paneType = other.m_paneType;
	m_paneIndex = other.m_paneIndex;
}
////////////////////////////////////////////////////////////////////////
CPcmSetDisplaySettingCommand::~CPcmSetDisplaySettingCommand()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmSetDisplaySettingCommand::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);

	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmSetDisplaySettingCommand::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);

	m_paneIndex = GetNodeValueInteger(mpMsg,"PANE_INDEX");
	m_paneType = GetNodeValueString(mpMsg,"PANE_TYPE");

}
////////////////////////////////////////////////////////////////////////
// 						CPcmCommandDummy
////////////////////////////////////////////////////////////////////////
CPcmCommandDummy::CPcmCommandDummy(int termId):CPcmCommand(termId)
{
	action_name = "";
}
////////////////////////////////////////////////////////////////////////
CPcmCommandDummy::CPcmCommandDummy(CPcmCommandDummy& other):CPcmCommand(other)
{	
}
////////////////////////////////////////////////////////////////////////
CPcmCommandDummy::~CPcmCommandDummy()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmCommandDummy::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmCommandDummy::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
