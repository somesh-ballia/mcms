/////////////////////////////////////////////////////////////////////////////
//                       CNetCalledParty functions
/////////////////////////////////////////////////////////////////////////////
#include "NetCalledParty.h"
#include "Trace.h"
#include "Macros.h"

/////////////////////////////////////////////////////////////////////////////
CNetCalledParty::CNetCalledParty()      // constructor
{
	m_numDigits		= 0;
	m_numType       = 0;
	m_numPlan       = 0;
	m_digits[0]     =  '\0';
}

/////////////////////////////////////////////////////////////////////////////
CNetCalledParty::~CNetCalledParty()     // destructor
{ 
}


/////////////////////////////////////////////////////////////////////////////
    
///////////////////////////////////////////////////////////////////////////// 
void CNetCalledParty::Serialize(WORD format,CSegment& seg) 
{
  switch ( format )  {    
	
    case SERIALEMBD :{

       if (m_numDigits > strlen((char*)m_digits))
          m_numDigits=strlen((char*)m_digits);

	   BYTE nop=0;
       seg << m_numDigits << m_numType << m_numPlan;
       seg.Put(m_digits,PRI_LIMIT_PHONE_DIGITS_LEN);
       seg << nop;			   
       break;        
    }		
    case NATIVE     : {

       BYTE nop=0;
       seg << m_numDigits << m_numType << m_numPlan;
       seg.Put(m_digits,PRI_LIMIT_PHONE_DIGITS_LEN);
       seg << nop;			   
      break;        
    }
		  
    default : {
      break;       
    }                            
	
  } //End switch                                                                     
}

/////////////////////////////////////////////////////////////////////////////
void CNetCalledParty::DeSerialize(WORD format,CSegment& seg)     
{
	WORD digit_cnt=1;

   switch ( format )  {    
      
   case SERIALEMBD :{
      
      BYTE nop;
      seg >> m_numDigits >> m_numType >> m_numPlan ;
      seg.Get(m_digits,PRI_LIMIT_PHONE_DIGITS_LEN); 
      seg >> nop;
      
      //erase suffix of non-digits from m_digits
      int i=0,j;  
      while ((i < m_numDigits) && (i < PRI_LIMIT_PHONE_DIGITS_LEN) && (digit_cnt < PRI_LIMIT_PHONE_DIGITS_LEN))
      {
         if ( !isdigit(m_digits[i]) && (m_digits[i] !='#') ) 
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
      seg >> m_numDigits >> m_numType >> m_numPlan ;
      seg.Get(m_digits,PRI_LIMIT_PHONE_DIGITS_LEN); 
      seg >> nop;
      
      //erase suffix of non-digits from m_digits
      
      int i=0,j;  
      while ((i < m_numDigits) && (i < PRI_LIMIT_PHONE_DIGITS_LEN) && (digit_cnt < PRI_LIMIT_PHONE_DIGITS_LEN))
      {
         if ( !isdigit(m_digits[i]) && (m_digits[i] !='#') ) 
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

/////////////////////////////////////////////////////////////////////////////
// date:10/2001		programer: ron
// remove illigal charecters from number 
// we use this function for dial in isdn calls
// because we changed deserialize to let '#' in dial out calles.

WORD CNetCalledParty::Legalize()
{
	WORD num_of_illigal_char = 0;
	int digit_index=0,fix_array_index,digits_count=0;
	// legalize m_numDigits
	if(m_numDigits>PRI_LIMIT_PHONE_DIGITS_LEN){
		PTRACE(eLevelError,"CNetCalledParty::Legalize : m_numDigits>=PRI_LIMIT_PHONE_DIGITS_LEN");
		m_numDigits=PRI_LIMIT_PHONE_DIGITS_LEN;
		m_digits[m_numDigits]='\0';
	}
	// removing illigal charecters
	while ( (digit_index < m_numDigits) && (digits_count < m_numDigits) )
	{
		if ( !isdigit(m_digits[digit_index]) )  
		{
			if(digit_index == m_numDigits-1){//last digit
				m_digits[digit_index]='\0';
			}else{// removing illigal char from array
				for(fix_array_index=digit_index+1;fix_array_index<m_numDigits;fix_array_index++){
					m_digits[fix_array_index-1]=m_digits[fix_array_index];
				}
				m_digits[fix_array_index-1]='\0';
				digit_index--;	/* in order to restart the test at the place */
								/* where a character was encountered */
			}

			num_of_illigal_char++;
		}
		digits_count++;
		digit_index++;

	}
	m_numDigits-=num_of_illigal_char;
    if(num_of_illigal_char>0){
		ALLOCBUFFER(Mess,ONE_LINE_BUFFER_LEN);
		sprintf(Mess,"num_of_illigal_char = %d  m_numDigits = %d",num_of_illigal_char,m_numDigits);
		PTRACE2(eLevelInfoNormal,"CNetCalledParty::Legalize  ", Mess);
		DEALLOCBUFFER(Mess);
	}

	return num_of_illigal_char;
}
/////////////////////////////////////////////////////////////////////////////
     
void CNetCalledParty::Trace(std::ostream& msg)
{
//  msg << '\n' << "called party: ";
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
//
//  msg << "tel num: ";
//  if (m_numDigits==0) msg << "NO INFO";
//  else {
//  for (BYTE i=0;i<m_numDigits;i++)
//   msg << (BYTE)(m_digits[i]);
//  }
}
