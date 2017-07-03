#ifndef __AUDITOR_API_H__
#define __AUDITOR_API_H__

#include <string>
using namespace std;

#include "ManagerApi.h"
#include "AuditDefines.h"
#include "SerializeObject.h"
#include "StructTm.h"
#include "SerializeObject.h"



enum eFreeDataType
{
    eFreeDataTypeText = 0,
    eFreeDataTypeXml,
    NUM_AUDIT_DATA_TYPES
};
static const char * GetFreeDataTypeName(eFreeDataType type)
{
    static const char *names []=
        {
            "text",
            "xml"
        };
    const char *name = ((DWORD)type < sizeof(names) / sizeof(names[0])
                        ?
                        names[type] : "InvalidType");
    return name;
}






/*--------------------------------------------------------------------------------------
  CFreeDataNode
--------------------------------------------------------------------------------------*/

class CFreeDataNode : public CSerializeObject
{
public:
    CFreeDataNode();
    CFreeDataNode(const string & xmlTag, eFreeDataType dataType, const string & data);
    virtual ~CFreeDataNode();

    virtual const char*  NameOf() const{return "CFreeDataNode";}
    virtual CSerializeObject* Clone(){return new CFreeDataNode(*this);}

    void operator = (const CFreeDataNode & other);

    virtual void Serialize(WORD format, CSegment *pSeg);
    virtual void DeSerialize(WORD format, CSegment *pSeg);

    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    void SetData(const string & xmlTag, eFreeDataType dataType, const string & data);

private:
    string m_XmlTag;
    eFreeDataType m_DataType;
    string m_Data;

};






/*--------------------------------------------------------------------------------------
  CFreeData
--------------------------------------------------------------------------------------*/

class CFreeData : public CSerializeObject
{
public:
    CFreeData();
    CFreeData(CFreeDataNode & freeDataNode1);
    CFreeData(CFreeDataNode & freeDataNode1, CFreeDataNode & freeDataNode2);
    virtual ~CFreeData();

    virtual CSerializeObject* Clone(){return new CFreeData(*this);}
    virtual const char*  NameOf() const{return "CFreeData";}

    void operator = (const CFreeData & other);

    virtual void Serialize(WORD format, CSegment *pSeg);
    virtual void DeSerialize(WORD format, CSegment *pSeg);

    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    CFreeDataNode & GetdataNode1(){return m_FreeDataNode1;}
    CFreeDataNode & GetdataNode2(){return m_FreeDataNode2;}

private:
    CFreeDataNode m_FreeDataNode1;
    CFreeDataNode m_FreeDataNode2;
};








/*--------------------------------------------------------------------------------------
  CAuditHdrWrapper
--------------------------------------------------------------------------------------*/

class CAuditHdrWrapper : public CSerializeObject
{
public:
    CAuditHdrWrapper(const AUDIT_EVENT_HEADER_S & hdr, const CFreeData & freeData);

    virtual ~CAuditHdrWrapper();



    virtual const char* NameOf()const{return "CAuditHdrWrapper";}
    virtual CSerializeObject* Clone(){return NULL;}

    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    virtual void Serialize(WORD format, CSegment *pSeg);
    virtual void DeSerialize(WORD format, CSegment *pSeg);

    void SetHdr(const AUDIT_EVENT_HEADER_S & hdr){memcpy(&m_Hdr, &hdr, sizeof(AUDIT_EVENT_HEADER_S));}
    const AUDIT_EVENT_HEADER_S & GetHdr()const{return m_Hdr;}

    void SetFreeData(const CFreeData & data){m_FreeData = data;}
    const CFreeData & GetFreeData(){return m_FreeData;}

    void SetEventTime(const CStructTm & time){m_EventTime = time;}

    void SetSequenceNum(DWORD num){m_SequenceNum = num;}
    DWORD GetSequenceNum()const{return m_SequenceNum;}

private:
    AUDIT_EVENT_HEADER_S  m_Hdr;
    CFreeData  m_FreeData;
    CStructTm  m_EventTime;
    DWORD m_SequenceNum;
};







/*--------------------------------------------------------------------------------------
  CAuditorApi
--------------------------------------------------------------------------------------*/

class CAuditorApi : public CManagerApi
{
public:
    CAuditorApi();
	virtual ~CAuditorApi();
    virtual const char*  NameOf() const {return "CAuditorApi";}

    STATUS SendEventOutsider(const AUDIT_EVENT_HEADER_S &auditHdr, const CFreeData & freeData);
    STATUS SendEventMcms(const AUDIT_EVENT_HEADER_S &auditHdr, const CFreeData & freeData);

    static void PrepareAuditHeader(AUDIT_EVENT_HEADER_S & outAuditHdr,
                                   const string & userIdName,
                                   APIU32 reportModule,
                                   const string & workStation,
                                   const string & ipAddress,
                                   APIU32 eventType,
                                   APIU32 status,
                                   const string & action,
                                   const string & description,
                                   const string & failure_description,
                                   const string & descriptionEx);
    static void PrepareFreeData(CFreeData & outFreeData,
                                  const string & xmlTag1,
                                  eFreeDataType data1Type,
                                  const string & data1,
                                  const string & xmlTag2,
                                  eFreeDataType data2Type,
                                  const string & data2);

private:
    STATUS SendEvent(OPCODE opcode, const AUDIT_EVENT_HEADER_S &auditHdr, const CFreeData & freeData);
};

#endif /* __AUDITOR_API_H__ */
