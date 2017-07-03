// ReceptionH323.h: API of CReceptionH323 class.


//////////////////////////////////////////////////////////////////////
//Revisions and Updates: 
//
//Date         Updated By         Description
//
//17/7/05		Yoella				Porting to Carmel
//========   ==============   =====================================================================


#if !defined(_ReceptionH323_H__)
#define _ReceptionH323_H__

#include "Reception.h"
#include "CommConf.h"

class CH323NetSetup;

class CReceptionH323 : public CReception
{
CLASS_TYPE_1(CReceptionH323,CReception )	
	public:
						// Constructors
	CReceptionH323(CTaskApp *pOwnerTask);    
	virtual ~CReceptionH323();  
	virtual const char* NameOf() const { return "CReceptionH323";}
	
						// Initializations  
	virtual void  CreateH323(DWORD monitorConfId,DWORD monitorPartyId,COsQueue* pConfRcvMbx,
                          COsQueue* pLobbyRcvMbx, /*CARMELCRsrcTbl* pRsrcTbl,*/CH323NetSetup* pH323NetSetUp,
						  CLobby* pLobby, char* pPartyName, const char* confName, CConfParty* pConfParty,
						  CCommConf* pComConf/*, WORD isChairEnabled,BYTE IsGateWay*/);    
                          
	virtual void  CreateH323(CH323NetSetup* pH323NetSetUp,COsQueue* pLobbyRcvMbx, CLobby* pLobby, int reason,
						char* alternativeAlias = NULL);
	
	virtual void  CreateSuspendConfExist(CH323NetSetup* pNetSetup,COsQueue* pLobbyRcvMbx,
                         CLobby* pLobby,CTaskApp* ListId,DWORD ConfId,char* confName);
	virtual void  CreateSuspend(CH323NetSetup* pNetSetup,COsQueue* pLobbyRcvMbx,
                         CLobby* pLobby,CTaskApp* ListId,DWORD ConfId,char* confName);
	void  GetH323CallParams(CH323NetSetup** pNetSetup);
									   
	
	virtual void  OnLobbyReleasePartySuspend(CSegment* pParam);
	//CARMEL
	virtual void  CreateMeetingRoom(CH323NetSetup* pNetSetup,/* CIpRsrcDesc *pIpDesc,*/COsQueue* pLobbyRcvMbx,
								       CLobby* pLobby,CTaskApp* ListId,DWORD ConfId,char* confName);
	virtual void  CreateGateWayConf(CH323NetSetup* pNetSetup, /*CIpRsrcDesc *pIpDesc,*/COsQueue* pLobbyRcvMbx,
										CLobby* pLobby, CTaskApp* ListId, DWORD ConfId,char* confName, char* targetNumber = NULL);
	void  OnTimerPartySuspend(CSegment* pParam);
	// Operations        
	virtual void*  GetMessageMap();                             
	void GetCallParams(CNetSetup** pNetSetUp/*,CNetConnRsrcDesc** pNetDesc*/);
	virtual BOOL IsPartyDefined(DWORD confMonitorId);
	CH323NetSetup* GetpH323NetSetUp();
	virtual void    AcceptParty(CNetSetup* pNetSetUp,CCommConf* pCommConf,CConfParty* pConfParty);
	
	protected:

	// Operations
	virtual void  OnException();            
	WORD  GetVideoSession(CCommConf* pCommConf);
	
	PDECLAR_MESSAGE_MAP  

};


#endif // !defined(_ReceptionH323_H__)
