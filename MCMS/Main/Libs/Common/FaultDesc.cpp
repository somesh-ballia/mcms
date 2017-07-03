//
//     Implementation of Faults Descriptors
//+========================================================================+


#include "psosxml.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "FaultsDefines.h"
#include "FaultDesc.h"
#include "InternalProcessStatuses.h"
#include "ProcessBase.h"
#include "ObjString.h"
#include "ApiStatuses.h"
#include "AlarmStrTable.h"




/////////////////////////////////////////////////////////////////////////////
char* FactoryFaultDesc::GetAllocatedFaultDesciption(BYTE subject,
                                                    DWORD errorCode,
                                                    BYTE errorLevel,
                                                    const string &description,
                                                    DWORD boardId,
                                                    DWORD unitId,
                                                    WORD theType)
{
    CFaultDesc *pFault = FactoryFaultDesc::GetAllocatedFaultDescBySubject(subject,
                                                                          errorCode,
                                                                          errorLevel,
                                                                          description,
                                                                          boardId,
                                                                          unitId,
                                                                          theType);
	char *pszMsg = pFault->SerializeString(0);
    POBJDELETE(pFault);
    return pszMsg;
}

/////////////////////////////////////////////////////////////////////////////
CFaultDesc *FactoryFaultDesc::GetAllocatedFaultDescByDescription(const char *desc)
{
    ALLOCBUFFER(pTempStr,FAULT_DESCRIPTION_LENGTH);
	memcpy(pTempStr, desc, FAULT_DESCRIPTION_LENGTH);
	pTempStr[FAULT_DESCRIPTION_LENGTH-1] = 0;

	CFaultDesc rFaultDescTemp;
	rFaultDescTemp.DeSerializeString(pTempStr);

    CFaultDesc* pFaultDesc = NULL;
	switch( rFaultDescTemp.GetSubject() )
	{
		case  FAULT_STARTUP_SUBJECT :
			pFaultDesc = new CFaultDesc;
			break;
		case  FAULT_GENERAL_SUBJECT :
			pFaultDesc = new CFaultGeneralDesc;
			break;
		case  FAULT_ASSERT_SUBJECT :
			pFaultDesc = new CFaultAssertDesc;
			break;
		case  FAULT_FILE_SUBJECT :
			pFaultDesc = new CFaultFileDesc;
			break;
		case  FAULT_CARD_SUBJECT :
			pFaultDesc = new CFaultCardDesc;
			break;
		case  FAULT_RESERVATION_SUBJECT :
		case  FAULT_CONFERENCE_SUBJECT  :
			pFaultDesc = new CFaultReservationDesc;
			break;
		case  FAULT_EXCEPTION_SUBJECT :
			pFaultDesc = new CFaultExceptHandlDesc;
			break;
		case  FAULT_UNIT_SUBJECT :
			pFaultDesc = new CFaultUnitDesc;
			break;
		case  FAULT_MPL_SUBJECT :
			pFaultDesc = new CFaultMplDesc;
			break;
	}
	if( pFaultDesc ) 
	{
		pFaultDesc->DeSerializeString(pTempStr);
    }

    DEALLOCBUFFER(pTempStr);
    
    return pFaultDesc;
}


CFaultDesc *FactoryFaultDesc::GetAllocatedFaultDescBySubject(BYTE subject,
                                                             DWORD errorCode,
                                                             BYTE errorLevel,
                                                             const string &description,
                                                             DWORD boardId,
                                                             DWORD unitId,
                                                             WORD theType)
{
    const eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
    
    CFaultDesc *pFault = NULL;
	if (FAULT_CARD_SUBJECT == subject)
    {   
		pFault = new CFaultCardDesc(subject,
                                    errorCode,
                                    errorLevel,
                                    boardId,
                                    unitId,
                                    theType,
                                    description.c_str(),
                                    processType);
    }
	else if(FAULT_UNIT_SUBJECT == subject)
    {
		pFault = new CFaultUnitDesc(subject,
                                    errorCode,
                                    errorLevel,
                                    boardId,
                                    unitId,
                                    theType,
                                    description.c_str(),
                                    processType);
    }
    else if (FAULT_MPL_SUBJECT == subject)
    {
        pFault = new CFaultMplDesc(subject,
                                   errorCode,
                                   errorLevel,
                                   theType,
                                   description.c_str(),
                                   processType);
    }
    else
    {
		pFault = new CFaultGeneralDesc(subject,
                                       errorCode,
                                       errorLevel,
                                       description.c_str(),
                                       processType);
    }
    return pFault;
}








/////////////////////////////////////////////////////////////////////////////
// CFaultDesc

CFaultDesc::CFaultDesc()
{
	m_subject     = 0;
	m_error_code  = 0;
	m_faultLevel  = MAJOR_ERROR_LEVEL;
	m_processName = eProcessTypeInvalid;
	m_faultId     = 0;
}

/////////////////////////////////////////////////////////////////////////////
CFaultDesc::CFaultDesc( const BYTE subject, const DWORD error, const BYTE error_level, const eProcessType processName )
{
	m_subject     = subject;
	m_error_code  = error;
	m_faultLevel  = error_level;
	m_processName = processName;
	m_faultId     = 0;
}

