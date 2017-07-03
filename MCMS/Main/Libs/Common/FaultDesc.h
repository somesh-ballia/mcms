//
//     Implementation of Faults Descriptors
//+========================================================================+

#ifndef __FAULT_DESC_H_
#define __FAULT_DESC_H_

#include <map>
using namespace std;

#include "SerializeObject.h"
#include "StringsLen.h"
#include "FaultsDefines.h"


class COstrStream;
class CIstrStream;
//class CXMLDOMElement;

/// Vasily - temp, should be in globals
//size of i(o)strstream



class CFaultDesc;
class CErrorCodeStringMapWrapper;





class FactoryFaultDesc : public CPObject
{
private:
    FactoryFaultDesc(){;}

public:

    static char* GetAllocatedFaultDesciption(BYTE subject,
                                  DWORD errorCode,
                                  BYTE errorLevel,
                                  const string &description,
                                  DWORD boardId,
                                  DWORD unitId,
                                  WORD theType);
    static CFaultDesc *GetAllocatedFaultDescByDescription(const char *desc);
    static CFaultDesc *GetAllocatedFaultDescBySubject(BYTE subject,
                                                      DWORD errorCode,
                                                      BYTE errorLevel,
                                                      const string &description,
                                                      DWORD boardId,
                                                      DWORD unitId,
                                                      WORD theType);
    virtual const char* NameOf() const { return "FactoryFaultDesc";}
};






/////////////////////////////////////////////////////////////////////////////
// CFaultDesc
class CFaultDesc : public CSerializeObject
{
	CLASS_TYPE_1(CFaultDesc,CSerializeObject)
public:
		//Constructors
	CFaultDesc();
	CFaultDesc( const BYTE subject, const DWORD error, const BYTE error_level,
	           const eProcessType processName=eProcessTypeInvalid);
	virtual ~CFaultDesc();
	
	virtual const char* NameOf() const { return "CFaultDesc";}

