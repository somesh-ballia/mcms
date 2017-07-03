#ifndef MCCFIVRDIALOGMANAGER_H_
#define MCCFIVRDIALOGMANAGER_H_

///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <map>

#include "Singleton.h"

#include "MccfHelper.h"

///////////////////////////////////////////////////////////////////////////////
enum IvrControlTypeEnum { ict_None, ict_Prepare, ict_Start, ict_Terminate };

///////////////////////////////////////////////////////////////////////////////
class CMccfIvrDialogManager : public SingletonHolder<CMccfIvrDialogManager>
{
	friend class SingletonHolder<CMccfIvrDialogManager>; // provide access to non-public constructor

public:
	void AllocateDialogID(AppServerID appServerID, IvrControlTypeEnum type, std::string& dialogID);
	void DeallocateDialogID(AppServerID appServerID, const std::string& dialogID);
	void DeallocateAll(AppServerID appServerID);

	IvrControlTypeEnum CheckDialogID(AppServerID appServerID, const std::string& dialogID);

private:
	CMccfIvrDialogManager() : counter_(0) {}

private:
	typedef std::pair<AppServerID, std::string> UniqueDialogID;
//	typedef std::multimap<AppServerID, std::string> AppServerDialogs;

	typedef std::map<UniqueDialogID, IvrControlTypeEnum> DialogIDs;

	DialogIDs dialogIDs_;
	size_t    counter_;
};

///////////////////////////////////////////////////////////////////////////////
#endif // MCCFIVRDIALOGMANAGER_H__
