#include "MccfIvrDialogManager.h"
#include <sstream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
void CMccfIvrDialogManager::AllocateDialogID(AppServerID appServerID, IvrControlTypeEnum type, std::string& dialogID)
{
	if (dialogID.empty())
	{
		std::ostringstream ostr;
		ostr << "msDi" << std::hex << std::setw(8) << std::setfill('0') << ++counter_;

		dialogID = ostr.str();
	}

	dialogIDs_.insert(std::make_pair(std::make_pair(appServerID, dialogID), type));
}

/////////////////////////////////////////////////////////////////////////////
IvrControlTypeEnum CMccfIvrDialogManager::CheckDialogID(AppServerID appServerID, const std::string& dialogID)
{
	if (dialogID.empty())
		return ict_None;

	DialogIDs::const_iterator it = dialogIDs_.find(std::make_pair(appServerID, dialogID));

	return it != dialogIDs_.end() ? it->second : ict_None;
}

/////////////////////////////////////////////////////////////////////////////
void CMccfIvrDialogManager::DeallocateDialogID(AppServerID appServerID, const std::string& dialogID)
{
	dialogIDs_.erase(std::make_pair(appServerID, dialogID));
}

/////////////////////////////////////////////////////////////////////////////
void CMccfIvrDialogManager::DeallocateAll(AppServerID appServerID)
{
	// TODO: ???
}

/////////////////////////////////////////////////////////////////////////////
