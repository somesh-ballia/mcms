#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <memory>
#include "Macros.h"
#include "H221.h"
#include "Segment.h"
#include "ObjString.h"
#include "H320AudioCaps.h"
#include "VideoDefines.h"


////////////////////////////////////////////////////////////////////////////
//                        CAudioAlg
////////////////////////////////////////////////////////////////////////////
CAudioAlg::CAudioAlg()
{
	m_audRateCap = (EAudioCapAlgorithm)0L;
}

//--------------------------------------------------------------------------
CAudioAlg::CAudioAlg(EAudioCapAlgorithm alg)
{
	m_audRateCap = (EAudioCapAlgorithm)0L;

	if (CAudioAlg::IsLegal(alg))
		m_audRateCap = alg;
}

//--------------------------------------------------------------------------
void CAudioAlg::Dump(std::ostream& msg) const
{
	msg << std::setw(20) << "m_audRateCap" << (std::hex) << m_audRateCap
	    << std::setw(20) << "; audio algorithm is " << GetAlgName() <<"\n";
}

//--------------------------------------------------------------------------
const char* CAudioAlg::GetAlgName() const
{
	switch (m_audRateCap)
	{
		case e_G722_1_Annex_C_24: return "G7221_AnnexC_24k";
		case e_G722_1_Annex_C_32: return "G7221_AnnexC_32k";
		case e_G722_1_Annex_C_48: return "G7221_AnnexC_48k";
		case e_G722_1_24        : return "G722_1_24"; // "Au_24k";
		case e_G722_1_32        : return "G722_1_32"; // "Au_32k";
		case e_Neutral          : return "Neutral";
		case e_A_Law            : return "A_Law";
		case e_U_Law            : return "U_Law";
		case e_G722_64          : return "G722_64";
		case e_G722_56          : return "G722_56";
		case e_G722_48          : return "G722_48";
		case e_Au_16k           : return "G728";
		case e_Au_Iso           : return "Au_Iso";
	//case e_G723_1           : return "G723_1";
	//case e_G729             : return "G729";
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return "Unknown";
}

//--------------------------------------------------------------------------
BOOL CAudioAlg::IsLegal(DWORD dwAudioAlgorithm) // static
{
	BOOL resultValue = (((EAudioCapAlgorithm)dwAudioAlgorithm >= e_Neutral) &&
	                    (dwAudioAlgorithm < e_Audio_Cap_Last));
	return resultValue;
}

//--------------------------------------------------------------------------
DWORD CAudioAlg::GetAudioPriority(DWORD alg)
{
	switch (alg)   // must be reviewed!!!
	{
		case e_G722_1_Annex_C_24: return 1; // G7221_AnnexC_24k:
		case e_G722_1_Annex_C_32: return 2; // G7221_AnnexC_32k:
		case e_G722_1_Annex_C_48: return 3; // G7221_AnnexC_48k:
		case e_G722_1_24        : return 4; // Au_24k:
		case e_G722_1_32        : return 5; // Au_32k:
		case e_G722_48          : return 6; // G722_m3:
		case e_Au_16k           : return 7; // G728:
		case e_G722_64          : return 8; // G722_m1:
		case e_Au_Iso           : return 9; // ???
	//case e_G723_1           : return 10; //IP only
	//case e_G729             : return 11; //IP only
		case e_Neutral          : return 12; // ???
		case e_A_Law            : return 13; // ???
		case e_U_Law            : return 14; // ???
	}
	return 0;
}

//--------------------------------------------------------------------------
void CAudioAlg::Serialize(WORD format, CSegment& stringSeg) const
{
	switch (format)
	{
		case NATIVE:
			stringSeg << (DWORD)m_audRateCap;
			break;

		case SERIALEMBD:
		{
			switch (m_audRateCap)
			{
				case e_Neutral:
				{
					BYTE seriableVal = Neutral | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_A_Law:
				{
					BYTE seriableVal = A_Law | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_U_Law:
				{
					BYTE seriableVal = U_Law | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_G722_64:
				{
					BYTE seriableVal = G722_64 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_G722_56:
				case e_G722_48:
				{
					BYTE seriableVal = G722_48 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Au_16k:
				{
					BYTE seriableVal = Au_16k | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Au_Iso:
				{
					BYTE seriableVal = Au_Iso | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_G722_1_32:
				{
					BYTE seriableVal = G722_1_32 | OTHERCAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_G722_1_24:
				{
					BYTE seriableVal = G722_1_24 | OTHERCAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_G722_1_Annex_C_48:
				{
					BYTE seriableVal = G722_1_Annex_C_48 | OTHERCAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_G722_1_Annex_C_32:
				{
					BYTE seriableVal = G722_1_Annex_C_32 | OTHERCAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_G722_1_Annex_C_24:
				{
					BYTE seriableVal = G722_1_Annex_C_24 | OTHERCAPATTR;
					stringSeg << seriableVal;
					break;
				}
				default:
				{
					std::ostringstream str;
					str << "illegal value = " << m_audRateCap;
					PTRACE2(eLevelInfoNormal, "CAudioAlg::Serialize", str.str().c_str());
					break;
				}
			} // switch
		}
		break;
	} // switch
}

//--------------------------------------------------------------------------
void CAudioAlg::DeSerialize(WORD format, CSegment& stringSeg)
{
	switch (format)
	{
		case SERIALEMBD:
		{
			PTRACE(eLevelInfoNormal, "CAudioAlg::DeSerialize - case SERIALEMBD - do nothing");
			break;
		}
		case NATIVE:
		{
			DWORD temp;
			stringSeg >> temp;
			m_audRateCap = (EAudioCapAlgorithm)temp;
			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
bool CAudioAlg::operator<(const CAudioAlg& other) const
{
	return (GetAudioPriority(m_audRateCap) > GetAudioPriority(other.m_audRateCap));
}


////////////////////////////////////////////////////////////////////////////
//                        CAudioCap
////////////////////////////////////////////////////////////////////////////
CAudioCap::CAudioCap()
{
}

//--------------------------------------------------------------------------
CAudioCap::CAudioCap(const CAudioCap& other) : CPObject(other)
{
	std::set<CAudioAlg*, CompareByAudioPriority>::iterator other_itr = other.m_audioAlgSet.begin();
	for (; other_itr != other.m_audioAlgSet.end(); ++other_itr)
	{
		CAudioAlg* aalg = new CAudioAlg;
		*aalg = *(*other_itr);
		m_audioAlgSet.insert(aalg);
	}
}

//--------------------------------------------------------------------------
CAudioCap::~CAudioCap()
{
	ClearAndDestroy();
}

//--------------------------------------------------------------------------
void CAudioCap::ClearAndDestroy()
{
	std::set<CAudioAlg*, CompareByAudioPriority>::iterator itr = m_audioAlgSet.begin();
	for ( ; itr != m_audioAlgSet.end (); ++itr) {
		CAudioAlg* temp = *itr;
		POBJDELETE(temp);
	}
	m_audioAlgSet.clear();
}

//--------------------------------------------------------------------------
void CAudioCap::Dump(std::ostream& msg) const
{
	msg << "\n==================    CAudioCap::Dump    ==================\n";
	std::set<CAudioAlg*, CompareByAudioPriority>::iterator itr = m_audioAlgSet.begin();
	for (; itr != m_audioAlgSet.end(); ++itr)
		(*itr)->Dump(msg);
}

//--------------------------------------------------------------------------
CAudioCap& CAudioCap::operator=(const CAudioCap& other)
{
	ClearAndDestroy();

	std::set<CAudioAlg*, CompareByAudioPriority>::iterator other_itr = other.m_audioAlgSet.begin();
	for (; other_itr != other.m_audioAlgSet.end(); ++other_itr)
	{
		CAudioAlg* aalg = new CAudioAlg;
		*aalg = *(*other_itr);
		m_audioAlgSet.insert(aalg);
	}

	return *this;
}

//--------------------------------------------------------------------------
BOOL CAudioCap::IsAudioAlgSupported(EAudioCapAlgorithm cap) const
{
	std::set<CAudioAlg*, CompareByAudioPriority>::iterator itr = m_audioAlgSet.begin();
	for (; itr != m_audioAlgSet.end(); ++itr)
		if ((*itr)->GetAudioAlg() == cap)
			return 1;

	return 0;
}

//--------------------------------------------------------------------------
void CAudioCap::AddAudioAlg(EAudioCapAlgorithm cap)
{
	if (CAudioAlg::IsLegal(cap) && !IsAudioAlgSupported(cap))
	{
		CAudioAlg* aalg = new CAudioAlg(cap);
		m_audioAlgSet.insert(aalg);
	}
}

//--------------------------------------------------------------------------
void CAudioCap::RemoveAudioAlg(EAudioCapAlgorithm cap)
{
	if (!m_audioAlgSet.empty())
	{
		CAudioAlg aalg((EAudioCapAlgorithm)cap);
		std::set<CAudioAlg*, CompareByAudioPriority>::iterator itr = m_audioAlgSet.find(&aalg);
		if (itr != m_audioAlgSet.end())
		{
			CAudioAlg* temp = *itr;
			m_audioAlgSet.erase(itr);
			POBJDELETE(temp);
		}
	}
}

//--------------------------------------------------------------------------
void CAudioCap::SetAudioDefaultAlg()
{
	// if no audio capability
	if (!IsAudioAlgSupported(e_A_Law) && !IsAudioAlgSupported(e_U_Law))
	{
		AddAudioAlg(e_A_Law);
		AddAudioAlg(e_U_Law);
	}
}

//--------------------------------------------------------------------------
void CAudioCap::ResetAudioCap()
{
	ClearAndDestroy();
}

//--------------------------------------------------------------------------
void CAudioCap::Serialize(WORD format, CSegment& stringSeg)
{
	WORD size = m_audioAlgSet.size();
	if (NATIVE == format)
		stringSeg << size;

	std::set<CAudioAlg*, CompareByAudioPriority>::iterator itr = m_audioAlgSet.begin();
	for (; itr != m_audioAlgSet.end(); ++itr)
		(*itr)->Serialize(format, stringSeg);
}

//--------------------------------------------------------------------------
void CAudioCap::DeSerialize(WORD format, CSegment& stringSeg)
{
	if (format != NATIVE)
		return;

	WORD size;
	stringSeg >> size;
	for (WORD i = 0; i < size && !stringSeg.EndOfSegment(); i++)
	{
		DWORD temp;
		stringSeg >> temp;
		this->AddAudioAlg((EAudioCapAlgorithm)temp);
	}
}


////////////////////////////////////////////////////////////////////////////
//                        CRateAlg
////////////////////////////////////////////////////////////////////////////
CRateAlg::CRateAlg()
{
	m_rateCap = (ERateCapAlgorithm)0L;
}

//--------------------------------------------------------------------------
CRateAlg::CRateAlg(ERateCapAlgorithm cap)
{
	m_rateCap = (ERateCapAlgorithm)0L;
	if (CRateAlg::IsLegal(cap))
		m_rateCap = cap;
}

//--------------------------------------------------------------------------
void CRateAlg::Dump(std::ostream& msg) const
{
	msg << std::setw(20) << "m_rateCap" << (std::hex)  << m_rateCap
	    << std::setw(20) << "; transfer agorithm is " << GetAlgName() <<"\n";
}

//--------------------------------------------------------------------------
void CRateAlg::Serialize(WORD format, CSegment& stringSeg) const
{
	BYTE seriableVal = 0;
	switch (format)
	{
		case NATIVE:
		{
			stringSeg << (DWORD)m_rateCap;
		} break;

		case SERIALEMBD:
		{
			switch (m_rateCap)
			{
				case e_Xfer_Cap_Sm_comp: {
					BYTE seriableVal = Sm_comp | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}

				case e_Xfer_Cap_128: {
					BYTE seriableVal = Xfer_Cap_128 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_192: {
					BYTE seriableVal = Xfer_Cap_192 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_256: {
					BYTE seriableVal = Xfer_Cap_256 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_320: {
					BYTE seriableVal = Xfer_Cap_320 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_512: {
					BYTE seriableVal = Xfer_Cap_512 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_768: {
					BYTE seriableVal = Xfer_Cap_768 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_1152: {
					BYTE seriableVal = Xfer_Cap_1152 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_B: {
					BYTE seriableVal = Xfer_Cap_B | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_2B: {
					BYTE seriableVal = Xfer_Cap_2B | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_3B: {
					BYTE seriableVal = Xfer_Cap_3B | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_4B: {
					BYTE seriableVal = Xfer_Cap_4B | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_5B: {
					BYTE seriableVal = Xfer_Cap_5B | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_6B: {
					BYTE seriableVal = Xfer_Cap_6B | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_Restrict: {
					BYTE seriableVal = Xfer_Cap_Restrict | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_6B_H0_Comp: {
					BYTE seriableVal = Xfer_Cap_6B_H0_Comp | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_H0: {
					BYTE seriableVal = Xfer_Cap_H0 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_2H0: {
					BYTE seriableVal = Xfer_Cap_2H0 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_3H0: {
					BYTE seriableVal = Xfer_Cap_3H0 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_4H0: {
					BYTE seriableVal = Xfer_Cap_4H0 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_5H0: {
					BYTE seriableVal = Xfer_Cap_5H0 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_1472: {
					BYTE seriableVal = Xfer_Cap_1472 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_H11: {
					BYTE seriableVal = Xfer_Cap_H11 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_H12: {
					BYTE seriableVal = Xfer_Cap_H12 | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_Restrict_L: {
					BYTE seriableVal = Restrict_L | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_Restrict_P: {
					BYTE seriableVal = Restrict_P | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				case e_Xfer_Cap_NoRestrict: {
					BYTE seriableVal = NoRestrict | AUDRATECAPATTR;
					stringSeg << seriableVal;
					break;
				}
				default:
				{
					std::ostringstream str;
					str << "illegal value = " << m_rateCap;
					PTRACE2(eLevelInfoNormal, "CRateAlg::Serialize", str.str().c_str());
					break;
				}
			} // switch
		}
		break;
	} // switch
}

//--------------------------------------------------------------------------
void CRateAlg::DeSerialize(WORD format, CSegment& stringSeg)
{
	switch (format)
	{
		case NATIVE:
		{
			DWORD temp;
			stringSeg >> temp;
			m_rateCap = (ERateCapAlgorithm)temp;
			break;
		}
		case SERIALEMBD:
			PTRACE(eLevelInfoNormal, "CRateAlg::DeSerialize - case SERIALEMBD - do nothing");
			break;
	} // switch
}

//--------------------------------------------------------------------------
bool CRateAlg::operator<(const CRateAlg& other) const
{
	return (this->GetNumChnl() < other.GetNumChnl());
}

//--------------------------------------------------------------------------
BOOL CRateAlg::IsLegal(DWORD rate)
{
	return (((ERateCapAlgorithm)rate >= e_Xfer_Cap_Sm_comp) && (rate < e_Xfer_Cap_Last));
}

//--------------------------------------------------------------------------
const char* CRateAlg::GetAlgName() const
{
	switch (m_rateCap)
	{
		case e_Xfer_Cap_128       : return "128k";
		case e_Xfer_Cap_192       : return "192k";
		case e_Xfer_Cap_256       : return "256k";
		case e_Xfer_Cap_320       : return "320k";
		case e_Xfer_Cap_512       : return "512k";
		case e_Xfer_Cap_768       : return "768k";
		case e_Xfer_Cap_1152      : return "1152k";
		case e_Xfer_Cap_1472      : return "1472k";
		case e_Xfer_Cap_H11       : return "H11";
		case e_Xfer_Cap_H12       : return "H12";
		case e_Xfer_Cap_B         : return "B";
		case e_Xfer_Cap_2B        : return "2B";
		case e_Xfer_Cap_3B        : return "3B";
		case e_Xfer_Cap_4B        : return "4B";
		case e_Xfer_Cap_5B        : return "5B";
		case e_Xfer_Cap_6B        : return "6B";
		case e_Xfer_Cap_H0        : return "H0";
		case e_Xfer_Cap_2H0       : return "2H0";
		case e_Xfer_Cap_3H0       : return "3H0";
		case e_Xfer_Cap_4H0       : return "4H0";
		case e_Xfer_Cap_5H0       : return "5H0";
		case e_Xfer_Cap_Restrict  : return "Restrict";
		case e_Xfer_Cap_6B_H0_Comp: return "6B-H0-Comp";
		case e_Xfer_Cap_Sm_comp   : return "SM-comp";
		case e_Xfer_Cap_Restrict_L: return "Restrict_L";
		case e_Xfer_Cap_Restrict_P: return "Restrict_P";
		case e_Xfer_Cap_NoRestrict: return "NoRestrict";
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	} // switch

	return "Unknown";
}

//--------------------------------------------------------------------------
DWORD CRateAlg::GetNumChnl() const
{
	switch (m_rateCap)
	{
		case e_Xfer_Cap_128       : return 2;
		case e_Xfer_Cap_192       : return 3;
		case e_Xfer_Cap_256       : return 4;
		case e_Xfer_Cap_320       : return 5;
		case e_Xfer_Cap_512       : return 8;
		case e_Xfer_Cap_768       : return 12;
		case e_Xfer_Cap_1152      : return 18;
		case e_Xfer_Cap_1472      : return 23;
		case e_Xfer_Cap_H11       : return 24;
		case e_Xfer_Cap_H12       : return 30;
		case e_Xfer_Cap_B         : return 1;
		case e_Xfer_Cap_2B        : return 2;
		case e_Xfer_Cap_3B        : return 3;
		case e_Xfer_Cap_4B        : return 4;
		case e_Xfer_Cap_5B        : return 5;
		case e_Xfer_Cap_6B        : return 6;
		case e_Xfer_Cap_H0        : return 6;
		case e_Xfer_Cap_2H0       : return 12;
		case e_Xfer_Cap_3H0       : return 18;
		case e_Xfer_Cap_4H0       : return 24;
		case e_Xfer_Cap_5H0       : return 30;
		case e_Xfer_Cap_Restrict  : return 100;
		case e_Xfer_Cap_6B_H0_Comp: return 100;
		case e_Xfer_Cap_Sm_comp   : return 100;
		case e_Xfer_Cap_Restrict_L: return 100;
		case e_Xfer_Cap_Restrict_P: return 100;
		case e_Xfer_Cap_NoRestrict: return 100;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	} // switch

	return 0;
}


////////////////////////////////////////////////////////////////////////////
//                        CRateCap
////////////////////////////////////////////////////////////////////////////
CRateCap::CRateCap()
{
}

//--------------------------------------------------------------------------
CRateCap::CRateCap(const CRateCap& other) : CPObject(other)
{
	std::multiset<CRateAlg*, CompareByRatePriority>::iterator other_itr = other.m_rateAlgSet.begin();
	for (; other_itr != other.m_rateAlgSet.end(); ++other_itr)
	{
		CRateAlg* alg = new CRateAlg;
		*alg = *(*other_itr);
		m_rateAlgSet.insert(alg);
	}
}

//--------------------------------------------------------------------------
CRateCap::~CRateCap()
{
	ClearAndDestroy();
}

//--------------------------------------------------------------------------
void CRateCap::ClearAndDestroy()
{
	std::multiset<CRateAlg*, CompareByRatePriority>::iterator itr = m_rateAlgSet.begin();
	for ( ; itr != m_rateAlgSet.end (); ++itr) {
		CRateAlg* temp = *itr;
		POBJDELETE(temp);
	}
	m_rateAlgSet.clear();
}

//--------------------------------------------------------------------------
void CRateCap::Dump(std::ostream& msg) const
{
	msg << "\n==================    CRateCap::Dump    ==================\n";

	std::multiset<CRateAlg*, CompareByRatePriority>::iterator itr = m_rateAlgSet.begin();
	for (; itr != m_rateAlgSet.end(); ++itr)
		(*itr)->Dump(msg);
}

//--------------------------------------------------------------------------
CRateCap& CRateCap::operator=(const CRateCap& other)
{
	ClearAndDestroy();

	std::multiset<CRateAlg*, CompareByRatePriority>::iterator other_itr = other.m_rateAlgSet.begin();
	for (; other_itr != other.m_rateAlgSet.end(); ++other_itr)
	{
		CRateAlg* alg = new CRateAlg;
		*alg = *(*other_itr);
		m_rateAlgSet.insert(alg);
	}

	return *this;
}

//--------------------------------------------------------------------------
void CRateCap::Serialize(WORD format, CSegment& stringSeg)
{
	WORD size = m_rateAlgSet.size();
	if (NATIVE == format)
		stringSeg << size;

	std::multiset<CRateAlg*, CompareByRatePriority>::iterator itr = m_rateAlgSet.begin();
	for (; itr != m_rateAlgSet.end(); ++itr)
		(*itr)->Serialize(format, stringSeg);
}

//--------------------------------------------------------------------------
void CRateCap::DeSerialize(WORD format, CSegment& stringSeg)
{
	if (format != NATIVE)
		return;

	WORD size;
	stringSeg >> size;
	for (WORD i = 0; i < size && !stringSeg.EndOfSegment(); i++)
	{
		DWORD temp;
		stringSeg >> temp;
		this->AddXferCap(temp);
	}
}

//--------------------------------------------------------------------------
void CRateCap::AddXferCap(WORD cap)
{
	if (CRateAlg::IsLegal(cap) && !IsXferCapSupported(cap))
	{
		CRateAlg* alg = new CRateAlg((ERateCapAlgorithm)cap);
		m_rateAlgSet.insert(alg);
	}
}

//--------------------------------------------------------------------------
void CRateCap::ResetXferCap()
{
	ClearAndDestroy();
}

//--------------------------------------------------------------------------
BOOL CRateCap::IsXferCapSupported(WORD cap) const
{
	std::multiset<CRateAlg*, CompareByRatePriority>::iterator itr = m_rateAlgSet.begin();
	for (; itr != m_rateAlgSet.end(); ++itr)
		if ((*itr)->GetRateAlg() == cap)
			return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCapB()
{
	AddXferCap(e_Xfer_Cap_B);
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCap2B()
{
	AddXferCap(e_Xfer_Cap_2B);
	SetXferCapB();
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCap3B()
{
	AddXferCap(e_Xfer_Cap_3B);
	SetXferCap2B();
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCap4B()
{
	AddXferCap(e_Xfer_Cap_4B);
	SetXferCap3B();
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCap5B()
{
	AddXferCap(e_Xfer_Cap_5B);
	SetXferCap4B();
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCap6B()
{
	AddXferCap(e_Xfer_Cap_6B);
	SetXferCap5B();
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCapH0()
{
	AddXferCap(e_Xfer_Cap_H0);
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCap2H0()
{
	AddXferCap(e_Xfer_Cap_2H0);
	SetXferCapH0();
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCap3H0()
{
	AddXferCap(e_Xfer_Cap_3H0);
	SetXferCap2H0();
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCap4H0()
{
	AddXferCap(e_Xfer_Cap_4H0);
	SetXferCap3H0();
}

//--------------------------------------------------------------------------
void CRateCap::SetXferCap5H0()
{
	AddXferCap(e_Xfer_Cap_5H0);
	SetXferCap4H0();
}


////////////////////////////////////////////////////////////////////////////
//                        CEncrypAlg
////////////////////////////////////////////////////////////////////////////
CEncrypAlg::CEncrypAlg()
{
	m_alg    = (0x0);
	m_params = (0x0);
	m_media  = (0x0);
}

//--------------------------------------------------------------------------
void CEncrypAlg::Create(BYTE params, BYTE alg, BYTE media)
{
	m_alg    = alg;
	m_params = params;
	m_media  = media;
}

//--------------------------------------------------------------------------
CEncrypAlg::CEncrypAlg(CEncrypAlg& other) : CPObject(other)
{
	m_alg    = other.m_alg;
	m_params = other.m_params;
	m_media  = other.m_media;
}

//--------------------------------------------------------------------------
CEncrypAlg& CEncrypAlg::operator=(const CEncrypAlg& other)
{
	if (&other == this)
		return *this;

	m_alg    = other.m_alg;
	m_params = other.m_params;
	m_media  = other.m_media;
	return *this;
}

//--------------------------------------------------------------------------
CEncrypAlg::~CEncrypAlg()
{
}

//--------------------------------------------------------------------------
void CEncrypAlg::Serialize(WORD format, CSegment& seg)
{
	switch (format)
	{
		case SERIALEMBD:
		{
			seg << (BYTE)m_media;
			seg << (BYTE)m_alg;
			seg << (BYTE)m_params;
			break;
		}
		case NATIVE:
		{
			seg << (BYTE)m_alg;
			seg << (BYTE)m_params;
			seg << (BYTE)m_media;

			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
void CEncrypAlg::DeSerialize(WORD format, CSegment& seg)
{
	switch (format)
	{
		case SERIALEMBD:
		{
			seg >> m_media;
			seg >> m_alg;
			seg >> m_params;
			break;
		}
		case NATIVE:
		{
			seg >> m_alg;
			seg >> m_params;
			seg >> m_media;
			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
BYTE CEncrypAlg::operator==(const CEncrypAlg& other)
{
	return ((m_alg == other.m_alg) && (m_params == other.m_params) && (m_media == other.m_media));
}


////////////////////////////////////////////////////////////////////////////
//                        CCapECS
////////////////////////////////////////////////////////////////////////////
CCapECS::CCapECS()          // constructor
{
	m_pEncrypAlg = NULL;
	m_keyExchCap = 0;
	m_numAlgo    = 0;
}

//--------------------------------------------------------------------------
CCapECS::CCapECS(const CCapECS& other) : CPObject(other)
{
	m_keyExchCap = other.m_keyExchCap;
	m_numAlgo    = other.m_numAlgo;
	if (m_pEncrypAlg)
		PDELETEA(m_pEncrypAlg);

	if (other.m_pEncrypAlg)
	{
		m_pEncrypAlg = new CEncrypAlg[m_numAlgo];
		for (int i = 0; i < m_numAlgo; i++)
			m_pEncrypAlg[i] = other.m_pEncrypAlg[i];
	}
	else
		m_pEncrypAlg = NULL;
}

//--------------------------------------------------------------------------
CCapECS::~CCapECS()
{
	if (CPObject::IsValidPObjectPtr(m_pEncrypAlg))
		PDELETEA(m_pEncrypAlg);

	m_keyExchCap = 0;
	m_numAlgo    = 0;
}

//--------------------------------------------------------------------------
void CCapECS::CreateLocalCapECS()
{
	m_numAlgo = NUM_OF_SUPPORTED_ENCRYPTION_ALG;
	if (m_pEncrypAlg)
		PDELETEA(m_pEncrypAlg);                   // bug 21946 memory leak

	m_pEncrypAlg = new CEncrypAlg[m_numAlgo];
	m_pEncrypAlg[0].SetAlg(AES_128_IDENTIFIER); // AES 128
	m_pEncrypAlg[0].SetParams(AES_PARAM);
	m_pEncrypAlg[0].SetMedia(AES_MEDIA);        // Only for local Cap

	m_keyExchCap = DIFFIE_HELMAN_MASK;          // DH Only
}

void CCapECS::SetKeyExchCap(BYTE newKeyExch)
{
	m_keyExchCap = newKeyExch;
}

BYTE CCapECS::GetKeyExchCap()
{
	return m_keyExchCap;
}

CCapECS& CCapECS::operator=(CCapECS& other)
{
	if (this == &other)
		return *this;

	m_keyExchCap = other.m_keyExchCap;
	m_numAlgo    = other.m_numAlgo;
	if (m_pEncrypAlg)
		PDELETEA(m_pEncrypAlg);

	if (other.m_pEncrypAlg)
	{
		m_pEncrypAlg = new CEncrypAlg[m_numAlgo];
		for (int i = 0; i < m_numAlgo; i++)
			m_pEncrypAlg[i] = other.m_pEncrypAlg[i];
	}
	else
		m_pEncrypAlg = NIL(CEncrypAlg);

	return *this;
}

void CCapECS::Serialize(WORD format, CSegment& seg)
{
	switch (format)
	{
		case SERIALEMBD: {
			if (m_numAlgo > 0)
			{
				seg << (BYTE)P8_Identifier; // P8
				seg << (BYTE)(ENCRYPTION_ALG_LENGTH_IN_BYTES*m_numAlgo);
				for (int i = 0; i < m_numAlgo; i++)
					m_pEncrypAlg[i].Serialize(format, seg);
			}

			seg << (BYTE)P0_Identifier;     // P0 Identifier
			seg << (BYTE)P0_CONTENT_LENGTH; // length
			seg << (BYTE)m_keyExchCap;      // content
		} break;
		case NATIVE:
		{
			seg << m_keyExchCap << m_numAlgo;
			for (int i = 0; i < m_numAlgo; i++)
			{
				m_pEncrypAlg[i].Serialize(format, seg);
			}
		} break;
	} // switch
}

void CCapECS::SerializeP0(CSegment& seg)
{
	seg << (BYTE)P0_Identifier;      // P0 Identifier
	seg << (BYTE)P0_CONTENT_LENGTH;  // length
	seg << (BYTE)m_keyExchCap;       // content
}

// /////////////////////////////////////////////////////////////////////////////
void CCapECS::DeSerialize(WORD format, CSegment& seg)
{
	ALLOCBUFFER(msg, ONE_LINE_BUFFER_LEN);

	switch (format)
	{
		case SERIALEMBD: {
			BYTE opcode = 0;
			BYTE length = 0;
			seg >> opcode;
			switch (opcode)
			{
				case (P0_Identifier):  {
					strcat(msg, " ECS - P0");
					seg >> length;
					if (length != P0_CONTENT_LENGTH) // length of P0 - includes 1 BYTE
						PASSERT(101);

					seg >> m_keyExchCap;
					break;
				}
				case (P8_Identifier):  {
					strcat(msg, " ECS - P8");
					seg >> length;
					if (length%ENCRYPTION_ALG_LENGTH_IN_BYTES) // length of P8 is always 3*N
						PASSERT(101);

					BYTE num_of_alg = 0;
					num_of_alg = length/ENCRYPTION_ALG_LENGTH_IN_BYTES;
					if (m_pEncrypAlg)
						PDELETEA(m_pEncrypAlg);

					m_pEncrypAlg = new CEncrypAlg[num_of_alg];

					for (int i = 0; i < num_of_alg; i++)
						m_pEncrypAlg[i].DeSerialize(format, seg);

					break;
				}

				default: {
					break;
				}
			} // switch

			break;
		}

		case NATIVE: {
			if (CPObject::IsValidPObjectPtr(m_pEncrypAlg))
				PDELETEA(m_pEncrypAlg);

			seg >> m_keyExchCap >> m_numAlgo;

			m_pEncrypAlg = new CEncrypAlg[m_numAlgo];

			for (int i = 0; i < m_numAlgo; i++)
			{
				m_pEncrypAlg[i].DeSerialize(format, seg);
			}

			break;
		}
		default:
		break;
	} // switch

	PTRACE2(eLevelInfoNormal, "CCapECS::DeSerialize : Name - ", msg);
	DEALLOCBUFFER(msg);
}
// ///////////////////////////////////////////////////////////////////////////
BYTE CCapECS::KeyExchCapIncludesDH()
{
	if ((GetKeyExchCap()&DIFFIE_HELMAN_MASK) != 0) // DH
		return TRUE;
	else
		return FALSE;
}
// ///////////////////////////////////////////////////////////////////////////
BYTE CCapECS::EncrypAlgCapIncludesAES128()
{
	if (m_pEncrypAlg)
	{
		for (int i = 0; i < m_numAlgo; i++)
		{
			if (m_pEncrypAlg[i].GetAlg() == AES_128_IDENTIFIER)
				return TRUE;
		}

		return FALSE;
	}

	return FALSE;
}
// ///////////////////////////////////////////////////////////////////////////
void CCapECS::SaveECSP0(CSegment* pParam)
{
	BYTE length = 0;
	BYTE info;

	*pParam >> length;
	if (length != P0_CONTENT_LENGTH) // length of P0 should always be - 1 BYTE
	{
		CSmallString str;
		str <<"CCapECS::SaveECSP0 :  Illegal Length !! "<< length;
		PTRACE(eLevelError, str.GetString());

		for (int i = 0; i < length; i++)
			*pParam >> info;

		return;
	}

	*pParam >> info;

	if ((info&INVALID_P0_MASK) != 0)  // checking P0 legal
	{
		if (info)
		{
			PASSERT_AND_RETURN(info);
		}
		else
		{
			PASSERT_AND_RETURN(101);
		}
	}

	m_keyExchCap = info;
}
// ///////////////////////////////////////////////////////////////////////////
void CCapECS::SaveECSP8(CSegment* pParam)
{
	BYTE length = 0;
	BYTE alg    = 0;
	BYTE params = 0;
	BYTE media  = 0;

	*pParam >> length;

	if (length%ENCRYPTION_ALG_LENGTH_IN_BYTES != 0)  // P8 will always be length 3*N
	{
		CSmallString str;
		str <<"CCapECS::SaveECSP8 :  Illegal Length !! "<< length;
		PTRACE(eLevelError, str.GetString());

		for (int i = 0; i < length; i++)
			*pParam >> params;

		return;
	}

	if (CPObject::IsValidPObjectPtr(m_pEncrypAlg))
		PDELETEA(m_pEncrypAlg);

	m_numAlgo    = length/ENCRYPTION_ALG_LENGTH_IN_BYTES;
	m_pEncrypAlg = new CEncrypAlg[m_numAlgo];

	for (int i = 0; i < m_numAlgo; i++)
	{
		*pParam >> media;
		*pParam >> alg;
		*pParam >> params;

		m_pEncrypAlg[i].Create(params, alg, media);
	}
}
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// void  CCapH320::CreateSipOptions(CCommConf* pCommConf)
// {
// CComMode commMode;
// // sudio only conf
// if(pCommConf->IsAudioConf()){
// commMode.CreateSipOptions(pCommConf,H261);
// Create(commMode);
// return;
// }

// // set free bit rate
// BYTE reserved_video_protocol = pCommConf->GetVideoProtocol(); // H261, H263, AUTO
// switch(reserved_video_protocol) {
// case H261:{
// commMode.CreateSipOptions(pCommConf,H261);
// Create(commMode);
// break;
// }
// case H263:{
// commMode.CreateSipOptions(pCommConf,H263);
// Create(commMode);
// break;
// }
// case H264:
// {
// if (::GetpSystemCfg()->IsH264())
// {
// commMode.CreateSipOptions(pCommConf,H264);
// }else{
// PTRACE(EXCEPTION_TRACE,"CCapH320::CreateSipOptions : H264 is disabled , vid mode set to H261");
// commMode.CreateSipOptions(pCommConf,H261);
// }
// Create(commMode);
// break;
// }
// case AUTO:  {
// commMode.CreateSipOptions(pCommConf,H261);
// Create(commMode);
// commMode.CreateSipOptions(pCommConf,H263);
// Create(commMode);
// if (::GetpSystemCfg()->IsH264())
// {
// commMode.CreateSipOptions(pCommConf,H264);
// Create(commMode,NULL,NULL,YES);
// }
// break;
// }
// default:{
// PTRACE(EXCEPTION_TRACE,"CCapH320::CreateSipOptions value is not valid!!! vid mode set to H261");
// //set max video scm (cif/qcif 30)
// commMode.CreateSipOptions(pCommConf,H261);
// DBGPASSERT(reserved_video_protocol);
// break;
// }
// }
// }

// void CCapH320::LegalizeH264()
// {
// m_h264cap.Legalize();

// UpdateH221string();

// }


// ///////////////////////////////////////////////////////////////////////////
// ////////////////////////////// CEncrypCap /////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////

CEncrypCap::CEncrypCap()
{
	m_pCapECS   = NULL;
	m_encrypCap = 0;
}

CEncrypCap::CEncrypCap(const CEncrypCap& other) : CPObject(other)
{
	if (other.m_pCapECS)
	{
		m_pCapECS  = new CCapECS;
		*m_pCapECS = *(other.m_pCapECS);
	}
	else
		m_pCapECS = NULL;
}

CEncrypCap::~CEncrypCap()
{
	POBJDELETE(m_pCapECS);
}

const char* CEncrypCap::NameOf() const
{
	return "CCapECS";
}

CEncrypCap& CEncrypCap::operator=(const CEncrypCap& other)
{
	if (m_pCapECS)
		POBJDELETE(m_pCapECS);

	if (other.m_pCapECS)
	{
		m_pCapECS  = new CCapECS;
		*m_pCapECS = *(other.m_pCapECS);
	}
	else
		m_pCapECS = NIL(CCapECS);

	m_encrypCap = other.m_encrypCap;

	return *this;
}

void CEncrypCap::CreateDefault()
{
	m_encrypCap = Encryp_Cap;
}

void CEncrypCap::CreateCCapECS()
{
	if (m_pCapECS)
		POBJDELETE(m_pCapECS);

	m_pCapECS = new CCapECS;
}

void CEncrypCap::CreateCCapECS(const CCapECS* pCapECS)
{
	if (m_pCapECS)
		POBJDELETE(m_pCapECS);

	m_pCapECS = new CCapECS(*pCapECS);
}

CCapECS* CEncrypCap::GetCapECS() const
{
	return m_pCapECS;
}

void CEncrypCap::CreateLocalCapECS()
{
	if (m_pCapECS)
		POBJDELETE(m_pCapECS);

	m_pCapECS = new CCapECS;
	m_pCapECS->CreateLocalCapECS();
}

void CEncrypCap::Dump(std::ostream& msg) const
{
	msg << "\n==================    CEncrypCap::Dump    ==================\n";
	msg << std::setw(20) << (Encryp_Cap == m_encrypCap ? "Encryp_Cap is set" : "") << "\n";
}

void CEncrypCap::Serialize(WORD format, CSegment& stringSeg)
{
	switch (format)
	{
		case NATIVE:
		{
			if (m_pCapECS)
			{
				stringSeg << (BYTE)1; // is Encryption Caps
				m_pCapECS->Serialize(format, stringSeg);
			}
			else
				stringSeg << (BYTE)0; // is Encryption Caps

			// isdn_encryption
			stringSeg << m_encrypCap;
		} break;
		case SERIALEMBD:
		if (m_encrypCap)
		{
			BYTE seriableVal = Encryp_Cap | DATAVIDCAPATTR;
			stringSeg << seriableVal;
		}

		break;
	} // switch
}

void CEncrypCap::DeSerialize(WORD format, CSegment& stringSeg)
{
	switch (format)
	{
		case NATIVE:
		{
			BYTE isECSCap;
			stringSeg >> isECSCap;
			if (isECSCap)
			{
				if (!m_pCapECS)
					m_pCapECS = new CCapECS;

				m_pCapECS->DeSerialize(format, stringSeg);
			}

			// isdn_encryption
			stringSeg >> m_encrypCap;
		} break;
		case SERIALEMBD:
		PTRACE(eLevelInfoNormal, "CEncrypCap::DeSerialize - case SERIALEMBD - do nothing");
		break;
	} // switch
}

void CEncrypCap::SetEncrypCap(BYTE cap)
{
	m_encrypCap = cap;
}

void CEncrypCap::RemoveEncrypCap()
{
	m_encrypCap = 0;
	if (m_pCapECS)
		POBJDELETE(m_pCapECS);
}
