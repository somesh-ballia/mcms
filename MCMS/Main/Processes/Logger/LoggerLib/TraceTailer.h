#ifndef TRACETAILER_H_
#define TRACETAILER_H_

#include "PObject.h"
#include <list>
#include <ostream>

const DWORD DEFAULT_MAX_TAIL_LEN = 100;
const DWORD         MAX_TAIL_LEN = 1000;
const DWORD         MIN_TAIL_LEN = 1;

typedef std::list<std::string> CTraceList; 


class CTraceTailer : public CPObject
{
CLASS_TYPE_1(CTraceTailer, CPObject)	
public:
	CTraceTailer();
	virtual ~CTraceTailer();
	
	virtual const char* NameOf() const { return "CTraceTailer"; }
	void SetTailLen(DWORD len);
	void SetTrace(const std::string &trace);
	void GetTail(std::ostream& answer) const;
	
private:
	// disabled
	CTraceTailer(const CTraceTailer&);
	CTraceTailer& operator=(const CTraceTailer&);
	
	
	void PushBack(const std::string &trace);
	void PopFront();
	
	
	DWORD m_Size; // efficiency
	DWORD m_MaxTailLen;
	CTraceList m_TailContent;
};

#endif /*TRACETAILER_H_*/
