/*
 * TestTraceTailer.cpp
 *
 *  Created on: Apr 24, 2014
 *      Author: vasily
 */





#include <iostream>
using namespace std;



#include "TestTraceTailer.h"
#include "TraceTailer.h"

#include <sstream>

const char* LINES_ARR [] = {
		"Line1\n",
		"Line2\n",
		"Line3\n",
		"Line4\n",
		"Line5\n",
		"Line6\n",
		"Line7\n",
		"Line8\n",
		"Line9\n",
		"Line10\n",
		"Line11\n",
		"Line12\n",
		"Line13\n",
		"Line14\n",
		"Line15\n",
		"Line16\n",
		"Line17\n",
		"Line18\n",
		"Line19\n",
		"Line20\n",
};

const size_t ARRAY_SIZE = sizeof(LINES_ARR)/sizeof(LINES_ARR[0]);


CPPUNIT_TEST_SUITE_REGISTRATION( CTestTraceTailer );


void CTestTraceTailer::testTailerBasic()
{
	std::string line = "Line1\n";

	CTraceTailer tailer;
	tailer.SetTrace(line);

	std::ostringstream sstream;
	tailer.GetTail(sstream);
	CPPUNIT_ASSERT_MESSAGE(sstream.str(),sstream.str() == line);
}

void CTestTraceTailer::testTailerSize0()
{
	std::string expected;

	CTraceTailer tailer;
	for (size_t i=0; i<ARRAY_SIZE; i++)
	{
		tailer.SetTrace(LINES_ARR[i]);

		if( i >= ARRAY_SIZE - MIN_TAIL_LEN )
			expected.append(LINES_ARR[i]);
	}
	tailer.SetTailLen(0);

	// should be MIN_TAIL_LEN last lines
	std::ostringstream sstream;
	tailer.GetTail(sstream);
	CPPUNIT_ASSERT_MESSAGE(sstream.str(),sstream.str() == expected);
}

void CTestTraceTailer::testTailerSizeMinus1()
{
	std::string line = "Line1\n";

	CTraceTailer tailer;
	tailer.SetTrace(line);
	tailer.SetTailLen(-1);

	std::ostringstream sstream;
	tailer.GetTail(sstream);
	CPPUNIT_ASSERT_MESSAGE(sstream.str(),sstream.str() == line);
}

void CTestTraceTailer::testTailerOverlapping()
{
	std::string expected;
	CTraceTailer tailer;
	tailer.SetTailLen(5);

	DWORD min_lines = std::max(MIN_TAIL_LEN,(DWORD)5);

	for (size_t i=0; i<ARRAY_SIZE; i++)
	{
		tailer.SetTrace(LINES_ARR[i]);

		if( i >= ARRAY_SIZE - min_lines )
			expected.append(LINES_ARR[i]);
	}

	// should content only 5 last lines
	std::ostringstream sstream;
	tailer.GetTail(sstream);
	CPPUNIT_ASSERT_MESSAGE(sstream.str(),sstream.str() == expected);
}

void CTestTraceTailer::testSizeDecreasing()
{
	std::string expected, full;
	CTraceTailer tailer;
	tailer.SetTailLen(ARRAY_SIZE);

	DWORD min_lines = std::max(MIN_TAIL_LEN,(DWORD)5);

	for (size_t i=0; i<ARRAY_SIZE; i++)
	{
		tailer.SetTrace(LINES_ARR[i]);

		full.append(LINES_ARR[i]);

		if( i >= ARRAY_SIZE - min_lines )
			expected.append(LINES_ARR[i]);
	}

	// should content all 20 lines
	std::ostringstream sstream20;
	tailer.GetTail(sstream20);
	CPPUNIT_ASSERT_MESSAGE(sstream20.str(),sstream20.str() == full);

	// decrease size to 5
	tailer.SetTailLen(5);
	// should content only 5 last lines
	std::ostringstream sstream5;
	tailer.GetTail(sstream5);
	CPPUNIT_ASSERT_MESSAGE(sstream5.str(),sstream5.str() == expected);
}










