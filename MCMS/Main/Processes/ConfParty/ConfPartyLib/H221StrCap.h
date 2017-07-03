//+========================================================================+
//                            H320Caps.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:   CH221StrCap.h: interface of the CH221StrCap class.                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 10.10.07    |                                                      |
//+========================================================================+

#ifndef _H221strCap_H
#define _H221strCap_H

#include <ostream>

#include "H221Str.h"


class CSegment;

class CH221strCap : public CH221Str
{
CLASS_TYPE_1(CH221strCap, CH221Str)
    public:
    virtual void Dump(std::ostream& ostr); //Capabilities Dump
    virtual const char* NameOf() const { return "CH221strCap";}
};


	
class CH221strCapDrv : public CH221strCap
{
CLASS_TYPE_1(CH221strCapDrv, CH221strCap)
public:
  void  DeSerialize(WORD format,CSegment& Seg);
  void  Serialize(WORD format,CSegment& Seg);
  virtual const char* NameOf() const { return "CH221strCapDrv";}
};
	


class CH221strCom : public CH221Str
{
CLASS_TYPE_1(CH221strCom,CH221Str )
    public:
    virtual void Dump(std::ostream& ostr); //CommMode Dump
    virtual const char* NameOf() const { return "CH221strCom";}
};

	
class CH221strComDrv : public CH221strCom
{
CLASS_TYPE_1(CH221strComDrv,CH221strCom )
public:
  void  DeSerialize(WORD format,CSegment& Seg);
  void  Serialize(WORD format,CSegment& Seg);
  virtual const char* NameOf() const { return "CH221strComDrv";}
};
	



#endif // !defined(_H221strCap_H)
