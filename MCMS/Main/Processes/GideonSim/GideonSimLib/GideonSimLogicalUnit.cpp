//+========================================================================+
//                GideonSimLogicalUnit.cpp                                 |
//+========================================================================+



// includes section
#include "Macros.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "IpMfaOpcodes.h"
#include "IpRtpReq.h"
#include "AcIndicationStructs.h"
#include "VideoStructs.h"
#include "SystemFunctions.h"
#include "StateMachine.h"
#include "TaskApi.h"
#include "MplMcmsProtocol.h"
#include "IvrApiStructures.h"
#include "IpRtpInd.h"
#include "CardsStructs.h"
#include "GideonSim.h"
#include "GideonSimConfig.h"
#include "SharedDefines.h"
#include "GideonSimLogicalUnit.h"
#include "StatusesGeneral.h"
#include "IpCmReq.h"
#include "MplMcmsProtocolTracer.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsVideo.h"
#include "AudRequestStructs.h"
#include "IvrApiStructures.h"
#include "McuMngrStructs.h"

#include "OpcodesRanges.h"  // for NET_ opcodes (until implemented by other sides)

#include "TraceStream.h"
#include "OpcodesMcmsNetQ931.h"


/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//

//const WORD  IDLE          = 0;        // default state -  defined in base class
const WORD  SIM_PORT_OPEN   = 1;

//const WORD  IDLE           = 0;        // default state -  defined in base class
const WORD  SIM_UNIT_ACTIVE  = 1;
const WORD  SIM_UNIT_FAILURE = 2;




/////////////////////////////////////////////////////////////////////////////
//
//   GideonSimLogicalUnit - base class (abstract) for all logical units
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CGideonSimUnit)
	//
PEND_MESSAGE_MAP(CGideonSimUnit,CStateMachine)


/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnit::CGideonSimUnit()
{
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnit::~CGideonSimUnit()
{
}


/////////////////////////////////////////////////////////////////////////////
void* CGideonSimUnit::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitMfa - MFA Units
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CGideonSimUnitMfa)
PEND_MESSAGE_MAP(CGideonSimUnitMfa,CGideonSimUnit)


/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitMfa::CGideonSimUnitMfa( WORD boardId, WORD subBoardId )
{
	m_boardId = boardId;
	m_subBoardId = subBoardId;
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitMfa::~CGideonSimUnitMfa()
{
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimUnitMfa::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitMfa::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent( opCode, pMsg );
}




/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitMfaMedia - MFA Media Units (ART (+-AudioController), Video)
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CGideonSimUnitMfaMedia)
PEND_MESSAGE_MAP(CGideonSimUnitMfaMedia,CGideonSimUnitMfa)


