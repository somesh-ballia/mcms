/*$Header: /MCMS/MAIN/subsys/mcmsoper/NTCHACON.CPP 14    25/11/01 14:14 Oshi $*/
//+========================================================================+
//                            NTCHACON.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       NTCHACON.CPP                                                |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

/*
#ifdef __HIGHC__

#include  <ntchacon.h> 
#include  <ntime.h>
#include  <mcmsoper.h>
#include <nstream.h>
#include  <cdrincld.h>
#include <typedef.h>
#include <psosxml.h>

#else

#include "Stdafx.h"
#include "m3c.h"
#include "ntime.h"
#include "ntchacon.h"
#include "mcmsoper.h"
#include "nstream.h"
#include "cdrincld.h"
#include "psosxml.h"
#include "typedef.h"
#endif

#ifdef _MCMS_WINDBG_LEAK
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif    
*/

////////////////////////////////////////////////////////////
//Constants
/*
#define MCU_INITIATOR 		0
#define REMOTE_PARTY_INITIATOR		1
#define VOICE		0x1
#define MODEM		0x2
#define CALL_TYPE_56K		0x4
#define CALL_TYPE_64K		0x8
#define CALL_TYPE_64K_RESTRICT		0x10
#define CALL_TYPE_384K		0x20
#define CALL_TYPE_384K_RESTRICT		0x40
#define NONE		0
#define ATT_SDN		1
#define NTI_PRIVATE		0xF1
#define ATI_MEGACOM		3
#define NTI_OUTWATS		0xF3
#define NTI_FX		4
#define NTI_TIE_TRUNK		5
#define ATT_ACCUNET		6
#define ATT_1800		8
#define NTI_TRO		16
#define NO_PRF		0
#define PRF_MODE_PREFERRED		1
#define PRF_MODE_EXCLUSIVE		2
*/

#include "NetChannConn.h"
#include "CallPart.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"
//#include "ConfPartyDefines.h"


////////////////////////////////////////////////////////////
//class CNetChanlCon

