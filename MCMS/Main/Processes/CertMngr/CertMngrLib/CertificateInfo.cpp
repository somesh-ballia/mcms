// CertificateInfo.cpp

#include "CertificateInfo.h"

#include <fstream>
#include <algorithm>
#include <iomanip>
#include <functional>
#include <limits>

#include "Trace.h"
#include "psosxml.h"
#include "SslFunc.h"
#include "OsFileIF.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CertMngrProcess.h"
#include "SSLInterface.h"
#include "OpcodesMcmsInternal.h"
#include "FaultsDefines.h"
// Static.
// First numbers are reserved for Active Alarms without CCertificateInfo object.
unsigned int CCertificateInfo::s_token = eMaxNumOfCertificateType;

// Static.
STATUS CCertificateInfo::CheckDiskSpace(const char* db_name,
                                        const char* fname)
{
  FPASSERT_AND_RETURN_VALUE(NULL == db_name, STATUS_FAIL);
  FPASSERT_AND_RETURN_VALUE(NULL == fname, STATUS_FAIL);

  // Reads file size of a new file, could be in DER that takes less
  // space than PEM.
  int file_size = GetFileSize(fname);
  FPASSERTSTREAM_AND_RETURN_VALUE(file_size < 0,
    "GetFileSize: " << fname << ": " << strerror(errno),
    STATUS_FILE_OPEN_ERROR);

  unsigned int db_size;
  if (IsFileExists(db_name))
  {
    // Reads file size of CRL XML database.
    db_size = GetFileSize(db_name);
    FPASSERTSTREAM_AND_RETURN_VALUE(
      file_size < 0,
      "GetFileSize: " << db_name << ": " <<
      strerror(errno),
      STATUS_FILE_OPEN_ERROR);
  }
  else
  {
    db_size = 0u;
  }

  // XML takes about kDERtoXMLRate time more space than DER file.
  float file_size_rate = CertMngrLimits::kDERtoXMLRate *
                         static_cast<float>(file_size);
  unsigned int new_db_size = db_size + static_cast<unsigned int>(file_size_rate);

  FPASSERTSTREAM_AND_RETURN_VALUE(new_db_size > CertMngrLimits::kXMLFileMaxSize,
      "Size " << db_name << " and size of " << fname
      << " multiplied by DER to XML Rate exceeded XML max size: ("
      << db_size << " + " << file_size
      << " * " << CertMngrLimits::kDERtoXMLRate << " = "
      << new_db_size << ") > " << CertMngrLimits::kXMLFileMaxSize,
      STATUS_CERTIFICATE_CRL_EXCEEDED_MAX_SIZE);

  return STATUS_OK;
}

CCertificateInfo::CCertificateInfo() :
  m_status(CertificateStatusToStr(eCertificateOK)),
  m_token(s_token++)
{
  PASSERT(s_token == std::numeric_limits<unsigned int>::max());
  m_certType = eCertificateTypeUnknown;
}

// Virtual.
CCertificateInfo::~CCertificateInfo()
{
  while (!m_chained.empty())
  {
    delete m_chained.front();
    m_chained.pop_front();
  }
}

const char* CCertificateInfo::NameOf() const
{
  return GetCompileType();
}
void    CCertificateInfo::SerializeXmlChained(CXMLDOMElement*& parent) const
{
	 CXMLDOMElement* node = parent->AddChildNode("CERTIFICATE_SUMMARY");

	  node->AddChildNode("SERIAL_NUMBER",      m_serial);
	  node->AddChildNode("ISSUED_TO",          m_subject);
	  node->AddChildNode("ISSUED_BY",          m_issuer);
	  node->AddChildNode("CERTIFICATE_STATUS", m_status);
	  node->AddChildNode("SERVICE_NAME",       m_serviceName);
	  node->AddChildNode("VALID_FROM",         m_notBefore);
	  node->AddChildNode("VALID_TO",           m_notAfter);


	  if (m_chained.empty())
	    return;

	  std::list<CCertificateInfo*>::const_iterator it;
	  std::string tmp;
	  for (it = m_chained.begin(); it != m_chained.end(); ++it)
	  {
		  if(((*it)->m_serial == m_serial)&&((*it)->GetSubject()==m_subject))
			  continue;
		  CXMLDOMElement* ChainNode = parent->AddChildNode("CERTIFICATE_SUMMARY");
		  ChainNode->AddChildNode("SERIAL_NUMBER",    (*it)->GetSerial()  );
		  ChainNode->AddChildNode("ISSUED_TO",         (*it)->GetSubject());
		  ChainNode->AddChildNode("ISSUED_BY",         (*it)->GetIssuer());
		  ChainNode->AddChildNode("CERTIFICATE_STATUS", (*it)->GetStatus());
		  ChainNode->AddChildNode("SERVICE_NAME",       (*it)->GetServiceName());
		  ChainNode->AddChildNode("VALID_FROM",         (*it)->GetValidFrom());
		  ChainNode->AddChildNode("VALID_TO",           (*it)->GetValidTo());
	  }
}

