// CertMngrProcess.cpp

#include "CertMngrProcess.h"

#include <fstream>
#include <algorithm>

#include "SslFunc.h"
#include "psosxml.h"
#include "OsFileIF.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "FaultsDefines.h"
#include "SystemFunctions.h"
#include "CertificateInfo.h"
#include <sstream>
#include "DefinesIpServiceStrings.h"

extern void CertMngrManagerEntryPoint(void* appParam);

// Functor finds an issuer in a list
class IssuerFinder
{
public:
  IssuerFinder(const char* issuer) :
    m_issuer(issuer)
  {}

  bool operator()(const CCertificateInfo* cert) const
  {
    FPASSERTMSG_AND_RETURN_VALUE(NULL == cert, "Illegal parameter", false);
    return 0 == cert->GetIssuer().compare(m_issuer);
  }

private:
  const char* m_issuer;
};

// Functor finds an issuer in a list with specified serial
class IssuerSerialFinder : public IssuerFinder
{
public:
  IssuerSerialFinder(const char* issuer, const char* serial) :
    IssuerFinder(issuer),
    m_serial(serial)
  {}

  bool operator()(const CCertificateInfo* cert) const
  {
    FPASSERTMSG_AND_RETURN_VALUE(NULL == cert, "Illegal parameter", false);

    return IssuerFinder::operator()(cert) &&
           0 == cert->GetSerial().compare(m_serial);
  }

private:
  const char* m_serial;
};

// Functor finds an issuer in a list
class SubjectFinder
{
public:
  SubjectFinder(const char* subject) :
    m_subject(subject)
  { }

  bool operator()(const CCertificateInfo* cert) const
  {
    FPASSERTMSG_AND_RETURN_VALUE(NULL == cert, "Illegal parameter", false);

    std::vector<std::string> sub = cert->GetSubjects();

    std::vector<std::string>::const_iterator it;
    for (it = sub.begin(); it < sub.end(); ++it)
    {
      if (0 == it->compare(m_subject))
        return true;
    }

    return false;
  }

private:
  const char* m_subject;
};


// Functor finds an issuer in a list
class ServiceNameFinder
{
public:
	ServiceNameFinder(std::string serviceName) :
		m_serviceName(serviceName)
  { }

  bool operator()(const CCertificateInfo* cert) const
  {
    FPASSERTMSG_AND_RETURN_VALUE(NULL == cert, "Illegal parameter", false);

    return   0 == m_serviceName.compare(cert->GetServiceName());
  }

private:
  const std::string m_serviceName;
};


CProcessBase* CreateNewProcess(void)
{
  return new CCertMngrProcess;
}

CCertMngrProcess::CCertMngrProcess(void) :
  m_CertificateTrustList(eCertificateTrust),
  m_CertificatePersonalList(eCertificatePersonal),
  m_CertificateCSPersonalList(eOCS),
  m_CertificateRevocationList(eCertificateRevocation)

{
  m_numOfIpServices = 0;
  m_alreayValidateCasForCS = FALSE;
}

// Virtual
const char* CCertMngrProcess::NameOf(void) const
{
  return GetCompileType();
}

// Virtual
eProcessType CCertMngrProcess::GetProcessType(void)
{
  return eProcessCertMngr;
}

// Virtual
BOOL CCertMngrProcess::UsingSockets(void)
{
  return NO;
}

// Virtual
int CCertMngrProcess::GetProcessAddressSpace(void)
{
  return CertMngrLimits::kAddressSpaceMaxSize;
}

// Virtual
// Increases the time to 120 seconds to let fips mode set during startup
DWORD CCertMngrProcess::GetMaxTimeForIdle(void) const
{
  return 12000;
}

TaskEntryPoint CCertMngrProcess::GetManagerEntryPoint()
{
  return CertMngrManagerEntryPoint;
}

const CCertificateList* CCertMngrProcess::ReadList(eCertificateType type) const
{
  return const_cast<CCertMngrProcess*>(this)->GetList(type);
}

CCertificateList* CCertMngrProcess::GetList(eCertificateType type)
{

  switch (type)
  {
    case eCertificateTrust:
      return &m_CertificateTrustList;

    case eCertificatePersonal:
    	return &m_CertificatePersonalList;

    case eOCS:
      return &m_CertificateCSPersonalList;

    case eCertificateRevocation:
      return &m_CertificateRevocationList;


    default:
      PASSERTSTREAM(true,
          "Unable to get list for " << CertificateTypeToStr(type));
  } // switch

  return NULL;
}


