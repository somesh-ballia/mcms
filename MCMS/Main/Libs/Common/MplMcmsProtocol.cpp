//+========================================================================+
//                            MplMcmsProtocol.CPP                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MplMcmsProtocol.CPP                                         |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#include <netinet/in.h>
#include <string.h>

#include "MplMcmsProtocol.h"


#include "TraceStream.h"
#include "Segment.h"
#include "Trace.h"
#include "Macros.h"
#include "ProcessBase.h"
#include "StatusesGeneral.h"
#include "MplMcmsProtocolTracer.h"
#include "ObjString.h"
#include "TaskApi.h"
#include "OpcodesMcmsInternal.h"
#include "InternalProcessStatuses.h"
#include "WrappersCommon.h"

extern const char* MainEntityToString(APIU32 entityType);

DWORD CMplMcmsProtocol::m_sequence_num = 0;

/////////////////////////////////////////////////////////////////////////////
// CSocketHdr class

CMplMcmsProtocol::CMplMcmsProtocol()
{
	m_PhysicalInfoHeaderCounter    = 0;
	m_PortDescriptionHeaderCounter = 0;
	m_pData = NULL;
	m_DataLen = 0;
	//m_sequence_num++;

	memset(&m_MplMcmsCommonHeader,0,sizeof(COMMON_HEADER_S));
	memset(&m_MplMcmsMsgDescriptionHeader,0,sizeof(MESSAGE_DESCRIPTION_HEADER_S));
	memset(&m_CS_McmsHeader,0,sizeof(CENTRAL_SIGNALING_HEADER_S));
	memset(&m_AuditHeader, 0, sizeof(AUDIT_EVENT_HEADER_S));

//	memset(&m_Errno, 0, sizeof(MplMcmsProtocolErrno_S));

	for  ( int PInfoHCounter=0; PInfoHCounter<MAX_PHYSICAL_INFO_HEADERS ; PInfoHCounter++ )
		memset(&(m_MplMcmsPhysicalInfoHeader[PInfoHCounter]),0,sizeof(PHYSICAL_INFO_HEADER_S));
	
	for  ( int PortdscHeaderCounter=0; PortdscHeaderCounter<MAX_PORT_DESCRIPTION_HEADERS; PortdscHeaderCounter++)
		memset(&(m_MplMcmsPortDescriptionHeader[PortdscHeaderCounter]),0,sizeof(PORT_DESCRIPTION_HEADER_S));

    for(int i = 0 ; i < NUM_OF_HEADER_TYPES ; i++)
    {
        m_HeaderInitArray[i] = false;
    }
    memset(&m_TraceHeader, 0, sizeof(TRACE_HEADER_S));

    m_Errno.m_Message="";
    m_Errno.m_Status=0;

}

/////////////////////////////////////////////////////////////////////////////
CMplMcmsProtocol::CMplMcmsProtocol(const CMplMcmsProtocol& other) : CPObject(other)
{
	memcpy(&m_MplMcmsCommonHeader,&(other.m_MplMcmsCommonHeader),sizeof(COMMON_HEADER_S));
	memcpy(&m_MplMcmsMsgDescriptionHeader,&(other.m_MplMcmsMsgDescriptionHeader),sizeof(MESSAGE_DESCRIPTION_HEADER_S));
	memcpy(&m_CS_McmsHeader,&(other.m_CS_McmsHeader),sizeof(CENTRAL_SIGNALING_HEADER_S));
	memcpy(&m_AuditHeader,&(other.m_AuditHeader),sizeof(AUDIT_EVENT_HEADER_S));

	BYTE  i;
	m_PhysicalInfoHeaderCounter = other.m_PhysicalInfoHeaderCounter;
	for( i=0; i<m_PhysicalInfoHeaderCounter; i++ )
		memcpy(&(m_MplMcmsPhysicalInfoHeader[i]),&(other.m_MplMcmsPhysicalInfoHeader[i]),sizeof(PHYSICAL_INFO_HEADER_S));
	for( i=m_PhysicalInfoHeaderCounter; i<MAX_PHYSICAL_INFO_HEADERS; i++ )
		memset(&(m_MplMcmsPhysicalInfoHeader[i]),0,sizeof(PHYSICAL_INFO_HEADER_S));
	m_PortDescriptionHeaderCounter = other.m_PortDescriptionHeaderCounter;
	for( i=0; i<m_PortDescriptionHeaderCounter; i++ )
		memcpy(&(m_MplMcmsPortDescriptionHeader[i]),&(other.m_MplMcmsPortDescriptionHeader[i]),sizeof(PORT_DESCRIPTION_HEADER_S));
	for( i=m_PortDescriptionHeaderCounter; i<MAX_PORT_DESCRIPTION_HEADERS; i++ )
		memset(&(m_MplMcmsPortDescriptionHeader[i]),0,sizeof(PORT_DESCRIPTION_HEADER_S));

	m_DataLen = other.m_DataLen;
	m_pData = NULL;
	if( m_DataLen ) {
		m_pData = new char[m_DataLen];
		memcpy(m_pData,other.m_pData,m_DataLen);
	}

    for(int i = 0 ; i < NUM_OF_HEADER_TYPES ; i++)
    {
        m_HeaderInitArray[i] = other.m_HeaderInitArray[i];
    }
    memset(&m_TraceHeader, 0, sizeof(TRACE_HEADER_S));

    m_Errno.m_Message=other.m_Errno.m_Message;
    m_Errno.m_Status=other.m_Errno.m_Status;
}

/////////////////////////////////////////////////////////////////////////////
CMplMcmsProtocol::~CMplMcmsProtocol()
{
	PDELETEA(m_pData);
}

/////////////////////////////////////////////////////////////////////////////
bool CMplMcmsProtocol::operator != (const CMplMcmsProtocol & other)const
{
    bool res = CMplMcmsProtocol::operator ==(other);
    return !res;
}

