#include "InstallPhaseCommon.h"

#include "psosxml.h"
#include "StringsMaps.h"

CInstallPhase::CInstallPhase() :
  m_id(0),
  m_progress(-1),
  m_status(eStatusNotStarted)
{}

void CInstallPhase::SerializeXml(CXMLDOMElement* pFatherNode) const
{
    CXMLDOMElement* pPhaseNode = pFatherNode->AddChildNode("INSTALL_PHASE");
    if (pPhaseNode)
    {
        pPhaseNode->AddChildNode("INSTALL_PHASE_TYPE",m_id,INSTALL_PHASE_TYPE_ENUM);
        pPhaseNode->AddChildNode("INSTALL_PHASE_PROGRESS",m_progress);
        pPhaseNode->AddChildNode("INSTALL_PHASE_STATUS",m_status,INSTALL_PHASE_STATUS_ENUM);
    }
}

CInstallPhaseList::CInstallPhaseList(BYTE permission) :
    m_phaseList("InstallPhaseList", permission, eInstallPhaseType_numberOfPhases)
{
    if (permission == READ_WRITE)  // only the master should init the values
        InitToZeroState();
}

void CInstallPhaseList::InitToZeroState()
{
    for (int index = (int)eInstallPhaseType_swLoading;
         index < (int)eInstallPhaseType_numberOfPhases;
         index++)
    {
        CInstallPhase phase;
        phase.m_id = index;
        phase.m_progress = 0;
        phase.m_status = eStatusNotStarted;
        m_phaseList.Add(phase);
    }
}

BOOL CInstallPhaseList::GetIsDuringInstall() const
{
  CInstallPhase loadingPhase;
  m_phaseList.Get((int)eInstallPhaseType_swLoading, loadingPhase);
  return (loadingPhase.m_status != eStatusNotStarted);
}

void CInstallPhaseList::SerializeXml(CXMLDOMElement* pFatherNode) const
{
    if (GetIsDuringInstall())
    {
        CXMLDOMElement * pListNode = pFatherNode->AddChildNode("INSTALL_PHASES_LIST");
        if (pListNode)
        {
            for (int index = (int ) eInstallPhaseType_swLoading;
                    index < (int) eInstallPhaseType_numberOfPhases;
                    index++)
            {
                CInstallPhase phase;
                m_phaseList.Get(index,phase);
                phase.SerializeXml(pListNode);
            }
        }
    }
}



