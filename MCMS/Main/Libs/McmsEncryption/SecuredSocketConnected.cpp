

#include "SecuredSocketConnected.h"
#include "TraceStream.h"
#include <errno.h>
#include "SystemFunctions.h"
#include "Segment.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "TaskApp.h"


///////////////////////////////////////////////////////////////
CSecuredSocketConnected::CSecuredSocketConnected(int size,
                                                 int threshold)
    
        :COsSocketConnected(size,threshold)
{
	m_pSsl = NULL;
}

///////////////////////////////////////////////////////////////
CSecuredSocketConnected::~CSecuredSocketConnected()
{
}

///////////////////////////////////////////////////////////////
void CSecuredSocketConnected::Serialize(CSegment& seg) const
{
	seg << (DWORD) m_descriptor;
    seg << (DWORD) m_remoteIp;
    seg << (DWORD) m_pSsl;
}  
/////////////////////////////////////////////////////////////////////////////
void CSecuredSocketConnected::DeSerialize(CSegment& seg)
{
	DWORD temp;
	seg >> temp;
	m_descriptor = temp;

	seg >> temp;
 	m_remoteIp = temp;
 	
 	seg >> temp;
	m_pSsl = (SSL*)temp;
}

///////////////////////////////////////////////////////////////
int CSecuredSocketConnected::Receive(char * buffer,int bytesToRead)
{
	// Allows to run other tasks during the operation
	//CTaskApp::Unlocker unlocker(true);// removed for BRIDGE-13399 

    int err2, end_loop=0;
    int nRead = 0;
    do
    {
        nRead = SSL_read(m_pSsl, (char *)buffer, bytesToRead);
        buffer[nRead]='\0';
        
        err2 = SSL_get_error( m_pSsl, nRead );
        end_loop++;
    } while ((err2==SSL_ERROR_WANT_READ || err2==SSL_ERROR_WANT_WRITE)
             && end_loop<10);
    
    return nRead;
}

/////////////////////////////////////////////////////////////////////////////
int CSecuredSocketConnected::Send(const char* buffer, int bytesToWrite)
{
    int nWrote;
    int err2, end_loop=0;
    
    if (m_pSsl == NULL)
    {
    	FTRACESTR(eLevelInfoNormal) << "CSecuredSocketConnected::Send - m_pSsl is NULL!!!";
    	return 0;
    }
    
    do
    {
        nWrote = SSL_write(m_pSsl, buffer, bytesToWrite);
        err2 = SSL_get_error( m_pSsl, nWrote );
        end_loop++;
    } while((err2==SSL_ERROR_WANT_READ||err2==SSL_ERROR_WANT_WRITE)
            && end_loop<10);
    
    return nWrote;
    
}
///////////////////////////////////////////////////////////////////////////// 
void CSecuredSocketConnected::SetTlsParams(void* ssl)
{
	m_pSsl = (SSL*)ssl;
}
///////////////////////////////////////////////////////////////////////////// 
BYTE CSecuredSocketConnected::IsSecured()
{
	return TRUE;
}
