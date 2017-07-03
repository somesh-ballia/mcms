// SerializeObject.cpp

#include "SerializeObject.h"

#include <ostream>
#include <errno.h>
#include <iomanip>
#include <stdlib.h>
#include <fstream>

#include "OsTask.h"
#include "zlib.h"
#include "psosxml.h"
#include "XmlApi.h"
#include "Trace.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "FaultsDefines.h"
#include "ObjString.h"
#include "TraceStream.h"
#include "OsFileIF.h"
#include "ProcessBase.h"
#include "EncodingConvertor.h"
#include "UnicodeDefines.h"
#include "StructTm.h"


class CHlogList;

extern char* FailReadingFileOperationTypeToString(eFailReadingFileOperationType operationType);
extern char* FailReadingFileActiveAlarmTypeToString(eFailReadingFileActiveAlarmType activeAlarmType);

// Static Private
DWORD CSerializeObject::s_max_xml_file_size = 2 * 1024 * 1024;

// Static
DWORD CSerializeObject::GetMaxXMLFileSize(void)
{
    return s_max_xml_file_size;
}

// Static
DWORD CSerializeObject::SetMaxXMLFileSize(DWORD size)
{
    DWORD old_size = s_max_xml_file_size;
    s_max_xml_file_size = size;
    return old_size;
}

CSerializeObject::CSerializeObject()
{
	m_pRequestfunction 	= NULL;
	m_updateCounter 	= 0;
	m_apiFormat = eXmlApi;
}

CSerializeObject::~CSerializeObject()
{}

//////////////////////////////////////////////////////////////////////
void CSerializeObject::SetRequestFunction(HANDLE_REQUEST function)
{
	m_pRequestfunction = function;
}
//////////////////////////////////////////////////////////////////////
void CSerializeObject::Serialize(WORD format,CSegment& rSeg)
{
	PASSERTMSG(1,"CSerializeObject::Serialize should never be called");
}

//////////////////////////////////////////////////////////////////////
void CSerializeObject::DeSerialize(WORD format,CSegment& rSeg)
{
	PASSERTMSG(1,"CSerializeObject::DeSerialize should never be called");
}


