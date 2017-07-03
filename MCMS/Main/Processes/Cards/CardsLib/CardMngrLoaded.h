// CardMngrLoaded.h: interface for the CCardMngrLoaded class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CardMngrLoaded_H_
#define _CardMngrLoaded_H_


#include "PObject.h"
#include "CardsStructs.h"

using namespace std;



class CCardMngrLoaded : public CPObject
{

CLASS_TYPE_1(CCardMngrLoaded, CPObject)

public:
	CCardMngrLoaded ();
	virtual const char* NameOf() const { return "CCardMngrLoaded";}
	CCardMngrLoaded (const CM_CARD_MNGR_LOADED_S* cardMngr);
	virtual ~CCardMngrLoaded ();
	virtual void Dump(ostream& msg) const;

	CCardMngrLoaded& operator = (const CCardMngrLoaded &other);

	CM_CARD_MNGR_LOADED_S  GetCardMngrLoadedStruct();

	char*             GetSerialNumber ();
	void              SetSerialNumber (const char* theNum);

	DWORD             GetStatus ();
	void              SetStatus (const DWORD status);

	eCardType         GetType ();
	void              SetType (const eCardType type);

	BYTE*             GetPostResultsList ();
	void              SetPostResultsList (const BYTE *postResults);

	BYTE*             GetUnitsTypesList ();
	void              SetUnitsTypesList (const BYTE *unitsTypes);

	VERSION_S         GetHardwareVersion ();
	void              SetHardwareVersion (const VERSION_S hwVer);

    CM_SW_VERSION_S*  GetSwVersionsList();
	void              SetSwVersionsList(CM_SW_VERSION_S* swVerList);

    WORD              GetIvrMountReadStatus();
	void              SetIvrMountReadStatus(WORD ivrMountStatus);

	void              SetData(const char *data);
	void              SetData(CCardMngrLoaded *other);
    void              ValidateStrings();

protected:
	CM_CARD_MNGR_LOADED_S m_cardMngrLoadedStruct;
};



#endif // _CardMngrLoaded_H_
