/*
 * MSSlaveSipPartyCntl.h
 *
 *  Created on: Sep 23 , 2013
 *      Author: Flora Yao
 */

#ifndef MSSLAVESIPADDPARTYCONTROLADD_H_
#define MSSLAVESIPADDPARTYCONTROLADD_H_

#include "SIPPartyControlAdd.h"

#define MSFT_SVC_MAX_VIDEO_SSRC_FOR_SLAVE 10

class CMSSlaveSipAddPartyCntl: public CSipAddPartyCntl
{
CLASS_TYPE_1(CMSSlaveSipAddPartyCntl, CSipAddPartyCntl)

public:
	
	CMSSlaveSipAddPartyCntl();
	virtual ~CMSSlaveSipAddPartyCntl();
	void OnTimerConnectDelay(CSegment* pParam);
	virtual const char* NameOf() const {return "CMSSlaveSipAddPartyCntl";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	
	void Create(PartyControlInitParameters&  partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	DWORD UpdateCapsWithMsftResources(MS_SSRC_PARTY_IND_PARAMS_S  &msSsrcParams);
	void OnPartyRsrcAllocatingRsp(CSegment* pParam);
	void  OnPartyMPLCreatingRsp(CSegment* pParam);
	void EstablishCall(BYTE eTransportType);
	void OnPartyChannelsConnectedSetup(CSegment* pParam); // channels connected or updated
	void ConnectBridgesSetup(); // channels connected or updated
	void  OnAudConnectPartyConnectAudio(CSegment* pParam);
	void OnPartyRemoteConnected(CSegment* pParam);
	void OffererPartyConnected(CSipComMode* pRemoteMode);
	void OnPartyChannelsConnectedConnecting(CSegment* pParam); // channels connected or updated
	void OnVideoBrdgConnected(CSegment* pParam);
	void SetupMSSlaveSlaveSipConParameters(PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	CIpComMode* NewAndGetPartyCntlScm(PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams );
	CSipNetSetup* NewAndSetupSipNetSetup(CConfParty* pConfParty,
					PartyControlInitParameters& partyControInitParam,
					PartyControlDataParameters &partyControlDataParams);
	CSipCaps* NewAndGetLocalSipCaps(CIpComMode* pPartyScm,CConfParty* pConfParty,
					CSipNetSetup* pIpNetSetup,DWORD& vidBitrate,
					PartyControlInitParameters& partyControInitParam, 
					PartyControlDataParameters& partyControlDataParams);
	void SetIceForConfParty(CConfParty* pConfParty);

	DWORD GetMSSlaveSsrcRangeStart() { return m_MSSlaveSsrcRangeStart; }
	void SendSingleUpdatePacsiInfoToSlavesController(const MsSvcParamsStruct& pacsiInfo, BYTE isReasonFecOrRed=FALSE);
protected:

	void StartPartyConnection();
	void AllocatePartyResources();
	PDECLAR_MESSAGE_MAP;

	eVideoPartyType m_VideoPartyType;

	DWORD	m_MSSlaveSsrcRangeStart;	
	BYTE m_SentAckToMaster;
};



#endif /* SIPSLAVEPARTYCONTROL_H_ */

