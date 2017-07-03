//+========================================================================+
//                                                                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE: CDRLGAPI.H                                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Michael                                                            |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/04/97     |                                                      |
//+========================================================================+


#ifndef _CDRLGAPI
#define _CDRLGAPI



#include  "ConfPartySharedDefines.h"
#include "XmlApi.h"
#include "TaskApi.h"


class CCdrShort;
class CCdrEvent;
class CStructTm;
class CSerializeObject;


                        
class CCdrLogApi : public CTaskApi
{
CLASS_TYPE_1(CCdrLogApi,CTaskApi )

public: 
	CCdrLogApi();
	virtual ~CCdrLogApi() {};
	virtual const char* NameOf() const { return "CCdrLogApi";}


  	STATUS StartConference(const CCdrShort &cdrshort);
  	STATUS ConferenceEvent(DWORD confId, const CCdrEvent &event);
  	STATUS EndConference(DWORD confid, eConfCdrStatus cause, const CStructTm& StartTime, const CStructTm &duration,  const CCdrEvent &event); 

private:
	//STATUS SendMessageToCDR(CSegment *seg);
    bool CheckValidityShort(const CCdrShort &cdrshort)const;
    bool CheckValidityEvent(const CCdrEvent &event)const;

    template <class ObjToCheckT>
    bool CheckValidityUTF8(const ObjToCheckT & objToCheck)const;
};

#endif /* _CDRLGAPI */
