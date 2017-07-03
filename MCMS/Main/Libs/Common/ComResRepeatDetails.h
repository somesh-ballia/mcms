#ifndef _REPEAT_H
#define _REPEAT_H

#include "SerializeObject.h"
#include "StructTm.h"
#include "ConfPartyApiDefines.h"

class CXMLDOMElement;


class CComResRepeatDetails  : public CSerializeObject
{
	CLASS_TYPE_1(CComResRepeatDetails, CSerializeObject)
public:

	CComResRepeatDetails();
	virtual ~CComResRepeatDetails(){};
	
	virtual const char* NameOf() const {return "CComResRepeatDetails";}
	virtual CSerializeObject* Clone(){return new CComResRepeatDetails;}
	
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int	 DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action);
	
	CComResRepeatDetails& operator = (const CComResRepeatDetails &other);

	void SetLimitTime(CStructTm LimitTime);
	void SetRecurrenceType(int nRecurrenceType);
	void SetOccurNumber(int nOccurNumber);
	void SetWeekDay(int nDayIndex, int bDay);
	void SetEndBy(int bEndBy);
	void SetInterval(int nInterval);
	void SetDayOfMonth(int nDayOfMonth);
	void SetInstance(int nInstance);
	void SetDaysIndex(int nDaysIndex);
	void SetByMonth(int bByMonth);

	CStructTm& GetLimitTime();
	int GetRecurrenceType() const;
	int GetOccurNumber() const;
	int GetWeekDay(int nDayIndex) const;
	int GetEndBy() const;
	int GetInterval() const;
	int GetDayOfMonth() const;
	int GetInstance() const;
	int GetDaysIndex() const;
	int GetByMonth() const;
	int GetGMTOffsetMinutes() const;
	
protected:

	CStructTm m_LimitTime;
	int m_RecurrenceType;
	int m_OccurNumber;
	int m_Week[7];
	int m_EndBy;
	int m_Interval;
	int	m_DayOfMonth;
	int m_Instance;
	int m_DaysIndex;
	int m_ByMonth;
	int m_nGMTOffsetMinutes;

	static char* m_szDays[];

protected:

	int DeSerializeXmlRepeatedEx(CXMLDOMElement *pRepeatedExNode,char *pszError);
	int DeSerializeXmlRepeated(CXMLDOMElement *pRepeatedNode,char *pszError);

};

#endif

