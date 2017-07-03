#include <stdio.h>
#include <stdlib.h>

#include "RtmIsdnService.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "RtmIsdnSpanMap.h"
#include "RtmIsdnPhoneNumberRange.h"
#include "ApiStatuses.h"
#include "StringsMaps.h"
#include "InternalProcessStatuses.h"
#include "../../../McmIncld/MPL/Card/RtmIsdnMaintenance/RtmIsdnMaintenanceStructs.h"
#include "TraceStream.h"
#include "DefinesGeneral.h"
#include "RtmIsdnMngrProcess.h"

////////////////////////////////////////////////////////////////////////////
//                        CRtmIsdnService
////////////////////////////////////////////////////////////////////////////
CRtmIsdnService::CRtmIsdnService()
{
  strcpy_safe(m_serviceName, "ISDN");

  memset(m_dialOutPrefix, 0, sizeof(m_dialOutPrefix));
  memset(m_mcuCli, 0, sizeof(m_mcuCli));
  memset(m_pPhoneNumRange, 0, sizeof(m_pPhoneNumRange));

  m_dfltNumType            = eDfltNumTypeUnknown;
  m_numPlan                = eNumPlanTypeUnknown;
  m_voice                  = eVoiceTypeSpeech;
  m_netSpecFacility        = eNetSpecFacilityTypeNull;
  m_pSpanDef               = new CRtmIsdnSpanDefinition;
  m_numb_of_phoneNumRanges = 0;
  m_ind_phoneRange         = 0;
  m_updateCounter          = 0;
  m_bChanged               = FALSE;

  for (int i = 0; i < MAX_NUM_OF_BOARDS; i++)
    SetIpAddressesList(i);
}

//--------------------------------------------------------------------------
CRtmIsdnService& CRtmIsdnService::operator=(const CRtmIsdnService& other)
{
  if(this == &other){
	  return *this;
  }

  strcpy_safe(m_serviceName, other.m_serviceName);
  strcpy_safe(m_dialOutPrefix, other.m_dialOutPrefix);
  strcpy_safe(m_mcuCli, other.m_mcuCli);

  m_voice                  = other.m_voice;
  m_numPlan                = other.m_numPlan;
  m_dfltNumType            = other.m_dfltNumType;
  m_netSpecFacility        = other.m_netSpecFacility;
  m_numb_of_phoneNumRanges = other.m_numb_of_phoneNumRanges;
  m_ind_phoneRange         = other.m_ind_phoneRange;
  m_updateCounter          = other.m_updateCounter;
  m_bChanged               = other.m_bChanged;

  PDELETE(m_pSpanDef);
  if (other.m_pSpanDef)
    m_pSpanDef = new CRtmIsdnSpanDefinition(*other.m_pSpanDef);

  for (int i = 0; i < MAX_ISDN_PHONE_NUMBER_IN_SERVICE; i++)
  {
    PDELETE(m_pPhoneNumRange[i]);
    if (other.m_pPhoneNumRange[i])
      m_pPhoneNumRange[i] = new CRtmIsdnPhoneNumberRange(*other.m_pPhoneNumRange[i]);
  }

  memcpy(m_ipAddressesList, other.m_ipAddressesList, sizeof(m_ipAddressesList));

  return *this;
}

