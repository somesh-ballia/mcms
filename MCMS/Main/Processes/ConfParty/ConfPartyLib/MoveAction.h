// RsrvPartyAction.h: interface for the CRsrvPartyAction class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class conf actions with one int param - such as auto layout = on/off
//========   ==============   =====================================================================



#if !defined(_MoveAction_H__)
#define _MoveAction_H__


#include "NStream.h"
#include "psosxml.h"
#include "SerializeObject.h"
#include "ConfContactInfo.h"
#include"ConfPartySharedDefines.h"
#include"CDRDefines.h"

class CMoveBaseAction : public CSerializeObject
{
CLASS_TYPE_1(CMoveBaseAction, CSerializeObject)
public:

	//Constructors
	CMoveBaseAction();
  //	CMoveBaseAction(const CCMoveBaseAction &other);
  //	CMoveBaseActionn& operator = (const CMoveBaseAction& other);
	virtual ~CMoveBaseAction();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
//	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CMoveBaseAction;}

  //	int convertStrActionToNumber(const char * strAction);
		
	const char * NameOf() const {return "CMoveBaseAction";}
    
	
	DWORD GetSourceConfID()const;
	DWORD GetPartyID()const;

	  
protected:

	DWORD       m_SourceConfID;
	DWORD       m_PartyID;
};

class CMoveAction : public CMoveBaseAction
{
CLASS_TYPE_1(CMoveAction, CMoveBaseAction)
public:

	//Constructors
	CMoveAction();
  //	CMoveAction(const CMoveAction &other);
  //	CMoveAction& operator = (const CMoveAction& other);
	virtual ~CMoveAction();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
//	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CMoveAction;}

	int convertStrActionToNumber(const char * strAction);
		
	const char * NameOf() const {return "CMoveAction";}
    
	
	DWORD GetTargetConfID()const;

	  
protected:

	DWORD       m_TargetConfID;
};


#endif // !defined(_MoveAction_H__)

