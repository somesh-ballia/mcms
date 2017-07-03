#ifndef _IVRSERVICELIST_H__
#define _IVRSERVICELIST_H__

#include "IVRService.h"
#include "SerializeObject.h"

class CXMLDOMElement;

////////////////////////////////////////////////////////////////////////////
//                        CAVmsgServiceList
////////////////////////////////////////////////////////////////////////////
class CAVmsgServiceList : public CSerializeObject
{
	CLASS_TYPE_1(CAVmsgServiceList, CSerializeObject)

public:
	                   CAVmsgServiceList();
	                   CAVmsgServiceList(const CAVmsgServiceList& other);
	virtual           ~CAVmsgServiceList();

	const char*        NameOf() const { return "CAVmsgServiceList";}
	CAVmsgServiceList& operator=(const CAVmsgServiceList& other);

	// Implementation
	void               Serialize(WORD format, std::ostream& m_ostr, WORD bIVR, DWORD apiNum);
	void               DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum);

	void               SerializeXml(CXMLDOMElement*& pFatherNode) const;
	void               SerializeXml(CXMLDOMElement* pFatherNode, DWORD ObjToken, const BYTE isIvr, BOOL removeGWIfCOP = FALSE) const;

	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

	CSerializeObject*  Clone()                                           { return new CAVmsgServiceList(); }

	int                SaveIvrListToFile(const char* fileName = NULL);
	int                SecureSaveIvrListToFile();

	WORD               GetServNumber() const                             { return m_numb_of_Serv; }
	WORD               GetIVRServNumber() const                          { return m_numb_of_IVR_Serv; }
	const char*        GetDefaultIVRName() const                         { return m_defaultIVRServiceName; }
	const char*        GetDefaultEQName() const                          { return m_defaultEQServiceName; }
	WORD               GetIVRServiceListFlag() const                     { return m_bIVRServiceListFlag; }
	BYTE               GetChanged() const                                { return m_bChanged; }

	DWORD              GetUpdateCounter() const                          { return m_updateCounter; }
	void               SetUpdateCounter(DWORD updateCounter)             { m_updateCounter = updateCounter; }
	void               IncreaseUpdateCounter();

	int                Add(CAVmsgService& other);
	int                AddOnlyMem(const CAVmsgService& other);
	int                Update(const CAVmsgService& other);
	int                Cancel(const char* name);
	int                FindAVmsgServ(const CAVmsgService& other);
	int                FindAVmsgServ(const char* name);
	CAVmsgService*     GetCurrentAVmsgService(const char* name);
	CAVmsgService*     GetAVmsgServiceInPos(WORD pos);
	CAVmsgService*     GetFirstService();
	CAVmsgService*     GetNextService();
	void               SetDefaultIVRName(const char* name);
	int                SetDefaultIVRName(const char* IvrServiceName, WORD setType);
	void               SetDefaultEQName(const char* name);
	BYTE               GetMusic(const char* name);
	int                IsDefaultService(const char* name);

	int                GetIVRMsgParams(const char* ivrServiceName, const WORD ivrFeatureOpcode, const WORD ivrEventOpcode,
	                                  char* ivrMsgFullPath, WORD* ivrMsgDuration, WORD* ivrMsgCheckSum, int* updateStatusRet = NULL);

	void               UpdateIVRMsgParamsInOtherServices(const WORD ivrFeatureOpcode, const WORD ivrEventOpcode,
	                                  const char* ivrMsgFullPath, const WORD ivrMsgDuration,
	                                  const WORD ivrMsgCheckSum, const time_t fileLastModified);

	// For files like Roll Call recordings and tone messages
	int                GetFileParams(const char* fileFullPath, WORD* duration, WORD* checksum);

	CIVRService*       GetIVRServiceByName(const char* ivrServiceName);
	DWORD              GetIsEQService(const char* ivrServiceName);
	void               SetAsDefaultIfNeeded(const CAVmsgService* ser);
	int                CheckIvrListValidity();
	BOOL               IsIdenticalToCurrent(CAVmsgServiceList* other);

	void               DeleteServices();

	int                AuditUpdateServiceIfNeeded(const CAVmsgService& serviceOld, const CAVmsgService& serviceNew) const;
	void               SendAuditEvent(const std::string strEvent, const std::string strDescription) const;

public:
	WORD               m_numb_of_Serv;        // total number; AV + IVR
	char               m_defaultIVRServiceName[AV_MSG_SERVICE_NAME_LEN];
	char               m_defaultEQServiceName[AV_MSG_SERVICE_NAME_LEN];
	CAVmsgService*     m_pAVmsgService[MAX_IVR_SERV_IN_LIST];
	WORD               m_bIVRServiceListFlag; // used by Operator

private:
	WORD               m_ind_serv;
	DWORD              m_updateCounter;
	WORD               m_numb_of_IVR_Serv;
	BYTE               m_bChanged;
};

#endif /* _IVRSERVICELIST_H__ */
