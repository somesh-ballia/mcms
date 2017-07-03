// HeaderParams.h: interface for the CHeaderParams class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HEADERPARAMS_H__541F9EB8_33AC_4773_BF78_5D9CAE730BCC__INCLUDED_)
#define AFX_HEADERPARAMS_H__541F9EB8_33AC_4773_BF78_5D9CAE730BCC__INCLUDED_

#include "ApacheDefines.h"
#include "OperatorDefines.h"


class CHeaderParams {

public:

	int m_nMessageId;
	int m_nAuthorization;
	int m_nContentLength;
	bool m_bZip;

public:

	CHeaderParams()
	{
		m_nMessageId = -1;
		m_nAuthorization = GUEST;
		m_nContentLength = 0;
		m_bZip = false;
	}
};


#endif //AFX_HEADERPARAMS_H__541F9EB8_33AC_4773_BF78_5D9CAE730BCC__INCLUDED_


