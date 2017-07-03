
#include <iostream>
using namespace std;



#include "TestTraceStatistics.h"
#include "TraceStatistics.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestTraceStat );


void CTestTraceStat::testKBSize()
{   
    CKBSize sizeBytes;
    sizeBytes.AddSize(1024);
    CLargeString bytes;
    sizeBytes.ToString(bytes);

    cout << "bytes: " << bytes.GetString() << endl;
    
    int dif = strcmp("1024 B", bytes.GetString());
    CPPUNIT_ASSERT_MESSAGE("Byte's range", 0 == dif);
    

    
    CKBSize sizeKilo;
    sizeKilo.AddSize(1024 * 8 + 1);
    CLargeString kilo;
    sizeKilo.ToString(kilo);

    cout << "kilos: " << kilo.GetString() << endl;

    dif = strcmp("8.1 KB", kilo.GetString());
    CPPUNIT_ASSERT_MESSAGE("Kilo's range", 0 == dif);

    
    CKBSize sizeMega;
    sizeMega.AddSize(1024 * 1024 * 8 + 1);
    CLargeString mega;
    sizeMega.ToString(mega);

    cout << "megas: " << mega.GetString() << endl;
    
    dif = strcmp("8.1 MB", mega.GetString());
    CPPUNIT_ASSERT_MESSAGE("Mega's range", 0 == dif);
    
    
    ULONGLONG gigaRange = 1024 * 1024;
    gigaRange *= 1024 * 8;
    gigaRange += 1;
    
    CKBSize sizeGiga;
    sizeGiga.AddSize(gigaRange);
    CLargeString giga;
    sizeGiga.ToString(giga);

    cout << "gigas: " << giga.GetString() << endl;

    dif = strcmp("8.1 GB", giga.GetString());
    CPPUNIT_ASSERT_MESSAGE("Giga's range", 0 == dif);
}



