/*
 * CurlHTTP.cpp
 *
 *  Created on: Jun 8, 2009
 *      Author: kobi
 */

#include <algorithm>
#include "CurlHTTP.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include "Trace.h"
#include "TraceStream.h"
#include "SysConfig.h"

CurlHTTP::CurlHTTP(bool bIsSecured/*=false*/,bool bSkipPeerVerification/*=false*/,bool bSkipHostNameVerfication/*=false*/)
{
	m_bIsSecured = bIsSecured;
	m_pstrContent = new std::string();
	m_bIsNTLMAuth = false;
	m_userNameAndPasswordNTLM = NULL;
	m_pcurl = NULL;
	/*
	       * If you want to connect to a site who isn't using a certificate that is
	       * signed by one of the certs in the CA bundle you have, you can skip the
	       * verification of the server's certificate. This makes the connection
	       * A LOT LESS SECURE.
	       *
	       * If you have a CA cert for the server stored someplace else than in the
	       * default bundle, then the CURLOPT_CAPATH option might come handy for
	       * you.
	       */
	m_bSkipPeerVerification = bSkipPeerVerification;

	/*
	       * If the site you're connecting to uses a different host name that what
	       * they have mentioned in their server certificate's commonName (or
	       * subjectAltName) fields, libcurl will refuse to connect. You can skip
	       * this check, but this will make the connection less secure.
	       */
	m_bSkipHostNameVerfication = bSkipHostNameVerfication;
	m_pcurl =  curl_easy_init();

	m_pszErrorBuffer = new char[CURL_ERROR_SIZE+1];
	memset(m_pszErrorBuffer,0,CURL_ERROR_SIZE+1);

	m_bIsSetSourceIF = false;
	m_timeout = 5;
	//struct data config;
//	config.trace_ascii = 1; /* enable ascii tracing */
	/*output the debug info*/
//    curl_easy_setopt(m_pcurl, CURLOPT_DEBUGFUNCTION, my_trace);
//    curl_easy_setopt(m_pcurl, CURLOPT_DEBUGDATA, &config);

    /* the DEBUGFUNCTION has no effect until we enable VERBOSE */
//	CURLcode res = curl_easy_setopt(m_pcurl, CURLOPT_VERBOSE, 1L);
//    if (res!=CURLE_OK)
//    {
//    	printf("DEBUG CurlHTTP Constructor >>>>> res=%d\n",(int)res);
//    }
}


bool CurlHTTP::Sendrequest(CURLcode& retCode,std::string strUrl,bool grabHeaders/* = true*/, bool grabUrl/* = true*/, const HTTP_REQUEST_TYPE method/* = eHttpGet*/)
{
	CURLcode res = CURLE_OK;

	m_pstrContent->clear();
	DATA data =	{ m_pstrContent, grabUrl ,true };
	DATA headers_data = {&m_headers , grabHeaders , true};

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_ERRORBUFFER , m_pszErrorBuffer)) != CURLE_OK )
		goto clean_up;

	AddAditionalConfiguration();
	SetMethodConfiguration(method);

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_URL, strUrl.c_str())) != CURLE_OK )
		goto clean_up;

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_USERAGENT, MY_USR_AGENT)) != CURLE_OK )
		goto clean_up;

	if (m_bIsNTLMAuth)
		SetNtlmAuthenticationParams(m_userName,m_password);

	if ( (res = curl_easy_perform(m_pcurl)) != CURLE_OK )
	{
		if ( grabUrl )
			goto clean_up;
	}

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_WRITEFUNCTION, writefunction)) != CURLE_OK )
		goto clean_up;

	if(grabHeaders)
	{
		if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_HEADERFUNCTION, writefunction)) != CURLE_OK )
			goto clean_up;

		if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_WRITEHEADER, (void *)&headers_data)) != CURLE_OK )
			goto clean_up;
	}

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_WRITEDATA, (void *)&data)) != CURLE_OK )
		goto clean_up;

	if (m_bIsNTLMAuth)
		SetNtlmAuthenticationParams(m_userName,m_password);

	if ( (res = curl_easy_perform(m_pcurl)) != CURLE_OK )
	{
		if ( grabUrl )
			goto clean_up;
	}

	return true;

