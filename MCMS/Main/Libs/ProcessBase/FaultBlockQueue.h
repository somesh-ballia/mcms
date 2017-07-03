#ifndef _FAULT_BLOCK_QUEUE_H__
#define _FAULT_BLOCK_QUEUE_H__


#include <list>
using namespace std;

#include "DataTypes.h"
#include "HlogElement.h"



struct FaultBlockNode
{   
public:
    FaultBlockNode(DWORD time, CLogFltElement *pNode)
            :m_Time(time), m_Node(pNode){}
    ~FaultBlockNode(){delete m_Node; m_Node = NULL;}

    bool IsSimilar(const FaultBlockNode &other)const{return m_Node->IsSimilar(*(other.m_Node));}

    DWORD GetTime()const{return m_Time;}
    CLogFltElement* GetElement()const{return m_Node;}
    

private:
    // disabled
    FaultBlockNode(const FaultBlockNode&);
    FaultBlockNode&operator=(const FaultBlockNode&);

    DWORD m_Time;
    CLogFltElement *m_Node;
};







class CFaultBlockQueue : public list<FaultBlockNode*>
{
public:
    CFaultBlockQueue();
    virtual ~CFaultBlockQueue();

    void Push(FaultBlockNode *pNode);
    void Replace(FaultBlockNode *pNodeOld, FaultBlockNode *pNodeNew);
    FaultBlockNode* FindSimilar(const FaultBlockNode *pNode);
    
    
private:
    // disabled
    CFaultBlockQueue(const CFaultBlockQueue&);
    CFaultBlockQueue&operator=(const CFaultBlockQueue&);
    
    iterator Find(const FaultBlockNode *pNode);
    bool CheckPreconditions_Push(const FaultBlockNode *pNode);
    bool CheckPreconditions_Replace(const FaultBlockNode *pNode);
};


#endif // _FAULT_BLOCK_QUEUE_H__
