// CertificateList.cpp

#include "CertificateList.h"

#include <fstream>
#include <iomanip>
#include <functional>
#include <algorithm>

#include "FaultsDefines.h"
#include "OsFileIF.h"
#include "StatusesGeneral.h"
#include "SslFunc.h"
#include "ApiStatuses.h"
#include "SharedDefines.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "TraceStream.h"
#include "CertificateInfoCTL.h"
#include "CertificateInfoCRL.h"
#include "DefinesIpServiceStrings.h"
#include "CertMngrProcess.h"

// Functor finds an issuer in a list
class CertificateFinder
{
public:
    CertificateFinder(const CCertificateInfo& cerInfo) :
       m_cer(cerInfo)
    {}

    bool operator()(const CCertificateInfo* cer) const
    {
    	//BRIDGE-14015 change way comparing certificates use rfc
    	/*rfc2459.txt
    	 * 4.1.2.2  Serial number

		   The serial number is an integer assigned by the CA to each
		   certificate.  It MUST be unique for each certificate issued by a
		   given CA (i.e., the issuer name and serial number identify a unique
		   certificate).
    	 * */
    	if((cer->GetIssuer()== m_cer.GetIssuer())&&(cer->GetSerial()== m_cer.GetSerial()))
    	{
    		 return true;
    	}
    	else
    		return false;
    }

private:
    const CCertificateInfo& m_cer;
};

// Functor prints out for DB file
class PrintCertOutDB
{
public:
    PrintCertOutDB(CXMLDOMElement& root) :
        m_root(root)
    {}

    void operator()(const CCertificateInfo* cert)
    {
        FPASSERT_AND_RETURN(NULL == cert);
        cert->RawSerializeXml(m_root);
    }

private:
    CXMLDOMElement& m_root;
};

// Functor verifies certificates
class VerifyCertList
{
public:


    VerifyCertList(const char* host,
                   const CStructTm& now,
                   eCertificateType type,
                   CERT_ALARM_VECTOR& out) :
        m_token(0),
        m_host(host),
        m_now(now),
        m_type(type),
        m_out(out)
    {}


    void operator()(const CCertificateInfo* cert)
    {
        FPASSERT_AND_RETURN(NULL == cert);
        FTRACEINTOFUNC << "start verify certificate cert  serial : " <<cert->GetSerial() << " Service Name :" << cert->GetServiceName();

        bool was_err = false;
        m_token = cert->GetToken();
        // Checks subject only for personal certificate
        if (eCertificatePersonal == m_type)
        {
            was_err = vfy_host(*cert);
        }

        // Checks start date
        if (vfy_start(*cert))
        {
            cert->UpdateStatus(eCertificateErr);
            return;
        }

        // Checks end date
        if (vfy_end(*cert))
        {
            cert->UpdateStatus(eCertificateErr);
            return;
        }

        // Checks going to expired date
        if (vfy_before(*cert))
        {
            cert->UpdateStatus(eCertificateErr);
            return;
        }


        // Checks chained certificates
        const std::list<CCertificateInfo*>& chained = cert->GetChained();

        // Verifies chained certificates recursively
        std::for_each(
            chained.begin(), chained.end(),
            VerifyCertList(m_host, m_now, m_type, m_out));

        // Checks status of the chained certificates
        for (std::list<CCertificateInfo*>::const_iterator it = chained.begin();
             it != chained.end(); ++it)
        {
            if (eCertificateErr != (*it)->GetStatus())
                continue;

            was_err = true;

            break;
        }
        cert->UpdateStatus(was_err ? eCertificateErr : eCertificateOK);
    }

private:
    bool add(WORD code, const std::string& desc)
    {
        m_out.push_back(CCertAlarmInfo(true, code, m_token, desc, m_type));
        return true;
    }

    bool rem(WORD code)
    {
        m_out.push_back(CCertAlarmInfo(false, code, m_token, "", m_type));
        return false;
    }

    bool vfy_host(const CCertificateInfo& cert)
    {
    	// due to BRIDGE-5107 the check of  common name is disabled.
    	return rem(AA_CERTIFICATE_COMMON_NAME_ERROR);
        /*FTRACEINTOFUNC << "Host Name in cert is : " << cert.GetSubject();
        if (0 == cert.GetSubject().compare(m_host))
        {
            return rem(AA_CERTIFICATE_COMMON_NAME_ERROR);
        }
        cert.CreateSelfSignedCert();

        std::ostringstream buf;
        if (m_type == eOCS)
        {
        	buf << cert.GetServiceName() << " ";
        }
        buf << CertificateTypeToStr(m_type)
            << " host name " << cert.GetSubject()
            << " differs from RMX host name " << m_host;

        FTRACEINTOFUNC << "WARNING: " << buf.str().c_str();

        return add(AA_CERTIFICATE_COMMON_NAME_ERROR, buf.str());*/
    }

