// CDRSettings.h

#ifndef CDR_SETTINGS_H_
#define CDR_SETTINGS_H_


#include "SerializeObject.h"



class CCDRSettings : public CSerializeObject
{
CLASS_TYPE_1(CCdrLog, CPObject)
public:
  	  	  	  	  	  CCDRSettings();
  virtual            ~CCDRSettings();
  //virtual const char* NameOf() const;
  CCDRSettings(const CCDRSettings &other);
  CCDRSettings& operator = (const CCDRSettings &other);
  bool operator !=(const CCDRSettings& other) const;

  // Implementation
  	virtual CSerializeObject* Clone(){return new CCDRSettings;}
  	virtual void SerializeXml(CXMLDOMElement*& pXMLRootElement) const;
  	virtual int  DeSerializeXml(CXMLDOMElement* pXMLRootElement,char *pszError,const char* action);
  	virtual void Serialize(WORD format, CSegment& rSeg);
  	virtual void DeSerialize(WORD format, CSegment& rSeg);
  	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(CDR_SETTINGS_FILE_NAME.c_str()); }
  	void WriteXmlFile() { CSerializeObject::WriteXmlFile(CDR_SETTINGS_FILE_NAME.c_str()); }
  	void PrintToConsole();
  	BOOL GetIsLocalCdrServer(){return m_IsLocalCdrServer;}
  	BOOL GetIsRemoteCdrServer(){return m_IsRemoteCdrServer;}
  	void IncreasecdrChangedCounter();
  	std::string GetIp(){return m_ip;}
  	std::string GetPort(){return m_port;}
  	std::string GetUser(){return m_user;}
  	std::string GetPwd(){return m_pwd;}
  	std::string GetSourceId(){return m_sourceId;}

  	BOOL m_isSerializeToEMA;
  	std::string m_sourceId;//returned as identifier by CDR server (not part of the cdr settings set via EMA)

  	static const std::string CDR_SETTINGS_FILE_NAME;
  protected:

  	int DeSerializeXmlFromEma(CXMLDOMElement* pXMLRootElement,char *pszError);
  	int DeSerializeXmlFromFile(CXMLDOMElement* pXMLRootElement,char *pszError);

  private:
  	BOOL m_IsLocalCdrServer;
  	BOOL m_IsRemoteCdrServer;
  	std::string m_ip;
  	std::string m_port;
  	std::string m_user;
  	std::string m_pwd;
  	BYTE  m_bChanged;

};

#endif

