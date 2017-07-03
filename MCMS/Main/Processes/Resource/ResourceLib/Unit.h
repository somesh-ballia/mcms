#ifndef UNIT_H_
#define UNIT_H_

#include <bitset>
#include <set>
#include <vector>
#include <map>
#include "PObject.h"
#include "StateMachine.h"
#include "InnerStructs.h"
#include "CardsStructs.h"
#include "IPServiceResources.h"

class CRsrcDetailElement;
class CBoard;

////////////////////////////////////////////////////////////////////////////
//                        CActivePort
////////////////////////////////////////////////////////////////////////////
// Represent Active Ports as a part of MFA units
class CActivePort : public CPObject
{
	CLASS_TYPE_1(CActivePort, CPObject)

public:
	                        CActivePort();
	                        CActivePort(PortID portId, AcceleratorID acceleratorId = 0, ConfRsrcID confId = 0, PartyRsrcID partyId = 0, DWORD capacity = 0, float promilUtilized = 0, eResourceTypes porType = ePhysical_res_none, DWORD utilizedBandWidth_in = 0, DWORD utilizedBandWidth_out = 0, float utilizedEncoderWeight = 0);
	                        CActivePort(const CActivePort&);
	virtual                ~CActivePort();
	void                    Dump(std::ostream& msg) const;

	const char*             NameOf() const                                               { return "CActivePort";     }

	friend WORD operator    ==(const CActivePort& lhs, const CActivePort& rhs);
	friend bool operator    <(const CActivePort& lhs, const CActivePort& rhs);

	WORD                    GetPortId() const                                            { return m_portId;          }
	void                    SetPortId(WORD portId)                                       { m_portId = portId;        }

	WORD                    GetAcceleratorId() const                                     { return m_acceleratorId;          }
	void                    SetAcceleratorId(WORD acceleratorId)                         { m_acceleratorId = acceleratorId; }

	ConfRsrcID              GetConfId() const                                            { return m_confId;          }
	void                    SetConfId(ConfRsrcID confId)                                 { m_confId = confId;        }

	PartyRsrcID             GetPartyId() const                                           { return m_partyId;         }
	void                    SetPartyId(PartyRsrcID partyId)                              { m_partyId = partyId;      }

	DWORD                   GetCapacity() const                                          { return m_capacity;        }
	void                    SetCapacity(DWORD capacity)                                  { m_capacity = capacity;    }

	eResourceTypes          GetPortType() const                                          { return m_type;            }
	void                    SetPortType(eResourceTypes type)                             { m_type = type;            }

	float                   GetPromilUtilized() const                                    { return m_promilUtilized;  }
	void                    SetPromilUtilized(float promilles)                           { m_promilUtilized = promilles;  }
	DWORD                   GetUtilizedBandWidth(BOOL bIn) const                         { return (bIn) ? m_utilizedBandWidth_in : m_utilizedBandWidth_out; }
	float                   GetUtilizedEncoderWeight() const                             { return m_utilizedEncoderWeight;  }

protected:
	PortID                  m_portId;
	AcceleratorID           m_acceleratorId;
	ConfRsrcID              m_confId;
	PartyRsrcID             m_partyId;
	DWORD                   m_capacity;
	eResourceTypes          m_type;
	float                   m_promilUtilized;
	DWORD                   m_utilizedBandWidth_in;
	DWORD                   m_utilizedBandWidth_out;
	float                   m_utilizedEncoderWeight;
};


typedef std::set<CActivePort> CActivePortsList;


////////////////////////////////////////////////////////////////////////////
//                        CUnitRsrc
////////////////////////////////////////////////////////////////////////////
class CUnitRsrc : public CStateMachine
{
	CLASS_TYPE_1(CUnitRsrc, CStateMachine)

public:
	                        CUnitRsrc(WORD bId, WORD uId, WORD boxId = 1, WORD subBoardId = 1, CTaskApp* pOwnerTask = NULL);
	                        CUnitRsrc(const CUnitRsrc& other);
	virtual                ~CUnitRsrc();
	void                    Dump(std::ostream& msg) const;
	const char*             NameOf() const                                               { return "CUnitRsrc"; }
	void*                   GetMessageMap();
	void                    HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	UnitID                  GetUnitId() const                                            { return m_unitId; }
	WORD                    GetBoxId() const                                             { return m_boxId;  }

