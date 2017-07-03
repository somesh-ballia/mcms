#!/mcms/python/bin/python


import sys
import os

from McmsConnection import *


#define HTTP_CONTINUE                      100
#define HTTP_SWITCHING_PROTOCOLS           101
#define HTTP_PROCESSING                    102
#define HTTP_OK                            200
#define HTTP_CREATED                       201
#define HTTP_ACCEPTED                      202
#define HTTP_NON_AUTHORITATIVE             203
#define HTTP_NO_CONTENT                    204
#define HTTP_RESET_CONTENT                 205
#define HTTP_PARTIAL_CONTENT               206
#define HTTP_MULTI_STATUS                  207
#define HTTP_MULTIPLE_CHOICES              300
#define HTTP_MOVED_PERMANENTLY             301
#define HTTP_MOVED_TEMPORARILY             302
#define HTTP_SEE_OTHER                     303
#define HTTP_NOT_MODIFIED                  304
#define HTTP_USE_PROXY                     305
#define HTTP_TEMPORARY_REDIRECT            307
#define HTTP_BAD_REQUEST                   400
#define HTTP_UNAUTHORIZED                  401
#define HTTP_PAYMENT_REQUIRED              402
#define HTTP_FORBIDDEN                     403
#define HTTP_NOT_FOUND                     404
#define HTTP_METHOD_NOT_ALLOWED            405
#define HTTP_NOT_ACCEPTABLE                406
#define HTTP_PROXY_AUTHENTICATION_REQUIRED 407
#define HTTP_REQUEST_TIME_OUT              408
#define HTTP_CONFLICT                      409
#define HTTP_GONE                          410
#define HTTP_LENGTH_REQUIRED               411
#define HTTP_PRECONDITION_FAILED           412
#define HTTP_REQUEST_ENTITY_TOO_LARGE      413
#define HTTP_REQUEST_URI_TOO_LARGE         414
#define HTTP_UNSUPPORTED_MEDIA_TYPE        415
#define HTTP_RANGE_NOT_SATISFIABLE         416
#define HTTP_EXPECTATION_FAILED            417
#define HTTP_UNPROCESSABLE_ENTITY          422
#define HTTP_LOCKED                        423
#define HTTP_FAILED_DEPENDENCY             424
#define HTTP_UPGRADE_REQUIRED              426
#define HTTP_INTERNAL_SERVER_ERROR         500
#define HTTP_NOT_IMPLEMENTED               501
#define HTTP_BAD_GATEWAY                   502
#define HTTP_SERVICE_UNAVAILABLE           503
#define HTTP_GATEWAY_TIME_OUT              504
#define HTTP_VERSION_NOT_SUPPORTED         505
#define HTTP_VARIANT_ALSO_VARIES           506
#define HTTP_INSUFFICIENT_STORAGE          507
#define HTTP_NOT_EXTENDED                  510



def HttpStatusToString(status):
    if(200 == status):
        return "HTTP_OK"
    if(403 == status):
        return "HTTP_FORBIDDEN"
    return "Unknown"


class IpDestination:
    m_IpAddress = "127.0.0.1"
    #m_Port = "8080"
    m_Port = "80"

    def Dump(self, title):
        print title + " " + self.m_IpAddress + ":" + self.m_Port
    

def ParseArgvParams(argv, outIpDestination):
    cnt = 0
    for arg in argv:
        cnt = cnt + 1
        if(cnt == 2):
            outIpDestination.m_IpAddress = arg
        if(cnt == 3):
            outIpDestination.m_Port = arg






ipDestination = IpDestination()
ParseArgvParams(sys.argv, ipDestination)

print ""
ipDestination.Dump("Sending TRACE request to")

conn = McmsConnection()
xmlResponse = conn.SendSpecialXmlRequest("TRACE", ipDestination.m_IpAddress, ipDestination.m_Port)


print ""
print "Response"

headers = xmlResponse.getheaders()

for (header, data) in headers:
    print header + " : " + data

statusString = HttpStatusToString(xmlResponse.status)

print ""
if(200 == xmlResponse.status):
    print "TRACE succeeded, status = " + str(xmlResponse.status) + " : " + statusString
else:
    print "TRACE did not succeed, status = " + str(xmlResponse.status) + " : " + statusString
print ""

