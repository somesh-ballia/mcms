#ifndef _H320AUDIOCAPS_H
#define _H320AUDIOCAPS_H

#include <set>
#include "PObject.h"
#include "AudPortDefinitions.h"
#include "EncryptionDefines.h"

class CSegment;

/* The enum defines all audio algorithms that signalled in H221 A.1 table
   in the capabilities columns
   not including values for non standard algorithms. */
typedef enum
{
	e_Neutral,
	e_A_Law,
	e_U_Law,
	e_G722_64,
	e_G722_56,
	e_G722_48,
	e_Au_16k,
	e_Au_Iso,
/*   e_G723_1, //IP only */
/*   e_G729,   //IP only */
	e_G722_1_32,
	e_G722_1_24,
	e_G722_1_Annex_C_48,
	e_G722_1_Annex_C_32,
	e_G722_1_Annex_C_24,
	e_Audio_Cap_Last
} EAudioCapAlgorithm;

typedef enum
{
	e_Xfer_Cap_Sm_comp,
	e_Xfer_Cap_128,
	e_Xfer_Cap_192,
	e_Xfer_Cap_256,
	e_Xfer_Cap_320,
	e_Xfer_Cap_512,
	e_Xfer_Cap_768,
	e_Xfer_Cap_1152,
	e_Xfer_Cap_B,
	e_Xfer_Cap_2B,
	e_Xfer_Cap_3B,
	e_Xfer_Cap_4B,
	e_Xfer_Cap_5B,
	e_Xfer_Cap_6B,
	e_Xfer_Cap_Restrict,
	e_Xfer_Cap_6B_H0_Comp,
	e_Xfer_Cap_H0,
	e_Xfer_Cap_2H0,
	e_Xfer_Cap_3H0,
	e_Xfer_Cap_4H0,
	e_Xfer_Cap_5H0,
	e_Xfer_Cap_1472,
	e_Xfer_Cap_H11,
	e_Xfer_Cap_H12,
	e_Xfer_Cap_Restrict_L,
	e_Xfer_Cap_Restrict_P,
	e_Xfer_Cap_NoRestrict,
	e_Xfer_Cap_Last
} ERateCapAlgorithm;

#define DBC2_PARAM (0x28) // system params 00101000

////////////////////////////////////////////////////////////////////////////
//                        CAudioAlg
////////////////////////////////////////////////////////////////////////////
/* This class describes one specific audio algorithm */
class CAudioAlg : public CPObject
{
	CLASS_TYPE_1(CAudioAlg, CPObject)

public:
	                       CAudioAlg();
	                       CAudioAlg(EAudioCapAlgorithm alg);

	virtual const char*    NameOf() const { return "CAudioAlg"; }
	virtual void           Dump(std::ostream&) const;

	void                   Serialize(WORD format, CSegment& stringSeg) const;
	void                   DeSerialize(WORD format, CSegment& stringSeg);

	EAudioCapAlgorithm     GetAudioAlg() const { return m_audRateCap; }
	const char*            GetAlgName() const;

	static BOOL            IsLegal(DWORD dwAudioAlgorithm);
	static DWORD           GetAudioPriority(DWORD alg);

	bool                   operator<(const CAudioAlg& other) const;

private:
	EAudioCapAlgorithm     m_audRateCap;    // audio capabilities
};


/* CompareByAudioPriority is comparison class: A class that takes two arguments
   of the same CAudioAlg type (container's elements) and returns true if p1 less then p2 */
struct CompareByAudioPriority
{
	bool operator()(const CAudioAlg* p1, const CAudioAlg* p2) const
	{
		return (* p1) < (* p2);
	}
};


////////////////////////////////////////////////////////////////////////////
//                        CAudioCap
////////////////////////////////////////////////////////////////////////////
/* This class is intended for audio capabilities definition.
   It keeps list of supported audio algorithms */
class CAudioCap : public CPObject
{
	CLASS_TYPE_1(CAudioCap, CPObject)

public:
	                       CAudioCap();
	                       CAudioCap(const CAudioCap& other);
	                      ~CAudioCap();

	virtual const char*    NameOf() const { return "CAudioCap"; }
	virtual void           Dump(std::ostream&) const;

	static EAudioAlgorithm TranslateAudioCap(WORD cap);

	void                   Serialize(WORD format, CSegment& stringSeg);
	void                   DeSerialize(WORD format, CSegment& stringSeg);

	CAudioCap&             operator=(const CAudioCap& other);

	BOOL                   IsAudioAlgSupported(EAudioCapAlgorithm cap) const;

	void                   AddAudioAlg(EAudioCapAlgorithm cap);
	void                   RemoveAudioAlg(EAudioCapAlgorithm cap);
	void                   SetAudioDefaultAlg();
	void                   ResetAudioCap();

private:
	void                   ClearAndDestroy();

	std::set<CAudioAlg*, CompareByAudioPriority> m_audioAlgSet;
};


////////////////////////////////////////////////////////////////////////////
//                        CRateAlg
////////////////////////////////////////////////////////////////////////////
/* This class describes one specific transfer algorithm */
class CRateAlg : public CPObject
{
	CLASS_TYPE_1(CRateCap, CPObject)

public:
	                       CRateAlg();
	                       CRateAlg(ERateCapAlgorithm cap);

	virtual const char*    NameOf() const { return "CRateAlg"; }
	virtual void           Dump(std::ostream&) const;

	static BOOL            IsLegal(DWORD rate);

	void                   Serialize(WORD format, CSegment& stringSeg) const;
	void                   DeSerialize(WORD format, CSegment& stringSeg);

