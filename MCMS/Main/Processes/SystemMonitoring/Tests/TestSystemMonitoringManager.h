/*
 * TestSystemMonitoringManager.h
 *
 *  Created on: Aug 1, 2014
 *      Author: penrod
 */

#ifndef TESTSYSTEMMONITORINGMANAGER_H_
#define TESTSYSTEMMONITORINGMANAGER_H_

#include <cppunit/extensions/HelperMacros.h>
#include "SystemMonitoringManager.h"
#include <string>

class TestSystemMonitoringManager;

class CTestedTmp : public CSystemMonitoringManager
{
	friend class TestSystemMonitoringManager;
public:
	CTestedTmp(){};
	virtual ~CTestedTmp(){};

};

class TestSystemMonitoringManager : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestSystemMonitoringManager );
	CPPUNIT_TEST( testNinjaCoreManager );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown();

	void testNinjaCoreManager();

protected:
	void createCoredumpMap(std::string strProcessName, int unCount);
	void createEmptyFile(const char *pFileName);
	void deleteFile(const char *pFileName);
	void DumpCores();
private:
	CTestedTmp *pSystemMonitoringManager;
};

#endif /* TESTSYSTEMMONITORINGMANAGER_H_ */
