#ifndef _DELISDNVOICEPARTYCNTL
#define _DELISDNVOICEPARTYCNTL


#include "IsdnPartyCntl.h"

class CDelIsdnVoicePartyCntl : public CIsdnPartyCntl
{
CLASS_TYPE_1(CDelIsdnVoicePartyCntl,CPartyCntl )
public: 
  // Constructors
  CDelIsdnVoicePartyCntl();
  virtual ~CDelIsdnVoicePartyCntl();  
	virtual const char* NameOf() const { return "CDelIsdnVoicePartyCntl";}
  CDelIsdnVoicePartyCntl& operator= (const CDelIsdnVoicePartyCntl & other);
  void  DisconnectPSTN(WORD mode = 0,DWORD disconnectionDelay = 0);	
  void  OnPartyDelayDisconnectIdle(CSegment* pParam);
  // Initializations  
	 						
  // Operations
  virtual void*	 GetMessageMap(); 
  void  Disconnect(WORD mode = 0);
  BOOL GetIsViolentDestroy();
  void SetIsViolentDestroy(BOOL isViolent);
  DWORD GetPartyTaskId();
  void  SetPartyTaskId(DWORD taskId);
                                          
protected:  
  // action functions

  void  OnPartyDisconnectDestroyParty(CSegment* pParam);
  void  OnPartyNetChnlDisconnetDisconnectNet(CSegment* pParam);
  void  OnAudDisConnectDisconnectAudio(CSegment* pParam);
  void  OnMplAckDestroyParty(CSegment* pParam);
  void  OnRsrcDeallocatePartyRspDeallocate(CSegment* pParam);
  void  OnTimerDisconnectNet(CSegment* pParam);
  void  OnTimerDisconnectAudio(CSegment* pParam);
  void  OnTimerDestroyParty(CSegment* pParam);
  void  OnTimerDeallocate(CSegment* pParam);
  void  OnTimerIdle(CSegment* pParam);
  void  OnEndAvcToSvcArtTranslatorDisconnectedDisconnectAudio(CSegment* pParam);
  void  OnDisconnectAvcToSvcArtTranslatorCloseToutDISCONNECTAUDIO(CSegment* pParam);

  
  void  BridgeDisconnetCompleted();
  void  DestroyParty();
  void  EndPartyDisconnect();              
  void  DisconnectAudioBridge();
  void  DeallocatePartyResources();
  void  UpdateConfEndDisconnect(WORD  status);
  void  InitAllAvcToSvcArtTranslatorFlags();
  
  
  WORD  m_isWaitForPartyDisconnection;
  WORD  m_isWaitForDeleteInd;
  BOOL  m_isViolentDestroy;
  DWORD m_partyTaskId;

  // Attributes
  PDECLAR_MESSAGE_MAP                                    
                 
};
  
#endif

