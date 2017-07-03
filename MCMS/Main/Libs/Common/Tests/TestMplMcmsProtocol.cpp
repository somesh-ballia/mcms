#include "TestMplMcmsProtocol.h"
#include "MplMcmsProtocol.h"
#include "NStream.h"
#include "Segment.h"




// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestMplMcmsProtocol );


void TestMplMcmsProtocol::setUp()
{
}


void TestMplMcmsProtocol::tearDown()
{
}


void TestMplMcmsProtocol::testConstructor()
{
	
	
}

void TestMplMcmsProtocol::testSerializeDeSerialzeMplApi()
{
    CMplMcmsProtocol mplPrtclBefore;
    mplPrtclBefore.AddCommonHeader(42);
    mplPrtclBefore.AddMessageDescriptionHeader();
    mplPrtclBefore.AddPhysicalHeader(1, 2, 3);
    mplPrtclBefore.AddPortDescriptionHeader(4 , 5, 6);  

    const char *data = "Cucu_lulu Data";
    mplPrtclBefore.AddData(strlen(data), data);

    CSegment seg;
    mplPrtclBefore.Serialize(seg, MPL_API_TYPE);

    CMplMcmsProtocol mplPrtclAfter;
    mplPrtclAfter.DeSerialize(seg, MPL_API_TYPE);
    
	CPPUNIT_ASSERT(mplPrtclAfter == mplPrtclBefore);
}

void TestMplMcmsProtocol::testSerializeDeSerialzeCSApi()
{
    CMplMcmsProtocol	mplProtBefore;
	mplProtBefore.AddCommonHeader(42, 1, 2, 3, 4);
	mplProtBefore.AddMessageDescriptionHeader();
	mplProtBefore.AddCSHeader(5, 6, 7);

    const char *data = "Cucu_lulu Data";
	mplProtBefore.AddData(strlen(data), data);

    CSegment seg;
    mplProtBefore.Serialize(seg, CS_API_TYPE);

    CMplMcmsProtocol mplProtAfter ;
    mplProtAfter.DeSerialize(seg, CS_API_TYPE);
    
	CPPUNIT_ASSERT(mplProtAfter == mplProtBefore);
}

void TestMplMcmsProtocol::testSerializeDeSerialzeTrace()
{
    string data = "cucu_lulu data";
    
    CMplMcmsProtocol mplProtBefore;
	mplProtBefore.AddCommonHeader(42, 1, 2, 3, 4);

    TRACE_HEADER_S header;
    header.m_processMessageNumber = 1;
    header.m_systemTick = 2;
    header.m_processType = 3;
    header.m_level = 4;
    header.m_sourceId = 5;
    header.m_messageLen = data.length();
    strncpy(header.m_taskName, "Task Name", MAX_TASK_NAME_LEN);    
    strncpy(header.m_objectName, "Object Name", MAX_OBJECT_NAME_LEN);
    header.m_topic_id = 6;
    header.m_unit_id = 7;
    header.m_conf_id = 8;
    header.m_party_id = 9;
    header.m_opcode = 10;
    strncpy(header.m_str_opcode, "Opcode Name", STR_OPCODE_LEN);
    strncpy(header.m_terminalName, "Terminal Name", MAX_TERMINAL_NAME_LEN);
    mplProtBefore.AddTraceHeader(header);

    mplProtBefore.AddData(data.length(), data.c_str());

    CSegment seg;
    mplProtBefore.SerializeLogger(seg);
    
    CMplMcmsProtocol mplProtAfter;
    mplProtAfter.DeSerialize(seg);
    
	CPPUNIT_ASSERT(mplProtAfter == mplProtBefore);

    // now trace from MPL (+ physical header)
    mplProtBefore.AddPhysicalHeader(1, 2, 3, 4, 5, 6);

    CSegment seg2;
    mplProtBefore.SerializeLogger(seg2);

    CMplMcmsProtocol mplProtAfter2;
    mplProtAfter2.DeSerialize(seg2);

    CPPUNIT_ASSERT(mplProtAfter2 == mplProtBefore);
}

void TestMplMcmsProtocol::TestOperatorAssignment()
{
    CMplMcmsProtocol	mplProt1;
    mplProt1.AddCommonHeader(42, 1, 2, 3, 4);
	mplProt1.AddMessageDescriptionHeader();
	mplProt1.AddCSHeader(5, 6, 7);

    const char *data = "Cucu_lulu Data";
	mplProt1.AddData(strlen(data), data);

    CMplMcmsProtocol	mplProt2;
    mplProt2.AddCommonHeader(42, 1, 2, 3, 4);
	mplProt2.AddMessageDescriptionHeader();
	mplProt2.AddCSHeader(5, 6, 7);
	mplProt2.AddData(strlen(data), data);

    CPPUNIT_ASSERT(mplProt1 == mplProt2);

    mplProt2.AddCommonHeader(42, 1, 2, 3, 84);

    CPPUNIT_ASSERT(mplProt1 != mplProt2);
}
