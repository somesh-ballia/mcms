#ifndef _IVRSERVICE_H__
#define _IVRSERVICE_H__

#include "ConfPartyDefines.h"
#include "PObject.h"

#define MAX_IVR_LANG_NAME_SIZE     16
#define MAX_IVR_DTMF_CODE_LEN      4
#define MAX_IVR_AUDIO_MSG_DURATION 180
#define MAX_IVR_MUSIC_MSG_DURATION 3600


class CXMLDOMElement;
class CIVRService;

////////////////////////////////////////////////////////////////////////////
//                        CAVmsgService
////////////////////////////////////////////////////////////////////////////
class CAVmsgService : public CPObject
{
	CLASS_TYPE_1(CAVmsgService, CPObject)

public:
	                   CAVmsgService(WORD isRecordingSystem = 0);
	                   CAVmsgService(const CAVmsgService& other);
	virtual           ~CAVmsgService();
	const char*        NameOf() const { return "CAVmsgService";}
	CAVmsgService&     operator=(const CAVmsgService& other);

	// Implementation
	char*              Serialize(WORD format, DWORD apiNum);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	const char*        GetName() const;
	void               SetName(const char* name);
	WORD               IsMusic() const;
	void               SetVideoFileName(const char* name);
	const char*        GetVideoFileName() const;
	WORD               GetIVRServiceFlag() const;

	const CIVRService* GetIVRService() const;
	void               SetIVRService(const CIVRService& other);
	void               SetDtmfCodesLen();
	WORD               CheckLegalDtmfTbl();
	WORD               CheckLegalOperatorAssistance();
	const char*        GetSlideName();
	int                IsLegalServiceName(std::string& err);
	int                IsLegalService(WORD chkLevel, std::string& err);
	int                SelfCorrections();
	BOOL               IsIdenticalToCurrent(CAVmsgService* other);

public:
	char               m_serviceName[AV_MSG_SERVICE_NAME_LEN];
	char               m_video_file_name[NEW_FILE_NAME_LEN];
	CIVRService*       m_pIVRService;
};


////////////////////////////////////////////////////////////////////////////
//                        CDTMFCode
////////////////////////////////////////////////////////////////////////////
class CDTMFCode : public CPObject
{
	CLASS_TYPE_1(CDTMFCode, CPObject)

public:
	                   CDTMFCode();
	                   CDTMFCode(const CDTMFCode& other);
	virtual           ~CDTMFCode();

	const char*        NameOf() const { return "CDTMFCode";}
	CDTMFCode&         operator =(const CDTMFCode& other);

	// Implementation
	void               Serialize(WORD format, std::ostream& m_ostr);
	void               DeSerialize(WORD format, std::istream& m_istr);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	void               SetDTMFOpcode(WORD opcode);
	WORD               GetDTMFOpcode() const;

	void               SetDTMFPermission(const BYTE permission);
	WORD               GetDTMFPermission() const;

	void               SetDTMFStr(const char* DTMF_str);
	const char*        GetDTMFStr() const;

	WORD               GetDTMFStrLen() const;
	void               SetDTMFStrLen();
	BOOL               IsIdenticalToCurrent(CDTMFCode* other);

public:
	WORD               m_DTMF_len;
	WORD               m_DTMF_opcode;
	BYTE               m_DTMF_permission;
	char               m_DTMF_str[DTMF_STRING_LEN];  // key field
};


////////////////////////////////////////////////////////////////////////////
//                        CDTMFCodeList
////////////////////////////////////////////////////////////////////////////
class CDTMFCodeList : public CPObject
{
	CLASS_TYPE_1(CDTMFCodeList, CPObject)

public:
	                   CDTMFCodeList();
	                   CDTMFCodeList(const CDTMFCodeList& other);
	virtual           ~CDTMFCodeList();

	const char*        NameOf() const { return "CDTMFCodeList";}
	CDTMFCodeList&     operator=(const CDTMFCodeList& other);

	// Implementation
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	int                IsLegalService(WORD chkLevel, std::string& err);

	WORD               GetDTMFCodeNumber() const;
	WORD               CheckLegalDtmfTbl();
	void               SetDtmfCodesLen();

	int                AddDTMFCode(CDTMFCode& other);
	int                UpdateDTMFCode(const CDTMFCode& other);
	int                CancelDTMFCode(const char* DTMF_str);
	int                FindDTMFCode(const CDTMFCode& other);
	int                FindDTMFCode(const char* DTMF_str);
	CDTMFCode*         GetCurrentDTMFCode(const char* DTMF_str);
	CDTMFCode*         GetCurrentDTMFCodeByOpcode(WORD DTMF_opcode_str);