    bool vfy_start(const CCertificateInfo& cert)
    {
        const CStructTm& begin = cert.GetValidFrom();

        if (begin < m_now)
        {
            return rem(AA_CERTIFICATE_NOT_VALID_YET);
        }
        CStructTm nowDoubleCheck;
        SystemGetTime(nowDoubleCheck);
        if(begin < nowDoubleCheck)
        {
        	m_now = nowDoubleCheck;
        	return rem(AA_CERTIFICATE_NOT_VALID_YET);
        }
	cert.CreateSelfSignedCert();
        std::ostringstream buf;
        if (m_type == eOCS)
        {
        	buf << cert.GetServiceName() << " ";
        }

        buf << CertificateTypeToStr(m_type);

        if (!cert.GetSubject().empty())
            buf << " " << cert.GetSubject();

        buf << " of " << cert.GetIssuer() << " is not valid yet";

        FTRACEINTOFUNC << "WARNING: "
                       << buf.str() << ", begin: " << begin
                       << ", now: " << m_now;

        rem(AA_CERTIFICATE_EXPIRED);
        rem(AA_CERTIFICATE_GOING_TO_BE_EXPIRED);

        return add(AA_CERTIFICATE_NOT_VALID_YET, buf.str());
    }

    bool vfy_end(const CCertificateInfo& cert)
    {
        const CStructTm& end = cert.GetValidTo();

        if (end > m_now)
        {
            return rem(AA_CERTIFICATE_EXPIRED);
        }
        else
	    {
			if (cert.CreateSelfSignedCert())
			{
				return add(AA_CERTIFICATE_EXPIRED, "Self signed certificate has expired.A new one was created. Please reset MCU");
			}
			else
			{
				std::ostringstream buf;
				if (m_type == eOCS)
				{
					buf << cert.GetServiceName() << " ";
				}
				buf << CertificateTypeToStr(m_type);

				if (!cert.GetSubject().empty())
					buf << " " << cert.GetSubject();

				buf << " of " << cert.GetIssuer() << " expired";

				FTRACEINTOFUNC << "WARNING: "
							   << buf.str() << ", end: " << end
							   << ", now: " << m_now;

				rem(AA_CERTIFICATE_GOING_TO_BE_EXPIRED);

				return add(AA_CERTIFICATE_EXPIRED, buf.str());
			 }
	    }
    }


    bool vfy_before(const CCertificateInfo& cert)
    {
        const CStructTm& end = cert.GetValidTo();
        // calculate the seconds in days
        int diff = (((end - m_now) / 60) / 60) / 24 + 1;
        //printf("\n days offffffff !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %d \n", diff);
        if (diff > DAYS_BEFORE_CERTIFICATE_EXPIRED_ALERT)
        {
        	return rem(AA_CERTIFICATE_GOING_TO_BE_EXPIRED);
        }
        else
        {
        	//printf("\n days offffffff !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
        	if (cert.CreateSelfSignedCert())
        	{
        		return add(AA_CERTIFICATE_GOING_TO_BE_EXPIRED, "Self signed certificate has expired.A new one was created. Please reset MCU");
        	}
        	else
        	{
				std::ostringstream buf;
				if (m_type == eOCS)
				{
					buf << cert.GetServiceName() << " ";
				}
				buf << CertificateTypeToStr(m_type);

				if (!cert.GetSubject().empty())
					buf << " " << cert.GetSubject();

				buf << " of " << cert.GetIssuer() << " is going to be expired in "
					<< diff << " day" << ((diff > 1) ? "s" : "");

				FTRACEINTOFUNC << "WARNING: "
							   << buf.str() << ", end: " << end
							   << ", now: " << m_now;

				return add(AA_CERTIFICATE_GOING_TO_BE_EXPIRED, buf.str());
        	}
        }

    }

    unsigned int       m_token;
    const char*        m_host;
    CStructTm		   m_now;
    eCertificateType   m_type;
    CERT_ALARM_VECTOR& m_out;
};

