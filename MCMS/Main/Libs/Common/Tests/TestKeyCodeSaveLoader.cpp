#include "TestKeyCodeSaveLoader.h"
#include "SystemFunctions.h"
#include "KeyCodeSaveLoader.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestKeyCodeSaveLoader );

static std::string originalCfgFileName;

extern std::string g_keyCodeFileName;

void TestKeyCodeSaveLoader::setUp()
{
	originalCfgFileName = g_keyCodeFileName;
        std::string keycodeFileName = MCU_TMP_DIR+"/keycode.txt";
	g_keyCodeFileName=keycodeFileName;
	std::string cmd = "rm -f " + keycodeFileName;
	system(cmd.c_str());
}


void TestKeyCodeSaveLoader::tearDown()
{
	g_keyCodeFileName = originalCfgFileName;
}


void TestKeyCodeSaveLoader::testSaveLoadNormal()
{
      bool ret;
	KeyCodeSaveLoader keySaveLoader;
	string const keycodeRef = "X2EB-E531-27B0-00FF-0024";
	VERSION_S const verRef = { 1, 2, 3, 4 };
	ret = keySaveLoader.SaveKeyCode(keycodeRef, verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	string keycodeLoaded;
	VERSION_S verLoaded;
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	CPPUNIT_ASSERT_EQUAL(keycodeRef, keycodeLoaded);

	string const keycodeRef2 = "X3EB-E531-27B0-11FF-1124";
	ret = keySaveLoader.SaveKeyCode(keycodeRef2, verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	CPPUNIT_ASSERT_EQUAL(keycodeRef2, keycodeLoaded);

	VERSION_S const verRef2 = { 2, 2, 3, 4 };
	ret = keySaveLoader.SaveKeyCode(keycodeRef, verRef2);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef2);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	CPPUNIT_ASSERT_EQUAL(keycodeRef, keycodeLoaded);

	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	CPPUNIT_ASSERT_EQUAL(keycodeRef2, keycodeLoaded);

	VERSION_S const verRef3 = { 2, 2, 6, 5 };
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef3);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	CPPUNIT_ASSERT_EQUAL(keycodeRef, keycodeLoaded);
	
	VERSION_S const verRef4 = { 1, 2, 7, 5 };
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef4);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	CPPUNIT_ASSERT_EQUAL(keycodeRef2, keycodeLoaded);
	
	VERSION_S const verRef5 = { 3, 2, 3, 5 };
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef5);
	CPPUNIT_ASSERT_EQUAL(false, ret);

}

void TestKeyCodeSaveLoader::testReset()
{
	bool ret ;
	KeyCodeSaveLoader keySaveLoader;
	string const keycodeRef = "X2EB-E531-27B0-00FF-0024";
	VERSION_S const verRef = { 1, 2, 3, 4 };

// case 1
	ret = keySaveLoader.SaveKeyCode(keycodeRef, verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	string keycodeLoaded;
	VERSION_S verLoaded;
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	CPPUNIT_ASSERT_EQUAL(keycodeRef, keycodeLoaded);
	
	ret = keySaveLoader.ResetKeyCode(verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef);
	CPPUNIT_ASSERT_EQUAL(false, ret);

//case 2
	ret = keySaveLoader.SaveKeyCode(keycodeRef, verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);

	string const keycodeRef2 = "X5EB-E531-27B0-00FF-0624";
	VERSION_S const verRef2 = { 5, 4, 3, 4 };
	ret = keySaveLoader.SaveKeyCode(keycodeRef2, verRef2);
	CPPUNIT_ASSERT_EQUAL(true, ret);

	ret = keySaveLoader.ResetKeyCode(verRef2);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef2);
	CPPUNIT_ASSERT_EQUAL(false, ret);

	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef);
	CPPUNIT_ASSERT_EQUAL(false, ret);

	ret = keySaveLoader.ResetKeyCode(verRef);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef);
	CPPUNIT_ASSERT_EQUAL(false, ret);

	//case 100
	std::string cmd = "rm -f " + MCU_TMP_DIR+"/keycode.txt";
	system(cmd.c_str());
	
	VERSION_S verRef100 = { 2, 0, 0, 1 };
	int i=0;
	for(; i< 100; i++)
	{
		char key[64];
		sprintf(key, "%d", i);
		string keycode100 = key;
		ret = keySaveLoader.SaveKeyCode(keycode100, verRef100);
		CPPUNIT_ASSERT_EQUAL(true, ret);
		verRef100.ver_minor++;
	}

	VERSION_S verRef101 = { 2, 0, 0, 1 };
	i=0;
	for(; i< 100; i++)
	{
		char key[64];
		sprintf(key, "%d", i);
		string keycode100 = key;

		ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef101);
		CPPUNIT_ASSERT_EQUAL(true, ret);
		CPPUNIT_ASSERT_EQUAL(0, keycodeLoaded.compare(keycode100));
		
		verRef101.ver_minor++;
	}

	verRef101.ver_minor = 55;
	string s58 = "158";
	ret = keySaveLoader.SaveKeyCode(s58, verRef101);
	CPPUNIT_ASSERT_EQUAL(true, ret);

	i=0;
	verRef101.ver_minor = 0;
	for(; i< 100; i++)
	{
		char key[64];
		sprintf(key, "%d", i);
		string keycode100 = key;

		ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef101);
		CPPUNIT_ASSERT_EQUAL(true, ret);
		 if(i  == 55) CPPUNIT_ASSERT_EQUAL(0, keycodeLoaded.compare("158"));
		else CPPUNIT_ASSERT_EQUAL(0, keycodeLoaded.compare(keycode100));
		
		verRef101.ver_minor++;
	}

//case
	verRef101.ver_minor = 100;
	string s100 = "100";
	ret = keySaveLoader.SaveKeyCode(s100, verRef101);
	CPPUNIT_ASSERT_EQUAL(true, ret);

	i = 1;
	verRef101.ver_minor = 1;
	for(; i< 101; i++)
	{
		char key[64];
		sprintf(key, "%d", i);
		string keycode100 = key;

		ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef101);
		CPPUNIT_ASSERT_EQUAL(true, ret);
		 if(i  == 55) CPPUNIT_ASSERT_EQUAL(0, keycodeLoaded.compare("158"));
		else CPPUNIT_ASSERT_EQUAL(0, keycodeLoaded.compare(keycode100));
		
		verRef101.ver_minor++;
	}

	verRef101.ver_minor = 0;
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef101);
	CPPUNIT_ASSERT_EQUAL(false, ret);
	


	//case 3
	verRef101.ver_minor = 101;
	s100 = "101";
	ret = keySaveLoader.SaveKeyCode(s100, verRef101);
	CPPUNIT_ASSERT_EQUAL(true, ret);
	
	i = 2;
	verRef101.ver_minor = 2;
	for(; i< 102; i++)
	{
		char key[64];
		sprintf(key, "%d", i);
		string keycode100 = key;

		ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef101);
		CPPUNIT_ASSERT_EQUAL(true, ret);
		 if(i  == 55) CPPUNIT_ASSERT_EQUAL(0, keycodeLoaded.compare("158"));
		else CPPUNIT_ASSERT_EQUAL(0, keycodeLoaded.compare(keycode100));
		
		verRef101.ver_minor++;
	}

	verRef101.ver_minor = 0;
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef101);
	CPPUNIT_ASSERT_EQUAL(false, ret);

	verRef101.ver_minor = 1;
	ret = keySaveLoader.LoadKeyCode(keycodeLoaded, verRef101);
	CPPUNIT_ASSERT_EQUAL(false, ret);
}

