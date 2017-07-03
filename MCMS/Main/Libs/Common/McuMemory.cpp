//+========================================================================+
//                            McuMemory.cpp                                |
//                                                                         |
//                                      .                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       McuMemory.cpp                                               |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Haggai                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |  24.2.05   |                                                      |
//+========================================================================+                     


#include <string.h>
#include "McuMemory.h"
#include "Transactions.h"
#include "psosxml.h"
#include "XmlDefines.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"

/////////////////////////////////////////////////////////////////////////////
// CMcuMemory

CMcuMemory::CMcuMemory()
{ 
	m_RAMtotal              = 0;
	m_partitionBufFree      = 0;
	m_regionMemFree         = 0;
	m_largestAvailableAlloc = 0;
	m_fragmentationProc     = 0;
	m_partitionLimit        = 0;
	m_regionLimit           = 0;
	m_fragmentationLimit    = 0;
}


/////////////////////////////////////////////////////////////////////////////
CMcuMemory::CMcuMemory(const CMcuMemory &other)
	:CSerializeObject(other)
{
	m_RAMtotal              = other.m_RAMtotal;
	m_partitionBufFree      = other.m_partitionBufFree;
	m_regionMemFree         = other.m_regionMemFree;
	m_largestAvailableAlloc = other.m_largestAvailableAlloc;
	m_fragmentationProc     = other.m_fragmentationProc;
	m_partitionLimit        = other.m_partitionLimit;
	m_regionLimit           = other.m_regionLimit;
	m_fragmentationLimit    = other.m_fragmentationLimit;
	
}

/////////////////////////////////////////////////////////////////////////////
CMcuMemory::~CMcuMemory()
{
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CMcuMemory::GetRAMtotal () const                 
{
    return m_RAMtotal;
}

/////////////////////////////////////////////////////////////////////////////
void  CMcuMemory::SetRAMtotal(const DWORD  num)                 
{
	m_RAMtotal = num;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CMcuMemory::GetPartitionBufFree () const                 
{
    return m_partitionBufFree;
}

/////////////////////////////////////////////////////////////////////////////
void  CMcuMemory::SetPartitionBufFree(const DWORD  proc)                 
{
	m_partitionBufFree = proc;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CMcuMemory::GetRegionMemFree () const                 
{
    return m_regionMemFree;
}

/////////////////////////////////////////////////////////////////////////////
void  CMcuMemory::SetRegionMemFree(const DWORD  proc)                 
{
	m_regionMemFree = proc;
}


/////////////////////////////////////////////////////////////////////////////
DWORD  CMcuMemory::GetLargestAvailableAlloc () const                 
{
    return m_largestAvailableAlloc;
}

/////////////////////////////////////////////////////////////////////////////
void  CMcuMemory::SetLargestAvailableAlloc(const DWORD  proc)                 
{
	m_largestAvailableAlloc=proc;
}


/////////////////////////////////////////////////////////////////////////////
DWORD  CMcuMemory::GetFragmentationProc () const                 
{
    return m_fragmentationProc;
}

/////////////////////////////////////////////////////////////////////////////
void  CMcuMemory::SetFragmentationProc(const DWORD  proc)                 
{
	m_fragmentationProc = proc;
}


/////////////////////////////////////////////////////////////////////////////
DWORD  CMcuMemory::GetPartitionLimit () const                 
{
    return m_partitionLimit;
}

/////////////////////////////////////////////////////////////////////////////
void  CMcuMemory::SetPartitionLimit(const DWORD  proc)                 
{
	m_partitionLimit = proc;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CMcuMemory::GetRegionLimit () const                 
{
    return m_regionLimit;
}

/////////////////////////////////////////////////////////////////////////////
void  CMcuMemory::SetRegionLimit(const DWORD  proc)                 
{
	m_regionLimit = proc;
}


/////////////////////////////////////////////////////////////////////////////
DWORD  CMcuMemory::GetFragmentationLimit () const                 
{
    return m_fragmentationLimit;
}

/////////////////////////////////////////////////////////////////////////////
void  CMcuMemory::SetFragmentationLimit(const DWORD  proc)                 
{
	m_fragmentationLimit = proc;
}


/////////////////////////////////////////////////////////////////////////////
void CMcuMemory::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pMcuMemoryNode = pFatherNode->AddChildNode("MCU_MEMORY_STATE");
	pMcuMemoryNode->AddChildNode("RAM_TOTAL",m_RAMtotal);
	pMcuMemoryNode->AddChildNode("PARTITION_BUFFER_FREE",m_partitionBufFree);
	pMcuMemoryNode->AddChildNode("REGION_MEMORY_FREE",m_regionMemFree);
	pMcuMemoryNode->AddChildNode("LARGEST_AVAILABLE_ALLOCATION",m_largestAvailableAlloc);
	pMcuMemoryNode->AddChildNode("FRAGMENTATION_PROC",m_fragmentationProc);
	pMcuMemoryNode->AddChildNode("PARTITION_LIMIT",m_partitionLimit);
	pMcuMemoryNode->AddChildNode("REGION_LIMIT",m_regionLimit);
	pMcuMemoryNode->AddChildNode("FRAGMENTATION_LIMIT",m_fragmentationLimit);
} 

/////////////////////////////////////////////////////////////////////////////
// schema file name:  obj_mcu_memory_state.xsd
int CMcuMemory::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"RAM_TOTAL",&m_RAMtotal,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"PARTITION_BUFFER_FREE",&m_partitionBufFree,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"REGION_MEMORY_FREE",&m_regionMemFree,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"LARGEST_AVAILABLE_ALLOCATION",&m_largestAvailableAlloc,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"FRAGMENTATION_PROC",&m_fragmentationProc,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"PARTITION_LIMIT",&m_partitionLimit,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"REGION_LIMIT",&m_regionLimit,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"FRAGMENTATION_LIMIT",&m_fragmentationLimit,_0_TO_DWORD);

	return STATUS_OK;
}

