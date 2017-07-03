
#ifndef _CBridgePartyVideoRelayMediaParams_H_
#define _CBridgePartyVideoRelayMediaParams_H_

#include <list>
#include "BridgePartyMediaParams.h"
#include "RelayMediaStream.h"
#include "DwordBitMask.h"
#include "VideoRelayInOutMediaStream.h"

class CBridgePartyVideoRelayMediaParams : public CBridgePartyMediaParams
{
public:
	CBridgePartyVideoRelayMediaParams ();
	CBridgePartyVideoRelayMediaParams (const CBridgePartyVideoRelayMediaParams& rOther);
	virtual ~CBridgePartyVideoRelayMediaParams ();

	CBridgePartyVideoRelayMediaParams& operator = (const CBridgePartyVideoRelayMediaParams& rOther);

	virtual const char* NameOf() const { return "CBridgePartyVideoRelayMediaParams";}
	virtual BOOL  IsValidParams() const ;

	void ClearStreamsList();

	void	SetChannelHandle(DWORD handle) {m_channelHandle = handle;}
	void	SetIsReady(BOOL bIsReady) {m_bIsReady = bIsReady;}
	void	SetScpRequestSequenceNumber(DWORD num) {m_scpRequestSequenceNumber = num;}
	void	SetMuteMask(CDwordBitMask& mute_mask) {m_mute_mask = mute_mask;}
	void	SetSupportSvcAvcTranslate(bool bIsSupportSvcAvcTranslate) {m_bIsSupportSvcAvcTranslate = bIsSupportSvcAvcTranslate;}
	void    SetPipeIdForIvrSlide(DWORD ssrc) {m_pipeIdForIvrSlide = ssrc;}
	void    SetIsCascadeLink(bool isCascade) {m_bIsCascadeLink = isCascade;}

	DWORD			               GetChannelHandle()const { return m_channelHandle; }
	BOOL			               GetIsReady() { return m_bIsReady; }
	DWORD			               GetScpRequestSequenceNumber() { return m_scpRequestSequenceNumber; }
	CDwordBitMask	               GetMuteMask() { return m_mute_mask; }
	bool			               GetSupportSvcAvcTranslate() { return m_bIsSupportSvcAvcTranslate; }
	std::list<CRelayMediaStream*>  GetRelayMediaStream(){return 	m_pStreamsList;}
	DWORD                          GetPipeIdForIvrSlide() {return m_pipeIdForIvrSlide;}
	bool                           GetIsCascadeLink() {return m_bIsCascadeLink;}





	std::list <CRelayMediaStream*>	m_pStreamsList;
	DWORD							m_channelHandle;
	BOOL							m_bIsReady;
	DWORD							m_scpRequestSequenceNumber;
	CDwordBitMask					m_mute_mask;
	bool							m_bIsSupportSvcAvcTranslate;
	DWORD                           m_pipeIdForIvrSlide;
	bool                            m_bIsCascadeLink;

};


#endif
