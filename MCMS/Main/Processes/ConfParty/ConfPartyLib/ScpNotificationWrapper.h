#ifndef SCP_NOTIFICATION_WRAPPER_H_
#define SCP_NOTIFICATION_WRAPPER_H_

#include <list>
#include "Segment.h"
#include "MrcStructs.h"


class CScpPipeWrapper : public CPObject {
CLASS_TYPE_1(CScpPipeWrapper,CPObject)
public:
	CScpPipeWrapper();
	CScpPipeWrapper(const CScpPipeWrapper &other);
	CScpPipeWrapper& operator= (const CScpPipeWrapper &other);
    CScpPipeWrapper& operator= (const MrmpScpPipe &other);
	bool operator==(const CScpPipeWrapper &other) const;
	void InitDefaults();

	virtual void  Serialize(WORD format,CSegment& seg);
	virtual void  DeSerialize(WORD format,CSegment& seg);
	void Dump(std::string sCaller);
	virtual const char* NameOf() const { return "CScpPipeWrapper"; }


public:
	APIU32		m_pipeId;
	APIU32		m_notificationType;
	APIU32		m_reason;
	APIUBOOL	m_bIsPermanent;
};

/////////////////////////////////////////////////////////////////////////////
class CScpNotificationWrapper: public CPObject
{
CLASS_TYPE_1(CScpNotificationWrapper,CPObject)
public:
	CScpNotificationWrapper();
	CScpNotificationWrapper(const CScpNotificationWrapper &other);
	CScpNotificationWrapper& operator= (const CScpNotificationWrapper &other);
    CScpNotificationWrapper& operator= (const MrmpScpStreamsNotificationStruct &other);
	~CScpNotificationWrapper();
	bool operator==(const CScpNotificationWrapper &other) const;
	void InitDefaults();

	virtual void Serialize(WORD format, CSegment& seg);
	virtual void DeSerialize(WORD format, CSegment& seg);
	void Dump(std::string sCaller);
	virtual const char* NameOf() const { return "CScpNotificationWrapper"; }

	bool FillPipeById(APIU32 pipeId, CScpPipeWrapper& pipeToFill);
	void AddPipe(const CScpPipeWrapper &newPipe);


public:
	APIU32				m_channelHandle;
	APIU32				m_sequenceNumber;
	APIU32				m_remoteSequenceNumber;
	APIS32				m_numOfPipes;
	std::list <CScpPipeWrapper>  m_pipes;
//	CScpPipeWrapper*	m_pPipes;
};


#endif /* SCP_NOTIFICATION_WRAPPER_H_ */
