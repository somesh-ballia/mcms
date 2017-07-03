/*$Header:   M:/SCM/m3cv1d1/subsys/mcms/RSRC.H_v   1.0   08 Jul 1997 17:15:56   CARMI  $*/
// rsrc.h : interface of the CRsrc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "PObject.h"

// An abstract class for all interface objects.
class CInterface : public CPObject
{
	CLASS_TYPE_1(CInterface, CPObject)
public: 
	// Constructors
	CInterface();
	virtual ~CInterface();  
		
	virtual const char* NameOf() const { return "CInterface";}
	// Operations
	
   
protected:

	
	// Attributes
	// Operations	
};

#endif /* _INTERFACE_H  */
///////////////////////////////





