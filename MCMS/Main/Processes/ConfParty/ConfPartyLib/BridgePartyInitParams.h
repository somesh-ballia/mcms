#ifndef _CBridgePartyInitParams_H_
#define _CBridgePartyInitParams_H_

#include "PObject.h"
#include "Conf.h"
#include "ConfPartyApiDefines.h"
#include "ConfAppBridgeParams.h"

class CBridge;
class CBridgePartyMediaParams;
class CConfAppBridgeParams;
class CBridgePartyCntl;
class CSiteNameInfo;

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyInitParams
////////////////////////////////////////////////////////////////////////////
class CBridgePartyInitParams : public CPObject
{
  CLASS_TYPE_1(CBridgePartyInitParams, CPObject)

public:
	CBridgePartyInitParams();
	CBridgePartyInitParams(
					const char* pPartyName,
					const CTaskApp* pParty,
					const PartyRsrcID partyRsrcID,
					const WORD partyRoomId,
					const WORD wNetworkInterface,
					const CBridgePartyMediaParams* pMediaInParams = NULL,
					const CBridgePartyMediaParams* pMediaOutParams = NULL,
					const CBridgePartyCntl* pBridgePartyCntl = NULL,
					const char* pSiteName = "",
					const BYTE bCascadeLinkMode = NONE,
					const BOOL isVideoRelay = false,
					const BOOL isUseSpeakerSsrcForTx=FALSE,
					const PartyRsrcID MS_masterPartyRsrcID = 0,
					const DWORD MSaudioLocalMsi = 0,
					const BYTE MsAvMcuIndex= 0);

	CBridgePartyInitParams(const CBridgePartyInitParams& rOtherBridgePartyInitParams);

	virtual                       ~CBridgePartyInitParams();
	virtual const char*            NameOf() const { return "CBridgePartyInitParams";}
	CBridgePartyInitParams&        operator =(const CBridgePartyInitParams& rOtherBridgePartyInitParams);

	virtual BOOL                   IsValidParams() const;
	virtual BOOL                   IsIvrInConf() const;
	virtual BOOL                   IsValidCascadeLinkMode() const;
	virtual BOOL                   IsValidCopParams() const;
	virtual BOOL                   IsValidXCodeParams() const;

	const char*                    GetPartyName() const                                  {return m_pPartyName;}
	const char*                    GetConfName() const                                   {return m_pConfName;}
	const CBridge*                 GetBridge() const                                     {return m_pBridge;}
	const CTaskApp*                GetParty() const                                      {return m_pParty;}
	const CConf*                   GetConf() const                                       {return m_pConf;}
	PartyRsrcID                    GetPartyRsrcID() const                                {return m_partyRsrcID;}
	ConfRsrcID                     GetConfRsrcID() const                                 {return m_confRsrcID;}
	WORD                           GetNetworkInterface() const                           {return m_wNetworkInterface;}
	const CBridgePartyCntl*        GetPartyCntl() const                                  {return m_pBridgePartyCntl;}
	const CBridgePartyMediaParams* GetMediaInParams() const                              {return m_pMediaInParams;}
	const CBridgePartyMediaParams* GetMediaOutParams() const                             {return m_pMediaOutParams;}
	const CConfAppBridgeParams*    GetConfAppParams() const                              {return &m_confAppParams;}
	const char*                    GetSiteName() const                                   {return m_pSiteName;}
	BYTE                           GetCascadeLinkMode() const                            {return m_bCascadeLinkMode;}
	WORD                           GetPartyRoomID() const                                {return m_partyRoomId;}
	const CSiteNameInfo*           GetSiteNameInfo() const                               {return m_pSiteNameInfo;}
	BOOL                           GetIsVideoRelay() const                               {return m_bIsVideoRelay;}
	BOOL                           GetUseSpeakerSsrcForTx() const                        {return m_bUseSpeakerSsrcForTx;}
	PartyRsrcID                    GetMsMasterPartyRsrcId() const                        {return m_MS_masterPartyRsrcID;}
	DWORD                          GetMsAudioLocalMsi() const                            {return m_MSaudioLocalMsi;}
	BYTE                           GetMsAvMcuIndex() const                               {return m_MsAvMcuIndex;}

	void                           SetConfAppParams(CConfAppBridgeParams& confAppParams) {m_confAppParams = confAppParams;}
	void                           SetBridge(const CBridge* pBridge)                     {m_pBridge = (CBridge*)pBridge;}
	void                           SetConfName(char* pConfName)                          {m_pConfName = pConfName;}
	void                           SetConf(CConf* pConf)                                 {m_pConf = pConf;}
	void                           SetConfRsrcID(ConfRsrcID confRsrcID)                  {m_confRsrcID = confRsrcID;}
	void                           SetSiteNameInfo(CSiteNameInfo* pSiteNameInfo)         {m_pSiteNameInfo = pSiteNameInfo;}
	void                           SetIsVideoRelay(BOOL bIsVideoRelay)                   {m_bIsVideoRelay = bIsVideoRelay;}
	void                           SetUseSpeakerSsrcForTx(BOOL bUseSpeakerSsrcForTx)     {m_bUseSpeakerSsrcForTx = bUseSpeakerSsrcForTx;}
	void                           SetMsMasterPartyRsrcId(PartyRsrcID partyId)           {m_MS_masterPartyRsrcID = partyId;}
	void                           SetMsAudioLocalMsi(DWORD MSaudioLocalMsi)             {m_MSaudioLocalMsi = MSaudioLocalMsi;}
	void                           SetMsAvMcuIndex(BYTE MsAvMcuIndex)                    {m_MsAvMcuIndex = MsAvMcuIndex;}

	virtual void                   Serialize(WORD format, CSegment& seg) const;
	virtual void                   DeSerialize(WORD format, CSegment& seg);

protected:
	const char* const              m_pPartyName;
	const char*                    m_pConfName;
	const CTaskApp* const          m_pParty;
	CConf*                         m_pConf;
	CBridge*                       m_pBridge;
	PartyRsrcID                    m_partyRsrcID;
	ConfRsrcID                     m_confRsrcID;
	const WORD                     m_wNetworkInterface;
	const CBridgePartyMediaParams* m_pMediaInParams;
	const CBridgePartyMediaParams* m_pMediaOutParams;
	CConfAppBridgeParams           m_confAppParams;
	const CBridgePartyCntl*        m_pBridgePartyCntl; // for Move
	const char*                    m_pSiteName;
	BYTE                           m_bCascadeLinkMode;  // CASCADE_MODE_MASTER , CASCADE_MODE_SLAVE , CASCADE_MODE_NONE
	WORD                           m_partyRoomId;
	CSiteNameInfo*                 m_pSiteNameInfo;
	BOOL                           m_bIsVideoRelay;
	BOOL                           m_bUseSpeakerSsrcForTx;
	PartyRsrcID                    m_MS_masterPartyRsrcID;
	DWORD                          m_MSaudioLocalMsi;
	BYTE                           m_MsAvMcuIndex;
};

#endif // ifndef _CBridgePartyInitParams_H_

