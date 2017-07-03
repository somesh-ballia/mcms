
#include "BondingPhoneNumbers.h"
#include "Segment.h"
#include <sstream>
using namespace std;

//==============================================================================================================//
// BondingPhoneNumber implementation
//==============================================================================================================//
const char*   BondingPhoneNumber::NameOf() const
{
  return "BondingPhoneNumber";
}
//==============================================================================================================//
BondingPhoneNumber::BondingPhoneNumber()
{
  // reset data member to \0
  memset(m_digits,'\0',BND_MAX_PHONE_LEN+1);
}
//==============================================================================================================//
BondingPhoneNumber::BondingPhoneNumber(char* phone_number)
{
  // reset data member to \0
  memset(m_digits,'\0',BND_MAX_PHONE_LEN+1);
  // copy digits
  if(phone_number != NULL){
    for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++){
      m_digits[digit_index] = phone_number[digit_index];
    }
  }
}

//==============================================================================================================//
BondingPhoneNumber::BondingPhoneNumber(const BondingPhoneNumber& other)
        :CPObject(other)
{
    // reset data member to \0
    memset(m_digits,'\0',BND_MAX_PHONE_LEN+1);
    // copy digits
    for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++){
        m_digits[digit_index] = other.m_digits[digit_index];
    }

}
//==============================================================================================================//
BondingPhoneNumber& BondingPhoneNumber::operator=(const BondingPhoneNumber& other)
{
    // reset data member to \0
    memset(m_digits,'\0',BND_MAX_PHONE_LEN+1);
    // copy digits
    for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++){
        m_digits[digit_index] = other.m_digits[digit_index];
    }
    return *this;
}
//==============================================================================================================//
BondingPhoneNumber::~BondingPhoneNumber()
{
}
//==============================================================================================================//
WORD BondingPhoneNumber::CopyToBuffer(char* Buffer)const
{
    WORD num_of_digits_copied = 0;
    BYTE found_end_of_string = 0;
    for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++){
        if((BYTE)m_digits[digit_index]!=(BYTE)'\f' && !found_end_of_string){
            Buffer[digit_index] = (BYTE)m_digits[digit_index];
            num_of_digits_copied++;
        }else{
            found_end_of_string = 1;
            Buffer[digit_index] =(BYTE)'\f';
        }            
    }

    return num_of_digits_copied;
}
//==============================================================================================================//
void BondingPhoneNumber::Dump(ostringstream& str)const
{
    //char m_digits[BND_MAX_PHONE_LEN+1];
    for(int i=0;i<BND_MAX_PHONE_LEN;i++)
    {
        str << m_digits[i];
    }
    
    //str << m_digits;    
}
//==============================================================================================================//
void BondingPhoneNumber::Serialize(WORD format, CSegment &seg)
{
	switch (format)  
	{    
		case SERIALEMBD :       		   
		case NATIVE  :
        {
            for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++){
                seg << (BYTE) m_digits[digit_index];
            }
            break;
            
        }
    }
}

//==============================================================================================================//
void BondingPhoneNumber::Deserialize(WORD format, CSegment &seg)
{
	switch (format)  
	{    
		case SERIALEMBD :       		   
		case NATIVE  :
        {
            BYTE tmp_digit;
            for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++){
                seg >> tmp_digit;
                m_digits[digit_index] = tmp_digit;
            }
            break;
            
        }
    }
}
//==============================================================================================================//
void BondingPhoneNumber::String2Emb()
{
    for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN+1;digit_index++)
    {
        int string_end = 0;
        
        if(m_digits[digit_index]=='\0' || string_end)
        {
          m_digits[digit_index]=BND_EON;
          string_end=1;
        }else{
          switch(m_digits[digit_index]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':{
                BYTE emb_digit = (BYTE) (m_digits[digit_index] - '0'); // convert '1' to (BYTE)1  
                m_digits[digit_index] = emb_digit; // replace string digit
                break;
					 }
            case '*':{  m_digits[digit_index]=BND_STAR; break;}
            case '#':{  m_digits[digit_index]=BND_DIEZ; break;}
            case 'E':{  m_digits[digit_index]=BND_EON;  break;}
            case 'P':{  m_digits[digit_index]=BND_PAD;  break;}
          
            default:{   m_digits[digit_index]='?'; break;}
          }// switch
        }// else
    }// for

    //   PTRACE2(eLevelInfoNormal,"BondingPhoneNumber::String2Emb after: ",m_digits);
 }
