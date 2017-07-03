#include "MccfIvrDispatcher.h"
#include "Trace.h"
#include "TraceStream.h"
#include "ConfPartyManager.h"

/////////////////////////////////////////////////////////////////////////////////
bool operator <(const UniqueDialogID& a, const UniqueDialogID& b)
{
	if (a.appServerID == b.appServerID)
		return (a.dialogID.compare(b.dialogID) < 0);

	return a.appServerID < b.appServerID;
}

/////////////////////////////////////////////////////////////////////////////////
bool CMccfIvrDispatcher::Dispatch(AppServerID appServerID, const std::string& dialogID, HANDLE hMccfMsg) const
{
	DialogsMap::const_iterator it = map_.find(UniqueDialogID(appServerID, dialogID));
	FPASSERT_AND_RETURN_VALUE(it == map_.end(), false);

	CConfPartyManager* pManager = it->second->pManager;
	DialogState* state = it->second;
	state->hMccfMsg = hMccfMsg;
	DialogHandler handler = it->second->handler;

	(pManager->*handler)(*state);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////
void CMccfIvrDispatcher::AddDialog(const DialogState& state, DialogHandler handler, CConfPartyManager* pMngr)
{
	FPASSERTSTREAM_AND_RETURN(!handler, "No handler");

	DialogStateEx* pState = new DialogStateEx(state, handler, pMngr);

	std::pair<DialogsMap::iterator, bool> res = map_.insert(std::make_pair(UniqueDialogID(state), pState));
	//FPASSERT(!res.second); //TODO check
}

/////////////////////////////////////////////////////////////////////////////////
void CMccfIvrDispatcher::RemoveDialog(const DialogState& state)
{
	DialogsMap::iterator it = map_.find(UniqueDialogID(state));
	FPASSERT_AND_RETURN(it == map_.end());

	delete it->second; // delete the DialogStateEx*
	map_.erase(it);
}

/////////////////////////////////////////////////////////////////////////////////
