
          
#include "TestNetChanCon.h"
#include "NetChannConn.h"
#include "Native.h"
#include "CallPart.h"
#include "NStream.h"
#include "Segment.h"
#include "SystemFunctions.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestNetChanConn );


void TestNetChanConn::setUp()
{
}

void TestNetChanConn::tearDown()
{
}

void TestNetChanConn::testConstructor()
{

}


       
                  
                 
         
   			
  
    	
   
     
               

void TestNetChanConn::testSerializeDeserialize()
{
    CNetChanlCon *pNetChanlCon = new CNetChanlCon;

    pNetChanlCon->SetPartyName("cucu-lulu");                 
    pNetChanlCon->SetPartyId(0);
    pNetChanlCon-> SetChanlNum(0);
    
    
    pNetChanlCon->SetChanlId(0);
    pNetChanlCon->SetConctIniator(0);
    pNetChanlCon->SetCallType(0);
    pNetChanlCon->SetNetSpec(0);
    
    CCallingParty *pCallingParty = new CCallingParty;
    pCallingParty->SetCallingNumType(0);                 
    pCallingParty->SetCallingNumPlan(0);                 
    pCallingParty->SetCallingPresentInd(0);
    pCallingParty->SetCallingScreenInd(0);
    pCallingParty->SetCallingPhoneNum("\0");
//    pCallingParty->SetCallingPhoneNum("0547781666");
  

    CCalledParty *pCalledParty = new CCalledParty;
    pCalledParty->SetCalledNumType(0);                 
    pCalledParty->SetCalledNumPlan(1);
    pCalledParty->SetCalledPhoneNum("0544561689");
    
    pNetChanlCon->SetCallingParty(*pCallingParty);
    pNetChanlCon->SetCalledParty(*pCalledParty);

    CStructTm curTime;
    SystemGetTime(curTime);
  
    CCdrEventDrv cdrEventSer;
    cdrEventSer.SetCdrEventType(NET_CHANNEL_CONNECTED);
    cdrEventSer.SetTimeStamp(curTime);
    cdrEventSer.SetNetChanlConnect(pNetChanlCon);

    CSegment seg;
    cdrEventSer.Serialize(NATIVE, seg);
   
    CCdrEventDrv cdrEventDes;
    cdrEventDes.DeSerialize(NATIVE ,seg);
   

    CNetChanlCon *pNetChanConnSer = cdrEventSer.GetNetChanlConnect() ;
    CNetChanlCon *pNetChanConnDes = cdrEventDes.GetNetChanlConnect() ;
    
    bool res;
    if((pNetChanConnSer == NULL) || (pNetChanConnDes == NULL) )
    	res = FALSE;
    else
    	res = (*pNetChanConnSer == *pNetChanConnDes);

    CPPUNIT_ASSERT(res);
}

void TestNetChanConn::testSerializeDeserializeCalling()
{
    CCallingParty *pCallingParty = new CCallingParty;
    pCallingParty->SetCallingNumType(0);                 
    pCallingParty->SetCallingNumPlan(0);                 
    pCallingParty->SetCallingPresentInd(0);
    pCallingParty->SetCallingScreenInd(0);
    pCallingParty->SetCallingPhoneNum("\0");

    CCalledParty *pCalledParty = new CCalledParty;
    pCalledParty->SetCalledNumType(0);                 
    pCalledParty->SetCalledNumPlan(1);
    pCalledParty->SetCalledPhoneNum("0544561689");
    
    COstrStream ostr;
    pCallingParty->Serialize(NATIVE, ostr);
    const char *ptr1 = ostr.str().c_str();
    
    pCalledParty->Serialize(NATIVE, ostr);
    const char *ptr2 = ostr.str().c_str();
    
    
    CCallingParty *pCallingPartyDes = new CCallingParty;
    CCalledParty *pCalledPartyDes = new CCalledParty;
    
    CIstrStream istr(ostr.str());  
    pCallingPartyDes->DeSerialize(NATIVE, istr);
    pCalledPartyDes->DeSerialize(NATIVE, istr);
    
    bool res1 = (*pCallingPartyDes == *pCallingParty);
    bool res2 = (*pCalledPartyDes == *pCalledParty);
    
    CPPUNIT_ASSERT(res1);
    CPPUNIT_ASSERT(res2);
    
}