	CDTMFCode*         GetFirstDTMFCode();
	CDTMFCode*         GetNextDTMFCode();

	WORD               GetNumberOfDtmfCodes() const;
	void               DeleteList();
	BOOL               IsIdenticalToCurrent(CDTMFCodeList* other);

public:
	WORD               m_numb_of_DTMF_code;
	CDTMFCode*         m_DTMF_code[MAX_DTMF_CODE_NUM];

private:
	WORD               m_ind_DTMF;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRMessage
////////////////////////////////////////////////////////////////////////////
class CIVRMessage : public CPObject
{
	CLASS_TYPE_1(CIVRMessage, CPObject)

public:
	                   CIVRMessage();
	                   CIVRMessage(const CIVRMessage& other);
	virtual           ~CIVRMessage();

	const char*        NameOf() const { return "CIVRMessage";}
	CIVRMessage&       operator=(const CIVRMessage& other);

	// Implementation
	void               Serialize(WORD format, std::ostream& m_ostr);
	void               DeSerialize(WORD format, std::istream& m_istr);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	void               SetLanguagePos(WORD language_pos);
	WORD               GetLanguagePos() const;

	void               SetMsgFileName(const char* name);
	const char*        GetMsgFileName() const;

	void               SetMsgDuration(const WORD msgDuration);
	WORD               GetMsgDuration() const;

	void               SetMsgCheckSum(const WORD msgChecksum);
	WORD               GetMsgCheckSum() const;

	void               SetMsgLastModified(const time_t msgLastModified);
	time_t             GetMsgLastModified() const;
	BOOL               IsIdenticalToCurrent(CIVRMessage* other);

public:
	WORD               m_msg_duration;                // filled in startup, is not transfered to Operator
	char               m_msg_file_name[IVR_MSG_NAME_LEN];

protected:
	WORD               m_msgCheckSum;
	time_t             m_msgLastModified;
	WORD               m_language_pos;         // key field
};


//
//////////////////////////////////////////////////////////////////////////////
////                        CIVRMessageExternal
//////////////////////////////////////////////////////////////////////////////
//class CIVRMessageExternal : public CIVRMessage
//{
//	CLASS_TYPE_1(CIVRMessageExternal, CIVRMessage)
//
//public:
//	CIVRMessageExternal();
//	CIVRMessageExternal(const CIVRMessage& other);
//	virtual  ~CIVRMessageExternal();
//
//	const char*        NameOf() const { return "CIVRMessageExternal";}
//	CIVRMessage&       operator=(const CIVRMessageExternal& other);
//	void  SetURLForMessage(const string urlString);
//	char* GetURLForMessage ();
//
//
//protected:
//	string m_URLForMessage;
//
//};
////////////////////////////////////////////////////////////////////////////
//                        CIVREvent
////////////////////////////////////////////////////////////////////////////
class CIVREvent : public CPObject
{
	CLASS_TYPE_1(CIVREvent, CPObject)

public:
	                   CIVREvent();
	                   CIVREvent(const CIVREvent& other);
	virtual           ~CIVREvent();

	const char*        NameOf() const { return "CIVREvent";}
	CIVREvent&         operator=(const CIVREvent& other);

	// Implementation
	void               Serialize(WORD format, std::ostream& m_ostr);
	void               DeSerialize(WORD format, std::istream& m_istr);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	WORD               GetEventOpcode() const;
	void               SetEventOpcode(const WORD event_opcode);

	WORD               GetIVRMessageNumber() const;

	int                AddMessage(CIVRMessage& other);
	int                CancelMessage(const WORD language_pos);
	int                FindMessage(const CIVRMessage& other);
	int                FindMessage(const WORD language_pos);
	CIVRMessage*       GetCurrentIVRMessage(const WORD language_pos) const;
	CIVRMessage*       GetFirstMessage();
	CIVRMessage*       GetIVRMessageInPos(const WORD pos) const;

	int                AddLanguage(const WORD language_pos);
	int                CancelLanguage(const WORD language_pos);

	int                NewDirectory(const char* under_dirName);
	BOOL               IsIdenticalToCurrent(CIVREvent* other);

public:
	WORD               m_event_opcode;           // key field
	WORD               m_numb_of_messages;
	CIVRMessage*       m_pIVRMessage[MAX_LANG_IN_IVR_SERVICE];

private:
	WORD               m_ind_message;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRFeature
////////////////////////////////////////////////////////////////////////////
class CIVRFeature : public CPObject
{
	CLASS_TYPE_1(CIVRFeature, CPObject)

public:
	                   CIVRFeature();
	                   CIVRFeature(const CIVRFeature& other);
	virtual           ~CIVRFeature();

	const char*        NameOf() const { return "CIVRFeature";}
	CIVRFeature&       operator=(const CIVRFeature& other);

