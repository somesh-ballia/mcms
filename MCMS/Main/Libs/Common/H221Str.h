// CH221StrCap.h: interface of the CH221StrCap class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _H221Str_H__
#define _H221Str_H__


//#include "NStream.h"
#include "PObject.h"

class CH221Str : public CPObject
{
CLASS_TYPE_1(CH221Str,CPObject )
public:             

								// Constructors
	CH221Str();
	CH221Str(const CH221Str &other);
	~CH221Str();
								// Initializations  
						
								// Operations
	virtual const char* NameOf() const { return "CH221Str";}
	
  virtual void   Dump(std::ostream  &ostr);
  virtual void   DumpHex(std::ostream  &ostr);
								 
								// serialize operations 

  virtual void    Serialize(WORD format, std::ostream  &ostr);     
  void    Serialize(WORD format, std::ostream  &ostr, BYTE billing);     
  void    Serialize(WORD format, std::ostream  &ostr, WORD fullformat) ;     
  virtual void    DeSerialize(WORD format, std::istream  &istr);
  void     DeSerialize(WORD format, std::istream  &istr, BYTE bilflag);
  void    SetH221FromString(const DWORD strLen,const char* pszCH221String);
  void	  SetDump(BYTE bDump);
  BYTE IsDump();
 								
	                           // general operations 
	DWORD  GetLen() const; 
	BYTE*  GetPtr() const;
  
	virtual CH221Str&  operator=(const CH221Str &other);
	bool  operator==(const CH221Str &other);
 	
                    
protected:        
								// Attributes
	WORD   m_size;
	BYTE*  m_pStr;
	BYTE   m_bDump;
};




#endif //_H221Str_H__