//--------------------------------------------------------------------------
CRtmIsdnService::CRtmIsdnService(const CRtmIsdnService& other)
  : CSerializeObject(other)
{
  strcpy_safe(m_serviceName, other.m_serviceName);
  strcpy_safe(m_dialOutPrefix, other.m_dialOutPrefix);
  strcpy_safe(m_mcuCli, other.m_mcuCli);

  m_voice                  = other.m_voice;
  m_numPlan                = other.m_numPlan;
  m_dfltNumType            = other.m_dfltNumType;
  m_netSpecFacility        = other.m_netSpecFacility;
  m_numb_of_phoneNumRanges = other.m_numb_of_phoneNumRanges;
  m_ind_phoneRange         = other.m_ind_phoneRange;
  m_updateCounter          = other.m_updateCounter;
  m_bChanged               = other.m_bChanged;

  if (other.m_pSpanDef == NULL)
    m_pSpanDef = NULL;
  else
    m_pSpanDef = new CRtmIsdnSpanDefinition(*other.m_pSpanDef);

  for (int i = 0; i < MAX_ISDN_PHONE_NUMBER_IN_SERVICE; i++)
  {
    if (other.m_pPhoneNumRange[i] == NULL)
      m_pPhoneNumRange[i] = NULL;
    else
      m_pPhoneNumRange[i] = new CRtmIsdnPhoneNumberRange(*other.m_pPhoneNumRange[i]);
  }

  memcpy(m_ipAddressesList, other.m_ipAddressesList, sizeof(m_ipAddressesList));
}

//--------------------------------------------------------------------------
CRtmIsdnService::~CRtmIsdnService()
{
  int i = 0;
  for (int i = 0; i < MAX_ISDN_PHONE_NUMBER_IN_SERVICE; i++)
    PDELETE(m_pPhoneNumRange[i]);

  PDELETE(m_pSpanDef);

}

//--------------------------------------------------------------------------
void CRtmIsdnService::SerializeXml(CXMLDOMElement*& pFatherNode, DWORD objToken) const
{
  CXMLDOMElement* pIsdnSrvNode = pFatherNode->AddChildNode("ISDN_SERVICE");

  pIsdnSrvNode->AddChildNode("OBJ_TOKEN", m_updateCounter);

  BYTE bChanged = InsertUpdateCntChanged(pIsdnSrvNode, objToken);

  if (!bChanged)
  {
    return;
  }

  pIsdnSrvNode->AddChildNode("CHANGED", bChanged, _BOOL);

  if (bChanged)
  {
    CXMLDOMElement* pCommonIsdnSrvNode = pIsdnSrvNode->AddChildNode("COMMON_SERVICE_PARAMS");
    pCommonIsdnSrvNode->AddChildNode("NAME", m_serviceName);
    if (m_pSpanDef)
      m_pSpanDef->SerializeXml(pCommonIsdnSrvNode);

    CXMLDOMElement* pTempNode = pCommonIsdnSrvNode->AddChildNode("NET_PHONE_LIST");
    for (int i = 0; i < m_numb_of_phoneNumRanges; i++)
      m_pPhoneNumRange[i]->SerializeXml(pTempNode);

    pIsdnSrvNode->AddChildNode("VOICE", m_voice);
    pIsdnSrvNode->AddChildNode("NUM_PLAN", m_numPlan, NUM_PLAN_ENUM);
    pIsdnSrvNode->AddChildNode("DEFAULT_NUM_TYPE", m_dfltNumType, DFLT_NUM_TYPE_ENUM);
    pIsdnSrvNode->AddChildNode("NET_SPECIFIC", m_netSpecFacility, NET_SPEC_FACILITY_ENUM);
    pIsdnSrvNode->AddChildNode("DIAL_OUT_PREFIX", m_dialOutPrefix);
    pIsdnSrvNode->AddChildNode("MCU_CLI", m_mcuCli);
  } // end if (bChanged)
}

//--------------------------------------------------------------------------
void CRtmIsdnService::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
  SerializeXml(pFatherNode, UPDATE_CNT_BEGIN_END);
}