/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitMfaMedia::CGideonSimUnitMfaMedia( WORD boardId, WORD subBoardId )
			: CGideonSimUnitMfa( boardId, subBoardId )
{
	m_portType = eArt;	// one of them
	m_maxPortType = MAX_PORT_IN_UNIT;
	int i=0;
	for (i = 0; i < MAX_PORT_IN_UNIT; i++)
		m_portsList[i] = NULL;

	// checking
	if (MAX_PORT_IN_ART_UNIT > MAX_PORT_IN_UNIT)
		PASSERT( 100 );
	if (MAX_PORT_IN_VIDEO_UNIT > MAX_PORT_IN_UNIT)
		PASSERT( 100 );
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitMfaMedia::~CGideonSimUnitMfaMedia()
{
	int i=0;
	for (i = 0; i < MAX_PORT_IN_UNIT; i++)
		POBJDELETE( m_portsList[i] );
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimUnitMfaMedia::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitMfaMedia::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent( opCode, pMsg );
}

/////////////////////////////////////////////////////////////////////////////
int CGideonSimUnitMfaMedia::UpdateBondingParameters( DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls)
{
	// only valid for Art unit
	PTRACE(eLevelError,"CGideonSimUnitMfaMedia::UpdateBondingParameters - Base class should not be called (not Art)");
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
int CGideonSimUnitMfaMedia::AddBondingChannel( DWORD port, BYTE *numOfChnls, BOOL *bBndAllChannelsConnected )
{
	// only valid for Art unit
	PTRACE(eLevelError,"CGideonSimUnitMfaMedia::AddBondingChannel - Base class should not be called (not Art)");
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CGideonSimUnitMfaMedia::GetMuxConnectionId(WORD portId)
{
	// only valid for Art unit
	PTRACE(eLevelError,"CGideonSimUnitMfaMedia::AddBondingChannel - Base class should not be called (not Art)");
	return (DWORD)(-1);
}


/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitMfaMediaArt - MFA Media Unit ART
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////



PBEGIN_MESSAGE_MAP(CGideonSimUnitMfaMediaArt)

//	ONEVENT( opcode,           IDLE,    			CGideonSimUnitMfaMediaArt::OnOpcodeIDLE)
//	ONEVENT( opcode,           SIM_UNIT_FAILURE,  	CGideonSimUnitMfaMediaArt::OnOpcodeFAILURE)

PEND_MESSAGE_MAP(CGideonSimUnitMfaMediaArt,CGideonSimUnitMfaMedia)


/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitMfaMediaArt::CGideonSimUnitMfaMediaArt( WORD boardId, WORD subBoardId )
			: CGideonSimUnitMfaMedia( boardId, subBoardId )
{
	m_portType = eArt;
	m_maxPortType = MAX_PORT_IN_ART_UNIT;
	m_audioController = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitMfaMediaArt::~CGideonSimUnitMfaMediaArt()
{
	POBJDELETE(m_audioController);
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimUnitMfaMediaArt::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitMfaMediaArt::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent( opCode, pMsg );
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitMfaMediaArt::OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol, DWORD port )
{

	TRACEINTO	<< " CGideonSimUnitMfaMediaArt::OnMcmsCommandReq port:" << port << " m_state:" << m_state;

	if (m_state == SIM_UNIT_FAILURE)	// unit in failure state
	{
		PASSERT( 100 );
		return;
	}

	if (port >= MAX_PORT_IN_UNIT || port >= m_maxPortType)			// art=10
	{
		PASSERT( port );
		return;
	}

	if (NULL == pMplProtocol)
	{
		PASSERT( 100 );
		return;
	}

	if (NULL == m_portsList[port])
		m_portsList[port] = new CSimUnitPort;

	m_portsList[port]->OnMcmsCommandReq( pMplProtocol );
}

/////////////////////////////////////////////////////////////////////////////
int CGideonSimUnitMfaMediaArt::UpdateBondingParameters( DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls)
{
	// only valid for Art unit
	if (eArt != m_portType)
		return -1;	// should not happen as this is an Art class

	if (port >= MAX_PORT_IN_UNIT)
	{
		TRACEINTO	<< " CGideonSimUnitMfaMediaArt::UpdateBondingParameters Illegal port number:" << port;
		return -1;
	}

	if (NULL == m_portsList[port])
	{
		TRACEINTO	<< " CGideonSimUnitMfaMediaArt::UpdateBondingParameters Illegal NULL port:" << port;
		return -1;
	}

	int status = m_portsList[port]->UpdateBondingParameters( muxConnectionID, confId, partyId, numOfChnls);
	return status;
}

/////////////////////////////////////////////////////////////////////////////
int CGideonSimUnitMfaMediaArt::AddBondingChannel( DWORD port, BYTE *numOfChnls, BOOL *bBndAllChannelsConnected )
{
	// only valid for Art unit
	if (eArt != m_portType)
		return -1;	// should not happen as this is an Art class

	if (port >= MAX_PORT_IN_UNIT)
	{
		TRACEINTO	<< " CGideonSimUnitMfaMediaArt::AddBondingChannel Illegal port number:" << port;
		return -1;
	}

	if (NULL == m_portsList[port])
	{
		TRACEINTO	<< " CGideonSimUnitMfaMediaArt::AddBondingChannel Illegal NULL port:" << port;
		return -1;
	}

	int status = m_portsList[port]->AddBondingChannel( numOfChnls, bBndAllChannelsConnected );
	return status;

}

/////////////////////////////////////////////////////////////////////////////
DWORD CGideonSimUnitMfaMediaArt::GetMuxConnectionId(WORD portId)
{
	return m_portsList[portId]->GetMuxConnectionId();
}




/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitMfaMediaVideo - MFA Media Unit Video
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CGideonSimUnitMfaMediaVideo)
PEND_MESSAGE_MAP(CGideonSimUnitMfaMediaVideo,CGideonSimUnitMfaMedia)


/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitMfaMediaVideo::CGideonSimUnitMfaMediaVideo( WORD boardId, WORD subBoardId )
			: CGideonSimUnitMfaMedia( boardId, subBoardId )
{
	m_portType = eVideo;
	m_maxPortType = MAX_PORT_IN_VIDEO_UNIT;
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitMfaMediaVideo::~CGideonSimUnitMfaMediaVideo()
{
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimUnitMfaMediaVideo::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitMfaMediaVideo::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent( opCode, pMsg );
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitMfaMediaVideo::OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol, DWORD port )
{
	if (m_state == SIM_UNIT_FAILURE)	// unit in failure state
	{
		PASSERT( 100 );
		return;
	}

	if (port >= m_maxPortType)			// art=10
	{
		PASSERT( port );
		return;
	}

	if (NULL == pMplProtocol)
	{
		PASSERT( 100 );
		return;
	}

	// gets the resource type
	APIU8 rsrcType = pMplProtocol->getPhysicalInfoHeaderResource_type();
	if( (rsrcType == ePhysical_video_encoder) && (0 != port && 2 != port) ) // the Video Encoder must be in port 0
	{
		PASSERT( 100 );
		return;
	}
	if( (rsrcType == ePhysical_video_decoder) && (1 != port && 3 != port) )	// the Video Decoder must be in port 1
	{
		PASSERT( 100 );
		return;
	}

	// allocate port if needed
	if (NULL == m_portsList[port])
		m_portsList[port] = new CSimUnitPort;

	// video comand to port
	m_portsList[port]->OnMcmsCommandReq( pMplProtocol );
}



/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitBarak - BARAK Units
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CGideonSimUnitBarak)
PEND_MESSAGE_MAP(CGideonSimUnitBarak,CGideonSimUnit)


/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitBarak::CGideonSimUnitBarak( WORD boardId, WORD subBoardId )
{
	m_boardId = boardId;
	m_subBoardId = subBoardId;
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitBarak::~CGideonSimUnitBarak()
{
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimUnitBarak::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitBarak::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent( opCode, pMsg );
}




/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitBarakMedia - BARAK Media Units (ART (+-AudioController), Video)
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CGideonSimUnitBarakMedia)
PEND_MESSAGE_MAP(CGideonSimUnitBarakMedia,CGideonSimUnitBarak)


/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitBarakMedia::CGideonSimUnitBarakMedia( WORD boardId, WORD subBoardId )
			: CGideonSimUnitBarak( boardId, subBoardId )
{
	m_portType = eArt;	// one of them
	m_maxPortType = MAX_PORT_IN_UNIT_BARAK;
	int i=0;
	for (i = 0; i < MAX_PORT_IN_UNIT_BARAK; i++)
		m_portsList[i] = NULL;

	// checking
	if (MAX_PORT_IN_ART_UNIT_BARAK > MAX_PORT_IN_UNIT_BARAK)
		PASSERT( 100 );
	if (MAX_PORT_IN_VIDEO_UNIT_BARAK > MAX_PORT_IN_UNIT_BARAK)
		PASSERT( 100 );
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitBarakMedia::~CGideonSimUnitBarakMedia()
{
	int i=0;
	for (i = 0; i < MAX_PORT_IN_UNIT_BARAK; i++)
		POBJDELETE( m_portsList[i] );
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimUnitBarakMedia::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitBarakMedia::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent( opCode, pMsg );
}


/////////////////////////////////////////////////////////////////////////////
int CGideonSimUnitBarakMedia::UpdateBondingParameters( DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls)
{
	// only valid for Art unit
	PTRACE(eLevelError,"CGideonSimUnitBarakMedia::UpdateBondingParameters - Base class should not be called (not Art)");
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
int CGideonSimUnitBarakMedia::AddBondingChannel( DWORD port, BYTE *numOfChnls, BOOL *bBndAllChannelsConnected )
{
	// only valid for Art unit
	PTRACE(eLevelError,"CGideonSimUnitBarakMedia::AddBondingChannel - Base class should not be called (not Art)");
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CGideonSimUnitBarakMedia::GetMuxConnectionId(WORD portId)
{
	// only valid for Art unit
	PTRACE(eLevelError,"CGideonSimUnitBarakMedia::AddBondingChannel - Base class should not be called (not Art)");
	return (DWORD)(-1);
}


/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitBarakMediaArt - BARAK Media Unit ART
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////



PBEGIN_MESSAGE_MAP(CGideonSimUnitBarakMediaArt)

//	ONEVENT( opcode,           IDLE,    			CGideonSimUnitBarakMediaArt::OnOpcodeIDLE)
//	ONEVENT( opcode,           SIM_UNIT_FAILURE,  	CGideonSimUnitBarakMediaArt::OnOpcodeFAILURE)

PEND_MESSAGE_MAP(CGideonSimUnitBarakMediaArt,CGideonSimUnitBarakMedia)


/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitBarakMediaArt::CGideonSimUnitBarakMediaArt( WORD boardId, WORD subBoardId )
			: CGideonSimUnitBarakMedia( boardId, subBoardId )
{
	m_portType = eArt;
	m_maxPortType = MAX_PORT_IN_ART_UNIT_BARAK;
	m_audioController = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitBarakMediaArt::~CGideonSimUnitBarakMediaArt()
{
	POBJDELETE(m_audioController);
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimUnitBarakMediaArt::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitBarakMediaArt::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent( opCode, pMsg );
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitBarakMediaArt::OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol, DWORD port )
{
	if (NULL == pMplProtocol)
	{
		PASSERT( 100 );
		return;
	}

	TRACEINTO	<< " CGideonSimUnitBarakMediaArt::OnMcmsCommandReq port:" << port << " m_state:" << m_state<<"physical resource type"<<(int)pMplProtocol->getPhysicalInfoHeaderResource_type()<<" lrt:"<<(int)pMplProtocol->getPortDescriptionHeaderLogical_resource_type_1()<<" request id:"<<pMplProtocol->getMsgDescriptionHeaderRequest_id();

	if(port >= MAX_PORT_IN_UNIT_BARAK)
	{
		PASSERT( port );
		return;
	}

	if (m_state == SIM_UNIT_FAILURE)	// unit in failure state
	{
		PASSERT( 100 );
		return;
	}

	if (port >= m_maxPortType)			// art=10
	{
		PASSERT( port );
		return;
	}


	if (port < MAX_PORT_IN_UNIT_BARAK && NULL == m_portsList[port])
		m_portsList[port] = new CSimUnitPort;

	if(port < MAX_PORT_IN_UNIT_BARAK )
		m_portsList[port]->OnMcmsCommandReq( pMplProtocol );
}

/////////////////////////////////////////////////////////////////////////////
int CGideonSimUnitBarakMediaArt::UpdateBondingParameters( DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls)
{
	// only valid for Art unit
	if (eArt != m_portType)
		return -1;	// should not happen as this is an Art class
	
	if (port >= MAX_PORT_IN_UNIT_BARAK)
	{
		TRACEINTO	<< " CGideonSimUnitBarakMediaArt::UpdateBondingParameters Illegal port number:" << port;
		return -1;
	}
	
	if (NULL == m_portsList[port])
	{
		TRACEINTO	<< " CGideonSimUnitBarakMediaArt::UpdateBondingParameters Illegal NULL port:" << port;
		return -1;
	}
	
	int status = m_portsList[port]->UpdateBondingParameters( muxConnectionID, confId, partyId, numOfChnls);
	return status;
}

/////////////////////////////////////////////////////////////////////////////
int CGideonSimUnitBarakMediaArt::AddBondingChannel( DWORD port, BYTE *numOfChnls, BOOL *bBndAllChannelsConnected )
{
	// only valid for Art unit
	if (eArt != m_portType)
		return -1;	// should not happen as this is an Art class
	
	if (port >= MAX_PORT_IN_UNIT_BARAK)
	{
		TRACEINTO	<< " CGideonSimUnitBarakMediaArt::AddBondingChannel Illegal port number:" << port;
		return -1;
	}
	
	if (NULL == m_portsList[port])
	{
		TRACEINTO	<< " CGideonSimUnitBarakMediaArt::AddBondingChannel Illegal NULL port:" << port;
		return -1;
	}

	int status = m_portsList[port]->AddBondingChannel( numOfChnls, bBndAllChannelsConnected );
	return status;
	
}

/////////////////////////////////////////////////////////////////////////////
DWORD CGideonSimUnitBarakMediaArt::GetMuxConnectionId(WORD portId)
{
	return m_portsList[portId]->GetMuxConnectionId();
}



/////////////////////////////////////////////////////////////////////////////
//
//   CGideonSimUnitBarakMediaVideo - BARAK Media Unit Video
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CGideonSimUnitBarakMediaVideo)
PEND_MESSAGE_MAP(CGideonSimUnitBarakMediaVideo,CGideonSimUnitBarakMedia)


/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitBarakMediaVideo::CGideonSimUnitBarakMediaVideo( WORD boardId, WORD subBoardId )
			: CGideonSimUnitBarakMedia( boardId, subBoardId )
{
	m_portType = eVideo;
	m_maxPortType = MAX_PORT_IN_VIDEO_UNIT_BARAK;
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimUnitBarakMediaVideo::~CGideonSimUnitBarakMediaVideo()
{
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimUnitBarakMediaVideo::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitBarakMediaVideo::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent( opCode, pMsg );
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimUnitBarakMediaVideo::OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol, DWORD port )
{
	if (m_state == SIM_UNIT_FAILURE)	// unit in failure state
	{
		PASSERT( 100 );
		return;
	}

	if (port >= m_maxPortType)			// art=10
	{
		PASSERT( port );
		return;
	}

	if(port >= MAX_PORT_IN_UNIT_BARAK)
	{
		PASSERT( port );
		return;
	}

	if (NULL == pMplProtocol)
	{
		PASSERT( 100 );
		return;
	}

	// gets the resource type
	APIU8 rsrcType = pMplProtocol->getPhysicalInfoHeaderResource_type();
	if( (rsrcType == ePhysical_video_encoder) && (0 != port % 2) ) // the Video Encoder must be on even port number
	{
		PASSERT( 100 );
		return;
	}
	if( (rsrcType == ePhysical_video_decoder) && (0 == port % 2) )	// the Video Decoder must be on odd port number
	{
		PASSERT( 100 );
		return;
	}

	// allocate port if needed
	if (NULL == m_portsList[port])
		m_portsList[port] = new CSimUnitPort;

	// video comand to port
	m_portsList[port]->OnMcmsCommandReq( pMplProtocol );
}



/////////////////////////////////////////////////////////////////////////////
//
//   CSimUnitPort - MFA Unit Port
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimUnitPort)

	ONEVENT( TB_MSG_OPEN_PORT_REQ,           IDLE,    			CSimUnitPort::OnMcmsOpenPortIDLE)
	ONEVENT( TB_MSG_OPEN_PORT_REQ,           SIM_PORT_OPEN,    	CSimUnitPort::OnMcmsOpenPortOPEN)

	ONEVENT( TB_MSG_CLOSE_PORT_REQ,			IDLE,    			CSimUnitPort::OnMcmsClosePortIDLE)
	ONEVENT( TB_MSG_CLOSE_PORT_REQ,         SIM_PORT_OPEN,    	CSimUnitPort::OnMcmsClosePortOPEN)

PEND_MESSAGE_MAP(CSimUnitPort,CStateMachine)


/////////////////////////////////////////////////////////////////////////////
CSimUnitPort::CSimUnitPort()
{
	m_confId = 0;
	m_partyId = 0;

	m_MuxConnectionId = 0;
	m_AudConnectionId = 0;
	m_VidConnectionId = 0;
	m_RtpConnectionId = 0;

	m_totalBondingChannels = 0;
	m_connectedBondingChannels = 0;
}

/////////////////////////////////////////////////////////////////////////////
CSimUnitPort::~CSimUnitPort()
{
}

/////////////////////////////////////////////////////////////////////////////
void* CSimUnitPort::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CSimUnitPort::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent( opCode, pMsg );
}

/////////////////////////////////////////////////////////////////////////////
void CSimUnitPort::OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol )
{
	CSegment *seg = new CSegment;
	*seg << pMplProtocol;

	DispatchEvent( pMplProtocol->getOpcode(), seg );

	TRACEINTO	<< " CSimUnitPort::OnMcmsCommandReq opcode:" << pMplProtocol->getOpcode();

	POBJDELETE( seg );
}


/////////////////////////////////////////////////////////////////////////////
void CSimUnitPort::OnMcmsOpenPortIDLE( CSegment* pData )
{
	m_state = SIM_PORT_OPEN;

	// for debug for debug for debug for debug for debug for debug for debug for debug for debug
	// for debug for debug for debug for debug for debug for debug for debug for debug for debug
	CMplMcmsProtocol* pMplProtocol = NULL;
	*pData >> (DWORD&)pMplProtocol;
	DWORD opcode = pMplProtocol->getOpcode();
	int x = 1;
	// for debug for debug for debug for debug for debug for debug for debug for debug for debug
	// for debug for debug for debug for debug for debug for debug for debug for debug for debug

}

/////////////////////////////////////////////////////////////////////////////
void CSimUnitPort::OnMcmsOpenPortOPEN( CSegment* pData )
{
// disabled temporary until VNGSW-277 is fixed 
//	PASSERT( 100 );
}

/////////////////////////////////////////////////////////////////////////////
void CSimUnitPort::OnMcmsClosePortIDLE( CSegment* pData )
{

	PASSERT( 100 );
}

/////////////////////////////////////////////////////////////////////////////
void CSimUnitPort::OnMcmsClosePortOPEN( CSegment* pData )
{
	m_state = IDLE;
}

/////////////////////////////////////////////////////////////////////////////
int CSimUnitPort::UpdateBondingParameters( DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls)
{
	if (SIM_PORT_OPEN != m_state)
	{
		TRACESTR(eLevelError) << " CSimUnitPort::UpdateBondingParameters - Illegal port state";
		return -1;	// port should be open
	}

//	if ((m_confId != confId) || (m_partyId != partyId))
//	{
//		TRACESTR(eLevelError) << " CSimUnitPort::UpdateBondingParameters - Illegal confID or partyID";
//		PASSERT(1);
//		return -1;	// port should be open
//	}

	m_MuxConnectionId 			= muxConnectionID; 	// update MUX connection ID
	m_totalBondingChannels 		= numOfChnls;		// number of total channels that should be connected
	m_connectedBondingChannels 	= 1;				// init-bondind command sent only after the first channel was already connected

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CSimUnitPort::AddBondingChannel( BYTE *numOfChnls, BOOL *bBndAllChannelsConnected )
{
	if (SIM_PORT_OPEN != m_state)
	{
		TRACESTR(eLevelError) << " CSimUnitPort::AddBondingChannel - Illegal port state";
		return -1;	// port should be open
	}

	*numOfChnls = m_totalBondingChannels;
	if (m_connectedBondingChannels < m_totalBondingChannels)
		m_connectedBondingChannels++;
	else
	{
		TRACESTR(eLevelError) << " CSimUnitPort::AddBondingChannel - Too many bonding channels";
		return -1;
	}

	if (m_connectedBondingChannels == m_totalBondingChannels)
		*bBndAllChannelsConnected = TRUE;
	else
		*bBndAllChannelsConnected = FALSE;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
//   CSimMfaUnitsList - MFA Units List
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CSimMfaUnitsList::CSimMfaUnitsList( WORD boardId, WORD subBoardId )
{
	m_boardId = boardId;
	m_subBoardId = subBoardId;
	int i=0;
	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		m_unitsList[i] = NULL;
		m_unitsState[i] = eOk;
	}
	m_numOfUnits = 26;
	m_cardType = eMfa_26;
	CalculateUnitsNum( m_cardType );
}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaUnitsList::CalculateUnitsNum( DWORD cardType )
{
	switch( cardType )
	{
		case eMfa_26:	m_numOfUnits = 26; break;
		case eMfa_13:   m_numOfUnits = 13; break;
///		case eMpmPlus_20:			m_numOfUnits = 9; break;
///		case eMpmPlus_40:			m_numOfUnits = 16; break;
///		case eMpmPlus_80:			m_numOfUnits = 32; break;
///		case eMpmPlus_MezzanineA:	m_numOfUnits = 24; break;
///		case eMpmPlus_MezzanineB:	m_numOfUnits = 24; break;
		default:
			PASSERT( 100 );
			break;
	}
}


/////////////////////////////////////////////////////////////////////////////
CSimMfaUnitsList::~CSimMfaUnitsList()
{
	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
		POBJDELETE( m_unitsList[i] );
}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaUnitsList::SetCardType( DWORD cardType )
{
	if ( cardType < NUM_OF_CARD_TYPES)
	{
		m_cardType = cardType;
		CalculateUnitsNum( m_cardType );
	}
}


////////////////////////////////////////////////////////////////////////////
void CSimMfaUnitsList::UpdateUnitStatus( WORD unitNum, STATUS status )
{
	if ( unitNum < MAX_NUM_OF_UNITS)
	{
		m_unitsState[unitNum] = status;
		if (m_unitsList[unitNum])
			m_unitsList[unitNum]->SetState( status );


	}
	else
		PTRACE(eLevelError,"CSimMfaUnitsList::UpdateUnitStatus Error - illegal unit #");
}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaUnitsList::GetUnitsStatus( KEEP_ALIVE_S* unitsStructStatus )
{
	int i=0;
	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
		unitsStructStatus->statusOfUnitsList[i] = (APIU32)m_unitsState[i];
}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaUnitsList::GetUnitsStatus( APIU8* unitsStatus )
{
	int i=0;
	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
		unitsStatus[i] = (APIU8)m_unitsState[i];
}

/////////////////////////////////////////////////////////////////////////////
void CSimMfaUnitsList::GetUnitsStatus( APIU32* unitsStatus )
{
	int i=0;
	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
		unitsStatus[i] = (APIU32)m_unitsState[i];
}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaUnitsList::GetUnitsType( APIU8* unitsType )
{
	int i=0;

	unitsType[0]  = ePQ;				// card manager

	for (i = 1; ((i <= m_numOfUnits) && (i < MAX_NUM_OF_UNITS)); i++)
		unitsType[i]  = eDsp;			// DSP

	for (i = (m_numOfUnits+1); i < MAX_NUM_OF_UNITS; i++)
		unitsType[i]  = eUndefined;		// undefined in this version
}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaUnitsList::OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol )
{
	if (NULL == pMplProtocol)
		return;

	// get unit and port
	DWORD unit = pMplProtocol->getPhysicalInfoHeaderUnit_id();
	DWORD port = pMplProtocol->getPhysicalInfoHeaderPort_id();
	DWORD opcode = pMplProtocol->getOpcode();
	APIU8 rsrcType = pMplProtocol->getPhysicalInfoHeaderResource_type();

	TRACEINTO	<< " CSimMfaUnitsList::OnMcmsCommandReq opcode:" << opcode << " unit:" << unit
										<< " port:" << port << " rsrcType: " << (int)rsrcType;

	if (unit >= MAX_NUM_OF_UNITS)
	{
		PASSERT( unit );
		return;
	}

	if (NULL == m_unitsList[unit])
	{
		PTRACE(eLevelError,"CSimMfaUnitsList::OnMcmsCommandReq - unit not found");
		PASSERT(unit);
		return;
	}

	switch( opcode )
	{
		case TB_MSG_OPEN_PORT_REQ:	// open port (art, video)
		case TB_MSG_CLOSE_PORT_REQ: // close port (art, video)
		{
			if( rsrcType == ePhysical_art  || rsrcType == ePhysical_art_light )
			{
				// open art port
				if (m_unitsList[unit]->GetPortType() != (DWORD)eArt)
				{
					PTRACE(eLevelError,"CSimMfaUnitsList::OnMcmsCommandReq - unit not type ART");
					PASSERT(unit);
					return;
				}

				TRACEINTO	<< " CSimMfaUnitsList::OnMcmsCommandReq rsrcType:" << (int)rsrcType;

				((CGideonSimUnitMfaMediaArt*)m_unitsList[unit])->OnMcmsCommandReq( pMplProtocol, port );
			}
			else if (rsrcType == ePhysical_video_encoder || rsrcType == ePhysical_video_decoder )
			{
				// open Video Decoder port
				if (m_unitsList[unit]->GetPortType() != (DWORD)eVideo)
				{
					PTRACE(eLevelError,"CSimMfaUnitsList::OnMcmsCommandReq - unit not type Video");
					PASSERT(unit);
					return;
				}
				((CGideonSimUnitMfaMediaVideo*)m_unitsList[unit])->OnMcmsCommandReq( pMplProtocol, port );
			}
			else
			{
				PASSERT( 100 );
				return;
			}
			break;
		}


		default:
			PTRACE(eLevelError,"CSimMfaUnitsList::OnMcmsCommandReq - unknown command");
			break;
	}
}


/////////////////////////////////////////////////////////////////////////////
int CSimMfaUnitsList::UnitsConfigReq( CMplMcmsProtocol* pMplProtocol )
{
	// gets the request struct
	CM_UNITS_CONFIG_S*  pUnits = (CM_UNITS_CONFIG_S*)(pMplProtocol->GetData());
	if (!pUnits)
		return STATUS_FAIL;

	// fill unit types
	int i=0;
	for (i = 1; i < MAX_NUM_OF_UNITS; i++)
	{
		if (NULL == m_unitsList[i])
		{
			if (pUnits->unitsParamsList[i].type == eArt || pUnits->unitsParamsList[i].type == eArtCntlr)
				m_unitsList[i] = (CGideonSimUnitMfaMedia*) new CGideonSimUnitMfaMediaArt( m_boardId, m_subBoardId );
			else  if (pUnits->unitsParamsList[i].type == eVideo)
				m_unitsList[i] = (CGideonSimUnitMfaMedia*) new CGideonSimUnitMfaMediaVideo( m_boardId, m_subBoardId );
			else
				break;	// end of units
			m_unitsList[i]->SetState( m_unitsState[i] ); // in case we want in the future to start with faulty unit
		}
	}

	return STATUS_OK;
}
	
	
/////////////////////////////////////////////////////////////////////////////
int CSimMfaUnitsList::UpdateBondingParameters( DWORD unit, DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls)
{
	if (unit >= MAX_NUM_OF_UNITS)
	{
		PTRACE(eLevelError,"CSimMfaUnitsList::UpdateBondingParameters - illegal unit number");
		return -1;
	}
	
	if (NULL == m_unitsList[unit])
	{
		PTRACE(eLevelError,"CSimMfaUnitsList::UpdateBondingParameters - illegal unit NULL");
		return -1;
	}
	
	int status = m_unitsList[unit]->UpdateBondingParameters( port, muxConnectionID, confId, partyId, numOfChnls);
	
	return status;
}

/////////////////////////////////////////////////////////////////////////////
int CSimMfaUnitsList::AddBondingChannel( CMplMcmsProtocol* pMplProtocol, BYTE *numOfChannels, BOOL *bBndAllChannelsConnected )
{
	DWORD unit = pMplProtocol->getPhysicalInfoHeaderUnit_id();
	DWORD port = pMplProtocol->getPhysicalInfoHeaderPort_id();
	
	if (unit >= MAX_NUM_OF_UNITS)
	{
		PTRACE(eLevelError,"CSimMfaUnitsList::AddBondingChannel - illegal unit number");
		return -1;
	}
	
	if (NULL == m_unitsList[unit])
	{
		PTRACE(eLevelError,"CSimMfaUnitsList::AddBondingChannel - illegal unit NULL");
		return -1;
	}
	
	int status = m_unitsList[unit]->AddBondingChannel( port, numOfChannels, bBndAllChannelsConnected );
	
	return status;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CSimMfaUnitsList::GetMuxConnectionId(WORD unitId, WORD portId)
{
	return m_unitsList[unitId]->GetMuxConnectionId(portId);
}

	
	
/////////////////////////////////////////////////////////////////////////////
//
//   CSimBarakUnitsList - BARAK Units List
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CSimBarakUnitsList::CSimBarakUnitsList( WORD boardId, WORD subBoardId )
{
	m_boardId = boardId;
	m_subBoardId = subBoardId;
	int i=0;
	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		m_unitsList[i] = NULL;
		m_unitsState[i] = eOk;
	}
	m_numOfUnits = 16;		///TBD
	m_cardType = eMpmPlus_40;
	CalculateUnitsNum( m_cardType );
}


/////////////////////////////////////////////////////////////////////////////
void CSimBarakUnitsList::CalculateUnitsNum( DWORD cardType )
{
	switch( cardType )
	{
		case eMpmPlus_20:			m_numOfUnits = 9; break;
		case eMpmPlus_40:			m_numOfUnits = 16; break;
		case eMpmPlus_80:			m_numOfUnits = 32; break;
		case eMpmPlus_MezzanineA:	m_numOfUnits = 24; break;
		case eMpmPlus_MezzanineB:	m_numOfUnits = 24; break;
		//tbd in some way - check that it is according to cardtype 
		case eMpmx_80:				m_numOfUnits = 40; break;
		case eMpmx_40:		        m_numOfUnits = 20; break;
		case eMpmx_20:		        m_numOfUnits = 10; break;
		case eMpmRx_Full:           m_numOfUnits = 32; break;
		case eMpmRx_Half:           m_numOfUnits = 20; break;

		default:
			PASSERT( 100 );
			break;
	}
}


/////////////////////////////////////////////////////////////////////////////
CSimBarakUnitsList::~CSimBarakUnitsList()
{
	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
		POBJDELETE( m_unitsList[i] );
}


/////////////////////////////////////////////////////////////////////////////
void CSimBarakUnitsList::SetCardType( DWORD cardType )
{
	if ( cardType < NUM_OF_CARD_TYPES)
	{
		m_cardType = cardType;
		CalculateUnitsNum( m_cardType );
	}
}


////////////////////////////////////////////////////////////////////////////
void CSimBarakUnitsList::UpdateUnitStatus( WORD unitNum, STATUS status )
{
	if ( unitNum < MAX_NUM_OF_UNITS)
	{
		m_unitsState[unitNum] = status;
		if (m_unitsList[unitNum])
			m_unitsList[unitNum]->SetState( status );


	}
	else
		PTRACE(eLevelError,"CSimBarakUnitsList::UpdateUnitStatus Error - illegal unit #");
}


/////////////////////////////////////////////////////////////////////////////
void CSimBarakUnitsList::GetUnitsStatus( KEEP_ALIVE_S* unitsStructStatus )
{
	int i=0;
	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
		unitsStructStatus->statusOfUnitsList[i] = (APIU32)m_unitsState[i];
}


/////////////////////////////////////////////////////////////////////////////
void CSimBarakUnitsList::GetUnitsStatus( APIU8* unitsStatus )
{
	int i=0;
	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
		unitsStatus[i] = (APIU8)m_unitsState[i];
}

/////////////////////////////////////////////////////////////////////////////
void CSimBarakUnitsList::GetUnitsStatus( APIU32* unitsStatus )
{
	int i=0;
	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
		unitsStatus[i] = (APIU32)m_unitsState[i];
}


/////////////////////////////////////////////////////////////////////////////
void CSimBarakUnitsList::GetUnitsType( APIU8* unitsType )
{
	int i=0;

	unitsType[0]  = ePQ;				// card manager

	int firstUnitNumber = 1;
	int lastUnitNumber = m_numOfUnits;

	if (9 == m_numOfUnits)	// in case of card type eMpmPlus_h (half carrier - 9 DSPs)
	{
/*		firstUnitNumber = 8;	// first unit is called "unit number 8"
		lastUnitNumber = 16;	// last unit is called "unit number 16"

		for (i = 1; i < firstUnitNumber; i++)
			unitsType[i]  = eUndefined;		// undefined in this version
*/
		// DSP = 1,2,3,4,9,10,11,12,13
		firstUnitNumber 	 = 1;	// first unit is called "unit number 1"
		lastUnitNumber 		 = 4;	// last unit is called "unit number 4"
		int firstUnitNumber2 = 9;	// first unit is called "unit number 9"
		int lastUnitNumber2  = 13;	// last unit is called "unit number 13"

		for (i = firstUnitNumber; ((i <= lastUnitNumber) && (i < MAX_NUM_OF_UNITS)); i++)
			unitsType[i]  = eDsp;			// DSP
		for (i = (lastUnitNumber+1); ((i < firstUnitNumber2) && (i < MAX_NUM_OF_UNITS)); i++)
			unitsType[i]  = eUndefined;		// undefined in this version
		for (i = firstUnitNumber2; ((i <= lastUnitNumber2) && (i < MAX_NUM_OF_UNITS)); i++)
			unitsType[i]  = eDsp;			// DSP
		for (i = (lastUnitNumber2+1); i < MAX_NUM_OF_UNITS; i++)
			unitsType[i]  = eUndefined;		// undefined in this version
	}
	else
	{
	for (i = firstUnitNumber; ((i <= lastUnitNumber) && (i < MAX_NUM_OF_UNITS)); i++)
		unitsType[i]  = eDsp;			// DSP

	for (i = (lastUnitNumber+1); i < MAX_NUM_OF_UNITS; i++)
		unitsType[i]  = eUndefined;		// undefined in this version
}
}



/////////////////////////////////////////////////////////////////////////////
void CSimBarakUnitsList::OnMcmsCommandReq( CMplMcmsProtocol* pMplProtocol )
{
	if (NULL == pMplProtocol)
		return;

	// get unit and port
	DWORD unit = pMplProtocol->getPhysicalInfoHeaderUnit_id();
	DWORD port = pMplProtocol->getPhysicalInfoHeaderPort_id();
	DWORD opcode = pMplProtocol->getOpcode();
	APIU8 rsrcType = pMplProtocol->getPhysicalInfoHeaderResource_type();
	
	TRACEINTO	<< " CSimBarakUnitsList::OnMcmsCommandReq opcode:" << opcode << " unit:" << unit 
										<< " port:" << port << " rsrcType: " << (int)rsrcType;

	if (unit >= MAX_NUM_OF_UNITS)
	{
		PASSERT( unit );
		return;
	}

	if (NULL == m_unitsList[unit])
	{
		PTRACE(eLevelError,"CSimBarakUnitsList::OnMcmsCommandReq - unit not found");
		PASSERT(unit);
		return;
	}

	switch( opcode )
	{
		case TB_MSG_OPEN_PORT_REQ:	// open port (art, video)
		case TB_MSG_CLOSE_PORT_REQ: // close port (art, video)
		{
			if( rsrcType == ePhysical_art  || rsrcType == ePhysical_art_light )
			{
				// open art port
				if (m_unitsList[unit]->GetPortType() != (DWORD)eArt)
				{
					PTRACE(eLevelError,"CSimBarakUnitsList::OnMcmsCommandReq - unit not type ART");
					PASSERT(unit);
					return;
				}				
				
				TRACEINTO	<< " CSimBarakUnitsList::OnMcmsCommandReq rsrcType:" << (int)rsrcType;
				
				((CGideonSimUnitBarakMediaArt*)m_unitsList[unit])->OnMcmsCommandReq( pMplProtocol, port );
			}
			else if (rsrcType == ePhysical_video_encoder || rsrcType == ePhysical_video_decoder )
			{
				// open Video Decoder port
				if (m_unitsList[unit]->GetPortType() != (DWORD)eVideo)
				{
					PTRACE(eLevelError,"CSimBarakUnitsList::OnMcmsCommandReq - unit not type Video");
					PASSERT(unit);
					return;
				}
				((CGideonSimUnitBarakMediaVideo*)m_unitsList[unit])->OnMcmsCommandReq( pMplProtocol, port );
			}
			else
			{
				PASSERT( 100 );
				return;
			}
			break;
		}


		default:
			PTRACE(eLevelError,"CSimBarakUnitsList::OnMcmsCommandReq - unknown command");
			break;
	}
}


/////////////////////////////////////////////////////////////////////////////
int CSimBarakUnitsList::UnitsConfigReq( CMplMcmsProtocol* pMplProtocol )
{
	// gets the request struct
	CM_UNITS_CONFIG_S*  pUnits = (CM_UNITS_CONFIG_S*)(pMplProtocol->GetData());
	if (!pUnits)
		return STATUS_FAIL;

	// fill unit types
	int i = 0;
	int firstUnitNumber = 1;
	int lastUnitNumber = m_numOfUnits;

	if (9 == m_numOfUnits)	// in case of card type eMpmPlus_h (half carrier - 9 DSPs)
	{
		// DSP = 1,2,3,4,9,10,11,12,13
		firstUnitNumber 	 = 1;	// first unit is called "unit number 1"
		lastUnitNumber 		 = 4;	// last unit is called "unit number 4"
		int firstUnitNumber2 = 9;	// first unit is called "unit number 9"
		int lastUnitNumber2  = 13;	// last unit is called "unit number 13"

		for (i = firstUnitNumber; ((i <= lastUnitNumber) && (i < MAX_NUM_OF_UNITS)); i++)
		{
			if (pUnits->unitsParamsList[i].type == eArt || pUnits->unitsParamsList[i].type == eArtCntlr)
				m_unitsList[i] = (CGideonSimUnitBarakMedia*) new CGideonSimUnitBarakMediaArt( m_boardId, m_subBoardId );
			else  if (pUnits->unitsParamsList[i].type == eVideo)
				m_unitsList[i] = (CGideonSimUnitBarakMedia*) new CGideonSimUnitBarakMediaVideo( m_boardId, m_subBoardId );
			else
				break;	// end of units
			m_unitsList[i]->SetState( m_unitsState[i] ); // in case we want in the future to start with faulty unit
		}
		for (i = firstUnitNumber2; ((i <= lastUnitNumber2) && (i < MAX_NUM_OF_UNITS)); i++)
		{
			if (pUnits->unitsParamsList[i].type == eArt || pUnits->unitsParamsList[i].type == eArtCntlr)
				m_unitsList[i] = (CGideonSimUnitBarakMedia*) new CGideonSimUnitBarakMediaArt( m_boardId, m_subBoardId );
			else  if (pUnits->unitsParamsList[i].type == eVideo)
				m_unitsList[i] = (CGideonSimUnitBarakMedia*) new CGideonSimUnitBarakMediaVideo( m_boardId, m_subBoardId );
			else
				break;	// end of units
			m_unitsList[i]->SetState( m_unitsState[i] ); // in case we want in the future to start with faulty unit
		}
	}

	for (i = firstUnitNumber; ((i < MAX_NUM_OF_UNITS) && (i <= lastUnitNumber)); i++)
	{
		if (NULL == m_unitsList[i])
		{
			if (pUnits->unitsParamsList[i].type == eArt || pUnits->unitsParamsList[i].type == eArtCntlr)
				m_unitsList[i] = (CGideonSimUnitBarakMedia*) new CGideonSimUnitBarakMediaArt( m_boardId, m_subBoardId );
			else  if (pUnits->unitsParamsList[i].type == eVideo)
				m_unitsList[i] = (CGideonSimUnitBarakMedia*) new CGideonSimUnitBarakMediaVideo( m_boardId, m_subBoardId );
			else
				break;	// end of units
			m_unitsList[i]->SetState( m_unitsState[i] ); // in case we want in the future to start with faulty unit
		}
	}

	return STATUS_OK;
}




/////////////////////////////////////////////////////////////////////////////
int CSimBarakUnitsList::UpdateBondingParameters( DWORD unit, DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls)
{
	if (unit >= MAX_NUM_OF_UNITS)
	{
		PTRACE(eLevelError,"CSimBarakUnitsList::UpdateBondingParameters - illegal unit number");
		return -1;
	}

	if (NULL == m_unitsList[unit])
	{
		PTRACE(eLevelError,"CSimBarakUnitsList::UpdateBondingParameters - illegal unit NULL");
		return -1;
	}

	int status = m_unitsList[unit]->UpdateBondingParameters( port, muxConnectionID, confId, partyId, numOfChnls);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
int CSimBarakUnitsList::AddBondingChannel( CMplMcmsProtocol* pMplProtocol, BYTE *numOfChannels, BOOL *bBndAllChannelsConnected )
{
	DWORD unit = pMplProtocol->getPhysicalInfoHeaderUnit_id();
	DWORD port = pMplProtocol->getPhysicalInfoHeaderPort_id();

	if (unit >= MAX_NUM_OF_UNITS)
	{
		PTRACE(eLevelError,"CSimBarakUnitsList::AddBondingChannel - illegal unit number");
		return -1;
	}

	if (NULL == m_unitsList[unit])
	{
		PTRACE(eLevelError,"CSimBarakUnitsList::AddBondingChannel - illegal unit NULL");
		return -1;
	}

	int status = m_unitsList[unit]->AddBondingChannel( port, numOfChannels, bBndAllChannelsConnected );

	return status;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CSimBarakUnitsList::GetMuxConnectionId(WORD unitId, WORD portId)
{
	return m_unitsList[unitId]->GetMuxConnectionId(portId);
}


int CSimBarakUnitsList::ChangeUnitType(WORD unitId, APIU8 newUnitType)
{
	if (unitId >= MAX_NUM_OF_UNITS)
	{
		PTRACE(eLevelError,"CSimBarakUnitsList::ChangeUnitType - illegal unit number");
		return STATUS_FAIL;
	}
	
	if (NULL == m_unitsList[unitId])
	{
		PTRACE(eLevelError,"CSimBarakUnitsList::ChangeUnitType - illegal unit == NULL");
		return STATUS_FAIL;
	}
	
	if (m_unitsList[unitId]->GetPortType() == newUnitType)
	{
		PTRACE(eLevelError,"CSimBarakUnitsList::ChangeUnitType - illegal unit already in same type");
		return STATUS_FAIL;
	}
	
	if (newUnitType != eArt && newUnitType != eArtCntlr && newUnitType != eVideo)
	{
		PTRACE(eLevelError,"CSimBarakUnitsList::ChangeUnitType - illegal unit type");
		return STATUS_FAIL;
	}
	
	// delete unit
	POBJDELETE( m_unitsList[unitId] );
	
	// realloc unit
	if (newUnitType == eArt || newUnitType == eArtCntlr)
		m_unitsList[unitId] = (CGideonSimUnitBarakMedia*) new CGideonSimUnitBarakMediaArt( m_boardId, m_subBoardId );
	else  // eVideo
		m_unitsList[unitId] = (CGideonSimUnitBarakMedia*) new CGideonSimUnitBarakMediaVideo( m_boardId, m_subBoardId );

	m_unitsList[unitId]->SetState( m_unitsState[unitId] ); // in case we want in the future to start with faulty unit
	
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
//
//   CSimSwitchUnitsList - Switch Card Units List
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CSimSwitchUnitsList::CSimSwitchUnitsList( WORD boardId )
{

	memset(&m_switchSM_units.unSmComp1,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp2,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp3,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp4,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp5,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp6,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp7,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp8,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp9,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp10,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp11,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp12,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp13,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp14,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp15,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp16,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp17,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp18,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp19,0,sizeof(SM_COMPONENT_STATUS_S));
    memset(&m_switchSM_units.unSmComp20,0,sizeof(SM_COMPONENT_STATUS_S));

    m_switchSM_units.unSmComp1.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp2.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp3.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp4.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp5.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp6.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp7.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp8.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp9.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp10.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp11.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp12.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp13.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp14.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp15.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp16.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp17.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp18.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp19.unStatus = eSmComponentNotExist;
    m_switchSM_units.unSmComp20.unStatus = eSmComponentNotExist;
}

/////////////////////////////////////////////////////////////////////////////
CSimSwitchUnitsList::~CSimSwitchUnitsList()
{
}

/////////////////////////////////////////////////////////////////////////////
void CSimSwitchUnitsList::UpdateUnitStatus(SM_COMPONENT_STATUS_S & rswitch_keep_alive_ind_Struct,
                                           WORD unitNum,
                                           STATUS status )
{
	/*switch(unitNum){
		case 2 :{
			     rswitch_keep_alive_ind_Struct.unSlotId=2;
			     rswitch_keep_alive_ind_Struct.unStatus=eShmComponentMajor;
			     rswitch_keep_alive_ind_Struct.unStatus=0x111;
			     strncpy( (char*)(rswitch_keep_alive_ind_Struct.sSmCompName), "simulation", strlen("simulation") );

		        break;
		        }
		case 3 :{
    			rswitch_keep_alive_ind_Struct.unStatus=3;
			    rswitch_keep_alive_ind_Struct.unStatus=eShmComponentNotExist;
			   // rswitch_keep_alive_ind_Struct.unStatus=eOk;
			    //rswitch_keep_alive_ind_Struct.unStatus=eOk;
			    break;
				}
				*/
  /*  case 4 :{
    rswitch_keep_alive_ind_Struct.unSmComp4.unStatus=eOk;
    break;
    }

    case 5 :{
    rswitch_keep_alive_ind_Struct.unSmComp5.unStatus=eOk;
    break;
    }
    case 6 :{
    rswitch_keep_alive_ind_Struct.unSmComp6.unStatus=eOk;
    break;
    }
    case 7 :{
    rswitch_keep_alive_ind_Struct.unSmComp7.unStatus=eOk;
    break;
    }
    case 8 :{
    rswitch_keep_alive_ind_Struct.unSmComp8.unStatus=eOk;
    break;
    }
    case 9 :{
    rswitch_keep_alive_ind_Struct.unSmComp9.unStatus=eOk;
    break;
    }
    case 10 :{
    rswitch_keep_alive_ind_Struct.unSmComp10.unStatus=eOk;
    break;
    }
    case 11 :{
    rswitch_keep_alive_ind_Struct.unSmComp11.unStatus=eOk;
    break;
    }
    case 12 :{
    rswitch_keep_alive_ind_Struct.unSmComp12.unStatus=eOk;
    break;
    }
    case 13 :{
    rswitch_keep_alive_ind_Struct.unSmComp13.unStatus=eOk;
    break;
    }
    case 14 :{
    rswitch_keep_alive_ind_Struct.unSmComp14.unStatus=eOk;
    break;
    }
    case 15 :{
    rswitch_keep_alive_ind_Struct.unSmComp15.unStatus=eOk;
    break;
    }
    case 16 :{
    rswitch_keep_alive_ind_Struct.unSmComp16.unStatus=eOk;
    break;
    }
    case 17 :{
    rswitch_keep_alive_ind_Struct.unSmComp17.unStatus=eOk;
    break;
    }
    case 18 :{
    rswitch_keep_alive_ind_Struct.unSmComp18.unStatus=eOk;
    break;
    }
    case 19 :{
    rswitch_keep_alive_ind_Struct.unSmComp19.unStatus=eOk;
    break;
    }
    case 20 :{
    rswitch_keep_alive_ind_Struct.unSmComp20.unStatus=eOk;
    break;
    }*/
	//}

}

/*void UpdateComponentStatus( WORD unSlotId,WORD unStatus ,WORD unStatusDescriptionBitmask,APIU8  sSmCompName[SM_COMP_NAME_LEN] )
{
	APIU32 unSlotId;
	APIU32 unStatus;	// eSmComponentStatus (eSmComponentOk / eSmComponentMajor/ eSmComponentNotExist)
	APIU32 unStatusDescriptionBitmask;		// 0-------00000
											//			   |> Other
											//			  |>  Voltage
											//			 |>   Temperature major
											//          |>    Temperature critical
											//         |>     Failed (gone through a reset)
	APIU8  sSmCompName[SM_COMP_NAME_LEN];
}
*/

/////////////////////////////////////////////////////////////////////////////
//
//   CSimRtmSpan
//
/////////////////////////////////////////////////////////////////////////////
const BYTE SIM_MAX_PORTS_PER_T1_SPAN = 23+1;
const BYTE SIM_MAX_PORTS_PER_E1_SPAN = 30+1;

/////////////////////////////////////////////////////////////////////////////
CSimNetSpan::CSimNetSpan()
{
	m_numPorts = SIM_MAX_PORTS_PER_E1_SPAN;
	m_paPorts = new CSimNetPort* [m_numPorts];
	for( int i=0; i<m_numPorts; i++ )
		m_paPorts[i] = new CSimNetPort();

	memset(&m_rConfigStruct, 0, sizeof(RTM_ISDN_SPAN_CONFIG_REQ_S));
}

/////////////////////////////////////////////////////////////////////////////
CSimNetSpan::~CSimNetSpan()
{
	for( int i=0; i<m_numPorts; i++ )
		POBJDELETE(m_paPorts[i]);
	PDELETEA(m_paPorts);
}

/////////////////////////////////////////////////////////////////////////////
void CSimNetSpan::Configure(const RTM_ISDN_SPAN_CONFIG_REQ_S& rConfigStruct)
{
	PTRACE(eLevelInfoNormal,"CSimNetSpan::Configure - configuration request.");

	memcpy(&m_rConfigStruct,&rConfigStruct,sizeof(RTM_ISDN_SPAN_CONFIG_REQ_S));

		// clean old ports array
	for( int i=0; i<m_numPorts; i++ )
		POBJDELETE(m_paPorts[i]);
	PDELETEA(m_paPorts);
		// number of ports
	if( PCM_MODE_T1 == m_rConfigStruct.unit_type )
		m_numPorts = SIM_MAX_PORTS_PER_T1_SPAN;
	else if( PCM_MODE_E1 == m_rConfigStruct.unit_type )
		m_numPorts = SIM_MAX_PORTS_PER_E1_SPAN;
	else
	{
		m_numPorts = 0;
		PASSERT(5000);
	}
		// create new ports array
	m_paPorts = new CSimNetPort* [m_numPorts];
	for( int i=0; i<m_numPorts; i++ )
		m_paPorts[i] = new CSimNetPort();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CSimNetSpan::IsPortValid(const BYTE portId) const
{
	if( portId >= m_numPorts || portId == 0 )
		return FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CSimNetSpan::IsVirtPortInUse(const BYTE virtPort) const
{
	for( int i=1; i<m_numPorts; i++ )
		if( m_paPorts[i]->GetVirtualPortNum() == virtPort )
			return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSimNetSpan::FindPortByConfPartyId(const DWORD confId, const DWORD partId, DWORD opcode) const
{
	for( int i=1; i<m_numPorts; i++ )
	{
		if (NET_DISCONNECT_ACK_REQ == opcode)
		{
			if (m_paPorts[i]->GetPartyId() == partId )
				return i;
		}
		else
		{
		if( m_paPorts[i]->GetConfId() == confId && m_paPorts[i]->GetPartyId() == partId )
			return i;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void CSimNetSpan::UpdatePortConfPartyId(const BYTE portId, const DWORD confId, const DWORD partId)
{
	if( portId < m_numPorts )
		m_paPorts[portId]->SetConfPartyId(confId,partId);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CSimNetSpan::IsPortInUseByConfParty(const DWORD portId, const DWORD confId, const DWORD partyId, DWORD opcode) const
{
	if( portId<m_numPorts ) {
		if ((NET_DISCONNECT_IND == opcode) || (NET_DISCONNECT_ACK_IND == opcode))
		{
			if( m_paPorts[portId]->GetPartyId() == partyId )
				return TRUE;
		}
		else
		{
		if( m_paPorts[portId]->GetConfId() == confId && m_paPorts[portId]->GetPartyId() == partyId )
			return TRUE;
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSimNetSpan::AllocatePort(const DWORD virtPortId, const DWORD confId, const DWORD partId, const DWORD channelConnectionId)
{
	for( int i=1; i<m_numPorts; i++ )
	{
		if( m_paPorts[i]->IsEmpty() && m_paPorts[i]->IsReadyToCall() )
		{
			m_paPorts[i]->SetVirtualPortNum(virtPortId);
			m_paPorts[i]->SetConfPartyId(confId, partId);
			m_paPorts[i]->SetChannelConnectionId(channelConnectionId);

			TRACEINTO << "CSimNetSpan::AllocatePort Port=" << i << " channelConnectionId=" << channelConnectionId << " conf=" << confId << " partyId=" << partId ;

			return i;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSimNetSpan::DeAllocatePort(const DWORD portId)
{
	if( portId < m_numPorts )
	{
		m_paPorts[portId]->CleanVirtualPortNum();
		m_paPorts[portId]->CleanConfPartyId();
		m_paPorts[portId]->SetChannelConnectionId(0);

		TRACEINTO << "CSimNetSpan::DeAllocatePort Port=" << portId;

		return STATUS_OK;
	}
	return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CSimNetSpan::GetVirtualPortNum(const DWORD portNum) const
{
	if( portNum < m_numPorts )
		return m_paPorts[portNum]->GetVirtualPortNum();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void CSimNetSpan::FillStatusStruct(RTM_ISDN_SPAN_STATUS_IND_S* pStatusStruct) const
{
	pStatusStruct->alarm_status = NO_ALARM;
	pStatusStruct->d_chnl_status = (m_rConfigStruct.d_chanl_needed == YES)?
				D_CHANNEL_ESTABLISHED : D_CHANNEL_NOT_ESTABLISHED;
	pStatusStruct->clocking_status = TO_MASTER;
}

/////////////////////////////////////////////////////////////////////////////
void CSimNetSpan::FillStatusStructBad( RTM_ISDN_SPAN_STATUS_IND_S* pStatusStruct,
	                                   ENetSpanStatus spanStatus,
	                                   bool isDChannelEstablished) const
{
	pStatusStruct->alarm_status = NO_ALARM;
	if (eNetSpanStatusRedAlarm == spanStatus)
	{
		pStatusStruct->alarm_status = RED_ALARM;
	}
	else if (eNetSpanStatusYellowAlarm == spanStatus)
	{
		pStatusStruct->alarm_status = YELLOW_ALARM;
	}

	pStatusStruct->d_chnl_status = (true == isDChannelEstablished)?
				D_CHANNEL_ESTABLISHED : D_CHANNEL_NOT_ESTABLISHED;

	pStatusStruct->clocking_status = TO_MASTER;
}


/////////////////////////////////////////////////////////////////////////////
BYTE CSimNetSpan::GetPortByConnectionId(const DWORD channelConnectionId) const
{
	for( int i=1; i<m_numPorts; i++ )
		if( m_paPorts[i]->GetChannelConnectionId() == channelConnectionId )
			return i;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSimNetSpan::DeAllocatePortByChannelConnectionId(const DWORD channelConnectionId)
{
	for( int i=1; i<m_numPorts; i++ )
	{
		if( m_paPorts[i]->GetChannelConnectionId() == channelConnectionId )
		{
			return DeAllocatePort(i);
		}
	}

	return STATUS_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
//
//   CSimNetPort
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CSimNetPort::CSimNetPort()
{
	m_status = eNetPortStatusNormal;
	CleanVirtualPortNum();
	CleanConfPartyId();
	m_channelConnectionId = 0;
}

/////////////////////////////////////////////////////////////////////////////
CSimNetPort::~CSimNetPort()
{
}

/////////////////////////////////////////////////////////////////////////////
void CSimNetPort::SetConfPartyId(const DWORD confId, const DWORD partId)
{
	m_confId  = confId;
	m_partyId = partId;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CSimNetPort::IsEmpty() const
{
	if( 0xFF == m_virtualPort && 0xFFFFFFFF == m_confId && 0xFFFFFFFF == m_partyId )
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CSimNetPort::IsReadyToCall() const
{
	if( eNetPortStatusNormal == m_status ||
		eNetPortStatusRedAlarm == m_status ||
		eNetPortStatusYellowAlarm == m_status )
			return TRUE;
	return FALSE;
}