/////////////////////////////////////////////////////////////////////////////
CFaultDesc::~CFaultDesc()
{
}


/////////////////////////////////////////////////////////////////////////////
// CFaultDesc Serialization

void CFaultDesc::Serialize(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	rOstr <<  (WORD)m_subject     << " ";
	rOstr <<  (WORD)m_faultLevel  << " ";
	rOstr <<  m_error_code        << " ";
	rOstr <<  (DWORD)m_processName << " ";
	rOstr <<  m_faultId	          << " ";
}

/////////////////////////////////////////////////////////////////////////////
// CFaultDesc Deserialization

void CFaultDesc::DeSerialize(CIstrStream& rIstr)
{
	// assuming format = OPERATOR_MCMS
	WORD tmp;

	rIstr >> tmp;
	m_subject = (BYTE)tmp;

	rIstr >> tmp;
	m_faultLevel = (BYTE)tmp;

	rIstr >> m_error_code;

    DWORD tmpDword = 0xffffffff;
	rIstr >> tmpDword ;
	m_processName = (eProcessType)tmpDword;

	rIstr >> m_faultId;
}

/////////////////////////////////////////////////////////////////////////////
// CFaultDesc Serialization
char* CFaultDesc::SerializeString(const WORD sizeType) const
{
	COstrStream* pOstr = NULL;
	pOstr = new COstrStream();

	if ( sizeType )
		SerializeLong( *pOstr);
	else
		Serialize( *pOstr );

	int   nMsgLen = pOstr->str().length();
	if( nMsgLen >= SIZE_STREAM )
		nMsgLen = SIZE_STREAM-1;

	char* pszMsg = new char[nMsgLen+1];

	memset(pszMsg,' ',nMsgLen);
	memcpy(pszMsg,pOstr->str().c_str(),nMsgLen);
	pszMsg[nMsgLen]='\0';

	PDELETE(pOstr);

	return pszMsg;
}

/////////////////////////////////////////////////////////////////////////////
void CFaultDesc::DeSerializeString( char* pszMsg )
{
	// assuming format = OPERATOR_MCMS

	CIstrStream*   pIstr;
	pIstr = new CIstrStream(pszMsg);

	DeSerialize( *pIstr );

	PDELETE(pIstr);
}

/////////////////////////////////////////////////////////////////////////////
// CFaultDesc Serialization for display
void CFaultDesc::SerializeLong(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	rOstr <<  "SUBJECT = " << GetSubjectAsString() << ";  ";

	switch (m_faultLevel) {
		case  MAJOR_ERROR_LEVEL:{
			rOstr <<  "LEVEL = MAJOR_ERROR;  ";
			break;
		}
//		case  MINOR_ERROR_LEVEL:{
//			rOstr <<  "LEVEL = MINOR_ERROR;  ";
//			break;
//		}
		case  STARTUP_ERROR_LEVEL:{
			rOstr <<  "LEVEL = STARTUP;  ";
			break;
		}
		case  SYSTEM_MESSAGE:{
			rOstr <<  "LEVEL = SYSTEM_MESSAGE;  ";
			break;
		}
	}

	rOstr << GetAlarmName(m_error_code) << "  ";
}

/////////////////////////////////////////////////////////////////////////////
void CFaultDesc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pFaultNode = pFatherNode->AddChildNode("FAULT_DESC_STARTUP");
	SerializeXmlCommon(pFaultNode);
}

