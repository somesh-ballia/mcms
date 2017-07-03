#if !defined(TEST_TestClient_H)
#define TEST_TestClient_H

#include <cppunit/extensions/HelperMacros.h>


class CTestTestClientProcess   : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( CTestTestClientProcess );
    CPPUNIT_TEST( testTRACEINTO );
    CPPUNIT_TEST( testConstructor );
    CPPUNIT_TEST( testSendMessage );
    CPPUNIT_TEST( testGetManagerQueue );
    CPPUNIT_TEST( testGetDispacherQueue );
    //CPPUNIT_TEST( testKillDispacher );
    //CPPUNIT_TEST( testSendSyncMessages );
    
    //CPPUNIT_TEST( testSendLocalMessages );
    //CPPUNIT_TEST( test1000IPCMessage );
    CPPUNIT_TEST( testAddTimer );
    CPPUNIT_TEST( testRemoveTimer );
    CPPUNIT_TEST( testCreateSingleToneTask );
    //CPPUNIT_TEST( testPTRACE );
    //CPPUNIT_TEST( testPASSERT );
    //CPPUNIT_TEST( testDividByZero );
    
    //...
    CPPUNIT_TEST( testSendStateMachineMessage );
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp();
    void tearDown(); 
    
    void testConstructor();
    void testSendMessage();
    void testGetManagerQueue();
    void testGetDispacherQueue();	
    void testSendSyncMessages();
    void testIPCSendMessage();
    void testSendLocalMessages();
    void test1000IPCMessage();
    void testAddTimer();
    void testRemoveTimer();
    void testDividByZero();
    void testPTRACE();
    void testPASSERT();
    //...
    void testTerminateServer();
    
    void testSendStateMachineMessage();
    
    void testCreateSingleToneTask();
    void testTRACEINTO();  
};

#endif // !defined(TEST_TestClient_H)