void CCertificateInfo::SerializeXml(CXMLDOMElement*& parent) const
{
  CXMLDOMElement* node = parent->AddChildNode("CERTIFICATE_SUMMARY");

  node->AddChildNode("SERIAL_NUMBER",      m_serial);
  node->AddChildNode("ISSUED_TO",          m_subject);
  node->AddChildNode("ISSUED_BY",          m_issuer);
  node->AddChildNode("CERTIFICATE_STATUS", m_status);
  node->AddChildNode("SERVICE_NAME",       m_serviceName);
  node->AddChildNode("VALID_FROM",         m_notBefore);
  node->AddChildNode("VALID_TO",           m_notAfter);
}

int CCertificateInfo::DeSerializeXml(CXMLDOMElement* node,
                                     char* pszError, const char* action)
{
  int nStatus;

  GET_VALIDATE_CHILD(node, "SERIAL_NUMBER",      m_serial,      ONE_LINE_BUFFER_LENGTH);
  GET_VALIDATE_CHILD(node, "ISSUED_TO",          m_subject,     ONE_LINE_BUFFER_LENGTH);
  GET_VALIDATE_CHILD(node, "ISSUED_BY",          m_issuer,      ONE_LINE_BUFFER_LENGTH);
  GET_VALIDATE_CHILD(node, "CERTIFICATE_STATUS", m_status,      ONE_LINE_BUFFER_LENGTH);
  GET_VALIDATE_CHILD(node, "SERVICE_NAME",       m_serviceName, ONE_LINE_BUFFER_LENGTH);
  GET_VALIDATE_CHILD(node, "VALID_FROM",         &m_notBefore,  DATE_TIME);
  GET_VALIDATE_CHILD(node, "VALID_TO",           &m_notAfter,   DATE_TIME);

  return STATUS_OK;
}

bool CCertificateInfo::CreateSelfSignedCert() const
{
	if (GetIssuer() == GetSubject() && (m_certType != eCertificateTrust))//means that it is a self signed certificate
	{
		std::string output;
		char strToAdd[256];
		CCertMngrProcess* proc = ((CCertMngrProcess*)CCertMngrProcess::GetProcess());
		proc->RemoveActiveAlarmFromProcess(AA_CERTIFICATE_EXPIRED);
		if (GetServiceName() == "Management Network")
		{
			snprintf(strToAdd,sizeof(strToAdd),"Scripts/Self_Signed_Cert.sh Create_managment_certificate");
			SystemPipedCommand(strToAdd,output);
			//CSslInterface::ConvertPrivateKey();
			//proc->CleanPersonalList();
			//proc->FillLists();
			/*proc->AddActiveAlarmFromProcess
									(FAULT_GENERAL_SUBJECT,
									AA_CERTIFICATE_EXPIRED,
									MAJOR_ERROR_LEVEL,
									"Self signed certificate has expired.A new one was created. Please reset MCU",
									true,
									true);*/
			return true;
		}
		else
		{
			int service_id=((CCertMngrProcess*)CProcessBase::GetProcess())->GetServiceId(GetServiceName());
			if (service_id <= 0)
				return false;
			std:string host_name=((CCertMngrProcess*)CProcessBase::GetProcess())->GetServiceHostName(GetServiceName());
			if (host_name == "")
				return false;
			snprintf(strToAdd,sizeof(strToAdd),"Scripts/Self_Signed_Cert.sh Create_ip_service_certificate %s %d", host_name.c_str(), service_id);
			//printf("\ncreate new cs certificate-it is expired!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %s \n", strToAdd);
			SystemPipedCommand(strToAdd,output);

			/*if(alarmMsg.empty())
			{
				alarmMsg = "Self signed certificate has expired.A new one was created .Please reset MCU";
			}
			proc->AddActiveAlarmFromProcess
									(FAULT_GENERAL_SUBJECT,
									AA_CERTIFICATE_EXPIRED,
									MAJOR_ERROR_LEVEL,
									alarmMsg.c_str(),
									true,
									true);*/
			//CSegment*  pRetParam = new CSegment;

			//proc->CleanCSList();
			//const COsQueue* pCsMbx =
					//CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

			//STATUS status = pCsMbx->Send(pRetParam,CSMNGR_CERTMNGR_IP_SERVICE_PARAM_REQ);

//			FPASSERTSTREAM_AND_RETURN_VALUE(0 != status,
//			"failed to send request to the CSMngr ",
//			false);

			return true;
		}


	}
	return false;
}

