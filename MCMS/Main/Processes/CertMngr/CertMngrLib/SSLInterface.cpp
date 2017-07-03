// SSLInterfcae.cpp: implementation of the CSslInterface class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//14/12/06		Judith M			  Class for ssl interface
//========   ==============   ===========================================

#include "SSLInterface.h"

#include <fstream>
#include <openssl/rand.h>
#include <openssl/pkcs12.h>

#include <openssl/pem.h>
#include <openssl/conf.h>

#include <openssl/x509v3.h>


#include "Trace.h"
#include "TraceStream.h"
#include "StructTm.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "OsFileIF.h"
#include "SslFunc.h"
#include "InternalProcessStatuses.h"
#include "CertificateRequest.h"
#include "CertMngrProcess.h"

#include "SysConfig.h"
#include "SysConfigKeys.h"
#include <algorithm>
#include <sys/stat.h>

int CSslInterface::CreateSslPrivateKey(EVP_PKEY** pkey,
                                       BOOL bForCS/*=FALSE*/,
                                       std::string folder_name/*=""*/)
{
	std::string temp_key_file_name;

	if (bForCS == TRUE)
	{
		if (folder_name=="")
			temp_key_file_name = TEMP_KEYF_FOR_CS;
		else
			temp_key_file_name = HOME_CS + folder_name + "/temp_private_for_cs.pem";
	}
	else
		temp_key_file_name = TEMP_KEYF;
	
	*pkey = EVP_PKEY_new();
	if (!*pkey)
	{
		CSslFunctions::print_err_trace("CSslInterface::CreateSslPrivateKey: error in creating private key");
		return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
	}

	BIO* bio_out;
	if ((bio_out = BIO_new(BIO_s_file())) == NULL)
	{
		CSslFunctions::print_err_trace("CSslInterface::CreateSslPrivateKey: error in creating BIO");
		return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
	}

	if (BIO_write_filename(bio_out,(char*)temp_key_file_name.c_str()) <= 0) 
	{
		CreateDirectory(HOME.c_str());
		if (BIO_write_filename(bio_out,(char*)temp_key_file_name.c_str()) <= 0) 
		{
            BIO_free(bio_out);
		    CSslFunctions::print_err_trace("CSslInterface::CreateSslPrivateKey: unable to write BIO");
		    return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
		}
	}

	RSA* rsa_private = RSA_generate_key (2048, RSA_F4, NULL, NULL);
    if (NULL == rsa_private)
	{
        BIO_free(bio_out);
        CSslFunctions::print_err_trace("CSslInterface::CreateSslPrivateKey: unable to generate key");
        return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
	}
    
 	EVP_PKEY_set1_RSA(*pkey, rsa_private);
	
 	if (bForCS)
 	{
	 	if (!PEM_write_bio_PrivateKey (bio_out,
	                                   *pkey,
	                                   NULL,
	                                   NULL,
	                                   0,
	                                   NULL,
	                                   NULL))
	 	{
            RSA_free(rsa_private);
            BIO_free(bio_out);
	 	    CSslFunctions::print_err_trace("CSslInterface::CreateSslPrivateKey: error in writing private key");
	 	    return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
	 	} 		
 	}
 	else
 	{
 	    std::string pass;
 	    STATUS stat = CSslFunctions::GetSSLPassPhrase(SSL_PASS_PHRASE_DIALOG_LINK.c_str(), pass);
 	    if (STATUS_OK != stat)
 	        return stat;

	 	//Write a private key to a BIO using triple DES encryption
	 	if (!PEM_write_bio_PrivateKey (bio_out,
	                                   *pkey,
	                                   EVP_des_ede3_cbc(),
	                                   NULL,
	                                   0,
	                                   NULL,
	                                   (void*)pass.c_str()))
	 	{
	 	    RSA_free(rsa_private);
            BIO_free(bio_out);
	 	    CSslFunctions::print_err_trace("CSslInterface::CreateSslPrivateKey: error in writing private key");
	 	    return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
	 	}
 	}
 	
	RSA_free(rsa_private);
	BIO_free(bio_out);
	chmod(temp_key_file_name.c_str(), S_IRUSR|S_IWUSR|S_IRGRP);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
int CSslInterface::CreateSslCertificateRequest(CCertificateRequest* cert_req,
                                               BOOL bForCS/*=FALSE*/)
{
	X509_REQ *req=NULL;
	EVP_PKEY *pkey=NULL;
	BIO *out=NULL;
	int i=0;
	WORD status = STATUS_OK;
	
	std::string temp_cert_req_file_name;
	
	std::string folder_name="";
		
	if (bForCS)
	{
		folder_name = GetFolderName(cert_req->GetServiceName());
		if (folder_name=="" && cert_req->GetServiceName()!="")
		{
			status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
			return status;
		}
		
		FTRACESTR(eLevelInfoNormal) << "CSslInterface::CreateSslCertificateRequest - folder_name = "<<folder_name;

		CSslInterface::DeleteCertificateTempFilesForCS();
		
		if (folder_name=="")
			temp_cert_req_file_name = TEMP_CERT_REQ_F_FOR_CS;
		else
			temp_cert_req_file_name = HOME_CS + folder_name + "/temp_cert_for_cs.csr";
		
		FTRACESTR(eLevelInfoNormal) << "CSslInterface::CreateSslCertificateRequest - temp_cert_req_file_name = "<<temp_cert_req_file_name;
	}
	else
	{
		CSslInterface::DeleteCertificateTempFiles();
		temp_cert_req_file_name = TEMP_CERT_REQ_F;
	}

	//create the private key
	status = CSslInterface::CreateSslPrivateKey(&pkey, bForCS, folder_name);

	if (status!=STATUS_OK)
	{
		CloseAll(pkey, req, out);
		return status;
	}
	
	if (!pkey)
	{
		CSslFunctions::print_err_trace("CSslInterface::CreateSslCertificateRequest: error in creating private key");
		status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
		CloseAll(pkey, req, out);
		return status;
	}

	req=X509_REQ_new();
	if (req == NULL)
	{
		CSslFunctions::print_err_trace("CSslInterface::CreateSslCertificateRequest: error in creating certificate request");
		status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
		CloseAll(pkey, req, out);
		return status;
	}

	status=CSslInterface::make_REQ(req,pkey,cert_req);
	if (status!=STATUS_OK)
	{
		CSslFunctions::print_err_trace("CSslInterface::CreateSslCertificateRequest: error in creating certificate request - one of the fields are illegal");
		CloseAll(pkey, req, out);
		return status;
	}

	if (cert_req->GetHashMethod() == eHashMethodSHA1)
	{
		if (!(i=X509_REQ_sign(req,pkey,EVP_sha1())))
		{
			CSslFunctions::print_err_trace("CSslInterface::CreateSslCertificateRequest: SHA1 error in signing certificate request");
			status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
			CloseAll(pkey, req, out);
			return status;
		}
	}
	else //sha256
	{
		if (!(i=X509_REQ_sign(req,pkey,EVP_sha256())))
		{
			CSslFunctions::print_err_trace("CSslInterface::CreateSslCertificateRequest: SHA256 error in signing certificate request");
			status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
			CloseAll(pkey, req, out);
			return status;
		}
	}

	out=BIO_new(BIO_s_file());
	if (out == NULL)
	{
		CSslFunctions::print_err_trace("CSslInterface::CreateSslCertificateRequest: unable to create BIO");
		status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
		CloseAll(pkey, req, out);
		return status;
	}
	
	//create csr (certificate request) file 
	i=(int)BIO_write_filename(out,(char*)temp_cert_req_file_name.c_str());

	if (!i)
	{
		CSslFunctions::print_err_trace("CSslInterface::CreateSslCertificateRequest: unable to write BIO");
		status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
		CloseAll(pkey, req, out);
		return status;
	}

	i=PEM_write_bio_X509_REQ_NEW(out,req);
	if (!i)
	{
		CSslFunctions::print_err_trace("CSslInterface::CreateSslCertificateRequest: unable to write x509 request");
		status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
		CloseAll(pkey, req, out);
		return status;
	}

	CloseAll(pkey, req, out);
	
	SyncMedia(FALSE);	// sync the private key and certificate request files on the disk */

	return status;
}
/////////////////////////////////////////////////////////////////////
void CSslInterface::DeleteCertificateTempFiles()
{
	remove(TEMP_KEYF.c_str());
	remove(TEMP_CERTF.c_str());
	remove(TEMP_CERT_REQ_F.c_str());
}
/////////////////////////////////////////////////////////////////////
void CSslInterface::DeleteCertificateTempFilesForCS()
{
	remove(TEMP_KEYF_FOR_CS.c_str());
	remove(TEMP_CERTF_FOR_CS.c_str());
	remove(TEMP_CERT_REQ_F_FOR_CS.c_str());
}
/////////////////////////////////////////////////////////////////////
void CSslInterface::CloseAll(EVP_PKEY* pkey, X509_REQ* req, BIO* out)
{
	EVP_PKEY_free(pkey);
	X509_REQ_free(req);
	BIO_free_all(out);
}
/////////////////////////////////////////////////////////////////////
int CSslInterface::make_REQ(X509_REQ *req, EVP_PKEY *pkey, CCertificateRequest* cert_req)
{
	int status = STATUS_OK;
	
	/* setup version number */
	if (!X509_REQ_set_version(req,0L))
		status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
	
	status = CSslInterface::auto_info(req, cert_req);

	if (status == STATUS_OK)
	{
		if (!X509_REQ_set_pubkey(req,pkey))
			status = STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
	}
	
	return status;


}




int add_ext(STACK_OF(X509_EXTENSION) *sk, int nid, char *value)
	{
	X509_EXTENSION *ex;
	ex = X509V3_EXT_conf_nid(NULL, NULL, nid, value);
	if (!ex)
		return 0;
	sk_X509_EXTENSION_push(sk, ex);

	return 1;
	}



/////////////////////////////////////////////////////////////////////
int CSslInterface::auto_info(X509_REQ *req, CCertificateRequest* cert_req)
{
	FTRACESTR(eLevelInfoNormal) << "CSslInterface::auto_info";

	X509_NAME *subj;
	
	subj = X509_REQ_get_subject_name(req);
	
	//Country Name (2 letter code)
	if (!X509_NAME_add_entry_by_txt(subj, "C", MBSTRING_ASC,
		(unsigned char*)(unsigned char*)cert_req->GetCountryName().c_str(), -1, -1, 0))
	{
		CSslFunctions::print_err_trace("CSslInterface::auto_info - Country name has an error");
		return STATUS_SSL_ILLEGAL_COUNTRY_NAME;//Error
	}

	//State or Province Name (full name)
	if (!X509_NAME_add_entry_by_txt(subj, "ST", MBSTRING_ASC,
		(unsigned char*)cert_req->GetStateOrProvince().c_str(), -1, -1, 0))
	{
		CSslFunctions::print_err_trace("CSslInterface::auto_info - State or Province Name has an error");
		return STATUS_SSL_ILLEGAL_STATE_OR_PROVINCE; //Error
	}
	
	//Locality Name (eg, city)
	if (!X509_NAME_add_entry_by_txt(subj, "L", MBSTRING_ASC,
		(unsigned char*)cert_req->GetLocality().c_str(), -1, -1, 0))
		return STATUS_SSL_ILLEGAL_LOCALITY_NAME;  //Error

	//Organization Name (eg, company)
	if (!X509_NAME_add_entry_by_txt(subj, "O", MBSTRING_ASC,
		(unsigned char*)cert_req->GetOrganization().c_str(), -1, -1, 0))
	{
		CSslFunctions::print_err_trace("CSslInterface::auto_info - Organization Name has an error");
		return STATUS_SSL_ILLEGAL_ORGANIZATION_NAME;   //Error
	}

	//Organizational Unit Name (eg, section)
	string sOrgUnit = cert_req->GetOrganizationUnit();
	size_t found = sOrgUnit.find(';');
	if (found != string::npos) // ';' is found - there are multiple organization units
	{
		FTRACESTR(eLevelInfoNormal) << "CSslInterface::auto_info - multiple organization units";

		STATUS multipleUnitsStat = HandleMultipleOrganizationUnits(req, cert_req, subj, sOrgUnit);
		if (STATUS_OK != multipleUnitsStat)
		{
			return multipleUnitsStat;
		}
	}
	else // ';' not found - there is a single organization unit
	{
		FTRACESTR(eLevelInfoNormal) << "CSslInterface::auto_info - single organization unit: " << cert_req->GetOrganizationUnit().c_str();

		if (!X509_NAME_add_entry_by_txt(subj, "OU", MBSTRING_ASC,
			(unsigned char*)cert_req->GetOrganizationUnit().c_str(), -1, -1, 0))
		{
			CSslFunctions::print_err_trace("CSslInterface::auto_info - Organizational Unit Name has an error");
			return STATUS_SSL_ILLEGAL_ORGANIZATION_UNIT_NAME;  //Error
		}
	}

	//Common Name (eg, YOUR name)
	if (!X509_NAME_add_entry_by_txt(subj, "CN", MBSTRING_ASC,
		(unsigned char*)cert_req->GetCommonName().c_str(), -1, -1, 0))
	{
		CSslFunctions::print_err_trace("CSslInterface::auto_info - Common Name has an error");
		//return STATUS_SSL_ILLEGAL_COMMON_NAME;  //Error
		//VNGFE-8606 change policy on certificates on common name
		if(!cert_req->GetCommonName().empty())
		{
			return STATUS_SSL_ILLEGAL_COMMON_NAME;
		}
	}
	



	char * pch=NULL;
	//char * tmpString = (char *)(cert_req->GetDomainName().c_str());
	int strLength = cert_req->GetSubjectAlternateName().length();
	if(strLength < 1)
		return STATUS_OK;

	char tmpString[strLength];
	memset(tmpString,0,strLength);
	strcpy(tmpString,(char *)(cert_req->GetSubjectAlternateName().c_str()));

	//std::string domainName;
	//domainName.assign(cert_req->GetDomainName().c_str(),cert_req->GetDomainName().size()); //= cert_req->GetDomainName();

	FTRACESTR(eLevelInfoNormal) << "CSslInterface::auto_info -  SubjectAlternateName"<< tmpString;

	if (tmpString !=NULL)
	{
		std::list<std::string> sanfieldsList;
		std::list<knownAlterNames> altNamesKnown;
		BuildKnownListAltNames(altNamesKnown);
		ParseSANFields(tmpString,sanfieldsList,altNamesKnown);

			STACK_OF(X509_EXTENSION) *exts = NULL;
			exts = sk_X509_EXTENSION_new_null();
			/* Standard extenions*/


			std::list<std::string>::iterator it ;
			DWORD status = 0;
			BOOL faliure = 0;
			for(it =sanfieldsList.begin();it !=sanfieldsList.end();it++  )
			{
				status = add_ext(exts, NID_subject_alt_name, (char *)(*it).c_str());
				if(!status)
				{
					faliure = 1;
					FTRACESTR(eLevelInfoNormal) << "CSslInterface::auto_info - failed to  add to subject alternative name" << (*it).c_str()
									<< "  add_ext status=" << status;
				}
			}
			/* This is a typical use for request extensions: requesting a value for
						 * subject alternative name.
						 */
			//add UPN
			//add_ext(exts, NID_subject_alt_name, (char *)sCSRSansUPN.c_str()); // example for UPN  "otherName:1.3.6.1.4.1.311.20.2.3;UTF8:PolycomMCU@JITCIL.ENG"
			//add_ext(exts, NID_subject_alt_name, "IP:17.22.22.10");



			/* Now we've created the extensions we add them to the request*/

			X509_REQ_add_extensions(req, exts);

			sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);

			if(faliure)
			{
				return STATUS_SSL_ILLEGAL_SUBJECT_ALT_NAME;
			}

	}


	return STATUS_OK;
}
void CSslInterface::BuildKnownListAltNames(std::list<knownAlterNames>& knowNames)
{
	knownAlterNames item;
			item.first = "DNS Name";
			item.second="DNS:";
			knowNames.push_back(item);
			item.first = "IP Address";
			item.second="IP:";
			knowNames.push_back(item);
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			std::string sCSRSansPrefix;
			sysConfig->GetDataByKey("CSR_SANS_PREFIX", sCSRSansPrefix);
			if (sCSRSansPrefix.size() > 0)
			{
				item.first = "Principal Name";
				item.second=sCSRSansPrefix;
				knowNames.push_back(item);
			}
			else
				FTRACESTR(eLevelInfoNormal) << "CSslInterface::CreateSslCertificateRequest - CSR_SANS_PREFIX is empty ";
}

