#ifndef _H320CAPS_H
#define _H320CAPS_H

#include "H320AudioCaps.h"
#include "H320VideoCaps.h"
#include "H320MediaCaps.h"
#include "H320CapPP.h"
#include "H320CapNS.h"
#include "H320ComMode.h"
#include "H221StrCap.h"
#include "NonStandardCaps.h"

////////////////////////////////////////////////////////////////////////////
//                        CCapH320
////////////////////////////////////////////////////////////////////////////
class CCapH320 : public CPObject
{
	CLASS_TYPE_1(CCapH320, CPObject)

public:
	                CCapH320();
	                CCapH320(const CCapH320& other);
	virtual        ~CCapH320();

	void            CreateDefault();
	void            Create(CComMode& scm, CCapH263* pCapH263 = NULL, CCapH264* pCapH264 = NULL, BYTE H263withH264 = NO);

	const char*     NameOf() const                              { return "CCapH320"; }
	virtual void    Dump(std::ostream&) const;

	void            Dump() const;

	void            Serialize(WORD format, CSegment& H221StringSeg);
	void            DeSerialize(WORD format, CSegment& H221StringSeg);

	void            HandleBas(BYTE bas, CSegment& seg);
	CCapH320&       operator=(CCapH320& other);

	// audio operations
	void            SetAudioCap(EAudioCapAlgorithm cap)         { m_audioCap.AddAudioAlg(cap); }
	void            SetVoiceOnlyCap(WORD audio_cap);
	WORD            OnAudioCap(EAudioCapAlgorithm cap) const    { return m_audioCap.IsAudioAlgSupported(cap); }
	void            RemoveAudioCap(EAudioCapAlgorithm cap)      { m_audioCap.RemoveAudioAlg(cap); }
	void            SetAudioCapSet(CCapH320& other)             { m_audioCap = other.m_audioCap; }

	// transfer operations
	void            SetXferCap(ERateCapAlgorithm cap)           { m_rateCap.AddXferCap(cap); }
	void            ResetXferCap()                              { m_rateCap.ResetXferCap(); } // reset the bits of xfer-rate cap only (not audio cap)
	WORD            OnXferCap(ERateCapAlgorithm cap) const      { return m_rateCap.IsXferCapSupported(cap); }
	void            SetXferRateFromScm(const CComMode& scm);

	// video operations
	void            MbeCommandParser(CSegment& seg);
	BOOL            IsMBECapOn(BYTE cap) const                  { return m_videoCap.IsMBECap(cap); }
	BOOL            IsVideoCapSupported(WORD cap) const         { return m_videoCap.IsVideoCapSupported(cap); }
	void            SetVideoCapSet(CCapH320& other)             { m_videoCap = other.m_videoCap; }

	// H261
	void            SetH261Caps(WORD cap, WORD mpiCif, WORD mpiQcif); // values are V_Cif,VQcif,V_1_29_97,V_1_29_97
	WORD            GetH261CapMpi(WORD cap) const               { return m_videoCap.GetH261CapMpi(cap); }
	BOOL            IsH261VideoCap(WORD cap) const              { return m_videoCap.IsH261VideoCap(cap); }
	void            RemoveH261Caps()                            { m_videoCap.RemoveH261Caps(); }

	// H263
	WORD            IsH263() const                              { return m_videoCap.IsH263(); }
	CCapH263*       GetCapH263()                                { return m_videoCap.GetCapH263(); }
	void            SetH263Caps(CSegment& seg, BYTE len)        { m_videoCap.CreateH263Cap(seg, len); }
	void            RemoveH263Caps()                            { m_videoCap.RemoveH263Caps(); }
	void            SendSecondAdditionalCap(WORD OnOff)         { m_videoCap.SendSecondAdditionalCap(OnOff); }

	// H264
	WORD            IsH264() const                              { return m_videoCap.IsH264(); }
	CCapH264*       GetCapH264()                                { return m_videoCap.GetCapH264(); }
	void            SetH264Caps(CSegment& seg, DWORD len)       { m_videoCap.CreateH264Cap(seg, len); }
	void            AddH264CapSet(CCapSetH264* pH264CapSetBuf)  { m_videoCap.InsertH264CapSet(pH264CapSetBuf); }
	void            RemoveH264Caps()                            { m_videoCap.RemoveH264Caps(); }

