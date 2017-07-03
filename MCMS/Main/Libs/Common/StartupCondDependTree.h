#ifndef STARTUPCONDDEPENDTREE_H_
#define STARTUPCONDDEPENDTREE_H_

#include <vector>
using namespace std;

#include "PObject.h"
#include "ActiveAlarmDefines.h"

class CStartupCondNode;
typedef vector<CStartupCondNode*> CChildVector;


const WORD MainRootDummyErrorCode = 0xFFFF;



/*------------------------------------------------------------------------
This is an implementation of a forest of trees. User defines a set of trees(see user level and down)
but the system keeps all this forest in one tree.

System:     root	
            /  \
User :     1    5  
          /\   /\
         2  4 6  7 
        /        
       3
------------------------------------------------------------------------*/








//------------------------------------------------------------------------
// Interface of the CStartupCondNode class
//------------------------------------------------------------------------
class CStartupCondNode : public CPObject
{
CLASS_TYPE_1(CStartupCondNode, CPObject)
public:
	CStartupCondNode(const CStartupCondition &sa);
	virtual const char* NameOf() const { return "CStartupCondNode";}
	virtual ~CStartupCondNode();

	WORD GetErrorCode()const{return m_SA.m_ActiveAlarm.m_ErrorCode;}
	DWORD GetNumOfChilds()const{return m_Childs.size();}
	void AddChild(CStartupCondNode *child);
	CStartupCondNode* RemoveChildByErrorCode(const WORD errorCode);
	bool IsThisYourChildByErrorCode(const WORD errorCode);
	bool IsStartupCondFaultOnly() const {return m_SA.m_isFaultOnly ;} ;
	eStartupConditionStatus GetStartupCondStatus()const{return m_SA.m_eStatus;}
	void SetStartupCondStatus(eStartupConditionStatus status){m_SA.m_eStatus = status;}
	void SetDesciption(const string &description){m_SA.m_ActiveAlarm.m_Description = description;}
	CActiveAlarm& GetActiveAlarm(){return m_SA.m_ActiveAlarm;}
	
	CChildVector::iterator GetFirstChild()	{return m_Childs.begin();}
	CChildVector::iterator GetEndChild()	{return m_Childs.end();}
	
private:
	// disabled
	CStartupCondNode(const CStartupCondNode&);
	CStartupCondNode& operator=(const CStartupCondNode&);

	CStartupCondition m_SA;
	CChildVector m_Childs;
};











//------------------------------------------------------------------------
// Interface of the CStartupCondDependTree class
//------------------------------------------------------------------------
class CStartupCondDependForest : public CPObject
{
CLASS_TYPE_1(CStartupCondDependForest, CPObject)
public:
	CStartupCondDependForest();
	virtual const char* NameOf() const { return "CStartupCondDependForest";}
	virtual ~CStartupCondDependForest();

	CStartupCondNode* GetMainRoot()const {return m_Root;}

	void AddRoot(CStartupCondNode *node);
	CStartupCondNode* FindByErrorCode(WORD errorCode);
	CStartupCondNode *RemoveNodeByErrorCode(WORD errorCode);
	bool IsThereStatus(eStartupConditionStatus status, bool checkOnlyRealAlarm );
	void SwitchStatus(eStartupConditionStatus statusFrom, eStartupConditionStatus statusTo);
	bool UpdateStartupConditionDescriptionByErrorCode(WORD errorCode, const string & description);

private:
	// disabled
	CStartupCondDependForest(const CStartupCondDependForest&);
	CStartupCondDependForest& operator=(const CStartupCondDependForest&);

	CStartupCondNode* FindRecursiveByErrorCode(CStartupCondNode *currentNode, WORD errorCode);
	CStartupCondNode* FindRecursiveByStatus(CStartupCondNode *currentNode, eStartupConditionStatus status, bool checkOnlyRealAlarm = false);
	CStartupCondNode* FindFatherRecursive(CStartupCondNode *currentNode, WORD errorCode);
	void SwitchStatusRecursive(	CStartupCondNode *currentNode,
								eStartupConditionStatus statusFrom,
								eStartupConditionStatus statusTo);

	CStartupCondNode *m_Root;
};

#endif /*STARTUPCONDDEPENDTREE_H_*/
