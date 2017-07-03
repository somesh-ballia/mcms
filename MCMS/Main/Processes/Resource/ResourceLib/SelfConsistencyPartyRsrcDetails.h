#ifndef SELFCONSISTENCYPARTYRSRCDETAILS_H_
#define SELFCONSISTENCYPARTYRSRCDETAILS_H_


#include "MplMcmsStructs.h"
#include "SharedDefines.h"
#include "PObject.h"
#include "NStream.h"
#include "AllocateStructs.h"

class CPartyRsrc;
class CRsrcDesc;
class CActivePort;
class CUdpRsrcDesc;
class CSelfConsistencyWhatToCheck;
class CBoard;

////////////////////////////////////////////////////////////////////////////
//                        CSelfConsistencyPartyRsrcDetails
////////////////////////////////////////////////////////////////////////////
class CSelfConsistencyPartyRsrcDetails : public CPObject
{
	CLASS_TYPE_1(CSelfConsistencyPartyRsrcDetails, CPObject)

public:
	CSelfConsistencyPartyRsrcDetails(CPartyRsrc* pPartyRsrc, DWORD rsrcConfId);
	virtual ~CSelfConsistencyPartyRsrcDetails();
	virtual const char* NameOf(void) const;

	void                AddResourceDescriptor(CRsrcDesc* pRsrcdesc);
	void                AddActivePort(CActivePort* pActivePort, CBoard* pBoard);
	void                AddUDPDescriptor(CUdpRsrcDesc* pUdpRsrcDesc);

	STATUS              CheckConsistency(CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck);

	eVideoPartyType     GetPartyType();
	ePartyRole          GetPartyRole();

	COstrStream&        GetOStreamAndPrintParty();

private:
	CPartyRsrc*         m_pPartyRsrc;
	CUdpRsrcDesc*       m_pUdpRsrcDesc;
	DWORD               m_RsrcConfId;
	WORD                m_num_of_art_descriptors;
	WORD                m_num_of_video_descriptors;
	WORD                m_num_of_net_descriptors;
	float               m_art_capacity_in_promilles;
	DWORD               m_weighted_art; // NGB, for MPM-Rx, the ART promilles are determined by weighted art.
	float               m_video_capacity_in_promilles;
	WORD                m_num_of_art_ports;
	WORD                m_num_of_video_ports;
	CBoard*             m_pArt_board;
	CBoard*             m_pVideo_board;


	STATUS              CheckUDPDescriptor();
	STATUS              CheckVideoCapacityAndPorts();
	STATUS              CheckArtCapacityAndPorts();
	STATUS              CheckNumVideoDescriptors();
	STATUS              CheckNumArtDescriptors();
	STATUS              CheckNumNetDescriptors();

	void                PrintWrongUDP();
};

#endif
