// RequestHandler.h: interface for the CRequestHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CREQUESTHANDLER_H__)
#define CREQUESTHANDLER_H__

#include <ostream>
#include "AlarmableTask.h"
#include "TransactionsFactory.h"
#include "OperatorDefines.h"
#include "OpcodesMcmsCommon.h"
#include "PostXmlHeader.h"
#include "AuditDefines.h"
#include "XmlApi.h"


class CXMLDOMDocument;
class CXMLDOMElement;
class CObjString;




class CRequestHandler : public CAlarmableTask
{
CLASS_TYPE_1(CRequestHandler,CAlarmableTask )
public:
	CRequestHandler();
	virtual ~CRequestHandler();

	virtual const char* NameOf() const { return "CRequestHandler";}
	virtual void HandlePostRequest(CSegment* pSeg);

	CXMLDOMElement* HandleRequest(CXMLDOMElement* pXMLRequest,
                                  COsQueue &q,
                                  WORD nAuthorization,DWORD ifNoneMatch,
                                  const string & encodingTypeFrom);

	virtual void InitTransactionsFactory();
	void InitTask();

protected:
	STATUS PostXml(const char *pRespBuff,
                   DWORD respBuffLen,
                   COsQueue& SendQ,
                   COsQueue* pReplyQ=NULL,
                   OPCODE opcode=XML_RESPONSE,
                   WORD nAuthorization=GUEST,bool bAddAdditionalParams=false);
	virtual void ReceiveAdditionalParams(CSegment* pSeg) {};
	virtual void SendAdditionalParams(CSegment* pSeg) {};
	virtual void ResetAdditionalParams() {};
	char  m_szErrorMessage[ERROR_MESSAGE_LEN];
	CTransactionsFactory m_requestTransactionsFactory;

    const CPostXmlHeader & GetCurrentMsgHdr()const{return m_CurrentMsgHdr;}
    const string& GetLoginName()const{return m_CurrentMsgHdr.GetUserName();}
    const std::string& GetCurrentClientAddress() const {return  m_CurrentMsgHdr.GetClientIp();}

    void SetAuditMoreInfo(const std::string & more_info){ m_audit_more_info=more_info;}

private:
    void UpdateUserName(const string & user);
    friend class CApacheModuleManager;

    STATUS ConvertToPreviousEncoding(char *& pFrom,
                                     const string & encodingTypeTo,
                                     ostream &errorString);

    STATUS ParseHandleTransCommon(CXMLDOMDocument *pDom,
                                  COsQueue &dualMbx,
                                  WORD nAuthorization,DWORD ifNoneMatch,
                                  const string & encodingTypeFrom,
                                  CXMLDOMElement *& pOutXmlResponse,
                                  ostream &outStatusString);
    void SendEventToAuditor(CXMLDOMElement* pXMLRequest,
                            CXMLDOMElement* pXMLResponse,
                            eXmlActionNodeType actionNodeType);
    void ParseResponseStatus(CXMLDOMElement *pXMLResponse,
                             eAuditEventStatus & outStatus,
                             string & outDescription,
                             string & outDesciprionEx,
                             bool & outIsGeneralResponse);
    void ParseResponseAction(CXMLDOMElement *pXMLResponse,
                             bool isGeneralResponse,
                             eXmlActionNodeType actionNodeType,
                             string & outAction);
    CXMLDOMElement* GetActionNode(CXMLDOMElement *pXMLResponse,
                                  bool isGeneralResponse,
                                  eXmlActionNodeType actionNodeType);
    bool ShadowPrivateData(CXMLDOMElement *pXMLRequest,
                           CXMLDOMElement *pXMLResponse,
                           const string & action,
                           eXmlActionNodeType actionNodeType);
    bool ShadowLogin(CXMLDOMElement *pXMLRequest,
                     CXMLDOMElement *pXMLResponse,
                     eXmlActionNodeType actionNodeType);
    bool ShadowAddNewOperator(CXMLDOMElement *pXMLRequest,
                              CXMLDOMElement *pXMLResponse,
                              eXmlActionNodeType actionNodeType);
    bool ShadowChangePassword(CXMLDOMElement *pXMLRequest,
                              CXMLDOMElement *pXMLResponse,
                              eXmlActionNodeType actionNodeType);
    bool ShadowIPService(CXMLDOMElement *pXMLRequest,
                                      CXMLDOMElement *pXMLResponse,
                                      eXmlActionNodeType actionNodeType);
    bool ShadowConferenceEntryPwd(CXMLDOMElement *pXMLRequest,
                                      CXMLDOMElement *pXMLResponse,
                                      eXmlActionNodeType actionNodeType);
    bool ShadowConferenceChairpersonPwd(CXMLDOMElement *pXMLRequest,
                                      CXMLDOMElement *pXMLResponse,
                                      eXmlActionNodeType actionNodeType);






    CPostXmlHeader m_CurrentMsgHdr;

    std::string m_audit_more_info;

};

#endif // !defined(CREQUESTHANDLER_H__)
