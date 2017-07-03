// CSApiProcess.h: interface for the CCSApiProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CSApiPROCESS_H__)
#define _CSApiPROCESS_H__

#include "ApiProcess.h"
#include "DataTypes.h"
#include "CSApiDefines.h"
#include "CardConnIdTable.h"



class CCSApiProcess : public CApiProcess  
{
public:
	friend class CTestCSApiProcess;

	CCSApiProcess();
	virtual ~CCSApiProcess();
	const char * NameOf(void) const {return "CCSApiProcess";}
	virtual eProcessType GetProcessType() {return eProcessCSApi;}
	virtual BOOL UsingSockets() {return YES;}
	virtual TaskEntryPoint GetManagerEntryPoint();

	void SetCurrentProtocol(eCommProtocol prot){m_CurrentProtocol = prot;}
	eCommProtocol GetCurrentProtocol()const{return m_CurrentProtocol;}

    virtual BOOL HasMonitorTask() {return FALSE;}
    virtual int GetProcessAddressSpace() {return 80 * 1024 * 1024;}

private:
	eCommProtocol m_CurrentProtocol;
};

#endif // !defined(_CSApiPROCESS_H__)

