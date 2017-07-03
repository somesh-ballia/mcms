#ifndef SCPPIPEMAPPINGNOTIFICATION_H_
#define SCPPIPEMAPPINGNOTIFICATION_H_

#include <list>
#include "Segment.h"
#include "VideoRelaySourcesParams.h"



class CScpPipeMappingObj : public CPObject {
CLASS_TYPE_1(CScpPipeMappingObj,CPObject)
public:
	CScpPipeMappingObj();
	CScpPipeMappingObj(const CScpPipeMappingObj &other);
	CScpPipeMappingObj& operator= (const CScpPipeMappingObj &other);
	bool operator==(const CScpPipeMappingObj &other) const;
	void InitDefaults();

	virtual void  Serialize(WORD format,CSegment& seg);
	virtual void  DeSerialize(WORD format,CSegment& seg);
	virtual const char* NameOf() const { return "CScpPipeMappingObj"; }

	APIU32 GetPipeId() {return m_pipeId;}
	APIU32 GetCsrc() { return m_csrc;}

	void   SetPipeId(DWORD pipeId){ m_pipeId = pipeId;}
	void   SetCsrc(DWORD csrc){ m_csrc = csrc;}

protected:
	APIU32		m_pipeId;
	APIU32		m_csrc;
};


class CScpPipeMappingNotification: public CPObject
{
CLASS_TYPE_1(CScpPipeMappingNotification,CPObject)
public:
	CScpPipeMappingNotification();
	CScpPipeMappingNotification(const CScpPipeMappingNotification &other);
	CScpPipeMappingNotification& operator= (const CScpPipeMappingNotification &other);
	virtual ~CScpPipeMappingNotification();
	bool operator==(const CScpPipeMappingNotification &other) const;
	void InitDefaults();

	virtual void Serialize(WORD format, CSegment& seg);
	virtual void DeSerialize(WORD format, CSegment& seg);
	virtual void Dump();
	virtual const char* NameOf() const { return "CScpPipeMappingNotification"; }

	void InitFromSourcesParams(CVideoRelaySourcesParams& rvideoSourcesRequest);

	void	SetSequenceNumber(APIU32 seqNum) { m_sequenceNumber = seqNum; }
	APIU32	GetSeuqenceNumber() { return m_sequenceNumber; }
	void	SetRemoteSequenceNumber(APIU32 seqNum) { m_remoteSequenceNumber = seqNum; }
	APIU32	GetRemoteSeuqenceNumber() { return m_remoteSequenceNumber; }
	void	RetrievePipesList(std::list <CScpPipeMappingObj> & otherList);

	int GetPipesListSize() { return (int)( m_listOfPipes.size() ); }


protected:
	void DeletePipesList();

	//void AddPipe(const CScpPipeWrapper &newPipe);
//	APIU32				m_channelHandle;
	APIU32				m_sequenceNumber;
	APIU32				m_remoteSequenceNumber;
	std::list <CScpPipeMappingObj>  m_listOfPipes;
};


#endif /* SCPPIPEMAPPINGNOTIFICATION_H_ */