	// Implementation
	virtual void       Serialize(WORD format, std::ostream& m_ostr);
	virtual void       DeSerialize(WORD format, std::istream& m_istr);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	virtual int        IsLegalService(WORD chkLevel, std::string& err, const char* language);

	WORD               GetFeatureOpcode() const;
	void               SetFeatureOpcode(const WORD opcode);

	BYTE               GetEnableDisable() const;
	void               SetEnableDisable(const BYTE yes_no);

	WORD               GetIVREventNumber() const;

	int                AddEvent(CIVREvent& other);
	int                CancelEvent(const WORD event_opcode);
	int                FindEvent(const CIVREvent& other);
	int                FindEvent(const WORD event_opcode);
	CIVREvent*         GetCurrentIVREvent(const WORD event_opcode) const;
	CIVREvent*         GetFirstEvent();
	CIVREvent*         GetNextEvent();
	CIVREvent*         GetIVREventInPos(const WORD pos) const;

	const char*        GetMsgFileName(const WORD event_opcode, const WORD language_pos, int& status) const;
	void               SetMsgFileName(const WORD event_opcode, const WORD language_pos, const char* file_name, int& status);

	int                AddLanguage(const WORD language_pos);
	int                CancelLanguage(const WORD language_pos);

	int                NewDirectory(const char* under_dirName);
	int                CheckAcaIvrFile(const char* fullPathMsgName);
	BOOL               IsIdenticalToCurrent(const CIVRFeature* other) const;
	int                CheckLegalFileAndGetParams(const char* fullPathMsgName, WORD* ivrMsgDuration, WORD* ivrMsgCheckSum) const;

protected:
	int                CheckLeagalAcaFile(const char* language, WORD feature_opcode, WORD event_ind, WORD chkLevel);
	int                CheckLegalIvrFileName(const char* msgName, const char* ext = NULL);

public:
	WORD               m_feature_opcode;        // key field
	BYTE               m_enable_disable;
	WORD               m_numb_of_IVR_Event;
	CIVREvent*         m_pIVREvent[MAX_IVR_EVENT_IN_FEATURE];
	WORD               m_Save_numb_of_IVR_Event;

private:
	WORD               m_ind_event;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRLangMenuFeature
////////////////////////////////////////////////////////////////////////////
class CIVRLangMenuFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRLangMenuFeature, CIVRFeature)

public:
	                   CIVRLangMenuFeature();
	                   CIVRLangMenuFeature(const CIVRLangMenuFeature& other);
	virtual           ~CIVRLangMenuFeature();

	const char*        NameOf() const { return "CIVRLangMenuFeature";}
	CIVRLangMenuFeature& operator=(const CIVRLangMenuFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	WORD               GetCurrentLang() const;
	void               SetCurrentLang(const WORD lang);
	BOOL               IsIdenticalToCurrent(CIVRLangMenuFeature* other);

public:
	WORD               m_current_language;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRWelcomeFeature
////////////////////////////////////////////////////////////////////////////
class CIVRWelcomeFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRWelcomeFeature, CIVRFeature)

public:
	                   CIVRWelcomeFeature();
	                   CIVRWelcomeFeature(const CIVRWelcomeFeature& other);
	virtual           ~CIVRWelcomeFeature();

	const char*        NameOf() const { return "CIVRWelcomeFeature";}
	CIVRWelcomeFeature& operator=(const CIVRWelcomeFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	BYTE               GetWaitForOperatorAfterMsgOnOff() const;
	void               SetWaitForOperatorAfterMsgOnOff(const BYTE wait_for_operator_after_msgOnOff);

	BYTE               GetbEntranceMsg() const;
	void               SetbEntranceMsg(const BYTE bEntranceMsg);
	virtual int        IsLegalService(WORD chkLevel, std::string& err, const char* language);
	BOOL               IsIdenticalToCurrent(CIVRWelcomeFeature* other);

public:
	BYTE               m_wait_for_operator_after_msgOnOff;
	BYTE               m_bEntranceMsg;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRConfPasswordFeature
////////////////////////////////////////////////////////////////////////////
class CIVRConfPasswordFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRConfPasswordFeature, CIVRFeature)

public:
	                   CIVRConfPasswordFeature();
	                   CIVRConfPasswordFeature(const CIVRConfPasswordFeature& other);
	virtual           ~CIVRConfPasswordFeature();