//--------------------------------------------------------------------------
int CRtmIsdnService::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
  int             nStatus = STATUS_OK;

  char*           ParentNodeName;
  CXMLDOMElement* pIsdnServiceNode;

  pActionNode->get_nodeName(&ParentNodeName);
  if (!strcmp(ParentNodeName, "ISDN_SERVICE"))
    pIsdnServiceNode = pActionNode;
  else
    GET_MANDATORY_CHILD_NODE(pActionNode, "ISDN_SERVICE", pIsdnServiceNode);

  char* pObjTypeValue = NULL;
  pIsdnServiceNode->getChildNodeValueByName("OBJ_TOKEN", &pObjTypeValue);
  if (pObjTypeValue)
    m_updateCounter = atoi(pObjTypeValue);

  CXMLDOMElement* pTempNode, * pListNode, * pCommonParamNode = NULL;
  WORD            tmp = 0;

  GET_MANDATORY_CHILD_NODE(pIsdnServiceNode, "COMMON_SERVICE_PARAMS", pCommonParamNode);
  if (pCommonParamNode)
  {
    GET_VALIDATE_CHILD(pCommonParamNode, "NAME", m_serviceName, _0_TO_RTM_ISDN_SERVICE_PROVIDER_NAME_LENGTH);
    if (strpbrk(m_serviceName, ","))
      return STATUS_ILLEGAL_SERVICE_NAME;

    GET_CHILD_NODE(pCommonParamNode, "SPAN_DEFINITION", pTempNode);
    if (pTempNode)
    {
      CRtmIsdnSpanDefinition* pSpanDef = new CRtmIsdnSpanDefinition;
      nStatus = pSpanDef->DeSerializeXml(pTempNode, pszError, "");

      if (nStatus != STATUS_OK)
      {
        POBJDELETE(pSpanDef);
        return nStatus;
      }

      POBJDELETE(m_pSpanDef);
      m_pSpanDef = pSpanDef;
    }

    GET_CHILD_NODE(pCommonParamNode, "NET_PHONE_LIST", pListNode);

    if (pListNode)
    {
      GET_FIRST_CHILD_NODE(pListNode, "NET_PHONE", pTempNode);
      m_numb_of_phoneNumRanges = 0;
      while (pTempNode && m_numb_of_phoneNumRanges < MAX_ISDN_PHONE_NUMBER_IN_SERVICE)
      {
        CRtmIsdnPhoneNumberRange* pPhoneNum = new CRtmIsdnPhoneNumberRange;
        nStatus = pPhoneNum->DeSerializeXml(pTempNode, pszError, "");

        if (nStatus != STATUS_OK)
        {
          POBJDELETE(pPhoneNum);
          return nStatus;
        }

        POBJDELETE(m_pPhoneNumRange[m_numb_of_phoneNumRanges]);
        m_pPhoneNumRange[m_numb_of_phoneNumRanges] = pPhoneNum;
        m_numb_of_phoneNumRanges++;
        GET_NEXT_CHILD_NODE(pListNode, "NET_PHONE", pTempNode);
      }
    }
  }

  GET_VALIDATE_CHILD(pIsdnServiceNode, "VOICE", &tmp, _0_TO_DWORD);
  m_voice = (eVoiceType)tmp;

  GET_VALIDATE_CHILD(pIsdnServiceNode, "NUM_PLAN", &tmp, NUM_PLAN_ENUM);
  m_numPlan = (eNumPlanType)tmp;

  GET_VALIDATE_CHILD(pIsdnServiceNode, "DEFAULT_NUM_TYPE", &tmp, DFLT_NUM_TYPE_ENUM);
  m_dfltNumType = (eDfltNumType)tmp;

  GET_VALIDATE_CHILD(pIsdnServiceNode, "NET_SPECIFIC", &tmp, NET_SPEC_FACILITY_ENUM);
  m_netSpecFacility = (eNetSpecFacilityType)tmp;

  GET_VALIDATE_CHILD(pIsdnServiceNode, "DIAL_OUT_PREFIX", m_dialOutPrefix, PHONE_NUMBER_DIGITS_LENGTH);

  GET_VALIDATE_CHILD(pIsdnServiceNode, "MCU_CLI", m_mcuCli, PHONE_NUMBER_DIGITS_LENGTH);

  return STATUS_OK;
}

