
#if !defined(TEST_SEMAPHORE_H)
#define TEST_SEMAPHORE_H


#include <cppunit/extensions/HelperMacros.h>


class  CTestSemaphore   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestSemaphore);
	//...
    //CPPUNIT_TEST(testCreateSemaphore);
    //CPPUNIT_TEST(testDestroySemaphore);
    /* CPPUNIT_TEST(testGetFirstFile);
    CPPUNIT_TEST(testCreateDirectoryExist);
    CPPUNIT_TEST( testDirStream );
    CPPUNIT_TEST(testDirStreamNull);
    CPPUNIT_TEST(testDirStreamFile);
    CPPUNIT_TEST(testGetFDByPathFile);
    CPPUNIT_TEST(testGetFDByPathDIR);
    CPPUNIT_TEST(testGetFDByPathNULL);
*/  
    
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	void testCreateSemaphore();
    void testDestroySemaphore();
    void testLockSemaphore();
    void testUnlockSemaphore();
/*	void testConstructor();
    void testDirStream();    
    void testDirStreamNull();    
    void testDirStreamFile();    
    void testGetFDByPathFile();    
    void testGetFDByPathDIR();    
    void testGetFDByPathNULL();    
    void testGetAllFilesinDir();
    void testGetDirectoryContents();
    void testGetFirstFile();
    void testCreateDirectoryExist();
*/

};

#endif //  !defined(TEST_FileTests_H)
