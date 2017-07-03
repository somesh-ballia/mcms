#!/mcms/python/bin/python
# -*- coding: utf-8 -*-




from McmsConnection import *


sleep(10)
print "sleep(10) - becouse ConfParty needs some time during its startup"


connection = McmsConnection()
connection.Connect()


connection.LoadXmlFile("Scripts/UpdateIPService.xml")

expectedStatus = "Status OK"
connection.Send(expectedStatus)





headers = {"Content-Type": "text/xml",
           "Server": "PolycomHTTPServer",
           "Connection": "Keep-Alive",
           "Cache-control": "private"}
connection.SendExpandedXml(headers, connection.loadedXml, expectedStatus)

print "Send without charset in header OK"




headers = {"Content-type": "text/xml; charset=cucu_lulu",
           "Server": "PolycomHTTPServer",
           "Connection": "Keep-Alive",
           "Cache-control": "private"}

expectedStatus = "Cannot encode unknown character set"
connection.SendExpandedXml(headers, connection.loadedXml, expectedStatus)

print "Send unknown charset OK"



headers = {"Content-type": "text/xml; charset=ASCII",
           "Server": "PolycomHTTPServer",
           "Connection": "Keep-Alive",
           "Cache-control": "private"}

expectedStatus = "Cannot determine encoding of character set"
connection.SendExpandedXml(headers, connection.loadedXml, expectedStatus)

print "Send conflict between header and comment OK"