STATUS CCertMngrProcess::AddCertificate(eCertificateType type,
                                        const char* fname)
{


    std::string ServiceName = "";

	if (type == eCertificatePersonal )
	{
		ServiceName = MANAGEMENT_NETWORK_NAME;
	}
	if (eOCS == type )
	{
		ServiceName =  GetServiceNameFromFileName(fname);
	}
  CCertificateList* list = GetList(type);
  if (!list)
  {
    return STATUS_FAIL;
  }

  STATUS stat = list->Add(fname, ServiceName);
  if (STATUS_OK != stat)
  {
    return stat;
  }

  // get a new issuer and serial
  std::string issuer = list->ReadList().back()->GetIssuer();
  std::string serial = list->ReadList().back()->GetSerial();


  if (eCertificateTrust == type)
  {
    // remove old CTL of the issuer and serial if exists
    const std::list<CCertificateInfo*>& ctl =
      m_CertificateTrustList.ReadList();

    std::list<CCertificateInfo*>::const_iterator before_last = ctl.end();
    --before_last;

    // look for the issuer into old CTL
    std::list<CCertificateInfo*>::const_iterator ptr =
      std::find_if(ctl.begin(), before_last,
                   IssuerSerialFinder(issuer.c_str(), serial.c_str()));

    // the issuer was existed in CTL, remove it
    if (ptr != before_last)
    {

      // it removes first occurrence
      stat = list->Delete(issuer.c_str(), serial.c_str());
      if (STATUS_OK != stat)
      {
        list->PopBack();
        return stat;
      }
    }
  }

  // Personal list has only single certificate
  if (eCertificatePersonal == type)
  {
    // Removes previous (first) if exist
    if (list->GetListSize() > 1)
      list->PopFront();
  }

  if (eOCS == type)
  {
	    // remove old CTL of the issuer and serial if exists
	    const std::list<CCertificateInfo*>& ctl =
	      m_CertificateCSPersonalList.ReadList();

	    std::list<CCertificateInfo*>::const_iterator before_last = ctl.end();
	    --before_last;


	    // look for the issuer into old CTL
	    std::list<CCertificateInfo*>::const_iterator ptr =
	      std::find_if(ctl.begin(), before_last,
	    		  ServiceNameFinder(list->ReadList().back()->GetServiceName()));


	    // the issuer was existed in CTL, remove it
	    if (ptr != before_last && ptr != ctl.end())
	    {

	      // it removes first occurrence
	      stat = list->Delete((*ptr)->GetIssuer().c_str(), (*ptr)->GetSerial().c_str());
	      if (STATUS_OK != stat)
	      {
	        list->PopBack();
	        return stat;
	      }
	    }
  }

  // Checks that the CTL has an issuer of the CRL
  if (eCertificateRevocation == type)
  {
    // Checks that the issuer exist at CTL
    const std::list<CCertificateInfo*>& ctl =
      m_CertificateTrustList.ReadList();

    // Looks for a CTL that issued the CRL
    std::list<CCertificateInfo*>::const_iterator ptr =
      std::find_if(ctl.begin(), ctl.end(), SubjectFinder(issuer.c_str()));

    // If not found delete CRL and set an error
    if (ptr == ctl.end())
    {
      list->PopBack();

      PASSERTSTREAM(true,
          "Unable to add CRL of issuer " << issuer
          << ". Add certificate of the issuer to CTL first.");
      return STATUS_CERTIFICATE_CTL_FOR_CRL_NOT_EXIST;
    }

    // Removes old CRL of the issuer if exists
    const std::list<CCertificateInfo*>& crl =
      m_CertificateRevocationList.ReadList();

    std::list<CCertificateInfo*>::const_iterator before_last = crl.end();
    --before_last;

    // Looks for the issuer into CRL
    ptr = std::find_if(crl.begin(), before_last, IssuerFinder(issuer.c_str()));

    // The issuer was existed in CRL, remove it
    if (ptr != before_last)
    {
      // Removes first occurrence
      stat = list->Delete(issuer.c_str(), NULL);
      if (STATUS_OK != stat)
      {
        list->PopBack();
        return stat;
      }
    }
  }

  stat = SaveCertificate(type);
  if (STATUS_OK != stat)
  {
    list->PopBack();
    return stat;
  }

  if (eCertificateRevocation != type)
  {
    TRACEINTOFUNC << "Added " << CertificateTypeToStr(type)
                  << ", issuer " << issuer
                  << ", serial " << serial;
  }
  else
  {
    TRACEINTOFUNC << "Added " << CertificateTypeToStr(type)
                  << ", issuer " << issuer;
  }

  return STATUS_OK;
}

