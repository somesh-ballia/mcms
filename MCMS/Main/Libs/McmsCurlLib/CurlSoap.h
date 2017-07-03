/*
 * CurlSoap.h
 *
 *  Created on: Jun 17, 2009
 *      Author: kobi
 */

#ifndef CURLSOAP_H_
#define CURLSOAP_H_

#include "CurlHTTP.h"

class CurlSoap: public CurlHTTP
{
public:
	CurlSoap(bool bIsSecured = false ,bool bSkipPeerVerification = false, bool bSkipHostNameVerfication = false);
	virtual ~CurlSoap();

	virtual bool PostSoapRequest(CURLcode& retCode, const std::string& strUrl,
			const std::string& strSoapAction, const std::string& strRequest,
			const bool grabHeaders = true, const bool grabUrl = true);

	virtual bool PostSoapRequestFromFile(CURLcode& retCode, const std::string& strUrl,
			const std::string& strSoapAction, const std::string& strPostDataFileName,
			const bool grabHeaders = true, const bool grabUrl = true);

	virtual void AddAditionalConfiguration();
	void ResetCurlLib();
private:
	bool m_bFirstSendTrans;
	//char m_strCombinedSoap[1024];
};

#endif /* CURLSOAP_H_ */
