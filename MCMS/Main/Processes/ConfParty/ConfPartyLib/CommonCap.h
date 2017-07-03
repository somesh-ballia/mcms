//+========================================================================+
//                            COMMCAP.H                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       COMMCAP.H                                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: ANAT A                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |06/08/03     |                                                     |
//+========================================================================+
#ifndef __COMMCAP_H__
#define __COMMCAP_H__


//#include "stdafx.h"
//#include "Mcmsoper.h"
#include  "PObject.h"
//#include  "nstream.h"

//class COstrStream;
//class CIstrStream;
class CSegment;    
class CXMLDOMElement;                                          

class CSecondaryParams : public CPObject
{
CLASS_TYPE_1(CSecondaryParams, CPObject)	
public:
    // constructors / destructor
    CSecondaryParams():capCode(0),resolution(0),frameRate(0),lineRate(0),problemParam(0),rmtProblemValue(0),currProblemValue(0),confType(0){}
    ~CSecondaryParams(){}
	virtual const char* NameOf() const { return "CSecondaryParams";}


//#ifdef __HIGHC__
	void  DeSerialize(WORD format,CSegment& seg);
	void  Serialize(WORD format,CSegment& seg) const;
//#endif

	/*void  Serialize(WORD format, COstrStream  &m_ostr, DWORD apiNum);
	void  DeSerialize(WORD format, CIstrStream &m_istr, DWORD apiNum);*/ //not needed - XML
	int   DeSerializeXml(CXMLDOMElement *pConfPartyNode,char* pszError);
	void  SerializeXml(CXMLDOMElement *pConfPartyNode);

	CSecondaryParams(const CSecondaryParams &other);
	CSecondaryParams& operator = (const CSecondaryParams& other);

	unsigned short				capCode;
	unsigned short				resolution;
	unsigned short				frameRate;
	unsigned short				lineRate;
	unsigned short				problemParam;
	long						rmtProblemValue; //changed from short to long in API ___
	long						currProblemValue;//changed from short to long in API ___
	unsigned long				confType;		 //for sip conferencing limitation
};

typedef enum
{
	UnKnown,
	CapCode,
	Resolution,
	FrameRate,
	LineRate,
	Annexes,
	Lacks263Plus,
	RoleLabel,
	PayloadType,
	//values only for H264 use:
	Level,
	MBPS, 
	FS,
	DPB,
	BrAndCpb,
	SipConfLimitation
} ESecondaryParam;

static char* SecParamsH323Array[] = {
	"UnKnown",
	"protocol",
	"resolution",
	"frame rate",
	"line rate",
	"annex",
	"lacks 263 plus capabilities",
	"role label",
	"payload type",
	//values only for H264 use:
	"level",
	"MBPS", 
	"FS",
	"DPB",
	"BR and CPB",
	"Sip conf limitation"
};
#endif
