
#include <iostream>
#include <stdio.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

#include "ProcessBase.h"
#include "SystemFunctions.h"
CProcessBase* CreateNewProcess();


// Recursive dumps the given Test hierarchy to std::cout
/////////////////////////////////////////////////////////////////////
CppUnit::Test * find(CppUnit::Test *test, const char *name)
{
  if (0 == test) return NULL;

  if (test->getName()== name)
  	  return test;

  if (test->getChildTestCount() == 0)
	  return NULL;

  for (int i = 0; i < test->getChildTestCount(); i++)
  {
	  CppUnit::Test * tmp = find(test->getChildTestAt(i),name);
	  if (tmp)
		  return tmp;
  }

  return NULL;
}

/////////////////////////////////////////////////////////////////////
void print(CppUnit::Test *test)
{
  if (0 == test) return;

  std::cout << test->getName() << std::endl;

  if (test->getChildTestCount() == 0)
	  return;

  for (int i = 0; i < test->getChildTestCount(); i++)
  {
	 print(test->getChildTestAt(i));
  }

  return;
}



int main(int argc, char* argv[])
{
	// Get the top level suite from the registry
	CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    // Create the event manager and test controller
    CPPUNIT_NS::TestResult controller;
    
    // Add a listener that colllects test result
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener( &result );        
    
    // Add a listener that print dots as test run.
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener( &progress );      
    
    // Add the top suite to the test runner
    CPPUNIT_NS::TestRunner runner;
	if (argc > 1)
	{
		bool missing = false;
		for (int i=1; i<argc; i++)
		{
			CppUnit::Test * tmp = find(suite,argv[i]);

			if (tmp)
				runner.addTest(tmp);
			else
			{
				std::cerr << "cannot find test called:" << argv[i] << std::endl;
				missing = true;
			}

		}
		if (missing)
		{
			std::cout << "select one of the following: ";
			print (suite);
			return -1;
		}
	}
	else
	{
    	runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
    }
    
	CProcessBase * test_process = CreateNewProcess();
    test_process->SetWorkMode(eProcessWorkModeUnitTest);

	if (test_process->RequiresProcessInstanceForUnitTests())
	{
		test_process->SetUp();    
		SystemSleep(10, FALSE);
		runner.run( controller ); 
		SystemSleep(10,FALSE);    
  		test_process->TearDown();
	}
	else
	{
	    runner.run( controller );    
	}

    POBJDELETE(test_process);
    
    
    // Print test in a compiler compatible format.
    CPPUNIT_NS::CompilerOutputter outputter( &result, std::cerr );
    outputter.write(); 
    
    return result.wasSuccessful() ? 0 : 1;
}


bool IsUnitTest()
{
	return true;
}

void UnitTestAssert(const char * text)
{
	//CPPUNIT_ASSERT_MESSAGE( text, false );
}



