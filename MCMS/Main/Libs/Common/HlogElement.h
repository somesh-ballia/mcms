//
//     Implementation of Faults elements
//+========================================================================+

#ifndef __HLOG_ELEMENT_H_
#define __HLOG_ELEMENT_H_


#include "SerializeObject.h"
#include "StructTm.h"
#include "FaultsDefines.h"

class COstrStream;
class CIstrStream;
class CSegment;


//////////////   Hlog common codes     ///////////////////////////////////////////////
//system faults
#define COMMON_SYSTEM_STARTUP 10


//////////////   Hlog fault codes     ///////////////////////////////////////////////
//software
#define MIN_COMMON_CODE		0
#define MAX_COMMON_CODE		999

#define MIN_FAULTS_CODE		1000
#define MAX_FAULTS_CODE		9999

#define MIN_NET_CODE		10000
#define MAX_NET_CODE		10999

#define MIN_ACCOUNT_CODE	20000
#define MAX_ACCOUNT_CODE	20999
 
#define FAULT_SOFTWARE_BUG	1000
#define FAULT_FATAL_FAIURE  1001
#define FAULT_XML_PARSE		1002


//cards configuration faults
#define FAULT_EMPTY_CARD_CONFIG				1100
#define FAULT_BAD_CARD_CONFIG				1101
#define FAULT_EMPTY_SLOT					1102
#define FAULT_CARD_IS_IN_SIMULATOR_MODE		1103


//phone numbers configuration faults
#define FAULT_EMPTY_PHONE_NUMBER_CONFIG		1200
#define FAULT_BAD_PHONE_NUMBER_CONFIG		1201

//General card faults
#define FAULT_CONNECTION_LOST				1250
#define FAULT_CARD_MANAGER_RECONNECT		1251
#define FAULT_EXTERNAL_FAULT_EMB			1252   

//net faults
#define FAULT_NET_ALARM						1300
#define FAULT_NET_D_CHANNEL_NOT_ESTABLISHED	1301

//comm board faults
#define FAULT_COMM_BOARD_KEEP_ALIVE_FAILURE	1500

//comm board faults
#define FAULT_TASK_ACTIVE_ALARM				1600

//////////////   Hlog Network codes     ///////////////////////////////////////////////
#define NETWORD_SETUP_REQUEST				10001
//////////////   Hlog Accounting codes     ////////////////////////////////////////////
#define ACCOUNT_CONF_START					20001


const DWORD FAULTS_MASK  = 1 << 0; 
const DWORD NETWORK_MASK = 1 << 1; 
const DWORD ACCOUNT_MASK = 1 << 2; 

class CHlogElement;

CHlogElement* NewCHlogElement(const WORD code);


///////////////////////////////////////////////////////////////////////////////////////
// CHlogElement - base for all Faults elements

class CHlogElement : public CSerializeObject
{
CLASS_TYPE_1(CHlogElement,CSerializeObject)
public:
		//Constructors
	CHlogElement();
	//CHlogElement(const WORD code, const char* description);
	virtual const char* NameOf() const { return "CHlogElement";}
	CHlogElement(const WORD code);
	CHlogElement(const CHlogElement&);
	virtual ~CHlogElement();
	
