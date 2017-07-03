//+========================================================================+
//                            PObject.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PObject.CPP                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Sami| 4/5/95     |                                                      |
//+========================================================================+

#include <iomanip>
#include "NStream.h"
#include "PObject.h"
#include "SystemFunctions.h"
#include "Trace.h"

/////////////////////////////////////////////////////////////////////////////
//                          CPObject     
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
CPObject::CPObject()
{ 
	m_validFlag = OBJ_STAMP; 
	m_type = "CPObject";

}

/////////////////////////////////////////////////////////////////////////////
CPObject::~CPObject() 
{ 
	m_validFlag = 0;
}  


/////////////////////////////////////////////////////////////////////////////
BOOL CPObject::IsValidPObjectPtr(const CPObject*  p)
{ 
	BOOL rval = FALSE;
#ifndef LINUX
	if ( p != NULL)
	{	
		try
		{
			if(SystemIsBadReadPtr((const void*)p,sizeof(p)) == FALSE)
			{
				if( p->m_validFlag == OBJ_STAMP ) 
					rval = TRUE;	
			}
		}
		catch(...) 
		{
			rval = FALSE;
			DBGFPASSERT(1);
		}
		
	}
#else
	if ( p != NULL)
	{	
		if( p->m_validFlag == OBJ_STAMP ) 
		{
			rval = TRUE;
		}
	}
#endif
	return rval;
}   

/////////////////////////////////////////////////////////////////////////////
void CPObject::Dump(std::ostream& str) const
{
	str << 	std::setw(20) << "this " << (std::hex) << (DWORD)this << "\n"
		<< 	std::setw(20) << "type " << m_type << "\n"
		<< 	std::setw(20) << "NameOf " << NameOf() << "\n"
		<< 	std::setw(20) << "validFlag " << (std::hex) << m_validFlag << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void CPObject::Dump(WORD level) const
{
	Dump(NULL,level);
}

/////////////////////////////////////////////////////////////////////////////
void CPObject::Dump(const char* title, WORD level) const
{
	COstrStream msg;
	
	if(title != NULL)
		msg << title;
	
	Dump(msg);
	
	PTRACE(level,msg.str().c_str());
}

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator<< (std::ostream& os, const CPObject& obj )
{
	obj.Dump(os);
	return os;
}

