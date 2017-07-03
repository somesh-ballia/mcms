#ifndef DTMFALGDB_H_
#define DTMFALGDB_H_

class CDtmfElement;
class CDtmfAlgSet;



#include "DataTypes.h"
#include "PObject.h"
#include "TraceStream.h"
#include "IpChannelParams.h"
#include "AudHostApiDefinitions.h"
#include "MediaMngr.h"

typedef enum
{
	//G711 64K
	E_G711_64K_DTMF_ALG,
	//G722
	E_G722_48K_DTMF_ALG,                                   
	E_G722_56K_DTMF_ALG,
	E_G722_64K_DTMF_ALG,
	//G729
	E_G729_8K_DTMF_ALG,                                     
	//G722.1
	E_G722_1_24K_DTMF_ALG,
	E_G722_1_32K_DTMF_ALG,
	//Siren14	
	E_SIREN_14_24K_DTMF_ALG,
	E_SIREN_14_32K_DTMF_ALG,
	E_SIREN_14_48K_DTMF_ALG,
	//Siren14 Stereo
	E_SIREN_14_STEREO_48K_DTMF_ALG,
	E_SIREN_14_STEREO_56K_DTMF_ALG,
	E_SIREN_14_STEREO_64K_DTMF_ALG,
	E_SIREN_14_STEREO_96K_DTMF_ALG,
	//G722.1.C
	E_G722_1_C_24K_DTMF_ALG,
	E_G722_1_C_32K_DTMF_ALG,
	E_G722_1_C_48K_DTMF_ALG,
	//G719
	E_G719_32K_DTMF_ALG,
	E_G719_48K_DTMF_ALG,
	E_G719_64K_DTMF_ALG,
	//G719 Stereo
	E_G719_STEREO_64K_DTMF_ALG,
	E_G719_STEREO_96K_DTMF_ALG,
	E_G719_STEREO_128K_DTMF_ALG,
	//Siren22
	E_SIREN_22_32K_DTMF_ALG,
	E_SIREN_22_48K_DTMF_ALG,
	E_SIREN_22_64K_DTMF_ALG,
	//Siren22 Stereo
	E_SIREN_22_STEREO_64K_DTMF_ALG,
	E_SIREN_22_STEREO_96K_DTMF_ALG,
	E_SIREN_22_STEREO_128K_DTMF_ALG,
	//SirenLPR
	E_SIREN_LPR_32K_DTMF_ALG,
	E_SIREN_LPR_48K_DTMF_ALG,
	E_SIREN_LPR_64K_DTMF_ALG,
	E_SIREN_LPR_STEREO_48K_DTMF_ALG,
	E_SIREN_LPR_STEREO_64K_DTMF_ALG,
	E_SIREN_LPR_STEREO_96K_DTMF_ALG,
	E_SIREN_LPR_STEREO_128K_DTMF_ALG,

	E_ALG_DTMF_COUNT
} EDtmfAlg;


static const char * EDtmfAlgStr[] = 
{
	"G711_64K_DTMF_ALG",
	"G722_48K_DTMF_ALG",
	"G722_56K_DTMF_ALG",
	"G722_64K_DTMF_ALG",	
	"G729_8K_DTMF_ALG",
	"G722_1_24K_DTMF_ALG",
	"G722_1_32K_DTMF_ALG",
	"SIREN_14_24K_DTMF_ALG",
	"SIREN_14_32K_DTMF_ALG",
	"SIREN_14_48K_DTMF_ALG",
	"SIREN_14_STEREO_48K_DTMF_ALG",
	"SIREN_14_STEREO_56K_DTMF_ALG",
	"SIREN_14_STEREO_64K_DTMF_ALG",
	"SIREN_14_STEREO_96K_DTMF_ALG",
	"G722_1_C_24K_DTMF_ALG",
	"G722_1_C_32K_DTMF_ALG",
	"G722_1_C_48K_DTMF_ALG",
	"G719_32K_DTMF_ALG",
	"G719_48K_DTMF_ALG",
	"G719_64K_DTMF_ALG",
	"G719_STEREO_64K_DTMF_ALG",
	"G719_STEREO_96K_DTMF_ALG",
	"G719_STEREO_128K_DTMF_ALG",
	"SIREN_22_32K_DTMF_ALG",
	"SIREN_22_48K_DTMF_ALG",
	"SIREN_22_64K_DTMF_ALG",
	"SIREN_22_STEREO_64K_DTMF_ALG",
	"SIREN_22_STEREO_96K_DTMF_ALG",
	"SIREN_22_STEREO_128K_DTMF_ALG",
	"SIREN_LPR_32K_DTMF_ALG",
	"SIREN_LPR_48K_DTMF_ALG",
	"SIREN_LPR_64K_DTMF_ALG",
	"SIREN_LPR_STEREO_64K_DTMF_ALG",
	"SIREN_LPR_STEREO_96K_DTMF_ALG",
	"SIREN_LPR_STEREO_128K_DTMF_ALG",

	"ALG_DTMF_COUNT"
};



