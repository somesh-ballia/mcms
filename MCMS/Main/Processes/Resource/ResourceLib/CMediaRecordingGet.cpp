// CMediaRecordinglGet.cpp: implementation of the CRsrcDetailGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//
//========   ==============   =====================================================================

#include "CMediaRecordingGet.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ApiStatuses.h"
 
///////////////////////////////////////////////////////////////////////////////////

CJunction::CJunction()
{
 m_Id = 0;
 m_pDescription = NULL;
}
///////////////////////////////////////////////////////////////////////////////////
CJunction::CJunction(WORD id, char* p_description)
{

 m_Id = id;
 
 if( ! p_description)
   m_pDescription = p_description;
  
  else
  {
   WORD length = strlen(p_description); 
   m_pDescription = new char[length+1];
   strncpy(m_pDescription,p_description, length);
   m_pDescription[length] = '\0';
  }

}
///////////////////////////////////////////////////////////////////////////////////
CJunction::CJunction(const CJunction& rhs):CSerializeObject(rhs)
{
 m_Id = rhs.m_Id;
 
 if( ! rhs.m_pDescription)
  m_pDescription = rhs.m_pDescription;
  
 else
 {
  WORD length = strlen(rhs.m_pDescription); 
  m_pDescription = new char[length+1];
  strncpy(m_pDescription, rhs.m_pDescription, length);
  m_pDescription[length] = '\0';
 }
 
}
////////////////////////////////////////////////////////////////////////////////////
CJunction::~CJunction()
{
  if(m_pDescription)
   delete [] m_pDescription;
}
////////////////////////////////////////////////////////////////////////////////////
const CJunction& CJunction::operator=(const CJunction& rhs)
{
 m_Id = rhs.m_Id;
 
 if( m_pDescription == rhs.m_pDescription 
                    && m_pDescription != NULL)
  PASSERT(1);
  
 else
 {
  if(m_pDescription != NULL)
   delete m_pDescription;
   
  if( ! rhs.m_pDescription)
   m_pDescription = rhs.m_pDescription;
  
  else
  {
   WORD length = strlen(rhs.m_pDescription); 
   m_pDescription = new char[length+1];
   strncpy(m_pDescription, rhs.m_pDescription, length);
   m_pDescription[length] = '\0';
  }
 }
 
 return *this;
 
}
/////////////////////////////////////////////////////////////////////////////////////
void CJunction::SetDescription(char* p_description)
{
 
  if(m_pDescription != NULL)
   delete m_pDescription;
   
  if( ! p_description)
   m_pDescription = p_description;
  
  else
  {
   WORD length = strlen(p_description); 
   m_pDescription = new char[length+1];
   strncpy(m_pDescription,p_description, length);
   m_pDescription[length] = '\0';
  }
 
}
/////////////////////////////////////////////////////////////////////////////////////
void   CJunction::SerializeXml(CXMLDOMElement*& pFatherNode) const 
{
 
 	CXMLDOMElement* pJunctionNode = pFatherNode->AddChildNode("JUNCTION");
 	
	pJunctionNode->AddChildNode("ID",          m_Id);
	pJunctionNode->AddChildNode("DESCRIPTION", m_pDescription);
};
//////////////////////////////////////////////////////////////////////////////////////
int    CJunction::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action) 
{

  
	int  nStatus = STATUS_OK;
	CXMLDOMElement *pChildNode = NULL;

	GET_VALIDATE_CHILD(pActionNode,"ID",&m_Id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"DESCRIPTION",m_pDescription,_0_TO_GENERAL_MES_LENGTH);

    return 0;
};
////////////////////////////////////////////////////////////////////////////////////// 
int    CJunction::DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action) 
{
    return 0;
};
/////////////////////////////////////////////////////////////////////////////////////
// **** junctions list
/////////////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////////////
CJunctionsList::CJunctionsList()
{ 
	m_numJunctions = 0;
	
	for(int i = 0; i < MAX_NUM_JUNCTIONS; i++ )
	  m_pJunctionsArray[i] = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////
CJunctionsList::CJunctionsList(const CJunctionsList& rhs):CSerializeObject(rhs)
{
 m_numJunctions = rhs.m_numJunctions;
 
 for(int i = 0; i < MAX_NUM_JUNCTIONS; i++ )
	  if(rhs.m_pJunctionsArray[i])
	    m_pJunctionsArray[i] = new CJunction( *(rhs.m_pJunctionsArray[i]) );
	  else
	    m_pJunctionsArray[i] = NULL;  

}
///////////////////////////////////////////////////////////////////////////////////////
CJunctionsList::~CJunctionsList()
{
 for(int i = 0; i < MAX_NUM_JUNCTIONS; i++ )
	  if(m_pJunctionsArray[i])
	    POBJDELETE( m_pJunctionsArray[i] );
}
///////////////////////////////////////////////////////////////////////////////////////
const CJunctionsList& CJunctionsList::operator=(const CJunctionsList& rhs)
{
	
 m_numJunctions = rhs.m_numJunctions;
 
 for(int i = 0; i < MAX_NUM_JUNCTIONS; i++ )
 {
    if(m_pJunctionsArray[i])
	   POBJDELETE(m_pJunctionsArray[i]);
	  
	if(rhs.m_pJunctionsArray[i])
	    m_pJunctionsArray[i] = new CJunction( *(rhs.m_pJunctionsArray[i]) );
	else
	    m_pJunctionsArray[i] = NULL; 
 }
	    
 return *this; 

}
///////////////////////////////////////////////////////////////////////////////////////
void   CJunctionsList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
 CXMLDOMElement* pListNode = pFatherNode->AddChildNode("JUNCTION_LIST");
 
 WORD i;
 for(i = 0; i < MAX_NUM_JUNCTIONS; i++ )
  if(m_pJunctionsArray[i])
    m_pJunctionsArray[i]->SerializeXml(pListNode); 

}
///////////////////////////////////////////////////////////////////////////////////////
int    CJunctionsList::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action)
{
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////
int    CJunctionsList::DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action)
{
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////
//* 
//* JUNCTION
//* PARAMS
//* CLASSES
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
CJunctionParam::CJunctionParam()
{
 m_Id = 0;
 m_rate = 0;
 
 m_fileName[0] ='\0';
}
///////////////////////////////////////////////////////////////////////////////////
CJunctionParam::CJunctionParam(const CJunctionParam& rhs):CSerializeObject(rhs)
{
 m_Id   = rhs.m_Id;
 m_rate = rhs.m_rate;
 
 //if( ! rhs.m_fileName)
  //m_fileName = rhs.m_fileName;
  
 //else
// {
  WORD length = strlen(rhs.m_fileName); 
  //m_fileName = new char[length+1];
  strncpy(m_fileName, rhs.m_fileName, length);
  m_fileName[length] = '\0';
 //}
 
}
////////////////////////////////////////////////////////////////////////////////////
CJunctionParam::~CJunctionParam()
{
  //if(m_fileName)
   //delete m_fileName;
}
////////////////////////////////////////////////////////////////////////////////////
const CJunctionParam& CJunctionParam::operator=(const CJunctionParam& rhs)
{
 m_Id = rhs.m_Id;
 m_rate = rhs.m_rate;
 
 //if( m_fileName == rhs.m_fileName 
                    //&& m_fileName != NULL)
  //PASSERT(1);
  
// else
 //{
  //if(m_fileName != NULL)
   //delete m_fileName;
   
  //if( ! rhs.m_fileName)
  // m_fileName = rhs.m_fileName;
  
  //else
  //{
   WORD length = strlen(rhs.m_fileName); 
   //m_fileName = new char[length+1];
   strncpy(m_fileName, rhs.m_fileName, length);
   m_fileName[length] = '\0';
 // }
/// }
 
 return *this;
 
}
/////////////////////////////////////////////////////////////////////////////////////
void CJunctionParam::SetFileName(char* p_fileName)
{
 
  //if(m_fileName != NULL)
   //delete m_fileName;
   
  //if( ! p_fileName)
  // m_fileName = p_fileName;
  
  //else
 // {
   //WORD length = strlen(p_fileName);
  // m_fileName = new char[length+1];
   strncpy(m_fileName,p_fileName, NEW_FILE_NAME_LEN - 1);
   m_fileName[NEW_FILE_NAME_LEN - 1] = '\0';
 // }
 
}

/////////////////////////////////////////////////////////////////////////////////////
void   CJunctionParam::SerializeXml(CXMLDOMElement*& pFatherNode) const 
{
 
 	CXMLDOMElement* pJunctionNode = pFatherNode->AddChildNode("JUNCTION_PARAM");
 	
	pJunctionNode->AddChildNode("ID",          m_Id);
	pJunctionNode->AddChildNode("RATE",        m_rate, TRANSFER_RATE_ENUM);
	pJunctionNode->AddChildNode("FILE_NAME",   m_fileName);
};
//////////////////////////////////////////////////////////////////////////////////////
int    CJunctionParam::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action) 
{
  
	int  nStatus = STATUS_OK;
	CXMLDOMElement *pChildNode = NULL;

	GET_VALIDATE_CHILD(pActionNode,"ID",&m_Id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"RATE",&m_rate,TRANSFER_RATE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"FILE_NAME",m_fileName,_1_TO_NEW_FILE_NAME_LENGTH);
	
	return nStatus;
	
};
////////////////////////////////////////////////////////////////////////////////////// 
int    CJunctionParam::DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action) 
{
   return 0; 
};
/////////////////////////////////////////////////////////////////////////////////////
// **** junction params list
/////////////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////////////
CJunctionParamList::CJunctionParamList()
{ 
	m_numJunctionParams = m_confId = m_partyId = m_size_limit = 0;
	
	for(int i = 0; i < MAX_NUM_JUNCTIONS; i++ )
	  m_pJunctionsParamArray[i] = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////
CJunctionParamList::CJunctionParamList(const CJunctionParamList& rhs):CSerializeObject(rhs)
{
 m_confId     = rhs.m_confId;
 m_partyId    = rhs.m_partyId;
 m_size_limit = rhs.m_size_limit;
 
 m_numJunctionParams = rhs.m_numJunctionParams;
 
 for(int i = 0; i < MAX_NUM_JUNCTIONS; i++ )
	  if(rhs.m_pJunctionsParamArray[i])
	    m_pJunctionsParamArray[i] = new CJunctionParam( *(rhs.m_pJunctionsParamArray[i]) );
	  else
	    m_pJunctionsParamArray[i] = NULL;  

}
///////////////////////////////////////////////////////////////////////////////////////
CJunctionParamList::~CJunctionParamList()
{
 for(int i = 0; i < MAX_NUM_JUNCTIONS; i++ )
	  if(m_pJunctionsParamArray[i])
	    POBJDELETE( m_pJunctionsParamArray[i] );
}
///////////////////////////////////////////////////////////////////////////////////////
const CJunctionParamList& CJunctionParamList::operator=(const CJunctionParamList& rhs)
{
	
 m_confId     = rhs.m_confId;
 m_partyId    = rhs.m_partyId;
 m_size_limit = rhs.m_size_limit;
 
 m_numJunctionParams = rhs.m_numJunctionParams;
 
 for(int i = 0; i < MAX_NUM_JUNCTIONS; i++ )
 {
  if(m_pJunctionsParamArray[i])
    POBJDELETE(m_pJunctionsParamArray[i]);
    
  if(rhs.m_pJunctionsParamArray[i])
	m_pJunctionsParamArray[i] = new CJunctionParam( *(rhs.m_pJunctionsParamArray[i]) );
  else
	m_pJunctionsParamArray[i] = NULL;
 } 
	    
 return *this; 

}
///////////////////////////////////////////////////////////////////////////////////////
void   CJunctionParamList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
 CXMLDOMElement* pListNode = pFatherNode->AddChildNode("JUNCTION_PARAM_LIST");
 
 pListNode->AddChildNode("ID",       m_confId);
 pListNode->AddChildNode("PARTY_ID", m_partyId);
 pListNode->AddChildNode("FILE_SIZE_LIMIT", m_size_limit);
 
 for(int i = 0; i < MAX_NUM_JUNCTIONS; i++ )
  if(m_pJunctionsParamArray[i])
    m_pJunctionsParamArray[i]->SerializeXml(pListNode); 

}
///////////////////////////////////////////////////////////////////////////////////////
int    CJunctionParamList::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action)
{

 int  nStatus = STATUS_OK;
 CXMLDOMElement *pChildNode = NULL, *pListNode = NULL;

 GET_VALIDATE_CHILD(pActionNode,"ID",&m_confId,_0_TO_DWORD);
 GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_partyId,_0_TO_DWORD);
 GET_VALIDATE_CHILD(pActionNode,"FILE_SIZE_LIMIT",&m_size_limit,_0_TO_DWORD);

 GET_FIRST_CHILD_NODE(pActionNode,"JUNCTION_PARAM_LIST",pListNode);
   GET_FIRST_CHILD_NODE(pListNode,"JUNCTION_PARAM",pChildNode);
 
 int i = 0;
 
 while( pChildNode  &&  i < MAX_NUM_JUNCTIONS )
 {

		m_pJunctionsParamArray[i] = new CJunctionParam;
		nStatus = m_pJunctionsParamArray[i]->DeSerializeXml(pChildNode,pszError,action);

		if( nStatus != STATUS_OK )
		{
			POBJDELETE(m_pJunctionsParamArray[i]);
			return nStatus;
		}
		
		i++;
		GET_NEXT_CHILD_NODE(pListNode,"JUNCTION_PARAM",pChildNode);

 }
 
 return nStatus;
}
///////////////////////////////////////////////////////////////////////////////////////
int    CJunctionParamList::DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action)
{
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CJunctionsList::InitJunctionsList()
{
 int i = 0;
 
 /*-------------------------------------------------------------------------------------*/
 /* ART Light Port Junctions                                                            */
 /*-------------------------------------------------------------------------------------*/

 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_LAN_INCOMING_AUDIO, "JUNCTION_LAN_INCOMING_AUDIO"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_AC_COMMAND, "JUNCTION_INCOMING_AC_COMMAND"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_COMPRESSED_AUDIO, "JUNCTION_INCOMING_COMPRESSED_AUDIO"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO1, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO1"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO2, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO2"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO3, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO3"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO4, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO4"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO5, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO5"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO6, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO6"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO7, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO7"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO8, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO8"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO9, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO9"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO10, "JUNCTION_INCOMING_UNCOMPRESSED_AUDIO10"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_PUBLIC_AUDIO_IVR, "JUNCTION_INCOMING_PUBLIC_AUDIO_IVR"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_PRIVATE_AUDIO_IVR, "JUNCTION_INCOMING_PRIVATE_AUDIO_IVR"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_PUBLIC_MUSIC, "JUNCTION_INCOMING_PUBLIC_MUSIC"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_PRIVATE_MUSIC, "JUNCTION_INCOMING_PRIVATE_MUSIC"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_MRMP_AUDIO, "JUNCTION_INCOMING_MRMP_AUDIO"); i++;

 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_LAN_OUTGOING_AUDIO, "JUNCTION_LAN_OUTGOING_AUDIO"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO1, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO1"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO2, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO2"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO3, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO3"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO4, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO4"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO5, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO5"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO6, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO6"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO7, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO7"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO8, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO8"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO9, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO9"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO10, "JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO10"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ROLL_CALL, "JUNCTION_OUTGOING_ROLL_CALL"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_COMPRESSED_AUDIO, "JUNCTION_OUTGOING_COMPRESSED_AUDIO"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ART_OUTGOING_INTERNAL_RECORDING1, "JUNCTION_ART_OUTGOING_INTERNAL_RECORDING1"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ART_OUTGOING_INTERNAL_RECORDING2, "JUNCTION_ART_OUTGOING_INTERNAL_RECORDING2"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_MRMP_AUDIO, "JUNCTION_OUTGOING_MRMP_AUDIO"); i++;

 
 /*-------------------------------------------------------------------------------------*/
 /* ART Port Junctions                                                                  */
 /*-------------------------------------------------------------------------------------*/
 
 
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_LAN_INCOMING_VIDEO, "JUNCTION_LAN_INCOMING_VIDEO"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_LAN_INCOMING_CONTENT, "JUNCTION_LAN_INCOMING_CONTENT"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_LAN_INCOMING_FECC, "JUNCTION_LAN_INCOMING_FECC"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_INCOMING_VIDEO1, "JUNCTION_FABRIC_INCOMING_VIDEO1"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_INCOMING_VIDEO2, "JUNCTION_FABRIC_INCOMING_VIDEO2"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_INCOMING_VIDEO3, "JUNCTION_FABRIC_INCOMING_VIDEO3"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_INCOMING_CONTENT, "JUNCTION_FABRIC_INCOMING_CONTENT"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_INCOMING_FECC, "JUNCTION_FABRIC_INCOMING_FECC"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_VC_VSW_COMMAND, "JUNCTION_INCOMING_VC_VSW_COMMAND"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_PRIVATE_VIDEO_IVR, "JUNCTION_INCOMING_PRIVATE_VIDEO_IVR"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_MRMP_INCOMING_FECC, "JUNCTION_MRMP_INCOMING_FECC"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_MRMP_INCOMING_VIDEO, "JUNCTION_MRMP_INCOMING_VIDEO"); i++;
 
 
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_LAN_OUTGOING_VIDEO, "JUNCTION_LAN_OUTGOING_VIDEO"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_LAN_OUTGOING_CONTENT, "JUNCTION_LAN_OUTGOING_CONTENT"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_LAN_OUTGOING_FECC, "JUNCTION_LAN_OUTGOING_FECC"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_OUTGOING_VIDEO1, "JUNCTION_FABRIC_OUTGOING_VIDEO1"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_OUTGOING_VIDEO2, "JUNCTION_FABRIC_OUTGOING_VIDEO2"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_OUTGOING_VIDEO3, "JUNCTION_FABRIC_OUTGOING_VIDEO3"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_OUTGOING_CONTENT, "JUNCTION_FABRIC_OUTGOING_CONTENT"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_FABRIC_OUTGOING_FECC, "JUNCTION_FABRIC_OUTGOING_FECC"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_MRMP_OUTGOING_FECC, "JUNCTION_MRMP_OUTGOING_FECC"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_MRMP_OUTGOING_VIDEO, "JUNCTION_MRMP_OUTGOING_VIDEO"); i++;
 
 /*-------------------------------------------------------------------------------------*/
 /* Video Decoder Port Junctions                                                        */
 /*-------------------------------------------------------------------------------------*/
 
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_COMPRESSED_VIDEO, "JUNCTION_INCOMING_COMPRESSED_VIDEO"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_4CIF, "JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_4CIF"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_CIF, "JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_CIF"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_QCIF, "JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_QCIF"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_VIDEO_DECODER_OUTGOING_INTERNAL_RECORDING1, "JUNCTION_VIDEO_DECODER_OUTGOING_INTERNAL_RECORDING1"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_VIDEO_DECODER_OUTGOING_INTERNAL_RECORDING2, "JUNCTION_VIDEO_DECODER_OUTGOING_INTERNAL_RECORDING2"); i++;
 
 /*-------------------------------------------------------------------------------------*/
