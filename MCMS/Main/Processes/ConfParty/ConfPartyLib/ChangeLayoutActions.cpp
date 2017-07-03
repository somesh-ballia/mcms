/*
 * ChangeLayoutActions.cpp
 *
 *  Created on: Sep 13, 2009
 *      Author: kgratz
 */
#include "ChangeLayoutActions.h"
#include "HostCommonDefinitions.h"
#include "ObjString.h"


//======================================================================================================================================================================//
// costructors
CChangeLayoutActions::CChangeLayoutActions()
{
	ResetActionTable();
}
//======================================================================================================================================================================//
CChangeLayoutActions::~CChangeLayoutActions()
{
}
//======================================================================================================================================================================//
eChangeLayoutActionsStatus CChangeLayoutActions::GetChangeLayoutActionStatusForDecoder(DWORD decoder_index,eCopDecoderActions action)
{

	if((decoder_index>=NUM_OF_COP_DECODERS)||(action>=eCopDecoderAction_Last))
	{
		CMedString cstr;
		cstr << "CChangeLayoutActions::GetChangeLayoutActionStatusForDecoder illegal params decoder index: " << decoder_index << " eCopDecoderActions: " << action;
		PASSERTMSG(101,cstr.GetString());
		return eActionNotNeeded;
	}
	else
	{
		return m_actionsTable[decoder_index][action].actionStatus;

	}
}
//======================================================================================================================================================================//
CChangeLayoutActions& CChangeLayoutActions::operator = (const CChangeLayoutActions& other)
{
	if ( &other == this ) return *this;
	for(int i=0; i< NUM_OF_COP_DECODERS; i++)
	{
		for(int j=0; j<eCopDecoderAction_Last; j++)
		{
			m_actionsTable[i][j].artId = other.m_actionsTable[i][j].artId;
			m_actionsTable[i][j].decoderResolution = other.m_actionsTable[i][j].decoderResolution;
			m_actionsTable[i][j].actionStatus = other.m_actionsTable[i][j].actionStatus;
		}
	}
	return *this;

}
//======================================================================================================================================================================//
DWORD CChangeLayoutActions::GetChangeLayoutActionArtIdForDecoder(DWORD decoder_index,eCopDecoderActions action)
{

	if((decoder_index>=NUM_OF_COP_DECODERS)||(action>=eCopDecoderAction_Last))
	{
		CMedString cstr;
		cstr << "CChangeLayoutActions::GetChangeLayoutActionArtIdForDecoder illegal params decoder index: " << decoder_index << " eCopDecoderActions: " << action;
		PASSERTMSG(101,cstr.GetString());

		return DUMMY_PARTY_ID;
	}
	else
	{
		return m_actionsTable[decoder_index][action].artId;

	}
}
//======================================================================================================================================================================//
ECopDecoderResolution CChangeLayoutActions::GetChangeLayoutActionDecoderResForDecoder(DWORD decoder_index,eCopDecoderActions action)
{

	if((decoder_index>=NUM_OF_COP_DECODERS)||(action>=eCopDecoderAction_Last))
	{
		CMedString cstr;
		cstr << "CChangeLayoutActions::GetChangeLayoutActionDecoderResForDecoder illegal params decoder index: " << decoder_index << " eCopDecoderActions: " << action;
		PASSERTMSG(101,cstr.GetString());

		return COP_decoder_resolution_Last;
	}
	else
	{
		return m_actionsTable[decoder_index][action].decoderResolution;

	}
}
//======================================================================================================================================================================//
void CChangeLayoutActions::UpdateActionStatus(DWORD decoder_index,eCopDecoderActions action,eChangeLayoutActionsStatus 	actionStatus)
{
	if((decoder_index>=NUM_OF_COP_DECODERS)||(action>=eCopDecoderAction_Last))
	{
		CMedString cstr;
		cstr << "CChangeLayoutActions::UpdateActionStatus illegal params decoder index: " << decoder_index << " eCopDecoderActions: " << action;
		PASSERTMSG(101,cstr.GetString());

		return;
	}
	else
	{
		m_actionsTable[decoder_index][action].actionStatus = actionStatus;

	}
}
//======================================================================================================================================================================//
void CChangeLayoutActions::UpdateReCapToPartyActionStatus(DWORD artId,eChangeLayoutActionsStatus 	actionStatus)
{
	for(int i=0; i< NUM_OF_COP_DECODERS; i++)
	{
		if(m_actionsTable[i][eCopDecoderAction_ReCapToParty].artId==artId)
		{
			m_actionsTable[i][eCopDecoderAction_ReCapToParty].actionStatus = actionStatus;
		}
	}
}
//======================================================================================================================================================================//
void CChangeLayoutActions::AddDetailedAction(DWORD decoder_index,eCopDecoderActions action,DWORD artId, ECopDecoderResolution decoderResolution, eChangeLayoutActionsStatus actionStatus)
{
	if((decoder_index>=NUM_OF_COP_DECODERS)||(action>=eCopDecoderAction_Last))
	{
		CMedString cstr;
		cstr << "CChangeLayoutActions::UpdateActionStatus illegal params decoder index: " << decoder_index << " eCopDecoderActions: " << action;
		PASSERTMSG(101,cstr.GetString());

		return;
	}
	else
	{
		m_actionsTable[decoder_index][action] = (CHANGE_LAYOUT_ACTION_DETAILED_ST){artId,decoderResolution,actionStatus};
	}
}
//======================================================================================================================================================================//
void CChangeLayoutActions::ResetActionTable()
{
	for(int i=0; i< NUM_OF_COP_DECODERS; i++)
	{
		for(int j=0; j<eCopDecoderAction_Last; j++)
		{
			m_actionsTable[i][j].artId = DUMMY_PARTY_ID;
			m_actionsTable[i][j].decoderResolution = COP_decoder_resolution_Last;
			m_actionsTable[i][j].actionStatus = eActionNotNeeded;
		}

	}
}
//======================================================================================================================================================================//
void CChangeLayoutActions::Dump()
{
	CManDefinedString cstr(ONE_LINE_BUFFER_LEN*5*NUM_OF_COP_DECODERS);
	/*
	cstr << "+-----------+-----------------------------------+-----------------------------------+-----------------------------------+-----------------------------------+-----------------------------------+-----------------------------------+\n"
		 << "| decoder   |           ReCapToParty            |           Open Decoder            |         Close Decoder             |        Disconnect Decoder         |          Update Decoder           |         Connnect Decoder          |\n"
		 << "| index     | PartyId |  resolution |   status  | PartyId |  resolution |   status  | PartyId |  resolution |   status  | PartyId |  resolution |   status  | PartyId |  resolution | 	status   | PartyId |  resolution | 	status   |\n"
		 << "+-----------+---------+-------------+-----------+---------+-------------+-----------+---------+-------------+-----------+---------+-------------+-----------+---------+-------------+-----------+---------+-------------+-----------+\n";


	for(int i=0; i< NUM_OF_COP_DECODERS; i++)
	{
		cstr << "|";
		AddDecoderIndexToCalcStr(i,cstr);

		for(int j=0; j<eCopDecoderAction_Last; j++)
		{
			AddPartyIdToCalcStr(m_actionsTable[i][j].artId,cstr);
			AddResolutionToCalcStr(m_actionsTable[i][j].decoderResolution, cstr);
			AddActionStatusToCalcStr(m_actionsTable[i][j].actionStatus, cstr);
		}
		cstr << "\n";
	}
	cstr << "+-----------+---------+-------------+-----------+---------+-------------+-----------+---------+-------------+-----------+---------+-------------+-----------+---------+-------------+-----------+---------+-------------+-----------+";
	PTRACE2(eLevelInfoNormal,"CChangeLayoutActions table action :\n",cstr.GetString());

	cstr.Clear();
	*/
		cstr << "+-----------+--------------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------|\n"
			 << "| decoder   |                    |ReCapToParty |     Open    |    Close    | Disconnect  |    Update   |   Connnect  | Sync Decoder|\n"
			 << "+-----------+--------------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------|\n";


		for(int i=0; i< NUM_OF_COP_DECODERS; i++)
		{
			cstr << "|           | art id:            |";
			for(int j=0; j<eCopDecoderAction_Last; j++)
			{
				AddPartyIdToCalcStr(m_actionsTable[i][j].artId,cstr);
			}
			cstr << "\n|";
			AddDecoderIndexToCalcStr(i,cstr);
			cstr << " decoder resolution:|";
			for(int j=0; j<eCopDecoderAction_Last; j++)
			{
				AddResolutionToCalcStr(m_actionsTable[i][j].decoderResolution, cstr);
			}
			cstr << "\n|           | action status:     |";
			for(int j=0; j<eCopDecoderAction_Last; j++)
			{
				AddActionStatusToCalcStr(m_actionsTable[i][j].actionStatus, cstr);
			}
			cstr << "\n+-----------+--------------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------|\n";

		}

		PTRACE2(eLevelInfoNormal,"CChangeLayoutActions table action :\n",cstr.GetString());


/*
	CLargeString mstr;

	for(int i=0; i< NUM_OF_COP_DECODERS; i++)
	{
		for(int j=0; j<eCopDecoderAction_Last; j++)
		{
			mstr << "Decoder num index : "<< i<< " Action: "<< j<< " artId " << m_actionsTable[i][j].artId ;
			mstr << " decoderResolution" << m_actionsTable[i][j].decoderResolution;
			mstr << " actionStatus" << m_actionsTable[i][j].actionStatus<<"\n";
		}

	}
	PTRACE2(eLevelInfoNormal,"CChangeLayoutActions table action :\n",mstr.GetString());
	*/
}
//======================================================================================================================================================================//
void CChangeLayoutActions::AddDecoderIndexToCalcStr(WORD decoder_index, CObjString& cstr)
{
	if (decoder_index == (WORD)-1)
		//cstr << "12345678901|";
		  cstr << "     NA    |";
	else if (decoder_index < 10 )
	{
		//cstr << "12345" << 6 			   << "78901|";
		  cstr << "     " << decoder_index << "     |";
	}
	else
	{
		//cstr << "12345" << 67 		   << "8901|";
		  cstr << "     " << decoder_index << "    |";
	}
}
//======================================================================================================================================================================//
void CChangeLayoutActions::AddPartyIdToCalcStr(DWORD party_id,CObjString& cstr)
{
	//" PartyId |"
	//"123456789012|"
	if (party_id == (DWORD)-1){
		cstr << "             |";


		//cstr << "    NA   |";
	}
	else if (party_id < 10){
		//"123456789|"
  	    //cstr << "1234" << 5 		 << "6789|";
		  cstr << "      " << party_id << "      |";
	}
	else if (party_id < 100){
		//cstr << "1234" << 56 		 << "789|";
		  cstr << "      " << party_id << "     |";
	}
	else if (party_id < 1000){
		//cstr << "123" << 456 		<< "789|";
		  cstr << "     " << party_id << "     |";
	}
	else if (party_id < 10000){
		cstr << "     " << party_id << "    |";
	}
	else if (party_id < 100000){
		cstr << "    "  << party_id << "    |";
	}
	else if (party_id < 1000000){
		cstr << "    "  << party_id << "   |";
	}
	else if (party_id < 10000000){
		cstr << "   "   << party_id << "   |";
	}
	else if (party_id < 100000000){
		cstr << "   "   << party_id << "  |";
	}
	else
	{
		cstr << "  " << party_id << "  |";
	}
}
//======================================================================================================================================================================//
void CChangeLayoutActions::AddActionStatusToCalcStr(eChangeLayoutActionsStatus stat,CObjString& cstr)
{

	switch(stat)
	{
		case(eActionNotNeeded):
		{

			cstr << "             |";
			//cstr << " NotNeeded |";
			break;
		}
		case(eActionNeeded):
		{
			cstr << "  Needed     |";
			break;
		}
		case(eActionNeededInFirstPrior):
		{
			cstr << "Needed FirstP|";
			break;
		}
		case(eActionInProgress):
		{
			cstr << " InProgress  |";
			break;
		}
		case(eActionComplete):
		{
			cstr << " Completed   |";
			break;
		}

		default:
		{
			cstr << " unknown     |";
			break;
		}
	}
}
//======================================================================================================================================================================//
void CChangeLayoutActions::AddResolutionToCalcStr(ECopDecoderResolution res, CObjString& cstr)
{
	if (res == COP_decoder_resolution_HD108030)
	{
		cstr << "   HD108030   |";
	}
	else if (res == COP_decoder_resolution_HD720p50)
	{
		cstr << "   HD720p50  |";
	}
	else if (res ==COP_decoder_resolution_HD720p25)
	{
		cstr << "   HD720p25  |";
	}
	else if (res == COP_decoder_resolution_W4CIF25)
	{
		cstr << "   W4CIF25   |";
	}
	else if(res ==COP_decoder_resolution_4CIF50)
	{
		cstr << "    4CIF50   |";
	}
	else if (res == COP_decoder_resolution_4CIF25)
	{
		cstr << "    4CIF25   |";
	}
	else if (res == COP_decoder_resolution_CIF25)
	{
		cstr << "    CIF25    |";
	}
	else if (res == COP_decoder_resolution_Last)
	{
		cstr << "             |";
	}
	else
	{
		cstr << "   INVALID!  |";
	}

}
//======================================================================================================================================================================//
WORD CChangeLayoutActions::GetDecoderIndexForReCapParty(DWORD artId)
{
	WORD decoderIndex = NUM_OF_COP_DECODERS;
	for(WORD i=0; i< NUM_OF_COP_DECODERS; i++)
	{
		if(m_actionsTable[i][eCopDecoderAction_ReCapToParty].artId==artId)
		{
			decoderIndex = i;
		}
	}
	return decoderIndex;
}
//======================================================================================================================================================================//
