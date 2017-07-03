//+========================================================================+
//                   EpSimEndpointsList.h						           |
//				Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimEndpointsList.h										   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                        |
//+========================================================================+

#ifndef __EPSIMENDPOINTSLIST_H__
#define __EPSIMENDPOINTSLIST_H__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
#include "OsQueue.h"
#include "BoardDetails.h"
#include "Endpoint.h"
#include "H323CsReq.h"

////////////////////////////////////////////////////////////////////////////
//  DECLARATIONS
//
class CMplMcmsProtocol;
class CCapSet;
class CCapSetsList;
class CH323Behavior;
class CH323BehaviorList;

////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
//
#define MAX_H323_EPS	4000


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CEndpointsList : public CPObject
{
	CLASS_TYPE_1(CEndpointsList,CPObject)

public:
	CEndpointsList( CTaskApi *pCSApi,CCapSetsList* pCapList,CH323BehaviorList* pBehavList);
	virtual ~CEndpointsList();
	virtual const char* NameOf() const { return "CEndpointsList";}


		// GUI interface functions
	void AddEpFromGui(CSegment* pParam);
	void AddEpRangeFromGui(CSegment* pParam);
	void GetFullEpListToGui(CSegment* pParam);
	void GetFullEpListToTerminal();
	void DeleteEpFromGui(CSegment* pParam);
	void UpdateEpFromGui(CSegment* pParam);
/*for test only*/ void		playSoundReqReq(CSegment* pParam);

	void HandleDtmf(CSegment* pParam);
	void HandleConnect(CSegment* pParam);
	void HandleDisconnect(CSegment* pParam);
	void HandleGuiEpMessage(const DWORD opcode,CSegment* pParam);

	void HandleBatchEvent(const DWORD opcode, CSegment* pParam);
	void HandleCsEvent(CMplMcmsProtocol* pMplProtocol);
	void HandleArtGideonMsg(const DWORD opcode, CSegment* pParam);
	void HandleRtmGideonMsg(const DWORD opcode, CSegment* pParam);
	void HandleMRMGideonMsg(const DWORD opcode, CSegment* pParam);

	static STATUS ParseIpAddress(const char* pszIpAddress, BYTE* bytes);
	static STATUS IpAddress2Dword(const char* pszIpAddress, DWORD& dwIp);
	static char*  GetProtocolTypeAsString(const eEndpointsTypes type);
	static bool CheckCascade( mcReqCallSetup* pSetup );

protected:

		// utilities
	CEndpoint*	CreateDialOutParty(const eEndpointsTypes type,
				const DWORD nConfID, const DWORD nPartyID, const DWORD nConnectionID,
				const WORD wCsHandle, const WORD wCsDestUnit,
				const WORD boardId,const WORD subBoardId,const WORD spanId,const WORD portId, bool isCascade=false);
	DWORD GenerateEndpointID() const;
	void  GenerateEndpointIp(char* pszIp, DWORD ipVer) const;
	friend class CEndpointLinker;
	static DWORD GenerateCallIndex();

	CEndpoint*	FindParty(const DWORD nId);
	CEndpoint*	FindParty(const char* pszName);
	CEndpoint* 	FindPartyByConfParty(const DWORD nConfId, const DWORD nPartyId);
	CEndpoint* 	FindPartyNumberXinConf(const DWORD nConfId, const DWORD partyNo);
	CEndpoint*	FindPartyByCallIndex(const DWORD nCsCallIndex);
	CEndpoint*	FindByCallAnswerReq(const DWORD opcode, BYTE* pContentData);

	STATUS		AddEndpoint(CEndpoint* pEp, char* pStatusStr);
//	STATUS		DeleteEndpoint(const DWORD nId);
//	STATUS		DeleteEndpoint(const char* pszName);
	DWORD		GetEpListLength();
	int			GetNextEmptyPlace();

	void		DumpAudioBoardsWaitingList();
	void		DumpEndpoints();

protected:
		// data members
	DWORD				m_updateCounter;
//	CTaskApi*			m_pCSApi;
	CEndpoint*			m_paEpArray[MAX_H323_EPS];
	CPartyBoardDetails	m_raAudioBoards[MAX_H323_EPS]; // waiting list
	CCapSetsList*		m_pCapList;  // only pointer, list is in Task
	CH323BehaviorList*	m_pBehaviorList;  // only pointer, list is in Task

	// tx mailbox of last GUI request
	COsQueue			m_guiTxMbx;
};

#endif // __EPSIMENDPOINTSLIST_H__ 