// Functor removes active alarms
class RemoveAACertList
{
public:
    RemoveAACertList(eCertificateType type, CERT_ALARM_VECTOR& out) :
        m_token(0),
        m_type(type),
        m_out(out)
    {}

    void operator()(const CCertificateInfo* cert)
    {
        FPASSERT_AND_RETURN(NULL == cert);

        m_token = cert->GetToken();
        if (eCertificatePersonal == m_type)
            rem(AA_CERTIFICATE_COMMON_NAME_ERROR);

        rem(AA_CERTIFICATE_NOT_VALID_YET);
        rem(AA_CERTIFICATE_EXPIRED);
        rem(AA_CERTIFICATE_GOING_TO_BE_EXPIRED);

        // Checks chained certificates
        const std::list<CCertificateInfo*>& chained = cert->GetChained();

        // Verifies chained certificates recursively
        std::for_each(
            chained.begin(), chained.end(),
            RemoveAACertList(m_type, m_out));
    }

private:
    void rem(WORD code)
    {
        m_out.push_back(CCertAlarmInfo(false, code, m_token, "", m_type));
    }

    unsigned int       m_token;
    eCertificateType   m_type;
    CERT_ALARM_VECTOR& m_out;
};

// Functor prints out for CA file
class PrintCertOutCA
{
public:
    PrintCertOutCA(std::ostream& out) :
        m_out(out)
    {}

    void operator()(const CCertificateInfo* cert)
    {
        m_out << cert->GetRaw().c_str() << endl;
    }

private:
    std::ostream& m_out;
};

CCertificateList::CCertificateList(eCertificateType type) :
    m_updateCounter(0),
    m_type(type)
{}

CCertificateList::~CCertificateList(void)
{
	while (!m_CertificateList.empty())
	    PopFront();
}

// Virtual
const char* CCertificateList::NameOf(void) const
{
    return GetCompileType();
}

// Virtual
CSerializeObject* CCertificateList::Clone(void)
{
    CCertificateList* ret = new CCertificateList(m_type);
    ret->m_CertificateList = m_CertificateList;
    ret->m_updateCounter = m_updateCounter;

    return ret;
}

void CCertificateList::PopFront(void)
{
    PASSERTMSG_AND_RETURN(m_CertificateList.empty(), "Wrong flow");

    delete m_CertificateList.front();
    m_CertificateList.pop_front();
}

void CCertificateList::PopBack(void)
{
    PASSERTMSG_AND_RETURN(m_CertificateList.empty(), "Wrong flow");

    delete m_CertificateList.back();
    m_CertificateList.pop_back();
}

void CCertificateList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	SerializeXml(pFatherNode, 0xFFFFFFFF);
}

void CCertificateList::SerializeXml(CXMLDOMElement*& pFatherNode, DWORD dwObjectToken) const
{
	SerializeXml(pFatherNode, dwObjectToken, NULL);
}


void CCertificateList::SerializeXml(CXMLDOMElement*& pFatherNode, DWORD dwObjectToken, const CCertificateList* additionListToSerialize) const
{
	BYTE bChanged = FALSE;
	CXMLDOMElement* pCASummaryListNode = NULL;

    if (NULL == pFatherNode)
    {
        pFatherNode = new CXMLDOMElement;
		pFatherNode->set_nodeName("CERTIFICATE_SUMMARY_LIST");
		pCASummaryListNode = pFatherNode;
    }
    else
    {
    	pCASummaryListNode = pFatherNode->AddChildNode("CERTIFICATE_SUMMARY_LIST");
	}

	pCASummaryListNode->AddChildNode("OBJ_TOKEN", m_updateCounter);

	if (dwObjectToken==0xFFFFFFFF)
	{
		bChanged = TRUE;
	}
	else
	{
		if (m_updateCounter > dwObjectToken || (additionListToSerialize != NULL && additionListToSerialize->m_updateCounter > dwObjectToken ))
		{
			bChanged = TRUE;
		}
	}
	pCASummaryListNode->AddChildNode("CHANGED", bChanged, _BOOL);

	std::list<CCertificateInfo*>::const_iterator it;
	for (it = m_CertificateList.begin(); it != m_CertificateList.end(); it++)
	{
		(*it)->SerializeXmlChained(pCASummaryListNode);
	}
	if (additionListToSerialize != NULL)
	{
		for (it = additionListToSerialize->m_CertificateList.begin(); it != additionListToSerialize->m_CertificateList.end(); it++)
		{
			(*it)->SerializeXml(pCASummaryListNode);
		}
	}
}

