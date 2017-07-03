// ApacheModule.cpp : Defines the entry point for the application.
//

#ifdef _CONSOLE
#include <stdio.h>
#else
#include <windows.h>
#endif

#include "ApacheModuleProcess.h"
#include "SystemFunctions.h"

#ifdef _CONSOLE
#ifdef _UNIT_TESTS
#include "cppunit/CompilerOutputter.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/ui/text/TestRunner.h"

CProcessBase* CreateNewProcess();

#endif //_UNIT_TESTS

int main(int argc, char* argv[])
{
	#ifdef _UNIT_TESTS

	// Get the top level suite from the registry
	CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
	
	// Adds the test to the list of test to run
	CppUnit::TextUi::TestRunner runner;
	runner.addTest( suite );
	
	// Change the default outputter to a compiler error format outputter
	runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(),
		std::cerr ) );
	// Run the tests.
	CProcessBase * test_process = CreateNewProcess();
	test_process->SetUp();
	SystemSleep(100,FALSE);
	bool wasSucessful = runner.run();
	SystemSleep(100,FALSE);
	test_process->TearDown();
	POBJDELETE(test_process);

	//bool wasSucessful = runner.run();
	
	// Return error code 1 if the one of test failed.
	return wasSucessful ? 0 : 1;	

	#endif //_UNIT_TESTS

	CApacheModuleProcess mainApacheModuleProcess;
	mainApacheModuleProcess.Run();
	return 0;

}


#else // not _CONSOLE 

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine, int       nCmdShow)
{
	CApacheModuleProcess mainApacheModuleProcess;
	mainApacheModuleProcess.Run();
	return 0;
}

#endif // _CONSOLE



