// CCardMngrLoaded.cpp: implementation of the CCCardMngrLoaded class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "CardMngrLoaded.h"
#include "ObjString.h"
#include "ProcessBase.h"

extern char* CardTypeToString(APIU32 cardType);
extern char* CardUnitPhysicalTypeToString(APIU32 unitPhysicalType);
extern char* CardUnitLoadedStatusToString(APIU32 unitLoadedStatus);

// ------------------------------------------------------------
CCardMngrLoaded::CCardMngrLoaded ()
{
}


// ------------------------------------------------------------
CCardMngrLoaded::CCardMngrLoaded (const CM_CARD_MNGR_LOADED_S* cardMngr)
{
	memcpy(&m_cardMngrLoadedStruct, cardMngr, sizeof(CM_CARD_MNGR_LOADED_S));
}


// ------------------------------------------------------------
CCardMngrLoaded::~CCardMngrLoaded ()
{
}


// ------------------------------------------------------------
void  CCardMngrLoaded::Dump(ostream& msg) const
{
	int i=0;
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n\n"
		<< "CardMngrLoaded::Dump\n"
		<< "--------------------\n";


	char* unitLoadedStatusTypeStr = ::CardUnitLoadedStatusToString(m_cardMngrLoadedStruct.status);
	char* cardTypeStr             = ::CardTypeToString(m_cardMngrLoadedStruct.cardType);


	msg	<< setw(20) << "Serial Number: " << m_cardMngrLoadedStruct.serialNum << "\n";
	
	if (unitLoadedStatusTypeStr)
	{
		msg << setw(20) << "Status: " << unitLoadedStatusTypeStr << "\n";
	}
	else
	{
		msg << setw(20) << "Status: (invalid: " << m_cardMngrLoadedStruct.status << ")\n";
	}
	
	if (cardTypeStr)
	{
		msg << setw(20) << "Type: " << cardTypeStr << "\n";
	}
	else
	{
		msg << setw(20) << "Type: (invalid: " << m_cardMngrLoadedStruct.cardType << ")\n";
	}
	
	
	msg	<< "\n\n"
		<< setw(20) << "Post Results: " << "\n"
		            << "------------"   << "\n";

	for (i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		char* unitLoadedStatusStr = ::CardUnitLoadedStatusToString( m_cardMngrLoadedStruct.postResultsList[i] );
		
		if (unitLoadedStatusStr)
		{
			msg << "unit " << i << ": " << unitLoadedStatusStr << "\n";
		}
		else
		{
			msg << "unit " << i << ": (invalid:" << m_cardMngrLoadedStruct.postResultsList[i] << ")\n";
		}
	}


	msg	<< "\n\n"
		<< setw(20) << "Units Types: " << "\n"
		            << "------------"  << "\n";
		            
	for (i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		char* cardUnitPhysicalTypeStr = ::CardUnitPhysicalTypeToString( m_cardMngrLoadedStruct.unitsTypesList[i] );
		
		if (cardUnitPhysicalTypeStr)
		{
			msg << "unit " << i << ": " << cardUnitPhysicalTypeStr << "\n";
		}
		else
		{
			msg << "unit " << i << ": (invalid: " << m_cardMngrLoadedStruct.unitsTypesList[i] << ")\n";
		}
	}

	
	msg	<< "\n"
		<< setw(20) << "HW Version: "    
		<< m_cardMngrLoadedStruct.hardwareVersion.ver_major    << "."
		<< m_cardMngrLoadedStruct.hardwareVersion.ver_minor    << "."
		<< m_cardMngrLoadedStruct.hardwareVersion.ver_release  << "."
		<< m_cardMngrLoadedStruct.hardwareVersion.ver_internal << "\n\n";
	


	msg	<< "\n"
		<< setw(20) << "SW Versions: " << "\n"
		            << "------------"  << "\n";
	for (i=0; i<MAX_NUM_OF_SW_VERSIONS; i++)
	{
		msg << setw(20) << "Version Desc: "
			            << m_cardMngrLoadedStruct.swVersionsList[i].versionDescriptor << "\n"
		    << setw(20) << "Version Number: "
			            << m_cardMngrLoadedStruct.swVersionsList[i].versionNumber     << "\n\n";
	}

	msg << "\n\n";
}


// ------------------------------------------------------------
CCardMngrLoaded& CCardMngrLoaded::operator = (const CCardMngrLoaded &other)
{
	memcpy( &m_cardMngrLoadedStruct,
		    &(other.m_cardMngrLoadedStruct),
			sizeof(CM_CARD_MNGR_LOADED_S) );

	return *this;
}


// ------------------------------------------------------------
CM_CARD_MNGR_LOADED_S  CCardMngrLoaded::GetCardMngrLoadedStruct()
{
	return m_cardMngrLoadedStruct;
}


// ------------------------------------------------------------
char* CCardMngrLoaded::GetSerialNumber ()
{
	return (char*)(m_cardMngrLoadedStruct.serialNum);
}


// ------------------------------------------------------------
void CCardMngrLoaded::SetSerialNumber (const char* theNum)
{
	memcpy( m_cardMngrLoadedStruct.serialNum,
	        theNum,
	        MPL_SERIAL_NUM_LEN );
}


// ------------------------------------------------------------
DWORD CCardMngrLoaded::GetStatus ()
{
	return m_cardMngrLoadedStruct.status;
}

	
// ------------------------------------------------------------
void CCardMngrLoaded::SetStatus (const DWORD status)
{
	m_cardMngrLoadedStruct.status = status;
}


