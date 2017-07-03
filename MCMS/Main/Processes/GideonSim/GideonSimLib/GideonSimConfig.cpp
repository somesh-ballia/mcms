//+========================================================================+
//                   GideonSimConfig.cpp                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimConfig.cpp                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimConfig.cpp: configuration of system.
//
//////////////////////////////////////////////////////////////////////


#include <string>
#include <stdio.h>
#include "psosxml.h"
#include "XmlDefines.h"
#include "XmlApi.h"
#include "StrArray.h"
#include "Trace.h"
#include "Macros.h"
#include "Segment.h"

#include "InitCommonStrings.h"
#include "GideonSim.h"
#include "GideonSimConfig.h"
#include "StatusesGeneral.h"
#include "CardsStructs.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"

#include "TraceStream.h"


/////////////////////////////////////////////////////////////////////////////
CGideonSimSystemCfg::CGideonSimSystemCfg()
	:m_BarakCardMode(BarakCard_NO_RTP_MODE)
{
	strncpy(m_szMplApiIp,"127.0.0.1",IP_ADDRESS_STR_LEN);
	strncpy(m_szBoxChassisId,"987654",MPL_SERIAL_NUM_LEN);

	m_wMplApiSwitchPortNumber	= 10004;
	m_wMplApiMfaPortNumber		= 10005;
	m_wMaxCardSlots				= MAX_NUM_OF_BOARDS;
	m_wGuiPortNumber			= 20004;
	//added by huiyu
	m_wPcmPortNumber			= 61000;
	m_wPcmFramePortNumber		= 20005;
	m_isSecured 				= FALSE;
	m_platformType				= eGideonLite; //eAmos

	m_ppCardsArr = new CCardCfg* [m_wMaxCardSlots];
	for( WORD i=0; i<m_wMaxCardSlots; i++ )
		m_ppCardsArr[i] = NULL;

	m_pBehaviourSwitch = new CBehaviourSwitch;
	m_pBehaviourMfa    = new CBehaviourMfa;
	m_pBehaviourIsdn   = new CBehaviourIsdn;


	int status = ReadXmlFile();
	bool isValid=CheckValidCfg();
	
	if( !isValid || status != STATUS_OK )
		WriteXmlFile();

//	Vasily:TEMP - create dummy card
//		m_wMaxCardSlots = MAX_NUM_OF_BOARDS;
//		for(WORD j=0; j<m_wMaxCardSlots; j++ )
//			POBJDELETE(m_ppCardsArr[j]);
//		PDELETEA(m_ppCardsArr);
//		m_ppCardsArr = new CCardCfg* [m_wMaxCardSlots];
//		CCardCfgMfaRtm*  pTempCardCfg = new CCardCfgMfaRtm(0,9707006,14);
//		for(WORD k=0; k<m_wMaxCardSlots; k++ )
//			m_ppCardsArr[k] = NULL;
//		m_ppCardsArr[0] = (CCardCfg*)pTempCardCfg;
//	Vasily:TEMP - end

	::SetGideonSystemCfg(this);
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimSystemCfg::~CGideonSimSystemCfg()
{
	::SetGideonSystemCfg(NULL);

	POBJDELETE(m_pBehaviourSwitch);
	POBJDELETE(m_pBehaviourMfa);
	POBJDELETE(m_pBehaviourIsdn);

	for( WORD i=0; i<m_wMaxCardSlots; i++ )
		POBJDELETE(m_ppCardsArr[i]);
	PDELETEA(m_ppCardsArr);
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimSystemCfg::CGideonSimSystemCfg(const CGideonSimSystemCfg& rOther)
	: CSerializeObject(rOther)
{
	*this = rOther;
}


/////////////////////////////////////////////////////////////////////////////
CGideonSimSystemCfg& CGideonSimSystemCfg::operator= (const CGideonSimSystemCfg& rOther)
{
	if( this == &rOther )
		return *this;

	//  clean old cards 
	for( WORD i=0; i<m_wMaxCardSlots; i++ )
		POBJDELETE(m_ppCardsArr[i]);
	PDELETEA(m_ppCardsArr);
	POBJDELETE(m_pBehaviourSwitch);
	POBJDELETE(m_pBehaviourMfa);
	POBJDELETE(m_pBehaviourIsdn);

	strncpy(m_szBoxChassisId, rOther.m_szBoxChassisId, MPL_SERIAL_NUM_LEN );
	strncpy(m_szMplApiIp,rOther.m_szMplApiIp,IP_ADDRESS_STR_LEN);

	m_wMplApiSwitchPortNumber	= rOther.m_wMplApiSwitchPortNumber;
	m_wMplApiMfaPortNumber		= rOther.m_wMplApiMfaPortNumber;
	m_wMaxCardSlots				= rOther.m_wMaxCardSlots;
	m_wGuiPortNumber			= rOther.m_wGuiPortNumber;
	//added by huiyu
	m_wPcmPortNumber			= rOther.m_wPcmPortNumber;
	m_wPcmFramePortNumber		= rOther.m_wPcmFramePortNumber;
	m_isSecured 				= rOther.m_isSecured;
	m_platformType				= rOther.m_platformType;

	m_ppCardsArr = new CCardCfg*[m_wMaxCardSlots];
	for( WORD k=0; k<m_wMaxCardSlots; k++ ) {
		if( rOther.m_ppCardsArr[k] )
			m_ppCardsArr[k] = rOther.m_ppCardsArr[k]->CreateCopy();
	}

	m_pBehaviourSwitch = new CBehaviourSwitch(*(rOther.m_pBehaviourSwitch));
	m_pBehaviourMfa    = new CBehaviourMfa(*(rOther.m_pBehaviourMfa));
	m_pBehaviourIsdn   = new CBehaviourIsdn(*(rOther.m_pBehaviourIsdn));
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSystemCfg::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// create <SYSTEM_DETAILS> section
	CXMLDOMElement* pSimDetailsNode = pFatherNode->AddChildNode("SYSTEM_DETAILS");

	// <SYSTEM_DETAILS> fields
	pSimDetailsNode->AddChildNode("BOX_CHASSIS_ID", m_szBoxChassisId);
	pSimDetailsNode->AddChildNode("MPL_API_IP_ADDRESS",m_szMplApiIp);
	pSimDetailsNode->AddChildNode("MPL_API_SWITCH_PORT", m_wMplApiSwitchPortNumber);
	pSimDetailsNode->AddChildNode("MPL_API_MFA_PORT", m_wMplApiMfaPortNumber);
	pSimDetailsNode->AddChildNode("MAX_CARD_SLOTS", m_wMaxCardSlots);
	pSimDetailsNode->AddChildNode("GUI_PORT", m_wGuiPortNumber);
	//added by huiyu
	pSimDetailsNode->AddChildNode("PCM_PORT", m_wPcmPortNumber);
	pSimDetailsNode->AddChildNode("PCM_FRAME_PORT", m_wPcmFramePortNumber);	
	pSimDetailsNode->AddChildNode("IS_SECURED", m_isSecured,_BOOL);
	pSimDetailsNode->AddChildNode("PLATFORM_TYPE", m_platformType);
	pSimDetailsNode->AddChildNode("CARD_BARAK_WORKING_MODE", m_BarakCardMode);

	// create <CARDS_LIST> section
	CXMLDOMElement* pCardsListNode = pFatherNode->AddChildNode("CARDS_LIST");
	for( WORD i=0; i<m_wMaxCardSlots; i++ ) {
		// if card in slot 'i' exists - serialize it
		if( ( m_ppCardsArr != NULL ) && CPObject::IsValidPObjectPtr(m_ppCardsArr[i]) )
			m_ppCardsArr[i]->SerializeXml(pCardsListNode);
	}

	m_pBehaviourSwitch->SerializeXml(pFatherNode);
	m_pBehaviourMfa->SerializeXml(pFatherNode);
}

/////////////////////////////////////////////////////////////////////////////
int	CGideonSimSystemCfg::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{

	//  clean old cards info
	WORD i=0;
	for( i=0; i<m_wMaxCardSlots; i++ )
		POBJDELETE(m_ppCardsArr[i]);

	int		nStatus			= STATUS_OK;
	char*	pszChildName	= NULL;

	// get <SYSTEM_DETAILS> section
	CXMLDOMElement*		pSimDetailsNode = NULL;
	GET_CHILD_NODE(pActionNode,"SYSTEM_DETAILS",pSimDetailsNode);

	// get fields from <SYSTEM_DETAILS> section
	if( pSimDetailsNode ) {
		GET_VALIDATE_CHILD(pSimDetailsNode,"BOX_CHASSIS_ID", m_szBoxChassisId, _0_TO_MPL_SERIAL_NUM_LENGTH);
		GET_VALIDATE_CHILD(pSimDetailsNode,"MPL_API_IP_ADDRESS",m_szMplApiIp,_1_TO_IP_ADDRESS_LENGTH);
		GET_VALIDATE_CHILD(pSimDetailsNode,"MPL_API_SWITCH_PORT",&m_wMplApiSwitchPortNumber,_0_TO_WORD);
		GET_VALIDATE_CHILD(pSimDetailsNode,"MPL_API_MFA_PORT",&m_wMplApiMfaPortNumber,_0_TO_WORD);
		GET_VALIDATE_CHILD(pSimDetailsNode,"MAX_CARD_SLOTS",&m_wMaxCardSlots,_0_TO_WORD);
		GET_VALIDATE_CHILD(pSimDetailsNode,"GUI_PORT",&m_wGuiPortNumber,_0_TO_WORD);
		//added by huiyu
		GET_VALIDATE_CHILD(pSimDetailsNode,"PCM_PORT",&m_wPcmPortNumber,_0_TO_WORD);
		GET_VALIDATE_CHILD(pSimDetailsNode,"PCM_FRAME_PORT",&m_wPcmFramePortNumber,_0_TO_WORD);		
		GET_VALIDATE_CHILD(pSimDetailsNode,"IS_SECURED",&m_isSecured,_BOOL);
		GET_VALIDATE_CHILD(pSimDetailsNode,"PLATFORM_TYPE",&m_platformType,_0_TO_WORD);
		GET_VALIDATE_CHILD(pSimDetailsNode,"CARD_BARAK_WORKING_MODE",&m_BarakCardMode,_0_TO_WORD);
	}
	if( !m_wMaxCardSlots )
		m_wMaxCardSlots = 1;
	PDELETEA(m_ppCardsArr);
	m_ppCardsArr = new CCardCfg*[m_wMaxCardSlots];
	for( i=0; i<m_wMaxCardSlots; i++ )
		m_ppCardsArr[i] = NULL;

	// get <CARDS_LIST> section
	CXMLDOMElement*		pCardsListNode = NULL;
	GET_CHILD_NODE(pActionNode,"CARDS_LIST",pCardsListNode);

	// get cards from <CARDS_LIST> section
	if( pCardsListNode ) {

		WORD				boardId   = 0;
		CCardCfg*			pCardCfg  = NULL;
		CXMLDOMElement*		pCardNode = NULL;
		pCardsListNode->firstChildNode(&pCardNode);

		while( pCardNode && nStatus == STATUS_OK ) {

			pCardCfg = NULL;
			WORD RtmCardTemp = eRtmIsdn; //the default
			WORD cardTemp = 0;

			pCardNode->get_nodeName(&pszChildName);

			//  find types of cards:
			//
			if( !strcmp(pszChildName,"CARD_SWITCH") ) {
				pCardCfg = new CCardCfgSwitch;
			} else if( !strcmp(pszChildName,"CARD_MFA") ) {

				GET_VALIDATE_CHILD(pCardNode,"RTM_CARD_TYPE",&RtmCardTemp,_0_TO_WORD);
				pCardCfg = new CCardCfgMfa;
				((CCardCfgMfa*)pCardCfg)->SetRtmCardType((eCardType)RtmCardTemp);


				GET_VALIDATE_CHILD(pCardNode,"CARD_TYPE",&cardTemp,_0_TO_WORD);
				((CCardCfgMfa*)pCardCfg)->SetDetailedCardType((eCardType)cardTemp);


			} else if( !strcmp(pszChildName,"CARD_BARAK") ) {
				GET_VALIDATE_CHILD(pCardNode,"RTM_CARD_TYPE",&RtmCardTemp,_0_TO_WORD);
				pCardCfg = new CCardCfgBarak;
				((CCardCfgBarak*)pCardCfg)->SetRtmCardType((eCardType)RtmCardTemp);


				GET_VALIDATE_CHILD(pCardNode,"CARD_TYPE",&cardTemp,_0_TO_WORD);
				((CCardCfgBarak*)pCardCfg)->SetDetailedCardType((eCardType)cardTemp);



			} else if( !strcmp(pszChildName,"CARD_BREEZE") ) {
				GET_VALIDATE_CHILD(pCardNode,"RTM_CARD_TYPE",&RtmCardTemp,_0_TO_WORD);
				pCardCfg = new CCardCfgBreeze;
				((CCardCfgBarak*)pCardCfg)->SetRtmCardType((eCardType)RtmCardTemp);

				GET_VALIDATE_CHILD(pCardNode,"CARD_TYPE",&cardTemp,_0_TO_WORD);
				((CCardCfgBarak*)pCardCfg)->SetDetailedCardType((eCardType)cardTemp);


			} else if( !strcmp(pszChildName,"CARD_MPMRX") ) {
				GET_VALIDATE_CHILD(pCardNode,"RTM_CARD_TYPE",&RtmCardTemp,_0_TO_WORD);
				pCardCfg = new CCardCfgMpmRx;
				((CCardCfgBarak*)pCardCfg)->SetRtmCardType((eCardType)RtmCardTemp);

				GET_VALIDATE_CHILD(pCardNode,"CARD_TYPE",&cardTemp,_0_TO_WORD);
				((CCardCfgBarak*)pCardCfg)->SetDetailedCardType((eCardType)cardTemp);


			} else if( !strcmp(pszChildName,"CARD_GIDEON_LITE") ) {
				GET_VALIDATE_CHILD(pCardNode,"RTM_CARD_TYPE",&RtmCardTemp,_0_TO_WORD);
				pCardCfg = new CCardGideonLite;
				((CCardCfgBarak*)pCardCfg)->SetRtmCardType((eCardType)RtmCardTemp);

				GET_VALIDATE_CHILD(pCardNode,"CARD_TYPE",&cardTemp,_0_TO_WORD);
				((CCardCfgBarak*)pCardCfg)->SetDetailedCardType((eCardType)cardTemp);


			}




			// if type of card is valid
			if( CPObject::IsValidPObjectPtr(pCardCfg) ) {

				// deserialize details of card
				nStatus = pCardCfg->DeSerializeXml(pCardNode,pszError,action);
				if( nStatus )
					break;

				boardId = pCardCfg->GetBoardId();
				if( boardId < m_wMaxCardSlots )
					m_ppCardsArr[boardId] = pCardCfg;
				else 
					POBJDELETE(pCardCfg);
			}
			pCardsListNode->nextChildNode(&pCardNode);
		}
	}

	CXMLDOMElement*	 pSwitchNode = NULL;
	GET_CHILD_NODE(pActionNode,"SWITCH_BEHAVIOUR",pSwitchNode);

	// get fields from <SWITCH_BEHAVIOUR> section
	if( pSwitchNode ) {
		nStatus = m_pBehaviourSwitch->DeSerializeXml(pSwitchNode,pszError,action);
	}

	CXMLDOMElement*	 pMfaNode = NULL;
	GET_CHILD_NODE(pActionNode,"MFA_BEHAVIOUR",pMfaNode);

	// get fields from <MFA_BEHAVIOUR> section
	if( pMfaNode ) {
		nStatus = m_pBehaviourMfa->DeSerializeXml(pMfaNode,pszError,action);
	}

	// get field from <ISDN_BEHAVIOUR> section
	CXMLDOMElement*	pIsdnNode = NULL;
	GET_CHILD_NODE(pActionNode,"ISDN_BEHAVIOUR", pIsdnNode);
	if( pIsdnNode ) {
		nStatus = m_pBehaviourIsdn->DeSerializeXml(pIsdnNode,pszError,action);
	}

	return nStatus;	
}

bool CGideonSimSystemCfg::CheckValidCfg()
{
	bool status=true;
	if(m_BarakCardMode==BarakCard_REAL_RTP_MODE && m_ppCardsArr) {//at most one Barak card can be started under REAL_RTP_MODE
		int barakCard_cnt=0;
		for( int i=0; i<m_wMaxCardSlots; ++i ) {
			// if card in slot 'i' exists - serialize it
			if( CPObject::IsValidPObjectPtr(m_ppCardsArr[i]) ) {
				if(m_ppCardsArr[i]->GetCardType()==eCardBarak) {
					++barakCard_cnt;
				}
				if(barakCard_cnt>1) {
					status=false;
					POBJDELETE(m_ppCardsArr[i]);
					TRACESTR(eLevelError) << "CGideonSimSystemCfg::CheckValidCfg :  Invalid cfg detected--More than one Barak Card "
											<<"are configured under REAL_RTP_MODE. Odd BaraCard will be removed.";
				}
			}
		}
	}

	//other checking can be added here...
	
	return status;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfg* CGideonSimSystemCfg::GetCardCfg(const WORD index)
{
	if( index < m_wMaxCardSlots )
		return m_ppCardsArr[index];
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSystemCfg::SetCardCfg( const WORD index, CCardCfg* cardCfg)
{
	if( index < m_wMaxCardSlots )
		m_ppCardsArr[index] = cardCfg;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSystemCfg::ClearCardCfg(const WORD index)
{
	if( index < m_wMaxCardSlots )
		POBJDELETE(m_ppCardsArr[index]);
}

/////////////////////////////////////////////////////////////////////////////
WORD CGideonSimSystemCfg::GetSwitchConfigTime() const
{
	return m_pBehaviourSwitch->GetConfigTime();
}

/////////////////////////////////////////////////////////////////////////////
WORD CGideonSimSystemCfg::GetMfaUnitConfigTime() const
{
	return m_pBehaviourMfa->GetUnitConfigReqTime();
}

/////////////////////////////////////////////////////////////////////////////
WORD CGideonSimSystemCfg::GetMfaMediaConfigTime() const
{
	return m_pBehaviourMfa->GetMediaConfigReqTime();
}

/////////////////////////////////////////////////////////////////////////////
WORD CGideonSimSystemCfg::GetMfaSpeakerChangeTime() const
{
	return m_pBehaviourMfa->GetSpeakerChangeTime();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CGideonSimSystemCfg::GetIsdnCapsFlag() const
{
    return m_pBehaviourIsdn->GetCapsFlag();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CGideonSimSystemCfg::GetIsdnXmitModeFlag() const
{
    return m_pBehaviourIsdn->GetXmitModeFlag();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CGideonSimSystemCfg::GetIsdnH230Flag() const
{
    return m_pBehaviourIsdn->GetH230Flag();
}
/////////////////////////////////////////////////////////////////////////////
void CGideonSimSystemCfg::Serialize(CSegment& rSegment) const
{
	rSegment.Put((BYTE*)m_szBoxChassisId, MPL_SERIAL_NUM_LEN);
	
	rSegment << (DWORD)m_wMplApiSwitchPortNumber
			 << (DWORD)m_wMplApiMfaPortNumber
			 << (DWORD)m_wMaxCardSlots
			 << (DWORD)m_wGuiPortNumber
			 << (DWORD)m_platformType
			 << (DWORD)m_wPcmPortNumber
			 << (DWORD)m_wPcmFramePortNumber;

	rSegment.Put((BYTE*)m_szMplApiIp,IP_ADDRESS_STR_LEN);

	m_pBehaviourSwitch->Serialize(rSegment);
	m_pBehaviourMfa->Serialize(rSegment);
	
	rSegment << (BYTE)m_isSecured;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSystemCfg::DeSerialize(CSegment& rSegment)
{
	rSegment.Get((BYTE*)m_szBoxChassisId, MPL_SERIAL_NUM_LEN);

	DWORD  temp = 0;
	rSegment >> temp;
	m_wMplApiSwitchPortNumber = (WORD)temp;
	rSegment >> temp;
	m_wMplApiMfaPortNumber    = (WORD)temp;
	rSegment >> temp;
	m_wMaxCardSlots   = (WORD)temp;
	rSegment >> temp;
	m_wGuiPortNumber  = (WORD)temp;
	rSegment >> temp;
	m_platformType = (WORD)temp;
	rSegment >> temp;
	m_wPcmPortNumber  = (WORD)temp;
	rSegment >> temp;
	m_wPcmFramePortNumber  = (WORD)temp;

	rSegment.Get((BYTE*)m_szMplApiIp,IP_ADDRESS_STR_LEN);

	m_pBehaviourSwitch->DeSerialize(rSegment);
	m_pBehaviourMfa->DeSerialize(rSegment);
	
	rSegment >> temp;
	m_isSecured  = (BYTE)temp;	
}

/////////////////////////////////////////////////////////////////////////////
int CGideonSimSystemCfg::IsEqual(const CGideonSimSystemCfg& rOther) const
{
	if( strncmp(m_szBoxChassisId,rOther.m_szBoxChassisId,MPL_SERIAL_NUM_LEN) != 0 )
		return FALSE;
	if( m_wMplApiSwitchPortNumber != rOther.m_wMplApiSwitchPortNumber )
		return FALSE;
	if( m_wMplApiMfaPortNumber != rOther.m_wMplApiMfaPortNumber )
		return FALSE;
	if( m_wMaxCardSlots != rOther.m_wMaxCardSlots )
		return FALSE;
	if( m_wGuiPortNumber != rOther.m_wGuiPortNumber )
		return FALSE;
	if( m_platformType != rOther.m_platformType )
		return FALSE;
	if( m_wPcmPortNumber != rOther.m_wPcmPortNumber )
		return FALSE;
	if( m_wPcmFramePortNumber != rOther.m_wPcmFramePortNumber )
		return FALSE;
	if( strncmp(m_szMplApiIp,rOther.m_szMplApiIp,IP_ADDRESS_STR_LEN) != 0 )
		return FALSE;
	if( !m_pBehaviourSwitch->IsEqual(*(rOther.m_pBehaviourSwitch)) )
		return FALSE;
	if( !m_pBehaviourMfa->IsEqual(*(rOther.m_pBehaviourMfa)) )
		return FALSE;
	if ( m_isSecured != rOther.m_isSecured )
		return FALSE;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CCardCfg::CCardCfg()
{
	m_wBoardId			= 0;
	strncpy(m_szSerialNumber,"621327",MPL_SERIAL_NUM_LEN);
}

/////////////////////////////////////////////////////////////////////////////
CCardCfg::~CCardCfg()
{
}

/////////////////////////////////////////////////////////////////////////////
CCardCfg::CCardCfg(const CCardCfg& rOther)
 	: CSerializeObject(rOther)
{
	*this = rOther;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfg& CCardCfg::operator= (const CCardCfg& other)
{
	if( this == &other )
		return *this;

	m_wBoardId = other.m_wBoardId;
	strncpy(m_szSerialNumber, other.m_szSerialNumber, MPL_SERIAL_NUM_LEN );

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CCardCfg::SerializeXml(CXMLDOMElement*& pCardNode) const
{
	// serializes only common fields, doesn't create item
	// CardNode - it's not father node, it's my node

	// fields in card' details
	pCardNode->AddChildNode("BOARD_ID", m_wBoardId);
	pCardNode->AddChildNode("MPL_SERIAL_NUMBER", m_szSerialNumber);
}

/////////////////////////////////////////////////////////////////////////////
int	CCardCfg::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int	nStatus	= STATUS_OK;

	// get common card fields
	GET_VALIDATE_CHILD(pActionNode,"BOARD_ID",&m_wBoardId,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"MPL_SERIAL_NUMBER",m_szSerialNumber,_0_TO_MPL_SERIAL_NUM_LENGTH);

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CCardCfg::IsMediaCard()
{
	switch(GetCardType())
	{
		case eCardMfa:
		case eCardBarak:
		case eCardBreeze:
		case eCardMpmRx:
			return TRUE;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CCardCfgSwitch::CCardCfgSwitch()
{
	m_isActive = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgSwitch::~CCardCfgSwitch()
{
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgSwitch::CCardCfgSwitch(const CCardCfgSwitch& rOther)
 	: CCardCfg(rOther)
{
	*this = rOther;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgSwitch& CCardCfgSwitch::operator= (const CCardCfgSwitch& other)
{
	if( this == &other )
		return *this;

	*((CCardCfg*)this) = *((CCardCfg*)&other);
	m_isActive	= other.m_isActive;

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
CCardCfg* CCardCfgSwitch::CreateCopy()
{
	CCardCfgSwitch* pMyCopy = new CCardCfgSwitch(*this);
	return (CCardCfg*)pMyCopy;
}

/////////////////////////////////////////////////////////////////////////////
void CCardCfgSwitch::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// common card's fields will serialize in base class
	CXMLDOMElement* pCardNode = pFatherNode->AddChildNode("CARD_SWITCH");
	CCardCfg::SerializeXml(pCardNode);
	// specific fields of SWITCH card
	if (m_isActive)
		pCardNode->AddChildNode("IS_ACTIVE", "true");
	else
		pCardNode->AddChildNode("IS_ACTIVE", "false");
}

/////////////////////////////////////////////////////////////////////////////
int	CCardCfgSwitch::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int	nStatus	= STATUS_OK;

	// get common card fields
	nStatus = CCardCfg::DeSerializeXml(pActionNode,pszError,action);
	if( nStatus == STATUS_OK ) {
		// get SWITCH card specific fields
		GET_VALIDATE_CHILD(pActionNode,"IS_ACTIVE",&m_isActive,_BOOL);
	}

	return nStatus;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CBaseMediaCardCfg::CBaseMediaCardCfg()
{
	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		m_unitList[i].unitId = 65535;
		m_unitList[i].unitStatus = 0;
	}	
	
	m_isRtmAttached = FALSE;
}
/////////////////////////////////////////////////////////////////////////////
CBaseMediaCardCfg::~CBaseMediaCardCfg()
{
	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		m_unitList[i].unitId = 65535;
		m_unitList[i].unitStatus = 0;
	}	
}
/////////////////////////////////////////////////////////////////////////////
CBaseMediaCardCfg& CBaseMediaCardCfg::operator= (const CBaseMediaCardCfg& other)
{
	m_isRtmAttached  = other.m_isRtmAttached;
	m_detailedCardType  = other.m_detailedCardType;
	
	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
		m_unitList[i] = other.m_unitList[i];
	
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CCardCfgMfa::CCardCfgMfa()
{
	m_detailedCardType = eMfa_26;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgMfa::CCardCfgMfa(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm)
{
	m_wBoardId       = wBoard;
	strncpy(m_szSerialNumber, szSerial, sizeof(m_szSerialNumber) - 1);
	m_szSerialNumber[sizeof(m_szSerialNumber) - 1] = '\0';
	SetRtmAttached(isRtm);
	m_detailedCardType = cardType;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgMfa::~CCardCfgMfa()
{
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgMfa::CCardCfgMfa(const CCardCfgMfa& rOther)
 	: CCardCfg(rOther), CBaseMediaCardCfg(rOther)
{
	*this = rOther;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgMfa& CCardCfgMfa::operator= (const CCardCfgMfa& other)
{
	if( this == &other )
		return *this;

	*((CCardCfg*)this) = *((CCardCfg*)&other);
	*((CBaseMediaCardCfg*)this) = *((CBaseMediaCardCfg*)&other);

	m_wBoardId       = other.m_wBoardId;
	strncpy(m_szSerialNumber, other.m_szSerialNumber, MPL_SERIAL_NUM_LEN );

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
CCardCfg* CCardCfgMfa::CreateCopy()
{
	CCardCfgMfa* pMyCopy = new CCardCfgMfa(*this);
	return (CCardCfg*)pMyCopy;
}

/////////////////////////////////////////////////////////////////////////////
void CCardCfgMfa::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// common card's fields will serialize in base class
	CXMLDOMElement* pCardNode = pFatherNode->AddChildNode("CARD_MFA");
	CCardCfg::SerializeXml(pCardNode);
	// specific fields of MFA card
	pCardNode->AddChildNode("CARD_TYPE", m_detailedCardType);
	pCardNode->AddChildNode("RTM_ATTACHED", m_isRtmAttached,_BOOL);
	
	// create <UNIT_LIST> section
	CXMLDOMElement* pUnitsListNode = pCardNode->AddChildNode("UNIT_LIST");
	CUnitCfg*			pUnitCfg  = NULL;
	UNIT_S currUnit;
	
	for( WORD i=0; i<MAX_NUM_OF_UNITS; i++ )
	{
		currUnit = GetUnit(i);
		if (currUnit.unitId != 65535)
		{
			//TRACEINTO << "serialize currUnit.unitId:" << currUnit.unitId ;			
			pUnitCfg = new CUnitCfg(currUnit.unitId, currUnit.unitStatus); AUTO_DELETE(pUnitCfg);
			// if unit in slot 'i' exists - serialize it
			if( CPObject::IsValidPObjectPtr(pUnitCfg) )
				pUnitCfg->SerializeXml(pUnitsListNode);
		}
	}
	
}

/////////////////////////////////////////////////////////////////////////////
int	CCardCfgMfa::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int	nStatus	= STATUS_OK;

	// get common card fields
	nStatus = CCardCfg::DeSerializeXml(pActionNode,pszError,action);
	if( nStatus == STATUS_OK )
	{
		// get MFA card specific fields
		GET_VALIDATE_CHILD(pActionNode,"CARD_TYPE",(WORD*)&m_detailedCardType,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"RTM_ATTACHED",&m_isRtmAttached,_BOOL);		
				
		int	nTmpStatus = nStatus;
		//Get Unit status list - UNIT_LIST
		CXMLDOMElement*		pUnitsListNode = NULL;
		GET_CHILD_NODE(pActionNode, "UNIT_LIST", pUnitsListNode);
		
		if (pUnitsListNode == NULL)
		{
			return nTmpStatus;
		}
		
		if( pUnitsListNode )
		{
			WORD				unitId   = 0;
			DWORD				unitStatus = 0;
			
			CUnitCfg*			pUnitCfg  = NULL;
			CXMLDOMElement*		pUnitNode = NULL;

			pUnitsListNode->firstChildNode(&pUnitNode);
			
			while( pUnitNode && nStatus == STATUS_OK )
			{
				pUnitCfg = new CUnitCfg; AUTO_DELETE(pUnitCfg);

				// if type of unit is valid
				if( CPObject::IsValidPObjectPtr(pUnitCfg) )
				{
					// deserialize details of unit
					nStatus = pUnitCfg->DeSerializeXml(pUnitNode, pszError, action);
					if( nStatus )
						break;
	
					unitId = pUnitCfg->GetUnitId();
					
					m_unitList[unitId].unitId = unitId;
					m_unitList[unitId].unitStatus = pUnitCfg->GetUnitStatus();
					
					POBJDELETE(pUnitCfg);
				}
				pUnitsListNode->nextChildNode(&pUnitNode);
			}
		}
		
	}
	
	return nStatus;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CCardCfgBarak::CCardCfgBarak()
{
	m_detailedCardType = eMpmPlus_40;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgBarak::CCardCfgBarak(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm)
{
	m_wBoardId       = wBoard;
	strncpy(m_szSerialNumber, szSerial, sizeof(m_szSerialNumber) - 1);
	m_szSerialNumber[sizeof(m_szSerialNumber) - 1] = '\0';
	SetRtmAttached(isRtm);
	m_detailedCardType = cardType;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgBarak::~CCardCfgBarak()
{
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgBarak::CCardCfgBarak(const CCardCfgBarak& rOther)
 	: CCardCfg(rOther), CBaseMediaCardCfg(rOther)
{
	*this = rOther;
}

/////////////////////////////////////////////////////////////////////////////
CCardCfgBarak& CCardCfgBarak::operator= (const CCardCfgBarak& other)
{
	if( this == &other )
		return *this;

	*((CCardCfg*)this) = *((CCardCfg*)&other);
	*((CBaseMediaCardCfg*)this) = *((CBaseMediaCardCfg*)&other);

	m_wBoardId       = other.m_wBoardId;
	strncpy(m_szSerialNumber, other.m_szSerialNumber, MPL_SERIAL_NUM_LEN );

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
CCardCfg* CCardCfgBarak::CreateCopy()
{
	CCardCfgBarak* pMyCopy = new CCardCfgBarak(*this);
	return (CCardCfg*)pMyCopy;
}

/////////////////////////////////////////////////////////////////////////////
void CCardCfgBarak::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// common card's fields will serialize in base class
	CXMLDOMElement* pCardNode;
	if(GetCardType() == eCardBreeze)
		pCardNode = pFatherNode->AddChildNode("CARD_BREEZE");
	else if (GetCardType() == eCardMpmRx)
	    pCardNode = pFatherNode->AddChildNode("CARD_MPMRX");
	else	
		pCardNode = pFatherNode->AddChildNode("CARD_BARAK");
	CCardCfg::SerializeXml(pCardNode);
	// specific fields of Barak card
	pCardNode->AddChildNode("CARD_TYPE", m_detailedCardType);
	pCardNode->AddChildNode("RTM_ATTACHED", m_isRtmAttached,_BOOL);
	
	// create <UNIT_LIST> section
	CXMLDOMElement* pUnitsListNode = pCardNode->AddChildNode("UNIT_LIST");
	CUnitCfg*			pUnitCfg  = NULL;
	UNIT_S currUnit;
	
	for( WORD i=0; i<MAX_NUM_OF_UNITS; i++ )
	{
		currUnit = GetUnit(i);
		if (currUnit.unitId != 65535)
		{
			//TRACEINTO << "serialize currUnit.unitId:" << currUnit.unitId ;			
			pUnitCfg = new CUnitCfg(currUnit.unitId, currUnit.unitStatus);
			// if unit in slot 'i' exists - serialize it
			if( CPObject::IsValidPObjectPtr(pUnitCfg) )
				pUnitCfg->SerializeXml(pUnitsListNode);
			POBJDELETE(pUnitCfg);
		}
	}
	
}

/////////////////////////////////////////////////////////////////////////////
int	CCardCfgBarak::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int	nStatus	= STATUS_OK;

	// get common card fields
	nStatus = CCardCfg::DeSerializeXml(pActionNode,pszError,action);
	if( nStatus == STATUS_OK )
	{
		// get Barak card specific fields
		GET_VALIDATE_CHILD(pActionNode,"CARD_TYPE",(WORD*)&m_detailedCardType,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"RTM_ATTACHED",&m_isRtmAttached,_BOOL);
		
		int	nTmpStatus = nStatus;
		//Get Unit status list - UNIT_LIST
		CXMLDOMElement*		pUnitsListNode = NULL;
		GET_CHILD_NODE(pActionNode, "UNIT_LIST", pUnitsListNode);
		
		if (pUnitsListNode == NULL)
		{
			return nTmpStatus;
		}
		
		if( pUnitsListNode )
		{
			WORD				unitId   = 0;
			DWORD				unitStatus = 0;
			
			CUnitCfg*			pUnitCfg  = NULL;
			CXMLDOMElement*		pUnitNode = NULL;

			pUnitsListNode->firstChildNode(&pUnitNode);
			
			while( pUnitNode && nStatus == STATUS_OK )
			{
				pUnitCfg = new CUnitCfg; AUTO_DELETE(pUnitCfg);

				// if type of unit is valid
				if( CPObject::IsValidPObjectPtr(pUnitCfg) )
				{
					// deserialize details of unit
					nStatus = pUnitCfg->DeSerializeXml(pUnitNode, pszError, action);
					if( nStatus )
						break;
	
					unitId = pUnitCfg->GetUnitId();
					
					m_unitList[unitId].unitId = unitId;
					m_unitList[unitId].unitStatus = pUnitCfg->GetUnitStatus();
					
					POBJDELETE(pUnitCfg);
				}
				pUnitsListNode->nextChildNode(&pUnitNode);
			}
		}
		
	}
	
	return nStatus;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CCardCfgBreeze::CCardCfgBreeze()
{
	m_detailedCardType = eMpmx_80;
}
/////////////////////////////////////////////////////////////////////////////
CCardCfgBreeze::CCardCfgBreeze(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm) : CCardCfgBarak(wBoard,szSerial,cardType,isRtm)
{
	
}
/////////////////////////////////////////////////////////////////////////////
CCardCfgBreeze::~CCardCfgBreeze()
{
	
}
/////////////////////////////////////////////////////////////////////////////
CCardCfgBreeze::CCardCfgBreeze(const CCardCfgBreeze& other) : CCardCfgBarak(other)
{
	
}
/////////////////////////////////////////////////////////////////////////////
CCardCfgBreeze& CCardCfgBreeze::operator= (const CCardCfgBreeze& other)
{
	if( this == &other )
		return *this;

	*((CCardCfgBarak*)this) = *((CCardCfgBarak*)&other);

	return *this;
}
/////////////////////////////////////////////////////////////////////////////
CCardCfg* CCardCfgBreeze::CreateCopy()
{
	CCardCfgBreeze* pMyCopy = new CCardCfgBreeze(*this);
	return (CCardCfg*)pMyCopy;
}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CCardCfgMpmRx::CCardCfgMpmRx()
{
    m_detailedCardType = eMpmRx_Full;
}
/////////////////////////////////////////////////////////////////////////////
CCardCfgMpmRx::CCardCfgMpmRx(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm) : CCardCfgBarak(wBoard,szSerial,cardType,isRtm)
{

}
/////////////////////////////////////////////////////////////////////////////
CCardCfgMpmRx::~CCardCfgMpmRx()
{

}
/////////////////////////////////////////////////////////////////////////////
CCardCfgMpmRx::CCardCfgMpmRx(const CCardCfgMpmRx& other) : CCardCfgBarak(other)
{

}
/////////////////////////////////////////////////////////////////////////////
CCardCfgMpmRx& CCardCfgMpmRx::operator= (const CCardCfgMpmRx& other)
{
    if( this == &other )
        return *this;

    *((CCardCfgBarak*)this) = *((CCardCfgBarak*)&other);

    return *this;
}
/////////////////////////////////////////////////////////////////////////////
CCardCfg* CCardCfgMpmRx::CreateCopy()
{
    CCardCfgMpmRx* pMyCopy = new CCardCfgMpmRx(*this);
    return (CCardCfg*)pMyCopy;
}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CCardGideonLite::CCardGideonLite()
{
}

/////////////////////////////////////////////////////////////////////////////
CCardGideonLite::CCardGideonLite(WORD wBoard,char* szSerial,eCardType cardType,BOOL isRtm)
		: CCardCfgMfa(wBoard,szSerial,cardType,isRtm)
{
}

/////////////////////////////////////////////////////////////////////////////
CCardGideonLite::~CCardGideonLite()
{
}

/////////////////////////////////////////////////////////////////////////////
CCardGideonLite::CCardGideonLite(const CCardGideonLite& rOther)
 	: CCardCfgMfa(rOther)
{
	*this = rOther;
}

/////////////////////////////////////////////////////////////////////////////
CCardGideonLite& CCardGideonLite::operator= (const CCardGideonLite& other)
{
	if( this == &other )
		return *this;

	*((CCardCfgMfa*)this) = *((CCardCfgMfa*)&other);

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
CCardCfg* CCardGideonLite::CreateCopy()
{
	CCardGideonLite* pMyCopy = new CCardGideonLite(*this);
	return (CCardCfg*)pMyCopy;
}

/////////////////////////////////////////////////////////////////////////////
void CCardGideonLite::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// common card's fields will serialize in base class
	CXMLDOMElement* pCardNode = pFatherNode->AddChildNode("CARD_GIDEON_LITE");
	CCardCfg::SerializeXml(pCardNode);
	// specific fields of MFA_&_RTM card
	pCardNode->AddChildNode("CARD_TYPE", m_detailedCardType);
	pCardNode->AddChildNode("RTM_ATTACHED", m_isRtmAttached,_BOOL);
}

/////////////////////////////////////////////////////////////////////////////
int	CCardGideonLite::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int	nStatus	= STATUS_OK;

	// get common card fields
	nStatus = CCardCfg::DeSerializeXml(pActionNode,pszError,action);
	if( nStatus == STATUS_OK ) {
		// get MFA_&_RTM card specific fields
		GET_VALIDATE_CHILD(pActionNode,"CARD_TYPE",(WORD*)&m_detailedCardType,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"RTM_ATTACHED",&m_isRtmAttached,_BOOL);
	}

	return nStatus;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CBehaviourSwitch::CBehaviourSwitch(const WORD timeout) : m_wConfigTime(timeout)
{
}

/////////////////////////////////////////////////////////////////////////////
CBehaviourSwitch::CBehaviourSwitch(const CBehaviourSwitch& rOther)
	: CSerializeObject(rOther)
{
	m_wConfigTime = rOther.m_wConfigTime;
}

/////////////////////////////////////////////////////////////////////////////
CBehaviourSwitch::~CBehaviourSwitch()
{
}

/////////////////////////////////////////////////////////////////////////////
void CBehaviourSwitch::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// create <SWITCH_BEHAVIOUR> section
	CXMLDOMElement* pSwitchNode = pFatherNode->AddChildNode("SWITCH_BEHAVIOUR");

	// <SWITCH_BEHAVIOUR> fields
	pSwitchNode->AddChildNode("CONFIG_TIME", m_wConfigTime);
}

/////////////////////////////////////////////////////////////////////////////
int	CBehaviourSwitch::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int		nStatus			= STATUS_OK;

	// get fields from <SWITCH_BEHAVIOUR> section
	if( pActionNode ) {
		GET_VALIDATE_CHILD(pActionNode,"CONFIG_TIME",&m_wConfigTime,_0_TO_WORD);
	}

	return nStatus;	
}

/////////////////////////////////////////////////////////////////////////////
void CBehaviourSwitch::Serialize(CSegment& rSegment) const
{
	rSegment << (DWORD)m_wConfigTime;
}

/////////////////////////////////////////////////////////////////////////////
void CBehaviourSwitch::DeSerialize(CSegment& rSegment)
{
	DWORD  temp = 0;
	rSegment >> temp;
	m_wConfigTime = (WORD)temp;
}

/////////////////////////////////////////////////////////////////////////////
int CBehaviourSwitch::IsEqual(const CBehaviourSwitch& rOther) const
{
	int res = (m_wConfigTime == rOther.m_wConfigTime) ? TRUE : FALSE;
	return res;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CBehaviourMfa::CBehaviourMfa( const WORD unitConfigTime,
					const WORD mediaConfigTime, const WORD speakerChangeTime)
		 : m_wUnitConfigReqTime(unitConfigTime),
		   m_wMediaConfigReqTime(mediaConfigTime),
		   m_wSpeakerChangeTime(speakerChangeTime)
{
}

/////////////////////////////////////////////////////////////////////////////
CBehaviourMfa::CBehaviourMfa(const CBehaviourMfa& rOther)
	: CSerializeObject(rOther)
{
	m_wUnitConfigReqTime  = rOther.m_wUnitConfigReqTime;
	m_wMediaConfigReqTime = rOther.m_wMediaConfigReqTime;
	m_wSpeakerChangeTime  = rOther.m_wSpeakerChangeTime;
}

/////////////////////////////////////////////////////////////////////////////
CBehaviourMfa::~CBehaviourMfa()
{
}

/////////////////////////////////////////////////////////////////////////////
void CBehaviourMfa::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// create <MFA_BEHAVIOUR> section
	CXMLDOMElement* pSwitchNode = pFatherNode->AddChildNode("MFA_BEHAVIOUR");

	// <MFA_BEHAVIOUR> fields
	pSwitchNode->AddChildNode("UNIT_CONFIG_TIME", m_wUnitConfigReqTime);
	pSwitchNode->AddChildNode("MEDIA_CONFIG_TIME", m_wMediaConfigReqTime);
	pSwitchNode->AddChildNode("SPEAKER_CHANGE_TIME", m_wSpeakerChangeTime);
}

/////////////////////////////////////////////////////////////////////////////
int	CBehaviourMfa::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int		nStatus			= STATUS_OK;

	// get fields from <MFA_BEHAVIOUR> section
	if( pActionNode ) {
		GET_VALIDATE_CHILD(pActionNode,"UNIT_CONFIG_TIME",&m_wUnitConfigReqTime,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"MEDIA_CONFIG_TIME",&m_wMediaConfigReqTime,_0_TO_WORD);
		GET_VALIDATE_CHILD(pActionNode,"SPEAKER_CHANGE_TIME",&m_wSpeakerChangeTime,_0_TO_WORD);
	}

	return nStatus;	
}

/////////////////////////////////////////////////////////////////////////////
void CBehaviourMfa::Serialize(CSegment& rSegment) const
{
	rSegment << (DWORD)m_wUnitConfigReqTime
			 << (DWORD)m_wMediaConfigReqTime
			 << (DWORD)m_wSpeakerChangeTime;
}

/////////////////////////////////////////////////////////////////////////////
void CBehaviourMfa::DeSerialize(CSegment& rSegment)
{
	DWORD  temp = 0;
	rSegment >> temp;
	m_wUnitConfigReqTime = (WORD)temp;
	rSegment >> temp;
	m_wMediaConfigReqTime = (WORD)temp;
	rSegment >> temp;
	m_wSpeakerChangeTime = (WORD)temp;
}

/////////////////////////////////////////////////////////////////////////////
int CBehaviourMfa::IsEqual(const CBehaviourMfa& rOther) const
{
	if( m_wUnitConfigReqTime != rOther.m_wUnitConfigReqTime )
		return FALSE;
	if( m_wMediaConfigReqTime != rOther.m_wMediaConfigReqTime )
		return FALSE;
	if( m_wSpeakerChangeTime != rOther.m_wSpeakerChangeTime )
		return FALSE;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CUnitCfg::CUnitCfg()
{
	m_wUnitId = 0;
	m_wUnitStatus = 0;
}

/////////////////////////////////////////////////////////////////////////////

CUnitCfg::CUnitCfg(WORD unitId, DWORD unitStatus)
{
	m_wUnitId = unitId;
	m_wUnitStatus = unitStatus;
}

/////////////////////////////////////////////////////////////////////////////

CUnitCfg::~CUnitCfg()
{
}

/////////////////////////////////////////////////////////////////////////////
CUnitCfg::CUnitCfg(const CUnitCfg& rOther) : CSerializeObject(rOther)
{
	*this = rOther;
}

/////////////////////////////////////////////////////////////////////////////
CUnitCfg& CUnitCfg::operator= (const CUnitCfg& other)
{
	if( this == &other )
		return *this;

	m_wUnitId = other.m_wUnitId;
	m_wUnitStatus = other.m_wUnitStatus;

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CUnitCfg::SerializeXml(CXMLDOMElement*& pUnitListNode) const
{
	// common card's fields will serialize in base class
	CXMLDOMElement* pUnitNode = pUnitListNode->AddChildNode("UNIT");

	// fields in unit details
	pUnitNode->AddChildNode("UNIT_ID", m_wUnitId);
	pUnitNode->AddChildNode("UNIT_STATUS", m_wUnitStatus);
}

/////////////////////////////////////////////////////////////////////////////
int	CUnitCfg::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int	nStatus	= STATUS_OK;

	// get common unit fields
	GET_VALIDATE_CHILD(pActionNode, "UNIT_ID", &m_wUnitId, _0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode, "UNIT_STATUS", &m_wUnitStatus, _0_TO_DWORD);
	
	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static int HexChar2Int(char hexChar)
{
    switch(hexChar)
    {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':    
            return 10;
        case 'b':
        case 'B':            
            return 11;
        case 'c':
        case 'C':    
            return 12;
        case 'd':
        case 'D':   
            return 13;
        case 'e':
        case 'E':    
            return 14;
        case 'f':
        case 'F':    
            return 15;
        default:
            return -1;
    }
}

/////////////////////////////////////////////////////////////////////////////
CBehaviourIsdn::CBehaviourIsdn() : m_num(0), m_index(-1),
								   m_caps(0), m_xmitMode(0), m_h230(0)
{
 	CSerializeObject::ReadXmlFile(ISDN_SIM_CONFIG_FILE_NAME);
}

/////////////////////////////////////////////////////////////////////////////
CBehaviourIsdn::CBehaviourIsdn(const CBehaviourIsdn& rOther)
  : CSerializeObject(rOther),
	m_num(rOther.m_num), m_index(rOther.m_index),
	m_caps(rOther.m_caps), m_xmitMode(rOther.m_xmitMode), m_h230(rOther.m_h230)
{
}

/////////////////////////////////////////////////////////////////////////////
CBehaviourIsdn::~CBehaviourIsdn()
{
}

/////////////////////////////////////////////////////////////////////////////
void CBehaviourIsdn::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	TRACESTR(eLevelInfoNormal) << " CBehaviourIsdn::SerializeXml";
}
/////////////////////////////////////////////////////////////////////////////
int	CBehaviourIsdn::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action)
{
	int nStatus = STATUS_OK, index = 0;
	char* pName = NULL;
	pActionNode->get_nodeName(&pName);

	if( !strcmp(pName,"ISDN_BEHAVIOUR")) {

	    // get fields from <ISDN_BEHAVIOUR> section of MPL_SIM.XML file
	    GET_VALIDATE_CHILD(pActionNode, "CAPS", &m_caps, _0_TO_BYTE);
		GET_VALIDATE_CHILD(pActionNode, "XMIT_MODE", &m_xmitMode, _0_TO_BYTE);
		GET_VALIDATE_CHILD(pActionNode, "H230", &m_h230, _0_TO_BYTE);

		TRACESTR(eLevelInfoNormal) << " CBehaviourIsdn::DeSerializeXml: caps = " << (DWORD)m_caps
							   << ", xmitMode = " << (DWORD)m_xmitMode << ", h230 = " << (DWORD)m_h230;
		return nStatus;
	}
	//get fields from ISDN_PARTY_CONFIG file
	CXMLDOMElement* pEpNode = NULL;
	nStatus = pActionNode->firstChildNode(&pEpNode);

	std::string strValue;

	while( pEpNode && nStatus == STATUS_OK ) {

		GET_VALIDATE_CHILD(pEpNode,"TIMER",&m_sgnList[index].timer,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pEpNode,"OPCODE",&m_sgnList[index].opcode,_0_TO_DWORD);
		GET_VALIDATE_CHILD(pEpNode,"DATA", strValue, FIFTY_LINE_BUFFER_LENGTH);

		int len = strValue.size(), j=0;


		for(int i=0; i<len-3; i++)
		  {
			if('0'==strValue[i] && 'x'==strValue[i+1])
			  {
				int bas_value = HexChar2Int(strValue[i+2])*16 + HexChar2Int(strValue[i+3]);
				m_sgnList[index].data[j] = bas_value;
				j++;
			  }
		  }
		m_sgnList[index].dataLen = j;

		nStatus = pActionNode->nextChildNode(&pEpNode);
		index++;
	}
	/* save the list size */
	m_num = index;
	/* initialize index */
	m_index = 0;

    TRACESTR(eLevelInfoNormal) << " CBehaviourIsdn::DeSerializeXml : size of signal list = " << m_num;

	return nStatus;	
}

const EP_SGN_S* CBehaviourIsdn::GetFirstSignal()
{
	m_index = 0;
	return (m_index >= m_num) ? NULL : (&m_sgnList[m_index]);
}

const EP_SGN_S* CBehaviourIsdn::GetCurrSignal()
{
    return (m_index < 0 || m_index >= m_num) ? NULL : (&m_sgnList[m_index]);
}

const EP_SGN_S* CBehaviourIsdn::GetNextSignal()
{
	m_index++;
    return (m_index < 0 || m_index >= m_num) ? NULL : (&m_sgnList[m_index]);
}


