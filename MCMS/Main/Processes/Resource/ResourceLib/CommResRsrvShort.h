#ifndef _COMM_RES_RSRV_SHORT_H_
#define _COMM_RES_RSRV_SHORT_H_

#include "PObject.h"
#include "OperMask.h"
#include "StructTm.h"
#include "ConfPartySharedDefines.h"
#include "CdrApiClasses.h"

class CXMLDOMElement;

#define  COPERBITOFFINRSRVSHORT(a, bit) if (a) a->SetSlowInfoBit(bit, FALSE);
#define  COPERMASKONINRSRVSHORT(a)  if (a) a->SetSlowInfoMask(TRUE);

class CCommResRsrvShort : public CPObject
{
	CLASS_TYPE_1(CCommResRsrvShort, CPObject)
public:
	//Constructors
	CCommResRsrvShort();
	CCommResRsrvShort(const CCommResRsrvShort& other);
	CCommResRsrvShort& operator = (const CCommResRsrvShort& other);
	virtual ~CCommResRsrvShort();

//	   // Implementation
	void              SerializeXml(CXMLDOMElement* pActionNode, int opcode, WORD type);
	int               DeSerializeXml(CXMLDOMElement* pSummaryNOperMaskode, int nType, char* pszError);
	const char*       NameOf() const;
	void              SetName(const char* name);
	const char*       GetName () const;
	void              SetDisplayName(const char* name);
	const char*       GetDisplayName () const;
	void              SetConferenceId(const DWORD confId);
	DWORD             GetConferenceId () const;
	DWORD             GetRepSchedulingId() const;
	void              SetRepSchedulingId(const DWORD repSchedulingId);
	void              SetAdHocProfileId(const DWORD profileId);
	DWORD             GetAdHocProfileId() const;
	void              SetContinuousPresenceScreenNumber(const BYTE contPresScreenNumber);
	BYTE              GetContinuousPresenceScreenNumber () const;
	void              SetAutoLayoutFlag(const BYTE isAutoLayout);
	BYTE              isAutoLayout () const;
	void              SetStartTime(const CStructTm& other);
	const CStructTm*  GetStartTime() const;
	void              SetDurationTime(const CStructTm& other);
	const CStructTm*  GetDurationTime() const;
	void              SetStatus(BYTE status); //Status Field
	BYTE              GetStatus();
	const CStructTm*  GetEndTime();
	const CStructTm*  GetCalculatedEndTime();
	BYTE              GetMeetingRoomState() const;
	void              SetMeetingRoomState(const BYTE meetingRoomState);
	BYTE              GetMeetMePerEntryQ () const;
	void              SetMeetMePerEntryQ(const BYTE meetMePerEntryQ);
	BYTE              GetOperatorConf () const;
	void              SetOperatorConf(const BYTE operator_conf);
	void              SetEntryPassword(const char* leader_password);
	const char*       GetEntryPassword () const;
	void              SetNumericConfId(const char* numericConfId);
	const char*       GetNumericConfId () const;
	void              SetSlowInfoMask(WORD onOff);
	void              SetRsrvFlags(const DWORD dwResFlags);
	void              SetRsrvFlags2(const DWORD flags2);
	DWORD             GetRsrvFlags () const;
	void              SetEncryptionType (BYTE confEncryptionType);
	BYTE              GetEncryptionType () const;
	void              SetPassw(const char* szPassw);
	const char*       GetPassw () const;
	void              SetSummeryCreationUpdateCounter(DWORD summaryDBCounter);
	DWORD             GetSummeryCreationUpdateCounter() const;
	void              SetSummeryUpdateCounter(WORD summaryDBCounter);
	DWORD             GetSummeryUpdateCounter();
	DWORD             GetNumParties() const;
	void              SetNumParties(const DWORD NumParties);
	DWORD             GetNumUndefParties() const;
	void              SetNumUndefParties(const DWORD NumUndefParties);
	BYTE              GetConfTransferRate () const;
	void              SetConfTransferRate(const BYTE confTransferRate);
	BYTE              GetNetwork() const;
	void              SetNetwork(const BYTE network);
	WORD              GetInfoOpcode() const;
	std::string       GetFileUniqueName() const                      {return m_commResFileName; }
	void              SetFileUniqueName(std::string commResFileName) { m_commResFileName = commResFileName; }
	bool              IsEntryQ() const                               {return ((m_dwRsrvFlags & ENTRY_QUEUE) == ENTRY_QUEUE); }
	bool              IsSIPFactory() const                           {return (m_dwRsrvFlags & SIP_FACTORY) == SIP_FACTORY; }
	WORD              GetNumServicePhone() const;
	CServicePhoneStr* GetFirstServicePhone();
	CServicePhoneStr* GetNextServicePhone();
	int               AddServicePhone(const CServicePhoneStr& other);
	int               FindServicePhone(const CServicePhoneStr& other);

protected:

	// Attributes
	char              m_H243confName[H243_NAME_LEN];  //conferences name
	char              m_confDisplayName[H243_NAME_LEN];
	char              m_H243_password[H243_NAME_LEN]; //password
	DWORD             m_confId;                       //conferences Id
	DWORD             m_repSchedulingId;              //id for repeated id
	CStructTm         m_startTime;                    //start conferences time
	CStructTm         m_duration;                     //conferences duration time
	BYTE              m_status;                       //Status Field
	CStructTm         m_endTime;                      //end   conferences time
	BYTE              m_meetingRoomState;
	DWORD             m_webReservUId;
	DWORD             m_webOwnerUId;
	DWORD             m_webDBId;
	BYTE              m_webReserved;
	COperMask         m_slowInfoMask;
	DWORD             m_dwRsrvFlags;
	DWORD             m_dwRsrvFlags2;
	char              m_entry_password[CONFERENCE_ENTRY_PASSWORD_LEN]; //conference le entry password
	BYTE              m_meetMePerEntryQ;                               // YES, NO
	char              m_NumericConfId[NUMERIC_CONFERENCE_ID_LEN];
	DWORD             m_SummeryCreationUpdateCounter;
	DWORD             m_SummeryUpdateCounter;
	DWORD             m_numParties;
	DWORD             m_numUndefParties;
	BYTE              m_confTransferRate;
	BYTE              m_network;
	DWORD             m_dwAdHocProfileId;
	BYTE              m_contPresScreenNumber;
	BYTE              m_isAutoLayout;
	WORD              m_infoOpcode;
	std::string       m_commResFileName;
	BYTE              m_operatorConf;
	BYTE              m_eEncryptionType;
	WORD              m_numServicePhoneStr;
	WORD              m_ind_service_phone;
	CServicePhoneStr* m_pServicePhoneStr[MAX_NET_SERV_PROVIDERS_IN_LIST];
};

#endif
