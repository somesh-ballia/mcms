#if !defined(_PartyPreviewDrv_H__)
#define _PartyPreviewDrv_H__

#include "SerializeObject.h"
#include "IpChannelParams.h"
#include "PObject.h"
#include "AllocateStructs.h"

////////////////////////////////////////////////////////////////////////////
//                        CPartyPreviewDrv
////////////////////////////////////////////////////////////////////////////
class CPartyPreviewDrv : public CSerializeObject, protected START_PREVIEW_IND_PARAMS_S
{
	CLASS_TYPE_1(PartyPreviewDrv, CSerializeObject)

public:
	                  CPartyPreviewDrv();
	                  CPartyPreviewDrv(CPartyPreviewDrv& other);
	                  CPartyPreviewDrv& operator =(const CPartyPreviewDrv& other);
	virtual          ~CPartyPreviewDrv();

	const char*       NameOf() const {return "CPartyPreviewDrv";}

	void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int               DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);

	void              Serialize(WORD format, std::ostream& m_ostr) const;
	void              DeSerialize(WORD format, std::istream& m_istr);

	int               convertStrActionToNumber(const char* strAction);

	CSerializeObject* Clone()                      { return new CPartyPreviewDrv(); }

	DWORD             GetConfID() const            { return monitor_conf_id; }
	DWORD             GetPartyID() const           { return monitor_party_id; }
	WORD              GetDirection() const         { return m_Direction; }
	DWORD             GetRemoteIP() const          { return m_RemoteIPAddress; }
	WORD              GetVideoPort() const         { return m_VideoPort; }
	WORD              GetAudioPort() const         { return m_AudioPort; }

	void              SetConfID(DWORD ConfId)      { monitor_conf_id = ConfId; }
	void              SetPartyID(DWORD PartyId)    { monitor_party_id = PartyId; }
	void              SetDirection(WORD direction) { m_Direction = direction; }
	void              SetRemoteIP(DWORD RmtIP)     { m_RemoteIPAddress = RmtIP; }
	void              SetVideoPort(WORD vidPort)   { m_VideoPort = vidPort; }
	void              SetAudioPort(WORD AudPort)   { m_AudioPort = AudPort; }
};

#endif // !defined(_PartyPreviewDrv_H__)
