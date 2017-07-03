#ifndef RTMISDNSERVICELIST_H_
#define RTMISDNSERVICELIST_H_


#include "SerializeObject.h"
#include "NStream.h"
#include "psosxml.h"
#include "RtmIsdnMngrInternalDefines.h"


class CRtmIsdnService;
class CRtmIsdnPhoneNumberRange;
class CRtmIsdnSpanParam;

/////////////////////////////////////////////////////////////////////////////
// CRtmIsdnServiceList
class CRtmIsdnServiceList : public CSerializeObject
{
CLASS_TYPE_1(CRtmIsdnServiceList,CSerializeObject)

public:
	CRtmIsdnServiceList();                   
	CRtmIsdnServiceList(const CRtmIsdnServiceList &other);
	CRtmIsdnServiceList& operator=(const CRtmIsdnServiceList& other);
	virtual ~CRtmIsdnServiceList() ;
	
//    void   Serialize(WORD format, COstrStream  &m_ostr, DWORD apiNum);     
//    void   DeSerialize(WORD format, CIstrStream &m_istr, DWORD apiNum);    
//    void   ShortSerialize(WORD format, COstrStream  &m_ostr, DWORD apiNum);     
//    void   ShortDeSerialize(WORD format, CIstrStream &m_istr, DWORD apiNum);    
 	void   SerializeXml(CXMLDOMElement* pFatherNode, DWORD ObjToken) const;
	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int	   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);


	CSerializeObject* Clone() {return new CRtmIsdnServiceList();}
    const char*  NameOf() const;                
	
    BOOL   IsEmpty();
    WORD   GetServProvNumber () const;
    void   SetServProvNumber(const WORD num);
    
    STATUS  Add(CRtmIsdnService &other, const char *file_name="");                 
    STATUS  Update(const CRtmIsdnService &other);
    STATUS  GetCancelServiceStat(const char* serviceName);
    STATUS  Cancel(const char* serviceName, bool isToCheckStat=true, const char *file_name="", bool isBothFiles=false);  
//   int    AddOnlyMem(const CRtmIsdnService&  other);                 

    STATUS CheckAddPhoneRange(const char* serviceName, const CRtmIsdnPhoneNumberRange &phoneRange);
	STATUS AddPhoneNumRange(const char* serviceName, const CRtmIsdnPhoneNumberRange &phoneRange, const char *file_name);
//	STATUS UpdatePhoneNumRange(const char* serviceName, const CRtmIsdnPhoneNumberRange &phoneRange, const char *file_name); - not implemented
	STATUS GetUpdatePhoneNumRangeStat(const char* serviceName, const CRtmIsdnPhoneNumberRange &phoneRange);
	STATUS DelPhoneNumRange(const char* serviceName, const CRtmIsdnPhoneNumberRange &phoneRange, const char *file_name, bool isToCheckStat=true);

    int    FindService(const CRtmIsdnService &other);  
    int    FindService(const char* name);  

    CRtmIsdnService*  GetCurrentServiceProvider(const char*  name); 
    CRtmIsdnService*  GetService(const WORD line); 
    CRtmIsdnService*  GetService(const char* firstNum, const char *latsNum); 
	CRtmIsdnService*  GetService(const int index);
    CRtmIsdnService*  GetFirstService();
    CRtmIsdnService*  GetNextService();

    void  SetDefaultName(const char* name, const char *file_name);                 
    const char*  GetDefaultName() const;                 
    bool  IsDefaultExists() const;                 
    bool  IsDefault(const char*  name) const;                 

	bool GetIsServiceAdded();
	
	eSpanType DeleteServiceWithInconsistentSpanType();


//	#ifdef __HIGHC__
//	STATUS AddGwRangeToService(int idx, const CGW_PhoneRange& newRange);
//	STATUS CancelGwRangeFromService(int idx, const CGW_PhoneRange& other);
//	STATUS UpdateGwRangeInService(int idx, const CGW_PhoneRange& other);
//	void SetGwPhoneRangesListFromGwRanges();
//	#endif

//	STATUS IsPhoneNumberExistsAtAll(CGW_PhoneRange pPhone);
	bool IsPhoneNumberExistsInRanges(const CRtmIsdnPhoneNumberRange &phone);
	bool IsPhoneNumberExistsInRangesExactly(const CRtmIsdnPhoneNumberRange &phone);

    DWORD GetUpdateCounter() const;
    BYTE  GetChanged() const;


private:
	   // Attributes
    WORD                m_numb_of_serv_prov;    
    char                m_defaultServiceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN]; 
    CRtmIsdnService*   m_pServiceProvider[MAX_ISDN_SERVICES_IN_LIST];
	WORD                m_ind_serv;
	DWORD               m_updateCounter;  //counts all updates made to list
	BYTE				m_bChanged;
	bool 		  m_IsServiceAdded;
};











#endif /*RTMISDNSERVICELIST_H_*/
