// XmlMiniParser.cpp: implementation of the CXmlMiniParser class.
//
//////////////////////////////////////////////////////////////////////

#include "XmlMiniParser.h"
#include "StatusesGeneral.h"
#include "ActionRedirection.h"
#include "psosxml.h"
#include "TraceStream.h"
#include "OsFileIF.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include <map>
#include <vector>
#include <iomanip>

#define MAX_KEY_MAP_SIZE 128

CStringToProcessEntry::CStringToProcessEntry(eProcessType proc, bool isAud, const char* name, const char* desc, const char* faultDesc)
{
	m_processType = proc;
	m_isAudit = isAud;

	m_transName = NULL;
	if (name)
	{
		int len = strlen(name);
		if (len)
		{
			m_transName = new char[len+1];
			strcpy_safe(m_transName, len+1, name);
		}
	}

	m_transDesc = NULL;
	if (desc)
	{
		int len = strlen(desc);
		if (len)
		{
			m_transDesc = new char[len+1];
			strcpy_safe(m_transDesc, len+1, desc);
		}
	}

	m_transFailureDesc = NULL;
	if (faultDesc)
	{
		int len = strlen(faultDesc);
		if (len)
		{
			m_transFailureDesc = new char[len+1];
			strcpy_safe(m_transFailureDesc, len+1, faultDesc);
		}
	}
}

CStringToProcessEntry::~CStringToProcessEntry()
{
	DEALLOCBUFFER(m_transName);
	DEALLOCBUFFER(m_transDesc);
	DEALLOCBUFFER(m_transFailureDesc);
}

CStringToProcessEntry::CStringToProcessEntry(const CStringToProcessEntry &other)
:CPObject(other)
{
	m_processType = other.m_processType;
	m_isAudit = other.m_isAudit;

	m_transName = NULL;
	if (other.m_transName)
	{
		int len = strlen(other.m_transName);
		if (len)
		{
			m_transName = new char[len+1];
			strcpy_safe(m_transName, len+1, other.m_transName);
		}
	}

	m_transDesc = NULL;
	if (other.m_transDesc)
	{
		int len = strlen(other.m_transDesc);
		if (len)
		{
			m_transDesc = new char[len+1];
			strcpy_safe(m_transDesc, len+1, other.m_transDesc);
		}
	}


	m_transFailureDesc = NULL;
	if (other.m_transFailureDesc)
	{
		int len = strlen(other.m_transFailureDesc);
		if (len)
		{
			m_transFailureDesc = new char[len+1];
			strcpy_safe(m_transFailureDesc, len+1, other.m_transFailureDesc);
		}
	}
}

CStringToProcessEntry& CStringToProcessEntry::operator =(const CStringToProcessEntry &other)
{
	if (this != &other)
	{
		CPObject::operator=(other);
		m_processType = other.m_processType;
		m_isAudit = other.m_isAudit;

		DEALLOCBUFFER(m_transName);
		if (other.m_transName)
		{
			int len = strlen(other.m_transName);
			if (len)
			{
				m_transName = new char[len+1];
				strcpy_safe(m_transName, len+1, other.m_transName);
			}
		}

		DEALLOCBUFFER(m_transDesc);
		if (other.m_transDesc)
		{
			int len = strlen(other.m_transDesc);
			if (len)
			{
				m_transDesc = new char[len+1];
				strcpy_safe(m_transDesc, len+1, other.m_transDesc);
			}
		}

		DEALLOCBUFFER(m_transFailureDesc);
		if (other.m_transFailureDesc)
		{
			int len = strlen(other.m_transFailureDesc);
			if (len)
			{
				m_transFailureDesc = new char[len+1];
				strcpy_safe(m_transFailureDesc, len+1, other.m_transFailureDesc);
			}
		}
	}
	return *this;
}

eProcessType CStringToProcessEntry::GetProcessType()
{
	return m_processType;
}

void CStringToProcessEntry::SetProcessType(const eProcessType process_type)
{
	m_processType = process_type;
}

bool CStringToProcessEntry::GetIsAudit()
{
	return m_isAudit;
}

char* CStringToProcessEntry::GetTransName()
{
	if (m_transName==NULL)
		return "";
	return m_transName;
}

char* CStringToProcessEntry::GetTransDesc()
{
	if (m_transDesc==NULL)
		return "";
	return m_transDesc;
}

char* CStringToProcessEntry::GetTransFailureDesc()
{
	if (m_transFailureDesc==NULL)
		return "";
	return m_transFailureDesc;
}

//////////////////////////////////////////////////////////////////////
// CActionRedirectionMap class
//////////////////////////////////////////////////////////////////////

