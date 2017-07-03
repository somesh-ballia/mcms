



#include "IVRAvMsgStruct.h"
//#include "NStream.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"


//////////////////////////////////////////////////////////////////////////////
/////////  class CAvMsgStruct


CAvMsgStruct::CAvMsgStruct()
{
    m_attended_welcome=0;
    m_av_msg_service_name[0]='\0';
}
/////////////////////////////////////////////////////////////////////////////
CAvMsgStruct::CAvMsgStruct(const CAvMsgStruct &other)
:CPObject(other)
{
	
    *this=other;
}
/////////////////////////////////////////////////////////////////////////////
CAvMsgStruct::~CAvMsgStruct()
{
}

/////////////////////////////////////////////////////////////////////////////
CAvMsgStruct& CAvMsgStruct::operator = (const CAvMsgStruct& other)
{
	if(this != &other)
	{
    	m_attended_welcome=other.m_attended_welcome;
    	strncpy(m_av_msg_service_name,other.m_av_msg_service_name,AV_SERVICE_NAME);
	}
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CAvMsgStruct::operator == (const CAvMsgStruct& other)
{
	if(m_attended_welcome != other.m_attended_welcome)
	{
		return false;
	}
	if(0 != strcmp(m_av_msg_service_name, other.m_av_msg_service_name))
	{
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CAvMsgStruct::operator != (const CAvMsgStruct& other)
{
	return !operator ==(other); 
}

/////////////////////////////////////////////////////////////////////////////
void CAvMsgStruct::Serialize(WORD format, std::ostream &ostr)
{
	ostr << (WORD)m_attended_welcome << "\n";
	ostr << m_av_msg_service_name << "\n";
	
}

/////////////////////////////////////////////////////////////////////////////
void CAvMsgStruct::DeSerialize(WORD format, std::istream &istr)
{
	// assuming format = OPERATOR_MCMS
	
	WORD tmp;
	istr >> tmp;
	m_attended_welcome = (BYTE)tmp;
	istr.ignore(1);
	istr.getline(m_av_msg_service_name,AV_SERVICE_NAME+1,'\n');
	
}

/////////////////////////////////////////////////////////////////////////////
void CAvMsgStruct::SerializeXml(CXMLDOMElement* pFatherNode)
{
	pFatherNode->AddChildNode("ATTENDED_MODE",m_attended_welcome,ATTENDED_MODE_ENUM);
	pFatherNode->AddChildNode("AV_MSG",m_av_msg_service_name);
}
/////////////////////////////////////////////////////////////////////////////
int	 CAvMsgStruct::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	char* pszVal = NULL;

	GET_VALIDATE_CHILD(pActionNode,"ATTENDED_MODE",&m_attended_welcome,ATTENDED_MODE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"AV_MSG",m_av_msg_service_name,AV_MSG_LENGTH);
	
	if (strlen(m_av_msg_service_name) > 0)
		m_attended_welcome = CONF_IVR;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CAvMsgStruct::CdrSerialize(WORD format, std::ostream &m_ostr)
{
	m_ostr << (WORD)m_attended_welcome << ",";

    // to be copatible with old applications, where 'AV_SERVICE_NAME' was 21
    // the cut of the name should be at the send side.
	// Important - Don't remove the compatibility support!!! (Judith)
    
    const DWORD OLD_AV_SERVICE_NAME = 21;    
    char buffer[OLD_AV_SERVICE_NAME];
    strncpy(buffer, m_av_msg_service_name, OLD_AV_SERVICE_NAME-1);
    buffer[OLD_AV_SERVICE_NAME-1] = '\0';
    
	m_ostr << buffer << ",";
}

/////////////////////////////////////////////////////////////////////////////
void CAvMsgStruct::CdrSerialize(WORD format, std::ostream &m_ostr, BYTE bilflag)
{
    //m_ostr << "attended welcome:"<<(WORD)m_attended_welcome  << "\n";
	
    switch ((WORD)m_attended_welcome) 
	{
	case 0 :
		{
			m_ostr << "attended welcome:none" << "\n";
			break;
		}
	case 1 :
		{
			m_ostr << "attended welcome:welcome but non attended" << "\n";
			break;
		}
	case 2 :
		{
			m_ostr << "attended welcome:attended" << "\n";
			break;
		}
	case 3 :
		{
			m_ostr << "attended welcome:IVR" << "\n";
			break;
		}
	default :
		{
			m_ostr<<"--"<<"\n";
			break;
		}
	}//endswitch
	
    m_ostr << "av msg service name:"<<m_av_msg_service_name   << "\n";
	
}

/////////////////////////////////////////////////////////////////////////////
void CAvMsgStruct::CdrDeSerialize(WORD format, std::istream &m_istr)
{
	// assuming format = OPERATOR_MCMS

	WORD tmp;
//	m_istr.ignore(1);
	m_istr >> tmp;
	m_attended_welcome = (BYTE)tmp;
	m_istr.ignore(1);

	m_istr.getline(m_av_msg_service_name,AV_SERVICE_NAME+1,',');
}

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
void   CAvMsgStruct::SetAttendedWelcome(const BYTE attended_welcome)
{
	m_attended_welcome=attended_welcome;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CAvMsgStruct::GetAttendedWelcome() const
{
    return m_attended_welcome;
}

////////////////////////////////////////////////////////////////////////////

void CAvMsgStruct::SetAvMsgServiceName(const char* av_msg_service_name)
{
	strncpy(m_av_msg_service_name, av_msg_service_name, sizeof(m_av_msg_service_name) - 1);
	m_av_msg_service_name[sizeof(m_av_msg_service_name) -1]='\0';
}
//////////////////////////////////////////////////////////////////////////////
const char*  CAvMsgStruct::GetAvMsgServiceName() const
{
    return m_av_msg_service_name;
}

