#include <algorithm>

#include "StartupCondDependTree.h"



class CStartupCondNodeHunterByErrorCode : public unary_function<CStartupCondNode*, bool>
{
public:
	CStartupCondNodeHunterByErrorCode(WORD errorCode)
	:m_ErrorCode(errorCode)
	{
	}
	
	bool operator () (CStartupCondNode *& elem)
	{
		return elem->GetErrorCode() == m_ErrorCode;
	}
	
private:
	const WORD m_ErrorCode;
};




//------------------------------------------------------------------------
// Implementation of the CStartupCondNode class
//------------------------------------------------------------------------
CStartupCondNode::CStartupCondNode(const CStartupCondition &sa)
:m_SA(sa)
{
	
}

CStartupCondNode::~CStartupCondNode()
{
	CChildVector::iterator iTer = GetFirstChild();
	CChildVector::iterator iEnd = GetEndChild();
	while(iTer != iEnd)
	{
		PDELETE(*iTer);
		
		iTer++;
	}
}

void CStartupCondNode::AddChild(CStartupCondNode *child)
{	
	m_Childs.push_back(child);
}

CStartupCondNode* CStartupCondNode::RemoveChildByErrorCode(const WORD errorCode)
{
	CStartupCondNodeHunterByErrorCode hunter(errorCode);
	CChildVector::iterator iBegin = GetFirstChild();
	CChildVector::iterator iFinish= GetEndChild();
	CChildVector::iterator iFnd = find_if(iBegin, iFinish, hunter);
	CStartupCondNode *child = *iFnd;
	m_Childs.erase(iFnd);
	return child;
}

bool CStartupCondNode::IsThisYourChildByErrorCode(const WORD errorCode)
{
	CStartupCondNodeHunterByErrorCode hunter(errorCode);
	CChildVector::iterator iBegin = GetFirstChild();
	CChildVector::iterator iFinish= GetEndChild();
	CChildVector::iterator iFind = find_if(iBegin, iFinish, hunter);
	bool res = (iFind != iFinish);
	return res;
}









//------------------------------------------------------------------------
// Implementation of the CStartupCondDependTree class
//------------------------------------------------------------------------
CStartupCondDependForest::CStartupCondDependForest()
{
	CStartupCondition sa;
	sa.m_ActiveAlarm.m_ErrorCode = MainRootDummyErrorCode; 
	m_Root = new CStartupCondNode(sa);
}

CStartupCondDependForest::~CStartupCondDependForest()
{
	PDELETE(m_Root);
}

void CStartupCondDependForest::AddRoot(CStartupCondNode *node)
{
	m_Root->AddChild(node);
}

CStartupCondNode* CStartupCondDependForest::RemoveNodeByErrorCode(WORD errorCode)
{
	CStartupCondNode *father = FindFatherRecursive(m_Root, errorCode);
	if(NULL == father)
	{
		PASSERTMSG(TRUE, "Bad Error Code");
		return NULL;
	}
	
	CStartupCondNode *child = father->RemoveChildByErrorCode(errorCode);
	return child;
}

CStartupCondNode* CStartupCondDependForest::FindByErrorCode(WORD errorCode)
{
	if(MainRootDummyErrorCode == errorCode)
	{
		PASSERTMSG(1, "Illegal Error Code");
		return NULL;
	}
	CStartupCondNode *found = FindRecursiveByErrorCode(m_Root, errorCode);
	return found;
}

CStartupCondNode* CStartupCondDependForest::FindRecursiveByErrorCode(CStartupCondNode *currentNode, WORD errorCode)
{
	if(errorCode == currentNode->GetErrorCode())
	{
		return currentNode;
	}
	
	CChildVector::iterator iTer = currentNode->GetFirstChild();
	CChildVector::iterator iEnd = currentNode->GetEndChild();
	while(iTer != iEnd)
	{
		CStartupCondNode *child = *iTer;
		CStartupCondNode *found = FindRecursiveByErrorCode(child, errorCode);
		if(NULL != found)
		{
			return found;
		}
		iTer++;
	}
	
	return NULL;
}

CStartupCondNode* CStartupCondDependForest::FindRecursiveByStatus(	CStartupCondNode *currentNode,
																	eStartupConditionStatus status,
																	bool checkOnlyRealAlarm)
{
	// if checkOnlyRealAlarm then it is not considered when it is fault only alaram
	if(status == currentNode->GetStartupCondStatus() && (!checkOnlyRealAlarm || !currentNode->IsStartupCondFaultOnly()))
	{
		return currentNode;
	}

	CChildVector::iterator iTer = currentNode->GetFirstChild();
	CChildVector::iterator iEnd = currentNode->GetEndChild();
	while(iTer != iEnd)
	{
		CStartupCondNode *child = *iTer;
		CStartupCondNode *found = FindRecursiveByStatus(child, status, checkOnlyRealAlarm);
		if(NULL != found)
		{
			return found;
		}
		iTer++;
	}
	
	return NULL;
}

CStartupCondNode* CStartupCondDependForest::FindFatherRecursive(CStartupCondNode *currentNode, WORD errorCode)
{
	bool res = currentNode->IsThisYourChildByErrorCode(errorCode);
	if(true == res)
	{
		return currentNode;
	}
	
	CChildVector::iterator iTer = currentNode->GetFirstChild();
	CChildVector::iterator iEnd = currentNode->GetEndChild();
	while(iTer != iEnd)
	{
		CStartupCondNode *child = *iTer;
		CStartupCondNode *found = FindFatherRecursive(child, errorCode);
		if(NULL != found)
		{
			return found;
		}
		iTer++;
	}
	
	return NULL;
}

void CStartupCondDependForest::SwitchStatusRecursive(	CStartupCondNode *currentNode, 
														eStartupConditionStatus statusFrom, 
														eStartupConditionStatus statusTo)
{
	const eStartupConditionStatus currentStatus = currentNode->GetStartupCondStatus();
	if(currentStatus == statusFrom)
	{
		currentNode->SetStartupCondStatus(statusTo);
	}
	
	CChildVector::iterator iTer = currentNode->GetFirstChild();
	CChildVector::iterator iEnd = currentNode->GetEndChild();
	while(iTer != iEnd)
	{
		CStartupCondNode *child = *iTer;
		SwitchStatusRecursive(child, statusFrom, statusTo);

		iTer++;
	}
}

bool CStartupCondDependForest::IsThereStatus(eStartupConditionStatus status, bool checkOnlyRealAlarm )
{
	CChildVector::iterator iTer = m_Root->GetFirstChild();
	CChildVector::iterator iEnd = m_Root->GetEndChild();
	while(iTer != iEnd)
	{
		CStartupCondNode *child = *iTer;
		CStartupCondNode *node = FindRecursiveByStatus(child, status, checkOnlyRealAlarm);
		if(NULL != node)
		{
			return true;
		}

		iTer++;
	}

	return false;
}

void CStartupCondDependForest::SwitchStatus(eStartupConditionStatus statusFrom, eStartupConditionStatus statusTo)
{
	CChildVector::iterator iTer = m_Root->GetFirstChild();
	CChildVector::iterator iEnd = m_Root->GetEndChild();
	while(iTer != iEnd)
	{
		CStartupCondNode *child = *iTer;
		SwitchStatusRecursive(child, statusFrom, statusTo);
	
		iTer++;
	}
}

bool CStartupCondDependForest::UpdateStartupConditionDescriptionByErrorCode(WORD errorCode, const string & description)
{
	CStartupCondNode *node = FindByErrorCode(errorCode);
	if(NULL != node)
	{
		node->SetDesciption(description);
	}
	return (NULL != node);
}










