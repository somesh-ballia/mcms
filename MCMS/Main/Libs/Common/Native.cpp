/*$Header: /MCMS/MAIN/subsys/mcms/NATIVE.CPP 8     27/06/01 14:17 Hagai $*/
//+========================================================================+
//                            NATIVE.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       NATIVE.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 8/15/95     |                                                     |
//+========================================================================+

#include "Native.h"
#include "Macros.h"
#include "Segment.h"
#include "NStream.h"

#define	PARTY_SERIALIZE_SIZE	400

/////////////////////////////////////////////////////////////////////////////
void CStructTmDrv::Serialize(WORD format,CSegment& seg)
{
	if (format != NATIVE) PASSERT(1);
  
  	COstrStream ostr;
  	CStructTm::Serialize(ostr);
  	seg << ostr.str();
} 

/////////////////////////////////////////////////////////////////////////////
void CStructTmDrv::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1);
    
    const char *ptr = seg.GetString();
    CIstrStream istr(ptr);
  	CStructTm::DeSerialize(istr);
  	PDELETEA(ptr);
} 

 
/////////////////////////////////////////////////////////////////////////////
/*
void CRsrvPartyDrv::Serialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1);
    char* p = CRsrvParty::Serialize(format);
    seg << p;
    //Notice, the function CRsrvParty::Serialize()
    //returns allocation to "new char[b+1];"
    PDELETEA(p)
} 

/////////////////////////////////////////////////////////////////////////////
void CRsrvPartyDrv::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1); 
    char* p = seg.GetString(); 
    CRsrvParty::DeSerialize(format, p); 
    //Notice, the function seg.GetString() 
    //returns allocation to "new char[strLen];"
    PDELETEA(p)
} 

/////////////////////////////////////////////////////////////////////////////
void CSetRequestDrv::Serialize(WORD format,CSegment& seg)
{ 
  if (format != NATIVE) PASSERT(1);
  char* p = CSetRequest::Serialize(OPERATOR_MCMS);
  seg << p;
  //CSetRequest::Serialize returns "new char[b+1]"
  PDELETEA(p)
} 

/////////////////////////////////////////////////////////////////////////////
void CSetRequestDrv::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1);
    char* p = seg.GetString();
    CSetRequest::DeSerialize(OPERATOR_MCMS, p); 
    //Notice, the function seg.GetString() 
    //returns allocation to "new char[strLen];"
    PDELETEA(p)
} 

/////////////////////////////////////////////////////////////////////////////
void CHlogElementDrv::Serialize(WORD format,CSegment& seg)
{ 
  if (format != NATIVE) PASSERT(1);
  WORD n;
  //BYTE* p = CHlogElement::Serialize(OPERATOR_MCMS, n);
  BYTE* p = ((CHlogElement*)this)-> Serialize(OPERATOR_MCMS, n); // virtual
  seg.Put(p,n);
  PDELETEA(p)
} 

/////////////////////////////////////////////////////////////////////////////
void CHlogElementDrv::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1);
    //BYTE* p = new BYTE[sizeof(hlogType)];
    BYTE* p = new BYTE[((CHlogElement*)this)->Sizeof()]; // virtual call
    seg.Get(p,((CHlogElement*)this)->Sizeof());
    //CHlogElement::DeSerialize(OPERATOR_MCMS, p);
    ((CHlogElement*)this)->DeSerialize(OPERATOR_MCMS, p); // virtual 
    PDELETEA(p)
}

#ifdef __HIGHC__
/////////////////////////////////////////////////////////////////////////////
void CHlogElementDrv::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement *pHLogNode = pFatherNode->AddChildNode("HLOG_DRV_ELEMENT");
	SerializeXmlCommon(pHLogNode);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CHlogElementDrv::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError);
	if( nStatus )
		return nStatus;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

CCommResDrv::CCommResDrv():CCommRes()	     
{  
}
/////////////////////////////////////////////////////////////////////////////
void CCommResDrv::Serialize(WORD format,CSegment& seg)
{
  if (format != NATIVE) PASSERT(1);
  //ostrstream*      m_pOstr;  
  COstrStream*      pOstr;  

  int bufferSize = MAX_PARTIES_IN_CONF * PARTY_SERIALIZE_SIZE;

  ALLOCBUFFER(msg_info, bufferSize);
  pOstr= new COstrStream(msg_info, bufferSize);    

	// ... while this section was made by Natalie, for more than 30 parties
  //WORD numParties = CCommRes::GetNumParties();
  //DWORD numBytes = 1000 + numParties * 1000;  //1000 bytes for one conference; 1000 bytes for one party
  //char* msg_info = new char[numBytes];
  //pOstr= new COstrStream(msg_info,numBytes);    


  // yurir  
  CCommRes::Serialize(format, *pOstr); 
  int b=pOstr->pcount();
  ALLOCBUFFER(msg,b+1);
  memset(msg,' ', b);
  memcpy(msg, msg_info,b); 
  msg[b]='\0';
  DEALLOCBUFFER(msg_info)
  POBJDELETE(pOstr);

  //char msg1[b+2];
  ALLOCBUFFER(msg1, b+2);
  msg1[0]='$';
  msg1[1]='\0';
  strcat(msg1, msg);

  seg << msg1;

  //check if buffer size is not too small for data.
  if((bufferSize - 100) <= b)
  {
	  if(bufferSize <= b)
		  PTRACE(eLevelError, "CCommResDrv::Serialize, Buffer is too small for serializing all the data! ");
	  else
		  PTRACE(eLevelError, "CCommResDrv::Serialize, Buffer is almost too small for serializing all the data! ");
	  PASSERT(1);
  }

  //seg << msg;
  DEALLOCBUFFER(msg)
  DEALLOCBUFFER(msg1)
} 


/////////////////////////////////////////////////////////////////////////////
void CCommResDrv::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1); 
    char* msg = seg.GetString(); 
    
    CIstrStream*      pIstr;  
    pIstr= new CIstrStream(msg);    
    CCommRes::DeSerialize(format, *pIstr);
    POBJDELETE(pIstr);
    //Notice, the function seg.GetString() 
    //returns allocation to "new char[strLen];"
    PDELETEA(msg)
} 

/////////////////////////////////////////////////////////////////////////////
void  CCommResDrv::SetStartTime(const CStructTm &other)
{
  CMcuTime McuTime;
  CStructTm tmpTime(other);
  McuTime.LocalTime(tmpTime, ::GetpMcuTime()->GetGMTOffset(),
	                 !(::GetpMcuTime()->GetGMTOffsetSign()));
  m_startTime=tmpTime;   

}

/////////////////////////////////////////////////////////////////////////////
const CStructTm*  CCommResDrv::GetStartTime()  
{
	CMcuTime McuTime;
    CStructTm tmpTime(m_startTime);
	McuTime.LocalTime(tmpTime, ::GetpMcuTime()->GetGMTOffset(),
	                 (::GetpMcuTime()->GetGMTOffsetSign()));
	m_startTimeLocal=tmpTime;
	return &m_startTimeLocal; 
}


/////////////////////////////////////////////////////////////////////////////
void  CCommResDrv::SetEndTime(const CStructTm &other)
{
  CMcuTime McuTime;
  CStructTm tmpTime(other);
  McuTime.LocalTime(tmpTime, ::GetpMcuTime()->GetGMTOffset(),
	                 !(::GetpMcuTime()->GetGMTOffsetSign()));
  m_endTime=tmpTime;   

  DWORD diffTime;
  diffTime = m_endTime - m_startTime;    
  int hour = diffTime/3600;     
  int min = (diffTime%3600)/60;     
  int sec = (diffTime%3600)%60;
  CStructTm TempTime(0,0,0,hour,min,sec);
  m_duration = TempTime;
}

/////////////////////////////////////////////////////////////////////////////
const CStructTm*  CCommResDrv::GetEndTime()  
{
	CMcuTime McuTime;
    CStructTm tmpTime(m_endTime);
	McuTime.LocalTime(tmpTime, ::GetpMcuTime()->GetGMTOffset(),
	                 (::GetpMcuTime()->GetGMTOffsetSign()));

	m_endTimeLocal=tmpTime;
	return &m_endTimeLocal; 
}



/////////////////////////////////////////////////////////////////////////////        
void  CH221strCapDrv::Serialize(WORD format,CSegment  &Seg)
{                        
    WORD    i;
 
	switch ( format )  {    
	
	case SERIALEMBD :{  
            for ( i=0 ; i < m_size ; i++)
                Seg <<  m_pStr[i];
               break; 
					 } 
	case  NATIVE  :{

			Seg << m_size; 
            
            for ( i=0 ; i < m_size ; i++)
                Seg <<  m_pStr[i];
       
               break; 
				   }
               
	   default :
		   { break;	}                            
	
	}	
}

/////////////////////////////////////////////////////////////////////////////
void  CH221strCapDrv::DeSerialize(WORD format,CSegment  &Seg)
{
	//BYTE 	bas;
    WORD    i;
    BYTE dummy;
    CSegment copy_of_seg(Seg);//used to find length of segment

	                                       
	switch ( format )  {    
	
	case SERIALEMBD :{       		   

            if (m_pStr != NULL) PDELETE(m_pStr);
            m_pStr = NULL;
            m_size = 0;
            while ( ! copy_of_seg.EndOfSegment() )  {
                copy_of_seg >> dummy;
                m_size++;
            }

            if (m_size > 0) {
                m_pStr = new BYTE[m_size];
                PASSERT(m_pStr == NULL);
            
                for ( i=0 ; i < m_size ; i++)
                    Seg >> m_pStr[i];
            }

	       
			break;
					 }
	case NATIVE     :{
	             
			Seg >> m_size; 

            if (m_pStr != NULL) PDELETE(m_pStr);
            m_pStr = NULL;
            if (m_size > 0) {
                m_pStr = new BYTE[m_size];
                PASSERT(m_pStr == NULL);
            
                for ( i=0 ; i < m_size ; i++)
                    Seg >> m_pStr[i];
            }
			break; 
					 }
		default :
			{	break;	 }                           
	
	}	                                                                     
}
/////////////////////////////////////////////////////////////////////////////        
void  CH221strComDrv::Serialize(WORD format,CSegment  &Seg)
{                        
    WORD    i;
 
	switch ( format )  {    
	
	case SERIALEMBD :{  
            for ( i=0 ; i < m_size ; i++)
                Seg <<  m_pStr[i];
               break; 
					 }
	case  NATIVE  :{

			Seg << m_size; 
            
            for ( i=0 ; i < m_size ; i++)
                Seg <<  m_pStr[i];
       
               break; 
				   }
	   default :
		   { break;	}                            
	
	}	
}

/////////////////////////////////////////////////////////////////////////////
void  CH221strComDrv::DeSerialize(WORD format,CSegment  &Seg)
{
	//BYTE 	bas;
    WORD    i;
    BYTE dummy;
    CSegment copy_of_seg(Seg);

	                                       
	switch ( format )  {    
	
	case SERIALEMBD :{       		   

            if (m_pStr != NULL) PDELETE(m_pStr);
            m_pStr = NULL;
            m_size = 0;
            while ( ! copy_of_seg.EndOfSegment() )  {
                copy_of_seg >> dummy;
                m_size++;
            }

            if (m_size > 0) {
                m_pStr = new BYTE[m_size];
                PASSERT(m_pStr == NULL);
            
                for ( i=0 ; i < m_size ; i++)
                    Seg >> m_pStr[i];
            }

	       
			break;
					 }
	case NATIVE     :{
	             
			Seg >> m_size; 

            if (m_pStr != NULL) PDELETE(m_pStr);
            m_pStr = NULL;
            if (m_size > 0) {
                m_pStr = new BYTE[m_size];
                PASSERT(m_pStr == NULL);
            
                for ( i=0 ; i < m_size ; i++)
                    Seg >> m_pStr[i];
            }
			break; 
					 }
		default :
			{		break;	  }                          
	
	}	                                                                     
}



/////////////////////////////////////////////////////////////////////////////
void CVideoLayoutDrv::Serialize(WORD format,CSegment& seg)
{
  if (format != NATIVE) PASSERT(1);
  COstrStream*      pOstr;  
  ALLOCBUFFER(msg_info,SIZE_STREAM);
  pOstr= new COstrStream(msg_info,SIZE_STREAM);    
  CVideoLayout::Serialize(format, *pOstr, 0); 
  int b=pOstr->pcount();
  ALLOCBUFFER(msg,b+1);
  memset(msg,' ', b);
  memcpy(msg, msg_info,b); 
  msg[b]='\0';
  DEALLOCBUFFER(msg_info)
  POBJDELETE(pOstr);
  seg << msg;
  DEALLOCBUFFER(msg)
} 


/////////////////////////////////////////////////////////////////////////////
void CVideoLayoutDrv::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1); 
    char* p = seg.GetString(); 
    CIstrStream*      pIstr;  
    pIstr= new CIstrStream(p);    
    CVideoLayout::DeSerialize(format, *pIstr, 0); 
    POBJDELETE(pIstr);
    //Notice, the function seg.GetString() 
    //returns allocation to "new char[strLen];"
    PDELETEA(p)
} 
*/

