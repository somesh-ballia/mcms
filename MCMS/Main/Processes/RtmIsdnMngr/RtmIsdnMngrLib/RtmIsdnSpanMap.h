#ifndef RTMISDNSPANMAP_H_
#define RTMISDNSPANMAP_H_


#include "SerializeObject.h"
#include "NStream.h"
#include "psosxml.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "RtmIsdnMngrInternalStructs.h"


/////////////////////////////////////////////////////////////////////////////
// CRtmIsdnSpanMap
class CRtmIsdnSpanMap : public CSerializeObject
{
CLASS_TYPE_1(CRtmIsdnSpanMap,CPObject)
public:
	CRtmIsdnSpanMap();                   
    virtual ~CRtmIsdnSpanMap() ;


//    void   Serialize(WORD format, COstrStream  &m_ostr);     
//    void   DeSerialize(WORD format, CIstrStream &m_istr);    
	void   SerializeXml(CXMLDOMElement*& pFatherNode/*,bool isToEma=false*/) const;
	int	   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	CSerializeObject* Clone() {return new CRtmIsdnSpanMap();}
    const char*  NameOf() const {return "CRtmIsdnSpanMap";} 
	
	void		InitSpanStatuses();

    WORD		GetBoardId() const;                 
    void		SetBoardId(const WORD boardId);   

	
    WORD		GetSpanId() const;                 
    void		SetSpanId(const WORD spanId);   

    void        SetIsSpanValid(const bool isSpanValid);
	
    const char*	GetServiceName () const;
    void		SetServiceName(const char*  serviceName);                 

	bool		GetIsAttachedToService() const;
	void		SetIsAttachedToService(bool isAttached);
	void		DetachFromService();

	eSpanAlarmType		GetAlarm();
	void				SetAlarm(eSpanAlarmType newAlarm);

	eDChannelStateType	GetDChannelState();
	void				SetDChannelState(eDChannelStateType newState);

	eClockingType		GetClocking();
	void				SetClocking(eClockingType newClocking);

	void ConvertToRtmIsdnSpanMapStruct(RTM_ISDN_SPAN_MAP_S &spanMapsStruct);


protected:
	char	m_serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN]; 
	BOOL	m_isAttachedToService; // reqiured by GUI team

	WORD	m_boardId;
	WORD	m_spanId;
	
	bool	m_isSpanValid; // for eRtmIsdn_9PRI card spans 10-12 are invalid

	eSpanAlarmType		m_spanAlarm;
	eDChannelStateType	m_dChannelState;
	eClockingType		m_clockingState;
};



#endif /*RTMISDNSPANMAP_H_*/
