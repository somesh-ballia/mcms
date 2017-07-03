////////////////////////////////////////////////////////////////////////////
//                       CIsdnNetSetup functions
/////////////////////////////////////////////////////////////////////////////
#include "IsdnNetSetup.h"
#include "TraceStream.h"


/////////////////////////////////////////////////////////////////////////////
CIsdnNetSetup::CIsdnNetSetup() :         // constructor
    m_netSpcf(0),
	m_net_connection_id(0),
	m_virtual_port_number(0),
	m_physical_port_number(0),
	m_boardId(0),
	m_box_id(0),m_sub_board_id(0)
{
    for (BYTE i=0; i<MAX_NUM_SPANS_ORDER; i++)
	    m_spanId[i]=0;
}

/////////////////////////////////////////////////////////////////////////////
CIsdnNetSetup::~CIsdnNetSetup()     // destructor
{ 
}

/////////////////////////////////////////////////////////////////////////////
const char* CIsdnNetSetup::NameOf()  const
{
	return "CIsdnNetSetup";
}

/////////////////////////////////////////////////////////////////////////////  
void CIsdnNetSetup::Dump(WORD switchFlag) 
{

//  OPENOSTRSTREAM(msg);  
//  Trace(msg);
//	PTRACE(eLevelInfoNormal,msg.str());
//  CLOSEOSTRSTREAM;
}

/////////////////////////////////////////////////////////////////////////////  
void CIsdnNetSetup::Serialize(WORD format,CSegment& seg) 
{ 
	switch ( format )  {    
	
	case SERIALEMBD :
	case NATIVE     : {

	    BYTE nop = 0;
	    seg << m_netSpcf << nop << nop << m_callType;
	    m_calling.Serialize(format,seg);
	    m_called.Serialize(format,seg);
	    //Serialize the header
		for (BYTE i=0; i<MAX_NUM_SPANS_ORDER; i++)
		  seg << m_spanId[i];
	    seg << m_net_connection_id;
	    seg << m_virtual_port_number;
	    seg << m_physical_port_number;
	    seg << m_boardId;
	    seg << m_box_id;
	    seg << m_sub_board_id;
		  break;        
	}
		   
	default :
	  break;                                   
		 
	}       //End switch                                                                     
}

/////////////////////////////////////////////////////////////////////////////
void CIsdnNetSetup::DeSerialize(WORD format,CSegment& seg)     
{    
	switch ( format )  {    
	
	case SERIALEMBD :

		case NATIVE : {       
		  BYTE nop;
		  seg >> m_netSpcf >> nop >> nop >> m_callType;
		  m_calling.DeSerialize(format , seg);
		  m_called.DeSerialize(format , seg);
		  for (BYTE i=0; i<MAX_NUM_SPANS_ORDER; i++)
			seg >> m_spanId[i];
		  seg >> m_net_connection_id;
		  seg >> m_virtual_port_number;
		  seg >> m_physical_port_number;
		  seg >> m_boardId;
		  seg >> m_box_id;
		  seg >> m_sub_board_id;
		  break;
		}		   
	    default :
		  break;

	}       //End switch    
}
/////////////////////////////////////////////////////////////////////////////
void CIsdnNetSetup::copy(const CNetSetup *rhs)
{
  const CIsdnNetSetup * pOtherNet = dynamic_cast<const CIsdnNetSetup *>(rhs);
  if (pOtherNet == NULL)
    {
      TRACESTR (eLevelError) << "CIsdnNetSetup::copy Dunamic Cast Failed!!";
      PASSERT(1);
      return;
    }
  
  m_netSpcf = pOtherNet->m_netSpcf;
  m_callType = pOtherNet->m_callType;
  m_called = pOtherNet->m_called;
  m_calling = pOtherNet->m_calling;

  for (BYTE i=0; i<MAX_NUM_SPANS_ORDER; i++)
      m_spanId[i]=pOtherNet->m_spanId[i];
  m_net_connection_id=pOtherNet->m_net_connection_id;
  m_virtual_port_number=pOtherNet->m_virtual_port_number;
  m_physical_port_number=pOtherNet->m_physical_port_number;

  m_boardId = pOtherNet->m_boardId;
  m_box_id =  pOtherNet->m_box_id;
  m_sub_board_id = pOtherNet->m_sub_board_id;
}
/////////////////////////////////////////////////////////////////////////////
CIsdnNetSetup&  CIsdnNetSetup::operator=(const CIsdnNetSetup &otherNet)
{  
  m_netSpcf = otherNet.m_netSpcf;
  m_callType = otherNet. m_callType;
  m_called = otherNet.m_called;
  m_calling = otherNet.m_calling;

  for (BYTE i=0; i<MAX_NUM_SPANS_ORDER; i++)
	m_spanId[i]=otherNet.m_spanId[i];

  m_net_connection_id=otherNet.m_net_connection_id;
  m_virtual_port_number=otherNet.m_virtual_port_number;
  m_physical_port_number=otherNet.m_physical_port_number;

  m_boardId = otherNet.m_boardId;
  m_box_id = otherNet.m_box_id;
  m_sub_board_id = otherNet.m_sub_board_id;
 
  return *this;
}

