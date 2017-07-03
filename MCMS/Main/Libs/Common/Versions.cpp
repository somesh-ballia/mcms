// Versions.cpp

#include "Versions.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "ApiStatuses.h"

CVersions::CVersions()
{
  m_mcuVersion.ver_major     = 0;
  m_mcuVersion.ver_minor     = 0;
  m_mcuVersion.ver_release   = 0;
  m_mcuVersion.ver_internal  = 0;

  m_mcmsVersion.ver_major    = 0;
  m_mcmsVersion.ver_minor    = 0;
  m_mcmsVersion.ver_release  = 0;
  m_mcmsVersion.ver_internal = 0;

  memset(m_mcuPrivateDescription , 0, ARRAYSIZE(m_mcuPrivateDescription));
  memset(m_mcuVerBaseLine        , 0, ARRAYSIZE(m_mcuVerBaseLine));
  memset(m_mcuDescription        , 0, ARRAYSIZE(m_mcuDescription));
  memset(m_mcuBuildDate          , 0, ARRAYSIZE(m_mcuBuildDate));
  memset(m_mcmsPrivateDescription, 0, ARRAYSIZE(m_mcmsPrivateDescription));
  memset(m_mcmsVerBaseLine       , 0, ARRAYSIZE(m_mcmsVerBaseLine));
}

void CVersions::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
  CXMLDOMElement* pVersionsNode, *pMcuVersionNode, *pMcmsVersionNode, *pCsVersionNode;

  pVersionsNode = pActionsNode->AddChildNode("VERSIONS");

  // ----------------  1. MCU version params  -----------------
  pMcuVersionNode = pVersionsNode->AddChildNode("MCU_VERSION");
  if (pMcuVersionNode)
  {
    pMcuVersionNode->AddChildNode("MAIN"               , m_mcuVersion.ver_major);
    pMcuVersionNode->AddChildNode("MAJOR"              , m_mcuVersion.ver_minor);
    pMcuVersionNode->AddChildNode("MINOR"              , m_mcuVersion.ver_release);
    pMcuVersionNode->AddChildNode("INTERNAL"           , m_mcuVersion.ver_internal);
    pMcuVersionNode->AddChildNode("PRIVATE_DESCRIPTION", m_mcuPrivateDescription);
    pMcuVersionNode->AddChildNode("BASELINE"           , m_mcuVerBaseLine);
    pMcuVersionNode->AddChildNode("BUILD_DATE"         , m_mcuBuildDate);
  }

  // ----------------  2. MCMS version params  -----------------
  pMcmsVersionNode = pVersionsNode->AddChildNode("MCMS_VERSION");
  if (pMcmsVersionNode)
  {
    pMcmsVersionNode->AddChildNode("MAIN"               , m_mcmsVersion.ver_major);
    pMcmsVersionNode->AddChildNode("MAJOR"              , m_mcmsVersion.ver_minor);
    pMcmsVersionNode->AddChildNode("MINOR"              , m_mcmsVersion.ver_release);
    pMcmsVersionNode->AddChildNode("INTERNAL"           , m_mcmsVersion.ver_internal);
    pMcmsVersionNode->AddChildNode("PRIVATE_DESCRIPTION", m_mcmsPrivateDescription);
  }
}

int CVersions::DeSerializeXml(CXMLDOMElement* pVersionsNode,
                              char*           pszError,
                              const char*     strAction)
{
  int             nStatus = STATUS_OK;
  CXMLDOMElement* pTempNode;

  // MCU version params
  GET_CHILD_NODE(pVersionsNode, "MCU_VERSION", pTempNode);
  if (pTempNode)
  {
    GET_VALIDATE_CHILD(pTempNode, "MAIN"               , &m_mcuVersion.ver_major   , _0_TO_DWORD);
    GET_VALIDATE_CHILD(pTempNode, "MAJOR"              , &m_mcuVersion.ver_minor   , _0_TO_DWORD);
    GET_VALIDATE_CHILD(pTempNode, "MINOR"              , &m_mcuVersion.ver_release , _0_TO_DWORD);
    GET_VALIDATE_CHILD(pTempNode, "INTERNAL"           , &m_mcuVersion.ver_internal, _0_TO_DWORD);
    GET_VALIDATE_CHILD(pTempNode, "PRIVATE_DESCRIPTION", m_mcuPrivateDescription   , _0_TO_PRIVATE_VERSION_DESC_LENGTH);
    GET_VALIDATE_CHILD(pTempNode, "BASELINE"           , m_mcuVerBaseLine          , _0_TO_PRIVATE_VERSION_DESC_LENGTH);
    GET_VALIDATE_CHILD(pTempNode, "BUILD_DATE"         , m_mcuBuildDate            , _0_TO_31_STRING_LENGTH);
  }

  // MCMS version params
  GET_CHILD_NODE(pVersionsNode, "MCMS_VERSION", pTempNode);
  if (pTempNode)
  {
    GET_VALIDATE_CHILD(pTempNode, "MAIN"               , &m_mcmsVersion.ver_major   , _0_TO_DWORD);
    GET_VALIDATE_CHILD(pTempNode, "MAJOR"              , &m_mcmsVersion.ver_minor   , _0_TO_DWORD);
    GET_VALIDATE_CHILD(pTempNode, "MINOR"              , &m_mcmsVersion.ver_release , _0_TO_DWORD);
    GET_VALIDATE_CHILD(pTempNode, "INTERNAL"           , &m_mcmsVersion.ver_internal, _0_TO_DWORD);
    GET_VALIDATE_CHILD(pTempNode, "PRIVATE_DESCRIPTION", m_mcmsPrivateDescription   , _0_TO_PRIVATE_VERSION_DESC_LENGTH);
    GET_VALIDATE_CHILD(pTempNode, "BASELINE"           , m_mcmsVerBaseLine          , _0_TO_PRIVATE_VERSION_DESC_LENGTH);
  }

  return nStatus;
}

const VERSION_S& CVersions::GetMcuVersion() const
{
  return m_mcuVersion;
}

const char* CVersions::GetMcuPrivateDescription() const
{
  return m_mcuPrivateDescription;
}

const char* CVersions::GetMcmsPrivateDescription() const
{
  return m_mcmsPrivateDescription;
}

const char* CVersions::GetMcmsBaseline() const
{
  return m_mcmsVerBaseLine;
}

const char* CVersions::GetMcuBaseline() const
{
  return m_mcuVerBaseLine;
}

const char* CVersions::GetMcuDescription() const
{
  return m_mcuDescription;
}

const VERSION_S& CVersions::GetMcmsVersion() const
{
  return m_mcmsVersion;
}

