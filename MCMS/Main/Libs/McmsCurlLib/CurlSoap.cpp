/*
 * CurlSoap.cpp
 *
 *  Created on: Jun 17, 2009
 *      Author: kobi
 */


#include "CurlSoap.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <curl/curl.h>
#include "Trace.h"
#include "TraceStream.h"
#include "ProcessBase.h"
#include "SysConfig.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
CurlSoap::CurlSoap(bool bIsSecured/*=false*/,bool bSkipPeerVerification/*=false*/,bool bSkipHostNameVerfication/*=false*/)
	: CurlHTTP(bIsSecured,bSkipPeerVerification,bSkipHostNameVerfication)
{
	m_bFirstSendTrans = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CurlSoap::~CurlSoap()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CurlSoap::AddAditionalConfiguration()
{
	CurlHTTP::AddAditionalConfiguration();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CurlSoap::PostSoapRequestFromFile(CURLcode& retCode, const std::string& strUrl,
		const std::string& strSoapAction, const std::string& strPostDataFileName,
		const bool grabHeaders /*= true*/, const bool grabUrl /*= true*/)
{
	char * buffer = LoadSoapBufferFromFile(strPostDataFileName.c_str());
	if (buffer==NULL)
	{
		FPTRACE(eLevelInfoNormal,"CurlSoap::PostSoapRequestFromFile failed to load file!");
		return false;
	}
	FPTRACE(eLevelInfoNormal,buffer);
	std::string strRequest = buffer;
	delete[] buffer;

	return PostSoapRequest(retCode,strUrl,strSoapAction,strRequest,grabHeaders,grabUrl);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CurlSoap::PostSoapRequest(CURLcode& res, const std::string& strUrl,
		const std::string& strSoapAction, const std::string& strRequest,
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

	std::string strCombinedSoap = "SOAPAction:" + strSoapAction;

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
	headers = curl_slist_append(headers, strCombinedSoap.c_str());
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

//	if (m_bFirstSendTrans==false) //if riding on the Same Post request there is no point to register the write Function again
//		return true;              // and we use the same connection that has already been NTLM authenticated, in first request
								  // it is required to send the same request time twice due to NTLM Authentication.

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
//			FPTRACE(eLevelInfoNormal,"CurlSoap::PostSoapRequestFromFile - change signal back to old");
		}
	}

	m_bFirstSendTrans = false;

	curl_slist_free_all(headers);
	delete [] buffer;

	start_pos = m_headers.find(SOAP_OR_REST_RESPONSE_OK);
	if( std::string::npos == start_pos )
	{
		FPTRACE2(eLevelInfoNormal,"CurlSoap::PostSoapRequestFromFile failure response from Server resp header = %s!",m_headers.c_str());
		return false;
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

void CurlSoap::ResetCurlLib()
{
	curl_easy_reset(m_pcurl);
}