/////////////////////////////////////////////////////////////////////////////
void CFaultDesc::SerializeXmlCommon(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pFaultCommonNode = pFatherNode->AddChildNode("FAULT_DESC_PARAMS");

	if( pFaultCommonNode )
	{
		pFaultCommonNode->AddChildNode("FAULT_SUBJECT",m_subject,FAULT_SUBJECT_ENUM);
		pFaultCommonNode->AddChildNode("FAULT_LEVEL",m_faultLevel,FAULT_LEVEL_ENUM);

		CXMLDOMElement *pFaultStatusNode = pFaultCommonNode->AddChildNode("ERROR_CODE");
		if( pFaultStatusNode )
		{
			pFaultStatusNode->AddChildNode("ID",m_error_code);
			pFaultStatusNode->AddChildNode("DESCRIPTION",GetAlarmName(m_error_code));
		}
		
		pFaultCommonNode->AddChildNode("PROCESS_NAME",(int)m_processName,PROCESS_NAME_ENUM);
		pFaultCommonNode->AddChildNode("FAULT_ID",m_faultId);
	}
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CFaultDesc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CFaultDesc::DeSerializeXmlCommon(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement *pChildNode = NULL;
	
	GET_CHILD_NODE(pActionNode, "FAULT_DESC_PARAMS", pChildNode);

	if( pChildNode ) {
		GET_VALIDATE_CHILD(pChildNode,"FAULT_SUBJECT",&m_subject,FAULT_SUBJECT_ENUM);
		GET_VALIDATE_CHILD(pChildNode,"FAULT_LEVEL",&m_faultLevel,FAULT_LEVEL_ENUM);

		CXMLDOMElement *pErrorNode = NULL;
		GET_CHILD_NODE(pChildNode, "ERROR_CODE", pErrorNode);

		if( pErrorNode ) {
			GET_VALIDATE_CHILD(pErrorNode,"ID",&m_error_code,_0_TO_DWORD);
		}
        
		GET_VALIDATE_CHILD(pChildNode,"PROCESS_NAME",(int*)&m_processName,PROCESS_NAME_ENUM);

		GET_VALIDATE_CHILD(pChildNode,"FAULT_ID",&m_faultId,_0_TO_DWORD);
	}

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
const char* CFaultDesc::GetSubjectAsString() const
{
	switch (m_subject) {
		case  FAULT_STARTUP_SUBJECT:
			return "STARTUP";
		case  FAULT_ASSERT_SUBJECT:
			return "ASSERT";
		case  FAULT_FILE_SUBJECT:
			return "FILE";
		case  FAULT_CARD_SUBJECT:
			return "CARD";
		case  FAULT_RESERVATION_SUBJECT:
			return "RESERVATION";
		case  FAULT_MEETING_ROOM_SUBJECT:
			return "MEETING ROOM";
		case  FAULT_CONFERENCE_SUBJECT:
			return "CONFERENCE";
		case  FAULT_GENERAL_SUBJECT:
			return "GENERAL";
		case  FAULT_EXCEPTION_SUBJECT:
			return "EXCEPTION";
		case  FAULT_DONGLE_SUBJECT:
			return "DONGLE";
		case  FAULT_UNIT_SUBJECT:
			return "UNIT";
		case  FAULT_MPL_SUBJECT:
			return "MPL";
	}
	return "UNKNOWN!!!";
}
/////////////////////////////////////////////////////////////////////////////

const char* CFaultDesc::GetLevelAsString() const
{
	switch (m_faultLevel) {
		case  MAJOR_ERROR_LEVEL:{
			return "MAJOR_ERROR";
		}
		case  STARTUP_ERROR_LEVEL:{
			return "STARTUP";
			break;
		}
		case  SYSTEM_MESSAGE:{
			return "SYSTEM_MESSAGE";

		default:
			return "UNKNOWN!!!";

		}
	}

	return "UNKNOWN!!!";
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CFaultDesc::GetSubject () const
{
	return m_subject;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultDesc::SetSubject(const BYTE subject)
{
	m_subject = subject;
}


/////////////////////////////////////////////////////////////////////////////
BYTE  CFaultDesc::GetFaultLevel () const
{
	return m_faultLevel;
}

/////////////////////////////////////////////////////////////////////////////
eProcessType  CFaultDesc::GetProcessName () const
{
	return m_processName;
}

////////////////////////////////////////////////////////////////////////////
DWORD CFaultDesc::GetFaultId() const
{
	return m_faultId;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultDesc::SetFaultLevel(const BYTE faultLevel)
{
	m_faultLevel = faultLevel;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CFaultDesc::GetErrorCode () const
{
	return m_error_code;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultDesc::SetErrorCode(const DWORD  error)
{
	m_error_code = error;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultDesc::SetProcessName(const eProcessType  processName)
{
	m_processName = processName;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultDesc::SetFaultId(const DWORD  fid)
{
	m_faultId = fid;
}


/////////////////////////////////////////////////////////////////////////////
// CFaultCardDesc
// Vasily - ?
CFaultCardDesc::CFaultCardDesc()//:CFaultDesc()
{
	m_boardId = 0;
	m_unitId  = 0;
	m_genMes[0] = '\0';
	m_cardType = 0;
}

/////////////////////////////////////////////////////////////////////////////
CFaultCardDesc::CFaultCardDesc( const BYTE subject, const DWORD error,
					const BYTE  error_level, const WORD boardId, const WORD unitId,
					const WORD cardType, const char* pszMes, const eProcessType processName )
{
	m_subject     = subject;
	m_error_code  = error;
	m_faultLevel  = error_level;
	m_processName = processName;
	
	m_boardId  = boardId;
	m_unitId   = unitId;
	m_cardType = cardType;
	strncpy(m_genMes,pszMes,sizeof(m_genMes) - 1);
    m_genMes[sizeof(m_genMes) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultCardDesc::~CFaultCardDesc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaultCardDesc Serialization
void CFaultCardDesc::Serialize(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::Serialize(rOstr);
	rOstr <<  m_boardId  << " ";
	rOstr <<  m_unitId   << " ";
	rOstr <<  m_cardType  << " ";
	rOstr <<  m_genMes  << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void CFaultCardDesc::SerializeLong(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::SerializeLong(rOstr);
	rOstr << "BoardId = "     << m_boardId  << " ;   ";
	rOstr << "UnitId = "      << m_unitId   << " ";
	rOstr << "CardType = "    << m_cardType << " ";
	rOstr << "Description = " << m_genMes  << "\n";
}

/////////////////////////////////////////////////////////////////////////////
// CFaultCardDesc Deserialization
void CFaultCardDesc::DeSerialize(CIstrStream& rIstr)
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::DeSerialize(rIstr);
	rIstr >> m_boardId;
	rIstr >> m_unitId;
	rIstr >> m_cardType;

	rIstr.ignore(1);
	rIstr.getline(m_genMes,GENERAL_MES_LEN+1,'\n');
}

/////////////////////////////////////////////////////////////////////////////
void CFaultCardDesc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pFaultNode = pFatherNode->AddChildNode("FAULT_DESC_CARD");
	SerializeXmlCommon(pFaultNode);

	pFaultNode->AddChildNode("BOARD_NUMBER",m_boardId,BOARD_ENUM);
	pFaultNode->AddChildNode("UNIT_NUMBER",m_unitId,UNIT_ENUM);
	pFaultNode->AddChildNode("FAULT_TYPE",m_cardType,FAULT_TYPE_ENUM);
	pFaultNode->AddChildNode("GENERAL_MESSAGE",m_genMes);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CFaultCardDesc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"BOARD_NUMBER",&m_boardId,BOARD_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"UNIT_NUMBER",&m_unitId,UNIT_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"FAULT_TYPE",&m_cardType,FAULT_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"GENERAL_MESSAGE",m_genMes,_0_TO_GENERAL_MES_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CFaultCardDesc::GetBoardId () const
{
	return m_boardId;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultCardDesc::SetBoardId(const WORD boardId)
{
	m_boardId = boardId;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CFaultCardDesc::GetUnitId () const
{
	return m_unitId;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultCardDesc::SetUnitId(const WORD unitId)
{
	m_unitId = unitId;
}


/////////////////////////////////////////////////////////////////////////////
WORD  CFaultCardDesc::GetCardType() const
{
	return m_cardType;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultCardDesc::SetCardType(const WORD cardType)
{
	m_cardType = cardType;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CFaultCardDesc::GetMessage () const
{
	return m_genMes;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultCardDesc::SetMessage(const char*  mes)
{
	strncpy(m_genMes, mes, sizeof(m_genMes) - 1);
	m_genMes[sizeof(m_genMes) - 1]='\0';
}


/////////////////////////////////////////////////////////////////////////////
// CFaultAssertDesc
// Vasily - ?
CFaultAssertDesc::CFaultAssertDesc()//:CFaultDesc()
{
	m_fileName[0] = '\0';
	m_lineNumber  = 0;
	m_assertCode  = 0;
	m_assertstring[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultAssertDesc::CFaultAssertDesc( const BYTE subject, const DWORD error,
				const BYTE  error_level, const char* fileName, const WORD lineNumber,
				const DWORD assertCode, const char* assertstr, const eProcessType processName )
{
	m_subject     = subject;
	m_error_code  = error;
	m_faultLevel  = error_level;
	m_processName = processName;
	
	strncpy(m_fileName, fileName,sizeof(m_fileName) - 1);
	m_fileName[sizeof(m_fileName) - 1] = '\0';
	m_lineNumber = lineNumber;
	m_assertCode = assertCode;
	strncpy(m_assertstring, assertstr,sizeof(m_assertstring) - 1);
	m_assertstring[sizeof(m_assertstring) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultAssertDesc::~CFaultAssertDesc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaultAssertDesc Serialization
void CFaultAssertDesc::Serialize(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::Serialize(rOstr);
	rOstr <<  m_fileName     << " ";
	rOstr <<  m_lineNumber   << " ";
	rOstr <<  m_assertCode   << " ";
	rOstr <<  m_assertstring << " ";
}

/////////////////////////////////////////////////////////////////////////////
void CFaultAssertDesc::SerializeLong(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::SerializeLong(rOstr);
	rOstr << "File  "         << m_fileName    << " ;   ";
	rOstr << "Line number = " << m_lineNumber  << " ;   ";
	rOstr << "Assert code = " << m_assertCode  << " ";
	rOstr <<  m_assertstring  << " ";
}

/////////////////////////////////////////////////////////////////////////////
// CFaultAssertDesc Deserialization
void CFaultAssertDesc::DeSerialize(CIstrStream& rIstr)
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::DeSerialize(rIstr);
	rIstr.ignore(1);
	rIstr.getline(m_fileName,NEW_FILE_NAME_LEN+1,' ');
	rIstr >> m_lineNumber;
	rIstr >> m_assertCode;
	rIstr.ignore(1);
	rIstr.getline(m_assertstring,EXCEPT_HANDL_MES_LEN+1,' ');
}

/////////////////////////////////////////////////////////////////////////////
void CFaultAssertDesc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pFaultNode = pFatherNode->AddChildNode("FAULT_DESC_ASSERT");
	SerializeXmlCommon(pFaultNode);

	pFaultNode->AddChildNode("FILE_NAME",m_fileName);
	pFaultNode->AddChildNode("FILE_LINE_NUMBER",m_lineNumber);
	pFaultNode->AddChildNode("ASSERT_RESULT_CODE",m_assertCode);
	pFaultNode->AddChildNode("ASSERT_DESCRIPTION",m_assertstring);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CFaultAssertDesc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"FILE_NAME",m_fileName,_1_TO_NEW_FILE_NAME_LENGTH);
//	GET_VALIDATE_CHILD(pActionNode,"FILE_NAME",m_fileName,_1_TO_FILE_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"FILE_LINE_NUMBER",&m_lineNumber,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"ASSERT_RESULT_CODE",&m_assertCode,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"ASSERT_DESCRIPTION",m_assertstring,_1_TO_EXCEPT_HANDL_MES_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CFaultAssertDesc::GetLineNumber () const
{
	return m_lineNumber;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultAssertDesc::SetLineNumber(const DWORD lineNumber)
{
	m_lineNumber = lineNumber;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CFaultAssertDesc::GetAssertCode () const
{
	return m_assertCode;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultAssertDesc::SetAssertCode(const DWORD assertCode)
{
	m_assertCode = assertCode;
}


/////////////////////////////////////////////////////////////////////////////
const char*  CFaultAssertDesc::GetFileName () const
{
	return m_fileName;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultAssertDesc::SetFileName(const char*  name)
{
	strncpy(m_fileName, name, sizeof(m_fileName) - 1);
	m_fileName[sizeof(m_fileName) - 1]='\0';
}

/////////////////////////////////////////////////////////////////////////////
const char*  CFaultAssertDesc::GetAssertString () const
{
	return m_assertstring;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultAssertDesc::SetAssertString(const char*  assertstr)
{;
	strncpy(m_assertstring, assertstr, sizeof(m_assertstring) - 1);
	m_assertstring[sizeof(m_assertstring) -1]='\0';
}


/////////////////////////////////////////////////////////////////////////////
// CFaultFileDesc
// Vasily - ?
CFaultFileDesc::CFaultFileDesc()//:CFaultDesc()
{
	m_fileType    = 0;
	m_fileName[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultFileDesc::CFaultFileDesc( const BYTE subject, const DWORD error, const BYTE  error_level,
					            const BYTE fileType, const char* fileName, const eProcessType processName )
{
	m_subject     = subject;
	m_error_code  = error;
	m_faultLevel  = error_level;
	m_processName = processName;

	m_fileType = fileType;
	strncpy(m_fileName,fileName,sizeof(m_fileName) - 1);
	m_fileName[sizeof(m_fileName) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultFileDesc::~CFaultFileDesc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaultFileDesc Serialization
void CFaultFileDesc::Serialize(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::Serialize(rOstr);
	rOstr <<  (WORD)m_fileType  << " ";
	rOstr <<  m_fileName  << " ";
}

/////////////////////////////////////////////////////////////////////////////
void CFaultFileDesc::SerializeLong(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::SerializeLong(rOstr);
	rOstr <<  "File - " <<  GetFileTypeAsString()  << "  ";

	rOstr <<  "File name - " << m_fileName << " ";
}

/////////////////////////////////////////////////////////////////////////////
// CFaultFileDesc Deserialization
void CFaultFileDesc::DeSerialize(CIstrStream& rIstr)
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::DeSerialize(rIstr);
	WORD tmp;
	rIstr >> tmp;
	m_fileType = (BYTE)tmp;
	rIstr.ignore(1);
	rIstr.getline(m_fileName,FILE_NAME_LEN+1,' ');
}

/////////////////////////////////////////////////////////////////////////////
void CFaultFileDesc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pFaultNode = pFatherNode->AddChildNode("FAULT_DESC_FILE");
	SerializeXmlCommon(pFaultNode);

	pFaultNode->AddChildNode("FILE_TYPE",m_fileType,FAULT_FILE_ENUM);
	pFaultNode->AddChildNode("FILE_NAME",m_fileName);
}


/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CFaultFileDesc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"FILE_TYPE",&m_fileType,FAULT_FILE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"FILE_NAME",m_fileName,_1_TO_FILE_NAME_LENGTH);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
const char* CFaultFileDesc::GetFileTypeAsString() const
{
	switch (m_fileType) {
		case  VERSION_CONFIGURATION:
			return "VCOstrStream& rOstr) constERSION CONFIGURATION ";
		case  CARDS_CONFIGURATION:
			return "CARDS CONFIGURATION ";
		case  NETWORK_CONFIGURATION:
			return "NETWORK CONFIGURATION ";
		case  OPERATORS_CONFIGURATION:
			return "OPERATORS CONFIGURATION ";
		case  SYSTEM_CONFIGURATION:
			return "SYSTEM CONFIGURATION ";
		case  LOGGING:
			return "LOGGING ";
		case  RESERVATION_DATABASE:
			return "RESERVATION DATABASE ";
		case  MESSAGE_CONFIGURATION:
			return "MESSAGE CONFIGURATION ";
		case  MEETING_ROOM_DATABASE:
			return "MEETING ROOM DATABASE ";
	}
	return "UNKNOWN!!! ";
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CFaultFileDesc::GetFileType () const
{
	return m_fileType;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultFileDesc::SetFileType(const BYTE fileType)
{
	m_fileType = fileType;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CFaultFileDesc::GetfileName () const
{
	return m_fileName;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultFileDesc::SetfileName(const char* name)
{
	strncpy(m_fileName, name, sizeof(m_fileName) - 1);
	m_fileName[sizeof(m_fileName) - 1]='\0';
}



/////////////////////////////////////////////////////////////////////////////
// CFaultReservationDesc
// Vasily - ?
CFaultReservationDesc::CFaultReservationDesc()//:CFaultDesc()
{
	m_confId  = 0xFFFFFFFF;
	m_partyId = 0xFFFFFFFF;
	m_H243confName[0]   = '\0';
	m_H243_partyName[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultReservationDesc::CFaultReservationDesc( const BYTE subject, const DWORD error, const BYTE  error_level,
						const DWORD confId, const DWORD partyId, const char* confName, const char* partyName,
						const eProcessType processName )
{
	m_subject     = subject;
	m_error_code  = error;
	m_faultLevel  = error_level;
	m_processName = processName;

	m_confId  = confId;
	m_partyId = partyId;
	strncpy(m_H243confName,confName,sizeof(m_H243confName) - 1);
    m_H243confName[sizeof(m_H243confName) - 1] = '\0';
	strncpy(m_H243_partyName,partyName,sizeof(m_H243_partyName) - 1);
    m_H243_partyName[sizeof(m_H243_partyName) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultReservationDesc::~CFaultReservationDesc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaultReservationDesc Serialization
void CFaultReservationDesc::Serialize(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::Serialize(rOstr);
	rOstr <<  m_confId  << " ";
	rOstr <<  m_partyId  << " ";
	rOstr <<  m_H243confName  << " ";
	rOstr <<  m_H243_partyName  << " ";
}

/////////////////////////////////////////////////////////////////////////////
void CFaultReservationDesc::SerializeLong(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::SerializeLong(rOstr);
	rOstr << "ConferenceId = "   << m_confId  << " ;   ";
	rOstr << "ConferenceName - " << m_H243confName  << " ;   ";

	if ( m_partyId != 0xFFFFFFFF )
	{
		rOstr << "PartyId = "   << m_partyId  << " ;   ";
		rOstr << "PartyName - " << m_H243_partyName  << " ;   ";
	}
	else
		rOstr << " ";
}

/////////////////////////////////////////////////////////////////////////////
// CFaultReservationDesc Deserialization
void CFaultReservationDesc::DeSerialize(CIstrStream& rIstr)
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::DeSerialize(rIstr);
	rIstr >> m_confId;
	rIstr >> m_partyId;
	rIstr.ignore(1);

	rIstr.getline(m_H243confName,H243_NAME_LEN+1,'\n');
	rIstr.getline(m_H243_partyName,H243_NAME_LEN+1,'\n');
}

/////////////////////////////////////////////////////////////////////////////
void CFaultReservationDesc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pFaultNode = pFatherNode->AddChildNode("FAULT_DESC_RESERVATION");
	SerializeXmlCommon(pFaultNode);

	CXMLDOMElement *pChildNode = NULL;
	pChildNode = pFaultNode->AddChildNode("CONF_DATA");
	if( pChildNode ) {
		pChildNode->AddChildNode("ID",m_confId);
		pChildNode->AddChildNode("NAME",m_H243confName);

		pChildNode = pFaultNode->AddChildNode("PARTY_DATA");
		if( pChildNode ) {
			pChildNode->AddChildNode("ID",m_partyId);
			pChildNode->AddChildNode("NAME",m_H243_partyName);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CFaultReservationDesc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	CXMLDOMElement *pChildNode = NULL;
	GET_CHILD_NODE(pActionNode, "CONF_DATA", pChildNode);
	if( pChildNode!=NULL ) {

		GET_VALIDATE_CHILD(pChildNode,"ID",&m_confId,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChildNode,"NAME",m_H243confName,_1_TO_H243_NAME_LENGTH);
	}

	GET_CHILD_NODE(pActionNode, "PARTY_DATA", pChildNode);
	if( pChildNode!=NULL )
	{
		GET_VALIDATE_CHILD(pChildNode,"ID",&m_partyId,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pChildNode,"NAME",m_H243_partyName,_1_TO_H243_NAME_LENGTH);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CFaultReservationDesc::GetConfId () const
{
	return m_confId;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultReservationDesc::SetConfId(const DWORD confId)
{
	m_confId = confId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CFaultReservationDesc::GetPartyId () const
{
	return m_partyId;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultReservationDesc::SetPartyId(const DWORD partyId)
{
	m_partyId = partyId;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CFaultReservationDesc::GetConfName () const
{
	return m_H243confName;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultReservationDesc::SetConfName(const char* name)
{
	strncpy(m_H243confName, name, sizeof(m_H243confName) - 1);
	m_H243confName[sizeof(m_H243confName) - 1]='\0';
}

/////////////////////////////////////////////////////////////////////////////
const char*  CFaultReservationDesc::GetPartyName () const
{
	return m_H243_partyName;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultReservationDesc::SetPartyName(const char* name)
{
	strncpy(m_H243_partyName, name, sizeof(m_H243_partyName) - 1);
	m_H243_partyName[sizeof(m_H243_partyName) - 1]='\0';
}




/////////////////////////////////////////////////////////////////////////////
// CFaultExceptHandlDesc
// Vasily - ?
CFaultExceptHandlDesc::CFaultExceptHandlDesc()//:CFaultDesc()
{
	m_excpHndlMes[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultExceptHandlDesc::CFaultExceptHandlDesc( const BYTE subject, const DWORD error, const BYTE  error_level,
                                              const char* pszMes, const eProcessType processName )
{
	m_subject     = subject;
	m_error_code  = error;
	m_faultLevel  = error_level;
	m_processName = processName;

	strncpy(m_excpHndlMes,pszMes,sizeof(m_excpHndlMes) - 1);
    m_excpHndlMes[sizeof(EXCEPT_HANDL_MES_LEN) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultExceptHandlDesc::~CFaultExceptHandlDesc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaultExceptHandlDesc Serialization
void CFaultExceptHandlDesc::Serialize(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::Serialize(rOstr);
	rOstr <<  m_excpHndlMes  << "\n";
}

/////////////////////////////////////////////////////////////////////////////
// CFaultExceptHandlDesc Deserialization
void CFaultExceptHandlDesc::DeSerialize(CIstrStream& rIstr)
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::DeSerialize(rIstr);
	rIstr.ignore(1);
	rIstr.getline(m_excpHndlMes,EXCEPT_HANDL_MES_LEN+1,'\n');
}

/////////////////////////////////////////////////////////////////////////////
void CFaultExceptHandlDesc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pFaultNode = pFatherNode->AddChildNode("FAULT_DESC_EXCEPTION");
	SerializeXmlCommon(pFaultNode);

	pFaultNode->AddChildNode("EXCEPTION_MESSAGE",m_excpHndlMes);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CFaultExceptHandlDesc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"EXCEPTION_MESSAGE",m_excpHndlMes,_1_TO_EXCEPT_HANDL_MES_LENGTH);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
const char*  CFaultExceptHandlDesc::GetMessage () const
{
	return m_excpHndlMes;
}


/////////////////////////////////////////////////////////////////////////////
void  CFaultExceptHandlDesc::SetMessage(const char*  mes)
{
	strncpy(m_excpHndlMes, mes, sizeof(m_excpHndlMes) - 1);
	m_excpHndlMes[sizeof(m_excpHndlMes) - 1]='\0';
}



/////////////////////////////////////////////////////////////////////////////
// CFaultGeneralDesc
// Vasily - ?
CFaultGeneralDesc::CFaultGeneralDesc()//:CFaultDesc()
{
	m_genMes[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultGeneralDesc::CFaultGeneralDesc( const BYTE subject, const DWORD error, const BYTE  error_level,
                                      const char* pszMes, const eProcessType processName )
{
	m_subject     = subject;
	m_error_code  = error;
	m_faultLevel  = error_level;
	m_processName = processName;
	
	strncpy(m_genMes,pszMes,sizeof(m_genMes) - 1);
    m_genMes[sizeof(m_genMes) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultGeneralDesc::~CFaultGeneralDesc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaultGeneralDesc Serialization
void CFaultGeneralDesc::Serialize(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::Serialize(rOstr);
	rOstr <<  m_genMes  << "\n";
}

/////////////////////////////////////////////////////////////////////////////
// CFaultGeneralDesc Deserialization
void CFaultGeneralDesc::DeSerialize(CIstrStream& rIstr)
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::DeSerialize(rIstr);
	rIstr.ignore(1);
	rIstr.getline(m_genMes,GENERAL_MES_LEN+1,'\n');
}

/////////////////////////////////////////////////////////////////////////////
void CFaultGeneralDesc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pFaultNode = pFatherNode->AddChildNode("FAULT_DESC_GENERAL");
	SerializeXmlCommon(pFaultNode);

	pFaultNode->AddChildNode("GENERAL_MESSAGE",m_genMes);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CFaultGeneralDesc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"GENERAL_MESSAGE",m_genMes,_0_TO_GENERAL_MES_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CFaultGeneralDesc::GetMessage () const
{
	return m_genMes;
}


/////////////////////////////////////////////////////////////////////////////
void  CFaultGeneralDesc::SetMessage(const char*  mes)
{
	strncpy(m_genMes, mes, sizeof(m_genMes) - 1);
	m_genMes[sizeof(m_genMes) - 1]='\0';
}



/////////////////////////////////////////////////////////////////////////////
// CFaultUnitDesc
CFaultUnitDesc::CFaultUnitDesc()//:CFaultDesc()
{
	m_boardId = 0;
	m_unitId  = 0;
	m_unitType	= 0;
	m_genMes[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultUnitDesc::CFaultUnitDesc( const BYTE subject, const DWORD error,
					const BYTE error_level, const WORD boardId, const WORD unitId,
					const WORD unitType, const char* pszMes, const eProcessType processName )
{
	m_subject     = subject;
	m_error_code  = error;
	m_faultLevel  = error_level;
	m_processName = processName;
	
	m_boardId = boardId;
	m_unitId  = unitId;
	m_unitType = unitType;
	strncpy(m_genMes,pszMes,sizeof(m_genMes) - 1);
    m_genMes[sizeof(m_genMes) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultUnitDesc::~CFaultUnitDesc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaultUnitDesc Serialization
void CFaultUnitDesc::Serialize(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::Serialize(rOstr);
	rOstr <<  m_boardId  << " ";
	rOstr <<  m_unitId  << " ";
	rOstr <<  m_unitType << " ";
	rOstr <<  m_genMes  << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void CFaultUnitDesc::SerializeLong(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::SerializeLong(rOstr);
	rOstr << "BoardId = "     << m_boardId  << " ;   ";
	rOstr << "UnitId = "      << m_unitId   << " ";
	rOstr << "UnitType = "    << m_unitType << " ";
	rOstr << "Description = " << m_genMes  << "\n";
}

/////////////////////////////////////////////////////////////////////////////
// CFaultUnitDesc Deserialization
void CFaultUnitDesc::DeSerialize(CIstrStream& rIstr)
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::DeSerialize(rIstr);
	rIstr >> m_boardId;
	rIstr >> m_unitId;
	rIstr >> m_unitType;

	rIstr.ignore(1);
	rIstr.getline(m_genMes,GENERAL_MES_LEN+1,'\n');
}

/////////////////////////////////////////////////////////////////////////////
void CFaultUnitDesc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pFaultNode = pFatherNode->AddChildNode("FAULT_DESC_UNIT");
	SerializeXmlCommon(pFaultNode);

	pFaultNode->AddChildNode("BOARD_NUMBER",m_boardId,BOARD_ENUM);
	pFaultNode->AddChildNode("UNIT_NUMBER",m_unitId,UNIT_ENUM);
	pFaultNode->AddChildNode("FAULT_TYPE",m_unitType,FAULT_TYPE_ENUM);
	pFaultNode->AddChildNode("GENERAL_MESSAGE",m_genMes);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CFaultUnitDesc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"BOARD_NUMBER",&m_boardId,BOARD_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"UNIT_NUMBER",&m_unitId,UNIT_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"FAULT_TYPE",&m_unitType,FAULT_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"GENERAL_MESSAGE",m_genMes,_0_TO_GENERAL_MES_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CFaultUnitDesc::GetBoardId () const
{
	return m_boardId;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultUnitDesc::SetBoardId(const WORD boardId)
{
	m_boardId = boardId;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CFaultUnitDesc::GetUnitId () const
{
	return m_unitId;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultUnitDesc::SetUnitId(const WORD unitId)
{
	m_unitId = unitId;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CFaultUnitDesc::GetUnitType() const
{
	return m_unitType;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultUnitDesc::SetUnitType(const WORD unitType)
{
	m_unitType = unitType;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CFaultUnitDesc::GetMessage () const
{
	return m_genMes;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultUnitDesc::SetMessage(const char*  mes)
{
	strncpy(m_genMes, mes, sizeof(m_genMes) - 1);
	m_genMes[sizeof(m_genMes) -1]='\0';
}


/////////////////////////////////////////////////////////////////////////////
// CFaultMplDesc
CFaultMplDesc::CFaultMplDesc()//:CFaultDesc()
{
	m_compType	= 0;
	m_genMes[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultMplDesc::CFaultMplDesc( const BYTE subject, const DWORD error, const BYTE  error_level,
                              const WORD compType, const char* pszMes, const eProcessType processName )
{
	m_subject     = subject;
	m_error_code  = error;
	m_faultLevel  = error_level;
	m_processName = processName;
	
	m_compType = compType;
	strncpy(m_genMes,pszMes,sizeof(m_genMes) - 1);
    m_genMes[sizeof(m_genMes) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CFaultMplDesc::~CFaultMplDesc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaultUnitDesc Serialization
void CFaultMplDesc::Serialize(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::Serialize(rOstr);
	rOstr <<  m_compType << " ";
	rOstr <<  m_genMes  << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void CFaultMplDesc::SerializeLong(COstrStream& rOstr) const
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::SerializeLong(rOstr);
	rOstr << "Component Type = " << m_compType << " ";
	rOstr << "Description = "    << m_genMes  << "\n";
}

/////////////////////////////////////////////////////////////////////////////
// CFaultUnitDesc Deserialization
void CFaultMplDesc::DeSerialize(CIstrStream& rIstr)
{
	// assuming format = OPERATOR_MCMS
	CFaultDesc::DeSerialize(rIstr);
	rIstr >> m_compType;

	rIstr.ignore(1);
	rIstr.getline(m_genMes,GENERAL_MES_LEN+1,'\n');
}

/////////////////////////////////////////////////////////////////////////////
void CFaultMplDesc::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pFaultNode = pFatherNode->AddChildNode("FAULT_DESC_MPL");
	SerializeXmlCommon(pFaultNode);

	pFaultNode->AddChildNode("FAULT_TYPE",m_compType,FAULT_TYPE_ENUM);
	pFaultNode->AddChildNode("GENERAL_MESSAGE",m_genMes);
}

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_faults_list.xsd
int CFaultMplDesc::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = DeSerializeXmlCommon(pActionNode,pszError,action);
	if( nStatus )
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode,"FAULT_TYPE",&m_compType,FAULT_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"GENERAL_MESSAGE",m_genMes,_0_TO_GENERAL_MES_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CFaultMplDesc::GetCompType() const
{
	return m_compType;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultMplDesc::SetCompType(const WORD compType)
{
	m_compType = compType;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CFaultMplDesc::GetMessage () const
{
	return m_genMes;
}

/////////////////////////////////////////////////////////////////////////////
void  CFaultMplDesc::SetMessage(const char*  mes)
{
	strncpy(m_genMes, mes, sizeof(m_genMes) - 1);
	m_genMes[sizeof(m_genMes) -1]='\0';
}