/////////////////////////////////////////////////////////////////////////////
void CCdrShortDrv::Serialize(WORD format,CSegment& seg)
{
 	PASSERT(format != NATIVE);

 	COstrStream ostr;
  	CCdrShort::Serialize(format, ostr, 0);
  	seg << ostr.str();
} 


/////////////////////////////////////////////////////////////////////////////
void CCdrShortDrv::DeSerialize(WORD format,CSegment& seg)
{
    PASSERT(format != NATIVE);
     
    char* p = seg.GetString(); 
    CIstrStream istr(p);    
    CCdrShort::DeSerialize(format, istr,0);
    
    //Notice, the function seg.GetString() 
    //returns allocation to "new char[strLen];"
    PDELETEA(p)
} 

/////////////////////////////////////////////////////////////////////////////
void CCdrEventDrv::Serialize(WORD format,CSegment& seg)
{
/*
	BYTE fullformat=0;
  if (format != NATIVE) PASSERT(1);
  COstrStream*      pOstr;  
  ALLOCBUFFER(msg_info,SIZE_STREAM);
  pOstr= new COstrStream(msg_info,SIZE_STREAM);    
  CCdrEvent::Serialize(format, *pOstr, 0, fullformat); 
  int b=pOstr->pcount();
  ALLOCBUFFER(msg,b+1);
  memset(msg,' ', b);
  memcpy(msg, msg_info,b); 
  msg[b]='\0';
  DEALLOCBUFFER(msg_info)
  POBJDELETE(pOstr);
  seg << msg;
  DEALLOCBUFFER(msg)
 */

 	BYTE fullformat=0;
 	COstrStream ostr;
  	CCdrEvent::Serialize(format, ostr, 0, fullformat);
  	seg << ostr.str();
} 