	ERateCapAlgorithm      GetRateAlg() const { return m_rateCap; }
	const char*            GetAlgName() const;
	DWORD                  GetNumChnl() const;
	bool                   operator<(const CRateAlg& other) const;

private:
	ERateCapAlgorithm      m_rateCap;       // rate capability
};


/* CompareByRatePriority is comparison class: a class that takes two arguments
   of the same CRateAlg type (container's elements) and returns true if p1 less then p2 */
struct CompareByRatePriority
{
	bool operator()(const CRateAlg* p1, const CRateAlg* p2) const
	{
		return (* p1) < (* p2);
	}
};



/* This class is intended for transfer capabilities definition.
   It keeps list of supported transfer algorithms */
class CRateCap : public CPObject
{
	CLASS_TYPE_1(CRateCap, CPObject)

public:
	                       CRateCap();
	                       CRateCap(const CRateCap& other);
	                      ~CRateCap();

	virtual const char*    NameOf() const { return "CRateCap"; }
	virtual void           Dump(std::ostream&) const;

	CRateCap&              operator=(const CRateCap& other);

	void                   Serialize(WORD format, CSegment& stringSeg);
	void                   DeSerialize(WORD format, CSegment& stringSeg);

	void                   AddXferCap(WORD cap);
	void                   ResetXferCap();

	void                   SetXferCapB();
	void                   SetXferCap2B();
	void                   SetXferCap3B();
	void                   SetXferCap4B();
	void                   SetXferCap5B();
	void                   SetXferCap6B();

	void                   SetXferCapH0();
	void                   SetXferCap2H0();
	void                   SetXferCap3H0();
	void                   SetXferCap4H0();
	void                   SetXferCap5H0();

	BOOL                   IsXferCapSupported(WORD cap) const;

private:
	void                   ClearAndDestroy();

	std::multiset<CRateAlg*, CompareByRatePriority> m_rateAlgSet; // rate capabilities
};


////////////////////////////////////////////////////////////////////////////
//                        CEncrypAlg
////////////////////////////////////////////////////////////////////////////
class CEncrypAlg : public CPObject
{
	CLASS_TYPE_1(CEncrypAlg, CPObject)

public:
	                       CEncrypAlg();
	                       CEncrypAlg(CEncrypAlg& other);
	virtual               ~CEncrypAlg();

	virtual const char*    NameOf() const { return "CEncrypAlg"; }

	CEncrypAlg&            operator=(const CEncrypAlg& other);
	BYTE                   operator==(const CEncrypAlg& other);

	void                   Create(BYTE params, BYTE alg, BYTE media);

	void                   SetAlg(BYTE newAlg)       { m_alg = newAlg; }
	void                   SetParams(BYTE newParams) { m_params = newParams; }
	void                   SetMedia(BYTE newMedia)   { m_media = newMedia; }
	BYTE                   GetAlg() const            { return m_alg; }
	BYTE                   GetParams() const         { return m_params; }
	BYTE                   GetMedia() const          { return m_media; }

	void                   DeSerialize(WORD format, CSegment& seg);
	void                   Serialize(WORD format, CSegment& seg);

protected:
	BYTE                   m_alg;
	BYTE                   m_params;
	BYTE                   m_media;
};


////////////////////////////////////////////////////////////////////////////
//                        CCapECS
////////////////////////////////////////////////////////////////////////////
class CCapECS : public CPObject
{
	CLASS_TYPE_1(CCapECS, CPObject)

public:
	                       CCapECS();
	                       CCapECS(const CCapECS& other);
	virtual               ~CCapECS();

	virtual const char*    NameOf() const { return "CCapECS"; }

	CCapECS&               operator=(CCapECS& other);

	void                   Serialize(WORD format, CSegment& ECSStringSeg);
	void                   SerializeP0(CSegment& ECSStringSeg);
	void                   DeSerialize(WORD format, CSegment& ECSStringSeg);

	void                   SetKeyExchCap(BYTE newKeyExch);
	BYTE                   GetKeyExchCap();
	void                   CreateLocalCapECS();
	BYTE                   KeyExchCapIncludesDH();
	BYTE                   EncrypAlgCapIncludesAES128();
	void                   SaveECSP0(CSegment* pParam);
	void                   SaveECSP8(CSegment* pParam);

protected:
	CEncrypAlg*            m_pEncrypAlg;
	BYTE                   m_keyExchCap;
	BYTE                   m_numAlgo;
};


////////////////////////////////////////////////////////////////////////////
//                        CEncrypCap
////////////////////////////////////////////////////////////////////////////
/* This class defines encryption capabilities */
class CEncrypCap : public CPObject
{
	CLASS_TYPE_1(CEncrypCap, CPObject)

public:
	                    CEncrypCap();
	                    CEncrypCap(const CEncrypCap& other);
	virtual            ~CEncrypCap();

	virtual const char* NameOf() const;
	virtual void        Dump(std::ostream&) const;

	void                Serialize(WORD format, CSegment& stringSeg);
	void                DeSerialize(WORD format, CSegment& stringSeg);

	CEncrypCap& operator=(const CEncrypCap& other);

	void                CreateDefault();
	void                CreateCCapECS();
	void                CreateCCapECS(const CCapECS* pCapECS);
	void                CreateLocalCapECS();
	void                SetEncrypCap(BYTE cap);
	void                RemoveEncrypCap();
	BOOL                IsEncrypCapOn(BYTE cap) const { return m_encrypCap == cap; }

	CCapECS*            GetCapECS() const;

private:

	CCapECS* m_pCapECS;
	BYTE     m_encrypCap;
};

#endif /* _H320AUDIOCAPS_H  */