void CSslInterface::ParseSANFields(char *pBuffStr,std::list<std::string>& sanlist,std::list<knownAlterNames>& knowNames)
{
	std::list<knownAlterNames>::iterator it;
	char * pch=NULL;
	pch = strtok (pBuffStr,",");
	std::string sanvalues;
	while (pch != NULL)
	{
		size_t index=0;
		std::string stoken  = pch;
		for(it = knowNames.begin();it !=knowNames.end();it++)
		{
			if((index= stoken.find(it->first)) != std::string::npos)
			{
				if((index= stoken.find('=',index +it->first.length())) != std::string::npos)
				{
					std::string value= stoken.substr(index+1);
					value.erase(std::remove_if(value.begin(), value.end(),&::isspace), value.end());
					stoken= it->second +value;
					break;
				}
			}
		}
		sanlist.push_back(stoken);
		pch = strtok (NULL, ",");
	}
}

////////////////////////////////////////////////////////////////////////////////
STATUS CSslInterface::HandleMultipleOrganizationUnits( X509_REQ *req,
													   CCertificateRequest* cert_req,
													   X509_NAME *subj,
													   const std::string sOrgUnit )
{
	FTRACESTR(eLevelInfoNormal) << "CSslInterface::HandleMultipleOrganizationUnits";

	ALLOCBUFFER(cTemp, sOrgUnit.length());
	memcpy(cTemp, sOrgUnit.c_str(), sOrgUnit.length());
	cTemp[sOrgUnit.length() - 1] = '\0';

	char *cSingleOrgUnit = strtok (cTemp, ";");
	while (cSingleOrgUnit != NULL)
	{
		FTRACESTR(eLevelInfoNormal) << "CSslInterface::HandleMultipleOrganizationUnits - unit: " << cSingleOrgUnit;

		if (!X509_NAME_add_entry_by_txt(subj, "OU", MBSTRING_ASC,
			(unsigned char*)cSingleOrgUnit, -1, -1, 0))
		{
			CSslFunctions::print_err_trace("CSslInterface::HandleMultipleOrganizationUnits - Organizational Unit Name has an error");

			DEALLOCBUFFER(cTemp);
			return STATUS_SSL_ILLEGAL_ORGANIZATION_UNIT_NAME;  //Error
		}

		cSingleOrgUnit = strtok (NULL, ";");
	}


	DEALLOCBUFFER(cTemp);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// static

STATUS CSslInterface::SaveSslCertificateEx(const char* certificate,
                                         const char* host_name,
                                         eCertificateType type,std::string folder_name)
{
	char * oldTZ = getenv("TZ");
	if(eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		putenv("TZ=UTC");
		tzset();
	}

	STATUS status= SaveSslCertificate(certificate,host_name,type, folder_name);

	if(eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		char* newTZ = getenv("TZ");
		if(oldTZ == NULL)
		{
			putenv("TZ=");
		}
		else
		{
			char buff[255];
			memset(buff, 0, sizeof(buff));
			if(strlen(oldTZ) <= sizeof(buff) - 4)
			{
				snprintf(buff, sizeof(buff), "TZ=%s", oldTZ);
				putenv(buff);
			}
			//else
				//PASSERT(1);
		}
		tzset();
	}
	return status;
}


STATUS CSslInterface::SaveSslCertificate(const char* certificate,
                                         const char* host_name,
                                         eCertificateType type, std::string folder_name/*=""*/)
{

    CCertMngrProcess* process = ((CCertMngrProcess*)CProcessBase::GetProcess());
    FPASSERTMSG_AND_RETURN_VALUE(!process, "Unable to continue", STATUS_FAIL);

	FILE* fp;
	int status = STATUS_OK;
	
	std::string temp_cert_file_name;
	std::string temp_key_file_name;
	std::string private_key_file_name;
	std::string certificate_file_name;
	
	switch (type)
	{
	case eOCS:
		if (folder_name=="")
		{
			temp_cert_file_name = TEMP_CERTF_FOR_CS;
			temp_key_file_name = TEMP_KEYF_FOR_CS;
			private_key_file_name = KEYF_FOR_CS;
			certificate_file_name = CERTF_FOR_CS;
		}
		else	//for multiple_services
		{
			temp_cert_file_name = HOME_CS + folder_name + "/temp_cert_off_for_cs.pem";
			temp_key_file_name = HOME_CS + folder_name + "/temp_private_for_cs.pem";
			private_key_file_name = HOME_CS + folder_name + "/pkey.pem";
			certificate_file_name = HOME_CS + folder_name + "/cert.pem";
		}
	    break;

	case eCertificatePersonal:
		temp_cert_file_name = TEMP_CERTF;
		temp_key_file_name = TEMP_KEYF;
		private_key_file_name = KEYF_DES3;
		certificate_file_name = CERTF;
	    break;

	case eCertificateTrust:
	    // TOREMOVE (drabkin) NOT SUPPORTED HERE
	    //Jud - TBD - add file name creator
    	//temp_cert_file_name = TEMP_CA_CERTIFICATE;
	    //break;

	default:
	    FPASSERTSTREAM(true,
	        "Unable to continue with " << CertificateTypeToStr(type));
	    return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE;
	}
	
	if ((fp=fopen(temp_cert_file_name.c_str(),"w+"))==NULL)
		return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE;//return status - file csr wasn't create
	
	fputs(certificate, fp);

	fflush(fp);
	fclose(fp);

	SSL_METHOD *meth;
	SSL_CTX* ctx;

	meth = (SSL_METHOD*)SSLv23_server_method();
	ctx = SSL_CTX_new (meth);

	if (!ctx)
	{
		CSslFunctions::print_err_trace("CSslInterface::SaveSslCertificate: error in creating ctx");
		return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE;
	}

	if (SSL_CTX_use_certificate_file(ctx, temp_cert_file_name.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		std::string error_str = "CSslInterface::SaveSslCertificate: error in reading certificate - " + temp_cert_file_name;
		CSslFunctions::print_err_trace((char*)error_str.c_str());
		return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE;
	}

	if (type == eCertificatePersonal)
		SSL_CTX_set_default_passwd_cb(ctx, CSslFunctions::pem_passwd_cb);


	if (SSL_CTX_use_PrivateKey_file(ctx, temp_key_file_name.c_str(), SSL_FILETYPE_PEM) <= 0)
  {
    std::string error_str = "CSslInterface::SaveSslCertificate: error in reading private key - " + temp_key_file_name;
    CSslFunctions::print_err_trace((char*)error_str.c_str());
    return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE;
  }

  if (!SSL_CTX_check_private_key(ctx))
  {
    //Private key does not match the certificate public key
    CSslFunctions::print_err_trace("CSslInterface::SaveSslCertificate: ssl certificate does not match the private key");
    return STATUS_SSL_CERTIFICATE_DOES_NOT_MATCH_THE_PRIVATE_KEY;
  }

	STATUS retStatus = STATUS_OK;

	// return status - file csr wasn't create
	if ((fp=fopen(temp_cert_file_name.c_str(),"r"))==NULL)
		return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE;

	X509* cert = NULL;
	
	if (!(cert = PEM_read_X509(fp, NULL, NULL, NULL)))
	{
		fclose(fp);
		CSslFunctions::print_err_trace("CSslInterface::SaveSslCertificate - Error reading client certificate file");
		return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
	}

/*
    std::string cert_str;
	if (eCertificateTrust == type)
	{
		rewind (fp);
		char szFileLine[100];
		while(fgets(szFileLine,99,fp))
			cert_str+=szFileLine;
	}	
*/
	
	fclose(fp);


	//check certificate expired date
	ASN1_TIME* expirationDate = CSslFunctions::GetCertificateExpirationDate(cert);
	int diff_days=0;
	retStatus = CSslFunctions::CheckCertificateExpirationDate(expirationDate, NULL, diff_days);
	
	if (retStatus!=STATUS_OK)
		return retStatus;
	
	//check certificate not before date
	ASN1_TIME* startDate = CSslFunctions::GetCertificateStartDate(cert);
	retStatus = CSslFunctions::CheckCertificateStartDate(startDate, NULL);
	
	if (retStatus!=STATUS_OK)
		return retStatus;
	
	if (type == eCertificatePersonal)
	{
		/******* due to BRIDGE-5107 the check of dns common name is disablled ***********
		//check certificate DNS - the CS check the DNS by them self
		//retStatus = CSslFunctions::CheckCertificateCommonName(cert, host_name);
		
		//if (retStatus!=STATUS_OK)
		//	return retStatus;
		********************************************************************************/
		//des3 private key issue doesn't exist in CS certificate
		
		//remove old files and rename all the temp files. 
		remove(KEYF.c_str());
		remove(CERTF.c_str());
	}
	else if (type == eOCS)
	{
		//remove old OCS files
		remove(private_key_file_name.c_str());
		remove(certificate_file_name.c_str());

		string file_name = HOME_CS + folder_name + "/rootCA.pem";
		remove(file_name.c_str());
		file_name = HOME_CS + folder_name + "/certPassword.txt";
		remove(file_name.c_str());
		file_name = HOME_CS + folder_name + "/pfxCert.pfx";
		remove(file_name.c_str());
	}
	// check if it is CA certificate - TBD - make better check if the CA is really a CA
/*
	if (eCertificateTrust == type)
	{
		CSslFunctions::print_trace("CSslInterface::SaveSslCertificate: check if it is CA certificate");
		CSslFunctions::CheckCertificateAuthority(cert);
	}
*/
	if (eCertificateTrust != type)
	{

		rename(temp_key_file_name.c_str(), private_key_file_name.c_str());
		rename(temp_cert_file_name.c_str(), certificate_file_name.c_str());

		if (eCertificatePersonal == type || eOCS == type)
		{
		    retStatus = process->AddCertificate(type, certificate_file_name.c_str());
		    if (STATUS_OK != retStatus)
		    {
		        return retStatus;
		    }
		}

	}
/*
 	else
	{
		CSslFunctions::print_trace("CSslInterface::SaveSslCertificate: add the certificate to the CA list in ca-bundle-client.crt file");
		retStatus = process->AddCertificate(eCertificateTrust, TEMP_CA_CERTIFICATE);
		if (retStatus != STATUS_OK)
			return retStatus;
	}
*/
	
	SyncMedia(FALSE);
	
	return status;
}

/////////////////////////////////////////////////////////////////////
static void callback(int p, int n, void *arg)
{
	char c='B';

	if (p == 0) c='.';
	if (p == 1) c='+';
	if (p == 2) c='*';
	if (p == 3) c='\n';
	fputc(c,stderr);
}

void CSslInterface::ConvertPrivateKey()
{

  CTraceStream                    logger(__FILE__, __LINE__, eLevelInfoNormal, NULL); logger.seekp(0, std::ios_base::cur);
  logger << "Convert the private key to PKCS8" << endl;
  std::string pass;
  STATUS stat = CSslFunctions::GetSSLPassPhrase(SSL_PASS_PHRASE_DIALOG_LINK.c_str(), pass);
  if (STATUS_OK != stat)
    return;

  const int KeyfileSize = GetFileSize(KEYF);
  const int KeyDes3fileSize = GetFileSize(KEYF_DES3);

  logger << "ConvertPrivateKey - KeyfileSize " << KeyfileSize << " KeyDes3fileSize " << KeyDes3fileSize;

   //new private key exist or no private key- fails at soft_mcu
  if (KeyDes3fileSize!=-1 || (KeyfileSize==-1 && KeyDes3fileSize==-1))
			return;

	//convert the private key to PKCS8
	//replace the openssl command: openssl pkcs8 -v1 PBE-SHA1-3DES -topk8 -in private.pem -out private3.pem
	EVP_PKEY* pkey = NULL;
	BIO* out = NULL;
	BIO* bio_err = NULL;;
	PKCS8_PRIV_KEY_INFO* p8inf;
	
	if (bio_err == NULL) bio_err = BIO_new_fp (stderr, BIO_NOCLOSE);
	
	//yaela consider move it   to init
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	
	if (!(out = BIO_new_file (KEYF_DES3.c_str(), "wb")))
	{
	    FTRACEINTO << "CSslInterface::ConvertPrivateKey - can't open output file: " << KEYF_DES3;
		return;
	}
	BIO* key = BIO_new(BIO_s_file());
	if (key == NULL)
	{
	    FTRACEINTO << "CSslInterface::ConvertPrivateKey - can't BIO_new key file";
		return;
	}

	if (BIO_read_filename(key, KEYF.c_str()) <= 0)
	{
		CSslFunctions::print_err_trace("CSslInterface::ConvertPrivateKey - unable to read file");
		BIO_free(key);
		return;
	}

	PW_CB_DATA cb_data;
	cb_data.password = pass.c_str();
	cb_data.prompt_info = KEYF.c_str();
	
	pkey = PEM_read_bio_PrivateKey(key, NULL, CSslFunctions::pem_passwd_cb, &cb_data);
	
	if (!pkey)
	{
	    CSslFunctions::print_err_trace("CSslInterface::ConvertPrivateKey - unable to read private key file");
		BIO_free_all(out);
		return;
	}

	if (!(p8inf = EVP_PKEY2PKCS8_broken(pkey, PKCS8_OK)))
	{		
		CSslFunctions::print_err_trace("CSslInterface::ConvertPrivateKey - Error converting key");
		EVP_PKEY_free(pkey);
		BIO_free_all(out);
		return;
	}

	X509_SIG *p8;
	int pbe_nid = NID_pbe_WithSHA1And3_Key_TripleDES_CBC;
	int iter = PKCS12_DEFAULT_ITER;
	
	if (!(p8 = PKCS8_encrypt(pbe_nid, NULL,
                             pass.c_str(), pass.length(),
                             NULL, 0, iter, p8inf)))
	{
		CSslFunctions::print_err_trace("CSslInterface::ConvertPrivateKey - Error encrypting key");
		PKCS8_PRIV_KEY_INFO_free(p8inf);
		EVP_PKEY_free(pkey);
		BIO_free_all(out);
		return;
	}
	PEM_write_bio_PKCS8(out, p8);

	X509_SIG_free(p8);
	PKCS8_PRIV_KEY_INFO_free (p8inf);
	EVP_PKEY_free(pkey);
	BIO_free_all(out);
	remove(KEYF.c_str());
    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    BOOL isJITCMode = NO;
    if (sysConfig)
    	sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, isJITCMode);
	if ( YES == IsTarget() ||YES == isJITCMode )
		chmod(KEYF_DES3.c_str(), S_IRUSR|S_IWUSR|S_IRGRP);
	else
		chmod(KEYF_DES3.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
}
std::string CSslInterface::GetFolderName(std::string service_name)
{
	CCertMngrProcess *pCertMngrProcess = (CCertMngrProcess*)CCertMngrProcess::GetProcess();
	
	std::string folder_name="";
	
	int service_id = pCertMngrProcess->GetServiceId(service_name);
	
	if (service_id==0)
		FTRACESTR(eLevelInfoNormal) << "CSslInterface::GetFolderName - service id is 0";
	else
		FTRACESTR(eLevelInfoNormal) << "CSslInterface::GetFolderName - service id is - "<<service_id;
	
	if (service_id==0)
	{
		FTRACESTR(eLevelInfoNormal) << "CSslInterface::GetFolderName - Can't find service - "<<service_name;
		return folder_name;
	}
	
	char str_srv_id[2];
	snprintf(str_srv_id, sizeof(str_srv_id), "%d", service_id);
	
	folder_name = "cs";
	folder_name+= str_srv_id;
	
	return folder_name;
}
/////////////////////////////////////////////////////////////////////
//Available only for pizza
//JudTemp void CSslInterface::CreateSelfSignCertificate()
//{
//	X509 *x;
//	EVP_PKEY *pk;
//	RSA *rsa;
//	X509_NAME *name=NULL;
//	int bits = 512;
//	int serial = 0;
//	int days = 365;
//	
//	if ((pk=EVP_PKEY_new()) == NULL)
//		return;
//
//	if ((x=X509_new()) == NULL)
//		return;
//
//	rsa=RSA_generate_key(bits,RSA_F4,callback,NULL);
//	if (!EVP_PKEY_assign_RSA(pk,rsa))
//		return;
//
//	X509_set_version(x,2);
//	ASN1_INTEGER_set(X509_get_serialNumber(x),serial);
//	X509_gmtime_adj(X509_get_notBefore(x),0);
//	X509_gmtime_adj(X509_get_notAfter(x),(long)60*60*24*days);
//	X509_set_pubkey(x,pk);
//
//	name=X509_get_subject_name(x);
//
//	const unsigned char country[3] = "IL";
//	X509_NAME_add_entry_by_txt(name,"C",MBSTRING_ASC, country, -1, -1, 0);
//	
//	string short_host_name;
//	GetPizzaHostName(short_host_name);
//	
//	X509_NAME_add_entry_by_txt(name,"CN",
//				MBSTRING_ASC, (const unsigned char*)short_host_name.c_str(), -1, -1, 0);
//
//	X509_set_issuer_name(x,name);
//
//	/* Add various extensions: standard extensions */
///*	add_ext(x, NID_basic_constraints, "critical,CA:TRUE");
//	add_ext(x, NID_key_usage, "critical,keyCertSign,cRLSign");
//
//	add_ext(x, NID_subject_key_identifier, "hash");
//
//	/ Some Netscape specific extensions
//	add_ext(x, NID_netscape_cert_type, "sslCA");
//
//	add_ext(x, NID_netscape_comment, "example comment extension");
//
//
//	// Maybe even add our own extension based on existing
//	{
//		int nid;
//		nid = OBJ_create("1.2.3.4", "MyAlias", "My Test Alias Extension");
//		X509V3_EXT_add_alias(nid, NID_netscape_comment);
//		add_ext(x, nid, "example comment alias");
//	}
//*/
//	
//	if (!X509_sign(x,pk,EVP_sha1()))
//		return;
//	
//	BIO *out=NULL;
//	
//	PEM_write_X509(stdout,x);
//	
////	PEM_write_X509(stdout,x509);
//}
/////////////////////////////////////////////////////////////////////
//JudTemp void CSslInterface::GetPizzaHostName(std::string& short_host_name)
//{
//	string system_command = "uname -n";   
//	string answer;
//	char host_name[1025];
//	STATUS status = SystemPipedCommand(system_command.c_str(), answer);
//	if(STATUS_OK != status)
//	{
//	    string message = "Failed to run command : ";
//	    message += system_command;
//	    perror(message.c_str());
//	    return ;//J status;
//	}
//	else
//	{
//		strcpy(host_name, answer.c_str());
//		host_name[answer.length() - 1] = '\0'; // removes \n
//		
//		int nEndOfShortName = answer.find_first_of('.');
//		if (nEndOfShortName!=-1)
//		{
//			strncpy(host_name, answer.c_str(), nEndOfShortName);
//			host_name[nEndOfShortName]='\0';
//		}
//		short_host_name = host_name;
//	}
//}
/////////////////////////////////////////////////////////////////////
