#ifndef MCCF_IVR_DISPATCHER_H__
#define MCCF_IVR_DISPATCHER_H__

/////////////////////////////////////////////////////////////////////////////////
#include <map>

#include "Segment.h"
#include "MccfIvrDialogManager.h"

#include "MccfHelper.h"

#include "Singleton.h"

/////////////////////////////////////////////////////////////////////////////////
class CConfPartyManager;
typedef void (CConfPartyManager::*DialogHandler)(DialogState&);

/////////////////////////////////////////////////////////////////////////////////
struct UniqueDialogID
{
	const AppServerID& appServerID;
	const std::string& dialogID;

	UniqueDialogID(const DialogState& state) : appServerID(state.appServerID), dialogID(state.dialogID) {}
	UniqueDialogID(const AppServerID appSrvId, const std::string& dialogID) : appServerID(appSrvId), dialogID(dialogID) {}
};

bool operator <(const UniqueDialogID& a, const UniqueDialogID& b);

/////////////////////////////////////////////////////////////////////////////////
class CMccfIvrDispatcher : public SingletonHolder<CMccfIvrDispatcher>
{
	friend bool operator <(const UniqueDialogID& a, const UniqueDialogID& b);

public:

	void AddDialog(const DialogState& state, DialogHandler handler, CConfPartyManager* pMngr);
	void RemoveDialog(const DialogState& state);
	bool Dispatch(AppServerID appServerID, const std::string& dialogID, HANDLE hMccfMsg) const;

private:

	struct DialogStateEx : public DialogState
	{
		DialogHandler handler;
		CConfPartyManager* pManager;

		DialogStateEx(const DialogState& state, DialogHandler h, CConfPartyManager* p) : DialogState(state), handler(h), pManager(p)
		{
			action = DialogState::dsa_stop;
		}
	};

private:

	typedef std::map<UniqueDialogID, DialogStateEx*> DialogsMap;
	DialogsMap map_;
};

/////////////////////////////////////////////////////////////////////////////////
#endif // MCCF_IVR_DISPATCHER_H__
