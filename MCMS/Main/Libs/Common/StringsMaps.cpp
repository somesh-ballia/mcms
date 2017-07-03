//+========================================================================+
//                            StringsMaps.cpp                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       StringsMaps.cpp                                             |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Anatoly                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#include <string.h>
#include <stdio.h>
#include "StringsMaps.h"
#include "Trace.h"
#include "XmlDefines.h"
#include "InitCommonStrings.h"
#include "PObject.h"
//
CItem ** CStringsMaps::m_pItemsArray;
WORD  CStringsMaps::m_bInit=FALSE;


///////////////////////////////////////////////////////////////
void CStringsMaps::CleanUp()
{
	if(m_bInit==TRUE)
	{
		m_bInit=FALSE;

		for (int i=0; i<MAX_NUMBER_OF_ITEMS; i++)
		{
			if (m_pItemsArray[i])
			{
				m_pItemsArray[i]->DeleteList();
				delete m_pItemsArray[i];
			}
		}
		delete [] m_pItemsArray;
		m_pItemsArray = NULL;
	}
}


///////////////////////////////////////////////////////////////
void CStringsMaps::Build()
{
	if(m_bInit==TRUE)
		return ;

	m_bInit=TRUE;

	m_pItemsArray = new CItem*[MAX_NUMBER_OF_ITEMS];
	for(int i=0;i<MAX_NUMBER_OF_ITEMS;i++)
	{
		m_pItemsArray[i]=NULL;
	}
}

///////////////////////////////////////////////////////////////
void CStringsMaps::AddMinMaxItem(int type,int min,int max)
{
	AddItem(type,min,"MIN");
	AddItem(type,max,"MAX");

}

///////////////////////////////////////////////////////////////
void CStringsMaps::AddItem(int type,
						   int value,
						   const char *strDescription)
{
	
	if (m_pItemsArray[type]==NULL)
		m_pItemsArray[type]=new CItem(value,strDescription);
	else
	{
		CItem *pItem=m_pItemsArray[type];
		CItem *pPrevItem=NULL;
		while (pItem!=NULL)
		{
			
			if(pItem->GetValue()==value)
			{
				
				DBGFPASSERT(1);

				char strFormat[256];
				sprintf(strFormat,"%d ,type %d",value,type);
				FPTRACE2(eLevelInfoHigh, "CStringsMaps::AddItem - duplicated value ", strFormat);

				return;

			}
			if(strcmp(pItem->GetDescription(),strDescription)==0)
			{
				bool bNotIsOpus = strncmp("opus",strDescription, 4);
				if (bNotIsOpus)	// only if NOT opus! (opus and  opus-128 has the same description)
				{
					DBGFPASSERT(1);

					char strFormat[256];
					snprintf(strFormat, sizeof(strFormat), "%s ,type %d",strDescription,type);
					FPTRACE2(eLevelInfoHigh, "CStringsMaps::AddItem -duplicated description: ", strFormat);

					return;
				}

			}

			pPrevItem=pItem;	
			pItem=pItem->GetNextItem();
		}
		pItem=m_pItemsArray[type];
		pPrevItem=pItem;
		while (pItem!=NULL)
		{
			
			if(pItem->GetValue()>value)
				break;

			pPrevItem=pItem;	
			pItem=pItem->GetNextItem();
		}
		CItem *pNextItem=pPrevItem->GetNextItem();
//		POBJDELETE(pItem);
//		pItem = NULL;
		//char * newStr = new char[strlen(strDescription) + 1];
		//strcpy(newStr, strDescription);
		//pItem=new CItem(value,newStr);

		pItem=new CItem(value,strDescription);
//		if(pItem != NULL)
//		{
			pItem->SetNextItem(pNextItem);
//		}
		pPrevItem->SetNextItem(pItem);
		
			
	}
	
}

