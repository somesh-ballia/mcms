#include <stdio.h>
#include <string.h>


#include "IsdnEncryptionCntl.h"
// state machine opcodes
#include "H221.h"
#include "ConfPartyOpcodes.h"
#include "OpcodesMcmsInternal.h"
#include "SysConfigKeys.h"
// generators and primes - double defines can be moved to common
#include "EncryptionKey.h"
// failure statuses
#include "ConfPartySharedDefines.h"
// HALF_KEY_IND_S
#include "EncryptionKeyServerAPI.h"
// P3_HALF_KEY
#include "EncryptionDefines.h"
#include "OpcodesMcmsMux.h"
#include "MuxHardwareInterface.h"
#include "ManagerApi.h"
#include "PartyApi.h"

#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/fips.h>
#include "EncryptionCommon.h"

#include "ConfPartyProcess.h"

// for session keys calculation
#if !defined(FAR)
	#define FAR
#endif /* INT8 */

#if !defined(TYPE_Uint8)
	typedef unsigned char Uint8, *pUint8;
	#define TYPE_Uint8
#endif /* UINT8 */

#if !defined(TYPE_Uint32)
	typedef unsigned int Uint32, *pUint32;
	#define TYPE_Uint32
#endif /* UINT8 */
extern "C"
{
  //Api for H320 - generating and decryption of keys
  FAR int  H320CreateCipherKey(const Uint8 *pucSharedSecret, Uint32 *punIVector, Uint8 *pucCipherKey, Uint8 *pucEncryptedCipherKey);
  FAR int  H320CreateCipherKeyOpenSSL(const Uint8 *pucSharedSecret, Uint8 *pucIVectorOpenSSL, Uint8 *pucCipherKey, Uint8 *pucEncryptedCipherKey);
  FAR void H320DecryptCipherKey(const Uint8 *pucSharedSecret, Uint32 *punIVector, const Uint8 *pucCipherKey, Uint8 *pucDecryptedCipherKey);
  FAR void H320DecryptCipherKeyOpenSSL(const Uint8 *pucSharedSecret, Uint8 *pucIVector, const Uint8 *pucCipherKey, Uint8 *pucDecryptedCipherKey);
  FAR void CreateRandom128Bits(Uint8 *pucRandKey);
  INT32    CreateRandom128BitsOpenSSL(UINT8 *pucRandKey);
  // for shared secret calculation
  int  GenerateSharedSecretWithPrime(BYTE *pstHKey, int pstLen, BYTE *pstRandNum, BYTE *pucShardKey, BYTE *prime);
}


const DWORD ENC_CONNECTION_MAX_TIME = 30*SECOND;
const DWORD KEY_REQ_MAX_TIME = 3*SECOND;

//===============================================================================================================
// state machine
//===============================================================================================================
  //enum STATE{ECS_IDLE,ECS_ALG_NEGOTIATION, ECS_DIFFIE_HELLMAN, ECS_KEY_EXCHANGE,ECS_IN_CALL};

  // remote events:
  // P0
  // P1
  // P2
  // P3
  // P4
  // P5
  // P6

  // local events:
  // from EncKeyServer:
  // HALF_KEY_IND
  // from party:
  // START_ENC
  // RESTART_ENC

  // timers:
  // KEY_REQ_TOUT
  // ENC_CONNECTION_TOUT

  // change states:
  // ECS_IDLE --> ECS_ALG_NEGOTIATION: first of: START_ENC from party, or P0 from EP
  // ECS_ALG_NEGOTIATION --> ECS_DIFFIE_HELLMAN: P8 received and sent
  // ECS_DIFFIE_HELLMAN --> ECS_KEY_EXCHANGE: P4 received and sent
  // ECS_KEY_EXCHANGE --> ECS_IN_CALL: P6 received and sent
  // ECS_IN_CALL --> ECS_IDLE  RESTART_ENC: from party
//===============================================================================================================
PBEGIN_MESSAGE_MAP(CIsdnEncryptCntl)
  // EP events
  ONEVENT(P0_Identifier         ,ECS_IDLE                      ,CIsdnEncryptCntl::OnRmtP0Idle)

  ONEVENT(P0_Identifier         ,ECS_ALG_NEGOTIATION           ,CIsdnEncryptCntl::OnRmtP0AlgNegotiation)
  ONEVENT(P8_Identifier         ,ECS_ALG_NEGOTIATION           ,CIsdnEncryptCntl::OnRmtP8AlgNegotiation)

  ONEVENT(P0_Identifier         ,ECS_IN_CALL           ,        CIsdnEncryptCntl::OnRmtP0AlgInCall)
  ONEVENT(P8_Identifier         ,ECS_IN_CALL           ,        CIsdnEncryptCntl::OnRmtP8AlgInCall)

  ONEVENT(P9_Identifier         ,ECS_DIFFIE_HELLMAN            ,CIsdnEncryptCntl::OnRmtP9DiffieHellman)
  ONEVENT(P3_Identifier         ,ECS_DIFFIE_HELLMAN            ,CIsdnEncryptCntl::OnRmtP3DiffieHellman)
  ONEVENT(P4_Identifier         ,ECS_DIFFIE_HELLMAN            ,CIsdnEncryptCntl::OnRmtP4DiffieHellman)

  ONEVENT(P5_Identifier         ,ECS_KEY_EXCHANGE              ,CIsdnEncryptCntl::OnRmtP5KeyExchange)
  ONEVENT(P6_Identifier         ,ECS_KEY_EXCHANGE              ,CIsdnEncryptCntl::OnRmtP6KeyExchange)

  ONEVENT(P1_Identifier         ,ANYCASE                       ,CIsdnEncryptCntl::OnRmtP1)
  ONEVENT(P2_Identifier         ,ANYCASE                       ,CIsdnEncryptCntl::OnRmtP2)

  // EncKeyServer events
  ONEVENT(GET_HALF_KEY_IND          ,ECS_IDLE                      ,CIsdnEncryptCntl::OnKeyServerHalfKeyIndIdle)
  ONEVENT(GET_HALF_KEY_IND         ,ECS_ALG_NEGOTIATION           ,CIsdnEncryptCntl::OnKeyServerHalfKeyIndAlgNegotiation)
  ONEVENT(GET_HALF_KEY_IND         ,ECS_DIFFIE_HELLMAN            ,CIsdnEncryptCntl::OnKeyServerHalfKeyIndDiffieHellman)
  
  // party events
  ONEVENT(START_ENC             ,ECS_IDLE                      ,CIsdnEncryptCntl::OnPartyStartEncIdle)
  ONEVENT(START_ENC             ,ECS_ALG_NEGOTIATION           ,CIsdnEncryptCntl::OnPartyStartEncAlgNegotiation)

  // timers events
  ONEVENT(KEY_REQ_TOUT          ,ANYCASE                       ,CIsdnEncryptCntl::OnTimerKeyReqTout)
  ONEVENT(ENC_CONNECTION_TOUT   ,ANYCASE                       ,CIsdnEncryptCntl::OnTimerEncConnectionTout)


PEND_MESSAGE_MAP(CIsdnEncryptCntl,CStateMachine);
//===============================================================================================================