clean_up:
	// error string
	std::stringstream streamMsg;
	streamMsg 	<< "Curl error: " << __FILE__ << ":" << __LINE__
				<< "; error: " << m_pszErrorBuffer
				<< "; retVal=" << res
				<< ";";
	FPTRACE(eLevelInfoNormal,streamMsg.str().c_str());

	//printf("(%s %d) error: %s; retVal=%d", __FILE__,__LINE__, pszError,res);

	return false;
}
bool CurlHTTP::PostRequest(CURLcode& res, const std::string& strUrl,
	    const std::string& strRequest,
		const bool grabHeaders /*= true*/, const bool grabUrl /*= true*/)
{
//	FPTRACE2(eLevelInfoNormal,"CurlSoap::PostSoapRequest=",strRequest.c_str());

	res = CURLE_OK;
	std::string::size_type start_pos;
	DATA data;
	data.bGrab = grabUrl;
	data.pstr =	m_pstrContent;
	data.bClearString = true;
	DATA headers_data = {&m_headers , grabHeaders, true};

	struct curl_slist* headers = NULL;
	int reason = 0;

	char * buffer = new char[strRequest.length()+1];
	memset(buffer,'\0',strRequest.length()+1);
	strncpy(buffer,strRequest.c_str(),strRequest.length());

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_ERRORBUFFER , m_pszErrorBuffer)) != CURLE_OK )
	{
		reason = 1;
		goto clean_up;
	}

	AddAditionalConfiguration();
	SetMethodConfiguration(eHttpPost);

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_POSTFIELDS, buffer)) != CURLE_OK )
	{
		reason = 2;
		goto clean_up;
	}
	headers = curl_slist_append(headers, "Content-Type: text/xml; charset=utf-8");
//	headers = curl_slist_append(headers, "Expect:100-continue");

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_HTTPHEADER , headers)) != CURLE_OK )
	{
		reason = 3;
		goto clean_up;
	}

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_TIMEOUT , /*5*/m_timeout)) != CURLE_OK )
	{
		reason = 4;
		goto clean_up;
	}

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_URL, strUrl.c_str())) != CURLE_OK )
	{
		reason = 5;
		goto clean_up;
	}

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_USERAGENT, MY_USR_AGENT)) != CURLE_OK )
	{
		reason = 6;
		goto clean_up;
	}

	if (m_bIsNTLMAuth)
		SetNtlmAuthenticationParams(m_userName,m_password);

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_POSTFIELDSIZE, strlen(buffer))) != CURLE_OK )
	{
		reason = 7;
		goto clean_up;
	}

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_WRITEFUNCTION, writefunction)) != CURLE_OK )
	{
		reason = 9;
		goto clean_up;
	}

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_WRITEDATA, (void *)&data)) != CURLE_OK )
	{
		reason = 10;
		goto clean_up;
	}

	if(grabHeaders)
	{
		if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_HEADERFUNCTION, writefunction)) != CURLE_OK )
		{
			reason = 11;
			goto clean_up;
		}

		if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_WRITEHEADER, (void *)&headers_data)) != CURLE_OK )
		{
			reason = 12;
			goto clean_up;
		}
	}

	if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_NOSIGNAL , 1L)) != CURLE_OK )
	{
		reason = 13;
		goto clean_up;
	}

	if(m_bIsSetSourceIF)
	{
		if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_INTERFACE , m_sourceIF.c_str())) != CURLE_OK )
			{
				reason = 14;
				goto clean_up;
			}
	}
	if( true )
	{
		sighandler_t  tOldSignalHandler = signal(SIGALRM,SIG_IGN);
//		FPTRACE(eLevelInfoNormal,"CurlSoap::PostSoapRequestFromFile - change signal to SIG_IGN");

		if ( (res = curl_easy_perform(m_pcurl)) != CURLE_OK )
		{
//			FPTRACE(eLevelInfoNormal,"CurlSoap::PostSoapRequestFromFile - change signal back to old");
			signal(SIGALRM,tOldSignalHandler);

			if ( grabUrl )
			{
				reason = 15;
				goto clean_up;
			}
		}
		else
		{
			signal(SIGALRM,tOldSignalHandler);
			FPTRACE(eLevelInfoNormal,"CurlSoap::PostSoapRequestFromFile - change signal back to old");
		}
	}

	curl_slist_free_all(headers);
	delete [] buffer;

	start_pos = m_headers.find(SOAP_OR_REST_RESPONSE_OK);
	if( std::string::npos == start_pos )
	{
		start_pos = m_headers.find(REST_RESPONSE_CREATED);
		if( std::string::npos == start_pos )
		{
			FPTRACE2(eLevelInfoNormal,"CurlSoap::PostSoapRequestFromFile failure response from Server resp header = %s!",m_headers.c_str());
			return false;
		}
	}
	return true;