void CCertificateInfo::RawSerializeXml(CXMLDOMElement& parent) const
{
  CXMLDOMElement* node = parent.AddChildNode("CERTIFICATE_DETAILS");
  PASSERTMSG_AND_RETURN(node == NULL, "XML failure");

  CXMLDOMElement* stat = node->AddChildNode("CERTIFICATE", m_raw);
  PASSERTMSG_AND_RETURN(stat == NULL, "XML failure");

  stat = node->AddChildNode("CERTIFICATE_FULL_DETAILS", m_details);
  PASSERTMSG_AND_RETURN(stat == NULL, "XML failure");

  SerializeXml(node);

  if (m_chained.empty())
    return;

  CXMLDOMElement* chained = node->AddChildNode("CERTIFICATE_CHAINED");
  PASSERTMSG_AND_RETURN(chained == NULL, "XML failure");

  std::list<CCertificateInfo*>::const_iterator it;
  for (it = m_chained.begin(); it != m_chained.end(); ++it)
    (*it)->RawSerializeXml(*chained);
}

int CCertificateInfo::RawDeSerializeXml(CXMLDOMElement* details,
                                        char* pszError, const char* action)
{
  int nStatus;

  if (details)
  {
    // VNGR-19854: 7.5 >> RMX 1500 >> Secure mode >> Add new certificate
    // ( big file 3.5 Mb) >> CertManager process crashed
    //
    // Use regular method on big strings causes to performance problems:
    // GET_VALIDATE_CHILD(details, "CERTIFICATE", m_raw, UNLIMITED_CHAR_LENGTH);
    //
    // Extracts big strings from XML fast and without validation.
    char* buf = NULL;
    HRES  hres = details->getChildNodeValueByName("CERTIFICATE", &buf);
    if (SEC_OK == hres)
    {
      if (buf != NULL)
        m_raw.assign(buf);
      else
        m_raw.clear();
    }
    else
    {
      PASSERTMSG(true, "XML failure");
    }
  }

  GET_VALIDATE_CHILD(details,
                     "CERTIFICATE_FULL_DETAILS",
                     m_details,
                     FIFTY_LINE_BUFFER_LENGTH);

  CXMLDOMElement* summary;
  GET_CHILD_NODE(details, "CERTIFICATE_SUMMARY", summary);

  DeSerializeXml(summary, pszError, action);

  CXMLDOMElement* chained;
  GET_CHILD_NODE(details, "CERTIFICATE_CHAINED", chained);

  CXMLDOMElement* cert;
  GET_FIRST_CHILD_NODE(chained, "CERTIFICATE_DETAILS", cert);
  while (cert)
  {
    // Produces same object with default constructor.
    CCertificateInfo* obj = Create();

    // Recursively deserializes data to the object.
    obj->RawDeSerializeXml(cert, pszError, action);

    m_chained.push_back(obj);

    GET_NEXT_CHILD_NODE(chained, "CERTIFICATE_DETAILS", cert);
  }

  return STATUS_OK;
}

// Static
STATUS CCertificateInfo::GetTime(const ASN1_TIME* time, CStructTm& out)
{
  STATUS stat = CSslFunctions::ConvertAns1TimeToStructTm(time, out);
  return (STATUS_OK == stat) ? STATUS_OK : STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
}

