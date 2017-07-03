#!/mcms/python/bin/python
# -*- coding: utf-8 -*-


from McmsConnection import *

timeToSleep = 5
print "sleep " + str(timeToSleep) + " seconds, because ConfParty takes its time during startup"
sleep(timeToSleep)

connection = McmsConnection()
connection.Connect()


connection.LoadXmlFile("Scripts/UpdateIPService.xml")


utf8SendName = u"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234ЖОПА"
utf8ReceivedName = u"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234ЖО"

connection.ModifyXml("IP_SERVICE","GATEKEEPER_NAME", utf8SendName)

connection.Send("Status OK")


connection.LoadXmlFile("Cfg/IPServiceListTmp.xml")

gateKeeperName = connection.loadedXml.getElementsByTagName("GATEKEEPER_NAME")[0].firstChild.data

if(gateKeeperName <> utf8ReceivedName):
    print "the names are not equal"
#    print gateKeeperName.encode("utf-8") + " <> " + utf8ReceivedName.encode("utf-8")
    sys.exit(1)

print gateKeeperName.encode("utf-8")
print "the big string was sliced properly"


timeToSleep = 5
print "sleep " + str(timeToSleep) + " seconds, because Logger takes its time under valgrind"
sleep(timeToSleep)
