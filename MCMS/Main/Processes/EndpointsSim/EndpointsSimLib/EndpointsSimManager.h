// EndpointsSimManager.h: interface for the CEndpointsSimManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_EndpointsSimMANAGER_H__)
#define _EndpointsSimMANAGER_H__

#include <list>
#include "ManagerTask.h"
#include "Macros.h"
#include "ExternalDbSim.h"
#include "EndpointsSim.h"
#include "EpSimSipSubscription.h"
#include "CSSimClusterList.h"

class CListenSocketApi;
class CEndpointsSimSystemCfg;
class CTerminalCommand;

class CEndpointsSimManager : public CManagerTask
{
CLASS_TYPE_1(CEndpointsSimManager,CManagerTask )
public:
	CEndpointsSimManager();
	virtual ~CEndpointsSimManager();

	virtual const char* NameOf() const { return "CEndpointsSimManager";}
	void SelfKill();

	void ManagerPostInitActionsPoint();

	TaskEntryPoint GetMonitorEntryPoint();
	virtual void HandlePostRequest(CSegment* pSeg);
	void*        GetMessageMap();

protected:
	STATUS HandleTerminalStartCS(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalStopCS(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalListCS(CTerminalCommand& command, std::ostream& answer);

				// Batch scripts running transactions
		// add party commands
	STATUS OnTransH323PartyAdd(CRequest *pRequest);
	STATUS OnTransH323PartyErrorBitrate(CRequest *pRequest);
	STATUS OnTransSipPartyAdd(CRequest *pRequest);
	STATUS OnTransPstnPartyAdd(CRequest *pRequest);
	STATUS OnTransIsdnPartyAdd(CRequest *pRequest);
	STATUS TransPartyAdd(CRequest *pRequest, const eEndpointsTypes type) const;
		// all parties
	STATUS OnTransPartyDel(CRequest *pRequest);
	STATUS OnTransPartyConnect(CRequest *pRequest);
	STATUS OnTransPartyDisconnect(CRequest *pRequest);
	STATUS OnTransPartyDtmf(CRequest *pRequest);
	STATUS OnTransPartyChangeMode(CRequest *pRequest);
	STATUS OnTransSetPartyCapset(CRequest *pRequest) const;
		//  Common - for all endpoints types
	STATUS OnTransDelCapset(CRequest *pRequest);
	STATUS OnTransAddCapset(CRequest *pRequest);
	STATUS OnTransActiveSpeaker(CRequest *pRequest);
	STATUS OnTransAudioSpeaker(CRequest *pRequest);
	STATUS OnTransSipMute(CRequest *pRequest);
	STATUS OnTransMute(CRequest *pRequest);
	STATUS OnTransUnmute(CRequest *pRequest);
	STATUS OnTransFeccTokenRequest(CRequest *pRequest);
	STATUS OnTransFeccTokenRelease(CRequest *pRequest);
	STATUS OnTransH239TokenRequest(CRequest *pRequest);
	STATUS OnTransH239TokenRelease(CRequest *pRequest);
	STATUS OnTransEndpointUpdateChannels(CRequest *pRequest);
	STATUS OnTransLprModeChangeRequest(CRequest *pRequest);
	STATUS OnTransFeccKeyRequest(CRequest *pRequest);	
	   // Sockets
	STATUS HandleSocketDisconnectAction(CRequest *pRequest);
		//  External DB simulation
	STATUS OnTransExtermalDbKeepAlive(CRequest *pRequest);
	STATUS OnTransExtermalDbCreate(CRequest *pRequest);
	STATUS OnTransExtermalDbAdd(CRequest *pRequest);
	STATUS OnTransExtermalDbUser(CRequest *pRequest);
		// SIP subscriptions requests
	STATUS OnTransSipAddSubscription(CRequest *pRequest);
	STATUS OnTransSipGetNotification(CRequest *pRequest);
	    // Scp streams request
	STATUS OnTransScpStreamsRequest(CRequest *pRequest);

				// Action functions
	void OnListenSocketOpenConnection(CSegment* pParam);
	void OnListenSocketCloseConnection(CSegment* pParam);
	void OnGuiMsgAnycase(CSegment* pParam);
	void OnCsMsgAnystate(CSegment* pParam);
	void OnGideonAudioMsgAnystate(CSegment* pParam);
	void OnGideonMRMMsgAnystate(CSegment* pParam);
	void OnGideonIsdnMsgAnystate(CSegment* pParam);
 	void OnExtDbKeppAliveTout(CSegment* pParam);
  
				// Utilities
	void SendCommandToEndpoints(COsQueue& txMbx,const OPCODE code, CSegment *pParam);
	void SendCommandToScripts(const COsQueue& txMbx,const OPCODE code, CSegment *pParam);
	void SendCommandToGKSim(const COsQueue& txMbx,const OPCODE code, CSegment *pParam);
	void HandleGuiRequest(COsQueue& txMbx,CSegment *pParam);
	
	STATUS HandleTerminalEncryptionFlag(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleTerminalSendLpr(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleTerminalShowAllParties(CTerminalCommand & command, std::ostream& answer);

protected:
	CCSSimClusterList m_csClusters;

	// configuration of system
	CEndpointsSimSystemCfg* m_pEpSimConfig;
	// listen socket for Cards GUI application
	CListenSocketApi* m_pListenSocketApi;

	CTaskApi* m_pScriptsApi;
	CTaskApi* m_pEndpointsTaskApi;
	CExtDbSimulator m_extDbSimulator;

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS
};

#endif // !defined(_EndpointsSimMANAGER_H__)
