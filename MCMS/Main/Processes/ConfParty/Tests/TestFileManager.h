#ifndef TESTFILEMANAGER_H_
#define TESTFILEMANAGER_H_

#include <cppunit/extensions/HelperMacros.h>
#include "FileManager.h"
#include <unistd.h>

class CCommRes;


//---------------------------- MOCK Object ------------------
class CFileManagerMock : public CFileManager<CCommRes>
{
public:
	CFileManagerMock(std::string dirName):CFileManager<CCommRes>(dirName){}
	CFileManagerMock(const CFileManagerMock & other):CFileManager<CCommRes>(other){}
	CFileManagerMock & operator =(const CFileManagerMock & other){
		CFileManager<CCommRes>::operator=(other);
		return *this;
	}
	
	bool operator ==(const CFileManagerMock & other)const{
		return (m_dirName == other.m_dirName &&
				m_filesArray == other.m_filesArray);
	}
};
//--------------------------------------------------------------



class CTestFileManager   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestFileManager );
	//CPPUNIT_TEST(TestGetFileData );
	CPPUNIT_TEST( TestAdd);
	CPPUNIT_TEST(TestGetFileData );
	CPPUNIT_TEST(TestDeleteData);
	CPPUNIT_TEST(TestCopyCtorAssignOperator);
	CPPUNIT_TEST(TestLoadDataToVect);
	//...
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown(); 
	
	void TestAdd();
	void TestGetFileData();
	void TestDeleteData();
	void TestScenario1();
	void TestCopyCtorAssignOperator();
	void TestLoadDataToVect();
	
	/** Optional functionality not tested
	 */
	void TestGetFileName();
	
	bool CompareCommRes(const CCommRes & commRes,const CCommRes & otherRes)const;
	
private:
	 CFileManagerMock * m_pFileManager;
	std::string m_testFolder;
};



#endif /*TESTFILEMANAGER_H_*/

