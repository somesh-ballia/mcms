
#if !defined(TEST_IpServicesList_H)
#define TEST_IpServicesList_H


#include "cppunit/extensions/HelperMacros.h"
#include "IpServiceListManager.h"
#include "ConfPartyProcess.h"    


class CTestIpServicesList   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestIpServicesList );
	CPPUNIT_TEST( CheckEmptyListOfIpServices );
	CPPUNIT_TEST(InsertAndDeleteOneServiceToList);
	CPPUNIT_TEST(InsertAndDeleteThreeServicesToList);
	CPPUNIT_TEST(FindServiceByName);

	//...
	CPPUNIT_TEST_SUITE_END();
public:
	
    CTestIpServicesList(){;}
	void setUp();
	void tearDown();
	void CheckEmptyListOfIpServices();
	void InsertAndDeleteOneServiceToList();
	void InsertAndDeleteThreeServicesToList();
	void FindServiceByName();


private:
	CIpServiceListManager* m_pIpServiceListManager ;

};

#endif // !defined(TEST_IpServicesList_H)

