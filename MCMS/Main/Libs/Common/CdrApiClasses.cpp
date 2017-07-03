//+========================================================================+
//                            NRESERV.CPP                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       NRESERV.CPP                                                 |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Anatoly                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#include "psosxml.h"
#include "CdrApiClasses.h"
#include "CDRDefines.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"


static int current_year = 1900;

//const char* GetStatusString(const int status);


////////////////////////////////////////////////////////////////////////////
// class CCardRsrsStruct

CCardRsrsStructBase::CCardRsrsStructBase()
{
    m_audio_board_id   = 0xFFFF;
    m_audio_unit_id    = 0xFFFF;
    m_video_board_id   = 0xFFFF;
    m_video_unit_id    = 0xFFFF;
    m_data_board_id    = 0xFFFF;
    m_data_unit_id     = 0xFFFF;
	
}
/////////////////////////////////////////////////////////////////////////////
CCardRsrsStructBase::CCardRsrsStructBase(const CCardRsrsStructBase &other)
{
	m_audio_board_id  = other.m_audio_board_id;
	m_audio_unit_id   = other.m_audio_unit_id;
	m_video_board_id  = other.m_video_board_id;
	m_video_unit_id   = other.m_video_unit_id;
	m_data_board_id   = other.m_data_board_id;
	m_data_unit_id    = other.m_data_unit_id;
	
}
/////////////////////////////////////////////////////////////////////////////
CCardRsrsStructBase::~CCardRsrsStructBase()
{
}

/////////////////////////////////////////////////////////////////////////////
bool CCardRsrsStructBase::operator != (const CCardRsrsStructBase &other)
{
	return !CCardRsrsStructBase::operator == (other);
}