clean_up:
	// error string
	std::stringstream streamMsg;
	streamMsg 	<< "Curl error: " << __FILE__ << ":" << __LINE__
				<< "; error: " << m_pszErrorBuffer
				<< "; retVal=" << res
				<< "; reason=" << reason
				<< ";";
	FPTRACE(eLevelInfoNormal,streamMsg.str().c_str());

	if( headers != NULL )
		curl_slist_free_all(headers);
	delete [] buffer;

	return false;
}



CurlHTTP::~CurlHTTP()
{
	delete [] m_pszErrorBuffer;
	m_pszErrorBuffer = NULL;

	curl_easy_cleanup(m_pcurl);
    m_pcurl = NULL;
    if ( m_userNameAndPasswordNTLM != NULL )
    	delete [] m_userNameAndPasswordNTLM;
    delete m_pstrContent;
}

bool CurlHTTP::SetMethodConfiguration(const HTTP_REQUEST_TYPE method)
{
	CURLcode res = CURLE_OK;
	switch(method)
	{
		case eHttpGet:
		{
			if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_HTTPGET, 1L)) != CURLE_OK )
				return false;
			break;
		}
		case eHttpPost:
		{
			if ( (res = curl_easy_setopt(m_pcurl, CURLOPT_HTTPPOST, 1L)) != CURLE_OK )
				return false;
			break;
		}
		case eHttpPut:
		{
			/*
			CURLOPT_PUT

			A  parameter  set  to  1  tells  the library to use HTTP PUT to
              transfer data. The data should be set with CURLOPT_READDATA and
              CURLOPT_INFILESIZE.

              This  option is deprecated and starting with version 7.12.1 you
              should instead use CURLOPT_UPLOAD.

			*/
			break;
		}
	}

	return true;
}

bool CurlHTTP::WriteResponseToFile(std::string strFileName/*="Httpout.html"*/)
{
	//printf("Headers : %s \n", m_headers.c_str());

	FILE *fp = fopen (strFileName.c_str(), "w");

	if(fp)
	{
		fwrite(m_pstrContent->c_str(), sizeof(char) , m_pstrContent->length(), fp);
		fclose(fp);
	}
	else
	{
		printf("Could not open file: out.html!");
		return 0;
	}
	return true;
}

