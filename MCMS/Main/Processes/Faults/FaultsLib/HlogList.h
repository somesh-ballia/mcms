//+========================================================================+
//                      HlogList.h                                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       HlogList.h  	                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 17.10.05   |                                                      |
//+========================================================================+


#ifndef  __HLOGLIST_H_
#define  __HLOGLIST_H_



#include <vector>

//#include "Trace.h"
#include "StatusesGeneral.h"
#include "SerializeObject.h"
#include "HlogElement.h"




//#define		MAX_HLOG_ELEMENTS_NUMBER_TO_SEND	10


template class std::vector< CHlogElement*>;
typedef std::vector< CHlogElement*> CLogFaultVector;



class CHlogList : public CSerializeObject
{
	CLASS_TYPE_1(CHlogList,CSerializeObject)
public:

	// Constructors
	CHlogList(const char* pszFileName);
	virtual ~CHlogList();
	// Operations

	virtual CSerializeObject* Clone() { return NULL; }

	int  GetNumOfFiles(const char * pathName);
	void GetFilePrefix(const char * dirName, char * baseName);
	void PrepareFileName(char * name, int num);

	virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;
	void          Serialize_Xml(CXMLDOMElement*& pFatherNode) const;
	void 		  SerializeXml(CXMLDOMElement*& pFatherNode, DWORD objToken, const DWORD idStart) const;
	//void          Serialize_Xml( CXMLDOMElement*& pFatherNode, DWORD objToken, const DWORD idStart ) const;
	void          SerializeXmlAllVector( CXMLDOMElement*& pFatherNode, DWORD objToken, const DWORD idStart ) const;
	virtual int   DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action);
	int   DeSerialize_Xml(CXMLDOMElement *pActionNode, char *pszError, const char* action,int FileNum=0);

	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(m_pszFileName, eNoActiveAlarm, eRenameFile); }
	STATUS WriteXmlFile() { return CSerializeObject::WriteXmlFile(m_pszFileName,"HLOG"); }
    void SortVectorByFaultId();
	STATUS	AddElement(CHlogElement* pHlog);
	void	TreatWriteXmlFileError();
	void	ClearFirstElemFromVector();
	void	ClearVector();
  /*  bool operator < (const CHlogElement & l, const CHlogElement & r) const ;
    bool operator == (const CHlogElement & l, const CHlogElement & r) const ;*/
    bool greater ( CHlogElement elem1, CHlogElement elem2 );
   
    void DumpAllVectorAsString(std::ostream &ostr) const;

private:
    CHlogList(const CHlogList& );
    CHlogList& operator=(const CHlogList&);
    DWORD GetBiggestFaultIdAndUpdateFileNum(WORD* FileNum);
    DWORD GetBiggestFaultId()const;
 	DWORD GetIncFaultId();

	void	SetIsFileOk(BOOL isOk);
	BOOL	GetIsFileOk();
    std::string RemoveInvalidXMLCharacters(char* xmlStr);
    char* ReadFaultFileIntoBuffer(char* FaultNameString);
    void WriteFaultFile(char* FaultNameString,std::string buffer);
	DWORD				m_FileNum;
	//BOOL operator<(const CHlogElement& first,const CHlogElement& second);
    //bool operator < (const element& l, const element& r)  ;
   // bool operator < (const CHlogElement & l, const CHlogElement & r)  ;
   //bool operator < (const CHlogElement & l)  ;
protected:
		// Attributes
    CLogFaultVector*	m_pVector;
	char*				m_pszFileName;
	DWORD				m_FaultId;

	BOOL				m_isFileOk;
	BOOL				m_isFileErrorAlreadyReported;
	DWORD               m_maxFaultsInList;
};






#endif /*__HLOGLIST_H_*/
