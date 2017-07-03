#include "TestConfStartCont1.h"
#include "NStream.h"
#include "IVRAvMsgStruct.h"
#include "ConfStart.h"



CPPUNIT_TEST_SUITE_REGISTRATION( CTestConfStartCont1 );



CTestConfStartCont1::CTestConfStartCont1()
{
}

CTestConfStartCont1::~CTestConfStartCont1()
{
}


void CTestConfStartCont1::setUp()
{
}

void CTestConfStartCont1::tearDown()
{
}

void CTestConfStartCont1::testSerializeDeSerialize()
{
	CConfStartCont1 confStartCont1;
	  confStartCont1.SetAudioTone (1);
//		  confStartCont1.SetAlertToneTiming (GetAlertToneTiming ());
	  confStartCont1.SetTalkHoldTime (2);
	  confStartCont1.SetAudioMixDepth (3);
//		  confStartCont1.SetOperatorConf(GetOperatorConf ());
	  confStartCont1.SetVideoProtocol (4);
	  confStartCont1.SetMeetMePerConf (5);
	  
//		   confStartCont1.SetNumServicePhone(GetNumServicePhone());
//		  int tempNumServicePhone=GetNumServicePhone();
	
	  /////////////////////////////////////////////////   
/*		 
//		 if (tempNumServicePhone>0)
//		 confStartCont1.AddServicePhone(*GetFirstServicePhone());
		
//		 for (int b=1;b<tempNumServicePhone;b++)
//		 confStartCont1.AddServicePhone(*GetNextServicePhone());
		    
*/		
	
	  confStartCont1.SetConf_password ("cucu_lulu1");
//		  confStartCont1.SetChairMode (GetChairMode ());
	  confStartCont1.SetCascadeMode (6);
	//  confStartCont1.SetMasterName (GetMasterName () );
	  // Romem - should be re-defined
     //confStartCont1.SetUnlimited_conf_flag(/*GetUnlimitedReservFlag ()*/ YES);
     //const BYTE limited_flag = confStartCont1.GetUnlimited_conf_flag();
	  confStartCont1.SetNumUndefParties (7);
	  confStartCont1.SetTime_beforeFirstJoin(8);
	  confStartCont1.SetTime_afterLastQuit(9);
	  confStartCont1.SetConfLockFlag(10);
	  confStartCont1.SetMax_parties(11);
//		  confStartCont1.SetCardRsrsStruct(*GetpCardRsrsStruct())  ;

	CAvMsgStruct IVRstruct1;
	
	IVRstruct1.SetAttendedWelcome(3);
	IVRstruct1.SetAvMsgServiceName("RNX IVR Service");
	
	  confStartCont1.SetAvMsgStruct(IVRstruct1);
	  
	CLectureModeParams lecturerMode;
	lecturerMode.SetLectureModeType(12);
	lecturerMode.SetLecturerName("cucu_lulu2");
	lecturerMode.SetLectureTimeInterval(13);
	lecturerMode.SetTimerOnOff(14);
	lecturerMode.SetAudioActivated(15);
	lecturerMode.SetLecturerId(16);
	confStartCont1.SetLectureMode(lecturerMode);

	COstrStream ostr;
    confStartCont1.Serialize(0, ostr, 0);
    
    const string tmpStr = ostr.str();
    
    CIstrStream istr(ostr.str());
	CConfStartCont1 confStartCont2;
	confStartCont2.DeSerialize(0, istr, 0);

	bool res = (confStartCont1 == confStartCont2);
    CPPUNIT_ASSERT(res);
}