	const char*        NameOf() const { return "CIVRConfPasswordFeature";}
	CIVRConfPasswordFeature& operator=(const CIVRConfPasswordFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	BYTE               GetDialOutEntryPassword() const;
	void               SetDialOutEntryPassword(const BYTE DialOutEntryPassword);

	BYTE               GetDialInEntryPassword() const;
	void               SetDialInEntryPassword(const BYTE DialInEntryPassword);
	virtual int        IsLegalService(WORD chkLevel, std::string& err, const char* language);
	BOOL               IsIdenticalToCurrent(const CIVRConfPasswordFeature* other) const;

public:
	BYTE               m_DialOutEntryPassword;
	BYTE               m_DialInEntryPassword;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRConfLeaderFeature
////////////////////////////////////////////////////////////////////////////
class CIVRConfLeaderFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRConfLeaderFeature, CIVRFeature)

public:
	                   CIVRConfLeaderFeature();
	                   CIVRConfLeaderFeature(const CIVRConfLeaderFeature& other);
	virtual           ~CIVRConfLeaderFeature();

	const char*        NameOf() const { return "CIVRConfLeaderFeature";}
	CIVRConfLeaderFeature& operator=(const CIVRConfLeaderFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	const char*        GetLeaderIdentifier() const;
	void               SetLeaderIdentifier(const char* szLeader_identifier);

	BYTE               GetConfPwAsLeaderPw() const;
	void               SetConfPwAsLeaderPw(const BYTE bConfPwAsLeaderPw);

	BYTE               GetIsBillingCode() const;
	void               SetIsBillingCode(const BYTE bIsBillingCode);
	virtual int        IsLegalService(WORD chkLevel, std::string& err, const char* language);
	BOOL               IsIdenticalToCurrent(CIVRConfLeaderFeature* other);

public:
	char               m_szLeader_identifier[MAX_DELIMETER_LEN];

protected:
	BYTE               m_bConfPwAsLeaderPw;
	BYTE               m_bIsBillingCode;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRPersonalPINCodeFeature
////////////////////////////////////////////////////////////////////////////
class CIVRPersonalPINCodeFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRPersonalPINCodeFeature, CIVRFeature)

public:
	                   CIVRPersonalPINCodeFeature();
	                   CIVRPersonalPINCodeFeature(const CIVRPersonalPINCodeFeature& other);
	virtual           ~CIVRPersonalPINCodeFeature();

	const char*        NameOf() const { return "CIVRPersonalPINCodeFeature";}
	CIVRPersonalPINCodeFeature& operator=(const CIVRPersonalPINCodeFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	BYTE               GetQuickLoginForLeaderOnOff() const;
	void               SetQuickLoginForLeaderOnOff(const BYTE quick_login_for_leaderOnOff);

public:
	BYTE               m_quick_login_for_leaderOnOff;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVROperAssistanceFeature
////////////////////////////////////////////////////////////////////////////
class CIVROperAssistanceFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVROperAssistanceFeature, CIVRFeature)

public:
	                   CIVROperAssistanceFeature();
	                   CIVROperAssistanceFeature(const CIVROperAssistanceFeature& other);
	virtual           ~CIVROperAssistanceFeature();

	const char*        NameOf() const { return "CIVROperAssistanceFeature";}
	CIVROperAssistanceFeature& operator=(const CIVROperAssistanceFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	BYTE               GetWaitForOperatorOnErrorOnOff() const;
	void               SetWaitForOperatorOnErrorOnOff(const BYTE wait_for_operator_on_errorOnOff);

public:
	BYTE               m_wait_for_operator_on_errorOnOff;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRGeneralMsgsFeature
////////////////////////////////////////////////////////////////////////////
class CIVRGeneralMsgsFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRGeneralMsgsFeature, CIVRFeature)

public:
	                   CIVRGeneralMsgsFeature();
	                   CIVRGeneralMsgsFeature(const CIVRGeneralMsgsFeature& other);
	virtual           ~CIVRGeneralMsgsFeature();

	const char*        NameOf() const { return "CIVRGeneralMsgsFeature";}
	CIVRGeneralMsgsFeature& operator=(const CIVRGeneralMsgsFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
	void               ChangeGeneralMsgTable(CIVRGeneralMsgsFeature* pGeneralMsg);
	int                GetBillingMsgIndex();
	BOOL               IsIdenticalToCurrent(CIVRGeneralMsgsFeature* other);
	virtual int        IsLegalService(WORD chkLevel, std::string& err, const char* language);
	int                IsLegalBillingCodeMsg(WORD chkLevel, std::string& err, const char* language);
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRInvitePartyFeature
////////////////////////////////////////////////////////////////////////////
class CIVRInvitePartyFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRInvitePartyFeature, CIVRFeature)

public:
	                   CIVRInvitePartyFeature();
	                   CIVRInvitePartyFeature(const CIVRInvitePartyFeature& other);
	virtual           ~CIVRInvitePartyFeature();

