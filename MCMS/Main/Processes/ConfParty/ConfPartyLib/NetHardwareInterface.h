/*$Header:   M:/SCM/m3cv1d1/subsys/mcms/RSRC.H_v   1.0   08 Jul 1997 17:15:56   CARMI  $*/
// rsrc.h : interface of the CRsrc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _NET_HARDWARE_INTERFACE_H
#define _NET_HARDWARE_INTERFACE_H

#include "HardwareInterface.h"
#include "NetSetup.h"
#include "NetCause.h"
#include "OpcodesMcmsNetQ931.h"
#include "IpChannelParams.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "Q931Structs.h"


class CIsdnNetSetup;

class CNetHardwareInterface : public CHardwareInterface
{
	CLASS_TYPE_1(CNetHardwareInterface, CHardwareInterface)
public:
	// Constructors
	CNetHardwareInterface();
	virtual ~CNetHardwareInterface();  
	virtual const char* NameOf() const { return "CNetHardwareInterface";}
	void	Create(CRsrcParams& rRsrcParams);
	void    Setup(CIsdnNetSetup & netSetup);  
	void    Clear(CNetCause& cause,const CIsdnNetSetup & netSetup);	   //Starts a disconnection session.
	void    DisconnectReq(CNetCause& cause,const CIsdnNetSetup & netSetup);
	void    SendMsgToMPL(BYTE* pStructure, int structureSize, DWORD opcode,const CIsdnNetSetup & netSetup);
	void    SendMsgWithPhysicalInfo();
	void    ConnectNetToAudioMux();
	void    ConnectAudioToNet();
	void    ConnectMuxToNet();
	void    DisconnectNetFromAudio(cmCapDirection direction);
	void    DisconnectAudioFromNet(cmCapDirection direction);
	void    DisconnectMuxFromNet(cmCapDirection direction);
	void    ClosePort(kChanneltype channelType,cmCapDirection direction);
	
	void SendOpenPort(kChanneltype channelType,cmCapDirection direction,DWORD connectionId);
	void SendClosePort(kChanneltype channelType,cmCapDirection direction,DWORD connectionId);
	STATUS   GetAckIndParams(kChanneltype& channelType, cmCapDirection& direction, APIU32& ackReason, CSegment* pParam);
	//WORD    GetNfasId() const;
	//WORD    GetNumPorts() const;
	//WORD    GetFirstPort() const;
	//void    SetNfasId(WORD);
	//void    SetNumPorts(WORD);
	//void    SetFirstPort(WORD);
	
        void    Alert(const CIsdnNetSetup & netSetup);
	void    Connect(const CIsdnNetSetup & netSetup);
	
	//      Operations

	APIU32 GetNumTypeAsApiVal(eDfltNumType numType)const;
	APIU32 GetNumPlanAsApiVal(eNumPlanType numPlan)const;
	void FillNetHeader(BYTE* pStructure, int structureSize,const CIsdnNetSetup & netSetup);
	void FillNetSetupHeader(BYTE* pStructure, int structureSize,const CIsdnNetSetup & netSetup);
	void SendMsgWithPhysicalInfo(OPCODE opcode,CRsrcParams & rsrcParams,
				     const CIsdnNetSetup & netSetup, WORD discause=causDEFAULT_VAL);
	  protected:
	//Resources ID
	DWORD m_netConnId     ;
	DWORD m_audioDecoderConnId ;
	DWORD m_audioEncoderConnId ;
	//WORD    m_numPorts; 
 	//WORD    m_firstPort; 
 	//WORD    m_netConnectionId; 
 	//WORD    m_nfasId; 
		
	// Operatios	
};

#endif /* _NETINTERFACE_H  */
