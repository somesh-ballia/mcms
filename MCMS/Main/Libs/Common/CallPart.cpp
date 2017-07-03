/*$Header: /MCMS/MAIN/subsys/mcmsoper/CALLPART.CPP 7     25/11/01 14:13 Oshi $*/
//+========================================================================+
//                            CALLPART.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CALLPART.CPP                                                |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#include "CallPart.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "CDRDefines.h"
#include "StatusesGeneral.h"

////////////////////////////////////////////////////////////////////////////////
// class CCallingParty
ACCCallingParty::ACCCallingParty()
{
	int i;
	m_numType         = 0;
	m_numPlan         = 0;
	m_presentationInd = 0xFF; 
	m_screeningInd    = 0xff; 

	for(i=0;i<PRI_LIMIT_PHONE_DIGITS_LEN;i++)
		m_digits[i]=0;
		
	m_digits[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';

}

/////////////////////////////////////////////////////////////////////////////
ACCCallingParty::ACCCallingParty(const ACCCallingParty &other)
{
	*this=other;
	
}

/////////////////////////////////////////////////////////////////////////////
ACCCallingParty::~ACCCallingParty()
{
}

/////////////////////////////////////////////////////////////////////////////
ACCCallingParty& ACCCallingParty::operator = (const ACCCallingParty& other)
{
	int i;
	m_numType         = other.m_numType;
	m_numPlan         = other.m_numPlan;
	m_presentationInd = other.m_presentationInd;
	m_screeningInd    = other.m_screeningInd;

	for(i=0;i<PRI_LIMIT_PHONE_DIGITS_LEN;i++)
		m_digits[i]=other.m_digits[i];

	m_digits[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool ACCCallingParty::operator== (const ACCCallingParty& other)
{
    if(m_numType != other.m_numType)
    {
        return false;
    }
    if(m_numPlan != other.m_numPlan)
    {
        return false;
    }
    if(m_presentationInd != other.m_presentationInd)
    {
        return false;
    }
    if(m_screeningInd != other.m_screeningInd)
    {
        return false;
    }
    if(0 != strncmp(m_digits, other.m_digits,  PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER))
    {
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
bool ACCCallingParty::operator!= (const ACCCallingParty& other)
{
    return !ACCCallingParty::operator==(other);
}


/////////////////////////////////////////////////////////////////////////////
CCallingParty::CCallingParty()
{
}

/////////////////////////////////////////////////////////////////////////////
CCallingParty::~CCallingParty()
{
}

/////////////////////////////////////////////////////////////////////////////
CCallingParty::CCallingParty(const ACCCallingParty &other)
{                     
	m_numType         = other.GetCallingNumType(); 
	m_numPlan         = other.GetCallingNumPlan();
	m_presentationInd = other.GetCallingPresentInd();
	m_screeningInd    = other.GetCallingScreenInd();
	strncpy(m_digits,other.GetCallingPhoneNum(),PRI_LIMIT_PHONE_DIGITS_LEN);
	m_digits[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';
	
}


/////////////////////////////////////////////////////////////////////////////
void CCallingParty::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag)
{

	m_ostr << "** Calling Party **" << "\n";

//	switch (m_numType) 
//	{
//	case UNKNOWN_TYPE :
//		{
//			m_ostr << "number type:unknown" << "\n";
//			break;
//		}
//	case INTERNATIONAL :
//		{
//			m_ostr << "number type:international" << "\n";
//			break;
//		}
//	case NATIONAL :
//		{
//			m_ostr << "number type:national" << "\n";
//			break;
//		}
//	case SUBSCRIBER :
//		{
//			m_ostr << "number type:subscriber" << "\n";
//			break;
//		}
//	case ABBREVIATED :
//		{
//			m_ostr << "number type:abbreviated" << "\n";
//			break;
//		}
//	default :
//		{
//			m_ostr<<"--"<<"\n";
//			break;
//		}
//	}//end witch num type	
	

	switch (m_numPlan) 
	{
	case UNKNOWN_PLAN :
		{
			m_ostr << "number plan:unknown" << "\n";
			break;
		}
	case ISDN :
		{
			m_ostr << "number plan:ISDN" << "\n";
			break;
		}
	case TELEPHONY :
		{
			m_ostr << "number plan:telephony" << "\n";
			break;
		}
	case PRIVATE :
		{
			m_ostr << "number plan:private" << "\n";
			break;
		}
	default :
		{
			m_ostr<<"--"<<"\n";
			break;
		}
	}//end switch number plan	
	

	switch (m_presentationInd) 
	{
	case ALLOWED :
		{
			m_ostr << "presentation ind:allowed" << "\n";
			break;
		}
	case RESTRICTED :
		{
			m_ostr << "presentation ind:restricted" << "\n";
			break;
		}
	case NOT_AVAILABLE :
		{			
			m_ostr << "presentation ind:not available" << "\n";
			break;
		}
	default :
		{
			m_ostr<<"--"<<"\n";
			break;
		}
	}//end switch presentation ind	
	

	switch (m_screeningInd) 
	{
	case NOT_SCREENED :
		{
			m_ostr << "screening ind:not screened"<< "\n";
			break;
		}
	case USER_VER_PASSED :
		{
			m_ostr << "screening ind:user ver passed" << "\n";
			break;
		}
	case USER_VER_FAILED :
		{
			m_ostr << "screening ind:user ver failed" << "\n";
			break;
		}
	default :
		{
			m_ostr<<"--"<<"\n";
			break;
		}
	}//end switch screening ind				

	m_ostr <<"calling phone number:"<< m_digits << "\n";
		  
}

/////////////////////////////////////////////////////////////////////////////
void CCallingParty::Serialize(WORD format, std::ostream &m_ostr)
{
	m_ostr << (WORD)m_numType << ","; 
	m_ostr << (WORD)m_numPlan   << ",";     
	m_ostr << (WORD)m_presentationInd << ","; 
	m_ostr <<	(WORD)m_screeningInd << ",";
	m_ostr << m_digits << ",";
	
}

/////////////////////////////////////////////////////////////////////////////
void CCallingParty::DeSerialize(WORD format, std::istream &m_istr)
{
	// assuming format = OPERATOR_MCMS
	
	WORD tmp;
	
	m_istr >> tmp;
	m_numType=(BYTE)tmp;

	m_istr.ignore(1);

	m_istr >> tmp;
	m_numPlan=(BYTE)tmp;

	m_istr.ignore(1);

	m_istr >> tmp;
	m_presentationInd=(BYTE)tmp;

	m_istr.ignore(1);

	m_istr >> tmp;
	m_screeningInd=(BYTE)tmp;

	m_istr.ignore(1);

	m_istr.getline(m_digits,PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER,',');
} 

/////////////////////////////////////////////////////////////////////////////
void CCallingParty::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pPartyNode = pFatherNode->AddChildNode("CALLING_PARTY");

	pPartyNode->AddChildNode("NUM_TYPE",m_numType,NUM_TYPE_ENUM);
	pPartyNode->AddChildNode("PLAN_TYPE",m_numPlan,NUM_PLAN_TYPE_ENUM);
	pPartyNode->AddChildNode("PRESENTATION_INDICATOR",m_presentationInd,PRESENTATION_INDICATOR_TYPE_ENUM);
	pPartyNode->AddChildNode("SCREEN_INDICATOR",m_screeningInd,SCREEN_INDICATOR_TYPE_ENUM);
	pPartyNode->AddChildNode("PHONE1",m_digits);
}
/////////////////////////////////////////////////////////////////////////////
int  CCallingParty::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	char* pszDummy = NULL;

	GET_VALIDATE_CHILD(pActionNode,"NUM_TYPE",&m_numType,NUM_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"PLAN_TYPE",&m_numPlan,NUM_PLAN_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"PRESENTATION_INDICATOR",&m_presentationInd,PRESENTATION_INDICATOR_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"SCREEN_INDICATOR",&m_screeningInd,SCREEN_INDICATOR_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"PHONE1",m_digits,PRI_LIMIT_PHONE_DIGITS_LENGTH);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
const char*  CCallingParty::NameOf() const                
{
	return "CCallingParty";
}

////////////////////////////////////////////////////////////////////////////
void   CCallingParty::SetCallingNumType(const BYTE numtype)                 
{
	m_numType = numtype;
}

////////////////////////////////////////////////////////////////////////////
BYTE  ACCCallingParty::GetCallingNumType() const
{
    return m_numType;
}

////////////////////////////////////////////////////////////////////////////
				
void CCallingParty::SetCallingNumPlan(const BYTE numplan)
{
   m_numPlan=numplan;
}

//////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCallingParty::GetCallingNumPlan() const
{
    return m_numPlan;
}

///////////////////////////////////////////////////////////////////////////////		               
void   CCallingParty::SetCallingPresentInd(const BYTE presentationind)
{
  m_presentationInd=presentationind;
}

///////////////////////////////////////////////////////////////////////////////	
BYTE ACCCallingParty::GetCallingPresentInd() const
{
   return m_presentationInd;
}

///////////////////////////////////////////////////////////////////////////////
void   CCallingParty::SetCallingScreenInd(const BYTE screeningind)
{
    m_screeningInd=screeningind;
}

////////////////////////////////////////////////////////////////////////////////		
BYTE ACCCallingParty::GetCallingScreenInd() const
{
  return  m_screeningInd;
}

/////////////////////////////////////////////////////////////////////////////
void  CCallingParty::SetCallingPhoneNum(const char* phonenumber)
{
   int len=strlen(phonenumber);
   strncpy(m_digits, phonenumber, PRI_LIMIT_PHONE_DIGITS_LEN );
   if (len>=PRI_LIMIT_PHONE_DIGITS_LEN)
        m_digits[PRI_LIMIT_PHONE_DIGITS_LEN]='\0';
}

/////////////////////////////////////////////////////////////////////////////
const char*  ACCCallingParty::GetCallingPhoneNum() const                 
{
    return m_digits;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCalledParty

ACCCalledParty::ACCCalledParty()
{
	int i;
	m_called_numType = 0;
	m_called_numPlan = 0;

	for(i=0;i<PRI_LIMIT_PHONE_DIGITS_LEN;i++)
		m_called_digits[i]=0;
	
	m_called_digits[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';	
}

/////////////////////////////////////////////////////////////////////////////
ACCCalledParty::ACCCalledParty(const ACCCalledParty &other)
{
	*this=other;
}

/////////////////////////////////////////////////////////////////////////////
ACCCalledParty::~ACCCalledParty()
{
}

/////////////////////////////////////////////////////////////////////////////
ACCCalledParty& ACCCalledParty::operator = (const ACCCalledParty& other)
{
	int i;
	m_called_numType = other.m_called_numType;
	m_called_numPlan = other.m_called_numPlan;
	
	for(i=0;i<PRI_LIMIT_PHONE_DIGITS_LEN;i++)
		m_called_digits[i]=other.m_called_digits[i];

	m_called_digits[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';
	
	return *this;
	
}

/////////////////////////////////////////////////////////////////////////////
bool ACCCalledParty::operator== (const ACCCalledParty& other)
{
    if(m_called_numType != other.m_called_numType)
    {
        return false;
    }
    if(m_called_numPlan != other.m_called_numPlan)
    {
        return false;
    }
    if(0 != strncmp(m_called_digits, other.m_called_digits, PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER))
    {
        return false;
    }
    return true;
}
bool ACCCalledParty::operator!= (const ACCCalledParty& other)
{
    return !ACCCalledParty::operator==(other);
}



/////////////////////////////////////////////////////////////////////////////
CCalledParty::CCalledParty()
{
}

/////////////////////////////////////////////////////////////////////////////
CCalledParty::~CCalledParty()
{
}

/////////////////////////////////////////////////////////////////////////////
CCalledParty::CCalledParty(const ACCCalledParty &other)
{  
	m_called_numType  = other.GetCalledNumType();
	m_called_numPlan  = other.GetCalledNumPlan();
	strncpy(m_called_digits,other.GetCalledPhoneNum(), PRI_LIMIT_PHONE_DIGITS_LEN);
	m_called_digits[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';
} 


/////////////////////////////////////////////////////////////////////////////
void CCalledParty::Serialize(WORD format, std::ostream &m_ostr, BYTE bilflag)
{
	m_ostr << "** Called Party **" << "\n";	
	switch (m_called_numType)
	{
	case CALLED_UNKNOWN_TYPE :
		{
			m_ostr << "number type:unknown" << "\n";
			break;
		}
	case CALLED_INTERNATIONAL :
		{
			m_ostr << "number type:international" << "\n";
			break;
		}
	case CALLED_NATIONAL :
		{
			m_ostr << "number type:national" << "\n";
			break;
						  }
	case CALLED_SUBSCRIBER :
		{
			m_ostr << "number type:subscriber" << "\n";
			break;
		}
	case CALLED_ABBREVIATED :
		{
			m_ostr << "number type:abbreviated" << "\n";
			break;
							 }
	default :
		{
			m_ostr<<"--"<<"\n";
			break;
		}
	}//end switch num type											
	switch (m_called_numPlan) 
	{
	case CALLED_UNKNOWN_PLAN :
		{
			m_ostr << "number plan:unknown" << "\n";
			break;
		}
	case CALLED_ISDN :
		{
			m_ostr << "number plan:ISDN" << "\n";
			break;
		}
	case CALLED_TELEPHONY :
		{
			m_ostr << "number plan:telephony" << "\n";
			break;
		}
	case CALLED_PRIVATE :
		{
			m_ostr << "number plan:private" << "\n";
			break;
		}
	default :
		{
			m_ostr<<"--"<<"\n";
			break;
		}
	}//end switch number plan											

	m_ostr << "called phone number:"<<m_called_digits << ";\n";
} 

/////////////////////////////////////////////////////////////////////////////
void CCalledParty::Serialize(WORD format, std::ostream &m_ostr)
{
	m_ostr << (WORD)m_called_numType << ","; 
	m_ostr << (WORD)m_called_numPlan   << ",";
	m_ostr << m_called_digits << ",;\n";
} 

/////////////////////////////////////////////////////////////////////////////
void CCalledParty::DeSerialize(WORD format, std::istream &m_istr)
{
	// assuming format = OPERATOR_MCMS
	
	WORD tmp;
	// m_istr.ignore(1);
	m_istr >> tmp;
	m_called_numType=(BYTE)tmp;

	m_istr.ignore(1);

	m_istr >> tmp;
	m_called_numPlan=(BYTE)tmp;

	m_istr.ignore(1);

	m_istr.getline(m_called_digits,PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER,',');
} 

/////////////////////////////////////////////////////////////////////////////
void CCalledParty::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pPartyNode = pFatherNode->AddChildNode("CALLED_PARTY");

	pPartyNode->AddChildNode("NUM_TYPE",m_called_numType,NUM_TYPE_ENUM);
	pPartyNode->AddChildNode("PLAN_TYPE",m_called_numPlan,NUM_PLAN_TYPE_ENUM);
	pPartyNode->AddChildNode("PHONE1",m_called_digits);
}
/////////////////////////////////////////////////////////////////////////////
int  CCalledParty::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	char* pszDummy = NULL;

	GET_VALIDATE_CHILD(pActionNode,"NUM_TYPE",&m_called_numType,NUM_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"PLAN_TYPE",&m_called_numPlan,NUM_PLAN_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"PHONE1",m_called_digits,PRI_LIMIT_PHONE_DIGITS_LENGTH);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
const char*  CCalledParty::NameOf() const                
{
	return "CCalledParty";
}

////////////////////////////////////////////////////////////////////////////
void   CCalledParty::SetCalledNumType(const BYTE numtype)                 
{
	m_called_numType=numtype;
}

////////////////////////////////////////////////////////////////////////////
BYTE  ACCCalledParty::GetCalledNumType() const
{
    return m_called_numType;
}

////////////////////////////////////////////////////////////////////////////
void CCalledParty::SetCalledNumPlan(const BYTE numplan)
{
	m_called_numPlan=numplan;
}

//////////////////////////////////////////////////////////////////////////////	 
BYTE  ACCCalledParty::GetCalledNumPlan() const
{
    return m_called_numPlan;
}

/////////////////////////////////////////////////////////////////////////////
void  CCalledParty::SetCalledPhoneNum(const char* phonenumber)
{
	int len=strlen(phonenumber);
	strncpy(m_called_digits, phonenumber, PRI_LIMIT_PHONE_DIGITS_LEN );

	if (len>=PRI_LIMIT_PHONE_DIGITS_LEN)
        m_called_digits[PRI_LIMIT_PHONE_DIGITS_LEN]='\0';
	
}

/////////////////////////////////////////////////////////////////////////////
const char*  ACCCalledParty::GetCalledPhoneNum() const                 
{
    return m_called_digits;
}




