#ifndef CHANGELAYOUTACTIONS_H_
#define CHANGELAYOUTACTIONS_H_

#include "COP_Layout_definitions.h"
#include "COP_ConfParty_Defs.h"

typedef enum
{
	eCopDecoderAction_ReCapToParty = 0,
	eCopDecoderAction_Open,
	eCopDecoderAction_Close,
	eCopDecoderAction_Disconnect,
	eCopDecoderAction_Update,
	eCopDecoderAction_Connect,
	eCopDecoderAction_WaitForSync,
	eCopDecoderAction_Last
} eCopDecoderActions;

typedef enum
{
	eActionNotNeeded,
	eActionNeeded,
	eActionNeededInFirstPrior,
	eActionInProgress,
	eActionComplete
} eChangeLayoutActionsStatus;

typedef struct
{
	DWORD 						artId;
	ECopDecoderResolution 		decoderResolution;
	eChangeLayoutActionsStatus 	actionStatus;
} CHANGE_LAYOUT_ACTION_DETAILED_ST;

class CObjString;

class CChangeLayoutActions : public CPObject
{
  CLASS_TYPE_1(CChangeLayoutActions, CPObject)
    public:
  // Constructors
  CChangeLayoutActions();
  virtual ~CChangeLayoutActions();
  CChangeLayoutActions& operator = (const CChangeLayoutActions& other);


   const char* NameOf() const { return "CChangeLayoutActions"; }

   void AddDetailedAction(DWORD decoder_index,eCopDecoderActions action,DWORD artId, ECopDecoderResolution decoderResolution = COP_decoder_resolution_Last, eChangeLayoutActionsStatus actionStatus = eActionNeeded);
   // Operations
   eChangeLayoutActionsStatus GetChangeLayoutActionStatusForDecoder(DWORD decoder_index,eCopDecoderActions action);
   DWORD GetChangeLayoutActionArtIdForDecoder(DWORD decoder_index,eCopDecoderActions action);
   ECopDecoderResolution GetChangeLayoutActionDecoderResForDecoder(DWORD decoder_index,eCopDecoderActions action);
   void UpdateActionStatus(DWORD decoder_index,eCopDecoderActions action,eChangeLayoutActionsStatus	actionStatus = eActionNeeded);
   void UpdateReCapToPartyActionStatus(DWORD artId,eChangeLayoutActionsStatus 	actionStatus);
   WORD GetDecoderIndexForReCapParty(DWORD artId);
   void ResetActionTable();
   void Dump();

   void AddDecoderIndexToCalcStr(WORD decoder_index, CObjString& cstr);
   void AddPartyIdToCalcStr(DWORD party_id,CObjString& cstr);
   void AddActionStatusToCalcStr(eChangeLayoutActionsStatus stat,CObjString& cstr);
   void AddResolutionToCalcStr(ECopDecoderResolution res, CObjString& cstr);



 protected:
	 CHANGE_LAYOUT_ACTION_DETAILED_ST m_actionsTable [NUM_OF_COP_DECODERS][eCopDecoderAction_Last] ;

public:
	 eChangeLayoutActionsStatus GetActionStatus(int decoderNumber, eCopDecoderActions decoderAction) { return m_actionsTable [decoderNumber][decoderAction].actionStatus; }
};

#endif /*CHANGELAYOUTACTIONS_H_*/
