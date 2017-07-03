#include "TestFileNameParser.h"
#include <string>
#include "CyclicFileList.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestFileNameParser );



 


void TestFileNameParser::testParser()
{
    FileNameHeaders_S fnh;
    const char * name;
    DWORD fileSequenceNumber;
    DWORD fileSize;
    CStructTm firstMessageTime;
    CStructTm lastMessageTime;
    BOOL containsStartup;
    BOOL isRetrived;
    BYTE cmprFormat;
    int nameFormatVersion;
    
    
    name = "LogFiles/Log_SN0000000001_FMD16112008_FMT080525_LMD16112008_LMT080629_SZ126747_SUY_CFzlib_NFV01.log";
    
    CCyclicFileList::ParseFileName(name,
                                   fnh,
                                   fileSequenceNumber,
                                   fileSize,
                                   firstMessageTime,
                                   lastMessageTime,
                                   containsStartup,
                                   isRetrived,
                                   cmprFormat,
                                   nameFormatVersion);
    
    CPPUNIT_ASSERT(fileSequenceNumber == 1);
    CPPUNIT_ASSERT(fileSize == 126747);
    CPPUNIT_ASSERT(containsStartup != false);
    CPPUNIT_ASSERT(isRetrived != false);
    CPPUNIT_ASSERT(nameFormatVersion == 1);

    name = "LogFiles/Log_SN0000000002_FMD16112008_FMT080629_LMD16112008_LMT090534_SZ1026648_SUN_CFzlib_NFV01.log";

    CCyclicFileList::ParseFileName(name,
                                   fnh,
                                   fileSequenceNumber,
                                   fileSize,
                                   firstMessageTime,
                                   lastMessageTime,
                                   containsStartup,
                                   isRetrived,
                                   cmprFormat,
                                   nameFormatVersion);
    
    CPPUNIT_ASSERT(fileSequenceNumber == 2);
    CPPUNIT_ASSERT(fileSize == 1026648);
    CPPUNIT_ASSERT(containsStartup == false);
    CPPUNIT_ASSERT(isRetrived != false);
    CPPUNIT_ASSERT(nameFormatVersion == 1);

    name = "LogFiles/Log_SN0000000029_FMD17112008_FMT140540_LMD17112008_LMT151016_SZ1042550_SUN_CFzlib_NFV02_RTN.log";

    CCyclicFileList::ParseFileName(name,
                                   fnh,
                                   fileSequenceNumber,
                                   fileSize,
                                   firstMessageTime,
                                   lastMessageTime,
                                   containsStartup,
                                   isRetrived,
                                   cmprFormat,
                                   nameFormatVersion);
    
    CPPUNIT_ASSERT(fileSequenceNumber == 29);
    CPPUNIT_ASSERT(fileSize == 1042550);
    CPPUNIT_ASSERT(containsStartup == false);
    CPPUNIT_ASSERT(isRetrived == false);
    CPPUNIT_ASSERT(nameFormatVersion == 2);


    name = "LogFiles/Log_SN0000000029_FMD17112008_FMT140540_LMD17112008_LMT151016_SZ1042550_SUN_CFzlib_NFV02_RTY.log";

    
    CCyclicFileList::ParseFileName(name,
                                   fnh,
                                   fileSequenceNumber,
                                   fileSize,
                                   firstMessageTime,
                                   lastMessageTime,
                                   containsStartup,
                                   isRetrived,
                                   cmprFormat,
                                   nameFormatVersion);
    
    CPPUNIT_ASSERT(fileSequenceNumber == 29);
    CPPUNIT_ASSERT(fileSize == 1042550);
    CPPUNIT_ASSERT(containsStartup == false);
    CPPUNIT_ASSERT(isRetrived != false);
    CPPUNIT_ASSERT(nameFormatVersion == 2);
    
    
    return;
}
