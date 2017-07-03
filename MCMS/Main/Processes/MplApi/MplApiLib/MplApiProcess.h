// MplApiProcess.h: interface for the CMplApiProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MplApiPROCESS_H__)
#define _MplApiPROCESS_H__


#include "ApiProcess.h"
#include "DataTypes.h"
#include "SharedMemoryMap.h"
#include "AllocateStructs.h"
#include "CardConnIdTable.h"
// 1080_60
#include "MplApiMasterSlaveReqAsyncHandler.h"
#include "LayoutSharedMemoryMap.h"
#include "IndicationIconSharedMemoryMap.h"



typedef CSharedMemoryMap<ConnToCardTableEntry> CSharedMemMap;

class CMplMcmsProtocol;

class CMplApiProcess : public CApiProcess  
{
public:
	friend class CTestMplApiProcess;
	
	CMplApiProcess();
	virtual ~CMplApiProcess();
	virtual const char* NameOf() const { return "CMplApiProcess";}
	virtual eProcessType GetProcessType() {return eProcessMplApi;}
	virtual BOOL UsingSockets() {return YES;}
	virtual TaskEntryPoint GetManagerEntryPoint();
	virtual int SetUp();
	virtual int TearDown();
    virtual BOOL HasMonitorTask() {return FALSE;}

	CSharedMemMap* GetSharedMemoryMap();
	void OnSpecialCommanderFailure(CMplMcmsProtocol &mplPrtcl, OPCODE opcodeBefore, STATUS status);
    
  virtual int GetProcessAddressSpace() {return 92 * 1024 * 1024;}
    void   SetUpgradeStarted();
    virtual void CloseConnection(const WORD conId);
    void GetDisconnectCounters(std::ostream& answer);
    virtual std::string GetIPAddressByBoardId(DWORD board_id);
    BOOL IsStartupFinished() const;
// 1080_60
    void SetSystemCardsBasedMode(const eSystemCardsMode systemCardsBasedMode);
    eSystemCardsMode GetSystemCardsBasedMode();
    CMplApiMasterSlaveReqAsyncHandler* getMasterSlaveReqHandler();
	//Change Layout Improvement - Layout Shared Memory (CL-SM)
	void 						InitializeLayoutSharedMemory();
	CLayoutSharedMemoryMap*  	GetLayoutSharedMemory();
	void						FreeLayoutSharedMemory();
	
	//Indication Icon Change Improvement - indication icon Shared Memory (CL-SM)
	void 						InitializeIndicationIconSharedMemory();
	CIndicationIconSharedMemoryMap*  	GetIndicationIconSharedMemory();
	void						FreeIndicationIconSharedMemory();

private:
	virtual void AddExtraStatusesStrings();


	CSharedMemMap *m_pSharedMemoryMap;
	BOOL m_isUpgradeStarted;
// 1080_60
    eSystemCardsMode	m_systemCardsBasedMode;
    CMplApiMasterSlaveReqAsyncHandler * m_masterSlaveReqHandler;
	//Change Layout Improvement - Layout Shared Memory (CL-SM)
	CLayoutSharedMemoryMap*		m_layoutSharedMemoryMap;

	//Indication Icon Change Improvement - indication icon Shared Memory (CL-SM)
	CIndicationIconSharedMemoryMap*		m_indicationIconSharedMemoryMap;

};



#endif // !defined(_MplApiPROCESS_H__)

