

#include "TcpDumpEntityListGet.h"
#include "StatusesGeneral.h"
#include "UtilityProcess.h"
#include "TcpDumpEntity.h"
//#include "IpService.h"
#include <stdlib.h>
#include "OsFileIF.h"



//////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityListGet::CTcpDumpEntityListGet()
{
	m_pProcess = (CUtilityProcess*)CUtilityProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityListGet::CTcpDumpEntityListGet( const CTcpDumpEntityListGet &other )
:CSerializeObject(other)
{
	m_pProcess = (CUtilityProcess*)CUtilityProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
}

/////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityListGet& CTcpDumpEntityListGet::operator = (const CTcpDumpEntityListGet &other)
{
	if(this == &other){
		return *this;
	}

	m_pProcess = (CUtilityProcess*)CUtilityProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityListGet::~CTcpDumpEntityListGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CTcpDumpEntityListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CTcpDumpEntityList* pTcpDumpEntityListGet = m_pProcess->GetTcpDumpEntityList();

	if (pTcpDumpEntityListGet)
	{
		// VNGR-20421 & VNGR-22106 : get size of dir MCU_OUTPUT_DIR/tcp_dump
		BOOL isTcpdumpFolderEmpty = (GetDirFilesNum((MCU_OUTPUT_DIR+"/tcp_dump/emb/").c_str()) ||
			GetDirFilesNum((MCU_OUTPUT_DIR+"/tcp_dump/mcms/").c_str()) || GetDirFilesNum((MCU_OUTPUT_DIR+"/tcp_dump/media-signaling/").c_str())) ? FALSE : TRUE;

		if (isTcpdumpFolderEmpty)
		{
			pTcpDumpEntityListGet->SetStorageInUsed(0);
		}
		else
		{
			std::string ans;
			std::string cmd;
			
			cmd = "du -s "+MCU_OUTPUT_DIR+"/tcp_dump/ | awk '{ print $1 }'";

			STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

			if (stat == STATUS_OK)
			{
				pTcpDumpEntityListGet->SetStorageInUsed(atoi(ans.c_str()));
			}
		}

		pTcpDumpEntityListGet->SerializeXml(pActionsNode);
	}
	
}

/////////////////////////////////////////////////////////////////////////////
int CTcpDumpEntityListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{

	return STATUS_OK;
}

