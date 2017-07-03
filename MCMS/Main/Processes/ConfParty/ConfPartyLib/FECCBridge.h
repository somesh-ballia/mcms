#ifndef _FECCBRIDGE_
  #define _FECCBRIDGE_

#include <ostream>
#include "Bridge.h"

class CConfApi;
class CConf;
class CPartyApi;
class CChairCntl;
class CFECCBridgeInitParams;

////////////////////////////////////////////////////////////////////////////
//                        CFECCBridge
////////////////////////////////////////////////////////////////////////////
class CFECCBridge : public CBridge
{
  CLASS_TYPE_1(CFECCBridge, CBridge)

public:
  // States definition
  enum STATE {CONNECTED = (IDLE+1), CHANGEMODE, DISCONNECTING};

                        CFECCBridge();
  virtual              ~CFECCBridge()        { }

  virtual const char*   NameOf() const       { return "CFECCBridge";}

  // Initializations
  virtual void          Create(const CFECCBridgeInitParams* pFECCInitParams);

  virtual void*         GetMessageMap()       { return m_msgEntries; }

  // Extract Data
  virtual BYTE          GetBitRate()          { return m_bitRate;    }
  virtual CTaskApp*     GetFECCTokenOwnerId() { return m_tokenOwner; }

  // Operations
  virtual void          SendDataTokenReject(CTaskApp* pParty);

  // Conf Operations
  virtual void          EndRateChange(WORD status);
  virtual void          UpdateLsdBitRate(BYTE bitRate);

  // Action Function
  virtual void          OnConfDisConnectConfCONNECTED(CSegment* pParam);
  virtual void          OnConfTerminateDISCONNECTING(CSegment* pParam);
  virtual void          OnConfConnectPartyCONNECTED(CSegment* pParam);
  virtual void          OnConfDisconnectPartyCONNECTED(CSegment* pParam);
  virtual void          OnEndPartyConnect(CSegment* pParam);
  virtual void          OnEndPartyDisConnect(CSegment* pParam);
  virtual void          OnEndRateChangeDataChannel(CSegment* pParam);
  virtual void          OnPartyDataTokenRequestCONNECTED(CSegment* pParam);
  virtual void          OnPartyDataTokenRequestDISCONNECTING(CSegment* pParam);
  virtual void          OnPartyDataTokenReleaseCONNECTED(CSegment* pParam);
  virtual void          OnPartyDataTokenReleaseDISCONNECTING(CSegment* pParam);
  virtual void          OnPartyDataTokenReleaseAndFreeCONNECTED(CSegment* pParam);
  virtual void          OnDataTokenWithdraw(CSegment* pParam);
  virtual void          OnDataTokenWithdrawAndFree(CSegment* pParam);
  virtual void          OnConfUpdateLsdBitRateCONNECTED(CSegment* pParam);

  // util
  void                  UpdateDataSrcId();

protected:
                        CFECCBridge(const CFECCBridge&);
  CFECCBridge&          operator=(const CFECCBridge&);
  
  // Operations
  void                  ForwardPartyToMasterOpCode(WORD opCode, CTaskApp* pParty, CSegment* pParam);
  void                  ForwardMasterToPartyOpCode(WORD opCode, CTaskApp* pParty);

protected:
  BYTE                  m_bitRate;
  CTaskApp*             m_tokenOwner;

  PDECLAR_MESSAGE_MAP
};

////////////////////////////////////////////////////////////////////////////
//                        CFECCBridgePartyCntl
////////////////////////////////////////////////////////////////////////////
class CFECCBridgePartyCntl : public CBridgePartyCntl
{
  CLASS_TYPE_1(CFECCBridgePartyCntl, CBridgePartyCntl)\

  enum STATE {CONNECTED = (IDLE+1)};

public:
                        CFECCBridgePartyCntl();
  virtual              ~CFECCBridgePartyCntl();

  virtual const char*   NameOf() const       { return "CFECCBridgePartyCntl";}

  virtual void          SetFullName(const char* partyName, const char* confName);

  // Operations
  virtual void*         GetMessageMap()     { return (void*)m_msgEntries; }

  // API
  virtual void          Connect();
  virtual void          DisConnect();
  virtual void          DataTokenAccept(WORD isCameraControl);
  virtual void          DataTokenReject();
  virtual void          DataTokenRelease();
  virtual void          DataTokenReleaseAndFree();
  virtual void          DataTokenRequest(BYTE bitRate);
  virtual void          DataTokenWithraw();
  virtual void          DataTokenReleaseRequest();
  virtual WORD          IsTokenBlocked();

  // Action Function
  virtual void          OnDatCntlConnectIDLE(CSegment* pParam);
  virtual void          OnDatCntlDisConnectCONNECTED(CSegment* pParam);
  virtual void          OnDataCntlTokenRequest(CSegment* pParam);
  virtual void          OnDataCntlTokenRelease(CSegment* pParam);
  virtual void          OnDataCntlTokenReleaseRequest(CSegment* pParam);
  virtual void          OnDataCntlTokenReleaseAndFree(CSegment* pParam);
  virtual void          OnDataCntlTokenAccept(CSegment* pParam);
  virtual void          OnDataCntlTokenReject(CSegment* pParam);
  virtual void          OnDataCntlTokenWithdraw(CSegment* pParam);
  virtual void          OnTimerBlockTokenRequest(CSegment* pParam);

protected:
                        CFECCBridgePartyCntl(const CFECCBridgePartyCntl&);
  CFECCBridgePartyCntl& operator=(const CFECCBridgePartyCntl&);

  WORD                  m_isLsdTokenBlocked;

  PDECLAR_MESSAGE_MAP
};

#endif // _FECCBRIDGE_