CCertificateInfo* CCertificateList::ProduceCertificateInfo(void)
{
    switch (m_type)
    {
    case eCertificateTrust:
    case eCertificatePersonal:

    case eOCS:
        return new CCertificateInfoCTL;

    case eCertificateRevocation:
        return new CCertificateInfoCRL;



    default:
        PASSERTSTREAM(true,
            "Unable to continue with " << CertificateTypeToStr(m_type));
        break;
    }

    return NULL;
}

// The function is relevant only when reading certificates from a file
int CCertificateList::DeSerializeXml(CXMLDOMElement* pCertificateListNode,
                                     char* pszError, const char* action)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pCertificateDetailsNode;
	GET_FIRST_CHILD_NODE(pCertificateListNode,
                         "CERTIFICATE_DETAILS",
                         pCertificateDetailsNode);

	while (pCertificateDetailsNode)
	{
		CCertificateInfo* cert = ProduceCertificateInfo();
		if (NULL == cert)
		    return STATUS_FAIL;

		nStatus = cert->RawDeSerializeXml(pCertificateDetailsNode,
                                          pszError, NULL);
		
		if (nStatus != STATUS_OK)
			return nStatus;
		
		nStatus = Add(cert);

		if (nStatus != STATUS_OK)
		{
		    delete cert;
		    return nStatus;
		}
		
		GET_NEXT_CHILD_NODE(pCertificateListNode,
                            "CERTIFICATE_DETAILS",
                            pCertificateDetailsNode);
	}
	
	return STATUS_OK;
}

STATUS CCertificateList::Add(const char* fname, const std::string serviceName)
{
    CCertificateInfo* cert = ProduceCertificateInfo();

    cert->SetServiceName(serviceName);

    if(cert == NULL)
    {
    	TRACEINTOFUNC << "cert is NULL";
    	return STATUS_FAIL;
    }
    TRACEINTOFUNC << "read certificate " << fname;
    STATUS stat = cert->Init(fname);


    if (STATUS_OK != stat)
    {
    	TRACEWARN << "failed to read certificate ";
    	if((STATUS_NO_START_LINE_CERTIFICATE ==stat) && ((eCertificatePersonal==m_type) || (eOCS==m_type)))
    	{
    		TRACEINTOFUNC << "an attempt to recover self sign certificate";
    		std::string alarm  ="Error in Self signed certificate for service "+ serviceName +" .Please reset MCU for creating a new certificate.";
    		cert->CreateSelfSignedCert();
    		CCertMngrProcess* proc = ((CCertMngrProcess*)CCertMngrProcess::GetProcess());
    		proc->AddActiveAlarmFromProcess(FAULT_GENERAL_SUBJECT,
    											AA_CERTIFICATE_EXPIRED,
    											MAJOR_ERROR_LEVEL,
    											"Self signed certificate has expired.A new one was created. Please reset MCU",
    											true,
    											true);
    	}
        delete cert;
        return stat;
    }

    stat = Add(cert);
    if (STATUS_OK != stat)
    {
        delete cert;
        return stat;
    }

	return STATUS_OK;
}

STATUS CCertificateList::Add(CCertificateInfo* cert)
{
    PASSERT_AND_RETURN_VALUE(NULL == cert, STATUS_FAIL);
    CTraceStream                    loggerInfo(__FILE__, __LINE__, eLevelInfoNormal, NULL); loggerInfo.seekp(0, std::ios_base::cur);

    CCertificateInfo* existingCert = IsExist(*cert);

    if (existingCert != NULL)
    {
		if ( eOCS != m_type || existingCert->GetServiceName() ==  cert->GetServiceName() )
		{
			TRACEWARN << "Unable to add existed certificate, issuer " << cert->GetIssuer()
					   << ", subject " << cert->GetSubject() << " ,serial " << cert->GetSerial();
			return STATUS_CERTIFICATE_ALREADY_EXIST;

		}
		else
		{
			loggerInfo<<"CCertificateList::Add line: " <<__LINE__  <<" Added certificate m_type==eOCS ," <<" existing Cert service = [" << existingCert->GetServiceName() << "]" << " curret cer service [ " << cert->GetServiceName() << "]" << endl;

		}

    }
    else
    {
    	loggerInfo<<"CCertificateList::Add line: " <<__LINE__  << " no previous certificate was found" << endl;
    }
    m_CertificateList.push_back(cert);
	IncreaseUpdateCounter();
	
	loggerInfo <<"CCertificateList::Add line: " <<__LINE__ <<" Added certificate, issuer " << cert->GetIssuer()
	              << ", subject " << cert->GetSubject();

	return STATUS_OK;
}