// ------------------------------------------------------------
eCardType CCardMngrLoaded::GetType ()
{
	return (eCardType)(m_cardMngrLoadedStruct.cardType);
}


// ------------------------------------------------------------
void CCardMngrLoaded::SetType (const eCardType type)
{
	m_cardMngrLoadedStruct.cardType = type;
}


// ------------------------------------------------------------
BYTE* CCardMngrLoaded::GetPostResultsList ()
{
	return m_cardMngrLoadedStruct.postResultsList;
}


// ------------------------------------------------------------
void CCardMngrLoaded::SetPostResultsList (const BYTE *postResults)
{
	memcpy(&m_cardMngrLoadedStruct.postResultsList, postResults, MAX_NUM_OF_UNITS);
}


// ------------------------------------------------------------
BYTE* CCardMngrLoaded::GetUnitsTypesList ()
{
	return m_cardMngrLoadedStruct.unitsTypesList;
}


// ------------------------------------------------------------
void CCardMngrLoaded::SetUnitsTypesList (const BYTE *unitsTypes)
{
	memcpy(&m_cardMngrLoadedStruct.unitsTypesList, unitsTypes, MAX_NUM_OF_UNITS);
}


// ------------------------------------------------------------
VERSION_S CCardMngrLoaded::GetHardwareVersion ()
{
	return m_cardMngrLoadedStruct.hardwareVersion;
}


// ------------------------------------------------------------
void CCardMngrLoaded::SetHardwareVersion (const VERSION_S hwVer)
{
	m_cardMngrLoadedStruct.hardwareVersion = hwVer;
}

// ------------------------------------------------------------
CM_SW_VERSION_S* CCardMngrLoaded::GetSwVersionsList()
{
	return m_cardMngrLoadedStruct.swVersionsList;
}


// ------------------------------------------------------------
void CCardMngrLoaded::SetSwVersionsList(CM_SW_VERSION_S* swVerList)
{
	memcpy( &m_cardMngrLoadedStruct.swVersionsList,
		    swVerList,
			(sizeof(CM_SW_VERSION_S))*MAX_NUM_OF_SW_VERSIONS);
}

// ------------------------------------------------------------

WORD CCardMngrLoaded::GetIvrMountReadStatus()
{
	return eOk;
	// to be inserted when field 'IvrMount' is implemented in CardMngrLoaded structure
	//     (instead of the 'eOk' in the line above)
	// return m_cardMngrLoadedStruct.ivrMountReadStatus;
}
// ------------------------------------------------------------
void CCardMngrLoaded::SetIvrMountReadStatus(WORD mountStatus)
{
	// to be inserted when field 'IvrMount' is implemented in CardMngrLoaded structure
	// m_cardMngrLoadedStruct.ivrMountReadStatus = mountStatus;
}



// ------------------------------------------------------------
void CCardMngrLoaded::SetData(const char *data)
{
	memcpy( &m_cardMngrLoadedStruct, data, sizeof(CM_CARD_MNGR_LOADED_S) );
	
	for (int i=0; i<MAX_NUM_OF_SW_VERSIONS; i++)
	{
		m_cardMngrLoadedStruct.swVersionsList[i].versionDescriptor[ONE_LINE_BUFFER_LEN-2]		= '\0';
		m_cardMngrLoadedStruct.swVersionsList[i].versionNumber[MAX_NUM_OF_SW_VERSION_DIGITS-2]	= '\0';
	}
}


// ------------------------------------------------------------
void CCardMngrLoaded::SetData(CCardMngrLoaded *other)
{
	memcpy( &m_cardMngrLoadedStruct, &(other->m_cardMngrLoadedStruct), sizeof(CM_CARD_MNGR_LOADED_S) );

	for (int i=0; i<MAX_NUM_OF_SW_VERSIONS; i++)
	{
		m_cardMngrLoadedStruct.swVersionsList[i].versionDescriptor[ONE_LINE_BUFFER_LEN-2]		= '\0';
		m_cardMngrLoadedStruct.swVersionsList[i].versionNumber[MAX_NUM_OF_SW_VERSION_DIGITS-2]	= '\0';
	}
}

// ------------------------------------------------------------
void CCardMngrLoaded::ValidateStrings()
{
    CProcessBase *pProcess = CProcessBase::GetProcess();
    
    // ===== 1. serialNum
    pProcess->TestStringValidity((char *)m_cardMngrLoadedStruct.serialNum,
    							 MPL_SERIAL_NUM_LEN,
    							 __PRETTY_FUNCTION__);

    // ===== 2. postResultsList
    pProcess->TestStringValidity((char *)m_cardMngrLoadedStruct.postResultsList,
    							 MAX_NUM_OF_UNITS,
    							 __PRETTY_FUNCTION__);

    // ===== 3. unitsTypesList
    pProcess->TestStringValidity((char *)m_cardMngrLoadedStruct.unitsTypesList,
    							 MAX_NUM_OF_UNITS,
    							 __PRETTY_FUNCTION__);

    // ===== 4. swVersionsList
    for(int i = 0 ; i < MAX_NUM_OF_SW_VERSIONS ; i++)
    {    
        CM_SW_VERSION_S &curVersionS =  m_cardMngrLoadedStruct.swVersionsList[i];
        
        pProcess->TestStringValidity((char *)curVersionS.versionDescriptor,
                                     ONE_LINE_BUFFER_LEN,
                                     __PRETTY_FUNCTION__);
        
        pProcess->TestStringValidity((char *)curVersionS.versionNumber,
                                     MAX_NUM_OF_SW_VERSION_DIGITS,
                                     __PRETTY_FUNCTION__);
    }
}
