/////////////////////////////////////////////////////////////////////////////
//                       CNetCause functions
/////////////////////////////////////////////////////////////////////////////
#include "NetCause.h"
#include "Q931Structs.h"

/////////////////////////////////////////////////////////////////////////////
CNetCause::CNetCause()          // constructor
  :m_causeVal(causDEFAULT_VAL)
{
	//Default initialization
	//m_codingStandard = codCCITT_VAL;
	//m_location = locUSER_VAL;
}


/////////////////////////////////////////////////////////////////////////////
CNetCause::~CNetCause()     // destructor
{        
}


/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
void CNetCause::Serialize(WORD format,CSegment& seg)
{    
	switch ( format )  {    
	
     case SERIALEMBD :
	   case NATIVE     : {

	       BYTE nop = 0;
	       seg << m_codingStandard << m_location << m_causeVal << nop;
			   
		   break;       
		 }
		   
	   default : {
			   break;       
		 }                            
	
	}                                                                            
}

/////////////////////////////////////////////////////////////////////////////
void CNetCause::DeSerialize(WORD format,CSegment& seg) 
{
	switch ( format )  {    
	
     case SERIALEMBD :
	   case NATIVE     : {

	       BYTE nop;
	       seg >> m_codingStandard >> m_location >> m_causeVal >> nop;
			   
		   break;       
		 }
		   
	   default : {
			   break;         
		 }                          
	
	}
}

void CNetCause::Trace(std::ostream& msg)

{
//  extern const char* GetQ931CauseAsString(const int status);
//  msg << '\n' << "cause: ";
//
//  msg << "cause = " ;
//  msg << (WORD) m_causeVal << " : " ;
//  msg << GetQ931CauseAsString(m_causeVal) << "\n"  ;
//
//  msg << "coding = " ;
//  switch ( m_codingStandard) {  
//      case codCCITT_VAL  :  {
//        msg << "CCITT " ; 
//        break;
//      }  
//      case codNATIONAL_STD_VAL  :  {
//        msg << "NATIONAL " ; 
//        break;
//      }  
//      case codSTD_SPF_2_LOC_VAL  :  {
//        msg << "SPECIFIC TO LOCATION " ; 
//        break;
//      }  
//      default           :  {
//        msg << "ERROR! ";
//        break;
//      }
//  }    //end switch
//
//  msg << "location = " ;
//  switch ( m_location) {  
//      case locUSER_VAL  :  {
//        msg << "USER " ; 
//        break;
//      }  
//      case locPVT_LOCAL_VAL  :  {
//        msg << "LOCAL PRIVATE NET  " ; 
//        break;
//      }  
//      case locPUB_LOCAL_VAL  :  {
//        msg << "LOCAL PUBLIC NET " ; 
//        break;
//      } 
//      case locTRANSIT_NET_VAL  :  {
//        msg << "TRANSIT NET " ; 
//        break;
//      }  
//      case locPUB_REMOTE_VAL  :  {
//        msg << "REMOTE PUBLIC NET " ; 
//        break;
//      }  
//      case locPVT_REMOTE_VAL  :  {
//        msg << "REMOTE PRIVATE NET " ; 
//        break;
//      }  
//      case locINTERNATIONAL_VAL  :  {
//        msg << "INTERNATIONAL NET " ; 
//        break;
//      }  
//      case locBEY_INTERWORK_VAL  :  {
//	      msg << "NETWORK BEYOND THE INTERWORKING POINT " ; 
//	      break;
//      }   
//      default           :  {
//        msg << "ERROR! ";
//        break;
//      }
//  }    //end switch
//	
}

