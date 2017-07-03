

#ifndef _IVRAVMSGSTRUCT_H__ 
#define _IVRAVMSGSTRUCT_H__


#include "DataTypes.h"
#include "ConfPartyApiDefines.h"
#include "PObject.h"
#include "ConfPartySharedDefines.h"
#include "CDREvent.h"

//class ACCAvMsgStruct;
class CAvMsgStruct;
class COstrStream; 
class CIstrStream;
class CXMLDOMElement;
//class CPObject;

#ifndef DLLPORT
#define DLLPORT
#endif

/*
#ifndef _BYTE_DEFINED
#define _BYTE_DEFINED
typedef unsigned char BYTE;

#endif // !_BYTE_DEFINED
*/

///////////////////////////////////////////////////////////////
/*
class DLLPORT ACCAvMsgStruct
{
public:
	   //Constructors
	ACCAvMsgStruct();
	ACCAvMsgStruct(const ACCAvMsgStruct &other);
	virtual ~ACCAvMsgStruct();
	ACCAvMsgStruct& operator = (const ACCAvMsgStruct& other);
   const BYTE  GetAttendedWelcome() const;                 
	const char*  GetAvMsgServiceName() const;                   
	void  SetAttendedWelcome(const BYTE attended_welcome);
	void  SetAvMsgServiceName(const char* av_msg_service_name);

protected:
	BYTE    m_attended_welcome;
	char   	m_av_msg_service_name[AV_SERVICE_NAME];

};
*/

///////////////////////////////////////////////////////////////

class CAvMsgStruct  : public CPObject /*,public ACCAvMsgStruct*/
{
CLASS_TYPE_1(CAvMsgStruct,CPObject)  //**check macro**
public:
	   //Constructors
	CAvMsgStruct();
	CAvMsgStruct(const CAvMsgStruct &other);
	~CAvMsgStruct();
	
	virtual const char* NameOf() const { return "CAvMsgStruct";}
	
	CAvMsgStruct& operator = (const CAvMsgStruct& other);
	bool operator == (const CAvMsgStruct& other);
	bool operator != (const CAvMsgStruct& other);
	
	// Implementation
	
	//char* Serialize(WORD format);	
///	void Serialize(WORD format, COstrStream &m_ostr);	
///	void DeSerialize(WORD format, CIstrStream &m_istr);
	void   Serialize(WORD format, std::ostream  &m_ostr);
 	void   DeSerialize(WORD format, std::istream &m_istr);

	void CdrSerialize(WORD format, std::ostream &m_ostr);
	void CdrSerialize(WORD format, std::ostream &m_ostr, BYTE bilflag);
	void CdrDeSerialize(WORD format, std::istream &m_istr);
		
	void SerializeXml(CXMLDOMElement* pFatherNode);
	int	 DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	
	
	BYTE  GetAttendedWelcome() const;
	const char*  GetAvMsgServiceName() const;                   
	void  SetAttendedWelcome(const BYTE attended_welcome);
	void  SetAvMsgServiceName(const char* av_msg_service_name);

protected:
	BYTE    m_attended_welcome;
	char   	m_av_msg_service_name[AV_SERVICE_NAME];
};


#endif /* _IVRAVMSGSTRUCT_H__ */