void CIsdnNetSetup::Trace(std::ostream& msg)
{
//  msg << '\n' ;
//
//  msg << "prfMode = " ;
//  switch ( m_prfMode ) {  
//      case prfNONE_VAL  :  {
//	msg << "NONE " ; 
//	break;
//      }  
//      case prfPREFERRED_VAL     :  {
//	msg << "PREFERRED " ;           
//	break;
//      }  
//      case prfEXCLUSUVE_VAL :  {
//	msg << "EXCLUSIVE " ;  
//	break;
//      }
//      default           :  {
//	msg << "ERROR! " ;
//	break;
//      }
//  }    //end switch
//
//  msg << "netSpcf = " ;
//  switch ( m_netSpcf ) {  
//      case PRInsNULL  :  {
//	      msg << "DEFAULT " ; 
//	      break;
//      }  
//      case PRInsATT_SDN        :  {
//	      msg << "AT&T Software Defined Network  or  Northern Tel Private Net " ;           
//	      break;
//      }  
//      case PRInsATT_MEGACOM800        :  {
//	      msg << "AT&T Megacom 800 service  or  Northern Tel InWats " ;           
//	      break;
//      }  
//      case PRInsATT_MEGACOM :  {
//	      msg << "AT&T Megacom  or  Northern Tel OutWats ";  
//	      break;
//      }
//      case PRInsNTI_FX :  {
//	      msg << "Northern Tel Foreign Exchange  " ;  
//	      break;
//      }
//      case PRInsNTI_TIE_TRUNK :  {
//	      msg << "Northern Tel Tie Trunk " ;  
//	      break;
//      }
//      case PRInsATT_ACCUNET :  {
//	      msg << "AT&T Accunet " ;  
//	      break;
//      }
//      case PRInsATT_I800 :  {
//	      msg << "AT&T International 800  " ;  
//	      break;
//      }
//      case PRInsNTI_TRO :  {
//	      msg << "Northern Tel TRO call " ;  
//	      break;
//      }
//      default           :  {
//	      msg << "ERROR! " ;
//	      break;
//      }
//  }    //end switch
//
//  msg << "callType = " ;
//  switch ( m_callType ) {  
//      case calltypVOICE_VAL  :  {
//	msg << "VOICE " ; 
//	break;
//      }  
//      case calltypMODEM_VAL     :  {
//	msg << "MODEM " ;           
//	break;
//      }  
//      case calltyp64K_VAL :  {
//	msg << "64K " ;  
//	break;
//      }
//      case calltyp64K_REST_VAL   :  {
//	msg << "64K RESTRICT " ;  
//	break;
//      }
//      case calltyp384K_VAL :  {
//	msg << "384K " ;  
//	break;
//      }
//      case calltyp384K_REST_VAL :  {
//	msg << "384K RESTRICT " ;  
//	break;
//      }
//      case calltyp56K_VAL :  {
//	msg << "56K " ;  
//	break;
//      }
//      case calltyp56K_UNREST :  {
//	msg << "56K_UNREST " ;  
//	break;
//      }
//      case calltypALAW_VOICE :  {
//	msg << "ALAW_VOICE " ;  
//	break;
//      }
//      case calltypALAW_MODEM :  {
//	msg << "ALAW_MODEM " ;  
//	break;
//      }
//      default           :  {
//	msg << "UNKNOWN" ;
//	break;
//      }
//  }    //end switch
//
//
//  m_calling.Trace(msg);
//  m_called.Trace(msg);
}