/////////////////////////////////////////////////////////////////////////////
void CCdrEventDrv::DeSerialize(WORD format,CSegment& seg)
{
	PASSERT(format != NATIVE);
    
    char* p = seg.GetString(); 
    string str = p; 
    
    
    //1) Notice, the function seg.GetString() 
    //   returns allocation to "new char[strLen];"
    PDELETEA(p); 
    
    CIstrStream istr(str);  
    CCdrEvent::DeSerialize(format, istr, 0); 
} 

/*
/////////////////////////////////////////////////////////////////////////////
void CAtmAddrDrv::Serialize(WORD format,CSegment& seg)
{
	WORD i;
    switch ( format )  {    
	   case SERIALEMBD :
		   {
			char s[ATM_LIMIT_ADDRESS_DIGITS_LEN];
			memset(s,0,ATM_LIMIT_ADDRESS_DIGITS_LEN);	

		    for ( i=0 ; i < ATM_ADDRESS_LEN ; i++)
				sprintf(&s[i*3],"%02x.",m_uniaddr[i]);

			s[ATM_LIMIT_ADDRESS_DIGITS_LEN-1]='\0';

			seg.Put((BYTE *)s, ATM_LIMIT_ADDRESS_DIGITS_LEN);
		    break;
		   }
	   case NATIVE :
		   {
			    COstrStream*      pOstr;  
                ALLOCBUFFER(msg_info,SIZE_STREAM);
                pOstr= new COstrStream(msg_info,SIZE_STREAM);    
                CAtmAddr::Serialize(OPERATOR_MCMS, *pOstr); 
                int b=pOstr->pcount();
                ALLOCBUFFER(msg,b+1);
                memset(msg,' ', b);
                memcpy(msg, msg_info,b); 
                msg[b]='\0';
                DEALLOCBUFFER(msg_info)
                POBJDELETE(pOstr);
                seg << msg;
                DEALLOCBUFFER(msg)
 
                break;
		   }
	   default :
		   {

		    break;
		   }
	}
}			    

/////////////////////////////////////////////////////////////////////////////
void CAtmAddrDrv::DeSerialize(WORD format,CSegment& seg)
{
	WORD i;
    switch ( format )  {    
	   case SERIALEMBD :
		   {
			char s[ATM_LIMIT_ADDRESS_DIGITS_LEN];
			memset(s,0,ATM_LIMIT_ADDRESS_DIGITS_LEN);	

			seg.Get((BYTE *)s, ATM_LIMIT_ADDRESS_DIGITS_LEN);
				
			const WORD len = ATM_ADDRESS_LEN+sizeof(int)+4;
			ALLOCBUFFER(szTemp,len);
			memset(szTemp,0,len);
				
			for (i=0; i<ATM_ADDRESS_LEN; i++)
   			    if (sscanf(&s[i*3],"%02x", &szTemp[i])!=1)
				    break;

			for (i=0; i<ATM_ADDRESS_LEN; i++)
				m_uniaddr[i] = szTemp[i];

			DEALLOCBUFFER(szTemp)

		    break;
		   }
	   case NATIVE :
		   {
                char* p = seg.GetString(); 
                CIstrStream*      pIstr;  
                pIstr= new CIstrStream(p);    
                CAtmAddr::DeSerialize(OPERATOR_MCMS, *pIstr); 
                POBJDELETE(pIstr);
                //Notice, the function seg.GetString() 
                //returns allocation to "new char[strLen];"
                PDELETEA(p) 

                break;
		   }
	   default :
		   {

		    break;
		   }
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAtmAddrDrv::Trace(COstrStream& msg)
{
  char s[ATM_LIMIT_ADDRESS_DIGITS_LEN];
  memset(s,0,ATM_LIMIT_ADDRESS_DIGITS_LEN);
  for (WORD i=0 ; i < ATM_ADDRESS_LEN ; i++)
    	sprintf(&s[i*3],"%02x.",m_uniaddr[i]);
  s[ATM_LIMIT_ADDRESS_DIGITS_LEN-1]='\0';

  msg << s;
}

/////////////////////////////////////////////////////////////////////////////
void CSoftwareCPPartyMonitoringDrv::Serialize(WORD format,CSegment& seg)
{
  if (format != NATIVE) PASSERT(1);
  COstrStream*      pOstr;  
  ALLOCBUFFER(msg_info, SIZE_STREAM);
  pOstr= new COstrStream(msg_info,SIZE_STREAM);    
  CSoftwareCPPartyMonitoring::Serialize(format, *pOstr, 0); 
  int b=pOstr->pcount();
  ALLOCBUFFER(msg,b+1);
  memset(msg,' ', b);
  memcpy(msg, msg_info,b); 
  msg[b]='\0';
  DEALLOCBUFFER(msg_info)    
  POBJDELETE(pOstr);
  seg << msg;
  DEALLOCBUFFER(msg)
}

/////////////////////////////////////////////////////////////////////////////
void CSoftwareCPPartyMonitoringDrv::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1); 
    char* p = seg.GetString(); 
    CIstrStream*      pIstr;  
    pIstr= new CIstrStream(p);    
    CSoftwareCPPartyMonitoring::DeSerialize(format, *pIstr, 0); 
    POBJDELETE(pIstr);
    //Notice, the function seg.GetString() 
    //returns allocation to "new char[strLen];"
    PDELETEA(p)
}
*/
