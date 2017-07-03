//+========================================================================+
//                       PcmConfirms.h                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PcmConfirms.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Eitan                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  August - 2007  | Description                               |
//-------------------------------------------------------------------------|
//			
//+========================================================================++

#ifndef _PCM_CONFIRMS_H_
#define _PCM_CONFIRMS_H_

#include "PcmMessage.h"

class CPcmPopMenuStatusConfirm : public CPcmConfirm {
CLASS_TYPE_1(CPcmPopMenuStatusConfirm,CPcmConfirm)

public:
	CPcmPopMenuStatusConfirm(int termId = 0);
	virtual ~CPcmPopMenuStatusConfirm();
	CPcmPopMenuStatusConfirm(CPcmPopMenuStatusConfirm& other);
	virtual const char*  NameOf() const{ return "CPcmPopMenuStatusConfirm"; }
	virtual CPcmMessage* Clone() {return new CPcmPopMenuStatusConfirm;}
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	
	virtual void Dump();
	int GetResult(){return result;}
	void SetResult(int _result){result = _result;}

protected:
	int result;

};

#endif //_PCM_CONFIRMS_H_