// Title is NULL for CA file
STATUS CCertificateList::Save(const char* fname, const char* title) const
{
    PASSERT_AND_RETURN_VALUE(NULL == fname, STATUS_FAIL);

    // Removes file if list is empty
    if (m_CertificateList.empty())
    {
        if (!IsFileExists(fname))
            return STATUS_OK;

        BOOL res = DeleteFile(fname);
        PASSERTSTREAM_AND_RETURN_VALUE(FALSE == res,
            "DeleteFile: " << fname << ": " << strerror(errno),
            STATUS_FILE_DELETE_ERROR);

        TRACEINTOFUNC << "Removed " << fname;

        return STATUS_OK;
    }

    // Writes list to file, remove old content
    std::ofstream out(fname, ios_base::trunc);
    PASSERTSTREAM_AND_RETURN_VALUE(!out,
        "ofstream::ofstream: unable to open " << fname << ": " << strerror(errno),
        STATUS_FILE_OPEN_ERROR);

    // Prints out info from each element in list
    if (title)
    {
        CXMLDOMElement root;
        root.set_nodeName(title);

        std::for_each(
            m_CertificateList.begin(), m_CertificateList.end(),
            PrintCertOutDB(root));

        char* buf;
        HRES res = root.DumpDataAsLongStringEx(&buf);
        if (SEC_OK != res)
        {
            DEALLOCBUFFER(buf);
            PASSERTMSG(true, "Unable to dump XML");
            return STATUS_FAIL;
        }

        out << buf;
        DEALLOCBUFFER(buf);
    }
    else
    {
        std::for_each(
            m_CertificateList.begin(), m_CertificateList.end(),
            PrintCertOutCA(out));
    }

    // Flushs data to file
    out.flush();
    PASSERTSTREAM_AND_RETURN_VALUE(!out,
        "ofstream::flush: " << fname << ": " << strerror(errno),
        STATUS_FILE_WRITE_ERROR);

    TRACEINTOFUNC << "Updated " << fname;

    return STATUS_OK;
}

CCertificateInfo* CCertificateList::IsExist(const CCertificateInfo& cer) const
{
    std::list<CCertificateInfo*>::const_iterator ptr =
        std::find_if(m_CertificateList.begin(), m_CertificateList.end(),
            CertificateFinder(cer));

    if (ptr != m_CertificateList.end())
    {
    	return *ptr;
    }
    else
    {
    	return NULL;
    }
}

DWORD CCertificateList::GetUpdateCounter(void) const
{
	return m_updateCounter;
}

void CCertificateList::IncreaseUpdateCounter(void)
{
    ++m_updateCounter;

    if (m_updateCounter == 0xFFFFFFFF)
        m_updateCounter = 0;
}

int CCertificateList::GetListSize(void) const
{
	return m_CertificateList.size();
}
//incase of chain it will return a pointer to the father certificate of the chained list
STATUS CCertificateList::Get(const char* issuer,
                             const char* serial,
                             std::list<CCertificateInfo*>::iterator& out,
                             CCertificateInfo* pFatherCert)
{
    PASSERTMSG_AND_RETURN_VALUE(!issuer || '\0' == *issuer,
        "Unable to find certificate: issuer name is empty",
        STATUS_ISSUER_NAME_IS_EMPTY);

    // Does not check serial number for CRL
    if (eCertificateRevocation != m_type)
    {
        PASSERTMSG_AND_RETURN_VALUE(!serial || '\0' == *serial,
            "Unable to find certificate: serial number name is empty",
            STATUS_SERIAL_NUMBER_IS_EMPTY);
    }

    std::list<CCertificateInfo*>::iterator it;
    for (it = m_CertificateList.begin(); it != m_CertificateList.end(); it++)
    {
        if (0 != strcmp(issuer, (*it)->GetIssuer().c_str()))
            continue;

        // Skips serial number for CRL
        if (m_type != eCertificateRevocation)
        {
        	if (0 != strcmp(serial, (*it)->GetSerial().c_str()))
        	{
        		//check chain certificates for CA
        		if(m_type == eCertificateTrust)
        		{
        			if((*it)->FindChainedCert(serial,out))
        			{
        				pFatherCert = *it;
        				return STATUS_OK;
        			}
        		}
        		continue;
        	}
        }

        out = it;
        return STATUS_OK;
    }

    if (eCertificateRevocation != m_type)
    {
    	TRACESTRFUNC(eLevelWarn) << "Unable to find " << CertificateTypeToStr(m_type)
                        << ", issuer " << issuer
                        << ", serial " << serial;
    }
    else
    {

    	TRACESTRFUNC(eLevelWarn) <<"Unable to find " << CertificateTypeToStr(m_type)
                        << ", issuer " << issuer;
    }

    return CertificateStatusNotExist(m_type);
}




