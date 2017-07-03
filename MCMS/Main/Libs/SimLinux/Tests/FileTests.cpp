// FileTests.cpp

/*
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include "FileTests.h"
#include "DataTypes.h"
#include "OsFileIF.h"
#include <unistd.h>
#include <stdlib.h>
#include <vector>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( CTestFiles);

using namespace std;

void CTestFiles::setUp()
{
    int res=0;
    res= mkdir(MCU_TMP_DIR+"/Filetests",0700);
    if (-1==res)
        perror("error creating Directory");
    res =open (MCU_TMP_DIR+"/Filetests/dummytext.test",O_CREAT|O_RDWR|O_TRUNC,0644);
    if (-1==res)
        perror ("error creating file");
    res = open (MCU_TMP_DIR+"/Filetests/dummytext1.test",O_CREAT|O_RDWR|O_TRUNC, 0644);
    if (-1==res)
        perror ("error creating file");
    res =mkdir (MCU_TMP_DIR+"/Filetests/george" ,0700);
    if (-1==res)
        perror("error creating directory george");
    
    
}


void CTestFiles::tearDown()
{
    int res =0;
    std::string cmd = "rm -Rf "+MCU_TMP_DIR+"/Filetests";
    res =system(cmd.c_str());
    if (-1==res)
        perror("error deleting the directory");
}

void CTestFiles::testGetDirectoryContents()
{
    vector <FDStruct> temp;
    bool bRes;
    bRes = GetDirectoryContents(MCU_TMP_DIR+"/Filetests/" ,temp);
    CPPUNIT_ASSERT_EQUAL(temp.size(),(size_t)3);
}

void CTestFiles::testGetFirstFile()
{
    vector <FDStruct> temp;
    vector <FDStruct>::iterator temp_iterator;
    bool bRes;
    bRes = GetDirectoryContents(MCU_TMP_DIR+"/Filetests/" ,temp);
    FDStruct * temp_struct = GetFirstFile( temp_iterator,temp);
    CPPUNIT_ASSERT_EQUAL(0 ,strcmp(temp_struct->name.c_str(),"dummytext1.test"));
}
void CTestFiles::testCreateDirectoryExist()
{
    bool bRes =CreateDirectory(MCU_TMP_DIR+"/Filetests");
    CPPUNIT_ASSERT_EQUAL(true ,bRes); 
}
void CTestFiles::testDirStream()
{
    DIR  * dirstream;
    bool bRes;
    bRes = GetDirStreamByPath(MCU_TMP_DIR+"/Filetests/",dirstream);
	CPPUNIT_ASSERT_EQUAL(bRes,true);
}

void CTestFiles::testDirStreamNull()
{
    DIR * dirstream;
    bool bRes;
    bRes = GetDirStreamByPath("",dirstream);
    CPPUNIT_ASSERT_EQUAL(bRes , false);
}

void CTestFiles::testDirStreamFile()
{
    DIR * dirstream;
    bool bRes;
    bRes = GetDirStreamByPath(MCU_TMP_DIR+"/Filetests/dummytext.test",dirstream);
    CPPUNIT_ASSERT_EQUAL(bRes ,false);
}

void CTestFiles::testGetFDByPathFile()
{
    int res =0;
    res = GetFDByPath(MCU_TMP_DIR+"/Filetests/dummytext.test");
    CPPUNIT_ASSERT_ASSERTION_FAIL(CPPUNIT_ASSERT_EQUAL(res , 0));
}

void CTestFiles::testGetFDByPathDIR()
{
    int res =0;
    res = GetFDByPath(MCU_TMP_DIR+"/Filetests//");
    CPPUNIT_ASSERT_ASSERTION_FAIL(CPPUNIT_ASSERT_EQUAL(res , 0));
}

void CTestFiles::testGetFDByPathNULL()
{
    int res =0;
    res = GetFDByPath("");
    CPPUNIT_ASSERT_EQUAL(res , -1);
}

void CTestFiles::testDeleteFile()
{
    int res =0;
    res = DeleteFile(MCU_TMP_DIR+"/Filetests/dummytext.test");
    CPPUNIT_ASSERT_EQUAL(res , -1);
}

void CTestFiles::testGetLastModified()
{
    int res =0;
    time_t lm  = GetLastModified(MCU_TMP_DIR+"/Filetests/dummytext.test");
    CPPUNIT_ASSERT_NEQUAL(lm , -1);
}
*/


