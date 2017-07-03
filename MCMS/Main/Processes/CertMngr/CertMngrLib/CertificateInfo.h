// CertificateInfo.h

#ifndef CERTIFICATE_INFO_H_
#define CERTIFICATE_INFO_H_

#include <list>
#include <string>
#include <vector>
#include <iostream>
#include <openssl/ssl.h>

#include "Macros.h"
#include "StructTm.h"
#include "DataTypes.h"
#include "PrettyTable.h"
#include "CertMngrDefines.h"
#include "SerializeObject.h"

class CXMLDOMElement;

class CCertificateInfo : public CSerializeObject, CNonCopyable
{
  CLASS_TYPE_1(CCertificateInfo, CSerializeObject)
 public:
                                      CCertificateInfo();
  virtual                            ~CCertificateInfo();

  virtual const char*                 NameOf() const;
  virtual void                        SerializeXml(CXMLDOMElement*& parent) const;
  virtual void                        SerializeXmlChained(CXMLDOMElement*& parent) const;
  virtual int                         DeSerializeXml(CXMLDOMElement* node,
                                                     char* pszError,
                                                     const char* action);
  virtual STATUS                      Init(const char* fname) = 0;
  virtual CCertificateInfo*           Create() const = 0;

  void                                RawSerializeXml(CXMLDOMElement& parent)
  const;
  int                                 RawDeSerializeXml(CXMLDOMElement* parent,
                                                        char* pszError,
                                                        const char* action);
  bool 								  CreateSelfSignedCert() const;
  const std::string&                  GetRaw() const;
  const std::string&                  GetSerial() const;
  const std::string&                  GetIssuer() const;
  const std::string&                  GetSubject() const;
  const std::string&                  GetDetails() const;
  const CStructTm&                    GetValidFrom() const;
  const CStructTm&                    GetValidTo() const;
  eCertificateStatus                  GetStatus() const;
  void                                UpdateStatus(eCertificateStatus stat)
  const;
  const std::list<CCertificateInfo*>& GetChained() const;
  bool                                FindChainedCert(const char* serial,std::list<CCertificateInfo*>::iterator& out);
  void 								  DeleteChainedCert(std::list<CCertificateInfo*>::iterator& it);
  std::vector<std::string>            GetSubjects() const;

  const std::string&                  GetServiceName() const;

  // Calculates size of an object in bytes.
  unsigned long                       SizeOf() const;

  // Describes run-time unique token that is used by Active Alarm.
  unsigned int                        GetToken() const;


  void SetServiceName(std::string serviceName);


 protected:
  static STATUS                       GetCN(X509_NAME* x509_name,
                                            std::string& name);
  static STATUS                       GetTime(const ASN1_TIME* time,
                                              CStructTm& out);
  static STATUS                       CheckDiskSpace(const char* db_name,
                                                     const char* fname);

  CStructTm                    m_notAfter;
  CStructTm                    m_notBefore;
  std::string                  m_raw;
  std::string                  m_serial;
  std::string                  m_issuer;
  std::string                  m_subject;
  std::string                  m_details;
  std::string                  m_serviceName;
  mutable std::string          m_status;
  std::list<CCertificateInfo*> m_chained;
  eCertificateType             m_certType;
 private:
  static unsigned int s_token;    // Keeps current run-time unique number.

  unsigned int m_token;    // The run-time unique token,
                           // starts from eMaxNumOfCertificateType.
};

// Prints out for ls_cert command.
class PrintCertOutList
{
 public:
  PrintCertOutList(std::ostream& out, const char* name);
  PrintCertOutList(const PrintCertOutList& rhs);
  ~PrintCertOutList();

  void operator()(const CCertificateInfo* cert);

 private:
  CPrettyTable<unsigned int,
               std::string,
               std::string,
               std::string,
               std::string,
               std::string,
               std::string,
               unsigned long> m_tbl;
  std::string                 m_name;
  std::ostream&               m_out;
};

#endif  // CERTIFICATE_INFO_H_

