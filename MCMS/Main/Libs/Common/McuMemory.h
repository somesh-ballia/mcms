//+========================================================================+
//                            McuMemory.h                                  |
//                                                                         |
//                                                                         |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       McuMemory.h                                                 |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Haggai                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |  24.2.05   |                                                      |
//+========================================================================+                     

#ifndef _MCU_MEMORY
#define _MCU_MEMORY

#include "SerializeObject.h"

/////////////////////////////////////////////////////////////////////////////
// CMcuMemory 

class CMcuMemory : public CSerializeObject
{
CLASS_TYPE_1(CMcuMemory,CSerializeObject )	
public:
	CMcuMemory();                                                                            
	CMcuMemory(const CMcuMemory &other);
	virtual ~CMcuMemory();
	virtual const char* NameOf() const { return "CMcuMemory";}

	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action = 0);
	CSerializeObject* Clone() {return new CMcuMemory;}

    DWORD  GetRAMtotal () const;                 
    void   SetRAMtotal(const DWORD  num);                 
    DWORD  GetPartitionBufFree () const;                 
    void   SetPartitionBufFree(const DWORD  proc);                 
    DWORD  GetRegionMemFree () const;                 
    void   SetRegionMemFree(const DWORD  proc);                 
    DWORD  GetLargestAvailableAlloc () const;                 
    void   SetLargestAvailableAlloc(const DWORD  proc);                 
    DWORD  GetFragmentationProc () const;                 
    void   SetFragmentationProc(const DWORD  proc);                 
    DWORD  GetPartitionLimit () const;                 
    void   SetPartitionLimit(const DWORD  proc);                 
    DWORD  GetRegionLimit () const;                 
    void   SetRegionLimit(const DWORD  proc);                 
    DWORD  GetFragmentationLimit () const;                 
    void   SetFragmentationLimit(const DWORD  proc);                 

protected:
  	 // Attributes
    DWORD  m_RAMtotal;      
    DWORD  m_partitionBufFree;  
    DWORD  m_regionMemFree;
    DWORD  m_largestAvailableAlloc;
    DWORD  m_fragmentationProc;      
    DWORD  m_partitionLimit;      
    DWORD  m_regionLimit;      
    DWORD  m_fragmentationLimit;      
};


/////////////////////////////////////////////////////////////////////////////
#endif /* _MCU_MEMORY */