	// data operation
	void            SetMlpCap(WORD cap);
	WORD            IsMlpCap(WORD cap) const                    { return m_dataCap.OnMlpCap(cap); }
	void            SetDataCap(WORD cap)                        { return m_dataCap.SetDataCap(cap); }
	void            RemoveDataCap(WORD cap)                     { return m_dataCap.RemoveDataCap(cap); }
	WORD            IsDataCap(WORD cap) const                   { return m_dataCap.OnDataCap(cap); }
	void            SetHsdHmlpCap(const CCapH320& cap)          { m_dataCap.SetHsdHmlpCap(cap.GetHsdHmlpBas()); }
	void            SetMlpCap(const CCapH320& cap)              { m_dataCap.SetMlpCap(cap.GetMlpBas()); }
	DWORD           GetHsdHmlpBas() const                       { return m_dataCap.GetHsdHmlpBas(); }
	DWORD           GetMlpBas() const                           { return m_dataCap.GetMlpBas(); }
	WORD            IsHsdHmlpCap(WORD cap) const                { return m_dataCap.OnHsdHmlpCap(cap); }
	void            SetLsdCap(WORD lsdMode);
	void            SetT120Cap(WORD T120Val);

	// content operation (H239)
	void            SetH239ExtendedVideoCaps(CSegment& seg, BYTE len);
	void            SetOnlyExtendedVideoCaps(const CCapH239* pCapH239);
	void            SetAMCCaps(CSegment& seg, BYTE len)         { m_contentCap.SetAMCCaps(seg, len); }
	void            RemoveH239Caps();
	const CCapH239* GetH239Caps()                               { return m_contentCap.GetH239Caps(); }
	BYTE            IsH239Cap(void) const;
	void            SetH239ControlCap(WORD IsH239ControlCap)    { m_h239ControlCap = IsH239ControlCap; }

	// encryption operations
	void            CreateLocalCapECS()                         { m_encrypCap.CreateLocalCapECS(); }
	void            CreateCCapECS()                             { m_encrypCap.CreateCCapECS(); }
	void            CreateCCapECS(const CCapECS* pCapECS)       { m_encrypCap.CreateCCapECS(pCapECS); }
	void            RemoveEncCaps()                             { m_encrypCap.RemoveEncrypCap(); }
	BOOL            IsEncrypCapOn(BYTE cap)                     { return m_encrypCap.IsEncrypCapOn(cap); }
	CCapECS*        GetCapECS() const                           { return m_encrypCap.GetCapECS(); }

	// CIC caps
	void            SetChairCap(WORD chair)                     { m_chair = chair; }
	void            SetMIHCap(WORD mih)                         { m_mih = mih; }
	WORD            isMIH() const                               { return m_mih; }

	// MVC caps
	void            SetMVCCap(WORD mvc)                         { m_mvc = (mvc) ? 1 : 0; }
	WORD            isMVC() const                               { return m_mvc; }

	// H221 string operations
	void            UpdateH221string(CH221strCapDrv& str);
	BYTE*           GetH221str(int& len); // use serialize / deserialize, returns string & length

	// NonStandard
	void            RemoveNSCap();
	void            HandleNSCap(CSegment& seg);
	void            SetNSCap(const CCapNS& nscap)               { m_nsCap = nscap; }
	const CCapNS*   GetNSCap() const                            { return &m_nsCap; }
	WORD            IsPeopleContent() const                     { return m_nsCap.OnPeopleContent(); }
	void            SetNScapSiren14()                           { m_nsCap.AddNScapSiren14(); }
	void            SetNScapSiren1424()                         { m_nsCap.AddNScapSiren1424(); }
	void            SetNScapSiren1432()                         { m_nsCap.AddNScapSiren1432(); }
	void            SetNScapSiren1448()                         { m_nsCap.AddNScapSiren1448(); }

