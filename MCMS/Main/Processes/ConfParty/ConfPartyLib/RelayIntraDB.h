#ifndef INTRADB_H_
#define INTRADB_H_

#include "DataTypes.h"
#include <map>
#include <list>


class AskForRelayIntra;
class CVideoBridge;

class CRelayIntraDB
{
public:
	CRelayIntraDB();
	virtual ~CRelayIntraDB();
	void SetVideoBridge(CVideoBridge* pVideoBridge) { m_pVideoBridge = pVideoBridge; }
	void AddParty(DWORD iPartyID, const std::list<unsigned int>& listSrc);
	void RemoveParty(DWORD iPartyID);
	bool SetNeedIntra(DWORD iPartyID);
	bool SetNeedIntra(DWORD iPartyID, unsigned int iSrc);
	void SetNeedIntra(const AskForRelayIntra& req);
	void Flush();
	void Dump(std::ostream& ost) const;

protected:
	void AddPartyNeedIntra(DWORD iPartyID, const std::list<unsigned int>& listSrc,bool isGRR);

	struct INTRA_SRC
	{
		unsigned int m_iSrc;
		bool m_bNeedIntra;
		bool m_bGDR;

		INTRA_SRC(unsigned int iSrc)
		{
			m_iSrc = iSrc;
			m_bNeedIntra = false;
			m_bGDR = false;
		}
		INTRA_SRC(int iSrc, bool bNeedIntra)
		{
			m_iSrc = iSrc;
			m_bNeedIntra = bNeedIntra;
			m_bGDR = false;
		}
		INTRA_SRC(int iSrc, bool bNeedIntra, bool bIsGDR)
		{
			m_iSrc = iSrc;
			m_bNeedIntra = bNeedIntra;
			m_bGDR = bIsGDR;
		}
	};

	std::map<int, std::list<INTRA_SRC> > m_mapPartyIntra;
	CVideoBridge* m_pVideoBridge;
};

#endif /* INTRADB_H_ */
