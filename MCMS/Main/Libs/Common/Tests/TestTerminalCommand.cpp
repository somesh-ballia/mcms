#include "TestTerminalCommand.h"
#include "TerminalCommand.h"
#include "Segment.h"
#include "NStream.h"


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestTerminalCommand );



TestTerminalCommand::TestTerminalCommand()
{
}

TestTerminalCommand::~TestTerminalCommand()
{
}

void TestTerminalCommand::testSerializeDeserializeSegment()
{
	char *argv [] = {"process_name", "command_name", "param1", "param2"};
	int argc = 4;
    CTerminalCommand command1("terminal_name", argv, argc);
    
    CSegment seg;
    command1.Serialize(seg);
    
    CTerminalCommand command2;
    command2.DeSerialize(seg);
    
    char *argvWrong [] = {"process_name", "command_nameWrong", "param1", "param2"};
	int argcWrong = 4;
    CTerminalCommand commandWrong("terminal_name", argvWrong, argcWrong);
    
    CPPUNIT_ASSERT(command1 == command2);
    CPPUNIT_ASSERT(commandWrong != command2);
}

void TestTerminalCommand::testSerializeDeserializeStream()
{
	char *argv [] = {"process_name", "command_name", "param1", "param2"};
	int argc = 4;
    CTerminalCommand command1("terminal_name", argv, argc);
    
    COstrStream ostr;
    command1.Serialize(ostr);
    
    CIstrStream istr(ostr.str().c_str());
    CTerminalCommand command2;
    command2.DeSerialize(istr);
    
    char *argvWrong [] = {"process_name", "command_nameWrong", "param1", "param2"};
	int argcWrong = 4;
    CTerminalCommand commandWrong("terminal_name", argvWrong, argcWrong);
    
    
    CPPUNIT_ASSERT(command1 == command2);
    CPPUNIT_ASSERT(commandWrong != command2);
}
