#ifndef INSTALL_PHASE_COMMON_H_
#define INSTALL_PHASE_COMMON_H_
#include "StatusesGeneral.h"
#include "SharedDefines.h"
#include "SharedMemoryArray.h"

class CXMLDOMElement;

enum eInstallPhaseType
{
    eInstallPhaseType_undefined   = -1,
    eInstallPhaseType_swLoading   = 0,
    eInstallPhaseType_ipmcBurning = 1,
    eInstallPhaseType_completed   = 2,
    // add here, update below...

    eInstallPhaseType_numberOfPhases = 3
};

enum eInstallPhaseStatus
{
    eStatusNotStarted = 1,
    eStatusInProgress = 2,
    eStatusSuccess    = 3,
    sStatusFailure    = 4
};

struct CInstallPhase
{
    CInstallPhase(void);
    void SerializeXml(CXMLDOMElement* pFatherNode) const;

    DWORD               m_id; // for the shared memory map
    int                 m_progress;
    eInstallPhaseStatus m_status;
};


class CInstallPhaseList
{
public:
    explicit CInstallPhaseList(BYTE permission);
    void SerializeXml(CXMLDOMElement* pFatherNode) const;

    void InitToZeroState();
    BOOL GetIsDuringInstall() const;

    CSharedMemoryArray<CInstallPhase> m_phaseList;
};

#endif  // INSTALL_PHASE_COMMON_H_
