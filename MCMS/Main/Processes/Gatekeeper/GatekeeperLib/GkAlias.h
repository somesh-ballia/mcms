#ifndef GKALIAS_H_
#define GKALIAS_H_

#include "PObject.h"
#include "DefinesIpService.h"
#include "GKManagerStructs.h"

class CGkAlias : public CPObject
{
	CLASS_TYPE_1(CGkAlias , CPObject)      
public:
	CGkAlias();
	CGkAlias(ALIAS_S *pAliasSt);
	virtual ~CGkAlias() {};
	virtual const char* NameOf() const { return "CGkAlias";}

	BYTE IsValidIndex(int index);
	char *GetAliasContent(int index);
	char *GetFirstAliasContent();
	WORD GetFirstAliasType();
	void SetAliasContent(char *pAliasContent,int index);
	WORD GetAliasType(int index);
	void  SetAliasType(WORD aliasType,int index);
	ALIAS_S *GetAliases();
	void SetAliases(ALIAS_S *pAlias);
	WORD GetAliasesLength() const;
	WORD GetAliasesNumber() const;
	
private:
	ALIAS_S		m_pAlias[MAX_ALIAS_NAMES_NUM];
};

#endif /*GKALIAS_H_*/
