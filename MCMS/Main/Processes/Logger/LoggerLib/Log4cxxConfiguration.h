#ifndef LOG4CXXCONFIGURATION_H_
#define LOG4CXXCONFIGURATION_H_

#include "LocalLogAppenderConfiguration.h"
#include "UDPAppenderConfiguration.h"
#include "SysLogAppenderConfiguration.h"

#include <vector>

using namespace std;


class CLog4cxxConfiguration : public CSerializeObject
{
CLASS_TYPE_1(CLog4cxxConfiguration,CSerializeObject)
public:

    //Constructors

	CLog4cxxConfiguration();
	CLog4cxxConfiguration(const CLog4cxxConfiguration &other);
	CLog4cxxConfiguration& operator = (const CLog4cxxConfiguration& other);
	virtual ~CLog4cxxConfiguration();
	virtual CSerializeObject* Clone() {return new CLog4cxxConfiguration();}
	virtual void   SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int    DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);

	const char*  NameOf() const {return "CLog4cxxConfiguration";}

	void						SetMsgAvgRate(ULONGLONG rate);
	ULONGLONG						GetMsgAvgRate();

	void						SetIsCSLogStarted(BOOL b);
	BOOL						GetIsCSLogStarted();
	WORD						GetMaxLogSize() const {return m_maxLogSize;}
	void 						SetMaxLogSize(const WORD maxLogSize);
	CUDPAppenderConfiguration*			GetUDPAppender(){ return &m_udpAppender;}
	CSysLogAppenderConfiguration*			GetSysLogAppender(){return &m_sysLogAppender;}
	CLocalLogAppenderConfiguration*			GetLocalLogAppender(){return &m_localLogAppender;}

	void							CopyValue(CLog4cxxConfiguration* pOther);
	CModuleContent*				InitModule(string moduleName,unsigned int processStartInd,unsigned int processEndInd);
	void  						InitAppenders(string moduleName,unsigned int processStartInd,unsigned int processEndInd);

protected:

	ULONGLONG								m_msgAvgRate;
	CUDPAppenderConfiguration				m_udpAppender;
	CSysLogAppenderConfiguration			m_sysLogAppender;
	CLocalLogAppenderConfiguration			m_localLogAppender;
	BYTE 									m_isCSLogStarted;
	WORD									m_maxLogSize;
};

/////////////////////////////////////////////////////////////////////////
#endif /*MODULECONTENT_H_*/