// Static
STATUS CCertificateInfo::GetCN(X509_NAME* x509_name, std::string& name)
{
  char  buf[256];
  char* pbuf = X509_NAME_oneline(x509_name, buf, ARRAYSIZE(buf));
  FPASSERTSTREAM_AND_RETURN_VALUE(NULL == pbuf,
    "X509_NAME_oneline: " << CSslFunctions::SSLErrMsg(),
    STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

  char* cn = strstr(buf, "CN=");

  if(NULL == cn)
  {
	  FTRACEWARN << " Unable to find CN in the certificate !";
	  return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
  }

  name = cn+3;
  return STATUS_OK;
}

void CCertificateInfo::UpdateStatus(eCertificateStatus stat) const
{
  // m_status is mutable data-member.
  m_status = CertificateStatusToStr(stat);
}

const std::string& CCertificateInfo::GetRaw() const
{
  return m_raw;
}

const std::string& CCertificateInfo::GetSerial() const
{
  return m_serial;
}

const std::string& CCertificateInfo::GetIssuer() const
{
  return m_issuer;
}

const std::string& CCertificateInfo::GetSubject() const
{
  return m_subject;
}


const std::string& CCertificateInfo::GetServiceName() const
{
	return m_serviceName;
}

const std::string& CCertificateInfo::GetDetails() const
{
  return m_details;
}

const CStructTm& CCertificateInfo::GetValidFrom() const
{
  return m_notBefore;
}

const CStructTm& CCertificateInfo::GetValidTo() const
{
  return m_notAfter;
}

eCertificateStatus CCertificateInfo::GetStatus() const
{
  return CertificateStatusFromStr(m_status.c_str());
}
bool  CCertificateInfo::FindChainedCert(const char* serial,std::list<CCertificateInfo*>::iterator& out)
{

	if(m_chained.empty())
		return false;
	std::list<CCertificateInfo*>::iterator it;

	for (it = m_chained.begin(); it != m_chained.end(); ++it)
	{
		if (0 != strcmp(serial, (*it)->GetSerial().c_str()))
			continue;
		out = it;
		return true;
	}
	return false;

}
void CCertificateInfo::DeleteChainedCert(std::list<CCertificateInfo*>::iterator& it)
{
	 // Removes from the list
	 delete *it;
	 // Removes from the list
	 m_chained.erase(it);

}
const std::list<CCertificateInfo*>& CCertificateInfo::GetChained() const
{
  return m_chained;
}

// Returns sequence of subjects included chained subjects.
std::vector<std::string> CCertificateInfo::GetSubjects() const
{
  std::vector<std::string> ret(m_chained.size() + 1);

  // The own subject goes to the first place.
  ret[0] = GetSubject();

  // Fills vector by chained subjects.
  std::transform(
    m_chained.begin(), m_chained.end(),
    ret.begin() + 1,
    std::mem_fun(&CCertificateInfo::GetSubject));

  return ret;
}


unsigned long CCertificateInfo::SizeOf() const
{
  unsigned long ret = 0;

  ret += sizeof m_notBefore;
  ret += sizeof m_notAfter;
  ret += m_raw.length();
  ret += m_serial.length();
  ret += m_issuer.length();
  ret += m_subject.length();
  ret += m_details.length();
  ret += m_serviceName.length();
  ret += m_status.length();


  std::list<CCertificateInfo*>::const_iterator it;
  for (it = m_chained.begin(); it != m_chained.end(); ++it)
    ret += (*it)->SizeOf();

  return ret;
}

unsigned int CCertificateInfo::GetToken() const
{
  return m_token;
}

void CCertificateInfo::SetServiceName(std::string serviceName)
{
	m_serviceName = serviceName;
}

PrintCertOutList::PrintCertOutList(std::ostream& out, const char* name) :
  m_tbl("id", "serial", "subject", "issuer", "valid from", "valid to", "service name", "sizeof"),
  m_name(name),
  m_out(out)
{}

// for_each algorithm gets the functor by value and returns it by value. It
// causes to additional destructor calling. The functor is designed to print
// messages at destructor. The copy constructor keeps pretty table empty
// by design.
PrintCertOutList::PrintCertOutList(const PrintCertOutList& rhs) :
  m_name(rhs.m_name),
  m_out(rhs.m_out)
{}

PrintCertOutList::~PrintCertOutList()
{
  if (m_tbl.IsEmpty())
    return;

  m_out << " " << m_name << ":";
  m_tbl.Dump(m_out);
}

void PrintCertOutList::operator()(const CCertificateInfo* cert)
{
  FPASSERT_AND_RETURN(NULL == cert);

  const CStructTm& from = cert->GetValidFrom();
  std::ostringstream from_str;
  from_str << std::setfill('0')
           << std::setw(4) << from.m_year
           << std::setw(2) << from.m_mon
           << std::setw(2) << from.m_day
           << std::setw(2) << from.m_hour
           << std::setw(2) << from.m_min
           << std::setw(2) << from.m_sec;

  const CStructTm& to = cert->GetValidTo();
  std::ostringstream to_str;
  to_str << std::setfill('0')
         << std::setw(4) << to.m_year
         << std::setw(2) << to.m_mon
         << std::setw(2) << to.m_day
         << std::setw(2) << to.m_hour
         << std::setw(2) << to.m_min
         << std::setw(2) << to.m_sec;

  m_tbl.Add(cert->GetToken(),
            cert->GetSerial(),
            cert->GetSubject(),
            cert->GetIssuer(),
            from_str.str(),
            to_str.str(),
            cert->GetServiceName(),
            cert->SizeOf());

  // Prints out chained certificates.
  const std::list<CCertificateInfo*>& chained = cert->GetChained();

  if (!chained.empty())
  {
    std::ostringstream name;
    name << "Chained, " << chained.size()
         << " ancestor" << ((chained.size() > 1) ? "s" : "");

    // Recursively prints chained certificates.
    std::for_each(
      chained.begin(), chained.end(),
      PrintCertOutList(m_out, name.str().c_str()));

    m_out << std::endl << std::endl;
  }
}

