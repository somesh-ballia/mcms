/*
 * SIPSlavePartyControl.h
 *
 *  Created on: Feb 14, 2011
 *      Author: mvolovik
 */

#ifndef SIPSLAVEPARTYCONTROL_H_
#define SIPSLAVEPARTYCONTROL_H_
#include "SIPInternals.h"

class CSipSlavePartyCntl: public CSipPartyCntl
{
CLASS_TYPE_1(CSipSlavePartyCntl, CSipPartyCntl)

public:
	CSipSlavePartyCntl();
	virtual ~CSipSlavePartyCntl();

	void Create(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	virtual const char* NameOf() const {return "CSipSlavePartyCntl";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	void OnPartyRsrcAllocatingRsp(CSegment* pParam);
	void OnPartyMPLCreatingRsp(CSegment* pParam);
	void EstablishCall(BYTE eTransportType);
	void ConnectBridgesSetup();
	void OnPartyChannelsConnectedSetup(CSegment* pParam);
	void OnAudConnectPartyConnectAudio(CSegment* pParam);
	void OnPartyRemoteConnected(CSegment* pParam);
	void OffererPartyConnected(CSipComMode* pRemoteMode);
	void OnPartyChannelsConnectedConnecting(CSegment* pParam); // channels connected or updated
	void ChangeScm(CIpComMode* pIpScm);
	void OnVideoBrdgConnected(CSegment* pParam);

protected:

	void SetSlaveTelepresenseEPInfo(); //_e_m_
	void StartPartyConnection();
	void AllocatePartyResources();
	void SetupSIPSlaveConParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	virtual CIpComMode*   NewAndGetPartyCntlScm(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams );
	virtual CSipNetSetup* NewAndSetupSipNetSetup(CConfParty* pConfParty, CIpComMode*pPartyScm, PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	virtual CSipCaps*     NewAndGetLocalSipCaps(CIpComMode* pPartyScm,CConfParty* pConfParty,CSipNetSetup* pIpNetSetup,DWORD& vidBitrate,PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams);
	PDECLAR_MESSAGE_MAP;

	eVideoPartyType m_VideoPartyType;
	BYTE m_SentAckToMaster;
};



#endif /* SIPSLAVEPARTYCONTROL_H_ */
