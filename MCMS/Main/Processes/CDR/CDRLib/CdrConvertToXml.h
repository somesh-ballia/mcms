#ifndef _CDR_CONVERT_TO_XML
#define _CDR_CONVERT_TO_XML

#include <set>

#include "TaskApi.h"
#include "ManagerTask.h"
#include "CDRProcess.h"
#include "CDRShort.h"

extern "C" void CdrConvertToXmlEntryPoint(void* appParam);

class CCdrConvertToXml : public CTaskApp
{
	CLASS_TYPE_1(CCdrConvertToXml, CTaskApp)
public:

	virtual const char* NameOf() const { return "CCdrConvertToXml"; }

	CCdrConvertToXml();
    virtual ~CCdrConvertToXml();
	virtual void Create(CSegment& appParam, WORD limited=FALSE); // called from entry point
	virtual eTaskRecoveryPolicyAfterSeveralRetries GetTaskRecoveryPolicyAfterSeveralRetries() const {return eCreateNewTask;}
	BOOL  IsSingleton() const { return YES; }
	void  CreateXmlFolder(CSegment* pSeg);
	void  Abort();
	STATUS AddAllCdrToXmlDirector(CStructTm* startTime, CStructTm* endTime);
	bool   IsValidTime(CCdrShort* cdrShort, CStructTm* startTime, CStructTm* endTime);
	STATUS AddCdrToXml(CSerializeObject* pResponse, string fileName);
	const char* GetTaskName() const;
	void  InitTask() {;}

 protected:

 private:

	CCDRProcess* m_CDRProcess;
	bool m_isAborted;


/* 	PDECLAR_TERMINAL_COMMANDS */
	PDECLAR_MESSAGE_MAP
};

#endif
