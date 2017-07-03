#include "TestServicePrefix.h"
#include "StatusesGeneral.h"
#include <iostream>
#include "H323Alias.h"
#include "psosxml.h"
#include <string>

CPPUNIT_TEST_SUITE_REGISTRATION(CTestServicePrefixStr);


void  CTestServicePrefixStr::setUp()
{
  m_numServicePrefixStr = 0;
  for (int i =0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
    {
      m_pServicePrefixStr[i] = NULL;
    }  
}

void CTestServicePrefixStr::tearDown()
{
  m_numServicePrefixStr = 0;
  for (int i=0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
    {
      POBJDELETE(m_pServicePrefixStr[i]);
    }
}


void CTestServicePrefixStr::TestAdd()
{
  ALLOCBUFFER(serviceName,NET_SERVICE_PROVIDER_NAME_LEN);
  ALLOCBUFFER(prefixName,ALIAS_NAME_LEN);
 
  for (int i=0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
    {
      memset(prefixName,'\0',ALIAS_NAME_LEN);
      memset(serviceName,'\0',NET_SERVICE_PROVIDER_NAME_LEN);
      sprintf(prefixName,"_(%d)",i+1);
      sprintf(serviceName,"_(%d)",i+1);
      
      m_pServicePrefixStr[i] = new CServicePrefixStr();
      m_pServicePrefixStr[i]->SetNetServiceName(serviceName);
      m_pServicePrefixStr[i]->SetDialInPrefix(prefixName);
    }

  for (int i=0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
    {
      memset(prefixName,'\0',ALIAS_NAME_LEN);
      memset(serviceName,'\0',NET_SERVICE_PROVIDER_NAME_LEN);
      sprintf(prefixName,"_(%d)",i+1);
      sprintf(serviceName,"_(%d)",i+1);
      
      CPPUNIT_ASSERT(strcmp(m_pServicePrefixStr[i]->GetNetServiceName(),serviceName) == 0);
      
      CPPUNIT_ASSERT(strcmp((m_pServicePrefixStr[i]->GetDialInPrefix())->m_aliasName,prefixName) == 0);
    }

  DEALLOCBUFFER(serviceName);
  DEALLOCBUFFER(prefixName);
}

void CTestServicePrefixStr::TestSerialixeXml()
{
  CServicePrefixStr* pServicePrefixStrDes[MAX_NET_SERV_PROVIDERS_IN_LIST];
  WORD numServicePrefixStrDes=0;
  
  for (int i =0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
    {
      pServicePrefixStrDes[i] = NULL;
    }
  
  ALLOCBUFFER(serviceName,NET_SERVICE_PROVIDER_NAME_LEN);
  ALLOCBUFFER(prefixName,ALIAS_NAME_LEN);
  std::string serviceStr,prefixStr;
  
  CXMLDOMElement *pResNode = new CXMLDOMElement;
  CXMLDOMElement * pTempNode=0;
  CXMLDOMElement * pChildNode = 0;
  CXMLDOMElement * pServiceNode= 0;
  STATUS nStatus=STATUS_OK;

  //Add Presfixes to the array
  for (int i=0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
    {
      memset(prefixName,'\0',ALIAS_NAME_LEN);
      memset(serviceName,'\0',NET_SERVICE_PROVIDER_NAME_LEN);
      sprintf(prefixName,"_(%d)",i+1);
      sprintf(serviceName,"_(%d)",i+1);
      serviceStr="DefaultService";
      prefixStr="1616";
      serviceStr+=serviceName;
      prefixStr+=prefixName;
      
      m_pServicePrefixStr[i] = new CServicePrefixStr();
      m_pServicePrefixStr[i]->SetNetServiceName(serviceStr.c_str());
      m_pServicePrefixStr[i]->SetDialInPrefix(prefixStr.c_str());
      m_numServicePrefixStr++;
    }

  //SerializeXml all of the array
  if(m_numServicePrefixStr>0)
    {
      pTempNode=pResNode->AddChildNode("DIAL_IN_H323_SRV_PREFIX_LIST");
      for (int i=0;i<(int)m_numServicePrefixStr;i++)
	{
	  m_pServicePrefixStr[i]->SerializeXml(pTempNode);
	}
    }

  //DeserializeXml to another array
  char pszError[100];
  
  GET_CHILD_NODE(pResNode, "DIAL_IN_H323_SRV_PREFIX_LIST", pChildNode);
  
  if(pChildNode)
    {      
      GET_FIRST_CHILD_NODE(pChildNode, "DIAL_IN_H323_SRV_PREFIX", pServiceNode);
      while(pServiceNode)
	{
	  CServicePrefixStr servicePrefixStr;
	  nStatus=servicePrefixStr.DeSerializeXml(pServiceNode,pszError);
	  CPPUNIT_ASSERT(nStatus ==STATUS_OK);

	  //Adding the new prefix
	  pServicePrefixStrDes[numServicePrefixStrDes]=new CServicePrefixStr(servicePrefixStr);;
	  ++numServicePrefixStrDes;

	  GET_NEXT_CHILD_NODE(pChildNode, "DIAL_IN_H323_SRV_PREFIX", pServiceNode);
	}
    }
  for (int i=0;i<MAX_NET_SERV_PROVIDERS_IN_LIST;i++)
    {
      memset(prefixName,'\0',ALIAS_NAME_LEN);
      memset(serviceName,'\0',NET_SERVICE_PROVIDER_NAME_LEN);
      sprintf(prefixName,"_(%d)",i+1);
      sprintf(serviceName,"_(%d)",i+1);
      serviceStr="DefaultService";
      prefixStr="1616";
      serviceStr+=serviceName;
      prefixStr+=prefixName;
      
      CPPUNIT_ASSERT(strcmp(pServicePrefixStrDes[i]->GetNetServiceName(),serviceStr.c_str()) == 0);
      CPPUNIT_ASSERT(strcmp((pServicePrefixStrDes[i]->GetDialInPrefix())->m_aliasName,prefixStr.c_str()) == 0);
    }
  
  DEALLOCBUFFER(serviceName);
  DEALLOCBUFFER(prefixName);
}

