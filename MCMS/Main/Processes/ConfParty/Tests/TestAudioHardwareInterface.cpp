
#include "TestAudioHardwareInterface.h"
#include "AudioHardwareInterface.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestAudioHardwareInterface );

void CTestAudioHardwareInterface::setUp()
{
	m_audioInterface = 0;
}

void CTestAudioHardwareInterface::tearDown()
{
	POBJDELETE(m_audioInterface);
}
 
void CTestAudioHardwareInterface::TestSendOpenPort()
{
	//ConnectionId,ParId,ConfId,LRT
	m_audioInterface = new CAudioHardwareInterface(1,2,3,eLogical_audio_encoder);
//	m_audioInterface->SendOpenPort();
}

