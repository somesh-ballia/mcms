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
// FILE:   CDRLGAPI.cpp                                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:Michael                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/04/97     |                                                      |
//+========================================================================+



#include "CDRLogApi.h"
#include "Native.h"
#include "CDRDefines.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "OpcodesMcmsInternal.h"
#include "psosxml.h"
#include "EncodingConvertor.h"
#include "ObjString.h"


CCdrLogApi::CCdrLogApi()
:CTaskApi(eProcessCDR,eManager)
{
}

/////////////////////////////////////////////////////////////////////////////
STATUS  CCdrLogApi::StartConference(const CCdrShort &cdrshort)  
{
	PTRACE(eLevelInfoNormal, "CCdrLogApi::StartConference");

    bool validationRes = CheckValidityShort(cdrshort);
    if(!validationRes)
    {
        return STATUS_FAIL;
    }
    
  	CSegment*  seg = new CSegment;
  	CCdrShortDrv* pShortDrv = (CCdrShortDrv*)&cdrshort;
  	pShortDrv->Serialize(NATIVE,*seg);
  	STATUS status = SendMsg(seg,CDR_START_CONF);
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS  CCdrLogApi::ConferenceEvent(DWORD confId, const CCdrEvent &event)  
{
	//PTRACE(eLevelInfoNormal, "CCdrLogApi::ConferenceEvent");

    bool validationRes = CheckValidityEvent(event);
    if(!validationRes)
    {
        return STATUS_FAIL;
    }
    
  	CSegment*  seg = new CSegment;
  	CCdrEventDrv* pEventDrv = (CCdrEventDrv*)&event;
  	*seg << confId;
  	pEventDrv->Serialize(NATIVE,*seg);
  	STATUS status = SendMsg(seg,CDR_EVENT);
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS  CCdrLogApi::EndConference(DWORD confid,
								  eConfCdrStatus cause,
								  const CStructTm &actualStartTime,
								  const CStructTm &actualduration,
								  const CCdrEvent &event) 
{
	PTRACE(eLevelInfoNormal, "CCdrLogApi::EndConference");
	
  	CSegment*  seg = new CSegment;
  	CCdrEventDrv* pEventDrv = (CCdrEventDrv*)&event;
	CStructTmDrv* pStartTimeDrv = (CStructTmDrv*)&actualStartTime;	    		
 	CStructTmDrv* pActualDurationDrv = (CStructTmDrv*)&actualduration;	    		
 	
 	*seg << confid
		 << (WORD)cause;
 	
 	pStartTimeDrv->Serialize(NATIVE,*seg);	
 	pActualDurationDrv->Serialize(NATIVE,*seg);	
 	pEventDrv->Serialize(NATIVE,*seg);
  	
 	STATUS status = SendMsg(seg,CDR_END_CONF);
	return status;
}

/////////////////////////////////////////////////////////////////////////////
bool CCdrLogApi::CheckValidityShort(const CCdrShort &cdrshort)const
{
    bool formatRes = CheckValidityUTF8<CCdrShort>(cdrshort);
    
    return formatRes;
}

/////////////////////////////////////////////////////////////////////////////
bool CCdrLogApi::CheckValidityEvent(const CCdrEvent &event)const
{
/**/    bool bIsValidEvent = CCdrEvent::IsValidEvent(event.GetCdrEventType());
    if (!bIsValidEvent)
    {
   		PASSERTMSG(false == bIsValidEvent,"CCdrLogApi::CheckValidityEvent - event not valid ");
   		return false;
    }/**/

    bool formatRes = CheckValidityUTF8<CCdrEvent>(event);

    return formatRes;
}

/////////////////////////////////////////////////////////////////////////////
template <class ObjToCheckT> // should have SerializeXml(CXMLDOMElement)
bool CCdrLogApi::CheckValidityUTF8(const ObjToCheckT & objToCheck)const
{
    CXMLDOMElement *pFatherNode = new CXMLDOMElement("Validation UTF-8");
    objToCheck.SerializeXml(pFatherNode);

    char *buff = NULL;
    pFatherNode->DumpDataAsLongStringEx(&buff, TRUE);
    
    COstrStream statusString;
    STATUS statusFormat = CEncodingConvertor::ValidateString("UTF-8",
                                                             buff,
                                                             statusString);
    if(STATUS_OK != statusFormat)
    {
        CLargeString message = "CCdrLogApi::CheckValidityUTF8: ";
        message << CProcessBase::GetProcess()->GetStatusAsString(statusFormat).c_str() << "\n"
                << statusString.str().c_str()
                << "\n\n--------------------------------------------\n\n"
                << buff;
        PASSERTMSG(TRUE, message.GetString());
    }
    
    PDELETE(pFatherNode);
    PDELETEA(buff);
    
    return (STATUS_OK == statusFormat);
}


