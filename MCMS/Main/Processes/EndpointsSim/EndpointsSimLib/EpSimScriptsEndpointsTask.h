//+========================================================================+
//                  EpSimScriptsEndpointsTask.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimScriptsEndpointsTask.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef __EPSIMSCRIPTSENDPOINTSTASK_H__
#define __EPSIMSCRIPTSENDPOINTSTASK_H__

#include <string>
#include "SerializeObject.h"
#include "TaskApp.h"


using namespace std;


class CXMLDOMElement;

#define	MAX_SCRIPT_ACTIONS	20
#define MAX_DTMF_SCRIPT		16
#define MAX_SCRIPTS_IN_SIM	50
#define	MAX_SCRIPT_NAME		250

#define SCRIPT_PAUSE		0
#define SCRIPT_DTMF			1
#define SCRIPT_CONNECT		2
#define SCRIPT_DISCONNECT	3
#define SCRIPT_HOLD     	4
#define SCRIPT_RESUME   	5


///     EVENT OPCODES

const WORD	ADD_SCRIPT_REQ			= 10219;
const WORD  GET_SCRIPT_LIST_REQ		= 10221;
const WORD	START_SCRIPT_REQ		= 10222;
const WORD	HOLD_SCRIPT_REQ			= 10223;
const WORD	RESUME_SCRIPT_REQ		= 10224;
const WORD	DELETE_SCRIPT_REQ		= 10225;
const WORD	HOLD_ALL_SCRIPT_REQ		= 10226;
const WORD	RESUME_ALL_SCRIPT_REQ	= 10227;





/////////////////////////////////////////////////////////////////////////////
struct CBaseScriptAction
{
	WORD m_type;					// Pause / DTMF / Call / Disconnect
	WORD m_timeDuration;			// time of the for the type in case Pause
	WORD m_timeBetweenDTMF;			// time between DTMF command (0 means all in one command)
	char m_Dtmf[MAX_DTMF_SCRIPT];	// DTMF to send
};


/////////////////////////////////////////////////////////////////////////////
class CRunScript  : public CStateMachine 
{
public:
	CRunScript() {}
	~CRunScript() {}
	void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	void  Start();
	virtual const char* NameOf() const { return "CRunScript";}


public:
	CBaseScriptAction	m_list[MAX_SCRIPT_ACTIONS];
	int m_timeElaps;
	int m_curCycle;
	int m_numOfCycles;
};


/////////////////////////////////////////////////////////////////////////////
class CScriptSet
{
public:
	CScriptSet() ;
	~CScriptSet() {}
	CScriptSet (const CScriptSet &other);
	CScriptSet& operator = (const CScriptSet& rOther);

	void SerializeXml  (CXMLDOMElement* pFatherNode) const;
	int DeSerializeXml(CXMLDOMElement* pFatherNode,char *pszError) ;
	void DeSerialize(CSegment *pParam);
	void Serialize(CSegment *pParam);
	string GetName() {return m_scriptName;}
	int IsRun();
	void StartRun();
	void StopRun();
	void HoldRun();

public:
//	char			m_scriptNameChar[MAX_SCRIPT_NAME];
	string			m_scriptName;
//	char			m_scriptDescriptionChar[MAX_SCRIPT_NAME];
	string			m_scriptDescription;

	string			m_IPAddress1;
	string			m_IPAddress2;
	string			PhoneNum1;
	string			PhoneNum2;
	DWORD			m_time_Between_Scripts_Start; 
	DWORD			m_durationChange;
	DWORD			m_timeMsBetweenConnections;	// ms
	WORD			m_dtmfIincrement; // To check if inceremnt is needed 
	WORD			m_IncrementDtmfEvery;
	WORD			m_AddTimeBetweenScriptsToDuration;
	WORD			m_numOfCycles;
	string			m_capabilityName;		
	WORD			m_actionsNum;
	CBaseScriptAction	m_list[MAX_SCRIPT_ACTIONS];
	
	CRunScript*		m_runManager;				// run manager
};


/////////////////////////////////////////////////////////////////////////////
class CSimScriptsEndpointsModule  : public CTaskApp,  public CSerializeObject
{
CLASS_TYPE_1(CSimScriptsEndpointsModule,CTaskApp )
public:
			// Constructors
	CSimScriptsEndpointsModule();
	virtual ~CSimScriptsEndpointsModule();
	virtual const char* NameOf() const { return "CSimScriptsEndpointsModule";}

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations

	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual void  SerializeXml(CXMLDOMElement* pFatherNode) const;
	void  SerializeXml(CXMLDOMElement* pFatherNode,DWORD ObjToken, const BYTE isIvr) const;

	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	// Pure virtual method from CSerializeObject 
	virtual CSerializeObject* Clone();

	void StartRun( string scriptName );
	void StopRun( string scriptName );
	void HoldRun( string scriptName );
	void HoldAllScripts();
	void ResumeAllScripts();

	void GetScriptsName( CSegment *pParam );
	void GetScriptsRunningName( CSegment *pParam );
	void GetScriptRunningDetails( CSegment *pParam );


protected:
			// Action functions
	void OnGUIMsgAll(CSegment* pParam);
	

protected:
			// Utilities
	void  InitTask(){;}
	BOOL  IsSingleton() const {return NO;}
	const char* GetTaskName() const;
	void SendBufferToGui( DWORD opcode, BYTE* buffer, WORD bufferLen );
	void SendCSegmentToGui( DWORD opcode, CSegment *pMsg );

	int	SerializeXML();
	void AddScript( CSegment* pParam );
	void DelScript( CSegment* pParam );
	void UpdateScript(CSegment* pParam);
	void SaveScriptsToDisk();
	void RestoreScriptsFromDisk();
	int  FindScript( string scriptName );
	void DeleteScript( int ind );

	void GetFullScriptListToGui(CSegment* pParam);
	

	
protected:
			// Attributes
	CTaskApi			*m_pMngrApi;
	CScriptSet			*m_list[MAX_SCRIPTS_IN_SIM];
	WORD				m_numOfScripts;


	PDECLAR_MESSAGE_MAP
};


#endif // __EPSIMSCRIPTSENDPOINTSTASK_H__ 