	const char*        NameOf() const { return "CIVRInvitePartyFeature";}
	CIVRInvitePartyFeature& operator=(const CIVRInvitePartyFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);

	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
	BOOL               IsIdenticalToCurrent(CIVRInvitePartyFeature* other);
	int                IsLegalService(WORD chkLevel, std::string& err, const char* language);
	map<WORD, WORD>&   getInterfaceOrderMap();
	WORD               getDtmfForwardDuration();

private:
	map<WORD, WORD>    m_interfaceCallOrder;  // key - interface, value - order
	map<WORD, WORD>    m_callOrderInterface;  // key - order, value - interface - used for Serialize/DeSerialize
	WORD               m_dtmfForwardDurationSec;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRLanguage
////////////////////////////////////////////////////////////////////////////
class CIVRLanguage : public CPObject
{
	CLASS_TYPE_1(CIVRLanguage, CPObject)

public:
	                   CIVRLanguage();
	                   CIVRLanguage(const CIVRLanguage& other);
	virtual           ~CIVRLanguage();

	const char*        NameOf() const { return "CIVRLanguage";}
	CIVRLanguage&      operator=(const CIVRLanguage& other);

	// Implementation
	void               Serialize(WORD format, std::ostream& m_ostr);
	void               DeSerialize(WORD format, std::istream& m_istr);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	void               SetLanguagePos(WORD language_pos);
	WORD               GetLanguagePos() const;

	void               SetLanguageDTMFCode(WORD language_DTMF_code);
	WORD               GetLanguageDTMFCode() const;

	void               SetLanguageName(const char* name);
	const char*        GetLanguageName() const;

public:
	WORD               m_language_pos;                    // position in list - unique
	WORD               m_language_DTMF_code;              // mast be unique, DTMF code configured for the language 1-9
	char               m_language_name[LANGUAGE_NAME_LEN];// user defined name
};


////////////////////////////////////////////////////////////////////////////
//                        CIVR_BillingCodeFeature
////////////////////////////////////////////////////////////////////////////
class CIVR_BillingCodeFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVR_BillingCodeFeature, CIVRFeature)

public:
	                   CIVR_BillingCodeFeature();
	                   CIVR_BillingCodeFeature(const CIVR_BillingCodeFeature& other);
	virtual           ~CIVR_BillingCodeFeature();

	const char*        NameOf() const { return "CIVR_BillingCodeFeature";}
	CIVR_BillingCodeFeature& operator=(const CIVR_BillingCodeFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	virtual int        IsLegalService(WORD chkLevel, std::string& err, const char* language, int indBillingInGeneral);
	BOOL               IsIdenticalToCurrent(CIVR_BillingCodeFeature* other);
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRRollCallFeature
////////////////////////////////////////////////////////////////////////////
class CIVRRollCallFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRRollCallFeature, CIVRFeature)

public:
	                   CIVRRollCallFeature();
	                   CIVRRollCallFeature(const CIVRRollCallFeature& other);
	virtual           ~CIVRRollCallFeature();

	const char*        NameOf() const { return "CIVRRollCallFeature";}
	CIVRRollCallFeature& operator=(const CIVRRollCallFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
	virtual int        IsLegalService(WORD chkLevel, std::string& err, const char* language);
	BOOL               IsIdenticalToCurrent(const CIVRRollCallFeature* other) const;
	BOOL               SetMessagesFromTonesOnlyToRollCall();
	void               Dump() const;

	void               SetUseTones(BYTE use_tones){m_use_tones = use_tones;}
	BYTE               GetUseTones() const        {return m_use_tones;}

public:
	BYTE               m_use_tones;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRVideoFeature
////////////////////////////////////////////////////////////////////////////
class CIVRVideoFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRVideoFeature, CIVRFeature)

public:
	                   CIVRVideoFeature();
	                   CIVRVideoFeature(const CIVRVideoFeature& other);
	virtual           ~CIVRVideoFeature();

	const char*        NameOf() const { return "CIVRVideoFeature";}
	CIVRVideoFeature&  operator=(const CIVRVideoFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	int                IsLegalService(WORD chkLevel, std::string& err, const char* language, const char* videoName);
	BOOL               IsIdenticalToCurrent(CIVRVideoFeature* other);

	BYTE               GetEnableVC() const;

public:
	BYTE               m_enable_VC;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRNumericConferenceIdFeature
////////////////////////////////////////////////////////////////////////////
class CIVRNumericConferenceIdFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRNumericConferenceIdFeature, CIVRFeature)

public:
	                   CIVRNumericConferenceIdFeature();
	                   CIVRNumericConferenceIdFeature(const CIVRNumericConferenceIdFeature& other);
	virtual           ~CIVRNumericConferenceIdFeature();

