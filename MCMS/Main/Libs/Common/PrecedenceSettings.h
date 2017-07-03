#ifndef _PRECEDENCE_SETTINGS
#define _PRECEDENCE_SETTINGS

#include "SerializeObject.h"




class CXMLDOMElement;


/////////////////////////////////////////////////////////////////////////////
// CPrecedenceSettings


#define NUM_SINGLE_DOMAINS 2
#define NUM_PRECEDENCE_LEVELS 6
#define PRECEDENCE_SETTINGS_FILE "Cfg/PrecedenceSettings.xml"
#define DEFAULT_PRECEDENCE_R_PRIORITY 	255
#define ERROR_SIGNALING_DSCP_VALUE 		254

typedef
struct
{
	ePrecedenceLevelType precedenceLevelType;
	BYTE r_priority;
	BYTE audio_dscp;
	BYTE video_dscp;

}PRECEDENCE_LEVEL;


class SingleDomainSettings
{
public:
	BYTE 				m_DomainId;
	std::string			m_DomainName;
	BYTE 				m_SignalingDSCP;
	PRECEDENCE_LEVEL 	m_PrecedenceLevels[NUM_PRECEDENCE_LEVELS];

};




class CPrecedenceSettings : public CSerializeObject
{
CLASS_TYPE_1(CPrecedenceSettings,CPObject)
public:
	   //Constructors
	   CPrecedenceSettings();
	   CPrecedenceSettings(const CPrecedenceSettings &other);
	   CPrecedenceSettings& operator = (const CPrecedenceSettings &other);
	   virtual ~CPrecedenceSettings();


	// Implementation
	virtual CSerializeObject* Clone(){return new CPrecedenceSettings;}
	virtual void SerializeXml(CXMLDOMElement*& pXMLRootElement) const;
	virtual int  DeSerializeXml(CXMLDOMElement* pXMLRootElement,char *pszError,const char* action);
	virtual void Serialize(WORD format, CSegment& rSeg);
	virtual void DeSerialize(WORD format, CSegment& rSeg);
	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(PRECEDENCE_SETTINGS_FILE_NAME); }
	void WriteXmlFile() { CSerializeObject::WriteXmlFile(PRECEDENCE_SETTINGS_FILE_NAME); }
//	void SaveToFile();
//	bool LoadFromFile();
	void PrintToConsole();
	const char* GetPrecedenceSettingsName(ePrecedenceLevelType precedenceLevelType);
	SingleDomainSettings& GetSingleDomain(int domainId) {return m_SingleDomains[domainId];}
	const SingleDomainSettings& GetSingleDomain(int domainId) const {return m_SingleDomains[domainId];}

	BYTE GetDomainId(const char* szDomainName) const;
	BYTE GetPrecedenceLevelForRPrio(BYTE domainId, BYTE RPrio) const;
	BYTE GetRPrioForPrecedenceLevel(BYTE domainId, BYTE precedenceLevel) const;
	BOOL IsPrecedenceEnabled() const {return m_pUsePrecedence;}
	BYTE GetSignalingDSCP() const {return m_SingleDomains[0].m_SignalingDSCP;}

	static const char* PRECEDENCE_SETTINGS_FILE_NAME;
protected:
	int m_pUsePrecedence;
	SingleDomainSettings m_SingleDomains[NUM_SINGLE_DOMAINS];
	int DeSerializeXmlFromEma(CXMLDOMElement* pXMLRootElement,char *pszError);
	int DeSerializeXmlFromFile(CXMLDOMElement* pXMLRootElement,char *pszError);
};



/////////////////////////////////////////////////////////////////////////////
#endif