//--------------------------------------------------------------------------
void CRtmIsdnService::SetIpAddressesList(int i)
{
  CRtmIsdnMngrProcess* pProcess           = (CRtmIsdnMngrProcess*)CRtmIsdnMngrProcess::GetProcess();
  int                  firstMediaBoardId  = pProcess->Get1stMediaBoardId(),
                       secondMediaBoardId = pProcess->Get2ndMediaBoardId(),
                       thirdMediaBoardId  = pProcess->Get3rdMediaBoardId(),
                       fourthMediaBoardId = pProcess->Get4thMediaBoardId();

  m_ipAddressesList[i].boardId = (WORD)i;

  if (firstMediaBoardId == i)
  {
    m_ipAddressesList[i].ipAddress_Rtm      = SystemIpStringToDWORD(RTM_ISDN_RTM_IP_BOARD_1);
    m_ipAddressesList[i].ipAddress_RtmMedia = SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_1);
  }
  else if (secondMediaBoardId == i)
  {
    m_ipAddressesList[i].ipAddress_Rtm      = SystemIpStringToDWORD(RTM_ISDN_RTM_IP_BOARD_2);
    m_ipAddressesList[i].ipAddress_RtmMedia = SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_2);
  }
  else if (thirdMediaBoardId == i)
  {
    m_ipAddressesList[i].ipAddress_Rtm      = SystemIpStringToDWORD(RTM_ISDN_RTM_IP_BOARD_3);
    m_ipAddressesList[i].ipAddress_RtmMedia = SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_3);
  }
  else if (fourthMediaBoardId == i)
  {
    m_ipAddressesList[i].ipAddress_Rtm      = SystemIpStringToDWORD(RTM_ISDN_RTM_IP_BOARD_4);
    m_ipAddressesList[i].ipAddress_RtmMedia = SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_4);
  }
  else // not FIRST/SECOND/THIRD/FOURTH_MEDIA_BOARD_SLOT_NUMBER
  {
    m_ipAddressesList[i].ipAddress_Rtm      = 0xFFFF;
    m_ipAddressesList[i].ipAddress_RtmMedia = 0xFFFF;
  }
}

//--------------------------------------------------------------------------
void CRtmIsdnService::IncreaseUpdateCounter()
{
  m_updateCounter++;
  if (m_updateCounter == 0xFFFFFFFF)
    m_updateCounter = 0;
}

//--------------------------------------------------------------------------
void CRtmIsdnService::SetSpanDef(const CRtmIsdnSpanDefinition& other)
{
  PDELETE(m_pSpanDef);
  m_pSpanDef = new CRtmIsdnSpanDefinition(other);
}

