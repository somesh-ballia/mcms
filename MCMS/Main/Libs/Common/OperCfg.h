// OperCfg.h: interface for the COperCfg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(COPERCFG__H_)
#define COPERCFG__H_

#include <string>
#include "string.h"

#include "SerializeObject.h"
#include "Transactions.h"
#include "CommonStructs.h"

//#define OPERATOR_NAME_LEN   20  //20 characters + 1 + terminator

// //Operator's authorization group
// #define SUPER           0
// #define ORDINARY        1
// #define AUTH_OPERATOR   2
// #define RECORDING_USER  3
// #define RECORDING_ADMIN 4
// #define GUEST           100

// ////////	 operators action types	  ///////
// #define DEL_OPERATOR      40  //Delete operator
// #define NEW_OPERATOR      41  //Add operator
// #define UPDATE_OPERATOR   42  //Update operator
// #define LOG_IN_OPERATOR   43
// #define LOG_OUT_OPERATOR  44
// #define CLOSE_CONNECTION  45


/////////////////////////////////////////////////////////////////////////////
// COperCfg 

class COperCfg : public CSerializeObject
{
CLASS_TYPE_1(COperCfg, CSerializeObject)
public:
//Constructors
    COperCfg();
    COperCfg(const COperCfg &other);
	virtual const char* NameOf() const { return "COperCfg";}
    virtual ~COperCfg();

    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action = 0);
    CSerializeObject* Clone() {return new COperCfg;}

    void   SetLogin(const std::string  login);
    const std::string&   GetLogin () const;
    void   SetPassword(const std::string  password);
    const std::string&   GetPassword () const;
    const std::string&  GetOldPassword () const;
    void   SetAuthorization(const WORD group);
    WORD   GetAuthorization() const;
    void   SetCompatibilityLevel(const WORD level);
    WORD   GetCompatibilityLevel() const;
    void   SetStationName(const std::string  name);
    const std::string&   GetStationName () const;
    void   SetOperSoftVersion(const VERSION_S  softVersion);
    const  VERSION_S  GetOperSoftVersion () const;
    void   SetApiNumber(const DWORD api_number);
    DWORD  GetApiNumber() const;
    DWORD  GetMcuIp () const;
    void   SetMcuIp(DWORD  ip);
    DWORD  GetMcuPort () const;
    void   SetMcuPort(DWORD  port);
    WORD   GetCompressionLevel () const;
    void   SetCompressionLevel(WORD  CompressionLevel);
    void   SetConferenceRecorderLogin(const BYTE group);
    BYTE   GetConferenceRecorderLogin() const;
    const std::string& GetMcuHostName () const;
    void   SetMcuHostName(const std::string ip);

protected:

    // Attributes
	std::string m_login;
	std::string m_password;
	std::string m_oldPassword;
	std::string m_stationName;
	std::string m_McuHostName;
	VERSION_S  m_operVersion;
    WORD m_authorizationGroup; 
    WORD m_compatibilityLevel;
    DWORD m_apiNumber;
    DWORD m_reserved1;
    DWORD m_reserved2;
    DWORD m_McuIp;
    DWORD m_McuPort;
    WORD  m_CompressionLevel;
    BYTE  m_ConferenceRecorderLogin;

};

#endif // !defined(COPERCFG__H_)
