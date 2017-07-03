// InstallerManager.h: interface for the CInstallerManager class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(_INSTALL_PREV_VER_H__)
#define _INSTALL_PREV_VER_H__

#include "SerializeObject.h"

enum eVersionType
{
	eVersionTypeFallback = 0,
	eVersionTypeFactory,

	NumOfVersionTypes
};

class CInstallPreviousVersion : public CSerializeObject
{
public:
	CInstallPreviousVersion();
	virtual ~CInstallPreviousVersion();

	virtual const char* NameOf() const {return "CInstallPreviousVersion";}
	virtual CSerializeObject* Clone(){return new CInstallPreviousVersion;}
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int	 DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action);

	eVersionType GetVersionType()const{return m_VersionType;}
	CInstallPreviousVersion& operator=(const CInstallPreviousVersion& other);
	
private:
	// disabled
	CInstallPreviousVersion(const CInstallPreviousVersion&);

	eVersionType m_VersionType;
};

#endif // !defined(_INSTALL_PREV_VER_H__)
