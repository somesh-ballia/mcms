//GkAlias.cpp

#include "GkAlias.h"

/////////////////////////////////////////////////////////////////////////////
CGkAlias::CGkAlias()
{   
   for( int i = 0; i< MAX_ALIAS_NAMES_NUM; i++ )
   {
	   m_pAlias[i].aliasContent[0] = '\0';
	   m_pAlias[i].aliasType	= 0;
   }
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGkAlias::IsValidIndex(int index)
{
	if(index < MAX_ALIAS_NAMES_NUM)
		return TRUE;
	else 
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
char *CGkAlias::GetAliasContent(int index)
{
	if(!IsValidIndex(index))
		return NULL;

	return (char*)(&m_pAlias[index].aliasContent[0]);
}

/////////////////////////////////////////////////////////////////////////////
char *CGkAlias::GetFirstAliasContent()
{
	return (char*)(&m_pAlias[0].aliasContent[0]);
}

/////////////////////////////////////////////////////////////////////////////
WORD CGkAlias::GetFirstAliasType()
{
	return m_pAlias[0].aliasType;
}

/////////////////////////////////////////////////////////////////////////////
void CGkAlias::SetAliasContent(char *pAliasContent,int index)
{
	int len = strlen(pAliasContent);

	if(IsValidIndex(index))
	{	
		strncpy((char*)(&m_pAlias[index].aliasContent[0]),pAliasContent,ALIAS_NAME_LEN);
		if(len > ALIAS_NAME_LEN)
			m_pAlias[index].aliasContent[ALIAS_NAME_LEN-1] = '\0';
	}
}

/////////////////////////////////////////////////////////////////////////////
WORD CGkAlias::GetAliasType(int index)
{
	if(!IsValidIndex(index))
		return 0;

	return m_pAlias[index].aliasType;
}

/////////////////////////////////////////////////////////////////////////////
void  CGkAlias::SetAliasType(WORD aliasType,int index)
{
	if(IsValidIndex(index))
		m_pAlias[index].aliasType = aliasType;
}

/////////////////////////////////////////////////////////////////////////////
ALIAS_S *CGkAlias::GetAliases()
{
	return m_pAlias;
}

/////////////////////////////////////////////////////////////////////////////
void CGkAlias::SetAliases(ALIAS_S *pAlias)
{
	for(int i=0; i<MAX_ALIAS_NAMES_NUM; i++ )
	{
		SetAliasType(pAlias[i].aliasType,i);
		SetAliasContent((char*)(pAlias[i].aliasContent),i);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Returns Total length of all aliases
WORD CGkAlias::GetAliasesLength() const
{
	DWORD length = 0;
	for(int i=0; i<MAX_ALIAS_NAMES_NUM; i++ )
		length+= strlen((char*)(m_pAlias[i].aliasContent));
	return length;	
}

/////////////////////////////////////////////////////////////////////////////
// Returns number of defined aliases (non Null aliases)
WORD CGkAlias::GetAliasesNumber() const
{
	DWORD num = 0;
	for(int i=0; i<MAX_ALIAS_NAMES_NUM; i++ )	
		if(m_pAlias[i].aliasContent[0] != '\0') 
			num++;
	return num;	
}