// Static
STATUS CCertMngrProcess::GetStatusByAlarm(const CERT_ALARM_VECTOR& aa)
{
  for (CERT_ALARM_VECTOR::const_iterator it = aa.begin();
       it < aa.end(); ++it)
  {
    if (!it->m_actv)
      continue;

    switch (it->m_code)
    {
      case AA_FAILED_TO_VALIDATE_CERTIFICATE:
        return CertificateStatusNotExist(it->m_type);

      case AA_CERTIFICATE_EXPIRED:
        return STATUS_CERTIFICATE_ALREADY_EXPIRED;

      case AA_CERTIFICATE_COMMON_NAME_ERROR:
        return STATUS_CERTIFICATE_COMMON_NAME_DIFFER_THAN_RMX_HOST_NAME;

      case AA_CERTIFICATE_NOT_VALID_YET:
        return STATUS_CERTIFICATE_NOT_VALID_YET;

      case AA_CERTIFICATE_GOING_TO_BE_EXPIRED:
        return STATUS_OK;
      case AA_CERTIFICATE_SELF_CERT_NOT_ALLOWED_FOR_MS_SERVICE:
    	  return AA_CERTIFICATE_SELF_CERT_NOT_ALLOWED_FOR_MS_SERVICE;
      default:
        FPASSERTSTREAM(true, "Unable to continue with code " << it->m_code);
    } // switch

    return STATUS_OK;
  }

  return STATUS_OK;
}

STATUS CCertMngrProcess::VerifyCertificates(const char* host_name,
                                            const CStructTm& now,
                                            bool add_aa,
                                            bool ca_validation,
                                            BYTE revocation_method,
                                            std::string& out) const
{

	CERT_ALARM_VECTOR aa;
	// TODO drabkin the function makes MCU_TMP_DIR/privateKey.pem and MCU_TMP_DIR/passphrase.sh
	STATUS stat = CSslFunctions::CheckValidationOfCerificateAndPrivateKey();
	  if (STATUS_OK != stat)
	  {
		std::ostringstream buf;
		buf << "Cannot validate "
			<< CertificateTypeToStr(eCertificatePersonal)
			<< ": " << GetStatusAsString(stat);

		aa.push_back(CCertAlarmInfo(true,
									AA_FAILED_TO_VALIDATE_CERTIFICATE,
									eCertificatePersonal,
									buf.str(),
									eCertificatePersonal));

		TRACEINTOFUNC << "WARNING: " << buf.str().c_str();
		// Continues to verify CTL and CRL
	  }

	  // Additional verification of Personal certificate
	  // AA_FAILED_TO_VALIDATE_CERTIFICATE will be removed there
	  STATUS result = m_CertificatePersonalList.Verify(host_name, now, aa);


	  // No need to validate ca/crl certificate again because if we validated them for cs service we never turn them off
	  if (ca_validation)
	  {
		  m_CertificateTrustList.Verify(host_name, now, aa);
		  if(eCrl == (eRevocationMethod)revocation_method)
		  {
			  m_CertificateRevocationList.Verify(host_name, now, aa);
		  }
	  }
	  else
	  {
		  TRACEINTOFUNC << "CA validation is skipped";
	  }

	  // add/remove active alarms
	  if (add_aa)
	  {
		  std::for_each(aa.begin(), aa.end(),
					  std::mem_fun_ref(&CCertAlarmInfo::Fire));
	  }

	  // Prints log active alarms
	  std::for_each(aa.begin(), aa.end(), CCertAlarmInfoPrinter(out));
	  TRACEINTOFUNC << "Management Certificate Active Alarm Table:\n" << out;

	  // Returns first error status if any
	  if (STATUS_OK != stat)
		return stat;

	  return GetStatusByAlarm(aa);
}
// In case of CS - VerifyCSServiceCertificates is performed for each CS service at start up.
// The alrams of the certificates are fired or not according the configuration at start up and not changed even
// when cs configuration changed.
STATUS CCertMngrProcess::VerifyCSServiceCertificates(const std::string serviceName,
											const CStructTm& now,
                                            bool add_aa,
                                            bool ca_validation,
                                            BYTE revocation_method,
                                            BYTE isMSService,
                                            std::string& out)
{
	 CERT_ALARM_VECTOR aa;

	 TRACEINTOFUNC << "VerifyCSServiceCertificates add_aa:" << (int)add_aa << " serviceName " << serviceName << " Is MS service " << isMSService
			 	   << " ca_validation " << (int)ca_validation << " time " << now;
	 ostringstream buff;
	 m_CertificateCSPersonalList.PrintOut(buff);
	 TRACEINTOFUNC << "VerifyCSServiceCertificates print CS certificates in list \n"  << buff.str().c_str();
	  STATUS stat = m_CertificateCSPersonalList.VerifyCSService(serviceName,isMSService, now, aa);

	  if (ca_validation)
	  {
		  m_alreayValidateCasForCS = TRUE;
		  m_CertificateTrustList.Verify("", now, aa);
		  if(eCrl == (eRevocationMethod)revocation_method)
			  m_CertificateRevocationList.Verify("", now, aa);
	  }
	  else
	  {
		  TRACEINTOFUNC << "CA validation for CS is skipped";
	  }

	  // add/remove active alarms
	  if (add_aa)
	  {

		  std::for_each(aa.begin(), aa.end(),
					  std::mem_fun_ref(&CCertAlarmInfo::Fire));


	  }
	  // Prints log active alarms
	  std::for_each(aa.begin(), aa.end(), CCertAlarmInfoPrinter(out));

	  TRACEINTOFUNC << "CS Certificate Active Alarm Table:\n" << out;

	  // Returns first error status if any
	  if (STATUS_OK != stat)
	  {
		  return stat;
	  }

	  return GetStatusByAlarm(aa);
}


