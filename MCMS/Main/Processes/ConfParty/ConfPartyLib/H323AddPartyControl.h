//+========================================================================+
//                            H323AddPartyControl.H                        |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323AddPartyControl.H                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 7/4/95     |                                                      |
//+========================================================================+
#ifndef _H323AddPartyControl
#define _H323AddPartyControl


// compile time flags are:
// __PARTY_CONTROL_SIMULATION__ to chose party control simulation or the real flow with H323dial out party.


#include "H323PartyControl.h"

#include "IpPartyControl.h"

class CH323AddPartyCntl :public CH323PartyCntl
{
	CLASS_TYPE_1(CH323AddPartyCntl,CH323PartyCntl )
public:
    // Constructors
	CH323AddPartyCntl();
	virtual ~CH323AddPartyCntl();
	
	// Initializations
	CH323AddPartyCntl& operator=(const CH323AddPartyCntl& other);
	CH323AddPartyCntl& operator =(const CH323PartyCntl& other);
	
	void  Create(PartyControlInitParameters & partyControInitParam,PartyControlDataParameters & partyControlDataParams);

	void  OnPartyH323ConnectChairCntlSetup(CSegment* pParam);
	void  Reconnect(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	void  DialOut(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	void  OnAudConnectParty(CSegment* pParam);
	void  ContinueAddPartyAfterMove();

  	// Operations
	const virtual char*  NameOf() const;
	virtual void*  GetMessageMap();
    virtual void   ChangeScm(CIpComMode* pH323Scm, EChangeMediaType eChangeMediaType);

    virtual void   PartyH323ConnectAllPartyConnectAudioOrChangeAll(CSegment* pParam);
    virtual void   OnPartyH323ConnectAllPartyReallocateRsrc(CSegment* pParam);

    //Multiple links for ITP in cascaded conference feature:
    virtual void  OnAddSubLinksParties(CSegment* pParam);

	// Action Functions
	void  OnPartyH323ConnectSetup(CSegment* pParam);
	void  OnPartyH323ConnectAllPartyConnectAudio(CSegment* pParam);
	void  OnPartyH323ConnectDelayIdle(CSegment* pParam);
	void  OnPartyH323ConnectDelayAllocRsc(CSegment* pParam);
	void  PartyH323ConnectDelay(CSegment* pParam);
	//  	void  NewScmForGatewayCall(CComMode* pScm,DWORD confRate,DWORD vid_bitrate,DWORD tdm_bitRate,WORD IsLinkToRemoteGW,CCapH263* pH263VideoSwitchCap);
//	void  OnPartyAddProtocolToH323Cap(CSegment* pParam);
	void  OnPartyRemoveProtocolFromH323Cap(CSegment* pParam);
	virtual void  OnPartyUpdateLocalCaps(CSegment* pParam);
//	void  OnPartySetNewRatesForVswCall(CSegment* pParam);
//	void  OnPartySetFlowControlPartyAndSendToConf(CSegment* pParam);
  	void  OnPartyH323SetSecondaryCause(CSegment* pParam);
	void  OpenDataChannel(WORD bitRate,WORD type);
	void  OnPartyMuteVideo(CSegment* pParam);
	void  OnRsrcAllocatePartyRspAllocate(CSegment* pParam);
	//void  OnRsrcReAllocatePartyRspReAllocate(CSegment* pParam);
	//void  OnRsrcReAllocatePartyRspChangeAll(CSegment* pParam);
	void  OnMplAckCreate(CSegment* pParam);

	void OnConnectToutAllocate(CSegment* pParam);
	void OnConnectToutReAllocate(CSegment* pParam);
	void OnConnectToutCreate(CSegment* pParam);
	void OnConnectToutPartySetup(CSegment* pParam);
	void OnConnectToutPartyConnectAudio(CSegment* pParam);

	void OnTimerWaitForRsrcAndAskAgainAllocate(CSegment* pParam);

    void OnPartyReceivedReCapsChangeAll(CSegment* pParam);

	virtual BYTE IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo) const;
	virtual BYTE IsRemoteAndLocalHasEPCContentOnly();

protected:
	// Operations
	WORD  GetHalfKeyType() const;
	void  EstablishH323Call();
    void  ChangePartyParamsAfterMove(DWORD newVidRate, DWORD newTdmVidRate, CComModeH323& rH323Scm);
	void  StartPartyConnection();
	void  AllocatePartyResources();
	void  InitUdpAddresses(CSegment* pParam);
	void  DumpUdpAddresses();
	virtual void  EndConnectionProcess(WORD status);
	
	void SetH323PartyCapsAndVideoParam(CIpComMode* pPartyScm, CCapH323* pH323Cap, CConfParty* pConfParty, DWORD vidBitrate, WORD dialType, DWORD setupRate, DWORD confRate, BYTE isRecordingLink,PartyControlDataParameters&partyControlDataParams);
	void SetupH323ConParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams );
	void SetupCallParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	void  SetupH323PartyTaskEntryPoint();
	void  ResetRedialParameters();
	CCapH323* NewAndSetupPartyCaps(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams,CIpComMode* pPartyScm,DWORD setupRate);
	void SetupTIPLinkInfo(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	void  ConnectH323(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	CH323NetSetup* CreateAndSetupH323NetSetup(CConfParty* pConfParty,CIpComMode*pPartyScm, PartyControlInitParameters& partyControlInitParam,PartyControlDataParameters &partyControlDataParams);	
	void SetMaxRateForIpNetSetup(CConfParty* pConfParty,CIpComMode*pPartyScm, PartyControlInitParameters& partyControlInitParam,CH323NetSetup* pIpNetSetup);
//	UdpAddresses   m_udpAddresses;
//	CH323NetSetup* m_pH323NetSetup;

	PDECLAR_MESSAGE_MAP
};

#endif //_H323AddPartyControl

