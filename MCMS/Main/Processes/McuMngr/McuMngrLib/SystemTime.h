#ifndef _SYSTEM_TIME
#define _SYSTEM_TIME

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"

class CXMLDOMElement;
class CStructTm;

/////////////////////////////////////////////////////////////////////////////
// CSystemTime

#define SYSTEM_TIME_PATH "Cfg/SystemTime.xml"


class CSystemTime : public CSerializeObject
{
CLASS_TYPE_1(CSystemTime,CPObject)
public:
	   //Constructors
	   CSystemTime();
	   CSystemTime(const CSystemTime &other);
	   CSystemTime& operator = (const CSystemTime &other);
	   bool operator == (const CSystemTime& other);

	   bool operator != (const CSystemTime& other);
	   virtual ~CSystemTime();

   static const string NA_IPV6_ADDRESS;
	// Implementation
	virtual CSerializeObject* Clone(){return new CSystemTime;}
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	void SetParams(const CSystemTime &other);

    const char*  NameOf() const;
    const  CStructTm*  GetMCUTime() const;
    BYTE   GetGMTOffsetSign () const;
    BYTE   GetGMTOffset () const;
	void   GetGMTOffset (BYTE & GMT_offset_sign,            // 0 for '-' 1 for '+'
					     BYTE & GMT_offset_hours,           //  0-15 hours
					     BYTE & GMT_offset_minutes)  const; // 0,5,10,15,...,55 in minutes
	BYTE   GetIsNTP ()  const;
	void   SetIsNTP (const BYTE  Is_NTP);
	std::string  GetNTP_IPAddress (int index) const;
	const char*  GetNTP_IPv6_Address (int index) const;
	const char** GetNTP_IPv6_Addresses() const { return (const char**)m_NTP_IPV6_ADDRESS;}
	bool IsNTPIpv6AddressesEmpty() const;
	void   UpdateCurrentTime();

	eNtpServerStatus GetNtpServerStatus(int index) const;
	void			 SetNtpServerStatus(int index, eNtpServerStatus serverStatus);

	WORD 			GetNumFailuresSinceConnecting(int index) const;
	void			SetNumFailuresSinceConnecting(int index, WORD numFailures);

	void PrintParams(string theCaller);



protected:
  	 // Attributes
    CStructTm* m_pMCUTime;
    BYTE  m_GMT_offset_sign;
    BYTE  m_GMT_offset;

	BYTE				m_bIsNTP;
	std::string			m_NTP_IP_ADDRESS[NTP_MAX_NUM_OF_SERVERS];
	char				m_NTP_IPV6_ADDRESS[NTP_MAX_NUM_OF_SERVERS][IPV6_ADDRESS_LEN];
	eNtpServerStatus	m_NtpServerStatus[NTP_MAX_NUM_OF_SERVERS];
	WORD 				m_numFailuresSinceConnecting[NTP_MAX_NUM_OF_SERVERS];
};


/////////////////////////////////////////////////////////////////////////////
#endif /* _SYSTEM_TIME */