//===============================================================================================================
// constructors
//===============================================================================================================
CIsdnEncryptCntl::CIsdnEncryptCntl(CMuxHardwareInterface* pMuxHardwareInterface,CPartyApi* pTaskApi,const char* partyConfName)
{
  // attributes
  m_pMuxHardwareInterface = new CMuxHardwareInterface(*pMuxHardwareInterface);
  m_pPartyApi = new CPartyApi(*pTaskApi);

  if(strlen(partyConfName)>0 && strlen(partyConfName)<=2*H243_NAME_LEN+50){
    m_partyConfName = partyConfName;
    //strncpy(m_partyConfName,partyConfName,strlen(partyConfName));
  }else{
    m_partyConfName = "empty_name";
  }

  m_pLocal_P3 = NULL;
  m_pLocal_P4 = NULL;
  m_pLocal_P5 = NULL;
  m_pLocal_P6 = NULL;

  m_pRemote_P0 = NULL;
  m_pRemote_P8 = NULL;
  m_pRemote_P9 = NULL; 
  m_pRemote_P3 = NULL;
  m_pRemote_P4 = NULL;
  m_pRemote_P5 = NULL;
  m_pRemote_P6 = NULL;

  m_pLocalRndForP3 = NULL;
  m_pLocalRndForP4 = NULL;

  m_pLocalSharedSecret = NULL;
  m_pRemoteSharedSecret = NULL;
  m_pCommonSharedSecret = NULL;
  m_pWaitingASN1Message = new CSegment();
}
//=============================================================================================================== 
CIsdnEncryptCntl::~CIsdnEncryptCntl()
{

  POBJDELETE(m_pMuxHardwareInterface);
  POBJDELETE(m_pPartyApi);

  // removing keys stamp from memory inside P*_Message destructors
  POBJDELETE(m_pLocal_P3);
  POBJDELETE(m_pLocal_P4);
  POBJDELETE(m_pLocal_P5);
  POBJDELETE(m_pLocal_P6);
  POBJDELETE(m_pRemote_P0);
  POBJDELETE(m_pRemote_P8);
  POBJDELETE(m_pRemote_P9); 
  POBJDELETE(m_pRemote_P3);
  POBJDELETE(m_pRemote_P4);
  POBJDELETE(m_pRemote_P5);
  POBJDELETE(m_pRemote_P6);
  POBJDELETE(m_pWaitingASN1Message);

  // removing keys stamp from memory (same size as allocated)
  // and delete
  if(m_pLocalRndForP3!=NULL){
    memset(m_pLocalRndForP3,'\0',HALF_KEY_SIZE);
    delete[] m_pLocalRndForP3;
  }
  if(m_pLocalRndForP4!=NULL){
    memset(m_pLocalRndForP4,'\0',HALF_KEY_SIZE);
    delete[] m_pLocalRndForP4;
  }
  if(m_pLocalSharedSecret!=NULL){
    memset(m_pLocalSharedSecret,'\0',HALF_KEY_SIZE);
    delete[] m_pLocalSharedSecret;
  }
  if(m_pRemoteSharedSecret!=NULL){
    memset(m_pRemoteSharedSecret,'\0',HALF_KEY_SIZE);
    delete[] m_pRemoteSharedSecret;
  }
  if(m_pCommonSharedSecret!=NULL){
    memset(m_pCommonSharedSecret,'\0',HALF_KEY_SIZE);
    delete[] m_pCommonSharedSecret;
  } 
}
//===============================================================================================================
CIsdnEncryptCntl::CIsdnEncryptCntl(const CIsdnEncryptCntl& other):CStateMachine(other)
{
  *this = other;
}
//===============================================================================================================
CIsdnEncryptCntl& CIsdnEncryptCntl::operator=(const CIsdnEncryptCntl& other)
{
  if( this == &other){
    return *this;
  }
  POBJDELETE(m_pMuxHardwareInterface);
  m_pMuxHardwareInterface = new CMuxHardwareInterface(*other.m_pMuxHardwareInterface);
  POBJDELETE(m_pPartyApi);  
  m_pPartyApi = new CPartyApi(*other.m_pPartyApi);

  if(strlen(other.m_partyConfName)>0 && strlen(other.m_partyConfName)<=2*H243_NAME_LEN+50){
    m_partyConfName = other.m_partyConfName;
    //strncpy(m_partyConfName,partyConfName,strlen(partyConfName));
  }else{
    m_partyConfName = "empty_name";
  }

  POBJDELETE(m_pLocal_P3);
  if(other.m_pLocal_P3 != NULL){
    m_pLocal_P3 = new P3_Message(*other.m_pLocal_P3);
  }
  POBJDELETE(m_pLocal_P4);
  if(other.m_pLocal_P4 != NULL){
    m_pLocal_P4 = new P4_Message(*other.m_pLocal_P4);
  }
  POBJDELETE(m_pLocal_P5);
  if(other.m_pLocal_P5 != NULL){
    m_pLocal_P5 = new P5_Message(*other.m_pLocal_P5);
  }
  POBJDELETE(m_pLocal_P6);
  if(other.m_pLocal_P6 != NULL){
    m_pLocal_P6 = new P6_Message(*other.m_pLocal_P6);
  }
  POBJDELETE(m_pRemote_P0);
  if(other.m_pRemote_P0 != NULL){
    m_pRemote_P0 = new P0_Message(*other.m_pRemote_P0);
  }
  POBJDELETE(m_pRemote_P8);
  if(other.m_pRemote_P8 != NULL){
    m_pRemote_P8 = new P8_Message(*other.m_pRemote_P8);
  }
  POBJDELETE(m_pRemote_P9); 
  if(other.m_pRemote_P9 != NULL){
    m_pRemote_P9 = new P9_Message(*other.m_pRemote_P9);
  }
  POBJDELETE(m_pRemote_P3);
  if(other.m_pRemote_P3 != NULL){
    m_pRemote_P3 = new P3_Message(*other.m_pRemote_P3);
  }
  POBJDELETE(m_pRemote_P4);
  if(other.m_pRemote_P4 != NULL){
    m_pRemote_P4 = new P4_Message(*other.m_pRemote_P4);
  }
  POBJDELETE(m_pRemote_P5);
  if(other.m_pRemote_P5 != NULL){
    m_pRemote_P5 = new P5_Message(*other.m_pRemote_P5);
  }
  POBJDELETE(m_pRemote_P6);
  if(other.m_pRemote_P6 != NULL){
    m_pRemote_P6 = new P6_Message(*other.m_pRemote_P6);
  }
  POBJDELETE(m_pWaitingASN1Message);
  if(other.m_pWaitingASN1Message != NULL){
    m_pWaitingASN1Message = new CSegment(*other.m_pWaitingASN1Message);
  }

  // removing keys stamp from memory (same size as allocated)
  // and delete
  if(m_pLocalRndForP3!=NULL){
    memset(m_pLocalRndForP3,'\0',HALF_KEY_SIZE);
    delete[] m_pLocalRndForP3;
  }
  if(other.m_pLocalRndForP3!=NULL){
    m_pLocalRndForP3 = new BYTE[HALF_KEY_SIZE];
    memcpy(m_pLocalRndForP3,other.m_pLocalRndForP3,HALF_KEY_SIZE);
  }

  if(m_pLocalRndForP4!=NULL){
    memset(m_pLocalRndForP4,'\0',HALF_KEY_SIZE);
    delete[] m_pLocalRndForP4;
  }
  if(other.m_pLocalRndForP4!=NULL){
    m_pLocalRndForP4 = new BYTE[HALF_KEY_SIZE];
    memcpy(m_pLocalRndForP4,other.m_pLocalRndForP4,HALF_KEY_SIZE);
  }

  if(m_pLocalSharedSecret!=NULL){
    memset(m_pLocalSharedSecret,'\0',HALF_KEY_SIZE);
    delete[] m_pLocalSharedSecret;
  }
  if(other.m_pLocalSharedSecret!=NULL){
    m_pLocalSharedSecret = new BYTE[HALF_KEY_SIZE];
    memcpy(m_pLocalSharedSecret,other.m_pLocalSharedSecret,HALF_KEY_SIZE);
  }

  if(m_pRemoteSharedSecret!=NULL){
    memset(m_pRemoteSharedSecret,'\0',HALF_KEY_SIZE);
    delete[] m_pRemoteSharedSecret;
  }
  if(other.m_pRemoteSharedSecret!=NULL){
    m_pRemoteSharedSecret = new BYTE[HALF_KEY_SIZE];
    memcpy(m_pRemoteSharedSecret,other.m_pRemoteSharedSecret,HALF_KEY_SIZE);
  }

  if(m_pCommonSharedSecret!=NULL){
    memset(m_pCommonSharedSecret,'\0',HALF_KEY_SIZE);
    delete[] m_pCommonSharedSecret;
  } 
  if(other.m_pCommonSharedSecret!=NULL){
    m_pCommonSharedSecret = new BYTE[HALF_KEY_SIZE];
    memcpy(m_pCommonSharedSecret,other.m_pCommonSharedSecret,HALF_KEY_SIZE);
  }
  return *this;
}
//===============================================================================================================
// CPobject / State machine pure virtual functions
//=============================================================================================================== 
const char* CIsdnEncryptCntl::NameOf() const
{
    return "CIsdnEncryptCntl";
}
//=============================================================================================================== 


//===============================================================================================================
// public API functions
//===============================================================================================================
// API to CMuxCntl for loca (party) events - CMuxCntl start encryption
void CIsdnEncryptCntl::HandleEvent (CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
  DispatchEvent(opCode,pMsg);
}
//=============================================================================================================== 
// API to CMuxCntl - CMuxCntl pass ECS events from EP to CIsdnEncryptCntl
// This message buffers ECS indication from EP until full ASN1 message received
void CIsdnEncryptCntl::HandleEcsEvent(CSegment* pParam)
{
  if(pParam == NULL){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::HandleASN1Event: pParam is NULL , ",GetPartyName());
    return;
  }

  DWORD ecs_asn1_data_len = 0; 
  *pParam >> ecs_asn1_data_len;
  PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::HandleASN1Event ecs_asn1_data_len = ",(DWORD)ecs_asn1_data_len);

  // saving data until full ECS message received
  AnalyzeHeaderByte(pParam);
}
//===============================================================================================================
// This function Handle the ASN1 event from EP
// it decode it (SERIALEMBD), and dispatch it
void CIsdnEncryptCntl::HandleASN1Event(CSegment* pParam)
{  
  DWORD ecs_opcode = 0;
  CSegment* pDecodedData = new CSegment();

  DWORD asn1_decode_status = DecodeASN1Event(pParam,ecs_opcode,pDecodedData);
  if(asn1_decode_status != STATUS_OK){
    DBGPASSERT(asn1_decode_status);
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::HandleASN1Event: failed to decode ASN1 evevt  ",GetPartyName());
    delete pDecodedData;
    return ;
  }
  DispatchEvent(ecs_opcode,pDecodedData);
  POBJDELETE(pDecodedData);
}
//===============================================================================================================



//===============================================================================================================
// state machine action functions - Party events
//===============================================================================================================
// Party start encryption negotiation
void CIsdnEncryptCntl::OnPartyStartEncIdle(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnPartyStartEncIdle [enc_trace] , ",GetPartyName());
  SendP0();
  SendP8();
  HalfKeyReq(POLYCOM_DH_GENERATOR,true); // immediatly request half key for P3, to shorten overall connection time
  m_state = ECS_ALG_NEGOTIATION;
  StartTimer(ENC_CONNECTION_TOUT,ENC_CONNECTION_MAX_TIME);
}
//===============================================================================================================
void CIsdnEncryptCntl::OnPartyStartEncAlgNegotiation(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnPartyStartEncAlgNegotiation  - do nothing encryption negotiation already started [enc_trace] , ",GetPartyName()); 
}
//===============================================================================================================