	BoardID                 GetBoardId() const                                           { return m_boardId; }
	CBoard*                 GetBoard() const;

	SubBoardID              GetSubBoardId() const                                        { return m_SubBoardId; }
	void                    SetSubBoardId(SubBoardID id)                                 { m_SubBoardId = id; }

	BYTE                    GetIsEnabled() const;
	void                    SetEnabled(BYTE isEnabled)                                   { m_isEnabled = isEnabled; }

	BYTE                    GetIsDisabledManually() const                                { return m_isDisabledManually; }
	void                    SetDisabledManually(BYTE isDisable);

	BYTE                    GetIsFatal() const                                           { return m_isFatal;    }
	void                    SetFatal(BYTE isFatal)                                       { m_isFatal = isFatal; }

	friend WORD operator    ==(const CUnitRsrc&, const CUnitRsrc&);
	friend bool operator    <(const CUnitRsrc&, const CUnitRsrc&);

private:
	WORD                    m_boxId;
	BoardID                 m_boardId;
	SubBoardID              m_SubBoardId;
	UnitID                  m_unitId;
	BYTE                    m_isEnabled;                         // TRUE/FALSE;
	BYTE                    m_isDisabledManually;                // TRUE/FALSE;
	BYTE                    m_isFatal;                           // TRUE/FALSE;
};


////////////////////////////////////////////////////////////////////////////
//                        CUnitMFA
////////////////////////////////////////////////////////////////////////////
// Represent ART DSP, VIDEO DSP as a configurable resource
class CUnitMFA : public CUnitRsrc
{
	CLASS_TYPE_1(CUnitMFA, CUnitRsrc)

public:
	                        CUnitMFA(WORD bid, WORD uid, eUnitType unitType);
	                        CUnitMFA(const CUnitMFA& other);
	                       ~CUnitMFA();
	const char*             NameOf() const;
	void*                   GetMessageMap();

	friend WORD operator    ==(const CUnitMFA&, const CUnitMFA&);

	eUnitType               GetUnitType() const                                          { return m_UnitType; }
	void                    SetUnitType(eUnitType newType);

	BYTE                    GetIsAllocated()                                             { return m_isAllocated; }

	BYTE                    GetIsRecovery() const                                        { return m_isRecovery; }
	void                    SetRecovery(BYTE isRecovery, WORD newPhysicalUnitId);
	BYTE                    IsRecoveryReservedUnit() const                               { return m_isRecoveryReserved; }
	void                    SetRecoveryReservedUnit(BOOL reserve)                        { m_isRecoveryReserved = reserve; }

	DWORD                   GetUtilizedBandwidth(BOOL bIn) const;
	float                   GetFreeCapacity(WORD accelerator_id = 0) const               { return m_FreeCapacity[accelerator_id]; }
	void                    SetFreeCapacity(float freeCapacity, WORD accelerator_id = 0) { m_FreeCapacity[accelerator_id] = freeCapacity; }
	int                     GetFreeARTChannels() const                                   { return m_MaxArtChannelsPerArt - m_OccupiedARTChannels; }
	int                     GetOccupiedARTChannels() const                               { return m_OccupiedARTChannels; }
	float                   GetFreeEncoderWeight() const                                 { return m_FreeEncoderWeight; }
	size_t                  GetPortNumber() const;
	STATUS                  GetRsrcDetail(CRsrcDetailElement* pDetail);
	STATUS                  GetRsrcDetailNinja(CRsrcDetailElement* pDetail);
	DWORD                   GetAverageAllocatedCapacity();
	DWORD                   GetTotalAllocatedCapacity();

	STATUS                  AddActivePort(WORD portId, WORD acceleratorId, DWORD confId, DWORD partyId, DWORD partyCapacity, float promilUtilized, eResourceTypes type, DWORD utilizedBandWidth_in, DWORD utilizedBandWidth_out, float utilizedEncoderWeight);
	STATUS                  FreeActivePort(WORD portId);
	STATUS                  AddActivePorts(DWORD rsrcConfId, WORD rsrcPartyId, DWORD partyCapacity, MediaUnit& mediaUnit);

	// allocations internal
	int                     AllocatePorts(ConfRsrcID rsrcConfId, PartyRsrcID rsrcPartyId, DWORD partyCapacity, MediaUnit& mediaUnit, eVideoPartyType videoPartyType, WORD numOfTipScreens = 0);
	int                     AllocateVideoPortsCOP(DWORD rsrcConfId, MediaUnit& mediaUnit);