STATUS CCertificateList::Delete(const char* issuer, const char* serial)
{
    std::list<CCertificateInfo*>::iterator it;
    CCertificateInfo* pFatherCert = NULL;
    STATUS stat = Get(issuer, serial, it,pFatherCert);
    if (stat)
        return stat;

    // Applies the functor to the certificate to get list of active alarms
    CERT_ALARM_VECTOR aa;

    RemoveAACertList(m_type, aa)(*it);

    // Removes possible Active Alarms
    std::for_each(aa.begin(), aa.end(), std::mem_fun_ref(&CCertAlarmInfo::Fire));

    // Prints log active alarms
    std::string info;
    std::for_each(aa.begin(), aa.end(), CCertAlarmInfoPrinter(info));
    TRACEINTOFUNC << "Certificate Active Alarm Table:\n" << info;

    if(m_type == eCertificateTrust && pFatherCert)
    {
    	pFatherCert->DeleteChainedCert(it);
    }
    else
    {
     // Frees object memory
     delete *it;

     // Removes from the list
     m_CertificateList.erase(it);
    }

    IncreaseUpdateCounter();

    if (eCertificateRevocation != m_type)
    {
        TRACEINTOFUNC << "Removed " << CertificateTypeToStr(m_type)
                      << ", issuer " << issuer
                      << ", serial " << serial;
    }
    else
    {
        TRACEINTOFUNC << "Removed " << CertificateTypeToStr(m_type)
                      << ", issuer " << issuer;
    }

    return STATUS_OK;
}

const CCertificateInfo* CCertificateList::GetCertificate(const char* issuer,
                                                         const char* serial) const
{
    std::list<CCertificateInfo*>::iterator it;
    STATUS stat = const_cast<CCertificateList*>(this)->Get(issuer, serial, it);
    if (stat)
        return NULL;

    return *it;
}

const std::list<CCertificateInfo*>& CCertificateList::ReadList(void) const
{
    return m_CertificateList;
}

void CCertificateList::PrintOut(std::ostream& out) const
{
    std::for_each(
        m_CertificateList.begin(), m_CertificateList.end(),
        PrintCertOutList(out, CertificateTypeToStr(m_type)));
}

STATUS CCertificateList::Verify(const char* host,
                                const CStructTm& now,
                                CERT_ALARM_VECTOR& out) const
{


    std::string fname = CertificateCAFileName(m_type);
    if ("" == fname)
        return STATUS_FAIL;

    if (!IsFileExists(fname.c_str()))
    {
        std::ostringstream buf;
        buf << "Cannot validate " << CertificateTypeToStr(m_type)
            << ": " << fname << ": No such file";

        out.push_back(CCertAlarmInfo(true,
                                     AA_FAILED_TO_VALIDATE_CERTIFICATE,
                                     m_type,
                                     buf.str(),
                                     m_type));

        TRACEINTOFUNC << "WARNING: " << buf.str().c_str();

        return STATUS_OK;
    }

    if (m_CertificateList.empty())
    {
        std::ostringstream buf;
        buf << "Cannot validate " << CertificateTypeToStr(m_type)
            << ": " << fname << ": No certificate";

        out.push_back(CCertAlarmInfo(true,
                                     AA_FAILED_TO_VALIDATE_CERTIFICATE,
                                     m_type,
                                     buf.str(),
                                     m_type));

        TRACEINTOFUNC << "WARNING: " << buf.str().c_str();

        return STATUS_OK;
    }

    // Removes possible alarm about validation
    out.push_back(CCertAlarmInfo(false,
                                 AA_FAILED_TO_VALIDATE_CERTIFICATE,
                                 m_type,
                                 "",
                                 m_type));

    std::for_each(
        m_CertificateList.begin(), m_CertificateList.end(),
        VerifyCertList(host, now, m_type, out));

    TRACEINTOFUNC << "Verified " << m_CertificateList.size() << " certificates" << " type " << m_type;

    return STATUS_OK;
}