///////////////////////////////////////////////////////////////
BYTE CStringsMaps::GetDescription(int type,
								  int value,
								  const char **strDescription)
{
	if (m_pItemsArray == NULL)
	{
		FPASSERT(1);
		return FALSE;
	}
	if(m_pItemsArray[type])
	{
		CItem *pItem=m_pItemsArray[type];
		CItem *pPrevItem=NULL;
		BYTE bFound=FALSE;
		while (pItem!=NULL)
		{
			if(pItem->GetValue()==value)
			{
				bFound=TRUE;
				*strDescription=pItem->GetDescription();
				break;
			}
			pItem=pItem->GetNextItem();

		}
		if(bFound==FALSE&&type==_BOOL)
		{
			if (value>0)
				*strDescription="true";
			else
				*strDescription="false";
			return TRUE;
		}

		return bFound;
	}
	else
		return FALSE;
		 
}

///////////////////////////////////////////////////////////////
// this function should not assert.
BOOL CStringsMaps::CheckExistance(const int type, const int value)
{
    if (m_pItemsArray == NULL)
	{
		return FALSE;
	}
    if(NULL == m_pItemsArray[type])
    {
        return FALSE;
    }

    CItem *pItem = m_pItemsArray[type];
    while (NULL != pItem)
    {
        if(pItem->GetValue() == value)
        {
            return TRUE;
        }
        pItem = pItem->GetNextItem();
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////
BYTE CStringsMaps::GetValue(int type,int &value,char *strDescription)
{
	if(m_pItemsArray[type])
	{
		CItem *pItem=m_pItemsArray[type];
		CItem *pPrevItem=NULL;
		BYTE bFound=FALSE;
		int size=0;
		char* strDescriptionTemp = NULL;

		//remove \n and \t from the beginning of the description	
		if (strDescription[0]=='\n')
		{
			strDescriptionTemp = new char[strlen(strDescription)];
			int index = 0;
			while ((strDescription[index]=='\t') || (strDescription[index]=='\n'))
				index++;
			strcpy(strDescriptionTemp, strDescription+index);
			strcpy(strDescription, strDescriptionTemp);
		}
		//remove \n from the end of the description	
		char* ch = strchr(strDescription, '\n');
		if (ch!=NULL)
		{
			int index = ch - strDescription;
			strDescription[index-1] = '\0';
		}
		//remove \t from the end of the description	
		ch = strchr(strDescription, '\t');
		if (ch!=NULL)
		{
			int index = ch - strDescription;
			strDescription[index] = '\0';
		}
				
		while (pItem!=NULL)
		{
			if(strDescription[0]==pItem->GetDescription()[0])
			{
				if(strcmp(pItem->GetDescription(),strDescription)==0)
				{
					bFound=TRUE;
					value=pItem->GetValue();
					break;
				}
			}
			pItem=pItem->GetNextItem();

		}
		if (strDescriptionTemp!=NULL)
			delete [] strDescriptionTemp;

		return bFound;
	}
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////
BOOL CStringsMaps::IsDefineType(int type)
{
	if(type<0||type>=MAX_NUMBER_OF_ITEMS)
		return FALSE;

	if(m_pItemsArray[type])
		return TRUE;
	else
		return FALSE;

}

///////////////////////////////////////////////////////////////
CItem::CItem(int value,const char *pDescription)
{
//	if(this == NULL)
//	{
//		return;
//	}

	m_Value=value;
	m_pDescription=pDescription;
	if(m_pDescription!=NULL)
		m_DesctiptionLen=strlen(m_pDescription);
	else
		m_DesctiptionLen=0;
	m_pNextItem=NULL;
}

///////////////////////////////////////////////////////////////
CItem* CItem::GetNextItem()
{
	return m_pNextItem;
}

///////////////////////////////////////////////////////////////
void CItem::DeleteList()
{
	CItem* pHeadItem;
	while (m_pNextItem)
	{
		pHeadItem = m_pNextItem;
		m_pNextItem = m_pNextItem->m_pNextItem;
		delete (pHeadItem);
		pHeadItem = NULL;
	}
}

///////////////////////////////////////////////////////////////
int CItem::GetValue()
{
	return m_Value;
}

///////////////////////////////////////////////////////////////
const char *CItem::GetDescription()
{
	return m_pDescription;
}

///////////////////////////////////////////////////////////////
int CItem::GetDescriptionLength()
{
	return m_DesctiptionLen;
}

///////////////////////////////////////////////////////////////
void CItem::SetNextItem(CItem *pItem)
{
	m_pNextItem=pItem;
}