/////////////////////////////////////////////////////////////////////////////  
void  CIsdnNetSetup::GetCalledNumber(WORD *numDigits,char* pTelNumber)
{
	*numDigits  = (WORD)m_called.m_numDigits;

	if( *numDigits > PRI_LIMIT_PHONE_DIGITS_LEN )
		*numDigits = PRI_LIMIT_PHONE_DIGITS_LEN;

	memcpy( pTelNumber,
			m_called.m_digits,
			*numDigits );
	
	*(pTelNumber + *numDigits ) = '\0';
}

/////////////////////////////////////////////////////////////////////////////
void  CIsdnNetSetup::SetCalledNumber(WORD numDigits,const char* pTelNumber)
{
	if (numDigits > PRI_LIMIT_PHONE_DIGITS_LEN)
		numDigits = PRI_LIMIT_PHONE_DIGITS_LEN;
	
	m_called.m_numDigits = numDigits;
  
	memcpy( m_called.m_digits,
			pTelNumber,
			(WORD)m_called.m_numDigits );

	*(m_called.m_digits + numDigits) = '\0';

}

/////////////////////////////////////////////////////////////////////////////
void  CIsdnNetSetup::SetCallingNumber(WORD numDigits,const char* pTelNumber)
{
	if (numDigits > PRI_LIMIT_PHONE_DIGITS_LEN)
		numDigits = PRI_LIMIT_PHONE_DIGITS_LEN;
	
	m_calling.m_numDigits = numDigits;
  
	memcpy( m_calling.m_digits,
			pTelNumber,
			(WORD)m_calling.m_numDigits );

	*(m_calling.m_digits + numDigits) = '\0';

}
/////////////////////////////////////////////////////////////////////////////
void  CIsdnNetSetup::ResetCallingNumber()
{
	m_calling.m_numDigits = 0;
	m_calling.m_digits[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
WORD CIsdnNetSetup::LegalizeCalledNumber()
{
	WORD num_err = 0;
	num_err = (m_called.Legalize());
	return num_err;
		
}
/////////////////////////////////////////////////////////////////////////////
void  CIsdnNetSetup::GetCallingNumber(WORD *numDigits,char* pTelNumber)
{
	
	*numDigits  = (WORD)m_calling.m_numDigits;
	
	if( *numDigits > PRI_LIMIT_PHONE_DIGITS_LEN )
		*numDigits = PRI_LIMIT_PHONE_DIGITS_LEN;

	memcpy( pTelNumber,
			m_calling.m_digits,
			*numDigits );
	
	*(pTelNumber + *numDigits ) = '\0';
}


void CIsdnNetSetup::SetNetCommnHeaderParams(NET_COMMON_PARAM_S& headerParams)
{
  m_spanId[0]=headerParams.span_id;//olga - spanID that was got from CLobby should be saved in the first array element
  m_net_connection_id=headerParams.net_connection_id;
  m_virtual_port_number=headerParams.virtual_port_number;
  m_physical_port_number=headerParams.physical_port_number;
}
