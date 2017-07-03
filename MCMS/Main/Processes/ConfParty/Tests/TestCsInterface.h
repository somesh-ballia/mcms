// TEST_CONFPARTYROUTINGTABLE.h: interface for the CTestConfPartyRsrcTable class.
// Unit tests using TDD of the ConfParty Process
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_CTESTCSINTERFACE_H)
#define TEST_CTESTCSINTERFACE_H

#include <cppunit/extensions/HelperMacros.h>

#include "CsInterface.h"
#include "MockCMplMcmsProtocol.h"
#include "ConfPartyProcess.h"



class CTestCsInterface   : public CPPUNIT_NS::TestFixture
{
//	CPPUNIT_TEST_SUITE( CTestCsInterface );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testUpdateResourceParam );
// 	CPPUNIT_TEST( testGetRsrcParams );
// 	CPPUNIT_TEST( testGetRsrcParams1 );
//	CPPUNIT_TEST( testGetPartyRsrcId );
// 	CPPUNIT_TEST( testGetConfRsrcId );
// 	CPPUNIT_TEST( testSetPartyRsrcId );
// 	CPPUNIT_TEST( testSetConfRsrcId );
// 	CPPUNIT_TEST( testGetMplMcmsProtocol );
// 	CPPUNIT_TEST( testMplApiWasAddCommonHeaderCalled );
// 	CPPUNIT_TEST( testSendMsgToCSApiCommandDispatcher );
// 	CPPUNIT_TEST( testAddMessageDescriptionHeaderCalled );
// 	CPPUNIT_TEST( testAddPortDescriptionHeader );
// 	CPPUNIT_TEST( testAddCSHeader );
// 	CPPUNIT_TEST( testAddData );
// 	CPPUNIT_TEST( testTraceMplMcmsProtocol );

	//...
//	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testUpdateResourceParam();
	void testGetRsrcParams();
	void testGetRsrcParams1();
	void testGetPartyRsrcId();
	void testGetConfRsrcId();
	void testSetPartyRsrcId();
	void testSetConfRsrcId();
	void testGetMplMcmsProtocol();

	// Mock
	void testMplApiWasAddCommonHeaderCalled();
	void testSendMsgToCSApiCommandDispatcher();
	void testAddMessageDescriptionHeaderCalled();
	void testAddPortDescriptionHeader();
	void testAddCSHeader();
	void testAddData();
	void testTraceMplMcmsProtocol();

	CCsInterface *		m_pCsInterface;
	CRsrcParams*		m_pDesc;
	CMockMplMcmsProtocol* m_MplMock;



};

#endif // !defined(TEST_CTESTCSINTERFACE_H)