    CHlogElement&  operator=(const CHlogElement& other);
	bool operator==(const CHlogElement &other)const;
	//bool operator==( const Obj &other ) const; 
	//bool operator<( const Obj &other ) const; 
	////bool operator < (const CHlogElement & l)const;
	//bool operator<(const CHlogElement& lhs,const CHlogElement& rhs)const;
	bool operator < (const CHlogElement &other)const;
	//bool operator==(const CHlogElement & lhs,const CHlogElement & rhs)const;
		// Overrides
	CSerializeObject* Clone() { return new CHlogElement(*this) ; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	int  DeSerialize_Xml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
//    int  DeSerializeXml_(CXMLDOMElement *pActionNode,char *pszError,const char* action,const char * fileName);
		// Implementation
	virtual CHlogElement* Copy() const;					// virtual copy constructor
	virtual char* Serialize() const;					// Asccii serialization
	virtual void  Serialize(COstrStream& oStr) const;	// Asccii serialization
	virtual void  DeSerialize(char* p);					// Ascii De-Serialize
	virtual void  DeSerialize(CIstrStream& iStr);		// Ascii De-Serialize
	virtual BYTE* Serialize(WORD& n) const;				// Binary serialization 
	virtual void  DeSerialize(BYTE* p);					// Binary De-Serialization 
	virtual void  Serialize(CSegment& seg) const;
	virtual void  DeSerialize(CSegment& seg);
	virtual void  Dump(std::ostream&) const{}
	virtual void  DumpAsString(std::ostream&) const{}

	
	void SerializeXmlCommon(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXmlCommon(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	void  SetTime(const CStructTm& tm); 
	const CStructTm& GetTime() const { return m_hlog.time; }
	DWORD GetMask() const { return m_hlog.subjectMask; }
	void SetMask(DWORD mask) { m_hlog.subjectMask = mask ; }

	void SetCode(WORD Code) { m_hlog.code = Code ; }
	WORD GetCode() const { return m_hlog.code;};
	void SetType(WORD Type) { m_hlog.type = Type ; }
	WORD GetType() const { return m_hlog.type;}
	virtual size_t Sizeof();// should be the amount Serialized to a file
	BYTE IsFault() const;
	
	void SetFaultId(DWORD fid);
	DWORD GetFaultId() const;

	void IncreaseReferenceCounter();
	void DecreaseReferenceCounter();
	int  GetReferenceCounter();
	
	void SetFileNum(WORD file_number) { m_read_from_file_number = file_number ; }
	WORD GetFileNum() const { return m_read_from_file_number;};
	//////////////////////////////////////////////////////////////////////////////
  // bool operator < (const CHlogElement & l);

//protected:
	// Attributes
	struct hlogType  {
		DWORD subjectMask; // network,faults or accounting
		WORD  code;
		WORD type;
		DWORD faultId;
		CStructTm time;
		//char description[FAULT_DESCRIPTION_LENGTH];    // move to derived class CLogFltElement
	} m_hlog;
	
	int m_referenceCounter; // for tracing the number of pointers that point to the element
	int m_read_from_file_number;
};


///////////////////////////////////////////////////////////////////////////////////////
class CLogFltElement : public CHlogElement
{
CLASS_TYPE_1(CLogFltElement,CHlogElement)
public:
	CLogFltElement();
	CLogFltElement(const WORD code, const char* description);
	CLogFltElement(const CLogFltElement&);
	virtual const char* NameOf() const { return "CLogFltElement";}
	virtual ~CLogFltElement();

    CLogFltElement&  operator=(const CLogFltElement& other);
    bool operator==(const CLogFltElement& other);
    bool operator!=(const CLogFltElement& other);
	virtual void Dump(std::ostream&) const;
	virtual void DumpAsString(std::ostream&) const;

	
		// Overrides
	CSerializeObject* Clone() { return new CLogFltElement(*this); }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		// Implementation
	virtual CHlogElement* Copy() const;
	char* Serialize() const;					// Asccii serialization
	void  DeSerialize(char* p);					// Asccii DeSerialization
	void  Serialize(COstrStream& oStr) const;	// Asccii serialization
	void  DeSerialize(CIstrStream& iStr);		// Asccii DeSerialization
	BYTE* Serialize(WORD& n) const;				// Binary serialization 
	void  DeSerialize(BYTE* p);					// Binary DeSerialization 

	virtual size_t Sizeof();
    
	const char* GetDescription() const { return m_hlogData.description; };
    void GetCleanDescription(string & outDescription)const;
    
	void SetDescription(const char* desc);
    void UpdateDescription(const char* desc);
    
	void SetIsExternal(bool val) {m_hlogData.isExternal = val;}
	bool GetIsExternal()const{return m_hlogData.isExternal;}

	void SetUserId(DWORD userId){m_hlogData.userId = userId;}
	DWORD GetUserId()const{return m_hlogData.userId;}

//	void SetIsItCardStatus(BOOL isIt);
//	BOOL GetIsItCardStatus();

    bool IsSimilar(const CLogFltElement & other)const;

    void SetIndex(DWORD index){m_hlogData.index = index;}
    DWORD GetIndex()const{return m_hlogData.index;}
    
    
	// Attributes
	struct hlogFltType{
		char description[FAULT_DESCRIPTION_LENGTH];
		bool isExternal;
		DWORD userId;
        DWORD index;
	} m_hlogData;
	
//	BOOL m_isItCardStatus;
};


///////////////////////////////////////////////////////////////////////////////////////
class CLogNetElement : public CHlogElement
{
	CLASS_TYPE_1(CLogNetElement,CHlogElement)
public:
	CLogNetElement();
	virtual ~CLogNetElement(){};

	virtual const char* NameOf() const { return "CLogNetElement";}
		// Overrides
	CSerializeObject* Clone() { return new CLogNetElement(*this); }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		// Implementation
	virtual size_t Sizeof();

protected:
	// Attributes
	struct hlogNetType  {
		// element data
	} m_hlogData;
};


///////////////////////////////////////////////////////////////////////////////////////
class CLogAccntElement : public CHlogElement
{
CLASS_TYPE_1(CLogAccntElement,CHlogElement)
public:
	CLogAccntElement();
	virtual ~CLogAccntElement(){};

	virtual const char* NameOf() const { return "CLogAccntElement";}
		// Overrides
	CSerializeObject* Clone() { return new CLogAccntElement(*this); }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		// Implementation
	virtual size_t Sizeof();	// should be the amount Serialized to a file

protected:
	// Attributes
	struct hlogAccntType  {
		// element data
	} m_hlogData;
};


#endif /* __HLOG_ELEMENT_H_ */












