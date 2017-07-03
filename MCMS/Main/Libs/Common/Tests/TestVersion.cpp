

#include "TestVersion.h"
#include "SystemFunctions.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestVersion );

std::string StripVersionFromFileName(const std::string &fileName);


void TestVersion::setUp()
{
}


void TestVersion::tearDown()
{
}


void TestVersion::testFileNameStrip()
{
    std::string filename1 = "xxx.yyy.zzz_skdhfsdkj.bin";
    std::string version1 = StripVersionFromFileName(filename1);
    CPPUNIT_ASSERT_EQUAL( std::string("xxx.yyy.zzz_skdhfsdkj") , version1 );

    std::string filename2 = "kjghas_sdfgsdf_Gsdfg.fgsdfg_dfg.bin.factory";
    std::string version2 = StripVersionFromFileName(filename2);
    CPPUNIT_ASSERT_EQUAL( std::string("kjghas_sdfgsdf_Gsdfg.fgsdfg_dfg") , version2 );


    std::string filename3 = "kjghas_sdfgsdf_Gsdfg.fgsdfg_dfg";
    std::string version3 = StripVersionFromFileName(filename3);
    CPPUNIT_ASSERT_EQUAL( std::string("kjghas_sdfgsdf_Gsdfg.fgsdfg_dfg") , version3 );


    std::string filename4 = "kjghas_sdfgsdf_Gsdfg.fgsdfg_dfg\n";
    std::string version4 = StripVersionFromFileName(filename4);
    CPPUNIT_ASSERT_EQUAL( std::string("kjghas_sdfgsdf_Gsdfg.fgsdfg_dfg") , version4 );
    
}

