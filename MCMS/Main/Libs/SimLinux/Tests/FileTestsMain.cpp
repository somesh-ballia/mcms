/*
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include "DataTypes.h"

int main( )
{
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
  runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
  runner.run( controller );

  // Print test in a compiler compatible format.
  CPPUNIT_NS::CompilerOutputter outputter( &result, std::cerr );
  outputter.write(); 

  return result.wasSuccessful() ? 0 : 1;
}

void PAssert(char const* a,
			 unsigned short b,
			 unsigned long c,
			 unsigned long d,
			 char const* e,
			 BYTE x)
{
}

void PAssertDebug(char const*, unsigned short, unsigned long, unsigned long)
{
}

void OutTraceMessage(const unsigned short level,
				 const unsigned long sourceId,
				 const char * message1,
				 const char * message2,
				 const unsigned char  unit_id,
				 const unsigned long conf_id,
				 const unsigned long party_id,
				 const unsigned long opcode,
				 const char * str_opcode)
{}


void UnitTestAssert(char const*)
{
    return;
}

*/
