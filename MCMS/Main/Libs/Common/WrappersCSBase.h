#ifndef WRAPPERSCSBASE_H_
#define WRAPPERSCSBASE_H_

#include "PObject.h"
#include "McuMngrStructs.h"


#define BOOL_TO_STRING(val) (TRUE == (val) ? "True" : "False")



class CBaseWrapper : public CPObject
{
CLASS_TYPE_1(CBaseWrapper,CPObject )	
public:
	CBaseWrapper();
	virtual ~CBaseWrapper();
	virtual void Dump(std::ostream&) const {};
	virtual const char* NameOf() const { return "CBaseWrapper";}
	
protected:
	void DumpHeader(std::ostream& os,  const char *header) const;	
};




/*-----------------------------------------------------------------------------
	class CAliasWrapper
-----------------------------------------------------------------------------*/
class CAliasWrapper : public CBaseWrapper
{	
public:
	CAliasWrapper(const ALIAS_S &data);
	virtual ~CAliasWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CAliasWrapper";}

private:
	const ALIAS_S &m_Data;	
};




#endif /*WRAPPERSCSBASE_H_*/
