#ifndef UDPAPPENDER_H_
#define UDPAPPENDER_H_

#include "AppenderConfiguration.h"

#include <vector>
#include <string>
using namespace std;


class CUDPAppenderConfiguration : public CAppenderConfiguration
{
CLASS_TYPE_1(CUDPAppenderConfiguration,CAppenderConfiguration)
public:

    //Constructors
    
	CUDPAppenderConfiguration();                                                                            
	CUDPAppenderConfiguration(const CUDPAppenderConfiguration &other);
	CUDPAppenderConfiguration& operator = (const CUDPAppenderConfiguration& other);
	virtual ~CUDPAppenderConfiguration();
	virtual CSerializeObject* Clone() {return new CUDPAppenderConfiguration();}  
	virtual void   SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int    DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);

	const char*  NameOf() const {return "CUDPAppenderConfiguration";}  

	int				GetDuration();
	void				SetDuration(DWORD duration);

protected:
	DWORD			m_duration;
	
  
};

/////////////////////////////////////////////////////////////////////////
#endif /*MODULECONTENT_H_*/



