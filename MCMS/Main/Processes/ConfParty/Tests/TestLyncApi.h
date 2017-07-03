#if !defined(TEST_MCCFMNGR_H)
#define TEST_MCCFMNGR_H

#include <cppunit/extensions/HelperMacros.h>
#include "SystemFunctions.h"

class CTestLyncApi : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestLyncApi);
	CPPUNIT_TEST(testEventManager);
	CPPUNIT_TEST(testXmlLoadSample);
	CPPUNIT_TEST(testXmlMergeSample);
	CPPUNIT_TEST_SUITE_END();

public:

	void setUp() { }
	void tearDown() { }
	
	void testEventManager();
	void testXmlLoadSample();
	void testXmlMergeSample();

private:
	size_t ReadXmlFile(char* xmlFile, char** xmlBuffer);
	void   DumpXmlFile(char* xmlFile, int xmlType);
	void   MergeXmlFile(char* xmlFile1, char* xmlFile2, DWORD partyId);
};

#endif
