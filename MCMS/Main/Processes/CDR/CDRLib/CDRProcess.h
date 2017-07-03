// CDRProcess.h: interface for the CCDRProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CDRPROCESS_H__)
#define _CDRPROCESS_H__

#include "ProcessBase.h"
#include "CDRSettings.h"
#include "TraceStream.h"
#include "Versions.h"

class CCdrList;
class CCdrLog;





class CCDRProcess : public CProcessBase  
{
CLASS_TYPE_1(CCDRProcess,CProcessBase )
public:
	friend class CTestCDRProcess;

	CCDRProcess();
	virtual ~CCDRProcess();

	const char * NameOf(void) const {return "CCDRProcess";}
	virtual eProcessType GetProcessType() {return eProcessCDR;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();

	virtual int GetProcessAddressSpace(){return 60 * 1024 * 1024;}
   
	CCdrLog* GetCdrLog();
	void SetCdrLog(CCdrLog *log);
	CCDRSettings* GetCdrSettings() {return m_pCdrSettings;};
	void SetCdrSettings(CCDRSettings* pCdrSettings);
    
    virtual void AddExtraStatusesStrings();
	virtual bool RequiresProcessInstanceForUnitTests() {return true;}
	
	VERSION_S GetMcuVersion() {CVersions ver; return ver.GetMcuVersion();}
private:
	CCdrLog 	*m_pCdrLog;
	CCDRSettings* m_pCdrSettings;

};

#endif // !defined(_CDRPROCESS_H__)