ACCCDREventNetChannelConnect::ACCCDREventNetChannelConnect()
{
        m_h243party_name[0]='\0';
		m_party_Id=0xffffffff;
		m_channel_Id=0xff;
		m_channel_num=0xff;
        m_connect_initiator=0xff;
		m_call_type=0;
		m_net_specific=0xff;
		m_prf_mode=NO_PRF;
		
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventNetChannelConnect::ACCCDREventNetChannelConnect(const ACCCDREventNetChannelConnect &other)
{
	*this=other;
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventNetChannelConnect::~ACCCDREventNetChannelConnect()
{
}
ACCCDREventNetChannelConnect& ACCCDREventNetChannelConnect::operator = (const ACCCDREventNetChannelConnect& other)
{
  strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN); 
  m_party_Id = other.m_party_Id;
  m_channel_Id = other.m_channel_Id;
  m_channel_num = other.m_channel_num;
  m_connect_initiator=other.m_connect_initiator;
  m_call_type=other.m_call_type;
  m_net_specific=other.m_net_specific;
  m_prf_mode=other.m_prf_mode;
  m_calling_party=other.m_calling_party;
  m_called_party=other.m_called_party;
  return *this;
}

bool ACCCDREventNetChannelConnect::operator == (const ACCCDREventNetChannelConnect & other)
{
    if(m_party_Id != other.m_party_Id)
    {
        return false;
    }
    if(m_channel_Id != other.m_channel_Id)
    {
        return false;
    }
    if(m_channel_num != other.m_channel_num)
    {
        return false;
    }
    if(m_connect_initiator != other.m_connect_initiator)
    {
        return false;
    }
    if(m_call_type != other.m_call_type)
    {
        return false;
    }
    if(m_net_specific != other.m_net_specific)
    {
        return false;
    }
    if(m_prf_mode != other.m_prf_mode)
    {
        return false;
    }
    if(0 != strncmp(m_h243party_name, other.m_h243party_name, H243_NAME_LEN))
    {
        return false;
    }

    if(m_calling_party != other.m_calling_party)
    {
        return false;
    }
    if(m_called_party!= other.m_called_party)
    {
        return false;
    }
    return true;
}


/////////////////////////////////////////////////////////////////////////////
/*
char* CNetChanlCon::Serialize(WORD format)
{
 // assuming format = OPERATOR_MCMS  
 
 std::ostream*     pOstr;  
 char* msg_info = new char[SIZE_STREAM];
 pOstr = new std::ostream(msg_info,SIZE_STREAM);    
 Serialize(format, *pOstr); 
 int b=pOstr->pcount();
 char* msg = new char[SIZE_RECORD];
 memset(msg,' ', SIZE_RECORD);
 memcpy(msg, msg_info,b); 
 msg[b]='\0';
 msg[SIZE_RECORD-1]='\n';
 delete msg_info;
 //delete m_pOstr;
 PDELETE(pOstr);
 return msg;
}
*/

/////////////////////////////////////////////////////////////////////////////
CNetChanlCon::CNetChanlCon()
{
}

/////////////////////////////////////////////////////////////////////////////
CNetChanlCon::~CNetChanlCon()
{
}

/////////////////////////////////////////////////////////////////////////////
bool CNetChanlCon::operator == (const CNetChanlCon & rHnd)
{
    return ACCCDREventNetChannelConnect::operator==(rHnd);
}

/////////////////////////////////////////////////////////////////////////////
void CNetChanlCon::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{

  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
     m_ostr << "party name:"<<m_h243party_name << "\n"; 
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

     m_ostr << "party name:"<<tmp << "\n"; 
  }

  m_ostr << "party ID:"<<m_party_Id   << "\n"; 
  m_ostr << "channel ID:"<<(WORD)m_channel_Id   << "\n";     
  m_ostr << "channels number:"<<(WORD)m_channel_num   << "\n";     
		switch (m_connect_initiator) {
					case MCU_INITIATOR :{
    										m_ostr << "connect initiator:MCU" << "\n";
												break;
											}
					case REMOTE_PARTY_INITIATOR :{
    										m_ostr << "connect initiator:remote party" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch connect_initiator										
		switch (m_call_type) {
					case VOICE :{
    										m_ostr << "call type:voice" << "\n";
												break;
											}
					case MODEM :{
    										m_ostr << "call type:modem" << "\n";
												break;
									  	}
					case CALL_TYPE_56K :{
    										m_ostr << "call type:56K" << "\n";
												break;
											}
					case CALL_TYPE_64K :{
    										m_ostr << "call type:64K" << "\n";
												break;
									  	}
					case CALL_TYPE_64K_RESTRICT:{
    										m_ostr << "call type:64K restrict" << "\n";
												break;
											}
					case CALL_TYPE_384K :{
    										m_ostr << "call type:384K" << "\n";
												break;
									  	}
					case CALL_TYPE_384K_RESTRICT :{
    										m_ostr << "call type:384K restrict" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch call_type											
		switch (m_net_specific) {
					case NONE :{
    										m_ostr << "no net specification" << "\n";
												break;
											}
					case ATT_SDN :{
    										m_ostr << "net spec.:ATT SDN" << "\n";
												break;
									  	}
					case NTI_PRIVATE :{
    										m_ostr << "net spec.:NTI PRIVATE" << "\n";
												break;
											}
					case ATI_MEGACOM :{
    										m_ostr << "net spec.:ATI MEGACOM" << "\n";
												break;
									  	}
					case NTI_OUTWATS :{
    										m_ostr << "net spec.:NTI OUTWATS" << "\n";
												break;
									  	}
					case NTI_FX:{
    										m_ostr << "net spec.:NTI FX" << "\n";
												break;
											}
					case NTI_TIE_TRUNK :{
    										m_ostr << "net spec.:NTI TIE TRUNK" << "\n";
												break;
									  	}
					case ATT_ACCUNET :{
    										m_ostr << "net spec.:ATT ACCUNET" << "\n";
												break;
											}
					case ATT_1800 :{
    										m_ostr << "net spec.:ATT 1800" << "\n";
												break;
									  	}
					case NTI_TRO :{
    										m_ostr << "net spec.:NTI TRO" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch net spec											
		switch (m_prf_mode) {
					case NO_PRF :{
    										m_ostr << "NO PRF"<< "\n";
												break;
											}
					case PRF_MODE_PREFERRED :{
    										m_ostr << "PRF mode:preferred" << "\n";
												break;
									  	}
					case PRF_MODE_EXCLUSIVE :{
    										m_ostr << "PRF mode:exclusive" << "\n";
												break;
											}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch prf_mode				
  ((CCallingParty&)m_calling_party).Serialize(format, m_ostr, bilflag);
	//m_ostr <<"\n";
  ((CCalledParty&)m_called_party).Serialize(format, m_ostr, bilflag);
	m_ostr <<"\n";
}	
 
/////////////////////////////////////////////////////////////////////////////
void CNetChanlCon::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
     m_ostr << m_h243party_name << ","; 
  else{
	 char tmp[H243_NAME_LEN];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN);
	 tmp[H243_NAME_LEN-1]='\0';

	 m_ostr << tmp << ","; 
  }
  m_ostr << m_party_Id   << ","; 
  m_ostr << (WORD)m_channel_Id   << ",";     
  m_ostr << (WORD)m_channel_num   << ",";     
  m_ostr << (WORD)m_connect_initiator   << ",";     
  m_ostr << (DWORD)m_call_type   << ",";     
  m_ostr << (WORD)m_net_specific   << ",";     
  m_ostr << (WORD)m_prf_mode   << ",";
  
  ((CCallingParty&)m_calling_party).Serialize(format, m_ostr);
  ((CCalledParty&)m_called_party).Serialize(format, m_ostr);
} 

/////////////////////////////////////////////////////////////////////////////
void CNetChanlCon::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  
  //m_istr.ignore(1);
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_istr.getline(m_h243party_name,H243_NAME_LEN+1,',');
  else
	 m_istr.getline(m_h243party_name,H243_NAME_LEN_OLD+1,',');

  m_istr >> m_party_Id;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_channel_Id=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_channel_num=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_connect_initiator=(BYTE)tmp;
  m_istr.ignore(1);
  
  // tmp_yurir
  DWORD tmpDWORD=0;
  m_istr >>tmpDWORD;
  PASSERT_AND_RETURN(!m_istr);
  m_call_type = tmpDWORD;
  
  m_istr.ignore(1);
  m_istr >> tmp;
  m_net_specific=(BYTE)tmp; 
  m_istr.ignore(1);
  m_istr >> tmp;
  m_prf_mode=(BYTE)tmp;
  m_istr.ignore(1);
  
  ((CCallingParty&)m_calling_party).DeSerialize(format, m_istr);
//  m_istr.ignore(1);
  ((CCalledParty&)m_called_party).DeSerialize(format, m_istr);
  m_istr.ignore(2);
} 

/////////////////////////////////////////////////////////////////////////////
void CNetChanlCon::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pNetChanConnectNode = pFatherNode->AddChildNode("NET_CHANNEL_CONNECT");

	pNetChanConnectNode->AddChildNode("NAME",m_h243party_name);
	pNetChanConnectNode->AddChildNode("PARTY_ID",m_party_Id);
	pNetChanConnectNode->AddChildNode("CHANNEL_ID",m_channel_Id);
	pNetChanConnectNode->AddChildNode("CHANNEL_NUM",m_channel_num);
	pNetChanConnectNode->AddChildNode("INITIATOR",m_connect_initiator,INITIATOR_ENUM);
	pNetChanConnectNode->AddChildNode("NET_SPECIFIC",m_net_specific,NET_SPECIFIC_TYPE_ENUM);
	pNetChanConnectNode->AddChildNode("PREFERRED",m_prf_mode,PREFERRED_TYPE_ENUM);
	pNetChanConnectNode->AddChildNode("CALL_TYPE",m_call_type,CHANNEL_CALL_TYPE_ENUM);

	((CCallingParty&)m_calling_party).SerializeXml(pNetChanConnectNode); 
	((CCalledParty&)m_called_party).SerializeXml(pNetChanConnectNode);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CNetChanlCon::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243party_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_party_Id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"CHANNEL_ID",&m_channel_Id,_0_TO_30_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"CHANNEL_NUM",&m_channel_num,_0_TO_30_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"INITIATOR",&m_connect_initiator,INITIATOR_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"NET_SPECIFIC",&m_net_specific,NET_SPECIFIC_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"PREFERRED",&m_prf_mode,PREFERRED_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"CALL_TYPE",&m_call_type,CHANNEL_CALL_TYPE_ENUM);

	CXMLDOMElement *pChild = NULL;

	GET_CHILD_NODE(pActionNode, "CALLING_PARTY", pChild);
	if(pChild)
	{
		nStatus = ((CCallingParty&)m_calling_party).DeSerializeXml(pChild, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "CALLED_PARTY", pChild);
	if( pChild )
	{
		((CCalledParty&)m_called_party).DeSerializeXml(pChild, pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CNetChanlCon::NameOf() const                
{
  return "CNetChanlCon";
}

/////////////////////////////////////////////////////////////////////////////
void  CNetChanlCon::SetPartyName(const char* h243partyname)                  
{ 
   strncpy(m_h243party_name, h243partyname, sizeof(m_h243party_name) - 1);
   m_h243party_name[sizeof(m_h243party_name) - 1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventNetChannelConnect::GetPartyName() const                 
{
    return m_h243party_name;
}


////////////////////////////////////////////////////////////////////////////
void   CNetChanlCon::SetPartyId(const DWORD partyid)                 
{
     m_party_Id=partyid;
}

////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDREventNetChannelConnect::GetPartyId() const
{
    return m_party_Id;
}

////////////////////////////////////////////////////////////////////////////
void CNetChanlCon::SetChanlId(const BYTE chanlid)
{
   m_channel_Id=chanlid;
}
//////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCDREventNetChannelConnect::GetChanlId() const
{
    return m_channel_Id;
}
////////////////////////////////////////////////////////////////////////////
void CNetChanlCon::SetChanlNum(const BYTE chanlNum)
{
   m_channel_num=chanlNum;
}
//////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCDREventNetChannelConnect::GetChanlNum() const
{
    return m_channel_num;
}

///////////////////////////////////////////////////////////////////////////////		               
void   CNetChanlCon::SetConctIniator(const BYTE conct_initiator)
{
  m_connect_initiator=conct_initiator;
}
///////////////////////////////////////////////////////////////////////////////	
BYTE ACCCDREventNetChannelConnect::GetConctIniator() const
{
   return m_connect_initiator;
}
///////////////////////////////////////////////////////////////////////////////
void   CNetChanlCon::SetCallType(const DWORD calltype)
{
    m_call_type=calltype;
}
////////////////////////////////////////////////////////////////////////////////		
DWORD  ACCCDREventNetChannelConnect::GetCallType() const
{
  return  m_call_type;
}
///////////////////////////////////////////////////////////////////////////////	                
void   CNetChanlCon::SetNetSpec(const BYTE net_specific)
{
  m_net_specific=net_specific;
}
////////////////////////////////////////////////////////////////////////////////	
BYTE  ACCCDREventNetChannelConnect::GetNetSpec() const
{
   return m_net_specific;
}
////////////////////////////////////////////////////////////////////////////////	 			
void   CNetChanlCon::SetPrfMode (const BYTE prf_mode)
{
   m_prf_mode=prf_mode;
}

/////////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCDREventNetChannelConnect::GetPrfMode() const
{
  return m_prf_mode;
}
/////////////////////////////////////////////////////////////////////////////////
void  CNetChanlCon::SetCallingParty(const ACCCallingParty &other)
{
  m_calling_party=other;
}
/////////////////////////////////////////////////////////////////////////////////
const ACCCallingParty* ACCCDREventNetChannelConnect::GetCallingParty() const
{
   return &m_calling_party;
}
/////////////////////////////////////////////////////////////////////////////////
void  CNetChanlCon::SetCalledParty(const ACCCalledParty &other)
{
  m_called_party=other;
}
/////////////////////////////////////////////////////////////////////////////////
// For CDR : 
// dial type get value from range {DIAL_OUT (0), DIAL_IN(5)} while CNetChanlCon::m_connect_initiator
// need to get value from range {MCU_INITIATOR(0), REMOTE_PARTY_INITIATOR(1)}
BYTE  CNetChanlCon::ConvertConnectionTypeToConnectInitiatorType(BYTE connectionType) const
{
	if ( connectionType == DIAL_OUT)
		return MCU_INITIATOR;
	else
		return REMOTE_PARTY_INITIATOR;
	
}
/////////////////////////////////////////////////////////////////////////////////
const ACCCalledParty* ACCCDREventNetChannelConnect::GetCalledParty() const
{
   return &m_called_party;
}
/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//class CNetChanlCon

ACCCDREventMPIChannelConnect::ACCCDREventMPIChannelConnect()
{
	m_h243party_name[0]='\0';
	m_party_Id=0xffffffff;
	m_channel_Id=0xff;
	m_channel_num=0xff;
	m_connect_initiator=0xff;
	m_resrict=0;
		
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventMPIChannelConnect::ACCCDREventMPIChannelConnect(const ACCCDREventMPIChannelConnect &other)
{
	*this=other;
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventMPIChannelConnect::~ACCCDREventMPIChannelConnect()
{
}
ACCCDREventMPIChannelConnect& ACCCDREventMPIChannelConnect::operator = (const ACCCDREventMPIChannelConnect& other)
{
	strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN); 
	m_party_Id = other.m_party_Id;
	m_channel_Id = other.m_channel_Id;
	m_connect_initiator=other.m_connect_initiator;
	m_resrict=other.m_resrict;
	m_called_party=other.m_called_party;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CMPIChanlCon::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
     m_ostr << m_h243party_name << ","; 
  else{
	 char tmp[H243_NAME_LEN];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN);
	 tmp[H243_NAME_LEN-1]='\0';

	 m_ostr << tmp << ","; 
  }
  m_ostr << m_party_Id   << ","; 
  m_ostr << (WORD)m_channel_num   << ","; 
  m_ostr << (WORD)m_channel_Id   << ",";     
  m_ostr << (WORD)m_connect_initiator   << ",";     
  m_ostr << m_resrict   << ",";     
  ((CCalledParty&)m_called_party).Serialize(format, m_ostr);
} 
/////////////////////////////////////////////////////////////////////////////

CMPIChanlCon::CMPIChanlCon()
{
}
/////////////////////////////////////////////////////////////////////////////

CMPIChanlCon::~CMPIChanlCon()
{
}
/////////////////////////////////////////////////////////////////////////////
void CMPIChanlCon::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{

  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
     m_ostr << "party name:"<<m_h243party_name << "\n"; 
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

     m_ostr << "party name:"<<tmp << "\n"; 
  }

  m_ostr << "party ID:"<<m_party_Id   << "\n"; 
  m_ostr << "channels number:"<<(WORD) m_channel_num   << "\n"; 
  m_ostr << "channel ID:"<<(WORD)m_channel_Id   << "\n";     
		switch ((WORD)m_connect_initiator) {
					case MCU_INITIATOR :{
    										m_ostr << "connect initiator:MCU" << "\n";
												break;
											}
					case REMOTE_PARTY_INITIATOR :{
    										m_ostr << "connect initiator:remote party" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch connect_initiator										
		switch (m_resrict) {
					case 0 :{
    										m_ostr << "restrict:NO" << "\n";
												break;
											}
					case 1 :{
    										m_ostr << "restrict:YES" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch call_type											
		
		((CCalledParty&)m_called_party).Serialize(format, m_ostr, bilflag);
	m_ostr <<"\n";
}	
 
/////////////////////////////////////////////////////////////////////////////
void CMPIChanlCon::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  
  
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_istr.getline(m_h243party_name,H243_NAME_LEN+1,',');
  else
	 m_istr.getline(m_h243party_name,H243_NAME_LEN_OLD+1,',');

  m_istr >> m_party_Id;
   m_istr.ignore(1);
  m_istr >> tmp;
  m_channel_Id=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_channel_num=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_connect_initiator=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> m_resrict;
  m_istr.ignore(1);
  ((CCalledParty&)m_called_party).DeSerialize(format, m_istr);
  m_istr.ignore(2);  
} 

/////////////////////////////////////////////////////////////////////////////
void CMPIChanlCon::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pMPIChanConnectNode = pFatherNode->AddChildNode("MPI_CHANNEL_CONNECT");

	pMPIChanConnectNode->AddChildNode("NAME",m_h243party_name);
	pMPIChanConnectNode->AddChildNode("PARTY_ID",m_party_Id);
	pMPIChanConnectNode->AddChildNode("CHANNEL_ID",m_channel_Id);
	pMPIChanConnectNode->AddChildNode("CHANNEL_NUM",m_channel_num);
	pMPIChanConnectNode->AddChildNode("INITIATOR",m_connect_initiator,INITIATOR_ENUM);
	pMPIChanConnectNode->AddChildNode("RESTRICT_MODE",m_resrict,RESTRICT_MODE_ENUM);

	CXMLDOMElement* pPartyNode = pMPIChanConnectNode->AddChildNode("CALLED_PARTY");

	pPartyNode->AddChildNode("NUM_TYPE",m_called_party.GetCalledNumType(),NUM_TYPE_ENUM);
	pPartyNode->AddChildNode("PLAN_TYPE",m_called_party.GetCalledNumPlan(),NUM_PLAN_TYPE_ENUM);
	pPartyNode->AddChildNode("PHONE1",m_called_party.GetCalledPhoneNum());
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CMPIChanlCon::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement *pChild = NULL;
	BYTE  bDummy;
	char* pszDummy = NULL;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243party_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_party_Id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"CHANNEL_ID",&m_channel_Id,_0_TO_30_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"CHANNEL_NUM",&m_channel_num,_0_TO_30_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"INITIATOR",&m_connect_initiator,INITIATOR_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"RESTRICT_MODE",&m_resrict,RESTRICT_MODE_ENUM);

	GET_CHILD_NODE(pActionNode, "CALLED_PARTY", pChild);
	if( pChild ) {

		GET_VALIDATE_CHILD(pChild,"NUM_TYPE",&bDummy,NUM_TYPE_ENUM);
		CCalledParty(m_called_party).SetCalledNumType(bDummy);

		GET_VALIDATE_CHILD(pChild,"PLAN_TYPE",&bDummy,NUM_PLAN_TYPE_ENUM);
		CCalledParty(m_called_party).SetCalledNumPlan(bDummy);

		GET_VALIDATE_CHILD(pChild,"PHONE1",&pszDummy,PRI_LIMIT_PHONE_DIGITS_LENGTH);
		CCalledParty(m_called_party).SetCalledPhoneNum(pszDummy);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CMPIChanlCon::NameOf() const                
{
  return "CMPIChanlCon";
}
////////////////////////////////////////////////////////////////////////////
				
void CMPIChanlCon::SetChanlNum(const BYTE chanlNum)
{
   m_channel_num=chanlNum;
}
//////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCDREventMPIChannelConnect::GetChanlNum() const
{
    return m_channel_num;
}

/////////////////////////////////////////////////////////////////////////////
void  CMPIChanlCon::SetPartyName(const char* h243partyname)                  
{ 
   strncpy(m_h243party_name, h243partyname, sizeof(m_h243party_name) - 1);
   m_h243party_name[sizeof(m_h243party_name) - 1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventMPIChannelConnect::GetPartyName() const                 
{
    return m_h243party_name;
}


////////////////////////////////////////////////////////////////////////////
void   CMPIChanlCon::SetPartyId(const DWORD partyid)                 
{
     m_party_Id=partyid;
}

////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDREventMPIChannelConnect::GetPartyId() const
{
    return m_party_Id;
}

////////////////////////////////////////////////////////////////////////////
				
void CMPIChanlCon::SetChanlId(const BYTE chanlid)
{
   m_channel_Id=chanlid;
}
//////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCDREventMPIChannelConnect::GetChanlId() const
{
    return m_channel_Id;
}
///////////////////////////////////////////////////////////////////////////////		               
void   CMPIChanlCon::SetConctIniator(const BYTE conct_initiator)
{
  m_connect_initiator=conct_initiator;
}
///////////////////////////////////////////////////////////////////////////////	
BYTE ACCCDREventMPIChannelConnect::GetConctIniator() const
{
   return m_connect_initiator;
}
///////////////////////////////////////////////////////////////////////////////
void   CMPIChanlCon::SetResrict(const DWORD resrict)
{
    m_resrict=resrict;
}
////////////////////////////////////////////////////////////////////////////////		
DWORD  ACCCDREventMPIChannelConnect::GetResrict() const
{
  return  m_resrict;
}
/////////////////////////////////////////////////////////////////////////////////
void  CMPIChanlCon::SetCalledParty(const ACCCalledParty &other)
{
  m_called_party=other;
}
/////////////////////////////////////////////////////////////////////////////////
const ACCCalledParty* ACCCDREventMPIChannelConnect::GetCalledParty() const
{
   return &m_called_party;
}

/////////////////////////////////////////////////////////////////////////////////
// For CDR : 
// dial type get value from range {DIAL_OUT (0), DIAL_IN(5)} while CMPIChanlCon::m_connect_initiator
// need to get value from range {MCU_INITIATOR(0), REMOTE_PARTY_INITIATOR(1)}
BYTE  CMPIChanlCon::ConvertConnectionTypeToConnectInitiatorType(BYTE connectionType) const
{
	if ( connectionType == DIAL_OUT)
		return MCU_INITIATOR;
	else
		return REMOTE_PARTY_INITIATOR;
}

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//class CATMChanlCon

ACCCDREventATMChannelConnect::ACCCDREventATMChannelConnect()
{
    m_h243party_name[0]='\0';
	m_party_Id=0xffffffff;
	m_channel_Id=0xff;
	m_channel_num=0xff;
    m_connect_initiator=0xff;
	m_resrict=0;
	m_atm_address[0]='\0';
		
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventATMChannelConnect::ACCCDREventATMChannelConnect(const ACCCDREventATMChannelConnect &other)
{
	*this=other;
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventATMChannelConnect::~ACCCDREventATMChannelConnect()
{
}
ACCCDREventATMChannelConnect& ACCCDREventATMChannelConnect::operator = (const ACCCDREventATMChannelConnect& other)
{
  strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN); 
  m_party_Id = other.m_party_Id;
  m_channel_Id = other.m_channel_Id;
  m_channel_num = other.m_channel_num;
  m_connect_initiator=other.m_connect_initiator;
  m_resrict=other.m_resrict;
  strncpy(m_atm_address,other.m_atm_address,20);
  m_called_party=other.m_called_party;
  return *this;
}
/*
/////////////////////////////////////////////////////////////////////////////
void CATMChanlCon::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
     m_ostr << m_h243party_name << ","; 
  else{
	 char tmp[H243_NAME_LEN];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN);
	 tmp[H243_NAME_LEN-1]='\0';

	 m_ostr << tmp << ","; 
  }
  m_ostr << m_party_Id   << ","; 
  m_ostr << (WORD)m_channel_Id   << ",";     
  m_ostr << (WORD)m_channel_num   << ",";     
  m_ostr << (WORD)m_connect_initiator   << ",";     
  m_ostr << m_resrict   << ",";     
  m_ostr << m_atm_address << ",";     
  ((CCalledParty&)m_called_party).Serialize(format, m_ostr);
} 
/////////////////////////////////////////////////////////////////////////////
void CATMChanlCon::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{

  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
     m_ostr << "party name:"<<m_h243party_name << "\n"; 
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

     m_ostr << "party name:"<<tmp << "\n"; 
  }

  m_ostr << "party ID:"<<m_party_Id   << "\n"; 
  m_ostr << "channel ID:"<<(WORD)m_channel_Id   << "\n";     
  m_ostr << "channels number:"<<(WORD)m_channel_num   << "\n";     
		switch (m_connect_initiator) {
					case MCU_INITIATOR :{
    										m_ostr << "connect initiator:MCU" << "\n";
												break;
											}
					case REMOTE_PARTY_INITIATOR :{
    										m_ostr << "connect initiator:remote party" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch connect_initiator										
		switch (m_resrict) {
					case 0 :{
    										m_ostr << "restrict:NO" << "\n";
												break;
											}
					case 1 :{
    										m_ostr << "restrict:YES" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch call_type
		m_ostr << "atm address:"<<m_atm_address   << "\n";
		
		((CCalledParty&)m_called_party).Serialize(format, m_ostr, bilflag);
	m_ostr <<"\n";
}	
 
/////////////////////////////////////////////////////////////////////////////
void CATMChanlCon::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  
  
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_istr.getline(m_h243party_name,H243_NAME_LEN+1,',');
  else
	 m_istr.getline(m_h243party_name,H243_NAME_LEN_OLD+1,',');

  m_istr >> m_party_Id;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_channel_Id=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_channel_num=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_connect_initiator=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> m_resrict;
  m_istr.ignore(1);
  m_istr.getline(m_atm_address,20+1,',');
  //m_istr.ignore(1);
  ((CCalledParty&)m_called_party).DeSerialize(format, m_istr);
   m_istr.ignore(2);
    
} 

/////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void CATMChanlCon::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pATMChanConnectNode = pFatherNode->AddChildNode("ATM_CHANNEL_CONNECT");

	pATMChanConnectNode->AddChildNode("NAME",m_h243party_name);
	pATMChanConnectNode->AddChildNode("PARTY_ID",m_party_Id);
	pATMChanConnectNode->AddChildNode("CHANNEL_ID",m_channel_Id);
	pATMChanConnectNode->AddChildNode("CHANNEL_NUM",m_channel_num);
	pATMChanConnectNode->AddChildNode("INITIATOR",m_connect_initiator,INITIATOR_ENUM);
	pATMChanConnectNode->AddChildNode("RESTRICT_MODE",m_resrict,RESTRICT_MODE_ENUM);
	pATMChanConnectNode->AddChildNode("ATM_ADDRESS",m_atm_address);

	CXMLDOMElement* pPartyNode = pATMChanConnectNode->AddChildNode("CALLED_PARTY");

	pPartyNode->AddChildNode("NUM_TYPE",m_called_party.GetCalledNumType(),NUM_TYPE_ENUM);
	pPartyNode->AddChildNode("PLAN_TYPE",m_called_party.GetCalledNumPlan(),NUM_PLAN_TYPE_ENUM);
	pPartyNode->AddChildNode("PHONE1",m_called_party.GetCalledPhoneNum());
}
//#endif

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CATMChanlCon::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243party_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_party_Id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"CHANNEL_ID",&m_channel_Id,_0_TO_30_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"CHANNEL_NUM",&m_channel_num,_0_TO_30_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"INITIATOR",&m_connect_initiator,INITIATOR_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"RESTRICT_MODE",&m_resrict,RESTRICT_MODE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"ATM_ADDRESS",m_atm_address,ATM_ADDRESS);

	CXMLDOMElement* pChild = NULL;
	BYTE  bDummy;
	char* pszDummy = NULL;

	GET_CHILD_NODE(pActionNode, "CALLED_PARTY", pChild);
	if( pChild ) {

		GET_VALIDATE_CHILD(pChild,"NUM_TYPE",&bDummy,NUM_TYPE_ENUM);
		CCalledParty(m_called_party).SetCalledNumType(bDummy);

		GET_VALIDATE_CHILD(pChild,"PLAN_TYPE",&bDummy,NUM_PLAN_TYPE_ENUM);
		CCalledParty(m_called_party).SetCalledNumPlan(bDummy);

		GET_VALIDATE_CHILD(pChild,"PHONE1",&pszDummy,PRI_LIMIT_PHONE_DIGITS_LENGTH);
		CCalledParty(m_called_party).SetCalledPhoneNum(pszDummy);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

CATMChanlCon::CATMChanlCon()
{
}
/////////////////////////////////////////////////////////////////////////////

CATMChanlCon::~CATMChanlCon()
{
}
/////////////////////////////////////////////////////////////////////////////
const char*  CATMChanlCon::NameOf() const                
{
  return "CATMChanlCon";
}

/////////////////////////////////////////////////////////////////////////////
void  CATMChanlCon::SetPartyName(const char* h243partyname)                  
{ 
   int len=strlen(h243partyname);
   strncpy(m_h243party_name, h243partyname, H243_NAME_LEN );
   if (len>H243_NAME_LEN-1)
        m_h243party_name[H243_NAME_LEN-1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventATMChannelConnect::GetPartyName() const                 
{
    return m_h243party_name;
}


////////////////////////////////////////////////////////////////////////////
void   CATMChanlCon::SetPartyId(const DWORD partyid)                 
{
     m_party_Id=partyid;
}

////////////////////////////////////////////////////////////////////////////
const DWORD  ACCCDREventATMChannelConnect::GetPartyId() const                 
{
    return m_party_Id;
}

////////////////////////////////////////////////////////////////////////////
				
void CATMChanlCon::SetChanlId(const BYTE chanlid)
{
   m_channel_Id=chanlid;
}
//////////////////////////////////////////////////////////////////////////////	 
const BYTE  ACCCDREventATMChannelConnect::GetChanlId() const 
{
    return m_channel_Id;
}
////////////////////////////////////////////////////////////////////////////
				
void CATMChanlCon::SetChanlNum(const BYTE chanlNum)
{
   m_channel_num=chanlNum;
}
//////////////////////////////////////////////////////////////////////////////	 
const BYTE  ACCCDREventATMChannelConnect::GetChanlNum() const 
{
    return m_channel_num;
}

///////////////////////////////////////////////////////////////////////////////		               
void   CATMChanlCon::SetConctIniator(const BYTE conct_initiator)
{
  m_connect_initiator=conct_initiator;
}
///////////////////////////////////////////////////////////////////////////////	
const BYTE ACCCDREventATMChannelConnect::GetConctIniator() const 
{
   return m_connect_initiator;
}
///////////////////////////////////////////////////////////////////////////////
void   CATMChanlCon::SetResrict(const DWORD resrict)
{
    m_resrict=resrict;
}
////////////////////////////////////////////////////////////////////////////////		
const DWORD  ACCCDREventATMChannelConnect::GetResrict() const
{
  return  m_resrict;
}
/////////////////////////////////////////////////////////////////////////////////
void  CATMChanlCon::SetCalledParty(const ACCCalledParty &other)
{
  m_called_party=other;
}
/////////////////////////////////////////////////////////////////////////////////
const ACCCalledParty* ACCCDREventATMChannelConnect::GetCalledParty() const
{
   return &m_called_party;
}
/////////////////////////////////////////////////////////////////////////////////
void  CATMChanlCon::SetATMaddress(const char* atm_address)                  
{ 
   int len=strlen(atm_address);
   strncpy(m_atm_address, atm_address, 20 );
   if (len>20)
        m_h243party_name[19]='\0';
		
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventATMChannelConnect::GetATMaddress() const                 
{
    return m_atm_address;
}

/////////////////////////////////////////////////////////////////////////////////
// For CDR : 
// dial type get value from range {DIAL_OUT (0), DIAL_IN(5)} while CATMChanlCon::m_connect_initiator
// need to get value from range {MCU_INITIATOR(0), REMOTE_PARTY_INITIATOR(1)}
BYTE  CATMChanlCon::ConvertConnectionTypeToConnectInitiatorType(BYTE connectionType) const
{
	if ( connectionType == DIAL_OUT)
		return MCU_INITIATOR;
	else
		return REMOTE_PARTY_INITIATOR;
}
*/

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//class C323ChanlCon

ACCCDREvent323ChannelConnect::ACCCDREvent323ChannelConnect()
{
        m_h243party_name[0]       ='\0';
		m_party_Id                =0xffffffff;
	    m_connect_initiator       =0xff;
	    m_IpminRate		      =0xFFFFFFFF;
		m_IpmaxRate		      =0;
    	m_IpsrcPartyAddress[0]  ='\0';
	    m_IpdestPartyAddress[0] ='\0';
    	m_IpendpointType		  =0;
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREvent323ChannelConnect::ACCCDREvent323ChannelConnect(const ACCCDREvent323ChannelConnect &other)
{
	*this=other;

}
/////////////////////////////////////////////////////////////////////////////
ACCCDREvent323ChannelConnect::~ACCCDREvent323ChannelConnect()
{
}
ACCCDREvent323ChannelConnect& ACCCDREvent323ChannelConnect::operator = (const ACCCDREvent323ChannelConnect& other)
{
  strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN); 
  m_party_Id = other.m_party_Id;
  m_connect_initiator=other.m_connect_initiator;
  m_IpminRate	=other.m_IpminRate;
  m_IpmaxRate	=other.m_IpmaxRate;
  strncpy(m_IpsrcPartyAddress,other.m_IpsrcPartyAddress,IP_LIMIT_ADDRESS_CHAR_LEN); 
  strncpy(m_IpdestPartyAddress,other.m_IpdestPartyAddress,IP_LIMIT_ADDRESS_CHAR_LEN); 
  m_IpendpointType=other.m_IpendpointType;
  return *this;
}
//*/
/////////////////////////////////////////////////////////////////////////////
/*
char* CNetChanlCon::Serialize(WORD format)
{
 // assuming format = OPERATOR_MCMS  
 
 std::ostream*     pOstr;  
 char* msg_info = new char[SIZE_STREAM];
 pOstr = new COstrStream(msg_info,SIZE_STREAM);    
 Serialize(format, *pOstr); 
 int b=pOstr->pcount();
 char* msg = new char[SIZE_RECORD];
 memset(msg,' ', SIZE_RECORD);
 memcpy(msg, msg_info,b); 
 msg[b]='\0';
 msg[SIZE_RECORD-1]='\n';
 delete msg_info;
 //delete m_pOstr;
 PDELETE(pOstr);
 return msg;
}
*/
/////////////////////////////////////////////////////////////////////////////
void CIpChanlCon::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
     m_ostr << m_h243party_name << ","; 
  else{
	 char tmp[H243_NAME_LEN];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN);
	 tmp[H243_NAME_LEN-1]='\0';

	 m_ostr << tmp << ","; 
  }
  m_ostr << m_party_Id   << ","; 
  m_ostr << (WORD)m_connect_initiator   << ",";  
  m_ostr <<m_IpminRate	<< ",";  
  m_ostr <<m_IpmaxRate	<< ",";  
  m_ostr <<m_IpsrcPartyAddress  << ",";  
  m_ostr <<m_IpdestPartyAddress << ",";  
  m_ostr <<(WORD)m_IpendpointType		 << ";\n";  


 } 
/////////////////////////////////////////////////////////////////////////////
void CIpChanlCon::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{
	
	if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
		m_ostr << "party name:"<<m_h243party_name << "\n"; 
	else{
		char tmp[H243_NAME_LEN_OLD];
		strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
		tmp[H243_NAME_LEN_OLD-1]='\0';
		
		m_ostr << "party name:"<<tmp << "\n"; 
	}
	
	m_ostr << "party ID:"<<m_party_Id   << "\n"; 
	switch ((WORD)m_connect_initiator) {
	case MCU_INITIATOR :{
		m_ostr << "connect initiator:MCU" << "\n";
		break;
						}
	case REMOTE_PARTY_INITIATOR :{
		m_ostr << "connect initiator:remote party" << "\n";
		break;
								 }
	default :{
		m_ostr<<"--"<<"\n";
		break;
			 }
	}//endswitch connect_initiator		
	
	m_ostr << "min rate:"<<m_IpminRate << "\n"; 
	   m_ostr << "max rate:"<<m_IpmaxRate << "\n"; 
	   m_ostr << "source_party_address:"<<m_IpsrcPartyAddress << "\n"; 
	   m_ostr << "destination_party_address:"<<m_IpdestPartyAddress << "\n"; 
	   m_ostr << "endpoint_type:"<<(WORD)m_IpendpointType << ";\n\n"; 
}	
 
/////////////////////////////////////////////////////////////////////////////
void CIpChanlCon::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  WORD tmp;
  //m_istr.ignore(1);
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_istr.getline(m_h243party_name,H243_NAME_LEN+1,',');
  else
	 m_istr.getline(m_h243party_name,H243_NAME_LEN_OLD+1,',');
  m_istr >> m_party_Id;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_connect_initiator=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> m_IpminRate;
  m_istr.ignore(1);
  m_istr >> m_IpmaxRate;
  m_istr.ignore(1);
  m_istr.getline(m_IpsrcPartyAddress,IP_LIMIT_ADDRESS_CHAR_LEN+1,',');
  m_istr.getline(m_IpdestPartyAddress,IP_LIMIT_ADDRESS_CHAR_LEN+1,',');
  m_istr >> tmp;
  m_IpendpointType=(BYTE)tmp;
  m_istr.ignore(1);
 
 
} 

/////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void CIpChanlCon::SerializeXml(CXMLDOMElement* pFatherNode,WORD nEventType)
{
	CXMLDOMElement* pIpCallSetupNode = NULL;
	switch( nEventType )
	{
		case H323_CALL_SETUP:
			pIpCallSetupNode = pFatherNode->AddChildNode("H323_CALL_SETUP");
			break;

		case SIP_CALL_SETUP:
			pIpCallSetupNode = pFatherNode->AddChildNode("SIP_CALL_SETUP");
			break;
	}

	if( pIpCallSetupNode ) {

		pIpCallSetupNode->AddChildNode("NAME",m_h243party_name);
		pIpCallSetupNode->AddChildNode("PARTY_ID",m_party_Id);
		pIpCallSetupNode->AddChildNode("INITIATOR",m_connect_initiator,INITIATOR_ENUM);
		pIpCallSetupNode->AddChildNode("MIN_RATE",m_IpminRate);
		pIpCallSetupNode->AddChildNode("MAX_RATE",m_IpmaxRate);
		pIpCallSetupNode->AddChildNode("SOURCE_ADDRESS",m_IpsrcPartyAddress);
		pIpCallSetupNode->AddChildNode("DESTINATION_ADDRESS",m_IpdestPartyAddress);
		pIpCallSetupNode->AddChildNode("END_POINT_TYPE",m_IpendpointType,END_POINT_TYPE_ENUM);
	}
}
//#endif

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CIpChanlCon::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243party_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_party_Id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"INITIATOR",&m_connect_initiator,INITIATOR_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"MIN_RATE",&m_IpminRate,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"MAX_RATE",&m_IpmaxRate,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"SOURCE_ADDRESS",m_IpsrcPartyAddress,_0_TO_IP_LIMIT_ADDRESS_CHAR_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"DESTINATION_ADDRESS",m_IpdestPartyAddress,_0_TO_IP_LIMIT_ADDRESS_CHAR_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"END_POINT_TYPE",&m_IpendpointType,END_POINT_TYPE_ENUM);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

CIpChanlCon::CIpChanlCon()
{
}
/////////////////////////////////////////////////////////////////////////////

CIpChanlCon::~CIpChanlCon()
{
}
/////////////////////////////////////////////////////////////////////////////
const char*  CIpChanlCon::NameOf() const                
{
  return "CIpChanlCon";
}

/////////////////////////////////////////////////////////////////////////////
void  CIpChanlCon::SetPartyName(const char* h243partyname)                  
{ 
   strncpy(m_h243party_name, h243partyname, sizeof(m_h243party_name) - 1);
   m_h243party_name[sizeof(m_h243party_name) - 1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREvent323ChannelConnect::GetPartyName() const                 
{
    return m_h243party_name;
}


////////////////////////////////////////////////////////////////////////////
void   CIpChanlCon::SetPartyId(const DWORD partyid)                 
{
     m_party_Id=partyid;
}

////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDREvent323ChannelConnect::GetPartyId() const
{
    return m_party_Id;
}

////////////////////////////////////////////////////////////////////////////
void   CIpChanlCon::SetConctIniator(const BYTE conct_initiator)
{
  m_connect_initiator=conct_initiator;
}
///////////////////////////////////////////////////////////////////////////////	
BYTE ACCCDREvent323ChannelConnect::GetConctIniator() const
{
   return m_connect_initiator;
}
///////////////////////////////////////////////////////////////////////////////
void  CIpChanlCon::SetSrcPartyAddress(const char * srcPartyAddr)                  
{ 
   strncpy(m_IpsrcPartyAddress, srcPartyAddr, sizeof(m_IpsrcPartyAddress) - 1);
   m_IpsrcPartyAddress[sizeof(m_IpsrcPartyAddress) - 1]='\0';
}

/////////////////////////////////////////////////////////////////////////////
void  CIpChanlCon::SetDstPartyAddress(const char * dstPartyAddr)                  
{ 
   strncpy(m_IpdestPartyAddress, dstPartyAddr, sizeof(m_IpdestPartyAddress) - 1);
   m_IpdestPartyAddress[IP_LIMIT_ADDRESS_CHAR_LEN-1]='\0';
}
////////////////////////////////////////////////////////////////////////////////      #i
