// ChangePassword.h: interface for the CChangePassword class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AUDIBLEALARM__H_)
#define AUDIBLEALARM__H_

#include "SerializeObject.h"


/////////////////////////////////////////////////////////////////////////////
// CChangePassword

class CAudibleAlarm : public CSerializeObject
{

	CLASS_TYPE_1(CAudibleAlarm, CSerializeObject)
	public:
	    CAudibleAlarm();
		~CAudibleAlarm();
		CAudibleAlarm& operator=(const CAudibleAlarm&);
		virtual const char*  NameOf() const {return "CFailoverConfiguration";}
		CSerializeObject* Clone() {return new CAudibleAlarm;}
		void SerializeXml(CXMLDOMElement*& pFatherNode) const;
		//void   SerializeXml(CXMLDOMElement*& pFatherNode,DWORD ObjToken);
		int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
		int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

		WORD GetIsEnabled()const {return m_bIsEnabled;}
		std::string GetUserName()const {return m_user_name;}
		//int GetOtherRmxPort()const {return m_strOtherRmxPort;}
		//DWORD GetHotBackupType()const {return m_HotBackupType;}
	    //void SetMasterSlaveState(eFailoverStatusType eStatusType) {m_statusType=eStatusType;}
	    //void SetHotBackupType(DWORD hotBackupType) {m_HotBackupType = hotBackupType;}

	private:

	    //CFailoverConfiguration(const CFailoverConfiguration&);
		//std::string m_strOtherRmxIp;
		DWORD m_AudibleAlarmType;
	    WORD m_bIsEnabled;
	    WORD m_bIsRepeat;
	    DWORD m_numOfRepetitions;
	    DWORD m_Repetitions_interval;
	    std::string m_user_name;
};

#endif // !defined(CHANGEPASSWORD__H_)
