
#ifndef MSVSRMSG_H_
#define MSVSRMSG_H_


#include "IpPartyMonitorDefinitions.h"
#include "PObject.h"
#include "SipVsrControl.h"
#include <set>
#include <map>

class CMsVsrEntry;

typedef std::set<CMsVsrEntry> MsVsrEntriesSet;
typedef std::set<EVideoResolutionType> ResolutionsSet;
typedef std::map<EVideoResolutionType,CMsVsrEntry> ResolutionsEntriesMap;

typedef enum{
	lowest_resolution = 0,
	highest_resolution,
	max_number_of_must,
	max_number_of_may
} stEntrySelectPolicy;

class CMsVsrEntry : public CPObject
{
    CLASS_TYPE_1(CMsVsrEntry, CPObject)
  // function members
  public:
    // constructors
     CMsVsrEntry(const ST_VSR_PARAMS& vsrEntryParamsSt);
     CMsVsrEntry();
    virtual ~CMsVsrEntry(){};
    // CPObject pure virtual
    virtual const char* NameOf() const { return "CMsVsrEntry";}
    // operators
    bool operator<(const CMsVsrEntry& other)const;
    bool operator==(const CMsVsrEntry& other)const;
    friend bool operator<(const CMsVsrEntry& first,const DWORD resolution);
    friend bool operator<(const DWORD resolution,const CMsVsrEntry& second);
    friend bool operator<=(const CMsVsrEntry& first,const DWORD resolution);
    friend bool operator<=(const DWORD resolution,const CMsVsrEntry& second);
    // Get-Set
    DWORD GetResolution()const;
    DWORD GetNumOfMust()const;
    DWORD GetNumOfMay()const;
    DWORD GetPayloadType()const;
    DWORD GetMaxWidth()const;
    DWORD GetMaxHight()const;
    void  SetMaxWidth(DWORD maxWidth);
    void  SetMaxHight(DWORD maxHight);

    EVideoResolutionType GetEncoderResolutionType()const;


    const ST_VSR_PARAMS& GetEntryParamsSt()const {return m_VsrEntryParamsSt;};
    // print
    void Dump(std::ostringstream& msg) const;
    void Dump() const;

//    bool AlignToHd720(); // we need to treat res > HD720 as HD720


//    void SetInUse(bool inUse){m_bInUse = inUse;};
//    bool IsInUse()const {return m_bInUse;};

    //Void Dump();


  protected:
    // TCmRtcpVsrEntry m_rtcpVsrEntrySt;
    ST_VSR_PARAMS m_VsrEntryParamsSt;
//    bool m_bInUse;
};


class CMsVsrMsg : public CPObject
{
    CLASS_TYPE_1(CMsVsrMsg, CPObject)
  // function members
  public:
    CMsVsrMsg(const ST_VSR_SINGLE_STREAM& vsrMsgParamsSt);
    virtual ~CMsVsrMsg(){};
    // CPObject pure virtual
    virtual const char* NameOf() const { return "CMsVsrMsg";}

    DWORD SelectBestEntries(const ResolutionsSet& resolutionsSet, ResolutionsEntriesMap&  resolutionsEntriesMap);
    void  SetSingleEntry(CMsVsrEntry& entry);

    void Dump(std::ostringstream& msg) const;
    void Dump() const;
    void DumpSelection(const ResolutionsSet& encodersResolutionsSet, ResolutionsEntriesMap&  selectedEntriesPerEncodersMap)const;

    void  SetEmptyVsr();

    const ST_VSR_SINGLE_STREAM& GetVsrSingleStreamStruct() const;


  protected:
    DWORD SelectMustEntries(const ResolutionsSet& resolutionsSet, ResolutionsEntriesMap&  resolutionsEntriesMap);
    DWORD SelectMayEntries(const ResolutionsSet& resolutionsSet, ResolutionsEntriesMap&  resolutionsEntriesMap);
    DWORD FillMustEntriesSet(MsVsrEntriesSet& entriesSet);
    DWORD FillMayEntriesSet(MsVsrEntriesSet& entriesSet, DWORD payload_type = MS_SVC_PT_H264);
    bool  SelectBestEncoderForVsrEntry(MsVsrEntriesSet::iterator& entriesIterator, const ResolutionsSet& encodersResolutionsSet, ResolutionsEntriesMap&  selectedEntriesPerEncodersMap);
    bool  SelectBestEntryForEncoder(EVideoResolutionType encoderResolutionType ,MsVsrEntriesSet& entiesSet, ResolutionsEntriesMap& selectedEntriesPerEncodersMap,bool same_resolution);
 //   bool  SelectBestEntryForEncoder(DWORD encoderResolution,DWORD previousEncoderResolution,MsVsrEntriesSet& entrySetPerEncoderResolution, stEntrySelectPolicy select_policy, ResolutionsEntriesMap& selectedEntriesPerEncodersMap);

    void SetEntry(CMsVsrEntry& entry, WORD entyIndex);
    void ResetEntry(WORD entyIndex);
    WORD GetNumOfEntries()const;
    EVideoResolutionType GetEncoderResolutionType(DWORD encoder_resolution)const;

    void DumpEncodersResolutionsSet(std::ostringstream& msg,const ResolutionsSet& encodersResolutionsSet) const;
    void DumpResolutionsEntriesMap(std::ostringstream& msg,ResolutionsEntriesMap&  selectedEntriesPerEncodersMap) const;



    ST_VSR_SINGLE_STREAM m_VsrMsgParamsSt;
};


/*
////// VSR structure required by party, external API..
	typedef struct
	{
					DWORD 	payload_type; //SVC or RTV
					DWORD 	aspect_ratio; //from decision matrix
					DWORD 	max_width; //from decision matrix
					DWORD 	max_height; //from decision matrix
					DWORD 	frame_rate; // enum: by msft RTP
					DWORD 	min_bitrate; //msft RTP: take care of units
					WORD	no_of_may; // At the moment, ignored for outgoing
					WORD    no_of_must; // At the moment, ignored for outgoing
	} ST_VSR_PARAMS;

	typedef struct
	{
					DWORD sender_ssrc; // RA
					DWORD partyId;
					DWORD msi; // source any on P2P
					DWORD key_frame; // bool: default YES
					DWORD num_vsrs_params; //1 for svc/rtv only, 2 - (deafault) rtv or svc
					ST_VSR_PARAMS st_vsrs_params[VSR_MAX_ENTRIES];
	} ST_VSR_SINGLE_STREAM;
*/

#endif
