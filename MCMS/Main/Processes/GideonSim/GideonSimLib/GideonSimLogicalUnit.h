
//+========================================================================+
//                 GideonSimLogicalUnit.h                                  |
//+========================================================================+


#if !defined(__GIDEONSIM_LOGICAL_UNIT_)
#define __GIDEONSIM_LOGICAL_UNIT_

// includes section
#include "CardsStructs.h"
#include "RtmIsdnMaintenanceStructs.h"
#include "StateMachine.h"

// classes declaration section
class CStateMachine;
class CTaskApi;
class CMplMcmsProtocol;
class CSimNetPort;

// define section
#define MAX_PORT_IN_UNIT	10
#define MAX_SWITCH_UNITS	15
#define MAX_PORT_IN_VIDEO_UNIT	4
#define MAX_PORT_IN_ART_UNIT	10

#define MAX_PORT_IN_UNIT_BARAK	48
#define MAX_PORT_IN_VIDEO_UNIT_BARAK	48
#define MAX_PORT_IN_ART_UNIT_BARAK	20


/////////////////////////////////////////////////////////////////////////////
//
//   CSimUnitPort - Port on MFA unit (ART, AC or Video)
//

class CSimUnitPort : public CStateMachine
{
CLASS_TYPE_1(CSimUnitPort,CStateMachine)
public:
			// Constructors
	CSimUnitPort();
	virtual ~CSimUnitPort();
	virtual const char* NameOf() const { return "CSimUnitPort";}

			// Initializations

			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

			// Utilities
	void OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol );
	int  UpdateBondingParameters( DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls);
	int  AddBondingChannel( BYTE *numOfChnls, BOOL *bBndAllChannelsConnected );
			// Utilities

	DWORD GetMuxConnectionId() { return m_MuxConnectionId; }

protected:
			// Action functions
	void OnMcmsOpenPortIDLE( CSegment* pData );
	void OnMcmsOpenPortOPEN( CSegment* pData );

	void OnMcmsClosePortIDLE( CSegment* pData );
	void OnMcmsClosePortOPEN( CSegment* pData );

protected:
			// Utilities
	DWORD			m_confId;
	DWORD			m_partyId;

	DWORD			m_MuxConnectionId;
	DWORD			m_AudConnectionId;
	DWORD			m_VidConnectionId;
	DWORD			m_RtpConnectionId;

	BYTE			m_totalBondingChannels;
	BYTE			m_connectedBondingChannels;

	PDECLAR_MESSAGE_MAP
};






/////////////////////////////////////////////////////////////////////////////
//
//   CSimAudioController
//
class CSimAudioController : public CPObject
{
CLASS_TYPE_1(CSimAudioController,CPObject)
public:
			// Constructors
	CSimAudioController() {;}
	virtual ~CSimAudioController(){;}
	virtual const char* NameOf() const { return "CSimAudioController";}

			// Initializations

			// Operations

protected:
			// Action functions

protected:
			// Utilities

};




/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnit - base class for all units components
//
class CGideonSimUnit : public CStateMachine
{
CLASS_TYPE_1(CGideonSimUnit,CStateMachine)
public:
			// Constructors
	CGideonSimUnit();
	virtual ~CGideonSimUnit();
	virtual const char* NameOf() const { return "CGideonSimUnit";}

			// Initializations

			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode) = 0;

public:
	void SetState( DWORD state ) { m_state = state; }

protected:
			// Action functions

protected:
			// Utilities

	PDECLAR_MESSAGE_MAP
};



/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitMfa - base class for all MFA units components
//
class CGideonSimUnitMfa : public CGideonSimUnit
{
public:
			// Constructors
	CGideonSimUnitMfa( WORD boardId, WORD subBoardId );
	virtual ~CGideonSimUnitMfa();

	virtual const char* NameOf() const { return "CGideonSimUnitMfa";}
			// Initializations

			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode) = 0;

protected:
			// Action functions

protected:
			// Utilities
	WORD	m_boardId;
	WORD	m_subBoardId;

	PDECLAR_MESSAGE_MAP

};


/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitMfaMedia - base class for all Media MFA units components
//
class CGideonSimUnitMfaMedia : public CGideonSimUnitMfa
{
public:
			// Constructors
	CGideonSimUnitMfaMedia( WORD boardId, WORD subBoardId );
	virtual ~CGideonSimUnitMfaMedia();

	virtual const char* NameOf() const { return "CGideonSimUnitMfaMedia";}
			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode) = 0;

	DWORD GetPortType() { return m_portType; }

	virtual int UpdateBondingParameters( DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls);
	virtual int AddBondingChannel( DWORD port, BYTE *numOfChnls, BOOL *bBndAllChannelsConnected );
	virtual DWORD GetMuxConnectionId(WORD portId);

protected:
			// Action functions