/* Video Encoder Port Junctions                                                        */
/*-------------------------------------------------------------------------------------*/

 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_1, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_1"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_2, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_2"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_3, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_3"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_4, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_4"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_5, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_5"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_6, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_6"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_7, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_7"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_8, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_8"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_9, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_9"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_11"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_12"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_13"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_14"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_15"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10, "JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_16"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_PUBLIC_GRAPHIC_OBJECTS, "JUNCTION_INCOMING_PUBLIC_GRAPHIC_OBJECTS"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_PRIVATE_GRAPHIC_OBJECTS, "JUNCTION_INCOMING_PRIVATE_GRAPHIC_OBJECTS"); i++;
 
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_COMPRESSED_VIDEO, "JUNCTION_OUTGOING_COMPRESSED_VIDEO"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_VIDEO_ENCODER_OUTGOING_INTERNAL_RECORDING1, "JUNCTION_VIDEO_ENCODER_OUTGOING_INTERNAL_RECORDING1"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_VIDEO_ENCODER_OUTGOING_INTERNAL_RECORDING2, "JUNCTION_VIDEO_ENCODER_OUTGOING_INTERNAL_RECORDING2"); i++;
 
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL01, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL01"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL02, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL02"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL03, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL03"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL04, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL04"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL05, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL05"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL06, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL06"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL07, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL07"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL08, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL08"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL09, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL09"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL10, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL10"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL11, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL11"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL12, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL12"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL13, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL13"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL14, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL14"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL15, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL15"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL16, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL16"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL17, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL17"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL18, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL18"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL19, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL19"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL20, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL20"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL21, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL21"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL22, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL22"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL23, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL23"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL24, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL24"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL25, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL25"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL26, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL26"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL27, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL27"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL28, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL28"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL29, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL29"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL30, "E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL30"); i++;
 
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL01, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL01"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL02, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL02"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL03, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL03"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL04, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL04"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL05, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL05"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL06, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL06"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL07, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL07"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL08, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL08"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL09, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL09"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL10, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL10"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL11, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL11"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL12, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL12"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL13, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL13"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL14, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL14"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL15, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL15"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL16, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL16"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL17, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL17"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL18, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL18"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL19, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL19"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL20, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL20"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL21, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL21"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL22, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL22"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL23, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL23"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL24, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL24"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL25, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL25"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL26, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL26"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL27, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL27"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL28, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL28"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL29, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL29"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_INCOMING_ISDN_CHANNEL30, "E_CM_JUNCTION_INCOMING_ISDN_CHANNEL30"); i++;
 
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL01, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL01"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL02, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL02"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL03, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL03"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL04, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL04"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL05, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL05"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL06, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL06"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL07, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL07"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL08, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL08"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL09, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL09"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL10, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL10"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL11, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL11"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL12, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL12"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL13, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL13"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL14, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL14"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL15, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL15"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL16, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL16"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL17, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL17"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL18, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL18"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL19, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL19"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL20, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL20"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL21, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL21"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL22, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL22"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL23, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL23"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL24, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL24"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL25, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL25"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL26, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL26"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL27, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL27"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL28, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL28"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL29, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL29"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL30, "E_CM_JUNCTION_ISDN_FROM_FABRIC_CHANNEL30"); i++;
 
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL01, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL01"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL02, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL02"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL03, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL03"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL04, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL04"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL05, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL05"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL06, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL06"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL07, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL07"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL08, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL08"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL09, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL09"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL10, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL10"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL11, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL11"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL12, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL12"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL13, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL13"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL14, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL14"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL15, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL15"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL16, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL16"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL17, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL17"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL18, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL18"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL19, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL19"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL20, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL20"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL21, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL21"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL22, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL22"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL23, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL23"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL24, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL24"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL25, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL25"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL26, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL26"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL27, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL27"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL28, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL28"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL29, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL29"); i++;
 m_pJunctionsArray[i] = new CJunction(E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL30, "E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL30"); i++;
 PTRACE2INT(eLevelInfoNormal, " CJunctionsList::InitJunctionsList() , num. junctions = " , i);

}
//
//-------------------------------------------------------------------------------------
// ART Light Port Junctions
//-------------------------------------------------------------------------------------
 
 //...
 //...
 // initialize all junctions
 
