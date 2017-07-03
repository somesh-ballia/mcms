
#ifndef LEGACYTOSACTRANSLATOR_H_
#define LEGACYTOSACTRANSLATOR_H_

#include "StateMachine.h"
#include "RsrcDesc.h"
#include "AudioHardwareInterface.h"
//#include "BridgePartyAudioInOut.h"

class CBridgePartyAudioIn;
class CHardwareInterface;
class CRsrcParams;

// Timers opcodes
#define LEGACY_TO_SAC_OPEN_ENCODER_TOUT                ((WORD)200)
#define LEGACY_TO_SAC_CLOSE_ENCODER_TOUT               ((WORD)201)

#define LEGACY_TO_SAC_OPEN_ENCODER_TOUT_VALUE 	3*SECOND
#define LEGACY_TO_SAC_CLOSE_ENCODER_TOUT_VALUE  3*SECOND


class CLegacyToSacTranslator : public CStateMachineValidation
{
  CLASS_TYPE_1(CLegacyToSacTranslator, CStateMachineValidation)

public:

  CLegacyToSacTranslator();
  virtual ~CLegacyToSacTranslator();
  virtual const char* NameOf() const { return "CLegacyToSacTranslator";}
  enum STATE {SETUP = (IDLE+1),CONNECTED, DISCONNECTING};

  // state machine functions
  virtual void	HandleEvent( CSegment *pMsg,DWORD msgLen,OPCODE opCode);
  virtual void*	GetMessageMap();

  // API functions (from CBridgePartyAudioIn)
  virtual void	Create( const CBridgePartyAudioIn*	pBridgePartyAudioIn, const CRsrcParams* pRsrcParamsAudioEncoder);
  virtual void	Connect();
  virtual void	Disconnect();
  void UpdateAudioRelayParams();
  bool IsActive();
  bool IsConnected();
  CRsrcParams* GetRsrcParams();
  void UpdateNewConfParams (DWORD confRsrcId);

protected:

  void	OnLegacyToSacTranslatorConnectIDLE(CSegment* pParam);
  void	OnLegacyToSacTranslatorConnectSETUP(CSegment* pParam);
  void	OnLegacyToSacTranslatorConnectCONNECTED(CSegment* pParam);
  void	OnLegacyToSacTranslatorConnectDISCONNECTING(CSegment* pParam);

  void OnLegacyToSacTranslatorDisconnect();
  void OnLegacyToSacTranslatorDisconnectIDLE(CSegment* pParam);
  void OnLegacyToSacTranslatorDisconnectSETUP(CSegment* pParam);
  void OnLegacyToSacTranslatorDisconnectCONNECTED(CSegment* pParam);
  void OnLegacyToSacTranslatorDisconnectDISCONNECTING(CSegment* pParam);

  void  OnMplAckIDLE( CSegment* pParam );
  void  OnMplAckSETUP( CSegment* pParam );
  void  OnMplAckCONNECTED( CSegment* pParam );
  void  OnMplAckDISCONNECTING( CSegment* pParam );

  void SendUpdateRelayParams();
  void OnLegacyToSacTranslatorUpdateRelayParamsIDLE( CSegment* pParam );
  void OnLegacyToSacTranslatorUpdateRelayParamsSETUP( CSegment* pParam );
  void OnLegacyToSacTranslatorUpdateRelayParamsCONNECTED( CSegment* pParam );
  void OnLegacyToSacTranslatorUpdateRelayParamsDISCONNECTING( CSegment* pParam );

  void OnLegacyToSacTranslatorOpenEncoderToutIDLE( CSegment* pParam );
  void OnLegacyToSacTranslatorOpenEncoderToutSETUP( CSegment* pParam );
  void OnLegacyToSacTranslatorOpenEncoderToutCONNECTED( CSegment* pParam );
  void OnLegacyToSacTranslatorOpenEncoderToutDISCONNECTING( CSegment* pParam );

  void OnLegacyToSacTranslatorCloseEncoderToutIDLE( CSegment* pParam );
  void OnLegacyToSacTranslatorCloseEncoderToutSETUP( CSegment* pParam );
  void OnLegacyToSacTranslatorCloseEncoderToutCONNECTED( CSegment* pParam );
  void OnLegacyToSacTranslatorCloseEncoderToutDISCONNECTING( CSegment* pParam );



  void  OnMplOpenAudioEncoderAck(STATUS status);
  void  OnMplCloseAudioEncoderAck(STATUS status);

  void  SendOpenAudioSacEncoder();
  void  SendCloseAudioSacEncoder();
  void  SetConnected();


  const char*  GetFullName();
  const char* GetConfName();
  PartyRsrcID GetPartyRsrcID();


  CBridgePartyAudioIn*	        m_pBridgePartyAudioIn;
  CAudioHardwareInterface*	    m_pHardwareInterface;
  DWORD 						m_lastReqId; // for mcu internal problem print in case request fails / timeout
  DWORD 						m_lastReq;
  bool 							m_bIsRelayParamsUpdated;
  PDECLAR_MESSAGE_MAP


};
#endif /* LEGACYTOSACTRANSLATOR_H_ */