protected:
			// Utilities
	DWORD			m_portType;		// ART(+- AudioController) / Video
	WORD			m_maxPortType;	// max ports in unit per specific unit type
	CSimUnitPort*	m_portsList[MAX_PORT_IN_UNIT];

	PDECLAR_MESSAGE_MAP

};



/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitMfaMediaArt - class for MFA Media ART unit component
//
class CGideonSimUnitMfaMediaArt : public CGideonSimUnitMfaMedia
{
public:
			// Constructors
	CGideonSimUnitMfaMediaArt( WORD boardId, WORD subBoardId );
	virtual ~CGideonSimUnitMfaMediaArt();

	virtual const char* NameOf() const { return "CGideonSimUnitMfaMediaArt";}
			// Initializations

			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

			// Utilities
	void OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol, DWORD port );
	int UpdateBondingParameters( DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls);
	int AddBondingChannel( DWORD port, BYTE *numOfChnls, BOOL *bBndAllChannelsConnected );
	DWORD GetMuxConnectionId(WORD portId);

protected:
			// Action functions

protected:
			// Utilities
	CSimAudioController*	m_audioController;

	PDECLAR_MESSAGE_MAP

};


/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitMfaMediaVideo - class for MFA Media Video unit component
//
class CGideonSimUnitMfaMediaVideo : public CGideonSimUnitMfaMedia
{
public:
			// Constructors
	CGideonSimUnitMfaMediaVideo( WORD boardId, WORD subBoardId );
	virtual ~CGideonSimUnitMfaMediaVideo();

	virtual const char* NameOf() const { return "CGideonSimUnitMfaMediaVideo";}
			// Initializations

			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

			// Utilities
	void OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol, DWORD port );

protected:
			// Action functions

protected:
			// Utilities

	PDECLAR_MESSAGE_MAP

};



/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitBarak - base class for all BARAK units components
//
class CGideonSimUnitBarak : public CGideonSimUnit
{
public:
			// Constructors
	CGideonSimUnitBarak( WORD boardId, WORD subBoardId );
	virtual ~CGideonSimUnitBarak();

	virtual const char* NameOf() const { return "CGideonSimUnitBarak";}
			// Initializations

			// Operations

	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode) = 0;

protected:
			// Action functions

protected:
			// Utilities
	WORD	m_boardId;
	WORD	m_subBoardId;

	PDECLAR_MESSAGE_MAP

};


/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitBarakMedia - base class for all Media BARAK units components
//
class CGideonSimUnitBarakMedia : public CGideonSimUnitBarak
{
public:
			// Constructors
	CGideonSimUnitBarakMedia( WORD boardId, WORD subBoardId );
	virtual ~CGideonSimUnitBarakMedia();

	virtual const char* NameOf() const { return "CGideonSimUnitBarakMedia";}
			// Operations

	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode) = 0;

	DWORD GetPortType() { return m_portType; }

	virtual int UpdateBondingParameters( DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls);
	virtual int AddBondingChannel( DWORD port, BYTE *numOfChnls, BOOL *bBndAllChannelsConnected );
	virtual DWORD GetMuxConnectionId(WORD portId);

protected:
			// Action functions

protected:
			// Utilities
	DWORD			m_portType;		// ART(+- AudioController) / Video
	WORD			m_maxPortType;	// max ports in unit per specific unit type
	CSimUnitPort*	m_portsList[MAX_PORT_IN_UNIT_BARAK];

	PDECLAR_MESSAGE_MAP

};



/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitBarakMediaArt - class for BARAK Media ART unit component
//
class CGideonSimUnitBarakMediaArt : public CGideonSimUnitBarakMedia
{
public:
			// Constructors
	CGideonSimUnitBarakMediaArt( WORD boardId, WORD subBoardId );
	virtual ~CGideonSimUnitBarakMediaArt();

	virtual const char* NameOf() const { return "CGideonSimUnitBarakMediaArt";}
			// Initializations

			// Operations

	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

			// Utilities
	void OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol, DWORD port );
	int UpdateBondingParameters( DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls);
	int AddBondingChannel( DWORD port, BYTE *numOfChnls, BOOL *bBndAllChannelsConnected );
	DWORD GetMuxConnectionId(WORD portId);

protected:
			// Action functions

protected:
			// Utilities
	CSimAudioController*	m_audioController;

	PDECLAR_MESSAGE_MAP

};


/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitBarakMediaVideo - class for BARAK Media Video unit component
//
class CGideonSimUnitBarakMediaVideo : public CGideonSimUnitBarakMedia
{
public:
			// Constructors
	CGideonSimUnitBarakMediaVideo( WORD boardId, WORD subBoardId );
	virtual ~CGideonSimUnitBarakMediaVideo();

