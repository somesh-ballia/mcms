//+========================================================================+
//                  EpSimScriptsEndpointsTask.cpp                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimScriptsEndpointsTask.cpp                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                      |
//+========================================================================+

#include <stdlib.h>
#include "Macros.h"
#include "Trace.h"
#include "IpCsOpcodes.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "TaskApi.h"
#include "MplMcmsProtocol.h"
#include "SimApi.h"
#include "EndpointsSim.h"
#include "EpGuiRxSocket.h"
#include "EpGuiTxSocket.h"
#include "EpSimScriptsEndpointsTask.h"
#include "ApiStatuses.h"
#include "OpcodesMcmsInternal.h"

/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  SETUP         = 1;
const WORD  CONNECT       = 2;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class



/////////////////////////////////////////////////////////////////////////////
//
//   EXTERNALS:
//
/////////////////////////////////////////////////////////////////////////////

//  task creation function
extern "C" void epSimScriptsEntryPoint(void* appParam);

using namespace std;

//#define _0_TO_NEW_FILE_NAME_LENGTH 256

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//   SimScriptsEndpointsModule - Scripts module Task
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CRunScript::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CRunScript::Start()
{
	// start running this script
}








/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//   SimScriptsEndpointsModule - Scripts module Task
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimScriptsEndpointsModule)
	// Manager: connect card
	ONEVENT( GUI_TO_SCRIPTS	,	IDLE,   CSimScriptsEndpointsModule::OnGUIMsgAll)

PEND_MESSAGE_MAP(CSimScriptsEndpointsModule,CTaskApp);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void epSimScriptsEntryPoint(void* appParam)
{
	CSimScriptsEndpointsModule*  pEpSimScriptsTask = new CSimScriptsEndpointsModule;
	pEpSimScriptsTask->Create(*(CSegment*)appParam);
}


/////////////////////////////////////////////////////////////////////////////
CSimScriptsEndpointsModule::CSimScriptsEndpointsModule()      // constructor
{
	m_pMngrApi = new CTaskApi;
	int i=0;
	for (i = 0; i < MAX_SCRIPTS_IN_SIM; i++)
		m_list[i] = NULL;
	m_numOfScripts = 0;

}

/////////////////////////////////////////////////////////////////////////////
CSimScriptsEndpointsModule::~CSimScriptsEndpointsModule()     // destructor
{
	FPTRACE( eLevelInfoNormal, "CSimScriptsEndpointsModule::~CSimScriptsEndpointsModule" );
	POBJDELETE (m_pMngrApi);
}


/////////////////////////////////////////////////////////////////////////////
void* CSimScriptsEndpointsModule::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam);

	// get the Mngr API
	m_pMngrApi->CreateOnlyApi(*m_pCreatorRcvMbx);

	// tests for XML serialize
	//	test:  AddScript( NULL );

	// restore scripts from hard-disk
	RestoreScriptsFromDisk();
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimScriptsEndpointsModule::GetTaskName() const
{
	return "SimScriptsEndpointsModule";
}


/////////////////////////////////////////////////////////////////////////////
// App Manager: establish connection with Gideon
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::OnGUIMsgAll(CSegment* pParam)
{
	FPTRACE(eLevelInfoNormal,"CSimScriptsEndpointsModule::OnGUIMsgAll");

	DWORD opcode = 0;
	*pParam >> opcode;
	char tmpchar[250];

	switch( opcode )
	{
	case ADD_SCRIPT_REQ:
		{
			AddScript( pParam );
			break;
		}

	case GET_SCRIPT_LIST_REQ:
		{
			GetFullScriptListToGui(pParam);
			break;
		}
	case START_SCRIPT_REQ:
		{
			*pParam >> tmpchar;
			break;
		}
	case HOLD_SCRIPT_REQ:
		{
			*pParam >> tmpchar;
			break;
		}
	case RESUME_SCRIPT_REQ:
		{
			*pParam >> tmpchar;
			break;
		}
	case DELETE_SCRIPT_REQ:
		{
			DelScript( pParam );
			break;
		}
	case HOLD_ALL_SCRIPT_REQ:
		{
			HoldAllScripts();
			break;
		}
	case RESUME_ALL_SCRIPT_REQ:
		{
			ResumeAllScripts();
			break;
		}
	default:
		{
			break;
		}
	}
}



