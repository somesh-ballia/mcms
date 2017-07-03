/*
 * TestTraceTailer.h
 *
 *  Created on: Apr 24, 2014
 *      Author: vasily
 */

#ifndef __TESTTRACETAILER_H__
#define __TESTTRACETAILER_H__


// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>


class CTestTraceTailer : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestTraceTailer );
	CPPUNIT_TEST( testTailerBasic );
	CPPUNIT_TEST( testTailerSize0 );
	CPPUNIT_TEST( testTailerSizeMinus1 );
	CPPUNIT_TEST( testTailerOverlapping );
	CPPUNIT_TEST( testSizeDecreasing );
	//...
	CPPUNIT_TEST_SUITE_END();

public:
	void testTailerBasic();
	void testTailerSize0();
	void testTailerSizeMinus1();
	void testTailerOverlapping();
	void testSizeDecreasing();
};

#endif /* __TESTTRACETAILER_H__ */
