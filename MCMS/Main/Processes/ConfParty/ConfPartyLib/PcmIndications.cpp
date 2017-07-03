
#include "PcmIndications.h"
#include "TaskApp.h"
#include "Party.h"
#include "Layout.h"
#include "CommConf.h"
#include "AddressBook.h"
#include "RsrvParty.h"
#include "VideoLayout.h"

LayoutType GetNewLayoutType(const BYTE oldLayoutType);
////////////////////////////////////////////////////////////////////////
//				CPcmControlKeyIndication
////////////////////////////////////////////////////////////////////////
CPcmControlKeyIndication::CPcmControlKeyIndication(int termId):CPcmIndication(termId)
{
	action_name = "control_key";
	m_key = "LEFT";
}
////////////////////////////////////////////////////////////////////////
CPcmControlKeyIndication::CPcmControlKeyIndication(CPcmControlKeyIndication& other):CPcmIndication(other)
{
	m_key = other.m_key;
}
////////////////////////////////////////////////////////////////////////
CPcmControlKeyIndication::~CPcmControlKeyIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmControlKeyIndication::NameOf() const
{
	return "CPcmControlKeyIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmControlKeyIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	
	AddChildNodeString(mpMsg,"KEY",m_key);
	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmControlKeyIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	m_key = GetNodeValueString(mpMsg,"KEY");
}
////////////////////////////////////////////////////////////////////////
//				CPcmInitialMenuBufferIndication
////////////////////////////////////////////////////////////////////////
CPcmInitialMenuBufferIndication::CPcmInitialMenuBufferIndication(int termId):CPcmIndication(termId)
{
	action_name = "initial_menu_buffer";
}
////////////////////////////////////////////////////////////////////////
CPcmInitialMenuBufferIndication::CPcmInitialMenuBufferIndication(CPcmInitialMenuBufferIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmInitialMenuBufferIndication::~CPcmInitialMenuBufferIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmInitialMenuBufferIndication::NameOf() const
{
	return "CPcmInitialMenuBufferIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmInitialMenuBufferIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmInitialMenuBufferIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmMenuStateIndication
////////////////////////////////////////////////////////////////////////
CPcmMenuStateIndication::CPcmMenuStateIndication(int termId, bool isActive, bool isDisenable, bool isCopActive):CPcmIndication(termId),m_isActive(isActive),m_isDisenable(isDisenable),m_isCopActive(isCopActive)
{
	action_name = "menu_state";
	
}
////////////////////////////////////////////////////////////////////////
CPcmMenuStateIndication::CPcmMenuStateIndication(CPcmMenuStateIndication& other):CPcmIndication(other)
{
	m_isActive = other.m_isActive;
	m_isDisenable = other.m_isDisenable;
	m_isCopActive = other.m_isCopActive;
}
////////////////////////////////////////////////////////////////////////
CPcmMenuStateIndication::~CPcmMenuStateIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmMenuStateIndication::NameOf() const
{
	return "CPcmMenuStateIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmMenuStateIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	AddChildNodeBool(mpMsg,"IS_ACTIVE",m_isActive);
	AddChildNodeBool(mpMsg,"IS_DISENABLE",m_isDisenable);
	AddChildNodeBool(mpMsg,"IS_COP_ACTIVE",m_isCopActive);
	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmMenuStateIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	m_isActive = GetNodeValueBool(mpMsg,"IS_ACTIVE");
	m_isDisenable = GetNodeValueBool(mpMsg,"IS_DISENABLE");
	m_isCopActive = GetNodeValueBool(mpMsg,"IS_COP_ACTIVE");
}
////////////////////////////////////////////////////////////////////////
//				CPcmImageSizeIndication
////////////////////////////////////////////////////////////////////////
CPcmImageSizeIndication::CPcmImageSizeIndication(int termId):CPcmIndication(termId)
{
	action_name = "image_size";
	m_type = "4CIF";
	m_ratio = "16:9";
}
////////////////////////////////////////////////////////////////////////
CPcmImageSizeIndication::CPcmImageSizeIndication(CPcmImageSizeIndication& other):CPcmIndication(other)
{
	m_type = other.m_type;
	m_ratio = other.m_ratio;
}
////////////////////////////////////////////////////////////////////////
CPcmImageSizeIndication::~CPcmImageSizeIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmImageSizeIndication::NameOf() const
{
	return "CPcmImageSizeIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmImageSizeIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	
	AddChildNodeString(mpMsg,"TYPE",m_type);
	AddChildNodeString(mpMsg,"RATIO",m_ratio);
	      
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmImageSizeIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	
	m_type =  GetNodeValueString(mpMsg,"TYPE");
	m_ratio = GetNodeValueString(mpMsg,"RATIO");
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//				CPcmConfLayoutModeInfoIndication
////////////////////////////////////////////////////////////////////////
CPcmConfLayoutModeInfoIndication::CPcmConfLayoutModeInfoIndication(int termId):CPcmIndication(termId)
{
	action_name = "conf_layout_mode_info";
	m_enableCpLayout = true;
}
////////////////////////////////////////////////////////////////////////
CPcmConfLayoutModeInfoIndication::CPcmConfLayoutModeInfoIndication(CPcmConfLayoutModeInfoIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmConfLayoutModeInfoIndication::~CPcmConfLayoutModeInfoIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmConfLayoutModeInfoIndication::NameOf() const
{
	return "CPcmConfLayoutModeInfoIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmConfLayoutModeInfoIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	AddChildNodeInteger(mpMsg,"LAYOUT_MODE",m_layoutMode);
	AddChildNodeBool(mpMsg,"ENABLE_CP_LAYOUT",m_enableCpLayout);
	AddChildNodeInteger(mpMsg,"CP_LAYOUT_TYPE",m_cpLayoutType);
	AddChildNodeInteger(mpMsg,"LECTURE_ROLE",m_lectureRole);//????<!-- AUTO -- 000000000000000000000000000000ff, guid-->
	
	int listSize = m_ongoingParties.size();
	AddChildNodeInteger(mpMsg,"TERM_COUNT",listSize);
	string guidStr = "TERM_GUID_";
	string nameStr = "TERM_NAME_";
	string tmpGuidStr , tmpNameStr;
	for (int i=1;i <= listSize;i++)
	{
		CConfParty* pCurConfParty = m_ongoingParties[i-1];
		//tmpGuidStr = guidStr;
		tmpGuidStr = "TERM_GUID_" + itoa(i);
		tmpNameStr = "TERM_NAME_" + itoa(i);
		AddChildNodeInteger(mpMsg,tmpGuidStr,pCurConfParty->GetPartyId());
		if (pCurConfParty->GetVisualPartyName()[0] != '\0')
			AddChildNodeString(mpMsg,tmpNameStr,pCurConfParty->GetVisualPartyName());
		else
			AddChildNodeString(mpMsg,tmpNameStr,pCurConfParty->GetName());
		
	}
	      	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmConfLayoutModeInfoIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	
	/*
	 * m_layoutMode = atoi(mpMsg["LAYOUT_MODE"].c_str());
	m_cpLayoutType = atoi(mpMsg["CP_LAYOUT_TYPE"].c_str());
	int listSize = atoi(mpMsg["TERM_COUNT"].c_str());
	string nameStr = "TERM_NAME_";
	string tmpNameStr;
	m_TerminalNames.clear();
	for (int i=1;i <= listSize;i++)
	{
		tmpNameStr = nameStr;
		tmpNameStr+=itoa(i);
		m_TerminalNames.push_back(mpMsg[tmpNameStr]);
		tmpNameStr.clear();
	}
	*/
	
}
////////////////////////////////////////////////////////////////////////
BYTE CPcmConfLayoutModeInfoIndication::FillOngoingPartiesVector(CCommConf* pCommConf)
{
	int oldVectorSize = m_ongoingParties.size();
	m_ongoingParties.clear();
	CConfParty* pCurConfParty = pCommConf->GetFirstParty();
	while(pCurConfParty != NULL)
	{
		DWORD partyState = pCurConfParty->GetPartyState();
		if (pCurConfParty->IsVideo_Member() && (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY || partyState ==PARTY_CONNECTED_WITH_PROBLEM))
		{
			m_ongoingParties.push_back(pCurConfParty);
		}
	   	pCurConfParty = pCommConf->GetNextParty();
	} // end while
	int newVectorSize = m_ongoingParties.size();
	
	return (oldVectorSize != newVectorSize);
}
////////////////////////////////////////////////////////////////////////
void CPcmConfLayoutModeInfoIndication::SetLayoutParams(CCommConf* pCommConf)
{
	LayoutType mcmsLayoutType = ::GetNewLayoutType(pCommConf->GetCurConfVideoLayout());
	m_cpLayoutType = TranslateMcmsLayoutTypeToPcmApiLayoutType(mcmsLayoutType);
	
	if (pCommConf->GetIsSameLayout())
		m_layoutMode = 1;
	else
	{
		CLectureModeParams* pLectureModeParms = pCommConf->GetLectureMode();
		if (CPObject::IsValidPObjectPtr(pLectureModeParms) && pLectureModeParms->GetLectureModeType())
		{
			m_layoutMode = 0;
			CConfParty* pCLecturerConfParty = pCommConf->GetCurrentParty(pLectureModeParms->GetLecturerName());
			if (CPObject::IsValidPObjectPtr(pCLecturerConfParty))
				m_lectureRole = pCLecturerConfParty->GetPartyId();
			else
				m_lectureRole = 0x000000FF;
			
		}
		else
			m_layoutMode = 1; //3?
	}
	
	FillOngoingPartiesVector(pCommConf);
}
////////////////////////////////////////////////////////////////////////
void CPcmConfLayoutModeInfoIndication::SetPrivateLayout(CVideoLayout* privateLayout)
{
	LayoutType mcmsLayoutType = ::GetNewLayoutType(privateLayout->GetScreenLayout());
	m_cpLayoutType = TranslateMcmsLayoutTypeToPcmApiLayoutType(mcmsLayoutType);

}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//				CPcmServiceSettingIndication
////////////////////////////////////////////////////////////////////////
CPcmServiceSettingIndication::CPcmServiceSettingIndication(int termId,int setting):CPcmIndication(termId),m_setting(setting)
{
	action_name = "service_setting";
	
}
////////////////////////////////////////////////////////////////////////
CPcmServiceSettingIndication::CPcmServiceSettingIndication(CPcmServiceSettingIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmServiceSettingIndication::~CPcmServiceSettingIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmServiceSettingIndication::NameOf() const
{
	return "CPcmServiceSettingIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmServiceSettingIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	
	AddChildNodeInteger(mpMsg,"SETTING",m_setting);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmServiceSettingIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	
	m_setting = GetNodeValueInteger(mpMsg,"SETTING");
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//				CPcmTermListInfoIndication
////////////////////////////////////////////////////////////////////////
CPcmTermListInfoIndication::CPcmTermListInfoIndication(int termId,CCommConf* pCommConf):CPcmIndication(termId)
{
	action_name = "term_list_info";
}
////////////////////////////////////////////////////////////////////////
CPcmTermListInfoIndication::CPcmTermListInfoIndication(CPcmTermListInfoIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmTermListInfoIndication::~CPcmTermListInfoIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmTermListInfoIndication::NameOf() const
{
	return "CPcmTermListInfoIndication";
}
////////////////////////////////////////////////////////////////////////
BYTE CPcmTermListInfoIndication::FillOngoingPartiesVector(CCommConf* pCommConf)
{
	int oldVectorSize = m_ongoingParties.size();
	m_ongoingParties.clear();
	CConfParty* pCurConfParty = pCommConf->GetFirstParty();
	while(pCurConfParty != NULL)
	{
		DWORD partyState = pCurConfParty->GetPartyState();
		if (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY || partyState ==PARTY_CONNECTED_WITH_PROBLEM)
		{
			m_ongoingParties.push_back(pCurConfParty);
		}
	   	pCurConfParty = pCommConf->GetNextParty();
	} // end while
	int newVectorSize = m_ongoingParties.size();
	
	return (oldVectorSize != newVectorSize);
}
////////////////////////////////////////////////////////////////////////
void CPcmTermListInfoIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	
	int listSize = m_ongoingParties.size();
	AddChildNodeInteger(mpMsg,"TERM_COUNT",listSize);//           <!--the number of patticipant-->
	for (int i = 1; i <= listSize;i++)
	{
		CConfParty* pCurConfParty = m_ongoingParties[i-1];
		if (pCurConfParty->GetVisualPartyName()[0] != '\0')
			AddChildNodeString(mpMsg,"TERM_NAME_"+itoa(i),pCurConfParty->GetVisualPartyName());
		else
			AddChildNodeString(mpMsg,"TERM_NAME_"+itoa(i), pCurConfParty->GetName());
		AddChildNodeByteToBoolStr(mpMsg,"VIDEO_MUTE_IN_"+itoa(i),pCurConfParty->IsVideoMutedByOperator());//? "true" : "false"; //by party? by MCU?
		AddChildNodeByteToBoolStr(mpMsg,"AUDIO_MUTE_IN_"+itoa(i),pCurConfParty->IsAudioMutedByOperator());//? "true" : "false"; //by party? by MCU?
		AddChildNodeByteToBoolStr(mpMsg,"AUDIO_MUTE_OUT_"+itoa(i),pCurConfParty->IsAudioBlocked());//? "true" : "false";
		AddChildNodeString(mpMsg,"NETWORK_STATUS_"+itoa(i),"false");//???
		if (pCurConfParty->GetVoice())
			AddChildNodeString(mpMsg,"TERM_TYPE_"+itoa(i),"audio");
		else
			AddChildNodeString(mpMsg,"TERM_TYPE_"+itoa(i),"video");
	}
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmTermListInfoIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmAllAudioMuteStateIndication 1
////////////////////////////////////////////////////////////////////////
CPcmAllAudioMuteStateIndication::CPcmAllAudioMuteStateIndication(int termId):CPcmIndication(termId)
{
	action_name = "all_audio_mute_state";
}
////////////////////////////////////////////////////////////////////////
CPcmAllAudioMuteStateIndication::CPcmAllAudioMuteStateIndication(CPcmAllAudioMuteStateIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmAllAudioMuteStateIndication::~CPcmAllAudioMuteStateIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmAllAudioMuteStateIndication::NameOf() const
{
	return "CPcmAllAudioMuteStateIndication";
}
////////////////////////////////////////////////////////////////////////
// option 1 - calculate 
void CPcmAllAudioMuteStateIndication::CalculateAllMuteFlag(CCommConf* pCommConf)
{
	bool areAllMuted = true;
	CConfParty* pCurConfParty = pCommConf->GetFirstParty();
	while(pCurConfParty != NULL && areAllMuted == true)
	{
		if (!pCurConfParty->GetIsLeader())
		{
			DWORD partyState = pCurConfParty->GetPartyState();
			if (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY || partyState ==PARTY_CONNECTED_WITH_PROBLEM)
			{
				if (!pCurConfParty->IsAudioMutedByOperator())
					areAllMuted = false;
			}
		}
		pCurConfParty = pCommConf->GetNextParty();
	} // end while
	
	m_AreAllMuted = areAllMuted;
}
////////////////////////////////////////////////////////////////////////
// option 2 - set (true if all muted except the chair)
void CPcmAllAudioMuteStateIndication::SetAllMuteButXFlag(BYTE onOff)
{
	if (onOff)
		m_AreAllMuted = true;
	else
		m_AreAllMuted = false;
}
////////////////////////////////////////////////////////////////////////
void CPcmAllAudioMuteStateIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	
	AddChildNodeBool(mpMsg,"IS_ALL_MUTE",m_AreAllMuted);
	//mpMsg["IS_ALL_MUTE"] = g_CommonFun.boolToString(m_AreAllMuted);	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmAllAudioMuteStateIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmConfInfoIndication
////////////////////////////////////////////////////////////////////////
CPcmConfInfoIndication::CPcmConfInfoIndication(int termId):CPcmIndication(termId)
{
	action_name = "conf_info";
}
////////////////////////////////////////////////////////////////////////
CPcmConfInfoIndication::CPcmConfInfoIndication(CPcmConfInfoIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmConfInfoIndication::~CPcmConfInfoIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmConfInfoIndication::NameOf() const
{
	return "CPcmConfInfoIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmConfInfoIndication::InitInternalParams(CCommConf* pCommConf)
{
	m_confName = pCommConf->GetName();
	m_confId = pCommConf->GetMonitorConfId();
	m_confPassword = pCommConf->GetEntryPassword();
	m_presiderPassword = pCommConf->GetH243Password();
	m_participantsCount = pCommConf->GetNumParties();
	m_isLocked = pCommConf->GetConfLockFlag()? true : false;
	m_isEncrypted = pCommConf->GetIsEncryption()? true : false;
	string m_textInfo = "";
	m_isSwitching = false;
	m_skinType = 0;
	m_enableRecord = pCommConf->GetEnableRecording()? true : false;
	
}
////////////////////////////////////////////////////////////////////////
void CPcmConfInfoIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	
	AddChildNodeString(mpMsg,"CONF_NAME",m_confName);
	AddChildNodeInteger(mpMsg,"CONF_ID",m_confId);       
	AddChildNodeString(mpMsg,"CONF_PASSWORD",m_confPassword);
	AddChildNodeString(mpMsg,"PRESIDER_PASSWORD",m_presiderPassword);
	AddChildNodeInteger(mpMsg,"PARTICIPANT_COUNT",m_participantsCount);
	AddChildNodeBool(mpMsg,"IS_LOCKED",m_isLocked);
	AddChildNodeBool(mpMsg,"IS_ENCRYPTED",m_isEncrypted);
	AddChildNodeString(mpMsg,"TEXT_INFO",m_textInfo);
	AddChildNodeBool(mpMsg,"IS_SWITCHING",m_isSwitching);
	AddChildNodeInteger(mpMsg,"SKIN_TYPE",m_skinType);
	AddChildNodeBool(mpMsg,"ENABLE_RECORD",m_enableRecord);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmConfInfoIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmDirectLoginConfIndication
////////////////////////////////////////////////////////////////////////
CPcmDirectLoginConfIndication::CPcmDirectLoginConfIndication(int termId):CPcmIndication(termId)
{
	action_name = "direct_login_conf";
}
////////////////////////////////////////////////////////////////////////
CPcmDirectLoginConfIndication::CPcmDirectLoginConfIndication(CPcmDirectLoginConfIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmDirectLoginConfIndication::~CPcmDirectLoginConfIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmDirectLoginConfIndication::NameOf() const
{
	return "CPcmDirectLoginConfIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmDirectLoginConfIndication::InitInternalParams(string confName,int confId, string termName,bool isPresider)
{
	m_confName = confName;
	m_confId = confId;
	m_termName = termName;
	m_isPresider = isPresider;

}
////////////////////////////////////////////////////////////////////////
void CPcmDirectLoginConfIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	
	AddChildNodeString(mpMsg,"CONF_NAME",m_confName);
	AddChildNodeInteger(mpMsg,"CONF_ID",m_confId);                       // <!--conference ID-->
	AddChildNodeString(mpMsg,"TERM_NAME",m_termName);       //<!--the current participant's name-->
	AddChildNodeBool(mpMsg,"IS_PRESIDER",m_isPresider);
	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmDirectLoginConfIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmPresiderToParticipantIndication
////////////////////////////////////////////////////////////////////////
CPcmPresiderToParticipantIndication::CPcmPresiderToParticipantIndication(int termId):CPcmIndication(termId)
{
	action_name = "initial_menu_buffer";
}
////////////////////////////////////////////////////////////////////////
CPcmPresiderToParticipantIndication::CPcmPresiderToParticipantIndication(CPcmPresiderToParticipantIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmPresiderToParticipantIndication::~CPcmPresiderToParticipantIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmPresiderToParticipantIndication::NameOf() const
{
	return "CPcmPresiderToParticipantIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmPresiderToParticipantIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmPresiderToParticipantIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmParticipantToPresiderIndication
////////////////////////////////////////////////////////////////////////
CPcmParticipantToPresiderIndication::CPcmParticipantToPresiderIndication(int termId):CPcmIndication(termId)
{
	action_name = "initial_menu_buffer";
}
////////////////////////////////////////////////////////////////////////
CPcmParticipantToPresiderIndication::CPcmParticipantToPresiderIndication(CPcmParticipantToPresiderIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmParticipantToPresiderIndication::~CPcmParticipantToPresiderIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmParticipantToPresiderIndication::NameOf() const
{
	return "CPcmParticipantToPresiderIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmParticipantToPresiderIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmParticipantToPresiderIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmInviteResultIndication
////////////////////////////////////////////////////////////////////////
CPcmInviteResultIndication::CPcmInviteResultIndication(int termId):CPcmIndication(termId)
{
	action_name = "invite_result";
	m_res = false;
}
////////////////////////////////////////////////////////////////////////
CPcmInviteResultIndication::CPcmInviteResultIndication(CPcmInviteResultIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmInviteResultIndication::~CPcmInviteResultIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmInviteResultIndication::NameOf() const
{
	return "CPcmInviteResultIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmInviteResultIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	
	AddChildNodeBool(mpMsg,"RESULT",m_res);	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmInviteResultIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmFeccEndIndication
////////////////////////////////////////////////////////////////////////
CPcmFeccEndIndication::CPcmFeccEndIndication(int termId):CPcmIndication(termId)
{
	action_name = "fecc_end";
}
////////////////////////////////////////////////////////////////////////
CPcmFeccEndIndication::CPcmFeccEndIndication(CPcmFeccEndIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmFeccEndIndication::~CPcmFeccEndIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmFeccEndIndication::NameOf() const
{
	return "CPcmFeccEndIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmFeccEndIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmFeccEndIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmRecordStateIndication
////////////////////////////////////////////////////////////////////////
CPcmRecordStateIndication::CPcmRecordStateIndication(int termId):CPcmIndication(termId)
{
	action_name = "record_state";
}
////////////////////////////////////////////////////////////////////////
CPcmRecordStateIndication::CPcmRecordStateIndication(CPcmRecordStateIndication& other):CPcmIndication(other)
{
	m_recordingState = other.m_recordingState;
}
////////////////////////////////////////////////////////////////////////
CPcmRecordStateIndication::~CPcmRecordStateIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmRecordStateIndication::NameOf() const
{
	return "CPcmRecordStateIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmRecordStateIndication::SetRecordingStateStrFromWord(WORD recordingState)
{
	switch (recordingState)
	{
		case(eStopRecording):
		{
			m_recordingState = "stop";
			break;
		}
		case(eStartRecording):
		case(eResumeRecording):
		{
			m_recordingState = "start";
			break;
		}
		case(ePauseRecording):
		{
			m_recordingState = "pause";
			break;
		}
		default:
		{
			m_recordingState = "unknown";
			break;	
		}
	
	}
		
}
////////////////////////////////////////////////////////////////////////
void CPcmRecordStateIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	AddChildNodeString(mpMsg,"RECORDING_STATE",m_recordingState);
	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmRecordStateIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmTerminalEndCallIndication
////////////////////////////////////////////////////////////////////////
CPcmTerminalEndCallIndication::CPcmTerminalEndCallIndication(int termId):CPcmIndication(termId)
{
	action_name = "term_end_call";
}
////////////////////////////////////////////////////////////////////////
CPcmTerminalEndCallIndication::CPcmTerminalEndCallIndication(CPcmTerminalEndCallIndication& other):CPcmIndication(other)
{
}
////////////////////////////////////////////////////////////////////////
CPcmTerminalEndCallIndication::~CPcmTerminalEndCallIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmTerminalEndCallIndication::NameOf() const
{
	return "CPcmTerminalEndCallIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmTerminalEndCallIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
		
	//str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmTerminalEndCallIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
////////////////////////////////////////////////////////////////////////
//				CPcmLanguageSettingIndication
////////////////////////////////////////////////////////////////////////
CPcmLanguageSettingIndication::CPcmLanguageSettingIndication(int termId):CPcmIndication(termId)
{
	action_name = "language_setting";
	m_lang = "ChineseSimplified";
}
////////////////////////////////////////////////////////////////////////
CPcmLanguageSettingIndication::CPcmLanguageSettingIndication(CPcmLanguageSettingIndication& other):CPcmIndication(other)
{
	m_lang = other.m_lang;
}
////////////////////////////////////////////////////////////////////////
CPcmLanguageSettingIndication::~CPcmLanguageSettingIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmLanguageSettingIndication::NameOf() const
{
	return "CPcmLanguageSettingIndication";
}
////////////////////////////////////////////////////////////////////////
void CPcmLanguageSettingIndication::SetLanguageFromSysConfig()
{
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	string langFromSysConfig;
	sysConfig->GetDataByKey("PCM_LANGUAGE", langFromSysConfig);

	if (langFromSysConfig == "ENGLISH")
	{
		m_lang = "English";
	}
	else if (langFromSysConfig == "CHINESE_SIMPLIFIED")
	{
		m_lang = "ChineseSimplified";
	}
	else if (langFromSysConfig == "CHINESE_TRADITIONAL")
	{
		m_lang = "ChineseTraditional";
	}

}
////////////////////////////////////////////////////////////////////////
void CPcmLanguageSettingIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	AddChildNodeString(mpMsg,"LANGUAGE ",m_lang);

	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmLanguageSettingIndication::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
}

/////////////*** CPcmLocalAddressBookIndication ***/////////////////////////////////////////////////////////////
CPcmLocalAddressBookIndication::CPcmLocalAddressBookIndication(int termId):CPcmIndication(termId)
{
	action_name = "local_addr_book_result";
	m_isFinished = true;
	m_sequence = 0;
	m_count = 0;//-1??
}
CPcmLocalAddressBookIndication::~CPcmLocalAddressBookIndication()
{

}
CPcmLocalAddressBookIndication::CPcmLocalAddressBookIndication(CPcmLocalAddressBookIndication& other):CPcmIndication(other)
{

}
const char*  CPcmLocalAddressBookIndication::NameOf() const
{
	return "CPcmLocalAddressBookIndication";
}

void CPcmLocalAddressBookIndication::SerializeXmlToStr(strmap& mpMsg, string& str)
{
  CPcmMessage::SerializeXmlToStr(mpMsg, str);

  int listSize = m_parties.size();
  char psIpAdr[IPV6_ADDRESS_LEN+8];// IPV6 max path plus brackets
  if (m_count > listSize)
  {
    DBGPASSERT(1);
    m_count = listSize;
  }

  AddChildNodeInteger(mpMsg, "TERM_COUNT", m_count);//   <!--the number of patticipant-->
  for (int i = 1; i <= m_count; ++i)
  {
    CRsrvParty* pParty = m_parties.at(i - 1);
    AddChildNodeString(mpMsg, "TERM_NAME_" + itoa(i), pParty->GetName());
    memset(psIpAdr, 0, sizeof(psIpAdr));
    ipToString(pParty->GetIpAddress(), psIpAdr, 0);
    AddChildNodeString(mpMsg, "TERM_IP_" + itoa(i), psIpAdr);
    AddChildNodeString(mpMsg, "TERM_E164_" + itoa(i), pParty->GetH323PartyAlias());
    AddChildNodeString(mpMsg, "TERM_STATUS_" + itoa(i), "offline"); //to do later? search the party in conference and update its status
  }
  AddChildNodeInteger(mpMsg, "RESULT", 0);
  AddChildNodeBool(mpMsg, "IS_FINISHED", m_isFinished);
  AddChildNodeInteger(mpMsg, "SEQUENCE", m_sequence);

  str = g_CommonFun.xml_MapToXmlStr(mpMsg, rootName, rootVer);
}
void CPcmLocalAddressBookIndication::DeSerializeXml(strmap& mpMsg)
{
	CPcmMessage::DeSerializeXml(mpMsg);
}
