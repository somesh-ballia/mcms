// CMediaRecordingGet.h: interface for the CRsrcDetailGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//17/8/05		    			  Classes for resource details monitoring
//========   ==============   =====================================================================
#ifndef CCOLLECTORINFO_
#define CCOLLECTORINFO_


#include "SerializeObject.h"
#include "CollectorProcess.h"
#include "TaskApi.h"
#include "Macros.h"
#include "SingleToneApi.h"
#include "Trace.h"
#include "InitCommonStrings.h"
#include "StructTm.h"

#define COLLECT_INFO_FAILED -1
typedef enum {
	eCollectingType_audit	= 0,
	eCollectingType_cdr,
	eCollectingType_cfg,
	eCollectingType_coreDump,
	eCollectingType_faults,
	eCollectingType_logs,
	eCollectingType_processInfo,
	eCollectingType_network_traffic_capture,
	eCollectingType_participants_recordings,
	eCollectingType_nids,

	MAX_NUM_OF_COLLECTED_INFO
}eCollectingType;

/////////////////////////////////////////////////////////////////////////////////////

class CCollectInfo : public CSerializeObject
{
	CLASS_TYPE_1(CJunction, CSerializeObject)
public:
	CCollectInfo();
	CCollectInfo(const CCollectInfo& rhs);
	virtual ~CCollectInfo();
	const CCollectInfo& operator=(const CCollectInfo& other);

	void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int  DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	virtual CSerializeObject* Clone() {return new CCollectInfo;}

	BOOL GetMarkForCollection();
    void SetMarkForCollection(BYTE mark_for_collection);

    void SetCollectingType(eCollectingType collecting_type);
    eCollectingType GetCollectingType() const { return m_CollectingType;};


private:
	eCollectingType m_CollectingType;
	BOOL  m_bMarkForCollection;
	DWORD m_estimatedSize;
};

/////////////////////////////////////////////////////////////////////////////////////

class CInfoTimeInterval : public CSerializeObject
{
CLASS_TYPE_1(CInfoTimeInterval, CSerializeObject)
public:
  CInfoTimeInterval();
  CInfoTimeInterval(CStructTm& start, CStructTm& end);
  CInfoTimeInterval(const CInfoTimeInterval& rhs);
  virtual ~CInfoTimeInterval();
  const CInfoTimeInterval& operator=(const CInfoTimeInterval& other);

  void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
  int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
  int    DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);
  virtual CSerializeObject* Clone() {return new CInfoTimeInterval;}

  void SetStartTime(CStructTm& start) {m_start = start;};
  CStructTm GetStartTime() { return m_start;};
  
  void SetEndTime(CStructTm& end) {m_end = end;};
  CStructTm GetEndTime()   { return m_end;};

  void SetStartAndEndTimeToCurrentTime();

  BOOL GetIsMarkForCollection (eCollectingType collecting_type);

  void RestartCollectingDetails();

  BOOL IsJitcMode() const;

private:
   
  void RestartDetails();

  CStructTm  m_start;
  CStructTm  m_end;
  CCollectInfo m_collectInfo[MAX_NUM_OF_COLLECTED_INFO];
 
 };
/////////////////////////////////////////////////////////////////////////////////////////

#endif // !defined(_CMediaRecordingGet_H__)