STATUS CCertMngrProcess::RemoveActiveAlarm(std::string& out, BOOL withCSAlarms)
{
  CERT_ALARM_VECTOR aa;

  m_CertificatePersonalList.RemoveAA(aa);

  if (withCSAlarms)
  {
	  m_CertificateCSPersonalList.RemoveAA(aa);
  }
  if (withCSAlarms || !m_alreayValidateCasForCS)
  {
	  m_alreayValidateCasForCS = FALSE;
	  m_CertificateTrustList.RemoveAA(aa);
	  m_CertificateRevocationList.RemoveAA(aa);

  }
  std::for_each(aa.begin(), aa.end(),
                std::mem_fun_ref(&CCertAlarmInfo::Fire));

  // print log active alarms
  std::for_each(aa.begin(), aa.end(), CCertAlarmInfoPrinter(out));
  TRACEINTOFUNC << "RemoveActiveAlarm: withCSAlarms,m_alreayValidateCasForCS : " << withCSAlarms << ", " << m_alreayValidateCasForCS <<
		  ". Certificate Active Alarm Table:\n" << out;

  return STATUS_OK;
}

STATUS CCertMngrProcess::SaveCertificate(eCertificateType type)
{
	if (type == eOCS)
	{
		// we don't have to do it for cs certificate- because we dont keep them in one list
		return STATUS_OK;
	}

  std::string       ca_name = CertificateCAFileName(type);
  CCertificateList* list = GetList(type);
  if (ca_name == "" || !list)
    return STATUS_FAIL;

  STATUS stat = list->Save(ca_name.c_str(), NULL);
  if (STATUS_OK != stat)
    return stat;

  // Personal certificate list doesn't have local db
  if (eCertificatePersonal == type || eOCS == type)
    return STATUS_OK;

  const char* db_name = CertificateDBFileName(type);
  const char* title = CertificateNodeTitle(type);
  if (!db_name || !title)
    return STATUS_FAIL;

  stat = list->Save(db_name, title);
  if (STATUS_OK != stat)
    return stat;

  return STATUS_OK;
}

void CCertMngrProcess::FillLists(void)
{
  FillList(eCertificateTrust);
  FillList(eCertificateRevocation);
  FillPersonalCertificateList();
}