bool CurlHTTP::WriteResponseHeaderToFile(std::string strFileName/*="HttpHeadersout.txt"*/)
{
	FILE *fp = fopen (strFileName.c_str(), "w");

	if(fp)
	{
		fwrite(m_headers.c_str(), sizeof(char) , m_headers.length(), fp);
		fclose(fp);
	}
	else
	{
		printf("Could not open file: %s!",strFileName.c_str() );
		return 0;
	}
	return true;
}
// VNGFE-7269 The name can be an interface name, an IP address, or a host name
void CurlHTTP::SetOutGoingInterface(std::string sourceInterface)
{
	m_bIsSetSourceIF = true;
	m_sourceIF = sourceInterface;
}
bool CurlHTTP::SetNtlmAuthenticationParams(std::string userName,std::string password)
{
	if (m_bIsNTLMAuth==false) //we want to set this option only Once
	{
		m_userName = userName;
		m_password = password;

		std::string userNamePW = m_userName + ":" + m_password;

		if ( m_userNameAndPasswordNTLM != NULL )
			delete [] m_userNameAndPasswordNTLM;

		m_userNameAndPasswordNTLM = new char[userNamePW.length()+1];
		m_userNameAndPasswordNTLM[userNamePW.length()] = '\0';
		strncpy(m_userNameAndPasswordNTLM,userNamePW.c_str(),userNamePW.length());

		CURLcode res = curl_easy_setopt(m_pcurl, CURLOPT_HTTPAUTH, CURLAUTH_NTLM );
		if (res != CURLE_OK)
		{
			printf("DEBUG SetNtlmAuthenticationParams >>>>> res=%d\n",(int)res);
			return false;
		}
		else
			m_bIsNTLMAuth = true;
	}
	if (curl_easy_setopt(m_pcurl, CURLOPT_USERPWD, m_userNameAndPasswordNTLM) != CURLE_OK)
	{
		return false;
	}
	return true;
}
bool CurlHTTP::SetBasicAuthenticationParams(std::string userName,std::string password)
{
	if (m_bIsNTLMAuth==false) //we want to set this option only Once
	{
		m_userName = userName;
		m_password = password;

		std::string userNamePW = m_userName + ":" + m_password;

		if ( m_userNameAndPasswordNTLM != NULL )
			delete [] m_userNameAndPasswordNTLM;

		m_userNameAndPasswordNTLM = new char[userNamePW.length()+1];
		m_userNameAndPasswordNTLM[userNamePW.length()] = '\0';
		strncpy(m_userNameAndPasswordNTLM,userNamePW.c_str(),userNamePW.length());

		CURLcode res = curl_easy_setopt(m_pcurl, CURLOPT_HTTPAUTH,  CURLAUTH_BASIC );
		if (res != CURLE_OK)
		{
			printf("DEBUG SetNtlmAuthenticationParams >>>>> res=%d\n",(int)res);
			return false;
		}
		else
			m_bIsNTLMAuth = true;
	}
	if (curl_easy_setopt(m_pcurl, CURLOPT_USERPWD, m_userNameAndPasswordNTLM) != CURLE_OK)
	{
		return false;
	}
	return true;
}

char* CurlHTTP::LoadSoapBufferFromFile(std::string fileName)
{
	FILE *fp = fopen (fileName.c_str(), "r");
	long lSize;
	char * buffer = NULL;
	if(fp)
	{
		  // obtain file size.
		  fseek (fp , 0 , SEEK_END);
		  lSize = ftell (fp);
		  rewind (fp);

		  // allocate memory to contain the whole file.
		  buffer = new char[lSize + 1];
		  // copy the file into the buffer.
		  fread (buffer,1,lSize,fp);
		  buffer[lSize]='\0';
		  /*** the whole file is loaded in the buffer. ***/

		  // terminate
		  fclose (fp);

	}
	return buffer;
}

void CurlHTTP::AddAditionalConfiguration()
{
	if (m_bIsSecured)
	{
		if (m_bSkipPeerVerification)
		{
			if (curl_easy_setopt(m_pcurl, CURLOPT_SSL_VERIFYPEER, 0L) != CURLE_OK )
				printf("(%s %d) error: %s", __FILE__,__LINE__, m_pszErrorBuffer);
		}
		if (m_bSkipHostNameVerfication)
		{
			if (curl_easy_setopt(m_pcurl, CURLOPT_SSL_VERIFYHOST, 0L) != CURLE_OK )
				printf("(%s %d) error: %s", __FILE__,__LINE__, m_pszErrorBuffer);
		}
	}
}