	virtual const char* NameOf() const { return "CGideonSimUnitBarakMediaVideo";}
			// Initializations

			// Operations

	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

			// Utilities
	void OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol, DWORD port );

protected:
			// Action functions

protected:
			// Utilities

	PDECLAR_MESSAGE_MAP

};




/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitSwitch - class for Switch unit components
//
class CGideonSimUnitSwitch : public CGideonSimUnit
{
public:
			// Constructors
	CGideonSimUnitSwitch( WORD boardId, WORD subBoardId );
	virtual ~CGideonSimUnitSwitch();

	virtual const char* NameOf() const { return "CGideonSimUnitSwitch";}
			// Initializations

			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode) = 0;

protected:
			// Action functions

protected:
			// Utilities
//    CSimSwitchUnitsList* m_units;
	PDECLAR_MESSAGE_MAP

};




/////////////////////////////////////////////////////////////////////////////
//
//   CSimMfaUnitsList - list of MFA units
//
class CSimMfaUnitsList : public CPObject
{
CLASS_TYPE_1(CSimMfaUnitsList,CPObject)
public:
			// Constructors
	CSimMfaUnitsList( WORD boardId, WORD subBoardId );
	virtual ~CSimMfaUnitsList();
	virtual const char* NameOf() const { return "CSimMfaUnitsList";}

			// Initializations

			// Operations

public:
	int UnitsConfigReq( CMplMcmsProtocol* pMplProtocol );
	void UpdateUnitStatus( WORD unitNum, STATUS status );
	void GetUnitsStatus( KEEP_ALIVE_S* unitsStructStatus );
	void GetUnitsStatus( APIU8* unitsStatus );
	void GetUnitsStatus( APIU32* unitsStatus );
	void GetUnitsType( APIU8* unitsType );
	void OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol );
	void SetCardType( DWORD cardType );
	void CalculateUnitsNum( DWORD cardType );
	int  UpdateBondingParameters( DWORD unit, DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls);
	int  AddBondingChannel( CMplMcmsProtocol* pMplProtocol, BYTE *numOfChannels, BOOL *bBndAllChannelsConnected );
	DWORD GetMuxConnectionId(WORD unitId, WORD portId);


protected:
			// Action functions

protected:
			// Utilities
	WORD	m_boardId;
	WORD	m_subBoardId;
	WORD	m_numOfUnits;
	CGideonSimUnitMfaMedia*	m_unitsList[MAX_NUM_OF_UNITS];
	DWORD	m_unitsState[MAX_NUM_OF_UNITS];
	DWORD	m_cardType;
};




/////////////////////////////////////////////////////////////////////////////
//
//   CSimBarakUnitsList - list of Barak units
//
class CSimBarakUnitsList : public CPObject
{
CLASS_TYPE_1(CSimBarakUnitsList,CPObject)
public:
			// Constructors
	CSimBarakUnitsList( WORD boardId, WORD subBoardId );
	virtual ~CSimBarakUnitsList();
	virtual const char* NameOf() const { return "CSimBarakUnitsList";}

			// Initializations

			// Operations



public:
	int UnitsConfigReq( CMplMcmsProtocol* pMplProtocol );
	void UpdateUnitStatus( WORD unitNum, STATUS status );
	void GetUnitsStatus( KEEP_ALIVE_S* unitsStructStatus );
	void GetUnitsStatus( APIU8* unitsStatus );
	void GetUnitsStatus( APIU32* unitsStatus );
	void GetUnitsType( APIU8* unitsType );
	void OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol );
	void SetCardType( DWORD cardType );
	void CalculateUnitsNum( DWORD cardType );
	int  UpdateBondingParameters( DWORD unit, DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls);
	int  AddBondingChannel( CMplMcmsProtocol* pMplProtocol, BYTE *numOfChannels, BOOL *bBndAllChannelsConnected );
	DWORD GetMuxConnectionId(WORD unitId, WORD portId);
	int ChangeUnitType(WORD unitId, APIU8 newUnitType);


protected:
			// Action functions

protected:
			// Utilities
	WORD	m_boardId;
	WORD	m_subBoardId;
	WORD	m_numOfUnits;
	CGideonSimUnitBarakMedia*	m_unitsList[MAX_NUM_OF_UNITS];
	DWORD	m_unitsState[MAX_NUM_OF_UNITS];
	DWORD	m_cardType;
};



/////////////////////////////////////////////////////////////////////////////
//
//   CSimSwitchUnitsList - list of Switch units
//
class CSimSwitchUnitsList : public CPObject
{
CLASS_TYPE_1(CSimSwitchUnitsList,CPObject)
public:
			// Constructors
	CSimSwitchUnitsList( WORD boardId );
	virtual ~CSimSwitchUnitsList();
	virtual const char* NameOf() const { return "CSimSwitchUnitsList";}

			// Initializations

