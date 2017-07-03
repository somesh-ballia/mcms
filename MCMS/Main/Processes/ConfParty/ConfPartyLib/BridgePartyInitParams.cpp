#include "BridgePartyInitParams.h"
#include "ConfPartyGlobals.h"
#include "Bridge.h"
#include "ConfAppBridgeParams.h"
#include "BridgePartyCntl.h"
#include "HostCommonDefinitions.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyInitParams
////////////////////////////////////////////////////////////////////////////
CBridgePartyInitParams::CBridgePartyInitParams () :
  m_pPartyName(NULL),
  m_pConfName(NULL),
  m_pParty(NULL),
  m_pConf(NULL),
  m_pBridge(NULL),
  m_partyRsrcID(0),
  m_confRsrcID(0),
  m_wNetworkInterface(AUTO_INTERFACE_TYPE),
  m_pMediaInParams(NULL),
  m_pMediaOutParams(NULL),
  m_pBridgePartyCntl(NULL),
  m_pSiteName(NULL),
  m_bCascadeLinkMode(NONE),
  m_partyRoomId(0xFFFF),
  m_pSiteNameInfo(NULL),
  m_bIsVideoRelay(false),
  m_bUseSpeakerSsrcForTx(FALSE),
  m_MS_masterPartyRsrcID(0),
  m_MSaudioLocalMsi(0),
  m_MsAvMcuIndex(0)
{
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyInitParams::CBridgePartyInitParams (const char* pPartyName, const CTaskApp* pParty,
                                                const PartyRsrcID partyRsrcID,
                                                const WORD partyRoomId,
                                                const WORD wNetworkInterface,
                                                const CBridgePartyMediaParams* pMediaInParams,
                                                const CBridgePartyMediaParams* pMediaOutParams,
                                                const CBridgePartyCntl* pBridgePartyCntl,
                                                const char* pSiteName,
                                                const BYTE bCascadeLinkMode,
                                                const BOOL isVideoRelay/*=false*/,
                                                const BOOL isUseSpeakerSsrcForTx/*=FALSE*/,
                                                const PartyRsrcID MS_masterPartyRsrcID,
                                                const DWORD MSaudioLocalMsi,
                                                const BYTE MsAvMcuIndex) :

  m_pPartyName(pPartyName),
  m_pConfName(NULL),
  m_pParty(pParty),
  m_pConf(NULL),
  m_pBridge(NULL),
  m_partyRsrcID(partyRsrcID),
  m_wNetworkInterface(wNetworkInterface),
  m_pMediaInParams(pMediaInParams),
  m_pMediaOutParams(pMediaOutParams),
  m_pBridgePartyCntl(pBridgePartyCntl),
  m_pSiteName(pSiteName),
  m_bCascadeLinkMode(bCascadeLinkMode),
  m_partyRoomId(partyRoomId),
  m_pSiteNameInfo(NULL),
  m_bIsVideoRelay(isVideoRelay),
  m_bUseSpeakerSsrcForTx(isUseSpeakerSsrcForTx),
  m_MS_masterPartyRsrcID(MS_masterPartyRsrcID),
  m_MSaudioLocalMsi(MSaudioLocalMsi),
  m_MsAvMcuIndex(MsAvMcuIndex)
{
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyInitParams::CBridgePartyInitParams (const CBridgePartyInitParams& rOtherBridgePartyInitParams)
  : CPObject(rOtherBridgePartyInitParams),
  m_pPartyName(rOtherBridgePartyInitParams.m_pPartyName),
  m_pConfName(rOtherBridgePartyInitParams.m_pConfName),
  m_pParty(rOtherBridgePartyInitParams.m_pParty),
  m_pConf(rOtherBridgePartyInitParams.m_pConf),
  m_pBridge(rOtherBridgePartyInitParams.m_pBridge),
  m_partyRsrcID(rOtherBridgePartyInitParams.m_partyRsrcID),
  m_confRsrcID(rOtherBridgePartyInitParams.m_confRsrcID),
  m_wNetworkInterface(rOtherBridgePartyInitParams.m_wNetworkInterface),
  m_pMediaInParams(rOtherBridgePartyInitParams.m_pMediaInParams),
  m_pMediaOutParams(rOtherBridgePartyInitParams.m_pMediaOutParams),
  m_confAppParams(rOtherBridgePartyInitParams.m_confAppParams),
  m_pBridgePartyCntl(rOtherBridgePartyInitParams.m_pBridgePartyCntl),
  m_pSiteName(rOtherBridgePartyInitParams.m_pSiteName),
  m_bCascadeLinkMode(rOtherBridgePartyInitParams.m_bCascadeLinkMode),
  m_partyRoomId(rOtherBridgePartyInitParams.m_partyRoomId),
  m_pSiteNameInfo(rOtherBridgePartyInitParams.m_pSiteNameInfo),
  m_bIsVideoRelay(rOtherBridgePartyInitParams.m_bIsVideoRelay),
  m_bUseSpeakerSsrcForTx(rOtherBridgePartyInitParams.m_bUseSpeakerSsrcForTx),
  m_MS_masterPartyRsrcID(rOtherBridgePartyInitParams.m_MS_masterPartyRsrcID),
  m_MSaudioLocalMsi(rOtherBridgePartyInitParams.m_MSaudioLocalMsi),
  m_MsAvMcuIndex(rOtherBridgePartyInitParams.m_MsAvMcuIndex)
{
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyInitParams::~CBridgePartyInitParams ()
{
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyInitParams& CBridgePartyInitParams::operator =(const CBridgePartyInitParams& rOtherBridgePartyInitParams)
{
  // Operator= is not available for this class because all members are const
  return *this;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyInitParams::Serialize(WORD format, CSegment& seg) const
{
  if (format == NATIVE)
  {
    seg <<  (DWORD)m_pPartyName;
    seg <<  (DWORD)m_pConfName;
    seg <<  (DWORD)m_pParty;
    seg <<  (DWORD)m_pConf;
    seg <<  (DWORD)m_pBridge;
    seg <<  (DWORD)m_partyRsrcID;
    seg <<  (WORD ) m_wNetworkInterface;
    seg <<  (DWORD)m_confRsrcID;
    seg <<  (DWORD)m_pMediaInParams;
    seg <<  (DWORD)m_pMediaOutParams;
    seg <<  (DWORD)m_pBridgePartyCntl;
    seg <<  (DWORD)m_pSiteName;
    seg <<  (WORD )m_bCascadeLinkMode;
    seg <<  (WORD )m_partyRoomId;
    seg <<  (DWORD)m_pSiteNameInfo;
    seg <<	(WORD)m_bIsVideoRelay;
    seg <<  (WORD)m_bUseSpeakerSsrcForTx;
    seg << (DWORD)m_MS_masterPartyRsrcID;
    seg << (DWORD)m_MSaudioLocalMsi;
    seg << (WORD)m_MsAvMcuIndex;

    seg.Put((BYTE*)&m_confAppParams, sizeof(m_confAppParams));


  }
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyInitParams::DeSerialize(WORD format, CSegment& seg)
{
  if (format == NATIVE)
  {
    WORD    tmpWord = 0;
    seg >> (DWORD&)m_pPartyName;
    seg >> (DWORD&)m_pConfName;
    seg >> (DWORD&)m_pParty;
    seg >> (DWORD&)m_pConf;
    seg >> (DWORD&)m_pBridge;
    seg >> (DWORD&)m_partyRsrcID;
    seg >> (WORD &)m_wNetworkInterface;
    seg >> (DWORD&)m_confRsrcID;
    seg >> (DWORD&)m_pMediaInParams;
    seg >> (DWORD&)m_pMediaOutParams;
    seg >> (DWORD&)m_pBridgePartyCntl;
    seg >> (DWORD&)m_pSiteName;

    seg >> tmpWord;
    m_bCascadeLinkMode = (BYTE)tmpWord;

    seg >> (WORD &)m_partyRoomId;
    seg >> (DWORD&)m_pSiteNameInfo;
    seg >> tmpWord;
    m_bIsVideoRelay = (BOOL) tmpWord;
    seg >> tmpWord;
    m_bUseSpeakerSsrcForTx = (BOOL)tmpWord;
    seg >> (DWORD&)m_MS_masterPartyRsrcID;
    seg >> (DWORD&)m_MSaudioLocalMsi;
    seg >> tmpWord;
    m_MsAvMcuIndex = (BYTE)tmpWord;

    seg.Get((BYTE*)&m_confAppParams, sizeof(m_confAppParams));


  }
}

////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyInitParams::IsValidParams() const
{
  #define NOT_VALID(s) "CBridgePartyInitParams::IsValidParams - Failed, invalid '" << s "'"

  TRACECOND_AND_RETURN_VALUE(!GetPartyName(),                                            NOT_VALID("m_pPartyName"),        FALSE);
  TRACECOND_AND_RETURN_VALUE(!GetConfName(),                                             NOT_VALID("m_pConfName"),         FALSE);
  TRACECOND_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(GetParty()),                   NOT_VALID("m_pParty"),            FALSE);
  TRACECOND_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(GetBridge()),                  NOT_VALID("m_pBridge"),           FALSE);
  TRACECOND_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(GetConf()),                    NOT_VALID("m_pConf"),             FALSE);
  TRACECOND_AND_RETURN_VALUE(!::IsValidRsrcID(m_partyRsrcID),                            NOT_VALID("m_partyRsrcID"),       FALSE);
  TRACECOND_AND_RETURN_VALUE(!::IsValidRsrcID(m_confRsrcID),                             NOT_VALID("m_confRsrcID"),        FALSE);
  TRACECOND_AND_RETURN_VALUE(!::IsValidInterfaceType(m_wNetworkInterface),               NOT_VALID("m_wNetworkInterface"), FALSE);
//  TRACECOND_AND_RETURN_VALUE((!IsIvrInConf() && m_confRsrcID == STANDALONE_CONF_ID),     NOT_VALID("m_isIvrInConf"),       FALSE); // TODO: fix this condition
  TRACECOND_AND_RETURN_VALUE((m_pMediaInParams && !m_pMediaInParams->IsValidParams()),   NOT_VALID("m_pMediaInParams"),    FALSE);
  TRACECOND_AND_RETURN_VALUE((m_pMediaOutParams && !m_pMediaOutParams->IsValidParams()), NOT_VALID("m_pMediaOutParams"),   FALSE);
  TRACECOND_AND_RETURN_VALUE(!GetSiteName(),                                             NOT_VALID("m_pSiteName"),         FALSE);
  TRACECOND_AND_RETURN_VALUE(!IsValidCascadeLinkMode(),                                  NOT_VALID("m_bCascadeLinkMode"),  FALSE);
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyInitParams::IsIvrInConf() const
{
  return m_confAppParams.IsIvrInConf();
}

////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyInitParams::IsValidCascadeLinkMode() const
{
  return ((CASCADE_MODE_NONE == m_bCascadeLinkMode) || (CASCADE_MODE_MASTER == m_bCascadeLinkMode) || (CASCADE_MODE_SLAVE == m_bCascadeLinkMode));
}

////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyInitParams::IsValidCopParams() const
{
  return (m_pMediaOutParams) ? m_pMediaOutParams->IsValidCopParams() : TRUE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyInitParams::IsValidXCodeParams() const
{
  return (m_pMediaOutParams) ? m_pMediaOutParams->IsValidXCodeParams() : TRUE;
}

