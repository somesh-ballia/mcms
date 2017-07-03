
#include "FaultBlockQueue.h"




const DWORD FAULT_BLOCK_QUEUE_MAX_SIZE = 10;
















/////////////////////////////////////////////////////////////////////////////
CFaultBlockQueue::CFaultBlockQueue()
{}

/////////////////////////////////////////////////////////////////////////////
CFaultBlockQueue::~CFaultBlockQueue()
{
    iterator iCurrent = begin();
    iterator iEnd = end();

    for(; iCurrent != iEnd ; iCurrent++)
    {
        FaultBlockNode *pCurrent = *iCurrent;
        delete pCurrent;
    }
}

/////////////////////////////////////////////////////////////////////////////
void CFaultBlockQueue::Push(FaultBlockNode *pNode)
{
    bool res = CheckPreconditions_Push(pNode);
    if(false == res)
    {
    	POBJDELETE(pNode);	//Judith - delete the object
        return;
    }
    
    const size_t listSize = size();
    if(listSize == FAULT_BLOCK_QUEUE_MAX_SIZE)
    {
        FaultBlockNode *pOldNode = front();
        POBJDELETE(pOldNode);
        
        pop_front();
    }

    push_back(pNode);
}

/////////////////////////////////////////////////////////////////////////////
void CFaultBlockQueue::Replace(FaultBlockNode *pNodeOld, FaultBlockNode *pNodeNew)
{
    bool res = CheckPreconditions_Replace(pNodeOld);
    if(false == res)
    {
    	POBJDELETE(pNodeNew);	//judith - someone needs to delete the new node in case of an error
        return;
    }
    
    iterator iter = Find(pNodeOld);
    insert(iter, 1, pNodeNew);

    FaultBlockNode *pOldNode = *iter;
    remove(pNodeOld);
    POBJDELETE(pNodeOld);
}

/////////////////////////////////////////////////////////////////////////////
FaultBlockNode* CFaultBlockQueue::FindSimilar(const FaultBlockNode *pNode)
{
    iterator iter = Find(pNode);
    FaultBlockNode *pSimilar = (iter != end() ? *iter : NULL);
    return pSimilar;
}

/////////////////////////////////////////////////////////////////////////////
CFaultBlockQueue::iterator CFaultBlockQueue::Find(const FaultBlockNode *pNode)
{
    iterator iCurrent = begin();
    iterator iEnd = end();

    for(; iCurrent != iEnd ; iCurrent++)
    {
        const FaultBlockNode *pCurrent = *iCurrent;
        if(true == pNode->IsSimilar(*pCurrent))
        {
            break;
        }        
    }
    return iCurrent;
}

/////////////////////////////////////////////////////////////////////////////
bool CFaultBlockQueue::CheckPreconditions_Push(const FaultBlockNode *pNode)
{
    FaultBlockNode *pSimilar = FindSimilar(pNode);
    if(NULL != pSimilar)
    {
        perror("CFaultBlockQueue::CheckPreconditions_Push, NULL != pSimilar");
        return false;
    }
    const size_t listSize = size();
    if(FAULT_BLOCK_QUEUE_MAX_SIZE < listSize)
    {
        perror("CFaultBlockQueue::CheckPreconditions_Push, listSize > FAULT_BLOCK_QUEUE_MAX_SIZE");
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CFaultBlockQueue::CheckPreconditions_Replace(const FaultBlockNode *pOldNode)
{
    FaultBlockNode *pNode = FindSimilar(pOldNode);
    if(NULL == pNode)
    {
        perror("CheckPreconditions_Replace: NULL == pNode");
        return false;
    }
    return true;
}

