#include "NStream.h"
#include "psosxml.h"
#include "IVRServiceList.h"
#include "ConfPartyDefines.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "ConfPartyStatuses.h"
#include "OsFileIF.h"
#include "AuditorApi.h"
#include "TraceStream.h"
#include "ConfPartyGlobals.h"

////////////////////////////////////////////////////////////////////////////
//                        CAVmsgServiceList
////////////////////////////////////////////////////////////////////////////
CAVmsgServiceList::CAVmsgServiceList()
{
	m_numb_of_Serv             = 0;
	m_numb_of_IVR_Serv         = 0;
	m_ind_serv                 = 0;
	m_defaultIVRServiceName[0] = '\0';
	m_defaultEQServiceName[0]  = '\0';
	m_bIVRServiceListFlag      = FALSE;
	m_bChanged                 = FALSE;
	m_updateCounter            = 0;

	memset(m_pAVmsgService, 0, sizeof(m_pAVmsgService));
}

//--------------------------------------------------------------------------
CAVmsgServiceList& CAVmsgServiceList::operator=(const CAVmsgServiceList& other)
{
	for (int i = 0; i < MAX_IVR_SERV_IN_LIST; i++)
	{
		PDELETE(m_pAVmsgService[i]);
		if (other.m_pAVmsgService[i])
			m_pAVmsgService[i] = new CAVmsgService(*other.m_pAVmsgService[i]);
	}

	m_numb_of_Serv        = other.m_numb_of_Serv;
	m_numb_of_IVR_Serv    = other.m_numb_of_IVR_Serv;
	m_ind_serv            = other.m_ind_serv;
	m_bIVRServiceListFlag = other.m_bIVRServiceListFlag;
	m_updateCounter       = other.m_updateCounter;
	m_bChanged            = other.m_bChanged;

	strcpy_safe(m_defaultIVRServiceName, other.m_defaultIVRServiceName);
	strcpy_safe(m_defaultEQServiceName, other.m_defaultEQServiceName);

	return *this;
}

//--------------------------------------------------------------------------
CAVmsgServiceList::CAVmsgServiceList(const CAVmsgServiceList& other) : CSerializeObject(other)
{
	memset(m_pAVmsgService, 0, sizeof(m_pAVmsgService));

	*this = other;
}