CActionRedirectionMap::CActionRedirectionMap()
{
	m_pMap = new CStringToProcessTypeMap;
}

////////////////////////////////////////////////////////////////////////////////////////////////
CActionRedirectionMap::~CActionRedirectionMap()
{
	MapFree();
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CActionRedirectionMap::MapFree()
{
	CStringToProcessTypeMap::iterator iTer = m_pMap->begin();
	CStringToProcessTypeMap::iterator iEnd = m_pMap->end();
	while(iTer != iEnd)
	{
		PDELETE(iTer->second);

		iTer++;
	}

	PDELETE(m_pMap);
}

////////////////////////////////////////////////////////////////////////////////////////////////
bool CActionRedirectionMap::IsValidTransName(char* pszSearchString, char** pTransName/*, CStringToProcessTypeMap *pActionRedirectionMap*/)
{
	char *pszStartTransName, *pszEndTransName;

	if (pszSearchString == NULL)
		return false;

	pszStartTransName = CXmlMiniParser::GetTransName(pszSearchString, &pszEndTransName);

	if (!pszStartTransName)
	{
		if (strstr(pszSearchString, "<REQUEST_") == NULL)
		{
			FTRACEINTO << "CActionRedirectionMap::IsValidTransName: Transaction name missing. Content: " << pszSearchString;
			return false;
		}
		return false;
	}

	strncpy(*pTransName, pszStartTransName, pszEndTransName - pszStartTransName);
	(*pTransName)[pszEndTransName - pszStartTransName] = 0;

	if (NULL == m_pMap/*pActionRedirectionMap*/)
	{
		FPASSERTMSG(TRUE, "NULL == m_pMap"); //pActionRedirectionMap");
		return false;
	}

	if (m_pMap->find(*pTransName) == m_pMap->end())
	{
		FTRACEINTO << "CActionRedirectionMap::IsValidTransName: Transaction name invalid. Content: " << pszSearchString << "; trans name: " << *pTransName;
		return false;
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CActionRedirectionMap::SerializeXml(CXMLDOMElement*& pXMLRootElement) const
{

}
////////////////////////////////////////////////////////////////////////////////////////////////
int CActionRedirectionMap::DeSerializeXml(CXMLDOMElement* pActionRedirectionMapNode,char* pszError,const char* strAction)
{
	int nStatus=STATUS_OK;
	CXMLDOMElement *pTransactionNode = NULL, *pActionNode = NULL;
	std::string message;
//	int jud_1 = 0;

/*	FILE* fp = NULL;
	fp=fopen((MCU_MCMS_DIR+"/judTemp.txt").c_str(),"a+");
	if (fp!=NULL)
		fputs("\n********Judfile beginning**************\n", fp);*/

	TRACEINTO << "CActionRedirectionMap::DeSerializeXml";

	if (pActionRedirectionMapNode == NULL)
		TRACEINTO << "CActionRedirectionMap::DeSerializeXml - pActionRedirectionMapNode is NULL!!";
	else
	{
		pActionRedirectionMapNode->firstChildNode(&pTransactionNode);
	//	jud_1++;
		if (pTransactionNode == NULL)
		{
			FTRACESTR(eLevelInfoNormal) << "CActionRedirectionMap::DeSerializeXml: Failed parsing Action Redirection configuration file - the file might be empty";
		}
		else
		{
			while (pTransactionNode)
			{
				char* pszTransactionName = NULL;
				pTransactionNode->get_nodeName(&pszTransactionName);

		/*		string jud_str = "no. ";
			    char buffer [32];
			    sprintf(buffer, "%d", jud_1);
			    jud_str += buffer;
				jud_str += " - Judith - pszTransactionName = ";
				jud_str += pszTransactionName;

				fputs("\nBeginning of new transaction\n", fp);
				fputs(jud_str.c_str(), fp);*/
//				FTRACEINTO << "CActionRedirectionMap::DeSerializeXml - pszTransactionName:" << pszTransactionName;

				pTransactionNode->firstChildNode(&pActionNode);
				while (pActionNode)
				{
					if (m_pMap->find(pszTransactionName) == m_pMap->end())
					{
						CStringToProcessEntry* entry = new CStringToProcessEntry(eProcessTypeInvalid, false, "", "", "");
						m_pMap->insert(CStringToProcessTypeMap::value_type(pszTransactionName,entry));
					}

					char *pszActionName = NULL, *pszAuditable = NULL, *pszName = NULL, *pszDescription = NULL, *pszFailureDescription = NULL, *pszProcessName = NULL, szMapKey[MAX_KEY_MAP_SIZE];
					pActionNode->get_nodeName(&pszActionName);

//					FTRACEINTO << "CActionRedirectionMap::DeSerializeXml - pszActionName:" << pszActionName;
					if (pszActionName)
					{
						bool isAuditable = FALSE;
						BYTE isNameExist = STATUS_OK, isDescriptionExist = STATUS_OK;

						pActionNode->getAttribute("auditable", &pszAuditable);
						if (pszAuditable == NULL)
						{
							string errorMessage = "attribute auditable was not found";
							OnMissingAuditAttr(pszActionName, errorMessage.c_str(), "");
						}
						else if (strcmp(pszAuditable, "yes")==0)
						{
							isAuditable = TRUE;
						}
						pActionNode->getAttribute("name", &pszName);
						pActionNode->getAttribute("description", &pszDescription);
						pActionNode->getAttribute("failure_description", &pszFailureDescription);
						if (isAuditable==TRUE && (pszName==NULL || pszDescription==NULL || pszFailureDescription==NULL))
						{
							string errorMessage = "attribute name or description or failure description were not found";
							OnMissingAuditAttr(pszActionName, errorMessage.c_str(), "");
						}
						pActionNode->get_nodeValue(&pszProcessName);

		/*				jud_str = "\nBeginning of new Action - pszActionName = ";
						jud_str += pszActionName;
						jud_str += " and pszProcessName = ";
						jud_str += pszProcessName;

						fputs(jud_str.c_str(), fp);*/

						eProcessType eProcType = ProcNameToProcType(pszProcessName);

						if(eProcType != eProcessTypeInvalid)
						{
							CStringToProcessEntry* entry = new CStringToProcessEntry(eProcType, isAuditable, pszName, pszDescription, pszFailureDescription);
							if ((strlen(pszActionName) + strlen(pszActionName))>MAX_KEY_MAP_SIZE)
							{
								message = "CActionRedirectionMap::DeSerializeXml Action name:";
								message += pszActionName;
								message += " is too Big and together with Transaction name goes over 128 characters!";
								FPASSERTMSG(TRUE, message.c_str());
							}
							if(!strcmp(pszActionName,"DEFAULT"))
								m_pMap->insert(CStringToProcessTypeMap::value_type(pszTransactionName, entry));
							else
							{
								snprintf(szMapKey, sizeof(szMapKey), "%s:%s",pszTransactionName,pszActionName);
								m_pMap->insert(CStringToProcessTypeMap::value_type(szMapKey, entry));
							}
						}
						else
						{
							message = "CActionRedirectionMap::DeSerializeXml Invalid ProccessName:";
							message += pszProcessName;
							FPASSERTMSG(TRUE, message.c_str());
						}
					}
					pTransactionNode->nextChildNode(&pActionNode);
				}
				pActionRedirectionMapNode->nextChildNode(&pTransactionNode);
		//		jud_1++;
			}
		}
	}
//	fclose(fp);

	return nStatus;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CActionRedirectionMap::OnMissingAuditAttr(const char *action, const char *desc, const char *detail)
{
    string message = "ERROR during parsing action-redirection map\n";
    message += action;
    message += " has problem with auditable attribute, check ActionRedirection.xml\n";
    message += desc;
    message += ", ";
    message += detail;

    FPASSERTMSG(TRUE, message.c_str());
}

eProcessType CActionRedirectionMap::ProcNameToProcType(char* pszProcName)
{
	if (!pszProcName)
    return eProcessTypeInvalid;

  for (eProcessType i = eProcessMcmsDaemon; i < NUM_OF_PROCESS_TYPES; ++i)
  {
    if (!strcmp(ProcessTypeToStr(i), pszProcName))
      return i;
  }

  return eProcessTypeInvalid;
}

void CActionRedirectionMap::ClearMap()
{
	m_pMap->clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////
CStringToProcessTypeMap* CActionRedirectionMap::GetMap()
{
	return m_pMap;
}
////////////////////////////////////////////////////////////////////////////////////////////////
bool CActionRedirectionMap::LoadFromFile()
{
	STATUS status = STATUS_FAIL;

	FTRACEINTO << "CActionRedirectionMap::LoadFromFile";
	int size = GetFileSize(ACTION_REDIRECTION_F);

	if( 0 < size )
		status = ReadXmlFile(ACTION_REDIRECTION_F.c_str());

	if (status!=STATUS_OK)
		FTRACESTR(eLevelInfoNormal) << "CActionRedirectionMap::LoadFromFile: Failed parsing Action Redirection configuration file";

	return status == STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CActionRedirectionMap::DumpActionRedirectionMap(std::ostream& ostr)
{
	if (NULL == m_pMap)
	{
		ostr << "NULL == m_pMap";
		return;
	}

	ostr.setf(std::ios::left, std::ios::adjustfield);
	ostr.setf(std::ios::showbase);

	ostr << endl << std::setw(50) << "Transaction" << std::setw(10) << "Auditable" << "Destination Process" << endl << "--------------------------------------------------------------------------------------" << endl;

	for (CStringToProcessTypeMap::const_iterator iTer = m_pMap->begin(); iTer != m_pMap->end(); iTer++)
	{
		const string & transName = iTer->first;
		const char *ptrTransName = strstr(transName.c_str(), ":");
		if (NULL == ptrTransName)
		{
			ptrTransName = transName.c_str();
		}
		else
		{
			ptrTransName++;
		}

		CStringToProcessEntry* currentEntry = iTer->second;

		if (eProcessEndpointsSim == currentEntry->GetProcessType() || eProcessGideonSim == currentEntry->GetProcessType() || eProcessTypeInvalid == currentEntry->GetProcessType())
		{
			continue;
		}

		if (!currentEntry->GetIsAudit())
		{
			continue;
		}

		const char *processName = CProcessBase::GetProcessName(currentEntry->GetProcessType());

		ostr << std::setw(40) << ptrTransName << '\t' << std::setw(10) << (currentEntry->GetIsAudit() ? "YES" : "NO") << currentEntry->GetTransName() << ":" << currentEntry->GetTransDesc() << ":" << currentEntry->GetTransFailureDesc() << endl;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
CStringToProcessEntry CActionRedirectionMap::GetDedicatedManager(char* pszRequest, char *pszStartActionName, char *pszEndActionName, eOtherProcessQueueEntry& eDestTask)
{
	//FTRACESTR(eLevelInfoNormal) << "CActionRedirectionMap::GetDedicatedManager";
    CStringToProcessEntry processEntry(eProcessTypeInvalid, false, "", "", "");

	if(!pszRequest || !pszStartActionName || !pszEndActionName)
		return processEntry;

	char szMapKey[MAX_KEY_MAP_SIZE];
	char *pszStartTransName, *pszEndTransName;

	pszStartTransName = CXmlMiniParser::GetTransName(pszRequest,&pszEndTransName);

	if(!pszStartTransName)
	{
		FTRACESTR(eLevelInfoNormal) << "CActionRedirectionMap::GetDedicatedManager: Transaction Name missing in Message";
		return processEntry;
	}

	memset(szMapKey,'\0', MAX_KEY_MAP_SIZE);

	short nTransNameLen = pszEndTransName-pszStartTransName;
	short nActionNameLen = pszEndActionName-pszStartActionName;
	if(MAX_KEY_MAP_SIZE > nTransNameLen)
	{
		strncpy(szMapKey,pszStartTransName,nTransNameLen );
		szMapKey[nTransNameLen] = 0;
	}

	if(MAX_KEY_MAP_SIZE > nTransNameLen + 1 + nActionNameLen)
	{
		sprintf(szMapKey+nTransNameLen,":");
		strncpy(szMapKey+nTransNameLen+1,pszStartActionName,nActionNameLen);
		szMapKey[nTransNameLen+1+nActionNameLen] = 0;
	}

	CStringToProcessTypeMap::iterator mapIterator;

	if((mapIterator = m_pMap->find(szMapKey)) != m_pMap->end())
	{
/*		CStringToProcessTypeMap::iterator iTer = m_pMap->begin();
		CStringToProcessTypeMap::iterator iEnd = m_pMap->end();
		while(iTer != iEnd)
		{
			FTRACEINTO << "transaction name = " << (iTer->first);
	*/
		CStringToProcessEntry* p2 = mapIterator->second;
		processEntry = *(mapIterator->second);
	}
	else
	{
		FTRACESTR(eLevelInfoNormal) << "CActionRedirectionMap::GetDedicatedManager: Transaction:Action pair missing in Redirection Map: " << szMapKey<<" - Looking only for transaction name";

		szMapKey[nTransNameLen] = 0;

		if((mapIterator = m_pMap->find(szMapKey)) != m_pMap->end())
			processEntry = *(mapIterator->second);
	}

	if(eProcessTypeInvalid != processEntry.GetProcessType())
	{
	    if(strstr(pszStartActionName,"GET_PARTY_PORTS_INFO"))// temp decision
		    eDestTask = eManager;
		else if(strstr(pszStartActionName,"GET") == pszStartActionName)
			eDestTask = eMonitor;
		else
			eDestTask = eManager;
	}
	else
		FTRACESTR(eLevelInfoNormal) << "CActionRedirectionMap::GetDedicatedManager: Transaction:Action pair missing in Redirection Map: " << szMapKey;

	return processEntry;
}
