//+========================================================================+
//                   MuxCntl.H                                             |
//		     Copyright 2005 Polycom, Inc                                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       MuxCntl.h                                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Olga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  10-2007  | Description                                     |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _ISDN_ENCRYPT_CNTL_H_
#define _ISDN_ENCRYPT_CNTL_H_


#include "StateMachine.h"
#include "EcsMessages.h"

class CMuxHardwareInterface;
class CSegment;                                              
class CParty;       
class CMuxCntl;
class CPartyApi;

class CIsdnEncryptCntl : public CStateMachine
{
CLASS_TYPE_1(CIsdnEncryptCntl, CStateMachine)	

public: 

  // states
  // ECS_IDLE = class created ecs signaling not started
  // ECS_ALG_NEGOTIATION = P0,P8 - until enc alg selected
  // ECS_DIFFIE_HELLMAN = P9, P3, P4 - until shared secret
  // ECS_KEY_EXCHANGE = P5, P6 encrypted key exchange
  enum STATE{ECS_IDLE,ECS_ALG_NEGOTIATION, ECS_DIFFIE_HELLMAN, ECS_KEY_EXCHANGE,ECS_IN_CALL};

  // constructors
  CIsdnEncryptCntl(CMuxHardwareInterface* pMuxHardwareInterface,CPartyApi* pTaskApi,const char* partyConfName);
  CIsdnEncryptCntl(const CIsdnEncryptCntl& other);
  CIsdnEncryptCntl& operator=(const CIsdnEncryptCntl& other);
  virtual ~CIsdnEncryptCntl();    

  // CPOBject / StateMachine virtual function
  virtual const char*  NameOf() const;
  virtual void   HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);

  // API to mux
  void HandleEcsEvent(CSegment* pParam);

protected:
  // state machine action functions
  // EP events
  void OnRmtP0Idle(CSegment* pParam);
  void OnRmtP0AlgNegotiation(CSegment* pParam);
   void OnRmtP0AlgInCall(CSegment* pParam);
   void OnRmtP8AlgInCall(CSegment* pParam);
  void OnRmtP8AlgNegotiation(CSegment* pParam);
  void OnRmtP9DiffieHellman(CSegment* pParam);
  void OnRmtP3DiffieHellman(CSegment* pParam);
  void OnRmtP4DiffieHellman(CSegment* pParam);
  void OnRmtP5KeyExchange(CSegment* pParam);
  void OnRmtP6KeyExchange(CSegment* pParam);
  void OnRmtP1(CSegment* pParam);
  void OnRmtP2(CSegment* pParam);
  // EncKeyServer events
  void OnKeyServerHalfKeyIndIdle(CSegment* pParam);
  void OnKeyServerHalfKeyIndAlgNegotiation(CSegment* pParam);
  void OnKeyServerHalfKeyIndDiffieHellman(CSegment* pParam);
  // party events
  void OnPartyStartEncIdle(CSegment* pParam);
  void OnPartyStartEncAlgNegotiation(CSegment* pParam);
  // timers events
  void OnTimerKeyReqTout(CSegment* pParam);
  void OnTimerEncConnectionTout(CSegment* pParam);

private:

  void SendP0();
  void SendP8();
  void SendP9();
  void SendP3();
  void SendP4();
  void SendP5();
  void SendP6();
  void SendP1();
  void SendP2();

  bool IsP0Received()const;
  bool IsP8Received()const;
  bool IsP4Received()const;
  bool IsLocalP3Ready()const;

  
  void HalfKeyReq(DWORD generator,bool isSyncCall=false);

  //  void SerializeEcsMessageAndSendToMux(P_Message& P_message)const;
  void SendEcsMessageToMux(BYTE identifier,DWORD asn1MessageLen, CSegment& asn1Message)const;
  void SetAndSendEcsBlock(BYTE identifier,DWORD data_len,BYTE ecs_header,CSegment& asn1Message)const;
  void SendEcsBlockToMux(BYTE identifier,DWORD ecsBlockLen, BYTE ecs_header, BYTE* ecsBlockData)const;

  const char* GetPartyName()const;
  void FailureToStartEncrypt(WORD disconnection_cause, BYTE p_messge_to_send=0);
  void StartKeyExchange();
  WORD OnKeyServerHalfKeyInd(CSegment* pParam);

  // help for HandleASN1Event
  void HandleASN1Event(CSegment* pParam);
  void AnalyzeHeaderByte(CSegment* pParam);
  void AddDataToWaitingAsn1Segment(CSegment* pParam);
  DWORD DecodeASN1Event(CSegment* pParam,DWORD& ecs_opcode,CSegment* pDecodedParams);
  P_Message* AllocateP_Message(BYTE identifier);

  WORD CalculateCommonSharedSecret();
  DWORD SetLocalP5P6FromSharedSecret();
  DWORD CalculateSessionKeys();
  DWORD BytesArrayToDWORD(BYTE * bytesArray);
  void ByteIVToDWORD(BYTE* bytesIV, DWORD* dwIV);
  void SendSessionKeysToMux();
  bool IsLegalEcsIdentifier(BYTE identifier);
  void DisconnectParty(WORD disconnection_cause);

  void DumpHex(const char* buffer_name,BYTE* data_buff,DWORD data_buffer_size,DWORD trace_level) const;
  


  // attributes

  const char* m_partyConfName;//[2*H243_NAME_LEN+50];
  
  CMuxHardwareInterface* m_pMuxHardwareInterface; // API to MPL
  CPartyApi* m_pPartyApi; // API to party

  P3_Message* m_pLocal_P3;
  P4_Message* m_pLocal_P4;
  P5_Message* m_pLocal_P5;
  P6_Message* m_pLocal_P6;

  P0_Message* m_pRemote_P0;
  P8_Message* m_pRemote_P8;
  P9_Message* m_pRemote_P9; 
  P3_Message* m_pRemote_P3;
  P4_Message* m_pRemote_P4;
  P5_Message* m_pRemote_P5;
  P6_Message* m_pRemote_P6;

  BYTE* m_pLocalRndForP3;
  BYTE* m_pLocalRndForP4;

  BYTE* m_pLocalSharedSecret;
  BYTE* m_pRemoteSharedSecret;
  BYTE* m_pCommonSharedSecret;

  BYTE  m_localSessionKeys[4][16]; 
  BYTE  m_remotelSessionKeys[4][16];

  BYTE m_rcvKey[16];
  BYTE m_xmitKey[16];

  CSegment* m_pWaitingASN1Message;

  PDECLAR_MESSAGE_MAP
};

#endif // _ISDN_ENCRYPT_CNTL_H_