/////////////////////////////////////////////////////////////////////////////
// App Manager: Message from CS-API


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::SendBufferToGui(DWORD opcode, BYTE* buffer, WORD bufferLen )
{
	CSegment *pMsg = new CSegment;
	*pMsg << (DWORD)opcode;
	*pMsg << (WORD)bufferLen;
	pMsg->Put( (BYTE*)buffer, bufferLen );
	m_pMngrApi->SendMsg(pMsg,SCRIPTS_TO_GUI);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::SendCSegmentToGui(DWORD opcode, CSegment *pMsg )
{
	m_pMngrApi->SendMsg(pMsg,opcode);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::RestoreScriptsFromDisk()
{
	FPTRACE(eLevelInfoNormal,"CSimScriptsEndpointsModule::RestoreScriptsFromDisk");

	std::string ScriptsFile = "Cfg/SimulationScripts.xml";
	ReadXmlFile(ScriptsFile.c_str());
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::AddScript(CSegment* pParam)
{
	FPTRACE(eLevelInfoNormal,"CSimScriptsEndpointsModule::AddScript");

	// add script to list
	if (m_numOfScripts >= MAX_SCRIPTS_IN_SIM)
	{
		FPTRACE(eLevelError,"CSimScriptsEndpointsModule::AddScript - Number of scripts overflow ");
		return;
	}

	// adding a new script
	if (m_list[m_numOfScripts])
		delete m_list[m_numOfScripts];
	m_list[m_numOfScripts] = new CScriptSet;
	m_list[m_numOfScripts]->DeSerialize( pParam );
	m_numOfScripts++;

	// save the new list
	SaveScriptsToDisk();

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::DelScript(CSegment* pParam)
{
	FPTRACE(eLevelInfoNormal,"CSimScriptsEndpointsModule::DelScript");

	char tempName[256];
	*pParam >> tempName;
	string scriptName = tempName;

	int ind = FindScript( scriptName );
	if (ind == -1)
	{
		FPTRACE(eLevelError,"CSimScriptsEndpointsModule::DelScript - Script not found");
		return;
	}

	// delete the script
	DeleteScript( ind );

	// save the new list
	SaveScriptsToDisk();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::UpdateScript(CSegment* pParam)
{
	FPTRACE(eLevelInfoNormal,"CSimScriptsEndpointsModule::UpdateScript");

	CScriptSet tempScript;
	tempScript.DeSerialize( pParam );
	string scriptName = tempScript.GetName();
	if (scriptName.empty() == TRUE) {
		FPTRACE(eLevelError,"CSimScriptsEndpointsModule::UpdateScript - Empty Script Name");
		return;
	}
	int ind = FindScript( scriptName );
	if (ind == -1)
	{
		FPTRACE2(eLevelError,"CSimScriptsEndpointsModule::UpdateScript - Script not found", scriptName.c_str() );
		return;
	}

	// check if the script is running now
	if (m_list[ind]->IsRun()) {
		FPTRACE2(eLevelError,"CSimScriptsEndpointsModule::UpdateScript - Script is Running Now", scriptName.c_str() );
		return;
	}

	// update script
	delete (m_list[ind]);
	m_list[ind] = new CScriptSet;
	*m_list[ind] = tempScript;

	// save to disk
	SaveScriptsToDisk();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::DeleteScript( int ind )
{
	FPTRACE(eLevelInfoNormal,"CSimScriptsEndpointsModule::DeleteScript");
	if (ind < m_numOfScripts)
	{
		if(ind > 0 && ind < MAX_SCRIPTS_IN_SIM)
		{
			delete (m_list[ind]);

			int i=0;
			for (i = ind; i < (m_numOfScripts-1); i++)
				m_list[i] = m_list[i+1];
			m_list[i] = NULL;			
		}
	}

	if (m_numOfScripts > 0)
		m_numOfScripts--;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CSimScriptsEndpointsModule::FindScript(string scriptName )
{
	int i=0;
	for (i = 0; i < m_numOfScripts; i++)
		if (NULL != m_list[i])
			if (m_list[i]->m_scriptName == scriptName)
				return i;

	return -1;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::SaveScriptsToDisk()
{
	if (m_list)
	{
		std::string ScriptsFile = "Cfg/SimulationScripts.xml";
		WriteXmlFile( ScriptsFile.c_str(), "Script_001");
	}
}



void CSimScriptsEndpointsModule::GetFullScriptListToGui(CSegment* pParam)
{
	COsQueue txMbx;
	txMbx.DeSerialize(*pParam);

	CSegment* pMsgSeg = new CSegment;
	*pMsgSeg	<< GUI_TO_ENDPOINTS
				<< GET_SCRIPT_LIST_REQ
				<< (DWORD)STATUS_OK;

	*pMsgSeg	<< m_numOfScripts;

	for (int i = 0; i < MAX_SCRIPTS_IN_SIM; i++)
	{
		if (m_list[i] != NULL)
		{
			//*pMsgSeg	<< tempEpSize;
			m_list[i]->Serialize(pMsgSeg);
		}
	}

	CTaskApi api;
	api.CreateOnlyApi(txMbx);
	api.SendMsg(pMsgSeg,SOCKET_WRITE);
	api.DestroyOnlyApi();
}





/////////////////////////////////////////////////////////////////////////////
CSerializeObject* CSimScriptsEndpointsModule::Clone()
{
	// TBD
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::SerializeXml(CXMLDOMElement* pFatherNode) const
{
	CXMLDOMElement* pScriptsListNode = pFatherNode->AddChildNode("SIM_SCRIPTS_LIST");

	CXMLDOMElement* pScriptNode = NULL;
	pScriptNode = pScriptsListNode->AddChildNode("SIM_SCRIPTS_GEN");
	pScriptNode->AddChildNode("NUM_OF_SCRIPTS",m_numOfScripts);


	int i=0;
	for( i=0; i<m_numOfScripts; i++ )
	{
			// create root for script
		CXMLDOMElement* pScriptNode = NULL;
		pScriptNode = pScriptsListNode->AddChildNode("SIM_SCRIPT");

		if (m_list[i] != NULL)
			m_list[i]->SerializeXml(pScriptNode);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::SerializeXml(CXMLDOMElement* pFatherNode,DWORD ObjToken, const BYTE isIvr) const
{
}

void CSimScriptsEndpointsModule::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

/////////////////////////////////////////////////////////////////////////////
int  CSimScriptsEndpointsModule::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	if (!pActionNode)
		return STATUS_FAIL;

	int nStatus = STATUS_OK;

	// gets the main list
	CXMLDOMElement *pScriptsListNode = NULL;
	GET_MANDATORY_CHILD_NODE(pActionNode, "SIM_SCRIPTS_LIST", pScriptsListNode)
	if (!pScriptsListNode)
		return STATUS_FAIL;


	// get number of scripts
	CXMLDOMElement *pScriptsGenChild = NULL;
	GET_MANDATORY_CHILD_NODE(pScriptsListNode, "SIM_SCRIPTS_GEN", pScriptsGenChild)
	if (!pScriptsGenChild)
		return STATUS_FAIL;
	m_numOfScripts = 0;
	GET_VALIDATE_MANDATORY_CHILD(pScriptsGenChild,"NUM_OF_SCRIPTS",&m_numOfScripts,_0_TO_WORD);
	if (m_numOfScripts > MAX_SCRIPTS_IN_SIM)
		return STATUS_FAIL;

	// getting the whole scripts
	int i = 0;
	for (i = 0; i < m_numOfScripts; i++)
	{

		// gets root for script
		CXMLDOMElement* pScriptNode = NULL;

		if (0 == i) {
			GET_FIRST_MANDATORY_CHILD_NODE(pScriptsListNode, "SIM_SCRIPT", pScriptNode);
		}
		else {
			GET_NEXT_CHILD_NODE(pScriptsListNode, "SIM_SCRIPT", pScriptNode);
		}

		if (!pScriptNode)
			return STATUS_FAIL;

		if (m_list[i] != NULL)
			delete m_list[i];
		m_list[i] = new CScriptSet;
		m_list[i]->DeSerializeXml( pScriptNode, pszError );
	}

	return nStatus;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CSimScriptsEndpointsModule::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	return nStatus;
}

void CSimScriptsEndpointsModule::StartRun( string scriptName )
{
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::StopRun( string scriptName )
{
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::HoldRun( string scriptName )
{
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::HoldAllScripts()
{
	//RNS
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimScriptsEndpointsModule::ResumeAllScripts()
{
	//RNS
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//
//			   CScriptSet
//
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CScriptSet::CScriptSet()
{
	m_numOfCycles = 1;
	m_timeMsBetweenConnections = 100;	// ms
	m_runManager = NULL;
	m_scriptName = "";
	m_IPAddress1 = "";
	m_IPAddress2 = "";
	PhoneNum1 = "";
	PhoneNum2 = "";
	m_actionsNum = 0;

	m_time_Between_Scripts_Start = 100;
	m_durationChange = 0;
	m_dtmfIincrement = 0;
	m_IncrementDtmfEvery = 0;
	m_AddTimeBetweenScriptsToDuration = 100;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CScriptSet::CScriptSet (const CScriptSet &other)
{
	m_runManager				= NULL;
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CScriptSet& CScriptSet::operator = (const CScriptSet& other)
{
	if( this == &other )
		return *this;

	m_scriptName				= other.m_scriptName;
	m_IPAddress1				= other.m_IPAddress1;
	m_IPAddress2				= other.m_IPAddress2;
	PhoneNum1					= other.PhoneNum1;
	PhoneNum2					= other.PhoneNum2;
	m_numOfCycles				= other.m_numOfCycles;
	m_timeMsBetweenConnections	= other.m_timeMsBetweenConnections;
	m_runManager				= NULL;
	m_actionsNum				= other.m_actionsNum;
	m_time_Between_Scripts_Start = other.m_time_Between_Scripts_Start;
	m_durationChange 			= other.m_durationChange;
	m_dtmfIincrement 			= other.m_dtmfIincrement;
	m_IncrementDtmfEvery 		= other.m_IncrementDtmfEvery;
	m_AddTimeBetweenScriptsToDuration = other.m_AddTimeBetweenScriptsToDuration;

	if (m_actionsNum > MAX_SCRIPT_ACTIONS)
		m_actionsNum = MAX_SCRIPT_ACTIONS;
	int i=0;
	for (i = 0; i < m_actionsNum; i++)
	{
		m_list[i].m_type			= other.m_list[i].m_type;
		m_list[i].m_timeDuration	= other.m_list[i].m_timeDuration;
		m_list[i].m_timeBetweenDTMF = other.m_list[i].m_timeBetweenDTMF;
		int len = strlen( other.m_list[i].m_Dtmf );
		if (len > 0)
		{
			strncpy( m_list[i].m_Dtmf, other.m_list[i].m_Dtmf, MAX_DTMF_SCRIPT);
			m_list[i].m_Dtmf[MAX_DTMF_SCRIPT-1] = '\0';
		}
		else
			m_list[i].m_Dtmf[0] = '\0';
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CScriptSet::SerializeXml(CXMLDOMElement* pScriptNode) const
{

	// inserting the general script parameters
	/*
	pScriptNode->AddChildNode("SCRIPT_NAME",m_scriptName.c_str());
	pScriptNode->AddChildNode("IP_First",m_IPAddress1.c_str());
	pScriptNode->AddChildNode("IP_Last",m_IPAddress2.c_str());
	pScriptNode->AddChildNode("Phone_First",PhoneNum1.c_str());
	pScriptNode->AddChildNode("Phone_Last",PhoneNum2.c_str());
	*/
	pScriptNode->AddChildNode("SCRIPT_NAME","stam script");
	pScriptNode->AddChildNode("IP_First","172.22.202.100");
	pScriptNode->AddChildNode("IP_Last","172.22.202.200");
	pScriptNode->AddChildNode("Phone_First","123456");
	pScriptNode->AddChildNode("Phone_Last","987678");


	// creating the root for flow
	CXMLDOMElement*  pFlowNode = NULL;
	pFlowNode = pScriptNode->AddChildNode("SIM_SCRIPT_FLOW");
	pFlowNode->AddChildNode("NumberOfActions",(WORD)m_actionsNum);

	// inserting the flow actions
	int i;
	for (i = 0; i < m_actionsNum; i++)
	{
		// creating root for every action
		CXMLDOMElement*  pAction = NULL;
		pAction = pFlowNode->AddChildNode("ACTION");

		// inserting the action parameters
		if (strlen(m_list[i].m_Dtmf))
			pAction->AddChildNode("DTMF",m_list[i].m_Dtmf);
		pAction->AddChildNode("TYPE",m_list[i].m_type);
		pAction->AddChildNode("Duration",m_list[i].m_timeDuration);
		pAction->AddChildNode("DTMF_TIME",m_list[i].m_timeBetweenDTMF);
	}

}

/////////////////////////////////////////////////////////////////////////////
int CScriptSet::DeSerializeXml(CXMLDOMElement* pScriptNode, char *pszError)
{
	if (!pScriptNode)
		return STATUS_FAIL;

	int nStatus = STATUS_OK;

	char tempVal[256];
	tempVal[0] = '\0';
	GET_VALIDATE_MANDATORY_CHILD(pScriptNode,"SCRIPT_NAME",tempVal,_0_TO_NEW_FILE_NAME_LENGTH);
	m_scriptName = tempVal;

	GET_VALIDATE_MANDATORY_CHILD(pScriptNode,"IP_First",tempVal,_0_TO_NEW_FILE_NAME_LENGTH);
	m_IPAddress1 = tempVal;

	GET_VALIDATE_MANDATORY_CHILD(pScriptNode,"IP_Last",tempVal,_0_TO_NEW_FILE_NAME_LENGTH);
	m_IPAddress2 = tempVal;

	GET_VALIDATE_MANDATORY_CHILD(pScriptNode,"Phone_First",tempVal,_0_TO_NEW_FILE_NAME_LENGTH);
	PhoneNum1 = tempVal;

	GET_VALIDATE_MANDATORY_CHILD(pScriptNode,"Phone_Last",tempVal,_0_TO_NEW_FILE_NAME_LENGTH);
	PhoneNum2 = tempVal;

	// gets root for flow
	CXMLDOMElement* pScriptFlowNode = NULL;
	GET_MANDATORY_CHILD_NODE(pScriptNode, "SIM_SCRIPT_FLOW", pScriptFlowNode)
	if (!pScriptFlowNode)
		return STATUS_FAIL;

	// gets number of actions for this script
	GET_VALIDATE_MANDATORY_CHILD(pScriptFlowNode,"NumberOfActions",&m_actionsNum,_0_TO_WORD);
	if (m_actionsNum > MAX_SCRIPT_ACTIONS)
		return STATUS_FAIL;

	// gets actions
	int i=0;
	for ( i = 0; i < m_actionsNum; i++)
	{
		// gets root for flow
		CXMLDOMElement* pActionNode = NULL;
		if (0 == i) {
			GET_FIRST_MANDATORY_CHILD_NODE(pScriptFlowNode, "ACTION", pActionNode);
		}
		else {
			GET_NEXT_CHILD_NODE(pScriptFlowNode, "ACTION", pActionNode);
		}

		if (!pActionNode)
			return STATUS_FAIL;

		// gets action parameters (DTMF, type, Duration, DTMF-Duration)
		char actionStr[256];
		m_list[i].m_Dtmf[0] = '\0';
		GET_VALIDATE_MANDATORY_CHILD( pActionNode,"DTMF", actionStr, _0_TO_NEW_FILE_NAME_LENGTH);
		if (strlen(actionStr) > 0) {
			strncpy( m_list[i].m_Dtmf, actionStr, sizeof(m_list[i].m_Dtmf) - 1);
			m_list[i].m_Dtmf[sizeof(m_list[i].m_Dtmf) - 1] = '\0';
		}

		WORD wAction = 0xFFFF;
		GET_VALIDATE_MANDATORY_CHILD(pActionNode,"TYPE",&wAction,_0_TO_WORD);
		m_list[i].m_type = wAction;

		GET_VALIDATE_MANDATORY_CHILD(pActionNode,"Duration",&wAction,_0_TO_WORD);
		m_list[i].m_timeDuration = wAction;

		GET_VALIDATE_MANDATORY_CHILD(pActionNode,"DTMF_TIME",&wAction,_0_TO_WORD);
		m_list[i].m_timeBetweenDTMF = wAction;
	}

	return nStatus;

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CScriptSet::DeSerialize(CSegment *pParam)
{
	char	tmpCharRead[MAX_SCRIPT_NAME];

	*pParam >> tmpCharRead;
	m_scriptName=tmpCharRead;

	*pParam >> tmpCharRead;
	m_scriptDescription=tmpCharRead;


	*pParam >> tmpCharRead;
	m_IPAddress1=tmpCharRead;

	*pParam >> tmpCharRead;
	m_IPAddress2=tmpCharRead;

	*pParam >> tmpCharRead;
	PhoneNum1=tmpCharRead;

	*pParam >> tmpCharRead;
	PhoneNum2=tmpCharRead;

	*pParam >> m_time_Between_Scripts_Start;

	*pParam >> m_durationChange;

	*pParam >> m_timeMsBetweenConnections;

	*pParam >> m_dtmfIincrement;

	*pParam >> m_IncrementDtmfEvery;

	*pParam >> m_AddTimeBetweenScriptsToDuration;

	*pParam >> m_numOfCycles;

	*pParam >> tmpCharRead;
	m_capabilityName=tmpCharRead;

	*pParam >> m_actionsNum;
	for(int i=0;i<m_actionsNum;i++)
	{
		*pParam >> tmpCharRead;
		switch (tmpCharRead[0])
		{
			case '1':
				{
					m_list[i].m_type=1;
					break;
				}
			case '2':
				{
					m_list[i].m_type=2;
					break;
				}
			case '3':
				{
					m_list[i].m_type=3;
					int temp_char=(int)(tmpCharRead[2]) - 48;
					temp_char = min (temp_char + 1, MAX_DTMF_SCRIPT - 1);
					strncpy(m_list[i].m_Dtmf,tmpCharRead+4,temp_char);
					m_list[i].m_Dtmf[temp_char] = 0;
					break;
				}
			case '4':
				{
					m_list[i].m_type=4;
					char tmpChar[15];
					strncpy(tmpChar, tmpCharRead+2, sizeof(tmpChar)-1);
					tmpChar[sizeof(tmpChar)-1] = '\0';
					m_list[i].m_timeDuration=atoi(tmpChar);
					break;
				}
			case '5':
				{
					m_list[i].m_type=5;
					char tmpChar[15];
          strncpy(tmpChar, tmpCharRead+2, sizeof(tmpChar)-1);
          tmpChar[sizeof(tmpChar)-1] = '\0';
					m_list[i].m_timeBetweenDTMF=atoi(tmpChar);
					break;
				}
		}
	}


}

void CScriptSet::Serialize( CSegment* pParam )
{
	char	tmpCharWrite[MAX_SCRIPT_NAME];

	strncpy(tmpCharWrite, m_scriptName.c_str(), sizeof(tmpCharWrite)-1);
	tmpCharWrite[sizeof(tmpCharWrite)-1] = '\0';
	*pParam << tmpCharWrite;

  strncpy(tmpCharWrite, m_scriptDescription.c_str(), sizeof(tmpCharWrite)-1);
  tmpCharWrite[sizeof(tmpCharWrite)-1] = '\0';
	*pParam << tmpCharWrite;


	//************

  strncpy(tmpCharWrite, m_IPAddress1.c_str(), sizeof(tmpCharWrite)-1);
  tmpCharWrite[sizeof(tmpCharWrite)-1] = '\0';
	*pParam << tmpCharWrite;

  strncpy(tmpCharWrite, m_IPAddress2.c_str(), sizeof(tmpCharWrite)-1);
  tmpCharWrite[sizeof(tmpCharWrite)-1] = '\0';
	*pParam << tmpCharWrite;

  strncpy(tmpCharWrite, PhoneNum1.c_str(), sizeof(tmpCharWrite)-1);
  tmpCharWrite[sizeof(tmpCharWrite)-1] = '\0';
	*pParam << tmpCharWrite;

  strncpy(tmpCharWrite, PhoneNum2.c_str(), sizeof(tmpCharWrite)-1);
  tmpCharWrite[sizeof(tmpCharWrite)-1] = '\0';
	*pParam << tmpCharWrite;

	*pParam << m_time_Between_Scripts_Start;

	*pParam << m_durationChange;

	*pParam << m_timeMsBetweenConnections;

	*pParam << m_dtmfIincrement;

	*pParam << m_IncrementDtmfEvery;

	*pParam << m_AddTimeBetweenScriptsToDuration;

	*pParam << m_numOfCycles;

  strncpy(tmpCharWrite, m_capabilityName.c_str(), sizeof(tmpCharWrite)-1);
  tmpCharWrite[sizeof(tmpCharWrite)-1] = '\0';
	*pParam << tmpCharWrite;

	*pParam << m_actionsNum;
	for(int i=0;i<m_actionsNum;i++)
	{

		switch (m_list[i].m_type)
		{
			case 1:
				{
					*pParam << m_list[i].m_type;
					break;
				}
			case 2:
				{
					*pParam << m_list[i].m_type;
					break;
				}
			case 3:
				{
					*pParam << m_list[i].m_type;
					*pParam << m_list[i].m_Dtmf;
					break;
				}
			case 4:
				{
					*pParam << m_list[i].m_type;
					*pParam << m_list[i].m_timeDuration;
					break;
				}
			case 5:
				{
					*pParam << m_list[i].m_type;
					*pParam << m_list[i].m_timeBetweenDTMF;
					break;
				}
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CScriptSet::IsRun()
{
	return (m_runManager != NULL);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CScriptSet::StartRun()
{
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CScriptSet::StopRun()
{
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CScriptSet::HoldRun()
{
}



/*
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const = 0;

	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action) = 0;
*/
