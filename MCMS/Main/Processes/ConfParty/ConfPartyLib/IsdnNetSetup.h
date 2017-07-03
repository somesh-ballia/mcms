#ifndef _ISDNNETSETUP
#define _ISDNNETSETUP

#include "NetSetup.h"
#include "Q931Structs.h"


class CIsdnNetSetup : public CNetSetup
{
CLASS_TYPE_1(CIsdnNetSetup, CNetSetup)
public:             
								// Constructors
  CIsdnNetSetup();
  virtual ~CIsdnNetSetup();  
  // Initializations  
  
  
  // Serialization
  const virtual char*  NameOf() const;
  virtual void copy(	const CNetSetup *rhs);
  CIsdnNetSetup& operator=(const CIsdnNetSetup& otherNet);
  void Serialize(WORD format,CSegment& seg);
  void DeSerialize(WORD format,CSegment& seg);
  void Trace(std::ostream& msg);
  virtual void   Dump(WORD switchFlag = 0);
  
  virtual void  GetCalledNumber(WORD *numDigits,char* pTelNumber);
  virtual void  SetCalledNumber(WORD numDigits,const char* pTelNumber);
  virtual void  GetCallingNumber(WORD *numDigits,char* pTelNumber);
  void  SetCallingNumber(WORD numDigits,const char* pTelNumber);
  virtual WORD  LegalizeCalledNumber();
  virtual void  ResetCallingNumber();
  void          SetNetCommnHeaderParams(NET_COMMON_PARAM_S& headerParams);
  
  //WORD	GetChannelsNumber() const { return m_chnlsNumber; }
  //void	SetChannelsNumber( const WORD chNum ){ m_chnlsNumber = chNum; }
  //WORD	GetRestrict() const { return m_isRestrict; }
  //void	SetRestrict( const WORD restricted ){ m_isRestrict = (restricted)? 1 : 0; }
  
  // attributes
  BYTE  m_netSpcf;
  CNetCallingParty m_calling;
  CNetCalledParty m_called;

  //Net CommonHeader params
  DWORD m_spanId[MAX_NUM_SPANS_ORDER];
  DWORD m_net_connection_id;
  DWORD m_virtual_port_number;
  DWORD m_physical_port_number;

  BYTE m_boardId;
  BYTE m_box_id;
  BYTE m_sub_board_id;
    
  // ATM fields
  //	CAtmAddrDrv m_AtmAddr;
  //	CAtmAddrDrv* m_pVgateArray;
  //	WORD m_numVgateAddress;
  // DWORD m_callType; // 1: restricted, 0: unrestricted
  //DWORD m_voice;
  //DWORD m_channelId;
  //DWORD m_prevCallId;
  
 protected:
  // V35 data
  //WORD	m_chnlsNumber;	// number of channels
  //WORD	m_isRestrict;	// restrict flag
  //WORD	m_dummy1;		// dummy value
  //WORD	m_dummy2;		// dummy value

};
#endif //_ISDNNETSETUP
