#include "NetCallingParty.h"

CNetCallingParty::CNetCallingParty()    // constructor
{
	m_numDigits = 0;   
	m_numType = 0;
	m_numPlan = 0;
	m_presentationInd = 0; 
	m_screeningInd = 0; 
	m_digits[0] = '\0';

}

/////////////////////////////////////////////////////////////////////////////
CNetCallingParty::~CNetCallingParty()     // destructor
{ 
}

/////////////////////////////////////////////////////////////////////////////

    
/////////////////////////////////////////////////////////////////////////////  
void CNetCallingParty::Serialize(WORD format,CSegment& seg)
{
  switch ( format )  {    
	
    case SERIALEMBD :{

       if (m_numDigits > strlen((char*)m_digits))
          m_numDigits=strlen((char*)m_digits);

	  BYTE nop=0;
      seg << m_numDigits << m_numType << m_numPlan  
          << m_presentationInd << m_screeningInd;
      seg.Put(m_digits,PRI_LIMIT_PHONE_DIGITS_LEN);
      seg << nop << nop << nop;                          			   
      break;  
			}
    case NATIVE     : {

      BYTE nop=0;
      seg << m_numDigits << m_numType << m_numPlan  
          << m_presentationInd << m_screeningInd;
      seg.Put(m_digits,PRI_LIMIT_PHONE_DIGITS_LEN);
      seg << nop << nop << nop;                          			   
      break;       
    }
		   
    default : {
      break;                                   
    }
		 
   } //end switch                                                                            
}  

/////////////////////////////////////////////////////////////////////////////  

void CNetCallingParty::DeSerialize(WORD format,CSegment& seg)
{
	WORD digit_cnt=1;

  switch ( format )  {    
	
    case SERIALEMBD :{
		     BYTE nop;
      seg >> m_numDigits >> m_numType >> m_numPlan  
          >> m_presentationInd >> m_screeningInd;               
      seg.Get(m_digits,PRI_LIMIT_PHONE_DIGITS_LEN);                            
      seg >> nop >> nop >> nop;

      //erase suffix of non-digits from m_digits
      int i=0,j;  
      while ((i < m_numDigits) && (i < PRI_LIMIT_PHONE_DIGITS_LEN) && (digit_cnt < PRI_LIMIT_PHONE_DIGITS_LEN))
			{
           if ( !isdigit(m_digits[i]) ) 
					 {
      				for(j=i+1;j<PRI_LIMIT_PHONE_DIGITS_LEN && m_digits[j] !='\0';j++)
			        		m_digits[j-1]=m_digits[j];
              m_digits[j-1]='\0';
		          i--; /* in order to restart the test at the place */
				           /* where a character was encountered */									
           }
           i++;
		   digit_cnt++;
      }
      
      break;        
    }

    case NATIVE     : {

      BYTE nop;
      seg >> m_numDigits >> m_numType >> m_numPlan  
          >> m_presentationInd >> m_screeningInd;               
      seg.Get(m_digits,PRI_LIMIT_PHONE_DIGITS_LEN);                            
      seg >> nop >> nop >> nop;

      //erase suffix of non-digits from m_digits
      int i=0,j;  
      while ((i < m_numDigits) && (i < PRI_LIMIT_PHONE_DIGITS_LEN)&& (digit_cnt < PRI_LIMIT_PHONE_DIGITS_LEN))
			{
           if ( !isdigit(m_digits[i]) ) 
					 {
       				for(j=i+1;j<PRI_LIMIT_PHONE_DIGITS_LEN && m_digits[j] !='\0';j++)
					         m_digits[j-1]=m_digits[j];
    					m_digits[j-1]='\0';
		          i--; /* in order to restart the test at the place */
				           /* where a character was encountered */
           }
           i++;
		   digit_cnt++;
      }
      break;        
    }
	  default : {
      break;        
    }                           
	
  }  //End switch                                                                     
}


void CNetCallingParty::Trace(std::ostream& msg)
{
//  msg << '\n' << "calling party: ";
//  if (m_numDigits == 0) {
//     msg << "NO INFO " ;
//     return;
//  } 
//
//  msg << "numType = " ;
//  switch ( m_numType ) {  
//      case numtUNKNOWN_VAL  :  {
//	msg << "UNKNOWN " ; 
//	break;
//      }  
//      case numtINTERNATIONAL_VAL        :  {
//	msg << "INTERNATIONAL " ;           
//	break;
//      }  
//      case numtNATIONAL_VAL :  {
//	msg << "NATIONAL " ;  
//	break;
//      }
//      case 3 :  { //NETWORK_SPECIFIC_TYPE
//	msg << " NETWORK SPECIFIC" ;  
//	break;
//      }  
//      case numtSUBSCRIBER_VAl :  {
//	msg << "SUBSCRIBER " ;          
//	break;
//      }
//      case numtABBREVIATED_VAL :  {
//	msg << "ABBREVIATED " ;  
//	break;
//      }  
//      default           :  {
//	msg << "ERROR! " ;
//	break;
//      }
//  }    //end switch
//
//  msg << "numPlan = " ;
//  switch ( m_numPlan ) {  
//      case numpUNKNOWN_VAL  :  {
//	msg << "UNKNOWN " ; 
//	break;
//      }  
//      case numpISDN_VAL :  {
//	msg << "ISDN " ;           
//	break;
//      }  
//      case numpTELEPHONY_VAL :  {
//	msg << "TELEPHONY " ;          
//	break;
//      }
//      case 3 :  { //DATA_PLAN
//	msg << "DATA " ;          
//	break;
//      }
//      case 4 :  { //TELEX_PLAN
//	msg << "TELEX " ;          
//	break;
//      }
//      case numpPRIVATE_VAL :  {
//	msg << "PRIVATE(5) " ;  
//	break;
//      }  
//      case 8 :  {  // numpNATIONAL_VAL
//	msg << "NATIONAL " ;  
//	break;
//      }  
//      case 9 :  {
//	msg << "PRIVATE " ;  
//	break;
//      }  
//      default           :  {
//	msg << "ERROR! " ;
//	break;
//      }
//  }    //end switch
//
//  msg << "presentation = " ;
//  switch ( m_presentationInd ) {  
//      case presALLOWED_VAL  :  {
//	msg << "ALLOWED " ; 
//	break;
//      }  
//      case presRESTRICTED       :  {
//	msg << "RESTRICTED " ;           
//	break;
//      }  
//      case presNUM_NOT_AVAIL_VAL :  {
//	msg << "NOT AVAILABLE " ;          
//	break;
//      }
//      default           :  {
//	msg << "ERROR! " ;
//	break;
//      }
//  }    //end switch
//
//  msg << '\n' << "screening = " ;
//  switch ( m_screeningInd ) {  
//      case scrUSER_NOT_SCREENED_VAL  :  {
//	msg << "USER PROVIDED , NOT SCREENED " ; 
//	break;
//      }  
//      case scrUSER_VER_PASSED_VAL       :  {
//	msg << "USER PROVIDED VERIFIED AND PASSED " ;           
//	break;
//      }  
//      case scrUSER_VER_FAILED_VAL :  {
//	msg << "USER PROVIDED VERIFIED AND FAILED " ;          
//	break;
//      }
//      case scrNETWORK_PROVIDED_VAL :  {
//	msg << "NETWORK PROVIDED " ;          
//	break;
//      }
//      default           :  {
//	msg << "ERROR! " ;
//	break;
//      }
//  }    //end switch 
//  
//  msg << "tel num: ";
//  for (BYTE i=0;i<m_numDigits;i++)
//   msg << (BYTE)(m_digits[i]);
}