	const char*        NameOf() const { return "CIVRNumericConferenceIdFeature";}
	CIVRNumericConferenceIdFeature& operator=(const CIVRNumericConferenceIdFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
	virtual int        IsLegalService(WORD chkLevel, std::string& err, const char* language);
	BOOL               IsIdenticalToCurrent(CIVRNumericConferenceIdFeature* other);
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRMuteNoisyLineFeature
////////////////////////////////////////////////////////////////////////////
class CIVRMuteNoisyLineFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRMuteNoisyLineFeature, CIVRFeature)

public:
	                   CIVRMuteNoisyLineFeature();
	                   CIVRMuteNoisyLineFeature(const CIVRMuteNoisyLineFeature& other);
	virtual           ~CIVRMuteNoisyLineFeature();

	const char*        NameOf() const { return "CIVRMuteNoisyLineFeature";}
	CIVRMuteNoisyLineFeature& operator=(const CIVRMuteNoisyLineFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	void               SetReturnAndUnmute(const BYTE IsReturnAndUnmute);
	BYTE               GetReturnAndUnmute() const;

	void               SetReturnAndMute(const BYTE IsReturnAndMute);
	BYTE               GetReturnAndMute() const;

	void               SetAdjustNoiseDetection(const BYTE IsAdjustNoiseDetection);
	BYTE               GetAdjustNoiseDetection() const;

	void               SetDisableNoiseDetection(const BYTE IsDisableNoiseDetection);
	BYTE               GetDisableNoiseDetection() const;

	void               SetPlayNoisyDetectionMessage(const BYTE isPlayNoisyDetectionMessage);
	BYTE               GetPlayNoisyDetectionMessage() const;

	void               SetMuteNoisyLineMenu(const BYTE isMuteNoisyLineMenu);
	BYTE               GetMuteNoisyLineMenu() const;

private:
	BYTE               m_play_noisy_line_menu;
	BYTE               m_return_and_unmute;
	BYTE               m_return_and_mute;
	BYTE               m_adjust_noise_detection;
	BYTE               m_disable_noise_detection;
	BYTE               m_play_noisy_detection_message;
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRRecordingFeature
////////////////////////////////////////////////////////////////////////////
class CIVRRecordingFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRRecordingFeature, CIVRFeature)

public:
	                   CIVRRecordingFeature();
	                   CIVRRecordingFeature(const CIVRRecordingFeature& other);
	virtual           ~CIVRRecordingFeature();

