#include "SystemState.h"
#include "Trace.h"


//////////////////////////////////////////////////////////////////////
CSystemState::CSystemState()
	:CShardMemory(SHARED_MEMORY_NAME,
				  SHARED_MEMORY_PERMISITIONS,
				  sizeof(eMcuState) * SHARED_MEMORY_MAX_NUM)
{		
	m_pState = NULL;
	if (m_status == STATUS_OK)
	{	
		m_pState = (eMcuState*)m_pView;
		if (m_first)
		{
			*m_pState = eMcuState_Invalid;
		}
	}
	FPASSERTMSG(m_status,"Shared memory allocation failed");
}

//////////////////////////////////////////////////////////////////////
CSystemState::~CSystemState()
{}

//////////////////////////////////////////////////////////////////////
eMcuState CSystemState::Get() const 
{
	return *m_pState;
}

//////////////////////////////////////////////////////////////////////
void CSystemState::Set(eMcuState state) 
{
	*m_pState = state;
}