/////////////////////////////////////////////////////////////////////////////
bool CMplMcmsProtocol::operator == (const CMplMcmsProtocol & other)const
{
    if(this == &other)
    {
        return true;
    }
    int res = memcmp(&m_MplMcmsCommonHeader, &other.m_MplMcmsCommonHeader, sizeof(COMMON_HEADER_S));
    if(0 != res)
    {
        return false;
    }
   //  res = memcmp(&m_MplMcmsMsgDescriptionHeader, &other.m_MplMcmsMsgDescriptionHeader, sizeof(MESSAGE_DESCRIPTION_HEADER_S));
//     if(0 != res)
//     {
//         return false;
//     }
    res = memcmp(&m_CS_McmsHeader, &other.m_CS_McmsHeader, sizeof(CENTRAL_SIGNALING_HEADER_S));
    if(0 != res)
    {
        return false;
    }
    res = memcmp(&m_TraceHeader, &other.m_TraceHeader, sizeof(TRACE_HEADER_S));
    if(0 != res)
    {
        return false;
    }
    if(m_DataLen != other.m_DataLen)
    {
        return false;
    }
    res = memcmp(m_pData, other.m_pData, m_DataLen);
    if(0 != res)
    {
        return false;
    }
    if(m_PhysicalInfoHeaderCounter != other.m_PhysicalInfoHeaderCounter)
    {
        return false;
    }
    for(int i = 0 ; i < m_PhysicalInfoHeaderCounter ; i++)
    {
        res = memcmp(&m_MplMcmsPhysicalInfoHeader[i], &other.m_MplMcmsPhysicalInfoHeader[i], sizeof(PHYSICAL_INFO_HEADER_S));
        if(0 != res)
        {
            return false;
        }
    }
    if(m_PortDescriptionHeaderCounter != other.m_PortDescriptionHeaderCounter)
    {
        return false;
    }
    for(int i = 0 ; i <  m_PortDescriptionHeaderCounter; i++)
    {
        res = memcmp(&m_MplMcmsPortDescriptionHeader[i], &other.m_MplMcmsPortDescriptionHeader[i], sizeof(PORT_DESCRIPTION_HEADER_S));
        if(0 != res)
        {
            return false;
        }
    }
    for(int i = 0 ; i < NUM_OF_HEADER_TYPES ; i++)
    {
        if(m_HeaderInitArray[i] != other.m_HeaderInitArray[i])
        {
            return false;
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::SerializeLogger(CSegment& seg)
{
    m_MplMcmsCommonHeader.time_stamp = SystemGetTickCount().GetIntegerPartForTrace();
    
    if(true == m_HeaderInitArray[eHeaderPhysical])
    {
        m_MplMcmsCommonHeader.next_header_type = (DWORD)eHeaderPhysical;
        m_MplMcmsCommonHeader.next_header_offset = sizeof(PHYSICAL_INFO_HEADER_S);
        seg.Put((BYTE*)&m_MplMcmsCommonHeader,sizeof(COMMON_HEADER_S));
        
        m_MplMcmsPhysicalInfoHeader[0].next_header_type = (DWORD)eHeaderTrace;
        m_MplMcmsPhysicalInfoHeader[0].next_header_size = sizeof(TRACE_HEADER_S);
        seg.Put((BYTE*)&m_MplMcmsPhysicalInfoHeader, sizeof(PHYSICAL_INFO_HEADER_S));
    }
    else
    {
        m_MplMcmsCommonHeader.next_header_type = (DWORD)eHeaderTrace;
        m_MplMcmsCommonHeader.next_header_offset = sizeof(TRACE_HEADER_S);
        seg.Put((BYTE*)&m_MplMcmsCommonHeader,sizeof(COMMON_HEADER_S));
    }
    
    seg.Put((BYTE*)&m_TraceHeader, sizeof(TRACE_HEADER_S));
    
    seg.Put((BYTE*)m_pData,m_DataLen);
}

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::Serialize(CSegment& seg,BOOL Type)
{
	//seg.Put((BYTE *)&m_TPKT_Header,(DWORD)sizeof(TPKT_HEADER_S));

	///  Prepare and put Common Header
	m_MplMcmsCommonHeader.next_header_type = (DWORD)eHeaderMsgDesc;
	m_MplMcmsCommonHeader.next_header_offset = sizeof(MESSAGE_DESCRIPTION_HEADER_S);
	m_MplMcmsCommonHeader.time_stamp       = SystemGetTickCount().GetIntegerPartForTrace();
   // SAGI : we take part of the system tick assuming we use this time stamp only for tracing (otherwise will get int overflow bugs!!!)
	seg.Put((BYTE*)&m_MplMcmsCommonHeader,sizeof(COMMON_HEADER_S));

	///  Prepare and put Message Description Header
	eHeaderType  enNextHeaderType = eHeaderNone;
	DWORD        nNextHeaderSize  = 0;
	if (CS_API_TYPE== Type)
	{
		enNextHeaderType = eHeaderCs;
		nNextHeaderSize  = sizeof(CENTRAL_SIGNALING_HEADER_S);
		m_MplMcmsMsgDescriptionHeader.next_header_type = (DWORD)enNextHeaderType;
    	m_MplMcmsMsgDescriptionHeader.next_header_size = nNextHeaderSize;
	    seg.Put((BYTE*)&m_MplMcmsMsgDescriptionHeader,sizeof(MESSAGE_DESCRIPTION_HEADER_S));
	}
	if( m_PhysicalInfoHeaderCounter ) {
		enNextHeaderType = eHeaderPhysical;
		nNextHeaderSize  = sizeof(PHYSICAL_INFO_HEADER_S);
	} else if( m_PortDescriptionHeaderCounter ) {
		enNextHeaderType = eHeaderPortDesc;
		nNextHeaderSize  = sizeof(PORT_DESCRIPTION_HEADER_S);
	}


	if (CS_API_TYPE== Type)
	{
	  m_CS_McmsHeader.next_header_type = (DWORD)enNextHeaderType;
      m_CS_McmsHeader.next_header_offset = nNextHeaderSize;
      seg.Put((BYTE*)&m_CS_McmsHeader,sizeof(CENTRAL_SIGNALING_HEADER_S));
	}
	else
	{
	m_MplMcmsMsgDescriptionHeader.next_header_type = (DWORD)enNextHeaderType;
	m_MplMcmsMsgDescriptionHeader.next_header_size = nNextHeaderSize;
	seg.Put((BYTE*)&m_MplMcmsMsgDescriptionHeader,sizeof(MESSAGE_DESCRIPTION_HEADER_S));
	}
	int i;
	///  Prepare and put all Physical Headers
	for ( i=0; i<m_PhysicalInfoHeaderCounter && i<MAX_PHYSICAL_INFO_HEADERS; i++ ) {
		// last array element should have another next_header_type and size
		if( i>=m_PhysicalInfoHeaderCounter-1 ) {
			if ( m_PortDescriptionHeaderCounter ) {
				enNextHeaderType = eHeaderPortDesc;
				nNextHeaderSize  = sizeof(PORT_DESCRIPTION_HEADER_S);
			} else {
				enNextHeaderType = eHeaderNone;
				nNextHeaderSize  = 0;
			}
		}

		m_MplMcmsPhysicalInfoHeader[i].next_header_type = (DWORD)enNextHeaderType;
		m_MplMcmsPhysicalInfoHeader[i].next_header_size = nNextHeaderSize;
		seg.Put((BYTE*)&m_MplMcmsPhysicalInfoHeader[i],sizeof(PHYSICAL_INFO_HEADER_S));
	}
	
	///  Prepare and put all Port Descriptor Headers
	for ( i=0; i<m_PortDescriptionHeaderCounter; i++ ) {
		// last array element should have another next_header_type and size
		if( i>=m_PortDescriptionHeaderCounter-1 ) {
			enNextHeaderType = eHeaderNone;
			nNextHeaderSize  = 0;
		}
		m_MplMcmsPortDescriptionHeader[i].next_header_type = (DWORD)enNextHeaderType;
		m_MplMcmsPortDescriptionHeader[i].next_header_size = nNextHeaderSize;
		seg.Put((BYTE*)&m_MplMcmsPortDescriptionHeader[i],sizeof(PORT_DESCRIPTION_HEADER_S));
	}

	///  Put data array
	seg.Put((BYTE*)m_pData,m_DataLen);
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
bool CMplMcmsProtocol::DeSerialize(CSegment& seg,BOOL Type)
{
	DWORD	nMsgLen =  seg.GetWrtOffset() - seg.GetRdOffset();
	if( 0 == nMsgLen )
    {
		return false;
    }
    
    BYTE*	pMessage = new BYTE[nMsgLen];
	seg.Get(pMessage,nMsgLen);
	bool res = DeSerialize(pMessage,nMsgLen,Type);
	PDELETEA(pMessage);

    return res;
}

///////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::InitErrno()
{
    m_Errno.m_Message = "";
    m_Errno.m_Status = PROTOCOL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////
bool CMplMcmsProtocol::DeSerialize(const BYTE* pBuffer,const DWORD nBufferLen,BOOL Type)
{
    InitErrno();
    
    if(CS_API_TYPE == Type)
    {
        OldDeserialize(pBuffer, nBufferLen, Type);
        return true;
    }
    
    DWORD nOffset = 0;

    if(pBuffer)
    {
		memcpy(&m_MplMcmsCommonHeader, pBuffer, sizeof(COMMON_HEADER_S));
		nOffset += sizeof(COMMON_HEADER_S);
	    m_HeaderInitArray[eHeaderCommon] = true;
    }
    else
    	PTRACE(eLevelError,"CMplMcmsProtocol::DeSerialize - buffer is NULL !!!!");

    int cnt = 0;
    DWORD nNextHeaderType = m_MplMcmsCommonHeader.next_header_type;






    while(eHeaderNone != nNextHeaderType)
    {   
        if(nNextHeaderType < eHeaderCommon || eHeaderUnknown <= nNextHeaderType)
        {
        	const CProcessBase* proc = CProcessBase::GetProcess();
        	std::string sop = proc ? proc->GetOpcodeAsString(m_MplMcmsCommonHeader.opcode) : "unknown";

        	CLargeString message;
        	message << __PRETTY_FUNCTION__ << " - 1: "
        			<< "Bad next header type, cnt = " << cnt
        			<< ", nNextHeaderType = " << nNextHeaderType
        			<< ", opcode = " << sop << " (" << m_MplMcmsCommonHeader.opcode << ")"
        			<< ", dst = " << MainEntityToString(m_MplMcmsCommonHeader.dest_id)
        			<< ", src = " << MainEntityToString(m_MplMcmsCommonHeader.src_id);

        	m_Errno.m_Message = message.GetString();

        	static bool firstAccurence = false;
        	CSegment* paramSeg = new CSegment;
        	if (!firstAccurence && pBuffer)
        	{
        		firstAccurence = true;
        		paramSeg->Put((BYTE*)pBuffer, nBufferLen );
        		paramSeg->DumpMsgHex();
        	}
        	POBJDELETE(paramSeg);


        	m_Errno.m_Status = PROTOCOL_STATUS_BAD_NEXT_HEADER;
        	return false;
        }

        m_HeaderInitArray[nNextHeaderType] = true;
        
        switch (nNextHeaderType)
		{
            case eHeaderAudit:
            {
                memcpy(&m_AuditHeader,
                       pBuffer + nOffset,
                       sizeof(AUDIT_EVENT_HEADER_S));
                nOffset += sizeof(AUDIT_EVENT_HEADER_S);
                nNextHeaderType = eHeaderNone;
                break;
            }
            
            case eHeaderTrace:
            {
                memcpy(&m_TraceHeader,
                       pBuffer + nOffset,
                       sizeof(TRACE_HEADER_S));
                nOffset += sizeof(TRACE_HEADER_S);
                nNextHeaderType = eHeaderNone; //m_TraceHeader.next_header_t;
                break;
            }
            
            case eHeaderCs:
            {
                memcpy(&m_CS_McmsHeader,
                       pBuffer + nOffset,
                       sizeof(CENTRAL_SIGNALING_HEADER_S));
                nOffset += sizeof(CENTRAL_SIGNALING_HEADER_S);
                nNextHeaderType = m_CS_McmsHeader.next_header_type;
                break;
            }

            case eHeaderMsgDesc:
            {
                memcpy(&m_MplMcmsMsgDescriptionHeader,
                       pBuffer + nOffset,
                       sizeof(MESSAGE_DESCRIPTION_HEADER_S));
                nOffset += sizeof(MESSAGE_DESCRIPTION_HEADER_S);
                nNextHeaderType = m_MplMcmsMsgDescriptionHeader.next_header_type;
                break;
            }
                
			case eHeaderPhysical:
            {
				memcpy(&m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter],
                       pBuffer + nOffset,
                       sizeof(PHYSICAL_INFO_HEADER_S));
				nOffset += sizeof(PHYSICAL_INFO_HEADER_S);

				nNextHeaderType = m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].next_header_type;

				if ( m_PhysicalInfoHeaderCounter < MAX_PHYSICAL_INFO_HEADERS )
                {
                    m_PhysicalInfoHeaderCounter++;
                }
				break;
			}

            case eHeaderPortDesc:
            {
                memcpy(&m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter],
                       pBuffer+nOffset,
                       sizeof(PORT_DESCRIPTION_HEADER_S));
				nOffset += sizeof(PORT_DESCRIPTION_HEADER_S);
				nNextHeaderType = m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].next_header_type;

				if ( m_PortDescriptionHeaderCounter < MAX_PORT_DESCRIPTION_HEADERS )
                {
					m_PortDescriptionHeaderCounter++;
                }
                break;
            }

            default :
            {
            	CMedString message;
            	message << "CMplMcmsProtocol::DeSerialize - 2. Bad next header type, cnt = "
            			<< cnt
            			<< "\n received type = "
            			<< nNextHeaderType;
            	m_Errno.m_Message = message.GetString();
            	m_Errno.m_Status = PROTOCOL_STATUS_BAD_NEXT_HEADER;
                
                return false;
                break;
            }
        }

        cnt++;
    }
    
    if (nOffset > nBufferLen)
    {
    	CMedString message;
    	message << "CMplMcmsProtocol::DeSerialize - offset len problem : "
    			<< "offset : " << nOffset << " , nBufferLen : " << nBufferLen;
    	m_Errno.m_Message = message.GetString();
    	m_Errno.m_Status = PROTOCOL_STATUS_BAD_OFFSET_LEN;
        return false;
    }
    
    m_DataLen = nBufferLen - nOffset;
    PDELETEA(m_pData);
    if(0 < m_DataLen)
    {
        m_pData = new char[m_DataLen + 1];
        memcpy(m_pData, pBuffer + nOffset, m_DataLen);
        m_pData[m_DataLen] = '\0';
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::OldDeserialize(const BYTE* pBuffer,const DWORD nBufferLen,BOOL Type)
{
    memcpy(&m_MplMcmsCommonHeader,pBuffer,sizeof(COMMON_HEADER_S));
	DWORD nOffset = sizeof(COMMON_HEADER_S);
    m_HeaderInitArray[eHeaderCommon] = true;
    
	memcpy(&m_MplMcmsMsgDescriptionHeader,pBuffer+nOffset,sizeof(MESSAGE_DESCRIPTION_HEADER_S));
	nOffset += sizeof(MESSAGE_DESCRIPTION_HEADER_S);
    m_HeaderInitArray[eHeaderMsgDesc] = true;
    
	DWORD nNextHeaderType = m_MplMcmsMsgDescriptionHeader.next_header_type;
    
	if (CS_API_TYPE== Type)
	{
        memcpy(&m_CS_McmsHeader,pBuffer+nOffset,sizeof(CENTRAL_SIGNALING_HEADER_S));
        nOffset += sizeof(CENTRAL_SIGNALING_HEADER_S);
        nNextHeaderType = m_CS_McmsHeader .next_header_type;
        m_HeaderInitArray[eHeaderCs] = true;
	}

	while ( (nNextHeaderType==eHeaderPhysical) || (nNextHeaderType==eHeaderPortDesc) )
	{
        m_HeaderInitArray[nNextHeaderType] = true;
        
		switch (nNextHeaderType)
		{
			case eHeaderPhysical:	{
				memcpy(&m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter],
						(pBuffer+nOffset),sizeof(PHYSICAL_INFO_HEADER_S));
				nOffset += sizeof(PHYSICAL_INFO_HEADER_S);

				nNextHeaderType = m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].next_header_type;

				if ( m_PhysicalInfoHeaderCounter < MAX_PHYSICAL_INFO_HEADERS )
					m_PhysicalInfoHeaderCounter++;

				break;
			}
			case eHeaderPortDesc:
			{
				memcpy(&m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter],
						(pBuffer+nOffset),sizeof(PORT_DESCRIPTION_HEADER_S));
				nOffset += sizeof(PORT_DESCRIPTION_HEADER_S);

				nNextHeaderType = m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].next_header_type;

				if ( m_PortDescriptionHeaderCounter < MAX_PORT_DESCRIPTION_HEADERS )
					m_PortDescriptionHeaderCounter++;

				break;
			}
			default :
				break;
		}
	}
	if (nOffset>nBufferLen)
	{
		PASSERT(1);
			PTRACE(eLevelInfoNormal,"CMplMcmsProtocol::DeSerialize : offset len problem");

	}
	else
	{
 	m_DataLen = nBufferLen - nOffset;
	PDELETEA(m_pData);
	if( m_DataLen ) {
		m_pData = new char[m_DataLen];
		memcpy(m_pData,pBuffer+nOffset,m_DataLen);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::Serialize_TPKT_Header(CSegment& seg) const
{/*
	seg.Put((BYTE *)&m_TPKT_Header,(DWORD)sizeof(TPKT_HEADER_S));
	*/
}

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::DeSerialize_TPKT_Header(CSegment& seg)
{
	/*
	seg.Get((BYTE*) &m_TPKT_Header,sizeof(TPKT_HEADER_S));
	*/
}


///////////////////////////////////////////////////////////////////////////
/*void CMplMcmsProtocol::ReadMplMcmsProtocol(char* pMainBuf,int MainBufLen)
{
	char* pCopyBuf = new char[MainBufLen];
	memcpy(pCopyBuf,pMainBuf,MainBufLen);

	CSegment *pSeg = new CSegment;
	pSeg->Create(pCopyBuf,(WORD)MainBufLen);
	DeSerialize(*pSeg);
	POBJDELETE(pSeg);
	// pCopyBuf was released in destructor of Segment
}*/

///////////////////////////////////////////////////////////////////////////
/*void CMplMcmsProtocol::ReadMplMcmsCommonHeaderPortsAndPhysicalInfoHedersAndData(char* Mainbuf,int MainBufLen)
{
	memcpy(&m_MplMcmsCommonHeader,Mainbuf,sizeof(COMMON_HEADER_S));
	memcpy(&m_MplMcmsMsgDescriptionHeader,Mainbuf,sizeof(MESSAGE_DESCRIPTION_HEADER_S));

	int NextHeader_type = m_MplMcmsMsgDescriptionHeader.next_header_type;
	int offsetFromTheBufferStart = sizeof(COMMON_HEADER_S) + sizeof(MESSAGE_DESCRIPTION_HEADER_S);

	while ( (NextHeader_type==eHeaderPhysical) || (NextHeader_type==eHeaderPortDesc) )
	{
		switch (NextHeader_type)
		{
			case eHeaderPhysical:	{
				memcpy(&m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter],
						(Mainbuf+offsetFromTheBufferStart),sizeof(PHYSICAL_INFO_HEADER_S));
				offsetFromTheBufferStart += sizeof(PHYSICAL_INFO_HEADER_S);

				NextHeader_type = m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].next_header_type;
				if ( m_PhysicalInfoHeaderCounter < MAX_PHYSICAL_INFO_HEADERS )
				{
					m_PhysicalInfoHeaderCounter++;
				}
				break;
			}
			case eHeaderPortDesc:
			{
				memcpy(&m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter],
						(Mainbuf+offsetFromTheBufferStart),sizeof(PORT_DESCRIPTION_HEADER_S));
				offsetFromTheBufferStart += sizeof(PORT_DESCRIPTION_HEADER_S);

				NextHeader_type = m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].next_header_type;
				if ( m_PortDescriptionHeaderCounter < MAX_PORT_DESCRIPTION_HEADERS )
				{
					m_PortDescriptionHeaderCounter++;
				}
				break;
			}
			default :
				break;
		}
	}
 	m_DataLen = (MainBufLen-offsetFromTheBufferStart);
	PDELETEA(m_pData);
	if( m_DataLen ) {
		m_pData = new char[m_DataLen];
		memcpy(m_pData,(Mainbuf+offsetFromTheBufferStart),m_DataLen);
	}
}*/

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::SetTPKT_Header(char* buf)
{
}

//////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::UpdateSendTimes()
{
	m_MplMcmsCommonHeader.time_stamp 			= m_MplMcmsMsgDescriptionHeader.time_stamp;
	m_MplMcmsMsgDescriptionHeader.time_stamp 	= SystemGetTickCount().GetIntegerPartForTrace();
}

//////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::AddCommonHeader     (const DWORD opcode,
											const BYTE  protocol_version,
                                            const BYTE  option,
											const BYTE  src_id,
											const BYTE  dest_id,
											const DWORD time_stamp,
										//	const DWORD sequence_num,
											const DWORD payload_len,
											const DWORD payload_offset,
											const DWORD next_header_type,
    										const DWORD next_header_size)
{
	if (protocol_version)
	{
		m_MplMcmsCommonHeader.protocol_version = protocol_version;
	}
	else
	{
		m_MplMcmsCommonHeader.protocol_version = MPL_PROTOCOL_VERSION_NUM;
	}
	
	m_MplMcmsCommonHeader.option           = option          ;        
	m_MplMcmsCommonHeader.src_id           = src_id          ; 
	m_MplMcmsCommonHeader.dest_id		   = dest_id		 ;       
	m_MplMcmsCommonHeader.opcode		   = opcode		     ;
	m_MplMcmsCommonHeader.time_stamp       = time_stamp	     ;       
//	m_MplMcmsCommonHeader.sequence_num	   = sequence_num	 ;       
	m_MplMcmsCommonHeader.payload_len	   = payload_len	 ;       
	m_MplMcmsCommonHeader.payload_offset   = payload_offset  ;       
	m_MplMcmsCommonHeader.next_header_type = next_header_type;       
    m_MplMcmsCommonHeader.next_header_offset = next_header_size;

    m_HeaderInitArray[eHeaderCommon] = true;
}

//////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::SetCommonHeaderOpcode(const OPCODE opcode)
{
	m_MplMcmsCommonHeader.opcode = opcode;
}

//////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::UpdateCommonHeaderToMPL(DWORD recivedSecNum)
{
	if (!m_MplMcmsCommonHeader.protocol_version)
		m_MplMcmsCommonHeader.protocol_version = MPL_PROTOCOL_VERSION_NUM;
	//	m_MplMcmsCommonHeader.option               = option          ;        
	if (!m_MplMcmsCommonHeader.src_id)
		m_MplMcmsCommonHeader.src_id           = eMcms                ;
	if (!m_MplMcmsCommonHeader.dest_id)
		m_MplMcmsCommonHeader.dest_id		   = eMpl	             ;       
	
	//	m_MplMcmsCommonHeader.opcode		   = opcode		         ;
	if (!m_MplMcmsCommonHeader.time_stamp)
		m_MplMcmsCommonHeader.time_stamp       = SystemGetTickCount().GetIntegerPartForTrace();
   // SAGI : we take part of the system tick assuming we use this time stamp only for tracing (otherwise will get int overflow bugs!!!)
	
// 1080_60
    if(recivedSecNum == 0)
    {
    	m_MplMcmsCommonHeader.sequence_num	       = GetUpdated_sequence_num() ;
    }
    else
    {
    	m_MplMcmsCommonHeader.sequence_num = recivedSecNum;
    }
	//	m_MplMcmsCommonHeader.payload_len	   = payload_len	 ;       
	//	m_MplMcmsCommonHeader.payload_offset   = payload_offset  ;       
	//	m_MplMcmsCommonHeader.next_header_type = next_header_type;       
	//  m_MplMcmsCommonHeader.next_header_size = next_header_size;
}

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::UpdateMessageDescriptionHeaderReqId(DWORD request_id)

{
	m_MplMcmsMsgDescriptionHeader.request_id       = request_id;
}
/////////////////////////////////////////////////////////////////////////////

void CMplMcmsProtocol::AddMessageDescriptionHeader(const DWORD request_id,
                                                   const DWORD entity_type,
                                                   const DWORD time_stamp, 
                                                   const DWORD next_header_type,
                                                   const DWORD next_header_size)

{
//	m_MplMcmsMsgDescriptionHeader.request_id       = request_id;
	m_MplMcmsMsgDescriptionHeader.request_id       = GetUpdated_sequence_num();

    if (entity_type)
		m_MplMcmsMsgDescriptionHeader.entity_type   = entity_type;
	else 
	{
		static CProcessBase * process = CProcessBase::GetProcess();
		m_MplMcmsMsgDescriptionHeader.entity_type = (APIU32)(NULL != process ? process->GetProcessType() : eProcessTypeInvalid);
	}

	m_MplMcmsMsgDescriptionHeader.time_stamp = (0 != time_stamp ? time_stamp : SystemGetTickCount().GetIntegerPartForTrace()); 
   // SAGI : we take part of the system tick assuming we use this time stamp only for tracing (otherwise will get int overflow bugs!!!)
	
	m_MplMcmsMsgDescriptionHeader.next_header_type = next_header_type;      
	m_MplMcmsMsgDescriptionHeader.next_header_size = next_header_size;
	m_MplMcmsMsgDescriptionHeader.msg_ack_ind = 0;
	m_MplMcmsMsgDescriptionHeader.future_use1 = 0;
	m_MplMcmsMsgDescriptionHeader.future_use1 = 0;

    m_HeaderInitArray[eHeaderMsgDesc] = true;
}

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::AddTraceHeader(const TRACE_HEADER_S & header)
{
    memcpy(&m_TraceHeader, &header, sizeof(TRACE_HEADER_S));
    m_HeaderInitArray[eHeaderTrace] = true;
}

////////////////////////////////////////////////////////////////////////////////////
DWORD CMplMcmsProtocol::GetUpdated_sequence_num()
{
	m_sequence_num++;
	return m_sequence_num;
}

////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::SetPortDescriptorHeaderLogicRsrcType1(const BYTE   logical_resource)
{
	m_MplMcmsPortDescriptionHeader[0].logical_resource_type_1 = logical_resource;
}

////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::SetPortDescriptorHeaderRoomId(const WORD   room_id)
{
	m_MplMcmsPortDescriptionHeader[0].room_id = room_id;
}

////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::AddPortDescriptionHeader    ( const DWORD  party_id,
													 const DWORD  conf_id,
													 const DWORD  connection_id,
													 const BYTE   logical_resource_type_1, 
													 const BYTE   logical_resource_type_2,
													 const BYTE   future_use1,
													 const BYTE   future_use2,
													 const DWORD  next_header_type,
													 const DWORD  next_header_size,
													 const WORD   room_id)
{
	if ( m_PortDescriptionHeaderCounter<MAX_PORT_DESCRIPTION_HEADERS )
	{
		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].party_id                = party_id               ;
		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].conf_id                 = conf_id                ;      
		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].connection_id           = connection_id          ;      
		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].logical_resource_type_1 = logical_resource_type_1;     
		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].logical_resource_type_2 = logical_resource_type_2;      
