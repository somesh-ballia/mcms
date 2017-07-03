/*$Header: /MCMS/MAIN/subsys/mcmsoper/BILPARTY.CPP 19    2/20/02 1:16p Amirk $*/
//+========================================================================+
//                            BILPARTY.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BILPARTY.CPP                                                |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#include <stdio.h>
#include  "BilParty.h"
#include "DataTypes.h"
#include "CDRDefines.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "NStream.h"
#include "H221Str.h"
#include "StatusesGeneral.h"
#include "ConfPartySharedDefines.h"
#include "Macros.h"
#include "CDRUtils.h"
#include "ObjString.h"
#include "TraceStream.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////////////////
//CRemoteComMode

ACCCDREventRemoteCommMode::ACCCDREventRemoteCommMode()
{
	m_h243party_name[0]	= '\0';
	m_party_Id			= 0xffffffff;
	m_pRemoteCommMode 	= new CH221Str;
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventRemoteCommMode::ACCCDREventRemoteCommMode(const ACCCDREventRemoteCommMode &other)
{
  	m_party_Id 			= other.m_party_Id;
  	m_pRemoteCommMode 	= new CH221Str(*other.m_pRemoteCommMode);
  	strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN);
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventRemoteCommMode::~ACCCDREventRemoteCommMode()
{
	PDELETE(m_pRemoteCommMode);
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventRemoteCommMode& ACCCDREventRemoteCommMode::operator = (const ACCCDREventRemoteCommMode &other)
{
	if(&other != this)
	{
	  	m_party_Id 			= other.m_party_Id;
		*m_pRemoteCommMode 	= *other.m_pRemoteCommMode;
	  	strncpy(m_h243party_name,other.m_h243party_name, H243_NAME_LEN);
	}
  	return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool ACCCDREventRemoteCommMode::operator == (const ACCCDREventRemoteCommMode& other)
{
	if(&other == this)
	{
		return true;
	}

	bool res = (m_party_Id 			== other.m_party_Id 		&&
				*m_pRemoteCommMode 	== *other.m_pRemoteCommMode &&
	  			0 == strcmp(m_h243party_name,other.m_h243party_name));
	return res;
}

/////////////////////////////////////////////////////////////////////////////
CRemoteComMode::CRemoteComMode()
{
}

/////////////////////////////////////////////////////////////////////////////
CRemoteComMode::~CRemoteComMode()
{
}

/////////////////////////////////////////////////////////////////////////////
void CRemoteComMode::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  BYTE bilflag=1;
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << m_h243party_name << ",";
  else{
	  char tmp[H243_NAME_LEN_OLD];
	  strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	  tmp[H243_NAME_LEN_OLD-1]='\0';

      m_ostr << tmp << ",";
  }

  m_pRemoteCommMode->Serialize(format, m_ostr, bilflag);
   m_ostr << m_party_Id   << ";\n";

}
/////////////////////////////////////////////////////////////////////////////
void CRemoteComMode::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{
   if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
      m_ostr << "party name:"<<m_h243party_name << "\n";
   else{
	  char tmp[H243_NAME_LEN_OLD];
	  strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	  tmp[H243_NAME_LEN_OLD-1]='\0';

	  m_ostr << "party name:"<< tmp << "\n";
   }

   m_pRemoteCommMode->Serialize(format, m_ostr,(WORD)bilflag);
   m_ostr << "party ID:"<<m_party_Id   << ";\n\n";

}

/////////////////////////////////////////////////////////////////////////////
void CRemoteComMode::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
	BYTE bilflag=0;
  m_istr.ignore(1);
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_istr.getline(m_h243party_name,H243_NAME_LEN+1,',');
  else
	 m_istr.getline(m_h243party_name,H243_NAME_LEN_OLD+1,',');

  m_pRemoteCommMode->DeSerialize(format, m_istr,bilflag);
  m_istr >> m_party_Id;

}

/////////////////////////////////////////////////////////////////////////////
//
void CRemoteComMode::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pRemoteComModeNode = pFatherNode->AddChildNode("REMOTE_COMMUNICATION_MODE");

	pRemoteComModeNode->AddChildNode("NAME",m_h243party_name);
	pRemoteComModeNode->AddChildNode("PARTY_ID",m_party_Id);
	pRemoteComModeNode->AddChildNode("REMOTE_COMM_MODE",((char*)(m_pRemoteCommMode->GetPtr())));
}


/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CRemoteComMode::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243party_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_party_Id,_0_TO_DWORD);

	char* pszDummy = NULL;

	GET_VALIDATE_CHILD(pActionNode,"REMOTE_COMM_MODE",&pszDummy,UNLIMITED_CHAR_LENGTH);
	if (pszDummy)
		m_pRemoteCommMode->SetH221FromString(strlen(pszDummy),pszDummy);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CRemoteComMode::NameOf() const
{
  return "CRemoteComMode";
}