//--------------------------------------------------------------------------
int CRtmIsdnService::AddPhoneNumRange(const CRtmIsdnPhoneNumberRange& other)
{
  if (MAX_ISDN_PHONE_NUMBER_IN_SERVICE <= m_numb_of_phoneNumRanges)
    return STATUS_NUMBER_OF_PHONES_IN_SERVICE_EXCEEDED;

  if (true == IsPhonesAlreadyExistsInServiceRanges(other))
    return STATUS_PHONE_NUMBER_ALREADY_EXISTS;

  m_pPhoneNumRange[m_numb_of_phoneNumRanges] = new CRtmIsdnPhoneNumberRange(other);
  m_numb_of_phoneNumRanges++;

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CRtmIsdnService::CancelPhoneNumRange(const CRtmIsdnPhoneNumberRange& other)
{
  if (MAX_ISDN_PHONE_NUMBER_IN_SERVICE < m_numb_of_phoneNumRanges)
    return STATUS_NUMBER_OF_PHONES_IN_SERVICE_EXCEEDED;
  
  int ind = FindPhoneRange(other);
  if (ind == NOT_FIND)
    return STATUS_PHONE_NUMBER_NOT_EXISTS;

  PASSERTSTREAM_AND_RETURN_VALUE(ind >= MAX_ISDN_PHONE_NUMBER_IN_SERVICE,
    "ind has invalid value " << ind, STATUS_FAIL);

  PDELETE(m_pPhoneNumRange[ind]);

  int i = 0, j = 0;
  for (i = 0; i < (int)m_numb_of_phoneNumRanges; i++)
  {
    if (NULL == m_pPhoneNumRange[i])
      break;
  }

  for (j = i; j < (int)m_numb_of_phoneNumRanges-1; j++)
  {
    m_pPhoneNumRange[j] = m_pPhoneNumRange[j+1];
  }

  m_pPhoneNumRange[m_numb_of_phoneNumRanges-1] = NULL;

  m_numb_of_phoneNumRanges--;

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CRtmIsdnService::FindPhoneRange(const CRtmIsdnPhoneNumberRange& other)
{
  string otherFirst = other.m_firstPhoneNumber,
         otherLast  = other.m_lastPhoneNumber,
         curFirst,
         curLast;

  for (int i = 0; i < (int)m_numb_of_phoneNumRanges; i++)
  {
    curFirst = m_pPhoneNumRange[i]->m_firstPhoneNumber;
    curLast  = m_pPhoneNumRange[i]->m_lastPhoneNumber;

    if ((curFirst == otherFirst) && (curLast == otherLast))
    {
      return i;
    }
  }

  return NOT_FIND;
}

//--------------------------------------------------------------------------
int CRtmIsdnService::FindPhoneRange(const char* firstNum, const char* lastNum)
{
  for (int i = 0; i < (int)m_numb_of_phoneNumRanges; i++)
  {
    if ((!strncmp(m_pPhoneNumRange[i]->m_firstPhoneNumber, firstNum, ISDN_PHONE_NUMBER_DIGITS_LEN)) &&
        (!strncmp(m_pPhoneNumRange[i]->m_lastPhoneNumber, lastNum, ISDN_PHONE_NUMBER_DIGITS_LEN)))
    {
      return i;
    }
  }

  return NOT_FIND;
}

//--------------------------------------------------------------------------
bool CRtmIsdnService::IsPhonesAlreadyExistsInServiceRanges(const char* firstNum, const char* lastNum)
{
  string otherFirst = firstNum, // o1
         otherLast  = lastNum;  // o2
  string curFirst,        // c1
         curLast;         // c2

  for (int i = 0; i < (int)m_numb_of_phoneNumRanges; i++)
  {
    curFirst = m_pPhoneNumRange[i]->m_firstPhoneNumber;
    curLast  = m_pPhoneNumRange[i]->m_lastPhoneNumber;

    if (curFirst.length() != otherFirst.length())
      continue;

    if (((otherFirst >= curFirst) && (otherFirst <= curLast)) ||  // c1_o1_c2_o2, c1_o1_o2_c2
        ((otherLast >= curFirst) && (otherLast <= curLast)) ||    // o1_c1_o2_c2, c1_o1_o2_c2
        ((otherFirst <= curFirst) && (otherLast >= curLast)))     // o1_c1_c2_o2
    {
      return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------
bool CRtmIsdnService::IsPhonesAlreadyExistsInServiceRanges(const CRtmIsdnPhoneNumberRange& other)
{
  bool isExist = IsPhonesAlreadyExistsInServiceRanges(other.m_firstPhoneNumber, other.m_lastPhoneNumber);

  return isExist;
}

//--------------------------------------------------------------------------
CRtmIsdnPhoneNumberRange* CRtmIsdnService::GetFirstPhoneRange()
{
  m_ind_phoneRange = 1;
  return m_pPhoneNumRange[0];
}

//--------------------------------------------------------------------------
CRtmIsdnPhoneNumberRange* CRtmIsdnService::GetNextPhoneRange()
{
  CRtmIsdnPhoneNumberRange* pPhoneRange = NULL;

  if (m_ind_phoneRange < m_numb_of_phoneNumRanges)
  {
    pPhoneRange = m_pPhoneNumRange[m_ind_phoneRange];
    m_ind_phoneRange++;
  }

  return pPhoneRange;
}

//--------------------------------------------------------------------------
CRtmIsdnPhoneNumberRange* CRtmIsdnService::GetPhoneRangeInIndex(const int index)
{
  PASSERTSTREAM_AND_RETURN_VALUE(index >= MAX_ISDN_PHONE_NUMBER_IN_SERVICE, "Index out of range, Index:" << index, NULL);
  return m_pPhoneNumRange[index];
}

//--------------------------------------------------------------------------
DWORD CRtmIsdnService::GetIpAddress_Rtm(WORD boardId) const
{
  DWORD retAddress = 0xFFFF;

  if ((MAX_NUM_OF_BOARDS-1) > boardId)
  {
    retAddress = m_ipAddressesList[boardId].ipAddress_Rtm;
  }

  return retAddress;
}

//--------------------------------------------------------------------------
void CRtmIsdnService::SetIpAddress_Rtm(const WORD boardId, const DWORD ipAddress)
{
  if ((MAX_NUM_OF_BOARDS-1) > boardId)
  {
    m_ipAddressesList[boardId].ipAddress_Rtm = ipAddress;
  }
}

//--------------------------------------------------------------------------
DWORD CRtmIsdnService::GetIpAddress_RtmMedia(WORD boardId) const
{
  DWORD retAddress = 0xFFFF;

  if ((MAX_NUM_OF_BOARDS-1) > boardId)
  {
    retAddress = m_ipAddressesList[boardId].ipAddress_RtmMedia;
  }

  return retAddress;
}

//--------------------------------------------------------------------------
void CRtmIsdnService::SetIpAddress_RtmMedia(const WORD boardId, const DWORD ipAddress)
{
  if ((MAX_NUM_OF_BOARDS-1) > boardId)
  {
    m_ipAddressesList[boardId].ipAddress_RtmMedia = ipAddress;
  }
}

//--------------------------------------------------------------------------
void CRtmIsdnService::ConvertToRtmIsdnParamsMcmsStruct(RTM_ISDN_PARAMS_MCMS_S& isdnParamsStruct)
{
  memset(&isdnParamsStruct, 0, sizeof(RTM_ISDN_PARAMS_MCMS_S));

  // ===== 1. base service attributes
  memcpy(&(isdnParamsStruct.serviceName),
         &m_serviceName,
         RTM_ISDN_SERVICE_PROVIDER_NAME_LEN-1);

  memcpy(&(isdnParamsStruct.dialOutPrefix),
         &m_dialOutPrefix,
         ISDN_PHONE_NUMBER_DIGITS_LEN-1);

  memcpy(&(isdnParamsStruct.mcuCli),
         &m_mcuCli,
         ISDN_PHONE_NUMBER_DIGITS_LEN-1);

  isdnParamsStruct.dfltNumType     = (WORD)m_dfltNumType;
  isdnParamsStruct.numPlan         = (WORD)m_numPlan;
  isdnParamsStruct.voice           = (WORD)m_voice;
  isdnParamsStruct.netSpecFacility = (WORD)m_netSpecFacility;


  // ===== 2. Span Definition attributes
  RTM_ISDN_SPAN_DEFINITION_S curSpanDefStruct;
  memset(&curSpanDefStruct, 0, sizeof(RTM_ISDN_SPAN_DEFINITION_S));

  CRtmIsdnSpanDefinition* curSpanDef = GetSpanDef();

  curSpanDefStruct.spanType    = (WORD)(curSpanDef->GetSpanType());
  curSpanDefStruct.serviceType = (WORD)(curSpanDef->GetServiceType());
  curSpanDefStruct.framing     = (WORD)(curSpanDef->GetFraming());
  curSpanDefStruct.side        = (WORD)(curSpanDef->GetSide());
  curSpanDefStruct.lineCoding  = (WORD)(curSpanDef->GetLineCoding());
  curSpanDefStruct.switchType  = (WORD)(curSpanDef->GetSwitchType());

  isdnParamsStruct.spanDef = curSpanDefStruct;


  // ===== 3. PhoneRanges attributes
  for (int i = 0;
       ((i < m_numb_of_phoneNumRanges) && (i < MAX_ISDN_PHONE_NUMBER_IN_SERVICE));
       i++)
  {
    RTM_ISDN_PHONE_RANGE_S curPhoneRangeStruct;
    memset(&curPhoneRangeStruct, 0, sizeof(RTM_ISDN_PHONE_RANGE_S));

    if (NULL != GetPhoneRangeInIndex(i))
    {
      CRtmIsdnPhoneNumberRange curRange(*(GetPhoneRangeInIndex(i)));

      curPhoneRangeStruct.dialInGroupId = curRange.GetDialInGroupId();
      curPhoneRangeStruct.category      = curRange.GetCategory();
      curPhoneRangeStruct.firstPortId   = curRange.GetFirstPortId();

      memcpy(&(curPhoneRangeStruct.firstPhoneNumber),
             curRange.GetFirstPhoneNumber(),
             ISDN_PHONE_NUMBER_DIGITS_LEN-1);

      memcpy(&(curPhoneRangeStruct.lastPhoneNumber),
             curRange.GetLastPhoneNumber(),
             ISDN_PHONE_NUMBER_DIGITS_LEN-1);
    }

    memcpy(&(isdnParamsStruct.phoneRangesList[i]),
           &curPhoneRangeStruct,
           sizeof(RTM_ISDN_PHONE_RANGE_S));
  }

  // ===== 4. IP Addresses
  for (int i = 0; i < MAX_NUM_OF_BOARDS; i++)
  {
    isdnParamsStruct.ipAddressesList[i].boardId            = m_ipAddressesList[i].boardId;
    isdnParamsStruct.ipAddressesList[i].ipAddress_Rtm      = m_ipAddressesList[i].ipAddress_Rtm;
    isdnParamsStruct.ipAddressesList[i].ipAddress_RtmMedia = m_ipAddressesList[i].ipAddress_RtmMedia;
  }

  return;
}


////////////////////////////////////////////////////////////////////////////
//                        CRtmIsdnSpanDefinition
////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanDefinition::CRtmIsdnSpanDefinition()
{
  m_spanType            = eSpanTypeT1;
  m_serviceType         = eServiceTypePri;
  m_framing             = eFramingType_T1_Sf;
  m_side                = eSideTypeUser;
  m_lineCoding          = eLineCodingType_T1_B8ZS;
  m_switchType          = eSwitchType_T1_Att4ess;
  m_switch_type_NI1_NI2 = 0;
}

//--------------------------------------------------------------------------
CRtmIsdnSpanDefinition::CRtmIsdnSpanDefinition(const CRtmIsdnSpanDefinition& other)
  : CSerializeObject(other)
{
  m_spanType            = other.m_spanType;
  m_serviceType         = other.m_serviceType;
  m_framing             = other.m_framing;
  m_lineCoding          = other.m_lineCoding;
  m_switchType          = other.m_switchType;
  m_side                = other.m_side;
  m_switch_type_NI1_NI2 = other.m_switch_type_NI1_NI2;
}

//--------------------------------------------------------------------------
CRtmIsdnSpanDefinition::~CRtmIsdnSpanDefinition()
{ }

//--------------------------------------------------------------------------
CRtmIsdnSpanDefinition& CRtmIsdnSpanDefinition::operator =(const CRtmIsdnSpanDefinition& other)
{
  m_spanType            = other.m_spanType;
  m_serviceType         = other.m_serviceType;
  m_framing             = other.m_framing;
  m_lineCoding          = other.m_lineCoding;
  m_switchType          = other.m_switchType;
  m_side                = other.m_side;
  m_switch_type_NI1_NI2 = other.m_switch_type_NI1_NI2;

  return *this;
}

//--------------------------------------------------------------------------
void CRtmIsdnSpanDefinition::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
  CXMLDOMElement* pIsdnSpanParamNode = pFatherNode->AddChildNode("SPAN_DEFINITION");
  CXMLDOMElement* pTempNode;
  pIsdnSpanParamNode->AddChildNode("SPAN_TYPE", m_spanType, SPAN_TYPE_ENUM);
  pIsdnSpanParamNode->AddChildNode("SERVICE_TYPE", m_serviceType, ISDN_SERVICE_TYPE_ENUM);

  if (eSpanTypeT1 == m_spanType)
  {
    pIsdnSpanParamNode->AddChildNode("FRAMING", m_framing, T1_FRAMING_ENUM);
    pIsdnSpanParamNode->AddChildNode("LINE_CODING", m_lineCoding, T1_LINE_CODING_ENUM);
  }
  else
  {
    pIsdnSpanParamNode->AddChildNode("FRAMING", m_framing, E1_FRAMING_ENUM);
    pIsdnSpanParamNode->AddChildNode("LINE_CODING", m_lineCoding, E1_LINE_CODING_ENUM);
  }

  // V4.5 testings - NI-1 does not work at RMX
  // NI-2 working and including NI-1 ==>  change internaly NI-1 to NI-2
  eSwitchType temp_switchType = m_switchType;
  if (temp_switchType == eSwitchType_T1_NI2 && m_switch_type_NI1_NI2 == TRUE)
  {
    PTRACE(eLevelInfoNormal, "CRtmIsdnSpanDefinition::SerializeXml: serialize eSwitchType_T1_NI1 changed from eSwitchType_T1_NI2");
    temp_switchType = eSwitchType_T1_NI1;
  }

  pIsdnSpanParamNode->AddChildNode("SWITCH_TYPE", temp_switchType, SWITCH_TYPE_ENUM);
  pIsdnSpanParamNode->AddChildNode("SIDE", m_side, SIDE_ENUM);
}

//--------------------------------------------------------------------------
int CRtmIsdnSpanDefinition::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
  int             nStatus = STATUS_OK;

  CXMLDOMElement* pTempNode, * pListNode;

  WORD            tmp = 0;

  GET_VALIDATE_CHILD(pActionNode, "SPAN_TYPE", &tmp, SPAN_TYPE_ENUM);
  m_spanType = (eSpanType)tmp;

  GET_VALIDATE_CHILD(pActionNode, "SERVICE_TYPE", &tmp, ISDN_SERVICE_TYPE_ENUM);
  m_serviceType = (eServiceType)tmp;

  if (eSpanTypeT1 == m_spanType)
  {
    GET_VALIDATE_CHILD(pActionNode, "FRAMING", &tmp, T1_FRAMING_ENUM);
    m_framing = (eFramingType)tmp;

    GET_VALIDATE_CHILD(pActionNode, "LINE_CODING", &tmp, T1_LINE_CODING_ENUM);
    m_lineCoding = (eLineCodingType)tmp;
  }
  else
  {
    GET_VALIDATE_CHILD(pActionNode, "FRAMING", &tmp, E1_FRAMING_ENUM);
    m_framing = (eFramingType)tmp;

    GET_VALIDATE_CHILD(pActionNode, "LINE_CODING", &tmp, E1_LINE_CODING_ENUM);
    m_lineCoding = (eLineCodingType)tmp;
  }

  GET_VALIDATE_CHILD(pActionNode, "SWITCH_TYPE", &tmp, SWITCH_TYPE_ENUM);
  m_switchType = (eSwitchType)tmp;

  // V4.5 testings - NI-1 does not work at RMX
  // NI-2 working and including NI-1 ==>  change internaly NI-1 to NI-2
  if (m_switchType == eSwitchType_T1_NI1)
  {
    PTRACE(eLevelInfoNormal, "CRtmIsdnSpanDefinition::DeSerializeXml: changing eSwitchType_T1_NI1 to eSwitchType_T1_NI2");
    m_switchType          = eSwitchType_T1_NI2;
    m_switch_type_NI1_NI2 = TRUE;
  }

  GET_VALIDATE_CHILD(pActionNode, "SIDE", &tmp, SIDE_ENUM);
  m_side = (eSideType)tmp;

  return STATUS_OK;
}

