/*
 * TcpDumpStatusGet.cpp
 *
 *  Created on: Mar 29, 2011
 *      Author: racohen
 */



#include "TcpDumpStatusGet.h"
#include "StatusesGeneral.h"
#include "UtilityProcess.h"
#include "TcpDumpEntity.h"
//#include "IpService.h"
#include <stdlib.h>
#include "OsFileIF.h"



//////////////////////////////////////////////////////////////////////////////
CTcpDumpStatusGet::CTcpDumpStatusGet()
{
	m_pProcess = (CUtilityProcess*)CUtilityProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CTcpDumpStatusGet::CTcpDumpStatusGet( const CTcpDumpStatusGet &other )
:CSerializeObject(other)
{
	m_pProcess = (CUtilityProcess*)CUtilityProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
}

/////////////////////////////////////////////////////////////////////////////
CTcpDumpStatusGet& CTcpDumpStatusGet::operator = (const CTcpDumpStatusGet &other)
{
	if(this == &other){
		return *this;
	}

	m_pProcess = (CUtilityProcess*)CUtilityProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CTcpDumpStatusGet::~CTcpDumpStatusGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CTcpDumpStatusGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CTcpDumpStatus* pTcpDumpStatusGet = m_pProcess->GetTcpDumpStatus();

	if (pTcpDumpStatusGet)
	{
		if (m_pProcess->GetIsTcpDumpRunning())
		{
			CStructTm curTime , elapseTime;
			SystemGetTime(curTime);
			//elapseTime.GetTimeDelta(curTime);
			curTime = curTime.GetTimeDelta(m_pProcess->GetTimeElapsed());

			pTcpDumpStatusGet->SetTimeElapsed(curTime);
		}

		// VNGR-20421 & VNGR-22106 : get size of dir MCU_OUTPUT_DIR/tcp_dump
		BOOL isTcpdumpFolderEmpty = (GetDirFilesNum((MCU_OUTPUT_DIR+"/tcp_dump/emb/").c_str()) ||
			GetDirFilesNum((MCU_OUTPUT_DIR+"/tcp_dump/mcms/").c_str()) || GetDirFilesNum((MCU_OUTPUT_DIR+"/tcp_dump/media-signaling/").c_str())) ? FALSE : TRUE;

		if (isTcpdumpFolderEmpty)
		{
			pTcpDumpStatusGet->SetStorageInUsed(0);
		}
		else
		{
			std::string ans;
			std::string cmd;

			cmd = "du -s "+MCU_OUTPUT_DIR+"/tcp_dump/ | awk '{ print $1 }'";

			STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

			if (stat == STATUS_OK)
			{
				pTcpDumpStatusGet->SetStorageInUsed(atoi(ans.c_str()));
			}
		}

		pTcpDumpStatusGet->SerializeXml(pActionsNode);

		if (pTcpDumpStatusGet->GetTcpDumpState()== e_TcpDumpState_Failed)
			pTcpDumpStatusGet->SetTcpDumpState(e_TcpDumpState_Idle);
	}
}

/////////////////////////////////////////////////////////////////////////////
int CTcpDumpStatusGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