void CCertMngrProcess::FillList(eCertificateType type)
{
  const char* db_name = CertificateDBFileName(type);
  CCertificateList* list = GetList(type);
  if (!list || !db_name)
    return;

  if (!IsFileExists(db_name))
  {
    TRACEINTOFUNC << "IsFileExists: " << db_name
                  << ": No DB file of " << CertificateTypeToStr(type);
    return;
  }

  // Reads certificate database file to memory. In case of
  // huge CRL files (more than 30MB in DER) the process can take more than
  // watchdog timeout (15 seconds). Lock/Unlock semaphores allows to answer
  // to watchdog's keep alive pinging.
  
  CTaskApp* pTaskApp = GetCurrentTask();
  if (pTaskApp == NULL)
  {
	  TRACEINTOFUNC << "pTaskApp is NULL!!";
	  return;
  }

  pTaskApp->UnlockRelevantSemaphore();
  STATUS stat = list->ReadXmlFile(db_name);
  pTaskApp->LockRelevantSemaphore();

  PASSERTSTREAM(STATUS_OK != stat,
                "Unable to read DB file of " << CertificateTypeToStr(type));
}

STATUS CCertMngrProcess::DeleteCertificate(eCertificateType type,
                                           const char* issuer,
                                           const char* serial)
{
  CCertificateList* list = GetList(type);
  if (!list)
    return CertificateStatusNotExist(type);

  const CCertificateInfo* cert = list->GetCertificate(issuer, serial);
  if (!cert)
    return CertificateStatusNotExist(type);

  // Blocks removing last certificate in case it is in secure mode
  if (CSslFunctions::IsJitcMode() && list->GetListSize() == 1)
    return STATUS_UNABLE_TO_DELETE_LAST_CERTIFICATE;

  // Keeps subjects of the deleted certificate included chained certificates
  std::vector<std::string> sub = cert->GetSubjects();

  STATUS stat = list->Delete(issuer, serial);
  if (STATUS_OK != stat)
    return stat;

  stat = SaveCertificate(type);
  if (STATUS_OK != stat)
    return stat;

  // Removes CRL of a CTL issuer
  if (eCertificateTrust != type)
    return STATUS_OK;

  // Gets CRL inner list
  const std::list<CCertificateInfo*>& crl =
    m_CertificateRevocationList.ReadList();

  bool changed = false;
  std::vector<std::string>::const_iterator it;
  for (it = sub.begin(); it < sub.end(); ++it)
  {
    // Looks for the CTL issuer into CRL inner list
    std::list<CCertificateInfo*>::const_iterator ptr =
      std::find_if(crl.begin(), crl.end(), IssuerFinder(it->c_str()));

    // Does nothing if not found
    if (ptr == crl.end())
      continue;

    // If found remove the CRL of the deleted CTL
    stat = m_CertificateRevocationList.Delete(it->c_str(), NULL);
    if (stat)
      return stat;

    if (!changed)
      changed = true;
  }

  // Save CRL to files
  if (changed)
    return SaveCertificate(eCertificateRevocation);

  return STATUS_OK;
}

void CCertMngrProcess::FillPersonalCertificateList(void)
{
	if (IsFileExists(CERTF))
	{
	  m_CertificatePersonalList.Add(CERTF.c_str(), MANAGEMENT_NETWORK_NAME);
	}
	else
	{
		TRACEINTOFUNC << "Unable to find personal certificate file " << CERTF;
	}
}


void CCertMngrProcess::SetServiceName(int pos, std::string service_name, BOOL isSecured,
										BOOL isRequestPeerCertificate,std::string host_name,
										BYTE revocation_method,std::string ocsp_url,BYTE  isMSService)

{
	PASSERTSTREAM_AND_RETURN((pos < 0 || pos >= MAX_NUMBER_OF_SERVICES_IN_RMX_4000),
        "Invalid pos for GetServide " << pos);


  if (m_IPServiceList[pos].ServName == "") {  // no name in this position
    m_numOfIpServices++;
  }

  TRACEINTOFUNC << "ServiceName: " << service_name << " Pos :" <<pos << " m_numOfIpServices" << m_numOfIpServices;

  m_IPServiceList[pos].ServName = service_name;
  m_IPServiceList[pos].IsSecured = isSecured;
  m_IPServiceList[pos].IsRequestPeerCertificate = isRequestPeerCertificate;
  m_IPServiceList[pos].HostName = host_name;
  m_IPServiceList[pos].Revocation_Method = revocation_method;
  m_IPServiceList[pos].OcspUrl = ocsp_url ;
  m_IPServiceList[pos].IsMsService = isMSService ;
  // add related certificate if exist
  std::ostringstream cerFileNameStream;
  cerFileNameStream << HOME_CS << "cs" << (pos +1 )<< "/cert.pem";

  TRACEINTOFUNC << "CS certificate full path :" << cerFileNameStream.str();
  if (IsFileExists(cerFileNameStream.str()))
  {
	    TRACEINTOFUNC << "CS certificate exist add it to the certificate list";
		 m_CertificateCSPersonalList.Add((const char *)cerFileNameStream.str().c_str(), service_name);

  }
  else
	  TRACEWARN << "CS certificate was not found full path :" << cerFileNameStream.str();

	string buf;
	CStructTm now;
	SystemGetTime(now);

	VerifyCSServiceCertificates(service_name,
			now,
			isSecured ? true : false,
					!m_alreayValidateCasForCS && isSecured && isRequestPeerCertificate,
					revocation_method,
					isMSService,
					buf);

  TRACEINTOFUNC << "SetServiceName " << service_name << " isSecured " << (int)isSecured << " m_alreayValidateCasForCS " << (int)m_alreayValidateCasForCS << " m_alreayValidateCasForCS " << (int)m_alreayValidateCasForCS
		  	  	<<" Is MS (Link) Service " << (int)isMSService << " aa:\n" << buf; ;
}

