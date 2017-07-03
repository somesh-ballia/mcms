// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestUnifiedComMode.h"
#include "UnifiedComModeMock.h" 
#include "Trace.h"
#include "SystemFunctions.h"
#include "cppunit/extensions/HelperMacros.h"
#include "H264.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestUnifiedComMode);

 
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::setUp()
{

}
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::tearDown()
{

	
}
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::CheckEmptyUnifiedComMode()
{   
  	CUnifiedComModeMock* pUnifiedCommodeMock = new CUnifiedComModeMock;
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode:::CheckEmptyUnifiedComMode", pUnifiedCommodeMock->GetIPSCM() != NULL);
	POBJDELETE(pUnifiedCommodeMock);
}
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::TestCallRate()
{   
  	CUnifiedComMode* pUnifiedCommode = new CUnifiedComMode(0,Xfer_384);
    pUnifiedCommode->SetCallRate(Xfer_384);
	DWORD callRate = pUnifiedCommode->GetCallRate();
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestCallRate", callRate == 384000);
	POBJDELETE(pUnifiedCommode);
}
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::TestConfType()
{   
  	CUnifiedComMode* pUnifiedCommode = new CUnifiedComMode(0,0);
    pUnifiedCommode->SetConfType(CONTINUOUS_PRESENCE);
	WORD confType = pUnifiedCommode->GetConfType();
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestConfType", confType == kCp);
    pUnifiedCommode->SetConfType(VIDEO_SWITCH);
	confType = pUnifiedCommode->GetConfType();
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestConfType", confType == kVideoSwitch);
    pUnifiedCommode->SetConfType(VIDEO_SWITCH_FIXED);
	confType = pUnifiedCommode->GetConfType();
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestConfType", confType == kVSW_Fixed);

	POBJDELETE(pUnifiedCommode);
}
/////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::TestVideoFlags()
{   
  	CUnifiedComMode* pUnifiedCommode = new CUnifiedComMode(0,0);
	WORD isLocalAutoRes = AUTO;
    pUnifiedCommode->SetIsAutoVidRes(isLocalAutoRes);
	WORD isAutoRes = pUnifiedCommode->IsAutoVidRes();
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestConfType", isLocalAutoRes == isAutoRes);
	WORD isLocalVideoProtocol = AUTO;
    pUnifiedCommode->SetIsAutoVidProtocol(isLocalVideoProtocol);
	WORD isAutoProtocol = pUnifiedCommode->IsAutoVidProtocol();
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestConfType", isLocalVideoProtocol == isAutoProtocol);
	POBJDELETE(pUnifiedCommode);
}
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::TestAudioMode()
{   
	 SystemSleep(10,FALSE);
  	CUnifiedComMode* pUnifiedCommode = new CUnifiedComMode(0,0);
	WORD  lDirection = cmCapTransmit;
	pUnifiedCommode->SetAudModeAndAudBitRate(G728,lDirection);
	WORD lAudMode = pUnifiedCommode->GetAudMode(lDirection);
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestAudioMode", lAudMode == G728);
	POBJDELETE(pUnifiedCommode);
}
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::TestFECCMode()
{   
  	CUnifiedComMode* pUnifiedCommode = new CUnifiedComMode(0,0);
	WORD lDirection = cmCapReceive;
    pUnifiedCommode->SetFECCMode(LSD_6400,lDirection);	
	WORD lFeccMode = pUnifiedCommode->GetFECCMode(lDirection);
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestFECCMode", lFeccMode == LSD_6400);
	POBJDELETE(pUnifiedCommode);
}
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::TestH264VideoMode()
{   
	SystemSleep(10);
  	CUnifiedComMode* pUnifiedCommode = new CUnifiedComMode(0,0);
	BYTE  lH264Level = H264_Level_1_3;
	WORD  lH264Profile = H264_Profile_BaseLine;
	APIS32 lMaxBR = 0;
	APIS32 lMaxMBPS = 0;
	APIS32 lMaxDPB = 0;
	APIS32	 lMaxFS = 10000;
	APIS32 lMaxSar = 255;
	APIS32 lMaxStaticMB = 143;
	
	WORD  lDirection = cmCapTransmit;
	WORD  isAutoSCM = 0;

	BYTE  rH264Level = 0;
	WORD  rH264Profile = 0;
	APIS32 rMaxBR = 0;
	APIS32 rMaxMBPS = 0;
	APIS32 rMaxFS = 0;
	APIS32 rMaxDPB = 0;
	APIS32 rMaxSAR = 0;
	APIS32 rMaxStaticMB = 0;	
    pUnifiedCommode->SetH264VidMode(lH264Profile, lH264Level,lMaxBR,lMaxMBPS,lMaxFS,lMaxDPB,lMaxSar,lMaxStaticMB,AUTO,lDirection);
    pUnifiedCommode->GetH264VidMode(rH264Profile, rH264Level, rMaxBR, rMaxMBPS,
	rMaxFS, rMaxDPB , rMaxSAR, rMaxStaticMB, isAutoSCM, lDirection);
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestH264VideoMode", ((rH264Profile == lH264Profile) && (rH264Level == lH264Level) && (rMaxBR == lMaxBR) && (rMaxFS == lMaxFS) && (rMaxDPB == lMaxDPB) && (rMaxMBPS == lMaxMBPS))) ;
	POBJDELETE(pUnifiedCommode);
}
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::TestH263VideoMode()
{   
  	CUnifiedComMode* pUnifiedCommode = new CUnifiedComMode(0,0);
	WORD lProtocol = H263;//eH263CapCode;
	char lQcifMpi = 1;
	char lCifMpi = 2;
	char lCif4Mpi = 0;
	char lCif16Mpi = 3;
	WORD lDirection = cmCapTransmit;

	pUnifiedCommode->SetProtocolClassicVidMode(lProtocol, lQcifMpi, lCifMpi, lCif4Mpi, lCif16Mpi,AUTO, lDirection);	
	
	WORD rProtocol = H263;//eH263CapCode;
	int rQcifMpi = 0;
	int rCifMpi = 0;
	int rCif4Mpi = 0;
	int rCif16Mpi = 0;
	WORD rDirection = cmCapTransmit;
	WORD  IsAutoVidScm = 0;

	pUnifiedCommode->GetProtocolClassicVidMode(rProtocol, rQcifMpi, rCifMpi, rCif4Mpi, rCif16Mpi,IsAutoVidScm, rDirection);
    
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestH263VideoMode", ((rQcifMpi == lQcifMpi)
	&& (rCifMpi == lCifMpi) && (rCif4Mpi == lCif4Mpi) && (rCif16Mpi == lCif16Mpi)));
	POBJDELETE(pUnifiedCommode);
}
//////////////////////////////////////////////////////////////////////
void CTestUnifiedComMode::TestH261VideoMode()
{   
  	CUnifiedComMode* pUnifiedCommode = new CUnifiedComMode(0,0);
	WORD lProtocol = H261;// eH261CapCode;
	char lQcifMpi = 1;
	char lCifMpi = 2;
	char lCif4Mpi = 0;
	char lCif16Mpi = 0;
	WORD lDirection = cmCapTransmit;

	pUnifiedCommode->SetProtocolClassicVidMode(lProtocol, lQcifMpi, lCifMpi, lCif4Mpi, lCif16Mpi, AUTO,lDirection);	
	
	WORD rProtocol = H261;//eH261CapCode;
	int rQcifMpi = 0;
	int rCifMpi = 0;
	int rCif4Mpi = 0;
	int rCif16Mpi = 0;
	WORD rDirection = cmCapTransmit;
	WORD  IsAutoVidScm = 0;

	pUnifiedCommode->GetProtocolClassicVidMode(rProtocol, rQcifMpi, rCifMpi, rCif4Mpi, rCif16Mpi,IsAutoVidScm, rDirection);
    
	CPPUNIT_ASSERT_MESSAGE("CTestUnifiedComMode::TestH261VideoMode", (rQcifMpi == lQcifMpi) && (rCifMpi == lCifMpi));
	POBJDELETE(pUnifiedCommode);
}