/*-------------------------------------------------------------------------------------*/
/* ART Light Port Junctions                                                            */
/*-------------------------------------------------------------------------------------*/

	/*
	E_CM_JUNCTION_LAN_INCOMING_AUDIO					= 51,     	//size = 8K, Lobby
	E_CM_JUNCTION_INCOMING_AC_COMMAND,								//size = 4K, Audio Rx
	E_CM_JUNCTION_INCOMING_COMPRESSED_AUDIO,						//size = 4K, ISDN interface
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO1,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO2,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO3,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO4,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO5,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO6,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO7,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO8,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO9,						//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_AUDIO10,					//size = 8K, Audio Tx
	E_CM_JUNCTION_INCOMING_PUBLIC_AUDIO_IVR,						//size = 4K, Audio Tx
	E_CM_JUNCTION_INCOMING_PRIVATE_AUDIO_IVR,						//size = 4K, Audio Tx
	E_CM_JUNCTION_INCOMING_PUBLIC_MUSIC,							//size = 4K, Audio Tx
	E_CM_JUNCTION_INCOMING_PRIVATE_MUSIC,							//size = 4K, Audio Tx
	
	E_CM_JUNCTION_LAN_OUTGOING_AUDIO					= 71,		//size = 4K, Audio Tx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO1,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO2,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO3,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO4,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO5,						//size = 8K, Audio Rtypex
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO6,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO7,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO8,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO9,						//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_AUDIO10,					//size = 8K, Audio Rx
	E_CM_JUNCTION_OUTGOING_ROLL_CALL,								//size = 4K, Audio Rx
	E_CM_JUNCTION_OUTGOING_COMPRESSED_AUDIO,						//size = 4K, Audio Tx
	E_CM_JUNCTION_ART_OUTGOING_INTERNAL_RECORDING1,					//size = 4K, Audio Rx
	E_CM_JUNCTION_ART_OUTGOING_INTERNAL_RECORDING2,					//size = 4K, Audio Tx

*/
/*-------------------------------------------------------------------------------------*/
/* ART Port Junctions                                                                  */
/*-------------------------------------------------------------------------------------*/
/*

	E_CM_JUNCTION_LAN_INCOMING_VIDEO					= 91,		//size = 20K, Lobby
	E_CM_JUNCTION_LAN_INCOMING_CONTENT,								//size = 20K, Lobby
	E_CM_JUNCTION_LAN_INCOMING_FECC,								//size = 8K,  Lobby
	E_CM_JUNCTION_FABRIC_INCOMING_VIDEO1,							//size = 16K, Video Tx
	E_CM_JUNCTION_FABRIC_INCOMING_VIDEO2,							//size = 16K, Video Tx
	E_CM_JUNCTION_FABRIC_INCOMING_VIDEO3,							//size = 16K, Video Tx
	E_CM_JUNCTION_FABRIC_INCOMING_CONTENT,							//size = 20K, Lobby
	E_CM_JUNCTION_FABRIC_INCOMING_FECC,								//size = 8K , Lobby
	E_CM_JUNCTION_INCOMING_VC_VSW_COMMAND,							//size = 4K , Video Rx
	E_CM_JUNCTION_INCOMING_PRIVATE_VIDEO_IVR,						//size = 16K, Video Tx

	E_CM_JUNCTION_LAN_OUTGOING_VIDEO					= 111,		//size = 16K, Video Tx
	E_CM_JUNCTION_LAN_OUTGOING_CONTENT,								//size = 16K, Content Tx
	E_CM_JUNCTION_LAN_OUTGOING_FECC,								//size = 4K , FECC Tx
	E_CM_JUNCTION_FABRIC_OUTGOING_VIDEO1,							//size = 16K, Video Rx
	E_CM_JUNCTION_FABRIC_OUTGOING_VIDEO2,							//size = 16K, Video Rx
	E_CM_JUNCTION_FABRIC_OUTGOING_VIDEO3,							//size = 16K, Video Rx
	E_CM_JUNCTION_FABRIC_OUTGOING_CONTENT,							//size = 20K, Content Rx
	E_CM_JUNCTION_FABRIC_OUTGOING_FECC,								//size = 8K , FECC RX

*/
/*-------------------------------------------------------------------------------------*/
/* Video Decoder Port Junctions                                                        */
/*-------------------------------------------------------------------------------------*/
/*
	E_CM_JUNCTION_INCOMING_COMPRESSED_VIDEO				= 131,		//size = 32K, Video Decoder
	
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_4CIF		= 151,		//size = 1280K, Video Decoder
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_CIF,					//size = 320K, Video Decoder
	E_CM_JUNCTION_OUTGOING_UNCOMPRESSED_VIDEO_QCIF,					//size = 80K, Video Decoder
	E_CM_JUNCTION_VIDEO_DECODER_OUTGOING_INTERNAL_RECORDING1,		//size = 4K, Video Decoder
	E_CM_JUNCTION_VIDEO_DECODER_OUTGOING_INTERNAL_RECORDING2,		//size = 4K, Video Decoder

*/