	int                     DeAllocatePort(WORD portId, eVideoPartyType videoPartyType, BYTE dsbl = FALSE, WORD acceleratorId = 0);
	int                     DeAllocatePort2C(WORD portId, BYTE dsbl = FALSE);
	int                     DeAllocatePortCOP(WORD portId, float capacity, BYTE dsbl = FALSE);
	void                    AllocateARTChannels(WORD needed_ART_channels);
	void                    DeAllocateARTChannels(WORD needed_ART_channels);
	STATUS                  ReAllocateARTChannels(WORD new_ART_channels, WORD old_ART_channels);

	STATUS                  UpdateActivePort(WORD portId, DWORD rsrcConfId, WORD rsrcPartyId);

	void                    SetConnId(DWORD connId)                                      { m_connId = connId; }
	DWORD                   GetConnId() const                                            { return m_connId;   }

	BYTE                    GetFipsStat() const                                          { return m_FipsStat;     }
	void                    SetFipsStat(BYTE fipsStat)                                   { m_FipsStat = fipsStat; }

	const CActivePortsList* GetActivePorts() const                                       { return &m_pActivePorts; }
	bool                    IsEmptyUnit() const                                          { return m_pActivePorts.empty(); }

	WORD                    GetFPGAIndex() const                                         { return m_FPGA_Index;       }
	void                    SetFPGAIndex(WORD FPGA_Index)                                { m_FPGA_Index = FPGA_Index; }

	static void             SetMaxArtChannelsPerArt(int max)                             { m_MaxArtChannelsPerArt = max;  }
	static int              GetMaxArtChannelsPerArt()                                    { return m_MaxArtChannelsPerArt; }

	WORD                    GetNumAllocatedTipScreens() const                            { return m_numAllocatedTipScreens; }
	void                    SetNumAllocatedTipScreens(WORD numOfTipScreens)              { m_numAllocatedTipScreens = numOfTipScreens; }
	void                    IncreaseNumAllocatedTipScreens(WORD numOfTipScreens);
	void                    DecreaseNumAllocatedTipScreens(WORD numOfTipScreens);

	void                    UpdateUnitIDToFPGAIndex(WORD uId, eCardType cardType);
	void                    FreeAllActivePorts();

	float                   GetFreeVideoPortsCapacity(WORD acceleratorId = 0) const;
	WORD                    GetFreeEncoderPorts(WORD acceleratorId = 0, BOOL includeFreeDisabled = TRUE) const;
	WORD                    GetFreeDecoderPorts(WORD acceleratorId = 0, BOOL includeFreeDisabled = TRUE) const;
	WORD                    GetFreeArtPorts(BOOL includeFreeDisabled = TRUE) const;

	void                    OnRecoveryUnitTimer(CSegment* pMsg);

	WORD                    GetPhysicalUnitId() const;
	void                    SetPhysicalUnitId(WORD unitId);

	void                    SetAllFreePortsToFreeDisable();
	void                    SetFreePortsToFreeDisable(WORD port);

	void                    DumpAllActivePorts(WORD min_num_of_potrs);

	bool                    Is1080p60SplitEncoderAllocated() const;

protected:
	int                     FindPortToOccupy(eLogicalResourceTypes type, WORD num_ports_per_unit, ePortAllocTypes oldPortStatus, WORD acceleratorId);
	int                     FindPortToOccupyCOP(eLogicalResourceTypes type, WORD num_ports_per_unit, ePortAllocTypes oldPortStatus);
	int                     FindPortToOccupySoftMCU(eLogicalResourceTypes type, WORD num_ports_per_unit, ePortAllocTypes oldPortStatus, int last_allocated_port = 0);