	const char*        NameOf() const { return "CIVRRecordingFeature";}
	CIVRRecordingFeature& operator=(const CIVRRecordingFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRPlaybackFeature
////////////////////////////////////////////////////////////////////////////
class CIVRPlaybackFeature : public CIVRFeature
{
	CLASS_TYPE_1(CIVRPlaybackFeature, CIVRFeature)

public:
	                   CIVRPlaybackFeature();
	                   CIVRPlaybackFeature(const CIVRPlaybackFeature& other);
	virtual           ~CIVRPlaybackFeature();

	const char*        NameOf() const { return "CIVRPlaybackFeature";}
	CIVRPlaybackFeature& operator=(const CIVRPlaybackFeature& other);

	// Implementation
	char*              Serialize(WORD format);
	void               Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void               SerializeXml(CXMLDOMElement* pFatherNode);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        CIVRService
////////////////////////////////////////////////////////////////////////////
class CIVRService : public CPObject
{
	CLASS_TYPE_1(CIVRService, CPObject)

public:
	                                      CIVRService(WORD isRecordingSystem = 0);
	                                      CIVRService(const CIVRService& other);
	virtual                              ~CIVRService();

	virtual const char*                   NameOf() const { return "CIVRService";}
	CIVRService&                          operator=(const CIVRService& other);

	// Implementation
	char*                                 Serialize(WORD format, DWORD apiNum);
	void                                  Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum);
	void                                  DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);
	void                                  SerializeXml(CXMLDOMElement* pFatherNode);
	int                                   DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	int                                   IsLegalService(WORD chkLevel, std::string& err, char* videoName);
	int                                   SelfCorrections();
	int                                   IsLegalServiceIVR(std::string& err);
	int 								  IsLegalServiceForSoftMcu(std::string& err);
	int                                   IsNeedBilling();
	int                                   GetBillingMsgIndex();
	const char*                           GetName() const;
	void                                  SetName(const char* name);
	virtual void                          Dump(WORD i = 0);
	const CDTMFCodeList*                  GetDTMFCodeList() const;
	void                                  SetDTMFCodeList(const CDTMFCodeList& other);

	const CIVRLangMenuFeature*            GetLangMenuFeature() const;
	void                                  SetLangMenuFeature(const CIVRLangMenuFeature& other);
	const CIVRWelcomeFeature*             GetWelcomeFeature() const;
	void                                  SetWelcomeFeature(const CIVRWelcomeFeature& other);
	const CIVRConfPasswordFeature*        GetConfPasswordFeature() const;
	void                                  SetConfPasswordFeature(const CIVRConfPasswordFeature& other);
	const CIVRConfLeaderFeature*          GetConfLeaderFeature() const;
	void                                  SetConfLeaderFeature(const CIVRConfLeaderFeature& other);
	const CIVROperAssistanceFeature*      GetOperAssistanceFeature() const;
	void                                  SetOperAssistanceFeature(const CIVROperAssistanceFeature& other);
	const CIVRGeneralMsgsFeature*         GetGeneralMsgsFeature() const;
	void                                  SetGeneralMsgsFeature(const CIVRGeneralMsgsFeature& other);
	const CIVR_BillingCodeFeature*        GetBillingCodeFeature() const;
	void                                  SetBillingCodeFeature(const CIVR_BillingCodeFeature& other);
	const CIVRInvitePartyFeature*         GetInvitePartyFeature() const;
	void                                  SetInvitePartyFeature(const CIVRInvitePartyFeature& other);
	const CIVRRollCallFeature*            GetRollCallFeature() const;
	void                                  SetRollCallFeature(const CIVRRollCallFeature& other);
	const CIVRVideoFeature*               GetVideoFeature() const;
	void                                  SetVideoFeature(const CIVRVideoFeature& other);
	const CIVRNumericConferenceIdFeature* GetNumericConferenceIdFeature() const;
	void                                  SetNumericConferenceIdFeature(const CIVRNumericConferenceIdFeature& other);
	const CIVRMuteNoisyLineFeature*       GetMuteNoisyLineFeature() const;
	void                                  SetMuteNoisyLineFeature(const CIVRMuteNoisyLineFeature& other);

	const CIVRRecordingFeature*           GetRecordingFeature() const;
	void                                  SetRecordingFeature(const CIVRRecordingFeature& other);

	const CIVRPlaybackFeature*            GetPlaybackFeature() const;
	void                                  SetPlaybackFeature(const CIVRPlaybackFeature& other);

	CIVRFeature*                          GetIVRFeature(const WORD feature_opcode) const;

	WORD                                  GetIVRLanguageNumber() const;

	int                                   AddLanguage(CIVRLanguage& other);
	int                                   CancelLanguage(const char* language_name);
	int                                   FindLanguage(const CIVRLanguage& other);
	int                                   FindLanguage(const char* language_name);
	CIVRLanguage*                         GetCurrentIVRLanguage(const char* language_name) const;
	CIVRLanguage*                         GetCurrentIVRLanguage(const WORD language_DTMF_code) const;
	CIVRLanguage*                         GetCurrentIVRLanguagePos(const WORD pos) const;
	CIVRLanguage*                         GetFirstLanguage();
	CIVRLanguage*                         GetNextLanguage();

	const char*                           GetMsgFileName(const WORD feature_opcode, const WORD event_opcode, const char* language_name, int& status) const;
	const char*                           GetMsgFileName(const WORD feature_opcode, const WORD event_opcode, WORD language_pos, int& status) const;
	const char*                           GetMsgFileName(const WORD feature_opcode, const WORD event_opcode, int& status) const;

	void                                  GetFullPathMsgFileName(char* fullPathMsgName, const WORD feature_opcode, const WORD event_opcode, int& status) const;

	void                                  SetMsgFileName(const WORD feature_opcode, const WORD event_opcode, const char* language_name, const char* file_name, int& status);

	WORD                                  GetLanguagePos(const char* language_name, int& status) const;
	WORD                                  GetLanguagePos(WORD language_DTMF_code, int& status) const;

	const map<WORD, WORD>&                getInterfaceOrderMap();
	WORD                                  getDtmfForwardDuration();

	int                                   NewIVRLanguage(const char* language_name);
	int                                   RemoveIVRLanguage(const char* language_name);

	BYTE                                  GetRetriesNum() const;
	void                                  SetRetriesNum(const BYTE retriesNum);

	BYTE                                  GetUserInputTimeout() const;
	void                                  SetUserInputTimeout(const BYTE user_input_timeout);

	const char*                           GetDelimiter() const;
	void                                  SetDelimiter(const char* szDelimiter);

	BYTE                                  GetEntryQueueService() const;
	void                                  SetEntryQueueService(const BYTE IsEntryQueueService);
	void                                  ChangeTable();
	void                                  CorrectDTMFTable(CDTMFCodeList* pDTMF_codeListNew);
	void                                  ChangeGeneralMsgTable();
	void                                  UpdateIvrDirectories();
	void                                  AddDTMFCodesToList(CDTMFCodeList* pCDTMFCodeList, WORD isRecordingSystem = 0);

	WORD                                  GetIvrExternalDB() const;
	void                                  SetIvrExternalDB(const WORD wExtenalDB);

	int                                   GetIVRMsgParams(const WORD ivrFeatureOpcode, const WORD ivrEventOpcode, char* ivrMsgFullPath, WORD* ivrMsgDuration, WORD* ivrMsgCheckSum, time_t* ivrMsgLastModified, int* updateStatus) const;
	void                                  UpdateIVRMsgParamsInService(const WORD ivrFeatureOpcode, const WORD ivrEventOpcode, const WORD fileMsgDuration, const WORD fileMsgCheckSum, const time_t fileLastModified);

	BOOL                                  IsIdenticalToCurrent(CIVRService* other);
	WORD                                  GetDTMFCodePermission(const WORD DTMF_opcode);

public:
	char                            m_serviceName[AV_MSG_SERVICE_NAME_LEN]; // key field

	CDTMFCodeList*                  m_pDTMF_codeList;

	CIVRLangMenuFeature*            m_pLangMenu;
	CIVRWelcomeFeature*             m_pWelcome;
	CIVRConfPasswordFeature*        m_pConfPassword;
	CIVRConfLeaderFeature*          m_pConfLeader;
	CIVRPersonalPINCodeFeature*     m_pPersonalPINCode;
	CIVROperAssistanceFeature*      m_pOperAssistance;
	CIVRGeneralMsgsFeature*         m_pGeneralMsgs;
	CIVR_BillingCodeFeature*        m_pBillingCode;
	CIVRInvitePartyFeature*         m_pInviteParty;
	CIVRRollCallFeature*            m_pRollCall;
	CIVRVideoFeature*               m_pVideo;
	CIVRNumericConferenceIdFeature* m_pNumericId;
	CIVRMuteNoisyLineFeature*       m_pMuteNoisyLine;
	CIVRRecordingFeature*           m_pRecording;
	CIVRPlaybackFeature*            m_pPlayback;

	BYTE                            m_retries_num;
	BYTE                            m_user_input_timeout;
	char                            m_szDelimiter[MAX_DELIMETER_LEN];

	WORD                            m_numb_of_language;
	CIVRLanguage*                   m_pIVRLanguage[MAX_LANG_IN_IVR_SERVICE];
	BYTE                            m_bIsEntryQueueService;
	WORD                            m_wIvrExternalDB;

private:
	WORD m_ind_language;
};


////////////////////////////////////////////////////////////////////////////
//                        structs
////////////////////////////////////////////////////////////////////////////
typedef struct
{
	// TWavFullRiffChunk:
	char  acHeader[4];      // = RIFF ; improtant - should be 5 to allow end of string
	DWORD unTotalLength;    // total length minus 8 bytes
	char  acType[4];        // = WAVE  ;

	// TWavFullFormatChunk:
	char  acChunkId[4];     // = fmt_ ;
	DWORD unChunkSize;      // = 16+Extra Format Size
	WORD  usFormatTag;      // should be 0x01
	WORD  usChannels;       // 0x01 = MONO / 0x02 = Stereo
	DWORD unSamplesPerSec;  // should be 16000
	DWORD unAvgBytesPerSec; // 32000
	WORD  usBlockAlign;     // 2
	WORD  usBitsPerSample;  // 16

	// TWavDataChunk:
	char  acDataChunkId[4];
	DWORD unDataChunkSize;
} TCompletePcmHeader;
#define WAV_HEADER_SIZE     44

typedef struct
{
	char  acHeader[4];      // = RIFF ; improtant - should be 5 to allow end of string
	DWORD unTotalLength;    // total length minus 8 bytes
	char  acType[4];        // = WAVE  ;
} TRIFFHeader;
#define RIFF_HEADER_SIZE    12

typedef struct
{
	char  acChunkId[4];     // Chunk-ID (4 characters)
	DWORD unChunkSize;      // chunk data length
} TWaveChunk;
#define WAV_CHUNK_SIZE      8

typedef struct            // without the ID and size
{
	WORD  usFormatTag;      // should be 0x01
	WORD  usChannels;       // 0x01 = MONO / 0x02 = Stereo
	DWORD unSamplesPerSec;  // should be 16000
	DWORD unAvgBytesPerSec; // 32000
	WORD  usBlockAlign;     // 2
	WORD  usBitsPerSample;  // 16
} TFmtChunkData;
#define FMT_CHUNK_DATA_SIZE 16

#endif /* _IVRSERVICE_H__ */
