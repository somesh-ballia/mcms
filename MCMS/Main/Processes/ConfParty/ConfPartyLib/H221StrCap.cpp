// CCH221StrCap.cpp: implementation of the CH323StrCap class.
//
//////////////////////////////////////////////////////////////////////

#include "Macros.h"
#include "Segment.h"
#include "CDRUtils.h"
#include "H221StrCap.h"


//cap dump of H221 BAS capabilits vector
void CH221strCap::Dump(std::ostream& ostr)
{
    CCDRUtils::DumpCap(m_pStr,m_size,ostr);
}

/////////////////////////////////////////////////////////////////////////////

void CH221strCapDrv::Serialize(WORD format, CSegment& Seg)
{                        
	switch ( format )  {    
	
	case SERIALEMBD: {  
	  for (WORD i=0 ; i < m_size ; i++)
		Seg <<  m_pStr[i];
	} break;
	case NATIVE: {
	  Seg << m_size; 
	  for (WORD i=0 ; i < m_size ; i++)
		Seg <<  m_pStr[i];
	} break;           
	default :
	  break;                             
	}	
}

void CH221strCapDrv::DeSerialize(WORD format,CSegment& Seg)
{
    BYTE dummy;
    CSegment copy_of_seg(Seg);//used to find length of segment
	                                       
	switch ( format )  {    
	
	case SERIALEMBD:
	  {       		   
		if (m_pStr != NULL)
		    PDELETE(m_pStr);

		m_pStr = NULL;
		m_size = 0;
		while ( ! copy_of_seg.EndOfSegment() )  {
		    copy_of_seg >> dummy;
			m_size++;
		}

		if (m_size > 0) {
		    m_pStr = new BYTE[m_size];
			PASSERT(m_pStr == NULL);
		
			for (WORD i=0 ; i < m_size ; i++)
			    Seg >> m_pStr[i];
		}
	  }
	  break;
	
	case NATIVE:
	  {
		Seg >> m_size; 

		if (m_pStr != NULL)
		  PDELETE(m_pStr);

		m_pStr = NULL;
		if (m_size > 0) {
		    m_pStr = new BYTE[m_size];
			PASSERT(m_pStr == NULL);
		
			for (WORD i=0 ; i < m_size ; i++)
			  Seg >> m_pStr[i];
		}
	  }
	  break; 
	default :
	  break;
	}                                                                 
}

/////////////////////////////////////////////////////////////////////////////        

//comm mode dump of H221 BAS commands vector
void CH221strCom::Dump(std::ostream& ostr)
{
    CCDRUtils::DumpH221Stream(ostr, m_size,m_pStr);
}

/////////////////////////////////////////////////////////////////////////////        
void CH221strComDrv::Serialize(WORD format,CSegment& Seg)
{                        
    WORD    i;
 
	switch ( format )  {	
	case SERIALEMBD:
	  {  
	    for ( i=0 ; i < m_size ; i++)
		    Seg <<  m_pStr[i];
		break; 
	  }
	case NATIVE:
	  {
		Seg << m_size; 
            
		for ( i=0 ; i < m_size ; i++)
		    Seg <<  m_pStr[i];
		break; 
	  }
	default :
	  break;                          
	}	
}

/////////////////////////////////////////////////////////////////////////////
void  CH221strComDrv::DeSerialize(WORD format,CSegment& Seg)
{
	//BYTE 	bas;
    WORD    i;
    BYTE dummy;
    CSegment copy_of_seg(Seg);

	                                       
	switch ( format )  {    
	
	case SERIALEMBD:
	  {       		   
		if (m_pStr != NULL)
		    PDELETEA(m_pStr);
		m_pStr = NULL;
		m_size = 0;
		while ( ! copy_of_seg.EndOfSegment() )  {
		    copy_of_seg >> dummy;
			m_size++;
		}
		if (m_size > 0) {
		    m_pStr = new BYTE[m_size];
			PASSERT(m_pStr == NULL);
            
			for ( i=0 ; i < m_size ; i++)
			  Seg >> m_pStr[i];
		}
		break;
	  }
	case NATIVE:
	  {
		  Seg >> m_size; 

		  if (m_pStr != NULL)
			PDELETE(m_pStr);
		  m_pStr = NULL;
		  if (m_size > 0) {
			  m_pStr = new BYTE[m_size];
			  PASSERT(m_pStr == NULL);
            
			  for ( i=0 ; i < m_size ; i++)
				  Seg >> m_pStr[i];
		  }
		  break; 
	  }
	default :
	    break;
	}	                                                                     
}
