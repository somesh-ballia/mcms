#include "Log4cxxConfiguration.h"
#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ModuleContent.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "LoggerProcess.h"


CLog4cxxConfiguration::CLog4cxxConfiguration()
{
	m_msgAvgRate = 0;

	InitAppenders(MCMS_MODULE_NAME,CLoggerProcess::eMcmsDaemon,CLoggerProcess::eUtility);
	InitAppenders(EMA_MODULE_NAME,CLoggerProcess::eEMA,CLoggerProcess::eEMA);
	InitAppenders(MEDIACARD_MODULE_NAME,CLoggerProcess::eOnlyMplProcess,CLoggerProcess::eMediaCardNA);
	m_isCSLogStarted = FALSE;
	m_maxLogSize = 8;
}

void  CLog4cxxConfiguration::InitAppenders(string moduleName,unsigned int processStartInd,unsigned int processEndInd)
{
	CModuleContent*pMoudle = InitModule(moduleName,processStartInd,processEndInd);
	GetUDPAppender()->AddModule(pMoudle);
	pMoudle = InitModule(moduleName,processStartInd,processEndInd);
	GetSysLogAppender()->AddModule(pMoudle);
	pMoudle = InitModule(moduleName,processStartInd,processEndInd);
	GetLocalLogAppender()->AddModule(pMoudle);
}

CModuleContent* CLog4cxxConfiguration::InitModule(string moduleName,unsigned int processStartInd,unsigned int processEndInd)
{
	CModuleContent*	pModule = new CModuleContent();
	pModule->SetLModuleName((char*)moduleName.c_str());

	for (unsigned int i = processStartInd ; i <= processEndInd; i++)
	{
		CProcessItemData *processItem = new CProcessItemData(1,i);
		pModule->GetProcessList().AddProcessItem(processItem);
	}

	return pModule;
}

CLog4cxxConfiguration::CLog4cxxConfiguration(const CLog4cxxConfiguration &other)
	 :CSerializeObject(other)
{

	m_msgAvgRate = other.m_msgAvgRate;
	m_udpAppender = other.m_udpAppender;
	m_sysLogAppender = other.m_sysLogAppender;
	m_localLogAppender= other.m_localLogAppender;
	m_isCSLogStarted = other.m_isCSLogStarted;
	m_maxLogSize = other.m_maxLogSize;
}

CLog4cxxConfiguration::~CLog4cxxConfiguration()
{

}

void CLog4cxxConfiguration::CopyValue(CLog4cxxConfiguration* pOther)
{
	if (pOther == NULL || (this == pOther))
		return;

	m_udpAppender.CopyValue(pOther->GetUDPAppender());
	m_udpAppender.SetDuration(pOther->GetUDPAppender()->GetDuration());
	m_sysLogAppender.CopyValue(pOther->GetSysLogAppender());
	m_localLogAppender.CopyValue(pOther->GetLocalLogAppender());
	m_maxLogSize=pOther->GetMaxLogSize();
}

CLog4cxxConfiguration& CLog4cxxConfiguration::operator = (const CLog4cxxConfiguration& other)
{
	if (this == &other)
		return *this;

	m_msgAvgRate = other.m_msgAvgRate;
	m_udpAppender = other.m_udpAppender;
	m_sysLogAppender = other.m_sysLogAppender;
	m_localLogAppender= other.m_localLogAppender;
	m_maxLogSize=other.m_maxLogSize;
	return *this;
}

void   CLog4cxxConfiguration::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* pAppenderNode = pParentNode->AddChildNode("LOG_CONFIG_DATA");

	pAppenderNode->AddChildNode("MSG_AVG_RATE",(DWORD)m_msgAvgRate,_0_TO_WORD);

	m_localLogAppender.SerializeXml(pAppenderNode);
	m_udpAppender.SerializeXml(pAppenderNode);
	m_sysLogAppender.SerializeXml(pAppenderNode);
	pAppenderNode->AddChildNode("IS_CS_LOG_STARTED",m_isCSLogStarted,_BOOL);
	pAppenderNode->AddChildNode("MAX_LOG_SIZE",m_maxLogSize,_2_TO_MAX_LOG_SIZE);
}

void CLog4cxxConfiguration::SetMaxLogSize(const WORD maxLogSize)
{
	m_maxLogSize = maxLogSize;
}

 int  CLog4cxxConfiguration::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
 {
 	int nStatus = STATUS_OK;

	CXMLDOMElement *pAppenderNode=NULL, *pTempNode=NULL, *pTempSonNode=NULL;
	GET_CHILD_NODE(pActionNode, "LOG_CONFIG_DATA", pAppenderNode);


	//GET_VALIDATE_CHILD(pAppenderNode, "MSG_AVG_RATE", &m_msgAvgRate,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pAppenderNode, "MAX_LOG_SIZE", &m_maxLogSize,_2_TO_MAX_LOG_SIZE);
	m_localLogAppender.DeSerializeXml(pAppenderNode,pszError,action);
	m_udpAppender.DeSerializeXml(pAppenderNode,pszError,action);
	m_sysLogAppender.DeSerializeXml(pAppenderNode,pszError,action);

    //Ugly pacth to disable the SysLogAppender till the core dump will be fixed
	//VNGFE-6838 - System had 9 core dumps due to lack of external syslog support
	m_sysLogAppender.SetEnabled(0);

	return STATUS_OK;
 }


void	CLog4cxxConfiguration::SetMsgAvgRate(ULONGLONG rate)
{
	m_msgAvgRate = rate;
}

ULONGLONG	CLog4cxxConfiguration::GetMsgAvgRate()
{
	return m_msgAvgRate;
}


void	CLog4cxxConfiguration::	SetIsCSLogStarted(BOOL b)
{
	m_isCSLogStarted = b;
}

BOOL	CLog4cxxConfiguration::	GetIsCSLogStarted()
{
	return m_isCSLogStarted;
}

