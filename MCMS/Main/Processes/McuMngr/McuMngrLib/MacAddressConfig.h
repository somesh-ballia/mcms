// MacAddressConfig.h: interface for the CMacAddressConfig class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MACADDRESSCONFIG_H_
#define MACADDRESSCONFIG_H_



#include "SerializeObject.h"
//#include "CommonStructs.h"
//#include "StringsLen.h"

class CMacAddressConfig : public CSerializeObject
{
CLASS_TYPE_1(CMacAddressConfig, CSerializeObject)

public:
	CMacAddressConfig();
	CMacAddressConfig(const CMacAddressConfig &other);
	virtual ~CMacAddressConfig();
	CMacAddressConfig& operator=(const CMacAddressConfig&);
	virtual const char*  NameOf() const {return "CMacAddressConfig";}
	virtual CSerializeObject* Clone(){return new CMacAddressConfig;}

	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	const string  GetEth1MacAddress();
	void		  SetEth1MacAddress(const string sAddress);

	const string  GetEth2MacAddress();
	void		  SetEth2MacAddress(const string sAddress);


private:
	string m_eth1MacAddress;
	string m_eth2MacAddress;
};



#endif /*MACADDRESSCONFIG_H_*/
