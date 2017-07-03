//+========================================================================+
//                            SecondaryParameters.CPP                                   |
//            Copyright 1995 Polycom Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SecondaryParameters.CPP                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: ANAT A                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |06/08/03     |                                                     |
//+========================================================================+

//#ifdef __HIGHC__

#include "SecondaryParameters.h"
#include "Segment.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "ConfPartyProcess.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"

//#ifndef __COMMCAP_H__
//#include <CommCap.h>
//#endif
//
//#ifndef _MCMSOPER
//#include  <mcmsoper.h>
//#endif
//
//#ifndef _PSOS_XML_
//#include <psosxml.h>
//#endif
//
//#ifndef _COBJSTRINGAPP
//#include    <obstring.h>
//#endif
//
//#ifndef __H323CAPABILITIES_H__
//#include   <h323oper.h>
//#endif
//#include <typedef.h>
//
//#else // __HIGHC__
//
//#include "CommCap.h"
//#include "nstream.h"

//#ifndef _MCMSOPER
//#include "Mcmsoper.h"
//#endif
//
//#include "psosxml.h"
//#include "typedef.h"
//#endif // __HIGHC__



CSecondaryParams ::CSecondaryParams(const CSecondaryParams &other):CPObject(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////	
CSecondaryParams& CSecondaryParams::operator = (const CSecondaryParams& other)
{
  m_capCode				= other.m_capCode;  
  m_resolution			= other.m_resolution;
  m_frameRate			= other.m_frameRate;
  m_lineRate			= other.m_lineRate;
  m_problemParam		= other.m_problemParam;  
  m_rmtProblemValue		= other.m_rmtProblemValue;
  m_currProblemValue	= other.m_currProblemValue;
  m_confType			= other.m_confType;

  return *this;
}

//#ifdef __HIGHC__
void  CSecondaryParams::Serialize(WORD format,CSegment  &seg) const
{
	switch ( format )  
	{
						  
	case NATIVE     : 
		{		
		seg << m_capCode;
		seg	<< m_resolution;
		seg	<< m_frameRate;
		seg	<< m_lineRate;
		seg	<< m_problemParam;
		seg	<< m_rmtProblemValue;
		seg	<< m_currProblemValue;
		seg << m_confType;
		break;
		}
	default : 
		break;			  
    }
		
}

/////////////////////////////////////////////////////////////////////////////	* /
void  CSecondaryParams::DeSerialize(WORD format,CSegment &seg)
{
	switch ( format )  
	{
						  
	case NATIVE     : 
		{
		seg >> m_capCode
			>> m_resolution
			>> m_frameRate
			>> m_lineRate
			>> m_problemParam
			>> m_rmtProblemValue
			>> m_currProblemValue
			>> m_confType;
		
		break;
		}
	default : {
		break;
			  }
    }
}

//#endif
/*
/////////////////////////////////////////////////////////////////////////////
void   CSecondaryParams::Serialize(WORD format, COstrStream &m_ostr, DWORD apiNum)
{
 // assuming format = OPERATOR_MCMS
	
	m_ostr << (WORD)m_capCode	<< "\n";		
	m_ostr << (WORD)m_resolution	<< "\n";	
	m_ostr << (WORD)m_frameRate	<< "\n";
	m_ostr << (WORD)m_lineRate	<< "\n";
	m_ostr << (WORD)m_problemParam		<< "\n";

	// since there no API number just XML and 
	if (apiNum >= API_NUM_H264)
	{
		m_ostr << (DWORD)m_rmtProblemValue  << "\n";
		m_ostr << (DWORD)m_currProblemValue << "\n";
	}
	else
	{
		m_ostr << (WORD)m_rmtProblemValue	 << "\n";
		m_ostr << (WORD)m_currProblemValue << "\n";
	}
	if (apiNum >= API_SIP_CONF_LOMITATION)
		m_ostr << (DWORD) m_confType << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void   CSecondaryParams::DeSerialize(WORD format, CIstrStream &m_istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
	m_istr >> (WORD)m_capCode;
	m_istr >> (WORD)m_resolution;
	m_istr >> (WORD)m_frameRate;
	m_istr >> (WORD)m_lineRate;
	m_istr >> (WORD)m_problemParam;
	
	if (apiNum >= API_NUM_H264)
	{
		m_istr >> (DWORD)m_rmtProblemValue;
	    m_istr >> (DWORD)m_currProblemValue;
	}
	else
	{
		WORD word_rmtProblemValue  = (WORD)m_rmtProblemValue;
		WORD word_currProblemValue = (WORD)m_currProblemValue;
		m_istr >> word_rmtProblemValue;
	    m_istr >> word_currProblemValue;
	}
	if (apiNum >= API_SIP_CONF_LOMITATION)
		m_istr >> (DWORD)m_confType;
}
*/
/////////////////////////////////////////////////////////////////////////////
void   CSecondaryParams::SerializeXml(CXMLDOMElement *pConfPartyNode)
{
	pConfPartyNode->AddChildNode("CAP_CODE",m_capCode,CAP_CODE_ENUM);
	pConfPartyNode->AddChildNode("FRAME_RATE_VALUE",m_frameRate);
	pConfPartyNode->AddChildNode("LINE_RATE_VALUE",m_lineRate);
	pConfPartyNode->AddChildNode("VIDEO_RESOLUTION",m_resolution,VIDEO_RESOLUTION_ENUM);
	pConfPartyNode->AddChildNode("SECONDARY_PROBLEM",m_problemParam,SECONDARY_PROBLEM_ENUM);
	pConfPartyNode->AddChildNode("SECONDARY_PROBLEM_VALUE",m_currProblemValue);
	pConfPartyNode->AddChildNode("REMOTE_SECONDARY_PROBLEM_VALUE",m_rmtProblemValue);
	pConfPartyNode->AddChildNode("SIP_CONFERENCING_LIMITATION",m_confType, SIP_CONFERENCING_LIMITATION_ENUM);
}
/////////////////////////////////////////////////////////////////////////////
int CSecondaryParams::DeSerializeXml(CXMLDOMElement *pConfPartyNode,char* pszError)
{
	int nStatus;

	GET_VALIDATE_CHILD(pConfPartyNode,"CAP_CODE",&m_capCode,CAP_CODE_ENUM);
	GET_VALIDATE_CHILD(pConfPartyNode,"FRAME_RATE_VALUE",&m_frameRate,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pConfPartyNode,"LINE_RATE_VALUE",&m_lineRate,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pConfPartyNode,"VIDEO_RESOLUTION",&m_resolution,VIDEO_RESOLUTION_ENUM);
	GET_VALIDATE_CHILD(pConfPartyNode,"SECONDARY_PROBLEM",&m_problemParam,SECONDARY_PROBLEM_ENUM);
	GET_VALIDATE_CHILD(pConfPartyNode,"SECONDARY_PROBLEM_VALUE",&m_currProblemValue,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pConfPartyNode,"REMOTE_SECONDARY_PROBLEM_VALUE",&m_rmtProblemValue,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pConfPartyNode,"SIP_CONFERENCING_LIMITATION",&m_confType,SIP_CONFERENCING_LIMITATION_ENUM);

	return STATUS_OK;
}
