//+========================================================================+
//                            H320VideoCaps.CPP                            |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H263Cap.CPP                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |  01/10/07  |                                                      |
//+========================================================================+


#include <sstream>
#include <iomanip>


#include "Macros.h"
#include "Segment.h"
#include "ObjString.h"
#include "H221.h"
#include "H263.h"
#include "H263Cap.h"
#include "ConfPartyGlobals.h"
#include "TraceStream.h"

using namespace std;



#define MAX_ENHANCEMENT_LAYERS   14
#define ADDITIONAL_CAPABILITIES          (unsigned short)7   //olga???
#define SECOND_ADDITIONAL_CAPABILITIES   (unsigned short)8   //olga???

/////////////////////////////////////////////////////////////////////////////
BYTE GetBit(BYTE byte,int bitNumber)
{
	BYTE res = 0;
	BYTE temp = (BYTE)1;
	if(byte & (temp << (8-bitNumber)))
		res = 1;
	return res;
}

/////////////////////////////////////////////////////////////////////////////
BYTE GenerateNewByte(BYTE byte,WORD first_bit,WORD last_bit)
{
	FPASSERT( (first_bit == (WORD)0) || (last_bit == (WORD)0) );
	FPASSERT( first_bit > last_bit );
	BYTE result=0;
	BYTE check=0;
	int current_bit_number = (int)last_bit;

	for(WORD i=0;i<=(last_bit-first_bit);i++){
		check = (BYTE)1 << (8-current_bit_number);
		if( byte & check )
			result += (BYTE)1 << i;
		current_bit_number--;
	}
	return (BYTE)result;
}
/////////////////////////////////////////////////////////////////////////////
WORD IsSameAdditionalCap(const CCapSetH263& CapSetH263,const CCapSetH263& RemoteLastFormatExplicitlyDeclared)
{
	if(CapSetH263.m_IndividualOptionIndicator == RemoteLastFormatExplicitlyDeclared.m_IndividualOptionIndicator &&
	   CapSetH263.m_Option_1 == RemoteLastFormatExplicitlyDeclared.m_Option_1 &&
	   CapSetH263.m_Option_2 == RemoteLastFormatExplicitlyDeclared.m_Option_2 &&
	   CapSetH263.m_Option_3 == RemoteLastFormatExplicitlyDeclared.m_Option_3 &&
	   CapSetH263.m_RefSliceParameters == RemoteLastFormatExplicitlyDeclared.m_RefSliceParameters &&
	   CapSetH263.m_ScalabilityDescriptor == RemoteLastFormatExplicitlyDeclared.m_ScalabilityDescriptor)
	{

	   for( WORD Index = 0; Index < MAX_ENHANCEMENT_LAYERS ; Index++)
	     if(CapSetH263.m_EnhancementLayerInfo[Index] !=  RemoteLastFormatExplicitlyDeclared.m_EnhancementLayerInfo[Index])
			 return FALSE;

	   return TRUE;// All fields are equal.
	}
	else
	   return FALSE;
}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CCapSetH263::CCapSetH263()          // constructor
{
    m_EnhancementLayerInfo = new BYTE[MAX_ENHANCEMENT_LAYERS];
	CreateDefault();
}
/////////////////////////////////////////////////////////////////////////////
CCapSetH263::CCapSetH263(BYTE vidFormat)		// Ctor 2
{
	m_EnhancementLayerInfo = new BYTE[MAX_ENHANCEMENT_LAYERS];

	m_vidImageFormat	= vidFormat;
	m_mpi				= (BYTE)0;
	m_optionalCap		= (BYTE)0;
	m_HRDbAndBPPmaxKb	= (BYTE)0;
	m_AdditionalH263Cap	= (BYTE)5; // No additional h.263 options are supported.
	switch (vidFormat)
	{
		// STANDARD formats
		case H263_QCIF_SQCIF :
		  m_MinCustomPictureWidth = 176/8 - 1;   m_MinCustomPictureHeight = 144/8 - 1;
		  break; // [176 x 144]
		case H263_CIF :
		  m_MinCustomPictureWidth = 352/8 - 1;   m_MinCustomPictureHeight = 288/8 - 1;
		  break; // [352 x 288]
		case H263_CIF_4 :
		  m_MinCustomPictureWidth = 704/8 - 1;   m_MinCustomPictureHeight = 576/8 - 1;
		  break; // [704 x 576]
		case H263_CIF_16 :
		  m_MinCustomPictureWidth = 1408/8 - 1;  m_MinCustomPictureHeight = 1152/8 - 1;
		  break; // [1408 x 1152]
		// CUSTOM formats
		case VGA :
		  m_MinCustomPictureWidth = 640/8 - 1;   m_MinCustomPictureHeight = 480/8 - 1;	m_vidImageFormat = H263_CUSTOM_FORMAT;
		  break; // [640 x 480]
		case NTSC :
		  m_MinCustomPictureWidth = 704/8 - 1;   m_MinCustomPictureHeight = 480/8 - 1;	m_vidImageFormat = H263_CUSTOM_FORMAT;
		  break; // [704 x 480]
		case SVGA :
		  m_MinCustomPictureWidth = 800/8 - 1;   m_MinCustomPictureHeight = 600/8 - 1;	m_vidImageFormat = H263_CUSTOM_FORMAT;
		  break; // [800 x 600]
		case XGA :
		  m_MinCustomPictureWidth = 1024/8 - 1;   m_MinCustomPictureHeight = 768/8 - 1;	m_vidImageFormat = H263_CUSTOM_FORMAT;
		  break; // [1024 x 768]
		case NTSC_60_FIELDS :
		  m_MinCustomPictureWidth = 352/8 - 1;   m_MinCustomPictureHeight = 240/8 - 1;	m_vidImageFormat = H263_CUSTOM_FORMAT;
		  break; // [352 x 240]
		case PAL_50_FIELDS :
		  m_MinCustomPictureWidth = 352/8 - 1;   m_MinCustomPictureHeight = 288/8 - 1;	m_vidImageFormat = H263_CUSTOM_FORMAT;
		  break; // [352 x 288]

		default : 
		  PASSERT(vidFormat);
		  m_MinCustomPictureWidth = 0;
		  m_MinCustomPictureHeight = 0;
	}

	 m_MaxCustomPictureHeight = (BYTE)0;
	 m_MaxCustomPictureWidth =  (BYTE)0;
	 m_CustomPCF_1 =  (BYTE)0;
	 m_CustomPCF_2 =  (BYTE)0;
     m_HRDBPPMaxKB =  (BYTE)0;
     m_CustomPixelWidth =  (BYTE)0;
	 m_CustomPixelHeight =  (BYTE)0;
	 m_IndividualOptionIndicator =  (BYTE)0;
     m_Option_1 =  (BYTE)0;
	 m_Option_2 =  (BYTE)0;
	 m_Option_3 =  (BYTE)0;
	 m_RefSliceParameters =  (BYTE)0;
	 m_ScalabilityDescriptor =  (BYTE)0;
	 m_SecondAdditionalH263Cap = (BYTE)0x40;// No second additinal capabilities are supported

     for( WORD Index = 0; Index < MAX_ENHANCEMENT_LAYERS ; Index++)
	     m_EnhancementLayerInfo[Index] = (BYTE)0;
}
/////////////////////////////////////////////////////////////////////////////
WORD CCapSetH263::operator==(const CCapSetH263& CapSetH263) const
{
	// Resolution is equal.
  	if( (m_MinCustomPictureHeight == CapSetH263.m_MinCustomPictureHeight) &&
        (m_MinCustomPictureWidth == CapSetH263.m_MinCustomPictureWidth) &&
        (m_MaxCustomPictureHeight == CapSetH263.m_MaxCustomPictureHeight) &&
        (m_MaxCustomPictureWidth == CapSetH263.m_MaxCustomPictureWidth) )
   	   return 1;
    else
       return 0;
}
/////////////////////////////////////////////////////////////////////////////
WORD CCapSetH263::operator<(const CCapSetH263& CapSetH263) const
{
	BYTE This_MaxCustomPictureHeight = 0;
	BYTE This_MaxCustomPictureWidth = 0;
    BYTE CapSetH263_MaxCustomPictureHeight = 0;
    BYTE CapSetH263_MaxCustomPictureWidth = 0;

	// This Object
    if(m_MaxCustomPictureWidth && m_MaxCustomPictureHeight)
	{
        This_MaxCustomPictureHeight = m_MaxCustomPictureHeight;
        This_MaxCustomPictureWidth = m_MaxCustomPictureWidth;
	}
    else
	{
	    This_MaxCustomPictureHeight = m_MinCustomPictureHeight;
        This_MaxCustomPictureWidth = m_MinCustomPictureWidth;
	}

	// CapSetH263 Object
	if(CapSetH263.m_MaxCustomPictureWidth && CapSetH263.m_MaxCustomPictureHeight)
	{
        CapSetH263_MaxCustomPictureHeight = CapSetH263.m_MaxCustomPictureHeight;
        CapSetH263_MaxCustomPictureWidth = CapSetH263.m_MaxCustomPictureWidth;
	}
    else
	{
	    CapSetH263_MaxCustomPictureHeight = CapSetH263.m_MinCustomPictureHeight;
        CapSetH263_MaxCustomPictureWidth = CapSetH263.m_MinCustomPictureWidth;
	}

	//This Object is CUSTOM FORMAT
	//CUSTOM FORMAT must be <= from STANDARD FORMAT / CUSTOM FORMAT in order to be considered as bigger.
	if(m_vidImageFormat == H263_CUSTOM_FORMAT)
	{
		if(((This_MaxCustomPictureHeight*This_MaxCustomPictureWidth) < (CapSetH263_MaxCustomPictureHeight*CapSetH263_MaxCustomPictureWidth)) ||
			(((This_MaxCustomPictureHeight*This_MaxCustomPictureWidth) == (CapSetH263_MaxCustomPictureHeight*CapSetH263_MaxCustomPictureWidth)) &&
				(This_MaxCustomPictureHeight < CapSetH263_MaxCustomPictureHeight)))
	         return 1;
        else
             return 0;
	}
	else   //This Object is STANDARD FORMAT
		   //STANDARD FORMAT can be <= from STANDARD FORMAT / CUSTOM FORMAT in order to be considered as bigger.
	{
		if((This_MaxCustomPictureHeight*This_MaxCustomPictureWidth) <= (CapSetH263_MaxCustomPictureHeight*CapSetH263_MaxCustomPictureWidth))

			return 1;
		else
			return 0;
	}
}
/////////////////////////////////////////////////////////////////////////////
CCapSetH263::~CCapSetH263()          // destructor
{
  	delete []m_EnhancementLayerInfo;
}
/////////////////////////////////////////////////////////////////////////////
const char* CCapSetH263::NameOf()  const
{
	return "CCapSetH263";
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::Dump(void) const
{
	std::ostringstream  msg;

	msg << "\nCCapSetH263::Dump\n"
      << "-----------\n"
      << setw(20) << "this"             << (hex) << (DWORD)this  << (dec) << "\n"
      << setw(20) << "m_vidImageFormat" << h263_cap_Format[(int)m_vidImageFormat] << "\n"
      << setw(20) << "m_mpi"            << h263_cap_MPI[(int)m_mpi]  << "\n"
      << setw(20) << "CPM"				<< (int)IsCPM()     << "\n"
      << setw(20) << "UMV"			    << (int)IsUMV()     << "\n"
      << setw(20) << "AMP (Annex F) "	<< (int)IsAMP()     << "\n"
      << setw(20) << "AC"				<< (int)IsAC()      << "\n"
	  << setw(20) << "PB"				<< (int)IsPB()      << "\n"
      << setw(20) << "HRD-B Size"		<< ((GetHRDb()<16) ? h263_cap_HRD_B_Size[(int)GetHRDb()] : "ERROR") << "\n"
      << setw(20) << "BPPmaxKB"			<< ((GetBPPmaxKb()<16) ? h263_cap_BPPmaxKB[(int)GetBPPmaxKb()] : "ERROR") << "\n";

	msg << "\n";

	PTRACE2(eLevelInfoNormal, "\n", (char*)msg.str().c_str());//INTO_MCMS_TRACE
}
/////////////////////////////////////////////////////////////////////////////
// this function is dump function for debug trace of highest common
// it dumps only the relevant parameters for highest common (04/04 , ron, for highest common phase 2)
void CCapSetH263::HcDump(CObjString* ostr,WORD local_dump) const
{
	WORD local_ostr = FALSE;
	if(ostr == NULL)
	{
		local_ostr = TRUE;
		ostr = new CMedString();
	}

	if(m_vidImageFormat != H263_CUSTOM_FORMAT && !GetInterlaceMode())
	{
		*ostr << "format = " << h263_cap_Format[(int)m_vidImageFormat] << "\n";
		*ostr << "mpi    = " << h263_cap_MPI[(int)m_mpi] << "\n";
		if(IsAMP()){
			*ostr << "+ AMP (Annex F)" << "\n";
		}
		if(IsAnnex_T()){
			*ostr << "+ Annex T" << "\n";
		}
		if(IsAnnex_N()){
			*ostr << "+ Annex N" << "\n";
		}
	}else if(GetInterlaceMode())
	{
		*ostr << "format = " << h263_cap_Format[(int)m_vidImageFormat] << "\n";
		*ostr << "Interlaced mode\n";
	}else // custom format not interlaced
	{
		*ostr << "format = " << h263_cap_Format[(int)m_vidImageFormat] << "\n";
		switch(GenerateNewByte(m_AdditionalH263Cap,1,2))
		{
		case STANDARD_FORMAT_ADDITIONAL_CAP :
			case CUSTOM_FORMAT_CAP_EQUAL_BOUNDS :
				{
					*ostr << "  [ " << ((m_MinCustomPictureWidth + 1) * 8) << " X " << ((m_MinCustomPictureHeight + 1) * 8) << " ]\n";
					break;
				}

			case CUSTOM_FORMAT_CAP_TWO_DISTINCT_BOUNDS :
				{
					*ostr << "  [ " << (int)((m_MinCustomPictureWidth + 1) * 8) << " - " << (int)((m_MinCustomPictureHeight + 1) * 8) << " ]" << " X ";
					*ostr << "[ " << (int)((m_MaxCustomPictureWidth + 1) * 8) << " - " << (int)((m_MaxCustomPictureHeight + 1) * 8) << " ]\n";
					break;
				}
			default :
				{
					*ostr << " FORBIDDEN "; // Standard : Value of 1 is not allowed.
				}
		}
		if(IsAnnex_T()){
			*ostr << "+ Annex T" << "\n";
		}
		if(IsAnnex_N()){
			*ostr << "+ Annex N" << "\n";
		}
	}
	if(local_dump){
	  PTRACE2(eLevelInfoNormal,"CCapSetH263::HcDump: \n",ostr->GetString());
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::Dump(std::ostream& ostr) const
{

   ostr << h263_cap_Format[(int)m_vidImageFormat];
   if(m_vidImageFormat != H263_CUSTOM_FORMAT) // Relevant only for STANDARD formats
   {
	   ostr	<< " at " << h263_cap_MPI[(int)m_mpi] << " ";

	   if(IsCPM())  ostr << setw(20) << "+ CPM ";
	   if(IsUMV())  ostr << setw(20) << "+ UMV ";
	   if(IsAMP())  ostr << setw(20) << "+ AMP (Annex F)";
	   if(IsAC())   ostr << setw(20) << "+ AC ";
	   if(IsPB())   ostr << setw(20) << "+ PB ";

	   if(GetBit(m_optionalCap,7) || GetBit(m_optionalCap,8))
	   {
		   ostr	<< "\n" << ((GetHRDb()<16) ? h263_cap_HRD_B_Size[(int)GetHRDb()] : "ERROR") << " + ";
		   ostr << ((GetBPPmaxKb()<16) ? h263_cap_BPPmaxKB[(int)GetBPPmaxKb()] : "ERROR") << "\n" << "\n";
	   }
   }

   if(m_vidImageFormat == H263_CUSTOM_FORMAT || GetInterlaceMode())
   {
       switch(GenerateNewByte(m_AdditionalH263Cap,1,2))
	   {
	   case STANDARD_FORMAT_ADDITIONAL_CAP :
			case CUSTOM_FORMAT_CAP_EQUAL_BOUNDS :
				{
					ostr << (dec) << "  [ " << (int)((m_MinCustomPictureWidth + 1) * 8) << " X " << (int)((m_MinCustomPictureHeight + 1) * 8) << " ]" << (hex);
					// print CustomMPI as FPS
					BYTE bb = GenerateNewByte(m_CustomPCF_2,1,6);
					if( bb < MPI_30+1 )
						ostr  << " at " << h263_cap_MPI[(int)bb];
					break;
				}

			case CUSTOM_FORMAT_CAP_TWO_DISTINCT_BOUNDS :
				{
					ostr << "  [ " << (int)((m_MinCustomPictureWidth + 1) * 8) << " - " << (int)((m_MinCustomPictureHeight + 1) * 8) << " ]" << " X ";
			   ostr << "[ " << (int)((m_MaxCustomPictureWidth + 1) * 8) << " - " << (int)((m_MaxCustomPictureHeight + 1) * 8) << " ]";
					break;
				}
			default :
				{
					ostr << " FORBIDDEN "; // Standard : Value of 1 is not allowed.
				}
		}

	   if(IsCustomPCFFlag())
	   {
		   ostr  << "\n" << "Clock Divisor - "            << (int)GenerateNewByte(m_CustomPCF_1,1,7)
			     << " , Clock Conversion Code - " << (int)GetBit(m_CustomPCF_1,8)
			     << " , Custom MPI Indicator - "  << (int)GenerateNewByte(m_CustomPCF_2,1,6);

	   }

	   if(GenerateNewByte(m_CustomPCF_2,7,8)) // HRDBPPMaxKB
	   {
				ostr << "\n" << h263_cap_HRD_B_Size[GenerateNewByte(m_HRDBPPMaxKB,1,4)]
				     << " , " << h263_cap_BPPmaxKB[GenerateNewByte(m_HRDBPPMaxKB,5,8)];
	   }

	   if(IsCustomPARFlag())
	   {
			ostr << "\n" << "CustomPixelWidth - " << (int)m_CustomPixelWidth
                 << " , " << "CustomPixelHeight - " << (int)m_CustomPixelHeight << "\n";
	   }
   }

   BYTE bOptionsIndicator = GetOptionsIndicator();
   if (bOptionsIndicator != 5)
   {
	   if( bOptionsIndicator < 8 )
		   ostr << "\n" << h263_OptionsIndicator[bOptionsIndicator];
	   else
		   ostr << "\n BAD OPTION INDICATOR = " << (int)bOptionsIndicator << ";";

	   if(IsOptions_1_FlagOn()) // Options 1 Flag
	   {
		   ostr << "\n";
           if(GetBit(m_Option_1,2))  ostr << "+ Annex I ";
		   if(GetBit(m_Option_1,3))  ostr << "+ Annex J ";
		   if(GetBit(m_Option_1,4))  ostr << "+ Full Picture Freeze ";
		   if(GetBit(m_Option_1,5))  ostr << "+ Annex T ";
		   if(GetBit(m_Option_1,6))  ostr << "+ Annex D ";
		   if(GetBit(m_Option_1,7))  ostr << "+ Annex P ";
		   if(GetBit(m_Option_1,8))  ostr << "+ Annex N ";
	   }

	   if(IsOptions_2_FlagOn()) // Options 2 Flag
	   {
		   ostr << "\n";
           if(GetBit(m_Option_2,2))  ostr << "+ Annex K ";
		   if(GetBit(m_Option_2,3))  ostr << "+ Annex R ";
		   if(GetBit(m_Option_2,4))  ostr << "+ Reduced Resolution Update ";
		   if(GetBit(m_Option_2,5))  ostr << "+ Annex L ";
		   if(GetBit(m_Option_2,6))  ostr << "+ Annex M ";
		   if(GetBit(m_Option_2,7))  ostr << "+ Partial Picture Freeze And Release ";
		   if(GetBit(m_Option_2,8))  ostr << "+ Annex S ";
	   }

	   if(IsOptions_3_FlagOn()) // Options 3 Flag
	   {
		   ostr << "\n";
           if(GenerateNewByte(m_Option_2,1,2))   ostr << ((GenerateNewByte(m_Option_2,1,2)<4) ? h263_DynamicWarping[GenerateNewByte(m_Option_2,1,2)] : "ERROR") << "  ";
		   if(GetBit(m_Option_3,3))  ostr << "+ Full Picture Snapshot ";
		   if(GetBit(m_Option_3,4))  ostr << "+ Partial Picture Snapshot ";
		   if(GetBit(m_Option_3,5))  ostr << "+ Video Segment Tagging ";
		   if(GetBit(m_Option_3,6))  ostr << "+ Progressive Refinement ";
		   if(GetBit(m_Option_3,7))  ostr << "+ Dynamic Picture Resizing Sixteenth Pel ";
		   if(GetBit(m_Option_3,8))  ostr << "+ Temporal Spatial Trade Off Capability ";
	   }

	   if(IsRefSliceParameters())
	   {
		   ostr << "\n";
		   ostr << "+ Video back channel - "<< ((GenerateNewByte(m_RefSliceParameters,1,3) < 8) ?
				   h263_VideoBackChannel[GenerateNewByte(m_RefSliceParameters,1,3)] : "ERROR");

		   if(GenerateNewByte(m_RefSliceParameters,4,6))
			   ostr << "\n" << "+ Number of Additional Picture Memories - "
				    << (int)GenerateNewByte(m_RefSliceParameters,4,6) << "\n";

		   ostr << "+ " << ((GenerateNewByte(m_RefSliceParameters,7,8)<4) ?
				   h263_SliceType[GenerateNewByte(m_RefSliceParameters,7,8)] : "ERROR");
	   }

	   if(IsScalabilityDescriptor_FlagOn())
	   {
		   ostr << "\n";
		   BYTE NumberOfScalableLayers = GetNumberOfScalableLayers();
		   ostr << "Number Of Scalable Layer - " << (int)NumberOfScalableLayers << "\n"
			    << "  Max Band Width Of base Layer - " << ((GenerateNewByte(m_ScalabilityDescriptor,5,8)<16) ?
			    		h263_MaxBandWidthOfLayer[GenerateNewByte(m_ScalabilityDescriptor,5,8)] : "ERROR");

		   for(int i = 0; i < NumberOfScalableLayers; i++)
		   {
			   ostr << "\n";
			   ostr << "  Enhancement Layer (" << i+1 << ")" << "\n"
				    << "  " << ((GenerateNewByte(m_EnhancementLayerInfo[i],1,4)<16) ?
				    		h263_MaxBandWidthOfEnhancementLayer[GenerateNewByte(m_EnhancementLayerInfo[i],1,4)] : "ERROR") << "\n";

			   if(GetBit(m_EnhancementLayerInfo[i],5))  ostr << "  Spatial Scalable 1D  ";
			   if(GetBit(m_EnhancementLayerInfo[i],6))  ostr << "  Spatial Scalable 2D  ";
			   if(GetBit(m_EnhancementLayerInfo[i],7))  ostr << "  SNR Scalable  ";
			   if(GetBit(m_EnhancementLayerInfo[i],8))  ostr << "  Temporal Scalable  ";
		   }
	   }

	   if(IsErrorCompensation_FlagOn())  // Error Compensation.
	   {
		  ostr << "\n" << "+ Error Compensation";
	   }
   }

  ostr << "\n";

   //Second additinal capabilities.
   if(m_SecondAdditionalH263Cap == 0x40)
	   ostr << "No second additional capabilities are supported";
   else
   {
	   if(GenerateNewByte(m_SecondAdditionalH263Cap,1,2) != 1)// 1 means not capable.
	   {
		    if(GenerateNewByte(m_SecondAdditionalH263Cap,1,2) < 4)
				ostr << h263_EnhancedReferencePicSelect[GenerateNewByte(m_SecondAdditionalH263Cap,1,2)] << "\n";
		    else
		    	ostr << "invalid value GenerateNewByte(m_SecondAdditionalH263Cap,1,2) = " << (int)GenerateNewByte(m_SecondAdditionalH263Cap,1,2) << "\n";
	   }

	   if(GetBit(m_SecondAdditionalH263Cap,4))
		   ostr << "Capable of using dataPartionedSlices  ";
	   if(GetBit(m_SecondAdditionalH263Cap,5))
		   ostr << "Capable of using FixedPointIDCT0  ";
	   if(GetBit(m_SecondAdditionalH263Cap,6))
		   ostr << "Capable of using interlacedFields  ";
	   if(GetBit(m_SecondAdditionalH263Cap,7))
		   ostr << "Capable of using currentPictureHeaderRepetition  ";
	   if(GetBit(m_SecondAdditionalH263Cap,8))
		   ostr << "secondOptionExtByte follows  ";
   }

   ostr << "\n";
}
/////////////////////////////////////////////////////////////////////////////
void  CCapSetH263::CreateDefault()
{
     m_vidImageFormat  = (BYTE)0;
	 m_mpi			   = (BYTE)0;
  	 m_optionalCap     = (BYTE)0;
  	 m_HRDbAndBPPmaxKb = (BYTE)0;
	 m_AdditionalH263Cap = (BYTE)5; // No additional h.263 options are supported.
	 m_MinCustomPictureHeight = (BYTE)0;
	 m_MinCustomPictureWidth =  (BYTE)0;
	 m_MaxCustomPictureHeight = (BYTE)0;
	 m_MaxCustomPictureWidth =  (BYTE)0;
	 m_CustomPCF_1 =  (BYTE)0;
	 m_CustomPCF_2 =  (BYTE)0;
     m_HRDBPPMaxKB =  (BYTE)0;
     m_CustomPixelWidth =  (BYTE)0;
	 m_CustomPixelHeight =  (BYTE)0;
	 m_IndividualOptionIndicator =  (BYTE)0;
     m_Option_1 =  (BYTE)0;
	 m_Option_2 =  (BYTE)0;
	 m_Option_3 =  (BYTE)0;
	 m_RefSliceParameters =  (BYTE)0;
	 m_ScalabilityDescriptor =  (BYTE)0;
	 m_SecondAdditionalH263Cap = (BYTE)0x40;// No second additinal capabilities are supported

     for( WORD Index = 0; Index < MAX_ENHANCEMENT_LAYERS ; Index++)
	     m_EnhancementLayerInfo[Index] = (BYTE)0;
}

/////////////////////////////////////////////////////////////////////////////
void  CCapSetH263::Create( BYTE capBuffer[] )
{
	m_vidImageFormat = capBuffer[VideoFormat];

	if(m_vidImageFormat != H263_CUSTOM_FORMAT) // Standart Format.
	    m_mpi = capBuffer[MPI];

	if( m_vidImageFormat > (BYTE)H263_CIF_16 && m_vidImageFormat != H263_CUSTOM_FORMAT)
		m_vidImageFormat = FORBIDDEN_IMAGE_FORMAT_H263;
	if( m_mpi > (BYTE)MPI_30 )
		m_mpi = FORBIDDEN_MPI_H263;

	m_optionalCap = (BYTE)0;
	SetUMV(capBuffer[UMV]);
	SetAMP(capBuffer[AMP]);
	SetAC(capBuffer[AC]);
	SetPB(capBuffer[PB]);
	SetHRDb(capBuffer[HRD_B]);
	SetBPPmaxKb(capBuffer[BPPmaxKB]);

	SetVideoFormatResolution( capBuffer[MinPictureHeight] , capBuffer[MinPictureWidth] );

	if(m_vidImageFormat > H263_CIF_16 || capBuffer[InterlaceMode])
	{
		SetClockDivisor(capBuffer[ClockDivisor]);
        SetClockConversionCode(capBuffer[ClockConversionCode]);
	    SetCustomMPIIndicator(capBuffer[CustomMPIIndicator]) ;
	}

	//Only for custom formats
	if(m_vidImageFormat > H263_CIF_16)
	    SetFormatIndicator(2); // 2 indicates on Custom Format.

		//Annexes
	SetAnnexP(capBuffer[Annex_P]);
    SetAnnexN(capBuffer[Annex_N]);
    SetAnnexT(capBuffer[Annex_T]);

	//Interlace Mode
	SetInterlaceMode(capBuffer[InterlaceMode]);
	SetAnnexL(capBuffer[Annex_L]);
}
/////////////////////////////////////////////////////////////////////////////
void  CCapSetH263::Serialize(WORD format,CSegment& seg)
{

	switch ( format )
	{

	case SERIALEMBD :
		{
	seg << GetBaseCapsH263();
	if( GetOptionalCap() )
	    seg << m_optionalCap;
	if( IsSpecifyHRD_B() || IsSpecifyBPPmaxKb() )
        seg << m_HRDbAndBPPmaxKb;
	  break;
					 }

	case NATIVE :
		{
	seg << GetBaseCapsH263();
	if( GetOptionalCap() )
	    seg << m_optionalCap;
	if( IsSpecifyHRD_B() || IsSpecifyBPPmaxKb() )
        seg << m_HRDbAndBPPmaxKb;
	  break;
					 }

	case ADDITIONAL_CAPABILITIES :
		{
			seg << m_AdditionalH263Cap;
            switch(GetFormatIndicator())
			{
			case STANDARD_FORMAT_ADDITIONAL_CAP :  // Do nothing
				{
					break;
				}

			case CUSTOM_FORMAT_CAP_EQUAL_BOUNDS :
				{
					seg << m_MinCustomPictureHeight << m_MinCustomPictureWidth ;
 					break;
				}

			case CUSTOM_FORMAT_CAP_TWO_DISTINCT_BOUNDS :
				{
					seg << m_MinCustomPictureHeight << m_MinCustomPictureWidth
						<< m_MaxCustomPictureHeight << m_MaxCustomPictureWidth;
					break;
				}

			default :
				{
					PASSERT(GetFormatIndicator()); // Standard : Value of 1 is not allowed.
				}
			}


			if(IsCustomPCFFlag())
			{
			   seg << m_CustomPCF_1 << m_CustomPCF_2;
			   if(IsHRDBPPMaxKB())
				   seg << m_HRDBPPMaxKB;
			}

			if(IsCustomPARFlag())
			   seg << m_CustomPixelWidth << m_CustomPixelHeight;

			if(!GetOptionsIndicator())
			{
				seg << m_IndividualOptionIndicator;

				if(IsOptions_1_FlagOn())
					seg << m_Option_1;

				if(IsOptions_2_FlagOn())
					seg << m_Option_2;

				if(IsOptions_3_FlagOn())
					seg << m_Option_3;

				if(IsRefSliceParameters())
					seg << m_RefSliceParameters;

				if(IsScalabilityDescriptor_FlagOn())
				{
					seg << m_ScalabilityDescriptor;
					BYTE NumberOfScalableLayers = GetNumberOfScalableLayers();
				    for( BYTE Index = 0; Index < NumberOfScalableLayers ; Index++)
					    seg << GetEnhancementLayerInfo(Index);
				}
			}

			break;
		}

	case SECOND_ADDITIONAL_CAPABILITIES:
		{
			seg << m_SecondAdditionalH263Cap;
			break;
		}


	   default :{
		   if(format)
		       DBGPASSERT(format);
		   else
			   DBGPASSERT(101);
		   break;
				}
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CCapSetH263::DeSerialize(WORD format,CSegment& seg)
{
	BYTE baseCap;
	switch ( format )  {

	case SERIALEMBD :{
			seg >> baseCap;
			SetBaseCapsH263(baseCap);
			if ( GetBit(baseCap,8) )
				seg	>> m_optionalCap;
			if ( GetBit(m_optionalCap,7) ||  GetBit(m_optionalCap,7) )
				seg >> m_HRDbAndBPPmaxKb;
			   break;
					 }
	   case NATIVE     :{

			seg >> baseCap;
			SetBaseCapsH263(baseCap);
			if ( GetBit(baseCap,8) )
				seg	>> m_optionalCap;
			if ( GetBit(m_optionalCap,7) ||  GetBit(m_optionalCap,7) )
				seg >> m_HRDbAndBPPmaxKb;
		       break;
						}
	   default :
		   { break; }
	}
}
/////////////////////////////////////////////////////////////////////////////
CCapSetH263& CCapSetH263::operator=(const CCapSetH263 &other)
{
	if(this == &other){
	    return *this;
	}

	m_vidImageFormat	= other.m_vidImageFormat;
    m_mpi				= other.m_mpi;
	m_optionalCap		= other.m_optionalCap;
    m_HRDbAndBPPmaxKb	= other.m_HRDbAndBPPmaxKb;
    m_AdditionalH263Cap = other.m_AdditionalH263Cap;
	m_MinCustomPictureHeight = other.m_MinCustomPictureHeight;
	m_MinCustomPictureWidth = other.m_MinCustomPictureWidth;
	m_MaxCustomPictureHeight = other.m_MaxCustomPictureHeight;
	m_MaxCustomPictureWidth = other.m_MaxCustomPictureWidth;
	m_CustomPCF_1 = other.m_CustomPCF_1;
	m_CustomPCF_2 = other.m_CustomPCF_2;
	m_HRDBPPMaxKB = other.m_HRDBPPMaxKB;
	m_CustomPixelWidth = other.m_CustomPixelWidth;
	m_CustomPixelHeight = other.m_CustomPixelHeight;
	m_IndividualOptionIndicator = other.m_IndividualOptionIndicator;
	m_Option_1 = other.m_Option_1;
	m_Option_2 = other.m_Option_2;
	m_Option_3 = other.m_Option_3;
	m_RefSliceParameters= other.m_RefSliceParameters;
	m_ScalabilityDescriptor = other.m_ScalabilityDescriptor;
	m_SecondAdditionalH263Cap = other.m_SecondAdditionalH263Cap;

	for( BYTE Index = 0; Index < MAX_ENHANCEMENT_LAYERS ; Index++)
		m_EnhancementLayerInfo[Index] = other.m_EnhancementLayerInfo[Index];

	return *this;
}
/////////////////////////////////////////////////////////////////////////////
// support for given CapSetH263 with Two distinct bounds added (v7 , Ron)
WORD CCapSetH263::IsComprisesCustomFormat(const CCapSetH263* CapSetH263) const
{
	WORD rvel = FALSE;

	if(CapSetH263->m_MaxCustomPictureHeight == 0 && CapSetH263->m_MaxCustomPictureWidth == 0)
	{	// The given CapSetH263 has equal bounds
		//LOCAL custom format has two distinct bounds.
		if(m_MaxCustomPictureWidth && m_MaxCustomPictureHeight)
		{
			if( CapSetH263->m_MinCustomPictureWidth <= m_MaxCustomPictureWidth   && // X
				CapSetH263->m_MinCustomPictureWidth >= m_MinCustomPictureWidth   &&
				CapSetH263->m_MinCustomPictureHeight <= m_MaxCustomPictureHeight && // Y
				CapSetH263->m_MinCustomPictureHeight >= m_MinCustomPictureHeight)
			{
				rvel = TRUE;
			}
		}
	}else if(CapSetH263->m_MaxCustomPictureHeight != 0 && CapSetH263->m_MaxCustomPictureWidth != 0)
	{	// The given CapSetH263 has Two distinct bounds
		BYTE min_in_bounds = NO;
		BYTE max_in_bounds = NO;
		if( CapSetH263->m_MinCustomPictureWidth <= m_MaxCustomPictureWidth   && // X
			CapSetH263->m_MinCustomPictureWidth >= m_MinCustomPictureWidth   &&
			CapSetH263->m_MinCustomPictureHeight <= m_MaxCustomPictureHeight && // Y
			CapSetH263->m_MinCustomPictureHeight >= m_MinCustomPictureHeight)
		{
			min_in_bounds = YES;
		}

		if( CapSetH263->m_MaxCustomPictureWidth <= m_MaxCustomPictureWidth   && // X
			CapSetH263->m_MaxCustomPictureWidth >= m_MinCustomPictureWidth   &&
			CapSetH263->m_MaxCustomPictureHeight <= m_MaxCustomPictureHeight && // Y
			CapSetH263->m_MaxCustomPictureHeight >= m_MinCustomPictureHeight)
		{
			 max_in_bounds = YES;
		}
		if(min_in_bounds && max_in_bounds)
		{
			rvel = TRUE;
		}
	}else{
		PASSERT(CapSetH263->m_MaxCustomPictureHeight);
		PASSERT(CapSetH263->m_MaxCustomPictureWidth);
	}

	return rvel;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetOptionalCaps(BYTE optional)
{
		// the first two bits of options byte must be zero!
	if(GetBit(optional,1))
		DBGPASSERT( GetBit(optional,1));
	if(GetBit(optional,2))
		DBGPASSERT( GetBit(optional,2));
	m_optionalCap = optional;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetBaseCapsH263(BYTE base)
{
	// the first bit of baseline byte must be 1 according to standard!
	DBGPASSERT( !GetBit(base,1) );
	SetVidImageFormat(base);
	SetMpiFromBaseByte(base);
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetHRDbAndBPPmaxKb(BYTE newHRDbAndBPPmaxKb)
{
	// check if the values of HRDb and BPPmaxKb are valid
	if(GenerateNewByte(newHRDbAndBPPmaxKb,1,4) > (BYTE)HRD_B_x_256)
		PASSERT(GenerateNewByte(newHRDbAndBPPmaxKb,1,4));
	if(GenerateNewByte(newHRDbAndBPPmaxKb,5,8) > (BYTE)BPPmax_x_256)
		PASSERT( GenerateNewByte(newHRDbAndBPPmaxKb,5,8) );
	m_HRDbAndBPPmaxKb = newHRDbAndBPPmaxKb;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetAdditionalH263Cap(BYTE AdditionalH263Cap)
{
	// Third bit must be 0 according to the standard.
	DBGPASSERT(GetBit(AdditionalH263Cap,3));
	m_AdditionalH263Cap = AdditionalH263Cap;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetIndividualOptionIndicator(BYTE IndividualOptionIndicator)
{
	//First bit must be 0 according to the standard.
	DBGPASSERT(GetBit(IndividualOptionIndicator,1));
	m_IndividualOptionIndicator = IndividualOptionIndicator;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetOption1Cap(BYTE Option1Cap)
{
	//First bit must be 0 according to the standard.
	DBGPASSERT(GetBit(Option1Cap,1));
	m_Option_1 = Option1Cap;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetOption2Cap(BYTE Option2Cap)
{
	//First bit must be 0 according to the standard.
	DBGPASSERT(GetBit(Option2Cap,1));
	m_Option_2 = Option2Cap;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetOption3Cap(BYTE Option3Cap)
{
	//Two first bits are not allow to be 11 according to the standard.
	PASSERT(GenerateNewByte(Option3Cap,1,2) == (BYTE)3);
	m_Option_3 = Option3Cap;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetRefSliceParameters(BYTE RefSliceParameters)
{
	//Three first bits are not allow to be 111 according to the standard.
	PASSERT(GenerateNewByte(RefSliceParameters,1,3) == (BYTE)7);
	m_RefSliceParameters = RefSliceParameters;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetScalabilityDescriptor(BYTE ScalabilityDescriptor)
{
	//Four first bits are not allow to be 1110 / 1111  according to the standard.
	PASSERT(GenerateNewByte(ScalabilityDescriptor,1,4) >= (BYTE)14);
	m_ScalabilityDescriptor = ScalabilityDescriptor;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetEnhancementLayerInfo(BYTE Index,BYTE Cap)
{
	m_EnhancementLayerInfo[Index] = Cap;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetVideoFormatResolution(BYTE MinFrameHeight,BYTE MinFrameWidth,BYTE MaxFrameHeight,BYTE MaxFrameWidth)
{
	// According to the standard : format resolution parameters are :
	// (MinFrameHeight/8 -1 , MinFrameWidth/8 -1 , MaxFrameHeight/8 -1 , MaxFrameWidth/8 -1)
    m_MinCustomPictureHeight = MinFrameHeight;
    m_MinCustomPictureWidth  =  MinFrameWidth;

	if(MaxFrameWidth && MaxFrameHeight) // Default 0
	{
		m_MaxCustomPictureHeight = MaxFrameHeight;
		m_MaxCustomPictureWidth  =  MaxFrameWidth;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetCustomPCF(BYTE CustomPCF_1,BYTE CustomPCF_2)
{
	BYTE ClockDivisor = GenerateNewByte(CustomPCF_1,1,7);
	if(ClockDivisor > 111 ) PASSERT(ClockDivisor); // Standard

	BYTE CustomMPIIndicator = GenerateNewByte(CustomPCF_2,1,6);
    if(CustomMPIIndicator > 55 ) PASSERT(CustomMPIIndicator); // Standard

	m_CustomPCF_1 = CustomPCF_1;
    m_CustomPCF_2 = CustomPCF_2;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetVidImageFormat(BYTE base)
{
	m_vidImageFormat = GenerateNewByte(base,6,7);
	if( m_vidImageFormat > (BYTE)H263_CIF_16 && m_vidImageFormat != H263_CUSTOM_FORMAT) {
		PASSERT(m_vidImageFormat);
		m_vidImageFormat = FORBIDDEN_IMAGE_FORMAT_H263;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetVidImageToCustomFormat()
{
	m_vidImageFormat = H263_CUSTOM_FORMAT;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetSecondAdditionalH263Cap(BYTE SecondAdditionalH263Cap)
{
    m_SecondAdditionalH263Cap = SecondAdditionalH263Cap;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetHRDBPPMaxKB(BYTE HRDBPPMaxKB)
{
	m_HRDBPPMaxKB = HRDBPPMaxKB;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetCustomPixelWidthAndHeight(BYTE CustomPixelWidth,BYTE CustomPixelHeight)
{
	if(CustomPixelWidth > 223 ) PASSERT(CustomPixelWidth); // Standard
    if(CustomPixelHeight > 223 ) PASSERT(CustomPixelHeight); // Standard

	m_CustomPixelWidth = CustomPixelWidth;
    m_CustomPixelHeight = CustomPixelHeight;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetVidImageFormatValue(WORD newFormatValue)
{
	if( newFormatValue > (BYTE)H263_CIF_16 && newFormatValue != H263_CUSTOM_FORMAT) {
		PASSERT(newFormatValue);
		m_vidImageFormat = FORBIDDEN_IMAGE_FORMAT_H263;
	}
	else
		m_vidImageFormat = newFormatValue;

}

/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetMPI(BYTE mpi)
{
	if( mpi > (BYTE)MPI_30 )
	{
		PASSERT(mpi);
		m_mpi = FORBIDDEN_MPI_H263;
	}
	else
		m_mpi = mpi;
}

/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetMpiFromBaseByte(BYTE base)
{
	m_mpi = GenerateNewByte(base,2,5);
	if( m_mpi > (BYTE)MPI_30 ) {
		PASSERT(m_mpi);
		m_mpi = FORBIDDEN_MPI_H263;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetUMV(BYTE bitValue)
{
		// bit value must be '0' or '1' only
	if(bitValue>1)
		PASSERT( bitValue);
	BYTE temp = bitValue;
	temp = temp << 5;
	m_optionalCap &= 0xDF;
	m_optionalCap |= temp;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetAMP(BYTE bitValue)
{
		// bit value must be '0' or '1' only
	if(bitValue>1)
		PASSERT( bitValue);
	BYTE temp = bitValue;
	temp = temp << 4;
	m_optionalCap &= 0xEF;
	m_optionalCap |= temp;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetAC(BYTE bitValue)
{
		// bit value must be '0' or '1' only
	if(bitValue>1)
		PASSERT( bitValue);
	BYTE temp = bitValue;
	temp = temp << 3;
	m_optionalCap &= 0xF7;
	m_optionalCap |= temp;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetPB(BYTE bitValue)
{
		// bit value must be '0' or '1' only
	if(bitValue>1)
		PASSERT( bitValue);
	BYTE temp = bitValue;
	temp = temp << 2;
	m_optionalCap &= 0xFB;
	m_optionalCap |= temp;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetHRDb(BYTE newHRDb)
{
	if( !newHRDb ) // the default value shall be used
		return;
	if(newHRDb > (BYTE)HRD_B_x_256)
		PASSERT( newHRDb );
	BYTE temp = newHRDb << 4;
	m_HRDbAndBPPmaxKb = GenerateNewByte(m_HRDbAndBPPmaxKb,5,8);
	m_HRDbAndBPPmaxKb |= temp;
	m_optionalCap |= (BYTE)2;	// switch on 'Specify HRD-B' bit of options byte
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetBPPmaxKb(BYTE newBPPmaxKb)
{
	if( !newBPPmaxKb ) // the default value shall be used
		return;
	if(newBPPmaxKb > (BYTE)BPPmax_x_256)
		PASSERT( newBPPmaxKb );
	m_HRDbAndBPPmaxKb = GenerateNewByte(m_HRDbAndBPPmaxKb,1,4);
	m_HRDbAndBPPmaxKb = m_HRDbAndBPPmaxKb << 4;
	m_HRDbAndBPPmaxKb |= newBPPmaxKb;
	m_optionalCap |= (BYTE)1;	// switch on 'Specify BPPmaxKB' bit of options byte
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetClockDivisor(BYTE bitValue)
{
	BYTE temp = (BYTE)1 << 4;// switch on 'Custom PCF Flag' bit
	m_AdditionalH263Cap |= temp;

	m_CustomPCF_1 |= bitValue << 1;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetClockConversionCode(BYTE OnOff)
{
	BYTE temp = (BYTE)1 << 4;// switch on 'Custom PCF Flag' bit
	m_AdditionalH263Cap |= temp;

    m_CustomPCF_1 |= OnOff;

}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetCustomMPIIndicator(BYTE bitValue)
{
	BYTE temp = (BYTE)1 << 4;// switch on 'Custom PCF Flag' bit
	m_AdditionalH263Cap |= temp;

	m_CustomPCF_2 |= bitValue << 2;
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetInterlaceMode(BYTE InterlaceMode)
{
	if(InterlaceMode)
	{
		BYTE temp = (BYTE)1 << 2;
		m_SecondAdditionalH263Cap |= temp;
	}
	else
	{
		m_SecondAdditionalH263Cap &= 0xFB; // Turn off interlace mode
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetEnhancedReferencePicSelect(BYTE EnhancedReferencePicSelect)
{
	m_SecondAdditionalH263Cap &= 0x3F; // EnhancedReferencePicSelect is 00
	m_SecondAdditionalH263Cap |= EnhancedReferencePicSelect;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetEnhancedReferencePicSelect()const
{
	return GenerateNewByte(m_SecondAdditionalH263Cap,1,2);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetInterlaceMode()const
{
	return GetBit(m_SecondAdditionalH263Cap,6);
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetAnnexP(BYTE OnOff)
{

	if(OnOff) // ON
	{
		SetOptionsIndicator(0);  // Indicates optional capabilities.
		m_IndividualOptionIndicator |= (BYTE)1 << 6; // switch on 'Option Byte 1 Flag' bit.
		m_Option_1 |= (BYTE)1 << 1; // switch on 'Dynamic Picture Resizing By Four' bit.
	}
	else // OFF
	{
       m_Option_1 &= (BYTE)0xFD; // switch off 'Dynamic Picture Resizing By Four' bit.
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetAnnexN(BYTE OnOff)
{
	if(OnOff) // ON
	{
		SetOptionsIndicator(0);  // Indicates optional capabilities.
		m_IndividualOptionIndicator |= (BYTE)0x44; // switch on 'Option Byte 1 Flag' bit + Error Compensation.
		m_Option_1 |= (BYTE)1; // switch on 'Ref Picture Selection' bit.
		m_RefSliceParameters = 0x84; // Set videoBackChannel=NONE,  additionalPictureMemory = 1, sliceType = 0
	}
	else // OFF
	{
		m_IndividualOptionIndicator &= (BYTE)0xFB;  // switch of 'Error Compensation'.
        m_Option_1 &= (BYTE)0xDF; // switch of 'Deblocking Filter Freeze' bit + Error Compensation.
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetAnnexL(BYTE OnOff)
{
	if(OnOff) // ON
	{
		SetOptionsIndicator(0);  // Indicates optional capabilities.
		m_IndividualOptionIndicator |= (BYTE)1 << 6; // switch on 'Option Byte 1 Flag' bit.
		m_Option_1 |= (BYTE)1 << 4; // switch on 'Full picture freeze' bit.
	}
	else // OFF
	{
       m_Option_1 &= (BYTE)0xEF; // switch off 'Full picture freeze' bit.
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetAnnexT(BYTE OnOff)
{

	if(OnOff) // ON
	{
		SetOptionsIndicator(0);  // Indicates optional capabilities.
		m_IndividualOptionIndicator |= (BYTE)1 << 6; // switch on 'Option Byte 1 Flag' bit.
		m_Option_1 |= (BYTE)1 << 3; // switch on 'Modified Quantization Mode' bit.
	}
	else // OFF
	{
       m_Option_1 &= (BYTE)0xF7; // switch off 'Modified Quantization Mode' bit.
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetSetLen() const
{
	BYTE len = 1;	// we assume that h263Set alaws contanes baseline h263 caps
	if(  ! IsOptionalCap() )
		return len;
	++len;
	if( IsSpecifyHRD_B() || IsSpecifyBPPmaxKb() )
		++len;
	return len;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetBaseCapsH263() const
{
	BYTE baselineCap = 0;

	baselineCap = m_mpi;
	baselineCap = baselineCap << 3;
	baselineCap |= (BYTE)1 << 7;
	if( IsOptionalCap() )
		baselineCap |= (BYTE)1;
	BYTE temp = m_vidImageFormat;
	baselineCap |= (temp << 1);
	return baselineCap;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetVidImageFormat() const
{
	return m_vidImageFormat;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetCustomVidImageFormat() const
{
	BYTE format = NUMBER_OF_H263_FORMATS; // will return if general custom format
	if(m_vidImageFormat == H263_CUSTOM_FORMAT)
	{
		if(m_MinCustomPictureWidth == 79 && m_MinCustomPictureHeight == 59)
			format =  VGA;
		if(m_MinCustomPictureWidth == 87 && m_MinCustomPictureHeight == 59)
			format =  NTSC;
		if(m_MinCustomPictureWidth == 99 && m_MinCustomPictureHeight == 74)
			format =  SVGA;
		if(m_MinCustomPictureWidth == 127 && m_MinCustomPictureHeight == 95)
			format =  XGA;
	}else{
		format =  m_vidImageFormat; // not valid format value
	}
	return format;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetCustomVidImageFormatWidthAndHight(BYTE& MinCustomPictureWidth,BYTE& MaxCustomPictureWidth,BYTE& MinCustomPictureHeight,BYTE& MaxCustomPictureHeight) const
{
	MinCustomPictureWidth = m_MinCustomPictureWidth;
	MaxCustomPictureWidth = m_MaxCustomPictureWidth;
	MinCustomPictureHeight = m_MinCustomPictureHeight;
	MaxCustomPictureHeight = m_MaxCustomPictureHeight;
	return m_vidImageFormat;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetMPI() const
{
	if(m_vidImageFormat == H263_CUSTOM_FORMAT)
		return GetCustomMPIIndicator();
	else
	return m_mpi;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsOptionalCap() const
{
	PASSERT( IsCPM() );
	if( m_optionalCap )
		return 1;
	else
		return 0;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsSpecifyHRD_B() const
{
	return GetBit(m_optionalCap,7);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsSpecifyBPPmaxKb() const
{
	return GetBit(m_optionalCap,8);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsCPM() const
{
	return GetBit(m_optionalCap,2);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsUMV() const
{
	return GetBit(m_optionalCap,3);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsAMP() const
{
	return GetBit(m_optionalCap,4);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsAC() const
{
	return GetBit(m_optionalCap,5);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsPB() const
{
	return GetBit(m_optionalCap,6);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsRefSliceParameters() const
{
	//If one of this bytes is ON - RefSliceParameters byte should be present.
	return(GetBit(m_Option_1,8) || GetBit(m_Option_2,2));
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetHRDb() const
{
	return GenerateNewByte(m_HRDbAndBPPmaxKb,1,4);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetBPPmaxKb() const
{
	return GenerateNewByte(m_HRDbAndBPPmaxKb,5,8);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetOptionalCap() const
{
	return m_optionalCap;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetHRDbAndBPPmaxKb() const
{
	return m_HRDbAndBPPmaxKb;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetNumberOfScalableLayers() const
{
	//Four first bits represent the number of scalable layers.
	return GenerateNewByte(m_ScalabilityDescriptor,1,4) + 1;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetFormatIndicator() const
{
     return GenerateNewByte(m_AdditionalH263Cap,1,2);
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetFormatIndicator(BYTE FormatIndicator)
{
	m_AdditionalH263Cap = m_AdditionalH263Cap & 0x3F;// Initial FormatIndicator bits with 0.
    BYTE temp = FormatIndicator << 6;
    m_AdditionalH263Cap = m_AdditionalH263Cap |= temp;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsCustomPCFFlag() const
{
     return GetBit(m_AdditionalH263Cap,4);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsCustomPARFlag() const
{
     return GetBit(m_AdditionalH263Cap,5);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsHRDBPPMaxKB() const
{
     return GenerateNewByte(m_CustomPCF_2,7,8);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetOptionsIndicator() const
{
     return GenerateNewByte(m_AdditionalH263Cap,6,8);
}
/////////////////////////////////////////////////////////////////////////////
void CCapSetH263::SetOptionsIndicator(BYTE OptionsIndicator)
{
	if(OptionsIndicator > 7)
		PASSERT(OptionsIndicator);
	m_AdditionalH263Cap = m_AdditionalH263Cap & 0xF8; // Option indicator bits are 0.
    m_AdditionalH263Cap = m_AdditionalH263Cap |= OptionsIndicator;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsOptions_1_FlagOn() const
{
     return GetBit(m_IndividualOptionIndicator,2);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsOptions_2_FlagOn() const
{
     return GetBit(m_IndividualOptionIndicator,3);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsOptions_3_FlagOn() const
{
     return GetBit(m_IndividualOptionIndicator,4);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsScalabilityDescriptor_FlagOn() const
{
     return GetBit(m_IndividualOptionIndicator,5);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsErrorCompensation_FlagOn() const
{
     return GetBit(m_IndividualOptionIndicator,6);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetEnhancementLayerInfo(BYTE Index) const
{
	return m_EnhancementLayerInfo[Index];
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetMinCustomPictureHeight() const
{
	return m_MinCustomPictureHeight;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetMinCustomPictureWidth() const
{
	return m_MinCustomPictureWidth;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetClockDivisor() const
{
	return GenerateNewByte(m_CustomPCF_1,1,7);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetClockConversionCode() const
{
	return GetBit(m_CustomPCF_1,8);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetCustomMPIIndicator() const
{
	return GenerateNewByte(m_CustomPCF_2,1,6);
}
/////////////////////////////////////////////////////////////////////////////
WORD CCapSetH263::CalculateFramePerSecond() const
{
	WORD CustomClockFrequency = 1800000 / (GetClockDivisor() * (GetClockConversionCode() + 1000));

	WORD FramePerSecond = CustomClockFrequency / (GetCustomMPIIndicator() + 1);

	return FramePerSecond;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::GetSecondAdditionalH263Cap() const
{
	return m_SecondAdditionalH263Cap;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsAnnex_P() const
{
	return GetBit(m_Option_1,7);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsAnnex_L() const
{
	if(GetBit(m_IndividualOptionIndicator, 2) && GetBit(m_Option_1, 4))
	   return 1;
	else
       return 0;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsAnnex_N() const
{
	if(GetBit(m_IndividualOptionIndicator,6) && GetBit(m_Option_1,8))
	   return 1;
	else
       return 0;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsAnnex_T() const
{
	return GetBit(m_Option_1,5);

}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsCapSetH263Valid() const
{
	if( GetBit(m_optionalCap,1) || GetBit(m_optionalCap,2))
	    return 0;
	if( FORBIDDEN_IMAGE_FORMAT_H263 == m_vidImageFormat || FORBIDDEN_MPI_H263 == m_mpi )
	    return 0;
	if( GetBit(m_AdditionalH263Cap,3) )
	    return 0;
	if( GetBit(m_IndividualOptionIndicator,1) )
	    return 0;
	if( GetBit(m_Option_1,1) )
	    return 0;
	if( GetBit(m_Option_2,1) )
	    return 0;
	if( GenerateNewByte(m_Option_3, 1, 2) == (BYTE)3)
	    return 0;
	if( GenerateNewByte(m_RefSliceParameters,1,3) == (BYTE)7 )
	    return 0;
	if( GenerateNewByte(m_ScalabilityDescriptor,1,4) >= (BYTE)14 )
	    return 0;
	if( (GenerateNewByte(m_HRDbAndBPPmaxKb,1,4) > (BYTE)HRD_B_x_256) && GenerateNewByte(m_HRDbAndBPPmaxKb,1,4) )
	    return 0;
	if( (GenerateNewByte(m_HRDbAndBPPmaxKb,5,8) > (BYTE)BPPmax_x_256) && GenerateNewByte(m_HRDbAndBPPmaxKb,5,8) )
	    return 0;
	if( m_CustomPixelWidth > 223 || m_CustomPixelHeight > 223 )
	    return 0;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
CCapH263::~CCapH263()	// destructor
{
    ClearAndDestroy();
	delete m_capH263;
}
/////////////////////////////////////////////////////////////////////////////
const char* CCapH263::NameOf()  const
{
	return "CCapH263";
}
/////////////////////////////////////////////////////////////////////////////
void CCapH263::Dump(void) const
{
	std::ostringstream  msg;
    msg << "\nCCapH263::Dump\n"
		<< "-----------\n"
		<< setw(20) << "m_H263_2000" << (dec) << (WORD)m_H263_2000 << "\n";

	if(m_numberOfH263Sets == 0)
	  return;

	int i=0;
    std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr, i++ )
	  {
		  msg << "\n\tCCapSetH263::Dump ----- Set number " << i+1 << "\n" ;
		  msg <<     "\t------------------------------------\n" ;
		  CCapSetH263* pCapSetH263 = (*itr);
		  pCapSetH263->Dump(msg);
	  }
	msg << "\n=============== CCapH263::Dump Finished!!! ===============\n" ;
	msg << "\n";
	PTRACE(eLevelInfoNormal,(char*)msg.str().c_str());
}
/////////////////////////////////////////////////////////////////////////////
void CCapH263::Dump(std::ostream& ostr) const
{
	ostr << "\n==================    CCapH263::Dump    ==================\n"
		 << setw(20) << "m_H263_2000" << (dec) << (WORD)m_H263_2000 << "\n";

	if(m_numberOfH263Sets == 0)
	    return;
	int i=0;
    std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr, i++ )
	  {
		  ostr << "\n\t CCapSetH263::Dump ----- Set number " << i+1 << "\n" ;
		  ostr <<     "\t------------------------------------\n" ;
          CCapSetH263* pCapSetH263 = (*itr);//(at(i));
		  pCapSetH263->Dump(ostr);
	  }
	ostr << "\n=============== CCapH263::Dump Finished!!! ===============\n" ;
	ostr << "\n";
}
/////////////////////////////////////////////////////////////////////////////
void CCapH263::SmartDump(std::ostream& ostr) const
{
	ostr << "\n==================    CCapH263::Dump    ==================\n"
		 << setw(20) << "m_H263_2000" << (dec) << (WORD)m_H263_2000 << "\n";

	BYTE optional = 0;
    std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
		BYTE videoImageFormat = (*itr)->GetVidImageFormat();
		BYTE mpi = (*itr)->GetMPI();

		ostr << ( videoImageFormat<6 ? h263_cap_Format[videoImageFormat] : "ERROR")
		     << " at " << ( mpi<16 ? h263_cap_MPI[mpi] : "ERROR")
			 << "\n";
		if((*itr)->IsUMV()) { ostr << "+UMV "; optional=1; }
		if((*itr)->IsAMP()) { ostr << "+AMP (Annex F)"; optional=1; }
		if((*itr)->IsAC())  { ostr << "+AC ";  optional=1; }
		if((*itr)->IsPB())  { ostr << "+PB";   optional=1; }
		if( optional ) { ostr << "\n"; optional = 0; }
	}
}
/////////////////////////////////////////////////////////////////////////////
void CCapH263::DumpCapH263(std::ostream& ostr) const
{

    ostr << "\nCCapH263::Dump\n"
		 << "-----------\n"
		 << setw(20) << "m_H263_2000" << (dec) << (WORD)m_H263_2000 << "\n";

    std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
		BYTE videoImageFormat = (*itr)->GetVidImageFormat();
		BYTE mpi = (*itr)->GetMPI();

		ostr << "H263_"
		     << (videoImageFormat<6 ? h263_cap_Format[videoImageFormat] : "ERROR")
		     << "\n"
			 << ( mpi<16 ? h263_cap_MPI[mpi] : "ERROR")
			 << ",";
		ostr << (int)(*itr)->IsUMV()
		     << (int)(*itr)->IsAMP()
		     << (int)(*itr)->IsAC()
		     << (int)(*itr)->IsPB();
		ostr << "\n";
		if( (*itr)->IsSpecifyHRD_B() || (*itr)->IsSpecifyBPPmaxKb() ){
			if( (*itr)->GetHRDb() < 16 )
				ostr << h263_cap_HRD_B_Size[(int)(*itr)->GetHRDb()]  << "\n";
			else
				ostr << "illegal value (*itr)->GetHRDb() = " << (*itr)->GetHRDb() << "\n";
			if((*itr)->GetBPPmaxKb() < 16 )
				ostr << h263_cap_BPPmaxKB[(int)(*itr)->GetBPPmaxKb()] << "\n";
			else
				ostr << "illegal value (*itr)->GetBPPmaxKb() = " << (*itr)->GetBPPmaxKb() << "\n";
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CCapH263::CreateDefault()
{
	m_numberOfH263Sets = (WORD)0;
}
/////////////////////////////////////////////////////////////////////////////
void  CCapH263::Create(CSegment& seg,BYTE mbeLen)
{
	BYTE cap,numberOfH263CapBytes,IsErrorEndOfSegment,readBytes = 0;
	m_numberOfH263Sets = 0;
	OFF(IsErrorEndOfSegment);

	PASSERT_AND_RETURN(!mbeLen);	//wrong lenght MBE H263 message.
	numberOfH263CapBytes = mbeLen -1;

	ClearAndDestroy(); // Remove all old CCapSetH263.
    CCapSetH263* NewCCapSetH263 = NULL;
	CSegment* CopyOfseg = new CSegment(seg);

    while( readBytes < numberOfH263CapBytes  )
	{
		if( seg.EndOfSegment() ) {
			// Wrong message length.
			PTRACE(eLevelError,"CCapH263::Create - BAD H263 CAP SEGMENT RECEIVED.");
			ON(IsErrorEndOfSegment);
			break;
		}

		seg >> cap;
		readBytes++;
		if( IsExtensionCode(cap) )
		{
			CreateAdditionalCapabilities(seg,numberOfH263CapBytes - readBytes,IsErrorEndOfSegment);
            break;
		}

		NewCCapSetH263 = new CCapSetH263;
		NewCCapSetH263->SetBaseCapsH263(cap); // Set H.263 Format + MPI

        WORD MinFrameHeight=0,MinFrameWidth=0;
		BYTE VideoImageFormat = NewCCapSetH263->GetVidImageFormat();

		switch (VideoImageFormat)  // STANDARD FORMAT
		{
        case H263_QCIF_SQCIF :  MinFrameWidth = 176;   MinFrameHeight = 144;    break; // [176 x 144]
		case H263_CIF :         MinFrameWidth = 352;   MinFrameHeight = 288;    break; // [352 x 288]
		case H263_CIF_4 :       MinFrameWidth = 704;   MinFrameHeight = 576;    break; // [704 x 576]
		case H263_CIF_16 :      MinFrameWidth = 1408;  MinFrameHeight = 1152;   break; // [1408 x 1152]

		default : PASSERT(VideoImageFormat);
		}

		// According to the standard : format resolution parameters are :
	    // (MinFrameHeight/8 -1 , MinFrameWidth/8 -1 )
        NewCCapSetH263->SetVideoFormatResolution( (BYTE)(MinFrameHeight/8 -1) , (BYTE)(MinFrameWidth/8 -1) );

		if( OnOptions(cap) )
		{
			if(seg.EndOfSegment()) // Wrong message length.
			{
				ON(IsErrorEndOfSegment);
				InsertH263CapSet(NewCCapSetH263);//m_capH263->insert(NewCCapSetH263);
				break;
			}

			seg >> cap;
			readBytes++;
			NewCCapSetH263->SetOptionalCaps(cap);

			if( OnSpecifyHRDb(cap) || OnSpecifyBPPmaxKb(cap) )
			{
				if(seg.EndOfSegment()) // Wrong message length.
				{
					ON(IsErrorEndOfSegment);
					InsertH263CapSet(NewCCapSetH263);//m_capH263->insert(NewCCapSetH263);
					break;
				}

				seg >> cap;
				readBytes++;
				NewCCapSetH263->SetHRDbAndBPPmaxKb(cap);
			}
		}

		InsertH263CapSet(NewCCapSetH263);//m_capH263->insert(NewCCapSetH263);
		NewCCapSetH263 = NULL;
	}

	m_numberOfH263Sets = m_capH263->size();

	if(IsErrorEndOfSegment)
	{
		DBGPASSERT(mbeLen);
	    PrintH263CapSeg(*CopyOfseg,mbeLen);
	}

	POBJDELETE(CopyOfseg);
}
/////////////////////////////////////////////////////////////////////////////
void  CCapH263::CreateAdditionalCapabilities(CSegment& seg,BYTE AdditionalCapLength,BYTE& IsErrorEndOfSegment)
{
    //PTRACE(eLevelInfoNormal,"CCapH263::CreateAdditionalCapabilities");
	BYTE AdditionalH263Cap;
	BYTE FormatIndicator,readBytes = 0;
	BYTE Cap =0,Cap1=0,Cap2=0,Cap3=0;
    BYTE IndexOfStandardH263Set = 1;
	BYTE NumberOfStandardFormatInInitialPart = m_capH263->size();
	CCapSetH263*   CapSetH263 = NULL;

	while(readBytes < AdditionalCapLength )
	{
		if(seg.EndOfSegment())
		{
			ON(IsErrorEndOfSegment);
			POBJDELETE(CapSetH263);
			break;
		} // Wrong message length.

		seg >> AdditionalH263Cap;
		readBytes++;
		if( IsExtensionCode(AdditionalH263Cap) )
		{
			CreateSecondAdditionalCapabilities(seg,AdditionalCapLength - readBytes,IsErrorEndOfSegment);
		    break; //Standard : The rest of the message shall be ignored.
		}

		BYTE isCapsetAdded = 0;
		FormatIndicator = GenerateNewByte(AdditionalH263Cap,1,2);
		switch(FormatIndicator)
		{

		case STANDARD_FORMAT_ADDITIONAL_CAP :
			{
			    if (0 == NumberOfStandardFormatInInitialPart) {
					ON(IsErrorEndOfSegment);
				    break;
				}
				if (IndexOfStandardH263Set > NumberOfStandardFormatInInitialPart)
				{
					CCapSetH263* LastStandardFormatInInitialPart;
					LastStandardFormatInInitialPart = GetStandardFomatAccordingToInitialOrder(NumberOfStandardFormatInInitialPart);
					if(!LastStandardFormatInInitialPart)
					{
						ON(IsErrorEndOfSegment);
						break;
					}
			        WORD MinFrameHeight=0,MinFrameWidth=0;
					BYTE VideoImageFormat = LastStandardFormatInInitialPart->GetVidImageFormat();
					BYTE NextStandardSmallerVidImgFrmt = VideoImageFormat - 1;
					switch (NextStandardSmallerVidImgFrmt)  // STANDARD FORMAT
					{
						case H263_QCIF_SQCIF :  MinFrameWidth = 176;   MinFrameHeight = 144;    break; // [176 x 144]
						case H263_CIF :         MinFrameWidth = 352;   MinFrameHeight = 288;    break; // [352 x 288]
						case H263_CIF_4 :       MinFrameWidth = 704;   MinFrameHeight = 576;    break; // [704 x 576]
						case H263_CIF_16 :      MinFrameWidth = 1408;  MinFrameHeight = 1152;   break; // [1408 x 1152]

						default : PASSERT(VideoImageFormat);
					}

					// According to the standard : format resolution parameters are :
					// (MinFrameHeight/8 -1 , MinFrameWidth/8 -1 )
					CapSetH263 = new CCapSetH263;
					CapSetH263->SetVidImageFormatValue(NextStandardSmallerVidImgFrmt);
					CapSetH263->SetVideoFormatResolution( (BYTE)(MinFrameHeight/8 -1) , (BYTE)(MinFrameWidth/8 -1) );
					CapSetH263->SetMPI(LastStandardFormatInInitialPart->GetMPI());
					CapSetH263->SetOptionalCaps(LastStandardFormatInInitialPart->GetOptionalCap());
					CapSetH263->SetHRDbAndBPPmaxKb(LastStandardFormatInInitialPart->GetHRDbAndBPPmaxKb());
					CapSetH263->SetAdditionalH263Cap(AdditionalH263Cap);
					isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
					if( !isCapsetAdded ) {
					    POBJDELETE(CapSetH263);
					} else {
					    PTRACE(eLevelInfoNormal,"CCapH263::CreateAdditionalCapabilities - new smaller capabilities");
					    Dump();
					}
				}
				else
				{
					CapSetH263 = GetStandardFomatAccordingToInitialOrder(IndexOfStandardH263Set++);
					if (CapSetH263)
						CapSetH263->SetAdditionalH263Cap(AdditionalH263Cap);
					else
						PTRACE(eLevelInfoNormal,"CCapH263::CreateAdditionalCapabilities - CapSetH263 is NULL");
				}
				break;
			}

		case CUSTOM_FORMAT_CAP_EQUAL_BOUNDS :
			{
				CapSetH263 = new CCapSetH263;
				CapSetH263->SetVidImageToCustomFormat();
				CapSetH263->SetAdditionalH263Cap(AdditionalH263Cap);

				if((seg.GetRdOffset()+2) > seg.GetWrtOffset()) // Wrong message length.
				{
					ON(IsErrorEndOfSegment);  // Invalid Cap we will delete it and not insert to RW
					//m_capH263->insert(CapSetH263);
					POBJDELETE(CapSetH263);
					break;
				}

				seg >> Cap >> Cap1;
				readBytes += 2;
                CapSetH263->SetVideoFormatResolution(Cap,Cap1);
				break;
			}

		case CUSTOM_FORMAT_CAP_TWO_DISTINCT_BOUNDS :
			{
				CapSetH263 = new CCapSetH263;
				CapSetH263->SetVidImageToCustomFormat();
				CapSetH263->SetAdditionalH263Cap(AdditionalH263Cap);

				if((seg.GetRdOffset()+4) > seg.GetWrtOffset())
				{
					ON(IsErrorEndOfSegment);  // Invalid Cap we will delete it and not insert to RW
					//m_capH263->insert(CapSetH263);
					POBJDELETE(CapSetH263);
					break;
				}

				seg >> Cap >> Cap1 >> Cap2 >> Cap3;
				readBytes += 4;
				CapSetH263->SetVideoFormatResolution(Cap,Cap1,Cap2,Cap3);
				break;
			}

		default :
			{
				PASSERT_AND_RETURN(FormatIndicator); // Standard : Value of 1 is not allowed.
			}
		}

		if(!IsErrorEndOfSegment)
		{
			if( CPObject::IsValidPObjectPtr(CapSetH263) && CapSetH263->IsCustomPCFFlag() ) // CustomPCFFlag
			{
				if((seg.GetRdOffset()+2) > seg.GetWrtOffset())
				{
					ON(IsErrorEndOfSegment);
					if(FormatIndicator)
					{
						POBJDELETE(CapSetH263);
					}
					break;
				} // Wrong message length.

				seg >> Cap >> Cap1;
				readBytes += 2;
				CapSetH263->SetCustomPCF(Cap,Cap1);
				if(CapSetH263->IsHRDBPPMaxKB()) //Either Specity HRD-B or Specify BPPMaxKB bits are 1.
				{
					if(seg.EndOfSegment()) // Wrong message length.
					{
						ON(IsErrorEndOfSegment);

						if(FormatIndicator) {
							isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
							if( !isCapsetAdded )
							    POBJDELETE(CapSetH263);
						}
						break;
					}

					seg >> Cap2;
					readBytes++;
					CapSetH263->SetHRDBPPMaxKB(Cap2);
				}
			}

			if( CPObject::IsValidPObjectPtr(CapSetH263) && CapSetH263->IsCustomPARFlag()) // CustomPARFlag
			{
				if(seg.EndOfSegment()) // Wrong message length.
				{
					ON(IsErrorEndOfSegment);

					if(FormatIndicator) {
						isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
						if( !isCapsetAdded )
						    POBJDELETE(CapSetH263);
					}
					break;
				}

				seg >> Cap >> Cap1;
				readBytes += 2;
				CapSetH263->SetCustomPixelWidthAndHeight(Cap,Cap1);
			}



			if( CPObject::IsValidPObjectPtr(CapSetH263) &&
				!CapSetH263->GetOptionsIndicator() )  // OptionsIndicator == 000. Signs for IndividualOptionIndicator.
			{
				BYTE IndividualOptionIndicator;

				if(seg.EndOfSegment()) // Wrong message length.
				{
					ON(IsErrorEndOfSegment);

					if(FormatIndicator) {
						isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
						if( !isCapsetAdded )
						    POBJDELETE(CapSetH263);
					}
					break;
				}

				seg >> IndividualOptionIndicator;
				readBytes++;
				CapSetH263->SetIndividualOptionIndicator(IndividualOptionIndicator);

				if(CapSetH263->IsOptions_1_FlagOn()) // Option_1 flag.
				{
					if(seg.EndOfSegment()) // Wrong message length.
					{
						ON(IsErrorEndOfSegment);

						if(FormatIndicator) {
							isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
							if( !isCapsetAdded )
							    POBJDELETE(CapSetH263);
						}
						break;
					}

					seg >> Cap;
					readBytes++;
					CapSetH263->SetOption1Cap(Cap);
				}

				if(CapSetH263->IsOptions_2_FlagOn()) // Option_2 flag.
				{
					if(seg.EndOfSegment()) // Wrong message length.
					{
						ON(IsErrorEndOfSegment);

						if(FormatIndicator) {
							isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
							if( !isCapsetAdded )
							    POBJDELETE(CapSetH263);
						}
						break;
					}

					seg >> Cap;
					readBytes++;
					CapSetH263->SetOption2Cap(Cap);
				}

				if(CapSetH263->IsOptions_3_FlagOn()) // Option_3 flag.
				{
					if(seg.EndOfSegment()) // Wrong message length.
					{
						ON(IsErrorEndOfSegment);

						if(FormatIndicator) {
							isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
							if( !isCapsetAdded )
							    POBJDELETE(CapSetH263);
						}
						break;
					}

					seg >> Cap;
					readBytes++;
					CapSetH263->SetOption3Cap(Cap);
				}

				if(CapSetH263->IsRefSliceParameters())  // RefSliceParameters
				{
					if(seg.EndOfSegment()) // Wrong message length.
					{
						ON(IsErrorEndOfSegment);

						if(FormatIndicator) {
						    isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
							if( !isCapsetAdded )
							    POBJDELETE(CapSetH263);
						}
						break;
					}

					seg >> Cap;
					readBytes++;
					CapSetH263->SetRefSliceParameters(Cap);
				}

				if(CapSetH263->IsScalabilityDescriptor_FlagOn())  // ScalabilityDescriptor flag.
				{
					if(seg.EndOfSegment()) // Wrong message length.
					{
						ON(IsErrorEndOfSegment);

						if(FormatIndicator) {
							isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
							if( !isCapsetAdded )
							    POBJDELETE(CapSetH263);
						}
						break;
					}

					seg >> Cap;
					readBytes++;
					CapSetH263->SetScalabilityDescriptor(Cap);
					BYTE NumberOfScalableLayers = CapSetH263->GetNumberOfScalableLayers();
					for( BYTE Index = 0; Index < NumberOfScalableLayers ; Index++)
					{
						if(seg.EndOfSegment()) // Wrong message length.
						{
							ON(IsErrorEndOfSegment);

							if(FormatIndicator) {
								isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
								if( !isCapsetAdded )
								    POBJDELETE(CapSetH263);
							}
							break;
						}

						seg >> Cap;
						readBytes++;
						CapSetH263->SetEnhancementLayerInfo(Index,Cap);
					}
				}

			}

			if(FormatIndicator) {
			    isCapsetAdded = InsertH263CapSet(CapSetH263);//m_capH263->insert(CapSetH263);
				if( !isCapsetAdded )
				    POBJDELETE(CapSetH263);
			}
		} // !IsErrorEndOfSegment

      } // while(readBytes < AdditionalCapLength )

	  // Update all lower resulotion to inherit additional cap.
	  // CapSetH263 might be invalid in case no data of additinal cap are declared.
	  if( CPObject::IsValidPObjectPtr(CapSetH263) )
	  {
		  std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->find(CapSetH263);//WORD IndexOfRemoteCapSetH263 = Index(CapSetH263);

		  if (*itr)//IndexOfRemoteCapSetH263!=RW_NPOS)
		  {
			  CCapSetH263* pCapSetH263 = NULL;
			  while ( itr != m_capH263->begin () ) //for(size_t i = IndexOfRemoteCapSetH263 ; i > 0 ; i--)
			  {
				  --itr;
				  pCapSetH263 = (*itr);//at(i-1);
				  if(CPObject::IsValidPObjectPtr(pCapSetH263))
					  pCapSetH263->SetOptionsIndicator(1); //Inherit options from immediately larger format.
			  }
		  }
	  }

	  m_numberOfH263Sets = m_capH263->size();
}
/////////////////////////////////////////////////////////////////////////////
void  CCapH263::CreateSecondAdditionalCapabilities(CSegment& seg,BYTE SecondAdditionalCapLength,BYTE& IsErrorEndOfSegment)
{
	PTRACE(eLevelInfoNormal,"CCapH263::CreateSecondAdditionalCapabilities");
	BYTE SecondAdditionalH263Cap;
	BYTE readBytes = 0;
	CCapSetH263*   CapSetH263 = NULL;
 	WORD NumberOfH263Sets = m_capH263->size();

	PASSERT_AND_RETURN(!NumberOfH263Sets);//At least one resolution should be declared.

	std::multiset<CCapSetH263*, CompareByH263>::reverse_iterator revitr = m_capH263->rbegin();

	while(readBytes < SecondAdditionalCapLength )
	{
		if(seg.EndOfSegment()) {
		  ON(IsErrorEndOfSegment);
		  break;
		} // Wrong message length.

		// fix - exception when SecondAdditionalCapLength > NumberOfH263Sets
		// we will NumberOfH263Sets will be set to 0xFFFF

		if(revitr == m_capH263->rend())// if(NumberOfH263Sets == 0)
		{
			PASSERT(SecondAdditionalCapLength - readBytes);
			break;
		}

		seg >> SecondAdditionalH263Cap;
		readBytes++;

		NumberOfH263Sets--;

		CapSetH263 = (*revitr);//at(NumberOfH263Sets);
		CapSetH263->SetSecondAdditionalH263Cap(SecondAdditionalH263Cap);
		++revitr;
	}

	// Update all lower resulotion to inherit additional cap.

// 	WORD IndexOfRemoteCapSetH263 = Index(CapSetH263);
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->find(CapSetH263);
	CCapSetH263* pCapSetH263 = NULL;
	while ( (*itr) && itr != m_capH263->begin () )// for(size_t i = IndexOfRemoteCapSetH263 ; i > 0 ; i--)
	{
		--itr;
	    pCapSetH263 = (*itr);//at(i-1);
		pCapSetH263->SetOptionsIndicator(1); //Inherit options from immediately larger format.
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CCapH263::PrintH263CapSeg(CSegment& seg,BYTE Length) const
{
	ALLOCBUFFER(Message,seg.GetLen() * 5 + 100);
	ALLOCBUFFER(Header,64);
	ALLOCBUFFER(MessageByte,16);
	BYTE cap;
	WORD Count = 0;

	sprintf(Header,"Message length = %d   OpCode = H262_H263 \n",Length);
	strcat(Message,Header);

	while(!seg.EndOfSegment())
	{
		seg >> cap;
		sprintf(MessageByte,"%02x ",cap);
		strcat(Message,MessageByte);
		Count++;
        if( !(Count%15) ) strcat(Message,"\n");
	}

	PTRACE2(eLevelInfoNormal,"***  WRONG MESSAGE OF H.263 CAPABILITIES  ***\n",Message);//INTERACTIVE_TRACE
	DEALLOCBUFFER(MessageByte);
	DEALLOCBUFFER(Header);
	DEALLOCBUFFER(Message);
}
/////////////////////////////////////////////////////////////////////////////
void  CCapH263::Serialize(WORD format,CSegment& seg)
{
	if( m_numberOfH263Sets == (WORD)0 )
		return;

    seg << ((BYTE)(Start_Mbe | ESCAPECAPATTR)); // Insert "Start- MBE" opcode.

	// First build the H263 cap message in order to calculate it's length.
    CSegment* H263CapSeg = new CSegment;
	*H263CapSeg << (BYTE)H262_H263;

    CCapSetH263* pCapSetH263 = NULL;
// 	BYTE length = m_capH263->size(), i = 0;

	// Initial part.
	std::multiset<CCapSetH263*, CompareByH263>::reverse_iterator revitr = m_capH263->rbegin();
	for ( ; revitr != m_capH263->rend (); ++revitr ) // for (i = length ; i > 0 ; i--)
	{
	    pCapSetH263 = (*revitr);//at(i-1);
        if( pCapSetH263->GetVidImageFormat() != H263_CUSTOM_FORMAT)
		    pCapSetH263->Serialize(format,*H263CapSeg);
	}

	if(IsAdditionalCapOrCustomFormats())
	{
		CCapSetH263* pRemoteLastFormatExplicitlyDeclared = NULL;
		BYTE FormatIndicator = 0;
		// Extension Code.
		*H263CapSeg << (BYTE)0x7f;

		// Additional part.
		pRemoteLastFormatExplicitlyDeclared = (*m_capH263->rbegin ());//at(length-1); /* First(=largest) CCapSetH263.*/

		for ( revitr = m_capH263->rbegin(); revitr != m_capH263->rend (); ++revitr )// for (i = length ; i > 0 ; i--)
		{
			pCapSetH263 = (*revitr);//at(i-1);

			if(revitr != m_capH263->rbegin())//if(length != i)   //All CCapSetH263 - except the first.
			{
				if(!pCapSetH263->GetOptionsIndicator()) // Options are signaled separately.
				{
				    if(IsSameAdditionalCap(*pCapSetH263,*pRemoteLastFormatExplicitlyDeclared))
						pCapSetH263->SetOptionsIndicator(1); //Inherit options from immediately larger format.
				}
		    	else
				{
					if(pCapSetH263->GetOptionsIndicator() != 1 ) //All cases except from inherit.
					    pRemoteLastFormatExplicitlyDeclared = pCapSetH263;
				}
			}

			pCapSetH263->Serialize(ADDITIONAL_CAPABILITIES,*H263CapSeg);
		}
	}

	if(IsSecondAdditionalCap() && m_IsSendSecondAdditionalCap)
	{
		CCapSetH263* pRemoteLastFormatExplicitlyDeclared = NULL;
		BYTE FormatIndicator = 0;
		// Extension Code.
		*H263CapSeg << (BYTE)0x7f;

		// Additional part.
		pRemoteLastFormatExplicitlyDeclared = (*m_capH263->rbegin());//at(length-1); /* First(=largest) CCapSetH263.*/

// 		for (i = length ; i > 0 ; i--)
		for ( revitr = m_capH263->rbegin(); revitr != m_capH263->rend (); ++revitr )
		{
		    pCapSetH263 = (*revitr);//at(i-1);

			if(revitr != m_capH263->rbegin())//if((length != i)) //All CCapSetH263 - except the first.
			{
				//If the lower resolution has the same second capabilities as the highest
				//sign that lowest is inhert it second cap from the highest.
				if(pRemoteLastFormatExplicitlyDeclared->GetSecondAdditionalH263Cap() ==
					pCapSetH263->GetSecondAdditionalH263Cap())
				{
					pCapSetH263->SetEnhancedReferencePicSelect(0);// 0 means inherit.
				}
				else
				{
					pRemoteLastFormatExplicitlyDeclared = pCapSetH263;
				}
			}

			pCapSetH263->Serialize(SECOND_ADDITIONAL_CAPABILITIES,*H263CapSeg);
		}
	}

    // Calculate MBE Command length.
	BYTE H263CapSegLength = ( (H263CapSeg->GetWrtOffset()) - (H263CapSeg->GetRdOffset()));

	seg << H263CapSegLength << *H263CapSeg;
	POBJDELETE(H263CapSeg);
}
/////////////////////////////////////////////////////////////////////////////
void  CCapH263::DeSerialize(WORD format,CSegment& seg)
{
	if(seg.EndOfSegment())  return;

	if( *(seg.GetPtr(1)) != ((BYTE)(Start_Mbe | ESCAPECAPATTR)) )
		return;

	CSegment* copySeg = new CSegment(seg);
	BYTE temp,mbeCommandLen;
	*copySeg >> temp; // Start_Mbe
	*copySeg >> mbeCommandLen;
	*copySeg >> temp; // H262_H263
	if(temp != H262_H263)
	{
		if(temp == H264_Mbe)  //H264_Mbe we do NOT know if the caps are h263 or h264
		{
			POBJDELETE(copySeg);
			return;
		}
		else
		{
			if(temp)   PASSERT(temp);
			else       PASSERT(101);
		}
	}
    else //Read the original segment
	{
		BYTE temp,mbeCommandLen;
		seg >> temp; // Start_Mbe
		seg >> mbeCommandLen;
		seg >> temp; // H262_H263
	}

	Create(seg,mbeCommandLen);  // H262_H263 or != 264_Mbe
	POBJDELETE(copySeg);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::InsertH263CapSet(CCapSetH263* pH263CapSetBuf)
{
    BYTE res = pH263CapSetBuf ? pH263CapSetBuf->IsCapSetH263Valid() : 0;
	if(res) {
	    m_capH263->insert(pH263CapSetBuf);
	    m_numberOfH263Sets = m_capH263->size();
	} else {
	    PTRACE(eLevelInfoNormal,"CCapH263::InsertH263CapSet - CapSetH263 is not valid");
		DBGPASSERT(101);
	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////
// CCapSetH263* CCapH263::GetH263CapSet(BYTE Index) const
// {
// 	return at(Index);
// }
/////////////////////////////////////////////////////////////////////////////
CCapSetH263* CCapH263::GetStandardFomatAccordingToInitialOrder(BYTE NumberOfStandardFormatInInitialPart) const
{
    BYTE NumberOfFoundStandard = 1;
	CCapSetH263* pCapSetH263 = NULL;

// 	BYTE length = m_capH263->size();
	std::multiset<CCapSetH263*, CompareByH263>::reverse_iterator revitr = m_capH263->rbegin();
	for ( ; revitr != m_capH263->rend (); ++revitr )// 	for(BYTE i = length; i > 0 ; i--)
	{
	    pCapSetH263 = (*revitr);//at(i -1);
	   	if(pCapSetH263->GetVidImageFormat() != H263_CUSTOM_FORMAT ) {
			if(NumberOfFoundStandard == NumberOfStandardFormatInInitialPart)
				break;
			else
                NumberOfFoundStandard++;
	   	}
	}

    PASSERT(!pCapSetH263);
    return pCapSetH263;
}
/////////////////////////////////////////////////////////////////////////////
void CCapH263::Remove()
{
	m_numberOfH263Sets = 0;
}
/////////////////////////////////////////////////////////////////////////////
void CCapH263::SendSecondAdditionalCap(WORD OnOff)
{
	if(OnOff)
		ON(m_IsSendSecondAdditionalCap);
	else
		OFF(m_IsSendSecondAdditionalCap);
}
/////////////////////////////////////////////////////////////////////////////
CCapSetH263* CCapH263::Find(CCapSetH263* pCapSetH263) const
{
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->find(pCapSetH263);
	if (itr != m_capH263->end())
		return *itr;
	else
		return NULL;
}
/////////////////////////////////////////////////////////////////////////////
// size_t CCapH263::Index(CCapSetH263* pCapSetH263) const
// {
// 	if(CPObject::IsValidPObjectPtr(pCapSetH263))
// 	    return m_capH263->index(pCapSetH263);
// 	else
// 		return 0;
// }
/////////////////////////////////////////////////////////////////////////////
CCapSetH263* CCapH263::FindEquivalentResolution(const CCapSetH263* pCapSetH263) const
{
	CCapSetH263* pEquivalentCapSetH263 = NULL;

	if(pCapSetH263->GetVidImageFormat() == H263_CUSTOM_FORMAT ||
		pCapSetH263->GetInterlaceMode())
		pEquivalentCapSetH263 = FindEqualOrComprisedCustomFormat(pCapSetH263);
	else  // STANDARD FORMAT.
		pEquivalentCapSetH263 = FindEqualOrLargerResolution(pCapSetH263);

	return pEquivalentCapSetH263;
}
/////////////////////////////////////////////////////////////////////////////
CCapSetH263* CCapH263::FindEqualOrLargerResolution(const CCapSetH263* pCapSetH263) const
{
	CCapSetH263* pLocalCapSetH263 = NULL;
    CCapSetH263* pTempCapSetH263 = NULL;

	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
	    pTempCapSetH263 = (*itr);
		if( (*pCapSetH263 == *pTempCapSetH263) || (*pCapSetH263 < *pTempCapSetH263) )
		{
			pLocalCapSetH263 = pTempCapSetH263;
			break;
		}
	}
	return pLocalCapSetH263;
}
/////////////////////////////////////////////////////////////////////////////
CCapSetH263* CCapH263::FindEqualOrComprisedCustomFormat(const CCapSetH263* pCapSetH263) const
{
	//This Function finds the equivalent pCapSetH263 custom format - at THIS CCapH263.
	//The equivalent resolution is a resolution which fulfils at least ONE of the following conditions:
	//(1) Equal custom format bounds ( Height & Width + Min & Max).
	//(2) Comprises the given custom format resolution - within its bounds.
	//    For example: [800 - 600] X [640 - 480] comprises VGA(640 x 480) / SVGA(704 x 480)
	//    and NTSC(800 x 600).

	CCapSetH263* pLocalCapSetH263 = NULL;
    CCapSetH263* pTempCapSetH263 = NULL;

	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
	    pTempCapSetH263 = (*itr);//at(Index);
		if( (*pTempCapSetH263 == *pCapSetH263) ||
			(pTempCapSetH263->IsComprisesCustomFormat(pCapSetH263)) )
		{
			pLocalCapSetH263 = pTempCapSetH263;
			break;
		}
	}
	return pLocalCapSetH263;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::GetMaxResolution()const
{
	BYTE max_image_format = 0;
	CCapSetH263* pTempCapSetH263 = NULL;
	if(m_numberOfH263Sets == 0)
	{
		PTRACE(eLevelInfoNormal,"CCapH263::GetMaxResolution: m_numberOfH263Sets = 0");
		return 0; // 0 = qcif
	}
	pTempCapSetH263 = *m_capH263->rbegin();//at(m_numberOfH263Sets - 1);
	if(pTempCapSetH263 != NULL)
	{
		max_image_format = pTempCapSetH263->GetVidImageFormat();
	}
	return max_image_format;
}

/////////////////////////////////////////////////////////////////////////////
void CCapH263::ClearAndDestroy()
{
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr ) {// Remove all old CCapSetH263.
	    CCapSetH263* temp = *itr;
	    POBJDELETE(temp);
	}
	m_capH263->clear();
	m_numberOfH263Sets = 0;
}
/////////////////////////////////////////////////////////////////////////////
WORD CCapH263::GetNumberOfH263Sets(void) const
{
	return	m_numberOfH263Sets;
}

/////////////////////////////////////////////////////////////////////////////
CCapSetH263* CCapH263::GetCapSetH263(WORD setNumber) const
{
// 	return at(setNumber-1);

	WORD index = setNumber-1, i = 0;
	if(index > m_capH263->size() - 1)
	{
		DBGPASSERT(index);
		return NULL;
	}
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr, i++ )
	    if (i == index)
		    return (*itr);
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
void CCapH263::GetFormatCapSet(WORD format, CCapSetH263* capSetH263) const
{
	CCapSetH263* pCapSetH263 = NULL;
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
	    pCapSetH263 = (*itr);
		if( pCapSetH263->GetVidImageFormat() == format ){
			*capSetH263 = *pCapSetH263;
			return;
		}
	}

	// Standard H.242 :
	// If MPI value and optional caps (for specified image format) are not explicitly defined in
	// separate baseline byte, use the next higher image format byte for these values

	std::multiset<CCapSetH263*, CompareByH263>::reverse_iterator revitr = m_capH263->rbegin();
	for ( ; revitr != m_capH263->rend (); ++revitr ) // for (WORD j=m_numberOfH263Sets; j > 0; j++)
		if( (*revitr)->GetVidImageFormat() > format ) {
			// copy the higher format capSet & change format value to that one we need
			*capSetH263 = *(*revitr);
			capSetH263->SetVidImageFormatValue(format);
			return;
		}

	// Specified format doesn't exist in H263 capabilities
	PTRACE(eLevelInfoNormal,"CCapH263::GetFormatCapSet : specified format not exists!!!");
	if(format)
		PASSERT(format);
	else
		PASSERT(101);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::GetFormatMPI(WORD format) const
{
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
		if( (*itr)->GetVidImageFormat() == format )
			return (*itr)->GetMPI();

	// Standard H.242 :
	// If MPI value (for specified image format) is not explicitly defined in
	// separate baseline byte, use MPI value from the next higher image format byte

	itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
		if( (*itr)->GetVidImageFormat() > format )
		  return (*itr)->GetMPI();

	// No MPI value defined for this image format : use the lowest MPI value - 1 fps
	CSmallString sstr;
	sstr << "\nformat = " << format <<"\ncap formats = ";

	itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr ) {
		sstr << (*itr)->GetVidImageFormat() << ";";
	}
	PTRACE2(eLevelInfoNormal,"CCapH263::GetFormatMPI : specified format not exists -> using 1 fps MPI!!!",sstr.GetString());

//	if(format)
//		DBGPASSERT(format);
//	else
//		DBGPASSERT(101);
	return MPI_30;
}
/////////////////////////////////////////////////////////////////////////////
// added for highest common phase 2 vga,xga,svga
BYTE CCapH263::GetCustomFormatMPI(BYTE format) const
{
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
		CCapSetH263* current_set = NULL;
		current_set = (*itr);
		if(current_set != NULL)
		{
			if( (current_set->GetCustomVidImageFormat()==format) || current_set->IsCustomTwoDistinctBoundsVidImageFormat(format))
			{
				return (*itr)->GetMPI();
			}
		}
	}
	// custom format not found
	return GetFormatMPI(format);
}
/////////////////////////////////////////////////////////////////////////////
// WORD CCapH263::GetFormatHcAnnexes(WORD format,HC_ANNEXES_ST& st_annexes) const
// {
// 	WORD ret_val = FALSE;
// 	BYTE format_found = NO;
// 	WORD format_capset_index = 0;
// 	BYTE inherit_options = NO;
// 	st_annexes.m_annex_F = 0;st_annexes.m_annex_T = 0;st_annexes.m_annex_N = 0;st_annexes.m_annex_I_NS = 0;
// 	for (WORD i=0; i < m_numberOfH263Sets; i++)
// 	{
// 		CCapSetH263* current_set = NULL;
// 		current_set = at(i);
// 		if(current_set)
// 		{

// 			if( (current_set->GetVidImageFormat() == format && format != H263_CUSTOM_FORMAT) || (format == H263_CUSTOM_FORMAT && current_set->GetCustomVidImageFormat() == format) || current_set->IsCustomTwoDistinctBoundsVidImageFormat(format) )
// 			{
// 				// format found
// 				format_found = YES;
// 				format_capset_index = i;
// 				// annex F
// 				st_annexes.m_annex_F = current_set->IsAMP();
// 				st_annexes.m_annex_T = current_set->IsAnnex_T();
// 				st_annexes.m_annex_N = current_set->IsAnnex_N();
// 				if(current_set->GetOptionsIndicator() == 1)
// 				{
// 					//Inherit options from immediately larger format
// 					inherit_options = YES;
// 				}
// 				ret_val = TRUE;
// 			}else{
// 				if(format_found && i>format_capset_index)
// 				{
// 					if(st_annexes.m_annex_F == 0){
// 						st_annexes.m_annex_F = current_set->IsAMP();
// 					}
// 					if(inherit_options == YES)
// 					{
// 						if(!current_set->GetOptionsIndicator())
// 						{
// 							//Inherit options from immediately larger format
// 							st_annexes.m_annex_T = current_set->IsAnnex_T();
// 							st_annexes.m_annex_N = current_set->IsAnnex_N();
// 							inherit_options = NO;
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// 	if(format_found == NO){
// 		PTRACE(eLevelInfoNormal,"CCapH263::GetFormatHcAnnexes - format not found");
// 	}
// 	return ret_val;
// }

/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::GetHighestFormat(void) const
{
	if(m_numberOfH263Sets == 0)
	{
		DBGPASSERT(1);
		return NUMBER_OF_H263_FORMATS;// not valid value
	}
	// Last h263 set contains the highest image format
	TRACESTR(eLevelInfoNormal) << "CCapH263::GetHighestFormat Dump:";
	Dump();
	return (*m_capH263->rbegin())->GetCustomVidImageFormat();//(at(m_numberOfH263Sets -1))->GetCustomVidImageFormat();
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::OnVidImageFormat(WORD format) const
{
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
		if( (*itr)->GetVidImageFormat() >= format )
			return 1;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::IsExtensionCode(BYTE cap) const
{
	//  ------------ FROM H.242 standard:
	//
	// An extension code has been defined for future expansion
	// of the H.262/H.263 capabilities.
	// This code has the value 01111111 (0x7F).
	// This code is reserved, and it shall not be used until its use
	// is defined at a later time.
	// If this code is encountered in an H.262/H.263 MBE message,
	// all of the data following this code shall be ignored.
	// The appearance of this code in an MBE message does not
	// affect the meaning of any bytes prior to this code byte.

	if( cap == (BYTE)0x7f )
		return 1;
	else
		return 0;
}
/////////////////////////////////////////////////////////////////////////////
BYTE  CCapH263::OnOptions(BYTE cap) const
{
	return GetBit(cap,8);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::OnSpecifyHRDb(BYTE cap) const
{
	return GetBit(cap,7);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::OnSpecifyBPPmaxKb(BYTE cap) const
{
	return GetBit(cap,8);
}
/////////////////////////////////////////////////////////////////////////////
CCapH263& CCapH263::operator=(const CCapH263 &other)
{
	// In order to prevent clearAndDestroy() to the same object during ExchangeCap() of the mux.
	if(this == &other)  return *this;

	ClearAndDestroy();
    m_numberOfH263Sets = other.m_capH263->size();

	if(other.m_numberOfH263Sets != other.m_capH263->size())
	{
		if(other.m_numberOfH263Sets)
			PASSERT(other.m_numberOfH263Sets);
		else if(other.m_capH263->size())
			PASSERT(other.m_capH263->size());
		else PASSERT(101);
	}
    CCapSetH263* pCapSetH263 = NULL;
	std::multiset<CCapSetH263*, CompareByH263>::iterator other_itr = other.m_capH263->begin();
	for ( ; other_itr != other.m_capH263->end(); ++other_itr )
	{
		 pCapSetH263 = new CCapSetH263;
		 *pCapSetH263 = *(*other_itr);
		 m_capH263->insert(pCapSetH263);
	}
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::IsAdditionalCapOrCustomFormats() const
{
	if(m_numberOfH263Sets != m_capH263->size())
	{
		if(m_numberOfH263Sets)
			PASSERT(m_numberOfH263Sets);
		else if(m_capH263->size())
			PASSERT(m_capH263->size());
		else PASSERT(101);
	}

	CCapSetH263* pCapSetH263 = NULL;
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
	    pCapSetH263 = (*itr);//at(i);
        if(pCapSetH263->GetVidImageFormat() == H263_CUSTOM_FORMAT)
			return TRUE;

		else // STANDARD_FORMAT
		{
           if(pCapSetH263->GetOptionsIndicator() != 5 ) // 5 = No additional h.263 options are supported.
			   return  TRUE;
		}

	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::IsSecondAdditionalCap() const
{
	if(m_numberOfH263Sets != m_capH263->size())
	{
		if(m_numberOfH263Sets)
			PASSERT(m_numberOfH263Sets);
		else if(m_capH263->size())
			PASSERT(m_capH263->size());
		else PASSERT(101);
	}

	CCapSetH263* pCapSetH263 = NULL;
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
	    pCapSetH263 = (*itr);
        if(pCapSetH263->GetSecondAdditionalH263Cap() != 0x40)// 0x40 means no Second Additional Cap.
			return TRUE;
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::IsInterlaceCap() const
{
	if(m_numberOfH263Sets != m_capH263->size())
	{
		if(m_numberOfH263Sets)
			PASSERT(m_numberOfH263Sets);
		else if(m_capH263->size())
			PASSERT(m_capH263->size());
		else PASSERT(101);
	}

	CCapSetH263* pCapSetH263 = NULL;
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
	    pCapSetH263 = (*itr);
		if(pCapSetH263->GetInterlaceMode())
			return TRUE;
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::IsInterlaceCap(WORD format) const
{
	if(m_numberOfH263Sets != m_capH263->size())
	{
		if(m_numberOfH263Sets)
			PASSERT(m_numberOfH263Sets);
		else if(m_capH263->size())
			PASSERT(m_capH263->size());
		else PASSERT(101);
	}

	CCapSetH263* pCapSetH263 = NULL;
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
	    pCapSetH263 = (*itr);
		if(pCapSetH263->GetInterlaceMode())
		{
			if(format == NTSC_60_FIELDS)
			{
				if(pCapSetH263->GetVidImageFormat() == H263_CUSTOM_FORMAT){
					return TRUE;
				}
			}else if(format == PAL_50_FIELDS){
				if(pCapSetH263->GetVidImageFormat() == H263_CIF){
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::IsCustomFormatsOrAnnexes() const
{
	// This Function checks only the MCMS supported Annexes.

	if(m_numberOfH263Sets != m_capH263->size())
	{
		if(m_numberOfH263Sets)
			PASSERT(m_numberOfH263Sets);
		else if(m_capH263->size())
			PASSERT(m_capH263->size());
		else PASSERT(101);
	}

	CCapSetH263* pCapSetH263 = NULL;
	std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
	    pCapSetH263 = (*itr);
        if(pCapSetH263->GetVidImageFormat() == H263_CUSTOM_FORMAT)
			return TRUE;

		else // STANDARD_FORMAT
		{
           if(pCapSetH263->GetOptionsIndicator() != 5 ) // 5 = No additional h.263 options are supported.
			   return  TRUE;
		   // Annex F is declared on the initial part so we have to check it individually.
		   if( pCapSetH263->IsAMP())
			   return  TRUE;
		}

	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// created:
// date:GW project	programer: Zohar
// Description:		the function intersect "this" capabilities with pRemoteCapH263
//					it set the data members to the intersection.
// input:			pRemoteCapH263 - other party caps
// Returns:			set the data members to the intersection.
//================================================================================================================================================================================================
// update 01: the function updated to be used in the Highest Common Phase 2 feature
// date:01/2004		programer: Ron
// Description:
// input:			pRemoteCapH263 - other party caps
// Returns:			added: is_common_caps_changed -  does the common capabilities (data members) changed.
//
// Remarks:			1.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapH263::SetHighestCommonVideoCapabilities(CCapH263* pRemoteCapH263,WORD isAutoVidScm, WORD& is_highest_common_param_changed)
{
	PTRACE(eLevelInfoNormal,"CCapH263::SetHighestCommonVideoCapabilities");

	is_highest_common_param_changed = FALSE;
	// local and remote current (working) cap sets
	CCapSetH263* pRemoteCapSetH263 = NULL;
	CCapSetH263* pLocalCapSetH263 = NULL;
	// intersection caps and cap sets
	CCapH263*    pCommonCapH263 = new CCapH263;
	CCapSetH263* pNewCommonCapSetH263 = NULL;

	// we start from the last set - the highest format
	std::multiset<CCapSetH263*, CompareByH263>::reverse_iterator revitr = m_capH263->rbegin();
	for ( ; revitr != m_capH263->rend (); ++revitr )// 	for(size_t i = m_numberOfH263Sets ; i > 0 ; i--)
	{
		// get local cap set and find equivalent resolution at remote caps
	    pLocalCapSetH263 = (*revitr);//at(i-1);
		pRemoteCapSetH263 = pRemoteCapH263->FindEquivalentResolution(pLocalCapSetH263);
//		PTRACE(eLevelInfoNormal,"CCapH263::SetHighestCommonVideoCapabilities - local set:");
//		pLocalCapSetH263->HcDump();

		if(!CPObject::IsValidPObjectPtr(pRemoteCapSetH263)){
			//equivalent resolution not found
			is_highest_common_param_changed = TRUE;
//			PTRACE(eLevelInfoNormal,"CCapH263::SetHighestCommonVideoCapabilities equivalent resolution not found");
		}else{

			//equivalent resolution found
			WORD is_cap_set_common_param_changed = FALSE;

//			PTRACE(eLevelInfoNormal,"CCapH263::SetHighestCommonVideoCapabilities equivalent resolution found:");
//			pRemoteCapSetH263->HcDump();

			// set new cap set for the intersection
			pNewCommonCapSetH263 = new CCapSetH263;
			SetCommonCapabilities(pLocalCapSetH263,pRemoteCapSetH263
				,pRemoteCapH263,*pNewCommonCapSetH263,isAutoVidScm,is_highest_common_param_changed);
			// add new cap set to the intersection caps
			pCommonCapH263->InsertH263CapSet(pNewCommonCapSetH263);
//			PTRACE(eLevelInfoNormal,"CCapH263::SetHighestCommonVideoCapabilities new (intersected) set:");
//			pNewCommonCapSetH263->HcDump();

			if(is_cap_set_common_param_changed){
				is_highest_common_param_changed = TRUE;
//				PTRACE(eLevelInfoNormal,"CCapH263::SetHighestCommonVideoCapabilities is_highest_common_param_changed = TRUE");
			}
		}

	}
	// update data members
	*this = *pCommonCapH263;
	// delete intersection caps
	POBJDELETE(pCommonCapH263);
}
/////////////////////////////////////////////////////////////////////////////
void CCapH263::SetCommonCapabilities(CCapSetH263* pLocalCapSetH263,
                                     CCapSetH263* pRemoteCapSetH263,
                                     CCapH263* pRemoteCapH263,
                                     CCapSetH263& pNewCommonCapSetH263,
                                     WORD isAutoVidScm,
                                     WORD& is_highest_common_param_changed)
{
  CCapSetH263* pRemoteLastFormatExplicitlyDeclared   = NULL;  // Last Video Format which explicitly declared on Additional capabilities.
  CCapSetH263* pRemoteLastStandardExplicitlyDeclared = NULL;  // Last STANDARD Format which explicitly declared on Initial capabilities.

  // find pRemoteLastFormatExplicitlyDeclared

  std::multiset<CCapSetH263*, CompareByH263>::iterator itr = pRemoteCapH263->m_capH263->find(pRemoteCapSetH263);
  BYTE NumberOfRemoteCapSetH263 = pRemoteCapH263->GetNumberOfH263Sets();
  for (; itr != pRemoteCapH263->m_capH263->end(); ++itr)
  {
    pRemoteLastFormatExplicitlyDeclared = (*itr);
    // Value of 1 means "Inherit options from immediately larger format"
    if (pRemoteLastFormatExplicitlyDeclared->GetOptionsIndicator() != 1)
      break;
  }

  // case of standard format
  // or promotion 50 fields (pal , video format is CIF)
  if (pLocalCapSetH263->GetVidImageFormat() != H263_CUSTOM_FORMAT)
  {
    // The pRemoteFirstFormat might be Standard or custom format.
    CCapSetH263* pRemoteFirstFormat = (*itr);
    BYTE         IsFirstStandard = TRUE;
    size_t       i = 0;

    itr = pRemoteCapH263->m_capH263->find(pRemoteCapSetH263);
    for (; itr != pRemoteCapH263->m_capH263->end(); ++itr)
    {
      pRemoteLastStandardExplicitlyDeclared = (*itr);  // pRemoteCapH263->GetCapSetH263(i+1);
      if (pRemoteLastStandardExplicitlyDeclared->GetVidImageFormat() != H263_CUSTOM_FORMAT)
      {
        if (IsFirstStandard)
        {
          OFF(IsFirstStandard);
          pRemoteFirstFormat = pRemoteLastStandardExplicitlyDeclared;
        }

        if (pRemoteLastStandardExplicitlyDeclared->IsOptionalCap())
          break;
      }
    }

    // No Optional capabilities are declared.
    if (i == NumberOfRemoteCapSetH263)
      pRemoteLastStandardExplicitlyDeclared = pRemoteFirstFormat;

    if (isAutoVidScm) // Only at VS AUTO Set Common MPI
    {
      // Set Common MPI
      if (pLocalCapSetH263->GetMPI() < pRemoteFirstFormat->GetMPI())
      {
        pNewCommonCapSetH263.SetMpiFromBaseByte(pRemoteFirstFormat->GetBaseCapsH263());
        is_highest_common_param_changed = TRUE;
      }
      else
      {
        pNewCommonCapSetH263.SetMpiFromBaseByte(pLocalCapSetH263->GetBaseCapsH263());
      }
    }
    else  // At VS FIX - set the fixed MPI
    {
      pNewCommonCapSetH263.SetMpiFromBaseByte(pLocalCapSetH263->GetBaseCapsH263());
    }

    // Set AMP ( For VS AUTO + FIX )
    if (pLocalCapSetH263->IsOptionalCap())
    {
      if (pLocalCapSetH263->IsAMP() && pRemoteLastStandardExplicitlyDeclared && pRemoteLastStandardExplicitlyDeclared->IsAMP())
        pNewCommonCapSetH263.SetAMP(1);
    }
    else
    {
      is_highest_common_param_changed = TRUE;
    }
  }

  // Set video Format.
  pNewCommonCapSetH263.SetVidImageFormatValue((WORD)pLocalCapSetH263->GetVidImageFormat());
  pNewCommonCapSetH263.SetFormatIndicator(pLocalCapSetH263->GetFormatIndicator());
  pNewCommonCapSetH263.SetVideoFormatResolution(pLocalCapSetH263->GetMinCustomPictureHeight(), pLocalCapSetH263->GetMinCustomPictureWidth());

  if (pLocalCapSetH263->IsCustomPCFFlag())
  {
    PASSERT_AND_RETURN(!pRemoteLastFormatExplicitlyDeclared);

    pNewCommonCapSetH263.SetClockDivisor(pLocalCapSetH263->GetClockDivisor());
    pNewCommonCapSetH263.SetClockConversionCode(pLocalCapSetH263->GetClockConversionCode());

    // Set common MPI
    if (pLocalCapSetH263->GetMPI() < pRemoteLastFormatExplicitlyDeclared->GetMPI())
    {
      pNewCommonCapSetH263.SetCustomMPIIndicator(pRemoteLastFormatExplicitlyDeclared->GetMPI());
      is_highest_common_param_changed = TRUE;
    }
    else
    {
      pNewCommonCapSetH263.SetCustomMPIIndicator(pLocalCapSetH263->GetMPI());
    }
  }

  if (pLocalCapSetH263->GetOptionsIndicator() != 5)
  {
    PASSERT_AND_RETURN(!pRemoteLastFormatExplicitlyDeclared);

    if (pLocalCapSetH263->IsOptions_1_FlagOn() && pRemoteLastFormatExplicitlyDeclared->IsOptions_1_FlagOn())
    {
      if (pLocalCapSetH263->IsAnnex_N())
      {
        if (pRemoteLastFormatExplicitlyDeclared->IsAnnex_N())
          pNewCommonCapSetH263.SetAnnexN(TRUE);
        else
          is_highest_common_param_changed = TRUE;
      }

      // added annex T for highest common phase 2 (ron)
      if (pLocalCapSetH263->IsAnnex_T())
      {
        if (pRemoteLastFormatExplicitlyDeclared->IsAnnex_T())
          pNewCommonCapSetH263.SetAnnexT(TRUE);
        else
          is_highest_common_param_changed = TRUE;
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
WORD CCapH263::operator<=(const CCapH263& other) const
{
  BYTE         RemoteNumberOfH263Sets = other.GetNumberOfH263Sets();
  CCapSetH263* pRemoteCapSetH263                      = NULL;
  CCapSetH263* pLocalCapSetH263                       = NULL;
  CCapSetH263* pRemoteLastFormatExplicitlyDeclared    = NULL; // Last Video Format which explicitly declared on Additional capabilities.
  CCapSetH263* pRemoteLastStandardExplicitlyDeclared  = NULL; // Last STANDARD Format which explicitly declared on Initial capabilities.
  CCapSetH263* pRemoteLastFormatExplicitlyDeclared2   = NULL; // Last Video Format which explicitly declared on second Additional capabilities.
  CCapSetH263* pLocalLastStandardExplicitlyDeclared   = NULL;
  CCapSetH263* pLocalLastStandardExplicitlyDeclared2  = NULL;
  CCapSetH263* pCapSetH263                            = NULL;
  size_t       IndexOfRemoteCapSetH263 = 0, IndexOfLocalCapSetH263 = 0;

  std::multiset<CCapSetH263*, CompareByH263>::reverse_iterator itr = m_capH263->rbegin();
  for (; itr != m_capH263->rend(); ++itr)
  {
    pLocalCapSetH263 = (*itr);

    pRemoteCapSetH263 = other.FindEquivalentResolution(pLocalCapSetH263);

    if (!CPObject::IsValidPObjectPtr(pRemoteCapSetH263))
      return FALSE;

    // Finds the REMOTE resolutions which declare explicitly on additional cap (1+2).
    std::multiset<CCapSetH263*, CompareByH263>::iterator other_itr = other.m_capH263->find(pRemoteCapSetH263);
    for (; other_itr != other.m_capH263->end(); ++other_itr)
    {
      pRemoteLastFormatExplicitlyDeclared  = (*other_itr);

      // Value of 1 means "Inherit options from immediately larger format"
      if (pRemoteLastFormatExplicitlyDeclared->GetOptionsIndicator() != 1)
        break;
    }

    other_itr = other.m_capH263->find(pRemoteCapSetH263);
    for (; other_itr != other.m_capH263->end(); ++other_itr)
    {
      pRemoteLastFormatExplicitlyDeclared2  = (*other_itr);

      // Value of 0 means "Inherit second options from immediately larger format"
      if (pRemoteLastFormatExplicitlyDeclared2->GetSecondAdditionalH263Cap())
        break;
    }

    // Finds the LOCAL resolutions which declare explicitly on additional cap (1+2).
    std::multiset<CCapSetH263*, CompareByH263>::iterator local_itr = m_capH263->find(pLocalCapSetH263);

    for (; local_itr != m_capH263->end(); ++local_itr)
    {
      pLocalLastStandardExplicitlyDeclared  = (*local_itr);

      // Value of 1 means "Inherit options from immediately larger format"
      if (pLocalLastStandardExplicitlyDeclared->GetOptionsIndicator() != 1)
        break;
    }

    local_itr = m_capH263->find(pLocalCapSetH263);
    for (; local_itr != m_capH263->end(); ++local_itr)
    {
      pLocalLastStandardExplicitlyDeclared2  = (*local_itr);

      // Value of 0 means "Inherit second options from immediately larger format"
      if (pLocalLastStandardExplicitlyDeclared2->GetSecondAdditionalH263Cap())
        break;
    }

    // Initial capabilities - relevant only for STANDARD FORMAT.
    if (pLocalCapSetH263->GetVidImageFormat() != H263_CUSTOM_FORMAT)
    {
      // The pRemoteFirstFormat might be Standard or custom format.
      other_itr = other.m_capH263->find(pRemoteCapSetH263);
      CCapSetH263* pRemoteFirstFormat = *other_itr;
      BYTE         IsFirstStandard = TRUE;

      for (; other_itr != other.m_capH263->end(); ++other_itr)
      {
        pRemoteLastStandardExplicitlyDeclared = (*other_itr);
        if (pRemoteLastStandardExplicitlyDeclared->GetVidImageFormat() != H263_CUSTOM_FORMAT)
        {
          if (IsFirstStandard)
          {
            OFF(IsFirstStandard);
            pRemoteFirstFormat = pRemoteLastStandardExplicitlyDeclared;
          }

          if (pRemoteLastStandardExplicitlyDeclared->IsOptionalCap())
            break;
        }
      }

      // No Optional capabilities are declared.
      if (other_itr == other.m_capH263->end())
        pRemoteLastStandardExplicitlyDeclared = pRemoteFirstFormat;

      // Check MPI
      if (pLocalCapSetH263->GetMPI() < pRemoteFirstFormat->GetMPI())
        return FALSE;

      // Check AMP
      if (pLocalCapSetH263->IsOptionalCap())
      {
        if (pLocalCapSetH263->IsCPM())
          if (pRemoteLastStandardExplicitlyDeclared && !pRemoteLastStandardExplicitlyDeclared->IsCPM())
            return FALSE;

        if (pLocalCapSetH263->IsUMV())
          if (pRemoteLastStandardExplicitlyDeclared && !pRemoteLastStandardExplicitlyDeclared->IsUMV())
            return FALSE;

        if (pLocalCapSetH263->IsAMP())
          if (pRemoteLastStandardExplicitlyDeclared && !pRemoteLastStandardExplicitlyDeclared->IsAMP())
            return FALSE;

        if (pLocalCapSetH263->IsAC())
          if (pRemoteLastStandardExplicitlyDeclared && !pRemoteLastStandardExplicitlyDeclared->IsAC())
            return FALSE;

        if (pLocalCapSetH263->IsPB())
          if (pRemoteLastStandardExplicitlyDeclared && !pRemoteLastStandardExplicitlyDeclared->IsPB())
            return FALSE;
      }
    }

    // Additional capabilities.
    if (pLocalCapSetH263->IsCustomPCFFlag())   // Relevant for Custom Format/interlace.
    {
      // Check MPI
      if (pRemoteCapSetH263->IsCustomPCFFlag()) // Declare on MPI
      {
        if (pLocalCapSetH263->CalculateFramePerSecond() > pRemoteCapSetH263->CalculateFramePerSecond())
          return FALSE;
      }
      else
      {
        // Might not support interlace and therefore party will be secondary
        return FALSE;
      }
    }

    // Check additional capabilities compatibility
    if (pLocalCapSetH263->GetOptionsIndicator() != 5)
    {
      if (pLocalLastStandardExplicitlyDeclared)
      {
        if (pLocalLastStandardExplicitlyDeclared->IsOptions_1_FlagOn())
        {
          if (pRemoteLastFormatExplicitlyDeclared && !pRemoteLastFormatExplicitlyDeclared->IsOptions_1_FlagOn())
            return FALSE;

          // In case of Interlace mode : We do not check annex L status
          // because its optional although we declare on it.
          if (pLocalLastStandardExplicitlyDeclared->IsAnnex_P())
            if (pRemoteLastFormatExplicitlyDeclared && !pRemoteLastFormatExplicitlyDeclared->IsAnnex_P())
              return FALSE;

          if (pLocalLastStandardExplicitlyDeclared->IsAnnex_N())
            if (pRemoteLastFormatExplicitlyDeclared && !pRemoteLastFormatExplicitlyDeclared->IsAnnex_N())
              return FALSE;
        }
      }
    }

    // Check second additional capabilities compatibility.
    if (pLocalCapSetH263->GetSecondAdditionalH263Cap() != 0x40) // 0x40 means no Second Additional Cap.
    {
      if (pLocalLastStandardExplicitlyDeclared2)
      {
        if (pLocalLastStandardExplicitlyDeclared2->GetInterlaceMode())
          if (pRemoteLastFormatExplicitlyDeclared2 && !pRemoteLastFormatExplicitlyDeclared2->GetInterlaceMode())
            return FALSE;
      }
    }
  }
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
WORD CCapH263::IsSupportErrorCompensation() const
{
	CCapSetH263* pCapSetH263 = NULL;
	BYTE i = m_capH263->size();

	if(i) // support H263 Cap
	{
		// Check the last (=highest) resolution if it supports Error Compensation.
		std::multiset<CCapSetH263*, CompareByH263>::reverse_iterator revitr = m_capH263->rbegin();
	    pCapSetH263 = (*revitr);//at(i-1);
		if(!pCapSetH263->IsErrorCompensation_FlagOn())
			return FALSE;

		// Check the REST of declared resolution.
		for ( ++revitr; revitr != m_capH263->rend (); ++revitr )//for(--i ; i > 0 ; i--)
		{
		    pCapSetH263 = (*revitr);//at(i-1);
			if(pCapSetH263->GetOptionsIndicator() == 1) // Inherit options from immediately larger format
				continue;
			else if(!pCapSetH263->IsErrorCompensation_FlagOn())
				return FALSE;
		}
		return TRUE;
	}
	else  // Doesn't support H263 Cap
		return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
void CCapH263::RemoveDifferentAnnexes()
{
	//Remote all annexes which are not supported at all resolution.

	CCapSetH263* pCapSetH263 = NULL;
	size_t i = 0;

	WORD AnnexF = YES;
	WORD AnnexP = YES;
	WORD AnnexN = YES;

	std::multiset<CCapSetH263*, CompareByH263>::reverse_iterator revitr = m_capH263->rbegin();
	for ( ; revitr != m_capH263->rend (); ++revitr )// 	for( i = m_numberOfH263Sets ; i > 0 ; i--)
	{
	    pCapSetH263 = (*revitr);//at(i-1);

		if(!pCapSetH263->IsAMP())
		 	AnnexF = NO;
        if(!pCapSetH263->IsAnnex_P())
	     	AnnexP = NO;
		if(!pCapSetH263->IsAnnex_N())
			AnnexN = NO;
	}

	for ( revitr = m_capH263->rbegin(); revitr != m_capH263->rend (); ++revitr )// 	for( i = m_numberOfH263Sets ; i > 0 ; i--)
	{
		pCapSetH263 = (*revitr);//at(i-1);

		if(!AnnexF)
			pCapSetH263->SetAMP(NO);
		if(!AnnexP)
			pCapSetH263->SetAnnexP(NO);
		if(!AnnexN)
			pCapSetH263->SetAnnexN(NO);
	}

	// If No annex P or N are declared for all resulotion
	// reset m_IndividualOptionIndicator
	if(!AnnexP && !AnnexN)
	{
		for ( m_capH263->rbegin(); revitr != m_capH263->rend (); ++revitr )// for( i = m_numberOfH263Sets ; i > 0 ; i--)
		{
			pCapSetH263 = (*revitr);//at(i-1);
			pCapSetH263->SetOptionsIndicator(5);// value of 5 means no additional annexes are supported
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapH263::RemoveAnnexesForDBC2()
{
	//Remove all annexes

	CCapSetH263* pCapSetH263 = NULL;
	size_t i = 0;

	std::multiset<CCapSetH263*, CompareByH263>::reverse_iterator revitr = m_capH263->rbegin();
	for ( ; revitr != m_capH263->rend (); ++revitr )// 	for( i = m_numberOfH263Sets ; i > 0 ; i--)
	{
		pCapSetH263 = (*revitr);//at(i-1);

		pCapSetH263->SetAMP(NO);	//Annex F
		pCapSetH263->SetAnnexP(NO);
		pCapSetH263->SetAnnexN(NO);
		pCapSetH263->SetAnnexL(NO);
		pCapSetH263->SetAnnexT(NO);


		pCapSetH263->SetInterlaceMode(NO);	//??

		pCapSetH263->SetOptionsIndicator(5);// value of 5 means no additional annexes are supported
	}
}
/////////////////////////////////////////////////////////////////////////////
// return TRUE only if it the format is equal to one of the capset.
BYTE CCapH263::IsVidImageFormat(WORD format) const
{
    std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
		if( (*itr)->GetVidImageFormat() == format )
			return 1;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCapH263::IsCustomVidImageFormat(BYTE format) const
{
	BYTE ret_val = 0;
    std::multiset<CCapSetH263*, CompareByH263>::iterator itr = m_capH263->begin();
	for ( ; itr != m_capH263->end(); ++itr )
	{
	    CCapSetH263* pCurrentSet = (*itr);
		if(pCurrentSet != NULL)
		{
			if(pCurrentSet->GetCustomVidImageFormat() == format ){
				ret_val = 1;
			}else if(pCurrentSet->IsCustomTwoDistinctBoundsVidImageFormat(format)){
				ret_val = 2;
			}
		}
	}
	return ret_val;
}
/////////////////////////////////////////////////////////////////////////////////
BYTE CCapSetH263::IsCustomTwoDistinctBoundsVidImageFormat(BYTE format) const
{
		BYTE ret_val = NO;
		BYTE MinCustomPictureWidth = 0;
		BYTE MaxCustomPictureWidth = 0;
		BYTE MinCustomPictureHeight = 0;
		BYTE MaxCustomPictureHeight = 0;
		GetCustomVidImageFormatWidthAndHight(MinCustomPictureWidth,MaxCustomPictureWidth,MinCustomPictureHeight,MaxCustomPictureHeight);
		if(MaxCustomPictureWidth!=0 && MaxCustomPictureHeight!=0)
		{
			switch(format)
			{
			case VGA:
				{
					if(	MaxCustomPictureWidth >= 79 &&
						MinCustomPictureWidth <= 79 &&
						MaxCustomPictureHeight >=59 &&
						MinCustomPictureHeight <=59)
					{
						ret_val = YES;
					}
					break;
				}
			case NTSC:
				{
					if(	MaxCustomPictureWidth >= 87 &&
						MinCustomPictureWidth <= 87 &&
						MaxCustomPictureHeight >=59 &&
						MinCustomPictureHeight <=59)
					{
						ret_val = YES;
					}
					break;
				}
			case SVGA:
				{
					if(	MaxCustomPictureWidth >= 99 &&
						MinCustomPictureWidth <= 99 &&
						MaxCustomPictureHeight >=74 &&
						MinCustomPictureHeight <=74)
					{
						ret_val = YES;
					}
					break;
				}
			case XGA:
				{
					if(	MaxCustomPictureWidth >= 127 &&
						MinCustomPictureWidth <= 127 &&
						MaxCustomPictureHeight >=95 &&
						MinCustomPictureHeight <=95)
					{
						ret_val = YES;
					}
					break;
				}
			default:
			    break;
			}
		}
		return ret_val;
}
/////////////////////////////////////////////////////////////////////////////////
void CCapH263::SetOneH263Cap(BYTE format, int mpi)
{
	CCapSetH263* pCurrentCapSet = NULL;
	CCapSetH263* pNewCapSet = new CCapSetH263(format);
	std::multiset<CCapSetH263*,CompareByH263>::iterator iter = m_capH263->find(pNewCapSet);
	pCurrentCapSet = (iter != m_capH263->end())? *iter : NULL ;

	if ( pCurrentCapSet != NULL )
	{
		BYTE currentMpi = pCurrentCapSet->GetMPI();
		mpi = MIN_(mpi, currentMpi); //we want the highest one (30,15 => take the 30)
		if( format > H263_CIF_16 )
			pCurrentCapSet->SetCustomMPIIndicator(mpi) ;
		else
			pCurrentCapSet->SetMPI(mpi);
	}
	else
	{//insert a new one
		BYTE* pH263CapSetBuf = new BYTE[LengthOfH263Buffer];

		memset(pH263CapSetBuf,0,LengthOfH263Buffer);

		::GetVSWH263VideoFormatCapBuf (format, pH263CapSetBuf);

		if( format > H263_CIF_16 )
			pH263CapSetBuf[CustomMPIIndicator] = mpi;
		else
			pH263CapSetBuf[MPI] = mpi;

		CCapSetH263* pCapSetH263 = new CCapSetH263;
		pCapSetH263->Create(pH263CapSetBuf);

		InsertH263CapSet(pCapSetH263);
		PDELETEA(pH263CapSetBuf);
	}
	POBJDELETE(pNewCapSet);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// void CCapH263::Intersect(CCapH263& rRemoteCapH263,WORD& is_highest_common_param_changed)
// {
// 	// highest common phase 2 debug trace
// 	//CLargeString* sstr = new CLargeString();
// 	//*sstr << "CCapH263::Intersect :\n";

// 	// intersected capabilities
// 	CCapH263*    pCommonCapH263 = new CCapH263;
// 	// pointers for working set
// 	CCapSetH263* pLocalCurrentCapSet = NULL;

// 	// loop over cap sets, start from highest resolution (last set)
// 	for(size_t localSetIndex = m_numberOfH263Sets ; localSetIndex > 0 ; localSetIndex--)
// 	{
// 		// get local cap set
// 		pLocalCurrentCapSet = at(localSetIndex - 1);

// 		// highest common phase 2 debug trace
// 		// *sstr << "local set:\n";
// 		//pLocalCurrentCapSet->HcDump(sstr,FALSE);

// 		// get remote equivalent cap set
// 		CCapSetH263* pRemoteEquivalentCapSet = NULL;
// 		pRemoteEquivalentCapSet = rRemoteCapH263.FindEquivalentResolution(pLocalCurrentCapSet);
// 		if(!CPObject::IsValidPObjectPtr(pRemoteEquivalentCapSet)){
// 			// equivalent resolution not found at remote capabilities
// 			// local cap will not be added to the intersection, and changed flug turn on.
// 			is_highest_common_param_changed = TRUE;

// 			// highest common phase 2 debug trace
// 			//*sstr << "equivalent resolution not found\n\n";
// 		}else{
// 			// equivalent resolution found at remote capabilities
// 			// highest common phase 2 debug trace
// 			// *sstr << "Remote equivalent set:\n";
// 			// pRemoteEquivalentCapSet->HcDump(sstr,FALSE);

// 			CCapSetH263* pLocalAdditionalCapabilitiesCapSet = NULL;
// 			CCapSetH263* pLocalSecondAdditionalCapabilitiesCapSet = NULL;
// 			CCapSetH263* pLocalStandardFormatAdditionalCapabilitiesCapSet = NULL;

// 			BYTE local_additional_found = NO,local_second_additional_found = NO,local_standard_additional_found = NO;
// 			for(size_t localAdditionalCapSetSetIndex= localSetIndex-1; localAdditionalCapSetSetIndex < m_numberOfH263Sets; localAdditionalCapSetSetIndex++)
// 			{
// 				CCapSetH263* pLocalTestedCapSet = at(localAdditionalCapSetSetIndex);

// 				// Value of 1 means "Inherit options from immediately larger format"
// 				if(pLocalTestedCapSet->GetOptionsIndicator() != 1 && !local_additional_found){
// 					pLocalAdditionalCapabilitiesCapSet = pLocalTestedCapSet;
// 					local_additional_found = YES;
// 				}
// 				// additional standard format capabilities
// 				if(pLocalTestedCapSet->GetVidImageFormat() != H263_CUSTOM_FORMAT && pLocalTestedCapSet->IsOptionalCap() && !local_standard_additional_found){
// 					pLocalStandardFormatAdditionalCapabilitiesCapSet = pLocalTestedCapSet;
// 					local_standard_additional_found = YES;
// 				}
// 				// second additional caps declaired
// 				if(pLocalTestedCapSet->GetSecondAdditionalH263Cap() && !local_second_additional_found){
// 					pLocalSecondAdditionalCapabilitiesCapSet = pLocalTestedCapSet;
// 					local_second_additional_found = YES;
// 				}
// 			}

// 			// highest common phase 2 debug trace
// 			/*
// 			if(pLocalAdditionalCapabilitiesCapSet == NULL){
// 				*sstr << "pLocalAdditionalCapabilitiesCapSet not found\n";
// 			}else{
// 				*sstr << "Local additional papabilities set:\n";
// 				pLocalAdditionalCapabilitiesCapSet->HcDump(sstr,FALSE);
// 			}
// 			if(pLocalSecondAdditionalCapabilitiesCapSet == NULL){
// 				*sstr << "pLocalSecondAdditionalCapabilitiesCapSet not found\n";
// 			}*/


// 			size_t IndexOfRemoteCapSetH263 = 0;
// 			size_t IndexOfRemoteEquivalentCapSet = rRemoteCapH263.Index(pRemoteEquivalentCapSet);
// 			CCapSetH263* pRemoteAdditionalCapabilitiesCapSet = NULL;
// 			CCapSetH263* pRemoteSecondAdditionalCapabilitiesCapSet = NULL;
// 			CCapSetH263* pRemoteFirstStandardFormatCapSet = NULL;
// 			CCapSetH263* pRemoteStandardFormatAdditionalCapabilitiesCapSet = NULL;


// 			BYTE first_standard_found = NO,standard_additional_found = NO,additional_found = NO,second_additional_found = NO;
// 			for(size_t remoteAdditionalCapSetSetIndex= IndexOfRemoteEquivalentCapSet; remoteAdditionalCapSetSetIndex < rRemoteCapH263.GetNumberOfH263Sets(); remoteAdditionalCapSetSetIndex++)
// 			{
// 				CCapSetH263* pRemoteTestedCapSet = rRemoteCapH263.at(remoteAdditionalCapSetSetIndex);
// 				// Value of 1 means "Inherit options from immediately larger format"
// 				if(pRemoteTestedCapSet->GetOptionsIndicator() != 1 && !additional_found){
// 					pRemoteAdditionalCapabilitiesCapSet = pRemoteTestedCapSet;
// 					additional_found = YES;
// 				}
// 				// second additional caps declaired
// 				if(pRemoteTestedCapSet->GetSecondAdditionalH263Cap() && !second_additional_found){
// 					pRemoteSecondAdditionalCapabilitiesCapSet = pRemoteTestedCapSet;
// 					second_additional_found = YES;
// 				}

// 				if(pRemoteTestedCapSet->GetVidImageFormat() != H263_CUSTOM_FORMAT)
// 				{
// 					// first remote cap set can be custom
// 					if(!first_standard_found){
// 						pRemoteFirstStandardFormatCapSet = pRemoteTestedCapSet;
// 						first_standard_found = YES;
// 					}
// 					// additional standard format capabilities
// 					if(pRemoteTestedCapSet->IsOptionalCap() && !standard_additional_found){
// 						pRemoteStandardFormatAdditionalCapabilitiesCapSet = pRemoteTestedCapSet;
// 						standard_additional_found = YES;
// 					}
// 				}
// 			}

// 			// highest common phase 2 debug trace
// 			/*
// 			if(pRemoteAdditionalCapabilitiesCapSet == NULL){
// 				*sstr << "pRemoteAdditionalCapabilitiesCapSet not found\n";
// 			}else{
// 				*sstr << "Remote additional papabilities set:\n";
// 				pRemoteAdditionalCapabilitiesCapSet->HcDump(sstr,FALSE);
// 			}
// 			if(pRemoteSecondAdditionalCapabilitiesCapSet == NULL){
// 				*sstr << "pRemoteSecondAdditionalCapabilitiesCapSet not found\n";
// 			}
// 			if(pRemoteFirstStandardFormatCapSet == NULL){
// 				*sstr << "pRemoteFirstStandardFormatCapSet not found\n";
// 			}else{
// 				*sstr << "Remote first standard format set:\n";
// 				pRemoteFirstStandardFormatCapSet->HcDump(sstr,FALSE);
// 			}
// 			if(pRemoteStandardFormatAdditionalCapabilitiesCapSet == NULL){
// 				*sstr << "pRemoteStandardFormatAdditionalCapabilitiesCapSet not found\n";
// 			}

// 			if(pLocalCurrentCapSet->GetVidImageFormat() != H263_CUSTOM_FORMAT && pRemoteFirstStandardFormatCapSet == NULL)
// 			{
// 				*sstr << "Local current cap set is standard and equvalent standard remote cap set not found\n";
// 				continue;
// 			}*/

// 			CCapSetH263* pNewCommonCapSet = new CCapSetH263;
// 			WORD is_cap_set_hc_param_changed = FALSE;
// 			IntersectCapSet(*pNewCommonCapSet,is_cap_set_hc_param_changed,*pLocalCurrentCapSet,*pRemoteEquivalentCapSet,pLocalAdditionalCapabilitiesCapSet,pLocalStandardFormatAdditionalCapabilitiesCapSet,pLocalSecondAdditionalCapabilitiesCapSet,
// 				pRemoteAdditionalCapabilitiesCapSet,pRemoteSecondAdditionalCapabilitiesCapSet,pRemoteFirstStandardFormatCapSet,pRemoteStandardFormatAdditionalCapabilitiesCapSet);


// 			pCommonCapH263->InsertH263CapSet(pNewCommonCapSet);

// 			// highest common phase 2 debug trace
// 			// *sstr << "CCapH263::Intersect new (intersected) set:\n";
// 			// pNewCommonCapSet->HcDump(sstr,FALSE);
// 			// *sstr << "\n";

// 			if(is_cap_set_hc_param_changed){
// 				is_highest_common_param_changed = TRUE;

// 				// highest common phase 2 debug trace
// 				// *sstr << "CCapH263::Intersect is_highest_common_param_changed = TRUE\n";
// 			}
// 		}
// 	}
// 	// update data members
// 	*this = *pCommonCapH263;
// 	// delete intersection caps
// 	POBJDELETE(pCommonCapH263);

// 	// highest common phase 2 debug trace
// 	// PTRACE(eLevelInfoNormal,sstr->GetString());
// 	// POBJDELETE(sstr);
// }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapH263::IntersectCapSet(CCapSetH263& rNewCommonCapSetH263,
							   WORD& is_highest_common_param_changed,
							   CCapSetH263& rLocalCurrentCapSet,
							   CCapSetH263& rRemoteEquivalentCapSet,
							   CCapSetH263* pLocalAdditionalCapabilitiesCapSet,
							   CCapSetH263* pLocalStandardFormatAdditionalCapabilitiesCapSet,
							   CCapSetH263* pLocalSecondAdditionalCapabilitiesCapSet,
							   CCapSetH263* pRemoteAdditionalCapabilitiesCapSet,
							   CCapSetH263* pRemoteSecondAdditionalCapabilitiesCapSet,
							   CCapSetH263* pRemoteFirstStandardFormatCapSet,
							   CCapSetH263* pRemoteStandardFormatAdditionalCapabilitiesCapSet)
{
	// set video format (from local set)
	rNewCommonCapSetH263.SetVidImageFormatValue((WORD)rLocalCurrentCapSet.GetVidImageFormat());

	// set mpi and annex F for standard format (or promotion 50 fields)
	if(rLocalCurrentCapSet.GetVidImageFormat() != H263_CUSTOM_FORMAT)
	{
		if(pRemoteFirstStandardFormatCapSet != NULL)
		{
			IntersectCapSetStandardFormatParameters(rNewCommonCapSetH263,
													is_highest_common_param_changed,
													rLocalCurrentCapSet,
													pLocalStandardFormatAdditionalCapabilitiesCapSet,
													pRemoteFirstStandardFormatCapSet,
													pRemoteStandardFormatAdditionalCapabilitiesCapSet);
		}else{
			PTRACE(eLevelError,"IntersectCapSet - remote first standerd format cap set not found");
		}
	}


	IntersectCapSetCustomParameters(rNewCommonCapSetH263,
									is_highest_common_param_changed,
									rLocalCurrentCapSet,
									rRemoteEquivalentCapSet);

	// intersect annexes T, N and interlaced mode
	IntersectCapSetAdditionalCapabilities(rNewCommonCapSetH263,
										  is_highest_common_param_changed,
										  rLocalCurrentCapSet,
										  pLocalAdditionalCapabilitiesCapSet,
										  pLocalSecondAdditionalCapabilitiesCapSet,
										  pRemoteAdditionalCapabilitiesCapSet,
										  pRemoteSecondAdditionalCapabilitiesCapSet);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapH263::IntersectCapSetStandardFormatParameters(CCapSetH263& rNewCommonCapSet,
													   WORD& is_highest_common_param_changed,
													   CCapSetH263& rLocalCurrentCapSet,
													   CCapSetH263* pLocalStandardFormatAdditionalCapabilitiesCapSet,
													   CCapSetH263* pRemoteFirstStandardFormatCapSet,
													   CCapSetH263* pRemoteStandardFormatAdditionalCapabilitiesCapSet)
{
	// validity test of remote cap set
	if(pRemoteFirstStandardFormatCapSet == NULL){
		PTRACE(eLevelError,"CCapH263::IntersectCapSetStandardFormatParameters remote cap set is NULL");
		return;
	}
	// mpi intersection
	// we set the maximum mpi value => minimum frames per second
	if(rLocalCurrentCapSet.GetMPI() < pRemoteFirstStandardFormatCapSet->GetMPI()){
		rNewCommonCapSet.SetMpiFromBaseByte(pRemoteFirstStandardFormatCapSet->GetBaseCapsH263());
		is_highest_common_param_changed = TRUE;
	}else{
		rNewCommonCapSet.SetMpiFromBaseByte(rLocalCurrentCapSet.GetBaseCapsH263());
	}

	// intersect additional capabilities
	// intersect AMP (Annex F) (only parameter relevant for highest common)
	if(pLocalStandardFormatAdditionalCapabilitiesCapSet!=NULL)
	{
		if(pLocalStandardFormatAdditionalCapabilitiesCapSet->IsAMP()){
			if(pRemoteStandardFormatAdditionalCapabilitiesCapSet != NULL)
			{
				if(pRemoteStandardFormatAdditionalCapabilitiesCapSet->IsAMP()){
					rNewCommonCapSet.SetAMP(1);
				}else{
					is_highest_common_param_changed = TRUE;
				}
			}else{
				is_highest_common_param_changed = TRUE;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapH263::IntersectCapSetCustomParameters(CCapSetH263& rNewCommonCapSet,
											   WORD& is_highest_common_param_changed,
											   CCapSetH263& rLocalCurrentCapSet,
											   CCapSetH263& rRemoteEquivalentCapSet)
{
	// Set format indicator and format resolution;
	// because local format is <= remote format we set format from local (already intersection
    rNewCommonCapSet.SetFormatIndicator(rLocalCurrentCapSet.GetFormatIndicator());
    rNewCommonCapSet.SetVideoFormatResolution(rLocalCurrentCapSet.GetMinCustomPictureHeight(),
											  rLocalCurrentCapSet.GetMinCustomPictureWidth());

	if(rLocalCurrentCapSet.IsCustomPCFFlag())
	{
		rNewCommonCapSet.SetClockDivisor(rLocalCurrentCapSet.GetClockDivisor());
        rNewCommonCapSet.SetClockConversionCode(rLocalCurrentCapSet.GetClockConversionCode());

		// intersect custom MPI
		BYTE common_mpi = rLocalCurrentCapSet.GetCustomMPIIndicator();
		BYTE remote_mpi = rRemoteEquivalentCapSet.GetCustomMPIIndicator();
		// intersection is the highest mpi value ==> lower frames for seccond
		if(common_mpi < remote_mpi){
			common_mpi = remote_mpi;
			is_highest_common_param_changed = TRUE;
		}
		rNewCommonCapSet.SetCustomMPIIndicator(common_mpi);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapH263::IntersectCapSetAdditionalCapabilities(CCapSetH263& rNewCommonCapSet,
													 WORD& is_highest_common_param_changed,
													 CCapSetH263& rLocalCurrentCapSet,
													 CCapSetH263* pLocalAdditionalCapabilitiesCapSet,
													 CCapSetH263* pLocalSecondAdditionalCapabilitiesCapSet,
													 CCapSetH263* pRemoteAdditionalCapabilitiesCapSet,
													 CCapSetH263* pRemoteSecondAdditionalCapabilitiesCapSet)
{
	// Intersect annexes T , N
	// and check annex L intersection for Interlaced mode setting
	WORD common_annex_L = FALSE;
	WORD annex_L_changed = FALSE;
	if(rLocalCurrentCapSet.GetOptionsIndicator() != 5)// 5 = No additional h.263 options are supported.
	{
		if(pLocalAdditionalCapabilitiesCapSet != NULL)
		{
			if(pLocalAdditionalCapabilitiesCapSet->IsOptions_1_FlagOn())
			{
				if(pRemoteAdditionalCapabilitiesCapSet != NULL)
				{
					if(pRemoteAdditionalCapabilitiesCapSet->IsOptions_1_FlagOn())
					{
						// added annex T for highest common phase 2 (ron)
						if(pLocalAdditionalCapabilitiesCapSet->IsAnnex_T()){
							if(pRemoteAdditionalCapabilitiesCapSet->IsAnnex_T())
							{
								rNewCommonCapSet.SetAnnexT(TRUE);
							}else{
								is_highest_common_param_changed = TRUE;
							}
						}
						// added annex T for highest common phase 2 (ron)
						if(pLocalAdditionalCapabilitiesCapSet->IsAnnex_N()){
							if(pRemoteAdditionalCapabilitiesCapSet->IsAnnex_N())
							{
								rNewCommonCapSet.SetAnnexN(TRUE);
							}else{
								is_highest_common_param_changed = TRUE;
							}
						}
						// added annex T for highest common phase 2 (ron)
						if(pLocalAdditionalCapabilitiesCapSet->IsAnnex_L()){
							if(pRemoteAdditionalCapabilitiesCapSet->IsAnnex_L())
							{
								common_annex_L = TRUE;
							}else{
								annex_L_changed = TRUE;
							}
						}
					}
				}
			}
		}
	}

	// Intersect Interlaced mode
	if(rLocalCurrentCapSet.GetSecondAdditionalH263Cap() != 0x40)// 0x40 means no Second Additional Cap.
	{
		if(pLocalSecondAdditionalCapabilitiesCapSet != NULL)
		{
			if(pLocalSecondAdditionalCapabilitiesCapSet->GetInterlaceMode())
			{
				if(pRemoteSecondAdditionalCapabilitiesCapSet != NULL)
				{
					if(pRemoteSecondAdditionalCapabilitiesCapSet->GetInterlaceMode())
					{
						rNewCommonCapSet.SetInterlaceMode(1);
						if(common_annex_L == TRUE){
							rNewCommonCapSet.SetAnnexL(TRUE);
						}
						if(annex_L_changed){
							is_highest_common_param_changed = TRUE;
						}
					}else{
						is_highest_common_param_changed = TRUE;
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this function is protected version to use instead of RW at function
// it is recomended to use this function because
// the simptom of sending wrong index to the RW is very bad
// CCapSetH263* CCapH263::at(size_t index) const
// {
// 	if(index > m_capH263->size() - 1)
// 	{
// 		DBGPASSERT(index);
// 		return NULL;
// 	}
// 	return m_capH263->at(index);
// }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this function setting for vsw auto conf the full h.263 caps,
// including all parameters that supported in highest common (added for highest common phase 2, v7.00, Ron)
void  CCapH263::SetVswAutoFullCap(BYTE is_custom_formats, BYTE is_interlaced)
{
	PTRACE(eLevelInfoNormal,"CCapH263::SetVswAutoFullCap");

	// standerd formats
	BYTE formats_init_array[NUMBER_OF_H263_FORMATS] = {NO,NO,NO,NO,NO,NO,NO,NO,NO,NO};
	formats_init_array[H263_QCIF_SQCIF] = YES;
	formats_init_array[H263_CIF] = YES;
	formats_init_array[H263_CIF_4] = YES;
	formats_init_array[H263_CIF_16] = NO; // always NO we don't declare 16 cif any more

	// add custom formats
	// custom formats in vsw auto highest common are vga,svga and xga
	if(is_custom_formats){
		formats_init_array[VGA] = YES;
		formats_init_array[NTSC] = NO; // always NO (not in vsw auto highest common)
		formats_init_array[SVGA] = YES;
		formats_init_array[XGA] = YES;
	}

	// add interlaced mode
	if(is_interlaced)
	{
		formats_init_array[NTSC_60_FIELDS] = YES;
		formats_init_array[PAL_50_FIELDS] = YES;
	}

	SetVswAutoCaps(formats_init_array);

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// this function setting for vsw auto conf the full h.263 caps,
// including all parameters that supported in highest common (added for highest common phase 2, v7.00, Ron)
void  CCapH263::SetVswAutoCaps(const BYTE* formats_array)
{
	PTRACE(eLevelInfoNormal,"CCapH263::SetVswAutoFullCap");

	// Reset caps
	ClearAndDestroy();

	BYTE* pH263CapSetBuf = new BYTE[LengthOfH263Buffer];
	for(BYTE i = 0; i < LengthOfH263Buffer; i++){
		pH263CapSetBuf[i] = 0;
	}
	CCapSetH263* pCapSetH263 = NULL;


	for(WORD FormatNumber = 0; FormatNumber < NUMBER_OF_H263_FORMATS; FormatNumber++)
	{
		if(formats_array[FormatNumber] == NO){
			continue;
		}

		::GetVSWH263VideoFormatCapBuf( FormatNumber, pH263CapSetBuf);

		// set annex F that part of highest common for standerd formats
		if(FormatNumber==H263_QCIF_SQCIF || FormatNumber==H263_CIF || FormatNumber==H263_CIF_4)
		{
			pH263CapSetBuf[AMP]   = 1;
		}
		if(FormatNumber!=NTSC_60_FIELDS && FormatNumber!=PAL_50_FIELDS)
		{
			pH263CapSetBuf[Annex_T]   = 1;
			pH263CapSetBuf[Annex_N]   = 1;
		}

		// set interlaced annexes for ntsc
		if(FormatNumber==NTSC_60_FIELDS)
		{
			pH263CapSetBuf[Annex_L]   = 1;
			pH263CapSetBuf[Annex_N]   = 1;
		}

		pCapSetH263 = new CCapSetH263;
		pCapSetH263->Create(pH263CapSetBuf);
		InsertH263CapSet(pCapSetH263);
	}

	delete [] pH263CapSetBuf;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// constructor
CCapH263::CCapH263()
{
	m_numberOfH263Sets = 0;
	m_IsSendSecondAdditionalCap = YES;
	m_H263_2000 = 0;
    m_capH263 = new std::multiset<CCapSetH263*, CompareByH263>;
}
/////////////////////////////////////////////////////////////////////////////
CCapH263::CCapH263(const CCapH263& other) : CPObject(other)
{
    m_capH263 = new std::multiset<CCapSetH263*, CompareByH263>;

	m_numberOfH263Sets = other.m_capH263->size();

	if(other.m_numberOfH263Sets != other.m_capH263->size())
	{
		if(other.m_numberOfH263Sets)
			PASSERT(other.m_numberOfH263Sets);
		else if(other.m_capH263->size())
			PASSERT(other.m_capH263->size());
		else PASSERT(101);
	}
    CCapSetH263* pCapSetH263 = NULL;
    std::multiset<CCapSetH263*, CompareByH263>::iterator other_itr = other.m_capH263->begin();
	for ( ; other_itr != other.m_capH263->end(); ++other_itr )
	{
		 pCapSetH263 = new CCapSetH263;
		 *pCapSetH263 = *(*other_itr);
		 m_capH263->insert(pCapSetH263);
	}
}


void CCapH263::SetCapH263_2000()
{
    PTRACE(eLevelInfoNormal, "CCapH263::SetCapH263_2000 is called");
    m_H263_2000 = H263_2000;
}

BOOL CCapH263::IsH263VideoCap(WORD cap) const
{
    return (m_H263_2000 == cap);
}

/*BYTE CCapH263::IsCapableOfSD15() const
{
	return (m_numberOfH263Sets && GetHighestFormat() >= H263_CIF_4);
}
*/
/////////////////////////////////////////////////////////////////////////
eVideoPartyType CCapH263::GetCPVideoPartyTypeAccordingToCapabilities(BYTE isH2634Cif15Supported)
{
	//eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	BYTE is4CIF = NO;
	BYTE highestH263Format = GetHighestFormat();
	if(m_numberOfH263Sets && (highestH263Format >= H263_CIF_4) && isH2634Cif15Supported)
	{
	  is4CIF = YES;
	}
	TRACESTR(eLevelInfoNormal) << "CCapH263::GetCPVideoPartyTypeAccordingToCapabilities , is4CIF = " << (WORD)is4CIF << " , highestH263Format = " << (WORD)highestH263Format << " , isH2634Cif15Supported = " << (WORD)isH2634Cif15Supported;
	eVideoPartyType videoPartyType = GetH261H263ResourcesPartyType(is4CIF);
	TRACESTR(eLevelInfoNormal) << "CCapH263::GetCPVideoPartyTypeAccordingToCapabilities , videoPartyType = " << eVideoPartyTypeNames[videoPartyType];
	return videoPartyType;
}
/////////////////////////////////////////////////////////////////////////
