// SerializeObject.h

#if !defined(_SERIALIZEOBJECT_H__)
#define _SERIALIZEOBJECT_H__

#include "TranEntry.h"
#include "PObject.h"
#include "ApiBaseObject.h"
#include "ApiBaseObjectPtr.h"

class CXMLDOMElement;

#define UPDATE_CNT_BEGIN_END	0xFFFFFFFF



enum eFailReadingFileActiveAlarmType
{
	eNoActiveAlarm = 0,
	eActiveAlarmExternal,
	eActiveAlarmInernal
};

enum eFailReadingFileOperationType
{
	eNoAction = 0,
	eRenameFile,
	eRemoveFile
};

class CSerializeObject : public CPObject
{
CLASS_TYPE_1(CSerializeObject, CPObject)
public:
  static DWORD GetMaxXMLFileSize(void);
  static DWORD SetMaxXMLFileSize(DWORD size);

	CSerializeObject();
	virtual ~CSerializeObject();

	virtual const char* NameOf() const { return "CSerializeObject";}
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode ) const = 0;
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode,bool isForFile) const ;

	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action) = 0;
	virtual int   DeSerialize_Xml(CXMLDOMElement *pActionNode, char *pszError, const char* action,int FileNum=0){return 0;};
	virtual void Serialize(WORD format,CSegment& rSeg);
	virtual void DeSerialize(WORD format,CSegment& rSeg);

	STATUS   WriteXmlFile(const char * file_name, const char * root_name ) const;
	STATUS   WriteXmlFile(const char * file_name) const;
	STATUS   WriteXml_File(const char * file_name, const char * root_name );
	STATUS   WriteXml_File(const char * file_name);
  STATUS   WriteXml_File(const char *file_name, CXMLDOMElement* pXMLRootElement);
  STATUS   WriteXmlFileAsParts(const char *file_name, CXMLDOMElement* pXMLRootElement);

	STATUS ReadXmlZipFile( const char * file_name,
						eFailReadingFileActiveAlarmType activeAlarmType=eNoActiveAlarm,
						eFailReadingFileOperationType operationType=eNoAction,
						int activeAlarmId=0 );
	STATUS ReadXmlFile( const char * file_name,
	                    eFailReadingFileActiveAlarmType activeAlarmType=eNoActiveAlarm,
	                    eFailReadingFileOperationType operationType=eNoAction,
	                    int activeAlarmId=0 );

   STATUS ReadXml_Files( const char * fileName,
	                                  eFailReadingFileActiveAlarmType activeAlarmType,
	                                  eFailReadingFileOperationType operationType,int FileNum,
	                                  int activeAlarmId =0);
	virtual void Dump(std::ostream&) const;

	void  SetUpdateCounter(DWORD counter);
	DWORD GetUpdateCounter()const;
	void IncreaseUpdateCounter();

	void  SetApiFormat(eApiFormat format);
	eApiFormat GetApiFormat()const;

	virtual CSerializeObject* Clone() = 0;
	void SetRequestFunction(HANDLE_REQUEST);

	ApiBaseObjectPtr GetRestApiObject()  const{ return m_apiBaseObjRest;}

	WORD InsertUpdateCntChanged(CXMLDOMElement* thisNode, DWORD objToken)const;

	HANDLE_REQUEST m_pRequestfunction;
	DWORD          m_updateCounter;
	mutable eApiFormat      m_apiFormat;

	mutable ApiBaseObjectPtr  m_apiBaseObjRest;


private:
	STATUS HandleFileErrorOpen(const char *fileName,
							  DWORD errnoCode,
		                      eFailReadingFileActiveAlarmType activeAlarmType,
		                      int activeAlarmId);
	STATUS HandleFileErrorParseXml(const char *fileName,
								  eFailReadingFileActiveAlarmType activeAlarmType,
	                              eFailReadingFileOperationType operationType,
	                              int activeAlarmId);
	void TreatFailFileOperation(const char * fileName,
                                 STATUS status,
                                 const char *decription,
                                 eFailReadingFileActiveAlarmType activeAlarmType,
                                 eFailReadingFileOperationType operationType,
                                 int activeAlarmId = 0);
    STATUS WriteXmlFile(const char *file_name, CXMLDOMElement*& pXMLRootElement) const;

    void UnZipError(FILE* infile, void* pCompressionStream, const char *ErrorMessage, const char *param, const DWORD ErrorCode);

    // Defines maximum size of XML file in bytes
    static DWORD s_max_xml_file_size;
};

#endif // !defined(_SERIALIZEOBJECT_H__)