//		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].future_use1             = future_use1            ;
//		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].future_use2             = future_use2            ;
		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].room_id 				   = room_id;
		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].next_header_type        = next_header_type       ;      
		m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].next_header_size        = next_header_size       ;      
		m_PortDescriptionHeaderCounter++;
	}
	else
	{
		//To Do
	}
    m_HeaderInitArray[eHeaderPortDesc] = true;
}
////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::UpdatePortDescriptionHeader ( const DWORD  party_id,
													 const DWORD  conf_id,
													 const DWORD  connection_id,
													 const BYTE   logical_resource_type_1, 
													 const BYTE   logical_resource_type_2,
													 const BYTE   future_use1,
													 const BYTE   future_use2,
													 const DWORD  next_header_type,
													 const DWORD  next_header_size, 
													 const int counter,
													 const WORD   room_id)
{
	m_MplMcmsPortDescriptionHeader[counter].party_id                = party_id               ;
	m_MplMcmsPortDescriptionHeader[counter].conf_id                 = conf_id                ;      
	m_MplMcmsPortDescriptionHeader[counter].connection_id           = connection_id          ;      
	m_MplMcmsPortDescriptionHeader[counter].logical_resource_type_1 = logical_resource_type_1;     
	m_MplMcmsPortDescriptionHeader[counter].logical_resource_type_2 = logical_resource_type_2;      
//	m_MplMcmsPortDescriptionHeader[counter].future_use1             = future_use1            ;
//	m_MplMcmsPortDescriptionHeader[counter].future_use2             = future_use2            ;
	m_MplMcmsPortDescriptionHeader[counter].room_id 				= room_id				 ;
	m_MplMcmsPortDescriptionHeader[counter].next_header_type        = next_header_type       ;      
	m_MplMcmsPortDescriptionHeader[counter].next_header_size        = next_header_size       ;      
	
}
////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::AddCSHeader		       (const WORD   cs_id ,
											   	const WORD   src_unit_id,
											    const WORD   dst_unit_id,
												const DWORD  call_index,
												const DWORD  service_id,
												const DWORD  channel_index,
												const DWORD  mc_channel_index,
												const APIS32 status,
												const WORD   next_header_type,
												const WORD   next_header_offset)
{
    m_CS_McmsHeader.cs_id = cs_id;
	//	m_CS_McmsHeader.unit_id = unit_id;
    m_CS_McmsHeader.src_unit_id = src_unit_id;
    m_CS_McmsHeader.dst_unit_id = dst_unit_id;
	//	m_CS_McmsHeader.reserved // = reserved; 
    m_CS_McmsHeader.call_index = call_index;
    m_CS_McmsHeader.service_id = service_id;
    m_CS_McmsHeader.channel_index = channel_index;
    m_CS_McmsHeader.mc_channel_index = mc_channel_index;
    m_CS_McmsHeader.status = status;
    m_CS_McmsHeader.next_header_type = next_header_type;
    m_CS_McmsHeader.next_header_offset = next_header_offset;
		
	m_HeaderInitArray[eHeaderCs] = true;
	
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::GetPhysicalInfoHeaderConnToCardTable(const ConnToCardTableEntry* pConnToCardTableEntry)
{
	m_MplMcmsPhysicalInfoHeader->box_id          = pConnToCardTableEntry->boxId;
	m_MplMcmsPhysicalInfoHeader->board_id        = pConnToCardTableEntry->boardId;
	m_MplMcmsPhysicalInfoHeader->sub_board_id    = pConnToCardTableEntry->subBoardId;
	m_MplMcmsPhysicalInfoHeader->unit_id         = pConnToCardTableEntry->unitId;
	m_MplMcmsPhysicalInfoHeader->accelerator_id  = pConnToCardTableEntry->acceleratorId;
	m_MplMcmsPhysicalInfoHeader->port_id         = pConnToCardTableEntry->portId;
	m_MplMcmsPhysicalInfoHeader->resource_type   = pConnToCardTableEntry->physicalRsrcType;

	//m_PhysicalInfoHeaderCounter++;
	m_PhysicalInfoHeaderCounter = 1;
}
////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::AddPhysicalHeader		  ( const BYTE   box_id,
													const BYTE   board_id,
													const BYTE   sub_board_id,
													const BYTE   unit_id,
													const WORD   port_id,
													const BYTE   accelerator_id,
													const BYTE   resource_type,
													const BYTE   future_use1,
													const BYTE   future_use2,
													const DWORD  next_header_type,
													const DWORD  next_header_size )
{
	if ( m_PhysicalInfoHeaderCounter<MAX_PHYSICAL_INFO_HEADERS )
	{

		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].box_id				= box_id;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].board_id			= board_id;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].sub_board_id		= sub_board_id;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].unit_id			= unit_id;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].port_id			= port_id;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].accelerator_id		= accelerator_id;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].resource_type		= resource_type;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].future_use1		= future_use1;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].future_use2		= future_use2;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].reserved[0]		= 0;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].reserved[1]		= 0;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].next_header_type	= next_header_type;
		m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].next_header_size	= next_header_size;
		m_PhysicalInfoHeaderCounter++;
	}
	else
	{
		//To Do
	}
    m_HeaderInitArray[eHeaderPhysical] = true;
}