	// p+c
	void            AddPeopleContent()                          { if (!IsPeopleContent()) m_nsCap.AddNScapPeopleContent();}
	void            RemovePeopleContents()                      { if (IsPeopleContent()) m_nsCap.RemovePeopleContent();}

	void            CreatePPCap(const CCapPP* pCapPP);
	void            HandlePPCap(CSegment& seg);
	BOOL            IsPPCap() const                             { return (m_pCapPP ? TRUE : FALSE); }
	const CCapPP*   GetPPCap() const                            { return m_pCapPP;}

	// old p+c implementation
	WORD            IsVisualConcertPC() const                   { return m_nsCap.OnVisualConcertPC(); }
	void            AddVisualConcertPC()                        { if (!IsVisualConcertPC()) m_nsCap.AddNScapVisualConcertPC();}
	void            RemoveVisualConcertPC()                     { if (IsVisualConcertPC()) m_nsCap.RemoveVisualConcertPC();}

	WORD            IsVisualConcertFX() const                   { return m_nsCap.OnVisualConcertFX(); }
	void            AddVisualConcertFX()                        { if (!IsVisualConcertFX()) m_nsCap.AddNScapVisualConcertFX();}
	void            RemoveVisualConcertFX()                     { if (IsVisualConcertFX()) m_nsCap.RemoveVisualConcertFX();}
	WORD            IsAbleJoinFXconf()                          { return m_nsCap.IsAbleJoinFXconf(); }
	void            AddNSH26LVideoCap(BYTE Octet0, BYTE Octet1) { m_nsCap.AddNSH26LVideoCap(Octet0, Octet1);}

	// H26L
	WORD            IsH26L() const                              { return m_nsCap.OnH26L(); }
	void            SetH26L(BYTE Octet0, BYTE Octet1)           { m_nsCap.AddNSH26LVideoCap(Octet0, Octet1); }
	WORD            IsH26LMpi(WORD h320FormatType)  const;
	WORD            GetH26LMpi(WORD h320FormatType)  const;
	WORD            GetH26LCifMpiForOneVideoStream()  const     { return m_nsCap.GetH26LCifMpiForOneVideoStream(); }
	WORD            GetH26L4CifMpiForOneVideoStream() const     { return m_nsCap.GetH26L4CifMpiForOneVideoStream(); }

	// video H26L only?
	BYTE            IsFieldDrop() const                         { return m_nsCap.OnFieldDrop(); }
	BYTE            GetFieldDropValue() const                   { return NS_FIELD_DROP; }

	// video HD
	BYTE            IsCapableOfHD720_15() const                 { return m_videoCap.IsCapableOfHD720_15(); }
	BYTE            IsCapableOfHD1080_15() const                { return m_videoCap.IsCapableOfHD1080_15(); }
	BYTE            IsCapableOfHD720_50() const                 { return m_videoCap.IsCapableOfHD720_50(); }

	BYTE            IsCapableOfVideo() const                    { return m_videoCap.IsCapableOfVideo(); }
	eVideoPartyType GetCPVideoPartyTypeAccordingToCapabilities(BYTE isH2634Cif15Supported);
	eVideoPartyType GetCPVideoPartyTypeAccordingToLocalCapabilities(BYTE isH2634Cif15Supported);

	// Testing
	BOOL            IsReducedcapbilities();

protected:
	CAudioCap       m_audioCap;       // audio capabilities
	CRateCap        m_rateCap;        // bandwidth capabilities
	CDataCap        m_dataCap;        // hsd + mlp capabilities
	CEncrypCap      m_encrypCap;
	CContentCap     m_contentCap;     // h239 Capabilities (standard P&C)
	CVideoCap       m_videoCap;       // data capabilities + video capabilities
	CCapNS          m_nsCap;          // Non-Standard capabilities container
	CCapPP*         m_pCapPP;         // Public Polycom capabilities

private:
	WORD            m_h239ControlCap; // h239 Control Cap
	WORD            m_chair;          // chair cntl cap
	WORD            m_tic;            // tic cap
	WORD            m_mih;            // MIH cap
	WORD            m_mvc;            // MVC (Multipoint Visualization Capability)
	WORD            m_illegalCap;
};

#endif /* _H320CAPS  */

