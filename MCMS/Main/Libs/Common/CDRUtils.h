#ifndef CDRUTILS_H_
#define CDRUTILS_H_

#include <iostream>
#include "DataTypes.h"
#include "Segment.h"



class CCDRUtils
{
public:
	CCDRUtils();
	virtual ~CCDRUtils();
	
	static const char *GetQ931CauseAsString(const int choice);
	static void CdrDumpH221Stream(std::ostream& msg, WORD len,BYTE* h221Array);
	
	static void CdrDumpH323Cap(BYTE *h323CapArray, WORD length, std::ostream& ostr, BYTE capflag);
	
	static const char* Get_Audio_Coding_Command_BitRate(BYTE command,unsigned short* pCBitRate);
	static const char* Get_Transfer_Rate_Command_BitRate(BYTE command,unsigned short* pCBitRate);
	static const char* Get_Video_Oth_Command_BitRate(BYTE command,unsigned short* pCBitRate);
	static const char* Get_Lsd_Mlp_Command_BitRate(BYTE command,unsigned short* pCBitRate);
	static const char* Get_Hsd_Hmlp_command_Bitrate(BYTE command,unsigned short* pCBitRate);

	static const char* Get_Content_command_Bitrate(BYTE command,unsigned short* pCBitRate);
	static const char* Get_Mlp_Hmlp_Command_BitRate(BYTE command,unsigned short* pCBitRate);
	static const char* Get_Lsd_Hsd_Mlp_Command(BYTE command);
	static const char* Get_Resolution(WORD x,WORD y,WORD& ResolutionNumber);
	
	static char* GetH264ProfileAsString(WORD profileValue);
	static char* GetH264LevelAsString(BYTE levelValue);
	
	static BYTE CalculateRate(BYTE StartSubChannel,  BYTE EndSubChannel);
	static BYTE CalculateH239Rate( WORD subTimeSlotCount );
	
	static void GetPPXCAllParameters(BYTE *h221str, WORD *i, int *ind, std::ostream& ostr);
	static void GetPPXCCapabilities(BYTE * CapArray, int *iter, std::ostream& ostr);
	static void GetAMSC64Rates(BYTE rateByte1, BYTE rateByte2, std::ostream& ostr);
	static void GetAMCCaps(BYTE optionByte1, BYTE optionByte2, std::ostream& ostr);
	static void GetNextNSCom(std::ostream& msg,BYTE* h221Array, BYTE * count);
	
	static void FullDumpCap(BYTE *h221str,WORD length,std::ostream& ostr,BYTE capflag);

	static BYTE IsH264TwoBytesNumber(WORD capH264Custom);//used by ISDN
	
	static BYTE CalcH264CustomParamFirstByte(WORD customParam);//used by ISDN
	static BYTE CalcH264CustomParamSecondByte(WORD customParam);//used by ISDN
	static WORD CalcH264WordFromBytes(BYTE first, BYTE second);//used by ISDN
	
	static void DumpH221Stream(std::ostream& msg, WORD len,BYTE* h221Array);//used by ISDN
	static void DumpCap(BYTE *h221str,WORD length,std::ostream& ostr);//used by ISDN
	static void DumpCap(BYTE cap,std::ostream& ostr,BYTE Table=1);//used by ISDN

	static void Dump230Opcode(CSegment& h221seg, std::ostream& ostr);//used by ISDN
	static void DumpVideoCap(BYTE *h221str,WORD length,std::ostream& ostr);

private:
	static void SmartDumpH263(std::ostream& ostr, int numberOfH263Bytes, BYTE h221str[]);
	static void SmartDumpCap(BYTE *h221str,WORD length,std::ostream& ostr);
	static void DumpCapH263(std::ostream& ostr, int numberOfH263Bytes, BYTE h221str[]);
	static void DumpCapH264(std::ostream& ostr, int numberOfH264Bytes, BYTE h221str[]);
	static void SmartDumpH263AdditionalCap(std::ostream& ostr, int numberOfH263Bytes, BYTE h221str[],int HighestStandardResolution,int IsAnnexF);
	static void SmartDumpH263SecondAdditionalCap(std::ostream& ostr, int numberOfSecondAdditionalH263Bytes, BYTE h221str[],BYTE btarrResolutionBounds[]);
	
	
	static char* GetRoleStr(DWORD role);
	
	static BYTE GetNewValueFromByte(BYTE byte,WORD first_bit,WORD last_bit);
	
	static const char* Get_Content_command_BitrateH239(BYTE command,unsigned short* pCBitRate);

	
	static int GetBitValue(BYTE byte,int bitNumber);
	
	
	static void PrintAnnexType(std::ostream& ostr, int annex);
	
	static void DumpH323Cap(std::ostream& ostr, WORD len,BYTE* h323CapArray);
	
	static void SetResolutionBounds(BYTE* resolutionBounds, WORD size, WORD index, BYTE value);
	static BYTE GetResolutionBounds(BYTE* resolutionBounds, WORD size, WORD index);
	
};

#endif /*CDRUTILS_H_*/