//==============================================================================================================//
void BondingPhoneNumber::Emb2String()
{
    //   PTRACE2(eLevelInfoNormal,"BondingPhoneNumber::Emb2String before: ",m_digits);

    for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++)
    {
        
        int string_end = 0;
        
        if(m_digits[digit_index]==BND_EON || string_end)
        {
          m_digits[digit_index]='\0';
          string_end=1;
        }else{
            
        
          switch(m_digits[digit_index]) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
              case 9:
              {
                  
                BYTE str_digit = (char) (m_digits[digit_index] + '0'); // convert 1 to '1'
                m_digits[digit_index] = str_digit; // replace string digit
              break;
              }
              
            
            case BND_STAR :  m_digits[digit_index] = '*'; break;
            case BND_DIEZ :  m_digits[digit_index] = '#'; break;
            case BND_EON  :{
                m_digits[digit_index] = '\0';
                string_end=1;
                break;
            }
                
            case BND_PAD  :  m_digits[digit_index] = 'P'; break;
          
            default:         m_digits[digit_index] = '?'; break;
          }// switch
        }// else
    }// for
    
        
    m_digits[BND_MAX_PHONE_LEN] = '\0';
 }
//==============================================================================================================//


//==============================================================================================================//
// BondingPhoneNumbersList implementation
//==============================================================================================================//
const char*   BondingPhoneNumbersList::NameOf() const
{
  return "BondingPhoneNumbersList";
}
//==============================================================================================================//
BondingPhoneNumbersList::BondingPhoneNumbersList()
{
}
//==============================================================================================================//
BondingPhoneNumbersList::~BondingPhoneNumbersList()
{
/*rons - VALGRIND
   std::vector< BondingPhoneNumber*>::iterator it = m_phone_numbers_vector.begin();
   std::vector< BondingPhoneNumber*>::iterator itEnd = m_phone_numbers_vector.end();
   
	for( it; it != itEnd ; ++it)
	{
	//	BondingPhoneNumber* pBondingPhoneNumber = *it;		
	//	POBJDELETE(pBondingPhoneNumber);
		if (CPObject::IsValidPObjectPtr(*it))
			POBJDELETE(*it);		
	}
	m_phone_numbers_vector.clear();	
//	PDELETE(m_phone_numbers_vector);	
//rons - VALGRIND	*/
}
//==============================================================================================================//
void BondingPhoneNumbersList::CleanPhoneList()
{
//rons - VALGRIND
   std::vector< BondingPhoneNumber*>::iterator it = m_phone_numbers_vector.begin();
   std::vector< BondingPhoneNumber*>::iterator itEnd = m_phone_numbers_vector.end();
   
	for( ; it != itEnd ; ++it)
	{
		if (CPObject::IsValidPObjectPtr(*it))
			POBJDELETE(*it);		
	}
	m_phone_numbers_vector.clear();	

//rons - VALGRIND	*/
}
//==============================================================================================================//
DWORD BondingPhoneNumbersList::AddPhoneNumber(char* phone_number,int emb2str)
{
  BondingPhoneNumber* pBondingPhoneNumber = new  BondingPhoneNumber(phone_number);
  if(emb2str)
  {
      pBondingPhoneNumber->Emb2String();
  }
  m_phone_numbers_vector.push_back(pBondingPhoneNumber);
  return m_phone_numbers_vector.size();
}
//==============================================================================================================//
void BondingPhoneNumbersList::Dump(ostringstream& str)const
{
    std::vector< BondingPhoneNumber*>::size_type sz = m_phone_numbers_vector.size();

    for(WORD phone_index = 0; phone_index < sz ; phone_index++)
    {
        m_phone_numbers_vector[phone_index]->Dump(str);
        str << "\n";
    }
}
//==============================================================================================================//

DWORD BondingPhoneNumbersList::GetNumOfPhoneNumbers()const
{
    std::vector< BondingPhoneNumber*>::size_type sz = m_phone_numbers_vector.size();
    return sz;
}
//==============================================================================================================//

BondingPhoneNumber* BondingPhoneNumbersList::GetFirstOfPhoneNumbers()const
{
      std::vector< BondingPhoneNumber*>::size_type sz = m_phone_numbers_vector.size();

      if(sz>0)
      {
          return m_phone_numbers_vector[0];
      }
      else{
          return NULL;
          
      }
}
//==============================================================================================================//

void BondingPhoneNumbersList::Serialize(WORD format, CSegment &seg)
{
	switch (format)  
	{    
		case SERIALEMBD :       		   
		case NATIVE  :
        {
            DWORD numOfPhoneNumbers = GetNumOfPhoneNumbers();
            seg << numOfPhoneNumbers;
            for(WORD phone_index = 0; phone_index <numOfPhoneNumbers ; phone_index++)
            {
                m_phone_numbers_vector[phone_index]->Serialize(format,seg);
                
            }
            break;
            
        }
    }
}
//==============================================================================================================//

void BondingPhoneNumbersList::Deserialize(WORD format, CSegment &seg)
{
	switch (format)  
	{    
		case SERIALEMBD :       		   
		case NATIVE  :
        {
            DWORD numOfPhoneNumbers;
            seg >> numOfPhoneNumbers;
            
            for(WORD phone_index = 0; phone_index <numOfPhoneNumbers ; phone_index++)
            {
                BondingPhoneNumber* pPhoneNumber = new BondingPhoneNumber();
                pPhoneNumber->Deserialize(format,seg);
                m_phone_numbers_vector.push_back(pPhoneNumber);
            }
            break;
            
        }
    }
}
//==============================================================================================================//