STATUS CCertificateList::VerifyCSService(const std::string serviceName,BYTE isMSService,
                                const CStructTm& now,
                                CERT_ALARM_VECTOR& out) const
{

	STATUS stat = STATUS_OK;

	std::string fname =  CertificateCAFileName(m_type, serviceName);
	if ("" == fname)
	{
		std::ostringstream bufNoFile;
		bufNoFile << "Cannot validate " << CertificateTypeToStr(m_type) << "of " << serviceName
							   << ": cannot get certificate file name.";

		out.push_back(CCertAlarmInfo(true,
						 AA_FAILED_TO_VALIDATE_CERTIFICATE,
						 m_type,
						 bufNoFile.str(),
						 m_type));

		TRACESTRFUNC(eLevelError) << "Failed to get cs certificate file name for service " << serviceName;

		stat =   STATUS_FAIL;
	}

	if (stat == STATUS_OK && !IsFileExists(fname.c_str()))
	{
		std::ostringstream buf;
		buf << "Cannot validate " << CertificateTypeToStr(m_type) << "of " << serviceName
			   << ": " << fname << ": No such file";

		out.push_back(CCertAlarmInfo(true,
						 AA_FAILED_TO_VALIDATE_CERTIFICATE,
						 m_type,
						 buf.str(),
						 m_type));

		TRACESTRFUNC(eLevelError) << "WARNING: " << buf.str().c_str();

		stat =   STATUS_FAIL;

	}
	if (stat !=  STATUS_FAIL)
	{
		// Removes possible alarm about validation
		out.push_back(CCertAlarmInfo(false,
									 AA_FAILED_TO_VALIDATE_CERTIFICATE,
									 m_type,
									 "",
									 m_type));

	}
	std::list<CCertificateInfo*>::const_iterator itCer = m_CertificateList.begin();
	int numCerCer = 0;
	for (; itCer != m_CertificateList.end(); ++itCer)
	{
		if (serviceName  == (*itCer)->GetServiceName())
		{
			++numCerCer;

			VerifyCertList("", now, m_type, out)(*itCer);
			if(isMSService)
			{
				if ((*itCer)->GetIssuer() == (*itCer)->GetSubject())
				{ //self sign certificated
					std::ostringstream msg;
					msg << "Service " << serviceName << " have self sign certificate please install a CA sign certificate.";
					out.push_back(CCertAlarmInfo(true,AA_CERTIFICATE_SELF_CERT_NOT_ALLOWED_FOR_MS_SERVICE,	 m_type,msg.str(),m_type));
				}
				else
				{
					out.push_back(CCertAlarmInfo(false,AA_CERTIFICATE_SELF_CERT_NOT_ALLOWED_FOR_MS_SERVICE,	 m_type,"",m_type));
				}


			}
		}
	}

    TRACEINTOFUNC << "Verified " << numCerCer << " cs certificates. " << " serviceName: " << serviceName << " now " << now;

    return stat;
}


STATUS CCertificateList::RemoveAA(CERT_ALARM_VECTOR& out) const
{
	if (eOCS != m_type)
	{
		out.push_back(CCertAlarmInfo(false,
									 AA_FAILED_TO_VALIDATE_CERTIFICATE,
									 m_type,
									 "",
									 m_type));
	    std::for_each(
	            m_CertificateList.begin(), m_CertificateList.end(),
	            RemoveAACertList(m_type, out));

	}
	else
	{
		std::list<CCertificateInfo*>::const_iterator itCer = m_CertificateList.begin();
		for (; itCer != m_CertificateList.end(); ++itCer)
		{
			out.push_back(CCertAlarmInfo(false,
										AA_FAILED_TO_VALIDATE_CERTIFICATE,
										(*itCer)->GetToken(),
										"",
										m_type));
			RemoveAACertList(m_type, out)(*itCer);
		}
	}
    TRACEINTOFUNC << "Removed Active Alarms of " << m_CertificateList.size()
                  << " certificates " ;


    return STATUS_OK;
}
