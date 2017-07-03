// FileTests.cpp

#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include "Semaphore.h"
#include "SemaphoreCheck.h"
#include "DataTypes.h"
#include <unistd.h>
#include <stdlib.h>
#include <string>

using namespace std;


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( CTestSemaphore);


void CTestSemaphore::setUp()
{
    /*  int res=0;
      res= mkdir(MCU_TMP_DIR+"/semaphore",0700);
    if (-1==res)
        perror("error creating Directory");
    res =open (MCU_TMP_DIR+"/semaphore/sem1",O_CREAT|O_RDWR|O_TRUNC,0644);
    if (-1==res)
        perror ("error creating file");
    res = open (MCU_TMP_DIR+"/semaphore/sem2",O_CREAT|O_RDWR|O_TRUNC, 0644);
    if (-1==res)
        perror ("error creating file");
     res = open (MCU_TMP_DIR+"/semaphore/sem3",O_CREAT|O_RDWR|O_TRUNC, 0644);
    if (-1==res)
        perror ("error creating file");
     res = open (MCU_TMP_DIR+"/semaphore/sem4",O_CREAT|O_RDWR|O_TRUNC, 0644);
    if (-1==res)
        perror ("error creating file");
     res = open (MCU_TMP_DIR+"/semaphore/sem5",O_CREAT|O_RDWR|O_TRUNC, 0644);
    if (-1==res)
        perror ("error creating file");
     res = open (MCU_TMP_DIR+"/semaphore/sem6",O_CREAT|O_RDWR|O_TRUNC, 0644);
    if (-1==res)
    perror ("error creating file");*/
    
}


void CTestSemaphore::tearDown()
{
    /*  int res =0;
    res =system("rm -Rf MCU_TMP_DIR/semaphore");
     if (-1==res)
     perror("error deleting the directory");*/
}

void CTestSemaphore::testCreateSemaphore()
{
    int sid =0;
    STATUS bRes = STATUS_FAIL;
    string sempath = "sem1";
    bRes = CreateSemaphore(&sid , sempath);
    CPPUNIT_ASSERT_EQUAL((int)bRes ,0);
}

void CTestSemaphore::testDestroySemaphore()
{
    int sid =0;
    STATUS bRes = STATUS_FAIL;
    bRes = RemoveSemaphore(sid);
    CPPUNIT_ASSERT_EQUAL ((int)bRes ,0);
}