std::string CCertMngrProcess::GetServiceName(int pos) const
{
	PASSERTSTREAM_AND_RETURN_VALUE(pos < 0 || pos >= MAX_NUMBER_OF_SERVICES_IN_RMX_4000,
        "Invalid pos for GetServide " << pos, "");

  return m_IPServiceList[pos].ServName;
}

BOOL CCertMngrProcess::GetServiceIsSecured(int pos) const
{
	PASSERTSTREAM_AND_RETURN_VALUE(pos < 0 || pos >= MAX_NUMBER_OF_SERVICES_IN_RMX_4000,
        "Invalid pos for GetServiceIsSecured " << pos, FALSE);

  return m_IPServiceList[pos].IsSecured;
}

BYTE   CCertMngrProcess::GetIsMsService(int pos ) const
{
	PASSERTSTREAM_AND_RETURN_VALUE(pos < 0 || pos >= MAX_NUMBER_OF_SERVICES_IN_RMX_4000,
        "Invalid pos for GetIsMsService " << pos, FALSE);

  return m_IPServiceList[pos].IsMsService;

}

BYTE   CCertMngrProcess::GetServiceRevocationMethod(int pos)
{
	PASSERTSTREAM_AND_RETURN_VALUE(pos < 0 || pos >= MAX_NUMBER_OF_SERVICES_IN_RMX_4000,
        "Invalid pos for GetServiceRevocationMethod " << pos, FALSE);

  return   m_IPServiceList[pos].Revocation_Method;
}

int CCertMngrProcess::GetServiceId(std::string service_name) const
{
  for (int i = 0; i < MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
  {
    TRACEINTOFUNC << m_IPServiceList[i].ServName << " and " <<service_name;
    if (m_IPServiceList[i].ServName == service_name)
      return i + 1;
  }

  return 0;
}

std::string CCertMngrProcess::GetServiceHostName(std::string service_name) const
{
  for (int i = 0; i < MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
  {
    TRACEINTOFUNC << m_IPServiceList[i].ServName << " and " <<service_name;
    if (m_IPServiceList[i].ServName == service_name)
      return m_IPServiceList[i].HostName;
  }

  return "";
}

BOOL CCertMngrProcess::GetIsRequestPeerCertificate (int pos) const
{
	PASSERTSTREAM_AND_RETURN_VALUE(pos < 0 || pos >= MAX_NUMBER_OF_SERVICES_IN_RMX_4000,
        "Invalid pos for GetIsRequestPeerCertificate " << pos, FALSE);

  return m_IPServiceList[pos].IsRequestPeerCertificate;
}

std::string CCertMngrProcess::GetServiceNameFromFileName(const char* fileName) const
{
	char* beforecsNumber = (char*)strstr(fileName, "KeysForCS/cs");
	if (beforecsNumber != NULL && strlen(beforecsNumber)  >  strlen("KeysForCS/cs") + 1)
	{
		char numCsChar = beforecsNumber[strlen("KeysForCS/cs")];

		// TRACEINTOFUNC << "CCertMngrProcess::GetServiceNameFromFileName  beforecsNumber " << beforecsNumber << " numChar " <<   (int(numCsChar - '0') -1)  << " service name " << GetServiceName(((int)(numCsChar - '0'))-1);

		return GetServiceName(((int)(numCsChar - '0'))-1);
	}
	return "";
}

