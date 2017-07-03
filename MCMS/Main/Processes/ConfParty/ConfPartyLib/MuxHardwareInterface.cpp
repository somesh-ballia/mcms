
#include "H320Caps.h"
#include "Segment.h"
#include "muxint.h"
#include "OpcodesMcmsMux.h"
#include "MuxHardwareInterface.h"
#include "MuxCntl.h"

// #include <sstream>
// #include "TraceStream.h"
// #include <iomanip>
// using namespace std;

#define ENCRYPTION_ON_BITMASK 0x0001



CMuxHardwareInterface::CMuxHardwareInterface() :
  CHardwareInterface()
{
    m_pRsrcParams = new CRsrcParams();
}

CMuxHardwareInterface::CMuxHardwareInterface(CRsrcParams& hardwareInterface) :
  CHardwareInterface()
{
    m_pRsrcParams = new CRsrcParams(hardwareInterface);
}

CMuxHardwareInterface::~CMuxHardwareInterface()
{
}

const char* CMuxHardwareInterface::NameOf() const
{
    return "CMuxHardwareInterface";
}

void CMuxHardwareInterface::InitComm(WORD numChnl, WORD channelWidth, CCapH320& localCap,
									 CSegment& xmitModeSeg, CSegment& h230Seg, WORD restrictMode)
{

    
    CSegment  capSeg;
	CSegment* paramSeg = new CSegment;
	H221_INIT_COMM_S  msg; 

	switch ( restrictMode ) {
	  case Derestrict  :  { msg.restrict_type = 0;     break; }
	  case Restrict    :  { msg.restrict_type = 1;     break; }
	  default          :  { msg.restrict_type = 0xFF;  break; }
	}

	msg.additional_flags = 0x0;
	if (localCap.IsEncrypCapOn(Encryp_Cap)){
	  TRACESTR(eLevelInfoNormal) << "CMuxHardwareInterface::InitComm - Encryp_Cap is on";
	    msg.additional_flags = (msg.additional_flags|ENCRYPTION_ON_BITMASK);
	}

	msg.channel_width = channelWidth;

	CCapH320 firstCapSet = localCap;
	/* when we send a first capset to the end-point we don't know if the
	   end-point is able to understand Mbe commands. Therefore we can't
	   send H263,H264 or NonStandard caps in the first capset. */
	firstCapSet.RemoveH263Caps();
	firstCapSet.RemoveH264Caps();
    firstCapSet.RemoveNSCap();
	firstCapSet.RemoveH239Caps();

    firstCapSet.Serialize(SERIALEMBD,capSeg);
	msg.local_caps.caps_bas.number_of_bytes = capSeg.GetWrtOffset();
  	msg.initial_xmit_mode.comm_mode_bas.number_of_bytes = xmitModeSeg.GetWrtOffset();
	msg.initial_h230.h230_bas.number_of_bytes = h230Seg.GetWrtOffset();

    capSeg.DumpHex();
    xmitModeSeg.DumpHex();
    h230Seg.DumpHex();

    BYTE dword_size = sizeof (APIU32);
    DWORD caplen = capSeg.GetWrtOffset();
    paramSeg->Put((BYTE*)&caplen,dword_size); 
//	*paramSeg << capSeg.GetWrtOffset(); //<< capSeg
    paramSeg->Put(capSeg.GetPtr(),capSeg.GetWrtOffset());

    DWORD xlen = xmitModeSeg.GetWrtOffset();
    paramSeg->Put((BYTE*)&xlen,dword_size);
    paramSeg->Put(xmitModeSeg.GetPtr(),xmitModeSeg.GetWrtOffset());

    DWORD h230len = h230Seg.GetWrtOffset();
    paramSeg->Put((BYTE*)&h230len,dword_size);
    paramSeg->Put(h230Seg.GetPtr(),h230Seg.GetWrtOffset());

    paramSeg->Put((BYTE*)&msg.channel_width,dword_size);
    paramSeg->Put((BYTE*)&msg.restrict_type,dword_size);
    paramSeg->Put((BYTE*)&msg.additional_flags,dword_size);              
    
	/* add the following values (channel_width, restrict_type, additional_flags) separately
	   'byte by byte' because CSegment does alignment while putting DWORD type */
//	BYTE dword_size = sizeof (APIU32);
// 	BYTE* temp_channel_width = (BYTE*)(&msg.channel_width);
// 	for (BYTE i=0; i<dword_size; i++)
// 	  *paramSeg << temp_channel_width[i];

// 	BYTE* temp_restrict_type = (BYTE*)(&msg.restrict_type);
// 	for (BYTE i=0; i<dword_size; i++)
// 	  *paramSeg << temp_restrict_type[i];

// 	BYTE* temp_additional_flags = (BYTE*)(&msg.additional_flags);
// 	for (BYTE i=0; i<dword_size; i++)
// 	  *paramSeg << temp_additional_flags[i];


	TRACESTR(eLevelInfoNormal) << "CMuxHardwareInterface::InitComm - channel_width = " << msg.channel_width
						   << ", restrict_type = " << msg.restrict_type << ", additional_flags = " << msg.additional_flags;


    paramSeg->DumpHex();
    
	SendMsgToMPL(H221_INIT_COMM, paramSeg);
	POBJDELETE(paramSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CMuxHardwareInterface::RestrictMode(WORD restrict_type)
{
    RESTRICT_MODE_S  msg; 
	CSegment* paramSeg = new CSegment;

	msg.restrict_type = restrict_type;
	
	paramSeg->Put( (BYTE*)&msg, sizeof(RESTRICT_MODE_S) );
    
	SendMsgToMPL(RESTRICT_MODE, paramSeg);
	POBJDELETE(paramSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CMuxHardwareInterface::SetXmitRcvMode(CComMode& rcomMode, DWORD mode, WORD bitRateFlag, BYTE isH239)
{
	if ( mode == SET_RCV_MODE )
	    return;  // set rcv is handled by mux itself
	
	CSegment* paramSeg = new CSegment;
    CSegment          basSeg;         
	SET_XMIT_MODE_S   msg; 
	WORD              padd = 0;

	rcomMode.Serialize(SERIALEMBD, basSeg, isH239);

	msg.xmit_mode.comm_mode_bas.number_of_bytes = basSeg.GetWrtOffset();
// 	paramSeg->Put((BYTE*)&msg, sizeof(SET_XMIT_MODE_S)); //don't use this struct because it has no space for com mode bytes

	*paramSeg << basSeg.GetWrtOffset() << basSeg;
    
    ostringstream str;
   rcomMode.Dump(str);

    paramSeg->DumpHex();
	SendMsgToMPL(mode, paramSeg);
	POBJDELETE(paramSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CMuxHardwareInterface::SendECS(CSegment& ECSString)
{
	CSegment* paramSeg = new CSegment;
	BAS_CMD_DESC msg; 
			
	msg.number_of_bytes = ECSString.GetWrtOffset();

	paramSeg->Put( (BYTE*)&msg, sizeof(BAS_CMD_DESC) );
	*paramSeg << ECSString;

 	SendMsgToMPL(SET_ECS, paramSeg);
	POBJDELETE(paramSeg);
}

/////////////////////////////////////////////////////////////////////////////
/*void CMuxHardwareInterface::SendAudioComfortNoiseRequest(BYTE comfortNoiseOnOff)
{
	CSegment* paramSeg = new CSegment;
	AUDIO_COMFORT_NOISE_REQ_S		msg; 

	msg.audio_stream_direction = 1; // from TDM (send comfort noise to remote). 
								    // We don't use other directions now.
	msg.status = comfortNoiseOnOff;

	paramSeg->Put( (BYTE*)&msg, sizeof(AUDIO_COMFORT_NOISE_REQ_S) );

	SendMsgToMPL(AUDIO_COMFORT_NOISE_REQ, paramSeg);
	POBJDELETE(paramSeg);
}
*/

/////////////////////////////////////////////////////////////////////////////
void CMuxHardwareInterface::ExchangeCap(CCapH320& localCap, WORD IsH263_2000Cap)
{
	CSegment* paramSeg = new CSegment;
    CSegment      capSeg;
	BAS_CMD_DESC  msg; 
		
	CCapH320* secondCapSet = new CCapH320;
	*secondCapSet = localCap;
    
	// Remote do not support H263 2000 capabilities -> we remove second additinal cap.
	// Currently we remove not just the second additinal cap but the first either.
	if(IsH263_2000Cap == 0) 
		secondCapSet->SendSecondAdditionalCap(NO);
	
	secondCapSet->Serialize(SERIALEMBD,capSeg);

	msg.number_of_bytes = capSeg.GetWrtOffset();

	paramSeg->Put( (BYTE*)&msg, sizeof(BAS_CMD_DESC) );
	*paramSeg << capSeg;

    paramSeg->DumpHex();
	SendMsgToMPL(EXCHANGE_CAPS, paramSeg);
	POBJDELETE(paramSeg);
	POBJDELETE(secondCapSet);
}
/////////////////////////////////////////////////////////////////////////////
void CMuxHardwareInterface::SendH230(CSegment& h230String)
{
	CSegment* paramSeg = new CSegment;
	BAS_CMD_DESC msg; 
			
	msg.number_of_bytes = h230String.GetWrtOffset();
	paramSeg->Put((BYTE*)&msg, sizeof(BAS_CMD_DESC));
	*paramSeg << h230String;

    paramSeg->DumpHex();
	SendMsgToMPL(SEND_H_230, paramSeg);
	POBJDELETE(paramSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CMuxHardwareInterface::SetRepeatedH230(CSegment& h230String)
{
	CSegment* paramSeg = new CSegment;
	BAS_CMD_DESC msg; 
			
	msg.number_of_bytes = h230String.GetWrtOffset();

	paramSeg->Put((BYTE*)&msg, sizeof(BAS_CMD_DESC));
	*paramSeg << h230String;

    paramSeg->DumpHex();
	SendMsgToMPL(REPEATED_H230, paramSeg);
	POBJDELETE(paramSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CMuxHardwareInterface::KillConnection()                                                       
{
	CSegment* paramSeg = new CSegment;

	SendMsgToMPL(H221_KILL_CONNECTION, paramSeg);
	POBJDELETE(paramSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CMuxHardwareInterface::Destroy()
{   
    //m_pRsrcTbl->RemoveRsrcMngrPtr(m_pMux->GetRsrcId());//olga
}
