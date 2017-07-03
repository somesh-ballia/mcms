// KeyCode.h: interface for the CKeyCode class.
//
//////////////////////////////////////////////////////////////////////

#ifndef KEYCODE_H_
#define KEYCODE_H_


#include "PObject.h"
#include "psosxml.h"
#include "McmsProcesses.h"
#include "DefinesGeneral.h"
#include "ObjString.h"
#include "CommonStructs.h"
#include "SerializeObject.h"

class CMcuMngrProcess;

using namespace std;

#define VERSION_BITMASK_LEN				8 // in 'U' keycode
#define EXACT_KEYCODE_LENGTH_IN_GL1		24




class CKeyCode : public CSerializeObject
{

CLASS_TYPE_1(CKeyCode, CSerializeObject)

public:
	CKeyCode (CMedString keyCodeStr="DummyKeyCode");
	CKeyCode (const char * str);
    CKeyCode( const CKeyCode &other );
	virtual ~CKeyCode ();

	const char* NameOf() const {return "CKeyCode";}
	virtual void Dump(ostream& msg) const;

 	virtual CSerializeObject* Clone(){return new CKeyCode;}
 	
    virtual void	SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int		DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
    int				DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	CKeyCode& operator = (const CKeyCode &rOther);
	CKeyCode& operator = (const char* rOther);

	void        SetKeyCode(CMedString KeyCode);
    CMedString  GetKeyCode();
    char        GetKeyCodeType();
    int         GetKeyCodeLength();

	STATUS	Validate(eProductType productType, CSmallString mplSerialNumberStr, int verMajor=-1, int verMinor=-1, VERSION_S  *keyCodeVersion=NULL);
	void	GenerateInputBitmask(eProductType productType, CSmallString &inBitmask, int verMajor=-1, int verMinor=-1, VERSION_S  *keyCodeVersion=NULL);
	void	GenerateKeyCode(char keycode[], char serial[], char option[], char c_type);
	void	GetOptionsFromKeyCode(char option_mask[], char key_code[]);
	
	// for U_keyCode (to be moved to derived class)
	void GetVersion(VERSION_S& ver);
	
	
protected:
	CMedString  m_keyCode;
};


#endif /*KEYCODE_H_*/