/////////////////////////////////////////////////////////////////////////////
STATUS CSerializeObject::WriteXmlFile(const char * file_name, const char * root_name ) const
{
	CXMLDOMElement* pXMLRootElement =  new CXMLDOMElement;
	pXMLRootElement->set_nodeName(root_name);

	STATUS status = WriteXmlFile(file_name, pXMLRootElement);

	PDELETE(pXMLRootElement);

	return status;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CSerializeObject::WriteXml_File(const char * file_name, const char * root_name )
{
	CXMLDOMElement* pXMLRootElement =  new CXMLDOMElement;
	pXMLRootElement->set_nodeName(root_name);

	STATUS status = WriteXmlFile(file_name, pXMLRootElement);////1

	PDELETE(pXMLRootElement);

	return status;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CSerializeObject::WriteXmlFile(const char * file_name) const
{
	CXMLDOMElement* pXMLRootElement = NULL; /*new CXMLDOMElement;*/

	STATUS status = WriteXmlFile(file_name, pXMLRootElement);

	PDELETE(pXMLRootElement);

	return status;
}
//////////////////////////////////////////////////////////////////
STATUS CSerializeObject::WriteXml_File(const char * file_name)
{
	CXMLDOMElement* pXMLRootElement = NULL; /*new CXMLDOMElement;*/

	STATUS status = WriteXmlFile(file_name, pXMLRootElement);

	PDELETE(pXMLRootElement);

	return status;
}

STATUS CSerializeObject::WriteXmlFile(const char *file_name,
                                      CXMLDOMElement*& pXMLRootElement) const
{
	SerializeXml(pXMLRootElement);
	STATUS status = STATUS_OK;

    ofstream file;
    file.open (file_name);
    if (file.is_open())
    {
        status = pXMLRootElement->DumpDataAsStringImpl(file,0,TRUE);

        file.flush();

        file.close();

        CProcessBase *pProcess = (CProcessBase*)CProcessBase::GetProcess();

        return status;
    }
    else
    {
        PASSERTMSG(TRUE, "open file failed");
    }
	return STATUS_FAIL;
}

STATUS CSerializeObject::ReadXml_Files(const char* fileName,
                                       eFailReadingFileActiveAlarmType activeAlarmType,
	                                     eFailReadingFileOperationType operationType,
	                                     int FileNum,
	                                     int activeAlarmId)
{
	if (NULL == fileName)
		return STATUS_FAIL;

	FILE* infile = fopen(fileName, "r");
	if (NULL == infile)
		return HandleFileErrorOpen(fileName, errno, activeAlarmType, activeAlarmId);

	fclose(infile);

	const int fileSize = GetFileSize(fileName);
	if (-1 == fileSize)
	  return STATUS_FAIL;

	PASSERTSTREAM_AND_RETURN_VALUE((DWORD)fileSize > GetMaxXMLFileSize(),
	    "File too big " << fileName << ": " << fileSize,
	    STATUS_FAIL);

	STATUS status = STATUS_OK;
	CXMLDOMDocument* pCfgRoot = new CXMLDOMDocument;
	BYTE bRes = ParseXMLFile(fileName, pCfgRoot);
	if (FALSE == bRes)
	{
		status = HandleFileErrorParseXml(fileName, activeAlarmType, operationType, activeAlarmId);
		PDELETE(pCfgRoot);
		return status;
	}

	CXMLDOMElement* pXMLRootElement = pCfgRoot->GetRootElement();
  if (NULL != pXMLRootElement)
  {
    ALLOCBUFFER(pszError, ERROR_MESSAGE_LEN);
    status = DeSerialize_Xml(pXMLRootElement,pszError,NULL,FileNum);
    DEALLOCBUFFER(pszError);
  }

	PDELETE(pCfgRoot);

	return status;
}

STATUS CSerializeObject::ReadXmlZipFile( const char * pszZipFileName,
        eFailReadingFileActiveAlarmType activeAlarmType,
        eFailReadingFileOperationType operationType,
        int activeAlarmId )
{
	PTRACE(eLevelInfoNormal,"CSerializeObject::ReadXmlZipFile : - begin");
	if(NULL == pszZipFileName)
		return STATUS_FAIL;

	STATUS status = STATUS_OK;
	int nStatus = STATUS_OK; // for the xml parsing

	FILE* infile = fopen(pszZipFileName, "rb");
	DWORD errnoCode = errno;
	if(NULL == infile)
	{
		status = HandleFileErrorOpen(pszZipFileName, errnoCode, activeAlarmType, activeAlarmId);
		return status;
	}
	fclose(infile);

	z_stream CompressionStream;
	DWORD CHUNK 		= 16384;
	BYTE ZipBuffer		[16384];
	BYTE UnZipBuffer	[16385];

	FILE *ZipFileHandle 	= NULL;

	ZipFileHandle = fopen(pszZipFileName, "rb");

	if(NULL == ZipFileHandle)
		return STATUS_UNZIP_XML_FILE_FAILED;

	CompressionStream.avail_in	= 0;
	CompressionStream.avail_out	= 0;
	CompressionStream.next_in	= NULL;
	CompressionStream.next_out	= NULL;
	CompressionStream.total_in	= 0;
	CompressionStream.total_out	= 0;
	CompressionStream.zalloc 	= (alloc_func)0;
	CompressionStream.zfree 	= (free_func)0;
	CompressionStream.opaque 	= (voidpf)0;

	int lerror = inflateInit(&CompressionStream);
	if (lerror!=Z_OK)
	{
		fclose(ZipFileHandle);
		return STATUS_UNZIP_XML_FILE_FAILED;
	}

	std::string sXml;

	do
	{
        int readNum = fread(ZipBuffer, 1, CHUNK, ZipFileHandle);
        if(ferror(ZipFileHandle))
        {
        	fclose(ZipFileHandle);
        	return STATUS_UNZIP_XML_FILE_FAILED;
        }

        if (0 == readNum)
            break;

        CompressionStream.avail_in = readNum;
        CompressionStream.next_in  = ZipBuffer;

        do
		{
            CompressionStream.avail_out = CHUNK;
            CompressionStream.next_out 	= UnZipBuffer;

            memset(UnZipBuffer, 0, CHUNK + 1);
            lerror = inflate(&CompressionStream, Z_NO_FLUSH);
            switch (lerror)
			{
				case Z_STREAM_ERROR:
					UnZipError(infile, &CompressionStream, "inflate finished with error", "Z_STREAM_ERROR", lerror);
					return STATUS_UNZIP_XML_FILE_FAILED;
				case Z_NEED_DICT:
					UnZipError(infile, &CompressionStream, "inflate finished with error", "Z_NEED_DICT", lerror);
					return STATUS_UNZIP_XML_FILE_FAILED;
				case Z_DATA_ERROR:
					UnZipError(infile, &CompressionStream, "inflate finished with error", "Z_DATA_ERROR", lerror);
					return STATUS_UNZIP_XML_FILE_FAILED;
				case Z_MEM_ERROR:
					UnZipError(infile, &CompressionStream, "inflate finished with error", "Z_MEM_ERROR", lerror);
					return STATUS_UNZIP_XML_FILE_FAILED;
				}

            int have = CHUNK - CompressionStream.avail_out;

			sXml += (char*)UnZipBuffer;

        } while (CompressionStream.avail_out == 0);
    } while (lerror != Z_STREAM_END);

    inflateEnd(&CompressionStream);

    fclose(ZipFileHandle);

    if (sXml.size() < 10)
    	return STATUS_UNZIP_XML_FILE_FAILED;
	PTRACE2(eLevelInfoNormal,"CSerializeObject::ReadXmlZipFile : - sXml:\n", sXml.c_str());

	CXMLDOMDocument* pRoot = new CXMLDOMDocument;
	unsigned char chRes;
	const char* pszXml = sXml.c_str();

	BYTE bRes = pRoot->Parse((const char **)&pszXml);
	PTRACE(eLevelInfoNormal,"CSerializeObject::ReadXmlZipFile : - after pRoot->Parse((const char **)&pszXml)");
	if(SEC_OK != bRes)
	{
		status = HandleFileErrorParseXml(pszZipFileName, activeAlarmType, operationType, activeAlarmId);
		PDELETE(pRoot);
		return status;
	}

	PTRACE(eLevelInfoNormal,"CSerializeObject::ReadXmlZipFile : - before pRoot->GetRootElement()");
	CXMLDOMElement* pXMLRootElement = pRoot->GetRootElement();
	PTRACE(eLevelInfoNormal,"CSerializeObject::ReadXmlZipFile : - after pRoot->GetRootElement()");
	if(NULL != pXMLRootElement)
	{
		ALLOCBUFFER(pszError, ERROR_MESSAGE_LEN);
		status = DeSerializeXml(pXMLRootElement,pszError,NULL);
		DEALLOCBUFFER(pszError);
	}

	PDELETE(pRoot);

	PTRACE(eLevelInfoNormal,"CSerializeObject::ReadXmlZipFile : - end");
	return status;
}

void CSerializeObject::UnZipError(FILE* infile,
                                  void* pCompressionStream,
                                  const char *ErrorMessage,
                                  const char *param,
                                  const DWORD ErrorCode)
{
  const char *errorStr = zError(ErrorCode);
  printf("%s : %s Error Code: %s\r\n", ErrorMessage, param, errorStr);
  deflateEnd((z_stream*) pCompressionStream);

  if (NULL != infile)
    fclose(infile);
}

STATUS CSerializeObject::ReadXmlFile(const char* fname,
                                     eFailReadingFileActiveAlarmType activeAlarmType,
                                     eFailReadingFileOperationType operationType,
                                     int activeAlarmId)
{
	std::ostringstream err;
	STATUS validate =
    CEncodingConvertor::ValidateFile(MCMS_INTERNAL_STRING_ENCODE_TYPE,
                                     fname,
                                     err);

	if (STATUS_OK != validate)
	{
		TRACEWARN << "ValidateFile: " << (fname ? fname : "null")
            	  << ": " << err.str().c_str();
		if (fname == NULL)
			return STATUS_PARSING_XML_FILE_FAILED;
		return HandleFileErrorParseXml(fname, activeAlarmType, operationType, activeAlarmId);
	}

	CXMLDOMDocument root;
	BYTE bRes = ParseXMLFile(fname, &root);
	if (!bRes)
	{
		TRACEWARN << "ParseXMLFile: " << (fname ? fname : "null");
		if (fname == NULL)
			return STATUS_PARSING_XML_FILE_FAILED;
		return HandleFileErrorParseXml(fname, activeAlarmType, operationType, activeAlarmId);
	}

	STATUS status = STATUS_OK;
	CXMLDOMElement* pXMLRootElement = root.GetRootElement();
	if (NULL != pXMLRootElement)
	{
		char buf[ERROR_MESSAGE_LEN] = { '\0' };
		status = DeSerializeXml(pXMLRootElement, buf, NULL);

		if (STATUS_OK != status)
		TRACEWARN << "DeSerializeXml: " << (fname ? fname : "null")
                  << ": " << buf;
	 }

	 return status;
}

STATUS CSerializeObject::HandleFileErrorOpen(const char *fileName,
                                             DWORD errnoCode,
                                             eFailReadingFileActiveAlarmType activeAlarmType,
                                             int activeAlarmId)
{
	STATUS status = STATUS_OK;
	CMedString decription;
	if (ENOENT == errnoCode)
	{
		decription = "XML file does not exist ";
		status = STATUS_FILE_NOT_EXIST;
	}
	else if (EACCES == errnoCode)
	{
		decription = "Not authorized to open XML file ";
		status = STATUS_OPEN_FILE_FAILED;
	}
	else
	{
		decription = "Unknown problem in opening XML file ";
		status = STATUS_UNKNOWN_FILE_FAILURE;
	}
	decription << fileName;
	decription << "; Error no: " << errnoCode;

	TreatFailFileOperation(fileName,
                           status,
                           decription.GetString(),
                           activeAlarmType,
                           eNoAction,
                           activeAlarmId);
	return status;
}

STATUS CSerializeObject::HandleFileErrorParseXml(const char *fileName,
												  eFailReadingFileActiveAlarmType activeAlarmType,
					                              eFailReadingFileOperationType operationType,
					                              int activeAlarmId)
{
	CSmallString decription = "Failed to parse XML file ";
	decription << fileName;
	STATUS status = STATUS_PARSING_XML_FILE_FAILED;

	TreatFailFileOperation(fileName,
                           status,
                           decription.GetString(),
                           activeAlarmType,
                           operationType,
                           activeAlarmId);
	return status;
}

/////////////////////////////////////////////////////////////////////////////
void CSerializeObject::TreatFailFileOperation( const char * fileName,
                                             STATUS status,
                                             const char *decription,
                                             eFailReadingFileActiveAlarmType activeAlarmType,
                                             eFailReadingFileOperationType operationType,
                                             int activeAlarmId )
{
	CProcessBase *pProcess = (CProcessBase*)CProcessBase::GetProcess();
	const string &statusStr = pProcess->GetStatusAsString(status);

	// ===== 1. print to log
	const char *operationStr = ::FailReadingFileOperationTypeToString(operationType);
	const char *aaPolicyStr  = ::FailReadingFileActiveAlarmTypeToString(activeAlarmType);
	TRACESTR(eLevelInfoNormal) << "\n ===== Operation On File Failed! ====="		<< endl
	                       << setw(15) << "File name: "   << fileName 			<< endl
	                       << setw(15) << "Status: "      << statusStr.c_str() 	<< endl
	                       << setw(15) << "Description: " << decription 		<< endl
	                       << setw(15) << "Operation: "   << operationStr 		<< endl
	                       << setw(15) << "ActiveAlarm: " << aaPolicyStr		<< endl ;

	// ===== 2. produce ActiveAlarm
	if ( (eActiveAlarmExternal == activeAlarmType) || (eActiveAlarmInernal == activeAlarmType) )
	{
		bool isExternal = (eActiveAlarmExternal == activeAlarmType);
		pProcess->AddActiveAlarmFromProcess( FAULT_GENERAL_SUBJECT,
		                                     AA_BAD_FILE,
		                                     MAJOR_ERROR_LEVEL,
		                                     decription,
		                                     isExternal,
		                                     isExternal,
		                                     activeAlarmId
		                                   );
	}

	// ===== 3. perform operation
	switch(operationType)
	{
		case eRenameFile:
			if ( IsFileExists(fileName) )
			{
				string originalName = fileName;
				string renamedName  = fileName;
				renamedName += "_error.xml";

				RenameFile(originalName, renamedName);
				TRACESTR(eLevelInfoNormal) << "\nCSerializeObject::TreatFailReadingFile : Error file Renamed - "
				                       << "\nOriginal name: " << fileName
				                       << "; New name: "      << renamedName.c_str();
			}
		break;

		case eRemoveFile:
			if (IsFileExists(fileName))
			{
				// VNGR-22008: W5 / CESL / RMX v7.6 InterOp -
				// RMX lost all meeting rooms, EQ, Conf Templates after upgrade to b93

				// Constructs a name for temporary keeping
				char* dup_fname = strdup(fileName);
				PASSERT_AND_RETURN(!dup_fname);
				const char* bname = basename(dup_fname);
				std::stringstream target;
				CStructTm dtCurrent;
				SystemGetTime(dtCurrent);


				target << MCU_OUTPUT_TMP_DIR << bname << "_" << dtCurrent.m_day << "_" << dtCurrent.m_mon << "_" << dtCurrent.m_year;
				free(dup_fname);

			// Copies bad file to MCU_TMP_DIR/fileName if it is not exist
      if (!IsFileExists(target.str()))
        CopyFile(fileName, target.str());

      // Deletes bad file
      DeleteFile(fileName);

      TRACEINTOFUNC << "Deleted file " << fileName
                    << " is copied to " << target.str() << " for debug";
			}
		break;

		case eNoAction:
		break;

		default:
			PASSERTMSG(operationType + 100, "Illegal operation type(operationType + 100)");
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSerializeObject::Dump(std::ostream& ostr) const
{
	CXMLDOMElement* pXMLRootElement =  new CXMLDOMElement;
	pXMLRootElement->set_nodeName(NameOf());

	SerializeXml(pXMLRootElement);

	CStrArray strArray;
	WORD CompressionLevel = 0;
	if( pXMLRootElement->DumpDataAsStringArrayEx(strArray,CompressionLevel, TRUE,TRUE) == SEC_OK )
	{

		DWORD dwStrLen = 0;
		for ( int i=0; i<strArray.GetNumOfAllocatedBuffers(); i++ )
		{
			if( strArray[i] != NULL )
				ostr << strArray[i];
			else
				break;
		}
	}

	PDELETE(pXMLRootElement);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CSerializeObject::SetUpdateCounter(DWORD counter)
{
	m_updateCounter = counter;
}

////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CSerializeObject::GetUpdateCounter() const
{
	return m_updateCounter ;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void  CSerializeObject::SetApiFormat(eApiFormat format)
{
	m_apiFormat = format;
	TRACESTR(eLevelInfoNormal) << "inside CSerializeObject::SetApiFormat, m_apiFormat=" << m_apiFormat;
}
////////////////////////////////////////////////////////////////////////////////////////////////
eApiFormat CSerializeObject::GetApiFormat()const
{
	return m_apiFormat;
}
////////////////////////////////////////////////////////////////////////////
void CSerializeObject::IncreaseUpdateCounter()
{
    m_updateCounter++;
    if (UPDATE_CNT_BEGIN_END == m_updateCounter)
	{
        m_updateCounter = 0;
	}
}

////////////////////////////////////////////////////////////////////////////
WORD CSerializeObject::InsertUpdateCntChanged(CXMLDOMElement* thisNode, DWORD objToken)const
{
	thisNode->AddChildNode("OBJ_TOKEN", m_updateCounter);

	WORD changed = FALSE;
	if (UPDATE_CNT_BEGIN_END == objToken)
	{
		changed = TRUE;
	}
	else if(m_updateCounter>objToken)
	{
		changed = TRUE;
	}
	thisNode->AddChildNode("CHANGED", changed,_BOOL);

	return changed;
}

STATUS CSerializeObject::WriteXmlFileAsParts(const char *file_name, CXMLDOMElement* pXMLRootElement)
{
	SerializeXml(pXMLRootElement);

	STATUS status = pXMLRootElement->WriteXmlFile(file_name);

	if (status == STATUS_WRITE_FILE_FAILED)
	{
		eProcessType processType = CProcessBase::GetProcess()->GetProcessType();
		if(eProcessFaults != processType)
			PASSERT(1);
	}

	return status;
}

void CSerializeObject::SerializeXml(CXMLDOMElement*& pFatherNode,bool isForFile) const
{
  //return;
}
