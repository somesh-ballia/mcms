#ifndef _ISDN_CHANGEMODE_CONTROL
#define _ISDN_CHANGEMODE_CONTROL



#include "IsdnPartyCntl.h"

////////////////////////////////////////////////////////////////////////////
//                        CIsdnChangeModeCntl
////////////////////////////////////////////////////////////////////////////
class CIsdnChangeModeCntl : public CIsdnPartyCntl
{
  CLASS_TYPE_1(CIsdnChangeModeCntl, CIsdnPartyCntl)

public:
                       CIsdnChangeModeCntl();
  virtual             ~CIsdnChangeModeCntl()    { }

  CIsdnChangeModeCntl& operator=(const CIsdnPartyCntl& other);
  CIsdnChangeModeCntl& operator=(const CIsdnChangeModeCntl& other);

  // Operations
  const virtual char*  NameOf() const   { return "CIsdnChangeModePartyCntl"; }
  virtual void*        GetMessageMap()  { return (void*)m_msgEntries;        }

  virtual void         ChangeScm(CComMode* pTargetTransmitScm, CComMode* pTargetReceiveScm);

  // from conf
  void                 OnConfChangeModeIdle(CSegment* pParam);
  void                 OnConfChangeModeChangeContent(CSegment* pParam);
  void                 OnConfChangeModeChangeVideo(CSegment* pParam);
  // from video bridge
  void                 OnVidBrdgConVideoIdle(CSegment* pParam);
  void                 OnVidBrdgConChangeVideo(CSegment* pParam);
  void                 OnVidBrdgConChangeContent(CSegment* pParam);
  void                 OnVidBrdgDisconnectAnycase(CSegment* pParam);
  void                 OnVideoInBrdgUpdatedChangeVideo(CSegment* pParam);
  void                 OnVideoInBrdgUpdatedAnycase(CSegment* pParam);
  void                 OnVideoOutBrdgUpdatedChangeVideo(CSegment* pParam);
  void                 OnVideoOutBrdgUpdatedAnycase(CSegment* pParam);
  // from content bridge
  int                  OnContentBrdgConnectedChangeContent(CSegment* pParam);
  void                 OnContentBrdgConnectedAnycase(CSegment* pParam);
  int                  OnContentBrdgDisconnected(CSegment* pParam);
  void                 OnXCodeBrdgDisconnected(CSegment* pParam);
  // from party
  void                 OnRmtXmitModeIdle(CSegment* pParam);
  void                 OnRmtXmitModeAnycase(CSegment* pParam);
  void                 OnRmtXmitModeChangeContent(CSegment* pParam);
  void                 OnRmtXmitModeChangeVideo(CSegment* pParam);
  // party sent new Capset
  void                 OnRmtCapsIdle(CSegment* pParam);
  void                 OnRmtCapsChangeContent(CSegment* pParam);
  void                 OnRmtCapsChangeVideo(CSegment* pParam);
  // party updated presentation stream
  void                 OnPartyPresentationOutStreamUpdatedIdle(CSegment* pParam);
  void                 OnPartyPresentationOutStreamUpdatedCHANGEVIDEO(CSegment* pParam);
  void                 OnPartyPresentationOutStreamUpdatedCHANGECONTENT(CSegment* pParam);
  void                 OnPartyPresentationOutStreamUpdated(CSegment* pParam);
  // party h230
  void                 OnPartyRefreshVideoAnycase(CSegment* pParam);
  // from CAM
  void                 OnCAMUpdatePartyInConf(CSegment* pParam);
  // local timers
  void                 OnTimerChangeVideo(CSegment* pParam);
  void                 OnTimerChangeContent(CSegment* pParam);
  void                 OnTimerVideoUpdate(CSegment* pParam);
  void                 OnTimerConnectPartyToVideoBridge(CSegment* pParam);

  virtual WORD         IsPartyInChangeMode() const {return (m_state != IDLE);}
  virtual WORD         IsFeccChanOpened() const    {return FALSE;} // DATA / FECC - currently not supported in RMX

protected:

  // Operations
  virtual void         ChangeContent();
  virtual void         ChangeVideo();
  virtual void         EndChangeMode();

  void                 ConnectToContentBridgeIfPossible();
  void                 ConnectPartyToContentBridge();

  // VIDEO BRIDGE API
  void                 HandleVideoBridgeUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges);
  BYTE                 ChangeVideoModeIfNeeded();
  void                 ConnectPartyToVideoBridge(EBridgeConnectionState eDirection);
  void                 ConnectPartyToVideoBridge(EBridgeConnectionState eDirection, DWORD wait_before_connect);
  BYTE                 ChangeVideoBridgeVideoOutState();  // open\change VideoOut
  BYTE                 ChangeVideoBridgeVideoInState();   // open\change VideoIn
  int                  UpdateVideoOutBridgeIfNeeded();
  int                  UpdateVideoInBridgeIfNeeded();
  void                 UpdateVideoOutBridgeH239Case();

  // internal actions functions -
  // when receiving new scm from party
  void                 OnPartyEndChangeVideo(CSegment* pParam);
  void                 OnPartyEndChangeContent(CSegment* pParam);
  void                 OnPartyEndChangeOther(CSegment* pParam);
  // when receiving new caps from party
  void                 OnRmtCaps(CSegment* pParam);

  // internal functions
  void                 SetPartyToSecondaryAndEndChangeMode();
  BYTE                 AreRemoteCapsEnableChangeMode();
  BYTE                 IsChangeModeNeeded();

  BOOL                 UpdateRemoteCaps(CSegment* pParam);
  void                 UpdateTargetTransmitScm();

  void                 CompareNewScmToOld(CComMode& OldComMode);

  BOOL                 IsH2634CifPreffered(CComMode* pTargetTransmitScm);

  WORD                 m_mlpConnectRequest;
  WORD                 m_lsdChangeModeRequest;
  WORD                 m_hsdChangeModeRequest;
  WORD                 m_mlpChangeModeRequest;
  WORD                 m_hmlpChangeModeRequest;
  BYTE                 m_needToCreateNewLocalCaps;
  BYTE                 m_needToChangeVideoIn;
  BYTE                 m_waitForAckFromVideoOut;
  BYTE                 m_waitForAckFromVideoIn;
  WORD                 m_isSentH239Out;
  BYTE                 m_myTest;

  PDECLAR_MESSAGE_MAP
};
#endif // _ISDN_CHANGEMODE_CONTROL
