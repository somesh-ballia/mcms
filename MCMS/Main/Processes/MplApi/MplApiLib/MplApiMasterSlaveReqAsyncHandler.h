/*
 * MplApiMasterSlaveReqAsyncHandler.h
 *
 *  Created on: Nov 13, 2011
 *      Author: mhalfon
 */

#ifndef MPLAPIMASTERSLAVEREQASYNCHANDLER_H_
#define MPLAPIMASTERSLAVEREQASYNCHANDLER_H_

#include <iomanip>
#include "DataTypes.h"
#include <vector>
#include <string>

#include "PObject.h"
#include "TaskApp.h"


// this is entry in the table represent 1 opcode
struct SMasterSlaveRequestData
{
  std::map<DWORD, BOOL> m_subIdsAckStatusMap; // <DWORD = sub id to encoder , BOOL = ack status> - using 2 for 2 sub ids for encoders
  DWORD m_opcode;                             // opcode
  time_t m_initiateTime;                      // time of insert entry (in seconds since ...1970)
};

class CMplApiMasterSlaveReqAsyncHandler : public CPObject
{
  CLASS_TYPE_1(CMplApiMasterSlaveReqAsyncHandler,CPObject )
public:
	
	CMplApiMasterSlaveReqAsyncHandler();
	virtual ~CMplApiMasterSlaveReqAsyncHandler();
	virtual const char* NameOf() const { return "CMplApiMasterSlaveReqAsyncHandler";}

	void setMasterSlaveRequestHandlerSubIdData(DWORD opcode, DWORD reqId, DWORD subReqId);
	BOOL setMasterSlaveAckToHandlerSubId(DWORD subReqId, BOOL &isFinishedHandler);
	void onCleanMasterSlaveRequestTout();
	BOOL deleteMasterSlaveHandlerAccordingToReqId(DWORD handlerId);
	BOOL deleteMasterSlaveHandlerAccordingToSubId(DWORD subReqId);

  BOOL IsInUse()const {return m_inUse;}
  bool GetHandlerIdBySubReqId(DWORD& handler_id,DWORD subReqId);


private:
    void traceMasterSlaveRequestHandler();
    BOOL isMasterSlaveRequestFinished(SMasterSlaveRequestData* pMasterSlaveReqIdData);
  bool GetTimeOutHandlerId(DWORD& handlerId);

  BOOL m_inUse;

  std::map<DWORD, SMasterSlaveRequestData*>   m_masterSlaveRequestIdToSubIdMap; // <DWORD = req id , SMasterSlaveRequestData* = opcode, time enter to table, sub Id's, sub Id's ack status>
  std::map<DWORD, DWORD>   		      m_masterSlaveRequestSubIdToIdMap; // <DWORD = sub req id, DWORD = req id>


};


#endif /* MPLAPIMASTERSLAVEREQASYNCHANDLER_H_ */