/////////////////////////////////////////////////////////////////////////////
void  CRemoteComMode::SetPartyName(const char* h243partyname)
{
   int len=strlen(h243partyname);
   strncpy(m_h243party_name, h243partyname, H243_NAME_LEN - 1);
   if (len>H243_NAME_LEN-1)
        m_h243party_name[H243_NAME_LEN-1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventRemoteCommMode::GetPartyName() const
{
    return m_h243party_name;
}



////////////////////////////////////////////////////////////////////////////
void   CRemoteComMode::SetPartyId(const DWORD partyid)
{
     m_party_Id=partyid;
}

////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDREventRemoteCommMode::GetPartyId() const
{
    return m_party_Id;
}

///////////////////////////////////////////////////////////////////////////////
void   CRemoteComMode::SetRemoteCommMode(const CH221Str &other)
{
  PDELETE(m_pRemoteCommMode);
  m_pRemoteCommMode = new CH221Str(other);
}
////////////////////////////////////////////////////////////////////////////////
//
const CH221Str*  ACCCDREventRemoteCommMode::GetRemoteCommMode() const
{
     return m_pRemoteCommMode;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//CpartyConnected

ACCCDREventPartyConnected::ACCCDREventPartyConnected()
{
	m_party_state		= 0xffffffff;
    m_second_cause		= 0xff;
	m_pCapabilities		= new CH221Str;
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyConnected::ACCCDREventPartyConnected(const ACCCDREventPartyConnected &other)
:ACCCDREventRemoteCommMode(other)
{
	m_party_state 		= other.m_party_state;
	m_second_cause 		= other.m_second_cause;
	m_pCapabilities 	= new CH221Str(*other.m_pCapabilities);
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyConnected::~ACCCDREventPartyConnected()
{
	PDELETE(m_pCapabilities);
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyConnected& ACCCDREventPartyConnected::operator = (const ACCCDREventPartyConnected& other)
{
	if(&other != this)
	{
		ACCCDREventRemoteCommMode::operator =(other);

		m_party_state 		= other.m_party_state;
		m_second_cause 		= other.m_second_cause;
		*m_pCapabilities 	= *other.m_pCapabilities;
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool ACCCDREventPartyConnected::operator == (const ACCCDREventPartyConnected& other)
{
	bool res = ACCCDREventRemoteCommMode::operator ==(other);
	if(false == res)
	{
		return res;
	}

	res = (	m_party_state == other.m_party_state &&
			m_second_cause == other.m_second_cause &&
			*m_pCapabilities == *other.m_pCapabilities);

	return res;
}

/////////////////////////////////////////////////////////////////////////////
CSvcSipPartyConnected::CSvcSipPartyConnected()
{
  	m_party_Id 			= -1;
  	m_h243party_name[0]	= '\0';
	m_party_state 		= -1;
	m_audioCodec		= -1;
	m_bitRateOut		= -1;
	m_bitRateIn			= -1;
}
CSvcSipPartyConnected::CSvcSipPartyConnected(const CSvcSipPartyConnected& other) : CPObject()
{
  	m_party_Id 			= other.m_party_Id;
  	strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN);
	m_party_state 		= other.m_party_state;
	m_listStreams		= other.m_listStreams;
	m_audioCodec		= other.m_audioCodec;
	m_bitRateOut		= other.m_bitRateOut;
	m_bitRateIn			= other.m_bitRateIn;
}
CSvcSipPartyConnected::~CSvcSipPartyConnected()
{
}

CSvcSipPartyConnected& CSvcSipPartyConnected::operator= (const CSvcSipPartyConnected& other)
{
  	m_party_Id 			= other.m_party_Id;
  	strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN);
	m_party_state 		= other.m_party_state;
	m_listStreams		= other.m_listStreams;
	m_audioCodec		= other.m_audioCodec;
	m_bitRateOut		= other.m_bitRateOut;
	m_bitRateIn			= other.m_bitRateIn;
	return *this;
}
//bool CSvcSipPartyConnected::operator==(const CSvcSipPartyConnected& other)
//{
//	if(&other == this)
//	{
//		return true;
//	}
//
//	bool res = (m_party_Id 		== other.m_party_Id 	&&
//				m_party_state	== other.m_party_state	&&
////				m_MediaList		== other.m_MediaList	&&
//	  			0 == strcmp(m_h243party_name,other.m_h243party_name));
//	return res;
//}

void CSvcSipPartyConnected::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{

}

void CSvcSipPartyConnected::Serialize(WORD format, std::ostream &ostr, DWORD apiNum)
{
	ostr << m_h243party_name << ","
		<< m_party_Id << ","
		<< m_party_state << ",";

	ostr << (int)(m_listStreams.size()) << ",";
	std::list<SvcStreamDesc>::const_iterator it = m_listStreams.begin();
	for ( ; it != m_listStreams.end(); ++it)
	{
		ostr << it->m_bitRate << ","
			<< it->m_frameRate << ","
			<< it->m_height << ","
			<< it->m_width << ",";
	}

	ostr << m_audioCodec << ","
		<< m_bitRateOut << ","
		<< m_bitRateIn << ";\n";
}

void CSvcSipPartyConnected::DeSerialize(WORD format, std::istream &istr, DWORD apiNum)
{
	istr.getline(m_h243party_name, H243_NAME_LEN, ',');
	istr >> m_party_Id; istr.ignore(1);
	istr >> m_party_state; istr.ignore(1);

	int n = 0;
	istr >> n; istr.ignore(1);
	m_listStreams.clear();
	for (int i=0; i < n; ++i)
	{
		SvcStreamDesc vstr;
		istr >> vstr.m_bitRate; istr.ignore(1);
		istr >> vstr.m_frameRate; istr.ignore(1);
		istr >> vstr.m_height; istr.ignore(1);
		istr >> vstr.m_width; istr.ignore(1);
		m_listStreams.push_back(vstr);
	}
	istr >> m_audioCodec; istr.ignore(1);
	istr >> m_bitRateOut; istr.ignore(1);
	istr >> m_bitRateIn;
        istr.ignore(1);
}

void CSvcSipPartyConnected::SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType)
{
	CXMLDOMElement* pSvcSipPartyConnectedNode = NULL;

	pSvcSipPartyConnectedNode = pFatherNode->AddChildNode("SVC_SIP_PARTY_CONNECTED");
	if (pSvcSipPartyConnectedNode)
	{
		pSvcSipPartyConnectedNode->AddChildNode("NAME", m_h243party_name);
		pSvcSipPartyConnectedNode->AddChildNode("PARTY_ID", m_party_Id);
		pSvcSipPartyConnectedNode->AddChildNode("SVC_PARTY_STATUS", m_party_state, ONGOING_PARTY_STATUS_ENUM);

		CXMLDOMElement* pReceiveLineRate = pSvcSipPartyConnectedNode->AddChildNode("RECEIVE_LINE_RATE");
		pReceiveLineRate->AddChildNode("NEGOTIATED", m_bitRateIn);

		CXMLDOMElement* pTransmitLineRate = pSvcSipPartyConnectedNode->AddChildNode("TRANSMIT_LINE_RATE");
		pTransmitLineRate->AddChildNode("NEGOTIATED", m_bitRateOut);

		CXMLDOMElement* pUpLinkVideoCaps = pSvcSipPartyConnectedNode->AddChildNode("UPLINK_VIDEO_CAPABILITIES");
		std::list<SvcStreamDesc>::const_iterator it = m_listStreams.begin();
		for (; it != m_listStreams.end(); ++it)
		{
			CXMLDOMElement* pStream = pUpLinkVideoCaps->AddChildNode("STREAM");
			pStream->AddChildNode("RESOLUTION_WIDTH", it->m_width);
			pStream->AddChildNode("RESOLUTION_HEIGHT", it->m_height);
			DWORD dw = it->m_frameRate / 256;
			if (dw * 256 == it->m_frameRate)
				pStream->AddChildNode("RELAY_MAX_FRAME_RATE", dw);
			else
			{
				char buff[10] = "";
				float tmp = ((float)(it->m_frameRate)) / 256.0;
				snprintf(buff, ARRAYSIZE(buff), "%.1f", tmp);
				pStream->AddChildNode("RELAY_MAX_FRAME_RATE", buff);
			}
			pStream->AddChildNode("RELAY_MAX_BIT_RATE", it->m_bitRate);
		}

		pSvcSipPartyConnectedNode->AddChildNode("AUDIO_CODEC", m_audioCodec, RELAY_CODEC_TYPE_ENUM);
	}
}

int CSvcSipPartyConnected::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	return STATUS_OK;
}
void CSvcSipPartyConnected::SetPartyName(const char* h243partyname)
{
   int len=strlen(h243partyname);
   strncpy(m_h243party_name, h243partyname, H243_NAME_LEN - 1);
   if (len>H243_NAME_LEN-1)
		m_h243party_name[H243_NAME_LEN-1]='\0';
}
void CSvcSipPartyConnected::SetPartyId(const DWORD partyid)
{
	m_party_Id = partyid;
}
void CSvcSipPartyConnected::SetPartyState(const DWORD partystate)
{
	m_party_state = partystate;
}
void CSvcSipPartyConnected::SetAudioCodec(const DWORD codec)
{
	m_audioCodec = codec;
}
void CSvcSipPartyConnected::SetBitRateOut(const DWORD dwBitRateOut)
{
	m_bitRateOut = dwBitRateOut;
}
void CSvcSipPartyConnected::SetBitRateIn(const DWORD dwBitRateIn)
{
	m_bitRateIn = dwBitRateIn;
}

void CSvcSipPartyConnected::SetStreams(const std::list<SvcStreamDesc>* pStreams)
{
	m_listStreams = *pStreams;
}

const char* CSvcSipPartyConnected::GetPartyName() const
{
	return m_h243party_name;
}
DWORD CSvcSipPartyConnected::GetPartyId() const
{
	return m_party_Id;
}
DWORD CSvcSipPartyConnected::GetPartyState() const
{
	return m_party_state;
}
DWORD CSvcSipPartyConnected::GetAudioCodec() const
{
	return m_audioCodec;
}
DWORD CSvcSipPartyConnected::GetBitRateOut() const
{
	return m_bitRateOut;
}
DWORD CSvcSipPartyConnected::GetBitRateIn() const
{
	return m_bitRateIn;
}

const std::list<SvcStreamDesc>* CSvcSipPartyConnected::GetStreams() const
{
	return &m_listStreams;
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnected::CPartyConnected()
{
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnected::CPartyConnected(const CPartyConnected &other)
:ACCCDREventPartyConnected(other)
{

}

/////////////////////////////////////////////////////////////////////////////
CPartyConnected& CPartyConnected::operator=(const CPartyConnected &other)
{
	if(&other != this)
	{
        ACCCDREventPartyConnected::operator =(other);
	}
	return *this;
}

bool CPartyConnected::operator == (const CPartyConnected &rHnd)
{
	if(&rHnd == this)
	{
		return true;
	}
	return ACCCDREventPartyConnected::operator ==(rHnd);
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnected::~CPartyConnected()
{
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnected::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  BYTE  bilflag=1;
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << m_h243party_name << ",";
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);

	 tmp[H243_NAME_LEN_OLD-1]='\0';

	 m_ostr << "party name:"<< tmp << "\n";
  }

  m_ostr << m_party_Id   << ",";
  m_ostr << m_party_state   << ",";
  m_pCapabilities->Serialize(format, m_ostr, bilflag);
  m_pRemoteCommMode->Serialize(format, m_ostr, bilflag);
  m_ostr << (WORD)m_second_cause  << ";\n";
}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnected::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{
	BYTE* ph221;
	DWORD len;

  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << "party name:"<<m_h243party_name << "\n";
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

	 m_ostr << "party name:"<< tmp << "\n";
  }
  m_ostr << "party ID:"<<m_party_Id   << "\n";
		switch (m_party_state) {
					case PARTY_CONNECTED :{
    										m_ostr << "party state: connected" << "\n";
												break;
											}
					case PARTY_SECONDARY :{
    										m_ostr << "party state: secondary" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch m_party_state

  //m_pCapabilities->Serialize(format, m_ostr, (WORD)bilflag);
	len=m_pCapabilities->GetLen();
	ph221=m_pCapabilities->GetPtr();
	BYTE capflag=0;
	CCDRUtils::FullDumpCap(ph221,len,m_ostr,capflag);

  //m_pRemoteCommMode->Serialize(format, m_ostr, (WORD)bilflag);
 	len=m_pRemoteCommMode->GetLen();
	ph221=m_pRemoteCommMode->GetPtr();
	capflag=1;
	//FullDumpCap(ph221,len,m_ostr,capflag);
    CCDRUtils::CdrDumpH221Stream(m_ostr,len,ph221);

	if(m_party_state==PARTY_SECONDARY)
	{
		switch((WORD)m_second_cause){
				case CAUSE_RESTRICT :{
									m_ostr << "secondary cause:Restrict"<< ";\n\n";
									break;
									}
				case SM_COMP :{
								m_ostr << "secondary cause:SM comp"<< ";\n\n";
								break;
								}
				case BIT_RATE :{
									m_ostr << "secondary cause:Bit rate"<< ";\n\n";
									break;
									}
				case CAUSE_LSD :{
								m_ostr << "secondary cause:LSD"<< ";\n\n";
								break;
								}
				case CAUSE_HSD :{
									m_ostr << "secondary cause:HSD"<< ";\n\n";
									break;
									}
				case CAUSE_MLP :{
								m_ostr << "secondary cause:MLP"<< ";\n\n";
								break;
								}
				case CAUSE_H_MLP :{
									m_ostr << "secondary cause:H-MLP"<< ";\n\n";
									break;
									}
				case CAUSE_QCIF :{
								m_ostr << "secondary cause:QCIF"<< ";\n\n";
								break;
								}
				case CAUSE_VIDEO_FRAME_RATE :{
									m_ostr << "secondary cause:Video frame rate"<< ";\n\n";
									break;
									}
				case CAUSE_H_243 :{
								m_ostr << "secondary cause:H243 bad behavior"<< ";\n\n";
								break;
								}
				case OTHER_CAUSE :{
    						       m_ostr << "secondary cause: Other" << ";\n\n";
								   break;
								 	}
				default :{
							m_ostr<<"--"<<";\n";
							break;
							}
		}//endswitch m_second_cause
	}
	else
		m_ostr <<";\n\n";

}

/////////////////////////////////////////////////////////////////////////////
void CPartyConnected::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  //m_istr.ignore(1);
  BYTE  bilflag=1;
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_istr.getline(m_h243party_name,H243_NAME_LEN+1,',');
  else
	 m_istr.getline(m_h243party_name,H243_NAME_LEN_OLD+1,',');

  m_istr >> m_party_Id;
  m_istr.ignore(1);
  m_istr >> m_party_state;
  m_istr.ignore(1);
  m_pCapabilities->DeSerialize(format, m_istr,bilflag);
  m_pRemoteCommMode->DeSerialize(format, m_istr,bilflag);
  WORD tmp;
  m_istr >> tmp;
  m_second_cause = (BYTE)tmp;
  m_istr.ignore(1);
}

/////////////////////////////////////////////////////////////////////////////
//
void CPartyConnected::SerializeXml(CXMLDOMElement* pFatherNode,WORD nEventType)
{
	CXMLDOMElement* pPartyConnectedNode = NULL;

	switch(nEventType)
	{
		case EVENT_PARTY_CONNECTED:
				pPartyConnectedNode = pFatherNode->AddChildNode("PARTY_CONNECTED");
				break;

		case IP_PARTY_CONNECTED:
				pPartyConnectedNode = pFatherNode->AddChildNode("H323_PARTY_CONNECTED");
				break;

		case SIP_PARTY_CONNECTED:
				pPartyConnectedNode = pFatherNode->AddChildNode("SIP_PARTY_CONNECTED");
				break;
	}

	if(pPartyConnectedNode)
	{
		pPartyConnectedNode->AddChildNode("NAME",m_h243party_name);
		pPartyConnectedNode->AddChildNode("PARTY_ID",m_party_Id);

		CXMLDOMElement* pPartyStatusNode = pPartyConnectedNode->AddChildNode("ONGOING_PARTY_STATUS");
		pPartyStatusNode->AddChildNode("ID",m_party_state);
		pPartyStatusNode->AddChildNode("DESCRIPTION",m_party_state,ONGOING_PARTY_STATUS_ENUM);

		CXMLDOMElement* pSecondaryCauseNode = pPartyConnectedNode->AddChildNode("SECONDARY_CAUSE");
		pSecondaryCauseNode->AddChildNode("ID",m_second_cause);
		pSecondaryCauseNode->AddChildNode("DESCRIPTION",m_second_cause,SECONDARY_CAUSE_ENUM);

		COstrStream* pOstr = NULL;

		switch( nEventType )
		{
			case EVENT_PARTY_CONNECTED: {
				pOstr = new COstrStream();
				CCDRUtils::FullDumpCap(m_pCapabilities->GetPtr(),m_pCapabilities->GetLen(),*pOstr,0);
				if(m_pCapabilities->GetLen())
					pPartyConnectedNode->AddChildNode("LOCAL_COMM_MODE", pOstr->str().c_str());
				else
					pPartyConnectedNode->AddChildNode("LOCAL_COMM_MODE","");
				POBJDELETE(pOstr);

				pOstr = new COstrStream();
				CCDRUtils::CdrDumpH221Stream(*pOstr,m_pRemoteCommMode->GetLen(),m_pRemoteCommMode->GetPtr());
				if(m_pRemoteCommMode->GetLen())
					pPartyConnectedNode->AddChildNode("REMOTE_COMM_MODE", pOstr->str().c_str());
				else
					pPartyConnectedNode->AddChildNode("REMOTE_COMM_MODE","");
				POBJDELETE(pOstr);

				break;
									}
			case IP_PARTY_CONNECTED:
			case SIP_PARTY_CONNECTED: {
				pOstr = new COstrStream();
				CCDRUtils::CdrDumpH323Cap(m_pCapabilities->GetPtr(), m_pCapabilities->GetLen(), *pOstr, 0);

				if(m_pCapabilities->GetLen())
					pPartyConnectedNode->AddChildNode("LOCAL_COMM_MODE", pOstr->str().c_str());
				else
					pPartyConnectedNode->AddChildNode("LOCAL_COMM_MODE","");
				POBJDELETE(pOstr);

				pOstr = new COstrStream();
				CCDRUtils::CdrDumpH323Cap(m_pRemoteCommMode->GetPtr(), m_pRemoteCommMode->GetLen(), *pOstr, 0);

				if(m_pRemoteCommMode->GetLen())
					pPartyConnectedNode->AddChildNode("REMOTE_COMM_MODE", pOstr->str().c_str());
				else
					pPartyConnectedNode->AddChildNode("REMOTE_COMM_MODE","");
				POBJDELETE(pOstr);

				break;
								 }
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CPartyConnected::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243party_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_party_Id,_0_TO_DWORD);

	CXMLDOMElement *pChild = NULL;

	GET_CHILD_NODE(pActionNode, "ONGOING_PARTY_STATUS", pChild);
	if (pChild)
		GET_VALIDATE_CHILD(pChild,"ID",&m_party_state,_0_TO_DWORD);

	GET_CHILD_NODE(pActionNode, "SECONDARY_CAUSE", pChild);
	if (pChild)
		GET_VALIDATE_CHILD(pChild,"ID",&m_second_cause,_0_TO_DWORD);

	char* pszDummy = NULL;
	GET_VALIDATE_CHILD(pActionNode,"LOCAL_COMM_MODE",&pszDummy,UNLIMITED_CHAR_LENGTH);
	if (pszDummy)
		m_pCapabilities->SetH221FromString(strlen(pszDummy),pszDummy);

	GET_VALIDATE_CHILD(pActionNode,"REMOTE_COMM_MODE",&pszDummy,UNLIMITED_CHAR_LENGTH);
	if (pszDummy)
		m_pRemoteCommMode->SetH221FromString(strlen(pszDummy),pszDummy);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CPartyConnected::NameOf() const
{
  return "CPartyConnected";
}

/////////////////////////////////////////////////////////////////////////////
void  CPartyConnected::SetPartyName(const char* h243partyname)
{
   int len=strlen(h243partyname);
   strncpy(m_h243party_name, h243partyname, H243_NAME_LEN - 1);
   if (len>H243_NAME_LEN-1)
        m_h243party_name[H243_NAME_LEN-1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventPartyConnected::GetPartyName() const
{
    return m_h243party_name;
}


////////////////////////////////////////////////////////////////////////////
void   CPartyConnected::SetPartyId(const DWORD partyid)
{
     m_party_Id=partyid;
}

////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDREventPartyConnected::GetPartyId() const
{
    return m_party_Id;
}

////////////////////////////////////////////////////////////////////////////

void CPartyConnected::SetPartyState(const DWORD partystate)
{
   m_party_state=partystate;
}
//////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDREventPartyConnected::GetPartyState() const
{
    return m_party_state;
}
///////////////////////////////////////////////////////////////////////////////
void   CPartyConnected::SetSecondaryCause(const BYTE second_cause)
{
  m_second_cause=second_cause;
}
///////////////////////////////////////////////////////////////////////////////
BYTE ACCCDREventPartyConnected::GetSecondaryCause() const
{
   return m_second_cause;
}
///////////////////////////////////////////////////////////////////////////////
void   CPartyConnected::SetCapabilities(const CH221Str&  other)
{
  PDELETE(m_pCapabilities);
  m_pCapabilities = new CH221Str(other);
}
////////////////////////////////////////////////////////////////////////////////
//
const CH221Str*  ACCCDREventPartyConnected::GetCapabilities() const
{
     return m_pCapabilities;
}
//#else
/*
void ACCCDREventPartyConnected::GetCapabilities(ACCH221Capabilities &H221)const
{
	H221=*m_pCapabilities;
}
*/

///////////////////////////////////////////////////////////////////////////////
void   CPartyConnected::SetRemoteCommMode(const CH221Str &other)
{
  PDELETE(m_pRemoteCommMode);
  m_pRemoteCommMode = new CH221Str(other);
}
////////////////////////////////////////////////////////////////////////////////
//
const CH221Str*  ACCCDREventPartyConnected::GetRemoteCommMode() const
{
     return m_pRemoteCommMode;
}
//#else
/*
void ACCCDREventPartyConnected::GetRemoteCommMode(ACCH221Capabilities &H221)const
{
	H221=*m_pRemoteCommMode;
}
*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//CPartyDisconctCuse

ACCCDREventPartyDisconnected::ACCCDREventPartyDisconnected()
{
	m_h243party_name[0]='\0';
	m_party_Id=0xffffffff;
	m_disconct_cause=0xffffffff;
	m_q931_disconct_cause=0xffffffff;

}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyDisconnected::ACCCDREventPartyDisconnected(const ACCCDREventPartyDisconnected &other)
{
	*this=other;
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyDisconnected::~ACCCDREventPartyDisconnected()
{
}

ACCCDREventPartyDisconnected& ACCCDREventPartyDisconnected::operator = (const ACCCDREventPartyDisconnected& other)
{
  strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN);
  m_party_Id = other.m_party_Id;
  m_disconct_cause = other.m_disconct_cause;
  m_q931_disconct_cause=other.m_q931_disconct_cause;
  return *this;
}


/////////////////////////////////////////////////////////////////////////////
CPartyDisconnected::CPartyDisconnected()
{
}

/////////////////////////////////////////////////////////////////////////////
CPartyDisconnected::~CPartyDisconnected()
{
}
/////////////////////////////////////////////////////////////////////////////
void CPartyDisconnected::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << m_h243party_name << ",";
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

	 m_ostr << "party name:"<< tmp << "\n";
  }

  m_ostr << m_party_Id   << ",";
  m_ostr << m_disconct_cause   << ",";
  m_ostr << m_q931_disconct_cause   << ";\n";

}
/////////////////////////////////////////////////////////////////////////////
void CPartyDisconnected::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << "party name:"<<m_h243party_name << "\n";
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

	 m_ostr << "party name:"<< tmp << "\n";
  }

  m_ostr << "party ID:"<<m_party_Id   << "\n";
  switch (m_disconct_cause) {
			case PARTY_HANG_UP :{
    						m_ostr << "disconnect cause:Party hang up" << "\n";
							break;
							}
			case DISCONNECTED_BY_OPERATOR :{
    						m_ostr << "disconnect cause:Disconnected by Operator" << "\n";
							break;
							}
			case DISCONNECTED_BY_CHAIR :{
    						m_ostr << "disconnect cause:Disconnected by Chair" << "\n";
							break;
							}
            /*Begin:added by Richer for BRIDGE-13006,2014.04.29*/
            case DISCONNECTED_BY_VIDEO_RECOVERY:{
                m_ostr << "disconnect cause:Disconnected by Video Recovery" << "\n";
                break;
                }
            /*End:added by Richer for BRIDGE-13006,2014.04.29*/
			case NO_ESTABL_H243_CONNECT :{
    						m_ostr << "disconnect cause:No H243 connection" << "\n";
							break;
							}
			case SUB_OR_MAIN_LINK_IS_SECONDARY:
                 // no break!
			case RESOURCES_DEFICIENCY :{
    						m_ostr << "disconnect cause:Resources deficiencies " << "\n";
							break;
							}
			case PASSWORD_FAILURE :{
    							m_ostr << "disconnect cause:Password failure" << "\n";
								break;
								}
			case BONDING_FAILURE :{
    							m_ostr << "disconnect cause:Bonding failure" << "\n";
								break;
								}
			case NO_NET_CONNECTION :{
    							m_ostr << "disconnect cause:No net connection" << "\n";
								break;
								}
			case NET_PORT_DEFICIENCY :{
    						m_ostr << "disconnect cause:Net port deficiency" << "\n";
							break;
							}
			case MUX_PORT_DEFICIENCY :{
    						m_ostr << "disconnect cause:Mux port deficiency" << "\n";
							break;
							}
	    	/*case T123_PORT_DEFICIENCY :{
    						m_ostr << "disconnect cause:T123 port deficiency" << "\n";
							break;
							}*/
		/*	case AUDIO_PORT_DEFICIENCY :{
    						m_ostr << "disconnect cause:Audio port deficiency" << "\n";
							break;
							}
			case VIDEO_PORT_DEFICIENCY :{
    						m_ostr << "disconnect cause:Video port deficiency" << "\n";
							break;
							}
			case DATA_PORT_DEFICIENCY :{
    						m_ostr << "disconnect cause:Data port deficiency " << "\n";
							break;
							}*/
			case AUDIO_MSG_PORT_DEFICIENCY :{
    						m_ostr << "disconnect cause:Audio Msg port deficiency" << "\n";
							break;
							}
			case V_GATE_NOT_RESPOND :{
    						m_ostr << "disconnect cause:V_Gate do not respond" << "\n";
							break;
							}
			case MCU_INTERNAL_PROBLEM :{
    						m_ostr << "disconnect cause: MCU internal problem" << "\n";
							break;
							}
			case LONG_DELAY_BETWEEN_CHANNELS :{
    						m_ostr << "disconnect cause:Long delay between channels" << "\n";
							break;
							}
			case INTERNAL_ERROR :{
    						m_ostr << "disconnect cause:Internal error" << "\n";
							break;
							}
			case NO_ANSWER_FROM_REMOTE_PART :{
    						m_ostr << "disconnect cause:No answer from remote part" << "\n";
							break;
							}
			case ERROR_IN_DIALING_ADDITIONAL_CHANNELS :{
    						m_ostr << "disconnect cause:Error in dialing additional channels" << "\n";
							break;
							}
			case FULL_BITRATE_CONNECTION_FAILURE :{
    						m_ostr << "disconnect cause:Full bitrate connection failure" << "\n";
							break;
							}
			case CAP_EXCHANGE_FAILURE_IN_ESTABLISH_CALL :{
    						m_ostr << "disconnect cause:Cap exchange failure in establish call" << "\n";
							break;
							}
			case CAP_EXCHANGE_FAILURE_IN_CHANGE_MODE :{
    						m_ostr << "disconnect cause:Cap exchange failure in change mode" << "\n";
							break;
							}
			case REMOTE_DEVICES_SELECTED_ENCRYPTION_ALGORITHM_DOES_NOT_MATCH_THE_LOCAL_SELECTED_ENCRYPTION_ALGORITHM :{
    						m_ostr << "disconnect cause: Remote device selected encryption algorithm does not match the local selected encryption algorithm" << "\n";
							break;
							}

			//323
			case H323_CALL_CLOSED_ARQTIMEOUT: {
							m_ostr << "disconnect cause: ARQ timer has ended without receiving indication from the GK" << "\n";
							break;
							}

			case H323_CALL_CLOSED_NO_PORT_LEFT_FOR_AUDIO:
			case H323_CALL_CLOSED_NO_PORT_LEFT_FOR_VIDEO:
			case H323_CALL_CLOSED_NO_PORT_LEFT_FOR_FECC: {
							m_ostr << "disconnect cause: Channel failed to open";
							break;
							}

			case H323_CALL_CLOSED_NO_CONTROL_PORT_LEFT:{
							m_ostr << "disconnect cause: Unable to capture a control port for the call";
							break;
							}

			case H323_CALL_CLOSED_SMALL_BANDWIDTH :{
							m_ostr << "disconnect cause: H.323 small bandwidth" << "\n";
							break;
							}
			case CALLER_NOT_REGISTERED:{
    						m_ostr << "disconnect cause: Caller is not registered" << "\n";
							break;
							}
			case H323_CALL_CLOSED_REMOTE_BUSY :{
    						m_ostr << "disconnect cause: H.323 remote busy" << "\n";
							break;
							}
			case H323_CALL_CLOSED_NORMAL :{
    						m_ostr << "disconnect cause: H.323 close normal" << "\n";
							break;
							}
			case H323_CALL_CLOSED_REMOTE_REJECT :{
    						m_ostr << "disconnect cause: H.323 remote reject" << "\n";
							break;
							}
			case H323_CALL_CLOSED_REMOTE_UNREACHABLE :{
    						m_ostr << "disconnect cause: H.323 remote unreachable" << "\n";
							break;
							}
			case H323_CALL_CLOSED_UNKNOWN_REASON :{
    						m_ostr << "disconnect cause: H.323 unknowen reason" << "\n";
							break;
							}
			case H323_CALL_CLOSED_BY_MCU :{
    						m_ostr << "disconnect cause: H.323 closed by MCU" << "\n";
							break;
							}
			case H323_CALL_CLOSED_FAULTY_DESTINATION_ADDRESS :{
    						m_ostr << "disconnect cause: H.323 faulty destination address" << "\n";
							break;
							}
			case H323_CALL_CLOSED_GATEKEEPER_FAILURE :{
    						m_ostr << "disconnect cause: H.323 gatekeeper failure" << "\n";
							break;
							}
			case H323_CALL_CLOSED_GATEKEEPER_REJECT_ARQ :{
    						m_ostr << "disconnect cause: H.323 gatekeeper reject arq" << "\n";
							break;
							}
			case H323_CALL_CLOSED_NO_PORT_LEFT :{
    						m_ostr << "disconnect cause: H.323 no port left" << "\n";
							break;
							}
			case H323_CALL_CLOSED_GATEKEEPER_DRQ :{
    						m_ostr << "disconnect cause: H.323 gatekeeper drq" << "\n";
							break;
							}
			case H323_CALL_CLOSED_NO_DESTINATION_IP_ADDRESS :{
							m_ostr << "disconnect cause: H.323 no destination ip address" << "\n";
							break;
							}
			case H323_CALL_CLOSED_REMOTE_HAS_NOT_SENT_CAPABILITY :{
							m_ostr << "disconnect cause: H323 call failed prior or during the capabilities negotiation stage" << "\n";
							break;
							}
			case H323_CALL_CLOSED_AUDIO_CHANNELS_NOT_OPEN :{
							m_ostr << "disconnect cause: H.323 call closed audio channels didn't open before timeout" << "\n";
							break;
							}
			case H323_CALL_CLOSED_BAD_REMOTE_CAP :{
							m_ostr << "disconnect cause: Remote sent bad capability" << "\n";
							break;
							}
			case H323_CALL_CLOSED_CAPS_NOT_ACCPTED_BY_REMOTE :{
							m_ostr << "disconnect cause: Local capability wasn't accepted by remote" << "\n";
							break;
															 }
			case H323_FAILURE :{
							m_ostr << "disconnect cause: H.323 failure" << "\n";
							break;
							   }
			case H323_CALL_CLOSED_REMOTE_STOP_RESPONDING :{
							m_ostr << "disconnect cause: H.323 remote stop responding" << "\n";
							break;
						}
			case H323_CALL_CLOSED_PROBLEM_WITH_CONTENT_CONNECTION_TO_MCU :{
							m_ostr << "disconnect cause: Problem with content connection to the mcu" << "\n";
							break;
						}
			case H323_CALL_CLOSED_PROBLEM_WITH_ACTIVE_CONTENT_SLAVE :{
							m_ostr << "disconnect cause: Can not connect slave conference while there is an active content speaker" << "\n";
							break;
						}
			case H323_CALL_CLOSED_MEDIA_DISCONNECTED :{
							m_ostr << "disconnect cause: Media disconnected, nothing received on RTP/RTCP" << "\n";
							break;
						}
			case IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR :{
							m_ostr << "disconnect cause: H239 content stream processing error" << "\n";
							break;
						}

			//end 323

			//SIP:
			case SIP_TIMER_POPPED_OUT: {
							m_ostr << "disconnect cause: SIP remote device did not respond in the given time frame" << "\n";
							break;
							}

			case SIP_CAPS_DONT_MATCH: {
							m_ostr << "disconnect cause: SIP remote device capabilities are not compatible with the conference" << "\n";
							break;
							}

			case SIP_REMOTE_CLOSED_CALL: {
							m_ostr << "disconnect cause: SIP remote device ended the call" << "\n";
							break;
							}
			case SIP_REMOTE_CANCEL_CALL: {
							m_ostr << "disconnect cause: SIP remote device canceled the call" << "\n";
							break;
							}
			case SIP_BAD_STATUS: {
							m_ostr << "disconnect cause: CS general error" << "\n";
							break;
							}
			case SIP_REDIRECTION_300: {
							m_ostr << "disconnect cause: Sip Redirection 300" << "\n";
							break;
							}
			case SIP_REMOTE_STOP_RESPONDING:{
							m_ostr << "disconnect cause: SIP remote device is not responding" << "\n";
							break;
							}
			case SIP_REMOTE_UNREACHABLE: {
							m_ostr << "disconnect cause: SIP remote device could not be reached" << "\n";
							break;
							}
			case SIP_TRANSPORT_ERROR: {
							m_ostr << "disconnect cause: No response from the remote device" << "\n";
							break;
							}
			case SIP_BAD_NAME: {
							m_ostr << "disconnect cause: Conference name is incompatible with SIP" << "\n";
							break;
							}
			case SIP_TRANS_ERROR_TCP_INVITE: {
							m_ostr << "disconnect cause: A SIP Invite was sent via TCP but the endpoint was not found" << "\n";
							break;
							}
			case SIP_MOVED_TEMPORARILY: {
							m_ostr << "disconnect cause: Sip moved temporarily" << "\n";
							break;
							}
			case SIP_MOVED_PERMANENTLY: {
							m_ostr << "disconnect cause: Sip moved permanently" << "\n";
							break;
							}
			case SIP_REDIRECTION_303: {
							m_ostr << "disconnect cause: Sip Redirection 303" << "\n";
							break;
							}
			case SIP_REDIRECTION_305: {
							m_ostr << "disconnect cause: Sip Redirection 305" << "\n";
							break;
							}
			case SIP_REDIRECTION_380: {
							m_ostr << "disconnect cause: Sip Redirection 380" << "\n";
							break;
							}

			case SIP_CLIENT_ERROR_400: {
							m_ostr << "disconnect cause: Sip client error 400" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_402: {
							m_ostr << "disconnect cause: Sip client error 402" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_405: {
							m_ostr << "disconnect cause: Sip client error 405" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_406: {
							m_ostr << "disconnect cause: Sip client error 406" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_407: {
							m_ostr << "disconnect cause: Sip client error 407" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_409: {
							m_ostr << "disconnect cause: Sip client error 409" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_411: {
							m_ostr << "disconnect cause: Sip client error 411" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_413: {
							m_ostr << "disconnect cause: Sip client error 413" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_414: {
							m_ostr << "disconnect cause: Sip client error 414" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_420: {
							m_ostr << "disconnect cause: Sip client error 420" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_481: {
							m_ostr << "disconnect cause: Sip client error 481" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_482: {
							m_ostr << "disconnect cause: Sip client error 482" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_483: {
							m_ostr << "disconnect cause: Sip client error 483" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_484: {
							m_ostr << "disconnect cause: Sip client error 484" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_485: {
							m_ostr << "disconnect cause: Sip client error 485" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_488: {
							m_ostr << "disconnect cause: Sip client error 488" << "\n";
							break;
							}
			case SIP_CLIENT_ERROR_491: {
							m_ostr << "disconnect cause: Sip client error 491" << "\n";
							break;
							}

			case SIP_FORBIDDEN: {
							m_ostr << "disconnect cause: Sip Forbidden" << "\n";
							break;
							}
			case SIP_NOT_FOUND:{
							m_ostr << "disconnect cause: Sip Not Found" << "\n";
							break;
							}
			case SIP_REQUEST_TIMEOUT: {
							m_ostr << "disconnect cause: Sip Request Timeout" << "\n";
							break;
							}
			case SIP_GONE: {
							m_ostr << "disconnect cause: Sip Gone" << "\n";
							break;
							}
			case SIP_UNSUPPORTED_MEDIA_TYPE: {
							m_ostr << "disconnect cause: Sip unsupported media type" << "\n";
							break;
							}
			case SIP_TEMPORARILY_NOT_AVAILABLE: {
							m_ostr << "disconnect cause: Sip temporarily unavailable" << "\n";
							break;
							}
			case SIP_BUSY_HERE: {
							m_ostr << "disconnect cause: Sip busy here" << "\n";
							break;
							}
			case SIP_REQUEST_TERMINATED: {
							m_ostr << "disconnect cause: Sip request terminated" << "\n";
							break;
							}
			case TIP_VIDEO_BIT_RATE_TOO_LOW: {
							m_ostr << "disconnect cause: Video bit rate is too low for TIP" << "\n";
							break;
							}
			case TIP_NEGOTIATION_FAILURE: {
							m_ostr << "disconnect cause: TIP negotiation failed" << "\n";
							break;
							}
			case WEBRTC_CONNECT_FAILURE: {
							m_ostr << "disconnect cause: WebRtc connect failed" << "\n";
							break;
							}
			case WEBRTC_CONNECT_TOUT: {
							m_ostr << "disconnect cause: WebRtc connect time out" << "\n";
							break;
							}
			case SIP_SERVER_ERROR_500: {
							m_ostr << "disconnect cause: Sip Server Error 500" << "\n";
							break;
							}
			case SIP_SERVER_ERROR_501: {
							m_ostr << "disconnect cause: Sip Server Error 501" << "\n";
							break;
							}
			case SIP_SERVER_ERROR_502: {
							m_ostr << "disconnect cause: Sip Server Error 502" << "\n";
							break;
							}
			case SIP_SERVER_ERROR_503: {
							m_ostr << "disconnect cause: Sip Server Error 503" << "\n";
							break;
							}
			case SIP_SERVER_ERROR_504: {
							m_ostr << "disconnect cause: Sip Server Error 504" << "\n";
							break;
							}
			case SIP_SERVER_ERROR_505: {
							m_ostr << "disconnect cause: Sip Server Error 505" << "\n";
							break;
							}

			case SIP_BUSY_EVERYWHERE:{
							m_ostr << "disconnect cause: Sip busy everywhere" << "\n";
							break;
							}

			case SIP_GLOBAL_FAILURE_603:{
							m_ostr << "disconnect cause: Sip Global Failure 603" << "\n";
							break;
							}
			case SIP_GLOBAL_FAILURE_604:{
							m_ostr << "disconnect cause: Sip Global Failure 604" << "\n";
							break;
							}
			case SIP_GLOBAL_FAILURE_606:{
							m_ostr << "disconnect cause: Sip Global Failure 606" << "\n";
							break;
							}
			//end of SIP

			case UNSPECIFIED :{
    						m_ostr << "disconnect cause:Unspecified" << "\n";
							break;
							}
			case NOT_RECIVED_END_INIT_COM :{
    						m_ostr << "disconnect cause:Not received end init com" << "\n";
							break;
							}
			default :{
					  m_ostr<<"disconnect cause:unknown"<<",\n";
					  break;
					 }
			}//endswitch m_disconct_cause
  //m_ostr << "disconnect cause:"<< m_disconct_cause   << ",";
  m_ostr << "Q931 disconnect cause:"<< (CCDRUtils::GetQ931CauseAsString(m_q931_disconct_cause)) << ";\n\n";

}

/////////////////////////////////////////////////////////////////////////////
void CPartyDisconnected::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  //m_istr.ignore(1);
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_istr.getline(m_h243party_name,H243_NAME_LEN+1,',');
  else
	 m_istr.getline(m_h243party_name,H243_NAME_LEN_OLD+1,',');

  m_istr >> m_party_Id;
  m_istr.ignore(1);
  m_istr >> m_disconct_cause;
  m_istr.ignore(1);
  m_istr >> m_q931_disconct_cause;
  m_istr.ignore(1);//---------cdr

}

/////////////////////////////////////////////////////////////////////////////
void CPartyDisconnected::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pPartyDisconnectedNode = pFatherNode->AddChildNode("PARTY_DISCONNECTED");

  pPartyDisconnectedNode->AddChildNode("NAME", m_h243party_name);
  pPartyDisconnectedNode->AddChildNode("PARTY_ID", m_party_Id);

  CXMLDOMElement* pDisconnectCauseNode = pPartyDisconnectedNode->AddChildNode("DISCONNECTION_CAUSE");

  pDisconnectCauseNode->AddChildNode("ID", m_disconct_cause);
  pDisconnectCauseNode->AddChildNode("DESCRIPTION", m_disconct_cause, DISCONNECTION_CAUSE_ENUM);

  pDisconnectCauseNode = pPartyDisconnectedNode->AddChildNode("Q931_DISCONNECTION_CAUSE");
  pDisconnectCauseNode->AddChildNode("ID", m_q931_disconct_cause);
  if (m_disconct_cause != MCU_INTERNAL_PROBLEM)
    pDisconnectCauseNode->AddChildNode("DESCRIPTION", CCDRUtils::GetQ931CauseAsString(m_q931_disconct_cause));
  else
  {
    char mipNum[11];
    memset(mipNum, 11, '\0');
    snprintf(mipNum, sizeof(mipNum), "%d", m_q931_disconct_cause);
    mipNum[6] = '\0';
    pDisconnectCauseNode->AddChildNode("DESCRIPTION", const_cast<char*> (mipNum));
  }
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CPartyDisconnected::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243party_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_party_Id,_0_TO_DWORD);

	CXMLDOMElement *pChild = NULL;

	GET_CHILD_NODE(pActionNode, "DISCONNECTION_CAUSE", pChild);
	if (pChild)
		GET_VALIDATE_CHILD(pChild,"ID",&m_disconct_cause,DISCONNECTION_CAUSE_ENUM);

	GET_CHILD_NODE(pActionNode, "Q931_DISCONNECTION_CAUSE", pChild);
	if (pChild)
		GET_VALIDATE_CHILD(pChild,"ID",&m_q931_disconct_cause,_0_TO_DWORD);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CPartyDisconnected::NameOf() const
{
  return "CPartyDisconnected";
}

/////////////////////////////////////////////////////////////////////////////
void  CPartyDisconnected::SetPartyName(const char* h243partyname)
{
   strncpy(m_h243party_name, h243partyname, H243_NAME_LEN - 1);
   m_h243party_name[H243_NAME_LEN-1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventPartyDisconnected::GetPartyName() const
{
    return m_h243party_name;
}


////////////////////////////////////////////////////////////////////////////
void   CPartyDisconnected::SetPartyId(const DWORD partyid)
{
     m_party_Id=partyid;
}

////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDREventPartyDisconnected::GetPartyId() const
{
    return m_party_Id;
}

////////////////////////////////////////////////////////////////////////////

void CPartyDisconnected::SetDisconctCause(const DWORD disconctcause)
{
   m_disconct_cause=disconctcause;
}
//////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDREventPartyDisconnected::GetDisconctCause() const
{
    return m_disconct_cause;
}
///////////////////////////////////////////////////////////////////////////////
void   CPartyDisconnected::SetQ931DisonctCause(const DWORD q931discocause)
{
  m_q931_disconct_cause=q931discocause;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ACCCDREventPartyDisconnected::GetQ931DisonctCause() const
{
   return m_q931_disconct_cause;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//CPartyErrors

ACCCDRPartyErrors::ACCCDRPartyErrors()
{
	m_h243party_name[0]='\0';
	m_party_Id=0xffffffff;
	m_resrc_failure=0;
	m_numH221_sync_loss=0;
	m_duration_h221_sync_loss=0;
	m_numH221_syncR_loss=0;
	m_duration_h221_syncR_loss=0;
	m_num_remote_vcu=0;

}

/////////////////////////////////////////////////////////////////////////////
ACCCDRPartyErrors::ACCCDRPartyErrors(const ACCCDRPartyErrors &other)
{
	*this=other;
}
/////////////////////////////////////////////////////////////////////////////
ACCCDRPartyErrors::~ACCCDRPartyErrors()
{
}
ACCCDRPartyErrors& ACCCDRPartyErrors::operator = (const ACCCDRPartyErrors& other)
{
	strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN);
	m_party_Id = other.m_party_Id;
	m_resrc_failure=other.m_resrc_failure;
	m_numH221_sync_loss=other.m_numH221_sync_loss;
	m_duration_h221_sync_loss=other.m_duration_h221_sync_loss;
	m_numH221_syncR_loss=other.m_numH221_syncR_loss;
	m_duration_h221_syncR_loss=other.m_duration_h221_syncR_loss;
	m_num_remote_vcu=other.m_num_remote_vcu;
	return *this;

}

/////////////////////////////////////////////////////////////////////////////
/*
char* CPartyErrors::Serialize(WORD format)
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

CPartyErrors::CPartyErrors()
{
}

/////////////////////////////////////////////////////////////////////////////
CPartyErrors::~CPartyErrors()
{
}
/////////////////////////////////////////////////////////////////////////////
void CPartyErrors::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{

  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << m_h243party_name << ",";
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

	 m_ostr << tmp << ",";
  }

  m_ostr << m_party_Id << ",";
  m_ostr << m_resrc_failure << ",";
  m_ostr << m_numH221_sync_loss  << ",";
  m_ostr << m_duration_h221_sync_loss << ",";
	m_ostr << m_numH221_syncR_loss << ",";
	m_ostr << m_duration_h221_syncR_loss << ",";
	m_ostr << m_num_remote_vcu << ",\n";

}
/////////////////////////////////////////////////////////////////////////////
void CPartyErrors::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{

  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << "party name:"<<m_h243party_name << "\n";
  else{
	  char tmp[H243_NAME_LEN_OLD];
	  strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	  tmp[H243_NAME_LEN_OLD-1]='\0';

	  m_ostr << "party name:"<< tmp << "\n";
  }

  m_ostr << "party ID:"<<m_party_Id << "\n";
		switch (m_resrc_failure) {
					case RESOURCE_FAILED :{
    										m_ostr << "resource failed" << "\n";
												break;
											}
					case NO_FAILURE :{
    										m_ostr << "no failure" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch m_resrc_failure
  m_ostr <<"number of sync loss:"<< m_numH221_sync_loss  << "\n";
  m_ostr <<"sync loss duration:"<< m_duration_h221_sync_loss << "\n";
	m_ostr << "number of R-sync loss:"<<m_numH221_syncR_loss << "\n";
	m_ostr << "R-sync loss duration:"<<m_duration_h221_syncR_loss << "\n";
	m_ostr << "number of remote VCU:"<<m_num_remote_vcu << ";\n\n";

}

/////////////////////////////////////////////////////////////////////////////
void CPartyErrors::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  m_istr.ignore(1);
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_istr.getline(m_h243party_name,H243_NAME_LEN+1,',');
  else
	 m_istr.getline(m_h243party_name,H243_NAME_LEN_OLD+1,',');

  m_istr >> m_party_Id;
  m_istr.ignore(1);
  m_istr >> m_resrc_failure;
  m_istr.ignore(1);
  m_istr >> m_numH221_sync_loss;
  m_istr.ignore(1);
  m_istr >> m_duration_h221_sync_loss;
  m_istr.ignore(1);
  m_istr >> m_numH221_syncR_loss;
  m_istr.ignore(1);
  m_istr >> m_duration_h221_syncR_loss;
  m_istr.ignore(1);
  m_istr >> m_num_remote_vcu;
}

/////////////////////////////////////////////////////////////////////////////
//
void CPartyErrors::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pPartyErrorsNode = pFatherNode->AddChildNode("PARTY_ERORRS");

	pPartyErrorsNode->AddChildNode("NAME",m_h243party_name);
	pPartyErrorsNode->AddChildNode("PARTY_ID",m_party_Id);
	pPartyErrorsNode->AddChildNode("L_SYNC_LOSS",m_numH221_sync_loss);
	pPartyErrorsNode->AddChildNode("L_SYNC_LOSS_DURATION",m_duration_h221_sync_loss);
	pPartyErrorsNode->AddChildNode("R_SYNC_LOSS",m_numH221_syncR_loss);
	pPartyErrorsNode->AddChildNode("R_SYNC_LOSS_DURATION",m_duration_h221_syncR_loss);
	pPartyErrorsNode->AddChildNode("R_VCU",m_num_remote_vcu);
	pPartyErrorsNode->AddChildNode("RESOURCE_FAIL",m_resrc_failure,_BOOL);
}


/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CPartyErrors::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243party_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_party_Id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"L_SYNC_LOSS",&m_numH221_sync_loss,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"L_SYNC_LOSS_DURATION",&m_duration_h221_sync_loss,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"R_SYNC_LOSS",&m_numH221_syncR_loss,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"R_SYNC_LOSS_DURATION",&m_duration_h221_syncR_loss,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"R_VCU",&m_num_remote_vcu,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"RESOURCE_FAIL",&m_resrc_failure,_BOOL);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CPartyErrors::NameOf() const
{
  return "CPartyErrors";
}

/////////////////////////////////////////////////////////////////////////////
void  CPartyErrors::SetPartyName(const char* h243partyname)
{
   int len=strlen(h243partyname);
   strncpy(m_h243party_name, h243partyname, H243_NAME_LEN - 1);
   if (len>H243_NAME_LEN-1)
        m_h243party_name[H243_NAME_LEN-1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDRPartyErrors::GetPartyName() const
{
    return m_h243party_name;
}

////////////////////////////////////////////////////////////////////////////
void   CPartyErrors::SetPartyId(const DWORD partyid)
{
     m_party_Id=partyid;
}

////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDRPartyErrors::GetPartyId() const
{
    return m_party_Id;
}

////////////////////////////////////////////////////////////////////////////
void CPartyErrors::SetResrcFailure(const BYTE rsrcfailure)
{
   m_resrc_failure=rsrcfailure;
}
//////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDRPartyErrors::GetResrcFailure() const
{
    return m_resrc_failure;
}
///////////////////////////////////////////////////////////////////////////////
void   CPartyErrors::SetNumH221SyncLoss(const DWORD h221syncloss)
{
  m_numH221_sync_loss=h221syncloss;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ACCCDRPartyErrors::GetNumH221SyncLoss() const
{
   return m_numH221_sync_loss;
}
///////////////////////////////////////////////////////////////////////////////
void   CPartyErrors::SetDurationH221SyncLoss(const DWORD durationsyncloss)
{
  m_duration_h221_sync_loss = durationsyncloss;
}
////////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDRPartyErrors::GetDurationH221SyncLoss() const
{
    return m_duration_h221_sync_loss;
}
///////////////////////////////////////////////////////////////////////////////
void   CPartyErrors::SetNumH221SyncRLoss(const DWORD h221syncRloss)
{
  m_numH221_syncR_loss = h221syncRloss;
}
////////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDRPartyErrors::GetNumH221SyncRLoss() const
{
     return m_numH221_syncR_loss;
}
///////////////////////////////////////////////////////////////////////////////
void   CPartyErrors::SetDurationH221SyncRLoss(const DWORD durationsynRcloss)
{
  m_duration_h221_syncR_loss = durationsynRcloss;
}
////////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDRPartyErrors::GetDurationH221SyncRLoss() const
{
    return m_duration_h221_syncR_loss;
}
///////////////////////////////////////////////////////////////////////////////
void   CPartyErrors::SetNumberRemoteVcu(const DWORD remote_vcu)
{
  m_num_remote_vcu = remote_vcu;
}
////////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDRPartyErrors::GetNumberRemoteVcu() const
{
     return m_num_remote_vcu;
}
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//CPartyDisconnectedCont1

ACCCDREventPartyDisconnectedCont1::ACCCDREventPartyDisconnectedCont1()
{
   m_L_syncLostCounter=0;
	m_R_syncLostCounter=0;
	m_L_videoSyncLostCounter=0;
	m_R_videoSyncLostCounter=0;
	m_MuxBoardId=0;
	m_MuxUnitId=0 ;
	m_AudioCodecBoardId=0;
	m_AudioCodecUnitId=0;
	m_AudioBrgBoardId=0 ;
	m_AudioBrgUnitId=0 ;
	m_VideoBoardId=0 ;
	m_VideoUnitId=0 ;
	m_T120BoardId=0 ;
	m_T120UnitId=0 ;
	m_MCSBoardId=0 ;
	m_MCSUnitId=0 ;
	m_H323BoardId=0 ;
	m_H323UnitId=0 ;


}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyDisconnectedCont1::ACCCDREventPartyDisconnectedCont1(const ACCCDREventPartyDisconnectedCont1 &other)
{
	*this=other;

}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyDisconnectedCont1::~ACCCDREventPartyDisconnectedCont1()
{
}
ACCCDREventPartyDisconnectedCont1& ACCCDREventPartyDisconnectedCont1::operator = (const ACCCDREventPartyDisconnectedCont1& other)
{
	m_L_syncLostCounter=other.m_L_syncLostCounter;
	m_R_syncLostCounter=other.m_R_syncLostCounter;
	m_L_videoSyncLostCounter=other.m_L_videoSyncLostCounter;
	m_R_videoSyncLostCounter=other.m_R_videoSyncLostCounter;
	m_MuxBoardId=other.m_MuxBoardId;
	m_MuxUnitId=other.m_MuxUnitId;
	m_AudioCodecBoardId=other.m_AudioCodecBoardId;
	m_AudioCodecUnitId=other.m_AudioCodecUnitId;
	m_AudioBrgBoardId=other.m_AudioBrgBoardId;
	m_AudioBrgUnitId=other.m_AudioBrgUnitId;
	m_VideoBoardId=other.m_VideoBoardId ;
	m_VideoUnitId=other.m_VideoUnitId;
	m_T120BoardId=other.m_T120BoardId ;
	m_T120UnitId=other.m_T120UnitId;
	m_MCSBoardId=other.m_MCSBoardId;
	m_MCSUnitId=other.m_MCSUnitId;
	m_H323BoardId=other.m_H323BoardId ;
	m_H323UnitId=other.m_H323UnitId;
	return *this;

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CPartyDisconnectedCont1::CPartyDisconnectedCont1()
{
}

/////////////////////////////////////////////////////////////////////////////
CPartyDisconnectedCont1::~CPartyDisconnectedCont1()
{
}

/////////////////////////////////////////////////////////////////////////////

void CPartyDisconnectedCont1::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  m_ostr <<m_L_syncLostCounter    << ",";
  m_ostr <<m_R_syncLostCounter    << ",";
  m_ostr <<m_L_videoSyncLostCounter    << ",";
  m_ostr <<m_R_videoSyncLostCounter    << ",";
  m_ostr <<m_MuxBoardId   << ",";
  m_ostr <<m_MuxUnitId    << ",";
  m_ostr <<m_AudioCodecBoardId  << ",";
  m_ostr <<m_AudioCodecUnitId   << ",";
  m_ostr <<m_AudioBrgBoardId    << ",";
  m_ostr <<m_AudioBrgUnitId    << ",";
  m_ostr <<m_VideoBoardId    << ",";
  m_ostr <<m_VideoUnitId    << ",";
  m_ostr <<m_T120BoardId    << ",";
  m_ostr <<m_T120UnitId    << ",";
  m_ostr <<m_MCSBoardId    << ",";
  m_ostr <<m_MCSUnitId    << ",";
  m_ostr <<m_H323BoardId    << ",";
  m_ostr <<m_H323UnitId     << ";\n";

}
/////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{
  m_ostr << "Local sync lost counter:"<<m_L_syncLostCounter   << "\n";
  m_ostr << "Remote sync lost counter:"<<m_R_syncLostCounter   << "\n";
  m_ostr << "Local video sync lost counter:"<<m_L_videoSyncLostCounter   << "\n";
  m_ostr << "Remote video sync lost counter:"<<m_R_videoSyncLostCounter   << "\n";
   if(m_MuxBoardId!=0xffff)
    m_ostr << "Mux Board Id:"<<m_MuxBoardId   << "\n";
  else
    m_ostr << "Mux Board Id:not in use" << "\n";

  if(m_MuxUnitId!=0xffff)
    m_ostr << "Mux Unit Id:" <<m_MuxUnitId   << "\n";
  else
    m_ostr << "Mux Unit Id:not in use" << "\n";
  m_ostr << "Audio codec Board Id:"<<m_AudioCodecBoardId   << "\n";
  m_ostr << "Audio codec Unit Id:"<<m_AudioCodecUnitId   << "\n";
  m_ostr << "Audio bridge Board Id:"<<m_AudioBrgBoardId   << "\n";
  m_ostr << "Audio bridge Unit Id:"<<m_AudioBrgUnitId   << "\n";
  m_ostr << "Video Board Id:"<<m_VideoBoardId   << "\n";
  m_ostr << "Video Unit Id:"<<m_VideoUnitId   << "\n";
  m_ostr << "T120 Board Id:"<<m_T120BoardId   << "\n";
  m_ostr << "T120 Unit Id:"<<m_T120UnitId   << "\n";
  m_ostr << "T120 MCS Board Id:"<<m_MCSBoardId   << "\n";
  m_ostr << "T120 MCS Unit Id:"<<m_MCSUnitId   << "\n";
  m_ostr << "IP Board Id:"<<m_H323BoardId  << "\n";
  m_ostr << "IP Unit Id:"<<m_H323UnitId   << ";\n\n";
}

/////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  m_istr >> m_L_syncLostCounter;
  m_istr.ignore(1);
  m_istr >> m_R_syncLostCounter;
  m_istr.ignore(1);
  m_istr >> m_L_videoSyncLostCounter;
  m_istr.ignore(1);
  m_istr >> m_R_videoSyncLostCounter ;
  m_istr.ignore(1);
  m_istr >> m_MuxBoardId;
  m_istr.ignore(1);
  m_istr >> m_MuxUnitId;
  m_istr.ignore(1);
  m_istr >> m_AudioCodecBoardId;
  m_istr.ignore(1);
  m_istr >> m_AudioCodecUnitId;
  m_istr.ignore(1);
  m_istr >> m_AudioBrgBoardId;
  m_istr.ignore(1);
  m_istr >> m_AudioBrgUnitId;
  m_istr.ignore(1);
  m_istr >> m_VideoBoardId ;
  m_istr.ignore(1);
  m_istr >> m_VideoUnitId ;
  m_istr.ignore(1);
  m_istr >> m_T120BoardId;
  m_istr.ignore(1);
  m_istr >> m_T120UnitId;
  m_istr.ignore(1);
  m_istr >> m_MCSBoardId;
  m_istr.ignore(1);
  m_istr >> m_MCSUnitId;
  m_istr.ignore(1);
  m_istr >> m_H323BoardId;
  m_istr.ignore(1);
  m_istr >> m_H323UnitId;
  m_istr.ignore(1);

}

/////////////////////////////////////////////////////////////////////////////
//
void CPartyDisconnectedCont1::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pPartyDisconnected1Node = pFatherNode->AddChildNode("PARTY_DISCONNECTED_1");

//	pPartyDisconnected1Node->AddChildNode("AUDIO_BOARD",m_AudioCodecBoardId,BOARD_ENUM);
//	pPartyDisconnected1Node->AddChildNode("VIDEO_BOARD",m_VideoBoardId,BOARD_ENUM);
//	pPartyDisconnected1Node->AddChildNode("DATA_BOARD",m_T120BoardId,BOARD_ENUM);
	pPartyDisconnected1Node->AddChildNode("MUX_BOARD",m_MuxBoardId,BOARD_ENUM);
	pPartyDisconnected1Node->AddChildNode("AUDIO_BRIDGE_BOARD",m_AudioBrgBoardId,BOARD_ENUM);
	pPartyDisconnected1Node->AddChildNode("H323_BOARD",m_H323BoardId,BOARD_ENUM);
	pPartyDisconnected1Node->AddChildNode("AUDIO_UNIT",m_AudioCodecUnitId,UNIT_ENUM);
//	pPartyDisconnected1Node->AddChildNode("VIDEO_UNIT",m_VideoUnitId,UNIT_ENUM);
//	pPartyDisconnected1Node->AddChildNode("DATA_UNIT",m_T120UnitId,UNIT_ENUM);
	pPartyDisconnected1Node->AddChildNode("MUX_UNIT",m_MuxUnitId,UNIT_ENUM);
	pPartyDisconnected1Node->AddChildNode("AUDIO_BRIDGE_UNIT",m_AudioBrgUnitId,UNIT_ENUM);
	pPartyDisconnected1Node->AddChildNode("H323_UNIT",m_H323UnitId,UNIT_ENUM);

	pPartyDisconnected1Node->AddChildNode("L_SYNC_LOSS",m_L_syncLostCounter);
	pPartyDisconnected1Node->AddChildNode("R_SYNC_LOSS",m_R_syncLostCounter);
	pPartyDisconnected1Node->AddChildNode("L_VIDEO_SYNC_LOSS",m_L_videoSyncLostCounter);
	pPartyDisconnected1Node->AddChildNode("R_VIDEO_SYNC_LOSS",m_R_videoSyncLostCounter);
}


/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CPartyDisconnectedCont1::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

//	GET_VALIDATE_CHILD(pActionNode,"AUDIO_BOARD",&m_AudioCodecBoardId,BOARD_ENUM);
//	GET_VALIDATE_CHILD(pActionNode,"VIDEO_BOARD",&m_VideoBoardId,BOARD_ENUM);
//	GET_VALIDATE_CHILD(pActionNode,"DATA_BOARD",&m_T120BoardId,BOARD_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"MUX_BOARD",&m_MuxBoardId,BOARD_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_BRIDGE_BOARD",&m_AudioBrgBoardId,BOARD_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"H323_BOARD",&m_H323BoardId,BOARD_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_UNIT",&m_AudioCodecUnitId,UNIT_ENUM);
//	GET_VALIDATE_CHILD(pActionNode,"VIDEO_UNIT",&m_VideoUnitId,UNIT_ENUM);
//	GET_VALIDATE_CHILD(pActionNode,"DATA_UNIT",&m_T120UnitId,UNIT_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"MUX_UNIT",&m_MuxUnitId,UNIT_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"AUDIO_BRIDGE_UNIT",&m_AudioBrgUnitId,UNIT_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"H323_UNIT",&m_H323UnitId,UNIT_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"L_SYNC_LOSS",&m_L_syncLostCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"R_SYNC_LOSS",&m_R_syncLostCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"L_VIDEO_SYNC_LOSS",&m_L_videoSyncLostCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"R_VIDEO_SYNC_LOSS",&m_R_videoSyncLostCounter,_0_TO_DWORD);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CPartyDisconnectedCont1::NameOf() const
{
  return "CPartyDisconnectedCont1";
}

////////////////////////////////////////////////////////////////////////////
WORD   ACCCDREventPartyDisconnectedCont1::GetL_syncLostCounter() const
{
    return m_L_syncLostCounter ;
}

////////////////////////////////////////////////////////////////////////////
void  CPartyDisconnectedCont1::SetL_syncLostCounter(const WORD num)
{
     m_L_syncLostCounter=num;
}

////////////////////////////////////////////////////////////////////////////
WORD  ACCCDREventPartyDisconnectedCont1::GetR_syncLostCounter() const
{
    return m_R_syncLostCounter;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetR_syncLostCounter(const WORD num)
{
     m_R_syncLostCounter=num;
}

////////////////////////////////////////////////////////////////////////////
WORD  ACCCDREventPartyDisconnectedCont1::GetL_videoSyncLostCounter() const
{
    return m_L_videoSyncLostCounter;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetL_videoSyncLostCounter(const WORD num)
{
     m_L_videoSyncLostCounter=num;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetR_videoSyncLostCounter() const
{
    return m_R_videoSyncLostCounter;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetR_videoSyncLostCounter(const WORD num)
{
     m_R_videoSyncLostCounter=num;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetMuxBoardId() const
{
    return m_MuxBoardId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetMuxBoardId(const WORD boardId)
{
    m_MuxBoardId=boardId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetMuxUnitId() const
{
    return m_MuxUnitId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetMuxUnitId(const WORD unitId)
{
    m_MuxUnitId=unitId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetAudioCodecBoardId() const
{
    return m_AudioCodecBoardId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetAudioCodecBoardId(const WORD boardId)
{
    m_AudioCodecBoardId=boardId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetAudioCodecUnitId() const
{
    return m_AudioCodecUnitId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetAudioCodecUnitId(const WORD unitId)
{
   m_AudioCodecUnitId=unitId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetAudioBrgBoardId() const
{
    return m_AudioBrgBoardId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetAudioBrgBoardId(const WORD boardId)
{
    m_AudioBrgBoardId =boardId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetAudioBrgUnitId() const
{
    return m_AudioBrgUnitId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetAudioBrgUnitId(const WORD unitId)
{
    m_AudioBrgUnitId=unitId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetVideoBoardId() const
{
    return m_VideoBoardId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetVideoBoardId(const WORD boardId)
{
    m_VideoBoardId=boardId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetVideoUnitId() const
{
    return m_VideoUnitId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetVideoUnitId(const WORD unitId)
{
    m_VideoUnitId=unitId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetT120BoardId() const
{
    return m_T120BoardId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetT120BoardId(const WORD boardId)
{
    m_T120BoardId=boardId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetT120UnitId() const
{
    return m_T120UnitId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetT120UnitId(const WORD unitId)
{
    m_T120UnitId=unitId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetMCSBoardId() const
{
    return m_MCSBoardId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetMCSBoardId(const WORD boardId)
{
    m_MCSBoardId=boardId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetMCSUnitId() const
{
    return m_MCSUnitId;
}
////////////////////////////////////////////////////////////////////////////
void  CPartyDisconnectedCont1::SetMCSUnitId(const WORD unitId)
{
    m_MCSUnitId=unitId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetH323BoardId() const
{
    return m_H323BoardId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetH323BoardId(const WORD boardId)
{
    m_H323BoardId=boardId;
}

////////////////////////////////////////////////////////////////////////////
WORD ACCCDREventPartyDisconnectedCont1::GetH323UnitId() const
{
    return m_H323UnitId;
}
////////////////////////////////////////////////////////////////////////////
void CPartyDisconnectedCont1::SetH323UnitId(const WORD unitId)
{
    m_H323UnitId=unitId;
}

////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CPartyConnected::Serialize323(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{
	BYTE* ph323;
	DWORD len;

  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << "party name:"<<m_h243party_name << "\n";
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

	 m_ostr << "party name:"<< tmp << "\n";
  }
  m_ostr << "party ID:"<<m_party_Id   << "\n";
		switch (m_party_state) {
					case PARTY_CONNECTED :{
    										m_ostr << "party state: connected" << "\n";
												break;
											}
					case PARTY_SECONDARY :{
    										m_ostr << "party state: secondary" << "\n";
												break;
									  	}
					default :{
												m_ostr<<"--"<<"\n";
												break;
											}
					}//endswitch m_party_state


	len=m_pCapabilities->GetLen();
	ph323=m_pCapabilities->GetPtr();
	BYTE capflag=0;
//	FullDumpCap(ph221,len,m_ostr,capflag);
//	CdrDumpH323Cap(m_ostr,len,ph323,capflag);
 	len=m_pRemoteCommMode->GetLen();
	ph323=m_pRemoteCommMode->GetPtr();
	capflag=1;
   // CdrDumpH221Stream(m_ostr,len,ph323);
//	CdrDumpH323Cap(m_ostr,len,ph323,capflag);
	if(m_party_state==PARTY_SECONDARY)
	{
		switch((WORD)m_second_cause){
				case CAUSE_RESTRICT :{
									m_ostr << "secondary cause:Restrict"<< ";\n\n";
									break;
									}
				case SM_COMP :{
								m_ostr << "secondary cause:SM comp"<< ";\n\n";
								break;
								}
				case BIT_RATE :{
									m_ostr << "secondary cause:Bit rate"<< ";\n\n";
									break;
									}
				case CAUSE_LSD :{
								m_ostr << "secondary cause:LSD"<< ";\n\n";
								break;
								}
				case CAUSE_HSD :{
									m_ostr << "secondary cause:HSD"<< ";\n\n";
									break;
									}
				case CAUSE_MLP :{
								m_ostr << "secondary cause:MLP"<< ";\n\n";
								break;
								}
				case CAUSE_H_MLP :{
									m_ostr << "secondary cause:H-MLP"<< ";\n\n";
									break;
									}
				case CAUSE_QCIF :{
								m_ostr << "secondary cause:QCIF"<< ";\n\n";
								break;
								}
				case CAUSE_VIDEO_FRAME_RATE :{
									m_ostr << "secondary cause:Video frame rate"<< ";\n\n";
									break;
									}
				case CAUSE_H_243 :{
								m_ostr << "secondary cause:H243 bad behavior"<< ";\n\n";
								break;
								}
				case OTHER_CAUSE :{
    						       m_ostr << "secondary cause: Other" << ";\n\n";
								   break;
								 	}
				default :{
							m_ostr<<"--"<<";\n";
							break;
							}
		}//endswitch m_second_cause
	}
	else
		m_ostr <<";\n\n";

}

/////////////////////////////////////////////////////////////////////////////

//////////////Billing Code///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//CpartyAddBillingCode

ACCCDREventPartyAddBillingCode::ACCCDREventPartyAddBillingCode()
{
      m_h243party_name[0]='\0';
	  m_party_Id=0xffffffff;
	  m_billing_data[0]='\0';//temp
}

/////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyAddBillingCode::ACCCDREventPartyAddBillingCode(const ACCCDREventPartyAddBillingCode &other)
{
	*this=other;
}
/////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyAddBillingCode::~ACCCDREventPartyAddBillingCode()
{
}

ACCCDREventPartyAddBillingCode& ACCCDREventPartyAddBillingCode::operator = (const ACCCDREventPartyAddBillingCode& other)
{
  strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN);
  m_party_Id = other.m_party_Id;
  strncpy(m_billing_data,other.m_billing_data,32);//temp
  return *this;
}

/////////////////////////////////////////////////////////////////////////////
CPartyAddBillingCode::CPartyAddBillingCode()
{
}

/////////////////////////////////////////////////////////////////////////////
CPartyAddBillingCode::~CPartyAddBillingCode()
{
}
/////////////////////////////////////////////////////////////////////////////
void CPartyAddBillingCode::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << m_h243party_name << ",";
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

	 m_ostr << "party name:"<< tmp << "\n";
  }

  m_ostr << m_party_Id   << ",";
  m_ostr << m_billing_data << ";\n";

}
/////////////////////////////////////////////////////////////////////////////
void CPartyAddBillingCode::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_ostr << "party name:"<<m_h243party_name << "\n";
  else{
	 char tmp[H243_NAME_LEN_OLD];
	 strncpy(tmp,m_h243party_name,H243_NAME_LEN_OLD-1);
	 tmp[H243_NAME_LEN_OLD-1]='\0';

	 m_ostr << "party name:"<< tmp << "\n";
  }

  m_ostr << "party ID:"<<m_party_Id   << "\n";

  m_ostr << "Billing data:"<< m_billing_data << ";\n\n";

}

/////////////////////////////////////////////////////////////////////////////
void CPartyAddBillingCode::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  //m_istr.ignore(1);
  if(apiNum>=API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
     m_istr.getline(m_h243party_name,H243_NAME_LEN+1,',');
  else
	 m_istr.getline(m_h243party_name,H243_NAME_LEN_OLD+1,',');

  m_istr >> m_party_Id;
  m_istr.ignore(1);
  m_istr.getline(m_billing_data,32+1,';');
 // m_istr.ignore(1);//---------cdr

}

/////////////////////////////////////////////////////////////////////////////
//
void CPartyAddBillingCode::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pAddBillingCodeNode = pFatherNode->AddChildNode("BILLING");

	pAddBillingCodeNode->AddChildNode("NAME",m_h243party_name);
	pAddBillingCodeNode->AddChildNode("ID",m_party_Id);
	pAddBillingCodeNode->AddChildNode("BILLING_DATA",m_billing_data);
}


/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CPartyAddBillingCode::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_h243party_name,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_party_Id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"BILLING_DATA",m_billing_data,_0_TO_31_STRING_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CPartyAddBillingCode::NameOf() const
{
  return "CPartyAddBillingCode";
}

/////////////////////////////////////////////////////////////////////////////
void  CPartyAddBillingCode::SetPartyName(const char* h243partyname)
{
   int len=strlen(h243partyname);
   strncpy(m_h243party_name, h243partyname, H243_NAME_LEN - 1);
   if (len>H243_NAME_LEN-1)
        m_h243party_name[H243_NAME_LEN-1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventPartyAddBillingCode::GetPartyName() const
{
    return m_h243party_name;
}


////////////////////////////////////////////////////////////////////////////
void   CPartyAddBillingCode::SetPartyId(const DWORD partyid)
{
     m_party_Id=partyid;
}

////////////////////////////////////////////////////////////////////////////
DWORD  ACCCDREventPartyAddBillingCode::GetPartyId() const
{
    return m_party_Id;
}
/////////////////////////////////////////////////////////////////////////////
void  CPartyAddBillingCode::SetBillingData(const char* pBillingData)
{
   int len=strlen(pBillingData);
   strncpy(m_billing_data, pBillingData, 32 - 1);//temp
   if (len>32-1)//temp
        m_billing_data[32-1]='\0';//temp
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCCDREventPartyAddBillingCode::GetBillingData() const
{
    return m_billing_data;
}


/*************************************************************************/
/*							class CCDRPartyGkInfo						 */
/*************************************************************************/

/////////////////////////////////////////////////////////////////
CCDRPartyGkInfo::CCDRPartyGkInfo()
{
	memset(&m_partyName, 0, H243_NAME_LEN);
	m_partyId = 0;
	memset(&m_gkCallId, 0, SIZE_OF_CALL_ID);
}

/////////////////////////////////////////////////////////////////
CCDRPartyGkInfo::CCDRPartyGkInfo(const CCDRPartyGkInfo& other)
{
	strcpy(m_partyName, other.m_partyName);
	m_partyId = other.m_partyId;
	memcpy(m_gkCallId, other.m_gkCallId, SIZE_OF_CALL_ID);
}

/////////////////////////////////////////////////////////////////
CCDRPartyGkInfo::~CCDRPartyGkInfo()
{
}

/////////////////////////////////////////////////////////////////
CCDRPartyGkInfo& CCDRPartyGkInfo::operator = (const CCDRPartyGkInfo& other)
{
	if (&other != this)
	{
		strcpy(m_partyName, other.m_partyName);
		m_partyId = other.m_partyId;
		memcpy(m_gkCallId, other.m_gkCallId, SIZE_OF_CALL_ID);
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CCDRPartyGkInfo::operator == (const CCDRPartyGkInfo& other)
{
	bool res1 = ( 	(strcmp(m_partyName ,other.m_partyName) == 0) &&
		    		(m_partyId == other.m_partyId));

	bool res2 = false;
	for(int i = 0 ; i < SIZE_OF_CALL_ID ; i++)
	{
		res2 = (m_gkCallId[i] == other.m_gkCallId[i]);
		if(false == res2)
		{
			break;
		}
	}

	return res1 && res2;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CCDRPartyGkInfo::GetPartyName() const
{
	return m_partyName;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCDRPartyGkInfo::GetPartyId() const
{
	return m_partyId;
}

////////////////////////////////////////////////////////////////////////////
const BYTE*  CCDRPartyGkInfo::GetGkCallId() const
{
	return m_gkCallId;
}


/*************************************************************************/
/*							class CGkInfo								 */
/*************************************************************************/

////////////////////////////////////////////////////////////////////////////
CGkInfo::CGkInfo()
{
}

////////////////////////////////////////////////////////////////////////////
CGkInfo::~CGkInfo()
{
}

////////////////////////////////////////////////////////////////////////////
CGkInfo& CGkInfo::operator=(const CGkInfo &other)
{
	if(&other != this)
	{
        CCDRPartyGkInfo::operator =(other);
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////
bool CGkInfo::operator == (const CGkInfo &rHnd)
{
	if(&rHnd == this)
	{
		return true;
	}
	return CCDRPartyGkInfo::operator ==(rHnd);
}

////////////////////////////////////////////////////////////////////////////
void CGkInfo::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
	m_ostr << m_partyName 	<< ",";
  	m_ostr << m_partyId 	<< ",";

	BYTE temp;
 	for(int j=0; j < SIZE_OF_CALL_ID; j++)
 	{
 		temp = m_gkCallId[j];
		m_ostr << (WORD)temp << ",";
 	}

	m_ostr  << ";\n";
}

////////////////////////////////////////////////////////////////////////////
void CGkInfo::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
	m_istr.getline(m_partyName, H243_NAME_LEN+1, ',');
	m_istr >> m_partyId;
  	m_istr.ignore(1);

    WORD temp;
 	for(int j=0; j < SIZE_OF_CALL_ID; j++)
 	{
		m_istr >> temp;
		m_gkCallId[j] = (BYTE)temp;
		m_istr.ignore(1);
 	}

  	m_istr.ignore(1);
}

////////////////////////////////////////////////////////////////////////////
void CGkInfo::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pGkNode = NULL;

	pGkNode = pFatherNode->AddChildNode("GATEKEEPER_INFORMATION");

	if (NULL == pGkNode)
	{
		return;
	}

	pGkNode->AddChildNode("NAME",m_partyName);
	pGkNode->AddChildNode("PARTY_ID",m_partyId);

	CSmallString str;
	//str << "0x";
	str.SetFormat("%02x");
	for(int j = 0; j < SIZE_OF_CALL_ID; j++)
	{
		WORD tmp = (WORD)(m_gkCallId[j]);
		str << tmp;
		if( j==3 || j==5 || j==7 || j==9)
		  str << "-";
	}

	pGkNode->AddChildNode("GK_CALL_ID",str.GetString());
}

////////////////////////////////////////////////////////////////////////////
int  CGkInfo::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
/*
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_partyName,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_partyId,_0_TO_DWORD);
*/
	PASSERTMSG(TRUE, "CGkInfo::DeSerializeXml - Should not be called");

	return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////
void CGkInfo::SetPartyName(const char* h243partyname)
{
   int len = strlen(h243partyname);
   strncpy(m_partyName, h243partyname, H243_NAME_LEN - 1);
   if (len > H243_NAME_LEN-1)
        m_partyName[H243_NAME_LEN-1]= '\0';
}

/////////////////////////////////////////////////////////////////////////////
void  CGkInfo::SetPartyId(const DWORD partyId)
{
	m_partyId = partyId;
}

/////////////////////////////////////////////////////////////////////////////
void  CGkInfo::SetGkCallId(const BYTE *gkCallId)
{
	memcpy(m_gkCallId, gkCallId, SIZE_OF_CALL_ID);
}

////////////////////////////////////////////////////////////////////////////
const char*  CGkInfo::NameOf() const
{
	return "CGkInfo";
}
////////////////////////////////////////////////////////////////////////////

////Class CCDRPartyNewRateInfo/////////////////////////////////////////////
/*************************************************************************/
/*							class CCDRPartyNewRateInfo						 */
/*************************************************************************/

/////////////////////////////////////////////////////////////////
CCDRPartyNewRateInfo::CCDRPartyNewRateInfo()
{
	memset(&m_partyName, 0, H243_NAME_LEN);
	m_partyId = 0;
	m_partyCurrentRate = 0;
}

/////////////////////////////////////////////////////////////////
CCDRPartyNewRateInfo::CCDRPartyNewRateInfo(const CCDRPartyNewRateInfo& other)
{
	strcpy(m_partyName, other.m_partyName);
	m_partyId = other.m_partyId;
	m_partyCurrentRate = other.m_partyCurrentRate;
}

/////////////////////////////////////////////////////////////////
CCDRPartyNewRateInfo::~CCDRPartyNewRateInfo()
{
}

/////////////////////////////////////////////////////////////////
CCDRPartyNewRateInfo& CCDRPartyNewRateInfo::operator = (const CCDRPartyNewRateInfo& other)
{
	if (&other != this)
	{
		strcpy(m_partyName, other.m_partyName);
		m_partyId = other.m_partyId;
		m_partyCurrentRate = other.m_partyCurrentRate;
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CCDRPartyNewRateInfo::operator == (const CCDRPartyNewRateInfo& other)
{
	bool res1 = ( 	(strcmp(m_partyName ,other.m_partyName) == 0) &&
		    		(m_partyId == other.m_partyId) && (m_partyCurrentRate == other.m_partyCurrentRate ) );

	return res1 ;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CCDRPartyNewRateInfo::GetPartyName() const
{
	return m_partyName;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCDRPartyNewRateInfo::GetPartyId() const
{
	return m_partyId;
}

////////////////////////////////////////////////////////////////////////////
DWORD  CCDRPartyNewRateInfo::GetCurrentRate() const
{
	return m_partyCurrentRate;
}
///////////////////////////////////////////////////////////////////////////
//CNewRateInfo

/*************************************************************************/
/*							class CNewRateInfo								 */
/*************************************************************************/

////////////////////////////////////////////////////////////////////////////
CNewRateInfo::CNewRateInfo()
{
}

////////////////////////////////////////////////////////////////////////////
CNewRateInfo::~CNewRateInfo()
{
}

////////////////////////////////////////////////////////////////////////////
CNewRateInfo& CNewRateInfo::operator=(const CNewRateInfo &other)
{
	if(&other != this)
	{
		CCDRPartyNewRateInfo::operator =(other);
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////
bool CNewRateInfo::operator == (const CNewRateInfo &rHnd)
{
	if(&rHnd == this)
	{
		return true;
	}
	return CCDRPartyNewRateInfo::operator ==(rHnd);
}

////////////////////////////////////////////////////////////////////////////
void CNewRateInfo::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
	m_ostr << m_partyName 	<< ",";
  	m_ostr << m_partyId 	<< ",";
  	m_ostr << m_partyCurrentRate;


	m_ostr  << ";\n";
}

////////////////////////////////////////////////////////////////////////////
void CNewRateInfo::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
	m_istr.getline(m_partyName, H243_NAME_LEN+1, ',');
	m_istr >> m_partyId;
  	m_istr.ignore(1);
  	m_istr >> m_partyCurrentRate;

  	m_istr.ignore(1);
}

////////////////////////////////////////////////////////////////////////////
void CNewRateInfo::SerializeXml(CXMLDOMElement* pFatherNode)
{

	CXMLDOMElement* pNewRateNode = NULL;

	pNewRateNode = pFatherNode->AddChildNode("PARTICIPANT_CONNECTION_RATE");

	if (NULL == pNewRateNode)
	{
		return;
	}

	pNewRateNode->AddChildNode("NAME",m_partyName);
	pNewRateNode->AddChildNode("PARTY_ID",m_partyId);
	pNewRateNode->AddChildNode("PARTICIPANT_CURRENT_RATE",m_partyCurrentRate);

}

////////////////////////////////////////////////////////////////////////////
int  CNewRateInfo::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
/*
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_partyName,_1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_partyId,_0_TO_DWORD);
*/
	PASSERTMSG(TRUE, "CNewRateInfo::DeSerializeXml - Should not be called");

	return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////
void CNewRateInfo::SetPartyName(const char* h243partyname)
{
   int len = strlen(h243partyname);
   strncpy(m_partyName, h243partyname, H243_NAME_LEN - 1);
   if (len > H243_NAME_LEN-1)
        m_partyName[H243_NAME_LEN-1]= '\0';
}

/////////////////////////////////////////////////////////////////////////////
void  CNewRateInfo::SetPartyId(const DWORD partyId)
{
	m_partyId = partyId;
}

/////////////////////////////////////////////////////////////////////////////
void  CNewRateInfo::SetPartyCurrentRate(const DWORD currentRate)
{
	m_partyCurrentRate = currentRate;
}

////////////////////////////////////////////////////////////////////////////
const char*  CNewRateInfo::NameOf() const
{
	return "CNewRateInfo";
}
////////////////////////////////////////////////////////////////////////////
ACCCDREventPartyChairPerson::ACCCDREventPartyChairPerson()
{
	m_h243party_name[0]='\0';
	m_party_Id=0xffffffff;
	m_bChair = false;
}

ACCCDREventPartyChairPerson::ACCCDREventPartyChairPerson(const ACCCDREventPartyChairPerson &other)
{
	*this=other;
}

ACCCDREventPartyChairPerson::~ACCCDREventPartyChairPerson()
{
}

ACCCDREventPartyChairPerson& ACCCDREventPartyChairPerson::operator = (const ACCCDREventPartyChairPerson& other)
{
  strncpy(m_h243party_name,other.m_h243party_name,H243_NAME_LEN);
  m_party_Id = other.m_party_Id;
  m_bChair = other.m_bChair;
  return *this;
}

const char*  ACCCDREventPartyChairPerson::GetPartyName() const
{
    return m_h243party_name;
}

DWORD  ACCCDREventPartyChairPerson::GetPartyId() const
{
    return m_party_Id;
}

bool ACCCDREventPartyChairPerson::IsChairPerson() const
{
	return m_bChair;
}

/////////////////////////////////////////////////////////////////////////////

CCDRPartyChairPerson::CCDRPartyChairPerson()
{
}

CCDRPartyChairPerson::~CCDRPartyChairPerson()
{
}

void CCDRPartyChairPerson::Serialize(WORD format, std::ostream &m_ostr,	DWORD apiNum)
{
	if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
		m_ostr << m_h243party_name << ",";
	else
	{
		char tmp[H243_NAME_LEN_OLD];
		strncpy(tmp, m_h243party_name, H243_NAME_LEN_OLD - 1);
		tmp[H243_NAME_LEN_OLD - 1] = '\0';

		m_ostr << "party name:" << tmp << "\n";
	}

	m_ostr << m_party_Id << ",";
	m_ostr << m_bChair << ";\n";
}

void CCDRPartyChairPerson::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag, DWORD apiNum)
{
	if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
		m_ostr << "party name:" << m_h243party_name << "\n";
	else
	{
		char tmp[H243_NAME_LEN_OLD];
		strncpy(tmp, m_h243party_name, H243_NAME_LEN_OLD - 1);
		tmp[H243_NAME_LEN_OLD - 1] = '\0';

		m_ostr << "party name:" << tmp << "\n";
	}

	m_ostr << "party ID:" << m_party_Id << "\n";
	m_ostr << "Is chair:"<< m_bChair  << ";\n\n";
}

/////////////////////////////////////////////////////////////////////////////
void CCDRPartyChairPerson::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS

	//m_istr.ignore(1);
	if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
		m_istr.getline(m_h243party_name, H243_NAME_LEN + 1, ',');
	else
		m_istr.getline(m_h243party_name, H243_NAME_LEN_OLD + 1, ',');

	m_istr >> m_party_Id;
	m_istr.ignore(1);
	m_istr >> m_bChair;
	m_istr.ignore(1);
}

/////////////////////////////////////////////////////////////////////////////
//
void CCDRPartyChairPerson::SerializeXml(CXMLDOMElement* pFatherNode)
{
	PTRACE(eLevelDebug,"CCDRPartyChairPerson::SerializeXml - CDR-Chair - begin");
	CXMLDOMElement* pNode = pFatherNode->AddChildNode("PARTY_CHAIR_UPDATE");

	pNode->AddChildNode("NAME", m_h243party_name);
	pNode->AddChildNode("PARTY_ID", m_party_Id);
	pNode->AddChildNode("CHAIR", m_bChair, _BOOL);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_cdr_full.xsd
int CCDRPartyChairPerson::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"NAME", m_h243party_name, _1_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID", &m_party_Id, _0_TO_DWORD);
	DWORD dwTmp = 0;
	GET_VALIDATE_CHILD(pActionNode,"CHAIR", &dwTmp, _BOOL);
	m_bChair = dwTmp == YES;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char* CCDRPartyChairPerson::NameOf() const
{
  return "CCDRPartyChairPerson";
}

void CCDRPartyChairPerson::SetPartyName(const char* h243partyname)
{
   int len=strlen(h243partyname);
   strncpy(m_h243party_name, h243partyname, H243_NAME_LEN - 1);
   if (len>H243_NAME_LEN-1)
        m_h243party_name[H243_NAME_LEN-1]='\0';
}

void  CCDRPartyChairPerson::SetPartyId(const DWORD partyid)
{
     m_party_Id=partyid;
}

void CCDRPartyChairPerson::SetIsChairPerson(const bool bChair)
{
   m_bChair = bChair;
}
////////////////////////////////////////////////////////////////////////////
////Class CCDRPartyCallInfo/////////////////////////////////////////////
/*************************************************************************/
/*							class CCDRPartyCallInfo						 */
/*************************************************************************/

/////////////////////////////////////////////////////////////////
CCDRPartyCallInfo	::CCDRPartyCallInfo	()
{
	memset(&m_partyName, 0, H243_NAME_LEN);
	m_partyId       = 0;
	m_maxBitRate    = 0;
	memset(&m_maxResolution, 0, RESOLUTION_LEN);
    memset(&m_maxFrameRate,  0, FRAME_RATE_LEN);
    //m_maxFrameRate =0;
	memset(&m_IPaddress, 0, IPADDRESS_LEN);

}

/////////////////////////////////////////////////////////////////
CCDRPartyCallInfo::CCDRPartyCallInfo(const CCDRPartyCallInfo& other)
{
	strcpy(m_partyName, other.m_partyName);
	m_partyId = other.m_partyId;
	m_maxBitRate = other.m_maxBitRate;
	strcpy(m_maxResolution, other.m_maxResolution);
    strcpy(m_maxFrameRate, other.m_maxFrameRate);
    //m_maxFrameRate = other.m_maxFrameRate;
	strcpy(m_IPaddress, other.m_IPaddress);
}

/////////////////////////////////////////////////////////////////
CCDRPartyCallInfo::~CCDRPartyCallInfo()
{
}

/////////////////////////////////////////////////////////////////
CCDRPartyCallInfo& CCDRPartyCallInfo::operator = (const CCDRPartyCallInfo& other)
{
	if (&other != this)
	{
		strcpy(m_partyName, other.m_partyName);
		m_partyId = other.m_partyId;
		m_maxBitRate = other.m_maxBitRate;
		strcpy(m_maxResolution, other.m_maxResolution);
        strcpy(m_maxFrameRate, other.m_maxFrameRate);
        //m_maxFrameRate = other.m_maxFrameRate;
		strcpy(m_IPaddress, other.m_IPaddress);
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CCDRPartyCallInfo::operator == (const CCDRPartyCallInfo& other)
{
	bool res1 = ( 	(strcmp(m_partyName ,other.m_partyName) == 0) &&
		    		(m_partyId == other.m_partyId) && (m_maxBitRate == other.m_maxBitRate) &&
		    		(strcmp(m_maxResolution ,other.m_maxResolution) == 0) && (strcmp(m_maxFrameRate ,other.m_maxFrameRate) == 0) && //(m_maxFrameRate == other.m_maxFrameRate) &&
		    		(strcmp(m_IPaddress ,other.m_IPaddress) == 0) );

	return res1 ;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CCDRPartyCallInfo::GetPartyName() const
{
	return m_partyName;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCDRPartyCallInfo::GetPartyId() const
{
	return m_partyId;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCDRPartyCallInfo::GetMaxBitRate() const
{
	return m_maxBitRate;
}
////////////////////////////////////////////////////////////////////////////
const char *CCDRPartyCallInfo::GetMaxResolution() const
{
	return m_maxResolution;
}
////////////////////////////////////////////////////////////////////////////
const char *CCDRPartyCallInfo::GetMaxFrameRate() const    //const WORD CCDRPartyCallInfo::GetMaxFrameRate() const
{
	return m_maxFrameRate;
}
////////////////////////////////////////////////////////////////////////////
const char* CCDRPartyCallInfo::GetAddress() const
{
	return m_IPaddress;
}
//////////////////////////////////////////////////////////////////////////
//CCallInfo

/*************************************************************************/
/*							class CCallInfo		     					 */
/*************************************************************************/

////////////////////////////////////////////////////////////////////////////
CCallInfo::CCallInfo()
{
}

////////////////////////////////////////////////////////////////////////////
CCallInfo::~CCallInfo()
{
}

////////////////////////////////////////////////////////////////////////////
CCallInfo& CCallInfo::operator=(const CCallInfo &other)
{
	if(&other != this)
	{
		CCDRPartyCallInfo::operator =(other);
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////
bool CCallInfo::operator == (const CCallInfo &rHnd)
{
	if(&rHnd == this)
	{
		return true;
	}
	return CCDRPartyCallInfo::operator ==(rHnd);
}

////////////////////////////////////////////////////////////////////////////
void CCallInfo::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
	m_ostr << m_partyName 	   << ",";
  	m_ostr << m_partyId 	   << ",";
  	m_ostr << m_maxBitRate     << ",";
  	m_ostr << m_maxResolution << ",";
  	m_ostr << m_maxFrameRate   << ",";
  	m_ostr << m_IPaddress;
  	//m_ostr << m_address        << ",";
  	//m_ostr << (WORD)m_endPointType;
	m_ostr  << ";\n";
}

////////////////////////////////////////////////////////////////////////////
void CCallInfo::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
	m_istr.getline(m_partyName, H243_NAME_LEN+1, ',');
	m_istr >> m_partyId;
  	m_istr.ignore(1);
  	m_istr >> m_maxBitRate;
  	m_istr.ignore(1);
  	m_istr.getline(m_maxResolution, RESOLUTION_LEN+1, ',');
    m_istr.getline(m_maxFrameRate, FRAME_RATE_LEN+1, ',');
   //m_istr >> m_maxFrameRate;
   //m_istr.ignore(1);
    m_istr.getline(m_IPaddress, IPADDRESS_LEN+1, ';');
    //m_istr.ignore(1);

  	//m_istr.ignore(1);
}

////////////////////////////////////////////////////////////////////////////
void CCallInfo::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pCallInfo = NULL;

	pCallInfo = pFatherNode->AddChildNode("PARTICIPANT_MAX_USAGE_INFO");

	if (NULL == pCallInfo)
	{
		return;
	}

	pCallInfo->AddChildNode("NAME",m_partyName);
	pCallInfo->AddChildNode("PARTY_ID",m_partyId);
	pCallInfo->AddChildNode("MAX_RATE",m_maxBitRate);
	pCallInfo->AddChildNode("MAX_RESOLUTION",m_maxResolution);
	pCallInfo->AddChildNode("MAX_FRAME_RATE",m_maxFrameRate);
	pCallInfo->AddChildNode("ADDRESS",m_IPaddress);

	//pCallInfo->AddChildNode("END_POINT_TYPE",m_endPointType,END_POINT_TYPE_ENUM);
}

////////////////////////////////////////////////////////////////////////////
int  CCallInfo::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{

	PASSERTMSG(TRUE, "CCallInfo::DeSerializeXml - Should not be called");

	return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////
void CCallInfo::SetPartyName(const char* h243partyname)
{
	strcpy_safe(m_partyName, h243partyname);
}

/////////////////////////////////////////////////////////////////////////////
void  CCallInfo::SetPartyId(const DWORD partyId)
{
	m_partyId = partyId;
}

/////////////////////////////////////////////////////////////////////////////
void  CCallInfo::SetPartyMaxBitRate(const DWORD maxBitRate)//currentRate)
{
	m_maxBitRate = maxBitRate;
}

/////////////////////////////////////////////////////////////////////////////
void  CCallInfo::SetPartyMaxResolution(const char *maxResolution)
{
	strcpy_safe(m_maxResolution, maxResolution);
}

/////////////////////////////////////////////////////////////////////////////
void  CCallInfo::SetPartyMaxFrameRate(const char *maxFrameRate)
{
	strcpy_safe(m_maxFrameRate, maxFrameRate);
}

/////////////////////////////////////////////////////////////////////////////
void CCallInfo::SetPartyAddress(const char* address)
{
	strcpy_safe(m_IPaddress, address);
}

////////////////////////////////////////////////////////////////////////////
const char*  CCallInfo::NameOf() const
{
	return "CCallInfo";
}
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////

CCDRPartyCorrelationDataInfo::CCDRPartyCorrelationDataInfo()
{
	m_party_name[0]='\0';
	m_party_Id=0xffffffff;
	m_sig_uuid[0]='\0';
}

CCDRPartyCorrelationDataInfo::CCDRPartyCorrelationDataInfo(const CCDRPartyCorrelationDataInfo &other)
{

	strncpy(m_party_name,other.m_party_name,H243_NAME_LEN);
        m_party_Id=other.m_party_Id;
        strncpy(m_sig_uuid,other.m_sig_uuid,CORRELATION_SIG_UUID_LEN);
}

CCDRPartyCorrelationDataInfo::~CCDRPartyCorrelationDataInfo()
{
}
CCDRPartyCorrelationDataInfo& CCDRPartyCorrelationDataInfo::operator = (const CCDRPartyCorrelationDataInfo& other)
{

	memset(m_party_name, 0, H243_NAME_LEN);
	int len = strlen(other.m_party_name);
	strncpy(m_party_name, other.m_party_name, H243_NAME_LEN -1 );
	if (len > H243_NAME_LEN-1)
		m_party_name[H243_NAME_LEN-1]= '\0';

	//TRACEINTOFUNC << "m_party_name = "<<m_party_name <<"other m_party_name"<<other.m_party_name;
	//strncpy(m_party_name,"abce",4);
	//m_party_name[3]= '\0';
	m_party_Id = 2;
	memset(m_sig_uuid, 0, CORRELATION_SIG_UUID_LEN);

	len = strlen(other.m_sig_uuid);
	strncpy(m_sig_uuid, other.m_sig_uuid, CORRELATION_SIG_UUID_LEN -1 );
	if (len > CORRELATION_SIG_UUID_LEN-1)
		m_sig_uuid[CORRELATION_SIG_UUID_LEN-1]= '\0';

 /* strncpy(m_party_name,other.m_party_name,H243_NAME_LEN);
  m_party_Id = other.m_party_Id;
  strncpy(m_sig_uuid,other.m_sig_uuid,CORRELATION_SIG_UUID_LEN);*/
/*	strncpy(m_party_name,"pName",4);
	m_party_Id = 2;
	strncpy(m_sig_uuid,"SIG",2);*/

  return *this;
}
////
bool CCDRPartyCorrelationDataInfo::operator == (const CCDRPartyCorrelationDataInfo& other)
{
	bool res1 = ( 	(strcmp(m_party_name ,other.m_party_name) == 0) &&
		    		(m_party_Id == other.m_party_Id) &&
		    		(strcmp(m_sig_uuid ,other.m_sig_uuid) == 0));


	return res1 ;
}

const char*  CCDRPartyCorrelationDataInfo::GetPartyName() const
{
    return m_party_name;
}
DWORD  CCDRPartyCorrelationDataInfo::GetPartyId() const
{
    return m_party_Id;
}
const char* CCDRPartyCorrelationDataInfo::GetSigUid() const
{
	return m_sig_uuid;
}

///////////////
/*************************************************************************/
/*							class CPartyCorrelationData					 */
/*************************************************************************/

////////////////////////////////////////////////////////////////////////////
CPartyCorrelationData::CPartyCorrelationData()
{
}

////////////////////////////////////////////////////////////////////////////
CPartyCorrelationData::~CPartyCorrelationData()
{
}
////////////////////////////////////////////////////////////////////////////
CPartyCorrelationData& CPartyCorrelationData::operator=(const CPartyCorrelationData &other)
{
	if(&other != this)
	{
		CCDRPartyCorrelationDataInfo::operator =(other);
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////
bool CPartyCorrelationData::operator == (const CPartyCorrelationData &rHnd)
{
	if(&rHnd == this)
	{
		return true;
	}
	return CCDRPartyCorrelationDataInfo::operator ==(rHnd);
}

////////////////////////////////////////////////////////////////////////////
void CPartyCorrelationData::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
	TRACEINTO << "CPartyCorrelationData::Serialize m_sig_uuid= " << m_sig_uuid;

	m_ostr << m_party_Id 	   << ",";
	m_ostr << m_sig_uuid 	   << ",";
	m_ostr << m_party_name;
	//m_ostr << m_address        << ",";
	//m_ostr << (WORD)m_endPointType;
	m_ostr  << ";\n";
}

////////////////////////////////////////////////////////////////////////////
void CPartyCorrelationData::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
	TRACEINTO << "CPartyCorrelationData::DeSerialize m_sig_uuid= " << m_sig_uuid;
	m_istr >> m_party_Id;
	m_istr.ignore(1);
	//m_istr >> m_sig_uuid;
	//m_istr.ignore(1);
	m_istr.getline(m_sig_uuid, CORRELATION_SIG_UUID_LEN + 1, ',');
	m_istr.getline(m_party_name, H243_NAME_LEN+1, ';');

}

////////////////////////////////////////////////////////////////////////////
void CPartyCorrelationData::SerializeXml(CXMLDOMElement* pFatherNode)
{

	FPTRACE(eLevelInfoNormal,"CPartyCorrelationData::SerializeXml -   ");
	CXMLDOMElement* pCorrelationData = NULL;

	pCorrelationData = pFatherNode->AddChildNode("PARTY_CORRELATION_DATA");

	if (NULL == pCorrelationData)
	{
		return;
	}


	pCorrelationData->AddChildNode("PARTY_ID",m_party_Id);
	pCorrelationData->AddChildNode("SIG_UUID",m_sig_uuid);
	pCorrelationData->AddChildNode("NAME",m_party_name);


}

////////////////////////////////////////////////////////////////////////////
int  CPartyCorrelationData::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{


	PASSERTMSG(TRUE, "CPartyCorrelationData::DeSerializeXml - Should not be called");

	return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////
void CPartyCorrelationData::SetPartyName(const char* h243partyname)
{

   strncpy(m_party_name, h243partyname, H243_NAME_LEN-1);
   m_party_name[H243_NAME_LEN-1]= '\0';

}

/////////////////////////////////////////////////////////////////////////////
void  CPartyCorrelationData::SetPartyId(const DWORD partyId)
{

	m_party_Id = partyId;

}
/////////////////////////////////////////////////////////////////////////////
void  CPartyCorrelationData::SetSigUuid(const char*  sigUuid)
{
	TRACEINTO << "CPartyCorrelationData::SetSigUuid sigUuid= " << sigUuid;

	strncpy(m_sig_uuid, sigUuid, CORRELATION_SIG_UUID_LEN-1);
	m_sig_uuid[CORRELATION_SIG_UUID_LEN-1]= '\0';
	TRACEINTO << "CPartyCorrelationData::SetSigUuid m_sig_uuid= " << m_sig_uuid;

}
/////////////////////////////////////////////////////////////////////////////
const char*  CPartyCorrelationData::NameOf() const
{
	return "CPartyCorrelationData";
}

//CConfCorrelationDataInfo:
/////////////////////////////////////////////

CCDRConfCorrelationDataInfo::CCDRConfCorrelationDataInfo()
{
	m_conf_uuid = "";
}

CCDRConfCorrelationDataInfo::CCDRConfCorrelationDataInfo(const CCDRConfCorrelationDataInfo &other)
{
	*this=other;
}

CCDRConfCorrelationDataInfo::~CCDRConfCorrelationDataInfo()
{
}
CCDRConfCorrelationDataInfo& CCDRConfCorrelationDataInfo::operator = (const CCDRConfCorrelationDataInfo& other)
{
  m_conf_uuid = other.m_conf_uuid;
  return *this;
}
////
bool CCDRConfCorrelationDataInfo::operator == (const CCDRConfCorrelationDataInfo& other)
{
	bool res1 = (m_conf_uuid == other.m_conf_uuid);

	return res1 ;
}
std::string CCDRConfCorrelationDataInfo::GetSigUid() const
{
	return m_conf_uuid;
}

///////////////