/*-------------------------------------------------------------------------------------*/
/* Video Encoder Port Junctions                                                        */
/*-------------------------------------------------------------------------------------*/
/* Incoming Uncompressed Video streams are depended on the layout used. According to   */ 
/* the current design of the Video Encoder port, there might be up to 16 differnt      */
/* Uncompressed Video streams. Each Uncompressed Video stream will have the same       */
/* junction type                                                                       */ 
/*-------------------------------------------------------------------------------------*/
/*

	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_1			= 171,		//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_2,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_3,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_4,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_5,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_6,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_7,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_8,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_9,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_10,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_PUBLIC_GRAPHIC_OBJECTS,					//size = 80K, Video Encoder
	E_CM_JUNCTION_INCOMING_PRIVATE_GRAPHIC_OBJECTS,					//size = 80K, Video Encoder

	E_CM_JUNCTION_OUTGOING_COMPRESSED_VIDEO				= 191,		//size = 16K, Video Encoder
	E_CM_JUNCTION_VIDEO_ENCODER_OUTGOING_INTERNAL_RECORDING1,		//size = 4K, Video Encoder
	E_CM_JUNCTION_VIDEO_ENCODER_OUTGOING_INTERNAL_RECORDING2,		//size = 4K, Video Encoder
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
 /*
 *<xsd:include schemaLocation="common_trans_obj.xsd"/>
<xsd:include schemaLocation="common_obj.xsd"/>


	<xsd:element name="JUNCTION_LIST" type="JunctionListContent"/>
	<xsd:element name="JUNCTION" type="JunctionContent"/>
	<xsd:element name="JUNCTION_PARAM_LIST" type="JunctionParamListContent"/>
	<xsd:element name="JUNCTION_PARAM" type="JunctionParamContent"/>


	<xsd:complexType name="JunctionListContent">
		<xsd:sequence>
			<xsd:element ref="JUNCTION" minOccurs="0" maxOccurs="unbounded"/>
			<xsd:any processContents="skip" minOccurs="0" maxOccurs="unbounded" namespace="##other"/>
		</xsd:sequence>
	</xsd:complexType>
	
	<xsd:complexType name="JunctionContent">
		<xsd:sequence>
			<xsd:element ref="ID"/>
			<xsd:element ref="DESCRIPTION"/>
			<xsd:any processContents="skip" minOccurs="0" maxOccurs="unbounded" namespace="##other"/>
		</xsd:sequence>
	</xsd:complexType>
	
	<xsd:complexType name="JunctionParamListContent">
		<xsd:sequence>
			<xsd:element ref="JUNCTION_PARAM" minOccurs="0" maxOccurs="unbounded"/>
			<xsd:any processContents="skip" minOccurs="0" maxOccurs="unbounded" namespace="##other"/>
		</xsd:sequence>
	</xsd:complexType>
	
	<xsd:complexType name="JunctionParamContent">
		<xsd:sequence>
			<xsd:element ref="ID"/>
			<xsd:element ref="RATE"/>
			<xsd:element ref="FILE_NAME"/>
			<xsd:any processContents="skip" minOccurs="0" maxOccurs="unbounded" namespace="##other"/>
		</xsd:sequence>
	</xsd:complexType>


</xsd:schema> 
*/