std::string CurlHTTP::GetCurlStatusAsString(CURLcode status)
{
	std::string result;
	switch(status)
	{
		case CURLE_OK:
			return "OK";
		case CURLE_UNSUPPORTED_PROTOCOL:    /* 1 */
			return "Unsupported protocol";
		case CURLE_FAILED_INIT:             /* 2 */
			return "Init failed";
		case CURLE_URL_MALFORMAT:           /* 3 */
			return "URL malformat";
		case CURLE_COULDNT_RESOLVE_HOST:    /* 6 */
			return "Couldn't resolve host";
		case CURLE_COULDNT_CONNECT:         /* 7 */
			return "No connection to exchange service. Check Exchange server address.";
		case CURLE_QUOTE_ERROR:             /* 21 - quote command failure */
			return "Quote command failure";
		case CURLE_HTTP_RETURNED_ERROR:     /* 22 */
			return "Http returned error";
		case CURLE_READ_ERROR:              /* 26 - couldn't open/read from file */
			return "Couldn't open/read from file";
		case CURLE_OUT_OF_MEMORY:           /* 27 */
			return "Out of memory";
		case CURLE_OPERATION_TIMEDOUT:      /* 28 - the timeout time was reached */
			return "Operation timeout";
		case CURLE_RANGE_ERROR:             /* 33 - RANGE "command" didn't work */
			return "Range error";
		case CURLE_HTTP_POST_ERROR:         /* 34 */
			return "HTTP post error";
		case CURLE_SSL_CONNECT_ERROR:       /* 35 - wrong when connecting with SSL */
			return "SSL connect error";
		case CURLE_SSL_ENGINE_NOTFOUND:     /* 53 - SSL crypto engine not found */
			return "SSL crypto engine not found";
		case CURLE_SSL_ENGINE_SETFAILED:    /* 54 - can not set SSL crypto engine as default */
			return "Can not set SSL crypto engine as default";
		case CURLE_SEND_ERROR:              /* 55 - failed sending network data */
			return "Failed sending network data";
		case CURLE_RECV_ERROR:              /* 56 - failure in receiving network data */
			return "Failure in receiving network data";
		case CURLE_SSL_CERTPROBLEM:         /* 58 - problem with the local certificate */
			return "Problem with the local SSL certificate";
		case CURLE_USE_SSL_FAILED:          /* 64 - Requested FTP SSL level failed */
			return "Requested FTP SSL level failed";
		case CURLE_SSL_ENGINE_INITFAILED:   /* 66 - failed to initialise ENGINE */
			return "Failed to initialise SSL engine";
		case CURLE_LOGIN_DENIED:            /* 67 - user, password or similar was not accepted and we failed to login */
			return "User, password or similar was not accepted and we failed to login";
		case CURLE_AGAIN:                   /* 81 - socket is not ready for send/recv, wait till it's ready and try again (Added in 7.18.2) */
			return "Socket is not ready for send/recv";
		default:
			char szTmp[32];
			sprintf(szTmp,"%d",(int)status);
			result = "Unknown status (";
			result += szTmp;
			result += ")!";
			break;
	}
	return result;
}

bool CurlHTTP::IsSecuredUrl(const std::string& strUrl)
{
	std::string strTemp = strUrl;

	std::transform(strTemp.begin(),strTemp.end(),strTemp.begin(), (int(*)(int))std::tolower); //(int(*)(int))

	if( strTemp.find_first_of("https://") == 0 )
		return true;
	if( strTemp.find(":443/") != std::string::npos )
		return true;

	return false;
}