		// Overrides
	CSerializeObject* Clone() { return NULL; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		// Implementation
	virtual void  Serialize(COstrStream& rOstr) const;
	virtual void  SerializeLong(COstrStream& rOstr) const;
	virtual void  DeSerialize(CIstrStream& rIstr);
	char*  SerializeString(const WORD sizeType = 0) const; // 0-short; 1-long
	void   DeSerializeString(char* pszMsg);

	void SerializeXmlCommon(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXmlCommon(CXMLDOMElement *pActionNode,char *pszError,const char* action="");
    
	const char* GetSubjectAsString() const;

	const char* GetLevelAsString() const;
    virtual const char *GetDescription(){return NULL;}
    virtual void SetDescription(const char *desc){;}
    
	BYTE			GetSubject() const;
	BYTE			GetFaultLevel() const;
	DWORD			GetErrorCode() const;
	eProcessType	GetProcessName() const;
	DWORD			GetFaultId() const;
	DWORD			GetStaticFaultId() const;
	void			SetSubject(const BYTE  subject);
	void			SetFaultLevel(const BYTE faultLevel);
	void			SetErrorCode(const DWORD errorCode);
	void			SetProcessName(const eProcessType processName);
	void			SetFaultId(const DWORD fid);

protected:
		// Attributes
	BYTE			m_subject;
	BYTE			m_faultLevel;
	DWORD			m_error_code;
	eProcessType	m_processName;
	DWORD			m_faultId;
};


/////////////////////////////////////////////////////////////////////////////
// CFaultCardDesc

class CFaultCardDesc : public CFaultDesc
{
CLASS_TYPE_1(CFaultCardDesc,CFaultDesc )
public:
		//Constructors
	CFaultCardDesc();
	CFaultCardDesc( const BYTE  subject, const DWORD error_code, const BYTE  faultLevel,
	                const WORD boardId, const WORD unitId, const WORD cardType,
	                const char* pszMes, const eProcessType processName=eProcessTypeInvalid );
	virtual ~CFaultCardDesc();
	virtual const char* NameOf() const { return "CFaultCardDesc";}
	
		// Overrides
	CSerializeObject* Clone() { return NULL; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action="");

		// Implementation
	void  Serialize(COstrStream& rOstr) const;
	void  SerializeLong(COstrStream& rOstr) const;
	void  DeSerialize(CIstrStream& rIstr);

	WORD   GetBoardId() const;
	void   SetBoardId(const WORD boardId);

	WORD   GetUnitId() const;
	void   SetUnitId(const WORD unitId);

	WORD   GetCardType() const;
	void   SetCardType(const WORD cardType);

	void   SetMessage(const char*  mes);
	const char*  GetMessage () const;

    virtual const char *GetDescription(){return m_genMes;}
    virtual void SetDescription(const char *desc){SetMessage(desc);}

protected:
		// Attributes
	WORD	m_boardId;
	WORD	m_unitId;
	WORD	m_cardType;
	char	m_genMes[GENERAL_MES_LEN];
};


/////////////////////////////////////////////////////////////////////////////
// CFaultAssertDesc

class CFaultAssertDesc : public CFaultDesc
{
CLASS_TYPE_1(CFaultAssertDesc,CFaultDesc)
public:
		//Constructors
	CFaultAssertDesc();
	CFaultAssertDesc( const BYTE subject, const DWORD error_code, const BYTE  faultLevel,
				const char* name, const WORD lineNumber, const DWORD assertCode, const char* assertstring,
				const eProcessType processName=eProcessTypeInvalid);
	virtual ~CFaultAssertDesc();
	virtual const char* NameOf() const { return "CFaultAssertDesc";}

		// Overrides
	CSerializeObject* Clone() { return NULL; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		// Implementation
	void  Serialize(COstrStream& rOstr) const;
	void  SerializeLong(COstrStream& rOstr) const;
	void  DeSerialize(CIstrStream& rIstr);

	void   SetFileName(const char*  name);
	const char*  GetFileName () const;
	DWORD  GetLineNumber() const;
	DWORD  GetAssertCode() const;
	void   SetLineNumber(const DWORD lineNumber);
	void   SetAssertCode(const DWORD assertCode);
	void   SetAssertString(const char*  assertstr);
	const char*  GetAssertString() const;

protected:
		// Attributes
	char	m_fileName[NEW_FILE_NAME_LEN];
	DWORD	m_lineNumber;
	DWORD	m_assertCode; 
	char	m_assertstring[EXCEPT_HANDL_MES_LEN];
};


/////////////////////////////////////////////////////////////////////////////
// CFaultFileDesc

class CFaultFileDesc : public CFaultDesc
{
	CLASS_TYPE_1(CFaultFileDesc, CFaultDesc)
public:
		//Constructors
	CFaultFileDesc();
	CFaultFileDesc( const BYTE subject, const DWORD error_code, const BYTE faultLevel,
				    const BYTE fileType, const char* fileName = "",
				    const eProcessType processName=eProcessTypeInvalid );
	virtual ~CFaultFileDesc();
	virtual const char* NameOf() const { return "CFaultFileDesc";}

		// Overrides
	CSerializeObject* Clone() { return NULL; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		// Implementation
	void  Serialize(COstrStream& rOstr) const;
	void  SerializeLong(COstrStream& rOstr) const;
	void  DeSerialize(CIstrStream& rIstr);

	const char* GetFileTypeAsString() const;

	BYTE   GetFileType() const;
	void   SetFileType(const BYTE fileType);
	const char*   GetfileName () const;
	void   SetfileName(const char*  name);

protected:
		// Attributes
	BYTE	m_fileType;
	char	m_fileName[FILE_NAME_LEN];
};


/////////////////////////////////////////////////////////////////////////////
// CFaultReservationDesc
class CFaultReservationDesc : public CFaultDesc
{
	CLASS_TYPE_1(CFaultReservationDesc, CFaultDesc)
public:
		//Constructors
	CFaultReservationDesc();
	CFaultReservationDesc( const BYTE subject, const DWORD error_code, const BYTE faultLevel,
				           const DWORD confId, const DWORD partyId, const char* confName = "",
				           const char* partyName = "", const eProcessType processName=eProcessTypeInvalid );
	virtual ~CFaultReservationDesc();
	virtual const char* NameOf() const { return "CFaultReservationDesc";}

		// Overrides
	CSerializeObject* Clone() { return NULL; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		// Implementation
	void  Serialize(COstrStream& rOstr) const;
	void  SerializeLong(COstrStream& rOstr) const;
	void  DeSerialize(CIstrStream& rIstr);

	DWORD  GetConfId() const;
	DWORD  GetPartyId() const;
	void   SetConfId(const DWORD confId);
	void   SetPartyId(const DWORD partyId);
	void   SetConfName(const char*  name);
	const char*   GetConfName () const;
	void   SetPartyName(const char*  name);
	const char*   GetPartyName () const;

protected:
		// Attributes
	DWORD	m_confId;
	DWORD	m_partyId;
	char	m_H243confName[H243_NAME_LEN];    //conferences name
	char	m_H243_partyName[H243_NAME_LEN];  //party name
};


/////////////////////////////////////////////////////////////////////////////
// CFaultExceptHandlDesc

class CFaultExceptHandlDesc : public CFaultDesc
{
	CLASS_TYPE_1(CFaultExceptHandlDesc, CFaultDesc)
public:
		//Constructors
	CFaultExceptHandlDesc();
	CFaultExceptHandlDesc( const BYTE subject, const DWORD error_code, const BYTE  faultLevel,
	                       const char* pszMes, const eProcessType processName=eProcessTypeInvalid );
	virtual ~CFaultExceptHandlDesc();
	virtual const char* NameOf() const { return "CFaultExceptHandlDesc";}

		// Overrides
	CSerializeObject* Clone() { return NULL; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		// Implementation
	void  Serialize(COstrStream& rOstr) const;
	void  DeSerialize(CIstrStream& rIstr);

	void   SetMessage(const char*  mes);
	const char*  GetMessage () const;

protected:
		// Attributes
	char	m_excpHndlMes[EXCEPT_HANDL_MES_LEN];
};


/////////////////////////////////////////////////////////////////////////////
// CFaultGeneralDesc

class CFaultGeneralDesc : public CFaultDesc
{
	CLASS_TYPE_1(CFaultGeneralDesc, CFaultDesc)
public:

		//Constructors
	CFaultGeneralDesc();
	CFaultGeneralDesc( const BYTE subject, const DWORD error_code, const BYTE faultLevel,
	                   const char* pszMes, const eProcessType processName=eProcessTypeInvalid );
	virtual ~CFaultGeneralDesc();
	virtual const char* NameOf() const { return "CFaultExceptHandlDesc";}

		// Overrides
	CSerializeObject* Clone() { return NULL; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

		// Implementation
	void  Serialize(COstrStream& rOstr) const;
	void  DeSerialize(CIstrStream& rIstr);

	void   SetMessage(const char*  mes);
	const char*  GetMessage () const;

    virtual const char * GetDescription(){return m_genMes;}
    virtual void SetDescription(const char *desc){SetMessage(desc);}
    
    
protected:
		// Attributes
	char	m_genMes[GENERAL_MES_LEN];
};

/////////////////////////////////////////////////////////////////////////////
// CFaultUnitDesc

class CFaultUnitDesc : public CFaultDesc
{
CLASS_TYPE_1(CFaultUnitDesc,CFaultDesc )
public:
		//Constructors
	CFaultUnitDesc();
	CFaultUnitDesc( const BYTE  subject, const DWORD error_code, const BYTE  faultLevel,
	                const WORD boardId, const WORD unitId, const WORD unitType,
	                const char* pszMes, const eProcessType processName=eProcessTypeInvalid );
	virtual ~CFaultUnitDesc();
	virtual const char* NameOf() const { return "CFaultUnitDesc";}

		// Overrides
	CSerializeObject* Clone() { return NULL; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action="");

		// Implementation
	void  Serialize(COstrStream& rOstr) const;
	void  SerializeLong(COstrStream& rOstr) const;
	void  DeSerialize(CIstrStream& rIstr);

	WORD   GetBoardId() const;
	void   SetBoardId(const WORD boardId);

	WORD   GetUnitId() const;
	void   SetUnitId(const WORD unitId);

	WORD   GetUnitType() const;
	void   SetUnitType(const WORD unitType);

	void   SetMessage(const char*  mes);
	const char*  GetMessage () const;

    virtual const char * GetDescription(){return m_genMes;}
    virtual void SetDescription(const char *desc){SetMessage(desc);}

protected:
		// Attributes
	WORD	m_boardId;
	WORD	m_unitId;
	WORD	m_unitType;
	char	m_genMes[GENERAL_MES_LEN];
};


/////////////////////////////////////////////////////////////////////////////
// CFaultMplDesc

class CFaultMplDesc : public CFaultDesc
{
CLASS_TYPE_1(CFaultMplDesc,CFaultDesc )
public:
		//Constructors
	CFaultMplDesc();
	CFaultMplDesc( const BYTE subject, const DWORD error_code,
	               const BYTE  faultLevel, const WORD compType,
	               const char* pszMes, const eProcessType processName=eProcessTypeInvalid );
	virtual ~CFaultMplDesc();
	virtual const char* NameOf() const { return "CFaultMplDesc";}

		// Overrides
	CSerializeObject* Clone() { return NULL; }
	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action="");

		// Implementation
	void  Serialize(COstrStream& rOstr) const;
	void  SerializeLong(COstrStream& rOstr) const;
	void  DeSerialize(CIstrStream& rIstr);

	WORD   GetCompType() const;
	void   SetCompType(const WORD compType);

	void   SetMessage(const char*  mes);
	const char*  GetMessage () const;

    virtual const char * GetDescription(){return m_genMes;}
    virtual void SetDescription(const char *desc){SetMessage(desc);}
    
protected:
		// Attributes
	WORD	m_compType;
	char	m_genMes[GENERAL_MES_LEN];
};



#endif /* __FAULT_DESC_H_ */