//--------------------------------------------------------------------------
CAVmsgServiceList::~CAVmsgServiceList()
{
	for (int i = 0; i < MAX_IVR_SERV_IN_LIST; i++)
		PDELETE(m_pAVmsgService[i]);
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::Serialize(WORD format, std::ostream& m_ostr, WORD bIVR, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS

	int  i;
	WORD num_serv = 0;

	num_serv = m_numb_of_IVR_Serv;

	m_ostr <<  num_serv  << "\n";
	m_ostr <<  m_defaultIVRServiceName  << "\n";
	m_ostr <<  m_defaultEQServiceName   << "\n";

	if (num_serv != 0)
		m_ostr << "~" << "\n";
	else
		return;

	for (i = 0; i < (int)m_numb_of_Serv; i++)
	{
		if (bIVR)
		{
			if (m_pAVmsgService[i]->GetIVRServiceFlag())
			{
				m_pAVmsgService[i]->Serialize(format, m_ostr, apiNum);

				m_ostr << "~" << "\n";
			}
		}
		else
		{
			if (!m_pAVmsgService[i]->GetIVRServiceFlag())
			{
				m_pAVmsgService[i]->Serialize(format, m_ostr, apiNum);

				m_ostr << "~" << "\n";
			}
		}
	}
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS

	int  i;
	char s;

	m_istr >> m_numb_of_Serv;
	m_istr.ignore(1);
	m_istr.getline(m_defaultIVRServiceName, AV_MSG_SERVICE_NAME_LEN+1, '\n');
	m_istr.getline(m_defaultEQServiceName, AV_MSG_SERVICE_NAME_LEN+1, '\n');

	if (m_numb_of_Serv != 0)
	{
		m_istr >> s;
		if (s != '~') return;
	}

	for (i = 0; i < (int)m_numb_of_Serv; i++)
	{
		m_pAVmsgService[i] = new CAVmsgService;
		m_pAVmsgService[i]->DeSerialize(format, m_istr, apiNum);

		m_numb_of_IVR_Serv++;

		m_istr >> s;
		if (s != '~') return;
	}
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	SerializeXml(pFatherNode, 0xFFFFFFFF, true);
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::SerializeXml(CXMLDOMElement* pFatherNode, DWORD ObjToken, const BYTE isIvr, BOOL removeGWIfCOP) const
{
	DWORD bChanged = FALSE;

	CXMLDOMElement* pServListNode = pFatherNode->AddChildNode("AV_SERVICE_LIST");

	pServListNode->AddChildNode("OBJ_TOKEN", m_updateCounter);

	if (ObjToken == 0xFFFFFFFF)
		bChanged = TRUE;
	else
	{
		if (m_updateCounter > ObjToken)
			bChanged = TRUE;
	}

	pServListNode->AddChildNode("CHANGED", bChanged, _BOOL);

	if (bChanged)
	{
		pServListNode->AddChildNode("DEFAULT_SERVICE_NAME", m_defaultIVRServiceName);
		pServListNode->AddChildNode("DEFAULT_EQ_SERVICE_NAME", m_defaultEQServiceName);

		for (int i = 0; i < m_numb_of_Serv; i++)
		{
			// VNGR-23571 - GW IVR Service and Gateway Profile should be unavailable in COP mode.
			if (::GetIsCOPdongleSysMode() && removeGWIfCOP == TRUE && strcmp(m_pAVmsgService[i]->GetName(), "GW IVR Service") == 0)
			{
				continue;
			}
			m_pAVmsgService[i]->SerializeXml(pServListNode);
		}
	}
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
	TRACECOND_AND_RETURN_VALUE(!pActionNode, "CAVmsgServiceList::DeSerializeXml - Failed, pActionNode is NULL", STATUS_FAIL);

	int status = STATUS_OK;

	CXMLDOMElement* nextItem;
	pActionNode->firstChildNode(&nextItem);

	if (nextItem)
		status = DeSerializeXml(nextItem, pszError);

	return status;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	m_bChanged = TRUE;

	// in case of error the nStatus is updated
	GET_VALIDATE_CHILD(pActionNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode, "CHANGED", &m_bChanged, _BOOL);

	if (m_bChanged)
	{
		// in case of error the nStatus is updated
		GET_VALIDATE_CHILD(pActionNode, "DEFAULT_SERVICE_NAME", m_defaultIVRServiceName, _0_TO_AV_MSG_SERVICE_NAME_LENGTH);
		GET_VALIDATE_CHILD(pActionNode, "DEFAULT_EQ_SERVICE_NAME", m_defaultEQServiceName, _0_TO_AV_MSG_SERVICE_NAME_LENGTH);

		// cleans previous list
		for (int i = 0; i < m_numb_of_Serv; i++)
			POBJDELETE(m_pAVmsgService[i]);

		// initialize counters
		m_numb_of_IVR_Serv = m_numb_of_Serv = 0;

		CXMLDOMElement* pCurrentChild = NULL;

		pActionNode->firstChildNode(&pCurrentChild);
		while (pCurrentChild && m_numb_of_Serv < MAX_IVR_SERV_IN_LIST)
		{
			char* pszChildName = NULL;
			pCurrentChild->get_nodeName(&pszChildName);

			if (!strcmp(pszChildName, "IVR_SERVICE") && m_numb_of_IVR_Serv < MAX_IVR_SERV_IN_LIST)
			{
				m_pAVmsgService[m_numb_of_Serv] = new CAVmsgService;
				nStatus = m_pAVmsgService[m_numb_of_Serv]->DeSerializeXml(pCurrentChild, pszError);
				if (nStatus != STATUS_OK)
				{
					POBJDELETE(m_pAVmsgService[m_numb_of_Serv]);
					break;
				}
				m_numb_of_Serv++;
				m_numb_of_IVR_Serv++;
			}
			pActionNode->nextChildNode(&pCurrentChild);
		}
	}
	return nStatus;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::Add(CAVmsgService& other)
{
	const char* serviceName = other.GetName();

	if (m_numb_of_IVR_Serv >= MAX_IVR_SERV_IN_LIST)
	{
		TRACEINTO << "CAVmsgServiceList::Add - ServiceName:" << serviceName << ", Failed, Number of IVR services exceeded";
		return STATUS_MAX_NUMBER_OF_IVR_SERVICES_EXCEEDED;
	}

	if (FindAVmsgServ(other) != NOT_FIND)
	{
		TRACEINTO << "CAVmsgServiceList::Add - ServiceName:" << serviceName << ", Failed, IVR service already exists";
		return STATUS_MSG_SERVICE_NAME_ALREADY_EXISTS;
	}

	if (0 == GetIsEQService(serviceName))   // IVR service
	{
		if (m_defaultIVRServiceName[0] == '\0')
		{
			strcpy_safe(m_defaultIVRServiceName, serviceName);
		}
	}
	else if (1 == GetIsEQService(serviceName))  // EQ service
	{
		if (m_defaultEQServiceName[0] == '\0')
		{
			strcpy_safe(m_defaultEQServiceName, serviceName);
		}
	}

	m_pAVmsgService[m_numb_of_Serv] = new CAVmsgService(other);

	m_numb_of_Serv++;
	m_numb_of_IVR_Serv++;

	IncreaseUpdateCounter();

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::AddOnlyMem(const CAVmsgService& other)
{
	const char* serviceName = other.GetName();

	if (m_numb_of_IVR_Serv >= MAX_IVR_SERV_IN_LIST)
	{
		TRACEINTO << "CAVmsgServiceList::AddOnlyMem - ServiceName:" << serviceName << ", Failed, Number of IVR services exceeded";
		return STATUS_MAX_NUMBER_OF_IVR_SERVICES_EXCEEDED;
	}

	if (FindAVmsgServ(other) != NOT_FIND)
	{
		TRACEINTO << "CAVmsgServiceList::AddOnlyMem - ServiceName:" << serviceName << ", Failed, IVR service already exists";
		return STATUS_MSG_SERVICE_NAME_ALREADY_EXISTS;
	}

	// sets the DTMF codes length, checks the correctness
	CAVmsgService* pTempAvMsgService = new CAVmsgService(other);
	pTempAvMsgService->SetDtmfCodesLen();
	if (0 != pTempAvMsgService->CheckLegalDtmfTbl())
	{
		POBJDELETE(pTempAvMsgService);
		TRACEINTO << "CAVmsgServiceList::AddOnlyMem - ServiceName:" << serviceName << ", Failed, Illegal DTMF table";
		return STATUS_IVR_ILLEAGAL_DTMF_TBL;
	}

	// get the service, update the counter
	m_pAVmsgService[m_numb_of_Serv++] = pTempAvMsgService;
	m_numb_of_IVR_Serv++;

	IncreaseUpdateCounter();

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::Update(const CAVmsgService& other)
{
	int ind = FindAVmsgServ(other);
	if (ind == NOT_FIND)
	{
		const char* serviceName = other.GetName();
		TRACEINTO << "CAVmsgServiceList::Update - ServiceName:" << serviceName << ", Failed, IVR Service not found";
		return STATUS_MSG_SERVICE_NAME_DOES_NOT_EXISTS;
	}

	PDELETE(m_pAVmsgService[ind]);
	m_pAVmsgService[ind] = new CAVmsgService(other);

	IncreaseUpdateCounter();

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::IsDefaultService(const char* name)
{
	if ((0 == strcmp(m_defaultIVRServiceName, name)) || (0 == strcmp(m_defaultEQServiceName, name)))
		return 1;

	return 0;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::Cancel(const char* name)
{
	TRACECOND_AND_RETURN_VALUE(!name, "CAVmsgServiceList::Cancel - Failed, Invalid IVR service name", STATUS_MSG_SERVICE_NAME_DOES_NOT_EXISTS);

	// if find, "ind" holds the index to the IVR service to be delete
	int ind = FindAVmsgServ(name);
	if (ind == NOT_FIND)
	{
		TRACEINTO << "CAVmsgServiceList::Cancel - ServiceName:" << name << ", Failed, IVR Service not found";
		return STATUS_MSG_SERVICE_NAME_DOES_NOT_EXISTS;
	}

	if ((!strcmp(m_defaultIVRServiceName, name)) || (!strcmp(m_defaultEQServiceName, name)))
	{
		// Default service cannot be removed
		TRACEINTO << "CAVmsgServiceList::Cancel - ServiceName:" << name << ", Default IVR Service cannot be removed";
		return STATUS_DEFAULT_MSG_SERVICE_CANNOT_BE_REMOVED;
	}

	// delete IVR Service (still will be NULL item in the list)
	PDELETE(m_pAVmsgService[ind]);

	// remove the deleted IVR (NULL item) from the list
	for (int j = ind; j < (int)m_numb_of_Serv-1; j++)
		m_pAVmsgService[j] = m_pAVmsgService[j+1];

	m_pAVmsgService[m_numb_of_Serv-1] = NULL;
	m_numb_of_Serv--;
	m_numb_of_IVR_Serv--;

	IncreaseUpdateCounter();

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::FindAVmsgServ(const CAVmsgService& other)
{
	return FindAVmsgServ(other.GetName());
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::FindAVmsgServ(const char* name)
{
	for (int i = 0; i < (int)m_numb_of_Serv; i++)
	{
		if (m_pAVmsgService[i] != NULL)
		{
			if (!strcmp(m_pAVmsgService[i]->GetName(), name))
				return i;
		}
	}
	return NOT_FIND;
}

//--------------------------------------------------------------------------
CAVmsgService* CAVmsgServiceList::GetCurrentAVmsgService(const char* name)
{
	for (int i = 0; i < (int)m_numb_of_Serv; i++)
	{
		if (m_pAVmsgService[i] != NULL)
		{
			if (!strcmp(m_pAVmsgService[i]->GetName(), name))
				return m_pAVmsgService[i];
		}
	}

	return NULL;          // STATUS_MSG_SERVICE_NAME_ALREADY_EXISTS
}

//--------------------------------------------------------------------------
CAVmsgService* CAVmsgServiceList::GetAVmsgServiceInPos(WORD pos)
{
	CAVmsgService* pAVmsgService = NULL;

	if (pos < GetServNumber())
		pAVmsgService = m_pAVmsgService[pos];

	return pAVmsgService;
}

//--------------------------------------------------------------------------
CAVmsgService* CAVmsgServiceList::GetFirstService()
{
	m_ind_serv = 1;
	return m_pAVmsgService[0];
}

//--------------------------------------------------------------------------
CAVmsgService* CAVmsgServiceList::GetNextService()
{
	if (m_ind_serv >= m_numb_of_Serv)
		return NULL;

	return m_pAVmsgService[m_ind_serv++];
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::SetDefaultIVRName(const char* name)
{
	strcpy_safe(m_defaultIVRServiceName, name);

	IncreaseUpdateCounter();
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::SetDefaultEQName(const char* name)
{
	strcpy_safe(m_defaultEQServiceName, name);

	IncreaseUpdateCounter();
}

//--------------------------------------------------------------------------
BYTE CAVmsgServiceList::GetMusic(const char* name)
{
	CAVmsgService* pAVmsgService = GetCurrentAVmsgService(name);
	return (pAVmsgService) ? (BYTE)pAVmsgService->IsMusic() : NO;
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::IncreaseUpdateCounter()
{
	m_updateCounter++;
	if (m_updateCounter == 0xFFFFFFFF)
		m_updateCounter = 0;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::GetIVRMsgParams(const char* ivrServiceName, const WORD ivrFeatureOpcode, const WORD ivrEventOpcode,
                                       char* ivrMsgFullPath, WORD* ivrMsgDuration, WORD* ivrMsgCheckSum, int* updateStatusRet)
{
	if (updateStatusRet)    // can be NULL
		*updateStatusRet = 0; // no need to update

	// checks the service name
	TRACECOND_AND_RETURN_VALUE(!ivrServiceName, "CAVmsgServiceList::GetIVRMsgParams - Failed, Invalid IVR service name", STATUS_FAIL);

	// checks the service name
	TRACECOND_AND_RETURN_VALUE(!strlen(ivrServiceName), "CAVmsgServiceList::GetIVRMsgParams - Failed, Empty IVR service name", STATUS_FAIL);

	// Get the relevant IVR Service
	CAVmsgService* pAVmsgService = GetCurrentAVmsgService(ivrServiceName);
	TRACECOND_AND_RETURN_VALUE(!pAVmsgService, "CAVmsgServiceList::GetIVRMsgParams - ServiceName:" << ivrServiceName << ", Failed, Invalid pAVmsgService", STATUS_FAIL);

	const CIVRService* pIVRService = pAVmsgService->GetIVRService();
	TRACECOND_AND_RETURN_VALUE(!pAVmsgService, "CAVmsgServiceList::GetIVRMsgParams - ServiceName:" << ivrServiceName << ", Failed, Invalid pIVRService", STATUS_FAIL);

	// Get IVR message Parameters
	time_t ivrMsgLastModified = 0;
	int    updateStatus       = -1;
	int    status             = pIVRService->GetIVRMsgParams(ivrFeatureOpcode, ivrEventOpcode,
	                                                         ivrMsgFullPath, ivrMsgDuration,
	                                                         ivrMsgCheckSum, &ivrMsgLastModified, &updateStatus);

	TRACECOND_AND_RETURN_VALUE(STATUS_OK != status, "CAVmsgServiceList::GetIVRMsgParams - ServiceName:" << ivrServiceName << ", Failed, Status:" << status, status);

	// In case the IVR message was changed - go over the IVRServiceList and update the
	// parameters of each appearance of this message file
	if (STATUS_NEED_TO_UPDATE == updateStatus)
	{
		TRACEINTO << "CAVmsgServiceList::GetIVRMsgParams - ServiceName:" << ivrServiceName << ", Status:STATUS_NEED_TO_UPDATE";
		UpdateIVRMsgParamsInOtherServices(ivrFeatureOpcode, ivrEventOpcode, ivrMsgFullPath, *ivrMsgDuration, *ivrMsgCheckSum, ivrMsgLastModified);
		if (updateStatusRet)    // can be NULL
			*updateStatusRet = 1; // need to update
	}
	return status;
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::UpdateIVRMsgParamsInOtherServices(const WORD ivrFeatureOpcode, const WORD ivrEventOpcode,
                                                          const char* ivrMsgFullPath, const WORD ivrMsgDuration,
                                                          const WORD ivrMsgCheckSum, const time_t ivrMsgLastModified)
{
	// Get the first IVR Service
	CAVmsgService* pAVmsgService = GetFirstService();
	char           serviceIvrMsgFullPath[MAX_FULL_PATH_LEN];
	serviceIvrMsgFullPath[0] = '\0';

	for (int i = 1; i <= m_numb_of_Serv; i++)
	{
		TRACECOND_AND_RETURN(!pAVmsgService, "CAVmsgServiceList::UpdateIVRMsgParamsInOtherServices - Index:" << i << ", Failed, Invalid pAVmsgService");

		const CIVRService* pIVRService = pAVmsgService->GetIVRService();
		if (NULL == pIVRService)
		{
			TRACEINTO << "CAVmsgServiceList::UpdateIVRMsgParamsInOtherServices - Index:" << i << ", Failed, Invalid pIVRService";
			continue;
		}

		// Get full path of the relevant IVR message file in the current IVR service
		int fullPathStatus = STATUS_OK;
		pIVRService->GetFullPathMsgFileName(serviceIvrMsgFullPath, ivrFeatureOpcode, ivrEventOpcode, fullPathStatus);
		if (STATUS_IVR_MSG_NAME_MISSING == fullPathStatus)
		{
			// No file for this event in this service
			continue;
		}

		if (STATUS_OK != fullPathStatus)
		{
			TRACEINTO << "CAVmsgServiceList::UpdateIVRMsgParamsInOtherServices - Index:" << i << ", Failed, Status:" << fullPathStatus;
			continue;
		}

		// Check if the message file that is used in the current IVR service is the one that should be updated.
		if (0 == (strcmp(serviceIvrMsgFullPath, ivrMsgFullPath)))
			((CIVRService*)pIVRService)->UpdateIVRMsgParamsInService(ivrFeatureOpcode, ivrEventOpcode, ivrMsgDuration,  // Casting for canceling the const
			                                                         ivrMsgCheckSum, ivrMsgLastModified);

		// Get the next IVR service in list
		pAVmsgService = GetNextService();
	}
}

//--------------------------------------------------------------------------
// For files like Roll Call recordings and tone messages
int CAVmsgServiceList::GetFileParams(const char* fileFullPath, WORD* duration, WORD* checksum)
{
	CIVRFeature temp;
	int status = temp.CheckLegalFileAndGetParams(fileFullPath, duration, checksum);
	if (status != STATUS_OK)
		return STATUS_IVR_INVALID_AUDIO_FILE;

	return STATUS_OK;
}

//--------------------------------------------------------------------------
CIVRService* CAVmsgServiceList::GetIVRServiceByName(const char* ivrServiceName)
{
	TRACECOND_AND_RETURN_VALUE(!ivrServiceName, "CAVmsgServiceList::GetIVRServiceByName - Failed, Invalid IVR service name", NULL);

	CAVmsgService* pAVmsgService = GetCurrentAVmsgService(ivrServiceName);
	TRACECOND_AND_RETURN_VALUE(!pAVmsgService, "CAVmsgServiceList::GetIVRServiceByName - ServiceName:" << ivrServiceName << ", Failed, Invalid pAVmsgService", NULL);

	return (CIVRService*)pAVmsgService->GetIVRService();
}

//--------------------------------------------------------------------------
DWORD CAVmsgServiceList::GetIsEQService(const char* ivrServiceName)
{
	TRACECOND_AND_RETURN_VALUE(!ivrServiceName, "CAVmsgServiceList::GetIsEQService - Failed, Invalid IVR service name", (DWORD)(-1));

	CIVRService* pIVRService = GetIVRServiceByName(ivrServiceName);
	TRACECOND_AND_RETURN_VALUE(!pIVRService, "CAVmsgServiceList::GetIsEQService - ServiceName:" << ivrServiceName << ", Failed, Invalid pIVRService", (DWORD)(-1));

	// Check if it is an IVR service (0) or an EQ service (1)
	return pIVRService->GetEntryQueueService();
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::SetDefaultIVRName(const char* ivrServiceName, WORD isEQSet)
{
	TRACECOND_AND_RETURN_VALUE(!ivrServiceName, "CAVmsgServiceList::SetDefaultIVRName - Failed, Invalid IVR service name", STATUS_IVR_SERVICE_NAME_DOES_NOT_EXISTS);

	int index = FindAVmsgServ(ivrServiceName);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "CAVmsgServiceList::SetDefaultIVRName - ServiceName:" << ivrServiceName << ", Failed, IVR Service not found", STATUS_IVR_SERVICE_NAME_DOES_NOT_EXISTS);

	int status = STATUS_OK;

	// Check if it is an IVR service or an EQ service
	DWORD isEQ = GetIsEQService(ivrServiceName);
	if (1 == isEQ)          // EQ Service
	{
		if (0 == isEQSet)     // Error: EQ service and need to set IVR default
		{
			TRACEINTO << "CAVmsgServiceList::SetDefaultIVRName - ServiceName:" << ivrServiceName << ", Failed, Mismatch IVR types - not an IVR Conference type";
			status = STATUS_INCOMPATIBLE_IVR_DEFAULT_SERVICE;
		}
		else                                  // IVR Conf service
			SetDefaultEQName(ivrServiceName);   // set as default
	}
	else                                    // IVR Service
	{
		if (1 == isEQSet)                     // Error: IVR service and need to set EQ default
		{
			TRACEINTO << "CAVmsgServiceList::SetDefaultIVRName - ServiceName:" << ivrServiceName << ", Failed, Mismatch IVR types - not an IVR EQ type";
			status = STATUS_INCOMPATIBLE_IVR_DEFAULT_SERVICE;
		}
		else                                  // IVR Conf service
			SetDefaultIVRName(ivrServiceName);  // set as default
	}
	return status;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::SaveIvrListToFile(const char* fileName)
{
	int status = STATUS_OK;

	// here is the idea for the future:
	// 1. save to temp file: IVRServiceList_Temp.xml
	// 2. read the temp file to temp list to check if it was written ok
	// 3. if all ok delete "last good file"
	// 4. rename real file to "last good file"
	// 5. rename temp file to real file

	// write IVR Service list to XML file
	std::string xmlFileName = FILE_IVR_CONFIG_XML;
	if (NULL != fileName)
		xmlFileName = fileName;

	status = WriteXmlFile(xmlFileName.c_str(), "IVRServiceList");
	if (STATUS_OK != status)
		TRACEINTO << "CAVmsgServiceList::SaveIvrListToFile - FileName:" << xmlFileName.c_str() << ", Failed, Status:" << status;

	return status;
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::SecureSaveIvrListToFile()
{
	// Save the temporary updated IVR Service list to a temporary XML file
	std::string xmlFileName = FILE_IVR_CONFIG_XML_TEMP;
	int status = SaveIvrListToFile(xmlFileName.c_str());
	if (status != STATUS_OK)
	{
		TRACEINTO << "CAVmsgServiceList::SecureSaveIvrListToFile - FileName:" << xmlFileName.c_str() << ", Failed to save IVR list to file, Status:" << status;
		return STATUS_FILE_WRITE_ERROR;
	}

	// Check if the XML temporary file can be read to IVR temporary list
	CAVmsgServiceList* pTempServiceListForTest = new CAVmsgServiceList;
	status = pTempServiceListForTest->ReadXmlFile(xmlFileName.c_str(), eNoActiveAlarm, eRenameFile);
	if (status != STATUS_OK)
	{
		TRACEINTO << "CAVmsgServiceList::SecureSaveIvrListToFile - FileName:" << xmlFileName.c_str() << ", Failed to read IVR list from file, Status:" << status;
		status = STATUS_FILE_READ_ERROR;
	}
	else
	{
		// Compare the two temporary lists in order to eliminate any undesirable changes
		if (!IsIdenticalToCurrent(pTempServiceListForTest))
		{
			TRACEINTO << "CAVmsgServiceList::SecureSaveIvrListToFile - FileName:" << xmlFileName.c_str() << ", Failed, The read list is not identical to the list before save";
			status = STATUS_FAIL; // this shouldn't happen, its internal problem
		}
	}

	// Delete the temporary list that was read from the disk (in any case)
	POBJDELETE(pTempServiceListForTest);

	if (status != STATUS_OK)  // if failed to read the file or if the list is not identical to original one
		return status;

	// Delete the old IVR service list and rename the temp list to the main list name
	BOOL bDeleted = DeleteFile(FILE_IVR_CONFIG_XML);
	if (!bDeleted)
	{
		TRACEINTO << "CAVmsgServiceList::SecureSaveIvrListToFile - FileName:" << FILE_IVR_CONFIG_XML << ", Failed, Cannot delete a file";
		return STATUS_FILE_DELETE_ERROR;
	}

	BOOL bRenamed = RenameFile(FILE_IVR_CONFIG_XML_TEMP, FILE_IVR_CONFIG_XML);
	if (!bRenamed)
	{
		TRACEINTO << "CAVmsgServiceList::SecureSaveIvrListToFile - FileName:" << FILE_IVR_CONFIG_XML << ", Failed, Cannot rename a file";
	}

	// Save a copy of IVRServiceList.xml in IVRServiceList_LastGoodCopy.xml
	BOOL bCopied = CopyFile(FILE_IVR_CONFIG_XML, FILE_IVR_CONFIG_XML_LAST_GOOD_COPY);
	if (!bCopied)
	{
		TRACEINTO << "CAVmsgServiceList::SecureSaveIvrListToFile - FileName:" << FILE_IVR_CONFIG_XML << ", Failed, Cannot copy a file";
	}

	return status;
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::SetAsDefaultIfNeeded(const CAVmsgService* pAVmsgService)
{
	if (!pAVmsgService)
		return;

	// Set default services if not set yet (EQ & IVR)
	const char*        ivrServiceName = pAVmsgService->GetName();
	const CIVRService* pIVRService    = pAVmsgService->GetIVRService();
	if (!pIVRService)
		return;

	DWORD isEQ = pIVRService->GetEntryQueueService();

	// Check if it is an IVR service (0) or an EQ service (1)
	if (isEQ && (0 == strcmp(GetDefaultEQName(), "\0")))
		SetDefaultEQName(ivrServiceName);
	else if (!isEQ && (0 == strcmp(GetDefaultIVRName(), "\0")))
		SetDefaultIVRName(ivrServiceName);
}

//--------------------------------------------------------------------------
int CAVmsgServiceList::CheckIvrListValidity()
{
	int          numOfConfIvrServices = 0;
	int          numOfEqIvrServices   = 0;
	int          status               = STATUS_OK;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	BOOL isNinjaGesher = (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType) ? TRUE : FALSE;

	// Get the first IVR Service
	CAVmsgService* pAVmsgService = GetFirstService();

	for (int i = 1; i <= m_numb_of_Serv; i++)
	{
		if (NULL == pAVmsgService)
		{
			TRACEINTO << "Index: " << i << ", end of list or Failed, Invalid pAVmsgService";
			break;
		}

		string err = " ";
		WORD   chkLevel = 1; // 1: check media existence
		status = pAVmsgService->IsLegalService(chkLevel, err);

		const CIVRService* pIVRService = pAVmsgService->GetIVRService();
		if (NULL == pIVRService)
		{
			TRACEINTO << "IVR-Error - Index: " << i << ", Failed, Invalid pIVRService";
			continue;
		}

		if (STATUS_OK == status)
		{
			if (pIVRService->GetEntryQueueService())
				numOfEqIvrServices++;
			else
				numOfConfIvrServices++;

//			if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily() && !isNinjaGesher) // commented those lines as we support Welcome! (BRIDGE-6587)
//			{
//				pIVRService->m_pWelcome->m_enable_disable = NO;
//			}

			// Get the next IVR service in list
			pAVmsgService = GetNextService();
		}
		else
		{
			TRACEINTO << "CAVmsgServiceList::CheckIvrListValidity - Failed, an illegal IVR service was found, error:" << err.c_str();
			PASSERT_AND_RETURN_VALUE(1, STATUS_FAIL);
		}
	}

	if ((numOfEqIvrServices < 1) || (numOfConfIvrServices < 1))
	{
		PASSERTMSG_AND_RETURN_VALUE(2, "CAVmsgServiceList::CheckIvrListValidity - Failed, default IVR service is missing", STATUS_FAIL);
	}

	return status;	// D.K.VNGR-22437 - Always return STATUS_OK on start-up to avoid deleting IVR-Services, Profiles and Templates because of some invalid IVR
}

//--------------------------------------------------------------------------
BOOL CAVmsgServiceList::IsIdenticalToCurrent(CAVmsgServiceList* other)
{
	if (m_numb_of_Serv != other->m_numb_of_Serv)
		return FALSE;

	for (int i = 0; i < m_numb_of_Serv; i++)
	{
		if (FALSE == m_pAVmsgService[i]->IsIdenticalToCurrent(other->m_pAVmsgService[i]))
			return FALSE;
	}

	if (m_numb_of_IVR_Serv != other->m_numb_of_IVR_Serv)
		return FALSE;

	if (0 != strncmp(m_defaultIVRServiceName, other->m_defaultIVRServiceName, AV_MSG_SERVICE_NAME_LEN))
		return FALSE;

	if (0 != strncmp(m_defaultEQServiceName, other->m_defaultEQServiceName, AV_MSG_SERVICE_NAME_LEN))
		return FALSE;

	return TRUE;
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::DeleteServices()
{
	for (int i = 0; i < MAX_IVR_SERV_IN_LIST; i++)
		PDELETE(m_pAVmsgService[i]);

	m_numb_of_Serv             = 0;
	m_numb_of_IVR_Serv         = 0;
	m_ind_serv                 = 0;
	m_defaultIVRServiceName[0] = '\0';
	m_defaultEQServiceName[0]  = '\0';
	m_bIVRServiceListFlag      = FALSE;

	IncreaseUpdateCounter();
}

//--------------------------------------------------------------------------////////////////////////
int CAVmsgServiceList::AuditUpdateServiceIfNeeded(const CAVmsgService& serviceOld, const CAVmsgService& serviceNew) const
{
	const CIVRService* pIvrServiceOld = serviceOld.GetIVRService();
	const CIVRService* pIvrServiceNew = serviceNew.GetIVRService();

	if (pIvrServiceOld == NULL || pIvrServiceNew == NULL)
		return STATUS_OK;

	if (FALSE == pIvrServiceOld->GetConfPasswordFeature()->IsIdenticalToCurrent(pIvrServiceNew->GetConfPasswordFeature()))
		SendAuditEvent("IVR service was changed.", "Conference password settings are changed");

	if (FALSE == pIvrServiceOld->GetRollCallFeature()->IsIdenticalToCurrent(pIvrServiceNew->GetRollCallFeature()))
		SendAuditEvent("IVR service was changed.", "Roll Call settings are changed");

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CAVmsgServiceList::SendAuditEvent(const std::string strEvent, const std::string strDescription) const
{
	// Audit
	AUDIT_EVENT_HEADER_S outAuditHdr;
	CAuditorApi::PrepareAuditHeader(outAuditHdr,
	                                "",
	                                eMcms,
	                                "",
	                                "",
	                                eAuditEventTypeInternal,
	                                eAuditEventStatusOk,
	                                strEvent,
	                                strDescription,
	                                "",
	                                "");
	CFreeData freeData;
	CAuditorApi::PrepareFreeData(freeData,
	                             "",
	                             eFreeDataTypeXml,
	                             "",
	                             "",
	                             eFreeDataTypeXml,
	                             "");
	CAuditorApi api;
	api.SendEventMcms(outAuditHdr, freeData);
}
