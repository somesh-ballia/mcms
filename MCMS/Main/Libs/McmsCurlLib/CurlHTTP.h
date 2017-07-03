/*
 * CurlHTTP.h
 *
 *  Created on: Jun 8, 2009
 *      Author: kobi
 */

#ifndef CURLHTTP_H_
#define CURLHTTP_H_

#include <curl/curl.h>
#include <string>
#include "CurlCommon.h"
#include "ProcessBase.h"

struct data {
    char trace_ascii; /* 1 or 0 */
};

class CurlHTTP{
public:
	CurlHTTP(bool bIsSecured=false,bool bSkipPeerVerification=false,bool bSkipHostNameVerfication=false);
	virtual ~CurlHTTP();

	virtual bool SetNtlmAuthenticationParams(std::string userName,std::string password);
	virtual bool SetBasicAuthenticationParams(std::string userName,std::string password);
	virtual void SetOutGoingInterface(std::string sourceInterface); // VNGFE-7269 The name can be an interface name, an IP address, or a host name
	virtual bool Sendrequest(CURLcode& retCode,std::string strUrl,bool grabHeaders = true, bool grabUrl = true, const HTTP_REQUEST_TYPE method = eHttpGet);
	virtual bool PostRequest(CURLcode& res, const std::string& strUrl,const std::string& strRequest,const bool grabHeaders = true, const bool grabUrl = true);
	virtual void AddAditionalConfiguration();
	virtual bool WriteResponseToFile(std::string strFileName="Httpout.html");
	virtual bool WriteResponseHeaderToFile(std::string strFileName/*="HttpHeadersout.txt"*/);
	std::string* GetResponseContent() { return m_pstrContent; }
	void SetTimeout(DWORD timeout) { m_timeout = timeout;}

	static char* LoadSoapBufferFromFile(std::string fileName);
	static std::string GetCurlStatusAsString(CURLcode status);
	std::string GetResponseHeader() { return m_headers;}
	static bool IsSecuredUrl(const std::string& strUrl);

protected:
	CURL* m_pcurl;
	std::string m_userName; //combination of user name : Password to connect with NTLM
	std::string m_password;
	char* m_userNameAndPasswordNTLM;
	std::string* m_pstrContent;
	std::string m_headers;
	bool m_bIsNTLMAuth;  //this flag when Set will enable to use NTLM Authentication
						//NTLM authentication requires user name and password to be sent
						//Moreover the first request will fail and a second request is Must to be able to connect
	bool m_bIsSecured; //if this field is set than the relevant request Skip verification will be refered
	bool m_bSkipPeerVerification,m_bSkipHostNameVerfication;
	char* m_pszErrorBuffer;
	bool m_bIsSetSourceIF;
    std::string m_sourceIF;
    DWORD m_timeout;

	bool SetMethodConfiguration(const HTTP_REQUEST_TYPE method);

	static size_t writefunction( void *ptr , size_t size , size_t nmemb , void *stream )
	{
		if ( !((DATA*) stream)->bGrab )
			return 0;
		std::string* pStr = ((DATA*) stream)->pstr;
		bool bClear = ((DATA*) stream)->bClearString;
		if (bClear)
		{
			pStr->empty();
			pStr->clear();
			((DATA*) stream)->bClearString = false;
		}

		if ( size * nmemb )
			pStr->append((const char*) ptr, size * nmemb);

		return nmemb * size;
	}

	static void dump(const char *text,
	            FILE *stream, unsigned char *ptr, size_t size,
	            char nohex)
	{
	    size_t i;
	    size_t c;

	    unsigned int width=0x10;

	    if(nohex)
	      /* without the hex output, we can fit more on screen */
	      width = 0x40;

	    fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
	            text, (long)size, (unsigned long)size);

	    for(i=0; i<size; i+= width) {

	      fprintf(stream, "%4.4lx: ", (unsigned long)i);

	      if(!nohex) {
	        /* hex not disabled, show it */
	        for(c = 0; c < width; c++)
	          if(i+c < size)
	            fprintf(stream, "%02x ", ptr[i+c]);
	          else
	            fputs("   ", stream);
	      }

	      for(c = 0; (c < width) && (i+c < size); c++) {
	        /* check for 0D0A; if found, skip past and start a new line of output */
	        if (nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
	          i+=(c+2-width);
	          break;
	        }
	        fprintf(stream, "%c",
	                (ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');
	        /* check again for 0D0A, to avoid an extra \n if it's at width */
	        if (nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
	          i+=(c+3-width);
	          break;
	        }
	      }
	      fputc('\n', stream); /* newline */
	    }
	    fflush(stream);
	 }

	static  int my_trace(CURL *handle, curl_infotype type,char *data, size_t size,void *userp)
	{
	    struct data *config = (struct data *)userp;
	    const char *text;
	    (void)handle; /* prevent compiler warning */

	    switch (type) {
	    case CURLINFO_TEXT:
	      fprintf(stderr, "== Infom_strContent: %s", data);
	    default: /* in case a new one is introduced to shock us */
	      return 0;

	    case CURLINFO_HEADER_OUT:
	      text = "=> Send header";
	      break;
	    case CURLINFO_DATA_OUT:
	      text = "=> Send data";
	      break;
	    case CURLINFO_SSL_DATA_OUT:
	      text = "=> Send SSL data";
	      break;
	    case CURLINFO_HEADER_IN:
	      text = "<= Recv header";
	      break;
	    case CURLINFO_DATA_IN:
	      text = "<= Recv data";
	      break;
	    case CURLINFO_SSL_DATA_IN:
	      text = "<= Recv SSL data";
	      break;
	   }

	   fprintf(stderr, "%s - data: %s; size: %zu; trace_ascii: %d", text, data, size, static_cast<int>(config->trace_ascii));

	   return 0;
	}

};

#endif /* CURLHTTP_H_ */