/////////////////////////////////////////////////////////////////////////////
bool CCardRsrsStructBase::operator == (const CCardRsrsStructBase &other)
{
	if(m_audio_board_id != other.m_audio_board_id)
	{
		return false;
	}
	if(m_audio_unit_id != other.m_audio_unit_id)
	{
		return false;
	}
	if(m_video_board_id != other.m_video_board_id)
	{
		return false;
	}
	if(m_video_unit_id != other.m_video_unit_id)
	{
		return false;
	}
	if(m_data_unit_id != other.m_data_unit_id)
	{
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CCardRsrsStruct::Serialize(WORD format, std::ostream &m_ostr)
{
	
	m_ostr << m_audio_board_id << "\n";
	m_ostr << m_audio_unit_id << "\n";
	m_ostr << m_video_board_id << "\n";
	m_ostr <<    m_video_unit_id << "\n";
	m_ostr << m_data_board_id << "\n";
	m_ostr << m_data_unit_id << "\n";
	
}

/////////////////////////////////////////////////////////////////////////////
void CCardRsrsStruct::DeSerialize(WORD format, std::istream &m_istr)
{
	// assuming format = OPERATOR_MCMS
	
	m_istr >> m_audio_board_id;
	m_istr >> m_audio_unit_id;
	m_istr >> m_video_board_id;
	m_istr >> m_video_unit_id;
	m_istr >> m_data_board_id;
	m_istr >> m_data_unit_id;
	
}
/////////////////////////////////////////////////////////////////////////////
void CCardRsrsStruct::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pCardRsrsNode = pFatherNode->AddChildNode("RESOURCE_FORCE");

//	pCardRsrsNode->AddChildNode("AUDIO_BOARD",m_audio_board_id,BOARD_ENUM);
//	pCardRsrsNode->AddChildNode("VIDEO_BOARD",m_video_board_id,BOARD_ENUM);
//	pCardRsrsNode->AddChildNode("DATA_BOARD",m_data_board_id,BOARD_ENUM);
//	pCardRsrsNode->AddChildNode("DATA_UNIT",m_data_unit_id,UNIT_ENUM);
//	pCardRsrsNode->AddChildNode("AUDIO_UNIT",m_audio_unit_id,UNIT_ENUM);
//	pCardRsrsNode->AddChildNode("VIDEO_UNIT",m_video_unit_id,UNIT_ENUM);
}
/////////////////////////////////////////////////////////////////////////////
int	 CCardRsrsStruct::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

//	GET_VALIDATE_CHILD(pActionNode,"AUDIO_BOARD",&m_audio_board_id,BOARD_ENUM);
//	GET_VALIDATE_CHILD(pActionNode,"VIDEO_BOARD",&m_video_board_id,BOARD_ENUM);
//	GET_VALIDATE_CHILD(pActionNode,"DATA_BOARD",&m_data_board_id,BOARD_ENUM);
//	GET_VALIDATE_CHILD(pActionNode,"DATA_UNIT",&m_data_unit_id,UNIT_ENUM);
//	GET_VALIDATE_CHILD(pActionNode,"AUDIO_UNIT",&m_audio_unit_id,UNIT_ENUM);
//	GET_VALIDATE_CHILD(pActionNode,"VIDEO_UNIT",&m_video_unit_id,UNIT_ENUM);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
void CCardRsrsStruct::CdrSerialize(WORD format, std::ostream &m_ostr)
{
	
	m_ostr << m_audio_board_id << ",";
	m_ostr << m_audio_unit_id << ",";
	m_ostr << m_video_board_id << ",";
	m_ostr <<    m_video_unit_id << ",";
	m_ostr << m_data_board_id << ",";
	m_ostr << m_data_unit_id << ",";
	
}
/////////////////////////////////////////////////////////////////////////////
void CCardRsrsStruct::CdrSerialize(WORD format, std::ostream &m_ostr, BYTE bilflag)
{
	if (m_audio_board_id!=0xffff)
	{
		m_ostr << "audio board id:"<<m_audio_board_id   << "\n";
		m_ostr << "audio unit  id:" <<m_audio_unit_id    << "\n";
	}
	else
		m_ostr << "no audio resource force."<< "\n";
	
	if (m_video_board_id!=0xffff)
	{
		m_ostr << "video board id:"<<m_video_board_id   << "\n";
		m_ostr << "video unit  id:" <<m_video_unit_id    << "\n";
	}
	else
		m_ostr << "no video resource force."<< "\n";
	
	if (m_video_board_id!=0xffff)
	{
		m_ostr << "data  board id:" <<m_data_board_id    << "\n";
		m_ostr << "data  unit  id:"  <<m_data_unit_id     << "\n";
	}
	else
		m_ostr << "no  data resource force."<< "\n";
	
	
}

/////////////////////////////////////////////////////////////////////////////
void CCardRsrsStruct::CdrDeSerialize(WORD format, std::istream &m_istr)
{
	// assuming format = OPERATOR_MCMS
//	m_istr.ignore(1);
	m_istr >> m_audio_board_id;
	m_istr.ignore(1);
	m_istr >> m_audio_unit_id;
	m_istr.ignore(1);
	m_istr >> m_video_board_id;
	m_istr.ignore(1);
	m_istr >> m_video_unit_id;
	m_istr.ignore(1);
	m_istr >> m_data_board_id;
	m_istr.ignore(1);
	m_istr >> m_data_unit_id;
	m_istr.ignore(1);	
}


/////////////////////////////////////////////////////////////////////////////
const char*  CCardRsrsStruct::NameOf() const
{
	return "CCardRsrsStruct";
}

////////////////////////////////////////////////////////////////////////////
void   CCardRsrsStruct::SetAudioBoardId(const WORD audio_board_id)
{
	m_audio_board_id=audio_board_id;
}

////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrsStructBase::GetAudioBoardId() const
{
    return m_audio_board_id;
}

////////////////////////////////////////////////////////////////////////////

void CCardRsrsStruct::SetAudioUnitId(const WORD audio_unit_id)
{
	m_audio_unit_id=audio_unit_id;
}

//////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrsStructBase::GetAudioUnitId() const
{
    return m_audio_unit_id;
}

///////////////////////////////////////////////////////////////////////////////
void   CCardRsrsStruct::SetVideoBoardId(const WORD video_board_id)
{
	m_video_board_id=video_board_id;
}

///////////////////////////////////////////////////////////////////////////////
WORD CCardRsrsStructBase::GetVideoBoardId() const
{
	return m_video_board_id;
}

///////////////////////////////////////////////////////////////////////////////
void   CCardRsrsStruct::SetVideoUnitId(const WORD video_unit_id)
{
    m_video_unit_id=video_unit_id;
}

////////////////////////////////////////////////////////////////////////////////
WORD CCardRsrsStructBase::GetVideoUnitId() const
{
	return  m_video_unit_id;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardRsrsStruct::SetDataBoardId(const WORD data_board_id)
{
    m_data_board_id=data_board_id;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrsStructBase::GetDataBoardId() const
{
    return m_data_board_id;
}

/////////////////////////////////////////////////////////////////////////////
void  CCardRsrsStruct::SetDataUnitId(const WORD data_unit_id)
{
    m_data_unit_id=data_unit_id;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCardRsrsStructBase::GetDataUnitId() const
{
    return m_data_unit_id;
}


























/////////  class CServicePhoneStr


CServicePhoneStr::CServicePhoneStr()
{
	m_netServiceName[0]='\0';
	
	for (int i=0;i<MAX_PHONE_NUMBERS_IN_CONFERENCE;i++) {
		m_pPhoneNumberList[i] = NULL;
	}
	m_numPhones = 0;
	m_ind_phone = 0;

	m_useServicePhonesAsRange = NO;
	m_phonePrefixForward[0] = '\0';
	m_phoneNumDigitsForward = 0;
}

/////////////////////////////////////////////////////////////////////////////
CServicePhoneStr::CServicePhoneStr(const CServicePhoneStr &other)
:CPObject(other)
{
	strncpy(m_netServiceName,other.m_netServiceName,NET_SERVICE_PROVIDER_NAME_LEN);
	m_numPhones=other.m_numPhones;
	m_useServicePhonesAsRange = other.m_useServicePhonesAsRange;
	m_phoneNumDigitsForward = other.m_phoneNumDigitsForward;
	strncpy(m_phonePrefixForward, other.m_phonePrefixForward, SERVICE_PHONE_PREFIX_LEN);//m_phonePrefixForward = other.m_phonePrefixForward;
	m_phonePrefixForward[SERVICE_PHONE_PREFIX_LEN-1] = '\0';

	for (int i=0;i<MAX_PHONE_NUMBERS_IN_CONFERENCE;i++)
	{
		if( other.m_pPhoneNumberList[i]==NULL)
			m_pPhoneNumberList[i]=NULL;
		else
		{
			m_pPhoneNumberList[i] = new Phone;//(*other.m_pPhoneNumberList[i])
			const char* phoneNumber = other.m_pPhoneNumberList[i]->phone_number;

			strncpy( m_pPhoneNumberList[i]->phone_number, phoneNumber, sizeof(m_pPhoneNumberList[i]->phone_number) - 1);
			m_pPhoneNumberList[i]->phone_number[sizeof(m_pPhoneNumberList[i]->phone_number) - 1]='\0';
		}
	}
	
	m_ind_phone=other.m_ind_phone;
}

/////////////////////////////////////////////////////////////////////////////
CServicePhoneStr&  CServicePhoneStr::operator = (const CServicePhoneStr& other)
{
	strncpy(m_netServiceName,other.m_netServiceName, NET_SERVICE_PROVIDER_NAME_LEN);
	m_numPhones=other.m_numPhones;
	m_useServicePhonesAsRange = other.m_useServicePhonesAsRange;
	m_phoneNumDigitsForward = other.m_phoneNumDigitsForward;
	strncpy(m_phonePrefixForward, other.m_phonePrefixForward, SERVICE_PHONE_PREFIX_LEN);//m_phonePrefixForward = other.m_phonePrefixForward;
	m_phonePrefixForward[SERVICE_PHONE_PREFIX_LEN-1] = '\0';
	
	for (int i=0;i<MAX_PHONE_NUMBERS_IN_CONFERENCE;i++)
	{
		PDELETE(m_pPhoneNumberList[i]);
		if( other.m_pPhoneNumberList[i]==NULL)
			m_pPhoneNumberList[i]=NULL;
		else
		{
			m_pPhoneNumberList[i]= new Phone;//(*other.m_pPhoneNumberList[i]);
			const char* phoneNumber = other.m_pPhoneNumberList[i]->phone_number;
			strncpy( m_pPhoneNumberList[i]->phone_number, phoneNumber, sizeof(m_pPhoneNumberList[i]->phone_number) - 1);
			m_pPhoneNumberList[i]->phone_number[sizeof(m_pPhoneNumberList[i]->phone_number) - 1]='\0';
		}
	}
	
	m_ind_phone=other.m_ind_phone;
	
	return *this;
}

///////////////////////////////////////////////////////////////////////////
bool CServicePhoneStr::operator == (const CServicePhoneStr& other)
{
	if(m_numPhones != other.m_numPhones)
	{
		return false;
	}
	if(0 != strcmp(m_netServiceName, other.m_netServiceName))
	{
		return false;
	}
	if(m_ind_phone != other.m_ind_phone)
	{
		return false;
	}
	for(int i = 0 ; i < MAX_PHONE_NUMBERS_IN_CONFERENCE ; i++)
	{
		if(m_pPhoneNumberList[i] == NULL && other.m_pPhoneNumberList[i] == NULL)
		{
			continue;
		}
		if(	(m_pPhoneNumberList[i] == NULL && other.m_pPhoneNumberList[i] != NULL) ||
			(m_pPhoneNumberList[i] != NULL && other.m_pPhoneNumberList[i] == NULL))
		{
			return false;
		}
		
		if(0 != strcmp(m_pPhoneNumberList[i]->phone_number, other.m_pPhoneNumberList[i]->phone_number))
		{
			return false;
		}
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////
BYTE CServicePhoneStr::operator >= (const CServicePhoneStr& other)
{
	BYTE bRes = TRUE;
	
	if( bRes == TRUE && ( m_numPhones < other.m_numPhones))
		bRes = FALSE;
	
	if( bRes == TRUE)
	{
		if(0 != strncmp(m_netServiceName,other.m_netServiceName,NET_SERVICE_PROVIDER_NAME_LEN))
			bRes = FALSE;
	}
	int iSearchRes = 0;
	if( bRes == TRUE){
		for(BYTE i=0;i<m_numPhones;i++){
			if( other.m_pPhoneNumberList[i] != NULL){
				
				iSearchRes = FindPhoneNumber((other.m_pPhoneNumberList[i])->phone_number);	
				// can be buggy : one service "included" in other
				
				
				if(iSearchRes == NOT_FIND)
				{
					bRes = FALSE;
					break;
				}
			}
			
		}
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CServicePhoneStr::operator < (const CServicePhoneStr& other){
	BYTE bRes = TRUE;

	if( bRes == TRUE && ( m_numPhones >= other.m_numPhones))
		bRes = FALSE;

	if( bRes == TRUE){
		if(0 != strncmp(m_netServiceName,other.m_netServiceName,NET_SERVICE_PROVIDER_NAME_LEN))
			bRes = FALSE;
	}
	int iSearchRes = 0;
	if( bRes == TRUE){
		for(BYTE i=0;i<m_numPhones;i++){
			if( m_pPhoneNumberList[i] != NULL){

				iSearchRes = other.FindPhoneNumber((m_pPhoneNumberList[i])->phone_number);	// can be buggy : one service "included" in other
				if(iSearchRes == NOT_FIND){
					bRes = FALSE;
					break;
				}
			}
			
		}
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
CServicePhoneStr::~CServicePhoneStr()
{
	for (int i=0;i<MAX_PHONE_NUMBERS_IN_CONFERENCE;i++)
		PDELETE(m_pPhoneNumberList[i]);
	
}

/////////////////////////////////////////////////////////////////////////////
void  CServicePhoneStr::Serialize(WORD format, std::ostream  &m_ostr)
{
    m_ostr << m_netServiceName << "\n";
    m_ostr << m_numPhones  << "\n";
    for (int i=0;i<(int)m_numPhones;i++)
		m_ostr << m_pPhoneNumberList[i]->phone_number << "\n";

    m_ostr << (WORD)m_useServicePhonesAsRange << "\n";
    m_ostr << m_phoneNumDigitsForward << "\n";
    m_ostr << m_phonePrefixForward << "\n";
}
/////////////////////////////////////////////////////////////////////////////
void  CServicePhoneStr::CdrSerialize(WORD format, std::ostream  &m_ostr,DWORD apiNum)
{
    m_ostr <<"net service name:"<< m_netServiceName << "\n";
    m_ostr <<"service phone number:";
    for (int i=0;i<(int)m_numPhones;i++)
		m_ostr <<m_pPhoneNumberList[i]->phone_number  << ",";
    m_ostr<< "\n";
}

/////////////////////////////////////////////////////////////////////////////
void  CServicePhoneStr::DeSerialize(WORD format, std::istream &m_istr)
{
	m_istr.getline(m_netServiceName,NET_SERVICE_PROVIDER_NAME_LEN+1,'\n');
	m_istr >> m_numPhones;
	m_istr.ignore(1);
	for (int i=0;i<(int)m_numPhones;i++) {
		m_pPhoneNumberList[i] = new Phone;
		m_istr.getline(m_pPhoneNumberList[i]->phone_number,PHONE_NUMBER_DIGITS_LEN+1,'\n');
	}
	WORD tmp;
	m_istr >> tmp;
	m_useServicePhonesAsRange = (BYTE)tmp;
	m_istr.ignore(1);
	m_istr >> m_phoneNumDigitsForward;
	m_istr.ignore(1);
	m_istr.getline(m_phonePrefixForward, SERVICE_PHONE_PREFIX_LEN+1, '\n');
}
/////////////////////////////////////////////////////////////////////////////
void  CServicePhoneStr::CdrSerialize(WORD format, std::ostream  &m_ostr)
{
    m_ostr << m_netServiceName << ",";
    m_ostr << m_numPhones  << ",";
    for (int i=0;i<(int)m_numPhones;i++)
		m_ostr << m_pPhoneNumberList[i]->phone_number  << ",";
}
/////////////////////////////////////////////////////////////////////////////
void  CServicePhoneStr::CdrDeSerialize(WORD format, std::istream &m_istr)
{
	// m_istr.ignore(1);
	m_istr.getline(m_netServiceName,NET_SERVICE_PROVIDER_NAME_LEN+1,',');
	m_istr >> m_numPhones;
	m_istr.ignore(1);
	for (int i=0;i<(int)m_numPhones;i++) {
		m_pPhoneNumberList[i] = new Phone;
		m_istr.getline(m_pPhoneNumberList[i]->phone_number,PHONE_NUMBER_DIGITS_LEN+1,',');
	}
}
/////////////////////////////////////////////////////////////////////////////
void CServicePhoneStr::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pServicePhoneNode = pFatherNode->AddChildNode("SERVICE");

	pServicePhoneNode->AddChildNode("NAME",m_netServiceName);
	pServicePhoneNode->AddChildNode("PHONE1",m_pPhoneNumberList[0]->phone_number);
	pServicePhoneNode->AddChildNode("PHONE2",m_pPhoneNumberList[1]->phone_number);

	CXMLDOMElement* pPhoneListNode = pServicePhoneNode->AddChildNode("PHONE_LIST_EX");

	for (int i=2;i<(int)m_numPhones;i++)
		pPhoneListNode->AddChildNode("PHONE", m_pPhoneNumberList[i]->phone_number);

	pServicePhoneNode->AddChildNode("USE_SERVICE_PHONES_AS_RANGE", m_useServicePhonesAsRange, _BOOL);
	pServicePhoneNode->AddChildNode("SERVICE_PHONES_PREFIX_FORWARD", m_phonePrefixForward);
	pServicePhoneNode->AddChildNode("SERVICE_PHONES_NUM_DIGITS_FORWARD", m_phoneNumDigitsForward);
}
/////////////////////////////////////////////////////////////////////////////
int	 CServicePhoneStr::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pChildNode, *pTempNode;
	char *szVal = NULL;

	GET_VALIDATE_CHILD(pActionNode,"NAME",m_netServiceName,NET_SERVICE_PROVIDER_NAME_LENGTH);

	m_numPhones = 0;
	GET_VALIDATE_CHILD(pActionNode,"PHONE1",&szVal,PHONE_NUMBER_DIGITS_LENGTH);
	if(nStatus == STATUS_OK && szVal && strlen(szVal)>0)
		AddPhoneNumber(szVal);
	GET_VALIDATE_CHILD(pActionNode,"PHONE2",&szVal,PHONE_NUMBER_DIGITS_LENGTH);
	if(nStatus == STATUS_OK && szVal && strlen(szVal)>0)
		AddPhoneNumber(szVal);

	GET_CHILD_NODE(pActionNode, "PHONE_LIST_EX", pChildNode);

	if (pChildNode)
	{
		GET_FIRST_CHILD_NODE(pChildNode, "PHONE", pTempNode);
		while (pTempNode)
		{
			GET_VALIDATE(pTempNode,&szVal,PHONE_NUMBER_DIGITS_LENGTH);
			if(nStatus == STATUS_OK)
				AddPhoneNumber(szVal);
			GET_NEXT_CHILD_NODE(pChildNode, "PHONE", pTempNode);
		}
	}

	GET_VALIDATE_CHILD(pActionNode,"USE_SERVICE_PHONES_AS_RANGE",&m_useServicePhonesAsRange,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"SERVICE_PHONES_NUM_DIGITS_FORWARD",&m_phoneNumDigitsForward,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"SERVICE_PHONES_PREFIX_FORWARD", &szVal, SERVICE_PHONE_PREFIX_LENGTH);
	if( nStatus == STATUS_OK && szVal && strlen(szVal)>0)
		SetPhonePrefixForward(szVal);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
int  CServicePhoneStr::AddPhoneNumber(const char*  phoneNumber)
{
	if(phoneNumber==NULL)
		return STATUS_ILLEGAL;

	if (m_numPhones>=MAX_PHONE_NUMBERS_IN_CONFERENCE)
		return  STATUS_MAX_PHONE_NUMBER_EXCEEDED;
	
	m_pPhoneNumberList[m_numPhones] = new Phone;
	

	strncpy(m_pPhoneNumberList[m_numPhones]->phone_number, phoneNumber, sizeof(m_pPhoneNumberList[m_numPhones]->phone_number) - 1);
	m_pPhoneNumberList[m_numPhones]->phone_number[sizeof(m_pPhoneNumberList[m_numPhones]->phone_number)-1]='\0';
	
	m_numPhones++;
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int  CServicePhoneStr::CancelPhoneNumber(const char*  phoneNumber)
{
	int ind;
	ind=FindPhoneNumber(phoneNumber);
	if (ind==NOT_FIND) return STATUS_PHONE_NUMBER_NOT_EXISTS;
	
	if (ind>0 && ind<MAX_PHONE_NUMBERS_IN_CONFERENCE)
	   PDELETE(m_pPhoneNumberList[ind]);
	
	int i;
	if (m_numPhones>=MAX_PHONE_NUMBERS_IN_CONFERENCE)
		return  STATUS_MAX_PHONE_NUMBER_EXCEEDED;
	
	for (i=0;i<(int)m_numPhones;i++)
	{
		if (m_pPhoneNumberList[i]==NULL)
			break;
	}
	for (int j=i;j<(int)m_numPhones-1;j++)
	{
		m_pPhoneNumberList[j]=m_pPhoneNumberList[j+1] ;
	}
	m_pPhoneNumberList[m_numPhones-1] = NULL;
	m_numPhones--;
	
	return  STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int  CServicePhoneStr::FindPhoneNumber(const char*  phoneNumber)const
{
	for (int i=0;i<(int)m_numPhones;i++)
	{
		if (m_pPhoneNumberList[i]!=NULL) {
			if (! strncmp(m_pPhoneNumberList[i]->phone_number, phoneNumber,PHONE_NUMBER_DIGITS_LEN))
				return i;
		}
	}
	return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
WORD CServicePhoneStr::GetNumPhoneNumbers()
{
    return m_numPhones;
}

/////////////////////////////////////////////////////////////////////////////
Phone* CServicePhoneStr::GetFirstPhoneNumber()
{
	m_ind_phone = 1;
	return m_pPhoneNumberList[0];
}

/////////////////////////////////////////////////////////////////////////////
Phone* CServicePhoneStr::GetFirstPhoneNumber(int& nPos)
{
	Phone*  pPhone = CServicePhoneStr::GetFirstPhoneNumber();
	nPos = m_ind_phone;
	
	return pPhone;
}

/////////////////////////////////////////////////////////////////////////////
Phone* CServicePhoneStr::GetNextPhoneNumber()
{
	if (m_ind_phone>=m_numPhones) return NULL;
	return m_pPhoneNumberList[m_ind_phone++];
}

/////////////////////////////////////////////////////////////////////////////
Phone* CServicePhoneStr::GetNextPhoneNumber(int ind,int& nPos)
{
	m_ind_phone = ind;
	Phone*  pPhone = CServicePhoneStr::GetNextPhoneNumber();
	nPos = m_ind_phone;
	
	return pPhone;
}

/////////////////////////////////////////////////////////////////////////////
void CServicePhoneStr::SetNetServiceName(const char*  name)
{
	strncpy(m_netServiceName, name, sizeof(m_netServiceName) - 1);
	m_netServiceName[sizeof(m_netServiceName) - 1]='\0';
}

/////////////////////////////////////////////////////////////////////////////
const char*  CServicePhoneStr::GetNetServiceName () const
{
    return m_netServiceName;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CServicePhoneStr::NameOf() const
{
	return "CServicePhoneStr";
}
//////////////////////////////////////////////////////////////////////////////
BYTE CServicePhoneStr::IsUseServicePhonesAsRange() const
{
	return m_useServicePhonesAsRange;
}
//////////////////////////////////////////////////////////////////////////////
void CServicePhoneStr::SetUseServicePhonesAsRange(BYTE useServicePhonesAsRange)
{
	m_useServicePhonesAsRange = useServicePhonesAsRange;
}
/////////////////////////////////////////////////////////////////////////////
const char* CServicePhoneStr::GetPhonePrefixForward() const
{
	return m_phonePrefixForward;
}
/////////////////////////////////////////////////////////////////////////////
void CServicePhoneStr::SetPhonePrefixForward(const char* phonePrefixForward)
{
	if( phonePrefixForward )
	{

		strncpy(m_phonePrefixForward, phonePrefixForward, sizeof(m_phonePrefixForward) - 1);
		m_phonePrefixForward[sizeof(m_phonePrefixForward) - 1]='\0';
	}
}
/////////////////////////////////////////////////////////////////////////////
WORD CServicePhoneStr::GetPhoneNumDigitsForward() const
{
	return m_phoneNumDigitsForward;
}
/////////////////////////////////////////////////////////////////////////////
void CServicePhoneStr::SetPhoneNumDigitsForward(WORD phoneNumDigitsForward)
{
	m_phoneNumDigitsForward = phoneNumDigitsForward;
}






















/////////  class CLectureMode

ACCLectureMode::ACCLectureMode()
{
	// From version API_NUM_LECTURE_SHOW (150) m_LectureModeOnOff may be 0/1/2
    m_LectureModeOnOff = NO;
    m_LecturerName[0]='\0';
    m_TimeInterval = 0xFFFF;
	
    m_timerOnOff = NO;
    m_audioActivated = NO;
    m_lecturerId = 0xFFFFFFFF;
}

/////////////////////////////////////////////////////////////////////////////
ACCLectureMode::ACCLectureMode(const ACCLectureMode &other)
{
    *this=other;
}

/////////////////////////////////////////////////////////////////////////////
ACCLectureMode::~ACCLectureMode()
{
	
}
////////////////////////////////////////////////////////////////////////////
ACCLectureMode&  ACCLectureMode::operator = (const CLectureModeParams& other)
{
	m_LectureModeOnOff=other.m_LectureModeOnOff;
	strncpy(m_LecturerName,other.m_LecturerName,H243_NAME_LEN);
	m_TimeInterval= other.m_TimeInterval;
	
	m_timerOnOff = other.m_timerOnOff;
	m_audioActivated = other.m_audioActivated;
	m_lecturerId = other.m_lecturerId;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool ACCLectureMode::operator != (const ACCLectureMode& other)
{
	return !ACCLectureMode::operator ==(other);
}

/////////////////////////////////////////////////////////////////////////////
bool ACCLectureMode::operator == (const ACCLectureMode& other)
{
	if(m_LectureModeOnOff != other.m_LectureModeOnOff)
	{
		return false;
	}
	if(m_TimeInterval != other.m_TimeInterval)
	{
		return false;
	}
	if(m_timerOnOff != other.m_timerOnOff)
	{
		return false;
	}
	if(m_audioActivated != other.m_audioActivated)
	{
		return false;
	}
	if(m_lecturerId != other.m_lecturerId)
	{
		return false;
	}
	if(0 != strcmp(m_LecturerName, other.m_LecturerName))
	{
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void  CLectureMode::Serialize(WORD format, std::ostream  &m_ostr, DWORD apiNum)
{
// From version API_NUM_LECTURE_SHOW (145) m_LectureModeOnOff may be 0/1/2
// From version API_NUM_PRESENTATION_MODE (521) m_LectureModeOnOff may be 0/1/2/3

    if( apiNum>=API_NUM_PRESENTATION_MODE || format != OPERATOR_MCMS ) 
	{
        m_ostr << (WORD)m_LectureModeOnOff << "\n";
    } 
	else if(apiNum>=API_NUM_LECTURE_SHOW) 
	{
		if(m_LectureModeOnOff==3)
			m_ostr << (WORD)0 << "\n";//Presentation mode not supported by api
		else
			m_ostr << (WORD)m_LectureModeOnOff << "\n";
	}
	else
	{
        if( m_LectureModeOnOff != NO )
            m_ostr << (WORD)1 << "\n";
        else
            m_ostr << (WORD)0 << "\n";
    }
    if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
        m_ostr << m_LecturerName << "\n";
    else{
        char tmp[H243_NAME_LEN_OLD];
        strncpy(tmp,m_LecturerName,H243_NAME_LEN_OLD-1);
        tmp[H243_NAME_LEN_OLD-1]='\0';
		
        m_ostr << tmp  << "\n";
    }
    m_ostr << m_TimeInterval  << "\n";
	
    if(apiNum>=33 || format != OPERATOR_MCMS){
		m_ostr << (WORD) m_timerOnOff  << "\n";
		m_ostr << (WORD) m_audioActivated  << "\n";
		m_ostr << m_lecturerId  << "\n";
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CLectureMode::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
    WORD tmp;
	
    m_istr >> tmp;
    m_LectureModeOnOff=(BYTE)tmp;
    m_istr.ignore(1);
    if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
        m_istr.getline(m_LecturerName,H243_NAME_LEN+1,'\n');
    else
        m_istr.getline(m_LecturerName,H243_NAME_LEN_OLD+1,'\n');
    m_istr >>  m_TimeInterval ;
	
    if(apiNum>=33 || format != OPERATOR_MCMS){
        m_istr >> tmp;
        m_timerOnOff = (BYTE)tmp;
        m_istr >> tmp;
        m_audioActivated = (BYTE)tmp;
        m_istr >>  m_lecturerId ;
    }
}
/////////////////////////////////////////////////////////////////////////////
int CLectureMode::DeSerializeXml(CXMLDOMElement *pLectureModeNode,char *pszError)
{
	int nStatus;

	GET_VALIDATE_CHILD(pLectureModeNode,"ON",&m_LectureModeOnOff,_BOOL);
	GET_VALIDATE_CHILD(pLectureModeNode,"TIMER",&m_timerOnOff,_BOOL);
	GET_VALIDATE_CHILD(pLectureModeNode,"INTERVAL",&m_TimeInterval,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pLectureModeNode,"AUDIO_ACTIVATED",&m_audioActivated,_BOOL);
	GET_VALIDATE_CHILD(pLectureModeNode,"LECTURE_NAME",m_LecturerName,_0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pLectureModeNode,"LECTURE_MODE_TYPE",&m_LectureModeOnOff,LECTURE_MODE_TYPE_ENUM);
	GET_VALIDATE_CHILD(pLectureModeNode,"LECTURE_ID",&m_lecturerId,_0_TO_DWORD);
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
void CLectureMode::SerializeXml(CXMLDOMElement *pFatherNode)
{
	CXMLDOMElement *pTempNode;

	pTempNode = pFatherNode->AddChildNode("LECTURE_MODE");
	pTempNode->AddChildNode("ON",m_LectureModeOnOff,_BOOL);
	pTempNode->AddChildNode("TIMER",m_timerOnOff,_BOOL);
	pTempNode->AddChildNode("INTERVAL",m_TimeInterval);
	pTempNode->AddChildNode("AUDIO_ACTIVATED",m_audioActivated,_BOOL);
	pTempNode->AddChildNode("LECTURE_NAME",m_LecturerName);
	pTempNode->AddChildNode("LECTURE_MODE_TYPE",m_LectureModeOnOff,LECTURE_MODE_TYPE_ENUM);
	pTempNode->AddChildNode("LECTURE_ID",m_lecturerId);
}
/////////////////////////////////////////////////////////////////////////////
void  CLectureMode::CdrSerialize(WORD format, std::ostream  &m_ostr, DWORD apiNum)
{
// From version API_NUM_LECTURE_SHOW (145) m_LectureModeOnOff may be 0/1/2
// From version API_PRESENTATION_MODE (521) m_LectureModeOnOff may be 0/1/2/3

    if( apiNum>=API_NUM_PRESENTATION_MODE || format != OPERATOR_MCMS ) 
	{
        m_ostr << (WORD)m_LectureModeOnOff << ",";
    } 
	else if(apiNum>=API_NUM_LECTURE_SHOW)
	{
		if( m_LectureModeOnOff == 3)
			m_ostr << (WORD)0 << ","; //API not support Presentation mode 
		else
			m_ostr << (WORD)m_LectureModeOnOff << ",";
	}
	else
	{
        if( m_LectureModeOnOff != NO )
            m_ostr << (WORD)1 << ",";
        else
            m_ostr << (WORD)0 << ",";
    }
    if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
        m_ostr << m_LecturerName << ",";
    else{
        char tmp[H243_NAME_LEN_OLD];
        strncpy(tmp,m_LecturerName,H243_NAME_LEN_OLD-1);
        tmp[H243_NAME_LEN_OLD-1]='\0';
		
        m_ostr << tmp  << ",";
    }
    m_ostr << m_TimeInterval  << ",";
    m_ostr << (WORD) m_timerOnOff  << ",";
    m_ostr << (WORD) m_audioActivated  << ",";
    m_ostr << m_lecturerId  << ";\n";
	
}

/////////////////////////////////////////////////////////////////////////////
void  CLectureMode::CdrSerialize(WORD format, std::ostream  &m_ostr, DWORD apiNum, BYTE bilflag)
{
// From version API_NUM_LECTURE_SHOW (145) m_LectureModeOnOff may be 0/1/2
    switch ((WORD)m_LectureModeOnOff) {
        case 0 :{
            m_ostr << "lecture mode: NONE"<< "\n";
            break;
        }
        case 1:{
            m_ostr << "lecture mode: LECTURE MODE" << "\n";
            break;
        }
        case 2:{
            if( apiNum >= API_NUM_LECTURE_SHOW || format != OPERATOR_MCMS )
                m_ostr << "lecture mode: LECTURE SHOW" << "\n";
            else
                m_ostr<<" --"<<"\n";
            break;
        }
		case 3:{
            if( apiNum >= API_NUM_PRESENTATION_MODE || format != OPERATOR_MCMS )
                m_ostr << "lecture mode: PRESENTATION MODE" << "\n";
            else
                m_ostr<<" --"<<"\n";
            break;
        }

        default :{
            m_ostr<<" --"<<"\n";
            break;
        }
    }
	
	
    if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
        m_ostr << "lecturer name:"  <<m_LecturerName  << "\n";
    else{
        char tmp[H243_NAME_LEN_OLD];
        strncpy(tmp,m_LecturerName,H243_NAME_LEN_OLD-1);
        tmp[H243_NAME_LEN_OLD-1]='\0';
		
        m_ostr << "lecturer name:" << tmp  << "\n";
    }
    m_ostr << "time interval:" <<  m_TimeInterval  << "\n";
	
    switch ((WORD)m_timerOnOff)
	{
	case 0 :
		{
			m_ostr << "timer mode:OFF"<< "\n";
			break;
		}
	case 1:
		{
			m_ostr << "timer mode:ON" << "\n";
			break;
		}
	default :{
		m_ostr<<" --"<<"\n";
		break;
			 }
	}
    switch ((WORD)m_audioActivated) 
	{
	case 0 :
		{
			m_ostr << "audio activated:OFF"<< "\n";
			break;
		}
	case 1:
		{
			m_ostr << "audio activated:ON" << "\n";
			break;
		}
	default :
		{
			m_ostr<<" --"<<"\n";
			break;
		}
	}
	
    if(m_lecturerId!=0xFFFFFFFF)
		m_ostr << "lecturer Id:"  <<m_lecturerId << ";\n\n"; //temp to delete in all the ser...
    else
		m_ostr << ";\n\n";
	
}

/////////

/////////////////////////////////////////////////////////////////////////////
void  CLectureMode::CdrDeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
    WORD tmp;
	// m_istr.ignore(1);
    m_istr >> tmp;
    m_LectureModeOnOff=(BYTE)tmp;
    m_istr.ignore(1);
    if(apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
        m_istr.getline(m_LecturerName,H243_NAME_LEN+1,',');
    else
        m_istr.getline(m_LecturerName,H243_NAME_LEN_OLD+1,',');
    //m_istr.ignore(1);
    m_istr >>  m_TimeInterval ;
    m_istr.ignore(1);
    m_istr >> tmp;
    m_timerOnOff = (BYTE)tmp;
    m_istr.ignore(1);
    m_istr >> tmp;
    m_audioActivated = (BYTE)tmp;
    m_istr.ignore(1);
    m_istr >>  m_lecturerId ;
    //m_istr.ignore(1);
}


/////////////////////////////////////////////////////////////////////////////
const char*  CLectureMode::NameOf() const
{
	return "CLectureMode";
}

/////////////////////////////////////////////////////////////////////////////
ACCLectureMode&  ACCLectureMode::operator = (const ACCLectureMode& other)
{
	m_LectureModeOnOff=other.m_LectureModeOnOff;
	strncpy(m_LecturerName,other.m_LecturerName,H243_NAME_LEN);
	m_TimeInterval= other.m_TimeInterval;
	
	m_timerOnOff = other.m_timerOnOff;
	m_audioActivated = other.m_audioActivated;
	m_lecturerId = other.m_lecturerId;
	
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
void   CLectureMode::SetLectureModeOnOff(BYTE  OnOff)
{
	// From version API_NUM_LECTURE_SHOW (145) m_LectureModeOnOff may be 0/1/2
// From version API_NUM_PRESENTATION_MODE (521) m_LectureModeOnOff may be 0/1/2/3	(3 PRESENTATION Mode)
  m_LectureModeOnOff=OnOff;
}
/////////////////////////////////////////////////////////////////////////////
BYTE    ACCLectureMode::GetLectureModeOnOff() const
{
	return m_LectureModeOnOff;
}
/////////////////////////////////////////////////////////////////////////////
void   CLectureMode::SetLecturerName(const char*  name)
{
	strncpy(m_LecturerName, name, sizeof(m_LecturerName) - 1);
	m_LecturerName[sizeof(m_LecturerName) - 1]= '\0';
}
/////////////////////////////////////////////////////////////////////////////
const char*  ACCLectureMode::GetSetLecturerName () const
{
    return m_LecturerName;
}

/////////////////////////////////////////////////////////////////////////////
void   CLectureMode::SetLectureTimeInterval(WORD  TimeInterval)
{
    m_TimeInterval=TimeInterval;
	
}
/////////////////////////////////////////////////////////////////////////////
WORD   ACCLectureMode::GetLectureTimeInterval()const
{
    return m_TimeInterval;
}

/////////////////////////////////////////////////////////////////////////////
void   CLectureMode::SetTimerOnOff(BYTE  timerOnOff)
{
    m_timerOnOff = timerOnOff;
}
/////////////////////////////////////////////////////////////////////////////
BYTE   ACCLectureMode::GetTimerOnOff()const
{
    return m_timerOnOff;
}

/////////////////////////////////////////////////////////////////////////////
void   CLectureMode::SetAudioActivated(BYTE  audioActivated)
{
    m_audioActivated = audioActivated;
}

/////////////////////////////////////////////////////////////////////////////
BYTE   ACCLectureMode::GetAudioActivated()const
{
    return m_audioActivated;
}

/////////////////////////////////////////////////////////////////////////////
void   CLectureMode::SetLecturerId(DWORD  lecturerId)
{
    m_lecturerId = lecturerId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  ACCLectureMode::GetLecturerId()const
{
    return m_lecturerId;
}


//////////////////////////////////////////////////////////////////////////////
/////////  class ACCAvMsgStruct


ACCAvMsgStruct::ACCAvMsgStruct()
{
    m_attended_welcome=0;
    m_av_msg_service_name[0]='\0';
}
/////////////////////////////////////////////////////////////////////////////
ACCAvMsgStruct::ACCAvMsgStruct(const ACCAvMsgStruct &other)
{
	
    *this=other;
}
/////////////////////////////////////////////////////////////////////////////
ACCAvMsgStruct::~ACCAvMsgStruct()
{
}

/////////////////////////////////////////////////////////////////////////////
ACCAvMsgStruct& ACCAvMsgStruct::operator = (const ACCAvMsgStruct& other)
{
    m_attended_welcome=other.m_attended_welcome;
    strncpy(m_av_msg_service_name,other.m_av_msg_service_name,AV_SERVICE_NAME);
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
ACCAvMsgStruct& ACCAvMsgStruct::operator = (const CAvMsgStruct& other)
{
    m_attended_welcome=other.GetAttendedWelcome();
    strncpy(m_av_msg_service_name,other.GetAvMsgServiceName(), sizeof(m_av_msg_service_name) - 1);
    m_av_msg_service_name[sizeof(m_av_msg_service_name) - 1] = '\0';
    return *this;
}

////////////////////////////////////////////////////////////////////////////
void   ACCAvMsgStruct::SetAttendedWelcome(const BYTE attended_welcome)
{
	m_attended_welcome=attended_welcome;
}

////////////////////////////////////////////////////////////////////////////
BYTE  ACCAvMsgStruct::GetAttendedWelcome() const
{
    return m_attended_welcome;
}

////////////////////////////////////////////////////////////////////////////

void ACCAvMsgStruct::SetAvMsgServiceName(const char* av_msg_service_name)
{
	strncpy(m_av_msg_service_name, av_msg_service_name, sizeof(m_av_msg_service_name) - 1);
	m_av_msg_service_name[sizeof(m_av_msg_service_name) - 1]='\0';
}
//////////////////////////////////////////////////////////////////////////////
const char*  ACCAvMsgStruct::GetAvMsgServiceName() const
{
    return m_av_msg_service_name;
}
