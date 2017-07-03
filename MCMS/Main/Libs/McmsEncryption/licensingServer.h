// licensingServer.h: interface for the CLicensingServer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef LICENSINGSERVER_H_
#define LICENSINGSERVER_H_


#include "PObject.h"
#include "psosxml.h"
#include "McmsProcesses.h"
#include "DefinesGeneral.h"
#include "ObjString.h"
#include "CommonStructs.h"
#include "SerializeObject.h"

class CMcuMngrProcess;

using namespace std;


class CLicensingServer : public CSerializeObject
{

CLASS_TYPE_1(CLicensingServer, CSerializeObject)
friend class CLicensing;

public:
    CLicensingServer ();
    CLicensingServer( const CLicensingServer &other );
	virtual ~CLicensingServer ();

	const char* NameOf() const {return "CLicensingServer";}
	virtual void Dump(ostream& msg) const;

 	virtual CSerializeObject* Clone(){return new CLicensingServer;}
 	
    virtual void	SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int		DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
    int				DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
    int             DeSerializeXmlFromEma(CXMLDOMElement* pXMLRootElement,char *pszError);
    int             DeSerializeXmlFromFile(CXMLDOMElement* pXMLRootElement,char *pszError);

    CLicensingServer& operator = (const CLicensingServer &rOther);

    void         SetPrimaryLicenseServer(std::string   primaryLicenseServer);
    std::string  GetPrimaryLicenseServer();
    void         SetPrimaryLicenseServerPort(DWORD  primaryLicenseServerPort);
    DWORD        GetPrimaryLicenseServerPort();
    DWORD  		 GetLicenseChangedCounter();
    void         SetLicenseChangedCounter(DWORD updatedCounter);
	
protected:
	std::string                  m_primaryLicenseServer;
	DWORD                        m_primaryLicenseServerPort;
	DWORD 						 m_LicenseChangedCounter;
};


#endif /*LICENSINGSERVER_H_*/