typedef enum
{
	E_DTMF_0,
	E_DTMF_1,
	E_DTMF_2,
	E_DTMF_3,
	E_DTMF_4,
	E_DTMF_5,
	E_DTMF_6,
	E_DTMF_7,
	E_DTMF_8,
	E_DTMF_9,
	E_DTMF_STAR,
	E_DTMF_POUND,
	E_DTMF_COUNT
} EDtmfIndex;


static const char * EDtmfIndexStr[] = 
{
	"DTMF_0",
	"DTMF_1",
	"DTMF_2",
	"DTMF_3",
	"DTMF_4",
	"DTMF_5",
	"DTMF_6",
	"DTMF_7",
	"DTMF_8",
	"DTMF_9",
	"DTMF_STAR",
	"DTMF_POUND",
	"DTMF_COUNT"
};



class CDtmfAlgDB : public CPObject
{
	CLASS_TYPE_1(CDtmfAlgDB,CPObject)
public:
	CDtmfAlgDB();
	virtual ~CDtmfAlgDB();
	
	void Init();
		
	virtual const char*	NameOf () const{return "CDtmfAlgDB";}
	
	//Methods
	CDtmfElement* GetDtmfElement(CapEnum audioAlg, EAudioTone audioTone);
	

private:
	int GetAlgDtmf(CapEnum audioAlg);
	int GetAudioTone(EAudioTone audioTone);
	

private:
	CDtmfAlgSet* m_dtmfAlgArray[E_ALG_DTMF_COUNT];
};




class CDtmfAlgSet : public CPObject
{
	CLASS_TYPE_1(CDtmfAlgSet,CPObject)
public:
	CDtmfAlgSet();
	virtual ~CDtmfAlgSet();
	
	
	virtual const char*	NameOf () const{return "CDtmfAlgSet";}
	
	//Methods
	void Init(int dtmfAlg);
	CDtmfElement* GetDtmfElement(int dtmfIndex);
	

private:
	CDtmfElement* m_dtmfArray[E_DTMF_COUNT];
	EDtmfAlg m_dtmfAlgType;
};





class CDtmfElement : public CPObject
{
	CLASS_TYPE_1(CDtmfElement,CPObject)
public:
	CDtmfElement();
	virtual ~CDtmfElement();
	
	
	virtual const char*	NameOf () const{return "CDtmfElement";}
	
	//Methods
	void Init(int dtmfTone, EDtmfAlg dtmfAlgType);
	BYTE* GetDtmfBuffer() { return m_dtmfBuffer;}
	int GetDtmfBufferSize() { return m_dtmfBufferSize;}
	
	string GetDtmfFileName(int dtmfTone, EDtmfAlg dtmfAlgType);
	int ReadDtmfFile(string fileName);
	
	string GetFullFileName() {return m_strFullFileName;}

private:
	BYTE* m_dtmfBuffer;
	int m_dtmfBufferSize;
	string m_strFullFileName;
};



#endif /*DTMFALGDB_H_*/
