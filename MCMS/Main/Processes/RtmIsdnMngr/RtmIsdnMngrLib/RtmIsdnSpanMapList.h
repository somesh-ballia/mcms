#ifndef RTMISDNSPANMAPLIST_H_
#define RTMISDNSPANMAPLIST_H_


#include "SerializeObject.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "RtmIsdnDefines.h"


class CRtmIsdnMngrProcess;
class CRtmIsdnSpanMap;


//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class CRtmIsdnSpanMapList                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////
class CRtmIsdnSpanMapList : public CSerializeObject
{
CLASS_TYPE_1(CRtmIsdnSpanMapList,CSerializeObject)
public:
	CRtmIsdnSpanMapList();
    CRtmIsdnSpanMapList( const CRtmIsdnSpanMapList &other );
    CRtmIsdnSpanMapList&  operator=( const CRtmIsdnSpanMapList& other );
    virtual ~CRtmIsdnSpanMapList() ;
 
 	void	SerializeXml(CXMLDOMElement* pFatherNode,DWORD ObjToken) const;
	void	SerializeXml(CXMLDOMElement*& pFatherNode/*,bool isToEma=false*/) const;
	int		DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	CSerializeObject*	Clone(){return new CRtmIsdnSpanMapList();} 
    
    STATUS ReadXmlFile( const char * file_name,
	                    eFailReadingFileActiveAlarmType activeAlarmType=eNoActiveAlarm,
	                    eFailReadingFileOperationType operationType=eNoAction,
	                    int activeAlarmId=0 );
    
    int		GetMaxNumOfSpanMaps() const;

    WORD	GetSpanMapNumber() const;
    void	SetSpanMapNumber( const WORD num );

    void SetSpanMapBoardID(const int i);
    void SetSpanMapSpanID(const int i);

	STATUS	GetUpdateSpanMapStat(const CRtmIsdnSpanMap &other);
	STATUS	UpdateSpanMap(const CRtmIsdnSpanMap& other, const bool isToCheckStat=true);
	STATUS	DetachSpanMap(const SPAN_DISABLE_S &other,bool isSpanValid = true);
	STATUS  SetIsSpanMapValid( const SPAN_DISABLE_S& other ,WORD boardId);
	STATUS  DetachSpanOfNonExistingService();
	STATUS	DetachAllSpanMapsOfService(const char* serviceName);
	int		GetNumOfAttachedSpansOnBoard(WORD boardId);
	STATUS	DetachExceededNumOfConfiguredSpans(const int maxAllowedNumber);
	bool	DetachSpecSpanIfExceeded( CRtmIsdnSpanMap *pCurSpanMap,
			                          int &configuredAccumulator_Board1,
			                          int &configuredAccumulator_Board2,
			                          int &configuredAccumulator_Board3,
			                          int &configuredAccumulator_Board4,
			                          const int maxAllowedNumber );

	BOOL	IsAnySpanAttachedToService();
	bool	IsAnySpanAttachedOnBoard(WORD boardId);

	//	STATUS	AddSpanMap(const CRtmIsdnSpanMap &other);
//	STATUS	CancelSpanMap( const WORD boardId, const WORD spanId );

	STATUS  UpdateSpanStatus(const RTM_ISDN_SPAN_STATUS_MCMS_S &other);

    int		FindSpanMap( const CRtmIsdnSpanMap &other )const;
	int		FindSpanMap( const WORD boardId, const WORD spanId )const;

    CRtmIsdnSpanMap*	GetSpanMap( const WORD boardId, const WORD spanId );

    CRtmIsdnSpanMap*	GetFirstSpanMap();
    CRtmIsdnSpanMap*	GetNextSpanMap();

    bool IsThereAnySpanConfigured(const WORD boardId) const;
    eCardsClockSourceStateType GetCardsClockSourceState(const WORD boardId) const;

    DWORD	GetUpdateCounter() const;
    BYTE	GetChanged() const;
   
    void SetUpdateCounter(DWORD cnt);

	
protected:
	CRtmIsdnMngrProcess* m_pProcess;
	
	void	InitParams();
	
	
	int					m_maxNumOfSpanMaps;
    WORD				m_numb_of_spanMaps;
    CRtmIsdnSpanMap*	m_pSpanMap[MAX_ISDN_SPAN_MAPS_IN_LIST];
    
    WORD	m_ind_spanMap;
    BYTE	m_bChanged;
};





#endif /*RTMISDNSPANMAPLIST_H_*/