//===============================================================================================================
// state machine action functions - Remote events
//===============================================================================================================
// EP initiate encryption
void CIsdnEncryptCntl::OnRmtP0Idle(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP0Idle [enc_trace] , ",GetPartyName());

  // deserialize params into data member
  if(m_pRemote_P0==NULL){
    m_pRemote_P0 = new P0_Message();
  }else{
    // we should not be here
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP0Idle m_pRemote_P0 != NULL , ",GetPartyName());
  }
  STATUS deserialize_status = m_pRemote_P0->DeSerialize(NATIVE,*pParam);
  if(deserialize_status==STATUS_OK && m_pRemote_P0->IsSupportDiffieHellman()){
    // state idle => we should send P0,P8
    SendP0();
    SendP8();
    m_state = ECS_ALG_NEGOTIATION;
    StartTimer(ENC_CONNECTION_TOUT,ENC_CONNECTION_MAX_TIME);
  }else{
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP0Idle - Encryption Failure: Remote P0 Does Not include DH [enc_trace]");
    FailureToStartEncrypt(A_COMMON_KEY_EXCHANGE_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE, P1_Identifier);
    return;
  }  
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP0AlgInCall(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP0AlgInCall [enc_trace] , ",GetPartyName());
    m_state = ECS_IDLE;
    POBJDELETE(m_pRemote_P0);
    POBJDELETE(m_pRemote_P8);
    POBJDELETE(m_pRemote_P9); 
    POBJDELETE(m_pRemote_P3);
    POBJDELETE(m_pRemote_P4);
    POBJDELETE(m_pRemote_P5);
    POBJDELETE(m_pRemote_P6);

    POBJDELETE(m_pLocal_P3);
    POBJDELETE(m_pLocal_P4);
    POBJDELETE(m_pLocal_P5);
    POBJDELETE(m_pLocal_P6);

    OnPartyStartEncIdle(NULL);
    
    OnRmtP0AlgNegotiation(pParam);
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP8AlgInCall(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP8AlgInCall [enc_trace] , ",GetPartyName());
    m_state = ECS_IDLE;
    POBJDELETE(m_pRemote_P0);
    POBJDELETE(m_pRemote_P8);
    POBJDELETE(m_pRemote_P9); 
    POBJDELETE(m_pRemote_P3);
    POBJDELETE(m_pRemote_P4);
    POBJDELETE(m_pRemote_P5);
    POBJDELETE(m_pRemote_P6);

    POBJDELETE(m_pLocal_P3);
    POBJDELETE(m_pLocal_P4);
    POBJDELETE(m_pLocal_P5);
    POBJDELETE(m_pLocal_P6);
  
    OnPartyStartEncIdle(NULL);
        
    OnRmtP8AlgNegotiation(pParam);
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP0AlgNegotiation(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP0AlgNegotiation [enc_trace] , ",GetPartyName());

  // deserialize params into data member
  if(m_pRemote_P0==NULL){
    m_pRemote_P0 = new P0_Message();
  }else{
    // we should not be here
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP0AlgNegotiation m_pRemote_P0 != NULL , ",GetPartyName());
  }
  STATUS deserialize_status = m_pRemote_P0->DeSerialize(NATIVE,*pParam);
  if(deserialize_status==STATUS_OK && m_pRemote_P0->IsSupportDiffieHellman()){
    if(IsP8Received()){
      SendP9();
      if(IsLocalP3Ready()){
	SendP3();
      }else{
	PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP0AlgNegotiation - not sending P3, waiting for key from EncryptionKeyServer");
      }
      m_state = ECS_DIFFIE_HELLMAN;
      
    }else{
      PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP0AlgNegotiation - waiting for Remote P8");
    }
  }else{
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP0AlgNegotiation - Encryption Failure: Remote P0 Does Not include DH [enc_trace]");
    FailureToStartEncrypt(A_COMMON_KEY_EXCHANGE_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE, P1_Identifier);
    return;
  }
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP8AlgNegotiation(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP8AlgNegotiation [enc_trace] , ",GetPartyName());

  // deserialize params into data member
  if(m_pRemote_P8==NULL){
    m_pRemote_P8 = new P8_Message();
  }else{
    // we should not be here
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP8AlgNegotiation m_pRemote_P8 != NULL , ",GetPartyName());
  }

  // deserialize params into data member
  STATUS deserialize_status = m_pRemote_P8->DeSerialize(NATIVE,*pParam);
  if(deserialize_status!=STATUS_OK){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP8AlgNegotiation - Encryption Failure: failed to deserialize remote P8 [enc_trace]");
    FailureToStartEncrypt(A_COMMON_ENCRYPTION_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE);
    return;
  }
  if(m_pRemote_P8->IsSupportAES128()){
    if(IsP0Received()){
      SendP9();
      if(IsLocalP3Ready()){
	SendP3();
      }else{
	PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP8AlgNegotiation - waiting for Remote P0");
      }
      m_state = ECS_DIFFIE_HELLMAN;      
    }else{
      PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP8AlgNegotiation - waiting for Remote P0 (P8 received before P0)");
    }
  }else{
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP8AlgNegotiation - Encryption Failure: Remote P8 Does Not include AES128 [enc_trace]");
    FailureToStartEncrypt(A_COMMON_ENCRYPTION_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE);
    return;
  }
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP9DiffieHellman(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP9DiffieHellman [enc_trace] , ",GetPartyName());

  // deserialize params into data member
  if(m_pRemote_P9==NULL){
    m_pRemote_P9 = new P9_Message();
  }else{
    // we should not be here
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP9DiffieHellman m_pRemote_P9 != NULL , ",GetPartyName());
  }
  STATUS deserialize_status = m_pRemote_P9->DeSerialize(NATIVE,*pParam);

  if(deserialize_status!=STATUS_OK){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP9DiffieHellman - Encryption Failure: failed to deserialize remote P9 [enc_trace]");
    FailureToStartEncrypt(A_COMMON_ENCRYPTION_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE);
    return;
  }
  if( !(m_pRemote_P9->IsSupportAES128()) ){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP9DiffieHellman - Encryption Failure: Remote P9 Does Not include AES128 [enc_trace]");
    FailureToStartEncrypt(A_COMMON_ENCRYPTION_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE);
    return;
  }
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP3DiffieHellman(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP3DiffieHellman [enc_trace] , ",GetPartyName());
  
  // deserialize params into data member
  if(m_pRemote_P3==NULL){
    m_pRemote_P3 = new P3_Message();
  }else{
    // we should not be here
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP3DiffieHellman m_pRemote_P3 != NULL , ",GetPartyName());
  }

  STATUS deserialize_status = m_pRemote_P3->DeSerialize(NATIVE,*pParam);
  if(deserialize_status!=STATUS_OK){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP3DiffieHellman - Encryption Failure: failed to deserialize remote P3 [enc_trace]");
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }

  // we support polycom and tandberg generators
  if(m_pRemote_P3->IsGeneratorSupported()){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP3DiffieHellman - request for half key , ",GetPartyName());
    if(!m_pRemote_P3->IsPrimeSupported()){
      PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP3DiffieHellman - Encryption Failure: prime not consist with generator - not supported [enc_trace]");
      // currently do nothing - just for debug
    }
    // after P3 we received the remote root and prime
    // Request for half key, using remote root and prime for P4
    HalfKeyReq(m_pRemote_P3->GetGenerator(),true);
  }else{
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP3DiffieHellman - Encryption Failure: Remote P3 generator Not Supported [enc_trace]");
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP4DiffieHellman(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP4DiffieHellman [enc_trace] , ",GetPartyName());

  // deserialize params into data member and update capabilities
  if(m_pRemote_P4==NULL){
    m_pRemote_P4 = new P4_Message();
  }else{
    // we should not be here
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP4DiffieHellman m_pRemote_P4 != NULL , ",GetPartyName());
  }
  STATUS deserialize_status = m_pRemote_P4->DeSerialize(NATIVE,*pParam);

  if(deserialize_status!=STATUS_OK){
    PTRACE(eLevelInfoNormal,"CCIsdnEncryptCntl::OnRmtP4DiffieHellman - Encryption Failure: failed to deserialize remote P4 [enc_trace]");
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }

  // calculate local shared secret 
  m_pLocalSharedSecret = new BYTE[HALF_KEY_SIZE];
  
  DumpHex("CIsdnEncryptCntl::OnRmtP4DiffieHellman , SharedSecretParams p4_halfkey = ",m_pRemote_P4->GetHalfKey(),HALF_KEY_SIZE,eLevelInfoNormal);
  DumpHex("CIsdnEncryptCntl::OnRmtP4DiffieHellman , SharedSecretParams m_pLocalRndForP3 = ",m_pLocalRndForP3,HALF_KEY_SIZE,eLevelInfoNormal);
  DumpHex("CIsdnEncryptCntl::OnRmtP4DiffieHellman , SharedSecretParams local_p3_prime = ",m_pLocal_P3->GetPrime(),HALF_KEY_SIZE,eLevelInfoNormal);

  // (remote_P4_half_key ^ local_rnd_for_p3 ) mod local_prime => into pLocaSharedSecret
  int generate_status = GenerateSharedSecretWithPrime(m_pRemote_P4->GetHalfKey(), HALF_KEY_SIZE, m_pLocalRndForP3,m_pLocalSharedSecret, m_pLocal_P3->GetPrime());
  
  DumpHex("CIsdnEncryptCntl::OnRmtP4DiffieHellman , SharedSecretResult m_pLocalSharedSecret = ", m_pLocalSharedSecret,HALF_KEY_SIZE,eLevelInfoNormal);

  if(generate_status!=SHARED_SECRET_LENGTH){
    PTRACE(eLevelInfoNormal,"CCIsdnEncryptCntl::OnRmtP4DiffieHellman - Encryption Failure: failed to generate local shared secret [enc_trace]");
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }

  StartKeyExchange();
}
//===============================================================================================================
// only MCU sends P5 message to the EP
// Receiving P5 means cascade call
// we don't disconnect a call because of wrong P5
void CIsdnEncryptCntl::OnRmtP5KeyExchange(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP5KeyExchange [enc_trace] , ",GetPartyName());
  
  // deserialize params into data member
  if(m_pRemote_P5==NULL){
    m_pRemote_P5 = new P5_Message();
  }else{
    // we should not be here
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP5KeyExchange m_pRemote_P5 != NULL , ",GetPartyName());
  }
  STATUS deserialize_status = m_pRemote_P5->DeSerialize(NATIVE,*pParam);
  if(deserialize_status!=STATUS_OK){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP5KeyExchange - Encryption Failure: failed to deserialize remote P5");
    //FailureToStartEncrypt(A_COMMON_ENCRYPTION_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE);
    return;
  }

  // check if: remote check code == local check code
//   DWORD check_code_status = CheckRemoteP5(); 
//   if(check_code_status != STATUS_OK){
//     FailureToStartEncrypt(A_COMMON_ENCRYPTION_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE);
//   }
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP6KeyExchange(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP6KeyExchange [enc_trace] , ",GetPartyName());
  // deserialize params into data member and update capabilities
  if(m_pRemote_P6==NULL){
    m_pRemote_P6 = new P6_Message();
  }else{
    // we should not be here
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP6KeyExchange m_pRemote_P6 != NULL , ",GetPartyName());
  }
  STATUS deserialize_status = m_pRemote_P6->DeSerialize(NATIVE,*pParam);
  if(deserialize_status!=STATUS_OK){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP6KeyExchange - Encryption Failure: failed to deserialize remote P6 [enc_trace]");
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }

  DWORD session_keys_status = CalculateSessionKeys();
  if(session_keys_status == STATUS_OK){
    SendSessionKeysToMux();
    m_state = ECS_IN_CALL;
    //m_state = ECS_IDLE;
    DeleteTimer(ENC_CONNECTION_TOUT);
   }else{
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP6KeyExchange - Encryption Failure: Remote P6 session keys failure [enc_trace]");
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }  
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP1(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP1 [enc_trace] , ",GetPartyName());
  FailureToStartEncrypt(A_COMMON_KEY_EXCHANGE_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE, P1_Identifier);
}
//===============================================================================================================
void CIsdnEncryptCntl::OnRmtP2(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnRmtP2 [enc_trace] , ",GetPartyName());
  FailureToStartEncrypt(A_COMMON_ENCRYPTION_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE);
}

//===============================================================================================================
// state machine action functions - EncryptionKeyServer events
//===============================================================================================================
void CIsdnEncryptCntl::OnKeyServerHalfKeyIndIdle(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyIndIdle [enc_trace] , ",GetPartyName());
  OnKeyServerHalfKeyInd(pParam);
}
//===============================================================================================================
void CIsdnEncryptCntl::OnKeyServerHalfKeyIndAlgNegotiation(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyIndAlgNegotiation [enc_trace] , ",GetPartyName());
  WORD half_key_use = OnKeyServerHalfKeyInd(pParam);
  if(half_key_use==P3_HALF_KEY){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyIndAlgNegotiation - local P3 is ready , ",GetPartyName());
  }else{
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyIndAlgNegotiation - wrong half key received [enc_trace] , ",GetPartyName());
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }
}
//===============================================================================================================
void CIsdnEncryptCntl::OnKeyServerHalfKeyIndDiffieHellman(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyIndDiffieHellman [enc_trace] , ",GetPartyName());
  WORD half_key_use = OnKeyServerHalfKeyInd(pParam);
  if(half_key_use==P3_HALF_KEY){
    SendP3();
  }else if(half_key_use==P4_HALF_KEY){
    SendP4();
    if(IsP4Received()){
      StartKeyExchange();
    }      
  }else{
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyIndDiffieHellman - wrong half key received [enc_trace] , ",GetPartyName());
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }
}

//===============================================================================================================
// state machine action functions - Timers events
//===============================================================================================================
void CIsdnEncryptCntl::OnTimerKeyReqTout(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnTimerKeyReqTout ",GetPartyName());
  FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
}
//===============================================================================================================
void CIsdnEncryptCntl::OnTimerEncConnectionTout(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnTimerEncConnectionTout ",GetPartyName());
  FailureToStartEncrypt(THE_ENCRYPTION_SETUP_PROCESS_DID_NOT_END_ON_TIME);
}
//===============================================================================================================


//===============================================================================================================
// Private functions
//===============================================================================================================
void CIsdnEncryptCntl::SendP0()
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP0 [enc_trace] , ",GetPartyName());

  P0_Message localP0(DIFFIE_HELMAN_MASK);

  CSegment* Asn1Seg = new CSegment();
  localP0.Serialize(SERIALEMBD,*Asn1Seg);
  DWORD asn1MessageLength = Asn1Seg->GetWrtOffset();
  SendEcsMessageToMux(localP0.GetIdentifier(),asn1MessageLength,*Asn1Seg);
  POBJDELETE(Asn1Seg);
}
//===============================================================================================================
void CIsdnEncryptCntl::SendP8()
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP8 [enc_trace] , ",GetPartyName());

  P8_Message localP8;
  localP8.CreateLocal();

  CSegment* Asn1Seg = new CSegment();
  localP8.Serialize(SERIALEMBD,*Asn1Seg);
  DWORD asn1MessageLength = Asn1Seg->GetWrtOffset();
  SendEcsMessageToMux(localP8.GetIdentifier(),asn1MessageLength,*Asn1Seg);
  POBJDELETE(Asn1Seg);
}
//===============================================================================================================
void CIsdnEncryptCntl::SendP9()
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP9 [enc_trace] , ",GetPartyName());

  P9_Message localP9;
  localP9.CreateLocal();

  CSegment* Asn1Seg = new CSegment();
  localP9.Serialize(SERIALEMBD,*Asn1Seg);
  DWORD asn1MessageLength = Asn1Seg->GetWrtOffset();
  SendEcsMessageToMux(localP9.GetIdentifier(),asn1MessageLength,*Asn1Seg);
  POBJDELETE(Asn1Seg);
}
//===============================================================================================================
void CIsdnEncryptCntl::SendP3()
{
  if(m_pLocal_P3==NULL){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP3 - m_pLocal_P3==NULL ",GetPartyName());
    return;
  }
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP3 [enc_trace] , ",GetPartyName());

  CSegment* Asn1Seg = new CSegment();
  m_pLocal_P3->Serialize(SERIALEMBD,*Asn1Seg);
  DWORD asn1MessageLength = Asn1Seg->GetWrtOffset();
  SendEcsMessageToMux(m_pLocal_P3->GetIdentifier(),asn1MessageLength,*Asn1Seg);
  POBJDELETE(Asn1Seg);  
}
//===============================================================================================================
void CIsdnEncryptCntl::SendP4()
{
  if(m_pLocal_P4==NULL){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP4 - m_pLocal_P4==NULL ",GetPartyName());
    return;
  }
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP4 [enc_trace] , ",GetPartyName());

  CSegment* Asn1Seg = new CSegment();
  m_pLocal_P4->Serialize(SERIALEMBD,*Asn1Seg);
  DWORD asn1MessageLength = Asn1Seg->GetWrtOffset();
  SendEcsMessageToMux(m_pLocal_P4->GetIdentifier(),asn1MessageLength,*Asn1Seg);
  POBJDELETE(Asn1Seg);
}
//===============================================================================================================
void CIsdnEncryptCntl::SendP5()
{
  if(m_pLocal_P5==NULL){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP5 -m_pLocal_P5==NULL ",GetPartyName());
    return;
  }
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP5 [enc_trace] , ",GetPartyName());

  CSegment* Asn1Seg = new CSegment();
  m_pLocal_P5->Serialize(SERIALEMBD,*Asn1Seg);
  DWORD asn1MessageLength = Asn1Seg->GetWrtOffset();
  SendEcsMessageToMux(m_pLocal_P5->GetIdentifier(),asn1MessageLength,*Asn1Seg);
  POBJDELETE(Asn1Seg);  
}
//===============================================================================================================
void CIsdnEncryptCntl::SendP6()
{
  if(m_pLocal_P6==NULL){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP6 - m_pLocal_P6==NULL ",GetPartyName());
    return;
  }
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP6 [enc_trace] , ",GetPartyName());

  CSegment* Asn1Seg = new CSegment();
  m_pLocal_P6->Serialize(SERIALEMBD,*Asn1Seg);
  DWORD asn1MessageLength = Asn1Seg->GetWrtOffset();
  SendEcsMessageToMux(m_pLocal_P6->GetIdentifier(),asn1MessageLength,*Asn1Seg);
  POBJDELETE(Asn1Seg);  
}
//===============================================================================================================
void CIsdnEncryptCntl::SendP1()
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP1 [enc_trace] , ",GetPartyName());

  P1_Message localP1;

  CSegment* Asn1Seg = new CSegment();
  localP1.Serialize(SERIALEMBD,*Asn1Seg);
  DWORD asn1MessageLength = Asn1Seg->GetWrtOffset();
  SendEcsMessageToMux(localP1.GetIdentifier(),asn1MessageLength,*Asn1Seg);
  POBJDELETE(Asn1Seg);
}
//===============================================================================================================
void CIsdnEncryptCntl::SendP2()
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SendP2 [enc_trace] , ",GetPartyName());

  P2_Message localP2;

  CSegment* Asn1Seg = new CSegment();
  localP2.Serialize(SERIALEMBD,*Asn1Seg);
  DWORD asn1MessageLength = Asn1Seg->GetWrtOffset();
  SendEcsMessageToMux(localP2.GetIdentifier(),asn1MessageLength,*Asn1Seg);
  POBJDELETE(Asn1Seg);
}
//===============================================================================================================
// request for half key from EncryptionKeyServer
void CIsdnEncryptCntl::HalfKeyReq(DWORD generator,bool isSyncCall)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::HalfKeyReq [enc_trace] , ",GetPartyName());
	  
  EncyptedSharedMemoryTables 	*pEncyptedSharedMemoryTables= ((CConfPartyProcess*) CProcessBase::GetProcess())->GetEncryptionKeysSharedMemory();
  if (pEncyptedSharedMemoryTables == NULL)
  {
	  PASSERTMSG(true,"Failed getting pEncyptedSharedMemoryTables");
	  return;
  }  
  
  EncryptedKey encryptedKey; 
  STATUS statusQ = pEncyptedSharedMemoryTables->DequeuEncryptedKey(generator, encryptedKey);
  
  if (statusQ != STATUS_OK)
  {
	PASSERTSTREAM(true, "Failed to get key from shared memory for generator " << generator);
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
	return;
  }
  
  // TRACESTR(eLevelInfoNormal) << " IsdnEncryptCntl::HalfKeyReq After getting key for generator " << generator << " KEY " << encryptedKey;	
    
  HALF_KEY_IND_S	resParams;
  memset(&resParams,0,sizeof(HALF_KEY_IND_S));
  resParams.status = STATUS_OK;  
  std::copy(encryptedKey.GetHalfKey(), encryptedKey.GetHalfKey() +  HALF_KEY_SIZE , resParams.halfKey.halfKey);  
  std::copy(encryptedKey.GetRandomNumber(), encryptedKey.GetRandomNumber() +  HALF_KEY_SIZE , resParams.randomNumber.halfKey);
  
  CSegment *pReqSeg = new CSegment;
   
  pReqSeg->Put( (BYTE *) &resParams , sizeof(HALF_KEY_IND_S));
  
  // dispatch half key ind event
  DispatchEvent(GET_HALF_KEY_IND,pReqSeg); 
}

//===============================================================================================================
// This function received ASN1 massage and identifier
// It cuts the message to sub-message (blocks) with maximum of ECS_BLOCK_DATA_SIZE length
// add block header and send to MUX DSP block by block
void CIsdnEncryptCntl::SendEcsMessageToMux(BYTE identifier,DWORD asn1MessageLen, CSegment& asn1Message)const
{
  DWORD data_len = 0;
  BYTE ecs_header = 0xFF;
  if(asn1MessageLen <= ECS_BLOCK_DATA_SIZE){
    // send single block
    data_len = asn1MessageLen;
    ecs_header = SE_HEADER_SINGLE_BLOCK;
    PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::SendEcsMessageToMux send singel block with  data_len = ",(DWORD)data_len);
    SetAndSendEcsBlock(identifier,data_len,ecs_header,asn1Message);

  }else{
    DWORD remaining_length = asn1MessageLen;
    // send first block
    data_len = ECS_BLOCK_DATA_SIZE;
    ecs_header = SE_HEADER_FIRST_BLOCK;
    PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::SendEcsMessageToMux send first block with  data_len = ",(DWORD)data_len);
    SetAndSendEcsBlock(identifier,data_len,ecs_header,asn1Message);
    remaining_length-=ECS_BLOCK_DATA_SIZE;

//  remarked: sleep every 10 blocks
//  WORD counter = 0;// simulation only
//  DWORD SleepTime = 100;
    while(remaining_length > ECS_BLOCK_DATA_SIZE){
//  counter++;// simulation only
//  if(counter==10){
// 	SystemSleep(SleepTime);
// 	counter=0;
//  }

      // send intermidiate block      
      data_len = ECS_BLOCK_DATA_SIZE;
      ecs_header = SE_HEADER_INTERMEDIATE_BLOCK;
      PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::SendEcsMessageToMux intermidiate block with  data_len = ",(DWORD)data_len);
      SetAndSendEcsBlock(identifier,data_len,ecs_header,asn1Message);
      remaining_length-=ECS_BLOCK_DATA_SIZE;
    }
    // send last block
    data_len = remaining_length;
    ecs_header = SE_HEADER_LAST_BLOCK;
    PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::SendEcsMessageToMux last block with  data_len = ",(DWORD)data_len);
    SetAndSendEcsBlock(identifier,data_len,ecs_header,asn1Message);
  }
} 
  
//===============================================================================================================
// set single ECS block
void CIsdnEncryptCntl::SetAndSendEcsBlock(BYTE identifier,DWORD data_len,BYTE ecs_header,CSegment& asn1Message)const
{
  BYTE* blockData = new BYTE[data_len];
  asn1Message.Get(blockData,data_len);
  SendEcsBlockToMux(identifier,data_len+1,ecs_header,blockData);
  delete[] blockData;  
}
//===============================================================================================================
// send to mux single ECS block
void CIsdnEncryptCntl::SendEcsBlockToMux(BYTE identifier,DWORD ecsBlockLen, BYTE ecs_header, BYTE* ecsBlockData)const
{
    CSegment* pMsg = new CSegment();
    // SET_ECS_S: p_opcode, len
    *pMsg << identifier;
    *pMsg << ecsBlockLen;
    // SET_ECS_S: asn1_message
    *pMsg << ecs_header;
    pMsg->Put(ecsBlockData,ecsBlockLen-1);
    // send to mpl
    m_pMuxHardwareInterface->SendMsgToMPL(SET_ECS,pMsg);
    POBJDELETE(pMsg);
}
//===============================================================================================================
inline const char* CIsdnEncryptCntl::GetPartyName()const
{
  return m_partyConfName;
}
//===============================================================================================================
// disconnection/failure scenario
void CIsdnEncryptCntl::FailureToStartEncrypt(WORD disconnection_cause, BYTE p_messge_to_send)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::FailureToStartEncrypt [enc_trace] , ",GetPartyName());

  if(p_messge_to_send == P1_Identifier){
    SendP1();
  }else if(p_messge_to_send == P2_Identifier){
    SendP2();
  }

  DisconnectParty(disconnection_cause);
}
//===============================================================================================================
void CIsdnEncryptCntl::DisconnectParty(WORD disconnection_cause)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::DisconnectParty ",GetPartyName());

  m_pPartyApi->EncryptionDisConnect(disconnection_cause);  
}
//===============================================================================================================



//===============================================================================================================
// half key received server from EncKeyServer
WORD CIsdnEncryptCntl::OnKeyServerHalfKeyInd(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyInd , ",GetPartyName());

  WORD ret_val = UNUSED_HALF_KEY;
  // get key struct from segment
  HALF_KEY_IND_S half_key_ind_s;
  pParam->Get((BYTE *) &half_key_ind_s,sizeof(HALF_KEY_IND_S));

  PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyInd , status = ",(DWORD)half_key_ind_s.status);
  DumpHex("CIsdnEncryptCntl::OnKeyServerHalfKeyInd , halfKey = ",half_key_ind_s.halfKey.halfKey,HALF_KEY_SIZE,eLevelInfoNormal);
  DumpHex("CIsdnEncryptCntl::OnKeyServerHalfKeyInd , randomNumber = ",half_key_ind_s.randomNumber.halfKey,HALF_KEY_SIZE,eLevelInfoNormal);
  // bad status (key = zeros)
  if(half_key_ind_s.status != STATUS_OK){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyInd failed to get half key from server , ",GetPartyName());
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return ret_val;
  }

  if(m_pLocal_P3 == NULL){
    // key for P3 message
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyInd  received key for P3 , ",GetPartyName());
    m_pLocal_P3 = new P3_Message(POLYCOM_DH_GENERATOR,polycomPrime,half_key_ind_s.halfKey.halfKey);
    m_pLocalRndForP3 = new BYTE[HALF_KEY_SIZE];
    memcpy(m_pLocalRndForP3,half_key_ind_s.randomNumber.halfKey,HALF_KEY_SIZE);
    ret_val = P3_HALF_KEY;
    m_pLocal_P3->Dump();
  }else if(m_pLocal_P4 == NULL){
    // key for P4 message
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::OnKeyServerHalfKeyInd  received key for P4 , ",GetPartyName());
    // initiate P4 data member
    m_pLocal_P4 = new P4_Message(half_key_ind_s.halfKey.halfKey);
    m_pLocalRndForP4 = new BYTE[HALF_KEY_SIZE];
    memcpy(m_pLocalRndForP4,half_key_ind_s.randomNumber.halfKey,HALF_KEY_SIZE);

    // calculate remote shared secret 
    m_pRemoteSharedSecret = new BYTE[HALF_KEY_SIZE];
    
    DumpHex("CIsdnEncryptCntl::OnKeyServerHalfKeyInd , SharedSecretParams p3_halfkey = ",m_pRemote_P3->GetHalfKey(),HALF_KEY_SIZE,eLevelInfoNormal);
    DumpHex("CIsdnEncryptCntl::OnKeyServerHalfKeyInd , SharedSecretParams m_pLocalRndForP4 = ",m_pLocalRndForP4,HALF_KEY_SIZE,eLevelInfoNormal);
    DumpHex("CIsdnEncryptCntl::OnKeyServerHalfKeyInd , SharedSecretParams p3_prime = ",m_pRemote_P3->GetPrime(),HALF_KEY_SIZE,eLevelInfoNormal);

    // (remote_P3_half_key ^ local_rnd_for_P4) mod remote_prime => into m_pRemoteSharedSecret
    int generate_status = GenerateSharedSecretWithPrime(m_pRemote_P3->GetHalfKey(),HALF_KEY_SIZE,m_pLocalRndForP4,m_pRemoteSharedSecret, m_pRemote_P3->GetPrime());

    DumpHex("CIsdnEncryptCntl::OnKeyServerHalfKeyInd , SharedSecretResult  m_pRemoteSharedSecret = ",m_pRemoteSharedSecret,HALF_KEY_SIZE,eLevelInfoNormal);

    // failed to calculat RemoteSharedSecret (shared secret = 0)
    if(generate_status!=SHARED_SECRET_LENGTH){                                                              
      FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
      return UNUSED_HALF_KEY;
    }
    ret_val = P4_HALF_KEY;
  }
  return ret_val;
}
//===============================================================================================================
// this function calculate common shared secret (remote xor local)
// derives and send P5 (check code) and p6 (session keys info)
void CIsdnEncryptCntl::StartKeyExchange(){
  WORD shared_secret_status = CalculateCommonSharedSecret();
  if(shared_secret_status != STATUS_OK){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::StartKeyExchange - Encryption Failure: failed to calculate common shared secret");
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }
  WORD p6_status = SetLocalP5P6FromSharedSecret();
  if(p6_status != STATUS_OK){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::StartKeyExchange - Encryption Failure: failed to calculate P6");
    FailureToStartEncrypt(ENCRYPTION_KEY_EXCHANGE_FAILED);
    return;
  }

  m_state = ECS_KEY_EXCHANGE;
  SendP5();
  SendP6();
}
//===============================================================================================================
bool CIsdnEncryptCntl::IsP0Received()const
{
  bool ret_val=false;
  if(m_pRemote_P0!=NULL){
    ret_val=true;
  }
  return ret_val;   
}
//===============================================================================================================
bool CIsdnEncryptCntl::IsP8Received()const
{
  bool ret_val=false;
  if(m_pRemote_P8!=NULL){
    ret_val=true;
  }
  return ret_val;   
}
//===============================================================================================================
bool CIsdnEncryptCntl::IsP4Received()const
{
  bool ret_val=false;
  if(m_pRemote_P4!=NULL){
    ret_val=true;
  }
  return ret_val;   
}
//===============================================================================================================
bool CIsdnEncryptCntl::IsLocalP3Ready()const
{
  bool ret_val=false;
  if(m_pLocal_P3!=NULL){
    ret_val=true;
  }
  return ret_val;
}
//===============================================================================================================
// this function decodes (full buffered) ASN1 events
// it uses the relevant P_message class Deserialize (SERIALEMBD) to decode it
// it serialize it back to segment without ASN1 encoding (NATIVE)
DWORD CIsdnEncryptCntl::DecodeASN1Event(CSegment* pParam,DWORD& ecs_opcode,CSegment* pDecodedParams)
{
  PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::DecodeASN1Event , ", GetPartyName());
  if(PRINT_THE_KEYS){
    pParam->DumpHex();
  }

  DWORD decoded_bytes = 0;  
  BYTE current_byte = 0;

  // extract opcode
  BYTE identifier = 0;
  *pParam >> identifier;
  decoded_bytes++;
  if(!IsLegalEcsIdentifier(identifier)){
    PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::DecodeASN1Event wrong identifier = ",(DWORD)identifier);
    return STATUS_ASN1_DECODING_FAILED;
  }
  ecs_opcode = (DWORD)identifier;

  // create the relevant P_Message
  P_Message* pP_Message = AllocateP_Message(identifier); AUTO_DELETE(pP_Message);
  if(pP_Message == NULL){
    PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::DecodeASN1Event wrong identifier = ",(DWORD)identifier);
    return STATUS_ASN1_DECODING_FAILED;
  }

  // deserialize => from ASN1 message to class
  DWORD deserialize_status = pP_Message->DeSerialize(SERIALEMBD,*pParam);
  if(deserialize_status != STATUS_OK){
    PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::DecodeASN1Event deserialize ASN1 failed = ",(DWORD)deserialize_status);
    return STATUS_ASN1_DECODING_FAILED;
  }
  
  // serialize message to pDecodedParams without ASN1 encoding
  pP_Message->Serialize(NATIVE,*pDecodedParams);
  POBJDELETE(pP_Message);
  
  return STATUS_OK;
}
//===============================================================================================================
// we handle all ecs messages (included p11 and null message which nothing is done)
P_Message* CIsdnEncryptCntl::AllocateP_Message(BYTE identifier)
{
  P_Message* pP_Message = NULL;

  switch(identifier){
  case P0_Identifier:
    pP_Message = new P0_Message();
    break;
  case P1_Identifier:
    pP_Message = new P1_Message();
    break;
  case P2_Identifier:
    pP_Message = new P2_Message();
    break;
  case P3_Identifier:
    pP_Message = new P3_Message();
    break;
  case P4_Identifier:
    pP_Message = new P4_Message();
    break;
  case P5_Identifier:
    pP_Message = new P5_Message();
    break;
  case P6_Identifier:
    pP_Message = new P6_Message();
    break;
  case P8_Identifier:
     pP_Message = new P8_Message();
    break;
 case P9_Identifier:
    pP_Message = new P9_Message();
    break;
  case P11_Identifier:
    pP_Message = new P11_Message();
    break;
  case NULL_Identifier:
    pP_Message = new NULL_Message();
    break;
  default:
    PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::AllocateP_Message wrong identifier = ",(DWORD)identifier);
  }
  return pP_Message;
}
//===============================================================================================================
bool CIsdnEncryptCntl::IsLegalEcsIdentifier(BYTE identifier)
{
  bool is_legal = false;

  switch(identifier){
  case P0_Identifier:
  case P1_Identifier:
  case P2_Identifier:
  case P3_Identifier:
  case P4_Identifier:
  case P5_Identifier:
  case P6_Identifier:
  case P8_Identifier:
  case P9_Identifier:
  case P11_Identifier:
  case NULL_Identifier:
    is_legal = true;
    break;
  default:
    PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::IsLegalEcsIdentifier illegal identifier = ",(DWORD)identifier);
    is_legal = false;
    break;
  }
  return is_legal;
}
//===============================================================================================================
// This function calculates common shared secret from local and remote shared secret (xor)
WORD CIsdnEncryptCntl::CalculateCommonSharedSecret()
{
  PTRACE2(eLevelInfoNormal,"EncryptCntl::CalculateCommonSharedSecret ", GetPartyName());
  WORD ret_val = 0;
  if(m_pLocalSharedSecret == NULL || m_pRemoteSharedSecret == NULL){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::CalculateCommonSharedSecret LocalSharedSecret and RemoteSharedSecret not ready");
    return 1;
  }
  if(m_pCommonSharedSecret == NULL){
    m_pCommonSharedSecret = new BYTE[HALF_KEY_SIZE];
  }else{
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::CalculateCommonSharedSecret m_pCommonSharedSecret already initiated");
  }

  WORD zeroBytes = 0;
  for(WORD byte_index=0;byte_index<HALF_KEY_SIZE;byte_index++)
  {
    m_pCommonSharedSecret[byte_index] = m_pLocalSharedSecret[byte_index] ^ m_pRemoteSharedSecret[byte_index];
    if(m_pCommonSharedSecret[byte_index] == 0){
      zeroBytes++;
    }
  }
  // check if common shared secret all zeros => return failure
  // this happens when  LocalSharedSecret==RemoteSharedSecret 
  if(zeroBytes==HALF_KEY_SIZE){
    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::CalculateCommonSharedSecret m_pCommonSharedSecret result 0");
    return 1;
  }
  return ret_val;
}
//===============================================================================================================
// this function builds P5 (check code) and p6 (session keys info) from shared secret
DWORD CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret()
{  
  // 1) derive check code and shared seret from m_pCommonSharedSecret
  WORD check_code_start_index = 120;
  WORD shared_secret_start_index = 104;
  BYTE DerivedSharedSecret[DERIVED_SHARED_SECRET_LENGTH]; // 16 bytes
  BYTE DerivedCheckCode[CHECK_CODE_LENGTH]; // 8 bytes

  for(WORD j=0;j<DERIVED_SHARED_SECRET_LENGTH;j++){
    DerivedSharedSecret[j] = m_pCommonSharedSecret[shared_secret_start_index+j];
  }
  for(WORD i=0;i<CHECK_CODE_LENGTH;i++){
    DerivedCheckCode[i] = m_pCommonSharedSecret[check_code_start_index+i];
  }

  // 2) build P5 (link_identifier = ConnectionId)
  DWORD link_identifier = m_pMuxHardwareInterface->GetConnectionId();  
  if(m_pLocal_P5!=NULL){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret - m_pLocal_P5!=NULL delete m_pLocal_P5 , ",GetPartyName());
    POBJDELETE(m_pLocal_P5);
  }
  m_pLocal_P5 = new P5_Message((BYTE*)&link_identifier,DerivedCheckCode);
    
  // 3) build P6
  BYTE  P6_InitVector[INIT_VECTOR_LEN];
  BYTE P6_InitVectorForAes[INIT_VECTOR_LEN]; 

  BOOL isOpenSSLFunc = NO;
  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_OPENSSL_ENC_FUNC, isOpenSSLFunc);
//  CSmallString cstr;
//  cstr << "CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret openssl flag is: " << ((isOpenSSLFunc)? "ON" : "OFF");
//  PTRACE(eLevelInfoNormal,cstr.GetString());

  // get random initial vector  
  if (!isOpenSSLFunc){    
    CreateRandom128Bits(P6_InitVector);
  }
  else if(!CreateRandom128BitsOpenSSL(P6_InitVector)){
    PTRACE2(eLevelError,"CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret: FIPS140 Test Failure, Failed to create initial vector - Disconnect the call!  [enc_trace] , ",GetPartyName());
    return FIPS140_STATUS_FAILURE;
  }
  memcpy(P6_InitVectorForAes,P6_InitVector,INIT_VECTOR_LEN);

  // initiate session keys buffers
  BYTE  EncryptedSessionKeyInfo[64];
  BYTE *pEncryptedSessionKeyInfo[4];	    
  for(WORD i=0; i<4; i++){
    pEncryptedSessionKeyInfo[i] = &EncryptedSessionKeyInfo[16*i];
  } 

  // print (start) initial vector and session keys
  DumpHex("CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret DerivedSharedSecret = ",DerivedSharedSecret,ECS_BLOCK_SIZE,eLevelInfoNormal);
  DumpHex("CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret P6_InitVectorForAes = ",P6_InitVectorForAes,ECS_BLOCK_SIZE,eLevelInfoNormal);

  // change initial vector to DWORD for local (not openSSL) H320CreateCipherKey
  DWORD dwIVector[4] = {0x00000000,0x00000000,0x00000000,0x00000000};
  ByteIVToDWORD(P6_InitVectorForAes,dwIVector);
  
  // get keys for P6
  DWORD fips140Status = STATUS_OK;
  for(WORD i=0; i<4 && fips140Status == STATUS_OK; i++)
  {
    if(isOpenSSLFunc){
      fips140Status = H320CreateCipherKeyOpenSSL(DerivedSharedSecret,P6_InitVectorForAes,m_localSessionKeys[i],pEncryptedSessionKeyInfo[i]);
    }else{
      fips140Status = H320CreateCipherKey(DerivedSharedSecret,dwIVector, m_localSessionKeys[i],pEncryptedSessionKeyInfo[i]);
    }
    // print session key + encrypted session key 
    DumpHex("CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret m_localSessionKeys = ",m_localSessionKeys[i],ECS_BLOCK_SIZE,eLevelInfoNormal);
    DumpHex("CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret localEncryptedSessionKeys = ",pEncryptedSessionKeyInfo[i],ECS_BLOCK_SIZE,eLevelInfoNormal);

    // fips failure
    if(fips140Status != STATUS_OK){
      PASSERT(fips140Status);
      CMedString errorString;
      errorString << "CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret: FIPS140 Test Failure - Disconnect the call!\n"; 
      																		
      ALLOCBUFFER(errStr,128);
      ERR_load_crypto_strings();
      ERR_load_FIPS_strings();
      ERR_error_string_n(ERR_get_error(),errStr,128);
      errorString << errStr;
      																																
      PTRACE(eLevelInfoNormal,errorString.GetString());
      																							
      DEALLOCBUFFER(errStr);
				

      return FIPS140_STATUS_FAILURE; 	
    }  			      
  }

  // set m_pLocal_P6
  if(m_pLocal_P6!=NULL){
    PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::SetLocalP5P6FromSharedSecret - m_pLocal_P6!=NULL delete m_pLocal_P6 , ",GetPartyName());
    POBJDELETE(m_pLocal_P6);
  }
  m_pLocal_P6 = new P6_Message(P6_InitVector,EncryptedSessionKeyInfo);

  return STATUS_OK;
}
//===============================================================================================================
// This function calculates the session session keys from local and remote session keys info (P6)
DWORD CIsdnEncryptCntl::CalculateSessionKeys()
{

  // get derived shared secret
  WORD shared_secret_start_index = 104;
  BYTE DerivedSharedSecret[DERIVED_SHARED_SECRET_LENGTH]; // 16 bytes

  WORD i=0;
  for(i=0;i<DERIVED_SHARED_SECRET_LENGTH;i++){
    DerivedSharedSecret[i] = m_pCommonSharedSecret[shared_secret_start_index+i];
  }

  // get remote initial vector and encrypted session keys from remote P6
  BYTE* remoteInitialVector = m_pRemote_P6->GetInitialVector();
  BYTE* remotepEncryptedSessionKeyInfo = m_pRemote_P6->GetEncryptedSessionKeyInfo();
  
  // print start parameters
  DumpHex("CIsdnEncryptCntl::CalculateSessionKeys remotepEncryptedSessionKeyInfo = ",remotepEncryptedSessionKeyInfo,64,eLevelInfoNormal);
  DumpHex("CIsdnEncryptCntl::CalculateSessionKeys DerivedSharedSecret = ",DerivedSharedSecret,DERIVED_SHARED_SECRET_LENGTH,eLevelInfoNormal);
  DumpHex("CIsdnEncryptCntl::CalculateSessionKeys remoteInitialVector = ",remoteInitialVector,16,eLevelInfoNormal);

  // change initial vector to DWORD for local (not openSSL) H320CreateCipherKey
  DWORD dwIVector[4] = {0x00000000,0x00000000,0x00000000,0x00000000};
  ByteIVToDWORD(remoteInitialVector,dwIVector);
  DumpHex("CIsdnEncryptCntl::CalculateSessionKeys dwIVector = ",(BYTE*)dwIVector,16,eLevelInfoNormal);

  // decrypt remote session keys
  BOOL isOpenSSLFunc = NO;
  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_OPENSSL_ENC_FUNC, isOpenSSLFunc);
//  CSmallString cstr;
//  cstr << "CIsdnEncryptCntl::CalculateSessionKeys openssl flag is: " << ((isOpenSSLFunc)? "ON" : "OFF");
//  PTRACE(eLevelInfoNormal,cstr.GetString());
  
  for (i=0; i<4; i++){
    BYTE* pRemoteEncryptedSessionKey = &(remotepEncryptedSessionKeyInfo[16*i]);
    DumpHex("CIsdnEncryptCntl::CalculateSessionKeys remoteEncryptedCipherKey = ",pRemoteEncryptedSessionKey,16,eLevelInfoNormal);

    isOpenSSLFunc? H320DecryptCipherKeyOpenSSL(DerivedSharedSecret,remoteInitialVector, pRemoteEncryptedSessionKey, m_remotelSessionKeys[i]) : 
    			   H320DecryptCipherKey(DerivedSharedSecret,dwIVector, pRemoteEncryptedSessionKey, m_remotelSessionKeys[i]);
    
    DumpHex("CIsdnEncryptCntl::CalculateSessionKeys remoteUnEncryptedSessionKey = ",m_remotelSessionKeys[i],16,eLevelInfoNormal);
  }

//    remarked code: test H320DecryptCipherKey with fixed values from trace
//    const BYTE const_test_SharedSecret[16] = {0x35,0x71,0x4e,0xd7,0x18,0x01,0xe3,0x1f,0x27,0xe2,0xb9,0xd7,0xec,0x52,0x51,0xca};
//    DWORD const_test_IVector[4] = {0xb4a04bc7,0x0a322487,0x3d651843,0x50a8b1cf};
//    const BYTE const_test_CipherKey[16] = {0xfe,0x36,0x23,0x24,0xef,0x24,0xdf,0x8d,0x30,0xee,0x10,0xc7,0x3d,0x71,0x1c,0x13};
//    BYTE const_test_result[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//    H320DecryptCipherKey(const_test_SharedSecret, const_test_IVector,const_test_CipherKey,const_test_result);
//    DumpHex("CIsdnEncryptCntl::CalculateSessionKeys const_test result = ",const_test_result,16,eLevelInfoNormal);
//    PTRACE(eLevelInfoNormal,"CIsdnEncryptCntl::CalculateSessionKeys const_test ep =  edb0a8ccd154c268b26b0bbb3e513010ddaec9390c193ffcbc15d2b1462f18f22a142151a60ec18d2b6c9df1f7b784c0f3bc580cd69b84c70da2138872d61");

  // calculate session key from local and remote (unencrypted) SessionKeyInfo
   BYTE *pLocalKey,*pRemoteKey;
   pLocalKey = m_localSessionKeys[0]; 		  
   pRemoteKey = m_remotelSessionKeys[2];

   // print local and remote keys info for xmitKey
   DumpHex("CIsdnEncryptCntl::CalculateSessionKeys XOR: pLocalKey = ",pLocalKey,16,eLevelInfoNormal);
   DumpHex("CIsdnEncryptCntl::CalculateSessionKeys XOR: pRemoteKey = ",pRemoteKey,16,eLevelInfoNormal);
   // calculate xmitKey
   for(i=0; i<16; i++){
	   m_xmitKey[i] = (pLocalKey[i] ^ pRemoteKey[i]);
   }
   // print xmitKey
   DumpHex("CIsdnEncryptCntl::CalculateSessionKeys XOR result: m_xmitKey = ",m_xmitKey,16,eLevelInfoNormal);
   
   pLocalKey = m_localSessionKeys[2]; 		  
   pRemoteKey = m_remotelSessionKeys[0];
   // print local and remote keys info for rcvKey
   DumpHex("CIsdnEncryptCntl::CalculateSessionKeys XOR: pLocalKey = ",pLocalKey,16,eLevelInfoNormal);
   DumpHex("CIsdnEncryptCntl::CalculateSessionKeys XOR: pRemoteKey = ",pRemoteKey,16,eLevelInfoNormal);
   // calculate rcvKey
   for(i=0; i<16; i++){
   	   m_rcvKey[i] = (pLocalKey[i] ^ pRemoteKey[i]);
   }
   // print rcvKey
   DumpHex("CIsdnEncryptCntl::CalculateSessionKeys XOR result: m_rcvKey = ",m_rcvKey,16,eLevelInfoNormal);

   return STATUS_OK;
}
//===============================================================================================================
void CIsdnEncryptCntl::SendSessionKeysToMux()
{
  CSegment* pMsg = new CSegment();
  pMsg->Put(m_xmitKey,16);
  pMsg->Put(m_rcvKey,16);

  m_pMuxHardwareInterface->SendMsgToMPL(ENC_KEYS_INFO_REQ,pMsg);
  POBJDELETE(pMsg);
}
//===============================================================================================================
// 	Bit 0 to select type:
// 	0 = SE (Session Exchange)
// 	1 = IV (Initialization Vector)
// 	Bits 1 and 2 to identify the blocks of a multi-block sequence:
// 	00 for a single block, not followed by related blocks
// 	01 for block #1 of a sequence of several blocks
// 	10 for an intermediate block in a sequence
// 	11 for the last block of a sequence
// 	Bits 3-7 of SE-type block: spare (s) set to "0"
void CIsdnEncryptCntl::AnalyzeHeaderByte(CSegment* pParam)
{
  BYTE header_byte = BAD_ECS_BLOCK;
  *pParam >> header_byte;

  switch(header_byte){
  case SE_HEADER_SINGLE_BLOCK:
    {
      PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::AnalyzeHeaderByte - SE_HEADER_SINGLE_BLOCK , ", GetPartyName());
      // reset asn1 segment  
      POBJDELETE(m_pWaitingASN1Message);
      m_pWaitingASN1Message = new CSegment();
      // add data to asn1 segment 
      AddDataToWaitingAsn1Segment(pParam);
      // handle asn1 segment  
      HandleASN1Event(m_pWaitingASN1Message);
      break;
    }
  case SE_HEADER_FIRST_BLOCK:
    {
      PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::AnalyzeHeaderByte - SE_HEADER_FIRST_BLOCK , ", GetPartyName());
      // reset asn1 segment  
      POBJDELETE(m_pWaitingASN1Message);
      m_pWaitingASN1Message = new CSegment();
      // add data to asn1 segment     
      AddDataToWaitingAsn1Segment(pParam);
      break;
    }
  case SE_HEADER_INTERMEDIATE_BLOCK:
    {
      PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::AnalyzeHeaderByte - SE_HEADER_INTERMEDIATE_BLOCK , ", GetPartyName());
      // add data to asn1 segment 
      AddDataToWaitingAsn1Segment(pParam);
      break;
    }
  case SE_HEADER_LAST_BLOCK:
    {
      PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::AnalyzeHeaderByte - SE_HEADER_LAST_BLOCK , ", GetPartyName());
      // add data to asn1 segment 
      AddDataToWaitingAsn1Segment(pParam);
      // handle asn1 segment  
      HandleASN1Event(m_pWaitingASN1Message);
      break;
    }
  case BAD_ECS_BLOCK:
  default:
    {
      PTRACE2(eLevelInfoNormal,"CIsdnEncryptCntl::AnalyzeHeaderByte - bad header byte , ", GetPartyName());
      if(PRINT_THE_KEYS){
	pParam->DumpHex();
      }
      break;
    }
  }
}
//===============================================================================================================
// buffer ASN1 blocks to ASN1 full message
void CIsdnEncryptCntl::AddDataToWaitingAsn1Segment(CSegment* pParam)
{
  // add data to asn1 segment      
  DWORD	asn1_len = pParam->GetWrtOffset() - pParam->GetRdOffset();
  PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::AddDataToWaitingAsn1Segment , data_len = ",asn1_len);
  if(asn1_len>ECS_BLOCK_DATA_SIZE){
    PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::AddDataToWaitingAsn1Segment , correcting data_len to 9 , received_data_len= ",asn1_len);
    asn1_len = ECS_BLOCK_DATA_SIZE;
  }
  BYTE*	pAsn1Data = new BYTE [asn1_len];
  pParam->Get(pAsn1Data,asn1_len);
  m_pWaitingASN1Message->Put(pAsn1Data,asn1_len);
  delete[]pAsn1Data;
}
//===============================================================================================================
// Hex print of buffer for all keys and signaling
void CIsdnEncryptCntl::DumpHex(const char* buffer_name,BYTE* data_buff,DWORD data_buffer_size,DWORD trace_level) const
{
  // avoid printing
  if(!PRINT_THE_KEYS){
    PTRACE2(trace_level,buffer_name,DONT_PRINT_THE_KEYS);
    return;
  }

  if(data_buffer_size>256){ // limit dump size for PTRACE macro
    data_buffer_size=256;
    PTRACE2INT(eLevelInfoNormal,"P_Message::DumpHex data_buffer_size > 256 - print first 256 bytes, data_buffer_size= ",(DWORD)data_buffer_size);
  }
  
  DWORD HexCharsPerDataByte = 5; // ",0xFF"
  DWORD secure_tail = 16*5;
  DWORD message_buffer_size =  data_buffer_size*HexCharsPerDataByte + secure_tail;
  char* msgStr = new char[message_buffer_size];
  memset(msgStr,'\0',message_buffer_size);

  char        temp[16];
  memset(temp,'\0',16);

  for (DWORD byte_index = 0; byte_index < data_buffer_size; byte_index++) {
    if (byte_index==0)
      sprintf(temp,"{0x%02x",(unsigned char)(data_buff[byte_index]));
    else if (byte_index == (data_buffer_size-1))
      sprintf(temp,",0x%02x}",(unsigned char)(data_buff[byte_index]));
    else
      sprintf(temp,",0x%02x",(unsigned char)(data_buff[byte_index]));

    strcat(msgStr,temp);
  }
  PTRACE2(trace_level,buffer_name,msgStr);
    
  delete [] msgStr;
}
//===============================================================================================================
// 4 byte array to DWORD little/big endian
DWORD CIsdnEncryptCntl::BytesArrayToDWORD(BYTE* bytesArray){

  DWORD ret_val = 0x00000000;
  for(WORD i=0;i<4;i++){
    BYTE current_byte = bytesArray[i];
    ret_val |= (current_byte << 8*(3-i));
    // Little / Big endian
    //ret_val |= (current_byte << 8*i);
  }
  return ret_val;
}
//===============================================================================================================
// change initial vector to DWORD for local (not openSSL) H320CreateCipherKey
void CIsdnEncryptCntl::ByteIVToDWORD(BYTE* bytesIV, DWORD* dwIV){
        
    WORD iv_byte_index = 0;
    WORD iv_dword_index = 0;
        
    BYTE dword_array[4] = {0x00,0x00,0x00,0x00};
    WORD dword_index = 0;

    while(iv_byte_index<16){
      dword_array[dword_index] = bytesIV[iv_byte_index];
      iv_byte_index++;
      dword_index++;
      if(dword_index > 3)
      {
    	  //	DumpHex("CIsdnEncryptCntl::CalculateSessionKeys dword_array = ",dword_array,4,eLevelInfoNormal);
    	  DWORD dw = BytesArrayToDWORD(dword_array);
		  if(iv_dword_index < 4)
		  {
   		    dwIV[iv_dword_index] = dw;
		    iv_dword_index++;
		  }
		  else
		  	PASSERTMSG(1, "iv_dword_index exceed 4"); 
    	  //	PTRACE2INT(eLevelInfoNormal,"CIsdnEncryptCntl::CalculateSessionKeys dw = ", dw);
    	  
    	  dword_index = 0;
      }
    }
}
//===============================================================================================================
