/*$Header: /MCMS/MAIN/subsys/mcmsoper/NETDISCO.CPP 12    25/11/01 14:13 Oshi $*/
//+========================================================================+
//                            NETDISCO.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       NETDISCO.CPP                                                |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#include "NetChannelDisco.h" 
#include "NetChannConn.h"
#include "CallPart.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"

  

extern const char* GetQ931CauseAsString(const int cause);

//////////////////////////////////////////////////////////////////////////////
// class CNetChannelDisco
CNetChannelDisco::CNetChannelDisco()
{
}
//////////////////////////////////////////////////////////////////////////////

CNetChannelDisco::~CNetChannelDisco()
{
}
//////////////////////////////////////////////////////////////////////////////

ACCCDREventNetChannelDisconnect::ACCCDREventNetChannelDisconnect()
{
  m_partyName[0]='\0';
	m_partId=0;
	m_channelId=0xFF; 
	m_disco_Init=0xff; 
		
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventNetChannelDisconnect::ACCCDREventNetChannelDisconnect(const ACCCDREventNetChannelDisconnect &other)
{
	*this=other;
	
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventNetChannelDisconnect::~ACCCDREventNetChannelDisconnect()
{
}
ACCCDREventNetChannelDisconnect& ACCCDREventNetChannelDisconnect::operator = (const ACCCDREventNetChannelDisconnect& other)
{
	strncpy(m_partyName,other.m_partyName,H243_NAME_LEN); 
	m_partId = other.m_partId;
	m_channelId = other.m_channelId;
	m_disco_Init=other.m_disco_Init;
	m_q931_cause=other.m_q931_cause;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
/*
char* CNetChannelDisco::Serialize(WORD format)
{
 // assuming format = OPERATOR_MCMS  
 
 COstrStream*     pOstr;  
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
void CNetChannelDisco::Serialize(WORD format, std::ostream &m_ostr ,DWORD apiNum)
{
   
 if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
    m_ostr <<m_partyName << ","; 
 
  else
  {
   char tmp[H243_NAME_LEN_OLD];
   strncpy(tmp,m_partyName,sizeof(tmp) - 1);
   tmp[sizeof(tmp) - 1]='\0';
   
   m_ostr <<  m_partyName << ",";
  }
  m_ostr << m_partId   << ",";     
  m_ostr << (WORD)m_channelId << ","; 
  m_ostr <<	(WORD)m_disco_Init << ",";
  m_q931_cause.Serialize(format, m_ostr);
	  
} 
 
/////////////////////////////////////////////////////////////////////////////
void CNetChannelDisco::Serialize(WORD format, std::ostream &m_ostr,BYTE fullformat ,DWORD apiNum)
{
  if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
    m_ostr << "party name:"<<m_partyName << "\n"; 
 
  else
  {
   char tmp[H243_NAME_LEN_OLD];
   strncpy(tmp,m_partyName,H243_NAME_LEN_OLD-1);
   tmp[H243_NAME_LEN_OLD-1]='\0';
   
   m_ostr <<  m_partyName << "\n";
  }
  
  m_ostr << "party ID:"<<m_partId   << "\n";     
  m_ostr << "channel ID:"<<(WORD)m_channelId << "\n"; 
		switch (m_disco_Init) {
					case MCU_DISCONCT_INIT :{
    										m_ostr << "MCU disconnect initiator"<< "\n";
												break;
											}
					case REMOTE_PARTY_DISCONCT_INIT :{
    										m_ostr << "remote party disconnect initiator" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch m_disco_Init		
  //m_ostr << "Q931 disconnect cause:"<< (GetQ931CauseAsString((int)m_q931_cause.GetDisconctCauseValue())) << ";\n\n";
	m_q931_cause.Serialize(format, m_ostr,fullformat);  
} 
/////////////////////////////////////////////////////////////////////////////
void CNetChannelDisco::DeSerialize(WORD format, std::istream &m_istr)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  m_istr.getline(m_partyName,H243_NAME_LEN+1,',');
  m_istr >> m_partId;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_channelId=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_disco_Init=(BYTE)tmp;
  m_istr.ignore(1);
  m_q931_cause.DeSerialize(format, m_istr);
  m_istr.ignore(1);
   
} 
/////////////////////////////////////////////////////////////////////////////
const char*  CNetChannelDisco::NameOf() const                
{
  return "CNetChannelDisco";
}

////////////////////////////////////////////////////////////////////////////
void   CNetChannelDisco::SetNetDiscoPartyName(const char* partyname)                 
{
   strncpy(m_partyName, partyname, sizeof(m_partyName) - 1);
   m_partyName[sizeof(m_partyName) - 1]= '\0';
}
/*
////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventNetChannelDisconnect::GetNetDiscoPartyName() const                 
{
    return m_partyName;
}
*/
////////////////////////////////////////////////////////////////////////////
				
void CNetChannelDisco::SetNetDiscoPartyId(const DWORD partyid)
{
   m_partId=partyid;
}
/*
//////////////////////////////////////////////////////////////////////////////	 
const DWORD  ACCCDREventNetChannelDisconnect::GetNetDiscoPartyId() const 
{
    return m_partId;
}*/
///////////////////////////////////////////////////////////////////////////////		               
void   CNetChannelDisco::SetNetDiscoChannelId(const BYTE channelid)
{
  m_channelId=channelid;
}

///////////////////////////////////////////////////////////////////////////////	
//const BYTE ACCCDREventNetChannelDisconnect::GetNetDiscoChannelId() const 
//{
//   return m_channelId;
//}
///////////////////////////////////////////////////////////////////////////////
void   CNetChannelDisco::SetNetDiscoInitiator(const BYTE disco_init)
{
    m_disco_Init=disco_init;
}
//////////////////////////////////////////////////////////////////////////////////		
//const BYTE ACCCDREventNetChannelDisconnect::GetNetDiscoInitiator() const
//{
//  return  m_disco_Init;
//}
///////////////////////////////////////////////////////////////////////////////
void   CNetChannelDisco::SetNetDiscoQ931(const ACCCDREventDisconnectCause &q931_cause)
{
    m_q931_cause=q931_cause;
}
////////////////////////////////////////////////////////////////////////////////		
const ACCCDREventDisconnectCause* ACCCDREventNetChannelDisconnect::GetCDisconctCause() const
{
  return  &m_q931_cause;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CNetChannelDisco::ShortSerialize(WORD format, std::ostream &m_ostr ,DWORD apiNum)
{
   
 if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
    m_ostr <<m_partyName << ","; 
 
  else
  {
   char tmp[H243_NAME_LEN_OLD];
   strncpy(tmp,m_partyName,H243_NAME_LEN_OLD-1);
   tmp[H243_NAME_LEN_OLD-1]='\0';
   
   m_ostr <<  m_partyName << ",";
  }
  m_ostr << m_partId   << ",";     
  m_ostr << (WORD)m_channelId << ","; 
  m_ostr <<	(WORD)m_disco_Init << ";\n";
  
	  
} 
 
/////////////////////////////////////////////////////////////////////////////
void CNetChannelDisco::ShortSerialize(WORD format, std::ostream &m_ostr,BYTE fullformat ,DWORD apiNum)
{
  if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
    m_ostr << "party name:"<<m_partyName << "\n"; 
 
  else
  {
   char tmp[H243_NAME_LEN_OLD];
   strncpy(tmp,m_partyName,H243_NAME_LEN_OLD-1);
   tmp[H243_NAME_LEN_OLD-1]='\0';
   
   m_ostr <<  m_partyName << "\n";
  }
  
  m_ostr << "party ID:"<<m_partId   << "\n";     
  m_ostr << "channel ID:"<<(WORD)m_channelId<< "\n"; 
		switch ((BYTE)m_disco_Init) {
					case MCU_DISCONCT_INIT :{
    										m_ostr << "MCU disconnect initiator"<< "\n\n";
												break;
											}
					case REMOTE_PARTY_DISCONCT_INIT :{
    										m_ostr << "remote party disconnect initiator"  << ";\n\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n\n";
												break;
											}
			}//endswitch m_disco_Init		
  
																																		
	  
} 
/////////////////////////////////////////////////////////////////////////////
void CNetChannelDisco::ShortDeSerialize(WORD format, std::istream &m_istr , DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
 WORD tmp;

 // m_istr.ignore(1);

  if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
   m_istr.getline(m_partyName,H243_NAME_LEN+1,',');
  else
   m_istr.getline(m_partyName,H243_NAME_LEN_OLD+1,',');
  
  m_istr >> m_partId;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_channelId=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_disco_Init=(BYTE)tmp;
  m_istr.ignore(1);
   
} 
/////////////////////////////////////////////////////////////////////////////
void CNetChannelDisco::Short323Serialize(WORD format, std::ostream &m_ostr ,DWORD apiNum)
{
   
 if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
    m_ostr <<m_partyName << ","; 
 
  else
  {
   char tmp[H243_NAME_LEN_OLD];
   strncpy(tmp,m_partyName,H243_NAME_LEN_OLD-1);
   tmp[H243_NAME_LEN_OLD-1]='\0';
   
   m_ostr <<  m_partyName << ",";
  }
  m_ostr << m_partId   << ",";     
  m_ostr <<	(WORD)m_disco_Init << ";\n";
  
	  
} 
 
/////////////////////////////////////////////////////////////////////////////
void CNetChannelDisco::Short323Serialize(WORD format, std::ostream &m_ostr,BYTE fullformat ,DWORD apiNum)
{
  if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS) 
    m_ostr << "party name:"<<m_partyName << "\n"; 
 
  else
  {
   char tmp[H243_NAME_LEN_OLD];
   strncpy(tmp,m_partyName,H243_NAME_LEN_OLD-1);
   tmp[H243_NAME_LEN_OLD-1]='\0';
   
   m_ostr <<  m_partyName << "\n";
  }
  
  m_ostr << "party ID:"<<m_partId   << "\n";     
 
		switch (m_disco_Init) {
					case MCU_DISCONCT_INIT :{
    										m_ostr << "MCU disconnect initiator"<< "\n";
												break;
											}
					case REMOTE_PARTY_DISCONCT_INIT :{
    										m_ostr << "remote party disconnect initiator"  << ";\n\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch m_disco_Init		
  
																																		
	  
} 
/////////////////////////////////////////////////////////////////////////////
void CNetChannelDisco::Short323DeSerialize(WORD format, std::istream &m_istr , DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
 WORD tmp;

 // m_istr.ignore(1);

  if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
   m_istr.getline(m_partyName,H243_NAME_LEN+1,',');
  else
   m_istr.getline(m_partyName,H243_NAME_LEN_OLD+1,',');
  
  m_istr >> m_partId;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_disco_Init=(BYTE)tmp;
  m_istr.ignore(1);
   
} 

/////////////////////////////////////////////////////////////////////////////

void CNetChannelDisco::SerializeXml(CXMLDOMElement* pFatherNode,WORD nEventType)
{
	CXMLDOMElement* pChanDisconnectNode = NULL;
	
	switch(nEventType)
	{
		case NET_CHANNEL_DISCONNECTED:
			pChanDisconnectNode = pFatherNode->AddChildNode("NET_CHANNEL_DISCONNECTED");
			break;

		case ATM_CHANNEL_DISCONNECTED:
			pChanDisconnectNode = pFatherNode->AddChildNode("ATM_CHANNEL_DISCONNECTED");
			break;

		case MPI_CHANNEL_DISCONNECTED:
			pChanDisconnectNode = pFatherNode->AddChildNode("MPI_CHANNEL_DISCONNECTED");
			break;

		case H323_CLEAR_INDICATION:
			pChanDisconnectNode = pFatherNode->AddChildNode("H323_CLEAR_INDICATION");
			break;

		case SIP_CLEAR_INDICATION:
			pChanDisconnectNode = pFatherNode->AddChildNode("SIP_CLEAR_INDICATION");
			break;
	}

	if(pChanDisconnectNode)
	{
		pChanDisconnectNode->AddChildNode("NAME",m_partyName);
		pChanDisconnectNode->AddChildNode("PARTY_ID",m_partId);
		pChanDisconnectNode->AddChildNode("CHANNEL_ID",m_channelId);
		pChanDisconnectNode->AddChildNode("INITIATOR",m_disco_Init,INITIATOR_TYPE_ENUM);
		m_q931_cause.SerializeXml(pChanDisconnectNode);
	}
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CNetChannelDisco::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_partyName,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_partId,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"CHANNEL_ID",&m_channelId,_0_TO_30_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"INITIATOR",&m_disco_Init,INITIATOR_TYPE_ENUM);
	m_q931_cause.DeSerializeXml(pActionNode,pszError);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CDisconctCause

ACCCDREventDisconnectCause::ACCCDREventDisconnectCause()
{
  m_coding_stndrd=0;
	m_location=0;
	m_cause_value=0;
	
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventDisconnectCause::ACCCDREventDisconnectCause(const ACCCDREventDisconnectCause &other)
{
	*this=other;

}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventDisconnectCause::~ACCCDREventDisconnectCause()
{
}
/*
ACCCDREventDisconnectCause& ACCCDREventDisconnectCause::operator = (const ACCCDREventDisconnectCause& other)
{
	m_coding_stndrd = other.m_coding_stndrd;
	m_location = other.m_location;
	m_cause_value=other.m_cause_value;
	return *this;
}*/
/////////////////////////////////////////////////////////////////////////////
/*
char* CDisconctCause::Serialize(WORD format)
{
 // assuming format = OPERATOR_MCMS  
 
 COstrStream*     pOstr;  
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
/*
/////////////////////////////////////////////////////////////////////////////

CDisconctCause::CDisconctCause () 
{
}
/////////////////////////////////////////////////////////////////////////////

CDisconctCause::~CDisconctCause()
{
}
*/
/////////////////////////////////////////////////////////////////////////////
void ACCCDREventDisconnectCause::Serialize(WORD format, std::ostream &m_ostr)
{
   
  m_ostr << (WORD)m_coding_stndrd << ","; 
  m_ostr << (WORD)m_location   << ",";     
  m_ostr << (WORD)m_cause_value << ";\n";
  
} 
////////////////////////////////////////////////////////////////////////////////////
void ACCCDREventDisconnectCause::SerializeXml(CXMLDOMElement* pChanDisconnectNode)
{
	
	if(pChanDisconnectNode)
	{
		pChanDisconnectNode->AddChildNode("DISCONNECT_CODING",m_coding_stndrd,DISCONNECT_CODING_TYPE_ENUM);
		pChanDisconnectNode->AddChildNode("DISCONNECT_LOCATION",m_location,DISCONNECT_LOCATION_TYPE_ENUM);

		CXMLDOMElement* pDisconCauseNode = pChanDisconnectNode->AddChildNode("Q931_DISCONNECTION_CAUSE");

		pDisconCauseNode->AddChildNode("ID",m_cause_value);
		pDisconCauseNode->AddChildNode("DESCRIPTION",CCDRUtils::GetQ931CauseAsString((int)(m_cause_value)));
	}


}

/////////////////////////////////////////////////////////////////////////////
void ACCCDREventDisconnectCause::Serialize(WORD format, std::ostream &m_ostr,BYTE fullformat)
{
		switch (m_coding_stndrd) {
					case PRIcodCCIT :{
    										m_ostr << "PRI code standard:CCIT"<< "\n";
												break;
											}
					case PRIcodNATIONAL_STD :{
    										m_ostr << "PRI code standard: national" << "\n";
												break;
									  	}
					case PRIcodSTD_SPF_TO_LOC :{
    										m_ostr << "PRI code standard:specific to location" << "\n";
												break;
											}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
			}//endswitch m_coding_stndrd			
 		switch (m_location) {
					case PRIlocUSER :{
    										m_ostr << "location: local user" << "\n";
												break;
											}
					case PRIlocPVT_local :{
    										m_ostr << "location:private network serves local user "<< "\n";
												break;
									  	}
					case PRIlocPUB_LOCAL :{
    										m_ostr << "location:public network serves local user" << "\n";
												break;
											}
					case PRIlocTRANSIT_NET :{
    										m_ostr << "location:transit network" << "\n";
												break;
									  	}
					case PRIlocPUB_REMOTE :{
    										m_ostr << "location:public network serves remote user" << "\n";
												break;
									  	}
					case PRIlocPVT_REMOTE :{
    										m_ostr << "location:private network serves remote user" << "\n";
												break;
											}
					case PRIlocINTERNATIONAL :{
    										m_ostr << "location:international network" << "\n";
												break;
									  	}
					case PRIlocBEY_INTRWORK :{
    										m_ostr << "location:network beyond the interworking point" << "\n";
												break;
											}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}
	
		
		//m_ostr << "cause value:"<<(WORD)m_cause_value   << ";\n\n";
		m_ostr << "Q931 disconnect cause value:"<< (CCDRUtils::GetQ931CauseAsString((int)m_cause_value)) << ";\n\n";
} 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  ACCCDREventDisconnectCause::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{

	int nStatus = STATUS_FAIL;
	CXMLDOMElement *pChild;
	
	GET_VALIDATE_CHILD(pActionNode,"DISCONNECT_CODING",&m_coding_stndrd,DISCONNECT_CODING_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"DISCONNECT_LOCATION",&m_location,DISCONNECT_LOCATION_TYPE_ENUM);

	GET_CHILD_NODE(pActionNode, "Q931_DISCONNECTION_CAUSE", pChild);
	if( pChild ) 
		GET_VALIDATE_CHILD(pChild,"ID",&m_cause_value,_0_TO_BYTE);

	return nStatus;


}

/////////////////////////////////////////////////////////////////////////////
void ACCCDREventDisconnectCause::DeSerialize(WORD format, std::istream &m_istr)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;

  m_istr >> tmp;
  m_coding_stndrd=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_location=(BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_cause_value=(BYTE)tmp;
  
} 

/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventDisconnectCause::NameOf() const                
{
  return "ACCCDREventDisconnectCause";
}

////////////////////////////////////////////////////////////////////////////
void   ACCCDREventDisconnectCause::SetDisconctCauseCodeStandrd(const BYTE code_stndrd)                 
{
     m_coding_stndrd=code_stndrd;
}

////////////////////////////////////////////////////////////////////////////
BYTE  ACCCDREventDisconnectCause::GetDisconctCauseCodeStandrd() const
{
    return m_coding_stndrd;
}

////////////////////////////////////////////////////////////////////////////
				
void ACCCDREventDisconnectCause::SetDisconctCauseLocation(const BYTE location)
{
   m_location=location;
}
//////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCDREventDisconnectCause::GetDisconctCauseLocation() const
{
    return m_location;
}

/////////////////////////////////////////////////////////////////////////////
void  ACCCDREventDisconnectCause::SetDisconctCauseValue(const BYTE cause_value)
{
   m_cause_value=cause_value;
}
/////////////////////////////////////////////////////////////////////////////
BYTE  ACCCDREventDisconnectCause::GetDisconctCauseValue() const
{
    return m_cause_value;
}