////////////////////////////////////////////////////////////////////////////////////



/////////
////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::AddData( const DWORD DataLen, const char* pData )
{
	PDELETEA(m_pData);
	
	m_DataLen = DataLen;
	if(m_DataLen > 0)
	{   
		m_pData = new char[m_DataLen];
		if(NULL!=m_pData)
		{
		 	memcpy(m_pData,pData,m_DataLen);
		}
		else
		{
			PTRACE(eLevelError,"CMplMcmsProtocol::AddData : not enough memory");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
STATUS CMplMcmsProtocol::SendMsgToMplApiCommandDispatcher()
{
    STATUS status = SendMessage(MPL_API_TYPE);
	return status;
}

////////////////////////////////////////////////////////////////////////////////////////
STATUS CMplMcmsProtocol::SendMsgToCSApiCommandDispatcher()
{
    m_HeaderInitArray[eHeaderCs] = true;
    STATUS status = SendMessage(CS_API_TYPE);
	return status;
}

////////////////////////////////////////////////////////////////////////////////////////
STATUS CMplMcmsProtocol::SendMessage(BOOL type,WORD numOfRetries)
{
    CSegment *seg = new CSegment;
    Serialize(*seg, type);

    eProcessType processType = eProcessCSApi;
    OPCODE opcode = BASIC_MSG_TO_CS_API;
    if(MPL_API_TYPE == type)
    {
        processType = eProcessMplApi;
        opcode = BASIC_MSG_TO_MPL_API;
    }
    
    CTaskApi api(processType, eDispatcher);

	STATUS status = api.SendMsg(seg, opcode,NULL,NULL,4,getOpcode());
    if(STATUS_OK != status)
    {	
        CLargeString message = "CMplMcmsProtocol::SendMsg : FAILED to send to : ";
        message << (CS_API_TYPE == type ? "CSApi" : "MplApi");
        message << "; opcode : ";
        
        CProcessBase * process = CProcessBase::GetProcess();
        if(eProcessMcuCmd != process->GetProcessType())
        {
            const string &strOpcode = process->GetOpcodeAsString(getOpcode());
            message << strOpcode.c_str();
        }
        PTRACE(eLevelInfoNormal, message.GetString());
    }
    return status;
}

// ////////////////////////////////////////////////////////////////////////////////////////
// STATUS CMplMcmsProtocol::SendMsgToMplApiCommandDispatcher()
// {
//     STATUS status= STATUS_OK;
// 	CProcessBase * process = CProcessBase::GetProcess();
// 	if (NULL != process)
// 	{
// 		CSegment *seg = new CSegment;
// 		Serialize(*seg);
    
//         CTaskApi api(eProcessMplApi,eDispatcher);
// 		status = api.SendMsg(seg,BASIC_MSG_TO_MPL_API);
// 		if(STATUS_OK != status)
// 		{
// 			CLargeString message = "CMplMcmsProtocol::SendMsgToMplApiCommandDispatcher : FAILED to send : ";
// 			if(eProcessMcuCmd != process->GetProcessType())
// 			{
// 				const string &strOpcode = process->GetOpcodeAsString(getOpcode());
// 				message << strOpcode.c_str();
// 			}
// 			PTRACE(eLevelInfoNormal, message.GetString());
// 		}
// 	}
// 	else
// 	{
// 		status=STATUS_FAIL;
// 		PTRACE(eLevelInfoNormal,"CMplMcmsProtocol::SendMsgToMplApiCommandDispatcher - failed to send BASIC_MSG_TO_MPL_API to MplApi ");
		
// 	}
// 	return status;
// }
// ////////////////////////////////////////////////////////////////////////////////////////
// STATUS CMplMcmsProtocol::SendMsgToCSApiCommandDispatcher()
// {
// 	STATUS status= STATUS_OK;
// 	CProcessBase * process = CProcessBase::GetProcess();
	
// 	if (process)
// 	{
// 		CSegment *seg = new CSegment;
// 		Serialize(*seg,CS_API_TYPE);
	
//         CTaskApi api(eProcessCSApi,eDispatcher);
//         status = api.SendMsg(seg,BASIC_MSG_TO_CS_API);
        
//         if(STATUS_OK != status)
// 		{	
// 			CLargeString message = "CMplMcmsProtocol::SendMsgToMplApiCommandDispatcher : FAILED to send : ";
// 			if(eProcessMcuCmd != process->GetProcessType())
// 			{
// 				const string &strOpcode = process->GetOpcodeAsString(getOpcode());
// 				message << strOpcode.c_str();
// 			}
// 			PTRACE(eLevelInfoNormal, message.GetString());
// 		}
// 	}
// 	else
// 	{
// 		status=STATUS_FAIL;
// 		PTRACE(eLevelInfoNormal,
//                "CMplMcmsProtocol::SendMsgToCSApiCommandDispatcher - failed to send BASIC_MSG_TO_CS_API to CSApi ");

// 	}
// 	return status;
// }

/////////////////////////////////////////////////////////////////////////////
OPCODE CMplMcmsProtocol::getOpcode() const
{
	return m_MplMcmsCommonHeader.opcode;
}

/////////////////////////////////////////////////////////////////////////////
// void CMplMcmsProtocol::ReadMplMcmsCommonHeaderPortsAndPhysicalInfoHedersAndData(char* Mainbuf,int MainBufLen)
// {
// 	int offsetFromTheBufferStart=0;

// 	memcpy(&m_MplMcmsCommonHeader,Mainbuf,sizeof (COMMON_HEADER_S));
// 	offsetFromTheBufferStart+=sizeof (COMMON_HEADER_S);

// 	if (TRACE_TO_LOGGER==getOpcode())
// 	{
// 		ReadLoggerHeader(Mainbuf+offsetFromTheBufferStart);
// 		offsetFromTheBufferStart+=sizeof (COMMON_HEADER_S);
// 	}
// 	else
// 	{
        
//         memcpy(&m_MplMcmsMsgDescriptionHeader,
//                Mainbuf+offsetFromTheBufferStart,
//                sizeof (MESSAGE_DESCRIPTION_HEADER_S));
        
//         int NextHeader_type = m_MplMcmsMsgDescriptionHeader.next_header_type;
        
//         offsetFromTheBufferStart +=(sizeof (MESSAGE_DESCRIPTION_HEADER_S));
        
//         while ((NextHeader_type==eHeaderPhysical) || (NextHeader_type==eHeaderPortDesc))
//         {
            
//             switch (NextHeader_type)
//             {
//                 case eHeaderPhysical:
//                 {    
//                     memcpy(&m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter],
//                            (Mainbuf+offsetFromTheBufferStart),
//                            sizeof(PHYSICAL_INFO_HEADER_S));
                    
//                     offsetFromTheBufferStart+=sizeof(PHYSICAL_INFO_HEADER_S);
                    
//                     NextHeader_type = m_MplMcmsPhysicalInfoHeader[m_PhysicalInfoHeaderCounter].next_header_type;
                    
//                     if (m_PhysicalInfoHeaderCounter <= MAX_PHYSICAL_INFO_HEADERS)
// 					{
// 						m_PhysicalInfoHeaderCounter++;
// 					}
//                     break;
//                 }
//                 case eHeaderPortDesc:
//                 {
//                     memcpy(&m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter],
//                            (Mainbuf+offsetFromTheBufferStart),
//                            sizeof(PORT_DESCRIPTION_HEADER_S));
                    
//                     offsetFromTheBufferStart+=sizeof(PORT_DESCRIPTION_HEADER_S);
                    
//                     NextHeader_type = m_MplMcmsPortDescriptionHeader[m_PortDescriptionHeaderCounter].next_header_type;
// 					if (m_PortDescriptionHeaderCounter<=MAX_PORT_DESCRIPTION_HEADERS)
// 					{
// 						m_PortDescriptionHeaderCounter++;
// 					}
//                     break;
//                 }
//                 default :break;
//             }
            
//         }
//         int Datalen = (MainBufLen+1-offsetFromTheBufferStart);
//         m_pData = new char[Datalen];
//         memcpy(m_pData,(Mainbuf+offsetFromTheBufferStart),Datalen-1);
//         m_DataLen = Datalen;
// 	}
// }

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::ReadLoggerHeader(char* Mainbuf)
{
	LOGGER_HEADER_S *loggerHdr = (LOGGER_HEADER_S *)Mainbuf;

	// PTRACELOGGER(
// 		DEBUG,
// 		"CMplMcmsProtocol::LoggerHeader",
// 		DEFAULT_TOPIC_ID,
// 		loggerHdr->unit_id,
// 		loggerHdr->conf_id,
// 		loggerHdr->party_id,
// 		loggerHdr->opcode,
// 		loggerHdr->str_opcode
// 		);
}
/////////////////////////////////////////////////////////////////////////////
STATUS CMplMcmsProtocol::GetPhysicalInfoHeaderCopy(const int index,PHYSICAL_INFO_HEADER_S* pStruct) const
{
	if( m_PhysicalInfoHeaderCounter && index < m_PhysicalInfoHeaderCounter ) {
		memcpy(pStruct,&(m_MplMcmsPhysicalInfoHeader[index]),sizeof(PHYSICAL_INFO_HEADER_S));
		return STATUS_OK;
	}
	return STATUS_FAIL;
}
/////////////////////////////////////////////////////////////////////////////

/*
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8  CMplMcmsProtocol::getCommonHeaderProtocolVersion() const
{ 
	return m_MplMcmsCommonHeader.protocol_version;

}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU8  CMplMcmsProtocol::getCommonHeaderOption()  const
{ 
	return m_MplMcmsCommonHeader.opcode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU8  CMplMcmsProtocol::getCommonHeaderSrc_id()  const
{ 
	return m_MplMcmsCommonHeader.src_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU8  CMplMcmsProtocol::getCommonHeaderDest_id() const
{ 
	return m_MplMcmsCommonHeader.dest_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////
OPCODE CMplMcmsProtocol::getCommonHeaderOpcode() const
{ 
	return m_MplMcmsCommonHeader.opcode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CMplMcmsProtocol::getCommonHeaderTime_stamp() const
{ 
	return m_MplMcmsCommonHeader.time_stamp;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CMplMcmsProtocol::getCommonHeaderSequence_num() const
{ 
	return m_MplMcmsCommonHeader.sequence_num;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CMplMcmsProtocol::getCommonHeaderPayload_len() const
{ 
	return m_MplMcmsCommonHeader.payload_len;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CMplMcmsProtocol::getCommonHeaderPayload_offset() const
{ 
	return m_MplMcmsCommonHeader.payload_offset;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CMplMcmsProtocol::getCommonHeaderNext_header_type() const
{ 
	return m_MplMcmsCommonHeader.next_header_type;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CMplMcmsProtocol::getCommonHeaderNext_header_size() const
{ 
	return m_MplMcmsCommonHeader.next_header_offset;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getMsgDescriptionHeaderRequest_id()  const
{ 
	return m_MplMcmsMsgDescriptionHeader.request_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getMsgDescriptionHeaderEntity_type() const
{ 
	return m_MplMcmsMsgDescriptionHeader.entity_type;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getMsgDescriptionHeaderTime_stamp() const
{ 
	return m_MplMcmsMsgDescriptionHeader.time_stamp;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getMsgDescriptionHeaderNext_header_type()  const
{ 
	return m_MplMcmsMsgDescriptionHeader.next_header_type;
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getMsgDescriptionHeaderNext_header_size() const
{ 
	return m_MplMcmsMsgDescriptionHeader.next_header_size;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
APIU8  CMplMcmsProtocol ::getPhysicalInfoHeaderBox_id(const int Counter) const
{ 
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].box_id;
	}
	else 
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPhysicalInfoHeaderBoard_id(const int Counter) const
{ 
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].board_id;
	}
	else 
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::SetPhysicalInfoHeaderBoard_id(WORD board_id, const int Counter)
{
	if ( Counter<MAX_PHYSICAL_INFO_HEADERS )
	{
		m_MplMcmsPhysicalInfoHeader[Counter].board_id = board_id;
		if(m_PhysicalInfoHeaderCounter==0){
		  m_PhysicalInfoHeaderCounter++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPhysicalInfoHeaderSub_board_id(const int Counter) const
{ 
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].sub_board_id;
	}
	else 
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPhysicalInfoHeaderUnit_id(const int Counter) const
{
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].unit_id;
	}
	else 
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPhysicalInfoHeaderAccelerator_id(const int Counter) const
{
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].accelerator_id;
	}
	else
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU16   CMplMcmsProtocol::getPhysicalInfoHeaderPort_id(const int Counter) const
{ 
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].port_id;
	}
	else 
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPhysicalInfoHeaderResource_type(const int Counter) const
{ 
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].resource_type;
	}
	else 
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPhysicalInfoHeaderFuture_use1(const int Counter) const
{ 
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].future_use1;
	}
	else 
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPhysicalInfoHeaderFuture_use2(const int Counter) const
{ 
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].future_use2;
	}
	else 
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getPhysicalInfoHeaderNext_header_type(const int Counter) const
{ 
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].next_header_type;
	}
	else 
	{
		return 0xffffffff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getPhysicalInfoHeaderNext_header_size(const int Counter) const
{ 
	if ( Counter<m_PhysicalInfoHeaderCounter )
	{
		return m_MplMcmsPhysicalInfoHeader[Counter].next_header_size;
	}
	else 
	{
		return 0xffffffff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getPortDescriptionHeaderParty_id(const int Counter) const
{ 
	if ( Counter<m_PortDescriptionHeaderCounter )
	{
		return m_MplMcmsPortDescriptionHeader[Counter].party_id;
	}
	else 
	{
		return 0xffffffff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getPortDescriptionHeaderConf_id(const int Counter) const
{ 
	if ( Counter<m_PortDescriptionHeaderCounter )
	{
		return m_MplMcmsPortDescriptionHeader[Counter].conf_id;
	}
	else 
	{
		return 0xffffffff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getPortDescriptionHeaderConnection_id(const int Counter) const
{ 
	if ( Counter<m_PortDescriptionHeaderCounter )
	{
		return m_MplMcmsPortDescriptionHeader[Counter].connection_id;
	}
	else 
	{
		return 0xffffffff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPortDescriptionHeaderLogical_resource_type_1(const int Counter) const
{ 
	if ( Counter<m_PortDescriptionHeaderCounter )
	{
		return m_MplMcmsPortDescriptionHeader[Counter].logical_resource_type_1; 
	}
	else 
	{
		return 0xff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPortDescriptionHeaderLogical_resource_type_2(const int Counter) const
{ 
	if ( Counter<m_PortDescriptionHeaderCounter )
	{
		return m_MplMcmsPortDescriptionHeader[Counter].logical_resource_type_2;
	}
	else 
	{
		return 0xff;
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPortDescriptionHeaderFuture_use1(const int Counter) const
{ 
//	if ( Counter<m_PortDescriptionHeaderCounter )
//	{
//		return m_MplMcmsPortDescriptionHeader[Counter].future_use1;
//	}
//	else
//	{
		return 0xff;
//	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU8   CMplMcmsProtocol::getPortDescriptionHeaderFuture_use2(const int Counter) const
{ 
//	if ( Counter<m_PortDescriptionHeaderCounter )
//	{
//		return m_MplMcmsPortDescriptionHeader[Counter].future_use2;
//	}
//	else
//	{
		return 0xff;
//	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU16   CMplMcmsProtocol::getPortDescriptionHeaderRoom_Id(const int Counter) const
{
	if ( Counter<m_PortDescriptionHeaderCounter )
	{
		return m_MplMcmsPortDescriptionHeader[Counter].room_id;
	}
	else
	{
		return 0xffff;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getPortDescriptionHeaderNext_header_type(const int Counter) const
{ 
	if ( Counter<m_PortDescriptionHeaderCounter )
	{
		return m_MplMcmsPortDescriptionHeader[Counter].next_header_type;
	}
	else 
	{
		return 0xffffffff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
APIU32  CMplMcmsProtocol::getPortDescriptionHeaderNext_header_size(const int Counter) const
{ 
	if ( Counter<m_PortDescriptionHeaderCounter )
	{
		return m_MplMcmsPortDescriptionHeader[Counter].next_header_size;
	}
	else 
	{
		return 0xffffffff;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CMplMcmsProtocol:: getPhysicalInfoHeaderCounter() const
{
	return m_PhysicalInfoHeaderCounter;
}
////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CMplMcmsProtocol:: getPortDescriptionHeaderCounter() const
{
	return m_PortDescriptionHeaderCounter;
}
////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CMplMcmsProtocol:: getDataLen() const
{
	return m_DataLen;
}
////////////////////////////////////////////////////////////////////////////////////////////////
char* CMplMcmsProtocol::getpData()
{
	return m_DataLen ? m_pData : NULL;
}

const char* CMplMcmsProtocol::getpData() const
{
	return m_DataLen ? m_pData : NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CMplMcmsProtocol::getSequence_num() const
{
	return m_sequence_num;
}
////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CMplMcmsProtocol::getCSStatus() const
{
	return m_CS_McmsHeader.status;
}

/////////////////////////////////////////////////////////////////////////////
void CMplMcmsProtocol::AddPayload_len(BOOL Type)
{
	APIU32 payload_len=0;
	
	payload_len=sizeof(COMMON_HEADER_S);
	payload_len+=sizeof(MESSAGE_DESCRIPTION_HEADER_S);
	if (CS_API_TYPE== Type)
	{
		payload_len+=sizeof(CENTRAL_SIGNALING_HEADER_S);
	}
	if( m_PhysicalInfoHeaderCounter ) 
		payload_len+=(sizeof(PHYSICAL_INFO_HEADER_S)*m_PhysicalInfoHeaderCounter);
	
	if( m_PortDescriptionHeaderCounter ) 
		payload_len+=(sizeof(PORT_DESCRIPTION_HEADER_S)*m_PortDescriptionHeaderCounter);

	m_MplMcmsCommonHeader.payload_offset = payload_len;
	
	payload_len += m_DataLen;
	m_MplMcmsCommonHeader.payload_len = payload_len;
	
}

////////////////////////////////////////////////////////////////////
STATUS CMplMcmsProtocol::ValidateDataSize(const DWORD expectedSize, BOOL assertOnGreaterSize) 
{
	const DWORD receivedSize = getDataLen();
	
	// 1. receivedSize == expectedSize: everything is good!
	if ( receivedSize == expectedSize)
	{
		return STATUS_OK;
	}
	
	// 2. receivedSize > expectedSize: so-so... continue working with the message
	STATUS retStatus = STATUS_OK;
	const char *title = "\nReceived Size is Greater than Expected; Message accepted";
	
	// 3. receivedSize < expectedSize: bad!! reject the message
	if (receivedSize < expectedSize)
	{
		retStatus = STATUS_WRONG_DATA_SIZE;
		title = "\nReceived Size is Smaller than Expected; Message rejected";
	}
	if((assertOnGreaterSize && (retStatus == STATUS_OK))|| (retStatus != STATUS_OK)) //In some cases we don't wont to assert in case the receivedSize > expectedSize 
	{
		CMplMcmsProtocolTracer tracer(*this);
		tracer.TraceMplMcmsProtocol(title, MPL_API_TYPE, false);
	
		CProcessBase * process = CProcessBase::GetProcess();
	
		CLargeString buff;
		buff << title															<< '\n'
			 << "Opcode: " << process->GetOpcodeAsString(getOpcode()).c_str()	<< '\n'
	    	 << "Status: " << process->GetStatusAsString(retStatus).c_str()		<< '\n'
		     << "Expected Size : " << expectedSize 								<< '\n'
		     << "Received Size : " << receivedSize 								<< '\n';
		PASSERTMSG(1, buff.GetString());
	}
	return retStatus;
}


/*
////////////////////////////////////////////////////////////////////////////////
APIU16 CMplMcmsProtocol::getCentralSignalingHeaderCsId() const
{
	return m_CS_McmsHeader.cs_id;
}
////////////////////////////////////////////////////////////////////////////////
APIU32 CMplMcmsProtocol::getCentralSignalingHeaderCallIndex() const
{
	return m_CS_McmsHeader.call_index;
}

////////////////////////////////////////////////////////////////////////////////
APIU32 CMplMcmsProtocol::getCentralSignalingHeaderChannelIndex() const
{
	return m_CS_McmsHeader.channel_index;
}

////////////////////////////////////////////////////////////////////////////////
APIU32 CMplMcmsProtocol::getCentralSignalingHeaderMcChannelIndex() const
{
	return m_CS_McmsHeader.mc_channel_index;
}

////////////////////////////////////////////////////////////////////////////////
APIS32 CMplMcmsProtocol::getCentralSignalingHeaderStatus() const
{
	return m_CS_McmsHeader.status;
}

////////////////////////////////////////////////////////////////////////////////
APIU16	CMplMcmsProtocol::getCentralSignalingHeaderSrcUnitId() const
{
	return m_CS_McmsHeader.src_unit_id;
}

////////////////////////////////////////////////////////////////////////////////
APIU16	CMplMcmsProtocol::getCentralSignalingHeaderDestUnitId() const
{
	return m_CS_McmsHeader.dst_unit_id;
}

////////////////////////////////////////////////////////////////////////////////
APIU32	CMplMcmsProtocol::getCentralSignalingHeaderServiceId() const
{
	return m_CS_McmsHeader.service_id;
}
*/
