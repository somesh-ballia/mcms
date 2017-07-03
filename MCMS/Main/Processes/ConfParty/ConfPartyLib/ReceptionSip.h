
#ifndef __RECEPTION_SIP__
#define __RECEPTION_SIP__

#include "Reception.h"
#include "CommConf.h"
#include "SipNetSetup.h"
#include "SipUtils.h"

class CReceptionSip : public CReception
{
CLASS_TYPE_1(CReceptionSip,CReception )
public:
	// Constructors
	CReceptionSip(CTaskApp *pOwnerTask);    
	virtual ~CReceptionSip();  
	
  
	virtual void	CreateSip(DWORD confMonitorId, DWORD partyMonitorId, COsQueue* pConfRcvMbx,
                          COsQueue* pLobbyRcvMbx, CSipNetSetup* pNetSetUp, CLobby* pLobby, char* pPartyName,
						  const char* confName, CConfParty* pConfParty, CCommConf* pComConf, WORD isChairEnabled, BYTE bIsSoftCp,
						  const sipSdpAndHeadersSt* pSdpAndHeaders);    
                          
	virtual void	CreateRejectSip(CSipNetSetup * pNetSetUp, COsQueue* pLobbyRcvMbx, CLobby* pLobby, int reason,
									const sipSdpAndHeadersSt* pSdpAndHeaders, char* pAltAddress=NULL, STATUS status = 0);
	
	virtual void	CreateSuspend(CSipNetSetup *pNetSetUp, const sipSdpAndHeadersSt* pSdpAndHeaders, COsQueue *pLobbyRcvMbx, CLobby *pLobby,
								  CTaskApp *pListId, DWORD confId,char* confName);
	virtual void  	CreateSuspendConfExist(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, 
										   COsQueue* pLobbyRcvMbx, CLobby* pLobby,
	          							   CTaskApp* ListId,DWORD confID,char* confName);
	virtual void	CreateMeetingRoom(CSipNetSetup * pNetSetUp, const sipSdpAndHeadersSt* pSdpAndHeaders, 
     					  COsQueue * pLobbyRcvMbx, CLobby * pLobby,CTaskApp * pListId, DWORD confId,char* confName);
	//								  char* pAdHocConfName, char* pAdHocNID, char* pMultiInviter = NULL, char* pEndPoints = NULL);
	virtual void    CreateGateWayConf(CSipNetSetup *pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, COsQueue *pLobbyRcvMbx,
		       				CLobby *pLobby,CTaskApp *pListId,DWORD gwID,char* gwName,char* dialString);
	void			CreateConfFromFactory(CSipNetSetup * pNetSetUp, COsQueue * pLobbyRcvMbx,
								      CLobby * pLobby, CTaskApp * pListId, const sipSdpAndHeadersSt * pSdpAndHeaders,
									  CCommRes* pFactory);
	void			GetSipCallParams(CSipNetSetup ** ppNetSetup, sipSdpAndHeadersSt **ppSdpAndHeaders);	
	virtual void	OnLobbyReleasePartySuspend(CSegment* pParam);
	void			OnTimerPartySuspend(CSegment* pParam);
	void			OnTimerPartyIdent(CSegment* pParam);
	void			OnTimerPartyTransfer(CSegment* pParam);
	void			AddMultiInvite(DWORD confId, WORD listId, WORD boardId);
	BYTE			IsMultiInvite();
	void			CallMultiInviteParties(DWORD confId);
	char*			GetFactoryCreatedConfName();
	eSipFactoryType	GetFactoryType();
	virtual void    AcceptParty(CNetSetup* pNetSetUp,CCommConf* pCommConf,CConfParty* pConfParty);

	const sipSdpAndHeadersSt*	GetSdpAndHeaders();
	virtual BOOL IsPartyDefined(DWORD confMonitorId);
	
	// Operations        
	virtual void*	GetMessageMap();                             
	virtual const char*	NameOf() const;  
//	DWORD  FindDestIp(CIpNetSetup* pNetSetup);
	mcTransportAddress*	FindDestIp(CSipNetSetup* pNetSetup);
//	void SetMsConversationId(const sipSdpAndHeadersSt* pSdpAndHeaders);
//	void SetMsConversationId(char* msConvId);
	void SetClickToConfId(const sipSdpAndHeadersSt* pSdpAndHeaders);
	void SetClickToConfId(char* ConfId);
//	const char* GetMsConversationId() const;
	const char* GetClickToConfId() const;
protected:

	// Operations
	virtual void	OnException();            
	// ***WORD			GetVideoSession(CComMode* pComMode, CConfDesc* pComConf);
	void			SetSdpAndHeaders(const sipSdpAndHeadersSt& SdpAndHeaders);
	mcTransportAddress*	FindDestIp(const sipSdpAndHeadersSt* pSdpAndHeaders);
	mcTransportAddress* FindDestIp(const char* pDestAddr);

	// Attributes
	sipSdpAndHeadersSt* m_pSdpAndHeaders;
	int				    m_SdpAndHeadersLen;
	char*			    m_endPoints;
	char*			    m_multiInviter;
	char*			    m_factoryCreatedConfName;
	eSipFactoryType		m_factoryType;
//	char				m_msConversationId[MS_CONVERSATION_ID_LEN]; // For suspend use only
	char				m_ClickToConfId[MS_CONVERSATION_ID_LEN]; // For suspend use only
	PDECLAR_MESSAGE_MAP  
};


#endif
