// CMediaRecordingGet.h: interface for the CRsrcDetailGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//17/8/05		    			  Classes for resource details monitoring
//========   ==============   =====================================================================
#ifndef CMEDIARECORDINGGET_
#define CMEDIARECORDINGGET_


#include "SerializeObject.h"
#include "ResourceProcess.h"
//#include "RsrcAlloc.h"
#include "TaskApi.h"
#include "CardsStructs.h"
#include "Macros.h"
#include "SingleToneApi.h"
#include "Trace.h"
#include "InitCommonStrings.h"
//#include "SystemResources.h"

#include "DefJunctions.h"


#define  UNKNOWN_ACTION      -1
#define  NOT_FIND            -1

#define MAX_NUM_JUNCTIONS   300 //temp
//#define MAX_FILE_NAME_LEN   255 //temp

/////////////////////////////////////////////////////////////////////////////////////
class CJunction : public CSerializeObject
{
CLASS_TYPE_1(CJunction, CSerializeObject)
public:
  CJunction();
  CJunction(WORD id, char* p_description);
  CJunction(const CJunction& rhs);
	virtual const char* NameOf() const { return "CJunction";}
  virtual ~CJunction();
  const CJunction& operator=(const CJunction& other);

  void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
  int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
  int    DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);
  virtual CSerializeObject* Clone() {return new CJunction;}

  void SetId(WORD id)    { m_Id = id;  };
  WORD GetId()           { return m_Id;};
  const char* GetDescription() { return m_pDescription;};
  void SetDescription(char* p_description);
   
protected:
   
  WORD  m_Id;
  char* m_pDescription;
 
 };
 
/////////////////////////////////////////////////////////////////////////////////////
class CJunctionsList : public CSerializeObject
{
CLASS_TYPE_1(CJunctionsList, CSerializeObject)
public:
  CJunctionsList();
  CJunctionsList(const CJunctionsList& rhs);
  virtual ~CJunctionsList();
	virtual const char* NameOf() const { return "CJunctionsList";}
  const CJunctionsList& operator=(const CJunctionsList& other);

  void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
  int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
  int    DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);
  virtual CSerializeObject* Clone() {return new CJunctionsList;}

  void SetNumJunctions(WORD numJunctions) { m_numJunctions = (numJunctions < MAX_NUM_JUNCTIONS) ? numJunctions : MAX_NUM_JUNCTIONS;}
  WORD GetNumJunctions() { return m_numJunctions;}
  
  void InitJunctionsList();
 
  protected:
   
   WORD m_numJunctions;
   CJunction*  m_pJunctionsArray[ MAX_NUM_JUNCTIONS ];
   
  
 };
/////////////////////////////////////////////////////////////////////////////////////////
// *** junction params
/////////////////////////////////////////////////////////////////////////////////////////
class CJunctionParam : public CSerializeObject
{
CLASS_TYPE_1(CJunctionParam, CSerializeObject)
public:
  CJunctionParam();
  CJunctionParam(const CJunctionParam& rhs);
  virtual ~CJunctionParam();
	virtual const char* NameOf() const { return "CJunctionParam";}
  const CJunctionParam& operator=(const CJunctionParam& other);

  void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
  int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
  int    DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);
  virtual CSerializeObject* Clone() {return new CJunctionParam;}

  void SetId(WORD id)    { m_Id = id;  };
  WORD GetId()           { return m_Id;};
  
  void SetRate(WORD rate)    { m_rate = rate;};
  WORD GetRate()             { return m_rate;};
  
  const char* GetFileName() { return m_fileName;};
  void SetFileName(char* p_fileName);
  
  
   
protected:
   
  WORD  m_Id;
  WORD  m_rate; //may be enum later?!
  char  m_fileName[NEW_FILE_NAME_LEN];
 
 };
 
/////////////////////////////////////////////////////////////////////////////////////
class CJunctionParamList : public CSerializeObject
{
CLASS_TYPE_1(CJunctionParamList, CSerializeObject)
public:
  CJunctionParamList();
  CJunctionParamList(const CJunctionParamList& rhs);
  virtual ~CJunctionParamList();
	virtual const char* NameOf() const { return "CJunctionParamList";}
  const CJunctionParamList& operator=(const CJunctionParamList& other);

  void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
  int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
  int    DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);
  virtual CSerializeObject* Clone() {return new CJunctionParamList;}

  DWORD GetConfId()    { return m_confId;};
  DWORD GetPartyId()   { return m_partyId;};
  DWORD GetSizeLimit() { return m_size_limit;};
  CJunctionParam* GetJunction(WORD i) {return ( i < MAX_NUM_JUNCTIONS )? m_pJunctionsParamArray[i] : NULL;};
  //void SetNumJunctions(WORD numJunctions) { m_numJunctions = (numJunctions < MAX_NUM_JUNCTIONS) ? numJunctions : MAX_NUM_JUNCTIONS;}
  //WORD GetNumJunctions() { return m_numJunctions;}
   
  protected:
   
   DWORD m_confId;
   DWORD m_partyId;
   DWORD m_size_limit;
   
   WORD m_numJunctionParams;
   CJunctionParam*  m_pJunctionsParamArray[ MAX_NUM_JUNCTIONS ];
   
  
 };
/////////////////////////////////////////////////////////////////////////////////////////

#endif // !defined(_CMediaRecordingGet_H__)