	DWORD                   m_connId;                       // for audio cntler;
	WORD                    m_FPGA_Index;
	eUnitType               m_UnitType;
	BYTE                    m_isAllocated;                  // TRUE/FALSE;
	float                   m_FreeCapacity[ACCELERATORS_PER_UNIT_NETRA];
	WORD                    m_OccupiedARTChannels;          // for ART units only: this counts the number of ISDN channels + a fixed number for each ip party
	float                   m_FreeEncoderWeight;            // For DaVinci DSP - each unit has 12 encoder ports weight (MAX_ENCODER_WEIGHT_PER_UNIT)
	CPortAllocTypeVector*   m_pPortsList;
	CActivePortsList        m_pActivePorts;
	static int              m_MaxArtChannelsPerArt;
	BYTE                    m_FipsStat;
	BYTE                    m_isRecovery;
	BYTE                    m_isRecoveryReserved;
	WORD                    m_physicalUnitId;
	WORD                    m_maxUnitNumPorts;
	WORD                    m_numAllocatedTipScreens;                 // for ART units only: this counts the number of TIP EP screens that allocated on this unit. current limit is 3.

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CSpanRTM
////////////////////////////////////////////////////////////////////////////
// Represent Net as a configurable resource
class CSpanRTM : public CUnitRsrc
{
	CLASS_TYPE_1(CSpanRTM, CUnitRsrc)

	void                    operator=(const CSpanRTM&) {} // Disallow assign

public:
	                        CSpanRTM(WORD bid, WORD uid);
	                        CSpanRTM(const CSpanRTM& other);
	                       ~CSpanRTM();
	const char*             NameOf() const;

	friend class            CResourceManager;

	STATUS                  GetRsrcDetail(CRsrcDetailElement* pDetail);

	WORD                    GetNumPorts() const                                          { return m_numPorts;                                }
	WORD                    GetNumFreePorts() const                                      { return m_numFreePorts;                            }
	WORD                    GetNumDialOutReservedPorts() const                           { return m_numDialOutReservedPorts;                 }
	DWORD                   GetNumFreePortsThatAreNotReserved() const                    { return m_numFreePorts - m_numDialOutReservedPorts;  }
	const char*             GetSpanServiceName() const                                   { return m_serviceName;                             }
	BYTE                    GetIsAllocated() const                                       { return m_isAllocated;                             }
	const CActivePortsList* GetActivePorts() const                                       { return m_pActivePorts;                            }

	eRTMSpanType            GetSpanType()                                                { return m_spanType;                                }
	void                    SetSpanType(eRTMSpanType spanType)                           { m_spanType = spanType;                            }

	STATUS                  ConfigureServiceOnSpan(char* serviceName, eRTMSpanType type, BYTE isEnabled);
	void                    SetNullConfiguration();

	STATUS                  AllocatePort(ConfRsrcID confId, PartyRsrcID partyId, eResourceTypes type, WORD& portId, BOOL bMoveToReserved);
	STATUS                  DeAllocatePort(WORD portId, BOOL bIsUpdated = TRUE);

	int                     AddActivePort(WORD portId, WORD acceleratorId, ConfRsrcID confId, PartyRsrcID partyId, WORD promilUtilized, eResourceTypes type);
	int                     FreeActivePort(WORD portId);

	STATUS                  UpdateActivePort(WORD portId, ConfRsrcID confId, PartyRsrcID partyId);
	STATUS                  FindFreePort(WORD& portId, BOOL bPortCanAlsoBeReserved);

	STATUS                  MovePortFromReservedToOccupied(WORD& portId, WORD acceleratorId, ConfRsrcID confId, PartyRsrcID partyId);
	STATUS                  RemoveReservedPort(WORD portId);

	void                    Dump(std::ostream& msg);
	void                    DumpActivePorts(std::ostream& msg);
	void                    FreeActivePorts(CRsrcDetailElement* pDetail);

private:
	WORD                    m_numPorts;
	WORD                    m_numFreePorts;
	WORD                    m_numDialOutReservedPorts;
	ePortAllocTypes*        m_pVirtualPorts;
	char                    m_serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN]; // '\0' - means Span is not configured.
	eRTMSpanType            m_spanType;                                        // SPAN_GENERIC - means span is not configured.
	BYTE                    m_isAllocated;
	CActivePortsList*       m_pActivePorts;
};


////////////////////////////////////////////////////////////////////////////
//                        CRecordingJunction
////////////////////////////////////////////////////////////////////////////
class CRecordingJunction : public CUnitRsrc, public CActivePort
{
public:
	                        CRecordingJunction(WORD bId, WORD uId, size_t portId, DWORD confId = 0, DWORD partyId = 0);
	                       ~CRecordingJunction();
	const char*             NameOf() const;

	friend WORD operator    ==(const CRecordingJunction& lhs, const CRecordingJunction& rhs);
	friend bool operator    <(const CRecordingJunction& lhs, const CRecordingJunction& rhs);

private:
	WORD junctionId;
};

#endif /* UNIT_H_ */