			// Operations
	SWITCH_SM_KEEP_ALIVE_S m_switchSM_units;

protected:
			// Action functions
  //  void UpdateUnitStatus( WORD unitNum, STATUS status );
    void UpdateUnitStatus(SM_COMPONENT_STATUS_S & rswitch_keep_alive_ind_Struct, WORD unitNum, STATUS status );
protected:
			// Utilities
	WORD	m_boardId;
	WORD	m_SubBoardId;
	WORD	m_numOfUnits;
	//CGideonSimUnitSwitch*	m_unitsList[MAX_SWITCH_UNITS];


};


/////////////////////////////////////////////////////////////////////////////
//
//   CSimNetSpan
//
enum ENetSpanStatus
{
	eNetSpanStatusEmpty = 0,
	eNetSpanStatusNormal,
	eNetSpanStatusRedAlarm,
	eNetSpanStatusYellowAlarm,

	eNetSpanStatusUnknown // should be last
};

class CSimNetSpan : public CPObject
{
CLASS_TYPE_1(CSimNetSpan,CPObject)
public:
		// Constructors
	CSimNetSpan();
	virtual ~CSimNetSpan();
	virtual const char* NameOf() const { return "CSimNetSpan";}

		// Initializations

		// Operations

	void Configure(const RTM_ISDN_SPAN_CONFIG_REQ_S& rConfigStruct);
	BOOL IsPortValid(const BYTE portId) const;
	BOOL IsVirtPortInUse(const BYTE virtPort) const;
	BYTE FindPortByConfPartyId(const DWORD confId, const DWORD partId, DWORD opcode) const;
	BOOL IsPortInUseByConfParty(const DWORD portId, const DWORD confId, const DWORD partyId, DWORD opcode) const;
	BYTE AllocatePort(const DWORD virtPortId, const DWORD confId, const DWORD partId, const DWORD channelConnectionId);
	STATUS DeAllocatePort(const DWORD portId);
	DWORD GetVirtualPortNum(const DWORD portNum) const;
	void UpdatePortConfPartyId(const BYTE portId, const DWORD confId, const DWORD partId);
	void FillStatusStruct(RTM_ISDN_SPAN_STATUS_IND_S* pStatusStruct) const;
	void FillStatusStructBad( RTM_ISDN_SPAN_STATUS_IND_S* pStatusStruct,
	                          ENetSpanStatus spanStatus,
	                          bool isDChannelEstablished) const;

	BYTE GetPortByConnectionId(const DWORD channelConnectionId) const;
	STATUS DeAllocatePortByChannelConnectionId(const DWORD channelConnectionId);

protected:
		// Utilities

protected:
		// data members
	BYTE						m_numPorts;
	CSimNetPort**				m_paPorts;
	RTM_ISDN_SPAN_CONFIG_REQ_S	m_rConfigStruct;
};

/////////////////////////////////////////////////////////////////////////////
//
//   CSimNetPort
//
enum ENetPortStatus
{
	eNetPortStatusNormal = 0,
	eNetPortStatusDisabled,
	eNetPortStatusFail,
	eNetPortStatusRedAlarm,
	eNetPortStatusYellowAlarm,
	eNetPortStatusIdle,
	eNetPortStatusCrushed,

	eNetPortStatusUnknown // should be last
};

class CSimNetPort : public CPObject
{
CLASS_TYPE_1(CSimNetPort, CPObject)
public:
		// Constructors
	CSimNetPort();
	virtual ~CSimNetPort();
	virtual const char* NameOf() const { return "CSimNetPort";}

		// Initializations

		// Operations
	ENetPortStatus GetStatus() { return m_status; }

	BYTE GetVirtualPortNum() const { return m_virtualPort; }
	void  SetVirtualPortNum(const BYTE virtPortNum) { m_virtualPort = virtPortNum; }
	void  CleanVirtualPortNum() { return SetVirtualPortNum(0xFF); }

	DWORD GetConfId() const { return m_confId; }
	DWORD GetPartyId() const { return m_partyId; }
	void  SetConfPartyId(const DWORD confId,const DWORD partId);
	void  CleanConfPartyId() { return SetConfPartyId(0xFFFFFFFF, 0xFFFFFFFF); }

	DWORD GetChannelConnectionId() const { return m_channelConnectionId; }
	void  SetChannelConnectionId(const DWORD channelConnectionId) { m_channelConnectionId = channelConnectionId; }

	BOOL  IsEmpty() const;
	BOOL  IsReadyToCall() const;

protected:
		// Utilities

protected:
		// data members
	ENetPortStatus	m_status;
	BYTE			m_virtualPort;
	DWORD			m_confId;
	DWORD			m_partyId;
	DWORD			m_channelConnectionId;
};








#endif // !defined(__GIDEONSIM_LOGICAL_UNIT_)

