#include <iomanip>
#include <iostream>
#include "WrappersCSBase.h"



CBaseWrapper::CBaseWrapper()
{
}

CBaseWrapper::~CBaseWrapper()
{
}

void CBaseWrapper::DumpHeader(std::ostream& os,  const char *header) const
{
	os.setf(std::ios::left,std::ios::adjustfield);
	os.setf(std::ios::showbase);
	
	os << std::endl << std::endl
		<< header <<
		std::endl << "-----------------------" << std::endl;
}






/*-----------------------------------------------------------------------------
	class CAliasWrapper
-----------------------------------------------------------------------------*/
CAliasWrapper::CAliasWrapper(const ALIAS_S &data)
:m_Data(data)
{}

CAliasWrapper::~CAliasWrapper()
{}
	
void CAliasWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "ALIAS_S::Dump");
	
	os << std::setw(20) << "Type: "   	<< m_Data.aliasType << std::endl;
	os << std::setw(20) << "Content: " 	<< m_Data.aliasContent << std::endl;	
}	
