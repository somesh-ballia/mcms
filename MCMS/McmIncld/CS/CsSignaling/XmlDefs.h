// XmlDefs.h
// Ami Noy

#ifndef _XMLDEFS_H_
#define _XMLDEFS_H_



#include "DataTypes.h"


// Macros:
//--------
#define XmlMessageLen		(1024 * 128)

#define MaxUnzipHeaderSize		512
#define CompressXmlBufferSize	XmlMessageLen
#define UncompressXmlBufferSize XmlMessageLen

//#define __NumOfElementsInFormatValidation__

#define IsStartupIndication(mcmsOpcode) \
    ((mcmsOpcode >= CS_FIRST_STARTUP_IND) && \
     (mcmsOpcode <= CS_LAST_STARTUP_IND))
     
#define IsStartupRequest(mcmsOpcode) \
    ((mcmsOpcode >= CS_FIRST_STARTUP_REQ) && \
     (mcmsOpcode <= CS_LAST_STARTUP_REQ))
     
#define IsServiceIndication(mcmsOpcode) \
    ((mcmsOpcode >= CS_FIRST_SERVICE_IND) && \
     (mcmsOpcode <= CS_LAST_SERVICE_IND))

#define IsServiceRequest(mcmsOpcode) \
    ((mcmsOpcode >= CS_FIRST_SERVICE_REQ) && \
     (mcmsOpcode <= CS_LAST_SERVICE_REQ))

#define IsCallTaskIndication(mcmsOpcode) \
	((mcmsOpcode >= H323_CS_SIG_FIRST_IND) && \
     (mcmsOpcode <= H323_CS_SIG_LAST_IND))
     
#define IsGkIfTaskIndication(mcmsOpcode) \
	((mcmsOpcode >= H323_CS_RAS_FIRST_IND) && \
     (mcmsOpcode <= H323_CS_RAS_LAST_IND))

#define IsMcmsRequestToCallTask(mcmsOpcode) \
	((mcmsOpcode >= H323_CS_SIG_FIRST_REQ) && \
     (mcmsOpcode <= H323_CS_SIG_LAST_REQ))

#define IsMcmsRequestToGkIfTask(mcmsOpcode) \
	((mcmsOpcode >= H323_CS_RAS_FIRST_REQ) && \
	 (mcmsOpcode <= H323_CS_RAS_LAST_REQ)) 

#define IsSipTaskIndication(spOpcode)		\
    ((spOpcode >= SIP_CS_SIG_FIRST_IND) &&	\
     (spOpcode < SIP_CS_SIG_LAST_IND))

#define IsSipTaskRequest(spOpcode)			\
    ((spOpcode >= SIP_CS_SIG_FIRST_REQ) &&	\
     (spOpcode < SIP_CS_SIG_LAST_REQ))

#define IsProxyIndication(prxOpcode)			\
    ((prxOpcode >= SIP_CS_PROXY_FIRST_IND) &&	\
     (prxOpcode < SIP_CS_PROXY_LAST_IND))

#define IsProxyRequest(prxOpcode)				\
    ((prxOpcode >= SIP_CS_PROXY_FIRST_REQ) &&	\
     (prxOpcode < SIP_CS_PROXY_LAST_REQ))

#define IsDnsIndication(dnsOpcode)			\
	((dnsOpcode >= DNS_CS_FIRST_IND) &&	\
     (dnsOpcode < DNS_CS_LAST_IND))

#define IsDnsRequest(dnsOpcode)			\
	((dnsOpcode >= DNS_CS_FIRST_REQ) &&	\
     (dnsOpcode < DNS_CS_LAST_REQ))  

//_mccf_
#define IsMccfIndication(spOpcode)		\
    ((spOpcode >= SIP_CS_MCCF_SIG_FIRST_IND) &&	\
     (spOpcode < SIP_CS_MCCF_SIG_LAST_IND))

#define IsMccfRequest(spOpcode)			\
    ((spOpcode >= SIP_CS_MCCF_SIG_FIRST_REQ) &&	\
     (spOpcode < SIP_CS_MCCF_SIG_LAST_REQ))


#endif //_XMLDEFS_H_
