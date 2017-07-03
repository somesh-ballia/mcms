#ifndef _CBRIDGEPARTYCNTL_H_
#define _CBRIDGEPARTYCNTL_H_

#include "StateMachine.h"
#include "BridgePartyInitParams.h"

class CBridgePartyMediaUniDirection;
class CBridgePartyVideoIn;

#define BRIDGE_PARTY_CONF_NAME_SIZE (2*H243_NAME_LEN+50)

typedef enum
{
	eNoCompareStyle       = 0,
	eCompareByTaskAppPtr  = 1,
	eCompareByName        = 2,
	eCompareByPartyRsrcID = 3
} ECompareStyle;

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyCntl
////////////////////////////////////////////////////////////////////////////
class CBridgePartyCntl : public CStateMachineValidation
{
	CLASS_TYPE_1(CBridgePartyCntl, CStateMachineValidation)

public:
	CBridgePartyCntl();
	CBridgePartyCntl(const void* partyId, const char* partyName = NULL, PartyRsrcID partyRsrcID = 0);
	CBridgePartyCntl(const CBridgePartyCntl& rOtherBridgePartyCntl);

	virtual ~CBridgePartyCntl ();
	virtual const char* NameOf() const { return "CBridgePartyCntl"; }

	CBridgePartyCntl&   operator=(const CBridgePartyCntl& rOtherBridgePartyCntl);

protected:
	friend bool operator<(const CBridgePartyCntl& rFirst, const CBridgePartyCntl& rSecond);
	friend bool operator==(const CBridgePartyCntl& rFirst, const CBridgePartyCntl& rSecond);

public:
	virtual void                   Create(const CBridgePartyInitParams* pBridgePartyInitParams);
	virtual void                   SetFullName(const char* pPartyName, const char* pConfName);

	void                           SetCompareFlag(const ECompareStyle eCompareBy)         { m_eCompareBy = eCompareBy; }
	void                           SetConnectionFailureCause(BYTE connectionFailureCause) { m_connectionFailureCause = connectionFailureCause; }

	void                           CopyInAndOut(const CBridgePartyCntl& rOtherBridgePartyCntl);

	virtual void                   UnregisterInTask();
	virtual void                   RegisterInTask(CTaskApp* myNewTask = NULL);

	char*                          GetFullName()                                          { return m_partyConfName; }
	char*                          GetName()                                              { return m_name; }

	const char*                    GetConfName();
	const CConf* 				   GetConf();
	DWORD 						   GetConfMediaType();

	CTaskApp*                      GetPartyTaskApp()                                      { return m_pParty; }
	CPartyApi*                     GetPartyApi()                                          { return m_pPartyApi; }
	PartyRsrcID                    GetPartyRsrcID() const                                 { return m_partyRsrcID; }
	ConfRsrcID                     GetConfRsrcID() const                                  { return m_confRsrcID; }
	CBridge*                       GetBridge() const                                      { return m_pBridge; }
	CBridgePartyMediaUniDirection* GetBridgePartyIn() const                               { return m_pBridgePartyIn; }
	CBridgePartyMediaUniDirection* GetBridgePartyOut() const                              { return m_pBridgePartyOut; }
	BYTE                           GetCascadeLinkMode() const                             { return m_bCascadeLinkMode; }
	CConfApi*                      GetConfApi () const 									  { return m_pConfApi;}

	BYTE                           GetIsGateWay();

	WORD                           GetNetworkInterface() const                            { return m_wNetworkInterface; }
	BYTE                           GetConnectionFailureCause() const                      { return m_connectionFailureCause; }
	BYTE                           GetConnectionFailureCauseDirection() const             { return m_connFailureCauseDirection; }
	BYTE                           GetConnectionFailureCauseTimerOrStatFailure() const    { return m_connFailureCauseTimerOrStatFailure; }
	BYTE                           GetConnectionFailureCausAction() const                 { return m_connFailureCauseAction; }
	EMediaDirection                GetDisconnectingDirectionsReq() const                  { return m_DisconnectingDirection; }

	virtual void                   SetDisConnectingDirectionsReq(EMediaDirection eDisConnectedDirection);

	// for debug info in case of "mcu internal problem"
	void                           SetConnectionFailureCauseAdditionalInfo(BYTE connFailureCauseDirection, BYTE connFailureCauseTimerOrStatFailure, BYTE connFailureCauseAction);

	BOOL                           IsUniDirectionConnection(EMediaDirection = eNoDirection);

	virtual void                   ArePortsOpened(BOOL& rIsInPortOpened, BOOL& rIsOutPortOpened) const;
	virtual void				   GetPortsOpened (std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)const {}

	virtual void                   HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	virtual void                   Destroy();
	virtual void                   Connect();
	virtual void                   DisConnect();
	virtual void                   Export();

	virtual void                   RemoveConfParams();
	virtual void                   UpdateNewConfParams(const CBridgePartyInitParams* pBridgePartyInitParams);
	virtual void                   DestroyPartyInOut(EMediaDirection ConnectedDirection);

	void                           DumpMcuInternalProblemDetailed(BYTE mipDirection, BYTE mipTimerStatus, BYTE mipMedia);
	void                           AddRsrcStrToMipDetailedStr(CLargeString& cstr, BYTE mipDirection, BYTE mipMedia);

	BOOL						   GetUseSpeakerSsrcForTx() const { return m_bUseSpeakerSsrcForTx; }
	void						   SetUseSpeakerSsrcForTx(BOOL bUseSpeakerSsrcForTx) { m_bUseSpeakerSsrcForTx = bUseSpeakerSsrcForTx; };

	BOOL                           IsAVMCUConnection(){return m_isAvMCUConnection;}
	void                           SetAVMCUConnection(BYTE isAVMCUConnection){m_isAvMCUConnection = isAVMCUConnection;}

	void                           SetIsCallGeneratorConference(BOOL bIsCallGeneratorConf) { m_bIsCallGeneratorConf = bIsCallGeneratorConf; }
	BOOL                           GetIsCallGeneratorConference()                          { return m_bIsCallGeneratorConf; }

	RoomID                         GetRoomID()const { return m_RoomId; }

protected:
	CTaskApp*                      m_pParty;
	ConfRsrcID                     m_confRsrcID;
	PartyRsrcID                    m_partyRsrcID;
	CPartyApi*                     m_pPartyApi;
	CConfApi*                      m_pConfApi;
	CBridge*                       m_pBridge;
	char                           m_name[H243_NAME_LEN];
	char                           m_partyConfName [BRIDGE_PARTY_CONF_NAME_SIZE];
	WORD                           m_wNetworkInterface;
	BYTE                           m_connectionFailureCause;
	BYTE                           m_connFailureCauseDirection;
	BYTE                           m_connFailureCauseTimerOrStatFailure;
	BYTE                           m_connFailureCauseAction;
	CBridgePartyMediaUniDirection* m_pBridgePartyIn;
	CBridgePartyMediaUniDirection* m_pBridgePartyOut;
	EMediaDirection                m_DisconnectingDirection;
	BYTE                           m_bCascadeLinkMode; // CASCADE_MODE_MASTER , CASCADE_MODE_SLAVE , CASCADE_MODE_NONE
	BYTE                           m_isAvMCUConnection;
	BOOL                           m_bUseSpeakerSsrcForTx;
	BOOL                           m_bIsCallGeneratorConf;
	RoomID                         m_RoomId;

private:
	ECompareStyle                  m_eCompareBy;
};

#endif // _CBRIDGEPARTYCNTL_H_

